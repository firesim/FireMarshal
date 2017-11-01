#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

static inline uint64_t get_cycle(void)
{
  register unsigned long __v;
  __asm__ __volatile__ ("rdcycle %0" : "=r" (__v));
  return __v;
}

bool test_single(void);
bool test_multi(void); 

int main(int argc, char *argv[]) {
  bool res = false;

  if(argc == 1 || strcmp(argv[1], "single") == 0) {
    printf("Running test_single\n");
    res = test_single();
  } else if (strcmp(argv[1], "multi") == 0) {
    printf("Running test_multi\n");
    res = test_multi();
  } else {
    printf("Unrecognized test\n");
    res = false;
  }

  if(res) {
    printf("Test Success!\n");
  } else {
    printf("Test Failure!\n");
  }
  
	return EXIT_SUCCESS;
}

bool test_single(void) {
  uint64_t before = 0;
  uint64_t after = 0;
 /* Allocate a single page from the heap */
  uint8_t *pg;
  posix_memalign((void**)&pg, getpagesize(), getpagesize());
  memset(pg, 42, getpagesize());

  /* Timer Verification */
  before = get_cycle();
  after = get_cycle();
  printf("Timer verification: %ld\n", after - before);

  printf("user calling sys on vaddr %p\n", pg);
  syscall(SYS_pfa, pg);
  printf("user called sys\n");

  /* Try to access */
  uint8_t val;
  printf("Gonna poke it (cycle %ld)\n", get_cycle());
  before = get_cycle();
  val = pg[0];
  pg[0] = 84;
  after = get_cycle();

  printf("I poked it (cycle %ld)!\n", after);
  printf("Total Cycles: %ld\n", after - before);

  /* Drain the newpage q */
  syscall(SYS_pfa, 0);

  if(val == 42) {
    return true;
  } else {
    return false;
  }
}

/* Number of pages to use for test_multi */
#define TEST_MULTI_NPG 10

bool test_multi(void) {
  uint8_t *pgs;
  size_t sz = TEST_MULTI_NPG*getpagesize();
  pgs = aligned_alloc(getpagesize(), sz);
  memset(pgs, 42, sz);

  /* Evict the pages using the syscall (in a loop) */
  printf("Evicting pages\n");
  for(int i = 0; i < TEST_MULTI_NPG; i++) {
    int res = syscall(SYS_pfa, &(pgs[i*getpagesize()]));
    if(!res) {
      printf("syscall failure\n");
      return false;
    }
  }
  printf("Done evicting pages\n");

  /* Try to access */
  uint8_t val;
  printf("Gonna poke them\n");
  for(int i = 0; i < TEST_MULTI_NPG; i++) {
    val = pgs[i*getpagesize()];
    if(val != 42) {
      printf("Got wrong data back\n");
      return false;
    }
  }
  printf("I poked them  (%d)!\n", val);

  /* Drain the newpage q */
  syscall(SYS_PFA, 0);

  return true;
}
