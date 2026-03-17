#ifndef TACIT_H
#define TACIT_H

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/types.h>

#define TACIT_COMM_LEN 16

struct tacit_log_record {
  uint32_t asid;
  pid_t pid;
  char comm[TACIT_COMM_LEN];
};

#define TACIT_LOG_RECORD_SIZE ((ssize_t)sizeof(struct tacit_log_record))

#define TRACE_IOC_MAGIC      't'
// --- IOCTL commands ---
// Enable the trace encoder
#define TRACE_IOC_ENABLE     _IO(TRACE_IOC_MAGIC, 0)
// Disable the trace encoder
#define TRACE_IOC_DISABLE    _IO(TRACE_IOC_MAGIC, 1)
// Set the trace target
#define TRACE_IOC_TARGET     _IOW(TRACE_IOC_MAGIC, 2, __u8)
// Read trace encoder stall count
#define TRACE_IOC_STALL_COUNT _IOR(TRACE_IOC_MAGIC, 3, __u64)

static inline int tacit_open(void) {
  const char *devpath = "/dev/tacit0";
  return open(devpath, O_RDWR | O_CLOEXEC);
}

static inline int tacit_enable(int fd) {
  return ioctl(fd, TRACE_IOC_ENABLE);
}

static inline int tacit_disable(int fd) {
  return ioctl(fd, TRACE_IOC_DISABLE);
}

static inline int tacit_target(int fd, __u8 target) {
  return ioctl(fd, TRACE_IOC_TARGET, target);
}

static inline int tacit_stall_count(int fd, uint64_t *count) {
  return ioctl(fd, TRACE_IOC_STALL_COUNT, count);
}

static inline int tacit_close(int fd) {
  return close(fd);
}

#endif
