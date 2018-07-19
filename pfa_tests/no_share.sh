#!/bin/sh
echo 10M > /sys/fs/cgroup/pfa_cg/memory.max
/root/bin/pfa_launch ./qsort/qsort 10000000 &
/root/bin/pfa_launch ./qsort/qsort 10000000 
cat /sys/kernel/mm/pfa_stat
