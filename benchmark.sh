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

WORKLOAD_BASE=ycsb/workloads
FS_DBDIR=./dbdir

mkdir -p $FS_DBDIR

workloads=("workloada8gb" "workloadb8gb" "workloadc8gb" "workloadd8gb" "workloadf8gb")

for wl in "${workloads[@]}"; do
    rm -rf $FS_DBDIR/*
    echo "Starting FS $wl"
    LD_LIBRARY_PATH=$LIB_PATH ./main --engine=fs --mode=ycsb --compaction-picker=all --ycsb-workload=$WORKLOAD_BASE/$wl
done

for wl in "${workloads[@]}"; do
    InstallQuark
    echo "Starting QuarkStore $wl"
    LD_LIBRARY_PATH=$LIB_PATH ./main --engine=quarkstore --mode=ycsb --compaction-picker=all --ycsb-workload=$WORKLOAD_BASE/$wl
done
