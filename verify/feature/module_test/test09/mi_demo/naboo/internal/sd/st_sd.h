#ifndef _ST_SD_H
#define _ST_SD_H
#include "mi_sd.h"

typedef struct ST_SD_ChannelInfo_s
{
    MI_U16 u16SDMaxW;
    MI_U16 u16SDMaxH;
    MI_U16 u16SDCropW;
    MI_U16 u16SDCropH;
    MI_S32 u32X;
    MI_S32 u32Y;
} ST_SD_ChannelInfo_t;

typedef struct ST_SD_PortInfo_s
{
    MI_SD_CHANNEL DepSDChannel;
    MI_U16 u16OutputWidth;                         // Width of target image
    MI_U16 u16OutputHeight;                        // Height of target image
    MI_SYS_PixelFormat_e  ePixelFormat;      // Pixel format of target image
    MI_SYS_CompressMode_e eCompressMode;     // Compression mode of the output
} ST_SD_PortInfo_t;

MI_S32 ST_SD_Init(void);
MI_S32 ST_SD_Exit(void);
MI_S32 ST_SD_CreateChannel(MI_SD_CHANNEL SDChannel, ST_SD_ChannelInfo_t *pstChannelInfo);
MI_S32 ST_SD_DestroyChannel(MI_SD_CHANNEL SDChannel);
//MI_S32 ST_SD_StartChannel(MI_SD_CHANNEL SDChannel);
//MI_S32 ST_SD_StopChannel(MI_SD_CHANNEL SDChannel);
MI_S32 ST_SD_CreatePort(ST_SD_PortInfo_t *pstPortInfo);
MI_S32 ST_SD_StopPort(MI_SD_CHANNEL SDChannel, MI_SD_PORT SDPort);
#endif

