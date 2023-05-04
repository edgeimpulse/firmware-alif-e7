/*
 * Copyright (c) 2020-2022 Arm Limited. All rights reserved.
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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <mpu.hpp>

#include <cachel1_armv7.h>
#include <inttypes.h>
#include <stdio.h>

using namespace std;

/****************************************************************************
 * Functions
 ****************************************************************************/

namespace EthosU {
namespace Mpu {

void dump() {
#ifdef ARM_MPU_ARMV8_H
    uint32_t mpuRegions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

    printf("MPU available with %" PRIu32 " regions.\n", mpuRegions);

    printf("    PRIVDEFENA : %lx\n", (MPU->CTRL & MPU_CTRL_PRIVDEFENA_Msk) >> MPU_CTRL_PRIVDEFENA_Pos);
    printf("      HFNMIENA : %lx\n", (MPU->CTRL & MPU_CTRL_HFNMIENA_Msk) >> MPU_CTRL_HFNMIENA_Pos);
    printf("        ENABLE : %lx\n", (MPU->CTRL & MPU_CTRL_ENABLE_Msk) >> MPU_CTRL_ENABLE_Pos);

    for (size_t region = 0; region < mpuRegions; region++) {
        MPU->RNR = region;
        printf("-- Region %2d - RBAR:%08" PRIx32 " RLAR:%08" PRIx32 "\n", region, MPU->RBAR, MPU->RLAR);
    }
#endif
}

#ifdef ARM_MPU_ARMV8_H
static void initializeAttributes() {
    /* Initialize attributes corresponding to the enums defined in mpu.hpp */
    const uint8_t WTRA =
        ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0); // Non-transient, Write-Through, Read-allocate, Not Write-allocate
    const uint8_t WBWARA = ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1); // Non-transient, Write-Back, Read-allocate, Write-allocate
    const uint8_t WTWARA =
        ARM_MPU_ATTR_MEMORY_(1, 0, 1, 1); // Non-transient, Write-Through, Read-allocate, Write-allocate

    ARM_MPU_SetMemAttr(WTRA_index, ARM_MPU_ATTR(WTRA, WTRA));
    ARM_MPU_SetMemAttr(WBWARA_index, ARM_MPU_ATTR(WBWARA, WBWARA));
    ARM_MPU_SetMemAttr(WTWARA_index, ARM_MPU_ATTR(WTWARA, WTWARA));
}
#endif

void loadAndEnableConfig(ARM_MPU_Region_t const *table, uint32_t cnt) {
#ifdef ARM_MPU_ARMV8_H
    initializeAttributes();

    ARM_MPU_Load(0, table, cnt);

    // Enable MPU with default priv access to all other regions
    ARM_MPU_Enable((1 << MPU_CTRL_PRIVDEFENA_Pos) & MPU_CTRL_PRIVDEFENA_Msk);
#endif
}

}; // namespace Mpu
}; // namespace EthosU
