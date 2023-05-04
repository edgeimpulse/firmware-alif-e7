/**
 * @file services_host_power.c
 *
 * @brief Cryptocell services service source file
 * @ingroup host_services
 * @par
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "services_lib_api.h"
#include "services_lib_protocol.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

/**
 * @brief Function to request stop mode
 * @param services_handle
 * @param power_profile
 * @param override
 * @return
 */
uint32_t SERVICES_power_stop_mode_req(uint32_t services_handle,
                                      services_power_profile_t power_profile,
                                      bool override)
{
  stop_mode_request_svc_t * p_svc = (stop_mode_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(stop_mode_request_svc_t));

  p_svc->power_profile = power_profile;
  p_svc->override      = override;

  return SERVICES_send_request(services_handle, 
                               SERVICE_POWER_STOP_MODE_REQ_ID, NULL);
}

/**
 * @brief Function to configure EWIC source
 * @param services_handle
 * @param ewic_source
 * @param value
 * @return
 */
uint32_t SERVICES_power_ewic_config(uint32_t services_handle,
                                    uint32_t ewic_source,
                                    services_power_profile_t power_profile)
{
  ewic_config_request_svc_t * p_svc = (ewic_config_request_svc_t *)
    SERVICES_prepare_packet_buffer(sizeof(ewic_config_request_svc_t));

  p_svc->send_ewic_source = ewic_source;
  p_svc->power_profile    = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_EWIC_CONFIG_REQ_ID, NULL);
}

/**
 * @brief Function to configure VBAT wake up source
 * @param services_handle
 * @param ewic_source
 * @param value
 * @return
 */
uint32_t SERVICES_power_wakeup_config(uint32_t services_handle,
                                      uint32_t vbat_wakeup_source,
                                      services_power_profile_t power_profile)
{
  vbat_wakeup_config_request_svc_t * p_svc =
    (vbat_wakeup_config_request_svc_t *)
    SERVICES_prepare_packet_buffer(sizeof(vbat_wakeup_config_request_svc_t));

  p_svc->send_vbat_wakeup_source = vbat_wakeup_source;
  p_svc->power_profile           = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID, NULL);
}

/**
 * @brief Function to configure memory retention
 * @param services_handle
 * @param mem_retention
 * @param power_profile
 * @return
 */
uint32_t
SERVICES_power_mem_retention_config(uint32_t services_handle,
                                    uint32_t mem_retention,
                                    services_power_profile_t power_profile)
{
  mem_ret_config_request_svc_t * p_svc =
      (mem_ret_config_request_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(mem_ret_config_request_svc_t));

  p_svc->send_mem_retention = mem_retention;
  p_svc->power_profile      = power_profile;

  return SERVICES_send_request(services_handle,
                               SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID,
                               NULL);
}

