#!/bin/bash

export MAKEFLAGS=-j16

# copy firesim-manager binaries
make -C fsim-rtc
mkdir -p ./buildroot-overlay/usr/bin
find fsim-rtc -executable -name "fsim*" | xargs -I{} cp {} ./buildroot-overlay/usr/bin

# overwrite buildroot's config with ours, then build rootfs
cp buildroot-config buildroot/.config
cd buildroot
make -j16
cd ..
cp buildroot/output/images/rootfs.ext2 rootfs0.ext2

# overwrite linux's config with ours, then build vmlinux image
cp linux-config riscv-linux/.config
cd riscv-linux
make -j16 ARCH=riscv vmlinux
cd ..

# build pk, provide vmlinux as payload
cd riscv-pk
mkdir build
cd build
../configure --host=riscv64-unknown-elf --with-payload=../../riscv-linux/vmlinux
make -j16
cp bbl ../../bbl-vmlinux

# make 7 more copies of the rootfs for f1.16xlarge nodes
cd ../../
for i in {1..7}
do
    cp rootfs0.ext2 rootfs$i.ext2
done
