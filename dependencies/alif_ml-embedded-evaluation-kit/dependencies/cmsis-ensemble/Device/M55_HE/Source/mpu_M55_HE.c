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
 * @file     mpu_M55_HE.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     14-December-2021
 * @brief    MPU Configuration Source
 *
 *           This file load the MPU regions from the table and provides the function to enable it.
 * @bug      None.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#if defined (M55_HE)
  #include "M55_HE.h"
#else
  #error device not specified!
#endif


/* Public functions ----------------------------------------------------------*/
/**
 * @brief  Load the MPU regions from the given table
 * @note   This function loads the region and also sets the
 *         attributes for the regions. Setting this function as weak,
 *         so that User can override from application.
 * @param  None
 * @retval None
 */
__attribute__ ((weak))
void MPU_Load_Regions(void)
{
    static const ARM_MPU_Region_t mpu_table[] __STARTUP_RO_DATA_ATTRIBUTE = {
        {   /* EXTSYS0 TCM : This should be added based on the Device and application
                              requirements. */
            .RBAR = ARM_MPU_RBAR(0x50000000UL, ARM_MPU_SH_OUTER, 0UL, 1UL, 1UL),
            .RLAR = ARM_MPU_RLAR(0x50FFFFFFUL, 0UL)
        },
        {
	    /* Low Power Peripheral Regions */
            .RBAR = ARM_MPU_RBAR(0x70000000UL, ARM_MPU_SH_NON, 0UL, 1UL, 1UL),
            .RLAR = ARM_MPU_RLAR(0x72FFFFFFUL, 1UL)
        },
    };

    /* Define the possible Attribute regions */
    ARM_MPU_SetMemAttr(0UL, ARM_MPU_ATTR( /* Attr0, Normal Memory, Non-Cacheable */
                            ARM_MPU_ATTR_NON_CACHEABLE,
                            ARM_MPU_ATTR_NON_CACHEABLE));

    ARM_MPU_SetMemAttr(1UL, ARM_MPU_ATTR(   /* Attr1, Device Memory */
                            ARM_MPU_ATTR_DEVICE,
                            ARM_MPU_ATTR_DEVICE_nGnRE));

    /* Load the regions from the table */
    ARM_MPU_Load(0U, &mpu_table[0], 2U);
}

/**
 * @brief  Clear all the MPU registers
 * @note   This function disables the MPU and clear
 *         all the existing regions.
 * @param  None
 * @retval None
 */
static void MPU_Clear_All_Regions(void)
{
    MPU_Type* mpu = MPU;
    /* Retrieve the number of regions */
    uint32_t num_regions = (mpu->TYPE >> 8);
    uint32_t cnt;

    ARM_MPU_Disable();

    for(cnt = 0U; cnt < num_regions; cnt++)
            ARM_MPU_ClrRegion(cnt);
}

/**
 * @brief  Configure the MPU.
 * @note   This function disables the MPU and loads the regions
 *         from the table. Once it is loaded, MPU is enabled.
 * @param  None
 * @retval None
 */
void MPU_Setup(void)
{
#define MPU_CONTROL (MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk)

    MPU_Clear_All_Regions();
    MPU_Load_Regions();
    ARM_MPU_Enable(MPU_CONTROL);
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
