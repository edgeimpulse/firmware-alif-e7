/*
 * Copyright (c) 2021-2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************************************************
 * Model: keyword_spotting_ds_cnn_large_clustered_int8
 * Source: https://github.com/ARM-software/ML-zoo/tree/22.02/models/keyword_spotting/ds_cnn_large/tflite_clustered_int8
 * Hexdump: python3 -c 'import numpy;i=numpy.load("output.npy");i.tofile("output.bin");' && xxd -i output.bin > output.h
 ******************************************************************************************************************/

unsigned char expectedOutputData[] __attribute__((
    aligned(16),
    section("expected_output_data_sec"))) = {0x80, 0x3d, 0x80, 0x80, 0x80, 0x80, 0x82, 0x80, 0x80, 0x80, 0x80, 0xc1};
