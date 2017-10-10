#!/bin/sh
echo 0 > /sys/fs/cgroup/pfa_cg/cgroup.procs
/usr/bin/time -v $@
