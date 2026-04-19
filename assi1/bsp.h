//
#ifndef __BSP_H__
#define __BSP_H__

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

/* system clock tick [Hz] */
#define BSP_TICKS_PER_SEC 100U

/* delay for a specified number of system clock ticks (polling) */
//void BSP_delay(uint32_t ticks);

/* get the current value of the clock tick counter (returns immedately) */
//uint32_t BSP_tickCtr(void);

#endif // __BSP_H__