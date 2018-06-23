#ifndef __MI_OD_H__
#define __MI_OD_H__

#include "mi_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _MI_OD_RET_E
{
	MI_OD_RET_SUCCESS						= 0x00000000,	/*OD API execution success*/

    MI_OD_RET_INIT_ERROR                    = 0x10000501,   /*OD init error*/
    MI_OD_RET_IC_CHECK_ERROR                = 0x10000502,   /*Incorrect platform check for OD*/
    MI_OD_RET_INVALID_HANDLE                = 0x10000503,   /*Invalid OD handle*/
    MI_OD_RET_INVALID_PARAMETER             = 0x10000504,   /*Invalid OD parameter*/
    MI_OD_RET_INVALID_WINDOW                = 0x10000505,   /*Invalid window*/
    MI_OD_RET_INVALID_COLOR_TYPE            = 0x10000506,   /*Invalid color tpye*/
} MI_OD_RET;

typedef char                    S8;
typedef short                   S16;
typedef int                     S32;

typedef unsigned char           U8;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned long long      U64;
typedef long long               S64;

typedef float                   F32;




typedef enum _MI_OD_WIN_STATE
{
     MI_OD_WIN_STATE_NON_TAMPER = 0,
     MI_OD_WIN_STATE_TAMPER = 1,
     MI_OD_WIN_STATE_NO_FEATURE = 2,
     MI_OD_WIN_STATE_FAIL = -1,
} MI_OD_WIN_STATE;

typedef enum
{
    OD_Y = 1,
    OD_COLOR_MAX
} ODColor_e;

typedef enum
{
    OD_WINDOW_1X1 = 0,
    OD_WINDOW_2X2,
    OD_WINDOW_3X3,
    OD_WINDOW_MAX
} ODWindow_e;

typedef void* OD_HANDLE;
typedef int   MI_RET;


U32 MI_OD_GetLibVersion();
OD_HANDLE MI_OD_Init(U16 inImgW, U16 inImgH, ODColor_e nClrType, ODWindow_e div);
S32 MI_OD_Run(OD_HANDLE odHandle, const U8 * yImage);
MI_OD_RET MI_OD_SetDetectWindow(OD_HANDLE odHandle, U16 lt_x, U16 lt_y, U16 rb_x, U16 rb_y);
MI_OD_RET MI_OD_GetDetectWindowSize(OD_HANDLE odHandle, U16* st_x, U16* st_y, U16* div_w, U16* div_h);
MI_OD_WIN_STATE	MI_OD_GetWindowResult(OD_HANDLE odHandle, U8 col, U8 row);
MI_RET MI_OD_SetWindowEnable(OD_HANDLE odHandle, U8 col, U8 row, U8 bEnable);
MI_OD_RET MI_OD_GetWindowEnable(OD_HANDLE odHandle, U8 col, U8 row, U8 *en);
MI_RET MI_OD_SetAttr(OD_HANDLE odHandle, S32 thd_tamper, S32 tamper_blk_thd, S32 min_duration, S32 alpha, S32 M);
MI_OD_RET MI_OD_GetAttr(OD_HANDLE odHandle, S32* thd_tamper, S32* tamper_blk_thd, S32* min_duration, S32* alpha, S32* M);
void MI_OD_Uninit(OD_HANDLE odHandle);

#ifdef __cplusplus
}
#endif

#endif //__MI_OD_H__
