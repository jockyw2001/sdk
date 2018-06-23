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
/// @file   venc_api.c
/// @brief venc module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "venc_ioctl.h"
#include "mi_venc.h"

//#include "camos_ossleep.h"
//#include "cam_os_wrapper.h"
//#include <linux/poll.h>
//#include <linux/signal.h>
//#include <linux/fs.h>
//#include <linux/fcntl.h>
//#include <sys/poll.h>
#ifndef __KERNEL__
#include <poll.h>
#endif
//#include <linux/time.h>

//FIXME could not find include
//typedef unsigned long int nfds_t;
//extern int poll (struct pollfd *__fds, nfds_t __nfds, int __timeout);
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

MI_MODULE_DEFINE(venc)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define MI_VENC_InitChnIoVar(localVar, VeChn, pInOutVar)\
    if (pInOutVar == NULL) return MI_ERR_VENC_NULL_PTR;\
    memset(&localVar, 0, sizeof(localVar));\
    localVar.VeChn = VeChn

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct _MI_VENC_UsrChnInfo_s
{
    MI_U32 u32DevId;
    MI_S32 s32Fd;
    MI_U32 u32MaxStrmCnt;
    MI_SYS_BUF_HANDLE hHandle;
} _MI_VENC_UsrChnInfo_t;

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static _MI_VENC_UsrChnInfo_t _astUsrChnInfo[VENC_MAX_CHN_NUM];

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 MI_VENC_GetChnDevid(MI_VENC_CHN VeChn, MI_U32 *pu32Devid)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetChnDevid_t stGetChnDevid;


    MI_VENC_InitChnIoVar(stGetChnDevid, VeChn, pu32Devid);
    *pu32Devid = (MI_U32)(-1);
    s32Ret = MI_SYSCALL(MI_VENC_GET_CHN_DEVID, &stGetChnDevid);
    if (s32Ret == MI_SUCCESS)
    {
        *pu32Devid = stGetChnDevid.u32DevId;
    }
    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetChnDevid);

MI_S32 MI_VENC_SetModParam(MI_VENC_ModParam_t *pstModParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_ModParam_t stSetModParam;

    memset(&stSetModParam, 0, sizeof(stSetModParam));
    //set &stSetModParam as MI_VENC_ModParam_t *pstModParam
    stSetModParam = *pstModParam;

    s32Ret = MI_SYSCALL(MI_VENC_SET_MOD_PARAM, &stSetModParam);
    if (s32Ret == MI_SUCCESS)
    {
        //memcpy(MI_VENC_ModParam_t *pstModParam);
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetModParam);

MI_S32 MI_VENC_GetModParam(MI_VENC_ModParam_t *pstModParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_ModParam_t stGetModParam;

    memset(&stGetModParam, 0, sizeof(stGetModParam));
    //set &stGetModParam as MI_VENC_ModParam_t *pstModParam
    //stGetModParam = *pstModParam;

    s32Ret = MI_SYSCALL(MI_VENC_GET_MOD_PARAM, &stGetModParam);
    if (s32Ret == MI_SUCCESS)
    {
        //memcpy(MI_VENC_ModParam_t *pstModParam);
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetModParam);

MI_S32 MI_VENC_CreateChn(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_CreateChn_t stCreateChn;

    MI_VENC_InitChnIoVar(stCreateChn, VeChn, pstAttr);
    stCreateChn.stAttr = *pstAttr;
    s32Ret = MI_SYSCALL(MI_VENC_CREATE_CHN, &stCreateChn);
    if(MI_SUCCESS == s32Ret)
    {
        _astUsrChnInfo[VeChn].u32MaxStrmCnt = 3;//TODO hard-coded, need to get from MI_SYS
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_CreateChn);

MI_S32 MI_VENC_DestroyChn(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    //MI_VENC_DestroyChn_t stDestroyChn;
    MI_VENC_CHN stDestroyChn;

    memset(&stDestroyChn, 0, sizeof(stDestroyChn));
    stDestroyChn = VeChn;

    s32Ret = MI_SYSCALL(MI_VENC_DESTROY_CHN, &stDestroyChn);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_DestroyChn);

MI_S32 MI_VENC_ResetChn(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    //MI_VENC_ResetChn_t stResetChn;
    MI_VENC_CHN stResetChn;

    memset(&stResetChn, 0, sizeof(stResetChn));
    stResetChn = VeChn;

    s32Ret = MI_SYSCALL(MI_VENC_RESET_CHN, &stResetChn);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_ResetChn);

MI_S32 MI_VENC_StartRecvPic(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    //MI_VENC_StartRecvPic_t stStartRecvPic;
    MI_VENC_CHN stStartRecvPic;

    memset(&stStartRecvPic, 0, sizeof(stStartRecvPic));
    stStartRecvPic = VeChn;

    s32Ret = MI_SYSCALL(MI_VENC_START_RECV_PIC, &stStartRecvPic);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_StartRecvPic);

MI_S32 MI_VENC_StartRecvPicEx(MI_VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_StartRecvPicEx_t stStartRecvPicEx;

    MI_VENC_InitChnIoVar(stStartRecvPicEx, VeChn, pstRecvParam);
    stStartRecvPicEx.stRecvParam = *pstRecvParam;
    s32Ret = MI_SYSCALL(MI_VENC_START_RECV_PIC_EX, &stStartRecvPicEx);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_StartRecvPicEx);

MI_S32 MI_VENC_StopRecvPic(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    //MI_VENC_StopRecvPic_t stStopRecvPic;
    MI_VENC_CHN stStopRecvPic;

    memset(&stStopRecvPic, 0, sizeof(stStopRecvPic));
    stStopRecvPic = VeChn;

    s32Ret = MI_SYSCALL(MI_VENC_STOP_RECV_PIC, &stStopRecvPic);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_StopRecvPic);

MI_S32 MI_VENC_Query(MI_VENC_CHN VeChn, MI_VENC_ChnStat_t *pstStat)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_Query_t stQuery;

    MI_VENC_InitChnIoVar(stQuery, VeChn, pstStat);
    stQuery.VeChn = VeChn;
    s32Ret = MI_SYSCALL(MI_VENC_QUERY, &stQuery);
    if (s32Ret == MI_SUCCESS)
    {
        *pstStat = stQuery.stStat;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_Query);

MI_S32 MI_VENC_SetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetChnAttr_t stSetChnAttr;

    MI_VENC_InitChnIoVar(stSetChnAttr, VeChn, pstAttr);
    stSetChnAttr.stAttr = *pstAttr;
    s32Ret = MI_SYSCALL(MI_VENC_SET_CHN_ATTR, &stSetChnAttr);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetChnAttr);

MI_S32 MI_VENC_GetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t*pstAttr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetChnAttr_t stGetChnAttr;

    MI_VENC_InitChnIoVar(stGetChnAttr, VeChn, pstAttr);
    s32Ret = MI_SYSCALL(MI_VENC_GET_CHN_ATTR, &stGetChnAttr);
    if (s32Ret == MI_SUCCESS)
    {
        *pstAttr = stGetChnAttr.stAttr;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetChnAttr);

#define MAX_PARSE_LEN (1500)//max 1K SEI in the SPEC.
#define SYNC_OFFSET (4) //fixed 00 00 00 01 in our solution
MI_S32 _MI_VENC_ParseNalu(MI_VENC_Stream_t *pstStream, MI_U8 *pData)
{
    MI_VENC_ModType_e eModType;
    MI_U32 u32ParseLen;
    MI_S32 s32Ret = MI_SUCCESS;

    if(pstStream == NULL || pData == NULL)
        return MI_ERR_VENC_NULL_PTR;

    //get hacked module type from kernel.
    eModType = (MI_VENC_ModType_e)pstStream->stH264Info.eRefType;
    pstStream->stH264Info.eRefType = 0;//reset it back from data hacking

    u32ParseLen = pstStream->pstPack[0].u32Len;
    if(u32ParseLen > MAX_PARSE_LEN)
        u32ParseLen = MAX_PARSE_LEN;

    if(eModType == E_MI_VENC_MODTYPE_H264E)
    {
        MI_U32 u32Code = (MI_U32)-1;
        MI_U8 u8NaluType;
        MI_U32 i;
        MI_U32 u32Idx = 0;
        MI_BOOL bFoundLastNalu = FALSE;
        MI_U32 u32Offset = 0, u32SyncOffset;

        if(u32ParseLen == 0)
        {
            pstStream->stH264Info.eRefType = E_MI_VENC_REF_TYPE_MAX;
            pstStream->pstPack[0].stDataType.eH264EType = E_MI_VENC_H264E_NALU_MAX;
            return MI_SUCCESS;
        }

        for (i = 0; (i < u32ParseLen) && (bFoundLastNalu == FALSE); ++i)
        {
            u32Code = (u32Code << 8) + pData[i];
            if((u32Code & 0xffffff00) == 0x100)
            {
                u8NaluType = u32Code & 0x1F;
                switch (u8NaluType)
                {
                    case E_MI_VENC_H264E_NALU_PSLICE://non-IDR
                        pstStream->stH264Info.eRefType = E_MI_VENC_BASE_P_REFBYBASE;
                        bFoundLastNalu = TRUE;
                        break;
                    case E_MI_VENC_H264E_NALU_SEI:
                    case E_MI_VENC_H264E_NALU_SPS: //SPS
                    case E_MI_VENC_H264E_NALU_PPS: //PPS
                    case E_MI_VENC_H264E_NALU_ISLICE: //IDR
                        if(u8NaluType == E_MI_VENC_H264E_NALU_ISLICE)
                        {
                            bFoundLastNalu = TRUE;
                        }
                        pstStream->stH264Info.eRefType = E_MI_VENC_BASE_IDR;
                        break;
                    case E_MI_VENC_H264E_NALU_IPSLICE:
                    default:
                        pstStream->stH264Info.eRefType = E_MI_VENC_REF_TYPE_MAX;
                        bFoundLastNalu = TRUE;
                        s32Ret = MI_ERR_VENC_UNDEFINED;//bad data
                        break;
                }
                if(u32Idx == 0)
                {
                    pstStream->pstPack[0].stDataType.eH264EType = u8NaluType;
                    pstStream->pstPack[0].u32Offset = u32Offset;
                    //do not change pstStream->pstPack[0].u32Len. It's the total packet length.
                }
                else
                {
                    if((pData[i - SYNC_OFFSET] == 0) && i > 3)
                    {
                        u32SyncOffset = 4;
                    }
                    else
                    {
                        u32SyncOffset = 3;
                    }
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].stPackType.eH264EType = u8NaluType;
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackOffset = i - u32SyncOffset;
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackLength =
                            pstStream->pstPack[0].u32Len - i + u32SyncOffset;
                    if(u32Idx >= 2)
                    {
                        pstStream->pstPack[0].asackInfo[u32Idx - 2].u32PackLength =
                            pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackOffset
                            - pstStream->pstPack[0].asackInfo[u32Idx - 2].u32PackOffset;
                    }
                }
                u32Idx++;
                u32Offset = i;
            }
        }
        pstStream->pstPack[0].u32DataNum = u32Idx - 1;
    }
    else if(eModType == E_MI_VENC_MODTYPE_H265E)
    {
        MI_U32 u32Code = (MI_U32)-1;
        MI_U8 u8NaluType;
        MI_U32 i;
        MI_U32 u32Idx = 0;
        MI_BOOL bFoundLastNalu = FALSE;
        MI_U32 u32Offset = 0, u32SyncOffset;

        if(u32ParseLen == 0)
        {
            pstStream->stH265Info.eRefType = E_MI_VENC_REF_TYPE_MAX;
            pstStream->pstPack[0].stDataType.eH265EType = E_MI_VENC_H265E_NALU_MAX;
            return MI_SUCCESS;
        }

        for (i = 0; (i < u32ParseLen) && (bFoundLastNalu == FALSE); ++i)
        {
            u32Code = (u32Code << 8) + pData[i];
            if((u32Code & 0xffffff00) == 0x100)
            {
                u8NaluType = (u32Code & 0x7E) >> 1;
                switch (u8NaluType)
                {
                    case E_MI_VENC_H265E_NALU_PSLICE://non-IDR
                        pstStream->stH265Info.eRefType = E_MI_VENC_BASE_P_REFBYBASE;
                        bFoundLastNalu = TRUE;
                        break;
                    case E_MI_VENC_H265E_NALU_SEI:
                    case E_MI_VENC_H265E_NALU_VPS:
                    case E_MI_VENC_H265E_NALU_SPS:
                    case E_MI_VENC_H265E_NALU_PPS:
                    case E_MI_VENC_H265E_NALU_ISLICE:
                        if(u8NaluType == E_MI_VENC_H265E_NALU_ISLICE)
                        {
                            bFoundLastNalu = TRUE;
                        }
                        pstStream->stH265Info.eRefType = E_MI_VENC_BASE_IDR;
                        break;
                    default:
                        pstStream->stH265Info.eRefType = E_MI_VENC_REF_TYPE_MAX;
                        bFoundLastNalu = TRUE;
                        s32Ret = MI_ERR_VENC_UNDEFINED;//bad data
                        break;
                }
                if(u32Idx == 0)
                {
                    pstStream->pstPack[0].stDataType.eH265EType = u8NaluType;
                    pstStream->pstPack[0].u32Offset = u32Offset;
                    //do not change pstStream->pstPack[0].u32Len. It's the total packet length.
                }
                else
                {
                    if((pData[i - SYNC_OFFSET] == 0) && i > 3)
                    {
                        u32SyncOffset = 4;
                    }
                    else
                    {
                        u32SyncOffset = 3;
                    }
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].stPackType.eH265EType = u8NaluType;
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackOffset = i - u32SyncOffset;
                    pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackLength =
                            pstStream->pstPack[0].u32Len - i + u32SyncOffset;
                    if(u32Idx >= 2)
                    {
                        pstStream->pstPack[0].asackInfo[u32Idx - 2].u32PackLength =
                            pstStream->pstPack[0].asackInfo[u32Idx - 1].u32PackOffset
                            - pstStream->pstPack[0].asackInfo[u32Idx - 2].u32PackOffset;
                    }
                }
                u32Idx++;
                u32Offset = i;
            }
        }
        pstStream->pstPack[0].u32DataNum = u32Idx - 1;
    }
    return s32Ret;
}

MI_S32 MI_VENC_GetStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec)
{
    MI_S32 s32Ret;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_SYS_BufInfo_t stBufInfo;
    //MI_S32 s32Fd = 0;
#if 0
    MI_VENC_Stream_t stStreamSpec3 = {
        .pstPack[0] =
        {
            .stDataType = 0,
            .u32DataNum = 3,
            .asackInfo[0].stDataType.eH264EType = E_MI_VENC_H264E_NALU_SPS,
            .asackInfo[1].stDataType.eH264EType = E_MI_VENC_H264E_NALU_PPS,
            .asackInfo[2].stDataType.eH264EType = E_MI_VENC_H264E_NALU_ISLICE,
        },
        .stH264Info.eRefType = E_MI_VENC_BASE_IDR,
    };
    MI_VENC_Stream_t stStreamI =
    {
        .pstPack[0] =
        {
            .stDataType = E_MI_VENC_H264E_NALU_SPS,
            .u32DataNum = 2,
            .asackInfo[0].stDataType.eH264EType = E_MI_VENC_H264E_NALU_PPS,
            .asackInfo[1].stDataType.eH264EType = E_MI_VENC_H264E_NALU_ISLICE,
        },
        .stH264Info.eRefType = E_MI_VENC_BASE_IDR,
    };
    MI_VENC_Stream_t stStreamIsei =
    {
        .pstPack[0] =
        {
            .stDataType = E_MI_VENC_H264E_NALU_SPS,
            .u32DataNum = 3,
            .asackInfo[0].stDataType.eH264EType = E_MI_VENC_H264E_NALU_PPS,
            .asackInfo[1].stDataType.eH264EType = E_MI_VENC_H264E_NALU_SEI,
            .asackInfo[2].stDataType.eH264EType = E_MI_VENC_H264E_NALU_ISLICE,
        },
        .stH264Info.eRefType = E_MI_VENC_BASE_IDR,
    };
    MI_VENC_Stream_t stStreamP =
    {
        .pstPack[0] =
        {
            .stDataType = E_MI_VENC_H264E_NALU_PSLICE,
            .u32DataNum = 0,
        },
        .stH264Info.eRefType = E_MI_VENC_BASE_P_REFBYBASE,
    };
#endif

    MI_VENC_GetStream_t stGetStream;

    if(pstStream == NULL)
        return MI_ERR_VENC_NULL_PTR;
    if(s32MilliSec < -1)
        return MI_ERR_VENC_ILLEGAL_PARAM;

    MI_VENC_InitChnIoVar(stGetStream, VeChn, pstStream);
    s32Ret = MI_SYSCALL(MI_VENC_GET_STREAM, &stGetStream);
    if (s32Ret != MI_SUCCESS)
    {
        return s32Ret;
    }
    s32Ret = MI_VENC_GetChnDevid(VeChn, &_astUsrChnInfo[VeChn].u32DevId);
    if(s32Ret != MI_SUCCESS)
    {
        return s32Ret;
    }

    if(pstStream->pstPack == NULL || pstStream->u32PackCount < 1)
    {
        pstStream->u32Seq = __LINE__;
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    memset(&stChnOutputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnOutputPort.eModId = E_MI_MODULE_ID_VENC;
    stChnOutputPort.u32DevId = _astUsrChnInfo[VeChn].u32DevId;
    stChnOutputPort.u32ChnId = VeChn;
    stChnOutputPort.u32PortId = 0;

    _astUsrChnInfo[VeChn].hHandle = MI_HANDLE_NULL;
    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    do
    {
        //fd_set read_fds;
        s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnOutputPort, &stBufInfo, &_astUsrChnInfo[VeChn].hHandle);
        if(MI_SUCCESS != s32Ret && MI_ERR_SYS_NOBUF != s32Ret)
        {
            pstStream->u32Seq = __LINE__ + _astUsrChnInfo[VeChn].u32DevId * 10000;
            return MI_ERR_VENC_BUF_EMPTY;
        }
        if(s32MilliSec == 0 && MI_ERR_SYS_NOBUF == s32Ret)
        {//non-blocked
            pstStream->u32Seq = __LINE__ + _astUsrChnInfo[VeChn].u32DevId * 10000;
            return MI_ERR_VENC_NOBUF;
        }

#if 0 //MI_SYS_GetFd
        s32Ret = MI_SYS_GetFd(&stChnOutputPort, &s32Fd);
        if(MI_SUCCESS != s32Ret)
            return s32Ret;
        {
            struct pollfd pfd[1] =
            {
                {s32Fd, POLLIN | POLLERR},
            };
            int iTimeOut, rval;

            if(s32MilliSec == -1)
                iTimeOut = 0x7FFFFFFF;
            else
                iTimeOut = s32MilliSec;

            rval = poll(pfd, 1, iTimeOut);

            if(rval <= 0)// time-out (0), or error ( < 0)
                return MI_ERR_VENC_BUF_EMPTY;
            if((pfd[0].revents & POLLIN) != POLLIN)//any error or not POLLIN
            {
                return MI_ERR_VENC_BUF_EMPTY;
            }

            //do it again
            s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnOutputPort, &stBufInfo, &_astUsrChnInfo[VeChn].hHandle);
        }
        usleep(1000);
        //FD_ZERO(&read_fds);
#else
        break;
#endif
    } while (0);

    if(MI_SUCCESS == s32Ret)
    {
        int i = 0;
        if(_astUsrChnInfo[VeChn].hHandle == MI_HANDLE_NULL)
        {
            pstStream->u32Seq = __LINE__;
            return MI_ERR_VENC_BUF_EMPTY;
        }
        if(stBufInfo.stRawData.pVirAddr == NULL)
            return MI_ERR_VENC_BADADDR;
        if(stBufInfo.stRawData.u32ContentSize >= stBufInfo.stRawData.u32BufSize)
            return MI_ERR_VENC_BUF_FULL;

        /* No need to flushCache in Userspace(after chgetoutputbuf), it will
        flushcache in chgetoutputbuf api. */
        //if(stBufInfo.stRawData.u32ContentSize > 0)
        //{//0 is invalid size for MI_SYS_FlushInvCache
           //MI_SYS_FlushInvCache(stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32BufSize);
        //}

        //anonymous union, copy 3 times
        pstStream->stJpegInfo = stGetStream.stStream.stJpegInfo;
        pstStream->stH264Info = stGetStream.stStream.stH264Info;
        pstStream->stH265Info = stGetStream.stStream.stH265Info;

        pstStream->u32Seq = (MI_U32)stBufInfo.stRawData.u64SeqNum;
        pstStream->u32PackCount = 1;
        pstStream->pstPack[i].bFrameEnd = 1;
        //pstStream->pstPack[i].stDataType.eH264EType = //TODO
        pstStream->pstPack[i].phyAddr = stBufInfo.stRawData.phyAddr;
        pstStream->pstPack[i].pu8Addr = stBufInfo.stRawData.pVirAddr;
        pstStream->pstPack[i].u32Offset = 0;
        pstStream->pstPack[i].u32Len = stBufInfo.stRawData.u32ContentSize;
        pstStream->pstPack[i].u64PTS = stBufInfo.u64Pts;
        pstStream->pstPack[i].u32DataNum = 0;
        if(stBufInfo.bEndOfStream)
            pstStream->pstPack[i].bFrameEnd |= 2;

        s32Ret = _MI_VENC_ParseNalu(pstStream, stBufInfo.stRawData.pVirAddr);
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetStream);

MI_S32 MI_VENC_ReleaseStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;

    if(VeChn < 0 || VeChn >= VENC_MAX_CHN_NUM)
        return MI_ERR_VENC_INVALID_CHNID;

    if(_astUsrChnInfo[VeChn].hHandle == MI_HANDLE_NULL)
        return MI_ERR_VENC_NULL_PTR;

    s32Ret = MI_SYS_ChnOutputPortPutBuf(_astUsrChnInfo[VeChn].hHandle);
    _astUsrChnInfo[VeChn].hHandle = MI_HANDLE_NULL;

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_ReleaseStream);

MI_S32 MI_VENC_InsertUserData(MI_VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_InsertUserData_t stInsertUserData;

    MI_VENC_InitChnIoVar(stInsertUserData, VeChn, pu8Data);
    stInsertUserData.u8Data = pu8Data;
    stInsertUserData.u32Len = u32Len;
    s32Ret = MI_SYSCALL(MI_VENC_INSERT_USER_DATA, &stInsertUserData);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_InsertUserData);

MI_S32 MI_VENC_SetMaxStreamCnt(MI_VENC_CHN VeChn, MI_U32 u32MaxStrmCnt)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;

    MI_U32 u32Devid;
    MI_SYS_ChnPort_t stSysChnPort;

    if(u32MaxStrmCnt <= 1)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    s32Ret = MI_VENC_GetChnDevid(VeChn, &u32Devid);
    if(s32Ret == MI_SUCCESS)
    {
        stSysChnPort.eModId = E_MI_MODULE_ID_VENC;
        stSysChnPort.u32DevId = u32Devid;
        stSysChnPort.u32ChnId = VeChn;
        stSysChnPort.u32PortId = 0;
        //This was set to (5, 10) and might be too big for kernel
        s32Ret = MI_SYS_SetChnOutputPortDepth(&stSysChnPort, u32MaxStrmCnt, u32MaxStrmCnt);
    }
    if(s32Ret == MI_SUCCESS)
    {
        _astUsrChnInfo[VeChn].u32MaxStrmCnt = u32MaxStrmCnt;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetMaxStreamCnt);

MI_S32 MI_VENC_GetMaxStreamCnt(MI_VENC_CHN VeChn, MI_U32 *pu32MaxStrmCnt)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    if(VeChn >= 0 && VeChn < VENC_MAX_CHN_NUM)
    {
        *pu32MaxStrmCnt = _astUsrChnInfo[VeChn].u32MaxStrmCnt;
        return MI_SUCCESS;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetMaxStreamCnt);

MI_S32 MI_VENC_RequestIdr(MI_VENC_CHN VeChn, MI_BOOL bInstant)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_RequestIdr_t stRequestIdr;

    memset(&stRequestIdr, 0, sizeof(stRequestIdr));
    stRequestIdr.VeChn = VeChn;
    stRequestIdr.bInstant = bInstant;
    s32Ret = MI_SYSCALL(MI_VENC_REQUEST_IDR, &stRequestIdr);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_RequestIdr);

MI_S32 MI_VENC_EnableIdr(MI_VENC_CHN VeChn, MI_BOOL bEnableIdr)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_EnableIdr_t stEnableIdr;

    memset(&stEnableIdr, 0, sizeof(stEnableIdr));
    stEnableIdr.VeChn = VeChn;
    stEnableIdr.bEnableIdr = bEnableIdr;
    s32Ret = MI_SYSCALL(MI_VENC_ENABLE_IDR, &stEnableIdr);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_EnableIdr);

MI_S32 MI_VENC_SetH264IdrPicId(MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264IdrPicId_t stSetH264IdrPicId;

    MI_VENC_InitChnIoVar(stSetH264IdrPicId, VeChn, pstH264eIdrPicIdCfg);
    stSetH264IdrPicId.stH264eIdrPicIdCfg = *pstH264eIdrPicIdCfg;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_IDR_PIC_ID, &stSetH264IdrPicId);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264IdrPicId);

MI_S32 MI_VENC_GetH264IdrPicId( MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg )
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264IdrPicId_t stGetH264IdrPicId;

    MI_VENC_InitChnIoVar(stGetH264IdrPicId, VeChn, pstH264eIdrPicIdCfg);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_IDR_PIC_ID, &stGetH264IdrPicId);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264eIdrPicIdCfg = stGetH264IdrPicId.stH264eIdrPicIdCfg;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264IdrPicId);

MI_S32 MI_VENC_GetFd(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_SYS_ChnPort_t stChnPort;
    MI_S32 s32Fd;
    //MI_VENC_GetFd_t stGetFd;

    memset(&stChnPort, 0, sizeof(stChnPort));
    stChnPort.eModId = E_MI_MODULE_ID_VENC;
    stChnPort.u32ChnId = VeChn;
    s32Ret = MI_VENC_GetChnDevid(VeChn, &stChnPort.u32DevId);
    if(s32Ret != MI_SUCCESS)
        return -1;

    s32Ret = MI_SYS_GetFd(&stChnPort , &s32Fd);
    if(s32Ret != MI_SUCCESS)
        return -2;
    _astUsrChnInfo[VeChn].s32Fd = s32Fd;
    return s32Fd;
}
EXPORT_SYMBOL(MI_VENC_GetFd);

MI_S32 MI_VENC_CloseFd(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    //MI_VENC_CloseFd_t stCloseFd;

    if(_astUsrChnInfo[VeChn].s32Fd <= 0)
        return -1;
    s32Ret = MI_SYS_CloseFd(_astUsrChnInfo[VeChn].s32Fd);
    _astUsrChnInfo[VeChn].s32Fd = -1;

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_CloseFd);

MI_S32 MI_VENC_SetRoiCfg(MI_VENC_CHN VeChn, MI_VENC_RoiCfg_t *pstVencRoiCfg)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRoiCfg_t stSetRoiCfg;

    MI_VENC_InitChnIoVar(stSetRoiCfg, VeChn, pstVencRoiCfg);
    stSetRoiCfg.stVencRoiCfg = *pstVencRoiCfg;
    s32Ret = MI_SYSCALL(MI_VENC_SET_ROI_CFG, &stSetRoiCfg);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetRoiCfg);

MI_S32 MI_VENC_GetRoiCfg(MI_VENC_CHN VeChn, MI_U32 u32Index, MI_VENC_RoiCfg_t *pstVencRoiCfg)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRoiCfg_t stGetRoiCfg;

    MI_VENC_InitChnIoVar(stGetRoiCfg, VeChn, pstVencRoiCfg);
    stGetRoiCfg.u32Index = u32Index;
    s32Ret = MI_SYSCALL(MI_VENC_GET_ROI_CFG, &stGetRoiCfg);
    if (s32Ret == MI_SUCCESS)
    {
        *pstVencRoiCfg = stGetRoiCfg.stVencRoiCfg;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetRoiCfg);

MI_S32 MI_VENC_SetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRoiBgFrameRate_t stSetRoiBgFrameRate;

    MI_VENC_InitChnIoVar(stSetRoiBgFrameRate, VeChn, pstRoiBgFrmRate);
    stSetRoiBgFrameRate.pstRoiBgFrmRate = *pstRoiBgFrmRate;
    s32Ret = MI_SYSCALL(MI_VENC_SET_ROI_BG_FRAME_RATE, &stSetRoiBgFrameRate);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetRoiBgFrameRate);

MI_S32 MI_VENC_GetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRoiBgFrameRate_t stGetRoiBgFrameRate;

    MI_VENC_InitChnIoVar(stGetRoiBgFrameRate, VeChn, pstRoiBgFrmRate);
    s32Ret = MI_SYSCALL(MI_VENC_GET_ROI_BG_FRAME_RATE, &stGetRoiBgFrameRate);
    if (s32Ret == MI_SUCCESS)
    {
        *pstRoiBgFrmRate = stGetRoiBgFrameRate.stRoiBgFrmRate;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetRoiBgFrameRate);

MI_S32 MI_VENC_SetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264SliceSplit_t stSetH264SliceSplit;

    MI_VENC_InitChnIoVar(stSetH264SliceSplit, VeChn, pstSliceSplit);
    stSetH264SliceSplit.stSliceSplit = *pstSliceSplit;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_SLICE_SPLIT, &stSetH264SliceSplit);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264SliceSplit);

MI_S32 MI_VENC_GetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264SliceSplit_t stGetH264SliceSplit;

    MI_VENC_InitChnIoVar(stGetH264SliceSplit, VeChn, pstSliceSplit);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_SLICE_SPLIT, &stGetH264SliceSplit);
    if (s32Ret == MI_SUCCESS)
    {
        *pstSliceSplit = stGetH264SliceSplit.stSliceSplit;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264SliceSplit);

MI_S32 MI_VENC_SetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264InterPred_t stSetH264InterPred;

    MI_VENC_InitChnIoVar(stSetH264InterPred, VeChn, pstH264InterPred);
    stSetH264InterPred.stH264InterPred = *pstH264InterPred;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_INTER_PRED, &stSetH264InterPred);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264InterPred);

MI_S32 MI_VENC_GetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264InterPred_t stGetH264InterPred;

    MI_VENC_InitChnIoVar(stGetH264InterPred, VeChn, pstH264InterPred);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_INTER_PRED, &stGetH264InterPred);
    if (s32Ret == MI_SUCCESS)
    {
        stGetH264InterPred.stH264InterPred = *pstH264InterPred;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264InterPred);

MI_S32 MI_VENC_SetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264IntraPred_t stSetH264IntraPred;

    MI_VENC_InitChnIoVar(stSetH264IntraPred, VeChn, pstH264IntraPred);
    stSetH264IntraPred.stH264IntraPred = *pstH264IntraPred;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_INTRA_PRED, &stSetH264IntraPred);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264IntraPred);

MI_S32 MI_VENC_GetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264IntraPred_t stGetH264IntraPred;

    MI_VENC_InitChnIoVar(stGetH264IntraPred, VeChn, pstH264IntraPred);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_INTRA_PRED, &stGetH264IntraPred);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264IntraPred = stGetH264IntraPred.stH264IntraPred;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264IntraPred);

MI_S32 MI_VENC_SetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Trans_t stSetH264Trans;

    if(pstH264Trans == NULL)
        return MI_ERR_VENC_NULL_PTR;

    if(pstH264Trans->u32InterTransMode != 0 ||
       pstH264Trans->u32IntraTransMode != 0)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    memset(&stSetH264Trans, 0, sizeof(stSetH264Trans));
    stSetH264Trans.VeChn = VeChn;
    stSetH264Trans.stH264Trans = *pstH264Trans;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_TRANS, &stSetH264Trans);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264Trans);

MI_S32 MI_VENC_GetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Trans_t stGetH264Trans;

    MI_VENC_InitChnIoVar(stGetH264Trans, VeChn, pstH264Trans);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_TRANS, &stGetH264Trans);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264Trans = stGetH264Trans.stH264Trans;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264Trans);

MI_S32 MI_VENC_SetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Entropy_t stSetH264Entropy;

    MI_VENC_InitChnIoVar(stSetH264Entropy, VeChn, pstH264EntropyEnc);
    stSetH264Entropy.stH264EntropyEnc = *pstH264EntropyEnc;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_ENTROPY, &stSetH264Entropy);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264Entropy);

MI_S32 MI_VENC_GetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Entropy_t stGetH264Entropy;

    MI_VENC_InitChnIoVar(stGetH264Entropy, VeChn, pstH264EntropyEnc);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_ENTROPY, &stGetH264Entropy);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264EntropyEnc = stGetH264Entropy.stH264EntropyEnc;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264Entropy);

MI_S32 MI_VENC_SetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Dblk_t stSetH264Dblk;

    MI_VENC_InitChnIoVar(stSetH264Dblk, VeChn, pstH264Dblk);
    stSetH264Dblk.stH264Dblk = *pstH264Dblk;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_DBLK, &stSetH264Dblk);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264Dblk);

MI_S32 MI_VENC_GetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Dblk_t stGetH264Dblk;

    MI_VENC_InitChnIoVar(stGetH264Dblk, VeChn, pstH264Dblk);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_DBLK, &stGetH264Dblk);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264Dblk = stGetH264Dblk.stH264Dblk;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264Dblk);

MI_S32 MI_VENC_SetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t*pstH264Vui)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH264Vui_t stSetH264Vui;

    MI_VENC_InitChnIoVar(stSetH264Vui, VeChn, pstH264Vui);
    stSetH264Vui.stH264Vui = *pstH264Vui;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H264_VUI, &stSetH264Vui);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH264Vui);

MI_S32 MI_VENC_GetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH264Vui_t stGetH264Vui;

    MI_VENC_InitChnIoVar(stGetH264Vui, VeChn, pstH264Vui);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H264_VUI, &stGetH264Vui);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH264Vui = stGetH264Vui.stH264Vui;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH264Vui);

MI_S32 MI_VENC_SetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265SliceSplit_t stSetH265SliceSplit;

    MI_VENC_InitChnIoVar(stSetH265SliceSplit, VeChn, pstSliceSplit);
    stSetH265SliceSplit.stSliceSplit = *pstSliceSplit;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_SLICE_SPLIT, &stSetH265SliceSplit);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265SliceSplit);

MI_S32 MI_VENC_GetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265SliceSplit_t stGetH265SliceSplit;

    MI_VENC_InitChnIoVar(stGetH265SliceSplit, VeChn, pstSliceSplit);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_SLICE_SPLIT, &stGetH265SliceSplit);
    if (s32Ret == MI_SUCCESS)
    {
        *pstSliceSplit = stGetH265SliceSplit.stSliceSplit;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265SliceSplit);

MI_S32 MI_VENC_SetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265InterPred_t stSetH265InterPred;

    MI_VENC_InitChnIoVar(stSetH265InterPred, VeChn, pstH265InterPred);
    stSetH265InterPred.stH265InterPred = *pstH265InterPred;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_INTER_PRED, &stSetH265InterPred);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265InterPred);

MI_S32 MI_VENC_GetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265InterPred_t stGetH265InterPred;

    MI_VENC_InitChnIoVar(stGetH265InterPred, VeChn, pstH265InterPred);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_INTER_PRED, &stGetH265InterPred);
    if (s32Ret == MI_SUCCESS)
    {
        stGetH265InterPred.stH265InterPred = *pstH265InterPred;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265InterPred);

MI_S32 MI_VENC_SetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265IntraPred_t stSetH265IntraPred;

    MI_VENC_InitChnIoVar(stSetH265IntraPred, VeChn, pstH265IntraPred);
    stSetH265IntraPred.stH265IntraPred = *pstH265IntraPred;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_INTRA_PRED, &stSetH265IntraPred);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265IntraPred);

MI_S32 MI_VENC_GetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265IntraPred_t stGetH265IntraPred;

    MI_VENC_InitChnIoVar(stGetH265IntraPred, VeChn, pstH265IntraPred);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_INTRA_PRED, &stGetH265IntraPred);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH265IntraPred = stGetH265IntraPred.stH265IntraPred;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265IntraPred);

MI_S32 MI_VENC_SetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Trans_t stSetH265Trans;

    if(pstH265Trans == NULL)
        return MI_ERR_VENC_NULL_PTR;

    if(pstH265Trans->u32InterTransMode != 0 ||
       pstH265Trans->u32IntraTransMode != 0)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    memset(&stSetH265Trans, 0, sizeof(stSetH265Trans));
    stSetH265Trans.VeChn = VeChn;
    stSetH265Trans.stH265Trans = *pstH265Trans;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_TRANS, &stSetH265Trans);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265Trans);

MI_S32 MI_VENC_GetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Trans_t stGetH265Trans;

    MI_VENC_InitChnIoVar(stGetH265Trans, VeChn, pstH265Trans);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_TRANS, &stGetH265Trans);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH265Trans = stGetH265Trans.stH265Trans;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265Trans);

MI_S32 MI_VENC_SetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Dblk_t stSetH265Dblk;

    MI_VENC_InitChnIoVar(stSetH265Dblk, VeChn, pstH265Dblk);
    stSetH265Dblk.stH265Dblk = *pstH265Dblk;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_DBLK, &stSetH265Dblk);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265Dblk);

MI_S32 MI_VENC_GetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Dblk_t stGetH265Dblk;

    MI_VENC_InitChnIoVar(stGetH265Dblk, VeChn, pstH265Dblk);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_DBLK, &stGetH265Dblk);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH265Dblk = stGetH265Dblk.stH265Dblk;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265Dblk);

MI_S32 MI_VENC_SetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t*pstH265Vui)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetH265Vui_t stSetH265Vui;

    MI_VENC_InitChnIoVar(stSetH265Vui, VeChn, pstH265Vui);
    stSetH265Vui.stH265Vui = *pstH265Vui;
    s32Ret = MI_SYSCALL(MI_VENC_SET_H265_VUI, &stSetH265Vui);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetH265Vui);

MI_S32 MI_VENC_GetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetH265Vui_t stGetH265Vui;

    MI_VENC_InitChnIoVar(stGetH265Vui, VeChn, pstH265Vui);
    s32Ret = MI_SYSCALL(MI_VENC_GET_H265_VUI, &stGetH265Vui);
    if (s32Ret == MI_SUCCESS)
    {
        *pstH265Vui = stGetH265Vui.stH265Vui;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetH265Vui);

MI_S32 MI_VENC_SetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetJpegParam_t stSetJpegParam;

    MI_VENC_InitChnIoVar(stSetJpegParam, VeChn, pstJpegParam);
    stSetJpegParam.stParamJpeg = *pstJpegParam;
    s32Ret = MI_SYSCALL(MI_VENC_SET_JPEG_PARAM, &stSetJpegParam);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetJpegParam);

MI_S32 MI_VENC_GetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetJpegParam_t stGetJpegParam;

    MI_VENC_InitChnIoVar(stGetJpegParam, VeChn, pstJpegParam);
    s32Ret = MI_SYSCALL(MI_VENC_GET_JPEG_PARAM, &stGetJpegParam);
    if (s32Ret == MI_SUCCESS)
    {
        *pstJpegParam = stGetJpegParam.stParamJpeg;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetJpegParam);

MI_S32 MI_VENC_SetRcParam(MI_VENC_CHN VeChn,MI_VENC_RcParam_t *pstRcParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRcParam_t stSetRcParam;

    MI_VENC_InitChnIoVar(stSetRcParam, VeChn, pstRcParam);
    stSetRcParam.stRcParam = *pstRcParam;
    s32Ret = MI_SYSCALL(MI_VENC_SET_RC_PARAM, &stSetRcParam);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetRcParam);

MI_S32 MI_VENC_GetRcParam(MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRcParam_t stGetRcParam;

    MI_VENC_InitChnIoVar(stGetRcParam, VeChn, pstRcParam);
    s32Ret = MI_SYSCALL(MI_VENC_GET_RC_PARAM, &stGetRcParam);
    if (s32Ret == MI_SUCCESS)
    {
        *pstRcParam = stGetRcParam.stRcParam;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetRcParam);

MI_S32 MI_VENC_SetRefParam(MI_VENC_CHN VeChn, MI_VENC_ParamRef_t *pstRefParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRefParam_t stSetRefParam;

    MI_VENC_InitChnIoVar(stSetRefParam, VeChn, pstRefParam);
    stSetRefParam.stRefParam = *pstRefParam;
    s32Ret = MI_SYSCALL(MI_VENC_SET_REF_PARAM, &stSetRefParam);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetRefParam);

MI_S32 MI_VENC_GetRefParam(MI_VENC_CHN VeChn, MI_VENC_ParamRef_t *pstRefParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRefParam_t stGetRefParam;

    MI_VENC_InitChnIoVar(stGetRefParam, VeChn, pstRefParam);
    s32Ret = MI_SYSCALL(MI_VENC_GET_REF_PARAM, &stGetRefParam);
    if (s32Ret == MI_SUCCESS)
    {
        *pstRefParam = stGetRefParam.stRefParam;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetRefParam);

MI_S32 MI_VENC_SetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetCrop_t stSetCrop;

    MI_VENC_InitChnIoVar(stSetCrop, VeChn, pstCropCfg);
    stSetCrop.stCropCfg = *pstCropCfg;
    s32Ret = MI_SYSCALL(MI_VENC_SET_CROP, &stSetCrop);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetCrop);

MI_S32 MI_VENC_GetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetCrop_t stGetCrop;

    MI_VENC_InitChnIoVar(stGetCrop, VeChn, pstCropCfg);
    s32Ret = MI_SYSCALL(MI_VENC_SET_CROP, &stGetCrop);
    if (s32Ret == MI_SUCCESS)
    {
        *pstCropCfg = stGetCrop.stCropCfg;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetCrop);

MI_S32 MI_VENC_SetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetFrameLostStrategy_t stSetFrameLostStrategy;

    MI_VENC_InitChnIoVar(stSetFrameLostStrategy, VeChn, pstFrmLostParam);
    stSetFrameLostStrategy.stFrmLostParam = *pstFrmLostParam;
    s32Ret = MI_SYSCALL(MI_VENC_SET_FRAME_LOST_STRATEGY, &stSetFrameLostStrategy);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetFrameLostStrategy);

MI_S32 MI_VENC_GetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetFrameLostStrategy_t stGetFrameLostStrategy;

    MI_VENC_InitChnIoVar(stGetFrameLostStrategy, VeChn, pstFrmLostParam);
    s32Ret = MI_SYSCALL(MI_VENC_GET_FRAME_LOST_STRATEGY, &stGetFrameLostStrategy);
    if (s32Ret == MI_SUCCESS)
    {
        *pstFrmLostParam = stGetFrameLostStrategy.stFrmLostParam;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetFrameLostStrategy);

MI_S32 MI_VENC_SetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetSuperFrameCfg_t stSetSuperFrameCfg;

    MI_VENC_InitChnIoVar(stSetSuperFrameCfg, VeChn, pstSuperFrmParam);
    stSetSuperFrameCfg.stSuperFrmParam = *pstSuperFrmParam;
    s32Ret = MI_SYSCALL(MI_VENC_SET_SUPER_FRAME_CFG, &stSetSuperFrameCfg);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetSuperFrameCfg);

MI_S32 MI_VENC_GetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetSuperFrameCfg_t stGetSuperFrameCfg;

    MI_VENC_InitChnIoVar(stGetSuperFrameCfg, VeChn, pstSuperFrmParam);
    s32Ret = MI_SYSCALL(MI_VENC_GET_SUPER_FRAME_CFG, &stGetSuperFrameCfg);
    if (s32Ret == MI_SUCCESS)
    {
        *pstSuperFrmParam = stGetSuperFrameCfg.stSuperFrmParam;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetSuperFrameCfg);

MI_S32 MI_VENC_SetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_SetRcPriority_t stSetRcPriority;

    MI_VENC_InitChnIoVar(stSetRcPriority, VeChn, peRcPriority);
    stSetRcPriority.eRcPriority = *peRcPriority;
    s32Ret = MI_SYSCALL(MI_VENC_SET_RC_PRIORITY, &stSetRcPriority);

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_SetRcPriority);

MI_S32 MI_VENC_GetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority)
{
    MI_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MI_VENC_GetRcPriority_t stGetRcPriority;

    MI_VENC_InitChnIoVar(stGetRcPriority, VeChn, peRcPriority);
    s32Ret = MI_SYSCALL(MI_VENC_GET_RC_PRIORITY, &stGetRcPriority);
    if (s32Ret == MI_SUCCESS)
    {
        *peRcPriority = stGetRcPriority.eRcPriority;
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_VENC_GetRcPriority);
