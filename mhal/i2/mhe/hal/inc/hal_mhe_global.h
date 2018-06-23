#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "hal_mhe_def.h"

// HEVC spec
//#define MIN_QP                      0
//#define MAX_QP                      51

// These are for kernel mode double -> int precision
// Variable with postfix _SCALED are with PREC_SCALE_FACTOR scaling
// If PREC_SCALE_BITS changed, codes marked with NEED_CHANGE_BY_PREC_SCALE_BITS must be checked.
#define PREC_SCALE_BITS  (16)
// Dervied
#define PREC_SCALE_FACTOR  (1<<PREC_SCALE_BITS)

// Histogram for perceptual RC tuning
#define LOG2_HIST_SIZE 5
// minimun bin number threshold for perceptual MBR enable
#define PMBR_BIN_NUM_THR 5

#define MBR_LUT_SIZE 15
// Refer to SliceType
#define SLICE_TYPE_NUM 3

#define MAX_LAMBDA_INT_VAL ((1<<15)-1)
#define MAX_CTB_BIT (8192-1)

#define HW_CORE_NUM 2

// constant values of Lambda-Qp transfer function
#define QP_LAMBMA_FORMULA_ADD  898643 //((int)(13.7122*PREC_SCALE_FACTOR))
#define QP_LAMBMA_FORMULA_MUL  275284 //((int)(4.2005*PREC_SCALE_FACTOR))


/*****************************************************************************/
// HW simulated io variables
/*****************************************************************************/


/*****************************************************************************/
// Global Enum, struct
/*****************************************************************************/

// supported slice type
enum SliceType
{
    I_SLICE,
    P_SLICE,
    B_SLICE
};

// HM-defined fixed lambda-Qp curve
//int LambdaToQp(double lambda);
//double QpToLambda(int Qp);
uint32 LambdaScaledToQp(const uint32 lambda_SCALED);
uint32 QpToLambdaScaled(const int Qp);

#endif
