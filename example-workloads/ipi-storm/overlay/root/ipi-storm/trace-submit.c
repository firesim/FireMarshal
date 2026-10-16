#include "tacit.h"
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>

// #define TARGET_DMA

static void drain_tacit_log(int fd, int index) {
  int flags = fcntl(fd, F_GETFL);
  if (flags >= 0 && !(flags & O_NONBLOCK)) {
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
      perror("fcntl(O_NONBLOCK)");
      return;
    }
  }
  printf("draining tacit log %d\n", index);

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

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: trace-submit <command> [args...]\n"); return 2; }

  int fd0 = tacit_open(0);
  int fd1 = tacit_open(1);
  if (fd0 < 0) {
    fprintf(stderr, "failed to open /dev/tacit0\n");
    return 1;
  }
  if (fd1 < 0) {
    fprintf(stderr, "failed to open /dev/tacit1\n");
    return 1;
  }

  #ifdef TARGET_DMA
  if (tacit_target(fd0, 1) < 0) {
    fprintf(stderr, "failed to set trace target to dma\n");
    return 1;
  }
  if (tacit_target(fd1, 1) < 0) {
    fprintf(stderr, "failed to set trace target to dma\n");
    return 1;
  }
  printf("trace target set to dma\n");
  #else
  if (tacit_target(fd0, 2) < 0) {
    fprintf(stderr, "failed to set trace target to fsim\n");
    return 1;
  }
  if (tacit_target(fd1, 2) < 0) {
    fprintf(stderr, "failed to set trace target to fsim\n");
    return 1;
  }
  printf("trace target set to fsim\n");
  #endif
  if (tacit_enable(fd0) < 0) {
      fprintf(stderr, "failed to enable tacit\n");
      return 1;
  }
  if (tacit_enable(fd1) < 0) {
    fprintf(stderr, "failed to enable tacit\n");
    return 1;
  }
  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "failed to fork\n");
    return 1;
  }
  if (pid == 0) {
    execvp(argv[1], &argv[1]);
    perror("execvp");
    return 127;
  }
  // parent
  int status;
  waitpid(pid, &status, 0);
  if (tacit_disable(fd0) < 0) {
    fprintf(stderr, "failed to disable tacit\n");
    return 1;
  }
  if (tacit_disable(fd1) < 0) {
    fprintf(stderr, "failed to disable tacit\n");
    return 1;
  }
  drain_tacit_log(fd0, 0);
  drain_tacit_log(fd1, 1);
  uint64_t count0;
  uint64_t count1;
  if (tacit_stall_count(fd0, &count0) < 0) {
    fprintf(stderr, "failed to get stall count\n");
    return 1;
  }
  if (tacit_stall_count(fd1, &count1) < 0) {
    fprintf(stderr, "failed to get stall count\n");
    return 1;
  }
  printf("stall count: %" PRIu64 " %" PRIu64 "\n", count0, count1);
  if (tacit_close(fd0) < 0) {
    fprintf(stderr, "failed to close /dev/tacit%d\n", 0);
    return 1;
  }
  if (tacit_close(fd1) < 0) {
    fprintf(stderr, "failed to close /dev/tacit%d\n", 1);
    return 1;
  }
  return 0;
}
