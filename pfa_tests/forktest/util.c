#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "util.h"

#define MAX(A, B) ((A > B) ? A : B)
#define MIN(A, B) ((A < B) ? A : B)

uint64_t big_rand(void)
{
  /* I don't trust rand() */
  static uint64_t r = 0xDEADBEEFCAFEBABE;
  r ^= (r << 21);
  r ^= (r >> 35);
  r ^= (r << 4);
  return r;
}

/* This test just walks randomly through memory of size "size" a few times */
void do_stuff(void)
{
  uint8_t *arr = malloc(TEST_SIZE);

  /* Force the array to be really allocated (no lazy allocation page-faults) */
  for(size_t i = 0; i < TEST_SIZE; i++)
  {
    arr[i] = i;
  }

  for(off_t i = 0; i < NTOUCH / CONTIGUITY; i++)
  {
    uint64_t idx = big_rand() % TEST_SIZE;
    off_t contig_end = MIN(TEST_SIZE - 1, idx + CONTIGUITY);
    for(int j = idx; j < contig_end; j++)
    {
      int write = rand();
      if((write % 100) < WRITE_RATIO) {
        /* Write! */
        arr[idx] = (uint8_t)write;
      } else {
        /* Read! */
        volatile uint8_t a = arr[idx];
      }
    }
  }

  return;
}

