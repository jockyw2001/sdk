
#ifndef __MFE_UTILITY_H__
#define __MFE_UTILITY_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cam_os_wrapper.h"
#include "mhal_common.h"

typedef enum ERROR {
    ErrorNone = 0,
    MfeFdError,
    MfeSetError,

} ERROR;

typedef int bool;

/*typedef enum BOOL {
    FALSE = 0,
    TRUE = !FALSE,
    BOOL_MAX = 0x7FFFFFFF
} BOOL;

typedef BOOL       u32;
typedef unsigned char       u8;
typedef signed   char       s8;
typedef unsigned short      u16;
typedef signed   short      s16;
typedef unsigned int        u32;
typedef signed   int        s32;
typedef unsigned long long  u64;
typedef signed   long long  s64;*/

/** defining global declarations */

#define VIDEO_ENCODER_INPUT_PORT_NUMBER     0
#define VIDEO_ENCODER_OUTPUT_PORT_NUMBER    1
#define MAX_ROI_NUM 8
#define MAX_SEI_NUM 4

typedef struct{
     u32 w;
     u32 h;
     char pixfm[64];    //NV12, NV21, YUYV
     u32 fps;
     u32 Gop;
     u32 bitrate;
     u32 nQp;
     u32 bEn;
}VideoBasicSettings;

typedef struct{
    u32 nBase;
    u32 nEnhance;
    u32 bEnabledPred;
    u32 bEn;
}H264LTR;

typedef struct{
    u32 deblock_filter_control;
    u32 disable_deblocking_filter_idc;
    s32 slice_alpha_c0_offset_div2;
    s32 slice_beta_offset_div2;
    u32 bEn;
}H264deblock;

typedef struct{
    u32 bDisposable;
    u32 bEn;
}H264Disposable;

typedef struct{
    u32 nPocType;
    u32 bEn;
}H264Poc;

typedef struct{
    u32 bEntropyCodingCABAC;
    u32 bEn;
}H264Entropy;


typedef enum Mfe_VIDEO_CONTROLRATETYPE {
        Mfe_RQCT_METHOD_CQP = 0,  //!< constant QP.
        Mfe_RQCT_METHOD_CBR,    //!< constant bitrate.
        Mfe_RQCT_METHOD_VBR,    //!< variable bitrate.
} Mfe_VIDEO_CONTROLRATETYPE;


typedef struct{
    Mfe_VIDEO_CONTROLRATETYPE eControlRate;
    u32 nImaxqp;
    u32 nIminqp;
    u32 nPmaxqp;
    u32 nPminqp;
    //u32 nTargetBitrate;
    //u32 nRefQp;
    //u32 nGop;
    u32 bEn;
}H264RC;

typedef struct{
    u32 bconstIpred;
    u32 bEn;
}H264IntraP;

typedef struct{
    u32 nDmv_X;/* dMV x-direction (8~16)*/
    u32 nDmv_Y;/* dMV y-direction (8/16)*/
    u32 nSubpel; /* subpel: 0-integral,1-half,2-quarter  */
    u32 bInter4x4PredEn;
    u32 bInter8x4PredEn;
    u32 bInter4x8PredEn;
    u32 bInter8x8PredEn;
    u32 bInter16x8PredEn;
    u32 bInter8x16PredEn;
    u32 bInter16x16PredEn;
    u32 bInterSKIPPredEn;
    u32 bEn;
}H264InterP;

typedef struct{
    u32 nRows;
    u32 nBits;
    u32 bEn;
}H264SS;

typedef struct{
    u32 nIdx;
    u32 bEnable;
    u32 bAbsQp;
    u32 nMbqp;
    u32 nMbX;
    u32 nMbY;
    u32 nMbW;
    u32 nMbH;
    u32 bEn;
}H264ROI;

typedef struct{
    bool bEnable;
    u32 nEntry[14];
    u32 nVirAddr;
    bool bEn;
}H264QPMAP;

typedef struct{
    u32 bColor2Grey;
    u32 bEn;
}H264Color2Grey;

typedef struct{
    u8 U8data[1024];
    u32 u32Len;
    u32 bEn;
}H264SEI;

typedef struct{
    u16 u16SarWidth;
    u16 u16SarHeight;
    u8 u8TimingInfoPresentFlag;
    u8 u8VideoFullRangeFlag;
    u8 u8VideoSignalTypePresentFlag;
    u8 u8VideoFormat;
    u8 u8ColourDescriptionPresentFlag;
    u32 bEn;
}H264VUI;

typedef struct{
    u32 nWidth;
    u32 nHeight;
    u32 xFramerate;
    u32 bEn;
}H264NewSeq;

typedef struct
{
    u32 nClkEn;
    u32 nClkSor;
    u32 bEn;
} VideoClkSettings_t;

typedef struct
{
    enum mfe_pmbr_cfg_e {
    MFE_PMBR_CFG_SEQ = 0,
    MFE_PMBR_CFG_TC,
    MFE_PMBR_CFG_PC,
    MFE_PMBR_CFG_SS,
    MFE_PMBR_CFG_MEM,
    MFE_PMBR_CFG_END
    } type;


    struct {
    //enum mfe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_TC"
    int                 i_LutEntryClipRange;
    int                 i_TextWeightType;
    int                 i_SkinLvShift;
    int                 i_WeightOffset;
    int                 i_TcOffset;
    int                 i_TcToSkinAlpha;
    int                 i_TcGradThr;
    u32 bEn;   //setting enable
    } tc;

    struct {
    //enum mfe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_PC"
    int                 i_PwYMax;
    int                 i_PwYMin;
    int                 i_PwCbMax;
    int                 i_PwCbMin;
    int                 i_PwCrMax;
    int                 i_PwCrMin;
    int                 i_PwCbPlusCrMin;
    int                 i_PwAddConditionEn;
    int                 i_PwCrOffset;
    int                 i_PwCbCrOffset;
    int                 i_PcDeadZone;
    u32 bEn;   //setting enable
    } pc;

    struct {
    //enum mfe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_SS"
    int                 i_SmoothEn;
    int                 i_SmoothClipMax;
    int                 i_SecStgAlpha;
    int                 i_SecStgBitWghtOffset;
    u32 bEn;   //setting enable
    } ss;

} H264RcRMBR;

typedef struct{
     int mfefd;
     int FramesToBeEncoded;
     char inputPath[64];
     char outputPath[64];
     VideoBasicSettings basic;
     H264LTR sLTR;
     H264deblock sDeblk;
     H264Disposable sDisposable;
     H264Poc sPoc;
     H264Entropy sEntropy;
     H264RC sRC;
     H264IntraP sIntraP;
     H264InterP sInterP;
     H264SS sMSlice;
     H264ROI sRoi[MAX_ROI_NUM];
     H264QPMAP sQpMap;
     H264SEI sSEI[MAX_SEI_NUM];
     H264VUI sVUI;
     H264NewSeq sNewSeq;
     VideoClkSettings_t sClock;
     H264RcRMBR sPMBR;
}Mfe_param;

int ReadDefaultConfig(const char * filename, Mfe_param* p);
void DisplayMenuSetting(void);
void DisplayMFESetting(void);
int SetInOutPath(Mfe_param* pParam);
void SetMFEParameter(Mfe_param* pParam);
void ShowCurrentMFESetting(Mfe_param* pParam);


#endif /*__MFE_UTILITY_H__*/
