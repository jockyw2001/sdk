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

#include <string.h>
#include "mi_common.h"
#include "mi_divp_datatype.h"
#include "HalDivpdatatype.h"
#include "HAL_DIVP.h"
#include "mi_divp_datatype_internal.h"


#define DIVP_TASK_DUMMY_REG (0x123CF0)//BK123C_78[15~0]
#define MI_DIVP_CHN_NULL (0xFF)
#define DIVP_CHN_MIN (0x00)
#define DIVP_CHN_MAX (MI_DIVP_CHN_MAX_NUM - 1)
#define DIVP_CHN_INPUT_PORT_ID (0)
#define DIVP_CHN_OUTPUT_PORT_ID (0)
#define DIVP_CROP_ALIGNMENT (2)
#define BIT(n) (1 << (n))
#define MI_DIVP_MDWIN_NUM (16)
#define MI_DIVP_OUTPUT_WIDTH_2K (1920)
#define MI_DIVP_OUTPUT_WIDTH_4K (3840)
#define MI_DIVP_OUTPUT_HEIGHT_2K (2160)
#define MI_DIVP_OUTPUT_HEIGHT_1K (1080)
#define INVALID_ADDRESS (0x0000)


#define MI_DIVP_CHECK_CHN(DivpChn)  \
    if(DivpChn < DIVP_CHN_MIN || DivpChn > DIVP_CHN_MAX)  \
    {   \
        printk("[%s : %d] DivpChn is invalid! DivpChn = %u.\n", __FUNCTION__, __LINE__, DivpChn);   \
        return MI_DIVP_ERR_INVALID_CHNID;   \
    }

#define MI_DIVP_CHECK_POINTER(pPtr)  \
    if(NULL == pPtr)  \
    {   \
        printk("[%s : %d] Invalid parameter! NULL pointer.\n", __FUNCTION__, __LINE__);   \
        return MI_DIVP_ERR_NULL_PTR;   \
    }

MI_SYS_DRV_HANDLE gDivpDevHdl = NULL;
mi_divp_ChnContex_t gstChnCtx[MI_DIVP_CHN_MAX_NUM];//save channel's settings
MI_U32 gu32ChnNum = 0;
DispCaptureCB gpfnDispCapCallback = NULL;

DECLARE_WAIT_QUEUE_HEAD(divp_isr_waitqueue);
LIST_HEAD(divp_todo_common_task_list);//process DRAM data
LIST_HEAD(divp_working_common_task_list);
LIST_HEAD(divp_todo_capture_task_list);//caputre Display timing
LIST_HEAD(divp_working_capture_task_list);
DECLARE_MUTEX(divp_working_common_task_list_sem);
DECLARE_MUTEX(divp_working_capture_task_list_sem);


static MI_BOOL _mi_divp_ChnBusy(DIVP_CHN DivpChn)
{
    MI_BOOL bRet = FALSE;
    list_head pos;
    mi_sys_ChnTaskInfo_t* pstChnTask = NULL;

    down(&divp_working_common_task_list_sem);
    if(!list_empty_careful(divp_working_common_task_list))
    {
        list_for_each(&pos, divp_working_common_task_list)
        {
            pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, cur_list);
            if(pstChnTask->u32ChnId == DivpChn)
            {
                bRet = TRUE;
                break;
            }
        }
    }
    up(&divp_working_common_task_list_sem);

    return bRet;
}

static void _mi_divp_SetChnAttr(DIVP_CHN DivpChn)
{
    cmd_mload_interface *pCmdInf = (cmd_mload_interface*)data;// get_sys_cmdq_service(CMDQ_ID_DIVP);
    HalDivpMirror_t stMirror;
    memset(&stMirror, 0, sizeof(stMirror));

    if(gstChnCtx[DivpChn].stChnAttr.eDiType != gstChnCtx[DivpChn].stChnAttrPre.eDiType)
    {
        if(HalDivpSetAttr(gstChnCtx[DivpChn].pHalCtx, HAL_DIVP_ATTR_DI,
                                                                &gstChnCtx[DivpChn].stChnAttr.eDiType, pCmdInf))
        {
            gstChnCtx[DivpChn].stChnAttr.eDiType = gstChnCtx[DivpChn].stChnAttr.eDiType;
        }
        else
        {
            printk("\n [%s : %d] setAttr fail!attr type = HAL_DIVP_ATTR_DI, attr value = %u", __FUNCTION__, __LINE__,
                gstChnCtx[DivpChn].stChnAttr.eDiType);
        }
    }

    if(gstChnCtx[DivpChn].stChnAttr.eTnrLevel != gstChnCtx[DivpChn].stChnAttrPre.eTnrLevel)
    {
        if(HalDivpSetAttr(gstChnCtx[DivpChn].pHalCtx, HAL_DIVP_ATTR_TNR,
                                                               &gstChnCtx[DivpChn].stChnAttr.eTnrLevel, pCmdInf))
        {
            gstChnCtx[DivpChn].stChnAttrPre.eTnrLevel = gstChnCtx[DivpChn].stChnAttr.eTnrLevel;
        }
        else
        {
            printk("\n [%s : %d] setAttr fail!attr type = HAL_DIVP_ATTR_TNR, attr value = %u", __FUNCTION__, __LINE__,
                gstChnCtx[DivpChn].stChnAttr.eTnrLevel);
        }
    }

    if(gstChnCtx[DivpChn].stChnAttr.eRotateType != gstChnCtx[DivpChn].stChnAttrPre.eRotateType)
    {
        if(HalDivpSetAttr(gstChnCtx[DivpChn].pHalCtx, HAL_DIVP_ATTR_ROTATE,
                                                            &gstChnCtx[DivpChn].stChnAttr.eRotateType, pCmdInf))
        {
            gstChnCtx[DivpChn].stChnAttrPre.eRotateType = gstChnCtx[DivpChn].stChnAttr.eRotateType;
        }
        else
        {
            printk("\n [%s : %d] setAttr fail!attr type = HAL_DIVP_ATTR_ROTATE, attr value = %u", __FUNCTION__,
                __LINE__, gstChnCtx[DivpChn].stChnAttr.eRotateType);
        }
    }

    if((gstChnCtx[DivpChn].stChnAttr.bHorMirror != gstChnCtx[DivpChn].stChnAttrPre.bHorMirror)
        || (gstChnCtx[DivpChn].stChnAttr.bVerMirror != gstChnCtx[DivpChn].stChnAttrPre.bVerMirror))
    {
        stMirror.bHMirror = gstChnCtx[DivpChn].stChnAttr.bHorMirror;
        stMirror.bVMirror = gstChnCtx[DivpChn].stChnAttr.bVerMirror;
        if(HalDivpSetAttr(gstChnCtx[DivpChn].pHalCtx, HAL_DIVP_ATTR_MIRROR, &stMirror, pCmdInf))
        {
            gstChnCtx[DivpChn].stChnAttrPre.bHorMirror = gstChnCtx[DivpChn].stChnAttr.bHorMirror;
            gstChnCtx[DivpChn].stChnAttrPre.bVerMirror = gstChnCtx[DivpChn].stChnAttr.bVerMirror;
        }
        else
        {
            printk("\n [%s : %d] setAttr fail!attr type = HAL_DIVP_ATTR_MIRROR, Mirror(H, V) = (%u, %u)", __FUNCTION__, __LINE__,
                gstChnCtx[DivpChn].stChnAttr.bHorMirror, gstChnCtx[DivpChn].stChnAttr.bVerMirror);
        }
    }

    if((gstChnCtx[DivpChn].stChnAttr.stCropRect.u16X != gstChnCtx[DivpChn].stChnAttrPre.stCropRect.u16X)
        || (gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Y != gstChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Y)
        || (gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Width != gstChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Width)
        || (gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Height != gstChnCtx[DivpChn].stChnAttrPre.stCropRect.u16Height))
    {
        if(HalDivpSetAttr(gstChnCtx[DivpChn].pHalCtx, HAL_DIVP_ATTR_CROP,
                                                       &gstChnCtx[DivpChn].stChnAttr.stCropRect, pCmdInf))
        {
            memcpy(&gstChnCtx[DivpChn].stChnAttrPre.stCropRect, &gstChnCtx[DivpChn].stChnAttr.stCropRect, sizeof(struct MI_SYS_WindowRect_t));
        }
        else
        {
            printk("\n [%s : %d] setAttr fail!attr type = HAL_DIVP_ATTR_CROP, crop(x, y, W, H) = (%u, %u, %u, %u)", __FUNCTION__, __LINE__,
                gstChnCtx[DivpChn].stChnAttr.stCropRect.u16X, gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Y,
                gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Width, gstChnCtx[DivpChn].stChnAttr.stCropRect.u16Height);
        }
    }
}


static HalDivpDisplayId_e _mi_divp_TransDispDeviceIdToDivpHal(mi_disp_dev_e eMiDispId)
{
    HalDivpDisplayId_e eHalDispId = HAL_DIVP_Display_MAX;

    switch(eMiDispId)
    {
        case MI_DISP_DEV_0:
             eHalDispId = HAL_DIVP_Display0;
             break;

        case MI_DISP_DEV_1:
             eHalDispId = HAL_DIVP_Display1;
             break;

        default:
             eHalDispId = HAL_DIVP_Display_MAX;
             break;
    }

    return eHalDispId;
}

static HalDivpCapStage_e _mi_divp_TransMiCaptureStagrToHal(mi_divp_CapStage_e stMiCapStage)
{
    HalDivpCapStage_e eHalCapStage = HAL_DIVP_CAP_STAGE_MAX;

    switch(stMiCapStage)
    {
        case E_MI_DIVP_CAP_STAGE_INPUT:
             eHalCapStage = HAL_DIVP_CAP_STAGE_INPUT;
             break;

        case E_MI_DIVP_CAP_STAGE_OUTPUT:
             eHalCapStage = HAL_DIVP_CAP_STAGE_OUTPUT;
             break;

        case E_MI_DIVP_CAP_STAGE_OUTPUT_WITH_OSD:
             eHalCapStage = HAL_DIVP_CAP_STAGE_OUTPUT_WITH_OSD;
             break;

        default:
             eHalCapStage = HAL_DIVP_CAP_STAGE_MAX;
             break;
    }

    return eHalCapStage;
}

static MI_BOOL _mi_divp_IsFrameConfigChange(DIVP_CHN DivpChn, HalDivpInputInfo_t* pstDivpInputInfo, HalDivpOutPutInfo_t* pstDivpOutputInfo)
{
    MI_BOOL bRet = FALSE;

    if((gstChnCtx[DivpChn].stInputInfoPre.u32Width != pstDivpInputInfo->nWidth)
        || (gstChnCtx[DivpChn].stInputInfoPre.u32Height != pstDivpInputInfo->nHeight)
        || (gstChnCtx[DivpChn].stInputInfoPre.eFormat != pstDivpInputInfo->ePxlFmt)
        || (gstChnCtx[DivpChn].stOutputInfoPre.u32Width != pstDivpOutputInfo->nWidth)
        || (gstChnCtx[DivpChn].stOutputInfoPre.u32Height != pstDivpOutputInfo->nHeight)
        || (gstChnCtx[DivpChn].stOutputInfoPre.eFormat != pstDivpOutputInfo->ePxlFmt))
    {
        bRet = TRUE;
    }

    return bRet;
}

static HalDivpDiType_e  _mi_divp_TransMiDiTypeToHal(MI_DIVP_DiType_e eMiDiType)
{
    HalDivpDiType_e eHalDiType = HAL_DIVP_DI_TYPE_OFF;

    switch(eMiDiType)
    {
        case E_MI_DIVP_DI_TYPE_OFF:
            eHalDiType = HAL_DIVP_DI_TYPE_OFF;
            break;

        case E_MI_DIVP_DI_TYPE_2D:
            eHalDiType = HAL_DIVP_DI_TYPE_2D;
            break;

        case E_MI_DIVP_DI_TYPE_3D:
            eHalDiType = HAL_DIVP_DI_TYPE_3D;
            break;

        case E_MI_DIVP_DI_TYPE_NUM:
            eHalDiType = HAL_DIVP_DI_TYPE_NUM;
            break;

        default:
            printk("\n [%s : %d] invalid di type = %u.  ", __FUNCTION__, __LINE__, eMiDiType);
            eHalDiType = HAL_DIVP_DI_TYPE_OFF;
            break;
    }

    return eHalDiType;
}

static HalDivpRotate_e  _mi_divp_TransSysRotateToDivpHal(MI_SYS_Rotate_e eSysRotateType)
{
    HalDivpRotate_e eHalRotate = HAL_DIVP_ROTATE_NONE;

    switch(eSysRotateType)
    {
        case E_MI_SYS_ROTATE_NONE:
            eHalRotate = HAL_DIVP_ROTATE_NONE;
            break;

        case E_MI_SYS_ROTATE_90:
            eHalRotate = HAL_DIVP_ROTATE_90;
            break;

        case E_MI_SYS_ROTATE_180:
            eHalRotate = HAL_DIVP_ROTATE_180;
            break;

        case E_MI_SYS_ROTATE_270:
            eHalRotate = HAL_DIVP_ROTATE_270;
            break;

        case E_MI_SYS_ROTATE_NUM:
            eHalRotate = HAL_DIVP_ROTATE_NUM;
            break;

        default:
            printk("\n [%s : %d] invalid rotate type = %u.  ", __FUNCTION__, __LINE__, eSysRotateType);
            eHalRotate = HAL_DIVP_ROTATE_NONE;
            break;
    }

    return eHalRotate;
}

static HalDivpTnrLevel_e  _mi_divp_TransMiTnrLevelToHal(MI_DIVP_TnrLevel_e eMiTnrLevel)
{
    HalDivpTnrLevel_e eHalTnrLvl = HAL_DIVP_TNR_LEVEL_OFF;

    switch(eMiTnrLevel)
    {
        case E_MI_DIVP_TNR_LEVEL_OFF:
            eHalTnrLvl = HAL_DIVP_DI_TYPE_OFF;
            break;

        case E_MI_DIVP_TNR_LEVEL_LOW:
            eHalTnrLvl = HAL_DIVP_TNR_LEVEL_LOW;
            break;

        case E_MI_DIVP_TNR_LEVEL_MIDDLE:
            eHalTnrLvl = HAL_DIVP_TNR_LEVEL_MIDDLE;
            break;

        case E_MI_DIVP_TNR_LEVEL_HIGH:
            eHalTnrLvl = HAL_DIVP_TNR_LEVEL_HIGH;
            break;

        case E_MI_DIVP_TNR_LEVEL_HIGH:
            eHalTnrLvl = HAL_DIVP_TNR_LEVEL_NUM;
            break;

        default:
            printk("\n [%s : %d] invalid Tnr level = %u.  ", __FUNCTION__, __LINE__, eMiTnrLevel);
            eHalTnrLvl = HAL_DIVP_TNR_LEVEL_OFF;
            break;
    }

    return eHalTnrLvl;
}

static MI_S32 _mi_divp_TransSysWindowToDivpHal(MI_SYS_WindowRect_t* pstWinRect, HalDivpWindow_t* pstHalWin)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    MI_DIVP_CHECK_POINTER(pstWinRect)
    MI_DIVP_CHECK_POINTER(pstHalWin)

    pstHalWin->nX = pstWinRect->u16X;
    pstHalWin->nY = pstWinRect->u16Y;
    pstHalWin->nWidth = pstWinRect->u16Width;
    pstHalWin->nHeight = pstWinRect->u16Height;

    return s32Ret;
}

static MI_S32 _mi_divp_TransMiChnAttrToHal(MI_DIVP_ChnAttrParams_t* pstMiAttr, HalDivpAttrParams_t* pstHalAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    MI_DIVP_CHECK_POINTER(pstMiAttr)
    MI_DIVP_CHECK_POINTER(pstHalAttr)

    pstHalAttr->eDiType = _mi_divp_TransMiDiTypeToHal(pstMiAttr->eDiType);
    pstHalAttr->eRotateType = _mi_divp_TransSysRotateToDivpHal(pstMiAttr->eRotateType);
    pstHalAttr->eTnrLevel = _mi_divp_TransMiTnrLevelToHal(pstMiAttr->eTnrLevel);
    _mi_divp_TransSysWindowToDivpHal(&pstMiAttr->stCropRect, &pstHalAttr->stCropWin);

    return s32Ret;
}

static HalDivpPixelFormat_e _mi_divp_TransSysPixelFormatToDivpHal(MI_SYS_PixelFormat_e ePixelFormat)
{
    HalDivpPixelFormat_e eHalPxlFmt = HAL_DIVP_PIXEL_FORMAT_MAX;

    switch(ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YUV422_YUYV;
             break;

        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_ARGB8888;
             break;

        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_ABGR8888;
             break;

        case E_MI_SYS_PIXEL_FRAME_RGB565:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_RGB565;
             break;

        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_ARGB1555;
             break;

        case E_MI_SYS_PIXEL_FRAME_I2:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_I2;
             break;

        case E_MI_SYS_PIXEL_FRAME_I4:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_I4;
             break;

        case E_MI_SYS_PIXEL_FRAME_I8:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_I8;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_422;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
             break;

        case E_MI_SYS_PIXEL_FRAME_YUV_MST_420://mstar private format for Vdeec
             ePixelFormat = HAL_DIVP_SCAN_MODE_PROGRESSIVE;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE1_H264;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE2_H265;
             break;

        case E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265:
             ePixelFormat = HAL_DIVP_PIXEL_FORMAT_YC420_MSTTILE3_H265;
             break;

        default:
             ePixelFormat = HAL_DIVP_SCAN_MODE_MAX;
             break;
    }

    return ePixelFormat;
}

static HalDivpTileMode_e _mi_divp_TransSysTileModeToDivpHal(MI_SYS_FrameTileMode_e eTileMode)
{
    HalDivpTileMode_e eHalTileMode = HAL_DIVP_TILE_MODE_MAX;

    switch(eTileMode)
    {
        case E_MI_SYS_FRAME_TILE_MODE_NONE:
             eHalTileMode = HAL_DIVP_TILE_MODE_NONE;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_16x16: // tile mode 16x16
             eHalTileMode = HAL_DIVP_TILE_MODE_16x16;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_16x32: // tile mode 16x32
             eHalTileMode = HAL_DIVP_TILE_MODE_16x32;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_32x16:// tile mode 32x16
             eHalTileMode = HAL_DIVP_TILE_MODE_32x16;
             break;

        case E_MI_SYS_FRAME_TILE_MODE_32x32:// tile mode 32x32
             eHalTileMode = HAL_DIVP_TILE_MODE_32x32;
             break;

        default:
             eHalTileMode = HAL_DIVP_TILE_MODE_MAX;
             break;
    }

    return eHalTileMode;
}

static HalDivpScanMode_e _mi_divp_TransSysScanModeToDivpHal(MI_SYS_FrameScanMode_e eScanMode)
{
    HalDivpScanMode_e eHalScanmode = HAL_DIVP_SCAN_MODE_MAX;

    switch(eScanMode)
    {
        case E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE://P mode
             eHalScanmode = HAL_DIVP_SCAN_MODE_PROGRESSIVE;
             break;

        case E_MI_SYS_FRAME_SCAN_MODE_INTERLACE:/// I mode
             eHalScanmode = HAL_DIVP_SCAN_MODE_INTERLACE;
             break;

        default:
             eHalScanmode = HAL_DIVP_SCAN_MODE_MAX;
             break;
    }

    return eHalScanmode;
}


static HalDivpFieldType_e _mi_divp_TransSysFieldTypeToDivpHal(MI_SYS_FieldType_e eFieldType)
{
    HalDivpFieldType_e eHalFieldType = HAL_DIVP_SCAN_MODE_MAX;

    switch(eFieldType)
    {
        case E_MI_SYS_FIELDTYPE_NONE://< no field.
             eHalFieldType = HAL_DIVP_FIELD_TYPE_NONE;
             break;

        case E_MI_SYS_FIELDTYPE_TOP://< Top field only.
             eHalFieldType = HAL_DIVP_FIELD_TYPE_TOP;
             break;

        case E_MI_SYS_FIELDTYPE_BOTTOM://< Bottom field only.
             eHalFieldType = HAL_DIVP_FIELD_TYPE_BOTTOM;
             break;

        case E_MI_SYS_FIELDTYPE_BOTH://< Both fields.
             eHalFieldType = HAL_DIVP_FIELD_TYPE_BOTH;
             break;

        default:
             eHalFieldType = HAL_DIVP_FIELD_TYPE_NUM;
             break;
    }

    return eHalFieldType;
}


static void _mi_divp_TransVdecMfdecInfoToDivpHal(const mi_vdec_DbInfo_t & stDbInfo, HalDivpMFdecInfo_t* pstHalMfdecInfo)
{
    pstHalMfdecInfo->bDbEnable = stDbInfo.bDbEnable;           // Decoder Buffer Enable
    pstHalMfdecInfo->nDbSelect = stDbInfo.u8DbSelect;          // Decoder Buffer select
    pstHalMfdecInfo->stMirror.bHMirror = stDbInfo.bHMirror;
    pstHalMfdecInfo->stMirror.bVMirror = stDbInfo.bVMirror;
    pstHalMfdecInfo->bUncompressMode = stDbInfo.bUncompressMode;
    pstHalMfdecInfo->bBypassCodecMode = stDbInfo.bBypassCodecMode;
    pstHalMfdecInfo->nStartX = stDbInfo.u16StartX;
    pstHalMfdecInfo->nStartY = stDbInfo.u16StartY;
    pstHalMfdecInfo->nWidth = stDbInfo.u16Width;
    pstHalMfdecInfo->nHeight = stDbInfo.u16Height;
    pstHalMfdecInfo->phyDbBase = stDbInfo.phyDbBase;          // Decoder Buffer base addr
    pstHalMfdecInfo->nDbPitch = stDbInfo.u16DbPitch;         // Decoder Buffer pitch
    pstHalMfdecInfo->nDbMiuSel = stDbInfo.u8DbMiuSel;         // Decoder Buffer Miu select
    pstHalMfdecInfo->phyLbAddr = stDbInfo.phyLbAddr;          // Lookaside buffer addr
    pstHalMfdecInfo->nLbSize = stDbInfo.u8LbSize;           // Lookaside buffer size
    pstHalMfdecInfo->nLbTableId = stDbInfo.u8LbTableId;        // Lookaside buffer table Id

    // Decoder Buffer mode
    if(E_MI_VDEC_DB_MODE_H264_H265 == stDbInfo.eDbMode)
    {
        pstHalMfdecInfo->eDbMode = HAL_DIVP_DB_MODE_H264_H265;
    }
    else if(E_MI_VDEC_DB_MODE_VP9 == stDbInfo.eDbMode)
    {
        pstHalMfdecInfo->eDbMode = HAL_DIVP_DB_MODE_VP9;
    }
    else
    {
        pstHalMfdecInfo->eDbMode = HAL_DIVP_DB_MODE_MAX;
    }
}

static void _mi_divp_TransMiCaptureInfoToHal(const mi_divp_CaptureInfo_t* pstMiCapInfo, HalDivpCaptureInfo_t* pstHalCapInfo)
{
    pstHalCapInfo->u32BufAddr[0] = pstMiCapInfo->u32BufAddr[0];
    pstHalCapInfo->u32BufAddr[1] = pstMiCapInfo->u32BufAddr[1];
    pstHalCapInfo->u32BufAddr[2] = pstMiCapInfo->u32BufAddr[2];
    pstHalCapInfo->u32BufSize = pstMiCapInfo->u32BufSize;
    pstHalCapInfo->nWidth = pstMiCapInfo->u16Width;
    pstHalCapInfo->nHeight = pstMiCapInfo->u16Height;
    pstHalCapInfo->nStride[0] = pstMiCapInfo->u16Stride[0];
    pstHalCapInfo->nStride[1] = pstMiCapInfo->u16Stride[1];
    pstHalCapInfo->nStride[2] = pstMiCapInfo->u16Stride[2];
    pstHalCapInfo->stMirror.bHMirror = pstMiCapInfo->stMirror.bHMirror;
    pstHalCapInfo->stMirror.bVMirror = pstMiCapInfo->stMirror.bVMirror;

    pstHalCapInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstMiCapInfo->ePxlFmt);
    pstHalCapInfo->eDispId = _mi_divp_TransDispDeviceIdToDivpHal(pstMiCapInfo->eDispId);
    pstHalCapInfo->stCapStage = _mi_divp_TransMiCaptureStagrToHal(pstMiCapInfo->eCapStage);
    pstHalCapInfo->eRota = _mi_divp_TransSysRotateToDivpHal(pstMiCapInfo->eMiRotate);
    _mi_divp_TransSysWindowToDivpHal(&pstMiCapInfo->stCapWin, &pstHalCapInfo->stCapWin);
}

static MI_S32 _mi_divp_GetInputInfo(mi_sys_ChnTaskInfo_t* pstTask, HalDivpInputInfo_t* pstDivpInputInfo)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_DIVP_CHECK_POINTER(pstDivpInputInfo)
    mi_vdec_DispFrame_t stVdecFrameInfo;
    memset(&stVdecFrameInfo, 0, sizeof(stVdecFrameInfo));

    ///1 PTS
    pstDivpInputInfo->u64Pts = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->u64Pts;

    if(E_MI_SYS_BUFDATA_FRAME == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType)///VIF input
    {
        pstDivpInputInfo->u32BufAddr[0] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u64PhyAddr[0];
        pstDivpInputInfo->u32BufAddr[1] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u64PhyAddr[1];
        pstDivpInputInfo->u32BufAddr[2] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u64PhyAddr[2];

        pstDivpInputInfo->u16Width = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Width;
        pstDivpInputInfo->u16Height = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Height;
        pstDivpInputInfo->u16Stride[0] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Stride[0];
        pstDivpInputInfo->u16Stride[1] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Stride[1];
        pstDivpInputInfo->u16Stride[2] = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Stride[2];

        pstDivpInputInfo->u32BufSize = pstDivpInputInfo->u16Height * (pstDivpInputInfo->u16Stride[0] +
                                                                 pstDivpInputInfo->u16Stride[1] + pstDivpInputInfo->u16Stride[2]);
        pstDivpInputInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.ePixelFormat);
        pstDivpInputInfo->eTileMode = _mi_divp_TransSysTileModeToDivpHal(pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.eTileMode);
        pstDivpInputInfo->eScanMode = _mi_divp_TransSysScanModeToDivpHal(pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.eFrameScanMode);
        pstDivpInputInfo->eFieldType = _mi_divp_TransSysFieldTypeToDivpHal(pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.eFieldType);

        pstDivpInputInfo->pstMfdecInfo = NULL;///MFdec info
        if(HAL_DIVP_SCAN_MODE_INTERLACE == pstDivpInputInfo->eScanMode)
        {
            pstDivpInputInfo->pst3dDiInfo  ///3D DI info
        }
        else
        {
            pstDivpInputInfo->pst3dDiInfo = NULL;
        }
    }
    else if(E_MI_SYS_BUFDATA_META == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->eBufType)//Vdec input
    {
        if(E_MI_SYS_MODULE_ID_VDEC == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.eDataFromModule)
        {
            memcpy(&stVdecFrameInfo; pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.pVirAddr; sizeof(stVdecFrameInfo));

            pstDivpInputInfo->u32BufAddr[0] = stVdecFrameInfo.stFrmInfo.u32LumaAddr;
            pstDivpInputInfo->u32BufAddr[1] = stVdecFrameInfo.stFrmInfo.u32ChromaAddr;
            pstDivpInputInfo->u32BufAddr[2] = INVALID_ADDRESS;
            pstDivpInputInfo->u32BufSize = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.u32Size;
            pstDivpInputInfo->u16Width = stVdecFrameInfo.stFrmInfo.u16Width;
            pstDivpInputInfo->u16Height = stVdecFrameInfo.stFrmInfo.u16Height;
            pstDivpInputInfo->u16Stride[0] = stVdecFrameInfo.stFrmInfo.u16Pitch;
            pstDivpInputInfo->u16Stride[1] = stVdecFrameInfo.stFrmInfo.u16Pitch;
            pstDivpInputInfo->u16Stride[2] = stVdecFrameInfo.stFrmInfo.u16Pitch;

            pstDivpInputInfo->ePxlFmt =?? _mi_divp_TransSysPixelFormatToDivpHal(pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID]->Buf_Info.ePixelFormat);
            pstDivpInputInfo->eTileMode = _mi_divp_TransSysTileModeToDivpHal(stVdecFrameInfo.stFrmInfoExt.eFrameTileMode);
            pstDivpInputInfo->eScanMode = _mi_divp_TransSysScanModeToDivpHal(stVdecFrameInfo.stFrmInfoExt.eFrameScanMode);
            pstDivpInputInfo->eFieldType = _mi_divp_TransSysFieldTypeToDivpHal(stVdecFrameInfo.stFrmInfo.eFieldType);

            _mi_divp_TransVdecMfdecInfoToDivpHal(stVdecFrameInfo.stDbInfo, &pstDivpInputInfo->pstMfdecInfo);///MFdec info

            if(HAL_DIVP_SCAN_MODE_INTERLACE == stVdecFrameInfo.stFrmInfoExt.eFrameScanMode)
            {
                pstDivpInputInfo->pst3dDiInfo  ///3D DI info
            }
        }
    }
    else if(E_MI_SYS_BUFDATA_RAW == pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID].eBufType)
    {
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

mi_sys_TaskIteratorCBAction_e mi_divp_TaskIteratorCallBack(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    mi_sys_TaskIteratorCBAction_e eRet = MI_SYS_ITERATOR_SKIP_CONTINUTE;
    mi_divp_IteratorWorkInfo_t *workInfo = (mi_divp_IteratorWorkInfo_t *)pUsrData;

    //set buffer info
    pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].eBufType = E_MI_SYS_BUFDATA_FRAME;
    pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.u16Width =
                                                         gstChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.u16Width;
    pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.u16Height =
                                                         gstChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.u16Height;
    pstTaskInfo->astOutputPortPerfBufConfig[DIVP_CHN_OUTPUT_PORT_ID].stFrameCfg.eFormat=
                                                         gstChnCtx[pstTaskInfo->u32ChnId].stOutputPortAttr.ePixelFormat;

    if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) == MI_SUCCESS)
    {
        //check if leak of output buf
        if((NULL != pstTaskInfo->astOutputPortBufInfo[DIVP_CHN_INPUT_PORT_ID])
            && E_MI_DIVP_CHN_STARTED == gstChnCtx[pstTaskInfo->u32ChnId].eStatus)
        {
            down(&divp_working_common_task_list_sem);
            list_add_tail(&pstChnTask->cur_list, &divp_todo_common_task_list);
            up(&divp_working_common_task_list_sem);

            //we at most process 32 batches at one time
            if(++workInfo->totalAddedTask >= 32)
            {
                eRet = MI_SYS_ITERATOR_ACCEPT_STOP;
            }
            else
            {
                eRet = MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
            }
        }
        else
        {
            printk("\n [%s : %d] reject task, channel status = %u.  ", __FUNCTION__, __LINE__, gstChnCtx[pstTaskInfo->u32ChnId].eStatus);
            eRet = MI_SYS_ITERATOR_SKIP_CONTINUTE;
        }
    }

    return eRet;
}

static MI_S32 _mi_divp_GetOutputInfo(mi_sys_ChnTaskInfo_t* pstTask, HalDivpOutPutInfo_t* pstDivpOutputInfo)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_DIVP_CHECK_POINTER(pstDivpOutputInfo)

    if(E_MI_SYS_BUFDATA_FRAME == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType)///VIF input
    {
        pstDivpOutputInfo->u32BufAddr[0] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->Buf_Info.u64PhyAddr[0];
        pstDivpOutputInfo->u32BufAddr[1] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u64PhyAddr[1];
        pstDivpOutputInfo->u32BufAddr[2] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u64PhyAddr[2];

        pstDivpOutputInfo->u16Width = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u32Width;
        pstDivpOutputInfo->u16Height = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u32Height;
        pstDivpOutputInfo->u16Stride[0] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u32Stride[0];
        pstDivpOutputInfo->u16Stride[1] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u32Stride[1];
        pstDivpOutputInfo->u16Stride[2] = pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType.u32Stride[2];

        pstDivpOutputInfo->u32BufSize = pstDivpOutputInfo->u16Height * (pstDivpOutputInfo->u16Stride[0] +
                                                                           pstDivpOutputInfo->u16Stride[1] + pstDivpOutputInfo->u16Stride[2])
        pstDivpOutputInfo->ePxlFmt = _mi_divp_TransSysPixelFormatToDivpHal(pstTask->dataFrame.ePixelFormat);
    }
    else if((E_MI_SYS_BUFDATA_RAW == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType)
        || (E_MI_SYS_BUFDATA_META == pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType))//Vdec input
    {
        printk("\n [%s : %d] DIVP can't support  output customized or raw data. buftype = %u.  ", __FUNCTION__, __LINE__,
            pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID]->eBufType);
        s32Ret = MI_DIVP_ERR_CHN_NOT_SUPPORT;
    }

    return s32Ret;
}

void mi_divp_IsrProcessThread(void* data)
{
    cmd_mload_interface *pCmdInf = (cmd_mload_interface*)data;
    MI_U16 u16Fence = 0;

    while(1)
    {
        interruptible_sleep_on_timeout(&divp_isr_queue, 10);
        list_head pos,n;

        ///1 callback to display after capture screen done.
        while(!list_empty(&divp_working_capture_task_list));
        {
            mi_divp_CaptureInfo_t* pstCapInfo;
            list_for_each_entry_safe(&pos, &n, divp_todo_common_task_list, capture_list)
            {
                pstCapInfo = container_of(&pos, mi_divp_CaptureInfo_t, capture_list);
                MI_SYS_GetReg(DIVP_TASK_DUMMY_REG, &u16Fence);
                if(is_fence_LE(u16Fence, pstCapInfo->u32Fence))
                {
                    down(&divp_working_capture_task_list_sem);
                    list_del(&pstCapInfo->capture_list);
                    up(&divp_working_capture_task_list_sem);

                    gpfnDispCapCallback();
                }
            }
        }

        ///2 notify MI_SYS finish buffer
        while(!list_empty(&divp_working_common_task_list));
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            int loop_cnt = 0;
            list_for_each_entry_safe(&pos, &n, divp_todo_common_task_list, capture_list)
            {
                pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, cur_list);
                MI_SYS_GetReg(DIVP_TASK_DUMMY_REG, &u16Fence);
                if(is_fence_LE(u16Fence, pstChnTask->u32Reserved0))
                {
                    if(!pCmdInf->is_cmdq_empty_idle(pCmdInf->ctx))
                    {
                        up(&divp_working_common_task_list_sem);
                        break;
                    }

                    down(&divp_working_common_task_list_sem);
                    list_del(&pstChnTask->cur_list);
                    up(&divp_working_common_task_list_sem);

                    mi_sys_FinishTaskBuf(pstChnTask);
                }
            }
        }

        ///3 clean ISR
        HalDivpCleanFrameDoneIsr();
    }
}

MI_S32 mi_divp_ProcessCommonTask(mi_sys_ChnTaskInfo_t* pstTask, cmd_mload_interface* pCmdInf, MS_U16 fence)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_BOOL bRet = FALSE;
    MI_BOOL bNeedDouble = FALSE;//if channel attribute change, pixel format or input/output size change, send the save frame twice to avoid show garbage.
    HalDivpInputInfo_t stDivpInputInfo;
    HalDivpOutPutInfo_t stDivpOutputInfo;
    MI_SYS_RawData_t stRawData;
    MI_SYS_FrameData_t stFrameData;
    MI_SYS_MetaData_t stMetaData;
    memset(&stDivpInputInfo, 0, sizeof(stDivpInputInfo));
    memset(&stDivpOutputInfo, 0, sizeof(stDivpOutputInfo));
    memset(&stRawData, 0, sizeof(stRawData));
    memset(&stFrameData, 0, sizeof(stFrameData));
    memset(&stMetaData, 0, sizeof(struct MI_SYS_MetaData_t));

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstTask)
    MI_DIVP_CHECK_POINTER(pCmdInf)

    ///2 prepare channel output buffer and get input/output buffer infomation
    if(MI_DIVP_OK == mi_sys_PrepareTaskOutputBuf(pstTask))
    {
        s32Ret = _mi_divp_GetInputInfo(pstTask, &stDivpInputInfo);
        if(MI_DIVP_OK == s32Ret)
        {
            s32Ret = _mi_divp_GetOutputInfo(pstTask, &stDivpOutputInfo);
        }
    }

    ///3 deal with dram data task.
    if(MI_DIVP_OK == s32Ret)
    {
        ///3.1 check and set channel attribute parameters
        _mi_divp_SetChnAttr(pstTask->u32ChnId);

        ///3.2 cheack input/output size, pixel format
        if(_mi_divp_IsFrameConfigChange(pstTask->u32ChnId, &stDivpInputInfo, &stDivpOutputInfo))
        {
            gstChnCtx[pstTask->u32ChnId].stInputInfoPre.u32Width = stDivpInputInfo.nWidth;
            gstChnCtx[pstTask->u32ChnId].stInputInfoPre.u32Height = stDivpInputInfo.nHeight;
            gstChnCtx[pstTask->u32ChnId].stInputInfoPre.eFormat = stDivpInputInfo.ePxlFmt;

            gstChnCtx[pstTask->u32ChnId].stOutputInfoPre.u32Width = stDivpOutputInfo.nWidth;
            gstChnCtx[pstTask->u32ChnId].stOutputInfoPre.u32Height = stDivpOutputInfo.nHeight;
            gstChnCtx[pstTask->u32ChnId].stOutputInfoPre.eFormat = stDivpOutputInfo.ePxlFmt;

            bNeedDouble = TRUE;
        }

        ///3.3 if output is connected to display, output frame is send to MDWIN buffer, or else send to output buffer.
        HalDivpProcessDramData(gstChnCtx[pstTask->u32ChnId].pHalCtx, &stDivpInputInfo, &stDivpOutputInfo, pCmdInf);
        if(bNeedDouble)
        {
            HalDivpProcessDramData(gstChnCtx[pstTask->u32ChnId].pHalCtx, &stDivpInputInfo, &stDivpOutputInfo, pCmdInf);
            bNeedDouble = FALSE;
        }

        ///3.4 transfer PTS
        pstTask->astOutputPortBufInfo[DIVP_CHN_OUTPUT_PORT_ID].u64Pts = pstTask->astInputPortBufInfo[DIVP_CHN_INPUT_PORT_ID].u64Pts;

        ///3.5 deal with cover and OSD.
        //call region HAL layer channel ID and module ID ,output size,
    }

    return s32Ret;
}

void mi_divp_WorkThread(void *data)
{
    MI_U16 u16Fence = 0;
    cmd_mload_interface *pCmdInf = (cmd_mload_interface*)data;// get_sys_cmdq_service(CMDQ_ID_DIVP);

    ///have cmd queue
    while(1)
    {
        list_head pos,n;
        MI_S32 s32Ret = MI_DIVP_ERR_FAILED;
        if(u16Fence > 0xFFFF)
        {
            u16Fence = 0;
        }

        ///1 deal with capture screen task
        if(list_empty(&divp_todo_capture_task_list))
        {
            mi_divp_CaptureInfo_t* pstCapInfo;
            HalDivpCaptureInfo_t stHalCapInfo;
            MI_U32 u32LoopCnt = 0;
            list_for_each_entry_safe(&pos, &n, divp_todo_capture_task_list, capture_list)
            {
                pstCapInfo = container_of(&pos, mi_divp_CaptureInfo_t, capture_list);
                while(!pCmdInf->ensure_cmdbuf_available(pCmdInf, 0x400, 0x1000))
                {
                    interruptible_sleep_on_timeout(&divp_isr_queue, 2);
                    u32LoopCnt++;
                    if(u32LoopCnt>1000)
                    {
                        BUG();//engine hang
                        break;
                    }
                }

                //process capture screen task
                _mi_divp_TransMiCaptureInfoToHal(pstCapInfo, &stHalCapInfo);
                s32Ret = HalDivpCaptureTiming(&stHalCapInfo, pCmdInf);

                if(MI_DIVP_OK == s32Ret)
                {
                    pCmdInf->cmdq_add_wait_event_cmd(pCmdInf, MI_U16 event);
                    pCmdInf->write_reg_cmdq_mask(pCmdInf,DIVP_TASK_DUMMY_REG, u16Fence, 0xFFFF);
                    pCmdInf->kick_off_cmdq(pCmdInf);

                    //record fence and remove task in todo list
                    pstCapInfo->u32Fence = u16Fence ++;
                    list_del(pstCapInfo->capture_list);

                    //add task info to working list
                    down(&divp_working_capture_task_list_sem);
                    list_add_tail(&pstCapInfo->capture_list, &divp_working_capture_task_list);
                    up(&divp_working_capture_task_list_sem);
                }
                else
                {
                    printk("\n [%s : %d] capture screen fail!", __FUNCTION__, __LINE__);
                }
            }
        }

        ///2 deal with common task
        mi_divp_IteratorWorkInfo_t workinfo;
        workinfo.totalAddedTask = 0;
        mi_sys_DevTaskIterator(E_MI_SYS_MODULE_ID_DIVP, E_MI_DIVP_Device0, mi_divp_TaskIteratorCallBack, &workinfo);
        if(list_empty(&divp_todo_common_task_list))
        {
            schedule();
            mi_sys_WaitOnInputTaskAvailable(E_MI_SYS_MODULE_ID_DIVP, E_MI_DIVP_Device0, 100);
            continue;
        }

        list_for_each_entry_safe(&pos, &n, divp_todo_common_task_list, cur_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_PHY menuload_buf_ring_beg_addr;
            MI_PHY menuload_buf_ring_end_addr;
            MI_U32 u32LoopCnt = 0;

            pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, cur_list);

            while(!pCmdInf->ensure_cmdbuf_available(pCmdInf, 0x400, 0x1000))
            {
                interruptible_sleep_on_timeout(&divp_isr_queue, 2);
                u32LoopCnt++;
                if(u32LoopCnt>1000)
                {
                    BUG();//engine hang
                    break;
                }
            }

            //menuload_buf_ring_end_addr = cmdinf->get_menuload_ringbuf_write_ptr(cmdinf);
            s32Ret = mi_divp_ProcessCommonTask(pstChnTask, pCmdInf);

            if(MI_DIVP_OK == s32Ret)
            {
                pCmdInf->cmdq_add_wait_event_cmd(pCmdInf, MI_U16 event);
                pCmdInf->write_reg_cmdq_mask(pCmdInf,DIVP_TASK_DUMMY_REG, u16Fence, 0xFFFF);
                pCmdInf->kick_off_cmdq(pCmdInf);

                pstChnTask->u32Reserved0 = u16Fence++;
                list_del(pstChnTask->cur_list);

                down(&divp_working_common_task_list_sem);
                list_add_tail(&pstChnTask->cur_list, &divp_working_common_task_list);
                up(&divp_working_common_task_list_sem);
            }
            else
            {
                mi_sys_RewindTaskBuf(pstChnTask);
                //CYS how to discard command in CMDQ buffer?
            }
        }
    }
}

void mi_divp_Isr(void *data)
{
    wake_up(&divp_isr_queue);
}

MI_S32 mi_divp_DeInit(mi_divp_DeviceId_e eDevId)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_U32 u32Index = 0;
    MI_U32 u32ChnNum = 0;

    for(u32Index = 0; u32Index < MI_DIVP_CHN_MAX_NUM; u32Index ++)
    {
        if(MI_DIVP_CHN_NULL != gstChnCtx[u32Index].u32ChnId)
        {
            s32Ret = MI_DIVP_ERR_FAILED;
            u32ChnNum ++;
        }
    }

    if(MI_DIVP_OK == s32Ret)
    {
        gpfnDispCapCallback = NULL;
        mi_sys_UnRegisterDev(gDivpDevHdl);
        HalDivpDeInit(HAL_DIVP_Device0);
        HalDivpEnableFrameDoneIsr(FALSE);
        gDivpDevHdl = NULL;
    }
    else
    {
        printk("\n [%s : %d] there are % channels without distroyed.", __FUNCTION__, __LINE__, u32ChnNum);
    }

    return s32Ret;
}

MI_S32 mi_divp_Init(mi_divp_DeviceId_e eDevId)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_U32 u32Index = 0;
     cmd_mload_interface *pCmdInf = get_sys_cmdq_service(CMDQ_ID_DIVP, TRUE);

    ///1 init DIVP HW and globle variable
    HalDivpInit();
    gpfnDispCapCallback = NULL;
    memset(gstChnCtx, 0, sizeof(gstChnCtx));
    for(u32Index = 0; u32Index < MI_DIVP_CHN_MAX_NUM; u32Index ++)
    {
        gstChnCtx[u32Index].u32ChnId = MI_DIVP_CHN_NULL;
    }

    ///2 register DIVP module to system.
    mi_sys_ModuleDevBindOps_s stDivpOps;
    stDivpOps.OnBindInputPort = mi_divp_OnBindInputPort;
    stDivpOps.OnUnBindInputPort = mi_divp_OnUnBindInputPort;
    stDivpOps.OnBindOutputPort = mi_divp_OnBindOutputPort;
    stDivpOps.OnUnBindOutputPort = mi_divp_OnUnBindOutputPort;

    mi_sys_ModuleDevInfo_t stModInfo;
    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.u32ModuleId = E_MI_SYS_MODULE_ID_DIVP;
    stModInfo.u32DevId = E_MI_DIVP_Device0;
    stModInfo.u32DevChnNum = MI_DIVP_CHN_MAX_NUM;
    stModInfo.u32InputPortNum = MI_DIVP_CHN_INPUT_PORT_NUM;
    stModInfo.u32OutputPortNum = MI_DIVP_CHN_OUTPUT_PORT_NUM;

    gDivpDevHdl = mi_sys_RegisterDev(&stModInfo, &stDivpOps);

    ///3 create thread to deal with tasks of all channels.
    kthread_create(mi_divp_WorkThread, pCmdInf);

    ///4 create thread to recieve ISR and release unused buffer
    kthread_create(mi_divp_IsrProcessThread, pCmdInf);
    request_irq(DIVP_DEV0_ISR_IDX, mi_divp_Isr, NULL);
    HalDivpEnableFrameDoneIsr(TRUE);
}

MI_S32 mi_divp_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    ///2 save output port infomation in channel context.
    if(E_MI_SYS_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        if((E_MI_DIVP_Device0 == pstChnCurryPort->s32DevId)
            && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->s32PortId))
        {
            memcpy(&gstChnCtx[pstChnCurryPort->s32ChnId].stOutputPort, pstChnPeerPort, sizeof(struct MI_SYS_ChnPort_t));
        }
        else
        {
            printk("\n [%s : %d] Invalid device or port ID. s32DevId = %d, s32PortId = %d", __FUNCTION__,
                __LINE__, pstChnCurryPort->s32DevId, pstChnCurryPort->s32PortId);
            s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        }
    }
    else
    {
        printk("\n [%s : %d] Invalid module ID. eModId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

MI_S32 mi_divp_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData))
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    if(E_MI_SYS_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        ///2 check channel status
        if(E_MI_DIVP_CHN_STARTED == gstChnCtx[pstChnCurryPort->s32ChnId].eStatus)
        {
            if (_mi_divp_ChnBusy(pstChnCurryPort->s32ChnId))
            {
                printk("\n [%s : %d] Unbind output port fail. s32ChnId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->s32ChnId);
                s32Ret = MI_DIVP_ERR_CHN_BUSY;
            }
        }

        ///3 reset output port infomation and stop status in channel context.
        if((MI_DIVP_OK == s32Ret) && E_MI_SYS_MODULE_ID_DIVP == pstChnCurryPort->eModId)
        {
            if((E_MI_DIVP_Device0 == pstChnCurryPort->s32DevId)
                && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->s32PortId))
            {
                gstChnCtx[pstChnCurryPort->s32ChnId].stOutputPort.eModId = E_MI_SYS_MODULE_ID_MAX;
                gstChnCtx[pstChnCurryPort->s32ChnId].stOutputPort.s32DevId = 0xFF;
                gstChnCtx[pstChnCurryPort->s32ChnId].stOutputPort.s32PortId = 0xFF;
                gstChnCtx[pstChnCurryPort->s32ChnId].stOutputPort.s32ChnId = MI_DIVP_CHN_NULL;

                gstChnCtx[pstChnCurryPort->s32ChnId].eStatus = E_MI_DIVP_CHN_STOPED;
            }
            else
            {
                printk("\n [%s : %d] Invalid device or port ID. s32DevId = %d, s32PortId = %d", __FUNCTION__,
                    __LINE__, pstChnCurryPort->s32DevId, pstChnCurryPort->s32PortId);
                s32Ret = MI_DIVP_ERR_INVALID_PARAM;
            }
        }
    }
    else
    {
        printk("\n [%s : %d] Invalid module ID. eModId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }
    return s32Ret;
}

MI_S32 mi_divp_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData))
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(pstChnCurryPort->s32ChnId)
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    ///2 save input port infomation in channel context.
    if(E_MI_SYS_MODULE_ID_DIVP == pstChnCurryPort->eModId)
    {
        if((E_MI_DIVP_Device0 == pstChnCurryPort->s32DevId)
            && (DIVP_CHN_INPUT_PORT_ID == pstChnCurryPort->s32PortId))
        {
            memcpy(&gstChnCtx[pstChnCurryPort->s32ChnId].stInputPort, pstChnPeerPort, sizeof(struct MI_SYS_ChnPort_t));
        }
        else
        {
            printk("\n [%s : %d] Invalid device or port ID. s32DevId = %d, s32PortId = %d", __FUNCTION__,
                __LINE__, pstChnCurryPort->s32DevId, pstChnCurryPort->s32PortId);
            s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        }
    }
    else
    {
        printk("\n [%s : %d] Invalid module ID. eModId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->eModId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return MI_DIVP_OK;
}

MI_S32 mi_divp_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData))
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(pstChnCurryPort->s32ChnId)
    MI_DIVP_CHECK_POINTER(pstChnCurryPort)
    MI_DIVP_CHECK_POINTER(pstChnPeerPort)

    if((E_MI_SYS_MODULE_ID_DIVP == pstChnCurryPort->eModId)
        && (E_MI_DIVP_Device0 == pstChnCurryPort->s32DevId)
        && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->s32PortId))
    {
        ///2 check channel status
        if(E_MI_DIVP_CHN_STARTED == gstChnCtx[pstChnCurryPort->s32ChnId].eStatus)
        {
            if (_mi_divp_ChnBusy(pstChnCurryPort->s32ChnId))
            {
                printk("\n [%s : %d] Unbind output port fail. s32ChnId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->s32ChnId);
                s32Ret = MI_DIVP_ERR_CHN_BUSY;
            }
        }

        ///3 reset output port infomation and stop status in channel context.
        if(MI_DIVP_OK == s32Ret)
        {
            if((E_MI_DIVP_Device0 == pstChnCurryPort->s32DevId)
                && (DIVP_CHN_OUTPUT_PORT_ID == pstChnCurryPort->s32PortId))
            {
                gstChnCtx[pstChnCurryPort->s32ChnId].stInputPort.eModId = E_MI_SYS_MODULE_ID_MAX;
                gstChnCtx[pstChnCurryPort->s32ChnId].stInputPort.s32DevId = 0xFF;
                gstChnCtx[pstChnCurryPort->s32ChnId].stInputPort.s32PortId = 0xFF;
                gstChnCtx[pstChnCurryPort->s32ChnId].stInputPort.s32ChnId = MI_DIVP_CHN_NULL;

                gstChnCtx[pstChnCurryPort->s32ChnId].eStatus = E_MI_DIVP_CHN_STOPED;
            }
            else
            {
                printk("\n [%s : %d] Invalid device or port ID. s32DevId = %d, s32PortId = %d", __FUNCTION__,
                    __LINE__, pstChnCurryPort->s32DevId, pstChnCurryPort->s32PortId);
                s32Ret = MI_DIVP_ERR_INVALID_PARAM;
            }
        }
    }
    else
    {
        printk("\n [%s : %d] Invalid module ID. eModId = %d, s32DevId = %d, s32PortId = %d.", __FUNCTION__, __LINE__,
            pstChnCurryPort->eModId, pstChnCurryPort->s32DevId, pstChnCurryPort->s32PortId);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }

    return s32Ret;
}

MI_S32 mi_divp_CaptureTiming(mi_divp_CaptureInfo_t* pstCapInfo, DispCaptureCB* pfCallback)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    HalDivpCaptureInfo_t stCaptureInfo;
    memset(&stCaptureInfo, 0, sizeof(stCaptureInfo));

    ///1 check input parameter
    MI_DIVP_CHECK_POINTER(pstCapInfo)
    MI_DIVP_CHECK_POINTER(pfCallback)

    ///2 add task to capture task list.
    down(&divp_working_capture_task_list_sem);
    list_add_tail(&pstCapInfo->capture_list, &divp_todo_capture_task_list);
    up(&divp_working_capture_task_list_sem);

    ///3 save callback
    if(NULL == gpfnDispCapCallback)
    {
        gpfnDispCapCallback = pfCallback;
    }

    return s32Ret;
}



MI_S32 MI_DIVP_SetOutputPortAttr(DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)
    MI_DIVP_CHECK_POINTER(pstOutputPortAttr)

    ///2 check HW limitation of scaling and pixel format
    if((MI_DIVP_OUTPUT_WIDTH_4K < pstOutputPortAttr->u32Width)
        || (MI_DIVP_OUTPUT_HEIGHT_2K < pstOutputPortAttr->u32Height)
        || ((E_MI_SYS_PIXEL_FRAME_YUV422_YUYV != pstOutputPortAttr->ePixelFormat)
            && (E_MI_SYS_PIXEL_FRAME_YUV_MST_420 != pstOutputPortAttr->ePixelFormat))
        || (E_MI_SYS_COMPRESS_MODE_NONE != pstOutputPortAttr->eCompMode))
    {
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }
    else
    {
        ///3 save settings
        memcpy(&gstChnCtx[DivpChn].stOutputPortAttr, pstOutputPortAttr, sizeof(struct MI_DIVP_OutputPortAttr_t));
    }

    return s32Ret;
}

MI_S32 MI_DIVP_GetOutputPortAttr(DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t * pstOutputPortAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)
    MI_DIVP_CHECK_POINTER(pstOutputPortAttr)

    //copy settings
    memcpy(pstOutputPortAttr, &gstChnCtx[DivpChn].pstOutputPortAttr, sizeof(struct MI_DIVP_OutputPortAttr_t));
    return s32Ret;
}

//global APIs======================================================================================================
MI_S32 MI_DIVP_CreateChn (DIVP_CHN *pDivpChn, MI_DIVP_ChnAttrParams_t* pstAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    DIVP_CHN DivpChn = 0;
    MI_U32 u32Index = 0;

    //check input parameter
    MI_DIVP_CHECK_POINTER(pstAttr)
    *pDivpChn = MI_DIVP_CHN_NULL;

    ///check HW resources
    if(gu32ChnNum >= MI_DIVP_CHN_MAX_NUM)
    {
        s32Ret = MI_DIVP_ERR_NO_RESOUCE;
    }
    else
    {
        if(((E_MI_SYS_ROTATE_90 != pstAttr->eRotateType)///DIVP only support rotate 90 and 270 degrees
                || (E_MI_SYS_ROTATE_270 != pstAttr->eRotateType)
                || (E_MI_SYS_ROTATE_NONE != pstAttr->eRotateType))
            || ((pstAttr->u32MaxHeight < pstAttr->stCropRect.u16Height)//crop need smaller than max size
                || (pstAttr->u32MaxWidth < pstAttr->stCropRect.u16Width))
            || ((MI_DIVP_OUTPUT_WIDTH_4K < pstAttr->u32MaxHeight)///Max height 2K.
                || (MI_DIVP_OUTPUT_HEIGHT_2K < pstAttr->u32MaxWidth)))///Max width 4K.
        {
            printk("\n [%s : %d] Can't change channel static attribute. max(W,H) = (%u,%u) \n
                eRotateType = %u, crop window (W,H) = (%u, %u) \n", __FUNCTION__, __LINE__,
                pstAttr->u32MaxWidth, pstAttr->u32MaxHeight, pstAttr->eRotateType,
                pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);
            s32Ret = MI_DIVP_ERR_INVALID_PARAM;
        }
        else
        {
            for(u32Index = 0; u32Index < MI_DIVP_CHN_MAX_NUM; u32Index ++)
            {
                //find valid contex
                if(MI_DIVP_CHN_NULL == gstChnCtx[u32Index].u32ChnId)
                {
                    //create instance in HAL layer.
                    if(HalDivpCreateInstance(&gstChnCtx[u32Index].pHalCtx, E_MI_DIVP_Device0))
                    {
                        //create channel ID and save channel attribute
                        gstChnCtx[u32Index].u32ChnId = u32Index;
                        gstChnCtx[u32Index].eStatus = E_MI_DIVP_CHN_CREATED;

                        memcpy(&gstChnCtx[u32Index].stChnAttr, pstAttr, sizeof(struct MI_DIVP_ChnAttrParams_t));
                        memset(&gstChnCtx[u32Index].stChnAttrPre, 0, sizeof(struct MI_DIVP_ChnAttrParams_t));
                        gu32ChnNum ++;
                    }
                    else
                    {
                        printk("\n [%s : %d] Fail to create instance in HAL layer.", __FUNCTION__, __LINE__);
                        s32Ret = MI_DIVP_ERR_FAILED;
                    }
                    break;
                }
            }

        }
    }

    return s32Ret;
}

MI_S32 MI_DIVP_DestroyChn(DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn);

    ///2 check channel status
    if (_mi_divp_ChnBusy(DivpChn))
    {
        printk("\n [%s : %d] Unbind output port fail. s32ChnId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->s32ChnId);
        s32Ret = MI_DIVP_ERR_CHN_BUSY;
    }
    else if(E_MI_DIVP_CHN_STOPED != gstChnCtx[DivpChn].eStatus)
    {
        printk("\n [%s : %d] Unbind output port fail. s32ChnId = %d.", __FUNCTION__, __LINE__, pstChnCurryPort->s32ChnId);
        s32Ret = MI_DIVP_ERR_CHN_NOT_STOPED;
    }

    ///2 destroy channel instance in HAL layer.
    if((MI_DIVP_OK == s32Ret) && HalDivpDestroyInstance(gstChnCtx[DivpChn].pHalCtx))
    {
        ///3 reset channel context
        memset(&gstChnCtx[DivpChn], 0, sizeof(struct mi_divp_ChnContex_t));
        gstChnCtx[DivpChn].u32ChnId = MI_DIVP_CHN_NULL;
        gstChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_DISTROYED;
        gu32ChnNum --;
    }
    else
    {
        printk("\n [%s : %d] Fail to destroy instance in HAL layer.", __FUNCTION__, __LINE__);
        s32Ret = MI_DIVP_ERR_FAILED;
    }

    return s32Ret;
}

MI_S32 MI_DIVP_SetChnAttr(DIVP_CHN DivpChn, MI_DIVP_ChnAttrParams_t* pstAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;
    MI_U32 u32DiBufSize = 0;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)
    MI_DIVP_CHECK_POINTER(pstAttr)

    ///2 check value of input parameter and save it.
    if(((E_MI_SYS_ROTATE_90 != pstAttr->eRotateType)///DIVP only support rotate 90 and 270 degrees
            || (E_MI_SYS_ROTATE_270 != pstAttr->eRotateType)
            || (E_MI_SYS_ROTATE_NONE != pstAttr->eRotateType))
        || ((gstChnCtx[DivpChn].stChnAttr.u32MaxHeight < pstAttr->stCropRect.u16Height)//crop need smaller than max size
            || (gstChnCtx[DivpChn].stChnAttr.u32MaxWidth < pstAttr->stCropRect.u16Width))
        || ((gstChnCtx[DivpChn].stChnAttr.u32MaxHeight != pstAttr->u32MaxHeight)///Max width and height are static attirbute that can't be changed.
            || (gstChnCtx[DivpChn].stChnAttr.u32MaxWidth != pstAttr->u32MaxWidth)))
    {
        printk("\n [%s : %d] Can't change channel static attribute. src(W,H) = (%u,%u), dst(W,H) = (%u,%u) \n
            eRotateType = %u, crop window (W,H) = (%u, %u) \n", __FUNCTION__, __LINE__,
            gstChnCtx[DivpChn].stChnAttr.u32MaxWidth, gstChnCtx[DivpChn].stChnAttr.u32MaxHeight,
            pstAttr->u32MaxWidth, pstAttr->u32MaxHeight, pstAttr->eRotateType,
            pstAttr->stCropRect.u16Width, pstAttr->stCropRect.u16Height);
        s32Ret = MI_DIVP_ERR_INVALID_PARAM;
    }
    else
    {
        memcpy(&gstChnCtx[DivpChn].stChnAttr, pstAttr, sizeof(struct MI_DIVP_ChnAttrParams_t));
    }

    return s32Ret;
}

MI_S32 MI_DIVP_GetChnAttr(DIVP_CHN DivpChn, MI_DIVP_ChnAttrParams_t* pstAttr)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)
    MI_DIVP_CHECK_POINTER(pstAttr)

    memcpy(pstAttr, &gstChnCtx[DivpChn].stChnAttr; sizeof(struct MI_DIVP_ChnAttrParams_t));

    return s32Ret;
}

MI_S32 MI_DIVP_StartChn(DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)

    gstChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_STARTED;
    return s32Ret;
}

MI_S32 MI_DIVP_StopChn(DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)

    ///2 check channel's status
    if(_mi_divp_ChnBusy(DivpChn))
    {
        printk("\n [%s : %d] Channel is busy. DivpChn = %u.", __FUNCTION__, __LINE__, DivpChn);
        s32Ret = MI_DIVP_ERR_CHN_BUSY;
    }
    else
    {
        if(gstChnCtx[DivpChn].eStatus < E_MI_DIVP_CHN_STOPED)
        {
            gstChnCtx[DivpChn].eStatus = E_MI_DIVP_CHN_STOPED;
        }
    }

    return s32Ret;
}


MI_S32 MI_DIVP_SetChnOverlay(DIVP_CHN DivpChn, MI_U32 u32OverlayMask)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)

    // the api for gop show/hide a specified OSD
    gstChnCtx[DivpChn].u32OverlayMask = u32OverlayMask;

    return s32Ret;
}

MI_S32 MI_DIVP_GetChnOverlay(DIVP_CHN DivpChn, MI_U32* pu32OverlayMask)
{
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_DIVP_CHECK_CHN(DivpChn)
    MI_DIVP_CHECK_POINTER(pu32OverlayMask)

    pu32OverlayMask = gstChnCtx[DivpChn].u32OverlayMask;

    return s32Ret;
}
