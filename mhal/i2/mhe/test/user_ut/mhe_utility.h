
#ifndef __MHE_UTILITY_H__
#define __MHE_UTILITY_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cam_os_wrapper.h"
#include "mhal_venc.h"

typedef enum MHE_ERROR {
    MheErrorNone = 0,
    MheFdError,
    MheSetError,

} MHE_ERROR;

typedef int bool;

/*typedef enum BOOL {
    FALSE = 0,
    TRUE = !FALSE,
    BOOL_MAX = 0x7FFFFFFF
} BOOL;

typedef BOOL       bool;
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

extern int nMultiInsNum;

typedef struct{
     u32 w;
     u32 h;
     char pixfm[64];    //NV12, NV21, YUYV
     u32 fps;
     u32 Gop;
     u32 bitrate;
     u32 nQp;
     bool bEn;
}VideoBasicSettings;

typedef struct{
    u32 nBase;
    u32 nEnhance;
    u32 bEnabledPred;
    u32 bEn;
}H265LTR;

typedef struct{
    bool b_deblocking_cross_slice_enable;
    bool b_deblocking_disable;
    s32 slice_tc_offset_div2;
    s32 slice_beta_offset_div2;
    bool bEn;
}H265deblock;

typedef struct{
    bool bDisposable;
    bool bEn;
}H265Disposable;

typedef struct{
    u32 nPocType;
    bool bEn;
}H265Poc;

typedef struct{
    bool nQpOffset;
    bool bEn;
}H265Trans;


typedef enum Mhe_VIDEO_CONTROLRATETYPE {
        Mhe_RQCT_METHOD_CQP = 0,  //!< constant QP.
        Mhe_RQCT_METHOD_CBR,    //!< constant bitrate.
        Mhe_RQCT_METHOD_VBR,    //!< variable bitrate.
} Mhe_VIDEO_CONTROLRATETYPE;


typedef struct{
    Mhe_VIDEO_CONTROLRATETYPE eControlRate;
    u32 nImaxqp;
    u32 nIminqp;
    u32 nPmaxqp;
    u32 nPminqp;
    //u32 nTargetBitrate;
    //u32 nRefQp;
    //u32 nGop;
    bool bEn;
}H265RC;

typedef struct{
    bool bconstIpred;
    bool bIntraCu8Lose;
    bool bInterCu8Lose;
    bool bIntraCu16Lose;
    bool bInterCu16Lose;
    bool bInterCu16MergeLose;
    bool bInterCu16SkipLose;
    bool bInterCu16MergeSkipLose;
    bool bIntraCu32Lose;
    bool bInterCu32MergeLose;
    bool bInterCu32MergeSkipLose;
    u16 nIntraCu8Pen;
    u16 nInterCu8Pen;
    u16 nIntraCu16Pen;
    u16 nInterCu16Pen;
    u16 nInterCu16MergePen;
    u16 nInterCu16SkipPen;
    u16 nInterCu16MergeSkipPen;
    u16 nIntra32Pen;
    u16 nIntra32MergePen;
    u16 nIntra32MergeSkipPen;
    bool bEn;
}H265IntraP;

typedef struct{
    u32 nDmv_X;/* dMV x-direction (8~16)*/
    u32 nDmv_Y;/* dMV y-direction (8/16)*/
    u32 nSubpel; /* subpel: 0-integral,1-half,2-quarter  */
    bool bInter4x4PredEn;
    bool bInter8x4PredEn;
    bool bInter4x8PredEn;
    bool bInter8x8PredEn;
    bool bInter16x8PredEn;
    bool bInter8x16PredEn;
    bool bInter16x16PredEn;
    bool bInterSKIPPredEn;
    bool bEn;
}H265InterP;

typedef struct{
    u32 nRows;
    u32 nBits;
    bool bEn;
}H265SS;

typedef struct{
    u32 nIdx;
    bool bEnable;
    bool bAbsQp;
    u32 nMbqp;
    u32 nMbX;
    u32 nMbY;
    u32 nMbW;
    u32 nMbH;
    bool bEn;
}H265ROI;

typedef struct{
    bool bEnable;
    u32 nEntry[14];
    u32 nVirAddr;
    bool bEn;
}H265QPMAP;

typedef struct{
    bool bColor2Grey;
    bool bEn;
}H265Color2Grey;

typedef struct{
    u8 U8data[1024];
    u32 u32Len;
    bool bEn;
}H265SEI;

typedef struct{
    u16 u16SarWidth;
    u16 u16SarHeight;
    u8 u8TimingInfoPresentFlag;
    u8 u8VideoFullRangeFlag;
    u8 u8VideoSignalTypePresentFlag;
    u8 u8VideoFormat;
    u8 u8ColourDescriptionPresentFlag;
    bool bEn;
}H265VUI;

typedef struct{
    u32 nWidth;
    u32 nHeight;
    u32 xFramerate;
    bool bEn;
}H265NewSeq;

typedef struct
{
    u32 nClkEn;
    u32 nClkSor;
    bool bEn;
} VideoClkSettings_t;

typedef struct
{
    enum mhe_pmbr_cfg_e {
    MHE_PMBR_CFG_SEQ = 0,
    MHE_PMBR_CFG_TC,
    MHE_PMBR_CFG_PC,
    MHE_PMBR_CFG_SS,
    MHE_PMBR_CFG_MEM,
    MHE_PMBR_CFG_END
    } type;


    struct {
    //enum mhe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_TC"
    int                 i_LutEntryClipRange;
    int                 i_TextWeightType;
    int                 i_SkinLvShift;
    int                 i_WeightOffset;
    int                 i_TcOffset;
    int                 i_TcToSkinAlpha;
    int                 i_TcGradThr;
    bool bEn;   //setting enable
    } tc;

    struct {
    //enum mhe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_PC"
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
    bool bEn;   //setting enable
    } pc;

    struct {
    //enum mhe_pmbr_cfg_e i_type;     //!< MUST BE "DRV_PMBR_CFG_SS"
    int                 i_SmoothEn;
    int                 i_SmoothClipMax;
    int                 i_SecStgAlpha;
    int                 i_SecStgBitWghtOffset;
    bool bEn;   //setting enable
    } ss;

} H265RcRMBR;

typedef struct{
     int nInstanceId;
     int FramesToBeEncoded;
     int mhefd;
     char cfgName[64];
     char inputPath[64];
     char outputPath[64];
     VideoBasicSettings basic;
     H265LTR sLTR;
     H265deblock sDeblk;
     H265Disposable sDisposable;
     H265Poc sPoc;
     H265Trans sTrans;
     H265RC sRC;
     H265IntraP sIntraP;
     H265InterP sInterP;
     H265SS sMSlice;
     H265ROI sRoi[MAX_ROI_NUM];
     H265QPMAP sQpMap;
     H265SEI sSEI[MAX_SEI_NUM];
     H265VUI sVUI;
     H265NewSeq sNewSeq;
     VideoClkSettings_t sClock;
     H265RcRMBR sPMBR;
}Mheparam;

int ReadDefaultConfig(Mheparam* p);
void DisplayMenuSetting(void);
void DisplayMHESetting(void);
int SetInOutPath(Mheparam* pParam);
void SetMHEParameter(Mheparam* pParam);
void ShowCurrentMHESetting(Mheparam* pParam);


#endif /*__MHE_UTILITY_H__*/
