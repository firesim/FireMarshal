#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>

int try_malloc(uint64_t num_bytes){
  printf("Attempting malloc of 0x%lx bytes\n", num_bytes);
  void * mem = malloc(num_bytes);
  if (mem != NULL) {
    free(mem);
    printf("    PASSED!\n");
    return 0;
  } else {
    printf("    FAILED!\n");
    return -1;
  }
}

int simple_memtest(uint64_t * mem, uint64_t size, uint64_t error = 0){
    uint64_t step_size = 0x1000000;

    for (uint64_t j = 0; j < size / step_size; j++){
        printf("Writing bytes 0x%lx - 0x%lx\n", j * step_size * sizeof(uint64_t),
               (j + 1) * step_size * sizeof(uint64_t)-1);

        for (uint64_t i = 0; i < step_size; i++){
            mem[j*step_size + i] = j*step_size + i;
        }
    }

    mem[error] = 0;

    for (uint64_t j = 0; j < size / step_size; j++){
        printf("Reading bytes 0x%lx - 0x%lx\n", j * step_size * sizeof(uint64_t),
               (j + 1) * step_size * sizeof(uint64_t)-1);
        for (uint64_t i = 0; i < step_size; i++){
            uint64_t read_value = mem[j*step_size + i];
            if (read_value != j*step_size + i) {
                printf("Read error. Got 0x%lx, expected 0x%lx\n", read_value, j*step_size + i);
                return -1;
            }
        }
    }
    return 0;

}

int main(int argc, char ** argv){
    uint64_t overhead = 0x8000000L;
    assert(argc > 1);
    uint64_t mem_size = std::stoll(std::string(argv[1]));
    //uint64_t mem_size = 0x80000000L - overhead;
    try_malloc(mem_size);
    uint64_t * mem = (uint64_t*)malloc(mem_size);
    return(simple_memtest(mem, mem_size/sizeof(uint64_t)));
}
