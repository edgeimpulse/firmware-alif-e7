
/*
 * Copyright (C) 2010-2021 Arm Limited or its affiliates. All rights reserved.
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

/* ----------------------------------------------------------------------
 * Project:      Arm-2D Library
 * Title:        __arm_2d_alpha_blending.inc
 * Description:  c code template for drawing pattern
 *
 * $Date:        08. Sept 2021
 * $Revision:    V.1.1.0
 *
 * -------------------------------------------------------------------- */


#ifndef __API_COLOUR
#   error You have to define __API_COLOUR before using this c template
#endif
#ifndef __API_INT_TYPE
#   error You have to define the __API_INT_TYPE before using this c template
#endif
#ifndef __API_PIXEL_BLENDING
#   error You have to define __API_PIXEL_BLENDING before using this c template
#endif

#undef ____ARM_2D_FUNC
#undef ___ARM_2D_FUNC
#undef __ARM_2D_FUNC
#define ____ARM_2D_FUNC(__NAME, __COLOUR)  __arm_2d_impl_##__COLOUR##_##__NAME
#define ___ARM_2D_FUNC(__NAME, __COLOUR)   ____ARM_2D_FUNC(__NAME, __COLOUR)
#define __ARM_2D_FUNC(__NAME)   ___ARM_2D_FUNC(__NAME, __API_COLOUR)

#ifndef __PATCH_ALPHA_BLENDING
__WEAK
void __ARM_2D_FUNC(alpha_blending) (__API_INT_TYPE *__RESTRICT pSourceBase,
                                    int16_t iSourceStride,
                                    __API_INT_TYPE *__RESTRICT pTargetBase,
                                    int16_t iTargetStride,
                                    arm_2d_size_t *__RESTRICT ptCopySize,
                                    uint_fast16_t hwRatio)
{
    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
    hwRatio += (hwRatio == 255);
#endif

    uint16_t        hwRatioCompl = 256 - hwRatio;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {
            __API_PIXEL_BLENDING(pSourceBase++, pTargetBase++, hwRatioCompl);
        }

        pSourceBase += (iSourceStride - iWidth);
        pTargetBase += (iTargetStride - iWidth);
    }

}
#else
extern
void __ARM_2D_FUNC(alpha_blending) (__API_INT_TYPE *__RESTRICT pSourceBase,
                                    int16_t iSourceStride,
                                    __API_INT_TYPE *__RESTRICT pTargetBase,
                                    int16_t iTargetStride,
                                    arm_2d_size_t *__RESTRICT ptCopySize,
                                    uint_fast16_t hwRatio);
#endif



#ifndef __PATCH_ALPHA_BLENDING_COLOUR_MASKING
__WEAK
void __ARM_2D_FUNC(alpha_blending_colour_keying)(
                                                __API_INT_TYPE * __RESTRICT pSourceBase,
                                                int16_t         iSourceStride,
                                                __API_INT_TYPE * __RESTRICT pTargetBase,
                                                int16_t         iTargetStride,
                                                arm_2d_size_t * __RESTRICT ptCopySize,
                                                uint_fast16_t hwRatio,
                                                __API_INT_TYPE   Colour)
{
    int_fast16_t    iHeight = ptCopySize->iHeight;
    int_fast16_t    iWidth = ptCopySize->iWidth;

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
    hwRatio += (hwRatio == 255);
#endif

    uint16_t        hwRatioCompl = 256 - hwRatio;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {

            if (*pSourceBase != Colour) {
                __API_PIXEL_BLENDING( pSourceBase, pTargetBase, hwRatioCompl);
            }
            pSourceBase++;
            pTargetBase++;
        }
        pSourceBase += (iSourceStride - iWidth);
        pTargetBase += (iTargetStride - iWidth);
    }
}
#else
extern
void __ARM_2D_FUNC(alpha_blending_colour_keying)(
                                                __API_INT_TYPE * __RESTRICT pSourceBase,
                                                int16_t         iSourceStride,
                                                __API_INT_TYPE * __RESTRICT pTargetBase,
                                                int16_t         iTargetStride,
                                                arm_2d_size_t * __RESTRICT ptCopySize,
                                                uint_fast16_t hwRatio,
                                                __API_INT_TYPE   Colour);
#endif

#ifndef __PATCH_COLOUR_FILLING_WITH_ALPHA
__WEAK
void __ARM_2D_FUNC(colour_filling_with_opacity)(
                                        __API_INT_TYPE *__RESTRICT pTargetBase,
                                        int16_t iTargetStride,
                                        arm_2d_size_t *__RESTRICT ptCopySize,
                                        __API_INT_TYPE Colour,
                                        uint_fast16_t hwRatio)
{
    int_fast16_t iWidth = ptCopySize->iWidth;
    int_fast16_t iHeight = ptCopySize->iHeight;

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
    hwRatio += (hwRatio == 255);
#endif

    uint16_t hwRatioCompl = 256 - hwRatio;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++){
            __API_PIXEL_BLENDING( &Colour,  pTargetBase++, hwRatioCompl);
        }

        pTargetBase += iTargetStride - iWidth;
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_with_opacity)(
                                        __API_INT_TYPE *__RESTRICT pTargetBase,
                                        int16_t iTargetStride,
                                        arm_2d_size_t *__RESTRICT ptCopySize,
                                        __API_INT_TYPE Colour,
                                        uint_fast16_t hwRatio);
#endif


#ifndef __PATCH_COLOUR_FILLING_A2_MASK
__WEAK
void __ARM_2D_FUNC(colour_filling_a2_mask)(
                            __API_INT_TYPE *__RESTRICT pTargetBase,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour)
{
    //! get in byte offset
    nAlphaOffset &= 0x03;
    nAlphaOffset *= 2;
    iAlphaStride = (iAlphaStride + 3) & ~0x03;
    iAlphaStride >>= 2;

    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        uint8_t *__RESTRICT pchMask = pchAlpha;
        int32_t nOffset = nAlphaOffset;
        uint8_t chBitMask = 0x03 << nOffset;
        __API_INT_TYPE * __RESTRICT pTarget = pTargetBase;

        for (int_fast16_t x = 0; x < iWidth; x++) {

            uint16_t hwTempAlpha = ((*pchMask & chBitMask) >> nOffset) * 85;

            uint16_t hwAlpha = 256 - hwTempAlpha;
#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 1);
#endif
           __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);

            chBitMask <<= 2;
            nOffset += 2;
            if (!chBitMask) {
                chBitMask = 0x03;
                nOffset = 0;
                pchMask++;
            }
        }

        pchAlpha += iAlphaStride;
        pTargetBase += iTargetStride;
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_a2_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour);
#endif


#ifndef __PATCH_COLOUR_FILLING_A4_MASK
__WEAK
void __ARM_2D_FUNC(colour_filling_a4_mask)(
                            __API_INT_TYPE *__RESTRICT pTargetBase,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour)
{
    //! get in byte offset
    nAlphaOffset &= 0x01;
    nAlphaOffset *= 4;
    iAlphaStride = (iAlphaStride + 1) & ~0x01;
    iAlphaStride >>= 1;

    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        uint8_t *__RESTRICT pchMask = pchAlpha;
        int32_t nOffset = nAlphaOffset;
        uint8_t chBitMask = 0x0F << nOffset;
        __API_INT_TYPE * __RESTRICT pTarget = pTargetBase;

        for (int_fast16_t x = 0; x < iWidth; x++) {

            uint16_t hwTempAlpha = ((*pchMask & chBitMask) >> nOffset) * 17;

            uint16_t hwAlpha = 256 - hwTempAlpha;
#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 1);
#endif
            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);


            chBitMask <<= 4;
            nOffset += 4;
            if (!chBitMask) {
                chBitMask = 0x0F;
                nOffset = 0;
                pchMask++;
            }
        }

        pchAlpha += iAlphaStride;
        pTargetBase += iTargetStride;
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_a4_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour);
#endif



#ifndef __PATCH_COLOUR_FILLING_ALPHA_MASK
__WEAK
void __ARM_2D_FUNC(colour_filling_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour)
{
    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwAlpha = 256 - (*pchAlpha++);
#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 1);
#endif
            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);
        }

        pchAlpha += (iAlphaStride - iWidth);
        pTarget += (iTargetStride - iWidth);
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour);
#endif


#ifndef __PATCH_COLOUR_FILLING_A2_MASK_OPACITY
__WEAK
void __ARM_2D_FUNC(colour_filling_a2_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTargetBase,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity)
{
    //! get in byte offset
    nAlphaOffset &= 0x03;
    nAlphaOffset *= 2;
    iAlphaStride = (iAlphaStride + 3) & ~0x03;
    iAlphaStride >>= 2;

    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;


    for (int_fast16_t y = 0; y < iHeight; y++) {

        uint8_t *__RESTRICT pchMask = pchAlpha;
        int32_t nOffset = nAlphaOffset;
        uint8_t chBitMask = 0x03 << nOffset;
        __API_INT_TYPE * __RESTRICT pTarget = pTargetBase;

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwTempAlpha = ((*pchMask & chBitMask) >> nOffset) * 85;
            uint16_t hwAlpha = 256 - (hwTempAlpha * hwOpacity >> 8);

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 2) * 2;
#endif

            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);

            chBitMask <<= 2;
            nOffset += 2;
            if (!chBitMask) {
                chBitMask = 0x03;
                nOffset = 0;
                pchMask++;
            }
        }

        pchAlpha += iAlphaStride;
        pTargetBase += iTargetStride;
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_a2_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity);
#endif



#ifndef __PATCH_COLOUR_FILLING_A4_MASK_OPACITY
__WEAK
void __ARM_2D_FUNC(colour_filling_a4_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTargetBase,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity)
{
    //! get in byte offset
    nAlphaOffset &= 0x01;
    nAlphaOffset *= 4;
    iAlphaStride = (iAlphaStride + 1) & ~0x01;
    iAlphaStride >>= 1;

    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;


    for (int_fast16_t y = 0; y < iHeight; y++) {

        uint8_t *__RESTRICT pchMask = pchAlpha;
        int32_t nOffset = nAlphaOffset;
        uint8_t chBitMask = 0x0F << nOffset;
        __API_INT_TYPE * __RESTRICT pTarget = pTargetBase;

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwTempAlpha = ((*pchMask & chBitMask) >> nOffset) * 17;
            uint16_t hwAlpha = 256 - (hwTempAlpha * hwOpacity >> 8);

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 2) * 2;
#endif

            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);

            chBitMask <<= 4;
            nOffset += 4;
            if (!chBitMask) {
                nOffset = 0;
                chBitMask = 0x0F;
                pchMask++;
            }
        }

        pchAlpha += iAlphaStride;
        pTargetBase += iTargetStride;
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_a4_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            int32_t nAlphaOffset,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity);
#endif


#ifndef __PATCH_COLOUR_FILLING_ALPHA_MASK_OPACITY
__WEAK
void __ARM_2D_FUNC(colour_filling_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity)
{
    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;


    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwAlpha = 256 - ((*pchAlpha++) * hwOpacity >> 8);

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 2) * 2;
#endif

            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);
        }

        pchAlpha += (iAlphaStride - iWidth);
        pTarget += (iTargetStride - iWidth);
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint8_t *__RESTRICT pchAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity);
#endif



#ifndef __PATCH_COLOUR_FILLING_CHANNEL_MASK
__WEAK
void __ARM_2D_FUNC(colour_filling_channel_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint32_t *__RESTRICT pwAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour)
{
    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwAlpha = 256 - *(uint8_t *)(pwAlpha++);

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 1);
#endif

            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);
        }

        pwAlpha += (iAlphaStride - iWidth);
        pTarget += (iTargetStride - iWidth);
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_channel_mask)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint32_t *__RESTRICT pwAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour);
#endif


#ifndef __PATCH_COLOUR_FILLING_CHANNEL_MASK_OPACITY
__WEAK
void __ARM_2D_FUNC(colour_filling_channel_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint32_t *__RESTRICT pwAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity)
{
    int_fast16_t iHeight = ptCopySize->iHeight;
    int_fast16_t iWidth  = ptCopySize->iWidth;

    for (int_fast16_t y = 0; y < iHeight; y++) {

        for (int_fast16_t x = 0; x < iWidth; x++) {
            uint16_t hwAlpha = 256 - (*(uint8_t *)(pwAlpha++) * hwOpacity >> 8);

#if !defined(__ARM_2D_CFG_UNSAFE_IGNORE_ALPHA_255_COMPENSATION__)
            hwAlpha -= (hwAlpha == 2) * 2;
#endif

            __API_PIXEL_BLENDING(&Colour, pTarget++, hwAlpha);
        }

        pwAlpha += (iAlphaStride - iWidth);
        pTarget += (iTargetStride - iWidth);
    }
}
#else
extern
void __ARM_2D_FUNC(colour_filling_channel_mask_opacity)(
                            __API_INT_TYPE *__RESTRICT pTarget,
                            int16_t iTargetStride,
                            uint32_t *__RESTRICT pwAlpha,
                            int16_t iAlphaStride,
                            arm_2d_size_t *__RESTRICT ptCopySize,
                            __API_INT_TYPE Colour,
                            uint_fast16_t hwOpacity);
#endif


#undef ____ARM_2D_FUNC
#undef ___ARM_2D_FUNC
#undef __ARM_2D_FUNC
#undef __API_COLOUR
#undef __API_INT_TYPE
#undef __API_PIXEL_BLENDING
#undef __PATCH_ALPHA_BLENDING
#undef __PATCH_ALPHA_BLENDING_COLOUR_MASKING
#undef __PATCH_COLOUR_FILLING_WITH_ALPHA
#undef __PATCH_COLOUR_FILLING_CHANNEL_MASK
#undef __PATCH_COLOUR_FILLING_ALPHA_MASK
#undef __PATCH_COLOUR_FILLING_ALPHA_MASK_OPACITY
#undef __PATCH_COLOUR_FILLING_CHANNEL_MASK_OPACITY

