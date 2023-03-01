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
 * @file     FLASH_ISSI_Baremetal.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     26-Oct-2022
 * @brief    Baremetal Application to demo the APIs of FLASH Driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "Driver_PINMUX_AND_PINPAD.h"
#include "Driver_Flash.h"
#include "RTE_Components.h"
#include CMSIS_device_header

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

#define FLASH_NUM 1

extern ARM_DRIVER_FLASH ARM_Driver_Flash_(FLASH_NUM);
#define ptrFLASH (&ARM_Driver_Flash_(FLASH_NUM))

#define FLASH_ADDR  0x00
#define BUFFER_SIZE 1024

/**
 * @fn      static int32_t setup_PinMUX(void)
 * @brief   Set up PinMUX and PinPAD
 * @note    none
 * @param   none
 * @retval  ARM_DRIVER_ERROR : If any param error
 *          0 : for Success
 */
static int32_t setup_PinMUX(void)
{
    int32_t ret;

    /* Configure OctalSPI 0 pins - DevBoard
    *
    * P1_16 .. P1_23 = D0..D7
    * P1_26 = RXDS
    * P1_25 = SCLK
    * P2_6 = CS
    * P2_7 = SCLKN
    */

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_16, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_17, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_18, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_19, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_20, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_21, PINMUX_ALTERNATE_FUNCTION_4);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_22, PINMUX_ALTERNATE_FUNCTION_4);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_23, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_25, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_26, PINMUX_ALTERNATE_FUNCTION_3);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_6, PINMUX_ALTERNATE_FUNCTION_4);
    if (ret)
        return ARM_DRIVER_ERROR;

    /* Configure pad control registers */
    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_16, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_17, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_18, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_19, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_20, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_21, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_22, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_23, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    ret = PINPAD_Config(PORT_NUMBER_1, PIN_NUMBER_26, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return ARM_DRIVER_ERROR;

    return 0;
}

/* Buffers for reading and writing data */
uint8_t read_buff[BUFFER_SIZE];
uint8_t write_buff[BUFFER_SIZE];

/**
 * @fn      int main ()
 * @brief   Main Function
 * @note    none
 * @param   none
 * @retval  0 : Success
 */
int main ()
{
    uint32_t ret, index, iter = 0, count = 0;
    int32_t status;
    ARM_DRIVER_VERSION version;
    ARM_FLASH_INFO *flash_info;

    /* Prepare the data for writing to flash */
    while (iter < BUFFER_SIZE)
    {
        for (index = 0; index < 256; index++)
        {
            write_buff[iter++] = index;
        }
    }

    printf("OSPI Flash Initialization\n");

    ret = setup_PinMUX();

    if (ret != ARM_DRIVER_OK)
    {
        printf("Set up pinmux failed\n");
        goto error_pinmux;
    }

    /* Get version of the flash */
    version = ptrFLASH->GetVersion();

    printf("\r\n FLASH version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize the flash */
    status = ptrFLASH->Initialize(NULL);

    if (status != ARM_DRIVER_OK)
    {
        printf("Flash initialization failed\n");
        goto error_uninitialize;
    }

    status = ptrFLASH->PowerControl(ARM_POWER_FULL);

    if (status != ARM_DRIVER_OK)
    {
        printf("Flash Power control failed\n");
        goto error_poweroff;
    }

    /* Get Flash Info.*/
    flash_info = ptrFLASH->GetInfo();

    printf("\r\n FLASH Info : \n Sector Count : %d\n Sector Size : %d Bytes\n Page Size : %d\n Program Unit : %d\n "
             "Erased Value : 0x%X \r\n",flash_info->sector_count, flash_info->sector_size, flash_info->page_size,
             flash_info->program_unit, flash_info->erased_value);

    printf("\nErasing the chip\n");

    /* Erase the chip */
    status = ptrFLASH->EraseChip();

    if (status != ARM_DRIVER_OK)
    {
        printf("Chip erase failed\n");
        goto error_poweroff;
    }

    printf("starting reading erased data\n");

    iter = 0;

    /* Read the 1KB data after erase and check if it is erased completely */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    /* Verify the read data */
    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != flash_info->erased_value)
            count++;
        iter++;
    }

    printf("Total errors after reading erased chip = %d\n", count);

    printf("Starting writing\n");

    /* Write 1 KB data to the flash */
    status = ptrFLASH->ProgramData(FLASH_ADDR, write_buff, BUFFER_SIZE);
    if (status != BUFFER_SIZE)
    {
        printf("Data not written completely\n");
        goto error_poweroff;
    }

    printf("Finished writing\n");

    iter = 0;
    count = 0;

    printf("Starting reading after writing\n");

    /* Read the 1KB data after writing to flash */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != write_buff[iter])
            count++;
        iter++;
    }

    printf("Total errors after reading data written to flash = %d\n", count);

    iter = 0;
    count = 0;

    /* Erase 4KB sector */
    status = ptrFLASH->EraseSector(FLASH_ADDR);

    if (status != ARM_DRIVER_OK)
    {
        printf("Sector erase failed\n");
        goto error_poweroff;
    }

    printf("starting reading after erasing a sector\n");

    /* Read the 1KB data after erasing a sector */
    status = ptrFLASH->ReadData(FLASH_ADDR, read_buff, BUFFER_SIZE);

    if (status != BUFFER_SIZE)
    {
        printf("Data not read completely\n");
        goto error_poweroff;
    }

    while (iter < BUFFER_SIZE)
    {
        if (read_buff[iter] != flash_info->erased_value)
            count++;
        iter++;
    }

    printf("Total errors after erasing a sector = %d\n", count);

    while (1);

error_poweroff :
    status = ptrFLASH->PowerControl(ARM_POWER_OFF);
    if (status != ARM_DRIVER_OK)
    {
        printf("Flash Power control failed\n");
    }

error_uninitialize :
    status = ptrFLASH->Uninitialize();
    if (status != ARM_DRIVER_OK)
    {
        printf("Flash un-initialization failed\n");
    }

error_pinmux :
    return 0;
}
