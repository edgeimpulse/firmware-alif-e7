#ifndef __BASE_DEF__H__
#define __BASE_DEF__H__

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

#if 0
#define    DEBUG_PRINTF(f,...)    ei_printf(f,##__VA_ARGS__)
#else
#define    DEBUG_PRINTF(f,...)
#endif
#define HW_REG_WORD(base,offset) *((volatile unsigned int *) (base + offset))

#endif  //!__BASE_DEF__H__