#include "Client.h"

#include "../utils/utils.h"

namespace ycsbc {
Client::Client(Config* config) : config_(config)
{
    bool ok;
    std::ifstream input(config->ycsb_workload_path);

    ok = props_.Load(input);
    if (!ok) {
        fprintf(stderr, "Parsing workload %s failed\n",
                config->ycsb_workload_path.c_str());
    }
    input.close();

    wl_ = new CoreWorkload();
    wl_->Init(props_);
    proxy_ = new WorkloadProxy(wl_);
    producer_ = new WorkloadProducer(proxy_);
    db_ = new DB(config_);
    ok = db_->Open() == 0;
    if (!ok) {
        fprintf(stderr, "failed to open DB\n");
    }
}

Client::~Client()
{
    db_->Close();
}

int Client::Load()
{
    int ok;
    uint64_t record_count;

    record_count = round_up_to_nearest_multiple(
        stoull(props_[ycsbc::CoreWorkload::RECORD_COUNT_PROPERTY]),
        config_->sst_file_size / BLOCK_SIZE);

    printf("Starting load of %ld KV pairs (~%ld MiB)...\n", record_count,
           (record_count * config_->kv_size()) >> 20);

    auto work_start = TIME_NOW;

    for (uint64_t i = 0; i < record_count; i++) {
        WorkloadProducer::Request* req = producer_->GetLoadRequest();
        ok = db_->Put(req->Key(), "-");
        if (ok == -1) {
            fprintf(stderr, "loading failed\n");
            return -1;
        }
    }

    double load_time = TIME_DURATION(work_start, TIME_NOW);
    printf("Finished loading in %.3lf s.\n\n%s\n", load_time / 1000000.0,
           db_->ToString().c_str());

    db_->ClearStats();

    return 0;
}

int Client::Work()
{
    int ok;
    uint64_t operation_count;

    uint64_t read_ops = 0, update_ops = 0, insert_ops = 0, scan_ops = 0,
             readmodifywrite_ops = 0;
    uint64_t reads = 0, writes = 0;
    uint64_t scan_sizes = 0;

    operation_count = stoull(props_[ycsbc::CoreWorkload::OPERATION_COUNT_PROPERTY]);

    printf("Starting work (%ld operations)...\n", operation_count);
    printf("Start time: %s\n", GetDayTime().c_str());
    auto work_start = TIME_NOW;

    TimePoint op_start;
    double op_time;

    for (uint64_t i = 0; i < operation_count; i++) {
        WorkloadProducer::Request* req = producer_->GetRequest();
        std::map<std::string, std::string>::iterator it;
        std::string dest;
        assert(req);

        op_start = TIME_NOW;

        switch (req->Type()) {
            case READ:
                ok = db_->Get(req->Key(), dest);
                read_ops++;
                break;
            case UPDATE:
                ok = db_->Put(req->Key(), "-");
                update_ops++;
                break;
            case INSERT:
                ok = db_->Put(req->Key(), "-");
                insert_ops++;
                break;
            case SCAN:
                ok = db_->Scan(req->Key(), req->Len(), it);
                scan_ops++;
                scan_sizes += req->Len();
                break;
            case READMODIFYWRITE:
                ok = db_->Get(req->Key(), dest);
                if (ok == -1) {
                    return -1;
                }
                ok = db_->Put(req->Key(), "-");
                readmodifywrite_ops++;
                break;
            default:
                fprintf(stderr, "unknown request type %d\n", req->Type());
                return -1;
        }

        op_time = TIME_DURATION(op_start, TIME_NOW);

        if (ok == -1) {
            return -1;
        }

        request_time_.Insert(op_time);

        switch (req->Type()) {
            case READ:
                reads++;
                // printf("Inserting read time: %.3lf us\n", op_time);
                read_time_.Insert(op_time);
                break;
            case UPDATE:
            case INSERT:
            case READMODIFYWRITE:
                writes++;
                // printf("Inserting write time: %.3lf us\n", op_time);
                write_time_.Insert(op_time);
                break;
            case SCAN:
                reads++;
                scan_time_.Insert(op_time);
                break;
            default:
                return -1;
        }
    }

    double work_time = TIME_DURATION(work_start, TIME_NOW);
    printf("End time:   %s\n", GetDayTime().c_str());
    printf("Work time:  %.3lf s", work_time / 1000000.0);
    printf("\n\n");

    printf("%s\n", db_->ToString().c_str());

    printf(
        "Finished work: %ld read operations, %ld update operations, %ld insert "
        "operations, %ld scan operations, %ld "
        "readmodifywrite operations\n",
        read_ops, update_ops, insert_ops, scan_ops, readmodifywrite_ops);

    // Reads
    if (read_time_.Size() > 0) {
        printf(
            "READS (%ld)\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf "
            "µs\n\t%-25s "
            "%10.3lf µs\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf µs\n",
            read_time_.Size(), "Average latency:", read_time_.Sum() / read_time_.Size(),
            "Median latency:", read_time_.Tail(0.5), "P75:", read_time_.Tail(0.75),
            "P90:", read_time_.Tail(0.90), "P99:", read_time_.Tail(0.99),
            "P999:", read_time_.Tail(0.999));
    }
    else {
        printf("No reads\n");
    }

    // Writes
    if (write_time_.Size() > 0) {
        printf(
            "WRITES (%ld)\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf "
            "µs\n\t%-25s "
            "%10.3lf µs\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf µs\n\t%-25s %10.3lf "
            "µs\n\t%-25s "
            "%10.3lf ms\n",
            write_time_.Size(),
            "Average latency:", write_time_.Sum() / write_time_.Size(),
            "Median latency:", write_time_.Tail(0.5), "P75:", write_time_.Tail(0.75),
            "P90:", write_time_.Tail(0.90), "P99:", write_time_.Tail(0.99),
            "P99.9:", write_time_.Tail(0.999), "P99.99:", write_time_.Tail(0.9999),
            "P99.999:", write_time_.Tail(0.99999) / 1000.0);
    }
    else {
        printf("No writes\n");
    }

    // Scans
    if (scan_time_.Size() > 0) {
        printf(
            "SCANS (%ld)\n\t%-25s %10.3lf ms\n\t%-25s %10.3lf ms\n\t%-25s %10.3lf "
            "ms\n\t%-25s "
            "%10.3lf ms\n\t%-25s %10.3lf ms\n\t%-25s %10.3lf ms\n\t%-25s %10.3lf "
            "ms\n\t%-25s "
            "%10.3lf ms\n",
            scan_time_.Size(),
            "Average latency:", (scan_time_.Sum() / scan_time_.Size()) / 1000.0,
            "Median latency:", scan_time_.Tail(0.5) / 1000.0, "P75:", scan_time_.Tail(0.75) / 1000.0,
            "P90:", scan_time_.Tail(0.90) / 1000.0, "P99:", scan_time_.Tail(0.99) / 1000.0,
            "P99.9:", scan_time_.Tail(0.999) / 1000.0, "P99.99:", scan_time_.Tail(0.9999) / 1000.0,
            "P99.999:", scan_time_.Tail(0.99999) / 1000.0);
        printf("Average scan size: %ld KV pairs\n", scan_sizes / scan_ops);
    }
    else {
        printf("No scans\n");
    }   

    printf("Average operation latency: %.3lf µs\n",
           request_time_.Sum() / request_time_.Size());
    printf("Operations per second: %.3lf K\n",
           operation_count / work_time * 1000 * 1000 / 1000);

    // Compactions
    TimeRecord compactions = db_->GetStats().compactionTimes;
    if (compactions.Size() > 0) {
        printf(
            "COMPACTIONS (%ld)\n\t%-25s %10.3lf s\n\t%-25s %10.3lf s\n\t%-25s %10.3lf "
            "s\n\t%-25s "
            "%10.3lf s\n\t%-25s %10.3lf s\n\t%-25s %10.3lf s\n",
            compactions.Size(),
            "Average latency:", (compactions.Sum() / compactions.Size()) / 1000000.0,
            "Median latency:", compactions.Tail(0.5) / 1000000.0,
            "P75:", compactions.Tail(0.75) / 1000000.0,
            "P90:", compactions.Tail(0.90) / 1000000.0,
            "P99:", compactions.Tail(0.99) / 1000000.0,
            "Last:", compactions.Last() / 1000000.0);
    }
    else {
        printf("No compactions\n");
    }

    return 0;
}

}  // namespace ycsbc