#!/bin/bash

echo "Running chores"
mkdir -p /root/chores

# Get the driver map
cat /proc/modules | awk '{print $1 " " $6}' > /root/chores/driver_map.txt

# Mount the debugfs and get the jump label patch map
mount -t debugfs none /sys/kernel/debug 2>/dev/null || true
cat /sys/kernel/debug/jump_label_snapshot > /root/chores/jump_label_patch_map.txt
# show the line count of the jump label patch map
wc -l /root/chores/jump_label_patch_map.txt
umount /sys/kernel/debug

poweroff