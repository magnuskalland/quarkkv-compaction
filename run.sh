#!/bin/bash

FlushDisk() {
    sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
    sudo sh -c "sync"
    sudo sh -c "sync"
    sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
}

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit
fi

InstallQuark() {
    cd ../Quark/experiments/tests
    bash quarkcontroller_uninstall.sh
    bash quarkcontroller_install.sh 1
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../Quark/quarkstore/quarklibio/build

ENGINE=quarkstore
MODE=manual
WORKLOAD=ycsb/workloads/8gb/workloade8gb
DIR=./dbdir
DISTRIBUTION=zipfian
PREPOPULATE=16384
WRITE_SIZE=0
READ_SIZE=100000
KEY_SIZE=56
PICKER=all

if [[ "$ENGINE" == "quarkstore_append" || "$ENGINE" == "quarkstore" ]]; then
    InstallQuark
else
    mkdir -p "$DIR"
    if [ "$PREPOPULATE" -gt 0 ]; then
        rm -rf "$DIR"/*
    fi
fi

LD_LIBRARY_PATH=$LIB_PATH ./main \
    --engine=$ENGINE \
    --mode=$MODE \
    --ycsb-workload=$WORKLOAD \
    --db-directory=$DIR \
    --workload-distribution=$DISTRIBUTION \
    --prepopulate-size=$PREPOPULATE \
    --write-size=$WRITE_SIZE \
    --read-size=$READ_SIZE \
    --key-size=$KEY_SIZE \
    --compaction-picker=$PICKER

ret=0

set +e

exit $ret
