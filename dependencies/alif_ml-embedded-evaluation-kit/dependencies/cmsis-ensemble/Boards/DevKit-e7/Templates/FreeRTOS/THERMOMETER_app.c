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
 * @file     THERMOMETER_app.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     10-September-2021
 * @brief    demo application for SPI.
 *           - Configuring the DS1722 Thermometer in continuous conversion mode with 12 bit resolution.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
/* System Includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*include for SPI Driver*/
#include "Driver_GPIO.h"
#include "Driver_SPI.h"
#include "Driver_PINMUX_AND_PINPAD.h"

/*RTOS Includes*/
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


#define DS1722_THREAD_STACK_SIZE                        (512)
#define THERMOMETER_EVENT_WAIT_TIME                     5000

#define DS1722_THERMOMETER_OK                           0
#define DS1722_THERMOMETER_ERROR                        (-1)

#define DS1722_THERMOMETER_SHUTDOWN_BIT                 0
#define DS1722_THERMOMETER_SHUTDOWN_BIT_mask            (0x1UL << DS1722_THERMOMETER_SHUTDOWN_BIT)

#define DS1722_THERMOMETER_RESOLUTION_BIT               1
#define DS1722_THERMOMETER_RESOLUTION_BIT_mask          (0x07UL << DS1722_THERMOMETER_RESOLUTION_BIT)
#define DS1722_THERMOMETER_RESOLUTION_R0_BIT            0x2
#define DS1722_THERMOMETER_RESOLUTION_R1_BIT            0x4
#define DS1722_THERMOMETER_RESOLUTION_R2_BIT            0x8

#define DS1722_THERMOMETER_ONE_SHOT_BIT                 4
#define DS1722_THERMOMETER_ONE_SHOT_BIT_mask            (0x01UL << DS1722_THERMOMETER_ONE_SHOT_BIT)

#define DS1722_THERMOMETER_CONVERSIONS_INACTIVE         0
#define DS1722_THERMOMETER_CONVERSIONS_ACTIVE           DS1722_THERMOMETER_SHUTDOWN_BIT_mask

#define DS1722_THERMOMETER_RESOLUTION_08                0
#define DS1722_THERMOMETER_RESOLUTION_09                DS1722_THERMOMETER_RESOLUTION_R0_BIT
#define DS1722_THERMOMETER_RESOLUTION_10                DS1722_THERMOMETER_RESOLUTION_R1_BIT
#define DS1722_THERMOMETER_RESOLUTION_11                (DS1722_THERMOMETER_RESOLUTION_R0_BIT | DS1722_THERMOMETER_RESOLUTION_R1_BIT)
#define DS1722_THERMOMETER_RESOLUTION_12                DS1722_THERMOMETER_RESOLUTION_R2_BIT

#define DS1722_THERMOMETER_CONTINUOUS_CONVERSION_MODE   0
#define DS1722_THERMOMETER_DISCONTINUOUS_CONVERSION_MODE DS1722_THERMOMETER_ONE_SHOT_BIT_mask

#define DS1722_THERMOMETER_RESERVED_BIT                 0xE0

#define DS1722_SET_CONFIGURATION_ADDRESS                0x80U
#define DS1722_GET_CONFIGURATION_ADDRESS                0x00U
#define DS1722_GET_TEMPERATURE_MSB                      0x0100U
#define DS1722_GET_TEMPERATURE_LSB                      0x0200U

#define GPIO2                                           2
#define SPI3                                            3

#define SPI3_PORT                                       PORT_NUMBER_2

#define SPI3_MISO_PIN                                   PIN_NUMBER_21
#define SPI3_MOSI_PIN                                   PIN_NUMBER_22
#define SPI3_CLOCK_PIN                                  PIN_NUMBER_23
#define SPI3_SS0_PIN                                    PIN_NUMBER_24

#define SPI3_MISO_PIN_AF                                PINMUX_ALTERNATE_FUNCTION_4
#define SPI3_MOSI_PIN_AF                                PINMUX_ALTERNATE_FUNCTION_3
#define SPI3_CLOCK_PIN_AF                               PINMUX_ALTERNATE_FUNCTION_3
#define SPI3_SS0_PIN_AF                                 PINMUX_ALTERNATE_FUNCTION_0


/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t  spi_xHandle;

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

/*Define for the CMSIS-RTOS2 objects*/
#define THERMOMETER_CALLBACK_EVENT                      0x1

extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO2);
ARM_DRIVER_GPIO *ptrGPIO2 = &ARM_Driver_GPIO_(GPIO2);

extern ARM_DRIVER_SPI ARM_Driver_SPI_(SPI3);
ARM_DRIVER_SPI *ptrSPI3 = &ARM_Driver_SPI_(SPI3);

/**
 * @fn      void SPI_callback (uint32_t event)
 * @brief   Ds1722 application call back function.
 * @note    none.
 * @param   event: SPI event.
 * @retval  None
 */
void SPI_callback (uint32_t event)
{
    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        ptrGPIO2->SetValue(SPI3_SS0_PIN, GPIO_PIN_OUTPUT_STATE_LOW);

        xTaskNotifyFromISR(spi_xHandle,THERMOMETER_CALLBACK_EVENT,eSetBits,NULL);
    }
}

/**
 * @fn      void Ds1722_Thread( void *arg)
 * @brief   Used to read the temperature from DS1722 thermometer.
 * @note    none.
 * @param   thread_input : thread input.
 * @retval  none.
 */
void Ds1722_Thread (void *pvParameters)
{
    uint16_t lower_byte_cmd, higher_byte_cmd, config_cmd_and_value;
    int32_t control_code, Baud_rate, ret = 0;
    BaseType_t xReturned;
    uint16_t higher_byte[2], lower_byte[2];
    ARM_SPI_STATUS var;

    /* Doing pinmux configuration for SPI pin's */
    ret = PINMUX_Config(SPI3_PORT, SPI3_MISO_PIN, SPI3_MISO_PIN_AF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_MISO_PIN\n");
        return;
    }

    ret = PINMUX_Config(SPI3_PORT, SPI3_MOSI_PIN, SPI3_MOSI_PIN_AF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_MOSI_PIN\n");
        return;
    }

    ret = PINMUX_Config(SPI3_PORT, SPI3_CLOCK_PIN, SPI3_CLOCK_PIN_AF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_CLOCK_PIN\n");
        return;
    }

    ret = PINMUX_Config(SPI3_PORT, SPI3_SS0_PIN, SPI3_SS0_PIN_AF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_SS0_PIN\n");
        return;
    }

    /*
     * DS1722 Thermometer sensor "CE (Chip Enable)" pin behavior is opposite to standard SPI chip selection behavior.
     * To operate CE (Chip Enable) pin configuring SPI3_SS0_PIN as GPIO, following Bit-Banging method.
    */

    ret = ptrGPIO2->Initialize(SPI3_SS0_PIN, NULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize SPI3_SS0_PIN as GPIO\n");
        return;
    }

    ret = ptrGPIO2->PowerControl(SPI3_SS0_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to Powered the SPI3_SS0_PIN\n");
        goto error_gpio_uninitialize;
    }

    ret = ptrGPIO2->SetDirection(SPI3_SS0_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to set direction for SPI3_SS0_PIN\n");
        goto error_gpio_power_off;
    }

    ret = ptrGPIO2->SetValue(SPI3_SS0_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to set the state for SPI3_SS0_PIN\n");
        goto error_gpio_power_off;
    }

    /*
     * SPI-3 connected to DS1722 Thermometer sensor. configuring to read the temperature.
     */

    control_code = (ARM_SPI_MODE_MASTER | ARM_SPI_SS_MASTER_SW | ARM_SPI_CPOL0_CPHA1|
                    ARM_SPI_DATA_BITS(16));

    Baud_rate = 1000000;  // SPI Baud rate 1 MHz

    lower_byte_cmd  =       DS1722_GET_TEMPERATURE_LSB;
    higher_byte_cmd =       DS1722_GET_TEMPERATURE_MSB;
    config_cmd_and_value =  ((DS1722_SET_CONFIGURATION_ADDRESS << 8) |
                            DS1722_THERMOMETER_RESOLUTION_12 | DS1722_THERMOMETER_RESERVED_BIT|
                            DS1722_THERMOMETER_CONTINUOUS_CONVERSION_MODE);

    ret = ptrSPI3->Initialize(SPI_callback);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize the SPI3\n");
        goto error_gpio_power_off;
    }

    ret = ptrSPI3->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to power SPI3\n");
        goto error_spi_uninitialize;
    }

    ret = ptrSPI3->Control(control_code, Baud_rate);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI3\n");
        goto error_spi_power_off;
    }

    ret = ptrSPI3->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to enable the slave select of SPI3\n");
        goto error_spi_power_off;
    }

    ret = ptrGPIO2->SetValue(SPI3_SS0_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to Set SPI3_SS0_PIN state to high\n");
        goto error_spi_power_off;
    }

    /* Configuring the DS1722 thermometer to 12 bit resolution, Continues operation mode */
    ret = ptrSPI3->Send(&config_cmd_and_value, 1);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to Configure the Ds1722-THERMOMETER\n");
        goto error_spi_power_off;
    }

    xReturned = xTaskNotifyWait(NULL,THERMOMETER_CALLBACK_EVENT,NULL, THERMOMETER_EVENT_WAIT_TIME);
    if (xReturned != pdTRUE)
    {
        printf("ERROR : event not received, timeout happened \n");
        goto error_spi_power_off;
    }

    while(1)
    {
        ret = ptrGPIO2->SetValue(SPI3_SS0_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
        if (ret != ARM_DRIVER_OK)
        {
            printf("ERROR: Failed to configure O/p state SPI3_SS0_PIN\n");
            goto error_spi_power_off;
        }

        ret = ptrSPI3->Transfer(&lower_byte_cmd, lower_byte, 1);
        if (ret != ARM_DRIVER_OK)
        {
            printf("ERROR: Failed to SPI Transfer %x\n", ret);
            goto error_spi_power_off;
        }

        xReturned = xTaskNotifyWait(NULL,THERMOMETER_CALLBACK_EVENT, NULL, THERMOMETER_EVENT_WAIT_TIME);
        if (xReturned != pdTRUE)
        {
            printf("ERROR : event not received, timeout happened \n");
            goto error_spi_power_off;
        }

        /* Between the command providing delay of 5 ticks */
        vTaskDelay(5);

        ret = ptrGPIO2->SetValue(SPI3_SS0_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
        if (ret != ARM_DRIVER_OK)
        {
            printf("ERROR: Failed to configure O/p state SPI3_SS0_PIN\n");
            goto error_spi_power_off;
        }

        ret = ptrSPI3->Transfer(&higher_byte_cmd, higher_byte, 1);
        if (ret != ARM_DRIVER_OK)
        {
            printf("ERROR: Failed to SPI Transfer %x\n", ret);
            goto error_spi_power_off;
        }

        xReturned = xTaskNotifyWait(NULL,THERMOMETER_CALLBACK_EVENT, NULL, THERMOMETER_EVENT_WAIT_TIME);
        if (xReturned != pdTRUE)
        {
            printf("ERROR : event not received, timeout happened \n");
            goto error_spi_power_off;
        }

        printf("Temperature = %d.%d Degree Celsius \n", lower_byte[0], (higher_byte[0] >>4)*0625);
    }

error_spi_power_off :
    ret = ptrSPI3->PowerControl(ARM_POWER_OFF);
    if (ret == ARM_DRIVER_OK)
    {
        printf("Power off SPI3\n");
    }

error_spi_uninitialize :
    ret = ptrSPI3->Uninitialize();
    if (ret == ARM_DRIVER_OK)
    {
        printf("Un-Initialized SPI3\n");
    }

error_gpio_power_off :
    ret = ptrGPIO2->PowerControl(SPI3_SS0_PIN, ARM_POWER_OFF);
    if (ret == ARM_DRIVER_OK)
    {
        printf("Power off SPI3_SS0_PIN\n");
    }

error_gpio_uninitialize :
    ret = ptrGPIO2->Uninitialize(SPI3_SS0_PIN);
    if (ret == ARM_DRIVER_OK)
    {
        printf("Un-Initialized SPI3_SS0_PIN\n");
    }
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
{
   /* System Initialization */
   SystemCoreClockUpdate();

   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(Ds1722_Thread, "Ds1722_Thread",DS1722_THREAD_STACK_SIZE, NULL,configMAX_PRIORITIES-1, &spi_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(spi_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();

}
