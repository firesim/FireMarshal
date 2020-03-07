/*
 * Copyright (c) 2009 The Trustees of Indiana University and Indiana
 *                    University Research and Technology
 *                    Corporation.  All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@cs.indiana.edu>
 *
 */

#include <math.h>

#ifndef HRT_ARCH
#error "HRT_ARCH macro not set or architecture not supported"
#else
#if HRT_ARCH==1
#include "x86_32-gcc-rdtsc.h"
#endif
#if HRT_ARCH==2
#include "x86_64-gcc-rdtsc.h"
#endif
#if HRT_ARCH==3
#include "ppc-gcc-tb.h"
#endif
#if HRT_ARCH==4
#include "ia64-gcc-itc.h"
#endif
#if HRT_ARCH==5
#include "mips64-sicortex-gcc.h"
#endif
#if HRT_ARCH==6
#include "mpi-wtime.h"
#endif
#if HRT_ARCH==7
#include "riscv.h"
#endif
#endif

/* global timer frequency in Hz */
extern unsigned long long g_timerfreq;

static double HRT_GET_USEC(unsigned long long ticks) {
  return 1e6*(double)ticks/(double)g_timerfreq;
}


static int sanity_check(int print) {
	
	HRT_TIMESTAMP_T t1, t2;
	uint64_t s, s2, s3;
	int sanity = 1;

	if(print) printf("# Sanity check of the timer\n");
	HRT_GET_TIMESTAMP(t1);
	sleep(1);
	HRT_GET_TIMESTAMP(t2);
	HRT_GET_ELAPSED_TICKS(t1, t2, &s)
  if(print) printf("# sleep(1) needed %"PRIu64" ticks.\n", s);
	HRT_GET_TIMESTAMP(t1);
	sleep(2);
	HRT_GET_TIMESTAMP(t2);
	HRT_GET_ELAPSED_TICKS(t1, t2, &s2)
	if(print) printf("# sleep(2)/2 needed %"PRIu64" ticks.\n", (s2/2));
	if (fabs((double)s - (double)s2/2) > (s*0.05)) {
		sanity = 0;
		printf("# The high performance timer gives bogus results on this system!\n");
	}
	s = s2/2;
	HRT_GET_TIMESTAMP(t1);
	sleep(3);
	HRT_GET_TIMESTAMP(t2);
	HRT_GET_ELAPSED_TICKS(t1, t2, &s3)
	if(print) printf("# sleep(3)/3 needed %"PRIu64" ticks.\n", s3/3);
	if (fabs((double)s - (double)s3/3) > (s*0.05)) {
		sanity = 0;
		printf("# The high performance timer gives bogus results on this system!\n");
	}
	return sanity;
}
