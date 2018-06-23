#ifndef _MI_UVC_H_
#define _MI_UVC_H_
#include "mi_uvc_datatype.h"

#ifdef  MI_INFINITY2_ENABLE
#else
#endif

#include "myvideo.h"

extern int pthread_setname_np(pthread_t __target_thread, 
        const char *__name);

MI_S32 MI_UVC_Init(char *uvc_name);
MI_S32 MI_UVC_Uninit(void);
MI_S32 MI_UVC_CreateDev(MI_UVC_CHANNEL Chn,MI_UVC_PORT PortId,const MI_UVC_ChnAttr_t* pstAttr);
MI_S32 MI_UVC_DestroyDev(MI_UVC_CHANNEL Chn);
MI_S32 MI_UVC_SetChnAttr(MI_UVC_CHANNEL Chn, const MI_UVC_ChnAttr_t* pstAttr);
MI_S32 MI_UVC_GetChnAttr(MI_UVC_CHANNEL Chn, const MI_UVC_ChnAttr_t* pstAttr);
MI_S32 MI_UVC_StartDev(void);
MI_S32 MI_UVC_StopDev(void);

#endif //_MI_UVC_H_

