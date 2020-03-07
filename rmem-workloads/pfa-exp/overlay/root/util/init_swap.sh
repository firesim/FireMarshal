#!/bin/sh
mkswap /dev/ram0
swapon /dev/ram0

# disable prefetching
echo 0 > /proc/sys/vm/page-cluster
