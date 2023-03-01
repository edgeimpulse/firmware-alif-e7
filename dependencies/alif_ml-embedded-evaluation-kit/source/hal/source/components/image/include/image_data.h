/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement 
 *
 * You should have received a copy of the Alif Semiconductor Software 
 * License Agreement with this file. If not, please write to: 
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int image_init();

const uint8_t *get_image_data(int width, int height);


#endif // IMAGE_DATA_H
