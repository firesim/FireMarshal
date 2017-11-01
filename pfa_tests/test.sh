#!/bin/sh
pushd util
./init_cgrp.sh
./init_swap.sh
# ../qsort/qsort 10000000
./pfa_launch ../qsort/qsort 10000000
# ./run_cg.sh ./pfa_launch ../qsort/qsort 100000000
popd
