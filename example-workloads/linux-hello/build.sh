#!/bin/bash

set -ex

riscv64-unknown-linux-gnu-gcc -static hello.c -o hello.riscv -I$PWD
riscv64-unknown-linux-gnu-gcc -static goodbye.c -o goodbye.riscv -I$PWD

mkdir -p overlay/root
cp -f hello.riscv overlay/root/
cp -f goodbye.riscv overlay/root/
