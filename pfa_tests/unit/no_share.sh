#!/bin/sh
echo 1M > /sys/fs/cgroup/pfa_cg/memory.max
/root/bin/pfa_launch ./unit &
/root/bin/pfa_launch ./unit
cat /sys/kernel/mm/pfa_stat
