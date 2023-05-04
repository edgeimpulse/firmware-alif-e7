/**
 * @file services_lib_api.h
 *
 * @brief Services library public API header file
 * @defgroup host_services Host Services
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
#ifndef __SERVICES_LIB_API_H__
#define __SERVICES_LIB_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdbool.h>
#include "services_lib_protocol.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/**
 * Error codes
 */
#define SERVICES_REQ_SUCCESS                       0x00
#define SERVICES_REQ_NOT_ACKNOWLEDGE               0xFF
#define SERVICES_REQ_ACKNOWLEDGE                   0xFE
#define SERVICES_REQ_TIMEOUT                       0xFD
#define SERVICES_RESP_UNKNOWN_COMMAND              0xFC
#define SERVICES_REQ_BAD_PACKET_SIZE               0xFB
#define SERVICES_REQ_CANNOT_EXECUTE_SERVICE        0xFA
#define SERVICES_REQ_BAD_PAYLOAD                   0xF9
#define SERVICES_REQ_BAD_PAYLOAD_LENGTH            0xF8
#define SERVICES_REQ_PAYLOAD_OK                    0xF7
#define SERVICES_REQ_BAD_PRINT_LENGTH              0xF6
#define SERVICES_REQ_NULL_PARAMETER                0xF5
#define SERVICES_REQ_PIN_LOCKED                    0xF4
#define SERVICES_REQ_COMMAND_NOT_IMPLEMENTED       0xF3
#define SERVICES_REQ_INVALID_OTP_OFFSET            0xF2

#define SERVICES_MAX_PACKET_BUFFER_SIZE            2048

/**
 * OTP Offsets
 */
#define OTP_MANUFACTURE_INFO_DATA_START           0x51
#define OTP_MANUFACTURE_INFO_DATA_END             0x58
#define OTP_MANUFACTURE_INFO_SERIAL_NUMBER_START  0x59
#define OTP_MANUFACTURE_INFO_SERIAL_NUMBER_END    0x5A

#define OTP_MANUFACTURE_INFO_OPTIONAL_LENGTH_BYTES 32
#define OTP_MANUFACTURE_INFO_SERIAL_NUMBER_LENGTH_BYTES  8

#define OTP_OSPI_KEY_OSPI0                        0x60
#define OTP_OSPI_KEY_OSPI1                        0x64
#define OSPI_KEY_LENGTH_BYTES                     16

/**
 * MBED TLS
 */
#define MBEDTLS_OP_DECRYPT                         0
#define MBEDTLS_OP_ENCRYPT                         1

#define MBEDTLS_AES_CRYPT_ECB                      0
#define MBEDTLS_AES_CRYPT_CBC                      1
#define MBEDTLS_AES_CRYPT_CTR                      2
#define MBEDTLS_AES_CRYPT_OFB                      3

#define MBEDTLS_HASH_SHA1                          0
#define MBEDTLS_HASH_SHA224                        1
#define MBEDTLS_HASH_SHA256                        2

#define MBEDTLS_CCM_KEY                            0
#define MBEDTLS_GCM_KEY                            1

#define MBEDTLS_CCM_ENCRYPT_AND_TAG                0
#define MBEDTLS_CCM_AUTH_DECRYPT                   1
#define MBEDTLS_CCM_STAR_ENCRYPT_AND_TAG           2
#define MBEDTLS_CCM_STAR_AUTH_DECRYPT              3
#define MBEDTLS_GCM_ENCRYPT_AND_TAG                4
#define MBEDTLS_GCM_DECRYPT_AND_TAG                5
#define MBEDTLS_GCM_AUTH_DECRYPT                   6

#define MBEDTLS_CHACHAPOLY_ENCRYPT_AND_TAG         0
#define MBEDTLS_CHACHAPOLY_AUTH_DECRYPT            1

#define SERVICES_NUMBER_OF_TOC_ENTRIES             15

/**
 * OSPI write key commands
 */
#define OSPI_WRITE_OTP_KEY_OSPI0                   0
#define OSPI_WRITE_OTP_KEY_OSPI1                   1
#define OSPI_WRITE_EXTERNAL_KEY_OSPI0              2
#define OSPI_WRITE_EXTERNAL_KEY_OSPI1              3

/**
 * TOC related
 */
#define TOC_NAME_LENGTH                             8

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

typedef int32_t (*wait_ms_t)(uint32_t wait_time_ms);
typedef int (*print_msg_t)(const char * fmt, ...);

/**
 *  @enum SERVICES_cpuid_t
 */
typedef enum
{
  FUSION_A32_0   = 0,                /**< A32_0 CPU         */
  FUSION_A32_1   = 1,                /**< A32_1 CPU         */
  FUSION_M55_HP  = 2,                /**< M55 HP CPU        */
  FUSION_M55_HE  = 3,                /**< M55 HE CPU        */
  FUSION_EXTERNAL_SYS0 = 4           /**< CPU in Ext SYS0   */
} SERVICES_cpuid_t;

/**
 *  @struct SERVICES_toc_info_t
 */
typedef struct
{
  uint8_t   image_identifier[TOC_NAME_LENGTH]; /**< TOC name         */
  uint32_t  version;                  /**< TOC Version      */
  uint32_t  cpu;                      /**< TOC Cpu ID       */
  uint32_t  store_address;            /**< TOC MRAM address */
  uint32_t  load_address;             /**< TOC load         */
  uint32_t  boot_address;             /**< TOC boot address */
  uint32_t  image_size;               /**< TOC image size   */
  uint32_t  flags;                    /**< TOC flag state   */
} SERVICES_toc_info_t;

/**
 * @struct SERVICES_otp_data_t
 * @todo   not all OTP fields are exposed, TBD
 *         ALIF serial number and manufacturing data is TBD
 */
typedef struct
{
  uint32_t  otp_alif_manufacturing_data[4];         /**< OTP ALIF Fab, Lot number.. */
  uint32_t  otp_alif_manufacturing_serial_number[2];/**< OTP ALIF Serial number     */
  uint8_t   otp_alif_manufacturing_part_number[16]; /**< OTP part number            */
  uint32_t  otp_alif_hbk_0[3];                      /**< OTP HBK0                   */
  uint32_t  otp_alif_hbk_1[3];                      /**< OTP HBK1                   */
  uint32_t  otp_alif_firmware_version_dcu[10];      /**< OTP FW, flags, DCU lockmask*/
} SERVICES_otp_data_t;

/**
 * @struct SERVICES_toc_data_t
 */
typedef struct
{
  uint32_t number_of_toc_entries;     /**< Number of real TOC objects */
  SERVICES_toc_info_t toc_entry[SERVICES_NUMBER_OF_TOC_ENTRIES]; /* TOC details */
} SERVICES_toc_data_t;

/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

// Services infrastructure APIs
uint32_t SERVICES_register_channel(uint32_t mhu_id,
                                   uint32_t channel_number);
void SERVICES_unregister_channel(uint32_t mhu_id,
                                 uint32_t channel_number);

uintptr_t SERVICES_prepare_packet_buffer(uint32_t size);

typedef void (*SERVICES_sender_callback) (uint32_t sender_id, uint32_t data);
uint32_t SERVICES_send_request(uint32_t services_handle,
                               uint16_t service_id,
                               SERVICES_sender_callback callback);

const char *SERVICES_version(void);
char *SERVICES_error_to_string(uint32_t error_code);


// Services functional APIs
uint32_t SERVICES_heartbeat(uint32_t services_handle);
uint32_t SERVICES_heartbeat_async(uint32_t services_handle, 
                                  SERVICES_sender_callback callback);
uint32_t SERVICES_uart_write(uint32_t services_handle, size_t size,
                             const uint8_t *uart_data);
uint32_t SERVICES_pinmux(uint32_t services_handle, 
                         uint8_t port_number, 
                         uint8_t pin_number, 
                         uint8_t config_data, 
                         uint32_t * error_code);
uint32_t SERVICES_padcontrol(uint32_t services_handle, 
                             uint8_t port_number, 
                             uint8_t pin_number,
                             uint8_t configuration_value, 
                             uint32_t * error_code);
uint32_t SERVICES_application_ospi_write_key(uint32_t services_handle,
                                             uint32_t command,
                                             uint8_t * key,
                                             uint32_t * error_code);

uint32_t SERVICES_cryptocell_get_rnd(uint32_t services_handle, 
                                     uint16_t rnd_len,
                                     void *rnd_value,
                                     int32_t * error_code);

uint32_t SERVICES_cryptocell_get_lcs(uint32_t services_handle, 
                                     uint32_t *lcs_state,
                                     int32_t * error_code);

// MbedTLS macros and APIs
uint32_t SERVICES_cryptocell_mbedtls_hardware_poll(uint32_t services_handle,
                                                   uint32_t * error_code,
                                                   uint32_t data,
                                                   uint32_t output,
                                                   uint32_t len,
                                                   uint32_t olen);

uint32_t SERVICES_cryptocell_mbedtls_aes_init(uint32_t services_handle, 
                                              uint32_t * error_code, 
                                              uint32_t ctx);
uint32_t SERVICES_cryptocell_mbedtls_aes_set_key(uint32_t services_handle, 
                                                 uint32_t * error_code, 
                                                 uint32_t ctx, 
                                                 uint32_t key, 
                                                 uint32_t keybits,
                                                 uint32_t dir);
uint32_t SERVICES_cryptocell_mbedtls_aes_crypt(uint32_t services_handle, 
                                               uint32_t * error_code, 
                                               uint32_t ctx, 
                                               uint32_t crypt_type, 
                                               uint32_t mode, 
                                               uint32_t length, 
                                               uint32_t iv, 
                                               uint32_t input, 
                                               uint32_t output);

uint32_t SERVICES_cryptocell_mbedtls_sha_starts(uint32_t services_handle, 
                                                uint32_t * error_code, 
                                                uint32_t ctx, 
                                                uint32_t sha_type);
uint32_t SERVICES_cryptocell_mbedtls_sha_process(uint32_t services_handle, 
                                                uint32_t * error_code, 
                                                uint32_t ctx, 
                                                uint32_t sha_type, 
                                                uint32_t data);
uint32_t SERVICES_cryptocell_mbedtls_sha_update(uint32_t services_handle, 
                                                uint32_t * error_code, 
                                                uint32_t ctx, 
                                                uint32_t sha_type, 
                                                uint32_t data, 
                                                uint32_t data_length);
uint32_t SERVICES_cryptocell_mbedtls_sha_finish(uint32_t services_handle, 
                                                uint32_t * error_code, 
                                                uint32_t ctx, 
                                                uint32_t sha_type, 
                                                uint32_t data);

uint32_t SERVICES_cryptocell_mbedtls_ccm_gcm_set_key(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t key_type,
    uint32_t cipher,
    uint32_t key_addr,
    uint32_t key_bits);
uint32_t SERVICES_cryptocell_mbedtls_ccm_gcm_crypt(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t crypt_type,
    uint32_t length,
    uint32_t iv_addr,
    uint32_t iv_length,
    uint32_t add_addr,
    uint32_t add_length,
    uint32_t input_addr,
    uint32_t output_addr,
    uint32_t tag_addr,
    uint32_t tag_length);
uint32_t SERVICES_cryptocell_mbedtls_chacha20_crypt(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t key_addr,
    uint32_t nonce_addr,
    uint32_t counter,
    uint32_t data_len,
    uint32_t input_addr,
    uint32_t output_addr);
uint32_t SERVICES_cryptocell_mbedtls_chachapoly_crypt(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t crypt_type,
    uint32_t length,
    uint32_t nonce_addr,
    uint32_t aad_addr,
    uint32_t aad_len,
    uint32_t tag_addr,
    uint32_t input_addr,
    uint32_t output_addr);
uint32_t SERVICES_cryptocell_mbedtls_poly1305_crypt(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t key_addr,
    uint32_t input_addr,
    uint32_t ilen,
    uint32_t mac_addr);
uint32_t SERVICES_cryptocell_mbedtls_cmac_init_setkey(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t key_addr,
    uint32_t key_bits);
uint32_t SERVICES_cryptocell_mbedtls_cmac_update(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t input_addr,
    uint32_t input_length);
uint32_t SERVICES_cryptocell_mbedtls_cmac_finish(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr,
    uint32_t output_addr);
uint32_t SERVICES_cryptocell_mbedtls_cmac_reset(uint32_t services_handle,
    uint32_t * error_code,
    uint32_t context_addr);

uint32_t SERVICES_system_get_toc_version(uint32_t services_handle, 
                                         uint32_t * toc_version,
                                         uint32_t * error_code);
uint32_t SERVICES_system_get_toc_number(uint32_t services_handle, 
                                        uint32_t *toc_number,
                                        uint32_t * error_code);
uint32_t SERVICES_system_get_toc_via_name(uint32_t services_handle, 
                                          const uint8_t *cpu_name,
                                          uint32_t * error_code);
uint32_t SERVICES_system_get_toc_via_cpuid(uint32_t services_handle, 
                                           SERVICES_cpuid_t cpuid,
                                           SERVICES_toc_info_t *toc_info,
                                           uint32_t * error_code);
uint32_t SERVICES_system_get_toc_data     (uint32_t services_handle,
                                           SERVICES_toc_data_t *toc_data,
                                           uint32_t * error_code);
uint32_t SERVICES_system_get_device_part_number(uint32_t services_handle, 
                                                uint32_t *device_part_number,
                                                uint32_t * error_code);
uint32_t SERVICES_system_set_services_debug (uint32_t services_handle,
                                             bool debug_enable,
                                             uint32_t *error_code);
uint32_t SERVICES_get_se_revision(uint32_t services_handle,
                                  uint8_t *revision_data, uint32_t *error_code);
uint32_t SERVICES_system_get_otp_data (uint32_t services_handle,
                                       SERVICES_otp_data_t *toc_info,
                                       uint32_t * error_code);
uint32_t SERVICES_system_read_otp     (uint32_t services_handle,
                                       uint32_t otp_offset,
                                       uint32_t *otp_value_word,
                                       uint32_t *error_code);

uint32_t SERVICES_boot_process_toc_entry(uint32_t services_handle, 
                                         const uint8_t * image_id,
                                         uint32_t * error_code);
uint32_t SERVICES_boot_cpu(uint32_t services_handle, 
                           uint32_t cpu_id,
                           uint32_t address,
                           uint32_t * error_code);
uint32_t SERVICES_boot_release_cpu(uint32_t services_handle,
                                   uint32_t cpu_id,
                                   uint32_t * error_code);
uint32_t SERVICES_boot_reset_cpu(uint32_t services_handle,
                                 uint32_t cpu_id,
                                 uint32_t * error_code);
uint32_t SERVICES_boot_reset_soc(uint32_t services_handle);

uint32_t SERVICES_power_stop_mode_req(uint32_t services_handle,
                                      services_power_profile_t power_profile,
                                      bool override);
uint32_t SERVICES_power_ewic_config(uint32_t services_handle,
                                    uint32_t ewic_source,
                                    services_power_profile_t power_profile);
uint32_t SERVICES_power_wakeup_config(uint32_t services_handle,
                                      uint32_t vbat_wakeup_source,
                                      services_power_profile_t power_profile);
uint32_t
SERVICES_power_mem_retention_config(uint32_t services_handle,
                                    uint32_t mem_retention,
                                    services_power_profile_t power_profile);


#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_API_H__ */
