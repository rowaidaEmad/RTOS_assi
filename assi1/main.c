#include "tm4c123gh6pm.h"
#include <stdint.h> 
#include "bsp.h"

void main_blinky1(void);
void main_blinky2(void);
static uint32_t volatile l_tickCtr;
uint32_t tickCtr;
uint32_t start;
uint32_t ticks=BSP_TICKS_PER_SEC / 100U;
void main_blinky1() {
    while (1) {
        GPIO_PORTF_DATA_R = LED_RED;
        __asm("CPSID  I");
        start = l_tickCtr;
        __asm("CPSIE  I");
        while ((l_tickCtr - start) < ticks) {
        }
        GPIO_PORTF_DATA_R &= ~LED_RED;
                __asm("CPSID  I");
        start = l_tickCtr;
        __asm("CPSIE  I");
        while ((l_tickCtr - start) < ticks) {
        }
    }
}

void main_blinky2() {
    while (1) {
        GPIO_PORTF_DATA_R = LED_BLUE;
        __asm("CPSID  I");
        start = l_tickCtr;
        __asm("CPSIE  I");
        while ((l_tickCtr - start) < ticks) {
        }
        GPIO_PORTF_DATA_R &= ~LED_BLUE;
        __asm("CPSID  I");
        start = l_tickCtr;
        __asm("CPSIE  I");
        while ((l_tickCtr - start) < ticks) {
        }
    }
}
int main()
{
    SYSCTL_RCGCGPIO_R = 0x20U;
    GPIO_PORTF_DIR_R = 0x0EU;
    GPIO_PORTF_DEN_R = 0x0EU;
    NVIC_ST_RELOAD_R = 0xFFFFFF;   /* reload reg. with max value */
    NVIC_ST_CTRL_R = 7;         /* enable it, enable interrupt, use system clock */
    // Enable interrupts to the processor.
    __asm("CPSIE  I");
     uint32_t volatile run = 0U; 
		if (run) {
			main_blinky1();
		}
		else {
                        main_blinky2();
		}
        while (1) {
    }
}    

void SysTick_Handler(void) {
    
  ++l_tickCtr;
}