#ifndef _ST_VDISP_H
#define _ST_VDISP_H

#include "st_common.h"
#include "mi_vdisp_datatype.h"

#define MI_VDISP_DEV_0 0
MI_S32 ST_Vdisp_Init(void);
MI_S32 ST_Vdisp_Exit(void);

MI_S32 ST_Vdisp_SetInputPortAttr(MI_VDISP_DEV DevId, MI_S32 s32InputPort, ST_Rect_t *pstRect);
MI_S32 ST_Vdisp_EnableInputPort(MI_VDISP_DEV DevId,MI_VDISP_PORT PortId);
MI_S32 ST_Vdisp_DisableInputPort(MI_VDISP_DEV DevId,MI_VDISP_PORT PortId);

MI_S32 ST_Vdisp_SetOutputPortAttr(MI_VDISP_DEV DevId, MI_S32 s32OutputPort,
    ST_Rect_t *pstRect, MI_S32 s32FrmRate, MI_S32 s32FrmDepth);
MI_S32 ST_Vdisp_StartDevice(MI_VDISP_DEV DevId);
MI_S32 ST_Vdisp_StopDevice(MI_VDISP_DEV DevId);

#endif//_ST_VDISP_H
