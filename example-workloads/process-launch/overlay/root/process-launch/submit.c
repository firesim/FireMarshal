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

  // launch once for warmup
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

  struct timespec* start_times = malloc(iterations * sizeof(struct timespec));
  struct timespec* end_times = malloc(iterations * sizeof(struct timespec));
  if (start_times == NULL || end_times == NULL) {
    fprintf(stderr, "failed to allocate memory for start and end times\n");
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

  printf("duration recorded (ns):\n");
  for (int i = 0; i < iterations; i++) {
    printf("%i: %ld\n",
           i, end_times[i].tv_nsec - start_times[i].tv_nsec);
  }

  return 0;
}
