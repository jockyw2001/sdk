
#include "hal_mfe_def.h"
#include "hal_mfe_reg.h"
#include "hal_mfe_pmbr.h"

#include "hal_mfe_global.h"
#include "hal_mfe_msmath.h"

#define MIN_QP  0
#define MAX_QP  51

//#define PMBR_CNT_PENCENT_THR 0.5
#define PMBR_CNT_PENCENT_THR_NUM   1
#define PMBR_CNT_PENCENT_THR_DENORM 2

// unit: 0.01
#define PMBR_CNT_PENCENT_THR 50

// minimun bin number threshold for perceptual MBR enable
//#define PMBR_BIN_THR 5

#define MAX_VALUE   0x7FFFFFFF

// CTB min, max QP
uint32 g_uiRcCtuMinQp = MIN_QP;
uint32 g_uiRcCtuMaxQp = MAX_QP;

unsigned int Mfe_MBR_lut_Size = 240;

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

#ifdef SIMULATE_ON_I3
uchar g_aiHWTextCplxHist_test[32*10] =
{
        0,6,10,8,17,21,30,45,49,35,32,46,32,25,17,12,7,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,3,9,8,13,23,38,46,40,44,38,40,35,22,16,12,5,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,7,6,11,13,24,37,44,46,40,42,36,32,21,16,11,5,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,4,8,8,12,27,35,47,45,45,39,38,30,18,21,6,9,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,4,9,9,17,31,34,38,47,41,42,41,20,24,16,12,4,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,3,6,16,14,27,31,40,42,38,49,40,22,22,23,10,7,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,9,14,22,19,31,43,38,44,36,44,29,24,19,11,7,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,10,18,14,23,32,41,26,50,46,37,28,22,21,14,5,5,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,2,6,18,14,18,32,46,30,45,47,37,36,19,16,14,8,5,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,4,4,17,17,19,33,39,42,45,44,36,33,16,15,11,14,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,

};
#endif


static void _set_hw_lut(pmbr_frmAttr* ptPmbr, uint64* table)
{
    uint32 uiQpY, uiBits, uiEntryIdx;
    uint64 ui64LutHigh, ui64LutLow;
    uint64 aui64LUTMemory[MBR_LUT_SIZE * 2];

    for (uiEntryIdx = 0; uiEntryIdx < MBR_LUT_SIZE; uiEntryIdx++)
    {
        uiQpY = ptPmbr->m_nLUTQp[uiEntryIdx];
        uiBits = ptPmbr->m_nLUTTarget[uiEntryIdx];
        // LUT entry : data packing
        ui64LutHigh = (uint64)(uiQpY << 21);
        ui64LutLow = (uint64) uiBits;

        // Fill LUT memory
        aui64LUTMemory[uiEntryIdx << 1] = ui64LutLow;
        aui64LUTMemory[(uiEntryIdx << 1) + 1] = ui64LutHigh;
    }

    if(table)
        memcpy(table, aui64LUTMemory, Mfe_MBR_lut_Size);
}

static int _pmbr_checkHistValid(int *pBin, int TotalBinNum, int binNumThr, int binCntThr)
{
    int nonZeroBinNum = 0, idx;
    for (idx = 0; idx < TotalBinNum; idx++)
    {
        if (pBin[idx] != 0)
        {
            nonZeroBinNum++;
        }
        if (pBin[idx] >= binCntThr)
        {
            return 0;
        }
    }
    if (nonZeroBinNum < binNumThr)
    {
        return 0;
    }
    return 1;
}
// ----------------------------------------------------------------
static void _perc_mbr_set_OneLutEntry(pmbr_frmAttr* mbr, uint32 uiEntryIdx, uint32 Qp, uint32 uiLambda_SCALED, uint32 uiBppMult1024)
{
    uint32 Bit;

    if(uiEntryIdx >= LUT_SIZE)
        return;

    Qp = uiClip3(g_uiRcCtuMinQp, g_uiRcCtuMaxQp, Qp);
    mbr->m_nLUTQp[uiEntryIdx] = Qp;
    Bit = iClip3(0, MAX_CTB_BIT, uiBppMult1024);
    mbr->m_nLUTTarget[uiEntryIdx] = Bit;
}


static void _pmbr_FillLUT(pmbr_ops* pmbr, int center_qp)
{
    mfe6_pmbr* pmbr_ex = (mfe6_pmbr*)pmbr;
    pmbr_frmAttr* mbr  = &(pmbr_ex->frmAttr);

    int entry, bin;
    int iHistSize = 1 << PMBR_LOG2_HIST_SIZE;
    int histCdf[1 << PMBR_LOG2_HIST_SIZE];
    unsigned char* tempPtr;
    int tempAddr;
    uint32 uiPicCtuNum = pmbr_ex->gblAttr.i_mbW*pmbr_ex->gblAttr.i_mbH;
    const uint32 uiLutSize = MBR_LUT_SIZE;
    uint32 auiTargetTc[1 << PMBR_LOG2_HIST_SIZE];
    int aiDeltaQpTable[MBR_LUT_SIZE] = {-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7};
    int64 remainder;
    uint32 uiHighestQp;
    uint32 uiEntryQp;
    int cntAccum = 0, binCnt;
    uint32 uiNormBinNum_SCALED;
    uint32 uiRoundVal;
    int iBinInteval;
    int binSel = 0, targetCnt, dist, histIncr, iTargetTc;
    int iTableSize;
    int centerIdx;

    MFE_MSG(MFE_MSG_DEBUG, "center_qp = %d\n", center_qp);

    //CamOsPrintf("center_qp = %d, pmbr->i_LutTyp = %d\n", center_qp, pmbr->i_LutTyp);

    if(pmbr->i_LutTyp == 1)
    {
        iTableSize = ((MBR_LUT_SIZE >> 1) << 1) + 1;
        centerIdx = iTableSize >> 1;
        memset(&mbr->m_nLUTQp, 0, sizeof(int) * LUT_SIZE);
        memset(&mbr->m_nLUTTarget, 0, sizeof(int) * LUT_SIZE);

        mbr->m_nLUTQp[centerIdx] = center_qp;
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

        for(entry = 0; entry < uiLutSize; entry++)
        {
            if(pmbr->i_LutRoiQp[entry] != 0)
            {
                uiEntryQp = pmbr->i_LutRoiQp[entry];
            }
            else
            {
                //uiEntryQp = uiHighestQp - entry;
                uiEntryQp = center_qp;
            }

            _perc_mbr_set_OneLutEntry(mbr, entry, uiEntryQp, 0, (uiLutSize - entry + 1));
        }

        return;
    }

    // Picture level Perceptual MBR decision
    if(pmbr->i_FrmCnt != 0 && pmbr_ex->gblAttr.b_PmbrEnable != 0)
    {
        int isValid = _pmbr_checkHistValid(mbr->m_nTcHist, (1 << PMBR_LOG2_HIST_SIZE), PMBR_BIN_NUM_THR, uiPicCtuNum*PMBR_CNT_PENCENT_THR/100);
        mbr->m_bPercPicEn = isValid;
    }
    else
    {
        mbr->m_bPercPicEn = 0;
    }

    //CamOsPrintf( "mbr->m_bPercPicEn = %d, center_qp = %d\n", mbr->m_bPercPicEn, center_qp);

    // Filling LUT for Perceptual MBR disable case
    if (!mbr->m_bPercPicEn)
    {
        uiHighestQp = center_qp + (uiLutSize >> 1);
        //assert(center_qp > (uiLutSize >> 1),"center_qp > (uiLutSize >> 1");

        for (entry = 0; entry < uiLutSize; entry++)
        {
            uiEntryQp = uiHighestQp - entry;
            uiEntryQp = ((int32)uiEntryQp < 0) ? 0 : uiEntryQp;
            _perc_mbr_set_OneLutEntry(mbr, entry, uiEntryQp, 0, (uiLutSize - entry + 1));
        }

        return;
    }

    // Histogram equalized base LUT filling
    {
        // Accumulate histogram CDF
        for (bin = 0; bin < iHistSize; bin++)
        {
            binCnt = mbr->m_nTcHist[bin];
            cntAccum += binCnt;
            histCdf[bin] = cntAccum;
        }
    }

    {
        // -- Tc range determination for bin equalization --
        uiNormBinNum_SCALED = (uiPicCtuNum<<PREC_SCALE_BITS)/uiLutSize;
        uiRoundVal = 1 << (PREC_SCALE_BITS-1);
        iBinInteval = 1 << (8 - LOG2_HIST_SIZE);
        binSel = 0;
        for(entry=0; entry<uiLutSize; entry++)
        {
            targetCnt = (int)((uiNormBinNum_SCALED*(entry+1) + uiRoundVal) >> PREC_SCALE_BITS );
            dist = MAX_VALUE;

            for(bin=binSel; bin<iHistSize; bin++)
            {
              if(histCdf[bin]>targetCnt){
                break;
              }
              if(iabs(histCdf[bin]-targetCnt)<dist)
              {
                dist = iabs(histCdf[bin]-targetCnt);
                binSel = bin;
              }
            }

            histIncr = histCdf[binSel+1] - histCdf[binSel];
            iTargetTc = (iBinInteval>>1) + (binSel<<3);
            if(histIncr!=0)
            {
                CDBZ(histIncr,"histIncr");
                iTargetTc += CamOsMathDivS64(((int64)(targetCnt - histCdf[binSel])*iBinInteval),histIncr, &remainder);
            }
            else
            {
              iTargetTc += iBinInteval;
            }
            auiTargetTc[entry]= iClip3(0, 255, iTargetTc);
         }
    }

    // LUT input validity check: entry value must be in mono-decresing order to meet HW constrain
    for (entry = 1; entry < MBR_LUT_SIZE; entry++)
    {
        if (auiTargetTc[entry] <= auiTargetTc[entry - 1])
        {
            auiTargetTc[entry] = auiTargetTc[entry - 1] + 1;
        }
    }

    {
        int centerEntry = MBR_LUT_SIZE >> 1;
        int upAccumQpDel_SCALED = 0, downAccumQpDel_SCALED = 0;
        int iter, upLastTc, downLastTc;
        int tcLvToDiffThr[16] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12, 12, 12 };
        int maxEntryQpDelta_SCALED = 2 << PREC_SCALE_BITS;
        int maxAccumQpDel_SCALED = 12 << PREC_SCALE_BITS;
        upLastTc = auiTargetTc[centerEntry];
        downLastTc = auiTargetTc[centerEntry];
        for (iter = 1; iter <= (MBR_LUT_SIZE >> 1); iter++)
        {
            int upTc = auiTargetTc[centerEntry + iter];
            int downTc = auiTargetTc[centerEntry - iter];

            int upTcLv = upLastTc >> 4;
            int downTcLv = downLastTc >> 4;

            int upTcBase = tcLvToDiffThr[upTcLv];
            int downTcBase = tcLvToDiffThr[downTcLv];

            int upTcDiff = iabs(upTc - upLastTc);
            int downTcDiff = iabs(downTc - downLastTc);

            int upQpDel_SCALED = iClip3(0, maxEntryQpDelta_SCALED, (upTcDiff << PREC_SCALE_BITS) / upTcBase);
            int downQpDel_SCALED = iClip3(0, maxEntryQpDelta_SCALED, (downTcDiff << PREC_SCALE_BITS) / downTcBase);

            upAccumQpDel_SCALED = iClip3(0, maxAccumQpDel_SCALED, upAccumQpDel_SCALED + upQpDel_SCALED + 1); // "+1": sync with c-model
            downAccumQpDel_SCALED = iClip3(0, maxAccumQpDel_SCALED, downAccumQpDel_SCALED + downQpDel_SCALED + 1); // "+1": sync with c-model

            aiDeltaQpTable[centerEntry + iter] = upAccumQpDel_SCALED >> PREC_SCALE_BITS;
            aiDeltaQpTable[centerEntry - iter] = -(downAccumQpDel_SCALED >> PREC_SCALE_BITS);

            upLastTc = upTc;
            downLastTc = downTc;
        }

        {
            int upDelQpSum = 0, downDelQpSum = 0;
            int compenQpOffset, isUpLarge;
            for (iter = 1; iter <= (MBR_LUT_SIZE >> 1); iter++)
            {
                upDelQpSum += aiDeltaQpTable[centerEntry + iter];
                downDelQpSum += aiDeltaQpTable[centerEntry - iter];
            }

            isUpLarge = (iabs(upDelQpSum) > iabs(downDelQpSum)) ? 1 : 0;
            compenQpOffset = (iabs(iabs(upDelQpSum) - iabs(downDelQpSum)) + (MBR_LUT_SIZE >> 2)) / (MBR_LUT_SIZE >> 1);

            if (isUpLarge)
            {
                for (iter = 1; iter <= (MBR_LUT_SIZE >> 1); iter++)
                {
                    int deltaQp = aiDeltaQpTable[centerEntry + iter] - compenQpOffset;
                    aiDeltaQpTable[centerEntry + iter] = iClip3(0, 12, deltaQp);
                }
            }
            else
            {
                for (iter = 1; iter <= (MBR_LUT_SIZE >> 1); iter++)
                {
                    int deltaQp = aiDeltaQpTable[centerEntry - iter] + compenQpOffset;
                    aiDeltaQpTable[centerEntry - iter] = iClip3(-12, 0, deltaQp);
                }
            }
        }
    }

    // set LUT table
    for (entry = 0; entry < MBR_LUT_SIZE; entry++)
    {
        int entry_qp = aiDeltaQpTable[entry] + center_qp;

        // entry index inverse since LUT target array follow decreasing order
        int entry_sel = MBR_LUT_SIZE - entry - 1;
        entry_qp = (entry_qp < 0) ? 0 : entry_qp;
        _perc_mbr_set_OneLutEntry(mbr, entry_sel, entry_qp, 0, auiTargetTc[entry]);
    }


    // Switch the In/Out Map
    tempPtr = mbr->m_nPercMBInMapVirt;
    mbr->m_nPercMBInMapVirt = mbr->m_nPercMBOutMapVirt;
    mbr->m_nPercMBOutMapVirt = tempPtr;

    tempAddr = mbr->m_nPercMBInMapPhy;
    mbr->m_nPercMBInMapPhy = mbr->m_nPercMBOutMapPhy;
    mbr->m_nPercMBOutMapPhy = tempAddr;

    return;
#if 0
RESET_STATS:
    memset(mbr->m_nTcHist, 0 , sizeof(int)*(1<<PMBR_LOG2_HIST_SIZE));
    memset(mbr->m_nPcCntHist, 0 , sizeof(int)*(1<<PMBR_LOG2_HIST_SIZE));
    memset(mbr->m_nLUTEntryHist, 0 , sizeof(int)*LUT_SIZE);

    mbr->m_nTcAccum = 0;
    mbr->m_nPcCntAccum = 0;
    mbr->m_nOutWeightAccum = 0;
    mbr->m_nSecStageWghtAccum = 0;

    // Switch the In/Out Map
    tempPtr = mbr->m_nPercMBInMapVirt;
    mbr->m_nPercMBInMapVirt = mbr->m_nPercMBOutMapVirt;
    mbr->m_nPercMBOutMapVirt = tempPtr;

    tempAddr = mbr->m_nPercMBInMapPhy;
    mbr->m_nPercMBInMapPhy = mbr->m_nPercMBOutMapPhy;
    mbr->m_nPercMBOutMapPhy = tempAddr;

    // Pack the PMBR LUT Entry data : 8*2 byte per entry
    //int64 table[(LUT_SIZE << 1)];
    table = (int64*)pmbr_ex->gblAttr.b_LutAddrVirt;

    for (entry = 0; entry < LUT_SIZE; entry++)
    {
        QpY = mbr->m_nLUTQp[entry];
        Target = mbr->m_nLUTTarget[entry];
        reg0 = (QpY << 21);
        reg1 = (int64)Target;
        reg0_extendBit = reg0;

        table[(entry << 1) + 1] = reg0_extendBit;
        table[(entry << 1)] = reg1;
    }
#endif
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
//  Function    : PmbrAllocate
//  Description :
//------------------------------------------------------------------------------
void* PmbrAllocate(void)
{
    pmbr_ops*   pmbr_op = NULL;
    mfe6_pmbr*  pmbr_ex;

    if (!(pmbr_op = MEM_ALLC(sizeof(mfe6_pmbr))))
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
    pmbr_ex = (mfe6_pmbr*)pmbr_op;
    pmbr_ex->gblAttr.i_LutEntryClipRange = 15;  //reg101_mbr_lut_entry_clip_range
    pmbr_ex->gblAttr.i_TextWeightType = 2;      //reg104_mbr_tc_text_weight_type
    pmbr_ex->gblAttr.i_SkinLvShift = 0;         //reg109_mbr_pw_pc_level_shift
    pmbr_ex->gblAttr.i_WeightOffset = 0;        //reg10b_mbr_sm_bit_weight_offset
    pmbr_ex->gblAttr.i_TcOffset = 0;            //reg104_mbr_tc_offset
    pmbr_ex->gblAttr.i_TcToSkinAlpha = 16;      //reg10b_mbr_sm_text_alpha
    pmbr_ex->gblAttr.i_TcGradThr = 63;          //reg104_mbr_tc_clip_thr

    pmbr_ex->gblAttr.i_PwYMax = 255;            //reg105_mbr_pw_y_max
    pmbr_ex->gblAttr.i_PwYMin = 0;              //reg105_mbr_pw_y_min
    pmbr_ex->gblAttr.i_PwCbMax = 133;           //reg106_mbr_pw_cb_max
    pmbr_ex->gblAttr.i_PwCbMin = 65;            //reg106_mbr_pw_cb_min
    pmbr_ex->gblAttr.i_PwCrMax = 200;           //reg107_mbr_pw_cr_max
    pmbr_ex->gblAttr.i_PwCrMin = 133;           //reg107_mbr_pw_cr_min
    pmbr_ex->gblAttr.i_PwCbPlusCrMin = 239;     //reg108_mbr_pw_cb_plus_cr_min
    pmbr_ex->gblAttr.i_PwAddConditionEn = 1;    //reg109_mbr_pw_cr_offset_on
    pmbr_ex->gblAttr.i_PwCrOffset = 130;        //reg109_mbr_pw_cr_offset
    pmbr_ex->gblAttr.i_PwCbCrOffset = 3648;     //reg10a_mbr_pw_cb_cr_offset
    pmbr_ex->gblAttr.i_PcDeadZone = 0;         //reg108_mbr_pw_pc_dead_zone

    pmbr_ex->gblAttr.i_SmoothEn = 0;            //reg10b_mbr_sm_on
    pmbr_ex->gblAttr.i_SmoothClipMax = 255;     //reg10c_mbr_sm_clip
    pmbr_ex->gblAttr.i_SecStgAlpha = 0;         //reg102_mbr_ss_alpha
    pmbr_ex->gblAttr.i_SecStgBitWghtOffset = 0; //reg103_mbr_ss_bit_weighting_offset

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
    mfe6_pmbr* pmbr_ex  = (mfe6_pmbr*)pmbr;
    mfe6_reg* regs      = (mfe6_reg*)mjob;

    pmbr->i_FrmCnt = 0;

    regs->reg100_mbr_bit_per_weight_low         = 1;
    regs->reg101_mbr_lut_entry_clip_range       = pmbr_ex->gblAttr.i_LutEntryClipRange;
    regs->reg101_mbr_const_qp_en                = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->reg102_mbr_ss_alpha                   = pmbr_ex->gblAttr.i_SecStgAlpha;
    regs->reg102_mbr_ss_turn_off_perceptual     = 0;//(pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->reg102_mbr_ss_sw_read_weighting       = 1;
    regs->reg102_mbr_ss_turn_off_read_weighting = 0;
    regs->reg103_mbr_ss_bit_weighting_offset    = pmbr_ex->gblAttr.i_SecStgBitWghtOffset;
    regs->reg104_mbr_tc_text_weight_type        = pmbr_ex->gblAttr.i_TextWeightType;
    regs->reg104_mbr_tc_clip_thr                = pmbr_ex->gblAttr.i_TcGradThr;
    regs->reg104_mbr_tc_offset                  = pmbr_ex->gblAttr.i_TcOffset;

    regs->reg105_mbr_pw_y_max                   = pmbr_ex->gblAttr.i_PwYMax;
    regs->reg105_mbr_pw_y_min                   = pmbr_ex->gblAttr.i_PwYMin;
    regs->reg106_mbr_pw_cb_max                  = pmbr_ex->gblAttr.i_PwCbMax;
    regs->reg106_mbr_pw_cb_min                  = pmbr_ex->gblAttr.i_PwCbMin;
    regs->reg107_mbr_pw_cr_max                  = pmbr_ex->gblAttr.i_PwCrMax;
    regs->reg107_mbr_pw_cr_min                  = pmbr_ex->gblAttr.i_PwCrMin;
    regs->reg108_mbr_pw_cb_plus_cr_min          = pmbr_ex->gblAttr.i_PwCbPlusCrMin;
    regs->reg108_mbr_pw_pc_dead_zone            = pmbr_ex->gblAttr.i_PcDeadZone;
    regs->reg109_mbr_pw_cr_offset               = pmbr_ex->gblAttr.i_PwCrOffset;
    regs->reg109_mbr_pw_cr_offset_on            = pmbr_ex->gblAttr.i_PwAddConditionEn;
    regs->reg109_mbr_pw_pc_level_shift          = pmbr_ex->gblAttr.i_SkinLvShift;
    regs->reg10a_mbr_pw_cb_cr_offset            = pmbr_ex->gblAttr.i_PwCbCrOffset;

    regs->reg10b_mbr_sm_bit_weight_offset       = pmbr_ex->gblAttr.i_WeightOffset;
    regs->reg10b_mbr_sm_on                      = pmbr_ex->gblAttr.i_SmoothEn;
    regs->reg10b_mbr_sm_text_alpha              = pmbr_ex->gblAttr.i_TcToSkinAlpha;
    regs->reg10b_mbr_sm_write_turn_off          = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->reg10c_mbr_sm_clip                    = pmbr_ex->gblAttr.i_SmoothClipMax;
    regs->reg10c_mbr_lut_roi_on                 = 0;//(input->UseRoiQp) ? 1 : 0;

    regs->reg110_mbr_gn_read_st_addr_low        = (pmbr_ex->frmAttr.m_nPercMBInMapPhy>>4)&0x0FFF;
    regs->reg111_mbr_gn_read_st_addr_high       = pmbr_ex->frmAttr.m_nPercMBInMapPhy>>16;
    regs->reg10e_mbr_gn_write_st_addr_low       = (pmbr_ex->frmAttr.m_nPercMBOutMapPhy>>4)&0x0FFF;
    regs->reg10f_mbr_gn_write_st_addr_high      = pmbr_ex->frmAttr.m_nPercMBOutMapPhy>>16;

    regs->reg112_mbr_lut_st_addr_low            = (pmbr_ex->gblAttr.i_LutAddrPhy>>4)&0x0FFF;
    regs->reg113_mbr_lut_st_addr_high           = pmbr_ex->gblAttr.i_LutAddrPhy>>16;

#if 0 // TBD
    regs->reg114_mbr_gn_read_map_st_addr_low    = (pBufInfo->m_nRoiQpMapAddr>>4)&0x0FFF;
    regs->reg115_mbr_gn_read_map_st_addr_high   = pBufInfo->m_nRoiQpMapAddr>>16;

#endif
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
    mfe6_pmbr* pmbr_ex  = (mfe6_pmbr*)pmbr;
    pmbr_frmAttr* mbr   = &(pmbr_ex->frmAttr);
    mfe6_reg* regs      = (mfe6_reg*)mjob;
    uint64* table;

#if 0
    if(pmbr->i_LutTyp == 1) //RQCT_MODE_CQP, disable PMBR
    {
        regs->reg101_mbr_const_qp_en = 0;
        regs->reg102_mbr_ss_turn_off_perceptual = 1;
        regs->reg102_mbr_ss_turn_off_read_weighting = 1;
        regs->reg10b_mbr_sm_write_turn_off = 1;

        return 0;
    }
#endif

    _pmbr_FillLUT(pmbr, pmbr->i_FrmQP);

    //set hw lut
    table = (int64*)pmbr_ex->gblAttr.b_LutAddrVirt;
    _set_hw_lut(mbr, table);

    //print_hex("mbr_lut by PMBR", pmbr_ex->gblAttr.b_LutAddrVirt, 240);

    /* Initialize Current Frame Config */
    regs->reg101_mbr_const_qp_en                = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : !(pmbr_ex->frmAttr.m_bPercPicEn);
    regs->reg102_mbr_ss_turn_off_read_weighting = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;
    regs->reg10b_mbr_sm_write_turn_off          = (pmbr_ex->gblAttr.b_PmbrEnable == 0) ? 1 : 0;// !(pmbr_ex->frmAttr.m_bPercPicEn);//

    if(pmbr_ex->gblAttr.b_PmbrEnable)
    {
        if(pmbr->i_PicTyp == 0 || pmbr->i_LutTyp == 1)
        {
            regs->reg102_mbr_ss_alpha = 0;
            regs->reg101_mbr_const_qp_en = 1;
            regs->reg102_mbr_ss_turn_off_read_weighting = 1;
        }
        else
        {
            regs->reg102_mbr_ss_alpha = 0;
            regs->reg101_mbr_const_qp_en = 0;
            regs->reg102_mbr_ss_turn_off_read_weighting = 0;
        }
    }

    //CamOsPrintf("regs->reg101_mbr_const_qp_en = %d\n",regs->reg101_mbr_const_qp_en);
    //CamOsPrintf("regs->reg102_mbr_ss_turn_off_perceptual = %d\n",regs->reg102_mbr_ss_turn_off_perceptual);

    regs->reg110_mbr_gn_read_st_addr_low        = (pmbr_ex->frmAttr.m_nPercMBInMapPhy>>4)&0x0FFF;
    regs->reg111_mbr_gn_read_st_addr_high       = pmbr_ex->frmAttr.m_nPercMBInMapPhy>>16;
    regs->reg10e_mbr_gn_write_st_addr_low       = (pmbr_ex->frmAttr.m_nPercMBOutMapPhy>>4)&0x0FFF;
    regs->reg10f_mbr_gn_write_st_addr_high      = pmbr_ex->frmAttr.m_nPercMBOutMapPhy>>16;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncDone
//  Description :
//------------------------------------------------------------------------------
static int _EncDone(pmbr_ops* pmbr, mhve_job* mjob)
{
    mfe6_pmbr* pmbr_ex  = (mfe6_pmbr*)pmbr;
    mfe6_reg* regs      = (mfe6_reg*)mjob;
    int i = 0;

#ifdef SIMULATE_ON_I3
    for(i = 0;i < 32; i++)
    {
        regs->pmbr_tc_hist[i] = g_aiHWTextCplxHist_test[pmbr->i_FrmCnt*32+i];
    }
#endif

    pmbr->i_FrmCnt++;

    for (i = 0; i < (1<<PMBR_LOG2_HIST_SIZE); i++) {
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
    mfe6_pmbr* pmbr_ex = (mfe6_pmbr*)pmbr;
    int err = -1;

    switch (pmbrcfg->type)
    {
        case PMBR_CFG_SEQ:
            pmbr_ex->gblAttr.b_PmbrEnable = pmbrcfg->seq.i_enable;
            pmbr_ex->gblAttr.i_mbW = pmbrcfg->seq.i_mbw;
            pmbr_ex->gblAttr.i_mbH = pmbrcfg->seq.i_mbh;
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

            memset(pmbr_ex->frmAttr.m_nTcHist, 0, sizeof(int)*(1<<PMBR_LOG2_HIST_SIZE));
            memset(pmbr_ex->frmAttr.m_nPcCntHist, 0, sizeof(int)*(1<<PMBR_LOG2_HIST_SIZE));
            memset(pmbr_ex->frmAttr.m_nPercMBInMapVirt, 0, pmbrcfg->mem.i_size[PMBR_MEM_MB_MAP_IN]);
            memset(pmbr_ex->frmAttr.m_nPercMBOutMapVirt, 0, pmbrcfg->mem.i_size[PMBR_MEM_MB_MAP_OUT]);
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
    mfe6_pmbr* pmbr_ex = (mfe6_pmbr*)pmbr;
    int err = -1;

    switch (pmbrcfg->type)
    {
        case PMBR_CFG_SEQ:
            pmbrcfg->seq.i_enable = pmbr_ex->gblAttr.b_PmbrEnable;
            pmbrcfg->seq.i_mbw = pmbr_ex->gblAttr.i_mbW;
            pmbrcfg->seq.i_mbh = pmbr_ex->gblAttr.i_mbH;
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
            pmbrcfg->lut.p_kptr[PMBR_LUT_AIIDCHIST] = pmbr_ex->frmAttr.m_nLUTEntryHist;
            err = 0;
            break;
        default:
            break;
    }

    return err;
}
