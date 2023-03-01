/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     HWSEM0_Baremetal.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-June-2022
 * @brief    TestApp to verify HWSEM interface using baremetal, no operating system.
 *             It locks the HWSEM, sends the message through UART and then unlocks.
 *               This application will run on two cores
 *               to demonstrate the working of the Hardware Semaphore.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <RTE_Components.h>
#include CMSIS_device_header

/* System Includes */
#include <stdio.h>
#include <string.h>

/* Project Includes */
/* include for UART Driver */
#include "Driver_USART.h"

/* PINMUX Driver */
#include "Driver_PINMUX_AND_PINPAD.h"
/* include for HWSEM Driver */
#include "Driver_HWSEM.h"

#ifdef M55_HP
const char * startup_msg = "\n<<<<M55_HP : HWSEM testApp starting up>>>>\r\n";
const char * msg = "\nPrinting from M55_HP";
const char * acq_msg = "\nM55_HP acquiring the semaphore, printing 10 messages\r\n";
const char * rel_msg = "\nM55_HP releasing the semaphore\r\n\n";
#else
const char * startup_msg = "\nM55_HE : HWSEM testApp\r\n";
const char * msg = "\nPrinting from M55_HE";
const char * acq_msg = "\nM55_HE acquiring the semaphore, printing 10 messages\r\n";
const char * rel_msg = "\nM55_HE releasing the semaphore\r\n\n";
#endif

/* HWSEM Driver instance (HWSEM0-HWSEM15) */
#define HWSEM      0
#define HWSEM_CB_EVENT            1U << 0

/* Mention the Uart instance */
#define UART    6

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2

volatile uint32_t event_flags_uart;
volatile uint32_t event_flags_hwsem = 0;

/* For Release build disable printf and semihosting */
//#define DISABLE_PRINTF

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
  void _sys_exit(int return_code) { while (1); }
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

/* HWSEM Driver */
extern ARM_DRIVER_HWSEM ARM_Driver_HWSEM_(HWSEM);

/* HWSEM Driver instance */
static ARM_DRIVER_HWSEM *HWSEMdrv = &ARM_Driver_HWSEM_(HWSEM);

void myHWSEM_app_entry();

/**
 * @function    void myUART_callback(uint32_t event)
 * @brief       UART isr callabck
 * @note        none
 * @param       event: USART Event
 * @retval      none
 */
void myUART_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        /* Send Success */
        event_flags_uart |= UART_CB_TX_EVENT;
    }

    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        /* Receive Success */
        event_flags_uart |= UART_CB_RX_EVENT;
    }

    if (event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        /* Receive Success with rx timeout */
        event_flags_uart |= UART_CB_RX_TIMEOUT;
    }
}

/**
 * @function    void myHWSEM_callback(int32_t event, uint8_t sem_id)
 * @brief       HWSEM isr callabck
 * @note        none
 * @param       event: HWSEM Available Event
 * @retval      none
 */
void myHWSEM_callback(int32_t event, uint8_t sem_id)
{
    if (event & HWSEM_AVAILABLE_CB_EVENT)
    {
        /* Send Success */
        event_flags_hwsem |= HWSEM_CB_EVENT;
    }
}

/**
 * @function    int32_t hardware_init(void)
 * @brief       Uart initialization
 * @note        none
 * @param       void
 * @retval      execution status
 */
int32_t hardware_init(void)
{
    int32_t ret = ARM_DRIVER_OK;
    ARM_DRIVER_VERSION version;

    /* PINMUX UART6_A */

    /* Configure GPIO Pin : P1_14 as UART6_RX_A */
    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_14, PINMUX_ALTERNATE_FUNCTION_1);

    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* PINMUX UART6_A */

    /* Configure GPIO Pin : P1_15 as UART6_TX_A */
    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_15, PINMUX_ALTERNATE_FUNCTION_1);

    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Initialize UART driver */
    ret = USARTdrv->Initialize(myUART_callback);

    if(ret)
    {
        printf("\r\n Error in UART Initialize.\r\n");
        return ARM_DRIVER_ERROR;
    }

    /* Power up UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);

    if(ret)
    {
        printf("\r\n Error in UART Power Up.\r\n");
        goto error_uninitialize;
    }

    /* Configure UART */
    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                                      ARM_USART_DATA_BITS_8 |
                                      ARM_USART_PARITY_NONE |
                                      ARM_USART_STOP_BITS_1 |
                                      ARM_USART_FLOW_CONTROL_NONE,
                                      115200);

    if(ret)
    {
        printf("\r\n Error in UART Control.\r\n");
        goto error_poweroff;
    }

    /* Enable Transmitter lines */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if(ret)
    {
        printf("\r\n Error in UART Control.\r\n");
        goto error_poweroff;
    }

    return ret;

error_poweroff:
    /* Received error Power off UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Power OFF.\r\n");
    }

error_uninitialize:
    /* Received error Un-initialize UART driver */
    ret = USARTdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Uninitialize.\r\n");
    }
    return ret;
}

/**
 * @function    void myHWSEM_app_entry()
 * @brief       TestApp to verify HWSEM interface
 *              Get the lock, send message through UART
 *              and then unlock
 * @note        none
 * @param       none
 * @retval      none
 */
void myHWSEM_app_entry()
{
    int32_t ret_hwsem, ret_uart, len;
    char uart_msg[30];


    /* Initialize HWSEM driver */
    ret_hwsem = HWSEMdrv->Initialize(myHWSEM_callback);

    if(ret_hwsem == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM initialization failed\r\n");
        goto error_initialize;
    }

    while(1)
    {
        /* Try to get lock */
        ret_hwsem = HWSEMdrv->Lock();

        if(ret_hwsem == ARM_DRIVER_ERROR)
        {
            printf("\r\n HWSEM lock failed\r\n");
            goto error_lock;
        }

    /* If hw semaphore is already locked, wait for interrupt */
        while(ret_hwsem == ARM_DRIVER_ERROR_BUSY)
        {
            /* Acquire the lock */
            if(event_flags_hwsem & HWSEM_CB_EVENT)
            {
                ret_hwsem = HWSEMdrv->Lock();
                event_flags_hwsem = 0;
                if (ret_hwsem == ARM_DRIVER_OK)
                    break;
            }
        }

        event_flags_uart &= ~UART_CB_TX_EVENT;

        ret_uart = USARTdrv->Send(acq_msg, strlen(acq_msg));
        if(ret_uart != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
        }

        /* wait for event flag after UART call */
        while(!(event_flags_uart & (UART_CB_TX_EVENT)));

        /* Print 10 messages from each core */
        for(int iter = 1; iter <= 10; iter++)
        {
            event_flags_uart &= ~UART_CB_TX_EVENT;

            len = sprintf(uart_msg, "%s %d\r\n", msg, iter);

            ret_uart = USARTdrv->Send(uart_msg, len);
            if(ret_uart != ARM_DRIVER_OK)
            {
                printf("\r\nError in UART Send.\r\n");
            }

            /* wait for event flag after UART call */
            while(!(event_flags_uart & (UART_CB_TX_EVENT)));
            PMU_delay_loop_us(500000);
        }

        event_flags_uart &= ~UART_CB_TX_EVENT;

        ret_uart = USARTdrv->Send(rel_msg, strlen(rel_msg));
        if(ret_uart != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
        }

        PMU_delay_loop_us(10000);
        /* wait for event flag after UART call */
        while(!(event_flags_uart & (UART_CB_TX_EVENT)));

        /* Unlock the hw semaphore */
        ret_hwsem = HWSEMdrv->UnLock();
        if(ret_hwsem != ARM_DRIVER_OK)
        {
            printf("\r\nError in HWSEM Unlock.\r\n");
            goto error_lock;
        }
        PMU_delay_loop_us(500000);
    }

    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();

error_lock:
    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();

error_initialize:
    return;
}

/* Define main entry point */
int main()
{
    int32_t ret_hwsem, ret_uart;
    ARM_DRIVER_VERSION version;

    version = HWSEMdrv->GetVersion();
    printf("\r\n HWSEM version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize the HWSEM Driver */
    ret_hwsem = HWSEMdrv->Initialize(myHWSEM_callback);

    if(ret_hwsem != ARM_DRIVER_OK)
    {
        printf("\r\n HWSEM initialization failed\r\n");
        goto error_initialize;
    }

    /* Try to get lock */
    ret_hwsem = HWSEMdrv->Lock();

    if(ret_hwsem == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM lock failed\r\n");
        goto error_lock;
    }

    /* If hw semaphore is already locked, wait for interrupt */
    while(ret_hwsem == ARM_DRIVER_ERROR_BUSY)
    {
        /* Acquire the lock */
        if(event_flags_hwsem & HWSEM_CB_EVENT)
        {
            ret_hwsem = HWSEMdrv->Lock();
            event_flags_hwsem = 0;
            if (ret_hwsem == ARM_DRIVER_OK)
                break;
        }
    }

    /* Initialize the UART Driver */
    ret_uart = hardware_init();

    if(ret_uart != 0)
    {
        printf("\r\n Error in UART hardware_init.\r\n");
        return -1;
    }

    event_flags_uart &= ~UART_CB_TX_EVENT;

    ret_uart = USARTdrv->Send(startup_msg, strlen(startup_msg));
    if(ret_uart != ARM_DRIVER_OK)
    {
        printf("\r\nError in UART Send.\r\n");
    }

    /* wait for event flag after UART call */
    while(!(event_flags_uart & (UART_CB_TX_EVENT)));

    /* Unlock the hw semaphore */
    ret_hwsem = HWSEMdrv->UnLock ();

    if(ret_hwsem == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM unlock failed\r\n");
        goto error_lock;
    }

    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();

    /* Call Application */
    myHWSEM_app_entry();
    return 0;

error_lock:
    HWSEMdrv->Uninitialize();
error_initialize:
    return -1;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
