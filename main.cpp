#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "DB.h"
#include "include/config.h"
#include "utils/argparser.h"
#include "utils/prepare.h"
#include "ycsb/Client.h"

void verify_config(Config* c)
{
    assert(c->engine == FS || c->engine == QUARKSTORE);
    assert(c->mode == MANUAL || c->mode == YCSB);
    assert(c->cp == ALL || c->cp == ONE);
    assert(c->distribution == UNIFORM || c->distribution == ZIPFIAN);
    assert(c->key_size == DEFAULT_KEY_SIZE);
    assert(c->practical_key_size <= c->key_size);
    assert(c->sst_file_size % (uint64_t)c->kv_size() == 0);
    assert(BLOCK_SIZE % c->index_block_entry_size() == 0);
    assert(BLOCK_SIZE == c->kv_size());
}

int run_ycsb(Config* config)
{
    int ok;
    ycsbc::Client client(config);
    ok = client.Load();
    if (ok == -1) {
        fprintf(stderr, "error during loading\n");
        return -1;
    }
    ok = client.Work();
    if (ok == -1) {
        fprintf(stderr, "error during work\n");
        return -1;
    }
    return 0;
}

int run_manual(Config* config)
{
    int ok;
    KeyGenerator* gen;
    DB db(config);

    ok = db.Open();
    if (ok == -1) {
        return -1;
    }

    gen = config->distribution == UNIFORM
              ? static_cast<KeyGenerator*>(
                    new UniformKeyGenerator(config->practical_key_size))
              : static_cast<KeyGenerator*>(
                    new ZipfianKeyGenerator(config->practical_key_size));

    for (uint64_t i = 0; i < config->prepopulate_size; i++) {
        std::string key = gen->Generate();
        ok = db.Put(key, "-");
        if (ok == -1) {
            return -1;
        }
    }

    uint64_t reads = 0, writes = 0;
    double ratio = static_cast<double>(config->read_size) / config->write_size;
    double read_target = 0.0, write_target = 0.0;
    std::string val;
    int keys_found = 0;

    while (reads < config->read_size || writes < config->write_size) {
        std::string key = gen->Generate();
        if (read_target <= write_target || config->write_size == 0) {
            reads++;
            read_target++;
            ok = db.Get(key, val);
            if (val.size() > 0) {
                keys_found++;
            }
        }
        else {
            writes++;
            write_target = write_target + ratio;
            ok = db.Put(key, "-");
        }
        if (ok == -1) {
            return -1;
        }
    }

    assert(reads == config->read_size && writes == config->write_size);

    db.Close();
    return 0;
}

int main(int argc, char* argv[])
{
    Config config;
    int ok;

    parse_args(argc, argv, &config);
    verify_config(&config);

    printf("%s\n", config.ToString().c_str());
    ok = config.mode == MANUAL ? run_manual(&config) : run_ycsb(&config);
    if (ok == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}