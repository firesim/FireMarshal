#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stdbool.h>

/* What percent of touches should be writes? */
#define WRITE_RATIO 50

/* How many bytes to read contiguously at a time */
#define CONTIGUITY 1024 

/* Size of test (in bytes) */
#define TEST_SIZE (1024*500)

/* How many things should I touch? */
#define NTOUCH TEST_SIZE

#ifdef __amd64__
#include <x86intrin.h>
static inline uint64_t get_cycle(void)
{
  return __rdtsc();
}
#else
//I'm not sure what the equivalent for __amd64__ is for riscv...
static inline uint64_t get_cycle(void)
{
	uint64_t n;

	__asm__ __volatile__ (
		"rdcycle %0"
		: "=r" (n));
	return n;
}
#endif


/* This test just walks randomly through memory of size "size" a few times
 * sz: Size of test (in bytes)
 * pflat: enable page fault latency test
 *
 * returns: EXIT_SUCCESS or EXIT_FAILURE
 */
int do_stuff(size_t sz, bool pflat);

/* Begin recording evicted pages to be used by time_fault later. Only pages
 * after a call to this will be reported */
int start_pflat(void);

/* Take measurements surrounding a single page fault. Will intentionally fault
 * on the last page evicted since start_pflat was called. This is not super
 * reliable, sorry. */
int time_fault(void);
#endif
