#!/bin/sh
#Disable swap prefetching (not compatible with pfa).
#Consider re-enabling for baseline experiments
echo 0 > /proc/sys/vm/page-cluster

# Even though we swap to PFA, linux needs at least one "real" swap device for
# silly implementation reasons. This will never actually get used.
mkswap /dev/ram0
swapon /dev/ram0
