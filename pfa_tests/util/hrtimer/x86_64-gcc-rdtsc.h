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
  if(print) printf("# initializing x86-64 timer (takes some seconds)\n"); \
  HRT_CALIBRATE(freq); \
} while(0) 


typedef struct {
	UINT32_T l;
	UINT32_T h;
} x86_64_timeval_t;

#define HRT_TIMESTAMP_T x86_64_timeval_t

/* TODO: Do we need a while loop here? aka Is rdtsc atomic? - check in the documentation */	
#define HRT_GET_TIMESTAMP(t1)  __asm__ __volatile__ ("rdtsc" : "=a" (t1.l), "=d" (t1.h));

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)	*numptr = (((( UINT64_T ) t2.h) << 32) | t2.l) - \
														  (((( UINT64_T ) t1.h) << 32) | t1.l);

#define HRT_GET_TIME(t1, time) time = (((( UINT64_T ) t1.h) << 32) | t1.l)
