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

/* this is horrible, I know -- but there doesn't seem to be any other portable
 * way to do this :-(. __WORDSIZE is only defined on Linux ... in
 * bits/wordsize.h . Maybe there is something in autoconf ... */
#define BITS 64

#if defined(__xlc__) || defined( __xlC__) 
#define __volatile__ volatile
#define __asm__ asm
#define FENCE __fence();
#else 
#define FENCE 
#endif

#define HRT_INIT(print, freq) do {\
  if(print) printf("# initializing ppc timer (takes some seconds)\n"); \
  HRT_CALIBRATE(freq); \
} while(0) 


#if BITS == 64
typedef UINT64_T ppc_timeval_t;
#else
typedef struct {
	UINT32_T tbl;
	UINT32_T tbu0;
	UINT32_T tbu1;
} ppc_timeval_t;
#endif

#define HRT_TIMESTAMP_T ppc_timeval_t

#if BITS == 64 
#define HRT_GET_TIMESTAMP(t1) 	FENCE \
do { \
  __asm__ __volatile__ ("mftb %0" : "=r" (t1) ); \
} while(0); \
FENCE

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)	*numptr = t2-t1;

#define HRT_GET_TIME(t1,time) time = t1;

#else

#define HRT_GET_TIMESTAMP(t1) 	FENCE \
do { \
__asm__ __volatile__ ("mftbu %0" : "=r" (t1.tbu0)); \
__asm__ __volatile__ ("mftb  %0" : "=r" (t1.tbl)); \
__asm__ __volatile__ ("mftbu %0" : "=r" (t1.tbu1)); \
} while (t1.tbu0 != t1.tbu1); \
FENCE

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)	*numptr = (((( UINT64_T ) t2.tbu0) << 32) | t2.tbl) - \
														  (((( UINT64_T ) t1.tbu0) << 32) | t1.tbl);


#define HRT_GET_TIME(t1,time) time = (((( UINT64_T ) t1.tbu0) << 32) | t1.tbl)
#endif
