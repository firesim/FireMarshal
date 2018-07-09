#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

#define USAGE_STR "./unit [-s SIZE] [-n NPROC]\n" \
"  -s SIZE\n" \
"    Size of working set for each process (in bytes)\n" \
"\n" \
"  -n NPROC\n"\
"    Number of processes to fork\n"

int main(int argc, char *argv[])
{
  size_t size = TEST_SIZE;
  int nproc = 1;
  int c;

  while((c = getopt(argc, argv, "s:p:")) != -1) {
    switch(c) {
      case 's':
        size = strtoll(optarg, NULL, 10);
        break;
      case 'p':
        nproc = atoi(optarg);
        if(nproc < 1 || nproc > 100) {
          printf("Unreasonable number of processes requested: %d\n", nproc);
          return EXIT_FAILURE;
        }
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
  if(do_stuff(size) == EXIT_FAILURE) {
    printf("Test Failure (pid=%d)\n", getpid());
    return EXIT_FAILURE;
  } else {
    printf("Done wasting time and touchin' memory (pid=%d)\n", getpid());
    return EXIT_SUCCESS;
  }
}
