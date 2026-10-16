#!/bin/sh
# Enable diagnosis-set tracepoints, then run the TACIT-traced benchmark:
# the ATT trace captures the ftrace probe handlers themselves, letting the
# decoder attribute instrumentation overhead per probe invocation.
# Usage: tp-trace.sh <full|lite> <iterations>
MODE=${1:-full}
ITERS=${2:-256}

mount -t tracefs nodev /sys/kernel/tracing 2>/dev/null
cd /sys/kernel/tracing || { echo "tp-trace: no tracefs"; exit 1; }

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
echo "tp-trace: mode=$MODE iters=$ITERS"

# Enabling events patches jump labels at runtime; dump the post-enable
# snapshot so the decoder gets the correct patch state for THIS boot.
mount -t debugfs none /sys/kernel/debug 2>/dev/null
echo "=== JL SNAPSHOT START ==="
cat /sys/kernel/debug/jump_label_snapshot
echo "=== JL SNAPSHOT END ==="
umount /sys/kernel/debug 2>/dev/null

echo 1 > tracing_on
/root/process-launch/trace-submit /root/process-launch/dummy $ITERS
rc=$?
echo 0 > tracing_on

echo "=== ftrace stats (cpu0) ==="
cat per_cpu/cpu0/stats

# Persist the instrumentation's own records into the (persistent) rootfs so
# they can be extracted offline via debugfs and compared against the ATT view.
echo "tp-trace: saving ftrace records"
cat trace | gzip -1 > /root/ftrace-records.txt.gz
sync
ls -la /root/ftrace-records.txt.gz
exit $rc
