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
 * @file     app_issi_flash.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     05-Dec-2022
 * @brief    Application to set up flash in XIP mode and jump to the flash
 *           image address.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "Driver_PINMUX_AND_PINPAD.h"
#include "setup_flash_xip.h"
#include "ospi_xip_user.h"

#if OSPI_XIP_INSTANCE == OSPI0
#define XIP_IMAGE_ADDRESS    OSPI0_BASE
#define XIP_IMAGE_MAX_SIZE   OSPI0_SIZE
#else
#define XIP_IMAGE_ADDRESS    OSPI1_BASE
#define XIP_IMAGE_MAX_SIZE   OSPI1_SIZE
#endif

typedef void (*pfun) (void);

static pfun EntryPoint;

/**
  \fn        static int32_t setup_PinMUX(void)
  \brief     Set up PinMUX and PinPAD
  \param[in] none
  \return    -1: If any error
 */
static int32_t setup_pinmux(void)
{
    int32_t ret;

#if OSPI_XIP_INSTANCE == OSPI0
    ret = PINMUX_Config(OSPI0_D0_PORT, OSPI0_D0_PIN, OSPI0_D0_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D1_PORT, OSPI0_D1_PIN, OSPI0_D1_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D2_PORT, OSPI0_D2_PIN, OSPI0_D2_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D3_PORT, OSPI0_D3_PIN, OSPI0_D3_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D4_PORT, OSPI0_D4_PIN, OSPI0_D4_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D5_PORT, OSPI0_D5_PIN, OSPI0_D5_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D6_PORT, OSPI0_D6_PIN, OSPI0_D6_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_D7_PORT, OSPI0_D7_PIN, OSPI0_D7_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_RXDS_PORT, OSPI0_RXDS_PIN, OSPI0_RXDS_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_SCLK_PORT, OSPI0_SCLK_PIN, OSPI0_SCLK_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI0_CS_PORT, OSPI0_CS_PIN, OSPI0_CS_PIN_FUNCTION);
    if (ret)
        return -1;

    /* Configure pad control registers */
    ret = PINPAD_Config(OSPI0_D0_PORT, OSPI0_D0_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D1_PORT, OSPI0_D1_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D2_PORT, OSPI0_D2_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D3_PORT, OSPI0_D3_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D4_PORT, OSPI0_D4_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D5_PORT, OSPI0_D5_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D6_PORT, OSPI0_D6_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_D7_PORT, OSPI0_D7_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI0_RXDS_PORT, OSPI0_RXDS_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
#else
    ret = PINMUX_Config(OSPI1_D0_PORT, OSPI1_D0_PIN, OSPI1_D0_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D1_PORT, OSPI1_D1_PIN, OSPI1_D1_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D2_PORT, OSPI1_D2_PIN, OSPI1_D2_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D3_PORT, OSPI1_D3_PIN, OSPI1_D3_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D4_PORT, OSPI1_D4_PIN, OSPI1_D4_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D5_PORT, OSPI1_D5_PIN, OSPI1_D5_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D6_PORT, OSPI1_D6_PIN, OSPI1_D6_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_D7_PORT, OSPI1_D7_PIN, OSPI1_D7_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_RXDS_PORT, OSPI1_RXDS_PIN, OSPI1_RXDS_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_SCLK_PORT, OSPI1_SCLK_PIN, OSPI1_SCLK_PIN_FUNCTION);
    if (ret)
        return -1;
    ret = PINMUX_Config(OSPI1_CS_PORT, OSPI1_CS_PIN, OSPI1_CS_PIN_FUNCTION);
    if (ret)
        return -1;

    /* Configure pad control registers */
    ret = PINPAD_Config(OSPI1_D0_PORT, OSPI1_D0_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D1_PORT, OSPI1_D1_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D2_PORT, OSPI1_D2_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D3_PORT, OSPI1_D3_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D4_PORT, OSPI1_D4_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D5_PORT, OSPI1_D5_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D6_PORT, OSPI1_D6_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_D7_PORT, OSPI1_D7_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
    ret = PINPAD_Config(OSPI1_RXDS_PORT, OSPI1_RXDS_PIN, PAD_FUNCTION_READ_ENABLE);
    if (ret)
        return -1;
#endif
    return 0;
}

/**
  \fn        void setup_MPU(void)
  \brief     Set up MPU for XIP
  \param[in] none
  \return    none
 */
static void setup_mpu(void)
{
#if OSPI_XIP_INSTANCE == OSPI0
    static const ARM_MPU_Region_t table[] =
    {
        {
            /* Configure OSPI0 region as normal memory */
            .RBAR = ARM_MPU_RBAR(OSPI0_BASE, ARM_MPU_SH_NON, 1UL, 1UL, 0UL),
            .RLAR = ARM_MPU_RLAR(OSPI0_BASE | OSPI0_SIZE, 1UL)
        },
    };
#else
    static const ARM_MPU_Region_t table[] =
    {
        {
            /* Configure OSPI1 region as normal memory */
            .RBAR = ARM_MPU_RBAR(OSPI1_BASE, ARM_MPU_SH_NON, 1UL, 1UL, 0UL),
            .RLAR = ARM_MPU_RLAR(OSPI1_BASE | OSPI1_SIZE, 1UL)
        },
    };
#endif

#define MPU_CONTROL (MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk)
#define RNR 0x1

    ARM_MPU_ClrRegion(RNR);

    /* Define the Attributes for mem region */
    ARM_MPU_SetMemAttr(1UL, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1UL, 1UL, 1UL, 0UL),
                                         ARM_MPU_ATTR_MEMORY_(1UL, 1UL, 1UL, 0UL))); /* Attr1, Normal memory */

    /* Load the regions from the table */
    ARM_MPU_Load(RNR, &table[0], 1U);
}

/**
  \fn        int main ()
  \brief     Main Function
  \param[in] none
  \return    0 : Success
 */
int main ()
{
    int32_t ret;

    ret = setup_pinmux();

    if (ret)
    {
        while(1);
    }

    ret = setup_flash_xip();

    if (ret)
    {
        while(1);
    }

    setup_mpu();

    /* Read the Reset_Handler address from the vector table of the image in OSPI */
    EntryPoint = (pfun)(*(__IO uint32_t *)(XIP_IMAGE_ADDRESS + 4));

    /* Jump to the Reset_Handler */
    EntryPoint();

    return 0;
}
