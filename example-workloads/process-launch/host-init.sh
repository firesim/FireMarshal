#!/bin/bash

echo "Building process-launch workload"
cd overlay/root/process-launch
make dummy
make trace-submit
make submit
make rcu-batch