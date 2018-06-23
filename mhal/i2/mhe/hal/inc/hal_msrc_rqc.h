/*
 * hal_msrc_rqc.h
 *
 *  Created on: Nov 13, 2017
 *      Author: derek.lee
 */

#ifndef DRIVER_MHE_HAL_INC_HAL_MSRC_RQC_H_
#define DRIVER_MHE_HAL_INC_HAL_MSRC_RQC_H_

#include "hal_mhe_global.h"

/*****************************************************************************/
// Constant values
/*****************************************************************************/

#define MIN_PIC_BIT 400

// NEED_CHANGE_BY_PREC_SCALE_BITS
// Pre-multipled with PREC_SCALE_FACTOR to change type from double to int
//#define ALPHA     6.7542
//#define BETA1     1.2517
//#define BETA2     1.7860
#define ALPHA_SCALED  (442643)
#define BETA1_SCALED  (82031)
#define BETA2_SCALED  (117047)
// For P-frame
//#define ALPHA_P     3.2003
//#define BETA_P      -1.367
#define ALPHA_P_SCALED   (209735)
#define BETA_P_SCALED    (-89588)
#define MIN_EST_LAMBA_SCALED (6554)

// NEED_CHANGE_BY_PREC_SCALE_BITS
//    Should check required bitcount of QpToLambdaScaled() maximal output;
//    and redesign NUM & SHIFT if total bitcount is over 32.
// pow( 2.0, g_dRcPicClipRange/3.0 )
#define LAMBDA_MAX_CLIP_NUM (161)
#define LAMBDA_MAX_CLIP_SHIFT (6)
// pow( 2.0, -g_dRcPicClipRange/3.0 )
#define LAMBDA_MIN_CLIP_NUM (203)
#define LAMBDA_MIN_CLIP_SHIFT (9)

// Valid range : [0.1, 1.0]
//double g_dPILambdaRatio = 0.6;
const uint32 g_uiPILambdaRatioNum = 6;      //Conard suggest
const uint32 g_uiPILambdaRatioDenorm = 10;  //Conard suggest

// Limitation of Lambda-Bpp model parameters
// NEED_CHANGE_BY_PREC_SCALE_BITS
const int g_iRCAlphaMinValue_SCALED = 3277;
const int g_iRCAlphaMaxValue_SCALED = 32768000;
const int g_iRCBetaMinValue_SCALED  = -195950;
const int g_iRCBetaMaxValue_SCALED  = -6554;

const int g_iRCIntraAlphaMinValue_SCALED = 3277;
const int g_iRCIntraAlphaMaxValue_SCALED = 32768000;
const int g_iRCIntraBetaMinValue_SCALED  = 21954;
const int g_iRCIntraBetaMaxValue_SCALED  = 4194304;

static const int g_iRCWeightHistoryLambda_MUL10 = 9;
static const int g_iGOPCompenRatioLimit_MUL100 = 10;

//const double g_WeightOrgPicBit = 0.5;
//const uint32 g_uiWeightOrgPicBit_SCALED = (1 << (PREC_SCALE_BITS - 1));


//const double g_dRcPicClipRange = 4.0;

/*****************************************************************************/
// Global Enum, struct
/*****************************************************************************/

// for PMBR
typedef struct
{
    int m_bPercPicEn;

    // From HW output
    int m_nTcHist[1 << LOG2_HIST_SIZE];

    //int m_nPcCntHist[1<<LOG2_HIST_SIZE];

    uint32 *m_nLUTTarget;
    uint32 *m_nLUTQp;
    //double *m_dLUTLambdas;
    uint32 *m_auiLUTLambdas_SCALED;
    int *m_nLUTEntryHist;
    int m_nTcAccum;
    int m_nPcCntAccum;
    int m_nOutWeightAccum;
    int m_nSecStageWghtAccum;

    unsigned char* m_nPercMBInMap;
    unsigned char* m_nPercMBOutMap;
    unsigned char* m_nPercMBMapBuff[HW_CORE_NUM * 2];
} PerceptualMbr;

// for MsRC
typedef struct
{
//  double dAlpha;
//  double dBeta;
    int Alpha_SCALED;
    int Beta_SCALED;
} rc_model_para;

typedef struct
{
    int64 iGopBitLeft;
    int iGopFrameLeft;
    int iPicAllocBit;
} ms_rc_gop;

typedef struct
{
    // Input: frame type
    /*SliceType*/int picType;
    int isFirstPpic;

    // Output: target picture QP, bitcount (and derived lambda from lambda-bpp model)
    uint32 uiPicQp;
    int iTargetBit;
    //double dPicLambda;
    uint32 uiPicLambda_SCALED;

    // Last-encoded frame : average QP and average lambda
    //   updated after encoding one frame (through LUT histogram)
    uint32 auiLevelQp[SLICE_TYPE_NUM];
    //double adLevelLambda[SLICE_TYPE_NUM];
    uint32 auiLevelLambda_SCALED[SLICE_TYPE_NUM];
} ms_rc_pic;

typedef struct
{
    //uint32 auiQps   [MBR_LUT_SIZE];
    //double adLambdas[MBR_LUT_SIZE];
    //uint32 auiLambdas_SCALED[MBR_LUT_SIZE];
    //uint32 auiBits  [MBR_LUT_SIZE];
    uint32 *auiQps;              //m_nLUTQp
    //double adLambdas[MBR_LUT_SIZE];
    uint32 *auiLambdas_SCALED;   //m_auiLUTLambdas_SCALED
    uint32 *auiBits;
    int  *aiIdcHist;             //m_nLUTEntryHist
    uint32 uiEntryNum; // number of available table entry
    //double dEntryDeltaQp;
} mbr_lut;

typedef struct
{
// Video seqeuence parameters
    uint32 uiTargetBitrate;
    uint32 uiFps;
    uint32 uiGopSize;
    uint32 uiPicPixelNum;
    uint32 uiInitialQp;
    uint32 uiRcIPicMaxQp;
    uint32 uiRcIPicMinQp;
    uint32 uiRcPPicMaxQp;
    uint32 uiRcPPicMinQp;
    //int bConstQp;
// Derived
    int iPicAvgBit;
    uint32 uiPicCtuNum;

// Internal state variables
    uint32 uiFrameCnt;
    int iStreamBitErr; // Accumulated (actual_bits - avg_bits)

    int iLambdaMinClipMul;
    int iLambdaMaxClipMul;
    uint32 uiMinIPicBit;
    uint32 uiMaxIPicBit;
    uint32 uiMinPicBit;
    uint32 uiMaxPicBit;

// Model variables
// Fixed
    uint32 uiStepAlpha_SCALED;//double dStepAlpha;
    uint32 uiStepBeta_SCALED;//double dStepBeta;
// Runtime updated
    int LastAlpha_SCALED;//double dLastAlpha;
    int LastBeta_SCALED;//double dLastBeta;
    int LastAlphaIncr_SCALED;//double dLastAlphaIncr;
    int LastBetaIncr_SCALED;//double dLastBetaIncr;
    int HistoryPLambda_SCALED;//double dHitoryPLambda;
    rc_model_para atModelPara[SLICE_TYPE_NUM];

    uint32 uiIntraMdlCplx;

    ms_rc_gop tRcGop;
    ms_rc_pic tRcPic;
    mbr_lut tMbrLut;
    PerceptualMbr tPmbr;
} ms_rc_top;


#endif /* DRIVER_MHE_HAL_INC_HAL_MSRC_RQC_H_ */
