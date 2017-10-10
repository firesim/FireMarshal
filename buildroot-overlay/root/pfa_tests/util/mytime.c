#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdio.h>

#ifdef RISCV
static inline uint64_t get_cycle(void)
{
  register unsigned long __v;
  __asm__ __volatile__ ("rdcycle %0" : "=r" (__v));
  return __v;
}
#else
static inline uint64_t get_cycle(void)
{
  printf("get_cycle doesn't work on x86");
  return 0;
}
#endif

int main(int argc, char *argv[]) {

  uint64_t before, after;
  before = get_cycle();

  int p = fork();
  if(p == 0) {
    execvp(argv[1], &argv[1]);
  }

  int s;
  pid_t pid = wait(&s);
  after = get_cycle();
  printf("Time: %ld cycles\n", after - before);
}
