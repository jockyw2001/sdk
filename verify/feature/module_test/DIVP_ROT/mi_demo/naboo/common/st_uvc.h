#ifndef _ST_UVC_H_
#define _ST_UVC_H_

#include "st_uvc_datatype.h"

extern int pthread_setname_np(pthread_t __target_thread, 
        const char *__name);

MI_S32 ST_UVC_Init(char *uvc_name);
MI_S32 ST_UVC_Uninit(void);
MI_S32 ST_UVC_CreateDev(const ST_UVC_ChnAttr_t* pstAttr);
MI_S32 ST_UVC_DestroyDev();
MI_S32 ST_UVC_SetChnAttr(const ST_UVC_ChnAttr_t* pstAttr);
MI_S32 ST_UVC_GetChnAttr(const ST_UVC_ChnAttr_t* pstAttr);
MI_S32 ST_UVC_StartDev(void);
MI_S32 ST_UVC_StopDev(void);
void save_file(void *buf,MI_U32 length,char type);
char* uvc_get_format(MI_U32 fcc);
#endif //_ST_UVC_H_
