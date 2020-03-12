#!/bin/bash

rootdir=overlay/root

build_benchmark () {
    make -C $rootdir/benchmarks/$1 clean
    make -C $rootdir/benchmarks/$1 CROSS=riscv
}

make -C $rootdir/util clean
make -C $rootdir/util

build_benchmark genome
build_benchmark qsort
build_benchmark unit
