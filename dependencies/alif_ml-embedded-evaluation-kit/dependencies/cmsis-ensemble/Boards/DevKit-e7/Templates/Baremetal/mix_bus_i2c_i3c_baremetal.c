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
 * @file     mix_bus_i2c_i3c_testApp.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     23-May-2022
 * @brief    Baremetal testapp to verify Mix Bus i2c and i3c communication with
 *            multiple i2c + i3c slave devices using i3c IP
 *
 *           Select appropriate i3c Speed mode as per i2c or i3c slave device.
 *             I3C_BUS_MODE_PURE                             : Only Pure I3C devices
 *             I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS  : Fast Mode Plus   1 Mbps
 *             I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS : Fast Mode      400 Kbps
 *             I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS : Standard Mode  100 Kbps
 * @bug      None.
 * @Note     None.
 ******************************************************************************/


/* System Includes */
#include <stdio.h>
#include <string.h>

/* Project Includes */
/* I3C Driver */
#include "Driver_I3C.h"
#include "system_utils.h"

/* PINMUX Driver */
#include "Driver_PINMUX_AND_PINPAD.h"

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

/* i3c Driver instance 0 */
extern ARM_DRIVER_I3C Driver_I3C0;
static ARM_DRIVER_I3C *I3Cdrv = &Driver_I3C0;

void mix_bus_i2c_i3c_demo_entry();

/* i3c callback events */
typedef enum {
	I3C_CB_EVENT_SUCCESS   = (1 << 0),
	I3C_CB_EVENT_ERROR     = (1 << 1)
}I3C_CB_EVENTS;

volatile int32_t cb_event_flag = 0;

/**
  \fn          INT hardware_init(void)
  \brief       i3c hardware pin initialization:
                - PIN-MUX configuration
                - PIN-PAD configuration
  \param[in]   void
  \return      0:success; -1:failure
*/
int32_t hardware_init(void)
{
	int32_t ret = 0;

	/* i3c Pin-Mux */

	/* Configure GPIO Pin : P3_8 as I3C_SDA_B */
	ret = PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_8, PINMUX_ALTERNATE_FUNCTION_3);
	if(ret != 0)
	{
		printf("\r\n Error: PINMUX failed.\r\n");
		return -1;
	}

	/* Configure GPIO Pin : P3_9 as I3C_SCL_B */
	ret = PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_9, PINMUX_ALTERNATE_FUNCTION_4);
	if(ret != 0)
	{
		printf("\r\n Error: PINMUX failed.\r\n");
		return -1;
	}

	/* i3c Pin-Pad */

	/* Pin-Pad P3_8 as I3C_SDA_B
	 * Pad function: weak pull up(0x8) + read enable(0x01)
	 *               + Output drive strength 4mA(0x20)
	 */
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_8,  \
                          ( PAD_FUNCTION_READ_ENABLE                          |
                            PAD_FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP    |
                            PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS ) );
	if(ret != 0)
	{
		printf("\r\n Error: PINPAD failed.\r\n");
		return -1;
	}

	/* Pin-Pad P3_9 as I3C_SCL_B
	 * Pad function: weak pull up(0x8) + read enable(0x01)
	 *               + Output drive strength 4mA(0x20)
	 */
	ret = PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_9,  \
                          ( PAD_FUNCTION_READ_ENABLE                          |
                            PAD_FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP    |
                            PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS ) );
	if(ret != 0)
	{
		printf("\r\n Error: PINPAD failed.\r\n");
		return -1;
	}

	return 0;
}

/**
  \fn          void I3C_callback(uint32_t event)
  \brief       i3c isr callback
  \param[in]   event: i3c Event
  \return      none
*/
void I3C_callback(uint32_t event)
{
	if (event & ARM_I3C_EVENT_TRANSFER_DONE)
	{
		/* Transfer Success */
		cb_event_flag = I3C_CB_EVENT_SUCCESS;
	}

	if (event & ARM_I3C_EVENT_TRANSFER_ERROR)
	{
		/* Transfer Error */
		cb_event_flag = I3C_CB_EVENT_ERROR;
	}
}

/**
  \fn          void mix_bus_i2c_i3c_demo_entry()
  \brief       Baremetal testApp to verify mix bus i2c and i3c communication with
                multiple i2c + i3c slave devices using i3c IP.

               This demo function does:
                 - initialize i3c driver;
                 - set i3c speed mode to Mixed bus i2c/i3c Fast Mode 400 Kbps;
                 - assign dynamic address and attach all i3c slave devices to i3c;
                 - send/receive i3c CCC (Common Command Codes) only for i3c slaves
                 - attach all i2c slave devices to i3c;
                 - continuously read from specific register address(chip-id)
                    for all the attached slaves;
                 - display result depending on whether
                   slave has given ACK or NACK.
  \param[in]   none
  \return      none
*/
void mix_bus_i2c_i3c_demo_entry()
{

/* Maximum 8 Slave Devices are supported */
#define MAX_SLAVE_SUPPORTED   8

/* Added 3 slaves for demo purpose
 *   i3c : Accelerometer and Magnometer,
 *   i2c : EEPROM
 */
#define TOTAL_SLAVE           3

/* ICM-42670-P Accelerometer Slave address(On-chip attached to A1 Base Board) */
#define I3C_ACCERO_ADDR       0x68

/* MMC5633NJL Magnometer Slave address(On-chip attached to A1 Base Board) */
#define I3C_MAGNETO_ADDR      0x30

/* EEPROM Slave address(On-chip attached to MEIB Board) */
#define I2C_EEPROM_ADDR       0x50

/* ICM-42670-P Accelerometer Slave chip-id register(WHO AM I) address and value
 *  as per datasheet
 */
#define I3C_ACCERO_REG_WHO_AM_I_ADDR        0x75
#define I3C_ACCERO_REG_WHO_AM_I_VAL         0x67

/* MMC5633NJL Magnometer Slave chip-id register(Product ID 1) address and value
 *  as per datasheet
 */
#define I3C_MAGNETO_REG_PRODUCT_ID_1_ADDR   0x39
#define I3C_MAGNETO_REG_PRODUCT_ID_1_VAL    0x10

/* Any EEPROM Location and its value. */
#define I2C_EEPROM_LOCATION                 0x32
#define I2C_EEPROM_LOCATION_VALUE           0xCD

	uint32_t   i      = 0;
	uint32_t   len    = 0;
	uint32_t  timeout = 0;
	int32_t   ret     = 0;

    /* Array of slave address :
     *       Dynamic Address for i3c and
     *       Static  Address for i2c
     */
	uint8_t slave_addr[TOTAL_SLAVE] =
	{
		0, /* I3C Accero  Dynamic Address: To be updated later using MasterAssignDA */
		0, /* I3C Magneto Dynamic Address: To be updated later using MasterAssignDA */
		I2C_EEPROM_ADDR /* I2C EEPROM Slave Address. */
	};

	/* transmit data to i3c */
	uint8_t tx_data[TOTAL_SLAVE] =
	{
		I3C_ACCERO_REG_WHO_AM_I_ADDR,
		I3C_MAGNETO_REG_PRODUCT_ID_1_ADDR,
		I2C_EEPROM_LOCATION
	};

	/* receive data from i3c */
	uint8_t rx_data[1] = {0};

	/* actual receive data as per slave datasheet */
	uint8_t actual_rx_data[TOTAL_SLAVE] =
	{
		I3C_ACCERO_REG_WHO_AM_I_VAL,
		I3C_MAGNETO_REG_PRODUCT_ID_1_VAL,
		I2C_EEPROM_LOCATION_VALUE
	};


	ARM_DRIVER_VERSION version;

	/* I3C CCC (Common Command Codes) */
	I3C_CMD i3c_cmd;
	uint8_t i3c_cmd_tx_data[1] = {0x0F};
	uint8_t i3c_cmd_rx_data[6] = {0};

	/* i3c Magneto Slave 48-bit Provisional ID. */
	uint8_t i3c_magneto_PID[6] = {0x04, 0xA2, 0x00, 0x00, 0xF0, 0x00};


	printf("\r\n \t\t >>> mix bus i2c and i3c communication demo starting up!!! <<< \r\n");

	/* Get i3c driver version. */
	version = I3Cdrv->GetVersion();
	printf("\r\n i3c version api:0x%X driver:0x%X \r\n",  \
                           version.api, version.drv);

	/* Initialize i3c hardware pins using PinMux Driver. */
	ret = hardware_init();
	if(ret != 0)
	{
		printf("\r\n Error: i3c hardware_init failed.\r\n");
		return;
	}

	/* Initialize I3C driver */
	ret = I3Cdrv->Initialize(I3C_callback);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Initialize failed.\r\n");
		return;
	}

	/* Power up I3C peripheral */
	ret = I3Cdrv->PowerControl(ARM_POWER_FULL);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Power Up failed.\r\n");
		goto error_uninitialize;
	}

	/* i3c Speed Mode Configuration:
	 *  I3C_BUS_MODE_PURE                             : Only Pure I3C devices
	 *  I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS  : Fast Mode Plus   1 Mbps
	 *  I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS : Fast Mode      400 Kbps
	 *  I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS : Standard Mode  100 Kbps
	 */
	ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,  \
                      I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Control failed.\r\n");
		goto error_poweroff;
	}

	/* Delay for 1000 micro second.
	 *  @Note: Minor delay is required if prints are disable.
	 */
	PMU_delay_loop_us(1000);

	/* Attach all i3c slave using dynamic address */

	/* Assign Dynamic Address to i3c Accelerometer */
	printf("\r\n >> i3c: Get dynamic addr for static addr:0x%X.\r\n",I3C_ACCERO_ADDR);

	ret = I3Cdrv->MasterAssignDA(&slave_addr[0], I3C_ACCERO_ADDR);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C MasterAssignDA failed.\r\n");
		goto error_poweroff;
	}
	printf("\r\n >> i3c: Received dyn_addr:0x%X for static addr:0x%X. \r\n",   \
                                 slave_addr[0],I3C_ACCERO_ADDR);

	/* Assign Dynamic Address to i3c Magnometer */
	printf("\r\n >> i3c: Get dynamic addr for static addr:0x%X.\r\n",I3C_MAGNETO_ADDR);

	/* Delay for 1000 micro second.
	 *  @Note: Minor delay is required if prints are disable.
	 */
	PMU_delay_loop_us(1000);

	ret = I3Cdrv->MasterAssignDA(&slave_addr[1], I3C_MAGNETO_ADDR);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C MasterAssignDA failed.\r\n");
		goto error_poweroff;
	}

	printf("\r\n >> i3c: Received dyn_addr:0x%X for static addr:0x%X. \r\n",   \
                                 slave_addr[1],I3C_MAGNETO_ADDR);

	/* Delay for n micro second. */
	PMU_delay_loop_us(1000);

	/* demo for I3C CCC (Common Command Codes) APIs */

	/* write I3C_CCC_SETMWL (Set Max Write Length) command to Accelerometer slave */
	i3c_cmd.rw     = 0;
	i3c_cmd.cmd_id = I3C_CCC_SETMWL(false);
	i3c_cmd.len    = 1;
	i3c_cmd.addr   = slave_addr[0];
	i3c_cmd.data   = i3c_cmd_tx_data;

	ret = I3Cdrv->MasterSendCommand(&i3c_cmd);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C MasterSendCommand failed.\r\n");
		goto error_detach;
	}

	/* Delay for 1000 micro second. */
	PMU_delay_loop_us(1000);

	/* read I3C_CCC_GETMWL (Get Max Write Length) command from Accelerometer slave */
	i3c_cmd.rw     = 1;
	i3c_cmd.cmd_id = I3C_CCC_GETMWL;
	i3c_cmd.len    = 1;
	i3c_cmd.addr   = slave_addr[0];
	i3c_cmd.data   = i3c_cmd_rx_data;

	ret = I3Cdrv->MasterSendCommand(&i3c_cmd);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C MasterSendCommand failed.\r\n");
		goto error_detach;
	}

	/* Delay for 1000 micro second. */
	PMU_delay_loop_us(1000);

	/* read I3C_CCC_GETPID (Get Provisional ID 48-bit) command from Magneto slave */
	i3c_cmd.rw     = 1;
	i3c_cmd.cmd_id = I3C_CCC_GETPID;
	i3c_cmd.len    = 6;
	i3c_cmd.addr   = slave_addr[1];
	i3c_cmd.data   = i3c_cmd_rx_data;

	ret = I3Cdrv->MasterSendCommand(&i3c_cmd);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C MasterSendCommand failed.\r\n");
		goto error_detach;
	}

	/* Delay for 1000 micro second. */
	PMU_delay_loop_us(1000);

	/* compare received 48-bit Provisional ID with actual for Magneto slave */
	if( memcmp(i3c_cmd_rx_data, i3c_magneto_PID, 6) == 0 )
	{
		printf("\r\n \t\t >> i3c magneto PID is VALID.\r\n");
	}
	else
	{
		printf("\r\n \t\t >> i3c magneto PID is INVALID.\r\n");
	}

	/* Delay for 1000 micro second. */
	PMU_delay_loop_us(1000);

	/* Attach all i2c slave using static address */
	printf("\r\n >> i2c: Attaching i2c slave addr:0x%X to i3c...\r\n",slave_addr[2]);

	ret = I3Cdrv->AttachI2Cdev(slave_addr[2]);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Attach I2C device failed.\r\n");
		goto error_poweroff;
	}


	/*
	 * @Note:
	 *  How much data(register address + actual data) user has to Transmit/Receive ?
	 *   it depends on Slave's register address location bytes.
	 *
	 *  Generally, Camera Slave supports       16-bit(2 Byte) reg-addr and (8/16/32 bit) data
	 *   Others Accero/Magneto/EEPROM supports  8-bit(1 Byte) reg-addr and (8/16/32 bit) data
	 *
	 *  First LSB[7-0] will be added to TX FIFO and first transmitted on the i3c bus;
	 *   remaining bytes will be added in LSB -> MSB order.
	 *
	 *  For Slave who supports 16-bit(2 Byte) register address and data:
	 *   Register Address[15:8] : Needs to be Transmit First  to the i3c
	 *   Register Address[07:0] : Needs to be Transmit Second to the i3c
	 *
	 *  That means,
	 *
	 *  While transmitting to TX FIFO,
	 *   MSB of TX data needs to be added first to the TX FIFO.
	 *
	 *  While receiving from RX FIFO,
	 *   First MSB will be received from RX FIFO.
	 *
	 *  START          I3C FIFO           END
	 *  MSB                               LSB
	 *  24-31 bit | 16-23 bit | 8-15 bit | 0-7 bit
	 *
	 *  So, USER has to modify
	 *  Transmit/Receive data (Little Endian <-> Big Endian and vice versa)
	 *  before sending/after receiving to/from i3c TX/RX FIFO.
	 */


	/* Let's Continuously read from chip-id register address for
	 *  all the attached slaves and display received data depending on
	 *  whether slave has given ACK or NACK.
	*/
	while(1)
	{
		for(i=0; i<TOTAL_SLAVE; i++)
		{
			/* To Read from any register address:
			 *  First write register address using MasterTransmit and
			 *   then Read data using MasterReceive
			 */

			/* TX/RX length is 1 Byte
			 * (assume slave requires 8-bit data for TX/RX).
			 */
			len = 1;

			printf("\r\n ------------------------------------------------------------ \r\n");
			printf("\r\n >> i=%d TX slave addr:0x%X reg_addr:[0]0x%X \r\n",  \
                                 i, slave_addr[i], tx_data[i]);

			/* Delay for 1000 micro second. */
			PMU_delay_loop_us(1000);

			/* For TX, User has to pass
			 * Slave Address + TX data + length of the TX data.
			 */
			cb_event_flag =0;

			ret = I3Cdrv->MasterTransmit(slave_addr[i], &tx_data[i], len);
			if(ret != ARM_DRIVER_OK)
			{
				printf("\r\n Error: I3C Master Transmit failed. \r\n");
				goto error_detach;
			}

			/* wait till any event success/error comes in isr callback */
			timeout = 100;
			while (timeout--)
			{
				/* Delay for 1000 micro second. */
				PMU_delay_loop_us(1000);

				if(cb_event_flag == I3C_CB_EVENT_SUCCESS)
				{
					printf("\r\n \t\t >> i=%d TX Success: Got ACK from slave addr:0x%X.\r\n",  \
					                               i, slave_addr[i]);
					break;
				}
				if(cb_event_flag == I3C_CB_EVENT_ERROR)
				{
					/* TX Error: Got NACK from slave */
					printf("\r\n \t\t >> i=%d TX Error: Got NACK from slave addr:0x%X \r\n",  \
					                               i, slave_addr[i]);
					break;
				}
			}

			if ( (!(timeout)) && (!(cb_event_flag)) )
			{
				printf("Error: event timeout \r\n");
				goto error_detach;
			}

			/* RX */
			printf("\r\n\r\n >> i=%d RX slave addr:0x%X \r\n",i, slave_addr[i]);

			/* clear rx data buffer. */
			rx_data[0] = 0;

			/* TX/RX length is 1 Byte
			 * (assume slave requires 8-bit data for TX/RX).
			 */
			len = 1;

			/* For RX, User has to pass
			 * Slave Address + Pointer to RX data + length of the RX data.
			 */
			cb_event_flag =0;

			ret = I3Cdrv->MasterReceive(slave_addr[i], rx_data, len);
			if(ret != ARM_DRIVER_OK)
			{
				printf("\r\n Error: I3C Master Receive failed. \r\n");
				goto error_detach;;
			}

			/* wait till any event success/error comes in isr callback */
			timeout =100;
			while (timeout--)
			{
				/* Delay for 1000 micro second. */
				PMU_delay_loop_us(1000);

				if(cb_event_flag == I3C_CB_EVENT_SUCCESS)
				{
					/* RX Success: Got ACK from slave */
					printf("\r\n \t\t >> i=%d RX Success: Got ACK from slave addr:0x%X.\r\n",  \
					                               i, slave_addr[i]);
					printf("\r\n \t\t >> i=%d RX Received Data from slave:[0]0x%X. actual data:0x%X\r\n",  \
					                               i,rx_data[0],actual_rx_data[i]);

				if(rx_data[0] == actual_rx_data[i])
				{
					printf("\r\n \t\t >> i=%d RX Received Data from slave is VALID.\r\n",i);
				}
				else
				{
					printf("\r\n \t\t >> i=%d RX Received Data from slave is INVALID.\r\n",i);
				}

				break;
				}

				if(cb_event_flag == I3C_CB_EVENT_ERROR)
				{
					/* RX Error: Got NACK from slave */
					printf("\r\n \t\t >> i=%d RX Error: Got NACK from slave addr:0x%X \r\n",  \
					                               i, slave_addr[i]);
					break;
				}
			}

			if ( (!(timeout)) && (!(cb_event_flag)) )
			{
				printf("Error: event timeout \r\n");
				goto error_detach;
			}

			printf("\r\n ---------------------------XXX------------------------------ \r\n");
		}
	}


error_detach:

	/* Detach all attached i2c/i3c slave device. */
	for(i=0; i<TOTAL_SLAVE; i++)
	{
		printf("\r\n i=%d detaching i2c or i3c slave addr:0x%X from i3c.\r\n",i, slave_addr[i]);
		ret = I3Cdrv->Detachdev(slave_addr[i]);
		if(ret != ARM_DRIVER_OK)
		{
			printf("\r\n Error: I3C Detach device failed.\r\n");
		}
	}

error_poweroff:

	/* Power off I3C peripheral */
	ret = I3Cdrv->PowerControl(ARM_POWER_OFF);
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Power OFF failed.\r\n");
	}

error_uninitialize:

	/* Un-initialize I3C driver */
	ret = I3Cdrv->Uninitialize();
	if(ret != ARM_DRIVER_OK)
	{
		printf("\r\n Error: I3C Uninitialize failed.\r\n");
	}

	printf("\r\n XXX I3C demo testapp exiting XXX...\r\n");
}

/* Define main entry point.  */
int main()
{
	mix_bus_i2c_i3c_demo_entry();
	return 0;

}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
