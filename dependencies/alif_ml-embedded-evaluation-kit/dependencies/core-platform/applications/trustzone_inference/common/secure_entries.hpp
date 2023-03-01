/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
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
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SECURE_ENTRIES_H__
#define SECURE_ENTRIES_H__

/* output a string on secure side stdio */
extern "C" void nonsecure_print(const char *p);

/* Run an inference on the secure side and return status */
extern "C" int run_secure_inference(void);

typedef int (*funcptr_ns)(void) __attribute__((cmse_nonsecure_call));
extern "C" void set_result_function(funcptr_ns callback_fn);
#endif // SECURE_ENTRIES_H__
