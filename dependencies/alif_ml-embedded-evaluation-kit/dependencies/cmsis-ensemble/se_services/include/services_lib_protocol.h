/**
 * @file services_lib_protocol.h
 *
 * @brief Private header file for services library
 *
 * @par
 * @ingroup services
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef __SERVICES_LIB_PROTOCOL_H__
#define __SERVICES_LIB_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/
/**
 * Version   JIRA         Description
 * 0.0.21   SE-1548       Added build support for SPARK
 * 0.0.20   SE-1512       Updated ALIF License
 * 0.0.19   SE-1440       Adding CMake build
 * 0.0.18   SE-1463       Stop mode power profile
 * 0.0.17   SE-1443       Fixed unimplemented function warnings in GCC build
 * 0.0.16   SE-<>         Update to RTSS V0.4.1
 * 0.0.15   SE-1423       Added examples installation, fixed build flags
 * 0.0.14   SE-1370       Addition of XIP examples
 * 0.0.13   SE-1272       Examples common directory
 * 0.0.12   SE-1290       get otp data (for real!)
 * 0.0.11   SE-1265       get all toc info
 *          SE-859        get otp data
 * 0.0.10   SE-1246       standardized variables for send/resp
 * 0.0.9    SE-1214       bounds checks for UART prints
 * 0.0.8    SE-1099       Added firmware version id
 * 0.0.7    SE-1165       mbed TLS accelerators
 * 0.0.6    SE-1173       Added enable / disable debug status
 * 0.0.5    SE-1144       Service API error code added
 * 0.0.4    SE-700        Service BOOT reset added
 * 0.0.3    SE-827        RPC Parameter changes
 * 0.0.2    SE-708        First refactoring
 * 0.0.1                  First implementation
 */
#define SE_SERVICES_VERSION_STRING "0.0.21"

#define IMAGE_NAME_LENGTH           8
#define VERSION_RESPONSE_LENGTH     80

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/**
 * @enum SERVICE_ID_t Service Identifiers (SID)
 */
typedef enum
{
  /**
   * Maintenance Services
   */
  SERVICE_MAINTENANCE_START  = 0,                                     /**< SERVICE_MAINTENANCE_START        */
  SERVICE_MAINTENANCE_HEARTBEAT_ID = SERVICE_MAINTENANCE_START,       /**< SERVICE_MAINTENANCE_HEARTBEAT_ID */
  SERVICE_MAINTENANCE_RTC_ID,                                         /**< SERVICE_MAINTENANCE_RTC_ID       */
  SERVICE_MAINTENANCE_END    = 99,                                    /**< SERVICE_MAINTENANCE_END          */

  /**
   * Application Services
   */
  SERVICE_APPLICATION_START  = 100,                                   /**< SERVICE_APPLICATION_START                  */
  SERVICE_APPLICATION_CLOCK_MANAGEMENT_ID = SERVICE_APPLICATION_START,/**< SERVICE_APPLICATION_CLOCK_MANAGEMENT_ID    */
  SERVICE_APPLICATION_PINMUX_ID,                                      /**< SERVICE_APPLICATION_PINMUX_ID              */
  SERVICE_APPLICATION_PAD_CONTROL_ID,                                 /**< SERVICE_APPLICATION_PAD_CONTROL            */
  SERVICE_APPLICATION_FIRMWARE_VERSION_ID,                            /**< SERVICE_APPLICATION_FIRMWARE_VERSION_ID    */
  SERVICE_APPLICATION_UART_WRITE_ID,                                  /**< SERVICE_APPLICATION_UART_WRITE_ID          */
  SERVICE_APPLICATION_OSPI_WRITE_KEY_ID,                              /**< SERVICE_APPLICATION_OSPI_WRITE_KEY_ID      */
  SERVICE_APPLICATION_END    = 199,                                   /**< SERVICE_APPLICATION_END                    */

  /**
   * System Management Services
   */
  SERVICE_SYSTEM_MGMT_START  = 200,                                   /**< SERVICE_SYSTEM_MGMT_START                  */
  SERVICE_SYSTEM_MGMT_GET_TOC_VERSION = SERVICE_SYSTEM_MGMT_START,    /**< SERVICE_SYSTEM_MGMT_GET_TOC_VERSION        */
  SERVICE_SYSTEM_MGMT_GET_TOC_NUMBER,                                 /**< SERVICE_SYSTEM_MGMT_GET_TOC_NUMBER         */
  SERVICE_SYSTEM_MGMT_GET_TOC_FLAGS,                                  /**< SERVICE_SYSTEM_MGMT_GET_TOC_FLAGS          */
  SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_ID,                             /**< SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_ID     */
  SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_NAME,                           /**< SERVICE_SYSTEM_MGMT_GET_TOC_VIA_CPU_NAME   */
  SERVICE_SYSTEM_MGMT_GET_TOC_INFO,                                   /**< SERVICE_SYSTEM_MGMT_GET_TOC_INFO           */
  SERVICE_SYSTEM_MGMT_GET_OTP_INFO,                                   /**< SERVICE_SYSTEM_MGMT_GET_OTP_INFO           */
  SERVICE_SYSTEM_MGMT_GET_DEVICE_PART_NUMBER,                         /**< SERVICE_SYSTEM_MGMT_GET_DEVICE_PART_NUMBER */
  SERVICE_SYSTEM_MGMT_SET_CAPABILITIES_DEBUG,                         /**< SERVICE_SYSTEM_MGMT_SET_CAPABILITIES_DEBUG */
  SERVICE_SYSTEM_MGMT_READ_OTP,                                       /**< SERVICE_SYSTEM_MGMT_OTP_READ               */
  SERVICE_SYSTEM_MGMT_END    = 299,                                   /**< SERVICE_SYSTEM_MGMT_END                    */

  /**
   * Power Services
   */
  SERVICE_POWER_START        = 300,                                   /**< SERVICE_POWER_START */
  SERVICE_POWER_STOP_MODE_REQ_ID,                                     /**< SERVICE_POWER_STOP_MODE_REQ_ID */
  SERVICE_POWER_EWIC_CONFIG_REQ_ID,                                   /**< SERVICE_POWER_EWIC_CONFIG_REQ_ID */
  SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID,                            /**< SERVICE_POWER_VBAT_WAKEUP_CONFIG_REQ_ID */
  SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID,                          /**< SERVICE_POWER_MEM_RETENTION_CONFIG_REQ_ID */
  SERVICE_POWER_END          = 399,                                   /**< SERVICE_POWER_END */

  /**
   * Cryptocell / Security Services
   */
  SERVICE_CRYPTOCELL_START   = 400,                                   /**< SERVICE_CRYPTOCELL_START */
  SERVICE_CRYPTOCELL_GET_RND =  SERVICE_CRYPTOCELL_START,             /**< SERVICE_CRYPTOCELL_GET_RND */
  SERVICE_CRYPTOCELL_GET_LCS,                                         /**< SERVICE_CRYPTOCELL_GET_LCS */
  SERVICE_CRYPTOCELL_MBEDTLS_AES_INIT,                                /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_INIT */
  SERVICE_CRYPTOCELL_MBEDTLS_AES_SET_KEY,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_SET_KEY */
  SERVICE_CRYPTOCELL_MBEDTLS_AES_CRYPT,                               /**< SERVICE_CRYPTOCELL_MBEDTLS_AES_CRYPT_ECB */
  SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_SET_KEY,                         /**< SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_SET_KEY */
  SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_CRYPT,                           /**< SERVICE_CRYPTOCELL_MBEDTLS_CCM_GCM_CRYPT */
  SERVICE_CRYPTOCELL_MBEDTLS_CHACHA20_CRYPT,                          /**< SERVICE_CRYPTOCELL_MBEDTLS_CHACHA20_CRYPT */
  SERVICE_CRYPTOCELL_MBEDTLS_CHACHAPOLY_CRYPT,                        /**< SERVICE_CRYPTOCELL_MBEDTLS_CHACHAPOLY_CRYPT */
  SERVICE_CRYPTOCELL_MBEDTLS_POLY1305_CRYPT,                          /**< SERVICE_CRYPTOCELL_MBEDTLS_POLY1305_CRYPT */
  SERVICE_CRYPTOCELL_MBEDTLS_SHA_STARTS,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_STARTS */
  SERVICE_CRYPTOCELL_MBEDTLS_SHA_PROCESS,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_PROCESS */
  SERVICE_CRYPTOCELL_MBEDTLS_SHA_UPDATE,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_UPDATE */
  SERVICE_CRYPTOCELL_MBEDTLS_SHA_FINISH,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_SHA_FINISH */
  SERVICE_CRYPTOCELL_MBEDTLS_TRNG_HARDWARE_POLL,                      /**< SERVICE_CRYPTOCELL_MBEDTLS_TRNG_HARDWARE_POLL */
  SERVICE_CRYPTOCELL_MBEDTLS_CMAC_INIT_SETKEY,                        /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_INIT_SETKEY */
  SERVICE_CRYPTOCELL_MBEDTLS_CMAC_UPDATE,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_UPDATE */
  SERVICE_CRYPTOCELL_MBEDTLS_CMAC_FINISH,                             /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_FINISH */
  SERVICE_CRYPTOCELL_MBEDTLS_CMAC_RESET,                              /**< SERVICE_CRYPTOCELL_MBEDTLS_CMAC_RESET */
  SERVICE_CRYPTOCELL_END     = 499,                                   /**< SERVICE_CRYPTOCELL_END */

  /**
   * Boot Services
   */
  SERVICE_BOOT_START         = 500,                                   /**< SERVICE_BOOT_START */
  SERVICE_BOOT_PROCESS_TOC_ENTRY = SERVICE_BOOT_START,                /**< SERVICE_BOOT_PROCESS_TOC_ENTRY */
  SERVICE_BOOT_CPU,                                                   /**< SERVICE_BOOT_CPU */
  SERVICE_BOOT_RELEASE_CPU,                                           /**< SERVICE_BOOT_RESEASE_CPU */
  SERVICE_BOOT_RESET_CPU,                                             /**< SERVICE_BOOT_RESET_CPU */
  SERVICE_BOOT_RESET_SOC,                                             /**< SERVICE_BOOT_RESET_SOC */
  SERVICE_BOOT_END           = 599,                                   /**< SERVICE_BOOT_END */

  /**
   * Update Services
   */
  SERVICE_UPDATE_START       = 600,                                   /**< SERVICE_UPDATE_START */
  SERVICE_UPDATE_END         = 699                                    /**< SERVICE_UPDATE_END */
} SERVICE_ID_t;

/**
 * Transport layer header structure, common for all services
 */

 /**
 * @struct service_header_t
 */
typedef struct
{
  volatile uint16_t send_service_id; // Requested Service ID
  volatile uint16_t send_flags;      // Request flags
  volatile uint16_t resp_error_code; // Transport layer error code
  volatile uint16_t none_padding;
} service_header_t;

/**
 * @struct Power profiles
 */
typedef enum
{
  LOWEST_POWER_PROFILE = 0,      /**< LOWEST_POWER_PROFILE */
  HIGH_PERFORMANCE_POWER_PROFILE,/**< HIGH_PERFORMANCE_POWER_PROFILE */
  USER_SPECIFIED_PROFILE,        /**< USER_SPECIFIED_PROFILE */
  DEFAULT_POWER_PROFILE,         /**< DEFAULT_POWER_PROFILE */
  NUMBER_OF_POWER_PROFILES       /**< NUMBER_OF_POWER_PROFILES */
} services_power_profile_t;


typedef enum
{
  EWIC_RTC_SE               = 0x1,          // bit0
  EWIC_MODEM                = 0x2,          // bit1
  EWIC_MODEM_TO_SE_IRQ      = 0xC,          // bit3:2
  EWIC_MODEM_PPU_IRQ        = 0x10,         // bit4
  EWIC_MODEM_WARM_RESET_REQ = 0x20,         // bit5
  EWIC_RTC_A                = 0x40,         // bit6
  EWIC_VBAT_TIMER           = 0x780,        // bit10:7
  EWIC_VBAT_GPIO            = 0x7F800,      // bit18:11
  EWIC_VBAT_LP_CMP_IRQ      = 0x00080000UL, // bit19
  EWIC_ES1_LP_I2C_IRQ       = 0x00100000UL, // bit20
  EWIC_ES1_LP_UART_IRQ      = 0x00200000UL, // bit21
  EWIC_BROWN_OUT            = 0x00400000UL, // bit22
} SERVICES_ewic_cfg_t;

typedef enum
{
  VBAT_WAKEUP_MDM           = 0x1,          // bit0
  VBAT_WAKEUP_RTC_SE        = 0x10,         // bit4
  VBAT_WAKEUP_RTC_A         = 0x20,         // bit5
  VBAT_WAKEUP_LPCMP         = 0x40,         // bit6
  VBAT_WAKEUP_BROWN_OUT     = 0x80,         // bit7
  VBAT_WAKEUP_LPTIMER       = 0XF00,        // bit11:8
  VBAT_WAKEUP_LPGPIO        = 0XFF0000,     // bit23:16
} SERVICES_wakeup_cfg_t;

// Memory retention bit encoding for mem_retention_enable
#define POWER_MEM_RETENTION_BACKUP_RAM          0x01
#define POWER_MEM_RETENTION_MODEM_BACKUP_RAM    0x02
#define POWER_MEM_RETENTION_FIREWALL_RAM        0x04
#define POWER_MEM_RETENTION_ES1_TCM             0x08
#define POWER_MEM_RETENTION_MODEM_TCM           0x10
#define POWER_MEM_RETENTION_SE_RAM              0x20


/*******************************************************************************
 *  Service format definitions
 ******************************************************************************/

// Crypto Services

// Get Random Data

// Same as MBEDTLS_CTR_DRBG_MAX_REQUEST in cryptocell-rt
#define MAX_RND_LENGTH 1024
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_rnd_length;
  volatile uint8_t   resp_rnd[MAX_RND_LENGTH];
  volatile int       resp_error_code;
} get_rnd_svc_t;

// Get LCS
typedef struct
{
  service_header_t header;
  volatile uint32_t  resp_lcs;
  volatile uint32_t  resp_error_code;
} get_lcs_svc_t;

// MBEDTLS TRNG HARDWARE POLL
typedef struct
{
  service_header_t header;
  volatile uint32_t send_data_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t send_len;
  volatile uint32_t send_olen_addr;
  volatile uint32_t resp_error_code;
} mbedtls_trng_hardware_poll_svc_t;

// MBEDTLS AES INIT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
} mbedtls_aes_init_svc_t;

// MBEDTLS AES SET KEY
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_key_addr;
  volatile uint32_t send_key_bits;
  volatile uint32_t send_direction;
  volatile uint32_t resp_error_code;
} mbedtls_aes_set_key_svc_t;

// MBEDTLS AES CRYPT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_crypt_type; // ECB, CBC, CTR_OFB
  volatile uint32_t send_mode;       // Encrypt, Decrypt
  volatile uint32_t send_length;
  volatile uint32_t send_iv_addr;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t resp_error_code;
} mbedtls_aes_crypt_svc_t;

// MBEDTLS SHA
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_sha_type;  // SHA1, SHA224, SHA256
  volatile uint32_t send_data_addr;
  volatile uint32_t send_data_length;
  volatile uint32_t resp_error_code;
} mbedtls_sha_svc_t;

// MBEDTLS CCM/GCM SET KEY
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_key_type;
  volatile uint32_t send_cipher;
  volatile uint32_t send_key_addr;
  volatile uint32_t send_key_bits;
  volatile uint32_t resp_error_code;
} mbedtls_ccm_gcm_set_key_svc_t;

// MBEDTLS CCM/GCM CRYPT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_crypt_type;
  volatile uint32_t send_length;
  volatile uint32_t send_iv_addr;
  volatile uint32_t send_iv_length;
  volatile uint32_t send_add_addr;
  volatile uint32_t send_add_length;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t send_tag_addr;
  volatile uint32_t send_tag_length;
  volatile uint32_t resp_error_code;
} mbedtls_ccm_gcm_crypt_svc_t;

// MBEDTLS CHACHA20 CRYPT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_key_addr;
  volatile uint32_t send_nonce_addr;
  volatile uint32_t send_counter;
  volatile uint32_t send_data_len;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t resp_error_code;
} mbedtls_chacha20_crypt_svc_t;

// MBEDTLS CHACHAPOLY CRYPT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_crypt_type;
  volatile uint32_t send_length;
  volatile uint32_t send_nonce_addr;
  volatile uint32_t send_aad_addr;
  volatile uint32_t send_aad_len;
  volatile uint32_t send_tag_addr;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t resp_error_code;
} mbedtls_chachapoly_crypt_svc_t;

// MBEDTLS POLY1305 CRYPT
typedef struct
{
  service_header_t header;
  volatile uint32_t send_key_addr;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_ilen;
  volatile uint32_t send_mac_addr;
  volatile uint32_t resp_error_code;
} mbedtls_poly1305_crypt_svc_t;

// MBEDTLS CMAC INIT/SETKEY
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_key_addr;
  volatile uint32_t send_key_bits;
  volatile uint32_t resp_error_code;
} mbedtls_cmac_init_setkey_svc_t;

// MBEDTLS CMAC UPDATE
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_input_addr;
  volatile uint32_t send_input_length;
  volatile uint32_t resp_error_code;
} mbedtls_cmac_update_svc_t;

// MBEDTLS CMAC FINISH
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t send_output_addr;
  volatile uint32_t resp_error_code;
} mbedtls_cmac_finish_svc_t;

// MBEDTLS CMAC RESET
typedef struct
{
  service_header_t header;
  volatile uint32_t send_context_addr;
  volatile uint32_t resp_error_code;
} mbedtls_cmac_reset_svc_t;


// Boot Services

// Process TOC Entry
typedef struct
{
  service_header_t header;
  volatile uint8_t   send_entry_id[IMAGE_NAME_LENGTH];
  volatile uint32_t  resp_error_code;
} process_toc_entry_svc_t;

// Boot CPU
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_cpu_id;
  volatile uint32_t  send_address;
  volatile uint32_t  resp_error_code;
} boot_cpu_svc_t;

// Control CPU
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_cpu_id;
  volatile uint32_t  resp_error_code;
} control_cpu_svc_t;

/**
 * Application Services
 */

// Pimux
typedef struct
{
  service_header_t header;
  volatile uint8_t  send_port_num;
  volatile uint8_t  send_pin_num;
  volatile uint8_t  send_config_data;
  volatile uint8_t  resp_error_code;
} pinmux_svc_t;

// Pad Control
typedef struct
{
  service_header_t header;
  volatile uint8_t  send_port_num;
  volatile uint8_t  send_pin_num;
  volatile uint8_t  send_config_data;
  volatile uint8_t  resp_error_code;
} pad_control_svc_t;

/* UART Write */
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_string_length;
  volatile uint8_t   send_string_contents[256];
  volatile uint32_t  resp_error_code;
} uart_write_svc_t;

// OSPI write key
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_command;
  volatile uint8_t   send_key[16];
  volatile uint32_t  resp_error_code;
} ospi_write_key_svc_t;

/**
 * @struct get_se_revision_t
 * Retrieve SERAM version banner
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t  resp_se_revision_length;
  volatile uint8_t   resp_se_revision[VERSION_RESPONSE_LENGTH];
  volatile uint32_t  resp_error_code;
} get_se_revision_t;

/**
 * System Management Services
 */

/**
 * @struct get_toc_version_svc_t
 * Get TOC Version
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t  resp_version;
  volatile uint32_t  resp_error_code;
} get_toc_version_svc_t;

/**
 * @struct get_toc_number_svc_t
 * Get TOC Number
 */
typedef struct
{
  service_header_t header;
  volatile  uint32_t  resp_number_of_toc;
  volatile uint32_t  resp_error_code;
} get_toc_number_svc_t;

/**
 * @struct get_toc_via_name_svc_t
 * Get TOC via name
 */
typedef struct
{
  service_header_t header;
  volatile uint8_t   send_cpu_name[IMAGE_NAME_LENGTH];
  volatile uint32_t  resp_error_code;
} get_toc_via_name_svc_t;

/**
 * @struct get_toc_via_cpu_id_svc_t
 * Get TOC via CPU ID
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t  send_cpu_id;
  volatile uint8_t   resp_image_identifier[IMAGE_NAME_LENGTH];
  volatile uint32_t  resp_image_version;
  volatile uint32_t  resp_image_flags;
  volatile uint32_t  resp_error_code;
} get_toc_via_cpu_id_svc_t;

/**
 * @struct get_toc_entry_t
 * @brief  one toc record
 */
typedef struct
{
  volatile  uint8_t  image_identifier[IMAGE_NAME_LENGTH];
  volatile uint32_t  version;
  volatile uint32_t  cpu;
  volatile uint32_t  store_address;
  volatile uint32_t  load_address;
  volatile uint32_t  boot_address;
  volatile uint32_t  image_size;
  volatile uint32_t  flags;
} get_toc_entry_t;

/**
 * @struct get_toc_data_t
 * @brief  Get all TOC data. 15 is the Max number of SES records in SERAM
 */
typedef struct
{
  service_header_t header;
  get_toc_entry_t resp_toc_entry[15];
  volatile uint32_t resp_number_of_toc_entries;
  volatile uint32_t resp_error_code;
} get_toc_data_t;

/**
 * @struct get_otp_data_t
 * @brief  bucket to hold otp data
 * @todo   to deprecate
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t  otp_alif_manufacturing_data[4];
  volatile uint32_t  otp_alif_manufacturing_serial_number[2];
  volatile uint8_t   otp_alif_manufacturing_part_number[16];
  volatile uint32_t  otp_alif_hbk_0[3];
  volatile uint32_t  otp_alif_hbk_1[3];
  volatile uint32_t  otp_alif_firmware_version_dcu[10];
  volatile uint32_t resp_error_code;
} get_otp_data_t;

/**
 * @struct  read_otp_data_t
 * @brief   request for otp read
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t send_offset;       /**< OTP offset to read */
  volatile uint32_t resp_otp_word;     /**< OTP contents       */
  volatile uint32_t resp_error_code;
} read_otp_data_t;

/**
 * @struct get_device_part_svc_t
 * Get Device Part
 */
typedef struct
{
  service_header_t header;
  volatile uint32_t  resp_device_string;
  volatile uint32_t  resp_error_code;
} get_device_part_svc_t;

/**
 * @struct set_services_capabilities_t
 * Set capabilities
 * @note for now it is limited to debug toggle
 */
typedef struct
{
  service_header_t header;
  volatile bool      send_services_debug;
  volatile uint32_t  resp_error_code;
} set_services_capabilities_t;

//////////////////////////////////////////////////////////////////
// Power Services
//

//----------------------------------------------------------------
// Stop Mode Request
//  enable - any bit enables
//           bit 1 - Set M55_HE VTOR to sentinel pattern
//           bit 2 - Enable SE TCM retention
//           bit 3 - force SE  WDOG expiration
//           bit 4 - force SOC WDOG expiration
//
typedef struct
{
  service_header_t header;
  services_power_profile_t power_profile;
  bool override;
} stop_mode_request_svc_t;

//----------------------------------------------------------------
// EWIC configuration Request
//
typedef struct
{
  service_header_t header;
  uint32_t send_ewic_source;
  services_power_profile_t power_profile;
  uint32_t resp_error_code;
} ewic_config_request_svc_t;

//----------------------------------------------------------------
// VBAT wakeup configuration Request
//
typedef struct
{
  service_header_t header;
  uint32_t send_vbat_wakeup_source;
  services_power_profile_t power_profile;
  uint32_t resp_error_code;
} vbat_wakeup_config_request_svc_t;

//----------------------------------------------------------------
// SE memory retention configuration Request
//
typedef struct
{
  service_header_t header;
  uint32_t send_mem_retention;
  services_power_profile_t power_profile;
  uint32_t resp_error_code;
} mem_ret_config_request_svc_t;


/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

uint32_t SERVICES_local_to_global_addr(uint32_t local_addr);
uint32_t SERVICES_global_to_local_addr(uint32_t global_addr);

#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_PROTOCOL_H__ */
