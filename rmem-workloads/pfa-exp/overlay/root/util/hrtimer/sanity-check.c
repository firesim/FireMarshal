/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "hrtimer.h"

int main(int argc, char **argv) {
	
  int sane = sanity_check(1);

  if(sane) exit(EXIT_SUCCESS);
  else exit(EXIT_FAILURE);
}
