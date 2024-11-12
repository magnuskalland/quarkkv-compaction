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
    bash quarkcontroller_install.sh
    cd $SCRIPT_DIR
    clear
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../Quark/quarkstore/quarklibio/build

WORKLOAD_BASE=ycsb/workloads
FS_DBDIR=/mnt/ramfs/quarkkv
ARGS="--mode=ycsb --compaction-picker=all"

mkdir -p $FS_DBDIR

workloads=("workloada16gb" "workloadb16gb" "workloadc16gb" "workloadd16gb" "workloadf16gb")

for wl in "${workloads[@]}"; do
   InstallQuark
   echo "Starting QuarkStore $wl"
   LD_LIBRARY_PATH=$LIB_PATH ./main --engine=quarkstore --ycsb-workload=$WORKLOAD_BASE/$wl $ARGS
done

cd ../Quark/experiments/tests
bash quarkcontroller_uninstall.sh
cd $SCRIPT_DIR

for wl in "${workloads[@]}"; do
    rm -rf $FS_DBDIR/*
    echo "Starting FS $wl"
    LD_LIBRARY_PATH=$LIB_PATH ./main --engine=fs --ycsb-workload=$WORKLOAD_BASE/$wl --db-directory=$FS_DBDIR $ARGS
done

