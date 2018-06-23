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
#define _MDRV_HVSP_C
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"
#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_st.h"
#include "drv_scl_hvsp.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PARSING_PAT_TGEN_TIMING(x)  (x == E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1920_1080_  ?  "E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1920_1080_" : \
                                     x == E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1024_768_6   ?  "E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1024_768_6" : \
                                     x == E_MDRV_SCLHVSP_PAT_TGEN_TIMING_640_480_60    ?  "E_MDRV_SCLHVSP_PAT_TGEN_TIMING_640_480_60" : \
                                     x == E_MDRV_SCLHVSP_PAT_TGEN_TIMING_UNDEFINED     ?  "E_MDRV_SCLHVSP_PAT_TGEN_TIMING_UNDEFINED" : \
                                                                                        "UNKNOWN")
#define IS_WRONG_TYPE(enHVSP_ID,enSrcType)((enHVSP_ID == E_MDRV_SCLHVSP_ID_1 && (enSrcType == E_MDRV_SCLHVSP_SRC_DRAM_LDC)) ||\
                        (enHVSP_ID == E_MDRV_SCLHVSP_ID_2 && enSrcType != E_MDRV_SCLHVSP_SRC_HVSP) )
#define Is_PTGEN_FHD(u16Htotal,u16Vtotal,u16Vfrequency) ((u16Htotal) == 2200 && (u16Vtotal) == 1125 && (u16Vfrequency) == 30)
#define Is_PTGEN_HD(u16Htotal,u16Vtotal,u16Vfrequency) ((u16Htotal) == 1344 && (u16Vtotal) == 806 && (u16Vfrequency) == 60)
#define Is_PTGEN_SD(u16Htotal,u16Vtotal,u16Vfrequency) ((u16Htotal) == 800 && (u16Vtotal) == 525 && (u16Vfrequency) == 60)
#define IS_HVSPNotOpen(u16Src_Width,u16Dsp_Width) (u16Src_Width == 0 && u16Dsp_Width == 0)
#define IS_NotScalingAfterCrop(bEn,u16Dsp_Height,u16Height,u16Dsp_Width,u16Width) (bEn && \
                        (u16Dsp_Height == u16Height) && (u16Dsp_Width== u16Width))
#define RATIO_CONFIG 512
#define CAL_HVSP_RATIO(input,output) ((u32)((u64)((u32)input * RATIO_CONFIG )/(u32)output))
#define SCALE(numerator, denominator,value) ((u16)((u32)(value * RATIO_CONFIG *  numerator) / (denominator * RATIO_CONFIG)))

#define MDrvSclHvspMutexLock()            //DrvSclOsObtainMutex(_MHVSP_Mutex,SCLOS_WAIT_FOREVER)
#define MDrvSclHvspMutexUnlock()          //DrvSclOsReleaseMutex(_MHVSP_Mutex)
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MDrvSclCtxMhvspGlobalSet_t *gstMhvspGlobalSet;
//keep
DrvSclHvspPatTgenConfig_t gstPatTgenCfg[E_MDRV_SCLHVSP_PAT_TGEN_TIMING_MAX] =
{
    {1125,  4,  5, 36, 1080, 2200, 88,  44, 148, 1920}, // 1920_1080_30
    { 806,  3,  6, 29,  768, 1344, 24, 136, 160, 1024}, // 1024_768_60
    { 525, 33,  2, 10,  480,  800, 16,  96,  48,  640}, // 640_480_60
    {   0, 20, 10, 20,    0,    0, 30,  15,  30,    0}, // undefined
};
s32 _MHVSP_Mutex = -1;
u8 gu8LevelInst;
//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

void _MDrvSclHvspSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstMhvspGlobalSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stMhvspCfg);
}
void _MDrvSclHvspSwInit(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    _MDrvSclHvspSetGlobal(pvCtx);
    MDrvSclHvspMutexLock();
    DrvSclOsMemset(&gstMhvspGlobalSet->gstHvspScalingCfg, 0, sizeof(MDrvSclHvspScalingConfig_t));
    DrvSclOsMemset(&gstMhvspGlobalSet->gstHvspPostCropCfg, 0, sizeof(MDrvSclHvspPostCropConfig_t));
    gu8LevelInst = 0xFF;
    MDrvSclHvspMutexUnlock();
}

MDrvSclHvspPatTgenTimingType_e _MDrvSclHvspGetPatTgenTiming(MDrvSclHvspTimingConfig_t *pTiming)
{
    MDrvSclHvspPatTgenTimingType_e enTiming;

    if(Is_PTGEN_FHD(pTiming->u16Htotal,pTiming->u16Vtotal,pTiming->u16Vfrequency))
    {
        enTiming = E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1920_1080_;
    }
    else if(Is_PTGEN_HD(pTiming->u16Htotal,pTiming->u16Vtotal,pTiming->u16Vfrequency))
    {
        enTiming = E_MDRV_SCLHVSP_PAT_TGEN_TIMING_1024_768_6;
    }
    else if(Is_PTGEN_SD(pTiming->u16Htotal,pTiming->u16Vtotal,pTiming->u16Vfrequency))
    {
        enTiming = E_MDRV_SCLHVSP_PAT_TGEN_TIMING_640_480_60;
    }
    else
    {
        enTiming = E_MDRV_SCLHVSP_PAT_TGEN_TIMING_UNDEFINED;
    }

    SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "[HVSP]%s(%d) Timing:%s(%d)", __FUNCTION__, __LINE__, PARSING_PAT_TGEN_TIMING(enTiming), enTiming);
    return enTiming;
}

bool _MDrvSclHvspIsInputSrcPatternGen(DrvSclHvspIpMuxType_e enIPMux, MDrvSclHvspInputConfig_t *pCfg)
{
    SCL_DBG(SCL_DBG_LV_DRVHVSP()&(Get_DBGMG_HVSP(0)), "[DRVHVSP]%s(%d)\n", __FUNCTION__,  __LINE__);
    if(enIPMux == E_DRV_SCLHVSP_IP_MUX_PAT_TGEN)
    {
        MDrvSclHvspPatTgenTimingType_e enPatTgenTiming;
        DrvSclHvspPatTgenConfig_t stPatTgenCfg;

        enPatTgenTiming = _MDrvSclHvspGetPatTgenTiming(&pCfg->stTimingCfg);
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(E_MDRV_SCLHVSP_ID_1)),
            "%s:%d type:%d\n", __FUNCTION__,E_MDRV_SCLHVSP_ID_1,enPatTgenTiming);
        DrvSclOsMemcpy(&stPatTgenCfg, &gstPatTgenCfg[enPatTgenTiming],sizeof(DrvSclHvspPatTgenConfig_t));
        if(enPatTgenTiming == E_MDRV_SCLHVSP_PAT_TGEN_TIMING_UNDEFINED)
        {
            stPatTgenCfg.u16HActive = pCfg->stCaptureWin.u16Width;
            stPatTgenCfg.u16VActive = pCfg->stCaptureWin.u16Height;
            stPatTgenCfg.u16Htt = stPatTgenCfg.u16HActive +
                                  stPatTgenCfg.u16HBackPorch +
                                  stPatTgenCfg.u16HFrontPorch +
                                  stPatTgenCfg.u16HSyncWidth;

            stPatTgenCfg.u16Vtt = stPatTgenCfg.u16VActive +
                                  stPatTgenCfg.u16VBackPorch +
                                  stPatTgenCfg.u16VFrontPorch +
                                  stPatTgenCfg.u16VSyncWidth;

        }
        return (bool)DrvSclHvspSetPatTgen(TRUE, &stPatTgenCfg);
    }
    else
    {
        return 0;
    }
}
static void  _MDrvSclHvspSetRegisterForce(u32 u32Size, u8 *pBuf,void *pvCtx)
{
    u32 i;
    u32 u32Reg;
    u16 u16Bank;
    u8  u8Addr, u8Val, u8Msk;

    // bank,  addrr,  val,  msk
    for(i=0; i<u32Size; i+=5)
    {
        u16Bank = (u16)pBuf[i+0] | ((u16)pBuf[i+1])<<8;
        u8Addr  = pBuf[i+2];
        u8Val   = pBuf[i+3];
        u8Msk   = pBuf[i+4];
        u32Reg  = (((u32)u16Bank) << 8) | (u32)u8Addr;

        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "%08lx, %02x, %02x\n", u32Reg, u8Val, u8Msk);
        DrvSclHvspSetRegisterForce(u32Reg, u8Val, u8Msk,pvCtx);
    }
}
static void  _MDrvSclHvspSetRegisterForceByInst(u32 u32Size, u8 *pBuf,void *pvCtx)
{
    u32 i;
    u32 u32Reg;
    u16 u16Bank,inst;
    u8  u8Addr, u8Val, u8Msk;
    s32 s32Handler;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    inst = *(u16 *)pvCtx;
    pCtxCfg = MDrvSclCtxGetCtx(inst);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if(inst<MDRV_SCL_CTX_INSTANT_MAX && ((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX))
            {
                break;
            }
            else if(inst>=MDRV_SCL_CTX_INSTANT_MAX&& ((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX))
            {
                break;
            }
        }
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        // bank,  addrr,  val,  msk
        for(i=0; i<u32Size; i+=5)
        {
            u16Bank = (u16)pBuf[i+0] | ((u16)pBuf[i+1])<<8;
            u8Addr  = pBuf[i+2];
            u8Val   = pBuf[i+3];
            u8Msk   = pBuf[i+4];
            u32Reg  = (((u32)u16Bank) << 8) | (u32)u8Addr;
            SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "%08lx, %02x, %02x\n", u32Reg, u8Val, u8Msk);
            DrvSclHvspSetRegisterForceByInst(u32Reg, u8Val, u8Msk,&pCtxCfg->stCtx);
        }
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
    }
    else
    {
        SCL_ERR("[Test]%s Inst Err\n",__FUNCTION__);
    }
}
static void  _MDrvSclHvspSetRegisterForceByAllInst(u32 u32Size, u8 *pBuf,void *pvCtx)
{
    u32 i,j;
    u32 u32Reg;
    u16 u16Bank;
    u8  u8Addr, u8Val, u8Msk;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    s32 s32Handler;
    // bank,  addrr,  val,  msk
    for(j=0; j<MDRV_SCL_CTX_INSTANT_MAX*E_MDRV_SCL_CTX_ID_NUM; j++)
    {
        pCtxCfg = MDrvSclCtxGetCtx((u16)j);
        if(pCtxCfg->bUsed)
        {
            for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
            {
                s32Handler = pCtxCfg->s32Id[i];
                if(j<MDRV_SCL_CTX_INSTANT_MAX && ((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX))
                {
                    break;
                }
                else if(j>=MDRV_SCL_CTX_INSTANT_MAX&& ((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX))
                {
                    break;
                }
            }
            MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
            for(i=0; i<u32Size; i+=5)
            {
                u16Bank = (u16)pBuf[i+0] | ((u16)pBuf[i+1])<<8;
                u8Addr  = pBuf[i+2];
                u8Val   = pBuf[i+3];
                u8Msk   = pBuf[i+4];
                u32Reg  = (((u32)u16Bank) << 8) | (u32)u8Addr;

                SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "%08lx, %02x, %02x\n", u32Reg, u8Val, u8Msk);
                DrvSclHvspSetRegisterForceByInst(u32Reg, u8Val, u8Msk,&pCtxCfg->stCtx);
            }
            MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        }
    }
}
//---------------------------------------------------------------------------------------------------------
// IOCTL function
//---------------------------------------------------------------------------------------------------------
void MDrvSclHvspRelease(MDrvSclHvspIdType_e enHVSP_ID,void *pvCtx)
{
    DrvSclHvspIdType_e enID;
    enID = enHVSP_ID == E_MDRV_SCLHVSP_ID_2 ? E_DRV_SCLHVSP_ID_2 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_3 ? E_DRV_SCLHVSP_ID_3 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_3 ? E_DRV_SCLHVSP_ID_4 :
                                           E_DRV_SCLHVSP_ID_1;
    _MDrvSclHvspSwInit((MDrvSclCtxCmdqConfig_t*)pvCtx);
}
void MDrvSclHvspReSetHw(void *pvCtx)
{
    DrvSclHvspReSetHw(pvCtx);
}
bool MDrvSclHvspExit(bool bCloseISR)
{
    if(_MHVSP_Mutex != -1)
    {
         DrvSclOsDeleteMutex(_MHVSP_Mutex);
         _MHVSP_Mutex = -1;
    }
    DrvSclHvspExit(bCloseISR);
    MDrvSclCtxDeInit();
    return 1;
}
bool MDrvSclHvspInit(MDrvSclHvspIdType_e enHVSP_ID, MDrvSclHvspInitConfig_t *pCfg)
{
    DrvSclHvspInitConfig_t stInitCfg;
    char word[] = {"_MHVSP_Mutex"};
    DrvSclOsMemset(&stInitCfg,0,sizeof(DrvSclHvspInitConfig_t));
    if(DrvSclOsInit() == FALSE)
    {
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[MDRVHVSP]%s(%d) init SclOS fail\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (_MHVSP_Mutex != -1)
    {
        SCL_DBG(SCL_DBG_LV_DRVHVSP(), "[MDRVHVSP]%s(%d) alrady done\n", __FUNCTION__, __LINE__);
        return 1;
    }
    _MHVSP_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);

    if (_MHVSP_Mutex == -1)
    {
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[DRVHVSP]%s(%d): create mutex fail\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }
    stInitCfg.u32RIUBase    = pCfg->u32Riubase;
    stInitCfg.u32IRQNUM     = pCfg->u32IRQNUM;
    stInitCfg.u32CMDQIRQNUM = pCfg->u32CMDQIRQNUM;
    stInitCfg.pvCtx = pCfg->pvCtx;
    DrvSclOsSetSclFrameBufferNum(DNR_BUFFER_MODE);
    if(DrvSclHvspInit(&stInitCfg) == FALSE)
    {
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[HVSP]%s Init Fail\n", __FUNCTION__);
        return FALSE;
    }
    else
    {
        MDrvSclHvspVtrackInit();
        _MDrvSclHvspSwInit((MDrvSclCtxCmdqConfig_t *)pCfg->pvCtx);
        return TRUE;
    }

}
bool MDrvSclHvspSuspend(MDrvSclHvspIdType_e enHVSP_ID, MDrvSclHvspSuspendResumeConfig_t *pCfg)
{
    DrvSclHvspSuspendResumeConfig_t stSuspendResumeCfg;
    bool bRet = TRUE;;
    DrvSclOsMemset(&stSuspendResumeCfg,0,sizeof(DrvSclHvspSuspendResumeConfig_t));
    stSuspendResumeCfg.u32IRQNUM = pCfg->u32IRQNum;
    stSuspendResumeCfg.u32CMDQIRQNUM = pCfg->u32CMDQIRQNum;
    if(DrvSclHvspSuspend(&stSuspendResumeCfg))
    {
        bRet = TRUE;
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[HVSP]%s Suspend Fail\n", __FUNCTION__);
        bRet = FALSE;
    }

    return bRet;
}

bool MDrvSclHvspResume(MDrvSclHvspIdType_e enHVSP_ID, MDrvSclHvspSuspendResumeConfig_t *pCfg)
{
    DrvSclHvspSuspendResumeConfig_t stSuspendResumeCfg;
    bool bRet = TRUE;;
    DrvSclOsMemset(&stSuspendResumeCfg,0,sizeof(DrvSclHvspSuspendResumeConfig_t));
    stSuspendResumeCfg.u32IRQNUM = pCfg->u32IRQNum;
    stSuspendResumeCfg.u32CMDQIRQNUM = pCfg->u32CMDQIRQNum;
    if(DrvSclHvspResume(&stSuspendResumeCfg))
    {
        MDrvSclHvspVtrackInit();
        bRet = TRUE;
    }
    else
    {
        SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "[HVSP]%s Resume Fail\n", __FUNCTION__);
        bRet = FALSE;
    }

    return bRet;
}
void _MDrvSclHvspSetPatTgenStatusByInst(bool u8inst)
{
    MDrvSclHvspInputConfig_t stCfg;
    DrvSclHvspInputInformConfig_t stInformCfg;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    s32 s32Handler;
    u32 i;
    if(u8inst>=MDRV_SCL_CTX_INSTANT_MAX)
    {
        u8inst = 0;
    }
    pCtxCfg = MDrvSclCtxGetCtx(u8inst);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX)
            {
                break;
            }
        }
        DrvSclOsMemset(&stInformCfg,0,sizeof(DrvSclHvspInputInformConfig_t));
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        DrvSclHvspGetCrop12Inform(&stInformCfg);
        stCfg.enColor = E_MDRV_SCLHVSP_COLOR_RGB;
        stCfg.enSrcType = E_MDRV_SCLHVSP_SRC_PAT_TGEN;
        stCfg.pvCtx = &(pCtxCfg->stCtx);
        stCfg.stCaptureWin.u16Height = stInformCfg.u16inCropHeight;
        stCfg.stCaptureWin.u16Width= stInformCfg.u16inCropWidth;
        MDrvSclHvspSetInputConfig(E_MDRV_SCLHVSP_ID_1,&stCfg);
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
    }
}
void MDrvSclHvspSetPatTgenStatus(bool bEn)
{
    u32 i;
    if(bEn)
    {
        for(i=0;i<MDRV_SCL_CTX_INSTANT_MAX;i++)
        {
            _MDrvSclHvspSetPatTgenStatusByInst((u8)i);
        }
    }
    else
    {
        _MDrvSclHvspSetPatTgenStatusByInst(gu8LevelInst);
    }
}
bool MDrvSclHvspSetInputConfig(MDrvSclHvspIdType_e enHVSP_ID, MDrvSclHvspInputConfig_t *pCfg)
{
    bool Ret = TRUE;
    SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "%s:%d\n", __FUNCTION__,enHVSP_ID);
    if(enHVSP_ID == E_MDRV_SCLHVSP_ID_1)
    {
        DrvSclHvspIpMuxType_e enIPMux;
        ST_HVSP_SIZE_CONFIG stSize;
        DrvSclOsMemset(&stSize,0,sizeof(ST_HVSP_SIZE_CONFIG));
        enIPMux = pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_ISP      ? E_DRV_SCLHVSP_IP_MUX_ISP :
                  pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_BT656    ? E_DRV_SCLHVSP_IP_MUX_BT656 :
                  pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_HVSP     ? E_DRV_SCLHVSP_IP_MUX_HVSP :
                  pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_PAT_TGEN ? E_DRV_SCLHVSP_IP_MUX_PAT_TGEN :
                                                                 E_DRV_SCLHVSP_IP_MUX_MAX;
        Ret &= (bool)DrvSclHvspSetInputMux(enIPMux,(DrvSclHvspClkConfig_t *)pCfg->stclk,pCfg->pvCtx);
        stSize.u16Height = pCfg->stCaptureWin.u16Height;
        stSize.u16Width = pCfg->stCaptureWin.u16Width;
        stSize.pvCtx = pCfg->pvCtx;
        DrvSclHvspSetInputSrcSize(&stSize);
        if(_MDrvSclHvspIsInputSrcPatternGen(enIPMux, pCfg))
        {
            Ret &= TRUE;
        }
        else
        {
            Ret &= (bool)DrvSclHvspSetPatTgen(FALSE, NULL);
        }

    }
    else if(enHVSP_ID == E_MDRV_SCLHVSP_ID_3)
    {
        DrvSclHvspIpMuxType_e enIPMux;
        enIPMux = pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_HVSP      ? E_DRV_SCLHVSP_IP_MUX_HVSP :
                  pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_DRAM_LDC      ? E_DRV_SCLHVSP_IP_MUX_LDC :
                  pCfg->enSrcType == E_MDRV_SCLHVSP_SRC_DRAM_RSC  ? E_DRV_SCLHVSP_IP_MUX_RSC :
                                                                 E_DRV_SCLHVSP_IP_MUX_MAX;
        if(enIPMux==E_DRV_SCLHVSP_IP_MUX_MAX)
        {
            return 0;
        }
        DrvSclHvspSetSc3InputMux(enIPMux,pCfg->pvCtx);
    }
    return Ret;
}

bool MDrvSclHvspSetScalingConfig(MDrvSclHvspIdType_e enHVSP_ID, MDrvSclHvspScalingConfig_t *pCfg )
{
    DrvSclHvspIdType_e enID;
    DrvSclHvspScalingConfig_t stScalingCfg;
    bool ret;
    DrvSclOsMemset(&stScalingCfg,0,sizeof(DrvSclHvspScalingConfig_t));
    SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(enHVSP_ID)), "%s:%d\n", __FUNCTION__,enHVSP_ID);
    enID = enHVSP_ID == E_MDRV_SCLHVSP_ID_2 ? E_DRV_SCLHVSP_ID_2 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_3 ? E_DRV_SCLHVSP_ID_3 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_4 ? E_DRV_SCLHVSP_ID_4 :
                                           E_DRV_SCLHVSP_ID_1;

    if(enID == E_DRV_SCLHVSP_ID_1)
    {
       MDrvSclHvspMutexLock();
       _MDrvSclHvspSetGlobal((MDrvSclCtxCmdqConfig_t *)pCfg->pvCtx);
       DrvSclOsMemcpy(&gstMhvspGlobalSet->gstHvspScalingCfg, pCfg, sizeof(MDrvSclHvspScalingConfig_t));
       MDrvSclHvspMutexUnlock();
    }

    if( SetPostCrop || (DrvSclHvspGetInputSrcMux(enID)==E_DRV_SCLHVSP_IP_MUX_PAT_TGEN))
    {
        stScalingCfg.bCropEn[DRV_HVSP_CROP_1]        = 0;
        stScalingCfg.u16Crop_X[DRV_HVSP_CROP_1]      = 0;
        stScalingCfg.u16Crop_Y[DRV_HVSP_CROP_1]      = 0;
        stScalingCfg.u16Crop_Width[DRV_HVSP_CROP_1]  = pCfg->u16Src_Width;
        stScalingCfg.u16Crop_Height[DRV_HVSP_CROP_1] = pCfg->u16Src_Height;
        stScalingCfg.bCropEn[DRV_HVSP_CROP_2]        = pCfg->stCropWin.bEn;
        stScalingCfg.u16Crop_X[DRV_HVSP_CROP_2]      = pCfg->stCropWin.u16X;
        stScalingCfg.u16Crop_Y[DRV_HVSP_CROP_2]      = pCfg->stCropWin.u16Y;
        stScalingCfg.u16Crop_Width[DRV_HVSP_CROP_2]  = pCfg->stCropWin.u16Width;
        stScalingCfg.u16Crop_Height[DRV_HVSP_CROP_2] = pCfg->stCropWin.u16Height;
    }
    else
    {
        stScalingCfg.bCropEn[DRV_HVSP_CROP_1]        = pCfg->stCropWin.bEn;
        stScalingCfg.u16Crop_X[DRV_HVSP_CROP_1]      = pCfg->stCropWin.u16X;
        stScalingCfg.u16Crop_Y[DRV_HVSP_CROP_1]      = pCfg->stCropWin.u16Y;
        stScalingCfg.u16Crop_Width[DRV_HVSP_CROP_1]  = pCfg->stCropWin.u16Width;
        stScalingCfg.u16Crop_Height[DRV_HVSP_CROP_1] = pCfg->stCropWin.u16Height;
        stScalingCfg.bCropEn[DRV_HVSP_CROP_2]        = 0;
        stScalingCfg.u16Crop_X[DRV_HVSP_CROP_2]      = 0;
        stScalingCfg.u16Crop_Y[DRV_HVSP_CROP_2]      = 0;
        stScalingCfg.u16Crop_Width[DRV_HVSP_CROP_2]  = pCfg->stCropWin.u16Width;
        stScalingCfg.u16Crop_Height[DRV_HVSP_CROP_2] = pCfg->stCropWin.u16Height;
    }
    stScalingCfg.u16Src_Width  = pCfg->u16Src_Width;
    stScalingCfg.u16Src_Height = pCfg->u16Src_Height;
    stScalingCfg.u16Dsp_Width  = pCfg->u16Dsp_Width;
    stScalingCfg.u16Dsp_Height = pCfg->u16Dsp_Height;
    stScalingCfg.pvCtx = pCfg->pvCtx;
    ret = DrvSclHvspSetScaling(enID, &stScalingCfg,(DrvSclHvspClkConfig_t *)pCfg->stclk);

    return  ret;
}




bool MDrvSclHvspSetMiscConfig(MDrvSclHvspMiscConfig_t *pCfg)
{
    u8 *pBuf = NULL;

    SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "%s\n", __FUNCTION__);

    pBuf = DrvSclOsVirMemalloc(pCfg->u32Size);

    if(pBuf == NULL)
    {
        SCL_ERR( "[HVSP1] allocate buffer fail\n");
        return 0;
    }


    if(DrvSclOsCopyFromUser(pBuf, (void *)pCfg->u32Addr, pCfg->u32Size))
    {
        SCL_ERR( "[HVSP1] copy msic buffer error\n");
        DrvSclOsVirMemFree(pBuf);
        return 0;
    }

    switch(pCfg->u8Cmd)
    {
        case E_MDRV_SCLHVSP_MISC_CMD_SET_REG:
            _MDrvSclHvspSetRegisterForce(pCfg->u32Size, pBuf,pCfg->pvCtx);
            break;

        default:
            break;
    }

    DrvSclOsVirMemFree(pBuf);

    return 1;
}

bool MDrvSclHvspSetMiscConfigForKernel(MDrvSclHvspMiscConfig_t *pCfg)
{
    u16 u16inst;
     SCL_DBG(SCL_DBG_LV_HVSP()&(Get_DBGMG_HVSP(3)), "%s\n", __FUNCTION__);
     switch(pCfg->u8Cmd)
     {
         case E_MDRV_SCLHVSP_MISC_CMD_SET_REG:
             _MDrvSclHvspSetRegisterForce(pCfg->u32Size, (u8 *)pCfg->u32Addr,pCfg->pvCtx);
             break;
         case E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINSTALL:
             _MDrvSclHvspSetRegisterForceByAllInst(pCfg->u32Size, (u8 *)pCfg->u32Addr,pCfg->pvCtx);
             break;
         case E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINST:
             u16inst = gu8LevelInst;
             if(gu8LevelInst==0xFF)
             {
                u16inst = 0;
             }
             _MDrvSclHvspSetRegisterForceByInst(pCfg->u32Size, (u8 *)pCfg->u32Addr,(void*)&u16inst);
             break;
         default:
             break;
     }
     return 1;
}
ssize_t MDrvSclHvspProcShowInst(char *buf,u8 u8inst)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    DrvSclHvspInformConfig_t sthvspformCfg;
    DrvSclHvspInformConfig_t sthvsp2formCfg;
    DrvSclHvspInformConfig_t sthvsp3formCfg;
    DrvSclHvspInformConfig_t sthvsp4formCfg;
    DrvSclHvspScInformConfig_t stzoomformCfg;
    DrvSclHvspIpmConfig_t stIpmformCfg;
    DrvSclHvspInputInformConfig_t stInformCfg;
    s32 s32Handler;
    u32 i;
    int u32idx;
    MDrvSclDmaGetConfig_t stGetCfg;
    MDrvSclDmaAttrType_t stScldmaAttr;
    MDrvSclDmaAttrType_t stScldmaAttr1;
    MDrvSclDmaAttrType_t stScldmaAttr2;
    MDrvSclDmaAttrType_t stScldmaAttr3;
    MDrvSclDmaAttrType_t stScldmaAttr4;
    MDrvSclDmaAttrType_t stScldmaAttr5;
    MDrvSclDmaAttrType_t stScldmaAttr6;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    MDrvSclCtxNrBufferGlobalSet_t *stNrGlobalSet;
    pCtxCfg = MDrvSclCtxGetCtx(u8inst);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX)
            {
                break;
            }
        }
        DrvSclOsMemset(&sthvspformCfg,0,sizeof(DrvSclHvspInformConfig_t));
        DrvSclOsMemset(&sthvsp2formCfg,0,sizeof(DrvSclHvspInformConfig_t));
        DrvSclOsMemset(&sthvsp3formCfg,0,sizeof(DrvSclHvspInformConfig_t));
        DrvSclOsMemset(&sthvsp4formCfg,0,sizeof(DrvSclHvspInformConfig_t));
        DrvSclOsMemset(&stzoomformCfg,0,sizeof(DrvSclHvspScInformConfig_t));
        DrvSclOsMemset(&stIpmformCfg,0,sizeof(DrvSclHvspIpmConfig_t));
        DrvSclOsMemset(&stInformCfg,0,sizeof(DrvSclHvspInputInformConfig_t));
        //out =0,in=1
        DrvSclOsMemset(&stGetCfg,0,sizeof(MDrvSclDmaGetConfig_t));
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        sthvspformCfg.pvCtx = &(pCtxCfg->stCtx);
        sthvsp2formCfg.pvCtx = &(pCtxCfg->stCtx);
        sthvsp3formCfg.pvCtx = &(pCtxCfg->stCtx);
        sthvsp4formCfg.pvCtx = &(pCtxCfg->stCtx);
        stIpmformCfg.pvCtx = &(pCtxCfg->stCtx);
        DrvSclHvspGetCrop12Inform(&stInformCfg);
        DrvSclHvspGetSCLInform(E_DRV_SCLHVSP_ID_1,&stzoomformCfg);
        DrvSclHvspGetHvspAttribute(E_DRV_SCLHVSP_ID_1,&sthvspformCfg);
        DrvSclHvspGetHvspAttribute(E_DRV_SCLHVSP_ID_2,&sthvsp2formCfg);
        DrvSclHvspGetHvspAttribute(E_DRV_SCLHVSP_ID_3,&sthvsp3formCfg);
        DrvSclHvspGetHvspAttribute(E_DRV_SCLHVSP_ID_4,&sthvsp4formCfg);
        DrvSclHvspGetFrameBufferAttribute(E_DRV_SCLHVSP_ID_1,&stIpmformCfg);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_1;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRM;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_1;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_SNP;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr1);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_2;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRM;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr2);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_2;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRM2;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr3);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_3;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRM;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr4);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_3;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRMR;
        stGetCfg.bReadDMAMode = 1;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr5);
        stGetCfg.enSCLDMA_ID = E_MDRV_SCLDMA_ID_MDWIN;
        stGetCfg.enMemType = E_MDRV_SCLDMA_MEM_FRM;
        stGetCfg.bReadDMAMode = 0;
        MDrvSclDmaGetDmaInformationByClient(&stGetCfg,&stScldmaAttr6);
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        pCtxCfg = MDrvSclCtxGetCtx(u8inst+MDRV_SCL_CTX_INSTANT_MAX);
        if(pCtxCfg->bUsed)
        {
            stNrGlobalSet = &(((MDrvSclCtxGlobalSet_t*)(pCtxCfg->stCtx.pgstGlobalSet))->stNrBufferCfg);
        }
        SCL_ERR("========================SCL PROC FRAMEWORK======================\n");
        SCL_ERR("Inst:%hhu\n",u8inst);
        SCL_ERR("Read Inst:%hhu\n",gu8LevelInst);
        SCL_ERR("------------------------SCL INPUT MUX----------------------\n");
        if(stInformCfg.enMux==0)
        {
            SCL_ERR("Input SRC :BT656\n");
        }
        else if(stInformCfg.enMux==1)
        {
            SCL_ERR("Input SRC :ISP\n");
        }
        else if(stInformCfg.enMux==3)
        {
            SCL_ERR("Input SRC :PTGEN\n");
        }
        else
        {
            SCL_ERR("Input SRC :OTHER\n");
        }
        if(stInformCfg.enSc3Mux==2)
        {
            SCL_ERR("SC3 SRC :HVSP\n");
        }
        else if(stInformCfg.enSc3Mux==4)
        {
            SCL_ERR("SC3 SRC :LDC\n");
        }
        else if(stInformCfg.enSc3Mux==5)
        {
            SCL_ERR("SC3 SRC :RSC\n");
        }
        else
        {
            SCL_ERR("SC3 SRC :OTHER\n");
        }
        SCL_ERR("Input H   :%hd\n",stInformCfg.u16inWidth);
        SCL_ERR("Input V   :%hd\n",stInformCfg.u16inHeight);
        SCL_ERR("(only for single ch)Receive H :%hd\n",stInformCfg.u16inWidthcount);
        SCL_ERR("(only for single ch)Receive V :%hd\n",stInformCfg.u16inHeightcount);
        SCL_ERR("------------------------SCL FB-----------------------------\n");
        SCL_ERR("FB H          :%hd\n",stIpmformCfg.u16Fetch);
        SCL_ERR("FB V          :%hd\n",stIpmformCfg.u16Vsize);
        if(pCtxCfg->bUsed)
        {
            SCL_ERR("FB Addr       :%lx\n",(u32)stNrGlobalSet->PhyMcnrYCMAddr);
            SCL_ERR("FB memsize    :%ld\n",(u32)stNrGlobalSet->u32McnrSize);
            SCL_ERR("FB Buffer     :%hhd\n",DrvSclOsGetSclFrameBufferNum());
        }
        SCL_ERR("------------------------SCL Crop----------------------------\n");
        SCL_ERR("Crop      :%hhd\n",stInformCfg.bEn);
        SCL_ERR("CropX     :%hd\n",stInformCfg.u16inCropX);
        SCL_ERR("CropY     :%hd\n",stInformCfg.u16inCropY);
        SCL_ERR("CropOutW  :%hd\n",stInformCfg.u16inCropWidth);
        SCL_ERR("CropOutH  :%hd\n",stInformCfg.u16inCropHeight);
        SCL_ERR("SrcW      :%hd\n",stInformCfg.u16inWidth);
        SCL_ERR("SrcH      :%hd\n",stInformCfg.u16inHeight);
        SCL_ERR("------------------------SCL Zoom----------------------------\n");
        SCL_ERR("Zoom      :%hhd\n",stzoomformCfg.bEn);
        SCL_ERR("ZoomX     :%hd\n",stzoomformCfg.u16X);
        SCL_ERR("ZoomY     :%hd\n",stzoomformCfg.u16Y);
        SCL_ERR("ZoomOutW  :%hd\n",stzoomformCfg.u16crop2OutWidth);
        SCL_ERR("ZoomOutH  :%hd\n",stzoomformCfg.u16crop2OutHeight);
        SCL_ERR("SrcW      :%hd\n",stzoomformCfg.u16crop2inWidth);
        SCL_ERR("SrcH      :%hd\n",stzoomformCfg.u16crop2inHeight);
        SCL_ERR("------------------------SCL HVSP1----------------------------\n");
        SCL_ERR("InputH    :%hd\n",sthvspformCfg.u16inWidth);
        SCL_ERR("InputV    :%hd\n",sthvspformCfg.u16inHeight);
        SCL_ERR("OutputH   :%hd\n",sthvspformCfg.u16Width);
        SCL_ERR("OutputV   :%hd\n",sthvspformCfg.u16Height);
        SCL_ERR("H en  :%hhx\n",sthvspformCfg.bEn&0x1);
        SCL_ERR("H function  :%hhx\n",(sthvspformCfg.bEn&0xC0)>>6);
        SCL_ERR("V en  :%hhx\n",(sthvspformCfg.bEn&0x2)>>1);
        SCL_ERR("V function  :%hhx\n",(sthvspformCfg.bEn&0x30)>>4);
        SCL_ERR("------------------------SCL HVSP2----------------------------\n");
        SCL_ERR("InputH    :%hd\n",sthvsp2formCfg.u16inWidth);
        SCL_ERR("InputV    :%hd\n",sthvsp2formCfg.u16inHeight);
        SCL_ERR("OutputH   :%hd\n",sthvsp2formCfg.u16Width);
        SCL_ERR("OutputV   :%hd\n",sthvsp2formCfg.u16Height);
        SCL_ERR("H en  :%hhx\n",sthvsp2formCfg.bEn&0x1);
        SCL_ERR("H function  :%hhx\n",(sthvsp2formCfg.bEn&0xC0)>>6);
        SCL_ERR("V en  :%hhx\n",(sthvsp2formCfg.bEn&0x2)>>1);
        SCL_ERR("V function  :%hhx\n",(sthvsp2formCfg.bEn&0x30)>>4);
        SCL_ERR("------------------------SCL HVSP3----------------------------\n");
        SCL_ERR("InputH    :%hd\n",sthvsp3formCfg.u16inWidth);
        SCL_ERR("InputV    :%hd\n",sthvsp3formCfg.u16inHeight);
        SCL_ERR("OutputH   :%hd\n",sthvsp3formCfg.u16Width);
        SCL_ERR("OutputV   :%hd\n",sthvsp3formCfg.u16Height);
        SCL_ERR("H en  :%hhx\n",sthvsp3formCfg.bEn&0x1);
        SCL_ERR("H function  :%hhx\n",(sthvsp3formCfg.bEn&0xC0)>>6);
        SCL_ERR("V en  :%hhx\n",(sthvsp3formCfg.bEn&0x2)>>1);
        SCL_ERR("V function  :%hhx\n",(sthvsp3formCfg.bEn&0x30)>>4);
        SCL_ERR("------------------------SCL HVSP4----------------------------\n");
        SCL_ERR("InputH    :%hd\n",sthvsp4formCfg.u16inWidth);
        SCL_ERR("InputV    :%hd\n",sthvsp4formCfg.u16inHeight);
        SCL_ERR("OutputH   :%hd\n",sthvsp4formCfg.u16Width);
        SCL_ERR("OutputV   :%hd\n",sthvsp4formCfg.u16Height);
        SCL_ERR("H en  :%hhx\n",sthvsp4formCfg.bEn&0x1);
        SCL_ERR("H function  :%hhx\n",(sthvsp4formCfg.bEn&0xC0)>>6);
        SCL_ERR("V en  :%hhx\n",(sthvsp4formCfg.bEn&0x2)>>1);
        SCL_ERR("V function  :%hhx\n",(sthvsp4formCfg.bEn&0x30)>>4);
        SCL_ERR("------------------------SCL DMA1FRM----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr.bVFilp);
        SCL_ERR("------------------------SCL DMA1SNP----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr1.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr1.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr1.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr1.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr1.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr1.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr1.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr1.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr1.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr1.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr1.bVFilp);
        SCL_ERR("------------------------SCL DMA2FRM----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr2.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr2.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr2.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr2.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr2.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr2.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr2.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr2.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr2.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr2.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr2.bVFilp);
        SCL_ERR("------------------------SCL DMA2FRM2----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr3.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr3.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr3.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr3.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr3.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr3.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr3.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr3.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr3.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr3.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr3.bVFilp);
        SCL_ERR("------------------------SCL DMA3FRMW----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr4.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr4.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr4.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr4.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr4.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr4.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr4.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr4.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr4.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr4.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr4.bVFilp);
        SCL_ERR("------------------------SCL DMA3FRMR----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr5.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr5.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr5.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr5.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr5.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr5.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr5.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr5.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr5.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr5.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr5.bVFilp);
        SCL_ERR("------------------------SCL MDWIN----------------------------\n");
        SCL_ERR("DMA Enable:%hhd\n",stScldmaAttr6.bDMAEn);
        SCL_ERR("DMA color format: %s\n",PARSING_SCLDMA_IOCOLOR(stScldmaAttr6.enColorType));
        SCL_ERR("DMA trigger mode: %s\n",PARSING_SCLDMA_IOBUFMD(stScldmaAttr6.enBufMDType));
        SCL_ERR("DMA Buffer Num: %hd\n",stScldmaAttr6.u16BufNum);
        for(u32idx=0 ;u32idx<stScldmaAttr6.u16BufNum;u32idx++)
        {
            SCL_ERR("DMA Buffer Y Address[%d]: %lx\n",u32idx,stScldmaAttr6.u32Base_Y[u32idx]);
            SCL_ERR("DMA Buffer C Address[%d]: %lx\n",u32idx,stScldmaAttr6.u32Base_C[u32idx]);
            SCL_ERR("DMA Buffer V Address[%d]: %lx\n",u32idx,stScldmaAttr6.u32Base_V[u32idx]);
        }
        SCL_ERR("DMA Stride: %lu\n",stScldmaAttr6.u32LineOffset_Y);
        SCL_ERR("DMA Mirror: %hhu\n",stScldmaAttr6.bHFilp);
        SCL_ERR("DMA Flip: %hhu\n",stScldmaAttr6.bVFilp);
        SCL_ERR("========================SCL PROC FRAMEWORK======================\n");
        end = end;
    }
    return (str - buf);
}
//#if defined (SCLOS_TYPE_LINUX_KERNEL)
void MDrvSclHvspSetProcInst(u8 u8level)
{
    gu8LevelInst = u8level;
}
ssize_t MDrvSclHvspProcShow(char *buf)
{
    u32 i;
    if(gu8LevelInst==0xFF)
    {
        // check all
        for(i=0;i<MDRV_SCL_CTX_INSTANT_MAX;i++)
        {
            MDrvSclHvspProcShowInst(buf,(u8)i);
        }
    }
    else if(gu8LevelInst<MDRV_SCL_CTX_INSTANT_MAX)
    {
        MDrvSclHvspProcShowInst(buf,gu8LevelInst);
    }
    return 0;
}
void MDrvSclHvspRegShowInst(u8 u8level,u32 u32RegAddr,bool bAllbank)
{
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    s32 s32Handler;
    u32 i;
    pCtxCfg = MDrvSclCtxGetCtx(u8level);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if(u8level<MDRV_SCL_CTX_INSTANT_MAX && ((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX))
            {
                break;
            }
            else if(u8level>=MDRV_SCL_CTX_INSTANT_MAX&& ((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX))
            {
                break;
            }
        }
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        SCL_ERR("[Dump]BANK:%lx Inst:%hhx\n",((u32RegAddr&0xFFFF00)>>8),u8level);
        MDrvSclCtxDumpRegSetting(u32RegAddr,bAllbank);
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
    }
}
ssize_t MDrvSclHvspRegShow(char *buf,u32 u32RegAddr,u16 *u16RegVal,bool bAllbank)
{
    u32 i;
    if(gu8LevelInst==0xFF)
    {
        // check all
        for(i=0;i<E_MDRV_SCL_CTX_ID_NUM *MDRV_SCL_CTX_INSTANT_MAX;i++)
        {
            MDrvSclHvspRegShowInst(i,u32RegAddr,bAllbank);
        }
    }
    else if(gu8LevelInst<E_MDRV_SCL_CTX_ID_NUM *MDRV_SCL_CTX_INSTANT_MAX)
    {
        MDrvSclHvspRegShowInst(gu8LevelInst,u32RegAddr,bAllbank);
    }
    return 0;
}
void MDrvSclHvspDbgmgDumpShow(u8 u8level)
{
    u32 i,j;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    s32 s32Handler;
    if(u8level==0xFF)
    {
        // check all
        for(j=0;j<E_MDRV_SCL_CTX_ID_NUM *MDRV_SCL_CTX_INSTANT_MAX;j++)
        {
            pCtxCfg = MDrvSclCtxGetCtx(j);
            if(pCtxCfg->bUsed)
            {
                for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
                {
                    s32Handler = pCtxCfg->s32Id[i];
                    if(j<MDRV_SCL_CTX_INSTANT_MAX && ((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX))
                    {
                        break;
                    }
                    else if(j>=MDRV_SCL_CTX_INSTANT_MAX&& ((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX))
                    {
                        break;
                    }
                }
                MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
                MDrvSclCtxDumpSetting();
                MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
            }
        }
    }
    else if(u8level<E_MDRV_SCL_CTX_ID_NUM*MDRV_SCL_CTX_INSTANT_MAX)
    {
        pCtxCfg = MDrvSclCtxGetCtx(u8level);
        if(pCtxCfg->bUsed)
        {
            for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
            {
                s32Handler = pCtxCfg->s32Id[i];
                if(u8level<MDRV_SCL_CTX_INSTANT_MAX && ((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX))
                {
                    break;
                }
                else if(u8level>=MDRV_SCL_CTX_INSTANT_MAX&& ((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX))
                {
                    break;
                }
            }
            MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
            MDrvSclCtxDumpSetting();
            MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        }
    }
}
ssize_t MDrvSclHvspDbgmgFlagShow(char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    str += DrvSclOsScnprintf(str, end - str, "========================SCL Debug Message BUTTON======================\n");
    str += DrvSclOsScnprintf(str, end - str, "CONFIG            ECHO        STATUS\n");
    str += DrvSclOsScnprintf(str, end - str, "MDRV_CONFIG       (1)         0x%x\n",gbdbgmessage[EN_DBGMG_MDRV_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "IOCTL_CONFIG      (2)         0x%x\n",gbdbgmessage[EN_DBGMG_IOCTL_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "HVSP_CONFIG       (3)         0x%x\n",gbdbgmessage[EN_DBGMG_HVSP_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "SCLDMA_CONFIG     (4)         0x%x\n",gbdbgmessage[EN_DBGMG_SCLDMA_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "PNL_CONFIG        (5)         0x%x\n",gbdbgmessage[EN_DBGMG_PNL_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "VIP_CONFIG        (6)         0x%x\n",gbdbgmessage[EN_DBGMG_VIP_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "DRVPQ_CONFIG      (7)         0x%x\n",gbdbgmessage[EN_DBGMG_DRVPQ_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "CTX_CONFIG        (8)         0x%x\n",gbdbgmessage[EN_DBGMG_CTX_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "VPESCL_CONFIG     (9)         0x%x\n",gbdbgmessage[EN_DBGMG_VPESCL_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "VPEIQ_CONFIG      (A)         0x%x\n",gbdbgmessage[EN_DBGMG_VPEIQ_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "DRVHVSP_CONFIG    (B)         0x%x\n",gbdbgmessage[EN_DBGMG_DRVHVSP_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "DRVSCLDMA_CONFIG  (C)         0x%x\n",gbdbgmessage[EN_DBGMG_DRVSCLDMA_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "DRVSCLIRQ_CONFIG  (D)         0x%x\n",gbdbgmessage[EN_DBGMG_DRVSCLIRQ_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "VPEISP_CONFIG     (E)         0x%x\n",gbdbgmessage[EN_DBGMG_CMDQ_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "DRVVIP_CONFIG     (F)         0x%x\n",gbdbgmessage[EN_DBGMG_DRVVIP_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "PRIORITY_CONFIG   (G)         0x%x\n",gbdbgmessage[EN_DBGMG_PRIORITY_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "UTILITY_CONFIG    (H)         0x%x\n",gbdbgmessage[EN_DBGMG_UTILITY_CONFIG]);
    str += DrvSclOsScnprintf(str, end - str, "UTILITY_DUMP      (I)         byLevel\n");
    str += DrvSclOsScnprintf(str, end - str, "ALL Reset         (0)\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL Debug Message BUTTON======================\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL Debug Message LEVEL======================\n");
    str += DrvSclOsScnprintf(str, end - str, "default is level 1\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------IOCTL LEVEL---------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : SC1\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : SC2\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : SC3\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : VIP\n");
    str += DrvSclOsScnprintf(str, end - str, "0x10 : SC1HLEVEL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x20 : SC2HLEVEL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x40 : LCD\n");
    str += DrvSclOsScnprintf(str, end - str, "0x80 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------HVSP LEVEL---------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : HVSP1\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : HVSP2\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : HVSP3\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------SCLDMA LEVEL-------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : SC1 FRM\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : SC1 SNP \n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : SC2 FRM\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : SC3 FRM\n");
    str += DrvSclOsScnprintf(str, end - str, "0x10 : SC1 FRM HL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x20 : SC1 SNP HL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x40 : SC2 FRM HL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x80 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------VIP LEVEL(IOlevel)-------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : NORMAL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : VIP LOG \n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : VIP SUSPEND\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "------------------------------CTX LEVEL---------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : Low freq\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : \n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : High freq\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : \n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------PQ LEVEL---------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : befor crop\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : color eng\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : VIP Y\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : VIP C\n");
    str += DrvSclOsScnprintf(str, end - str, "0x10 : AIP\n");
    str += DrvSclOsScnprintf(str, end - str, "0x20 : AIP post\n");
    str += DrvSclOsScnprintf(str, end - str, "0x40 : HVSP\n");
    str += DrvSclOsScnprintf(str, end - str, "0x80 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------SCLIRQ LEVEL(drvlevel)---------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : NORMAL\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : SC1RINGA \n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : SC1RINGN\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : SC1SINGLE\n");
    str += DrvSclOsScnprintf(str, end - str, "0x10 : SC2RINGA\n");
    str += DrvSclOsScnprintf(str, end - str, "0x20 : SC2RINGN \n");
    str += DrvSclOsScnprintf(str, end - str, "0x40 : SC3SINGLE\n");
    str += DrvSclOsScnprintf(str, end - str, "0x80 : ELSE\n");
    str += DrvSclOsScnprintf(str, end - str, "-------------------------------VPE LEVEL-------------------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 1 : Level 1 low sc:freq size Iq:cal\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 2 : Level 2 low freq config\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 4 : Level 3 high sc:freq size Iq:cal\n");
    str += DrvSclOsScnprintf(str, end - str, "0x 8 : Level 4 high freq config\n");
    str += DrvSclOsScnprintf(str, end - str, "0x10 : Level 5 port 0 \n");
    str += DrvSclOsScnprintf(str, end - str, "0x20 : Level 6 port 1 \n");
    str += DrvSclOsScnprintf(str, end - str, "0x40 : Level 7 port 2 \n");
    str += DrvSclOsScnprintf(str, end - str, "0x80 : Level 8 port 3 \n");
    str += DrvSclOsScnprintf(str, end - str, "\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL Debug Message LEVEL======================\n");
    end = end;
    return (str - buf);
}
u32 DrvSclIrqGetProcessHist(u32 enType);
u32 DrvSclIrqGetProcessDiffDoneHist(u32 enType);
void DrvSclIrqGetFps(u32 enType,bool bHW,u32 *u32Fps,u32 *u32fFps);

ssize_t MDrvSclHvspIntsShow(char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    DrvSclHvspScIntsType_t stInts;
    u32 u32val;
    u32 u32fps;
    u32 u32ffps;
    MDrvSclCtxCmdqConfig_t* pvCtx;
    DrvSclOsMemset(&stInts,0,sizeof(DrvSclHvspScIntsType_t));
    pvCtx = MDrvSclCtxGetConfigCtx(E_MDRV_SCL_CTX_ID_SC_ALL);
    DrvSclHvspGetSclInts(pvCtx,&stInts);
    str += DrvSclOsScnprintf(str, end - str, "========================SCL INTS======================\n");
    str += DrvSclOsScnprintf(str, end - str, "AFF           Count: %lu\n",stInts.u32AffCount);
    str += DrvSclOsScnprintf(str, end - str, "DMAERROR      Count: %hu\n",stInts.u16ErrorCount);
    str += DrvSclOsScnprintf(str, end - str, "FAKEERROR      Count: %hu\n",stInts.u16FakeErrorCount);
    str += DrvSclOsScnprintf(str, end - str, "ISPIn         Count: %lu\n",stInts.u32ISPInCount);
    str += DrvSclOsScnprintf(str, end - str, "ISPDone       Count: %lu\n",stInts.u32ISPDoneCount);
    str += DrvSclOsScnprintf(str, end - str, "DIFF          Count: %lu\n",(stInts.u32ISPInCount-stInts.u32ISPDoneCount));
    str += DrvSclOsScnprintf(str, end - str, "Port0Done     Count: %lu\n",stInts.u32DoneCount[E_DRV_SCLHVSP_ID_1]);
    str += DrvSclOsScnprintf(str, end - str, "Port1Done     Count: %lu\n",stInts.u32DoneCount[E_DRV_SCLHVSP_ID_2]);
    str += DrvSclOsScnprintf(str, end - str, "Port2Done     Count: %lu\n",stInts.u32DoneCount[E_DRV_SCLHVSP_ID_3]);
    str += DrvSclOsScnprintf(str, end - str, "Port3Done     Count: %lu\n",stInts.u32DoneCount[E_DRV_SCLHVSP_ID_4]);
    str += DrvSclOsScnprintf(str, end - str, "SCLMainDone   Count: %lu\n",stInts.u32SCLMainDoneCount);
    str += DrvSclOsScnprintf(str, end - str, "M2MDone       Count: %lu\n",stInts.u32M2MDoneCount);
    DrvSclIrqGetFps(0,1,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port0          Fps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(1,1,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port1          Fps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(2,1,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port2          Fps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(3,1,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port3          Fps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(0,0,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port0  ProcessFps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(1,0,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port1  ProcessFps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(2,0,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port2  ProcessFps: %lu.%lu\n",u32fps,u32ffps);
    DrvSclIrqGetFps(3,0,&u32fps,&u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port3  ProcessFps: %lu.%lu\n",u32fps,u32ffps);
    str += DrvSclOsScnprintf(str, end - str, "Port0Active   Addr: %lx\n",stInts.u32ActiveAddr[E_DRV_SCLHVSP_ID_1]);
    str += DrvSclOsScnprintf(str, end - str, "Port1Active   Addr: %lx\n",stInts.u32ActiveAddr[E_DRV_SCLHVSP_ID_2]);
    str += DrvSclOsScnprintf(str, end - str, "Port2Active   Addr: %lx\n",stInts.u32ActiveAddr[E_DRV_SCLHVSP_ID_3]);
    str += DrvSclOsScnprintf(str, end - str, "Port3Active   Addr: %lx\n",stInts.u32ActiveAddr[E_DRV_SCLHVSP_ID_4]);
    u32val = (stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_1] >=stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_1]) ?
        stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_1] - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_1] : stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_1] + 1 + (0xFF - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_1]);
    str += DrvSclOsScnprintf(str, end - str, "Port0InQueue  Count: %lu\n",u32val);
    str += DrvSclOsScnprintf(str, end - str, "Port0Active   En: %hhu\n",stInts.u8Act[E_DRV_SCLHVSP_ID_1]);
    u32val = (stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_2] >=stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_2]) ?
        stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_2] - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_2] : stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_2] + 1 + (0xFF - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_2]);
    str += DrvSclOsScnprintf(str, end - str, "Port1InQueue  Count: %lu\n",u32val);
    str += DrvSclOsScnprintf(str, end - str, "Port1Active   En: %hhu\n",stInts.u8Act[E_DRV_SCLHVSP_ID_2]);
    u32val = (stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_3] >=stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_3]) ?
        stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_3] - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_3] : stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_3] + 1 + (0xFF - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_3]);
    str += DrvSclOsScnprintf(str, end - str, "Port2InQueue  Count: %lu\n",u32val);
    str += DrvSclOsScnprintf(str, end - str, "Port2Active   En: %hhu\n",stInts.u8Act[E_DRV_SCLHVSP_ID_3]);
    u32val = (stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_4] >=stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_4]) ?
        stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_4] - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_4] : stInts.u8QueueWCnt[E_DRV_SCLHVSP_ID_4] + 1 + (0xFF - stInts.u8QueueRCnt[E_DRV_SCLHVSP_ID_4]);
    str += DrvSclOsScnprintf(str, end - str, "Port3InQueue  Count: %lu\n",u32val);
    str += DrvSclOsScnprintf(str, end - str, "Port3Active   En: %hhu\n",stInts.u8Act[E_DRV_SCLHVSP_ID_4]);
    u32val = stInts.u32SCLMainActiveTime;
    str += DrvSclOsScnprintf(str, end - str, "SCLMain       ActiveTime: %lu (us)\n",u32val);
    u32val = stInts.u32M2MActiveTime;
    str += DrvSclOsScnprintf(str, end - str, "M2M            ActiveTime: %lu (us)\n",u32val);
    u32val = stInts.u32ISPTime;
    str += DrvSclOsScnprintf(str, end - str, "ISP           ActiveTime: %lu (us)\n",u32val);
    str += DrvSclOsScnprintf(str, end - str, "ISP           BlankingTime: %lu (us)\n",stInts.u32ISPBlankingTime);
    str += DrvSclOsScnprintf(str, end - str, "Process     DiffTime: %lu (us)\n",stInts.u32ProcessDiffTime);
    str += DrvSclOsScnprintf(str, end - str, "Process DiffDoneTime: %lu (us)\n",stInts.u32ProcessDiffDoneTime);
    str += DrvSclOsScnprintf(str, end - str, "Process       Count: %hhu\n",stInts.u8ProcessCnt);
    str += DrvSclOsScnprintf(str, end - str, "Process       Active: %hhu\n",stInts.u8ProcessActCnt);
    str += DrvSclOsScnprintf(str, end - str, "Process Late not Drop Count: %hhu\n",stInts.u8ProcessLateCnt);
    str += DrvSclOsScnprintf(str, end - str, "Process Late and Drop Count: %hhu\n",stInts.u8ProcessDropCnt);
    str += DrvSclOsScnprintf(str, end - str, "---------------------Process diff hist---------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0~10ms            Count: %lu\n",DrvSclIrqGetProcessHist(0));
    str += DrvSclOsScnprintf(str, end - str, "10~20ms           Count: %lu\n",DrvSclIrqGetProcessHist(1));
    str += DrvSclOsScnprintf(str, end - str, "20~30ms           Count: %lu\n",DrvSclIrqGetProcessHist(2));
    str += DrvSclOsScnprintf(str, end - str, "30~40ms           Count: %lu\n",DrvSclIrqGetProcessHist(3));
    str += DrvSclOsScnprintf(str, end - str, "40~50ms           Count: %lu\n",DrvSclIrqGetProcessHist(4));
    str += DrvSclOsScnprintf(str, end - str, "50~60ms           Count: %lu\n",DrvSclIrqGetProcessHist(5));
    str += DrvSclOsScnprintf(str, end - str, "60~70ms           Count: %lu\n",DrvSclIrqGetProcessHist(6));
    str += DrvSclOsScnprintf(str, end - str, "70~80ms           Count: %lu\n",DrvSclIrqGetProcessHist(7));
    str += DrvSclOsScnprintf(str, end - str, "80~90ms           Count: %lu\n",DrvSclIrqGetProcessHist(8));
    str += DrvSclOsScnprintf(str, end - str, "90~100ms           Count: %lu\n",DrvSclIrqGetProcessHist(9));
    str += DrvSclOsScnprintf(str, end - str, "up to 100ms      Count: %lu\n",DrvSclIrqGetProcessHist(10));
    str += DrvSclOsScnprintf(str, end - str, "---------------------Process diff done hist---------------------\n");
    str += DrvSclOsScnprintf(str, end - str, "0~10ms            Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(0));
    str += DrvSclOsScnprintf(str, end - str, "10~20ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(1));
    str += DrvSclOsScnprintf(str, end - str, "20~30ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(2));
    str += DrvSclOsScnprintf(str, end - str, "30~40ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(3));
    str += DrvSclOsScnprintf(str, end - str, "40~50ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(4));
    str += DrvSclOsScnprintf(str, end - str, "50~60ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(5));
    str += DrvSclOsScnprintf(str, end - str, "60~70ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(6));
    str += DrvSclOsScnprintf(str, end - str, "70~80ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(7));
    str += DrvSclOsScnprintf(str, end - str, "80~90ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(8));
    str += DrvSclOsScnprintf(str, end - str, "90~100ms           Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(9));
    str += DrvSclOsScnprintf(str, end - str, "up to 100ms      Count: %lu\n",DrvSclIrqGetProcessDiffDoneHist(10));
    str += DrvSclOsScnprintf(str, end - str, "========================SCL INTS======================\n");
    end = end;
    return (str - buf);
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
void _MDrvSclHvspScIqStore(u8 u8Inst,const char *str,MDrvSclHvspIdType_e enHVSP_ID)
{
    DrvSclHvspIdType_e enID;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    enID = enHVSP_ID == E_MDRV_SCLHVSP_ID_2 ? E_DRV_SCLHVSP_ID_2 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_3 ? E_DRV_SCLHVSP_ID_3 :
           enHVSP_ID == E_MDRV_SCLHVSP_ID_4 ? E_DRV_SCLHVSP_ID_4 :
                                           E_DRV_SCLHVSP_ID_1;

   pCtxCfg = MDrvSclCtxGetCtx(u8Inst);
   if(pCtxCfg->bUsed)
   {
       SCL_ERR( "[SCLIQ]Set %d\n",(int)*str);
        if((int)*str == 49)    //input 1
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_Tbl0,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 50)  //input 2
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_Tbl1,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 51)  //input 3
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_Tbl2,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 52)  //input 4
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_Tbl3,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 53)  //input 5
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_BYPASS,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 54)  //input 6
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_H_BILINEAR,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 55)  //input 7
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_Tbl0,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 56)  //input 8
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_Tbl1,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 57)  //input 9
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_Tbl2,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 65)  //input A
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_Tbl3,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 66)  //input B
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_BYPASS,((void *)(&pCtxCfg->stCtx)));
        }
        else if((int)*str == 67)  //input C
        {
            DrvSclHvspSclIq(enID,E_DRV_SCLHVSP_IQ_V_BILINEAR,((void *)(&pCtxCfg->stCtx)));
        }
    }
}
void MDrvSclHvspScIqStore(const char *buf,MDrvSclHvspIdType_e enHVSP_ID)
{
    const char *str = buf;
    u32 i;
    if(gu8LevelInst==0xFF)
    {
        for(i=0;i<MDRV_SCL_CTX_INSTANT_MAX;i++)
        {
            _MDrvSclHvspScIqStore(i,str,enHVSP_ID);
        }
    }
    else
    {
        _MDrvSclHvspScIqStore(gu8LevelInst,str,enHVSP_ID);
    }
}
void _MDrvSclHvspScIqShow(u8 u8Inst,char **str,char **end, MDrvSclHvspIdType_e enHVSP_ID)
{
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    DrvSclHvspInformConfig_t sthvspformCfg;
    s32 s32Handler;
    u32 i;
    DrvSclOsMemset(&sthvspformCfg,0,sizeof(DrvSclHvspInformConfig_t));
    pCtxCfg = MDrvSclCtxGetCtx(u8Inst);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if((s32Handler&HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX)
            {
                break;
            }
        }
        sthvspformCfg.pvCtx = &(pCtxCfg->stCtx);
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        DrvSclHvspGetHvspAttribute(enHVSP_ID,&sthvspformCfg);
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        *str += DrvSclOsScnprintf(*str, *end - *str, "Inst  :%hhx\n",u8Inst);
        *str += DrvSclOsScnprintf(*str, *end - *str, "H en  :%hhx\n",sthvspformCfg.bEn&0x1);
        *str += DrvSclOsScnprintf(*str, *end - *str, "H function  :%hhx\n",(sthvspformCfg.bEn&0xC0)>>6);
        *str += DrvSclOsScnprintf(*str, *end - *str, "V en  :%hhx\n",(sthvspformCfg.bEn&0x2)>>1);
        *str += DrvSclOsScnprintf(*str, *end - *str, "V function  :%hhx\n",(sthvspformCfg.bEn&0x30)>>4);
        *str += DrvSclOsScnprintf(*str, *end - *str, "------------------------SCL IQ----------------------\n");
    }

}
ssize_t MDrvSclHvspScIqShow(char *buf, MDrvSclHvspIdType_e enHVSP_ID)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    u32 i;
    str += DrvSclOsScnprintf(str, end - str, "------------------------SCL IQ----------------------\n");
    if(gu8LevelInst==0xFF)
    {
        for(i=0;i<MDRV_SCL_CTX_INSTANT_MAX;i++)
        {
            _MDrvSclHvspScIqShow(i,&str,&end,enHVSP_ID);
        }
    }
    else
    {
        _MDrvSclHvspScIqShow(gu8LevelInst,&str,&end,enHVSP_ID);
    }
    str += DrvSclOsScnprintf(str, end - str, "SC H   :1~6\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 0   :1\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 1   :2\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 2   :3\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 3   :4\n");
    str += DrvSclOsScnprintf(str, end - str, "SC bypass    :5\n");
    str += DrvSclOsScnprintf(str, end - str, "SC bilinear  :6\n");
    str += DrvSclOsScnprintf(str, end - str, "SC V :7~C\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 0   :7\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 1   :8\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 2   :9\n");
    str += DrvSclOsScnprintf(str, end - str, "SC table 3   :A\n");
    str += DrvSclOsScnprintf(str, end - str, "SC bypass    :B\n");
    str += DrvSclOsScnprintf(str, end - str, "SC bilinear  :C\n");
    str += DrvSclOsScnprintf(str, end - str, "------------------------SCL IQ----------------------\n");
    end = end;
    return (str - buf);
}
ssize_t MDrvSclHvspLockShow(char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    str += DrvSclOsScnprintf(str, end - str, "------------------------SCL Lock----------------------------\n");
    str = DrvSclOsCheckMutex(str,end);
    str += DrvSclOsScnprintf(str, end - str, "------------------------SCL Lock----------------------------\n");
    end = end;
    return (str - buf);
}
#endif
bool MDrvSclHvspVtrackEnable( u8 u8FrameRate, MDrvSclHvspVtrackEnableType_e bEnable)
{
    DrvSclHvspVtrackEnable(u8FrameRate, bEnable);
    return 1;
}
bool MDrvSclHvspVtrackSetPayloadData(u16 u16Timecode, u8 u8OperatorID)
{
    DrvSclHvspVtrackSetPayloadData(u16Timecode,u8OperatorID);
    return 1;
}
bool MDrvSclHvspVtrackSetKey(bool bUserDefinded, u8 *pu8Setting)
{
    DrvSclHvspVtrackSetKey(bUserDefinded,pu8Setting);
    return 1;
}

bool MDrvSclHvspVtrackSetUserDefindedSetting(bool bUserDefinded, u8 *pu8Setting)
{
    DrvSclHvspVtrackSetUserDefindedSetting(bUserDefinded,pu8Setting);
    return 1;
}
bool MDrvSclHvspVtrackInit(void)
{
    DrvSclHvspVtrackSetUserDefindedSetting(0,NULL);
    DrvSclHvspVtrackSetPayloadData(0,0);
    DrvSclHvspVtrackSetKey(0,NULL);
    return 1;
}
