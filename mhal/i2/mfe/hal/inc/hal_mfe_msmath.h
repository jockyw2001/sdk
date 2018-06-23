#ifndef _MSMATH_H_
#define _MSMATH_H_

#include "hal_mfe_def.h"

// ----------------------------------------------------------------------------
// fixed-point implementation Q16.16
// ----------------------------------------------------------------------------

#define MATH_FUNC_SCALING_BITS (16)
// Input and output of below math functions are with MATH_FUNC_SCALING_BITS scaling


uint32 fSqrt(uint32 x);
uint64 fPow64(uint32 base, int exponment);
uint32 fExp(int exponment);
int fLog(uint32 x);

#endif // _MSMATH_H_
