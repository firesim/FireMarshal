#!/bin/bash
# Initialize all marshal-specific submodules. Skip submodules in
# example-workloads (if any). Users should always manually initialize workloads
# if they need them.
#
# You do not need to call this script if you only intend to build bare-metal workloads.
set -x

git submodule update --progress --filter=tree:0 --init \
  boards/default/linux \
  boards/default/firmware/opensbi \
  wlutil/busybox \
  boards/default/distros/br/buildroot \
  boards/firechip/drivers/*
