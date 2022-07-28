/*
 * nx_eth_user.h: User configuration for the NetXDuo Ethernet driver
 *
 * Author   : Silesh C V <silesh@alifsemi.com>
 *
 * Copyright (C) 2021 ALIF SEMICONDUCTOR
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
  \file nx_eth_user.h
  \brief User configuration for NetXDuo network driver for Designware QOS MAC.
 */
#ifndef NX_ETH_USER_H
#define NX_ETH_USER_H

#include "tx_api.h"

#ifdef  __cplusplus
extern "C"
{
#endif
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> NetXDuo Ethernet driver Configuration
// =========================================


/**
  \def NX_DRIVER_PHY_POLLING_INTERVAL_SHORT
  \brief The interval in which the driver polls for the link status when the link is down.
*/

//   <o> PHY polling interval (Short) [Seconds]
//   <i> Defines the interval in seconds in which the driver polls the link status when the link is down.
//   <i> Default:1

#define NX_DRIVER_PHY_POLLING_INTERVAL_SHORT    (1)

/**
  \def NX_DRIVER_PHY_POLLING_INTERVAL_LONG
  \brief The interval in which the driver polls for the link status when the link is up.
*/

//   <o> PHY polling interval (Long) [Seconds]
//   <i> Defines the interval in seconds in which the driver polls the link status when the link is up.
//   <i> Default:15

#define NX_DRIVER_PHY_POLLING_INTERVAL_LONG     (15)

/**
  \def PHY_POLL_THREAD_PRIORITY
  \brief ThreadX priority of the thread responsible for polling the link status.
*/

//   <o> PHY polling thread priority
//   <i> Defines the Azure RTOS thread priority of the PHY polling thread.
//   <i> Default:10

#define PHY_POLL_THREAD_PRIORITY                10

/**
  \def MAC_ADDRESS_HIGH
  \brief The higher 2 bytes of the MAC Address.
*/

//   <o> MAC Address (Higher 2 bytes)
//   <i> Defines the higher 2 bytes of the MAC address to be used for the Ethernet interface.
//   <i> Default:0x0201

#define MAC_ADDRESS_HIGH                        (0x0201)

/**
  \def MAC_ADDRESS_LOW
  \brief The lower 4 bytes of the MAC Address.
*/

//   <o> MAC Address (Lower 4 bytes)
//   <i> Defines the lower 4 bytes of the MAC address to be used for the Ethernet interface.
//   <i> Default:0x02030405

#define MAC_ADDRESS_LOW                         (0x02030405)

/**
  \def ETH_PHY_ADDRESS
  \brief The PHY address.
*/

//   <o> PHY address <0-31>
//   <i> Defines the address of the Ethernet PHY device. Used during MDIO communications.
//   <i> Default:1

#define ETH_PHY_ADDRESS                         1

/**
  \def ETH_AN_CONFIG
  \brief Autonegotiation configuration. Can be set to either \ref AN_DISABLE or \ref AN_ENABLE.
*/

//   <o ETH_AN_CONFIG> Autonegotiation configuration
//      <AN_ENABLE=>     Enable Auto negotiation
//      <AN_DISABLE=>    Disable Auto neogtiation
//   <i> Autonegotiation configuraton

#define ETH_AN_CONFIG                           AN_DISABLE


/**
  \def ETH_SPEED_CONFIG
  \brief Ethernet link speed configuration. Can be set to either \ref ETH_SPEED_10M or \ref ETH_SPEED_100M.
*/

//   <o ETH_SPEED_CONFIG> Speed configuration
//      <ETH_SPEED_10M=>  10mbps
//      <ETH_SPEED_100M=> 100mbps
//   <i> Link speed configuration

#define ETH_SPEED_CONFIG                        ETH_SPEED_10M

/**
  \def ETH_DUPLEX_CONFIG
  \brief Ethernet link duplex configuration. Can be set to either \ref ETH_DUPLEX_FULL or \ref ETH_DUPLEX_HALF..
*/

//   <o ETH_DUPLEX_CONFIG> Duplex configuration
//      <ETH_DUPLEX_FULL=> Full Duplex
//      <ETH_DUPLEX_HALF=> Half Duplex
//   <i> Link Duplex configuration

#define ETH_DUPLEX_CONFIG                       ETH_DUPLEX_FULL

// </h>
//------------- <<< end of configuration section >>> ---------------------------

/* Pin/Pad mux and configuration for Ethernet pads */
#define ETH_RXD0_PORT                           PORT_NUMBER_1
#define ETH_RXD0_PIN                            PIN_NUMBER_0
#define ETH_RXD0_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_RXD1_PORT                           PORT_NUMBER_1
#define ETH_RXD1_PIN                            PIN_NUMBER_1
#define ETH_RXD1_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_CRSDV_PORT                          PORT_NUMBER_1
#define ETH_CRSDV_PIN                           PIN_NUMBER_2
#define ETH_CRSDV_FUNCTION                      PINMUX_ALTERNATE_FUNCTION_3

#define ETH_RST_PORT                            PORT_NUMBER_1
#define ETH_RST_PIN                             PIN_NUMBER_3
#define ETH_RST_FUNCTION                        PINMUX_ALTERNATE_FUNCTION_3

#define ETH_TXD0_PORT                           PORT_NUMBER_1
#define ETH_TXD0_PIN                            PIN_NUMBER_4
#define ETH_TXD0_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_TXD1_PORT                           PORT_NUMBER_1
#define ETH_TXD1_PIN                            PIN_NUMBER_5
#define ETH_TXD1_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_TXEN_PORT                           PORT_NUMBER_1
#define ETH_TXEN_PIN                            PIN_NUMBER_6
#define ETH_TXEN_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_IRQ_PORT                            PORT_NUMBER_1
#define ETH_IRQ_PIN                             PIN_NUMBER_7
#define ETH_IRQ_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#define ETH_REFCLK_PORT                         PORT_NUMBER_1
#define ETH_REFCLK_PIN                          PIN_NUMBER_8
#define ETH_REFCLK_PIN_FUNCTION                 PINMUX_ALTERNATE_FUNCTION_3

#define ETH_MDIO_PORT                           PORT_NUMBER_1
#define ETH_MDIO_PIN                            PIN_NUMBER_9
#define ETH_MDIO_PIN_FUNCTION                   PINMUX_ALTERNATE_FUNCTION_3

#define ETH_MDC_PORT                            PORT_NUMBER_1
#define ETH_MDC_PIN                             PIN_NUMBER_10
#define ETH_MDC_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_3

#ifdef  __cplusplus
}
#endif

#endif /* NX_ETH_USER_H */
