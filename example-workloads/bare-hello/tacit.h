#ifndef __L_TRACE_ENCODER_H
#define __L_TRACE_ENCODER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __I volatile const
#define __IO volatile

static inline void set_bits_u32(volatile uint32_t *reg, uint32_t bits) {
  *reg |= bits;
}

static inline void clear_bits_u32(volatile uint32_t *reg, uint32_t bits) {
  *reg &= ~bits;
}

typedef struct {
  __IO uint32_t TR_TE_CTRL; //0x00
  __I uint32_t TR_TE_INFO; //0x04
  __IO uint32_t TR_TE_BUBBLE[6]; //0x08-0x1C
  __IO uint32_t TR_TE_TARGET; //0x20
  __IO uint32_t TR_TE_BRANCH_MODE; //0x24
  __IO uint64_t TR_TE_STALL_COUNT; //0x28
} LTraceEncoderType;

typedef struct {
  __IO uint64_t TR_SK_DMA_ADDR; //0x00
  __IO uint64_t TR_SK_DMA_COUNT; //0x08
  __IO uint64_t TR_SK_DMA_MAX_SIZE; //0x10
  __IO uint32_t TR_SK_DMA_RESET; //0x18
  __IO uint32_t TR_SK_DMA_MODE; //0x1C
  __IO uint32_t TR_SK_DMA_WRAP_COUNT; //0x20
} LTraceSinkDmaType;

// Trace Sink Targets
#define TARGET_PRINT 0x0
#define TARGET_DMA 0x1
#define L_TRACE_ENCODER_BASE_ADDRESS 0x3000000

// Trace Branch Mode
#define BRANCH_MODE_TARGET    0x0
#define BRANCH_MODE_RESERVED0 0x1
#define BRANCH_MODE_PREDICT   0x2
#define BRANCH_MODE_RESERVED1 0x3

// Trace Sink DMA Mode
#define DMA_MODE_OVERFLOW 0x0
#define DMA_MODE_RING_BUFFER 0x1

// SBUS Bypass 
#define SBUS_BYPASS_ADDRESS 0x1000000000ULL

#define L_TRACE_ENCODER0 ((LTraceEncoderType *)(L_TRACE_ENCODER_BASE_ADDRESS + 0x0000))
#define L_TRACE_ENCODER1 ((LTraceEncoderType *)(L_TRACE_ENCODER_BASE_ADDRESS + 0x1000))
#define L_TRACE_ENCODER2 ((LTraceEncoderType *)(L_TRACE_ENCODER_BASE_ADDRESS + 0x2000))
#define L_TRACE_ENCODER3 ((LTraceEncoderType *)(L_TRACE_ENCODER_BASE_ADDRESS + 0x3000))

#define L_TRACE_SINK_DMA_BASE_ADDRESS 0x3010000
#define L_TRACE_SINK_DMA0 ((LTraceSinkDmaType *)(L_TRACE_SINK_DMA_BASE_ADDRESS + 0x0000))
#define L_TRACE_SINK_DMA1 ((LTraceSinkDmaType *)(L_TRACE_SINK_DMA_BASE_ADDRESS + 0x1000))
#define L_TRACE_SINK_DMA2 ((LTraceSinkDmaType *)(L_TRACE_SINK_DMA_BASE_ADDRESS + 0x2000))
#define L_TRACE_SINK_DMA3 ((LTraceSinkDmaType *)(L_TRACE_SINK_DMA_BASE_ADDRESS + 0x3000))

static inline LTraceEncoderType *l_trace_encoder_get(uint32_t hart_id) {
  return (LTraceEncoderType *)(L_TRACE_ENCODER_BASE_ADDRESS + hart_id * 0x1000);
}

static inline LTraceSinkDmaType *l_trace_sink_dma_get(uint32_t hart_id) {
  return (LTraceSinkDmaType *)(L_TRACE_SINK_DMA_BASE_ADDRESS + hart_id * 0x1000);
}

static inline void l_trace_encoder_start(LTraceEncoderType *encoder) {
  set_bits_u32(&encoder->TR_TE_CTRL, 0x1U << 1);
}

static inline void l_trace_encoder_stop(LTraceEncoderType *encoder) {
  clear_bits_u32(&encoder->TR_TE_CTRL, 0x1U << 1);
}

static inline uint64_t l_trace_encoder_get_stall_count(LTraceEncoderType *encoder) {
  return encoder->TR_TE_STALL_COUNT;
}

static inline void l_trace_encoder_configure_target(LTraceEncoderType *encoder, uint64_t target) {
  encoder->TR_TE_TARGET = target;
}

static inline void l_trace_encoder_configure_branch_mode(LTraceEncoderType *encoder, uint64_t branch_mode) {
  encoder->TR_TE_BRANCH_MODE = branch_mode;
}

static inline void l_trace_sink_dma_configure_addr(LTraceSinkDmaType *sink_dma, uint64_t dma_addr, int bypass) {
  sink_dma->TR_SK_DMA_ADDR = bypass ? (SBUS_BYPASS_ADDRESS|dma_addr) : dma_addr;
}

static inline void l_trace_sink_dma_configure_max_size(LTraceSinkDmaType *sink_dma, uint64_t max_size) {
  sink_dma->TR_SK_DMA_MAX_SIZE = max_size;
}

static inline void l_trace_sink_dma_configure_mode(LTraceSinkDmaType *sink_dma, uint64_t mode) {
  sink_dma->TR_SK_DMA_MODE = mode;
}

static inline void l_trace_sink_dma_reset(LTraceSinkDmaType *sink_dma) {
  set_bits_u32(&sink_dma->TR_SK_DMA_RESET, 0x1U);
}

void l_trace_sink_dma_read(LTraceSinkDmaType *sink_dma, volatile uint8_t *buffer);

#ifdef __cplusplus
}
#endif
#endif /* __L_TRACE_ENCODER_H */
