#ifndef _MI_VIF_H_
#define _MI_VIF_H_

MI_RESULT MI_VIF_SetDevAttr(MI_U32 u32VifDev, MI_VIF_DevAttr_t *pstDevAttr);
MI_RESULT MI_VIF_GetDevAttr(MI_U32 u32VifDev, MI_VIF_DevAttr_t *pstDevAttr);
MI_RESULT MI_VIF_EnableDev(MI_U32 u32VifDev);
MI_RESULT MI_VIF_DisableDev(MI_U32 u32VifDev);
MI_RESULT MI_VIF_SetChnPortAttr(MI_U32 u32VifChn, MI_U32 u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr);
MI_RESULT MI_VIF_GetChnPortAttr(MI_U32 u32VifChn, MI_U32 u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr);
MI_RESULT MI_VIF_EnableChnPort(MI_U32 u32VifChn, MI_U32 u32ChnPort);
MI_RESULT MI_VIF_DisableChnPort(MI_U32 u32VifChn, MI_U32 u32ChnPort);
MI_RESULT MI_VIF_Query(MI_U32 u32VifChn, MI_VIF_ChnPortStat_t *pstStat);
//MI_RESULT _MI_VIF_FillBuffers();
//MI_RESULT _MI_VIF_FillBuffersDone();

#endif //_MI_VIF_H_
