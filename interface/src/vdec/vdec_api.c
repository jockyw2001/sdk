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
#include <stdlib.h>

#include "mi_syscall.h"
#include "mi_print.h"

#include "mi_vdec.h"
#include "vdec_ioctl.h"
#include "mi_sys.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
MI_MODULE_DEFINE(vdec)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define FRAMING_BUF_MAX_SIZE (512 * 1024)
#define MI_VDEC_MAX_CHN_NUM (33)

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_VDEC_FramingParams_s
{
    MI_U8 *pu8FramingBuf;
    MI_U32 u32RemainSize;
    MI_U32 u32FramingBufSize;
    MI_BOOL bStart;
    MI_VDEC_CodecType_e eCodecType;
    MI_VDEC_VideoMode_e eVideoMode;
} MI_VDEC_FramingParams_t;


//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
static MI_VDEC_FramingParams_t _astFramingParams[MI_VDEC_MAX_CHN_NUM];

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 MI_VDEC_CreateChn(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_CreateChn_t stCreateChn;

    memset(&stCreateChn, 0, sizeof(MI_VDEC_CreateChn_t));
    stCreateChn.VdecChn = VdecChn;
    memcpy(&stCreateChn.stChnAttr, pstChnAttr, sizeof(MI_VDEC_ChnAttr_t));

    s32Ret = MI_SYSCALL(MI_VDEC_CREATE_CHN, &stCreateChn);

    _astFramingParams[VdecChn].eVideoMode = pstChnAttr->eVideoMode;
    if ((MI_SUCCESS == s32Ret) && (E_MI_VDEC_VIDEO_MODE_STREAM == _astFramingParams[VdecChn].eVideoMode))
    {
        _astFramingParams[VdecChn].u32FramingBufSize = FRAMING_BUF_MAX_SIZE;
        if (!_astFramingParams[VdecChn].pu8FramingBuf)
        {
            _astFramingParams[VdecChn].pu8FramingBuf = (MI_U8 *)malloc(_astFramingParams[VdecChn].u32FramingBufSize);
            _astFramingParams[VdecChn].eCodecType = pstChnAttr->eCodecType;
        }
    }

    return s32Ret;
}

MI_S32 MI_VDEC_DestroyChn(MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_VDEC_DESTROY_CHN, &VdecChn);

    return s32Ret;
}

MI_S32 MI_VDEC_GetChnAttr(MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstChnAttr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_CreateChn_t stCreateChn;

    memset(&stCreateChn, 0, sizeof(MI_VDEC_CreateChn_t));
    stCreateChn.VdecChn = VdecChn;

    s32Ret = MI_SYSCALL(MI_VDEC_GET_CHN_ATTR, &stCreateChn);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstChnAttr, &stCreateChn.stChnAttr, sizeof(MI_VDEC_ChnAttr_t));
    }
    return s32Ret;
}

MI_S32 MI_VDEC_StartChn(MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_SYSCALL(MI_VDEC_START_CHN, &VdecChn);
    if (MI_SUCCESS == s32Ret)
    {
        _astFramingParams[VdecChn].bStart = TRUE;
        _astFramingParams[VdecChn].u32RemainSize = 0;
    }
    return s32Ret;
}

MI_S32 MI_VDEC_StopChn(MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_SYSCALL(MI_VDEC_STOP_CHN, &VdecChn);
    if (MI_SUCCESS == s32Ret)
    {
        _astFramingParams[VdecChn].bStart = FALSE;
    }
    return s32Ret;
}

MI_S32 MI_VDEC_GetChnStat(MI_VDEC_CHN VdecChn, MI_VDEC_ChnStat_t *pstChnStat)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_GetChnStat_t stGetChnStat;

    memset(&stGetChnStat, 0, sizeof(MI_VDEC_GetChnStat_t));
    stGetChnStat.VdecChn = VdecChn;

    s32Ret = MI_SYSCALL(MI_VDEC_GET_CHN_STAT, &stGetChnStat);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstChnStat, &stGetChnStat.stChnStat, sizeof(MI_VDEC_ChnStat_t));
    }
    return s32Ret;
}

MI_S32 MI_VDEC_ResetChn(MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_SYSCALL(MI_VDEC_RESET_CHN, &VdecChn);
    return s32Ret;
}

MI_S32 MI_VDEC_SetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_GetChnParam_t stGetChnParam;

    memset(&stGetChnParam, 0, sizeof(MI_VDEC_GetChnParam_t));
    stGetChnParam.VdecChn = VdecChn;

    memcpy(&stGetChnParam.stChnParam, pstChnParam, sizeof(MI_VDEC_ChnParam_t));
    s32Ret = MI_SYSCALL(MI_VDEC_SET_CHN_PARAM, &stGetChnParam);
    return s32Ret;
}

MI_S32 MI_VDEC_GetChnParam(MI_VDEC_CHN VdecChn, MI_VDEC_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_GetChnParam_t stGetChnParam;

    memset(&stGetChnParam, 0, sizeof(MI_VDEC_GetChnParam_t));
    stGetChnParam.VdecChn = VdecChn;

    s32Ret = MI_SYSCALL(MI_VDEC_GET_CHN_PARAM, &stGetChnParam);
    if (MI_SUCCESS == s32Ret)
    {
        memcpy(pstChnParam, &stGetChnParam.stChnParam, sizeof(MI_VDEC_ChnParam_t));
    }
    return s32Ret;
}

static MI_S32 _MI_VDEC_SendFrame(MI_VDEC_CHN VdecChn, MI_VDEC_VideoStream_t *pstVideoStream, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hBufHandle = MI_HANDLE_NULL;

    stChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = VdecChn;
    stChnPort.u32PortId = 0;

    stBufCfg.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;
    stBufCfg.stRawCfg.u32Size = pstVideoStream->u32Len;

    s32Ret = MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufCfg, &stBufInfo, &hBufHandle, s32MilliSec);
    if (MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    if (stBufInfo.stRawData.u32BufSize < stBufCfg.stRawCfg.u32Size)
    {
        MI_SYS_ChnInputPortPutBuf(hBufHandle, &stBufInfo, TRUE);
        DBG_ERR("Get Buffer Error, Size Error\n");
        return E_MI_ERR_FAILED;
    }

    stBufInfo.u64Pts = pstVideoStream->u64PTS;
    stBufInfo.stRawData.u32ContentSize = pstVideoStream->u32Len;
    memcpy(stBufInfo.stRawData.pVirAddr, pstVideoStream->pu8Addr, pstVideoStream->u32Len);

    s32Ret = MI_SYS_ChnInputPortPutBuf(hBufHandle, &stBufInfo, FALSE);
    return s32Ret;
}

static void _MI_VDEC_ForceToPanic(void)
{
    while (TRUE)
    {
        DBG_ERR("Need To Panic\n");
    }
}

MI_S32 MI_VDEC_SendStream(MI_VDEC_CHN VdecChn, MI_VDEC_VideoStream_t *pstVideoStream, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    if (!pstVideoStream)
    {
        return s32Ret;
    }

    if (!pstVideoStream->u32Len)
    {
        return s32Ret;
    }

    if (   (_astFramingParams[VdecChn].pu8FramingBuf)
        && (E_MI_VDEC_VIDEO_MODE_STREAM == _astFramingParams[VdecChn].eVideoMode))
    {
        ///stream mode
        MI_U32 u32FrmBound = 0;
        MI_U32 u32SendByte = 0;
        MI_U8 *pu8CurPos = NULL;
        MI_VDEC_VideoStream_t stVideoStream;
        MI_S64 s64RemainLen = 0;
        MI_U8 bBoundAlignStart = FALSE;

        if (_astFramingParams[VdecChn].u32RemainSize)
        {
            MI_U32 u32CopyLen = 0;
            u32CopyLen = _astFramingParams[VdecChn].u32FramingBufSize - _astFramingParams[VdecChn].u32RemainSize;
            if (!u32CopyLen)
            {
                _MI_VDEC_ForceToPanic();
            }

            u32CopyLen = (u32CopyLen > pstVideoStream->u32Len) ? pstVideoStream->u32Len : u32CopyLen;
            memcpy(_astFramingParams[VdecChn].pu8FramingBuf + _astFramingParams[VdecChn].u32RemainSize, pstVideoStream->pu8Addr, u32CopyLen);
            pu8CurPos = _astFramingParams[VdecChn].pu8FramingBuf;
        }

        s64RemainLen = pstVideoStream->u32Len + _astFramingParams[VdecChn].u32RemainSize;
        while (s64RemainLen > 0)
        {
            if (!_astFramingParams[VdecChn].bStart)
            {
                ///chn close
                _astFramingParams[VdecChn].u32RemainSize = 0;
                return s32Ret;
            }

            if (u32SendByte >= _astFramingParams[VdecChn].u32RemainSize)
            {
                pu8CurPos = pstVideoStream->pu8Addr + (u32SendByte - _astFramingParams[VdecChn].u32RemainSize);
            }

            u32FrmBound = get_next_frame_start_position(pu8CurPos, (unsigned int)s64RemainLen, _astFramingParams[VdecChn].eCodecType, &bBoundAlignStart);
            if (u32FrmBound)
            {
                stVideoStream.u64PTS = pstVideoStream->u64PTS;
                stVideoStream.bEndOfFrame = TRUE;
                stVideoStream.bEndOfStream = TRUE;
                stVideoStream.pu8Addr = pu8CurPos;
                stVideoStream.u32Len = u32FrmBound;
                if (bBoundAlignStart)
                {
                    s32Ret = _MI_VDEC_SendFrame(VdecChn, &stVideoStream, s32MilliSec);
                }
                else
                {
                    DBG_ERR("Chn(%d), Start Pos Not Aligned Bound, Drop\n", VdecChn, _astFramingParams[VdecChn].eCodecType);
                }

                if ((MI_SUCCESS == s32Ret) || (!bBoundAlignStart))
                {
                    pu8CurPos += u32FrmBound;
                    u32SendByte += u32FrmBound;
                    s64RemainLen -= u32FrmBound;
                    continue;
                }
            }


            if (s64RemainLen >= _astFramingParams[VdecChn].u32FramingBufSize)
            {
                if (!u32FrmBound)
                {
                    DBG_ERR("Framing Buffer Small, And Frame UnFound, Need To Panic\n");
                    _MI_VDEC_ForceToPanic();
                }

                DBG_WRN("Framing Buffer Small, Need To Try Send Frame\n");
                s32MilliSec = 30;
                continue;
            }

            ///goto return;
            if (s64RemainLen <= 0)
            {
                return s32Ret;
            }

            if (u32SendByte >= _astFramingParams[VdecChn].u32RemainSize)
            {
                ///copy from pstVideoStream->pu8Addr
                memcpy(_astFramingParams[VdecChn].pu8FramingBuf, pu8CurPos, s64RemainLen);
                _astFramingParams[VdecChn].u32RemainSize = s64RemainLen;
            }
            else
            {
                ///copy from _astFramingParams
                MI_U8 *pu8TmpBuf = NULL;
                MI_U32 u32FrmingBufLen = 0;

                pu8TmpBuf = (MI_U8 *)malloc(s64RemainLen);
                if (!pu8TmpBuf)
                {
                    DBG_ERR("Buffer Malloc Error\n");
                    return s32Ret;
                }

                u32FrmingBufLen = _astFramingParams[VdecChn].u32RemainSize - u32SendByte;
                memcpy(pu8TmpBuf, pu8CurPos, u32FrmingBufLen);
                memcpy(pu8TmpBuf + u32FrmingBufLen, pstVideoStream->pu8Addr, s64RemainLen - u32FrmingBufLen);
                memcpy(_astFramingParams[VdecChn].pu8FramingBuf, pu8TmpBuf, s64RemainLen);
                _astFramingParams[VdecChn].u32RemainSize = s64RemainLen;
                free(pu8TmpBuf);
                pu8TmpBuf = NULL;
            }
            return MI_SUCCESS;
        }
    }
    else
    {
        ///frame mode
        s32Ret = _MI_VDEC_SendFrame(VdecChn, pstVideoStream, s32MilliSec);
    }

    return s32Ret;
}

MI_S32 MI_VDEC_GetUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    static MI_VDEC_GetUserData_t stGetUserData;

    memset(&stGetUserData, 0, sizeof(MI_VDEC_GetUserData_t));
    stGetUserData.VdecChn = VdecChn;
    stGetUserData.u16DataLen = sizeof(stGetUserData.au8UserData);
    stGetUserData.s32MilliSec = s32MilliSec;

    s32Ret = MI_SYSCALL(MI_VDEC_GET_USER_DATA, &stGetUserData);
    if ((MI_SUCCESS == s32Ret) && (stGetUserData.u16DataLen > 0))
    {
        pstUserData->pu8Addr = stGetUserData.au8UserData;
        pstUserData->u32Len = stGetUserData.u16DataLen;
        pstUserData->bValid = stGetUserData.bValid;
    }
    else
    {
        pstUserData->pu8Addr = NULL;
        pstUserData->bValid = 0;
        pstUserData->u32Len = 0;
    }
    return s32Ret;
}

MI_S32 MI_VDEC_ReleaseUserData(MI_VDEC_CHN VdecChn, MI_VDEC_UserData_t *pstUserData)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 MI_VDEC_SetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e eDisplayMode)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_GetDisplayMode_t stGetDisplayMode;

    memset(&stGetDisplayMode, 0, sizeof(MI_VDEC_GetDisplayMode_t));
    stGetDisplayMode.VdecChn = VdecChn;
    stGetDisplayMode.eDisplayMode = eDisplayMode;

    s32Ret = MI_SYSCALL(MI_VDEC_SET_DISPLAY_MODE, &stGetDisplayMode);
    return s32Ret;
}

MI_S32 MI_VDEC_GetDisplayMode(MI_VDEC_CHN VdecChn, MI_VDEC_DisplayMode_e *peDisplayMode)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_VDEC_GetDisplayMode_t stGetDisplayMode;

    memset(&stGetDisplayMode, 0, sizeof(MI_VDEC_GetDisplayMode_t));
    stGetDisplayMode.VdecChn = VdecChn;

    s32Ret = MI_SYSCALL(MI_VDEC_GET_DISPLAY_MODE, &stGetDisplayMode);
    if (MI_SUCCESS == s32Ret)
    {
        *peDisplayMode = stGetDisplayMode.eDisplayMode;
    }
    return s32Ret;
}

EXPORT_SYMBOL(MI_VDEC_CreateChn);
EXPORT_SYMBOL(MI_VDEC_DestroyChn);
EXPORT_SYMBOL(MI_VDEC_GetChnAttr);
EXPORT_SYMBOL(MI_VDEC_StartChn);
EXPORT_SYMBOL(MI_VDEC_StopChn);
EXPORT_SYMBOL(MI_VDEC_GetChnStat);
EXPORT_SYMBOL(MI_VDEC_ResetChn);
EXPORT_SYMBOL(MI_VDEC_SetChnParam);
EXPORT_SYMBOL(MI_VDEC_GetChnParam);
EXPORT_SYMBOL(MI_VDEC_SendStream);
EXPORT_SYMBOL(MI_VDEC_GetUserData);
EXPORT_SYMBOL(MI_VDEC_ReleaseUserData);
EXPORT_SYMBOL(MI_VDEC_SetDisplayMode);
EXPORT_SYMBOL(MI_VDEC_GetDisplayMode);
