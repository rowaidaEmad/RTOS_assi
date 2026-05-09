#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global Handle for the Counting Semaphore
SemaphoreHandle_t xResourcePool = NULL;

// Global variables to observe in Keil Logic Analyzer
// ActiveResources will NEVER exceed 2, even though 5 tasks are running.
volatile uint32_t ActiveResources = 0; 
volatile uint8_t TaskUsingResource[5] = {0, 0, 0, 0, 0};

/* -------------------------------------------------------------------------
 * Hardware Initialization (Port F for Status LED and Button)
 * ------------------------------------------------------------------------- */
void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) {}; 

    // PF4 (SW1) Input, PF1 (Red) Output
    GPIO_PORTF_DIR_R &= ~0x10; 
    GPIO_PORTF_DIR_R |= 0x02;  
    GPIO_PORTF_DEN_R |= 0x12;  
    GPIO_PORTF_PUR_R |= 0x10;  

    // Configure External Interrupt on PF4 (SW1)
    GPIO_PORTF_IM_R &= ~0x10;  
    GPIO_PORTF_IS_R &= ~0x10;  
    GPIO_PORTF_IBE_R &= ~0x10; 
    GPIO_PORTF_IEV_R &= ~0x10; 
    GPIO_PORTF_ICR_R |= 0x10;  
    GPIO_PORTF_IM_R |= 0x10;   

    // Set NVIC Priority to 5 (Safe for FreeRTOS)
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000; 
    NVIC_EN0_R = (1 << 30); 
}

/* -------------------------------------------------------------------------
 * The ISR (Using the exact vector name requested)
 * ------------------------------------------------------------------------- */
void GPIOF_Handler(void) {
    // Clear the hardware flag
    GPIO_PORTF_ICR_R = 0x10;

    // Toggle Red LED to prove the vector is mapped correctly
    GPIO_PORTF_DATA_R ^= 0x02; 
}

/* -------------------------------------------------------------------------
 * The Worker Task (5 instances of this will run concurrently)
 * ------------------------------------------------------------------------- */
void vWorkerTask(void *pvParameters) {
    // Cast the parameter back to an integer to get this task's unique ID (0 to 4)
    int taskID = (int)pvParameters;

    for (;;) {
        // 1. Attempt to claim one of the 2 available resources
        // If 2 resources are already taken, this task will safely BLOCK here.
        if (xSemaphoreTake(xResourcePool, portMAX_DELAY) == pdTRUE) {
            
            // --- CRITICAL SECTION: RESOURCE ACQUIRED ---
            ActiveResources++; 
            TaskUsingResource[taskID] = 1; // Mark this specific task as active
            
            // Simulate "using" the resource for a varied amount of time
            // We use (taskID * 100) to ensure the 5 tasks get out of sync 
            // and truly compete randomly for the resources.
            vTaskDelay(pdMS_TO_TICKS(300 + (taskID * 100)));
            
            // --- RELEASE RESOURCE ---
            TaskUsingResource[taskID] = 0;
            ActiveResources--; 
            
            xSemaphoreGive(xResourcePool);
        }
        
        // Wait a bit before trying to claim a resource again
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

/* -------------------------------------------------------------------------
 * System Integration
 * ------------------------------------------------------------------------- */
int main(void) {
    PortF_Init();

    // Initialize the Counting Semaphore
    // Parameter 1: Maximum count (2 resources)
    // Parameter 2: Initial count (Starts with 2 available)
    xResourcePool = xSemaphoreCreateCounting(2, 2);

    if (xResourcePool != NULL) {
        // Create 5 identical worker tasks, passing 0-4 as their IDs
        xTaskCreate(vWorkerTask, "Worker0", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker1", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker2", configMINIMAL_STACK_SIZE, (void *)2, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker3", configMINIMAL_STACK_SIZE, (void *)3, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vWorkerTask, "Worker4", configMINIMAL_STACK_SIZE, (void *)4, tskIDLE_PRIORITY + 1, NULL);

        vTaskStartScheduler();
    }

    for (;;);
}