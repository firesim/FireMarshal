#!/bin/sh
mkswap /dev/ram0
swapon /dev/ram0

# Alternative method:
# dd if=/dev/zero of=/imaswap bs=1M count=100
# mkswap imaswap
# losetup /dev/loop0 /imaswap
# swapon /dev/loop0

# Turn on frontswap (where we intercept swap operations)
# echo 1 > /sys/module/rswap/parameters/enabled
