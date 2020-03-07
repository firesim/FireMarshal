/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "hrtimer.h"

#define NUMTESTS 5

int main() {

  static volatile HRT_TIMESTAMP_T t1, t2;
  static volatile UINT64_T elapsed_ticks, min = (UINT64_T)(~0x1);
  int notsmaller=0;

  while(notsmaller<3) {
    HRT_GET_TIMESTAMP(t1);
    sleep(1);
    HRT_GET_TIMESTAMP(t2);
    HRT_GET_ELAPSED_TICKS(t1, t2, &elapsed_ticks);

    notsmaller++;
    if(elapsed_ticks<min) {
      min = elapsed_ticks;
      notsmaller = 0;
    }
    fprintf(stderr,"."); fflush(stderr);
  }
  printf("%"PRIu64"\n", min);
  exit(0);
}
