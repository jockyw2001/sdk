#ifndef _MI_SYSCFG_H_
#define _MI_SYSCFG_H_

#include "mi_syscfg_datatype.h"

MI_BOOL MI_SYSCFG_GetMmapInfo(const char *name, const MI_SYSCFG_MmapInfo_t **ppstMmap);
MI_BOOL MI_SYSCFG_GetMemoryInfo(const MI_SYSCFG_MemoryInfo_t **ppstMem);
MI_U8 MI_SYSCFG_GetDLCTableCount(void);
PanelInfo_t* MI_SYSCFG_GetPanelInfo(EN_DISPLAYTIMING_RES_TYPE eTiming, MI_BOOL bHdmiTx);

void MI_SYSCFG_SetupIniLoader(void);
void MI_SYSCFG_SetupMmapLoader(void);
void MI_SYSCFG_InitCmdqMmapInfo(void);
#endif
