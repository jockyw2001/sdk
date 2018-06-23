//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#define HAL_VPE_C


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
// SC Definition
#include "drv_scl_os.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_dma_io_wrapper.h"
#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_io_wrapper.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_m2m_io_st.h"
#include "drv_scl_m2m_io_wrapper.h"
#include "drv_scl_verchk.h"
#include "drv_scl_irq.h"
#include "mhal_vpe.h"
#include "infinity2_reg_isp0.h"
#include <libcamera.h>
#include <libcamera_if.h>
#if defined(SCLOS_TYPE_LINUX_TEST)
#else
#include <bufptr.h>
#endif
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define minmax(v,a,b)       (((v)<(a))? (a) : ((v)>(b)) ? (b) : (v))

#if defined (SCLOS_TYPE_LINUX_KERNEL)
#else
#ifdef abs
#undef abs
#endif
#define abs(a)              ((a)<0?-(a):(a))

#ifdef min
#undef min
#endif
#define min(x,y) \
  ( x < y ?  x :  y)
#ifdef max
#undef max
#endif
#define max(x,y) \
  ( x > y ? x : y)
#endif
//-------------------------------------------------------------------------------------------------
//  SC Defines & Macro
//-------------------------------------------------------------------------------------------------
#define VPE_HANDLER_INSTANCE_NUM    (64)
#define VPE_RETURN_ERR 0
#define VPE_RETURN_OK 1
#define VPE_DBG_LV_SC(enLevel)               (gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_SC]&enLevel)
#define VPE_DBG_LV_IQ(enLevel)             (gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_IQ]&enLevel)
#define VPE_DBG_LV_ISP()                (gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_ISP])
#define VPEIRQ_MSK_SC3_ENG_FRM_END      (((u64)1)<<1)
#define VPEIRQ_SC_IN_FRM_END            0
#define VPEIRQ_SC3_ENG_FRM_END          1
#define VPEIRQ_SC3_DMA_W_ACTIVE         33
#define VPEIRQ_SC3_DMA_W_ACTIVE_N       34
#define VPEIRQ_SC_ENG_FRM_END           2
#define VPEIRQ_VSYNC_IDCLK              4
#define VPEIRQ_AFF_FULL                 14
#define SCLIRQ_SC1_FRM_W_ACTIVE         7
#define SCLIRQ_SC1_FRM_W_ACTIVE_N       8
#define VPEIRQ_SC4_HVSP_FINISH          62
#define VPEIRQ_VSYNC_FCLK_LDC           35
#define VPE_IMI_Y_BUFFER_ADDR   0x0
#define VPE_IMI_C_BUFFER_ADDR   0x0
//ne buffer offset
#define NEBUF_SADY          0
#define NEBUF_SUMY          16
#define NEBUF_CNTY          48
#define NEBUF_SADC          64
#define NEBUF_SUMC          80
#define NEBUF_CNTC          112
#define NEBUF_TARGET        128
#define NEBUF_TH_GAIN       130
#define NEBUF_BLK_NUM       132
#define NEBUF_SAD_MIN       133
#define NEBUF_NOISE_MEAN    134
#define NEBUF_LEARN         135
#define NEBUF_YGAIN         136
#define NEBUF_CGAIN         137

//qmap config offset
#define QMAP_CONFIG_MCNR_YTBL       0
#define QMAP_CONFIG_MCNR_CTBL       16
#define QMAP_CONFIG_MCNR_MV_YTBL    32
#define QMAP_CONFIG_MCNR_MV_CTBL    48
#define QMAP_CONFIG_NLM_SHIFT       64
#define QMAP_CONFIG_NLM_MAIN        65
#define QMAP_CONFIG_NLM_WB          81
#define QMAP_CONFIG_XNR             97
#define QMAP_CONFIG_YEE_ETCP        101
#define QMAP_CONFIG_YEE_ETCM        107
#define QMAP_CONFIG_SIZE            113

//qmap onoff offset
#define QMAP_ONOFF_NLM_EN      0
#define QMAP_ONOFF_XNR_EN      1
#define QMAP_ONOFF_YEE_EN      2
#define QMAP_ONOFF_NLM_SHIFT   3
#define QMAP_ONOFF_NLM_MAIN    4
#define QMAP_ONOFF_NLM_WB      20
#define QMAP_ONOFF_WDR_EN      36
#define QMAP_ONOFF_UVM_EN      37
#define QMAP_ONOFF_UVM         38
#define QMAP_ONOFF_SIZE        43

//qmap process offset
#define QMAP_PROCESS_MCNR_YGAIN     0
#define QMAP_PROCESS_MCNR_CGAIN     1
#define QMAP_PROCESS_MCNR_LUMAY     2
#define QMAP_PROCESS_MCNR_LUMAC     10
#define QMAP_PROCESS_MCNR_EN        18  //for video start, need to delay 1 frame to enable mcnr
#define QMAP_PROCESS_WDR_STR        19
#define QMAP_PROCESS_SIZE           21

//qmap process offset
#define QMAP_PROCESS_NE_MEANTHY     0
#define QMAP_PROCESS_NE_MEANTHC     16
#define QMAP_PROCESS_NE_SADMINY     32
#define QMAP_PROCESS_NE_SADMINC     48
#define QMAP_PROCESS_NE_MEANY       64
#define QMAP_PROCESS_NE_MEANC       80
#define QMAP_PROCESS_NE_SIZE        96
#if defined (SCLOS_TYPE_LINUX_KERNEL)

#define VPE_ERR(_fmt, _args...)       printk(KERN_WARNING _fmt, ## _args)
#define VPE_DBGERR(_fmt, _args...)       printk(KERN_DEBUG _fmt, ## _args)
#define VPE_DBG(dbglv, _fmt, _args...)             \
    do                                             \
    if(dbglv)                                      \
    {                                              \
            printk(KERN_DEBUG _fmt, ## _args);       \
    }while(0)

#else
#define VPE_ERR(_fmt, _args...)        UartSendTrace(_fmt,##_args)
#define VPE_DBGERR(_fmt, _args...)
#define VPE_DBG(dbglv, _fmt, _args...)             \
    do                                             \
    if(dbglv)                                      \
    {                                              \
            UartSendTrace(_fmt, ## _args);       \
    }while(0)

#endif

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_SCLHVSP_ID_1 = 0,      ///< ID_1
    E_HAL_SCLHVSP_ID_2,      ///< ID_2
    E_HAL_SCLHVSP_ID_3,      ///< ID_3
    E_HAL_SCLHVSP_ID_4,      ///< ID_4
    E_HAL_SCLDMA_ID_1,      ///< ID_1
    E_HAL_SCLDMA_ID_2,      ///< ID_2
    E_HAL_SCLDMA_ID_3,      ///< ID_3
    E_HAL_SCLDMA_ID_4,      ///< ID_4
    E_HAL_ID_MAX,      ///< ID_4
    E_HAL_M2M_ID = 0,      ///< ID_4
} DrvSclIdType_e;
typedef enum
{
    E_HAL_INIT_DEINIT = 0,
    E_HAL_INIT_BY_PROBE,      ///< ID_1
    E_HAL_INIT_BY_CREATE,      ///< ID_2
    E_HAL_INIT_TYPE,      ///< ID_4
} DrvSclInitType_e;
typedef enum
{
    E_HAL_SCL_INST_SCL = 0,  ///< include I5 SC_BOT
    E_HAL_SCL_INST_M2M,      ///< M2M
    E_HAL_SCL_INST_MAX,      ///< ID_4
    E_HAL_SCL_INST_LDC,      ///< I5_LDC
    E_HAL_SCL_INST_ROT,      ///< I5 SC_TOP
} DrvSclInstType_e;

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    //hw control
    u8 in_reg_blk_sample_step;     //8
    u8 in_reg_blk_meanY_lb;        //16
    u8 in_reg_blk_meanY_ub;        //240
    u8 in_reg_blk_sadY_dc_ub;      //8
    u16 in_reg_blk_sad_ub;         //2560

    //hw output
    u16 in_reg_blk_sad_min_tmp_Y[8];
    u16 in_reg_blk_sad_min_tmp_C[8];
    u32 in_reg_sum_noise_mean_Y[8];
    u32 in_reg_sum_noise_mean_C[8];
    u16 in_reg_count_noise_mean_Y[8];
    u16 in_reg_count_noise_mean_C[8];

    //sw control
    u16 in_blk_sample_num_target;      //32768
    u8 in_noise_mean_th_gain;          //6
    u8 in_inten_blknum_lb;             //16
    u8 in_blk_sad_min_lb;              //32
    u8 in_noise_mean_lb;               //32
    u8 in_learn_rate;                  // 2
    u8 in_dnry_gain;                   // 4
    u8 in_dnrc_gain;                   // 4

    u16 out_reg_noise_mean_th_Y[8];     //192, 192, 192, 192, 192, 192, 192, 192
    u16 out_reg_noise_mean_th_C[8];

    u16 out_reg_blk_sad_min_Y[8];
    u16 out_reg_blk_sad_min_C[8];
    u16 out_reg_noise_mean_Y[8];
    u16 out_reg_noise_mean_C[8];

    //out 3dnr setting
    u8 out_reg_dnry_gain;
    u8 out_reg_dnrc_gain;
    u8 out_reg_lumaLutY[8];
    u8 out_reg_lumaLutC[8];
} NoiseEst_t;

typedef struct
{
    MHalVpeSclCropConfig_t stCropCfg;
    MHalVpeSclWinSize_t stPortSizeCfg[E_MHAL_DMA_PORT_MAX];
    MHalVpeSclWinSize_t stInputCfg;
    MHalPixelFormat_e      enPortFormat[E_MHAL_DMA_PORT_MAX];
    bool bEn[E_MHAL_DMA_PORT_MAX];
    bool bSet[E_MHAL_DMA_PORT_MAX];
    bool bEnImi;
}MHalSclCtxConfig_t;
typedef struct
{
    bool bUsed;
    s32 s32Handle[E_HAL_ID_MAX];
    u16 u16InstId;
    MHalSclCtxConfig_t stCtx;
	MHhalVpeSclInstance_e eInst;
	MHalVpeTopInputMode_e enTopIn;
	MHalSclInputPortMode_e eInMode;
	MHalSclOutputMode_e eOutMode;
} MHalSclHandleConfig_t;
typedef struct
{
    bool bUsed;
    s32 s32Handle;
    u16 u16InstId;
    MHalVpeIqWdrRoiReport_t stWdrBuffer;
    void *pvNrBuffer;
    u32 u32BaseAddr[WDR_HIST_BUFFER];  //for wdr roi
    u32 u32VirBaseAddr[WDR_HIST_BUFFER]; // for wdr roi
    u32 u32DlcHistAddr; // for dlc hist
    u32 u32VirDlcHistAddr; // for dlc hist
    bool bEsEn;
    bool bNREn;
    u8 pu8UVM[4];
    u8 u8NLMShift_ESOff;
    u8 pu8NLM_ESOff[16];
    u8 u8NLMShift_ESOn;
    u8 pu8NLM_ESOn[16];
    bool bProcess;
    u16 pu16NE_blk_sad_min_Y[8];
    u16 pu16NE_blk_sad_min_C[8];
    u16 pu16NE_noise_mean_Y[8];
    u16 pu16NE_noise_mean_C[8];
    bool bWDREn;
    bool bWDRActive;
    u8 u8Wdr_Str;
    u8 u8Wdr_Slope;
    bool bNrFBAllocate;
	MHalVpeTopInputMode_e eMode;
} MHalIqHandleConfig_t;
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MHalSclHandleConfig_t *pgstScHandler[VPE_HANDLER_INSTANCE_NUM];
MHalIqHandleConfig_t *pgstIqHandler[VPE_HANDLER_INSTANCE_NUM];
bool gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE] = {E_MHAL_VPE_DEBUG_OFF,E_MHAL_VPE_DEBUG_OFF,E_MHAL_VPE_DEBUG_OFF};
char KEY_DMEM_IQ_HIST[20] = "IQROIDLC";
DrvSclInitType_e genScinit = E_HAL_INIT_DEINIT;
MHalVpeIrqEnableMode_e genIrqmode[E_HAL_SCL_INST_MAX] = {E_MHAL_SCL_IRQ_OFF,E_MHAL_SCL_IRQ_OFF};
bool gbHistints = 0;

#if defined(USE_USBCAM)
MhalInterruptCb       _gpMahlIntCb = NULL;
#endif

//-------------------------------------------------------------------------------------------------
//  Functions ISP Internal Calc
//-------------------------------------------------------------------------------------------------
extern void _DrvSclVpeModuleDeInit(void);
extern void _DrvSclVpeModuleInit(void);

extern void _DrvSclIrqSetIsBlankingRegion(bool bBk);
extern bool DrvSclIrqGetIsBlankingRegion(void);
void MHalVpeSetbHist(bool bEn)
{
    gbHistints = bEn;
}
bool MHalVpeGetbHist(void)
{
    return gbHistints;
}

void _IQNoiseEstimationDriver(u16 *reg_blk_sad_min_tmp, u32 *reg_sum_noise_mean, u16 *reg_count_noise_mean, NoiseEst_t _reg, u16 *blk_sad_min, u16 *noise_mean, u16 *reg_noise_mean_th)
{
    u16 *blkSADMin = reg_blk_sad_min_tmp;
    u32 *sum_noiseMean = reg_sum_noise_mean;
    u16 *count_noiseMean = reg_count_noise_mean;
    int noiseMean[8];
    int noise_mean_left = 0 , noise_mean_right = 0;
    int delta;
    int i, j;

    //-------------------------------------------------
    // Init estimation (blkSADMin, noiseMean)
    //-------------------------------------------------
    for (i = 0; i < 8; i++)
    {
        // blkSAD_min
        blkSADMin[i] = max(blkSADMin[i], (u16)_reg.in_blk_sad_min_lb);

        // noiseMean
        if (count_noiseMean[i] > _reg.in_inten_blknum_lb)
            noiseMean[i] = sum_noiseMean[i] / max((int)count_noiseMean[i], 1);
        else
            noiseMean[i] = 0;
    }

    //-------------------------------------------------
    // Filling missing segment (noiseMean)
    //-------------------------------------------------
    if (noiseMean[0] == 0)
    {
        for (i = 1; i < 8; i++)
        {
            if (noiseMean[i] != 0)
            {
                noiseMean[0] = noiseMean[i];
                break;
            }
            else if ((i == 7) && (noiseMean[7] == 0))
            {
                noiseMean[0] = _reg.in_noise_mean_lb;
            }
        }
    }

    if (noiseMean[7] == 0)
    {
        for (i = 6; i >= 0; i--)
        {
            if (noiseMean[i] != 0)
            {
                noiseMean[7] = noiseMean[i];
                break;
            }
        }
    }

    for (i = 1; i < 7; i++)
    {
        if (noiseMean[i] == 0)
        {
            for (j = i - 1; j >= 0; j--)
            {
                if (noiseMean[j] != 0)
                {
                    noise_mean_left = noiseMean[j];
                    break;
                }
            }

            for (j = i + 1; j < 8; j++)
            {
                if (noiseMean[j] != 0)
                {
                    noise_mean_right = noiseMean[j];
                    break;
                }
            }

            noiseMean[i] = (noise_mean_left + noise_mean_right) >> 1;
        }
    }

    //--------------------------------------------
    // Noise profile stabilization and output
    //--------------------------------------------
    for (i = 0; i < 8; i++)
    {
        // blkSAD_min
        delta = blkSADMin[i] - blk_sad_min[i];

        if (delta < 0)
            blk_sad_min[i] = blk_sad_min[i] - ((_reg.in_learn_rate * abs(delta)) >> 2);
        else
            blk_sad_min[i] = blk_sad_min[i] + ((_reg.in_learn_rate * delta) >> 2);

        // noiseMean
        delta = noiseMean[i] - noise_mean[i];

        if (delta < 0)
            noise_mean[i] = noise_mean[i] - ((_reg.in_learn_rate * abs(delta)) >> 2);
        else
            noise_mean[i] = noise_mean[i] + ((_reg.in_learn_rate * delta) >> 2);
    }

    //--------------------------------------------
    // Calculate "noiseMean_Th"
    //--------------------------------------------
    for (i = 0; i < 8; i++)
        reg_noise_mean_th[i] = blk_sad_min[i] * _reg.in_noise_mean_th_gain;
}

void _IQCalcNRDnrGainAndLumaLut(u16 *reg_noise_mean_Y, u16 *reg_noise_mean_C, u8 reg_ne_dnry_gain, u8 reg_ne_dnrc_gain, u8 *reg_nr_dnry_gain, u8 *reg_nr_dnrc_gain, u8 *reg_lumaLutY, u8 *reg_lumaLutC)
{
    int noise_level_min_Y = reg_noise_mean_Y[0];
    int noise_level_min_C = reg_noise_mean_C[0];
    int i, a, b;

    //--- Calculate nr_dnry_gain ---
    for (i = 1; i < 8; i++)
    {
        if (reg_noise_mean_Y[i] < noise_level_min_Y)
            noise_level_min_Y = reg_noise_mean_Y[i];

        if (reg_noise_mean_C[i] < noise_level_min_C)
            noise_level_min_C = reg_noise_mean_C[i];
    }

    a = ((reg_ne_dnry_gain << 12) + (noise_level_min_Y >> 1)) / max((int)noise_level_min_Y, 1);
    b = ((reg_ne_dnrc_gain << 12) + (noise_level_min_C >> 1)) / max((int)noise_level_min_C, 1);
    *reg_nr_dnry_gain = minmax(a, 0, 255);
    *reg_nr_dnrc_gain = minmax(b, 0, 255);

    //--- Calculate lumaLut ---
    for (i = 0; i < 8; i++)
    {
        reg_lumaLutY[i] = ((noise_level_min_Y << 4) + (reg_noise_mean_Y[i] >> 1)) / max((int)reg_noise_mean_Y[i], 1);
        reg_lumaLutC[i] = ((noise_level_min_C << 4) + (reg_noise_mean_C[i] >> 1)) / max((int)reg_noise_mean_C[i], 1);
    }
}

void _IQSyncESSetting(void *pCtx)
{
    int i=0;
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler = pCtx;

    //use nlm setting to do edge smooth effect
    if (pstIqHandler->bEsEn) {
        if (pstIqHandler->bNREn) {
            pstIqHandler->u8NLMShift_ESOn = minmax(pstIqHandler->u8NLMShift_ESOff, 3, 15);
            for (i=0; i<16; i++) {
                pstIqHandler->pu8NLM_ESOn[i] = (pstIqHandler->pu8NLM_ESOff[i]>4) ? (pstIqHandler->pu8NLM_ESOff[i]): (4);
            }
        }
        else {
            pstIqHandler->u8NLMShift_ESOn = 3;
            for (i=0; i<16; i++) {
                pstIqHandler->pu8NLM_ESOn[i] = 4;
            }
        }
    }

    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);
}

void _IQBufHLInverse(u8 *pSrc, u16 size)
{
    int i;
    u8 tmp;

    for (i=0; i<size; i+=2)
    {
        tmp = pSrc[i+0];
        pSrc[i+0] = pSrc[i+1];
        pSrc[i+1] = tmp;
    }
}
//-------------------------------------------------------------------------------------------------
//  VPE local function
//-------------------------------------------------------------------------------------------------
void _MHalVpeCleanScInstBuffer(MHalSclHandleConfig_t *pstScHandler)
{
    s16 s16Idx = -1;
    s16 i ;
    //clean buffer
    s16Idx = pstScHandler->u16InstId;
    pstScHandler->bUsed = 0;
    pstScHandler->u16InstId = 0;
    for(i = 0; i < E_HAL_ID_MAX; i++)
    {
        pstScHandler->s32Handle[i] = 0;
    }
    DrvSclOsMemFree(pstScHandler);
    pgstScHandler[s16Idx] = NULL;
}
void _MHalVpeIqMemNaming(char *pstIqName,s16 s16Idx)
{
    pstIqName[0] = 48+(((s16Idx&0xFFF)%1000)/100);
    pstIqName[1] = 48+(((s16Idx&0xFFF)%100)/10);
    pstIqName[2] = 48+(((s16Idx&0xFFF)%10));
    pstIqName[3] = '_';
    pstIqName[4] = '\0';
    DrvSclOsStrcat(pstIqName,KEY_DMEM_IQ_HIST);
}
void _MHalVpeCleanIqInstBuffer(MHalIqHandleConfig_t *pstIqHandler)
{
    s16 s16Idx = -1;
    u16 i;
    u16 u16Size = 0;
    char sg_Iq_Roi_name[16];
    //clean buffer
    u16Size = DRV_SCLVIP_IO_WDR_HIST1_BUFFER_SIZE + DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE*3 +DRV_SCLVIP_IO_DLC_HIST_SIZE;
    if(u16Size%0x1000)
    {
        u16Size = ((u16Size/0x1000)+1)*0x1000;
    }
    s16Idx = pstIqHandler->u16InstId;
    pstIqHandler->bUsed = 0;
    pstIqHandler->u16InstId = 0;
    pstIqHandler->s32Handle = 0;
    DrvSclOsMemFree(pstIqHandler->pvNrBuffer);
    pstIqHandler->pvNrBuffer = NULL;
    _MHalVpeIqMemNaming(sg_Iq_Roi_name,s16Idx);
    DrvSclOsDirectMemFree
        (sg_Iq_Roi_name,u16Size,(void *)pgstIqHandler[s16Idx]->u32VirBaseAddr[0],pgstIqHandler[s16Idx]->u32BaseAddr[0]);
    for(i=0;i<ROI_WINDOW_MAX;i++)
    {
        pgstIqHandler[s16Idx]->u32BaseAddr[i] = 0;
        pgstIqHandler[s16Idx]->u32VirBaseAddr[i] = 0;
        pstIqHandler->stWdrBuffer.u32Y[i] =0;
    }
    DrvSclOsMemFree(pstIqHandler);
    pgstIqHandler[s16Idx] = NULL;
}
void _MHalVpeIqKeepCmdqFunction(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    _DrvSclVipIoKeepCmdqFunction(pstCmdQInfo);
}
void _MHalVpeSclKeepCmdqFunction(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    _DrvSclDmaIoKeepCmdqFunction(pstCmdQInfo);
}
void _MHalVpeM2MKeepCmdqFunction(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    _DrvM2MIoKeepCmdqFunction(pstCmdQInfo);
}
void _MHalVpeKeepAllocFunction(const MHalAllocPhyMem_t *pstAlloc)
{
    DrvSclOsAllocPhyMem_t pf;
    DrvSclOsMemset(&pf,0,sizeof(DrvSclOsAllocPhyMem_t));
    if(pstAlloc)
    {
        pf.SclOsAlloc = (void *)pstAlloc->alloc;
        pf.SclOsFree = (void *)pstAlloc->free;
        pf.SclOsmap = (void *)pstAlloc->map;
        pf.SclOsunmap = (void *)pstAlloc->unmap;
        pf.SclOsflush_cache= (void *)pstAlloc->flush_cache;
        DrvSclOsKeepAllocFunction(&pf);
    }
    else
    {
        pf.SclOsAlloc = NULL;
        pf.SclOsFree = NULL;
        pf.SclOsmap = NULL;
        pf.SclOsunmap = NULL;
        pf.SclOsflush_cache= NULL;
        DrvSclOsKeepAllocFunction(&pf);
    }
}
bool _MHalVpeReqmemConfig(s32 s32Handle,const MHalVpeSclWinSize_t *stMaxWin)
{
    //DrvSclHvspIoReqMemConfig_t stReqMemCfg;
    DrvSclVipIoReqMemConfig_t stReqMemCfg;
    //DrvSclOsMemset(&stReqMemCfg,0,sizeof(DrvSclHvspIoReqMemConfig_t));
    DrvSclOsMemset(&stReqMemCfg,0,sizeof(DrvSclVipIoReqMemConfig_t));
    stReqMemCfg.u16Pitch   = stMaxWin->u16Width;
    stReqMemCfg.u16Vsize   = stMaxWin->u16Height;
    stReqMemCfg.u32MemSize = (stMaxWin->u16Width * stMaxWin->u16Height * 4);
    stReqMemCfg.enCeType = E_DRV_SCLVIP_IO_UCMCE_8;
    FILL_VERCHK_TYPE(stReqMemCfg, stReqMemCfg.VerChk_Version, stReqMemCfg.VerChk_Size, DRV_SCLHVSP_VERSION);
    //if(_DrvSclHvspIoReqmemConfig(s32Handle,&stReqMemCfg))
    if(_DrvSclVipIoReqmemConfig(s32Handle,&stReqMemCfg))
    {
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
DrvSclHvspIoColorType_e _MHalVpeTranHvspColorFormat(MHalPixelFormat_e enType)
{
    DrvSclHvspIoColorType_e enColor;
    switch(enType)
    {
        case E_MHAL_PIXEL_FRAME_YUV422_YUYV:
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422:
            enColor = E_DRV_SCLHVSP_IO_COLOR_YUV422;
            break;
        case E_MHAL_PIXEL_FRAME_ARGB8888:
            enColor = E_DRV_SCLHVSP_IO_COLOR_RGB;
            break;
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        case E_MHAL_PIXEL_FRAME_YUV_MST_420:
            enColor = E_DRV_SCLHVSP_IO_COLOR_YUV420;
            break;
        default :
            enColor = E_DRV_SCLHVSP_IO_COLOR_NUM;
            break;
    }
    return enColor;
}
DrvSclHvspIoColorType_e _MHalVpeTranMdwinColorFormat(MHalPixelFormat_e enType,MHalPixelCompressMode_e enCompress)
{
    DrvSclHvspIoColorType_e enColor;
    switch(enType)
    {
        case E_MHAL_PIXEL_FRAME_YUV422_YUYV:
            if(enCompress==E_MHAL_COMPRESS_MODE_NONE)
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV422;
            }
            else
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV422CE;
            }
            break;
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422:
            if(enCompress==E_MHAL_COMPRESS_MODE_NONE)
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV422;
            }
            else
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV422CE;
            }
            break;
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            if(enCompress==E_MHAL_COMPRESS_MODE_NONE)
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV420;
            }
            else
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV420CE;
            }
            break;
        case E_MHAL_PIXEL_FRAME_YUV_MST_420:
            if(enCompress==E_MHAL_COMPRESS_MODE_NONE)
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV420;
            }
            else
            {
                enColor = E_DRV_MDWIN_IO_COLOR_YUV420CE;
            }
            break;
        case E_MHAL_PIXEL_FRAME_ARGB8888:
                enColor = E_DRV_MDWIN_IO_COLOR_ARGB;
            break;
        case E_MHAL_PIXEL_FRAME_ABGR8888:
                enColor = E_DRV_MDWIN_IO_COLOR_ABGR;
            break;
        default :
            enColor = E_DRV_SCLDMA_IO_COLOR_NUM;
            break;
    }
    return enColor;
}
DrvSclHvspIoColorType_e _MHalVpeTranDmaColorFormat(MHalPixelFormat_e enType,MHalPixelCompressMode_e enCompress)
{
    DrvSclHvspIoColorType_e enColor;
    switch(enType)
    {
        case E_MHAL_PIXEL_FRAME_YUV422_YUYV:
            enColor = E_DRV_SCLDMA_IO_COLOR_YUV422;
            break;
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422:
            enColor = E_DRV_SCLDMA_IO_COLOR_YCSep422;
            break;
        case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            enColor = E_DRV_SCLDMA_IO_COLOR_YUV420;
            break;
        default :
            enColor = E_DRV_SCLDMA_IO_COLOR_NUM;
            break;
    }
    return enColor;
}
bool _MHalVpeScIsInputSizeNeedReSetting(MHalSclHandleConfig_t* pCtx ,const MHalVpeSclInputSizeConfig_t *pCfg)
{
    if(((pCfg->u16Height != pCtx->stCtx.stInputCfg.u16Height)&& pCfg->u16Height) ||
        ((pCfg->u16Width != pCtx->stCtx.stInputCfg.u16Width)&& pCfg->u16Width))
    {
        return VPE_RETURN_OK;
    }
    return VPE_RETURN_ERR;
}
bool _MHalVpeScIsOutputSizeNeedReSetting(MHalSclHandleConfig_t* pCtx ,const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    if(((pCfg->u16Height != pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Height)&&
        pCfg->u16Height) ||
        ((pCfg->u16Width != pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width)&&
        pCfg->u16Width))
    {
        return VPE_RETURN_OK;
    }
    return VPE_RETURN_ERR;
}
bool _MHalVpeScIsCropSizeNeedReSetting(MHalSclHandleConfig_t* pCtx ,const MHalVpeSclCropConfig_t *pCfg)
{
    if(((pCfg->stCropWin.u16Height != pCtx->stCtx.stCropCfg.stCropWin.u16Height)&&pCfg->stCropWin.u16Height) ||
        ((pCfg->stCropWin.u16Width != pCtx->stCtx.stCropCfg.stCropWin.u16Width)&&pCfg->stCropWin.u16Width)||
        (pCfg->stCropWin.u16X != pCtx->stCtx.stCropCfg.stCropWin.u16X)||
        (pCfg->stCropWin.u16Y != pCtx->stCtx.stCropCfg.stCropWin.u16Y)||
        (pCfg->bCropEn != pCtx->stCtx.stCropCfg.bCropEn))
    {
        return VPE_RETURN_OK;
    }
    return VPE_RETURN_ERR;
}
void _MHalVpeScInputSizeReSetting(MHalSclHandleConfig_t* pCtx ,u32 u32Width,u32 u32Height)
{
    pCtx->stCtx.stInputCfg.u16Height = u32Height;
    pCtx->stCtx.stInputCfg.u16Width = u32Width;
    if(pCtx->stCtx.stCropCfg.bCropEn)
    {
        if(pCtx->stCtx.stCropCfg.stCropWin.u16Height >= u32Height)
        {
            pCtx->stCtx.stCropCfg.stCropWin.u16Height = u32Height;
        }
        if(pCtx->stCtx.stCropCfg.stCropWin.u16Width >= u32Width)
        {
            pCtx->stCtx.stCropCfg.stCropWin.u16Width = u32Width;
        }
        if(pCtx->stCtx.stCropCfg.stCropWin.u16X +pCtx->stCtx.stCropCfg.stCropWin.u16Width > u32Width)
        {
            pCtx->stCtx.stCropCfg.stCropWin.u16X = 0;
        }
        if(pCtx->stCtx.stCropCfg.stCropWin.u16Y +pCtx->stCtx.stCropCfg.stCropWin.u16Height> u32Height)
        {
            pCtx->stCtx.stCropCfg.stCropWin.u16Y = 0;
        }
        if(pCtx->stCtx.stCropCfg.stCropWin.u16Width == u32Width && pCtx->stCtx.stCropCfg.stCropWin.u16Height == u32Height)
        {
            pCtx->stCtx.stCropCfg.bCropEn = 0;
        }
    }
    else
    {
        pCtx->stCtx.stCropCfg.stCropWin.u16Height = u32Height;
        pCtx->stCtx.stCropCfg.stCropWin.u16Width = u32Width;
    }
}
void _MHalVpeScCropSizeReSetting(MHalSclHandleConfig_t* pCtx ,const MHalVpeSclCropConfig_t *stCropCfg)
{
    if(stCropCfg->stCropWin.u16Height)
    {
        pCtx->stCtx.stCropCfg.stCropWin.u16Height = stCropCfg->stCropWin.u16Height;
    }
    if(stCropCfg->stCropWin.u16Width)
    {
        pCtx->stCtx.stCropCfg.stCropWin.u16Width = stCropCfg->stCropWin.u16Width;
    }
    if(pCtx->stCtx.stCropCfg.stCropWin.u16Width%2)
    {
        pCtx->stCtx.stCropCfg.stCropWin.u16Width--;
    }
    pCtx->stCtx.stCropCfg.stCropWin.u16X = stCropCfg->stCropWin.u16X;
    if(pCtx->stCtx.stCropCfg.stCropWin.u16X%2)
    {
        pCtx->stCtx.stCropCfg.stCropWin.u16X++;
    }
    pCtx->stCtx.stCropCfg.stCropWin.u16Y = stCropCfg->stCropWin.u16Y;
    pCtx->stCtx.stCropCfg.bCropEn= stCropCfg->bCropEn;
}
void _MHalVpeScOutputSizeReSetting(MHalSclHandleConfig_t* pCtx, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Height = pCfg->u16Height;
    pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width = pCfg->u16Width;
}
void _MHalVpeFillHVSP1ScaleCfg(MHalSclHandleConfig_t* pCtx,DrvSclHvspIoScalingConfig_t *pstCfg)
{
    pstCfg->u16Src_Width = pCtx->stCtx.stInputCfg.u16Width;
    pstCfg->u16Src_Height = pCtx->stCtx.stInputCfg.u16Height;
    pstCfg->u16Dsp_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Width;
    pstCfg->u16Dsp_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Height;
    pstCfg->bCropEn = pCtx->stCtx.stCropCfg.bCropEn;
    pstCfg->stCropWin.u16X = pCtx->stCtx.stCropCfg.stCropWin.u16X;
    pstCfg->stCropWin.u16Y = pCtx->stCtx.stCropCfg.stCropWin.u16Y;
    pstCfg->stCropWin.u16Width = pCtx->stCtx.stCropCfg.stCropWin.u16Width;
    pstCfg->stCropWin.u16Height = pCtx->stCtx.stCropCfg.stCropWin.u16Height;
    if(((pstCfg->u16Src_Width != pstCfg->stCropWin.u16Width)||
        (pstCfg->u16Src_Height != pstCfg->stCropWin.u16Height))&&!pstCfg->bCropEn)
    {
        pstCfg->bCropEn = 1;
    }
}
void _MHalVpeFillHVSP2ScaleCfg(MHalSclHandleConfig_t* pCtx,DrvSclHvspIoScalingConfig_t *pstCfg)
{
    pstCfg->u16Src_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Width;
    pstCfg->u16Src_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Height;
    pstCfg->u16Dsp_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Width;
    pstCfg->u16Dsp_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Height;
}
void _MHalVpeFillHVSP3ScaleCfg(MHalSclHandleConfig_t* pCtx,DrvSclHvspIoScalingConfig_t *pstCfg)
{
    MHalVpeDmaPort_e enSrc = E_MHAL_SCL_OUTPUT_PORT0;
    MHalVpeDmaPort_e enDsp = E_MHAL_SCL_OUTPUT_PORT2;
    if(pCtx->eInst == E_MHAL_VPE_SCL_SUB && (pCtx->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        enSrc = E_MHAL_M2M_INPUT_PORT;
        enDsp = E_MHAL_SCL_OUTPUT_PORT2;
    }
    else if(pCtx->eOutMode & E_MHAL_SCL_OUTPUT_MODE_SCALING_2LEVEL_MODE &&(pCtx->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        enSrc = E_MHAL_SCL_OUTPUT_PORT0;
        enDsp = E_MHAL_SCL_OUTPUT_PORT2;
    }
    else if(pCtx->eOutMode & E_MHAL_SCL_OUTPUT_MODE_SCALING_3LEVEL_MODE &&(pCtx->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        enSrc = E_MHAL_SCL_OUTPUT_PORT1;
        enDsp = E_MHAL_SCL_OUTPUT_PORT2;
    }
    pstCfg->u16Src_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Width;
    pstCfg->u16Src_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Height;
    pstCfg->u16Dsp_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Width;
    pstCfg->u16Dsp_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Height;
}
void _MHalVpeFillHVSP4ScaleCfg(MHalSclHandleConfig_t* pCtx,DrvSclHvspIoScalingConfig_t *pstCfg)
{
    pstCfg->u16Src_Width = pCtx->stCtx.stCropCfg.stCropWin.u16Width;
    pstCfg->u16Src_Height = pCtx->stCtx.stCropCfg.stCropWin.u16Height;
    pstCfg->u16Dsp_Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Width;
    pstCfg->u16Dsp_Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Height;
}
void _MHalVpeFillHVSPScaleCfg(DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx,DrvSclHvspIoScalingConfig_t *pstCfg)
{
    switch(enId)
    {
        case E_HAL_SCLHVSP_ID_1:
            _MHalVpeFillHVSP1ScaleCfg(pCtx,pstCfg);
            break;
        case E_HAL_SCLHVSP_ID_2:
            _MHalVpeFillHVSP2ScaleCfg(pCtx,pstCfg);
            break;
        case E_HAL_SCLHVSP_ID_3:
            _MHalVpeFillHVSP3ScaleCfg(pCtx,pstCfg);
            break;
        case E_HAL_SCLHVSP_ID_4:
            _MHalVpeFillHVSP4ScaleCfg(pCtx,pstCfg);
            break;
        default:
            VPE_ERR("[VPESCL]%s ID Not Support\n",__FUNCTION__);
            break;
    }
}
bool _MHalVpeSclbSetting(DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx)
{
    bool bSet = 1;
    switch(enId)
    {
        case E_HAL_SCLHVSP_ID_1:
        case E_HAL_SCLDMA_ID_1:
            if(!(pCtx->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT0))
            {
                bSet = 0;
            }
        break;
        case E_HAL_SCLHVSP_ID_2:
        case E_HAL_SCLDMA_ID_2:
            if(!(pCtx->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT1))
            {
                bSet = 0;
            }
        break;
        case E_HAL_SCLHVSP_ID_3:
        case E_HAL_SCLDMA_ID_3:
            if(!(pCtx->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT2))
            {
                bSet = 0;
            }
        break;
        case E_HAL_SCLHVSP_ID_4:
        case E_HAL_SCLDMA_ID_4:
            if(!(pCtx->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT3))
            {
                bSet = 0;
            }
        break;
        default:
        break;
    }
    return bSet;
}

void _MHalVpeScScalingConfig(DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx)
{
    DrvSclHvspIoScalingConfig_t stHvspScaleCfg;
    DrvSclOsMemset(&stHvspScaleCfg,0,sizeof(DrvSclHvspIoScalingConfig_t));
    _MHalVpeFillHVSPScaleCfg(enId,pCtx,&stHvspScaleCfg);
    stHvspScaleCfg = FILL_VERCHK_TYPE(stHvspScaleCfg, stHvspScaleCfg.VerChk_Version,
        stHvspScaleCfg.VerChk_Size,DRV_SCLHVSP_VERSION);
    if(_MHalVpeSclbSetting(enId,pCtx))
    {
        _DrvSclHvspIoSetScalingConfig(pCtx->s32Handle[enId], &stHvspScaleCfg);
    }
}
void _MHalVpeSclOutputPort0SizeConfig(void *pCtx)
{
    MHalSclHandleConfig_t *pstScHandler;
    pstScHandler = pCtx;
    _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_1,pCtx);
    if((pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT1))
    {
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_2,pCtx);
    }
    if((pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2) &&
        (pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_SCALING_2LEVEL_MODE))
    {
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_3,pCtx);
    }
}
void _MHalVpeSclOutputPort1SizeConfig(void *pCtx)
{
    MHalSclHandleConfig_t *pstScHandler;
    pstScHandler = pCtx;
    _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_2,pCtx);
    if((pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2) &&
        (pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_SCALING_3LEVEL_MODE))
    {
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_3,pCtx);
    }
}
void _MHalVpeSclOutputPort2SizeConfig(void *pCtx)
{
    _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_3,pCtx);
}
void _MHalVpeSclOutputPort3SizeConfig(void *pCtx)
{
    _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_4,pCtx);
}
void _MHalVpeSclOutputSizeConfig(void *pCtx, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    switch(pCfg->enOutPort)
    {
        case E_MHAL_SCL_OUTPUT_PORT0:
            _MHalVpeSclOutputPort0SizeConfig(pCtx);
            break;
        case E_MHAL_SCL_OUTPUT_PORT1:
            _MHalVpeSclOutputPort1SizeConfig(pCtx);
            break;
        case E_MHAL_SCL_OUTPUT_PORT2:
            _MHalVpeSclOutputPort2SizeConfig(pCtx);
            break;
        case E_MHAL_SCL_OUTPUT_PORT3:
            _MHalVpeSclOutputPort3SizeConfig(pCtx);
            break;
        default:
            VPE_ERR("[VPESCL]%s PORT Not Support\n",__FUNCTION__);
            break;
    }
}
bool _MHalVpeDmaImiConfig
    (DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx, const MHalVpeSclOutputIMIConfig_t *pCfg)
{
    DrvSclDmaIoBufferConfig_t stSCLDMACfg;
    DrvSclDmaIoProcessConfig_t stTrig;
    bool bRet = VPE_RETURN_OK;
    DrvSclOsMemset(&stSCLDMACfg,0,sizeof(DrvSclDmaIoBufferConfig_t));
    DrvSclOsMemset(&stTrig,0,sizeof(DrvSclDmaIoProcessConfig_t));
    stSCLDMACfg.enBufMDType = E_DRV_SCLDMA_IO_BUFFER_MD_RING;
    stSCLDMACfg.enMemType   = E_DRV_SCLDMA_IO_MEM_IMI;
    stSCLDMACfg.enColorType = _MHalVpeTranDmaColorFormat(pCfg->enFormat,E_MHAL_COMPRESS_MODE_NONE);
    stSCLDMACfg.u16Height   = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Height;
    stSCLDMACfg.u16Width    = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width;
    stSCLDMACfg.u16BufNum    = 1;
    stSCLDMACfg.u8Flag = 0;
    stSCLDMACfg.bHFlip = 0;
    stSCLDMACfg.bVFlip = 0;
    stTrig.stCfg.stBufferInfo.u64PhyAddr[0] = VPE_IMI_Y_BUFFER_ADDR;
    stTrig.stCfg.stBufferInfo.u64PhyAddr[1] = VPE_IMI_C_BUFFER_ADDR;
    stTrig.stCfg.bEn = pCfg->bEn;
    pCtx->stCtx.bEnImi = pCfg->bEn;
    stTrig.stCfg.enMemType = E_DRV_SCLDMA_IO_MEM_IMI;
    if(stSCLDMACfg.enColorType != E_DRV_SCLDMA_IO_COLOR_YUV422)
    {
        stTrig.stCfg.stBufferInfo.u32Stride[0] = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width;
        stTrig.stCfg.stBufferInfo.u32Stride[1] = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width;
    }
    else
    {
        stTrig.stCfg.stBufferInfo.u32Stride[0] = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width*2;
        stTrig.stCfg.stBufferInfo.u32Stride[1] = pCtx->stCtx.stPortSizeCfg[pCfg->enOutPort].u16Width*2;
    }
    if(stSCLDMACfg.enColorType ==E_DRV_SCLDMA_IO_COLOR_NUM)
    {
        VPE_ERR("[VPESCL]%s COLOR Not Support\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    else
    {
        stSCLDMACfg = FILL_VERCHK_TYPE(stSCLDMACfg, stSCLDMACfg.VerChk_Version, stSCLDMACfg.VerChk_Size,DRV_SCLDMA_VERSION);
        if(_MHalVpeSclbSetting(enId,pCtx))
        {
            _DrvSclDmaIoSetOutBufferConfig(pCtx->s32Handle[enId], &stSCLDMACfg);
        }
        if(_MHalVpeSclbSetting(enId,pCtx))
        {
            _DrvSclDmaIoInstProcess(pCtx->s32Handle[enId],&stTrig);
        }
    }
    return bRet;
}
bool _MHalVpeDmaConfig
    (DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx, const MHalVpeSclDmaConfig_t *pCfg)
{
    DrvSclDmaIoBufferConfig_t stSCLDMACfg;
    bool bRet = VPE_RETURN_OK;
    DrvSclOsMemset(&stSCLDMACfg,0,sizeof(DrvSclDmaIoBufferConfig_t));
    stSCLDMACfg.enBufMDType = E_DRV_SCLDMA_IO_BUFFER_MD_SINGLE;
    stSCLDMACfg.enMemType   = E_DRV_SCLDMA_IO_MEM_FRM;
    if(pCfg->enPort!=E_MHAL_SCL_OUTPUT_PORT3)
    {
        stSCLDMACfg.enColorType = _MHalVpeTranDmaColorFormat(pCfg->enFormat,pCfg->enCompress);
    }
    else
    {
        stSCLDMACfg.enColorType = _MHalVpeTranMdwinColorFormat(pCfg->enFormat,pCfg->enCompress);
    }
    stSCLDMACfg.u16Height   = pCtx->stCtx.stPortSizeCfg[pCfg->enPort].u16Height;
    stSCLDMACfg.u16Width    = pCtx->stCtx.stPortSizeCfg[pCfg->enPort].u16Width;
    stSCLDMACfg.u16BufNum    = 1;
    stSCLDMACfg.u8Flag = 0;
    stSCLDMACfg.bHFlip = 0;
    stSCLDMACfg.bVFlip = 0;
    if(pCfg->enCompress && pCfg->enPort!=E_MHAL_SCL_OUTPUT_PORT3)
    {
        VPE_ERR("[VPESCL]%s Compress Not Support\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }

    if(stSCLDMACfg.enColorType ==E_DRV_SCLDMA_IO_COLOR_NUM)
    {
        VPE_ERR("[VPESCL]%s COLOR Not Support\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    else
    {
        stSCLDMACfg = FILL_VERCHK_TYPE(stSCLDMACfg, stSCLDMACfg.VerChk_Version, stSCLDMACfg.VerChk_Size,DRV_SCLDMA_VERSION);
        if(_MHalVpeSclbSetting(enId,pCtx))
        {
            _DrvSclDmaIoSetOutBufferConfig(pCtx->s32Handle[enId], &stSCLDMACfg);
        }
    }
    return bRet;
}
bool _MHalVpeDmaInputConfig
    (DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx, const MHalVpeSclDmaConfig_t *pCfg)
{
    DrvSclDmaIoBufferConfig_t stSCLDMACfg;
    bool bRet = VPE_RETURN_OK;
    DrvSclOsMemset(&stSCLDMACfg,0,sizeof(DrvSclDmaIoBufferConfig_t));
    stSCLDMACfg.enBufMDType = E_DRV_SCLDMA_IO_BUFFER_MD_SINGLE;
    stSCLDMACfg.enMemType   = E_DRV_SCLDMA_IO_MEM_FRMR;
    if(pCfg->enPort!=E_MHAL_SCL_OUTPUT_PORT3)
    {
        stSCLDMACfg.enColorType = _MHalVpeTranDmaColorFormat(pCfg->enFormat,pCfg->enCompress);
    }
    else
    {
        stSCLDMACfg.enColorType = _MHalVpeTranMdwinColorFormat(pCfg->enFormat,pCfg->enCompress);
    }
    stSCLDMACfg.u16Height   = pCtx->stCtx.stPortSizeCfg[pCfg->enPort].u16Height;
    stSCLDMACfg.u16Width    = pCtx->stCtx.stPortSizeCfg[pCfg->enPort].u16Width;
    stSCLDMACfg.u16BufNum    = 1;
    stSCLDMACfg.u8Flag = 0;
    stSCLDMACfg.bHFlip = 0;
    stSCLDMACfg.bVFlip = 0;
    if(pCfg->enCompress && pCfg->enPort!=E_MHAL_SCL_OUTPUT_PORT3)
    {
        VPE_ERR("[VPESCL]%s Compress Not Support\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }

    if(stSCLDMACfg.enColorType ==E_DRV_SCLDMA_IO_COLOR_NUM)
    {
        VPE_ERR("[VPESCL]%s COLOR Not Support\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    else
    {
        stSCLDMACfg = FILL_VERCHK_TYPE(stSCLDMACfg, stSCLDMACfg.VerChk_Version, stSCLDMACfg.VerChk_Size,DRV_SCLDMA_VERSION);
        if(_MHalVpeSclbSetting(enId,pCtx))
        {
            _DrvSclDmaIoSetInBufferConfig(pCtx->s32Handle[enId], &stSCLDMACfg);
        }
    }
    return bRet;
}
bool _MHalVpeHvspInConfig
    (DrvSclIdType_e enId,MHalSclHandleConfig_t* pCtx, const MHalVpeSclInputSizeConfig_t *pCfg)
{
    DrvSclHvspIoInputConfig_t   stHvspInCfg;
    DrvSclOsMemset(&stHvspInCfg,0,sizeof(DrvSclHvspIoInputConfig_t));
    stHvspInCfg.stCaptureWin.u16X = 0;
    stHvspInCfg.stCaptureWin.u16Y = 0;
    stHvspInCfg.stCaptureWin.u16Width = pCfg->u16Width;
    stHvspInCfg.stCaptureWin.u16Height = pCfg->u16Height;
    stHvspInCfg.enColor = _MHalVpeTranHvspColorFormat(pCfg->ePixelFormat);
    if(enId ==E_HAL_SCLHVSP_ID_1)
    {
        if(stHvspInCfg.enColor == E_DRV_SCLHVSP_IO_COLOR_RGB)
        {
            stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_PAT_TGEN;
        }
        else
        {
            stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_ISP;
        }
    }
    else if(enId ==E_HAL_SCLHVSP_ID_3)
    {
        if((pCtx->eInMode==E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME))
        {
            stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_DRAM_RSC;
        }
        else if((pCtx->eInMode==E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME))
        {
            stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_DRAM_LDC;
        }
        else
        {
            stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_HVSP;
        }
    }
    else
    {
        stHvspInCfg.enSrcType = E_DRV_SCLHVSP_IO_SRC_HVSP;
    }
    stHvspInCfg = FILL_VERCHK_TYPE(stHvspInCfg, stHvspInCfg.VerChk_Version, stHvspInCfg.VerChk_Size,DRV_SCLHVSP_VERSION);
    if(_MHalVpeSclbSetting(enId,pCtx)&& _DrvSclHvspIoSetInputConfig(pCtx->s32Handle[enId], &stHvspInCfg))
    {
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeM2MSetInputConfig(MHalSclHandleConfig_t* pCtx)
{
    DrvM2MIoConfig_t stCfg;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvM2MIoConfig_t));
    stCfg.enPort = E_DRV_M2M_INPUT_PORT;
    stCfg.u16Width = pCtx->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Width;
    stCfg.u16Height = pCtx->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Height;
    stCfg.enBufMDType = E_DRV_M2M_IO_BUFFER_MD_SINGLE;
    stCfg.enColorType =
    _MHalVpeTranDmaColorFormat(pCtx->stCtx.enPortFormat[E_MHAL_M2M_INPUT_PORT],E_MHAL_COMPRESS_MODE_NONE);
    stCfg.enSrc = E_DRV_M2M_IO_SRC_DRAM_RSC;
    stCfg.enTiming = E_DRV_M2M_IO_OUTTIMING_SELF;
    stCfg.enMemType = E_DRV_M2M_IO_MEM_FRMR;
    _DrvM2MIoSetM2MConfig(pCtx->s32Handle[E_HAL_M2M_ID],&stCfg);
    return VPE_RETURN_OK;
}
bool _MHalVpeM2MSetOutputConfig(MHalVpeDmaPort_e enPortId,MHalSclHandleConfig_t* pCtx)
{
    DrvM2MIoConfig_t stCfg;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvM2MIoConfig_t));
    stCfg.enPort = enPortId;
    stCfg.u16Width = pCtx->stCtx.stPortSizeCfg[enPortId].u16Width;
    stCfg.u16Height = pCtx->stCtx.stPortSizeCfg[enPortId].u16Height;
    stCfg.enBufMDType = E_DRV_M2M_IO_BUFFER_MD_SINGLE;
    stCfg.enColorType =
        _MHalVpeTranDmaColorFormat(pCtx->stCtx.enPortFormat[enPortId],E_MHAL_COMPRESS_MODE_NONE);
    stCfg.enSrc = E_DRV_M2M_IO_SRC_DRAM_RSC;
    stCfg.enTiming = E_DRV_M2M_IO_OUTTIMING_SELF;
    stCfg.enMemType = E_DRV_M2M_IO_MEM_FRM;
    _DrvM2MIoSetM2MConfig(pCtx->s32Handle[E_HAL_M2M_ID],&stCfg);
    return VPE_RETURN_OK;
}
void _MHalVpeDeInit(void)
{
    if(genScinit== E_HAL_INIT_BY_CREATE)
    //if(genScinit)
    {
        VPE_ERR("[VPESCL]%s Remove start\n",__FUNCTION__);
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_VIP)))
        {
            _DrvSclVipIoDeInit();
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_M2M)))
        {
            _DrvM2MIoDeInit();
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_4)))
        {
            _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_4);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_3)))
        {
            _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_3);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_2)))
        {
            _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_2);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_1)))
        {
            _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_1);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_4)))
        {
            _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_4);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_3)))
        {
            _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_3);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_2)))
        {
            _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_2);
        }
        if((DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_1)))
        {
            _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_1);
        }
        _DrvSclVpeModuleDeInit();
        genScinit = E_HAL_INIT_DEINIT;
    }
}
void _MHalVpeInit(void)
{
    u16 i;
    if(genScinit == 0)
    {
        genScinit = E_HAL_INIT_BY_PROBE;
        for(i = 0; i < VPE_HANDLER_INSTANCE_NUM; i++)
        {
            pgstScHandler[i] = NULL;
            pgstIqHandler[i] = NULL;
        }
        if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_ALL)==E_DRV_SCLOS_INIT_ALL))
        {
            //init all
            _DrvSclVpeModuleInit();
            DrvSclOsSetAccessRegMode(E_DRV_SCLOS_AccessReg_CPU);
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_M2M)))
            {
                _DrvM2MIoInit();
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_M2M);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_1)))
            {
                _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_1);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_HVSP_1);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_2)))
            {
                _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_2);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_HVSP_2);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_3)))
            {
                _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_3);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_HVSP_3);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_HVSP_4)))
            {
                _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_4);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_HVSP_4);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_1)))
            {
                _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_1);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_DMA_1);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_2)))
            {
                _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_2);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_DMA_2);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_3)))
            {
                _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_3);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_DMA_3);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA_4)))
            {
                _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_4);
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_DMA_4);
            }
            if(!(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_VIP)))
            {
                _DrvSclVipIoInit();
                DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_VIP);
            }
            genScinit = E_HAL_INIT_BY_CREATE;
        }

    }
}
bool _MHalVpeFindEmptyInst(s16 *s16Idx)
{
    u16 i;
    for(i = 0; i < VPE_HANDLER_INSTANCE_NUM; i++)
    {
        if(pgstScHandler[i] == NULL)
        {
            pgstScHandler[i] = DrvSclOsMemalloc(sizeof(MHalSclHandleConfig_t),GFP_KERNEL);
            if(pgstScHandler[i]==NULL)
            {
                return VPE_RETURN_ERR;
            }
            DrvSclOsMemset(pgstScHandler[i],0,sizeof(MHalSclHandleConfig_t));
            *s16Idx = i;
            break;
        }
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeFindEmptyIqInst(s16 *s16Idx)
{
    u16 i;
    for(i = 0; i < VPE_HANDLER_INSTANCE_NUM; i++)
    {
        if(pgstIqHandler[i] == NULL)
        {
            pgstIqHandler[i] = DrvSclOsMemalloc(sizeof(MHalIqHandleConfig_t),GFP_KERNEL);
            if(pgstIqHandler[i]==NULL)
            {
                return VPE_RETURN_ERR;
            }
            *s16Idx = i;
            break;
        }
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeCloseM2MDevice(s32 *s32Handle)
{
    _DrvM2MIoRelease(s32Handle[E_HAL_M2M_ID]);
    return VPE_RETURN_OK;
}
bool _MHalVpeCloseDevice(s32 *s32Handle)
{
    _DrvSclHvspIoRelease(s32Handle[E_HAL_SCLHVSP_ID_1]);
    _DrvSclHvspIoRelease(s32Handle[E_HAL_SCLHVSP_ID_2]);
    _DrvSclHvspIoRelease(s32Handle[E_HAL_SCLHVSP_ID_3]);
    _DrvSclHvspIoRelease(s32Handle[E_HAL_SCLHVSP_ID_4]);
    _DrvSclDmaIoRelease(s32Handle[E_HAL_SCLDMA_ID_1]);
    _DrvSclDmaIoRelease(s32Handle[E_HAL_SCLDMA_ID_2]);
    _DrvSclDmaIoRelease(s32Handle[E_HAL_SCLDMA_ID_3]);
    _DrvSclDmaIoRelease(s32Handle[E_HAL_SCLDMA_ID_4]);
    return VPE_RETURN_OK;
}
bool _MHalVpeOpenIqDevice(s16 s16Idx)
{
    u16 i;
    char sg_Iq_Roi_name[16];
    u16 u16Size = 0;
    u16Size = DRV_SCLVIP_IO_WDR_HIST1_BUFFER_SIZE + DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE*3 +DRV_SCLVIP_IO_DLC_HIST_SIZE;
    if(u16Size%0x1000)
    {
        u16Size = ((u16Size/0x1000)+1)*0x1000;
    }
    pgstIqHandler[s16Idx]->bUsed = 1;
    pgstIqHandler[s16Idx]->u16InstId = s16Idx;
    pgstIqHandler[s16Idx]->s32Handle = _DrvSclVipIoOpen(E_DRV_SCLVIP_IO_ID_1);
    pgstIqHandler[s16Idx]->pvNrBuffer = DrvSclOsMemalloc(DRV_SCLVIP_IO_NR_SIZE,GFP_KERNEL);
    if(pgstIqHandler[s16Idx]->pvNrBuffer ==NULL)
    {
        VPE_ERR("[VPE]%s Alloc pvNrBuffer fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    else
    {
        VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2),"[VPE]%s Alloc pvNrBuffer %lx\n",__FUNCTION__,(u32)pgstIqHandler[s16Idx]->pvNrBuffer);
    }
    _MHalVpeIqMemNaming(sg_Iq_Roi_name,s16Idx);
    pgstIqHandler[s16Idx]->u32VirBaseAddr[0] =
        (u32)DrvSclOsDirectMemAlloc(sg_Iq_Roi_name,(u32)u16Size,(DrvSclOsDmemBusType_t *)&pgstIqHandler[s16Idx]->u32BaseAddr[0]);
    pgstIqHandler[s16Idx]->u32VirBaseAddr[1] = pgstIqHandler[s16Idx]->u32VirBaseAddr[0]+DRV_SCLVIP_IO_WDR_HIST1_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32VirBaseAddr[2] = pgstIqHandler[s16Idx]->u32VirBaseAddr[1]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32VirBaseAddr[3] = pgstIqHandler[s16Idx]->u32VirBaseAddr[2]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32BaseAddr[1] = pgstIqHandler[s16Idx]->u32BaseAddr[0]+DRV_SCLVIP_IO_WDR_HIST1_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32BaseAddr[2] = pgstIqHandler[s16Idx]->u32BaseAddr[1]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32BaseAddr[3] = pgstIqHandler[s16Idx]->u32BaseAddr[2]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    //alloc dlc buffer
    pgstIqHandler[s16Idx]->u32DlcHistAddr = pgstIqHandler[s16Idx]->u32BaseAddr[3]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    pgstIqHandler[s16Idx]->u32VirDlcHistAddr = pgstIqHandler[s16Idx]->u32VirBaseAddr[3]+DRV_SCLVIP_IO_WDR_HIST_BUFFER_SIZE;
    for(i=0;i<ROI_WINDOW_MAX;i++)
    {
        pgstIqHandler[s16Idx]->stWdrBuffer.u32Y[i] =0;
    }
    if(pgstIqHandler[s16Idx]->s32Handle==-1)
    {
        VPE_ERR("[VPE]%s Open Device handler fail\n",__FUNCTION__);
        _MHalVpeCleanIqInstBuffer(pgstIqHandler[s16Idx]);
        return VPE_RETURN_ERR;
    }
    //init iq
    pgstIqHandler[s16Idx]->bEsEn = 0;
    pgstIqHandler[s16Idx]->bNREn = 0;
    pgstIqHandler[s16Idx]->bProcess = 0;
    pgstIqHandler[s16Idx]->bNrFBAllocate = 0;
    pgstIqHandler[s16Idx]->pu8UVM[0] = 0x20;
    pgstIqHandler[s16Idx]->pu8UVM[1] = 0x00;
    pgstIqHandler[s16Idx]->pu8UVM[2] = 0x00;
    pgstIqHandler[s16Idx]->pu8UVM[3] = 0x20;
    pgstIqHandler[s16Idx]->u8NLMShift_ESOn = 0;
    pgstIqHandler[s16Idx]->u8NLMShift_ESOff = 0;
    for (i=0; i<16; i++) {
        pgstIqHandler[s16Idx]->pu8NLM_ESOn[i] = 0;
        pgstIqHandler[s16Idx]->pu8NLM_ESOff[i] = 0;
    }
    pgstIqHandler[s16Idx]->bWDREn = 0;
    pgstIqHandler[s16Idx]->bWDRActive= 0;
    pgstIqHandler[s16Idx]->u8Wdr_Str= 0;
    pgstIqHandler[s16Idx]->u8Wdr_Slope= 0;

    return VPE_RETURN_OK;
}
bool _MHalVpeOpenM2MDevice(s16 s16Idx, const MHalVpeSclWinSize_t *stMaxWin)
{
    DrvM2MIoIdType_e enType = 0;
    enType = (pgstScHandler[s16Idx]->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME) ? E_DRV_M2M_IO_ID_LDC :
            (pgstScHandler[s16Idx]->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION) ? E_DRV_M2M_IO_ID_ROT :
            (pgstScHandler[s16Idx]->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME) ? E_DRV_M2M_IO_ID_RSC :
            E_DRV_M2M_IO_ID_RSC;
    pgstScHandler[s16Idx]->bUsed = 1;
    pgstScHandler[s16Idx]->u16InstId = s16Idx;
    pgstScHandler[s16Idx]->s32Handle[E_HAL_M2M_ID] = _DrvM2MIoOpen(enType);
    if(pgstScHandler[s16Idx]->s32Handle[E_HAL_M2M_ID]==-1)
    {
        VPE_ERR("[VPE]%s Open Device handler fail\n",__FUNCTION__);
        _MHalVpeCloseM2MDevice(pgstScHandler[s16Idx]->s32Handle);
        _MHalVpeCleanScInstBuffer(pgstScHandler[s16Idx]);
        return VPE_RETURN_ERR;
    }
    //init Ctx
        pgstScHandler[s16Idx]->stCtx.stCropCfg.bCropEn = 0;
        pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Height = stMaxWin->u16Height;
        pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Width = stMaxWin->u16Width;
        pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16X = 0;
        pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Y = 0;
        pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Height = stMaxWin->u16Height;
        pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Width = stMaxWin->u16Width;
        pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Height = stMaxWin->u16Height;
        pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Width = stMaxWin->u16Width;
        pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_M2M_INPUT_PORT] = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422;
        if(pgstScHandler[s16Idx]->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT0)
        {
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Height = stMaxWin->u16Height;
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Width = stMaxWin->u16Width;
            pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT0] = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
        }
        if(pgstScHandler[s16Idx]->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT1)
        {
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Height = stMaxWin->u16Height;
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Width = stMaxWin->u16Width;
            pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT1] = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
        }
        if(pgstScHandler[s16Idx]->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2)
        {
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Height = stMaxWin->u16Height;
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Width = stMaxWin->u16Width;
            pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT2] = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
        }
        if(pgstScHandler[s16Idx]->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT3)
        {
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Height = stMaxWin->u16Height;
            pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Width = stMaxWin->u16Width;
            pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT3] = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
        }
        return VPE_RETURN_OK;
}
bool _MHalVpeOpenDevice(s16 s16Idx, const MHalVpeSclWinSize_t *stMaxWin)
{
    u16 i;
    pgstScHandler[s16Idx]->bUsed = 1;
    pgstScHandler[s16Idx]->u16InstId = s16Idx;
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLHVSP_ID_1] = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_1);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLHVSP_ID_2] = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_2);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLHVSP_ID_3] = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_3);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLHVSP_ID_4] = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_4);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_1] = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_1);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_2] = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_2);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_3] = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_3);
    pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_4] = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_4);
    for(i = 0; i < E_HAL_ID_MAX; i++)
    {
        if(pgstScHandler[s16Idx]->s32Handle[i]==-1)
        {
            VPE_ERR("[VPE]%s Open Device handler fail\n",__FUNCTION__);
            _MHalVpeCloseDevice(pgstScHandler[s16Idx]->s32Handle);
            _MHalVpeCleanScInstBuffer(pgstScHandler[s16Idx]);
            return VPE_RETURN_ERR;
        }
    }//init Ctx
    pgstScHandler[s16Idx]->stCtx.bEnImi = 0;
    pgstScHandler[s16Idx]->stCtx.stCropCfg.bCropEn = 0;
    pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Height = stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Width = stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16X = 0;
    pgstScHandler[s16Idx]->stCtx.stCropCfg.stCropWin.u16Y = 0;
    pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Height = stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Width = stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Height = stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT0].u16Width = stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Height = (stMaxWin->u16Height >720) ? 720 : stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT1].u16Width = (stMaxWin->u16Width > 1280) ? 1280 : stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Height = (stMaxWin->u16Height > 1080) ? 1080 : stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT2].u16Width = (stMaxWin->u16Width > 1920) ? 1920 : stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Height = stMaxWin->u16Height;
    pgstScHandler[s16Idx]->stCtx.stPortSizeCfg[E_MHAL_SCL_OUTPUT_PORT3].u16Width = stMaxWin->u16Width;
    pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT0] = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT1] = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT2] = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT3] = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    return VPE_RETURN_OK;
}
bool _MHalVpeCreateM2MInst(s32 *s32Handle)
{
    DrvM2MIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvM2MIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = M2M_DEVICE_COUNT;
    if(_DrvM2MIoCreateInstConfig(*s32Handle,&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Create Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeCreateInst(s32 *s32Handle)
{
    DrvSclDmaIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclDmaIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = VPE_DEVICE_COUNT;
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclDmaIoCreateInstConfig(s32Handle[E_HAL_SCLDMA_ID_1],&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Create Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeCreateIqInst(s32 *ps32Handle)
{
    DrvSclVipIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclVipIoLockConfig_t));
    stIoInCfg.ps32IdBuf = ps32Handle;
    stIoInCfg.u8BufSize = 1;
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclVipIoCreateInstConfig(*ps32Handle,&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Create Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeDestroyM2MInst(s32 *s32Handle)
{
    DrvM2MIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvM2MIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = M2M_DEVICE_COUNT;
    if(_DrvM2MIoDestroyInstConfig(*s32Handle,&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Destroy Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeDestroyInst(s32 *s32Handle)
{
    DrvSclDmaIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclDmaIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = VPE_DEVICE_COUNT;
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclDmaIoDestroyInstConfig(s32Handle[E_HAL_SCLDMA_ID_1],&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Destroy Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
bool _MHalVpeDestroyIqInst(s32 *s32Handle)
{
    DrvSclVipIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclVipIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = 1;
    //free wdr mload
    _DrvSclVipIoFreeWdrMloadBuffer(*s32Handle);
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclVipIoDestroyInstConfig(*s32Handle,&stIoInCfg))
    {
        VPE_ERR("[VPE]%s Destroy Inst Fail\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    return VPE_RETURN_OK;
}
void _MHalVpeSclInstFillProcessCfg
    (DrvSclDmaIoProcessConfig_t *stProcess,DrvSclOutputPort_e enPort,const MHalVpeSclOutputBufferConfig_t *pstIoInCfg)
{
    u8 j;
    stProcess->stCfg.bEn = pstIoInCfg->stCfg[enPort].bEn;
    stProcess->stCfg.enMemType = E_DRV_SCLDMA_IO_MEM_FRM;
    for(j =0;j<3;j++)
    {
        stProcess->stCfg.stBufferInfo.u64PhyAddr[j]=
            pstIoInCfg->stCfg[enPort].stBufferInfo.u64PhyAddr[j];
        stProcess->stCfg.stBufferInfo.u32Stride[j]=
            pstIoInCfg->stCfg[enPort].stBufferInfo.u32Stride[j];
    }
}
void _MHalVpeM2MInstFillProcessCfg
    (DrvM2MIoProcessConfig_t *stProcess,MHalVpeDmaPort_e enPort,const MHalVpeSclOutputBufferConfig_t *pstIoInCfg)
{
    u8 j;
    stProcess->enPort = enPort;
    stProcess->stCfg.bEn = pstIoInCfg->stCfg[enPort].bEn;
    stProcess->stCfg.enMemType = (enPort== E_MHAL_M2M_INPUT_PORT)? E_DRV_SCLDMA_IO_MEM_FRMR : E_DRV_SCLDMA_IO_MEM_FRM;
    for(j =0;j<3;j++)
    {
        stProcess->stCfg.stBufferInfo.u64PhyAddr[j]=
            pstIoInCfg->stCfg[enPort].stBufferInfo.u64PhyAddr[j];
        stProcess->stCfg.stBufferInfo.u32Stride[j]=
            pstIoInCfg->stCfg[enPort].stBufferInfo.u32Stride[j];
    }
}
void _MHalVpeM2MProcessFlip(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    DrvM2MIoProcessConfig_t stIoCfg;
    DrvSclOsMemset(&stIoCfg,0,sizeof(DrvM2MIoProcessConfig_t));
    _MHalVpeM2MInstFillProcessCfg(&stIoCfg,E_MHAL_M2M_INPUT_PORT,pBuffer);
    _DrvM2MIoInstProcess(pstScHandler->s32Handle[E_HAL_M2M_ID],&stIoCfg);
    if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_1,pstScHandler))
    {
        _MHalVpeM2MInstFillProcessCfg(&stIoCfg,E_MHAL_SCL_OUTPUT_PORT0,pBuffer);
        _DrvM2MIoInstProcess(pstScHandler->s32Handle[E_HAL_M2M_ID],&stIoCfg);
    }
    if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_2,pstScHandler))
    {
        _MHalVpeM2MInstFillProcessCfg(&stIoCfg,E_MHAL_SCL_OUTPUT_PORT1,pBuffer);
        _DrvM2MIoInstProcess(pstScHandler->s32Handle[E_HAL_M2M_ID],&stIoCfg);
    }
    if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_3,pstScHandler))
    {
        _MHalVpeM2MInstFillProcessCfg(&stIoCfg,E_MHAL_SCL_OUTPUT_PORT2,pBuffer);
        _DrvM2MIoInstProcess(pstScHandler->s32Handle[E_HAL_M2M_ID],&stIoCfg);
    }
    if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_4,pstScHandler))
    {
        _MHalVpeM2MInstFillProcessCfg(&stIoCfg,E_MHAL_SCL_OUTPUT_PORT3,pBuffer);
        _DrvM2MIoInstProcess(pstScHandler->s32Handle[E_HAL_M2M_ID],&stIoCfg);
    }
    _DrvM2MIoInstFlip(pstScHandler->s32Handle[E_HAL_M2M_ID]);
}
void _MHalVpeSclProcessFlip(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    DrvSclDmaIoProcessConfig_t stIoCfg;
    DrvSclOsMemset(&stIoCfg,0,sizeof(DrvSclDmaIoProcessConfig_t));
    if(pstScHandler->stCtx.bSet[E_DRV_SCL_OUTPUT_PORT0])
    {
        _MHalVpeSclInstFillProcessCfg(&stIoCfg,E_DRV_SCL_OUTPUT_PORT0,pBuffer);
        if(_MHalVpeSclbSetting(E_HAL_SCLDMA_ID_1,pstScHandler))
        {
            _DrvSclDmaIoInstProcess(pstScHandler->s32Handle[E_HAL_SCLDMA_ID_1],&stIoCfg);
        }
    }
    if(pstScHandler->stCtx.bSet[E_DRV_SCL_OUTPUT_PORT1])
    {
        _MHalVpeSclInstFillProcessCfg(&stIoCfg,E_DRV_SCL_OUTPUT_PORT1,pBuffer);
        if(_MHalVpeSclbSetting(E_HAL_SCLDMA_ID_2,pstScHandler))
        {
            _DrvSclDmaIoInstProcess(pstScHandler->s32Handle[E_HAL_SCLDMA_ID_2],&stIoCfg);
        }
    }
    if(pstScHandler->stCtx.bSet[E_DRV_SCL_OUTPUT_PORT2])
    {
        _MHalVpeSclInstFillProcessCfg(&stIoCfg,E_DRV_SCL_OUTPUT_PORT2,pBuffer);
        if(_MHalVpeSclbSetting(E_HAL_SCLDMA_ID_3,pstScHandler))
        {
            _DrvSclDmaIoInstProcess(pstScHandler->s32Handle[E_HAL_SCLDMA_ID_3],&stIoCfg);
        }
    }
    if(pstScHandler->stCtx.bSet[E_DRV_SCL_OUTPUT_PORT3])
    {
        _MHalVpeSclInstFillProcessCfg(&stIoCfg,E_DRV_SCL_OUTPUT_PORT3,pBuffer);
        if(_MHalVpeSclbSetting(E_HAL_SCLDMA_ID_4,pstScHandler))
        {
            _DrvSclDmaIoInstProcess(pstScHandler->s32Handle[E_HAL_SCLDMA_ID_4],&stIoCfg);
        }
    }
    _DrvSclDmaIoInstFlip(pstScHandler->s32Handle[E_HAL_SCLDMA_ID_1]);
}
bool _MHalVpeSclbFlip(MHalVpeDmaPort_e enPort, MHalSclHandleConfig_t * pstScHandler,bool bEn)
{
    bool ret = 0;
    if(bEn)
    {
        ret = 1;
    }
    else
    {
        if(pstScHandler->stCtx.bEn[enPort])
        {
            ret = 1;
        }
    }
    return ret;
}
bool _MHalVpeSclProcessInvaild(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer,u64 u64Time)
{
    bool bRet = 0;
    if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn == 0 &&
    pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn == 0 &&
    pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn == 0 &&
    pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn == 0)
    {
        bRet = 1;
    }
    else if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn == 1 &&!(pstScHandler->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT0))
    {
        VPE_ERR("[VPE]Port0 not support This mode\n");
        bRet = 1;
    }
    else if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn == 1 &&!(pstScHandler->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT1))
    {
        VPE_ERR("[VPE]Port1 not support This mode\n");
        bRet = 1;
    }
    else if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn == 1 &&!(pstScHandler->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        VPE_ERR("[VPE]Port2 not support This mode\n");
        bRet = 1;
    }
    else if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn == 1 &&!(pstScHandler->eOutMode &E_MHAL_SCL_OUTPUT_MODE_PORT3))
    {
        VPE_ERR("[VPE]Port3 not support This mode\n");
        bRet = 1;
    }
    else
    {
        if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn == 1 && gbHistints)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT0QUEUE,u64Time);
        }
        pstScHandler->stCtx.bSet[E_MHAL_SCL_OUTPUT_PORT0] =
        _MHalVpeSclbFlip(E_MHAL_SCL_OUTPUT_PORT0,pstScHandler,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn);
        pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT0] =pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn;
        if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn == 1 && gbHistints)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT1QUEUE,u64Time);
        }
        pstScHandler->stCtx.bSet[E_MHAL_SCL_OUTPUT_PORT1] =
        _MHalVpeSclbFlip(E_MHAL_SCL_OUTPUT_PORT1,pstScHandler,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn);
        pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT1] = pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn;
        if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn == 1 && gbHistints)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT2QUEUE,u64Time);
        }
        pstScHandler->stCtx.bSet[E_MHAL_SCL_OUTPUT_PORT2] =
        _MHalVpeSclbFlip(E_MHAL_SCL_OUTPUT_PORT2,pstScHandler,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn);
        pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT2] = pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn;
        if(pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn == 1 && gbHistints)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT3QUEUE,u64Time);
        }
        pstScHandler->stCtx.bSet[E_MHAL_SCL_OUTPUT_PORT3] =
        _MHalVpeSclbFlip(E_MHAL_SCL_OUTPUT_PORT3,pstScHandler,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn);
        pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT3] = pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn;
        bRet = 0;
    }
    return bRet;
}
bool _MHalVpeM2MProcessInvaild(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    bool bRet = 0;
    u64 u64Time = (DrvSclOsGetSystemTimeStamp());
    if(pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].bEn == 0)
    {
        bRet = 1;
    }
    else
    {
        if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT0)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT0QUEUE,u64Time);
            pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT0] =pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn;
        }
        if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT1)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT1QUEUE,u64Time);
            pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT1] =pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn;
        }
        if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT2QUEUE,u64Time);
            pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT2] =pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn;
        }
        if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT3)
        {
            DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_ADDPORT3QUEUE,u64Time);
            pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT3] =pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn;
        }
        if(pstScHandler->eInst == E_MHAL_VPE_SCL_SUB)
        {
            pstScHandler->stCtx.bEn[E_MHAL_M2M_INPUT_PORT] =pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].bEn;
        }
        bRet = 0;
    }
    return bRet;
}
void _MHalVpeM2MProcessDbg(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL5), "[VPESCL]%s Inst %hu InputPort En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
        __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].bEn,
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[0],
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[1],
        pBuffer->stCfg[E_MHAL_M2M_INPUT_PORT].stBufferInfo.u32Stride[2]);
    if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT0)
    {
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL5), "[VPESCL]%s Inst %hu OutputPort En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
            __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn,
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[2],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[2]);
    }
    if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT1)
    {
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL6), "[VPESCL]%s Inst %hu OutputPort En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
            __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].bEn,
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u64PhyAddr[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u64PhyAddr[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u64PhyAddr[2],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u32Stride[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u32Stride[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_MODE_PORT1].stBufferInfo.u32Stride[2]);
    }
    if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2)
    {
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL7), "[VPESCL]%s Inst %hu OutputPort En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
            __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn,
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[2],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[2]);
    }
    if(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT3)
    {
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL8), "[VPESCL]%s Inst %hu OutputPort En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
            __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn,
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[2],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[0],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[1],
            pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[2]);
    }
}
void _MHalVpeSclProcessDbg(MHalSclHandleConfig_t *pstScHandler,const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL5), "[VPESCL]%s Inst %hu Prot0 En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
        __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].bEn,
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT0].stBufferInfo.u32Stride[2]);
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL6), "[VPESCL]%s Inst %hu Prot1 En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
        __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].bEn,
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u32Stride[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u32Stride[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT1].stBufferInfo.u32Stride[2]);
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL7), "[VPESCL]%s Inst %hu Prot2 En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
        __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].bEn,
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT2].stBufferInfo.u32Stride[2]);
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL8), "[VPESCL]%s Inst %hu Prot3 En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%x,0x%x,0x%x}\n",
        __FUNCTION__,pstScHandler->u16InstId,pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].bEn,
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[0],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[1],
        pBuffer->stCfg[E_MHAL_SCL_OUTPUT_PORT3].stBufferInfo.u32Stride[2]);
}
void _MHalVpeSclForISPInputAlign(MHalSclHandleConfig_t *pstScHandler,MHalVpeSclInputSizeConfig_t *pCfg)
{
    if(pCfg->u16Width&0x1F)
    {
        pCfg->u16Width = ((pCfg->u16Width&0xFFE0)+0x20);
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL1), "[VPESCL]%s New Width:%hu\n",
            __FUNCTION__,pCfg->u16Width);
    }
}
//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
//ISP
typedef struct
{
    //MHalVpeIspInputFormat_e enInType;
    //MHalVpeIspRotationType_e enRotType;
    MS_U32 uCropX;
    MS_U32 uCropY;
    MS_U32 uImgW;
    MS_U32 uImgH;
    MHalVpeTopInputMode_e eMode;
    MHalPixelFormat_e ePixelFmt;
    //MHalRGBBayerID_e eBayerID;
    LibcameraIF_t *pLibcameraIf;
    const MHalAllocPhyMem_t *pMemAlloc;
    MHalHDRType_e eHDRMode;
#if defined(SCLOS_TYPE_LINUX_TEST)
#else
    BufPtr_t *pExtHDRBuf;
#endif
}MHalVpeCtx_t;

#define BANK_TO_ADDR(bank) (bank<<9)
//#define RIU_ISP_REG(bank,offset) ((bank<<9)+(offset<<2)) //register table to RIU address
#define CMQ_RIU_REG(bank,offset) ((bank<<8)+(offset<<1)) //register address from CMQ
#define CPU_RIU_REG(bank,offset) ((bank<<9)+(offset<<2)) //register address from CPU

#define BANK_ISP0 0x1509
#define BANK_ISP7 0x1510
#define BANK_DMAG0 0x1517
#define BANK_DMA_IRQ 0x153B
#define BANK_SC_FLIP    0x153E
#define BANK_SC_SC0    0x1525
#define BANK_SC_SC1    0x1526
#define BANK_SC_DWIN    0x13F2
#define BANK_SC_MCNR    0x1532
#define BANK_SC_WDR0    0x151B
#define BANK_SC_WDR1    0x151C
#define BANK_SC_ARB_ROI 0x1539
#define BANK_CMDQ_CMDQ0    0x1235

/* IO REMAP */
void *gpVpeIoBase = NULL;
MS_BOOL _VpeIsCmdq(MHAL_CMDQ_CmdqInterface_t* pstCmdQInfo)
{
    if(pstCmdQInfo==NULL|| pstCmdQInfo->MHAL_CMDQ_WriteRegCmdq==NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void _VpeIspRegW(MHAL_CMDQ_CmdqInterface_t* pstCmdQInfo,unsigned short uBank,unsigned short uOffset,unsigned short uVal)
{
    if(pstCmdQInfo==NULL || pstCmdQInfo->MHAL_CMDQ_WriteRegCmdq==NULL)
        *(unsigned short*)((unsigned long)gpVpeIoBase + CPU_RIU_REG(uBank,uOffset)) = uVal;
    else
        pstCmdQInfo->MHAL_CMDQ_WriteRegCmdq(pstCmdQInfo,CMQ_RIU_REG(uBank,uOffset),uVal);
}
void _VpeIspRegWMsk(MHAL_CMDQ_CmdqInterface_t* pstCmdQInfo,unsigned short uBank,unsigned short uOffset,unsigned short uVal,unsigned short uMsk)
{
    if(pstCmdQInfo==NULL || pstCmdQInfo->MHAL_CMDQ_WriteRegCmdq==NULL)
    {
        if(gpVpeIoBase==NULL)
        {
            gpVpeIoBase = (void*)0xFD000000;
        }
        *(unsigned short*)((unsigned long)gpVpeIoBase + CPU_RIU_REG(uBank,uOffset)) =
        ((*(unsigned short*)((unsigned long)gpVpeIoBase + CPU_RIU_REG(uBank,uOffset)) & (~uMsk))|(uVal&uMsk));
    }
    else
        pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo,CMQ_RIU_REG(uBank,uOffset),uVal,uMsk);
}
unsigned short _VpeIspRegR(unsigned short uBank,unsigned short uOffset)
{
    if(gpVpeIoBase==NULL)
    {
        gpVpeIoBase = (void*)0xFD000000;
    }
    return *(unsigned short*)((unsigned long)gpVpeIoBase + CPU_RIU_REG(uBank,uOffset));
}
//Time = ns
int _VpeIspRegPollWait(MHAL_CMDQ_CmdqInterface_t* pstCmdQInfo,unsigned short uBank,unsigned short uOffset,unsigned short uMask,unsigned short uVal,u32 u32Time)
{
    u32 wait_cnt = 0;
    if(pstCmdQInfo==NULL || pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits==NULL)
    {
        while( !(_VpeIspRegR(uBank,uOffset)&(uMask&uVal) ))
        {
            if(++wait_cnt > 200)
            {
                //VPE_ERR("[%s]Vpe Poll err\n",__FUNCTION__);
                //break;
            }
            DrvSclOsDelayTask(((u32Time/10000) ? (u32Time/10000) :1)); //wait time ns/1000
        }
    }
    else
    {
        pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits_ByTime(pstCmdQInfo,CMQ_RIU_REG(uBank,uOffset),uVal,uMask,TRUE,u32Time);
    }
    return 0;
}

int _VpeSclRegPollDummyWait(unsigned short uBank,unsigned short uOffset,unsigned short uMask,unsigned short uVal,u32 u32Time)
{
	u32 wait_cnt = 0;

 	if(gpVpeIoBase==NULL)
    {
        gpVpeIoBase = (void*)0xFD000000;
    }

	while( !(_VpeIspRegR(uBank,uOffset)&(uMask&uVal) ))
	{
	    if(++wait_cnt > 200)
	    {
	        VPE_ERR("[%s]Vpe Poll err\n",__FUNCTION__);
	        break;
	    }
	    DrvSclOsDelayTask(((u32Time/10000) ? (u32Time/10000) :1)); //wait time ns/1000
	}
	_VpeIspRegWMsk(NULL,uBank,uOffset,0x0,ISSW_IDLEDUMMY);
	return 0;
}
bool _MHalVpeCreateSclInstance(const MHalVpeSclWinSize_t *stMaxWin, s16 s16Idx)
{
	bool bRet = VPE_RETURN_OK;
	MHalVpeSclInputSizeConfig_t stCfg;
	MHalVpeSclDmaConfig_t stDmaCfg;
    DrvSclDmaIoProcessConfig_t stIoCfg;
	DrvSclOsMemset(&stCfg,0,sizeof(MHalVpeSclInputSizeConfig_t));
	DrvSclOsMemset(&stDmaCfg,0,sizeof(MHalVpeSclDmaConfig_t));
	DrvSclOsMemset(&stIoCfg,0,sizeof(DrvSclDmaIoProcessConfig_t));
	//open device
	bRet = _MHalVpeOpenDevice(s16Idx,stMaxWin);
	if(bRet == VPE_RETURN_ERR)
	{
		return bRet;
	}
	//create inst
	bRet = _MHalVpeCreateInst(pgstScHandler[s16Idx]->s32Handle);
	if(bRet == VPE_RETURN_ERR)
	{
		_MHalVpeCloseDevice(pgstScHandler[s16Idx]->s32Handle);
		_MHalVpeCleanScInstBuffer(pgstScHandler[s16Idx]);
		return bRet;
	}

	//Default Setting (create Reg tbl and inquire tbl)
	//ToDo
	stCfg.u16Height = pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Height;
	stCfg.u16Width = pgstScHandler[s16Idx]->stCtx.stInputCfg.u16Width;
	_MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_1,pgstScHandler[s16Idx],&stCfg);
	_MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_2,pgstScHandler[s16Idx],&stCfg);
	_MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_3,pgstScHandler[s16Idx],&stCfg);
	_MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_4,pgstScHandler[s16Idx],&stCfg);
	_MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_1,pgstScHandler[s16Idx]);
	_MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_2,pgstScHandler[s16Idx]);
	_MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_3,pgstScHandler[s16Idx]);
	_MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_4,pgstScHandler[s16Idx]);
	stDmaCfg.enPort = E_MHAL_SCL_OUTPUT_PORT0;
	stDmaCfg.enFormat = pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT0];
	_MHalVpeDmaConfig(E_HAL_SCLDMA_ID_1,pgstScHandler[s16Idx],&stDmaCfg);
	stDmaCfg.enPort = E_MHAL_SCL_OUTPUT_PORT1;
	stDmaCfg.enFormat = pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT1];
	_MHalVpeDmaConfig(E_HAL_SCLDMA_ID_2,pgstScHandler[s16Idx],&stDmaCfg);
	stDmaCfg.enPort = E_MHAL_SCL_OUTPUT_PORT2;
	stDmaCfg.enFormat = pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT2];
	_MHalVpeDmaConfig(E_HAL_SCLDMA_ID_3,pgstScHandler[s16Idx],&stDmaCfg);
	stDmaCfg.enPort = E_MHAL_SCL_OUTPUT_PORT3;
	stDmaCfg.enFormat = pgstScHandler[s16Idx]->stCtx.enPortFormat[E_MHAL_SCL_OUTPUT_PORT3];
	_MHalVpeDmaConfig(E_HAL_SCLDMA_ID_4,pgstScHandler[s16Idx],&stDmaCfg);
	stIoCfg.stCfg.enMemType = E_DRV_SCLDMA_IO_MEM_FRM;
    _DrvSclDmaIoInstProcess(pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_1],&stIoCfg);
    _DrvSclDmaIoInstProcess(pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_2],&stIoCfg);
    _DrvSclDmaIoInstProcess(pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_3],&stIoCfg);
    _DrvSclDmaIoInstProcess(pgstScHandler[s16Idx]->s32Handle[E_HAL_SCLDMA_ID_4],&stIoCfg);
    return bRet;
}
bool _MHalVpeCreateM2MInstance(const MHalVpeSclWinSize_t *stMaxWin,const s16 s16Idx)
{
	bool bRet = VPE_RETURN_OK;
	//open device
	bRet = _MHalVpeOpenM2MDevice(s16Idx,stMaxWin);
	if(bRet == VPE_RETURN_ERR)
	{
		return bRet;
	}
	//create inst
	bRet = _MHalVpeCreateM2MInst(&pgstScHandler[s16Idx]->s32Handle[E_HAL_M2M_ID]);
	if(bRet == VPE_RETURN_ERR)
	{
		_MHalVpeCloseM2MDevice(pgstScHandler[s16Idx]->s32Handle);
		_MHalVpeCleanScInstBuffer(pgstScHandler[s16Idx]);
		return bRet;
	}
	//Default Setting (create Reg tbl and inquire tbl)
	//ToDo
	_MHalVpeM2MSetInputConfig(pgstScHandler[s16Idx]);
	if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_1,pgstScHandler[s16Idx]))
	{
	    _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT0,pgstScHandler[s16Idx]);
    }
	if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_2,pgstScHandler[s16Idx]))
	{
	    _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT1,pgstScHandler[s16Idx]);
    }
	if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_3,pgstScHandler[s16Idx]))
	{
	    _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT2,pgstScHandler[s16Idx]);
    }
	if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_4,pgstScHandler[s16Idx]))
	{
	    _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT3,pgstScHandler[s16Idx]);
    }
    return bRet;
}
bool _MHalVpeM2MProcess(MHalSclHandleConfig_t *pstScHandler, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    bool bRet = VPE_RETURN_OK;
    u32 u32Event;
    _MHalVpeM2MKeepCmdqFunction(pstCmdQInfo);
    _MHalVpeM2MProcessDbg(pstScHandler,pBuffer);
    if(_MHalVpeM2MProcessInvaild(pstScHandler,pBuffer))
    {
        return VPE_RETURN_ERR;
    }
    if(!pstCmdQInfo && genIrqmode[E_HAL_SCL_INST_M2M])
    {
        DrvSclOsWaitEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN, &u32Event, E_DRV_SCLOS_EVENT_MD_OR,100);
        DrvSclOsClearEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN);
    }
    _MHalVpeM2MProcessFlip(pstScHandler,pBuffer);
    return bRet;
}
bool _MHalVpeSclProcess(MHalSclHandleConfig_t *pstScHandler, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    bool bRet = VPE_RETURN_OK;
    static bool bSet = 0;
    u64 u64Cnt = 0;
    u64 u64Time = ((u64)DrvSclOsGetSystemTimeStamp());
    _MHalVpeSclKeepCmdqFunction(pstCmdQInfo);
    if(VPE_DBG_LV_SC(0xF0))
    _MHalVpeSclProcessDbg(pstScHandler,pBuffer);
    if(_MHalVpeSclProcessInvaild(pstScHandler,pBuffer,u64Time))
    {
        return VPE_RETURN_ERR;
    }
    // if bSet equel the dummy(bSet+1) ,it's mean cmdq not move.
    if(gbHistints || pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE)
    {
        _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,0x0,ISCMDQ_IDLEDUMMY);
        _VpeIspRegWMsk(pstCmdQInfo,(ISSCPROCESS_DUMMYReg>>8),(ISSCPROCESS_DUMMYReg&0xFF)/2,((bSet+1)),ISSCPROCESS_DUMMY);
    }
	if(pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE && pstScHandler->eInst == E_MHAL_VPE_SCL_MAIN)
	{
		if(_VpeIsCmdq(pstCmdQInfo))
		{
		// bSet ==dummy && CMDQ idle is mean Cmdq not move.
		    if(DrvSclIrqGetIsCMDQIdle())
		    {
		        DrvSclIrqSetCMDQIdle(0);
		        if(DrvSclIrqGetIsBlankingRegion())
		        {
                    DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_PROCESSLATE,u64Time);
		        }
		        else
		        {
                    DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_PROCESSDROP,u64Time);
                }
		    }
            pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits(pstCmdQInfo,ISSW_IDLEDUMMYReg,ISSW_IDLEDUMMY,ISSW_IDLEDUMMY,1);
            pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo,ISSW_IDLEDUMMYReg,0x0,ISSW_IDLEDUMMY);
		}
		else
		{
			_VpeSclRegPollDummyWait((ISSW_IDLEDUMMYReg>>8),(ISSW_IDLEDUMMYReg&0xFF)/2,ISSW_IDLEDUMMY,ISSW_IDLEDUMMY,200000);
		}
	}
    if(gbHistints || pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE)
    {
        bSet ++;
        bSet &=ISSCPROCESS_DUMMYLENTH;
        u64Cnt = bSet;
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_PROCESSCNT,u64Cnt);
        DrvSclIrqSetSclInts(E_DRV_SCL_IRQ_PROCESSDIFF,(u64)u64Time);
    }
    _MHalVpeSclProcessFlip(pstScHandler,pBuffer);
    if((pstScHandler->eOutMode&E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1C,0xFFFB,0xFFFF);
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0xFFFB,0xFFFF);
    }
    else
    {
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1C,0xFFFD,0xFFFF);
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0xFFFD,0xFFFF);
    }
    return bRet;
}
bool _MHalVpeSclInputConfig(MHalSclHandleConfig_t *pstScHandler, MHalVpeSclInputSizeConfig_t *pstForISPCfg)
{
    bool bRet = VPE_RETURN_OK;
    _MHalVpeSclForISPInputAlign(pstScHandler,pstForISPCfg);
    if(_MHalVpeScIsInputSizeNeedReSetting(pstScHandler,pstForISPCfg))
    {
        // set global
        _MHalVpeScInputSizeReSetting(pstScHandler,pstForISPCfg->u16Width,pstForISPCfg->u16Height);
        // set input
        bRet = _MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_1,pstScHandler,pstForISPCfg);
        // set scaling size
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_1,pstScHandler);
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_4,pstScHandler);
    }
    else
    {
        bRet = _MHalVpeHvspInConfig(E_HAL_SCLHVSP_ID_1,pstScHandler,pstForISPCfg);
    }
    return bRet;
}
bool _MHalVpeM2MInputConfig(MHalSclHandleConfig_t *pstScHandler, MHalVpeSclInputSizeConfig_t *pstForISPCfg)
{
    bool bRet = VPE_RETURN_OK;
    if(_MHalVpeScIsInputSizeNeedReSetting(pstScHandler,pstForISPCfg))
    {
        // set global
        _MHalVpeScInputSizeReSetting(pstScHandler,pstForISPCfg->u16Width,pstForISPCfg->u16Height);
        pstScHandler->stCtx.enPortFormat[E_MHAL_M2M_INPUT_PORT] = pstForISPCfg->ePixelFormat;
        pstScHandler->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Height = pstForISPCfg->u16Height;
        pstScHandler->stCtx.stPortSizeCfg[E_MHAL_M2M_INPUT_PORT].u16Width = pstForISPCfg->u16Width;
        bRet = _MHalVpeM2MSetInputConfig(pstScHandler);
        if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_1,pstScHandler))
        {
            _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT0,pstScHandler);
        }
        if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_2,pstScHandler))
        {
            _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT1,pstScHandler);
        }
        if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_3,pstScHandler))
        {
            _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT2,pstScHandler);
        }
        if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_4,pstScHandler))
        {
            _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT3,pstScHandler);
        }
    }
    else
    {
        pstScHandler->stCtx.enPortFormat[E_MHAL_M2M_INPUT_PORT] = pstForISPCfg->ePixelFormat;
        bRet = _MHalVpeM2MSetInputConfig(pstScHandler);
    }
    return bRet;
}
bool _MHalVpeM2MPortSizeConfig(MHalSclHandleConfig_t *pstScHandler, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    if(pCfg->u16Width>1920&&
    (pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2)&&
    ((pstScHandler->stCtx.enPortFormat[E_MHAL_M2M_INPUT_PORT]==E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420)||
    (pstScHandler->stCtx.enPortFormat[pCfg->enOutPort]==E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420)))
    {
        VPE_ERR("[VPE]Port:%d Width over spec :%hu\n",pCfg->enOutPort,pCfg->u16Width);
        return VPE_RETURN_ERR;
    }
    if(_MHalVpeScIsOutputSizeNeedReSetting(pstScHandler,pCfg))
    {
        // set global
        _MHalVpeScOutputSizeReSetting(pstScHandler,pCfg);
        if(pCfg->enOutPort == E_MHAL_M2M_INPUT_PORT)
        {
            pstScHandler->stCtx.stInputCfg.u16Height = pCfg->u16Height;
            pstScHandler->stCtx.stInputCfg.u16Width = pCfg->u16Width;
            _MHalVpeM2MSetInputConfig(pstScHandler);
            if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_1,pstScHandler))
            {
                _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT0,pstScHandler);
            }
            if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_2,pstScHandler))
            {
                _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT1,pstScHandler);
            }
            if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_3,pstScHandler))
            {
                _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT2,pstScHandler);
            }
            if(_MHalVpeSclbSetting(E_HAL_SCLHVSP_ID_4,pstScHandler))
            {
                _MHalVpeM2MSetOutputConfig(E_MHAL_SCL_OUTPUT_PORT3,pstScHandler);
            }
        }
        else
        {
            // set scaling size
            bRet = _MHalVpeM2MSetOutputConfig(pCfg->enOutPort,pstScHandler);
        }
    }
    return bRet;
}
bool _MHalVpeSclPortSizeConfig(MHalSclHandleConfig_t *pstScHandler, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalVpeSclDmaConfig_t stDmaCfg;
    DrvSclIdType_e enId;
    DrvSclOsMemset(&stDmaCfg,0,sizeof(MHalVpeSclDmaConfig_t));
    enId = (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT0) ? E_HAL_SCLDMA_ID_1 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT1) ? E_HAL_SCLDMA_ID_2 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT2) ? E_HAL_SCLDMA_ID_3 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT3) ? E_HAL_SCLDMA_ID_4 :
                    E_HAL_ID_MAX;
    if(pCfg->enOutPort==E_MHAL_SCL_OUTPUT_PORT1)
    {
        if(pCfg->u16Width>1280)
        {
            VPE_ERR("[VPE]Port:%d Width over spec :%hu\n",pCfg->enOutPort,pCfg->u16Width);
            return VPE_RETURN_ERR;
        }
    }
    if(pCfg->enOutPort==E_MHAL_SCL_OUTPUT_PORT2)
    {
        // ToDo:422 to 420 has limitation.
        if(pCfg->u16Width>1920)
        {
            VPE_ERR("[VPE]Port:%d Width over spec :%hu\n",pCfg->enOutPort,pCfg->u16Width);
            return VPE_RETURN_ERR;
        }
    }
    if(_MHalVpeScIsOutputSizeNeedReSetting(pstScHandler,pCfg))
    {
        // set global
        _MHalVpeScOutputSizeReSetting(pstScHandler,pCfg);
        // set scaling size
        _MHalVpeSclOutputSizeConfig(pstScHandler,pCfg);
        stDmaCfg.enPort = pCfg->enOutPort;
        stDmaCfg.enFormat = pstScHandler->stCtx.enPortFormat[pCfg->enOutPort];
        _MHalVpeDmaConfig(enId,pstScHandler,&stDmaCfg);
    }
    return bRet;
}
bool _MHalVpeSclDmaImiPortConfig(MHalSclHandleConfig_t *pstScHandler, const MHalVpeSclOutputIMIConfig_t *pCfg)
{
    DrvSclIdType_e enId;
    bool bRet = VPE_RETURN_OK;
    enId = (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT0) ? E_HAL_SCLDMA_ID_1 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT1) ? E_HAL_SCLDMA_ID_2 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT2) ? E_HAL_SCLDMA_ID_3 :
           (pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT3) ? E_HAL_SCLDMA_ID_4 :
                    E_HAL_ID_MAX;
    if(enId==E_HAL_ID_MAX)
    {
        VPE_ERR("[VPESCL]%s Output Port Num Error\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    else
    {
        bRet = _MHalVpeDmaImiConfig(enId,pstScHandler,pCfg);
    }
    return bRet;
}
bool _MHalVpeSclDmaPortConfig(MHalSclHandleConfig_t *pstScHandler, const MHalVpeSclDmaConfig_t *pCfg)
{
    DrvSclIdType_e enId;
    bool bRet = VPE_RETURN_OK;
    enId = (pCfg->enPort ==E_MHAL_SCL_OUTPUT_PORT0) ? E_HAL_SCLDMA_ID_1 :
           (pCfg->enPort ==E_MHAL_SCL_OUTPUT_PORT1) ? E_HAL_SCLDMA_ID_2 :
           (pCfg->enPort ==E_MHAL_SCL_OUTPUT_PORT2) ? E_HAL_SCLDMA_ID_3 :
           (pCfg->enPort ==E_MHAL_SCL_OUTPUT_PORT3) ? E_HAL_SCLDMA_ID_4 :
                    E_HAL_ID_MAX;
    if(enId==E_HAL_ID_MAX)
    {
        VPE_ERR("[VPESCL]%s Output Port Num Error\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    else
    {
        pstScHandler->stCtx.enPortFormat[pCfg->enPort] = pCfg->enFormat;
        bRet = _MHalVpeDmaConfig(enId,pstScHandler,pCfg);
    }
    return bRet;
}
bool _MHalVpeM2MDmaPortConfig(MHalSclHandleConfig_t *pstScHandler, const MHalVpeSclDmaConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    pstScHandler->stCtx.enPortFormat[pCfg->enPort] = pCfg->enFormat;
    if(pCfg->enPort==E_MHAL_M2M_INPUT_PORT)
    {
        bRet = _MHalVpeM2MSetInputConfig(pstScHandler);
    }
    else
    {
        bRet = _MHalVpeM2MSetOutputConfig(pCfg->enPort,pstScHandler);
    }
    return bRet;
}
bool _MHalVpeM2MSetWaitDone(MHalSclHandleConfig_t *pstScHandler, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    bool bRet = VPE_RETURN_OK;
    u32 u32Event;
    _MHalVpeM2MKeepCmdqFunction(pstCmdQInfo);
    if(_VpeIsCmdq(pstCmdQInfo))
    {
        if(genIrqmode[E_HAL_SCL_INST_M2M])
        {
            _VpeIspRegPollWait(pstCmdQInfo,(ISM2MDone_DUMMYReg>>8),(ISM2MDone_DUMMYReg&0xFF)/2,ISM2MDone_DUMMY,ISM2MDone_DUMMY,100000);
            _VpeIspRegWMsk(pstCmdQInfo,(ISM2MDone_DUMMYReg>>8),(ISM2MDone_DUMMYReg&0xFF)/2,0x0,ISM2MDone_DUMMY);
        }
        else
        {
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x0,0x4);
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_SC0,0x53,0x4,0x4,100000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x4,0x4);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x0,0x4);
        }
    }
    else
    {
        if(genIrqmode[E_HAL_SCL_INST_M2M])
        {
            DrvSclOsWaitEvent(DrvSclIrqGetIRQM2MEventID(), E_SCLIRQ_M2MEVENT_ACTIVEN, &u32Event, E_DRV_SCLOS_EVENT_MD_OR,100);
        }
        else
        {
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x0,0x4);
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_SC0,0x53,0x4,0x4,100000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x4,0x4);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x50,0x0,0x4);
        }
    }
    return bRet;
}
bool _MHalVpeSclSetWaitDone(MHalSclHandleConfig_t *pstScHandler, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, MHalVpeWaitDoneType_e enWait)
{
    u16 u16dummy = 0;
    u16 u16Waitval = 0;
    u16 u16Waitbit = 0;
    bool bRet = VPE_RETURN_OK;
    _MHalVpeSclKeepCmdqFunction(pstCmdQInfo);
    // wait done
    if((pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE && pstScHandler->eInst==E_MHAL_VPE_SCL_MAIN) &&
    enWait == E_MHAL_VPE_WAITDONE_MDWINONLY)
    {
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0xB,0,0x4000);
    }
    else
    {
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0xB,0x4000,0x4000);
    }
    if((pstScHandler->eOutMode&E_MHAL_SCL_OUTPUT_MODE_PORT2))
    {
        u16Waitval = 0xFFFB;
        u16Waitbit = 0x4;
    }
    else
    {
        u16Waitval = 0xFFFD;
        u16Waitbit = 0x2;
    }
    if(_VpeIsCmdq(pstCmdQInfo))
    {
        VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s Inst %hu CMDQ\n",  __FUNCTION__,pstScHandler->u16InstId);
        if(enWait==E_MHAL_VPE_WAITDONE_DMAONLY)
        {
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1C,u16Waitval,0xFFFF);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,u16Waitval,0xFFFF);
            pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo,E_MHAL_CMDQEVE_SC_TRIG013);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0xFFFF,0xFFFF);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }
        else if(enWait == E_MHAL_VPE_WAITDONE_MDWINONLY)
        {
            pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo,E_MHAL_CMDQEVE_S0_MDW_W_DONE);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }
        else
        {
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1C,u16Waitval,0xFFFF);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,u16Waitval,0xFFFF);
            pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo,E_MHAL_CMDQEVE_SC_TRIG013);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0xFFFF,0xFFFF);
            pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo,E_MHAL_CMDQEVE_S0_MDW_W_DONE);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }
    }
    else
    {
        if(enWait==E_MHAL_VPE_WAITDONE_DMAONLY)
        {
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_SC0,0x1F,u16Waitbit,u16Waitbit,100000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,u16Waitbit,u16Waitbit);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0x0,u16Waitbit);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }
        else if(enWait == E_MHAL_VPE_WAITDONE_MDWINONLY)
        {
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_DWIN,0x70,0x8000,0x8000,100000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_DWIN,0x70,0x4000,0x4000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_DWIN,0x70,0x0,0x4000);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }
        else
        {
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_SC0,0x1F,u16Waitbit,u16Waitbit,100000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,u16Waitbit,u16Waitbit);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0x1E,0x0,u16Waitbit);
            _VpeIspRegPollWait(pstCmdQInfo,BANK_SC_DWIN,0x70,0x8000,0x8000,1000); //mdwin
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_DWIN,0x70,0x4000,0x4000);
            _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_DWIN,0x70,0x0,0x4000);
            _VpeIspRegWMsk(pstCmdQInfo,(ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2,ISCMDQ_IDLEDUMMY,ISCMDQ_IDLEDUMMY);
        }


    }
    if(gbHistints || pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE)
    {
        if(pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT0])
        {
            u16dummy |= ISPORT0_USEDUMMY;
        }
        if(pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT1])
        {
            u16dummy |= ISPORT1_USEDUMMY;
        }
        if(pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT2])
        {
            u16dummy |= ISPORT2_USEDUMMY;
        }
        if(pstScHandler->stCtx.bEn[E_MHAL_SCL_OUTPUT_PORT3])
        {
            u16dummy |= ISPORT3_USEDUMMY;
        }
        _VpeIspRegWMsk(pstCmdQInfo,(ISPORTALL_DUMMYReg>>8),(ISPORTALL_DUMMYReg&0xFF)/2,u16dummy,ISPORTALL_DUMMY);
    }
    if((pstScHandler->enTopIn == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE&& pstScHandler->eInst==E_MHAL_VPE_SCL_MAIN) &&
    _VpeIsCmdq(pstCmdQInfo))
    {
        pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo,ISSW_IDLEDUMMYReg,ISSW_IDLEDUMMY,ISSW_IDLEDUMMY);
        pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo,ISDV_DUMMYReg,ISDVuse_DUMMY,ISDVuse_DUMMY);
        pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits(pstCmdQInfo,(((u32)BANK_SC_SC0<<8) |0x28) ,0x10,0x10,0); //polling vsync
        pstCmdQInfo->MHAL_CMDQ_WriteRegCmdqMask(pstCmdQInfo,ISDV_DUMMYReg,0,ISDVuse_DUMMY);
        // avoid to jpe be block.(if jpe use vpe cmdq)
        if(pstScHandler->stCtx.bEnImi == 0)
        {
            pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits(pstCmdQInfo,ISINPROCESSQ_DUMMYReg,
                _VpeIspRegR((ISINPROCESSQ_DUMMYReg>>8),(ISINPROCESSQ_DUMMYReg&0xFF)/2),ISINPROCESSQ_DUMMY,0);//polling process cnt
            pstCmdQInfo->MHAL_CMDQ_CmdqPollRegBits(pstCmdQInfo,ISSW_IDLEDUMMYReg,ISSW_IDLEDUMMY,ISSW_IDLEDUMMY,1);// polling blanking for Iq
        }
        else
        {
            DrvSclIrqSetDmaOff(pstScHandler->s32Handle[E_HAL_SCLHVSP_ID_1],pstScHandler->stCtx.bEnImi);
        }
    }
    return bRet;
}
bool _MHalVpeSclIsPortBindSuccess(MHalVpeDmaPort_e enPort,MHalSclHandleConfig_t* pstScHandler)
{
    if(enPort==E_MHAL_SCL_OUTPUT_PORT0)
    {
        if(!(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT0))
        {
            VPE_ERR("[VPE]Port:%d Mode :%d not support\n",enPort,pstScHandler->eOutMode);
            return VPE_RETURN_ERR;
        }
    }
    if(enPort==E_MHAL_SCL_OUTPUT_PORT1)
    {
        if(!(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT1))
        {
            VPE_ERR("[VPE]Port:%d Mode :%d not support\n",enPort,pstScHandler->eOutMode);
            return VPE_RETURN_ERR;
        }
    }
    if(enPort==E_MHAL_SCL_OUTPUT_PORT2)
    {
        if(!(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT2))
        {
            VPE_ERR("[VPE]Port:%d Mode :%d not support\n",enPort,pstScHandler->eOutMode);
            return VPE_RETURN_ERR;
        }
    }
    if(enPort==E_MHAL_SCL_OUTPUT_PORT3)
    {
        if(!(pstScHandler->eOutMode & E_MHAL_SCL_OUTPUT_MODE_PORT3))
        {
            VPE_ERR("[VPE]Port:%d Mode :%d not support\n",enPort,pstScHandler->eOutMode);
            return VPE_RETURN_ERR;
        }
    }
    return VPE_RETURN_OK;
}

MS_BOOL MHalVpeCreateIspInstance(const MHalAllocPhyMem_t *pstAlloc,const MhalVpeIspCreate_t *pstCfg,void **pCtx)
{
	bool bRet = VPE_RETURN_OK;
	MHalVpeCtx_t  *pMHalVpeCtx;
	if(pstCfg->eInput <= E_MHAL_VPE_INPUT_MODE_INVALID)
		return VPE_RETURN_ERR;

	*pCtx = (void*)DrvSclOsVirMemalloc(sizeof(MHalVpeCtx_t));
	DrvSclOsMemset(*pCtx,0,sizeof(MHalVpeCtx_t));

	if(!gpVpeIoBase) //workaround before MHalGlobalInit/MHalGlobalDeinit released.
	{
		//gpVpeIoBase = ioremap(0x1F000000,0x400000);//remap whole IO region
		gpVpeIoBase = (void*) 0xFD000000;
	}
	pMHalVpeCtx =(MHalVpeCtx_t*) *pCtx;
	pMHalVpeCtx->eMode = pstCfg->eInput;
	pMHalVpeCtx->eHDRMode = pstCfg->eHDRMode;
	pMHalVpeCtx->pLibcameraIf = LibcameraIfGet();
	pMHalVpeCtx->pMemAlloc = pstAlloc;

    return bRet;
}
bool MHalVpeDestroyIspInstance(void *pCtx)
{
    bool bRet = VPE_RETURN_OK;
    MHalVpeCtx_t  *ctx = (MHalVpeCtx_t*) pCtx;
    if(ctx->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE && ctx->pLibcameraIf->CameraClose)
    {
#if defined(SCLOS_TYPE_LINUX_TEST)
#else
        if(ctx->pExtHDRBuf && ctx->eMode == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE && ctx->eHDRMode==E_MHAL_HDR_TYPE_DOL)
            ctx->pExtHDRBuf->Release(ctx->pExtHDRBuf);
#endif
        ctx->pLibcameraIf->CameraClose();
    }
    DrvSclOsVirMemFree(pCtx);
    return bRet;
}

CameraPixelFormat_e _MHalVpeIspPixelFormatToBPP(MHalPixelFormat_e eFmt)
{
    CameraPixelFormat_e eRet;
    switch(eFmt)
    {
    /* Bayer 8 */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG:
        eRet = eCameraPixelFormat_Bayer8BPP;
        break;
    /* Bayer 10 */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG:
        eRet = eCameraPixelFormat_Bayer10BPP;
        break;
    /* Bayer 16 */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG:
        eRet = eCameraPixelFormat_Bayer12BPP;
        break;
    /* Bayer 14 */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_BG:
        eRet = eCameraPixelFormat_Invalid;
        break;
    /* Bayer 16 */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG:
        eRet = eCameraPixelFormat_Bayer16BPP;
        break;
    case E_MHAL_PIXEL_FRAME_YUV422_YUYV:
        eRet = eCameraPixelFormat_YUYV;
        break;
    case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        eRet = eCameraPixelFormat_NV12;
        break;
    default:
        eRet = eCameraPixelFormat_Invalid;
        break;
    }
    return eRet;
}

CameraBayerID_e _MHalVpeIspPixelFormatToBayerID(MHalPixelFormat_e eFmt)
{
    CameraPixelFormat_e eRet;
    switch(eFmt)
    {
    /* Bayer RG */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG:
        eRet = eCamerBayerID_RG;
        break;
    /* Bayer GB */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB:
        eRet = eCamerBayerID_GB;
        break;
    /* Bayer GR */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR:
        eRet = eCamerBayerID_GR;
        break;
    /* Bayer BG */
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG:
        eRet = eCamerBayerID_BG;
        break;
    default:
        eRet = eCamerBayerID_Invalid;
        break;
    }
    return eRet;
}

/* ISP command queue */
typedef struct
{
    MHAL_CMDQ_CmdqInterface_t *ptMhalCmdq;
    IspCmdqIf_t tIspCmdq;
}CameraCmdqIf_t;
//CameraCmdqIf_t g_CmdQ = {0};

static void CameraCmdqRegWM(struct IspCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal,unsigned short nMask)
{
    CameraCmdqIf_t *ptHandle = CAM_OS_CONTAINER_OF(this,CameraCmdqIf_t,tIspCmdq);
    MS_S32 nRet;
    //pr_info("Write CMDQ: reg=0x%X, val=0x%X, mask=0x%X\n",uRiuAddr,uVal,nMask);
    nRet = ptHandle->ptMhalCmdq->MHAL_CMDQ_WriteRegCmdqMask( ptHandle->ptMhalCmdq, (u32)uRiuAddr, (u32)uVal, (u32)nMask);
    if(nRet!=MHAL_SUCCESS)
    {
        pr_err("CmdQ CameraCmdqRegWM error %d\n",nRet);
    }
    //pr_info("CmdQ CameraCmdqRegWM=0x%X, Val=0x%X, Mask=0x%X\n",uRiuAddr,uVal,nMask);
}

static void CameraCmdqRegW(struct IspCmdqIf_t* this,unsigned int uRiuAddr,unsigned short uVal)
{
    CameraCmdqIf_t *ptHandle = CAM_OS_CONTAINER_OF(this,CameraCmdqIf_t,tIspCmdq);
    MS_S32 nRet;
    //pr_info("Write CMDQ: reg=0x%X, val=0x%X\n",(u32)uRiuAddr,(u32)uVal);
    nRet = ptHandle->ptMhalCmdq->MHAL_CMDQ_WriteRegCmdq( ptHandle->ptMhalCmdq, uRiuAddr, uVal);
    if(nRet!=MHAL_SUCCESS)
    {
        pr_err("CmdQ CameraCmdqRegW error %d\n",nRet);
    }
    //pr_info("CmdQ CameraCmdqRegW=0x%X, Val=0x%X\n",uRiuAddr,uVal);
}
#if 0
static IspCmdqIf_t* _GetCmdqIF(void)
{
    MHAL_CMDQ_BufDescript_t stCfg;
    MHAL_CMDQ_CmdqInterface_t *pCfg=NULL;
    stCfg.u16MloadBufSizeAlign = 32;
    stCfg.u32CmdqBufSizeAlign = 32;
    stCfg.u32CmdqBufSize = 0x40000;
    stCfg.u32MloadBufSize = 0x10000;
    pCfg = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE,&stCfg,0);

    if(pCfg)
    {
        g_CmdQ.ptMhalCmdq = pCfg;
        g_CmdQ.tIspCmdq.RegWM = CameraCmdqRegWM;
        g_CmdQ.tIspCmdq.RegW = CameraCmdqRegW;
    }
    else
    {
        pr_err("ISPMID failed to get cmdq service");
    }

    return &g_CmdQ.tIspCmdq;
}
#endif
/*------------------------------------------------------------------------------*/

bool _MHalVpeIspProcessBayer(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo)
{
    bool bRet = VPE_RETURN_OK;
    CameraRawFetchCfg_t tCfg;
    CameraCmdqIf_t tCmdq;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;

    tCmdq.ptMhalCmdq = pstCmdQInfo;
    tCmdq.tIspCmdq.RegWM = CameraCmdqRegWM;
    tCmdq.tIspCmdq.RegW = CameraCmdqRegW;

    tCfg.eBayerID = _MHalVpeIspPixelFormatToBPP(ctx->ePixelFmt);
    tCfg.ePixelFmt = _MHalVpeIspPixelFormatToBPP(ctx->ePixelFmt);

    tCfg.nPitch = pstVidInfo->u32Stride[0]/2;
    tCfg.nWidth = ctx->uImgW/2;
    tCfg.nHeight = ctx->uImgH;
    tCfg.nPhysAddr = pstVidInfo->u64PhyAddr[0]+0x20000000;

    /* Unmask DMA irq */
#define MASKOF_RDMA0_DONE (0x0040)
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x035,0x0,MASKOF_RDMA0_DONE);

    //ctx->pLibcameraIf->CameraChangeCmdqIF(&tCmdq.tIspCmdq);
    ctx->pLibcameraIf->MHalChangeCmdqIF(pstCmdQInfo);
    /* TODO: Calling CameraContextSwitch with channel number*/
    ctx->pLibcameraIf->CameraContextSwitch(0);
    ctx->pLibcameraIf->CameraRawFetch(eRawFetchNode_P0HEAD,&tCfg);
    ctx->pLibcameraIf->CameraRawFetchTrigger(eRawFetchNode_P0HEAD);
    //ctx->pLibcameraIf->CameraChangeCmdqIF(NULL);
    ctx->pLibcameraIf->MHalChangeCmdqIF(NULL);

    /* Wait RDMA done done*/
    _VpeIspRegPollWait(pstCmdQInfo,BANK_DMA_IRQ,0x038,MASKOF_RDMA0_DONE,0x0040,200000);
    /* Clear DMA interrupt status */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x037,0x0040,MASKOF_RDMA0_DONE);
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x037,0x0,MASKOF_RDMA0_DONE);
    /* Mask DMA status */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x035,0x0040,MASKOF_RDMA0_DONE);
#undef MASKOF_RDMA0_DONE

    return bRet;
}

#define LIMIT_BITS(x, bits)     ((x) > ((1<<(bits))-1) ? (((1<<(bits))-1)) : (x))
int GetRDMAPitch(MHalPixelFormat_e eFmt,int nImgW)
{
    int denomt = 128;
    int nPitch = 0;
    switch(_MHalVpeIspPixelFormatToBPP(eFmt))
    {
        case eCameraPixelFormat_YUYV:
        case eCameraPixelFormat_Bayer16BPP:
            denomt = 16;
            nPitch = (nImgW+15)>>4; //16 data pack 1 miu data
            break;
        case eCameraPixelFormat_Bayer12BPP:
            denomt = 32;
            nPitch = LIMIT_BITS((u32)(((nImgW + denomt - 1) / denomt) * 3), 12);
            break;
        case eCameraPixelFormat_Bayer10BPP:
            denomt = 64;
            nPitch = LIMIT_BITS((u32)(((nImgW + denomt - 1) / denomt) * 5), 12);
            break;
        case eCameraPixelFormat_Bayer8BPP:
            denomt = 16;
            nPitch = LIMIT_BITS((u32)(((nImgW + denomt - 1) / denomt) * 1), 12);
            break;
        default:
            return -1;
    }
    return nPitch;
}

int GetRDMADataPrecision(MHalPixelFormat_e eFmt)
{
    switch(_MHalVpeIspPixelFormatToBPP(eFmt))
    {
        case eCameraPixelFormat_YUYV:
        case eCameraPixelFormat_Bayer16BPP:
            return 0;
            break;
        case eCameraPixelFormat_Bayer12BPP:
            return 1;
            break;
        case eCameraPixelFormat_Bayer10BPP:
            return 2;
            break;
        case eCameraPixelFormat_Bayer8BPP:
            return 3;
            break;
        default:
            return -1;
            break;
    }
}

#define ISP7_REG09_DIPR_NO_ROT     0x0001 //bit 0
#define ISP7_REG09_DIPR_SW_TRIGGER 0x0002 //bit 1
#define ISP7_REG09_DIPR_420_LINEAR 0x0004 //bit 2
#define ISP7_REG09_DIPR_422_ROT    0x0008 //bit 3
#define ISP7_REG09_DIPR_ROT_DIR_CCW 0x0010 //bit 4
#define ISP7_REG09_DIPR_CROP_EN    0x0080 //bit 7
#define ISP7_REG09_DIPR_HW_EN      0x0800 //bit 11
#define _ALIGN32(x) (((x+31)/32)*32)

bool _MHalVpeIspProcessYUV422(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo)
{
    bool bRet = VPE_RETURN_OK;
    u32 nBaseAddr;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;

	VPE_DBG(VPE_DBG_LV_ISP(),"_MHalVpeIspProcessYUV422 , w=%d, h=%d, Stride=%d\n",ctx->uImgW,ctx->uImgH,pstVidInfo->u32Stride[0]);

    /* Enable ISP for DMA */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x0,0x1,0x1);

    /* YUV422 input mode */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x10,0x0000,MASKOF(reg_isp_sensor_rgb_in));

    /* YC16 bit mode */
    //_VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x11,0x0050,MASKOF(reg_isp_sensor_yc16bit));

    /* ISP pipe0 raw fetch mode */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x08,0x0002,MASKOF(reg_p0h_rawfetch_mode));

    /* do not reset isp at frame end */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x66,0x0081, MASKOF(reg_fend_rstz_mode)
                                                  |MASKOF(reg_isp_dp_rstz_mode)
                                                  |MASKOF(reg_m3isp_rawdn_clk_force_dis)
                                                  |MASKOF(reg_isp_dp_clk_force_dis)
                   );
    /*  420 to 422 dma disable */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x07,0x0700,MASKOF(reg_mload_miu_sel)
                                                    |MASKOF(reg_40to42_dma_en)//[15] yuv fetch miu sel ; [14] 420 to 422 dma output enable
                                                    |MASKOF(reg_data_store_src_sel) //ch4 test
                                                    );
    /* YUV order:YUYV , YC separate mode:OFF*/
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x11,0x0050,MASKOF(reg_isp_sensor_yc16bit)
                                                    |MASKOF(reg_isp_sensor_yuv_order)
                                                    );

    /* ISP pipe0 rmux sel SIF*/
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x04,0x0230,MASKOF(reg_isp_if_rmux_sel));

    /* Crop X*/
    _VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x12,ctx->uCropX);
    /* Crop Y*/
    _VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x13,ctx->uCropY);
    /* Crop W*/
    _VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x14,ctx->uImgW-1);
    /* Crop H*/
    _VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x15,ctx->uImgH-1);

    /* RDMA enable*/
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x00,0x0201);
    /* Width */
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x04,ctx->uImgW-1);
    /* Height */
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x05,ctx->uImgH-1);
    /* Start X */
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x06,ctx->uCropX);
    /* RDMA Pitch */
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x01,GetRDMAPitch(ctx->ePixelFmt,pstVidInfo->u32Stride[0]/2));
    /* RDMA data mode, and data align 16 bit mode */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMAG0,0x18,0x01|(GetRDMADataPrecision(ctx->ePixelFmt)<<1),0x7);
    /* DMA buffer address */
    nBaseAddr = pstVidInfo->u64PhyAddr[0];
    nBaseAddr >>= 5;
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x08,nBaseAddr&0xFFFF);
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x09,(nBaseAddr>>16)&0x07FF);

#define MASKOF_RDMA0_DONE (0x0040)
    /* Unmask DMA irq */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x035,0x0,MASKOF_RDMA0_DONE);

    /* RDMA trigger */
    _VpeIspRegW(pstCmdQInfo,BANK_DMAG0,0x10,1); //#Bit 2, rdma sw trigger
    /* Wait RDMA done done*/
    _VpeIspRegPollWait(pstCmdQInfo,BANK_DMA_IRQ,0x038,MASKOF_RDMA0_DONE,0x0040,200000);
    /* Clear DMA interrupt status */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x037,0x0040,MASKOF_RDMA0_DONE);
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x037,0x0,MASKOF_RDMA0_DONE);
    /* Mask DMA status */
    _VpeIspRegWMsk(pstCmdQInfo,BANK_DMA_IRQ,0x035,0x0040,MASKOF_RDMA0_DONE);
#undef MASKOF_RDMA0_DONE
    return bRet;
}

bool _MHalVpeIspProcessYUV420(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo)
{
    bool bRet = VPE_RETURN_OK;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;
    u16 trigger_ctl = 0;
    u16 tmp=0;
    u32 n=0;

    if(pstVidInfo->u32Stride[0]%32 !=0)
    {
        VPE_DBG(VPE_DBG_LV_ISP(),"Error, Stride must be 32 bytes aligned.");
        SCLOS_BUG();
    }
    if(ctx->uImgW > pstVidInfo->u32Stride[0])
    {
        VPE_DBG(VPE_DBG_LV_ISP(),"Error, Invalid image stride value.");
        SCLOS_BUG();
    }
    if(ctx->uImgW%32)
    {
        VPE_ERR("Image width:%d is not 32 bytes aligned.\n",ctx->uImgW);
    }
    if(ctx->uImgH%32)
    {
        VPE_DBG(VPE_DBG_LV_ISP(), "Image height:%d is not 32 bytes aligned.\n",ctx->uImgH);
    }
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP7,0x1A,0x1,0x1);
    //_VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x10,0x0000); //YUV422 input mode
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x10,0x0000,MASKOF(reg_isp_sensor_rgb_in)); //YUV422 input mode
    //_VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x66,0x0081); //do not isp at frame end
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x66,0x0081, MASKOF(reg_fend_rstz_mode)
                                                  |MASKOF(reg_isp_dp_rstz_mode)
                                                  |MASKOF(reg_m3isp_rawdn_clk_force_dis)
                                                  |MASKOF(reg_isp_dp_clk_force_dis)
                                                   ); //do not isp at frame end

    //_VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x07,0xC700); //[15] yuv fetch miu sel ; [14] 420 to 422 dma output enable
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x07,0xC700,MASKOF(reg_mload_miu_sel)
                                                    |MASKOF(reg_40to42_dma_en)//[15] yuv fetch miu sel ; [14] 420 to 422 dma output enable
                                                    |MASKOF(reg_data_store_src_sel) //ch4 test
                                                    );
    //_VpeIspRegW(pstCmdQInfo,BANK_ISP0,0x11,0x0050);
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x11,0x0050,MASKOF(reg_isp_sensor_yc16bit)
                                                    |MASKOF(reg_isp_sensor_yuv_order)
                                                    );

    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x3E,0x08F7); //RDMAY MIU parameter
    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x3F,0x0000); //RDMAY output blanking
    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x4E,0x08F7); //RDMAC MIU parameter
    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x4F,0x0000); //RDMAC output blanking

        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x35,0x0201); //#Bit 0, dmagy rdma enable
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x45,0x0201); //#Bit 0, dmagc rdma enable
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x09,0x0805); //#Bit 0, dipr_en(Set 1 for no rotate, 0 for rotate)
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x19,0x0001); //arbiter
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x0A,(pstVidInfo->u64PhyAddr[0]>>5)&0xFFFF); //Y buffer , L Addr
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x0B,(pstVidInfo->u64PhyAddr[0]>>21)&0xFFFF); //Y buffer , H Addr
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x0C,(pstVidInfo->u64PhyAddr[1]>>5)&0xFFFF); //UV , L Addr
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x0D,(pstVidInfo->u64PhyAddr[1]>>21)&0xFFFF); //UV , H Addr
        _VpeIspRegW(pstCmdQInfo,BANK_SC_FLIP,0x11,0);
        _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_DWIN,0x78,0,0x2);
        trigger_ctl |= ISP7_REG09_DIPR_NO_ROT;

        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x10,pstVidInfo->u32Stride[0]); //Frame Width
        //_VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x10,ctx->uImgW);           //Frame Width
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x11,ctx->uImgH);               //Frame Height
        /* Stride for MIU access*/
        tmp = (_ALIGN32(ctx->uImgW)/32);
        if(tmp>32)
        {
            for(n=2;n<=13;n++)
            {
                if(tmp%n==0 && tmp/n <=32)
                {
                    tmp /= n;
                    break;
                }
                if(n==13)
                {
                    VPE_ERR("nonsupported image width.");
                }
            }
        }
        tmp-=1;
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x13, ((tmp<<8)&0xFF00)|(tmp&0x00FF)); //request number once for 32 Byte MIU (32/32's factor minus 1 for yuv420 burst length)

        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x0E,0x0010); //ISP ROT output blanking (mul 2)

    if(pstVidInfo->u32Stride[0] > _ALIGN32(ctx->uImgW))
    {
        /* Enable Crop */
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x14,_ALIGN32(ctx->uImgW)); //Src Width
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x15,ctx->uImgH); //Src Height
        trigger_ctl |= ISP7_REG09_DIPR_HW_EN|ISP7_REG09_DIPR_420_LINEAR|ISP7_REG09_DIPR_SW_TRIGGER|ISP7_REG09_DIPR_CROP_EN;
    }
    else
    {
        /* No Crop */
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x14,0); //Src Width
        _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x15,0); //Src Height
        trigger_ctl |= ISP7_REG09_DIPR_HW_EN|ISP7_REG09_DIPR_420_LINEAR|ISP7_REG09_DIPR_SW_TRIGGER;
    }
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x65,0x0,MASKOF(reg_rdpath_swrst));    //group0 rdma reset enable
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x65,0x5,MASKOF(reg_rdpath_swrst));    //group0 rdma reset disable
    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x09,trigger_ctl); //#Bit 2, rdma sw trigger
#if 0
    /*
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x65,0x0,MASKOF(reg_rdpath_swrst));    //group0 rdma reset enable
    _VpeIspRegWMsk(pstCmdQInfo,BANK_ISP0,0x65,0x5,MASKOF(reg_rdpath_swrst));    //group0 rdma reset disable
    */
    _VpeIspRegWMsk(pstCmdQInfo,0x153B,0x035,0x0,0x4000);   //pipe1 reset enable
    _VpeIspRegW(pstCmdQInfo,BANK_ISP7,0x09,trigger_ctl); //#Bit 2, rdma sw trigger
    _VpeIspRegPollWait(pstCmdQInfo,0x153B,0x038,0x4000,0x4000,200000);
    //pstCmdQInfo->MHAL_CMDQ_CmdqAddWaitEventCmd(pstCmdQInfo,E_MHAL_CMDQEVE_ISP_TRIG);
    _VpeIspRegWMsk(pstCmdQInfo,0x153B,0x037,0x4000,0x4000);   //pipe1 reset enable
    _VpeIspRegWMsk(pstCmdQInfo,0x153B,0x037,0x0,0x4000);   //pipe1 reset enable
    _VpeIspRegWMsk(pstCmdQInfo,0x153B,0x035,0x4000,0x4000);   //pipe1 reset enable
#endif
    return bRet;
}


bool MHalVpeIspProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo)
{
    bool bRet = VPE_RETURN_OK;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;

    if(ctx->eMode == E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE)
        return VPE_RETURN_OK;
    //DrvSclOsDelayTask(8);//for process test
    switch(ctx->ePixelFmt)
    {
    case E_MHAL_PIXEL_FRAME_YUV422_YUYV:
        //bRet = VPE_RETURN_ERR;
        VPE_DBG(VPE_DBG_LV_ISP(),"Input YUV422 mode");
        return _MHalVpeIspProcessYUV422(pCtx,pstCmdQInfo,pstVidInfo);
        break;
    case E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        VPE_DBG(VPE_DBG_LV_ISP(),"Input YUV420 mode");
        _MHalVpeIspProcessYUV420(pCtx,pstCmdQInfo,pstVidInfo);
        break;
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB:
    case E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG:
        if(ctx->eMode == E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE)
            _MHalVpeIspProcessBayer(pCtx,pstCmdQInfo,pstVidInfo);
        break;
    case E_MHAL_ISP_INPUT_TYPE:
        bRet = VPE_RETURN_ERR;
        VPE_ERR("Unknown input type");
    default:
        break;
    }

    return bRet;
}

bool MHalVpeIspDbgLevel(void *p)
{
    bool bRet = VPE_RETURN_OK;
    gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_ISP] = *((bool *)p);
    VPE_ERR("[VPEISP]%s Dbg Level = %hhx\n",__FUNCTION__,gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_ISP]);
    return bRet;
}
#if defined(SCLOS_TYPE_LINUX_TEST)
#else
BufPtr_t *DrvVifGetRtHdrBufHandle(void);
#endif
bool _MHalIspInit(void *pCtx,MHalVpeTopInputMode_e eMode,MHalPixelFormat_e ePixelFmt, u32 nWidth, u32 nHeight)
{
    bool bRet = VPE_RETURN_ERR;
    CameraOpenAttr_t tAttr;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;

    memset(&tAttr,0,sizeof(tAttr));
    tAttr.nCropX = 0;
    tAttr.nCropY = 0;
    tAttr.nCropWidth = nWidth;
    tAttr.nCropHeight = nHeight;

    tAttr.eBayerID = _MHalVpeIspPixelFormatToBayerID(ctx->ePixelFmt);
    tAttr.ePixelFmt = _MHalVpeIspPixelFormatToBPP(ePixelFmt);
    tAttr.ptMemAlloc = (CameraMemAlloctor_t*) ctx->pMemAlloc;

    tAttr.nOutCropX = 0;
    tAttr.nOutCropY = 0;
    tAttr.nOutCropHeight = nHeight;
    tAttr.nOutCropWidth = nWidth;
    tAttr.nOutHeight = nHeight;
    tAttr.nOutWidth = nWidth;
    tAttr.nCh = 0;

    if(tAttr.ePixelFmt == eCameraPixelFormat_Invalid)
        return VPE_RETURN_ERR;

    switch(eMode)
    {
    case E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE:
        bRet = VPE_RETURN_OK;
        break;
    case E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE:
        tAttr.eMode = eFrameModeNoHDR;
        if(ctx->pLibcameraIf->MhalCameraOpen(&tAttr)==0)
            bRet = VPE_RETURN_OK;
        break;
    case E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE:
        {
            switch(ctx->eHDRMode)
            {
            case E_MHAL_HDR_TYPE_OFF:
                tAttr.eMode = eRealTimeModeNoHDR;
                if(ctx->pLibcameraIf->MhalCameraOpen(&tAttr)==0)
                    bRet = VPE_RETURN_OK;
                break;
#if defined(SCLOS_TYPE_LINUX_TEST)
#else
            case E_MHAL_HDR_TYPE_DOL:
                ctx->pExtHDRBuf = DrvVifGetRtHdrBufHandle();
                if(ctx->pExtHDRBuf )
                {
                    ctx->pExtHDRBuf->AddRef(ctx->pExtHDRBuf);
                }
                else
                {
                    pr_err("***%s Buffer pointer is NULL ***\n",__FUNCTION__);
                }

                tAttr.eMode = eRealTimeModeHDR;
                tAttr.nHDRFB = (u32)BufPtr(ctx->pExtHDRBuf);
                if(ctx->pLibcameraIf->MhalCameraOpen(&tAttr)==0)
                    bRet = VPE_RETURN_OK;
                break;
#endif
            default:
                pr_err("HDR mode %d not supported.\n",ctx->eHDRMode);
                break;
            }
        }
        break;
    default:
        bRet = VPE_RETURN_ERR;
        break;
    }
    return bRet;
}

bool MHalVpeIspInputConfig(void *pCtx, const MHalVpeIspInputConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalVpeCtx_t *ctx = (MHalVpeCtx_t*) pCtx;
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    ctx->uCropX = 0;
    ctx->uCropY = 0;
    ctx->uImgW = pCfg->u32Width;
    ctx->uImgH = pCfg->u32Height;
    //ctx->enInType = pCfg->enInType;
    ctx->ePixelFmt = pCfg->ePixelFormat;
    //ctx->eBayerID = pCfg->eBayerID;
    VPE_DBG(VPE_DBG_LV_ISP(),"%s ImgW=%d, ImgH=%d eMode:%d\n",__FUNCTION__,ctx->uImgW,ctx->uImgH,ctx->eMode);
    switch(ctx->eMode)
    {
    case E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE:
    case E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE:
        bRet = _MHalIspInit(    pCtx,
                                ctx->eMode,
                                ctx->ePixelFmt,
                                ctx->uImgW,ctx->uImgH
                           );
        break;
    default:
        break;
    }
    return bRet;
}

bool MHalVpeCreateIqInstance(const MHalAllocPhyMem_t *pstAlloc ,const MHalVpeSclWinSize_t *pstMaxWin, MHalVpeTopInputMode_e eMode ,void **pCtx)
{
	bool bRet = VPE_RETURN_OK;
	s16 s16Idx = -1;
	DrvSclVipIoSetMaskOnOff_t stCfg;
	MHalVpeIqOnOff_t stOnOff;
	MHalVpeIqConfig_t stIqCfg;
	//keep Alloc
	_MHalVpeKeepAllocFunction(pstAlloc);
	//init
	_MHalVpeInit();
	//check mode
	if(eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_ERR;
	//find new inst
	bRet = _MHalVpeFindEmptyIqInst(&s16Idx);
	if(bRet == VPE_RETURN_ERR || s16Idx==-1)
	{
	  return VPE_RETURN_ERR;
	}
	//open device
	bRet = _MHalVpeOpenIqDevice(s16Idx);
	if(bRet == VPE_RETURN_ERR)
	{
	  return bRet;
	}

	if(eMode <= E_MHAL_VPE_INPUT_MODE_INVALID)
		return VPE_RETURN_ERR;

	pgstIqHandler[s16Idx]->eMode = eMode;

	//create inst
	bRet = _MHalVpeCreateIqInst(&(pgstIqHandler[s16Idx]->s32Handle));
	if(bRet == VPE_RETURN_ERR)
	{
	  _DrvSclVipIoRelease((pgstIqHandler[s16Idx]->s32Handle));
	  _MHalVpeCleanIqInstBuffer(pgstIqHandler[s16Idx]);
	  return bRet;
	}
	bRet = _MHalVpeReqmemConfig(pgstIqHandler[s16Idx]->s32Handle,pstMaxWin);
	pgstIqHandler[s16Idx]->bNrFBAllocate = bRet;
	if(bRet == VPE_RETURN_ERR)
	{
	  VPE_ERR("[VPEIQ]%s Create Inst %hu Allocate Fail\n",	__FUNCTION__,pgstIqHandler[s16Idx]->u16InstId);
	  bRet = VPE_RETURN_OK;
	}
	//alloc Wdr Mload buffer
	_DrvSclVipIoReqWdrMloadBuffer(pgstIqHandler[s16Idx]->s32Handle);
	//Default Setting (create Reg tbl and inquire tbl)
	//ToDo
	DrvSclOsMemset(&stOnOff,0,sizeof(MHalVpeIqOnOff_t));
	MHalVpeIqOnOff(pgstIqHandler[s16Idx],&stOnOff);
	DrvSclOsMemset(&stIqCfg,0,sizeof(MHalVpeIqConfig_t));
	MHalVpeIqConfig(pgstIqHandler[s16Idx], &stIqCfg);
	stCfg.bOnOff = 1;
	stCfg.enMaskType = E_DRV_SCLVIP_IO_MASK_WDR;
	_DrvSclVipIoSetMaskOnOff(pgstIqHandler[s16Idx]->s32Handle,&stCfg);
	stCfg.enMaskType = E_DRV_SCLVIP_IO_MASK_NR;
	_DrvSclVipIoSetMaskOnOff(pgstIqHandler[s16Idx]->s32Handle,&stCfg);
	//[wdr] ===================================================
	//assign Ctx
	//======void *pCtx
	//======func(&pCtx)
	*pCtx = pgstIqHandler[s16Idx];
	VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2), "[VPEIQ]%s Create Inst %hu Success\n",  __FUNCTION__,pgstIqHandler[s16Idx]->u16InstId);
	return bRet;
}

bool MHalVpeDestroyIqInstance(void *pCtx)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    //void *pCtx = pCreate;
    //func(pCtx)
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;
    //destroy inst
    bRet = _MHalVpeDestroyIqInst(&(pstIqHandler->s32Handle));
    if(bRet == VPE_RETURN_ERR)
    {
        return bRet;
    }
    //close device
    _DrvSclVipIoRelease(pstIqHandler->s32Handle);
    //clean scl inst
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2), "[VPEIQ]%s Destroy Inst %hu Success\n",  __FUNCTION__,pstIqHandler->u16InstId);
    _MHalVpeCleanIqInstBuffer(pstIqHandler);
    return bRet;
}

#define VPE_NE_TEST_ON 0

bool MHalVpeIqProcess(void *pCtx, const MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    NoiseEst_t ne_handle;
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoAipConfig_t stCfg;
    MHAL_CMDQ_CmdqInterface_t stCmdQInfo2;
    static u8 u32Vir_data[QMAP_PROCESS_SIZE];
    static u8 u32Vir_dataNE[QMAP_PROCESS_NE_SIZE];
    u8 *pNRBuf;
    //for debug
    static u8 fcnt = 0;
    u8 *p8;
    u16 *p16;
    u32 *p32;

	if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    if(pstCmdQInfo)
    {
        DrvSclOsMemcpy(&stCmdQInfo2,pstCmdQInfo,sizeof(MHAL_CMDQ_CmdqInterface_t));
        _MHalVpeIqKeepCmdqFunction(&stCmdQInfo2);
    }
    else
    {
        DrvSclOsMemset(&stCmdQInfo2,0,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }

    if (pstIqHandler->pvNrBuffer == NULL) {
        VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s IpvNrBuffer = NULL\n",  __FUNCTION__);
        VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);
        return bRet;
    }

    //[wdr] ===================================================
    //set_script_wdr_table(&stCmdQInfo2);
    //set_script_wdr_reg(&stCmdQInfo2);


    //[ne] noise est ==========================================
    DrvSclOsMemset(&ne_handle,0,sizeof(NoiseEst_t));
    pNRBuf = (u8*)pstIqHandler->pvNrBuffer;

    //[ne] 1. get noise est. reg
    DrvSclOsMemcpy(&ne_handle.in_reg_blk_sad_min_tmp_Y[0],  &pNRBuf[NEBUF_SADY], sizeof(ne_handle.in_reg_blk_sad_min_tmp_Y));
    DrvSclOsMemcpy(&ne_handle.in_reg_sum_noise_mean_Y[0],   &pNRBuf[NEBUF_SUMY], sizeof(ne_handle.in_reg_sum_noise_mean_Y));
    DrvSclOsMemcpy(&ne_handle.in_reg_count_noise_mean_Y[0], &pNRBuf[NEBUF_CNTY], sizeof(ne_handle.in_reg_count_noise_mean_Y));
    DrvSclOsMemcpy(&ne_handle.in_reg_blk_sad_min_tmp_C[0],  &pNRBuf[NEBUF_SADC], sizeof(ne_handle.in_reg_blk_sad_min_tmp_C));
    DrvSclOsMemcpy(&ne_handle.in_reg_sum_noise_mean_C[0],   &pNRBuf[NEBUF_SUMC], sizeof(ne_handle.in_reg_sum_noise_mean_C));
    DrvSclOsMemcpy(&ne_handle.in_reg_count_noise_mean_C[0], &pNRBuf[NEBUF_CNTC], sizeof(ne_handle.in_reg_count_noise_mean_C));

    DrvSclOsMemcpy(&ne_handle.out_reg_blk_sad_min_Y[0], &pstIqHandler->pu16NE_blk_sad_min_Y[0], sizeof(ne_handle.out_reg_blk_sad_min_Y));
    DrvSclOsMemcpy(&ne_handle.out_reg_blk_sad_min_C[0], &pstIqHandler->pu16NE_blk_sad_min_C[0], sizeof(ne_handle.out_reg_blk_sad_min_Y));
    DrvSclOsMemcpy(&ne_handle.out_reg_noise_mean_Y[0], &pstIqHandler->pu16NE_noise_mean_Y[0], sizeof(ne_handle.out_reg_noise_mean_Y));
    DrvSclOsMemcpy(&ne_handle.out_reg_noise_mean_C[0], &pstIqHandler->pu16NE_noise_mean_C[0], sizeof(ne_handle.out_reg_noise_mean_C));

    ne_handle.in_blk_sample_num_target  = pNRBuf[NEBUF_TARGET] + (pNRBuf[NEBUF_TARGET+1]<<8); // 32768;
    ne_handle.in_noise_mean_th_gain     = pNRBuf[NEBUF_TH_GAIN]; // 6;
    ne_handle.in_inten_blknum_lb        = pNRBuf[NEBUF_BLK_NUM]; // 16;
    ne_handle.in_blk_sad_min_lb         = pNRBuf[NEBUF_SAD_MIN]; // 32;
    ne_handle.in_noise_mean_lb          = pNRBuf[NEBUF_NOISE_MEAN]; // 32;
    ne_handle.in_learn_rate             = pNRBuf[NEBUF_LEARN]; // 2;
    ne_handle.in_dnry_gain              = pNRBuf[NEBUF_YGAIN]; // 4;
    ne_handle.in_dnrc_gain              = pNRBuf[NEBUF_CGAIN]; // 4

    //[ne] 2. calc
    _IQNoiseEstimationDriver(
        ne_handle.in_reg_blk_sad_min_tmp_Y, ne_handle.in_reg_sum_noise_mean_Y, ne_handle.in_reg_count_noise_mean_Y,
        ne_handle, ne_handle.out_reg_blk_sad_min_Y, ne_handle.out_reg_noise_mean_Y, ne_handle.out_reg_noise_mean_th_Y);
    _IQNoiseEstimationDriver(
        ne_handle.in_reg_blk_sad_min_tmp_C, ne_handle.in_reg_sum_noise_mean_C, ne_handle.in_reg_count_noise_mean_C,
        ne_handle, ne_handle.out_reg_blk_sad_min_C, ne_handle.out_reg_noise_mean_C, ne_handle.out_reg_noise_mean_th_C);

    _IQCalcNRDnrGainAndLumaLut(
        ne_handle.out_reg_noise_mean_Y, ne_handle.out_reg_noise_mean_C,
        ne_handle.in_dnry_gain, ne_handle.in_dnrc_gain,
        &ne_handle.out_reg_dnry_gain, &ne_handle.out_reg_dnrc_gain, ne_handle.out_reg_lumaLutY, ne_handle.out_reg_lumaLutC);

    //[ne] 3. set noise est. reg
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_MEANTHY], &ne_handle.out_reg_noise_mean_th_Y, sizeof(ne_handle.out_reg_noise_mean_th_Y));
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_MEANTHC], &ne_handle.out_reg_noise_mean_th_C, sizeof(ne_handle.out_reg_noise_mean_th_C));
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_SADMINY], &ne_handle.out_reg_blk_sad_min_Y, sizeof(ne_handle.out_reg_blk_sad_min_Y));
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_SADMINC], &ne_handle.out_reg_blk_sad_min_C, sizeof(ne_handle.out_reg_blk_sad_min_C));
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_MEANY], &ne_handle.out_reg_noise_mean_Y, sizeof(ne_handle.out_reg_noise_mean_Y));
    DrvSclOsMemcpy(&u32Vir_dataNE[QMAP_PROCESS_NE_MEANC], &ne_handle.out_reg_noise_mean_C, sizeof(ne_handle.out_reg_noise_mean_C));
    DrvSclOsMemcpy(&pstIqHandler->pu16NE_blk_sad_min_Y[0], &ne_handle.out_reg_blk_sad_min_Y, sizeof(ne_handle.out_reg_blk_sad_min_Y));
    DrvSclOsMemcpy(&pstIqHandler->pu16NE_blk_sad_min_C[0], &ne_handle.out_reg_blk_sad_min_C, sizeof(ne_handle.out_reg_blk_sad_min_C));
    DrvSclOsMemcpy(&pstIqHandler->pu16NE_noise_mean_Y[0], &ne_handle.out_reg_noise_mean_Y, sizeof(ne_handle.out_reg_noise_mean_Y));
    DrvSclOsMemcpy(&pstIqHandler->pu16NE_noise_mean_C[0], &ne_handle.out_reg_noise_mean_C, sizeof(ne_handle.out_reg_noise_mean_C));

    u32Vir_data[QMAP_PROCESS_MCNR_YGAIN] = ne_handle.out_reg_dnry_gain;
    u32Vir_data[QMAP_PROCESS_MCNR_CGAIN] = ne_handle.out_reg_dnrc_gain;
    DrvSclOsMemcpy(&u32Vir_data[QMAP_PROCESS_MCNR_LUMAY], &ne_handle.out_reg_lumaLutY, sizeof(ne_handle.out_reg_lumaLutY));
    DrvSclOsMemcpy(&u32Vir_data[QMAP_PROCESS_MCNR_LUMAC], &ne_handle.out_reg_lumaLutC, sizeof(ne_handle.out_reg_lumaLutC));
    _IQBufHLInverse(&u32Vir_data[QMAP_PROCESS_MCNR_LUMAY], 8);
    _IQBufHLInverse(&u32Vir_data[QMAP_PROCESS_MCNR_LUMAC], 8);
    if(pstIqHandler->bProcess)
    {
        u32Vir_data[QMAP_PROCESS_MCNR_EN] = ((pstIqHandler->bNREn << 1)|pstIqHandler->bNREn);
        if(!pstIqHandler->bNrFBAllocate && pstIqHandler->bNREn)
        {
            u32Vir_data[QMAP_PROCESS_MCNR_EN] = 0;
            VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]NrFB not allocate ,can't open Nr\n");
        }
        else if(!pstIqHandler->bNREn)
        {
            pstIqHandler->bProcess = 0;
        }
    }
    else
    {
        u32Vir_data[QMAP_PROCESS_MCNR_EN] = pstIqHandler->bNREn;
        if(pstIqHandler->bNREn)
        {
            pstIqHandler->bProcess = 1;
        }
    }
    //wdr delay 1 frame to active
    if (pstIqHandler->bWDRActive) {
        u32Vir_data[QMAP_PROCESS_WDR_STR]   = pstIqHandler->u8Wdr_Str;
        u32Vir_data[QMAP_PROCESS_WDR_STR+1] = pstIqHandler->u8Wdr_Slope;
    }
    else {
        u32Vir_data[QMAP_PROCESS_WDR_STR]   = 0;
        u32Vir_data[QMAP_PROCESS_WDR_STR+1] = 0;
    }
    if (pstIqHandler->bWDREn)
        pstIqHandler->bWDRActive = 1;
    else
        pstIqHandler->bWDRActive = 0;


    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[VPEIQ]%s:%d, \n",  __FUNCTION__, fcnt);

    //printf input data
    p16 = &ne_handle.in_reg_blk_sad_min_tmp_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_SADY]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p32 = &ne_handle.in_reg_sum_noise_mean_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_SUMY]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,\n",  p32[0],p32[1],p32[2],p32[3],p32[4],p32[5],p32[6],p32[7]);

    p16 = &ne_handle.in_reg_count_noise_mean_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_CNTY]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.in_reg_blk_sad_min_tmp_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_SADC]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p32 = &ne_handle.in_reg_sum_noise_mean_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_SUMC]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,\n",  p32[0],p32[1],p32[2],p32[3],p32[4],p32[5],p32[6],p32[7]);

    p16 = &ne_handle.in_reg_count_noise_mean_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[NEBUF_CNTC]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_blk_sample_num_targe] = %d\n", ne_handle.in_blk_sample_num_target);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_noise_mean_th_gain] = %d\n", ne_handle.in_noise_mean_th_gain);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_inten_blknum_lb] = %d\n", ne_handle.in_inten_blknum_lb);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_blk_sad_min_lb] = %d\n", ne_handle.in_blk_sad_min_lb);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_noise_mean_lb] = %d\n", ne_handle.in_noise_mean_lb);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_learn_rate] = %d\n", ne_handle.in_learn_rate);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_dnry_gain] = %d\n", ne_handle.in_dnry_gain);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[in_dnrc_gain] = %d\n", ne_handle.in_dnrc_gain);

    //printf output data
    p16 = &ne_handle.out_reg_noise_mean_th_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_noise_mean_th_Y]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.out_reg_noise_mean_th_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_noise_mean_th_C]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.out_reg_blk_sad_min_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_blk_sad_min_Y]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.out_reg_blk_sad_min_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_blk_sad_min_C]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.out_reg_noise_mean_Y[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_noise_mean_Y]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p16 = &ne_handle.out_reg_noise_mean_C[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_noise_mean_C]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p16[0],p16[1],p16[2],p16[3],p16[4],p16[5],p16[6],p16[7]);

    p8 = &ne_handle.out_reg_lumaLutY[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_lumaLutY]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p8[0],p8[1],p8[2],p8[3],p8[4],p8[5],p8[6],p8[7]);

    p8 = &ne_handle.out_reg_lumaLutC[0];
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_lumaLutC]\n");
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "%d,%d,%d,%d,%d,%d,%d,%d,\n",  p8[0],p8[1],p8[2],p8[3],p8[4],p8[5],p8[6],p8[7]);

    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_dnry_gain] = %d\n", ne_handle.out_reg_dnry_gain);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[out_reg_dnrc_gain] = %d\n", ne_handle.out_reg_dnrc_gain);

    //[wdr] wdr ==========================================
    //[wdr] 1. get wdr result

    //[wdr] 2. set wdr result
    stCfg.u32Viraddr = (u32)u32Vir_dataNE;
    stCfg.enAIPType = E_DRV_SCLVIP_IO_AIP_FORVPEPROCESSNE;
    stCfg = FILL_VERCHK_TYPE(stCfg, stCfg.VerChk_Version, stCfg.VerChk_Size,DRV_SCLVIP_VERSION);
    _DrvSclVipIoSetAipConfig(pstIqHandler->s32Handle, &stCfg);
    stCfg.u32Viraddr = (u32)u32Vir_data;
    stCfg.enAIPType = E_DRV_SCLVIP_IO_AIP_FORVPEPROCESS;
    stCfg = FILL_VERCHK_TYPE(stCfg, stCfg.VerChk_Version, stCfg.VerChk_Size,DRV_SCLVIP_VERSION);
    _DrvSclVipIoSetAipConfig(pstIqHandler->s32Handle, &stCfg);
    _DrvSclVipIoSetFlip(pstIqHandler->s32Handle);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);
    return bRet;
}

bool MHalVpeIqDbgLevel(void *p)
{
    bool bRet = VPE_RETURN_OK;
    gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_IQ] = *((bool *)p);
    VPE_ERR("[VPEIQ]%s Dbg Level = %hhx\n",__FUNCTION__,gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_IQ]);
    return bRet;
}

bool MHalVpeIqConfig(void *pCtx, const MHalVpeIqConfig_t *pCfg)
{
    int i=0, idx=0, id=0, id_up=0, a=0, b=0, c=0;
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoAipConfig_t  stCfg;
    static u8 u32Vir_data[QMAP_CONFIG_SIZE];
    s16 in_motion_th   = pCfg->u8NRY_BLEND_MOTION_TH;
    s16 in_still_th    = pCfg->u8NRY_BLEND_STILL_TH;
    s16 in_motion_wei  = pCfg->u8NRY_BLEND_MOTION_WEI;
    s16 in_other_wei   = pCfg->u8NRY_BLEND_OTHER_WEI;
    s16 in_still_wei   = pCfg->u8NRY_BLEND_STILL_WEI;
    u8 in_sad_shift    = pCfg->u8NRY_SF_STR;
    u8 in_xnr_dw_th    = pCfg->u8NRC_SF_STR;
    u8 in_contrast     = pCfg->u8Contrast;
    int luma_node = 8;
    int luma_step = 37; //=255/8
    int luma_lut[][16] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {50,35,20,10,3,1,0,0,0,0,0,0,0,0,0,0},
        {61,57,50,35,20,10,3,1,0,0,0,0,0,0,0,0},
        {61,57,50,40,28,18,10,6,4,2,2,1,1,1,1,1},
        {61,61,56,48,42,36,28,16,8,4,4,4,4,4,4,4},
        {61,61,58,55,52,48,42,30,18,12,9,8,8,8,8,8},
        {61,61,61,61,59,59,56,48,36,18,14,12,12,12,8,8},
        //{61,61,61,61,60,60,56,48,36,28,24,18,16,12,8,8},
        {61,61,61,61,61,60,60,56,48,36,28,20,16,14,12,12}
    };
    int wdr_str[][3] = {
		{   0,   0,    0},
		{  40, 128, 140},
		{ 255, 255, 255}
	};
	int node = 3;
	int id_0 = 0, id_1 = 0, out_a, out_b;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    //===DNR_Y_LUT, DNR_Y_Gain===
    id = 0;
    for (i=0; i<luma_node; i++)
    {
        if (pCfg->u8NRY_TF_STR <= luma_step*i)
        {
          id = minmax(i-1, 0, 7);
          id_up = minmax(id+1, 0, 7);
          break;
        }
    }
    for(i = 0 ; i < 16; i++) {
        a = luma_lut[id][i];
        b = luma_lut[id_up][i];
        c = luma_step*id;
        u32Vir_data[QMAP_CONFIG_MCNR_YTBL+i] = a + ((b-a) * (pCfg->u8NRY_TF_STR-c) / luma_step);
    }
    _IQBufHLInverse(&u32Vir_data[QMAP_CONFIG_MCNR_YTBL], 16);
    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_MCNR_MV_YTBL], &u32Vir_data[QMAP_CONFIG_MCNR_YTBL], 16);

    //===DNR_C_LUT, DNR_C_Gain===
    id = 0;
    for (i=0; i<luma_node; i++)
    {
        if (pCfg->u8NRC_TF_STR <= luma_step*i)
        {
          id = minmax(i-1, 0, 7);
          id_up = minmax(id+1, 0, 7);
          break;
        }
    }
    for(i = 0 ; i < 16; i++) {
        a = luma_lut[id][i];
        b = luma_lut[id_up][i];
        c = luma_step*id;
        u32Vir_data[QMAP_CONFIG_MCNR_CTBL+i] = a + ((b-a) * (pCfg->u8NRC_TF_STR-c) / luma_step);
    }
    _IQBufHLInverse(&u32Vir_data[QMAP_CONFIG_MCNR_CTBL], 16);
    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_MCNR_MV_CTBL], &u32Vir_data[QMAP_CONFIG_MCNR_CTBL], 16);

    //===NLM+ES===================================
    //===NLM_main wb===
    if (in_motion_th >= in_still_th) in_still_th = minmax(in_motion_th+1, 0, 15);
    for(idx = 0; idx < 16; idx++)
    {
        if(idx <= in_motion_th)
            pstIqHandler->pu8NLM_ESOff[idx] = in_motion_wei;
        else if(idx >= in_still_th)
            pstIqHandler->pu8NLM_ESOff[idx] = in_still_wei;
        else
            pstIqHandler->pu8NLM_ESOff[idx] = (in_other_wei-in_motion_wei)*(idx-in_motion_th)/(in_still_th-in_motion_th-1)+in_motion_wei;
    }

    //===NLM===
    pstIqHandler->u8NLMShift_ESOff = in_sad_shift/29;

    //sync es
    _IQSyncESSetting(pstIqHandler);
    u32Vir_data[QMAP_CONFIG_NLM_SHIFT] = pstIqHandler->u8NLMShift_ESOff;

    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_NLM_MAIN], &pstIqHandler->pu8NLM_ESOff[0], sizeof(pstIqHandler->pu8NLM_ESOff));
    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_NLM_WB], &pstIqHandler->pu8NLM_ESOff[0], sizeof(pstIqHandler->pu8NLM_ESOff));
    _IQBufHLInverse(&u32Vir_data[QMAP_CONFIG_NLM_MAIN], 16);
    _IQBufHLInverse(&u32Vir_data[QMAP_CONFIG_NLM_WB], 16);
    //===NLM+ES END===============================

    //===XNR===
    u32Vir_data[QMAP_CONFIG_XNR+0] = in_xnr_dw_th;
    u32Vir_data[QMAP_CONFIG_XNR+1] = in_xnr_dw_th;
    u32Vir_data[QMAP_CONFIG_XNR+2] = minmax(in_xnr_dw_th <<1, 0, 255);
    u32Vir_data[QMAP_CONFIG_XNR+3] = minmax(in_xnr_dw_th <<1, 0, 255);

    //===YEE===
    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_YEE_ETCP], pCfg->u8EdgeGain, sizeof(pCfg->u8EdgeGain));
    DrvSclOsMemcpy(&u32Vir_data[QMAP_CONFIG_YEE_ETCM], pCfg->u8EdgeGain, sizeof(pCfg->u8EdgeGain));

    //===Contrast===
    in_contrast = minmax(in_contrast, wdr_str[0][0], wdr_str[node-1][0]);

	for (i = 0; i<node; i++)
	{
		if (in_contrast <= wdr_str[i][0])
		{
			id_1 = i;
			break;
		}
	}
	id_0 = max(id_1 - 1, 0);
	a = max(wdr_str[id_1][0] - wdr_str[id_0][0],1);
	b = in_contrast - wdr_str[id_0][0];

	c = wdr_str[id_1][1] - wdr_str[id_0][1];
	out_a = wdr_str[id_0][1] + (c* b / a);
	out_a = minmax(out_a, wdr_str[0][1], wdr_str[node-1][1]);

	c = wdr_str[id_1][2] - wdr_str[id_0][2];
	out_b = wdr_str[id_0][2] + (c* b / a);
	out_b = minmax(out_b, wdr_str[0][2], wdr_str[node-1][2]);

    pstIqHandler->u8Wdr_Str = out_a; //wdr_strength
    pstIqHandler->u8Wdr_Slope = out_b; //wdr_syn_slp


#if 1
    stCfg.u32Viraddr = (u32)u32Vir_data;
    stCfg.enAIPType = E_DRV_SCLVIP_IO_AIP_FORVPECFG;
    stCfg = FILL_VERCHK_TYPE(stCfg, stCfg.VerChk_Version, stCfg.VerChk_Size,DRV_SCLVIP_VERSION);
    _DrvSclVipIoSetAipConfig(pstIqHandler->s32Handle, &stCfg);
#endif
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);
    return bRet;
}

bool MHalVpeIqOnOff(void *pCtx, const MHalVpeIqOnOff_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoAipConfig_t  stCfg;
    static u8 u32Vir_data[QMAP_ONOFF_SIZE];
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    //nr en
    //u32Vir_data[QMAP_ONOFF_MCNR_EN] = pCfg->bNREn << 1;
    u32Vir_data[QMAP_ONOFF_NLM_EN]  = (pCfg->bNREn | pCfg->bESEn);
    u32Vir_data[QMAP_ONOFF_XNR_EN]  = pCfg->bNREn;
    pstIqHandler->bNREn = pCfg->bNREn;

    //edge en
    u32Vir_data[QMAP_ONOFF_YEE_EN] = pCfg->bEdgeEn;

    //es en
    pstIqHandler->bEsEn = pCfg->bESEn;
    _IQSyncESSetting(pstIqHandler);
    if (pCfg->bESEn) {
        u32Vir_data[QMAP_ONOFF_NLM_SHIFT] = pstIqHandler->u8NLMShift_ESOn;
        DrvSclOsMemcpy(&u32Vir_data[QMAP_ONOFF_NLM_MAIN], &pstIqHandler->pu8NLM_ESOn[0], 16*sizeof(u8));
        DrvSclOsMemcpy(&u32Vir_data[QMAP_ONOFF_NLM_WB], &pstIqHandler->pu8NLM_ESOn[0], 16*sizeof(u8));
    }
    else {
        u32Vir_data[QMAP_ONOFF_NLM_SHIFT] = pstIqHandler->u8NLMShift_ESOff;
        DrvSclOsMemcpy(&u32Vir_data[QMAP_ONOFF_NLM_MAIN], &pstIqHandler->pu8NLM_ESOff[0], 16*sizeof(u8));
        DrvSclOsMemcpy(&u32Vir_data[QMAP_ONOFF_NLM_WB], &pstIqHandler->pu8NLM_ESOff[0], 16*sizeof(u8));
    }
    _IQBufHLInverse(&u32Vir_data[QMAP_ONOFF_NLM_MAIN], 16);
    _IQBufHLInverse(&u32Vir_data[QMAP_ONOFF_NLM_WB], 16);

    //contrast en
    u32Vir_data[QMAP_ONOFF_WDR_EN] = pCfg->bContrastEn;
    pstIqHandler->bWDREn = pCfg->bContrastEn;

    //uv invert en
    u32Vir_data[QMAP_ONOFF_UVM_EN] = (!pCfg->bUVInvert) & 0x01;
    if (pCfg->bUVInvert) {
        u32Vir_data[QMAP_ONOFF_UVM+0] = pstIqHandler->pu8UVM[1];
        u32Vir_data[QMAP_ONOFF_UVM+1] = pstIqHandler->pu8UVM[0];
        u32Vir_data[QMAP_ONOFF_UVM+2] = pstIqHandler->pu8UVM[3];
        u32Vir_data[QMAP_ONOFF_UVM+3] = pstIqHandler->pu8UVM[2];
    }
    else {
        u32Vir_data[QMAP_ONOFF_UVM+0] = pstIqHandler->pu8UVM[0];
        u32Vir_data[QMAP_ONOFF_UVM+1] = pstIqHandler->pu8UVM[1];
        u32Vir_data[QMAP_ONOFF_UVM+2] = pstIqHandler->pu8UVM[2];
        u32Vir_data[QMAP_ONOFF_UVM+3] = pstIqHandler->pu8UVM[3];
    }

#if 1
    stCfg.u32Viraddr = (u32)u32Vir_data;
    stCfg.enAIPType = E_DRV_SCLVIP_IO_AIP_FORVPEONOFF;
    stCfg = FILL_VERCHK_TYPE(stCfg, stCfg.VerChk_Version, stCfg.VerChk_Size,DRV_SCLVIP_VERSION);
    _DrvSclVipIoSetAipConfig(pstIqHandler->s32Handle, &stCfg);
#endif
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL2), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);

    return bRet;
}

bool MHalVpeIqGetWdrRoiHist(void *pCtx, MHalVpeIqWdrRoiReport_t * pstRoiReport)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoWdrRoiReport_t stCfg;
    u64 u64TmpY;
    u64 u64Remaind;
    bool i;
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipIoWdrRoiReport_t));
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    _DrvSclVipIoGetWdrHistogram(pstIqHandler->s32Handle,&stCfg);
    for(i = 0;i<ROI_WINDOW_MAX;i++)
    {
        if(stCfg.s32YCnt[i])
        {
            u64TmpY = stCfg.s32Y[i];
            u64TmpY*=4;// for output 12bit size, (I2 original is 10bit)
            pstIqHandler->stWdrBuffer.u32Y[i] =
                (u32)CamOsMathDivU64(u64TmpY,(u64)(stCfg.s32YCnt[i]),&u64Remaind);
            if(u64Remaind>=(stCfg.s32YCnt[i]/2))
            {
                pstIqHandler->stWdrBuffer.u32Y[i]++;
            }
        }
        else
        {
            pstIqHandler->stWdrBuffer.u32Y[i] = (u32)(stCfg.s32Y[i]*4);
        }
        pstRoiReport->u32Y[i] = pstIqHandler->stWdrBuffer.u32Y[i];
    }
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);

    return bRet;
}
bool MHalVpeIqRead3DNRTbl(void *pCtx)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoNrHist_t stCfg;
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipIoNrHist_t));
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    stCfg.u32Viraddr = (u32)pstIqHandler->pvNrBuffer;
    _DrvSclVipIoGetNRHistogram(pstIqHandler->s32Handle,&stCfg);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);

    return bRet;
}

bool MHalVpeIqSetWdrRoiHist(void *pCtx, const MHalVpeIqWdrRoiHist_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoWdrRoiHist_t stCfg;
    u16 idx,idx2;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;
    //DrvSclOsMemcpy(&stCfg,pCfg,sizeof(MHalVpeIqWdrRoiHist_t));
    stCfg.bEn = pCfg->bEn;
    stCfg.enPipeSrc = pCfg->enPipeSrc;
    stCfg.u8WinCount = pCfg->u8WinCount;
    for(idx=0;idx<DRV_SCLVIP_IO_ROI_WINDOW_MAX;idx++)
    {
        stCfg.stRoiCfg[idx].bEnSkip = 1; //force to enable skip point for avoid 4k/2k issue
        for(idx2=0;idx2<4;idx2++)
        {
            stCfg.stRoiCfg[idx].u16RoiAccX[idx2] = pCfg->stRoiCfg[idx].u16RoiAccX[idx2];
            stCfg.stRoiCfg[idx].u16RoiAccY[idx2] = pCfg->stRoiCfg[idx].u16RoiAccY[idx2];
        }
        if(pstIqHandler->u32BaseAddr[idx])
        {
            stCfg.u32BaseAddr[idx] = pstIqHandler->u32BaseAddr[idx];
        }
    }
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[VPEIQ]%s {X0 X1 X2 X3] [%hu %hu %hu %hu]\n",
        __FUNCTION__,pCfg->stRoiCfg[0].u16RoiAccX[0],pCfg->stRoiCfg[0].u16RoiAccX[1],pCfg->stRoiCfg[0].u16RoiAccX[2],pCfg->stRoiCfg[0].u16RoiAccX[3]);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL3), "[VPEIQ]%s {Y0 Y1 Y2 Y3] [%hu %hu %hu %hu]\n",
        __FUNCTION__,pCfg->stRoiCfg[0].u16RoiAccY[0],pCfg->stRoiCfg[0].u16RoiAccY[1],pCfg->stRoiCfg[0].u16RoiAccY[2],pCfg->stRoiCfg[0].u16RoiAccY[3]);
    _DrvSclVipIoSetRoiConfig(pstIqHandler->s32Handle,&stCfg);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstIqHandler->u16InstId,bRet);

    return bRet;
}

bool MHalVpeIqSetWdrRoiMask(void *pCtx,const bool bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoSetMaskOnOff_t stCfg;
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipIoSetMaskOnOff_t));
    _MHalVpeIqKeepCmdqFunction(pstCmdQInfo);

    stCfg.bOnOff = bEnMask;
    stCfg.enMaskType = E_DRV_SCLVIP_IO_MASK_WDR;
    _DrvSclVipIoSetMaskOnOff(pstIqHandler->s32Handle,&stCfg);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu bEnMask :%hhu Ret:%hhd\n",
        __FUNCTION__,pstIqHandler->u16InstId,bEnMask,bRet);

    return bRet;
}
bool MHalVpeIqSetDnrTblMask(void *pCtx,const bool bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    bool bRet = VPE_RETURN_OK;
    MHalIqHandleConfig_t *pstIqHandler;
    DrvSclVipIoSetMaskOnOff_t stCfg;
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }

  	pstIqHandler = pCtx;

	if(pstIqHandler->eMode != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
		return VPE_RETURN_OK;

    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipIoSetMaskOnOff_t));
    _MHalVpeIqKeepCmdqFunction(pstCmdQInfo);

    stCfg.bOnOff = bEnMask;
    stCfg.enMaskType = E_DRV_SCLVIP_IO_MASK_NR;
    _DrvSclVipIoSetMaskOnOff(pstIqHandler->s32Handle,&stCfg);
    VPE_DBG(VPE_DBG_LV_IQ(E_MHAL_VPE_DEBUG_LEVEL4), "[VPEIQ]%s Inst %hu bEnMask :%hhu Ret:%hhd\n",
        __FUNCTION__,pstIqHandler->u16InstId,bEnMask,bRet);
    return bRet;
}
bool MHalVpeCreateSclInstance(const MHalAllocPhyMem_t *pstAlloc, const MHalVpeSclCreate_t *stCfg ,void **pCtx)
{
	bool bRet = VPE_RETURN_OK;
	s16 s16Idx = -1;
	//init
	_MHalVpeInit();
	//keep Alloc
	_MHalVpeKeepAllocFunction(pstAlloc);
	//find new inst
	bRet = _MHalVpeFindEmptyInst(&s16Idx);
	if(bRet == VPE_RETURN_ERR || s16Idx==-1)
	{
		return VPE_RETURN_ERR;
	}
	if(stCfg->enOutmode <= E_MHAL_SCL_OUTPUT_MODE_INVALID || stCfg->enTopIn <= E_MHAL_VPE_INPUT_MODE_INVALID)
		return VPE_RETURN_ERR;
    if(stCfg->enTopIn != E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE)
    {
        MHalVpeSetbHist(1);
    }
	pgstScHandler[s16Idx]->eOutMode = stCfg->enOutmode;
	pgstScHandler[s16Idx]->eInMode = stCfg->enInputPortMode;
	pgstScHandler[s16Idx]->eInst = stCfg->enSclInstance;
	pgstScHandler[s16Idx]->enTopIn = stCfg->enTopIn;
	VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2),
	    "[VPESCL]%s Create Inst %hu InstType:%d Top:%d Input:%d OutPut:%x \n"
	     ,__FUNCTION__,pgstScHandler[s16Idx]->u16InstId,stCfg->enSclInstance,
	     stCfg->enTopIn, stCfg->enInputPortMode,stCfg->enOutmode);
	if(stCfg->enInputPortMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
	{
        bRet = _MHalVpeCreateM2MInstance(&stCfg->stMaxWin,s16Idx);
    }
    else if(stCfg->enInputPortMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(stCfg->enInputPortMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(stCfg->enInputPortMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    else
    {
        bRet = _MHalVpeCreateSclInstance(&stCfg->stMaxWin,s16Idx);
    }
	//open device
	//assign Ctx
	//======void *pCtx
	//======func(&pCtx)
	*pCtx = pgstScHandler[s16Idx];
	VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Create Inst %hu Success\n",  __FUNCTION__,pgstScHandler[s16Idx]->u16InstId);
	return bRet;
}

bool MHalVpeDestroySclInstance(void *pCtx)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    u16 u16InstId;
    //void *pCtx = pCreate;
    //func(pCtx)
    if(pCtx==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    u16InstId = pstScHandler->u16InstId;
    //release MCNR
    //_DrvSclHvspIoReleaseMemConfig(pstScHandler->s32Handle[E_HAL_SCLHVSP_ID_1]);
    //destroy inst
    if(pstScHandler->eInMode ==E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeDestroyM2MInst(&pstScHandler->s32Handle[E_HAL_M2M_ID]);
        //close device
        _MHalVpeCloseM2MDevice(&pstScHandler->s32Handle[E_HAL_M2M_ID]);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    else
    {
        if(pstScHandler->enTopIn ==E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE && pstScHandler->eInst == E_MHAL_VPE_SCL_MAIN)
        {
            VPE_ERR("[VPESCL]%s Destroy process fence %hx \n",
                __FUNCTION__,_VpeIspRegR((ISINPROCESSQ_DUMMYReg>>8),(ISINPROCESSQ_DUMMYReg&0xFF)/2));
            _VpeIspRegWMsk(NULL,(ISINPROCESSQ_DUMMYReg>>8),(ISINPROCESSQ_DUMMYReg&0xFF)/2,
            (u16)(_VpeIspRegR((ISINPROCESSQ_DUMMYReg>>8),(ISINPROCESSQ_DUMMYReg&0xFF)/2)+0xF),ISINPROCESSQ_DUMMYLENTH);
            _VpeIspRegWMsk(NULL,BANK_CMDQ_CMDQ0, 0x52, 0x8002,0xFFFF);// force CMDQ trigger bus
            do
            {
                DrvSclOsDelayTask(5);
            }
            while(!_VpeIspRegR((ISCMDQ_IDLEDUMMYReg>>8),(ISCMDQ_IDLEDUMMYReg&0xFF)/2));
            _VpeIspRegWMsk(NULL,BANK_CMDQ_CMDQ0, 0x52, 0x0,0xFFFF);// force CMDQ trigger bus
            _VpeIspRegWMsk(NULL,BANK_CMDQ_CMDQ0, 0x54, 0xFFFF,0xFFFF);// clear CMDQ trigger bus
        }
        bRet = _MHalVpeDestroyInst(pstScHandler->s32Handle);
        //close device
        _MHalVpeCloseDevice(pstScHandler->s32Handle);
    }
    _MHalVpeCleanScInstBuffer(pstScHandler);
    if(bRet == VPE_RETURN_ERR)
    {
        return bRet;
    }
    //clean scl inst
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Destroy Inst %hu Success\n",  __FUNCTION__,u16InstId);
    return bRet;
}
bool MHalVpeSclProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeSclOutputBufferConfig_t *pBuffer)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pBuffer==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME)
    {
        bRet = _MHalVpeSclProcess(pstScHandler,pstCmdQInfo,pBuffer);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeM2MProcess(pstScHandler,pstCmdQInfo,pBuffer);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}

bool MHalVpeSclDbgLevel(void *p)
{
    bool bRet = VPE_RETURN_OK;
    gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_SC] = *((bool *)p);
    VPE_ERR("[VPEIQ]%s Dbg Level = %hhx\n",__FUNCTION__,gbDbgLevel[E_MHAL_VPE_DEBUG_TYPE_SC]);
    return bRet;
}

bool MHalVpeSclCropConfig(void *pCtx, const MHalVpeSclCropConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    if(pCfg->stCropWin.u16Height==0 || pCfg->stCropWin.u16Width==0)
    {
        VPE_ERR("[%s]input size 0\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        VPE_ERR("[%s]I2 M2M not support Crop\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support Crop\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT not support Crop\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT not support Crop\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL1), "[VPESCL]%s Inst %hu (X,Y,W,H)=(%hu,%hu,%hu,%hu) En:%d\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->stCropWin.u16X,pCfg->stCropWin.u16Y,
        pCfg->stCropWin.u16Width,pCfg->stCropWin.u16Height,pCfg->bCropEn);
    if(_MHalVpeScIsCropSizeNeedReSetting(pstScHandler,pCfg))
    {
        // set global
        _MHalVpeScCropSizeReSetting(pstScHandler,pCfg);
        // set scaling size
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_1,pstScHandler);
        _MHalVpeScScalingConfig(E_HAL_SCLHVSP_ID_4,pstScHandler);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}


bool MHalVpeSclInputConfig(void *pCtx, const MHalVpeSclInputSizeConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    MHalVpeSclInputSizeConfig_t stForISPCfg;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    if(pCfg->u16Height==0 || pCfg->u16Width==0)
    {
        VPE_ERR("[%s]input size 0\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    DrvSclOsMemcpy(&stForISPCfg,pCfg,sizeof(MHalVpeSclInputSizeConfig_t));
    pstScHandler = pCtx;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL1), "[VPESCL]%s Inst %hu Height:%hu Width:%hu PixelFormat:%d\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->u16Height,pCfg->u16Width,pCfg->ePixelFormat);
    if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeM2MInputConfig(pstScHandler,&stForISPCfg);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    else
    {
        bRet = _MHalVpeSclInputConfig(pstScHandler,&stForISPCfg);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}
bool MHalVpeSclPortSizeConfig(void *pCtx, const MHalVpeSclOutputSizeConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    if(pCfg->u16Height==0 || pCfg->u16Width==0)
    {
        VPE_ERR("[%s]output size 0\n",__FUNCTION__);
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL1), "[VPESCL]%s Inst %hu Height:%hu Width:%hu @OutPort:%d\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->u16Height,pCfg->u16Width,pCfg->enOutPort);
    if(!_MHalVpeSclIsPortBindSuccess(pCfg->enOutPort,pstScHandler))
    {
        return VPE_RETURN_ERR;
    }
    if(pstScHandler->eInMode ==E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeM2MPortSizeConfig(pstScHandler,pCfg);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    else
    {
        bRet = _MHalVpeSclPortSizeConfig(pstScHandler,pCfg);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}
bool MHalVpeSclDmaPortConfig(void *pCtx, const MHalVpeSclDmaConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    if(!_MHalVpeSclIsPortBindSuccess(pCfg->enPort,pstScHandler))
    {
        return VPE_RETURN_ERR;
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu enCompress:%d OutFormat:%d @OutPort:%d\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->enCompress,pCfg->enFormat,pCfg->enPort);
    if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeM2MDmaPortConfig(pstScHandler,pCfg);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    else
    {
        bRet = _MHalVpeSclDmaPortConfig(pstScHandler,pCfg);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}
bool MHalVpeSclImiPortConfig(void *pCtx, const MHalVpeSclOutputIMIConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu OutType:%d @OutPort:%d bEn:%hhd\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->enFormat,pCfg->enOutPort,pCfg->bEn);
    if(pCfg->enOutPort !=E_MHAL_SCL_OUTPUT_PORT0)
    {
        VPE_ERR("[VPESCL]%s Output Port To IMI Error\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    _MHalVpeSclDmaImiPortConfig(pstScHandler,pCfg);
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);

    return bRet;
}

bool MHalVpeSclOutputMDWinConfig(void *pCtx, const MHalVpeSclOutputMDwinConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    if(pCtx==NULL || pCfg==NULL)
    {
        return VPE_RETURN_ERR;
    }
    pstScHandler = pCtx;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu OutType:%d @OutPort:%d\n",
        __FUNCTION__,pstScHandler->u16InstId,pCfg->enOutType,pCfg->enOutPort);
    if(pCfg->enOutPort !=E_MHAL_SCL_OUTPUT_PORT3 &&pCfg->enOutType ==E_MHAL_SCL_OUTPUT_MDWIN)
    {
        VPE_ERR("[VPESCL]%s Output Port To MDWIN Error\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    if(pCfg->enOutPort ==E_MHAL_SCL_OUTPUT_PORT3 &&pCfg->enOutType !=E_MHAL_SCL_OUTPUT_MDWIN)
    {
        VPE_ERR("[VPESCL]%s Output Port 3 To Dram Error\n",__FUNCTION__);
        bRet = VPE_RETURN_ERR;
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL2), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);

    return bRet;
}
//new
bool MHalVpeSclSetSwTriggerIrq(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    bool bRet = VPE_RETURN_OK;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s Ret:%hhd\n",  __FUNCTION__,bRet);
    _MHalVpeSclKeepCmdqFunction(pstCmdQInfo);
    _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0xC,0x2,0x2);
    _VpeIspRegWMsk(pstCmdQInfo,BANK_SC_SC0,0xC,0x0,0x2);
    return bRet;
}
bool MHalVpeSclSetWaitDone(void *pSclCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, MHalVpeWaitDoneType_e enWait)
{
    bool bRet = VPE_RETURN_OK;
    MHalSclHandleConfig_t *pstScHandler;
    pstScHandler = pSclCtx;
    //Add dma off cmd
    if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME)
    {
        bRet = _MHalVpeSclSetWaitDone(pstScHandler,pstCmdQInfo,enWait);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME)
    {
        bRet = _MHalVpeM2MSetWaitDone(pstScHandler,pstCmdQInfo);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME)
    {
        VPE_ERR("[%s]I2 LDC not support now\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_SUB_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT SUB not support\n",__FUNCTION__);
    }
    else if(pstScHandler->eInMode == E_MHAL_SCL_MAIN_FRAME_ROTATION)
    {
        VPE_ERR("[%s]I2 ROT Main not support\n",__FUNCTION__);
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s Inst %hu Ret:%hhd\n",  __FUNCTION__,pstScHandler->u16InstId,bRet);
    return bRet;
}
bool MHalVpeSclGetIrqNum(unsigned int *pIrqNum,MHalVpeIrqEnableMode_e eMode)
{
    bool bRet = VPE_RETURN_OK;
    u32 u32IrqNum;
    _MHalVpeInit();
    if(eMode&E_MHAL_SCL_IRQ_M2M_MODE || eMode&E_MHAL_SCL_IRQ_OFFM2M)
    {
        u32IrqNum = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC1);
    }
    else
    {
        u32IrqNum = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0);
    }
    *pIrqNum = u32IrqNum;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s IrqNum=%lu Ret:%hhd\n",  __FUNCTION__,u32IrqNum,bRet);

    return bRet;
}
bool MHalVpeSclRotationConfig(void *pCtx, const MHalVpeSclRotationConfig_t *pCfg)
{
    bool bRet = VPE_RETURN_OK;
    VPE_ERR("[VPE] %s NOT SUPPORT\n",__FUNCTION__);
    return bRet;
}

#if defined(USE_USBCAM)
irqreturn_t  _MHalVpeSclDoneIrq(int eIntNum, void* dev_id)
{
	MHalVpeIrqStatus_e eIrqstst;
    MHalVpeIrqEnableMode_e eMode;
	eIrqstst = MHalVpeSclCheckIrq(DrvSclIrqGetSclIsrOpenMode()) ;

	if (eIrqstst != E_MHAL_SCL_IRQ_NONE)
	{
	   MHalVpeSclClearIrq(eMode);
#if 1
	   if((eIrqstst & E_MHAL_SCL_IRQ_FRAMEDONE))
	   {
	   	if(_gpMahlIntCb)
	   		_gpMahlIntCb();
	   }
#else
	   if(eIrqstst == E_MHAL_SCL_IRQ_FRAMEDONE)
	   {
	   	if(_gpMahlIntCb)
	   		_gpMahlIntCb();
	   }
#endif
	}

    return IRQ_HANDLED;
}


bool MHalVpeSetIrqCallback(MhalInterruptCb pMhalIntCb)
{
    _gpMahlIntCb = pMhalIntCb;
    return VPE_RETURN_OK;
}
bool MHalVpeSclEnableIrqForMiddle(MHalVpeIrqEnableMode_e eMode)
{
    bool bRet = VPE_RETURN_OK;
    unsigned int  nIrqNum = 0 ;
    if(eMode != E_MHAL_SCL_IRQ_OFF)
    {
        if(_gpMahlIntCb == NULL)
        {
            VPE_ERR("[DRVSCLIRQ]%s(%d):: Request FUNC NULL\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        MHalVpeSclGetIrqNum(&nIrqNum,eMode);
        if(DrvSclOsAttachInterrupt(nIrqNum, (InterruptCb)_MHalVpeSclDoneIrq ,IRQF_DISABLED, "SCLINTR"))
        {
            VPE_ERR("[DRVSCLIRQ]%s(%d):: Request IRQ Fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
        DrvSclOsDisableInterrupt(nIrqNum);
        DrvSclOsEnableInterrupt(nIrqNum);
        MHalVpeSclEnableIrq(eMode);
    }
    else if(eMode == E_MHAL_SCL_IRQ_OFFM2M)
    {
        MHalVpeSclEnableIrq(eMode);
        DrvSclOsDisableInterrupt(nIrqNum);
    }
    else
    {
        MHalVpeSclEnableIrq(eMode);
        DrvSclOsDisableInterrupt(nIrqNum);
        _gpMahlIntCb = NULL;
    }

	DrvSclIrqSetSclIsrOpenMode((DrvSclIrqEnableMode_e)eMode) ;
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s bOn= %lx Ret:%hhd\n",  __FUNCTION__,(u32)eMode,bRet);
    return bRet;
}
#endif

void MHalVpeGetCaps(MHalVpeGetCap_t *pstCaps)
{
	if(pstCaps == NULL)
		return ;

	DrvSclOsMemset(pstCaps,0x0,sizeof(MHalVpeGetCap_t));
	DrvSclOsGetCap((DrvSclOsGetCap_t *)pstCaps);
}
bool MHalVpeSclEnableIrq(MHalVpeIrqEnableMode_e eMode)
{
    bool bRet = VPE_RETURN_OK;
    u16 u16FramedoneNum = (eMode&E_MHAL_SCL_IRQ_PORT2) ? VPEIRQ_SC3_ENG_FRM_END : VPEIRQ_SC_ENG_FRM_END;
    if(eMode & E_MHAL_SCL_IRQ_M2M_MODE)
    {
        DrvSclIrqM2MInterruptEnable(VPEIRQ_SC3_DMA_W_ACTIVE);
        DrvSclIrqM2MInterruptEnable(VPEIRQ_SC3_DMA_W_ACTIVE_N);
        DrvSclIrqSetM2MIsrOpenMode((DrvSclIrqEnableMode_e)eMode);
        genIrqmode[E_HAL_SCL_INST_M2M] = eMode;
    }
    else if(eMode & E_MHAL_SCL_IRQ_OFFM2M)
    {
        DrvSclIrqM2MDisable(VPEIRQ_SC3_DMA_W_ACTIVE);
        DrvSclIrqM2MDisable(VPEIRQ_SC3_DMA_W_ACTIVE_N);
        DrvSclIrqSetM2MIsrOpenMode((DrvSclIrqEnableMode_e)0);
        genIrqmode[E_HAL_SCL_INST_M2M] = 0;
    }
    else if(eMode != E_MHAL_SCL_IRQ_OFF)
    {
        DrvSclIrqInterruptEnable(u16FramedoneNum);
        DrvSclIrqSetFrameDoneInt(((u64)1<<u16FramedoneNum));
		if(eMode & E_MHAL_SCL_IRQ_MULTIINT_MODE)
		{
			DrvSclIrqInterruptEnable(VPEIRQ_VSYNC_IDCLK);
			DrvSclIrqInterruptEnable(VPEIRQ_SC_IN_FRM_END);
			// only for debug
			//DrvSclIrqInterruptEnable(VPEIRQ_AFF_FULL);
			//DrvSclIrqInterruptEnable(VPEIRQ_VSYNC_FCLK_LDC);
			//DrvSclIrqInterruptEnable(SCLIRQ_SC1_FRM_W_ACTIVE);
			//DrvSclIrqInterruptEnable(SCLIRQ_SC1_FRM_W_ACTIVE_N);
			//enable in WaitDone
			//DrvSclIrqInterruptEnable(VPEIRQ_SC4_HVSP_FINISH);
		}
		else
		{
			DrvSclIrqDisable(VPEIRQ_VSYNC_IDCLK);
			DrvSclIrqDisable(VPEIRQ_SC_IN_FRM_END);
			DrvSclIrqDisable(VPEIRQ_SC4_HVSP_FINISH);
			// only for debug
			//DrvSclIrqDisable(VPEIRQ_AFF_FULL);
			//DrvSclIrqDisable(VPEIRQ_VSYNC_FCLK_LDC);
			//DrvSclIrqDisable(SCLIRQ_SC1_FRM_W_ACTIVE);
			//DrvSclIrqDisable(SCLIRQ_SC1_FRM_W_ACTIVE_N);
		}
        DrvSclIrqSetSclIsrOpenMode((DrvSclIrqEnableMode_e)eMode);
        genIrqmode[E_HAL_SCL_INST_SCL] = eMode;
    }
    else
    {
        DrvSclIrqDisable(VPEIRQ_SC3_ENG_FRM_END);
        DrvSclIrqDisable(VPEIRQ_SC_ENG_FRM_END);
		DrvSclIrqDisable(VPEIRQ_VSYNC_IDCLK);
        DrvSclIrqDisable(VPEIRQ_SC_IN_FRM_END);
        DrvSclIrqDisable(VPEIRQ_SC4_HVSP_FINISH);
        // only for debug
        //DrvSclIrqDisable(VPEIRQ_AFF_FULL);
        //DrvSclIrqDisable(VPEIRQ_VSYNC_FCLK_LDC);
        //DrvSclIrqDisable(SCLIRQ_SC1_FRM_W_ACTIVE);
        //DrvSclIrqDisable(SCLIRQ_SC1_FRM_W_ACTIVE_N);
        DrvSclIrqSetSclIsrOpenMode((DrvSclIrqEnableMode_e)eMode);
        genIrqmode[E_HAL_SCL_INST_SCL] = eMode;
    }
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s bOn= %lx Ret:%hhd\n",  __FUNCTION__,(u32)eMode,bRet);
    return bRet;
}
bool MHalVpeSclClearIrq(MHalVpeIrqEnableMode_e eMode)
{
    bool bRet = VPE_RETURN_OK;
    //DrvSclIrqSetClear(VPEIRQ_SC3_ENG_FRM_END);
	//DrvSclIrqSetClear(VPEIRQ_VSYNC_IDCLK);
    VPE_DBG(VPE_DBG_LV_SC(E_MHAL_VPE_DEBUG_LEVEL4), "[VPESCL]%s Ret:%hhd\n",  __FUNCTION__,bRet);
    return bRet;
}
MHalVpeIrqStatus_e MHalVpeSclCheckIrq(MHalVpeIrqEnableMode_e eMode)
{
    MHalVpeIrqStatus_e eIrqstat = E_MHAL_SCL_IRQ_NONE;
    DrvSclIrqScIntsType_e enRet;
	u64 u64Time;
    u64Time = (DrvSclOsGetSystemTimeStamp());
    if(eMode & E_MHAL_SCL_IRQ_M2M_MODE)
    {
        enRet = DrvSclIrqM2MIsrHandler(u64Time);
    }
    else
    {
        enRet = DrvSclIrqIsrHandler(u64Time);
    }
    if(enRet&E_DRV_SCL_IRQ_SCDONE)
    {
        eIrqstat |= E_MHAL_SCL_IRQ_FRAMEDONE;
    }
    if((enRet&E_DRV_SCL_IRQ_VSYNC) && (eMode&E_DRV_SCL_IRQ_REALTIME_MODE))
    {
        eIrqstat |= E_MHAL_SCL_IRQ_VSYNC;
    }
    return eIrqstat;
}
bool MHalVpeInit(const MHalAllocPhyMem_t *pstAlloc ,MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo)
{
    _MHalVpeKeepAllocFunction(pstAlloc);
    _MHalVpeSclKeepCmdqFunction(pstCmdQInfo);
    _MHalVpeInit();
    gbHistints = 0;
    return VPE_RETURN_OK;
}
bool MHalVpeDeInit(void)
{
    _MHalVpeDeInit();
    _MHalVpeKeepAllocFunction(NULL);
    _MHalVpeSclKeepCmdqFunction(NULL);
    gbHistints = 0;
    return VPE_RETURN_OK;
}
#if defined (SCLOS_TYPE_LINUX_KERNEL)
EXPORT_SYMBOL(MHalVpeSclRotationConfig);
EXPORT_SYMBOL(MHalVpeInit);
EXPORT_SYMBOL(MHalVpeDeInit);
EXPORT_SYMBOL(MHalVpeSclCheckIrq);
EXPORT_SYMBOL(MHalVpeSclSetWaitDone);
EXPORT_SYMBOL(MHalVpeIqRead3DNRTbl);
EXPORT_SYMBOL(MHalVpeSclSetSwTriggerIrq);
EXPORT_SYMBOL(MHalVpeSclClearIrq);
EXPORT_SYMBOL(MHalVpeSclEnableIrq);
EXPORT_SYMBOL(MHalVpeSclGetIrqNum);
EXPORT_SYMBOL(MHalVpeSclPortSizeConfig);
EXPORT_SYMBOL(MHalVpeSclInputConfig);
EXPORT_SYMBOL(MHalVpeSclDmaPortConfig);
EXPORT_SYMBOL(MHalVpeSclCropConfig);
EXPORT_SYMBOL(MHalVpeSclDbgLevel);
EXPORT_SYMBOL(MHalVpeSclProcess);
EXPORT_SYMBOL(MHalVpeDestroySclInstance);
EXPORT_SYMBOL(MHalVpeCreateSclInstance);
EXPORT_SYMBOL(MHalVpeIspInputConfig);
EXPORT_SYMBOL(MHalVpeIspDbgLevel);
EXPORT_SYMBOL(MHalVpeIspProcess);
EXPORT_SYMBOL(MHalVpeCreateIspInstance);
EXPORT_SYMBOL(MHalVpeDestroyIspInstance);
EXPORT_SYMBOL(MHalVpeIqSetDnrTblMask);
EXPORT_SYMBOL(MHalVpeIqSetWdrRoiMask);
EXPORT_SYMBOL(MHalVpeIqSetWdrRoiHist);
EXPORT_SYMBOL(MHalVpeIqGetWdrRoiHist);
EXPORT_SYMBOL(MHalVpeIqOnOff);
EXPORT_SYMBOL(MHalVpeIqConfig);
EXPORT_SYMBOL(MHalVpeIqDbgLevel);
EXPORT_SYMBOL(MHalVpeIqProcess);
EXPORT_SYMBOL(MHalVpeDestroyIqInstance);
EXPORT_SYMBOL(MHalVpeCreateIqInstance);
EXPORT_SYMBOL(MHalVpeGetCaps);
#if defined(USE_USBCAM)
EXPORT_SYMBOL(MHalVpeSetIrqCallback);
EXPORT_SYMBOL(MHalVpeSclEnableIrqForMiddle);
#endif
#endif
#undef HAL_VPE_C
