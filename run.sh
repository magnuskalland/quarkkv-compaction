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
}

set -e
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
LIB_PATH=../Quark/quarkstore/quarklibio/build 

ENGINE=fs
DATA_DIR=~/ssd/quarkkv_data
WORK_DIR=~/ssd/quarkkv_work
MODE=load

mkdir -p $DATA_DIR $WORK_DIR

InstallQuark

clear

LD_LIBRARY_PATH=$LIB_PATH ./main    \
    --engine=$ENGINE                \
    --data-directory=$DATA_DIR      \
    --working-directory=$WORK_DIR   \
    --mode=$MODE

ret=0

set +e

exit $ret