#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "config.h"
#include "fs/FSLoaderWorker.h"

void parse_args(int argc, char *argv[], Config *dest) {
    int opt;
    bool dir = false, mode = false, engine = false;

    while ((opt = getopt(argc, argv, "d:m:s:")) != -1) {
        switch (opt) {
            case 'd':
                dir = true;
                dest->directory = optarg;
                break;
            case 'm':
                mode = true;
                if (std::string(optarg) == "load")
                    dest->mode = LOAD;
                else if (std::string(optarg) == "compact")
                    dest->mode = COMPACT;
                else {
                    fprintf(stderr, "invalid option for mode: '%s'\n", optarg);
                    goto err;
                }
                break;
            case 's':
                engine = true;
                if (std::string(optarg) == "fs")
                    dest->engine = FS;
                else if (std::string(optarg) == "quarkstore")
                    dest->engine = QUARKSTORE;
                else {
                    fprintf(stderr, "invalid option for storage engine: '%s'\n", optarg);
                    goto err;
                }
                break;
            case '?':
            default:
                goto err;
        }
    }

    if (!(dir && mode && engine)) goto err;
    return;
err:
    fprintf(stderr, "usage: %s -d <data directory> -m <load | compact> -s <fs | quarkstore>\n", argv[0]);
    exit(EXIT_FAILURE);
}

int load(Config *config) {
    printf("%-25s %20ld\n%-25s %20ld\n%-25s %20ld\n", "Dataset size:", DATASET_SIZE, "SST file size:", SSTFILE_SIZE,
           "Number of SST files:", N_SSTFILES);

    int status;
    std::string cmd;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "rm -rf %s/*\n", config->directory.c_str());
    cmd = buf;

    status = system(cmd.c_str());
    if (status != 0) {
        perror("system");
        return -1;
    }

    std::atomic<int> counter(0);
    std::vector<std::thread> loaders;

    for (int i = 0; i < LOADER_THREADS; ++i) {
        auto worker = std::make_shared<FSLoaderWorker>(config, &counter);
        loaders.emplace_back([worker]() { worker->Work(); });
    }
    for (int i = 0; i < LOADER_THREADS; ++i) {
        loaders[i].join();
    }

    return 0;
}

int main(int argc, char *argv[]) {
    Config config;
    int ok;

    parse_args(argc, argv, &config);

    printf("%-20s %25s\n%-20s %25s\n%-20s %25s\n", "Data directory:", config.directory.c_str(),
           "Mode:", config.mode == LOAD ? "Load" : "Compact",
           "Storage engine:", config.engine == FS ? "File system" : "QuarkStore");

    if (config.mode == LOAD) {
        if (config.engine == FS) {
            ok = load(&config);
        }
    }

    return 0;
}