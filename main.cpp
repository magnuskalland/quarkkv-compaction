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

static const char *usage = R"(usage %s [OPTIONS]
    -e, --engine <fs|quarkstore>    Specify storage engine      (REQUIRED)
    -d, --data-directory            Data directory              (REQUIRED)
    -w, --working-directory         Working directory           (REQUIRED)
    -m, --mode <load|compact>       Specify mode                (Default 'compact')
    -l, --levels                    Number of SST file levels   (Default %d)
    -s, --fanout                    Level size multiplier       (Default %d)
    -k, --key-size                  Key size                    (Default %d)
    -z, --level0-max                Level 0 max size            (Default %d)
    -f, --sst-file-size             SST file size               (Default %d)
    -h, --help                      Display this help message
)";

void parse_args(int argc, char *argv[], Config *dest)
{
    int opt;
    bool engine = false, ddir = false, wdir = false;

    static struct option long_options[] = {
        {"engine", required_argument, 0, 'e'},
        {"mode", required_argument, 0, 'm'},
        {"data-directory", required_argument, 0, 'd'},
        {"working-directory", required_argument, 0, 'w'},
        {"levels", required_argument, 0, 'l'},
        {"fanout", required_argument, 0, 's'},
        {"key-size", required_argument, 0, 'k'},
        {"level0-max", required_argument, 0, 'z'},
        {"sst-file-size", required_argument, 0, 'f'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "e:m:d:w:l:s:k:f:h", long_options, nullptr)) != -1) {
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
                if (std::string(optarg) == "load")
                    dest->mode = LOAD;
                else if (std::string(optarg) == "compact")
                    dest->mode = COMPACT;
                else {
                    fprintf(stderr, "invalid mode: %s\n", optarg);
                    goto parse_args_err;
                }
                break;

            case 'd':
                ddir = true;
                dest->ddir = std::string(optarg);
                break;

            case 'w':
                wdir = true;
                dest->wdir = std::string(optarg);
                break;

            case 'l':
                dest->n_levels = atoi(optarg);
                break;

            case 's':
                dest->level_size_multiplier = atoi(optarg);
                break;

            case 'k':
                dest->key_size = atoi(optarg);
                break;

            case 'z':
                dest->level0_max_size = atoi(optarg);
                break;

            case 'f':
                dest->sst_file_size = atoi(optarg);
                break;

            case 'h':
            case '?':
            default:
                goto parse_args_err;
        }
    }
    if (!(engine && ddir && wdir)) goto parse_args_err;
    return;
parse_args_err:
    fprintf(stderr, usage, argv[0], DEFAULT_LEVELS, DEFAULT_LEVEL_SIZE_MULTIPLIER, DEFAULT_KEY_SIZE,
            DEFAULT_LEVEL0_MAX_SIZE, DEFAULT_SST_FILE_SIZE);
    exit(EXIT_FAILURE);
}

int load_fs(Config *config)
{
    int status;
    std::string cmd;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "rm -rf %s/*\n", config->wdir.c_str());
    cmd = buf;

    status = system(cmd.c_str());
    if (status != 0) {
        perror("system");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    Config config;
    int ok;

    parse_args(argc, argv, &config);

    printf("%s\n", config.ToString().c_str());

    DB db(&config);
    db.VerifyConfig();
    ok = db.Open();

    UniformKeyGenerator gen(config.key_size);

    while (1) {
        std::string key = gen.Generate();
        ok = db.Put(key, "-");
        if (ok == -1) {
            fprintf(stderr, "error");
        }
    }

    db.Close();
    return 0;
}