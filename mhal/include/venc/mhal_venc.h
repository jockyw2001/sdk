#ifndef MI_VENC_MHAL_VENC_H_
#define MI_VENC_MHAL_VENC_H_

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

#include "mhal_common.h"

#define MHAL_VENC_RC_TEXTURE_THR_SIZE     1       // Fixme

#define CONNECT_DUMMY_HAL (1) //Set 1 for dummy HAL driver testing

#define MHAL_VENC_DUMMY_IRQ (10000) //a random invalid dummy number
//#define MHAL_VENC_DUMMY_CMD_BUF_LEN (0)

/****************************************************************************************
|----------------------------------------------------------------|
| 1 |  USAGE_ID  |  MODULE_ID  |           INDEX_ID              |
|----------------------------------------------------------------|
|<--><--3bits----><--12 bits---><-----------16 bits ------------>|

Fixed 1 to ensure the length if index is declared in enum.
MODULE_ID: Use type VENCModType for now, while this could be use as bit field later.
           for example, Rate Control structure could be used for both H.264 and H.265
****************************************************************************************/
#define MHAL_VENC_DEF_IDX(modType, usageType, ID) \
    (( (0x80000000) | ((usageType & 0x7) << 28) | ((modType & 0xFFF) << 16) | (ID & 0xFFFF) ))

#define MHAL_VENC_SET_VER(st, ver) (st).stVerCtl.u32Size = sizeof(st); (st).stVerCtl.u32Version = ver

///Quickly set the version only of the variable is TYPE and the version is TYPE_ver.
///e.g. MHAL_VENC_ChnState_t (type), and MHAL_VENC_ChnState_t_ver (define)
#define MHAL_VENC_INIT_PARAM(type, var) memset(&(var), 0, sizeof(type)); MHAL_VENC_SET_VER(var,type##_ver)

typedef enum
{
    E_MHAL_VENC_USAGE_UNSPECIFIED,
    E_MHAL_VENC_USAGE_STATIC_CONFIG,    ///< This should be used while the instance is idle.
    E_MHAL_VENC_USAGE_PARAM,            ///< This should be used while the instance is idle or running.
    E_MHAL_VENC_USAGE_RUNTIME,          ///< This should be used while the instance is running.
    E_MHAL_VENC_USAGE_MAX,
    E_MHAL_VENC_USAGE_LIMIT = 7         ///< Manually check if E_MHAL_USAGE_MAX should <= this number
} MHAL_VENC_Usage_e;

typedef enum
{
    E_MHAL_VENC_MOD_TYPE_264E = 0,
    E_MHAL_VENC_MOD_TYPE_265E,
    E_MHAL_VENC_MOD_TYPE_JPEG,
    E_MHAL_VENC_MOD_TYPE_MAX
} MHAL_VENC_ModeType_e;


#define E_MHAL_VENC_H264_CFG_START   MHAL_VENC_DEF_IDX(E_MHAL_VENC_MOD_TYPE_264E, E_MHAL_VENC_USAGE_UNSPECIFIED,  0)
#define E_MHAL_VENC_H265_CFG_START   MHAL_VENC_DEF_IDX(E_MHAL_VENC_MOD_TYPE_265E, E_MHAL_VENC_USAGE_UNSPECIFIED,  0)
#define E_MHAL_VENC_JPEG_CFG_START   MHAL_VENC_DEF_IDX(E_MHAL_VENC_MOD_TYPE_JPEG, E_MHAL_VENC_USAGE_UNSPECIFIED,  0)

///complete index
///CFG: Static Configuration
///RT: Run-time parameters
typedef enum
{
    //common
    E_MHAL_VENC_IDX = MHAL_VENC_DEF_IDX(E_MHAL_VENC_MOD_TYPE_MAX, E_MHAL_VENC_USAGE_UNSPECIFIED, 0),
    E_MHAL_VENC_HW_IRQ_NUM,                                     //MHAL_VENC_PARAM_Int_t
    E_MHAL_VENC_HW_CMDQ_BUF_LEN,                                //MHAL_VENC_PARAM_Int_t
    E_MHAL_VENC_HW_RESOLUTION_ALIGN,                            //MHAL_VENC_ResolutionAlign_t,
    E_MHAL_VENC_IDX_STREAM_ON,                                  //MHAL_VENC_InternalBuf_t, CFG
    E_MHAL_VENC_IDX_STREAM_OFF,                                 //RT
    E_MHAL_VENC_PARTIAL_NOTIFY,                                 //MHAL_VENC_PartialNotify_t, RT
    E_MHAL_VENC_ENABLE_IDR,                                     //MHAL_VENC_EnableIdr_t, CFG
    E_MAHL_VENC_CROP,                                           //MHAL_VENC_CropCfg_t, CFG
    E_MHAL_VENC_ROI,                                            //MHAL_VENC_RoiCfg_t
    E_MHAL_VENC_USER_DATA,                                      //MHAL_VENC_UserData_t
    E_MHAL_VENC_RESTORE,
    E_MHAL_VENC_FRAME_CFG,
    E_MHAL_VENC_IDX_MAX,

    E_MHAL_VENC_JPEG_IDX = E_MHAL_VENC_JPEG_CFG_START,
    E_MHAL_VENC_JPEG_RESOLUTION,
    E_MHAL_VENC_JPEG_CROP,
    E_MHAL_VENC_JPEG_RC,
    E_MHAL_VENC_JPEG_IDX_MAX,

    E_MHAL_VENC_264_RESOLUTION = E_MHAL_VENC_H264_CFG_START,    //MHAL_VENC_Resoluton_t, CFG
    E_MHAL_VENC_264_CROP,                                       //MHAL_VENC_CropCfg_t, CFG
    E_MHAL_VENC_264_REF,                                        //MHAL_VENC_ParamRef_t, CFG
    E_MHAL_VENC_264_VUI,                                        //MHAL_VENC_ParamH264Vui_t, CFG
    E_MHAL_VENC_264_DBLK,                                       //MHAL_VENC_ParamH264Dblk_t, CFG
    E_MHAL_VENC_264_ENTROPY,                                    //MHAL_VENC_ParamH264Entropy_t, CFG
    E_MHAL_VENC_264_TRANS,                                      //MHAL_VENC_ParamH264Trans_t, CFG
    E_MHAL_VENC_264_INTRA_PRED,                                 //MHAL_VENC_ParamH264IntraPred_t, CFG
    E_MHAL_VENC_264_INTER_PRED,                                 //MHAL_VENC_ParamH264InterPred_t, CFG
    E_MHAL_VENC_264_I_SPLIT_CTL,                                //MHAL_VENC_ParamSplit_t, CFG
    E_MHAL_VENC_264_ROI,                                        //MHAL_VENC_RoiCfg_t, RT
    E_MHAL_VENC_264_RC,                                         //MHAL_VENC_RcInfo_t, RT
    E_MHAL_VENC_264_USER_DATA,                                  //MHAL_VENC_UserData_t, RT
    E_MHAL_VENC_264_FRAME_LOST,                                 //MHAL_VENC_ParamFrameLost_t, RT
    E_MHAL_VENC_264_FRAME_CFG,                                  //MHAL_VENC_SuperFrameCfg_t, RT
    E_MHAL_VENC_264_RC_PRIORITY,                                //MHAL_VENC_RcPriorityCfg_t, RT
    E_MHAL_VENC_264_IDX_MAX,

    E_MHAL_VENC_265_RESOLUTION = E_MHAL_VENC_H265_CFG_START,    //MHAL_VENC_Resoluton_t, CFG
    E_MHAL_VENC_265_CROP,                                       //MHAL_VENC_CropCfg_t, CFG
    E_MHAL_VENC_265_REF,                                        //MHAL_VENC_ParamRef_t, CFG
    E_MHAL_VENC_265_VUI,                                        //MHAL_VENC_ParamH265Vui_t, CFG
    E_MHAL_VENC_265_DBLK,                                       //MHAL_VENC_ParamH265Dblk_t, CFG
    E_MHAL_VENC_265_ENTROPY,                                    //Not used. Left it here to compare with h264, CFG
    E_MHAL_VENC_265_TRANS,                                      //MHAL_VENC_ParamH265Trans_t, CFG
    E_MHAL_VENC_265_INTRA_PRED,                                 //MHAL_VENC_ParamH265IntraPred_t, CFG
    E_MHAL_VENC_265_INTER_PRED,                                 //MHAL_VENC_ParamH265InterPred_t, CFG
    E_MHAL_VENC_265_I_SPLIT_CTL,                                //MHAL_VENC_ParamSplit_t, CFG
    E_MHAL_VENC_265_ROI,                                        //MHAL_VENC_RoiCfg_t, RT
    E_MHAL_VENC_265_RC,                                         //MHAL_VENC_RcInfo_t, RT
    E_MHAL_VENC_265_USER_DATA,                                  //MHAL_VENC_UserData_t, RT
    E_MHAL_VENC_265_FRAME_LOST,									//MHAL_VENC_ParamFrameLost_t, RT
    E_MHAL_VENC_265_FRAME_CFG,									//MHAL_VENC_SuperFrameCfg_t, RT
    E_MHAL_VENC_265_RC_PRIORITY,								//MHAL_VENC_RcPriorityCfg_t, RT
    E_MHAL_VENC_265_IDX_MAX,
} MHAL_VENC_Idx_e;

typedef enum
{
    E_MHAL_VENC_RC_MODE_H264CBR = 1,
    E_MHAL_VENC_RC_MODE_H264VBR,
    E_MHAL_VENC_RC_MODE_H264FIXQP,
    E_MHAL_VENC_RC_MODE_H265CBR,
    E_MHAL_VENC_RC_MODE_H265VBR,
    E_MHAL_VENC_RC_MODE_H265FIXQP,
    E_MHAL_VENC_RC_MODE_MAX,
} MHAL_VENC_RCMode_e;

typedef enum
{
    E_MHAL_VENC_SUPERFRM_NONE,
    E_MHAL_VENC_SUPERFRM_DISCARD,
    E_MHAL_VENC_SUPERFRM_REENCODE,
    E_MHAL_VENC_SUPERFRM_MAX
} MHAL_VENC_SuperFrmMode_e;

//Fixme
typedef enum
{
    E_MHAL_VENC_FMT_NV12 = 0,   //!< pixel format NV12.
    E_MHAL_VENC_FMT_NV21,       //!< pixel format NV21.
    E_MHAL_VENC_FMT_YUYV,       //!< pixel format YUYV.
    E_MHAL_VENC_FMT_YVYU,       //!< pixel format YVYU.
} MHAL_VENC_InputFmt_e;

typedef enum
{
    E_MHAL_VENC_FRAME_LOST_NORMAL,
    E_MHAL_VENC_FRAME_LOST_PKSIP,
    E_MHAL_VENC_FRAME_LOST_MAX,
} MHAL_VENC_FrameLostMode_e;

typedef enum
{
    E_MHAL_VENC_FRM_DONE = 0,
    E_MHAL_VENC_BUF_RDY_UNDER_RUN,
    E_MHAL_VENC_BUF_RDY,
    E_MHAL_VENC_OTHERS,
} MHAL_VENC_IrqStatus_e;

typedef enum
{
    E_MHAL_VENC_ROI_MODE_ROIAREA = 0,
    E_MHAL_VENC_ROI_MODE_QPMAP,
    E_MHAL_VENC_ROI_MODE_BGFPS,
} MHAL_VENC_RoiMode_e;

typedef struct MHAL_VENC_Version_s
{
    MS_U32 u32Size; ///< The size of structure in bytes
    union
    {
        struct
        {
            MS_U8 u8VersionMajor;   /**< Major version accessor element */
            MS_U8 u8VersionMinor;   /**< Minor version accessor element */
            MS_U8 u8Revision;       /**< Revision version accessor element */
            MS_U8 u8Step;           /**< Step version accessor element */
        } s;
        MS_U32 u32Version;
    };//GNU anonymous union
} MHAL_VENC_Version_t;

//This is used as function parameter type only. It hints the caller for MHAL_VENC_Param_t types.
//That is, the type with MHAL_VENC_Version_t
typedef void MHAL_VENC_Param_t;

typedef struct MHAL_VENC_PARAM_Int_s
{
    MHAL_VENC_Version_t stVerCtl;
    MS_U32 u32Val;
} MHAL_VENC_ParamInt_t;
#define MHAL_VENC_ParamInt_t_ver (0x01)


/*
///Declare something like this in module driver header file
typedef struct MHAL_VENC_Example_s {
    MHAL_VENC_Version_t stVerCtl;
    MS_U32 u32Example1;
    MS_U32 u32Example2;
    MS_U32 u32Example3;
} MHAL_VENC_Example_t;
#define MHAL_VENC_Example_t_ver (0x01) //version of this type. The format must be <TYPE>_ver
*/
typedef VOID* MHAL_VENC_DEV_HANDLE; ///< might be void* or MS_U32
typedef VOID* MHAL_VENC_INST_HANDLE; ///< might be void* or MS_U32

typedef MS_U32 MHAL_VENC_IDX;

typedef struct MHAL_VENC_Rect_s
{
    MS_U32 u32X;
    MS_U32 u32Y;
    MS_U32 u32W;
    MS_U32 u32H;
} MHAL_VENC_Rect_t;

typedef struct MHAL_VENC_ChnState_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 u32LeftPics;
    MS_U32 u32LeftStreamBytes;
    MS_U32 u32LeftStreamFrames;
    MS_U32 u32CurPacks;
    MS_U32 u32LeftRecvPics;
    MS_U32 u32LeftEncPics;
} MHAL_VENC_ChnState_t;
#define MHAL_VENC_ChnState_t_ver (0x01)

typedef struct MHAL_VENC_ParamSplit_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_BOOL bSplitEnable;
    MS_U32 u32SliceRowCount;
} MHAL_VENC_ParamSplit_t;
#define MHAL_VENC_ParamSplit_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264InterPred_s
{
    MHAL_VENC_Version_t stVerCtl;
    /* search window */
    MS_U32 u32HWSize;
    MS_U32 u32VWSize;
    MS_BOOL bInter16x16PredEn;
    MS_BOOL bInter16x8PredEn;
    MS_BOOL bInter8x16PredEn;
    MS_BOOL bInter8x8PredEn;
    MS_BOOL bExtedgeEn;
} MHAL_VENC_ParamH264InterPred_t;
#define MHAL_VENC_ParamH264InterPred_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264IntraPred_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_BOOL bIntra16x16PredEn;
    MS_BOOL bIntraNxNPredEn;
    MS_U32 constrained_intra_pred_flag;//special naming for CODEC ISO SPEC.
    MS_BOOL bIpcmEn;
    MS_U32 u32Intra16x16Penalty;
    MS_U32 u32Intra4x4Penalty;
    MS_BOOL bIntraPlanarPenalty;
} MHAL_VENC_ParamH264IntraPred_t;
#define MHAL_VENC_ParamH264IntraPred_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264Trans_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 u32IntraTransMode;
    MS_U32 u32InterTransMode;
    MS_S32 s32ChromaQpIndexOffset;
} MHAL_VENC_ParamH264Trans_t;
#define MHAL_VENC_ParamH264Trans_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264Entropy_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 u32EntropyEncModeI;
    MS_U32 u32EntropyEncModeP;
} MHAL_VENC_ParamH264Entropy_t;
#define MHAL_VENC_ParamH264Entropy_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264Dblk_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 disable_deblocking_filter_idc;//special naming for CODEC ISO SPEC.
    MS_S32 slice_alpha_c0_offset_div2;//special naming for CODEC ISO SPEC.
    MS_S32 slice_beta_offset_div2;//special naming for CODEC ISO SPEC.
} MHAL_VENC_ParamH264Dblk_t;
#define MHAL_VENC_ParamH264Dblk_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264VuiAspectRatio_s
{
    MS_U8	u8AspectRatioInfoPresentFlag;
    MS_U8	u8AspectRatioIdc;
    MS_U8	u8OverscanInfoPresentFlag;
    MS_U8	u8OverscanAppropriateFlag;
    MS_U16	u16SarWidth;
    MS_U16	u16SarHeight;
} MHAL_VENC_ParamH264VuiAspectRatio_t;
#define MHAL_VENC_ParamH264VuiAspectRatio_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264VuiTimeInfo_s
{
    MS_U8	u8TimingInfoPresentFlag;
    MS_U8	u8FixedFrameRateFlag;
    MS_U32	u32NumUnitsInTick;
    MS_U32	u32TimeScale;
} MHAL_VENC_ParamH264VuiTimeInfo_t;
#define MHAL_VENC_ParamH264VuiTimeInfo_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264VideoSignal_s
{
	MS_U8 u8VideoSignalTypePresentFlag;
	MS_U8 u8VideoFormat;
	MS_U8 u8VideoFullRangeFlag;
	MS_U8 u8ColourDescriptionPresentFlag;
	MS_U8 u8ColourPrimaries;
	MS_U8 u8TransferCharacteristics;
	MS_U8 u8MatrixCoefficients;
} MHAL_VENC_ParamH264VuiVideoSignal_t;
#define MHAL_VENC_ParamH264VuiVideoSignal_t_ver (0x01)

typedef struct MHAL_VENC_ParamH264Vui_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_ParamH264VuiAspectRatio_t	stVuiAspectRatio;
    MHAL_VENC_ParamH264VuiTimeInfo_t    stVuiTimeInfo;
    MHAL_VENC_ParamH264VuiVideoSignal_t	stVuiVideoSignal;
} MHAL_VENC_ParamH264Vui_t;
#define MHAL_VENC_ParamH264Vui_t_ver (0x02)

typedef struct MHAL_VENC_RoiBgFrameRate_s
{
    MS_U32 u32SrcFrmRate;
    MS_U32 u32DstFrmRate;
} MHAL_VENC_RoiBgFrameRate_t;

typedef struct MHAL_VENC_RoiArea_s
{
    MS_U32 u32Index;
    MS_BOOL bEnable;
    MS_BOOL bAbsQp;
    MS_S32 s32Qp;
    MHAL_VENC_Rect_t stRect;
    //MHAL_VENC_RoiBgFrameRate_t RoiBgCtl;
} MHAL_VENC_RoiArea_t;

typedef struct MHAL_VENC_QpMap_s
{
    MS_BOOL bDaQpMapEnable;
    MS_S8 DaQpMapEntry[14];
    MS_U8 *pDaQpMap;
} MHAL_VENC_QpMap_t;

typedef struct MHAL_VENC_RoiCfg_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_RoiMode_e eRoiMode;

    union
    {
        MHAL_VENC_RoiArea_t stAttrRoiArea;
        MHAL_VENC_QpMap_t   stAttrQpMap;
        MHAL_VENC_RoiBgFrameRate_t RoiBgCtl;
    };
} MHAL_VENC_RoiCfg_t;
#define MHAL_VENC_RoiCfg_t_ver (0x02)

typedef struct MHAL_VENC_ParamRef_s
{
    MHAL_VENC_Version_t stVerCtl;
    MS_U32  u32Base;
    MS_U32  u32Enhance;
    MS_BOOL bEnablePred;
    MS_U32  u32RefLayerMode;
} MHAL_VENC_ParamRef_t;
#define MHAL_VENC_ParamRef_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265Dblk_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 disable_deblocking_filter_idc;
    MS_S32 slice_tc_offset_div2;
    MS_S32 slice_beta_offset_div2;
} MHAL_VENC_ParamH265Dblk_t;
#define MHAL_VENC_ParamH265Dblk_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265Trans_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 u32IntraTransMode;
    MS_U32 u32InterTransMode;
    MS_S32 s32ChromaQpIndexOffset;
    MS_S32 s32CbQpOffset;
    MS_S32 s32CrQpOffset;
} MHAL_VENC_ParamH265Trans_t;
#define MHAL_VENC_ParamH265Trans_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265IntraPred_s
{
    MHAL_VENC_Version_t stVerCtl;

    //MS_BOOL bIntra32x32PredEn;
    //MS_BOOL bIntra16x16PredEn;
    //MS_BOOL bIntra8x8PredEn;
    MS_U32 constrained_intra_pred_flag;
    //MS_BOOL bIpcmEn;
    MS_U32 u32Intra32x32Penalty;
    MS_U32 u32Intra16x16Penalty;
    MS_U32 u32Intra8x8Penalty;
} MHAL_VENC_ParamH265IntraPred_t;
#define MHAL_VENC_ParamH265IntraPred_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265InterPred_s
{
    MHAL_VENC_Version_t stVerCtl;
    /* search window */
    MS_U32 u32HWSize;
    MS_U32 u32VWSize;
    MS_BOOL bInter16x16PredEn;
    MS_BOOL bInter16x8PredEn;
    MS_BOOL bInter8x16PredEn;
    MS_BOOL bInter8x8PredEn;
    MS_BOOL bExtedgeEn;
} MHAL_VENC_ParamH265InterPred_t;
#define MHAL_VENC_ParamH265InterPred_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265VuiAspectRatio_s
{
    MS_U8	u8AspectRatioInfoPresentFlag;
    MS_U8	u8AspectRatioIdc;
    MS_U8	u8OverscanInfoPresentFlag;
    MS_U8	u8OverscanAppropriateFlag;
    MS_U16	u16SarWidth;
    MS_U16	u16SarHeight;
} MHAL_VENC_ParamH265VuiAspectRatio_t;
#define MHAL_VENC_ParamH265VuiAspectRatio_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265VuiTimeInfo_s
{
    MS_U8	u8TimingInfoPresentFlag;
    MS_U32  u32NumUnitsInTick;
    MS_U32  u32TimeScale;
    MS_U32	u32NumTicksPocDiffOneMinus1;
} MHAL_VENC_ParamH265VuiTimeInfo_t;
#define MHAL_VENC_ParamH265VuiTimeInfo_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265VideoSignal_s
{
	MS_U8 u8VideoSignalTypePresentFlag;		//video_signal_type_present_flag ;
	MS_U8 u8VideoFormat;					//video_format ;
	MS_U8 u8VideoFullRangeFlag;				//video_full_range_flag;
	MS_U8 u8ColourDescriptionPresentFlag;	//colour_description_present_flag ;
	MS_U8 u8ColourPrimaries;				//colour_primaries ;
	MS_U8 u8TransferCharacteristics;		//transfer_characteristics;
	MS_U8 u8MatrixCoefficients;				//matrix_coefficients;
} MHAL_VENC_ParamH265VuiVideoSignal_t;
#define MHAL_VENC_ParamH265VuiVideoSignal_t_ver (0x01)

typedef struct MHAL_VENC_ParamH265Vui_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_ParamH265VuiAspectRatio_t	stVuiAspectRatio;
    MHAL_VENC_ParamH265VuiTimeInfo_t    stVuiTimeInfo;
    MHAL_VENC_ParamH265VuiVideoSignal_t	stVuiVideoSignal;
} MHAL_VENC_ParamH265Vui_t;
#define MHAL_VENC_ParamH265Vui_t_ver (0x01)
//================================================================//

typedef struct MHAL_VENC_H264Cbr_s
{
    MS_U32	u32Gop;
    MS_U32	u32StatTime;
    MS_U32	u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32	u32BitRate;
    MS_U32	u32FluctuateLevel;
} MHAL_VENC_H264Cbr_t;

typedef struct MHAL_VENC_H264Vbr_s
{
    MS_U32	u32Gop;
    MS_U32	u32StatTime;
    MS_U32	u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32	u32MaxBitRate;
    MS_U32	u32MaxQp;
    MS_U32	u32MinQp;
} MHAL_VENC_H264Vbr_t;

typedef struct MHAL_VENC_H264FixQp_s
{
    MS_U32	u32Gop;
    MS_U32	u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32	u32IQp;
    MS_U32	u32PQp;
} MHAL_VENC_H264FixQp_t;

typedef struct MHAL_VENC_MjpegRc_s
{
    MS_U32	u32Qfactor;
    MS_U8   u8YQt[64];
    MS_U8   u8CbCrQt[64];
} MHAL_VENC_MjpegRc_t;

typedef struct MHAL_VENC_H265Cbr_s
{
    MS_U32  u32Gop;
    MS_U32  u32StatTime;
    MS_U32  u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32  u32BitRate;
    MS_U32  u32FluctuateLevel;
} MHAL_VENC_H265Cbr_t;

typedef struct MHAL_VENC_H265Vbr_s
{
    MS_U32  u32Gop;
    MS_U32  u32StatTime;
    MS_U32  u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32  u32MaxBitRate;
    MS_U32  u32MaxQp;
    MS_U32  u32MinQp;
} MHAL_VENC_H265Vbr_t;   //Fixme

typedef struct MHAL_VENC_H265FixQp_s
{
    MS_U32  u32Gop;
    MS_U32  u32SrcFrmRateNum;
    MS_U32  u32SrcFrmRateDen;
    MS_U32  u32IQp;
    MS_U32  u32PQp;
} MHAL_VENC_H265FixQp_t;

typedef struct  MHAL_VENC_RcInfo_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_RCMode_e eRcMode;
    union
    {
        MHAL_VENC_H264Cbr_t	  stAttrH264Cbr;
        MHAL_VENC_H264Vbr_t	  stAttrH264Vbr;
        MHAL_VENC_H264FixQp_t stAttrH264FixQp;
        MHAL_VENC_H265Cbr_t	  stAttrH265Cbr;
        MHAL_VENC_H265Vbr_t	  stAttrH265Vbr;
        MHAL_VENC_H265FixQp_t stAttrH265FixQp;
        MHAL_VENC_MjpegRc_t   stAttrMJPGRc;
    };
    MS_U32 au32ThrdI[MHAL_VENC_RC_TEXTURE_THR_SIZE];
    MS_U32 au32ThrdP[MHAL_VENC_RC_TEXTURE_THR_SIZE];
    MS_U32 u32RowQpDelta;
    MS_U8 *pu8PerceptureQpMap;
} MHAL_VENC_RcInfo_t;
#define  MHAL_VENC_RcInfo_t_ver (0x01)

typedef struct MHAL_VENC_Resoluton_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 u32Width;
    MS_U32 u32Height;
    MHAL_VENC_InputFmt_e eFmt;
} MHAL_VENC_Resoluton_t;
#define MHAL_VENC_Resoluton_t_ver (0x01)

typedef struct MHAL_VENC_CropCfg_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_BOOL bEnable;	/* Crop region enable */
    MHAL_VENC_Rect_t stRect;	/* Crop region, note: s32X must be multi of 16 */
} MHAL_VENC_CropCfg_t;
#define MHAL_VENC_CropCfg_t_ver (0x01)

typedef struct MHAL_VENC_UserData_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U8 *pu8Data;
    MS_U32 u32Len;
} MHAL_VENC_UserData_t;
#define MHAL_VENC_UserData_t_ver (0x01)

typedef struct MHAL_VENC_FrameCfg_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_S8 s8QpOffset;
    MS_U32 u32IFrmBitsThr;
    MS_U32 u32PFrmBitsThr;
} MHAL_VENC_FrameCfg_t;
#define MHAL_VENC_FrameCfg_t_ver (0x01)

typedef struct MHAL_VENC_ParamFrameLost_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_BOOL bFrmLostOpen;
    MS_U32 u32FrmLostBpsThr;
    MHAL_VENC_FrameLostMode_e eFrmLostMode;
    MS_U32 u32EncFrmGaps;
} MHAL_VENC_ParamFrameLost_t;
#define MHAL_VENC_ParamFrameLost_t_ver (0x01)

typedef struct MHAL_VENC_SuperFrameCfg_t
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_SuperFrmMode_e eSuperFrmMode;
    MS_U32 u32SuperIFrmBitsThr;
    MS_U32 u32SuperPFrmBitsThr;
    MS_U32 u32SuperBFrmBitsThr;
} MHAL_VENC_SuperFrameCfg_t;
#define MHAL_VENC_SuperFrameCfg_t_ver (0x01)

//Fixme
typedef struct MHAL_VENC_RcPriorityCfg_s
{
    MHAL_VENC_Version_t stVerCtl;

    enum VEncRcPriority
    {
        E_MHAL_VENC_RC_PRIORITY_BITRATE_FIRST = 1,
        E_MHAL_VENC_RC_PRIORITY_FRAME_BITS_FIRST,
        E_MHAL_VENC_RC_PRIORITY_MAX,
    } eRcPriority;
} MHAL_VENC_RcPriorityCfg_t;
#define MHAL_VENC_RcPriorityCfg_t_ver (0x01)

typedef struct MHAL_VENC_EnableIdr_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_BOOL bEnable;
} MHAL_VENC_EnableIdr_t;
#define MHAL_VENC_EnableIdr_t_ver (0x01)

typedef struct MHAL_VENC_DynamicCtl_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_RcInfo_t stRcCtl;
    MHAL_VENC_ParamFrameLost_t stFrameLostCtl;
    MHAL_VENC_SuperFrameCfg_t stSuperFrameCtl;
    MHAL_VENC_RcPriorityCfg_t stRcPriorityCtl;
    MHAL_VENC_RoiCfg_t stRoiCtl;
} MHAL_VENC_DynamicCtl_t;
#define MHAL_VENC_DynamicCtl_t_ver (0x01)

typedef struct MHAL_VENC_H264StaticCtl_s
{
    MHAL_VENC_Version_t stVerCtl;

    MHAL_VENC_Resoluton_t stResCtl;    //Fixme
    MHAL_VENC_CropCfg_t stCropCtl;
    MHAL_VENC_ParamRef_t stRefCtl;
    MHAL_VENC_ParamH264Vui_t stVuiCtl;

    MHAL_VENC_ParamH264Dblk_t stDblkCtl;
    MHAL_VENC_ParamH264Entropy_t stRntCtl;
    MHAL_VENC_ParamH264Trans_t stTranCtl;
    MHAL_VENC_ParamH264IntraPred_t stIntraCtl;
    MHAL_VENC_ParamH264InterPred_t stInterCtl;
    MHAL_VENC_ParamSplit_t stSplitCtl;
} MHAL_VENC_H264StaticCtl_t;
#define MHAL_VENC_H264StaticCtl_t_ver (0x01)

typedef struct MHAL_VENC_InternalBuf_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U8     *pu8IntrAlVirBuf;  //Internal Virtual Buffer Address for algorithm(ROI,PMBR)
    MS_U64     phyIntrAlPhyBuf;   //Internal PHY Buffer Address for algorithm(ROI,PMBR)
    MS_U32     u32IntrAlBufSize;  //Internal Buffer Size for algorithm
    MS_U64     phyIntrRefPhyBuf;  //Internal Ref/Reconstruct PHY Buffer Address
    MS_U32     u32IntrRefBufSize; //Internal Ref/Reconstruct Buffer Size
} MHAL_VENC_InternalBuf_t;
#define MHAL_VENC_InternalBuf_t_ver (0x01)

typedef struct MHAL_VENC_PartialNotify_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32     u32NotifySize;
    void (*notifyFunc)(unsigned long nBufAddr, unsigned long nNotifyOffset, unsigned long nNotifySize, unsigned short bFrameDone);
} MHAL_VENC_PartialNotify_t;
#define MHAL_VENC_PartialNotify_t_ver (0x01)

typedef struct MHAL_VENC_InOutBuf_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32 *pu32RegBase0;
    MS_U32 *pu32RegBase1;
    VOID   *pCmdQ;//type cast to (cmd_mload_interface *)
    MS_BOOL    bRequestI;

    MS_U64     phyInputYUVBuf1;
    MS_U32     u32InputYUVBuf1Size;
    MS_U64     phyInputYUVBuf2;//the second address of YUV
    MS_U32     u32InputYUVBuf2Size;
    MS_U64     phyInputYUVBuf3;//the third address of YUV.
    MS_U32     u32InputYUVBuf3Size;
    MS_U64     phyOutputBuf;
    MS_U64     virtOutputBuf;
    MS_U32     u32OutputBufSize;
    MS_S32     (*pFlushCacheCb)(VOID *va, MS_U32 len);
} MHAL_VENC_InOutBuf_t;
#define MHAL_VENC_InOutBuf_t_ver (0x01)

typedef struct MHAL_VENC_EncResult_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32      eStatus;    // MFE_CB_FRAMEDONE, MFE_CB_BUF_OVERFLOW
    MS_U64      phyOutputBuf;
    MS_U32      u32OutputBufUsed;
    VOID*       pUserPtr;
} MHAL_VENC_EncResult_t;
#define MHAL_VENC_EncResult_t_ver (0x01)

typedef struct MHAL_VENC_ResolutionAlign_s
{
    MHAL_VENC_Version_t stVerCtl;

    MS_U32      u32AlignW;
    MS_U32      u32AlignH;
    MS_U32      u32AlignCropX;
    MS_U32      u32AlignCropY;
    MS_U32      u32AlignCropW;
    MS_U32      u32AlignCropH;
} MHAL_VENC_ResolutionAlign_t;
#define MHAL_VENC_ResolutionAlign_t_ver (0x01)

typedef struct MHAL_VENC_ParamModH264e_s
{
    MHAL_VENC_DynamicCtl_t stRtlH264Ctl;
    MHAL_VENC_H264StaticCtl_t stStaH264Ctl;
} MHAL_VENC_ParamModH264e_t;

typedef struct MHAL_VENC_ParamModH265e_s
{
    MHAL_VENC_DynamicCtl_t stRtlH265Ctl;
    //VEncH265StaticCtl StaH265Ctl;   //Fixme
} MHAL_VENC_ParamModH265e_t;

typedef struct MHAL_VENC_ParamModJpege_s
{
    MHAL_VENC_DynamicCtl_t stRtJPGECtl;
    //VEncJPGEStaticCtl  StaJPGECtl;   //Fixme
} MHAL_VENC_ParamModJpege_t;

typedef struct MHAL_VENC_EncStatus_s
{
    MHAL_VENC_IrqStatus_e eEncSts;
    MS_U64     phyCurrInBuf;
    MS_U64     phyCurrOutBuf;
    MHAL_VENC_INST_HANDLE phInst;
} MHAL_VENC_EncStatus_t;

typedef struct MHAL_VENC_AddOutBuf_s
{
    MS_U64     phyOutBuff;
    MS_U32     u32OutBuffSize;
} MHAL_VENC_AddOutBuf_t;

#if 0//reserved as reference for previous design
typedef struct MHAL_VENC_ChnInfo_s
{
    MHAL_VENC_ModeType_e eVencModType;
    union
    {
        MHAL_VENC_ParamModH264e_t stH264eModParam;
        MHAL_VENC_ParamModH265e_t stH265eModParam;
        MHAL_VENC_ParamModJpege_t stJpegeModParam;
    };
} MHAL_VENC_ChnInfo_t;
#endif


//MHAL_ErrCode_e is not fully defined. It might use MHAL_DEF_ERR macro to define error for this module later.
/**
 *
 * @param pOsDev[in]
 * @param ppBase[out]
 * @param pSize[out]
 * @param phDev[out]
 * @return
 */
//MS_S32 MHAL_VENC_CreateDevice(VOID *pOsDev, VOID** ppBase, MS_S32 *pSize, MHAL_VENC_DEV_HANDLE *phDev);
//MS_S32 MHAL_VENC_DestroyDevice(MHAL_VENC_DEV_HANDLE hDev);

//==== MHAL device interfaces
/**
 *
 * @param hDev[in]
 * @param type[in]
 * @param param[out]
 * @return
 */
//MS_S32 MHAL_VENC_GetDevConfig(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_IDX type, MHAL_VENC_Param_t* param);
/**
 *
 * @param hDev[in]
 * @param phInst[out]
 * @return
 */
//MS_S32 MHAL_VENC_CreateInstance(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst);

//==== MHAL instance interfaces
//MS_S32 MHAL_VENC_DestroyInstance(MHAL_VENC_INST_HANDLE hInst);
/**
 *
 * @param hInst[in]
 * @param type[in]
 * @param param[out]
 * @return
 */
//MS_S32 MHAL_VENC_SetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* param);
/**
 *
 * @param hInst[in]
 * @param type[in]
 * @param param[out]
 * @return
 */
//MS_S32 MHAL_VENC_GetParam(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* param);

/**
 *
 * @param hInst[in]
 * @return
 */
//MS_S32 MHAL_VENC_EncodeOneFrame(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf);

/**
 *
 * @param hInst[in]
 * @param pEncRet[out]
 * @return
 */
//MS_S32 MHAL_VENC_EncDone(MHAL_VENC_INST_HANDLE hInst,MHAL_VENC_EncResult_t* pEncRet);

/**
 *
 * @param hInst[in]
 * @param pstSize[out] Note that only sizes are used. Other fields are kept untouched.
 * @return
 */
//MS_S32 MHAL_VENC_QueryBufSize(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstSize);

//function pointers of HAL driver
typedef struct MHAL_VENC_Drv_s
{
    MS_S32 (*Init)(void);
    MS_S32 (*CreateDevice)(MS_U32 u32CoreId, MHAL_VENC_DEV_HANDLE *phDev);
    MS_S32 (*DestroyDevice)(MHAL_VENC_DEV_HANDLE hDev);
    MS_S32 (*GetDevConfig)(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam);
    MS_S32 (*IsrProc)(MHAL_VENC_DEV_HANDLE hDev);
    MS_S32 (*CreateInstance)(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_INST_HANDLE *phInst);
    MS_S32 (*DestroyInstance)(MHAL_VENC_INST_HANDLE hInst);
    MS_S32 (*SetParam)(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* param);
    MS_S32 (*GetParam)(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_IDX type, MHAL_VENC_Param_t* param);
    MS_S32 (*EncodeOneFrame)(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf);
    MS_S32 (*EncodeDone)(MHAL_VENC_INST_HANDLE hInst,MHAL_VENC_EncResult_t* pEncRet);
    MS_S32 (*EncodeCancel)(MHAL_VENC_DEV_HANDLE hDev);
    MS_S32 (*EncodeAddOutBuff)(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_AddOutBuf_t *pstAddOutBuf);
    MS_S32 (*QueryBufSize)(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InternalBuf_t *pstSize);
    MS_S32 (*GetEngStatus)(MHAL_VENC_DEV_HANDLE hDev, MHAL_VENC_EncStatus_t *pstParam);
} MHAL_VENC_Drv_t;

#endif /* MI_VENC_MHAL_VENC_H_ */
