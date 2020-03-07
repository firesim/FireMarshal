#!/bin/sh
mount -t cgroup2 none /sys/fs/cgroup
echo "+memory" > /sys/fs/cgroup/cgroup.subtree_control
