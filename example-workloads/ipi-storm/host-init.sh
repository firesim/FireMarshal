#!/bin/bash

echo "Building ipi-storm workload"
cd overlay/root/ipi-storm
make hello
make trace-submit