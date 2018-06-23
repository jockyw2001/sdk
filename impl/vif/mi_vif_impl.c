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
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "mi_common.h"
#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mi_vif.h"
#include "mi_vif_datatype.h"
#include "mi_vif_internal.h"
#include "mi_vif_impl.h"
#include "mi_gfx.h"
#include "mhal_vif_datatype.h"
#include "mhal_vif.h"
#include "mi_sys_proc_fs_internal.h"

#include "../sys/mi_sys_buf_mgr.h"

#define  eNotifyState stFrameData.eCompressMode
#define  E_MI_SYS_FRAME_NOTIFY_QUEUED E_MI_SYS_COMPRESS_MODE_NONE
#define  E_MI_SYS_FRAME_NOTIFY_NEED_NOTIFY E_MI_SYS_COMPRESS_MODE_SEG
#define  E_MI_SYS_FRAME_NOTIFY_NOTIFIED E_MI_SYS_COMPRESS_MODE_LINE
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MI_VIF_TIMEOUT_NUM 10
#define MI_VIF_PORT_MAX_BUFFER 4
#define MI_VIF_BUFFER_MAGIC   0xFFFFFFFF

// for debug
#define MI_VIF_PORT_BUFFER_LIST 1
#define MI_VIF_TIMEOUT_CHECK 1
#define MI_VIF_FRAME_CHECK 0
#define MI_VIF_FRAME_DUMP 1

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------
#define MI_VIF_CHECK_POINTER(pointer)\
    {\
        if(pointer == NULL)\
        {\
            DBG_ERR("NULL pointer!!!\n");\
            return MI_ERR_VIF_INVALID_NULL_PTR;\
        }\
    }

#define MI_VIF_CHECK_PARA_RANGE(x, a, b,e) \
    {   \
        if(x < a)  \
        {   \
            DBG_ERR("%d para too small [%d %d]!\n",x,a,b);    \
            return e;   \
        }   \
        else if(x >= b)  \
        {   \
            DBG_ERR("%d para too big! [%d %d]\n",x,a,b);    \
            return e;   \
        }   \
    }

enum
{
    E_MI_VIF_TIME_BEGIN,
    E_MI_VIF_TIME_VIF_PROCESS_IN = E_MI_VIF_TIME_BEGIN,
    E_MI_VIF_TIME_VIF_PROCESS_OUT,
    E_MI_VIF_TIME_SYS_GET_BUF,
    E_MI_VIF_TIME_SYS_FINISH_BUF,
    E_MI_VIF_TIME_SYS_REWIND_BUF,
    E_MI_VIF_TIME_MHAL_QUEUE_BUF,
    E_MI_VIF_TIME_NUM
};

typedef struct
{
    MI_S8* ps8Name;
    MI_U32 u32TimeoutCount;
    MI_U32 u32TimeThreshold;
    MI_U32 u32Time[MI_VIF_TIMEOUT_NUM];
} mi_vif_timeout_t;

typedef struct mi_vif_PortContext_s
{
    MI_BOOL bEnable;
    MI_VIF_ChnPortAttr_t stChnPortAttr;
    MI_SYS_ChnPort_t stSysPort;

    MI_U32 u32FrameCount;
    MI_U32 u32FrameCountJiffies;
    MI_U32 u32FrameRate;
    MI_U32 u32ProcessInCount;
    MI_U32 u32ProcessOutCount;
    MI_U32 u32HalBufferHold;

#if (MI_VIF_FRAME_DUMP == 1)
    MI_BOOL bDumpFrame;
    MI_S8 s8DumpPath[32];
#endif

#if (MI_VIF_TIMEOUT_CHECK > 0)
    MI_BOOL bCheckTimeOut;
#endif

#if (MI_VIF_FRAME_CHECK == 1)
    MI_BOOL bCheckFrame;
    MI_U32 u32CheckFrameFailCount;
#endif

#if (MI_VIF_PORT_BUFFER_LIST == 1)
    struct list_head stBufferList;
#endif
} mi_vif_PortContext_t;

typedef struct mi_vif_ChnContext_s
{
    mi_vif_PortContext_t stPortCtx[MI_VIF_MAX_CHN_OUTPORT];
    struct mutex stLock;
} mi_vif_ChnContext_t;

typedef struct mi_vif_DevContext_s
{
    MI_VIF_DevAttr_t stDevAttr;
    MI_BOOL bEnable;
} mi_vif_DevContext_t;

typedef struct MI_VIF_BufTrackInfo_s
{
    struct list_head list;
    MI_SYS_BufInfo_t* pstBufInfo;
} mi_vif_BufTrackInfo_t;

static MI_BOOL gbVifInit;
static MI_BOOL gbVifDumpDmaBank;
static MI_BOOL gbVifDumpChnBank;
static MI_BOOL gbVifDumpClockBank;
static mi_vif_DevContext_t gstVifDevCtx[MI_VIF_MAX_DEV_NUM];
static mi_vif_ChnContext_t gstVifChnCtx[MI_VIF_MAX_PHYCHN_NUM];
static MI_SYS_DRV_HANDLE hVifHandle;
static struct task_struct *gpstVifWorkTask;
static struct completion gstVifProcessWakeup;
struct kmem_cache *gstpVifBufferCache;
static struct timer_list gstVifTimer;

#if (MI_VIF_TIMEOUT_CHECK > 0)
    static mi_vif_timeout_t gstTimeout[E_MI_VIF_TIME_NUM];
#endif

static void _mi_vif_BuffListAdd(mi_vif_PortContext_t* pPortCtx, MI_SYS_BufInfo_t* pstBufInfo)
{
#if (MI_VIF_PORT_BUFFER_LIST == 1)
    mi_vif_BufTrackInfo_t *pstBufTrackInfo;

    pstBufTrackInfo = kmem_cache_zalloc(gstpVifBufferCache, GFP_KERNEL);
    pstBufTrackInfo->pstBufInfo = pstBufInfo;
    list_add_tail(&pstBufTrackInfo->list, &pPortCtx->stBufferList);
#endif
}

static void _mi_vif_BuffListDel(mi_vif_PortContext_t* pPortCtx, MI_SYS_BufInfo_t *pstBufInfo)
{
#if (MI_VIF_PORT_BUFFER_LIST == 1)
    mi_vif_BufTrackInfo_t *pstBufTrackInfo, *tmp;
    MI_BOOL bFound = FALSE;

    //DBG_INFO("\n");
    //printk("del %p begin\n",pstBufInfo);

    list_for_each_entry_safe(pstBufTrackInfo, tmp, &pPortCtx->stBufferList, list)
    {
        //DBG_INFO("\n");
        //DBG_INFO("%s pstBufInfo:%p\n",__FUNCTION__,pstBufInfo);
        if(pstBufTrackInfo->pstBufInfo == pstBufInfo)
        {
            list_del_init(&pstBufTrackInfo->list);
            kmem_cache_free(gstpVifBufferCache, pstBufTrackInfo);
            bFound = TRUE;
        }

        //DBG_INFO("\n");
    }

    if(!bFound)
    {
        DBG_ERR("%s can not find %p\n", __FUNCTION__, pstBufInfo);
        MI_SYS_BUG();
    }
    else
    {
        //printk("del %p end\n",pstBufInfo);
    }

    //DBG_INFO("out\n");
#endif
}

MI_S32 _mi_vif_CheckDevAttr(MI_VIF_DevAttr_t* pstDevAttr)
{
    MI_VIF_CHECK_POINTER(pstDevAttr);
    MI_VIF_CHECK_PARA_RANGE(pstDevAttr->eIntfMode, E_MI_VIF_MODE_BT656, E_MI_VIF_MODE_MAX, MI_ERR_VIF_INVALID_PARA);
    MI_VIF_CHECK_PARA_RANGE(pstDevAttr->eWorkMode, E_MI_VIF_WORK_MODE_1MULTIPLEX, E_MI_VIF_WORK_MODE_MAX, MI_ERR_VIF_INVALID_PARA);
    MI_VIF_CHECK_PARA_RANGE(pstDevAttr->eClkEdge, E_MI_VIF_CLK_EDGE_SINGLE_UP, E_MI_VIF_CLK_EDGE_MAX, MI_ERR_VIF_INVALID_PARA);

    if(pstDevAttr->eIntfMode == E_MI_VIF_MODE_DIGITAL_CAMERA)
    {
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->eDataSeq, E_MI_VIF_INPUT_DATA_VUVU, E_MI_VIF_INPUT_DATA_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eVsync, E_MI_VIF_VSYNC_FIELD, E_MI_VIF_VSYNC_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eVsyncNeg, E_MI_VIF_VSYNC_NEG_HIGH, E_MI_VIF_VSYNC_NEG_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eHsync, E_MI_VIF_HSYNC_VALID_SINGNAL, E_MI_VIF_HSYNC_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eHsyncNeg, E_MI_VIF_HSYNC_NEG_HIGH, E_MI_VIF_HSYNC_NEG_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eVsyncValid, E_MI_VIF_VSYNC_NORM_PULSE, E_MI_VIF_VSYNC_VALID_MAX, MI_ERR_VIF_INVALID_PARA);
        MI_VIF_CHECK_PARA_RANGE(pstDevAttr->stSynCfg.eVsyncValidNeg, E_MI_VIF_VSYNC_VALID_NEG_HIGH, E_MI_VIF_VSYNC_VALID_NEG_MAX, MI_ERR_VIF_INVALID_PARA);
    }

    return MI_SUCCESS;
}

MI_S32 _mi_vif_CheckChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t* pstAttr)
{
    MI_VIF_CHECK_PARA_RANGE(pstAttr->eCapSel, 0, E_MI_SYS_FIELDTYPE_NUM, MI_ERR_VIF_INVALID_PARA);
    MI_VIF_CHECK_PARA_RANGE(pstAttr->eScanMode, 0, E_MI_SYS_FRAME_SCAN_MODE_MAX, MI_ERR_VIF_INVALID_PARA);
    MI_VIF_CHECK_PARA_RANGE(pstAttr->ePixFormat, 0, E_MI_SYS_PIXEL_FRAME_FORMAT_MAX, MI_ERR_VIF_INVALID_PARA);
    MI_VIF_CHECK_PARA_RANGE(pstAttr->eFrameRate, 0, E_MI_VIF_FRAMERATE_MAX, MI_ERR_VIF_INVALID_PARA);

    if(pstAttr->bMirror)                   return MI_ERR_VIF_NOT_SUPPORT;

    if(pstAttr->bFlip)                     return MI_ERR_VIF_NOT_SUPPORT;

    if(pstAttr->stCapRect.u16X & 0x3)        return MI_ERR_VIF_INVALID_PARA;

    if(pstAttr->stCapRect.u16Y & 0x3)        return MI_ERR_VIF_INVALID_PARA;

    if(pstAttr->stCapRect.u16Width & 0x3)    return MI_ERR_VIF_INVALID_PARA;

    if(pstAttr->stCapRect.u16Width < 32)   return MI_ERR_VIF_INVALID_PARA;

    if(pstAttr->stCapRect.u16Height & 0x3)   return MI_ERR_VIF_INVALID_PARA;

    if(pstAttr->stCapRect.u16Height < 32)  return MI_ERR_VIF_INVALID_PARA;

    //MI_VIF_CHECK_PARA_RANGE(pstAttr->stCapRect.u32Width , 0, __SENSOR_WIDTH_, MI_ERR_VIF_INVALID_PARA);
    //MI_VIF_CHECK_PARA_RANGE(pstAttr->stCapRect.u32Height , 0, __SENSOR_HEIGHT_, MI_ERR_VIF_INVALID_PARA);

    if(u32ChnPort == 0)
    {
        if(pstAttr->stCapRect.u16Height != pstAttr->stDestSize.u16Height)
        {
            DBG_ERR("Port 0 stCapRect.u32Height must equal stDestSize.u32Height!\n");
            return MI_ERR_VIF_INVALID_PARA;
        }

        if(pstAttr->stCapRect.u16Width != pstAttr->stDestSize.u16Width)
        {
            DBG_ERR("Port 0 stCapRect.u32Width must equal stDestSize.u32Width!\n");
            return MI_ERR_VIF_INVALID_PARA;
        }
    }
    else  if(u32ChnPort == 1)
    {
#if 0
        mi_vif_ChnContext_t* pChnCtx    = &gstVifChnCtx[u32VifChn];

        mi_vif_PortContext_t* pPort0Ctx = &pChnCtx->stPortCtx[0];

        if(pPort0Ctx->stChnPortAttr.stDestSize.u32Width / 2 != pstAttr->stDestSize.u32Width)
        {
            DBG_ERR("port 1 u32Width must be half of port 0 , %d %d!\n", pPort0Ctx->stChnPortAttr.stDestSize.u32Width / 2, pstAttr->stDestSize.u32Width);
            return MI_ERR_VIF_INVALID_PARA;
        }

        if(pPort0Ctx->stChnPortAttr.stDestSize.u32Height / 2 != pstAttr->stDestSize.u32Height)
        {
            DBG_ERR("port 1 u32Height must be half of port 0, %d %d!\n", pPort0Ctx->stChnPortAttr.stDestSize.u32Height / 2, pstAttr->stDestSize.u32Height);
            return MI_ERR_VIF_INVALID_PARA;
        }

#endif
    }

    return MI_SUCCESS;
}

int _mi_vif_write_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t* pstFrameData)
{
    int width, height;
    char *dst_buf;
    int i;
    mm_segment_t fs = get_fs();


    MI_SYS_BUG_ON(pstFrameData->pVirAddr[0] == NULL);

    if(IS_ERR(filehandle)) //for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n", __FUNCTION__, __LINE__, PTR_ERR(filehandle)); //base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return -1;
    }

    if(pstFrameData->ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && pstFrameData->ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 &&
            (pstFrameData->ePixelFormat != E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_RG))
    {
        printk("%s:%d ePixelFormat %d not support!\n", __FUNCTION__, __LINE__, pstFrameData->ePixelFormat);
        return -1;
    }

    width = pstFrameData->u16Width;
    height = pstFrameData->u16Height;
    set_fs(KERNEL_DS);

    switch(pstFrameData->ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            dst_buf = pstFrameData->pVirAddr[0];

            for(i = 0; i < height; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width * 2, &filehandle->f_pos) != width * 2)
                    goto ERR_RET;

                dst_buf += pstFrameData->u32Stride[0];
            }

            return 0;

        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            dst_buf = pstFrameData->pVirAddr[0];

            for(i = 0; i < height; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width, &filehandle->f_pos) != width)
                    goto ERR_RET;

                dst_buf += pstFrameData->u32Stride[0];
            }

            dst_buf = pstFrameData->pVirAddr[1];

            for(i = 0; i < height / 2; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width, &filehandle->f_pos) != width)
                    goto ERR_RET;

                dst_buf += pstFrameData->u32Stride[1];
            }

            set_fs(fs);
            return 0;

        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_RG:
            dst_buf = pstFrameData->pVirAddr[0];
            i = vfs_write(filehandle, (char *)dst_buf, width * height * 2, &filehandle->f_pos);

            if(i != width * height * 2)
                goto ERR_RET;

            return 0;

        default:
            printk("%s:%d   not support framedata.ePixelFormat=%d\n", __FUNCTION__, __LINE__, pstFrameData->ePixelFormat);
            return -1;
    }

ERR_RET:
    set_fs(fs);
    printk("%s:%d fail\n", __FUNCTION__, __LINE__);
    return -1;
}


#include "../sys/mi_sys_buf_mgr.h"


void _mi_vif_CheckFrameData(MI_S32 u32ChnIndex, MI_S32 u32PortIndex, MI_SYS_FrameData_t* pstFrameData)
{
#if (MI_VIF_FRAME_CHECK == 1)
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];
    MI_U32 *pu64Data;

    if(pPortCtx->bCheckFrame == FALSE)
        return;

    pu64Data = mi_sys_Vmap(pstFrameData->phyAddr[0], 4096, FALSE);
    MI_SYS_BUG_ON(pu64Data == NULL);

    if(*pu64Data == MI_VIF_BUFFER_MAGIC)
    {
        pPortCtx->u32CheckFrameFailCount ++;
    }

    mi_sys_UnVmap(pu64Data);
#endif
}

static void _mi_vif_DumpFrame(MI_S32 u32ChnIndex, MI_S32 u32PortIndex, MI_SYS_BufInfo_t* pstBufInfo)
{
#if (MI_VIF_FRAME_DUMP == 1)
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];
    MI_S8 aName[128];
    FILE_HANDLE hFileDump;
    MI_SYS_BufRef_t *pstBufRef;
    MI_SYS_BufferAllocation_t* pstAllocation;

    if(pPortCtx->bDumpFrame == FALSE)
        return;

    pPortCtx->bDumpFrame = FALSE;

    if(pPortCtx->stChnPortAttr.ePixFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
    {
        sprintf(aName, "%s/mi_vif_%d_%d_[%dx%d]_%lld.yuyv", pPortCtx->s8DumpPath, u32ChnIndex, u32PortIndex,
                ALIGN_UP(pPortCtx->stChnPortAttr.stDestSize.u16Width, 32),
                pPortCtx->stChnPortAttr.stDestSize.u16Height, ktime_to_ms(ktime_get()));
    }
    else if(pPortCtx->stChnPortAttr.ePixFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        sprintf(aName, "%s/mi_vif_%d_%d_[%dx%d]_%lld.yuv", pPortCtx->s8DumpPath, u32ChnIndex, u32PortIndex,
                ALIGN_UP(pPortCtx->stChnPortAttr.stDestSize.u16Width, 32),
                pPortCtx->stChnPortAttr.stDestSize.u16Height, ktime_to_ms(ktime_get()));
    }
    else if(pPortCtx->stChnPortAttr.ePixFormat == E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_RG)
    {
        DBG_ERR("\n");
        sprintf(aName, "%s/mi_vif_%d_%d_[%dx%d]_%lld.12bit.rg.raw", pPortCtx->s8DumpPath, u32ChnIndex, u32PortIndex,
                ALIGN_UP(pPortCtx->stChnPortAttr.stDestSize.u16Width, 32),
                pPortCtx->stChnPortAttr.stDestSize.u16Height, ktime_to_ms(ktime_get()));
    }
    else
    {
        sprintf(aName, "%s/mi_vif_%d_%d_[%dx%d]_%lld.unknown", pPortCtx->s8DumpPath, u32ChnIndex, u32PortIndex,
                ALIGN_UP(pPortCtx->stChnPortAttr.stDestSize.u16Width, 32),
                pPortCtx->stChnPortAttr.stDestSize.u16Height, ktime_to_ms(ktime_get()));
    }

    hFileDump = open_yuv_file(aName, 1);
    pstBufRef = container_of(pstBufInfo, MI_SYS_BufRef_t, bufinfo);
    pstAllocation = pstBufRef->pstBufAllocation;

    if(pstBufInfo->stFrameData.pVirAddr[0] == NULL)
    {
        pstBufInfo->stFrameData.pVirAddr[0] = pstAllocation->ops.vmap_kern(pstAllocation);
        pstBufInfo->stFrameData.pVirAddr[1]  = (char*)pstBufInfo->stFrameData.pVirAddr[0]  + (pstBufInfo->stFrameData.phyAddr[1] - pstBufInfo->stFrameData.phyAddr[0]);
        pstBufInfo->stFrameData.pVirAddr[2]  = (char*)pstBufInfo->stFrameData.pVirAddr[0]  + (pstBufInfo->stFrameData.phyAddr[2] - pstBufInfo->stFrameData.phyAddr[0]);
    }

    _mi_vif_write_yuv_file(hFileDump, &pstBufInfo->stFrameData);
    close_yuv_file(hFileDump);
    pstAllocation->ops.vunmap_kern(pstAllocation);
#endif
}

void _mi_vif_FillFrameData(MI_SYS_FrameData_t* pstFrameData)
{
#if (MI_VIF_FRAME_CHECK == 1)
    MI_GFX_Surface_t stSurface;
    MI_GFX_Rect_t stRect;
    MI_U16 u16Fence;

    if(pstFrameData->ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
    {
        stSurface.eColorFmt = E_MI_GFX_FMT_YUV422;
        stSurface.phyAddr   = pstFrameData->phyAddr[0];
        stSurface.u32Height = pstFrameData->u16Height;
        stSurface.u32Width  = pstFrameData->u16Width;
        stSurface.u32Stride = pstFrameData->u32Stride[0];
        stRect.s32Xpos = 0;
        stRect.s32Ypos = 0;
        stRect.u32Width = pstFrameData->u16Width;
        stRect.u32Height = pstFrameData->u16Height;

        if(MI_SUCCESS != MI_GFX_QuickFill(&stSurface, &stRect, -1, &u16Fence))
        {
            DBG_ERR("Gfx clear buf error!\n");
            return;
        }

        MI_GFX_WaitAllDone(FALSE, u16Fence);
    }

    if(pstFrameData->ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        stSurface.eColorFmt = E_MI_GFX_FMT_I8;
        stSurface.phyAddr   = pstFrameData->phyAddr[0];
        stSurface.u32Height = pstFrameData->u16Height;
        stSurface.u32Width  = pstFrameData->u16Width;
        stSurface.u32Stride = pstFrameData->u32Stride[0];
        stRect.s32Xpos = 0;
        stRect.s32Ypos = 0;
        stRect.u32Width = pstFrameData->u16Width;
        stRect.u32Height = pstFrameData->u16Height;

        if(MI_SUCCESS != MI_GFX_QuickFill(&stSurface, &stRect, -1, &u16Fence))
        {
            DBG_ERR("Gfx clear buf error!\n");
            return;
        }

        MI_GFX_WaitAllDone(FALSE, u16Fence);

        stSurface.eColorFmt = E_MI_GFX_FMT_I8;
        stSurface.phyAddr   = pstFrameData->phyAddr[1];
        stSurface.u32Height = pstFrameData->u16Height / 2;
        stSurface.u32Width  = pstFrameData->u16Width;
        stSurface.u32Stride = pstFrameData->u32Stride[1];

        stRect.s32Xpos = 0;
        stRect.s32Ypos = 0;
        stRect.u32Width = pstFrameData->u16Width;
        stRect.u32Height = pstFrameData->u16Height / 2 ;

        if(MI_SUCCESS != MI_GFX_QuickFill(&stSurface, &stRect, -1, &u16Fence))
        {
            DBG_ERR("Gfx clear buf error!\n");
            return;
        }

        MI_GFX_WaitAllDone(FALSE, u16Fence);
    }

#endif
}

void _mi_vif_timeout_init(MI_S32 u32ChnIndex,    MI_S32 u32PortIndex)
{
#if (MI_VIF_TIMEOUT_CHECK > 0)
    gstTimeout[E_MI_VIF_TIME_SYS_GET_BUF].u32TimeThreshold     = 80;
    gstTimeout[E_MI_VIF_TIME_SYS_GET_BUF].ps8Name              = "Get";
    gstTimeout[E_MI_VIF_TIME_SYS_FINISH_BUF].u32TimeThreshold  = 70;
    gstTimeout[E_MI_VIF_TIME_SYS_FINISH_BUF].ps8Name           = "Finish";
    gstTimeout[E_MI_VIF_TIME_SYS_REWIND_BUF].u32TimeThreshold  = 30;
    gstTimeout[E_MI_VIF_TIME_SYS_REWIND_BUF].ps8Name           = "ReWind";
    gstTimeout[E_MI_VIF_TIME_VIF_PROCESS_IN].u32TimeThreshold  = 80;
    gstTimeout[E_MI_VIF_TIME_VIF_PROCESS_IN].ps8Name           = "In";
    gstTimeout[E_MI_VIF_TIME_VIF_PROCESS_OUT].u32TimeThreshold = 70;
    gstTimeout[E_MI_VIF_TIME_VIF_PROCESS_OUT].ps8Name          = "Out";
    gstTimeout[E_MI_VIF_TIME_MHAL_QUEUE_BUF].u32TimeThreshold  = 40;
    gstTimeout[E_MI_VIF_TIME_MHAL_QUEUE_BUF].ps8Name           = "Queue";
#endif
}

void _mi_vif_timeout_add(MI_S32 u32ChnIndex,    MI_S32 u32PortIndex, MI_U32 u32TimeoutType, ktime_t stStartTime)
{
#if (MI_VIF_TIMEOUT_CHECK > 0)
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];
    mi_vif_timeout_t* pstTimeout = &gstTimeout[u32TimeoutType];
    MI_S64 s64TimeUse;

    if(pPortCtx->bCheckTimeOut == FALSE)
        return;

    s64TimeUse = ktime_us_delta(ktime_get(), stStartTime);

    if(s64TimeUse > pstTimeout->u32TimeThreshold)
    {
        pstTimeout->u32Time[pstTimeout->u32TimeoutCount % MI_VIF_TIMEOUT_NUM] = s64TimeUse;
        pstTimeout->u32TimeoutCount++;
        //DBG_ERR("chn:%d port:%d type:%u threshold:%u timeout:%lld\n",u32ChnIndex,u32PortIndex,u32TimeoutType,pstTimeout->u32TimeThreshold,s64TimeUse);
        //MI_SYS_BUG();
    }

#endif
}


void _MI_VIF_FrameModePreNotify(MI_S32 u32ChnIndex, MI_S32 u32PortIndex)
{
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    mi_vif_BufTrackInfo_t *pstBufTrackInfo, *tmp;
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

    MI_SYS_BUG_ON(u32PortIndex);

    list_for_each_entry_safe(pstBufTrackInfo, tmp, &pPortCtx->stBufferList, list)
    {
        if(pstBufTrackInfo->pstBufInfo->eNotifyState == E_MI_SYS_FRAME_NOTIFY_NEED_NOTIFY)
        {
            //change pts into current time
            pstBufTrackInfo->pstBufInfo->u64Pts = ktime_to_us(ktime_get());
            pstBufTrackInfo->pstBufInfo->eNotifyState = E_MI_SYS_FRAME_NOTIFY_NOTIFIED;

            BUG_ON(pstBufTrackInfo->pstBufInfo->stFrameData.eCompressMode == E_MI_SYS_COMPRESS_MODE_NONE);

            MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_NotifyPreProcessBuf(pstBufTrackInfo->pstBufInfo));
        }
    }
}


void  _MI_ISR_FrameEndCallback(MHal_VIF_CHN u32VifChn, MHal_VIF_EventType_e eType, MHal_VIF_RingBufElm_t *pstFbInfo, void *pPrivateData)
{
    MI_SYS_BUG_ON(eType != E_VIF_EVENT_FRAME_END);

    //DBG_ERR("chn:%d type:%d ele:%p priv:%p mi_priv:%p\n", u32VifChn, eType, pstFbInfo, pPrivateData, pstFbInfo->nMiPriv);
    //complete(&gstVifProcessWakeup);
    return;
}

void  _MI_ISR_FrameLineCountCallback(MHal_VIF_CHN u32VifChn, MHal_VIF_EventType_e eType, MHal_VIF_RingBufElm_t *pstFbInfo, void *pPrivateData)
{
    MI_SYS_BufInfo_t* pstBufInfo;

    MI_SYS_BUG_ON(eType != E_VIF_EVENT_LINE_COUNT_HIT0);

    MI_SYS_BUG_ON(!pstFbInfo);
    pstBufInfo = (MI_SYS_BufInfo_t*)pstFbInfo->nMiPriv;
    MI_SYS_BUG_ON(!pstBufInfo);
    if (pstBufInfo->eNotifyState == E_MI_SYS_FRAME_NOTIFY_QUEUED)
    {
        pstBufInfo->eNotifyState = E_MI_SYS_FRAME_NOTIFY_NEED_NOTIFY;
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
        mi_sys_buf_mgr_init_trackinfo(pstBufInfo, 0);
#endif
        //complete(&gstVifProcessWakeup);
    }

    return;
}
static void _MI_VIF_ProcessFrameModeIn(MI_VIF_DEV u32DevIndex)
{
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_BufInfo_t* pstBufInfo;
    MHal_VIF_RingBufElm_t stFbInfo;
    MI_S32 u32ChnIndex;
    MI_S32 u32PortIndex;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    ktime_t stStartTime, stStartTimeTmp;
    MS_S32 s32MhalRet;

    //DBG_INFO("in\n");

    for(u32ChnIndex = MI_VIF_MAX_WAY_NUM_PER_DEV * u32DevIndex; u32ChnIndex < MI_VIF_MAX_WAY_NUM_PER_DEV * u32DevIndex + MI_VIF_MAX_WAY_NUM_PER_DEV; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        mutex_lock(&pChnCtx->stLock);

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == FALSE)
            {
                continue;
            }

            //DBG_ERR("\n");

            stStartTime = ktime_get();

            memset(&stBufCfg, 0, sizeof(MI_SYS_BufConf_t));
            stBufCfg.eBufType             = E_MI_SYS_BUFDATA_FRAME;
            stBufCfg.u64TargetPts         = MI_SYS_INVALID_PTS;
            stBufCfg.stFrameCfg.eFormat   = pPortCtx->stChnPortAttr.ePixFormat;
            stBufCfg.stFrameCfg.u16Width  = ALIGN_UP(pPortCtx->stChnPortAttr.stDestSize.u16Width, 32);
            stBufCfg.stFrameCfg.u16Height = pPortCtx->stChnPortAttr.stDestSize.u16Height;

            while(pPortCtx->u32HalBufferHold < MI_VIF_PORT_MAX_BUFFER)
            {
                stStartTimeTmp = ktime_get();
                pstBufInfo = mi_sys_GetOutputPortBuf(hVifHandle, u32ChnIndex, u32PortIndex, &stBufCfg, &bBlockedByRateCtrl);
                _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_SYS_GET_BUF, stStartTimeTmp);

                if(pstBufInfo == NULL)
                    break;



                if(pstBufInfo->stFrameData.u16Width < stBufCfg.stFrameCfg.u16Width)
                {
                    DBG_ERR("err:stFrameData.u16Width:%d < stFrameCfg.u16Width:%d\n", pstBufInfo->stFrameData.u16Width, stBufCfg.stFrameCfg.u16Width);
                    MI_SYS_BUG();
                    mi_sys_RewindBuf(pstBufInfo);
                    break;
                }

                if(pstBufInfo->stFrameData.u16Height < stBufCfg.stFrameCfg.u16Height)
                {
                    DBG_ERR("err:stFrameData.u16Height < stFrameCfg.u16Height\n");
                    MI_SYS_BUG();
                    mi_sys_RewindBuf(pstBufInfo);
                    break;
                }

                stFbInfo.u64PhyAddr[0] =  pstBufInfo->stFrameData.phyAddr[0];
                stFbInfo.u64PhyAddr[1] =  pstBufInfo->stFrameData.phyAddr[1];
                //stFbInfo.u64PhyAddr[2] =  pstBufInfo->stFrameData.phyAddr[2];
                stFbInfo.u32Stride[0]  =  pstBufInfo->stFrameData.u32Stride[0];
                stFbInfo.u32Stride[1]  =  pstBufInfo->stFrameData.u32Stride[1];
                stFbInfo.nCropX        =  pPortCtx->stChnPortAttr.stCapRect.u16X;
                stFbInfo.nCropY        =  pPortCtx->stChnPortAttr.stCapRect.u16Y;
                stFbInfo.nCropW        =  pPortCtx->stChnPortAttr.stCapRect.u16Width;
                stFbInfo.nCropH        =  pPortCtx->stChnPortAttr.stCapRect.u16Height;
                stFbInfo.nMiPriv       = (MS_U32)pstBufInfo;
                stFbInfo.nStatus       =  E_VIF_BUF_INVALID;

                _mi_vif_FillFrameData(&pstBufInfo->stFrameData);

                stStartTimeTmp = ktime_get();
                s32MhalRet = MHal_VIF_QueueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo);
                _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_MHAL_QUEUE_BUF, stStartTimeTmp);

                if(MHAL_SUCCESS != s32MhalRet)
                {
                    //MI_SYS_BUG();
                    stStartTimeTmp = ktime_get();
                    mi_sys_RewindBuf(pstBufInfo);
                    _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_SYS_REWIND_BUF, stStartTimeTmp);
                    break;
                }

                _mi_vif_BuffListAdd(pPortCtx, pstBufInfo);

                if(pPortCtx->stChnPortAttr.u32FrameModeLineCount)
                {
                    pstBufInfo->eNotifyState = E_MI_SYS_FRAME_NOTIFY_QUEUED;
                }

                pPortCtx->u32ProcessInCount++;
                pPortCtx->u32HalBufferHold++;
            }

            _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_VIF_PROCESS_IN, stStartTime);
        }

        mutex_unlock(&pChnCtx->stLock);
    }
}

static void _MI_VIF_FlushPortBuffer(MI_VIF_CHN u32ChnIndex, MI_VIF_PORT u32PortIndex)
{
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];
    MHal_VIF_RingBufElm_t stFbInfo;
    MI_SYS_BufInfo_t* pstBufInfo;

    //printk("1 %d %d u32HalBufferHold = %u\n", u32ChnIndex, u32PortIndex, pPortCtx->u32HalBufferHold);

    //DBG_ENTER();
#if (MI_VIF_PORT_BUFFER_LIST == 1)
    mi_vif_BufTrackInfo_t *pstBufTrackInfo, *tmp;
    list_for_each_entry_safe(pstBufTrackInfo, tmp, &pPortCtx->stBufferList, list)
    {
        if(MHAL_SUCCESS != MHal_VIF_DequeueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo))
        {
            MI_SYS_BUG();
            continue;
        }

        list_del_init(&pstBufTrackInfo->list);
        kmem_cache_free(gstpVifBufferCache, pstBufTrackInfo);

        pstBufInfo         = (MI_SYS_BufInfo_t*)stFbInfo.nMiPriv;
        pstBufInfo->u64Pts = stFbInfo.nPTS;
        //printk("%d pstBufInfo = %p\n", __LINE__, pstBufInfo);
        mi_sys_FinishBuf(pstBufInfo);
        pPortCtx->u32HalBufferHold --;
    }
#else

    while(pPortCtx->u32HalBufferHold)
    {
        if(MHAL_SUCCESS != MHal_VIF_DequeueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo))
        {
            MI_SYS_BUG();
            continue;
        }

        pstBufInfo         = (MI_SYS_BufInfo_t*)stFbInfo.nMiPriv;
        pstBufInfo->u64Pts = stFbInfo.nPTS;
        //printk("%d pstBufInfo = %p\n", __LINE__, pstBufInfo);
        mi_sys_FinishBuf(pstBufInfo);
        pPortCtx->u32HalBufferHold --;
    }

#endif

    MI_SYS_BUG_ON(pPortCtx->u32HalBufferHold);
    //printk("3 %d %d u32HalBufferHold = %u\n",u32ChnIndex,u32PortIndex,pPortCtx->u32HalBufferHold);
    //DBG_EXIT_OK();
}


static void _MI_VIF_FlushBuffer(void)
{
    MI_S32 u32ChnIndex;
    MI_S32 u32PortIndex;
    mi_vif_ChnContext_t* pChnCtx;

    DBG_ENTER();

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        pChnCtx = &gstVifChnCtx[u32ChnIndex];

        if(pChnCtx->stPortCtx[0].bEnable == TRUE)
            MHal_VIF_ChnDisable(u32ChnIndex);
        if(pChnCtx->stPortCtx[1].bEnable == TRUE)
            MHal_VIF_SubChnDisable(u32ChnIndex);
    }

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        pChnCtx = &gstVifChnCtx[u32ChnIndex];
        mutex_lock(&pChnCtx->stLock);

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            if(pChnCtx->stPortCtx[u32PortIndex].bEnable)
                _MI_VIF_FlushPortBuffer(u32ChnIndex, u32PortIndex);
        }

        mutex_unlock(&pChnCtx->stLock);
    }

    DBG_EXIT_OK();
}

static void _MI_VIF_ProcessFrameModeOut(MI_VIF_DEV u32DevIndex)
{
    MI_SYS_BufInfo_t* pstBufInfo;
    MHal_VIF_RingBufElm_t stFbInfo;
    MI_S32 u32ChnIndex;
    MI_S32 u32PortIndex;
    MI_U32 u32BufferNum;
    ktime_t stStartTime, stStartTimeTmp;

    //DBG_ENTER();

    for(u32ChnIndex = MI_VIF_MAX_WAY_NUM_PER_DEV * u32DevIndex; u32ChnIndex < MI_VIF_MAX_WAY_NUM_PER_DEV * u32DevIndex + MI_VIF_MAX_WAY_NUM_PER_DEV; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        mutex_lock(&pChnCtx->stLock);

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == FALSE)
            {
                continue;
            }
            if ((0 == u32PortIndex) && (pPortCtx->stSysPort.bEnableLowLatencyReceiveMode == TRUE))
            {
                _MI_VIF_FrameModePreNotify(u32ChnIndex, u32PortIndex);
            }

            if(jiffies - pPortCtx->u32FrameCountJiffies > HZ)
            {
                pPortCtx->u32FrameRate = pPortCtx->u32FrameCount;
                pPortCtx->u32FrameCount = 0;
                pPortCtx->u32FrameCountJiffies = jiffies;
            }

            stStartTime = ktime_get();

            if(MHAL_SUCCESS != MHal_VIF_QueryFrames(u32ChnIndex, u32PortIndex, &u32BufferNum))
            {
                DBG_INFO("the value of u32BufferNum is 0\n");
                break;
            }

            //DBG_INFO("ChnIndex: %d PortIndex: %d  BufferNum:%d\n", u32ChnIndex, u32PortIndex, u32BufferNum);

            while(u32BufferNum--)
            {
                if(MHAL_SUCCESS != MHal_VIF_DequeueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo))
                {
                    DBG_ERR("MHal_VIF_DequeueFrameBuffer fail chn:%d port:%d!\n", u32ChnIndex, u32PortIndex);
                    MI_SYS_BUG();
                    break;
                }
                else
                {
                    MI_SYS_BUG_ON(stFbInfo.nMiPriv == NULL);
                    //DBG_INFO("MHal_VIF_DequeueFrameBuffer sucesss chn:%d port:%d %p!\n", u32ChnIndex, u32PortIndex,stFbInfo.nMiPriv);
                }

                pPortCtx->u32HalBufferHold --;

                pstBufInfo         = (MI_SYS_BufInfo_t*)stFbInfo.nMiPriv;
                pstBufInfo->u64Pts = stFbInfo.nPTS;

                pstBufInfo->stFrameData.eFrameScanMode = pPortCtx->stChnPortAttr.eScanMode;
                pstBufInfo->stFrameData.eFieldType = stFbInfo.eFieldType ;
                //pstBufInfo->stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                pstBufInfo->u64SidebandMsg = stFbInfo.u64MetaInfo;

                _mi_vif_DumpFrame(u32ChnIndex, u32PortIndex, pstBufInfo);
                _mi_vif_CheckFrameData(u32ChnIndex, u32PortIndex, &pstBufInfo->stFrameData);
                _mi_vif_BuffListDel(pPortCtx, pstBufInfo);
                stStartTimeTmp = ktime_get();
                if ((pstBufInfo->eNotifyState == E_MI_SYS_FRAME_NOTIFY_NOTIFIED) /*&& (pPortCtx->stSysPort.bEnableLowLatencyReceiveMode == TRUE)*/)
                {
                    mi_sys_FinishBuf_LL(pstBufInfo);
                }
                else
                {
                    mi_sys_FinishBuf(pstBufInfo);
                }

                _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_SYS_FINISH_BUF, stStartTimeTmp);

                pPortCtx->u32FrameCount++;
                pPortCtx->u32ProcessOutCount++;
            }

            _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_VIF_PROCESS_OUT, stStartTime);
        }

        mutex_unlock(&pChnCtx->stLock);
    }

    //DBG_EXIT_OK();
}

void _MI_VIF_ProcessRealTime(MI_VIF_DEV u32DevIndex)
{
    MI_S32 u32ChnIndex;
    MI_S32 u32PortIndex;
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_BufInfo_t* pstBufInfo;
    MI_BOOL bBlockedByRateCtrl = FALSE;
    mi_vif_InputInfo_t *pstVifInputInfo = NULL;
    mi_vif_ChnContext_t* pChnCtx;
    mi_vif_PortContext_t* pPortCtx;
    ktime_t stStartTime;
    ktime_t stStartTimeTmp;

    u32ChnIndex = MI_VIF_MAX_WAY_NUM_PER_DEV * u32DevIndex;
    u32PortIndex = 0;
    pChnCtx = &gstVifChnCtx[u32ChnIndex];
    pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

    if(pPortCtx->bEnable == FALSE)
    {
        return;
    }


    mutex_lock(&pChnCtx->stLock);

    stStartTime = ktime_get();

    if(jiffies - pPortCtx->u32FrameCountJiffies > HZ)
    {
        pPortCtx->u32FrameRate = pPortCtx->u32FrameCount;
        pPortCtx->u32FrameCount = 0;
        pPortCtx->u32FrameCountJiffies = jiffies;
    }

    memset(&stBufCfg, 0, sizeof(MI_SYS_BufConf_t));
    stBufCfg.eBufType = E_MI_SYS_BUFDATA_META;
    stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;
    stBufCfg.stMetaCfg.u32Size = sizeof(mi_vif_InputInfo_t);
    stBufCfg.u32Flags = MI_SYS_MAP_VA;

    stStartTimeTmp = ktime_get();
    pstBufInfo = mi_sys_GetOutputPortBuf(hVifHandle, u32ChnIndex, u32PortIndex, &stBufCfg, &bBlockedByRateCtrl);
    _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_SYS_GET_BUF, stStartTimeTmp);

    if(pstBufInfo == NULL)
    {
        //pPortCtx->u32GetSysBufferFailCount ++;
        mutex_unlock(&pChnCtx->stLock);
        return;
    }

    MI_SYS_BUG_ON(pstBufInfo->eBufType != E_MI_SYS_BUFDATA_META);
    MI_SYS_BUG_ON(pstBufInfo->stMetaData.u32Size < sizeof(mi_vif_InputInfo_t));
    pstBufInfo->stMetaData.eDataFromModule = E_MI_MODULE_ID_VIF;
    pstVifInputInfo = pstBufInfo->stMetaData.pVirAddr;
    pstVifInputInfo->ePixelFormat = pPortCtx->stChnPortAttr.ePixFormat;
    pstVifInputInfo->u16Height    = pPortCtx->stChnPortAttr.stDestSize.u16Height;
    pstVifInputInfo->u16Width     = pPortCtx->stChnPortAttr.stDestSize.u16Width;

    stStartTimeTmp = ktime_get();
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    mi_sys_buf_mgr_init_trackinfo(pstBufInfo, 0);
#endif
    MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_FinishBuf(pstBufInfo));
    _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_SYS_FINISH_BUF, stStartTimeTmp);

    pPortCtx->u32FrameCount++;
    pPortCtx->u32ProcessOutCount++;
    _mi_vif_timeout_add(u32ChnIndex, u32PortIndex, E_MI_VIF_TIME_VIF_PROCESS_OUT, stStartTime);

    mutex_unlock(&pChnCtx->stLock);

}

//Compare vif port attr, Port support to dynamic set framerate
static int _mi_vif_CompareFrameRate(MI_VIF_ChnPortAttr_t* pstAttrOld, MI_VIF_ChnPortAttr_t* pstAttrNew)
{
    if (((pstAttrOld->stCapRect.u16X == pstAttrNew->stCapRect.u16X) &&
            (pstAttrOld->stCapRect.u16Y == pstAttrNew->stCapRect.u16Y) &&
            (pstAttrOld->stCapRect.u16Width == pstAttrNew->stCapRect.u16Width) &&
            (pstAttrOld->stCapRect.u16Height == pstAttrNew->stCapRect.u16Height) &&
            (pstAttrOld->stDestSize.u16Width == pstAttrNew->stDestSize.u16Width) &&
            (pstAttrOld->stDestSize.u16Height == pstAttrNew->stDestSize.u16Height) &&
            (pstAttrOld->eCapSel == pstAttrNew->eCapSel) &&
            (pstAttrOld->bFlip == pstAttrNew->bFlip) &&
            (pstAttrOld->bMirror == pstAttrNew->bMirror) &&
            (pstAttrOld->ePixFormat == pstAttrNew->ePixFormat) &&
            (pstAttrOld->eScanMode == pstAttrNew->eScanMode)) &&
            (pstAttrOld->eFrameRate != pstAttrNew->eFrameRate))
    {
        return 0;
    }

    return 1;
}

static void _MI_VIF_Process(void)
{
    MI_VIF_DEV u32DevIndex;

    for(u32DevIndex = 0; u32DevIndex < MI_VIF_MAX_DEV_NUM; u32DevIndex++)
    {
        switch(gstVifDevCtx[u32DevIndex].stDevAttr.eWorkMode)
        {
            case E_MI_VIF_WORK_MODE_RGB_REALTIME:
                _MI_VIF_ProcessRealTime(u32DevIndex);
                break;

            case E_MI_VIF_WORK_MODE_RGB_FRAMEMODE:
            case E_MI_VIF_WORK_MODE_1MULTIPLEX:
            case E_MI_VIF_WORK_MODE_2MULTIPLEX:
            case E_MI_VIF_WORK_MODE_4MULTIPLEX:
                _MI_VIF_ProcessFrameModeIn(u32DevIndex);
                _MI_VIF_ProcessFrameModeOut(u32DevIndex);
                break;

            default:
                // err
                break;
        }
    }

    //DBG_EXIT_OK();
}

static int _mi_vif_WorkTask(void *data)
{
    DBG_ENTER();

    do
    {
        _MI_VIF_Process();
        wait_for_completion_timeout(&gstVifProcessWakeup, msecs_to_jiffies(1000));
    }
    while(!kthread_should_stop());

    DBG_EXIT_OK();
    return 0;
}

static void mi_vif_timer_func(unsigned long data)
{
    complete(&gstVifProcessWakeup);
    mod_timer(&gstVifTimer, jiffies + HZ / 100); // 10ms
}

static MI_S32 MI_VIF_OnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[pstChnCurryPort->u32ChnId];
    DBG_ENTER();
    if(pstChnCurryPort->u32DevId)
    {
        DBG_ERR("mi vif dev id must be 0!!!\n");
        MI_SYS_BUG();
    }
	pChnCtx->stPortCtx[pstChnCurryPort->u32PortId].stSysPort = *pstChnPeerPort;
	


    return MI_SUCCESS;
}


static MI_S32 MI_VIF_OnUnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[pstChnCurryPort->u32ChnId];
    DBG_ENTER();
    
    memset(&pChnCtx->stPortCtx[pstChnCurryPort->u32PortId].stSysPort, 0, sizeof(MI_SYS_ChnPort_t));
    return MI_SUCCESS;
}

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VIF_PROCFS_DEBUG == 1)
static MI_S32 _MI_VIF_ProcOnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_U32  u32VifDevIdx;
    MI_VIF_DevAttr_t* pstDevAttr;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    handle.OnPrintOut(handle, "------------------------------------------------------------------------------------\n");

    handle.OnPrintOut(handle, "dev intf work clk rev         mask                         chn              \n");

    for(u32VifDevIdx = 0; u32VifDevIdx < MI_VIF_MAX_DEV_NUM ; u32VifDevIdx++)
    {
        //handle.OnPrintOut(handle, "u32VifDevIdx: %d enable: %d\n", u32VifDevIdx ,gstVifDevCtx[u32VifDevIdx].bEnable);
        if(gstVifDevCtx[u32VifDevIdx].bEnable == FALSE)
            continue;

        pstDevAttr = &gstVifDevCtx[u32VifDevIdx].stDevAttr;
        handle.OnPrintOut(handle, "%3d %4d %4d %3d %3d ", u32VifDevIdx, pstDevAttr->eIntfMode, pstDevAttr->eWorkMode, pstDevAttr->eClkEdge, pstDevAttr->bDataRev);
        handle.OnPrintOut(handle, "(%08x %08x) ", pstDevAttr->au32CompMask[0], pstDevAttr->au32CompMask[1]);
        handle.OnPrintOut(handle, "(%08x %08x %08x %08x)\n", pstDevAttr->as32AdChnId[0], pstDevAttr->as32AdChnId[1], pstDevAttr->as32AdChnId[2], pstDevAttr->as32AdChnId[3]);

        if(pstDevAttr->eIntfMode == E_MI_VIF_MODE_DIGITAL_CAMERA)
        {
            handle.OnPrintOut(handle, "eDataSeq:%d\n", pstDevAttr->eDataSeq);
            handle.OnPrintOut(handle, "eHsync: %d eHsyncNeg:%d \n", pstDevAttr->stSynCfg.eHsync, pstDevAttr->stSynCfg.eHsyncNeg);
            handle.OnPrintOut(handle, "eVsync: %d eVsyncNeg:%d  eVsyncValid:%d eVsyncValidNeg:%d \n", pstDevAttr->stSynCfg.eVsync, pstDevAttr->stSynCfg.eVsyncNeg, pstDevAttr->stSynCfg.eVsyncValid, pstDevAttr->stSynCfg.eVsyncValidNeg);

            handle.OnPrintOut(handle, "u32HsyncAct: %d u32HsyncHbb:%d  u32HsyncHfb:%d \n", pstDevAttr->stSynCfg.stTimingBlank.u32HsyncAct, pstDevAttr->stSynCfg.stTimingBlank.u32HsyncHbb, pstDevAttr->stSynCfg.stTimingBlank.u32HsyncHfb);
            handle.OnPrintOut(handle, "u32VsyncVact: %d u32VsyncVbact:%d  u32VsyncVbb:%d \n", pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVact, pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbact, pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbb);
            handle.OnPrintOut(handle, "u32VsyncVbbb: %d u32VsyncVbfb:%d  u32VsyncVfb:%d \n", pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbbb, pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbfb, pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVfb);
        }
    }


    return MI_SUCCESS;
}

static void mi_sys_DumpRiuBank(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32Bank)
{
    static MI_U32 u32RegBase = 0;
    MI_U32* pu32BankVir;
    MI_U32 i = 0;
    MI_U32 idx = 0;

    if(u32RegBase == 0)
    {
        u32RegBase = (unsigned long)ioremap(0x1F000000, 0x400000);
    }

    pu32BankVir = (MI_U32*)(u32RegBase + (u32Bank << 9));

    handle.OnPrintOut(handle, "BANK = 0x%4X\n", u32Bank);

    for(i = 0 ; i < 16 ; i ++)
    {
        handle.OnPrintOut(handle, "%02x: ", idx);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "0x%04X ", *pu32BankVir++);
        handle.OnPrintOut(handle, "\n");
        idx += 8;
    }
}

static MI_S32 _MI_VIF_ProcOnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
#if 0
    mi_vif_ChnContext_t *pstChnCtx = NULL;
    MI_U32 u32ChnIndex;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    MI_VIF_CHECK_PARA_RANGE(u32ChnId, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];
    }

#endif


    MI_U32 mi_vif_chn_bank[8] = {0x1502, 0x1503, 0x1504, 0x1505,
                                 0x1506, 0x1507, 0x1705, 0x1706
                                };
    MI_U32 mi_vif_clock_bank[4] = {0x100a, 0x100b, 0x1033, 0x1432};
    MI_U32 mi_vif_dma_bank[21] = {0x1420, 0x1421, 0x1422, 0x1423,
                                  0x1424, 0x1425, 0x1426, 0x1427,
                                  0x1428, 0x1429, 0x142A, 0x142B,
                                  0x142C, 0x142D, 0x142E, 0x142F,
                                  0x1509, 0x1514, 0x1516, 0x1517,
                                  0x153B
                                 };

    MI_U32 u32VifBankIdx;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(gbVifDumpChnBank)
    {
        handle.OnPrintOut(handle, "---------------------VIF CHN BANK----------------------------------\n");

        for(u32VifBankIdx = 0; u32VifBankIdx < 8; u32VifBankIdx++)
        {
            mi_sys_DumpRiuBank(handle, mi_vif_chn_bank[u32VifBankIdx]);
        }
    }

    if(gbVifDumpDmaBank)
    {
        handle.OnPrintOut(handle, "---------------------VIF DMA BANK----------------------------------\n");

        for(u32VifBankIdx = 0; u32VifBankIdx < 21; u32VifBankIdx++)
        {
            mi_sys_DumpRiuBank(handle, mi_vif_dma_bank[u32VifBankIdx]);
        }
    }

    if(gbVifDumpClockBank)
    {
        handle.OnPrintOut(handle, "---------------------VIF CLOCK BANK--------------------------------\n");

        for(u32VifBankIdx = 0; u32VifBankIdx < 4; u32VifBankIdx++)
        {
            mi_sys_DumpRiuBank(handle, mi_vif_clock_bank[u32VifBankIdx]);
        }
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VIF_ProcOnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_VIF_CHN u32ChnIndex;
    MI_VIF_PORT u32PortIndex;
    mi_vif_PortContext_t* pPortCtx;
    //MI_VIF_ChnPortStat_t stPortStat;
#if (MI_VIF_TIMEOUT_CHECK > 0)
    mi_vif_timeout_t* pstTimeOut;
    MI_U32 u32TimeOutIdx;
    MI_U32 u32TimeOutType;
#endif

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    handle.OnPrintOut(handle, "------------------------------------------------------------------------------------\n");

    handle.OnPrintOut(handle, "           cap        dest      sel scan  fmt rate  linecnt  mi_frame_status\n");

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == TRUE)
            {
                handle.OnPrintOut(handle, "%2d %d ", u32ChnIndex, u32PortIndex);
                handle.OnPrintOut(handle, "(%u,%u,%4u,%4u) ", pPortCtx->stChnPortAttr.stCapRect.u16X, pPortCtx->stChnPortAttr.stCapRect.u16Y, pPortCtx->stChnPortAttr.stCapRect.u16Width, pPortCtx->stChnPortAttr.stCapRect.u16Height);
                handle.OnPrintOut(handle, "(%4u,%4u) ", pPortCtx->stChnPortAttr.stDestSize.u16Width, pPortCtx->stChnPortAttr.stDestSize.u16Height);
                handle.OnPrintOut(handle, "%d    %d    %d    %d    %d    ", pPortCtx->stChnPortAttr.eCapSel, pPortCtx->stChnPortAttr.eScanMode, pPortCtx->stChnPortAttr.ePixFormat, pPortCtx->stChnPortAttr.eFrameRate, pPortCtx->stChnPortAttr.u32FrameModeLineCount);
                handle.OnPrintOut(handle, "(%u,%x,%x,%u)", pPortCtx->u32HalBufferHold, pPortCtx->u32ProcessInCount, pPortCtx->u32ProcessOutCount, pPortCtx->u32FrameRate);
                handle.OnPrintOut(handle, "\n");
            }
        }
    }

    handle.OnPrintOut(handle, "\n      rev_size    out_size  sub_out_size  rwd_idx    mhal_frame_status    ring_buf_status\n");

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == TRUE)
            {
                MHal_VIF_ChnStat_t stMhalChnStat;

                if(u32PortIndex == 0)
                    MHal_VIF_ChnQuery(u32ChnIndex, &stMhalChnStat);
                else
                    MHal_VIF_SubChnQuery(u32ChnIndex, &stMhalChnStat);

                handle.OnPrintOut(handle, "%2d %d ", u32ChnIndex, u32PortIndex);
                handle.OnPrintOut(handle, "(%4u,%4u) (%4u,%4u) (%4u,%4u) ", stMhalChnStat.nReceiveWidth, stMhalChnStat.nReceiveHeight, stMhalChnStat.nOutputWidth, stMhalChnStat.nOutputHeight, stMhalChnStat.nSubOutputWidth, stMhalChnStat.nSubOutputHeight);
                handle.OnPrintOut(handle, "(%1u,%1u,%1u) ", stMhalChnStat.nReadIdx, stMhalChnStat.nWriteIdx, stMhalChnStat.nDequeueIdx);
                handle.OnPrintOut(handle, "(%x,%x,%x,%x)", stMhalChnStat.nFrameStartCnt, stMhalChnStat.nFrameDoneCnt, stMhalChnStat.u32VbFail, stMhalChnStat.nDropFrameCnt);
                handle.OnPrintOut(handle, "(%u,%u,%u,%u,%u,%u,%u,%u)", stMhalChnStat.eStatus[0], stMhalChnStat.eStatus[1], stMhalChnStat.eStatus[2], stMhalChnStat.eStatus[3], stMhalChnStat.eStatus[4], stMhalChnStat.eStatus[5], stMhalChnStat.eStatus[6], stMhalChnStat.eStatus[7]);
                handle.OnPrintOut(handle, "\n");
            }
        }
    }

#if (MI_VIF_TIMEOUT_CHECK > 0)
    handle.OnPrintOut(handle, "\n-------------------------- TimeOut Statistic --------------------------------\n");

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        for(u32PortIndex = 0; u32PortIndex < MI_VIF_MAX_CHN_OUTPORT; u32PortIndex++)
        {
            pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == TRUE && pPortCtx->bCheckTimeOut == TRUE)
            {
                handle.OnPrintOut(handle, "%d %d \n", u32ChnIndex, u32PortIndex);

                for(u32TimeOutType = E_MI_VIF_TIME_BEGIN ; u32TimeOutType < E_MI_VIF_TIME_NUM ; u32TimeOutType ++)
                {
                    pstTimeOut =  &gstTimeout[u32TimeOutType];
                    handle.OnPrintOut(handle, "%6.6s     [%d %d]  ", pstTimeOut->ps8Name, pstTimeOut->u32TimeoutCount, pstTimeOut->u32TimeThreshold);

                    for(u32TimeOutIdx = 0 ; u32TimeOutIdx < MI_VIF_TIMEOUT_NUM ; u32TimeOutIdx ++)
                    {
                        handle.OnPrintOut(handle, "%d ", pstTimeOut->u32Time[u32TimeOutIdx]);
                    }

                    handle.OnPrintOut(handle, "\n");
                }
            }
        }
    }

#endif
    return MI_SUCCESS;
}

static MI_S32 _MI_VIF_ProcOnHelp(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    handle.OnPrintOut(handle, "dump chn port path;    dump one frame from [chn,port] to [path].\n");
    handle.OnPrintOut(handle, "check_timeout  chn port [ON, OFF];    Enable/Disable [chn,port] timeout check.\n");
    handle.OnPrintOut(handle, "check_timeout_all  [ON, OFF];    Enable/Disable all port timeout check.\n");
    handle.OnPrintOut(handle, "dump_bank_dma  [ON, OFF];    Enable/Disable dump bank dma.\n");
    handle.OnPrintOut(handle, "dump_bank_clock  [ON, OFF];    Enable/Disable dump bank clock.\n");
    handle.OnPrintOut(handle, "dump_bank_chn  [ON, OFF];    Enable/Disable dump bank chn.\n");

    return s32Ret;
}

static MI_S32 _MI_VIF_ProcTimeoutAll(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if (MI_VIF_TIMEOUT_CHECK > 0)
    MI_VIF_CHN u32VifChn;
    MI_VIF_PORT u32ChnPort;
    MI_BOOL bTimeoutON = FALSE;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(strcmp(argv[1], "ON") == 0)
        bTimeoutON = TRUE;

    for(u32VifChn = 0; u32VifChn < MI_VIF_MAX_PHYCHN_NUM; u32VifChn++)
    {
        for(u32ChnPort = 0; u32ChnPort < MI_VIF_MAX_CHN_OUTPORT; u32ChnPort++)
        {
            gstVifChnCtx[u32VifChn].stPortCtx[u32ChnPort].bCheckTimeOut = bTimeoutON;
        }
    }

#endif
    return s32Ret;
}

static MI_S32 _MI_VIF_ProcTimeout(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if (MI_VIF_TIMEOUT_CHECK > 0)
    MI_VIF_CHN u32VifChn;
    MI_VIF_PORT u32ChnPort;
    mi_vif_PortContext_t* pPortCtx;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    kstrtouint(argv[1], 10, &u32VifChn);
    kstrtouint(argv[2], 10, &u32ChnPort);
    pPortCtx = &gstVifChnCtx[u32VifChn].stPortCtx[u32ChnPort];

    if(strcmp(argv[3], "ON") == 0)
    {
        pPortCtx->bCheckTimeOut = TRUE;
    }
    else if(strcmp(argv[3], "OFF") == 0)
    {
        pPortCtx->bCheckTimeOut = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
        s32Ret = MI_ERR_VIF_NOT_SUPPORT;
    }

    handle.OnPrintOut(handle, "timeout chn %d port %d %d\n", u32VifChn, u32ChnPort, pPortCtx->bCheckTimeOut);
#endif
    return s32Ret;
}

static MI_S32 _MI_VIF_ProcDisableChnPort(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VIF_CHN u32VifChn;
    MI_VIF_PORT u32ChnPort;
    if (argc > 2)
    {
        kstrtouint(argv[1], 10, &u32VifChn);
        kstrtouint(argv[2], 10, &u32ChnPort);
        s32Ret = MI_VIF_IMPL_DisableChnPort(u32VifChn, u32ChnPort);
        if (MI_SUCCESS != s32Ret)
        {
            handle.OnPrintOut(handle, "Disable channel port fail: chn(%d) port(%d).\n", u32VifChn, u32ChnPort);
            s32Ret = MI_ERR_VIF_NOT_SUPPORT;
        }
    }

    return s32Ret;
}

static MI_S32 _MI_VIF_ProcEnableChnPort(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VIF_CHN u32VifChn;
    MI_VIF_PORT u32ChnPort;
    if (argc > 2)
    {
        kstrtouint(argv[1], 10, &u32VifChn);
        kstrtouint(argv[2], 10, &u32ChnPort);
        s32Ret = MI_VIF_IMPL_EnableChnPort(u32VifChn, u32ChnPort);
        if (MI_SUCCESS != s32Ret)
        {
            handle.OnPrintOut(handle, "Enable channel port fail: chn(%d) port(%d).\n", u32VifChn, u32ChnPort);
            s32Ret = MI_ERR_VIF_NOT_SUPPORT;
        }
    }

    return s32Ret;
}

static MI_S32 _MI_VIF_ProcDumpClockBank(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(strcmp(argv[1], "ON") == 0)
    {
        gbVifDumpClockBank = TRUE;
    }
    else if(strcmp(argv[1], "OFF") == 0)
    {
        gbVifDumpClockBank = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
        s32Ret = MI_ERR_VIF_NOT_SUPPORT;
    }

    handle.OnPrintOut(handle, "VifDumpClockBank %d\n", gbVifDumpClockBank);

    return MI_SUCCESS;
}

static MI_S32 _MI_VIF_ProcDumpChnBank(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(strcmp(argv[1], "ON") == 0)
    {
        gbVifDumpChnBank = TRUE;
    }
    else if(strcmp(argv[1], "OFF") == 0)
    {
        gbVifDumpChnBank = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
        s32Ret = MI_ERR_VIF_NOT_SUPPORT;
    }

    handle.OnPrintOut(handle, "VifDumpChnBank %d\n", gbVifDumpChnBank);

    return MI_SUCCESS;
}

static MI_S32 _MI_VIF_ProcDumpDmaBank(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    if(strcmp(argv[1], "ON") == 0)
    {
        gbVifDumpDmaBank = TRUE;
    }
    else if(strcmp(argv[1], "OFF") == 0)
    {
        gbVifDumpDmaBank = FALSE;
    }
    else
    {
        handle.OnPrintOut(handle, "Unsupport command: %s.\n", argv[0]);
        s32Ret = MI_ERR_VIF_NOT_SUPPORT;
    }

    handle.OnPrintOut(handle, "VifDumpDmaBank %d\n", gbVifDumpDmaBank);

    return MI_SUCCESS;
}

static MI_S32 _MI_VIF_ProcDumpFrame(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if (MI_VIF_FRAME_DUMP == 1)
    MI_VIF_CHN u32ChnIndex;
    MI_VIF_PORT u32PortIndex;
    mi_vif_PortContext_t* pPortCtx;

    MI_SYS_BUG_ON(handle.OnPrintOut == NULL);

    kstrtouint(argv[1], 10, &u32ChnIndex);
    kstrtouint(argv[2], 10, &u32PortIndex);
    pPortCtx = &gstVifChnCtx[u32ChnIndex].stPortCtx[u32PortIndex];

    pPortCtx->bDumpFrame = TRUE;
    strcpy(pPortCtx->s8DumpPath, argv[3]);

    handle.OnPrintOut(handle, "dump %d %d %s\n", u32ChnIndex, u32PortIndex, pPortCtx->s8DumpPath);
#endif
    return s32Ret;
}

#endif

MI_S32 MI_VIF_IMPL_Init(void)
{
    mi_sys_ModuleDevBindOps_t stVIFPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    MI_VIF_CHN u32VifChn;
    MI_VIF_PORT u32ChnPort;
    //struct sched_param param;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    DBG_ENTER();

    if(gbVifInit)
    {
        DBG_ERR("MI_VIF already Inited, return ok !\n");
        MI_SYS_BUG();
        return MI_SUCCESS;
    }

    //memset(&gstVifDevCtx,0,sizeof(gstVifDevCtx));
    //memset(&gstVifChnCtx,0,sizeof(gstVifChnCtx));
    memset(&stVIFPOps, 0, sizeof(mi_sys_ModuleDevBindOps_t));
    stVIFPOps.OnBindOutputPort   = MI_VIF_OnBindChnnOutputCallback;
    stVIFPOps.OnUnBindOutputPort = MI_VIF_OnUnBindChnnOutputCallback;
    memset(&stModInfo, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId          = E_MI_MODULE_ID_VIF;
    stModInfo.u32DevId           = 0;
    stModInfo.u32DevChnNum       = MI_VIF_MAX_PHYCHN_NUM;
    stModInfo.u32InputPortNum    = 0;
    stModInfo.u32OutputPortNum   = MI_VIF_MAX_CHN_OUTPORT;

    if(hVifHandle)
    {
        DBG_ERR("vif already init!\n");
        MI_SYS_BUG();
        goto __vif_already_init_;
    }

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VIF_PROCFS_DEBUG == 1)
    memset(&pstModuleProcfsOps, 0, sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_VIF_ProcOnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_VIF_ProcOnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_VIF_ProcOnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_VIF_ProcOnHelp;
#else
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif


    hVifHandle  = mi_sys_RegisterDev(&stModInfo, &stVIFPOps, NULL
#ifdef MI_SYS_PROC_FS_DEBUG
                                     , &pstModuleProcfsOps
                                     , MI_COMMON_GetSelfDir
#endif
                                    );

    if(hVifHandle == NULL)
    {
        DBG_ERR("mi_sys_RegisterDev failed!\n");
        MI_SYS_BUG();
        goto __register_vif_dev_fail;
    }
    else
    {
        DBG_INFO("mi_sys_Register Vif Dev success! %p\n", hVifHandle);
    }

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_VIF_PROCFS_DEBUG == 1)
    mi_sys_RegistCommand("dump", 3, _MI_VIF_ProcDumpFrame, hVifHandle);
    mi_sys_RegistCommand("dump_bank_dma", 1, _MI_VIF_ProcDumpDmaBank, hVifHandle);
    mi_sys_RegistCommand("dump_bank_clock", 1, _MI_VIF_ProcDumpClockBank, hVifHandle);
    mi_sys_RegistCommand("dump_bank_chn", 1, _MI_VIF_ProcDumpChnBank, hVifHandle);
    mi_sys_RegistCommand("check_timeout", 3, _MI_VIF_ProcTimeout, hVifHandle);
    mi_sys_RegistCommand("check_timeout_all", 1, _MI_VIF_ProcTimeoutAll, hVifHandle);
    mi_sys_RegistCommand("disableport", 2, _MI_VIF_ProcDisableChnPort, hVifHandle);
    mi_sys_RegistCommand("enableport", 2, _MI_VIF_ProcEnableChnPort, hVifHandle);
#endif

    if(MHAL_SUCCESS != MHal_VIF_Init())
    {
        DBG_ERR(" MHalVifInit failed!\n");
        MI_SYS_BUG();
        goto  __init_vif_hal_fail;
    }
    else
    {
        DBG_INFO("MHal_VIF_Init success!\n");
    }

    init_completion(&gstVifProcessWakeup);

#if (MI_VIF_PORT_BUFFER_LIST == 1)
    gstpVifBufferCache = kmem_cache_create("gstpVifBufferCache", sizeof(mi_vif_BufTrackInfo_t), 0, SLAB_HWCACHE_ALIGN, NULL);
    MI_SYS_BUG_ON(!gstpVifBufferCache);
#endif

    for(u32VifChn = 0; u32VifChn < MI_VIF_MAX_PHYCHN_NUM; u32VifChn++)
    {
        mutex_init(&gstVifChnCtx[u32VifChn].stLock);

        for(u32ChnPort = 0; u32ChnPort < MI_VIF_MAX_CHN_OUTPORT; u32ChnPort++)
        {
#if (MI_VIF_TIMEOUT_CHECK > 0)
            _mi_vif_timeout_init(u32VifChn, u32ChnPort);
#endif
        }
    }

    gpstVifWorkTask = kthread_create(_mi_vif_WorkTask, NULL, "_mi_vif_WorkTask");

    if(IS_ERR(gpstVifWorkTask))
    {
        DBG_ERR("run mi_vif_work_tsk failed!\n");
        MI_SYS_BUG();
        goto __create_work_thread_fail;
    }
    else
    {
        DBG_INFO("create _mi_vif_WorkTask success!\n");
    }

    //param.sched_priority = 99;
    //sched_setscheduler(gpstVifWorkTask, SCHED_RR, &param);
    wake_up_process(gpstVifWorkTask);

    init_timer(&gstVifTimer);
    gstVifTimer.expires = jiffies + HZ / 100;
    gstVifTimer.function = mi_vif_timer_func;
    add_timer(&gstVifTimer);

#if MI_VIF_FRAME_CHECK==1

    if(MI_SUCCESS != MI_GFX_Open())
    {
        DBG_ERR("Gfx open error!\n");
    }

#endif

    gbVifInit = TRUE;

    return MI_SUCCESS;

__create_work_thread_fail:
    MI_SYS_BUG_ON(MHAL_SUCCESS != MHal_VIF_Deinit());
__init_vif_hal_fail:
    MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_UnRegisterDev(hVifHandle));
__register_vif_dev_fail:
__vif_already_init_:
    return E_MI_ERR_FAILED;
}

MI_S32 MI_VIF_IMPL_DeInit(void)
{
    DBG_ENTER();

    if(gbVifInit == FALSE)
    {
        DBG_INFO("MI_VIF already deinit, return ok !\n");
        MI_SYS_BUG();
        return MI_SUCCESS;
    }

    del_timer(&gstVifTimer);
    complete(&gstVifProcessWakeup);

    if(!IS_ERR(gpstVifWorkTask))
    {
        DBG_INFO("gpstVifWorkTask stop begin\n");
        kthread_stop(gpstVifWorkTask);
        DBG_INFO("gpstVifWorkTask stop end\n");
        gpstVifWorkTask = NULL;
    }

    if(hVifHandle == NULL)
    {
        DBG_ERR("_vif_again_Deinit failed!\n");
        MI_SYS_BUG();
    }

    if(MHAL_SUCCESS != MHal_VIF_StopMCU())
    {
        DBG_ERR("MHal_VIF_StopMCU failed!\n");
        MI_SYS_BUG();
    }

    _MI_VIF_FlushBuffer();

#if (MI_VIF_PORT_BUFFER_LIST == 1)
    kmem_cache_destroy(gstpVifBufferCache);
    gstpVifBufferCache = NULL;
#endif

    if(MHAL_SUCCESS != MHal_VIF_Deinit())
    {
        DBG_ERR("create MHal_VIF_Deinit failed!\n");
        MI_SYS_BUG();
    }

    if(hVifHandle == NULL)
    {
        DBG_ERR("hVifHandle NULL!\n");
        MI_SYS_BUG();
    }
    else
    {
        MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_UnRegisterDev(hVifHandle));
        hVifHandle = NULL;
    }

    gbVifInit = FALSE;
#if (MI_VIF_FRAME_CHECK==1)
    MI_GFX_Close();
#endif
    DBG_EXIT_OK();
    return 0;
}

static MS_S32 _MI_VIF_IMPL_MiSysAlloc(MS_U8 *pu8Name, MS_U32 u32Size, MS_PHYADDR * phyAddr)
{
    MI_SYS_BUG_ON(NULL == phyAddr);

    return (MS_S32)mi_sys_MMA_Alloc((MI_U8*)pu8Name, (MI_U32)u32Size, (MI_PHY *)phyAddr);
}
static MS_S32 _MI_VIF_IMPL_MiSysFree (MS_PHYADDR phyAddr)
{
    return (MS_S32)mi_sys_MMA_Free((MI_PHY)phyAddr);
}

static MS_S32 _MI_VIF_IMPL_MiSysFlashCache(void *pVirtAddr, MS_U32 u32Size)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);

    return (MS_S32)mi_sys_VFlushInvCache(pVirtAddr, (MI_U32)u32Size);
}

static void* _MI_VIF_IMPL_MiSysMap(MS_PHYADDR u64PhyAddr, MS_U32 u32Size, MS_BOOL bCache)
{
    return mi_sys_Vmap((MI_PHY)u64PhyAddr, (MI_U32)u32Size, (MI_BOOL)bCache);
}

static void _MI_VIF_IMPL_MiSysUnMap(void *pVirtAddr)
{
    MI_SYS_BUG_ON(NULL == pVirtAddr);
    return mi_sys_UnVmap(pVirtAddr);
}

MI_S32 MI_VIF_IMPL_SetDevAttr(MI_VIF_DEV u32VifDev, MI_VIF_DevAttr_t *pstDevAttr)
{
    MHal_VIF_DevCfg_t  stDevAttr;
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];
    MHalAllocPhyMem_t stAlloc;

    memset(&stAlloc, 0, sizeof(MHalAllocPhyMem_t));
    DBG_ENTER();

    MI_VIF_CHECK_PARA_RANGE(u32VifDev, 0, MI_VIF_MAX_DEV_NUM, MI_ERR_VIF_INVALID_DEVID);
    MI_VIF_CHECK_POINTER(pstDevAttr);

    if(pDevctx->bEnable == TRUE)
    {
        DBG_ERR("dev %d has enabled!\n", u32VifDev);
        return MI_ERR_VIF_FAILED_DEVNOTDISABLE;
    }

    _mi_vif_CheckDevAttr(pstDevAttr);

    stDevAttr.eIntfMode                            = pstDevAttr->eIntfMode;
    stDevAttr.eWorkMode                            = pstDevAttr->eWorkMode;
    stDevAttr.eHDRMode                             = pstDevAttr->eHDRType;
    memcpy(&stDevAttr.au32CompMask[0], &pstDevAttr->au32CompMask[0], 8);
    stDevAttr.eClkEdge                             = pstDevAttr->eClkEdge;
    memcpy(&stDevAttr.as32AdChnId[0], &pstDevAttr->as32AdChnId[0], 16);
    stDevAttr.eDataSeq                             = pstDevAttr->eDataSeq;

    stDevAttr.stSynCfg.eHsync                      = pstDevAttr->stSynCfg.eHsync;
    stDevAttr.stSynCfg.eHsyncNeg                   = pstDevAttr->stSynCfg.eHsyncNeg;
    stDevAttr.stSynCfg.eVsync                      = pstDevAttr->stSynCfg.eVsync;
    stDevAttr.stSynCfg.eVsyncNeg                   = pstDevAttr->stSynCfg.eVsyncNeg;
    stDevAttr.stSynCfg.eVsyncValid                 = pstDevAttr->stSynCfg.eVsyncValid;
    stDevAttr.stSynCfg.eVsyncValidNeg              = pstDevAttr->stSynCfg.eVsyncValidNeg;

    stDevAttr.stSynCfg.stTimingBlank.u32HsyncAct   = pstDevAttr->stSynCfg.stTimingBlank.u32HsyncAct;
    stDevAttr.stSynCfg.stTimingBlank.u32HsyncHbb   = pstDevAttr->stSynCfg.stTimingBlank.u32HsyncHbb;
    stDevAttr.stSynCfg.stTimingBlank.u32HsyncHfb   = pstDevAttr->stSynCfg.stTimingBlank.u32HsyncHfb;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVact  = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVact;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVbact = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbact;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVbb   = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbb;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVbbb  = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbbb;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVbfb  = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVbfb;
    stDevAttr.stSynCfg.stTimingBlank.u32VsyncVfb   = pstDevAttr->stSynCfg.stTimingBlank.u32VsyncVfb;

    stDevAttr.bDataRev                             = pstDevAttr->bDataRev;

    stAlloc.alloc = _MI_VIF_IMPL_MiSysAlloc;
    stAlloc.free  = _MI_VIF_IMPL_MiSysFree;
    stAlloc.map   = _MI_VIF_IMPL_MiSysMap;
    stAlloc.unmap = _MI_VIF_IMPL_MiSysUnMap;
    stAlloc.flush_cache = _MI_VIF_IMPL_MiSysFlashCache;

    if(MHAL_SUCCESS != MHal_VIF_DevSetConfig(u32VifDev, &stDevAttr, &stAlloc))
    {
        DBG_ERR("DevSetConfig failed!\n");
        MI_SYS_BUG();
        return E_MI_ERR_FAILED;
    }

    memcpy(&gstVifDevCtx[u32VifDev].stDevAttr, &stDevAttr, sizeof(MI_VIF_DevAttr_t));

    DBG_EXIT_OK();

    return MI_SUCCESS;
}

MI_S32 MI_VIF_IMPL_GetDevAttr(MI_VIF_DEV u32VifDev, MI_VIF_DevAttr_t *pstDevAttr)
{
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];
    DBG_ENTER();

    MI_VIF_CHECK_PARA_RANGE(u32VifDev, 0, MI_VIF_MAX_DEV_NUM, MI_ERR_VIF_INVALID_DEVID);
    MI_VIF_CHECK_POINTER(pstDevAttr);

    if(pDevctx->bEnable == FALSE)
    {
        DBG_ERR("dev %d not enable!\n", u32VifDev);
        MI_SYS_BUG();
        return MI_ERR_VIF_FAILED_DEVNOTENABLE;
    }

    memcpy(pstDevAttr, &gstVifDevCtx[u32VifDev].stDevAttr, sizeof(MI_VIF_DevAttr_t));

    DBG_EXIT_OK();

    return MI_SUCCESS;
}

MI_S32 MI_VIF_IMPL_EnableDev(MI_VIF_DEV u32VifDev)
{
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];

    MI_VIF_CHECK_PARA_RANGE(u32VifDev, 0, MI_VIF_MAX_DEV_NUM, MI_ERR_VIF_INVALID_DEVID);

    DBG_INFO(">>> dev:%d bEnable:%d\n", u32VifDev, pDevctx->bEnable);

    if(pDevctx->bEnable == FALSE)
    {
        if(MHAL_SUCCESS != MHal_VIF_DevEnable(u32VifDev))
        {
            DBG_ERR("DevSetConfig failed!\n");
            return E_MI_ERR_FAILED;
        }

        pDevctx->bEnable = TRUE;
    }

    DBG_INFO("<<< dev: %d isEnable: %d\n", u32VifDev, pDevctx->bEnable);

    return MI_SUCCESS;
}

MI_S32 MI_VIF_IMPL_DisableDev(MI_VIF_DEV u32VifDev)
{
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];

    DBG_INFO(">>> u32VifDev:%d\n", u32VifDev);

    MI_VIF_CHECK_PARA_RANGE(u32VifDev, 0, MI_VIF_MAX_DEV_NUM, MI_ERR_VIF_INVALID_DEVID);

    if(pDevctx->bEnable == TRUE)
    {
        if(MHAL_SUCCESS != MHal_VIF_DevDisable(u32VifDev))
        {
            DBG_ERR("DisableDev failed!\n");
            return E_MI_ERR_FAILED;
        }

        pDevctx->bEnable = FALSE;
    }

    DBG_INFO("<<< u32VifDev: %d isEnable: %d\n", u32VifDev, pDevctx->bEnable);

    return MI_SUCCESS;
}

MI_S32 MI_VIF_IMPL_SetChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr)
{
    MI_VIF_DEV u32VifDev = u32VifChn / 4;
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32VifChn];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32ChnPort];
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];
    MI_S32 s32Ret = MI_SUCCESS;

    DBG_INFO(">>> u32VifChn:%d u32ChnPort:%d\n", u32VifChn, u32ChnPort);

    MI_VIF_CHECK_PARA_RANGE(u32VifChn, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);
    MI_VIF_CHECK_PARA_RANGE(u32ChnPort, 0, MI_VIF_MAX_CHN_OUTPORT, MI_ERR_VIF_INVALID_PORTID);
    MI_VIF_CHECK_POINTER(pstAttr);

    if(pDevctx->bEnable == FALSE)
    {
        DBG_ERR("dev %d not enabled!\n", u32VifDev);
        return MI_ERR_VIF_FAILED_DEVNOTENABLE;
    }

    if(pDevctx->stDevAttr.eIntfMode == E_MI_VIF_MODE_MIPI ||
            pDevctx->stDevAttr.eIntfMode == E_MI_VIF_MODE_DIGITAL_CAMERA)
    {
        if(u32ChnPort == 1)
        {
            DBG_ERR("dev %d not support port %d!\n", u32VifDev, u32ChnPort);
            return MI_ERR_VIF_NOT_SUPPORT;
        }
    }
    if(pPortCtx->bEnable == TRUE)
    {
        if (_mi_vif_CompareFrameRate(&pPortCtx->stChnPortAttr, pstAttr))
        {
            DBG_ERR("chn %d port :%d has enabled!\n", u32VifChn, u32ChnPort);
            return MI_ERR_VIF_FAILED_PORTNOTDISABLE;
        }
    }

    s32Ret = _mi_vif_CheckChnPortAttr(u32VifChn, u32ChnPort, pstAttr);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    mutex_lock(&pChnCtx->stLock);

    if(u32ChnPort == 0)
    {
        MHal_VIF_ChnCfg_t stChnAttr;
        stChnAttr.stCapRect.u16X            = pstAttr->stCapRect.u16X;
        stChnAttr.stCapRect.u16Y            = pstAttr->stCapRect.u16Y;
        stChnAttr.stCapRect.u16Width        = pstAttr->stCapRect.u16Width;
        stChnAttr.stCapRect.u16Height       = pstAttr->stCapRect.u16Height;

        stChnAttr.eScanMode                 = (MHalFrameScanMode_e)pstAttr->eScanMode;
        stChnAttr.ePixFormat                = (MHalPixelFormat_e)pstAttr->ePixFormat;
        stChnAttr.eFrameRate                = (MHal_VIF_FrameRate_e)pstAttr->eFrameRate;
        stChnAttr.eCapSel                   = (MHalFieldType_e)pstAttr->eCapSel;

        stChnAttr.OnFrameStart              = NULL;
        stChnAttr.OnFrameLineCount          = NULL;
        if ((0 == u32VifChn) || (4 == u32VifChn) || (8 == u32VifChn) || (12 == u32VifChn))
        {
            stChnAttr.OnFrameEnd = _MI_ISR_FrameEndCallback;
        }
        else
        {
            stChnAttr.OnFrameEnd = NULL;
        }

        if(pDevctx->stDevAttr.eWorkMode == E_MI_VIF_WORK_MODE_RGB_FRAMEMODE)
        {
            if(pstAttr->u32FrameModeLineCount)
            {
            //#if LOW_LATENCY_ENABLE
                stChnAttr.u32FrameModeLineCount = pstAttr->u32FrameModeLineCount;
                stChnAttr.OnFrameLineCount      = _MI_ISR_FrameLineCountCallback;
            //#else
            //    stChnAttr.u32FrameModeLineCount = 0;
            //    stChnAttr.OnFrameLineCount      = NULL;
            //#endif
            }
        }
        else if(pDevctx->stDevAttr.eWorkMode == E_MI_VIF_WORK_MODE_RGB_REALTIME)
        {
            stChnAttr.OnFrameEnd = NULL;
        }

        if(MHAL_SUCCESS != MHal_VIF_ChnSetConfig(u32VifChn, &stChnAttr))
        {
            DBG_ERR("ChnSetConfig failed!\n");
            MI_SYS_BUG();
            s32Ret = E_MI_ERR_FAILED;
            goto done;
        }
    }
    else if(u32ChnPort == 1)
    {
        MHal_VIF_SubChnCfg_t stSubChnAttr;
        stSubChnAttr.eFrameRate  = pstAttr->eFrameRate;
        if ((0 == u32VifChn) || (4 == u32VifChn) || (8 == u32VifChn) || (12 == u32VifChn))
        {
            stSubChnAttr.OnFrameEnd = _MI_ISR_FrameEndCallback;
        }
        else
        {
            stSubChnAttr.OnFrameEnd = NULL;
        }

        if(MHAL_SUCCESS != MHal_VIF_SubChnSetConfig(u32VifChn, &stSubChnAttr))
        {
            DBG_ERR("SubChnSetConfig failed!\n");
            MI_SYS_BUG();
            s32Ret = E_MI_ERR_FAILED;
            goto done;
        }
    }

    pPortCtx->stChnPortAttr = *pstAttr;
done:
    mutex_unlock(&pChnCtx->stLock);
    return s32Ret;
}

MI_S32 MI_VIF_IMPL_GetChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr)
{
    mi_vif_PortContext_t* pChnCtx = &gstVifChnCtx[u32VifChn].stPortCtx[u32ChnPort];

    DBG_INFO(">>> u32VifChn:%d u32ChnPort:%d\n", u32VifChn, u32ChnPort);

    MI_VIF_CHECK_PARA_RANGE(u32VifChn, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);
    MI_VIF_CHECK_PARA_RANGE(u32ChnPort, 0, MI_VIF_MAX_CHN_OUTPORT, MI_ERR_VIF_INVALID_PORTID);
    MI_VIF_CHECK_POINTER(pstAttr);
    memcpy(pstAttr, &pChnCtx->stChnPortAttr, sizeof(MI_VIF_ChnPortAttr_t));
    return MI_SUCCESS;
}

MI_S32 MI_VIF_IMPL_EnableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort)
{
    MI_VIF_DEV u32VifDev = u32VifChn / 4;
    mi_vif_DevContext_t* pDevctx = &gstVifDevCtx[u32VifDev];
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32VifChn];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32ChnPort];
    MI_S32 s32Ret = MI_SUCCESS;

    DBG_INFO(">>> u32VifChn:%d u32ChnPort:%d\n", u32VifChn, u32ChnPort);

    MI_VIF_CHECK_PARA_RANGE(u32VifChn, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);
    MI_VIF_CHECK_PARA_RANGE(u32ChnPort, 0, MI_VIF_MAX_CHN_OUTPORT, MI_ERR_VIF_INVALID_PORTID);

    if(pDevctx->bEnable == FALSE)
    {
        DBG_ERR("dev %d not enabled!\n", u32VifDev);
        return MI_ERR_VIF_FAILED_DEVNOTENABLE;
    }

    mutex_lock(&pChnCtx->stLock);

    if(pPortCtx->bEnable == FALSE)
    {
        if(u32ChnPort == 0)
        {
            if(MHAL_SUCCESS != MHal_VIF_ChnEnable(u32VifChn))
            {
                DBG_ERR("ChnEnable failed!\n");
                s32Ret = E_MI_ERR_FAILED;
                goto done;
            }
        }
        else if(u32ChnPort == 1)
        {
            if(MHAL_SUCCESS != MHal_VIF_SubChnEnable(u32VifChn))
            {
                DBG_ERR("SubChnEnable failed!\n");
                s32Ret = E_MI_ERR_FAILED;
                goto done;
            }
        }

        MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_EnableChannel(hVifHandle, u32VifChn));
        MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_EnableOutputPort(hVifHandle, u32VifChn, u32ChnPort));

#if (MI_VIF_PORT_BUFFER_LIST == 1)
        INIT_LIST_HEAD(&pPortCtx->stBufferList);
#endif

        pPortCtx->bEnable = TRUE;
    }

done:
    mutex_unlock(&pChnCtx->stLock);
    return s32Ret;
}

MI_S32 MI_VIF_IMPL_DisableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort)
{
    MI_S32 s32Ret = MI_SUCCESS;
    mi_vif_ChnContext_t* pChnCtx = &gstVifChnCtx[u32VifChn];
    mi_vif_PortContext_t* pPortCtx = &pChnCtx->stPortCtx[u32ChnPort];

    DBG_INFO(">>> u32VifChn:%d u32ChnPort:%d\n", u32VifChn, u32ChnPort);

    MI_VIF_CHECK_PARA_RANGE(u32VifChn, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);

    mutex_lock(&pChnCtx->stLock);

    if(pPortCtx->bEnable == TRUE)
    {
        pPortCtx->bEnable = FALSE;
        _MI_VIF_FlushPortBuffer(u32VifChn, u32ChnPort);
        MI_SYS_BUG_ON(MI_SUCCESS != mi_sys_DisableOutputPort(hVifHandle, u32VifChn, u32ChnPort));

        if(u32ChnPort == 0)
        {
            if(MHAL_SUCCESS != MHal_VIF_ChnDisable(u32VifChn))
            {
                DBG_ERR("ChnDisable failed!\n");
                MI_SYS_BUG();
                s32Ret = E_MI_ERR_FAILED;
                goto done;
            }
        }
        else if(u32ChnPort == 1)
        {
            if(MHAL_SUCCESS != MHal_VIF_SubChnDisable(u32VifChn))
            {
                DBG_ERR("SubChnDisable failed!\n");
                MI_SYS_BUG();
                s32Ret = E_MI_ERR_FAILED;
                goto done;
            }
        }

        pPortCtx->u32HalBufferHold = 0;
        pPortCtx->u32FrameCountJiffies = 0;
        pPortCtx->u32FrameRate = 0;
    }

done:
    mutex_unlock(&pChnCtx->stLock);
    return s32Ret;
}

MI_S32 MI_VIF_IMPL_Query(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortStat_t *pstStat)
{
    mi_vif_PortContext_t* pChnCtx = &gstVifChnCtx[u32VifChn].stPortCtx[u32ChnPort];
    MHal_VIF_ChnStat_t stMhalChnStat;
    MS_BOOL err;
    DBG_ENTER();

    MI_VIF_CHECK_PARA_RANGE(u32VifChn, 0, MI_VIF_MAX_PHYCHN_NUM, MI_ERR_VIF_INVALID_CHNID);
    MI_VIF_CHECK_PARA_RANGE(u32ChnPort, 0, MI_VIF_MAX_CHN_OUTPORT, MI_ERR_VIF_INVALID_PORTID);
    MI_VIF_CHECK_POINTER(pstStat);

    DBG_INFO("ChnID:%d	PortID: %d\n", u32VifChn, u32ChnPort);

    pstStat->bEnable       = pChnCtx->bEnable;
    //pstStat->eFrameRate    = pChnCtx->stChnPortAttr.eFrameRate;
    //pstStat.u32IntCnt     = pChnCtx->;    // how support ???
    //pstStat.u32LostInt    =  stpstStat.u32LostInt;  // how support ???
    pstStat->u32PicHeight  =  pChnCtx->stChnPortAttr.stDestSize.u16Height;
    pstStat->u32PicWidth   =  pChnCtx->stChnPortAttr.stDestSize.u16Width;
    pstStat->u32FrameRate  =  pChnCtx->u32FrameRate;

    if(u32ChnPort == 0)
    {
        err = MHal_VIF_ChnQuery(u32VifChn, &stMhalChnStat);
    }
    else
    {
        err = MHal_VIF_SubChnQuery(u32VifChn, &stMhalChnStat);
    }

    if(MHAL_SUCCESS == err)
    {
        pstStat->u32VbFail = stMhalChnStat.u32VbFail;
    }
    else
    {
        pstStat->u32VbFail = 0;
        MI_SYS_BUG();
    }

    return MI_SUCCESS;
}
