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

if [[ "$DIR" == "quarkstore" ]]; then
    InstallQuark
else
    mkdir -p $DIR
    rm -rf $DIR/*
fi

LD_LIBRARY_PATH=$LIB_PATH ./main \
    --engine=$ENGINE \
    --db-directory=$DIR \
    --prepopulate-size=65536

ret=0

rm -rf $DIR

set +e

exit $ret
