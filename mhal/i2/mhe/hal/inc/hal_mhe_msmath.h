#ifndef _MSMATH_H_
#define _MSMATH_H_

#include "hal_mhe_def.h"

// ----------------------------------------------------------------------------
// fixed-point implementation Q16.16
// ----------------------------------------------------------------------------

#define MATH_FUNC_SCALING_BITS (16)
// Input and output of below math functions are with MATH_FUNC_SCALING_BITS scaling

uint32 MhefSqrt(uint32 x);
uint64 MhefPow64(uint32 base, int exponment);
uint32 MhefExp(int exponment);
int MhefLog(uint32 x);

#endif // _MSMATH_H_
