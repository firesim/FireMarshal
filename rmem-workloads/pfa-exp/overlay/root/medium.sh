#!/bin/sh
pushd /root/benchmarks/qsort
reset_cg 5M
pfa_launch ./qsort 10000000
popd
cat /sys/kernel/mm/pfa_stat
