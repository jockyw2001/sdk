#ifndef _ST_DIVP_H
#define _ST_DIVP_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"

//divp module
MI_S32 ST_Divp_Init();

//divp channel
MI_S32 ST_Divp_CreateChannel(MI_DIVP_CHN DivpChn);
MI_S32 ST_Divp_DestroyChannel(MI_DIVP_CHN DivpChn);
MI_S32 ST_Divp_StartChn(MI_DIVP_CHN DivpChn);
MI_S32 ST_Divp_StopChn(MI_DIVP_CHN DivpChn);

//divp port
MI_S32 ST_Divp_SetOutputPortAttr(MI_DIVP_CHN DivpChn);
MI_S32 ST_Divp_GetOutputPortAttr(MI_DIVP_CHN DivpChn);

#endif//_ST_DIVP_H
