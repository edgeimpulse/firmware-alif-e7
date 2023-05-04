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
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     23-April-2021
 * @brief    TestApp to verify UART interface using Threadx as an operating system.
 *           UART interactive console application (using UART4 instance):
 *             UART waits for a char on serial terminal;
 *               if 'Enter' key is received; UART Sends back "Hello World!".
 *               else, it sends back received character.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Includes --------------------------------------------------------------------------- */

/* System Includes */
#include <stdio.h>
#include "tx_api.h"

/* Project Includes */
/* include for UART Driver */
#include "Driver_USART.h"

/* PINMUX Driver */
#include "Driver_PINMUX_AND_PINPAD.h"

/* UART Driver instance (UART0-UART7) */
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

void myUART_Thread_entry(ULONG thread_input);

/* Define the ThreadX object control blocks...  */
#define DEMO_STACK_SIZE                        1024
#define DEMO_BYTE_POOL_SIZE                    9120
#define TX_UART_CB_EVENT                       0x01

TX_THREAD               UART_thread;
TX_BYTE_POOL            byte_pool_0;
UCHAR                   memory_area[DEMO_BYTE_POOL_SIZE];
TX_EVENT_FLAGS_GROUP    event_flags_uart;


/**
 * @function		int hardware_init(void)
 * @brief		UART hardware pin initialization using PIN-MUX driver
 * @note		none
 * @param		void
 * @retval		0:success, -1:failure
 */
int hardware_init(void)
{
	INT ret;

	/* PINMUX UART4_B */

	/* Configure GPIO Pin : P3_1 as UART4_RX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in PINMUX.\r\n");
		return -1;
	}

	/* Configure GPIO Pin : P3_2 as UART4_TX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in PINMUX.\r\n");
		return -1;
	}

	return 0;
}

/**
 * @function		void myUART_callback(UINT event)
 * @brief		UART isr callabck
 * @note		none
 * @param		event: USART Event
 * @retval		none
 */
void myUART_callback(UINT event)
{
	if (event & ARM_USART_EVENT_SEND_COMPLETE)
	{
		/* Send Success: Wake-up Thread. */
		tx_event_flags_set(&event_flags_uart, TX_UART_CB_EVENT, TX_OR);
	}

	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		/* Receive Success: Wake-up Thread. */
		tx_event_flags_set(&event_flags_uart, TX_UART_CB_EVENT, TX_OR);
	}

	if (event & ARM_USART_EVENT_RX_TIMEOUT)
	{
		/* Receive Success with rx timeout: Wake-up Thread. */
		tx_event_flags_set(&event_flags_uart, TX_UART_CB_EVENT, TX_OR);
	}
}

/**
 * @function    void myUART_Thread_entry(ULONG thread_input)
 * @brief       TestApp to verify UART interface
 *              UART interactive console application:
 *                UART waits for a char on serial terminal;
 *                 if 'Enter' key is received; UART Sends back "Hello World!".
 *                 else, it sends back received character.
 * @note        none
 * @param       thread_input : thread input
 * @retval      none
 */
void myUART_Thread_entry(ULONG thread_input)
{
	CHAR  cmd    = 0;
	INT   ret    = 0;
	ULONG events = 0;
	ARM_DRIVER_VERSION version;

	printf("\r\n >>> UART testApp starting up!!!...<<< \r\n");

	version = USARTdrv->GetVersion();
	printf("\r\n UART version api:%X driver:%X...\r\n",version.api, version.drv);

	/* Initialize UART hardware pins using PinMux Driver. */
	ret = hardware_init();
	if(ret != 0)
	{
		printf("\r\n Error in UART hardware_init.\r\n");
		return;
	}

	/* Initialize UART driver */
	ret = USARTdrv->Initialize(myUART_callback);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Initialize.\r\n");
		return;
	}

	/* Power up UART peripheral */
	ret = USARTdrv->PowerControl(ARM_POWER_FULL);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Power Up.\r\n");
		goto error_uninitialize;
	}

    /* Configure UART to 115200 Bits/sec */
    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                             ARM_USART_DATA_BITS_8       |
                             ARM_USART_PARITY_NONE       |
                             ARM_USART_STOP_BITS_1       |
                             ARM_USART_FLOW_CONTROL_NONE, 115200);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Control.\r\n");
		goto error_poweroff;
	}

	/* Enable Receiver and Transmitter lines */
	ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Control TX.\r\n");
		goto error_poweroff;
	}

	ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Control RX.\r\n");
		goto error_poweroff;
	}

	printf("\r\n Press Enter or any character on serial terminal to receive a message:\r\n");

	ret = USARTdrv->Send("\nPress Enter or any character to receive a message\n", 51);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Send.\r\n");
		goto error_poweroff;
	}

	/* wait till Send complete event comes in isr callback */
	ret = tx_event_flags_get(&event_flags_uart, TX_UART_CB_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
	if (ret != TX_SUCCESS)
	{
		printf(" \r\n Error in tx_event_flags_get.\r\n");
		goto error_poweroff;
	}

	while(1)
	{
		/* Get byte from UART */
		ret = USARTdrv->Receive(&cmd, 1);
		if(ret != ARM_DRIVER_OK)
		{
			printf("\r\n Error in UART Receive.\r\n");
			goto error_poweroff;
		}

		/* wait till Receive complete event comes in isr callback */
		ret = tx_event_flags_get(&event_flags_uart, TX_UART_CB_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
		if (ret != TX_SUCCESS)
		{
			printf(" \r\n Error in tx_event_flags_get.\r\n");
			goto error_poweroff;
		}

		if (cmd == 13) /* received 'Enter', send back "Hello World!". */
		{
			ret = USARTdrv->Send("\nHello World!\n", 14);
		}
		else /* else send back received character. */
		{
			ret = USARTdrv->Send(&cmd, 1);
		}

		/* wait till Send complete event comes in isr callback */
		ret = tx_event_flags_get(&event_flags_uart, TX_UART_CB_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
		if (ret != TX_SUCCESS)
		{
			printf(" \r\n Error in tx_event_flags_get.\r\n");
			goto error_poweroff;
		}
	}

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

	printf("\r\n XXX UART demo thread exiting XXX...\r\n");
}

/* Define main entry point.  */
int main()
{
	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();
}


/* Define what the initial system looks like.  */
void tx_application_define(void *first_unused_memory)
{
	CHAR    *pointer = TX_NULL;
	INT      status  = 0;

	/* Create a byte memory pool from which to allocate the thread stacks.  */
	status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);
	if (status != TX_SUCCESS)
	{
		printf("Could not create byte pool\n");
		return;
	}

	/* Create the event flags group used by UART thread */
	status = tx_event_flags_create(&event_flags_uart, "event flags UART");
	if (status != TX_SUCCESS)
	{
		printf("Could not create event flags\n");
		return;
	}

	/* Allocate the stack for thread.  */
	status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
	if (status != TX_SUCCESS)
	{
		printf("Could not allocate memory for thread stack.\n");
		return;
	}

	/* Create the main thread.  */
	status = tx_thread_create(&UART_thread, "UART_thread", myUART_Thread_entry, 0,
            pointer, DEMO_STACK_SIZE,
            1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
	if (status != TX_SUCCESS)
	{
		printf("Could not create uart demo thread \n");
		return;
	}

}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
