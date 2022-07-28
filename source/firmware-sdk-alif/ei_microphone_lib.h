/* Edge Impulse inferencing library
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __EI_MICROPHONE_LIB__H__
#define __EI_MICROPHONE_LIB__H__

#include "edge-impulse-sdk/dsp/memory.hpp"
#include "firmware-sdk-alif/ei_device_info_lib.h"
#include "firmware-sdk-alif/ei_device_memory.h"
#include "firmware-sdk-alif/sensor_aq.h"
#include "sensor_aq_mbedtls_hs256.h"

constexpr int CTX_BUF_SIZE = 1024;

// Interface for mic
class EiMicrophone {
public:
    /**
     * @brief Called before calling await_samples in loop. Optional
     *
     * @param buffer Location to write the samples 
     * @param size Number of samples (not bytes!) to write
     * @return 0 on success
     */
    virtual int async_start(microphone_sample_t *buffer, uint32_t size) = 0;

    /**
     * @brief Override if your driver has a limit on buffer size
     * 
     * @return uint32_t Desired buffer size, 0 for don't care
     */
    virtual uint32_t requested_buffer_size()
    {
        return 0;
    }

    /**
     * @brief Synchronous call to wait for samples
     * 
     * @return uint32_t Number of samples written
     */
    virtual uint32_t await_samples() = 0;
};

/* Dummy functions for sensor_aq_ctx type */
static size_t ei_write(const void *, size_t, size_t count, EI_SENSOR_AQ_STREAM *)
{
    return count;
}

static int ei_seek(EI_SENSOR_AQ_STREAM *, long int, int)
{
    return 0;
}

static int insert_ref(char *buffer, int hdrLength)
{
#define EXTRA_BYTES(a) ((a & 0x3) ? 4 - (a & 0x3) : (a & 0x03))
    const char *ref = "Ref-BINARY-i16";
    int addLength = 0;
    int padding = EXTRA_BYTES(hdrLength);

    buffer[addLength++] = 0x60 + 14 + padding;
    for (size_t i = 0; i < strlen(ref); i++) {
        buffer[addLength++] = *(ref + i);
    }
    for (int i = 0; i < padding; i++) {
        buffer[addLength++] = ' ';
    }

    buffer[addLength++] = 0xFF;

    return addLength;
}

static uint32_t create_header(sensor_aq_payload_info *payload, EiDeviceMemory *mem, sensor_aq_signing_ctx_t *ei_mic_signing_ctx, sensor_aq_mbedtls_hs256_ctx_t *ei_mic_hs_ctx, sensor_aq_ctx *ei_mic_ctx)
{
    auto dev = EiDeviceInfo::get_device();

    sensor_aq_init_mbedtls_hs256_context(
        ei_mic_signing_ctx,
        ei_mic_hs_ctx,
        dev->get_sample_hmac_key().c_str());

    auto ret = sensor_aq_init(ei_mic_ctx, payload, NULL, true);

    if (ret != AQ_OK) {
        ei_printf("sensor_aq_init failed (%d)\n", ret);
        return 0;
    }

    // then we're gonna find the last byte that is not 0x00 in the CBOR buffer.
    // That should give us the whole header
    int end_of_header_ix = 0;
    for (int ix = ei_mic_ctx->cbor_buffer.len - 1; ix >= 0; ix--) {
        if (((uint8_t *)ei_mic_ctx->cbor_buffer.ptr)[ix] != 0x0) {
            end_of_header_ix = ix;
            break;
        }
    }

    if (end_of_header_ix == 0) {
        ei_printf("Failed to find end of header\n");
        return 0;
    }

    int ref_size =
        insert_ref(((char *)ei_mic_ctx->cbor_buffer.ptr + end_of_header_ix), end_of_header_ix);
    // and update the signature
    ret = ei_mic_ctx->signature_ctx->update(
        ei_mic_ctx->signature_ctx,
        ((uint8_t *)(ei_mic_ctx->cbor_buffer.ptr) + end_of_header_ix),
        ref_size);
    if (ret != 0) {
        ei_printf("Failed to update signature from header (%d)\n", ret);
        return 0;
    }
    end_of_header_ix += ref_size;

    // Write to blockdevice
    ret = mem->write_data((uint8_t *)ei_mic_ctx->cbor_buffer.ptr, 0, end_of_header_ix);

    if (ret != end_of_header_ix) {
        ei_printf("Failed to write to header blockdevice (%d)\n", ret);
        return 0;
    }

    return end_of_header_ix;
}

// ********** Public / interface functions ************

/**
 * @brief Get the number of samples needed, based on config
 * 
 * @return uint32_t Number of samples (not bytes!)
 */
uint32_t ei_microphone_get_samples_required()
{
    auto dev = EiDeviceInfo::get_device();
    return (uint32_t)((dev->get_sample_length_ms()) / dev->get_sample_interval_ms());
}

/**
 * @brief Get the buffer sized needed for entire transfer (header and samples)
 * Useful when using EiDeviceRAM
 * 
 * @return uint32_t number of B
 */
uint32_t ei_microphone_get_buffer_size()
{
    return ei_microphone_get_samples_required() * sizeof(microphone_sample_t) + CTX_BUF_SIZE;
}

/**
 * @brief 
 * 
 * 
 * Callbacks are std::function.  
 * Use lambdas or wrapper funcs to bind whatever input params you need
 * @param init , return 0 on success
 * @return true Success
 * @return false 
 */
bool ei_microphone_sample_record_lib(EiMicrophone *mic, EiDeviceMemory *mem)
{
    sensor_aq_signing_ctx_t ei_mic_signing_ctx;
    sensor_aq_mbedtls_hs256_ctx_t ei_mic_hs_ctx;

    unsigned char* ei_mic_ctx_buffer;
    EI_ALLOCATE_AUTO_POINTER(ei_mic_ctx_buffer, CTX_BUF_SIZE)
    EI_ERR_AND_RETURN_ON_NULL(ei_mic_ctx_buffer, false)

    sensor_aq_ctx ei_mic_ctx = {
        { ei_mic_ctx_buffer, CTX_BUF_SIZE }, &ei_mic_signing_ctx, &ei_write, &ei_seek, NULL,
    };

    auto dev = EiDeviceInfo::get_device();
    uint8_t *page_buffer;
    sensor_aq_payload_info payload = { dev->get_device_id().c_str(),
                                       dev->get_device_type().c_str(),
                                       dev->get_sample_interval_ms(),
                                       { { "audio", "wav" } } };

    ei_printf("Sampling settings:\n");
    ei_printf("\tInterval: %.5f ms.\n", dev->get_sample_interval_ms());
    ei_printf("\tLength: %lu ms.\n", dev->get_sample_length_ms());
    ei_printf("\tName: %s\n", dev->get_sample_label().c_str());
    ei_printf("\tHMAC Key: %s\n", dev->get_sample_hmac_key().c_str());
    ei_printf("\tFile name: %s\n", dev->get_sample_label().c_str());

    /* TODO: use time calculations from target and signal exact time to studio */
    ei_printf(
        "Starting in %lu ms... (or until all flash was erased)\n",
        2000);

    // Minimum delay of 2000 ms for daemon
    uint32_t start_time = ei_read_timer_ms();

    auto sample_buffer_size = ei_microphone_get_buffer_size();
    if (mem->erase_data(0, sample_buffer_size) != (sample_buffer_size)) {
        return false;
    }

    // if erasing took less than 2 seconds, wait additional time
    auto elapsed_time = ei_read_timer_ms() - start_time;
    if (elapsed_time < 2000) {
        ei_sleep(2000 - elapsed_time);
    }

    uint32_t buffer_offset = create_header(&payload, mem, &ei_mic_signing_ctx, &ei_mic_hs_ctx, &ei_mic_ctx);
    if (buffer_offset == 0) {
        return false;
    }

    auto requested_buffer_size = mic->requested_buffer_size();

    ei_printf("Sampling...\n");

    auto samples_required = ei_microphone_get_samples_required();
    for (auto samples_left = samples_required; samples_left > 0;) //decrement inside loop
    {
        uint32_t samples_written = 0;
        microphone_sample_t *sample_buffer = NULL;
        bool free_me = false;
        if (mem->is_ram()) {
            // If this is RAM, and the driver doesn't care, just dump everything at once
            auto buf_size = requested_buffer_size ? requested_buffer_size : samples_left;
            EiDeviceRAM *ram = reinterpret_cast<EiDeviceRAM *>(mem);
            sample_buffer = reinterpret_cast<microphone_sample_t *>(
                ((uint8_t *)(ram->get_ram())) + buffer_offset);
            int res = mic->async_start(sample_buffer, buf_size);
            if (res) {
                ei_printf("Failed to start microphone %i\n", res);
                return false;
            }
            samples_written = mic->await_samples();
        }
        else {
            // either use block size, or the requested buffer size
            auto buf_size = requested_buffer_size ? requested_buffer_size : mem->block_size;
            // then clamp if the buffer is bigger than samples still needed
            buf_size = buf_size > samples_left ? samples_left : buf_size;

            sample_buffer = (microphone_sample_t *)ei_malloc(buf_size);
            if (!sample_buffer) {
                ei_printf("Failed to allocate memory\n");
                return false;
            }
            free_me = true;
            int res = mic->async_start(sample_buffer, buf_size);
            if (res) {
                ei_printf("Failed to start microphone %i\n", res);
                return false;
            }
            samples_written = mic->await_samples();
            mem->write_data(
                (uint8_t *)sample_buffer,
                buffer_offset,
                samples_written * sizeof(microphone_sample_t));
        }

        // update data hash
        ei_mic_ctx.signature_ctx->update(
            ei_mic_ctx.signature_ctx,
            (uint8_t *)sample_buffer,
            samples_written * sizeof(microphone_sample_t));

        buffer_offset += samples_written * sizeof(microphone_sample_t);
        samples_left -= samples_written;
        if (free_me) {
            ei_free(sample_buffer);
        }
    }

    auto ret = ei_mic_ctx.signature_ctx->finish(ei_mic_ctx.signature_ctx, ei_mic_ctx.hash_buffer.buffer);
    if (ret != 0) {
        ei_printf("Failed to finish signature (%d)\n", ret);
        return false;
    }

    // load the first page in flash...
    page_buffer = (uint8_t *)malloc(mem->block_size);
    // attach a smart pointer to auto free
    ei_unique_ptr_t auto_free(page_buffer, ei_free);
    if (!page_buffer) {
        ei_printf("Failed to allocate a page buffer to write the hash\n");
        return false;
    }

    auto ret2 = mem->read_data(page_buffer, 0, mem->block_size);
    if (ret2 != mem->block_size) {
        ei_printf("Failed to read first page (read %ld, requersted %ld)\n", ret2, mem->block_size);
        return false;
    }

    // update the hash
    uint8_t *hash = ei_mic_ctx.hash_buffer.buffer;
    // we have allocated twice as much for this data (because we also want to be able to represent in hex)
    // thus only loop over the first half of the bytes as the signature_ctx has written to those
    for (size_t hash_ix = 0; hash_ix < ei_mic_ctx.hash_buffer.size / 2; hash_ix++) {
        // this might seem convoluted, but snprintf() with %02x is not always supported e.g. by newlib-nano
        // we encode as hex... first ASCII char encodes top 4 bytes
        uint8_t first = (hash[hash_ix] >> 4) & 0xf;
        // second encodes lower 4 bytes
        uint8_t second = hash[hash_ix] & 0xf;

        // if 0..9 -> '0' (48) + value, if >10, then use 'a' (97) - 10 + value
        char first_c = first >= 10 ? 87 + first : 48 + first;
        char second_c = second >= 10 ? 87 + second : 48 + second;

        page_buffer[ei_mic_ctx.signature_index + (hash_ix * 2) + 0] = first_c;
        page_buffer[ei_mic_ctx.signature_index + (hash_ix * 2) + 1] = second_c;
    }

    auto ret3 = mem->erase_data(0, mem->block_size);
    if (ret3 != mem->block_size) {
        ei_printf("Failed to erase first page (read %ld, requested %ld)\n", ret3, mem->block_size);
        return false;
    }

    ret3 = mem->write_data(page_buffer, 0, mem->block_size);

    if (ret3 != mem->block_size) {
        ei_printf(
            "Failed to write first page with updated hash (read %ld, requested %ld)\n",
            ret3,
            mem->block_size);
        return false;
    }

    ei_printf("Done sampling, total bytes collected: %lu\n", buffer_offset);
    ei_printf("[1/1] Uploading file to Edge Impulse...\n");
    ei_printf(
        "Not uploading file, not connected to WiFi. Used buffer, from=%u, to=%lu.\n",
        0,
        buffer_offset);
    ei_printf("[1/1] Uploading file to Edge Impulse OK (took %d ms.)\n", 0);//upload_timer.read_ms());
    ei_printf("OK\n");

    return true;
}

// ********* Dummy EiMicrophone.  Good for test / early dev
class EiMicrophoneDummy : public EiMicrophone {
    uint32_t size;

public:
    int async_start(microphone_sample_t *buffer, uint32_t size) override
    {
        this->size = size;
        for (uint32_t i = 0; i < size; i++) {
            buffer[i] = i;
        }
        return 0;
    }

    uint32_t await_samples()
    {
        return size;
    }
};

#endif //include guard