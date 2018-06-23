#ifndef __MHAL_AUDIO_DATATYPE_H__

#define __MHAL_AUDIO_DATATYPE_H__
#include "mhal_common.h"

/**
*  @file mhal_audio_datatype.h
*  @brief audio driver data structure
*/

typedef enum
{
    E_MHAL_AUDIO_AI_DEV_MIC,
    E_MHAL_AUDIO_AI_DEV_LINE_IN,
    E_MHAL_AUDIO_AI_DEV_I2S_RX,
    E_MHAL_AUDIO_AI_DEV_BT_I2S_RX,
    E_MHAL_AUDIO_AI_DEV_MAX,

}MHAL_AUDIO_AiDev_e;

typedef enum
{
    E_MHAL_AUDIO_AO_DEV_LINE_OUT,
    E_MHAL_AUDIO_AO_DEV_I2S_TX,
    E_MHAL_AUDIO_AO_DEV_HDMI,
    E_MHAL_AUDIO_AO_DEV_ALL,
    E_MHAL_AUDIO_AO_DEV_MAX,

}MHAL_AUDIO_AoDev_e;

typedef enum
{
    E_MHAL_AUDIO_RATE_8K = 8000,
    E_MHAL_AUDIO_RATE_16K = 16000,
    E_MHAL_AUDIO_RATE_32K = 32000,
    E_MHAL_AUDIO_RATE_48K = 48000,
}MHAL_AUDIO_Rate_e;

typedef enum
{
    E_MHAL_AUDIO_BITWIDTH_16,
    E_MHAL_AUDIO_BITWIDTH_24,
    E_MHAL_AUDIO_BITWIDTH_32,
}MHAL_AUDIO_BitWidth_e;

typedef enum
{
    E_MHAL_AUDIO_I2S_FMT_I2S,
    E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY,
}MHAL_AUDIO_I2sFmt_e;

typedef enum
{
    E_MHAL_AUDIO_MODE_I2S_MASTER,
    E_MHAL_AUDIO_MODE_I2S_SLAVE,
    E_MHAL_AUDIO_MODE_TDM_MASTER
}MHAL_AUDIO_I2sMode_e;


typedef struct MHAL_AUDIO_I2sCfg_s
{
    MHAL_AUDIO_I2sMode_e eMode;
    MHAL_AUDIO_BitWidth_e eWidth;
    MHAL_AUDIO_I2sFmt_e eFmt;
    MS_U16 u16Channels;
}MHAL_AUDIO_I2sCfg_t;

typedef struct MHAL_AUDIO_PcmCfg_s
{
    MHAL_AUDIO_Rate_e eRate;
    MHAL_AUDIO_BitWidth_e eWidth;
    MS_U16 u16Channels;
    MS_BOOL bInterleaved;
    MS_U8 *pu8DmaArea;	/* DMA area */
    MS_PHYADDR phyDmaAddr; //dma buffer size should be nPeriodSize*nPeriodCount
    MS_U32 u32BufferSize;
    MS_U32 u32PeriodSize; //bytes
    MS_U32 u32StartThres;
}MHAL_AUDIO_PcmCfg_t;

typedef struct MHAL_AUDIO_MmapCfg_s
{
    MS_U8     u8Gid;                         // Mmap ID
    MS_U8     u8Layer;                       // Memory Layer
    MS_U8     u8MiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
    MS_U8     u8CMAHid;                      // Memory CMAHID
    MS_U32    u32Addr;                       // Memory Address
    MS_U32    u32Size;                       // Memory Size
    MS_U32    u32Align;                      // Memory Align
    MS_U32    u32MemoryType;                 // Memory Type
}MHAL_AUDIO_MmapCfg_t;

#endif //__MHAL_AUDIO_DATATYPE_H__