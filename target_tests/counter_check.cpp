#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

constexpr int page_size = 4096 / sizeof(uint64_t);
constexpr int NUM_PAGES = 4096;
constexpr int TLB_REACH = page_size * 128;
constexpr int MAX_INDEX = NUM_PAGES * page_size ;
constexpr int LINE_SIZE = 8;
constexpr int LOOP_SIZE = 10000000;

#define read_csr_safe(reg) ({ register long __tmp asm("a0"); \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

long read_loads() { return read_csr_safe(hpmcounter3); }
long read_stores() { return read_csr_safe(hpmcounter4); }
long read_i_misses() { return read_csr_safe(hpmcounter5); }
long read_d_misses() { return read_csr_safe(hpmcounter6); }
long read_dtlb_misses() { return read_csr_safe(hpmcounter9); }

int main() {
    uint64_t * data = (uint64_t*) malloc(NUM_PAGES * 4096);

    printf("DCACHE store check: 100 hit rate\n");
    long stores = read_stores();
    long d_misses = read_d_misses();
    for (size_t i = 0; i < LOOP_SIZE; i++) {
        data[i%page_size] = (uint64_t) i;
    }
    stores = read_stores() - stores;
    d_misses = read_d_misses() - d_misses;
    printf("New stores: %ld, New D$ Misses:  %ld \n\n", stores, d_misses);


    printf("DCACHE store check: 100 miss rate\n");
    stores = read_stores();
    d_misses = read_d_misses();
    for (size_t i = 0; i < LOOP_SIZE; i++) {
        data[(i * LINE_SIZE * 2)%MAX_INDEX] = (uint64_t) i;
    }
    stores = read_stores() - stores;
    d_misses = read_d_misses() - d_misses;
    printf("New stores: %ld, New D$ Misses:  %ld \n\n", stores, d_misses);

    uint64_t count = 0;
    printf("DCACHE load check: 1.0 hit rate\n");
    long loads = read_loads();
    d_misses = read_d_misses();
    for (size_t i = 0; i < LOOP_SIZE; i++) {
        count += data[i%page_size];
    }
    loads = read_loads() - loads;
    d_misses = read_d_misses() - d_misses;
    printf("New loads: %ld, New D$ Misses:  %ld \n\n", loads, d_misses);

    printf("DCACHE load check: 1.0 miss rate\n");
    loads = read_loads();
    d_misses = read_d_misses();
    for (size_t i = 0; i < LOOP_SIZE; i++) {
        count += data[(i * LINE_SIZE * 2)%MAX_INDEX];
    }
    loads = read_loads() - loads;
    d_misses = read_d_misses() - d_misses;
    printf("New loads: %ld, New D$ Misses:  %ld \n\n", loads, d_misses);

    printf("DTLB check");
    long tlb = read_dtlb_misses();
    stores = read_stores();
    for (size_t i = 0; i < LOOP_SIZE; i++) {
        data[(i * page_size)%MAX_INDEX] = (uint64_t) i;
    }
    stores = read_stores() - stores;
    tlb = read_dtlb_misses() - tlb;
    printf("New stores: %ld, New DTLB Misses:  %ld \n\n", stores, tlb);

    printf("%ld", count);

    return 0;
}

