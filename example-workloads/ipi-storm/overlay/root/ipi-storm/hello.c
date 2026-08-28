#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sched.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdatomic.h>

#define PAGE_SIZE 4096
#define CHANGE_ITER 100

struct thread_ctx {
  int *shared_array;
  atomic_int done;
};

void* reader_thread(void* arg) {
  struct thread_ctx *ctx = (struct thread_ctx *)arg;
  int *shared_array = ctx->shared_array;
  volatile int read_count = 0;
  while (!atomic_load(&ctx->done)) {
    for (int i = 0; i < PAGE_SIZE / sizeof(int); i++) {
      read_count += shared_array[i];
    }
  }
  return NULL;
}

void* writer_thread(void* arg) {
  struct thread_ctx *ctx = (struct thread_ctx *)arg;
  int *shared_array = ctx->shared_array;
  for (int i = 0; i < CHANGE_ITER; i++) {
    mprotect(shared_array, PAGE_SIZE, PROT_READ | PROT_WRITE);
    mprotect(shared_array, PAGE_SIZE, PROT_READ);
  }
  atomic_store(&ctx->done, 1);
  return NULL;
}

int main(void) {

  int* shared_array = malloc(PAGE_SIZE);
  if (shared_array == NULL) {
    fprintf(stderr, "failed to allocate shared array\n");
    exit(1);
  }

  struct thread_ctx ctx = { .shared_array = shared_array, .done = 0 };

  cpu_set_t cpuset0, cpuset1;

  CPU_ZERO(&cpuset0);
  CPU_SET(0, &cpuset0); // Pin to core 0

  CPU_ZERO(&cpuset1);
  CPU_SET(1, &cpuset1); // Pin to core 1

  pthread_t thread0, thread1;
  pthread_attr_t attr0, attr1;

  pthread_attr_init(&attr0);
  pthread_attr_setaffinity_np(&attr0, sizeof(cpu_set_t), &cpuset0);
  pthread_create(&thread0, &attr0, reader_thread, &ctx);
  pthread_attr_destroy(&attr0);

  pthread_attr_init(&attr1);
  pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &cpuset1);
  pthread_create(&thread1, &attr1, writer_thread, &ctx);
  pthread_attr_destroy(&attr1);
  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
  return 0;
}
