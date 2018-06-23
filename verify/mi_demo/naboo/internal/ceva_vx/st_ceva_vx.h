#ifndef __ST_CEVA_VX_H__
#define __ST_CEVA_VX_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "mi_sys.h"


typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_U32 depth;
    MI_PHY src[1];
    MI_PHY dst[1];
    MI_SYS_PixelFormat_e eFormat;
}ST_CEVA_VX_ImageAttr_t;



MI_S32 ST_CEVA_VX_DspBootUp(const MI_S8 *pDspIniPath);
MI_S32 ST_CEVA_VX_DspShutDown(void);
void ST_CEVA_VX_SetImageAttr(ST_CEVA_VX_ImageAttr_t *pstImageAttr, MI_SYS_ChnPort_t stSinkPort, MI_SYS_ChnPort_t stInjectPort);
MI_S32 ST_CEVA_VX_Start(MI_BOOL bInjectToUsr);
MI_S32 ST_CEVA_VX_Stop(MI_BOOL bInjectToUsr);
MI_U32 ST_CEVA_VX_GetOutputBuf(void *pUsrBuf);
void ST_CEVA_VX_SwitchLog(void);

#endif

