/*
 * mhal_ut_wrapper.h
 *
 *  Created on: Aug 24, 2017
 *      Author: derek.lee
 */

#ifndef INCLUDE_MHAL_UT_WRAPPER_H_
#define INCLUDE_MHAL_UT_WRAPPER_H_

#include "mhal_venc.h"

typedef struct
{
  MHAL_VENC_Idx_e type;
  void* param;
} VencParamUT;

#endif /* INCLUDE_MHAL_UT_WRAPPER_H_ */
