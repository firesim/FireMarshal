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
int do_stuff(size_t size)
{
  uint8_t *arr = malloc(size);

  /* Force the array to be really allocated (no lazy allocation page-faults) */
  for(size_t i = 0; i < size; i++)
  {
    arr[i] = i;
  }

  /* Walk through the array randomly touching/reading in CONTIGUITY sized groups */
  for(off_t i = 0; i < size / CONTIGUITY; i++)
  {
    uint64_t idx = big_rand() % size;
    off_t contig_end = MIN(size - 1, idx + CONTIGUITY);
    for(int j = idx; j < contig_end; j++)
    {
      int write = rand();
      if((write % 100) < WRITE_RATIO) {
        /* Write! */
        arr[idx] = idx;
      } else {
        /* Read! */
        volatile uint8_t a = arr[idx];
      }
    }
  }

  /* Check that the array is still good */
  for(size_t i = 0; i < size; i++)
  {
    if(arr[i] != i % 256) {
      printf("Array Corrupted\n");
      size_t end = MIN(size, i+8192);
      for(; i < end; i++) {
        printf("arr[%d] = %d!\n", i, arr[i]);
      }
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

