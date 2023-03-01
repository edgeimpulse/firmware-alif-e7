/*
 * SPDX-FileCopyrightText: Copyright 2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
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
#ifndef ETHOS_U_NPU_INIT_H
#define ETHOS_U_NPU_INIT_H

#if defined(ARM_NPU)

/**
 * @brief   Initialises the Arm Ethos-U NPU
 * @return  0 if successful, error code otherwise
 **/
int arm_ethosu_npu_init(void);

/**
 * @brief   Defines the Arm Ethos-U NPU interrupt handler: just a wrapper
 *          around the default implementation.
 **/
void arm_ethosu_npu_irq_handler(void);

#endif /* ARM_NPU */

#endif /* ETHOS_U_NPU_INIT_H */
