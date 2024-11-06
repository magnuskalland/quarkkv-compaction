#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "DB.h"
#include "include/config.h"
#include "utils/argparser.h"
#include "utils/prepare.h"

int run_ycsb(Config* config)
{
    return 0;
}

int run_manual(Config* config)
{
    int ok;
    KeyGenerator* gen;
    DB db(config);
    uint64_t reads, writes;

    ok = db.Open();
    if (ok == -1) {
        return -1;
    }

    gen = config->distribution == UNIFORM
              ? static_cast<KeyGenerator*>(new UniformKeyGenerator(config->key_size))
              : static_cast<KeyGenerator*>(new ZipfianKeyGenerator(config->key_size));

    for (uint64_t i = 0; i < config->prepopulate_size; i++) {
        std::string key = gen->Generate();
        ok = db.Put(key, "-");
        if (ok == -1) {
            return -1;
        }
    }

    db.Close();
    return 0;
}

int main(int argc, char* argv[])
{
    Config config;
    int ok;

    parse_args(argc, argv, &config);

    if (config.engine == FS) {
        ok = prepare_fs(&config);
        if (ok == -1) {
            return EXIT_FAILURE;
        }
    }

    printf("%s\n", config.ToString().c_str());
    ok = config.mode == MANUAL ? run_manual(&config) : run_ycsb(&config);
    if (ok == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}