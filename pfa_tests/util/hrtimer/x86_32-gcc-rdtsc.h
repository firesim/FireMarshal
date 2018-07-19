/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include "calibrate.h"
#define UINT32_T uint32_t
#define UINT64_T uint64_t

#define HRT_INIT(print, freq) do {\
  if(print) printf("# initializing x86-32 timer (takes some seconds)\n"); \
  HRT_CALIBRATE(freq); \
} while(0) 

#define HRT_TIMESTAMP_T uint64_t

#define HRT_GET_TIMESTAMP(t1) __asm__ __volatile__("rdtsc": "=A" (t1));

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr) *numptr = t2 - t1;

#define HRT_GET_TIME(t1,time) time = t1
