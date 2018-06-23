#ifndef __ENV_H__
#define __ENV_H__

#include "mi_common_datatype.h"

#define DEF_QP (25)
#define MAX_CFG_STR_LEN (64)

#define BUILD_YUV_H (288) //0 for none, 240 for 320x240, 288 for 352x288.


#if BUILD_YUV_H == 288
    #define BUILD_YUV_W (352)
#elif BUILD_YUV_H == 240
    #define BUILD_YUV_W (240)
#endif

void dump_cfg(void);
void env_dump_cfg_2_col(void);
void get_cfg_from_env(void);
int find_cfg_idx(char *szCfg);
MI_S32 get_cfg_int(char *szCfg, MI_BOOL *pbError);
char* get_cfg_str(char *szCfg, MI_BOOL *pbError);
MI_BOOL set_cfg_str(char *szCfg, char *szValue);
void set_result_int(char *szCfg, int value);
#endif
