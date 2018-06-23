#ifndef _MI_PANEL_H_
#define _MI_PANEL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"
#include "mi_panel_datatype.h"

MI_S32 MI_PANEL_Init(MI_PANEL_LinkType_e eLinkType);
MI_S32 MI_PANEL_DeInit(void);
MI_S32 MI_PANEL_SetIndex(MI_PANEL_INDEX_e eIndex);
MI_S32 MI_PANEL_GetTotalNum(MI_U32 *pu32TotalNum);
MI_S32 MI_PANEL_SetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg);
MI_S32 MI_PANEL_GetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg);
MI_S32 MI_PANEL_SetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_GetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_SetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_GetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg);
MI_S32 MI_PANEL_SetSscConfig(MI_PANEL_SscConfig_t *pstSscCfg);
MI_S32 MI_PANEL_SetMipiDsiConfig(MI_PANEL_MipiDsiConfig_t *pstMipiDsiCfg);
MI_S32 MI_PANEL_SetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg);
MI_S32 MI_PANEL_SetDrvCurrentConfig(MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg);
MI_S32 MI_PANEL_SetOutputPattern(MI_PANEL_TestPatternConfig_t * pstTestPatternCfg);
MI_S32 MI_PANEL_SetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg);

#ifdef __cplusplus
}
#endif

#endif ///_MI_PANEL_H_
