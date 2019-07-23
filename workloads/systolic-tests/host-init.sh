#!/bin/bash

# This script will run on the host from the workload directory
# (e.g. workloads/example-fed) every time the workload is built.
# It is recommended to call into something like a makefile because
# this script may be called multiple times.
echo "Building systolic-rocc-tests benchmark"
cd overlay/root/systolic-rocc-tests
autoconf
mkdir -p build && cd build
../configure --with-riscvtools=$RISCV
make TARGET=riscv64-unknown-linux-gnu-
