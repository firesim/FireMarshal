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
#include <unistd.h>
#include "calibrate.h"
#define UINT32_T uint32_t
#define UINT64_T uint64_t

#define HRT_INIT(print, freq) do {\
  if(print) printf("# initializing mips64 (sicortex) timer (takes some seconds)\n"); \
  HRT_CALIBRATE(freq); \
} while(0) 


#define HRT_TIMESTAMP_T UINT64_T

#define HRT_GET_TIMESTAMP(t1)  __asm__ __volatile__( \
                       ".set  push      \n"          \
                       ".set  mips32r2  \n"          \
                       "rdhwr $3, $30   \n"          \
                       ".set  pop       \n"          \
                       "move  %0, $3    \n"          \
                       : "=r"(t1) : : "$2", "$3"); 

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)	*numptr = t2 - t1;

#define HRT_GET_TIME(t1, time) time = t1;
