/**
 * @file rocketcore.h
 * @author -T.K.- / t_k_233@outlook.com
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */

 #ifndef __ROCKETCORE_H
 #define __ROCKETCORE_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdint.h>
 #include <stddef.h>
 
 #include "riscv.h"
 
 
 /* Core CSR Bit Field Definition */
 #define MIE_USIE_POS                  0x00U
 #define MIE_USIE_MSK                  (1U << MIE_USIE_POS)
 #define MIE_SSIE_POS                  0x01U
 #define MIE_SSIE_MSK                  (1U << MIE_SSIE_POS)
 #define MIE_VSSIE_POS                 0x02U
 #define MIE_VSSIE_MSK                 (1U << MIE_VSSIE_POS)
 #define MIE_MSIE_POS                  0x03U
 #define MIE_MSIE_MSK                  (1U << MIE_MSIE_POS)
 #define MIE_UTIE_POS                  0x04U
 #define MIE_UTIE_MSK                  (1U << MIE_UTIE_POS)
 #define MIE_STIE_POS                  0x05U
 #define MIE_STIE_MSK                  (1U << MIE_STIE_POS)
 #define MIE_VSTIE_POS                 0x06U
 #define MIE_VSTIE_MSK                 (1U << MIE_VSTIE_POS)
 #define MIE_MTIE_POS                  0x07U
 #define MIE_MTIE_MSK                  (1U << MIE_MTIE_POS)
 #define MIE_UEIE_POS                  0x08U
 #define MIE_UEIE_MSK                  (1U << MIE_UEIE_POS)
 #define MIE_SEIE_POS                  0x09U
 #define MIE_SEIE_MSK                  (1U << MIE_SEIE_POS)
 #define MIE_VSEIE_POS                 0x0AU
 #define MIE_VSEIE_MSK                 (1U << MIE_VSEIE_POS)
 #define MIE_MEIE_POS                  0x0BU
 #define MIE_MEIE_MSK                  (1U << MIE_MEIE_POS)
 #define MIE_SGEIE_POS                 0x0CU
 #define MIE_SGEIE_MSK                 (1U << MIE_SGEIE_POS)
 
 #define MIE_USIP_POS                  0x00U
 #define MIE_USIP_MSK                  (1U << MIE_USIP_POS)
 #define MIP_SSIP_POS                  0x01U
 #define MIP_SSIP_MSK                  (1U << MIP_SSIP_POS)
 #define MIP_VSSIP_POS                 0x02U
 #define MIP_VSSIP_MSK                 (1U << MIP_VSSIP_POS)
 #define MIP_MSIP_POS                  0x03U
 #define MIP_MSIP_MSK                  (1U << MIP_MSIP_POS)
 #define MIE_UTIP_POS                  0x04U
 #define MIE_UTIP_MSK                  (1U << MIE_UTIP_POS)
 #define MIP_STIP_POS                  0x05U
 #define MIP_STIP_MSK                  (1U << MIP_STIP_POS)
 #define MIP_VSTIP_POS                 0x06U
 #define MIP_VSTIP_MSK                 (1U << MIP_VSTIP_POS)
 #define MIP_MTIP_POS                  0x07U
 #define MIP_MTIP_MSK                  (1U << MIP_MTIP_POS)
 #define MIP_SEIP_POS                  0x09U
 #define MIP_SEIP_MSK                  (1U << MIP_SEIP_POS)
 #define MIP_VSEIP_POS                 0x0AU
 #define MIP_VSEIP_MSK                 (1U << MIP_VSEIP_POS)
 #define MIP_MEIP_POS                  0x0BU
 #define MIP_MEIP_MSK                  (1U << MIP_MEIP_POS)
 #define MIP_SGEIP_POS                 0x0CU
 #define MIP_SGEIP_MSK                 (1U << MIP_SGEIP_POS)
 
 static inline size_t get_hart_id() {
   return READ_CSR("mhartid");
 }
 
 static inline uint64_t get_cycles() {
   return READ_CSR("mcycle");
 }
 
 static inline void disable_global_interrupt() {
   CLEAR_CSR_BITS("mstatus", 1U << 3U);
 }
 
 static inline void enable_global_interrupt() {
   SET_CSR_BITS("mstatus", 1U << 3U);
 }
 
 static inline void disable_timer_interrupt() {
   CLEAR_CSR_BITS("mie", 1U << 7U);
 }
 
 static inline void enable_timer_interrupt() {
   SET_CSR_BITS("mie", 1U << 7U);
 }
 
 static inline void disable_irq(uint32_t IRQn) {
   CLEAR_CSR_BITS("mie", 1U << IRQn);
 }
 
 static inline void enable_irq(uint32_t IRQn) {
   SET_CSR_BITS("mie", 1U << IRQn);
 }
 
 static inline void clear_irq(uint32_t IRQn) {
   CLEAR_CSR_BITS("mip", 1U << IRQn);
 }
 
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* __ROCKETCORE_H */