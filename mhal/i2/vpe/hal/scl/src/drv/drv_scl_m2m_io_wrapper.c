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
#define __DRV_SCL_M2M_IO_WRAPPER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"

#include "drv_scl_verchk.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_m2m_io_st.h"
#include "drv_scl_m2m_io_wrapper.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define DRV_M2M_IO_LOCK_MUTEX(x)    DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER)
#define DRV_M2M_IO_UNLOCK_MUTEX(x)  DrvSclOsReleaseMutex(x)


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    s32 s32Id;
    DrvM2MIoIdType_e enIdType;
    MDrvSclCtxLockConfig_t *pLockCfg;
    MDrvSclCtxConfig_t  *pCmdqCtx;
    MDrvSclCtxIdType_e enCtxId;
    bool bEn[E_DRV_M2M_PORT_MAX];
}DrvM2MIoCtxConfig_t;

typedef struct
{
    s32 s32Handle;
    DrvM2MIoIdType_e enM2MId;
    DrvM2MIoCtxConfig_t stCtxCfg;
    DrvM2MIoConfig_t stPort[E_DRV_M2M_PORT_MAX];
}DrvM2MIoHandleConfig_t;



//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvM2MIoHandleConfig_t _gstM2MHandler[DRV_M2M_HANDLER_MAX];

DrvM2MIoFunctionConfig_t _gstM2MFunc;
s32 s32M2MIoHandlerMutex = -1;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

DrvM2MIoCtxConfig_t *_DrvM2MIoGetCtxConfig(s32 s32Handler)
{
    s16 i;
    s16 s16Idx = -1;
    DrvM2MIoCtxConfig_t *pCtxCfg;

    DRV_M2M_IO_LOCK_MUTEX(s32M2MIoHandlerMutex);

    for(i = 0; i < DRV_M2M_HANDLER_MAX; i++)
    {
        if(_gstM2MHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        pCtxCfg = NULL;
    }
    else
    {
        pCtxCfg = &_gstM2MHandler[i].stCtxCfg;
    }

    DRV_M2M_IO_UNLOCK_MUTEX(s32M2MIoHandlerMutex);
    return pCtxCfg;
}

bool _DrvM2MIoIsLockFree(DrvM2MIoCtxConfig_t *pstCfg)
{
    bool bLockFree = FALSE;
    bool bFound = 0;
    u8 i;

    if(pstCfg->pLockCfg->bLock == FALSE)
    {
        bLockFree = TRUE;
    }
    else
    {
        for(i=0;i< pstCfg->pLockCfg->u8IdNum; i++)
        {
            if(pstCfg->s32Id == pstCfg->pLockCfg->s32Id[i])
            {
                bFound = 1;
                break;
            }
        }

        bLockFree = bFound ? TRUE : FALSE;
    }

    return bLockFree;
}

bool _DrvM2MIoGetMdrvIdType(s32 s32Handler, MDrvSclDmaIdType_e *penM2MId, MDrvSclHvspIdType_e *penM2MHvspId)
{
    s16 i;
    s16 s16Idx = -1;
    bool bRet = TRUE;

    for(i = 0; i < DRV_M2M_HANDLER_MAX; i++)
    {
        if(_gstM2MHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        *penM2MId = E_MDRV_SCLDMA_ID_NUM;
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        switch(_gstM2MHandler[s16Idx].enM2MId)
        {
            //ToDo
            case E_DRV_M2M_IO_ID_RSC:
                *penM2MId = E_MDRV_SCLDMA_ID_3;
                *penM2MHvspId = E_MDRV_SCLHVSP_ID_3;
                break;
            case E_DRV_M2M_IO_ID_LDC:
                *penM2MId = E_MDRV_SCLDMA_ID_3;
                *penM2MHvspId = E_MDRV_SCLHVSP_ID_3;
                break;
            default:
                bRet = FALSE;
                *penM2MId = E_MDRV_SCLDMA_ID_NUM;
                *penM2MHvspId = E_MDRV_SCLHVSP_ID_MAX;
                break;
        }

    }

    return bRet;
}
bool _DrvM2MIoGetOutMdrvIdType
(DrvM2MPort_e enPort,MDrvSclDmaIdType_e *penM2MId, MDrvSclHvspIdType_e *penM2MHvspId)
{
    bool bRet = TRUE;
    switch(enPort)
    {
        //ToDo
        case E_DRV_M2M_OUTPUT_PORT0:
            *penM2MId = E_MDRV_SCLDMA_ID_1;
            *penM2MHvspId = E_MDRV_SCLHVSP_ID_1;
            break;
        case E_DRV_M2M_OUTPUT_PORT1:
            *penM2MId = E_MDRV_SCLDMA_ID_2;
            *penM2MHvspId = E_MDRV_SCLHVSP_ID_2;
            break;
        case E_DRV_M2M_OUTPUT_PORT2:
            *penM2MId = E_MDRV_SCLDMA_ID_3;
            *penM2MHvspId = E_MDRV_SCLHVSP_ID_3;
            break;
        case E_DRV_M2M_OUTPUT_PORT3:
            *penM2MId = E_MDRV_SCLDMA_ID_4;
            *penM2MHvspId = E_MDRV_SCLHVSP_ID_4;
            break;
        default:
            bRet = FALSE;
            *penM2MId = E_MDRV_SCLDMA_ID_NUM;
            *penM2MHvspId = E_MDRV_SCLHVSP_ID_MAX;
            break;
    }


    return bRet;
}
void _DrvM2MIoFillCtxConfig(s32 s32Id,DrvM2MIoConfig_t *stIODMABufferCfg)
{
    DrvSclOsMemcpy(&_gstM2MHandler[s32Id].stPort[stIODMABufferCfg->enPort],stIODMABufferCfg,sizeof(DrvM2MIoConfig_t));
}
void _DrvM2MIoFillInputConfig
(DrvM2MIoConfig_t *stIODMABufferCfg,MDrvSclHvspInputConfig_t *stInCfg)
{
    stInCfg->enColor =
    (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUV420) ? E_MDRV_SCLHVSP_COLOR_YUV420 :
    (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUVSep420) ? E_MDRV_SCLHVSP_COLOR_YUV420 :
        E_MDRV_SCLHVSP_COLOR_YUV422;
    stInCfg->enSrcType = (stIODMABufferCfg->enSrc==E_DRV_M2M_IO_SRC_DRAM_LDC) ? E_MDRV_SCLHVSP_SRC_DRAM_LDC :
                        (stIODMABufferCfg->enSrc==E_DRV_M2M_IO_SRC_DRAM_RSC) ? E_MDRV_SCLHVSP_SRC_DRAM_RSC :
                            E_MDRV_SCLHVSP_SRC_NUM;
    stInCfg->stCaptureWin.u16Height = stIODMABufferCfg->u16Height;
    stInCfg->stCaptureWin.u16Height = stIODMABufferCfg->u16Width;
}
void _DrvM2MIoFillScalingConfig
(DrvM2MIoConfig_t *stInBufferCfg,DrvM2MIoConfig_t *stIODMABufferCfg,MDrvSclHvspScalingConfig_t *stSclCfg)
{
    stSclCfg->u16Src_Width = stInBufferCfg->u16Width;
    stSclCfg->u16Src_Height = stInBufferCfg->u16Height;
    stSclCfg->u16Dsp_Width = stIODMABufferCfg->u16Width;
    stSclCfg->u16Dsp_Height = stIODMABufferCfg->u16Height;
    stSclCfg->stCropWin.bEn = 0;
    stSclCfg->stCropWin.u16Height = 0;
    stSclCfg->stCropWin.u16Width = 0;
    stSclCfg->stCropWin.u16X = 0;
    stSclCfg->stCropWin.u16Y = 0;
}

void _DrvM2MIoFillBufferConfig
(DrvM2MPort_e enPort,DrvM2MIoConfig_t *stIODMABufferCfg,MDrvSclDmaBufferConfig_t *stDMABufferCfg)
{
    stDMABufferCfg->u8Flag = 0;
    stDMABufferCfg->u16BufNum = 1;
    stDMABufferCfg->enBufMDType = (stIODMABufferCfg->enBufMDType==E_DRV_M2M_IO_BUFFER_MD_SINGLE) ?
                                E_DRV_SCLDMA_IO_BUFFER_MD_SINGLE :
                                    E_DRV_SCLDMA_IO_BUFFER_MD_NUM;
    stDMABufferCfg->enColorType = (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUV422) ?
                                E_DRV_SCLDMA_IO_COLOR_YUV422 :
                                (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUV420)?
                                E_DRV_SCLDMA_IO_COLOR_YUV420 :
                                (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUVSep422)?
                                E_DRV_SCLDMA_IO_COLOR_YUVSep422 :
                                (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YCSep422)?
                                E_DRV_SCLDMA_IO_COLOR_YCSep422 :
                                (stIODMABufferCfg->enColorType==E_DRV_M2M_IO_COLOR_YUVSep420)?
                                E_DRV_SCLDMA_IO_COLOR_YUVSep420 :E_DRV_SCLDMA_IO_COLOR_NUM;
    stDMABufferCfg->enMemType = (stIODMABufferCfg->enMemType==E_DRV_M2M_IO_MEM_FRM) ?
                                E_DRV_SCLDMA_IO_MEM_FRM :
                                (stIODMABufferCfg->enMemType==E_DRV_M2M_IO_MEM_FRM2) ?
                                E_DRV_SCLDMA_IO_MEM_FRM2 :
                                (stIODMABufferCfg->enMemType==E_DRV_M2M_IO_MEM_SNP) ?
                                E_DRV_SCLDMA_IO_MEM_SNP :
                                (stIODMABufferCfg->enMemType==E_DRV_M2M_IO_MEM_FRMR) ?
                                E_DRV_SCLDMA_IO_MEM_FRMR :
                                (stIODMABufferCfg->enMemType==E_DRV_M2M_IO_MEM_IMI) ?
                                E_DRV_SCLDMA_IO_MEM_IMI : E_DRV_SCLDMA_IO_MEM_NUM;
    stDMABufferCfg->u16Height = stIODMABufferCfg->u16Height;
    stDMABufferCfg->u16Width = stIODMABufferCfg->u16Width;
    stDMABufferCfg->bHFlip  = stIODMABufferCfg->bHFlip;
    stDMABufferCfg->bVFlip  = stIODMABufferCfg->bVFlip;
    DrvSclOsMemset(stDMABufferCfg->u32Base_Y,0,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);
    DrvSclOsMemset(stDMABufferCfg->u32Base_C,0,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);
    DrvSclOsMemset(stDMABufferCfg->u32Base_V,0,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);

}

MDrvSclDmaIdType_e _DrvM2MGetDmaId(DrvM2MIoIdType_e enM2MId)
{
    MDrvSclDmaIdType_e enDev;
    switch(enM2MId)
    {
        case E_DRV_M2M_IO_ID_LDC:
            enDev = E_MDRV_SCLDMA_ID_LDC;
            break;
        case E_DRV_M2M_IO_ID_ROT:
            enDev = E_MDRV_SCLDMA_ID_ROT;
            break;
        case E_DRV_M2M_IO_ID_RSC:
            enDev = E_MDRV_SCLDMA_ID_3;
            break;
        default:
            enDev = E_MDRV_SCLDMA_ID_NUM;
                break;
    }
    return enDev;
}
MDrvSclHvspIdType_e _DrvM2MGetHvspId(DrvM2MIoIdType_e enM2MId)
{
    MDrvSclHvspIdType_e enDev;
    switch(enM2MId)
    {
        case E_DRV_M2M_IO_ID_LDC:
            enDev = E_MDRV_SCLHVSP_ID_3;
            break;
        case E_DRV_M2M_IO_ID_ROT:
            enDev = E_MDRV_SCLHVSP_ID_1; //ToDo
            break;
        case E_DRV_M2M_IO_ID_RSC:
            enDev = E_MDRV_SCLHVSP_ID_3;
            break;
        default:
            enDev = E_MDRV_SCLHVSP_ID_MAX;
                break;
    }
    return enDev;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool _DrvM2MIoDeInit(void)
{
    DrvSclOsClearProbeInformation(E_DRV_SCLOS_INIT_M2M);
    if(s32M2MIoHandlerMutex != -1)
    {
         DrvSclOsDeleteMutex(s32M2MIoHandlerMutex);
         s32M2MIoHandlerMutex = -1;
    }
    return TRUE;
}
bool _DrvM2MIoInit(void)
{
    u16 i;

    if(s32M2MIoHandlerMutex == -1)
    {
        s32M2MIoHandlerMutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "M2M_IO", SCLOS_PROCESS_SHARED);

        if(s32M2MIoHandlerMutex == -1)
        {
            SCL_ERR("%s %d, Create Mutex Fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }
    // Handler
    for(i=0; i<DRV_M2M_HANDLER_INSTANCE_NUM; i++)
    {
        _gstM2MHandler[i].s32Handle = -1;
        _gstM2MHandler[i].enM2MId = E_DRV_M2M_IO_ID_NUM;
        _gstM2MHandler[i].stCtxCfg.s32Id = -1;
        _gstM2MHandler[i].stCtxCfg.enIdType = E_DRV_M2M_IO_ID_NUM;
        _gstM2MHandler[i].stCtxCfg.pLockCfg = NULL;
        _gstM2MHandler[i].stCtxCfg.pCmdqCtx = NULL;
        DrvSclOsMemset(_gstM2MHandler[i].stPort,0,sizeof(DrvM2MIoConfig_t)*E_DRV_M2M_PORT_MAX);
    }
    //Ctx Init
    if( MDrvSclCtxInit() == FALSE)
    {
        SCL_ERR("%s %d, Init Ctx\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    //M2M Function
    DrvSclOsMemset(&_gstM2MFunc, 0, sizeof(DrvM2MIoFunctionConfig_t));
    _gstM2MFunc.DrvM2MIoSetM2MConfig                = _DrvM2MIoSetM2MConfig;
    _gstM2MFunc.DrvM2MIoCreateInstConfig            = _DrvM2MIoCreateInstConfig;
    _gstM2MFunc.DrvM2MIoDestroyInstConfig           = _DrvM2MIoDestroyInstConfig;
    _gstM2MFunc.DrvM2MIoInstProcessConfig           = _DrvM2MIoInstProcess;

    return TRUE;
}

s32 _DrvM2MIoOpen(DrvM2MIoIdType_e enM2MId)
{
    s32 s32Handle = -1;
    s16 s16Idx = -1;
    s16 i ;
    MDrvSclCtxIdType_e enCtxId;
    DRV_M2M_IO_LOCK_MUTEX(s32M2MIoHandlerMutex);
    for(i=0; i<DRV_M2M_HANDLER_MAX; i++)
    {
        if(_gstM2MHandler[i].s32Handle == -1)
        {
            s16Idx = i;
            break;
        }
    }
    //ToDo
    enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;

    if(s16Idx == -1)
    {
        s32Handle = -1;
        SCL_ERR("[M2M]: Handler is not empyt\n");
    }
    else
    {
        s32Handle = s16Idx | DRV_M2M_HANDLER_PRE_FIX | (enM2MId<<(HANDLER_PRE_FIX_SHIFT));
        _gstM2MHandler[s16Idx].s32Handle = s32Handle ;
        _gstM2MHandler[s16Idx].enM2MId = enM2MId;
        _gstM2MHandler[s16Idx].stCtxCfg.enCtxId = enCtxId;
        _gstM2MHandler[s16Idx].stCtxCfg.enIdType = enM2MId;
        _gstM2MHandler[s16Idx].stCtxCfg.s32Id = s16Idx;
        _gstM2MHandler[s16Idx].stCtxCfg.pLockCfg = MDrvSclCtxGetLockConfig(enCtxId);
        _gstM2MHandler[s16Idx].stCtxCfg.pCmdqCtx = MDrvSclCtxGetDefaultCtx();
    }
    DRV_M2M_IO_UNLOCK_MUTEX(s32M2MIoHandlerMutex);
    return s32Handle;
}

DrvM2MIoErrType_e _DrvM2MIoRelease(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;
    DrvM2MIoErrType_e eRet = E_DRV_M2M_IO_ERR_OK;
    for(i=0; i<DRV_M2M_HANDLER_MAX; i++)
    {
        if(_gstM2MHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        SCL_DBG(SCL_DBG_LV_IOCTL(),"[DMA]   %s %d not support s32Handler:%lx\n", __FUNCTION__, __LINE__,s32Handler);
        eRet = E_DRV_M2M_IO_ERR_FAULT;
    }
    else
    {
        _gstM2MHandler[s16Idx].s32Handle = -1;
        _gstM2MHandler[s16Idx].enM2MId = E_DRV_M2M_IO_ID_NUM;
        _gstM2MHandler[s16Idx].stCtxCfg.s32Id = -1;
        _gstM2MHandler[s16Idx].stCtxCfg.enIdType = E_DRV_M2M_IO_ID_NUM;
        _gstM2MHandler[s16Idx].stCtxCfg.pCmdqCtx = NULL;
        _gstM2MHandler[s16Idx].stCtxCfg.pLockCfg = NULL;
        DrvSclOsMemset(_gstM2MHandler[s16Idx].stPort,0,sizeof(DrvM2MIoConfig_t)*E_DRV_M2M_PORT_MAX);
        eRet = E_DRV_M2M_IO_ERR_OK;
    }
    return eRet;
}
void _DrvM2MIoKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    MDrvSclCtxKeepM2MCmdqFunction(pstCmdq);
}

DrvM2MIoErrType_e _DrvM2MIoSetM2MConfig(s32 s32Handler, DrvM2MIoConfig_t *pstIoInCfg)
{
    MDrvSclDmaBufferConfig_t stDMABufferCfg;
    MDrvSclDmaBufferConfig_t stDMAOutBufferCfg;
    MDrvSclDmaIdType_e enMdrvDmaIdType;
    MDrvSclHvspIdType_e enMdrvHvspIdType;
    MDrvSclDmaIdType_e enMdrvOutDmaIdType;
    MDrvSclHvspIdType_e enMdrvOutHvspIdType;
    DrvM2MIoCtxConfig_t *pstM2MCtxCfg;
    MDrvSclHvspInputConfig_t stInCfg;
    MDrvSclHvspScalingConfig_t stSclCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stDMABufferCfg,0,sizeof(MDrvSclDmaBufferConfig_t));
    DrvSclOsMemset(&stDMAOutBufferCfg,0,sizeof(MDrvSclDmaBufferConfig_t));
    DrvSclOsMemset(&stInCfg,0,sizeof(MDrvSclHvspInputConfig_t));
    DrvSclOsMemset(&stSclCfg,0,sizeof(MDrvSclHvspScalingConfig_t));
    //for input use.
    if(_DrvM2MIoGetMdrvIdType(s32Handler, &enMdrvDmaIdType, &enMdrvHvspIdType) == FALSE)
    {
        SCL_ERR( "[M2M]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }
    if(IsMDrvM2MIdType_Max(enMdrvDmaIdType))
    {
        SCL_ERR( "[M2M]   %s %d  \n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    pstM2MCtxCfg =_DrvM2MIoGetCtxConfig(s32Handler);
    if( pstM2MCtxCfg == NULL)
    {
        SCL_ERR( "[M2M]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }
    enCtxId = pstM2MCtxCfg->enCtxId;
    _DrvM2MIoFillCtxConfig(pstM2MCtxCfg->s32Id,pstIoInCfg);
    if(pstIoInCfg->enPort == E_DRV_M2M_INPUT_PORT)
    {
        _DrvM2MIoFillInputConfig(&_gstM2MHandler[pstM2MCtxCfg->s32Id].stPort[E_DRV_M2M_INPUT_PORT],&stInCfg);
        _DrvM2MIoFillBufferConfig(E_DRV_M2M_INPUT_PORT,
            &_gstM2MHandler[pstM2MCtxCfg->s32Id].stPort[E_DRV_M2M_INPUT_PORT],&stDMABufferCfg);
    }
    else
    {
        //for Output use.
        if(_DrvM2MIoGetOutMdrvIdType(pstIoInCfg->enPort, &enMdrvOutDmaIdType, &enMdrvOutHvspIdType) == FALSE)
        {
            SCL_ERR( "[M2M]   %s %d  \n", __FUNCTION__, __LINE__);
            return E_DRV_M2M_IO_ERR_INVAL;
        }
        _DrvM2MIoFillScalingConfig(&_gstM2MHandler[pstM2MCtxCfg->s32Id].stPort[E_DRV_M2M_INPUT_PORT],
            &_gstM2MHandler[pstM2MCtxCfg->s32Id].stPort[pstIoInCfg->enPort],&stSclCfg);
        _DrvM2MIoFillBufferConfig(pstIoInCfg->enPort,
            &_gstM2MHandler[pstM2MCtxCfg->s32Id].stPort[pstIoInCfg->enPort],&stDMAOutBufferCfg);
    }
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    stInCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    stSclCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    stDMABufferCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    stDMAOutBufferCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    if(pstIoInCfg->enPort == E_DRV_M2M_INPUT_PORT)
    {
        if(!MDrvSclHvspSetInputConfig(enMdrvHvspIdType,  &stInCfg))
        {
            MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
            return E_DRV_M2M_IO_ERR_FAULT;
        }
        if(!MDrvSclDmaSetDmaReadClientConfig(enMdrvDmaIdType,  &stDMABufferCfg))
        {
            MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
            return E_DRV_M2M_IO_ERR_FAULT;
        }
    }
    else
    {
        if(!MDrvSclHvspSetScalingConfig(enMdrvOutHvspIdType,  &stSclCfg ))
        {
            MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
            return E_DRV_M2M_IO_ERR_FAULT;
        }
        if(!MDrvSclDmaSetDmaWriteClientConfig(enMdrvOutDmaIdType,  &stDMAOutBufferCfg))
        {
            MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
            return E_DRV_M2M_IO_ERR_FAULT;
        }
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_M2M_IO_ERR_OK;
}

DrvM2MIoErrType_e _DrvM2MIoCreateInstConfig(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg)
{
    DrvM2MIoCtxConfig_t *pstM2MCtxCfg;
    MDrvSclCtxInstConfig_t stCtxInst;
    DrvSclOsMemset(&stCtxInst,0,sizeof(MDrvSclCtxInstConfig_t));

    pstM2MCtxCfg =_DrvM2MIoGetCtxConfig(s32Handler);

    if( pstM2MCtxCfg == NULL)
    {
        SCL_ERR( "[M2M]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    if(pstM2MCtxCfg->enIdType != E_DRV_M2M_IO_ID_RSC)
    {
        SCL_ERR( "[M2M]   %s  %d, Not Support ID\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[M2M]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    DRV_M2M_IO_LOCK_MUTEX(s32M2MIoHandlerMutex);
    //alloc Ctx handler
    stCtxInst.ps32IdBuf = pstIoInCfg->ps32IdBuf;
    stCtxInst.u8IdNum = pstIoInCfg->u8BufSize;
    pstM2MCtxCfg->pCmdqCtx = MDrvSclCtxAllocate(E_MDRV_SCL_CTX_ID_SC_ALL,&stCtxInst);
    //alloc DMA Ctx
    if(pstM2MCtxCfg->pCmdqCtx == NULL)
    {
        SCL_ERR("%s %d::Allocate Ctx Fail\n", __FUNCTION__, __LINE__);
        DRV_M2M_IO_UNLOCK_MUTEX(s32M2MIoHandlerMutex);

        return E_DRV_M2M_IO_ERR_FAULT;
    }



    DRV_M2M_IO_UNLOCK_MUTEX(s32M2MIoHandlerMutex);

    return E_DRV_M2M_IO_ERR_OK;
}
DrvM2MIoErrType_e _DrvM2MIoDestroyInstConfig(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg)
{
    DrvM2MIoCtxConfig_t *pstM2MCtxCfg;
    pstM2MCtxCfg =_DrvM2MIoGetCtxConfig(s32Handler);

    if( pstM2MCtxCfg == NULL)
    {
        SCL_ERR( "[M2M]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    if(pstM2MCtxCfg->enIdType != E_DRV_M2M_IO_ID_RSC)
    {
        SCL_ERR( "[M2M]   %s  %d, Not Support ID\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[M2M]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }

    DRV_M2M_IO_LOCK_MUTEX(s32M2MIoHandlerMutex);
    //free Ctx handler
    MDrvSclCtxFree(pstM2MCtxCfg->pCmdqCtx);
    //free DMA Ctx

    DRV_M2M_IO_UNLOCK_MUTEX(s32M2MIoHandlerMutex);

    return E_DRV_M2M_IO_ERR_OK;
}
void _DrvM2MIoInstFillProcessCfg
(MDrvSclDmaProcessConfig_t *stProcess, DrvM2MIoProcessConfig_t *pstIoInCfg,DrvM2MIoCtxConfig_t *pstM2MCtxCfg)
{
    u8 j;
    pstM2MCtxCfg->bEn[pstIoInCfg->enPort] = pstIoInCfg->stCfg.bEn;
    stProcess->stCfg.bEn = pstIoInCfg->stCfg.bEn;
    stProcess->stCfg.enMemType = pstIoInCfg->stCfg.enMemType;
    for(j =0;j<2;j++)
    {
        stProcess->stCfg.stBufferInfo.u64PhyAddr[j]=
            pstIoInCfg->stCfg.stBufferInfo.u64PhyAddr[j];
        stProcess->stCfg.stBufferInfo.u32Stride[j]=
            pstIoInCfg->stCfg.stBufferInfo.u32Stride[j];
    }
}

DrvM2MIoErrType_e _DrvM2MIoInstProcess(s32 s32Handler, DrvM2MIoProcessConfig_t *pstIoInCfg)
{
    DrvM2MIoCtxConfig_t *pstM2MCtxCfg;
    MDrvSclDmaProcessConfig_t stProcess;
    MDrvSclDmaIdType_e enMdrvDmaIdType;
    MDrvSclHvspIdType_e enMdrvHvspIdType;
    DrvM2MIoErrType_e eRet = E_DRV_M2M_IO_ERR_OK;
    DrvSclOsMemset(&stProcess,0,sizeof(MDrvSclDmaProcessConfig_t));
    if(pstIoInCfg->enPort == E_DRV_M2M_INPUT_PORT)
    {
        if(_DrvM2MIoGetMdrvIdType(s32Handler, &enMdrvDmaIdType,&enMdrvHvspIdType) == FALSE)
        {
            SCL_ERR( "[M2M]   %s %d  \n", __FUNCTION__, __LINE__);
            return E_DRV_M2M_IO_ERR_INVAL;
        }
    }
    else
    {
        if(_DrvM2MIoGetOutMdrvIdType(pstIoInCfg->enPort, &enMdrvDmaIdType,&enMdrvHvspIdType) == FALSE)
        {
            SCL_ERR( "[M2M]   %s %d  \n", __FUNCTION__, __LINE__);
            return E_DRV_M2M_IO_ERR_INVAL;
        }
    }
    pstM2MCtxCfg =_DrvM2MIoGetCtxConfig(s32Handler);

    if( pstM2MCtxCfg == NULL)
    {
        SCL_ERR( "[M2M]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_M2M_IO_ERR_INVAL;
    }
    MDrvSclCtxSetLockConfig(s32Handler,pstM2MCtxCfg->enCtxId);
    _DrvM2MIoInstFillProcessCfg(&stProcess,pstIoInCfg,pstM2MCtxCfg);
    stProcess.pvCtx = (void *)MDrvSclCtxGetConfigCtx(pstM2MCtxCfg->enCtxId);
    MDrvSclDmaInstProcess(enMdrvDmaIdType,&stProcess);
    MDrvSclCtxSetUnlockConfig(s32Handler,pstM2MCtxCfg->enCtxId);
    return eRet;
}
DrvM2MIoErrType_e _DrvM2MIoInstFlip(s32 s32Handler)
{
    MDrvSclCtxCmdqConfig_t *pvCtx;
    DrvM2MIoCtxConfig_t *pstM2MCtxCfg;
    DrvM2MIoErrType_e eRet = E_DRV_M2M_IO_ERR_OK;
    bool bFireMload = 0;
    MDrvSclCtxSetLockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
    pvCtx = MDrvSclCtxGetConfigCtx(E_MDRV_SCL_CTX_ID_SC_ALL);
    pstM2MCtxCfg =_DrvM2MIoGetCtxConfig(s32Handler);
    MDrvSclCtxFireM2M(pvCtx);
    if(pstM2MCtxCfg->bEn[E_DRV_M2M_OUTPUT_PORT0])
    {
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC1,pvCtx);
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC1,pvCtx);
    }
    if(pstM2MCtxCfg->bEn[E_DRV_M2M_OUTPUT_PORT1])
    {
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC2,pvCtx);
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC2,pvCtx);
    }
    if(pstM2MCtxCfg->bEn[E_DRV_M2M_OUTPUT_PORT2])
    {
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC3,pvCtx);
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC3,pvCtx);
    }
    if(pstM2MCtxCfg->bEn[E_DRV_M2M_OUTPUT_PORT3])
    {
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC4,pvCtx);
        bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC4,pvCtx);
    }
    MDrvSclCtxFireMload(bFireMload,pvCtx);
    MDrvSclCtxTriggerM2M(1,pvCtx);
    MDrvSclCtxSetUnlockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
    return eRet;
}
