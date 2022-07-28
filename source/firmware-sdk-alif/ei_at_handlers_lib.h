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

#ifndef __EI_AT_HANDLERS_LIB__H__
#define __EI_AT_HANDLERS_LIB__H__

bool at_get_device_id(void);

bool at_set_device_id(const char **argv, const int argc);

bool at_get_upload_host(void);

bool at_set_upload_host(const char **argv, const int argc);

bool at_get_upload_settings(void);

bool at_set_upload_settings(const char **argv, const int argc);

bool at_get_mgmt_url(void);

bool at_set_mgmt_url(const char **argv, const int argc);

bool at_get_sample_settings(void);

bool at_set_sample_settings(const char **argv, const int argc);

bool at_clear_config(void);

bool at_unlink_file(const char **argv, const int argc);

bool at_read_buffer(const char **argv, const int argc);

bool at_sample_start(const char **argv, const int argc);

bool at_get_snapshot(void);

bool at_get_config(void);

#endif  //!__EI_AT_HANDLERS_LIB__H__