#!/bin/sh
# echo 0 > /sys/fs/cgroup/pfa_cg/cgroup.procs
echo 0 > /sys/fs/cgroup/memory/pfa/tasks
exec $@
