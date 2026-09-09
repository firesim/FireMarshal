/**
 * @file metal.h
 * @brief Baremetal programming helpers
 *
 * This file provides some common definitions for baremetal programming.
 * It includes memory register attributes, bit operation definitions,
 * and common enumerations for state and status values that is used by
 * the Hardware Abstraction Layer (HAL) library.
 * 
 */
 
 #ifndef __METAL_H
 #define __METAL_H
 
 #include <stdint.h>
 #include <stddef.h>
 
 /* ================ Memory register attributes ================ */
 #ifdef __cplusplus
   #define   __I     volatile             /** Defines "read only" permissions */
 #else
   #define   __I     volatile const       /** Defines "read only" permissions */
 #endif
 #define     __O     volatile             /** Defines "write only" permissions */
 #define     __IO    volatile             /** Defines "read / write" permissions */
 
 /* following defines should be used for structure members */
 #define     __IM     volatile const      /** Defines "read only" structure member permissions */
 #define     __OM     volatile            /** Defines "write only" structure member permissions */
 #define     __IOM    volatile            /** Defines "read / write" structure member permissions */
 
 
 /* ================ Bit Operation definitions ================ */
 #define SET_BITS(REG, BIT)                    ((REG) |= (BIT))
 #define CLEAR_BITS(REG, BIT)                  ((REG) &= ~(BIT))
 #define READ_BITS(REG, BIT)                   ((REG) & (BIT))
 #define WRITE_BITS(REG, CLEARMASK, SETMASK)   ((REG) = (((REG) & (~(CLEARMASK))) | (SETMASK)))
 
 
 /* ================ Common definitions ================ */
 typedef enum {
   RESET = 0UL,
   SET   = !RESET,
 
   DISABLE = RESET,
   ENABLE  = SET,
   
   LOW   = RESET,
   HIGH  = SET,
 } State;
 
 typedef enum {
   OK = 0U,
   ERROR,
   BUSY,
   TIMEOUT
 } Status;
 
 #endif  /* __METAL_H */
 