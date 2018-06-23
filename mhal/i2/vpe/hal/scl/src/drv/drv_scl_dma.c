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
#define DRV_SCLDMA_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "hal_scl_util.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "drv_scl_dma_st.h"
#include "drv_scl_hvsp_st.h"
#include "hal_scl_dma.h"
#include "drv_scl_dma.h"
#include "hal_scl_reg.h"
#include "drv_scl_dbg.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SCLDMA_DBG(x)
#define DRV_SCLDMA_DBG_H(x)
#define DRV_SCLDMA_ERR(x)     x
#define FHD_Width   1920
#define FHD_Height  1080
#define HD_Width    1280
#define HD_Height   720
#define PNL_Width   800
#define PNL_Height  480
#define SINGLE_SKIP 0xF
#define FRM_POLLIN  0x1
#define SNP_POLLIN  0x2
#define FRM2_POLLIN  0x2
#define SCL_IMIinitAddr 0x14000

#define SINGLE_BUFF_ACTIVE_TIMIEOUT      100
#define _Is_RingMode(enClientType)                  (gstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_RING)
#define _Is_SingleMode(enClientType)                (gstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SINGLE)
#define _Is_SWRingMode(enClientType)            (gstScldmaInfo->enBuffMode[(enClientType)] == E_DRV_SCLDMA_BUF_MD_SWRING)
#define _Is_DMACanReOpen(bReOpen,bRpchange) ((bReOpen)&&(bRpchange))
#define _Is_DMAClientOn(enClientType)           (gstScldmaInfo->bDMAOnOff[enClientType])
#define _Is_DMAClientOff(enClientType)           (!gstScldmaInfo->bDMAOnOff[enClientType])
#define _Is_IdxRingCircuit(enClientType,idx) (idx == (gstScldmaInfo->bMaxid[enClientType]-1))

#define _Is_SC3Singlemode(enClientType) (_Is_SingleMode(enClientType) && \
    (enClientType == E_DRV_SCLDMA_3_FRM_R || enClientType == E_DRV_SCLDMA_3_FRM_W))
#define _Is_SC3Ringmode(enClientType) (_Is_RingMode(enClientType) && \
        (enClientType == E_DRV_SCLDMA_3_FRM_R || enClientType == E_DRV_SCLDMA_3_FRM_W))
#define _Is_SC3SWRingmode(enClientType) (_Is_SWRingMode(enClientType) && \
            (enClientType == E_DRV_SCLDMA_3_FRM_R || enClientType == E_DRV_SCLDMA_3_FRM_W))

#define _Is_OnlySC1SNPSingleDone(u32Event) (u32Event == E_DRV_SCLIRQ_EVENT_ISTSC1SNP &&_Is_SingleMode(E_DRV_SCLDMA_1_SNP_W))
#define _Is_OnlySC1FRMSingleDone(u32Event) (u32Event == E_DRV_SCLIRQ_EVENT_ISTSC1FRM &&_Is_SingleMode(E_DRV_SCLDMA_1_FRM_W))
#define _Is_OnlySC2FRMSingleDone(u32Event) (u32Event == E_DRV_SCLIRQ_EVENT_ISTSC2FRM &&_Is_SingleMode(E_DRV_SCLDMA_2_FRM_W))
#define _Is_VsrcId(enSCLDMA_ID) ((enSCLDMA_ID ==E_DRV_SCLDMA_ID_1_W)||(enSCLDMA_ID ==E_DRV_SCLDMA_ID_2_W))
#define SCLDMA_SIZE_ALIGN(x, align)                 ((x+align) & ~(align-1))
#define SCLDMA_CHECK_ALIGN(x, align)                (x & (align-1))

#define DRV_SCLDMA_MUTEX_LOCK()            //DrvSclOsObtainMutex(_SCLDMA_Mutex,SCLOS_WAIT_FOREVER)
#define DRV_SCLDMA_MUTEX_UNLOCK()          //DrvSclOsReleaseMutex(_SCLDMA_Mutex)
#define DRV_SCLDMA_MUTEX_LOCK_ISR()        DrvSclOsObtainMutexIrq(_SCLIRQ_SCLDMA_Mutex);
#define DRV_SCLDMA_MUTEX_UNLOCK_ISR()      DrvSclOsReleaseMutexIrq(_SCLIRQ_SCLDMA_Mutex);

#define PARSING_SCLDMA_ID(x)           (x==E_DRV_SCLDMA_ID_1_W   ? "SCLDMA_1_W" : \
                                        x==E_DRV_SCLDMA_ID_2_W   ? "SCLDMA_2_W" : \
                                        x==E_DRV_SCLDMA_ID_3_W   ? "SCLDMA_3_W" : \
                                        x==E_DRV_SCLDMA_ID_3_R   ? "SCLDMA_3_R" : \
                                        x==E_DRV_SCLDMA_ID_PNL_R ? "SCLDMA_PNL_R" : \
                                                               "UNKNOWN")



#define PARSING_SCLDMA_RWMD(x)        (x==E_DRV_SCLDMA_FRM_W ? "FRM_W" : \
                                       x==E_DRV_SCLDMA_SNP_W ? "SNP_W" : \
                                       x==E_DRV_SCLDMA_IMI_W ? "IMI_W" : \
                                       x==E_DRV_SCLDMA_FRM_R ? "FRM_R" : \
                                       x==E_DRV_SCLDMA_DBG_R ? "DBG_R" : \
                                       x==E_DRV_SCLDMA_FRM2_W ? "FRM2_W" : \
                                                           "UNKNOWN")

#define PARSING_SCLDMA_BUFMD(x)       (x==E_DRV_SCLDMA_BUF_MD_RING   ? "RING" : \
                                       x==E_DRV_SCLDMA_BUF_MD_SWRING ? "SWRING" : \
                                       x==E_DRV_SCLDMA_BUF_MD_SINGLE ? "SINGLE" : \
                                                                   "UNKNOWN")

#define PARSING_SCLDMA_COLOR(x)       (x==E_DRV_SCLDMA_COLOR_YUV422? "422PACK" : \
                                       x==E_DRV_SCLDMA_COLOR_YUV420 ? "YCSep420" : \
                                       x==E_DRV_SCLDMA_COLOR_YCSep422 ? "YCSep422" : \
                                       x==E_DRV_SCLDMA_COLOR_YUVSep422 ? "YUVSep422" : \
                                       x==E_DRV_SCLDMA_COLOR_YUVSep420 ? "YUVSep420" : \
                                                                  "UNKNOWN")

#define PARSING_SCLDMA_CLIENT(x)      (x==E_DRV_SCLDMA_1_FRM_W ? "E_DRV_SCLDMA_1_FRM_W" : \
                                       x==E_DRV_SCLDMA_1_SNP_W ? "E_DRV_SCLDMA_1_SNP_W" : \
                                       x==E_DRV_SCLDMA_1_IMI_W ? "E_DRV_SCLDMA_1_IMI_W" : \
                                       x==E_DRV_SCLDMA_2_FRM_W ? "E_DRV_SCLDMA_2_FRM_W" : \
                                       x==E_DRV_SCLDMA_2_FRM2_W ? "E_DRV_SCLDMA_2_FRM2_W" : \
                                       x==E_DRV_SCLDMA_2_IMI_W ? "E_DRV_SCLDMA_2_IMI_W" : \
                                       x==E_DRV_SCLDMA_3_FRM_W ? "E_DRV_SCLDMA_3_FRM_W" : \
                                       x==E_DRV_SCLDMA_3_FRM_R ? "E_DRV_SCLDMA_3_FRM_R" : \
                                       x==E_DRV_SCLDMA_4_FRM_R ? "E_DRV_SCLDMA_4_FRM_R" : \
                                                             "UNKNOWN")

#define PARSING_SCLDMA_ISR_LOG(x)       (x==E_DRV_SCLDMA_ISR_LOG_ISPOFF   ? "ISPOFF" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SNPONLY   ? "SNPONLY" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SNPISR   ? "SNPISR" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_ISPON   ? "ISPON" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC1ON ? "SC1ON" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC1OFF ? "SC1OFF" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC2ON ? "SC2ON" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC2OFF ? "SC2OFF" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC1A ? "SC1A" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC1N ? "SC1N" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC2A ? "SC2A" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_SC2N ? "SC2N" : \
                                        x==E_DRV_SCLDMA_ISR_LOG_FRMEND ? "FRMEND" : \
                                                               "UNKNOWN")

#define OMX_VSPL_POLLTIME 80*1000

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvSclDmaInfoType_t  *gstScldmaInfo;
//need isr or Ring mode

//keep
/////////////////
/// _SCLDMA_Mutex
/// use in scldma mutex,not include isr
////////////////
s32 _SCLDMA_Mutex        = -1;

/////////////////
/// _SCLIRQ_SCLDMA_Mutex
/// scldma and sclirq mutex, include isr
////////////////
s32 _SCLIRQ_SCLDMA_Mutex = -1;
MDrvSclCtxHalHvspGlobalSet_t *gstGlobalDmaHalHvspSet;

bool gbScldmaSuspend;
//-------------------------------------------------------------------------------------------------
//  Functions/
//-------------------------------------------------------------------------------------------------
void _DrvSclDmaSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstScldmaInfo = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stScldmaInfo);
}

void _DrvSclDmaSC1OnOff(DrvSclDmaRwModeType_e enRW,bool bEn)
{
    HalSclDmaSetSC1HandshakeForce(enRW, bEn);
    HalSclDmaSetSC1DMAEn(enRW, bEn);
}
void _DrvSclDmaSC2OnOff(DrvSclDmaRwModeType_e enRW,bool bEn)
{
    HalSclDmaSetSC2HandshakeForce(enRW, bEn);
    HalSclDmaSetSC1ToSC2HandshakeForce(bEn);
    HalSclDmaSetSC2DMAEn(enRW, bEn);
}
void _DrvSclDmaSetVsyncRegenMode(DrvSclDmaVsIdType_e enIDType, DrvSclDmaVsTrigModeType_e enTrigType)
{
    HalSclDmaSetVSyncRegenMode(enIDType,enTrigType);
}
void _DrvSclDmaSetVsyncTrigConfig(DrvSclDmaVsIdType_e enIDType)
{
    //hard code by hw setting
    HalSclDmaSetRegenVSyncStartPoint(enIDType,10);
    HalSclDmaSetRegenVSyncWidth(enIDType,40);
}
void _DrvSclDmaResetGlobalParameter(void)
{
    DRV_SCLDMA_MUTEX_LOCK();
    gbScldmaSuspend = 0;
    DRV_SCLDMA_MUTEX_UNLOCK();
}
void _DrvSclDmaResetGlobalParameterByClient(DrvSclDmaClientType_e u8ClientIdx)
{
    u8 u8BufferIdx;
    DRV_SCLDMA_MUTEX_LOCK_ISR();
    gstScldmaInfo->bDMAOnOff[u8ClientIdx]  = 0;
    DRV_SCLDMA_MUTEX_UNLOCK_ISR();
    DRV_SCLDMA_MUTEX_LOCK();
    gstScldmaInfo->enBuffMode[u8ClientIdx] = E_DRV_SCLDMA_BUF_MD_NUM;
    gstScldmaInfo->bMaxid[u8ClientIdx]     = 0;
    gstScldmaInfo->enColor[u8ClientIdx]    = 0;
    for(u8BufferIdx=0;u8BufferIdx<4;u8BufferIdx++)
    {
        gstScldmaInfo->u32Base_Y[u8ClientIdx][u8BufferIdx]     = 0;
        gstScldmaInfo->u32Base_C[u8ClientIdx][u8BufferIdx]     = 0;
        gstScldmaInfo->u32Base_V[u8ClientIdx][u8BufferIdx]     = 0;
    }
    DRV_SCLDMA_MUTEX_UNLOCK();
}

void _DrvSclDmaInitVariable(void)
{
    u8 u8ClientIdx;
    DrvSclOsMemset(gstScldmaInfo, 0, sizeof(DrvSclDmaInfoType_t));
    _DrvSclDmaResetGlobalParameter();
    for(u8ClientIdx=0; u8ClientIdx<E_DRV_SCLDMA_CLIENT_NUM; u8ClientIdx++)
    {
        _DrvSclDmaResetGlobalParameterByClient(u8ClientIdx);
    }
}


static DrvSclDmaClientType_e _DrvSclDmaTransToClientType
    (DrvSclDmaIdType_e enSCLDMA_ID, DrvSclDmaRwModeType_e enRWMode)
{
    DrvSclDmaClientType_e enClientType;

    if(enSCLDMA_ID == E_DRV_SCLDMA_ID_1_W)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_FRM_W ? E_DRV_SCLDMA_1_FRM_W :
                       enRWMode == E_DRV_SCLDMA_SNP_W ? E_DRV_SCLDMA_1_SNP_W :
                       enRWMode == E_DRV_SCLDMA_IMI_W ? E_DRV_SCLDMA_1_IMI_W :
                                                    E_DRV_SCLDMA_CLIENT_NUM;

    }
    else if(enSCLDMA_ID == E_DRV_SCLDMA_ID_2_W)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_FRM_W ? E_DRV_SCLDMA_2_FRM_W :
                       enRWMode == E_DRV_SCLDMA_FRM2_W ? E_DRV_SCLDMA_2_FRM2_W :
                       enRWMode == E_DRV_SCLDMA_IMI_W ? E_DRV_SCLDMA_2_IMI_W :
                                                    E_DRV_SCLDMA_CLIENT_NUM;
    }
    else if(enSCLDMA_ID == E_DRV_SCLDMA_ID_3_R)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_FRM_R ? E_DRV_SCLDMA_3_FRM_R :
                       enRWMode == E_DRV_SCLDMA_IMI_R ? E_DRV_SCLDMA_3_IMI_R :
                                                    E_DRV_SCLDMA_CLIENT_NUM;
    }
    else if(enSCLDMA_ID == E_DRV_SCLDMA_ID_3_W)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_FRM_W ? E_DRV_SCLDMA_3_FRM_W :
                                                    E_DRV_SCLDMA_CLIENT_NUM;

    }
    else if(enSCLDMA_ID == E_DRV_SCLDMA_ID_MDWIN_W)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_DWIN_W ? E_DRV_SCLDMA_4_FRM_W :
                                                    E_DRV_SCLDMA_CLIENT_NUM;

    }
    else if(enSCLDMA_ID == E_DRV_SCLDMA_ID_PNL_R)
    {
        enClientType = enRWMode == E_DRV_SCLDMA_DBG_R ? E_DRV_SCLDMA_4_FRM_R :
                                                    E_DRV_SCLDMA_CLIENT_NUM;
    }
    else
    {
        enClientType = enRWMode == E_DRV_SCLDMA_FRM_R ? E_DRV_SCLDMA_4_FRM_R :
                                                    E_DRV_SCLDMA_CLIENT_NUM;
    }

    return enClientType;
}


void _DrvSclDmaHWInitProcess(void)
{
    //_DrvSclDmaInit_FHD();
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRMSCLDMA]%s \n",__FUNCTION__);
    HalSclDmaHWInit();
    _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_SC3, E_DRV_SCLDMA_VS_TRIG_MODE_HW_IN_VSYNC);
    _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_SC, E_DRV_SCLDMA_VS_TRIG_MODE_HW_IN_VSYNC);
    _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_AFF, E_DRV_SCLDMA_VS_TRIG_MODE_HW_IN_VSYNC);
    //ToDo:maybe cause mdwin to hangup.
    _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_LDC, E_DRV_SCLDMA_VS_TRIG_MODE_HW_DELAY);
    _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_DISP, E_DRV_SCLDMA_VS_TRIG_MODE_HW_IN_VSYNC);
    HalSclDmaSetRegenVSyncVariableWidthEn(FALSE);
    HalSclDmaSetRegenVSyncStartPoint(E_DRV_SCLDMA_VS_ID_LDC,0x600);
    HalSclDmaSetRegenVSyncWidth(E_DRV_SCLDMA_VS_ID_LDC,0x40);
    HalSclDmaSetSC1HandshakeForce(E_DRV_SCLDMA_FRM_W, FALSE);
    HalSclDmaSetSC1HandshakeForce(E_DRV_SCLDMA_SNP_W, FALSE);
    HalSclDmaSetSC1HandshakeForce(E_DRV_SCLDMA_IMI_W, FALSE);
    HalSclDmaSetSC2HandshakeForce(E_DRV_SCLDMA_FRM_W, FALSE);
    HalSclDmaSetSC2HandshakeForce(E_DRV_SCLDMA_FRM2_W, FALSE);
    HalSclDmaSetSC2HandshakeForce(E_DRV_SCLDMA_IMI_W, FALSE);
    HalSclDmaSetSC1ToSC2HandshakeForce(FALSE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_2_IMI_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_1_IMI_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_2_FRM_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_1_FRM_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_1_SNP_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_3_FRM_W, TRUE);
    HalSclDmaSetCheckFrmEndSignal(E_DRV_SCLDMA_2_FRM2_W, TRUE);
    HalSclDmaSetMdwinInputFormat(E_DRV_SCLDMA_MDWIN_INPUT_YUV);
    HalSclDmaSetMdwinYCSwap(0);
    DrvSclIrqCmdqInterruptEnable(SCLIRQ_CMDQ_SC3_ENG_FRM_END);
}
void _DrvSclDmaSetSuspendFlagByClient(DrvSclDmaClientType_e enClientType)
{
    DRV_SCLDMA_MUTEX_LOCK_ISR();
    gstScldmaInfo->bDMAOnOff[enClientType] = FALSE;
    DRV_SCLDMA_MUTEX_UNLOCK_ISR();
}

void _DrvSclDmaResetGlobalSwitchByID(DrvSclDmaIdType_e enSCLDMA_ID)
{
    u8 u8ClientIdx;
    switch(enSCLDMA_ID)
    {
        case E_DRV_SCLDMA_ID_1_W:
            for(u8ClientIdx=0; u8ClientIdx<E_DRV_SCLDMA_2_FRM_W; u8ClientIdx++)
            {
                _DrvSclDmaResetGlobalParameterByClient(u8ClientIdx);
            }
            _DrvSclDmaResetGlobalParameter();
            break;

        case E_DRV_SCLDMA_ID_2_W:
            for(u8ClientIdx=E_DRV_SCLDMA_2_FRM_W; u8ClientIdx<E_DRV_SCLDMA_3_FRM_R; u8ClientIdx++)
            {
                _DrvSclDmaResetGlobalParameterByClient(u8ClientIdx);
            }
            break;

        case E_DRV_SCLDMA_ID_3_W:
        case E_DRV_SCLDMA_ID_3_R:
            for(u8ClientIdx=E_DRV_SCLDMA_3_FRM_R; u8ClientIdx<E_DRV_SCLDMA_4_FRM_R; u8ClientIdx++)
            {
                _DrvSclDmaResetGlobalParameterByClient(u8ClientIdx);
            }
            break;

        case E_DRV_SCLDMA_ID_PNL_R:
            _DrvSclDmaResetGlobalParameterByClient(E_DRV_SCLDMA_4_FRM_R);
            break;
        case E_DRV_SCLDMA_ID_MDWIN_W:
            _DrvSclDmaResetGlobalParameterByClient(E_DRV_SCLDMA_4_FRM_W);
            break;
        default:
            DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s Release fail\n", __FUNCTION__));
            break;
    }
}
void _DrvSclDmaSetDmaInformationForGlobal(DrvSclDmaClientType_e enClientType, DrvSclDmaRwConfig_t *stCfg)
{
    u8 u8BufferIdx;
    DRV_SCLDMA_MUTEX_LOCK();
    gstScldmaInfo->enBuffMode[enClientType] = stCfg->enBuffMode;
    gstScldmaInfo->bMaxid[enClientType] = stCfg->u8MaxIdx;
    gstScldmaInfo->enColor[enClientType] = stCfg->enColor;
    gstScldmaInfo->u16FrameWidth[enClientType] =  stCfg->u16Width;
    gstScldmaInfo->u16FrameHeight[enClientType] = stCfg->u16Height;
    gstScldmaInfo->bHFilp[enClientType] = stCfg->bHFlip;
    gstScldmaInfo->bVFilp[enClientType] = stCfg->bVFlip;
    for (u8BufferIdx = 0; u8BufferIdx <= stCfg->u8MaxIdx; u8BufferIdx++)
    {
        gstScldmaInfo->u32Base_Y[enClientType][u8BufferIdx] = stCfg->u32Base_Y[u8BufferIdx];
        gstScldmaInfo->u32Base_C[enClientType][u8BufferIdx] = stCfg->u32Base_C[u8BufferIdx];
        gstScldmaInfo->u32Base_V[enClientType][u8BufferIdx] = stCfg->u32Base_V[u8BufferIdx];
    }
    DRV_SCLDMA_MUTEX_UNLOCK();
}
void _DrvSclDmaHvspSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalDmaHalHvspSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stHalHvspCfg);
}
DrvSclHvspIpMuxType_e _DrvSclDmaGetInputMuxType(DrvSclHvspIdType_e enIpType)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclDmaHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    return gstGlobalDmaHalHvspSet->genIpType[enIpType];
}
void _DrvSclDmaSetVsyncTrigMode(DrvSclDmaClientType_e enClientType)
{
    DrvSclHvspIpMuxType_e enIpType;
    enIpType = (_DrvSclDmaGetInputMuxType(E_DRV_SCLHVSP_ID_3));
    if(enIpType==E_DRV_SCLHVSP_IP_MUX_LDC || enIpType==E_DRV_SCLHVSP_IP_MUX_RSC)
    {
        if(_Is_SC3Singlemode(enClientType))
        {
            _DrvSclDmaSetVsyncTrigConfig(E_DRV_SCLDMA_VS_ID_SC3);
            _DrvSclDmaSetVsyncRegenMode(E_DRV_SCLDMA_VS_ID_SC3, E_DRV_SCLDMA_VS_TRIG_MODE_SWTRIGGER);
        }
    }
}
void _DrvSclDmaSysInit(bool bEn)
{
    if(bEn)
    {
        DrvSclDmaRwConfig_t stIMICfg;
        void *pvCtx;
        pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
        DrvSclOsMemset(&stIMICfg,0,sizeof(DrvSclDmaRwConfig_t));
        stIMICfg.enRWMode       = E_DRV_SCLDMA_IMI_W;
        stIMICfg.u16Height      = HD_Height;
        stIMICfg.u16Width       = HD_Width;
        stIMICfg.u8MaxIdx       = 0;
        stIMICfg.u8Flag         = 1;
        stIMICfg.enBuffMode     = E_DRV_SCLDMA_BUF_MD_RING;
        stIMICfg.u32Base_Y[0]   = SCL_IMIinitAddr;
        stIMICfg.u32Base_C[0]   = SCL_IMIinitAddr;
        stIMICfg.u32Base_V[0]   = SCL_IMIinitAddr;
        stIMICfg.enColor        = E_DRV_SCLDMA_COLOR_YUV420 ;
        stIMICfg.pvCtx = pvCtx;
        DrvSclDmaSetDmaClientConfig(E_DRV_SCLDMA_ID_1_W,&stIMICfg);
    }
    else
    {
        HalSclDmaSetIMIClientReset();
    }

}
void _DrvSclDmaSetSC1DMAConfig(DrvSclDmaProcessConfig_t *pBuffer,bool bEnLineOffset)
{
    HalSclDmaSetSC1DMABufferConfig(pBuffer->stCfg.enRWMode,&pBuffer->stCfg.stBufferInfo,bEnLineOffset);
    _DrvSclDmaSC1OnOff(pBuffer->stCfg.enRWMode,pBuffer->stCfg.bEn);
}
void _DrvSclDmaSetSC2DMAConfig(DrvSclDmaProcessConfig_t *pBuffer,bool bEnLineOffset)
{
    HalSclDmaSetSC2DMABufferConfig(pBuffer->stCfg.enRWMode,&pBuffer->stCfg.stBufferInfo,bEnLineOffset);
    _DrvSclDmaSC2OnOff(pBuffer->stCfg.enRWMode,pBuffer->stCfg.bEn);
}
void _DrvSclDmaSetSC3DMAConfig(DrvSclDmaProcessConfig_t *pBuffer,bool bEnLineOffset)
{
    DrvSclHvspIpMuxType_e enIpType;
    enIpType = (_DrvSclDmaGetInputMuxType(E_DRV_SCLHVSP_ID_3));
    HalSclDmaSetSC3DMABufferConfig(pBuffer->stCfg.enRWMode,&pBuffer->stCfg.stBufferInfo,bEnLineOffset);
    HalSclDmaSetSC3DMAEn(pBuffer->stCfg.enRWMode, pBuffer->stCfg.bEn);
    HalSclDmaSetSC1ToSC3HandshakeForce(pBuffer->stCfg.bEn);
}
void _DrvSclDmaSetMDwinConfig(DrvSclDmaProcessConfig_t *pBuffer)
{
    HalSclDmaSetSC4MDwinBufferConfig(E_DRV_SCLDMA_DWIN_W,&pBuffer->stCfg.stBufferInfo);
    HalSclDmaSetSC4DMAEn(E_DRV_SCLDMA_DWIN_W, pBuffer->stCfg.bEn);
    HalSclDmaSetSC4HandshakeForce(pBuffer->stCfg.bEn);
}
bool _DrvSclDmaSetDMALineOffsetOnOff(DrvSclDmaClientType_e enClientType )
{
    if((gstScldmaInfo->u32LineOffset_Y[enClientType] == (gstScldmaInfo->u16FrameWidth[enClientType]))||
        !gstScldmaInfo->u32LineOffset_Y[enClientType])
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
void _DrvSclDmaSetDmaProcessInfoForGlobal(DrvSclDmaClientType_e enClientType,DrvSclDmaProcessConfig_t *pBuffer)
{
    DRV_SCLDMA_MUTEX_LOCK();
    gstScldmaInfo->u32Base_Y[enClientType][0] = pBuffer->stCfg.stBufferInfo.u64PhyAddr[0];
    gstScldmaInfo->u32Base_C[enClientType][0] = pBuffer->stCfg.stBufferInfo.u64PhyAddr[1];
    gstScldmaInfo->u32Base_V[enClientType][0] = pBuffer->stCfg.stBufferInfo.u64PhyAddr[2];

    gstScldmaInfo->u32LineOffset_Y[enClientType] = pBuffer->stCfg.stBufferInfo.u32Stride[0];
    gstScldmaInfo->u32LineOffset_en[enClientType] = _DrvSclDmaSetDMALineOffsetOnOff(enClientType);

    gstScldmaInfo->bDMAOnOff[enClientType] = pBuffer->stCfg.bEn;

    DRV_SCLDMA_MUTEX_UNLOCK();
}
void _DrvSclDmaProcessDbg(DrvSclDmaClientType_e enClientType,DrvSclDmaProcessConfig_t *pBuffer)
{
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE,
        "[SCLDMA]%s Prot:%d En:%hhd PhyAddr:{0x%llx,0x%llx,0x%llx},Stride:{0x%lx,0x%lx,0x%lx}\n",
        __FUNCTION__,enClientType,pBuffer->stCfg.bEn,
        pBuffer->stCfg.stBufferInfo.u64PhyAddr[0],
        pBuffer->stCfg.stBufferInfo.u64PhyAddr[1],
        pBuffer->stCfg.stBufferInfo.u64PhyAddr[2],
        pBuffer->stCfg.stBufferInfo.u32Stride[0],
        pBuffer->stCfg.stBufferInfo.u32Stride[1],
        pBuffer->stCfg.stBufferInfo.u32Stride[2]);
}
bool DrvSclDmaSuspend(DrvSclDmaIdType_e enSCLDMA_ID, DrvSclDmaSuspendResumeConfig_t *pCfg)
{
    DrvSclIrqSuspendResumeConfig_t stSclIrqCfg;
    u8 u8Clientidx;
    bool bRet = TRUE;
    bool bAllClientOn = 0;
    DrvSclOsMemset(&stSclIrqCfg,0,sizeof(DrvSclIrqSuspendResumeConfig_t));
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pCfg->pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s(%d), ID:%s(%d), bSuspend=%d\n",
        __FUNCTION__, __LINE__,PARSING_SCLDMA_ID(enSCLDMA_ID), enSCLDMA_ID,gbScldmaSuspend);

    switch(enSCLDMA_ID)
    {
        case E_DRV_SCLDMA_ID_1_W:
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_1_FRM_W);
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_1_SNP_W);
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_1_IMI_W);
            break;

        case E_DRV_SCLDMA_ID_2_W:
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_2_FRM_W);
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_2_FRM2_W);
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_2_IMI_W);
            break;

        case E_DRV_SCLDMA_ID_3_W:
        case E_DRV_SCLDMA_ID_3_R:
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_3_FRM_W);
            _DrvSclDmaSetSuspendFlagByClient(E_DRV_SCLDMA_3_FRM_R);
            break;

        case E_DRV_SCLDMA_ID_PNL_R:
            DRV_SCLDMA_MUTEX_LOCK_ISR();
                gstScldmaInfo->bDMAOnOff[E_DRV_SCLDMA_4_FRM_R] = FALSE;
            DRV_SCLDMA_MUTEX_UNLOCK_ISR();
            break;

        default:
            DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s Suspend fail\n", __FUNCTION__));
            return FALSE;
    }

    for(u8Clientidx=E_DRV_SCLDMA_1_FRM_W ;u8Clientidx<E_DRV_SCLDMA_CLIENT_NUM; u8Clientidx++)
    {
        DRV_SCLDMA_MUTEX_LOCK_ISR();
        bAllClientOn |= gstScldmaInfo->bDMAOnOff[u8Clientidx];
        DRV_SCLDMA_MUTEX_UNLOCK_ISR();
    }

    if(gbScldmaSuspend == 0)
    {
        if(bAllClientOn == 0)
        {
            stSclIrqCfg.u32IRQNUM = pCfg->u32IRQNum;
            if(DrvSclIrqSuspend(&stSclIrqCfg))
            {
                bRet = TRUE;
                gbScldmaSuspend = 1;
            }
            else
            {
                DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s Suspend fail\n", __FUNCTION__));
                bRet = FALSE;
            }
        }
        else
        {
            SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s not all scldma suspend\n",__FUNCTION__);
            bRet = TRUE;
        }
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s already suspend\n",__FUNCTION__);
        bRet = TRUE;
    }

    return bRet;
}

bool DrvSclDmaResume(DrvSclDmaIdType_e enSCLDMA_ID, DrvSclDmaSuspendResumeConfig_t *pCfg)
{
    DrvSclIrqSuspendResumeConfig_t stSclIrqCfg;
    bool bRet = TRUE;
    DrvSclOsMemset(&stSclIrqCfg,0,sizeof(DrvSclIrqSuspendResumeConfig_t));
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pCfg->pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s(%d), ID:%s(%d), bSuspend=%d\n",
        __FUNCTION__, __LINE__, PARSING_SCLDMA_ID(enSCLDMA_ID), enSCLDMA_ID, gbScldmaSuspend);

    if(gbScldmaSuspend == 1)
    {
        stSclIrqCfg.u32IRQNUM = pCfg->u32IRQNum;
        if(DrvSclIrqResume(&stSclIrqCfg))
        {
            _DrvSclDmaHWInitProcess();
            gbScldmaSuspend = 0;
            bRet = TRUE;
        }
        else
        {
            DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s Resume fail\n", __FUNCTION__));
            bRet = FALSE;
        }
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s already Resume\n",__FUNCTION__);
        bRet = TRUE;
    }

    return bRet;
}
void DrvSclDmaExit(bool bCloseISR)
{
    if(_SCLDMA_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_SCLDMA_Mutex);
        _SCLDMA_Mutex = -1;
    }
    if(_SCLIRQ_SCLDMA_Mutex != -1)
    {
        DrvSclOsDeleteSpinlock(_SCLIRQ_SCLDMA_Mutex);
        _SCLDMA_Mutex = -1;
    }
    if(bCloseISR)
    {
        DrvSclIrqExit();
    }
}
bool DrvSclDmaInit(DrvSclDmaInitConfig_t *pInitCfg)
{
    char word[]     = {"_SCLDMA_Mutex"};
    char word2[]    = {"_IRQDMA_Mutex"};
    //int i;
    DrvSclIrqInitConfig_t stIRQInitCfg;
    DrvSclOsMemset(&stIRQInitCfg,0,sizeof(DrvSclIrqInitConfig_t));
    if(_SCLDMA_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s already done\n",__FUNCTION__);
        return TRUE;
    }

    if(DrvSclOsInit() == FALSE)
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s DrvSclOsInit Fail\n", __FUNCTION__));
        return FALSE;
    }

    _SCLDMA_Mutex           = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);
    _SCLIRQ_SCLDMA_Mutex    = DrvSclOsCreateSpinlock(E_DRV_SCLOS_FIFO, word2, SCLOS_PROCESS_SHARED);
    if (_SCLDMA_Mutex == -1)
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s create mutex fail\n", __FUNCTION__));
        return FALSE;
    }

    stIRQInitCfg.u32RiuBase    = pInitCfg->u32RIUBase;
    stIRQInitCfg.u32IRQNUM     = pInitCfg->u32IRQNUM;
    stIRQInitCfg.u32CMDQIRQNUM = pInitCfg->u32CMDQIRQNUM;
    stIRQInitCfg.pvCtx         = pInitCfg->pvCtx;
    // init processing
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pInitCfg->pvCtx);
    HalSclDmaSetRiuBase(pInitCfg->u32RIUBase);
    if(DrvSclIrqInit(&stIRQInitCfg) == FALSE)
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s Init IRQ Fail\n", __FUNCTION__));
        return FALSE;
    }
    _DrvSclDmaInitVariable();
    _DrvSclDmaHWInitProcess();
    _DrvSclDmaSysInit(1);
    return TRUE;
}
void DrvSclDmaReSetHw(DrvSclDmaIdType_e enSCLDMA_ID,DrvSclDmaClkConfig_t *stclk)
{
    HalSclDmaReSetHw();
}
void DrvSclDmaRelease(DrvSclDmaIdType_e enSCLDMA_ID,DrvSclDmaClkConfig_t *stclk)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&EN_DBGMG_SCLDMALEVEL_ELSE, "[DRVSCLDMA]%s \n",__FUNCTION__);
    _DrvSclDmaResetGlobalSwitchByID(enSCLDMA_ID);
    DrvSclIrqInitVariable();
}


s32* DrvSclDmaGetDmaandIRQCommonMutex(void)
{
    return &_SCLIRQ_SCLDMA_Mutex;
}
bool _DrvSclDmaPixelFormatProcessByColor
    (DrvSclDmaProcessConfig_t *pstProcess,DrvSclDmaColorType_e enColor,DrvSclDmaClientType_e enClientType)
{
    switch(enColor)
    {
        case E_DRV_SCLDMA_COLOR_YUV422:
            pstProcess->stCfg.stBufferInfo.u64PhyAddr[1] =
                pstProcess->stCfg.stBufferInfo.u64PhyAddr[0]+16;
            pstProcess->stCfg.stBufferInfo.u32Stride[0] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0]/2;
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        case E_DRV_SCLDMA_COLOR_YUV420:
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        case E_DRV_SCLDMA_COLOR_YCSep422:
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        case E_DRV_SCLDMA_COLOR_YUVSep422:
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            pstProcess->stCfg.stBufferInfo.u32Stride[2] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        case E_DRV_SCLDMA_COLOR_YUVSep420:
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            pstProcess->stCfg.stBufferInfo.u32Stride[2] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        default:
            return 0;
    }
    return 1 ;
}
bool _DrvSclDmaPixelFormatProcessByMDwinColor
    (DrvSclDmaProcessConfig_t *pstProcess,DrvSclDmaColorType_e enColor)
{
    switch(enColor)
    {
        case E_DRV_MDWIN_COLOR_YUV422:
        case E_DRV_MDWIN_COLOR_YUV422CE:
        case E_DRV_MDWIN_COLOR_YUV4226CE:
            pstProcess->stCfg.stBufferInfo.u32Stride[0] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0]/2;
            break;
        case E_DRV_MDWIN_COLOR_YUV420:
        case E_DRV_MDWIN_COLOR_YUV420CE:
        case E_DRV_MDWIN_COLOR_YUV4206CE:
            pstProcess->stCfg.stBufferInfo.u32Stride[0] =
                (pstProcess->stCfg.stBufferInfo.u32Stride[0]*2/3);
            pstProcess->stCfg.stBufferInfo.u32Stride[1] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0];
            break;
        case E_DRV_MDWIN_COLOR_YUV444:
        case E_DRV_MDWIN_COLOR_YUV444A:
        case E_DRV_MDWIN_COLOR_ARGB:
        case E_DRV_MDWIN_COLOR_ABGR:
        case E_DRV_MDWIN_COLOR_RGBA:
        case E_DRV_MDWIN_COLOR_BGRA:
            pstProcess->stCfg.stBufferInfo.u32Stride[0] =
                pstProcess->stCfg.stBufferInfo.u32Stride[0]/3;
            break;
        default:
            return 0;
    }
    return 1 ;
}
bool _DrvSclDmaPixelFormatProcess(DrvSclDmaClientType_e enClientType,DrvSclDmaProcessConfig_t *pstProcess)
{
    if(enClientType!= E_DRV_SCLDMA_4_FRM_W)
    {
        _DrvSclDmaPixelFormatProcessByColor(pstProcess,gstScldmaInfo->enColor[enClientType],enClientType);
    }
    else
    {
        _DrvSclDmaPixelFormatProcessByMDwinColor(pstProcess,gstScldmaInfo->enColor[enClientType]);
    }
    return 1;
}
bool DrvSclDmaInstProcess(DrvSclDmaIdType_e enSCLDMA_ID,DrvSclDmaProcessConfig_t *pstProcess)
{
    bool bRet = 1;
    DrvSclDmaClientType_e enClientType;
    enClientType = _DrvSclDmaTransToClientType(enSCLDMA_ID ,pstProcess->stCfg.enRWMode);
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pstProcess->pvCtx);
    _DrvSclDmaProcessDbg(enClientType,pstProcess);
    // check
    if( SCLDMA_CHECK_ALIGN(pstProcess->stCfg.stBufferInfo.u64PhyAddr[0], 8))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: YBase must be 8 byte align\n", __FUNCTION__, __LINE__));
        bRet = FALSE;
        return bRet;
    }
    _DrvSclDmaPixelFormatProcess(enClientType,pstProcess);
    _DrvSclDmaSetDmaProcessInfoForGlobal(enClientType,pstProcess);

    switch(enSCLDMA_ID)
    {
        case E_DRV_SCLDMA_ID_1_W:
            _DrvSclDmaSetSC1DMAConfig(pstProcess,gstScldmaInfo->u32LineOffset_en[enClientType]);
            break;

        case E_DRV_SCLDMA_ID_2_W:
            _DrvSclDmaSetSC2DMAConfig(pstProcess,gstScldmaInfo->u32LineOffset_en[enClientType]);
            break;

        case E_DRV_SCLDMA_ID_3_W:
        case E_DRV_SCLDMA_ID_3_R:
            _DrvSclDmaSetSC3DMAConfig(pstProcess,gstScldmaInfo->u32LineOffset_en[enClientType]);
            break;

        case E_DRV_SCLDMA_ID_PNL_R:
        case E_DRV_SCLDMA_ID_MDWIN_W:
            _DrvSclDmaSetMDwinConfig(pstProcess);
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

bool DrvSclDmaSetDmaClientConfig(DrvSclDmaIdType_e enSCLDMA_ID, DrvSclDmaRwConfig_t *stCfg)
{
    bool bRet = TRUE;
    u32 u32Time = 0;
    DrvSclDmaClientType_e enClientType;
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)stCfg->pvCtx);
    u32Time = (((u32)DrvSclOsGetSystemTimeStamp()));
    enClientType = _DrvSclDmaTransToClientType(enSCLDMA_ID ,stCfg->enRWMode);
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&Get_DBGMG_SCLDMAclient(enClientType,0),
        "[DRVSCLDMA]%s %d, DmaID:%s(%d), Flag:%x, RW:%s(%d), Buf:%s(%d), color:%s(%d), (W:%d, H:%d)@:%lu\n",
        __FUNCTION__, __LINE__,
        PARSING_SCLDMA_ID(enSCLDMA_ID), enSCLDMA_ID,
        stCfg->u8Flag,
        PARSING_SCLDMA_RWMD(stCfg->enRWMode), stCfg->enRWMode,
        PARSING_SCLDMA_BUFMD(stCfg->enBuffMode), stCfg->enBuffMode,
        PARSING_SCLDMA_COLOR(stCfg->enColor), stCfg->enColor,
        stCfg->u16Width, stCfg->u16Height,u32Time);
    SCL_DBG(SCL_DBG_LV_DRVSCLDMA()&Get_DBGMG_SCLDMAclient(enClientType,0),
        "[DRVSCLDMA]%s %d, maxbuf:%d, (%lx, %lx, %lx), (%lx, %lx, %lx),(%lx, %lx, %lx), (%lx, %lx, %lx)@:%lu\n",
        __FUNCTION__, __LINE__,
        stCfg->u8MaxIdx,
        stCfg->u32Base_Y[0], stCfg->u32Base_C[0],stCfg->u32Base_V[0],
        stCfg->u32Base_Y[1], stCfg->u32Base_C[1],stCfg->u32Base_V[1],
        stCfg->u32Base_Y[2], stCfg->u32Base_C[2],stCfg->u32Base_V[2],
        stCfg->u32Base_Y[3], stCfg->u32Base_C[3],stCfg->u32Base_V[3],u32Time);

    // check
    if(SCLDMA_CHECK_ALIGN(stCfg->u16Height, 2))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]: Height must be align 2\n"));
        bRet = FALSE;
    }

    if(stCfg->enColor == E_DRV_SCLDMA_COLOR_YUV422 && SCLDMA_CHECK_ALIGN(stCfg->u16Width, 8))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: Width must be align 8\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }
    else if(stCfg->enColor == E_DRV_SCLDMA_COLOR_YUV420 &&SCLDMA_CHECK_ALIGN(stCfg->u16Width, 16))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: Width must be align 16\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }
    else if(stCfg->enColor == E_DRV_SCLDMA_COLOR_YCSep422 &&SCLDMA_CHECK_ALIGN(stCfg->u16Width, 16))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: Width must be align 16\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }
    else if(stCfg->enColor == E_DRV_SCLDMA_COLOR_YUVSep422 &&SCLDMA_CHECK_ALIGN(stCfg->u16Width, 16))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: Width must be align 16\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }
    else if(stCfg->enColor == E_DRV_SCLDMA_COLOR_YUVSep420 &&SCLDMA_CHECK_ALIGN(stCfg->u16Width, 16))
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: Width must be align 16\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }

    if(stCfg->enBuffMode == E_DRV_SCLDMA_BUF_MD_SINGLE &&  stCfg->u8MaxIdx > 1)
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: More than 1 buffer to SINGLE mode\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }

    if(stCfg->enRWMode == E_DRV_SCLDMA_RW_NUM)
    {
        DRV_SCLDMA_ERR(sclprintf("[DRVSCLDMA]%s %d: RW mode is not coreect\n", __FUNCTION__, __LINE__));
        bRet =  FALSE;
    }
    if(bRet == FALSE)
    {
        return FALSE;
    }

    _DrvSclDmaSetDmaInformationForGlobal(enClientType, stCfg);
    _DrvSclDmaSetVsyncTrigMode(enClientType);

    switch(enSCLDMA_ID)
    {
        case E_DRV_SCLDMA_ID_1_W:
            DRV_SCLDMA_MUTEX_LOCK();
             HalSclDmaSetSC1DMAConfig(stCfg);
             DRV_SCLDMA_MUTEX_UNLOCK();
            break;

        case E_DRV_SCLDMA_ID_2_W:
            DRV_SCLDMA_MUTEX_LOCK();
            HalSclDmaSetSC2DMAConfig(stCfg);
            DRV_SCLDMA_MUTEX_UNLOCK();
            break;

        case E_DRV_SCLDMA_ID_3_W:
        case E_DRV_SCLDMA_ID_3_R:
            DRV_SCLDMA_MUTEX_LOCK();
            HalSclDmaSetSC3DMAConfig(stCfg);
            DRV_SCLDMA_MUTEX_UNLOCK();
            break;

        case E_DRV_SCLDMA_ID_PNL_R:
        case E_DRV_SCLDMA_ID_MDWIN_W:
            DRV_SCLDMA_MUTEX_LOCK();
            HalSclDmaSetDisplayDMAConfig(stCfg);
            DRV_SCLDMA_MUTEX_UNLOCK();
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

bool DrvSclDmaGetDmaInformationByClient
    (DrvSclDmaIdType_e enSCLDMA_ID,DrvSclDmaRwModeType_e enRWMode,DrvSclDmaAttrType_t *pstAttr)
{
    DrvSclDmaClientType_e enClientType;
    u8 u8BufferIdx;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _DrvSclDmaSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    enClientType        = _DrvSclDmaTransToClientType(enSCLDMA_ID ,enRWMode);
    pstAttr->u16DMAcount  = HalSclDmaGetDMAOutputCount(enClientType);
    pstAttr->u16DMAH      = HalSclDmaGetOutputHsize(enClientType);
    pstAttr->u16DMAV      = HalSclDmaGetOutputVsize(enClientType);
    pstAttr->enBuffMode   = gstScldmaInfo->enBuffMode[enClientType];
    pstAttr->enColor      = gstScldmaInfo->enColor[enClientType];
    pstAttr->u8MaxIdx     = gstScldmaInfo->bMaxid[enClientType];
    pstAttr->u32LineOffset_en = gstScldmaInfo->u32LineOffset_en[enClientType];
    pstAttr->u32LineOffset_Y = gstScldmaInfo->u32LineOffset_Y[enClientType];
    pstAttr->bHFilp = gstScldmaInfo->bHFilp[enClientType];
    pstAttr->bVFilp = gstScldmaInfo->bVFilp[enClientType];
    for(u8BufferIdx=0;u8BufferIdx<=pstAttr->u8MaxIdx;u8BufferIdx++)
    {
        pstAttr->u32Base_Y[u8BufferIdx] = gstScldmaInfo->u32Base_Y[enClientType][u8BufferIdx];
        pstAttr->u32Base_C[u8BufferIdx] = gstScldmaInfo->u32Base_C[enClientType][u8BufferIdx];
        pstAttr->u32Base_V[u8BufferIdx] = gstScldmaInfo->u32Base_V[enClientType][u8BufferIdx];
    }
    pstAttr->bDMAEn = gstScldmaInfo->bDMAOnOff[enClientType];
    return 1;
}
#undef DRV_SCLDMA_C
