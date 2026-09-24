#!/bin/sh
# Enable the diagnosis-set tracepoints (rcu:*, sched switch/waking/wakeup,
# softirq entry/exit) via tracefs, then run the collect benchmark.
# Usage: tp-collect.sh <full|lite> <iterations>
#   lite = full minus the high-rate rcu_invoke_callback probe
MODE=$1
ITERS=$2

mount -t tracefs nodev /sys/kernel/tracing 2>/dev/null
cd /sys/kernel/tracing || { echo "tp-collect: no tracefs"; exit 1; }

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

echo "tp-collect: mode=$MODE, enabled events:"
cat set_event

echo 1 > tracing_on
/root/process-launch/submit /root/process-launch/dummy $ITERS
rc=$?
echo 0 > tracing_on

echo "=== ftrace stats (cpu0) ==="
cat per_cpu/cpu0/stats
exit $rc
