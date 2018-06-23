#ifndef _MI_PANEL_DATATYPE_INTERNAL_H_
#define _MI_PANEL_DATATYPE_INTERNAL_H_

#include "mi_panel.h"

typedef struct MI_PANEL_DevContex_s
{
    void *pHalCtx;
    MI_BOOL bSetMipiDsiCfg;
    MI_SYS_DRV_HANDLE hDevSysHandle;
    MI_PANEL_LinkType_e eLinkType;
    MI_PANEL_PowerConfig_t stPowerCfg;
    MI_PANEL_BackLightConfig_t stBackLightCfg;
    MI_PANEL_SscConfig_t stSscCfg;
    MI_PANEL_MipiDsiConfig_t stMipiDsiCfg;
    MI_PANEL_TimingConfig_t stTimingCfg;
    MI_PANEL_DrvCurrentConfig_t stDrvCurrentCfg;
    MI_PANEL_TestPatternConfig_t stTestPatternCfg;
    MI_PANEL_PowerConfig_t stParamCfg;
}MI_PANEL_DevContex_t;

#endif

