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
#define __DRV_SCL_DMA_IO_WRAPPER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_verchk.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_dma_io_wrapper.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define DRV_SCLDMA_IO_LOCK_MUTEX(x)    DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER)
#define DRV_SCLDMA_IO_UNLOCK_MUTEX(x)  DrvSclOsReleaseMutex(x)


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    s32 s32Id;
    DrvSclDmaIoIdType_e enIdType;
    MDrvSclCtxLockConfig_t *pLockCfg;
    MDrvSclCtxConfig_t  *pCmdqCtx;
    MDrvSclCtxIdType_e enCtxId;
}DrvSclDmaIoCtxConfig_t;

typedef struct
{
    s32 s32Handle;
    DrvSclDmaIoIdType_e enSclDmaId;
    DrvSclDmaIoCtxConfig_t stCtxCfg;
}DrvSclDmaIoHandleConfig_t;

typedef struct
{
    u32 u32StructSize;
    u32 *pVersion;
    u32 u32VersionSize;
}DrvSclDmaIoVersionChkConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvSclDmaIoHandleConfig_t _gstSclDmaHandler[DRV_SCLDMA_HANDLER_MAX];

DrvSclDmaIoFunctionConfig_t _gstSclDmaFunc;
s32 s32SclDmaIoHandlerMutex = -1;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void _DrvSclDmaIoFillVersionChkStruct(u32 u32StructSize,u32 u32VersionSize,u32 *pVersion,DrvSclDmaIoVersionChkConfig_t *stVersion)
{
    stVersion->u32StructSize  = (u32)u32StructSize;
    stVersion->u32VersionSize = (u32)u32VersionSize;
    stVersion->pVersion      = (u32 *)pVersion;
}

s32 _DrvSclDmaIoVersionCheck(DrvSclDmaIoVersionChkConfig_t *stVersion)
{
    if ( CHK_VERCHK_HEADER(stVersion->pVersion) )
    {
        if( CHK_VERCHK_MAJORVERSION_LESS( stVersion->pVersion, DRV_SCLDMA_VERSION) )
        {

            VERCHK_ERR("[SCLDMA] Version(%04lx) < %04x!!! \n",
                *(stVersion->pVersion) & VERCHK_VERSION_MASK,
                DRV_SCLDMA_VERSION);

            return -1;
        }
        else
        {
            if( CHK_VERCHK_SIZE( &stVersion->u32VersionSize, stVersion->u32StructSize) == 0 )
            {
                VERCHK_ERR("[SCLDMA] Size(%04lx) != %04lx!!! \n",
                    stVersion->u32StructSize,
                    stVersion->u32VersionSize);

                return -1;
            }
            else
            {
                SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_ELSE, "[SCLDMA] Size(%ld) \n",stVersion->u32StructSize );
                return 0;
            }
        }
    }
    else
    {
        VERCHK_ERR("[SCLDMA] No Header !!! \n");
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return -1;
    }
}

DrvSclDmaIoCtxConfig_t *_DrvSclDmaIoGetCtxConfig(s32 s32Handler)
{
    s16 i;
    s16 s16Idx = -1;
    DrvSclDmaIoCtxConfig_t *pCtxCfg;

    //DRV_SCLDMA_IO_LOCK_MUTEX(s32SclDmaIoHandlerMutex);

    for(i = 0; i < DRV_SCLDMA_HANDLER_MAX; i++)
    {
        if(_gstSclDmaHandler[i].s32Handle == s32Handler)
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
        pCtxCfg = &_gstSclDmaHandler[i].stCtxCfg;
    }

    //DRV_SCLDMA_IO_UNLOCK_MUTEX(s32SclDmaIoHandlerMutex);
    return pCtxCfg;
}

bool _DrvSclDmaIoGetMdrvIdType(s32 s32Handler, MDrvSclDmaIdType_e *penSclDmaId)
{
    s16 i;
    s16 s16Idx = -1;
    bool bRet = TRUE;

    for(i = 0; i < DRV_SCLDMA_HANDLER_MAX; i++)
    {
        if(_gstSclDmaHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        *penSclDmaId = E_MDRV_SCLDMA_ID_NUM;
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        switch(_gstSclDmaHandler[s16Idx].enSclDmaId)
        {
            case E_DRV_SCLDMA_IO_ID_1:
                *penSclDmaId = E_MDRV_SCLDMA_ID_1;
                break;
            case E_DRV_SCLDMA_IO_ID_2:
                *penSclDmaId = E_MDRV_SCLDMA_ID_2;
                break;
            case E_DRV_SCLDMA_IO_ID_3:
                *penSclDmaId = E_MDRV_SCLDMA_ID_3;
                break;
            case E_DRV_SCLDMA_IO_ID_4:
                *penSclDmaId = E_MDRV_SCLDMA_ID_MDWIN;
                break;
            default:
                bRet = FALSE;
                *penSclDmaId = E_MDRV_SCLDMA_ID_NUM;
                break;
        }

    }

    return bRet;
}

void _DrvSclDmaIoFillBufferConfig(DrvSclDmaIoBufferConfig_t *stIODMABufferCfg,MDrvSclDmaBufferConfig_t *stDMABufferCfg)
{
    stDMABufferCfg->u8Flag = stIODMABufferCfg->u8Flag;
    stDMABufferCfg->enBufMDType = stIODMABufferCfg->enBufMDType;
    stDMABufferCfg->enColorType = stIODMABufferCfg->enColorType;
    stDMABufferCfg->enMemType = stIODMABufferCfg->enMemType;
    stDMABufferCfg->u16BufNum = stIODMABufferCfg->u16BufNum;
    stDMABufferCfg->u16Height = stIODMABufferCfg->u16Height;
    stDMABufferCfg->u16Width = stIODMABufferCfg->u16Width;
    stDMABufferCfg->bHFlip  = stIODMABufferCfg->bHFlip;
    stDMABufferCfg->bVFlip  = stIODMABufferCfg->bVFlip;
    DrvSclOsMemcpy(stDMABufferCfg->u32Base_Y,stIODMABufferCfg->u32Base_Y,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);
    DrvSclOsMemcpy(stDMABufferCfg->u32Base_C,stIODMABufferCfg->u32Base_C,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);
    DrvSclOsMemcpy(stDMABufferCfg->u32Base_V,stIODMABufferCfg->u32Base_V,sizeof(unsigned long)*BUFFER_BE_ALLOCATED_MAX);

}
DrvSclosDevType_e _DrvSclDmaGetDevId(DrvSclDmaIoIdType_e enSclDmaId)
{
    DrvSclosDevType_e enDev;
    switch(enSclDmaId)
    {
        case E_DRV_SCLDMA_IO_ID_1:
            enDev = E_DRV_SCLOS_DEV_DMA_1;
            break;
        case E_DRV_SCLDMA_IO_ID_2:
            enDev = E_DRV_SCLOS_DEV_DMA_2;
            break;
        case E_DRV_SCLDMA_IO_ID_3:
            enDev = E_DRV_SCLOS_DEV_DMA_3;
            break;
        case E_DRV_SCLDMA_IO_ID_4:
            enDev = E_DRV_SCLOS_DEV_DMA_4;
            break;
        default:
            enDev = E_DRV_SCLOS_DEV_MAX;
                break;
    }
    return enDev;
}

MDrvSclDmaIdType_e _DrvSclDmaGetDmaId(DrvSclDmaIoIdType_e enSclDmaId)
{
    MDrvSclDmaIdType_e enDev;
    switch(enSclDmaId)
    {
        case E_DRV_SCLDMA_IO_ID_1:
            enDev = E_MDRV_SCLDMA_ID_1;
            break;
        case E_DRV_SCLDMA_IO_ID_2:
            enDev = E_MDRV_SCLDMA_ID_2;
            break;
        case E_DRV_SCLDMA_IO_ID_3:
            enDev = E_MDRV_SCLDMA_ID_3;
            break;
        case E_DRV_SCLDMA_IO_ID_4:
            enDev = E_MDRV_SCLDMA_ID_MDWIN;
            break;
        default:
            enDev = E_DRV_SCLOS_DEV_MAX;
                break;
    }
    return enDev;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool _DrvSclDmaIoDeInit(DrvSclDmaIoIdType_e enSclDmaId)
{
    DrvSclosProbeType_e enType;
    enType = (enSclDmaId==E_DRV_SCLDMA_IO_ID_1) ? E_DRV_SCLOS_INIT_DMA_1 :
            (enSclDmaId==E_DRV_SCLDMA_IO_ID_2) ? E_DRV_SCLOS_INIT_DMA_2 :
            (enSclDmaId==E_DRV_SCLDMA_IO_ID_3) ?   E_DRV_SCLOS_INIT_DMA_3 :
             (enSclDmaId==E_DRV_SCLDMA_IO_ID_4) ?    E_DRV_SCLOS_INIT_DMA_4:
                E_DRV_SCLOS_INIT_NONE;
    DrvSclOsClearProbeInformation(enType);
    if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_ALL) == 0)
    {
        MDrvSclDmaExit(1);
    }
    else if(!DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_DMA))
    {
        MDrvSclDmaExit(0);
    }
    if(s32SclDmaIoHandlerMutex != -1)
    {
         DrvSclOsDeleteMutex(s32SclDmaIoHandlerMutex);
         s32SclDmaIoHandlerMutex = -1;
    }
    return TRUE;
}
bool _DrvSclDmaIoInit(DrvSclDmaIoIdType_e enSclDmaId)
{
    MDrvSclDmaInitConfig_t stSCLDMAInitCfg;
    MDrvSclDmaIdType_e enMSclDmaId;
    u16 i, start, end;
    MDrvSclCtxConfig_t *pvCfg;
    DrvSclOsMemset(&stSCLDMAInitCfg,0,sizeof(MDrvSclDmaInitConfig_t));
    if(enSclDmaId >= E_DRV_SCLDMA_IO_ID_NUM)
    {
        SCL_ERR("%s %d, Id out of range: %d\n", __FUNCTION__, __LINE__, enSclDmaId);
        return FALSE;
    }

    if(s32SclDmaIoHandlerMutex == -1)
    {
        s32SclDmaIoHandlerMutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "SCLDMA_IO", SCLOS_PROCESS_SHARED);

        if(s32SclDmaIoHandlerMutex == -1)
        {
            SCL_ERR("%s %d, Create Mutex Fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }
    enMSclDmaId = _DrvSclDmaGetDmaId(enSclDmaId);
    // Handler
    start = enSclDmaId * DRV_SCLDMA_HANDLER_INSTANCE_NUM;
    end = start + DRV_SCLDMA_HANDLER_INSTANCE_NUM;

    for(i=start; i<end; i++)
    {
        _gstSclDmaHandler[i].s32Handle = -1;
        _gstSclDmaHandler[i].enSclDmaId = E_DRV_SCLDMA_IO_ID_NUM;
        _gstSclDmaHandler[i].stCtxCfg.s32Id = -1;
        _gstSclDmaHandler[i].stCtxCfg.enIdType = E_DRV_SCLDMA_IO_ID_NUM;
        _gstSclDmaHandler[i].stCtxCfg.pLockCfg = NULL;
        _gstSclDmaHandler[i].stCtxCfg.pCmdqCtx = NULL;
    }
    //Ctx Init
    if( MDrvSclCtxInit() == FALSE)
    {
        SCL_ERR("%s %d, Init Ctx\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    //Dma Function
    if(enSclDmaId == E_DRV_SCLDMA_IO_ID_1)
    {
        DrvSclOsMemset(&_gstSclDmaFunc, 0, sizeof(DrvSclDmaIoFunctionConfig_t));
        _gstSclDmaFunc.DrvSclDmaIoSetInBufferConfig           = _DrvSclDmaIoSetInBufferConfig;
        _gstSclDmaFunc.DrvSclDmaIoSetOutBufferConfig          = _DrvSclDmaIoSetOutBufferConfig;
        _gstSclDmaFunc.DrvSclDmaIoCreateInstConfig            = _DrvSclDmaIoCreateInstConfig;
        _gstSclDmaFunc.DrvSclDmaIoDestroyInstConfig           = _DrvSclDmaIoDestroyInstConfig;
        _gstSclDmaFunc.DrvSclDmaIoInstProcessConfig           = _DrvSclDmaIoInstProcess;
    }

    //ToDo each Dma init
    stSCLDMAInitCfg.u32Riubase = 0x1F000000; //ToDo
    stSCLDMAInitCfg.u32IRQNUM     = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0);
    pvCfg = MDrvSclCtxGetDefaultCtx();
    stSCLDMAInitCfg.pvCtx   = (void *)&(pvCfg->stCtx);
    if( MDrvSclDmaInit(enMSclDmaId, &stSCLDMAInitCfg) == 0)
    {
        return -EFAULT;
    }
    return TRUE;
}

s32 _DrvSclDmaIoOpen(DrvSclDmaIoIdType_e enSclDmaId)
{
    s32 s32Handle = -1;
    s16 s16Idx = -1;
    s16 i ;
    MDrvSclCtxIdType_e enCtxId;
    DRV_SCLDMA_IO_LOCK_MUTEX(s32SclDmaIoHandlerMutex);
    for(i=0; i<DRV_SCLDMA_HANDLER_MAX; i++)
    {
        if(_gstSclDmaHandler[i].s32Handle == -1)
        {
            s16Idx = i;
            break;
        }
    }
#if I2_DVR
    enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
#else
    if(enSclDmaId == E_DRV_SCLDMA_IO_ID_3)
    {
        enCtxId = E_MDRV_SCL_CTX_ID_M2M;
    }
    else
    {
        enCtxId = E_MDRV_SCL_CTX_ID_SC_ALL;
    }
#endif

    if(s16Idx == -1)
    {
        s32Handle = -1;
        SCL_ERR("[SCLDMA]: Handler is not empyt\n");
    }
    else
    {
        s32Handle = s16Idx | DRV_SCLDMA_HANDLER_PRE_FIX | (enSclDmaId<<(HANDLER_PRE_FIX_SHIFT));
        _gstSclDmaHandler[s16Idx].s32Handle = s32Handle ;
        _gstSclDmaHandler[s16Idx].enSclDmaId = enSclDmaId;
        _gstSclDmaHandler[s16Idx].stCtxCfg.enCtxId = enCtxId;
        _gstSclDmaHandler[s16Idx].stCtxCfg.enIdType = enSclDmaId;
        _gstSclDmaHandler[s16Idx].stCtxCfg.s32Id = s32Handle;
        _gstSclDmaHandler[s16Idx].stCtxCfg.pLockCfg = MDrvSclCtxGetLockConfig(enCtxId);
        _gstSclDmaHandler[s16Idx].stCtxCfg.pCmdqCtx = MDrvSclCtxGetDefaultCtx();

    }
    DRV_SCLDMA_IO_UNLOCK_MUTEX(s32SclDmaIoHandlerMutex);
    return s32Handle;
}

DrvSclDmaIoErrType_e _DrvSclDmaIoRelease(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;
    u16 u16loop = 0;
    DrvSclDmaIoErrType_e eRet = E_DRV_SCLDMA_IO_ERR_OK;
    for(i=0; i<DRV_SCLDMA_HANDLER_MAX; i++)
    {
        if(_gstSclDmaHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        SCL_DBG(SCL_DBG_LV_IOCTL(),"[DMA]   %s %d not support s32Handler:%lx\n", __FUNCTION__, __LINE__,s32Handler);
        eRet = E_DRV_SCLDMA_IO_ERR_FAULT;
    }
    else
    {

        MDrvSclDmaIdType_e enMdrvIdType;

        if(_DrvSclDmaIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
        {
            SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
            eRet = E_DRV_SCLDMA_IO_ERR_INVAL;
        }
        else
        {
            SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_SC1HLEVEL, "[SCLDMA %d] Release:: == %lx == \n",  enMdrvIdType,s32Handler);
            _gstSclDmaHandler[s16Idx].s32Handle = -1;
            _gstSclDmaHandler[s16Idx].enSclDmaId = E_DRV_SCLDMA_IO_ID_NUM;
            _gstSclDmaHandler[s16Idx].stCtxCfg.s32Id = -1;
            _gstSclDmaHandler[s16Idx].stCtxCfg.enIdType = E_DRV_SCLDMA_IO_ID_NUM;
            _gstSclDmaHandler[s16Idx].stCtxCfg.pCmdqCtx = NULL;
            _gstSclDmaHandler[s16Idx].stCtxCfg.pLockCfg = NULL;
            for(i = 0; i < DRV_SCLDMA_HANDLER_MAX; i++)
            {
                if(_gstSclDmaHandler[i].s32Handle != -1)
                {
                    u16loop = 1;
                    break;
                }
            }
            if(!u16loop)
            {
                MDrvSclDmaRelease(enMdrvIdType, NULL);
                MDrvSclDmaReSetHw(enMdrvIdType,  NULL);
            }
            eRet = E_DRV_SCLDMA_IO_ERR_OK;
        }
    }
    return eRet;
}
void _DrvSclDmaIoKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    MDrvSclCtxKeepCmdqFunction(pstCmdq);
}

DrvSclDmaIoErrType_e _DrvSclDmaIoSetInBufferConfig(s32 s32Handler, DrvSclDmaIoBufferConfig_t *pstIoInCfg)
{
    MDrvSclDmaBufferConfig_t stDMABufferCfg;
    DrvSclDmaIoVersionChkConfig_t stVersion;
    MDrvSclDmaIdType_e enMdrvIdType;
    DrvSclDmaIoCtxConfig_t *pstSclDmaCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stDMABufferCfg,0,sizeof(MDrvSclDmaBufferConfig_t));
    _DrvSclDmaIoFillVersionChkStruct(sizeof(DrvSclDmaIoBufferConfig_t),
                                                 pstIoInCfg->VerChk_Size,
                                                 &pstIoInCfg->VerChk_Version,&stVersion);

    if(_DrvSclDmaIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(_DrvSclDmaIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(IsMDrvScldmaIdType_Max(enMdrvIdType))
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    //scldma1, scldma2 not support
    if(IsMDrvScldmaIdType_1(enMdrvIdType) || IsMDrvScldmaIdType_2(enMdrvIdType))
    {
        SCL_ERR( "[SCLDMA] Not Support %s %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    pstSclDmaCtxCfg =_DrvSclDmaIoGetCtxConfig(s32Handler);

    if( pstSclDmaCtxCfg == NULL)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    enCtxId = pstSclDmaCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    //do when id is scldma3 & scldma4
    _DrvSclDmaIoFillBufferConfig(pstIoInCfg,&stDMABufferCfg);
    stDMABufferCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    if(!MDrvSclDmaSetDmaReadClientConfig(enMdrvIdType,  &stDMABufferCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLDMA_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);

    return E_DRV_SCLDMA_IO_ERR_OK;
}

DrvSclDmaIoErrType_e _DrvSclDmaIoSetOutBufferConfig(s32 s32Handler, DrvSclDmaIoBufferConfig_t *pstIoInCfg)
{
    MDrvSclDmaBufferConfig_t stDMABufferCfg;
    DrvSclDmaIoVersionChkConfig_t stVersion;
    MDrvSclDmaIdType_e enMdrvIdType;
    DrvSclDmaIoCtxConfig_t *pstSclDmaCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stDMABufferCfg,0,sizeof(MDrvSclDmaBufferConfig_t));
    _DrvSclDmaIoFillVersionChkStruct(sizeof(DrvSclDmaIoBufferConfig_t),
                                                 pstIoInCfg->VerChk_Size,
                                                 &pstIoInCfg->VerChk_Version,&stVersion);

    if(_DrvSclDmaIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }


    if(_DrvSclDmaIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(IsMDrvScldmaIdType_Max(enMdrvIdType))
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    //scldma4 not support
    if(IsMDrvScldmaIdType_PNL(enMdrvIdType) )
    {
        SCL_ERR( "[SCLDMA] Not Support %s %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    pstSclDmaCtxCfg =_DrvSclDmaIoGetCtxConfig(s32Handler);
    if( pstSclDmaCtxCfg == NULL)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    enCtxId = pstSclDmaCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclDmaIoFillBufferConfig(pstIoInCfg,&stDMABufferCfg);
    stDMABufferCfg.pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    if(!MDrvSclDmaSetDmaWriteClientConfig(enMdrvIdType,  &stDMABufferCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLDMA_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLDMA_IO_ERR_OK;
}

DrvSclDmaIoErrType_e _DrvSclDmaIoCreateInstConfig(s32 s32Handler, DrvSclDmaIoLockConfig_t *pstIoInCfg)
{
    DrvSclDmaIoVersionChkConfig_t stVersion;
    DrvSclDmaIoCtxConfig_t *pstSclDmaCtxCfg;
    MDrvSclCtxInstConfig_t stCtxInst;
    DrvSclOsMemset(&stCtxInst,0,sizeof(MDrvSclCtxInstConfig_t));
    _DrvSclDmaIoFillVersionChkStruct(sizeof(DrvSclDmaIoLockConfig_t),
                                              (pstIoInCfg->VerChk_Size),
                                              &(pstIoInCfg->VerChk_Version),&stVersion);

    if(_DrvSclDmaIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLDMA]   %s  %d, version fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    pstSclDmaCtxCfg =_DrvSclDmaIoGetCtxConfig(s32Handler);

    if( pstSclDmaCtxCfg == NULL)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(pstSclDmaCtxCfg->enIdType != E_DRV_SCLDMA_IO_ID_1)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, Not Support\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    DRV_SCLDMA_IO_LOCK_MUTEX(s32SclDmaIoHandlerMutex);
    //alloc Ctx handler
    stCtxInst.ps32IdBuf = pstIoInCfg->ps32IdBuf;
    stCtxInst.u8IdNum = pstIoInCfg->u8BufSize;
    pstSclDmaCtxCfg->pCmdqCtx = MDrvSclCtxAllocate(E_MDRV_SCL_CTX_ID_SC_ALL,&stCtxInst);
    //alloc DMA Ctx
    if(pstSclDmaCtxCfg->pCmdqCtx == NULL)
    {
        SCL_ERR("%s %d::Allocate Ctx Fail\n", __FUNCTION__, __LINE__);
        DRV_SCLDMA_IO_UNLOCK_MUTEX(s32SclDmaIoHandlerMutex);

        return E_DRV_SCLDMA_IO_ERR_FAULT;
    }



    DRV_SCLDMA_IO_UNLOCK_MUTEX(s32SclDmaIoHandlerMutex);

    return E_DRV_SCLDMA_IO_ERR_OK;
}
DrvSclDmaIoErrType_e _DrvSclDmaIoDestroyInstConfig(s32 s32Handler, DrvSclDmaIoLockConfig_t *pstIoInCfg)
{
    DrvSclDmaIoVersionChkConfig_t stVersion;
    DrvSclDmaIoCtxConfig_t *pstSclDmaCtxCfg;

    _DrvSclDmaIoFillVersionChkStruct(sizeof(DrvSclDmaIoLockConfig_t),
                                              (pstIoInCfg->VerChk_Size),
                                              &(pstIoInCfg->VerChk_Version),&stVersion);

    if(_DrvSclDmaIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLDMA]   %s  %d, version fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    pstSclDmaCtxCfg =_DrvSclDmaIoGetCtxConfig(s32Handler);

    if( pstSclDmaCtxCfg == NULL)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(pstSclDmaCtxCfg->enIdType != E_DRV_SCLDMA_IO_ID_1)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, Not Support\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }

    DRV_SCLDMA_IO_LOCK_MUTEX(s32SclDmaIoHandlerMutex);
    //free Ctx handler
    MDrvSclCtxFree(pstSclDmaCtxCfg->pCmdqCtx);
    //free DMA Ctx

    DRV_SCLDMA_IO_UNLOCK_MUTEX(s32SclDmaIoHandlerMutex);

    return E_DRV_SCLDMA_IO_ERR_OK;
}
void _DrvSclDmaIoInstFillProcessCfg(MDrvSclDmaProcessConfig_t *stProcess, DrvSclDmaIoProcessConfig_t *pstIoInCfg)
{
    u8 j;
    stProcess->stCfg.bEn = pstIoInCfg->stCfg.bEn;
    stProcess->stCfg.enMemType = pstIoInCfg->stCfg.enMemType;
    for(j =0;j<3;j++)
    {
        stProcess->stCfg.stBufferInfo.u64PhyAddr[j]=
            pstIoInCfg->stCfg.stBufferInfo.u64PhyAddr[j];
        stProcess->stCfg.stBufferInfo.u32Stride[j]=
            pstIoInCfg->stCfg.stBufferInfo.u32Stride[j];
    }
}

DrvSclDmaIoErrType_e _DrvSclDmaIoInstProcess(s32 s32Handler, DrvSclDmaIoProcessConfig_t *pstIoInCfg)
{
    DrvSclDmaIoCtxConfig_t *pstSclDmaCtxCfg;
    MDrvSclDmaProcessConfig_t stProcess;
    MDrvSclDmaIdType_e enMdrvIdType;
    DrvSclDmaIoErrType_e eRet = E_DRV_SCLDMA_IO_ERR_OK;
    DrvSclOsMemset(&stProcess,0,sizeof(MDrvSclDmaProcessConfig_t));
    if(_DrvSclDmaIoGetMdrvIdType(s32Handler, &enMdrvIdType) == FALSE)
    {
        SCL_ERR( "[SCLDMA]   %s %d  \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    pstSclDmaCtxCfg =_DrvSclDmaIoGetCtxConfig(s32Handler);

    if( pstSclDmaCtxCfg == NULL)
    {
        SCL_ERR( "[SCLDMA]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLDMA_IO_ERR_INVAL;
    }
    MDrvSclCtxSetLockConfig(s32Handler,pstSclDmaCtxCfg->enCtxId);
    _DrvSclDmaIoInstFillProcessCfg(&stProcess,pstIoInCfg);
    stProcess.pvCtx = (void *)MDrvSclCtxGetConfigCtx(pstSclDmaCtxCfg->enCtxId);
    MDrvSclDmaInstProcess(enMdrvIdType,&stProcess);
    MDrvSclCtxSetUnlockConfig(s32Handler,pstSclDmaCtxCfg->enCtxId);
    return eRet;
}
DrvSclDmaIoErrType_e _DrvSclDmaIoInstFlip(s32 s32Handler)
{
    MDrvSclCtxCmdqConfig_t *pvCtx;
    DrvSclDmaIoErrType_e eRet = E_DRV_SCLDMA_IO_ERR_OK;
    bool bFireMload = 0;
    MDrvSclCtxSetLockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
    pvCtx = MDrvSclCtxGetConfigCtx(E_MDRV_SCL_CTX_ID_SC_ALL);
    MDrvSclCtxFire(pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC1,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC1,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC2,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC2,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC3,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC3,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_HSP_Y_SC4,pvCtx);
    bFireMload += MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_VSP_Y_SC4,pvCtx);
    MDrvSclCtxFireMload(bFireMload,pvCtx);
    MDrvSclCtxSetUnlockConfig(s32Handler,E_MDRV_SCL_CTX_ID_SC_ALL);
    return eRet;
}
