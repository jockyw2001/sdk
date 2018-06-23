#include "st_common.h"
#include "mi_venc.h"

// Insert User Data
MI_U8 _test_insert_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};

// ROI Config
MI_VENC_RoiCfg_t _test_roi_cfg =
{
    .u32Index = 0,
    .bEnable = TRUE,
    .bAbsQp = FALSE,
    .s32Qp = -12,
    .stRect = 
    {
        .u32Left = 32,
        .u32Top = 32,
        .u32Width = 256,
        .u32Height = 256,
    }
};

//IDR config
MI_BOOL g_idr_config = FALSE;
MI_BOOL _test_idr_enable = FALSE;
int _test_tmp_interval = 0, _test_idr_interval = 80;

//Slice Split
MI_VENC_ParamH265SliceSplit_t _test_split_265 =
{
    .bSplitEnable = TRUE,
    .u32SliceRowCount = 6,
};

MI_VENC_ParamH264SliceSplit_t _test_split_264 =
{
    .bSplitEnable = TRUE,
    .u32SliceRowCount = 6,
};

//Intra Pred
MI_VENC_ParamH265IntraPred_t _test_intra_pred_265 =
{
    .u32Intra32x32Penalty = 0,  //0~65535
    .u32Intra16x16Penalty = 0,  //0~65535
    .u32Intra8x8Penalty = 0,    //0~65535
};

MI_VENC_ParamH264IntraPred_t _test_intra_pred_264 =
{
    .bIntra16x16PredEn = TRUE,
    .bIntraNxNPredEn = TRUE,
    .constrained_intra_pred_flag = 0,
    .bIpcmEn = FALSE,
    .u32Intra16x16Penalty = 0,
    .u32Intra4x4Penalty = 0,
    .bIntraPlanarPenalty = FALSE,
};

//Inter Pred
MI_VENC_ParamH265InterPred_t _test_inter_pred_265 =
{
    .u32HWSize = 32,
    .u32VWSize = 16,
    .bInter16x16PredEn = TRUE,
    .bInter16x8PredEn = TRUE,
    .bInter8x16PredEn = TRUE,
    .bInter8x8PredEn = TRUE,
    .bExtedgeEn = FALSE,
};

MI_VENC_ParamH264InterPred_t _test_inter_pred_264 =
{
    .u32HWSize = 32,
    .u32VWSize = 16,
    .bInter16x16PredEn = TRUE,
    .bInter16x8PredEn = TRUE,
    .bInter8x16PredEn = TRUE,
    .bInter8x8PredEn = TRUE,
    .bExtedgeEn = FALSE,
};

//Trans
MI_VENC_ParamH265Trans_t _test_trans_265 =
{
    .u32IntraTransMode = 0,
    .u32InterTransMode = 0,
    .s32ChromaQpIndexOffset = 0,
};

MI_VENC_ParamH264Trans_t _test_trans_264 =
{
    .u32IntraTransMode = 0,
    .u32InterTransMode = 0,
    .s32ChromaQpIndexOffset = 0,
};

//Entropy
MI_VENC_ParamH264Entropy_t _test_entropy_264 =
{
    .u32EntropyEncModeI = 1,
    .u32EntropyEncModeP = 1,
};

//VUI
MI_VENC_ParamH264Vui_t _test_vui_264 =
{
    .stVuiAspectRatio =
    {
        .u8AspectRatioInfoPresentFlag = 0,
        .u8AspectRatioIdc = 1,
        .u8OverscanInfoPresentFlag = 0,
        .u8OverscanAppropriateFlag = 0,
        .u16SarWidth = 1,
        .u16SarHeight = 1,
    },
    .stVuiTimeInfo =
    {
        .u8TimingInfoPresentFlag = 0,
        .u8FixedFrameRateFlag = 1,
        .u32NumUnitsInTick = 1,
        .u32TimeScale = 60,
    },
    .stVuiVideoSignal =
    {
        .u8VideosignalTypePresentFlag = 1,
        .u8VideoFormat = 5,
        .u8VideoFullRangeFlag = 1,
        .u8ColourDescriptionPresentFlag = 1,
        .u8ColourPrimaries = 1,
        .u8TransferCharacteristics = 1,
        .u8MatrixCoefficients = 1,
    },
};

MI_VENC_ParamH265Vui_t _test_vui_265 =
{
    .stVuiAspectRatio =
    {
        //.u8AspectRatioInfoPresentFlag = 0,
        //.u8AspectRatioIdc = 1,
        //.u8OverscanInfoPresentFlag = 0,
        //.u8OverscanAppropriateFlag = 0,
        .u16SarWidth = 1,
        .u16SarHeight = 1,
    },
    .stVuiTimeInfo =
    {
        .u8TimingInfoPresentFlag = 0,
        //.u8FixedFrameRateFlag = 1,
        //.u32NumUnitsInTick = 1,
        //.u32TimeScale = 60,
    },
    .stVuiVideoSignal =
    {
        .u8VideosignalTypePresentFlag = 1,
        .u8VideoFormat = 5,
        .u8VideoFullRangeFlag = 1,
        .u8ColourDescriptionPresentFlag = 1,
        .u8ColourPrimaries = 1,
        .u8TransferCharacteristics = 1,
        .u8MatrixCoefficients = 1,
    },
};

//LTR
MI_VENC_ParamRef_t _test_ltr =
{
    .u32Base = 1,
    .u32Enhance = 49,
    .bEnablePred = FALSE,
};

//Crop
MI_VENC_CropCfg_t _test_crop =
{
    .bEnable = FALSE,
    .stRect =
    {
        .u32Left = 64,      //H264:256x, H265:32x
        .u32Top = 32,       //H264:32x, H265:2x
        .u32Width = 1024,   //H264:16x, H265:32x
        .u32Height =512,    //H264:16x, H265:32x
    }
};

//Deblocking
MI_VENC_ParamH265Dblk_t _test_dblk_265 =
{
    .disable_deblocking_filter_idc = 1,
    .slice_tc_offset_div2 = 0,
    .slice_beta_offset_div2 = 0,
};

MI_VENC_ParamH264Dblk_t _test_dblk_264 =
{
    .disable_deblocking_filter_idc = 1,
    .slice_alpha_c0_offset_div2 = 0,
    .slice_beta_offset_div2 = 0,
};

// Video Case
ST_CaseDesc_t g_stCaseDesc[] =
{
    {
        .stDesc =
        {
            .szDesc = "1x4K(3840x2160)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 3840,
                        .u16MainHeight = 2160,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x4K(3840x2160)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 3840,
                        .u16MainHeight = 2160,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x1080P@25 H264 + 1xD1@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x1080P(1920x1080)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x1080P(1920x1080)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "2x1080P(1920x1080)@25 H264 Encode",
            .u32VencNum = 2,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "4x1080P(1920x1080)@25 H264 Encode",
            .u32VencNum = 4,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "4x1080P(1920x1080)@25 H265 Encode",
            .u32VencNum = 4,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1080,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1xCIF(352x288)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 352,
                        .u16MainHeight = 288,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1xCIF(352x288)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 352,
                        .u16MainHeight = 288,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x720P(1280x720)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x720P(1280x720)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "4x720P(1280x720)@25 H264 Encode",
            .u32VencNum = 4,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "4x720P(1280x720)@25 H265 Encode",
            .u32VencNum = 4,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 1280,
                        .u16MainHeight = 720,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1xD1(736x576)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 15,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1xD1(720x576)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 15,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x960H(960x576)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x960H(960x576)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "9x960H(960x576)@25 H264 Encode",
            .u32VencNum = 9,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 4,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 5,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 6,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 7,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 8,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "9x960H(960x576)@25 H265 Encode",
            .u32VencNum = 9,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 4,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 5,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 6,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 7,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 8,
                        .u16MainWidth = 960,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "16xD1(736x576)@25 H264 Encode",
            .u32VencNum = 16,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 4,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 5,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 6,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 7,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 8,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 9,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 10,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 11,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 12,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 13,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 14,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 15,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "16xD1(720x576)@25 H265 Encode",
            .u32VencNum = 16,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 4,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 5,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 6,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 7,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 8,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 9,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 10,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 11,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 12,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 13,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 14,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 15,
                        .u16MainWidth = 720,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "16xD1(736x576)@25 H264 + 16xD1(736x576)@25 H265 Encode",
            .u32VencNum = 32,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 4,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 5,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 6,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 7,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 8,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 9,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 10,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 11,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 12,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 13,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 14,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 15,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 16,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 17,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 18,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 19,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 20,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 21,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 22,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 23,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 24,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 25,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 26,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 27,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 28,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 29,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 30,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 31,
                        .u16MainWidth = 736,
                        .u16MainHeight = 576,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 25,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x4MP(2560x1440)@25 H265 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H265E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x4MP(2560x1440)@25 H264 Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "4x4MP(2560x1440)@25 H264 Encode",
            .u32VencNum = 4,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 1,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 2,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 3,
                        .u16MainWidth = 2560,
                        .u16MainHeight = 1440,
                        .eType = E_MI_VENC_MODTYPE_H264E,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            }
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1x4K(3840x2160)@25 JPEG Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 3840,
                        .u16MainHeight = 2160,
                        .eType = E_MI_VENC_MODTYPE_JPEGE,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
    {
        .stDesc =
        {
            .szDesc = "1xFHD(1920x1088)@25 JPEG Encode",
            .u32VencNum = 1,
        },
        .stCaseArgs =
        {
            {
                .eVideoChnType = E_ST_VIF_VENC_CHN,
                .uChnArg =
                {
                    .stVencChnArg =
                    {
                        .u32Chn = 0,
                        .u16MainWidth = 1920,
                        .u16MainHeight = 1088,
                        .eType = E_MI_VENC_MODTYPE_JPEGE,
                        .u16SubWidth = 720,
                        .u16SubHeight = 576,
                        .s32MainBitRate = 100,
                        .s32SubBitRate = 100,
                        .s32MainFrmRate = 30,
                        .s32SubFrmRate = 25
                    },
                }
            },
        },
    },
};
