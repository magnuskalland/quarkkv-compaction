#include "Client.h"

#include "../utils/utils.h"

namespace ycsbc {
Client::Client(Config* config) : config_(config), db_(new DB(config))
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

    printf("Starting load of %ld KV pairs...\n", record_count);

    for (uint64_t i = 0; i < record_count; i++) {
        WorkloadProducer::Request* req = producer_->GetLoadRequest();
        ok = db_->Put(req->Key(), "-");
        if (ok == -1) {
            fprintf(stderr, "loading failed\n");
            return -1;
        }
    }

    printf("Finished loading.\n\n%s\n", db_->ToString().c_str());

    return 0;
}

int Client::Work()
{
    int ok;
    uint64_t operation_count;

    uint64_t reads = 0, updates = 0, inserts = 0, readmodifywrites = 0;

    operation_count = stoull(props_[ycsbc::CoreWorkload::OPERATION_COUNT_PROPERTY]);

    printf("Starting work (%ld operations)...\n", operation_count);

    for (uint64_t i = 0; i < operation_count; i++) {
        WorkloadProducer::Request* req = producer_->GetRequest();
        std::string dest;
        assert(req);
        switch (req->Type()) {
            case READ:
                ok = db_->Get(req->Key(), dest);
                reads++;
                break;
            case UPDATE:
                ok = db_->Put(req->Key(), "-");
                updates++;
                break;
            case INSERT:
                ok = db_->Put(req->Key(), "-");
                inserts++;
                break;
            case READMODIFYWRITE:
                ok = db_->Get(req->Key(), dest);
                if (ok == -1) {
                    return -1;
                }
                ok = db_->Put(req->Key(), "-");
                readmodifywrites++;
                break;
            default:
                fprintf(stderr, "unknown request type %d\n", req->Type());
                return -1;
        }
        if (ok == -1) {
            return -1;
        }
    }

    printf("Finished work (%ld reads, %ld updates, %ld inserts, %ld readmodifywrites)\n",
           reads, updates, inserts, readmodifywrites);

    printf("%s\n", db_->ToString().c_str());

    return 0;
}
}  // namespace ycsbc