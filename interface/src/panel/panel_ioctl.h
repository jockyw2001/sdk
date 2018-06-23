#ifndef _PANEL_IOCTL_H_
#define _PANEL_IOCTL_H_
#include <linux/ioctl.h>
#include "mi_panel.h"

typedef enum
{
    E_MI_PANEL_CMD_INIT,
    E_MI_PANEL_CMD_DEINIT,
    E_MI_PANEL_CMD_SETINDEX,
    E_MI_PANEL_CMD_GETTOTALNUM,
    E_MI_PANEL_CMD_SETPOWERON,
    E_MI_PANEL_CMD_GETPOWERON,
    E_MI_PANEL_CMD_SETBACKLIGHT,
    E_MI_PANEL_CMD_GETBACKLIGHT,
    E_MI_PANEL_CMD_SETBACKLIGHTLEVEL,
    E_MI_PANEL_CMD_GETBACKLIGHTLEVEL,
    E_MI_PANEL_CMD_SETSSCCONFIG,
    E_MI_PANEL_CMD_SETMIPIDSICONFIG,
    E_MI_PANEL_CMD_SETTIMINGCONFIG,
    E_MI_PANEL_CMD_SETDRVCURRENTCONFIG,
    E_MI_PANEL_CMD_SETOUTPUTPATTERN,
    E_MI_PANEL_CMD_SETPARAMCONFIG,
    E_MI_PANEL_CMD_MAX
}MI_PANEL_Cmd_e;

typedef struct MI_PANEL_GetTotalNum_s
{
    MI_U32 u32TotalNum;
}MI_PANEL_GetTotalNum_t;

typedef struct MI_PANEL_SetPowerOn_s
{
    MI_PANEL_PowerConfig_t stPowerCfg;
}MI_PANEL_SetPowerOn_t;

typedef struct MI_PANEL_GetPowerOn_s
{
    MI_PANEL_PowerConfig_t stPowerCfg;
}MI_PANEL_GetPowerOn_t;

typedef struct MI_PANEL_SetBackLight_s
{
    MI_PANEL_BackLightConfig_t stBackLightCfg;
}MI_PANEL_SetBackLight_t;

typedef struct MI_PANEL_GetBackLight_s
{
    MI_PANEL_BackLightConfig_t stBackLightCfg;
}MI_PANEL_GetBackLight_t;

typedef struct MI_PANEL_SetBackLightLevel_s
{
    MI_PANEL_BackLightConfig_t stBackLightCfg;
}MI_PANEL_SetBackLightLevel_t;

typedef struct MI_PANEL_GetBackLightLevel_s
{
    MI_PANEL_BackLightConfig_t stBackLightCfg;
}MI_PANEL_GetBackLightLevel_t;

typedef struct MI_PAENL_SetSscConfig_s
{
    MI_PANEL_SscConfig_t stSscCfg;
}MI_PANEL_SetSscConfig_t;

typedef struct MI_PANEL_SetMipiDsiConfig_s
{
    MI_PANEL_MipiDsiConfig_t stMipiDsiCfg;
}MI_PANEL_SetMipiDsiConfig_t;

typedef struct MI_PANEL_SetTimingConfig_s
{
    MI_PANEL_TimingConfig_t stTimingCfg;
}MI_PANEL_SetTimingConfig_t;

typedef struct MI_PANEL_SetDrvCurrentConfig_s
{
    MI_PANEL_DrvCurrentConfig_t stDrvCurrentCfg;
}MI_PANEL_SetDrvCurrentConfig_t;

typedef struct MI_PANEL_SetOutputPattern_s
{
    MI_PANEL_TestPatternConfig_t stTestPatternCfg;
}MI_PANEL_SetOutputPattern_t;

typedef struct MI_PANEL_SetParamConfig_s
{
    MI_PANEL_ParamConfig_t stParamCfg;
}MI_PANEL_SetParamConfig_t;

#define MI_PANEL_INIT _IOW('i', E_MI_PANEL_CMD_INIT, MI_PANEL_LinkType_e)
#define MI_PANEL_DEINIT _IO('i', E_MI_PANEL_CMD_DEINIT)
#define MI_PANEL_SETINDEX _IOW('i', E_MI_PANEL_CMD_SETINDEX, MI_PANEL_INDEX_e)
#define MI_PANEL_GETTOTALNUM _IOWR('i', E_MI_PANEL_CMD_GETTOTALNUM, MI_PANEL_GetTotalNum_t)
#define MI_PANEL_SETPOWERON _IOW('i', E_MI_PANEL_CMD_SETPOWERON, MI_PANEL_SetPowerOn_t)
#define MI_PANEL_GETPOWERON _IOWR('i', E_MI_PANEL_CMD_GETPOWERON, MI_PANEL_GetPowerOn_t)
#define MI_PANEL_SETBACKLIGHT _IOW('i', E_MI_PAENL_CMD_SETBACKLIGHT, MI_PANEL_SetBackLight_t)
#define MI_PANEL_GETBACKLIGHT _IOWR('i', E_MI_PANEL_CMD_GETBACKLIGHT, MI_PANEL_GetBackLight_t)
#define MI_PANEL_SETBACKLIGHTLEVEL _IOW('i', E_MI_PANEL_CMD_SETBACKLIGHTLEVEL, MI_PANEL_SetBackLightLevel_t)
#define MI_PANEL_GETBACKLIGHTLEVEL _IOWR('i', E_MI_PANEL_CMD_GETBACKLIGHTLEVEL, MI_PANEL_GetBackLightLevel_t)
#define MI_PANEL_SETSSCCONFIG _IOW('i', E_MI_PANEL_CMD_SETSSCCONFIG, MI_PANEL_SetSscConfig_t)
#define MI_PANEL_SETMIPIDSICONFIG _IOW('i', E_MI_PANEL_CMD_SETMIPIDSICONFIG, MI_PANEL_SetMipiDsiConfig_t)
#define MI_PANEL_SETTIMINGCONFIG _IOW('i', E_MI_PANEL_CMD_SETTIMINGCONFIG, MI_PANEL_SetTimingConfig_t)
#define MI_PANEL_SETDRVCURRENTCONFIG _IOW('i', E_MI_PANEL_CMD_SETDRVCURRENTCONFIG, MI_PANEL_SetDrvCurrentConfig_t)
#define MI_PANEL_SETOUTPUTPATTERN _IOW('i', E_MI_PANEL_CMD_SETOUTPUTPATTERN, MI_PANEL_SetOutputPattern_t)
#define MI_PANEL_SETPARAMCONFIG _IOW('i', E_MI_PANEL_CMD_SETPARAMCONFIG, MI_PANEL_SetParamConfig_t)

#endif //_HDMI_IOCTL_H_


