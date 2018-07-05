#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

int main(int argc, char *argv[])
{
  size_t size;
  if(argc == 2) {
    size = strtoll(argv[1], NULL, 10);
  } else {
    size = TEST_SIZE;
  }

  printf("Touching %.2lfM of stuff (pid=%d)\n", (double)size / 1000000.0, getpid());
  if(do_stuff(size) == EXIT_FAILURE) {
    printf("Test Failure (pid=%d)\n", getpid());
    return EXIT_FAILURE;
  } else {
    printf("Done wasting time and touchin' memory (pid=%d)\n", getpid());
    return EXIT_SUCCESS;
  }
}
