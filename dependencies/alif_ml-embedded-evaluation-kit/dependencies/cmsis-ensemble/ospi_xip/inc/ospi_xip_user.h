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
 * @file     ospi_xip_user.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     05-Dec-2022
 * @brief    User configuration parameters for flash XIP application.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef OSPI_XIP_USER_H_
#define OSPI_XIP_USER_H_

#ifdef  __cplusplus
extern "C"
{
#endif
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> OSPI XIP Configuration
// =========================================

#define OSPI0                                    0U
#define OSPI1                                    1U


/**
  \def OSPI_XIP_INSTANCE
  \brief OSPI instance configuration. Can be set to either \ref OSPI0 or \ref OSPI1.
*/

//   <o OSPI_XIP_INSTANCE> OSPI instance selection
//      <OSPI0=>  0
//      <OSPI1=>  1
//   <i> OSPI instance selection

#define OSPI_XIP_INSTANCE                        OSPI0

/**
  \def OSPI_CLOCK_MHZ
  \brief Required ospi clock output (sclk_out).
*/

//   <o> OSPI Clock (in MHz)
//   <i> Defines the frequency of sclk_out of SPI controller.
//   <i> Default:5

#define OSPI_CLOCK_MHZ                           30
#define OSPI_CLOCK                               (OSPI_CLOCK_MHZ * 1000000)

// </h>
//------------- <<< end of configuration section >>> ---------------------------

/* Pin/Pad mux and configuration for OSPI0 pads */
#define OSPI0_D0_PORT                            PORT_NUMBER_1
#define OSPI0_D0_PIN                             PIN_NUMBER_16
#define OSPI0_D0_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_D1_PORT                            PORT_NUMBER_1
#define OSPI0_D1_PIN                             PIN_NUMBER_17
#define OSPI0_D1_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_D2_PORT                            PORT_NUMBER_1
#define OSPI0_D2_PIN                             PIN_NUMBER_18
#define OSPI0_D2_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_D3_PORT                            PORT_NUMBER_1
#define OSPI0_D3_PIN                             PIN_NUMBER_19
#define OSPI0_D3_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_D4_PORT                            PORT_NUMBER_1
#define OSPI0_D4_PIN                             PIN_NUMBER_20
#define OSPI0_D4_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_D5_PORT                            PORT_NUMBER_1
#define OSPI0_D5_PIN                             PIN_NUMBER_21
#define OSPI0_D5_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI0_D6_PORT                            PORT_NUMBER_1
#define OSPI0_D6_PIN                             PIN_NUMBER_22
#define OSPI0_D6_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI0_D7_PORT                            PORT_NUMBER_1
#define OSPI0_D7_PIN                             PIN_NUMBER_23
#define OSPI0_D7_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_SCLK_PORT                          PORT_NUMBER_1
#define OSPI0_SCLK_PIN                           PIN_NUMBER_25
#define OSPI0_SCLK_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_RXDS_PORT                          PORT_NUMBER_1
#define OSPI0_RXDS_PIN                           PIN_NUMBER_26
#define OSPI0_RXDS_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_3

#define OSPI0_CS_PORT                            PORT_NUMBER_2
#define OSPI0_CS_PIN                             PIN_NUMBER_6
#define OSPI0_CS_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI0_SCLKN_PORT                         PORT_NUMBER_2
#define OSPI0_SCLKN_PIN                          PIN_NUMBER_7
#define OSPI0_SCLKN_PIN_FUNCTION                 PINMUX_ALTERNATE_FUNCTION_4

/* Pin/Pad mux and configuration for OSPI1 pads */
#define OSPI1_D0_PORT                            PORT_NUMBER_2
#define OSPI1_D0_PIN                             PIN_NUMBER_8
#define OSPI1_D0_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D1_PORT                            PORT_NUMBER_2
#define OSPI1_D1_PIN                             PIN_NUMBER_9
#define OSPI1_D1_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D2_PORT                            PORT_NUMBER_2
#define OSPI1_D2_PIN                             PIN_NUMBER_10
#define OSPI1_D2_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D3_PORT                            PORT_NUMBER_2
#define OSPI1_D3_PIN                             PIN_NUMBER_11
#define OSPI1_D3_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D4_PORT                            PORT_NUMBER_2
#define OSPI1_D4_PIN                             PIN_NUMBER_12
#define OSPI1_D4_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D5_PORT                            PORT_NUMBER_2
#define OSPI1_D5_PIN                             PIN_NUMBER_13
#define OSPI1_D5_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_D6_PORT                            PORT_NUMBER_2
#define OSPI1_D6_PIN                             PIN_NUMBER_14
#define OSPI1_D6_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI1_D7_PORT                            PORT_NUMBER_2
#define OSPI1_D7_PIN                             PIN_NUMBER_15
#define OSPI1_D7_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_RXDS_PORT                          PORT_NUMBER_2
#define OSPI1_RXDS_PIN                           PIN_NUMBER_16
#define OSPI1_RXDS_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_SCLK_PORT                          PORT_NUMBER_2
#define OSPI1_SCLK_PIN                           PIN_NUMBER_19
#define OSPI1_SCLK_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_4

#define OSPI1_CS_PORT                            PORT_NUMBER_2
#define OSPI1_CS_PIN                             PIN_NUMBER_17
#define OSPI1_CS_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define OSPI1_SCLKN_PORT                         PORT_NUMBER_2
#define OSPI1_SCLKN_PIN                          PIN_NUMBER_18
#define OSPI1_SCLKN_PIN_FUNCTION                 PINMUX_ALTERNATE_FUNCTION_4

#ifdef  __cplusplus
}
#endif

#endif /* OSPI_XIP_USER_H_ */

