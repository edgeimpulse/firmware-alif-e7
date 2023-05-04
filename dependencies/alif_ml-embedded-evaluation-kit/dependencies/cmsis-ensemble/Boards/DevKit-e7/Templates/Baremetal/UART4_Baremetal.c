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
 * @file     UART4_Baremetal.c
 * @author   Ahmad Rashed
 * @email    ahmad.rashed@alifsemi.com
 * @version  V1.0.0
 * @date     11-January-2022
 * @brief    TestApp to verify UART interface using baremetal, no operating system.
 *           UART interactive console application (using UART4 instance):
 *             UART waits for a char on serial terminal;
 *               if 'Enter' key is received; UART Sends back "Hello World!".
 *               else, it sends back received character.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <RTE_Components.h>
#include CMSIS_device_header

/* System Includes */
#include <stdio.h>

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
  void _sys_exit(int return_code) { while (1); }
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

void myUART_Thread_entry();

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2
volatile uint32_t event_flags_uart;


/**
 * @function    int hardware_init(void)
 * @brief       UART hardware pin initialization using PIN-MUX driver
 * @note        none
 * @param       void
 * @retval      0:success, -1:failure
 */
int hardware_init(void)
{
	int32_t ret;

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
 * @function    void myUART_Thread_entry()
 * @brief       TestApp to verify UART interface
 *              UART interactive console application:
 *                  UART waits for a char on serial terminal;
 *                  if 'Enter' key is received; UART Sends back "Hello World!".
 *                  else, it sends back received character.
 * @note        none
 * @param       none
 * @retval      none
 */
void myUART_Thread_entry()
{
	char cmd;
	int32_t ret;
	ARM_DRIVER_VERSION version;
	event_flags_uart = 0;

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

	event_flags_uart &= ~UART_CB_TX_EVENT;
	ret = USARTdrv->Send("\r\nPress Enter or any character to receive a message\r\n", 53);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error in UART Send.\r\n");
		goto error_poweroff;
	}

	/* wait for event flag after UART call */
	while(!(event_flags_uart & (UART_CB_TX_EVENT)));

	while(1)
	{
		/* Get byte from UART - clear event flag before UART call */
		event_flags_uart &= ~(UART_CB_RX_EVENT | UART_CB_RX_TIMEOUT);
		ret = USARTdrv->Receive(&cmd, 1);
		if(ret != ARM_DRIVER_OK)
		{
			printf("\r\n Error in UART Receive.\r\n");
			goto error_poweroff;
		}

		/* wait for event flag after UART call */
		while(!(event_flags_uart & (UART_CB_RX_EVENT | UART_CB_RX_TIMEOUT)));

		if (event_flags_uart & (UART_CB_RX_EVENT))
		{
			/* Write byte(s) to UART - again, set event flag before UART call */
			event_flags_uart &= ~UART_CB_TX_EVENT;
			if (cmd == 13)
			{
				/* received 'Enter', send back "Hello World!". */
				ret = USARTdrv->Send("\r\nHello World!\r\n", 16);
			}
			else
			{
				/* else send back received character. */
				ret = USARTdrv->Send(&cmd, 1);
			}

			/* wait for event flag after UART call */
			while(!(event_flags_uart & (UART_CB_TX_EVENT)));
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

/* Define main entry point */
int main()
{
	hardware_init();
	myUART_Thread_entry();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
