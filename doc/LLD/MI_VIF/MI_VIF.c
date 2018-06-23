#include <string.h>
#include "mi_common.h"
#include "MI_VIF_datatype.h"
#include "Hal_VIF_datatype.h"
#include "HAL_VIF.h"
#include "MI_VIF_datatype_internal.h"


static mi_vif_DevContex_t gstVifDevCtx[MI_VIF_MAX_DEV_NUM];
static mi_vif_ChnContex_t gstVifChnCtx[MI_VIF_MAX_PHYCHN_NUM];
static MI_SYS_DRV_HANDLE hDevSysHandle;
//static struct task_struct *mi_vif_in_tsk;
//static struct task_struct *mi_vif_out_tsk;
static struct task_struct *mi_vif_work_tsk;
static struct completion mi_vif_process_wakeup;


DECLARE_WAIT_QUEUE_HEAD(vif_isr_waitqueue);

MI_SYS_FieldType_e _mi_vif_TransMiCapSelToSysFieldType(MI_VIF_Capsel_e           eCapSel)
{
    switch(eCapSel)
    {
        case E_MI_VIF_CAPSEL_TOP:
            return E_MI_SYS_FIELDTYPE_TOP;

        case E_MI_VIF_CAPSEL_BOTTOM:
            return E_MI_SYS_FIELDTYPE_BOTTOM;

        case E_MI_VIF_CAPSEL_BOTH:
            return E_MI_SYS_FIELDTYPE_BOTH;

        default:
            return E_MI_SYS_FIELDTYPE_NONE;
    }
}

void _mi_vif_process_in()
{
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_ChnPort_t stPortSel;
    MI_SYS_BufInfo_t* pstBufInfo;
    VifRingBufElm_t stFbInfo;
    U32 u32ChnIndex;
    U32 u32PortIndex;
    MI_SYS_DRV_HANDLE miSysDrvHandle;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_BOOL bBlockedByRateCtrl = FALSE;

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContex_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        for(u32PortIndex = 0; u32PortIndex < 2; u32PortIndex++)
        {
            mi_vif_PortContex_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == FALSE)
                continue;

            memset(&stBufCfg, 0x0, sizeof(MI_SYS_BufConf_t));
            stBufCfg.eBufType = E_MI_SYS_BUFDATA_FRAME;
            stBufCfg.u64TargetPts = MI_SYS_INVALID_PTS;
            stBufCfg.stFrameCfg.eFormat = pPortCtx->eFormat;
            stBufCfg.stFrameCfg.u16Width = pPortCtx->stChnPortAttr.stDestSize.u32Width;
            stBufCfg.stFrameCfg.u16Height = pPortCtx->stChnPortAttr.stDestSize.u32Height;

            pstBufInfo = mi_sys_GetOutputPortBuf(hDevSysHandle, u32ChnIndex, u32PortIndex, &stBufCfg , &bBlockedByRateCtrl);

            if(pstBufInfo == NULL)
            {
                pPortCtx->u32VbFail++;
                continue;
            }

            stFbInfo->nPhyBufAddr = pstBufInfo.stFrameData.u64PhyAddr;//8bytes->4bytes?
            stFbInfo->nCropX   =  pPortCtx->stChnPortAttr.stCapRect.s32X;
            stFbInfo->nCropY   =  pPortCtx->stChnPortAttr.stCapRect.s32Y;
            stFbInfo->nCropW   =  pPortCtx->stChnPortAttr.stCapRect.u32Width;
            stFbInfo->nCropH   =  pPortCtx->stChnPortAttr.stCapRect.u32Height;
            MHalVifQueueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo);
        }
    }
}



void _mi_vif_process_out()
{
    MI_SYS_BufConf_t stBufCfg;
    MI_SYS_ChnPort_t stPortSel;
    MI_SYS_BufInfo_t stBufInfo;
    VifRingBufElm_t stFbInfo;
    U32 u32ChnIndex;
    U32 u32PortIndex;
    MI_SYS_DRV_HANDLE miSysDrvHandle;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    MI_U32 u32BufferNum;

    for(u32ChnIndex = 0; u32ChnIndex < MI_VIF_MAX_PHYCHN_NUM; u32ChnIndex++)
    {
        mi_vif_ChnContex_t* pChnCtx = &gstVifChnCtx[u32ChnIndex];

        for(u32PortIndex = 0; u32PortIndex < 2; u32PortIndex++)
        {
            mi_vif_PortContex_t* pPortCtx = &pChnCtx->stPortCtx[u32PortIndex];

            if(pPortCtx->bEnable == FALSE)
                continue;

            MHalVifQueryFrames(u32ChnIndex, u32PortIndex, &u32BufferNum);

            while(u32BufferNum--)
            {
                MHalVifDequeueFrameBuffer(u32ChnIndex, u32PortIndex, &stFbInfo);

                stBufInfo.eBufType = E_MI_SYS_BUFDATA_FRAME;
                stBufInfo.u64Pts = stFbInfo.nPTS;
                stBufInfo.stFrameData.ePixelFormat = pPortCtx->stChnPortAttr.ePixFormat;
                stBufInfo.stFrameData.eFrameScanMode = pPortCtx->stChnPortAttr.eScanMode;
                stBufInfo.stFrameData.eFieldType = _mi_vif_TransMiCapSelToSysFieldType(pPortCtx->stChnPortAttr.eCapSel);
                stBufInfo.stFrameData.stWindowRect.u16X = stFbInfo.nCropX;
                stBufInfo.stFrameData.stWindowRect.u16Y = stFbInfo.nCropY;
                stBufInfo.stFrameData.stWindowRect.u16Width = stFbInfo.nCropW;
                stBufInfo.stFrameData.stWindowRect.u16Height = stFbInfo.nCropH;
                stBufInfo.stFrameData.u16Width = stFbInfo.nCropW;
                stBufInfo.stFrameData.u16Height = stFbInfo.nCropH;
                stBufInfo.stFrameData.u64PhyAddr[0] = stFbInfo.nPhyBufAddr;// Y ???
                stBufInfo.stFrameData.u64PhyAddr[1] = stFbInfo.nPhyBufAddr;// UV ??
                stBufInfo.stFrameData.u32Stride[0]; // TODO
                stBufInfo.stFrameData.u32Stride[1]; // TODO
                mi_sys_FinishBuf(&stBufInfo);
            }
        }
    }

}

void VIF_WORK_Thread()
{    
    do
    {
        _mi_vif_process_in();
        _mi_vif_process_out();
        wait_for_completion(&mi_vif_process_wakeup);
    } while (!kthread_should_stop());
}

void VIF_8051_ISR(void *pdata)
{
    complete(&mi_vif_process_wakeup);
}

MI_S32 _MI_VIF_OnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort , void *pUsrData)
{
    //pstChnCurryPort: port in my module. pstChnPeerPort is the port other module. pUsrData is reserved.
    MI_S32 s32Ret = MI_DIVP_OK;

    ///1 check input parameter
    MI_VIF_CHECK_POINTER(pstChnCurryPort)
    MI_VIF_CHECK_POINTER(pstChnPeerPort)

    if(pstChnCurryPort->eModId != E_MI_SYS_MODULE_ID_VIF)
        return xxx;

    if(pstChnCurryPort->s32ChnId   < 0 ||  pstChnCurryPort->s32ChnId >= 16)
        return xxx;

    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;

    if(pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP
            || pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VPE)
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stPeerInputPortInfo = *pstChnPeerPort;
        return MI_OK;
    }
    else
    {
        return MI_ERR_NOT_SUPPORT;
    }

    return s32Ret;
}


static MI_RESULT _MI_VIF_OnUnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort , void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;

    // modify
    if((pstChnCurryPort->eModId == E_MI_SYS_MODULE_ID_VPE)          // Check Callback information ok ?
            && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->s32ChnId)   // Check support Channel
            && MI_VPE_CHECK_PORT_SUPPORTED(pstChnCurryPort->s32PortId)  // Check support Port Id
            && ((pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP)     // supported input module
                || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VENC)
                || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VDISP)
                || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DISP))
      )
    {
        memset(pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stOutPortInfo[pstChnCurryPort->s32PortId].stPeerOutputPortInfo, 0,    sizeof(MI_SYS_ChnPort_t));
        return MI_OK;
    }
    else
    {
        return MI_FAIL;
    }

    // Check Busy ??
    // if(list_empty(&VPE_working_task_list))
}

static MI_S32 VIF_Init()
{
    mi_sys_ModuleDevBindOps_t stVIFPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    stVIFPOps.OnBindOutputPort   = _MI_VIF_OnBindChnnOutputCallback;
    stVIFPOps.OnUnBindOutputPort = _MI_VIF_OnUnBindChnnOutputCallback;

    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId        = E_MI_SYS_MODULE_ID_VIF;
    stModInfo.u32DevId         = 0;
    stModInfo.u32DevChnNum     = MI_VIF_MAX_PHYCHN_NUM;
    stModInfo.u32InputPortNum  = 0;
    stModInfo.u32OutputPortNum = 2;
    hDevSysHandle  = mi_sys_RegisterDev(&stModInfo, &stVIFPOps, NULL);

    HalVifInit();
    init_completion(&mi_vif_process_wakeup);
#if 0
    mi_vif_in_tsk  = kthread_run(VIF_IN_Thread, NULL , "mi_vif_in_tsk");
    if (IS_ERR(mi_vif_in_tsk)) {
            printk(KERN_INFO "create mi_vif_in_tsk failed!\n");
            return -1;
    }

    mi_vif_out_tsk = kthread_run(VIF_OUT_Thread, NULL , "mi_vif_out_tsk");
    if (IS_ERR(mi_vif_out_tsk)) {
            printk(KERN_INFO "create mi_vif_out_tsk failed!\n");
            return -1;
    }
#else
    mi_vif_work_tsk = kthread_run(VIF_WORK_Thread, NULL , "mi_vif_work_tsk");
    if (IS_ERR(mi_vif_work_tsk)) {
            printk(KERN_INFO "create mi_vif_work_tsk failed!\n");
            return -1;
    }

    request_irq(VIF_8051_ISR_IDX, VIF_8051_ISR, NULL);
#endif
}


static void VIF_Deinit()
{
    HalVifDeinit
    mi_sys_UnRegisterDev

    if (!IS_ERR(mi_vif_in_tsk))
        kthread_stop(mi_vif_in_tsk);

    if (!IS_ERR(mi_vif_out_tsk))
        kthread_stop(mi_vif_out_tsk);
}


MI_RESULT MI_VIF_SetDevAttr(MI_U32 u32VifDev, MI_VIF_DevAttr_t *pstDevAttr)
{
    HalVifDevCfg_t  stDevAttr;

    memcpy(&stDevAttr.as32AdChnId[0], &pstDevAttr->as32AdChnId[0], 16);
    stDevAttr.eIntfMode           = pstDevAttr->eIntfMode;
    stDevAttr.eWorkMode           = pstDevAttr->eWorkMode;
    memcpy(&stDevAttr.au32CompMask[0], &pstDevAttr->au32CompMask[0], 8);
    stDevAttr.eClkEdge            = pstDevAttr->eClkEdge;
    memcpy(&stDevAttr.as32AdChnId[0], &pstDevAttr->as32AdChnId[0], 16);
    stDevAttr.eDataSeq            = pstDevAttr->eDataSeq;
    stDevAttr.stSynCfg            = pstDevAttr->stSynCfg;
    stDevAttr.eInputDataType      = pstDevAttr->eInputDataType;
    stDevAttr.bDataRev            = pstDevAttr->bDataRev;

    MHalVifDevSetConfig(&stDevAttr);

    memcpy(&gstVifDevCtx.stDevAttr, &stDevAttr, sizeof(MI_VIF_DevAttr_t));
    return MI_OK;
}

MI_RESULT MI_VIF_GetDevAttr(MI_U32 u32VifDev, MI_VIF_DevAttr_t *pstDevAttr)
{
    memcpy(pstDevAttr, &gstVifDevCtx.stDevAttr, sizeof(MI_VIF_DevAttr_t));
    return MI_OK;
}

MI_RESULT MI_VIF_EnableDev(MI_U32 u32VifDev)
{
    mi_vif_DevContex_t* pDevctx = &gstVifDevCtx[u32VifDev];

    if(pDevctx.bEnable == FALSE)
    {
        MHalVifDevEnable(u32VifDev);
        pDevctx.bEnable = TRUE;
    }

    return MI_OK;
}

MI_RESULT MI_VIF_DisableDev(MI_U32 u32VifDev)
{
    mi_vif_DevContex_t* pDevctx = &gstVifDevCtx[u32VifDev];

    if(pDevctx.bEnable == TRUE)
    {
        MHalVifDevDisable(u32VifDev);
        pDevctx.bEnable = FALSE;
    }

    return MI_OK;
}

MI_RESULT MI_VIF_SetChnPortAttr(MI_U32 u32VifChn, MI_U32 u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr)
{
    mi_vif_PortContex_t* pChnCtx = &gstVifChnCtx[u32VifChn][u32ChnPort];

    if(u32ChnPort == 0)
    {
        HalVifChnCfg_t stChnAttr;
        stChnAttr.eCapSel       = (HalVifCapsel_e)pstAttr->eCapSel;
        memcpy(&stChnAttr.stCapRect, &pstAttr->stCapRect, sizeof(MI_Rect_t));
        stChnAttr.eScanMode     = (HalVifScanMode_e)pstAttr->eScanMode;
        stChnAttr.ePixFormat    = (HalVifPixelFormat_e)pstAttr->ePixFormat;
        stChnAttr.u32FrameRate  = pstAttr->u32FrameRate;
        MHalVifChnSetConfig(u32VifChn, &stChnAttr);
    }
    else if(u32ChnPort == 1)
    {
        HalVifSubChnCfg_t stSubChnAttr;
        stSubChnAttr.stDestSize.u32X        = 0;
        stSubChnAttr.stDestSize.u32Y        = 0;
        stSubChnAttr.stDestSize.u32Width    = pstAttr->stDestSize.u32Width;
        stSubChnAttr.stDestSize.u32Height   = pstAttr->stDestSize.u32Height;
        stSubChnAttr.u32FrameRate           = pstAttr->u32FrameRate;
        MHalVifSubChnSetConfig(u32VifChn, &stSubChnAttr);
    }
    else
    {
        return E_MI_ERR_ILLEGAL_PARAM;
    }

    pChnCtx->stChnPortAttr = *pstAttr;

    return MI_OK;
}

MI_RESULT MI_VIF_GetChnPortAttr(MI_U32 u32VifChn, MI_U32 u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr)
{
    mi_vif_PortContex_t* pChnCtx = &gstVifChnCtx[u32VifChn][u32ChnPort];
    memcpy(pstAttr, pChnCtx->stChnPortAttr, sizeof(MI_VIF_ChnPortAttr_t));
    return MI_OK;
}

MI_RESULT MI_VIF_EnableChnPort(MI_U32 u32VifChn, MI_U32 u32ChnPort)
{
    mi_vif_PortContex_t* pChnCtx = &gstVifChnCtx[u32VifChn][u32ChnPort];

    if(pChnCtx.bEnable == FALSE)
    {
        if(u32ChnPort == 0)
            MHalVifChnEnable(u32VifChn);
        else if(u32ChnPort == 1)
            MHalVifSubChnEnable(u32VifChn);
    }

    pChnCtx.bEnable = TRUE;
    return MI_OK;
}
MI_RESULT MI_VIF_DisableChnPort(MI_U32 u32VifChn, MI_U32 u32ChnPort)
{
    mi_vif_PortContex_t* pChnCtx = &gstVifChnCtx[u32VifChn][u32ChnPort];

    if(pChnCtx.bEnable == TRUE)
    {
        if(u32ChnPort == 0)
            MHalVifChnDisable(u32VifChn);
        else if(u32ChnPort == 1)
            MHalVifSubChnDisable(u32VifChn);
    }

    pChnCtx.bEnable == FALSE;
    return MI_OK;
}
MI_RESULT MI_VIF_Query(MI_U32 u32VifChn, MI_U32 , MI_U32 u32ChnPort, MI_VIF_ChnPortStat_t *pstStat)
{
    mi_vif_PortContex_t* pChnCtx = &gstVifChnCtx[u32VifChn][u32ChnPort];

    pstStat.bEnable       = pChnCtx->bEnable;
    pstStat.u32FrmRate    = pChnCtx->stChnPortAttr.u32FrameRate;
    //pstStat.u32IntCnt     = pChnCtx->;    // how support ???
    //pstStat.u32LostInt    =  stpstStat.u32LostInt;  // how support ???
    pstStat.u32PicHeight  =  pChnCtx->stChnPortAttr.stDestSize.u32Height;
    pstStat.u32PicWidth   =  pChnCtx->stChnPortAttr.stDestSize.u32Width;
    pstStat.u32VbFail     =  pChnCtx->u32VbFail;
    return MI_OK;
}

