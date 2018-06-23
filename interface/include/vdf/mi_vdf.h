#ifndef __MI_VDF_H__
#define __MI_VDF_H__

#include "mi_vdf_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif


MI_S32 MI_VDF_Init(void);
MI_S32 MI_VDF_Uninit(void);
MI_S32 MI_VDF_CreateChn(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr);
MI_S32 MI_VDF_DestroyChn(MI_VDF_CHANNEL VdfChn);
MI_S32 MI_VDF_SetChnAttr(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr);
MI_S32 MI_VDF_GetChnAttr(MI_VDF_CHANNEL VdfChn, MI_VDF_ChnAttr_t* pstAttr);
MI_S32 MI_VDF_EnableSubWindow(MI_VDF_CHANNEL VdfChn, MI_U8 u8Col, MI_U8 u8Row, MI_U8 u8Enable);
MI_S32 MI_VDF_Run(MI_VDF_WorkMode_e enWorkMode);
MI_S32 MI_VDF_Stop(MI_VDF_WorkMode_e enWorkMode);
MI_S32 MI_VDF_GetResult(MI_VDF_CHANNEL VdfChn, MI_VDF_Result_t* pstVdfResult, MI_S32 s32MilliSec);
MI_S32 MI_VDF_PutResult(MI_VDF_CHANNEL VdfChn, MI_VDF_Result_t* pstVdfResult);
MI_S32 MI_VDF_GetLibVersion(MI_VDF_CHANNEL VdfChn, MI_U32* u32VDFVersion);


#ifdef __cplusplus
}
#endif

#endif /* __MI_VDF_H__ */
