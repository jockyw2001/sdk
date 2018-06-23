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

#include <linux/string.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/fs.h>

#include "mi_common.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_rgn_impl.h"
#include "mi_rgn_internal.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"


typedef struct MI_RGN_WinowHandler_s
{
    struct list_head handleToWinHandler;
    MI_RGN_WIN_HANDLER hWinHandler;
}MI_RGN_WinowHandler_t;
typedef struct MI_RGN_HandleListData_s
{
    struct list_head handleList;
    struct list_head handleToWinHandler;
    MI_S32 hHandle;
    MI_RGN_Attr_t stRegAttr;
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_BOOL bGetCanvas;
}MI_RGN_HandleListData_t;

typedef struct MI_RGN_HandleListHead_s
{
    struct list_head handleList;

    mi_rgn_WindowManger_t stWinManger;
    mi_rgn_BufferManager_t stBufManger;
    MI_BOOL bRegistManger;

    struct workqueue_struct *pStBufDelWorkQueue;
    struct work_struct bufDelFenceWork;
    MI_U64 u64Fence;

    MI_BOOL bInitFlag;
    MI_U16 u16ListCnt;
}MI_RGN_HandleListHead_t;

typedef struct mi_rgn_BufCntHead_s
{
    struct list_head listBufCnt;
    MI_U32 u32BufferCntKmalloc;
    MI_U32 u32BufferCntMma;
}mi_rgn_BufCntHead_t;
typedef struct mi_rgn_BufCntData_s
{
    struct list_head listBufCnt;
    MI_U32 u32Size;
    void *pAddr;
}mi_rgn_BufCntData_t;


#ifdef MI_SYS_PROC_FS_DEBUG
#define MI_RGN_PROC_DUMP_PROCESS_DURITION 1
static MI_SYS_DRV_HANDLE hSysDrvHandle = NULL;
#endif
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
typedef struct MI_RGN_ProcessTimeCheck_s
{
    MI_U32 u32TimeDiff;
    MI_U32 u32Channel;
}MI_RGN_ProcessTimeCheck_t;
#endif

static DEFINE_SEMAPHORE(_gstRgnListSem);
static DEFINE_SEMAPHORE(_gStBufCntSem);
static DEFINE_SEMAPHORE(_gStRgnStateSem);
static mi_rgn_BufCntHead_t _gstBufCntHead = {.listBufCnt = {&_gstBufCntHead.listBufCnt, &_gstBufCntHead.listBufCnt}, .u32BufferCntKmalloc = 0, .u32BufferCntMma = 0};
static MI_RGN_HandleListHead_t _stRegionlist = {.handleList = {&_stRegionlist.handleList, &_stRegionlist.handleList}, .bRegistManger = FALSE, .bInitFlag = FALSE};
static mi_rgn_DrvCapability_t _gstCapablity;
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
static MI_RGN_ProcessTimeCheck_t stVpeCur[MI_RGN_VPE_PORT_MAXNUM], stVpeMax[MI_RGN_VPE_PORT_MAXNUM], stDivpCur[MI_RGN_DIVP_PORT_MAXNUM], stDivpMax[MI_RGN_DIVP_PORT_MAXNUM];
#endif
#if MI_RGN_MODULE_TEST
static MI_BOOL bUseInitFlag = FALSE;
#endif
void mi_rgn_MemAllocCnt(void *pAddr, MI_U32 u32Size, MI_BOOL bMode);
void mi_rgn_MemFreeCnt(void *pAddr, MI_BOOL bMode);

static MI_S32 _MI_RGN_IMPL_CheckRgnOsdAttr( MI_RGN_OsdInitParam_t *pstOsdInitPara)
{
    MI_RGN_RANGE_CHECK(pstOsdInitPara->stSize.u32Height, _gstCapablity.u16OsdMinHeight, _gstCapablity.u16OsdMaxHeight, return MI_ERR_RGN_ILLEGAL_PARAM;);
    MI_RGN_RANGE_CHECK(pstOsdInitPara->stSize.u32Width, _gstCapablity.u16OsdMinWidth, _gstCapablity.u16OsdMaxWidth, return MI_ERR_RGN_ILLEGAL_PARAM;);

    if (pstOsdInitPara->ePixelFmt >= E_MI_RGN_PIXEL_FORMAT_MAX)
        return MI_ERR_RGN_ILLEGAL_PARAM;
    if (!_gstCapablity.bSupportFormat[pstOsdInitPara->ePixelFmt])
        return MI_ERR_RGN_ILLEGAL_PARAM;
    switch(pstOsdInitPara->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
            MI_RGN_ALIGN_CHECK(pstOsdInitPara->stSize.u32Width, _gstCapablity.u8OsdI2WidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
        default:
            MI_RGN_ALIGN_CHECK(pstOsdInitPara->stSize.u32Width, _gstCapablity.u8OsdWidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
    }

    return MI_RGN_OK;
}


static MI_S32 _MI_RGN_IMPL_GetHandleListData(MI_RGN_HANDLE hHandle, MI_RGN_HandleListData_t**ppHandleListData)
{
    MI_RGN_HandleListData_t *pstLd = NULL;

    MI_RGN_PTR_CHECK(ppHandleListData, MI_SYS_BUG(););
    *ppHandleListData = NULL;
    list_for_each_entry(pstLd, &_stRegionlist.handleList, handleList)
    {
        if (hHandle == pstLd->hHandle)
        {
            *ppHandleListData = pstLd;
            break;
        }
    }

    return MI_RGN_OK;
}
static MI_S32 _MI_RGN_IMPL_CheckCanvasBufAttr( MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    MI_SYS_BUG_ON(!pstCanvasInfo);

    MI_RGN_RANGE_CHECK(pstCanvasInfo->stSize.u32Height, _gstCapablity.u16OsdMinHeight, _gstCapablity.u16OsdMaxHeight, return MI_ERR_RGN_ILLEGAL_PARAM;);
    MI_RGN_RANGE_CHECK(pstCanvasInfo->stSize.u32Width, _gstCapablity.u16OsdMinWidth, _gstCapablity.u16OsdMaxWidth, return MI_ERR_RGN_ILLEGAL_PARAM;);
    if (pstCanvasInfo->ePixelFmt >= E_MI_RGN_PIXEL_FORMAT_MAX)
        return MI_ERR_RGN_ILLEGAL_PARAM;

    switch(pstCanvasInfo->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
            MI_RGN_ALIGN_CHECK(pstCanvasInfo->stSize.u32Width, _gstCapablity.u8OsdI2WidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
        default:
            MI_RGN_ALIGN_CHECK(pstCanvasInfo->stSize.u32Width, _gstCapablity.u8OsdWidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
    }

    return MI_RGN_OK;
}
static MI_S32 _MI_RGN_IMPL_CheckCoverAttr(MI_RGN_Point_t *pstPonit, MI_RGN_CoverChnPortParam_t *pstCoverPara)
{
    MI_SYS_BUG_ON(!pstCoverPara);
    MI_SYS_BUG_ON(!pstPonit);

    MI_RGN_RANGE_CHECK(pstCoverPara->stSize.u32Height, _gstCapablity.u16CoverMinHeight, _gstCapablity.u16CoverMaxHeight, return MI_ERR_RGN_ILLEGAL_PARAM;);
    MI_RGN_RANGE_CHECK(pstCoverPara->stSize.u32Width, _gstCapablity.u16CoverMinWidth, _gstCapablity.u16CoverMaxWidth, return MI_ERR_RGN_ILLEGAL_PARAM;);
    MI_RGN_RANGE_CHECK(pstPonit->u32Y, 0, _gstCapablity.u16CoverMaxHeight - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);
    MI_RGN_RANGE_CHECK(pstPonit->u32X, 0, _gstCapablity.u16CoverMaxWidth - 1, return MI_ERR_RGN_ILLEGAL_PARAM;);

    return MI_RGN_OK;
}
static MI_S32 _MI_RGN_IMPL_CheckOsdPosAttr(MI_RGN_Point_t *pstPonit, MI_RGN_PixelFormat_e ePixelFmt)
{
    MI_SYS_BUG_ON(!pstPonit);

    MI_RGN_ALIGN_CHECK(pstPonit->u32Y, _gstCapablity.u8OsdWidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
    if (ePixelFmt >= E_MI_RGN_PIXEL_FORMAT_MAX)
        return MI_ERR_RGN_ILLEGAL_PARAM;
    switch(ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_I2:
            MI_RGN_ALIGN_CHECK(pstPonit->u32X, _gstCapablity.u8OsdI2WidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
        default:
            MI_RGN_ALIGN_CHECK(pstPonit->u32X, _gstCapablity.u8OsdWidthAlignment, return MI_ERR_RGN_ILLEGAL_PARAM;);
            break;
    }

    return MI_RGN_OK;
}
static MI_S32 _MI_RGN_IMPL_CheckChnPortAttr(MI_RGN_Type_e eType, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;

    MI_SYS_BUG_ON(!pstChnPortAttr);

    switch (eType)
    {
        case E_MI_RGN_TYPE_OSD:
        {
            s32ErrorCode = _MI_RGN_IMPL_CheckOsdPosAttr(&pstChnPortAttr->stPoint, E_MI_RGN_TYPE_OSD);
            if (s32ErrorCode != MI_RGN_OK)
            {
                DBG_EXIT_ERR("Osd pos error\n");
                goto ERROR_CODE;
            }
        }
        break;
        case E_MI_RGN_TYPE_COVER:
        {
            s32ErrorCode = _MI_RGN_IMPL_CheckCoverAttr(&pstChnPortAttr->stPoint, &pstChnPortAttr->stCoverPara);
            if (s32ErrorCode != MI_RGN_OK)
            {
                DBG_EXIT_ERR("Cover attr error!\n");
                goto ERROR_CODE;
            }
        }
        break;
        default:
        {
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            DBG_EXIT_ERR("Error region type .\n");
            goto ERROR_CODE;
        }
        break;
    }

ERROR_CODE:
    return s32ErrorCode;
}

static MI_S32 _MI_RGN_IMPL_GetWindowListData(MI_RGN_HandleListData_t *pHandleListData, MI_RGN_ChnPort_t* pstChPort, MI_RGN_WinowHandler_t **ppstWinHandler)
{
    MI_RGN_WinowHandler_t *pos;
    MI_RGN_ChnPort_t stChnPortTmp;

    MI_RGN_PTR_CHECK(pHandleListData, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(pstChPort, MI_SYS_BUG(););
    MI_RGN_PTR_CHECK(ppstWinHandler, MI_SYS_BUG(););

    *ppstWinHandler = NULL;
    list_for_each_entry(pos, &pHandleListData->handleToWinHandler, handleToWinHandler)
    {
        _stRegionlist.stWinManger.fpGetChPort(pos->hWinHandler, &stChnPortTmp);
        if (stChnPortTmp.s32ChnId == pstChPort->s32ChnId
            && stChnPortTmp.s32OutputPortId == pstChPort->s32OutputPortId
            && stChnPortTmp.eModId == pstChPort->eModId)
        {
            *ppstWinHandler = pos;
            break;
        }
    }

    return MI_RGN_OK;
}
#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_RGN_PROCFS_DEBUG == 1)
static MI_S32 _MI_RGN_IMPL_GetCapability(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_U8 i = 0;

    handle.OnPrintOut(handle, "\n---------------------------------------- start dump region capability info -----------------------------------------\n");
    handle.OnPrintOut(handle, "Module name : REGION\n");
    handle.OnPrintOut(handle, "\nLimitation :\n");
    for (i = 0; i < E_MI_RGN_PIXEL_FORMAT_MAX; i++)
        handle.OnPrintOut(handle, "%-13s : fmt%4d %-11s\n", "Osd supported format", i, _gstCapablity.bSupportFormat[i] ? "Support" : "Unsupport");
    handle.OnPrintOut(handle, "%-13s : %4d (0 ~ %4d)\n", "Region handle", MI_RGN_MAX_HANDLE, MI_RGN_MAX_HANDLE - 1);
    handle.OnPrintOut(handle, "%-13s : %4d (0 ~ %4d)\n", "Vpe channel",_gstCapablity.u8VpeMaxChannelNum, _gstCapablity.u8VpeMaxChannelNum - 1);
    handle.OnPrintOut(handle, "%-13s : %4d (0 ~ %4d)\n", "Vpe port", _gstCapablity.u8VpeMaxPortNum, _gstCapablity.u8VpeMaxPortNum - 1);
    handle.OnPrintOut(handle, "%-13s : %4d (0 ~ %4d)\n", "Divp channel", _gstCapablity.u8DivpMaxChannelNum, _gstCapablity.u8DivpMaxChannelNum - 1);
    handle.OnPrintOut(handle, "%-13s : %4d (0 ~ %4d)\n", "Divp port", _gstCapablity.u8DivpMaxPortNum, _gstCapablity.u8DivpMaxPortNum - 1);
    handle.OnPrintOut(handle, "%-13s : %4d (each channel port)\n", "Osd attach", _gstCapablity.u8AttachedOsdCnt);
    handle.OnPrintOut(handle, "%-13s : %4d (each channel port)\n", "Cover attach", _gstCapablity.u8AttachedCoverCnt);

    handle.OnPrintOut(handle, "\nOverlay support :\n");
    handle.OnPrintOut(handle, "Format :\n");
    handle.OnPrintOut(handle, "  %-8s : width alignment %d pixel\n", "ARGB1555", _gstCapablity.u8OsdWidthAlignment);
    handle.OnPrintOut(handle, "  %-8s : width alignment %d pixel\n", "ARGB4444", _gstCapablity.u8OsdWidthAlignment);
    handle.OnPrintOut(handle, "  %-8s : width alignment %d pixel\n", "I2", _gstCapablity.u8OsdI2WidthAlignment);
    handle.OnPrintOut(handle, "  %-8s : width alignment %d pixel\n", "I4", _gstCapablity.u8OsdWidthAlignment);
    handle.OnPrintOut(handle, "Size :\n");
    handle.OnPrintOut(handle, "  %-7s : %d ~ %4d\n", "Width", _gstCapablity.u16OsdMinWidth, _gstCapablity.u16OsdMaxWidth);
    handle.OnPrintOut(handle, "  %-7s : %d ~ %4d\n", "Height", _gstCapablity.u16OsdMinHeight, _gstCapablity.u16OsdMaxHeight);
    handle.OnPrintOut(handle, "  %-7s : %d frontbuffs\n", "HW mode", _gstCapablity.u8OsdHwModeCnt);
    handle.OnPrintOut(handle, "  %-7s : Not support\n", "Overlap");

    handle.OnPrintOut(handle, "\nCover support :\n");
    handle.OnPrintOut(handle, "Size :\n");
    handle.OnPrintOut(handle, "  %-7s : %d ~ %4d\n", "Width", _gstCapablity.u16CoverMinWidth, _gstCapablity.u16CoverMaxWidth);
    handle.OnPrintOut(handle, "  %-7s : %d ~ %4d\n", "Height", _gstCapablity.u16CoverMinHeight, _gstCapablity.u16CoverMaxHeight);
    handle.OnPrintOut(handle, "  %-7s : Support\n", "Overlap");
    handle.OnPrintOut(handle, "\n---------------------------------------- end dump region capability info -------------------------------------------\n");

    return MI_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_DumpRegionAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_RGN_HandleListData_t *pstHl = NULL;

    handle.OnPrintOut(handle, "\n---------------------------------------------- start dump region attr ----------------------------------------------\n");
    handle.OnPrintOut(handle, "%15s%15s%15s%15s%15s%15s%15s%15s\n", "Handle", "Type", "Width", "Height", "Stride", "Format", "VirAddr", "PhyAddr");
    down(&_gstRgnListSem);
    list_for_each_entry(pstHl, &_stRegionlist.handleList, handleList)
    {
        switch (pstHl->stRegAttr.eType)
        {
            case E_MI_RGN_TYPE_OSD:
            {
                handle.OnPrintOut(handle, "%15d%15s%15d%15d%15d%15d%15x%15x\n", pstHl->hHandle, "OSD",
                    pstHl->stRegAttr.stOsdInitParam.stSize.u32Width, pstHl->stRegAttr.stOsdInitParam.stSize.u32Height, pstHl->stCanvasInfo.u32Stride,
                     pstHl->stRegAttr.stOsdInitParam.ePixelFmt, pstHl->stCanvasInfo.virtAddr, pstHl->stCanvasInfo.phyAddr);
            }
            break;
            case E_MI_RGN_TYPE_COVER:
            {
                handle.OnPrintOut(handle, "%15d%15s\n", pstHl->hHandle, "COVER");
            }
            break;
            default:
                break;
        }
    }
    up(&_gstRgnListSem);

    handle.OnPrintOut(handle, "\n----------------------------------------------- end dump region attr -----------------------------------------------\n");
    return MI_SUCCESS;
}

static LIST_HEAD(_gDumpListHead);

typedef struct MI_RGN_DumpFrontBufferInfo_s
{
    MI_BOOL bShow;
    MI_RGN_CanvasInfo_t stFrontBuf;
    MI_RGN_Point_t stPoint;
}MI_RGN_DumpFrontBufferInfo_t;

typedef struct MI_RGN_DumpChnPortListHead_s
{
    struct list_head stListHead;
    struct list_head stDumpList;
    MI_U32 u32MagicNumber;
    MI_RGN_ChnPort_t stChnPort;
    MI_RGN_DumpFrontBufferInfo_t *pstFrontBuffer;
    MI_RGN_Size_t stBaseWindow;
    MI_U8 u8FrontBufferCount;
}MI_RGN_DumpChnPortListHead_t;
typedef struct MI_RGN_DumpChnPort_s
{
    struct list_head stDumpList;
    MI_S32 hHandle;
    MI_RGN_Attr_t stRgnAttr;
    MI_RGN_CanvasInfo_t stBackgroundBuf;
    MI_RGN_ChnPortParam_t stChnPortPara;
}MI_RGN_DumpChnPort_t;

static MI_S32 _MI_RGN_IMPL_GetDumpListHead(MI_RGN_ChnPort_t *pstChnPort, MI_RGN_DumpChnPortListHead_t **ppstDumpListHead)
{
    MI_RGN_DumpChnPortListHead_t *pos = NULL, *pstList = NULL;
    MI_U32 u32MagicNumber = 0;
    MI_U8 i = 0;

    MI_SYS_BUG_ON(!pstChnPort);
    MI_SYS_BUG_ON(!ppstDumpListHead);

    u32MagicNumber = pstChnPort->s32OutputPortId | ((pstChnPort->s32ChnId & 0xFF) << 8) | ((pstChnPort->eModId & 0xFF) << 16);

    list_for_each_entry(pos, &_gDumpListHead, stListHead)
    {
        if (pos->u32MagicNumber == u32MagicNumber)
        {
            *ppstDumpListHead = pos;
            return MI_RGN_OK;
        }
        if (pos->u32MagicNumber > u32MagicNumber)
        {
            MI_RGN_MEMALLOC(pstList, sizeof(MI_RGN_DumpChnPortListHead_t));
            MI_SYS_BUG_ON(!pstList);
            pstList->u32MagicNumber = u32MagicNumber;
            memcpy(&pstList->stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
            pstList->u8FrontBufferCount = _gstCapablity.u8OsdHwModeCnt;
            MI_RGN_MEMALLOC(pstList->pstFrontBuffer, sizeof(MI_RGN_DumpFrontBufferInfo_t) * pstList->u8FrontBufferCount);
            MI_SYS_BUG_ON(!pstList->pstFrontBuffer);
            for (i = 0; i < pstList->u8FrontBufferCount; i++)
            {
                _stRegionlist.stWinManger.fpGetFrontBuffer(i, &pstList->stChnPort, &pstList->pstFrontBuffer[i].stFrontBuf, &pstList->pstFrontBuffer[i].stPoint, &pstList->pstFrontBuffer[i].bShow);
            }
            _stRegionlist.stWinManger.fpGetBaseWindow(&pstList->stChnPort, &pstList->stBaseWindow);
            INIT_LIST_HEAD(&pstList->stDumpList);
            list_add(&pstList->stListHead, &pos->stListHead);
            *ppstDumpListHead = pstList;
            return MI_RGN_OK;
        }
    }
    MI_RGN_MEMALLOC(pstList, sizeof(MI_RGN_DumpChnPortListHead_t));
    MI_SYS_BUG_ON(!pstList);
    pstList->u32MagicNumber = u32MagicNumber;
    memcpy(&pstList->stChnPort, pstChnPort, sizeof(MI_RGN_ChnPort_t));
    pstList->u8FrontBufferCount = _gstCapablity.u8OsdHwModeCnt;
    MI_RGN_MEMALLOC(pstList->pstFrontBuffer, sizeof(MI_RGN_DumpFrontBufferInfo_t) * pstList->u8FrontBufferCount);
    MI_SYS_BUG_ON(!pstList->pstFrontBuffer);
    for (i = 0; i < pstList->u8FrontBufferCount; i++)
    {
        _stRegionlist.stWinManger.fpGetFrontBuffer(i, &pstList->stChnPort, &pstList->pstFrontBuffer[i].stFrontBuf, &pstList->pstFrontBuffer[i].stPoint, &pstList->pstFrontBuffer[i].bShow);
    }
    _stRegionlist.stWinManger.fpGetBaseWindow(&pstList->stChnPort, &pstList->stBaseWindow);
    INIT_LIST_HEAD(&pstList->stDumpList);
    list_add_tail(&pstList->stListHead, &_gDumpListHead);
    *ppstDumpListHead = pstList;

    return MI_RGN_OK;
}

static MI_S32 _MI_RGN_IMPL_DumpInfoToFile(const MI_U8 *pszFileName, const MI_U8 *pszBuffer, MI_U32 u32Length)
{
    struct file *pFile = NULL;
    mm_segment_t stSeg;
    loff_t pos = 0;
    ssize_t writeBytes = 0;
    pFile = filp_open(pszFileName, O_RDWR | O_CREAT,0644);

    if (IS_ERR(pFile))
    {
        printk("Create file:%s error.\n", pszFileName);
        return -1;
    }

    stSeg = get_fs();
    set_fs(KERNEL_DS);
    writeBytes = vfs_write(pFile, (char*)pszBuffer, u32Length, &pos);
    filp_close(pFile, NULL);
    set_fs(stSeg);
    //printk("Write %d bytes to %s.\n", writeBytes, pszFileName);

    return (MI_S32)writeBytes;
}

// 3 args, arg 0 is chn, arg 1 is port, arg 3 is filepath
static MI_S32 _MI_RGN_IMPL_DumpFrontBufToFile(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_U8 i = 0;
    MI_BOOL bShow = TRUE;
    MI_U8 *pFileName = NULL;
    MI_RGN_ChnPort_t stChPort;
    MI_RGN_Point_t stPoint;
    MI_RGN_CanvasInfo_t stFrontBuff;
    char *ps8After = NULL;
    MI_U8 *u8DumpAddr = NULL;

    MI_RGN_MEMALLOC(pFileName, 256);
    memset(pFileName, 0, 256);
    memset(&stChPort, 0, sizeof(stChPort));
    stChPort.eModId = (MI_RGN_ModId_e)(simple_strtoul(argv[1], &ps8After, 10));
    stChPort.s32ChnId = simple_strtoul(argv[2], &ps8After, 10);
    stChPort.s32OutputPortId = simple_strtoul(argv[3], &ps8After, 10);
    stPoint.u32X = 0;
    stPoint.u32Y = 0;

    down(&_gstRgnListSem);
    for (i = 0; i < _gstCapablity.u8OsdHwModeCnt; i++)
    {
        memset(&stFrontBuff, 0, sizeof(stFrontBuff));
        _stRegionlist.stWinManger.fpGetFrontBuffer(i, &stChPort, &stFrontBuff, &stPoint, &bShow);

        if (stFrontBuff.phyAddr == 0 && stFrontBuff.stSize.u32Width == 0
            && stFrontBuff.stSize.u32Height == 0 && stFrontBuff.u32Stride == 0)
        {
            continue;
        }
        u8DumpAddr = mi_sys_Vmap(stFrontBuff.phyAddr, stFrontBuff.u32Stride * stFrontBuff.stSize.u32Height, FALSE);
        if (NULL == u8DumpAddr)
        {
            DBG_EXIT_ERR("Map error.\n");
            continue;
        }
        sprintf(pFileName, "%s/%s_Chn%d_Port%d_frontBuf%d_fmt%d_%dX%d_stride%d", argv[4], ((stChPort.eModId == E_MI_RGN_MODID_VPE) ? "Vpe" : "Divp"),
                stChPort.s32ChnId, stChPort.s32OutputPortId, i, stFrontBuff.ePixelFmt, stFrontBuff.stSize.u32Width, stFrontBuff.stSize.u32Height, stFrontBuff.u32Stride);
        _MI_RGN_IMPL_DumpInfoToFile(pFileName, u8DumpAddr, stFrontBuff.stSize.u32Height*stFrontBuff.u32Stride);
        mi_sys_UnVmap(u8DumpAddr);
        handle.OnPrintOut(handle, "dump %s chn %d port %d frontbuff info to %s\n", ((stChPort.eModId == E_MI_RGN_MODID_VPE) ? "Vpe" : "Divp"),
                          stChPort.s32ChnId, stChPort.s32OutputPortId, pFileName);
    }
    up(&_gstRgnListSem);

    MI_RGN_MEMFREE(pFileName);

    return MI_SUCCESS;
}

// 2args, arg 0 is rgn handle, arg 1 is filepath
static MI_S32 _MI_RGN_IMPL_DumpRgnBufToFile(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_U8 *pFileName = NULL;
    MI_RGN_HandleListData_t *pstHl = NULL;
    char *ps8After = NULL;
    MI_HANDLE hHandle = simple_strtoul(argv[1], &ps8After, 10);
    MI_U8 *u8DumpAddr = NULL;

    MI_RGN_MEMALLOC(pFileName, 256);
    memset(pFileName, 0, 256);

    down(&_gstRgnListSem);
    _MI_RGN_IMPL_GetHandleListData(hHandle, &pstHl);
    if (pstHl)
    {
        if (pstHl->stRegAttr.eType == E_MI_RGN_TYPE_OSD)
        {
            u8DumpAddr = mi_sys_Vmap(pstHl->stCanvasInfo.phyAddr, pstHl->stCanvasInfo.u32Stride * pstHl->stCanvasInfo.stSize.u32Height, FALSE);
            if (NULL == u8DumpAddr)
            {
                DBG_EXIT_ERR("Map error.\n");
                up(&_gstRgnListSem);

                return MI_ERR_RGN_NOBUF;
            }
            sprintf(pFileName, "%s/Rgn%d_canvasInfo_fmt%d_%dX%d_stride%d", argv[2], hHandle, pstHl->stCanvasInfo.ePixelFmt,
                                pstHl->stCanvasInfo.stSize.u32Width, pstHl->stCanvasInfo.stSize.u32Height, pstHl->stCanvasInfo.u32Stride);
            _MI_RGN_IMPL_DumpInfoToFile(pFileName, u8DumpAddr, pstHl->stCanvasInfo.stSize.u32Height*pstHl->stCanvasInfo.u32Stride);
            mi_sys_UnVmap(u8DumpAddr);
            handle.OnPrintOut(handle, "dump rgn %d canvas info to %s\n", hHandle, pFileName);
        }
    }
    up(&_gstRgnListSem);

    MI_RGN_MEMFREE(pFileName);

    return MI_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_DumpBufUsedInfo(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    handle.OnPrintOut(handle, "\n---------------------------------------- start dump info of buffers in used ----------------------------------------\n");
    down(&_gStBufCntSem);
//    handle.OnPrintOut(handle, "\033[1;36mMemory used:\n\033[0m");
//    handle.OnPrintOut(handle, "\033[1;36m%-10s : %d bytes\n\033[0m", "kmalloc", _gstBufCntHead.u32BufferCntKmalloc);
//    handle.OnPrintOut(handle, "\033[1;36m%-10s : %d bytes\n\033[0m", "mma malloc", _gstBufCntHead.u32BufferCntMma);

    handle.OnPrintOut(handle, "Memory used:\n");
    handle.OnPrintOut(handle, "%-10s : %d bytes\n", "kmalloc", _gstBufCntHead.u32BufferCntKmalloc);
    handle.OnPrintOut(handle, "%-10s : %d bytes\n", "mma malloc", _gstBufCntHead.u32BufferCntMma);
    up(&_gStBufCntSem);
    handle.OnPrintOut(handle, "\n---------------------------------------- end dump info of buffers in used ------------------------------------------\n");

    return MI_SUCCESS;
}

static MI_S32 _MI_RGN_IMPL_DumpChannelPortInfo(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{

    MI_U8 i = 0;
    MI_RGN_DumpChnPortListHead_t *pstDumpListHead = NULL;
    MI_RGN_ChnPort_t stChPort;
    MI_RGN_ChnPortParam_t stChnPortPara;

    MI_RGN_HandleListData_t *pstHl = NULL;
    MI_RGN_WinowHandler_t *pstWl = NULL;
    MI_RGN_DumpChnPort_t *pstChnPort = NULL;
    MI_RGN_DumpChnPort_t *pstChnPortN = NULL;
    MI_RGN_DumpChnPortListHead_t *pos = NULL, *posN = NULL;

    down(&_gstRgnListSem);
    list_for_each_entry(pstHl, &_stRegionlist.handleList, handleList)
    {
        list_for_each_entry(pstWl, &pstHl->handleToWinHandler, handleToWinHandler)
        {
            _stRegionlist.stWinManger.fpGetChPort(pstWl->hWinHandler, &stChPort);
            _stRegionlist.stWinManger.fpGetAttr(pstWl->hWinHandler, &stChnPortPara);

            MI_RGN_MEMALLOC(pstChnPort, sizeof(MI_RGN_DumpChnPort_t));
            MI_SYS_BUG_ON(!pstChnPort);
            pstChnPort->hHandle = pstHl->hHandle;
            memcpy(&pstChnPort->stRgnAttr, &pstHl->stRegAttr, sizeof(MI_RGN_Attr_t));
            memcpy(&pstChnPort->stBackgroundBuf, &pstHl->stCanvasInfo, sizeof(MI_RGN_CanvasInfo_t));
            memcpy(&pstChnPort->stChnPortPara, &stChnPortPara, sizeof(MI_RGN_ChnPortParam_t));
            _MI_RGN_IMPL_GetDumpListHead(&stChPort, &pstDumpListHead);
            list_add_tail(&pstChnPort->stDumpList, &pstDumpListHead->stDumpList);
        }
    }
    up(&_gstRgnListSem);

    list_for_each_entry_safe(pos, posN, &_gDumpListHead, stListHead)
    {
        handle.OnPrintOut(handle, "\n--------------------------------------- start dump %s Channel %d port %d info ---------------------------------------\n",
        (pos->stChnPort.eModId)?"Divp":"Vpe", pos->stChnPort.s32ChnId, pos->stChnPort.s32OutputPortId);
        handle.OnPrintOut(handle, "\n--------------------------------------------------- Basw Window ----------------------------------------------------\n");
        handle.OnPrintOut(handle, "%11s%11s\n%11d%11d\n", "Width", "Height", pos->stBaseWindow.u32Width, pos->stBaseWindow.u32Height);
        handle.OnPrintOut(handle, "\n-------------------------------------------------- frontbuff info --------------------------------------------------\n");
        handle.OnPrintOut(handle, "%11s%11s%11s%11s%11s%11s%11s%11s%11s%11s\n", "Index", "bShow", "OffsetX", "OffsetY", "Width",
                                    "Height", "Stride", "Format", "VirAddr", "PhyAddr");
        for (i = 0; i < pos->u8FrontBufferCount; i++)
        {
            handle.OnPrintOut(handle, "%11d%11d%11d%11d%11d%11d%11d%11d%11x%11llx\n", i, pos->pstFrontBuffer[i].bShow, pos->pstFrontBuffer[i].stPoint.u32X,
            pos->pstFrontBuffer[i].stPoint.u32Y, pos->pstFrontBuffer[i].stFrontBuf.stSize.u32Width, pos->pstFrontBuffer[i].stFrontBuf.stSize.u32Height,
            pos->pstFrontBuffer[i].stFrontBuf.u32Stride, pos->pstFrontBuffer[i].stFrontBuf.ePixelFmt, pos->pstFrontBuffer[i].stFrontBuf.virtAddr,
            pos->pstFrontBuffer[i].stFrontBuf.phyAddr);
        }
        MI_RGN_MEMFREE(pos->pstFrontBuffer);
        handle.OnPrintOut(handle, "\n--------------------------------------------------- region info ----------------------------------------------------\n");
        handle.OnPrintOut(handle, "%8s%7s%7s%7s%8s%8s%10s%10s%10s%10s%8s%10s%10s\n", "Handle", "Type", "bShow", "Layer",
                                "Color", "Width", "Height", "Stride", "PositionX", "PositionY", "Format", "VirAddr", "PhyAddr");
        list_for_each_entry_safe(pstChnPort, pstChnPortN, &pos->stDumpList, stDumpList)
        {
            switch (pstChnPort->stRgnAttr.eType)
            {
                case E_MI_RGN_TYPE_OSD:
                {
                    handle.OnPrintOut(handle, "%8d%7s%7d%7s%8s%8d%10d%10d%10d%10d%8d%10x%10llx\n", pstChnPort->hHandle, "OSD",
                        pstChnPort->stChnPortPara.bShow, "", "", pstChnPort->stRgnAttr.stOsdInitParam.stSize.u32Width,
                        pstChnPort->stRgnAttr.stOsdInitParam.stSize.u32Height, pstChnPort->stBackgroundBuf.u32Stride, pstChnPort->stChnPortPara.stPoint.u32X,
                        pstChnPort->stChnPortPara.stPoint.u32Y, pstChnPort->stRgnAttr.stOsdInitParam.ePixelFmt, pstChnPort->stBackgroundBuf.virtAddr,
                        pstChnPort->stBackgroundBuf.phyAddr);
                }
                break;
                case E_MI_RGN_TYPE_COVER:
                {
                    handle.OnPrintOut(handle, "%8d%7s%7d%7d%8x%8d%10d%10s%10d%10d%8s%10s%10s\n", pstChnPort->hHandle, "COVER",
                        pstChnPort->stChnPortPara.bShow, pstChnPort->stChnPortPara.stCoverPara.u32Layer, pstChnPort->stChnPortPara.stCoverPara.u32Color,
                        pstChnPort->stChnPortPara.stCoverPara.stSize.u32Width, pstChnPort->stChnPortPara.stCoverPara.stSize.u32Height, "",
                        pstChnPort->stChnPortPara.stPoint.u32X, pstChnPort->stChnPortPara.stPoint.u32Y, "", "", "");
                }
                break;
                default:
                    break;
            }
            list_del(&pstChnPort->stDumpList);
            MI_RGN_MEMFREE(pstChnPort);
        }
        list_del(&pos->stListHead);
        MI_RGN_MEMFREE(pos);
        handle.OnPrintOut(handle, "\n--------------------------------------- end dump %s Channel %d port %d info ----------------------------------------\n",
        (pos->stChnPort.eModId)?"Divp":"Vpe", pos->stChnPort.s32ChnId, pos->stChnPort.s32OutputPortId);
    }

    return MI_SUCCESS;

}
static MI_S32 _MI_RGN_IMPL_DumpPalette(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_RGN_PaletteTable_t *pstPalette;
    MI_U16 u16Idx;

    MI_RGN_MEMALLOC(pstPalette, sizeof(MI_RGN_PaletteTable_t));
    MI_SYS_BUG_ON(!pstPalette);
    handle.OnPrintOut(handle, "\n------------------------------------- start dump palette--------------------------------------------------------------\n");
    handle.OnPrintOut(handle, "%8s%8s%8s%8s%8s\n", "Index", "Alpha", "Red", "Green", "Blue");
    _stRegionlist.stWinManger.fpGetPalette(pstPalette);
    for (u16Idx = 0; u16Idx < MI_RGN_MAX_PALETTE_TABLE_NUM; u16Idx++)
    {
        handle.OnPrintOut(handle, "%8d%8x%8x%8x%8x\n", u16Idx, pstPalette->astElement[u16Idx].u8Alpha, pstPalette->astElement[u16Idx].u8Red, pstPalette->astElement[u16Idx].u8Green, pstPalette->astElement[u16Idx].u8Blue);
    }
    handle.OnPrintOut(handle, "\n------------------------------------- end dump palette--------------------------------------------------------------\n");
    MI_RGN_MEMFREE(pstPalette);

    return MI_SUCCESS;
}
#if MI_RGN_PROC_DUMP_PROCESS_DURITION

static MI_U64 _MI_RGN_IMPL_GetCurTime(void)
{
    struct timespec sttime;
    MI_U64 u64Value;

    memset(&sttime, 0, sizeof(sttime));
    ktime_get_ts(&sttime);
    u64Value = timespec_to_ns(&sttime);

    return u64Value;
}
/*argc[1] = 0 clear max time 1, dump current.*/
static MI_S32 _MI_RGN_IMPL_ProcessDurationTime(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_U8 i = 0;


    down(&_gStRgnStateSem);
    if (simple_strtoul(argv[1], NULL, 0))
    {
        handle.OnPrintOut(handle, "\n------------------------------------- start get process dur time-----------------------------------------------------\n");
        for (i = 0 ; i < MI_RGN_VPE_PORT_MAXNUM; i++)
        {
            handle.OnPrintOut(handle, "Current info:\n");
            handle.OnPrintOut(handle, "%8s%8s%8s %8s\n", "Mode", "Channel", "Port", "Time diff");
            handle.OnPrintOut(handle, "%8s%8d%8d %8u ns\n", "VPE", stVpeCur[i].u32Channel, i, stVpeCur[i].u32TimeDiff);
            handle.OnPrintOut(handle, "Max info:\n");
            handle.OnPrintOut(handle, "%8s%8s%8s %8s\n", "Mode", "Channel", "Port", "Time diff");
            handle.OnPrintOut(handle, "%8s%8d%8d %8u ns\n", "VPE", stVpeMax[i].u32Channel, i, stVpeMax[i].u32TimeDiff);

        }
        for (i = 0 ; i < MI_RGN_DIVP_PORT_MAXNUM; i++)
        {
            handle.OnPrintOut(handle, "Current info:\n");
            handle.OnPrintOut(handle, "%8s%8s%8s %8s\n", "Mode", "Channel", "Port", "Time diff");
            handle.OnPrintOut(handle, "%8s%8d%8d %8u ns\n", "DIVP", stDivpCur[i].u32Channel, i, stDivpCur[i].u32TimeDiff);
            handle.OnPrintOut(handle, "Max info:\n");
            handle.OnPrintOut(handle, "%8s%8s%8s %8s\n", "Mode", "Channel", "Port", "Time diff");
            handle.OnPrintOut(handle, "%8s%8d%8d %8u ns\n", "DIVP", stDivpCur[i].u32Channel, i, stDivpCur[i].u32TimeDiff);
        }
        handle.OnPrintOut(handle, "\n------------------------------------- end get process dur time-------------------------------------------------------\n");
    }
    else
    {
        memset(stVpeMax, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_VPE_PORT_MAXNUM);
        memset(stDivpCur, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_DIVP_PORT_MAXNUM);
    }
    up(&_gStRgnStateSem);

    return MI_SUCCESS;
}
#endif
static MI_S32 _MI_RGN_IMPL_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId,void *pUsrData)
{

    _MI_RGN_IMPL_GetCapability(handle, u32DevId, 0, NULL, pUsrData);
    _MI_RGN_IMPL_DumpRegionAttr(handle, u32DevId, 0, NULL, pUsrData);
    _MI_RGN_IMPL_DumpChannelPortInfo(handle, u32DevId, 0, NULL, pUsrData);
    _MI_RGN_IMPL_DumpBufUsedInfo(handle, u32DevId, 0, NULL, pUsrData);

    return MI_SUCCESS;
}
static MI_S32 _MI_RGN_IMPL_OnHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_U32 u32Data = 6;

    handle.OnPrintOut(handle,"Hello %d %s %x %p %u %ld %lu %lx %llx\n", u32Data, "haha", u32Data, &u32Data, u32Data, (long)u32Data, (long unsigned int)u32Data, (long unsigned int)u32Data, (MI_U64)u32Data);
    return MI_SUCCESS;
}
#endif

MI_S32 MI_RGN_IMPL_Init(MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_RGN_PaletteTable_t __user *pstUsrPaletteTable = NULL;
    MI_RGN_PaletteTable_t *pstPaletteTableAlloc = NULL;
#if MI_RGN_MODULE_TEST
    MHAL_CMDQ_CmdqInterface_t cmdinf;
    mi_rgn_ProcessCmdInfo_t stProcessCmdInfo;
    MI_RGN_ModId_e  eModId = E_MI_RGN_MODID_VPE;
    MI_U64 u64Fence = 0;
    MI_U8 i = 0;
#endif

    MI_RGN_PTR_CHECK(pstPaletteTable, return MI_ERR_RGN_NULL_PTR;);
    if (_stRegionlist.bInitFlag == TRUE)
    {
        return MI_ERR_RGN_BUSY;
    }
    pstUsrPaletteTable = pstPaletteTable;
    if (!access_ok(VERIFY_READ, pstUsrPaletteTable, sizeof(MI_RGN_PaletteTable_t)))
    {
        DBG_ERR("palette point error!\n");
        return MI_ERR_RGN_NULL_PTR;
    }
    MI_RGN_MEMALLOC(pstPaletteTableAlloc, sizeof(MI_RGN_PaletteTable_t));
    MI_SYS_BUG_ON(!pstPaletteTableAlloc);
    copy_from_user(pstPaletteTableAlloc, pstUsrPaletteTable, sizeof(MI_RGN_PaletteTable_t));

#if MI_RGN_MODULE_TEST
    if (!bUseInitFlag)
    {
        _stRegionlist.bInitFlag = TRUE;
        _stRegionlist.stWinManger.fpSetPalette(pstPaletteTableAlloc);
        memset(&_gstCapablity, 0, sizeof(mi_rgn_DrvCapability_t));
        _stRegionlist.stWinManger.fpGetCapability(&_gstCapablity);
        bUseInitFlag = TRUE;
    }
    else
    {
        memset(&cmdinf, 0, sizeof(MHAL_CMDQ_CmdqInterface_t));
        memset(&stProcessCmdInfo, 0, sizeof(mi_rgn_ProcessCmdInfo_t));
        eModId = (MI_RGN_ModId_e)pstPaletteTableAlloc->astElement[15].u8Alpha;
        stProcessCmdInfo.u32chnID = (MI_U32)pstPaletteTableAlloc->astElement[15].u8Green;
        DBG_INFO("Mod id %d, chnid %d\n", eModId, stProcessCmdInfo.u32chnID);

        if (eModId == E_MI_RGN_MODID_VPE)
        {
            for (i = 0; i < _gstCapablity.u8VpeMaxPortNum; i++)
            {
                stProcessCmdInfo.stVpePort[i].bEnable = TRUE;
                stProcessCmdInfo.stVpePort[i].u32Width = 1920;
                stProcessCmdInfo.stVpePort[i].u32Height = 1080;
            }
            mi_rgn_VpeProcess(&stProcessCmdInfo, &cmdinf, &u64Fence);
            mi_rgn_NotifyFenceDone(u64Fence);
        }
        else if (eModId == E_MI_RGN_MODID_DIVP)
        {
            for (i = 0; i < _gstCapablity.u8DivpMaxPortNum; i++)
            {
                stProcessCmdInfo.stDivpPort[i].bEnable = TRUE;
                stProcessCmdInfo.stDivpPort[i].u32Width = 1920;
                stProcessCmdInfo.stDivpPort[i].u32Height = 1080;
            }
            mi_rgn_DivpProcess(&stProcessCmdInfo, &cmdinf, &u64Fence);
            mi_rgn_NotifyFenceDone(u64Fence);
        }
    }
#else
    _stRegionlist.bInitFlag = TRUE;
    _stRegionlist.stWinManger.fpSetPalette(pstPaletteTableAlloc);
    memset(&_gstCapablity, 0, sizeof(mi_rgn_DrvCapability_t));
    _stRegionlist.stWinManger.fpGetCapability(&_gstCapablity);
#endif
    MI_RGN_MEMFREE(pstPaletteTableAlloc);

    return MI_RGN_OK;
}
MI_S32 MI_RGN_IMPL_DeInit()
{
    if (_stRegionlist.bInitFlag == FALSE)
    {
#if MI_RGN_MODULE_TEST
        bUseInitFlag = FALSE;
#endif
        return MI_ERR_RGN_BUSY;
    }
    _stRegionlist.bInitFlag = FALSE;
#if MI_RGN_MODULE_TEST
    bUseInitFlag = FALSE;
#endif
    return MI_RGN_OK;
}

MI_S32 MI_RGN_IMPL_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    /*1.var define*/
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_S32 s32ErrorCode = MI_RGN_OK;

    DBG_ENTER();
    MI_RGN_PTR_CHECK(pstRegion, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);
    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    /*4.normal code*/

    down(&_gstRgnListSem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd != NULL)
    {
        DBG_EXIT_ERR("Handle is exist.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    if (_stRegionlist.u16ListCnt == MI_RGN_MAX_HANDLE)
    {
        DBG_EXIT_ERR("Handle num is max\n");
        s32ErrorCode = MI_ERR_RGN_NOMEM;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    /*Apply list node buffer & fill the data*/
    /*Tommy kmalloc*/
    MI_RGN_MEMALLOC(pstLd, sizeof(MI_RGN_HandleListData_t));
    MI_RGN_PTR_CHECK(pstLd, s32ErrorCode = MI_ERR_RGN_NOMEM;    \
                                up(&_gstRgnListSem);  \
                                goto ERROR_CODE;);
    memset(pstLd, 0, sizeof(MI_RGN_HandleListData_t));

    INIT_LIST_HEAD(&pstLd->handleToWinHandler);

    switch (pstRegion->eType)
    {
        case E_MI_RGN_TYPE_OSD:
        {
            /***************
                Apply buffer from mi_sys
            ***************/

            s32ErrorCode = _MI_RGN_IMPL_CheckRgnOsdAttr(&pstRegion->stOsdInitParam);
            if (MI_RGN_OK != s32ErrorCode)
            {
                DBG_EXIT_ERR("Check osd attr error!\n");
                s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
                MI_RGN_MEMFREE(pstLd);
                up(&_gstRgnListSem);
                goto ERROR_CODE;
            }
            pstLd->stCanvasInfo.ePixelFmt = pstRegion->stOsdInitParam.ePixelFmt;
            pstLd->stCanvasInfo.stSize.u32Height = pstRegion->stOsdInitParam.stSize.u32Height;
            pstLd->stCanvasInfo.stSize.u32Width = pstRegion->stOsdInitParam.stSize.u32Width;

            if (MI_RGN_OK != _stRegionlist.stBufManger.fpBufferAlloc(&pstLd->stCanvasInfo))
            {
                DBG_EXIT_ERR("MiSys alloc buffer error!\n");
                s32ErrorCode = MI_ERR_RGN_NOMEM;
                MI_RGN_MEMFREE(pstLd);
                up(&_gstRgnListSem);
                goto ERROR_CODE;
            }
            DBG_INFO("Canvas pa alloc %llx\n", pstLd->stCanvasInfo.phyAddr);
        }
        break;
        case E_MI_RGN_TYPE_COVER:
        {
            /***************
            todo:
                do cover case.
            ***************/
        }
        break;
        default:
        {
            DBG_EXIT_ERR("Region type error!\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            MI_RGN_MEMFREE(pstLd);
            up(&_gstRgnListSem);
            goto ERROR_CODE;
        }
        break;
    }
    pstLd->hHandle = hHandle;
    memcpy(&pstLd->stRegAttr,pstRegion,sizeof(MI_RGN_Attr_t));
    DBG_INFO("handle %d\n", pstLd->hHandle);
     list_add_tail(&pstLd->handleList, &_stRegionlist.handleList);
     _stRegionlist.u16ListCnt++;
    up(&_gstRgnListSem);

    DBG_EXIT_OK();
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;
}
MI_S32 MI_RGN_IMPL_Destroy(MI_RGN_HANDLE hHandle)
{
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL, *pstWhN = NULL;
    MI_S32 s32ErrorCode = MI_RGN_OK;

    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);

    /*1. Find each handle list if not return false*/
    down(&_gstRgnListSem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        up(&_gstRgnListSem);
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;
    }
    list_for_each_entry_safe(pstWh, pstWhN, &pstLd->handleToWinHandler, handleToWinHandler)
    {
        _stRegionlist.stWinManger.fpDestroy(pstWh->hWinHandler);
        list_del(&pstWh->handleToWinHandler);
        MI_RGN_MEMFREE(pstWh);
    }
    switch (pstLd->stRegAttr.eType)
    {
        case E_MI_RGN_TYPE_OSD:
        {
            _stRegionlist.stBufManger.fpBufferDel(&pstLd->stCanvasInfo);
        }
        break;
        case E_MI_RGN_TYPE_COVER:
        {
        }
        break;
        default:
            MI_SYS_BUG();
    }

    list_del(&pstLd->handleList);
    MI_RGN_MEMFREE(pstLd);
    _stRegionlist.u16ListCnt--;

    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;
}
MI_S32 MI_RGN_IMPL_GetAttr(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;

    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);
    MI_RGN_PTR_CHECK(pstRegion, return MI_ERR_RGN_NULL_PTR;);

    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd != NULL)
    {
        memcpy(pstRegion, &pstLd->stRegAttr, sizeof(MI_RGN_Attr_t));
    }
    else
    {
        DBG_EXIT_ERR("Handle not found.\n");
        up(&_gstRgnListSem);
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;
    }
    up(&_gstRgnListSem);

    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;
}

 MI_S32 MI_RGN_IMPL_SetBitMap(MI_RGN_HANDLE hHandle,  MI_RGN_Bitmap_t *pstBitmap)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;
    MI_U32 u32Height = 0;
    MI_U32 u32Width = 0;
    MI_U32 u32Bits = 0;
    MI_U32 u32CopyByte = 0;
    MI_U8 *pAddr = NULL;
    void * __user pUsrData = NULL;

    MI_U16 i = 0;

    /*1. Check handle avaliable. Check para.*/
    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstBitmap, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstBitmap->pData, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);

    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        up(&_gstRgnListSem);
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;

    }
    if (pstLd->stRegAttr.eType != E_MI_RGN_TYPE_OSD)
    {
        DBG_EXIT_ERR("Reg type error.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    if (pstLd->bGetCanvas)
    {
        DBG_WRN("Warning! GetCavas by another process, so ignore SetBitMap\n");
        up(&_gstRgnListSem);
        return MI_RGN_OK;
    }

    /*Translate to canvas info*/
    stCanvasInfo.ePixelFmt = pstBitmap->ePixelFormat;
    stCanvasInfo.stSize.u32Height = pstBitmap->stSize.u32Height;
    stCanvasInfo.stSize.u32Width = pstBitmap->stSize.u32Width;
    stCanvasInfo.virtAddr = (MI_VIRT)pstBitmap->pData;
    MI_RGN_BITS_PER_PIXEL(u32Bits, pstBitmap->stSize.u32Width, pstBitmap->ePixelFormat);
    stCanvasInfo.u32Stride =  ALIGN_UP(u32Bits, 8)/8;
    s32ErrorCode = _MI_RGN_IMPL_CheckCanvasBufAttr(&stCanvasInfo);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("ePixelFmt error!\n");
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    if (pstLd->stCanvasInfo.ePixelFmt != stCanvasInfo.ePixelFmt)
    {
        DBG_EXIT_ERR("Pix fmt not match.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    /*Calculate copy byte*/
    u32Width = MI_RGN_MIN(stCanvasInfo.stSize.u32Width, pstLd->stCanvasInfo.stSize.u32Width);
    u32Height = MI_RGN_MIN(stCanvasInfo.stSize.u32Height, pstLd->stCanvasInfo.stSize.u32Height);
    MI_RGN_BITS_PER_PIXEL(u32Bits, u32Width,  pstLd->stCanvasInfo.ePixelFmt);
    u32CopyByte = ALIGN_UP(u32Bits, 8)/8;

    /*Map buffer and check user buf*/
    pUsrData = (void *)stCanvasInfo.virtAddr; //From
    if (!access_ok(VERIFY_READ, pUsrData, stCanvasInfo.u32Stride * u32Height))
    {
        DBG_EXIT_ERR("User data error.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    pstLd->stCanvasInfo.virtAddr = (MI_VIRT)mi_sys_Vmap(pstLd->stCanvasInfo.phyAddr, pstLd->stCanvasInfo.u32Stride * u32Height, FALSE);
    pAddr = (MI_U8 *)pstLd->stCanvasInfo.virtAddr; //To
    if (NULL == pAddr)
    {
        DBG_EXIT_ERR("User data error.\n");
        s32ErrorCode = MI_ERR_RGN_NOMEM;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    /*Do copy*/
    for (i = 0; i < u32Height; i++)
    {
        copy_from_user(pAddr, pUsrData, u32CopyByte);
        pAddr += pstLd->stCanvasInfo.u32Stride;
        pUsrData += stCanvasInfo.u32Stride;
    }

    /*Unmap*/
    mi_sys_UnVmap((void *)pstLd->stCanvasInfo.virtAddr);
    pstLd->stCanvasInfo.virtAddr = 0;

    /*Do blit front buffer.*/
    list_for_each_entry(pstWh, &pstLd->handleToWinHandler, handleToWinHandler)
    {
        _stRegionlist.stWinManger.fpBlitOsdFrontBuffer(pstWh->hWinHandler, &pstLd->stCanvasInfo);
    }

    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:

    return s32ErrorCode;
}

MI_S32 MI_RGN_IMPL_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    DBG_ENTER();

    /*1. Check handle avaliable. Check para.*/
    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstCanvasInfo, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);

    down(&_gstRgnListSem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    if (pstLd->stRegAttr.eType == E_MI_RGN_TYPE_COVER)
    {
        DBG_EXIT_ERR("Region is cover.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    memcpy(pstCanvasInfo, &pstLd->stCanvasInfo, sizeof(MI_RGN_CanvasInfo_t));

    pstLd->bGetCanvas = TRUE;
    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_UpdateCanvas(MI_RGN_HANDLE hHandle)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;

    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_HANDLE_CHECK(hHandle, return MI_ERR_RGN_INVALID_HANDLE;);

    DBG_ENTER();

    down(&_gstRgnListSem);
    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    if (pstLd->stRegAttr.eType != E_MI_RGN_TYPE_OSD)
    {
        DBG_EXIT_ERR("Region is cover.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    list_for_each_entry(pstWh, &pstLd->handleToWinHandler, handleToWinHandler)
    {
        if (pstLd->stRegAttr.eType == E_MI_RGN_TYPE_OSD)
        {
            _stRegionlist.stWinManger.fpBlitOsdFrontBuffer(pstWh->hWinHandler, &pstLd->stCanvasInfo);
        }
    }

    pstLd->bGetCanvas = FALSE;
    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;

}

MI_S32 MI_RGN_IMPL_AttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort,  MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;

    /*1. Check handle avaliable. Check para.*/
    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR);
    MI_RGN_PTR_CHECK(pstChnAttr, return MI_ERR_RGN_NULL_PTR);

    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        goto ERROR_CODE;
    }

    s32ErrorCode = _MI_RGN_IMPL_GetWindowListData(pstLd, pstChnPort, &pstWh);
    if (MI_RGN_OK != s32ErrorCode || pstWh != NULL)
    {
        DBG_EXIT_ERR("Channel port had been attached.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_CheckChnPortAttr(pstLd->stRegAttr.eType, pstChnAttr);
    if (MI_RGN_OK != s32ErrorCode)
    {
        goto ERROR_CODE;
    }

     /*Create a chPortList data */

    MI_RGN_MEMALLOC(pstWh, sizeof(MI_RGN_WinowHandler_t));
    MI_RGN_PTR_CHECK(pstWh, s32ErrorCode = MI_ERR_RGN_NOMEM;goto ERROR_CODE;);
    memset(pstWh, 0, sizeof(MI_RGN_WinowHandler_t));
    /*Para init*/

    s32ErrorCode = _stRegionlist.stWinManger.fpCreate(&pstWh->hWinHandler, pstChnPort, pstChnAttr, &pstLd->stRegAttr, &pstLd->stCanvasInfo);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("Window create error.\n");
        goto WIN_CREATE_ERROR;
    }
    list_add_tail(&pstWh->handleToWinHandler, &pstLd->handleToWinHandler);
    DBG_INFO("Handle %d attached to mode %d channel %d port %d x %d y %d success\n", hHandle, pstChnPort->eModId, pstChnPort->s32ChnId, pstChnPort->s32OutputPortId, pstChnAttr->stPoint.u32X, pstChnAttr->stPoint.u32Y);
    up(&_gstRgnListSem);

    return MI_RGN_OK;

WIN_CREATE_ERROR:
    MI_RGN_MEMFREE(pstWh);
ERROR_CODE:
    up(&_gstRgnListSem);
    return s32ErrorCode;
}

MI_S32 MI_RGN_IMPL_DetachFromChn(MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t *pstChnPort)
{

    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;

    /*1. Check handle avaliable. Check para.*/
    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR;);

    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetWindowListData(pstLd, pstChnPort, &pstWh);
    if (MI_RGN_OK != s32ErrorCode || pstWh == NULL)
    {
        DBG_EXIT_ERR("pstChnPort and handle not matched.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    _stRegionlist.stWinManger.fpDestroy(pstWh->hWinHandler);
    list_del(&pstWh->handleToWinHandler);
    MI_RGN_MEMFREE(pstWh);
    DBG_INFO("Handle %d detached success mode %d channel %d port %d\n", hHandle, pstChnPort->eModId, pstChnPort->s32ChnId, pstChnPort->s32OutputPortId);
    up(&_gstRgnListSem);

    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_SetDisplayAttr(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{

    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;

    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstChnPortAttr, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR;);

    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_CheckChnPortAttr(pstLd->stRegAttr.eType, pstChnPortAttr);
    if (MI_RGN_OK != s32ErrorCode)
    {
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetWindowListData(pstLd, pstChnPort, &pstWh);
    if (MI_RGN_OK != s32ErrorCode || pstWh == NULL)
    {
        DBG_EXIT_ERR("pstChnPort and handle not matched.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    s32ErrorCode = _stRegionlist.stWinManger.fpSetAttr(pstWh->hWinHandler, pstChnPortAttr);
    if (MI_RGN_OK != s32ErrorCode)
    {
         DBG_EXIT_ERR("wapper set chn attr error.\n");
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }

    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;

}
MI_S32 MI_RGN_IMPL_GetDisplayAttr(MI_RGN_HANDLE hHandle,  MI_RGN_ChnPort_t *pstChnPort, MI_RGN_ChnPortParam_t *pstChnPortAttr)
{

    MI_S32 s32ErrorCode = MI_RGN_OK;
    MI_RGN_HandleListData_t *pstLd = NULL;
    MI_RGN_WinowHandler_t *pstWh = NULL;

    MI_RGN_INIT_CHECK(_stRegionlist.bInitFlag, return MI_ERR_RGN_NOTREADY;);
    MI_RGN_PTR_CHECK(pstChnPortAttr, return MI_ERR_RGN_NULL_PTR;);
    MI_RGN_PTR_CHECK(pstChnPort, return MI_ERR_RGN_NULL_PTR;);


    down(&_gstRgnListSem);

    s32ErrorCode = _MI_RGN_IMPL_GetHandleListData(hHandle, &pstLd);
    if (pstLd == NULL)
    {
        DBG_EXIT_ERR("Handle not found.\n");
        s32ErrorCode = MI_ERR_RGN_INVALID_HANDLE;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _MI_RGN_IMPL_GetWindowListData(pstLd, pstChnPort, &pstWh);
    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("pstChnPort and handle not matched.\n");
        s32ErrorCode = MI_ERR_RGN_BUSY;
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    s32ErrorCode = _stRegionlist.stWinManger.fpGetAttr(pstWh->hWinHandler, pstChnPortAttr);
    if (MI_RGN_OK != s32ErrorCode)
    {
        _stRegionlist.stWinManger.fpDestroy(pstWh->hWinHandler);
        DBG_EXIT_ERR("wapper set chn attr error.\n");
        up(&_gstRgnListSem);
        goto ERROR_CODE;
    }
    up(&_gstRgnListSem);
    return MI_RGN_OK;

ERROR_CODE:
    return s32ErrorCode;

}

/*bMode 1 for kmalloc 0 for mma alloc*/
void mi_rgn_MemAllocCnt(void *pAddr, MI_U32 u32Size, MI_BOOL bMode)
{
    mi_rgn_BufCntData_t *pstBufCnt = NULL;

    MI_RGN_PTR_CHECK(pAddr, return;);

    down(&_gStBufCntSem);
    pstBufCnt = kmalloc(sizeof(mi_rgn_BufCntData_t), GFP_ATOMIC);
    MI_RGN_PTR_CHECK(pstBufCnt, up(&_gStBufCntSem);return;);
    pstBufCnt->pAddr = pAddr;
    if (pAddr == NULL)
    {
        DBG_ERR("Buf NULL\n");
    }
    _gstBufCntHead.u32BufferCntKmalloc += sizeof(mi_rgn_BufCntData_t);
    if (bMode)
    {
        _gstBufCntHead.u32BufferCntKmalloc += u32Size;
    }
    else
    {
        _gstBufCntHead.u32BufferCntMma += u32Size;
    }
    pstBufCnt->u32Size = u32Size;
    list_add_tail(&pstBufCnt->listBufCnt, &_gstBufCntHead.listBufCnt);
    up(&_gStBufCntSem);
}
void mi_rgn_MemCntPrint(void)
{
    down(&_gStBufCntSem);
    if (!_gstBufCntHead.u32BufferCntKmalloc && !_gstBufCntHead.u32BufferCntMma)
    {
        printk("\033[5;36mOh yeah ! Region Buffer Clear ready!!!!\n\033[0m");
    }
    else
    {
        printk("\033[1;31mOops!!!!! Region buffer not clear ready kmalloc %d mma %d\n\033[0m", _gstBufCntHead.u32BufferCntKmalloc, _gstBufCntHead.u32BufferCntMma);
    }
    up(&_gStBufCntSem);
}
void mi_rgn_MemFreeCnt(void *pAddr, MI_BOOL bMode)
{
    mi_rgn_BufCntData_t *pos, *posN;

    down(&_gStBufCntSem);
    list_for_each_entry_safe(pos, posN, &_gstBufCntHead.listBufCnt, listBufCnt)
    {
        if (pos->pAddr == pAddr)
        {
            list_del(&pos->listBufCnt);
            _gstBufCntHead.u32BufferCntKmalloc -= sizeof(mi_rgn_BufCntData_t);
            if (bMode)
            {
                _gstBufCntHead.u32BufferCntKmalloc -= pos->u32Size;
            }
            else
            {
                _gstBufCntHead.u32BufferCntMma -= pos->u32Size;;
            }
            kfree(pos);
            break;
        }
    }
    if (&pos->listBufCnt == &_gstBufCntHead.listBufCnt)
    {
        DBG_ERR("Free buf error Bad addr is %x.\n", pAddr);
        MI_SYS_BUG();
    }
    up(&_gStBufCntSem);
}
MI_S32 mi_rgn_RegistDriver(mi_rgn_WindowManger_t * pstWm, mi_rgn_BufferManager_t *pstBm)
{
    MI_RGN_PTR_CHECK(pstWm, return MI_ERR_RGN_NULL_PTR;);

    down(&_gStRgnStateSem);
    memcpy(&_stRegionlist.stWinManger, pstWm, sizeof(mi_rgn_WindowManger_t));
    memcpy(&_stRegionlist.stBufManger, pstBm, sizeof(mi_rgn_BufferManager_t));

    _stRegionlist.bRegistManger = TRUE;
    up(&_gStRgnStateSem);

    return MI_RGN_OK;
}
MI_S32 mi_rgn_UnregistDriver(void)
{
    down(&_gStRgnStateSem);
    memset(&_stRegionlist.stWinManger, 0, sizeof(mi_rgn_WindowManger_t));
    memset(&_stRegionlist.stBufManger, 0, sizeof(mi_rgn_BufferManager_t));

    _stRegionlist.bRegistManger = FALSE;
    up(&_gStRgnStateSem);

    return MI_RGN_OK;
}


MI_S32 mi_rgn_NotifyFenceDone(MI_U64 u64Fence)
{
    MI_S32 s32ErrorCode = MI_RGN_OK;

    down(&_gStRgnStateSem);
    if (!_stRegionlist.bRegistManger || !_stRegionlist.bInitFlag) //Internal call without app use.
    {
        up(&_gStRgnStateSem);
        return MI_RGN_OK;
    }
    s32ErrorCode = _stRegionlist.stWinManger.fpDone(u64Fence);
    up(&_gStRgnStateSem);

    if (MI_RGN_OK != s32ErrorCode)
    {
        DBG_EXIT_ERR("stWinManger done error.\n");
        s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
        goto ERROR_CODE;
    }

ERROR_CODE:
    return s32ErrorCode;
}
MI_S32 mi_rgn_Init(void)
{

#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevBindOps_t stDevBindOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;

    memset(&stDevBindOps, 0, sizeof(mi_sys_ModuleDevBindOps_t));
    stDevBindOps.OnBindOutputPort   = NULL;
    stDevBindOps.OnUnBindOutputPort = NULL;
    memset(&stModInfo, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId          = E_MI_MODULE_ID_RGN;
    stModInfo.u32DevId           = 0;
    stModInfo.u32DevChnNum       = 0;
    stModInfo.u32InputPortNum    = 0;
    stModInfo.u32OutputPortNum   = 0;

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if (MI_RGN_PROCFS_DEBUG == 1)
    pstModuleProcfsOps.OnDumpDevAttr = _MI_RGN_IMPL_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = _MI_RGN_IMPL_OnHelp;
 #else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif

    hSysDrvHandle = mi_sys_RegisterDev(&stModInfo, &stDevBindOps, NULL, &pstModuleProcfsOps,MI_COMMON_GetSelfDir);
    if (!hSysDrvHandle)
    {
        DBG_EXIT_ERR("mi_sys_RegisterDev error.\n");
        return MI_ERR_RGN_INVALID_HANDLE;
    }

#if (MI_RGN_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("getcap", 0, _MI_RGN_IMPL_GetCapability, hSysDrvHandle);
    mi_sys_RegistCommand("dumprgn", 0, _MI_RGN_IMPL_DumpRegionAttr, hSysDrvHandle);
    mi_sys_RegistCommand("dumpchport", 0, _MI_RGN_IMPL_DumpChannelPortInfo, hSysDrvHandle);
    mi_sys_RegistCommand("bufcnt", 0, _MI_RGN_IMPL_DumpBufUsedInfo, hSysDrvHandle);
    mi_sys_RegistCommand("dumpRgnBuf", 2, _MI_RGN_IMPL_DumpRgnBufToFile, hSysDrvHandle);
    mi_sys_RegistCommand("dumpFrontBuf", 4, _MI_RGN_IMPL_DumpFrontBufToFile, hSysDrvHandle);
    mi_sys_RegistCommand("dumpPalette", 0, _MI_RGN_IMPL_DumpPalette, hSysDrvHandle);
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
    mi_sys_RegistCommand("processdur", 1, _MI_RGN_IMPL_ProcessDurationTime, hSysDrvHandle);
#endif
#endif
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
    memset(&stVpeCur, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_VPE_PORT_MAXNUM);
    memset(&stVpeMax, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_VPE_PORT_MAXNUM);
    memset(&stDivpCur, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_DIVP_PORT_MAXNUM);
    memset(&stDivpMax, 0, sizeof(MI_RGN_ProcessTimeCheck_t) * MI_RGN_DIVP_PORT_MAXNUM);
#endif

#endif

    return MI_RGN_OK;
}
MI_S32 mi_rgn_ClearRgnBuffer(void)
{
    MI_RGN_HandleListData_t *pstHl = NULL;
    MI_RGN_HandleListData_t *pstHlN = NULL;
    MI_RGN_WinowHandler_t *pstWl = NULL;
    MI_RGN_WinowHandler_t *pstWlN = NULL;

    down(&_gstRgnListSem);
    list_for_each_entry_safe(pstHl, pstHlN, &_stRegionlist.handleList, handleList)
    {
        list_for_each_entry_safe(pstWl, pstWlN, &pstHl->handleToWinHandler, handleToWinHandler)
        {
             _stRegionlist.stWinManger.fpDestroy(pstWl->hWinHandler);
             list_del(&pstWl->handleToWinHandler);
            MI_RGN_MEMFREE(pstWl);
        }
        if (pstHl->stRegAttr.eType == E_MI_RGN_TYPE_OSD)
        {
            _stRegionlist.stBufManger.fpBufferDel(&pstHl->stCanvasInfo);
        }
        list_del(&pstHl->handleList);
        MI_RGN_MEMFREE(pstHl);
    }
    up(&_gstRgnListSem);

    MI_RGN_IMPL_DeInit();

    return MI_RGN_OK;
}
MI_S32 mi_rgn_Deinit(void)
{

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_RGN_PROCFS_DEBUG == 1)
    if (hSysDrvHandle)
    {
        mi_sys_UnRegisterDev(hSysDrvHandle);
    }
#endif
    mi_rgn_MemCntPrint();

    return MI_RGN_OK;
}

MI_S32 mi_rgn_DivpProcess(mi_rgn_ProcessCmdInfo_t *pstCmdInfo, MHAL_CMDQ_CmdqInterface_t *cmdinf, MI_U64 *pu64Fence)
{
    MI_U8 i = 0;
    MI_RGN_Size_t stDispWindowSize;
    MI_RGN_ChnPort_t stChnPort;
    MI_S32 s32ErrorCode = MI_RGN_OK;
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
    MI_U64 u64CurTime = 0;
#endif

    //DBG_ENTER();
    MI_SYS_BUG_ON(!pstCmdInfo);
    down(&_gStRgnStateSem);
    if (!_stRegionlist.bRegistManger || !_stRegionlist.bInitFlag) //Internal call without app use.
    {
        up(&_gStRgnStateSem);
        return MI_RGN_OK;
    }
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));

    for (i = 0; i < MI_RGN_DIVP_PORT_MAXNUM; i++)
    {
        stDispWindowSize.u32Height = pstCmdInfo->stDivpPort[i].u32Height;
        stDispWindowSize.u32Width = pstCmdInfo->stDivpPort[i].u32Width;
        stChnPort.s32OutputPortId = i;
        stChnPort.s32ChnId = pstCmdInfo->u32chnID;
        stChnPort.eModId = E_MI_RGN_MODID_DIVP;
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
        u64CurTime = _MI_RGN_IMPL_GetCurTime();
#endif
        s32ErrorCode = _stRegionlist.stWinManger.fpUpdate(&stChnPort,  &stDispWindowSize, pstCmdInfo->stDivpPort[i].bEnable, cmdinf, pu64Fence);
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("_MI_RGN_IMPL_UpdateChPortListData error.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            goto ERROR_CODE;
        }
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
        stDivpCur[i].u32TimeDiff = (MI_U32)(_MI_RGN_IMPL_GetCurTime() - u64CurTime);
        stDivpCur[i].u32Channel = pstCmdInfo->u32chnID;
        if (stDivpCur[i].u32TimeDiff > stDivpMax[i].u32TimeDiff)
        {
            memcpy(&stDivpMax[i], &stDivpCur[i], sizeof(MI_RGN_ProcessTimeCheck_t));
        }
#endif
    }

ERROR_CODE:
    up(&_gStRgnStateSem);

    return s32ErrorCode;
}
MI_S32 mi_rgn_VpeProcess(mi_rgn_ProcessCmdInfo_t *pstCmdInfo, MHAL_CMDQ_CmdqInterface_t *cmdinf, MI_U64 *pu64Fence)
{
    MI_U8 i = 0;
    MI_RGN_Size_t stDispWindowSize;
    MI_RGN_ChnPort_t stChnPort;
    MI_S32 s32ErrorCode = MI_RGN_OK;
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
    MI_U64 u64CurTime = 0;
#endif

    //DBG_ENTER();
    MI_SYS_BUG_ON(!pstCmdInfo);
    down(&_gStRgnStateSem);
    if (!_stRegionlist.bRegistManger || !_stRegionlist.bInitFlag) //Internal call without app use.
    {
        up(&_gStRgnStateSem);
        return MI_RGN_OK;
    }
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));

    for (i = 0; i < MI_RGN_VPE_PORT_MAXNUM; i++)
    {
        stDispWindowSize.u32Height = pstCmdInfo->stVpePort[i].u32Height;
        stDispWindowSize.u32Width = pstCmdInfo->stVpePort[i].u32Width;
        stChnPort.s32OutputPortId = i;
        stChnPort.s32ChnId = pstCmdInfo->u32chnID;
        stChnPort.eModId = E_MI_RGN_MODID_VPE;
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
        u64CurTime = _MI_RGN_IMPL_GetCurTime();
#endif
        s32ErrorCode = _stRegionlist.stWinManger.fpUpdate(&stChnPort,  &stDispWindowSize, pstCmdInfo->stVpePort[i].bEnable, cmdinf, pu64Fence);
        if (MI_RGN_OK != s32ErrorCode)
        {
            DBG_EXIT_ERR("_MI_RGN_IMPL_UpdateChPortListData error.\n");
            s32ErrorCode = MI_ERR_RGN_ILLEGAL_PARAM;
            goto ERROR_CODE;
        }
#if MI_RGN_PROC_DUMP_PROCESS_DURITION
        stVpeCur[i].u32TimeDiff = (MI_U32)(_MI_RGN_IMPL_GetCurTime() - u64CurTime);
        stVpeCur[i].u32Channel = pstCmdInfo->u32chnID;
        if (stVpeCur[i].u32TimeDiff > stVpeMax[i].u32TimeDiff)
        {
            memcpy(&stVpeMax[i], &stVpeCur[i], sizeof(MI_RGN_ProcessTimeCheck_t));
        }
#endif
    }

ERROR_CODE:
    up(&_gStRgnStateSem);

    return s32ErrorCode;
}

MI_S32 mi_rgn_OverlayProcess(MI_SYS_FrameData_t* pFrmData, MI_U8 u8GfxColorFmtTo, MI_U8 u8ChnId, MI_U8 u8PortId, MI_RGN_ModId_e eModId)
{
    MI_RGN_CanvasInfo_t stCanvasTo;
    MI_RGN_ChnPort_t stChnPort;
    MI_U8 i = 0;

    down(&_gStRgnStateSem);
    if (!_stRegionlist.bRegistManger || !_stRegionlist.bInitFlag) //Internal call without app use.
    {
        up(&_gStRgnStateSem);
        return MI_RGN_OK;
    }

    if (!pFrmData)
    {
        up(&_gStRgnStateSem);
        return MI_ERR_RGN_NULL_PTR;
    }

    memset(&stCanvasTo, 0, sizeof(stCanvasTo));
    memset(&stChnPort, 0, sizeof(stChnPort));

    stCanvasTo.phyAddr = pFrmData->phyAddr[0];
    stCanvasTo.u32Stride = pFrmData->u32Stride[0];
    stCanvasTo.stSize.u32Width = pFrmData->u16Width;
    stCanvasTo.stSize.u32Height = pFrmData->u16Height;
    stChnPort.eModId = eModId;
    stChnPort.s32ChnId = u8ChnId;
    stChnPort.s32OutputPortId = u8PortId;

    for (i = 0; i < _gstCapablity.u8OsdHwModeCnt; i++)
    {
        _stRegionlist.stBufManger.fpOverlayBlit(u8GfxColorFmtTo, &stCanvasTo, &stChnPort, i);
    }

    up(&_gStRgnStateSem);

    return MI_RGN_OK;
}


EXPORT_SYMBOL(mi_rgn_VpeProcess);
EXPORT_SYMBOL(mi_rgn_DivpProcess);
EXPORT_SYMBOL(mi_rgn_OverlayProcess);
EXPORT_SYMBOL(mi_rgn_NotifyFenceDone);

