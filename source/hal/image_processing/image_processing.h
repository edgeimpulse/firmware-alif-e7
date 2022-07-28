/*
 * resize_frame.h
 *
 *  Created on: 24-Jan-2022
 *      Author: prasanna
 */

#ifndef IMAGE_PROCESSING_H_
#define IMAGE_PROCESSING_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RGB_BYTES 		3
#define PIXEL_BYTES 	1

// Camera dimensions
#define CIMAGE_X		560
#define CIMAGE_Y		560

// ML scaled image dimensions
#define MIMAGE_X		96
#define MIMAGE_Y		96

// Display dimensions
#define DIMAGE_X		480
#define DIMAGE_Y		800
#define DISPLAY_BUFFER_SIZE  (DIMAGE_X*DIMAGE_Y*RGB_BYTES)

#define TIFF_HDR_NUM_ENTRY 8
#define TIFF_HDR_SIZE 10+TIFF_HDR_NUM_ENTRY*12

/*error status*/
#define FRAME_FORMAT_NOT_SUPPORTED   -1
#define FRAME_OUT_OF_RANGE           -2

#define IMAGE_DATA_SIZE  ((MIMAGE_X*MIMAGE_Y*PIXEL_BYTES*RGB_BYTES)+ TIFF_HDR_SIZE)

#define CAMERA_MODE_VIDEO 		0x0001
#define CAMERA_MODE_SNAPSHOT 	0x0011


typedef struct {
	uint8_t tiff_header[TIFF_HDR_SIZE];
	union {
		uint8_t		bytes[MIMAGE_X*MIMAGE_Y*RGB_BYTES];
		uint32_t  	words[(MIMAGE_X*MIMAGE_Y*RGB_BYTES + 2) / 4];
	} buffer;
} ml_image_t;


int frame_crop(void *input_fb, uint32_t ip_row_size, uint32_t ip_col_size, uint32_t row_start, uint32_t col_start, void *output_fb, uint32_t op_row_size, uint32_t op_col_size, uint32_t bpp);
int crop_and_interpolate( uint8_t const *srcImage, uint32_t srcWidth, uint32_t srcHeight, uint8_t *dstImage, uint32_t dstWidth, uint32_t dstHeight, uint32_t bpp);
void color_correction(uint8_t *sp, uint8_t *dp);
void white_balance(uint8_t *sp, uint8_t *dp);
uint8_t *put_tiff(uint8_t * rgb, uint32_t width, uint32_t height, uint16_t bpp);
int bayer_to_RGB(uint8_t *src, uint8_t *dest);

int32_t camera_init();
int32_t camera_vsync(uint32_t timeout_ms);
int32_t camera_wait(uint32_t timeout_ms);
int32_t camera_start(uint32_t mode, uint8_t* buffer);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_PROCESSING_H_ */
