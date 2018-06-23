#ifndef _ST_VIF_H
#define _ST_VIF_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_vif.h"


typedef struct ST_VIF_PortInfo_s
{
	MI_U32  u32RectX;
	MI_U32  u32RectY;
	MI_U32	u32RectWidth;
	MI_U32	u32RectHeight;
	MI_U32	u32DestWidth;
	MI_U32	u32DestHeight;
} ST_VIF_PortInfo_t;

//vif
MI_S32 ST_Vif_Init(void);
MI_S32 ST_Vif_Exit(void);
//vif dev
MI_S32 ST_Vif_CreateDev(MI_VIF_DEV VifDev);
MI_S32 ST_Vif_DisableDev(MI_VIF_DEV VifDev);

//vif channel not use
// MI_S32 ST_Vif_CreateChannel(MI_VIF_CHN VifChn);

//vif port
MI_S32 ST_Vif_CreatePort(MI_VIF_CHN VifChn, MI_VIF_PORT VifPort, ST_VIF_PortInfo_t *pstPortInfoInfo);
MI_S32 ST_Vif_StartPort(MI_VIF_DEV VifDev, MI_VIF_CHN VifChn, MI_VIF_PORT VifPort);
MI_S32 ST_Vif_StopPort(MI_VIF_CHN VifChn, MI_VIF_PORT VifPort);



#endif //_ST_VPE_H
