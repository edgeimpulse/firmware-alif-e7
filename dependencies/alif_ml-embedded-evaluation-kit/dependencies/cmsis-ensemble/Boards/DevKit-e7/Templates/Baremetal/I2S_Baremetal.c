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
 * @file     I2S_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     13-June-2022
 * @brief    Baremetal Test Application for I2S for Carrier board.
 *           I2S0 is configured as master transmitter with MCLK 24.576Mhz and 24bit
 *           I2S2 is configured as master receiver SPH0645LM4H-1 device 24bit
 * @bug      None.
 * @Note	 None
 ******************************************************************************/

/*System Includes */
#include <stdio.h>
#include <string.h>

/* Project Includes */
#include <Driver_SAI.h>
#include <Driver_PINMUX_AND_PINPAD.h>

/*Audio samples */
#include "i2s_samples.h"

void DAC_Init ();
void ADC_Init ();
void Receiver ();

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
        (void)return_code;
        while (1);
    }
#endif

/* Enable this to feed the predefined hello sample in the TX path and RX path is disabled */
//#define DAC_PREDEFINED_SAMPLES

/* 1 to send the data stream continuously , 0 to send data only once */
#define REPEAT_TX 1

#define ERROR  -1
#define SUCCESS 0

#define I2S_DAC 0                    /* DAC I2S Controller 0 */
#define I2S_ADC 2                    /* ADC I2S Controller 2 */

#define DAC_SEND_COMPLETE_EVENT    (1U << 0)
#define ADC_RECEIVE_COMPLETE_EVENT (1U << 1)
#define ADC_RECEIVE_OVERFLOW_EVENT (1U << 2)

#define NUM_SAMPLES                 51200

uint32_t    buf_len2 = 0;
volatile int32_t event_flag = 0;

/* Buffer for ADC samples */
uint32_t sample_buf[NUM_SAMPLES];

uint32_t wlen = 24;
uint32_t sampling_rate = 48000;        /* 48Khz audio sampling rate */

extern ARM_DRIVER_SAI ARM_Driver_SAI_(I2S_DAC);
ARM_DRIVER_SAI *i2s_dac = &ARM_Driver_SAI_(I2S_DAC);

extern ARM_DRIVER_SAI ARM_Driver_SAI_(I2S_ADC);
ARM_DRIVER_SAI *i2s_adc = &ARM_Driver_SAI_(I2S_ADC);

/**
  \fn          void dac_callback(uint32_t event)
  \brief       Callback routine from the i2s driver
  \param[in]   event: Event for which the callback has been called
*/
void dac_callback(uint32_t event)
{
    if(event & ARM_SAI_EVENT_SEND_COMPLETE)
    {
        /* Send Success: Wake-up Thread. */
        event_flag |= DAC_SEND_COMPLETE_EVENT;
    }
}

/**
  \fn          void dac_pinmux_config(void)
  \brief       Initialize the pinmux for DAC
  \return      status
*/
int32_t dac_pinmux_config(void)
{
    int32_t status;

    /* Configure I2S0 SDO */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_29, PINMUX_ALTERNATE_FUNCTION_3);
    if(status)
        return ERROR;

    /* Configure I2S0 WS */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_31, PINMUX_ALTERNATE_FUNCTION_3);
    if(status)
        return ERROR;

    /* Configure I2S0 SCLK */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_30, PINMUX_ALTERNATE_FUNCTION_2);
    if(status)
        return ERROR;

    return SUCCESS;
}

/**
  \fn          void DAC_Init()
  \brief       DAC thread for master transmission
  \param[in]   None
*/
void DAC_Init()
{
    ARM_DRIVER_VERSION   version;
    ARM_SAI_CAPABILITIES cap;
    int32_t              status;

    /* Configure the dac pins */
    if(dac_pinmux_config())
    {
        printf("DAC pinmux failed\n");
        return;
    }

    /* Verify the I2S API version for compatibility */
    version = i2s_dac->GetVersion();
    printf("I2S API version = %d\n", version.api);

    /* Verify if I2S protocol is supported */
    cap = i2s_dac->GetCapabilities();
    if(!cap.protocol_i2s)
    {
        printf("I2S is not supported\n");
        return;
    }

    /* Initializes I2S0 interface */
    status = i2s_dac->Initialize(dac_callback);
    if(status)
    {
        printf("DAC Init failed status = %d\n", status);
        goto error_initialize;
    }

    /* Enable the power for I2S0 */
    status = i2s_dac->PowerControl(ARM_POWER_FULL);
    if(status)
    {
        printf("DAC Power Failed status = %d\n", status);
        goto error_power;
    }

    /* configure I2S0 Transmitter to Asynchronous Master */
    status = i2s_dac->Control(ARM_SAI_CONFIGURE_TX |
                                ARM_SAI_MODE_MASTER  |
                                ARM_SAI_ASYNCHRONOUS |
                                ARM_SAI_PROTOCOL_I2S |
                                ARM_SAI_DATA_SIZE(wlen), wlen*2, sampling_rate);
    if(status)
    {
        printf("DAC Control status = %d\n", status);
        goto error_control;
    }

    /* enable Transmitter */
    status = i2s_dac->Control(ARM_SAI_CONTROL_TX, 1, 0);
    if(status)
    {
        printf("DAC TX status = %d\n", status);
        goto error_control;
    }

#ifndef DAC_PREDEFINED_SAMPLES
    ADC_Init();

    /* enable Receiver */
    status = i2s_adc->Control(ARM_SAI_CONTROL_RX, 1, 0);
    if(status)
    {
        printf("ADC RX status = %d\n", status);
    }
#endif

    do
    {
#ifdef DAC_PREDEFINED_SAMPLES
        /* Using predefined samples */
        buf_len2 = sizeof(hello_samples_24bit_48khz)/sizeof(hello_samples_24bit_48khz[0]);

        /* Transmit the samples */
        status = i2s_dac->Send(hello_samples_24bit_48khz, buf_len2);
        if(status)
        {
            printf("DAC Send status = %d\n", status);
            goto error_send;
        }

        /* Wait for the completion event */
        while (1)
        {
            if (event_flag & DAC_SEND_COMPLETE_EVENT)
            {
                event_flag &= ~DAC_SEND_COMPLETE_EVENT;
                break;
            }
        }

#else
        /* Function to receive digital signal from mic */
        Receiver();
        /* Transmit the samples */
        status = i2s_dac->Send((uint32_t *)sample_buf, NUM_SAMPLES);
        if(status)
        {
            printf("DAC Send status = %d\n", status);
            goto error_send;
        }

        /* Wait for the completion event */
        while (1)
        {
            if (event_flag & DAC_SEND_COMPLETE_EVENT)
            {
                event_flag &= ~DAC_SEND_COMPLETE_EVENT;
                break;
            }
        }
#endif
    }while(REPEAT_TX);

#ifndef DAC_PREDEFINED_SAMPLES
    /* Stop the RX */
    status = i2s_adc->Control(ARM_SAI_CONTROL_RX, 0, 0);
    if(status)
    {
      printf("ADC RX status = %d\n", status);
      return;
    }
#endif

    /* Stop the TX */
    status = i2s_dac->Control(ARM_SAI_CONTROL_TX, 0, 0);
    if(status)
    {
        printf("DAC TX status = %d\n", status);
        goto error_control;
    }

error_send:
error_control:
    i2s_dac->PowerControl(ARM_POWER_OFF);
error_power:
    i2s_dac->Uninitialize();
error_initialize:
    while(1) {
    }
}

/**
  \fn          void adc_callback(uint32_t event)
  \brief       Callback routine from the i2s driver
  \param[in]   event Event for which the callback has been called
*/
void adc_callback(uint32_t event)
{
    if(event & ARM_SAI_EVENT_RECEIVE_COMPLETE)
    {
        /* Send Success: Wake-up Thread. */
        event_flag |= ADC_RECEIVE_COMPLETE_EVENT;
    }

    if(event & ARM_SAI_EVENT_RX_OVERFLOW)
    {
        /* Send Success: Wake-up Thread. */
        event_flag |= ADC_RECEIVE_OVERFLOW_EVENT;
    }
}

/**
  \fn          void adc_pinmux_config(void)
  \brief       Initialize the pinmux for ADC
  \return      status
*/
int32_t adc_pinmux_config(void)
{
    int32_t  status;

    /* Configure I2S2 WS */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_4, PINMUX_ALTERNATE_FUNCTION_2);
    if(status)
        return ERROR;

    /* Configure I2S2 SCLK */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_3, PINMUX_ALTERNATE_FUNCTION_3);
    if(status)
        return ERROR;

    /* Configure I2S2 SDI */
    status = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_3);
    if(status)
        return ERROR;

    return SUCCESS;
}

/**
  \fn          void ADC_Thread()
  \brief       ADC Thread to initialize ADC
  \param[in]   None
*/
void ADC_Init()
{
    ARM_DRIVER_VERSION   version;
    ARM_SAI_CAPABILITIES cap;
    int32_t              status;

    /* Configure the adc pins */
    if(adc_pinmux_config())
    {
        printf("ADC pinmux failed\n");
        return;
    }

    /* Verify the I2S API version for compatibility*/
    version = i2s_adc->GetVersion();
    printf("I2S API version = %d\n", version.api);

    /* Verify if I2S protocol is supported */
    cap = i2s_adc->GetCapabilities();
    if(!cap.protocol_i2s)
    {
        printf("I2S is not supported\n");
        return;
    }

    /* Initializes I2S2 interface */
    status = i2s_adc->Initialize(adc_callback);
    if(status)
    {
        printf("ADC Init failed status = %d\n", status);
        return;
    }

    /* Enable the power for I2S2 */
    status = i2s_adc->PowerControl(ARM_POWER_FULL);
    if(status)
    {
        printf("ADC Power failed status = %d\n", status);
        return;
    }

    /* configure I2S2 Receiver to Asynchronous Master */
    status = i2s_adc->Control(ARM_SAI_CONFIGURE_RX |
                                ARM_SAI_MODE_MASTER  |
                                ARM_SAI_ASYNCHRONOUS |
                                ARM_SAI_PROTOCOL_I2S |
                                ARM_SAI_DATA_SIZE(wlen), wlen*2, sampling_rate);
    if(status)
    {
      printf("ADC Control status = %d\n", status);
      return;
    }
}

/**
  \fn          void Receiver()
  \brief       Function performing reception from mic
  \param[in]   None
*/
void Receiver()
{
    int32_t  status;

    /* Receive data */
    status = i2s_adc->Receive((uint32_t *)sample_buf, NUM_SAMPLES);
    if(status)
    {
        printf("ADC Receive status = %d\n", status);
        return;
    }

    /* Wait for the completion event */
    while (1)
    {
        if (event_flag & ADC_RECEIVE_COMPLETE_EVENT)
        {
               event_flag &= ~ADC_RECEIVE_COMPLETE_EVENT;
               break;
        }

        if (event_flag & ADC_RECEIVE_OVERFLOW_EVENT)
        {
               event_flag &= ~ADC_RECEIVE_OVERFLOW_EVENT;
        }
    }
}

int main()
{
    DAC_Init();
    return 0;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
