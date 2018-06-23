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
/// @file   sys_ioctl.c
/// @brief vdec module ioctl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_device.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "mi_sys_impl.h"
#include "mi_sys_mma_heap_impl.h"

#include "mi_print.h"
#include "sys_ioctl.h"

static DEFINE_MUTEX(mutex);

static MI_S32 MI_SYS_IOCTL_Init(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_Init();
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_Exit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_Exit();
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_BindChnPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_ChnPortArg_t *arg = ptr;
    MI_SYS_ChnPort_t *pSrcChn = &(arg->stSrcChnPort);
    MI_SYS_ChnPort_t *pDstChn = &(arg->stDstChnPort);
    MI_U32 u32SrcFrmrate = arg->u32SrcFrmrate;
    MI_U32 u32DstFrmrate = arg->u32DstFrmrate;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_BindChnPort(pSrcChn , pDstChn ,u32SrcFrmrate ,u32DstFrmrate);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_UnBindChnPort(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_ChnPortArg_t *arg = ptr;
    MI_SYS_ChnPort_t *pSrcChn = &(arg->stSrcChnPort);
    MI_SYS_ChnPort_t *pDstChn = &(arg->stDstChnPort);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_UnBindChnPort(pSrcChn , pDstChn);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetBindbyDest(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_ChnPortArg_t *arg = ptr;
    MI_SYS_ChnPort_t *pSrcChn = &(arg->stSrcChnPort);
    MI_SYS_ChnPort_t *pDstChn = &(arg->stDstChnPort);
    MI_SYS_ChnPort_t stChnPort;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_GetBindbyDest(pDstChn , &stChnPort);
    memcpy(pSrcChn, &stChnPort, sizeof(MI_SYS_ChnPort_t));
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetVersion(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Version_t *arg = ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_GetVersion(arg);

    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetCurPts(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U64 *arg = ptr;
    MI_U64 u64Pts;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_GetCurPts(&u64Pts);
    *arg = u64Pts;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_InitPtsBase(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U64 *arg = ptr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_InitPtsBase(*arg);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_SyncPts(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U64 *arg = ptr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_SyncPts(*arg);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_Mmap(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Mmap_t *arg = ptr;
    MI_PHY phyAddr = arg->phyAddr;
    MI_U32 u32Size = arg->u32Size;
    MI_BOOL bCache = arg->bCache;
    void *pVirtualAddress;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_Mmap(phyAddr , u32Size , &pVirtualAddress , bCache);
    arg->pVirtualAddress = pVirtualAddress;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_Munmap(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Munmap_t *arg = ptr;
    void *pVirtualAddress = (arg->pVirtualAddress);
    MI_U32 u32Size = arg->u32Size;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_Munmap(pVirtualAddress , u32Size);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_SetReg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetReg_t *arg = ptr;
    MI_U32 u32RegAddr = arg->u32RegAddr;
    MI_U16 u16Value = arg->u16Value;
    MI_U16 u16Mask = arg->u16Mask;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_SetReg(u32RegAddr ,u16Value ,u16Mask);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetReg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetReg_t *arg = ptr;
    MI_U32 u32RegAddr = arg->u32RegAddr;
    MI_U16 u16Value = 0;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_GetReg(u32RegAddr ,&u16Value);
    arg->u16Value = u16Value;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_SetChnMMAConf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetChnPortMMAConf_t *arg = ptr;
    MI_U8 *pu8MMAHeapName = arg->u8MMAHeapName;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_SetChnMMAConf(arg->eModId ,arg->u32DevId, arg->u32ChnId ,pu8MMAHeapName);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetChnMMAConf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetChnPortMMAConf_t *arg = ptr;
    MI_U32 u32Length = arg->u32Length;
    MI_U8 u8Data[MAX_MMA_HEAP_NAME_LENGTH] = {0};
    AUTO_LOCK(env, mutex);
    u32Length = min((MI_U32)MAX_MMA_HEAP_NAME_LENGTH, u32Length);
    s32Ret = MI_SYS_IMPL_GetChnMMAConf(arg->eModId ,arg->u32DevId, arg->u32ChnId, u8Data, u32Length);
    memcpy(arg->u8Data , u8Data , u32Length);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ChnInputPortGetBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnInputPortGetBuf_t *arg = ptr;

    MI_SYS_ChnPort_t *pstChnPort = &(arg->stChnPort);
    MI_SYS_BufConf_t *pstBufConf = &(arg->stBufConf);
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_ChnInputPortGetBuf(pstChnPort, pstBufConf, &stBufInfo, &BufHandle, arg->s32TimeOutMs, arg->u32ExtraFlags);
    memcpy(&(arg->stBufInfo) , &stBufInfo , sizeof(MI_SYS_BufInfo_t));
    arg->BufHandle = BufHandle;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ChnInputPortPutBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnInputPortPutBuf_t *arg = ptr;

    MI_SYS_BUF_HANDLE BufHandle = arg->BufHandle;
    MI_SYS_BufInfo_t *pstBufInfo = &(arg->stBufInfo);
    MI_BOOL bDropBuf = arg->bDropBuf;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_ChnInputPortPutBuf(BufHandle , pstBufInfo , bDropBuf);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ChnOutputPortGetBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnOutputPortGetBuf_t *arg = ptr;
    MI_SYS_ChnPort_t *pstChnPort = &(arg->stChnPort);
    MI_SYS_BufInfo_t stBufInfo = arg->stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_ChnOutputPortGetBuf(pstChnPort , &stBufInfo , &BufHandle, arg->u32ExtraFlags);
    memcpy(&(arg->stBufInfo) , &stBufInfo , sizeof(MI_SYS_BufInfo_t));
    arg->BufHandle = BufHandle;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ChnOutputPortPutBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BUF_HANDLE *arg = ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_ChnOutputPortPutBuf(*arg);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_SetChnOutputPortDepth(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetChnOutputPortDepth_t *arg = ptr;

    MI_SYS_ChnPort_t *pstChnPort = &(arg->stChnPort);

    MI_U32 u32UserFrameDepth = arg->u32UserFrameDepth;
    MI_U32 u32BufQueueDepth = arg->u32BufQueueDepth;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_SetChnOutputPortDepth(pstChnPort, u32UserFrameDepth, u32BufQueueDepth);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_GetFd(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetFd_t *arg = ptr;
    MI_SYS_ChnPort_t *pstChnPort = &(arg->stChnPort);
    MI_S32 s32Fd ;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_GetFd(pstChnPort , &s32Fd);
    arg->s32Fd = s32Fd;
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ChnPortInjectBuf(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPortInjectBuf_t *arg = ptr;
    MI_SYS_ChnPort_t *pstChnPort = &(arg->stChnPort);
    MI_SYS_BUF_HANDLE hBufHandle = arg->BufHandle;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_InjectBuf(hBufHandle , pstChnPort);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_MmaAlloc(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_Mma_Alloc_t *stMmaAlloc;

    stMmaAlloc = (MI_SYS_Mma_Alloc_t *)ptr;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_SYS_IMPL_MmaAlloc( stMmaAlloc->szMMAHeapName,stMmaAlloc->blkSize,&stMmaAlloc->pu64PhyAddr);


return s32Ret;
}



static MI_S32 MI_SYS_IOCTL_MmaFree(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_Mma_Free_t *stMmaFree;
    stMmaFree = (MI_SYS_Mma_Free_t *)ptr;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_SYS_IMPL_MmaFree(stMmaFree->phyAddr);
    return s32Ret;
}


static MI_S32 MI_SYS_IOCTL_FlushInvCache(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_FlushInvCache_t *pstFlushInvCache;
    pstFlushInvCache = (MI_SYS_FlushInvCache_t *)ptr;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_SYS_IMPL_FlushInvCache(pstFlushInvCache->pVirtualAddress,pstFlushInvCache->length);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ConfDevPubPools(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_ConfDevPubPools_t *stConfDevPubPools;
    stConfDevPubPools = (MI_SYS_ConfDevPubPools_t *)ptr;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_SYS_IMPL_ConfDevPubPools(stConfDevPubPools->eModule,stConfDevPubPools->u32DevId,stConfDevPubPools->stPoolListConf);
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_RelDevPubPools(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_RelDevPubPools_t *stRelDevPubPools;
    stRelDevPubPools = (MI_SYS_RelDevPubPools_t *)ptr;

    AUTO_LOCK(env, mutex);

    s32Ret = MI_SYS_IMPL_RelDevPubPools(stRelDevPubPools->eModule,stRelDevPubPools->u32DevId);

    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_ConfGloPubPools(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ConfGloPubPools_t *stConfGloPubPools;
    stConfGloPubPools = (MI_SYS_ConfGloPubPools_t *)ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_ConfGloPubPools(stConfGloPubPools->stPoolListConf);


    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_RelGloPubPools(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_RelGloPubPools();
    return s32Ret;
}

static MI_S32 MI_SYS_IOCTL_Set_Vdec_UseVbPool_Flag(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 *pu32Value = (MI_U32 *)ptr;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_SYS_IMPL_Set_VDEC_VBPool_Flag(*pu32Value);
    return s32Ret;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_SYS_CMD_MAX] = {
    [E_MI_SYS_CMD_INIT] = MI_SYS_IOCTL_Init,
    [E_MI_SYS_CMD_EXIT] = MI_SYS_IOCTL_Exit,
    [E_MI_SYS_CMD_BIND_CHN_PORT] = MI_SYS_IOCTL_BindChnPort,
    [E_MI_SYS_CMD_UNBIND_CHN_PORT] = MI_SYS_IOCTL_UnBindChnPort,
    [E_MI_SYS_CMD_GET_BIND_BY_DEST] = MI_SYS_IOCTL_GetBindbyDest,
    [E_MI_SYS_CMD_GET_VERSION] = MI_SYS_IOCTL_GetVersion,
    [E_MI_SYS_CMD_GET_CUR_PTS] = MI_SYS_IOCTL_GetCurPts,
    [E_MI_SYS_CMD_INIT_PTS_BASE] = MI_SYS_IOCTL_InitPtsBase,
    [E_MI_SYS_CMD_SYNC_PTS] = MI_SYS_IOCTL_SyncPts,
    [E_MI_SYS_CMD_MMAP] = MI_SYS_IOCTL_Mmap,
    [E_MI_SYS_CMD_MUNMAP] = MI_SYS_IOCTL_Munmap,
    [E_MI_SYS_CMD_SET_REG] = MI_SYS_IOCTL_SetReg,
    [E_MI_SYS_CMD_GET_REG] = MI_SYS_IOCTL_GetReg,
    [E_MI_SYS_CMD_SET_CHN_MMA_CONF] = MI_SYS_IOCTL_SetChnMMAConf,
    [E_MI_SYS_CMD_GET_CHN_MMA_CONF] = MI_SYS_IOCTL_GetChnMMAConf,
    [E_MI_SYS_CMD_CHN_INPUT_PORT_GET_BUF] = MI_SYS_IOCTL_ChnInputPortGetBuf,
    [E_MI_SYS_CMD_CHN_INPUT_PORT_PUT_BUF] = MI_SYS_IOCTL_ChnInputPortPutBuf,
    [E_MI_SYS_CMD_CHN_OUTPUT_PORT_GET_BUF] = MI_SYS_IOCTL_ChnOutputPortGetBuf,
    [E_MI_SYS_CMD_CHN_OUTPUT_PORT_PUT_BUF] = MI_SYS_IOCTL_ChnOutputPortPutBuf,
    [E_MI_SYS_CMD_SET_CHN_OUTPUT_PORT_DEPTH] = MI_SYS_IOCTL_SetChnOutputPortDepth,
    [E_MI_SYS_CMD_GET_FD] = MI_SYS_IOCTL_GetFd,
    [E_MI_SYS_CMD_CHN_PORT_INJECT_BUF] = MI_SYS_IOCTL_ChnPortInjectBuf,
    [E_MI_SYS_MMA_ALLOC] = MI_SYS_IOCTL_MmaAlloc,
    [E_MI_SYS_MMA_FREE] = MI_SYS_IOCTL_MmaFree,
    [E_MI_SYS_FLUSH_INV_CACHE] = MI_SYS_IOCTL_FlushInvCache,
    [E_MI_SYS_CONF_DEV_PUB_POOLS] = MI_SYS_IOCTL_ConfDevPubPools,
    [E_MI_SYS_REL_DEV_PUB_POOLS] = MI_SYS_IOCTL_RelDevPubPools,
    [E_MI_SYS_CONF_GLO_PUB_POOLS] = MI_SYS_IOCTL_ConfGloPubPools ,
    [E_MI_SYS_REL_GLO_PUB_POOLS] = MI_SYS_IOCTL_RelGloPubPools,
    [E_MI_SYS_CMD_VDEC_USE_VBPOOL] = MI_SYS_IOCTL_Set_Vdec_UseVbPool_Flag
};

static void mi_sys_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
    MI_SYS_IMPL_Init();
}

static void mi_sys_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
    ///MI_SYS_IMPL_Init();
}

static void mi_sys_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");

MI_DEVICE_DEFINE(mi_sys_insmod,
                ioctl_table, E_MI_SYS_CMD_MAX,
                mi_sys_process_init, mi_sys_process_exit);

