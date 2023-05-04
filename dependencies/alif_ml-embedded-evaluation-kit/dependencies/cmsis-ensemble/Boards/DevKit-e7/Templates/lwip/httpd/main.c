/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include <stdio.h>
#include <stdint.h>

#include "ethernetif.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/apps/httpd.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"

#include "Driver_PINMUX_AND_PINPAD.h"

static void net_init (void);
static void net_periodic (uint32_t tick);
static void net_timer (uint32_t *tick);

static struct netif netif;

#define HTTP_DEMO_DEBUG

static int pin_mux_init(void)
{
  int32_t ret;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_0, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_3, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_4, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_5, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_6, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_7, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_8, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_9, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  ret = PINMUX_Config(PORT_NUMBER_1, PIN_NUMBER_10, PINMUX_ALTERNATE_FUNCTION_3);
  if (ret)
    return -1;

  return 0;
}

/* Initialize lwIP */
static void net_init (void)
{
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  lwip_init ();

#if LWIP_DHCP
  ipaddr.addr  = IPADDR_ANY;
  netmask.addr = IPADDR_ANY;
  gw.addr      = IPADDR_ANY;
#else
  IP4_ADDR (&ipaddr,  192,168,1,11);
  IP4_ADDR (&netmask, 255,255,255,0);
  IP4_ADDR (&gw,      192,168,1,1);
#endif

  /* Add the network interface to the netif_list. */
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /* Register the default network interface. */
  netif_set_default(&netif);
  netif_set_up(&netif);

#if LWIP_DHCP
  dhcp_start (&netif);
#endif
}

/* Link check */
static void net_periodic (uint32_t tick)
{
  static uint32_t old_tick;
  static ip4_addr_t ip = {0};

  if (tick == old_tick) {
    return;
  }
  old_tick = tick;

  ethernetif_check_link (&netif);

#ifdef HTTP_DEMO_DEBUG
  if (netif_is_link_up(&netif)) {
    /* Print IP address if debug is enabled */
    if (ip.addr != netif.ip_addr.addr) {
      ip.addr = netif.ip_addr.addr;
      printf ("Link up. IP: %s.\n", ipaddr_ntoa(&ip));
    }
  }
#endif
}

/* Tick timer callback */
static void net_timer (uint32_t *tick) {
  ++*tick;
}

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
void app_main (void *argument)
{
  static uint32_t tick;
  int32_t ret;
  osTimerId_t id;

  ret = pin_mux_init ();

  if (ret < 0) {
#ifdef HTTP_DEMO_DEBUG
    printf("pin mux initialization failed\n");
#endif
    goto error;
  }

  /* Create tick timer, tick interval = 500ms */
  id = osTimerNew ((osTimerFunc_t)&net_timer, osTimerPeriodic, &tick, NULL);
  osTimerStart (id, 500);


  net_init ();

  httpd_init ();

  while (1) {
    /* check for packet reception */
    ethernetif_poll (&netif);
    /* handle system timers for lwIP */
    sys_check_timeouts ();
    net_periodic (tick);
  }
error:
  while(1);
}

int main (void)
{
  // System Initialization
  SystemCoreClockUpdate();

  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
