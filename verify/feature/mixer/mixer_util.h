#ifndef _MIXER_UTIL_H_
#define _MIXER_UTIL_H_

int mixer_init_hdmi(void);
int mixer_deInit_hdmi(void);
int mixer_init_disp(void);
int mixer_deinit_disp(void);


void mixer_util_set_debug_level(MIXER_DBG_LEVEL_e debug_level);
void mixer_util_set_func_trace(MI_BOOL bTrace);

MI_U32 mixer_util_get_time();
int mixer_write_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata);
int mixer_bind_module(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort , MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate);
#endif // _MIXER_UTIL_H_
