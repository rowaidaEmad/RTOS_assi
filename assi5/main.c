#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Binary Semaphore
SemaphoreHandle_t xBinarySemaphore = NULL;

// Global counters to observe in Keil Logic Analyzer or Watch Window
volatile uint32_t InterruptsFired = 0;
volatile uint32_t TasksProcessed = 0;

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for simulation purposes)
 * ------------------------------------------------------------------------- */
void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    GPIO_PORTF_DIR_R |= 0x0E;  
    GPIO_PORTF_DEN_R |= 0x1E;
    
    GPIO_PORTF_IM_R |= 0x10;   

    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; 
    
    NVIC_EN0_R = (1 << 30); 
}

/* -------------------------------------------------------------------------
 * The ISR (Simulated Hardware Interrupt)
 * ------------------------------------------------------------------------- */
void GPIOF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIO_PORTF_ICR_R = 0x10;

    InterruptsFired++;

    if (xBinarySemaphore != NULL) {
        xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* -------------------------------------------------------------------------
 * Task 1: The Trigger Task (Simulates high-frequency hardware events)
 * ------------------------------------------------------------------------- */
void vTriggerTask(void *pvParameters) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        NVIC_SW_TRIG_R = 30; // Wakes Handler
        vTaskDelay(pdMS_TO_TICKS(100));
        
        NVIC_SW_TRIG_R = 30; // Fills Semaphore to 1
        vTaskDelay(pdMS_TO_TICKS(100));
        
        NVIC_SW_TRIG_R = 30; // DROPPED! 
    }
}

/* -------------------------------------------------------------------------
 * Task 2: The Handler Task (Simulates slow event processing)
 * ------------------------------------------------------------------------- */
void vHandlerTask(void *pvParameters) {
    for (;;) {
        // Block until an interrupt gives the semaphore
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
            
            GPIO_PORTF_DATA_R |= 0x02; 
            
            TasksProcessed++;
            
            // SIMULATE HEAVY PROCESSING
            // Because this is a busy-wait, FreeRTOS cannot run this task's 
            // xSemaphoreTake again until this loop finishes.
            for(volatile int i = 0; i < 2000000; i++) {}
            
            // Turn off Red LED
            GPIO_PORTF_DATA_R &= ~0x02; 
        }
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();

    // Create the Binary Semaphore
    xBinarySemaphore = xSemaphoreCreateBinary();

    if (xBinarySemaphore != NULL) {
        // Trigger Task runs at Priority 2 to successfully interrupt the Handler
        xTaskCreate(vTriggerTask, "Trigger", configMINIMAL_STACK_SIZE, NULL, 2, NULL);    
        
        // Handler Task runs at Priority 1
        xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE, NULL, 1, NULL);    

        vTaskStartScheduler();
    }

    for (;;);
}