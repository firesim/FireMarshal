#!/bin/sh
reset_cg 500K
pushd /root/benchmarks/unit/
pfa_launch ./unit
popd
cat /sys/kernel/mm/pfa_stat
