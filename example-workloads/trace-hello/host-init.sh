#!/bin/bash

echo "Building trace-hello workload"
cd overlay/root/trace-hello
make hello
make trace-submit