#!/bin/sh
pushd util
./init_cgrp.sh
./init_swap.sh
./run_cg.sh ./pfa_launch ../qsort/qsort 10000000
popd
