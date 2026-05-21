#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Counting Semaphore
SemaphoreHandle_t xCountingSemaphore = NULL;

// Global counters to track the exact sequence of events
volatile uint32_t InterruptsFired = 0;
volatile uint32_t TokensProcessed = 0;

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for simulation)
 * ------------------------------------------------------------------------- */
void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    // Configure LEDs (PF1, PF2, PF3) as Output
    GPIO_PORTF_DIR_R |= 0x0E;  
    GPIO_PORTF_DEN_R |= 0x0E;  
    
    // Unmask the interrupt at the peripheral level
    GPIO_PORTF_IM_R |= 0x10;   

    // Set NVIC Priority for Port F (Interrupt 30) to 5
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; 
    NVIC_EN0_R = (1 << 30); 
}

/* -------------------------------------------------------------------------
 * The ISR (Simulated Hardware Interrupt)
 * ------------------------------------------------------------------------- */
void GPIOF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Clear the peripheral hardware flag
    GPIO_PORTF_ICR_R = 0x10;

    // Track the actual number of times the CPU entered the ISR
    InterruptsFired++;

    // Give a token to the counting semaphore (increments count by 1)
    if (xCountingSemaphore != NULL) {
        xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* -------------------------------------------------------------------------
 * Task 1: The Trigger Task (Priority 2 - High)
 * ------------------------------------------------------------------------- */
void vTriggerTask(void *pvParameters) {
    for (;;) {
        // Wait 3 seconds before starting the burst
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Burst: Fire the interrupt 3 times with a short 50ms gap.
        // We use the Software Trigger Interrupt Register (STIR) so the Keil 
        // simulator correctly jumps to the ISR.
        
        NVIC_SW_TRIG_R = 30; // 1st Interrupt 
        vTaskDelay(pdMS_TO_TICKS(50));
        
        NVIC_SW_TRIG_R = 30; // 2nd Interrupt 
        vTaskDelay(pdMS_TO_TICKS(50));
        
        NVIC_SW_TRIG_R = 30; // 3rd Interrupt 
    }
}

/* -------------------------------------------------------------------------
 * Task 2: The Handler Task (Priority 1 - Low)
 * ------------------------------------------------------------------------- */
void vHandlerTask(void *pvParameters) {
    for (;;) {
        // Block until an interrupt gives a token to the counting semaphore
        if (xSemaphoreTake(xCountingSemaphore, portMAX_DELAY) == pdTRUE) {
            
            // Turn on Blue LED to indicate we are processing a token
            GPIO_PORTF_DATA_R |= 0x04; 
            
            // Track how many queued events we have processed
            TokensProcessed++;
            
            // SIMULATE HEAVY PROCESSING
            // Because this is a busy-wait loop instead of vTaskDelay, FreeRTOS 
            // cannot run this task's xSemaphoreTake again until this loop finishes.
            // The Trigger task will interrupt this loop multiple times!
            for(volatile int i = 0; i < 2000000; i++) {}
            
            // Turn off Blue LED
            GPIO_PORTF_DATA_R &= ~0x04; 
        }
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();

    // Initialize the Counting Semaphore
    // Parameter 1: Maximum count (We expect 3 rapid interrupts, so 5 is safe)
    // Parameter 2: Initial count (Starts at 0, ISR will add tokens)
    xCountingSemaphore = xSemaphoreCreateCounting(5, 0);

    if (xCountingSemaphore != NULL) {
        // Priority 2 ensures the Trigger Task can forcefully interrupt the Handler
        xTaskCreate(vTriggerTask, "Trigger", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);    
        
        // Priority 1
        xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);    

        vTaskStartScheduler();
    }

    for (;;);
}