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

int load(DB* db, Config* c)
{
    printf("Populating DB with %d KV pairs\n", c->populateSize);
    int ok;
    uint32_t count = 0;
    ZipfianKeyGenerator gen(c->practical_key_size);
    while (count++ < c->populateSize) {
        std::string key = gen.Generate();
        ok = db->Put(key, "-");
        if (ok == -1) {
            return -1;
        }
    }
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

    DB db(&config);
    ok = db.Open();
    if (ok == -1) {
        return EXIT_FAILURE;
    }
    printf("%s\n", config.ToString().c_str());

    if (config.mode == LOAD) {
        ok = load(&db, &config);
        if (ok == -1) {
            return EXIT_FAILURE;
        }
    }
    else if (config.mode == COMPACT) {
    }

    db.Close();
    return EXIT_SUCCESS;
}