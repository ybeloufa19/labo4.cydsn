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

task_params_t task_A = {
    .delay = 1000,
    .message = "Tache A en cours \n\r"
};
task_params_t task_B = {
    .delay = 900,
    .message = "Tache B en cours \n\r"
};

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

void print_loop(void* params){

     for(;;){
        task_params_t copyParams;
        copyParams = *(task_params_t *)params;
        vTaskDelay(pdMS_TO_TICKS(copyParams.delay));
        UART_PutString(copyParams.message);
    }

}

void bouton_task(){
    for(;;){   
        if( bouton_semph != NULL )
        {
            vTaskDelay(pdMS_TO_TICKS(20));           
            if( xSemaphoreTake( bouton_semph, pdMS_TO_TICKS(0) ) == pdTRUE ){
                if(Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM) == false){
                    UART_PutString("Bouton appuye \r\n");
                    
                }else if (Cy_GPIO_Read(BOUTON_0_PORT, BOUTON_0_NUM)){
                    UART_PutString("Bouton relache \r\n");                    
                }    
            }
            //xSemaphoreGive( bouton_semph );
        }
    }
}

void isr_bouton(){
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
    
    xTaskCreate(inverseLED,"LED", 200, NULL, 0, NULL);
    xTaskCreate(bouton_task,"Bouton", 1000, NULL, 2, NULL);    
    xTaskCreate(print_loop, "task A", configMINIMAL_STACK_SIZE, (void*) &task_A, 1, NULL);
    xTaskCreate(print_loop, "task B", configMINIMAL_STACK_SIZE, (void*) &task_B, 1, NULL);
    vTaskStartScheduler();

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
