/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     UTIMER_app.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     31-August-2021
 * @brief    DEMO application for UTIMER.
 *           - Configuring the UTIMER Channel 0 for 500ms basic mode.
 *           - Configuring the UTIMER Channel 1 for 500ms, 1000ms, 1500ms buffering mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
/* include for Utimer Driver */
#include "Driver_UTIMER.h"

/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/*Define for the CMSIS-RTOS2 objects*/
#define UTIMER_BASIC_MODE_CB_EVENT               (0x01)
#define UTIMER_BUFFERING_MODE_CB_EVENT           (0x02)

#define UTIMER_BASIC_MODE_WAIT_TIME              1000
#define UTIMER_BUFFERING_MODE_WAIT_TIME          5000

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t  Utimer_buffer_xHandle;
TaskHandle_t  Utimer_basic_xHandle;

extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;



/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
   *ppxIdleTaskTCBBuffer = &IdleTcb;
   *ppxIdleTaskStackBuffer = IdleStack;
   *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
   (void) pxTask;

   for (;;);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
      StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
   *ppxTimerTaskTCBBuffer = &TimerTcb;
   *ppxTimerTaskStackBuffer = TimerStack;
   *pulTimerTaskStackSize = TIMER_SERVICE_TASK_STACK_SIZE;
}

void vApplicationIdleHook(void)
{
   for (;;);
}

/*****************Only for FreeRTOS use *************************/


void utimer_basic_mode_cb_fun (uint8_t event)
{
    if (event & ARM_UTIMER_OVER_FLOW_EVENT)
    {
	xTaskNotifyFromISR(Utimer_basic_xHandle,UTIMER_BASIC_MODE_CB_EVENT,eSetBits, NULL);
    }
}

void utimer_basic_mode_app (void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 0;
    BaseType_t xReturned;
    uint32_t count_array[ARM_UTIMER_MODE_BASIC_COUNT_NUMBER];

    printf("utimer demo application for basic mode started\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6)) = 20000000
     *
     * DEC = 20000000
     * HEX = 0xBEBC200
     *
     */
    count_array[0] = 0x00000000;   /*< initial counter value >*/
    count_array[1] = 0xBEBC200;    /*< over flow count value >*/

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_BASIC,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_BASIC_COUNT_NUMBER
    };

    ret = ptrUTIMER->Initialize (channel, utimer_basic_mode_cb_fun);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_basic_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_basic_mode_poweroff;
    }

    printf("utimer channel '%d'configured on basic mode for 500 ms\r\n", channel);

    ret = ptrUTIMER->Start (channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_basic_mode_poweroff;
    } else {
        printf("utimer channel '%d': timer started\n\n", channel);
    }

    xReturned = xTaskNotifyWait(NULL,UTIMER_BASIC_MODE_CB_EVENT,NULL, UTIMER_BASIC_MODE_WAIT_TIME);
    if( xReturned != pdTRUE )
    {
        printf("\r\n Task notify wait timeout expired\r\n");
        goto error_basic_mode_poweroff;
    }

    printf("utimer channel %d :500ms timer expired \n", channel);

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_basic_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_basic_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }
    printf("demo application: basic mode completed \r\n");
    /*To wait here until another Thread finishes job or it will enter wait forever in app_main Thread*/
    vTaskDelay(portMAX_DELAY);
}

void utimer_buffering_mode_cb_fun (uint8_t event)
{
    if (event & ARM_UTIMER_OVER_FLOW_EVENT)
    {
	xTaskNotifyFromISR(Utimer_buffer_xHandle,UTIMER_BUFFERING_MODE_CB_EVENT,eSetBits, NULL);
    }
}

void utimer_buffering_mode_app (void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 1;
    uint8_t index;
    BaseType_t xReturned;
    uint32_t count_array[ARM_UTIMER_MODE_DOUBLE_BUFFERING_COUNT_NUMBER];

    printf("utimer demo application for buffering mode started\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6))
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 1000ms = (1000*(10^-3))/(0.0025*(10^-6))
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 1500ms = (500*(10^-3))/(0.0025*(10^-6))
     * DEC = 60000000
     * HEX = 0x23C34600
     *
     */

    count_array[0] = 0x00000000;     /*< Initial counter value>*/
    count_array[1] = 0xBEBC200;      /*< Over flow count value for First Iteration>*/
    count_array[2] = 0x17D78400;     /*< Over flow count value for Second Iteration>*/
    count_array[3] = 0x23C34600;     /*< Over flow count value for Third Iteration>*/

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_BUFFERING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_DOUBLE_BUFFERING_COUNT_NUMBER
    };

    ret = ptrUTIMER->Initialize (channel, utimer_buffering_mode_cb_fun);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_buffering_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    printf("channel '%d'configured on buffering mode for 500, 1000, and 1500 ms \r\n", channel);

    ret = ptrUTIMER->Start(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_buffering_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n\n", channel);
    }

    for (index=0; index<3; index++)
    {
        xReturned = xTaskNotifyWait(NULL,UTIMER_BUFFERING_MODE_CB_EVENT,NULL, UTIMER_BUFFERING_MODE_WAIT_TIME);
        if( xReturned != pdTRUE )
        {
		printf("\r\n Task notify wait timeout expired\r\n");
		goto error_buffering_mode_poweroff;
        }

        printf("utimer channel %d: %d ms timer expired\n", channel, (500+(500*index)));
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_buffering_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("uTIMER channel %d failed power off \n", channel);
    }

error_buffering_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("demo application: buffering mode completed \r\n");
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
{
   /* System Initialization */
   SystemCoreClockUpdate();

   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(utimer_buffering_mode_app, "utimer_buffering_mode_app",512, NULL,configMAX_PRIORITIES-1, &Utimer_buffer_xHandle);
   if (xReturned != pdPASS)
   {
       vTaskDelete(Utimer_buffer_xHandle);
       return -1;
    }

   /* Create application main thread */
    xReturned = xTaskCreate(utimer_basic_mode_app, "utimer_basic_mode_app",512, NULL,configMAX_PRIORITIES-1, &Utimer_basic_xHandle);
    if (xReturned != pdPASS)
    {
       vTaskDelete(Utimer_basic_xHandle);
       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}
