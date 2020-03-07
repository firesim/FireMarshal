#!/bin/sh

# Handy to have this saved for reference later
cp /proc/config.gz .

cd /root/util/
./init_swap.sh
./init_cgrp.sh

# Install commands (to avoid annoying paths)
ln -sf /root/util/pfa_launch /bin/
ln -sf /root/util/reset_cg /bin/
ln -sf /root/util/run_cg /bin/
ln -sf /root/util/mytime /bin/
