/* Board Support Package (BSP) for the EK-TM4C123GXL board */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */

#include "bsp.h"
//#include "miros.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

//static uint32_t volatile l_tickCtr;

//void BSP_delay(uint32_t ticks) {
 //   uint32_t start = BSP_tickCtr();
 //   while ((BSP_tickCtr() - start) < ticks) {
//    }
//}
//void SysTick_Handler(void) {
//    ++l_tickCtr;
//}

//uint32_t BSP_tickCtr(void) {
 //   uint32_t tickCtr;

//   __asm("CPSID  I");
//    tickCtr = l_tickCtr;
//   __asm("CPSIE  I");

 //   return tickCtr;
//}