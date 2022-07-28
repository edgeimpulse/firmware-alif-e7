/* Copyright (c) 2022 ALIF SEMICONDUCTOR

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.
 *
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 ---------------------------------------------------------------------------*/

/**************************************************************************//**
 * @file     image_capture.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     19-April-2022
 * @brief    Image capture functions.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
#include <stdint.h>

#include "base_def.h"
#include "Driver_PINMUX_AND_PINPAD.h"
#include "Driver_GPIO.h"
#include "Driver_Camera_Controller.h"
#include "Driver_MIPI_DSI.h"
#include "Camera_Common.h"

#include "image_processing.h"

/* Camera  Driver instance 0 */
extern ARM_DRIVER_CAMERA_CONTROLLER Driver_CAMERA0;
static ARM_DRIVER_CAMERA_CONTROLLER *CAMERAdrv = &Driver_CAMERA0;

/* MIPI DSI Driver instance*/
extern ARM_DRIVER_MIPI_DSI Driver_MIPI_DSI;
static ARM_DRIVER_MIPI_DSI *DPHYmaster = &Driver_MIPI_DSI;

/*Set minimum frequency for DPHY TX*/
#define MIPI_DSI_DPHY_FREQUENCY                   40000000

/* Camera callback events */
typedef enum {
	CAM_CB_EVENT_FRAME_VSYNC_DETECTED = (1 << 0),
	CAM_CB_EVENT_CAPTURE_STOPPED      = (1 << 1),
	CAM_CB_EVENT_ERROR                = (1 << 2)
}CAMERA_CB_EVENTS;

static volatile uint32_t camera_event_flags;
static volatile uint32_t actual_events;

static void camera_callback(uint32_t event)
{
	if (event & ARM_CAMERA_CONTROLLER_EVENT_CAMERA_FRAME_VSYNC_DETECTED)
	{
		/* Transfer Success: Frame VSYNC detected, Wake-up Thread. */
		//tx_event_flags_set(&camera_event_flags, CAM_CB_EVENT_FRAME_VSYNC_DETECTED, TX_OR);
		camera_event_flags |= CAM_CB_EVENT_FRAME_VSYNC_DETECTED;
	}

	if (event & ARM_CAMERA_CONTROLLER_EVENT_CAMERA_CAPTURE_STOPPED)
	{
		/* Transfer Success: Capture Stop detected, Wake-up Thread. */
		//tx_event_flags_set(&camera_event_flags, CAM_CB_EVENT_CAPTURE_STOPPED, TX_OR);
		camera_event_flags |= CAM_CB_EVENT_CAPTURE_STOPPED;
	}

	if (event & ARM_CAMERA_CONTROLLER_EVENT_ERR_CAMERA_FIFO_OVERRUN)
	{
		/* Transfer Error: Received FIFO over-run, Wake-up Thread. */
		//tx_event_flags_set(&camera_event_flags, CAM_CB_EVENT_ERROR, TX_OR);
		camera_event_flags |= CAM_CB_EVENT_ERROR;
	}

	if (event & ARM_CAMERA_CONTROLLER_EVENT_ERR_CAMERA_FIFO_UNDERRUN)
	{
		/* Transfer Error: Received FIFO under-run, Wake-up Thread. */
		//tx_event_flags_set(&camera_event_flags, CAM_CB_EVENT_ERROR, TX_OR);
		camera_event_flags |= CAM_CB_EVENT_ERROR;
	}

	if (event & ARM_CAMERA_CONTROLLER_EVENT_ERR_HARDWARE)
	{
		/* Transfer Error: Received Hardware error, Wake-up Thread. */
		//tx_event_flags_set(&camera_event_flags, CAM_CB_EVENT_ERROR, TX_OR);
		camera_event_flags |= CAM_CB_EVENT_ERROR;
	}
}


static void setup_pinmux()
{
	/* Configure GPIO Pin : P3_8 as I3C_SDA_B */
	PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_8, PINMUX_ALTERNATE_FUNCTION_3);
	/* Configure GPIO Pin : P3_9 as I3C_SCL_B */
	PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_9, PINMUX_ALTERNATE_FUNCTION_4);

	/* i3c Pin-Pad 
	 * Pad function: weak pull up(0x8) + read enable(0x01)
	 *               + Output drive strength 4mA(0x20) */
	PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_8,  \
			(0x09 | PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
	PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_9,  \
			(0x09 | PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));

	/* Configure GPIO Pin : P2_7 as CAM_XVCLK_B */
	PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_7, PINMUX_ALTERNATE_FUNCTION_6);
}

int32_t camera_init()
{
	ARM_CAMERA_RESOLUTION camera_resolution = CAMERA_RESOLUTION_560x560;

    setup_pinmux();

	int32_t ret2, ret = DPHYmaster->Initialize(MIPI_DSI_DPHY_FREQUENCY);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Initialize failed.\r\n");
		return ret;
	}

	ret = CAMERAdrv->Initialize(camera_resolution, camera_callback);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: CAMERA Initialize failed.\r\n");
		goto error_uninitialize_dphy;
	}

	/* Power up MIPI DSI Master DPHY*/
	ret = DPHYmaster->PowerControl(ARM_POWER_FULL);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Power Up failed.\r\n");
		goto error_uninitialize_camera;
	}

	/* Power up Camera peripheral */
	ret = CAMERAdrv->PowerControl(ARM_POWER_FULL);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: CAMERA Power Up failed.\r\n");
		goto error_poweroff_dphy;
	}

	ret =  DPHYmaster->StartPHY();
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master setup failed.\r\n");
		goto error_poweroff_camera;
	}

	ret = CAMERAdrv->Control(CAMERA_SENSOR_CONFIG, CAMERA_RESOLUTION_560x560);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: CAMERA SENSOR Configuration failed.\r\n");
		goto error_poweroff_camera;
	}
	return ARM_DRIVER_OK;
    
error_poweroff_camera:
	/* Power off CAMERA peripheral */
	ret2 = CAMERAdrv->PowerControl(ARM_POWER_OFF);
	if (ret2 != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Power OFF failed.\r\n");
    }
error_poweroff_dphy:
	/* Power off MIPI DSI Master DPHY*/
	ret2 = DPHYmaster->PowerControl(ARM_POWER_OFF);
	if (ret2 != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Power Off failed.\r\n");
    }
error_uninitialize_camera:
	/* Un-initialize CAMERA driver */
	ret2 = CAMERAdrv->Uninitialize();
	if (ret2 != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Uninitialize failed.\r\n");
    }
error_uninitialize_dphy:
	/* Un-initialize MIPI DSI Master DPHY driver */
	ret2 = DPHYmaster->Uninitialize();
	if (ret2 != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Uninitialize failed.\r\n");
	}
	return ret;
};

int32_t camera_start(uint32_t mode, uint8_t* buffer)
{
    camera_event_flags = actual_events = 0;
	int32_t ret = CAMERAdrv->CaptureFrame(buffer);
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: CAMERA Capture Frame failed.\r\n");
		goto error_poweroff_camera;
	}
    return ARM_DRIVER_OK;

error_poweroff_camera:
#if 0
	/* Power off CAMERA peripheral */
	ret2 = CAMERAdrv->PowerControl(ARM_POWER_OFF);
	if (ret != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Power OFF failed.\r\n");
    }
	/* Power off MIPI DSI Master DPHY*/
	ret2 = DPHYmaster->PowerControl(ARM_POWER_OFF);
	if (ret2 != ARM_DRIVER_OK)
	{
        DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Power Off failed.\r\n");
    }
	/* Un-initialize CAMERA driver */
	ret2 = CAMERAdrv->Uninitialize();
	if (ret2 != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Uninitialize failed.\r\n");
    }
	/* Un-initialize MIPI DSI Master DPHY driver */
	ret2 = DPHYmaster->Uninitialize();
	if (ret2 != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Uninitialize failed.\r\n");
	}
#endif
	return ret;
}


int32_t camera_vsync(uint32_t timeout_ms)
{
    int32_t ret = -1;

	/* wait till any event success/error comes in isr callback,
	 *  and if event is set then clear that event.
	 * if the event flags are not set,
	 *  this service suspends for a maximum of 'n' timer-ticks.
	 */
    while (camera_event_flags == 0)
    {
        /* code */
        actual_events = camera_event_flags;
    }
    
/*
	wait_timer_ticks = TX_TIMER_TICKS_PER_SECOND;
	ret = tx_event_flags_get(&camera_event_flags, \
			CAM_CB_EVENT_FRAME_VSYNC_DETECTED  |  \
			CAM_CB_EVENT_CAPTURE_STOPPED      |   \
			CAM_CB_EVENT_ERROR,                   \
			TX_OR_CLEAR,                          \
			&actual_events,                       \
			wait_timer_ticks);
	if (ret != TX_SUCCESS)
	{
		DEBUG_PRINTF("Error: CAMERA tx_event_flags_get failed.\n");
		goto error_poweroff_camera;
	}
*/
	if (actual_events & CAM_CB_EVENT_ERROR)
	{
		/* Error: Camera Capture Frame failed. */
		DEBUG_PRINTF("\r\n \t\t >> Error: CAMERA Capture Frame failed. \r\n");
		goto error_poweroff_camera;
	}

	if (actual_events & (CAM_CB_EVENT_CAPTURE_STOPPED | CAM_CB_EVENT_FRAME_VSYNC_DETECTED))
	{
		/* Success: Camera Capture Frame success. */
		DEBUG_PRINTF("\r\n \t\t >> Success: CAMERA Capture Frame Success. \r\n");
	}

	/* Okay, we have received Success: Camera Capture Frame VSYNC detected.
	 * now stop Camera Capture.
	 */
	ret = CAMERAdrv->Stop();
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: CAMERA stop Capture failed.\r\n");
		goto error_poweroff_camera;
	}

	return ARM_DRIVER_OK;

error_poweroff_camera:
#if 0
	/* Power off CAMERA peripheral */
	ret = CAMERAdrv->PowerControl(ARM_POWER_OFF);
	if (ret != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Power OFF failed.\r\n");
    }
	/* Power off MIPI DSI Master DPHY*/
	ret = DPHYmaster->PowerControl(ARM_POWER_OFF);
	if (ret != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Power Off failed.\r\n");
    }
	/* Un-initialize CAMERA driver */
	ret = CAMERAdrv->Uninitialize();
	if (ret != ARM_DRIVER_OK)
    {
		DEBUG_PRINTF("\r\n Error: CAMERA Uninitialize failed.\r\n");
    }
	/* Un-initialize MIPI DSI Master DPHY driver */
	ret = DPHYmaster->Uninitialize();
	if (ret != ARM_DRIVER_OK)
	{
		DEBUG_PRINTF("\r\n Error: MIPI DSI DPHY Master Uninitialize failed.\r\n");
	}
#endif
	return ret;
}

int32_t camera_wait(uint32_t timeout_ms)
{
    /*
	while(HW_REG_WORD(CAMERA0_BASE, CAM_CTRL) & 0x2) {

	};*/
	return ARM_DRIVER_OK;
}
