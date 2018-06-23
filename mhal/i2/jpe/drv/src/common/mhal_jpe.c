/*
 * hal_jpe_device.c
 *
 *  Created on: Aug 14, 2017
 *      Author: giggs.huang
 */

//#include "drv_jpe_kernel.h"
#include "drv_jpe_io.h"
#include "drv_jpe_io_st.h"
#include "_drv_jpe_ctx.h"
#include "_drv_jpe_dev.h"
#include "hal_jpe_ops.h"
#include "hal_jpe_ios.h"
#include "mhal_jpe.h"
#include "mhal_venc.h"
#include "mhal_ut_wrapper.h"
#include "jpegenc_marker.h"

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

#define DCTSIZE2 64
u16 std_luminance_quant_tbl[DCTSIZE2] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};

u16 std_chrominance_quant_tbl[DCTSIZE2] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

static u8 jpe_init_flag = 0;

JPE_IOC_RET_STATUS_e _JpeCtxIoctlInit(JpeCtx_t* pCtxIn, JpeCfg_t* pJpeCfg);
JPE_IOC_RET_STATUS_e _JpeCtxEncodeFrame(JpeCtx_t* pCtxIn, JpeBitstreamInfo_t* pStreamInfo);
JPE_IOC_RET_STATUS_e _JpeCtxGetBitInfo(JpeCtx_t* pCtxIn, JpeBitstreamInfo_t* pStreamInfo);

void _JpeSetHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

MHAL_ErrCode_e _JpeCheckHeader(VOID* pHeader, MS_U32 u32Size)
{
    MHAL_ErrCode_e eError = 0;

    MHAL_VENC_Version_t* ver;

    if (pHeader == NULL) {
        CamOsPrintf("In %s the header is null\n",__func__);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    ver = (MHAL_VENC_Version_t*)pHeader;

    if(ver->u32Size != u32Size) {
        CamOsPrintf("In %s the header has a wrong size %i should be %i\n",__func__,ver->u32Size,u32Size);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    if(ver->s.u8VersionMajor != SPECVERSIONMAJOR ||
            ver->s.u8VersionMinor != SPECVERSIONMINOR) {
        CamOsPrintf("The version does not match\n");
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return eError;
}

static int JpegQuality2Scaling(int quality)
{
    int scaling = 60;

    /* Safety limit on quality factor.    Convert 0 to 1 to avoid zero divide. */
    if(quality <= 0) quality = 1;

    if(quality > 100) quality = 100;

    /* The basic table is used as-is (scaling 100) for a quality of 50.
        * Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
        * note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
        * to make all the table entries 1 (hence, minimum quantization loss).
        * Qualities 1..50 are converted to scaling percentage 5000/Q.
        */
    if(quality < 50)
    {
        scaling = 5000 / quality;
    }
    else
    {
        scaling = 200 - quality * 2;
    }

    return scaling;
}

static int drv_jpegenc_encoder_HwJpegMarkerInit(void *pOutputBuf, int nBufSize, JpeCfg_t *pCInfo)
{
    struct CamOsListHead_t head;
    u32 size;
    void* data;
    u32 yuv422;

    switch(pCInfo->eRawFormat)
    {
    case JPE_RAW_YUYV:
    case JPE_RAW_YVYU:
        yuv422 = 1;
        break;

    case JPE_RAW_NV12:
    case JPE_RAW_NV21:
        yuv422 = 0;
        break;

    default:
        CamOsPrintf("Unknown RAW format\n");
        return 0;
    }

    /* Initialize & create baseline marker */
    drv_jpegenc_marker_init(&head);
    drv_jpegenc_marker_create_baseline(&head, yuv422, pCInfo->nCropW, pCInfo->nCropH, pCInfo->YQTable, pCInfo->CQTable, pCInfo->nQScale);

#if 1
    /* Add dummy app 15 for alignment */
    size = 0x10 - ((u32)(pOutputBuf + drv_jpegenc_marker_size(&head)) & 0xF);

    if(size)
    {
        if(size < 4)
            size += 0x10 - 4;
        else
            size -= 4;

        //CamOsPrintf("In %s() Dummy app 15 size is %ld\n", __func__, size);

        data = drv_jpegenc_marker_create_app_n(&head, JPEG_MARKER_APP15, size);
        memset(data, 0x00, size);
    }
#endif

    /* Check necessary size of marker, we do not expect that buffer size is smaller than marker size. */
    size = drv_jpegenc_marker_size(&head);
    if(nBufSize < size)
    {
        return 0;
    }

    /* Dump markers to the output buffer */
    size = drv_jpegenc_marker_dump(&head, pOutputBuf);

    /* free maker */
    drv_jpegenc_marker_release(&head);

    //CamOsPrintf("Out %s() Total header size is %ld\n", __func__, size);

    return size;
}

MS_S32 MHAL_JPE_Init(void)
{
    MHAL_ErrCode_e eError = 0;

    JpeClkInit();

    jpe_init_flag = 1;

    return eError;
}

MS_S32 MHAL_JPE_CreateDevice(MS_U32 u32DevId, MHAL_VENC_DEV_HANDLE *phDev)
{
    JpeDev_t *mdev = NULL;
    JpeIosCB_t* mios = NULL;
    JpeRegIndex_t mregs;

    *phDev = NULL;

    if(0 == jpe_init_flag)
    {
        CamOsPrintf("JPE Warning!! MHAL_JPE_Init must be executed first!!\n");
        MHAL_JPE_Init();
    }

    do
    {
        if (!(mdev = CamOsMemAlloc(sizeof(JpeDev_t))))
            break;

        CamOsMutexInit(&mdev->m_mutex);
        CamOsTsemInit(&mdev->tGetBitsSem, 1);
        CamOsTcondInit(&mdev->m_wqh);

        if (!(mdev->p_asicip = JpeIosAcquire("jpe")))
            break;
        mios = mdev->p_asicip;

        JpeDevGetResourceMem(u32DevId, &mregs.base, &mregs.size);
        mregs.i_id = 0;
        mios->setBank(mios, &mregs);

#if 0
        if (0 != JpeDevClkInit(mdev, u32DevId))
        {
            //CamOsPrintf("MHAL_JPE_CreateDevice clock init fail\n");
            //break;    // fixme: not break for FPGA easy test
        }
#endif

        if (0 != JpeDevGetResourceIrq(u32DevId, &mdev->irq))
        {
            CamOsPrintf("MHAL_JPE_CreateDevice irq fail (0)\n");
            break;
        }

        *phDev = (MHAL_VENC_DEV_HANDLE)mdev;

        return 0;
    }
    while (0);

    if (mdev)
    {
        JpeIosCB_t* mios = mdev->p_asicip;
        if (mios)
            mios->release(mios);
        CamOsMemRelease(mdev);
    }

    return E_MHAL_ERR_ILLEGAL_PARAM;
}

MS_S32 MHAL_JPE_DestroyDevice(MHAL_VENC_DEV_HANDLE hDev)
{
    MHAL_ErrCode_e eError = 0;
    JpeDev_t *mdev = (JpeDev_t *)hDev;
    JpeIosCB_t* mios = NULL;

    if (mdev)
    {
        mios = mdev->p_asicip;

        JpeDevClkDeinit(mdev);

        if (mios)
            mios->release(mios);
        CamOsMemRelease(mdev);

        eError = 0;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }
    return eError;
}

MS_S32 MHAL_JPE_GetDevConfig(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    JpeDev_t *mdev = (JpeDev_t *)hDev;
    MHAL_ErrCode_e eError;
    MHAL_VENC_ParamInt_t* pVEncInt = NULL;
    MHAL_VENC_ResolutionAlign_t* pVEncResolutionAlign = NULL;

    if (mdev)
    {
        switch (eType)
        {
        case E_MHAL_VENC_HW_IRQ_NUM:
            if(pstParam == NULL)
                return E_MHAL_ERR_NULL_PTR;

            pVEncInt = (MHAL_VENC_ParamInt_t* )pstParam;

            if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_ParamInt_t))))
            {
                break;
            }

            pVEncInt->u32Val = mdev->irq;

            eError = 0;
            break;

        case E_MHAL_VENC_HW_CMDQ_BUF_LEN:
            if(pstParam == NULL)
                return E_MHAL_ERR_NULL_PTR;

            pVEncInt = (MHAL_VENC_ParamInt_t* )pstParam;

            if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_ParamInt_t))))
            {
                break;
            }

            pVEncInt->u32Val = 0;

            eError = 0;
            break;

        case E_MHAL_VENC_HW_RESOLUTION_ALIGN:
            if(pstParam == NULL)
                return E_MHAL_ERR_NULL_PTR;

            pVEncResolutionAlign = (MHAL_VENC_ResolutionAlign_t* )pstParam;

            if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_ResolutionAlign_t))))
            {
                break;
            }

            pVEncResolutionAlign->u32AlignW = 16;
            pVEncResolutionAlign->u32AlignH = 2;
            pVEncResolutionAlign->u32AlignCropX = 256;
            pVEncResolutionAlign->u32AlignCropY = 16;
            pVEncResolutionAlign->u32AlignCropW = 16;
            pVEncResolutionAlign->u32AlignCropH = 2;

            eError = 0;
            break;

        default:
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
            break;
        }
    }
    else
    {
        return E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_JPE_CreateInstance(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst)
{
    JpeDev_t* mdev = (JpeDev_t *)hDev;
    JpeCtx_t* mctx;
    int nRet;
    MHAL_ErrCode_e eError;

    if (!mdev)
        return E_MHAL_ERR_NULL_PTR;

    do
    {
        if (!(mctx = JpeCtxAcquire(mdev)))
        {
            JPE_MSG(JPE_MSG_ERR, "JpeCtxAcquire Fail\n");
            CamOsPrintf("%s:%d\n", __FUNCTION__, __LINE__);
            eError = E_MHAL_ERR_BUSY;
            break;
        }
        if (0 > JpeDevRegister(mdev, mctx))
        {
            JPE_MSG(JPE_MSG_ERR, "JpeDevRegister Fail, ret=%d\n", nRet);
            CamOsPrintf("%s:%d\n", __FUNCTION__, __LINE__);
            eError = E_MHAL_ERR_BUSY;
            break;
        }

        nRet = _DevPowserOn(mdev, JPE_CLOCK_ON);
        if(nRet)
        {
            JPE_MSG(JPE_MSG_ERR, "clk_set_rate() Fail, ret=%d\n", nRet);
            CamOsPrintf("%s:%d\n", __FUNCTION__, __LINE__);
            eError = E_MHAL_ERR_BUSY;
            break;
        }

        mctx->tJpeCfg.nQFactor = 90;
        mctx->tJpeCfg.nQScale = JpegQuality2Scaling(mctx->tJpeCfg.nQFactor);
        memcpy(mctx->tJpeCfg.YQTable, std_luminance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));
        memcpy(mctx->tJpeCfg.CQTable, std_chrominance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));

        mdev->nRefCount++;
        *phInst = mctx;

        return 0;
    }
    while (0);

    if (mctx)
        mctx->release(mctx);

    CamOsPrintf("%s:%d\n", __FUNCTION__, __LINE__);
    return E_MHAL_ERR_ILLEGAL_PARAM;
}

MS_S32 MHAL_JPE_DestroyInstance(MHAL_VENC_INST_HANDLE hInst)
{
    MHAL_ErrCode_e eError = 0;
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    JpeDev_t* mdev = NULL;

    if (mctx)
    {
        mdev = mctx->p_device;
        _DevPowserOn(mdev, JPE_CLOCK_OFF);
        JpeDevUnregister(mctx->p_device, mctx);
        mctx->release(mctx);
        mdev->nRefCount--;
        eError = 0;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_JPE_SetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MHAL_ErrCode_e eError = 0;
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    int i;

    if(NULL == mctx)
    {
        eError = E_MHAL_ERR_NULL_PTR;
        CamOsPrintf("In %s parameter mctx is null! err = %x\n",__func__, eError);
        return eError;
    }

    switch (eType)
    {
        case E_MHAL_VENC_IDX_STREAM_ON:
            {
                // TODO
                break;
            }
        case E_MHAL_VENC_IDX_STREAM_OFF:
            {
                // TODO
                break;
            }
             //Cfg setting
        case E_MHAL_VENC_PARTIAL_NOTIFY:
            {
                MHAL_VENC_PartialNotify_t* pVEncPartialNotify = (MHAL_VENC_PartialNotify_t* )pstParam;
                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_PartialNotify_t))))
                {
                    break;
                }

                mctx->tJpeCfg.nPartialNotifySize =  pVEncPartialNotify->u32NotifySize;
                mctx->tJpeCfg.notifyFunc =  pVEncPartialNotify->notifyFunc;

                break;
            }
        case E_MHAL_VENC_JPEG_RESOLUTION:
            {
                MHAL_VENC_Resoluton_t *pVEncRes = (MHAL_VENC_Resoluton_t *)pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_Resoluton_t))))
                {
                    break;
                }

                mctx->tJpeCfg.nWidth = pVEncRes->u32Width;
                mctx->tJpeCfg.nHeight = pVEncRes->u32Height;
                mctx->tJpeCfg.nCropW = pVEncRes->u32Width;
                mctx->tJpeCfg.nCropH = pVEncRes->u32Height;
                mctx->tJpeCfg.nCropOffsetX = 0;
                mctx->tJpeCfg.nCropOffsetY = 0;
                switch (pVEncRes->eFmt)
                {
                case E_MHAL_VENC_FMT_NV12:
                    mctx->tJpeCfg.eRawFormat = JPE_RAW_NV12;
                    break;
                case E_MHAL_VENC_FMT_NV21:
                    mctx->tJpeCfg.eRawFormat = JPE_RAW_NV21;
                    break;
                case E_MHAL_VENC_FMT_YUYV:
                    mctx->tJpeCfg.eRawFormat = JPE_RAW_YUYV;
                    break;
                case E_MHAL_VENC_FMT_YVYU:
                    mctx->tJpeCfg.eRawFormat = JPE_RAW_YVYU;
                    break;
                default :
                    eError = E_MHAL_ERR_ILLEGAL_PARAM;
                    break;
                }

                break;
            }

        case E_MHAL_VENC_JPEG_CROP:
            {
                MHAL_VENC_CropCfg_t* pVEncCrop = (MHAL_VENC_CropCfg_t *)pstParam;

                if((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_CropCfg_t))))
                {
                    break;
                }

                if((pVEncCrop->stRect.u32W == 0) || ( pVEncCrop->stRect.u32H == 0))
                {
                    eError = E_MHAL_ERR_ILLEGAL_PARAM;
                    break;
                }

               if(((pVEncCrop->stRect.u32X+pVEncCrop->stRect.u32W) > mctx->tJpeCfg.nWidth) ||
                       ((pVEncCrop->stRect.u32Y+ pVEncCrop->stRect.u32H) > mctx->tJpeCfg.nHeight))
                {
                    eError = E_MHAL_ERR_ILLEGAL_PARAM;
                    break;
                }

                if((pVEncCrop->stRect.u32X % 256) || (pVEncCrop->stRect.u32Y % 16))
                {
                    eError = E_MHAL_ERR_ILLEGAL_PARAM;
                    break;
                }

                mctx->tJpeCfg.nCropW = pVEncCrop->stRect.u32W;
                mctx->tJpeCfg.nCropH = pVEncCrop->stRect.u32H;
                mctx->tJpeCfg.nCropOffsetX =  pVEncCrop->stRect.u32X;
                mctx->tJpeCfg.nCropOffsetY = pVEncCrop->stRect.u32Y;
                break;
            }

        case E_MHAL_VENC_JPEG_RC:
            {
                MHAL_VENC_RcInfo_t* pVEncRcInfo = (MHAL_VENC_RcInfo_t* )pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_RcInfo_t))))
                {
                    break;
                }

                mctx->tJpeCfg.nQFactor = pVEncRcInfo->stAttrMJPGRc.u32Qfactor;
                mctx->tJpeCfg.nQScale = JpegQuality2Scaling(mctx->tJpeCfg.nQFactor);

                for (i=0; i<DCTSIZE2; i++)
                    mctx->tJpeCfg.YQTable[i] = pVEncRcInfo->stAttrMJPGRc.u8YQt[i];
                for (i=0; i<DCTSIZE2; i++)
                    mctx->tJpeCfg.CQTable[i] = pVEncRcInfo->stAttrMJPGRc.u8CbCrQt[i];

                break;
            }

        case E_MHAL_VENC_RESTORE:
            {
                CamOsPrintf("E_MHAL_VENC_RESTORE is unsupport in JPE\n");
                break;
            }

        case E_MHAL_VENC_FRAME_CFG:
            {
                MHAL_VENC_FrameCfg_t* pVEncFrameCfg = (MHAL_VENC_FrameCfg_t *)pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_FrameCfg_t))))
                {
                    break;
                }

                mctx->i_threshold = pVEncFrameCfg->u32IFrmBitsThr;
                break;
            }

        default:
            CamOsPrintf("In %s command is not support! err = %x\n",__func__, eError);
            eError = E_MHAL_ERR_NOT_SUPPORT;
            break;

    }

    return eError;
}

MS_S32 MHAL_JPE_GetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MHAL_ErrCode_e eError;
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    int i;

    if(mctx == NULL)
        return E_MHAL_ERR_NULL_PTR;
    if(pstParam == NULL)
        return E_MHAL_ERR_NULL_PTR;

    switch (eType)
    {
        //Cfg setting
        case E_MHAL_VENC_PARTIAL_NOTIFY:
            {
                MHAL_VENC_PartialNotify_t *pVEncPartialNotify = (MHAL_VENC_PartialNotify_t* )pstParam;
                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_PartialNotify_t))))
                {
                    break;
                }

                pVEncPartialNotify->u32NotifySize = mctx->tJpeCfg.nPartialNotifySize;
                pVEncPartialNotify->notifyFunc = mctx->tJpeCfg.notifyFunc;

                break;
            }
        case E_MHAL_VENC_JPEG_RESOLUTION:
            {
                MHAL_VENC_Resoluton_t *pVEncRes = (MHAL_VENC_Resoluton_t *)pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_Resoluton_t))))
                {
                    break;
                }

                pVEncRes->u32Width = mctx->tJpeCfg.nWidth;
                pVEncRes->u32Height = mctx->tJpeCfg.nHeight;
                switch (mctx->tJpeCfg.eRawFormat)
                {
                case JPE_RAW_NV12:
                    pVEncRes->eFmt = E_MHAL_VENC_FMT_NV12;
                    break;
                case JPE_RAW_NV21:
                    pVEncRes->eFmt = E_MHAL_VENC_FMT_NV21;
                    break;
                case JPE_RAW_YUYV:
                    pVEncRes->eFmt = E_MHAL_VENC_FMT_YUYV;
                    break;
                case JPE_RAW_YVYU:
                    pVEncRes->eFmt = E_MHAL_VENC_FMT_YVYU;
                    break;
                default :
                    eError = E_MHAL_ERR_ILLEGAL_PARAM;
                    break;
                }

                break;
            }

        case E_MHAL_VENC_JPEG_CROP:
            {
                MHAL_VENC_CropCfg_t* pVEncCropCfg = (MHAL_VENC_CropCfg_t*)pstParam;

                if((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_CropCfg_t))))
                {
                    break;
                }

                pVEncCropCfg->stRect.u32X = mctx->tJpeCfg.nCropOffsetX;
                pVEncCropCfg->stRect.u32Y = mctx->tJpeCfg.nCropOffsetY;
                pVEncCropCfg->stRect.u32W = mctx->tJpeCfg.nCropW;
                pVEncCropCfg->stRect.u32H = mctx->tJpeCfg.nCropH;

                break;
            }

        case E_MHAL_VENC_JPEG_RC:
            {
                MHAL_VENC_RcInfo_t* pVEncRcInfo = (MHAL_VENC_RcInfo_t* )pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_RcInfo_t))))
                {
                    break;
                }

                pVEncRcInfo->stAttrMJPGRc.u32Qfactor = mctx->tJpeCfg.nQFactor;

                for (i=0; i<DCTSIZE2; i++)
                    pVEncRcInfo->stAttrMJPGRc.u8YQt[i] = mctx->tJpeCfg.YQTable[i];
                for (i=0; i<DCTSIZE2; i++)
                    pVEncRcInfo->stAttrMJPGRc.u8CbCrQt[i] = mctx->tJpeCfg.CQTable[i];

                break;
            }

        case E_MHAL_VENC_FRAME_CFG:
            {
                MHAL_VENC_FrameCfg_t* pVEncFrameCfg = (MHAL_VENC_FrameCfg_t* )pstParam;

                if ((eError = _JpeCheckHeader(pstParam, sizeof(MHAL_VENC_FrameCfg_t))))
                {
                    break;
                }

                pVEncFrameCfg->s8QpOffset = 0;
                pVEncFrameCfg->u32IFrmBitsThr = mctx->i_threshold;
                pVEncFrameCfg->u32PFrmBitsThr = 0;

                break;
            }

        default:
            eError = E_MHAL_ERR_NOT_SUPPORT;
            break;
    }

    return eError;
}

MS_S32 MHAL_JPE_EncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pstInOutBuf)
{
    MHAL_ErrCode_e eError = 0;
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    JpeCfg_t tJpeCfg;
    JpeBitstreamInfo_t tStreamInfo;
    JPE_IOC_RET_STATUS_e nRet;
    MS_U64 ybias = 0;
    MS_U64 cbias = 0;
    MS_U32 o_size = pstInOutBuf->u32OutputBufSize;

    if (mctx && pstInOutBuf)
    {
        tJpeCfg.eInBufMode       = JPE_IBUF_FRAME_MODE;
        tJpeCfg.eRawFormat       = mctx->tJpeCfg.eRawFormat;
        tJpeCfg.eCodecFormat     = JPE_CODEC_JPEG;
        tJpeCfg.nWidth           = mctx->tJpeCfg.nWidth;
        tJpeCfg.nHeight          = mctx->tJpeCfg.nHeight;
        tJpeCfg.nCropOffsetX = mctx->tJpeCfg.nCropOffsetX;
        tJpeCfg.nCropOffsetY = mctx->tJpeCfg.nCropOffsetY;
        tJpeCfg.nCropW = mctx->tJpeCfg.nCropW;
        tJpeCfg.nCropH = mctx->tJpeCfg.nCropH;

        tJpeCfg.nQFactor = mctx->tJpeCfg.nQFactor;
        memcpy(tJpeCfg.YQTable, mctx->tJpeCfg.YQTable, DCTSIZE2 * sizeof(unsigned short));
        memcpy(tJpeCfg.CQTable, mctx->tJpeCfg.CQTable, DCTSIZE2 * sizeof(unsigned short));
        tJpeCfg.nQScale          = mctx->tJpeCfg.nQScale;
        if(JPE_RAW_YUYV == tJpeCfg.eRawFormat || JPE_RAW_YVYU == tJpeCfg.eRawFormat)
        {
            ybias =  ((tJpeCfg.nWidth * tJpeCfg.nCropOffsetY) + tJpeCfg.nCropOffsetX) << 1;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned long)pstInOutBuf->phyInputYUVBuf1 + ybias;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nSize = tJpeCfg.nWidth * tJpeCfg.nHeight * 2;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = 0;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nSize = 0;
        }
        else if(JPE_RAW_NV12 == tJpeCfg.eRawFormat || JPE_RAW_NV21 == tJpeCfg.eRawFormat)
        {
            ybias =  (tJpeCfg.nWidth * tJpeCfg.nCropOffsetY) + tJpeCfg.nCropOffsetX;
            cbias =  ((tJpeCfg.nWidth * tJpeCfg.nCropOffsetY) >> 1) + tJpeCfg.nCropOffsetX;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned long)pstInOutBuf->phyInputYUVBuf1 + ybias;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nSize = tJpeCfg.nWidth * tJpeCfg.nHeight;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = (unsigned long)pstInOutBuf->phyInputYUVBuf2 + cbias;
            tJpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nSize = tJpeCfg.nWidth*tJpeCfg.nHeight/2;
        }

        if(mctx->i_threshold && (pstInOutBuf->u32OutputBufSize > mctx->i_threshold))
            o_size = mctx->i_threshold;

        mctx->nHeaderLen = drv_jpegenc_encoder_HwJpegMarkerInit((void *)(unsigned long)pstInOutBuf->virtOutputBuf, o_size, &tJpeCfg);

        tJpeCfg.OutBuf.nAddr = (unsigned long)pstInOutBuf->phyOutputBuf + mctx->nHeaderLen;
        tJpeCfg.OutBuf.nSize = o_size - mctx->nHeaderLen;
        tJpeCfg.nJpeOutBitOffset = 0;
        tJpeCfg.nHeaderLen = mctx->nHeaderLen;
        tJpeCfg.nPartialNotifySize = mctx->tJpeCfg.nPartialNotifySize;
        tJpeCfg.notifyFunc = mctx->tJpeCfg.notifyFunc;
        /*CamOsPrintf("%s:%d  %d %d %d\n", __FUNCTION__, __LINE__,
                tJpeCfg.eRawFormat,
                tJpeCfg.nWidth,
                tJpeCfg.nHeight);*/

        nRet = _JpeCtxIoctlInit(mctx, &tJpeCfg);
        if(nRet != JPE_IOC_RET_SUCCESS)
        {
            CamOsPrintf("_JpeCtxIoctlInit Fail , err %d\n", nRet);
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
        }

        #if (NON_BLOCKING_MODE == 0)
        nRet = _JpeCtxEncodeFrame(mctx, &tStreamInfo);
        if(nRet != JPE_IOC_RET_SUCCESS)
        {
            CamOsPrintf("_JpeCtxEncodeFrame Fail , err %d\n", nRet);
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
        }
        #else
        memset(&tStreamInfo, 0, sizeof(JpeBitstreamInfo_t));
        nRet = JpeCtxEncFireAndReturn(mctx);
        if(nRet != JPE_IOC_RET_SUCCESS)
        {
            CamOsPrintf("JpeCtxEncFireAndReturn Fail , err %d\n", nRet);
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
        }
        #endif

        eError = 0;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_JPE_EncodeFrameDone(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_EncResult_t* pstEncRet)
{
    MHAL_ErrCode_e eError = 0;
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    JpeBitstreamInfo_t tStreamInfo;
    JPE_IOC_RET_STATUS_e nRet;

    if (mctx && pstEncRet)
    {
        #if(NON_BLOCKING_MODE == 0)
        nRet = _JpeCtxGetBitInfo(mctx, &tStreamInfo);
        if(nRet != JPE_IOC_RET_SUCCESS)
        {
            CamOsPrintf("_JpeCtxGetBitInfo Fail , err %d\n", nRet);
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
        }
        #else
        nRet = JpeCtxEncPostProc(mctx, &tStreamInfo);
        if(nRet != JPE_IOC_RET_SUCCESS)
        {
            CamOsPrintf("JpeCtxEncPostProc Fail , err %d\n", nRet);
            eError = E_MHAL_ERR_ILLEGAL_PARAM;
        }
        #endif

        pstEncRet->u32OutputBufUsed = tStreamInfo.nOutputSize + mctx->nHeaderLen;

        eError = 0;
    }
    else
    {
        eError = E_MHAL_ERR_NULL_PTR;
    }

    return eError;
}

MS_S32 MHAL_JPE_QueryBufSize(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstParam)
{
    if (pstParam)
    {
        pstParam->u32IntrAlBufSize = 0;
        pstParam->u32IntrRefBufSize = 0;
    }

    return 0;
}

MS_S32 MHAL_JPE_IsrProc(MHAL_VENC_DEV_HANDLE hDev)
{
    JpeDev_t *mdev = (JpeDev_t *)hDev;

    if (mdev)
        return JpeDevIsrFnx(mdev);

    return 0;
}

MS_S32 MHAL_JPE_GetEngStatus(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_EncStatus_t *pstParam)
{
    MHAL_ErrCode_e eError = 0;
    JpeDev_t *mdev = (JpeDev_t *)hDev;
    int irqSts;
    u32 phyInAddr;
    u32 phyOutAddr;

    if(CamOsInInterrupt())
    {
        CamOsPrintf("MHAL_JPE_GetEngStatus can't run in thread context \n");
        return E_MHAL_ERR_NOT_PERM;
    }

    if(mdev)
    {
        eError = JpeDevGetEngStatus(mdev, &irqSts, &phyInAddr, &phyOutAddr);
        if(irqSts == 1)   // JPE_DEV_BUSY
            pstParam->eEncSts =  E_MHAL_VENC_BUF_RDY;
        else if(irqSts == 2)   // JPE_DEV_ENC_DONE
            pstParam->eEncSts = E_MHAL_VENC_FRM_DONE;
        else if(irqSts == 3)    // JPE_DEV_OUTBUF_FULL
            pstParam->eEncSts = E_MHAL_VENC_BUF_RDY_UNDER_RUN;
        else
            pstParam->eEncSts = E_MHAL_VENC_OTHERS;

        if(mdev->i_usersidx >= 0)
            pstParam->phInst = (MHAL_VENC_INST_HANDLE)mdev->user[mdev->i_usersidx].pCtx;
        else
            pstParam->phInst = NULL;

        pstParam->phyCurrInBuf = (MS_U64)phyInAddr;
        pstParam->phyCurrOutBuf = (MS_U64)phyOutAddr;

        return eError;
    }

    eError = E_MHAL_ERR_NULL_PTR;
    return eError;
}

MS_S32 MHAL_JPE_EncodeAddOutBuff(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_AddOutBuf_t *pstAddOutBuf)
{
    JpeDev_t *mdev = (JpeDev_t *)hDev;

    if(mdev)
    {
        if(mdev->i_state == JPE_DEV_STATE_BUSY)
        {
            return JpeDevAddOutBuff(mdev,  (u32)pstAddOutBuf->phyOutBuff, pstAddOutBuf->u32OutBuffSize);
        }
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return E_MHAL_ERR_NULL_PTR;
}

MS_S32 MHAL_JPE_EncodeCancel(MHAL_VENC_INST_HANDLE hInst)
{
    JpeCtx_t* mctx = (JpeCtx_t*)hInst;
    JpeDev_t* mdev = mctx->p_device;

    if(mdev)
    {
        if(mdev->i_state == JPE_DEV_STATE_BUSY)
        {
            mctx->i_state = JPE_CTX_STATE_IDLE;
            return JpeDevEncodeCancel(mdev);  //pDev->i_state = JPE_DEV_STATE_IDLE;
        }
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return E_MHAL_ERR_NULL_PTR;
}
