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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#define __DRV_PNL_IO_WRAPPER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_NOS
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#endif


#include "cam_os_wrapper.h"
#include "ms_platform.h"
#include "ms_types.h"

#include "drv_pnl_dbg.h"
#include "hal_pnl_util.h"

#include "drv_pnl_verchk.h"
#include "drv_pnl_io_st.h"
#include "drv_pnl_io_wrapper.h"
#include "drv_pnl.h"
#include "drv_pnl_dbg.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#if 0

#define DRV_PNL_IO_LOCK_MUTEX(x)    \
    PNL_ERR("+++ [MUTEX_LOCK][%s]_1_[%d] \n", __FUNCTION__, __LINE__); \
    CamOsMutexLock(x, SCLOS_WAIT_FOREVER); \
    PNL_ERR("+++ [MUTEX_LOCK][%s]_2_[%d] \n", __FUNCTION__, __LINE__);

#define DRV_PNL_IO_UNLOCK_MUTEX(x)  \
    PNL_ERR("--- [MUTEX_LOCK][%s]   [%d] \n", __FUNCTION__, __LINE__); \
    CamOsMutexUnlock(x);



#else
#if defined(MSOS_TYPE_NOS)
#define DRV_PNL_IO_LOCK_MUTEX(x)
#define DRV_PNL_IO_UNLOCK_MUTEX(x)
#else
#define DRV_PNL_IO_LOCK_MUTEX(x)    CamOsMutexLock(x)
#define DRV_PNL_IO_UNLOCK_MUTEX(x)  CamOsMutexUnlock(x)
#endif

#endif

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvPnlHandleConfig_t _gstPnlHandler[DRV_PNL_HANDLER_MAX];


CamOsMutex_t stPnlIoHandlerMutex;
bool bDrvPnlIoInit = FALSE;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
DrvPnlLinkType_e _DrvPolIoTransLinkTypeIo2Drv(DrvPnlIoLinkType_e enIoLinkType)
{
    DrvPnlLinkType_e enDrvLinkType;

    enDrvLinkType = enIoLinkType == E_DRV_PNL_IO_LINK_LVDS     ? E_DRV_PNL_LINK_LVDS :
                    enIoLinkType == E_DRV_PNL_IO_LINK_MIPI_DSI ? E_DRV_PNL_LINK_MIPI_DSI :
                    enIoLinkType == E_DRV_PNL_IO_LINK_TTL      ? E_DRV_PNL_LINK_TTL :
                    enIoLinkType == E_DRV_PNL_IO_LINK_VBY1     ? E_DRV_PNL_LINK_VBY1 :
                                                                 E_DRV_PNL_LINK_MAX;
    return enDrvLinkType;
}

DrvPnlIoLinkType_e _DrvPnlIoTransLinkTypeDrv2Io(DrvPnlLinkType_e enDrvLinkType)
{
    DrvPnlIoLinkType_e enIoLinkType;

    enIoLinkType = enDrvLinkType == E_DRV_PNL_LINK_LVDS     ? E_DRV_PNL_IO_LINK_LVDS :
                   enDrvLinkType == E_DRV_PNL_LINK_MIPI_DSI ? E_DRV_PNL_IO_LINK_MIPI_DSI :
                   enDrvLinkType == E_DRV_PNL_LINK_TTL      ? E_DRV_PNL_IO_LINK_TTL :
                   enDrvLinkType == E_DRV_PNL_LINK_VBY1     ? E_DRV_PNL_IO_LINK_VBY1 :
                                                              E_DRV_PNL_LINK_MAX;
    return enIoLinkType;
}


DrvPnlMipiDsiLaneNum_e _DrvPnlIoTransMipiDsiLaneNumIo2Drv(DrvPnlIoMipiDsiLaneNum_e enIoLaneNum)
{
    DrvPnlMipiDsiLaneNum_e enDrvLaneNum;

    enDrvLaneNum = enIoLaneNum == E_DRV_PNL_IO_MIPI_DSI_LANE_1 ? E_DRV_PNL_MIPI_DSI_LANE_1 :
                   enIoLaneNum == E_DRV_PNL_IO_MIPI_DSI_LANE_2 ? E_DRV_PNL_MIPI_DSI_LANE_2 :
                   enIoLaneNum == E_DRV_PNL_IO_MIPI_DSI_LANE_3 ? E_DRV_PNL_MIPI_DSI_LANE_3 :
                   enIoLaneNum == E_DRV_PNL_IO_MIPI_DSI_LANE_4 ? E_DRV_PNL_MIPI_DSI_LANE_4 :
                                                                 E_DRV_PNL_MIPI_DSI_LANE_NONE;

    return enDrvLaneNum;
}

DrvPnlMipiDsiCtrlMode_e _DrvPnlIoTransMipiDsiCtrlModeIo2Drv(DrvPnlIoMipiDsiCtrlMode_e enIoCtrlMode)
{
    DrvPnlMipiDsiCtrlMode_e enDrvCtrlMode;

    enDrvCtrlMode = enIoCtrlMode == E_DRV_PNL_IO_MIPI_DSI_CMD_MODE   ? E_DRV_PNL_MIPI_DSI_CMD_MODE   :
                    enIoCtrlMode == E_DRV_PNL_IO_MIPI_DSI_SYNC_PULSE ? E_DRV_PNL_MIPI_DSI_SYNC_PULSE :
                    enIoCtrlMode == E_DRV_PNL_IO_MIPI_DSI_SYNC_EVENT ? E_DRV_PNL_MIPI_DSI_SYNC_EVENT :
                                                                       E_DRV_PNL_MIPI_DSI_BURST_MODE;
    return enDrvCtrlMode;
}

DrvPnlMipiDsiFormat_e _DrvPnlIoTransMipiDsiFormatIo2Drv(DrvPnlIoMipiDsiFormat_e enIoFormat)
{
    DrvPnlMipiDsiFormat_e enDrvFormat;

    enDrvFormat = enIoFormat == E_DRV_PNL_IO_MIPI_DSI_RGB565         ? E_DRV_PNL_MIPI_DSI_RGB565 :
                  enIoFormat == E_DRV_PNL_IO_MIPI_DSI_RGB666         ? E_DRV_PNL_MIPI_DSI_RGB666 :
                  enIoFormat == E_DRV_PNL_IO_MIPI_DSI_LOOSELY_RGB666 ? E_DRV_PNL_MIPI_DSI_LOOSELY_RGB666 :
                                                                       E_DRV_PNL_MIPI_DSI_RGB888;
    return enDrvFormat;
}

void _DrvPnlIoTransMipiCfgIo2Drv(DrvPnlIoMipiDsiConfig_t *pIoMipiDsiCfg, DrvPnlMipiDsiConfig_t *pDrvMipiDsiCfg)
{
    pDrvMipiDsiCfg->u8HsTrail   = pIoMipiDsiCfg->u8HsTrail;
    pDrvMipiDsiCfg->u8HsPrpr    = pIoMipiDsiCfg->u8HsPrpr;
    pDrvMipiDsiCfg->u8HsZero    = pIoMipiDsiCfg->u8HsZero;
    pDrvMipiDsiCfg->u8ClkHsPrpr = pIoMipiDsiCfg->u8ClkHsPrpr;
    pDrvMipiDsiCfg->u8ClkHsExit = pIoMipiDsiCfg->u8ClkHsExit;
    pDrvMipiDsiCfg->u8ClkTrail  = pIoMipiDsiCfg->u8ClkTrail;
    pDrvMipiDsiCfg->u8ClkZero   = pIoMipiDsiCfg->u8ClkZero;
    pDrvMipiDsiCfg->u8ClkHsPost = pIoMipiDsiCfg->u8ClkHsPost;
    pDrvMipiDsiCfg->u8DaHsExit  = pIoMipiDsiCfg->u8DaHsExit;
    pDrvMipiDsiCfg->u8ContDet   = pIoMipiDsiCfg->u8ContDet;

    pDrvMipiDsiCfg->u8Lpx       = pIoMipiDsiCfg->u8Lpx;
    pDrvMipiDsiCfg->u8TaGet     = pIoMipiDsiCfg->u8TaGet;
    pDrvMipiDsiCfg->u8TaSure    = pIoMipiDsiCfg->u8TaSure;
    pDrvMipiDsiCfg->u8TaGo      = pIoMipiDsiCfg->u8TaGo;

    pDrvMipiDsiCfg->u16Hactive  = pIoMipiDsiCfg->u16Hactive;
    pDrvMipiDsiCfg->u16Hpw      = pIoMipiDsiCfg->u16Hpw;
    pDrvMipiDsiCfg->u16Hbp      = pIoMipiDsiCfg->u16Hbp;
    pDrvMipiDsiCfg->u16Hfp      = pIoMipiDsiCfg->u16Hfp;

    pDrvMipiDsiCfg->u16Vactive  = pIoMipiDsiCfg->u16Vactive;
    pDrvMipiDsiCfg->u16Vpw      = pIoMipiDsiCfg->u16Vpw;
    pDrvMipiDsiCfg->u16Vbp      = pIoMipiDsiCfg->u16Vbp;
    pDrvMipiDsiCfg->u16Vfp      = pIoMipiDsiCfg->u16Vfp;

    pDrvMipiDsiCfg->u16Bllp     = pIoMipiDsiCfg->u16Bllp;
    pDrvMipiDsiCfg->u16Fps      = pIoMipiDsiCfg->u16Fps;

    pDrvMipiDsiCfg->enLaneNum   = _DrvPnlIoTransMipiDsiLaneNumIo2Drv(pIoMipiDsiCfg->enLaneNum);
    pDrvMipiDsiCfg->enCtrl      = _DrvPnlIoTransMipiDsiCtrlModeIo2Drv(pIoMipiDsiCfg->enCtrl);
    pDrvMipiDsiCfg->enformat    = _DrvPnlIoTransMipiDsiFormatIo2Drv(pIoMipiDsiCfg->enformat);

    pDrvMipiDsiCfg->pu8CmdBuf     = pIoMipiDsiCfg->pu8CmdBuf;
    pDrvMipiDsiCfg->u32CmdBufSize = pIoMipiDsiCfg->u32CmdBufSize;
}

void _DrvPnlIoTransModCfgIo2Drv(DrvPnlIoModConfig_t *pIoModCfg,  DrvPnlModConfig_t *pDrvModCfg)
{
    u8 i;

    pDrvModCfg->bTiMode      = pIoModCfg->bTiMode;
    pDrvModCfg->bChPolarity  = pIoModCfg->bChPolarity;
    pDrvModCfg->bHsyncInvert = pIoModCfg->bHsyncInvert;
    pDrvModCfg->bVsyncInvert = pIoModCfg->bVsyncInvert;
    pDrvModCfg->bDeInvert    = pIoModCfg->bDeInvert;
    pDrvModCfg->enTiBitMode  =
        pIoModCfg->enTiBitMode == E_DRV_PNL_IO_TI_BIT_8 ? E_DRV_PNL_TI_BIT_8 :
        pIoModCfg->enTiBitMode == E_DRV_PNL_IO_TI_BIT_6 ? E_DRV_PNL_TI_BIT_6 :
                                                                          E_DRV_PNL_TI_BIT_10;
    for(i=0; i<DRV_PNL_IO_SIGNAL_CTRL_CH_MAX; i++)
    {
        pDrvModCfg->enCh[i] =
            pIoModCfg->enCh[i] == E_DRV_PNL_IO_CH_SWAP_0 ? E_DRV_PNL_CH_SWAP_0 :
            pIoModCfg->enCh[i] == E_DRV_PNL_IO_CH_SWAP_1 ? E_DRV_PNL_CH_SWAP_1 :
            pIoModCfg->enCh[i] == E_DRV_PNL_IO_CH_SWAP_2 ? E_DRV_PNL_CH_SWAP_2 :
            pIoModCfg->enCh[i] == E_DRV_PNL_IO_CH_SWAP_3 ? E_DRV_PNL_CH_SWAP_3 :
                                                           E_DRV_PNL_CH_SWAP_4;

    }

}


bool _DrvPnlGetCtx(s32 s32Handler, DrvPnlIoCtxConfig_t *pCtxCfg)
{
    bool bRet = TRUE;
    s16  i;
    s16 s16Idx = -1;

    DRV_PNL_IO_LOCK_MUTEX(&stPnlIoHandlerMutex);

    for(i = 0; i < DRV_PNL_HANDLER_MAX; i++)
    {
        if(_gstPnlHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        pCtxCfg->pCtx = NULL;
        bRet = FALSE;
        PNL_ERR( "[PNL]   %s %d  \n", __FUNCTION__, __LINE__);
    }
    else
    {
        pCtxCfg->pCtx = _gstPnlHandler[s16Idx].stCtxCfg.pCtx;
        if(pCtxCfg->pCtx != NULL)
        {
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
            PNL_ERR( "[PNL]   %s %d  \n", __FUNCTION__, __LINE__);
        }
    }

    DRV_PNL_IO_UNLOCK_MUTEX(&stPnlIoHandlerMutex);

    return bRet;
}



DrvPnlVersionChkConfig_t _DrvPnlFillVersionChkStruct(u32 u32StructSize, u32 u32VersionSize, u32 *pVersion)
{
    DrvPnlVersionChkConfig_t stVersion;
    stVersion.u32StructSize  = (u32)u32StructSize;
    stVersion.u32VersionSize = (u32)u32VersionSize;
    stVersion.pVersion      = (u32 *)pVersion;
    return stVersion;
}


s32 _DrvPnlVersionCheck(DrvPnlVersionChkConfig_t stVersion)
{
    if ( CHK_VERCHK_HEADER(stVersion.pVersion) )
    {
        if( CHK_VERCHK_MAJORVERSION_LESS( stVersion.pVersion, DRV_PNL_VERSION) )
        {

            VERCHK_ERR("[PNL] Version(%04x) < %04x!!! \n",
                       *(stVersion.pVersion) & VERCHK_VERSION_MASK,
                       DRV_PNL_VERSION);

            return -1;
        }
        else
        {
            if( CHK_VERCHK_SIZE( &stVersion.u32VersionSize, stVersion.u32StructSize) == 0 )
            {
                VERCHK_ERR("[PNL] Size(%04x) != %04x!!! \n",
                           stVersion.u32StructSize,
                           stVersion.u32VersionSize);

                return -1;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        VERCHK_ERR("[PNL] No Header !!! \n");
        PNL_ERR( "[PNL]   %s  \n", __FUNCTION__);
        return -1;
    }
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool _DrvPnlIoInit(void)
{
    u8 i;

    if(bDrvPnlIoInit == TRUE)
    {
        PNL_DBG(PNL_DBG_LEVEL_IO, "[PNL] %s %d, Already Init \n", __FUNCTION__, __LINE__ );
        return TRUE;
    }

#if !defined(MSOS_TYPE_NOS)
    if( CamOsMutexInit(&stPnlIoHandlerMutex) != CAM_OS_OK)
    {
        PNL_ERR("%s %d, Create Mutex Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    DrvPnlInit();

    for(i=0; i<DRV_PNL_HANDLER_MAX; i++)
    {
        _gstPnlHandler[i].stCtxCfg.pCtx = NULL;
        _gstPnlHandler[i].s32Handle = -1;
    }
    bDrvPnlIoInit = TRUE;

    return TRUE;
}

s32 _DrvPnlIoOpen(void)
{
    s32 s32Handle = -1;
    s16 s16Idx = -1;
    s16 i ;

    DRV_PNL_IO_LOCK_MUTEX(&stPnlIoHandlerMutex);

    for(i=0; i<DRV_PNL_HANDLER_MAX; i++)
    {
        if(_gstPnlHandler[i].s32Handle == -1)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        s32Handle = -1;
        PNL_ERR("%s %d, Handler is not empyt\n", __FUNCTION__, __LINE__);
    }
    else
    {
        void *pCtx = DrvPnlCtxAllocate();
        if(pCtx)
        {
            s32Handle = s16Idx | DRV_PNL_HANDLER_PRE_FIX;
            _gstPnlHandler[s16Idx].s32Handle = s32Handle ;
            _gstPnlHandler[s16Idx].stCtxCfg.pCtx = pCtx;
        }
        else
        {
            PNL_ERR("%s %d::Allocate Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }

    DRV_PNL_IO_UNLOCK_MUTEX(&stPnlIoHandlerMutex);

    return s32Handle;
}


DrvPnlIoErrType_e _DrvPnlIoClose(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;
    DrvPnlIoErrType_e eRet = E_DRV_PNL_IO_ERR_OK;

    DRV_PNL_IO_LOCK_MUTEX(&stPnlIoHandlerMutex);

    for(i=0; i<DRV_PNL_HANDLER_MAX; i++)
    {
        if(_gstPnlHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        PNL_ERR( "%s %d  idx is -1\n", __FUNCTION__, __LINE__);
        eRet = E_DRV_PNL_IO_ERR_FAULT;
    }
    else
    {
        _gstPnlHandler[s16Idx].s32Handle = -1;
        DrvPnlCtxFree(_gstPnlHandler[s16Idx].stCtxCfg.pCtx);
        _gstPnlHandler[s16Idx].stCtxCfg.pCtx = NULL;
        eRet = E_DRV_PNL_IO_ERR_OK;
    }

    DRV_PNL_IO_UNLOCK_MUTEX(&stPnlIoHandlerMutex);

    return eRet;
}


DrvPnlIoErrType_e _DrvPnlIoSetTimingConfig(s32 s32Handler, DrvPnlIoTimingConfig_t *pstIoTimingCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlTimingConfig_t stPnlTimingCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoTimingConfig_t),
                                                 pstIoTimingCfg->VerChk_Size,
                                                 &pstIoTimingCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    memset(&stPnlTimingCfg, 0, sizeof(DrvPnlTimingConfig_t));

    stPnlTimingCfg.enLinkType    = _DrvPolIoTransLinkTypeIo2Drv(pstIoTimingCfg->enLinkType);
    stPnlTimingCfg.u16Htt        = pstIoTimingCfg->u16Htt;
    stPnlTimingCfg.u16Hpw        = pstIoTimingCfg->u16Hpw;
    stPnlTimingCfg.u16Hbp        = pstIoTimingCfg->u16Hbp;
    stPnlTimingCfg.u16Hactive    = pstIoTimingCfg->u16Hactive;
    stPnlTimingCfg.u16Hstart     = pstIoTimingCfg->u16Hstart;
    stPnlTimingCfg.u16Vtt        = pstIoTimingCfg->u16Vtt;
    stPnlTimingCfg.u16Vpw        = pstIoTimingCfg->u16Vpw;
    stPnlTimingCfg.u16Vbp        = pstIoTimingCfg->u16Vbp;
    stPnlTimingCfg.u16Vactive    = pstIoTimingCfg->u16Vactive;
    stPnlTimingCfg.u16Vstart     = pstIoTimingCfg->u16Vstart;
    stPnlTimingCfg.u16Fps        = pstIoTimingCfg->u16Fps;

    if( DrvPnlSetTimingConfig(stCtxCfg.pCtx, &stPnlTimingCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoSetSscConfig(s32 s32Handler, DrvPnlIoSscConfig_t *pstIoSscParamCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlSscConfig_t stSscCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoSscConfig_t),
                                             pstIoSscParamCfg->VerChk_Size,
                                             &pstIoSscParamCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    memset(&stSscCfg, 0, sizeof(DrvPnlSscConfig_t));

    stSscCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pstIoSscParamCfg->enLinkType);
    stSscCfg.bEn        = pstIoSscParamCfg->bEn;
    stSscCfg.u16Step    = pstIoSscParamCfg->u16Step;
    stSscCfg.u16Span    = pstIoSscParamCfg->u16Span;

    if( DrvPnlSetSscConfig(stCtxCfg.pCtx, &stSscCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}


DrvPnlIoErrType_e _DrvPnlIoSetTestPatternConfig(s32 s32Handler, DrvPnlIoTestPatternConfig_t *pIoTestPatCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlTestPatternConfig_t stTestPatCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoTestPatternConfig_t),
                                             pIoTestPatCfg->VerChk_Size,
                                             &pIoTestPatCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stTestPatCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoTestPatCfg->enLinkType);
    stTestPatCfg.bEn        = pIoTestPatCfg->bEn;
    stTestPatCfg.u16R    = pIoTestPatCfg->u16R;
    stTestPatCfg.u16G    = pIoTestPatCfg->u16G;
    stTestPatCfg.u16B    = pIoTestPatCfg->u16B;

    if(DrvPnlSetTestPatternConfig(stCtxCfg.pCtx, &stTestPatCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}


DrvPnlIoErrType_e _DrvPnlIoSetTestTgenConfig(s32 s32Handler, DrvPnlIoTestTgenConfig_t *pIoTestTgenCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlTestTgenConfig_t stTestTgenCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoTestTgenConfig_t),
                                             pIoTestTgenCfg->VerChk_Size,
                                             &pIoTestTgenCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stTestTgenCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoTestTgenCfg->enLinkType);
    stTestTgenCfg.bEn        = pIoTestTgenCfg->bEn;
    stTestTgenCfg.u16Htt     = pIoTestTgenCfg->u16Htt;
    stTestTgenCfg.u16Hpw     = pIoTestTgenCfg->u16Hpw;
    stTestTgenCfg.u16Hbp     = pIoTestTgenCfg->u16Hbp;
    stTestTgenCfg.u16Hactive = pIoTestTgenCfg->u16Hactive;
    stTestTgenCfg.u16Vtt     = pIoTestTgenCfg->u16Vtt;
    stTestTgenCfg.u16Vpw     = pIoTestTgenCfg->u16Vpw;
    stTestTgenCfg.u16Vbp     = pIoTestTgenCfg->u16Vbp;
    stTestTgenCfg.u16Vactive = pIoTestTgenCfg->u16Vactive;

    if(DrvPnlSetTestTgenConfig(stCtxCfg.pCtx, &stTestTgenCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoSetSignalCtrlConfig(s32 s32Handler, DrvPnlIoSignalCtrlConfig_t *pIoSignalCtrlCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlSignalCtrlConfig_t stSignalCtrlCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoSignalCtrlConfig_t),
                                             pIoSignalCtrlCfg->VerChk_Size,
                                             &pIoSignalCtrlCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stSignalCtrlCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoSignalCtrlCfg->enLinkType);

    if(pIoSignalCtrlCfg->enLinkType == E_DRV_PNL_IO_LINK_LVDS)
    {
        _DrvPnlIoTransModCfgIo2Drv(&pIoSignalCtrlCfg->stModCfg, &stSignalCtrlCfg.stModCfg);
    }
    else if(pIoSignalCtrlCfg->enLinkType == E_DRV_PNL_IO_LINK_MIPI_DSI)
    {
        _DrvPnlIoTransMipiCfgIo2Drv(&pIoSignalCtrlCfg->stMipiDsiCfg, &stSignalCtrlCfg.stMipiDsiCfg);
    }


    if(DrvPnlSetSignalCtrlConfig(stCtxCfg.pCtx, &stSignalCtrlCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoSetPowerConfig(s32 s32Handler, DrvPnlIoPowerConfig_t *pIoPowerCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlPowerConfig_t stPowerCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoPowerConfig_t),
                                             pIoPowerCfg->VerChk_Size,
                                             &pIoPowerCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stPowerCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoPowerCfg->enLinkType);
    stPowerCfg.bEn        = pIoPowerCfg->bEn;

    if(DrvPnlSetPowerConfig(stCtxCfg.pCtx, &stPowerCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}


DrvPnlIoErrType_e _DrvPnlIoSetBackLightConfig(s32 s32Handler, DrvPnlIoBackLightConfig_t *pIoBackLightCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlBackLightConfig_t stBackLightCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoBackLightConfig_t),
                                             pIoBackLightCfg->VerChk_Size,
                                             &pIoBackLightCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stBackLightCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoBackLightCfg->enLinkType);
    stBackLightCfg.bEn        = pIoBackLightCfg->bEn;
    stBackLightCfg.u16Level   = pIoBackLightCfg->u16Level;

    if(DrvPnlSetBackLightConfig(stCtxCfg.pCtx, &stBackLightCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoGetPowerConfig(s32 s32Handler, DrvPnlIoPowerConfig_t *pIoPowerCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlPowerConfig_t stPowerCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoPowerConfig_t),
                                             pIoPowerCfg->VerChk_Size,
                                             &pIoPowerCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    if(DrvPnlGetPowerConfig(stCtxCfg.pCtx, &stPowerCfg))
    {
        pIoPowerCfg->enLinkType = _DrvPnlIoTransLinkTypeDrv2Io(stPowerCfg.enLinkType);
        pIoPowerCfg->bEn        = stPowerCfg.bEn;
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}


DrvPnlIoErrType_e _DrvPnlIoGetBackLightConfig(s32 s32Handler, DrvPnlIoBackLightConfig_t *pIoBackLightCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlBackLightConfig_t stBackLightCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoBackLightConfig_t),
                                             pIoBackLightCfg->VerChk_Size,
                                             &pIoBackLightCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }


    if(DrvPnlGetBackLightConfig(stCtxCfg.pCtx, &stBackLightCfg))
    {
        pIoBackLightCfg->enLinkType = _DrvPnlIoTransLinkTypeDrv2Io(stBackLightCfg.enLinkType);
        pIoBackLightCfg->bEn        = stBackLightCfg.bEn;
        pIoBackLightCfg->u16Level   = stBackLightCfg.u16Level;
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoSetDrvCurrentConfig(s32 s32Handler, DrvPnlIoDrvCurrentConfig_t *pIoDrvCurrentCfg)
{
    DrvPnlIoCtxConfig_t stCtxCfg;
    DrvPnlVersionChkConfig_t stVersion;
    DrvPnlDrvCurrentConfig_t stDrvCurrentCfg;

    stVersion =  _DrvPnlFillVersionChkStruct(sizeof(DrvPnlIoDrvCurrentConfig_t),
                                             pIoDrvCurrentCfg->VerChk_Size,
                                             &pIoDrvCurrentCfg->VerChk_Version);
    if(_DrvPnlVersionCheck(stVersion))
    {
        PNL_ERR( "%s %d  VerChk Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_INVAL;
    }

    if(_DrvPnlGetCtx(s32Handler, &stCtxCfg) == FALSE)
    {
        PNL_ERR( "%s %d  Get Ctx Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }

    stDrvCurrentCfg.enLinkType = _DrvPolIoTransLinkTypeIo2Drv(pIoDrvCurrentCfg->enLinkType);
    stDrvCurrentCfg.u16Val     = pIoDrvCurrentCfg->u16Val;

    if(DrvPnlSetDrvCurrentConfig(stCtxCfg.pCtx, &stDrvCurrentCfg))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

DrvPnlIoErrType_e _DrvPnlIoSetDbgLevelConfig(s32 s32Handler, DrvPnlIoDbgLevelConfig_t *pIoDbgLvCfg)
{
    if(DrvPnlSetDbgLevelConfig(NULL, pIoDbgLvCfg->u32Level))
    {
        return E_DRV_PNL_IO_ERR_OK;
    }
    else
    {
        PNL_ERR("%s %d, Set Cfg Fail\n", __FUNCTION__, __LINE__);
        return E_DRV_PNL_IO_ERR_FAULT;
    }
}

#if defined(PNL_OS_TYPE_LINUX_KERNEL)
EXPORT_SYMBOL(_DrvPnlIoInit);
EXPORT_SYMBOL(_DrvPnlIoOpen);
EXPORT_SYMBOL(_DrvPnlIoClose);
EXPORT_SYMBOL(_DrvPnlIoSetTimingConfig);
EXPORT_SYMBOL(_DrvPnlIoSetSscConfig);
EXPORT_SYMBOL(_DrvPnlIoSetTestPatternConfig);
EXPORT_SYMBOL(_DrvPnlIoSetTestTgenConfig);
EXPORT_SYMBOL(_DrvPnlIoSetSignalCtrlConfig);
EXPORT_SYMBOL(_DrvPnlIoSetPowerConfig);
EXPORT_SYMBOL(_DrvPnlIoSetBackLightConfig);
EXPORT_SYMBOL(_DrvPnlIoGetPowerConfig);
EXPORT_SYMBOL(_DrvPnlIoGetBackLightConfig);
EXPORT_SYMBOL(_DrvPnlIoSetDrvCurrentConfig);
EXPORT_SYMBOL(_DrvPnlIoSetDbgLevelConfig);
#endif

