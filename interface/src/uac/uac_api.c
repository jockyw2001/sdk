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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   vdec_api.c
/// @brief vdec module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL__
#include <unistd.h>
#include <stdio.h>
#endif
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_uac.h"
#include "uac_ioctl.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

MI_MODULE_DEFINE(uac)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

MI_S32 MI_UAC_Init(void)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL_VOID(MI_UAC_INIT);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_Init);

MI_S32 MI_UAC_Exit(void)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL_VOID(MI_UAC_EXIT);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_Exit);

MI_S32 MI_UAC_OpenDevice(MI_UAC_DEV DevId)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL(MI_UAC_OPEN_DEVICE, &DevId);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_OpenDevice);

MI_S32 MI_UAC_CloseDevice(MI_UAC_DEV DevId)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL(MI_UAC_CLOSE_DEVICE, &DevId);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_CloseDevice);

MI_S32 MI_UAC_GetCapturePortAttr(MI_UAC_DEV DevId,MI_UAC_PORT PortId,MI_UAC_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret;
    MI_UAC_GetInputPortAttr_t stGetInputPortAttr;

    memset(&stGetInputPortAttr, 0, sizeof(stGetInputPortAttr));
    stGetInputPortAttr.DevId=DevId;
    stGetInputPortAttr.PortId=PortId;
    s32Ret = MI_SYSCALL(MI_UAC_GET_CAPTURE_PORT_ATTR, &stGetInputPortAttr);
    if (s32Ret == 0)
    {
        memcpy(pstInputPortAttr, &stGetInputPortAttr.stInputPortAttr, sizeof(*pstInputPortAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_GetCapturePortAttr);

MI_S32 MI_UAC_GetPlaybackPortAttr(MI_UAC_DEV DevId,MI_UAC_PORT PortId,MI_UAC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 s32Ret;
    MI_UAC_GetOutputPortAttr_t stGetOutputPortAttr;

    memset(&stGetOutputPortAttr, 0, sizeof(stGetOutputPortAttr));
    stGetOutputPortAttr.DevId=DevId;
    stGetOutputPortAttr.PortId=PortId;
    s32Ret = MI_SYSCALL(MI_UAC_GET_PLAYBACK_PORT_ATTR, &stGetOutputPortAttr);
    if (s32Ret == 0)
    {
        memcpy(pstOutputPortAttr, &stGetOutputPortAttr.stOutputPortAttr, sizeof(*pstOutputPortAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_GetPlaybackPortAttr);

MI_S32 MI_UAC_StartDev(MI_UAC_DEV DevId)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL(MI_UAC_START_DEV, &DevId);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_StartDev);

MI_S32 MI_UAC_StopDev(MI_UAC_DEV DevId)
{
    MI_S32 s32Ret;

    s32Ret = MI_SYSCALL(MI_UAC_STOP_DEV, &DevId);

    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_StopDev);

MI_S32 MI_UAC_SendFrame(MI_UAC_CHN UacChn, MI_UAC_Frame_t *pstData, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hBufHandle = MI_HANDLE_NULL;

	if(!UAC_VALID_CHNID(UacChn)){
        s32Ret = MI_UAC_ERR_INVALID_CHNID;
        goto exit;
    }

    if(NULL==pstData->pu8Addr || 0==pstData->u32Len){
        s32Ret = MI_UAC_ERR_ILLEGAL_PARAM;
        goto exit;
    }

    memset(&stBufInfo,0x00,sizeof(MI_SYS_BufInfo_t));

    stChnPort.eModId = E_MI_MODULE_ID_UAC;
    stChnPort.u32DevId = UAC_CAPTURE_DEV;
    stChnPort.u32ChnId = UacChn;
    stChnPort.u32PortId = 0;

    stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufConf.u32Flags = 0;
    stBufConf.u64TargetPts = MI_SYS_INVALID_PTS;
    stBufConf.stRawCfg.u32Size = pstData->u32Len;

    s32Ret = MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf, &stBufInfo, &hBufHandle , s32MilliSec);
    if(MI_SUCCESS!=s32Ret){
        goto exit;
    }

    if(stBufInfo.stRawData.u32BufSize < stBufConf.stRawCfg.u32Size){
        DBG_ERR("Get Buffer Error, Size Error\n");
        s32Ret = E_MI_ERR_FAILED;
        goto exit_buf;
    }

    stBufInfo.u64Pts = pstData->u64PTS;
    stBufInfo.stRawData.u32ContentSize = pstData->u32Len;
    memcpy(stBufInfo.stRawData.pVirAddr, pstData->pu8Addr, pstData->u32Len);

    s32Ret = MI_SUCCESS;
exit_buf:
    if(0 > MI_SYS_ChnInputPortPutBuf(hBufHandle, &stBufInfo, false))
          s32Ret = E_MI_ERR_FAILED;
exit:
    return s32Ret;
}
EXPORT_SYMBOL(MI_UAC_SendFrame);
