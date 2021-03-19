/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "params.h"
#include "queue.h"

volatile SemaphoreHandle_t bouton_semph;
volatile bool boutonAppuye = false;

 void inverseLED(){
    for(;;){
        Cy_GPIO_Write(P1_1_PORT, P1_1_NUM, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        Cy_GPIO_Write(P1_1_PORT, P1_1_NUM, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

void bouton_task(){
    for(;;){   
        if( bouton_semph != NULL )
        {
            
            xSemaphoreTake( bouton_semph, pdMS_TO_TICKS(20));            
            if( xSemaphoreTake( bouton_semph, pdMS_TO_TICKS(20) ) == pdTRUE ){
                if(Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM) == false){
                    UART_PutString("Bouton appuye \r\n");
                    boutonAppuye = true;
                }else if (Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM)){
                    UART_PutString("Bouton relache \r\n");
                    boutonAppuye = false;
                }    
                xSemaphoreGive( bouton_semph );
            }
        }
    }
}

void isr_bouton(){
    /*if(Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM)){
        UART_PutString("Interruption enclenche 2 \r\n");
    }else if(Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM) == false){
        UART_PutString("Interruption enclenche 1 \r\n");
    }
   UART_PutString("Interruption enclenche \r\n");*/
    
    xSemaphoreGiveFromISR(bouton_semph,NULL);
    Cy_GPIO_ClearInterrupt(BOUTON_0_PORT, BOUTON_0_NUM);
    NVIC_ClearPendingIRQ(bouton_ISR_cfg.intrSrc);
    
}

int main(void)
{
    bouton_semph = xSemaphoreCreateBinary();
    
    Cy_SysInt_Init(&bouton_ISR_cfg, isr_bouton);
    NVIC_ClearPendingIRQ(bouton_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(bouton_ISR_cfg.intrSrc);
    __enable_irq(); /* Enable global interrupts. */
    
    UART_Start();
    
    UART_PutString("Debut programme \r\n");
    
    xTaskCreate(inverseLED,"LED", 200, NULL, 1, NULL);
    xTaskCreate(bouton_task,"Bouton", 1000, NULL, 2, NULL);    
    vTaskStartScheduler();

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
