#!/bin/bash
riscv64-unknown-linux-gnu-gcc client.c -o overlay/root/client
riscv64-unknown-linux-gnu-gcc server.c -o overlay/root/server
