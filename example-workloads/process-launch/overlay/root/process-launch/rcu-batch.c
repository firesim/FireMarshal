// Fix A from ANALYSIS.md: move the RCU grace-period kthread to SCHED_BATCH.
// BATCH tasks never wakeup-preempt (check_preempt_wakeup_fair), so the GP-end
// wake can no longer preempt ksoftirqd between rcu_do_batch() batches; the GP
// kthread instead runs right after the callback drain completes.
//
// Usage: rcu-batch [comm ...]   (defaults: rcu_sched rcu_preempt)
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#ifndef SCHED_BATCH
#define SCHED_BATCH 3
#endif

static int read_comm(const char* pid, char* buf, size_t len) {
  char path[300];
  snprintf(path, sizeof(path), "/proc/%s/comm", pid);
  FILE* f = fopen(path, "r");
  if (f == NULL) {
    return -1;
  }
  if (fgets(buf, len, f) == NULL) {
    fclose(f);
    return -1;
  }
  fclose(f);
  buf[strcspn(buf, "\n")] = '\0';
  return 0;
}

int main(int argc, char** argv) {
  const char* defaults[] = {"rcu_sched", "rcu_preempt"};
  const char** targets = defaults;
  int ntargets = 2;
  if (argc > 1) {
    targets = (const char**)&argv[1];
    ntargets = argc - 1;
  }

  DIR* d = opendir("/proc");
  if (d == NULL) {
    perror("opendir /proc");
    return 1;
  }

  int changed = 0;
  struct dirent* e;
  while ((e = readdir(d)) != NULL) {
    if (e->d_name[0] < '0' || e->d_name[0] > '9') {
      continue;
    }
    char comm[32];
    if (read_comm(e->d_name, comm, sizeof(comm)) != 0) {
      continue;
    }
    for (int i = 0; i < ntargets; i++) {
      if (strcmp(comm, targets[i]) != 0) {
        continue;
      }
      pid_t pid = (pid_t)atoi(e->d_name);
      struct sched_param sp = {.sched_priority = 0};
      if (sched_setscheduler(pid, SCHED_BATCH, &sp) != 0) {
        fprintf(stderr, "rcu-batch: sched_setscheduler(%d, %s): %s\n", pid,
                comm, strerror(errno));
        closedir(d);
        return 1;
      }
      printf("rcu-batch: pid %d (%s) -> SCHED_BATCH\n", pid, comm);
      changed++;
    }
  }
  closedir(d);

  if (changed == 0) {
    fprintf(stderr, "rcu-batch: no matching kthread found\n");
    return 1;
  }
  return 0;
}
