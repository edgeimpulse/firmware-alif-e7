/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BSP_CORE_LOG_H
#define BSP_CORE_LOG_H

#include "uart_stdout.h"    /* UART for logging */

#include <stdio.h>

#define LOG_LEVEL_TRACE       0
#define LOG_LEVEL_DEBUG       1
#define LOG_LEVEL_INFO        2
#define LOG_LEVEL_WARN        3
#define LOG_LEVEL_ERROR       4

#ifndef LOG_LEVEL
#define LOG_LEVEL             LOG_LEVEL_INFO
#endif /*LOG_LEVEL*/

#if (LOG_LEVEL == LOG_LEVEL_TRACE)
    #define trace(...)        printf("TRACE - "); printf(__VA_ARGS__)
#else
    #define trace(...)
#endif  /* LOG_LEVEL == LOG_LEVEL_TRACE */

#if (LOG_LEVEL <= LOG_LEVEL_DEBUG)
    #define debug(...)        printf("DEBUG - "); printf(__VA_ARGS__)
#else
    #define debug(...)
#endif  /* LOG_LEVEL > LOG_LEVEL_TRACE */

#if (LOG_LEVEL <= LOG_LEVEL_INFO)
    #define info(...)         printf("INFO - "); printf(__VA_ARGS__)
#else
    #define info(...)
#endif  /* LOG_LEVEL > LOG_LEVEL_DEBUG */

#if (LOG_LEVEL <= LOG_LEVEL_WARN)
    #define warn(...)         printf("WARN - "); printf(__VA_ARGS__)
#else
    #define warn(...)
#endif  /* LOG_LEVEL > LOG_LEVEL_INFO */

#if (LOG_LEVEL <= LOG_LEVEL_ERROR)
    #define printf_err(...)   printf("ERROR - "); printf(__VA_ARGS__)
#else
    #define printf_err(...)
#endif  /* LOG_LEVEL > LOG_LEVEL_INFO */

#define UNUSED(x)       ((void)(x))

#endif /* BSP_CORE_LOG_H */
