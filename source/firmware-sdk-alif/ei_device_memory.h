/* Edge Impulse firmware SDK
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

#ifndef EI_DEVICE_MEMORY_H
#define EI_DEVICE_MEMORY_H

#include <cmath>
#include <cstdint>
#include <cstring>

/**
 * @brief Interface class for all memory type storages in Edge Impulse compatible devices.
 * The memory should be organized in a blocks, because all EI sensor drivers depends on block oranization.
 * Additionaly, at least one or two block of memory at the beginning is used for device configuration
 * (see ei_device_info_lib.h)
 */
class EiDeviceMemory {
protected:
    /**
     * @brief number of blocks occupied by config. Typically 1, but depending on memory
     * type and config size, it can be multiple blocks.
     * 
     */
    uint32_t used_blocks;
    /**
     * @brief total number of blocks in the memory
     * 
     */
    uint32_t memory_blocks;
    /**
     * @brief total size of memory, typically integer multiply of blocks
     * 
     */
    uint32_t memory_size;

public:
    /**
     * @brief Direct read from memory, should be implemented per device/memory type.
     * 
     * @param data pointer to buffer for data to be read
     * @param address absolute address in the memory (format and value depending on memory chip/type)
     * @param num_bytes number of bytes to be read @refitem data should be at leas num_bytes long
     * @return uint32_t number of bytes actually read. If differs from num_bytes, then some error occured.
     */
    virtual uint32_t read_data(uint8_t *data, uint32_t address, uint32_t num_bytes) = 0;

    /**
     * @brief Direct write to memory, should be implemented per device/memory type.
     * 
     * @param data pointer to bufer with data to write
     * @param address absolute address in the memory (format and value depending on memory chip/type)
     * @param num_bytes number of bytes to write
     * @return uint32_t number of bytes that has been written, if differs from num_bytes some error occured.
     */
    virtual uint32_t write_data(const uint8_t *data, uint32_t address, uint32_t num_bytes) = 0;

    /**
     * @brief Erase memory region
     * 
     * @param address absolute address in memory whwere the erase should begin. Typically block aligned.
     * @param num_bytes number of bytes to be erased, typically multiple of block size.
     * @return uint32_t numer of bytes that has been erased, if differes from num_bytes, then some oerror occured.
     */
    virtual uint32_t erase_data(uint32_t address, uint32_t num_bytes) = 0;
    /**
     * @brief size of the memory block in bytes
     */

    const uint32_t block_size;

    /**
     * @brief Construct a new Ei Device Memory object, make sure to pass all necessary data
     * from derived class. Usually constructor of the derived class needs to get this data 
     * from user code or read from chip.
     * 
     * @param memory_size see property description
     * @param block_size see property description
     */
    EiDeviceMemory(uint32_t memory_size, uint32_t block_size)
        : used_blocks(0)
        , memory_blocks(memory_size / block_size)
        , memory_size(memory_size)
        , block_size(block_size)
    {
    }
    virtual uint32_t get_available_sample_blocks(void)
    {
        return memory_blocks - used_blocks;
    }

    virtual uint32_t get_available_sample_bytes(void)
    {
        return (memory_blocks - used_blocks) * block_size;
    }

    virtual bool is_ram()
    {
        return false;
    }
};

class EiDeviceRAM : public EiDeviceMemory {

protected:
    // Make as uint32 so we'll be 4B aligned
    uint32_t *ram_memory;

public:
    uint32_t read_data(uint8_t *data, uint32_t address, uint32_t num_bytes)
    {
        if (num_bytes > memory_size - address) {
            num_bytes = memory_size - address;
        }

        memcpy(data, &ram_memory[address], num_bytes);

        return num_bytes;
    }

    uint32_t write_data(const uint8_t *data, uint32_t address, uint32_t num_bytes)
    {
        if (num_bytes > memory_size - address) {
            num_bytes = memory_size - address;
        }

        memcpy(&ram_memory[address], data, num_bytes);

        return num_bytes;
    }

    uint32_t erase_data(uint32_t address, uint32_t num_bytes)
    {
        if (num_bytes > memory_size - address) {
            num_bytes = memory_size - address;
        }

        memset(&ram_memory[address], 0, num_bytes);

        return num_bytes;
    }

    // use assign function to attach memory later
    EiDeviceRAM()
        : EiDeviceMemory(0, 512)
    {
    }

    bool is_ram() override
    {
        return true;
    }

    uint32_t *get_ram()
    {
        return ram_memory;
    }

    void assign_memory(uint32_t *buffer, uint32_t size)
    {
        ram_memory = buffer;
        memory_size = size;
    }
};

// abstract saving off of config somewhere.  RAM version not needed -AE
class EiDeviceConfig {
public:
    virtual void save_config(const uint8_t *data, uint32_t size) = 0;
    virtual void load_config(uint8_t *data, uint32_t size) = 0;
};

#endif /* EI_DEVICE_MEMORY_H */
