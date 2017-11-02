#!/bin/sh
pushd util
./init_swap.sh
./init_cgrp.sh
popd
