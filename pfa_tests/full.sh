#!/bin/sh
# Beware, this is very long and haiiiiry test fool

echo "Running unit tests"
LOG=/root/full_log.csv
echo -n "Benchmark,MemSize,TotalRuntime," >> $LOG
cat /sys/kernel/mm/pfa_stat_label >> $LOG

echo "Running genome tests"
pushd /root/genome

echo 64M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "64000000,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "48000000,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "32000000,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 24M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "24000000,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "16000000,Genome," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

popd

echo "Running qsort tests"
pushd /root/qsort

echo 64M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "64000000,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "48000000,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "32000000,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 24M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "24000000,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
mytime pfa_launch ./qsort 64000000 2>> $LOG
echo -n "16000000,Qsort," >> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG

popd

poweroff
