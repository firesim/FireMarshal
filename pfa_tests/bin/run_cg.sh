#!/bin/bash

ARCH=`uname -m`

if [ $ARCH = 'x86_64' ]; then
  CGROUP_TASK=/sys/fs/cgroup/memory/pfa/tasks
elif [ $ARCH = 'riscv64' ]; then 
  CGROUP_TASK=/sys/fs/cgroup/pfa_cg/cgroup.procs
else
  echo "Unrecognized architecture"
  exit 1
fi

echo 0 > $CGROUP_TASK
exec $@
