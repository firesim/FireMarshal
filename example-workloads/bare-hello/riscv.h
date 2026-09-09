/**
 * @file rv_common.h
 * @brief RISC-V Definitions
 * 
 * This header file provides common definitions and operations for RISC-V core programming.
 * It includes memory register attributes, bit operation definitions, RISC-V specific definitions,
 * and common enumerations for state and status values.
 *
 * The memory register attributes define volatile permissions for read-only, write-only, and read/write access.
 * The bit operation definitions provide macros for setting, clearing, reading, and writing specific bits in a register.
 * The RISC-V specific definitions include macros for reading and writing control and status registers (CSRs),
 * as well as operations to swap, set, and clear specific bits in a CSR.
 * The common definitions include enumerations for state values (such as RESET and SET), and status values (such as OK and ERROR).
 *
 * @note This file should be included to access RISC-V core-specific definitions and perform common operations.
 *
 * @author -T.K.-
 * @date 2023-05-20
 */
 
 #ifndef __RV_H
 #define __RV_H
 
 #ifdef __riscv_xlen
   #define RISCV_XLEN __riscv_xlen
 #else
   #warning "__riscv_xlen not defined, defaulting to 64"
   #define RISCV_XLEN 64
 #endif
 
 #if RISCV_XLEN == 64
   #define LREG ld
   #define SREG sd
   #define REGBYTES 8
 #elif RISCV_XLEN == 32
   #define LREG lw
   #define SREG sw
   #define REGBYTES 4
 #else
   #error "Unsupported RISCV_XLEN"
 #endif
 
 
 /* ================ RISC-V specific definitions ================ */
 #define READ_CSR(REG) ({                          \
   unsigned long __tmp;                            \
   asm volatile ("csrr %0, " REG : "=r"(__tmp));  \
   __tmp; })
 
 #define WRITE_CSR(REG, VAL) ({                    \
   asm volatile ("csrw " REG ", %0" :: "rK"(VAL)); })
 
 #define SWAP_CSR(REG, VAL) ({                     \
   unsigned long __tmp;                            \
   asm volatile ("csrrw %0, " REG ", %1" : "=r"(__tmp) : "rK"(VAL)); \
   __tmp; })
 
 #define SET_CSR_BITS(REG, BIT) ({                 \
   unsigned long __tmp;                            \
   asm volatile ("csrrs %0, " REG ", %1" : "=r"(__tmp) : "rK"(BIT)); \
   __tmp; })
 
 #define CLEAR_CSR_BITS(REG, BIT) ({               \
   unsigned long __tmp;                            \
   asm volatile ("csrrc %0, " REG ", %1" : "=r"(__tmp) : "rK"(BIT)); \
   __tmp; })
 
 
 #endif /* __RV_H */
 