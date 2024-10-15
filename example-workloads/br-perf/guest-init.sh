#!/bin/bash
set -ex

cp -r /root/lib64/* /lib64
poweroff -f
