#include "mi_common.h"

#ifndef _MI_PANEL_IMPL_H_
#define _MI_PANEL_IMPL_H_
#include "mi_panel.h"
#include "mi_panel_datatype.h"

MI_S32 MI_PANEL_IMPL_Init(MI_PANEL_LinkType_e eLinkType);
MI_S32 MI_PANEL_IMPL_DeInit(void);
MI_S32 MI_PANEL_IMPL_SetIndex(MI_PANEL_INDEX_e eIndex);
MI_S32 MI_PANEL_IMPL_GetTotalNum(MI_U32 *pu32TotalNum);
MI_S32 MI_PANEL_IMPL_SetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg);
MI_S32 MI_PANEL_IMPL_GetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg);
MI_S32 MI_PANEL_IMPL_SetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_IMPL_GetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_IMPL_SetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_IMPL_GetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_IMPL_SetSscConfig(MI_PANEL_SscConfig_t *pstSscCfg);
MI_S32 MI_PANEL_IMPL_SetMipiDsiConfig(MI_PANEL_MipiDsiConfig_t *pstMipiDsiCfg);
MI_S32 MI_PANEL_IMPL_SetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg);
MI_S32 MI_PANEL_IMPL_SetDrvCurrentConfig(MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg);
MI_S32 MI_PANEL_IMPL_SetOutputPattern(MI_PANEL_TestPatternConfig_t *pstTestPatternCfg);
MI_S32 MI_PANEL_IMPL_SetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg);
MI_S32 MI_PANEL_IMPL_GetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg);
MI_S32 MI_PANEL_IMPL_GetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg);

#endif

