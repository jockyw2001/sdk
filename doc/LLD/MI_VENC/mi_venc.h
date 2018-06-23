//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
#ifndef _MI_VENC_H_
#define _MI_VENC_H_

#include "mi_venc_internal.h"

//TODO make this driver variables later?
#define VENC_MAX_CHN_NUM_PER_MODULE (16)
//max supported channel number. But the number would be limited by each module.
#define VENC_MAX_CHN_NUM	(VENC_MAX_CHN_NUM_PER_MODULE * 4) //MHE*2, MFE*1, JPEG
#define RC_TEXTURE_THR_SIZE	12

typedef enum
{
  E_MI_VENC_MODTYPE_VENC = 1,
  E_MI_VENC_MODTYPE_H264E,
  E_MI_VENC_MODTYPE_H265E,
  E_MI_VENC_MODTYPE_JPEGE,
  E_MI_VENC_MODTYPE_BUTT
} MI_VENC_ModType_e;

typedef enum
{
  E_MI_VENC_H264E_NALU_PSLICE    =    1,
  E_MI_VENC_H264E_NALU_ISLICE    =    5,
  E_MI_VENC_H264E_NALU_SEI    =    6,
  E_MI_VENC_H264E_NALU_SPS    =    7,
  E_MI_VENC_H264E_NALU_PPS    =    8,
  E_MI_VENC_H264E_NALU_IPSLICE=9,
  E_MI_VENC_H264E_NALU_BUTT
} MI_VENC_H264eNaluType_e;

typedef enum
{
  E_MI_VENC_H264E_REFSLICE_FOR_1X    =    1,
  E_MI_VENC_H264E_REFSLICE_FOR_2X    =    2,
  E_MI_VENC_H264E_REFSLICE_FOR_4X,
  E_MI_VENC_H264E_REFSLICE_FOR_BUTT    =    5
} MI_VENC_H264eRefSliceType_e;

typedef enum
{
  E_MI_VENC_JPEGE_PACK_ECS    =    5,
  E_MI_VENC_JPEGE_PACK_APP    =    6,
  E_MI_VENC_JPEGE_PACK_VDO    =    7,
  E_MI_VENC_JPEGE_PACK_PIC    =    8,
  E_MI_VENC_JPEGE_PACK_BUTT
} MI_VENC_JpegePackType_e;

typedef enum
{
  E_MI_VENC_H265E_NALU_PSLICE    =    1,
  E_MI_VENC_H265E_NALU_ISLICE    =    19,
  E_MI_VENC_H265E_NALU_VPS    =    32,
  E_MI_VENC_H265E_NALU_SPS    =    33,
  E_MI_VENC_H265E_NALU_PPS    =    34,
  E_MI_VENC_H265E_NALU_SEI    =    39,
  E_MI_VENC_H265E_NALU_BUTT
} MI_VENC_H265eNaulType_e;

typedef union MI_VENC_DataType_s
{
  MI_VENC_H264eNaluType_e    eH264EType;
  MI_VENC_JpegePackType_e    eJPEGEType;
  MI_VENC_Mpeg4ePackType_e eMPEG4EType;
  MI_VENC_H265eNaulType_e    eH265EType;
}MI_VENC_DataType_t;

typedef struct MI_VENC_PackInfo_s
{
  MI_VENC_DataType_t stPackType;
  MI_U32 u32PackOffset;
  MI_U32 u32PackLength;
} MI_VENC_PackInfo_t;

typedef struct MI_VENC_Pack_s
{
  MI_U32    u32PhyAddr;
  MI_U8    *pu8Addr;
  MI_U32    u32Len;
  MI_U64    u64PTS;
  MI_BOOL    bFrameEnd;
  MI_VENC_DataType_t    stDataType;
  MI_U32    u32Offset;
  MI_U32    u32DataNum;
  MI_VENC_PackInfo_t asackInfo[8];
}MI_VENC_Pack_t;

typedef struct MI_VENC_StreamInfoH264_s
{
  MI_U32 u32PicBytesNum;
  MI_U32 u32PSkipMbNum;
  MI_U32 u32IpcmMbNum;
  MI_U32 u32Inter16x8MbNum;
  MI_U32 u32Inter16x16MbNum;
  MI_U32 u32Inter8x16MbNum;
  MI_U32 u32Inter8x8MbNum;
  MI_U32 u32Intra16MbNum;
  MI_U32 u32Intra8MbNum;
  MI_U32 u32Intra4MbNum;
  MI_VENC_H264eRefSliceType_e eRefSliceType;
  MI_VENC_H264eRefType_e    eRefType;
  MI_U32 u32UpdateAttrCnt;
  MI_U32 u32StartQp;
}MI_VENC_StreamInfoH264_t;

typedef struct MI_VENC_StreamInfoJpeg_s
{
  MI_U32 u32PicBytesNum;
  MI_U32 u32UpdateAttrCnt;
  MI_U32 u32Qfactor;
}MI_VENC_StreamInfoJpeg_t;

typedef struct MI_VENC_StreamInfoH265_s
{
  MI_U32 u32PicBytesNum; MI_U32 u32Inter64x64CuNum;
  MI_U32 u32Inter32x32CuNum;
  MI_U32 u32Inter16x16CuNum;
  MI_U32 u32Inter8x8CuNum;
  MI_U32 u32Intra32x32CuNum;
  MI_U32 u32Intra16x16CuNum;
  MI_U32 u32Intra8x8CuNum;
  MI_U32 u32Intra4x4CuNum;
  H265E_REF_TYPE_E    enRefType;
  MI_U32  u32UpdateAttrCnt;
  MI_U32 u32StartQp;
}MI_VENC_StreamInfoH265_t;

typedef struct MI_VENC_Stream_s
{
  MI_VENC_Pack_t *pstPack;
  MI_U32    u32PackCount;
  MI_U32    u32Seq; union
  {
    MI_VENC_StreamInfoH264_t stH264Info;
    MI_VENC_StreamInfoJpeg_t stJpegInfo;
    MI_VENC_StreamInfoMpeg4_t stMpeg4Info;
    MI_VENC_StreamInfoH265_t stH265Info;
  };
}MI_VENC_Stream_t;

typedef struct MI_VENC_StreamBufInfo_s
{
  MI_PHY    u32PhyAddr;
  MI_VOID *pUserAddr;
  MI_U32    u32BufSize;
} MI_VENC_StreamBufInfo_t;

typedef struct MI_VENC_AttrH264_s
{
  MI_U32 u32MaxPicWidth;
  MI_U32 u32MaxPicHeight;
  MI_U32  u32BufSize;
  MI_U32 u32Profile;
  MI_BOOL bByFrame;
  MI_U32 u32PicWidth;
  MI_U32 u32PicHeight;
  MI_U32 u32BFrameNum;
  MI_U32 u32RefNum;
}MI_VENC_AttrH264_t;

typedef struct MI_VENC_AttrMjpeg_s
{
  MI_U32 u32MaxPicWidth;
  MI_U32 u32MaxPicHeight;
  MI_U32    u32BufSize;
  MI_BOOL  bByFrame;
  MI_U32    u32PicWidth;
  MI_U32    u32PicHeight;
}MI_VENC_AttrMjpeg_t;

typedef struct MI_VENC_AttrJpeg_s
{
  MI_U32 u32MaxPicWidth;
  MI_U32 u32MaxPicHeight;
  MI_U32    u32BufSize;
  MI_BOOL  bByFrame;
  MI_U32    u32PicWidth;
  MI_U32    u32PicHeight;
  MI_BOOL bSupportDCF;
}MI_VENC_AttrJpeg_t;

typedef struct MI_VENC_AttrH265_s
{
  MI_U32 u32MaxPicWidth;
  MI_U32 u32MaxPicHeight;
  MI_U32  u32BufSize;
  MI_U32 u32Profile;
  MI_BOOL bByFrame;
  MI_U32 u32PicWidth;
  MI_U32 u32PicHeight;
  MI_U32 u32BFrameNum;
  MI_U32 u32RefNum;
}MI_VENC_AttrH265_t;

typedef struct MI_VENC_Attr_s
{
  MI_VENC_PayloadType_e enType;
  union
  {
    MI_VENC_AttrH264_t stAttrH264e;
    MI_VENC_AttrMjpeg_t stAttrMjpeg;
    MI_VENC_AttrJpeg_t stAttrJpeg;
    MI_VENC_AttrMpeg4_t stAttrMpeg4;
    MI_VENC_AttrH265_t stAttrH265e;
  };
}MI_VENC_Attr_t;

typedef struct MI_VENC_RcAttr_s MI_VENC_RcAttr_t;
typedef struct MI_VENC_ChnAttr_s
{
  MI_VENC_Attr_t    stVeAttr;
  MI_VENC_RcAttr_t stRcAttr;
}MI_VENC_ChnAttr_t;


typedef struct MI_VENC_ChnStat_s
{
  MI_U32  u32LeftPics;
  MI_U32 u32LeftStreamBytes;
  MI_U32 u32LeftStreamFrames;
  MI_U32  u32CurPacks;
  MI_U32 u32LeftRecvPics;
  MI_U32 u32LeftEncPics;
}MI_VENC_ChnStat_t;

typedef struct MI_VENC_ParamH264SliceSplit_s
{
  MI_BOOL bSplitEnable;
  MI_U32 u32SliceRowCount;
} MI_VENC_ParamH264SliceSplit_t;

typedef struct MI_VENC_ParamH264InterPred_s
{
  /* search window */
  MI_U32 u32HWSize;
  MI_U32 u32VWSize;
  MI_BOOL bInter16x16PredEn;
  MI_BOOL bInter16x8PredEn;
  MI_BOOL bInter8x16PredEn;
  MI_BOOL bInter8x8PredEn;
  MI_BOOL bExtedgeEn;
} MI_VENC_ParamH264InterPred_t;

typedef struct MI_VENC_ParamH264IntraPred_s
{
  MI_BOOL bIntra16x16PredEn;
  MI_BOOL bIntraNxNPredEn;
  MI_U32 constrained_intra_pred_flag;
  MI_BOOL bIpcmEn;
  MI_U32 u32Intra16x16Penalty;
  MI_U32 u32Intra4x4Penalty;
  MI_BOOL bIntraPlanarPenalty;
}MI_VENC_ParamH264IntraPred_t;

typedef struct MI_VENC_ParamH264Trans_s
{
  MI_U32 u32IntraTransMode;
  MI_U32 u32InterTransMode;
  MI_S32 s32ChromaQpIndexOffset;
}MI_VENC_ParamH264Trans_t;

typedef struct MI_VENC_ParamH264Entropy_s
{
  MI_U32 u32EntropyEncModeI;
  MI_U32 u32EntropyEncModeP;
}MI_VENC_ParamH264Entropy_t;

typedef struct MI_VENC_ParamH265InterPred_s
{
  /* search window */
  MI_U32 u32HWSize;
  MI_U32 u32VWSize;
  MI_BOOL bInter16x16PredEn;
  MI_BOOL bInter16x8PredEn;
  MI_BOOL bInter8x16PredEn;
  MI_BOOL bInter8x8PredEn;
  MI_BOOL bExtedgeEn;
} MI_VENC_ParamH265InterPred_t;

typedef struct MI_VENC_ParamH265IntraPred_s
{
  MI_U32 u32Intra32x32Penalty;
  MI_U32 u32Intra16x16Penalty;
  MI_U32 u32Intra4x4Penalty;
}MI_VENC_ParamH265IntraPred_t;

typedef struct MI_VENC_ParamH265Trans_s
{
  MI_U32 u32IntraTransMode;
  MI_U32 u32InterTransMode;
  MI_S32 s32ChromaQpIndexOffset;
}MI_VENC_ParamH265Trans_t;

typedef struct MI_VENC_ParamH264Dblk_s
{
  MI_U32 disable_deblocking_filter_idc;
  MI_S32 slice_alpha_c0_offset_div2;
  MI_S32 slice_beta_offset_div2;
}MI_VENC_ParamH264Dblk_t;

typedef struct MI_VENC_ParamH264Vui_s
{
  MI_VENC_ParamH264VuiAspectRatio_t    stVuiAspectRatio;
  MI_VENC_ParamH264VuiTimeInfo_t        stVuiTimeInfo;
  MI_VENC_ParamH264VuiVideoSignal_t    stVuiVideoSignal;
}MI_VENC_ParamH264Vui_t;

typedef struct MI_VENC_ParamH264VuiAspectRatio_s
{
  MI_U8    u8AspectRatioInfoPresentFlag;
  MI_U8    u8AspectRatioIdc;
  MI_U8    u8OverscanInfoPresentFlag;
  MI_U8    u8OverscanAppropriateFlag;
  MI_U16    u16SarWidth;
  MI_U16    u16SarHeight;
}MI_VENC_ParamH264VuiAspectRatio_t;

typedef struct MI_VENC_ParamH264VuiTimeInfo_s
{
  MI_U8    u8TimingInfoPresentFlag;
  MI_U8    u8FixedFrameRateFlag;
  MI_U32    u32NumUnitsInTick;
  MI_U32    u32TimeScale;
}MI_VENC_ParamH264VuiTimeInfo_t;

typedef struct MI_VENC_ParamVuiVideoSignal_s
{
  MI_U8    u8VideosignalTypePresentFlag;
  MI_U8    u8VideoFormat;
  MI_U8    u8VideoFullRangeFlag;
  MI_U8    u8ColourDescriptionPresentFlag;
  MI_U8    u8ColourPrimaries;
  MI_U8    u8TransferCharacteristics;
  MI_U8    u8MatrixCoefficients;
}MI_VENC_ParamH264VuiVideoSignal_t;

typedef struct MI_VENC_ParamJpeg_s
{
  MI_U32 u32Qfactor;
  MI_U8 u8YQt[64];
  MI_U32 u32MCUPerECS;
} MI_VENC_ParamJpeg_t;

typedef struct MI_VENC_ParamMjpeg_s
{
  MI_U8 u8YQt[64];
  MI_U32 u32MCUPerECS;
} MI_VENC_ParamMjpeg_t;

typedef struct MI_VENC_RoiCfg_s
{
  MI_U32 u32Index;
  MI_BOOL bEnable;
  MI_BOOL bAbsQp;
  RECT_S stRect;
  MI_U8 *pDaQpMap;
}MI_VENC_RoiCfg_t;

typedef struct MI_VENC_RoiBgFrameRate_s
{
  MI_S32 s32SrcFrmRate;
  MI_S32 s32DstFrmRate;
}MI_VENC_RoiBgFrameRate_t;

typedef struct MI_VENC_ParamRef_s
{
  MI_U32  u32RefLayerMode;
}MI_VENC_ParamRef_t;


typedef struct MI_VENC_RcAttr_s
{
  MI_VENC_RcMode_e eRcMode;
  union
  {
    MI_VENC_AttrH264Cbr_t    stAttrH264Cbr;
    MI_VENC_AttrH264Vbr_t    stAttrH264Vbr;
    MI_VENC_AttrH264FixQp_t stAttrH264FixQp;
    MI_VENC_AttrH264Abr_t    stAttrH264Abr;
    MI_VENC_AttrH265Cbr_t    stAttrH265Cbr;
    MI_VENC_AttrH265Vbr_t    stAttrH265Vbr;
    MI_VENC_AttrH265FixQp_t stAttrH265FixQp;
  };
  MI_VOID*    pRcAttr ;
}MI_VENC_RcAttr_t;

typedef enum
{
  E_MI_VENC_RC_MODE_H264CBR = 1,
  E_MI_VENC_RC_MODE_H264VBR,
  E_MI_VENC_RC_MODE_H264ABR,
  E_MI_VENC_RC_MODE_H264FIXQP,
  E_MI_VENC_RC_MODE_MJPEGFIXQP,
  E_MI_VENC_RC_MODE_H265CBR,
  E_MI_VENC_RC_MODE_H265VBR,
  E_MI_VENC_RC_MODE_H265FIXQP,
  E_MI_VENC_RC_MODE_BUTT,
}MI_VENC_RcMode_e;

typedef struct MI_VENC_AttrH264Cbr_s
{
  MI_U32    u32Gop;
  MI_U32    u32StatTime;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32BitRate;
  MI_U32    u32FluctuateLevel;
} MI_VENC_AttrH264Cbr_t;

typedef struct MI_VENC_AttrH264Vbr_s
{
  MI_U32    u32Gop;
  MI_U32    u32StatTime;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32MaxBitRate;
  MI_U32    u32MaxQp;
  MI_U32    u32MinQp;
}MI_VENC_AttrH264Vbr_t;

typedef struct MI_VENC_AttrH264FixQp_s
{
  MI_U32    u32Gop;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32IQp;
  MI_U32    u32PQp;
} MI_VENC_AttrH264FixQp_t;

typedef struct MI_VENC_AttrH264Abr_s
{
  MI_U32    u32Gop;    /*the interval of ISLICE. */
  MI_U32    u32StatTime;    /* the rate statistic time, the unit is senconds(s) */
  MI_U32    u32SrcFrmRate;    /* the input frame rate of the venc */
  MI_U32    u32AvgBitRate;    /* average bitrate */ MI_U32    u32MaxBitRate;    /* the max bitrate */
}MI_VENC_AttrH264Abr_t;

typedef struct MI_VENC_AttrMjpegFixQp_s
{
  MI_U32    u32Qfactor;
}MI_VENC_AttrMjpegFixQp_t;

typedef struct MI_VENC_AttrH265Cbr_s
{
  MI_U32    u32Gop;
  MI_U32    u32StatTime;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32BitRate;
  MI_U32    u32FluctuateLevel;
} MI_VENC_AttrH265Cbr_t;

typedef struct MI_VENC_AttrH265Vbr_s
{
  MI_U32    u32Gop;
  MI_U32    u32StatTime;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32MaxBitRate;
  MI_U32    u32MaxQp;
  MI_U32    u32MinQp;
}MI_VENC_AttrH265Vbr_t;

typedef struct MI_VENC_AttrH265FixQp_s
{
  MI_U32    u32Gop;
  MI_U32    u32SrcFrmRate;
  MI_U32    u32IQp;
  MI_U32    u32PQp;
} MI_VENC_AttrH265FixQp_t;

typedef enum
{
  E_MI_VENC_SUPERFRM_NONE,
  E_MI_VENC_SUPERFRM_DISCARD,
  E_MI_VENC_SUPERFRM_REENCODE,
  E_MI_VENC_SUPERFRM_BUTT
}MI_VENC_SuperFrmMode_e;

typedef struct MI_VENC_ParamH264Vbr_s
{
  MI_S32    s32IPQPDelta;
  MI_S32    s32ChangePos;
  MI_U32 u32MinIprop;
  MI_U32 u32MaxIprop;
  MI_U32 u32MinIQP;
}MI_VENC_ParamH264Vbr_t;

typedef struct MI_VENC_ParamH264Cbr_s
{
  MI_U32 u32MinIprop;
  MI_U32 u32MaxIprop;
  MI_U32 u32MaxQp;
  MI_U32 u32MinQp;
  MI_S32 s32IPQPDelta;
  MI_S32 s32QualityLevel;
  MI_U32 u32MinIQp;
}MI_VENC_ParamH264Cbr_t;

typedef struct MI_VENC_ParamH265Vbr_s
{
  MI_S32 s32IPQPDelta;
  MI_U32 u32MinIQp;
}MI_VENC_ParamH265Vbr_t;

typedef struct MI_VENC_ParamH265Cbr_s
{
  MI_U32 u32MaxQp;
  MI_U32 u32MinQp;
  MI_S32 s32IPQPDelta;
  MI_S32 s32QualityLevel;
  MI_U32 u32MinIQp;
}MI_VENC_ParamH265Cbr_t;

typedef struct MI_VENC_RcParam_s
{
  MI_U32 u32ThrdI[RC_TEXTURE_THR_SIZE];
  MI_U32 u32ThrdP[RC_TEXTURE_THR_SIZE];
  MI_U32 u32RowQpDelta;
  union
  {
    MI_VENC_ParamH264Cbr_t stParamH264Cbr;
    MI_VENC_ParamH264Vbr_t stParamH264VBR;
    MI_VENC_ParamMjpegCbr_t stParamMjpegCbr;
    MI_VENC_ParamMjpegVbr_t stParamMjpegVbr;
    MI_VENC_ParamMpeg4Cbr_t stParamMpeg4Cbr;
    MI_VENC_ParamMpeg4Vbr_t stParamMpeg4Vbr;
    MI_VENC_ParamH265Cbr_t    stParamH265Cbr;
    MI_VENC_ParamH265Vbr_t    stParamH265Vbr;
  };
  MI_VOID* pRcParam;
}MI_VENC_RcParam_t;

typedef struct MI_VENC_CropCfg_s
{
  MI_BOOL bEnable;    /* Crop region enable */
  RECT_S stRect;    /* Crop region, note: s32X must be multi of 16 */
}MI_VENC_CropCfg_t;

typedef struct MI_VENC_RecvPicParam_s
{
  MI_S32 s32RecvPicNum;
} MI_VENC_RecvPicParam_t;

typedef enum
{
  E_MI_VENC_H264E_IDR_PIC_ID_MODE_USR,
} MI_VENC_H264eIdrPicIdMode_e;

typedef struct MI_VENC_H264IdrPicIdCfg_s
{
  MI_VENC_H264eIdrPicIdMode_e eH264eIdrPicIdMode;
  MI_U32 u32H264eIdrPicId;
} MI_VENC_H264IdrPicIdCfg_t;

typedef enum
{
  E_MI_VENC_FRMLOST_NORMAL,
  E_MI_VENC_FRMLOST_PSKIP,
  E_MI_VENC_FRMLOST_BUTT,
}MI_VENC_FrameLostMode_e;

typedef struct MI_VENC_ParamFrameLost_s
{
  MI_BOOL bFrmLostOpen;
  MI_U32 u32FrmLostBpsThr;
  MI_VENC_FrameLostMode_e eFrmLostMode;
  MI_U32 u32EncFrmGaps;
}MI_VENC_ParamFrameLost_t;

typedef struct MI_VENC_SuperFrameCfg_s
{
  MI_VENC_SuperFrmMode_e eSuperFrmMode;
  MI_U32 u32SuperIFrmBitsThr;
  MI_U32 u32SuperPFrmBitsThr;
  MI_U32 u32SuperBFrmBitsThr;
} MI_VENC_SuperFrameCfg_t;

typedef enum
{
  E_MI_VENC_RC_PRIORITY_BITRATE_FIRST = 1,
  E_MI_VENC_RC_PRIORITY_FRAMEBITS_FIRST,
  E_MI_VENC_RC_PRIORITY_BUTT,
} MI_VENC_RcPriority_e;


typedef struct MI_VENC_ModParam_s
{
  MI_VENC_ModType_e eVencModType;
  union
  {
    MI_VENC_ParamModVenc_t stVencModParam;
    MI_VENC_ParamModH264e_t stH264eModParam;
    MI_VENC_ParamModH265e_t stH265eModParam;
    MI_VENC_ParamModJpege_t stJpegeModParam;
  };
} MI_VENC_ModParam_t;

typedef enum
{
  E_MI_VENC_MODTYPE_VENC = 1,
  E_MI_VENC_MODTYPE_H264E,
  E_MI_VENC_MODTYPE_H265E,
  E_MI_VENC_MODTYPE_JPEGE,
  E_MI_VENC_MODTYPE_BUTT
} MI_VENC_ModType_e;

typedef struct MI_VENC_ParamModH265e_s
{
  MI_U32 u32OneStreamBuffer;
  MI_U32 u32H265eMiniBufMode;
} MI_VENC_ParamModH265e_t;

typedef struct MI_VENC_ParamModJpege_s
{
  MI_U32 u32OneStreamBuffer;
  MI_U32 u32JpegeMiniBufMode;
} MI_VENC_ParamModJpege_t;


MI_RESULT MI_VENC_SetModParam(MI_VENC_ModParam_t *pstModParam);
MI_RESULT MI_VENC_GetModParam(MI_VENC_ModParam_t *pstModParam);
MI_RESULT MI_VENC_CreateChn(VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr);
MI_RESULT MI_VENC_DestroyChn(VENC_CHN VeChn);
MI_RESULT MI_VENC_ResetChn(VENC_CHN VeChn);
MI_RESULT MI_VENC_StartRecvPic(VENC_CHN VeChn);
MI_RESULT MI_VENC_StartRecvPicEx(VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam);
MI_RESULT MI_VENC_StopRecvPic(VENC_CHN VeChn);
MI_RESULT MI_VENC_Query(VENC_CHN VeChn, MI_VENC_ChnStat_t *pstStat);
MI_RESULT MI_VENC_SetChnAttr(VENC_CHN VeChn, MI_VENC_ChnAttr_t* pstAttr);
MI_RESULT MI_VENC_GetChnAttr(VENC_CHN VeChn, MI_VENC_ChnAttr_t*pstAttr);
MI_RESULT MI_VENC_GetStream(VENC_CHN VeChn, MI_VENC_Stream_t *pstStream,MI_S32 s32MilliSec);
MI_RESULT MI_VENC_ReleaseStream(VENC_CHN VeChn, MI_VENC_Stream_t *pstStream);
MI_RESULT MI_VENC_InsertUserData(VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len);
MI_RESULT MI_VENC_SetMaxStreamCnt(VENC_CHN VeChn,MI_U32 u32MaxStrmCnt);
MI_RESULT MI_VENC_GetMaxStreamCnt(VENC_CHN VeChn,MI_U32 *pu32MaxStrmCnt);
MI_RESULT MI_VENC_RequestIDR(VENC_CHN VeChn, MI_BOOL bInstant);
MI_RESULT MI_VENC_SetH264IdrPicId(VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg);
MI_RESULT MI_VENC_GetH264IdrPicId( VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg );
MI_RESULT MI_VENC_GetFd(VENC_CHN VeChn);
MI_RESULT MI_VENC_CloseFd(VENC_CHN VeChn);
MI_RESULT MI_VENC_SetRoiCfg(VENC_CHN VeChn, MI_VENC_RoiCfg_t *pstVencRoiCfg);
MI_RESULT MI_VENC_GetRoiCfg(VENC_CHN VeChn, MI_U32 u32Index, MI_VENC_RoiCfg_t *pstVencRoiCfg);
MI_RESULT MI_VENC_SetRoiBgFrameRate(VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t * pstRoiBgFrmRate);
MI_RESULT MI_VENC_GetRoiBgFrameRate(VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate);
MI_RESULT MI_VENC_SetH264SliceSplit(VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
MI_RESULT MI_VENC_SetH264InterPred(VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred);
MI_RESULT MI_VENC_GetH264InterPred(VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred);
MI_RESULT MI_VENC_SetH264IntraPred(VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred);
MI_RESULT MI_VENC_GetH264IntraPred(VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred);
MI_RESULT MI_VENC_SetH264Trans(VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
MI_RESULT MI_VENC_GetH264Trans(VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
MI_RESULT MI_VENC_SetH264Entropy(VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
MI_RESULT MI_VENC_GetH264Entropy(VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
MI_RESULT MI_VENC_SetH264Dblk(VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
MI_RESULT MI_VENC_GetH264Dblk(VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
MI_RESULT MI_VENC_SetH264Vui(VENC_CHN VeChn, MI_VENC_ParamH264Vui_t*pstH264Vui);
MI_RESULT MI_VENC_GetH264Vui(VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui);
MI_RESULT MI_VENC_SetRcParam(VENC_CHN VeChn,MI_VENC_RcParam_t *pstRcParam);
MI_RESULT MI_VENC_GetRcParam(VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam);
MI_RESULT MI_VENC_SetRefParam(VENC_CHN VeChn,MI_VENC_ParamRef_t * pstRefParam);
MI_RESULT MI_VENC_GetRefParam(VENC_CHN VeChn, MI_VENC_ParamRef_t * pstRefParam);
MI_RESULT MI_VENC_SetCrop(VENC_CHNVeChn, MI_VENC_CropCfg_t *pstCropCfg);
MI_RESULT MI_VENC_SetCrop(VENC_CHNVeChn, MI_VENC_CropCfg_t *pstCropCfg);
MI_RESULT MI_VENC_SetFrameLostStrategy(VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam);
MI_RESULT MI_VENC_GetFrameLostStrategy(VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam);
MI_RESULT MI_VENC_SetSuperFrameCfg(VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
MI_RESULT MI_VENC_GetSuperFrameCfg(VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
MI_RESULT MI_VENC_SetRcPriority(VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
MI_RESULT MI_VENC_GetRcPriority(VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
#endif
