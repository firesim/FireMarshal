firesim-software
==================================

Builds a linux distro for firesim target nodes. Assumes priv 1.10.

# Prereqs:

    riscv-tools + riscv-gnu-toolchain for linux

# How to use:
You can build the PFA SW image for a number of different targets:
* spike: The PFA version of the Spike isa simulator (or the vanilla spike if
  you turn off PFA features)
* firesim: The RTL simulator, the difference between spike and firesim is mostly in how the disk is managed.
* initramfs: This version works anywhere, but it doesn't save state between runs.

To build, run:
    ./build.sh TARGET

e.g.
    ./build.sh initramfs

The default is 'firesim' to maintain compatibility with mainline firesim-software.

Running:
Spike with a disk:
  spike +disk=rootfs0.ext2 bbl-vmlinux

Spike with initramfs:
  spike bbl-vmlinux

# Configuring Linux
Default linux configurations are provided as, e.g., linux-config-spike. Note
that these replace the .config in riscv-linux/ every time you run ./build.sh so
you should not expect changes made by "make menuconfig" to be persisted. If you
would like to change these configurations, run "make ARCH=riscv menuconfig" in
riscv-linux/ and then copy the .config file to the root firesim-software
directory. You can also modify the configs directly.

## Changes from default
We are mostly compatible with the default configs used elsewhere (e.g. fedora
or mainline firesim-software), but there are a few changes:
* PFA Options: You can just use defaults here, or you can enable various features (see Kernel type -> PFA)
* Enable sysfs: PFA features are exposed through /sys/kernel/mm/pfa\_\* files
* Enable ramdisk: We use a ramdisk as swap backing (to trick the kernel). You should create at least one RAM disk with a few MB (I usually do 128MB)
* Enable frontswap: Used for some PFA interfaces and RSWAP baseline
