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
/// @file   disp_api.c
/// @brief disp module api
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys.h"
#include "disp_internal_api.h"
#include "mi_disp.h"
#include "disp_ioctl.h"



//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

MI_MODULE_DEFINE(disp)

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------

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

MI_S32 mi_disp_GetScreenFrameSize(MI_U32* pu32Size, MI_U32* pu32Width, MI_U32* pu32Height, MI_U32 *pu32Stride)
{
    MI_S32 s32Ret = MI_DISP_FAIL;
    MI_U32 u32Width = 0;
    MI_U32 u32Height = 0;
    MI_U32 u32Size = 0;
    MI_DISP_GetOutputTiming_t stOutputTiming;
    memset(&stOutputTiming, 0, sizeof(MI_DISP_GetOutputTiming_t));

    s32Ret = MI_SYSCALL(MI_DISP_GET_OUTPUTTIMING, &stOutputTiming);
    if(s32Ret)
    {
        goto EXIT;
    }
    else
    {
        mi_disp_GetOutputResolution(&stOutputTiming, &u32Width, &u32Height);
    }

    mi_disp_GetOutputResolution(&stOutputTiming, &u32Width, &u32Height);
    mi_disp_AdjustScreenFrameSize(&u32Width, &u32Height);
    u32Size = u32Width * u32Height * 2;
    u32Size = ALIGN_UP(u32Size, MIU_BURST_BITS/8);
    // Just output YUV422_yuyv
    *pu32Size = u32Size;
    *pu32Width = u32Width;
    *pu32Height = u32Height;
    *pu32Stride = u32Width * 2;
    s32Ret = MI_DISP_SUCCESS;
EXIT:
    return s32Ret;
}

//
MI_S32 _mi_disp_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetScreenFrame_t stGetScreenFrame;
    MI_PHY phyCapFrame;
    void* pVirtualAddress = NULL;
    MI_U32 u32FrameSize = 0;

    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    memset(&stGetScreenFrame, 0, sizeof(stGetScreenFrame));
    stGetScreenFrame.DispLayer = DispLayer;

    if(mi_disp_GetScreenFrameSize(&u32FrameSize, &(stGetScreenFrame.stVFrame.u32Width), &(stGetScreenFrame.stVFrame.u32Height), &stGetScreenFrame.stVFrame.u32Stride) != MI_SUCCESS)
    {
        s32Ret = MI_DISP_FAIL;
        goto EXIT;
    }
    stGetScreenFrame.stVFrame.u32Size = u32FrameSize;
    if(MI_SYS_MMA_Alloc("mma_heap_name0", u32FrameSize, &phyCapFrame) != MI_SUCCESS)
    {
        s32Ret = MI_DISP_FAIL;
        goto EXIT;
    }

    stGetScreenFrame.stVFrame.aphyAddr = phyCapFrame;
    stGetScreenFrame.DispLayer = DispLayer;
    s32Ret = MI_SYSCALL(MI_DISP_GET_SCREEN_FRAME, &stGetScreenFrame);
    //MI_SYS_Mmap(MI_PHY phyAddr, MI_U32 u32Size, void * pVirtualAddress)
    // stGetScreenFrame.stVFrame.pavirAddr = pVirtualAddress;
    if (s32Ret == MI_DISP_SUCCESS)
    {
        //memcpy(pstVFrame, &(stGetScreenFrame.stVFrame), sizeof(MI_DISP_VideoFrame_t));
        pstVFrame->aphyAddr = stGetScreenFrame.stVFrame.aphyAddr;
        pstVFrame->ePixelFormat = stGetScreenFrame.stVFrame.ePixelFormat;
        pstVFrame->pavirAddr = stGetScreenFrame.stVFrame.pavirAddr;
        pstVFrame->u32Height = stGetScreenFrame.stVFrame.u32Height;
        pstVFrame->u32PrivateData = stGetScreenFrame.stVFrame.u32PrivateData;
        pstVFrame->u32Size = stGetScreenFrame.stVFrame.u32Size;
        pstVFrame->u32Width = stGetScreenFrame.stVFrame.u32Width;
        pstVFrame->u32Stride = stGetScreenFrame.stVFrame.u32Stride;

        if(MI_SYS_Mmap(phyCapFrame, u32FrameSize, &(pVirtualAddress), FALSE) != MI_SUCCESS)
        {
            MI_SYS_MMA_Free(phyCapFrame);
            s32Ret = MI_DISP_FAIL;
            goto EXIT;
        }
        pstVFrame->pavirAddr = pVirtualAddress;
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

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------

MI_S32 MI_DISP_Enable(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_ENABLE, &DispDev);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_Enable);

MI_S32 MI_DISP_Disable(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_DISABLE, &DispDev);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_Disable);

MI_S32 MI_DISP_SetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetPubAttr_t stSetPubAttr;

    memset(&stSetPubAttr, 0, sizeof(stSetPubAttr));
    stSetPubAttr.DispDev = DispDev;
    memcpy(&stSetPubAttr.stPubAttr, pstPubAttr, sizeof(*pstPubAttr));
    s32Ret = MI_SYSCALL(MI_DISP_SET_PUB_ATTR, &stSetPubAttr);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetPubAttr);

MI_S32 MI_DISP_GetPubAttr(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetPubAttr_t stGetPubAttr;

    memset(&stGetPubAttr, 0, sizeof(stGetPubAttr));
    stGetPubAttr.DispDev = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_GET_PUB_ATTR, &stGetPubAttr);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstPubAttr, &stGetPubAttr.stPubAttr, sizeof(*pstPubAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetPubAttr);

MI_S32 MI_DISP_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DeviceAttach_t stDeviceAttach;

    memset(&stDeviceAttach, 0, sizeof(stDeviceAttach));
    stDeviceAttach.DispSrcDev = DispSrcDev;
    stDeviceAttach.DispDstDev = DispDstDev;
    s32Ret = MI_SYSCALL(MI_DISP_DEVICE_ATTACH, &stDeviceAttach);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_DeviceAttach);

MI_S32 MI_DISP_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DeviceDetach_t stDeviceDetach;

    memset(&stDeviceDetach, 0, sizeof(stDeviceDetach));
    stDeviceDetach.DispDstDev = DispDstDev;
    stDeviceDetach.DispSrcDev = DispSrcDev;

    s32Ret = MI_SYSCALL(MI_DISP_DEVICE_DETACH, &stDeviceDetach);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_DeviceDetach);

MI_S32 MI_DISP_EnableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_ENABLE_VIDEO_LAYER, &DispLayer);
    if(s32Ret == MI_DISP_SUCCESS)
    {
        //_mi_disp_VideoLayerInitVirable(DispLayer);
    }
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_EnableVideoLayer);

MI_S32 MI_DISP_DisableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_DISABLE_VIDEO_LAYER, &DispLayer);
    if(s32Ret == MI_DISP_SUCCESS)
    {
       // _mi_disp_VideoLayerDeinitVirable(DispLayer);
    }
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_DisableVideoLayer);

MI_S32 MI_DISP_SetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetVideoLayerAttr_t stSetVideoLayerAttr;

    memset(&stSetVideoLayerAttr, 0, sizeof(stSetVideoLayerAttr));
    stSetVideoLayerAttr.DispLayer = DispLayer;
    memcpy(&stSetVideoLayerAttr.stLayerAttr, pstLayerAttr, sizeof(*pstLayerAttr));
    s32Ret = MI_SYSCALL(MI_DISP_SET_VIDEO_LAYER_ATTR, &stSetVideoLayerAttr);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetVideoLayerAttr);

MI_S32 MI_DISP_GetVideoLayerAttr(MI_DISP_LAYER DispLayer, MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetVideoLayerAttr_t stGetVideoLayerAttr;

    memset(&stGetVideoLayerAttr, 0, sizeof(stGetVideoLayerAttr));
    stGetVideoLayerAttr.DispLayer = DispLayer;
    s32Ret = MI_SYSCALL(MI_DISP_GET_VIDEO_LAYER_ATTR, &stGetVideoLayerAttr);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstLayerAttr, &stGetVideoLayerAttr.stLayerAttr, sizeof(*pstLayerAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetVideoLayerAttr);

MI_S32 MI_DISP_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_BindVideoLayer_t stBindVideoLayer;

    memset(&stBindVideoLayer, 0, sizeof(stBindVideoLayer));
    stBindVideoLayer.DispLayer = DispLayer;
    stBindVideoLayer.DispDev   = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_BIND_VIDEO_LAYER, &stBindVideoLayer);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_BindVideoLayer);

MI_S32 MI_DISP_UnBindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_UnBindVideoLayer_t stUnBindVideoLayer;

    memset(&stUnBindVideoLayer, 0, sizeof(stUnBindVideoLayer));
    stUnBindVideoLayer.DispLayer = DispLayer;
    stUnBindVideoLayer.DispDev   = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_UN_BIND_VIDEO_LAYER, &stUnBindVideoLayer);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_UnBindVideoLayer);

MI_S32 MI_DISP_SetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 u32Toleration)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetPlayToleration_t stSetPlayToleration;

    memset(&stSetPlayToleration, 0, sizeof(stSetPlayToleration));
    stSetPlayToleration.DispLayer     = DispLayer;
    stSetPlayToleration.u32Toleration = u32Toleration;
    s32Ret = MI_SYSCALL(MI_DISP_SET_PLAY_TOLERATION, &stSetPlayToleration);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetPlayToleration);

MI_S32 MI_DISP_GetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 *pu32Toleration)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetPlayToleration_t stGetPlayToleration;

    memset(&stGetPlayToleration, 0, sizeof(stGetPlayToleration));
    stGetPlayToleration.DispLayer = DispLayer;
    s32Ret = MI_SYSCALL(MI_DISP_GET_PLAY_TOLERATION, &stGetPlayToleration);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        *pu32Toleration = stGetPlayToleration.u32Toleration;
    }

    return s32Ret;
}

EXPORT_SYMBOL(MI_DISP_GetPlayToleration);

MI_S32 MI_DISP_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = _mi_disp_GetScreenFrame(DispLayer, pstVFrame);
    return s32Ret;
}

EXPORT_SYMBOL(MI_DISP_GetScreenFrame);

MI_S32 MI_DISP_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = _mi_disp_ReleaseScreenFrame(DispLayer, pstVFrame);
    return s32Ret;
}

EXPORT_SYMBOL(MI_DISP_ReleaseScreenFrame);

MI_S32 MI_DISP_SetVideoLayerAttrBegin(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_SET_VIDEO_LAYER_ATTR_BEGIN, &DispLayer);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetVideoLayerAttrBegin);

MI_S32 MI_DISP_SetVideoLayerAttrEnd(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    s32Ret = MI_SYSCALL(MI_DISP_SET_VIDEO_LAYER_ATTR_END, &DispLayer);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetVideoLayerAttrEnd);

MI_S32 MI_DISP_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortAttr_t stSetInputPortAttr;

    memset(&stSetInputPortAttr, 0, sizeof(stSetInputPortAttr));
    stSetInputPortAttr.DispLayer = DispLayer;
    stSetInputPortAttr.LayerInputPort = LayerInputPort;
    memcpy(&stSetInputPortAttr.stInputPortAttr, pstInputPortAttr, sizeof(*pstInputPortAttr));
    s32Ret = MI_SYSCALL(MI_DISP_SET_INPUT_PORT_ATTR, &stSetInputPortAttr);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetInputPortAttr);

MI_S32 MI_DISP_GetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetInputPortAttr_t stGetInputPortAttr;

    memset(&stGetInputPortAttr, 0, sizeof(stGetInputPortAttr));
    stGetInputPortAttr.DispLayer = DispLayer;
    stGetInputPortAttr.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_GET_INPUT_PORT_ATTR, &stGetInputPortAttr);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstInputPortAttr, &stGetInputPortAttr.stInputPortAttr, sizeof(*pstInputPortAttr));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetInputPortAttr);

MI_S32 MI_DISP_EnableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_EnableInputPort_t stEnableInputPort;

    memset(&stEnableInputPort, 0, sizeof(stEnableInputPort));
    stEnableInputPort.DispLayer = DispLayer;
    stEnableInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_ENABLE_INPUT_PORT, &stEnableInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_EnableInputPort);

MI_S32 MI_DISP_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_DisableInputPort_t stDisableInputPort;

    memset(&stDisableInputPort, 0, sizeof(stDisableInputPort));
    stDisableInputPort.DispLayer = DispLayer;
    stDisableInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_DISABLE_INPUT_PORT, &stDisableInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_DisableInputPort);

MI_S32 MI_DISP_SetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortDispPos_t stSetInputPortDispPos;

    memset(&stSetInputPortDispPos, 0, sizeof(stSetInputPortDispPos));
    stSetInputPortDispPos.DispLayer = DispLayer;
    stSetInputPortDispPos.LayerInputPort = LayerInputPort;
    memcpy(&stSetInputPortDispPos.stDispPos, pstDispPos, sizeof(*pstDispPos));
    s32Ret = MI_SYSCALL(MI_DISP_SET_INPUT_PORT_DISP_POS, &stSetInputPortDispPos);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetInputPortDispPos);

MI_S32 MI_DISP_GetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetInputPortDispPos_t stGetInputPortDispPos;

    memset(&stGetInputPortDispPos, 0, sizeof(stGetInputPortDispPos));
    stGetInputPortDispPos.DispLayer = DispLayer;
    stGetInputPortDispPos.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_GET_INPUT_PORT_DISP_POS, &stGetInputPortDispPos);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstDispPos, &stGetInputPortDispPos.stDispPos, sizeof(*pstDispPos));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetInputPortDispPos);

MI_S32 MI_DISP_PauseInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_PauseInputPort_t stPauseInputPort;

    memset(&stPauseInputPort, 0, sizeof(stPauseInputPort));
    stPauseInputPort.DispLayer = DispLayer;
    stPauseInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_PAUSE_INPUT_PORT, &stPauseInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_PauseInputPort);

MI_S32 MI_DISP_ResumeInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ResumeInputPort_t stResumeInputPort;

    memset(&stResumeInputPort, 0, sizeof(stResumeInputPort));
    stResumeInputPort.DispLayer = DispLayer;
    stResumeInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_RESUME_INPUT_PORT, &stResumeInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_ResumeInputPort);

MI_S32 MI_DISP_StepInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_StepInputPort_t stStepInputPort;

    memset(&stStepInputPort, 0, sizeof(stStepInputPort));
    stStepInputPort.DispLayer = DispLayer;
    stStepInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_STEP_INPUT_PORT, &stStepInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_StepInputPort);

MI_S32 MI_DISP_ShowInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ShowInputPort_t stShowInputPort;

    memset(&stShowInputPort, 0, sizeof(stShowInputPort));
    stShowInputPort.DispLayer = DispLayer;
    stShowInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_SHOW_INPUT_PORT, &stShowInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_ShowInputPort);

MI_S32 MI_DISP_HideInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_HideInputPort_t stHideInputPort;

    memset(&stHideInputPort, 0, sizeof(stHideInputPort));
    stHideInputPort.DispLayer = DispLayer;
    stHideInputPort.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_HIDE_INPUT_PORT, &stHideInputPort);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_HideInputPort);

MI_S32 MI_DISP_SetInputPortSyncMode(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_SyncMode_e eMode)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetInputPortSyncMode_t stSetInputPortSyncMode;

    memset(&stSetInputPortSyncMode, 0, sizeof(stSetInputPortSyncMode));
    stSetInputPortSyncMode.DispLayer = DispLayer;
    stSetInputPortSyncMode.LayerInputPort = LayerInputPort;
    stSetInputPortSyncMode.eMode = eMode;
    s32Ret = MI_SYSCALL(MI_DISP_SET_INPUT_PORT_SYNC_MODE, &stSetInputPortSyncMode);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetInputPortSyncMode);

MI_S32 MI_DISP_QueryInputPortStat(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_QueryChannelStatus_t *pstStatus)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_QueryInputPortStat_t stQueryInputPortStat;

    memset(&stQueryInputPortStat, 0, sizeof(stQueryInputPortStat));
    stQueryInputPortStat.DispLayer = DispLayer;
    stQueryInputPortStat.LayerInputPort = LayerInputPort;
    s32Ret = MI_SYSCALL(MI_DISP_QUERY_INPUT_PORT_STAT, &stQueryInputPortStat);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstStatus, &stQueryInputPortStat.stStatus, sizeof(*pstStatus));
    }

    return s32Ret;
}


EXPORT_SYMBOL(MI_DISP_SetZoomInWindow);

MI_S32 MI_DISP_SetZoomInWindow(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_VidWinRect_t* pstZoomRect)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetZoomInWindow_t stZoomWindowInfo;

    memset(&stZoomWindowInfo, 0, sizeof(MI_DISP_SetZoomInWindow_t));
    stZoomWindowInfo.DispLayer = DispLayer;
    stZoomWindowInfo.LayerInputPort = LayerInputPort;
    stZoomWindowInfo.stCropWin = *pstZoomRect;
    s32Ret = MI_SYSCALL(MI_DISP_SET_ZOOM_WINDOW_STAT, &stZoomWindowInfo);
    return s32Ret;
}


EXPORT_SYMBOL(MI_DISP_QueryInputPortStat);

MI_S32 MI_DISP_GetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetVgaParam_t stGetVgaParam;

    memset(&stGetVgaParam, 0, sizeof(stGetVgaParam));
    stGetVgaParam.DispDev = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_GET_VGA_PARAM, &stGetVgaParam);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstVgaParam, &stGetVgaParam.stVgaParam, sizeof(*pstVgaParam));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetVgaParam);

MI_S32 MI_DISP_SetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetVgaParam_t stSetVgaParam;

    memset(&stSetVgaParam, 0, sizeof(stSetVgaParam));
    stSetVgaParam.DispDev = DispDev;
    memcpy(&stSetVgaParam.stVgaParam, pstVgaParam, sizeof(*pstVgaParam));
    s32Ret = MI_SYSCALL(MI_DISP_SET_VGA_PARAM, &stSetVgaParam);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetVgaParam);

MI_S32 MI_DISP_GetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetHdmiParam_t stGetHdmiParam;

    memset(&stGetHdmiParam, 0, sizeof(stGetHdmiParam));
    stGetHdmiParam.DispDev = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_GET_HDMI_PARAM, &stGetHdmiParam);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstHdmiParam, &stGetHdmiParam.stHdmiParam, sizeof(*pstHdmiParam));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetHdmiParam);

MI_S32 MI_DISP_SetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetHdmiParam_t stSetHdmiParam;

    memset(&stSetHdmiParam, 0, sizeof(stSetHdmiParam));
    stSetHdmiParam.DispDev = DispDev;
    memcpy(&stSetHdmiParam.stHdmiParam, pstHdmiParam, sizeof(*pstHdmiParam));
    s32Ret = MI_SYSCALL(MI_DISP_SET_HDMI_PARAM, &stSetHdmiParam);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetHdmiParam);

MI_S32 MI_DISP_GetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_GetCvbsParam_t stGetCvbsParam;

    memset(&stGetCvbsParam, 0, sizeof(stGetCvbsParam));
    stGetCvbsParam.DispDev = DispDev;
    s32Ret = MI_SYSCALL(MI_DISP_GET_CVBS_PARAM, &stGetCvbsParam);
    if (s32Ret == MI_DISP_SUCCESS)
    {
        memcpy(pstCvbsParam, &stGetCvbsParam.stCvbsParam, sizeof(*pstCvbsParam));
    }

    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_GetCvbsParam);

MI_S32 MI_DISP_SetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_SetCvbsParam_t stSetCvbsParam;

    memset(&stSetCvbsParam, 0, sizeof(stSetCvbsParam));
    stSetCvbsParam.DispDev = DispDev;
    memcpy(&stSetCvbsParam.stCvbsParam, pstCvbsParam, sizeof(*pstCvbsParam));
    s32Ret = MI_SYSCALL(MI_DISP_SET_CVBS_PARAM, &stSetCvbsParam);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_SetCvbsParam);

MI_S32 MI_DISP_ClearInputPortBuffer(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bClrAll)
{
    MI_S32 s32Ret = MI_ERR_DISP_BUSY;
    MI_DISP_ClearInportBuff_t stClearInportbuff;

    memset(&stClearInportbuff, 0, sizeof(stClearInportbuff));
    stClearInportbuff.DispLayer = DispLayer;
    stClearInportbuff.LayerInputPort = LayerInputPort;
    stClearInportbuff.bClrAll = bClrAll;
    s32Ret = MI_SYSCALL(MI_DISP_CLEAR_INPORT_BUFFER, &stClearInportbuff);
    return s32Ret;
}
EXPORT_SYMBOL(MI_DISP_ClearInputPortBuffer);


