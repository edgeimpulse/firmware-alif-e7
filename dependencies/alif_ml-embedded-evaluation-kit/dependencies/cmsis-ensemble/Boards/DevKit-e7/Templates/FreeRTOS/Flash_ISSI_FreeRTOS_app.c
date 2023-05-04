/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


/**************************************************************************//**
 * @file     FLASH_ISSI_FreeRTOS_app.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @Version  V1.0.0
 * @date     28-Oct-2022
 * @brief    FreeRTOS Application to demo the APIs of FLASH Driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "stdint.h"
#include "Driver_Flash.h"
#include "Driver_PINMUX_AND_PINPAD.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/*RTOS Includes*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/* For Release build disable printf and semihosting */
#define DISABLE_PRINTF

#ifdef DISABLE_PRINTF
  #define printf( fmt, ... ) ( 0 )
  /* Also Disable Semihosting */
  #if __ARMCC_VERSION >= 6000000
    __asm( ".global __use_no_semihosting" );
  #elif __ARMCC_VERSION >= 5000000
    #pragma import( __use_no_semihosting )
  #else
    #error Unsupported compiler
  #endif
  void _sys_exit( int return_code ) { while ( 1 ); }
#endif

#define DEMO_THREAD_STACK_SIZE                        ( 512 )

/* Thread id of thread */
TaskHandle_t xDemoTask;

#define FLASH_NUM 1

extern ARM_DRIVER_FLASH ARM_Driver_Flash_( FLASH_NUM );
#define ptrFLASH ( &ARM_Driver_Flash_( FLASH_NUM ) )

#define FLASH_ADDR 0x00
#define BUFFER_SIZE 1024

/* Buffers for reading and writing data */
uint8_t ucReadBuff[  BUFFER_SIZE  ];
uint8_t ucWriteBuff[  BUFFER_SIZE  ];


/**
 * @fn      static int32_t prvSetupPinMUX( void )
 * @brief   Set up PinMUX and PinPAD
 * @note    none
 * @param   none
 * @retval  ARM_DRIVER_ERROR
 *          0 : for Success
 */
static int32_t prvSetupPinMUX( void )
{
    int32_t lRet;

    /* Configure OctalSPI 0 pins - DevBoard
    *
    * P1_16 .. P1_23 = D0..D7
    * P1_26 = RXDS
    * P1_25 = SCLK
    * P2_6 = CS
    * P2_7 = SCLKN
    */

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_16, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_17, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_18, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_19, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_20, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_21, PINMUX_ALTERNATE_FUNCTION_4 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_22, PINMUX_ALTERNATE_FUNCTION_4 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_23, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_25, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_1, PIN_NUMBER_26, PINMUX_ALTERNATE_FUNCTION_3 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINMUX_Config( PORT_NUMBER_2, PIN_NUMBER_6, PINMUX_ALTERNATE_FUNCTION_4 );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    /* Configure pad control registers */
    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_16, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_17, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_18, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_19, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_20, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_21, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_22, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_23, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    lRet = PINPAD_Config( PORT_NUMBER_1, PIN_NUMBER_26, PAD_FUNCTION_READ_ENABLE );
    if ( lRet )
        return ARM_DRIVER_ERROR;

    return 0;
}

/**
 * @fn      void vFlashThread ( void *pvParameters )
 * @brief   Used to read, write and erase the flash.
 * @note    none.
 * @param   thread_input : thread input.
 * @retval  none.
 */
void vFlashThread ( void *pvParameters )
{
    uint32_t ulIndex, ulIter = 0, ulCount = 0;
    int32_t lStatus, lRet;
    ARM_DRIVER_VERSION xVersion;
    ARM_FLASH_INFO *pxFlashInfo;

    /* Prepare the data for writing to flash */
    while ( ulIter < BUFFER_SIZE )
    {
        for ( ulIndex = 0; ulIndex < 256; ulIndex++ )
        {
            ucWriteBuff[ ulIter++ ] = ulIndex;
        }
    }

    printf( "OSPI Flash Initialization\n" );

    lRet = prvSetupPinMUX();

    if ( lRet != ARM_DRIVER_OK )
    {
        printf( "Set up pinmux failed\n" );
        goto error_pinmux;
    }

    /* Get xVersion of the flash */
    xVersion = ptrFLASH->GetVersion();

    printf( "\r\n FLASH xVersion api:%X driver:%X...\r\n",xVersion.api, xVersion.drv );

    /* Initialize the flash */
    lStatus = ptrFLASH->Initialize( NULL );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash initialization failed\n" );
        goto error_uninitialize;
    }

    lStatus = ptrFLASH->PowerControl( ARM_POWER_FULL );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash Power control failed\n" );
        goto error_poweroff;
    }

    /* Get Flash Info.*/
    pxFlashInfo = ptrFLASH->GetInfo();

    printf( "\r\n FLASH Info : \n Sector ulCount : %d\n Sector Size : %d Bytes\n Page Size : %d\n Program Unit : %d\n "
             "Erased Value : 0x%X \r\n",pxFlashInfo->sector_count, pxFlashInfo->sector_size, pxFlashInfo->page_size,
             pxFlashInfo->program_unit, pxFlashInfo->erased_value );

    printf( "\nErasing the chip\n" );

    /* Erase the chip */
    lStatus = ptrFLASH->EraseChip();

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Chip erase failed\n" );
        goto error_poweroff;
    }

    printf( "starting reading erased data\n" );

    ulIter = 0;

    /* Read the 1KB data after erase and check if it is erased completely */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, ucReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    /* Verify the read data */
    while ( ulIter < BUFFER_SIZE )
    {
        if ( ucReadBuff[ ulIter ] != pxFlashInfo->erased_value )
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after reading erased chip = %d\n", ulCount );

    printf( "Starting writing\n" );

    /* Write 1 KB data to the flash */
    lStatus = ptrFLASH->ProgramData( FLASH_ADDR, ucWriteBuff, BUFFER_SIZE );
    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not written completely\n" );
        goto error_poweroff;
    }

    printf( "Finished writing\n" );

    ulIter = 0;
    ulCount = 0;

    printf( "Starting reading after writing\n" );

    /* Read the 1KB data after writing to flash */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, ucReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    while ( ulIter < BUFFER_SIZE )
    {
        if ( ucReadBuff[ ulIter ] != ucWriteBuff[ ulIter ] )
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after reading data written to flash = %d\n", ulCount );

    ulIter = 0;
    ulCount = 0;

    /* Erase 4KB sector */
    lStatus = ptrFLASH->EraseSector( FLASH_ADDR );

    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Sector erase failed\n" );
        goto error_poweroff;
    }

    printf( "starting reading after erasing a sector\n" );

    /* Read the 1KB data after erasing a sector */
    lStatus = ptrFLASH->ReadData( FLASH_ADDR, ucReadBuff, BUFFER_SIZE );

    if ( lStatus != BUFFER_SIZE )
    {
        printf( "Data not read completely\n" );
        goto error_poweroff;
    }

    while ( ulIter < BUFFER_SIZE )
    {
        if ( ucReadBuff[ ulIter ] != pxFlashInfo->erased_value )
            ulCount++;
        ulIter++;
    }

    printf( "Total errors after erasing a sector = %d\n", ulCount );

    while ( 1 );

error_poweroff :
    lStatus = ptrFLASH->PowerControl( ARM_POWER_OFF );
    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash Power control failed\n" );
    }

error_uninitialize :
    lStatus = ptrFLASH->Uninitialize();
    if ( lStatus != ARM_DRIVER_OK )
    {
        printf( "Flash un-initialization failed\n" );
    }

error_pinmux :
    printf( "Pinmux error\n" );
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main(  void  )
{
    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate( vFlashThread, "vFlashThread",
    DEMO_THREAD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &xDemoTask );

    if ( xReturned != pdPASS )
    {
        vTaskDelete( xDemoTask );
        return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}

