#!/bin/sh
mount -t cgroup2 none /sys/fs/cgroup
echo "+memory" > /sys/fs/cgroup/cgroup.subtree_control
mkdir /sys/fs/cgroup/pfa_cg
# to set limits automatically, modify here or repeat these commands manually
echo 5M > /sys/fs/cgroup/pfa_cg/memory.max 
