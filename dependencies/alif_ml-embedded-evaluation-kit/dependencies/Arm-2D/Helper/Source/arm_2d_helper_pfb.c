/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
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
 * Title:        #include "arm_2d_helper_pfb.c"
 * Description:  the pfb helper service source code
 *
 * $Date:        25. Nov 2022
 * $Revision:    V.1.3.9
 *
 * Target Processor:  Cortex-M cores
 * -------------------------------------------------------------------- */

/*============================ INCLUDES ======================================*/
#define __ARM_2D_IMPL__

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "arm_2d_helper.h"

#if defined(__PERF_COUNTER__)
#   include "perf_counter.h"
#else
#   include <time.h>
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#   pragma clang diagnostic ignored "-Wundef"
#   pragma clang diagnostic ignored "-Wgnu-statement-expression"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#   pragma clang diagnostic ignored "-Wunreachable-code-return"
#elif defined(__IS_COMPILER_GCC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#   pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

/*============================ MACROS ========================================*/

#undef  this
#define this            (*ptThis)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1)
arm_2d_pfb_t *__arm_2d_helper_pfb_new(arm_2d_helper_pfb_t *ptThis)
{
    assert(NULL != ptThis);
    
    arm_2d_pfb_t *ptPFB = NULL;
    arm_irq_safe {
        this.Adapter.hwFreePFBCount--;
        ARM_LIST_STACK_POP(this.Adapter.ptFreeList, ptPFB);
    }
    
    ptPFB->ptPFBHelper = ptThis;
    
    return ptPFB;
}

ARM_NONNULL(1)
void __arm_2d_helper_pfb_free(arm_2d_helper_pfb_t *ptThis, arm_2d_pfb_t *ptPFB)
{
    assert(NULL != ptThis);
    
    if (NULL == ptPFB) {
        return ;
    }

    arm_irq_safe {
        ARM_LIST_STACK_PUSH(this.Adapter.ptFreeList, ptPFB);
        this.Adapter.hwFreePFBCount++;
    }
}

ARM_NONNULL(1,2)
arm_2d_err_t arm_2d_helper_pfb_init(arm_2d_helper_pfb_t *ptThis, 
                                    arm_2d_helper_pfb_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    
    arm_2d_helper_init();
    
    memset(ptThis, 0, sizeof(this));
    this.tCFG = *ptCFG;
    
    if (NULL == this.tCFG.Dependency.evtOnLowLevelRendering.fnHandler) {
        return ARM_2D_ERR_MISSING_PARAM;
    }
    
    // perform validation
    do {
        int_fast16_t n = this.tCFG.FrameBuffer.hwPFBNum;
        arm_2d_pfb_t *ptItem = this.tCFG.FrameBuffer.ptPFBs;
        uint32_t wBufferSize = this.tCFG.FrameBuffer.wBufferSize;
        
        // handle alignments
        wBufferSize += __alignof__(arm_2d_pfb_t) - 1;
        wBufferSize &= ~(__alignof__(arm_2d_pfb_t) - 1);
        
        if (0 == n || NULL == ptItem) {
            return ARM_2D_ERR_MISSING_PARAM;
        } else if ( (0 == this.tCFG.FrameBuffer.tFrameSize.iHeight)
                 || (0 == this.tCFG.FrameBuffer.tFrameSize.iWidth)
                 || (0 == this.tCFG.FrameBuffer.wBufferSize)) {
            return ARM_2D_ERR_INVALID_PARAM;
        }
        
        this.Adapter.hwFreePFBCount = 0;
        // add PFBs to pool
        do {
            ptItem->tTile = (arm_2d_tile_t) {
                .tRegion = {
                    .tSize = this.tCFG.FrameBuffer.tFrameSize,
                },
                .tInfo.bIsRoot = true,
                .pchBuffer = (uint8_t *)((uintptr_t)ptItem + sizeof(arm_2d_pfb_t)),
            };
            
            // update buffer size
            ptItem->u24Size = wBufferSize;

            __arm_2d_helper_pfb_free(ptThis, ptItem);
            
            // update pointer
            ptItem = (arm_2d_pfb_t *)(  (uintptr_t)ptItem 
                                     +  wBufferSize
                                     +  sizeof(arm_2d_pfb_t));
        } while(--n);
    
    } while(0);

    this.Adapter.bFirstIteration = true;
    this.Adapter.bIsFlushRequested = true;
    
    return ARM_2D_ERR_NONE;
}

ARM_NONNULL(1)
arm_2d_region_t arm_2d_helper_pfb_get_display_area(arm_2d_helper_pfb_t *ptThis)
{
    assert(NULL != ptThis);
    
    return this.tCFG.tDisplayArea;
}

__WEAK 
void arm_2d_helper_swap_rgb16(uint16_t *phwBuffer, uint32_t wCount)
{
    assert(NULL != phwBuffer);

    if (0 == wCount) {
        return ;
    }

    // aligned (2)
    assert((((uintptr_t) phwBuffer) & 0x01) == 0);

    // it is not aligned to 4
    if ((((uintptr_t) phwBuffer) & 0x03) == 0x02) {
        // handle the leading pixel
        uint32_t wTemp = *phwBuffer;
        *phwBuffer++ = (uint16_t)__REV16(wTemp);
        wCount--;
    }


    uint32_t wWords = wCount >> 1;
    uint32_t *pwBuffer = (uint32_t *)phwBuffer;
    wCount &= 0x01;

    if (wWords > 0) {
        do {
            uint32_t wTemp = *pwBuffer;
            *pwBuffer++ = __REV16(wTemp);
        } while(--wWords);
    }

    if (wCount) {
        uint32_t wTemp = *pwBuffer;
        (*(uint16_t *)pwBuffer) = (uint16_t)__REV16(wTemp);
    }
}

ARM_NONNULL(1)
void arm_2d_helper_pfb_flush(arm_2d_helper_pfb_t *ptThis)
{
    assert(NULL != ptThis);

    arm_2d_pfb_t *ptPFB = NULL;
    
    arm_irq_safe {
        ARM_LIST_QUEUE_DEQUEUE( this.Adapter.FlushFIFO.ptHead, 
                                this.Adapter.FlushFIFO.ptTail, 
                                ptPFB);
        this.Adapter.bIsFlushRequested = (NULL == ptPFB);
    }

    if (NULL != ptPFB) {
        this.Adapter.ptFlushing = ptPFB;
        ptPFB->ptPFBHelper = ptThis;

        // call handler
        (*this.tCFG.Dependency.evtOnLowLevelRendering.fnHandler)(
                        this.tCFG.Dependency.evtOnLowLevelRendering.pTarget,
                        ptPFB,
                        ptPFB->bIsNewFrame);
    }
}

static 
void __arm_2d_helper_enqueue_pfb(arm_2d_helper_pfb_t *ptThis)
{
    this.Adapter.ptCurrent->bIsNewFrame = this.Adapter.bFirstIteration;
    bool bIsFlushRequested;
    
    arm_irq_safe {
        bIsFlushRequested = this.Adapter.bIsFlushRequested;
        ARM_LIST_QUEUE_ENQUEUE( this.Adapter.FlushFIFO.ptHead, 
                                this.Adapter.FlushFIFO.ptTail, 
                                this.Adapter.ptCurrent);
    }
    
    if (bIsFlushRequested) {
        arm_2d_helper_pfb_flush(ptThis);
    }
    
}


static
void __arm_2d_helper_low_level_rendering(arm_2d_helper_pfb_t *ptThis)
{

    assert(NULL != this.tCFG.Dependency.evtOnLowLevelRendering.fnHandler);
    assert(NULL != this.Adapter.ptCurrent);
    
    // update location info
    this.Adapter.ptCurrent->tTile.tRegion.tLocation = (arm_2d_location_t) {
        .iX = this.Adapter.tDrawRegion.tLocation.iX
            + this.Adapter.tTargetRegion.tLocation.iX,
        .iY = this.Adapter.tDrawRegion.tLocation.iY
            + this.Adapter.tTargetRegion.tLocation.iY,
    };

    if (this.tCFG.FrameBuffer.bSwapRGB16) {
        arm_2d_helper_swap_rgb16( this.Adapter.ptCurrent->tTile.phwBuffer, 
                                    get_tile_buffer_pixel_count(
                                        this.Adapter.ptCurrent->tTile));
    }

    __arm_2d_helper_enqueue_pfb(ptThis);

    this.Adapter.bFirstIteration = false;

}


__STATIC_INLINE bool __when_dirty_region_list_is_empty(arm_2d_helper_pfb_t *ptThis)
{
    /* check whether has already been switched to the navigation dirty 
     * region list 
     */
    if (this.Adapter.bNoAdditionalDirtyRegionList) {
        // no dirty region is available
        this.Adapter.bFirstIteration = true;
        
        return false;
    } else if ( (NULL != this.tCFG.Dependency.Navigation.evtOnDrawing.fnHandler)
           &&   (NULL != this.tCFG.Dependency.Navigation.ptDirtyRegion)) {
        
        /* switch to navigation dirty region list */
        this.Adapter.ptDirtyRegion = this.tCFG.Dependency.Navigation.ptDirtyRegion;
        this.Adapter.bNoAdditionalDirtyRegionList = true;
        
        this.Adapter.bIsRegionChanged = true;
        return true;
    } else {
        // no dirty region is available
        this.Adapter.bFirstIteration = true;
        
        return false;
    }
}

static bool __arm_2d_helper_pfb_get_next_dirty_region(arm_2d_helper_pfb_t *ptThis)
{
    if (NULL == this.Adapter.ptDirtyRegion) {
        return __when_dirty_region_list_is_empty(ptThis);
    } 
    
    this.Adapter.ptDirtyRegion = this.Adapter.ptDirtyRegion->ptNext;
    
    if (NULL == this.Adapter.ptDirtyRegion) {
        // reach last item in a chain
        return __when_dirty_region_list_is_empty(ptThis);
    } else {
        this.Adapter.bIsRegionChanged = true;
    }
    
    return true;
}


/*! \brief begin a iteration of drawing and request a frame buffer from 
 *         low level display driver.
 *  \param[in] ptThis the PFB helper control block
 *  \param[in] ptTargetRegion the address of the target region in the LCD
 *         passing NULL means we want to draw the whole LCD.
 *  \retval NULL the display driver is not ready
 *  \retval (intptr_t)-1 the display driver want to ignore this drawing 
 *          (maybe because the target area is out of the LCD)
 *  \retval non-null a tile which contains the (partial) frame buffer
 */
static 
arm_2d_tile_t * __arm_2d_helper_pfb_drawing_iteration_begin(
                                    arm_2d_helper_pfb_t *ptThis,
                                    arm_2d_region_list_item_t *ptDirtyRegions)
{ 
//    arm_irq_safe {
//        ARM_LIST_STACK_POP(this.Adapter.ptFreeList, this.Adapter.ptCurrent);
//    }
    this.Adapter.ptCurrent = NULL;
    arm_irq_safe {
        /* allocating pfb only when the number of free PFB blocks is larger than
         * the reserved threashold
         */
        if (this.Adapter.hwFreePFBCount > this.tCFG.FrameBuffer.u4PoolReserve) {
            this.Adapter.ptCurrent = __arm_2d_helper_pfb_new(ptThis);
        }
    }
    
    if (NULL == this.Adapter.ptCurrent) {
        // no resource left
        return NULL;
    }
    arm_2d_tile_t *ptPartialFrameBuffer = &(this.Adapter.ptCurrent->tTile);

    if (this.Adapter.bFirstIteration) {
        this.Adapter.ptDirtyRegion = ptDirtyRegions;
        
        if (NULL == ptDirtyRegions) {
            /* since we draw the whole frame, no need to take the additional 
             * dirty region list into consideration 
             */
            this.Adapter.bNoAdditionalDirtyRegionList = true;
        } else {
            /* reset flag */
            this.Adapter.bNoAdditionalDirtyRegionList = false;
        }
        //this.Adapter.bFirstIteration = false;
        this.Adapter.bIsRegionChanged = true;
    }

    do {
        if (this.Adapter.bIsRegionChanged) {
        
            this.Adapter.bIsRegionChanged = false;

            if (NULL != this.Adapter.ptDirtyRegion) { 
                // calculate the valid region
                if (!arm_2d_region_intersect(   &this.tCFG.tDisplayArea, 
                                                &(this.Adapter.ptDirtyRegion->tRegion), 
                                                &this.Adapter.tTargetRegion)) {
                                                
                    if (__arm_2d_helper_pfb_get_next_dirty_region(ptThis)) {
                        // try next region
                        continue;
                    }
                    // out of lcd 
                    return (arm_2d_tile_t *)-1;
                }
            } else {
                this.Adapter.tTargetRegion = this.tCFG.tDisplayArea;
            }

        #if __ARM_ARCH == 6 || __TARGET_ARCH_THUMB == 3
            // reset adapter frame size
            this.Adapter.tFrameSize = this.tCFG.FrameBuffer.tFrameSize;
        #else
            if (this.tCFG.FrameBuffer.bDisableDynamicFPBSize) {
                // reset adapter frame size
                this.Adapter.tFrameSize = this.tCFG.FrameBuffer.tFrameSize;

            } else {                                                            //!< update PFB size
                uint32_t wTargetPixelCount 
                    = this.Adapter.tTargetRegion.tSize.iWidth
                    * this.Adapter.tTargetRegion.tSize.iHeight;
                
                uint32_t wPFBPixelCount
                    = this.tCFG.FrameBuffer.tFrameSize.iWidth
                    * this.tCFG.FrameBuffer.tFrameSize.iHeight;
                        
                if (    (wTargetPixelCount <= wPFBPixelCount)
                   ||   (   this.Adapter.tTargetRegion.tSize.iWidth 
                        <   this.tCFG.FrameBuffer.tFrameSize.iWidth)) {
                    // redefine the shape of PFB
                    
                    this.Adapter.tFrameSize.iWidth 
                        = this.Adapter.tTargetRegion.tSize.iWidth;
                        
                    this.Adapter.tFrameSize.iHeight = (int16_t)(
                            wPFBPixelCount 
                        /   (uint32_t)this.Adapter.tTargetRegion.tSize.iWidth);
                
                } else {
                    // reset adapter frame size
                    this.Adapter.tFrameSize = this.tCFG.FrameBuffer.tFrameSize;
                }
            } 
        #endif
        }
        break;
    } while(true);
    
    
    arm_2d_region_t tTempRegion = {
        .tSize = this.tCFG.tDisplayArea.tSize,
        .tLocation = {
            .iX = - (   this.Adapter.tTargetRegion.tLocation.iX 
                    +   this.Adapter.tDrawRegion.tLocation.iX),
            .iY = - (   this.Adapter.tTargetRegion.tLocation.iY 
                    +   this.Adapter.tDrawRegion.tLocation.iY),
        },
    };
    
    
    ptPartialFrameBuffer->tRegion.tSize.iWidth 
        = MIN(  this.Adapter.tFrameSize.iWidth, 
                this.Adapter.tTargetRegion.tSize.iWidth 
            -   this.Adapter.tDrawRegion.tLocation.iX);
    ptPartialFrameBuffer->tRegion.tSize.iHeight 
        = MIN(  this.Adapter.tFrameSize.iHeight, 
                this.Adapter.tTargetRegion.tSize.iHeight 
            -   this.Adapter.tDrawRegion.tLocation.iY);
            
    arm_2d_tile_generate_child( ptPartialFrameBuffer, 
                                &tTempRegion, 
                                &this.Adapter.tPFBTile, 
                                false);


    if (!this.tCFG.FrameBuffer.bDoNOTUpdateDefaultFrameBuffer) {
        // update default frame buffer
        arm_2d_set_default_frame_buffer(&this.Adapter.tPFBTile);
    }
    
    // uncomment this when necessary for debug purpose
    //arm_2d_rgb16_fill_colour(&this.Adapter.tPFBTile, NULL, 0);

    return (arm_2d_tile_t *)&(this.Adapter.tPFBTile);
}



/*! \brief end a drawing iteration and decide wether a new iteration is required
 *         or not based on the return value
 *  \param[in] ptThis the PFB control block
 *  \retval true a new iteration is required
 *  \retval false no more iteration is required
 */
static 
bool __arm_2d_helper_pfb_drawing_iteration_end(arm_2d_helper_pfb_t *ptThis)
{
    __arm_2d_helper_low_level_rendering(ptThis);
    
    arm_2d_tile_t *ptPartialFrameBuffer = &(this.Adapter.ptCurrent->tTile);
    
    if (!this.tCFG.FrameBuffer.bDoNOTUpdateDefaultFrameBuffer) {
        // update default frame buffer
        arm_2d_set_default_frame_buffer(NULL);
    }

    this.Adapter.tDrawRegion.tLocation.iX 
        += ptPartialFrameBuffer->tRegion.tSize.iWidth;
    if (    this.Adapter.tDrawRegion.tLocation.iX 
        >=  this.Adapter.tTargetRegion.tSize.iWidth) {
        this.Adapter.tDrawRegion.tLocation.iY 
            += ptPartialFrameBuffer->tRegion.tSize.iHeight;
        this.Adapter.tDrawRegion.tLocation.iX = 0;
        
        if (    this.Adapter.tDrawRegion.tLocation.iY 
            >=  this.Adapter.tTargetRegion.tSize.iHeight) {
            // finished
            this.Adapter.tDrawRegion.tLocation.iY = 0;

            return __arm_2d_helper_pfb_get_next_dirty_region(ptThis);
            
        }
    }

    return true;
}

__WEAK void __arm_2d_helper_perf_counter_start(int64_t *plTimestamp)
{
    assert(NULL != plTimestamp);
    *plTimestamp = arm_2d_helper_get_system_timestamp();
}

__WEAK int32_t __arm_2d_helper_perf_counter_stop(int64_t *plTimestamp)
{
    assert(NULL != plTimestamp);
    return (int32_t)(   (int64_t)arm_2d_helper_get_system_timestamp() 
                    -   (int64_t)*plTimestamp);
}


ARM_NONNULL(1)
void __arm_2d_helper_pfb_report_rendering_complete( arm_2d_helper_pfb_t *ptThis,
                                                    arm_2d_pfb_t *ptPFB)
{
    assert(NULL != ptThis);

    /* note: in fact, user can only pass either NULL or this.Adapter.ptFlushing
     *       to the ptPFB. This makes ptPFB useless. We only keep it for 
     *       backward compatible.
     */

    arm_irq_safe {
        if (NULL == ptPFB) {
            ptPFB = this.Adapter.ptFlushing;
            this.Adapter.ptFlushing = NULL;
        } else if (ptPFB == this.Adapter.ptFlushing) {
            this.Adapter.ptFlushing = NULL;
        }
    }

    if (NULL == ptPFB) {
        assert(false);  /* this should not happen */
        return ;
    }

    ptPFB->tTile.tRegion.tLocation = (arm_2d_location_t) {0,0};
    
    __arm_2d_helper_pfb_free(ptThis, ptPFB);
    
    arm_2d_helper_pfb_flush(ptThis);
}


ARM_NONNULL(1,3)
arm_2d_err_t arm_2d_helper_pfb_update_dependency(
                            arm_2d_helper_pfb_t *ptThis, 
                            uint_fast8_t chMask,
                            const arm_2d_helper_pfb_dependency_t *ptDependency)
{
    assert(NULL != ptThis);
    assert(NULL != ptDependency);
    
    arm_irq_safe {
        if (chMask & ARM_2D_PFB_DEPEND_ON_LOW_LEVEL_RENDERING) {
            this.tCFG.Dependency.evtOnLowLevelRendering 
                = ptDependency->evtOnLowLevelRendering;
        }

        if (chMask & ARM_2D_PFB_DEPEND_ON_DRAWING) {
            this.tCFG.Dependency.evtOnDrawing 
                = ptDependency->evtOnDrawing;
        }

        if (chMask & ARM_2D_PFB_DEPEND_ON_LOW_LEVEL_SYNC_UP) {
            this.tCFG.Dependency.evtOnLowLevelSyncUp 
                = ptDependency->evtOnLowLevelSyncUp;
        }
        
        if (chMask & ARM_2D_PFB_DEPEND_ON_EACH_FRAME_CPL) {
            this.tCFG.Dependency.evtOnEachFrameCPL
                = ptDependency->evtOnEachFrameCPL;
        }

        if (chMask & ARM_2D_PFB_DEPEND_ON_NAVIGATION) {
            this.tCFG.Dependency.Navigation = ptDependency->Navigation;
        }
    }
    
    return ARM_2D_ERR_NONE;
}


arm_fsm_rt_t arm_2d_helper_pfb_task(arm_2d_helper_pfb_t *ptThis, 
                                    arm_2d_region_list_item_t *ptDirtyRegions) 
{   
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.Dependency.evtOnDrawing.fnHandler);
    arm_fsm_rt_t tResult;

ARM_PT_BEGIN(this.Adapter.chPT)

    this.Statistics.nTotalCycle = 0;
    this.Statistics.nRenderingCycle = 0;
    this.Adapter.bIsNewFrame = true;
    __arm_2d_helper_perf_counter_start(&this.Statistics.lTimestamp); 
    do {
        /* begin of the drawing iteration, 
         * try to request the tile of frame buffer
         */
        do {
        
            /*! \note In deep embedded applications, a LCD usually is connected 
             *       via a serial interface to save pins, hence the bandwidth 
             *       is limited and the FPS is low due to the bandwidth.
             *       To overcome this issue, some partial-flushing schemes are 
             *       used, such as:
             *       - Dirty Region based partial-flushing
             *       - Flush the known and fixed small area that is updated 
             *         frequently based on the application scenarios. 
             *       
             *       It is worth emphasizing that as we are using partial 
             *       flushing scheme, which means for a given frame, we only 
             *       update those changed area(s) but not the complete frame,
             *       using the term frame per sec (FPS) might confuse people,
             *       hence, we decide to introduce a NEW term called update per
             *       sec (UPS) to avoid this confusion. It reflects what people
             *       feel when looking at the LCD but not necessarily means
             *       the rate that a complete frame is flushed into LCD.  
             *       
             *       In Arm-2D:
             *       - FPS is a sub-set of UPS. 
             *       - UPS forcus on how people feel and FPS is sticks to the 
             *         concept of (full) frame per sec.              
             */
        
            // request to draw the whole LCD
            this.Adapter.ptFrameBuffer = 
                __arm_2d_helper_pfb_drawing_iteration_begin( 
                                                        ptThis, 
                                                        ptDirtyRegions);
            if (NULL == this.Adapter.ptFrameBuffer) {
                if (NULL != this.tCFG.Dependency.evtOnLowLevelSyncUp.fnHandler){
                     // wait until lcd is ready
                    (*this.tCFG.Dependency.evtOnLowLevelSyncUp.fnHandler)(
                        this.tCFG.Dependency.evtOnLowLevelSyncUp.pTarget
                    );
                }
                continue;
            } else if (-1 == (intptr_t)this.Adapter.ptFrameBuffer) {
                /* display driver wants to end the drawing */
                goto label_pfb_task_rt_cpl;
            }
        } while(NULL == this.Adapter.ptFrameBuffer);

        /* LCD Latency includes the time of waiting for a PFB block */
        this.Statistics.nRenderingCycle 
            += __arm_2d_helper_perf_counter_stop(&this.Statistics.lTimestamp); 

    ARM_PT_ENTRY()
        
        __arm_2d_helper_perf_counter_start(&this.Statistics.lTimestamp); 
        /* draw all the gui elements on target frame buffer */
        tResult = this.tCFG.Dependency.evtOnDrawing.fnHandler(
                                        this.tCFG.Dependency.evtOnDrawing.pTarget,
                                        this.Adapter.ptFrameBuffer,
                                        this.Adapter.bIsNewFrame);
        // just in case some one forgot to do this...
        arm_2d_op_wait_async(NULL);

        this.Statistics.nTotalCycle += __arm_2d_helper_perf_counter_stop(&this.Statistics.lTimestamp);

        /* draw navigation layer */
        if (NULL != this.tCFG.Dependency.Navigation.evtOnDrawing.fnHandler) {
            if (arm_fsm_rt_cpl == tResult) {
    ARM_PT_ENTRY()
                __arm_2d_helper_perf_counter_start(&this.Statistics.lTimestamp); 

                tResult = this.tCFG.Dependency.Navigation.evtOnDrawing.fnHandler(
                                        this.tCFG.Dependency.Navigation.evtOnDrawing.pTarget,
                                        this.Adapter.ptFrameBuffer,
                                        this.Adapter.bIsNewFrame);
                arm_2d_op_wait_async(NULL);
                
                this.Statistics.nTotalCycle += __arm_2d_helper_perf_counter_stop(&this.Statistics.lTimestamp);
            }
        }

        if (arm_fsm_rt_on_going == tResult) {
    ARM_PT_GOTO_PREV_ENTRY(arm_fsm_rt_on_going)
        } else if (tResult < 0) {
            // error was reported
    ARM_PT_RETURN(tResult)
        } else if (arm_fsm_rt_wait_for_obj == tResult) {
    ARM_PT_REPORT_STATUS(tResult)
        } else { 
    ARM_PT_YIELD(arm_fsm_rt_on_going)
        }

        this.Adapter.bIsNewFrame = false;
        __arm_2d_helper_perf_counter_start(&this.Statistics.lTimestamp); 
    } while(__arm_2d_helper_pfb_drawing_iteration_end(ptThis));
    
    this.Statistics.nRenderingCycle += __arm_2d_helper_perf_counter_stop(&this.Statistics.lTimestamp);
    
label_pfb_task_rt_cpl:
ARM_PT_END()

    /* invoke the On Each Frame Complete Event */
    ARM_2D_INVOKE(  this.tCFG.Dependency.evtOnEachFrameCPL.fnHandler,
                    this.tCFG.Dependency.evtOnEachFrameCPL.pTarget);

    return arm_fsm_rt_cpl;
}


