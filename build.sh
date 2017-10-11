#!/bin/bash
set -e

LINUX_SRC=${PWD}/riscv-linux

cp buildroot-config buildroot/.config
pushd buildroot
# Note: Buildroot doesn't support parallel make
make -j1
popd
cp buildroot/output/images/rootfs.ext4 .

# cp linux-config riscv-linux/.config
pushd $LINUX_SRC
make -j16 ARCH=riscv vmlinux
popd

pushd riscv-pk
mkdir -p build
pushd build
../configure --host=riscv64-unknown-elf --with-payload=$LINUX_SRC/vmlinux
make -j16
cp bbl ../../bbl-vmlinux
popd
popd
