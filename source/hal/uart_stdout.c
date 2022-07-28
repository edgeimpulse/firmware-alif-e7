/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "uart_stdout.h"
#include "Driver_PINMUX_AND_PINPAD.h"
#include "Driver_USART.h"

#include <stdio.h>
#include <stdint.h>

#define CNTLQ       0x11
#define CNTLS       0x13
#define DEL         0x7F
#define BACKSPACE   0x08
#define CR          0x0D
#define LF          0x0A
#define ESC         0x1B

#define UARTBASE    (PL011_UART0_BASE)

/*****************************************************************************/
/*  UART Control Register Locations                                          */
/*****************************************************************************/
#define UART0_DR   *((volatile unsigned *) UARTBASE)
#define UART0_RSR  *((volatile unsigned *)(UARTBASE + 0x04))
#define UART0_ECR  *((volatile unsigned *)(UARTBASE + 0x04))
#define UART0_LCRH *((volatile unsigned *)(UARTBASE + 0x2C))
#define UART0_LCRM *((volatile unsigned *)(UARTBASE + 0x28))
#define UART0_LCRL *((volatile unsigned *)(UARTBASE + 0x24))
#define UART0_CR   *((volatile unsigned *)(UARTBASE + 0x30))
#define UART0_FR   *((volatile unsigned *)(UARTBASE + 0x18))
#define UART0_IIR  *((volatile unsigned *)(UARTBASE + 0x1C))
#define UART0_ICR  *((volatile unsigned *)(UARTBASE + 0x44))

/*****************************************************************************/
/* Received Status Register - RSR                                            */
/*****************************************************************************/
#define RSR_OVERRUN_ERROR   0x08
#define RSR_BREAK_ERROR     0x04
#define RSR_PARITY_ERROR    0x02
#define RSR_FRAMING_ERROR   0x01

/*****************************************************************************/
/* Line Control High Byte Register - LCRH                                    */
/*****************************************************************************/
#define LCRH_WORD_LENGTH_8  0x60
#define LCRH_WORD_LENGTH_7  0x40
#define LCRH_WORD_LENGTH_6  0x20
#define LCRH_WORD_LENGTH_5  0x00
#define LCRH_FIFO_ENABLED   0x10
#define LCRH_2_STOP_BITS    0x08
#define LCRH_EVEN_PARITY    0x04
#define LCRH_PARITY_ENABLE  0x02
#define LCRH_SEND_BREAK     0x01

/*****************************************************************************/
/* Line Control Medium Byte Register - LCRM                                  */
/* This register specifies the high byte of the Baud rate divisor            */
/*****************************************************************************/
#define LCRM_BAUD_460800  0x00
#define LCRM_BAUD_230400  0x00
#define LCRM_BAUD_115200  0x00
#define LCRM_BAUD_76800   0x00
#define LCRM_BAUD_57600   0x00
#define LCRM_BAUD_38400   0x00
#define LCRM_BAUD_19200   0x00
#define LCRM_BAUD_14400   0x00
#define LCRM_BAUD_9600    0x00
#define LCRM_BAUD_2400    0x01
#define LCRM_BAUD_1200    0x02

/*****************************************************************************/
/* Line Control Low Byte Register - LCRL                                     */
/* This register specifies the low byte of the Baud rate divisor             */
/*****************************************************************************/
#define LCRL_BAUD_460800  0x01
#define LCRL_BAUD_230400  0x03
#define LCRL_BAUD_115200  0x07
#define LCRL_BAUD_76800   0x0B
#define LCRL_BAUD_57600   0x0F
#define LCRL_BAUD_38400   0xC
#define LCRL_BAUD_19200   0x2F
#define LCRL_BAUD_14400   0x3F
#define LCRL_BAUD_9600    0x5F
#define LCRL_BAUD_2400    0x7F
#define LCRL_BAUD_1200    0xFF

/*****************************************************************************/
/* Control Register - CR                                                     */
/*****************************************************************************/
#define CR_LOOP_BACK_EN   0x80
#define CR_TIMEOUT_INT_EN 0x40
#define CR_TX_INT_ENABLE  0x100
#define CR_RX_INT_ENABLE  0x200
#define CR_MODSTAT_INT_EN 0x08
#define CR_UART_ENABLE    0x01

/*****************************************************************************/
/* Flag Register - FR                                                        */
/*****************************************************************************/
#define FR_TX_FIFO_EMPTY  0x80
#define FR_RX_FIFO_FULL   0x40
#define FR_TX_FIFO_FULL   0x20
#define FR_RX_FIFO_EMPTY  0x10
#define FR_BUSY           0x08
#define FR_CARRIER_DETECT 0x04
#define FR_SET_READY      0x02
#define FR_CLEAR_TO_SEND  0x01

/*****************************************************************************/
/* Interrupt Identification Register - IIR                                   */
/*****************************************************************************/
#define IIR_RX_TIME_OUT   0x08
#define IIR_TX            0x04
#define IIR_RX            0x02
#define IIR_MODEM         0x01

// <h>STDOUT USART Interface

//   <o>Connect to hardware via Driver_USART# <0-255>
//   <i>Select driver control block for USART interface
#define USART_DRV_NUM           2

//   <o>Baudrate
#define USART_BAUDRATE          115200

// </h>


#define _USART_Driver_(n)  Driver_USART##n
#define  USART_Driver_(n) _USART_Driver_(n)

extern ARM_DRIVER_USART  USART_Driver_(USART_DRV_NUM);
#define ptrUSART       (&USART_Driver_(USART_DRV_NUM))

/**
  Initialize pinmux

  \return          0 on success, or -1 on error.
*/
static int usart_pinmux_init(void)
{
	int32_t ret;
	uint32_t port_config = PAD_FUNCTION_READ_ENABLE				|
						   PAD_FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP;

#if USART_DRV_NUM == 1	/* Rev A0 - DevKit Alpha */
	/* PINMUX UART1_A */
	/* Configure GPIO Pin : P1_4 as UART1_RX_A */
	ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_4, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_4, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}

	/* Configure GPIO Pin : P1_5 as UART1_TX_A */
	ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_5, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_5, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
#elif USART_DRV_NUM == 2	/* Rev A1 - DevKit Beta */
	/* PINMUX UART2_B */
	/* Configure GPIO Pin : P3_16 as UART2_RX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_16, PINMUX_ALTERNATE_FUNCTION_2);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_16, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}

	/* Configure GPIO Pin : P3_17 as UART2_TX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_17, PINMUX_ALTERNATE_FUNCTION_2);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_17, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
#elif USART_DRV_NUM == 4	/* Rev A1 - DevKit Beta */
	/* PINMUX UART4_B */
	/* Configure GPIO Pin : P3_1 as UART4_RX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_1, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}

	/* Configure GPIO Pin : P3_2 as UART4_TX_B */
	ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_1);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_2, port_config);
	if(ret != ARM_DRIVER_OK) {
		return ret;
	}
#endif

	return ret;
}

void usart_callback(uint32_t event)
{
	if (event & ARM_USART_EVENT_SEND_COMPLETE)
	{
		/* Send Success */
	}

	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		/* Receive Success */
	}

	if (event & ARM_USART_EVENT_RX_TIMEOUT)
	{
		/* Receive Success with rx timeout */
	}
}

int32_t UartStdOutInit(void)
{
  int32_t status;

  status = usart_pinmux_init();

  status += ptrUSART->Initialize(usart_callback);

  status += ptrUSART->PowerControl(ARM_POWER_FULL);

  status += ptrUSART->Control(ARM_USART_MODE_ASYNCHRONOUS |
                             ARM_USART_DATA_BITS_8       |
                             ARM_USART_PARITY_NONE       |
                             ARM_USART_STOP_BITS_1       |
                             ARM_USART_FLOW_CONTROL_NONE,
                             USART_BAUDRATE);

  status += ptrUSART->Control(ARM_USART_CONTROL_TX, 1);
  status += ptrUSART->Control(ARM_USART_CONTROL_RX, 1);

  return status;
}

unsigned char UartPutc(unsigned char ch)
{
  uint8_t buf[1];
  buf[0] = ch;

  if (ptrUSART->Send(buf, 1) != ARM_DRIVER_OK) {
    return (-1);
  }
  while (ptrUSART->GetTxCount() != 1);

  if (buf[0] == '\n') {
	//   buf[0] = '\r';
	  if (ptrUSART->Send(buf, 1) != ARM_DRIVER_OK) {
		return (-1);
	  }
	  while (ptrUSART->GetTxCount() != 1);
  }

  return (ch);
}

unsigned char UartGetc(void)
{
	uint8_t buf[1];

	if (ptrUSART->Receive(buf, 1) != ARM_DRIVER_OK)
	{
		return (-1);
	}
	while (ptrUSART->GetRxCount() != 1)
		;
	return (buf[0]);
}

unsigned char UartGetcNoBlock(void)
{
	static uint8_t buf[1];
	static bool buffer_queued = false;

	if (!buffer_queued)
	{
		if (ptrUSART->Receive(buf, 1) != ARM_DRIVER_OK)
		{
			return (-1);
		} else {
			buffer_queued = true;
		}
	}
	if (ptrUSART->GetRxCount() == 0) {
		return -1;
	} else {
		buffer_queued = false;
		return (buf[0]);
	}
}


bool GetLine (char *lp, unsigned int len)
{
   unsigned int cnt = 0;
   char c;

   do {
       c = UartGetc();
       switch (c) {
           case CNTLQ:                       /* ignore Control S/Q.            */
           case CNTLS:
               break;
           case BACKSPACE:
           case DEL:
               if (cnt == 0) {
                   break;
               }
               cnt--;                         /* decrement count.               */
               lp--;                          /* and line pointer.              */
               UartPutc (0x08);               /* echo backspace.                */
               UartPutc (' ');
               UartPutc (0x08);
               fflush (stdout);
               break;
           case ESC:
           case 0:
               *lp = 0;                       /* ESC - stop editing line.       */
               return false;
           case CR:                            /* CR - done, stop editing line. */
               UartPutc (*lp = c);             /* Echo and store character.     */
               lp++;                           /* Increment line pointer        */
               cnt++;                          /* and count.                    */
               c = LF;
               UartPutc (*lp = c);             /* Echo and store character.      */
               fflush (stdout);
               lp++;                           /* Increment line pointer         */
               break;
           default:
               UartPutc (*lp = c);            /* echo and store character.      */
               fflush (stdout);
               lp++;                          /* increment line pointer.        */
               cnt++;                         /* and count.                     */
               break;
       }
   } while (cnt < len - 2  &&  c != LF);      /* check limit and CR.            */
   *lp = 0;                                   /* mark end of string.            */
    return true;
}

__attribute__((noreturn)) void UartEndSimulation(int code)
{
//    UartPutc((char) 0x4);  // End of simulation
//    UartPutc((char) code); // Exit code
    while(1);
}
