
#include "hal_mhe_def.h"
#include "hal_mhe_reg.h"
#include "hal_mhe_pmbr.h"

#include "hal_mhe_global.h"
#include "hal_mhe_msmath.h"

#define MIN_QP                      0
#define MAX_QP                      51

//#define PMBR_CNT_PENCENT_THR 0.5
#define PMBR_CNT_PENCENT_THR_NUM   1
#define PMBR_CNT_PENCENT_THR_DENORM 2

// minimun bin number threshold for perceptual MBR enable
//#define PMBR_BIN_THR 5

#define MAX_VALUE   0x7FFFFFFF

//const int g_iQpRangeInPic = 8;

static unsigned int MBR_lut_Size = 240;

// chroma Qp offset parameters
//static int g_iPPSCbQpOffset = 0;
//static int g_iPPSCrQpOffset = 0;
static int g_iSliceQpDeltaCb = 0;
static int g_iSliceQpDeltaCr = 0;

// CTB min, max QP
static uint32 g_uiRcCtuMinQp = MIN_QP;
static uint32 g_uiRcCtuMaxQp = MAX_QP;

// Refer to HEVC spec: Derivation process for quantization parameters
static const uint32 g_auiChromaScale[58] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 29, 30, 31, 32,
    33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51
};

static inline int imin(int a, int b)
{
    return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
    return ((a) > (b)) ? (a) : (b);
}

static inline int iabs(int a)
{
    return ((a) < 0) ? -(a) : (a);
}

static inline int iClip3(int low, int high, int x)
{
    x = imax(x, low);
    x = imin(x, high);
    return x;
}

static inline uint32 uimin(uint32 a, uint32 b)
{
    return ((a) < (b)) ? (a) : (b);
}

static inline uint32 uimax(uint32 a, uint32 b)
{
    return ((a) > (b)) ? (a) : (b);
}

static inline uint32 uiClip3(uint32 low, uint32 high, uint32 x)
{
    x = uimax(x, low);
    x = uimin(x, high);
    return x;
}

static int _pmbr_checkHistValid(int *pBin, int TotalBinNum, int binNumThr, int binCntThr)
{
    int nonZeroBinNum = 0, idx;
    for(idx = 0; idx < TotalBinNum; idx++)
    {
        if(pBin[idx] != 0)
        {
            nonZeroBinNum++;
        }
        if(pBin[idx] >= binCntThr)
        {
            return 0;
        }
    }
    if(nonZeroBinNum < binNumThr)
    {
        return 0;
    }
    return 1;
}

static void _perc_mbr_set_OneLutEntry(pmbr_frmAttr* mbr, uint32 uiEntryIdx, uint32 Qp, uint32 uiLambda_SCALED, uint32 uiBppMult1024)
{
    uint32 Bit;

    Qp = uiClip3(PMBR_CTU_MINQP, PMBR_CTU_MAXQP, Qp);
    mbr->m_nLUTQp[uiEntryIdx] = Qp;


    //Lambda = dClip3(QpToLambda(g_iRcCtuMinQp-0.5), QpToLambda(g_iRcCtuMaxQp+0.5), Lambda);
    //Lambda = dClip3(0.1, (double)MAX_LAMBDA_INT_VAL, Lambda);  // This is HW limitation
    {
        uint32 uiMinLambda_SCALED = QpToLambdaScaled(g_uiRcCtuMinQp);
        uint32 uiMaxLambda_SCALED = QpToLambdaScaled(g_uiRcCtuMaxQp);
        uiLambda_SCALED = uiClip3(uiMinLambda_SCALED, uiMaxLambda_SCALED, uiLambda_SCALED);
        uiLambda_SCALED = uiClip3(PREC_SCALE_FACTOR / 10, MAX_LAMBDA_INT_VAL * PREC_SCALE_FACTOR, uiLambda_SCALED);  // This is HW limitation
    }
    mbr->m_auiLUTLambdas_SCALED[uiEntryIdx] = uiLambda_SCALED;


    Bit = iClip3(1, MAX_CTB_BIT, uiBppMult1024);
    mbr->m_nLUTTarget[uiEntryIdx] = Bit;
}

uint32 _LambdaScaledToQp(const uint32 lambda_SCALED)
{
    int64 RoundAdd = 1 << (PREC_SCALE_BITS - 1);
    int64 qp_lambda_add = QP_LAMBMA_FORMULA_ADD;
    int64 forula_mul_log_term = (((int64) QP_LAMBMA_FORMULA_MUL) * MhefLog(lambda_SCALED)) >> PREC_SCALE_BITS;
    return (uint32)((forula_mul_log_term + qp_lambda_add + RoundAdd) >> PREC_SCALE_BITS);
}

static void _set_hw_lut(pmbr_frmAttr* ptPmbr, int64* table)
{
    uint32 uiEntryIdx;

    uint32 uiQpY, uiBits;
    uint32 uiLambda_SCALED;
    uint32 uiMotionLambda_SCALED;

    uint32 uiYQpRem, uiYQpPer;
    int CbQpOffset, CrQpOffset;
    uint32 uiQpCb, uiCbQpRem, uiCbQpPer;
    uint32 uiQpCr, uiCrQpRem, uiCrQpPer;

    uint64 ui64LutHigh, ui64LutLow;
    uint64 aui64LUTMemory[MBR_LUT_SIZE * 2];

    memset(aui64LUTMemory, 0, MBR_lut_Size);

    for(uiEntryIdx = 0; uiEntryIdx < MBR_LUT_SIZE; uiEntryIdx++)
    {
        uiQpY = ptPmbr->m_nLUTQp[uiEntryIdx];
        uiBits = ptPmbr->m_nLUTTarget[uiEntryIdx];

        uiLambda_SCALED = ptPmbr->m_auiLUTLambdas_SCALED[uiEntryIdx];

        uiMotionLambda_SCALED = MhefSqrt(uiLambda_SCALED);

        //CamOsPrintf("[PMBR LUT] %d: uiQpY = %d\n", uiEntryIdx, uiQpY);
        if(uiEntryIdx == 7)
            MHE_MSG(MHE_MSG_DEBUG, "%d: uiMotionLambda_SCALED, uiLambda_SCALED = (%d %d)\n", uiEntryIdx, uiMotionLambda_SCALED, uiLambda_SCALED);

        // Prepare for LUT filling: lambda should be 10-bit-scaled, motionLambda should be 16-bit-scaled
        uiLambda_SCALED >>= (MATH_FUNC_SCALING_BITS - 10);
        //uiMotionLambda_SCALED >>= (MATH_FUNC_SCALING_BITS-16);

        uiYQpRem = uiQpY % 6;
        uiYQpPer = uiQpY / 6;

        //CbQpOffset = g_iPPSCbQpOffset + g_iSliceQpDeltaCb;
        CbQpOffset = ptPmbr->i_CbQpOffset + g_iSliceQpDeltaCb;
        uiQpCb = g_auiChromaScale[uiClip3(0, 57, (int) uiQpY + CbQpOffset)];

        uiCbQpRem = uiQpCb % 6;
        uiCbQpPer = uiQpCb / 6;

        //CrQpOffset = g_iPPSCrQpOffset + g_iSliceQpDeltaCr;
        CrQpOffset = ptPmbr->i_CrQpOffset + g_iSliceQpDeltaCr;
        if(CrQpOffset == CbQpOffset)
        {
            uiQpCr = uiQpCb;
            uiCrQpRem = uiCbQpRem;
            uiCrQpPer = uiCbQpPer;
        }
        else
        {
            uiQpCr = g_auiChromaScale[uiClip3(0, 57, (int) uiQpY + CrQpOffset)];
            uiCrQpRem = uiQpCr % 6;
            uiCrQpPer = uiQpCr / 6;
        }

        //if(uiEntryIdx == 7)
        //    CamOsPrintf("[%s %d] Cb/Cr QP Offset: %d / %d \n",__FUNCTION__,__LINE__,ptPmbr->i_CbQpOffset, ptPmbr->i_CrQpOffset);

        // LUT entry : data packing
        ui64LutHigh = (uint64)(uiYQpPer | (uiYQpRem << 4) | (uiCbQpPer << 7) | (uiCbQpRem << 11) | (uiCrQpPer << 14) | (uiCrQpRem << 18) | (uiQpY << 21));
        // MotionLambda :  8 bit integer, 16 bit fractional
        // lambda       : 15 bit integer, 10 bit fractional
        ui64LutLow = ((uint64) uiBits) | (((uint64) uiMotionLambda_SCALED) << 13) | (((uint64) uiLambda_SCALED) << 37);

        if(uiEntryIdx == 7)
            MHE_MSG(MHE_MSG_DEBUG, "%d: (ui64LutHigh, ui64LutLow) %llu  %llu %lu %lu %lu\n", uiEntryIdx, ui64LutHigh, ui64LutLow, uiBits, uiMotionLambda_SCALED, uiLambda_SCALED);

        // Fill LUT memory
        aui64LUTMemory[uiEntryIdx << 1] = ui64LutLow;

        aui64LUTMemory[(uiEntryIdx << 1) + 1] = ui64LutHigh;
    }

    if(table)
        memcpy(table, aui64LUTMemory, MBR_lut_Size);

    //CamOsHexdump((char *)aui64LUTMemory,MBR_lut_Size);
}

// Generate from C-Model (search "g_DefaultLambdaByQp")
// Q16.16
//I frame qp to lambda table
static const uint32 g_DefaultLambdaByQp[52] =
{
    2334,   2941,   3706,   4669,   5883,
    7412,   9338,   11766,  14824,  18677,
    23532,  29649,  37355,  47065,  59298,
    74711,  94130,  118596, 149422, 188260,
    237192, 298844, 376520, 474385, 597688,
    753040, 948771, 1195376,    1506080,    1897542,
    2390753,    3012160,    3795084,    4781506,    6024320,
    7590168,    9563013,    12048641,   15180337,   19126026,
    24097283,   30360674,   38252052,   48194566,   60721348,
    76504104,   96389132,   121442696,  153008209,  192778264,
    242885393,  306016419,
};

//P frame qp to lambda table
static const uint32 g_PFrameLambdaByQp[52] =
{
    1893,       2386,         3006,       3787,       4772,
    6013,       7575,         9545,       12026,     15151,
    19090,     24052,       30303,     38180,     48104,
    60607,     76360,      96208,     121215,   152721,
    192417,   242430,     305443,   384834,   484861,
    610887,   769669,     969723,   1221774,       1539339,
    1939446, 2443549,   3078678, 3878892,       4887098,
    6157357, 7757784,   9774196, 12314715, 15515569,
    19548392, 24629431, 31031138, 39096784, 49258862,
    62062277, 78193569, 98517724, 124124554, 156387139,
    197035448, 248249109,
};

static void _pmbr_FillLUT(pmbr_ops* pmbr, int center_qp, int center_lambda_SCALED)
{
    mhe_pmbr* pmbr_ex   = (mhe_pmbr*)pmbr;
    pmbr_frmAttr* mbr   = &(pmbr_ex->frmAttr);

    int entry, bin;
    int iLutSize = PMBR_LUT_SIZE;
    int iHistSize = 1 << PMBR_LOG2_HIST_SIZE;
    int histCdf[1 << PMBR_LOG2_HIST_SIZE];
    unsigned char* tempPtr;
    int tempAddr;
    //int bSingleEntry;
    int iTableSize;
    int centerIdx;
    const uint32 uiLutSize = PMBR_LUT_SIZE;
    uint32 uiPicCtuNum = pmbr_ex->gblAttr.i_ctuW * pmbr_ex->gblAttr.i_ctuH;
    uint32 auiTargetTc[1 << LOG2_HIST_SIZE];
    int64 remainder;
    uint32 uiHighestQp;
    uint32 uiEntryQp;
    uint32 uiNormBinNum_SCALED;
    uint32 uiRoundVal;
    int iBinInteval;
    int binSel = 0, targetCnt, dist, histIncr, iTargetTc;
    uint64 uiremainder;
    const int g_iQpRangeInPic = 8;
    uint32 uiQpRangeInTable;
    uint64 uiQpPrecision_SCALE;
    uint32 uiLambdaScale;
    uint32 uiBaseLambda;
    uint32 uiEntryLambda_SCALE;
    uint32 uiEntryIdx;
    int isValid;
    int cntAccum = 0, binCnt;
    int entry_lambda_SCALED;

    MHE_MSG(MHE_MSG_DEBUG, "center_qp = %d\n", center_qp);
    MHE_MSG(MHE_MSG_DEBUG, "center_lambda_SCALED = %d\n", center_lambda_SCALED);
    MHE_MSG(MHE_MSG_DEBUG, "uiPicCtuNum = %d\n", uiPicCtuNum);

    if(pmbr->i_LutTyp == 1)
    {
        iTableSize = ((MBR_LUT_SIZE >> 1) << 1) + 1;
        centerIdx = iTableSize >> 1;
        memset(&mbr->m_nLUTQp, 0, sizeof(int) * LUT_SIZE);
        memset(&mbr->m_auiLUTLambdas_SCALED, 0, sizeof(int) * LUT_SIZE);
        memset(&mbr->m_nLUTTarget, 0, sizeof(int) * LUT_SIZE);

        mbr->m_nLUTQp[centerIdx] = center_qp;
        mbr->m_auiLUTLambdas_SCALED[centerIdx] = center_lambda_SCALED;
        mbr->m_nLUTTarget[centerIdx] = 0;
        memset(&mbr->m_nLUTEntryHist, 0, sizeof(int) * LUT_SIZE);
        mbr->m_nLUTEntryHist[centerIdx] = uiPicCtuNum;        //total CTB number
        return;
    }
    else if(pmbr->i_LutTyp == 2)
    {
        mbr->m_bPercPicEn = 0;

        uiHighestQp = center_qp + (uiLutSize >> 1);

        //assert(center_qp > (uiLutSize >> 1),"center_qp > (uiLutSize >> 1)");

        for(entry = 0; entry < iLutSize; entry++)
        {
            if(pmbr->i_LutRoiQp[entry] != 0)
            {
                uiEntryQp = pmbr->i_LutRoiQp[entry];

                if(pmbr->i_PicTyp == 0)
                    entry_lambda_SCALED = g_DefaultLambdaByQp[uiEntryQp];
                else
                    entry_lambda_SCALED = g_PFrameLambdaByQp[uiEntryQp];
            }
            else
            {
                //uiEntryQp = uiHighestQp - entry;
                uiEntryQp = center_qp;
                entry_lambda_SCALED = center_lambda_SCALED;
            }

            _perc_mbr_set_OneLutEntry(mbr, entry, uiEntryQp, entry_lambda_SCALED, (uiLutSize - entry + 1));
        }

        return;
    }


    // Picture level Perceptual MBR decision
    if(pmbr->i_FrmCnt != 0 && pmbr_ex->gblAttr.b_PmbrEnable != 0)
    {
        isValid = _pmbr_checkHistValid(mbr->m_nTcHist, (1 << PMBR_LOG2_HIST_SIZE), PMBR_BIN_NUM_THR, uiPicCtuNum * PMBR_CNT_PENCENT_THR_NUM / PMBR_CNT_PENCENT_THR_DENORM);
        mbr->m_bPercPicEn = isValid;
    }
    else
    {
        mbr->m_bPercPicEn = 0;
    }

    //CamOsPrintf( "mbr->m_bPercPicEn = %d, center_qp = %d\n", mbr->m_bPercPicEn, center_qp);

    MHE_MSG(MHE_MSG_DEBUG, "mbr->m_bPercPicEn = %d\n", mbr->m_bPercPicEn);

    // Filling LUT for Perceptual MBR disable case
    if(!mbr->m_bPercPicEn)
    {
        uiHighestQp = center_qp + (uiLutSize >> 1);

        //assert(center_qp > (uiLutSize >> 1),"center_qp > (uiLutSize >> 1)");
        for(entry = 0; entry < iLutSize; entry++)
        {
            uiEntryQp = uiHighestQp - entry;
            uiEntryQp = ((int32)uiEntryQp < 0) ? 0 : uiEntryQp;
            _perc_mbr_set_OneLutEntry(mbr, entry, uiEntryQp, center_lambda_SCALED, (uiLutSize - entry + 1));
        }

        return;   //c-model
    }

    // Histogram equalized base LUT filling
    {
        // Accumulate histogram CDF

        for(bin = 0; bin < iHistSize; bin++)
        {
            binCnt = mbr->m_nTcHist[bin];
            cntAccum += binCnt;
            histCdf[bin] = cntAccum;
        }
    }

    {
        // Text complexity range determination for bin equalization
        //double dNormBinNum = pmbr_ex->gblAttr.i_ctuW * pmbr_ex->gblAttr.i_ctuH / ((double)iClip3(PMBR_BIN_THR, iLutSize, nonZeroBinNum));
        uiNormBinNum_SCALED = (uiPicCtuNum << PREC_SCALE_BITS) / uiLutSize;
        uiRoundVal = 1 << (PREC_SCALE_BITS - 1);
        iBinInteval = 1 << (8 - PMBR_LOG2_HIST_SIZE);

        for(entry = 0; entry < iLutSize; entry++)
        {
            targetCnt = (int)((uiNormBinNum_SCALED * (entry + 1) + uiRoundVal) >> PREC_SCALE_BITS);
            dist = MAX_VALUE;

            for(bin = binSel; bin < iHistSize; bin++)
            {
                if(histCdf[bin] > targetCnt)
                {
                    break;
                }
                if(iabs(histCdf[bin] - targetCnt) < dist)
                {
                    dist = iabs(histCdf[bin] - targetCnt);
                    binSel = bin;
                }
            }

            histIncr = histCdf[binSel + 1] - histCdf[binSel];
            iTargetTc = (iBinInteval >> 1) + (binSel << 3);

            if(histIncr != 0)
            {
                //iTargetTc += ((int64)(targetCnt - histCdf[binSel])*iBinInteval)/histIncr; //c-model
                iTargetTc += (int)CamOsMathDivS64(((int64)(targetCnt - histCdf[binSel]) * iBinInteval), histIncr, &remainder);
            }
            else
            {
                iTargetTc += iBinInteval;
            }

            auiTargetTc[entry] = iClip3(0, 255, iTargetTc);
        }
    }

    // Set LUT table
    {

        //uint32 uiPicQp = _LambdaScaledToQp(center_lambda_SCALED);
        uiQpRangeInTable = (g_iQpRangeInPic << 1) - 1;
        uiQpPrecision_SCALE = CamOsMathDivU64(((uint64) uiQpRangeInTable << (PREC_SCALE_BITS << 1)), uiLutSize, &uiremainder);
        uiLambdaScale = MhefExp((int)(CamOsMathDivU64(uiQpPrecision_SCALE, QP_LAMBMA_FORMULA_MUL, &uiremainder)));
        uiBaseLambda = (uint32)(CamOsMathDivU64(((uint64)center_lambda_SCALED << PREC_SCALE_BITS), MhefPow64(uiLambdaScale, ((uiLutSize - 1) >> 1) << PREC_SCALE_BITS), &uiremainder));

        for(entry = 0; entry < uiLutSize; entry++)
        {
            uiEntryLambda_SCALE = uiBaseLambda;
            uiEntryQp = _LambdaScaledToQp(uiEntryLambda_SCALE);
            uiEntryIdx = uiLutSize - entry - 1;
            _perc_mbr_set_OneLutEntry(mbr, uiEntryIdx, uiEntryQp, uiEntryLambda_SCALE, auiTargetTc[entry]);
            uiBaseLambda = (uint32)(((uint64) uiLambdaScale * uiBaseLambda) >> PREC_SCALE_BITS);
        }
    }

    // Switch the In/Out Map
    tempPtr = mbr->m_nPercMBInMapVirt;
    mbr->m_nPercMBInMapVirt = mbr->m_nPercMBOutMapVirt;
    mbr->m_nPercMBOutMapVirt = tempPtr;

    tempAddr = mbr->m_nPercMBInMapPhy;
    mbr->m_nPercMBInMapPhy = mbr->m_nPercMBOutMapPhy;
    mbr->m_nPercMBOutMapPhy = tempAddr;


    return;
}

static int _SeqSync(pmbr_ops* pmbr, mhve_job* mjob);
static int _SeqDone(pmbr_ops* pmbr);
static int _SeqConf(pmbr_ops* pmbr);
static int _SetConf(pmbr_ops* pmbr, pmbr_cfg* pmbrcfg);
static int _GetConf(pmbr_ops* pmbr, pmbr_cfg* pmbrcfg);
static int _EncConf(pmbr_ops* pmbr, mhve_job* mjob);
static int _EncDone(pmbr_ops* pmbr, mhve_job* mjob);

static void _PmbrFree(pmbr_ops* pmbr)
{
    MEM_FREE(pmbr);
}

//------------------------------------------------------------------------------
//  Function    : MhePmbrAllocate
//  Description :
//------------------------------------------------------------------------------
void* MhePmbrAllocate(void)
{
    pmbr_ops*   pmbr_op = NULL;
    mhe_pmbr*   pmbr_ex;

    if(!(pmbr_op = MEM_ALLC(sizeof(mhe_pmbr))))
        return NULL;

    /* Link member function */
    pmbr_op->release  = _PmbrFree;
    pmbr_op->seq_sync = _SeqSync;
    pmbr_op->seq_done = _SeqDone;
    pmbr_op->seq_conf = _SeqConf;
    pmbr_op->set_conf = _SetConf;
    pmbr_op->get_conf = _GetConf;
    pmbr_op->enc_conf = _EncConf;
    pmbr_op->enc_done = _EncDone;

    /* Initialize Global Attribute */
    pmbr_ex = (mhe_pmbr*)pmbr_op;
    pmbr_ex->gblAttr.i_LutEntryClipRange = 15;  //reg_mbr_lut_entry_clip_range
    pmbr_ex->gblAttr.i_TextWeightType = 2;      //reg_mbr_tc_text_weight_type
    pmbr_ex->gblAttr.i_SkinLvShift = 8;         //reg_mbr_pw_pc_level_shift
    pmbr_ex->gblAttr.i_WeightOffset = 0;        //reg_mbr_sm_bit_weight_offset
    pmbr_ex->gblAttr.i_TcOffset = 0;            //reg_mbr_tc_offset
    pmbr_ex->gblAttr.i_TcToSkinAlpha = 16;      //reg_mbr_sm_text_alpha
    pmbr_ex->gblAttr.i_TcGradThr = 63;          //reg_mbr_tc_clip_thr

    pmbr_ex->gblAttr.i_PwYMax = 255;            //reg_mbr_pw_y_max
    pmbr_ex->gblAttr.i_PwYMin = 0;              //reg_mbr_pw_y_min
    pmbr_ex->gblAttr.i_PwCbMax = 133;           //reg_mbr_pw_cb_max
    pmbr_ex->gblAttr.i_PwCbMin = 65;            //reg_mbr_pw_cb_min
    pmbr_ex->gblAttr.i_PwCrMax = 200;           //reg_mbr_pw_cr_max
    pmbr_ex->gblAttr.i_PwCrMin = 133;           //reg_mbr_pw_cr_min
    pmbr_ex->gblAttr.i_PwCbPlusCrMin = 239;     //reg_mbr_pw_cb_plus_cr_min
    pmbr_ex->gblAttr.i_PwAddConditionEn = 1;    //reg_mbr_pw_cr_offset_on
    pmbr_ex->gblAttr.i_PwCrOffset = 130;        //reg_mbr_pw_cr_offset
    pmbr_ex->gblAttr.i_PwCbCrOffset = 3648;     //reg_mbr_pw_cb_cr_offset
    pmbr_ex->gblAttr.i_PcDeadZone = 16;         //reg_mbr_pw_pc_dead_zone

    pmbr_ex->gblAttr.i_SmoothEn = 1;            //reg_mbr_sm_on
    pmbr_ex->gblAttr.i_SmoothClipMax = 255;     //reg_mbr_sm_clip
    pmbr_ex->gblAttr.i_SecStgAlpha = 0;         //reg_mbr_ss_alpha
    pmbr_ex->gblAttr.i_SecStgBitWghtOffset = 0; //reg_mbr_ss_bit_weighting_offset

    /* Initial Frame Attribute */
    pmbr_ex->frmAttr.i_CbQpOffset = 0;
    pmbr_ex->frmAttr.i_CrQpOffset = 0;

    return pmbr_ex;
}

//------------------------------------------------------------------------------
//  Function    : _SeqConf
//  Description :
//------------------------------------------------------------------------------
static int _SeqConf(pmbr_ops* pmbr)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqSync
//  Description :
//------------------------------------------------------------------------------
static int _SeqSync(pmbr_ops* pmbr, mhve_job* mjob)
{
    mhe_pmbr* pmbr_ex   = (mhe_pmbr*)pmbr;
    mhe_reg* regs       = (mhe_reg*)mjob;
    uint reg_mbr_bit_per_weight = 0x400;

    pmbr->i_FrmCnt = 0;

    regs->hev_bank1.reg_mbr_bit_per_weight_low          = 1;
    regs->hev_bank1.reg_mbr_lut_entry_clip_range        = pmbr_ex->gblAttr.i_LutEntryClipRange;
    regs->hev_bank1.reg_mbr_const_qp_en                 = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->hev_bank1.reg_mbr_ss_alpha                    = pmbr_ex->gblAttr.i_SecStgAlpha;
    regs->hev_bank1.reg_mbr_ss_turn_off_perceptual      = 0;//(pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->hev_bank1.reg_mbr_ss_sw_read_weighting        = 1;
    regs->hev_bank1.reg_mbr_ss_turn_off_read_weighting  = 0;
    regs->hev_bank1.reg_mbr_ss_bit_weighting_offset     = pmbr_ex->gblAttr.i_SecStgBitWghtOffset;
    regs->hev_bank1.reg_mbr_tc_text_weight_type         = pmbr_ex->gblAttr.i_TextWeightType;
    regs->hev_bank1.reg_mbr_tc_clip_thr                 = pmbr_ex->gblAttr.i_TcGradThr;
    regs->hev_bank1.reg_mbr_tc_offset                   = pmbr_ex->gblAttr.i_TcOffset;

    regs->hev_bank1.reg_mbr_pw_y_max                    = pmbr_ex->gblAttr.i_PwYMax;
    regs->hev_bank1.reg_mbr_pw_y_min                    = pmbr_ex->gblAttr.i_PwYMin;
    regs->hev_bank1.reg_mbr_pw_cb_max                   = pmbr_ex->gblAttr.i_PwCbMax;
    regs->hev_bank1.reg_mbr_pw_cb_min                   = pmbr_ex->gblAttr.i_PwCbMin;
    regs->hev_bank1.reg_mbr_pw_cr_max                   = pmbr_ex->gblAttr.i_PwCrMax;
    regs->hev_bank1.reg_mbr_pw_cr_min                   = pmbr_ex->gblAttr.i_PwCrMin;
    regs->hev_bank1.reg_mbr_pw_cb_plus_cr_min           = pmbr_ex->gblAttr.i_PwCbPlusCrMin;
    regs->hev_bank1.reg_mbr_pw_pc_dead_zone             = pmbr_ex->gblAttr.i_PcDeadZone;
    regs->hev_bank1.reg_mbr_pw_cr_offset                = pmbr_ex->gblAttr.i_PwCrOffset;
    regs->hev_bank1.reg_mbr_pw_cr_offset_on             = pmbr_ex->gblAttr.i_PwAddConditionEn;
    regs->hev_bank1.reg_mbr_pw_pc_level_shift           = pmbr_ex->gblAttr.i_SkinLvShift;
    regs->hev_bank1.reg_mbr_pw_cb_cr_offset             = pmbr_ex->gblAttr.i_PwCbCrOffset;

    regs->hev_bank1.reg_mbr_sm_bit_weight_offset        = pmbr_ex->gblAttr.i_WeightOffset;
    regs->hev_bank1.reg_mbr_sm_on                       = pmbr_ex->gblAttr.i_SmoothEn;
    regs->hev_bank1.reg_mbr_sm_text_alpha               = pmbr_ex->gblAttr.i_TcToSkinAlpha;
    regs->hev_bank1.reg_mbr_sm_write_turn_off           = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->hev_bank1.reg_mbr_sm_clip                     = pmbr_ex->gblAttr.i_SmoothClipMax;
    regs->hev_bank1.reg_mbr_lut_roi_on                  = 0;//(input->UseRoiQp) ? 1 : 0;

    regs->hev_bank1.reg_mbr_gn_read_st_addr_low         = (pmbr_ex->frmAttr.m_nPercMBInMapPhy >> 4) & 0x0FFF;
    regs->hev_bank1.reg_mbr_gn_read_st_addr_high        = pmbr_ex->frmAttr.m_nPercMBInMapPhy >> 16;
    regs->hev_bank1.reg_mbr_gn_write_st_addr_low        = (pmbr_ex->frmAttr.m_nPercMBOutMapPhy >> 4) & 0x0FFF;
    regs->hev_bank1.reg_mbr_gn_write_st_addr_high       = pmbr_ex->frmAttr.m_nPercMBOutMapPhy >> 16;

    regs->hev_bank1.reg_mbr_lut_st_addr_low             = (pmbr_ex->gblAttr.i_LutAddrPhy >> 4) & 0x0FFF;
    regs->hev_bank1.reg_mbr_lut_st_addr_high            = pmbr_ex->gblAttr.i_LutAddrPhy >> 16;

    regs->hev_bank1.reg_mbr_smooth_window = 12;
    regs->hev_bank1.reg_mbr_smooth_window_on = 0;
    regs->hev_bank1.reg_mbr_bit_per_weight_low = reg_mbr_bit_per_weight & 0xFFFF;
    regs->hev_bank1.reg_mbr_bit_per_weight_high = reg_mbr_bit_per_weight >> 16;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SeqDone
//  Description :
//------------------------------------------------------------------------------
static int _SeqDone(pmbr_ops* pmbr)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncConf
//  Description :
//------------------------------------------------------------------------------
static int _EncConf(pmbr_ops* pmbr, mhve_job* mjob)
{
    mhe_pmbr* pmbr_ex   = (mhe_pmbr*)pmbr;
    pmbr_frmAttr* mbr   = &(pmbr_ex->frmAttr);
    mhe_reg* regs       = (mhe_reg*)mjob;
    int64* table;

    _pmbr_FillLUT(pmbr, pmbr->i_FrmQP, pmbr->i_FrmLamdaScaled);

    //set hw lut
    table = (int64*)pmbr_ex->gblAttr.b_LutAddrVirt;
    _set_hw_lut(mbr, table);

    /* Initialize Current Frame Config */
    regs->hev_bank1.reg_mbr_const_qp_en                 = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : !(pmbr_ex->frmAttr.m_bPercPicEn);
    regs->hev_bank1.reg_mbr_ss_turn_off_read_weighting  = !(pmbr_ex->frmAttr.m_bPercPicEn);

    //I frame or ContantQP, reg_mbr_const_qp_en always be enabled
    if(pmbr_ex->gblAttr.b_PmbrEnable)
    {
        if(pmbr->i_PicTyp == 0 || pmbr->i_LutTyp == 1)
        {
            regs->hev_bank1.reg_mbr_ss_alpha = 16;
            regs->hev_bank1.reg_mbr_const_qp_en = 1;
            regs->hev_bank1.reg_mbr_ss_turn_off_read_weighting = 1;
        }
        else
        {
            regs->hev_bank1.reg_mbr_ss_alpha = 0;
            regs->hev_bank1.reg_mbr_const_qp_en = 0;
            regs->hev_bank1.reg_mbr_ss_turn_off_read_weighting = 0;
        }
    }

    //CamOsPrintf("regs->hev_bank1.reg_mbr_const_qp_en = %d\n",regs->hev_bank1.reg_mbr_const_qp_en);
    //CamOsPrintf("regs->hev_bank1.reg_mbr_ss_turn_off_perceptual = %d\n",regs->hev_bank1.reg_mbr_ss_turn_off_perceptual);

    regs->hev_bank1.reg_mbr_gn_read_st_addr_low         = (pmbr_ex->frmAttr.m_nPercMBInMapPhy >> 4) & 0x0FFF;
    regs->hev_bank1.reg_mbr_gn_read_st_addr_high        = pmbr_ex->frmAttr.m_nPercMBInMapPhy >> 16;
    regs->hev_bank1.reg_mbr_gn_write_st_addr_low        = (pmbr_ex->frmAttr.m_nPercMBOutMapPhy >> 4) & 0x0FFF;
    regs->hev_bank1.reg_mbr_gn_write_st_addr_high       = pmbr_ex->frmAttr.m_nPercMBOutMapPhy >> 16;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncDone
//  Description :
//------------------------------------------------------------------------------
static int _EncDone(pmbr_ops* pmbr, mhve_job* mjob)
{
    mhe_pmbr* pmbr_ex   = (mhe_pmbr*)pmbr;
    mhe_reg* regs       = (mhe_reg*)mjob;
    int i = 0;

    pmbr->i_FrmCnt++;


    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        pmbr_ex->frmAttr.m_nTcHist[i] = regs->pmbr_tc_hist[i];
        pmbr_ex->frmAttr.m_nPcCntHist[i] = regs->pmbr_pc_hist[i];
    }

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        pmbr_ex->frmAttr.m_nLUTEntryHist[i] = regs->pmbr_lut_hist[i];
    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SetConf
//  Description :
//------------------------------------------------------------------------------
static int _SetConf(pmbr_ops* pmbr, pmbr_cfg* pmbrcfg)
{
    mhe_pmbr* pmbr_ex = (mhe_pmbr*)pmbr;
    int err = -1;

    switch(pmbrcfg->type)
    {
        case PMBR_CFG_SEQ:
            pmbr_ex->gblAttr.b_PmbrEnable = pmbrcfg->seq.i_enable;
            pmbr_ex->gblAttr.i_ctuW = pmbrcfg->seq.i_ctuw;
            pmbr_ex->gblAttr.i_ctuH = pmbrcfg->seq.i_ctuh;
            err = 0;
            break;
        case PMBR_CFG_TC:
            pmbr_ex->gblAttr.i_LutEntryClipRange = pmbrcfg->tc.i_LutEntryClipRange;
            pmbr_ex->gblAttr.i_TextWeightType = pmbrcfg->tc.i_TextWeightType;
            pmbr_ex->gblAttr.i_SkinLvShift = pmbrcfg->tc.i_SkinLvShift;
            pmbr_ex->gblAttr.i_WeightOffset = pmbrcfg->tc.i_WeightOffset;
            pmbr_ex->gblAttr.i_TcOffset = pmbrcfg->tc.i_TcOffset;
            pmbr_ex->gblAttr.i_TcToSkinAlpha = pmbrcfg->tc.i_TcToSkinAlpha;
            pmbr_ex->gblAttr.i_TcGradThr = pmbrcfg->tc.i_TcGradThr;
            err = 0;
            break;
        case PMBR_CFG_PC:
            pmbr_ex->gblAttr.i_PwYMax = pmbrcfg->pc.i_PwYMax;
            pmbr_ex->gblAttr.i_PwYMin = pmbrcfg->pc.i_PwYMin;
            pmbr_ex->gblAttr.i_PwCbMax = pmbrcfg->pc.i_PwCbMax;
            pmbr_ex->gblAttr.i_PwCbMin = pmbrcfg->pc.i_PwCbMin;
            pmbr_ex->gblAttr.i_PwCrMax = pmbrcfg->pc.i_PwCrMax;
            pmbr_ex->gblAttr.i_PwCrMin = pmbrcfg->pc.i_PwCrMin;
            pmbr_ex->gblAttr.i_PwCbPlusCrMin = pmbrcfg->pc.i_PwCbPlusCrMin;
            pmbr_ex->gblAttr.i_PwAddConditionEn = pmbrcfg->pc.i_PwAddConditionEn;
            pmbr_ex->gblAttr.i_PwCrOffset = pmbrcfg->pc.i_PwCrOffset;
            pmbr_ex->gblAttr.i_PwCbCrOffset = pmbrcfg->pc.i_PwCbCrOffset;
            pmbr_ex->gblAttr.i_PcDeadZone = pmbrcfg->pc.i_PcDeadZone;
            err = 0;
            break;
        case PMBR_CFG_SS:
            pmbr_ex->gblAttr.i_SmoothEn = pmbrcfg->ss.i_SmoothEn;
            pmbr_ex->gblAttr.i_SmoothClipMax = pmbrcfg->ss.i_SmoothClipMax;
            pmbr_ex->gblAttr.i_SecStgAlpha = pmbrcfg->ss.i_SecStgAlpha;
            pmbr_ex->gblAttr.i_SecStgBitWghtOffset = pmbrcfg->ss.i_SecStgBitWghtOffset;
            err = 0;
            break;
        case PMBR_CFG_MEM:
            pmbr_ex->frmAttr.m_nPercMBInMapVirt = pmbrcfg->mem.p_kptr[PMBR_MEM_MB_MAP_IN];
            pmbr_ex->frmAttr.m_nPercMBInMapPhy = pmbrcfg->mem.u_phys[PMBR_MEM_MB_MAP_IN];
            pmbr_ex->frmAttr.m_nPercMBOutMapVirt = pmbrcfg->mem.p_kptr[PMBR_MEM_MB_MAP_OUT];
            pmbr_ex->frmAttr.m_nPercMBOutMapPhy = pmbrcfg->mem.u_phys[PMBR_MEM_MB_MAP_OUT];

            pmbr_ex->gblAttr.b_LutAddrVirt = pmbrcfg->mem.p_kptr[PMBR_MEM_LUT];
            pmbr_ex->gblAttr.i_LutAddrPhy = pmbrcfg->mem.u_phys[PMBR_MEM_LUT];

            memset(pmbr_ex->frmAttr.m_nTcHist, 0, sizeof(int) * (1 << PMBR_LOG2_HIST_SIZE));
            memset(pmbr_ex->frmAttr.m_nPcCntHist, 0, sizeof(int) * (1 << PMBR_LOG2_HIST_SIZE));
            memset(pmbr_ex->frmAttr.m_nPercMBInMapVirt, 0, pmbrcfg->mem.i_size[PMBR_MEM_MB_MAP_IN]);
            memset(pmbr_ex->frmAttr.m_nPercMBOutMapVirt, 0, pmbrcfg->mem.i_size[PMBR_MEM_MB_MAP_OUT]);
            err = 0;
            break;
        case PMBR_CFG_LUT:
            err = 0;
            break;
        case PMBR_CFG_FRM:
            pmbr_ex->frmAttr.i_CbQpOffset = pmbrcfg->frm.i_cb_qp_offset;
            pmbr_ex->frmAttr.i_CrQpOffset = pmbrcfg->frm.i_cr_qp_offset;
            err = 0;
            break;
        default:
            break;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : _GetConf
//  Description : TBD
//------------------------------------------------------------------------------
static int _GetConf(pmbr_ops* pmbr, pmbr_cfg* pmbrcfg)
{
    mhe_pmbr* pmbr_ex = (mhe_pmbr*)pmbr;
    int err = -1;

    switch(pmbrcfg->type)
    {
        case PMBR_CFG_SEQ:
            pmbrcfg->seq.i_enable = pmbr_ex->gblAttr.b_PmbrEnable;
            pmbrcfg->seq.i_ctuw = pmbr_ex->gblAttr.i_ctuW;
            pmbrcfg->seq.i_ctuh = pmbr_ex->gblAttr.i_ctuH;
            err = 0;
            break;
        case PMBR_CFG_TC:
            pmbrcfg->tc.i_LutEntryClipRange = pmbr_ex->gblAttr.i_LutEntryClipRange;
            pmbrcfg->tc.i_TextWeightType = pmbr_ex->gblAttr.i_TextWeightType;
            pmbrcfg->tc.i_SkinLvShift = pmbr_ex->gblAttr.i_SkinLvShift;
            pmbrcfg->tc.i_WeightOffset = pmbr_ex->gblAttr.i_WeightOffset;
            pmbrcfg->tc.i_TcOffset = pmbr_ex->gblAttr.i_TcOffset;
            pmbrcfg->tc.i_TcToSkinAlpha = pmbr_ex->gblAttr.i_TcToSkinAlpha;
            pmbrcfg->tc.i_TcGradThr = pmbr_ex->gblAttr.i_TcGradThr;
            err = 0;
            break;
        case PMBR_CFG_PC:
            pmbrcfg->pc.i_PwYMax = pmbr_ex->gblAttr.i_PwYMax;
            pmbrcfg->pc.i_PwYMin = pmbr_ex->gblAttr.i_PwYMin;
            pmbrcfg->pc.i_PwCbMax = pmbr_ex->gblAttr.i_PwCbMax;
            pmbrcfg->pc.i_PwCbMin = pmbr_ex->gblAttr.i_PwCbMin;
            pmbrcfg->pc.i_PwCrMax = pmbr_ex->gblAttr.i_PwCrMax;
            pmbrcfg->pc.i_PwCrMin = pmbr_ex->gblAttr.i_PwCrMin;
            pmbrcfg->pc.i_PwCbPlusCrMin = pmbr_ex->gblAttr.i_PwCbPlusCrMin;
            pmbrcfg->pc.i_PwAddConditionEn = pmbr_ex->gblAttr.i_PwAddConditionEn;
            pmbrcfg->pc.i_PwCrOffset = pmbr_ex->gblAttr.i_PwCrOffset;
            pmbrcfg->pc.i_PwCbCrOffset = pmbr_ex->gblAttr.i_PwCbCrOffset;
            pmbrcfg->pc.i_PcDeadZone = pmbr_ex->gblAttr.i_PcDeadZone;
            err = 0;
            break;
        case PMBR_CFG_SS:
            pmbrcfg->ss.i_SmoothEn = pmbr_ex->gblAttr.i_SmoothEn;
            pmbrcfg->ss.i_SmoothClipMax = pmbr_ex->gblAttr.i_SmoothClipMax;
            pmbrcfg->ss.i_SecStgAlpha = pmbr_ex->gblAttr.i_SecStgAlpha;
            pmbrcfg->ss.i_SecStgBitWghtOffset = pmbr_ex->gblAttr.i_SecStgBitWghtOffset;
            err = 0;
            break;
        case PMBR_CFG_MEM:
            pmbrcfg->mem.p_kptr[PMBR_MEM_MB_MAP_IN] = pmbr_ex->frmAttr.m_nPercMBInMapVirt;
            pmbrcfg->mem.u_phys[PMBR_MEM_MB_MAP_IN] = pmbr_ex->frmAttr.m_nPercMBInMapPhy;
            pmbrcfg->mem.p_kptr[PMBR_MEM_MB_MAP_OUT] = pmbr_ex->frmAttr.m_nPercMBOutMapVirt;
            pmbrcfg->mem.u_phys[PMBR_MEM_MB_MAP_OUT] = pmbr_ex->frmAttr.m_nPercMBOutMapPhy;
            pmbrcfg->mem.p_kptr[PMBR_MEM_LUT] = pmbr_ex->gblAttr.b_LutAddrVirt;
            pmbrcfg->mem.u_phys[PMBR_MEM_LUT] = pmbr_ex->gblAttr.i_LutAddrPhy;
            err = 0;
            break;
        case PMBR_CFG_LUT:
            pmbrcfg->lut.p_kptr[PMBR_LUT_QP] = pmbr_ex->frmAttr.m_nLUTQp;
            pmbrcfg->lut.p_kptr[PMBR_LUT_TARGET] = pmbr_ex->frmAttr.m_nLUTTarget;
            pmbrcfg->lut.p_kptr[PMBR_LUT_LAMBDA_SCALED] = pmbr_ex->frmAttr.m_auiLUTLambdas_SCALED;
            pmbrcfg->lut.p_kptr[PMBR_LUT_AIIDCHIST] = pmbr_ex->frmAttr.m_nLUTEntryHist;
            err = 0;
            break;
        case PMBR_CFG_FRM:
            pmbrcfg->frm.i_cb_qp_offset = pmbr_ex->frmAttr.i_CbQpOffset;
            pmbrcfg->frm.i_cr_qp_offset = pmbr_ex->frmAttr.i_CrQpOffset;
            err = 0;
            break;
        default:
            break;
    }

    return err;
}
