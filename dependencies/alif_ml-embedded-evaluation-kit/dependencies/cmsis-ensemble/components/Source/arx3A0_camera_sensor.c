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
 * @file     arx3A0_camera_sensor.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     1-April-2022
 * @brief    ONsemi ARX3A0 Camera Sensor driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* Proceed only if ARX3A0 Camera Sensor is enabled. */
#if RTE_ARX3A0_CAMERA_SENSOR_ENABLE

/* Project Includes */
#include "arx3A0_camera_sensor.h"
#include "Driver_GPIO.h"
#include "Driver_Camera_Controller.h"

#if (RTE_ARX3A0_CAMERA_RESET_GPIO_PORT == 1)
extern ARM_DRIVER_GPIO Driver_GPIO1;
static ARM_DRIVER_GPIO *GPIO_Driver_CAM = &Driver_GPIO1;
#elif (RTE_ARX3A0_CAMERA_RESET_GPIO_PORT == 2)
extern ARM_DRIVER_GPIO Driver_GPIO2;
static ARM_DRIVER_GPIO *GPIO_Driver_CAM = &Driver_GPIO2;
#elif (RTE_ARX3A0_CAMERA_RESET_GPIO_PORT == 3)
extern ARM_DRIVER_GPIO Driver_GPIO3;
static ARM_DRIVER_GPIO *GPIO_Driver_CAM = &Driver_GPIO3;
#elif (RTE_ARX3A0_CAMERA_RESET_GPIO_PORT == 4)
extern ARM_DRIVER_GPIO Driver_GPIO4;
static ARM_DRIVER_GPIO *GPIO_Driver_CAM = &Driver_GPIO4;
#else
#error "GPIO Port for Camera Reset not found"
#endif

/**
  \brief ARX3A0 Camera Sensor slave i2c Configuration
  \ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
  */
static CAMERA_SENSOR_SLAVE_I2C_CONFIG arx3A0_camera_sensor_i2c_cnfg =
{
	.I3Cx_instance                  = RTE_ARX3A0_CAMERA_SENSOR_I2C_USING_I3Cx_INSTANCE,
	.speed_mode                     = CAMERA_SENSOR_I2C_SPEED_SS_100_KBPS,
	.cam_sensor_slave_addr          = ARX3A0_CAMERA_SENSOR_SLAVE_ADDR,
	.cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide delay using PMU(Performance Monitoring Unit).
 */
#define ARX3A0_DELAY_uSEC(usec)       PMU_delay_loop_us(usec)

/* Wrapper function for i2c read
 *  read register value from ARX3A0 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for ARX3A0 Camera Sensor specific i2c configurations
 *   see \ref ARX3A0_camera_sensor_i2c_cnfg
 */
#define ARX3A0_READ_REG(reg_addr, reg_value, reg_size) \
	camera_sensor_i2c_read(&arx3A0_camera_sensor_i2c_cnfg, \
			reg_addr,  \
			reg_value, \
			(CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to ARX3A0 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for ARX3A0 Camera Sensor specific i2c configurations
 *   see \ref ARX3A0_camera_sensor_i2c_cnfg
 */
#define ARX3A0_WRITE_REG(reg_addr, reg_value, reg_size) \
	camera_sensor_i2c_write(&arx3A0_camera_sensor_i2c_cnfg, \
			reg_addr,  \
			reg_value, \
			(CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/**
  \fn           int32_t ARX3A0_Bulk_Write_Reg(const ARX3A0_REG arx3A0_reg[],
  uint32_t total_num)
  \brief        write array of registers value to ARX3A0 Camera Sensor registers.
  \param[in]    arx3A0_reg : ARX3A0 Camera Sensor Register Array Structure
  \ref ARX3A0_REG
  \param[in]    total_num   : total number of registers(size of array)
  \return       \ref execution_status
  */
static int32_t ARX3A0_Bulk_Write_Reg(const ARX3A0_REG arx3A0_reg[],
		uint32_t total_num, uint32_t reg_size)
{
	uint32_t i  = 0;
	int32_t ret = 0;

	for(i = 0; i < total_num; i++)
	{
		ret = ARX3A0_WRITE_REG(arx3A0_reg[i].reg_addr, arx3A0_reg[i].reg_value, \
				reg_size);
		if(ret != ARM_DRIVER_OK)
			return ret;
	}

	return ARM_DRIVER_OK;
}

/**
  \fn           void ARX3A0_Sensor_Clk_Src(void)
  \brief        ARX3A0 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void ARX3A0_Sensor_Clk_Src(void)
{
	volatile uint32_t *reg = NULL;

	reg = (uint32_t *)CFGSLV1_BASE;
	*reg &= ~(ARX3A0_CLK_DIV_Msk);
	*reg |= _VAL2FLD(ARX3A0_CLK_DIV, RTE_ARX3A0_CAMERA_SENSOR_CLK_SCR_DIV);
	SET_BIT(*reg, ARX3A0_CLK_EN_Msk);
}



/**
  \fn           int32_t ARX3A0_Camera_Hard_Reseten(void)
  \brief        Hard Reset ARX3A0 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t ARX3A0_Camera_Hard_Reseten(void)
{
	int32_t ret = 0;

	if(GPIO_Driver_CAM == NULL)
		return ARM_DRIVER_ERROR_PARAMETER;

	ret = GPIO_Driver_CAM->Initialize(RTE_ARX3A0_CAMERA_RESET_PIN_NO,NULL);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ret = GPIO_Driver_CAM->PowerControl(RTE_ARX3A0_CAMERA_RESET_PIN_NO,  ARM_POWER_FULL);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ret = GPIO_Driver_CAM->SetDirection(RTE_ARX3A0_CAMERA_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ret = GPIO_Driver_CAM->SetValue(RTE_ARX3A0_CAMERA_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ARX3A0_DELAY_uSEC(2000);

	ret = GPIO_Driver_CAM->SetValue(RTE_ARX3A0_CAMERA_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ARX3A0_DELAY_uSEC(2000);

	ret = GPIO_Driver_CAM->SetValue(RTE_ARX3A0_CAMERA_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ARX3A0_DELAY_uSEC(100000);

	return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ARX3A0_Camera_Soft_Reseten(void)
  \brief        Software Reset ARX3A0 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t ARX3A0_Camera_Soft_Reseten(void)
{
	int32_t ret = 0;

	ret = ARX3A0_WRITE_REG(ARX3A0_SOFTWARE_RESET_REGISTER, 0x01, 1);
	if(ret != ARM_DRIVER_OK)
		return ret;

	/* @Observation: more delay is required for Camera Sensor
	 *               to setup after Soft Reset.
	 */
	ARX3A0_DELAY_uSEC(1000);

	return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ARX3A0_Camera_Cfg(ARM_CAMERA_RESOLUTION cam_resolution)
  \brief        Initialize ARX3A0 Camera Sensor.
  this function will
  - configure Camera Sensor resolution registers as per input parameter.
  (currently supports only 560x560(WxH) Camera resolution)
  - configure Camera Sensor slew rate.
  \param[in]    cam_resolution    : Camera Sensor Resolution
  \ref ARX3A0_Camera_Resolution

  \return       \ref execution_status
  */
static int32_t ARX3A0_Camera_Cfg(ARM_CAMERA_RESOLUTION cam_resolution)
{
	uint32_t total_num     = 0;

	/* Configure Camera Sensor Resolution */
	if(cam_resolution == CAMERA_RESOLUTION_560x560)
	{
		/* Camera Sensor Resolution: 560x560(WxH) */
		total_num = (sizeof(arx3a0_560_regs) / sizeof(ARX3A0_REG));
		return ARX3A0_Bulk_Write_Reg(arx3a0_560_regs, total_num, 2);
	}
	else
	{
		return ARM_DRIVER_ERROR_PARAMETER;
	}

	return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ARX3A0_Camera_Gain(uint32_t gain)
  \brief        Set camera gain
  this function will
  - configure Camera Sensor gain as per input parameter.
  (currently supports only analogue gain control)
  \param[in]    gain    : gain value * 65536 (so 1.0 gain = 65536); 0 to read

  \return       \ref actual gain
  */
static int32_t ARX3A0_Camera_Gain(const uint32_t gain)
{
	uint32_t digital_gain;
	uint32_t fine_gain = gain;
	uint32_t coarse_gain;
	if (gain != 0)
	{
		/* From the Design Guide:
		 *
		 * Total Gain = (R0x305E[0:3]/16+1)*2^R0x0305E[4:6]*(R0x305E[7:15]/64)
		 *
		 * (Register guide is misleading, no doubt in part due to the backwards-
		 * compatibility analogue gain API in register 0x3028).
		 *
		 * First clamp analogue gain, using digital gain to get more if
		 * necessary. Otherwise digital gain is used to fine adjust.
		 */
		if (gain < 0x10000)
		{
			/* Minimum gain is 1.0 */
			fine_gain = 0x10000;
		}
		else if (gain > 0x80000)
		{
			/* Maximum analogue gain is 8.0 */
			fine_gain = 0x80000;
		}

		/*
		 * First get coarse analogue power of two, leaving fine gain in range [0x10000,0x1FFFF]
		 */
		coarse_gain = 0;
		while (fine_gain >= 0x20000)
		{
			coarse_gain++;
			fine_gain /= 2;
		}

		/* And we then have 16 steps of fine gain. Round down here, and
		 * we tune with digital gain >= 1.0
		 */
		fine_gain = (fine_gain - 0x10000) / 0x1000;

		/* Use digital gain to extend gain beyond the analogue limits of
		 * x1 to x8, or to fine-tune within that range.
		 *
		 * We don't let digital gain go below 1.0 - it just loses information,
		 * and clamping it lets an auto-gain controller see that we are
		 * unable to improve exposure by such lowering. Another camera might
		 * be able to usefully set gain to <1.0, so a controller could try it.
		 *
		 * (When we're fine tuning, digital gain is always >=1.0, because we
		 * round down analogue gain, so it can only go below 1.0 by the user
		 * requesting total gain < 1.0).
		 */
		uint32_t resulting_analogue_gain = ((fine_gain + 16) << coarse_gain) * 0x1000;
		digital_gain = (64 * gain + (resulting_analogue_gain / 2)) / resulting_analogue_gain;
		if (digital_gain > 0x1FF)
		{
			/* Maximum digital gain is just under 8.0 (limited by register size) */
			digital_gain = 0x1FF;
		}
		else if (digital_gain < 64)
		{
			/* Digital gain >= 1.0, as per discussion above */
			digital_gain = 64;
		}
		int32_t ret = ARX3A0_WRITE_REG(0x305e, (digital_gain << 7) | (coarse_gain << 4) | fine_gain, 2);
		if (ret != 0)
			return ret;
	}
	else
	{
		uint32_t reg_value;
		int32_t ret = ARX3A0_READ_REG(0x305e, &reg_value, 2);
		if (ret != 0)
			return ret;

		digital_gain = reg_value >> 7;
		coarse_gain = (reg_value >> 4) & 7;
		fine_gain = reg_value & 0xF;
	}

	// Fixed point factors of 16 and 64 in registers, so multiply by 64 to get our *0x10000 scale
	return ((fine_gain + 16) << coarse_gain) * digital_gain * 64;
}

/**
  \fn           int32_t ARX3A0_Init(ARM_CAMERA_RESOLUTION cam_resolution)
  \brief        Initialize ARX3A0 Camera Sensor
  this function will
  - initialize i2c using i3c instance
  - software reset ARX3A0 Camera Sensor
  - read ARX3A0 chip-id, proceed only it is correct.
  \param[in]    cam_resolution  : Camera Resolution \ref ARM_CAMERA_RESOLUTION
  \return       \ref execution_status
  */
int32_t ARX3A0_Init(ARM_CAMERA_RESOLUTION cam_resolution)
{
	int32_t  ret = 0;
	uint32_t rcv_data = 0;

	/*Unused*/
	(void)cam_resolution;

	/*camera sensor clock source config*/
	ARX3A0_Sensor_Clk_Src();

	/*camera sensor resten*/
	ret = ARX3A0_Camera_Hard_Reseten();
	if(ret != ARM_DRIVER_OK)
		return ret;

	/* Initialize i2c using i3c driver instance depending on
	 *  ARX3A0 Camera Sensor specific i2c configurations
	 *   \ref arx3A0_camera_sensor_i2c_cnfg
	 */

	ret = camera_sensor_i2c_init(&arx3A0_camera_sensor_i2c_cnfg);
	if(ret != ARM_DRIVER_OK)
		return ret;

	/* Soft Reset ARX3A0 Camera Sensor */
	ret = ARX3A0_Camera_Soft_Reseten();
	if(ret != ARM_DRIVER_OK)
		return ret;

	/* Read ARX3A0 Camera Sensor CHIP ID */
	ret = ARX3A0_READ_REG(ARX3A0_CHIP_ID_REGISTER, &rcv_data, 2);
	if(ret != ARM_DRIVER_OK)
		return ret;

	/* Proceed only if CHIP ID is correct. */
	if(rcv_data != ARX3A0_CHIP_ID_REGISTER_VALUE)
		return ARM_DRIVER_ERROR;

	/*Putting sensor in standby mode*/
	ret = ARX3A0_WRITE_REG(ARX3A0_MODE_SELECT_REGISTER, 0x00, 1);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ret = ARX3A0_READ_REG(ARX3A0_MIPI_CONFIG_REGISTER, &rcv_data, 2);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ret = ARX3A0_WRITE_REG(ARX3A0_MIPI_CONFIG_REGISTER, rcv_data | (1U << 7), 2);
	if(ret != ARM_DRIVER_OK)
		return ret;

	/*start streaming*/
	ret = ARX3A0_WRITE_REG(ARX3A0_MODE_SELECT_REGISTER, 0x01, 1);
	if(ret != ARM_DRIVER_OK)
		return ret;

	ARX3A0_DELAY_uSEC(50000);

	/*stop streaming*/
	ret = ARX3A0_WRITE_REG(ARX3A0_MODE_SELECT_REGISTER, 0x00, 1);
	if(ret != ARM_DRIVER_OK)
		return ret;

	/*Adding delay to finish streaming*/
	ARX3A0_DELAY_uSEC(500000);

	return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ARX3A0_Start(void)
  \brief        Start ARX3A0 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t ARX3A0_Start(void)
{
	/* Start streaming */
	return ARX3A0_WRITE_REG(ARX3A0_MODE_SELECT_REGISTER, 0x01, 1);
}

/**
  \fn           int32_t ARX3A0_Stop(void)
  \brief        Stop ARX3A0 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t ARX3A0_Stop(void)
{
	/* Suspend any stream */
	return ARX3A0_WRITE_REG(ARX3A0_MODE_SELECT_REGISTER, 0x00, 1);
}

/**
  \fn           int32_t ARX3A0_Control(uint32_t control, uint32_t arg)
  \brief        Control ARX3A0 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
int32_t ARX3A0_Control(uint32_t control, uint32_t arg)
{
	switch (control)
	{
		case CAMERA_SENSOR_CONFIGURE:
			return ARX3A0_Camera_Cfg((ARM_CAMERA_RESOLUTION)arg);
		case CAMERA_SENSOR_GAIN:
			return ARX3A0_Camera_Gain(arg);
		default:
			return ARM_DRIVER_ERROR_PARAMETER;
	}
}

/**
  \fn           int32_t ARX3A0_Uninit(void)
  \brief        Un-initialize ARX3A0 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t ARX3A0_Uninit(void)
{
	return ARM_DRIVER_OK;
}

/**
  \brief ARX3A0 Camera Sensor Information
  \ref CAMERA_SENSOR_INFO
  */
static CAMERA_SENSOR_INFO arx3a0_info =
{
	.output_format     = ARX3A0_USER_SELECT_CAMERA_OUTPUT_FORMAT,
	.additional_width  = 0,
	.additional_height = 0,
};

/**
  \brief ARX3A0 Camera Sensor Configurations
  \ref CAMERA_SENSOR_CONFIG
  */
static CAMERA_SENSOR_CONFIG arx3a0_config =
{
	.interface    = CAMERA_SENSOR_INTERFACE_MIPI_CSI,
	.clock_source = RTE_ARX3A0_CAMERA_CLOCK_SOURCE,
	.pixelclk_pol = RTE_ARX3A0_CAMERA_SENSOR_PIXEL_CLK_POL,
	.hsync_pol    = RTE_ARX3A0_CAMERA_SENSOR_HSYNC_POL,
	.vsync_pol    = RTE_ARX3A0_CAMERA_SENSOR_VSYNC_POL,
	.hsync_mode   = RTE_ARX3A0_CAMERA_SENSOR_HSYNC_MODE,
	.data_mode    = RTE_ARX3A0_CAMERA_SENSOR_DATA_MODE,
	.data_mask    = RTE_ARX3A0_CAMERA_SENSOR_DATA_MASK,
};

/**
  \brief ARX3A0 Camera Sensor Operations
  \ref CAMERA_SENSOR_OPERATIONS
  */
static CAMERA_SENSOR_OPERATIONS arx3a0_ops =
{
	.Init    = ARX3A0_Init,
	.Uninit  = ARX3A0_Uninit,
	.Start   = ARX3A0_Start,
	.Stop    = ARX3A0_Stop,
	.Control = ARX3A0_Control,
};

/**
  \brief ARX3A0 Camera Sensor Device Structure
Contains:
- ARX3A0 Camera Sensor Information
- ARX3A0 Camera Sensor Configurations
- ARX3A0 Camera Sensor Operations
\ref CAMERA_SENSOR_DEVICE
*/
extern CAMERA_SENSOR_DEVICE arx3a0_camera_sensor;
CAMERA_SENSOR_DEVICE arx3a0_camera_sensor =
{
	.Info    = &arx3a0_info,
	.Config  = &arx3a0_config,
	.Ops     = &arx3a0_ops,
};

#endif /* RTE_ARX3A0_CAMERA_SENSOR_ENABLE */
