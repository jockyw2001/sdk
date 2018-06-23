#ifndef _ST_DIVP_H
#define _ST_DIVP_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_divp.h"


#ifdef __cplusplus
extern "C" {
#endif
#define ST_DIVP_PIP_CHN 8
//divp module
MI_S32 ST_Divp_Init();

//divp channel
MI_S32 ST_Divp_CreatChannel(MI_DIVP_CHN DivpChn, MI_SYS_Rotate_e eRoate, MI_SYS_WindowRect_t *pstCropWin);
MI_S32 ST_Divp_SetOutputAttr(MI_DIVP_CHN DivpChn, MI_SYS_PixelFormat_e eOutPixel, MI_SYS_WindowRect_t *pstOutWin);
MI_S32 ST_Divp_StartChn(MI_DIVP_CHN DivpChn);

MI_S32 ST_Divp_StopChn(MI_DIVP_CHN DivpChn);
MI_S32 ST_Divp_DestroyChn(MI_DIVP_CHN DivpChn);

#ifdef __cplusplus
}
#endif
#endif//_ST_DIVP_H
