#!/bin/sh
echo 500K > /sys/fs/cgroup/pfa_cg/memory.max
pushd /root/forktest/
/root/bin/pfa_launch ./fork2
popd
cat /sys/kernel/mm/pfa_stat
