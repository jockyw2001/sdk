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
/// @file   disp_internel_api.c
/// @brief disp module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/string.h>
#include "disp_internal_api.h"
#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "mi_disp.h"
#include "disp_ioctl.h"


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

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

#define MIU_BURST_BITS        (256)
#define YUV422_PIXEL_ALIGN    (2)
#define YUV422_BYTE_PER_PIXEL (2)
#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#define MI_DISP_CHECK_INVAILDLAYER(Layer)\
        {\
            if(Layer > MI_DISP_LAYER_MAX)\
            {\
                DBG_ERR("Invild Display Layer!!!\n");\
                goto EXIT;\
            }\
        }


//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
MI_S32 mi_disp_GetOutputResolution(MI_DISP_GetOutputTiming_t* pstOutputtiming, MI_U32* pu32Width, MI_U32* pu32Height)
{
    MI_S32 s32Ret = MI_DISP_FAIL;
    MI_U32 u32Width = 0;
    MI_U32 u32Height = 0;
    switch(pstOutputtiming->eOutputTiming)
    {
        case E_MI_DISP_OUTPUT_PAL:
            u32Width = 720;
            u32Height = 576;
            break;
        case E_MI_DISP_OUTPUT_NTSC:
            u32Width = 720;
            u32Height = 480;
            break;
        case E_MI_DISP_OUTPUT_960H_PAL:
            u32Width = 960;
            u32Height = 576;
            break;
        case E_MI_DISP_OUTPUT_960H_NTSC:
            u32Width = 960;
            u32Height = 480;
            break;
    
        case E_MI_DISP_OUTPUT_480i60:
        case E_MI_DISP_OUTPUT_480P60:
            u32Width = 720;
            u32Height = 480;
            break;
        case E_MI_DISP_OUTPUT_576i50:
        case E_MI_DISP_OUTPUT_576P50:
            u32Width = 720;
            u32Height = 576;
            break;
        case E_MI_DISP_OUTPUT_720P50:
        case E_MI_DISP_OUTPUT_720P60:
            u32Width = 1280;
            u32Height = 720;
            break;
        case E_MI_DISP_OUTPUT_1080P24:
        case E_MI_DISP_OUTPUT_1080P25:
        case E_MI_DISP_OUTPUT_1080P30:
        case E_MI_DISP_OUTPUT_1080I50:
        case E_MI_DISP_OUTPUT_1080I60:
        case E_MI_DISP_OUTPUT_1080P50:
        case E_MI_DISP_OUTPUT_1080P60:
            u32Width = 1920;
            u32Height = 1080;
            break;
        case E_MI_DISP_OUTPUT_640x480_60:
            u32Width = 640;
            u32Height = 480;
            break;
        case E_MI_DISP_OUTPUT_800x600_60:
            u32Width = 800;
            u32Height = 600;
            break;
        case E_MI_DISP_OUTPUT_1024x768_60:
            u32Width = 1024;
            u32Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1280x1024_60:
            u32Width = 1280;
            u32Height = 1024;
            break;
        case E_MI_DISP_OUTPUT_1366x768_60:
            u32Width = 1366;
            u32Height = 768;
            break;
        case E_MI_DISP_OUTPUT_1440x900_60:
            u32Width = 1440;
            u32Height = 900;
            break;
        case E_MI_DISP_OUTPUT_1280x800_60:
            u32Width = 1280;
            u32Height = 800;
            break;
        case E_MI_DISP_OUTPUT_1680x1050_60:
            u32Width = 1680;
            u32Height = 1050;
            break;
        case E_MI_DISP_OUTPUT_1920x2160_30:
            u32Width = 1920;
            u32Height = 2160;
            break;
        case E_MI_DISP_OUTPUT_1600x1200_60:
            u32Width = 1600;
            u32Height = 1200;
            break;
        case E_MI_DISP_OUTPUT_1920x1200_60:
            u32Width = 1920;
            u32Height = 1200;
            break;
        case E_MI_DISP_OUTPUT_2560x1440_30:
            u32Width = 2560;
            u32Height = 1440;
            break;
        case E_MI_DISP_OUTPUT_2560x1600_60:
            u32Width = 2560;
            u32Height = 1600;
            break;
        case E_MI_DISP_OUTPUT_3840x2160_30:
        case E_MI_DISP_OUTPUT_3840x2160_60:
            u32Width = 3840;
            u32Height = 2160;
            break;
        case E_MI_DISP_OUTPUT_USER:
            u32Width = pstOutputtiming->stSyncInfo.u16Hpw;
            u32Height = pstOutputtiming->stSyncInfo.u16Vpw;
            break;
        default:
            u32Width = 0;
            u32Height = 0;
    }
    *pu32Width = u32Width;
    *pu32Height = u32Height;
    s32Ret = MI_DISP_SUCCESS;
    return s32Ret;
}

MI_S32 mi_disp_AdjustScreenFrameSize(MI_U32* pu32Width, MI_U32* pu32Height)
{
    MI_U32 u32TmpWidth = *pu32Width;
    MI_U32 u32TmpHeight = *pu32Height;
    u32TmpWidth = (u32TmpWidth >= 1280)? 1280 : u32TmpWidth;
    u32TmpHeight = (u32TmpHeight >= 720)? 720 : u32TmpHeight;
    *pu32Width = u32TmpWidth;
    *pu32Height = u32TmpHeight;
    return MI_DISP_SUCCESS;
}

MI_S32 mi_disp_GetScreenFrameSize(MI_U32* pu32Size)
{
    MI_S32 s32Ret = MI_DISP_FAIL;
    MI_U32 u32Width = 0;
    MI_U32 u32Height = 0;
    MI_U32 u32Size = 0;
    MI_DISP_GetOutputTiming_t stOutputTiming;
    memset(&stOutputTiming, 0, sizeof(MI_DISP_GetOutputTiming_t));
    s32Ret = MI_SYSCALL(MI_DISP_GET_OUTPUTTIMING, &stOutputTiming);
    if(!s32Ret)
    {
        goto EXIT;
    }
    else
    {
        mi_disp_GetOutputResolution(&stOutputTiming, &u32Width, &u32Height);
    }
    mi_disp_AdjustScreenFrameSize(&u32Width, &u32Height);
    u32Size = u32Width * u32Height * 2;
    u32Size = ALIGN_UP(u32Size, MIU_BURST_BITS/8);
    *pu32Size = u32Size;
    s32Ret = MI_DISP_SUCCESS;
EXIT:
    return s32Ret;
}

// TODO:  π”√const
MI_S32 _mi_disp_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame, MI_U32  u32MilliSec)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetScreenFrame_t stGetScreenFrame;
    MI_PHY phyCapFrame;
    void* pVirtualAddress;
    MI_U32 u32FrameSize = 0;
    long i = 0;
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    memset(&stGetScreenFrame, 0, sizeof(stGetScreenFrame));
    stGetScreenFrame.DispLayer = DispLayer;
    memcpy(&stGetScreenFrame.stVFrame, pstVFrame, sizeof(*pstVFrame));
    stGetScreenFrame.u32MilliSec = u32MilliSec;

    while(i < 10000)
    {
        i++;
        MI_PRINT("[%s %d] Test %u\n", __FUNCTION__, __LINE__, u32FrameSize);
    }
    if(!mi_disp_GetScreenFrameSize(&u32FrameSize))
    {
        s32Ret = MI_DISP_FAIL;
        goto EXIT;
    }
    MI_PRINT("[%s %d] Frame Size = %u\n", __FUNCTION__, __LINE__, u32FrameSize);
    if(!MI_SYS_MMA_Alloc(NULL, u32FrameSize, &phyCapFrame))
    {
        s32Ret = MI_DISP_FAIL;
        goto EXIT;
    }
    stGetScreenFrame.stVFrame.aphyAddr = phyCapFrame;
    s32Ret = MI_SYSCALL(MI_DISP_GET_SCREEN_FRAME, &stGetScreenFrame);
    //MI_SYS_Mmap(MI_PHY phyAddr, MI_U32 u32Size, void * pVirtualAddress)
    // stGetScreenFrame.stVFrame.pavirAddr = pVirtualAddress;
    if (s32Ret == MI_DISP_SUCCESS)
    {
    
        MI_PRINT("[%s %d] phy Addr = 0x%llx \n", __FUNCTION__, __LINE__, stGetScreenFrame.stVFrame.aphyAddr);
        memcpy(pstVFrame, &stGetScreenFrame.stVFrame, sizeof(*pstVFrame));
        if(!MI_SYS_Mmap(phyCapFrame, u32FrameSize, pVirtualAddress, FALSE))
        {
            MI_SYS_MMA_Free(phyCapFrame);
            s32Ret = MI_DISP_FAIL;
            goto EXIT;
        }
        pstVFrame->pavirAddr = pVirtualAddress;
        MI_PRINT("[%s %d] vir Addr = %p \n", __FUNCTION__, __LINE__, pstVFrame->pavirAddr);

    }
    s32Ret = MI_DISP_SUCCESS;
EXIT:
    return s32Ret;
}

MI_S32 _mi_disp_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ReleaseScreenFrame_t stReleaseScreenFrame;
    if(pstVFrame == NULL)
    {
        s32Ret = MI_ERR_DISP_NULL_PTR;
        goto EXIT;
    }
    memset(&stReleaseScreenFrame, 0, sizeof(MI_DISP_ReleaseScreenFrame_t));
    stReleaseScreenFrame.DispLayer = DispLayer;
    memcpy(&stReleaseScreenFrame.stVFrame, pstVFrame, sizeof(*pstVFrame));
    s32Ret = MI_SYSCALL(MI_DISP_RELEASE_SCREEN_FRAME, &stReleaseScreenFrame);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        if(!MI_SYS_Munmap(&(pstVFrame->pavirAddr), pstVFrame->u32Size))
        {
            s32Ret = MI_DISP_FAIL;
            goto EXIT;
        }
        if(!MI_SYS_MMA_Free(pstVFrame->aphyAddr))
        {
            s32Ret = MI_DISP_FAIL;
            goto EXIT;
        }
    }

EXIT:
    return s32Ret;
}

