#ifndef _MI_SHADOW_IMPL_H_
#define _MI_SHADOW_IMPL_H_
#include "mi_common.h"
#include "mi_common_internal.h"

#include "mi_shadow_datatype.h"

void MI_SHADOW_IMPL_Init(void);
void MI_SHADOW_IMPL_DeInit(void);
MI_S32 MI_SHADOW_IMPL_RegisterDev(
    MI_SHADOW_ModuleDevInfo_t *pstModDevInfo,
    MI_SHADOW_HANDLE *phShadow);
MI_S32 MI_SHADOW_IMPL_UnRegisterDev(MI_SHADOW_HANDLE hShadow);
MI_S32 MI_SHADOW_IMPL_GetOutputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufConf_t *pstBufConfig,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_BOOL *pbBlockedByRateCtrl,
    MI_SYS_BUF_HANDLE *phBufHandle);
MI_S32 MI_SHADOW_IMPL_GetInputPortBuf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_BufInfo_t *pstBufInfo,
    MI_SYS_BUF_HANDLE *phBufHandle);
MI_S32 MI_SHADOW_IMPL_FinishBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle);
MI_S32 MI_SHADOW_IMPL_RewindBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle);
MI_S32 MI_SHADOW_IMPL_WaitOnInputTaskAvailable(MI_SHADOW_HANDLE hShadow ,MI_S32 u32TimeOutMs);
MI_S32 MI_SHADOW_IMPL_WaitCallBack(
    MI_SHADOW_HANDLE hShadow,
    MI_S32 u32TimeOutMs,
    MI_SYS_ChnPort_t *pstChnCurryPort,
    MI_SYS_ChnPort_t *pstChnPeerPort,
    MI_SHADOW_CALLBACK_EVENT_e *peCallbackEvent);
void MI_SHADOW_IMPL_SetCallbackResult(MI_SHADOW_HANDLE hShadow, MI_S32 s32CallbackResult);
MI_S32 MI_SHADOW_IMPL_SetChannelStatus(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_BOOL bEnable,
    MI_BOOL bChn,
    MI_BOOL bInputPort);
MI_S32 MI_SHADOW_IMPL_SetInputPortBufExtConf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);
MI_S32 MI_SHADOW_IMPL_SetOutputPortBufExtConf(
    MI_SHADOW_HANDLE hShadow,
    MI_U32 u32ChnId,
    MI_U32 u32PortId,
    MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);

#endif /* _MI_SHADOW_IMPL_H_ */
