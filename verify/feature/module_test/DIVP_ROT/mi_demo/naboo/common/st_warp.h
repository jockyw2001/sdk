#ifndef _ST_WARP_H
#define _ST_WARP_H

#include "mi_warp_datatype.h"

typedef enum
{
    E_WARP_320_240_NV12,
    E_WARP_640_480_NV12,
    E_WARP_1280_720_NV12,
    E_WARP_1920_1080_NV12,
    E_WARP_2560_1440_NV12,
    E_WARP_3840_2160_NV12
}ST_Warp_Timming_e;

MI_S32 ST_Warp_Init(ST_Warp_Timming_e eTimming);
MI_S32 ST_Warp_Exit(void);
MI_S32 ST_Warp_CreateChannel(MI_WARP_CHN warpChn);
MI_S32 ST_Warp_DestroyChannel(MI_WARP_CHN warpChn);

#endif //_ST_VPE_H
