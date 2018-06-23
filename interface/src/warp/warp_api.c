
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
/// @file   wrap_api.c
/// @brief wrap module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#include <time.h>
#endif

#include "mi_print.h"
#include "mi_syscall.h"
#include "mi_sys.h"
#include "mi_warp.h"
#include "warp_ioctl.h"

MI_MODULE_DEFINE(wrap);


#define MI_WARP_DEBUG_INFO  1
//#define MI_WARP_STAT_TIME   1

#if defined(MI_WARP_DEBUG_INFO)&&(MI_WARP_DEBUG_INFO==1)
#define MI_WARP_FUNC_ENTRY()        if(1){DBG_INFO("%s Function In\n", __func__);}
#define MI_WARP_FUNC_EXIT()         if(1){DBG_INFO("%s Function Exit\n", __func__);}
#define MI_WARP_FUNC_ENTRY2(ViChn)  if(1){DBG_INFO("%s Function In:chn=%d\n", __func__,ViChn);}
#define MI_WARP_FUNC_EXIT2(ViChn)   if(1){DBG_INFO("%s Function Exit:chn=%d\n", __func__, ViChn);}
#else
#define MI_WARP_FUNC_ENTRY()
#define MI_WARP_FUNC_EXIT()
#define MI_WARP_FUNC_ENTRY2(ViChn)
#define MI_WARP_FUNC_EXIT2(ViChn)
#endif



#define MI_WARP_CHECK_DEVID_VALID(id) do {  \
                        if (id >= MI_WARP_MAX_DEVICE_NUM) \
                        { \
                           return MI_ERR_WARP_ILLEGAL_PARAM; \
                        } \
                    } while(0);

#define MI_WARP_CHECK_CHNID_VALID(chn) do {  \
                        if (chn >= MI_WARP_MAX_CHN_NUM) \
                        { \
                           return MI_ERR_WARP_ILLEGAL_PARAM; \
                        } \
                    } while(0);

static MI_S32 _MI_WARP_CreateChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_CreateChannel_t stCreateChn;
    stCreateChn.devId = devId;
    stCreateChn.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_CREATE_CHANNEL, &stCreateChn);

    return s32Ret;
}

static MI_S32 _MI_WARP_DestroyChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_DestroyChannel_t stDestroyChn;
    stDestroyChn.devId = devId;
    stDestroyChn.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_DESTROY_CHANNEL, &stDestroyChn);
    return s32Ret;
}

static MI_S32 _MI_WARP_EnableChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_EnableChannel_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_WARP_EnableChannel_t));
    stChnStatus.devId = devId;
    stChnStatus.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_ENABLECHANNEL, &stChnStatus);
    return s32Ret;
}

static MI_S32 _MI_WARP_DisableChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_EnableChannel_t stChnStatus;
    memset(&stChnStatus, 0x0, sizeof(MI_WARP_EnableChannel_t));
    stChnStatus.devId = devId;
    stChnStatus.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_DISABLECHANNEL, &stChnStatus);
    return s32Ret;
}

static MI_S32 _MI_WARP_EnableInputPort(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_EnableInputPort_t stEnableInputPort;

    memset(&stEnableInputPort, 0, sizeof(stEnableInputPort));
    stEnableInputPort.devId = devId;
    stEnableInputPort.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_ENABLEINPUTPORT, &stEnableInputPort);

    return s32Ret;
}

static MI_S32 _MI_WARP_DisableInputPort(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_DisableInputPort_t stDisableInputPort;

    memset(&stDisableInputPort, 0, sizeof(stDisableInputPort));
    stDisableInputPort.devId = devId;
    stDisableInputPort.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_DISABLEINPUTPORT, &stDisableInputPort);

    return s32Ret;
}

static MI_S32 _MI_WARP_EnableOutputPort(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_EnableOutputPort_t stEnableOutputPort;

    memset(&stEnableOutputPort, 0, sizeof(stEnableOutputPort));
    stEnableOutputPort.devId = devId;
    stEnableOutputPort.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_ENABLEOUTPUTPORT, &stEnableOutputPort);

    return s32Ret;
}

static MI_S32 _MI_WARP_DisableOutputPort(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_DisableOutputPort_t stDisableOutputPort;

    memset(&stDisableOutputPort, 0, sizeof(stDisableOutputPort));
    stDisableOutputPort.devId = devId;
    stDisableOutputPort.chnId = chnId;
    s32Ret = MI_SYSCALL(MI_WARP_DISABLEOUTPUTPORT, &stDisableOutputPort);

    return s32Ret;
}


MI_S32 MI_WARP_CreateDevice(MI_WARP_DEV devId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    s32Ret = MI_SYSCALL(MI_WARP_CREATE_DEV, &devId);

    return s32Ret;
}

MI_S32 MI_WARP_DestroyDevice(MI_WARP_DEV devId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    s32Ret = MI_SYSCALL(MI_WARP_DESTROY_DEV, &devId);
    return s32Ret;
}

MI_S32 MI_WARP_StartDev(MI_WARP_DEV devId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    s32Ret = MI_SYSCALL(MI_WARP_STARTDEV, &devId);
    return s32Ret;
}

MI_S32 MI_WARP_StopDev(MI_WARP_DEV devId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    s32Ret = MI_SYSCALL(MI_WARP_STOPDEV, &devId);
    return s32Ret;
}

MI_S32 MI_WARP_CreateChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;

    // create channel
    DBG_INFO("%s %d: create Chn\n", __FUNCTION__, __LINE__);
    s32Ret = _MI_WARP_CreateChannel(devId, chnId);
    if (MI_WARP_OK != s32Ret)
    {
        DBG_ERR("Dev %d Chn%d failed\n", devId, chnId);
        return MI_ERR_WARP_FAIL;
    }
    // enable chn & input/output port,  (1 in, 1 out)
    DBG_INFO("%s %d: enable Chn\n", __FUNCTION__, __LINE__);
    s32Ret = _MI_WARP_EnableChannel(devId, chnId);
    if (MI_WARP_OK != s32Ret)
    {
        DBG_ERR("Dev %d Chn%d failed\n", devId, chnId);
        goto err_case3;
    }
    DBG_INFO("%s %d: enable inputport\n", __FUNCTION__, __LINE__);
    s32Ret = _MI_WARP_EnableInputPort(devId, chnId);
    if (MI_WARP_OK != s32Ret)
    {
        DBG_ERR("Dev %d Chn%d failed\n", devId, chnId);
        goto err_case2;
    }
    DBG_INFO("%s %d: enable outputport\n", __FUNCTION__, __LINE__);
    s32Ret = _MI_WARP_EnableOutputPort(devId, chnId);
    if (MI_WARP_OK != s32Ret)
    {
        DBG_ERR("Dev %d Chn%d failed\n", devId, chnId);
        goto err_case1;
    }

    return MI_WARP_OK;

err_case1:
    _MI_WARP_DisableInputPort(devId, chnId);
err_case2:
    _MI_WARP_DisableChannel(devId, chnId);
err_case3:
    _MI_WARP_DestroyChannel(devId, chnId);

    return s32Ret;

}

MI_S32 MI_WARP_DestroyChannel(MI_WARP_DEV devId, MI_WARP_CHN chnId)
{
    MI_WARP_FUNC_ENTRY();
    MI_WARP_CHECK_DEVID_VALID(devId);
    MI_WARP_CHECK_CHNID_VALID(chnId);

    // disable chn & input/output port
    _MI_WARP_DisableOutputPort(devId, chnId);
    _MI_WARP_DisableInputPort(devId, chnId);
    _MI_WARP_DisableChannel(devId, chnId);
    _MI_WARP_DestroyChannel(devId, chnId);

    MI_WARP_FUNC_EXIT();

    return MI_WARP_OK;
}

MI_S32 _MI_WARP_SetTable(MI_WARP_DEV devId, MI_WARP_CHN chnId, MI_WARP_TableType_e eTableType, void *pTableAddr, MI_U32 u32TableSize)
{
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    MI_WARP_SetTable_t stSetTable;

    MI_WARP_FUNC_ENTRY();
    memset(&stSetTable, 0, sizeof(MI_WARP_SetTable_t));
    stSetTable.devId = devId;
    stSetTable.chnId = chnId;
    stSetTable.eTableType = eTableType;
    stSetTable.pTableAddr = pTableAddr;
    stSetTable.u32TableSize = u32TableSize;
    s32Ret = MI_SYSCALL(MI_WARP_SETTABLE, &stSetTable);

    MI_WARP_FUNC_EXIT();
    return s32Ret;

}

MI_S32 MI_WARP_SetChnBin(MI_WARP_DEV devId, MI_WARP_CHN chnId, MI_WARP_TableType_e eTableType, const MI_S8 *pTablePath)
{
#ifndef __KERNEL__
    MI_S32 s32Ret = MI_ERR_WARP_FAIL;
    struct stat statbuff;
    MI_U8 *pBufData = NULL;
    MI_S32 s32TableFd = 0;
    MI_U32 u32TableSize = 0;

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    struct timeval startTime, endTime;
#endif

    if (pTablePath == NULL)
    {
        DBG_ERR("File path null!\n");
        return MI_ERR_WARP_ILLEGAL_PARAM;
    }
    DBG_INFO("Read file %s\n", pTablePath);
    memset(&statbuff, 0, sizeof(struct stat));
    if(stat(pTablePath, &statbuff) < 0)
    {
        DBG_ERR("Bb table file not exit!\n");
        return MI_ERR_WARP_ILLEGAL_PARAM;
    }
    else
    {
        if (statbuff.st_size == 0)
        {
            DBG_ERR("File size is zero!\n");
            return MI_ERR_WARP_ILLEGAL_PARAM;
        }
        u32TableSize = statbuff.st_size;
    }

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    gettimeofday(&startTime, NULL);
#endif

    s32TableFd = open(pTablePath, O_RDONLY);

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    gettimeofday(&endTime, NULL);
    DBG_WRN("open file spend time: %d us, type: %d\n", 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec), eTableType);
#endif

    if (s32TableFd < 0)
    {
        DBG_ERR("Open file[%s] error!\n", s32TableFd);
        return MI_ERR_WARP_ILLEGAL_PARAM;
    }
    pBufData = (MI_U8 *)malloc(u32TableSize);
    if (!pBufData)
    {
        DBG_ERR("Malloc error!\n");
        close(s32TableFd);

        return MI_ERR_WARP_ILLEGAL_PARAM;
    }

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
        gettimeofday(&startTime, NULL);
#endif

    read(s32TableFd, pBufData, u32TableSize);

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    gettimeofday(&endTime, NULL);
    DBG_WRN("read file spend time: %d us, type: %d\n", 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec), eTableType);
#endif

    close(s32TableFd);

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    gettimeofday(&startTime, NULL);
#endif

    s32Ret = _MI_WARP_SetTable(devId, chnId, eTableType, pBufData, u32TableSize);
    if (s32Ret != MI_WARP_OK)
    {
        DBG_ERR("_MI_WARP_SetTable error!\n");
        free(pBufData);
        return s32Ret;
    }

    free(pBufData);

#if defined(MI_WARP_STAT_TIME)&&(MI_WARP_STAT_TIME==1)
    gettimeofday(&endTime, NULL);
    DBG_WRN("_MI_WARP_SetTable spend time: %d us, type: %d\n", 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec), eTableType);
#endif

#endif
    return MI_WARP_OK;
}



EXPORT_SYMBOL(MI_WARP_CreateDevice);
EXPORT_SYMBOL(MI_WARP_DestroyDevice);
EXPORT_SYMBOL(MI_WARP_StartDev);
EXPORT_SYMBOL(MI_WARP_StopDev);

EXPORT_SYMBOL(MI_WARP_CreateChannel);
EXPORT_SYMBOL(MI_WARP_DestroyChannel);

EXPORT_SYMBOL(MI_WARP_SetChnBin);
