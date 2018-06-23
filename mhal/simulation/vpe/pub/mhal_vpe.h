////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
/**
 *  @file HAL_VPE.h
 *  @brief HAL_VPE Driver interface
 */

/**
 * \defgroup HAL_VPE_group  HAL_VPE driver
 * @{
 */
#ifndef __MHAL_VPE_H__
#define __MHAL_VPE_H__

//=============================================================================
// Includs
//=============================================================================
#include "mhal_cmdq.h"
//#include "mhal_common.h"

//-------------------------------------------------------------------------------------------------
//  Defines & enum
//-------------------------------------------------------------------------------------------------
#define  ROI_WINDOW_MAX 4
#define  WDR_HIST_BUFFER 4
#define  TEMP_WAIT 1

typedef enum
{
    E_MHAL_SCL_ROTATION_Off,     //
    E_MHAL_SCL_ROTATION_90,      //
    E_MHAL_SCL_ROTATION_180,     //
    E_MHAL_SCL_ROTATION_270,     //
    E_MHAL_SCL_ROTATION_TYPE,    //
}MHalVpeSclRotationType_e;
typedef enum
{
    E_MHAL_ISP_INPUT_YUV420,      //
    E_MHAL_ISP_INPUT_YUV422,      //
    E_MHAL_ISP_INPUT_TYPE,        //
}MHalVpeIspInputFormat_e;
typedef enum
{
    E_MHAL_SCL_OUTPUT_DRAM,      //
    E_MHAL_SCL_OUTPUT_MDWIN,      //
    E_MHAL_SCL_OUTPUT_IMI,
}MHalVpeSclOutputType_e;
typedef enum
{
    E_MHAL_IQ_ROISRC_BEFORE_WDR,      //
    E_MHAL_IQ_ROISRC_AFTER_WDR,      //
    E_MHAL_IQ_ROISRC_WDR,        //
    E_MHAL_IQ_ROISRC_TYPE,        //
}MHalVpeIqWdrRoiSrcType_e;
typedef enum
{
    E_MHAL_SCL_OUTPUT_NV12420,      //
    E_MHAL_SCL_OUTPUT_YUYV422,      //
    E_MHAL_SCL_OUTPUT_YCSEP422,      //
    E_MHAL_SCL_OUTPUT_YUVSEP422,      //
    E_MHAL_SCL_OUTPUT_YUVSEP420,      //
    E_MHAL_SCL_OUTPUT_TYPE,        //
}MHalVpeSclOutputFormat_e;
typedef enum
{
    E_MHAL_SCL_OUTPUT_PORT0 = 0,      //
    E_MHAL_SCL_OUTPUT_PORT1 = 1,      //
    E_MHAL_SCL_OUTPUT_PORT2 = 2,      //
    E_MHAL_SCL_OUTPUT_PORT3 = 3,      //
    E_MHAL_M2M_INPUT_PORT = 4,      //
    E_MHAL_ROT_WDMA_PORT = 5,      //
	E_MHAL_DMA_PORT_MAX		//I2 Max =5 ,I5 Max=6
}MHalVpeDmaPort_e;
typedef enum
{
    E_MHAL_SCL_OUTPUT_MDWin0,      //
    E_MHAL_SCL_OUTPUT_MDWin1,      //
    E_MHAL_SCL_OUTPUT_MDWin2,      //
    E_MHAL_SCL_OUTPUT_MDWin3,      //
    E_MHAL_SCL_OUTPUT_MDWin4,      //
    E_MHAL_SCL_OUTPUT_MDWin5,      //
    E_MHAL_SCL_OUTPUT_MDWin6,      //
    E_MHAL_SCL_OUTPUT_MDWin7,      //
    E_MHAL_SCL_OUTPUT_MDWin8,      //
    E_MHAL_SCL_OUTPUT_MDWin9,      //
    E_MHAL_SCL_OUTPUT_MDWin10,      //
    E_MHAL_SCL_OUTPUT_MDWin11,      //
    E_MHAL_SCL_OUTPUT_MDWin12,      //
    E_MHAL_SCL_OUTPUT_MDWin13,      //
    E_MHAL_SCL_OUTPUT_MDWin14,      //
    E_MHAL_SCL_OUTPUT_MDWin15,      //
}MHalVpeSclOutputMDWin_e;

typedef enum
{
    E_MHAL_SCL_OSD_NUM0,      //
    E_MHAL_SCL_OSD_NUM1,      //
    E_MHAL_SCL_OSD_NUM2,      //
    E_MHAL_SCL_OSD_NUM3,      //
    E_MHAL_SCL_COVER_NUM0,      //
    E_MHAL_SCL_COVER_NUM1,      //
    E_MHAL_SCL_OSD_TYPE,        //
}MHalVpeSclOsdNum_e;


typedef enum
{
    E_MHAL_SCL_OUTPUT_MODE_INVALID = 0,
    E_MHAL_SCL_OUTPUT_MODE_PORT0 = 0x1,
    E_MHAL_SCL_OUTPUT_MODE_PORT1 = 0x2,
    E_MHAL_SCL_OUTPUT_MODE_PORT2 = 0x4,
    E_MHAL_SCL_OUTPUT_MODE_PORT3 = 0x8,
    E_MHAL_SCL_OUTPUT_MODE_SCALING_2LEVEL_MODE = 0x100,
    E_MHAL_SCL_OUTPUT_MODE_SCALING_3LEVEL_MODE = 0x200,
    E_MHAL_SCL_OUTPUT_MODE_3PORT_2LEVEL_MODE = (E_MHAL_SCL_OUTPUT_MODE_PORT0|E_MHAL_SCL_OUTPUT_MODE_PORT1|E_MHAL_SCL_OUTPUT_MODE_PORT3|E_MHAL_SCL_OUTPUT_MODE_SCALING_2LEVEL_MODE),
    E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE = (E_MHAL_SCL_OUTPUT_MODE_PORT0|E_MHAL_SCL_OUTPUT_MODE_PORT1|E_MHAL_SCL_OUTPUT_MODE_PORT2|E_MHAL_SCL_OUTPUT_MODE_PORT3|E_MHAL_SCL_OUTPUT_MODE_SCALING_2LEVEL_MODE),
    E_MHAL_SCL_OUTPUT_MODE_PORT2_M2M_MODE = (E_MHAL_SCL_OUTPUT_MODE_PORT2),
    E_MHAL_SCL_OUTPUT_MODE_MAX,
} MHalSclOutputMode_e;
typedef enum
{
    E_MHAL_VPE_INPUT_MODE_INVALID = 0,
    E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,
    E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE,
    E_MHAL_VPE_INPUT_MODE_REALTIME_MODE,
} MHalVpeTopInputMode_e;
typedef enum
{
     E_MHAL_VPE_SCL_MAIN,
     E_MHAL_VPE_SCL_SUB
}MHhalVpeSclInstance_e;
 typedef enum
 {
    E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME ,
    E_MHAL_SCL_MAIN_FRAME_ROTATION ,
    E_MHAL_SCL_SUB_INPUT_MODE_LDC_TIME,
    E_MHAL_SCL_SUB_FRAME_ROTATION,
    E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME,
 }MHalSclInputPortMode_e;

typedef enum
{
	E_MHAL_SCL_IRQ_NONE   = 0,
	E_MHAL_SCL_IRQ_VSYNC     = 0x1,
	E_MHAL_SCL_IRQ_FRAMEDONE = 0x2
}MHalVpeIrqStatus_e;

typedef enum
{
    E_MHAL_SCL_IRQ_PORT0 = 0x1,
    E_MHAL_SCL_IRQ_PORT1 = 0x2,
    E_MHAL_SCL_IRQ_PORT2 = 0x4,
    E_MHAL_SCL_IRQ_PORT3 = 0x8,
    E_MHAL_SCL_IRQ_3PORT_MODE = (E_MHAL_SCL_IRQ_PORT0|E_MHAL_SCL_IRQ_PORT1|E_MHAL_SCL_IRQ_PORT3),
    E_MHAL_SCL_IRQ_4PORT_MODE = (E_MHAL_SCL_IRQ_PORT0|E_MHAL_SCL_IRQ_PORT1|E_MHAL_SCL_IRQ_PORT2|E_MHAL_SCL_IRQ_PORT3),
    E_MHAL_SCL_IRQ_ONLYDONEINT_MODE = 0x10,
    E_MHAL_SCL_IRQ_MULTIINT_MODE = 0x20,
    E_MHAL_SCL_IRQ_FRAME_MODE = 0x40,
    E_MHAL_SCL_IRQ_REALTIME_MODE = 0x80,
    E_MHAL_SCL_IRQ_M2M_MODE = 0x100,
    E_MHAL_SCL_IRQ_LDC_MODE = 0x200,
    E_MHAL_SCL_IRQ_ROT_MODE = 0x400,
    E_MHAL_SCL_IRQ_PORT_ROTWDMA = 0x800,
}MHalVpeIrqEnableSubMode_e;

typedef enum
{
	E_MHAL_SCL_IRQ_OFF   = 0,
    E_MHAL_SCL_IRQ_OFFM2M = 0x1000,
    E_MHAL_SCL_IRQ_OFFROT = 0x2000,
	E_MHAL_SCL_IRQ_DVR_FRAMEBUF_4PORT = (E_MHAL_SCL_IRQ_4PORT_MODE|E_MHAL_SCL_IRQ_ONLYDONEINT_MODE|E_MHAL_SCL_IRQ_FRAME_MODE),
    E_MHAL_SCL_IRQ_DVR_FRAMEBUF_3PORT = (E_MHAL_SCL_IRQ_3PORT_MODE|E_MHAL_SCL_IRQ_ONLYDONEINT_MODE|E_MHAL_SCL_IRQ_FRAME_MODE),
    E_MHAL_SCL_IRQ_CAM_FRAMEBUF_4PORT = (E_MHAL_SCL_IRQ_4PORT_MODE|E_MHAL_SCL_IRQ_MULTIINT_MODE|E_MHAL_SCL_IRQ_FRAME_MODE),
	E_MHAL_SCL_IRQ_CAM_FRAMEBUF_3PORT = (E_MHAL_SCL_IRQ_3PORT_MODE|E_MHAL_SCL_IRQ_MULTIINT_MODE|E_MHAL_SCL_IRQ_FRAME_MODE),
    E_MHAL_SCL_IRQ_CAM_REALTIME_4PORT = (E_MHAL_SCL_IRQ_4PORT_MODE|E_MHAL_SCL_IRQ_MULTIINT_MODE|E_MHAL_SCL_IRQ_REALTIME_MODE),
    E_MHAL_SCL_IRQ_CAM_REALTIME_3PORT = (E_MHAL_SCL_IRQ_3PORT_MODE|E_MHAL_SCL_IRQ_MULTIINT_MODE|E_MHAL_SCL_IRQ_REALTIME_MODE),
    E_MHAL_SCL_IRQ_PORT2_M2M = (E_MHAL_SCL_IRQ_PORT2|E_MHAL_SCL_IRQ_M2M_MODE),
    E_MHAL_SCL_IRQ_ROT_WDMA = (E_MHAL_SCL_IRQ_ROT_MODE|E_MHAL_SCL_IRQ_PORT_ROTWDMA),
}MHalVpeIrqEnableMode_e;

#if 0
/// data type 64bit physical address
typedef unsigned long long                       PHY;        // 8 bytes
typedef enum
{
    E_MHAL_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MHAL_PIXEL_FRAME_ARGB8888,
    E_MHAL_PIXEL_FRAME_ABGR8888,

    E_MHAL_PIXEL_FRAME_RGB565,
    E_MHAL_PIXEL_FRAME_ARGB1555,
    E_MHAL_PIXEL_FRAME_I2,
    E_MHAL_PIXEL_FRAME_I4,
    E_MHAL_PIXEL_FRAME_I8,

    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    // mstar mdwin/mgwin
    E_MHAL_PIXEL_FRAME_YUV_MST_420,

    //vdec mstar private video format
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE1_H264,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE2_H265,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE3_H265,
    E_MHAL_PIXEL_FRAME_FORMAT_MAX,
} MHalPixelFormat_e;

typedef enum
{
    E_MHAL_COMPRESS_MODE_NONE,//no compress
    E_MHAL_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_MHAL_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_MHAL_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_MHAL_COMPRESS_MODE_BUTT, //number
} MHalPixelCompressMode_e;
#endif
typedef enum
{
    E_MHAL_VPE_DEBUG_OFF         = 0x0,      //
    E_MHAL_VPE_DEBUG_LEVEL1      = 0x1,      //low freq size         //low cal
    E_MHAL_VPE_DEBUG_LEVEL2      = 0x2,      //low freq config     //low config
    E_MHAL_VPE_DEBUG_LEVEL3      = 0x4,      //high freq size       //high cal
    E_MHAL_VPE_DEBUG_LEVEL4      = 0x8,      //high freq config   //high config
    E_MHAL_VPE_DEBUG_LEVEL5      = 0x10,      //port 0              //reserve for IQ
    E_MHAL_VPE_DEBUG_LEVEL6      = 0x20,      //port 1              //reserve for IQ
    E_MHAL_VPE_DEBUG_LEVEL7      = 0x40,      //port 2              //reserve for IQ
    E_MHAL_VPE_DEBUG_LEVEL8      = 0x80,      //port 3              //reserve for IQ
    E_MHAL_VPE_DEBUG_NUM,        //
}MHalVpeDebugLevel_e;
typedef enum
{
    E_MHAL_VPE_DEBUG_TYPE_SC     = 0,      //
    E_MHAL_VPE_DEBUG_TYPE_ISP,      //
    E_MHAL_VPE_DEBUG_TYPE_IQ,      //
    E_MHAL_VPE_DEBUG_TYPE,        //
}MHalVpeDebugType_e;
typedef enum
{
    E_MHAL_VPE_WAITDONE_ERR     = 0,      //
    E_MHAL_VPE_WAITDONE_DMAONLY,      //
    E_MHAL_VPE_WAITDONE_MDWINONLY,      //
    E_MHAL_VPE_WAITDONE_DMAANDMDWIN,      //
    E_MHAL_VPE_WAITDONE_TYPE,        //
}MHalVpeWaitDoneType_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MS_U8 u8Channel;
} MHalVpeSclExchangeCtx_t;

typedef struct
{
    MHalVpeSclRotationType_e enRotType;
} MHalVpeSclRotationConfig_t;

typedef struct
{
    MHalVpeIspInputFormat_e enInType;
    MHalPixelFormat_e  ePixelFormat;
    MHalPixelCompressMode_e eCompressMode;
    MS_S32 u32Width;
    MS_S32 u32Height;
} MHalVpeIspInputConfig_t;

typedef struct
{
    MS_BOOL bNREn;
    MS_BOOL bEdgeEn;
    MS_BOOL bESEn;
    MS_BOOL bContrastEn;
    MS_BOOL bUVInvert;
} MHalVpeIqOnOff_t;

typedef struct
{
    MS_U8 u8NRC_SF_STR; //0 ~ 255;
    MS_U8 u8NRC_TF_STR; //0 ~ 255
    MS_U8 u8NRY_SF_STR; //0 ~ 255
    MS_U8 u8NRY_TF_STR; //0 ~ 255
    MS_U8 u8NRY_BLEND_MOTION_TH; //0 ~ 15
    MS_U8 u8NRY_BLEND_STILL_TH; //0 ~ 15
    MS_U8 u8NRY_BLEND_MOTION_WEI; //0 ~ 16
    MS_U8 u8NRY_BLEND_OTHER_WEI; //0 ~ 16
    MS_U8 u8NRY_BLEND_STILL_WEI; //0 ~ 16
    MS_U8 u8EdgeGain[6];//0~255
    MS_U8 u8Contrast;//0~255
} MHalVpeIqConfig_t;

//-------------------------------------------------
// ROI Defination:
//       AccX[0]AccY[0]   AccX[1]AccY[1]
//       *---------------------*
//       |                     |
//       |                     |
//       *---------------------*
//       AccX[2]AccY[2]   AccX[3]AccY[3]
//--------------------------------------------------
typedef struct
{
    MS_BOOL bEnSkip;
    MS_U16 u16RoiAccX[4];
    MS_U16 u16RoiAccY[4];
} MHalVpeIqWdrRoiConfig_t;

typedef struct
{
    MS_U32 u32Y[ROI_WINDOW_MAX];
} MHalVpeIqWdrRoiReport_t;

typedef struct
{
    MS_BOOL bEn;
    MS_U8 u8WinCount;
    MHalVpeIqWdrRoiSrcType_e enPipeSrc;
    MHalVpeIqWdrRoiConfig_t stRoiCfg[ROI_WINDOW_MAX];
} MHalVpeIqWdrRoiHist_t;

typedef struct
{
    MS_U16 u16X;        ///< horizontal starting position
    MS_U16 u16Y;        ///< vertical starting position
    MS_U16 u16Width;    ///< horizontal size
    MS_U16 u16Height;   ///< vertical size
}HalSclCropWindowConfig_t;

typedef struct
{
    MS_BOOL bCropEn;           ///< the control flag of Crop on/off
    HalSclCropWindowConfig_t stCropWin;   ///< crop configuration
} MHalVpeSclCropConfig_t;

typedef struct
{
    MHalVpeDmaPort_e enPort;
    MHalPixelFormat_e      enFormat;
    MHalPixelCompressMode_e enCompress;
} MHalVpeSclDmaConfig_t;

typedef  struct  HalVideoBuffer_s
{
    MS_U64 u64PhyAddr[3];
    MS_U32 u32Stride[3];
    MS_U64 u64MetaInfo;
} MHalVideoBufferInfo_t;

typedef MHalVideoBufferInfo_t MHalVpeIspVideoInfo_t;

typedef struct
{
    MHalVideoBufferInfo_t stBufferInfo;
    MS_BOOL bEn;
} MHalVpeSclOutputPortBufferConfig_t;

typedef struct
{
    MHalVpeSclOutputPortBufferConfig_t stCfg[E_MHAL_DMA_PORT_MAX];
} MHalVpeSclOutputBufferConfig_t;
typedef struct
{
    MHalVpeDmaPort_e enOutPort;
    MHalVpeSclOutputType_e enOutType;
} MHalVpeSclOutputMDwinConfig_t;
typedef struct
{
    MHalVpeDmaPort_e enOutPort;
    MHalPixelFormat_e      enFormat;
    MS_BOOL bEn;
} MHalVpeSclOutputIMIConfig_t;

typedef struct
{
    MHalVpeDmaPort_e enOutPort;
    MS_U16 u16Width;
    MS_U16 u16Height;
} MHalVpeSclOutputSizeConfig_t;

typedef struct
{
    MS_U16 u16Width;
    MS_U16 u16Height;
} MHalVpeSclWinSize_t;
typedef struct
{
    MHhalVpeSclInstance_e enSclInstance;
    MHalVpeSclWinSize_t stMaxWin;
	MHalVpeTopInputMode_e enTopIn;
    MHalSclInputPortMode_e enInputPortMode;
    MHalSclOutputMode_e enOutmode;//scl mux config
} MHalVpeSclCreate_t;

typedef struct
{
    MHalPixelFormat_e  ePixelFormat;
    MHalPixelCompressMode_e eCompressMode;
    MS_U16 u16Width;
    MS_U16 u16Height;
} MHalVpeSclInputSizeConfig_t;

// FOR HW IP DATA Buffer
typedef struct {
    MS_S32 (*alloc)(MS_U8 *pu8Name, MS_U32 size, MS_PHYADDR * phyAddr);
    MS_S32 (*free)(MS_PHYADDR u64PhyAddr);
    void * (*map)(MS_PHYADDR u64PhyAddr, MS_U32 u32Size , MS_BOOL bCache);
    void   (*unmap)(void *pVirtAddr);
    MS_S32 (*flush_cache)(void *pVirtAddr, MS_U32 u32Size);
} MHalAllocPhyMem_t;

typedef struct
{
    MS_U16  nbFrmBufDvr : 1,
            nbFrmBufCam : 1,
            nbRealTime  : 1,
            nReserved : 13;
} MHalVpeGetCap_t;

typedef void ( *MhalInterruptCb ) (void);

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

//=============================================================================
// API
//=============================================================================
#ifndef __HAL_VPE_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
// Driver Physical memory: MI
// MI init call
INTERFACE MS_BOOL MHalVpeInit(const MHalAllocPhyMem_t *pstAlloc ,MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo);
//MI deinit call
INTERFACE MS_BOOL MHalVpeDeInit(void);
//IQ
INTERFACE MS_BOOL MHalVpeCreateIqInstance(const MHalAllocPhyMem_t *pstAlloc ,const MHalVpeSclWinSize_t *pstMaxWin, MHalVpeTopInputMode_e eMode ,void **pCtx);
INTERFACE MS_BOOL MHalVpeDestroyIqInstance(void *pCtx);
INTERFACE MS_BOOL MHalVpeIqProcess(void *pCtx, const MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo);
INTERFACE MS_BOOL MHalVpeIqDbgLevel(void *p);

INTERFACE MS_BOOL MHalVpeIqConfig(void *pCtx, const MHalVpeIqConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeIqOnOff(void *pCtx, const MHalVpeIqOnOff_t *pCfg);
INTERFACE MS_BOOL MHalVpeIqGetWdrRoiHist(void *pCtx, MHalVpeIqWdrRoiReport_t * pstRoiReport);
INTERFACE MS_BOOL MHalVpeIqSetWdrRoiHist(void *pCtx, const MHalVpeIqWdrRoiHist_t *pCfg);
// SCL in irq bottom: Read 3DNR register
INTERFACE MS_BOOL MHalVpeIqRead3DNRTbl(void *pSclCtx);
// Register write via cmdQ
INTERFACE MS_BOOL MHalVpeIqSetWdrRoiMask(void *pCtx,const MS_BOOL bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo);
// Register write via cmdQ
INTERFACE MS_BOOL MHalVpeIqSetDnrTblMask(void *pCtx,const MS_BOOL bEnMask, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo);

//ROI Buffer

//ISP
INTERFACE MS_BOOL MHalVpeCreateIspInstance(const MHalAllocPhyMem_t *pstAlloc, MHalVpeTopInputMode_e eMode ,void **pCtx);
INTERFACE MS_BOOL MHalVpeDestroyIspInstance(void *pCtx);
INTERFACE MS_BOOL MHalVpeIspProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeIspVideoInfo_t *pstVidInfo);
INTERFACE MS_BOOL MHalVpeIspDbgLevel(void *p);
INTERFACE MS_BOOL MHalVpeIspInputConfig(void *pCtx, const MHalVpeIspInputConfig_t *pCfg);


// SCL

INTERFACE MS_BOOL MHalVpeCreateSclInstance(const MHalAllocPhyMem_t *pstAlloc, const MHalVpeSclCreate_t *stCfg ,void **pCtx);
INTERFACE MS_BOOL MHalVpeDestroySclInstance(void *pCtx);
INTERFACE MS_BOOL MHalVpeSclProcess(void *pCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, const MHalVpeSclOutputBufferConfig_t *pBuffer);
INTERFACE MS_BOOL MHalVpeSclDbgLevel(void *p);
INTERFACE MS_BOOL MHalVpeSclCropConfig(void *pCtx, const MHalVpeSclCropConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeSclDmaPortConfig(void *pCtx, const MHalVpeSclDmaConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeSclInputConfig(void *pCtx, const MHalVpeSclInputSizeConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeSclPortSizeConfig(void *pCtx, const MHalVpeSclOutputSizeConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeSclOutputMDWinConfig(void *pCtx, const MHalVpeSclOutputMDwinConfig_t *pCfg);
INTERFACE MS_BOOL MHalVpeSclImiPortConfig(void *pCtx, const MHalVpeSclOutputIMIConfig_t *pCfg);
// SCL polling MdwinDone
INTERFACE MS_BOOL MHalVpeSclSetWaitDone(void *pSclCtx, MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo, MHalVpeWaitDoneType_e enWait);
INTERFACE MS_BOOL MHalVpeSclRotationConfig(void *pCtx, const MHalVpeSclRotationConfig_t *pCfg);

// SCL vpe irq.
INTERFACE MS_BOOL MHalVpeSclGetIrqNum(unsigned int *pIrqNum,MHalVpeIrqEnableMode_e eMode);
// RIU write register
INTERFACE bool MHalVpeSclEnableIrq(MHalVpeIrqEnableMode_e eMode);
// RIU write register
INTERFACE MS_BOOL MHalVpeSclClearIrq(MHalVpeIrqEnableMode_e eMode);
INTERFACE MHalVpeIrqStatus_e MHalVpeSclCheckIrq(MHalVpeIrqEnableMode_e eMode);


// SCL sw trigger irq by CMDQ or Sc
INTERFACE MS_BOOL MHalVpeSclSetSwTriggerIrq(MHAL_CMDQ_CmdqInterface_t *pstCmdQInfo);

#if 1
INTERFACE MS_BOOL MHalVpeSetIrqCallback(MhalInterruptCb pMhalIntCb);
INTERFACE bool MHalVpeSclEnableIrqForMiddle(MHalVpeIrqEnableMode_e eMode);
#endif
INTERFACE VOID MHalVpeGetCaps(MHalVpeGetCap_t *pstCaps);
#endif //
/** @} */ // end of HAL_VPE_group
