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
#define  _MDRV_SCLDMA_C
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"
#include "drv_scl_dma_st.h"
#include "drv_scl_dma.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define BUFFER_NUMBER_TO_HWIDX_OFFSET 1
#define _ISQueueNeedCopyToUser(enUsedType) (enUsedType==EN_MDRV_SCLDMA_BUFFER_QUEUE_TYPE_PEEKQUEUE \
        || enUsedType==EN_MDRV_SCLDMA_BUFFER_QUEUE_TYPE_DEQUEUE)
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------
DrvSclDmaRwModeType_e _MDrvSclDmaSwitchIDtoReadModeForDriverlayer
(MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaMemType_e enMemType,DrvSclDmaIdType_e *enID)
{
    DrvSclDmaRwModeType_e enRWMode;
    switch(enSCLDMA_ID)
    {
        case E_MDRV_SCLDMA_ID_3:
            *enID = E_DRV_SCLDMA_ID_3_R;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ?  E_DRV_SCLDMA_FRM_R :
                       enMemType == E_MDRV_SCLDMA_MEM_FRMR ?  E_DRV_SCLDMA_FRM_R :
                                                              E_DRV_SCLDMA_RW_NUM;
            break;

        case E_MDRV_SCLDMA_ID_PNL:
            *enID = E_DRV_SCLDMA_ID_PNL_R;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_DBG_R :
                                                             E_DRV_SCLDMA_RW_NUM;
            break;

        default:
        case E_MDRV_SCLDMA_ID_1:
        case E_MDRV_SCLDMA_ID_2:
            enRWMode = E_DRV_SCLDMA_RW_NUM;
            SCL_ERR( "[SCLDMA]%s %d::Not support In TRIGGER\n",__FUNCTION__, __LINE__);
            break;
    }
    return enRWMode;
}
DrvSclDmaRwModeType_e _MDrvSclDmaSwitchIDtoWriteModeForDriverlayer
(MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaMemType_e enMemType,DrvSclDmaIdType_e *enID)
{
    DrvSclDmaRwModeType_e enRWMode;
    switch(enSCLDMA_ID)
    {
        case E_MDRV_SCLDMA_ID_1:
            *enID = E_DRV_SCLDMA_ID_1_W;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_FRM_W :
                        enMemType == E_MDRV_SCLDMA_MEM_SNP ? E_DRV_SCLDMA_SNP_W :
                        enMemType == E_MDRV_SCLDMA_MEM_IMI ? E_DRV_SCLDMA_IMI_W :
                                                              E_DRV_SCLDMA_RW_NUM;
            break;

        case E_MDRV_SCLDMA_ID_2:
            *enID = E_DRV_SCLDMA_ID_2_W;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_FRM_W :
                        enMemType == E_MDRV_SCLDMA_MEM_FRM2 ? E_DRV_SCLDMA_FRM2_W :
                        enMemType == E_MDRV_SCLDMA_MEM_IMI ? E_DRV_SCLDMA_IMI_W :
                                                              E_DRV_SCLDMA_RW_NUM;

            break;

        case E_MDRV_SCLDMA_ID_3:
            *enID = E_DRV_SCLDMA_ID_3_W;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_FRM_W :
                                                              E_DRV_SCLDMA_RW_NUM;
            break;
        case E_MDRV_SCLDMA_ID_MDWIN:
            *enID = E_DRV_SCLDMA_ID_MDWIN_W;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_DWIN_W:
                                                              E_DRV_SCLDMA_RW_NUM;
            break;
        case E_MDRV_SCLDMA_ID_PNL:
            *enID = E_DRV_SCLDMA_ID_PNL_R;
            enRWMode = enMemType == E_MDRV_SCLDMA_MEM_FRM ? E_DRV_SCLDMA_DBG_R:
                                                              E_DRV_SCLDMA_RW_NUM;
            break;
        default:
            SCL_ERR( "[SCLDMA]%s %d::Not support In SCLDMA\n",__FUNCTION__, __LINE__);
            enRWMode = E_DRV_SCLDMA_RW_NUM;
            break;
    }
    return enRWMode;
}
DrvSclDmaRwModeType_e _MDrvSclDmaSwitchIDForDriverlayer
    (MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaMemType_e enMemType,DrvSclDmaIdType_e *enID,bool bReadDMAMode)
{
    DrvSclDmaRwModeType_e enRWMode;
    if(bReadDMAMode)
    {
        enRWMode = _MDrvSclDmaSwitchIDtoReadModeForDriverlayer(enSCLDMA_ID,enMemType,enID);
    }
    else
    {
        enRWMode = _MDrvSclDmaSwitchIDtoWriteModeForDriverlayer(enSCLDMA_ID,enMemType,enID);
    }
    return enRWMode;
}
void _MDrvSclDmaFillDmaInfoStruct(DrvSclDmaAttrType_t *stDrvDMACfg,MDrvSclDmaAttrType_t *stSendToIOCfg)
{
    int u32BufferIdx;
    stSendToIOCfg->u16DMAcount  = stDrvDMACfg->u16DMAcount;
    stSendToIOCfg->u16DMAH      = stDrvDMACfg->u16DMAH;
    stSendToIOCfg->u16DMAV      = stDrvDMACfg->u16DMAV;
    stSendToIOCfg->enBufMDType  = stDrvDMACfg->enBuffMode;
    stSendToIOCfg->enColorType  = stDrvDMACfg->enColor;
    stSendToIOCfg->u16BufNum    = stDrvDMACfg->u8MaxIdx+1;
    stSendToIOCfg->bDMAEn       = stDrvDMACfg->bDMAEn;
    for(u32BufferIdx=0;u32BufferIdx<stSendToIOCfg->u16BufNum;u32BufferIdx++)
    {
        stSendToIOCfg->u32Base_Y[u32BufferIdx] = stDrvDMACfg->u32Base_Y[u32BufferIdx];
        stSendToIOCfg->u32Base_C[u32BufferIdx] = stDrvDMACfg->u32Base_C[u32BufferIdx];
        stSendToIOCfg->u32Base_V[u32BufferIdx] = stDrvDMACfg->u32Base_V[u32BufferIdx];
    }
    stSendToIOCfg->bHFilp= stDrvDMACfg->bHFilp;
    stSendToIOCfg->bVFilp= stDrvDMACfg->bVFilp;
    stSendToIOCfg->u32LineOffset_en= stDrvDMACfg->u32LineOffset_en;
    stSendToIOCfg->u32LineOffset_Y= stDrvDMACfg->u32LineOffset_Y;
}


void _MDrvSclDmaFillRWCfgStruct
    (MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaBufferConfig_t *pCfg,
    DrvSclDmaIdType_e *enID,bool bReadDMAMode,DrvSclDmaRwConfig_t *stSCLDMACfg)
{
    u16 u16BufferIdx;
    DrvSclOsMemset(stSCLDMACfg, 0, sizeof(DrvSclDmaRwConfig_t));
    stSCLDMACfg->enRWMode = _MDrvSclDmaSwitchIDForDriverlayer(enSCLDMA_ID,pCfg->enMemType,enID,bReadDMAMode);
    stSCLDMACfg->u16Height = pCfg->u16Height;
    stSCLDMACfg->u16Width  = pCfg->u16Width;
    stSCLDMACfg->u8MaxIdx  = pCfg->u16BufNum -BUFFER_NUMBER_TO_HWIDX_OFFSET;
    stSCLDMACfg->u8Flag    = pCfg->u8Flag;
    stSCLDMACfg->enBuffMode= (pCfg->enBufMDType == E_MDRV_SCLDMA_BUFFER_MD_RING) ? E_DRV_SCLDMA_BUF_MD_RING :
                            (pCfg->enBufMDType == E_MDRV_SCLDMA_BUFFER_MD_SWRING) ? E_DRV_SCLDMA_BUF_MD_SWRING :
                                                                                E_DRV_SCLDMA_BUF_MD_SINGLE;

    for(u16BufferIdx=0; u16BufferIdx<pCfg->u16BufNum; u16BufferIdx++)
    {
        stSCLDMACfg->u32Base_Y[u16BufferIdx] = pCfg->u32Base_Y[u16BufferIdx];
        stSCLDMACfg->u32Base_C[u16BufferIdx] = pCfg->u32Base_C[u16BufferIdx];
        stSCLDMACfg->u32Base_V[u16BufferIdx] = pCfg->u32Base_V[u16BufferIdx];
    }
    stSCLDMACfg->bHFlip = pCfg->bHFlip;
    stSCLDMACfg->bVFlip = pCfg->bVFlip;
    stSCLDMACfg->enColor = pCfg->enColorType == E_MDRV_SCLDMA_COLOR_YUV422 ? E_DRV_SCLDMA_COLOR_YUV422 :
                          pCfg->enColorType == E_MDRV_SCLDMA_COLOR_YUV420 ? E_DRV_SCLDMA_COLOR_YUV420 :
                          pCfg->enColorType == E_MDRV_SCLDMA_COLOR_YCSep422 ? E_DRV_SCLDMA_COLOR_YCSep422 :
                          pCfg->enColorType == E_MDRV_SCLDMA_COLOR_YUVSep422 ? E_DRV_SCLDMA_COLOR_YUVSep422 :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV420 ? E_DRV_MDWIN_COLOR_YUV420 :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV420CE ? E_DRV_MDWIN_COLOR_YUV420CE :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV4206CE ? E_DRV_MDWIN_COLOR_YUV4206CE :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV422 ? E_DRV_MDWIN_COLOR_YUV422 :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV422CE ? E_DRV_MDWIN_COLOR_YUV422CE :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV4226CE ? E_DRV_MDWIN_COLOR_YUV4226CE :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV444 ? E_DRV_MDWIN_COLOR_YUV444 :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_YUV444A ? E_DRV_MDWIN_COLOR_YUV444A :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_RGB565 ? E_DRV_MDWIN_COLOR_RGB565 :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_ARGB ? E_DRV_MDWIN_COLOR_ARGB :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_ABGR ? E_DRV_MDWIN_COLOR_ABGR :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_RGBA ? E_DRV_MDWIN_COLOR_RGBA :
                          pCfg->enColorType == E_MDRV_MDWIN_COLOR_BGRA ? E_DRV_MDWIN_COLOR_BGRA :
                            E_DRV_SCLDMA_COLOR_NUM;
    stSCLDMACfg->pvCtx = pCfg->pvCtx;
}
bool MDrvSclDmaSuspend(MDrvSclDmaIdType_e enSCLDMA_ID)
{
    DrvSclDmaSuspendResumeConfig_t stSCLDMASuspendResumeCfg;
    DrvSclDmaIdType_e enID;
    bool bRet = TRUE;
    void *pvCtx;
    DrvSclOsMemset(&stSCLDMASuspendResumeCfg,0,sizeof(DrvSclDmaSuspendResumeConfig_t));
    enID =  enSCLDMA_ID == E_MDRV_SCLDMA_ID_1   ? E_DRV_SCLDMA_ID_1_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_2   ? E_DRV_SCLDMA_ID_2_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_3   ? E_DRV_SCLDMA_ID_3_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_PNL ? E_DRV_SCLDMA_ID_PNL_R :
                                                  E_DRV_SCLDMA_ID_MAX;
    DrvSclOsMemset(&stSCLDMASuspendResumeCfg, 0, sizeof(DrvSclDmaSuspendResumeConfig_t));

    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    stSCLDMASuspendResumeCfg.pvCtx = pvCtx;

    if(DrvSclDmaSuspend(enID, &stSCLDMASuspendResumeCfg))
    {
        bRet = TRUE;
    }
    else
    {
        SCL_ERR( "[SCLDMA]%s %d::Suspend Fail\n",__FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

bool MDrvSclDmaResume(MDrvSclDmaIdType_e enSCLDMA_ID)
{
    DrvSclDmaSuspendResumeConfig_t stSCLDMASuspendResumeCfg;
    DrvSclDmaIdType_e enID;
    bool bRet = TRUE;
    void *pvCtx;
    DrvSclOsMemset(&stSCLDMASuspendResumeCfg,0,sizeof(DrvSclDmaSuspendResumeConfig_t));
    enID =  enSCLDMA_ID == E_MDRV_SCLDMA_ID_1   ? E_DRV_SCLDMA_ID_1_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_2   ? E_DRV_SCLDMA_ID_2_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_3   ? E_DRV_SCLDMA_ID_3_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_PNL ? E_DRV_SCLDMA_ID_PNL_R :
                                                  E_DRV_SCLDMA_ID_MAX;

    DrvSclOsMemset(&stSCLDMASuspendResumeCfg, 0, sizeof(DrvSclDmaSuspendResumeConfig_t));
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(MDRV_SCL_CTX_ID_DEFAULT);
    stSCLDMASuspendResumeCfg.pvCtx = pvCtx;
    if(DrvSclDmaResume(enID, &stSCLDMASuspendResumeCfg))
    {
        bRet = TRUE;
    }
    else
    {
        SCL_ERR( "[SCLDMA]%s %d::Resume Fail\n",__FUNCTION__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}
bool MDrvSclDmaExit(bool bCloseISR)
{
    DrvSclDmaExit(bCloseISR);
    return 1;
}

bool MDrvSclDmaInit(MDrvSclDmaIdType_e enSCLDMA_ID, MDrvSclDmaInitConfig_t *pCfg)
{
    bool bRet = FALSE;
    DrvSclDmaInitConfig_t stDMAInitCfg;
    SCL_DBG(SCL_DBG_LV_SCLDMA()&(Get_DBGMG_SCLDMA(enSCLDMA_ID,0,0)), "[SCLDMA]%s\n", __FUNCTION__);
    DrvSclOsMemset(&stDMAInitCfg,0,sizeof(DrvSclDmaInitConfig_t));
    stDMAInitCfg.u32RIUBase     = pCfg->u32Riubase;
    stDMAInitCfg.u32IRQNUM      = pCfg->u32IRQNUM;
    stDMAInitCfg.u32CMDQIRQNUM  = pCfg->u32CMDQIRQNUM;
    stDMAInitCfg.pvCtx          = pCfg->pvCtx;
    if(DrvSclDmaInit(&stDMAInitCfg) == FALSE)
    {
        SCL_ERR( "[SCLDMA]%s %d::Init Fail\n",__FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
    }

    return bRet;
}
void MDrvSclDmaRelease(MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaClkConfig_t *stclkcfg)
{
    DrvSclDmaIdType_e enID;
    DrvSclDmaClkConfig_t *stclk;
    enID =  enSCLDMA_ID == E_MDRV_SCLDMA_ID_1   ? E_DRV_SCLDMA_ID_1_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_2   ? E_DRV_SCLDMA_ID_2_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_3   ? E_DRV_SCLDMA_ID_3_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_PNL ? E_DRV_SCLDMA_ID_PNL_R :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_MDWIN ? E_DRV_SCLDMA_ID_MDWIN_W :
                                                  E_DRV_SCLDMA_ID_MAX;

    stclk = NULL;
    DrvSclDmaRelease(enID,stclk);
}
void MDrvSclDmaReSetHw(MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaClkConfig_t *stclkcfg)
{
    DrvSclDmaClkConfig_t *stclk;
    DrvSclDmaIdType_e enID;
    enID =  enSCLDMA_ID == E_MDRV_SCLDMA_ID_1   ? E_DRV_SCLDMA_ID_1_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_2   ? E_DRV_SCLDMA_ID_2_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_3   ? E_DRV_SCLDMA_ID_3_W   :
            enSCLDMA_ID == E_MDRV_SCLDMA_ID_PNL ? E_DRV_SCLDMA_ID_PNL_R :
                                                  E_DRV_SCLDMA_ID_MAX;
    stclk = NULL;
    DrvSclDmaReSetHw(enID,stclk);
}
bool MDrvSclDmaInstProcess(MDrvSclDmaIdType_e enSCLDMA_ID,MDrvSclDmaProcessConfig_t *pstProcess)
{
    DrvSclDmaProcessConfig_t stProcess;
    DrvSclDmaIdType_e enID;
    bool bReadDMAMode;
    bReadDMAMode = (pstProcess->stCfg.enMemType == E_MDRV_SCLDMA_MEM_FRMR) ? 1 :0;
    DrvSclOsMemcpy(&stProcess,pstProcess,sizeof(DrvSclDmaProcessConfig_t));
    stProcess.stCfg.enRWMode = _MDrvSclDmaSwitchIDForDriverlayer(enSCLDMA_ID,pstProcess->stCfg.enMemType,&enID,bReadDMAMode);
    DrvSclDmaInstProcess(enID,&stProcess);
    return 1;
}

bool MDrvSclDmaSetDmaReadClientConfig
    (MDrvSclDmaIdType_e enSCLDMA_ID, MDrvSclDmaBufferConfig_t *pCfg)
{
    DrvSclDmaIdType_e enID;
    DrvSclDmaRwConfig_t stSCLDMACfg;
    bool  bRet;
    DrvSclOsMemset(&stSCLDMACfg,0,sizeof(DrvSclDmaRwConfig_t));
    SCL_DBG(SCL_DBG_LV_SCLDMA()&(Get_DBGMG_SCLDMA(enSCLDMA_ID,pCfg->enMemType,0)), "[SCLDMA]%s\n", __FUNCTION__);

    _MDrvSclDmaFillRWCfgStruct(enSCLDMA_ID,pCfg,&enID,1,&stSCLDMACfg);
    bRet = (bool)DrvSclDmaSetDmaClientConfig(enID, &stSCLDMACfg);

    return bRet;
}

bool MDrvSclDmaSetDmaWriteClientConfig
    (MDrvSclDmaIdType_e enSCLDMA_ID, MDrvSclDmaBufferConfig_t *pCfg)
{
    DrvSclDmaIdType_e enID;
    DrvSclDmaRwConfig_t stSCLDMACfg;
    bool  bRet;
    DrvSclOsMemset(&stSCLDMACfg,0,sizeof(DrvSclDmaRwConfig_t));
    SCL_DBG(SCL_DBG_LV_SCLDMA()&(Get_DBGMG_SCLDMA(enSCLDMA_ID,pCfg->enMemType,0)),
        "[SCLDMA]%s  ID:%d\n", __FUNCTION__,enSCLDMA_ID);

    _MDrvSclDmaFillRWCfgStruct(enSCLDMA_ID,pCfg,&enID,0,&stSCLDMACfg);

    bRet = (bool)DrvSclDmaSetDmaClientConfig(enID, &stSCLDMACfg);

    return bRet;
}
void MDrvSclDmaGetDmaInformationByClient
    (MDrvSclDmaGetConfig_t *stGetCfg,MDrvSclDmaAttrType_t *stSendToIOCfg)
{
    DrvSclDmaIdType_e enID;
    DrvSclDmaRwModeType_e enRWMode;
    DrvSclDmaAttrType_t stDrvDMACfg;
    DrvSclOsMemset(&stDrvDMACfg,0,sizeof(DrvSclDmaAttrType_t));
    enRWMode = _MDrvSclDmaSwitchIDForDriverlayer(stGetCfg->enSCLDMA_ID,stGetCfg->enMemType,&enID,stGetCfg->bReadDMAMode);
    DrvSclDmaGetDmaInformationByClient(enID, enRWMode,&stDrvDMACfg);
    _MDrvSclDmaFillDmaInfoStruct(&stDrvDMACfg,stSendToIOCfg);
}
