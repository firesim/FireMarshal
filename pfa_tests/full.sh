#!/bin/sh
# Beware, this is very long and haiiiiry test fool

if [ $# -eq 1 ]; then
  CONFIG=$1
else
  CONFIG="UnknownConfig"
fi

echo "Running unit tests"
LOG=/root/full_log.csv
echo -n "Config,Benchmark,MemSize,TotalRuntime," >> $LOG
cat /sys/kernel/mm/pfa_stat_label >> $LOG

echo "Running qsort tests"
pushd /root/qsort

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Qsort,48000000," >> $LOG
mytime pfa_launch ./qsort 64000000 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Qsort,32000000," >> $LOG
mytime pfa_launch ./qsort 64000000 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Qsort,16000000," >> $LOG
mytime pfa_launch ./qsort 64000000 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

# echo 64M > /sys/fs/cgroup/pfa_cg/memory.max
# echo -n "$CONFIG,Qsort,64000000," >> $LOG
# mytime pfa_launch ./qsort 64000000 2>> $LOG
# cat /sys/kernel/mm/pfa_stat >> $LOG

popd

echo "Running genome tests"
pushd /root/genome

echo 48M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Genome,48000000," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

echo 32M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Genome,32000000," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

echo 16M > /sys/fs/cgroup/pfa_cg/memory.max
echo -n "$CONFIG,Genome,16000000," >> $LOG
mytime pfa_launch ./assemble little.dat 2>> $LOG
cat /sys/kernel/mm/pfa_stat >> $LOG
tail -n 1 $LOG

# echo 64M > /sys/fs/cgroup/pfa_cg/memory.max
# echo -n "$CONFIG,Genome,64000000," >> $LOG
# mytime pfa_launch ./assemble little.dat 2>> $LOG
# cat /sys/kernel/mm/pfa_stat >> $LOG

popd

echo "Full Results (also available in $LOG):"
cat $LOG

# poweroff
