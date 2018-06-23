#ifndef _ST_FB_H
#define _ST_FB_H

#include "mstarFb.h"

#define ARGB2PIXEL8888(a,r,g,b)	\
	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define PIXEL8888ALPHA(pixelval)	(((pixelval) >> 24) & 0xff)
#define PIXEL8888RED(pixelval)  	(((pixelval) >> 16) & 0xff)
#define PIXEL8888GREEN(pixelval)	(((pixelval) >> 8) & 0xff)
#define PIXEL8888BLUE(pixelval) 	((pixelval) & 0xff)

#define ARGB888_BLACK   ARGB2PIXEL8888(128,0,0,0)
#define ARGB888_RED     ARGB2PIXEL8888(128,255,0,0)
#define ARGB888_GREEN   ARGB2PIXEL8888(128,0,255,0)
#define ARGB888_BLUE    ARGB2PIXEL8888(128,0,0,255)

MI_S32 ST_Fb_Init();
MI_S32 ST_Fb_SetColorKey();
MI_S32 ST_Fb_FillRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal);
MI_S32 ST_Fb_DeInit();
MI_S32 ST_Fb_GetColorKey(MI_U32 *pu32ColorKeyVal);
MI_S32 ST_Fb_InitMouse(MI_S32 s32MousePicW, MI_S32 s32MousePicH, MI_S32 s32BytePerPixel, MI_U8 *pu8MouseFile);
MI_S32 ST_Fb_MouseSet(MI_U32 u32X, MI_U32 u32Y);
void ST_FB_Show(MI_BOOL bShown);
void ST_FB_GetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo);
void ST_FB_SetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo);
void ST_FB_ChangeResolution(int width, int height);

#endif//_ST_FB_H
