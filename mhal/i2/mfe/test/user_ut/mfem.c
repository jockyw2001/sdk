
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <mdrv_rqct_io.h>
#include <mdrv_mfe_io.h>
#include <mhal_venc.h>
#include "mhal_ut_wrapper.h"
#include "mfe_utility.h"
#include "cam_os_wrapper.h"
#include "md5.h"

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

void setHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

MHAL_ErrCode_e checkHeader(VOID* header, MS_U32 size)
{
    MHAL_ErrCode_e eError = 0;

    MHAL_VENC_Version_t* ver;

    if (header == NULL) {
        CamOsPrintf("In %s the header is null\n",__func__);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    ver = (MHAL_VENC_Version_t*)header;

    if(ver->u32Size != size) {
        CamOsPrintf("In %s the header has a wrong size %i should be %i\n",__func__,ver->u32Size,(int)size);
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    if(ver->s.u8VersionMajor != SPECVERSIONMAJOR ||
            ver->s.u8VersionMinor != SPECVERSIONMINOR) {
        CamOsPrintf("The version does not match\n");
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    return eError;
}

int _drawQpMap(MS_U8 * addr)
{

    memset(addr, 0, 200);

    memset(addr+500, 3, 200);

    memset(addr+1000, 5, 200);

    memset(addr+1500, 13, 200);

    return 0;
}

int _SetMfeParameterByMhal(Mfe_param* pParam)
{
    ERROR err = ErrorNone;
    VencParamUT param = {0};
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stMfeIntrBuf;
    MHAL_VENC_ParamSplit_t SliceCtl;
    MHAL_VENC_RoiCfg_t RoiCtl;
    MHAL_VENC_ParamH264Entropy_t EntropyCtl;
    MHAL_VENC_ParamH264Dblk_t DblkCtl;
    MHAL_VENC_ParamH264Trans_t TransCtl;
    MHAL_VENC_ParamH264InterPred_t InterPredCtl;
    MHAL_VENC_ParamH264IntraPred_t IntraPredCtl;
    //MHAL_VENC_UserData_t UserData;
    MHAL_VENC_ParamH264Vui_t VuiCtl;
    MHAL_VENC_ParamRef_t RefCtl;
    u32 nRoiCnt;
    int i;

    /* Set MFE parameter by iocal */
    memset(&ResCtl, 0, sizeof(ResCtl));
    setHeader(&ResCtl, sizeof(ResCtl));
    ResCtl.u32Width = pParam->basic.w;
    ResCtl.u32Height = pParam->basic.h;
    if (strncmp(pParam->basic.pixfm, "NV12", 4) == 0)
    {
        ResCtl.eFmt = E_MHAL_VENC_FMT_NV12;
    }
    else if (strncmp(pParam->basic.pixfm, "NV21", 4) == 0)
    {
        ResCtl.eFmt = E_MHAL_VENC_FMT_NV21;
    }
    else if (strncmp(pParam->basic.pixfm, "YUYV", 4) == 0)
    {
        ResCtl.eFmt = E_MHAL_VENC_FMT_YUYV;
    }
    else if (strncmp(pParam->basic.pixfm, "YVYU", 4) == 0)
    {
        ResCtl.eFmt = E_MHAL_VENC_FMT_YVYU;
    }
    param.type = E_MHAL_VENC_264_RESOLUTION;
    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    switch(pParam->sRC.eControlRate)
    {
    case Mfe_RQCT_METHOD_CBR:
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H264CBR;
        RcCtl.stAttrH264Cbr.u32Gop              = pParam->basic.Gop;
        RcCtl.stAttrH264Cbr.u32StatTime         = 1;    // TBD
        RcCtl.stAttrH264Cbr.u32SrcFrmRateNum    = pParam->basic.fps;
        RcCtl.stAttrH264Cbr.u32SrcFrmRateDen    = 1;
        RcCtl.stAttrH264Cbr.u32BitRate          = pParam->basic.bitrate;
        RcCtl.stAttrH264Cbr.u32FluctuateLevel   = 0;    // TBD
        break;
    case Mfe_RQCT_METHOD_VBR:
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H264VBR;
        RcCtl.stAttrH264Vbr.u32Gop              = pParam->basic.Gop;
        RcCtl.stAttrH264Vbr.u32StatTime         = 1;    // TBD
        RcCtl.stAttrH264Vbr.u32SrcFrmRateNum    = pParam->basic.fps;
        RcCtl.stAttrH264Vbr.u32SrcFrmRateDen    = 1;
        RcCtl.stAttrH264Vbr.u32MaxBitRate       = pParam->basic.bitrate;
        RcCtl.stAttrH264Vbr.u32MaxQp            = pParam->sRC.nImaxqp;
        RcCtl.stAttrH264Vbr.u32MinQp            = pParam->sRC.nIminqp;
        break;
    case Mfe_RQCT_METHOD_CQP:
    default :
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H264FIXQP;
        RcCtl.stAttrH264FixQp.u32SrcFrmRateNum  = pParam->basic.fps;
        RcCtl.stAttrH264FixQp.u32SrcFrmRateDen  = 1;
        RcCtl.stAttrH264FixQp.u32Gop            = pParam->basic.Gop;
        RcCtl.stAttrH264FixQp.u32IQp            = pParam->basic.nQp;
        RcCtl.stAttrH264FixQp.u32PQp            = pParam->basic.nQp;
        break;
    }
    param.type = E_MHAL_VENC_264_RC;
    param.param = (void *)&RcCtl;
    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_RT_RC Error = %x\n",__func__,err);
    }


    // Set Entropy
    if (pParam->sEntropy.bEn)
    {
        memset(&EntropyCtl, 0, sizeof(EntropyCtl));
        setHeader(&EntropyCtl, sizeof(EntropyCtl));
        EntropyCtl.u32EntropyEncModeI = pParam->sEntropy.bEntropyCodingCABAC;
        EntropyCtl.u32EntropyEncModeP = pParam->sEntropy.bEntropyCodingCABAC;
        param.type = E_MHAL_VENC_264_ENTROPY;
        param.param = (void *)&EntropyCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_ENTROPY Error = %x\n",__func__,err);
        }
    }


    // Set Deblocking
    if (pParam->sDeblk.bEn)
    {
        memset(&DblkCtl, 0, sizeof(DblkCtl));
        setHeader(&DblkCtl, sizeof(DblkCtl));
        DblkCtl.disable_deblocking_filter_idc   = pParam->sDeblk.disable_deblocking_filter_idc;
        DblkCtl.slice_alpha_c0_offset_div2      = pParam->sDeblk.slice_alpha_c0_offset_div2;
        DblkCtl.slice_beta_offset_div2          = pParam->sDeblk.slice_beta_offset_div2;
        param.type = E_MHAL_VENC_264_DBLK;
        param.param = (void *)&DblkCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_DBLK Error = %x\n",__func__,err);
        }
    }


    // Set Trans Mode
    memset(&TransCtl, 0, sizeof(TransCtl));
    setHeader(&TransCtl, sizeof(TransCtl));
    TransCtl.u32IntraTransMode = 0;
    TransCtl.u32InterTransMode = 0;
    TransCtl.s32ChromaQpIndexOffset = 0;
    param.type = E_MHAL_VENC_264_TRANS;
    param.param = (void *)&TransCtl;
    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_TRANS Error = %x\n",__func__,err);
    }


    // Set Inter Pred
    if (pParam->sInterP.bEn)
    {
        memset(&InterPredCtl, 0, sizeof(InterPredCtl));
        setHeader(&InterPredCtl, sizeof(InterPredCtl));
        InterPredCtl.u32HWSize = pParam->sInterP.nDmv_X;
        InterPredCtl.u32VWSize = pParam->sInterP.nDmv_Y;
        InterPredCtl.bInter16x16PredEn = pParam->sInterP.bInter16x16PredEn;
        InterPredCtl.bInter16x8PredEn = pParam->sInterP.bInter16x8PredEn;
        InterPredCtl.bInter8x16PredEn = pParam->sInterP.bInter8x16PredEn;
        InterPredCtl.bInter8x8PredEn = pParam->sInterP.bInter8x8PredEn;
        InterPredCtl.bExtedgeEn = 1;
        param.type = E_MHAL_VENC_264_INTER_PRED;
        param.param = (void *)&InterPredCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_INTER_PRED Error = %x\n",__func__,err);
        }
    }


    // Set Intra Pred
    if (pParam->sIntraP.bEn)
    {
        memset(&IntraPredCtl, 0, sizeof(IntraPredCtl));
        setHeader(&IntraPredCtl, sizeof(IntraPredCtl));
        IntraPredCtl.bIntra16x16PredEn = 1;
        IntraPredCtl.bIntraNxNPredEn = 1;
        IntraPredCtl.constrained_intra_pred_flag = pParam->sIntraP.bconstIpred;
        IntraPredCtl.bIpcmEn = 0;
        IntraPredCtl.u32Intra16x16Penalty = 0;
        IntraPredCtl.u32Intra4x4Penalty = 0;
        IntraPredCtl.bIntraPlanarPenalty = 0;
        param.type = E_MHAL_VENC_264_INTRA_PRED;
        param.param = (void *)&IntraPredCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_INTRA_PRED Error = %x\n",__func__,err);
        }
    }

    // Set VUI
    if(pParam->sVUI.bEn)
    {
        memset(&VuiCtl, 0, sizeof(VuiCtl));
        setHeader(&VuiCtl, sizeof(VuiCtl));

        VuiCtl.stVuiVideoSignal.u8VideoFullRangeFlag = pParam->sVUI.u8VideoFullRangeFlag;
        VuiCtl.stVuiVideoSignal.u8VideoFormat = pParam->sVUI.u8VideoFormat;
        VuiCtl.stVuiVideoSignal.u8VideoSignalTypePresentFlag = pParam->sVUI.u8VideoSignalTypePresentFlag;
        VuiCtl.stVuiVideoSignal.u8ColourDescriptionPresentFlag = pParam->sVUI.u8ColourDescriptionPresentFlag;
        VuiCtl.stVuiTimeInfo.u8TimingInfoPresentFlag = pParam->sVUI.u8TimingInfoPresentFlag;
        VuiCtl.stVuiAspectRatio.u16SarWidth = pParam->sVUI.u16SarWidth;
        VuiCtl.stVuiAspectRatio.u16SarHeight = pParam->sVUI.u16SarHeight;

        CamOsPrintf("Set VuiCtl.stVuiVideoSignal.u8VideoFullRangeFlag = %d\n", VuiCtl.stVuiVideoSignal.u8VideoFullRangeFlag);
        CamOsPrintf("Set VuiCtl.stVuiVideoSignal.u8VideoFormat = %d\n", VuiCtl.stVuiVideoSignal.u8VideoFormat);
        CamOsPrintf("Set VuiCtl.stVuiVideoSignal.u8VideoSignalTypePresentFlag = %d\n", VuiCtl.stVuiVideoSignal.u8VideoSignalTypePresentFlag);
        CamOsPrintf("Set VuiCtl.stVuiVideoSignal.u8ColourDescriptionPresentFlag = %d\n", VuiCtl.stVuiVideoSignal.u8ColourDescriptionPresentFlag);
        CamOsPrintf("Set VuiCtl.stVuiTimeInfo.u8TimingInfoPresentFlag = %d\n", VuiCtl.stVuiTimeInfo.u8TimingInfoPresentFlag);
        CamOsPrintf("Set VuiCtl.stVuiAspectRatio.u16SarWidth = %d\n", VuiCtl.stVuiAspectRatio.u16SarWidth);
        CamOsPrintf("Set VuiCtl.stVuiAspectRatio.u16SarHeight = %d\n", VuiCtl.stVuiAspectRatio.u16SarHeight);

        param.type = E_MHAL_VENC_264_VUI;
        param.param = (void *)&VuiCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_VUI Error = %x\n",__func__,err);
        }

        memset(&VuiCtl, 0, sizeof(VuiCtl));
        setHeader(&VuiCtl, sizeof(VuiCtl));

        param.type = E_MHAL_VENC_264_VUI;
        param.param = (void *)&VuiCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_264_VUI Error = %x\n",__func__,err);
        }

        CamOsPrintf("Get VuiCtl.stVuiVideoSignal.u8VideoFullRangeFlag = %d\n", VuiCtl.stVuiVideoSignal.u8VideoFullRangeFlag);
        CamOsPrintf("Get VuiCtl.stVuiVideoSignal.u8VideoFormat = %d\n", VuiCtl.stVuiVideoSignal.u8VideoFormat);
        CamOsPrintf("Get VuiCtl.stVuiVideoSignal.u8VideoSignalTypePresentFlag = %d\n", VuiCtl.stVuiVideoSignal.u8VideoSignalTypePresentFlag);
        CamOsPrintf("Get VuiCtl.stVuiVideoSignal.u8ColourDescriptionPresentFlag = %d\n", VuiCtl.stVuiVideoSignal.u8ColourDescriptionPresentFlag);
        CamOsPrintf("Get VuiCtl.stVuiTimeInfo.u8TimingInfoPresentFlag = %d\n", VuiCtl.stVuiTimeInfo.u8TimingInfoPresentFlag);
        CamOsPrintf("Get VuiCtl.stVuiAspectRatio.u16SarWidth = %d\n", VuiCtl.stVuiAspectRatio.u16SarWidth);
        CamOsPrintf("Get VuiCtl.stVuiAspectRatio.u16SarHeight = %d\n", VuiCtl.stVuiAspectRatio.u16SarHeight);

    }

    //Set LTR
    if(pParam->sLTR.bEn)
    {
        memset(&RefCtl, 0, sizeof(RefCtl));
        setHeader(&RefCtl, sizeof(RefCtl));

        RefCtl.bEnablePred = pParam->sLTR.bEnabledPred;
        RefCtl.u32Enhance = pParam->sLTR.nEnhance;
        RefCtl.u32Base = pParam->sLTR.nBase;

        CamOsPrintf("Set RefCtl.bEnablePred = %d \n", RefCtl.bEnablePred);
        CamOsPrintf("Set RefCtl.u32Enhance = %d \n", RefCtl.u32Enhance);
        CamOsPrintf("Set RefCtl.u32Base = %d \n", RefCtl.u32Base);

        param.type = E_MHAL_VENC_264_REF;
        param.param = (void *)&RefCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_264_REF Error = %x\n",__func__,err);
        }

        memset(&RefCtl, 0, sizeof(RefCtl));
        setHeader(&RefCtl, sizeof(RefCtl));

        param.type = E_MHAL_VENC_264_REF;
        param.param = (void *)&RefCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_264_REF Error = %x\n",__func__,err);
        }

        CamOsPrintf("Get RefCtl.bEnablePred = %d \n", RefCtl.bEnablePred);
        CamOsPrintf("Get RefCtl.u32Enhance = %d \n", RefCtl.u32Enhance);
        CamOsPrintf("Get RefCtl.u32Base = %d \n", RefCtl.u32Base);
    }


    /* MFE Stream On */
    memset(&stMfeIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMfeIntrBuf, sizeof(stMfeIntrBuf));
    stMfeIntrBuf.u32IntrAlBufSize   = 0;
    stMfeIntrBuf.pu8IntrAlVirBuf    = NULL;
    stMfeIntrBuf.phyIntrAlPhyBuf    = NULL;
    stMfeIntrBuf.u32IntrRefBufSize  = 0;
    stMfeIntrBuf.phyIntrRefPhyBuf   = NULL;
    param.type = E_MHAL_VENC_IDX_STREAM_ON;
    param.param = (void*)&stMfeIntrBuf;
    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }


    // Set Multi-Slice
    memset(&SliceCtl, 0, sizeof(SliceCtl));
    setHeader(&SliceCtl, sizeof(SliceCtl));
    SliceCtl.bSplitEnable = pParam->sMSlice.bEn;
    SliceCtl.u32SliceRowCount = pParam->sMSlice.nRows;
    param.type = E_MHAL_VENC_264_I_SPLIT_CTL;
    param.param = (void *)&SliceCtl;
    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_I_SPLIT_CTL Error = %x\n",__func__,err);
    }


    // Set ROI
    for (nRoiCnt=0; nRoiCnt<MAX_ROI_NUM; nRoiCnt++)
    {
        if (pParam->sRoi[nRoiCnt].bEn)
        {
            memset(&RoiCtl, 0, sizeof(RoiCtl));
            setHeader(&RoiCtl, sizeof(RoiCtl));
            RoiCtl.stAttrRoiArea.u32Index = nRoiCnt;
            RoiCtl.stAttrRoiArea.bEnable = pParam->sRoi[nRoiCnt].bEnable;
            RoiCtl.stAttrRoiArea.bAbsQp = pParam->sRoi[nRoiCnt].bAbsQp;
            RoiCtl.stAttrRoiArea.s32Qp = pParam->sRoi[nRoiCnt].nMbqp;
            RoiCtl.stAttrRoiArea.stRect.u32X = pParam->sRoi[nRoiCnt].nMbX*16;
            RoiCtl.stAttrRoiArea.stRect.u32Y = pParam->sRoi[nRoiCnt].nMbY*16;
            RoiCtl.stAttrRoiArea.stRect.u32W = pParam->sRoi[nRoiCnt].nMbW*16;
            RoiCtl.stAttrRoiArea.stRect.u32H = pParam->sRoi[nRoiCnt].nMbH*16;

            CamOsPrintf("ROI%d => Abs:%d, Qp:%d, (%d, %d, %d, %d)\n",
                    RoiCtl.stAttrRoiArea.u32Index,
                    RoiCtl.stAttrRoiArea.bAbsQp,
                    RoiCtl.stAttrRoiArea.s32Qp,
                    RoiCtl.stAttrRoiArea.stRect.u32X,
                    RoiCtl.stAttrRoiArea.stRect.u32Y,
                    RoiCtl.stAttrRoiArea.stRect.u32W,
                    RoiCtl.stAttrRoiArea.stRect.u32H);

            param.type = E_MHAL_VENC_264_ROI;
            param.param = (void *)&RoiCtl;
            err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
            if( err )
            {
                CamOsPrintf("%s Set E_MHAL_VENC_264_ROI Error = %x\n",__func__,err);
            }
        }
    }

    if(pParam->sQpMap.bEn)
    {
        memset(&RoiCtl, 0, sizeof(RoiCtl));
        setHeader(&RoiCtl, sizeof(RoiCtl));
        RoiCtl.eRoiMode = E_MHAL_VENC_ROI_MODE_QPMAP;
        RoiCtl.stAttrQpMap.bDaQpMapEnable = pParam->sQpMap.bEnable;
        CamOsPrintf("QPEntry: ");
        for(i = 0; i < 14; i++)
        {
            RoiCtl.stAttrQpMap.DaQpMapEntry[i] = pParam->sQpMap.nEntry[i];
            CamOsPrintf("(%d)%d ",i,RoiCtl.stAttrQpMap.DaQpMapEntry[i]);

        }
        CamOsPrintf("\n");
        RoiCtl.stAttrQpMap.pDaQpMap = (MS_U8 *)pParam->sQpMap.nVirAddr;

        _drawQpMap(RoiCtl.stAttrQpMap.pDaQpMap);

        CamOsPrintf("UT Draw QPMAP\n");
        CamOsHexdump((char *)RoiCtl.stAttrQpMap.pDaQpMap,(pParam->basic.w/16)*(pParam->basic.h/16));

        param.type = E_MHAL_VENC_264_ROI;
        param.param = (void *)&RoiCtl;
        err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_264_ROI Error = %x\n",__func__,err);
        }
    }

#if 0
    // Set User Data
    memset(&UserData, 0, sizeof(UserData));
    setHeader(&UserData, sizeof(UserData));
    UserData.pu8Data = NULL;
    UserData.u32Len = 0;
    param.type = E_MHAL_VENC_264_USER_DATA;
    param.param = (void *)&UserData;
    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_USER_DATA Error = %x\n",__func__,err);
    }
#endif

    return err;
}

int _GetMfeParameterByMhal(Mfe_param* pParam)
{
    ERROR err = ErrorNone;
    VencParamUT param = {0};
    memset(&param, 0, sizeof(VencParamUT));

    MHAL_VENC_Resoluton_t ResCtl;
    memset(&ResCtl, 0, sizeof(ResCtl));

    CamOsPrintf("======[ %s ]======\n",__func__);


    param.type = E_MHAL_VENC_264_RESOLUTION;
    setHeader(&ResCtl, sizeof(ResCtl));
    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Get E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
    }

    CamOsPrintf("[%s %d] ResCtl.u32Width = %d \n", __func__, __LINE__,ResCtl.u32Width);
    CamOsPrintf("[%s %d] ResCtl.u23Height = %d \n", __func__, __LINE__, ResCtl.u32Height);
    CamOsPrintf("[%s %d] ResCtl.eFmt = %d \n",__func__, __LINE__, ResCtl.eFmt);

    MHAL_VENC_RcInfo_t RcCtl;
    memset(&RcCtl, 0, sizeof(RcCtl));

    param.type = E_MHAL_VENC_264_RC;
    setHeader(&RcCtl, sizeof(RcCtl));

    param.param = (void *)&RcCtl;

    err = ioctl(pParam->mfefd, IOCTL_MMFE_MHAL_GET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_264_RT_RC Error = %x\n",__func__,err);
    }

    CamOsPrintf("[%s %d] RcCtl.eRcMode = %d \n", __func__, __LINE__,RcCtl.eRcMode);

    if(RcCtl.eRcMode == E_MHAL_VENC_RC_MODE_H264FIXQP)
    {
        CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32SrcFrmRateNum = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32SrcFrmRateNum);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32SrcFrmRateDen = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32SrcFrmRateDen);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32Gop = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32Gop);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32IQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32IQp);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264FixQp.u32PQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264FixQp.u32PQp);
    }
    else if(RcCtl.eRcMode == E_MHAL_VENC_RC_MODE_H264CBR)
    {
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32Gop = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32Gop);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32StatTime = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32StatTime);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32SrcFrmRateNum = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32SrcFrmRateNum);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32SrcFrmRateDen = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32SrcFrmRateDen);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32BitRate = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32BitRate);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Cbr.u32FluctuateLevel = %d \n",__func__, __LINE__, RcCtl.stAttrH264Cbr.u32FluctuateLevel);
    }
    else if(RcCtl.eRcMode == E_MHAL_VENC_RC_MODE_H264VBR)
    {
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32Gop = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32Gop);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32StatTime = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32StatTime);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32SrcFrmRateNum = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32SrcFrmRateNum);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32SrcFrmRateDen = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32SrcFrmRateDen);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32MaxBitRate = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32MaxBitRate);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32MaxQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32MaxQp);
        CamOsPrintf("[%s %d] RcCtl.stAttrH264Vbr.u32MinQp = %d \n",__func__, __LINE__, RcCtl.stAttrH264Vbr.u32MinQp);
    }


    return err;
}

void print_hex(char* title, void* buf, int num) {
        int i;
        char *data = (char *) buf;

        CamOsPrintf(
                        "%s\nOffset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
                        "----------------------------------------------------------",
                        title);
        for (i = 0; i < num; i++) {
                if (i % 16 == 0)
                {
                    //CamOsPrintf("\n%08X   ", i);
                    CamOsPrintf("\n");
                }
                CamOsPrintf("%02X ", data[i]);
        }
        CamOsPrintf("\n");
}

int _RunMfe(Mfe_param* pParam)
{
    int ret;
    int mfefd = open("/dev/mmfe", O_RDWR);
    FILE * fi = NULL;
    FILE * fo = NULL;
    int pictw, picth, picts, ysize, csize;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pBsBufVitr = NULL;
    void* pBsBufPhys = NULL;
    void* pBsBufMiu = NULL;
    int length = 0;
    int err = 0;
    int total = 0;
    mmfe_parm avc_param;
    mmfe_buff vbuff[2];
    int frame = 1;
    MHAL_VENC_InOutBuf_t InOutBuf;
    MHAL_VENC_EncResult_t EncRet;
    int pixw, pixh;
    MD5_CTX stMd5 = {0};
    unsigned char u8Md5Result[16] = {0};
    void* pQpMapBufVitr = NULL;
    void* pQpMapBufPhys = NULL;
    void* pQpMapBufMiu = NULL;
    int qpmapsize;


    pParam->mfefd = mfefd;

    if(pParam->mfefd == NULL)
    {
        CamOsPrintf("Get \"/dev/mmfe\" is NULL\n");
        return 0;
    }

    MD5_Init(&stMd5);

    do
    {
        if (mfefd < 0)
            break;
        fi = fopen(pParam->inputPath, "rb");
        if(fi == NULL)
            CamOsPrintf("%s not exit!!\n",pParam->inputPath);
        fo = fopen(pParam->outputPath, "wb");
        if(fo == NULL)
            CamOsPrintf("%s not exit!!\n",pParam->outputPath);
        pictw = pParam->basic.w;
        picth = pParam->basic.h;
        if(picth%16 != 0)
            picth = picth+ picth%16;
        pixw = pParam->basic.w;
        pixh = pParam->basic.h;
        if (!strncmp(pParam->basic.pixfm, "NV12", 4))
        {
            ysize = pictw*picth;
            csize = ysize/2;
        }
        if (!strncmp(pParam->basic.pixfm, "NV21", 4))
        {
            ysize = pictw*picth;
            csize = ysize/2;
        }
        if (!strncmp(pParam->basic.pixfm, "YUYV", 4))
        {
            ysize = pictw*picth*2;
            csize = 0;
        }

        sleep(1);

        do
        {   /* now, check arguments */
            if (fi == NULL || fo == NULL)
            {
                break;
            }

            if ((pictw%16) != 0 || (picth%8) != 0)
                break;

            picts = ysize+csize;
            length = pixw*pixh;

            CamOsDirectMemAlloc("MFE-IBUFF", picts, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);

            if(pYuvBufVitr == NULL)
            {
                CamOsPrintf("!!!!!!!!! pixels is NULL !!!!!!!!!!\n");
            }

            CamOsDirectMemAlloc("MFE-OBUFF", length, &pBsBufVitr, &pBsBufPhys, &pBsBufMiu);

            memset(pYuvBufVitr, 128, picts);
            memset(vbuff, 0, sizeof(vbuff[2]));

            qpmapsize = (pixw/16)*(pixh/16);

            CamOsDirectMemAlloc("QPMAP", qpmapsize, &pQpMapBufVitr, &pQpMapBufPhys, &pQpMapBufMiu);
            CamOsPrintf("QPMAP virtual address: 0x%X\n",pQpMapBufVitr);

            if(pParam->sQpMap.bEn)
            {
                pParam->sQpMap.nVirAddr = (u32)pQpMapBufVitr;
                memset((void *)pParam->sQpMap.nVirAddr, 0xFF, qpmapsize);
            }


            _SetMfeParameterByMhal(pParam);


            CamOsPrintf("---------------------\n");
            //sleep(1);

            _GetMfeParameterByMhal(pParam);

            sleep(1);

            while (length == fread(pYuvBufVitr, 1, length, fi))
            {
                fread(pYuvBufVitr+ysize, 1, length/2, fi);

                InOutBuf.pu32RegBase0 = NULL;
                InOutBuf.pu32RegBase1 = NULL;
                //InOutBuf.pCmdQ = &InOutBuf;     // If not null, fire by CmdQ
                InOutBuf.pCmdQ = NULL;
                InOutBuf.bRequestI = 0;
                InOutBuf.phyInputYUVBuf1 = (MS_PHYADDR)(intptr_t)pYuvBufMiu;
                InOutBuf.u32InputYUVBuf1Size = ysize;
                InOutBuf.phyInputYUVBuf2 = InOutBuf.phyInputYUVBuf1 + InOutBuf.u32InputYUVBuf1Size;
                InOutBuf.u32InputYUVBuf2Size = csize;
                InOutBuf.phyOutputBuf = (MS_PHYADDR)(intptr_t)pBsBufMiu;
                InOutBuf.u32OutputBufSize = length;
                InOutBuf.pFlushCacheCb = NULL;

                ret = ioctl(mfefd, IOCTL_MMFE_MHAL_ENCODE_ONE_FRAME, &InOutBuf);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ENCODE_NONBLOCKING failed (%d) !!\n", ret);
                }

                ret = ioctl(mfefd, IOCTL_MMFE_MHAL_KICKOFF_CMDQ, NULL);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_MHAL_KICKOFF_CMDQ failed (%d) !!\n", ret);
                }

                usleep(20000);

                ret = ioctl(mfefd, IOCTL_MMFE_MHAL_ENCODE_FRAME_DONE, &EncRet);
                if (ret)
                {
                    CamOsPrintf("!! IOCTL_MMFE_ACQUIRE_NONBLOCKINGA failed (%d) !!\n", ret);
                }

                fwrite(pBsBufVitr, 1, EncRet.u32OutputBufUsed, fo);

                //print_hex("Bitstream", pBsBufVitr, EncRet.u32OutputBufUsed);

                total += EncRet.u32OutputBufUsed;
                CamOsPrintf("frame:%d, size:%d \n", frame, EncRet.u32OutputBufUsed);
                MD5_Update(&stMd5, pBsBufVitr, EncRet.u32OutputBufUsed);
                frame++;

                if(frame > pParam->FramesToBeEncoded)
                    break;

            }

            avc_param.type = E_MHAL_VENC_IDX_STREAM_OFF;
            err = ioctl(mfefd, IOCTL_MMFE_MHAL_SET_PARAM, &avc_param);
            if( err )
            {
                CamOsPrintf("%s Set E_MHAL_VENC_264_Resolution Error = %x\n",__func__,err);
            }

            CamOsPrintf("total-size:%8d\n",total);

            MD5_Final(u8Md5Result, &stMd5);
            CamOsPrintf("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                        u8Md5Result[0], u8Md5Result[1], u8Md5Result[2], u8Md5Result[3],
                        u8Md5Result[4], u8Md5Result[5], u8Md5Result[6], u8Md5Result[7],
                        u8Md5Result[8], u8Md5Result[9], u8Md5Result[10], u8Md5Result[11],
                        u8Md5Result[12], u8Md5Result[13], u8Md5Result[14], u8Md5Result[15]);
        }
        while (0);

        if (fi) fclose(fi);
        if (fo) fclose(fo);

        CamOsDirectMemRelease(pYuvBufVitr, picts);
        CamOsDirectMemRelease(pBsBufVitr, length);
        CamOsDirectMemRelease(pQpMapBufVitr, qpmapsize);
    }
    while (0);

    if (mfefd > 0)  close(mfefd);

    return 0;

}

int main(int argc, char** argv) {
    int cmd;
    Mfe_param MfeParam1 = {0};

    ReadDefaultConfig("avc.cfg", &MfeParam1);

    DisplayMenuSetting();
    do{
        CamOsPrintf("==>");
        cmd = getchar();
        switch(cmd){
            case 'f':
                SetInOutPath(&MfeParam1);
                DisplayMenuSetting();
                break;
            case 's':
                SetMFEParameter(&MfeParam1);
                DisplayMenuSetting();
                break;
            case 'e':
                _RunMfe(&MfeParam1);
                DisplayMenuSetting();
                break;
            case 'r':
                ReadDefaultConfig("avc.cfg", &MfeParam1);
                break;
            case 'p':
                ShowCurrentMFESetting(&MfeParam1);
                break;
            case 'h':
                DisplayMenuSetting();
                break;
            default:
                break;
        }
    }while('q' != cmd );

    return 0;
}
