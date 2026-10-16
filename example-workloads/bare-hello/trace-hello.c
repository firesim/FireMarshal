#include "tacit.h"
#include "rocketcore.h"
#include <stdio.h>
#include <inttypes.h>

static void bubble_sort(int *arr, size_t n) {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j + 1 < n - i; j++) {
      if (arr[j] > arr[j + 1]) {
        int tmp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tmp;
      }
    }
  }
}

static void init_descending(int *arr, size_t n, int bias) {
  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)(n - i) + bias;
  }
}

int main(int argc, char **argv) {

  #define TRACE_SINK_DMA_MAX_SIZE (1024) // very small for testing purposes
  #define DMA_ADDRESS 0x101c00000
  // __attribute__((aligned(64), section(".noinit"))) static volatile uint8_t dma_buffer[TRACE_SINK_DMA_MAX_SIZE];
  #define SORT_N (1024)
  #define SORT_ROUNDS (4)
  static int data[SORT_N];

  LTraceEncoderType *encoder = l_trace_encoder_get(get_hart_id());
  // l_trace_encoder_configure_branch_mode(encoder, BRANCH_MODE_PREDICT);
  l_trace_encoder_configure_branch_mode(encoder, BRANCH_MODE_TARGET);

  LTraceSinkDmaType *sink_dma = l_trace_sink_dma_get(get_hart_id());
  l_trace_sink_dma_configure_addr(sink_dma, DMA_ADDRESS, 0);
  l_trace_sink_dma_configure_max_size(sink_dma, TRACE_SINK_DMA_MAX_SIZE);
  l_trace_sink_dma_configure_mode(sink_dma, DMA_MODE_RING_BUFFER);
  // l_trace_sink_dma_configure_mode(sink_dma, DMA_MODE_OVERFLOW);
  l_trace_encoder_configure_target(encoder, TARGET_DMA);
  // l_trace_sink_dma_reset(sink_dma);

  l_trace_encoder_start(encoder);

  for (size_t round = 0; round < SORT_ROUNDS; round++) {
    init_descending(data, SORT_N, (int)round);
    bubble_sort(data, SORT_N);
  }

  l_trace_encoder_stop(encoder);

  printf("Bubble sort done on hart %lu (%u elems x %u rounds)\n",
         (unsigned long)get_hart_id(), (unsigned)SORT_N, (unsigned)SORT_ROUNDS);
  printf("Sorted sample: first=%d mid=%d last=%d\n",
         data[0], data[SORT_N / 2], data[SORT_N - 1]);

  // read the stall count
  uint64_t stall_count = l_trace_encoder_get_stall_count(encoder);
  printf("[l_trace_encoder_get_stall_count] stall_count: %lu\n", (unsigned long)stall_count);

  // l_trace_sink_dma_read(sink_dma, (uint8_t *)dma_buffer);
  // ltrace_sink_dma_flush(sink_dma);
}

void l_trace_sink_dma_read(LTraceSinkDmaType *sink_dma, volatile uint8_t *buffer) {
  uint64_t count = sink_dma->TR_SK_DMA_COUNT;
  printf("[l_trace_sink_dma_read] count: %" PRIu64 ", wrap_count: %u\n",
         count, sink_dma->TR_SK_DMA_WRAP_COUNT);

  // Keep output bounded so UART doesn't dominate sim time.
  const uint64_t max_dump = 1024;
  uint64_t dump_count = (count < max_dump) ? count : max_dump;
  if (count > max_dump) {
    printf("[l_trace_sink_dma_read] dumping first %" PRIu64 " bytes only\n", dump_count);
  }

  for (uint64_t i = 0; i < dump_count; i++) {
    if ((i & 0xF) == 0) {
      printf("\n%08" PRIx64 ": ", i);
    }
    printf("%02x ", (uint8_t)buffer[i]);
  }
  printf("\n\n");
}

