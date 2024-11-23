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
    cd ../../Quark/experiments/tests
    bash quarkcontroller_uninstall.sh
    bash quarkcontroller_install.sh 0
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../../Quark/quarkstore/quarklibio/build

WORKLOAD_BASE=../ycsb/workloads
BASE_CMD="LD_LIBRARY_PATH=$LIB_PATH ../main"
ARGS="--mode=ycsb --compaction-picker=all"
FS_DBDIR=/mnt/pmem
mkdir -p $FS_DBDIR

InstallQuark

# fillseq QuarkStore copy
# for s in 8 16, 32 64; do
#     cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/fillseq${w}${s}gb --engine=quarkstore $ARGS"
#     echo $cmd
#     eval $cmd > quarkstore/fillseq${s}gb.txt
#     InstallQuark
# done

# fillseq QuarkStore append
for s in 8 16 32 64 128 256 512; do
    cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/fillseq${w}${s}gb --engine=quarkstore_append $ARGS"
    echo $cmd
    eval $cmd > quarkstore_append/fillseq${s}gb.txt
    InstallQuark
done

sizes=("8" "64" "512")
workloads=("a" "b" "c" "d" "f")

# YCSB QuarkStore copy
# InstallQuark
# for s in "${sizes[@]}"; do
#     for w in "${workloads[@]}"; do
#         cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=quarkstore $ARGS"
#         echo $cmd
#         eval $cmd
#         InstallQuark
#     done
# done

# YCSB FS
# rm -rf $FS_DBDIR/*
# for s in "${sizes[@]}"; do
#     for w in "${workloads[@]}"; do
#         cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=fs --db-directory=$FS_DBDIR $ARGS"
#         echo $cmd
#         eval $cmd 
#         rm -rf $FS_DBDIR/*
#     done
# done
