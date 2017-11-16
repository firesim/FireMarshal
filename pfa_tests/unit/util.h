#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

/* What percent of touches should be writes? */
#define WRITE_RATIO 50

/* How many bytes to read contiguously at a time */
#define CONTIGUITY 1024 

/* Size of test (in bytes) */
#define TEST_SIZE (1024*500)

/* How many things should I touch? */
#define NTOUCH TEST_SIZE

/* This test just walks randomly through memory of size "size" a few times */
int do_stuff(size_t);

#endif
