
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <mdrv_msys_io.h>
#include <mdrv_verchk.h>
#include <mdrv_msys_io_st.h>

#include <mdrv_rqct_io.h>
#include <mdrv_mhe_io.h>
#include <mdrv_pmbr_io.h>

#include "cam_os_wrapper.h"

#include "mhe_utility.h"
#include "mhe_control.h"
#include "mhal_ut_wrapper.h"
#include "md5.h"

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define CLP(a,b,x)  (MIN(MAX(x,a),b))

#define CBR_UPPER_QP        51
#define CBR_LOWER_QP        12

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

typedef MSYS_DMEM_INFO msys_dmem;
typedef MSYS_ADDR_TRANSLATION_INFO addr_info;


#ifdef MHE_MSRC_SIMULATION
void hw_enc_frame(ms_rc_top* ptMsRc, , rqct_ops* rqct, int pic_idx)
{
#define MBR_LUT_SIZE 15

    ms_rc_pic *ptRcPic = &ptMsRc->tRcPic;

    FILE *g_pfRcInfo;
    FILE *g_pfRcStats;
    FILE *g_pfLutInfo;

    // golden variable
    int g_aiHWLUTTarget[MBR_LUT_SIZE];
    int g_aiHWLUTQp[MBR_LUT_SIZE];
    int g_aiHWLUTLambda[MBR_LUT_SIZE];

    int g_iPicAllocBit;
    int g_iPicEstQp;
    int g_iPicEstLambda;

    double dummy;
    char Filename[64];
    //char msg[64];
    int idx, entry, val;

    g_pfRcInfo = fopen("./msrc/rc_info.txt", "r");

    // get frame level info
    fscanf(g_pfRcInfo, "%d %d %d %d %f\n", &g_iHWActPicEncBits, &g_iPicAllocBit, &g_iPicEstQp, &g_iPicEstLambda, &dummy);

    // get mbr info
    sprintf(Filename, "./msrc/mbr_top_%03d.txt", pic_idx);
    g_pfRcStats = fopen(Filename, "r");

    fscanf(g_pfRcStats, "tc_accum = %x\n", &g_iHWTextCplxAccum);
    fscanf(g_pfRcStats, "pc_pixel_count_accum = %x\n", &g_iHWPCCntAccum);
    fscanf(g_pfRcStats, "bit_weighting_out_accum = %x\n", &g_iHWBWOuwAccum);
    fscanf(g_pfRcStats, "ctb_bit_weighting_accum = %x\n", &g_iHWCtbBWAccum);

    for(idx = 0; idx < 15; idx++)
    {
        fscanf(g_pfRcStats, "HIST[%d] = %x\n", &entry, &val);
        g_aiHWLutIdcHist[entry] = val;
    }
    for(idx = 0; idx < 32; idx++)
    {
        fscanf(g_pfRcStats, "HIST_PC[%d] = %x\n", &entry, &val);
        g_aiPCCntHist[entry] = val;
    }
    for(idx = 0; idx < 32; idx++)
    {
        fscanf(g_pfRcStats, "HIST_TW[%d] = %x\n", &entry, &val);
        g_aiHWTextCplxHist[entry] = val;
    }
    fclose(g_pfRcStats);
    // -----------------------------------------------------------
    // get lut info
    sprintf(Filename, "./msrc/mbr_miu_readLUT%03d.txt", pic_idx);
    g_pfLutInfo = fopen(Filename, "r");
    for(idx = 0; idx < 15; idx++)
    {
        int reg0, reg1, reg2;
        fscanf(g_pfLutInfo, "%08x%08x%08x", &reg0, &reg1, &reg2);

        g_aiHWLUTTarget[idx] = reg2 & 0x1FFF;
        g_aiHWLUTLambda[idx] = reg1 >> 5;
        g_aiHWLUTQp[idx] = reg0 >> 21;
    }

    fclose(g_pfLutInfo);

    //------------------------------------------------------------
    ptRcPic->iTargetBit = g_iHWActPicEncBits;
    for(idx = 0; idx < MBR_LUT_SIZE; idx++)
    {
        rqct->auiLambdas_SCALED[idx] = g_aiHWLUTLambda[idx];
    }
    for(idx = 0; idx < MBR_LUT_SIZE; idx++)
    {
        rqct->auiBits[idx] = g_aiHWLUTTarget[idx];
    }
    for(idx = 0; idx < MBR_LUT_SIZE; idx++)
    {
        rqct->auiQps[idx] = g_aiHWLUTQp[idx];
    }
    for(idx = 0; idx < MBR_LUT_SIZE; idx++)
    {
        rqct->aiIdcHist[idx] = g_aiHWLutIdcHist[idx];
    }
}
#endif

void setHeader(VOID* header, MS_U32 size) {
    MHAL_VENC_Version_t* ver = (MHAL_VENC_Version_t*)header;
    ver->u32Size = size;

    ver->s.u8VersionMajor = SPECVERSIONMAJOR;
    ver->s.u8VersionMinor = SPECVERSIONMINOR;
    ver->s.u8Revision = SPECREVISION;
    ver->s.u8Step = SPECSTEP;
}

int _drawQpMap(MS_U8 * addr)
{

    memset(addr, 0, 200);

    memset(addr+500, 3, 200);

    memset(addr+1000, 5, 200);

    memset(addr+1500, 13, 200);

    return 0;
}

#if 1
int _SetMheParameterByMhal(Mheparam* pParam)
{
    int err = 0;
    VencParamUT param = {0};
    MHAL_VENC_Resoluton_t ResCtl;
    MHAL_VENC_RcInfo_t RcCtl;
    MHAL_VENC_InternalBuf_t stMheIntrBuf;
    MHAL_VENC_ParamSplit_t SliceCtl;
    MHAL_VENC_RoiCfg_t RoiCtl;
    MHAL_VENC_ParamH265Dblk_t DblkCtl;
    MHAL_VENC_ParamH265Trans_t TransCtl;
    MHAL_VENC_ParamH265InterPred_t InterPredCtl;
    MHAL_VENC_ParamH265IntraPred_t IntraPredCtl;
    MHAL_VENC_ParamH265Vui_t VuiCtl;
    MHAL_VENC_ParamRef_t RefCtl;
    u32 nRoiCnt;
    u32 i;

    /* Set MHE parameter by iocal */
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
    param.type = E_MHAL_VENC_265_RESOLUTION;
    param.param = (void *)&ResCtl;

    err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_265_Resolution Error = %x\n",__func__,err);
    }


    memset(&RcCtl, 0, sizeof(RcCtl));
    setHeader(&RcCtl, sizeof(RcCtl));
    switch(pParam->sRC.eControlRate)
    {
    case Mhe_RQCT_METHOD_CBR:
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H265CBR;
        RcCtl.stAttrH265Cbr.u32Gop              = pParam->basic.Gop;
        RcCtl.stAttrH265Cbr.u32StatTime         = 1;    // TBD
        RcCtl.stAttrH265Cbr.u32SrcFrmRateNum    = pParam->basic.fps;
        RcCtl.stAttrH265Cbr.u32SrcFrmRateDen    = 1;
        RcCtl.stAttrH265Cbr.u32BitRate          = pParam->basic.bitrate;
        RcCtl.stAttrH265Cbr.u32FluctuateLevel   = 0;    // TBD
        break;
    case Mhe_RQCT_METHOD_VBR:
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H265VBR;
        RcCtl.stAttrH265Vbr.u32Gop              = pParam->basic.Gop;
        RcCtl.stAttrH265Vbr.u32StatTime         = 1;    // TBD
        RcCtl.stAttrH265Vbr.u32SrcFrmRateNum    = pParam->basic.fps;
        RcCtl.stAttrH265Vbr.u32SrcFrmRateDen    = 1;
        RcCtl.stAttrH265Vbr.u32MaxBitRate       = pParam->basic.bitrate;
        RcCtl.stAttrH265Vbr.u32MaxQp            = pParam->sRC.nImaxqp;
        RcCtl.stAttrH265Vbr.u32MinQp            = pParam->sRC.nIminqp;
        break;
    case Mhe_RQCT_METHOD_CQP:
    default :
        RcCtl.eRcMode                           = E_MHAL_VENC_RC_MODE_H265FIXQP;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateNum  = pParam->basic.fps;
        RcCtl.stAttrH265FixQp.u32SrcFrmRateDen  = 1;
        RcCtl.stAttrH265FixQp.u32Gop            = pParam->basic.Gop;
        RcCtl.stAttrH265FixQp.u32IQp            = pParam->basic.nQp;
        RcCtl.stAttrH265FixQp.u32PQp            = pParam->basic.nQp;
        break;
    }
    param.type = E_MHAL_VENC_265_RC;
    param.param = (void *)&RcCtl;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_265_RC Error = %x\n",__func__,err);
    }

    // Set Deblocking
    if (pParam->sDeblk.bEn)
    {
        memset(&DblkCtl, 0, sizeof(DblkCtl));
        setHeader(&DblkCtl, sizeof(DblkCtl));
        DblkCtl.disable_deblocking_filter_idc = 2;//pParam->sDeblk.b_deblocking_disable;
        DblkCtl.slice_tc_offset_div2 = pParam->sDeblk.slice_tc_offset_div2;
        DblkCtl.slice_beta_offset_div2 = pParam->sDeblk.slice_beta_offset_div2;
        param.type = E_MHAL_VENC_265_DBLK;
        param.param = (void *)&DblkCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_DBLK Error = %x\n",__func__,err);
        }

        memset(&DblkCtl, 0, sizeof(DblkCtl));
        setHeader(&DblkCtl, sizeof(DblkCtl));
        param.type = E_MHAL_VENC_265_DBLK;
        param.param = (void *)&DblkCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_DBLK Error = %x\n",__func__,err);
        }
    }


    // Set Trans Mode
    memset(&TransCtl, 0, sizeof(TransCtl));
    setHeader(&TransCtl, sizeof(TransCtl));
    TransCtl.u32IntraTransMode = 0;
    TransCtl.u32InterTransMode = 0;
    TransCtl.s32ChromaQpIndexOffset = 0;
    param.type = E_MHAL_VENC_265_TRANS;
    param.param = (void *)&TransCtl;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_265_TRANS Error = %x\n",__func__,err);
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

        CamOsPrintf("Set InterPredCtl.u32HWSize = %d\n", InterPredCtl.u32HWSize);
        CamOsPrintf("Set InterPredCtl.u32VWSize = %d\n", InterPredCtl.u32VWSize);
        CamOsPrintf("Set InterPredCtl.bInter16x16PredEn = %d\n", InterPredCtl.bInter16x16PredEn);
        CamOsPrintf("Set InterPredCtl.bInter16x8PredEn = %d\n", InterPredCtl.bInter16x8PredEn);
        CamOsPrintf("Set InterPredCtl.bInter8x16PredEn = %d\n", InterPredCtl.bInter8x16PredEn);
        CamOsPrintf("Set InterPredCtl.bInter8x8PredEn = %d\n", InterPredCtl.bInter8x8PredEn);

        param.type = E_MHAL_VENC_265_INTER_PRED;
        param.param = (void *)&InterPredCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_INTER_PRED Error = %x\n",__func__,err);
        }

        memset(&InterPredCtl, 0, sizeof(InterPredCtl));
        setHeader(&InterPredCtl, sizeof(InterPredCtl));

        param.type = E_MHAL_VENC_265_INTER_PRED;
        param.param = (void *)&InterPredCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_INTER_PRED Error = %x\n",__func__,err);
        }

        CamOsPrintf("Get InterPredCtl.u32HWSize = %d\n", InterPredCtl.u32HWSize);
        CamOsPrintf("Get InterPredCtl.u32VWSize = %d\n", InterPredCtl.u32VWSize);
        CamOsPrintf("Get InterPredCtl.bInter16x16PredEn = %d\n", InterPredCtl.bInter16x16PredEn);
        CamOsPrintf("Get InterPredCtl.bInter16x8PredEn = %d\n", InterPredCtl.bInter16x8PredEn);
        CamOsPrintf("Get InterPredCtl.bInter8x16PredEn = %d\n", InterPredCtl.bInter8x16PredEn);
        CamOsPrintf("Get InterPredCtl.bInter8x8PredEn = %d\n", InterPredCtl.bInter8x8PredEn);
    }


    // Set Intra Pred
    if (pParam->sIntraP.bEn)
    {
        memset(&IntraPredCtl, 0, sizeof(IntraPredCtl));
        setHeader(&IntraPredCtl, sizeof(IntraPredCtl));
//        IntraPredCtl.bIntra8x8PredEn = 1;
//        IntraPredCtl.bIntra16x16PredEn = 1;
//        IntraPredCtl.bIntra32x32PredEn = 1;
        IntraPredCtl.constrained_intra_pred_flag = pParam->sIntraP.bconstIpred;
//        IntraPredCtl.bIpcmEn = 0;
        IntraPredCtl.u32Intra8x8Penalty = pParam->sIntraP.nIntraCu8Pen;
        IntraPredCtl.u32Intra16x16Penalty = pParam->sIntraP.nIntraCu16Pen;
        IntraPredCtl.u32Intra32x32Penalty = pParam->sIntraP.nIntra32Pen;

//        CamOsPrintf("Set IntraPredCtl.bIntra8x8PredEn = %d\n", IntraPredCtl.bIntra8x8PredEn);
//        CamOsPrintf("Set IntraPredCtl.bIntra16x16PredEn = %d\n", IntraPredCtl.bIntra16x16PredEn);
//        CamOsPrintf("Set IntraPredCtl.bIntra32x32PredEn = %d\n", IntraPredCtl.bIntra32x32PredEn);
        CamOsPrintf("Set IntraPredCtl.u32Intra8x8Penalty = %d\n", IntraPredCtl.u32Intra8x8Penalty);
        CamOsPrintf("Set IntraPredCtl.u32Intra16x16Penalty = %d\n", IntraPredCtl.u32Intra16x16Penalty);
        CamOsPrintf("Set IntraPredCtl.u32Intra32x32Penalty = %d\n", IntraPredCtl.u32Intra32x32Penalty);

        param.type = E_MHAL_VENC_265_INTRA_PRED;
        param.param = (void *)&IntraPredCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_INTRA_PRED Error = %x\n",__func__,err);
        }

        memset(&IntraPredCtl, 0, sizeof(IntraPredCtl));
        setHeader(&IntraPredCtl, sizeof(IntraPredCtl));

        param.type = E_MHAL_VENC_265_INTRA_PRED;
        param.param = (void *)&IntraPredCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_265_INTRA_PRED Error = %x\n",__func__,err);
        }
//        CamOsPrintf("Get IntraPredCtl.bIntra8x8PredEn = %d\n", IntraPredCtl.bIntra8x8PredEn);
//        CamOsPrintf("Get IntraPredCtl.bIntra16x16PredEn = %d\n", IntraPredCtl.bIntra16x16PredEn);
//        CamOsPrintf("Get IntraPredCtl.bIntra32x32PredEn = %d\n", IntraPredCtl.bIntra32x32PredEn);
        CamOsPrintf("Get IntraPredCtl.u32Intra8x8Penalty = %d\n", IntraPredCtl.u32Intra8x8Penalty);
        CamOsPrintf("Get IntraPredCtl.u32Intra16x16Penalty = %d\n", IntraPredCtl.u32Intra16x16Penalty);
        CamOsPrintf("Get IntraPredCtl.u32Intra32x32Penalty = %d\n", IntraPredCtl.u32Intra32x32Penalty);
    }

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

        param.type = E_MHAL_VENC_265_VUI;
        param.param = (void *)&VuiCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_VUI Error = %x\n",__func__,err);
        }

        memset(&VuiCtl, 0, sizeof(VuiCtl));
        setHeader(&VuiCtl, sizeof(VuiCtl));

        param.type = E_MHAL_VENC_265_VUI;
        param.param = (void *)&VuiCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_265_VUI Error = %x\n",__func__,err);
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

        param.type = E_MHAL_VENC_265_REF;
        param.param = (void *)&RefCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_265_REF Error = %x\n",__func__,err);
        }

        memset(&RefCtl, 0, sizeof(RefCtl));
        setHeader(&RefCtl, sizeof(RefCtl));

        param.type = E_MHAL_VENC_265_REF;
        param.param = (void *)&RefCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Get E_MHAL_VENC_265_REF Error = %x\n",__func__,err);
        }

        CamOsPrintf("Get RefCtl.bEnablePred = %d \n", RefCtl.bEnablePred);
        CamOsPrintf("Get RefCtl.u32Enhance = %d \n", RefCtl.u32Enhance);
        CamOsPrintf("Get RefCtl.u32Base = %d \n", RefCtl.u32Base);
    }


    // MHE Stream On
    memset(&stMheIntrBuf, 0, sizeof(MHAL_VENC_InternalBuf_t));
    setHeader(&stMheIntrBuf, sizeof(stMheIntrBuf));
    stMheIntrBuf.u32IntrAlBufSize   = 0;
    stMheIntrBuf.pu8IntrAlVirBuf    = NULL;
    stMheIntrBuf.phyIntrAlPhyBuf    = NULL;
    stMheIntrBuf.u32IntrRefBufSize  = 0;
    stMheIntrBuf.phyIntrRefPhyBuf   = NULL;
    param.type = E_MHAL_VENC_IDX_STREAM_ON;
    param.param = (void*)&stMheIntrBuf;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
    if( err )
    {
        CamOsPrintf("%s Set E_MHAL_VENC_IDX_STREAM_ON Error = %x\n",__func__,err);
    }


    if(pParam->sMSlice.bEn)
    {
        // Set Multi-Slice
        memset(&SliceCtl, 0, sizeof(SliceCtl));
        setHeader(&SliceCtl, sizeof(SliceCtl));
        SliceCtl.bSplitEnable = pParam->sMSlice.bEn;
        SliceCtl.u32SliceRowCount = pParam->sMSlice.nRows;
        param.type = E_MHAL_VENC_265_I_SPLIT_CTL;
        param.param = (void *)&SliceCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_I_SPLIT_CTL Error = %x\n",__func__,err);
        }

        memset(&SliceCtl, 0, sizeof(SliceCtl));
        setHeader(&SliceCtl, sizeof(SliceCtl));
        param.type = E_MHAL_VENC_265_I_SPLIT_CTL;
        param.param = (void *)&SliceCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_GET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265_I_SPLIT_CTL Error = %x\n",__func__,err);
        }

        CamOsPrintf("SliceCtl.bSplitEnable = %d\n", SliceCtl.bSplitEnable);
        CamOsPrintf("SliceCtl.u32SliceRowCount = %d\n", SliceCtl.u32SliceRowCount);
    }

    // Set ROI
    for (nRoiCnt=0; nRoiCnt<MAX_ROI_NUM; nRoiCnt++)
    {
        if (pParam->sRoi[nRoiCnt].bEn)
        {
            memset(&RoiCtl, 0, sizeof(RoiCtl));
            setHeader(&RoiCtl, sizeof(RoiCtl));
            RoiCtl.eRoiMode = E_MHAL_VENC_ROI_MODE_ROIAREA;
            RoiCtl.stAttrRoiArea.u32Index = nRoiCnt;
            RoiCtl.stAttrRoiArea.bEnable = pParam->sRoi[nRoiCnt].bEnable;
            RoiCtl.stAttrRoiArea.bAbsQp = pParam->sRoi[nRoiCnt].bAbsQp;
            RoiCtl.stAttrRoiArea.s32Qp = pParam->sRoi[nRoiCnt].nMbqp;
            RoiCtl.stAttrRoiArea.stRect.u32X = pParam->sRoi[nRoiCnt].nMbX*32;
            RoiCtl.stAttrRoiArea.stRect.u32Y = pParam->sRoi[nRoiCnt].nMbY*32;
            RoiCtl.stAttrRoiArea.stRect.u32W = pParam->sRoi[nRoiCnt].nMbW*32;
            RoiCtl.stAttrRoiArea.stRect.u32H = pParam->sRoi[nRoiCnt].nMbH*32;

            CamOsPrintf("ROI%d => Abs:%d, Qp:%d, (%d, %d, %d, %d)\n",
                    RoiCtl.stAttrRoiArea.u32Index,
                    RoiCtl.stAttrRoiArea.bAbsQp,
                    RoiCtl.stAttrRoiArea.s32Qp,
                    RoiCtl.stAttrRoiArea.stRect.u32X,
                    RoiCtl.stAttrRoiArea.stRect.u32Y,
                    RoiCtl.stAttrRoiArea.stRect.u32W,
                    RoiCtl.stAttrRoiArea.stRect.u32H);

            param.type = E_MHAL_VENC_265_ROI;
            param.param = (void *)&RoiCtl;
            err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
            if( err )
            {
                CamOsPrintf("%s Set E_MHAL_VENC_265ROI Error = %x\n",__func__,err);
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
        CamOsHexdump((char *)RoiCtl.stAttrQpMap.pDaQpMap,(pParam->basic.w/32)*(pParam->basic.h/32));

        param.type = E_MHAL_VENC_265_ROI;
        param.param = (void *)&RoiCtl;
        err = ioctl(pParam->mhefd, IOCTL_MMHE_MHAL_SET_PARAM, &param);
        if( err )
        {
            CamOsPrintf("%s Set E_MHAL_VENC_265ROI Error = %x\n",__func__,err);
        }
    }

    return err;
}
#endif

//-------------------------------------------------

int _RunMhe(Mheparam* pParam)
{
    int ret;
    FILE * fi = NULL;
    FILE * fo = NULL;
    int pictw, picth, picts, ysize, csize, pixfm = -1;
    //int bps, sqp;
    bool disposable = 0;
    unsigned long long base_pix = 0;
    unsigned long long obase_pix = 0;
    unsigned char* pixels = NULL;
    unsigned char* buffer = NULL;
    int length = 0;
    //int err = 0;
    //int dmvx = 32;
    //int dmvy = 16;
    int total = 0;//, cabac = 0, spl = 0;
    msys_dmem sdmem = {0};
    msys_dmem obmem = {0};
    //addr_info saddr={0};
    //mmhe_parm param;
    //mmhe_ctrl cntrl;
    mmhe_buff* ibuff;
    mmhe_buff* obuff;
    mmhe_buff vbuff[2];
    //rqct_conf rqcnf;
    int pixw, pixh;
    int frame = 1;

    void* pQpMapBufVitr = NULL;
    void* pQpMapBufPhys = NULL;
    void* pQpMapBufMiu = NULL;
    int qpmapsize;

    MD5_CTX stMd5 = {0};
    unsigned char u8Md5Result[16] = {0};

    int sysfd = open("/dev/msys", O_RDWR | O_SYNC);
    int memfd = open("/dev/mem" , O_RDWR | O_SYNC);

    pParam->mhefd = open("/dev/mmhe", O_RDWR);

    ibuff = &vbuff[0];
    obuff = &vbuff[1];

    MD5_Init(&stMd5);

    do
    {
        if(pParam->mhefd < 0 || sysfd < 0 || memfd < 0)
            break;

        fi = fopen(pParam->inputPath, "rb");
        fo = fopen(pParam->outputPath, "wb");
        pictw = pParam->basic.w;
        picth = pParam->basic.h;
        if(picth % 16 != 0)
            picth = picth + picth % 16;
        pixw = pParam->basic.w;
        pixh = pParam->basic.h;
        if(!strncmp(pParam->basic.pixfm, "NV12", 4))
            pixfm = MMHE_PIXFMT_NV12;
        if(!strncmp(pParam->basic.pixfm, "NV21", 4))
            pixfm = MMHE_PIXFMT_NV21;
        if(!strncmp(pParam->basic.pixfm, "YUYV", 4))
            pixfm = MMHE_PIXFMT_YUYV;
        //bps = pParam->basic.bitrate;
        //sqp = pParam->sRC.nRefQp;
        //spl = pParam->sMSlice.nRows;
        disposable = pParam->sDisposable.bDisposable;
        //cabac = pParam->sEntropy.bEntropyCodingCABAC;

        //SetMheParameter(pParam);
        qpmapsize = (pixw/32)*(pixh/32);

        CamOsDirectMemAlloc("QPMAP", qpmapsize, &pQpMapBufVitr, &pQpMapBufPhys, &pQpMapBufMiu);
        CamOsPrintf("QPMAP virtual address: 0x%X\n",pQpMapBufVitr);

        if(pParam->sQpMap.bEn)
        {
            pParam->sQpMap.nVirAddr = (u32)pQpMapBufVitr;
            memset((void *)pParam->sQpMap.nVirAddr, 0xFF, qpmapsize);
        }

        _SetMheParameterByMhal(pParam);

        sleep(1);

        do
        {
            /* now, check arguments */
            if(fi == NULL || fo == NULL)
            {
                CamOsPrintf("fi or fo is NULL!!!\n");
                break;
            }

            if((pictw % 16) != 0 || (picth % 16) != 0)
            {
                CamOsPrintf("Width or Hight not align!!!\n");
                break;
            }

            if(pixfm < 0)
            {
                CamOsPrintf("format no setting!!!\n");
                break;
            }

            /* validate params */
            //sqp = CLP(12,48,sqp);
            switch(pixfm)
            {
                case MMHE_PIXFMT_NV12:
                case MMHE_PIXFMT_NV21:
                    ysize = pictw * picth;
                    csize = ysize / 2;
                    break;
                case MMHE_PIXFMT_YUYV:
                    ysize = pictw * picth * 2;
                    csize = 0;
                    break;
            }

            picts = ysize + csize;
            length = pixw * pixh;
            /* request direct-memory */
            memset(&sdmem, 0, sizeof(msys_dmem));
            snprintf(sdmem.name, 15, "%d:MHE-IBUFF", pParam->nInstanceId);
            FILL_VERCHK_TYPE(sdmem, sdmem.VerChk_Version, sdmem.VerChk_Size, IOCTL_MSYS_VERSION);
            sdmem.option = 0;
            sdmem.kvirt = 0;
            sdmem.phys = 0;
            sdmem.length = picts;
            if(ioctl(sysfd, IOCTL_MSYS_REQUEST_DMEM, &sdmem))
            {
                sdmem.length = 0;
                printf("Request DMEM %s Failed !!!\n", sdmem.name);
                break;
            }
            pixels = mmap(NULL, sdmem.length, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, sdmem.phys);

            if(pixels == NULL)
            {
                printf("!!!!!!!!! pixels is NULL !!!!!!!!!!\n");
            }

            //saddr.addr = sdmem.phys;
            //FILL_VERCHK_TYPE(saddr, saddr.VerChk_Version, saddr.VerChk_Size, IOCTL_MSYS_VERSION);
            //ioctl(sysfd, IOCTL_MSYS_PHYS_TO_MIU, &saddr);
            //base_pix = saddr.addr;
            base_pix = sdmem.phys;

            memset(&obmem, 0, sizeof(msys_dmem));
            snprintf(obmem.name, 15, "%d:MHE-OBUFF", pParam->nInstanceId);
            FILL_VERCHK_TYPE(obmem, obmem.VerChk_Version, obmem.VerChk_Size, IOCTL_MSYS_VERSION);
            obmem.option = 0;
            obmem.kvirt = 0;
            obmem.phys = 0;
            obmem.length = length;
            if(ioctl(sysfd, IOCTL_MSYS_REQUEST_DMEM, &obmem))
            {
                obmem.length = 0;
                printf("Request DMEM %s Failed !!!\n", sdmem.name);
                break;
            }
            buffer = mmap(NULL, obmem.length, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, obmem.phys);
            obase_pix = obmem.phys;

            memset(pixels, 128, picts);
            memset(&vbuff, 0, sizeof(vbuff));
            ibuff->i_timecode = 0;
            ibuff->i_width = pictw;
            ibuff->i_height = picth;
            ibuff->i_memory = MMHE_MEMORY_MMAP;
            switch(pixfm)
            {
                case MMHE_PIXFMT_NV12:
                case MMHE_PIXFMT_NV21:
                    ibuff->i_planes = 2;
                    ibuff->planes[0].mem.phys = base_pix;
                    ibuff->planes[1].mem.phys = base_pix + ysize;
                    ibuff->planes[0].i_size = ysize;
                    ibuff->planes[1].i_size = csize;
                    break;
                case MMHE_PIXFMT_YUYV:
                    ibuff->i_planes = 1;
                    ibuff->planes[0].mem.phys = base_pix;
                    ibuff->planes[1].mem.phys = 0;
                    ibuff->planes[0].i_size = ysize;
                    ibuff->planes[1].i_size = 0;
                    break;
                default:
                    break;
            }
            obuff->i_memory = MMHE_MEMORY_MMAP;
            obuff->i_planes = 1;
            obuff->planes[0].mem.phys = obase_pix;
            obuff->planes[0].i_size = length;
            obuff->planes[0].i_used = 0;
            obuff->i_flags = 0;

            /* set parameters */
            /*            param.type = MMHE_PARM_RES;
                        param.res.i_pict_w = pictw;
                        param.res.i_pict_h = picth;
                        param.res.i_pixfmt = pixfm;
                        param.res.i_outlen = -1;
                        param.res.i_flags = 0;
                        if (ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
                            break;*/
#if 0
            param.type = MMHE_PARM_FPS;
            param.fps.i_num = 25;
            param.fps.i_den = 1;
            if(ioctl(mhefd, IOCTL_MMHE_S_PARM, &param))
                break;
            param.type = MMHE_PARM_MOT;
            if(ioctl(mhefd, IOCTL_MMHE_G_PARM, &param))
                break;
            param.mot.i_dmv_x = dmvx;
            param.mot.i_dmv_y = dmvy;
            if(ioctl(mhefd, IOCTL_MMHE_S_PARM, &param))
                break;
            param.type = MMHE_PARM_AVC;
            if(ioctl(mhefd, IOCTL_MMHE_G_PARM, &param))
                break;

            if(disposable > 0)
                param.avc.i_poc_type = 0;
            param.avc.b_cabac = 0;
            param.avc.i_profile = MMHE_AVC_PROFILE_BP;
            if(cabac)
            {
                param.avc.b_cabac = 1;
                param.avc.i_profile = MMHE_AVC_PROFILE_MP;
            }
            if(ioctl(mhefd, IOCTL_MMHE_S_PARM, &param))
                break;
            /* streaming loop */
            if(ioctl(mhefd, IOCTL_MMHE_STREAMON))
                break;
            rqcnf.type = RQCT_CONF_SEQ;
            rqcnf.seq.i_method = !bps ? RQCT_METHOD_CQP : RQCT_METHOD_CBR;
            rqcnf.seq.i_period = 25;
            rqcnf.seq.i_leadqp = sqp;
            rqcnf.seq.i_btrate = bps;
            if(ioctl(mhefd, IOCTL_RQCT_S_CONF, &rqcnf))
                break;

            cntrl.type = MMHE_CTRL_SPL;
            cntrl.spl.i_rows = spl;
            cntrl.spl.i_bits = 0;
            if(ioctl(mhefd, IOCTL_MMHE_S_CTRL, &cntrl))
                break;
#else
            /* streaming loop */
            //if(ioctl(pParam->mhefd, IOCTL_MMHE_STREAMON))
            //    break;

            //SetMheControl(pParam);
#endif

            while(length == fread(pixels, 1, length, fi))
            {

                fread(pixels + ysize, 1, length / 2, fi);
                ibuff->i_flags = disposable > 0 ? MMHE_FLAGS_DISPOSABLE : 0;
                ibuff->planes[0].i_used = ibuff->planes[0].i_size;
                ibuff->planes[1].i_used = ibuff->planes[1].i_size;
                obuff->planes[0].i_used = 0;

                if((ret = ioctl(pParam->mhefd, IOCTL_MMHE_ENCODE, vbuff)) < 0)
                {
                    printf("!! IOCTL_MMHE_ENCODE failed (%d) !!\n", ret);
                }

                if(disposable < 2 || !(obuff->i_flags & MMHE_FLAGS_DISPOSABLE))
                    fwrite(buffer, 1, obuff->planes[0].i_used, fo);

                total += obuff->planes[0].i_used;
                printf("frame:%d, size:%d \n", frame, obuff->planes[0].i_used);
                MD5_Update(&stMd5, buffer, obuff->planes[0].i_used);
                frame++;

                if(frame > pParam->FramesToBeEncoded)
                    break;
            }
            ioctl(pParam->mhefd, IOCTL_MMHE_STREAMOFF);
            printf("total-size:%8d\n", total);
        }
        while(0);

        CamOsPrintf("Exit encode flow!!!\n");

        MD5_Final(u8Md5Result, &stMd5);
        CamOsPrintf("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    u8Md5Result[0], u8Md5Result[1], u8Md5Result[2], u8Md5Result[3],
                    u8Md5Result[4], u8Md5Result[5], u8Md5Result[6], u8Md5Result[7],
                    u8Md5Result[8], u8Md5Result[9], u8Md5Result[10], u8Md5Result[11],
                    u8Md5Result[12], u8Md5Result[13], u8Md5Result[14], u8Md5Result[15]);

        if(fi) fclose(fi);
        if(fo) fclose(fo);
        if(buffer)
            munmap(buffer, obmem.length);
        if(pixels)
            munmap(pixels, sdmem.length);
        if(sdmem.length > 0)
            ioctl(sysfd, IOCTL_MSYS_RELEASE_DMEM, &sdmem);
        if(obmem.length > 0)
            ioctl(sysfd, IOCTL_MSYS_RELEASE_DMEM, &obmem);

        CamOsDirectMemRelease(pQpMapBufVitr, qpmapsize);
    }
    while(0);

    if(pParam->mhefd > 0)  close(pParam->mhefd);
    if(sysfd > 0)  close(sysfd);
    if(memfd > 0)  close(memfd);

    CamOsPrintf("Exit %s!!!\n", __FUNCTION__);

    return 0;
}

void *MheEncodePthread(void *ptr)
{
    printf("== MheEncodePthread Start ==\n");
    _RunMhe((Mheparam*)ptr);
    printf("== MheEncodePthread Exit ==\n");

    return NULL;
}


int _RunMheEncode(Mheparam** pParam)
{
    int i;
    pthread_t thread[nMultiInsNum];

    for(i = 0 ; i < nMultiInsNum ; i++)
    {
        pthread_create(&thread[i], NULL , MheEncodePthread , (void*) pParam[i]);
    }

    for(i = 0 ; i < nMultiInsNum; i++)
    {
        pthread_join(thread[i], NULL);
        //free(pParam[i]);
    }

    return 0;
}

void _ShowCurrentMHESetting(Mheparam** pParam)
{
    int i;

    for(i = 0 ; i < nMultiInsNum ; i++)
    {
        printf("[Mhe Instance %d Cfg]\n", pParam[i]->nInstanceId);
        ShowCurrentMHESetting(pParam[i]);
        printf("------------------------------\n");
    }
}

int _SetInOutPath(Mheparam** pParam)
{
    int nInstanceId = 0;

    if(nMultiInsNum > 1)
    {
        printf("> Input Mhe Instance Id: ");
        if(scanf("%d" , &nInstanceId) == EOF)
        {
            printf("get nInstanceId fail\n");
        }
    }

    SetInOutPath(pParam[nInstanceId]);

    return 0;
}

void _SetMHEParameter(Mheparam** pParam)
{
    int nInstanceId = 0;

    if(nMultiInsNum > 1)
    {
        printf("> Input Mhe Instance Id: ");
        if(scanf("%d" , &nInstanceId) == EOF)
        {
            printf("get nInstanceId fail\n");
        }
    }

    SetMHEParameter(pParam[nInstanceId]);
}

int _ReadDefaultConfig(Mheparam** pParam)
{
    int i;

    for(i = 0 ; i < nMultiInsNum; i++)
    {
        printf("[Read Mhe Instance %d Cfg: %s]\n", pParam[i]->nInstanceId, pParam[i]->cfgName);
        ReadDefaultConfig(pParam[i]);
        printf("------------------------------\n");
    }

    return 0;
}


int main(int argc, char** argv)
{
    int i;
    int cmd;

    if(argc > 1)
    {
        printf("> Run Mhe Instance Test: %d <\n", atoi(argv[1]));
        nMultiInsNum = atoi(argv[1]);
    }

    Mheparam* pMheI2[nMultiInsNum];
    for(i = 0 ; i < nMultiInsNum; i++)
    {
        pMheI2[i] = (Mheparam*)malloc(sizeof(Mheparam));
        memset(pMheI2[i], 0, sizeof(Mheparam));

        if(i == 0)
            snprintf(pMheI2[i]->cfgName, 64, "hevc.cfg");
        else
            snprintf(pMheI2[i]->cfgName, 64, "hevc%d.cfg", i);

        ReadDefaultConfig(pMheI2[i]);

        pMheI2[i]->nInstanceId = i;
    }

    DisplayMenuSetting();

    do
    {
        printf("==>");
        cmd = getchar();
        switch(cmd)
        {
            case 'f':
                _SetInOutPath(pMheI2);
                DisplayMenuSetting();
                break;
            case 's':
                _SetMHEParameter(pMheI2);
                DisplayMenuSetting();
                break;
            case 'e':
                _RunMheEncode(pMheI2);
                DisplayMenuSetting();
                break;
            case 'r':
                _ReadDefaultConfig(pMheI2);
                break;
            case 'p':
                _ShowCurrentMHESetting(pMheI2);
                break;
            case 'h':
                DisplayMenuSetting();
                break;
            default:
                break;
        }
    }
    while('q' != cmd);

    for(i = 0 ; i < nMultiInsNum; i++)
        free(pMheI2[i]);
    return 0;
}
