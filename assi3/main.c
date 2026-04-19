/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "TM4C123GH6PM.h"

#include "semphr.h"


#define mainGPIO_INTERRUPT_ID              GPIOF_IRQn
#define mainGPIO_INTERRUPT_PRIORITY   ( 5 )

static void prvSetupGPIOInterrupt( void );
void GPIOF_Handler( void );
int HandlerCounter=0;
int PeriodicCounter =0;

void PeriodicTask(void* pvParameters);
void HandlerTask(void* pvParameters);
xSemaphoreHandle xSemaphore;

void myDelay()
{
    for(int i = 0; i < 10 ; i++);
}

static void prvSetupGPIOInterrupt( void )
{
    SYSCTL->RCGCGPIO |= (1U << 5);          /* Enable clock for Port F */
    while((SYSCTL->PRGPIO & (1U << 5)) == 0);

    GPIOF->DIR &= ~(1U << 4);               /* PF4 (SW1) input */
    GPIOF->DEN |= (1U << 4);                /* Digital enable PF4 */
    GPIOF->PUR |= (1U << 4);                /* Enable pull-up on PF4 */

    GPIOF->IS  &= ~(1U << 4);               /* Edge-sensitive */
    GPIOF->IBE &= ~(1U << 4);               /* Single edge */
    GPIOF->IEV &= ~(1U << 4);               /* Falling edge (button press) */
    GPIOF->ICR  =  (1U << 4);               /* Clear pending flag */
    GPIOF->IM  |=  (1U << 4);               /* Unmask interrupt */

  
    NVIC_SetPriority( mainGPIO_INTERRUPT_ID, mainGPIO_INTERRUPT_PRIORITY  );

    NVIC_EnableIRQ( mainGPIO_INTERRUPT_ID );
}

void PeriodicTask(void* pvParameters)
{
    while(1)
        {
            myDelay();
            PeriodicCounter++;
            myDelay();
            vTaskDelay(100/portTICK_RATE_MS);
        }
}
void HandlerTask(void* pvParameters)
{
    while(1)
    {
        xSemaphoreTake(xSemaphore,portMAX_DELAY);
        HandlerCounter++;
    }
}

int main(void) {
        xTaskCreate(PeriodicTask,"Periodic Task",240,NULL,3,NULL);
        xTaskCreate(HandlerTask,"Handler Task",240,NULL,5,NULL);
        xSemaphore = xSemaphoreCreateBinary(); // empty semaphore

        prvSetupGPIOInterrupt();
        __ASM("CPSIE I");
        vTaskStartScheduler();
    for(;;);
}



void GPIOF_Handler( void )
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore,&xHigherPriorityTaskWoken);
    GPIOF->ICR = (1U << 4);
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
