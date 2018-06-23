#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "hal_mfe_def.h"

// MFE HW limit
//#define MIN_QP                      5
//#define MAX_QP                      48

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

#define RC_MODEL_HISTORY 12


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

// Qp to Qstep tranformation
//int Qstep2QP(double Qstep);
//double QP2Qstep(int QP);

int Qstep_12bitSCALED2QP(int Qstep_SCALED);
int QP2Qstep_SCALED(int QP);

#endif
