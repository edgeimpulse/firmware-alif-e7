/**
 * @file services_host_handler.c
 * @brief Services handler file
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * @ingroup host_services
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "services_lib_linux.h"
#include "services_lib_protocol.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define UNUSED(x) (void)(x)

#if 0

#include <linux/rpmsg.h>

#else

struct rpmsg_endpoint_info
{
  const char * unused0;
  uint32_t unused1;
  uint32_t unused2;
};

#define RPMSG_CREATE_EPT_IOCTL  0

#endif


#define MHU_NUMBER                 3
#define MHU_NUMBER_OF_CHANNELS_MAX 2

static services_lib_t s_services_host = {0};

struct rpmsg_endpoint_info s_mhu_eptinfo[MHU_NUMBER] = {
    {"se_mhu0", 0XFFFFFFFF, 0xFFFFFFFF},
    {"se_mhu1", 0XFFFFFFFF, 0xFFFFFFFF},
    {"se_mhu2", 0XFFFFFFFF, 0xFFFFFFFF}
};
static int s_rpmsg_ctrl_fd[MHU_NUMBER];
static int s_rpmsg_fd[MHU_NUMBER];

/**
 * @brief Function to initialize the services library
 * @param init_params Initialization parameters
 */
void SERVICES_initialize(services_lib_t * init_params)
{
  s_services_host.packet_buffer_address = init_params->packet_buffer_address;
  s_services_host.fn_print_msg = init_params->fn_print_msg;

  memset(&s_rpmsg_ctrl_fd, 0, sizeof(s_rpmsg_ctrl_fd));
  memset(&s_rpmsg_fd, 0, sizeof(s_rpmsg_fd));
}

/**
 * @brief prepare the packet buffer ()
 * @return
 */
uintptr_t SERVICES_prepare_packet_buffer(uint32_t size)
{
  memset((void *)(uintptr_t)s_services_host.packet_buffer_address, 0x0, size);
  return s_services_host.packet_buffer_address;
}

/**
 * @fn      uint32_t SERVICES_register_channel(uint32_t mhu_id,
 *                                             uint32_t channel_number)
 * @brief   Register a MHU communication channel with the Services library
 * @param   mhu_id
 * @param   channel_number
 * @return  Handle to be used in subsequent service calls
 */
uint32_t SERVICES_register_channel(uint32_t mhu_id, 
                                   uint32_t channel_number)
{
  if (mhu_id >= MHU_NUMBER)
  {
    return SERVICES_LIB_ERROR;
  }

  char buf[40];
  sprintf(buf, "/dev/rpmsg_ctrl%d", mhu_id);
  int fd_ctrl = open(buf, O_RDWR);
  if (fd_ctrl < 0)
  {
    return SERVICES_LIB_ERROR;
  }
  int status = ioctl(fd_ctrl, RPMSG_CREATE_EPT_IOCTL, &s_mhu_eptinfo[mhu_id]);
  if (status < 0)
  {
    close(fd_ctrl);
    return SERVICES_LIB_ERROR;
  }
  sprintf(buf, "/dev/rpmsg%d", mhu_id);
  int fd_msg = open(buf, O_RDWR);
  if (fd_msg < 0)
  {
    close(fd_ctrl);
    return SERVICES_LIB_ERROR;
  }

  s_rpmsg_ctrl_fd[mhu_id] = fd_ctrl;
  s_rpmsg_fd[mhu_id] = fd_msg;

  // TODO: handle channel_number;
  return mhu_id;
}

/**
 * @fn      uint32_t SERVICES_unregister_channel(uint32_t mhu_id,
 *                                             uint32_t channel_number)
 * @brief   Unregister a MHU communication channel with the Services library
 * @param   mhu_id
 * @param   channel_number
 * @return
 */
void SERVICES_unregister_channel(uint32_t mhu_id,
                                 uint32_t channel_number)
{
  if (mhu_id >= MHU_NUMBER) return;

  if (s_rpmsg_ctrl_fd[mhu_id] != 0)
  {
    close(s_rpmsg_ctrl_fd[mhu_id]);
    s_rpmsg_ctrl_fd[mhu_id] = 0;
  }
  if (s_rpmsg_fd[mhu_id] != 0)
  {
    close(s_rpmsg_fd[mhu_id]);
    s_rpmsg_fd[mhu_id] = 0;
  }
}

/**
 * @brief Send services request to MHU
 * @param services_handle
 * @param service_id
 * @param service_data
 * @param callback
 * @return
 */
uint32_t SERVICES_send_request(uint32_t services_handle, 
                               uint16_t service_id, 
                               SERVICES_sender_callback callback)
{
  UNUSED(callback);

  if (services_handle >= MHU_NUMBER) return SERVICES_LIB_ERROR;

  s_services_host.fn_print_msg("[SERVICESLIB] Send service request 0x%x\n", 
                               service_id);

  // Initialize the service request common header
  service_header_t * p_header = (service_header_t *)(uintptr_t)s_services_host.packet_buffer_address;
  p_header->send_service_id = service_id;
  p_header->send_flags = 0;
  
  // Send the MHU message
  uint32_t mhu_id = services_handle / MHU_NUMBER_OF_CHANNELS_MAX;
  int status = write(s_rpmsg_fd[mhu_id],
                     &s_services_host.packet_buffer_address,
                     sizeof(uint32_t));
  
  if (status < 0) return SERVICES_LIB_ERROR;

  // Read the response MHU message
  uint32_t rec_phy_addr;
  status = read(s_rpmsg_fd[mhu_id], &rec_phy_addr, sizeof(uint32_t));

  if (status < 0) return SERVICES_LIB_ERROR;

  if (s_services_host.packet_buffer_address == rec_phy_addr)
  {
    s_services_host.fn_print_msg("[SERVICESLIB] rx_msg=0x%x \n", rec_phy_addr);
  }

  return p_header->resp_error_code;
}
