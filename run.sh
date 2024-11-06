#!/bin/bash

FlushDisk() {
    sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
    sudo sh -c "sync"
    sudo sh -c "sync"
    sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
}

# if [ "$EUID" -ne 0 ]; then
#     echo "Please run as root"
#     exit
# fi

InstallQuark() {
    cd ../Quark/experiments/tests
    bash quarkcontroller_uninstall.sh
    bash quarkcontroller_install.sh
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../Quark/quarkstore/quarklibio/build

ENGINE=fs
DIR=./dbdir
DISTRIBUTION=zipfian
PREPOPULATE=65536
WRITE_SIZE=0
READ_SIZE=100000
KEY_SIZE=12

if [[ "$DIR" == "quarkstore" ]]; then
    InstallQuark
else
    mkdir -p $DIR
    if [ "$PREPOPULATE" -gt 0 ]; then
        rm -rf $DIR/*
    fi
fi

LD_LIBRARY_PATH=$LIB_PATH ./main \
    --engine=$ENGINE \
    --db-directory=$DIR \
    --workload-distribution=$DISTRIBUTION \
    --prepopulate-size=$PREPOPULATE \
    --write-size=$WRITE_SIZE \
    --read-size=$READ_SIZE \
    --key-size=$KEY_SIZE

ret=0

rm -rf $DIR

set +e

exit $ret
