#ifndef __MI_MD_H__
#define __MI_MD_H__

#include "mi_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef char                    S8;
typedef short                   S16;
typedef int                     S32;

typedef unsigned char           U8;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned long long      U64;
typedef long long               S64;

typedef float                   F32;


typedef struct
{
    // (0: disable, 1: enable)
	U8 enable;
    //(0 ~ 99)
	U8 size_perct_thd_min;
    //(1 ~ 100), must be larger than size_perct_thd_min
	U8 size_perct_thd_max;
    //(10, 20, 30, ..., 100), 100 is the most sensitive
	U8 sensitivity;
    //(1000 ~ 30000)
	U16 learn_rate;
} MDParamsIn_t;

typedef struct
{
	U8 md_result;
    U32 obj_cnt;
} MDParamsOut_t;
typedef void* MD_HANDLE;


U32 MI_MD_GetLibVersion();
MD_HANDLE MI_MD_Init(U16 width, U16 height, U8 color, U8 w_div, U8 h_div);
void MI_MD_Uninit(MD_HANDLE handle);
S32 MI_MD_Run(MD_HANDLE handle, const U8* pImage);
MI_RET MI_MD_SetDetectWindow(MD_HANDLE handle, U16 lt_x, U16 lt_y, U16 rb_x, U16 rb_y, U8 w_div, U8 h_div);
MI_RET MI_MD_GetDetectWindowSize(MD_HANDLE handle, U16* st_x, U16* st_y, U16* div_w, U16* div_h);
MI_RET MI_MD_SetWindowParamsIn(MD_HANDLE handle, U8 w_num, U8 h_num, const MDParamsIn_t* param);
MI_RET MI_MD_GetWindowParamsIn(MD_HANDLE handle, U8 w_num, U8 h_num, MDParamsIn_t* param);
MI_RET MI_MD_GetWindowParamsOut(MD_HANDLE handle, U8 w_num, U8 h_num, MDParamsOut_t* param);
void MI_MD_SetTime(MD_HANDLE handle, U32 time_diff);
void MI_MD_GetYMean(MD_HANDLE handle, U32* mean);


#ifdef __cplusplus
}
#endif

#endif /* __MI_MD_H__ */
