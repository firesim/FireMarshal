#!/bin/bash
# Initialize all marshal-specific submodules. Skip submodules in
# example-workloads (if any). Users should always manually initialize workloads
# if they need them.
#
# You do not need to call this script if you only intend to build bare-metal workloads.

git fetch --recurse-submodules -j5

git submodule update --init --recursive
