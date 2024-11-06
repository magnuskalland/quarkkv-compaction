#include "argparser.h"

#include <getopt.h>

static const char *usage = R"(usage %s [OPTIONS]
    -e, --engine <fs|quarkstore>    Specify storage engine      (REQUIRED)
    -m, --mode <manual|ycsb>        Specify mode                (Default 'manual')
    -y, --ycsb-workload             YCSB workload path          (Required if mode=ycsb)
    -d, --db-directory              DB directory                (Required if engine=fs)
    -p, --prepopulate-size          Number of initial DB size   (Default %d KVs (~%ld SSTs))
    -r, --read-size                 Number of KVs to read       (Default %ld)
    -w, --write-size                Number of KVs to write      (Default %ld)
    -c, --compaction-picker         Compaction picker           (Default '%s')
    -l, --levels                    Number of SST file levels   (Default %d)
    -f, --fanout                    Fanout                      (Default %d)
    -k, --key-size                  Key size                    (Default %d)
    -z, --level0-max                Level 0 max size            (Default %d)
    -s, --sst-file-size             SST file size               (Default %d MiB)
    -h, --help                      Display this help message
)";

void parse_args(int argc, char *argv[], Config *dest)
{
    int opt;
    bool engine = false, ycsb_workload = false, fs_dbdir = false;

    static struct option long_options[] = {
        {"engine", required_argument, 0, 'e'},
        {"mode", required_argument, 0, 'm'},
        {"ycsb-workload", required_argument, 0, 'y'},
        {"db-directory", required_argument, 0, 'd'},
        {"prepopulate-size", required_argument, 0, 'p'},
        {"read-size", required_argument, 0, 'r'},
        {"write-size", required_argument, 0, 'w'},
        {"compaction-picker", required_argument, 0, 'c'},
        {"levels", required_argument, 0, 'l'},
        {"fanout", required_argument, 0, 'f'},
        {"key-size", required_argument, 0, 'k'},
        {"level0-max", required_argument, 0, 'z'},
        {"sst-file-size", required_argument, 0, 's'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "e:m:y:d:p:r:w:c:l:f:k:z:s:h", long_options,
                              nullptr)) != -1) {
        switch (opt) {
            case 'e':
                engine = true;
                if (std::string(optarg) == "fs")
                    dest->engine = FS;
                else if (std::string(optarg) == "quarkstore")
                    dest->engine = QUARKSTORE;
                else {
                    fprintf(stderr, "invalid engine: %s\n", optarg);
                    goto parse_args_err;
                }
                break;

            case 'm':
                if (std::string(optarg) == "manual")
                    dest->mode = MANUAL;
                else if (std::string(optarg) == "ycsb")
                    dest->mode = YCSB;
                else {
                    fprintf(stderr, "invalid mode: %s\n", optarg);
                    goto parse_args_err;
                }
                break;

            case 'y':
                ycsb_workload = true;
                dest->ycsb_workload_path = std::string(optarg);
                break;

            case 'd':
                fs_dbdir = true;
                dest->fs_dbdir = std::string(optarg);
                break;

            case 'p':
                dest->prepopulate_size = atoi(optarg);
                break;

            case 'r':
                dest->read_size = atoi(optarg);
                break;

            case 'w':
                dest->write_size = atoi(optarg);
                break;

            case 'c':
                if (std::string(optarg) == "all")
                    dest->cp = ALL;
                else if (std::string(optarg) == "one")
                    dest->cp = ONE;
                else {
                    fprintf(stderr, "invalid compaction picker: %s\n", optarg);
                    goto parse_args_err;
                }
                break;

            case 'l':
                dest->n_levels = atoi(optarg);
                break;

            case 'f':
                dest->fanout = atoi(optarg);
                break;

            case 'k':
                dest->key_size = atoi(optarg);
                break;

            case 'z':
                dest->level0_max_size = atoi(optarg);
                break;

            case 's':
                dest->sst_file_size = atoi(optarg);
                break;

            case 'h':
            case '?':
            default:
                goto parse_args_err;
        }
    }
    if (!(engine && (dest->mode == MANUAL || ycsb_workload) &&
          (dest->engine == QUARKSTORE || fs_dbdir)))
        goto parse_args_err;
    return;
parse_args_err:
    fprintf(stderr, usage, argv[0], DEFAULT_PREPOPULATE_SIZE,
            (DEFAULT_PREPOPULATE_SIZE * BLOCK_SIZE) / DEFAULT_SST_FILE_SIZE,
            DEFAULT_READ_SIZE, DEFAULT_WRITE_SIZE, "One", DEFAULT_LEVELS, DEFAULT_FANOUT,
            DEFAULT_PRACTICAL_KEY_SIZE, DEFAULT_LEVEL0_MAX_SIZE,
            DEFAULT_SST_FILE_SIZE >> 20);
    exit(EXIT_FAILURE);
}
