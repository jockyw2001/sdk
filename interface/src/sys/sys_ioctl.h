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

#ifndef _SYS_IOCTL_H_
#define _SYS_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_sys.h"


#define MAX_MMA_HEAP_NAME_LENGTH 64

typedef enum
{
    E_MI_SYS_CMD_INIT,
    E_MI_SYS_CMD_EXIT,
    E_MI_SYS_CMD_BIND_CHN_PORT,
    E_MI_SYS_CMD_UNBIND_CHN_PORT,
    E_MI_SYS_CMD_GET_BIND_BY_DEST,
    E_MI_SYS_CMD_GET_VERSION,
    E_MI_SYS_CMD_GET_CUR_PTS,
    E_MI_SYS_CMD_INIT_PTS_BASE,
    E_MI_SYS_CMD_SYNC_PTS,
    E_MI_SYS_CMD_MMAP,
    E_MI_SYS_CMD_MUNMAP,
    E_MI_SYS_CMD_SET_REG,
    E_MI_SYS_CMD_GET_REG,
    E_MI_SYS_CMD_SET_CHN_MMA_CONF,
    E_MI_SYS_CMD_GET_CHN_MMA_CONF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_GET_BUF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_PUT_BUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_GET_BUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_PUT_BUF,
    E_MI_SYS_CMD_SET_CHN_OUTPUT_PORT_DEPTH,
    E_MI_SYS_CMD_GET_FD,
    E_MI_SYS_CMD_CHN_PORT_INJECT_BUF,
    E_MI_SYS_MMA_ALLOC,
    E_MI_SYS_MMA_FREE,
    E_MI_SYS_FLUSH_INV_CACHE,
    E_MI_SYS_CONF_DEV_PUB_POOLS,
    E_MI_SYS_REL_DEV_PUB_POOLS,
    E_MI_SYS_CONF_GLO_PUB_POOLS,
    E_MI_SYS_REL_GLO_PUB_POOLS,
    E_MI_SYS_CMD_VDEC_USE_VBPOOL,
    E_MI_SYS_CMD_MAX,
} MI_SYS_Cmd_e;

typedef struct MI_SYS_ChnPortArg_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
} MI_SYS_ChnPortArg_t;

typedef struct MI_SYS_Mmap_s
{
    MI_BOOL bCache;
    MI_U32 u32Size;
    void *pVirtualAddress;
    MI_PHY phyAddr;
} MI_SYS_Mmap_t;

typedef struct MI_SYS_Munmap_s
{
    void *pVirtualAddress;
    MI_U32 u32Size;
} MI_SYS_Munmap_t;

typedef struct MI_SYS_SetReg_s
{
    MI_U32 u32RegAddr;
    MI_U16 u16Value;
    MI_U16 u16Mask;
} MI_SYS_SetReg_t;

typedef struct MI_SYS_GetReg_s
{
    MI_U32 u32RegAddr;
    MI_U16 u16Value;
} MI_SYS_GetReg_t;

typedef struct MI_SYS_SetChnPortMMAConf_s
{
    MI_ModuleId_e eModId;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U8 u8MMAHeapName[MAX_MMA_HEAP_NAME_LENGTH];
} MI_SYS_SetChnPortMMAConf_t;

typedef struct MI_SYS_GetChnPortMMAConf_s
{
    MI_ModuleId_e eModId;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U8 u8Data[MAX_MMA_HEAP_NAME_LENGTH];
    MI_U32 u32Length;
} MI_SYS_GetChnPortMMAConf_t;

typedef struct MI_SYS_SetChnOutputFrcCtrl_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U32 u32FrcCtrlNumerator;
    MI_U32 u32FrcCtrlDenominator;
} MI_SYS_SetChnOutputFrcCtrl_t;

typedef struct MI_SYS_ChnInputPortGetBuf_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    MI_S32 s32TimeOutMs;
    MI_U32 u32ExtraFlags;
} MI_SYS_ChnInputPortGetBuf_t;

typedef struct MI_SYS_ChnInputPortPutBuf_s
{
    MI_SYS_BUF_HANDLE BufHandle;
    MI_SYS_BufInfo_t stBufInfo;
    MI_BOOL bDropBuf;
} MI_SYS_ChnInputPortPutBuf_t;

typedef struct MI_SYS_ChnOutputPortGetBuf_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    MI_U32 u32TimeoutMs;
    MI_U32 u32ExtraFlags;
} MI_SYS_ChnOutputPortGetBuf_t;

typedef struct MI_SYS_GetFd_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_S32 s32Fd;
} MI_SYS_GetFd_t;

typedef struct MI_SYS_SetChnOutputPortDepth_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U32 u32UserFrameDepth;
    MI_U32 u32BufQueueDepth;
} MI_SYS_SetChnOutputPortDepth_t;

typedef struct MI_SYS_ChnPortInjectBuf_s
{
    MI_SYS_BUF_HANDLE BufHandle;
    MI_SYS_ChnPort_t stChnPort;
} MI_SYS_ChnPortInjectBuf_t;

#define  MI_SYS_MMA_HEAP_NAME_MAX_LEN 256
typedef struct MI_SYS_Mma_Alloc_s
{
    char szMMAHeapName[MI_SYS_MMA_HEAP_NAME_MAX_LEN];
    unsigned int blkSize ;
    unsigned long long pu64PhyAddr;
}MI_SYS_Mma_Alloc_t;

typedef struct MI_SYS_Mma_Free_s
{
    MI_PHY phyAddr;
}MI_SYS_Mma_Free_t;

typedef struct MI_SYS_FlushInvCache_s
{
    void* pVirtualAddress;
    unsigned long length;
}MI_SYS_FlushInvCache_t;

typedef struct MI_SYS_ConfDevPubPools_s
{
    MI_ModuleId_e eModule;
    MI_U32 u32DevId;
    MI_VB_PoolListConf_t stPoolListConf;
}MI_SYS_ConfDevPubPools_t;

typedef struct MI_SYS_RelDevPubPools_t
{
    MI_ModuleId_e eModule;
    MI_U32 u32DevId;
}MI_SYS_RelDevPubPools_t;

typedef struct MI_SYS_ConfGloPubPools_s
{
    MI_VB_PoolListConf_t stPoolListConf;
}
MI_SYS_ConfGloPubPools_t;

#define MI_SYS_INIT _IOR('i', E_MI_SYS_CMD_INIT, MI_U32)
#define MI_SYS_EXIT _IOR('i', E_MI_SYS_CMD_EXIT, MI_U32)
#define MI_SYS_BIND_CHN_PORT _IOW('i', E_MI_SYS_CMD_BIND_CHN_PORT, MI_SYS_ChnPortArg_t)
#define MI_SYS_UNBIND_CHN_PORT _IOW('i', E_MI_SYS_CMD_UNBIND_CHN_PORT, MI_SYS_ChnPortArg_t)
#define MI_SYS_GET_BIND_BY_DEST _IOWR('i', E_MI_SYS_CMD_GET_BIND_BY_DEST, MI_SYS_ChnPortArg_t)
#define MI_SYS_GET_VERSION _IOR('i', E_MI_SYS_CMD_GET_VERSION , MI_SYS_Version_t)
#define MI_SYS_GET_CUR_PTS _IOR('i', E_MI_SYS_CMD_GET_CUR_PTS, MI_U64)
#define MI_SYS_INIT_PTS_BASE _IOW('i', E_MI_SYS_CMD_INIT_PTS_BASE, MI_U64)
#define MI_SYS_SYNC_PTS _IOW('i', E_MI_SYS_CMD_SYNC_PTS, MI_U64)
#define MI_SYS_MMAP _IOWR('i', E_MI_SYS_CMD_MMAP, MI_SYS_Mmap_t)
#define MI_SYS_MUNMAP _IOW('i', E_MI_SYS_CMD_MUNMAP, MI_SYS_Munmap_t)
#define MI_SYS_SET_REG _IOW('i', E_MI_SYS_CMD_SET_REG, MI_SYS_SetReg_t)
#define MI_SYS_GET_REG _IOWR('i', E_MI_SYS_CMD_GET_REG, MI_SYS_GetReg_t)
#define MI_SYS_SET_CHN_MMA_CONF _IOW('i', E_MI_SYS_CMD_SET_CHN_MMA_CONF, MI_SYS_SetChnPortMMAConf_t)
#define MI_SYS_GET_CHN_MMA_CONF _IOWR('i', E_MI_SYS_CMD_GET_CHN_MMA_CONF, MI_SYS_GetChnPortMMAConf_t)
#define MI_SYS_CHN_INPUT_PORT_GET_BUF _IOWR('i', E_MI_SYS_CMD_CHN_INPUT_PORT_GET_BUF, MI_SYS_ChnInputPortGetBuf_t)
#define MI_SYS_CHN_INPUT_PORT_PUT_BUF _IOW('i', E_MI_SYS_CMD_CHN_INPUT_PORT_PUT_BUF, MI_SYS_ChnInputPortPutBuf_t)
#define MI_SYS_CHN_OUTPUT_PORT_GET_BUF _IOWR('i', E_MI_SYS_CMD_CHN_OUTPUT_PORT_GET_BUF, MI_SYS_ChnOutputPortGetBuf_t)
#define MI_SYS_CHN_OUTPUT_PORT_PUT_BUF _IOW('i', E_MI_SYS_CMD_CHN_OUTPUT_PORT_PUT_BUF, MI_SYS_BUF_HANDLE)
#define MI_SYS_SET_CHN_OUTPUT_PORT_DEPTH _IOW('i', E_MI_SYS_CMD_SET_CHN_OUTPUT_PORT_DEPTH, MI_SYS_SetChnOutputPortDepth_t)
#define MI_SYS_GET_FD _IOWR('i', E_MI_SYS_CMD_GET_FD, MI_SYS_GetFd_t)
#define MI_SYS_CHN_PORT_INJECT_BUF _IOW('i', E_MI_SYS_CMD_CHN_PORT_INJECT_BUF, MI_SYS_ChnPortInjectBuf_t)
#define MI_SYS_MMA_ALLOC _IOWR('i',E_MI_SYS_MMA_ALLOC,MI_SYS_Mma_Alloc_t)
#define MI_SYS_MMA_FREE _IOW('i',E_MI_SYS_MMA_FREE,MI_SYS_Mma_Free_t)
#define MI_SYS_FLUSH_INV_CACHE _IOW('i',E_MI_SYS_FLUSH_INV_CACHE,MI_SYS_FlushInvCache_t)

#define MI_SYS_CONF_DEV_PUB_POOLS _IOW('i',E_MI_SYS_CONF_DEV_PUB_POOLS,MI_SYS_ConfDevPubPools_t)

#define MI_SYS_REL_DEV_PUB_POOLS _IOW('i',E_MI_SYS_REL_DEV_PUB_POOLS,MI_SYS_RelDevPubPools_t)
#define MI_SYS_CONF_GLO_PUB_POOLS _IOW('i',E_MI_SYS_CONF_GLO_PUB_POOLS,MI_SYS_ConfGloPubPools_t)
#define MI_SYS_REL_GLO_PUB_POOLS _IOW('i',E_MI_SYS_REL_GLO_PUB_POOLS,MI_U32)

#define MI_SYS_VDEC_USE_VBPOOL _IOR('i', E_MI_SYS_CMD_VDEC_USE_VBPOOL, MI_U32)

#endif ///_SYS_IOCTL_H_
