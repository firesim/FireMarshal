#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

int main(int argc, char *argv[])
{
  printf("Fork 2 Touching %.2lfM of stuff\n", (double)TEST_SIZE / 1000000.0);
  do_stuff();
  printf("Fork 2 Done wasting time and touchin' memory\n");
  return EXIT_SUCCESS;
}
