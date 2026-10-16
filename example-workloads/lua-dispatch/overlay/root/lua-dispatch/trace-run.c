// Generic traced-run wrapper: enable tacit tracing (fsim bridge sink),
// exec the given command once, disable, print duration + asid->comm map.
#include "tacit.h"
#include <stdio.h>
#include <spawn.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>
#include <unistd.h>

extern char **environ;

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
             rec.asid, rec.pid, TACIT_COMM_LEN, rec.comm);
      continue;
    }
    if (ret < 0) {
      if (errno == EAGAIN) break;
      perror("read");
      break;
    }
    if (ret == 0) break;
    fprintf(stderr, "short read from tacit log (%zd bytes)\n", ret);
    break;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: trace-run <command> [args...]\n");
    return 2;
  }

  int fd = tacit_open();
  if (fd < 0) { fprintf(stderr, "failed to open /dev/tacit0\n"); return 1; }
  if (tacit_target(fd, 2) < 0) { fprintf(stderr, "failed to set trace target to fsim\n"); return 1; }
  if (tacit_enable(fd) < 0) { fprintf(stderr, "failed to enable tacit\n"); return 1; }

  struct timespec t0, t1;
  syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &t0);

  pid_t pid;
  int spawn_rc = posix_spawnp(&pid, argv[1], NULL, NULL, &argv[1], environ);
  if (spawn_rc != 0) { errno = spawn_rc; perror("posix_spawnp"); return 1; }
  int status;
  if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return 1; }

  syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &t1);

  if (tacit_disable(fd) < 0) { fprintf(stderr, "failed to disable tacit\n"); return 1; }

  double dur = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
  printf("trace-run: child exit status %d, duration %.3f s\n", WEXITSTATUS(status), dur);

  drain_tacit_log(fd);
  if (tacit_close(fd) < 0) { fprintf(stderr, "failed to close /dev/tacit0\n"); return 1; }
  return 0;
}
