/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */

/* TODO: That should be done by autoconf - how? */
#include <inttypes.h>
#include <stdio.h>
#include <mpi.h>
#define UINT32_T uint32_t
#define UINT64_T uint64_t

extern double g_hrttimer_startvalue;

#define HRT_INIT(print, freq)  do {\
  int initialized; \
  if(print) printf("# initializing MPI_Wtime timer\n"); \
  MPI_Initialized(&initialized); \
  if(!initialized) MPI_Init(NULL,NULL); \
  g_hrttimer_startvalue = MPI_Wtime(); \
  freq = 1e9; \
} while(0)

#define HRT_TIMESTAMP_T double

#define HRT_GET_TIMESTAMP(t1) t1 = MPI_Wtime();

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr) *numptr = ( UINT64_T ) ( (t2-t1) * 1e9 );

#define HRT_GET_TIME(t1, time) time = ( UINT64_T ) ( (t1-g_hrttimer_startvalue) * 1e9 );
