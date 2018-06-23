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

#include "mi_syscall.h"
#include "mi_print.h"

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_rgn_datatype.h"
#include "mi_rgn.h"
#include "rgn_ioctl.h"
#ifndef __KERNEL__
#include <pthread.h>
#endif
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
typedef struct MI_RGN_CanvasUserAddr_s
{
    void * pUsrAddr;
    MI_U32 u32Size;
}MI_RGN_CanvasUserAddr_t;

MI_MODULE_DEFINE(rgn);

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

#ifndef __KERNEL__
static pthread_mutex_t _gstCanvasAddrMutex = PTHREAD_MUTEX_INITIALIZER;
static MI_RGN_CanvasUserAddr_t _gstCanvasUsrAddr[MI_RGN_MAX_HANDLE];
#endif
//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

MI_S32 MI_RGN_Init(MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;

    s32Ret = MI_SYSCALL(MI_RGN_INIT, &pstPaletteTable);
#ifndef __KERNEL__
    memset(_gstCanvasUsrAddr, 0, sizeof(MI_RGN_CanvasUserAddr_t) * MI_RGN_MAX_HANDLE);
#endif
    return s32Ret;
}

MI_S32 MI_RGN_DeInit(void)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;

    //set &stDeInit as void
    s32Ret = MI_SYSCALL_VOID(MI_RGN_DE_INIT);

    return s32Ret;
}

MI_S32 MI_RGN_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_CreatePara_t stCreate;

    memset(&stCreate, 0, sizeof(stCreate));
    //set &stCreate as MI_RGN_HANDLE hHandle,  MI_RGN_Attr_t *pstRegion

    stCreate.hHandle = hHandle;
    memcpy(&stCreate.stRegion, pstRegion, sizeof(MI_RGN_Attr_t));
    s32Ret = MI_SYSCALL(MI_RGN_CREATE, &stCreate);

    return s32Ret;
}

MI_S32 MI_RGN_Destroy(MI_RGN_HANDLE hHandle)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;

    s32Ret = MI_SYSCALL(MI_RGN_DESTROY, &hHandle);
#ifndef __KERNEL__
    if (s32Ret == MI_RGN_OK)
    {
        pthread_mutex_lock(&_gstCanvasAddrMutex);
        //set &stDestroy as MI_RGN_HANDLE hHandle
        if (_gstCanvasUsrAddr[hHandle].pUsrAddr)
        {
            s32Ret = MI_SYS_Munmap(_gstCanvasUsrAddr[hHandle].pUsrAddr, _gstCanvasUsrAddr[hHandle].u32Size);
            if (MI_SUCCESS != s32Ret)
            {
               s32Ret = MI_ERR_RGN_BADADDR;
               pthread_mutex_unlock(&_gstCanvasAddrMutex);
               goto ERROR;
            }
            else
            {
                s32Ret = MI_RGN_OK;
                _gstCanvasUsrAddr[hHandle].pUsrAddr = NULL;
                _gstCanvasUsrAddr[hHandle].u32Size = 0;
            }
        }
        pthread_mutex_unlock(&_gstCanvasAddrMutex);
    }
ERROR:
#endif

    return s32Ret;
}

MI_S32 MI_RGN_GetAttr(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_CreatePara_t stGetAttr;

    memset(&stGetAttr, 0, sizeof(stGetAttr));
    //set &stGetAttr as MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion
    stGetAttr.hHandle = hHandle;
    memcpy(&stGetAttr.stRegion, pstRegion, sizeof(MI_RGN_Attr_t));

    s32Ret = MI_SYSCALL(MI_RGN_GET_ATTR, &stGetAttr);
    if (s32Ret == MI_RGN_OK)
    {
        memcpy(pstRegion, &stGetAttr.stRegion, sizeof(MI_RGN_Attr_t));
    }

    return s32Ret;
}

MI_S32 MI_RGN_SetBitMap(MI_RGN_HANDLE hHandle, MI_RGN_Bitmap_t *pstBitmap)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_SetBitMapPara_t stSetBitMap;

    memset(&stSetBitMap, 0, sizeof(stSetBitMap));
    //set &stSetBitMap as MI_RGN_HANDLE hHandle,  MI_RGN_Bitmap_t *pstBitmap
    stSetBitMap.hHandle = hHandle;
    memcpy(&stSetBitMap.stBitmap, pstBitmap, sizeof(MI_RGN_Bitmap_t));
    s32Ret = MI_SYSCALL(MI_RGN_SET_BIT_MAP, &stSetBitMap);

    return s32Ret;
}

MI_S32 MI_RGN_AttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t stAttachToChn;

    memset(&stAttachToChn, 0, sizeof(stAttachToChn));
    //set &stAttachToChn as MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t* pstChnPort,  MI_RGN_ChnPortParam_t *pstChnAttr

    stAttachToChn.hHandle = hHandle;
    memcpy(&stAttachToChn.stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    memcpy(&stAttachToChn.stChnAttr, pstChnAttr, sizeof(MI_RGN_ChnPortParam_t));
    s32Ret = MI_SYSCALL(MI_RGN_ATTACH_TO_CHN, &stAttachToChn);

    return s32Ret;
}

MI_S32 MI_RGN_DetachFromChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_DetachFromChnPara_t stDetachFromChn;

    memset(&stDetachFromChn, 0, sizeof(stDetachFromChn));
    //set &stDetachFromChn as MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t *pstChnPort

    stDetachFromChn.hHandle = hHandle;
    memcpy(&stDetachFromChn.stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    s32Ret = MI_SYSCALL(MI_RGN_DETACH_FROM_CHN, &stDetachFromChn);

    return s32Ret;
}

MI_S32 MI_RGN_SetDisplayAttr(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t stSetDisplayAttr;

    memset(&stSetDisplayAttr, 0, sizeof(stSetDisplayAttr));
    //set &stSetDisplayAttr as MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t *pstChnPort,  MI_RGN_ChnPortParam_t *pstChnPortAttr
    stSetDisplayAttr.hHandle = hHandle;
    memcpy(&stSetDisplayAttr.stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    memcpy(&stSetDisplayAttr.stChnAttr, pstChnPortAttr, sizeof(MI_RGN_ChnPortParam_t));
    s32Ret = MI_SYSCALL(MI_RGN_SET_DISPLAY_ATTR, &stSetDisplayAttr);

    return s32Ret;
}

MI_S32 MI_RGN_GetDisplayAttr(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_AttachToChnPara_t stGetDisplayAttr;

    memset(&stGetDisplayAttr, 0, sizeof(stGetDisplayAttr));
    //set &stGetDisplayAttr as MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr
    stGetDisplayAttr.hHandle = hHandle;
    memcpy(&stGetDisplayAttr.stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));

    s32Ret = MI_SYSCALL(MI_RGN_GET_DISPLAY_ATTR, &stGetDisplayAttr);
    if (s32Ret == MI_RGN_OK)
    {
        memcpy(pstChnPortAttr, &stGetDisplayAttr.stChnAttr, sizeof(MI_RGN_ChnPortParam_t));
    }

    return s32Ret;
}
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))

MI_S32 MI_RGN_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;
    MI_RGN_GetCanvasInfoPara_t stGetCanvasInfo;

    memset(&stGetCanvasInfo, 0, sizeof(stGetCanvasInfo));
    //set &stGetCanvasInfo as MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo
    stGetCanvasInfo.hHandle = hHandle;


    s32Ret = MI_SYSCALL(MI_RGN_GET_CANVAS_INFO, &stGetCanvasInfo);

#ifndef __KERNEL__
    MI_U32 u32ApplySize;
    void * pAddr = NULL;

    if (s32Ret == MI_RGN_OK)
    {
        pthread_mutex_lock(&_gstCanvasAddrMutex);
        if (_gstCanvasUsrAddr[hHandle].pUsrAddr == NULL)
        {
            u32ApplySize = stGetCanvasInfo.stCanvasInfo.u32Stride * stGetCanvasInfo.stCanvasInfo.stSize.u32Height;
            s32Ret = MI_SYS_Mmap(stGetCanvasInfo.stCanvasInfo.phyAddr, u32ApplySize, &pAddr, FALSE);
            if (s32Ret != MI_SUCCESS || pAddr == NULL)
            {
                pthread_mutex_unlock(&_gstCanvasAddrMutex);
                s32Ret = MI_ERR_RGN_BADADDR;
                goto ERROR;
            }
            s32Ret = MI_RGN_OK;
            _gstCanvasUsrAddr[hHandle].pUsrAddr = pAddr;
            _gstCanvasUsrAddr[hHandle].u32Size = u32ApplySize;
        }
        stGetCanvasInfo.stCanvasInfo.virtAddr = (MI_VIRT)_gstCanvasUsrAddr[hHandle].pUsrAddr;
        pthread_mutex_unlock(&_gstCanvasAddrMutex);
        memcpy(pstCanvasInfo, &stGetCanvasInfo.stCanvasInfo, sizeof(MI_RGN_CanvasInfo_t));
    }
ERROR:
#endif
    return s32Ret;
}

MI_S32 MI_RGN_UpdateCanvas(MI_RGN_HANDLE hHandle)
{
    MI_S32 s32Ret = MI_ERR_RGN_BUSY;

    //set &stUpdateCanvas as MI_RGN_HANDLE hHandle
    s32Ret = MI_SYSCALL(MI_RGN_UPDATE_CANVAS, &hHandle);
    return s32Ret;
}

EXPORT_SYMBOL(MI_RGN_Init);
EXPORT_SYMBOL(MI_RGN_DeInit);
EXPORT_SYMBOL(MI_RGN_Create);
EXPORT_SYMBOL(MI_RGN_Destroy);
EXPORT_SYMBOL(MI_RGN_GetAttr);
EXPORT_SYMBOL(MI_RGN_SetBitMap);
EXPORT_SYMBOL(MI_RGN_AttachToChn);
EXPORT_SYMBOL(MI_RGN_DetachFromChn);
EXPORT_SYMBOL(MI_RGN_SetDisplayAttr);
EXPORT_SYMBOL(MI_RGN_GetDisplayAttr);
EXPORT_SYMBOL(MI_RGN_GetCanvasInfo);
EXPORT_SYMBOL(MI_RGN_UpdateCanvas);

