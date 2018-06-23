#include "mi_syscall.h"
#include "mi_print.h"
#include <string.h>

#include "mi_panel.h"
#include "panel_ioctl.h"
#include <pthread.h>

MI_MODULE_DEFINE(panel)
    
MI_S32 MI_PANEL_Init(MI_PANEL_LinkType_e eLinkType)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_PANEL_INIT, &eLinkType);

    return s32Ret;
}
MI_S32 MI_PANEL_DeInit(void)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL_VOID(MI_PANEL_DEINIT);

    return s32Ret;
}
MI_S32 MI_PANEL_SetIndex(MI_PANEL_INDEX_e eIndex)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    s32Ret = MI_SYSCALL(MI_PANEL_SETINDEX, &eIndex);

    return s32Ret;
}
MI_S32 MI_PANEL_GetTotalNum(MI_U32 *pu32TotalNum)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_GetTotalNum_t stGetTotalNum; 
    
    s32Ret = MI_SYSCALL(MI_PANEL_GETTOTALNUM, &stGetTotalNum);
    if(s32Ret == MI_SUCCESS)
    {
        *pu32TotalNum = stGetTotalNum.u32TotalNum;
    }

    return s32Ret;
}
MI_S32 MI_PANEL_SetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetPowerOn_t stSetPowerOn;

    memcpy(&stSetPowerOn.stPowerCfg, pstPowerCfg, sizeof(MI_PANEL_PowerConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETPOWERON, &stSetPowerOn);

    return s32Ret;
}
MI_S32 MI_PANEL_GetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_GetPowerOn_t stGetPowerOn;

    s32Ret = MI_SYSCALL(MI_PANEL_GETPOWERON, &stGetPowerOn);
    if(s32Ret == MI_SUCCESS)
    {
        memcpy(pstPowerCfg, &stGetPowerOn.stPowerCfg, sizeof(MI_PANEL_PowerConfig_t));
    }
    
    return s32Ret;
}
MI_S32 MI_PANEL_SetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetBackLight_t stSetBackLight;

    memcpy(&stSetBackLight.stBackLightCfg, pstBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_DEINIT, &stSetBackLight);

    return s32Ret;
}
MI_S32 MI_PANEL_GetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_GetBackLight_t stGetBackLight;

    s32Ret = MI_SYSCALL(MI_PANEL_GETBACKLIGHT, &stGetBackLight);
    if(s32Ret == MI_SUCCESS)
    {
        memcpy(pstBackLightCfg, &stGetBackLight.stBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));
    }
    return s32Ret;
}
MI_S32 MI_PANEL_SetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetBackLightLevel_t stSetBackLightLevel;

    memcpy(&stSetBackLightLevel.stBackLightCfg, pstBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETBACKLIGHTLEVEL, &stSetBackLightLevel);

    return s32Ret;
}
MI_S32 MI_PANEL_GetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_GetBackLightLevel_t stGetBackLightLevel;

    s32Ret = MI_SYSCALL(MI_PANEL_GETBACKLIGHTLEVEL, &stGetBackLightLevel);
    if(s32Ret == MI_SUCCESS)
    {
        memcpy(pstBackLightCfg, &stGetBackLightLevel.stBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));
    }
    return s32Ret;
}
MI_S32 MI_PANEL_SetSscConfig(MI_PANEL_SscConfig_t *pstSscCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetSscConfig_t stSetSscConfig;

    memcpy(&stSetSscConfig.stSscCfg, pstSscCfg, sizeof(MI_PANEL_SscConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETSSCCONFIG, &stSetSscConfig);

    return s32Ret;
}
MI_S32 MI_PANEL_SetMipiDsiConfig(MI_PANEL_MipiDsiConfig_t *pstMipiDsiCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetMipiDsiConfig_t stSetMipiDsiConfig;

    memcpy(&stSetMipiDsiConfig.stMipiDsiCfg, pstMipiDsiCfg, sizeof(MI_PANEL_MipiDsiConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETMIPIDSICONFIG, &stSetMipiDsiConfig);

    return s32Ret;
}
MI_S32 MI_PANEL_SetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetTimingConfig_t stSetTimingConfig;   

    memcpy(&stSetTimingConfig.stTimingCfg, pstTimingCfg, sizeof(MI_PANEL_TimingConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETTIMINGCONFIG, &stSetTimingConfig);

    return s32Ret;
}
MI_S32 MI_PANEL_SetDrvCurrentConfig(MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetDrvCurrentConfig_t stSetDrvCurrentConfig;   

    memcpy(&stSetDrvCurrentConfig.stDrvCurrentCfg, pstDrvCurrentCfg, sizeof(MI_PANEL_DrvCurrentConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETDRVCURRENTCONFIG, &stSetDrvCurrentConfig);

    return s32Ret;
}
MI_S32 MI_PANEL_SetOutputPattern(MI_PANEL_TestPatternConfig_t * pstTestPatternCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetOutputPattern_t stSetOutputPattern;

    memcpy(&stSetOutputPattern.stTestPatternCfg, pstTestPatternCfg, sizeof(MI_PANEL_TestPatternConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETOUTPUTPATTERN, &stSetOutputPattern);

    return s32Ret;
}
MI_S32 MI_PANEL_SetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_PANEL_SetParamConfig_t stSetParamConfig;

    memcpy(&stSetParamConfig.stParamCfg, pstParamCfg, sizeof(MI_PANEL_ParamConfig_t));
    s32Ret = MI_SYSCALL(MI_PANEL_SETPARAMCONFIG, &stSetParamConfig);

    return s32Ret;
}

