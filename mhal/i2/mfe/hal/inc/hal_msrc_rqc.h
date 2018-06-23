/*
 * hal_msrc_rqc.h
 *
 *  Created on: Nov 13, 2017
 *      Author: derek.lee
 */

#ifndef DRIVER_MFE_HAL_INC_HAL_MSRC_RQC_H_
#define DRIVER_MFE_HAL_INC_HAL_MSRC_RQC_H_

#include "hal_mfe_global.h"

/*****************************************************************************/
// Constant values
/*****************************************************************************/

#define MIN_PIC_BIT 200

const int g_iGOPCompenRatioLimit_MUL100 = 10;
//const double g_WeightOrgPicBit = 0.5;
const uint32 g_uiWeightOrgPicBit_SCALED = (1<<(PREC_SCALE_BITS-1));

const int g_iRCIoverPQpDelta = -3;
const uint32 g_uiRcIMaxQPChange = 3;

/*****************************************************************************/
// Global Enum, struct
/*****************************************************************************/

// for PMBR
typedef struct {
  int m_bPercPicEn;

  // From HW output
  int m_nTcHist[1<<LOG2_HIST_SIZE];

  //int m_nPcCntHist[1<<LOG2_HIST_SIZE];
  uint32 *m_nLUTTarget;
  uint32 *m_nLUTQp;
  uint32 *m_auiLUTLambdas_SCALED;
  int *m_nLUTEntryHist;
  int m_nTcAccum;
  int m_nPcCntAccum;
  int m_nOutWeightAccum;
  int m_nSecStageWghtAccum;

  unsigned char* m_nPercMBInMap;
  unsigned char* m_nPercMBOutMap;
  unsigned char* m_nPercMBMapBuff[HW_CORE_NUM*2];
} PerceptualMbr;


typedef struct {
  int64 iGopBitLeft;
  int iGopFrameLeft;
  int iPicAllocBit;

  int iPPicQpSum;
  uint32 uiGopIdx;
} ms_rc_gop;

typedef struct {
  // Input: frame type
  /*SliceType*/int picType;
  int isFirstPpic;

  // Output: target picture QP, bitcount (and derived lambda from lambda-bpp model)
  uint32 uiPicQp;
  int iTargetBit;

  // Last-encoded frame : average QP and average lambda
  //   updated after encoding one frame (through LUT histogram)
  uint32 auiLevelQp[SLICE_TYPE_NUM];
} ms_rc_pic;

typedef struct {
   uint32 *auiQps;
   uint32 *auiBits;
   int  *aiIdcHist;
   uint32 uiEntryNum; // number of available table entry
} mbr_lut;

typedef struct {
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
// Derived
  int iPicAvgBit;
  uint32 uiPicCtuNum;
   uint32 uiPAverageQp;
// Internal state variables
  uint32 uiFrameCnt;
  int iStreamBitErr; // Accumulated (actual_bits - avg_bits)
  uint32 uiQPLastGOP;
  uint32 uiMinIPicBit;
  uint32 uiMaxIPicBit;
  uint32 uiMinPicBit;
  uint32 uiMaxPicBit;

// Model variables
  int iSampleQs_SCALED[RC_MODEL_HISTORY];
  int iSampleR[RC_MODEL_HISTORY];
  int64 X1;
  int64 X2;


  uint32 uiIntraMdlCplx;

  ms_rc_gop tRcGop;
  ms_rc_pic tRcPic;
  mbr_lut tMbrLut;
  PerceptualMbr tPmbr;
} ms_rc_top;


#endif /* DRIVER_MFE_HAL_INC_HAL_MSRC_RQC_H_ */
