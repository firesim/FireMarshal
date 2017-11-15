#!/bin/sh
pushd /root/genome
echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
/root/bin/pfa_launch ./assemble little.dat
popd
cat /sys/kernel/mm/pfa_stat
