### Build

- Checkout branch `block_concat` of QuarkStore. This contains the `ioctls` for doing block concatenation with QuarkStore.
- Compile necessary dependencies from QuarkStore: `quarklibio.so`, `quarkstore.ko`.
- Set environment variable `QUARKSTORE_SRC_DIR` to the directory of the QuarkStore source code.

```sh
export QUARKSTORE_SRC_DIR=<path>
```

- Run

```sh
make
```

### Running experiments

There are multiple ways of running QuarkKV experiments. The easiest way is to use [run.sh](/run.sh) in this directory. run.sh relies on the `insmod` from `Quark/experiments/tests/quarkcontroller_install.sh`, so make sure you have set correct devices in that script.

See [run.sh](/run.sh) for an example execution. You can see all possible configurations in the [argument parser](/utils/argparser.h), but the most important ones are the following:

- `--engine`: this specifies the storage engine to use: either a traditional file system. raw QuarkStore without QuarkFS, or raw QuarkStore with block concatenation. Possible options are `fs`, `quarkstore` and `quarkstore_append`. If you any of the QuarkStore engines, it will use the storage locations defined when inserting the QuarkStore kernel module, but if you use the file system option, you must specify which directory to store the database with the `--db-directory` option.
- `--mode`: this specifies if you want to run YCSB or specify your own benchmark with a number of reads and writes. If you specify YCSB, you must specify the relative path to the YCSB workload with the `--ycsb-workload` option. 

Example execution:

```sh
sudo QUARKSTORE_SRC_DIR=/Quark bash run.sh
```

For more rigorous benchmarking with YCSB, see [benchmark.sh](/results/benchmark.sh).

### Clean

`make clean`