#!/bin/bash

set -ex

riscv64-unknown-linux-gnu-gcc -static hello.c -o hello.riscv
riscv64-unknown-linux-gnu-objdump -S hello.riscv &> hello.dump
mkdir -p overlay/root
cp -f hello.riscv overlay/root/
