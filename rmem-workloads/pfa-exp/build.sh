#!/bin/bash

rootdir=overlay/root

make -C $rootdir/util
make -C $rootdir/benchmarks/genome
make -C $rootdir/benchmarks/qsort CROSS=riscv
make -C $rootdir/benchmarks/unit
