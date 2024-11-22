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
    bash quarkcontroller_install.sh $1
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../../Quark/quarkstore/quarklibio/build

WORKLOAD_BASE=../ycsb/workloads
BASE_CMD="LD_LIBRARY_PATH=$LIB_PATH ../main"
ARGS="--mode=ycsb --compaction-picker=one"
FS_DBDIR=/mnt/ramfs/quarkkv
# mkdir -p $FS_DBDIR

sizes=("8" "64" "512")
workloads=("a" "b" "c" "d" "f")

# InstallQuark
for s in "${sizes[@]}"; do
    for w in "${workloads[@]}"; do
        cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=quarkstore $ARGS"
        echo $cmd
        # eval $cmd
        # InstallQuark
    done
done

# rm -rf $FS_DBDIR/*
for s in "${sizes[@]}"; do
    for w in "${workloads[@]}"; do
        cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=fs --db-directory=$FS_DBDIR $ARGS"
        echo $cmd
        # eval $cmd
        # rm -rf $FS_DBDIR/*
    done
done
