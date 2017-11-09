#!/bin/sh

echo $$ > /sys/fs/cgroup/pfa_cg/cgroup.procs
echo $$ > /sys/kernel/mm/pfa_stat
echo $$ > /sys/kernel/mm/pfa_tsk

exec $@
