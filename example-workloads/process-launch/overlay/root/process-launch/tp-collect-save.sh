#!/bin/sh
# Same as tp-collect.sh, but persist the ftrace ring buffer into the rootfs
# afterwards so FireSim can copy it back (declare /root/ftrace-<mode>.txt.gz as
# a job output). Text format: `cat trace` (Perfetto-ingestible).
# Usage: tp-collect-save.sh <full|lite> <iterations>
MODE=$1
ITERS=$2

mount -t tracefs nodev /sys/kernel/tracing 2>/dev/null
cd /sys/kernel/tracing || { echo "tp-collect-save: no tracefs"; exit 1; }

echo 65536 > buffer_size_kb
echo 1 > events/rcu/enable
if [ "$MODE" = "lite" ]; then
  echo 0 > events/rcu/rcu_invoke_callback/enable
fi
echo 1 > events/sched/sched_switch/enable
echo 1 > events/sched/sched_waking/enable
echo 1 > events/sched/sched_wakeup/enable
echo 1 > events/irq/softirq_entry/enable
echo 1 > events/irq/softirq_exit/enable

echo "tp-collect-save: mode=$MODE iters=$ITERS, enabled events:"
cat set_event

echo 1 > tracing_on
/root/process-launch/submit /root/process-launch/dummy $ITERS
rc=$?
echo 0 > tracing_on

echo "=== ftrace stats (cpu0) ==="
cat per_cpu/cpu0/stats

echo "tp-collect-save: dumping ftrace records"
cat trace | gzip -1 > /root/ftrace-$MODE.txt.gz
sync
ls -la /root/ftrace-$MODE.txt.gz
exit $rc
