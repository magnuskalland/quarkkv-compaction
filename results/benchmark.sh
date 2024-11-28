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
    bash quarkcontroller_install.sh 1
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../../Quark/quarkstore/quarklibio/build

WORKLOAD_BASE=../ycsb/workloads
BASE_CMD="LD_LIBRARY_PATH=$LIB_PATH ../main"
ARGS="--mode=ycsb --compaction-picker=all"
FS_DBDIR=/mnt/nvme
mkdir -p $FS_DBDIR

InstallQuark

###############
## QS append ##
############### 

# fillseq
for s in 32 64; do
    cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/fillseq${s}gb --engine=quarkstore_append $ARGS"
    echo $cmd
    eval $cmd > nvme/quarkstore_append/fillseq${s}gb.txt
    InstallQuark
done

# # YCSB
# for s in 32; do
#     for w in a b c d e f; do
#         cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=quarkstore_append $ARGS"
#         echo $cmd
#         eval $cmd > nvme/quarkstore_append/workload${w}${s}gb.txt
#         InstallQuark
#     done
# done

#################
## QS baseline ##
#################

# # fillseq
# for s in 8 16 32 64; do
#     cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/fillseq${s}gb --engine=quarkstore $ARGS"
#     echo $cmd
#     eval $cmd > nvme/quarkstore/fillseq${s}gb.txt
#     InstallQuark
# done

# YCSB
for s in 32; do
    for w in a b; do
        cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=quarkstore $ARGS"
        echo $cmd
        eval $cmd > nvme/quarkstore/workload${w}${s}gb.txt
        InstallQuark
    done
done

########
## FS ##
########

ulimit -n 65535

# # fillseq
# rm -rf $FS_DBDIR/*
# for s in 8 16 32 64; do
#     cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/fillseq${s}gb --engine=fs --db-directory=$FS_DBDIR $ARGS"
#     echo $cmd
#     eval $cmd > nvme/fs/fillseq${s}gb.txt
#     rm -rf $FS_DBDIR/*
# done

# # YCSBs
# rm -rf $FS_DBDIR/*
# for s in 8 16 32; do
#     for w in a b c d e f; do
#         cmd="$BASE_CMD --ycsb-workload=$WORKLOAD_BASE/${s}gb/workload${w}${s}gb --engine=fs --db-directory=$FS_DBDIR $ARGS"
#         echo $cmd
#         eval $cmd > nvme/fs/workload${w}${s}gb.txt
#         rm -rf $FS_DBDIR/*
#     done
# done
