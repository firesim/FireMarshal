#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
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

int start_pflat(void)
{
  /* Reset the pf latency system in the kernel */
  FILE *lat_file = fopen("/sys/kernel/mm/pfa_pflat", "w");
  if(!lat_file) {
    printf("Failed to open pfa_pflat sysfs file\n");
    return 0;
  }
  fprintf(lat_file, "0");
  fclose(lat_file);
  return 1;
}

int time_fault(void)
{
  size_t sz = 0;
  /* Time at which the Linux PF handler started */
  int64_t pf_start_time = 0;
  uintptr_t vaddr = 0;
  uint8_t val;
  FILE *lat_file;
 
  /* Get the vaddr of an evicted page from the kernel. */
  lat_file = fopen("/sys/kernel/mm/pfa_pflat", "r");
  if(!lat_file) {
    printf("Failed to open pfa_pflat sysfs file\n");
    return 0;
  }
  sz = fscanf(lat_file, "0x%lx\n", &vaddr);
  fclose(lat_file);
  if(sz != 1 || vaddr == 0) {
    printf("No pages were evicted, skipping latency test (sz=%lu, vaddr=%lx)\n", sz, vaddr);
    fclose(lat_file);
    return 0;
  }
  printf("Gonna probe vaddr: 0x%"PRIx64"\n", vaddr);

  /* Fault on the vaddr */
  int64_t start = get_cycle();
  val = *(volatile uint64_t *)vaddr;
  int64_t end = get_cycle();

  /* Read the start time of the handler from the kernel (this will give us the
   * trap latency) */
  lat_file = fopen("/sys/kernel/mm/pfa_pflat", "r");
  sz = fscanf(lat_file, "%ld\n", &pf_start_time);
  fclose(lat_file);
  printf("Trap started at: %ld\n", pf_start_time);

  printf("Faulted once: \n");
  printf("Start: %ld\tEnd: %lu\n", start, end);
  printf("Trap took %ld cycles\n", pf_start_time - start);
  printf("End-To-End Took %ld cycles\n", end - start);
  return 1;
}

/* This test just walks randomly through memory of size "size" a few times */
int do_stuff(size_t size, bool pflat)
{
  /* Start tracking evictions in the kernel */
  if(pflat) {
    if(!start_pflat())
      return 0;
  }

  uint8_t *arr = malloc(size);
  printf("Initializing memory (pid=%d)\n", getpid());
  /* Force the array to be really allocated (no lazy allocation page-faults) */
  for(size_t i = 0; i < size; i++)
  {
    arr[i] = i;
  }

  /* Optional latency test */
  if(pflat) {
    printf("Measuring page-fault latency\n");
    if(!time_fault()) {
      printf("Page Fault Latency test failed, exiting\n");
      return EXIT_FAILURE;
    }
  }

  /* Walk through the array randomly touching/reading in CONTIGUITY sized groups */
  printf("Done initializing memory. Starting the touchy touchy. (pid=%d)\n", getpid());
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

  printf("Done with touchy touch. Checking array. (pid=%d)\n", getpid());
  /* Check that the array is still good */
  for(size_t i = 0; i < size; i++)
  {
    if(arr[i] != i % 256) {
      printf("Array Corrupted Son! (watchudid?!) (pid=%d)\n", getpid());
      /* size_t end = MIN(size, i+8192); */
      /* for(; i < end; i++) { */
      /*   printf("arr[%d] = %d!\n", i, arr[i]); */
      /* } */
      return EXIT_FAILURE;
    }
  }
  printf("Array looks reeeeeal gud (pid=%d)\n", getpid());

  return EXIT_SUCCESS;
}

