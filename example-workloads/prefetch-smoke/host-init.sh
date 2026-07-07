#!/bin/bash
set -e
echo "Building prefetch-smoke workload"
cd overlay/root/prefetch-smoke
make prefetch-smoke
