#!/bin/bash

# pass in expected QPS as first arg
mutilate3 -s 192.168.0.2 --noload -T 4 -Q 1000 -D 4 -C 4 -t 1 -c 4 -i exponential:1 -a 192.168.0.3 -a 192.168.0.4 -a 192.168.0.5 -a 192.168.0.6 -a 192.168.0.7 -a 192.168.0.8 --scan=1100:141100:10000
