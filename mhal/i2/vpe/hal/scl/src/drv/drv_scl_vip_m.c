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
#define _MDRV_VIP_C

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_vip.h"
#include "drv_scl_pq_define.h"
#include "drv_scl_pq_declare.h"
#include "drv_scl_pq.h"

#include "drv_scl_vip_m_st.h"
#include "drv_scl_vip_m.h"
#include "drv_scl_irq_st.h"
#include "drv_scl_irq.h"
#include "hal_scl_reg.h"

#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define MDrvSclCmdqMutexLock(enIP,bEn)          //(bEn ? DrvSclCmdqGetModuleMutex(enIP,1) : 0)
#define MDrvSclCmdqMutexUnlock(enIP,bEn)        //(bEn ? DrvSclCmdqGetModuleMutex(enIP,0) : 0)
#define MDrvSclVipMutexLock()                   //DrvSclOsObtainMutex(_MVIP_Mutex,SCLOS_WAIT_FOREVER)
#define MDrvSclVipMutexUnlock()                 //DrvSclOsReleaseMutex(_MVIP_Mutex)

#define _IsFrameBufferAllocatedReady()          (DrvSclOsGetSclFrameBufferAlloced() &(E_DRV_SCLOS_FBALLOCED_YCM))
#define AIPOffset                               PQ_IP_YEE_Main
#define _GetAipOffset(u32Type)                  (u32Type +AIPOffset)
#define MDRV_SCLVIP_WDR_MLOAD_SIZE 2048 //256pack x 8byte
#define AIPBUFFERSIZE 4096

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MDrvSclCtxMvipGlobalSet_t* gstGlobalMVipSet;
//keep
s32 _MVIP_Mutex = -1;
bool gbMultiSensor = 0;
void *gvAipBuffer = NULL;
void *gvAipStSize = NULL;
u32 gu32AipSize = AIPBUFFERSIZE;
//-------------------------------------------------------------------------------------------------
//  Local Function
//-------------------------------------------------------------------------------------------------
void _MDrvSclVipSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstGlobalMVipSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stMvipCfg);
}
u16 _MDrvSclVipCioceStructSizeFromPqType(u8 u8PQIPIdx)
{
    u16 u16DataSize;
    switch(u8PQIPIdx)
    {
        case PQ_IP_NLM_Main:
            u16DataSize = PQ_IP_NLM_Size;
            break;
        case PQ_IP_422to444_Main:
            u16DataSize = PQ_IP_422to444_Size;
            break;
        case PQ_IP_VIP_Main:
            u16DataSize = PQ_IP_VIP_Size;
            break;
        case PQ_IP_VIP_LineBuffer_Main:
            u16DataSize = PQ_IP_VIP_LineBuffer_Size;
            break;
        case PQ_IP_VIP_HLPF_Main:
            u16DataSize = PQ_IP_VIP_HLPF_Size;
            break;
        case PQ_IP_VIP_HLPF_dither_Main:
            u16DataSize = PQ_IP_VIP_HLPF_dither_Size;
            break;
        case PQ_IP_VIP_VLPF_coef1_Main:
        case PQ_IP_VIP_VLPF_coef2_Main:
            u16DataSize = PQ_IP_VIP_VLPF_coef1_Size;
            break;
        case PQ_IP_VIP_VLPF_dither_Main:
            u16DataSize = PQ_IP_VIP_VLPF_dither_Size;
            break;
        case PQ_IP_VIP_Peaking_Main:
            u16DataSize = PQ_IP_VIP_Peaking_Size;
            break;
        case PQ_IP_VIP_Peaking_band_Main:
            u16DataSize = PQ_IP_VIP_Peaking_band_Size;
            break;
        case PQ_IP_VIP_Peaking_adptive_Main:
            u16DataSize = PQ_IP_VIP_Peaking_adptive_Size;
            break;
        case PQ_IP_VIP_Peaking_Pcoring_Main:
            u16DataSize = PQ_IP_VIP_Peaking_Pcoring_Size;
            break;
        case PQ_IP_VIP_Peaking_Pcoring_ad_Y_Main:
            u16DataSize = PQ_IP_VIP_Peaking_Pcoring_ad_Y_Size;
            break;
        case PQ_IP_VIP_Peaking_gain_Main:
            u16DataSize = PQ_IP_VIP_Peaking_gain_Size;
            break;
        case PQ_IP_VIP_Peaking_gain_ad_Y_Main:
            u16DataSize = PQ_IP_VIP_Peaking_gain_ad_Y_Size;
            break;
        case PQ_IP_VIP_YC_gain_offset_Main:
            u16DataSize = PQ_IP_VIP_YC_gain_offset_Size;
            break;
        case PQ_IP_VIP_LCE_Main:
            u16DataSize = PQ_IP_VIP_LCE_Size;
            break;
        case PQ_IP_VIP_LCE_dither_Main:
            u16DataSize = PQ_IP_VIP_LCE_dither_Size;
            break;
        case PQ_IP_VIP_LCE_setting_Main:
            u16DataSize = PQ_IP_VIP_LCE_setting_Size;
            break;
        case PQ_IP_VIP_LCE_curve_Main:
            u16DataSize = PQ_IP_VIP_LCE_curve_Size;
            break;
        case PQ_IP_VIP_DLC_Main:
            u16DataSize = PQ_IP_VIP_DLC_Size;
            break;
        case PQ_IP_VIP_DLC_dither_Main:
            u16DataSize = PQ_IP_VIP_DLC_dither_Size;
            break;
        case PQ_IP_VIP_DLC_His_range_Main:
            u16DataSize = PQ_IP_VIP_DLC_His_range_Size;
            break;
        case PQ_IP_VIP_DLC_His_rangeH_Main:
            u16DataSize = PQ_IP_VIP_DLC_His_rangeH_Size;
            break;
        case PQ_IP_VIP_DLC_His_rangeV_Main:
            u16DataSize = PQ_IP_VIP_DLC_His_rangeV_Size;
            break;
        case PQ_IP_VIP_DLC_PC_Main:
            u16DataSize = PQ_IP_VIP_DLC_PC_Size;
            break;
        case PQ_IP_VIP_UVC_Main:
            u16DataSize = PQ_IP_VIP_UVC_Size;
            break;
        case PQ_IP_VIP_FCC_full_range_Main:
            u16DataSize = PQ_IP_VIP_FCC_full_range_Size;
            break;
        case PQ_IP_VIP_FCC_T1_Main:
        case PQ_IP_VIP_FCC_T2_Main:
        case PQ_IP_VIP_FCC_T3_Main:
        case PQ_IP_VIP_FCC_T4_Main:
        case PQ_IP_VIP_FCC_T5_Main:
        case PQ_IP_VIP_FCC_T6_Main:
        case PQ_IP_VIP_FCC_T7_Main:
        case PQ_IP_VIP_FCC_T8_Main:
            u16DataSize = PQ_IP_VIP_FCC_T1_Size;
            break;
        case PQ_IP_VIP_FCC_T9_Main:
            u16DataSize = PQ_IP_VIP_FCC_T9_Size;
            break;
        case PQ_IP_VIP_IHC_Main:
            u16DataSize = PQ_IP_VIP_IHC_Size;
            break;
        case PQ_IP_VIP_IHC_Ymode_Main:
            u16DataSize = PQ_IP_VIP_IHC_Ymode_Size;
            break;
        case PQ_IP_VIP_IHC_dither_Main:
            u16DataSize = PQ_IP_VIP_IHC_dither_Size;
            break;
        case PQ_IP_VIP_IHC_SETTING_Main:
            u16DataSize = PQ_IP_VIP_IHC_SETTING_Size;
            break;
        case PQ_IP_VIP_ICC_Main:
            u16DataSize = PQ_IP_VIP_ICC_Size;
            break;
        case PQ_IP_VIP_ICC_Ymode_Main:
            u16DataSize = PQ_IP_VIP_ICC_Ymode_Size;
            break;
        case PQ_IP_VIP_ICC_dither_Main:
            u16DataSize = PQ_IP_VIP_ICC_dither_Size;
            break;
        case PQ_IP_VIP_ICC_SETTING_Main:
            u16DataSize = PQ_IP_VIP_ICC_SETTING_Size;
            break;
        case PQ_IP_VIP_Ymode_Yvalue_ALL_Main:
            u16DataSize = PQ_IP_VIP_Ymode_Yvalue_ALL_Size;
            break;
        case PQ_IP_VIP_Ymode_Yvalue_SETTING_Main:
            u16DataSize = PQ_IP_VIP_Ymode_Yvalue_SETTING_Size;
            break;
        case PQ_IP_VIP_IBC_Main:
            u16DataSize = PQ_IP_VIP_IBC_Size;
            break;
        case PQ_IP_VIP_IBC_dither_Main:
            u16DataSize = PQ_IP_VIP_IBC_dither_Size;
            break;
        case PQ_IP_VIP_IBC_SETTING_Main:
            u16DataSize = PQ_IP_VIP_IBC_SETTING_Size;
            break;
        case PQ_IP_VIP_ACK_Main:
            u16DataSize = PQ_IP_VIP_ACK_Size;
            break;
        case PQ_IP_VIP_YCbCr_Clip_Main:
            u16DataSize = PQ_IP_VIP_YCbCr_Clip_Size;
            break;
        default:
            u16DataSize = 0;

            break;
    }
    return u16DataSize;
}
void _MDrvSclVipFillPqCfgByType(u8 u8PQIPIdx, u8 *pData, u16 u16DataSize,MDrvSclVipSetPqConfig_t *stSetPQCfg)
{
    stSetPQCfg->enPQIPType    = u8PQIPIdx;
    stSetPQCfg->pPointToCfg   = pData;
    stSetPQCfg->u32StructSize = u16DataSize;
}


void _MDrvSclVipSetPqParameter(MDrvSclVipSetPqConfig_t *stSetBypassCfg)
{
    MDrv_Scl_PQ_LoadSettingByData(0,stSetBypassCfg->enPQIPType,stSetBypassCfg->pPointToCfg,stSetBypassCfg->u32StructSize);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MVIP]enPQIPType:%ld,u32StructSize:%ld\n"
        ,stSetBypassCfg->enPQIPType,stSetBypassCfg->u32StructSize);
}
void _MDrvSclVipSetPqByType(u8 u8PQIPIdx, u8 *pData)
{
    MDrvSclVipSetPqConfig_t stSetPQCfg;
    u16 u16Structsize;
    u16 *p16StructSize = gvAipStSize;
    DrvSclOsMemset(&stSetPQCfg,0,sizeof(MDrvSclVipSetPqConfig_t));
    if(u8PQIPIdx>=AIPOffset )
    {
        u16Structsize = p16StructSize[u8PQIPIdx-AIPOffset];
    }
    else if(u8PQIPIdx==PQ_IP_MCNR_Main)
    {
        u16Structsize = p16StructSize[E_MDRV_SCLVIP_AIP_NUM];
    }
    else
    {
        u16Structsize = _MDrvSclVipCioceStructSizeFromPqType(u8PQIPIdx);
    }
    _MDrvSclVipFillPqCfgByType(u8PQIPIdx,pData,u16Structsize,&stSetPQCfg);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MVIP]Struct size:%hd,%hd\n"
        ,u16Structsize,MDrv_Scl_PQ_GetIPRegCount(u8PQIPIdx));
    _MDrvSclVipSetPqParameter(&stSetPQCfg);
}
void _MDrvSclVipSetMcnr(void *pstCfg)
{
    _MDrvSclVipSetPqByType(PQ_IP_MCNR_Main,(u8 *)pstCfg);
}


bool _MDrvSclVipForSetEachPqTypeByIp
    (u8 u8FirstType,u8 u8LastType,u8 ** pPointToData)
{
    u8 u8PQType;
    bool bRet = 0;
    for(u8PQType = u8FirstType;u8PQType<=u8LastType;u8PQType++)
    {
        _MDrvSclVipSetPqByType(u8PQType,pPointToData[u8PQType-u8FirstType]);
    }
    return bRet;
}
u32 _MDrvSclVipFillSettingBuffer(u16 u16PQIPIdx, u32 u32pstViraddr, u16 u16StructSize)
{
    u32 u32Viraddr;
    if(gu32AipSize >= u16StructSize)
    {
        u32Viraddr = (u32)gvAipBuffer;
    }
    else
    {
        if(gvAipBuffer)
        {
            DrvSclOsMemFree(gvAipBuffer);
            gvAipBuffer = NULL;
            gu32AipSize = u16StructSize;
        }
        gvAipBuffer = DrvSclOsMemalloc(gu32AipSize,GFP_KERNEL);
        u32Viraddr = (u32)gvAipBuffer;
    }
    if(!u32Viraddr)
    {
        SCL_ERR("[MDRVIP]%s(%d) Init *u32gpViraddr Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }
    DrvSclOsMemset((void *)u32Viraddr,0,u16StructSize);
    if(DrvSclOsCopyFromUser((void *)u32Viraddr, (void *)u32pstViraddr, u16StructSize))
    {
        DrvSclOsMemcpy((void *)u32Viraddr, (void *)u32pstViraddr, u16StructSize);
    }
    return u32Viraddr;
}
void _MDrvSclVipAipSettingDebugMessage
    (MDrvSclVipAipConfig_t *stCfg,void * pvPQSetParameter,u16 u16StructSize)
{
#if defined(SCLOS_TYPE_LINUX_DEBUG)
    u16 u16AIPsheet;
    u8 word1,word2;
    u16 u16idx;
    u16AIPsheet = _GetAipOffset(stCfg->u16AIPType);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]Sheet:%hd,size:%hd\n",u16AIPsheet,u16StructSize);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]copy addr:%lx,vir addr:%lx\n"
        ,(u32)pvPQSetParameter,(u32)stCfg->u32Viraddr);
    if(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG)
    {
        for(u16idx =0 ;u16idx<u16StructSize;u16idx++)
        {
            word1 = *((u8 *)pvPQSetParameter+u16idx);
            word2 = *((u8 *)stCfg->u32Viraddr+u16idx);
            SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]idx :%hd, copy value:%hhx,vir value:%hhx\n"
                ,u16idx,(u8)(word1),(u8)(word2));
        }
    }
#endif
}
void _MDrvSclVipAipSramSettingDebugMessage
    (MDrvSclVipAipSramConfig_t *stCfg,void * pvPQSetParameter,u16 u16StructSize)
{
#if defined(SCLOS_TYPE_LINUX_DEBUG)
    u8 word1,word2;
    u16 u16idx;
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]%s\n",__FUNCTION__);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]enType:%hd,size:%hd\n",stCfg->enAIPType,u16StructSize);
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]copy addr:%lx,vir addr:%lx\n"
        ,(u32)pvPQSetParameter,(u32)stCfg->u32Viraddr);
    if(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG)
    {
        for(u16idx =0 ;u16idx<u16StructSize;u16idx++)
        {
            word1 = *((u8 *)pvPQSetParameter+u16idx);
            word2 = *((u8 *)stCfg->u32Viraddr+u16idx);
            SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_VIPLOG, "[MDRVVIP]idx :%hd, copy value:%hhx,vir value:%hhx\n"
                ,u16idx,(u8)(word1),(u8)(word2));
        }
    }
#endif
}
u16 _MDrvSclVipGetSramBufferSize(MDrvSclVipAipSramType_e enAIPType)
{
    u16 u16StructSize;
    switch(enAIPType)
    {
        case E_MDRV_SCLVIP_AIP_SRAM_WDR_TBL:
            u16StructSize = (33*2*4)+33;
            break;
        default:
            u16StructSize = 0;
            break;
    }
    return u16StructSize;
}
void _MDrvSclVipMemNaming(char *pstIqName,s16 s16Idx)
{
    char KEY_DMEM_IQ_HIST[20] = "IQWDR";
    pstIqName[0] = 48+(((s16Idx&0xFFF)%1000)/100);
    pstIqName[1] = 48+(((s16Idx&0xFFF)%100)/10);
    pstIqName[2] = 48+(((s16Idx&0xFFF)%10));
    pstIqName[3] = '_';
    pstIqName[4] = '\0';
    DrvSclOsStrcat(pstIqName,KEY_DMEM_IQ_HIST);
}
void _MDrvSclVipFillIpmStruct(MDrvSclVipIpmConfig_t *pCfg ,DrvSclVipIpmConfig_t *stIPMCfg)
{
    stIPMCfg->bYCMRead       = (bool)((pCfg->enRW)&E_MDRV_SCLHVSP_MCNR_YCM_R);
    stIPMCfg->bYCMWrite      = (bool)(((pCfg->enRW)&E_MDRV_SCLHVSP_MCNR_YCM_W)>>1);
    stIPMCfg->u32YCBaseAddr    = pCfg->u32YCMPhyAddr;
    stIPMCfg->enCeType = (DrvSclVipUCMCEType_e)pCfg->enCeType;
    stIPMCfg->u32MemSize = pCfg->u32MemSize;
    stIPMCfg->pvCtx          = pCfg->pvCtx;
}

//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

void MDrvSclVipResume(MDrvSclVipInitConfig_t *pCfg)
{
    void *pvCtx;
    pvCtx = pCfg->pvCtx;
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    DrvSclVipHWInit();
}

bool MDrvSclVipInit(MDrvSclVipInitConfig_t *pCfg)
{
    DrvSclVipInitConfig_t stIniCfg;
    MS_PQ_Init_Info    stPQInitInfo;
    bool      ret = FALSE;
    char word[] = {"_MVIP_Mutex"};
    DrvSclOsMemset(&stPQInitInfo,0,sizeof(MS_PQ_Init_Info));
    DrvSclOsMemset(&stIniCfg,0,sizeof(DrvSclVipInitConfig_t));
    _MVIP_Mutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, word, SCLOS_PROCESS_SHARED);
    if (_MVIP_Mutex == -1)
    {
        SCL_ERR("[MDRVVIP]%s create mutex fail\n", __FUNCTION__);
        return FALSE;
    }
    stIniCfg.u32RiuBase         = pCfg->u32RiuBase;
    stIniCfg.pvCtx              = pCfg->pvCtx;
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)stIniCfg.pvCtx);
    if(DrvSclVipInit(&stIniCfg) == 0)
    {
        SCL_ERR( "[MDRVVIP]%s, Fail\n", __FUNCTION__);
        return FALSE;
    }
    MDrv_Scl_PQ_init_RIU(pCfg->u32RiuBase);
    DrvSclOsMemset(&stPQInitInfo, 0, sizeof(MS_PQ_Init_Info));
    gvAipBuffer = DrvSclOsMemalloc(gu32AipSize,GFP_KERNEL);
    gvAipStSize = DrvSclOsMemalloc((E_MDRV_SCLVIP_AIP_NUM+1)*sizeof(u16),GFP_KERNEL);// +1for mcnr use
    DrvSclOsMemset(gvAipStSize,0,((E_MDRV_SCLVIP_AIP_NUM+1)*sizeof(u16)));
    // Init PQ
    stPQInitInfo.u16PnlWidth    = 1920;
    stPQInitInfo.u8PQBinCnt     = 0;
    stPQInitInfo.u8PQTextBinCnt = 0;
    if(MDrv_Scl_PQ_Init(&stPQInitInfo))
    {
        MDrv_Scl_PQ_DesideSrcType(PQ_MAIN_WINDOW, PQ_INPUT_SOURCE_ISP);
        //ToDo: init val
        MDrv_Scl_PQ_LoadSettings(PQ_MAIN_WINDOW);
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    DrvSclVipHWInit();
    return ret;
}
void MDrvSclVipDelete(bool bCloseISR)
{
    if (_MVIP_Mutex != -1)
    {
        DrvSclOsDeleteMutex(_MVIP_Mutex);
        _MVIP_Mutex = -1;
    }
    if(bCloseISR)
    {
        DrvSclIrqExit();
    }
    if(gvAipBuffer)
    {
        DrvSclOsMemFree(gvAipBuffer);
        gvAipBuffer = NULL;
        gu32AipSize = AIPBUFFERSIZE;
    }
    if(gvAipStSize)
    {
        DrvSclOsMemFree(gvAipStSize);
        gvAipStSize = NULL;
    }
    MDrv_Scl_PQ_Exit();
    DrvSclVipExit();
}
void MDrvSclVipReSetHw(void)
{
    DrvSclOsAccessRegType_e bAccessRegMode;
    bAccessRegMode = DrvSclOsGetAccessRegMode();
    DrvSclOsSetAccessRegMode(E_DRV_SCLOS_AccessReg_CPU);
    MDrv_Scl_PQ_LoadSettings(PQ_MAIN_WINDOW);
    DrvSclOsSetAccessRegMode(bAccessRegMode);
    DrvSclVipHwReset();//hist off
}
bool MDrvSclVipSetMcnrConfig(void *pCfg)
{
    MDrvSclVipMcnrConfig_t *pstCfg = pCfg;
    void *pvCtx;
    void * pvPQSetParameter;
    u16 u16StructSize;
    u16 *p16StructSize = gvAipStSize;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
#if 1/*Tmp solution*/
    if(1)
#else
    if(_IsFrameBufferAllocatedReady())
#endif
    {
        MDrvSclVipMutexLock();
        if(p16StructSize[E_MDRV_SCLVIP_AIP_NUM]==0)
        {
            u16StructSize = MDrv_Scl_PQ_GetIPRegCount(PQ_IP_MCNR_Main);
            p16StructSize[E_MDRV_SCLVIP_AIP_NUM] = u16StructSize;
        }
        else
        {
            u16StructSize = p16StructSize[E_MDRV_SCLVIP_AIP_NUM];
        }
        pvPQSetParameter = (void *)_MDrvSclVipFillSettingBuffer(PQ_IP_MCNR_Main,pstCfg->u32Viraddr,u16StructSize);
        if(!pvPQSetParameter)
        {
            MDrvSclVipMutexUnlock();
            return 0;
        }
        _MDrvSclVipSetMcnr(pvPQSetParameter);
        MDrvSclVipMutexUnlock();
        return TRUE;
    }
    else
    {
        SCL_ERR( "[MDRVVIP]%s,MCNR buffer not alloc \n", __FUNCTION__);
        return FALSE;
    }
    return 1;
}


bool MDrvSclVipSetPeakingConfig(void *pvCfg)
{
    MDrvSclVipPeakingConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_Peaking_gain_ad_Y_Main-PQ_IP_VIP_HLPF_Main+1)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0]     = (u8*)&(pCfg->stHLPF);
    p8PQType[1]     = (u8*)&(pCfg->stHLPFDith);
    p8PQType[2]     = (u8*)&(pCfg->stVLPFcoef1);
    p8PQType[3]     = (u8*)&(pCfg->stVLPFcoef2);
    p8PQType[4]     = (u8*)&(pCfg->stVLPFDith);
    p8PQType[5]     = (u8*)&(pCfg->stOnOff);
    p8PQType[6]     = (u8*)&(pCfg->stBand);
    p8PQType[7]     = (u8*)&(pCfg->stAdp);
    p8PQType[8]     = (u8*)&(pCfg->stPcor);
    p8PQType[9]     = (u8*)&(pCfg->stAdpY);
    p8PQType[10]    = (u8*)&(pCfg->stGain);
    p8PQType[11]    = (u8*)&(pCfg->stGainAdpY);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_HLPF_Main,PQ_IP_VIP_Peaking_gain_ad_Y_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}
bool MDrvSclVipReqWdrMloadBuffer(s16 s16Idx)
{
    void *pvCtx;
    bool bRet = TRUE;
    u32 u32MiuAddr;
    char sg_Iq_Wdr_name[16];
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    _MDrvSclVipMemNaming(sg_Iq_Wdr_name,s16Idx);
    gstGlobalMVipSet->u32VirWdrMloadBuf =
        (u32)DrvSclOsDirectMemAlloc
        (sg_Iq_Wdr_name,(u32)MDRV_SCLVIP_WDR_MLOAD_SIZE,(DrvSclOsDmemBusType_t *)&gstGlobalMVipSet->u32WdrMloadBuf);
    if(gstGlobalMVipSet->u32VirWdrMloadBuf)
    {
        u32MiuAddr = gstGlobalMVipSet->u32WdrMloadBuf;
        DrvSclVipSetWdrMloadBuffer(u32MiuAddr);
        SCL_DBG(SCL_DBG_LV_VIP(),"[MDRVVIP]%s MLOAD Buffer:%lx\n", __FUNCTION__,gstGlobalMVipSet->u32VirWdrMloadBuf);
        bRet = TRUE;
    }
    else
    {
        SCL_ERR("[MDRVVIP]%s MLOAD Buffer:Allocate Fail\n", __FUNCTION__);
        bRet = FALSE;
    }
    MDrvSclVipMutexUnlock();
    return bRet;
}
bool MDrvSclVipFreeWdrMloadBuffer(s16 s16Idx)
{
    void *pvCtx;
    bool bRet = TRUE;
    char sg_Iq_Wdr_name[16];
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    if(gstGlobalMVipSet->u32VirWdrMloadBuf)
    {
        _MDrvSclVipMemNaming(sg_Iq_Wdr_name,s16Idx);
        DrvSclOsDirectMemFree
            (sg_Iq_Wdr_name,MDRV_SCLVIP_WDR_MLOAD_SIZE,
            (void *)gstGlobalMVipSet->u32VirWdrMloadBuf,(DrvSclOsDmemBusType_t)gstGlobalMVipSet->u32WdrMloadBuf);
        gstGlobalMVipSet->u32WdrMloadBuf = NULL;
        gstGlobalMVipSet->u32VirWdrMloadBuf = NULL;
        bRet = TRUE;
    }
    else
    {
        SCL_ERR("[MDRVVIP]%s MLOAD Buffer:Free Not Allocate\n", __FUNCTION__);
        bRet = FALSE;
    }
    MDrvSclVipMutexUnlock();
    return bRet;
}
bool MDrvSclVipSetMaskOnOff(void *pvCfg)
{
    MDrvSclVipSetMaskOnOff_t *pCfg = pvCfg;
    DrvSclVipSetMaskOnOff_t stCfg;
    void *pvCtx;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipSetMaskOnOff_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    DrvSclOsMemcpy(&stCfg,pCfg,sizeof(DrvSclVipSetMaskOnOff_t));
    DrvSclVipSetMaskOnOff(&stCfg);
    MDrvSclVipMutexUnlock();
    SCL_DBG(SCL_DBG_LV_VIP(),"[MDRVVIP]%s\n", __FUNCTION__);
    return TRUE;
}
bool MDrvSclVipGetWdrHistogram(void *pvCfg)
{
    MDrvSclVipWdrRoiReport_t *pCfg = pvCfg;
    DrvSclVipWdrRoiReport_t stCfg;
    void *pvCtx;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipWdrRoiReport_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    DrvSclVipGetWdrHistogram(&stCfg);
    DrvSclOsMemcpy(pCfg,&stCfg,sizeof(MDrvSclVipWdrRoiReport_t));
    MDrvSclVipMutexUnlock();
    SCL_DBG(SCL_DBG_LV_VIP(),"[MDRVVIP]%s\n", __FUNCTION__);
    return TRUE;
}
bool MDrvSclVipGetNRHistogram(void *pvCfg)
{
    DrvSclVipGetNRHistogram(pvCfg);
    return 1;
}
bool MDrvSclVipSetRoiConfig(void *pvCfg)
{
    MDrvSclVipWdrRoiHist_t *pCfg = pvCfg;
    DrvSclVipWdrRoiHist_t stCfg;
    void *pvCtx;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipWdrRoiHist_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    DrvSclOsMemcpy(&stCfg,pCfg,sizeof(DrvSclVipWdrRoiHist_t));
    DrvSclVipSetRoiConfig(&stCfg);
    MDrvSclVipMutexUnlock();
    return TRUE;
}
bool MDrvSclVipSetHistogramConfig(void *pvCfg)
{
    ST_VIP_DLC_HISTOGRAM_CONFIG stDLCCfg;
    u16 i;
    MDrvSclVipDlcHistogramConfig_t *pCfg = pvCfg;
    void *pvCtx;
    DrvSclOsMemset(&stDLCCfg,0,sizeof(ST_VIP_DLC_HISTOGRAM_CONFIG));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    stDLCCfg.bVariable_Section  = pCfg->bVariable_Section;
    stDLCCfg.bstat_MIU          = pCfg->bstat_MIU;
    stDLCCfg.bcurve_fit_en      = pCfg->bcurve_fit_en;
    stDLCCfg.bcurve_fit_rgb_en  = pCfg->bcurve_fit_rgb_en;
    stDLCCfg.bDLCdither_en      = pCfg->bDLCdither_en;
    stDLCCfg.bhis_y_rgb_mode_en = pCfg->bhis_y_rgb_mode_en;
    stDLCCfg.bstatic            = pCfg->bstatic;
    stDLCCfg.bRange             = pCfg->bRange;
    stDLCCfg.u16Vst             = pCfg->u16Vst;
    stDLCCfg.u16Hst             = pCfg->u16Hst;
    stDLCCfg.u16Vnd             = pCfg->u16Vnd;
    stDLCCfg.u16Hnd             = pCfg->u16Hnd;
    stDLCCfg.u8HistSft          = pCfg->u8HistSft;
    stDLCCfg.u8trig_ref_mode    = pCfg->u8trig_ref_mode;
    stDLCCfg.u32StatBase[0]     = pCfg->u32StatBase[0];
    stDLCCfg.u32StatBase[1]     = pCfg->u32StatBase[1];

    for(i=0;i<VIP_DLC_HISTOGRAM_SECTION_NUM;i++)
    {
        stDLCCfg.u8Histogram_Range[i] = pCfg->u8Histogram_Range[i];
    }
    MDrvSclVipMutexLock();

    if(DrvSclVipSetDlcHistogramConfig(&stDLCCfg,0) == 0)
    {
        SCL_DBG(SCL_DBG_LV_VIP(), "[MDRVVIP]%s, Fail\n", __FUNCTION__);
        MDrvSclVipMutexUnlock();
        return FALSE;
    }
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipGetDlcHistogramReport(void *pvCfg)
{
    DrvSclVipDlcHistogramReport_t stDLCCfg;
    u16 i;
    MDrvSclVipDlcHistogramReport_t *pCfg = pvCfg;
    void *pvCtx;
    DrvSclOsMemset(&stDLCCfg,0,sizeof(DrvSclVipDlcHistogramReport_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    DrvSclVipGetDlcHistogramConfig(&stDLCCfg);
    pCfg->u32PixelWeight = stDLCCfg.u32PixelWeight;
    pCfg->u32PixelCount  = stDLCCfg.u32PixelCount;
    pCfg->u8MaxPixel     = stDLCCfg.u8MaxPixel;
    pCfg->u8MinPixel     = stDLCCfg.u8MinPixel;
    pCfg->u8Baseidx      = stDLCCfg.u8Baseidx;
    for(i=0;i<VIP_DLC_HISTOGRAM_REPORT_NUM;i++)
    {
        stDLCCfg.u32Histogram[i]    = DrvSclVipGetDlcHistogramReport(i);
        pCfg->u32Histogram[i]       = stDLCCfg.u32Histogram[i];
    }

    return TRUE;
}

bool MDrvSclVipSetDlcConfig(void *pvCfg)
{
    MDrvSclVipDlcConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_YC_gain_offset_Main-PQ_IP_VIP_DLC_His_range_Main+1)];//add PQ_IP_VIP_YC_gain_offset_Main
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->sthist);
    p8PQType[1] = (u8*)&(pCfg->stEn);
    p8PQType[2] = (u8*)&(pCfg->stDither);
    p8PQType[3] = (u8*)&(pCfg->stHistH);
    p8PQType[4] = (u8*)&(pCfg->stHistV);
    p8PQType[5] = (u8*)&(pCfg->stPC);
    p8PQType[6] = (u8*)&(pCfg->stGainOffset);
    MDrvSclVipMutexLock();

    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_DLC_His_range_Main,PQ_IP_VIP_YC_gain_offset_Main,p8PQType);

    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetLceConfig(void *pvCfg)
{
    MDrvSclVipLceConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_LCE_curve_Main-PQ_IP_VIP_LCE_Main+1)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stOnOff);
    p8PQType[1] = (u8*)&(pCfg->stDITHER);
    p8PQType[2] = (u8*)&(pCfg->stSet);
    p8PQType[3] = (u8*)&(pCfg->stCurve);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_LCE_Main,PQ_IP_VIP_LCE_curve_Main,p8PQType);

    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetUvcConfig(void *pvCfg)
{
    u8 *pUVC = NULL;
    MDrvSclVipUvcConfig_t *pCfg = pvCfg;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    pUVC = (u8*)&(pCfg->stUVC);
    MDrvSclVipMutexLock();
    _MDrvSclVipSetPqByType(PQ_IP_VIP_UVC_Main,pUVC);
    MDrvSclVipMutexUnlock();
    return TRUE;
}


bool MDrvSclVipSetIhcConfig(void *pvCfg)
{
    MDrvSclVipIhcConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_IHC_dither_Main-PQ_IP_VIP_IHC_Main+2)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stOnOff);
    p8PQType[1] = (u8*)&(pCfg->stYmd);
    p8PQType[2] = (u8*)&(pCfg->stDither);
    p8PQType[3] = (u8*)&(pCfg->stset);
    MDrvSclVipMutexLock();
    _MDrvSclVipSetPqByType(PQ_IP_VIP_IHC_SETTING_Main,p8PQType[3]);
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_IHC_Main,PQ_IP_VIP_IHC_dither_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetICEConfig(void *pvCfg)
{
    MDrvSclVipIccConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_ICC_SETTING_Main-PQ_IP_VIP_ICC_dither_Main+2)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stEn);
    p8PQType[1] = (u8*)&(pCfg->stYmd);
    p8PQType[2] = (u8*)&(pCfg->stDither);
    p8PQType[3] = (u8*)&(pCfg->stSet);
    MDrvSclVipMutexLock();
    _MDrvSclVipSetPqByType(PQ_IP_VIP_ICC_SETTING_Main,p8PQType[3]);
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_ICC_Main,PQ_IP_VIP_ICC_dither_Main,p8PQType);

    MDrvSclVipMutexUnlock();
    return TRUE;
}


bool MDrvSclVipSetIhcICCADPYConfig(void *pvCfg)
{
    MDrvSclVipIhcIccConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_Ymode_Yvalue_SETTING_Main-PQ_IP_VIP_Ymode_Yvalue_ALL_Main+1)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stYmdall);
    p8PQType[1] = (u8*)&(pCfg->stYmdset);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_Ymode_Yvalue_ALL_Main,PQ_IP_VIP_Ymode_Yvalue_SETTING_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetIbcConfig(void *pvCfg)
{
    MDrvSclVipIbcConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_IBC_SETTING_Main-PQ_IP_VIP_IBC_Main+1)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stEn);
    p8PQType[1] = (u8*)&(pCfg->stDither);
    p8PQType[2] = (u8*)&(pCfg->stSet);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_IBC_Main,PQ_IP_VIP_IBC_SETTING_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetFccConfig(void *pvCfg)
{
    MDrvSclVipFccConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_FCC_T9_Main-PQ_IP_VIP_FCC_T1_Main+2)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[9] = (u8*)&(pCfg->stfr);
    p8PQType[0] = (u8*)&(pCfg->stT[0]);
    p8PQType[1] = (u8*)&(pCfg->stT[1]);
    p8PQType[2] = (u8*)&(pCfg->stT[2]);
    p8PQType[3] = (u8*)&(pCfg->stT[3]);
    p8PQType[4] = (u8*)&(pCfg->stT[4]);
    p8PQType[5] = (u8*)&(pCfg->stT[5]);
    p8PQType[6] = (u8*)&(pCfg->stT[6]);
    p8PQType[7] = (u8*)&(pCfg->stT[7]);
    p8PQType[8] = (u8*)&(pCfg->stT9);
    MDrvSclVipMutexLock();
    _MDrvSclVipSetPqByType(PQ_IP_VIP_FCC_full_range_Main,p8PQType[9]);
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_FCC_T1_Main,PQ_IP_VIP_FCC_T9_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}


bool MDrvSclVipSetAckConfig(void *pvCfg)
{
    MDrvSclVipAckConfig_t *pCfg = pvCfg;
    u8 *p8PQType[(PQ_IP_VIP_YCbCr_Clip_Main-PQ_IP_VIP_ACK_Main+1)];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->stACK);
    p8PQType[1] = (u8*)&(pCfg->stclip);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_VIP_ACK_Main,PQ_IP_VIP_YCbCr_Clip_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetNlmConfig(void *pvCfg)
{
    u8 *stNLM = NULL;
    MDrvSclVipNlmConfig_t *pCfg = pvCfg;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    stNLM  = (u8*)&(pCfg->stNLM);
    MDrvSclVipMutexLock();
    _MDrvSclVipSetPqByType(PQ_IP_NLM_Main,stNLM);
    MDrvSclVipMutexUnlock();
    return TRUE;
}

bool MDrvSclVipSetNlmSramConfig(MDrvSclVipNlmSramConfig_t stSRAM)
{
    DrvSclVipNlmSramConfig_t stCfg;
    void *pvCtx;
    DrvSclOsMemset(&stCfg,0,sizeof(DrvSclVipNlmSramConfig_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    stCfg.u32baseadr    = stSRAM.u32Baseadr;
    stCfg.bCLientEn     = stSRAM.bEn;
    stCfg.u32viradr     = stSRAM.u32viradr;
    stCfg.btrigContinue = 0;                    //single
    stCfg.u16depth      = VIP_NLM_ENTRY_NUM;    // entry
    stCfg.u16reqlen     = VIP_NLM_ENTRY_NUM;
    stCfg.u16iniaddr    = 0;
    SCL_DBG(SCL_DBG_LV_VIP(),
        "[MDRVVIP]%s, flag:%hhx ,addr:%lx,addr:%lx \n", __FUNCTION__,stSRAM.bEn,stSRAM.u32viradr,stSRAM.u32Baseadr);
    DrvSclVipSetNlmSramConfig(&stCfg);
    return TRUE;
}

bool MDrvSclVipSetVipOtherConfig(void *pvCfg)
{
    MDrvSclVipConfig_t *pCfg = pvCfg;
    u8 *p8PQType[2];
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    p8PQType[0] = (u8*)&(pCfg->st422_444);
    p8PQType[1] = (u8*)&(pCfg->stBypass);
    MDrvSclVipMutexLock();
    _MDrvSclVipForSetEachPqTypeByIp(PQ_IP_422to444_Main,PQ_IP_VIP_Main,p8PQType);
    MDrvSclVipMutexUnlock();
    return TRUE;
}


void MDrvSclVipSetMultiSensorConfig(bool bEn)
{
    MDrvSclVipMutexLock();
    if(bEn)
    {
        gbMultiSensor++;
    }
    else
    {
        if(gbMultiSensor)
        {
            gbMultiSensor--;
        }
    }
    MDrvSclVipMutexUnlock();
}

bool MDrvSclVipSetWdrMloadConfig(void)
{
    void *pvCtx;
    bool bEn = 0;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    if(DrvSclVipGetWdrOnOff())
    {
        bEn = (gbMultiSensor>1)? 1 : 0;// for >2 is real multisensor
    }
    else
    {
        bEn = 0;
    }
    DrvSclVipSetWdrMloadConfig(bEn,gstGlobalMVipSet->u32WdrMloadBuf);
    return bEn;
}
bool MDrvSclVipSetAipConfig(MDrvSclVipAipConfig_t *stCfg)
{
    void * pvPQSetParameter;
    u16 u16StructSize;
    u16 u16AIPsheet;
    u16 *p16StructSize = gvAipStSize;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    u16AIPsheet = _GetAipOffset(stCfg->u16AIPType);
    if(p16StructSize[stCfg->u16AIPType]==0)
    {
        u16StructSize = MDrv_Scl_PQ_GetIPRegCount(u16AIPsheet);
        p16StructSize[stCfg->u16AIPType] = u16StructSize;
    }
    else
    {
        u16StructSize = p16StructSize[stCfg->u16AIPType];
    }
    pvPQSetParameter = (void *)_MDrvSclVipFillSettingBuffer(u16AIPsheet,stCfg->u32Viraddr,u16StructSize);
    _MDrvSclVipAipSettingDebugMessage(stCfg,pvPQSetParameter,u16StructSize);
    _MDrvSclVipSetPqByType(u16AIPsheet,(u8 *)pvPQSetParameter);
    return 1;
}


bool MDrvSclVipSetAipSramConfig(MDrvSclVipAipSramConfig_t *stCfg)
{
    void * pvPQSetParameter;
    void * pvPQSetPara;
    u16 u16StructSize;
    DrvSclVipSramType_e enAIPType;
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    MDrvSclVipMutexLock();
    u16StructSize = _MDrvSclVipGetSramBufferSize(stCfg->enAIPType);
    if(gu32AipSize >= u16StructSize)
    {
        pvPQSetParameter = gvAipBuffer;
    }
    else
    {
        if(gvAipBuffer)
        {
            DrvSclOsMemFree(gvAipBuffer);
            gvAipBuffer = NULL;
            gu32AipSize = u16StructSize;
        }
        gvAipBuffer = DrvSclOsMemalloc(gu32AipSize,GFP_KERNEL);
        pvPQSetParameter = gvAipBuffer;
    }
    if(pvPQSetParameter==NULL)
    {
        MDrvSclVipMutexUnlock();
        return 0;
    }
    DrvSclOsMemset(pvPQSetParameter,0,u16StructSize);

    if(DrvSclOsCopyFromUser(pvPQSetParameter, (void  *)stCfg->u32Viraddr, u16StructSize))
    {
        DrvSclOsMemcpy(pvPQSetParameter, (void *)stCfg->u32Viraddr, u16StructSize);
    }
    enAIPType = stCfg->enAIPType;
    _MDrvSclVipAipSramSettingDebugMessage(stCfg,pvPQSetParameter,u16StructSize);
    // NOt need to lock CMDQ ,because hal will do it.
    pvPQSetPara = DrvSclVipSetAipSramConfig(pvPQSetParameter,enAIPType);
    MDrvSclVipMutexUnlock();
    return 1;
}
bool MDrvSclVipSetInitIpmConfig(MDrvSclVipIpmConfig_t *pCfg)
{
    DrvSclVipIpmConfig_t stIPMCfg;
    DrvSclOsMemset(&stIPMCfg,0,sizeof(DrvSclVipIpmConfig_t));
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_NORMAL, "%s:PhyAddr=%lx, width=%x, height=%x \n",  __FUNCTION__, pCfg->u32YCMPhyAddr, pCfg->u16Width, pCfg->u16Height);
    _MDrvSclVipFillIpmStruct(pCfg,&stIPMCfg);
    // ToDo
    // DrvSclVipSetPrv2CropOnOff(stIPMCfg.bYCMRead,);
    if(DrvSclVipSetIPMConfig(&stIPMCfg) == FALSE)
    {
        SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_NORMAL, "[VIP] Set IPM Config Fail\n");
        return FALSE;
    }
    else
    {
        return TRUE;;
    }
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
void MDrvSclVipFillBasicStructSetPqCfg(MDrvSclVipConfigType_e enVIPtype,void *pPointToCfg,MDrvSclVipSetPqConfig_t* stSetPQCfg)
{
    void *pvCtx;
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    _MDrvSclVipSetGlobal((MDrvSclCtxCmdqConfig_t *)pvCtx);
    stSetPQCfg->pPointToCfg = pPointToCfg;
    switch(enVIPtype)
    {
        case E_MDRV_SCLVIP_SETROI_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipWdrRoiHist_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetRoiConfig;
            break;
        case E_MDRV_SCLVIP_MCNR_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipMcnrConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetMcnrConfig;
            break;

        case E_MDRV_SCLVIP_ACK_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipAckConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetAckConfig;
            break;

        case E_MDRV_SCLVIP_IBC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipIbcConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetIbcConfig;
            break;

        case E_MDRV_SCLVIP_IHCICC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipIhcIccConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetIhcICCADPYConfig;
            break;

        case E_MDRV_SCLVIP_ICC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipIccConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetICEConfig;
            break;

        case E_MDRV_SCLVIP_IHC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipIhcConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetIhcConfig;
            break;

        case E_MDRV_SCLVIP_FCC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipFccConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetFccConfig;
            break;
        case E_MDRV_SCLVIP_UVC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipUvcConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetUvcConfig;
            break;

        case E_MDRV_SCLVIP_DLC_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipDlcConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetDlcConfig;
            break;

        case E_MDRV_SCLVIP_DLC_HISTOGRAM_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipDlcHistogramConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetHistogramConfig;
            break;

        case E_MDRV_SCLVIP_LCE_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipLceConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetLceConfig;
            break;

        case E_MDRV_SCLVIP_PEAKING_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipPeakingConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetPeakingConfig;
            break;

        case E_MDRV_SCLVIP_NLM_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipNlmConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetNlmConfig;
            break;
        case E_MDRV_SCLVIP_CONFIG:
                stSetPQCfg->u32StructSize = sizeof(MDrvSclVipConfig_t);
                stSetPQCfg->pfForSet = MDrvSclVipSetVipOtherConfig;
            break;
        default:
                stSetPQCfg->u32StructSize = 0;
                stSetPQCfg->bSetConfigFlag = 0;
                stSetPQCfg->pfForSet = NULL;
            break;

    }
}
ssize_t _MDrvSclVipVipShow(char *buf,u8 u8Inst)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    MDrvSclCtxConfig_t *pCtxCfg = NULL;
    s32 s32Handler;
    u32 i;
    DrvSclVipWdrRoiConfig_t stCfg;
    DrvSclVipWdrRoiReport_t stCfg2;
    pCtxCfg = MDrvSclCtxGetCtx(u8Inst);
    if(pCtxCfg->bUsed)
    {
        for(i=0; i<MDRV_SCL_CTX_CLIENT_ID_MAX; i++)
        {
            s32Handler = pCtxCfg->s32Id[i];
            if((s32Handler&HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX)
            {
                break;
            }
        }
        MDrvSclCtxSetLockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
        str += DrvSclOsScnprintfD(str, end - str, "========================SCL VIP STATUS======================\n");
        str += DrvSclOsScnprintfD(str, end - str, "Inst  :%hhx\n",u8Inst);
        str += DrvSclOsScnprintfD(str, end - str, "     IP      Status\n");
        str += DrvSclOsScnprintfD(str, end - str, "---------------VIP---------------------\n");
        str += DrvSclOsScnprintfD(str, end - str, "  VIP Bypass     %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_CONFIG) ? "Bypass" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     MCNR        %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_MCNR_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     NLM         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_NLM_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     ACK         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_ACK_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     IBC         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_IBC_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     ICC         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_ICC_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     IHC         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_IHC_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC1        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x1) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC2        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x2) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC3        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x4) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC4        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x8) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC5        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x10) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC6        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x20) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC7        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x40) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC8        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x80) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     FCC9        %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_FCC_CONFIG)&0x100) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     UVC         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_UVC_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, " DLC  CURVEFIT   %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_DLC_CONFIG)&0x1) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, " DLC  STATISTIC  %s\n", (DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_DLC_CONFIG)&0x2) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     LCE         %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_LCE_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "     PK          %s\n", DrvSclVipGetBypassStatus(E_MDRV_SCLVIP_PEAKING_CONFIG) ? "OFF" :"ON");
        str += DrvSclOsScnprintfD(str, end - str, "---------------AIP--------------------\n");
        str += DrvSclOsScnprintfD(str, end - str, "     EE          %s\n", (DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_YEE)&0x1) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     EYEE        %s\n", (DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_YEE)&0x2) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     YEE Merge   %s\n", (DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_YEE)&0x4) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     WDR         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_WDR) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     MXNR        %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_MXNR) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     UVByY       %s\n", (DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_UVADJ)&0x1) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     YUVADJ      %s\n", (DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_UVADJ)&0x2) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     XNR         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_XNR) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     YCUVM       %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_YCUVM) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     COLOR TRAN  %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_COLORTRAN) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     YUV GAMMA   %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_GAMMA) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "---------------COLOR ENGINE--------------------\n");
        str += DrvSclOsScnprintfD(str, end - str, "     Y2R         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_YUVTORGB) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     GM10to12    %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_GM10TO12) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     CCM         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_CCM) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     HSV         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_HSV) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     GM12to10    %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_GM12TO10) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "     R2Y         %s\n", DrvSclVipGetAIPStatus(E_MDRV_SCLVIP_AIP_RGBTOYUV) ? "ON" :"OFF");
        str += DrvSclOsScnprintfD(str, end - str, "---------------ROI--------------------\n");
        DrvSclVipGetRoiConfig(0,&stCfg);
        DrvSclVipGetWdrHistogram(&stCfg2);
        str += DrvSclOsScnprintfD(str, end - str, "     1 (%hx,%hx),(%hx,%hx),(%hx,%hx),(%hx,%hx) y:%lx\n"
        ,stCfg.u16RoiAccX[0],stCfg.u16RoiAccY[0],stCfg.u16RoiAccX[1],stCfg.u16RoiAccY[1]
        ,stCfg.u16RoiAccX[2],stCfg.u16RoiAccY[2],stCfg.u16RoiAccX[3],stCfg.u16RoiAccY[3]
        ,(stCfg2.s32Y[0]/((stCfg2.s32YCnt[0]) ?(stCfg2.s32YCnt[0]) : 1)));
        DrvSclVipGetRoiConfig(1,&stCfg);
        str += DrvSclOsScnprintfD(str, end - str, "     2 (%hx,%hx),(%hx,%hx),(%hx,%hx),(%hx,%hx) y:%lx\n"
            ,stCfg.u16RoiAccX[0],stCfg.u16RoiAccY[0],stCfg.u16RoiAccX[1],stCfg.u16RoiAccY[1]
            ,stCfg.u16RoiAccX[2],stCfg.u16RoiAccY[2],stCfg.u16RoiAccX[3],stCfg.u16RoiAccY[3]
            ,(stCfg2.s32Y[1]/((stCfg2.s32YCnt[1]) ?(stCfg2.s32YCnt[1]) : 1)));
        DrvSclVipGetRoiConfig(2,&stCfg);
        str += DrvSclOsScnprintfD(str, end - str, "     3 (%hx,%hx),(%hx,%hx),(%hx,%hx),(%hx,%hx) y:%lx\n"
            ,stCfg.u16RoiAccX[0],stCfg.u16RoiAccY[0],stCfg.u16RoiAccX[1],stCfg.u16RoiAccY[1]
            ,stCfg.u16RoiAccX[2],stCfg.u16RoiAccY[2],stCfg.u16RoiAccX[3],stCfg.u16RoiAccY[3]
            ,(stCfg2.s32Y[2]/((stCfg2.s32YCnt[2]) ?(stCfg2.s32YCnt[2]) : 1)));
        DrvSclVipGetRoiConfig(3,&stCfg);
        str += DrvSclOsScnprintfD(str, end - str, "     4 (%hx,%hx),(%hx,%hx),(%hx,%hx),(%hx,%hx) y:%lx\n"
            ,stCfg.u16RoiAccX[0],stCfg.u16RoiAccY[0],stCfg.u16RoiAccX[1],stCfg.u16RoiAccY[1]
            ,stCfg.u16RoiAccX[2],stCfg.u16RoiAccY[2],stCfg.u16RoiAccX[3],stCfg.u16RoiAccY[3]
            ,(stCfg2.s32Y[3]/((stCfg2.s32YCnt[3]) ?(stCfg2.s32YCnt[3]) : 1)));
        str += DrvSclOsScnprintfD(str, end - str, "========================SCL VIP STATUS======================\n");
        MDrvSclCtxSetUnlockConfig(s32Handler,pCtxCfg->stCtx.enCtxId);
    }
    end = end;
    return (str - buf);
}
ssize_t MDrvSclVipVipShow(char *buf)
{
    u32 i;
    char *str = buf;
    ssize_t size = 0;
    if(gu8LevelInst==0xFF)
    {
        // check all
        for(i=MDRV_SCL_CTX_INSTANT_MAX;i<MDRV_SCL_CTX_INSTANT_MAX*E_MDRV_SCL_CTX_ID_NUM;i++)
        {
            _MDrvSclVipVipShow(str,(u8)i);
        }
    }
    else if(gu8LevelInst<MDRV_SCL_CTX_INSTANT_MAX*E_MDRV_SCL_CTX_ID_NUM &&gu8LevelInst >= MDRV_SCL_CTX_INSTANT_MAX)
    {
        _MDrvSclVipVipShow(buf,gu8LevelInst);
    }
    return size;
}
#endif
