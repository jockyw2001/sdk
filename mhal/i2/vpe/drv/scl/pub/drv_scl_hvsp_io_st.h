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
 *  @file drv_scl_hvsp_io_st.h
 *  @brief SclHvsp Driver struct parameter interface
 */

/**
* \ingroup sclhvsp_group
* @{
*/

#ifndef __DRV_SCL_HVSP_IO_ST_H__
#define __DRV_SCL_HVSP_IO_ST_H__

//=============================================================================
// Defines
//=============================================================================
#define DRV_SCLHVSP_VERSION                        0x0100
#define DRV_SCLHVSP_IO_VTRACK_KEY_SETTING_LENGTH       8       ///< DRV_SCLHVSP_IO_VTRACK_KEY_SETTING_LENGTH
#define DRV_SCLHVSP_IO_VTRACK_SETTING_LENGTH           23      ///< DRV_SCLHVSP_IO_VTRACK_SETTING_LENGTH


//=============================================================================
// enum
//=============================================================================

/// @cond
/**
* The ID type of SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_ID_1,      ///< ID_1
    E_DRV_SCLHVSP_IO_ID_2,      ///< ID_2
    E_DRV_SCLHVSP_IO_ID_3,      ///< ID_3
    E_DRV_SCLHVSP_IO_ID_4,      ///< ID_4
    E_DRV_SCLHVSP_IO_ID_NUM,    ///< The max number of ID
} DrvSclHvspIoIdType_e;
/// @endcond

/**
* The input source type of SlcHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_SRC_ISP,       ///< input source: ISP
    E_DRV_SCLHVSP_IO_SRC_BT656,     ///< input source: BT656
    E_DRV_SCLHVSP_IO_SRC_DRAM_LDC,      ///< input source: DRAM,LDC
    E_DRV_SCLHVSP_IO_SRC_DRAM_RSC,      ///< input source: DRAM
    E_DRV_SCLHVSP_IO_SRC_HVSP,      ///< input source: HVSP1
    E_DRV_SCLHVSP_IO_SRC_PAT_TGEN,  ///< input source: PATGEN
    E_DRV_SCLHVSP_IO_SRC_DRAM_ROT,      ///< input source: DRAM,ROT
    E_DRV_SCLHVSP_IO_SRC_NUM,       ///< The max number of input source
} DrvSclHvspIoSrcType_e;

/**
* Thecolor format type of SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_COLOR_RGB,     ///< color format:RGB
    E_DRV_SCLHVSP_IO_COLOR_YUV444,  ///< color format:YUV444
    E_DRV_SCLHVSP_IO_COLOR_YUV422,  ///< color format:YUV422
    E_DRV_SCLHVSP_IO_COLOR_YUV420,  ///< color format:YUV420
    E_DRV_SCLHVSP_IO_COLOR_NUM,     ///< The max number of color format
} DrvSclHvspIoColorType_e;

/**
* The R/W status of MCNR for SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_MCNR_YCM_R   = 0x01,    ///< IP only read
    E_DRV_SCLHVSP_IO_MCNR_YCM_W   = 0x02,    ///< IP only write
    E_DRV_SCLHVSP_IO_MCNR_YCM_RW  = 0x03,    ///< IP R/W
    E_DRV_SCLHVSP_IO_MCNR_CIIR_R  = 0x04,    ///< IP only read
    E_DRV_SCLHVSP_IO_MCNR_CIIR_W  = 0x08,    ///< IP only write
    E_DRV_SCLHVSP_IO_MCNR_CIIR_RW = 0x0C,    ///< IP R/W
    E_DRV_SCLHVSP_IO_MCNR_NON     = 0x10,   ///< IP none open
} DrvSclHvspIoMcnrType_e;


/**
*  The FB control option of SclHvsp in debug mode
*/

/// @cond
/**
* The error type of SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_ERR_OK    =  0, ///< No Error
    E_DRV_SCLHVSP_IO_ERR_FAULT = -1, ///< Fault
    E_DRV_SCLHVSP_IO_ERR_INVAL = -2, ///< Invalid value
    E_DRV_SCLHVSP_IO_ERR_MULTI = -3, ///< MultiInstance Fault
}DrvSclHvspIoErrType_e;
/// @endcond

/**
* The Rotate type of SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_ROTATE_0,      ///< Rotate degree 0
    E_DRV_SCLHVSP_IO_ROTATE_90,     ///< Rotate degree 90
    E_DRV_SCLHVSP_IO_ROTATE_180,    ///< Rotate degree 180
    E_DRV_SCLHVSP_IO_ROTATE_270,    ///< Rotate degree 270
    E_DRV_SCLHVSP_IO_ROTATE_NUM,    ///< The max number of rotate type
}DrvSclHvspIoRotateType_e;

/**
* The output type of SclHvsp
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_OUT_HVSP,      ///< output type: HVSP
    E_DRV_SCLHVSP_IO_OUT_EXTERNAL,  ///< output type: external
    E_DRV_SCLHVSP_IO_OUT_NUM,       ///< The max number of output type.
}DrvSclHvspIoOutputType_e;
//=============================================================================
// struct
//=============================================================================
/**
*  The Version of SclHvsp
*/
typedef struct
{
    u32   VerChk_Version ; ///< structure version
    u32   u32Version;      ///< version
    u32   VerChk_Size;     ///< structure size for version checking
} __attribute__ ((__packed__))DrvSclHvspIoVersionConfig_t;

/**
* The rectangle configuration for Crop Window, Capture Window, Display Window of SclHvsp
*/
typedef struct
{
    u16 u16X;        ///< horizontal starting position
    u16 u16Y;        ///< vertical starting position
    u16 u16Width;    ///< horizontal size
    u16 u16Height;   ///< vertical size
} __attribute__ ((__packed__))DrvSclHvspIoWindowConfig_t;

/**
* The timing configuration of SclHvsp
*/
typedef struct
{
    bool  bInterlace;   ///< be interlace or progressive
    u16 u16Htotal;      ///< horizontal total
    u16 u16Vtotal;      ///< vertical total
    u16 u16Vfrequency;  ///< vertical frequency
} DrvSclHvspIoTimingConfig_t;


/**
*  The input source configuration of SclHvsp
*/
typedef struct
{
    u32   VerChk_Version ;                      ///< structure version
    DrvSclHvspIoSrcType_e        enSrcType;     ///< Input source type
    DrvSclHvspIoColorType_e      enColor;       ///< Input color type
    DrvSclHvspIoWindowConfig_t   stCaptureWin;  ///< Input Source Size(input src is scl capture win
    DrvSclHvspIoTimingConfig_t    stTimingCfg;  ///< Input Timing configuration
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                          ///< structure size for version checking
} __attribute__ ((__packed__))DrvSclHvspIoInputConfig_t;


// struct for for DrvSclHvspIoOutputConfig_t
/**
* The output configuration of SclHvsp
*/
typedef struct
{
    u32   VerChk_Version ;                    ///< structure version
    DrvSclHvspIoColorType_e     enColor;      ///< Output color type
    DrvSclHvspIoWindowConfig_t  stDisplayWin; ///< Output display size
    DrvSclHvspIoTimingConfig_t  stTimingCfg;  ///< Output timing
    DrvSclHvspIoOutputType_e    enOutput;     ///< Output type
    u32   VerChk_Size;                        ///< structure size for version checking
} __attribute__ ((__packed__)) DrvSclHvspIoOutputConfig_t;

/// @cond
/*
* Used to set CLK mux of SclHvsp
*/
typedef struct
{
    void* idclk;  ///< idclk
    void* fclk1;  ///< fclk (SC1 SC2
    void* fclk2;  ///< fclk (SC3
    void* odclk;  ///< odclk (display Lpll
} DrvSclHvspIoClkConfig_t;
/// @endcond

/**
*  The scaling configuration of SclHvsp
*/
typedef struct
{
    u32  VerChk_Version ;   ///< structure version
    u16  u16Src_Width;      ///< horizontal size of input source
    u16  u16Src_Height;     ///< vertical size of input source
    u16  u16Dsp_Width;      ///< horizontal size of output display
    u16  u16Dsp_Height;     ///< vertifcal size of output display
    bool bCropEn;           ///< the control flag of Crop on/off
    DrvSclHvspIoWindowConfig_t stCropWin;   ///< crop configuration
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;      ///< structure size for version checking
} __attribute__ ((__packed__)) DrvSclHvspIoScalingConfig_t;

/**
* The configuration of memory allocated for SclHvsp
*/
typedef struct
{
    u32   VerChk_Version ;  ///< structure version
    u16   u16Vsize;         ///< vertical size of framebuffer
    u16   u16Pitch;         ///< horizontal size of framebuffer
    u32   u32MemSize;       ///< total size of allocated memory, height*width *2(YUV422) *2(2frame)
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;      ///< structure size for version checking
} __attribute__ ((__packed__)) DrvSclHvspIoReqMemConfig_t;

/**
* The configuration of R/W registers for SclHvsp
*/
typedef struct
{
    u8    u8Cmd;      ///< register value
    u32   u32Size;    ///< number
    u32   u32Addr;    ///< bank&addr
} DrvSclHvspIoMiscConfig_t;

/**
* Used to setup the post crop of SclHvsp  if need
*/
typedef struct
{
    u32  VerChk_Version ;   ///< structure version
    bool bCropEn;           ///< post crop En
    u16  u16X;              ///< crop frame start x point
    u16  u16Y;              ///< crop frame start y point
    u16  u16Width;          ///< crop frame width
    u16  u16Height;         ///< crop frame height
    bool bFmCntEn;          ///< Is use CMDQ to set
    u8   u8FmCnt;           ///< when frame count
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;      ///< structure size for version checking
} __attribute__ ((__packed__)) DrvSclHvspIoPostCropConfig_t;

/**
*  The setting information of SclHvsp
*/
typedef struct
{
    u16 u16X;               ///< horizotnal starting position of input source
    u16 u16Y;               ///< vertical starting position of input source
    u16 u16Width;           ///< horizontal size of output display
    u16 u16Height;          ///< vertical size of oputput display
    u16 u16crop2inWidth;    ///< framebuffer width
    u16 u16crop2inHeight;   ///< framebuffer height
    u16 u16crop2OutWidth;   ///< horizotnal size of after crop
    u16 u16crop2OutHeight;  ///< vertical size after crop
} DrvSclHvspIoScInformConfig_t;


/**
* The private id of multi-instance for SclHvsp
*/
typedef struct
{
    s32 s32Id;  ///< private ID
} DrvSclHvspIoPrivateIdConfig_t;


/// @cond
/**
* Used to set CB function of Poll
*/

typedef void (*PollCB) (void *);

typedef struct
{
    u8   u8retval;           ///< return
    u8   u8pollval;          ///< get
    u32  u32Timeout;         ///< u32Timeout
    PollCB *pCBFunc;
}__attribute__ ((__packed__)) DrvSclHvspIoPollConfig_t;
///@endcond

/**
* The configuration of rotate for SCLHVSP
*/
typedef struct
{
    u32   VerChk_Version ;                  ///< structure version
    DrvSclHvspIoRotateType_e enRotateType;  ///< rotate type
    bool  bEn;                              ///< enable/disable
    u32   VerChk_Size;                      ///< structure size for version checking
}__attribute__ ((__packed__)) DrvSclHvspIoRotateConfig_t;

/**
*  The lock configuration of multi-instance for SCLHVSP
*/
typedef struct
{
    u32   VerChk_Version ;  ///< structure version
    s32 *ps32IdBuf;         ///< buffer ID
    u8 u8BufSize;           ///< buffer size
    u32   VerChk_Size;      ///< structure size for version checking
}__attribute__ ((__packed__)) DrvSclHvspIoLockConfig_t;
/**
* Vtrack enable/disalbe type
*/
typedef enum
{
    E_DRV_SCLHVSP_IO_VTRACK_ENABLE_ON,      ///< Vtrack on
    E_DRV_SCLHVSP_IO_VTRACK_ENABLE_OFF,     ///< Vtrack off
    E_DRV_SCLHVSP_IO_VTRACK_ENABLE_DEBUG,   ///< Vtrack debug
}DrvSclHvspIoVtrackEnableType_e;
/**
* Setup Vtrack On/Off configuration
*/
typedef struct
{
    u32   VerChk_Version ;                  ///< structure version
    DrvSclHvspIoVtrackEnableType_e EnType;   ///< On/Off
    u8 u8framerate;                         ///< frame rate
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                      ///< structure size for version checking
}__attribute__ ((__packed__)) DrvSclHvspIoVtrackOnOffConfig_t;
/**
* Setup Vtrack configuration
*/
typedef struct
{
    u32   VerChk_Version ;                                  ///< structure version
    bool bSetKey;                                           ///< key
    bool bSetUserDef;                                       ///< user def
    u8 u8SetKey[DRV_SCLHVSP_IO_VTRACK_KEY_SETTING_LENGTH];   ///< key
    u8 u8SetUserDef[DRV_SCLHVSP_IO_VTRACK_SETTING_LENGTH];   ///< userdef
    u8 u8OperatorID;                                        ///< operator
    u16 u16Timecode;                                        ///< timecode
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    u32   VerChk_Size;                                      ///< structure size for version checking
}__attribute__ ((__packed__)) DrvSclHvspIoVtrackConfig_t;

///@cond
/**
* The configuration of rotate for SCLHVSP
*/
typedef struct
{
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetInputConfig)(s32 s32Handler, DrvSclHvspIoInputConfig_t *pstIoInCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetOutputConfig)(s32 s32Handler, DrvSclHvspIoOutputConfig_t *pstIoOutCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetScalingConfig)(s32 s32Handler, DrvSclHvspIoScalingConfig_t *pstIOSclCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetMiscConfig)(s32 s32Handler, DrvSclHvspIoMiscConfig_t *pstIOMiscCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetPostCropConfig)(s32 s32Handler, DrvSclHvspIoPostCropConfig_t *pstIOPostCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoGetInformConfig)(s32 s32Handler, DrvSclHvspIoScInformConfig_t *pstIOInfoCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetRotateConfig)(s32 s32Handler, DrvSclHvspIoRotateConfig_t *pstIoRotateCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetVtrackConfig)(s32 s32Handler, DrvSclHvspIoVtrackConfig_t *pstCfg);
    DrvSclHvspIoErrType_e (*DrvSclHvspIoSetVtrackOnOffConfig)(s32 s32Handler, DrvSclHvspIoVtrackOnOffConfig_t *pstCfg);
}DrvSclHvspIoFunctionConfig_t;
///@endcond

//=============================================================================

#endif //
/** @} */ // end of sclhvsp_group
