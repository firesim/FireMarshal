#!/bin/sh
echo 1M > /sys/fs/cgroup/pfa_cg/memory.max
/root/bin/pfa_launch ./unit -p 2
cat /sys/kernel/mm/pfa_stat
