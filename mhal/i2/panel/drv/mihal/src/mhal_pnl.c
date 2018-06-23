////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
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

#define __MHAL_PNL_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifdef PNL_OS_TYPE_LINUX_KERNEL
#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#elif defined(MSOS_TYPE_NOS)
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#include "mhal_common.h"

#include "cam_os_wrapper.h"

#include "drv_pnl_io_st.h"
#include "drv_pnl_verchk.h"
#include "drv_pnl_io_wrapper.h"
#include "drv_pnl_dbg.h"

#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define PARSING_MHAL_LINKTYPE(x)    ( x == E_MHAL_PNL_LINK_TTL               ? "TTL"              : \
                                      x == E_MHAL_PNL_LINK_LVDS              ? "LVDS"             : \
                                      x == E_MHAL_PNL_LINK_RSDS              ? "RSDS"             : \
                                      x == E_MHAL_PNL_LINK_MINILVDS          ? "MINILVDS"         : \
                                      x == E_MHAL_PNL_LINK_ANALOG_MINILVDS   ? "ANALOG_MINILVDS"  : \
                                      x == E_MHAL_PNL_LINK_DIGITAL_MINILVDS  ? "DIGITAL_MINILVDS" : \
                                      x == E_MHAL_PNL_LINK_MFC               ? "MFC"              : \
                                      x == E_MHAL_PNL_LINK_DAC_I             ? "DAC_I"            : \
                                      x == E_MHAL_PNL_LINK_DAC_P             ? "DAC_P"            : \
                                      x == E_MHAL_PNL_LINK_PDPLVDS           ? "PDPLVDS"          : \
                                      x == E_MHAL_PNL_LINK_EXT               ? "EXT"              : \
                                      x == E_MHAL_PNL_LINK_MIPI_DSI          ? "MIPI_DSI"         : \
                                                                               "UNKNOWN")


#define PARSING_MHAL_TI_BIT(x)      (x == E_MHAL_PNL_TI_10BIT_MODE ? "TI_10BIT" :\
                                     x == E_MHAL_PNL_TI_8BIT_MODE  ? "TI_8BIT" :\
                                     x == E_MHAL_PNL_TI_6BIT_MODE  ? "TI_6BIT" :\
                                                                     "UNKNOWN")

#define PARSING_MHAL_CH_SWAP(x)     (x == E_MHAL_PNL_CH_SWAP_0 ? "CH_SWAP_0" \
                                     x == E_MHAL_PNL_CH_SWAP_1 ? "CH_SWAP_1" \
                                     x == E_MHAL_PNL_CH_SWAP_2 ? "CH_SWAP_2" \
                                     x == E_MHAL_PNL_CH_SWAP_3 ? "CH_SWAP_3" \
                                     x == E_MHAL_PNL_CH_SWAP_4 ? "CH_SWAP_4" \
                                                                 "UNKNOWN")


#define PARSING_MHAL_MIPI_DSI_CTRL(x)    ( x == E_MHAL_PNL_MIPI_DSI_CMD_MODE   ? "CMD_MODE"   : \
                                           x == E_MHAL_PNL_MIPI_DSI_SYNC_PULSE ? "SYNC_PULSE" : \
                                           x == E_MHAL_PNL_MIPI_DSI_SYNC_EVENT ? "SYNC_EVENT" : \
                                           x == E_MHAL_PNL_MIPI_DSI_BURST_MODE ? "BURST_MODE" : \
                                                                                      "UNKNOWN")

#define PARSING_MHAL_MIPI_DSI_FMT(x)     ( x == E_MHAL_PNL_MIPI_DSI_RGB565          ? "RGB565" : \
                                           x == E_MHAL_PNL_MIPI_DSI_RGB666          ? "RGB666" : \
                                           x == E_MHAL_PNL_MIPI_DSI_RGB888          ? "RGB888" : \
                                           x == E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666  ? "LOOSELY_RGB666" : \
                                                                                      "UNKOWN")

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define MHAL_PNL_INSTANCE_MAX   1

//-------------------------------------------------------------------------------------------------
//  Local enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_PNL_CTX_FLAG_NONE                    = 0x00000000,
    E_MHAL_PNL_CTX_FLAG_PARAM_CFG               = 0x00000001,
    E_MHAL_PNL_CTX_FLAG_MIPI_DSI_CFG            = 0x00000002,
    E_MHAL_PNL_CTX_FLAG_SSC_CFG                 = 0x00000004,
    E_MHAL_PNL_CTX_FLAG_TIMING_CFG              = 0x00000008,
    E_MHAL_PNL_CTX_FLAG_TEST_PAT_CFG            = 0x00000010,
    E_MHAL_PNL_CTX_FLAG_POWER_CFG               = 0x00000020,
    E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_ONOFF_CFG    = 0x00000040,
    E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_LEVEL_CFG    = 0x00000080,
    E_MHAL_PNL_CTX_FLAG_DRV_CURRENT_CVG         = 0x00000100,
}MhalPnlCtxFlag_e;

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bUsed;
    s32 s32Handler;
    u32 u32Flag;
    MhalPnlLinkType_e enLinkType;
    MhalPnlParamConfig_t stParamCfg;
    MhalPnlMipiDsiConfig_t stMipiDisCfg;
    MhalPnlSscConfig_t stSscCfg;
    MhalPnlTimingConfig_t stTimingCfg;
    MhalPnlTestPatternConfig_t stTestPatCfg;
    MhalPnlPowerConfig_t stPowerCfg;
    MhalPnlBackLightOnOffConfig_t stBackLightOnOffCfg;
    MhalPnlBackLightLevelConfig_t stBackLightLevelCfg;
    MhalPnlDrvCurrentConfig_t stDrvCurrentCfg;
}MhalPnlCtxConfig_t;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
MhalPnlCtxConfig_t _gstMhalPnlCtxCfg[MHAL_PNL_INSTANCE_MAX];

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------
DrvPnlIoChannelSwapType_e _MhalPnlTransChSwapToIo(MhalPnlChannelSwapType_e enMhalChSwap)
{
    DrvPnlIoChannelSwapType_e eIoChSwap;

    eIoChSwap = enMhalChSwap == E_MHAL_PNL_CH_SWAP_0 ? E_DRV_PNL_IO_CH_SWAP_0 :
                enMhalChSwap == E_MHAL_PNL_CH_SWAP_1 ? E_DRV_PNL_IO_CH_SWAP_1 :
                enMhalChSwap == E_MHAL_PNL_CH_SWAP_2 ? E_DRV_PNL_IO_CH_SWAP_2 :
                enMhalChSwap == E_MHAL_PNL_CH_SWAP_3 ? E_DRV_PNL_IO_CH_SWAP_3 :
                                                       E_DRV_PNL_IO_CH_SWAP_4;
    return eIoChSwap;
}


DrvPnlIoLinkType_e _MhalPnlTransLinkTypeToIo(MhalPnlLinkType_e enMhalLinkType)
{
    DrvPnlIoLinkType_e enIoLinkType;

    enIoLinkType = enMhalLinkType == E_MHAL_PNL_LINK_LVDS     ? E_DRV_PNL_IO_LINK_LVDS :
                   enMhalLinkType == E_MHAL_PNL_LINK_TTL      ? E_DRV_PNL_IO_LINK_TTL :
                   enMhalLinkType == E_MHAL_PNL_LINK_MIPI_DSI ? E_DRV_PNL_IO_LINK_MIPI_DSI :
                                                                E_DRV_PNL_IO_LINK_MAX;
    return enIoLinkType;
}


MhalPnlLinkType_e _MhalPnlTransLinkTypeFromIo(DrvPnlIoLinkType_e enIoLinkType)
{
    MhalPnlLinkType_e enMhalLinkType;

    enMhalLinkType = enIoLinkType == E_DRV_PNL_IO_LINK_LVDS ?     E_MHAL_PNL_LINK_LVDS :
                     enIoLinkType == E_DRV_PNL_IO_LINK_TTL  ?     E_MHAL_PNL_LINK_TTL  :
                     enIoLinkType == E_DRV_PNL_IO_LINK_MIPI_DSI ? E_MHAL_PNL_LINK_MIPI_DSI :
                                                                  E_MHAL_PNL_LINK_LVDS;
    return enMhalLinkType;
}

DrvPnlIoMipiDsiLaneNum_e _MhalPnlTransMipiDsiLaneNumToIo(MhalPnlMipiDsiLaneMode_e enMhalLaneMode)
{
    DrvPnlIoMipiDsiLaneNum_e enIoLaneNum;

    enIoLaneNum = enMhalLaneMode == E_MHAL_PNL_MIPI_DSI_LANE_1 ? E_DRV_PNL_IO_MIPI_DSI_LANE_1 :
                  enMhalLaneMode == E_MHAL_PNL_MIPI_DSI_LANE_2 ? E_DRV_PNL_IO_MIPI_DSI_LANE_2 :
                  enMhalLaneMode == E_MHAL_PNL_MIPI_DSI_LANE_3 ? E_DRV_PNL_IO_MIPI_DSI_LANE_3 :
                  enMhalLaneMode == E_MHAL_PNL_MIPI_DSI_LANE_4 ? E_DRV_PNL_IO_MIPI_DSI_LANE_4 :
                                                                 E_DRV_PNL_IO_MIPI_DSI_LANE_NONE;
    return enIoLaneNum;
}

DrvPnlIoMipiDsiCtrlMode_e _MhalPnlTransMipiDsiCtrlModeToIo(MhalPnlMipiDsiCtrlMode_e enMhalCtrlMode)
{
    DrvPnlIoMipiDsiCtrlMode_e enIoCtrlMode;

    enIoCtrlMode = enMhalCtrlMode == E_MHAL_PNL_MIPI_DSI_CMD_MODE   ? E_DRV_PNL_IO_MIPI_DSI_CMD_MODE :
                   enMhalCtrlMode == E_MHAL_PNL_MIPI_DSI_SYNC_PULSE ? E_DRV_PNL_IO_MIPI_DSI_SYNC_PULSE :
                   enMhalCtrlMode == E_MHAL_PNL_MIPI_DSI_SYNC_EVENT ? E_DRV_PNL_IO_MIPI_DSI_SYNC_EVENT :
                                                                      E_DRV_PNL_IO_MIPI_DSI_BURST_MODE;
    return enIoCtrlMode;
}

DrvPnlIoMipiDsiFormat_e _MhalPnlTransMipiDsiFormatToIo(MhalPnlMipiDsiFormat_e enMhalFormat)
{
    DrvPnlIoMipiDsiFormat_e enIoFormat;

    enIoFormat = enMhalFormat == E_MHAL_PNL_MIPI_DSI_RGB565         ? E_DRV_PNL_IO_MIPI_DSI_RGB565 :
                 enMhalFormat == E_MHAL_PNL_MIPI_DSI_RGB666         ? E_DRV_PNL_IO_MIPI_DSI_RGB666 :
                 enMhalFormat == E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666 ? E_DRV_PNL_IO_MIPI_DSI_LOOSELY_RGB666 :
                                                                      E_DRV_PNL_IO_MIPI_DSI_RGB888;

    return enIoFormat;
}

void _MhalPnlTransMipiDsiCfgToIoSignalCtrlCfg(DrvPnlIoMipiDsiConfig_t *pIoMipiDsiCfg, MhalPnlMipiDsiConfig_t *pMhalMipiDsiCfg)
{
    pIoMipiDsiCfg->u8HsTrail   = pMhalMipiDsiCfg->u8HsTrail;
    pIoMipiDsiCfg->u8HsPrpr    = pMhalMipiDsiCfg->u8HsPrpr;
    pIoMipiDsiCfg->u8HsZero    = pMhalMipiDsiCfg->u8HsZero;
    pIoMipiDsiCfg->u8ClkHsPrpr = pMhalMipiDsiCfg->u8ClkHsPrpr;
    pIoMipiDsiCfg->u8ClkHsExit = pMhalMipiDsiCfg->u8ClkHsExit;
    pIoMipiDsiCfg->u8ClkTrail  = pMhalMipiDsiCfg->u8ClkTrail;
    pIoMipiDsiCfg->u8ClkZero   = pMhalMipiDsiCfg->u8ClkZero;
    pIoMipiDsiCfg->u8ClkHsPost = pMhalMipiDsiCfg->u8ClkHsPost;
    pIoMipiDsiCfg->u8DaHsExit  = pMhalMipiDsiCfg->u8DaHsExit;
    pIoMipiDsiCfg->u8ContDet   = pMhalMipiDsiCfg->u8ContDet;

    pIoMipiDsiCfg->u8Lpx       = pMhalMipiDsiCfg->u8Lpx;
    pIoMipiDsiCfg->u8TaGet     = pMhalMipiDsiCfg->u8TaGet;
    pIoMipiDsiCfg->u8TaSure    = pMhalMipiDsiCfg->u8TaSure;
    pIoMipiDsiCfg->u8TaGo      = pMhalMipiDsiCfg->u8TaGo;

    pIoMipiDsiCfg->u16Hactive  = pMhalMipiDsiCfg->u16Hactive;
    pIoMipiDsiCfg->u16Hpw      = pMhalMipiDsiCfg->u16Hpw;
    pIoMipiDsiCfg->u16Hbp      = pMhalMipiDsiCfg->u16Hbp;
    pIoMipiDsiCfg->u16Hfp      = pMhalMipiDsiCfg->u16Hfp;

    pIoMipiDsiCfg->u16Vactive  = pMhalMipiDsiCfg->u16Vactive;
    pIoMipiDsiCfg->u16Vpw      = pMhalMipiDsiCfg->u16Vpw;
    pIoMipiDsiCfg->u16Vbp      = pMhalMipiDsiCfg->u16Vbp;
    pIoMipiDsiCfg->u16Vfp      = pMhalMipiDsiCfg->u16Vfp;

    pIoMipiDsiCfg->u16Bllp     = pMhalMipiDsiCfg->u16Bllp;
    pIoMipiDsiCfg->u16Fps      = pMhalMipiDsiCfg->u16Fps;

    pIoMipiDsiCfg->enLaneNum = _MhalPnlTransMipiDsiLaneNumToIo(pMhalMipiDsiCfg->enLaneNum);
    pIoMipiDsiCfg->enCtrl    = _MhalPnlTransMipiDsiCtrlModeToIo(pMhalMipiDsiCfg->enCtrl);
    pIoMipiDsiCfg->enformat  = _MhalPnlTransMipiDsiFormatToIo(pMhalMipiDsiCfg->enFormat);

    pIoMipiDsiCfg->pu8CmdBuf     = pMhalMipiDsiCfg->pu8CmdBuf;
    pIoMipiDsiCfg->u32CmdBufSize = pMhalMipiDsiCfg->u32CmdBufSize;
}
//-------------------------------------------------------------------------------------------------
//  Pubic Functions
//-------------------------------------------------------------------------------------------------
bool MhalPnlCreateInstance(void **pCtx, MhalPnlLinkType_e enLinkType)
{
    static bool bFirstCreateDone = FALSE;
    u8 i;
    bool bRet;

    if(bFirstCreateDone == FALSE)
    {
        _DrvPnlIoInit();

        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, The first CreateInstance \n", __FUNCTION__, __LINE__);

        memset(&_gstMhalPnlCtxCfg, 0, sizeof(MhalPnlCtxConfig_t)*MHAL_PNL_INSTANCE_MAX);
        for(i=0; i<MHAL_PNL_INSTANCE_MAX; i++)
        {
            _gstMhalPnlCtxCfg[i].s32Handler = -1;
            _gstMhalPnlCtxCfg[i].stBackLightLevelCfg.u16Level = 50;
        }

        bFirstCreateDone = TRUE;
    }

    for(i=0; i<MHAL_PNL_INSTANCE_MAX; i++)
    {
        if(_gstMhalPnlCtxCfg[i].bUsed == 0)
        {
            break;
        }
    }

    if(i < MHAL_PNL_INSTANCE_MAX)
    {
        _gstMhalPnlCtxCfg[i].s32Handler = _DrvPnlIoOpen();

        if(_gstMhalPnlCtxCfg[i].s32Handler > 0)
        {
            _gstMhalPnlCtxCfg[i].enLinkType = enLinkType;
            *pCtx = (void *)&_gstMhalPnlCtxCfg[i];

            bRet = TRUE;
            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, Success \n", __FUNCTION__, __LINE__);
        }
        else
        {
            bRet = FALSE;
            *pCtx = NULL;
            PNL_ERR("%s %d, Can't Get Handler\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet = FALSE;
        PNL_ERR("%s %d, out of Range\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlDestroyInstance(void *pCtx)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx == NULL)
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    else
    {
        if( _DrvPnlIoClose(pMhalPnlCtx->s32Handler) == E_DRV_PNL_IO_ERR_OK)
        {
            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, Success \n", __FUNCTION__, __LINE__);

            bRet = TRUE;
            memset(pMhalPnlCtx, 0, sizeof(MhalPnlCtxConfig_t));
            pMhalPnlCtx->s32Handler = -1;
            pMhalPnlCtx->stBackLightLevelCfg.u16Level = 50;
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
        }
    }
    return bRet;
}

bool MhalPnlSetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    DrvPnlIoTimingConfig_t stIoTimingCfg;
    DrvPnlIoSignalCtrlConfig_t stIoSignalCtrlCfg;
    DrvPnlIoTestTgenConfig_t stIoTestTgenCfg;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->enLinkType == pParamCfg->eLinkType)
        {
            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, Link(%d %s), Ti:%d, Bits=%s, ChPol=%d, Invert(%d %d %d), bChSwap=%d, CH(%d %d %d %d %d)\n",
                __FUNCTION__, __LINE__,
                pParamCfg->eLinkType, PARSING_MHAL_LINKTYPE(pParamCfg->eLinkType),
                pParamCfg->u8LVDS_TI_MODE, PARSING_MHAL_TI_BIT(pParamCfg->eTiBitMode),
                pParamCfg->u8SwapLVDS_POL, pParamCfg->u8InvHSync, pParamCfg->u8InvVSync,
                pParamCfg->u8SwapLVDS_CH,
                pParamCfg->u8InvDE, pParamCfg->eCh0, pParamCfg->eCh1,
                pParamCfg->eCh2, pParamCfg->eCh3, pParamCfg->eCh4);

            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, H(%d %d %d %d %d) V:(%d %d %d %d %d) DCLK(%d)\n",
                __FUNCTION__, __LINE__,
                pParamCfg->u16HTotal, pParamCfg->u16HSyncWidth, pParamCfg->u16HSyncBackPorch,
                pParamCfg->u16HStart, pParamCfg->u16Width,
                pParamCfg->u16VTotal, pParamCfg->u16VSyncWidth, pParamCfg->u16VSyncBackPorch,
                pParamCfg->u16VStart, pParamCfg->u16Height,
                pParamCfg->u16DCLK);

            pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_PARAM_CFG;
            memcpy(&pMhalPnlCtx->stParamCfg, pParamCfg, sizeof(MhalPnlParamConfig_t));

            FILL_VERCHK_TYPE(stIoTimingCfg, stIoTimingCfg.VerChk_Version, stIoTimingCfg.VerChk_Size, DRV_PNL_VERSION);
            stIoTimingCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pParamCfg->eLinkType);
            stIoTimingCfg.u16Htt        = pParamCfg->u16HTotal;
            stIoTimingCfg.u16Hpw        = pParamCfg->u16HSyncWidth;
            stIoTimingCfg.u16Hbp        = pParamCfg->u16HSyncBackPorch;
            stIoTimingCfg.u16Hactive    = pParamCfg->u16Width;
            stIoTimingCfg.u16Hstart     = pParamCfg->u16HStart;
            stIoTimingCfg.u16Vtt        = pParamCfg->u16VTotal;
            stIoTimingCfg.u16Vpw        = pParamCfg->u16VSyncWidth;
            stIoTimingCfg.u16Vbp        = pParamCfg->u16VSyncBackPorch;
            stIoTimingCfg.u16Vactive    = pParamCfg->u16Height;
            stIoTimingCfg.u16Vstart     = pParamCfg->u16VStart;
            stIoTimingCfg.u16Fps        = (u32)(pParamCfg->u16DCLK * 1000000) / ((u32)pParamCfg->u16HTotal * (u32)pParamCfg->u16VTotal);


            FILL_VERCHK_TYPE(stIoSignalCtrlCfg, stIoSignalCtrlCfg.VerChk_Version, stIoSignalCtrlCfg.VerChk_Size, DRV_PNL_VERSION);
            stIoSignalCtrlCfg.enLinkType           = _MhalPnlTransLinkTypeToIo(pParamCfg->eLinkType);
            stIoSignalCtrlCfg.stModCfg.bTiMode     = pParamCfg->u8LVDS_TI_MODE ? TRUE : FALSE;
            stIoSignalCtrlCfg.stModCfg.enTiBitMode =
                    pParamCfg->eTiBitMode == E_MHAL_PNL_TI_8BIT_MODE ? E_DRV_PNL_IO_TI_BIT_8 :
                    pParamCfg->eTiBitMode == E_MHAL_PNL_TI_6BIT_MODE ? E_DRV_PNL_IO_TI_BIT_6 :
                                                                       E_DRV_PNL_IO_TI_BIT_10;

            stIoSignalCtrlCfg.stModCfg.enCh[0] = pParamCfg->u8SwapLVDS_CH ? _MhalPnlTransChSwapToIo(pParamCfg->eCh0) : E_DRV_PNL_IO_CH_SWAP_0;
            stIoSignalCtrlCfg.stModCfg.enCh[1] = pParamCfg->u8SwapLVDS_CH ? _MhalPnlTransChSwapToIo(pParamCfg->eCh1) : E_DRV_PNL_IO_CH_SWAP_1;
            stIoSignalCtrlCfg.stModCfg.enCh[2] = pParamCfg->u8SwapLVDS_CH ? _MhalPnlTransChSwapToIo(pParamCfg->eCh2) : E_DRV_PNL_IO_CH_SWAP_2;
            stIoSignalCtrlCfg.stModCfg.enCh[3] = pParamCfg->u8SwapLVDS_CH ? _MhalPnlTransChSwapToIo(pParamCfg->eCh3) : E_DRV_PNL_IO_CH_SWAP_3;
            stIoSignalCtrlCfg.stModCfg.enCh[4] = pParamCfg->u8SwapLVDS_CH ? _MhalPnlTransChSwapToIo(pParamCfg->eCh4) : E_DRV_PNL_IO_CH_SWAP_4;

            stIoSignalCtrlCfg.stModCfg.bChPolarity  = pParamCfg->u8SwapLVDS_POL ? TRUE : FALSE;
            stIoSignalCtrlCfg.stModCfg.bHsyncInvert = pParamCfg->u8InvHSync ? TRUE : FALSE;
            stIoSignalCtrlCfg.stModCfg.bVsyncInvert = pParamCfg->u8InvVSync ? TRUE : FALSE;
            stIoSignalCtrlCfg.stModCfg.bDeInvert    = pParamCfg->u8InvDE ? TRUE : FALSE;

            FILL_VERCHK_TYPE(stIoTestTgenCfg, stIoTestTgenCfg.VerChk_Version, stIoTestTgenCfg.VerChk_Size, DRV_PNL_VERSION);
            stIoTestTgenCfg.enLinkType = _MhalPnlTransLinkTypeToIo(pParamCfg->eLinkType);
            stIoTestTgenCfg.bEn        = 0;
            stIoTestTgenCfg.u16Htt     = pParamCfg->u16HTotal;
            stIoTestTgenCfg.u16Hpw     = pParamCfg->u16HSyncWidth;
            stIoTestTgenCfg.u16Hbp     = pParamCfg->u16HSyncBackPorch;
            stIoTestTgenCfg.u16Hactive = pParamCfg->u16Width;
            stIoTestTgenCfg.u16Vtt     = pParamCfg->u16VTotal;
            stIoTestTgenCfg.u16Vpw     = pParamCfg->u16VSyncWidth;
            stIoTestTgenCfg.u16Vbp     = pParamCfg->u16VSyncBackPorch;
            stIoTestTgenCfg.u16Vactive = pParamCfg->u16Height;

            if(_DrvPnlIoSetTimingConfig(pMhalPnlCtx->s32Handler, &stIoTimingCfg) == E_DRV_PNL_IO_ERR_OK)
            {
                if(pParamCfg->eLinkType != E_MHAL_PNL_LINK_MIPI_DSI)
                {
                    if(_DrvPnlIoSetSignalCtrlConfig(pMhalPnlCtx->s32Handler, &stIoSignalCtrlCfg) == E_DRV_PNL_IO_ERR_OK)
                    {
                        if(_DrvPnlIoSetTestTgenConfig(pMhalPnlCtx->s32Handler, &stIoTestTgenCfg) != E_DRV_PNL_IO_ERR_OK)
                        {
                            bRet = FALSE;
                            PNL_ERR("%s %d, SetTestTgenConfig Fail\n", __FUNCTION__, __LINE__);
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                        PNL_ERR("%s %d, SetSignalCtrlConfig Fail\n", __FUNCTION__, __LINE__);
                    }
                }
            }
            else
            {
                bRet = FALSE;
                PNL_ERR("%s %d, SetTimingConfig Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, ParamLink(%d %s) Not Match InstanceLink(%d %s)\n",
                __FUNCTION__, __LINE__,
                pParamCfg->eLinkType, PARSING_MHAL_LINKTYPE(pParamCfg->eLinkType),
                pMhalPnlCtx->enLinkType, PARSING_MHAL_LINKTYPE(pMhalPnlCtx->enLinkType));
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlGetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->u32Flag & E_MHAL_PNL_CTX_FLAG_PARAM_CFG)
        {
            memcpy(pParamCfg, &pMhalPnlCtx->stParamCfg, sizeof(MhalPnlParamConfig_t));
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Not Yet To Call SetParamConfig !!\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pMipiDsiCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoSignalCtrlConfig_t stIoSignalCtrlCfg;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->enLinkType == E_MHAL_PNL_LINK_MIPI_DSI)
        {
            PNL_DBG(PNL_DBG_LEVEL_MHAL,
                "%s %d, H(%d %d %d %d) V(%d %d %d %d), Lane=%d, Ctrl=%s, Fmt=%s\n",
                __FUNCTION__, __LINE__,
                pMipiDsiCfg->u16Hactive, pMipiDsiCfg->u16Hpw, pMipiDsiCfg->u16Hbp, pMipiDsiCfg->u16Hfp,
                pMipiDsiCfg->u16Vactive, pMipiDsiCfg->u16Vpw, pMipiDsiCfg->u16Vbp, pMipiDsiCfg->u16Vfp,
                pMipiDsiCfg->enLaneNum,
                PARSING_MHAL_MIPI_DSI_CTRL(pMipiDsiCfg->enCtrl),
                PARSING_MHAL_MIPI_DSI_FMT(pMipiDsiCfg->enFormat));

            pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_MIPI_DSI_CFG;
            memcpy(&pMhalPnlCtx->stMipiDisCfg, pMipiDsiCfg, sizeof(MhalPnlMipiDsiConfig_t));

            FILL_VERCHK_TYPE(stIoSignalCtrlCfg, stIoSignalCtrlCfg.VerChk_Version, stIoSignalCtrlCfg.VerChk_Size, DRV_PNL_VERSION);

            stIoSignalCtrlCfg.enLinkType = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);

            _MhalPnlTransMipiDsiCfgToIoSignalCtrlCfg(&stIoSignalCtrlCfg.stMipiDsiCfg, pMipiDsiCfg);

            if( _DrvPnlIoSetSignalCtrlConfig(pMhalPnlCtx->s32Handler, &stIoSignalCtrlCfg)!= E_DRV_PNL_IO_ERR_OK)
            {
                bRet = FALSE;
                PNL_ERR("%s %d, SetSignalCtrlConfig Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, LinkType(%d, %s) not correct\n",
                __FUNCTION__, __LINE__,
                pMhalPnlCtx->enLinkType,
                PARSING_MHAL_LINKTYPE(pMhalPnlCtx->enLinkType));
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetSscConfig(void *pCtx, MhalPnlSscConfig_t *pSscCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoSscConfig_t stIoSscCfg;

    if(pMhalPnlCtx)
    {
        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, En=%d, Step=%04x, Span=%04x\n",
            __FUNCTION__, __LINE__, pSscCfg->bEn, pSscCfg->u16Step, pSscCfg->u16Span);

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_SSC_CFG;
        memcpy(&pMhalPnlCtx->stSscCfg, pSscCfg, sizeof(MhalPnlParamConfig_t));

        FILL_VERCHK_TYPE(stIoSscCfg, stIoSscCfg.VerChk_Version, stIoSscCfg.VerChk_Size, DRV_PNL_VERSION);

        stIoSscCfg.enLinkType = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
        stIoSscCfg.bEn        = pSscCfg->bEn;
        stIoSscCfg.u16Step    = pSscCfg->u16Step;
        stIoSscCfg.u16Span    = pSscCfg->u16Span;
        if(_DrvPnlIoSetSscConfig(pMhalPnlCtx->s32Handler, &stIoSscCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetSscConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoTimingConfig_t stIoTimingCfg;
    DrvPnlIoSignalCtrlConfig_t stIoSignalCtrlCfg;

    if(pMhalPnlCtx)
    {

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_TIMING_CFG;
        memcpy(&pMhalPnlCtx->stTimingCfg, pTimingCfg, sizeof(MhalPnlTimingConfig_t));

        if(pMhalPnlCtx->enLinkType == E_MHAL_PNL_LINK_MIPI_DSI)
        {
            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, H(%d %d %d %d) V:(%d %d %d %d)\n",
                __FUNCTION__, __LINE__,
                pTimingCfg->u16HActive, pTimingCfg->u16HSyncWidth,
                pTimingCfg->u16HSyncBackPorch, pTimingCfg->u16HSyncFrontPorch,
                pTimingCfg->u16VActive, pTimingCfg->u16VSyncWidth,
                pTimingCfg->u16VSyncBackPorch, pTimingCfg->u16VSyncFrontPorch);

            FILL_VERCHK_TYPE(stIoSignalCtrlCfg, stIoSignalCtrlCfg.VerChk_Version, stIoSignalCtrlCfg.VerChk_Size, DRV_PNL_VERSION);

            stIoSignalCtrlCfg.enLinkType = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);

            _MhalPnlTransMipiDsiCfgToIoSignalCtrlCfg(&stIoSignalCtrlCfg.stMipiDsiCfg, &pMhalPnlCtx->stMipiDisCfg);

            stIoSignalCtrlCfg.stMipiDsiCfg.u16Hactive = pTimingCfg->u16HActive;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Hpw     = pTimingCfg->u16HSyncWidth;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Hbp     = pTimingCfg->u16HSyncBackPorch;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Hfp     = pTimingCfg->u16HSyncFrontPorch;

            stIoSignalCtrlCfg.stMipiDsiCfg.u16Vactive = pTimingCfg->u16VActive;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Vpw     = pTimingCfg->u16VSyncWidth;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Vbp     = pTimingCfg->u16VSyncBackPorch;
            stIoSignalCtrlCfg.stMipiDsiCfg.u16Vfp     = pTimingCfg->u16VSyncFrontPorch;

            stIoSignalCtrlCfg.stMipiDsiCfg.pu8CmdBuf     = NULL;
            stIoSignalCtrlCfg.stMipiDsiCfg.u32CmdBufSize = 0;

            if(_DrvPnlIoSetSignalCtrlConfig(pMhalPnlCtx->s32Handler, &stIoSignalCtrlCfg)!= E_DRV_PNL_IO_ERR_OK)
            {
                bRet = FALSE;
                PNL_ERR("%s %d, SetSignalCtrlConfig Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, H(%d %d %d %d %d) V:(%d %d %d %d %d) DCLK(%d)\n",
                __FUNCTION__, __LINE__,
                pTimingCfg->u16HTotal, pTimingCfg->u16HSyncWidth, pTimingCfg->u16HSyncBackPorch,
                pTimingCfg->u16HStart, pTimingCfg->u16HActive,
                pTimingCfg->u16VTotal, pTimingCfg->u16VSyncWidth, pTimingCfg->u16VSyncBackPorch,
                pTimingCfg->u16VStart, pTimingCfg->u16VActive,
                pTimingCfg->u16Dclk);

            FILL_VERCHK_TYPE(stIoTimingCfg, stIoTimingCfg.VerChk_Version, stIoTimingCfg.VerChk_Size, DRV_PNL_VERSION);
            stIoTimingCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
            stIoTimingCfg.u16Htt        = pTimingCfg->u16HTotal;
            stIoTimingCfg.u16Hpw        = pTimingCfg->u16HSyncWidth;
            stIoTimingCfg.u16Hbp        = pTimingCfg->u16HSyncBackPorch;
            stIoTimingCfg.u16Hactive    = pTimingCfg->u16HActive;
            stIoTimingCfg.u16Hstart     = pTimingCfg->u16HStart;
            stIoTimingCfg.u16Vtt        = pTimingCfg->u16VTotal;
            stIoTimingCfg.u16Vpw        = pTimingCfg->u16VSyncWidth;
            stIoTimingCfg.u16Vbp        = pTimingCfg->u16VSyncBackPorch;
            stIoTimingCfg.u16Vactive    = pTimingCfg->u16VActive;
            stIoTimingCfg.u16Vstart     = pTimingCfg->u16VStart;
            stIoTimingCfg.u16Fps        = (u32)(pTimingCfg->u16Dclk * 1000000) / ((u32)pTimingCfg->u16HTotal * (u32)pTimingCfg->u16VTotal);

            if(_DrvPnlIoSetTimingConfig(pMhalPnlCtx->s32Handler, &stIoTimingCfg) != E_DRV_PNL_IO_ERR_OK)
            {
                bRet = FALSE;
                PNL_ERR("%s %d, SetTimingConfig Fail\n", __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlGetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->u32Flag & E_MHAL_PNL_CTX_FLAG_TIMING_CFG)
        {
            memcpy(pTimingCfg, &pMhalPnlCtx->stTimingCfg, sizeof(MhalPnlTimingConfig_t));
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Not Yet To Call SetTimingConfig\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoPowerConfig_t stIoPowerCfg;

    if(pMhalPnlCtx)
    {
        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, En=%d\n",
            __FUNCTION__, __LINE__, pPowerCfg->bEn);

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_POWER_CFG;
        memcpy(&pMhalPnlCtx->stPowerCfg, pPowerCfg, sizeof(MhalPnlPowerConfig_t));


        FILL_VERCHK_TYPE(stIoPowerCfg, stIoPowerCfg.VerChk_Version, stIoPowerCfg.VerChk_Size, DRV_PNL_VERSION);
        stIoPowerCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
        stIoPowerCfg.bEn           = pPowerCfg->bEn;

        if(_DrvPnlIoSetPowerConfig(pMhalPnlCtx->s32Handler, &stIoPowerCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetPowerConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlGetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->u32Flag & E_MHAL_PNL_CTX_FLAG_POWER_CFG)
        {
            memcpy(pPowerCfg, &pMhalPnlCtx->stPowerCfg, sizeof(MhalPnlPowerConfig_t));
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Not Yet Call SetPowerConfig\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightOnOffCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoBackLightConfig_t stIoBackLightCfg;

    if(pMhalPnlCtx)
    {
        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, bEn=%d\n",
            __FUNCTION__, __LINE__, pBackLightOnOffCfg->bEn);

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_ONOFF_CFG;
        memcpy(&pMhalPnlCtx->stBackLightOnOffCfg, pBackLightOnOffCfg, sizeof(MhalPnlBackLightOnOffConfig_t));

        FILL_VERCHK_TYPE(stIoBackLightCfg, stIoBackLightCfg.VerChk_Version, stIoBackLightCfg.VerChk_Size, DRV_PNL_VERSION);
        stIoBackLightCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
        stIoBackLightCfg.bEn           = pBackLightOnOffCfg->bEn;
        stIoBackLightCfg.u16Level      = pMhalPnlCtx->stBackLightLevelCfg.u16Level;

        if(_DrvPnlIoSetBackLightConfig(pMhalPnlCtx->s32Handler, &stIoBackLightCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetBackLightConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlGetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightOnOffCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->u32Flag & E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_ONOFF_CFG)
        {
            memcpy(pBackLightOnOffCfg, &pMhalPnlCtx->stBackLightOnOffCfg, sizeof(MhalPnlBackLightOnOffConfig_t));
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Not Yet Call SetBackLightOnOffConfig\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightLevelCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoBackLightConfig_t stIoBackLightCfg;

    if(pMhalPnlCtx)
    {

        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, Level=%04x\n",
            __FUNCTION__, __LINE__, pBackLightLevelCfg->u16Level);

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_LEVEL_CFG;
        memcpy(&pMhalPnlCtx->stBackLightLevelCfg, pBackLightLevelCfg, sizeof(MhalPnlBackLightOnOffConfig_t));

        FILL_VERCHK_TYPE(stIoBackLightCfg, stIoBackLightCfg.VerChk_Version, stIoBackLightCfg.VerChk_Size, DRV_PNL_VERSION);
        stIoBackLightCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
        stIoBackLightCfg.bEn           = pMhalPnlCtx->stBackLightOnOffCfg.bEn;
        stIoBackLightCfg.u16Level      = pBackLightLevelCfg->u16Level;

        if(_DrvPnlIoSetBackLightConfig(pMhalPnlCtx->s32Handler, &stIoBackLightCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetBackLightConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlGetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightLevelCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;

    if(pMhalPnlCtx)
    {
        if(pMhalPnlCtx->u32Flag & E_MHAL_PNL_CTX_FLAG_BACK_LIGHT_LEVEL_CFG)
        {
            memcpy(pBackLightLevelCfg, &pMhalPnlCtx->stBackLightLevelCfg, sizeof(MhalPnlBackLightLevelConfig_t));
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, Not Yet Call SetBackLightLevelConfig\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetDrvCurrentConfig(void *pCtx, MhalPnlDrvCurrentConfig_t *pDrvCurrentCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoDrvCurrentConfig_t stIoDrvCurrentCfg;

    if(pMhalPnlCtx)
    {
        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, Val=%04x\n",
            __FUNCTION__, __LINE__, pDrvCurrentCfg->u16Val);

        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_DRV_CURRENT_CVG;
        memcpy(&pMhalPnlCtx->stDrvCurrentCfg, pDrvCurrentCfg, sizeof(MhalPnlDrvCurrentConfig_t));


        FILL_VERCHK_TYPE(stIoDrvCurrentCfg, stIoDrvCurrentCfg.VerChk_Version, stIoDrvCurrentCfg.VerChk_Size, DRV_PNL_VERSION);
        stIoDrvCurrentCfg.enLinkType    = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);
        stIoDrvCurrentCfg.u16Val        = pDrvCurrentCfg->u16Val;

        if(_DrvPnlIoSetDrvCurrentConfig(pMhalPnlCtx->s32Handler, &stIoDrvCurrentCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetDrvCurrentConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool MhalPnlSetTestPatternConfig(void *pCtx, MhalPnlTestPatternConfig_t *pTestPatternCfg)
{
    MhalPnlCtxConfig_t *pMhalPnlCtx = (MhalPnlCtxConfig_t *)pCtx;
    bool bRet = TRUE;
    DrvPnlIoTestPatternConfig_t stIoTestPatCfg;

    if(pMhalPnlCtx)
    {
        PNL_DBG(PNL_DBG_LEVEL_MHAL, "%s %d, En=%d (%04x, %04x, %04x\n",
            __FUNCTION__, __LINE__, pTestPatternCfg->bEn,
            pTestPatternCfg->u16R, pTestPatternCfg->u16G, pTestPatternCfg->u16B);


        pMhalPnlCtx->u32Flag |= E_MHAL_PNL_CTX_FLAG_TEST_PAT_CFG;
        memcpy(&pMhalPnlCtx->stTestPatCfg, pTestPatternCfg, sizeof(MhalPnlTestPatternConfig_t));

        FILL_VERCHK_TYPE(stIoTestPatCfg, stIoTestPatCfg.VerChk_Version, stIoTestPatCfg.VerChk_Size, DRV_PNL_VERSION);

        stIoTestPatCfg.enLinkType = _MhalPnlTransLinkTypeToIo(pMhalPnlCtx->enLinkType);

        stIoTestPatCfg.bEn  = pTestPatternCfg->bEn;
        stIoTestPatCfg.u16R = pTestPatternCfg->u16R;
        stIoTestPatCfg.u16G = pTestPatternCfg->u16G;
        stIoTestPatCfg.u16B = pTestPatternCfg->u16B;

        if(_DrvPnlIoSetTestPatternConfig(pMhalPnlCtx->s32Handler, &stIoTestPatCfg) != E_DRV_PNL_IO_ERR_OK)
        {
            bRet = FALSE;
            PNL_ERR("%s %d, SetTestTgenConfig Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Ctx is Empty\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

#if defined(PNL_OS_TYPE_LINUX_KERNEL)
EXPORT_SYMBOL(MhalPnlCreateInstance);
EXPORT_SYMBOL(MhalPnlDestroyInstance);
EXPORT_SYMBOL(MhalPnlSetParamConfig);
EXPORT_SYMBOL(MhalPnlGetParamConfig);
EXPORT_SYMBOL(MhalPnlSetMipiDsiConfig);
EXPORT_SYMBOL(MhalPnlSetSscConfig);
EXPORT_SYMBOL(MhalPnlSetTimingConfig);
EXPORT_SYMBOL(MhalPnlGetTimingConfig);
EXPORT_SYMBOL(MhalPnlSetPowerConfig);
EXPORT_SYMBOL(MhalPnlGetPowerConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlSetDrvCurrentConfig);
EXPORT_SYMBOL(MhalPnlSetTestPatternConfig);
#endif



