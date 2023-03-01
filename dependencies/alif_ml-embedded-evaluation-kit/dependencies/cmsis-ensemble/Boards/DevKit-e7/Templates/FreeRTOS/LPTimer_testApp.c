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
 * @file     LPTimer_testApp.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     28-August-2021
 * @brief    demo application for lptimer.
 *           - Configuring the lptimer channel 0 for 500micro second.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
/* include for UART Driver */
#include "Driver_LPTIMER.h"
/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/*Define for the FreeRTOS objects*/
#define LPTIMER_CALLBACK_EVENT     0x01
#define LPTIMER_CHANNEL_0          0 // lptimer have 0-3 channels, using channel zero for demo app
#define LPTIMER_EVENT_WAIT_TIME    1000


/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t lptimer_xHandle;


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


void lptimer_cb_fun (uint8_t event)
{
    if (event & ARM_LPTIMER_EVENT_UNDERFLOW) {
        xTaskNotifyFromISR(lptimer_xHandle,LPTIMER_CALLBACK_EVENT,eSetBits, NULL);
    }
}

void lptimer_Thread (void *pvParameters)
{
    extern ARM_DRIVER_LPTIMER DRIVER_LPTIMER0;
    ARM_DRIVER_LPTIMER *ptrDrv = &DRIVER_LPTIMER0;

    /* Configuring the lptimer channel 0 for 500micro second
     *
     * System clock frequency (F)= 400Mhz
     *
     * time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To increment or decrement timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500us = (500*(10^-6))/(0.0025*(10^-6)) = 200000
     *
     * DEC = 200000
     * HEX = 0x30D40
    */

    /*Timer channel configured 500 usec*/
    uint32_t count = 0x30D40;
    uint8_t channel = LPTIMER_CHANNEL_0;
    BaseType_t xReturned;
    uint32_t ret;

    ret = ptrDrv->Initialize (channel, lptimer_cb_fun);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to initialize\r\n", channel);
        return;
    }

    ret = ptrDrv->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to power up\r\n", channel);
        goto error_uninstall;
    }

    /**< Loading the counter value >*/
    ret = ptrDrv->Control (channel, ARM_LPTIMER_SET_COUNT1, &count);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to load count\r\n", channel);
        goto error_poweroff;
    }

    printf("demo application: lptimer channel '%d'configured for 500 us \r\n\n", channel);

    ret = ptrDrv->Start (channel);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to start channel '%d' timer\n", channel);
        goto error_poweroff;
    } else {
        printf("timer started\r\n");
    }

    xReturned = xTaskNotifyWait(NULL,LPTIMER_CALLBACK_EVENT,NULL, LPTIMER_EVENT_WAIT_TIME);
    if (xReturned != pdTRUE )
    {
	printf("\n\r Task Wait Time out expired \n\r");
	goto error_poweroff;
    }

    printf("500us timer expired \r\n");

    ret = ptrDrv->Stop(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to stop channel %d\n", channel);
    } else {
        printf("timer stopped\r\n\n");
    }

error_poweroff:

    ret = ptrDrv->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to power off channel '%d'\n", channel);
    }

error_uninstall:

	ret = ptrDrv->Uninitialize(channel);
	if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to un-initialize channel %d\n", channel);
    }

    printf("demo application: completed \r\n");
}


/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
{
   /* System Initialization */
   SystemCoreClockUpdate();

   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(lptimer_Thread, "lpTimer_Thread", 1024, NULL,configMAX_PRIORITIES-1, &lptimer_xHandle);
   if (xReturned != pdPASS)
   {

       vTaskDelete(lptimer_xHandle);
       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}
