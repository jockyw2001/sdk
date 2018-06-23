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
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "mi_syscfg_datatype.h"
#include "mi_syscfg.h"

#include "mi_print.h"
#include "mi_gfx_impl.h"
#include "mi_gfx_datatype.h"

#include "mi_common.h"
#include "apiGFX.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Macros
//-------------------------------------------------------------------------------------------------

#define GFX_ISVALID_POINT(X)  \
    {   \
        if( X == NULL)  \
        {   \
            DBG_ERR("MI_ERR_INVALID_PARAMETER!\n");  \
            return MI_ERR_GFX_INVALID_PARAM;   \
        }   \
    }   \

//#define ENABLE_GFX_TRACE_PERF (1)
#if defined(ENABLE_GFX_TRACE_PERF) && (ENABLE_GFX_TRACE_PERF == 1)
#define GFX_PERF_TIME(pu64Time) do {\
        struct timespec sttime;\
        memset(&sttime, 0, sizeof(sttime));\
        do_posix_clock_monotonic_gettime(&sttime);\
         *(pu64Time) = ((MI_U64)sttime.tv_sec) * 1000000ULL + (sttime.tv_nsec / 1000);\
    } while(0)
#else
#define GFX_PERF_TIME(pu64Time)
#endif

DECLARE_WAIT_QUEUE_HEAD(gfx_isr_waitqueue);

//-------------------------------------------------------------------------------------------------
//  Local Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MI_GFX_YUV_RGB2YUV_PC = 0,  //Y£º16~235 UV:16~240
    E_MI_GFX_YUV_RGB2YUV_255,     //Y£º0-255  UV:0~255
    E_MI_GFX_YUV_RGB2YUV_MAX
} MI_GFX_Rgb2Yuv_e;

typedef enum
{
    E_MI_GFX_YUV_OUT_255 = 0,    // 0~255
    E_MI_GFX_YUV_OUT_PC,         // Y: 16~235, UV:16~240
    E_MI_GFX_YUV_OUT_MAX
}MI_GFX_YuvOutRange_e;

typedef enum
{
    E_MI_GFX_UV_IN_255 = 0,    // 0~255
    E_MI_GFX_UV_IN_127,        // -128~127
    E_MI_GFX_UV_IN_MAX
} MI_GFX_UvInRange_e;

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_GFX_ResMgr_s
{
    MI_BOOL bInitFlag;
    MI_U8 u8Thresholdvalue;
    MI_U8 u8InitCnt;
} MI_GFX_ResMgr_t;

#if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
typedef struct MI_GFX_ColorFormt_s
{
    GFX_Buffer_Format QuickFillColorFormt;
    GFX_Buffer_Format BitblitSrcColorFormt;
    GFX_Buffer_Format BitblitDstColorFormt;
} MI_GFX_ColorFormt_t;
#endif
//-------------------------------------------------------------------------------------------------
// Local l Variables
//-------------------------------------------------------------------------------------------------
static MI_GFX_ResMgr_t _stGfxMgr = {0};

#if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
static MI_SYS_DRV_HANDLE gGfxDevHdl = NULL;
static MI_GFX_ColorFormt_t stGfxColorFormt;
#endif
//-------------------------------------------------------------------------------------------------
//  local function  prototypes
//-------------------------------------------------------------------------------------------------
static MS_ColorFormat _MI_GFX_MappingColorFmt(MI_GFX_ColorFmt_e eColorFmt)
{
    switch(eColorFmt)
    {
        case E_MI_GFX_FMT_I1:
            return E_MS_FMT_I1;
        case E_MI_GFX_FMT_I2:
            return E_MS_FMT_I2;
        case E_MI_GFX_FMT_I4:
            return E_MS_FMT_I4;
        case E_MI_GFX_FMT_I8:
            return E_MS_FMT_I8;
        case E_MI_GFX_FMT_FABAFGBG2266:
            return E_MS_FMT_FaBaFgBg2266;
        case E_MI_GFX_FMT_1ABFGBG12355:
            return E_MS_FMT_1ABFgBg12355;
        case E_MI_GFX_FMT_RGB565:
            return E_MS_FMT_RGB565;
        case E_MI_GFX_FMT_ARGB1555:
            return E_MS_FMT_ARGB1555;
        case E_MI_GFX_FMT_ARGB4444:
            return E_MS_FMT_ARGB4444;
        case E_MI_GFX_FMT_ARGB1555_DST:
            return E_MS_FMT_ARGB1555_DST;
        case E_MI_GFX_FMT_YUV422:
            return E_MS_FMT_YUV422;
        case E_MI_GFX_FMT_ARGB8888:
            return E_MS_FMT_ARGB8888;
        case E_MI_GFX_FMT_RGBA5551:
            return E_MS_FMT_RGBA5551;
        case E_MI_GFX_FMT_RGBA4444:
            return E_MS_FMT_RGBA4444;
        case E_MI_GFX_FMT_ABGR8888:
            return E_MS_FMT_ABGR8888;
        case E_MI_GFX_FMT_BGRA5551:
            return E_MS_FMT_BGRA5551;
        case E_MI_GFX_FMT_ABGR1555:
            return E_MS_FMT_ABGR1555;
        case E_MI_GFX_FMT_ABGR4444:
            return E_MS_FMT_ABGR4444;
        case E_MI_GFX_FMT_BGRA4444:
            return E_MS_FMT_BGRA4444;
        case E_MI_GFX_FMT_BGR565:
            return E_MS_FMT_BGR565;
        case E_MI_GFX_FMT_RGBA8888:
            return E_MS_FMT_RGBA8888;
        case E_MI_GFX_FMT_BGRA8888:
            return E_MS_FMT_BGRA8888;
        default:
            return E_MS_FMT_ARGB8888;
    }
}

static GFX_DFBBldOP _MI_GFX_GetDfbBlendMode(MI_GFX_DfbBldOp_e eDfbBlendMode)
{
    switch(eDfbBlendMode)
    {
        case E_MI_GFX_DFB_BLD_ZERO:
            return GFX_DFB_BLD_OP_ZERO;
        case E_MI_GFX_DFB_BLD_ONE:
            return GFX_DFB_BLD_OP_ONE;
        case E_MI_GFX_DFB_BLD_SRCCOLOR:
            return GFX_DFB_BLD_OP_SRCCOLOR;
        case E_MI_GFX_DFB_BLD_INVSRCCOLOR:
            return GFX_DFB_BLD_OP_INVSRCCOLOR;
        case E_MI_GFX_DFB_BLD_SRCALPHA:
            return GFX_DFB_BLD_OP_SRCALPHA;
        case E_MI_GFX_DFB_BLD_INVSRCALPHA:
            return GFX_DFB_BLD_OP_INVSRCALPHA;
        case E_MI_GFX_DFB_BLD_DESTALPHA:
            return GFX_DFB_BLD_OP_DESTALPHA;
        case E_MI_GFX_DFB_BLD_INVDESTALPHA:
            return GFX_DFB_BLD_OP_INVDESTALPHA;
        case E_MI_GFX_DFB_BLD_DESTCOLOR:
            return GFX_DFB_BLD_OP_DESTCOLOR;
        case E_MI_GFX_DFB_BLD_INVDESTCOLOR:
            return GFX_DFB_BLD_OP_INVDESTCOLOR;
        case E_MI_GFX_DFB_BLD_SRCALPHASAT:
            return GFX_DFB_BLD_OP_SRCALPHASAT;
        default:
            return GFX_DFB_BLD_OP_ZERO;
    }
}

static GFX_ROP2_Op _MI_GFX_GetROP(MI_GFX_RopCode_e eROP)
{
    switch(eROP)
    {
        case E_MI_GFX_ROP_BLACK:
            return ROP2_OP_ZERO;
        case E_MI_GFX_ROP_NOTMERGEPEN:
            return ROP2_OP_NOT_PS_OR_PD;
        case E_MI_GFX_ROP_MASKNOTPEN:
            return ROP2_OP_NS_AND_PD;
        case E_MI_GFX_ROP_NOTCOPYPEN:
            return ROP2_OP_NS;
        case E_MI_GFX_ROP_MASKPENNOT:
            return ROP2_OP_PS_AND_ND;
        case E_MI_GFX_ROP_NOT:
            return ROP2_OP_ND;
        case E_MI_GFX_ROP_XORPEN:
            return ROP2_OP_PS_XOR_PD;
        case E_MI_GFX_ROP_NOTMASKPEN:
            return ROP2_OP_NOT_PS_AND_PD;
        case E_MI_GFX_ROP_MASKPEN:
            return ROP2_OP_PS_AND_PD;
        case E_MI_GFX_ROP_NOTXORPEN:
            return ROP2_OP_PS_XOR_PD;
        case E_MI_GFX_ROP_NOP:
            return ROP2_OP_PD;
        case E_MI_GFX_ROP_MERGENOTPEN:
            return ROP2_OP_NS_OR_PD;
        case E_MI_GFX_ROP_COPYPEN:
            return ROP2_OP_PS;
        case E_MI_GFX_ROP_MERGEPENNOT:
            return ROP2_OP_PS_OR_ND;
        case E_MI_GFX_ROP_MERGEPEN:
            return ROP2_OP_PD_OR_PS;
        case E_MI_GFX_ROP_WHITE:
            return ROP2_OP_ONE;
        default:
            break;
    }
    return ROP2_OP_PS;
}

#if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)

static MI_S32 _MI_GFX_IMPL_GetCapability(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    handle.OnPrintOut(handle, "\n-----------------------------------------Gfx Dev Attr---------------------------------------\n");
    handle.OnPrintOut(handle, "%9s%16s%21s%22s%23s\n", "InitCnt", "Thresholdvalue", "QuickFillColorFormt", "BitblitSrcColorFormt", "BitblitDstColorFormt");
    handle.OnPrintOut(handle, "%9d", _stGfxMgr.u8InitCnt);
    handle.OnPrintOut(handle, "%16d", _stGfxMgr.u8Thresholdvalue);
    handle.OnPrintOut(handle, "%21d", stGfxColorFormt.QuickFillColorFormt);
    handle.OnPrintOut(handle, "%22d", stGfxColorFormt.BitblitSrcColorFormt);
    handle.OnPrintOut(handle, "%22d\n", stGfxColorFormt.BitblitDstColorFormt);

    return MI_SUCCESS;
}

static MI_S32 _MI_GFX_IMPL_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevID, void *pUsrData)
{
    _MI_GFX_IMPL_GetCapability(handle, u32DevID, 0, NULL, pUsrData);

    return MI_SUCCESS;
}

static MI_U32 MI_Moduledev_RegisterDev(void)
{
    mi_sys_ModuleDevBindOps_t stGfxOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    printf("\033[1;33m MI_Moduledev_RegisterDev \033[0m \n");
    memset(&stGfxOps, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stGfxOps.OnBindInputPort = NULL;
    stGfxOps.OnUnBindInputPort = NULL;
    stGfxOps.OnBindOutputPort = NULL;
    stGfxOps.OnUnBindOutputPort = NULL;
    stGfxOps.OnOutputPortBufRelease = NULL;

    memset(&stModInfo, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId = E_MI_MODULE_ID_GFX;
    stModInfo.u32DevId = 0;
    stModInfo.u32DevChnNum = 0;
    stModInfo.u32InputPortNum = 0;
    stModInfo.u32OutputPortNum = 0;

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_GFX_IMPL_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;

    pstModuleProcfsOps.OnHelp = NULL;
    gGfxDevHdl = mi_sys_RegisterDev(&stModInfo, &stGfxOps, NULL, &pstModuleProcfsOps,MI_COMMON_GetSelfDir);
    if(!gGfxDevHdl)
    {
        DBG_EXIT_ERR("mi_sys_RegisterDev error.\n");
    }
    DBG_INFO("gGfxDevHdl = %p.\n", gGfxDevHdl);

    mi_sys_RegistCommand("getcap", 0, _MI_GFX_IMPL_GetCapability, gGfxDevHdl);
    return MI_SUCCESS;
}
#endif
static MI_S32 _MI_GFX_GetDefaultBlitOpt(MI_GFX_Opt_t *pstBlitOpt)
{
    pstBlitOpt->bEnGfxRop = FALSE;
    pstBlitOpt->eRopCode = E_MI_GFX_ROP_NONE;
    pstBlitOpt->eSrcDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eDstDfbBldOp = E_MI_GFX_DFB_BLD_NONE;
    pstBlitOpt->eMirror = E_MI_GFX_MIRROR_NONE;
    pstBlitOpt->eRotate = E_MI_GFX_ROTATE_0;
    pstBlitOpt->eSrcYuvFmt = 0;
    pstBlitOpt->eDstYuvFmt = 0;
    pstBlitOpt->stClipRect.s32Xpos = 0;
    pstBlitOpt->stClipRect.s32Ypos = 0;
    pstBlitOpt->stClipRect.u32Width  = 0;
    pstBlitOpt->stClipRect.u32Height = 0;

    return MI_SUCCESS;
}

static MI_S32 _MI_GFX_SetBitBlitOption(MI_GFX_Opt_t *pstBlitOpt, MI_GFX_ColorFmt_e eSrcFmt, MI_GFX_ColorFmt_e eDstFmt)
{
    //handle colorkey
    MI_GFX_ColorKeyOp_e eSrcCKeyOP = E_MI_GFX_RGB_OP_EQUAL;
    MI_GFX_ColorKeyOp_e eDstCKeyOP = E_MI_GFX_RGB_OP_EQUAL;
    MI_BOOL bEnSrcColorKey = FALSE;
    MI_BOOL bEnDstColorKey = FALSE;
    GFX_ColorKeyMode eColorKeyMode = CK_OP_EQUAL;
    GFX_Buffer_Format u8SrcFmt = E_MS_FMT_ARGB8888;
    GFX_Buffer_Format u8DstFmt = E_MS_FMT_ARGB8888;
    GFX_RgbColor stColorkeySrcStart = {0, 0, 0, 0};
    GFX_RgbColor stColorkeySrcEnd = {0, 0, 0, 0};
    GFX_RgbColor stColorkeyDstStart = {0, 0, 0, 0};
    GFX_RgbColor stColorkeyDstEnd = {0, 0, 0, 0};
    GFX_DFBBldOP eSrcDfbBldOp = _MI_GFX_GetDfbBlendMode(pstBlitOpt->eSrcDfbBldOp);
    GFX_DFBBldOP eDstDfbBldOp = _MI_GFX_GetDfbBlendMode(pstBlitOpt->eDstDfbBldOp);
    MI_BOOL bRopEnable = pstBlitOpt->bEnGfxRop;
    GFX_ROP2_Op eRopMode = ROP2_OP_ZERO;


    bEnSrcColorKey = pstBlitOpt->stSrcColorKeyInfo.bEnColorKey;
    if (TRUE == bEnSrcColorKey)
    {
        eSrcCKeyOP = pstBlitOpt->stSrcColorKeyInfo.eCKeyOp;
        u8SrcFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(pstBlitOpt->stSrcColorKeyInfo.eCKeyFmt);

        stColorkeySrcStart.a = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorStart >> 24) & 0xFF;
        stColorkeySrcStart.r = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorStart >> 16) & 0xFF;
        stColorkeySrcStart.g = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorStart >> 8) & 0xFF;
        stColorkeySrcStart.b = pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorStart & 0xFF;

        stColorkeySrcEnd.a = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorEnd >> 24) & 0xFF;
        stColorkeySrcEnd.r = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorEnd >> 16) & 0xFF;
        stColorkeySrcEnd.g = (pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorEnd >> 8) & 0xFF;
        stColorkeySrcEnd.b = pstBlitOpt->stSrcColorKeyInfo.stCKeyVal.u32ColorEnd & 0xFF;

        if (E_MI_GFX_ARGB_OP_EQUAL == eSrcCKeyOP) // process rgb & alpha
        {
            eColorKeyMode = CK_OP_EQUAL;
            MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &stColorkeySrcStart, &stColorkeySrcEnd);
            eColorKeyMode = AK_OP_EQUAL;
            MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &stColorkeySrcStart, &stColorkeySrcEnd);
        }
        else if(E_MI_GFX_ARGB_OP_NOT_EQUAL == eSrcCKeyOP)
        {
            eColorKeyMode = CK_OP_NOT_EQUAL;
            MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &stColorkeySrcStart, &stColorkeySrcEnd);
            eColorKeyMode = AK_OP_NOT_EQUAL;
            MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &stColorkeySrcStart, &stColorkeySrcEnd);
        }
        else
        {
            eColorKeyMode = (GFX_ColorKeyMode)eSrcCKeyOP;
            MApi_GFX_SetSrcColorKey(bEnSrcColorKey, eColorKeyMode, u8SrcFmt, &stColorkeySrcStart, &stColorkeySrcEnd);
        }
    }
    else
    {
        memset(&stColorkeySrcStart, 0, sizeof(GFX_RgbColor));
        memset(&stColorkeySrcEnd, 0, sizeof(GFX_RgbColor));
        MApi_GFX_SetSrcColorKey(FALSE, CK_OP_NOT_EQUAL, GFX_FMT_ARGB1555, &stColorkeySrcStart, &stColorkeySrcEnd);
    }

    bEnDstColorKey = pstBlitOpt->stDstColorKeyInfo.bEnColorKey;
    if (TRUE == bEnDstColorKey)
    {
        eDstCKeyOP = pstBlitOpt->stDstColorKeyInfo.eCKeyOp;
        u8DstFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(pstBlitOpt->stDstColorKeyInfo.eCKeyFmt);

        stColorkeyDstStart.a = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorStart >> 24) & 0xFF;
        stColorkeyDstStart.r = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorStart >> 16) & 0xFF;
        stColorkeyDstStart.g = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorStart >> 8) & 0xFF;
        stColorkeyDstStart.b = pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorStart & 0xFF;

        stColorkeyDstEnd.a = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorEnd >> 24) & 0xFF;
        stColorkeyDstEnd.r = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorEnd >> 16) & 0xFF;
        stColorkeyDstEnd.g = (pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorEnd >> 8) & 0xFF;
        stColorkeyDstEnd.b = pstBlitOpt->stDstColorKeyInfo.stCKeyVal.u32ColorEnd & 0xFF;

        if(E_MI_GFX_ARGB_OP_EQUAL == eSrcCKeyOP) // process rgb & alpha
        {
            eColorKeyMode = CK_OP_EQUAL;
            MApi_GFX_SetDstColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &stColorkeyDstStart, &stColorkeyDstEnd);
            eColorKeyMode = AK_OP_EQUAL;
            MApi_GFX_SetDstColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &stColorkeyDstStart, &stColorkeyDstEnd);
        }
        else if(E_MI_GFX_ARGB_OP_NOT_EQUAL == eSrcCKeyOP)
        {
            eColorKeyMode = CK_OP_NOT_EQUAL;
            MApi_GFX_SetDstColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &stColorkeyDstStart, &stColorkeyDstEnd);
            eColorKeyMode = AK_OP_NOT_EQUAL;
            MApi_GFX_SetDstColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &stColorkeyDstStart, &stColorkeyDstEnd);
        }
        else
        {
            eColorKeyMode = (GFX_ColorKeyMode)eSrcCKeyOP;
            MApi_GFX_SetDstColorKey(bEnDstColorKey, eColorKeyMode, u8SrcFmt, &stColorkeyDstStart, &stColorkeyDstEnd);
        }
    }
    else
    {
        memset(&stColorkeyDstStart, 0, sizeof(GFX_RgbColor));
        memset(&stColorkeyDstEnd, 0, sizeof(GFX_RgbColor));
        MApi_GFX_SetSrcColorKey(FALSE, CK_OP_NOT_EQUAL, GFX_FMT_ARGB1555, &stColorkeyDstStart, &stColorkeyDstEnd);
    }

    //handle alpha blend mode
    if ((E_MI_GFX_DFB_BLD_NONE != pstBlitOpt->eSrcDfbBldOp) || (E_MI_GFX_DFB_BLD_NONE != pstBlitOpt->eDstDfbBldOp))
    {
        MApi_GFX_EnableAlphaBlending(FALSE);
        MApi_GFX_SetDFBBldOP(eSrcDfbBldOp, eDstDfbBldOp);
        MApi_GFX_EnableDFBBlending(TRUE);
    }

    //handle rop mode
    if (TRUE == bRopEnable)
    {
        eRopMode = _MI_GFX_GetROP(pstBlitOpt->eRopCode);
        MApi_GFX_SetROP2(bRopEnable, eRopMode);
    }
    else
    {
        MApi_GFX_SetROP2(FALSE, ROP2_OP_ZERO);
    }

    //handle YUV csc fmt
    if ((E_MI_GFX_FMT_YUV422 == eSrcFmt) || (E_MI_GFX_FMT_YUV422 == eDstFmt))
    {
        MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_PC, GFX_YUV_IN_255, pstBlitOpt->eSrcYuvFmt, pstBlitOpt->eDstYuvFmt);
    }

    //handle rotation
    MApi_GFX_SetRotate((GFX_RotateAngle)pstBlitOpt->eRotate);

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_Open(void)
{
    GFX_Config stGFXcfg;
    const MI_SYSCFG_MmapInfo_t* pstMmapInfo = NULL;

    _stGfxMgr.u8InitCnt++;
    if ((_stGfxMgr.bInitFlag))
    {
        //DBG_WRN("%s: Module has been initialized!\n", __FUNCTION__);
        return MI_SUCCESS;
    }
    memset(&stGFXcfg,0,sizeof(GFX_Config));
    MI_SYSCFG_GetMmapInfo("E_MMAP_ID_GE_VQ", &pstMmapInfo);
    if(pstMmapInfo != NULL)
    {
        printk("Main buffer Addr =  0x%x \n", pstMmapInfo->u32Addr);
        printk("Main buffer size =  0x%x \n", pstMmapInfo->u32Size);
        printk("Main buffer u32Align =  0x%x \n", pstMmapInfo->u32Align);
        printk("Main buffer u8MiuNo =  0x%x \n", pstMmapInfo->u8MiuNo);
        stGFXcfg.u32VCmdQAddr = pstMmapInfo->u32Addr;
        stGFXcfg.u32VCmdQSize = pstMmapInfo->u32Size;
        stGFXcfg.u8Miu = pstMmapInfo->u8MiuNo;
    }
    else
    {
        printk("Get GE VQ buffer Fail.\n");
    }

    stGFXcfg.bIsCompt = TRUE;
    stGFXcfg.bIsHK = TRUE;
    MApi_GFX_Init(&stGFXcfg);

    if(stGFXcfg.u32VCmdQSize >= 4*1024)
    {
        GFX_VcmqBufSize vqSize;
        GFX_Result ret = GFX_FAIL;

        if(stGFXcfg.u32VCmdQSize >= 512*1024)
            vqSize = GFX_VCMD_512K;
        else if(stGFXcfg.u32VCmdQSize >= 256*1024)
            vqSize = GFX_VCMD_256K;
        else if(stGFXcfg.u32VCmdQSize >= 128*1024)
            vqSize = GFX_VCMD_128K;
        else if(stGFXcfg.u32VCmdQSize >= 64*1024)
            vqSize = GFX_VCMD_64K;
        else if(stGFXcfg.u32VCmdQSize >= 32*1024)
            vqSize = GFX_VCMD_32K;
        else if(stGFXcfg.u32VCmdQSize >= 16*1024)
            vqSize = GFX_VCMD_16K;
        else if(stGFXcfg.u32VCmdQSize >= 8*1024)
            vqSize = GFX_VCMD_8K;
        else
            vqSize = GFX_VCMD_4K;
        ret = MApi_GFX_SetVCmdBuffer(stGFXcfg.u32VCmdQAddr, vqSize);

        if(GFX_SUCCESS == ret)
            MApi_GFX_EnableVCmdQueue(TRUE);
        else
            MApi_GFX_EnableVCmdQueue(FALSE);
    }
    else
    {
        MApi_GFX_EnableVCmdQueue(FALSE);
    }

    _stGfxMgr.bInitFlag = TRUE;
    _stGfxMgr.u8Thresholdvalue = 1; //default

#if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
    MI_Moduledev_RegisterDev();
#endif

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_Close(void)
{
//    MI_U8 u8GfxExit = 0;
    _stGfxMgr.u8InitCnt--;
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }
    if (0 == _stGfxMgr.u8InitCnt)
    {
        //MApi_GFX_Close(&u8GfxExit);
        _stGfxMgr.bInitFlag = FALSE;
    }

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_PendingDone(MI_U16 u16TargetFence)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    if (GFX_SUCCESS != MApi_GFX_PollingTAGID(u16TargetFence))
    {
        return MI_ERR_GFX_DEV_BUSY;
    }
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    ;//
#endif

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_WaitAllDone(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence, struct mutex *pstMutex)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    if (TRUE == bWaitAllDone)
    {
         u16TargetFence = MApi_GFX_SetNextTAGID();
    }
    while(1)
    {
        if(MI_SUCCESS == MI_GFX_IMPL_PendingDone(u16TargetFence))
        {
            return MI_SUCCESS;
        }
        mutex_unlock(pstMutex);
        //msleep(1);
        interruptible_sleep_on_timeout(&gfx_isr_waitqueue, msecs_to_jiffies(1));
        mutex_lock(pstMutex);
    }
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    if (TRUE == bWaitAllDone)
    {
        MI_U16 u16TagId = MApi_GFX_SetNextTAGID();
        MApi_GFX_WaitForTAGID(u16TagId);
    }
    else
    {
        MApi_GFX_WaitForTAGID(u16TargetFence);
    }
#endif
    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
    MI_U32 u32ColorVal, MI_U16 *pu16Fence)
{
    MI_GFX_Opt_t stBlitOpt;
    GFX_RgbColor stColor = {0,0,0,0};
    GFX_RectFillInfo stFillBlk;
    GFX_Point stPiont0, stPiont1;
    GFX_BufferInfo stDstBuff;
    GFX_Result eRet;

    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }

    GFX_ISVALID_POINT(pstDst);
    GFX_ISVALID_POINT(pstDstRect);
    GFX_ISVALID_POINT(pu16Fence);
    memset(&stBlitOpt, 0x0, sizeof(MI_GFX_Opt_t));
    //INIT_ST(stBlitOpt);
    _MI_GFX_GetDefaultBlitOpt(&stBlitOpt);

    MApi_GFX_BeginDraw();
    //set destination area color
    memset(&stColor, 0x0,sizeof(GFX_RgbColor));
    stColor.a = (u32ColorVal >> 24) & 0xFF;
    stColor.r = (u32ColorVal >> 16) & 0xFF;
    stColor.g = (u32ColorVal >> 8) & 0xFF;
    stColor.b = (u32ColorVal) & 0xFF;

    // set destination clip area
    stPiont0.x = 0;
    stPiont0.y = 0;
    stPiont1.x = pstDst->u32Width;
    stPiont1.y = pstDst->u32Height;
    MApi_GFX_SetClip(&stPiont0, &stPiont1);

    // set destination buffer info
    stDstBuff.u32Addr = pstDst->phyAddr;
    stDstBuff.u32Width = pstDst->u32Width;
    stDstBuff.u32Height = pstDst->u32Height;
    stDstBuff.u32Pitch = pstDst->u32Stride;
    stDstBuff.u32ColorFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(pstDst->eColorFmt);
    if (E_MI_GFX_FMT_ARGB1555 == pstDst->eColorFmt)
    {
        stDstBuff.u32ColorFmt = GFX_FMT_ARGB1555_DST;
    }
    MApi_GFX_SetDstBufferInfo(&stDstBuff,0);
    #if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
	stGfxColorFormt.QuickFillColorFormt = stDstBuff.u32ColorFmt;
    #endif
    //Config the fill rect params to vars
    stFillBlk.dstBlock.x = pstDstRect->s32Xpos;
    stFillBlk.dstBlock.y = pstDstRect->s32Ypos;
    stFillBlk.dstBlock.width = pstDstRect->u32Width;
    stFillBlk.dstBlock.height = pstDstRect->u32Height;
    stFillBlk.colorRange.color_s = stColor;
    stFillBlk.colorRange.color_e = stColor;
    stFillBlk.fmt = stDstBuff.u32ColorFmt;
    stFillBlk.flag = GFXRECT_FLAG_COLOR_CONSTANT;
    eRet = MApi_GFX_RectFill(&stFillBlk);
    if (GFX_SUCCESS != eRet)
    {
        DBG_ERR("MApi_GFX_BitBlt Fail rRet :%d \n",eRet);
        MApi_GFX_EndDraw();
        switch (eRet)
        {
            case GFX_INVALID_PARAMETERS:
                return MI_ERR_GFX_INVALID_PARAM;
            case GFX_DRV_NOT_SUPPORT:
                return MI_ERR_GFX_DRV_NOT_SUPPORT;
            case GFX_DRV_FAIL_FORMAT:
                return MI_ERR_GFX_DRV_FAIL_FORMAT;
            case GFX_NON_ALIGN_ADDRESS:
                return MI_ERR_GFX_NON_ALIGN_ADDRESS;
            case GFX_NON_ALIGN_PITCH:
                return MI_ERR_GFX_NON_ALIGN_PITCH;
            case GFX_DRV_FAIL_BLTADDR:
                return MI_ERR_GFX_DRV_FAIL_BLTADDR;
            case GFX_DRV_FAIL_OVERLAP:
                return MI_ERR_GFX_DRV_FAIL_OVERLAP;
            case GFX_DRV_FAIL_STRETCH:
                return MI_ERR_GFX_DRV_FAIL_STRETCH;
            case GFX_DRV_FAIL_ITALIC:
                return MI_ERR_GFX_DRV_FAIL_ITALIC;
            case GFX_DRV_FAIL_LOCKED:
                return MI_ERR_GFX_DRV_FAIL_LOCKED;
            default:
                return MI_ERR_GFX_INVALID_PARAM;
        }
    }
    *pu16Fence = MApi_GFX_SetNextTAGID();
    MApi_GFX_EndDraw();

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_GetAlphaThresholdValue(MI_U8 *pu8ThresholdValue)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }

    GFX_ISVALID_POINT(pu8ThresholdValue);
    *pu8ThresholdValue = _stGfxMgr.u8Thresholdvalue;

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_SetAlphaThresholdValue(MI_U8 u8ThresholdValue)
{
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }
    //if (GFX_SUCCESS != MApi_GFX_SetAlpha_ARGB1555(u8ThresholdValue))
    //{
    //   DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
    //    return MI_ERR_GFX_INVALID_PARAM;
    //}
    _stGfxMgr.u8Thresholdvalue = u8ThresholdValue;

    return MI_SUCCESS;
}

MI_S32 MI_GFX_IMPL_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence)
{
    GFX_DrawRect stBitBltInfo;
    GFX_BufferInfo stGfxSrcBuf, stGfxDstBuf;
    GFX_Point stPiont0, stPiont1;
    MI_S32 s32Ret = -1;
    MI_U32 u32DrawFlag = 0;
    GFX_Result eRet;
#if defined(ENABLE_GFX_TRACE_PERF) && (ENABLE_GFX_TRACE_PERF == 1)
    MI_U64 u64BeginDraw = 0;
    MI_U64 u64BeginBuffer = 0;
    MI_U64 u64BeginClip = 0;
    MI_U64 u64BeginBitBlt = 0;
    MI_U64 u64BeginEndDraw = 0;
    MI_U64 u64End = 0;
#endif
    if (!_stGfxMgr.bInitFlag)
    {
        DBG_WRN("%s: Module is NOT initialized!\n", __FUNCTION__);
        return MI_ERR_GFX_NOT_INIT;
    }

    GFX_ISVALID_POINT(pstSrc);
    GFX_ISVALID_POINT(pstSrcRect);
    GFX_ISVALID_POINT(pstDst);
    GFX_ISVALID_POINT(pstDstRect);
    GFX_ISVALID_POINT(pstOpt);
    GFX_ISVALID_POINT(pu16Fence);

    GFX_PERF_TIME(&u64BeginDraw);
    MApi_GFX_BeginDraw();

    s32Ret = _MI_GFX_SetBitBlitOption(pstOpt, pstSrc->eColorFmt, pstDst->eColorFmt);
    if (MI_SUCCESS == s32Ret)
    {
        DBG_INFO("_MI_GFX_SetBitBlitOption success\n");
    }
    else
    {
        DBG_ERR("_MI_GFX_SetBitBlitOption Fail!!!\n");
        MApi_GFX_EndDraw();
        return s32Ret;
    }
    switch (pstOpt->eMirror)
    {
        case E_MI_GFX_MIRROR_VERTICAL://error
            pstSrcRect->s32Xpos += pstSrcRect->u32Width - 1;
            MApi_GFX_SetMirror(TRUE, FALSE);
            break;
        case E_MI_GFX_MIRROR_HORIZONTAL:
            pstSrcRect->s32Ypos += pstSrcRect->u32Height - 1;
            MApi_GFX_SetMirror(FALSE, TRUE);
            break;
        case E_MI_GFX_MIRROR_BOTH:
            pstSrcRect->s32Xpos += pstSrcRect->u32Width - 1;
            pstSrcRect->s32Ypos += pstSrcRect->u32Height - 1;
            MApi_GFX_SetMirror(TRUE, TRUE);
            break;
        case E_MI_GFX_MIRROR_NONE:
        default:
            MApi_GFX_SetMirror(FALSE, FALSE);
            break;
    }

    GFX_PERF_TIME(&u64BeginBuffer);
    stGfxSrcBuf.u32ColorFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(pstSrc->eColorFmt);
    stGfxSrcBuf.u32Addr = pstSrc->phyAddr;
    stGfxSrcBuf.u32Pitch = pstSrc->u32Stride;
    stGfxSrcBuf.u32Width = pstSrc->u32Width;
    stGfxSrcBuf.u32Height = pstSrc->u32Height;
    MApi_GFX_SetSrcBufferInfo(&stGfxSrcBuf, 0);
#if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
    stGfxColorFormt.BitblitSrcColorFormt = stGfxSrcBuf.u32ColorFmt;
#endif
    stGfxDstBuf.u32ColorFmt = (GFX_Buffer_Format)_MI_GFX_MappingColorFmt(pstDst->eColorFmt);
    stGfxDstBuf.u32Addr = pstDst->phyAddr;
    stGfxDstBuf.u32Pitch = pstDst->u32Stride;
    stGfxDstBuf.u32Width = pstDst->u32Width;
    stGfxDstBuf.u32Height = pstDst->u32Height;
    #if defined(MI_SYS_PROC_FS_DEBUG)&&(MI_GFX_PROCFS_DEBUG == 1)
	stGfxColorFormt.BitblitDstColorFormt = stGfxDstBuf.u32ColorFmt;
    #endif
    if(E_MI_GFX_FMT_ARGB1555 == pstDst->eColorFmt)
        stGfxDstBuf.u32ColorFmt = GFX_FMT_ARGB1555_DST;
    MApi_GFX_SetDstBufferInfo(&stGfxDstBuf, 0);

    stPiont0.x = 0;
    stPiont0.y = 0;

    if (pstSrc->u32Width >= pstDst->u32Width)
    {
        stPiont1.x = pstSrc->u32Width;
    }
    else
    {
        stPiont1.x = pstDst->u32Width;
    }
    if (pstSrc->u32Height >= pstDst->u32Height)
    {
        stPiont1.y = pstSrc->u32Height;
    }
    else
    {
        stPiont1.y = pstDst->u32Height;
    }

    GFX_PERF_TIME(&u64BeginClip);
    MApi_GFX_SetClip(&stPiont0, &stPiont1);

    stBitBltInfo.srcblk.height = pstSrcRect->u32Height;
    stBitBltInfo.srcblk.width = pstSrcRect->u32Width;
    stBitBltInfo.srcblk.x = pstSrcRect->s32Xpos;
    stBitBltInfo.srcblk.y = pstSrcRect->s32Ypos;

    stBitBltInfo.dstblk.height = pstDstRect->u32Height;
    stBitBltInfo.dstblk.width = pstDstRect->u32Width;
    stBitBltInfo.dstblk.x = pstDstRect->s32Xpos;
    stBitBltInfo.dstblk.y = pstDstRect->s32Ypos;

    if ((pstSrcRect->u32Width == pstDstRect->u32Width) && (pstSrcRect->u32Height == pstDstRect->u32Height))
    {
        u32DrawFlag = GFXDRAW_FLAG_DEFAULT;
    }
    else // stretch bitblt, driver will switch  to 1P mode automatically
    {
        u32DrawFlag = GFXDRAW_FLAG_SCALE; // scale
    }

    GFX_PERF_TIME(&u64BeginBitBlt);
    eRet = MApi_GFX_BitBlt(&stBitBltInfo, u32DrawFlag);
    if(GFX_SUCCESS != eRet)
    {
        DBG_ERR("MApi_GFX_BitBlt Fail rRet :%d \n",eRet);
        MApi_GFX_EndDraw();
        switch (eRet)
        {
            case GFX_INVALID_PARAMETERS:
                return MI_ERR_GFX_INVALID_PARAM;
            case GFX_DRV_NOT_SUPPORT:
                return MI_ERR_GFX_DRV_NOT_SUPPORT;
            case GFX_DRV_FAIL_FORMAT:
                return MI_ERR_GFX_DRV_FAIL_FORMAT;
            case GFX_NON_ALIGN_ADDRESS:
                return MI_ERR_GFX_NON_ALIGN_ADDRESS;
            case GFX_NON_ALIGN_PITCH:
                return MI_ERR_GFX_NON_ALIGN_PITCH;
            case GFX_DRV_FAIL_BLTADDR:
                return MI_ERR_GFX_DRV_FAIL_BLTADDR;
            case GFX_DRV_FAIL_OVERLAP:
                return MI_ERR_GFX_DRV_FAIL_OVERLAP;
            case GFX_DRV_FAIL_STRETCH:
                return MI_ERR_GFX_DRV_FAIL_STRETCH;
            case GFX_DRV_FAIL_ITALIC:
                return MI_ERR_GFX_DRV_FAIL_ITALIC;
            case GFX_DRV_FAIL_LOCKED:
                return MI_ERR_GFX_DRV_FAIL_LOCKED;
            default:
                return MI_ERR_GFX_INVALID_PARAM;
        }
    }
    *pu16Fence = MApi_GFX_SetNextTAGID();
    GFX_PERF_TIME(&u64BeginEndDraw);

    MApi_GFX_EndDraw();
    GFX_PERF_TIME(&u64End);
#if defined(ENABLE_GFX_TRACE_PERF) && (ENABLE_GFX_TRACE_PERF == 1)
    if ((u64End - u64BeginDraw) > 5000)
    {
        printk("GFX: BeginDraw: %llu Buffer: %llu, Clip: %llu, BitBlit: %llu, EnDraw: %llu.\n",
            u64BeginBuffer - u64BeginDraw,  u64BeginClip - u64BeginBuffer, u64BeginBitBlt - u64BeginClip,
            u64BeginEndDraw - u64BeginBitBlt, u64End - u64BeginEndDraw);
    }
#endif
    return MI_SUCCESS;
}
