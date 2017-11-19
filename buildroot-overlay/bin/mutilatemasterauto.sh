#!/bin/bash

# pass in expected QPS as first arg
mutilate3 -s 192.168.0.2 --noload -T 4 -t 1 -v -i exponential:1 -a 192.168.0.3 -a 192.168.0.4 -a 192.168.0.5 -a 192.168.0.6 -a 192.168.0.7 -a 192.168.0.8 --scan=200:4000:100
