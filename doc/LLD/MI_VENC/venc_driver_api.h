
//obsolete
#ifdef OBSOLETE
void *MApi_MFE_Init(hal_Ch_Info *mfe_Info,unsigned char *pOPBuf, unsigned int BufSize);
void MApi_MFE_UpdateSetting(hal_Ch_Info *mfe_Info,void *handler);
void MApi_MFE_GetSetting(hal_Ch_Info *mfe_Info,void *handler);
unsigned int MApi_MFE_EncodeOneFrame(void *Handler);
unsigned int MAPi_MFE_UpdateRateControl(void *Handler,RateControl *RCInfo);
IRQ_CallBack(void);
void MApi_MFE_DeInit(void *Handler);


void *MApi_MHE_Init(hal_Ch_Info *mhe_Info,unsigned char *pOPBuf, unsigned int BufSize);
void MApi_MHE_UpdateSetting(hal_Ch_Info *mhe_Info,void *handler);  //mfe_Info ±a cmd and in/out buffer ptr, regster pointers
void MApi_MHE_GetSetting(hal_Ch_Info *mhe_Info,void *handler);
unsigned int MApi_MHE_EncodeOneFrame(void *Handler);
IRQ_CallBack(void);
unsigned int MAPi_MHE_UpdateRateControl(void *Handler,RateControl *RCInfo);
void MApi_MHE_DeInit(void *Handler);


void *MApi_MJPGE_Init(hal_Ch_Info *mjpg_Info,unsigned char *pOPBuf, unsigned int BufSize);
void MApi_MJPGE_UpdateSetting(hal_Ch_Info *mjpg_Info,void *handler);
void MApi_MJPGE_GetSetting(hal_Ch_Info *mjpg_Info,void *handler);
unsigned int MApi_MJPGE_EncodeOneFrame(void *Handler);
unsigned int MAPi_MJPGE_UpdateRateControl(void *Handler,RateControl *RCInfo);
IRQ_CallBack(void);
void MApi_MJPGE_DeInit(void *Handler);
#endif

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

#include "mi_venc_internal.h"

#if 0
typedef enum
{
  VencModType264E=0,
  VencModType265E,
  VencModTypeJpeg,
  VencModTypeUTT
} VENCModType_e;
#endif

typedef struct
{
  U32 LeftPics;
  U32 LeftStreamBytes;
  U32 LeftStreamFrames;
  U32 CurPacks;
  U32 LeftRecvPics;
  U32 LeftEncPics;
}VEncChnState;

typedef struct
{
  BOOL SplitEnable;
  U32 SliceRowCount;
} VEncParamSplit;

typedef struct
{
  /* search window */
  U32 HWSize;
  U32 VWSize;
  BOOL bInter16x16PredEn;
  BOOL bInter16x8PredEn;
  BOOL bInter8x16PredEn;
  BOOL bInter8x8PredEn;
  BOOL bExtedgeEn;
} VEncParamH264InterPred;

typedef struct
{
  BOOL bIntra16x16PredEn;
  BOOL bIntraNxNPredEn;
  U32 constrained_intra_pred_flag;
  BOOL bIpcmEn;
  U32 u32Intra16x16Penalty;
  U32 u32Intra4x4Penalty;
  BOOL bIntraPlanarPenalty;
}VEncParamH264IntraPred;

typedef struct
{
  U32 u32IntraTransMode;
  U32 u32InterTransMode;
  S32 s32ChromaQpIndexOffset;
}VEncParamH264Trans;

typedef struct MI_VENC_ParamH264Entropy_s
{
  U32 u32EntropyEncModeI;
  U32 u32EntropyEncModeP;
}VEncParamH264Entropy;

typedef struct
{
 U32 disable_deblocking_filter_idc;
 S32 slice_alpha_c0_offset_div2;
 S32 slice_beta_offset_div2;
}VEncParamH264Dblk;

typedef struct
{
  U8    u8AspectRatioInfoPresentFlag;
  U8    u8AspectRatioIdc;
  U8    u8OverscanInfoPresentFlag;
  U8    u8OverscanAppropriateFlag;
  U16    u16SarWidth;
  U16    u16SarHeight;
}VEncParamH264VuiAspectRatio;

typedef struct
{
  U8    u8TimingInfoPresentFlag;
  U8    u8FixedFrameRateFlag;
  U32    u32NumUnitsInTick;
  U32    u32TimeScale;
}VEncParamH264VuiTimeInfo;

typedef struct
{
  ParamH264VuiAspectRatio    stVuiAspectRatio;
  ParamH264VuiTimeInfo        stVuiTimeInfo;
  //ParamH264VuiVideoSignal    stVuiVideoSignal;
}VEncParamH264Vui;

#if 0
typedef struct
{
  S32 s32SrcFrmRate;
  S32 s32DstFrmRate;
}VEncRoiBgFrameRate;

typedef struct
{
  U32 u32Index;
  BOOL bEnable;
  BOOL bAbsQp;
  RECT_S stRect;
  VEncRoiBgFrameRate RoiBgCtl;
  U8 *pDaQpMap;
}VEncRoiCfg;
#endif

typedef struct
{
  U32  u32RefLayerMode;
}VENCParamRef;

typedef enum
{
  VEncRCModeH264CBR = 1,
  VEncRCModeH264VBR,
  VEncRCModeH264FIXQP,
  VEncRCModeH265CBR,
  VEncRCModeH265VBR,
  VEncRCModeH265FIXQP,
  VEncRCModeBUTT,
} VEncRcMode;

typedef struct
{
  U32    Gop;
  U32    StatTime;
  U32    SrcFrmRate;
  U32    BitRate;
  U32    FluctuateLevel;
} VEncH264Cbr;

typedef struct
{
  U32    Gop;
  U32    StatTime;
  U32    SrcFrmRate;
  U32    MaxBitRate;
  U32    MaxQp;
  U32    MinQp;
}VEncH264Vbr;

typedef struct
{
  U32    Gop;
  U32    SrcFrmRate;
  U32    IQp;
  U32    PQp;
} VEncH264FixQp;

typedef struct
{
  U32    Qfactor;
}VEncMJPGRC;

typedef struct
{
  U32    Gop;
  U32    StatTime;
  U32    SrcFrmRate;
  U32    BitRate;
  U32    FluctuateLevel;
} VEncH265Cbr;

typedef struct
{
  U32    Gop;
  U32    StatTime;
  U32    SrcFrmRate;
  U32    MaxBitRate;
  U32    MaxQp;
  U32    MinQp;
} VEncH265Vbr;

typedef struct
{
  U32    Gop;
  U32    SrcFrmRate;
  U32    IQp;
  U32    PQp;
} VEncH265FixQp;

#if 0
typedef struct
{
  VEncRcMode RcMode;
  union
  {
    VEncH264Cbr    AttrH264Cbr;
    VEncH264Vbr    AttrH264Vbr;
    VEncH264FixQp AttrH264FixQp;
    VEncH265Cbr    AttrH265Cbr;
    VEncH265Vbr    AttrH265Vbr;
    VEncH265FixQp AttrH265FixQp;
    VEncMJPGRC AttrMJPGRc;
  };
#define RC_TEXTURE_THR_SIZE 12 //FIXME This was in mi_enc.h which is higher layer
  U32 u32ThrdI[RC_TEXTURE_THR_SIZE];
  U32 u32ThrdP[RC_TEXTURE_THR_SIZE];
  U32 u32RowQpDelta;
  U8 *PerceptureQpMap;
}VENCRcInfo;
#endif

typedef enum
{
  E_MI_VENC_SUPERFRM_NONE,
  E_MI_VENC_SUPERFRM_DISCARD,
  E_MI_VENC_SUPERFRM_REENCODE,
  E_MI_VENC_SUPERFRM_BUTT
}VEncSuperFrmMode_e;

typedef struct
{
  bool bEnable;    /* Crop region enable */
  RECT_S stRect;    /* Crop region, note: s32X must be multi of 16 */
}VEncCropCfg;

typedef enum
{
  VEncFrameLostNormal,
  VEncFrameLostPskip,
  VEncFrameLostButt,
}VencFrameLostMode_e;

typedef struct
{
  BOOL bFrmLostOpen;
  U32 u32FrmLostBpsThr;
  VencFrameLostMode_e FrmLostMode;
  U32 u32EncFrmGaps;
}VencParamFrameLost;

typedef struct
{
  VEncSuperFrmMode_e SuperFrmMode;
  U32 SuperIFrmBitsThr;
  U32 SuperPFrmBitsThr;
  U32 SuperBFrmBitsThr;
} VEncSuperFrameCfg;

typedef enum
{
  VEncRCPriorityBitrateFirst = 1,
  VEncRCPriorityFrameBitsFirst,
  VEncRCPriorityButt,
} VEncRcPriority;

//==================================================//
typedef struct
{
  VENCRcInfo RcCtl;
  VencParamFrameLost FrameLostCtl;
  VEncSuperFrameCfg SuperFrameCtl;
  VEncRcPriority RcPriorityCtl;
} VEncRcControl;

typedef struct
{
  VEncCropCfg CropCtl;
  VENCParamRef RefCtl;
  VEncParamH264Vui VuiCtl;
  VEncRoiCfg ROICtl;
  VEncParamH264Dblk DblkCtl;
  VEncParamH264Entropy RntCtl;
  VEncParamH264Trans TranCtl;
  VEncParamH264IntraPred IntraCtl;
  VEncParamH264InterPred InterCtl;
  VEncParamSplit SplitCtl;
} VEncH264Feature;

//temporary definition, which should be wrong but leave it as they are. Fix them while implementing driver.
typedef VEncParamH264Vui VEncParamH265Vui;
typedef VEncParamH264Trans VEncParamH265Trans;
typedef VEncParamH264IntraPred VEncParamH265IntraPred;
typedef VEncParamH264InterPred VEncParamH265InterPred;
typedef VEncH264Feature VEncJPGEFeature;

typedef struct
{
  VEncCropCfg CropCtl;
  VENCParamRef RefCtl;
  VEncParamH265Vui VuiCtl;
  VEncRoiCfg ROICtl;
  //VEncParamH265Dblk DblkCtl;
  //VEncParamH265Entropy RntCtl;
  VEncParamH265Trans TranCtl;
  VEncParamH265IntraPred IntraCtl;
  VEncParamH265InterPred InterCtl;
  VEncParamSplit SplitCtl;
} VEncH265Feature;



typedef struct
{
  U32 *RegBase0;//input
  U32 *RegBase1;//input
  bool AutoUnLockFlg;//register, wait for interrupt

  U32 *PseudoRegisterBase0;
  U32 PseudoRegisterSet0Size;
  U32 *PseudoRegisterBase1;
  U32 PseudoRegisterSet1Size;

} VencOut;

#if 0
typedef struct VENCChInfos
{
  VEncRcControl RcH264;
  VEncH264Feature Feature264;

  VencOut OutCtl;
} VencParamModH264e;
#endif

typedef struct VENCChInfos
{
  VEncRcControl RcH265;
  VEncH265Feature FeatureH265;

  VencOut OutCtl;
} VencParamModH265e;

typedef struct VENCChInfos
{
  VEncRcControl RcJPGE;
  VEncJPGEFeature FutureJPGE;

  VencOut OutCtl;
} VencParamModJpege;




typedef struct VENCChInfos
{
  VENCModType_e eVencModType;
  union
  {
    VencParamModH264e H264eModParam;
    VencParamModH265e H265eModParam;
    VencParamModJpege JpegeModParam;
  };
} Ch_Info;

typedef struct venc_hal_drv_funcs_t {
    void (*pfn_init)(Ch_Info *chInfo, unsigned char *pOPBuf, unsigned int BufSize);
    void (*pfn_set)(void *handler, Ch_Info *chInfo);
    void (*pfn_get)(void *handler, Ch_Info *chInfo);
    unsigned int (*pfn_enc_one_frm)(void *Handler);
    unsigned int (*pfn_update_rc)(void *Handler, RateControl *RCInfo);
    //MfeIrqCallBack(void);
    unsigned int (*pfn_frm_done)(void *Handler);
    void (*pfn_deinit)(void *Handler);
} venc_hal_drv_funcs_t;

void *HalMfeInit(Ch_Info *mfe_Info,unsigned char *pOPBuf, unsigned int BufSize);
void HALMfeUpdateSetting(Ch_Info *mfe_Info,void *handler);
void HalMfeGetSetting(Ch_Info *mfe_Info,void *handler);
unsigned int HalMfeEncodeOneFrame(void *Handler);
unsigned int HalMfeUpdateRateControl(void *Handler,RateControl *RCInfo);
//MfeIrqCallBack(void);
void HalMfeDeInit(void *Handler);


void *HalMheInit(Ch_Info *mhe_Info,unsigned char *pOPBuf, unsigned int BufSize);
void HalMheUpdateSetting(Ch_Info *mhe_Info,void *handler);  //mfe_Info ±a cmd and in/out buffer ptr, regster pointers
void HalMheGetSetting(Ch_Info *mhe_Info,void *handler);
unsigned int HalMheEncodeOneFrame(void *Handler);
//MheIrqCallBack(void);
unsigned int HalMheUpdateRateControl(void *Handler,RateControl *RCInfo);
void HalMheDeInit(void *Handler);


void *HalJpgeInit(Ch_Info *mjpg_Info,unsigned char *pOPBuf, unsigned int BufSize);
void HalJpgeUpdateSetting(Ch_Info *mjpg_Info,void *handler);
void HalJpgeGetSetting(Ch_Info *mjpg_Info,void *handler);
unsigned int HalJpgeEncodeOneFrame(void *Handler);
unsigned int HalJpgeUpdateRateControl(void *Handler,RateControl *RCInfo);
//JpgeIrqCallBack(void);
void HalJpgeDeInit(void *Handler);
