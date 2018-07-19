#!/bin/sh
pushd unit
echo 500K > /sys/fs/cgroup/pfa_cg/memory.max
../util/run_cg.sh ./unit -l
popd
