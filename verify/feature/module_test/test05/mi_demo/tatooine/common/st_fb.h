#ifndef _ST_FB_H
#define _ST_FB_H


MI_S32 ST_Fb_Init();
MI_S32 ST_Fb_SetColorKey();
MI_S32 ST_Fb_FillRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal);
MI_S32 ST_Fb_DeInit();
MI_S32 ST_Fb_GetColorKey(MI_U32 *pu32ColorKeyVal);
MI_S32 ST_Fb_InitMouse(MI_S32 s32MousePicW, MI_S32 s32MousePicH, MI_S32 s32BytePerPixel, MI_U8 *pu8MouseFile);
MI_S32 ST_Fb_MouseSet(MI_U32 u32X, MI_U32 u32Y);
void ST_FB_Show(MI_BOOL bShown);

#endif//_ST_FB_H
