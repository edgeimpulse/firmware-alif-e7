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
 * @file     uart4_testApp.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     23-August-2021
 * @brief    TestApp to verify UART interface using FreeRTOS as an operating system.
 *           UART interactive console application (using UART4 instance):
 *             UART waits for a char on serial terminal;
 *               if 'Enter' key is received; UART Sends back "Hello World!".
 *               else, it sends back received character.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Includes --------------------------------------------------------------------------- */

/* System Includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Project Includes */

/* include for UART Driver */
#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include "Driver_PINMUX_AND_PINPAD.h"

/*RTOS Includes*/
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* UART Driver instance */
#define UART      4

/* For Release build disable printf and semihosting */
#define DISABLE_PRINTF

#ifdef DISABLE_PRINTF
#define printf(fmt, ...) (0)
/* Also Disable Semihosting */
#if __ARMCC_VERSION >= 6000000
__asm(".global __use_no_semihosting");
#elif __ARMCC_VERSION >= 5000000
            #pragma import(__use_no_semihosting)
    #else
            #error Unsupported compiler
    #endif

void _sys_exit(int return_code) {
   while (1);
}
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

/*Define for FreeRTOS objects */
#define UART_RX_CB_EVENT                       0x01
#define UART_TX_CB_EVENT                       0x02

TaskHandle_t Uart_xHandle;

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

/**
 * @function   void UART_callback(uint32_t event)
 * @brief      UART isr callabck
 * @note       none
 * @param      event: USART Event
 * @retval     none
 */
void myUART_callback(uint32_t event)
{
   if (event & ARM_USART_EVENT_SEND_COMPLETE) {
      /* Success: Wakeup Thread */
      xTaskNotifyFromISR(Uart_xHandle,UART_TX_CB_EVENT,eSetBits, NULL);
   }
   if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
      /* Success: Wakeup Thread */
      xTaskNotifyFromISR(Uart_xHandle,UART_RX_CB_EVENT,eSetBits, NULL);
   }
   if (event & ARM_USART_EVENT_RX_TIMEOUT) {
      /* Error: Call debugger or replace with custom error handling */
   }
   if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
      /* Error: Call debugger or replace with custom error handling */
   }
}

/**
 * @function   int hardware_init(void)
 * @brief      UART hardware pin initialization using PIN-MUX driver
 * @note       none
 * @param      void
 * @retval     0:success, -1:failure
 */
int hardware_init(void)
{
   int ret;

   /* PINMUX UART4_B */

   /* Configure GPIO Pin : P3_1 as UART4_RX_B */
   ret = PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_1,
   PINMUX_ALTERNATE_FUNCTION_1);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in PINMUX \r\n");
      return -1;
   }

   /* Configure GPIO Pin : P3_2 as UART4_TX_B */
   ret = PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_2,
   PINMUX_ALTERNATE_FUNCTION_1);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in PINMUX.\r\n");
      return -1;
   }

   return 0;
}

void Uart_Thread(void *pvParameters)
{

   uint8_t cmd = 0;
   uint32_t ret = 0;
   uint32_t events = 0;
   ARM_DRIVER_VERSION version;

   printf("\r\n >>> UART testApp starting up!!!...<<< \r\n");

   version = USARTdrv->GetVersion();
   printf("\r\n UART version api:%X driver:%X...\r\n", version.api,version.drv);

   /* Initialize UART hardware pins using PinMux Driver. */
   ret = hardware_init();
   if (ret != 0)
   {
      printf("\r\n Error in UART hardware_init.\r\n");
      return;
   }

   /* Initialize UART driver */
   ret = USARTdrv->Initialize(myUART_callback);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Initialize.\r\n");
      return;
   }

   /* Power up UART peripheral */
   ret = USARTdrv->PowerControl(ARM_POWER_FULL);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Power Up.\r\n");
      goto error_uninitialize;
   }

   /* Configure UART to 115200 Bits/sec */
   ret = USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
   ARM_USART_DATA_BITS_8 |
   ARM_USART_PARITY_NONE |
   ARM_USART_STOP_BITS_1 |
   ARM_USART_FLOW_CONTROL_NONE, 115200);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Control.\r\n");
      goto error_poweroff;
   }

   /* Enable Receiver and Transmitter lines */
   ret = USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Control TX.\r\n");
      goto error_poweroff;
   }

   ret = USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Control RX.\r\n");
      goto error_poweroff;
   }

   printf("\r\n Press Enter or any character on serial terminal to receive a message:\r\n");

   ret = USARTdrv->Send("\nPress Enter or any character to receive a message\n", 51);
   if (ret != ARM_DRIVER_OK)
   {
      printf("\r\n Error in UART Send.\r\n");
      goto error_poweroff;
   }
   xTaskNotifyWait(NULL,UART_TX_CB_EVENT,NULL, portMAX_DELAY);

   while (1)
   {
      /* Get byte from UART */
      ret = USARTdrv->Receive(&cmd, 1);
      if (ret != ARM_DRIVER_OK)
      {
         printf("\r\n Error in UART Receive.\r\n");

         goto error_poweroff;
      }
      /* wait till Receive complete event comes in isr callback */
      xTaskNotifyWait(NULL,UART_RX_CB_EVENT,NULL, portMAX_DELAY);

      if (cmd == 13) /* CR, send greeting  */
      {
         USARTdrv->Send("\nHello World!", 14);

      }
      else /* else send back received character. */
      {
         ret = USARTdrv->Send(&cmd, 1);
      }
      xTaskNotifyWait(NULL,UART_TX_CB_EVENT,NULL, portMAX_DELAY);

   }

   printf("\r\n XXX UART demo thread exiting XXX...\r\n");

error_poweroff:

   /* Received error Power off UART peripheral */
   ret = USARTdrv->PowerControl(ARM_POWER_OFF);
   if (ret != ARM_DRIVER_OK) {
      printf("\r\n Error in UART Power OFF.\r\n");
   }

error_uninitialize:

   /* Received error Un-initialize UART driver */
   ret = USARTdrv->Uninitialize();
   if (ret != ARM_DRIVER_OK) {
      printf("\r\n Error in UART Uninitialize.\r\n");
   }

}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main(void)
{
   /* System Initialization */
   SystemCoreClockUpdate();

   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(Uart_Thread, "UartThread", 1024, NULL,configMAX_PRIORITIES-1, &Uart_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(Uart_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();
}
