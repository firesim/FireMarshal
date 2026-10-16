#include "tacit.h"
#include <stdio.h>
#include <spawn.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

extern char **environ;

#define WARMUP_ITERATIONS 870

static void drain_tacit_log(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags >= 0 && !(flags & O_NONBLOCK)) {
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
      perror("fcntl(O_NONBLOCK)");
      return;
    }
  }

  while (1) {
    struct tacit_log_record rec;
    ssize_t ret = read(fd, &rec, sizeof(rec));
    if (ret == (ssize_t)sizeof(rec)) {
      printf("tacit: asid=%u pid=%d comm=%.*s\n",
             rec.asid, rec.pid,
             TACIT_COMM_LEN, rec.comm);
      continue;
    }
    if (ret < 0) {
      if (errno == EAGAIN)
        break;
      perror("read");
      break;
    }
    if (ret == 0) {
      break;
    }
    fprintf(stderr, "short read from tacit log (%zd bytes)\n", ret);
    break;
  }
}

static inline int clock_gettime_syscall(clockid_t clk_id, struct timespec *ts) {
  return (int)syscall(SYS_clock_gettime, clk_id, ts);
}

int main(int argc, char **argv) {
  if (argc < 3) { fprintf(stderr, "usage: trace-submit <command> <iterations> [args...]\n"); return 2; }

  int iterations = atoi(argv[2]);
  if (iterations <= 0) {
    fprintf(stderr, "iterations must be greater than 0\n");
    return 2;
  }

  // launch ncecessary number of processes for warmup
  for (int i = 0; i < WARMUP_ITERATIONS; i++) {
    pid_t pid;
    int spawn_rc = posix_spawnp(&pid, argv[1], NULL, NULL, &argv[1], environ);
    if (spawn_rc != 0) {
      errno = spawn_rc;
      perror("posix_spawnp");
      return 1;
    }
    int status;
    if (waitpid(pid, &status, 0) < 0) {
      perror("waitpid");
      return 1;
    }
  }

  struct timespec* start_times = malloc(iterations * sizeof(struct timespec));
  struct timespec* end_times = malloc(iterations * sizeof(struct timespec));
  if (start_times == NULL || end_times == NULL) {
    fprintf(stderr, "failed to allocate memory for start and end times\n");
    return 1;
  }

  int fd = tacit_open();
  if (fd < 0) {
    fprintf(stderr, "failed to open /dev/tacit0\n");
    return 1;
  }

  if (tacit_target(fd, 2) < 0) {
    fprintf(stderr, "failed to set trace target to fsim\n");
    return 1;
  }

  if (tacit_enable(fd) < 0) {
      fprintf(stderr, "failed to enable tacit\n");
      return 1;
  }

  for (int i = 0; i < iterations; i++) {
    clock_gettime_syscall(CLOCK_MONOTONIC, &start_times[i]);
    pid_t pid;
    int spawn_rc = posix_spawnp(&pid, argv[1], NULL, NULL, &argv[1], environ);
    if (spawn_rc != 0) {
      errno = spawn_rc;
      perror("posix_spawnp");
      return 1;
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
      perror("waitpid");
    }
    clock_gettime_syscall(CLOCK_MONOTONIC, &end_times[i]);
  }
  
  if (tacit_disable(fd) < 0) {
    fprintf(stderr, "failed to disable tacit\n");
    return 1;
  }

  for (int i = 0; i < iterations; i++) {
    printf("iteration %d: start time = %ld ns, end time = %ld ns, duration = %ld ns\n",
           i, start_times[i].tv_nsec, end_times[i].tv_nsec, end_times[i].tv_nsec - start_times[i].tv_nsec);
  }

  drain_tacit_log(fd);
  if (tacit_close(fd) < 0) {
    fprintf(stderr, "failed to close /dev/tacit0\n");
    return 1;
  }
  return 0;
}
