#!/bin/sh
# Beware, this is very long and haiiiiry test fool

echo "Running unit tests"

echo "Running genome tests"
pushd /root/genome

LOG=/root/genome_log
echo -n "Benchmark,MemSize,TotalRuntime," >> $LOG
cat /sys/kernel/mm/pfa_stat_label >> $LOG

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "48M,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "32M,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 24M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "24M,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "16M,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

popd

echo "Running qsort tests"
pushd /root/qsort

LOG=/root/qsort_log
echo -n "Benchmark,MemSize,TotalRuntime," >> $LOG
cat /sys/kernel/mm/pfa_stat_label >> $LOG

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "48M,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "32M,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 24M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "24M,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "16M,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

popd

poweroff
