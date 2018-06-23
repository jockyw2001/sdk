///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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
 *  @file drv_scl_m2m_io_st.h
 *  @brief SCLM2M Driver IOCTL parameter interface
 */

 /**
 * \ingroup sclm2m_group
 * @{
 */

#ifndef __DRV_SCL_M2M_IO_ST_H__
#define __DRV_SCL_M2M_IO_ST_H__

//=============================================================================
// Defines
//=============================================================================
// library information
//1.0.1:for clean SCLM2M.
//1.1.1:for add buffer queue handler and swring mode.
//1.1.3:refine for 1.1.1 scl test OK,stabilize not yet.
#define DRV_M2M_VERSION             0x0100 ///< H:Major L:minor H3:Many Change H2:adjust Struct L1:add struct L0:adjust driver

//=============================================================================
// enum
//=============================================================================
/// @cond
/**
* The ID type of M2M
*/
typedef enum
{
    E_DRV_M2M_IO_ID_LDC,
    E_DRV_M2M_IO_ID_ROT,
    E_DRV_M2M_IO_ID_RSC,
    E_DRV_M2M_IO_ID_NUM,
}DrvM2MIoIdType_e;
/// @endcond
typedef struct
{
    MS_U16 u16Width;
    MS_U16 u16Height;
} DrvM2MIoWinSize_t;

/**
* The color format of DMA buffer for  M2M
*/
typedef enum
{
    E_DRV_M2M_IO_COLOR_YUV422,       ///< color format: 422Pack
    E_DRV_M2M_IO_COLOR_YUV420,       ///< color format: YCSep420
    E_DRV_M2M_IO_COLOR_YCSep422,     ///< color format: YC422
    E_DRV_M2M_IO_COLOR_YUVSep422,    ///< color format: YUVSep422
    E_DRV_M2M_IO_COLOR_YUVSep420,    ///< color format: YUVSep420
    E_DRV_M2M_IO_COLOR_NUM,          ///< The max number of color format
}DrvM2MIoColorType_e;

/**
* The memory type of DMA buffer for M2M
*/
typedef enum
{
     E_DRV_M2M_IO_MEM_FRM =0,  ///< memory type: FRM
     E_DRV_M2M_IO_MEM_SNP =1,  ///< memory type: SNP
     E_DRV_M2M_IO_MEM_FRM2 =2, ///< memory type: SNP
     E_DRV_M2M_IO_MEM_IMI =3,  ///< memory type: IMI
     E_DRV_M2M_IO_MEM_FRMR =4,  ///< memory type: IMI
     E_DRV_M2M_IO_MEM_NUM =5,  ///< The max number of memory type
}DrvM2MIoMemType_e;

/**
* The buffer mode of DMA for M2M
*/
typedef enum
{
    E_DRV_M2M_IO_BUFFER_MD_SINGLE, ///< bufer mode : single
    E_DRV_M2M_IO_BUFFER_MD_NUM,    ///< The max number of buffer mode
}DrvM2MIoBufferModeType_e;

/// @cond
typedef enum
{
    E_DRV_M2M_IO_ERR_OK    =  0,     ///< No Error
    E_DRV_M2M_IO_ERR_FAULT = -1,     ///< Fault
    E_DRV_M2M_IO_ERR_INVAL = -2,     ///< Invalid Value
    E_DRV_M2M_IO_ERR_MULTI = -3,     ///< MultiInstance Fault
}DrvM2MIoErrType_e;
/// @endcond

/**
* The external input type of M2M
*/
typedef enum
{
    E_DRV_M2M_OUTPUT_PORT0,      //sc1 frm
    E_DRV_M2M_OUTPUT_PORT1,      //sc2 frm
    E_DRV_M2M_OUTPUT_PORT2,      //sc3 frm
    E_DRV_M2M_OUTPUT_PORT3,      //sc4 frm
    E_DRV_M2M_INPUT_PORT,      //sc3 frmr
    E_DRV_M2M_PORT_MAX,        //
}DrvM2MPort_e;
typedef enum
{
    E_DRV_M2M_IO_SRC_DRAM_LDC,      ///< input source: DRAM,LDC
    E_DRV_M2M_IO_SRC_DRAM_RSC,      ///< input source: DRAM
    E_DRV_M2M_IO_SRC_DRAM_ROT,      ///< input source: DRAM
    E_DRV_M2M_IO_SRC_DRAM_NR,      ///< input source: DRAM
    E_DRV_M2M_IO_SRC_NUM,       ///< The max number of input source
} DrvM2MIoSrcType_e;
typedef enum
{
    E_DRV_M2M_IO_OUTTIMING_SELF,      ///< output type: SELF
    E_DRV_M2M_IO_OUTTIMING_PIPE,  ///< output type: pipe
    E_DRV_M2M_IO_OUTTIMING_NUM,       ///< The max number of output type.
}DrvM2MIoOutputTiming_e;
//=============================================================================
// struct
//=============================================================================
/**
* The rectangle configuration for Crop Window, Capture Window, Display Window of SclHvsp
*/
typedef struct
{
    u16 u16X;        ///< horizontal starting position
    u16 u16Y;        ///< vertical starting position
    u16 u16Width;    ///< horizontal size
    u16 u16Height;   ///< vertical size
} __attribute__ ((__packed__))DrvM2MIoWindowConfig_t;
typedef struct
{
    DrvM2MPort_e enPort;
    DrvM2MIoSrcType_e   enSrc;
    DrvM2MIoOutputTiming_e enTiming;
    DrvM2MIoMemType_e   enMemType;         ///< memory type
    DrvM2MIoColorType_e enColorType;       ///< color type
    DrvM2MIoBufferModeType_e enBufMDType;  ///< buffer mode
    DrvM2MIoWindowConfig_t stCropWin;   ///< crop configuration
    bool     bCropEn;           ///< the control flag of Crop on/off
    u16      u16Width;      ///< width of buffer
    u16      u16Height;     ///< height of buffer
    bool     bHFlip;        ///< bEn HFilp
    bool     bVFlip;        ///< bEn VFilp
}__attribute__ ((__packed__)) DrvM2MIoConfig_t;


/// @cond
typedef void ( *pDrvM2MIoPollCb ) (void);        ///< DrvM2MIoPoll callback function

/**
* The frame buffer configuration for M2M
*/
typedef struct
{
    u8   u8retval;           ///< return
    u8   u8pollval;          ///< get
    u32  u32Timeout;         ///< u32Timeout
    pDrvM2MIoPollCb pfnCb ; ///< callback function
}__attribute__ ((__packed__))DrvM2MIoPollConfig_t;
/// @endcond

/**
*  The lock configuration of multi-instance for M2M
*/
typedef struct
{
    s32 *ps32IdBuf;         ///< buffer ID
    u8 u8BufSize;           ///< buffer size
}__attribute__ ((__packed__)) DrvM2MIoLockConfig_t;
typedef  struct
{
    u64 u64PhyAddr[3];
    u32 u32Stride[3];
} DrvM2MIoOutputBufferConfig_t;
typedef struct
{
    DrvM2MIoOutputBufferConfig_t stBufferInfo;
    bool bEn;
    DrvM2MIoMemType_e   enMemType;             ///< memory type
} DrvM2MIoOutputPortConfig_t;

/**
*  The Process for M2M
*/
typedef struct
{
    DrvM2MIoOutputPortConfig_t stCfg;
    DrvM2MPort_e enPort;
}__attribute__ ((__packed__)) DrvM2MIoProcessConfig_t;


typedef struct
{
    DrvM2MIoErrType_e (*DrvM2MIoSetM2MConfig)(s32 s32Handler, DrvM2MIoConfig_t *pstIoInCfg);
    DrvM2MIoErrType_e (*DrvM2MIoCreateInstConfig)(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg);
    DrvM2MIoErrType_e (*DrvM2MIoDestroyInstConfig)(s32 s32Handler, DrvM2MIoLockConfig_t *pstIoInCfg);
    DrvM2MIoErrType_e (*DrvM2MIoInstProcessConfig)(s32 s32Handler, DrvM2MIoProcessConfig_t *pstIoInCfg);
}DrvM2MIoFunctionConfig_t;

//=============================================================================

#endif //

/** @} */ // end of M2M_group
