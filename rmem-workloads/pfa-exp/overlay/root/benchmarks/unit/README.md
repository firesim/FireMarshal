This is a unit test for the PFA and paging in general. It runs a handful of little tests and can trigger basic swapping.

# Basic Swapping
The basic test will allocate an array and iterate through it with a mix of reads and writes. If you run this binary under a cgroup with constrained memory, you will trigger paging. See './unit -?' for default memory size and how to change it.

# Concurrency
## Multi-Threading
Not currently supported.

## Multiple Processes
Run unit with the '-p' option to fork multiple processes. These processes will
not share any core data, but they will share a small amount of memory from
forking in general.

If you want to test without any sharing at all, you will need to launch
multiple copies of unit using a shell script that adds itself to the cgroup
(and PFA if enabled) and then 'exec's unit directly (no forking). The run\_cg.sh
and pfa\_launch scripts in ../util should do the trick.

# Page Fault Latency
To measure page fault latency directly, you will need special support in the
kernel (provided by the PFA\_PFLAT flag in the kernel config). You may then pass
the "-l" option to measure trap time and a single page fault latency. This
support is disabled by default in the kernel (it can be slow).

See riscv-linux/PFA\_README.md for details on how this works.
