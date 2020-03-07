#!/bin/sh
pushd /root/benchmarks/genome
reset_cg 32M
pfa_launch ./assemble little.dat
popd
cat /sys/kernel/mm/pfa_stat_label
cat /sys/kernel/mm/pfa_stat
