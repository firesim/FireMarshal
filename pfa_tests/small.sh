#!/bin/sh
echo 500K > /sys/fs/cgroup/pfa_cg/memory.max
pushd /root/unit/
/root/bin/pfa_launch ./unit
popd
cat /sys/kernel/mm/pfa_stat
