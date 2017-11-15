#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"

/* Fork1 forks before working */
int main(int argc, char *argv[])
{
  if(fork() != 0){
    execlp("./fork2", "fork2", (char*)0);
  }
  printf("Fork1 Touching %.2lfM of stuff\n", (double)TEST_SIZE / 1000000.0);
  do_stuff();
  printf("Fork1 Done wasting time and touchin' memory\n");
  return EXIT_SUCCESS;
}
