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
 * @file     CRC_baremetal.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     18-april-2022
 * @brief    Baremetal demo application code for CRC driver
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include <string.h>

/* Project Includes */
/* include for CRC Driver */
#include "Driver_CRC.h"

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

#define ENABLE   1
#define DISABLE  0

/* Select the CRC algorithm */
#define CRC_8_BIT           1
#define CRC_16_BIT          0
#define CRC_32_BIT          0
#define CRC_CUSTOM_32_BIT   0

/* Select the type of input */
#define INPUT_HEX_VALUE     1
#define INPUT_STRING_VALUE  0

/* CRC driver instance */
extern ARM_DRIVER_CRC Driver_CRC0;
static ARM_DRIVER_CRC *CRCdrv = &Driver_CRC0;

void crc_demo();

#if INPUT_HEX_VALUE

uint8_t input_value[] = {0x67, 0x3F, 0x90, 0xC9, 0x25, 0xF0, 0x4A, 0xB1,0x12 };

#endif

#if INPUT_STRING_VALUE

uint8_t input_value[] = {"Hello"};

#endif

uint32_t seed_value_8_bit   = (0x00000000);  /* Seed value for 8 bit */
uint32_t seed_value_16_bit  = (0x00000000);  /* Seed value for 16 bit */
uint32_t seed_value_32_bit  = (0xFFFFFFFF);  /* Seed value for 32 bit */
uint32_t seed_value_32C_bit = (0xFFFFFFFF);  /* Seed value for 32 bit custom polynomial */

uint32_t polynomial = 0x2CEEA6C8;  /* polynomial value for 32 bit custom polynomial */

/**
 * @fn         :void crc_demo()
 * @brief      :CRC demo :
                  - This initialize the CRC.
                  - For 8 bit, 16 bit and 32 bit,the seed value will be added
                    into the Seed register.
                  - If user provides 8 bit or 16 bit or 32 bit or 32 bit custom
                    polynomial input , according to that 8 bit or 16 bit or 32
                    bit output will be printed.
                  - For 8 bit and 16 bit CRC the input will be added to DATA_IN_8
                    register and for 32 bit CRC the input will be added to the
                    DATA_IN_32 register.
 * @return     : none
 */
void crc_demo()
{
    uint32_t  ret = 0;
    uint8_t len;
    uint32_t crc_output;
    ARM_DRIVER_VERSION version;

    printf("\r\n >>> CRC demo starting up!!! <<< \r\n");

    version = CRCdrv->GetVersion();
    printf("\r\n CRC version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize CRC driver */
    ret = CRCdrv->Initialize();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC init failed\n");
        return;
    }

    /* Enable the power for CRC */
    ret = CRCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Power up failed\n");
        goto error_uninitialize;
    }

#if INPUT_HEX_VALUE
    len = sizeof(input_value) / sizeof(input_value[0]);
#endif

#if INPUT_STRING_VALUE
    len = strlen((char *)input_value);
#endif

/* For 8 bit */
#if CRC_8_BIT

    /* To disable the Reflect, Invert, Bit, Byte, Custom polynomial bit of CRC */
    ret = CRCdrv->Control(ARM_CRC_CONTROL_MASK, DISABLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control mask for 8 bit data failed\n");
        goto error_poweroff;
    }

    /* To select the 8_BIT_CCITT */
    ret= CRCdrv->Control(ARM_CRC_ALGORITHM_SEL,ARM_CRC_ALGORITHM_SEL_8_BIT_CCITT);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control Algorithm for 8 bit data failed\n");
        goto error_poweroff;
    }

    /* Adding 8 bit seed value to the seed register */
    ret = CRCdrv->Seed(seed_value_8_bit);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error:CRC Seed value for 8 bit failed\n");
        goto error_uninitialize;
    }

    /*sending User input */
    ret = CRCdrv->Compute(input_value, len, &crc_output);
    printf("8 bit output : %x\n", crc_output);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC 8 bit data sending failed\n");
        goto error_send;

    }

#endif

/* For 16 bit CRC*/
#if CRC_16_BIT

    /* To disable the Reflect, Invert, Bit, Byte, Custom polynomial bit of CRC */
    ret = CRCdrv->Control(ARM_CRC_CONTROL_MASK, DISABLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control mask for 16 bit data failed\n");
        goto error_poweroff;
    }

    /* To select the 16_BIT_CCITT */
    ret= CRCdrv->Control(ARM_CRC_ALGORITHM_SEL,ARM_CRC_ALGORITHM_SEL_16_BIT_CCITT);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control Algorithm for 16 bit data failed\n");
        goto error_poweroff;
    }

    /* Adding 16 bit seed value to the seed register */
    ret = CRCdrv->Seed(seed_value_16_bit);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Seed value for 16 bit data failed\n");
        goto error_uninitialize;
    }

    /*sending user input */
    ret = CRCdrv->Compute (input_value, len, &crc_output);
    printf("16 bit output : %x\n", crc_output);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC 16 bit data sending failed\n");
        goto error_send;
    }

#endif

/* For 32 bit CRC */
#if CRC_32_BIT

    /* To Enable the Reflect, Invert, Bit swap of CRC control register */
    ret = CRCdrv->Control(ARM_CRC_ENABLE_REFLECT |
                          ARM_CRC_ENABLE_INVERT  |
                          ARM_CRC_ENABLE_BIT_SWAP,ENABLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control for 32 bit failed\n");
        goto error_poweroff;
    }

    /* To select the 32_BIT CRC algorithm */
    ret= CRCdrv->Control(ARM_CRC_ALGORITHM_SEL,ARM_CRC_ALGORITHM_SEL_32_BIT);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control Algorithm for 32 bit data failed\n");
        goto error_poweroff;
    }

    /* Adding 32 bit seed value to the seed register */
    ret = CRCdrv->Seed(seed_value_32_bit);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Seed value for 32 bit failed\n");
        goto error_uninitialize;
    }

    /*sending user input */
    ret = CRCdrv->Compute(input_value, len, &crc_output);
    printf("32 bit output : %x\n", crc_output);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC 32 bit data sending failed\n");
        goto error_send;
    }
#endif

/* For 32 bit Custom polynomial */
#if CRC_CUSTOM_32_BIT

    /* Adding polynomial value */
    ret = CRCdrv->PolyCustom(polynomial);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC PolyCustom value failed\n");
        goto error_uninitialize;
    }

    /* To disable the Reflect, Invert, Bit swap of CRC control register */
    ret = CRCdrv->Control(ARM_CRC_ENABLE_REFLECT |
                          ARM_CRC_ENABLE_INVERT  |
                          ARM_CRC_ENABLE_BIT_SWAP,ENABLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control for 32 bit custom polynomial failed\n");
        goto error_poweroff;
    }

    /* To enable the CRC custom polynomial */
    ret = CRCdrv->Control(ARM_CRC_ENABLE_CUSTOM_POLY,ENABLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control custom polynomial value failed\n");
        goto error_poweroff;
    }

    /* To select the 32_BIT_CUSTOM_POLY crc algorithm */
    ret= CRCdrv->Control(ARM_CRC_ALGORITHM_SEL,ARM_CRC_ALGORITHM_SEL_32_BIT_CUSTOM_POLY);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Control Algorithm for 32 bit data custom polynomial failed\n");
        goto error_poweroff;
    }

    /* Adding 32 bit poly custom seed value to the seed register */
    ret = CRCdrv->Seed(seed_value_32C_bit);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Seed value for 32 bit data custom polynomial failed\n");
        goto error_uninitialize;
    }

    /*sending user input */
    ret = CRCdrv->Compute (input_value, len, &crc_output);
    printf("32 bit CUSTOM POLY output value: %x\n", crc_output);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC 32 bit custom polynomial data sending failed\n");
        goto error_send;
    }
#endif

    error_send:
    error_poweroff:
    /* Power off CRC peripheral */
    ret = CRCdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CRC Power OFF failed.\r\n");
    }

    error_uninitialize:
    /* UnInitialize CRC driver */
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CRC Uninitialize failed.\r\n");
    }
}

/* Define main entry point */
int main()
{
    crc_demo();

    return 0;
}
