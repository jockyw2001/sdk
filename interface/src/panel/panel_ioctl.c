#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_panel_impl.h"
#include "panel_ioctl.h"


static DEFINE_MUTEX(mutex);

static MI_S32 MI_PANEL_IOCTL_Init(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_LinkType_e eLinkType = *(MI_PANEL_LinkType_e*)ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_Init(eLinkType);

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_DeInit(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_DeInit();

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_SetIndex(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_INDEX_e eIndex = *(MI_PANEL_INDEX_e*)ptr;

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetIndex(eIndex);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_GetTotalNum(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_GetTotalNum_t *arg = ptr;
    MI_U32 *pu32TotalNum = &(arg->u32TotalNum);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_GetTotalNum(pu32TotalNum);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetPowerOn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetPowerOn_t *arg = ptr;
    MI_PANEL_PowerConfig_t *pstPowerCfg = &(arg->stPowerCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetPowerOn(pstPowerCfg);

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_GetPowerOn(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_GetPowerOn_t *arg = ptr;
    MI_PANEL_PowerConfig_t *pstPowerCfg = &(arg->stPowerCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_GetPowerOn(pstPowerCfg);

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_SetBackLight(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetBackLight_t *arg = ptr;
    MI_PANEL_BackLightConfig_t *pstBackLightCfg = &(arg->stBackLightCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetBackLight(pstBackLightCfg);

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_GetBackLight(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_GetBackLight_t *arg = ptr;
    MI_PANEL_BackLightConfig_t *pstBackLightCfg = &(arg->stBackLightCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_GetBackLight(pstBackLightCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetBackLightLevel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetBackLightLevel_t *arg = ptr;
    MI_PANEL_BackLightConfig_t *pstBackLightCfg = &(arg->stBackLightCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetBackLightLevel(pstBackLightCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_GetBackLightLevel(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_GetBackLightLevel_t *arg = ptr;
    MI_PANEL_BackLightConfig_t *pstBackLightCfg = &(arg->stBackLightCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_GetBackLightLevel(pstBackLightCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetSscConfig(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetSscConfig_t *arg = ptr;
    MI_PANEL_SscConfig_t *pstSscCfg = &(arg->stSscCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetSscConfig(pstSscCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetMipiDsiConfig(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetMipiDsiConfig_t *arg = ptr;
    MI_PANEL_MipiDsiConfig_t *pstMipiDsiCfg = &(arg->stMipiDsiCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetMipiDsiConfig(pstMipiDsiCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetTimingConifg(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetTimingConfig_t *arg = ptr;
    MI_PANEL_TimingConfig_t *pstTimingCfg = &(arg->stTimingCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetTimingConfig(pstTimingCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetDrvCurrentConfig(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetDrvCurrentConfig_t *arg = ptr;
    MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg = &(arg->stDrvCurrentCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetDrvCurrentConfig(pstDrvCurrentCfg);

    return s32Ret;
}
static MI_S32 MI_PANEL_IOCTL_SetOutputPattern(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetOutputPattern_t *arg = ptr;
    MI_PANEL_TestPatternConfig_t *pstTestPatternCfg = &(arg->stTestPatternCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetOutputPattern(pstTestPatternCfg);

    return s32Ret;
}

static MI_S32 MI_PANEL_IOCTL_SetParamConfig(MI_DEVICE_Context_t *env, void *ptr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_PANEL_SetParamConfig_t *arg = ptr;
    MI_PANEL_ParamConfig_t *pstParamCfg = &(arg->stParamCfg);

    AUTO_LOCK(env, mutex);
    s32Ret = MI_PANEL_IMPL_SetPanelParam(pstParamCfg);

    return s32Ret;
}
static MI_DEVICE_WrapperFunction_t ioctl_table[E_MI_PANEL_CMD_MAX] = {
    [E_MI_PANEL_CMD_INIT] = MI_PANEL_IOCTL_Init,
    [E_MI_PANEL_CMD_DEINIT] = MI_PANEL_IOCTL_DeInit,
    [E_MI_PANEL_CMD_SETINDEX] = MI_PANEL_IOCTL_SetIndex,
    [E_MI_PANEL_CMD_GETTOTALNUM] = MI_PANEL_IOCTL_GetTotalNum,
    [E_MI_PANEL_CMD_SETPOWERON] = MI_PANEL_IOCTL_SetPowerOn,
    [E_MI_PANEL_CMD_GETPOWERON] = MI_PANEL_IOCTL_GetPowerOn,
    [E_MI_PANEL_CMD_SETBACKLIGHT] = MI_PANEL_IOCTL_SetBackLight,
    [E_MI_PANEL_CMD_GETBACKLIGHT] = MI_PANEL_IOCTL_GetBackLight,
    [E_MI_PANEL_CMD_SETBACKLIGHTLEVEL] = MI_PANEL_IOCTL_SetBackLightLevel,
    [E_MI_PANEL_CMD_GETBACKLIGHTLEVEL] = MI_PANEL_IOCTL_GetBackLightLevel,
    [E_MI_PANEL_CMD_SETSSCCONFIG] = MI_PANEL_IOCTL_SetSscConfig,
    [E_MI_PANEL_CMD_SETMIPIDSICONFIG] = MI_PANEL_IOCTL_SetMipiDsiConfig,
    [E_MI_PANEL_CMD_SETTIMINGCONFIG] = MI_PANEL_IOCTL_SetTimingConifg,
    [E_MI_PANEL_CMD_SETDRVCURRENTCONFIG] = MI_PANEL_IOCTL_SetDrvCurrentConfig,
    [E_MI_PANEL_CMD_SETOUTPUTPATTERN] = MI_PANEL_IOCTL_SetOutputPattern,
    [E_MI_PANEL_CMD_SETPARAMCONFIG] = MI_PANEL_IOCTL_SetParamConfig,
};

static void mi_panel_insmod(const char *name){
    MI_PRINT("module [%s] init\n", name);
}

static void mi_panel_process_init(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_panel_process_exit(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mstar");

MI_DEVICE_DEFINE(mi_panel_insmod,
               ioctl_table, E_MI_PANEL_CMD_MAX,
               mi_panel_process_init, mi_panel_process_exit);

