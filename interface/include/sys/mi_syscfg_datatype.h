#ifndef _MI_SYSCFG_DATATYPE_H_
#define _MI_SYSCFG_DATATYPE_H_

#include "mi_common_datatype.h"

typedef struct MI_SYSCFG_MmapInfo_s
{
    MI_U8     u8Gid;                         // Mmap ID
    MI_U8     u8Layer;                       // Memory Layer
    MI_U8     u8MiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
    MI_U8     u8CMAHid;                      // Memory CMAHID
    MI_U32    u32Addr;                       // Memory Address
    MI_U32    u32Size;                       // Memory Size
    MI_U32    u32Align;                      // Memory Align
    MI_U32    u32MemoryType;                 // Memory Type
} MI_SYSCFG_MmapInfo_t;

typedef struct MI_SYSCFG_MemoryInfo_s
{
    MI_U32    u32TotalSize;
    MI_U32    u32Miu0Size;
    MI_U32    u32Miu1Size;
    MI_U32    u32MiuBoundary;
    MI_U32    u32MmapItemsNum;
    MI_BOOL   bIs4kAlign;
    MI_BOOL   bMiu1Enable;
} MI_SYSCFG_MemoryInfo_t;

#define PANEL_MAX_NUM 100
#define PANEL_INFO_VALIDATE_NUM 100
#define AUDIO_INPUT_MAX_NUM 30
#define AUDIO_PATH_MAX_NUM 30
#define AUDIO_OUTPUT_MAX_NUM 30
#define HDMITX_ANALOG_INFO_MAX_NUM 30
#define GAMMA_TABLE_MAX_NUM 10
#define NAME_MAX_NUM 256
#define PATH_MAX_NUM 256
#define WINDOW_TYPE_NUM 12
#define WINDOW_TIMMING_NUM 0x40
#define MAXRESOLUTIONSIZE (30)
#define DLCTABLE_MAX_NUM 10
#define MAX_XC_CLOCK                            (2200UL * 1125UL * 60UL)//Max xc clock
typedef struct
{
    int nResolutionSize;

} ResolutionInfoSize;

//CURRENT MODE
typedef enum
{
    E_CURRENT_MODE_FULL,
    E_CURRENT_MODE_HALF,
    E_CURRENT_MODE_QUART,

    E_CURRENT_MODE_NUM,
    E_CURRENT_MODE_NONE = E_CURRENT_MODE_NUM,
} EN_CURRENT_MODE_TYPE;

/// the PQ window
typedef enum
{
    /// main window
    MAPI_PQ_MAIN_WINDOW = 0,
    /// sub window
    MAPI_PQ_SUB_WINDOW = 1,
    /// counts of this enum
    MAPI_PQ_MAX_WINDOW,
} MAPI_PQ_WIN;
    /// the display color when screen mute
    typedef enum
    {
        /// black
        E_SCREEN_MUTE_BLACK = 0,
        /// white
        E_SCREEN_MUTE_WHITE,
        /// red
        E_SCREEN_MUTE_RED,
        /// blue
        E_SCREEN_MUTE_BLUE,
        /// green
        E_SCREEN_MUTE_GREEN,
        /// customer color, get from customer ini
        E_SCREEN_MUTE_CUSTOMER,
        /// counts of this enum
        E_SCREEN_MUTE_NUMBER,
    } MAPI_VIDEO_Screen_Mute_Color;


    typedef enum
    {
        /// Default
        E_AR_DEFAULT = 0,
        /// 16x9
        E_AR_16x9,
        /// 4x3
        E_AR_4x3,
        /// Auto
        E_AR_AUTO,
        /// Panorama
        E_AR_Panorama,
        /// Just Scan
        E_AR_JustScan,
        /// Zoom 1
        E_AR_Zoom1,
        /// Zoom 2
        E_AR_Zoom2,
        E_AR_14x9,
        /// point to point
        E_AR_PqBypass,
         /// Subtitle
        E_AR_Subtitle,
        /// movie
        E_AR_Movie,
        /// Personal
        E_AR_Personal,
        /// 4x3 Panorama
        E_AR_4x3_PanScan,
        /// 4x3 Letter Box
        E_AR_4x3_LetterBox,
        /// 16x9 PillarBox
        E_AR_16x9_PillarBox,
        /// 16x9 PanScan
        E_AR_16x9_PanScan,
        /// 4x3 Combind
        E_AR_4x3_Combind,
        /// 16x9  Combind
        E_AR_16x9_Combind,
        /// Zoom 2X
        E_AR_Zoom_2x,
        /// Zoom 3X
        E_AR_Zoom_3x,
        /// Zoom 4X
        E_AR_Zoom_4x,
        /// In front of E_AR_CUS is Supernova area and the customization area at the back of E_AR_CUS.
        E_AR_CUS =0x20,
        /// Maximum value of this enum
        E_AR_MAX=0x40,
    } MAPI_VIDEO_ARC_Type;


/// the input source type
typedef enum
{
    MAPI_INPUT_SOURCE_VGA,          ///<VGA input     0
    MAPI_INPUT_SOURCE_ATV,          ///<TV input      1

    MAPI_INPUT_SOURCE_CVBS,         ///<AV 1           2
    MAPI_INPUT_SOURCE_CVBS2,        ///<AV 2           3
    MAPI_INPUT_SOURCE_CVBS3,        ///<AV 3           4
    MAPI_INPUT_SOURCE_CVBS4,        ///<AV 4           5
    MAPI_INPUT_SOURCE_CVBS5,        ///<AV 5           6
    MAPI_INPUT_SOURCE_CVBS6,        ///<AV 6           7
    MAPI_INPUT_SOURCE_CVBS7,        ///<AV 7           8
    MAPI_INPUT_SOURCE_CVBS8,        ///<AV 8           9
    MAPI_INPUT_SOURCE_CVBS_MAX,     ///<AV max         10

    MAPI_INPUT_SOURCE_SVIDEO,       ///<S-video 1      11
    MAPI_INPUT_SOURCE_SVIDEO2,      ///<S-video 2      12
    MAPI_INPUT_SOURCE_SVIDEO3,      ///<S-video 3      13
    MAPI_INPUT_SOURCE_SVIDEO4,      ///<S-video 4      14
    MAPI_INPUT_SOURCE_SVIDEO_MAX,   ///<S-video max    15

    MAPI_INPUT_SOURCE_YPBPR,        ///<Component 1    16
    MAPI_INPUT_SOURCE_YPBPR2,       ///<Component 2    17
    MAPI_INPUT_SOURCE_YPBPR3,       ///<Component 3    18
    MAPI_INPUT_SOURCE_YPBPR_MAX,    ///<Component max   19

    MAPI_INPUT_SOURCE_SCART,        ///<Scart 1         20
    MAPI_INPUT_SOURCE_SCART2,       ///<Scart 2         21
    MAPI_INPUT_SOURCE_SCART_MAX,    ///<Scart max       22

    MAPI_INPUT_SOURCE_HDMI,         ///<HDMI 1          23
    MAPI_INPUT_SOURCE_HDMI2,        ///<HDMI 2          24
    MAPI_INPUT_SOURCE_HDMI3,        ///<HDMI 3          25
    MAPI_INPUT_SOURCE_HDMI4,        ///<HDMI 4          26
    MAPI_INPUT_SOURCE_HDMI_MAX,     ///<HDMI max         27

    MAPI_INPUT_SOURCE_DTV,          ///<DTV              28

    MAPI_INPUT_SOURCE_DVI,          ///<DVI 1            29
    MAPI_INPUT_SOURCE_DVI2,         ///<DVI 2            30
    MAPI_INPUT_SOURCE_DVI3,         ///<DVI 2            31
    MAPI_INPUT_SOURCE_DVI4,         ///<DVI 4            32
    MAPI_INPUT_SOURCE_DVI_MAX,      ///<DVI max          33

    // Application source
    MAPI_INPUT_SOURCE_STORAGE,  ///<Storage              34
    MAPI_INPUT_SOURCE_KTV,      ///<KTV                  35
    MAPI_INPUT_SOURCE_JPEG,     ///<JPEG                 36
    MAPI_INPUT_SOURCE_DTV2,      ///<DTV2                37
    MAPI_INPUT_SOURCE_STORAGE2,     ///<Storage2          38
    MAPI_INPUT_SOURCE_DTV3,      ///<DTV3                 39
	MAPI_INPUT_SOURCE_SCALER_OP, ///< video from op        40
    MAPI_INPUT_SOURCE_RVU,          ///<RVU            41
    MAPI_INPUT_SOURCE_NUM,      ///<number of the source   42
    MAPI_INPUT_SOURCE_NONE = MAPI_INPUT_SOURCE_NUM,    ///<NULL input
} MAPI_INPUT_SOURCE_TYPE;

/// the input source table
typedef struct _MAPI_VIDEO_INPUTSRCTABLE
{
    MI_U32 u32EnablePort;
    MI_U32 u32Port[2];
} MAPI_VIDEO_INPUTSRCTABLE;

///VD capture window mode
typedef enum
{
    EN_Mode_1135,
    EN_Mode_1135_1P5,
    EN_Mode_Dynamic,
} EN_VD_CAPTURE_WINDOW_MODE;

///////////////////////////////////////////////////////////////////////////////
//audio input  mux
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    MI_U32 u32Port;
} AudioMux_t;

typedef struct
{
    MI_U32 u32Path;
} AudioPath_t;

typedef struct
{
    MI_U32 u32Output;
} AudioOutputType_t;

/// the audio input source
typedef enum
{
    MAPI_AUDIO_SOURCE_DTV = 0,
    MAPI_AUDIO_SOURCE_DTV2,
    MAPI_AUDIO_SOURCE_DTV_MAX = MAPI_AUDIO_SOURCE_DTV2,

    MAPI_AUDIO_SOURCE_ATV,
    MAPI_AUDIO_SOURCE_PC,

    MAPI_AUDIO_SOURCE_YPBPR,
    MAPI_AUDIO_SOURCE_YPBPR2,
    MAPI_AUDIO_SOURCE_YPBPR_MAX = MAPI_AUDIO_SOURCE_YPBPR2,

    MAPI_AUDIO_SOURCE_AV,
    MAPI_AUDIO_SOURCE_AV2,
    MAPI_AUDIO_SOURCE_AV3,
    MAPI_AUDIO_SOURCE_AV_MAX = MAPI_AUDIO_SOURCE_AV3,

    MAPI_AUDIO_SOURCE_SV,
    MAPI_AUDIO_SOURCE_SV2,
    MAPI_AUDIO_SOURCE_SV_MAX = MAPI_AUDIO_SOURCE_SV2,

    MAPI_AUDIO_SOURCE_SCART,
    MAPI_AUDIO_SOURCE_SCART2,
    MAPI_AUDIO_SOURCE_SCART_MAX = MAPI_AUDIO_SOURCE_SCART2,

    MAPI_AUDIO_SOURCE_HDMI,
    MAPI_AUDIO_SOURCE_HDMI2,
    MAPI_AUDIO_SOURCE_HDMI3,
    MAPI_AUDIO_SOURCE_HDMI_MAX = MAPI_AUDIO_SOURCE_HDMI3,

    MAPI_AUDIO_SOURCE_DVI,
    MAPI_AUDIO_SOURCE_DVI2,
    MAPI_AUDIO_SOURCE_DVI3,
    MAPI_AUDIO_SOURCE_DVI_MAX = MAPI_AUDIO_SOURCE_DVI3,

    MAPI_AUDIO_SOURCE_KTV,

    MAPI_AUDIO_SOURC_NUM,
    MAPI_AUDIO_SOURCE_NONE = MAPI_AUDIO_SOURC_NUM,

} MAPI_AUDIO_INPUT_SOURCE_TYPE;

/// the audio path
typedef enum
{
    MAPI_AUDIO_PATH_MAIN_SPEAKER = 0,
    MAPI_AUDIO_PATH_HP,
    MAPI_AUDIO_PATH_LINEOUT,
    MAPI_AUDIO_PATH_SIFOUT,

    MAPI_AUDIO_PATH_SCART1,
    MAPI_AUDIO_PATH_SCART2,
    MAPI_AUDIO_PATH_SCART_MAX = MAPI_AUDIO_PATH_SCART2,

    MAPI_AUDIO_PATH_SPDIF,
    MAPI_AUDIO_PATH_HDMI,
    MAPI_AUDIO_PATH_NUM,
    MAPI_AUDIO_PATH_NONE = MAPI_AUDIO_PATH_NUM,
} MAPI_AUDIO_PATH_TYPE;

/// the audio output type
typedef enum
{
    MAPI_AUDIO_OUTPUT_MAIN_SPEAKER,
    MAPI_AUDIO_OUTPUT_HP,
    MAPI_AUDIO_OUTPUT_LINEOUT,
    MAPI_AUDIO_OUTPUT_SIFOUT,

    MAPI_AUDIO_OUTPUT_SCART1,
    MAPI_AUDIO_OUTPUT_SCART2,
    MAPI_AUDIO_OUTPUT_SCART_MAX = MAPI_AUDIO_OUTPUT_SCART2,

    MAPI_AUDIO_OUTPUT_NUM,
    MAPI_AUDIO_OUTPUT_NONE = MAPI_AUDIO_OUTPUT_NUM,
} MAPI_AUDIO_OUTPUT_TYPE;


typedef struct
{
    MAPI_AUDIO_INPUT_SOURCE_TYPE eAudioSrc;
    MAPI_AUDIO_PATH_TYPE eAudioPath;
    MAPI_AUDIO_OUTPUT_TYPE eAudioOutput;
} AudioDefualtInit_t;

///////////////////////////////////////////////////////////////////////////////
// HDMI info
///////////////////////////////////////////////////////////////////////////////
typedef struct HDMITx_Analog_Param_s
{
    // HDMI Tx Current, Pre-emphasis and Double termination
    MI_U8    tm_txcurrent; // TX current control, (U4: 0x11302B[13:12], K1: 0x11302B[13:11], K2: 0x11302B[13:12])
    MI_U8    tm_pren2; // pre-emphasis mode control, 0x11302D[5]
    MI_U8    tm_precon; // TM_PRECON, 0x11302E[7:4]
    MI_U8    tm_pren; // pre-emphasis enable, 0x11302E[11:8]
    MI_U8    tm_tenpre; // Double termination pre-emphasis enable, 0x11302F[3:0]
    MI_U8    tm_ten; // Double termination enable, 0x11302F[7:4]

} HDMITx_Analog_Param_t;

/// to define the structure for the video window info
typedef struct
{
    MI_U16 u16H_CapStart;  ///< Capture window H start
    MI_U16 u16V_CapStart;  ///< Capture window V start

    MI_U8 u8HCrop_Left;    ///< H Crop Left
    MI_U8 u8HCrop_Right;   ///< H crop Right
    MI_U8 u8VCrop_Up;      ///< V Crop Up
    MI_U8 u8VCrop_Down;    ///< V Crop Down
} ST_MAPI_VIDEO_WINDOW_INFO;

#define DLC_HISTOGRAM_LIMIT_CURVE_ARRARY_NUM    17   //the ucDlcHistogramLimitCurve arrary num
//----------------------------
// XC DLC initialize
//----------------------------
/*!
 *  Initial Settings of MF Dyanmic Luma Curve
 */
typedef struct
{
    /// Default luma curve
    MI_U8 ucLumaCurve[16];
    /// Default luma curve 2a
    MI_U8 ucLumaCurve2_a[16];
    /// Default luma curve 2b
    MI_U8 ucLumaCurve2_b[16];
    /// Default luma curve 2
    MI_U8 ucLumaCurve2[16];

    /// default value: 10
    MI_U8 u8_L_L_U;
    /// default value: 10
    MI_U8 u8_L_L_D;
    /// default value: 10
    MI_U8 u8_L_H_U;
    /// default value: 10
    MI_U8 u8_L_H_D;
    /// default value: 128 (0x80)
    MI_U8 u8_S_L_U;
    /// default value: 128 (0x80)
    MI_U8 u8_S_L_D;
    /// default value: 128 (0x80)
    MI_U8 u8_S_H_U;
    /// default value: 128 (0x80)
    MI_U8 u8_S_H_D;
    /// -31 ~ 31 (bit7 = minus, ex. 0x88 => -8)
    MI_U8 ucCGCCGain_offset;
    /// 0x00~0x6F
    MI_U8 ucCGCChroma_GainLimitH;
    /// 0x00~0x10
    MI_U8 ucCGCChroma_GainLimitL;
    /// 0x01~0x20
    MI_U8 ucCGCYCslope;
    /// 0x01
    MI_U8 ucCGCYth;
    /// Compare difference of max and min bright
    MI_U8 ucDlcPureImageMode;
    /// n = 0 ~ 4 => Limit n levels => ex. n=2, limit 2 level 0xF7, 0xE7
    MI_U8 ucDlcLevelLimit;
    /// n = 0 ~ 50, default value: 12
    MI_U8 ucDlcAvgDelta;
    /// n = 0 ~ 15 => 0: disable still curve, 1 ~ 15: enable still curve
    MI_U8 ucDlcAvgDeltaStill;
    /// min 17 ~ max 32
    MI_U8 ucDlcFastAlphaBlending;
    /// some event is triggered, DLC must do slowly // for PIP On/Off, msMultiPic.c
    MI_U8 ucDlcSlowEvent;
    /// for IsrApp.c
    MI_U8 ucDlcTimeOut;
    /// for force to do fast DLC in a while
    MI_U8 ucDlcFlickAlphaStart;
    /// default value: 128
    MI_U8 ucDlcYAvgThresholdH;
    /// default value: 0
    MI_U8 ucDlcYAvgThresholdL;
    /// n = 24 ~ 64, default value: 48
    MI_U8 ucDlcBLEPoint;
    /// n = 24 ~ 64, default value: 48
    MI_U8 ucDlcWLEPoint;
    /// 1: enable; 0: disable
    MI_U8 bCGCCGainCtrl : 1;
    /// 1: enable; 0: disable
    MI_U8 bEnableBLE : 1;
    /// 1: enable; 0: disable
    MI_U8 bEnableWLE : 1;
} MAPI_XC_DLC_MFinit;

/*!
 *  Initial Settings of MF Dyanmic Luma Curve Ex
 */
typedef struct
{
    MI_U32 u32DLC_MFinit_Ex_Version;   ///<Version of current structure. Please always set to "DLC_MFINIT_EX_VERSION" as input
    /// Default luma curve
    MI_U8 ucLumaCurve[16];
    /// Default luma curve 2a
    MI_U8 ucLumaCurve2_a[16];
    /// Default luma curve 2b
    MI_U8 ucLumaCurve2_b[16];
    /// Default luma curve 2
    MI_U8 ucLumaCurve2[16];

    /// default value: 10
    MI_U8 u8_L_L_U;
    /// default value: 10
    MI_U8 u8_L_L_D;
    /// default value: 10
    MI_U8 u8_L_H_U;
    /// default value: 10
    MI_U8 u8_L_H_D;
    /// default value: 128 (0x80)
    MI_U8 u8_S_L_U;
    /// default value: 128 (0x80)
    MI_U8 u8_S_L_D;
    /// default value: 128 (0x80)
    MI_U8 u8_S_H_U;
    /// default value: 128 (0x80)
    MI_U8 u8_S_H_D;
    /// -31 ~ 31 (bit7 = minus, ex. 0x88 => -8)
    MI_U8 ucCGCCGain_offset;
    /// 0x00~0x6F
    MI_U8 ucCGCChroma_GainLimitH;
    /// 0x00~0x10
    MI_U8 ucCGCChroma_GainLimitL;
    /// 0x01~0x20
    MI_U8 ucCGCYCslope;
    /// 0x01
    MI_U8 ucCGCYth;
    /// Compare difference of max and min bright
    MI_U8 ucDlcPureImageMode;
    /// n = 0 ~ 4 => Limit n levels => ex. n=2, limit 2 level 0xF7, 0xE7
    MI_U8 ucDlcLevelLimit;
    /// n = 0 ~ 50, default value: 12
    MI_U8 ucDlcAvgDelta;
    /// n = 0 ~ 15 => 0: disable still curve, 1 ~ 15: enable still curve
    MI_U8 ucDlcAvgDeltaStill;
    /// min 17 ~ max 32
    MI_U8 ucDlcFastAlphaBlending;
    /// some event is triggered, DLC must do slowly // for PIP On/Off, msMultiPic.c
    MI_U8 ucDlcSlowEvent;
    /// for IsrApp.c
    MI_U8 ucDlcTimeOut;
    /// for force to do fast DLC in a while
    MI_U8 ucDlcFlickAlphaStart;
    /// default value: 128
    MI_U8 ucDlcYAvgThresholdH;
    /// default value: 0
    MI_U8 ucDlcYAvgThresholdL;
    /// n = 24 ~ 64, default value: 48
    MI_U8 ucDlcBLEPoint;
    /// n = 24 ~ 64, default value: 48
    MI_U8 ucDlcWLEPoint;
    /// 1: enable; 0: disable
    MI_U8 bCGCCGainCtrl : 1;
    /// 1: enable; 0: disable
    MI_U8 bEnableBLE : 1;
    /// 1: enable; 0: disable
    MI_U8 bEnableWLE : 1;
    ///
    MI_U8 ucDlcYAvgThresholdM;
    /// Compare difference of max and min bright
    MI_U8 ucDlcCurveMode;
    /// min 00 ~ max 128
    MI_U8 ucDlcCurveModeMixAlpha;
    ///
    MI_U8 ucDlcAlgorithmMode;
    /// Dlc Histogram Limit Curve
    MI_U8 ucDlcHistogramLimitCurve[DLC_HISTOGRAM_LIMIT_CURVE_ARRARY_NUM];
    ///
    MI_U8 ucDlcSepPointH;
    ///
    MI_U8 ucDlcSepPointL;
    ///
    MI_U16 uwDlcBleStartPointTH;
    ///
    MI_U16 uwDlcBleEndPointTH;
    ///
    MI_U8 ucDlcCurveDiff_L_TH;
    ///
    MI_U8 ucDlcCurveDiff_H_TH;
    ///
    MI_U16 uwDlcBLESlopPoint_1;
    ///
    MI_U16 uwDlcBLESlopPoint_2;
    ///
    MI_U16 uwDlcBLESlopPoint_3;
    ///
    MI_U16 uwDlcBLESlopPoint_4;
    ///
    MI_U16 uwDlcBLESlopPoint_5;
    ///
    MI_U16 uwDlcDark_BLE_Slop_Min;
    ///
    MI_U8 ucDlcCurveDiffCoringTH;
    ///
    MI_U8 ucDlcAlphaBlendingMin;
    ///
    MI_U8 ucDlcAlphaBlendingMax;
    ///
    MI_U8 ucDlcFlicker_alpha;
    ///
    MI_U8 ucDlcYAVG_L_TH;
    ///
    MI_U8 ucDlcYAVG_H_TH;
    ///
    MI_U8 ucDlcDiffBase_L;
    ///
    MI_U8 ucDlcDiffBase_M;
    ///
    MI_U8 ucDlcDiffBase_H;
#if defined(UFO_XC_HDR_VERSION) && (UFO_XC_HDR_VERSION == 2)
    /// Left Max Threshold
    MI_U8 u8LMaxThreshold;
    /// Left Min Threshold
    MI_U8 u8LMinThreshold;
    /// Left Max Correction
    MI_U8 u8LMaxCorrection;
    /// Left Min Correction
    MI_U8 u8LMinCorrection;
    /// Right Max Threshold
    MI_U8 u8RMaxThreshold;
    /// Right Min Threshold
    MI_U8 u8RMinThreshold;
    /// Right Max Correction
    MI_U8 u8RMaxCorrection;
    /// Right Min Correction
    MI_U8 u8RMinCorrection;
    /// Allow Lose Contrast
    MI_U8 u8AllowLoseContrast;
#endif
} MAPI_XC_DLC_MFinit_Ex;

/*!
 *  Initial Settings of Dynamic Backlight Control
 */
typedef struct
{
    /// Max PWM
    MI_U8 ucMaxPWM;
    /// Min PWM
    MI_U8 ucMinPWM;
    /// Max Video
    MI_U8 ucMax_Video;
    /// Mid Video
    MI_U8 ucMid_Video;
    /// Min Video
    MI_U8 ucMin_Video;
    /// Current PWM
    MI_U8 ucCurrentPWM;
    /// Alpha
    MI_U8 ucAlpha;
    /// Backlight thres
    MI_U8 ucBackLight_Thres;
    /// Avg delta
    MI_U8 ucAvgDelta;
    /// Flick alpha
    MI_U8 ucFlickAlpha;
    /// Fast alpha blending, min 17 ~ max 32
    MI_U8 ucFastAlphaBlending;
    // TBD
    MI_U8 ucLoop_Dly;
    // TBD
    MI_U8 ucLoop_Dly_H_Init;
    // TBD
    MI_U8 ucLoop_Dly_MH_Init;
    // TBD
    MI_U8 ucLoop_Dly_ML_Init;
    // TBD
    MI_U8 ucLoop_Dly_L_Init;
    /// Y gain H
    MI_U8 ucY_Gain_H;
    /// C gain H
    MI_U8 ucC_Gain_H;
    /// Y gain M
    MI_U8 ucY_Gain_M;
    /// C gain M
    MI_U8 ucC_Gain_M;
    /// Y gain L
    MI_U8 ucY_Gain_L;
    /// C gain L
    MI_U8 ucC_Gain_L;
    /// 1: enable; 0: disable
    MI_U8 bYGainCtrl : 1;
    /// 1: enable; 0: disable
    MI_U8 bCGainCtrl : 1;
}MAPI_XC_DLC_DBC_MFinit;

typedef struct
{
    MI_S16 s16SDYuv2Rgb[9];
    MI_S16 s16HDYuv2Rgb[9];
    MI_S16 s16Default[32];
    MI_S16 s16HDTV[32];
    MI_S16 s16SDTV[32];
    MI_S16 s16ATV[32];
    MI_S16 s16SdYPbPr[32];
    MI_S16 s16HdYPbPr[32];
    MI_S16 s16HdHdmi[32];
    MI_S16 s16SdHdmi[32];
    MI_S16 s16AV[32];
    MI_S16 s16SV[32];
    MI_S16 s16Vga[32];
    MI_S16 s16DAC[32];
}MAPI_COLOR_MATRIX;

typedef enum VideoInfo_s
{
    E_DTV = 0,
    E_HDMI,
    E_YPbPr,
    E_CVBS,
    E_RVU,
    E_ATV,
    E_SCART,
    E_VIDEOINFO_ID_MAX,
} VideoInfo_t;
#define MAPI_ENABLE_10_BIT_DLC   0
/*!
 *  Initial Settings of Dyanmic Luma Curve
 */
typedef struct
{
    /// Scaler DCL MF init
    MAPI_XC_DLC_MFinit DLC_MFinit;
    /// Scaler DCL MF init Ex
    MAPI_XC_DLC_MFinit_Ex DLC_MFinit_Ex;
    /// Curve Horizontal start
    MI_U16 u16CurveHStart;
    /// Curve Horizontal end
    MI_U16 u16CurveHEnd;
    /// Curve Vertical start
    MI_U16 u16CurveVStart;
    /// Curve Vertical end
    MI_U16 u16CurveVEnd;
    /// Scaler DLC MF init
    MAPI_XC_DLC_DBC_MFinit DLC_DBC_MFinit;
#if (MAPI_ENABLE_10_BIT_DLC)
    /// DLC init ext
    MI_BOOL b10BitsEn;
#endif
#if defined(UFO_XC_HDR_VERSION) && (UFO_XC_HDR_VERSION == 2)
    MI_U8 u8DlcMode;
    MI_U8 u8TmoMode;
#endif

} MAPI_XC_DLC_init;

/// Picture Mode Curve
typedef struct
{
    /// Contrast Curve
    MI_U8 u8ContrastCurve[101]; //101 bytes
    /// Brightness Curve
    MI_U8 u8BrightnessCurve[101]; //101 bytes
    /// Saturation Curve
    MI_U8 u8SaturationCurve[101]; //101 bytes
    /// Sharpness Curve
    MI_U8 u8SharpnessCurve[101]; //101 bytes
    /// Hue Curve
    MI_U8 u8HueCurve[101]; //101 bytes
} PictureModeCurve_t;

///////////////////////////////////////////////////////////////////////////////
//input src mux
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
    int nSize;
} MuxSize;



// Gamma Table info
#define GammaArrayMAXSize 386

typedef struct
{
    MI_U8 nGammaTableSize;
} GammaTableSize_t;

typedef struct
{
    MI_U8 NormalGammaR[GammaArrayMAXSize];
    MI_U8 NormalGammaG[GammaArrayMAXSize];
    MI_U8 NormalGammaB[GammaArrayMAXSize];
} GAMMA_TABLE_t;

/// Define PANEL Signaling Type
typedef enum
{
    E_MAPI_LINK_TTL,                              ///< TTL  type
    E_MAPI_LINK_LVDS,                             ///< LVDS type
    E_MAPI_LINK_RSDS,                             ///< RSDS type
    E_MAPI_LINK_MINILVDS,                         ///< TCON
    E_MAPI_LINK_ANALOG_MINILVDS,                  ///< Analog TCON
    E_MAPI_LINK_DIGITAL_MINILVDS,                 ///< Digital TCON
    E_MAPI_LINK_MFC,                              ///< Ursa (TTL output to Ursa)
    E_MAPI_LINK_DAC_I,                            ///< DAC output
    E_MAPI_LINK_DAC_P,                            ///< DAC output
    E_MAPI_LINK_PDPLVDS,                          ///< For PDP(Vsync use Manually MODE)
    E_MAPI_LINK_EXT,                              /// EXT LPLL TYPE
}MAPI_APIPNL_LINK_TYPE;

/// Define aspect ratio
typedef enum
{
    E_MAPI_PNL_ASPECT_RATIO_4_3    = 0,         ///< set aspect ratio to 4 : 3
    E_MAPI_PNL_ASPECT_RATIO_WIDE,               ///< set aspect ratio to 16 : 9
    E_MAPI_PNL_ASPECT_RATIO_OTHER,              ///< resvered for other aspect ratio other than 4:3/ 16:9
}MAPI_PNL_ASPECT_RATIO;

/// Define TI bit mode
typedef enum
{
    E_MAPI_TI_10BIT_MODE = 0,
    E_MAPI_TI_8BIT_MODE = 2,
    E_MAPI_TI_6BIT_MODE = 3,
} MAPI_APIPNL_TIBITMODE;

/// Define panel output format bit mode
typedef enum
{
    E_MAPI_OUTPUT_10BIT_MODE = 0,//default is 10bit, becasue 8bit panel can use 10bit config and 8bit config.
    E_MAPI_OUTPUT_6BIT_MODE = 1, //but 10bit panel(like PDP panel) can only use 10bit config.
    E_MAPI_OUTPUT_8BIT_MODE = 2, //and some PDA panel is 6bit.
} MAPI_APIPNL_OUTPUTFORMAT_BITMODE;


/// Define which panel output timing change mode is used to change VFreq for same panel
typedef enum
{
    E_MAPI_PNL_CHG_DCLK   = 0,      ///<change output DClk to change Vfreq.
    E_MAPI_PNL_CHG_HTOTAL = 1,      ///<change H total to change Vfreq.
    E_MAPI_PNL_CHG_VTOTAL = 2,      ///<change V total to change Vfreq.
} MAPI_APIPNL_OUT_TIMING_MODE;



#if 0
//temp solution : use this for change paneltype information to packed format
typedef struct
{
    char pPanelName[128];                      ///<  PanelName
    //
    //  Panel output
    //
    MI_U8 bPanelDither : 1;                ///<  PANEL_DITHER
    MAPI_APIPNL_LINK_TYPE ePanelLinkType   : 4; ///<  define PANEL_LINK

    MI_U8 bPanelDualPort  : 1;             ///<  define PANEL_DUAL_PORT
    MI_U8 bPanelSwapPort  : 1;             ///<  define PANEL_SWAP_PORT
    MI_U8 bPanelSwapOdd_ML    : 1;         ///<  define PANEL_SWAP_ODD_ML
    MI_U8 bPanelSwapEven_ML   : 1;         ///<  define PANEL_SWAP_EVEN_ML
    MI_U8 bPanelSwapOdd_RB    : 1;         ///<  define PANEL_SWAP_ODD_RB
    MI_U8 bPanelSwapEven_RB   : 1;         ///<  define PANEL_SWAP_EVEN_RB

    MI_U8 bPanelSwapLVDS_POL  : 1;         //  #define PANEL_SWAP_LVDS_POL   0
    MI_U8 bPanelSwapLVDS_CH   : 1;         //  #define PANEL_SWAP_LVDS_CH    0
    MI_U8 bPanelPDP10BIT      : 1;         //  #define PANEL_PDP_10BIT       0
    MI_U8 bPanelLVDS_TI_MODE  : 1;         //  #define PANEL_LVDS_TI_MODE    _PNL_FUNC_EN_

    MI_U8 ucPanelDCLKDelay;                //  #define PANEL_DCLK_DELAY      0x00
    MI_U8 bPanelInvDCLK   : 1;             //  #define PANEL_INV_DCLK        0
    MI_U8 bPanelInvDE : 1;                 //  #define PANEL_INV_DE          0
    MI_U8 bPanelInvHSync  : 1;             //  #define PANEL_INV_HSYNC       0
    MI_U8 bPanelInvVSync  : 1;             //  #define PANEL_INV_VSYNC       0

    ///////////////////////////////////////////////
    // Output tmming setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    MI_U8 ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    MI_U8 ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    MI_U8 ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    MI_U8 ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    MI_U16 wPanelOnTiming1;                ///<  time between panel & data while turn on power
    MI_U16 wPanelOnTiming2;                ///<  time between data & back light while turn on power
    MI_U16 wPanelOffTiming1;               ///<  time between back light & data while turn off power
    MI_U16 wPanelOffTiming2;               ///<  time between data & panel while turn off power

    MI_U8 ucPanelHSyncWidth;               ///<  define PANEL_HSYNC_WIDTH
    MI_U8 ucPanelHSyncBackPorch;           ///<  define PANEL_HSYNC_BACK_PORCH

    MI_U8 ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    MI_U8 ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    MI_U16 wPanelHStart;                   ///<  define PANEL_HSTART (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    MI_U16 wPanelVStart;                   ///<  define PANEL_VSTART (PANEL_VSYNC_WIDTH + PANEL_VSYNC_BACK_PORCH)
    MI_U16 wPanelWidth;                    ///<  define PANEL_WIDTH
    MI_U16 wPanelHeight;                   ///<  define PANEL_HEIGHT

    MI_U16 wPanelMaxHTotal;                ///<  define PANEL_MAX_HTOTAL
    MI_U16 wPanelHTotal;                   ///<  define PANEL_HTOTAL
    MI_U16 wPanelMinHTotal;                ///<  define PANEL_MIN_HTOTAL

    MI_U16 wPanelMaxVTotal;                ///<  define PANEL_MAX_VTOTAL
    MI_U16 wPanelVTotal;                   ///<  define PANEL_VTOTAL
    MI_U16 wPanelMinVTotal;                ///<  define PANEL_MIN_VTOTAL

    MI_U8 dwPanelMaxDCLK;                  ///<  define PANEL_MAX_DCLK
    MI_U8 dwPanelDCLK;                     ///<  define PANEL_DCLK
    MI_U8 dwPanelMinDCLK;                  ///<  define PANEL_MIN_DCLK

    ///<  spread spectrum
    MI_U16 wSpreadSpectrumStep;            ///<  Value for Spread_Spectrum_Control register(B7..3:Period,B2..0:Amplitude)
    MI_U16 wSpreadSpectrumSpan;            ///<  Value for Spread_Spectrum_Control register(B7..3:Period,B2..0:Amplitude)

    MI_U8 ucDimmingCtl;                    ///<  Initial Dimming Value
    MI_U8 ucMaxPWMVal;                     ///<  Max Dimming Value
    MI_U8 ucMinPWMVal;                     ///<  Min Dimming Value

    MI_U8 bPanelDeinterMode   : 1;         ///<  define PANEL_DEINTER_MODE
    MAPI_PNL_ASPECT_RATIO ucPanelAspectRatio; ///<  Panel Aspect Ratio
    /**
    *
    * Board related params
    *
    *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
    *    : This polarity swap value =
    *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
    *  Otherwise
    *    : The value shall set to 0.
    */
    MI_U16 u16LVDSTxSwapValue;
    MAPI_APIPNL_TIBITMODE ucTiBitMode;
    MAPI_APIPNL_OUTPUTFORMAT_BITMODE ucOutputFormatBitMode;

    MI_U8 bPanelSwapOdd_RG    : 1;         ///<  define PANEL_SWAP_ODD_RG
    MI_U8 bPanelSwapEven_RG   : 1;         ///<  define PANEL_SWAP_EVEN_RG
    MI_U8 bPanelSwapOdd_GB    : 1;         ///<  define PANEL_SWAP_ODD_GB
    MI_U8 bPanelSwapEven_GB   : 1;         ///<  define PANEL_SWAP_EVEN_GB

    /**
    *  Others
    */
    MI_U8 bPanelDoubleClk     : 1;            ///<  define Double Clock
    MI_U32 dwPanelMaxSET;                     ///<  define PANEL_MAX_SET
    MI_U32 dwPanelMinSET;                     ///<  define PANEL_MIN_SET
    MAPI_APIPNL_OUT_TIMING_MODE ucOutTimingMode;   ///<Define which panel output timing change mode is used to change VFreq for same panel
	MI_U8 bPanelNoiseDith     :1;    ///<  PAFRC mixed with noise dither disable

    // Panel DCLK (U16 data type), used to break DCLK up bound 255.
    MI_U16 u16PanelMaxDCLK;
    MI_U16 u16PanelDCLK;
    MI_U16 u16PanelMinDCLK;
} __attribute__((packed)) MAPI_PanelType;

#else
/// CFD attribute
typedef struct
{
    /// Color attribute
    /// Panel Rx
    MI_U16 u16tRx;
    /// Panel Ry
    MI_U16 u16tRy;
    /// Panel Gx
    MI_U16 u16tGx;
    /// Panel Gy
    MI_U16 u16tGy;
    /// Panel Bx
    MI_U16 u16tBx;
    /// Panel By
    MI_U16 u16tBy;
    /// Panel Wx
    MI_U16 u16tWx;
    /// Panel Wy
    MI_U16 u16tWy;
    /// Panel max luminance
    MI_U16 u16MaxLuminance;
    /// Panel med luminance
    MI_U16 u16MedLuminance;
    /// Panel min luminance
    MI_U16 u16MinLuminance;
    /// Panel color format, BT.601/BT.709/BT.2020/....
    MI_U8 u8ColorFormat;
    /// Panel color data format, RGB/YUV422/YUV444/YUV420
    MI_U8 u8ColorDataFormat;
    /// Panel Full range
    MI_BOOL bIsFullRange;
    /// Linear RGB
    MI_BOOL bLinearRgb;
    /// Customer color primaries
    MI_BOOL bCustomerColorPrimaries;
    /// Source white x
    MI_U16 u16SourceWx;
    /// Source white y
    MI_U16 u16SourceWy;
} ST_MAPI_PANEL_CFD_ATTRIBUTE;

typedef struct
{
    char pPanelName[128];                      ///<  PanelName
    //
    //  Panel output
    //
    MI_U8 bPanelDither;                ///<  PANEL_DITHER
    MAPI_APIPNL_LINK_TYPE ePanelLinkType; ///<  define PANEL_LINK

    MI_U8 bPanelDualPort;             ///<  define PANEL_DUAL_PORT
    MI_U8 bPanelSwapPort;             ///<  define PANEL_SWAP_PORT
    MI_U8 bPanelSwapOdd_ML;         ///<  define PANEL_SWAP_ODD_ML
    MI_U8 bPanelSwapEven_ML;         ///<  define PANEL_SWAP_EVEN_ML
    MI_U8 bPanelSwapOdd_RB;         ///<  define PANEL_SWAP_ODD_RB
    MI_U8 bPanelSwapEven_RB;         ///<  define PANEL_SWAP_EVEN_RB

    MI_U8 bPanelSwapLVDS_POL;         //  #define PANEL_SWAP_LVDS_POL   0
    MI_U8 bPanelSwapLVDS_CH;         //  #define PANEL_SWAP_LVDS_CH    0
    MI_U8 bPanelPDP10BIT;         //  #define PANEL_PDP_10BIT       0
    MI_U8 bPanelLVDS_TI_MODE;         //  #define PANEL_LVDS_TI_MODE    _PNL_FUNC_EN_

    MI_U8 ucPanelDCLKDelay;                //  #define PANEL_DCLK_DELAY      0x00
    MI_U8 bPanelInvDCLK;             //  #define PANEL_INV_DCLK        0
    MI_U8 bPanelInvDE;                 //  #define PANEL_INV_DE          0
    MI_U8 bPanelInvHSync;             //  #define PANEL_INV_HSYNC       0
    MI_U8 bPanelInvVSync;             //  #define PANEL_INV_VSYNC       0

    ///////////////////////////////////////////////
    // Output tmming setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    MI_U8 ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    MI_U8 ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    MI_U8 ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    MI_U8 ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    MI_U16 wPanelOnTiming1;                ///<  time between panel & data while turn on power
    MI_U16 wPanelOnTiming2;                ///<  time between data & back light while turn on power
    MI_U16 wPanelOffTiming1;               ///<  time between back light & data while turn off power
    MI_U16 wPanelOffTiming2;               ///<  time between data & panel while turn off power

    MI_U8 ucPanelHSyncWidth;               ///<  define PANEL_HSYNC_WIDTH
    MI_U8 ucPanelHSyncBackPorch;           ///<  define PANEL_HSYNC_BACK_PORCH

    MI_U8 ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    MI_U8 ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    MI_U16 wPanelHStart;                   ///<  define PANEL_HSTART (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    MI_U16 wPanelVStart;                   ///<  define PANEL_VSTART (PANEL_VSYNC_WIDTH + PANEL_VSYNC_BACK_PORCH)
    MI_U16 wPanelWidth;                    ///<  define PANEL_WIDTH
    MI_U16 wPanelHeight;                   ///<  define PANEL_HEIGHT

    MI_U16 wPanelMaxHTotal;                ///<  define PANEL_MAX_HTOTAL
    MI_U16 wPanelHTotal;                   ///<  define PANEL_HTOTAL
    MI_U16 wPanelMinHTotal;                ///<  define PANEL_MIN_HTOTAL

    MI_U16 wPanelMaxVTotal;                ///<  define PANEL_MAX_VTOTAL
    MI_U16 wPanelVTotal;                   ///<  define PANEL_VTOTAL
    MI_U16 wPanelMinVTotal;                ///<  define PANEL_MIN_VTOTAL

    MI_U8 dwPanelMaxDCLK;                  ///<  define PANEL_MAX_DCLK
    MI_U8 dwPanelDCLK;                     ///<  define PANEL_DCLK
    MI_U8 dwPanelMinDCLK;                  ///<  define PANEL_MIN_DCLK

    ///<  spread spectrum
    MI_U16 wSpreadSpectrumStep;            ///<  Value for Spread_Spectrum_Control register(B7..3:Period,B2..0:Amplitude)
    MI_U16 wSpreadSpectrumSpan;            ///<  Value for Spread_Spectrum_Control register(B7..3:Period,B2..0:Amplitude)

    MI_U8 ucDimmingCtl;                    ///<  Initial Dimming Value
    MI_U8 ucMaxPWMVal;                     ///<  Max Dimming Value
    MI_U8 ucMinPWMVal;                     ///<  Min Dimming Value

    MI_U8 bPanelDeinterMode;         ///<  define PANEL_DEINTER_MODE
    MAPI_PNL_ASPECT_RATIO ucPanelAspectRatio; ///<  Panel Aspect Ratio
    /**
    *
    * Board related params
    *
    *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
    *    : This polarity swap value =
    *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
    *  Otherwise
    *    : The value shall set to 0.
    */
    MI_U16 u16LVDSTxSwapValue;
    MAPI_APIPNL_TIBITMODE ucTiBitMode;
    MAPI_APIPNL_OUTPUTFORMAT_BITMODE ucOutputFormatBitMode;

    MI_U8 bPanelSwapOdd_RG;         ///<  define PANEL_SWAP_ODD_RG
    MI_U8 bPanelSwapEven_RG;         ///<  define PANEL_SWAP_EVEN_RG
    MI_U8 bPanelSwapOdd_GB;         ///<  define PANEL_SWAP_ODD_GB
    MI_U8 bPanelSwapEven_GB;         ///<  define PANEL_SWAP_EVEN_GB

    /**
    *  Others
    */
    MI_U8 bPanelDoubleClk;            ///<  define Double Clock
    MI_U32 dwPanelMaxSET;                     ///<  define PANEL_MAX_SET
    MI_U32 dwPanelMinSET;                     ///<  define PANEL_MIN_SET
    MAPI_APIPNL_OUT_TIMING_MODE ucOutTimingMode;   ///<Define which panel output timing change mode is used to change VFreq for same panel
	MI_U8 bPanelNoiseDith;    ///<  PAFRC mixed with noise dither disable

    ////different from PanelType in apiPNL.h
    MI_BOOL bPanel3DFreerunFlag;           ///<  define flag to check if this panel should force freerun or not under 3D mode    1:forcefreerun 0: use defualt mode
    MI_BOOL bPanel2DFreerunFlag;           ///<  define flag to check if this panel should force freerun or not under 2D mode    1:forcefreerun 0: use defualt mode
    MI_BOOL bPanelReverseFlag;             ///<  define flag to check if this panel should Set 3D LRSwitch or not under 3D mode  1:Set 3D LRSwitch once  0:use default LR mode
    MI_BOOL bSGPanelFlag;                  ///<  define flag to check if this panel is SG panel   1:SG panel  0: not SG panel
    MI_BOOL bXCOutput120hzSGPanelFlag;     ///<  define flag to check if this panel is scaler direct output 120hz SG panel   1:SG panel  0: not SG panel

    /// Panel DCLK (U16 data type), used to break DCLK up bound 255.
    MI_U16 u16PanelMaxDCLK;
    MI_U16 u16PanelDCLK;
    MI_U16 u16PanelMinDCLK;

    /// CFD panel attributes.
    ST_MAPI_PANEL_CFD_ATTRIBUTE stCfdAttribute;
    MI_U16 bMirrorMode;
    MI_U16 bMirrorModeH;
    MI_U16 bMirrorModeV;

} MAPI_PanelType; ///<temp solution : use this for change paneltype information to packed format
#endif

typedef struct
{
    int nSize;
} PanelSize;

    typedef enum
    {
        DISPLAYTIMING_MIN =0,
        DISPLAYTIMING_DACOUT_DEFAULT =0,
        DISPLAYTIMING_DACOUT_FULL_HD,
        DISPLAYTIMING_DACOUT_480I,
        DISPLAYTIMING_DACOUT_480P,
        DISPLAYTIMING_DACOUT_576I,
        DISPLAYTIMING_DACOUT_576P,//5
        DISPLAYTIMING_DACOUT_720P_50,
        DISPLAYTIMING_DACOUT_720P_60,
        DISPLAYTIMING_DACOUT_1080P_24,
        DISPLAYTIMING_DACOUT_1080P_25,
        DISPLAYTIMING_DACOUT_1080P_30,//10
        DISPLAYTIMING_DACOUT_1080I_50,
        DISPLAYTIMING_DACOUT_1080P_50,
        DISPLAYTIMING_DACOUT_1080I_60,
        DISPLAYTIMING_DACOUT_1080P_60,
        DISPLAYTIMING_DACOUT_1440P_50,
        DISPLAYTIMING_DACOUT_1470P_24,
        DISPLAYTIMING_DACOUT_1470P_30,
        DISPLAYTIMING_DACOUT_1470P_60,
        DISPLAYTIMING_DACOUT_2205P_24,
        //For 2k2k
        DISPLAYTIMING_DACOUT_2K2KP_24,//20
        DISPLAYTIMING_DACOUT_2K2KP_25,
        DISPLAYTIMING_DACOUT_2K2KP_30,
        DISPLAYTIMING_DACOUT_2K2KP_60,
        //For 4k0.5k
        DISPLAYTIMING_DACOUT_4K540P_240,
        //For 4k1k
        DISPLAYTIMING_DACOUT_4K1KP_30,
        DISPLAYTIMING_DACOUT_4K1KP_60,
        DISPLAYTIMING_DACOUT_4K1KP_120,
        //For 4k2k
        DISPLAYTIMING_DACOUT_4K2KP_24,
        DISPLAYTIMING_DACOUT_4K2KP_25,
        DISPLAYTIMING_DACOUT_4K2KP_30,//30
        DISPLAYTIMING_DACOUT_4K2KP_50,
        DISPLAYTIMING_DACOUT_4K2KP_60,
        DISPLAYTIMING_DACOUT_4096P_24,
        DISPLAYTIMING_DACOUT_4096P_25,
        DISPLAYTIMING_DACOUT_4096P_30,
        DISPLAYTIMING_DACOUT_4096P_50,
        DISPLAYTIMING_DACOUT_4096P_60,
        //For VGA OUTPUT
        DISPLAYTIMING_VGAOUT_640x480P_60,
        DISPLAYTIMING_VGAOUT_1280x720P_60,
        DISPLAYTIMING_VGAOUT_1024x768P_60,
        DISPLAYTIMING_VGAOUT_1280x1024P_60,
        DISPLAYTIMING_VGAOUT_1440x900P_60,
        DISPLAYTIMING_VGAOUT_1600x1200P_60,
        DISPLAYTIMING_VGAOUT_1920x1080P_60,//40
        //For TTL output
        DISPLAYTIMING_TTLOUT_480X272_60,
        DISPLAYTIMING_DACOUT_4K2KP_120,

        DISPLAYTIMING_MAX_NUM,
    } EN_DISPLAYTIMING_RES_TYPE;

    ///Dac table timing
    typedef enum
    {
        DISPLAYTIMING_DAC_TBL_720x480I_60Hz = 0,
        DISPLAYTIMING_DAC_TBL_720x480P_60Hz,
        DISPLAYTIMING_DAC_TBL_720x576I_50Hz,
        DISPLAYTIMING_DAC_TBL_720x576P_50Hz,
        DISPLAYTIMING_DAC_TBL_1280x720P_50Hz,
        DISPLAYTIMING_DAC_TBL_1280x720P_60Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080I_50Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080I_60Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080P_50Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080P_60Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080P_30Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080P_25Hz,
        DISPLAYTIMING_DAC_TBL_1920x1080P_24Hz,

        DISPLAYTIMING_DAC_TBL_VGA_START = 0x40,      ///< Resolution for VGA output
        DISPLAYTIMING_DAC_TBL_VGA640x480P_60Hz = DISPLAYTIMING_DAC_TBL_VGA_START,
        DISPLAYTIMING_DAC_TBL_VGA1280x720P_60Hz,
        DISPLAYTIMING_DAC_TBL_VGA1920x1080P_60Hz,

        DISPLAYTIMING_DAC_TBL_TTL_START = 0xC0,      ///< Resolution for TTL output(will not coexist with VGA).
        DISPLAYTIMING_DAC_TBL_TTL480X272P_60Hz = DISPLAYTIMING_DAC_TBL_TTL_START,
    } EN_DISPLAYTIMING_DAC_TBL_TYPE;


    typedef enum
    {
        E_MAPI_HDMITX_OUTPUT_24HZ,
        E_MAPI_HDMITX_OUTPUT_25HZ,
        E_MAPI_HDMITX_OUTPUT_30HZ,
        E_MAPI_HDMITX_OUTPUT_50HZ,
        E_MAPI_HDMITX_OUTPUT_60HZ,
        E_MAPI_HDMITX_OUTPUT_FRAME_RATE_MAX,
    } EN_MAPI_HDMITX_OUTPUT_FRAME_RATE;

    typedef enum
    {
        E_DAC_TABTYPE_INIT,
        E_DAC_TABTYPE_INIT_GPIO,
        E_DAC_TABTYPE_INIT_SC,
        E_DAC_TABTYPE_INIT_MOD,
        E_DAC_TABTYPE_INIT_HDGEN,
        E_DAC_TABTYPE_INIT_HDMITX_8BIT,
        E_DAC_TABTYPE_INIT_HDMITX_8BIT_DIVIDER,
        E_DAC_TABTYPE_INIT_HDMITX_10BIT,
        E_DAC_TABTYPE_INIT_HDMITX_10BIT_DIVIDER,
        E_DAC_TABTYPE_INIT_HDMITX_12BIT,
        E_DAC_TABTYPE_INIT_HDMITX_12BIT_DIVIDER,
        E_DAC_TABTYPE_INIT_HDMITX_16BIT,
        E_DAC_TABTYPE_INIT_HDMITX_16BIT_DIVIDER,
        E_DAC_TABTYPE_GAMMA,
        E_DAC_TABTYPE_SC1_INIT = 14,
        E_DAC_TABTYPE_SC1_INIT_SC = 15
    } EN_DAC_TAB_TYPE;

    /// DAC table information
    typedef struct
    {
        MI_U8           *pu8DacINITTab;
        EN_DAC_TAB_TYPE   eINITtype;
        MI_U8           *pu8DacINIT_GPIOTab;
        EN_DAC_TAB_TYPE   eINIT_GPIOtype;
        MI_U8           *pu8DacINIT_SCTab;
        EN_DAC_TAB_TYPE   eINIT_SCtype;
        MI_U8           *pu8DacINIT_MODTab;
        EN_DAC_TAB_TYPE   eINIT_MODtype;
        MI_U8           *pu8DacINIT_HDGENTab;
        EN_DAC_TAB_TYPE   eINIT_HDGENtype;
        MI_U8           *pu8DacINIT_HDMITX_8BITTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_8BITtype;
        MI_U8           *pu8DacINIT_HDMITX_8BIT_DividerTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_8BIT_Dividertype;
        MI_U8           *pu8DacINIT_HDMITX_10BITTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_10BITtype;
        MI_U8           *pu8DacINIT_HDMITX_10BIT_DividerTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_10BIT_Dividertype;
        MI_U8           *pu8DacINIT_HDMITX_12BITTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_12BITtype;
        MI_U8           *pu8DacINIT_HDMITX_12BIT_DividerTab;
        EN_DAC_TAB_TYPE   eINIT_HDMITX_12BIT_Dividertype;
        MI_U8           *pu8DacGammaTab;
        EN_DAC_TAB_TYPE   eGammatype;
        MI_U8           *pu8DacSC1_INITTab;
        EN_DAC_TAB_TYPE   eSC1_INITtype;
        MI_U8           *pu8DacSC1_INIT_SCTab;
        EN_DAC_TAB_TYPE   eSC1_INIT_SCtype;
    } ST_DAC_TAB_INFO;

typedef struct
{

    MAPI_PanelType PanelAttr; ///PanelType
    MI_U16 u16PanelLinkExtType;
    EN_DISPLAYTIMING_RES_TYPE eTiming;
    MI_BOOL bHdmiTx;
} PanelInfo_t;

/// define SAW Type Configuration
typedef enum
{
    /// 0: dual saw
    DUAL_SAW = 0,
    /// 1: external single saw
    EXTERNAL_SINGLE_SAW,
    /// 2: silicon tuner, need tri-state
    SILICON_TUNER,
    /// 2: internal single saw DIF, need tri-state
    INTERNAL_SINGLE_SAW_DIF = SILICON_TUNER,
    /// 3: no saw
    NO_SAW,
    /// 4: internal single saw VIF
    INTERNAL_SINGLE_SAW_VIF,
    /// 5: no saw (DIF)
    NO_SAW_DIF,
     /// 6: for few pin package
    SAW6,

    SAW_NUMS
}SawArchitecture;



/// define panel backlight PWM info
typedef struct
{
    /// Panel backlight PWM Period
    MI_U32    u32PeriodPWM;
    /// Panel backlight PWM Duty
    MI_U32    u32DutyPWM;
    /// Panel backlight PWM divide
    MI_U16    u16DivPWM;
    /// Panel backlight Max PWM output threshold
    MI_U16    u16MaxPWMvalue;
    /// Panel backlight Max MSB PWM output threshold
    MI_U16    u16MaxPWMvalueMSB;
    /// Panel backligth min PWM output threshold
    MI_U16    u16MinPWMvalue;
    /// Panel backligth Min MSB PWM output threshold
    MI_U16    u16MinPWMvalueMSB;
    /// Panel backlight is from which PWM port
    MI_U8     u8PWMPort;
    /// Panel backlight PWM polarity
    MI_BOOL   bPolPWM;
    /// Panel backlight PWM Freq sync with framerate
    MI_BOOL    bBakclightFreq2Vfreq;
} PanelBacklightPWMInfo;


/// define Panel MOD PVDD Type Configuration
typedef struct
{
    /// enable
    MI_BOOL bEnabled;
    /// MOD PVDD Power Type, 0:3.3V, 1:2.5V
    MI_BOOL    bPanelModPvddPowerType;
} PanelModPvddPowerInfo;

/// to define the structure for the Auto NR Param
typedef struct DLL_PUBLIC
{
    MI_U16 u16AutoNr_L2M_Thr;
    MI_U16 u16AutoNr_M2L_Thr;
    MI_U16 u16AutoNr_M2H_Thr;
    MI_U16 u16AutoNr_H2M_Thr;
    MI_U8 u8DebugLevel;
}MAPI_AUTO_NR_INIT_PARAM;

typedef struct MI_SYSCFG_Config_s
{
    ///input mux info
    MuxSize m_MuxConf;
    MAPI_VIDEO_INPUTSRCTABLE m_pU32MuxInfo[MAPI_INPUT_SOURCE_NUM+1];

    /// VD capture window mode
    EN_VD_CAPTURE_WINDOW_MODE m_VDCaptureWinMode;

    ///audio mux info
    AudioMux_t m_pAudioMuxInfo[AUDIO_INPUT_MAX_NUM];
    AudioPath_t m_pAudioPathInfo[AUDIO_PATH_MAX_NUM];
    AudioOutputType_t m_pAudioOutputTypeInfo[AUDIO_OUTPUT_MAX_NUM];
    AudioDefualtInit_t m_AudioDefaultInit;
    AudioPath_t m_AudioMicPath;
    /// HDMI analog param info
    HDMITx_Analog_Param_t m_pHdmiTxAnalogInfo[HDMITX_ANALOG_INFO_MAX_NUM];
    ///Enable Picture Mode Use Fac Curve
    MI_BOOL m_bEnablePictureModeUseFacCurve;

    ///picture mode curve
    PictureModeCurve_t m_PictureModeCurve;

    //gamma table info
    GammaTableSize_t m_GammaConf;
    GAMMA_TABLE_t m_pGammaTableInfo[GAMMA_TABLE_MAX_NUM];
    MI_U8 m_u8DefaultGammaIdx;
    MI_U8 m_u8DLCTableCount;

    ///PanelType_name
    char m_strPanelName[NAME_MAX_NUM];

    //panel info
    PanelSize m_PanelConf;
    PanelInfo_t *m_pCurrentPanelInfo;
    PanelInfo_t m_pPanelInfo[PANEL_MAX_NUM];
    char m_pPanelInfoValidate[PANEL_INFO_VALIDATE_NUM];

    ///Panel Backlight PWM
    PanelBacklightPWMInfo  m_PanelBacklightPWMInfo;

    ///Panel MOD PVDD Power Type
    PanelModPvddPowerInfo  m_PanelModPvddPowerInfo;

    /// LVDS output type
    MI_U16 m_u16LVDS_Output_type;

    /// Auto NR Param
    MAPI_AUTO_NR_INIT_PARAM m_AutoNrParam;

    /// SAW
    SawArchitecture enSawType;

    ///CustomerPQ
    char pMainPQPath[PATH_MAX_NUM];
    char pSubPQPath[PATH_MAX_NUM];

    ///mirror config
    MI_BOOL m_bMirrorVideo;
    ///mirror mode
    MI_U8 m_u8MirrorMode;
    //video info
    ResolutionInfoSize m_ResoSize[MAXRESOLUTIONSIZE];
    ST_MAPI_VIDEO_WINDOW_INFO m_pVideoWinInfo[WINDOW_TYPE_NUM][WINDOW_TIMMING_NUM][WINDOW_TIMMING_NUM];
    MI_BOOL m_bPqBypassSupported;

    ///Enable Freerun
    MI_BOOL m_bEnableFreerun[2];
    ///Enable 3D SG Panel
    MI_BOOL m_bIsSGPanel;
    ///Is scaler output 120hz sg panel
    MI_BOOL m_bIsXCOutput120hzSGPanel;
    ///FBL's Threshold
    MI_U32  m_u32FBLThreshold;

    ///Customer Blue Screen Color Flag
    MI_BOOL m_bUseCustomerScreenMuteColor;
    ///Screen Mute Color Type
    MI_U8 m_u8CustomerScreenMuteColorType;
    ///Frame Color Type
    MI_U8 m_u8CustomerFrameColorType[3];

    ///MAC SPI Offset
    MI_U16 m_u16MacSPIOffset;

    char pAmpInitBinPath[PATH_MAX_NUM];

    MI_U8 m_PQPathName[PATH_MAX_NUM];

    //DLC
    MAPI_XC_DLC_init Board_DLC_init[DLCTABLE_MAX_NUM] ;

    //Current Mode
    EN_CURRENT_MODE_TYPE m_CurrentMode;

    //Color matrix
    MAPI_COLOR_MATRIX m_stMatrx;

    //BW set depends on customer mode.1:customer mode,0:no customer mode.
    MI_BOOL m_bBwCusMode;
} MI_SYSCFG_Config_t;

#define DYNAMIC_NR_TBL_MOTION_LEVEL_NUM    6
#define DYNAMIC_NR_TBL_LUMA_LEVEL_NUM      8
#define DYNAMIC_NR_TBL_NOISE_LEVEL_NUM     5

#define DYNAMIC_NR_TBL_REG_NUM      8


#define ENABLE_DYNAMIC_NR   1
#if ENABLE_DYNAMIC_NR
    typedef struct
    {
        MI_U8 u8CoringOffset;
        MI_U8 u8SNROffset;
    } MAPI_PQL_DYNAMIC_NR_MISC_PARAM_LUMA;

    typedef struct
    {
        MI_U8 u8Spike_NR_0;
        MI_U8 u8Spike_NR_1;
        MI_U8 u8Gray_Guard_En;
        MI_U8 u8Gray_Guard_Gain;
    } MAPI_PQL_DYNAMIC_NR_MISC_PARAM;

    typedef struct
    {
        MI_U8 u8CoringOffset;
        MI_U8 u8SharpnessOffset;
        MI_U8 u8NM_V;
        MI_U8 u8NM_H_0;
        MI_U8 u8NM_H_1;
        MI_U8 u8SC_Coring;
        MI_U8 u8GNR_0;
        MI_U8 u8GNR_1;
        MI_U8 u8SpikeNR_0;
        MI_U8 u8SpikeNR_1;
        MI_U8 u8CP;
        MI_U8 u8DP;
        MI_U8 u8AGC_Gain_Offset;
        MI_U8 u8Gray_Guard_En;
        MI_U8 u8Gray_Guard_Gain;
    } MAPI_PQL_DYNAMIC_NR_MISC_PARAM_NOISE;


    typedef struct
    {
        MI_U8   u8DeFlicker_Step1;
        MI_U8   u8DeFlicker_Step2;
        MI_U8   u8M_DeFi_Th;
        MI_U8   u8L_DeFi_Th;
        MI_U8   u8NoiseThreshold[DYNAMIC_NR_TBL_NOISE_LEVEL_NUM-1];
        MI_U8   u8NoiseMag;
        MI_BOOL bMotionEn;
        MI_BOOL bLumaEn;
        MI_U8   u8MotionStartLvl;
        MI_U8   u8MotionEndLvl;
        MI_U8   u8LumaStartLvl;
        MI_U8   u8LumaEndLvl;
    } MAPI_PQL_DYNAMIC_NR_PARAM;
#endif

/// DLC init curve
typedef enum
{
    /// ucLumaCurve
    E_MAPI_DLC_INIT_LUMA_CURVE = 0,
    /// ucLumaCurve2_a
    E_MAPI_DLC_INIT_LUMA_CURVE2_A ,
    /// ucLumaCurve2_b
    E_MAPI_DLC_INIT_LUMA_CURVE2_B ,
    /// ucDlcHistogramLimitCurve
    E_MAPI_DLC_INIT_HISTOGRAM_LIMIT_CURVE ,
} E_MAPI_DLC_PURE_INIT_CURVE ;

typedef struct MI_SYSCFG_PQConfig_s {
    MI_U8 m_u8DBC_MAX_VIDEO_DBC          ;
    MI_U8 m_u8DBC_MID_VIDEO_DBC          ;
    MI_U8 m_u8DBC_MIN_VIDEO_DBC          ;
    MI_U8 m_u8DBC_MAX_PWM                ;
    MI_U8 m_u8DBC_BACKLIGHT_THRES        ;
    MI_U8 m_u8DBC_MIN_PWM                ;
    MI_U8 m_u8DBC_Y_GAIN_M               ;
    MI_U8 m_u8DBC_Y_GAIN_L               ;
    MI_U8 m_u8DBC_C_GAIN_M               ;
    MI_U8 m_u8DBC_C_GAIN_L               ;
    MI_U8 m_u8DBC_ALPHA_BLENDING_CURRENT ;
    MI_U8 m_u8DBC_AVG_DELTA              ;
    MI_U8 m_u8DBC_FAST_ALPHABLENDING     ;
    MI_U8 m_u8DBC_LOOP_DLY_H             ;
    MI_U8 m_u8DBC_LOOP_DLY_MH            ;
    MI_U8 m_u8DBC_LOOP_DLY_ML            ;
    MI_U8 m_u8DBC_LOOP_DLY_L            ;
    MI_U8 m_u8tDynamicNRTbl_Motion_rows;
    MI_U8 m_u8tDynamicNRTbl_Motion_cols;
    MI_U8 m_u8tDynamicNRTbl_Motion_HDMI_DTV_rows;
    MI_U8 m_u8tDynamicNRTbl_Motion_HDMI_DTV_cols;
    MI_U8 m_u8tDynamicNRTbl_Motion_COMP_PC_rows;
    MI_U8 m_u8tDynamicNRTbl_Motion_COMP_PC_cols;
    MI_U8 m_u8tDynamicNRTbl_MISC_Param_rows;
    MI_U8 m_u8tDynamicNRTbl_MISC_Param_cols;
    MI_U8 m_u8tDynamicNRTbl_MISC_Luma_rows;
    MI_U8 m_u8tDynamicNRTbl_MISC_Luma_cols;
    MI_U8 m_u8tDynamicNRTbl_Noise_rows;
    MI_U8 m_u8tDynamicNRTbl_Noise_cols;
    MI_U8 m_u8tDynamicNR_MISC_Noise_rows;
    MI_U8 m_u8tDynamicNR_MISC_Noise_cols;

    MI_U8 *m_pu8tDynamicNRTbl_Noise;
    MI_U8 *m_pu8tDynamicNRTbl_Motion;
    MI_U8 *m_pu8tDynamicNRTbl_Motion_HDMI_DTV;
    MI_U8 *m_pu8tDynamicNRTbl_Motion_COMP_PC;

    //MI_U8 **m_ppu8tDynamicNRTbl_MISC_Param;
    MAPI_PQL_DYNAMIC_NR_MISC_PARAM* tDynamicNRTbl_MISC_Param;
    //MI_U8 **m_ppu8tDynamicNRTbl_MISC_Luma;
    MAPI_PQL_DYNAMIC_NR_MISC_PARAM_LUMA* tDynamicNRTbl_MISC_Luma;
    //MI_U8 **m_ppu8tDynamicNR_MISC_Noise;
    MAPI_PQL_DYNAMIC_NR_MISC_PARAM_NOISE* tDynamicNR_MISC_Noise;
} MI_SYSCFG_PQConfig_t;

#endif /* _MI_SYSCFG_DATATYPE_H_ */
