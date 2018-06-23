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
#define DRV_HVSP_C


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_hvsp_st.h"
#include "hal_scl_hvsp.h"
#include "drv_scl_hvsp.h"
#include "drv_scl_dma_st.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "hal_scl_reg.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_HVSP_MUTEX_LOCK()            //DrvSclOsObtainMutex(_HVSP_Mutex,SCLOS_WAIT_FOREVER)
#define DRV_HVSP_MUTEX_UNLOCK()          //DrvSclOsReleaseMutex(_HVSP_Mutex)
#define FHD_Width   1920
#define FHD_Height  1080
#define _3M_Width   2048
#define _3M_Height  1536
#define HD_Width    1280
#define HD_Height   720
#define D_Width    704
#define D_Height   576
#define PNL_Width   800
#define PNL_Height  480
#define SRAMFORSCALDOWN 0x10
#define SRAMFORSCALUP 0x21
#define SRAMFORSC2ALDOWN 0x20
#define SRAMFORSC2ALUP 0x21
#define SRAMFORSC3HDOWN 0x00
#define SRAMFORSC3HUP 0x21
#define SRAMFORSC3VDOWN 0x00
#define SRAMFORSC3VUP 0x21
#define SRAMFORSC4ALDOWN 0x10
#define SRAMFORSC4ALUP 0x21
#define _CHANGE_SRAM_V_QMAP(enHVSP_ID,up)         (((gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height\
    > gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height)&&(up)) || (((gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height\
    < gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height) && !(up))))
#define _CHANGE_SRAM_H_QMAP(enHVSP_ID,up)         (((gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width\
        > gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width)&&(up)) || (((gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width\
        < gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width) && !(up))))
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MDrvSclCtxHvspGlobalSet_t *gstGlobalHvspSet;

//keep
s32 _HVSP_Mutex = -1;
/////////////////
/// gbHvspSuspend
/// To Save suspend status.
////////////////
bool gbHvspSuspend = 0;

//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------

void _DrvSclHvspSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalHvspSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stHvspCfg);
}
void _DrvSclHvspInitSWVarialbe(DrvSclHvspIdType_e HVSP_IP, void *pvCtx)
{
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    DrvSclOsMemset(&gstGlobalHvspSet->gstScalinInfo[HVSP_IP], 0, sizeof(DrvSclHvspScalingInfo_t));
    gstGlobalHvspSet->gbVScalingup[HVSP_IP] = SRAMFORSCALDOWN;
    gstGlobalHvspSet->gbHScalingup[HVSP_IP] = SRAMFORSCALDOWN;
    gstGlobalHvspSet->gbSc3FirstHSet = 1;
    gstGlobalHvspSet->gbSc3FirstVSet = 1;
    if(HVSP_IP == E_DRV_SCLHVSP_ID_1)
    {
        DrvSclOsMemset(&gstGlobalHvspSet->gstIPMCfg, 0, sizeof(DrvSclHvspIpmConfig_t));
        gstGlobalHvspSet->gstSrcSize.u16Height    = FHD_Height;
        gstGlobalHvspSet->gstSrcSize.u16Width     = FHD_Width;
        gstGlobalHvspSet->gstSrcSize.bSet         = 0;
        HalSclHvspSetInputSrcSize(&gstGlobalHvspSet->gstSrcSize);
    }

}
void _Drv_HVSP_SetCoringThrdOn(DrvSclHvspIdType_e enHVSP_ID)
{
    HalSclHvspSetHspCoringThrdC(enHVSP_ID,0x1);
    HalSclHvspSetHspCoringThrdY(enHVSP_ID,0x1);
    HalSclHvspSetVspCoringThrdC(enHVSP_ID,0x1);
    HalSclHvspSetVspCoringThrdY(enHVSP_ID,0x1);
}


//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------

void DrvSclHvspReSetHw(void *pvCtx)
{
    HalSclHvspReSetHw(pvCtx);
}
bool DrvSclHvspSuspend(DrvSclHvspSuspendResumeConfig_t *pCfg)
{
    DrvSclIrqSuspendResumeConfig_t stSclirq;
    bool bRet = TRUE;
    DrvSclOsMemset(&stSclirq,0,sizeof(DrvSclIrqSuspendResumeConfig_t));
    DrvSclOsObtainMutex(_HVSP_Mutex, SCLOS_WAIT_FOREVER);
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pCfg->pvCtx);
    HalSclHvspSetReset((MDrvSclCtxCmdqConfig_t *)pCfg->pvCtx);
    if(gbHvspSuspend == 0)
    {
        stSclirq.u32IRQNUM =  pCfg->u32IRQNUM;
        stSclirq.u32CMDQIRQNUM =  pCfg->u32CMDQIRQNUM;
        if(DrvSclIrqSuspend(&stSclirq))
        {
            bRet = TRUE;
            gbHvspSuspend = 1;
        }
        else
        {
            bRet = FALSE;
            SCL_ERR("[DRVHVSP]%s(%d) Suspend IRQ Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(3)), "[DRVHVSP]%s(%d) alrady suspned\n", __FUNCTION__, __LINE__);
        bRet = TRUE;
    }

    DrvSclOsReleaseMutex(_HVSP_Mutex);

    return bRet;
}
void _Drv_HVSP_SetHWinit(void)
{
    HalSclHvspSetTestPatCfg();
    HalSclHvspVtrackSetUUID();
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V,SRAMFORSCALUP);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_1,SRAMFORSC2ALDOWN);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_2,SRAMFORSC3VDOWN);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_V_3,SRAMFORSC4ALDOWN);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H,SRAMFORSCALUP);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_1,SRAMFORSC2ALDOWN);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_2,SRAMFORSC3HDOWN);
    HalSclHvspSramDump(E_HAL_SCLHVSP_SRAM_DUMP_HVSP_H_3,SRAMFORSC4ALDOWN);
}

bool DrvSclHvspResume(DrvSclHvspSuspendResumeConfig_t *pCfg)
{
    DrvSclIrqSuspendResumeConfig_t stSclirq;
    bool bRet = TRUE;
    DrvSclOsMemset(&stSclirq,0,sizeof(DrvSclIrqSuspendResumeConfig_t));
    DrvSclOsObtainMutex(_HVSP_Mutex, SCLOS_WAIT_FOREVER);
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pCfg->pvCtx);
    //sclprintf("%s,(%d) %d\n", __FUNCTION__, __LINE__, gbHvspSuspend);
    if(gbHvspSuspend == 1)
    {
        stSclirq.u32IRQNUM =  pCfg->u32IRQNUM;
        stSclirq.u32CMDQIRQNUM =  pCfg->u32CMDQIRQNUM;
        if(DrvSclIrqResume(&stSclirq))
        {
            //DrvSclIrqInterruptEnable(SCLIRQ_VSYNC_FCLK_LDC);
            HalSclHvspSetTestPatCfg();
            gbHvspSuspend = 0;
            bRet = TRUE;
        }
        else
        {

            SCL_ERR("[DRVHVSP]%s(%d) Resume IRQ Fail\n", __FUNCTION__, __LINE__);
            bRet = FALSE;
        }
        _Drv_HVSP_SetHWinit();
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(3)), "[DRVHVSP]%s(%d) alrady resume\n", __FUNCTION__, __LINE__);
        bRet = TRUE;
    }

    DrvSclOsReleaseMutex(_HVSP_Mutex);

    return bRet;
}
void DrvSclHvspExit(u8 bCloseISR)
{
    if(_HVSP_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_HVSP_Mutex);
        _HVSP_Mutex = -1;
    }
    if(bCloseISR)
    {
        DrvSclIrqExit();
    }
    HalSclHvspMloadSramBufferFree();
    HalSclHvspExit();
}

bool DrvSclHvspInit(DrvSclHvspInitConfig_t *pInitCfg)
{
    char word[] = {"_HVSP_Mutex"};
    DrvSclIrqInitConfig_t stIRQInitCfg;
    u8 u8IDidx;
    DrvSclOsMemset(&stIRQInitCfg,0,sizeof(DrvSclIrqInitConfig_t));
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pInitCfg->pvCtx);
    if(_HVSP_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(3)), "[DRVHVSP]%s(%d) alrady done\n", __FUNCTION__, __LINE__);
        return TRUE;
    }

    if(DrvSclOsInit() == FALSE)
    {
        SCL_ERR("[DRVHVSP]%s(%d) DrvSclOsInit Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    _HVSP_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);

    if (_HVSP_Mutex == -1)
    {
        SCL_ERR("[DRVHVSP]%s(%d): create mutex fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    stIRQInitCfg.u32RiuBase = pInitCfg->u32RIUBase;
    stIRQInitCfg.u32IRQNUM  = pInitCfg->u32IRQNUM;
    stIRQInitCfg.u32CMDQIRQNUM  = pInitCfg->u32CMDQIRQNUM;
    stIRQInitCfg.pvCtx = pInitCfg->pvCtx;
    if(DrvSclIrqInit(&stIRQInitCfg) == FALSE)
    {
        SCL_ERR("[DRVHVSP]%s(%d) Init IRQ Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    HalSclHvspSetRiuBase(pInitCfg->u32RIUBase);
    DRV_HVSP_MUTEX_LOCK();
    HalSclHvspSetReset((MDrvSclCtxCmdqConfig_t *)pInitCfg->pvCtx);
    for(u8IDidx = E_DRV_SCLHVSP_ID_1; u8IDidx<E_DRV_SCLHVSP_ID_MAX; u8IDidx++)
    {
        _DrvSclHvspInitSWVarialbe(u8IDidx,(MDrvSclCtxCmdqConfig_t *)pInitCfg->pvCtx);
        _Drv_HVSP_SetCoringThrdOn(u8IDidx);
    }
    DRV_HVSP_MUTEX_UNLOCK();
    _Drv_HVSP_SetHWinit();
    HalSclHvspMloadSramBufferPrepare();
    return TRUE;
}
u8 _DrvSclHvspGetScalingHRatioConfig(DrvSclHvspIdType_e enHVSP_ID,u8 bUp)
{
    u8 bret = 0;
    if(bUp)
    {
        bret = SRAMFORSCALUP;
    }
    else
    {
        bret = SRAMFORSCALDOWN;
    }
    return bret;
}
u8 _DrvSclHvspGetScalingVRatioConfig(DrvSclHvspIdType_e enHVSP_ID,u8 bUp)
{
    u8 bret = 0;
    if(bUp)
    {
        bret = SRAMFORSCALUP;
    }
    else
    {
        bret = SRAMFORSCALDOWN;
    }
    return bret;
}
void _DrvSclHvspSetHorizotnalScalingConfig(DrvSclHvspIdType_e enHVSP_ID,bool bEn)
{

    HalSclHvspSetScalingHoEn(enHVSP_ID, bEn);
    HalSclHvspSetScalingHoFacotr(enHVSP_ID, bEn ? gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_H: 0);
    //ToDo:Sram Mload
    HalSclHvspSetModeYHo(enHVSP_ID,bEn ? E_HAL_SCLHVSP_FILTER_MODE_SRAM_0: E_HAL_SCLHVSP_FILTER_MODE_BYPASS);
    HalSclHvspSetModeCHo(enHVSP_ID,bEn ? E_HAL_SCLHVSP_FILTER_MODE_BILINEAR: E_HAL_SCLHVSP_FILTER_MODE_BYPASS,E_HAL_SCLHVSP_SRAM_SEL_0);
    HalSclHvspSetHspDithEn(enHVSP_ID,bEn);
    HalSclHvspSetHspCoringEnC(enHVSP_ID,bEn);
    HalSclHvspSetHspCoringEnY(enHVSP_ID,bEn);
    HalSclHvspSramDumpbyMload(enHVSP_ID,gstGlobalHvspSet->gbHScalingup[enHVSP_ID],1,0);
}
void _DrvSclHvspSetHTbl(DrvSclHvspIdType_e enHVSP_ID)
{
    if(gstGlobalHvspSet->gbHScalingup[enHVSP_ID] &0x1)
    {
        gstGlobalHvspSet->gbHScalingup[enHVSP_ID] = SRAMFORSCALUP;
    }
    else
    {
        gstGlobalHvspSet->gbHScalingup[enHVSP_ID] = SRAMFORSCALDOWN;
    }
}
void _DrvSclHvspSetVTbl(DrvSclHvspIdType_e enHVSP_ID)
{
    if(gstGlobalHvspSet->gbVScalingup[enHVSP_ID] &0x1)
    {
        gstGlobalHvspSet->gbVScalingup[enHVSP_ID] = SRAMFORSCALUP;
    }
    else
    {
        gstGlobalHvspSet->gbVScalingup[enHVSP_ID] = SRAMFORSCALDOWN;
    }
}
void _DrvSclHvspSetHorizotnalSramTblbyMload(DrvSclHvspIdType_e enHVSP_ID)
{
    if((_CHANGE_SRAM_H_QMAP(enHVSP_ID,(gstGlobalHvspSet->gbHScalingup[enHVSP_ID]&0x1))))
    {
        gstGlobalHvspSet->gbHScalingup[enHVSP_ID] &= 0x1;
        gstGlobalHvspSet->gbHScalingup[enHVSP_ID] = (gstGlobalHvspSet->gbHScalingup[enHVSP_ID]^1);//XOR 1 :reverse
        _DrvSclHvspSetHTbl(enHVSP_ID);
    }
    else if(((_DrvSclHvspGetScalingHRatioConfig(enHVSP_ID,gstGlobalHvspSet->gbHScalingup[enHVSP_ID]&0x1))!=
        ((gstGlobalHvspSet->gbHScalingup[enHVSP_ID]&0xF0)>>4)))
    {
        _DrvSclHvspSetHTbl(enHVSP_ID);
    }
    HalSclHvspSramDumpbyMload(enHVSP_ID,gstGlobalHvspSet->gbHScalingup[enHVSP_ID],1,1);
}
void _DrvSclHvspSetVerticalSramTblbyMload(DrvSclHvspIdType_e enHVSP_ID)
{
    if((_CHANGE_SRAM_V_QMAP(enHVSP_ID,(gstGlobalHvspSet->gbVScalingup[enHVSP_ID]&0x1))))
    {
        gstGlobalHvspSet->gbVScalingup[enHVSP_ID] &= 0x1;
        gstGlobalHvspSet->gbVScalingup[enHVSP_ID] = (gstGlobalHvspSet->gbVScalingup[enHVSP_ID]^1);//XOR 1 :reverse
        _DrvSclHvspSetVTbl(enHVSP_ID);
    }
    else if(((_DrvSclHvspGetScalingVRatioConfig(enHVSP_ID,gstGlobalHvspSet->gbVScalingup[enHVSP_ID]&0x1))!=
        ((gstGlobalHvspSet->gbVScalingup[enHVSP_ID]&0xF0)>>4)))
    {
        _DrvSclHvspSetVTbl(enHVSP_ID);
    }
    HalSclHvspSramDumpbyMload(enHVSP_ID,gstGlobalHvspSet->gbVScalingup[enHVSP_ID],0,1);
}
void _DrvSclHvspSetVerticalScalingConfig(DrvSclHvspIdType_e enHVSP_ID,bool bEn)
{
    HalSclHvspSetScalingVeEn(enHVSP_ID, bEn);
    HalSclHvspSetScalingVeFactor(enHVSP_ID,  bEn ? gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_V: 0);
    HalSclHvspSetModeYVe(enHVSP_ID,bEn ? E_HAL_SCLHVSP_FILTER_MODE_SRAM_0: E_HAL_SCLHVSP_FILTER_MODE_BYPASS);
    HalSclHvspSetModeCVe(enHVSP_ID,bEn ? E_HAL_SCLHVSP_FILTER_MODE_BILINEAR: E_HAL_SCLHVSP_FILTER_MODE_BYPASS,E_HAL_SCLHVSP_SRAM_SEL_0);
    HalSclHvspSetVspDithEn(enHVSP_ID,bEn);
    HalSclHvspSetVspCoringEnC(enHVSP_ID,bEn);
    HalSclHvspSetVspCoringEnY(enHVSP_ID,bEn);
    HalSclHvspSramDumpbyMload(enHVSP_ID,gstGlobalHvspSet->gbVScalingup[enHVSP_ID],0,0);
}
void _DrvSclHvspSetCrop(bool u8CropID,DrvSclHvspScalingConfig_t *stCfg,HalSclHvspCropInfo_t *stCropInfo)
{
    //I3
    //u16In_hsize = (u8CropID ==DRV_HVSP_CROP_1) ? gstGlobalHvspSet->gstSrcSize.u16Width : gstGlobalHvspSet->gstIPMCfg.u16Fetch;
    //u16In_vsize = (u8CropID ==DRV_HVSP_CROP_1) ? gstGlobalHvspSet->gstSrcSize.u16Height : gstGlobalHvspSet->gstIPMCfg.u16Vsize;
    stCropInfo->bEn         = stCfg->bCropEn[u8CropID];
    if(u8CropID ==DRV_HVSP_CROP_1)
    {
        stCropInfo->u16In_hsize = stCfg->u16Src_Width;
        stCropInfo->u16In_vsize = stCfg->u16Src_Height;
    }
    else if(u8CropID ==DRV_HVSP_CROP_2)
    {
        stCropInfo->u16In_hsize = stCfg->u16Crop_Width[DRV_HVSP_CROP_1];
        stCropInfo->u16In_vsize = stCfg->u16Crop_Height[DRV_HVSP_CROP_1];
    }
    stCropInfo->u16Hsize    = stCfg->u16Crop_Width[u8CropID];
    stCropInfo->u16Vsize    = stCfg->u16Crop_Height[u8CropID];
    if(stCfg->u16Crop_Width[u8CropID]%2)
    {
        stCropInfo->u16Hsize      = stCfg->u16Crop_Width[u8CropID]-1;
    }
    if(u8CropID == DRV_HVSP_CROP_1)
    {
        gstGlobalHvspSet->gstIPMCfg.u16Fetch = stCropInfo->u16Hsize;
        gstGlobalHvspSet->gstIPMCfg.u16Vsize = stCropInfo->u16Vsize;
    }
    if(stCfg->u16Crop_X[u8CropID]%2)
    {
        stCropInfo->u16Hst      = stCfg->u16Crop_X[u8CropID]+1;
    }
    else
    {
        stCropInfo->u16Hst      = stCfg->u16Crop_X[u8CropID];
    }
    stCropInfo->u16Vst      = stCfg->u16Crop_Y[u8CropID];
}
bool DrvSclHvspSetScaling(DrvSclHvspIdType_e enHVSP_ID, DrvSclHvspScalingConfig_t *stCfg, DrvSclHvspClkConfig_t* stclk)
{
    HalSclHvspCropInfo_t stCropInfo_2;
    HalSclHvspCropInfo_t stCropInfo_1;
    u64 u64Temp;
    u64 u64Temp2;
    DrvSclOsMemset(&stCropInfo_1,0,sizeof(HalSclHvspCropInfo_t));
    DrvSclOsMemset(&stCropInfo_2,0,sizeof(HalSclHvspCropInfo_t));
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)stCfg->pvCtx);
    stCropInfo_1.bEn = 0;
    stCropInfo_2.bEn = 0;

    if(enHVSP_ID == E_DRV_SCLHVSP_ID_1)
    {


        SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[DRVHVSP]%s id:%d @:%lu\n",
            __FUNCTION__,enHVSP_ID,(u32)DrvSclOsGetSystemTime());
        // setup cmd trig config
        DRV_HVSP_MUTEX_LOCK();
        _DrvSclHvspSetCrop(DRV_HVSP_CROP_1,stCfg,&stCropInfo_1);
        _DrvSclHvspSetCrop(DRV_HVSP_CROP_2,stCfg,&stCropInfo_2);

        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width  = stCropInfo_2.u16Hsize;//stCfg->u16Src_Width;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height = stCropInfo_2.u16Vsize;//stCfg->u16Src_Height;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width  = stCfg->u16Dsp_Width;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height = stCfg->u16Dsp_Height;
        gstGlobalHvspSet->gstIPMCfg.u16Fetch = stCropInfo_2.u16In_hsize;
        gstGlobalHvspSet->gstIPMCfg.u16Vsize = stCropInfo_2.u16In_vsize;
        // Crop1
        HalSclHvspSetCropConfig(E_DRV_SCLHVSP_CROP_ID_1, &stCropInfo_1);
        // Crop2
        HalSclHvspSetCropConfig(E_DRV_SCLHVSP_CROP_ID_2, &stCropInfo_2);
        HalSclHvspSetIpmvSize(stCropInfo_2.u16In_vsize);
        HalSclHvspSetIpmLineOffset(stCropInfo_2.u16In_hsize);
        HalSclHvspSetIpmFetchNum(stCropInfo_2.u16In_hsize);
        HalSclHvspSetNeSampleStep(stCropInfo_2.u16In_hsize,stCropInfo_2.u16In_vsize);
        // NLM size
        HalSclHvspSetVipSize(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width,
                gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);
        HalSclHvspSetNlmLineBufferSize(stCropInfo_2.u16In_hsize,stCropInfo_2.u16In_vsize);
        HalSclHvspSetNlmEn(1);
        //HalSclHvspSetNlmRegionSize(&stCropInfo_1);
        //AIP size
        HalSclHvspSetXnrSize(stCropInfo_2.u16In_hsize,stCropInfo_2.u16In_vsize);
        HalSclHvspSetWdrLocalSize(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width,
                gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);
        HalSclHvspSetMXnrSize(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width,
                gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);
        HalSclHvspSetUVadjSize(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width,
                gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);
    }
    else
    {
        DRV_HVSP_MUTEX_LOCK();
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width      = stCfg->u16Src_Width;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height     = stCfg->u16Src_Height;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width   = stCfg->u16Dsp_Width;
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height  = stCfg->u16Dsp_Height;
    }
    if(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width>1 && (gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width>1))
    {
        u64Temp = (((u64)gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width) * 1048576);
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_H =
            (u32)CamOsMathDivU64(u64Temp, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width,&u64Temp2);
    }
    else
    {
        SCL_ERR("[DRVHVSP]%d Ratio Error\n",enHVSP_ID);
    }
    if(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height>1 && (gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height>1))
    {
        u64Temp = (((u64)gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height) * 1048576);
        gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_V =
            (u32)CamOsMathDivU64(u64Temp, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height,&u64Temp2);
    }
    else
    {
        SCL_ERR("[DRVHVSP]%d Ratio Error\n",enHVSP_ID);
    }
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(enHVSP_ID)),
        "[DRVHVSP]%s(%d):: HVSP_%d, AfterCrop(%d, %d)\n", __FUNCTION__, __LINE__,
        enHVSP_ID, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(enHVSP_ID)),
        "[DRVHVSP]%s(%d):: HVSP_%d, AfterScaling(%d, %d)\n", __FUNCTION__, __LINE__,
        enHVSP_ID, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height);
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(enHVSP_ID)),
        "[DRVHVSP]%s(%d):: HVSP_%d, Ratio(%lx, %lx)\n", __FUNCTION__, __LINE__,
        enHVSP_ID, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_H, gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].u32ScalingRatio_V);

    // horizotnal HVSP Scaling
    if(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width == gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width)
    {
        _DrvSclHvspSetHorizotnalScalingConfig(enHVSP_ID, FALSE);
    }
    else
    {
        _DrvSclHvspSetHorizotnalScalingConfig(enHVSP_ID, TRUE);
        //ToDo :MLOAD
        _DrvSclHvspSetHorizotnalSramTblbyMload(enHVSP_ID);
    }

    // vertical HVSP Scaling
    if(gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height == gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height)
    {
        _DrvSclHvspSetVerticalScalingConfig(enHVSP_ID, FALSE);
    }
    else
    {
        _DrvSclHvspSetVerticalScalingConfig(enHVSP_ID, TRUE);
        //ToDo :MLOAD
        _DrvSclHvspSetVerticalSramTblbyMload(enHVSP_ID);
    }

    // HVSP In size
    HalSclHvspSetHVSPInputSize(enHVSP_ID,
                                 gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Width,
                                 gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterCrop.u16Height);

    // HVSP Out size
    HalSclHvspSetHVSPOutputSize(enHVSP_ID,
                                  gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Width,
                                  gstGlobalHvspSet->gstScalinInfo[enHVSP_ID].stSizeAfterScaling.u16Height);
    DRV_HVSP_MUTEX_UNLOCK();
    stCfg->bRet = 1;
    return stCfg->bRet;
}
void DrvSclHvspSetInputSrcSize(ST_HVSP_SIZE_CONFIG *stSize)
{
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(0)), "[DRVHVSP]%s(%d)\n", __FUNCTION__,  __LINE__);
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)stSize->pvCtx);
    if(stSize->u16Height > 0)
    {
        gstGlobalHvspSet->gstSrcSize.u16Height    = stSize->u16Height;
    }
    if(stSize->u16Width > 0)
    {
        gstGlobalHvspSet->gstSrcSize.u16Width     = stSize->u16Width;
    }
    gstGlobalHvspSet->gstSrcSize.bSet = 1;
    HalSclHvspSetInputSrcSize(&gstGlobalHvspSet->gstSrcSize);
}
u32 DrvSclHvspGetInputSrcMux(DrvSclHvspIdType_e enID)
{
    return HalSclHvspGetInputSrcMux(enID);
}

bool DrvSclHvspSetInputMux(DrvSclHvspIpMuxType_e enIP,DrvSclHvspClkConfig_t* stclk,void *pvCtx)
{
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(0)), "[DRVHVSP]%s(%d): IP=%x\n", __FUNCTION__,  __LINE__,enIP);
    HalSclHvspSetInputMuxType(E_DRV_SCLHVSP_ID_1,enIP);
    if(enIP >= E_DRV_SCLHVSP_IP_MUX_MAX)
    {
        SCL_ERR("[DRVHVSP]%s(%d):: Wrong IP Type\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    return TRUE;
}
bool DrvSclHvspSetSc3InputMux(DrvSclHvspIpMuxType_e enIP, void *pvCtx)
{
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pvCtx);
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(0)), "[DRVHVSP]%s(%d): MuxType=%x\n", __FUNCTION__,  __LINE__,enIP);
    HalSclHvspSetInputMuxType(E_DRV_SCLHVSP_ID_3,enIP);
    HalSclHvspSetHwSc3InputMux(enIP,pvCtx);
    return TRUE;
}
bool DrvSclHvspSetRegisterForceByInst(u32 u32Reg, u8 u8Val, u8 u8Msk, void *pvCtx)
{
    HalSclHvspSetRegisterForceByInst(u32Reg, u8Val, u8Msk);
    return TRUE;
}

bool DrvSclHvspSetRegisterForce(u32 u32Reg, u8 u8Val, u8 u8Msk, void *pvCtx)
{
    HalSclHvspSetReg(u32Reg, u8Val, u8Msk);
    return TRUE;
}
void DrvSclHvspGetSclInts(void *pvCtx,DrvSclHvspScIntsType_t *sthvspints)
{
    DrvSclIrqScIntsType_t *stints;
    stints = DrvSclIrqGetSclInts();
    DrvSclOsMemcpy(sthvspints,stints,sizeof(DrvSclHvspScIntsType_t));
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
void DrvSclHvspSclIq(DrvSclHvspIdType_e enID,DrvSclHvspIqType_e enIQ,void *pvCtx)
{
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)pvCtx);
    switch(enIQ)
    {
        case E_DRV_SCLHVSP_IQ_H_Tbl0:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbHScalingup[enID] = ((gstGlobalHvspSet->gbHScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,(gstGlobalHvspSet->gbHScalingup[enID]),1,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_H_Tbl1:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbHScalingup[enID] = (0x10 |(gstGlobalHvspSet->gbHScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,(gstGlobalHvspSet->gbHScalingup[enID]),1,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_H_Tbl2:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbHScalingup[enID] = (0x20 |(gstGlobalHvspSet->gbHScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,(gstGlobalHvspSet->gbHScalingup[enID]),1,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_H_Tbl3:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbHScalingup[enID] = (0x30 |(gstGlobalHvspSet->gbHScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,(gstGlobalHvspSet->gbHScalingup[enID]),1,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_H_BYPASS:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_BYPASS);
        break;
        case E_DRV_SCLHVSP_IQ_H_BILINEAR:
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_BILINEAR);
        break;
        case E_DRV_SCLHVSP_IQ_V_Tbl0:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbVScalingup[enID] = ((gstGlobalHvspSet->gbVScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,(gstGlobalHvspSet->gbVScalingup[enID]),0,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_V_Tbl1:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbVScalingup[enID] = (0x10 |(gstGlobalHvspSet->gbVScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,gstGlobalHvspSet->gbVScalingup[enID],0,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_V_Tbl2:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbVScalingup[enID] = (0x20 |(gstGlobalHvspSet->gbVScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,gstGlobalHvspSet->gbVScalingup[enID],0,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_V_Tbl3:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            gstGlobalHvspSet->gbVScalingup[enID] = (0x30 |(gstGlobalHvspSet->gbVScalingup[enID]&0x1));
            HalSclHvspSramDumpbyMload(enID,gstGlobalHvspSet->gbVScalingup[enID],0,1); //level 1 :up 0:down
        break;
        case E_DRV_SCLHVSP_IQ_V_BYPASS:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_BYPASS);
        break;
        case E_DRV_SCLHVSP_IQ_V_BILINEAR:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_BILINEAR);
        break;
        default:
            HalSclHvspSetModeYVe(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
            HalSclHvspSetModeYHo(enID,E_HAL_SCLHVSP_FILTER_MODE_SRAM_0);
        break;
    }
}
#endif
bool DrvSclHvspGetSCLInform(DrvSclHvspIdType_e enID,DrvSclHvspScInformConfig_t *stInformCfg)
{
    stInformCfg->u16X               = HalSclHvspGetCrop2Xinfo();
    stInformCfg->u16Y               = HalSclHvspGetCrop2Yinfo();
    stInformCfg->u16Width           = HalSclHvspGetHvspOutputWidth(enID);
    stInformCfg->u16Height          = HalSclHvspGetHvspOutputHeight(enID);
    stInformCfg->u16crop2inWidth    = HalSclHvspGetCrop2InputWidth();
    stInformCfg->u16crop2inHeight   = HalSclHvspGetCrop2InputHeight();
    stInformCfg->u16crop2OutWidth   = HalSclHvspGetCrop2OutputWidth();
    stInformCfg->u16crop2OutHeight  = HalSclHvspGetCrop2OutputHeight();
    stInformCfg->bEn                = HalSclHvspGetCrop2En();
    return TRUE;
}
bool DrvSclHvspGetHvspAttribute(DrvSclHvspIdType_e enID,DrvSclHvspInformConfig_t *stInformCfg)
{
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)stInformCfg->pvCtx);
    stInformCfg->u16Width           = HalSclHvspGetHvspOutputWidth(enID);
    stInformCfg->u16Height          = HalSclHvspGetHvspOutputHeight(enID);
    stInformCfg->u16inWidth         = HalSclHvspGetHvspInputWidth(enID);
    stInformCfg->u16inHeight        = HalSclHvspGetHvspInputHeight(enID);
    stInformCfg->bEn                = HalSclHvspGetScalingFunctionStatus(enID);
    stInformCfg->bEn |= (gstGlobalHvspSet->gbVScalingup[enID]&0xF0);
    stInformCfg->bEn |= ((gstGlobalHvspSet->gbHScalingup[enID]&0xF0)<<2);
    return TRUE;
}
bool DrvSclHvspGetFrameBufferAttribute(DrvSclHvspIdType_e enID,DrvSclHvspIpmConfig_t *stInformCfg)
{
    bool bLDCorPrvCrop=0;
    _DrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t*)stInformCfg->pvCtx);
    stInformCfg->bYCMWrite = gstGlobalHvspSet->gstIPMCfg.bYCMWrite;
    stInformCfg->u16Fetch = gstGlobalHvspSet->gstIPMCfg.u16Fetch;
    stInformCfg->u16Vsize = gstGlobalHvspSet->gstIPMCfg.u16Vsize;
    stInformCfg->u32YCBaseAddr = gstGlobalHvspSet->gstIPMCfg.u32YCBaseAddr+0x20000000;
    stInformCfg->u32MemSize = gstGlobalHvspSet->gstIPMCfg.u32MemSize;
    stInformCfg->bYCMRead = gstGlobalHvspSet->gstIPMCfg.bYCMRead;
    return bLDCorPrvCrop;
}
bool DrvSclHvspSetPatTgen(bool bEn, DrvSclHvspPatTgenConfig_t *pCfg)
{
    u16 u16VSync_St, u16HSync_St;
    bool bRet = TRUE;
    DrvSclIrqSetPTGenStatus(bEn);
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(0)), "[DRVHVSP]%s(%d)\n", __FUNCTION__,  __LINE__);
    if(bEn)
    {
        u16VSync_St = 1;
        u16HSync_St = 0;
        if(pCfg)
        {
            if((u16VSync_St + pCfg->u16VSyncWidth + pCfg->u16VBackPorch + pCfg->u16VActive - 1 )<1125)
            {
                HalSclHvspSetPatTgVtt(0xFFFF); //scaling up need bigger Vtt, , using vtt of 1920x1080 for all timing
            }
            else
            {
                HalSclHvspSetPatTgVtt(0xFFFF); //rotate
            }
            HalSclHvspSetPatTgVsyncSt(u16VSync_St);
            HalSclHvspSetPatTgVsyncEnd(u16VSync_St + pCfg->u16VSyncWidth - 1);
            HalSclHvspSetPatTgVdeSt(u16VSync_St + pCfg->u16VSyncWidth + pCfg->u16VBackPorch);
            HalSclHvspSetPatTgVdeEnd(u16VSync_St + pCfg->u16VSyncWidth + pCfg->u16VBackPorch + pCfg->u16VActive - 1);
            HalSclHvspSetPatTgVfdeSt(u16VSync_St + pCfg->u16VSyncWidth + pCfg->u16VBackPorch);
            HalSclHvspSetPatTgVfdeEnd(u16VSync_St + pCfg->u16VSyncWidth + pCfg->u16VBackPorch + pCfg->u16VActive - 1);

            HalSclHvspSetPatTgHtt(0xFFFF); // scaling up need bigger Vtt, , using vtt of 1920x1080 for all timing
            HalSclHvspSetPatTgHsyncSt(u16HSync_St);
            HalSclHvspSetPatTgHsyncEnd(u16HSync_St + pCfg->u16HSyncWidth - 1);
            HalSclHvspSetPatTgHdeSt(u16HSync_St + pCfg->u16HSyncWidth + pCfg->u16HBackPorch);
            HalSclHvspSetPatTgHdeEnd(u16HSync_St + pCfg->u16HSyncWidth + pCfg->u16HBackPorch + pCfg->u16HActive - 1);
            HalSclHvspSetPatTgHfdeSt(u16HSync_St + pCfg->u16HSyncWidth + pCfg->u16HBackPorch);
            HalSclHvspSetPatTgHfdeEnd(u16HSync_St + pCfg->u16HSyncWidth + pCfg->u16HBackPorch + pCfg->u16HActive - 1);

            HalSclHvspSetPatTgEn(TRUE);
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        HalSclHvspSetPatTgEn(FALSE);
        bRet = TRUE;
    }
    return bRet;
}
void DrvSclHvspGetCrop12Inform(DrvSclHvspInputInformConfig_t *stInformCfg)
{
    stInformCfg->bEn = HalSclHvspGetCrop1En();
    stInformCfg->u16inWidth         = HalSclHvspGetCrop1Width();
    stInformCfg->u16inHeight        = HalSclHvspGetCrop1Height();
    stInformCfg->u16inCropWidth         = HalSclHvspGetCrop2InputWidth();
    stInformCfg->u16inCropHeight        = HalSclHvspGetCrop2InputHeight();
    stInformCfg->u16inCropX         = HalSclHvspGetCropX();
    stInformCfg->u16inCropY        = HalSclHvspGetCropY();
    stInformCfg->u16inWidthcount    = HalSclHvspGetCrop1WidthCount();
    if(stInformCfg->u16inWidthcount)
    {
        stInformCfg->u16inWidthcount++;
    }
    stInformCfg->u16inHeightcount   = HalSclHvspGetCrop1HeightCount();
    stInformCfg->enMux              = HalSclHvspGetInputSrcMux(E_DRV_SCLHVSP_ID_1);
    stInformCfg->enSc3Mux           = HalSclHvspGetInputSrcMux(E_DRV_SCLHVSP_ID_3);
}
bool DrvSclHvspVtrackSetPayloadData(u16 u16Timecode, u8 u8OperatorID)
{
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s \n", __FUNCTION__);
    HalSclHvspVtrackSetPayloadData(u16Timecode, u8OperatorID);
    return 1;
}
bool DrvSclHvspVtrackSetKey(bool bUserDefinded, u8 *pu8Setting)
{
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s \n", __FUNCTION__);
    HalSclHvspVtrackSetKey(bUserDefinded, pu8Setting);
    return 1;
}
bool DrvSclHvspVtrackSetUserDefindedSetting(bool bUserDefinded, u8 *pu8Setting)
{
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s \n", __FUNCTION__);
    HalSclHvspVtrackSetUserDefindedSetting(bUserDefinded, pu8Setting);
    return 1;
}
bool DrvSclHvspVtrackEnable( u8 u8FrameRate, DrvSclHvspVtrackEnableType_e bEnable)
{
    SCL_DBG(SCL_DBG_LV_DRVVIP(), "[DRVVIP]%s \n", __FUNCTION__);
    HalSclHvspVtrackEnable(u8FrameRate, bEnable);
    return 1;
}

#undef DRV_HVSP_C
