#!/bin/bash
sudo cgdelete memory:pfa
sudo cgcreate -t xarc:xarc -a xarc:xarc -g memory:pfa
echo "1M" > /sys/fs/cgroup/memory/pfa/memory.limit_in_bytes
