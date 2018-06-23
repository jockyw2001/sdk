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
/// @file   mi_shadow_impl.c
/// @brief shadow module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/idr.h>

#include "mi_print.h"
#include "mi_shadow_impl.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define DOWN(x)
#define UP(x)

#define MI_SHADOW_CHECK_INIT() \
    do\
    {\
        if (!_stDevMgr.bInit)\
        {\
            DBG_ERR("MI SHADOW Not Init\n");\
            return E_MI_ERR_FAILED;\
        }\
    } while (0)

#define _MTTEST_ (0)
//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct MI_SHADOW_DevInfo_s
{
    struct list_head list;
    MI_SHADOW_HANDLE hShadow;
    wait_queue_head_t stWaitCallback;
    wait_queue_head_t stWaitCallbackResult;
    MI_S32 s32CallbackResult;
    MI_SHADOW_ModuleDevInfo_t stRegDevParams;
    MI_SYS_ChnPort_t stOnCbChnCurryPort;
    MI_SYS_ChnPort_t stOnCbChnPeerPort;
    MI_SHADOW_CALLBACK_EVENT_e eLastCallbackEvent;
    unsigned char padding[32];
} MI_SHADOW_DevInfo_t;

typedef struct MI_SHADOW_BufInfo_s
{
    struct list_head list;
    MI_SHADOW_HANDLE hShadow;
    MI_SYS_BufInfo_t *pstSysBufInfo;
} MI_SHADOW_BufInfo_t;

typedef struct MI_SHADOW_DevMgr_s
{
    struct semaphore mutex;
    struct list_head listDevInfoHead;
    struct idr idrBufInfo;
    MI_BOOL bInit;
    struct kmem_cache *pstDevInfoCache;
} MI_SHADOW_DevMgr_t;

// TODO refine flag use
MI_U32 flag=0;

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

MI_SHADOW_DevMgr_t _stDevMgr;

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

/////////////////for bind callback start
MI_S32 MI_SHADOW_IMPL_WaitCallBack(
    MI_SHADOW_HANDLE hShadow,
    MI_S32 u32TimeOutMs,
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    MI_SHADOW_CALLBACK_EVENT_e *peCallbackEvent)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_DevInfo_t *p, *n;
    MI_SHADOW_DevInfo_t *pstDevInfo = NULL;
    MI_S32 s32WaitRes = -1;

    MI_SHADOW_CHECK_INIT();
    DOWN(&(_stDevMgr.mutex));
    list_for_each_entry_safe(p, n, &(_stDevMgr.listDevInfoHead), list)
    {
        pstDevInfo = list_entry(&(p->list), MI_SHADOW_DevInfo_t, list);
        if (pstDevInfo)
        {
            if (pstDevInfo->hShadow == hShadow)
            {
                s32Ret = MI_SUCCESS;
                break;
            }
        }
    }

    UP(&(_stDevMgr.mutex));

    if (MI_SUCCESS != s32Ret)
    {
        *peCallbackEvent = MI_SHADOW_CALLBACK_EVENT_EXIT;
        return s32Ret;
    }

    ///user wait for callback
    DBG_INFO("wait_event_interruptible_timeout:flag=%d\n", flag);
    s32WaitRes = wait_event_interruptible(pstDevInfo->stWaitCallback,flag != 0);
    DBG_INFO("wait_event_interruptible_timeout:\n");

    ///if dev unregister or exit, can't found dev info
    s32Ret = E_MI_ERR_FAILED;
    DOWN(&(_stDevMgr.mutex));
    list_for_each_entry_safe(p, n, &(_stDevMgr.listDevInfoHead), list)
    {
        pstDevInfo = list_entry(&(p->list), MI_SHADOW_DevInfo_t, list);
        if (pstDevInfo)
        {
            if (pstDevInfo->hShadow == hShadow)
            {
                s32Ret = MI_SUCCESS;
                *peCallbackEvent = pstDevInfo->eLastCallbackEvent;
                memcpy(pstChnCurryPort, &(pstDevInfo->stOnCbChnCurryPort), sizeof(MI_SYS_ChnPort_t));
                memcpy(pstChnPeerPort, &(pstDevInfo->stOnCbChnPeerPort), sizeof(MI_SYS_ChnPort_t));
                break;
            }
        }
    }
    flag = 0;
    UP(&(_stDevMgr.mutex));
    return s32Ret;
}

MI_S32 _MI_SHADOW_IMPL_SetWakeUpDevCallback(
    MI_SHADOW_DevInfo_t *pstDevInfo,
    MI_SHADOW_CALLBACK_EVENT_e eCallbackEvent,
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort)
{
    pstDevInfo->s32CallbackResult = E_MI_ERR_FAILED;
    pstDevInfo->eLastCallbackEvent = eCallbackEvent;

    memcpy(&(pstDevInfo->stOnCbChnCurryPort), pstChnCurryPort, sizeof(MI_SYS_ChnPort_t));

    memcpy(&(pstDevInfo->stOnCbChnPeerPort), pstChnPeerPort, sizeof(MI_SYS_ChnPort_t));

    DBG_INFO("wake_up_interruptible:before\n");
	flag = 1;
    wake_up_interruptible(&(pstDevInfo->stWaitCallback));
    DBG_INFO("wake_up_interruptible:after\n");



    if (MI_SHADOW_CALLBACK_EVENT_EXIT != eCallbackEvent)
    {
        ///need to wait exec callback function result

        if (!wait_event_interruptible_timeout(pstDevInfo->stWaitCallbackResult,TRUE, msecs_to_jiffies(1000)))
        {

            pstDevInfo->s32CallbackResult = E_MI_ERR_FAILED;
        }
    }
    else
    {

        pstDevInfo->s32CallbackResult = MI_SUCCESS;
    }

    DBG_INFO("hShadow:0x%x callback_result:%d\n", pstDevInfo->hShadow, pstDevInfo->s32CallbackResult);
    return pstDevInfo->s32CallbackResult;
}

void MI_SHADOW_IMPL_SetCallbackResult(MI_SHADOW_HANDLE hShadow, MI_S32 s32CallbackResult)
{
    MI_SHADOW_DevInfo_t *p, *n;
    MI_SHADOW_DevInfo_t *pstDevInfo = NULL;

    if (!_stDevMgr.bInit)
    {
        DBG_ERR("MI SHADOW Not Init\n");
        return;
    }

    DOWN(&(_stDevMgr.mutex));
    list_for_each_entry_safe(p, n, &(_stDevMgr.listDevInfoHead), list)
    {
        pstDevInfo = list_entry(&(p->list), MI_SHADOW_DevInfo_t, list);
        if (pstDevInfo)
        {
            if (pstDevInfo->hShadow == hShadow)
            {
                pstDevInfo->s32CallbackResult = s32CallbackResult;
                wake_up_interruptible(&(pstDevInfo->stWaitCallbackResult));
                break;
            }
        }
    }
    UP(&(_stDevMgr.mutex));
}

MI_S32 MI_SHADOW_IMPL_SetChannelStatus(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_BOOL bEnable,
    MI_BOOL bChn,
    MI_BOOL bInputPort)
{
    MI_S32 s32Res = E_MI_ERR_FAILED;



    if (bChn)
    {
        ///channel
        if (bEnable)
        {

            s32Res = mi_sys_EnableChannel((MI_SYS_DRV_HANDLE)hShadow, u32ChnId);
        }
        else
        {

            s32Res = mi_sys_DisableChannel((MI_SYS_DRV_HANDLE)hShadow, u32ChnId);
        }
    }
    else
    {

        ///port
        if (bInputPort)
        {

            ///input port
            if (bEnable)
            {

                s32Res = mi_sys_EnableInputPort((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId);
            }
            else
            {

                s32Res = mi_sys_DisableInputPort((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId);
            }
        }
        else
        {
            ///output port
            if (bEnable)
            {

                s32Res = mi_sys_EnableOutputPort((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId);
            }
            else
            {

                s32Res = mi_sys_DisableOutputPort((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId);
            }
        }
    }

    DBG_EXIT_OK();
    return s32Res;
}


static MI_S32 _MI_SHADOW_IMPL_OnBindInputPort(
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    void *pUsrData)
{
    MI_SHADOW_DevInfo_t *pstDevInfo = (MI_SHADOW_DevInfo_t *)pUsrData;
    DBG_INFO("pstDevInfo:%p\n", pstDevInfo);

    return _MI_SHADOW_IMPL_SetWakeUpDevCallback(
        pstDevInfo,
        MI_SHADOW_CALLBACK_EVENT_ONBINDINPUTPORT,
        pstChnCurryPort,
        pstChnPeerPort);
}

static MI_S32 _MI_SHADOW_IMPL_OnBindOutputPort(
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    void *pUsrData)
{
    MI_SHADOW_DevInfo_t *pstDevInfo = (MI_SHADOW_DevInfo_t *)pUsrData;
    DBG_INFO("pstDevInfo:%p\n", pstDevInfo);

    return _MI_SHADOW_IMPL_SetWakeUpDevCallback(
        pstDevInfo,
        MI_SHADOW_CALLBACK_EVENT_ONBINDOUTPUTPORT,
        pstChnCurryPort,
        pstChnPeerPort);
}

static MI_S32 _MI_SHADOW_IMPL_OnUnBindInputPort(
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    void *pUsrData)
{
    MI_SHADOW_DevInfo_t *pstDevInfo = (MI_SHADOW_DevInfo_t *)pUsrData;
    DBG_INFO("pstDevInfo:%p\n", pstDevInfo);

    return _MI_SHADOW_IMPL_SetWakeUpDevCallback(
        pstDevInfo,
        MI_SHADOW_CALLBACK_EVENT_ONUNBINDINPUTPORT,
        pstChnCurryPort,
        pstChnPeerPort);
}

static MI_S32 _MI_SHADOW_IMPL_OnUnBindOutputPort(
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    void *pUsrData)
{
    MI_SHADOW_DevInfo_t *pstDevInfo = (MI_SHADOW_DevInfo_t *)pUsrData;
    DBG_INFO("pstDevInfo:%p\n", pstDevInfo);

    return _MI_SHADOW_IMPL_SetWakeUpDevCallback(
        pstDevInfo,
        MI_SHADOW_CALLBACK_EVENT_ONUNBINDOUTPUTPORT,
        pstChnCurryPort,
        pstChnPeerPort);
}

/////////////////for bind callback end
#if _MTTEST_
static MI_PHY _MI_SHADOW_GetBufPhyAddr(MI_SYS_BufInfo_t *pstBufInfo)
{
    MI_PHY phyAddr = 0;
    switch (pstBufInfo->eBufType)
    {
        case E_MI_SYS_BUFDATA_RAW:
        {
            phyAddr = pstBufInfo->stRawData.phyAddr;
        } break;
        case E_MI_SYS_BUFDATA_FRAME:
        {
            phyAddr = pstBufInfo->stFrameData.phyAddr[0];
        } break;
        case E_MI_SYS_BUFDATA_META:
        {
            phyAddr = pstBufInfo->stMetaData.phyAddr;
        } break;
        default:
            phyAddr = 0;

    }

    return phyAddr;
}
#endif

static MI_SYS_BufInfo_t *_MI_SHADOW_GetSysBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BUF_HANDLE hBufHandle)
{
    MI_SYS_BufInfo_t *pstBufInfo = NULL;
    void *phMapToUserBuf = NULL;

    DBG_INFO("hBufHandle:0x%x\n", hBufHandle);
    DOWN(&(_stDevMgr.mutex));
    phMapToUserBuf = idr_find(&(_stDevMgr.idrBufInfo), hBufHandle);
    if (!phMapToUserBuf)
    {
        DBG_ERR("Can't Found, hBufHandle:0x%x\n", hBufHandle);
        UP(&(_stDevMgr.mutex));
        return NULL;
    }

    idr_remove(&(_stDevMgr.idrBufInfo), hBufHandle);
    UP(&(_stDevMgr.mutex));

    if (MI_SUCCESS != mi_sys_UnmapBufToUser(miSysDrvHandle, phMapToUserBuf, &pstBufInfo))
    {
        DBG_ERR("Can't Find Unmap Buf To User\n");
        return NULL;
    }

#if _MTTEST_
    {
        MI_PHY phyShadowAddr = 0;
        phyShadowAddr = _MI_SHADOW_GetBufPhyAddr(pstBufInfo);
        DBG_INFO("Buffer(%p) phyShadowAddr:0x%llx\n", pstBufInfo, phyShadowAddr);
    }
#endif
    DBG_INFO("pstBufInfo:%p\n", pstBufInfo);
    return pstBufInfo;
}

MI_S32 _MI_SHADOW_IMPL_GetPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufConf_t *pstBufConfig,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_BOOL *pbBlockedByRateCtrl,
    MI_SYS_BUF_HANDLE *phBufHandle)
{
#define MI_SYS_INPUT_PORT_HANDLE_START 0x40
#define MI_SYS_INPUT_PORT_HANDLE_END (0x4000-1)
#define MI_SYS_OUTPUT_PORT_HANDLE_START 0x4000
#define MI_SYS_OUTPUT_PORT_HANDLE_END (0x8000-1)

    MI_SYS_BufInfo_t *pstSysBufInfo = NULL;
    MI_BOOL bGetOutputPortBuf = FALSE;
    void *phMapToUserBuf  = NULL;



    if (!(phBufHandle && pstBufInfo && pstBufInfo))
    {
        DBG_ERR("phBufHandle:%p, pstBufInfo:%p\n", phBufHandle, pstBufInfo);
        return E_MI_ERR_FAILED;
    }



    if (pstBufConfig)
    {
        bGetOutputPortBuf = TRUE;
    }




    if (bGetOutputPortBuf)
    {

        pstSysBufInfo = mi_sys_GetOutputPortBuf((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId, pstBufConfig, pbBlockedByRateCtrl);
    }
    else
    {

        pstSysBufInfo = mi_sys_GetInputPortBuf((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId, 0);
    }




    if (!pstSysBufInfo)
    {
        if (pstBufConfig)
        {
            DBG_INFO("Call Get Sys Output Port Buf Fail, hShadow:0x%x, u32ChnId:%d, u32PortId:%d\n",
                hShadow,
                u32ChnId,
                u32PortId);
            DBG_INFO("Call Get Sys Output Port Buf Fail, eBufType:%d, u64TargetPts:%d\n",
                pstBufConfig->eBufType,
                pstBufConfig->u64TargetPts);
        }
        else
        {
            DBG_INFO("Call Get Sys Input Port Buf Fail, hShadow:0x%x, u32ChnId:%d, u32PortId:%d\n",
                hShadow,
                u32ChnId,
                u32PortId);
        }
        return E_MI_ERR_FAILED;
    }





    if (MI_SUCCESS != mi_sys_MmapBufToUser((MI_SYS_DRV_HANDLE)hShadow, pstSysBufInfo, &phMapToUserBuf))
    {
        return E_MI_ERR_FAILED;
    }




    DBG_INFO("phMapToUserBuf:%p\n", phMapToUserBuf);
    DOWN(&(_stDevMgr.mutex));

    if (bGetOutputPortBuf)
    {
        *phBufHandle = idr_alloc(&(_stDevMgr.idrBufInfo), phMapToUserBuf, MI_SYS_OUTPUT_PORT_HANDLE_START, MI_SYS_OUTPUT_PORT_HANDLE_END, GFP_KERNEL);

    }
    else
    {
        *phBufHandle = idr_alloc(&(_stDevMgr.idrBufInfo), phMapToUserBuf,  MI_SYS_INPUT_PORT_HANDLE_START, MI_SYS_INPUT_PORT_HANDLE_END, GFP_KERNEL);
    }

    if(*phBufHandle < 0)
    {
        UP(&(_stDevMgr.mutex));

        mi_sys_UnmapBufToUser((MI_SYS_DRV_HANDLE)hShadow, phMapToUserBuf, &pstBufInfo);
        MI_SYS_BUG_ON(pstBufInfo != pstSysBufInfo);
        mi_sys_RewindBuf(pstSysBufInfo);
        return E_MI_ERR_FAILED;
    }
    UP(&(_stDevMgr.mutex));



    *pstBufInfo = *pstSysBufInfo;
    DBG_INFO("0x%x, %p\n", *phBufHandle, pstBufInfo);
#if _MTTEST_
    {
        MI_PHY phyAddr = 0;
        phyAddr = _MI_SHADOW_GetBufPhyAddr(pstBufInfo);
        DBG_INFO("Get %sport Buffer Done,hShadow:0x%x, phyAddr:0x%llx, 0x%x\n", (bGetOutputPortBuf ? "Out":"In"), hShadow, phyAddr, *phBufHandle);
    }
#endif

    DBG_EXIT_OK();
    return MI_SUCCESS;
}

void MI_SHADOW_IMPL_Init(void)
{
    DBG_ENTER();
    memset(&_stDevMgr, 0x0, sizeof(MI_SHADOW_DevMgr_t));
    INIT_LIST_HEAD(&(_stDevMgr.listDevInfoHead));
    sema_init(&(_stDevMgr.mutex), 1);
    idr_init(&(_stDevMgr.idrBufInfo));

    _stDevMgr.pstDevInfoCache = (struct kmem_cache *)kmem_cache_create(
        "mi_shadow_devinfo_cacheheap",
        sizeof(MI_SHADOW_DevInfo_t),
        0,
        SLAB_HWCACHE_ALIGN,
        NULL);
    _stDevMgr.bInit = TRUE;
}

void MI_SHADOW_IMPL_DeInit(void)
{
    MI_SHADOW_DevInfo_t *p, *n, *pstDevInfo;

    if (!_stDevMgr.bInit)
    {
        DBG_ERR("MI SHADOW Not Init\n");
        return;
    }

    DBG_ENTER();
    DOWN(&(_stDevMgr.mutex));
    list_for_each_entry_safe(p, n, &(_stDevMgr.listDevInfoHead), list)
    {
        pstDevInfo = list_entry(&(p->list), MI_SHADOW_DevInfo_t, list);
        list_del(&p->list);
        if (pstDevInfo)
        {
            ///exit user wait callback thread
            wake_up_interruptible(&(pstDevInfo->stWaitCallback));
            wake_up_interruptible(&(pstDevInfo->stWaitCallbackResult));
            kmem_cache_free(_stDevMgr.pstDevInfoCache, pstDevInfo);
        }
    }
    UP(&(_stDevMgr.mutex));

    idr_destroy(&(_stDevMgr.idrBufInfo));
    _stDevMgr.bInit = FALSE;
}

void _MI_SHADOW_IMPL_MTDevInit(MI_SHADOW_HANDLE hShadow);
MI_S32 MI_SHADOW_IMPL_RegisterDev(
    MI_SHADOW_ModuleDevInfo_t *pstModDevInfo,
    MI_SHADOW_HANDLE *phShadow)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_DevInfo_t *pstDevInfo = NULL;
    mi_sys_ModuleDevInfo_t stMouleInfo;
    mi_sys_ModuleDevBindOps_t stModuleBindOps;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    MI_SHADOW_CHECK_INIT();
    DBG_ENTER();
    if (!pstModDevInfo)
    {
        DBG_ERR("pstModDevInfo Is Null\n");
        return s32Ret;
    }

    pstDevInfo = (MI_SHADOW_DevInfo_t *)kmem_cache_alloc(_stDevMgr.pstDevInfoCache, GFP_KERNEL);

    memset(&stMouleInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    memset(&stModuleBindOps, 0x0, sizeof(mi_sys_ModuleDevBindOps_t));

    DBG_INFO("Register Device Info:eModuleId%d, u32DevId:%d, u32DevChnNum:%d, "
        "u32InputPortNum:%d, u32OutputPortNum:%d\n",
        pstModDevInfo->eModuleId,
        pstModDevInfo->u32DevId,
        pstModDevInfo->u32DevChnNum,
        pstModDevInfo->u32InputPortNum,
        pstModDevInfo->u32OutputPortNum);

    stMouleInfo.eModuleId = pstModDevInfo->eModuleId;
    stMouleInfo.u32DevId = pstModDevInfo->u32DevId;
    stMouleInfo.u32DevChnNum = pstModDevInfo->u32DevChnNum;
    stMouleInfo.u32InputPortNum = pstModDevInfo->u32InputPortNum;
    stMouleInfo.u32OutputPortNum = pstModDevInfo->u32OutputPortNum;

    stModuleBindOps.OnBindInputPort = _MI_SHADOW_IMPL_OnBindInputPort;
    stModuleBindOps.OnBindOutputPort = _MI_SHADOW_IMPL_OnBindOutputPort;
    stModuleBindOps.OnUnBindInputPort = _MI_SHADOW_IMPL_OnUnBindInputPort;
    stModuleBindOps.OnUnBindOutputPort = _MI_SHADOW_IMPL_OnUnBindOutputPort;
    DBG_INFO("pstDevInfo:%p\n", pstDevInfo);
#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = NULL;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = NULL;
#endif
    *phShadow = (MI_SHADOW_HANDLE)mi_sys_RegisterDev(&stMouleInfo, &stModuleBindOps, pstDevInfo
                                                      #ifdef MI_SYS_PROC_FS_DEBUG
                                                          , &pstModuleProcfsOps
                                                          ,MI_COMMON_GetSelfDir
                                                        #endif
                                                    );
    DBG_INFO("Register:%d, hShadow:0x%x, stModuleBindOps.OnBindInputPort:%p pstDevInfo:%p\n", stMouleInfo.eModuleId, *phShadow,
        stModuleBindOps.OnBindInputPort, pstDevInfo);
    if (MI_HANDLE_NULL != (*phShadow))
    {
        DOWN(&(_stDevMgr.mutex));
        if (pstDevInfo)
        {
            memset(pstDevInfo, 0x0, sizeof(MI_SHADOW_DevInfo_t));
            memcpy(&(pstDevInfo->stRegDevParams), pstModDevInfo, sizeof(MI_SHADOW_ModuleDevInfo_t));
            pstDevInfo->hShadow = *phShadow;
            init_waitqueue_head(&(pstDevInfo->stWaitCallback));
            init_waitqueue_head(&(pstDevInfo->stWaitCallbackResult));
            list_add_tail(&(pstDevInfo->list), &(_stDevMgr.listDevInfoHead));
#if _MTTEST_
            _MI_SHADOW_IMPL_MTDevInit(*phShadow);
#endif
            s32Ret = MI_SUCCESS;
        }
        UP(&(_stDevMgr.mutex));
    }
    else
    {
        kmem_cache_free(_stDevMgr.pstDevInfoCache, pstDevInfo);
    }

    DBG_EXIT_OK();
    return s32Ret;
}

MI_S32 MI_SHADOW_IMPL_UnRegisterDev(MI_SHADOW_HANDLE hShadow)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SHADOW_DevInfo_t *p, *n, *pstDevInfo;

    MI_SHADOW_CHECK_INIT();
    DBG_INFO("hShadow:0x%x\n", hShadow);
    DOWN(&(_stDevMgr.mutex));
    list_for_each_entry_safe(p, n, &(_stDevMgr.listDevInfoHead), list)
    {
        pstDevInfo = list_entry(&(p->list), MI_SHADOW_DevInfo_t, list);
        if (pstDevInfo)
        {
            if (pstDevInfo->hShadow == hShadow)
            {
                ///exit user wait callback thread
                list_del(&(p->list));
                INIT_LIST_HEAD(&(p->list));
                wake_up_interruptible(&(pstDevInfo->stWaitCallback));
                wake_up_interruptible(&(pstDevInfo->stWaitCallbackResult));
                kmem_cache_free(_stDevMgr.pstDevInfoCache, pstDevInfo);
                DBG_INFO("Remove hShadow:0x%x done\n", hShadow);
                s32Ret = MI_SUCCESS;
                break;
            }
        }
    }
    UP(&(_stDevMgr.mutex));
    return s32Ret;
}


MI_S32 MI_SHADOW_IMPL_SetInputPortBufExtConf(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    return mi_sys_SetInputPortBufExtConf((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId, pstBufExtraConf);
}

MI_S32 MI_SHADOW_IMPL_SetOutputPortBufExtConf(MI_SHADOW_HANDLE hShadow, MI_U32 u32ChnId, MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    return mi_sys_SetOutputPortBufExtConf((MI_SYS_DRV_HANDLE)hShadow, u32ChnId, u32PortId, pstBufExtraConf);
}

MI_S32 MI_SHADOW_IMPL_GetOutputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufConf_t *pstBufConfig,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_BOOL *pbBlockedByRateCtrl,
    MI_SYS_BUF_HANDLE *phBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SHADOW_CHECK_INIT();
    if (!pstBufConfig)
    {
        DBG_ERR("BufConfig Is Null\n");
        return E_MI_ERR_FAILED;
    }


    s32Ret = _MI_SHADOW_IMPL_GetPortBuf(hShadow, u32ChnId, u32PortId, pstBufConfig, pstBufInfo, pbBlockedByRateCtrl, phBufHandle);
    DBG_INFO("s32Ret:%d, hShadow:0x%x, u32ChnId:%d, u32PortId:%d, eBufType:%d, u64TargetPts:%d, FRC:%d, phBufHandle:0x%x\n",
        s32Ret,
        hShadow,
        u32ChnId,
        u32PortId,
        pstBufConfig->eBufType,
        pstBufConfig->u64TargetPts,
        *pbBlockedByRateCtrl,
        *phBufHandle);
    return s32Ret;
}

MI_S32 MI_SHADOW_IMPL_GetInputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_SYS_BUF_HANDLE *phBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SHADOW_CHECK_INIT();

    s32Ret = _MI_SHADOW_IMPL_GetPortBuf(hShadow, u32ChnId, u32PortId, NULL, pstBufInfo, NULL, phBufHandle);
    DBG_INFO("s32Ret:%d hShadow:0x%x, u32ChnId:%d, u32PortId:%d, phBufHandle:%p\n",
        s32Ret,
        hShadow,
        u32ChnId,
        u32PortId,
        phBufHandle);
    DBG_EXIT_OK();
    return s32Ret;
}

MI_S32 MI_SHADOW_IMPL_FinishBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t *pstSysBufInfo = NULL;

    MI_SHADOW_CHECK_INIT();

    pstSysBufInfo = _MI_SHADOW_GetSysBufInfo((MI_SYS_DRV_HANDLE)hShadow, hBufHandle);
    if (pstSysBufInfo)
    {

        s32Ret = mi_sys_FinishBuf(pstSysBufInfo);
        DBG_INFO("Exec mi_sys_FinishBuf s32Ret:%d pstSysBufInfo:%p\n", s32Ret, pstSysBufInfo);
    }
    DBG_EXIT_OK();
    return s32Ret;
}

MI_S32 MI_SHADOW_IMPL_RewindBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t *pstSysBufInfo = NULL;

    MI_SHADOW_CHECK_INIT();

    pstSysBufInfo = _MI_SHADOW_GetSysBufInfo((MI_SYS_DRV_HANDLE)hShadow, hBufHandle);
    if (pstSysBufInfo)
    {
        s32Ret = mi_sys_RewindBuf(pstSysBufInfo);
        DBG_INFO("Exec mi_sys_RewindBuf s32Ret:%d pstSysBufInfo:%p\n", s32Ret, pstSysBufInfo);
    }

    return s32Ret;
}


MI_S32 MI_SHADOW_IMPL_WaitOnInputTaskAvailable(MI_SHADOW_HANDLE hShadow, MI_S32 u32TimeOutMs)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SHADOW_CHECK_INIT();

    DBG_INFO("hShadow:0x%x, u32TimeOutMs:%d\n", hShadow, u32TimeOutMs);
    s32Ret = mi_sys_WaitOnInputTaskAvailable((MI_SYS_DRV_HANDLE)hShadow, u32TimeOutMs);
    DBG_INFO("s32Ret:%d\n", s32Ret);
    return s32Ret;
}

#if (_MTTEST_)
////////////////////////////////////////////////////////////////////////////////////////
///MT TEST START
struct task_struct *ptskTsShadowTask = NULL;
struct task_struct *ptskTsShadowTask2 = NULL;
MI_SHADOW_HANDLE hIVEShadow = 0;
MI_SHADOW_HANDLE hVDFShadow = 0;

typedef struct
{
    mi_sys_ModuleDevInfo_t stMouleInfo;
    void *pUsrData;
    mi_sys_ModuleDevBindOps_t stModuleBindOps;
} MI_SYS_RegisterDev_t;

void _MI_SHADOW_IMPL_MTDevInit(MI_SHADOW_HANDLE hShadow)
{
    MI_SYS_RegisterDev_t *pstIVEShadow = (MI_SYS_RegisterDev_t *)hShadow;
    if (pstIVEShadow->stMouleInfo.eModuleId == E_MI_MODULE_ID_IVE)
    {
        hIVEShadow = hShadow;
        DBG_INFO("Register IVE Now\n");
    }
    else if (pstIVEShadow->stMouleInfo.eModuleId == E_MI_MODULE_ID_VDF)
    {
        hVDFShadow = hShadow;
        DBG_INFO("Register VDF Now\n");
    }
    else
    {
        DBG_ERR("error register dev:%d\n", pstIVEShadow->stMouleInfo.eModuleId);
    }
}

static int _MI_SHADOW_IMPL_MTShadowTask(void *pUsrData)
{
    MI_SYS_RegisterDev_t *pstIVEShadow = (MI_SYS_RegisterDev_t *)hIVEShadow;
    MI_SYS_ChnPort_t stChnCurryPort;
    MI_SYS_ChnPort_t stChnPeerPort;

    DBG_INFO("pstIVEShadow:%p %p %p\n", pstIVEShadow, pstIVEShadow->stModuleBindOps.OnBindInputPort, pstIVEShadow->pUsrData);
    msleep(1000);;
    stChnCurryPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VENC;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstIVEShadow->stModuleBindOps.OnBindInputPort(&stChnCurryPort, &stChnPeerPort, pstIVEShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_DISP;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstIVEShadow->stModuleBindOps.OnUnBindInputPort(&stChnCurryPort, &stChnPeerPort, pstIVEShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_AI;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_AO;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstIVEShadow->stModuleBindOps.OnBindOutputPort(&stChnCurryPort, &stChnPeerPort, pstIVEShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_RGN;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstIVEShadow->stModuleBindOps.OnUnBindOutputPort(&stChnCurryPort, &stChnPeerPort, pstIVEShadow->pUsrData);

    return 0;
}

static int _MI_SHADOW_IMPL_MTShadowTask2(void *pUsrData)
{
    MI_SYS_RegisterDev_t *pstVDFShadow = (MI_SYS_RegisterDev_t *)hVDFShadow;
    MI_SYS_ChnPort_t stChnCurryPort;
    MI_SYS_ChnPort_t stChnPeerPort;

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VENC;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstVDFShadow->stModuleBindOps.OnBindOutputPort(&stChnCurryPort, &stChnPeerPort, pstVDFShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_DISP;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstVDFShadow->stModuleBindOps.OnUnBindOutputPort(&stChnCurryPort, &stChnPeerPort, pstVDFShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_AI;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_AO;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstVDFShadow->stModuleBindOps.OnBindInputPort(&stChnCurryPort, &stChnPeerPort, pstVDFShadow->pUsrData);

    msleep(1000);
    stChnCurryPort.eModId = E_MI_MODULE_ID_RGN;
    stChnCurryPort.s32DevId = 1;
    stChnCurryPort.s32ChnId = 2;
    stChnCurryPort.s32PortId = 3;

    stChnPeerPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPeerPort.s32DevId = 1;
    stChnPeerPort.s32ChnId = 2;
    stChnPeerPort.s32PortId = 3;
    pstVDFShadow->stModuleBindOps.OnUnBindInputPort(&stChnCurryPort, &stChnPeerPort, pstVDFShadow->pUsrData);

    return 0;
}

void MTInit(void)
{
    ptskTsShadowTask = kthread_create(_MI_SHADOW_IMPL_MTShadowTask, NULL, "shadow_mt");
    wake_up_process(ptskTsShadowTask);

    ptskTsShadowTask2 = kthread_create(_MI_SHADOW_IMPL_MTShadowTask2, NULL, "shadow_mt2");
    wake_up_process(ptskTsShadowTask2);
}

///MT TEST END
#endif //_MTTEST_
