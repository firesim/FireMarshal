#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

#define USAGE_STR "./unit [-s SIZE] [-n NPROC]\n" \
"  -s SIZE\n" \
"    Size of working set for each process (in bytes)\n" \
"\n" \
"  -n NPROC\n"\
"    Number of processes to fork\n" \
"\n" \
"  -l\n"\
"    Enable pagefault latency test (requires special kernel support)"

int main(int argc, char *argv[])
{
  size_t size = TEST_SIZE;
  uint64_t start, end;
  int nproc = 1;
  int c;
  bool pflat = false;

  while((c = getopt(argc, argv, "s:p:l")) != -1) {
    switch(c) {
      case 's':
        // Size
        size = strtoll(optarg, NULL, 10);
        break;
      case 'p':
        // Number of processes
        nproc = atoi(optarg);
        if(nproc < 1 || nproc > 100) {
          printf("Unreasonable number of processes requested: %d\n", nproc);
          return EXIT_FAILURE;
        }
        break;
      case 'l':
        //Latency test
        pflat = true;
        break;
      case '?':
        if(optopt == 's' || optopt == 'p') {
          printf("Option %c requires an argument\n", optopt);
        } else {
          printf("Unrecognized argument: %c\n", optopt);
        }
        printf("Usage: %s\n", USAGE_STR);
        return EXIT_FAILURE;
      default:
        printf("Unknown error while parsing arguments (arg was %x)\n", c);
        return EXIT_FAILURE;
    }
  }

  printf("Touching %.2lfM of stuff in %d processes\n", (double)size / 1000000.0, nproc);

  /* Note that the first process isn't forked at all so that we can test a case
   * with no forking */
  nproc--;
  while(nproc) {
    if(fork() == 0)
      break;
    nproc--;
  }

  printf("Pid %d starting\n", getpid());
  start = get_cycle();
  if(do_stuff(size, pflat) == EXIT_FAILURE) {
    printf("Test Failure (pid=%d)\n", getpid());
    return EXIT_FAILURE;
  } else {
    end = get_cycle();
    printf("Done wasting time and touchin' memory. Took %" PRId64 " cycles. (pid=%d)\n", end - start, getpid());
    return EXIT_SUCCESS;
  }
}
