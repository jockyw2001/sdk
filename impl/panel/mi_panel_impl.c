#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/math64.h>
#include <asm/uaccess.h>

#include "mi_common_datatype.h"
#include "mi_sys_sideband_msg.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_print.h"
#include "mhal_common.h"

#include "mi_panel_impl.h"
#include "mi_panel_datatype.h"
#include "mi_panel_datatype_internal.h"
#include "mhal_pnl.h"

static MI_PANEL_DevContex_t _gstPanelDevCtx;

#define PARSING_PANEL_LINKTYPE(x) ( x == E_MI_PNL_LINK_TTL ? "TTL" : \
                                    x == E_MI_PNL_LINK_LVDS ? "LVDS" : \
                                    x == E_MI_PNL_LINK_RSDS ? "RSDS" : \
                                    x == E_MI_PNL_LINK_MINILVDS ? "MINILVDS" : \
                                    x == E_MI_PNL_LINK_ANALOG_MINILVDS ? "ANALOG_MINILVDS" : \
                                    x == E_MI_PNL_LINK_DIGITAL_MINILVDS ? "DIGITAL_MINILVDS" : \
                                    x == E_MI_PNL_LINK_MFC ? "MFC" : \
                                    x == E_MI_PNL_LINK_DAC_I ? "DAC_I" : \
                                    x == E_MI_PNL_LINK_DAC_P ? "DAC_P" : \
                                    x == E_MI_PNL_LINK_PDPLVDS ? "PDPLVDS" : \
                                    x == E_MI_PNL_LINK_EXT ? "EXT" : \
                                    x == E_MI_PNL_LINK_MIPI_DSI ? "MIPI_DSI" : \
                                                                "UNKNOWN" )

#define PARSING_PANEL_TIBITMODE(x) ( x == E_MI_PNL_TI_10BIT_MODE ? "10BIT " : \
                                     x == E_MI_PNL_TI_8BIT_MODE ? "8BIT" : \
                                     x == E_MI_PNL_TI_6BIT_MODE ? "6BIT" : \
                                                                  "UNKNOWN")

#define PARSING_PANEL_OUTPUTFORMATBITMODE(x) (x == E_MI_PNL_OUTPUT_10BIT_MODE ? "10BIT" : \
                                              x == E_MI_PNL_OUTPUT_6BIT_MODE ? "6BIT" : \
                                              x == E_MI_PNL_OUTPUT_8BIT_MODE ? "8BIT" : \
                                                                               "UNKNOWN")
MhalPnlLinkType_e _MI_PANEL_ConvertToMhalLinkType(MI_PANEL_LinkType_e eLinkType)
{
    MhalPnlLinkType_e eMhalLinkType;

    switch(eLinkType)
    {
        case E_MI_PNL_LINK_TTL:
            eMhalLinkType = E_MHAL_PNL_LINK_TTL;
            break;
        case E_MI_PNL_LINK_LVDS:
            eMhalLinkType = E_MHAL_PNL_LINK_LVDS;
            break;
        case E_MI_PNL_LINK_RSDS:
            eMhalLinkType = E_MHAL_PNL_LINK_RSDS;
            break;
        case E_MI_PNL_LINK_MINILVDS:
            eMhalLinkType = E_MHAL_PNL_LINK_MINILVDS;
            break;
        case E_MI_PNL_LINK_ANALOG_MINILVDS:
            eMhalLinkType = E_MHAL_PNL_LINK_ANALOG_MINILVDS;
            break;
        case E_MI_PNL_LINK_DIGITAL_MINILVDS:
            eMhalLinkType = E_MHAL_PNL_LINK_DIGITAL_MINILVDS;
            break;
        case E_MI_PNL_LINK_MFC:
            eMhalLinkType = E_MHAL_PNL_LINK_MFC;
            break;
        case E_MI_PNL_LINK_DAC_I:
            eMhalLinkType = E_MHAL_PNL_LINK_DAC_I;
            break;
        case E_MI_PNL_LINK_DAC_P:
            eMhalLinkType = E_MHAL_PNL_LINK_DAC_P;
            break;
        case E_MI_PNL_LINK_PDPLVDS:
            eMhalLinkType = E_MHAL_PNL_LINK_PDPLVDS;
            break;
        case E_MI_PNL_LINK_EXT:
            eMhalLinkType = E_MHAL_PNL_LINK_EXT;
            break;
        case E_MI_PNL_LINK_MIPI_DSI:
            eMhalLinkType = E_MHAL_PNL_LINK_MIPI_DSI;
            break;
        default: 
            break;
    }
    return eMhalLinkType;
}

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_PANEL_PROCFS_DEBUG == 1)

static MI_S32 _MI_PANEL_IMPL_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevID, void *pUsrData)
{
    MI_PANEL_ParamConfig_t stParamCfg;

    if(MI_PANEL_IMPL_GetPanelParam(&stParamCfg) != MI_SUCCESS)
    {
        DBG_ERR("dump dev attr--get panel param failed\n");
        return E_MI_ERR_FAILED;
    }
    handle.OnPrintOut(handle, "\n--------------------------------------- PANEL Dev%d Info -------------------------------------\n", u32DevID);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s\n","LVDS_POL","LVDS_CH","LINK_TYPE","TI_MODE");
    handle.OnPrintOut(handle,"%10d%15d%15s%15d\n",stParamCfg.u8SwapLVDS_POL,stParamCfg.u8SwapLVDS_CH,PARSING_PANEL_LINKTYPE(stParamCfg.eLinkType),stParamCfg.u8LVDS_TI_MODE);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s\n","SW_ODD","SW_EVEN","SW_ODD_RB","SW_EVEN_RB");
    handle.OnPrintOut(handle,"%10d%15d%15d%15d\n",stParamCfg.u8SwapOdd_ML,stParamCfg.u8SwapEven_ML,stParamCfg.u8SwapOdd_RB,stParamCfg.u8SwapEven_RB);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","H_Total","V_Total","Width","Height","H_Start","V_Start");
    handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",stParamCfg.u16HTotal,stParamCfg.u16VTotal,stParamCfg.u16Width,stParamCfg.u16Height,stParamCfg.u16HStart,stParamCfg.u16VStart);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","DClk","FrameRate","INV_DCLK","INV_DE","DwpanelMAX","DwpanelMIN");
    handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",stParamCfg.u16DCLK,stParamCfg.u16DCLK/(stParamCfg.u16HTotal*stParamCfg.u16VTotal),stParamCfg.u8InvDCLK,stParamCfg.u8InvDE,stParamCfg.u32MaxSET,stParamCfg.u32MinSET);
    handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s\n","SSC_Enable","SSC_Span","SSC_Step","TI_BIT","Format");
    handle.OnPrintOut(handle,"%10s%15d%15d%15d%15d\n",(_gstPanelDevCtx.stSscCfg.bEn == TRUE ? "enable" : "disable"),stParamCfg.u16SpreadSpectrumSpan,stParamCfg.u16SpreadSpectrumStep,PARSING_PANEL_TIBITMODE(stParamCfg.eTiBitMode),PARSING_PANEL_OUTPUTFORMATBITMODE(stParamCfg.eOutputFormatBitMode));

    if(_gstPanelDevCtx.bSetMipiDsiCfg == TRUE)
    {
        handle.OnPrintOut(handle,"--------------------------------MIPI CONFIG-------------------------------\n");
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","HsTrail","HsPrpr","HsZero","ClkHsPrpr","ClkHsExit","ClkTrail");
        handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",_gstPanelDevCtx.stMipiDsiCfg.u8HsTrail,_gstPanelDevCtx.stMipiDsiCfg.u8HsPrpr,_gstPanelDevCtx.stMipiDsiCfg.u8HsZero,_gstPanelDevCtx.stMipiDsiCfg.u8ClkHsPrpr,_gstPanelDevCtx.stMipiDsiCfg.u8ClkHsExit,_gstPanelDevCtx.stMipiDsiCfg.u8ClkTrail);
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","ClkZero","ClkHsPost","DaHsExit","ContDet","Lpx","TaGet");
        handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",_gstPanelDevCtx.stMipiDsiCfg.u8ClkZero,_gstPanelDevCtx.stMipiDsiCfg.u8ClkHsPost,_gstPanelDevCtx.stMipiDsiCfg.u8DaHsExit,_gstPanelDevCtx.stMipiDsiCfg.u8ContDet,_gstPanelDevCtx.stMipiDsiCfg.u8Lpx,_gstPanelDevCtx.stMipiDsiCfg.u8TaGet);
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","TaSure","TaGo","Hactive","Hpw","Hbp","Hfp");
        handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",_gstPanelDevCtx.stMipiDsiCfg.u8TaSure,_gstPanelDevCtx.stMipiDsiCfg.u8TaGo,_gstPanelDevCtx.stMipiDsiCfg.u16Hactive,_gstPanelDevCtx.stMipiDsiCfg.u16Hpw,_gstPanelDevCtx.stMipiDsiCfg.u16Hbp,_gstPanelDevCtx.stMipiDsiCfg.u16Hfp);
        handle.OnPrintOut(handle,"%10s%15s%15s%15s%15s%15s\n","Vactive","Vpw","Vpb","Vfp","Bllp","Fps");
        handle.OnPrintOut(handle,"%10d%15d%15d%15d%15d%15d\n",_gstPanelDevCtx.stMipiDsiCfg.u16Vactive,_gstPanelDevCtx.stMipiDsiCfg.u16Vpw,_gstPanelDevCtx.stMipiDsiCfg.u16Vbp,_gstPanelDevCtx.stMipiDsiCfg.u16Vfp,_gstPanelDevCtx.stMipiDsiCfg.u16Bllp,_gstPanelDevCtx.stMipiDsiCfg.u16Fps);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_PANEL_IMPL_OnHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32  u32DevId, void *pUsrData)
{
    handle.OnPrintOut(handle,"CatModuleInfo: cat /proc/mi_modules/mi_panel/mi_panel%d\n",u32DevId);
    handle.OnPrintOut(handle,"setoutputpattern: echo setoutputpattern [r] [g] [b] > /proc/mi_modules/mi_panel/mi_panel%d\n",u32DevId);
    return MI_SUCCESS;
}

static MI_S32 _MI_PANEL_IMPL_CmdSetOutputPattern(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_PANEL_TestPatternConfig_t stTestPatternCfg;

    if(argc != 4)
    {
        DBG_ERR("param is Invalid\n");   
        return E_MI_ERR_FAILED;
    }
    stTestPatternCfg.bEn = TRUE;
    stTestPatternCfg.u16R = simple_strtoul(argv[1],NULL,10);
    stTestPatternCfg.u16G = simple_strtoul(argv[2],NULL,10);
    stTestPatternCfg.u16B = simple_strtoul(argv[3],NULL,10);
    MI_PANEL_IMPL_SetOutputPattern(&stTestPatternCfg);

    return MI_SUCCESS;
}

static MI_U32 MI_Moduledev_RegisterDev(void)
{
    mi_sys_ModuleDevBindOps_t stModOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    memset(&stModOps, 0, sizeof(mi_sys_ModuleDevBindOps_t));
    stModOps.OnBindInputPort = NULL;
    stModOps.OnUnBindInputPort = NULL;
    stModOps.OnBindOutputPort = NULL;
    stModOps.OnUnBindOutputPort = NULL;
    stModOps.OnOutputPortBufRelease = NULL;

    memset(&stModInfo, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId = E_MI_MODULE_ID_PANEL;
    stModInfo.u32DevId = 0;
    stModInfo.u32DevChnNum = 0;
    stModInfo.u32InputPortNum = 0;
    stModInfo.u32OutputPortNum = 0;

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_PANEL_IMPL_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;
    pstModuleProcfsOps.OnHelp = _MI_PANEL_IMPL_OnHelp;

    _gstPanelDevCtx.hDevSysHandle = mi_sys_RegisterDev(&stModInfo, &stModOps, NULL, &pstModuleProcfsOps,MI_COMMON_GetSelfDir);
    if(!_gstPanelDevCtx.hDevSysHandle)
    {
        DBG_EXIT_ERR("mi_sys_RegisterDev error.\n");
    }
    DBG_INFO("DevSysHandle = %p\n", _gstPanelDevCtx.hDevSysHandle);

    mi_sys_RegistCommand("setoutputpattern", 3, _MI_PANEL_IMPL_CmdSetOutputPattern, _gstPanelDevCtx.hDevSysHandle);

    return MI_SUCCESS;
}
#endif

MI_S32 MI_PANEL_IMPL_Init(MI_PANEL_LinkType_e eLinkType)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlLinkType_e eMhalLinkType = _MI_PANEL_ConvertToMhalLinkType(eLinkType);

    if(MhalPnlCreateInstance(&_gstPanelDevCtx.pHalCtx, eMhalLinkType) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel init failed\n");
        goto EXIT;
    }
    _gstPanelDevCtx.eLinkType = eLinkType;

#if (defined MI_SYS_PROC_FS_DEBUG) && (MI_PANEL_PROCFS_DEBUG == 1)
    MI_Moduledev_RegisterDev();
#endif

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_DeInit(void)
{
    MI_S32 s32Ret = MI_SUCCESS;

#if defined(MI_SYS_PROC_FS_DEBUG) && (MI_PANEL_PROCFS_DEBUG ==1)
    if(_gstPanelDevCtx.hDevSysHandle)
    {
        mi_sys_UnRegisterDev(_gstPanelDevCtx.hDevSysHandle);
        _gstPanelDevCtx.hDevSysHandle = NULL;
    }
#endif

    if(MhalPnlDestroyInstance(&_gstPanelDevCtx.pHalCtx) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel deinit failed\n");
        goto EXIT;
    }

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetIndex(MI_PANEL_INDEX_e eIndex)
{
    //no support
    return E_MI_ERR_FAILED;
}

MI_S32 MI_PANEL_IMPL_GetTotalNum(MI_U32 *pu32TotalNum)
{
    //no support
    return E_MI_ERR_FAILED;
}

MI_S32 MI_PANEL_IMPL_SetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlPowerConfig_t stMhalPnlPowerCfg;
    
    stMhalPnlPowerCfg.bEn = pstPowerCfg->bEn;
    if(MhalPnlSetPowerConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlPowerCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel set power status failed\n");
        goto EXIT;
    }

    memcpy(&_gstPanelDevCtx.stPowerCfg, pstPowerCfg, sizeof(MI_PANEL_ParamConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_GetPowerOn(MI_PANEL_PowerConfig_t *pstPowerCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlPowerConfig_t stMhalPnlPowerCfg;
    
    if(MhalPnlGetPowerConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlPowerCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        printk("mi panel get power status failed");
        goto EXIT;
    }
    pstPowerCfg->bEn = stMhalPnlPowerCfg.bEn;

EXIT:
    return MI_SUCCESS;
}

MI_S32 MI_PANEL_IMPL_SetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlBackLightOnOffConfig_t stMhalPnlBackLightOnOffCfg; 

    stMhalPnlBackLightOnOffCfg.bEn = pstBackLightCfg->bEn;
    if(MhalPnlSetBackLightOnOffConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlBackLightOnOffCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel setbacklight on/off failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stBackLightCfg, pstBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_GetBackLight(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlBackLightOnOffConfig_t stMhalPnlBackLightOnOffCfg;

    if(MhalPnlGetBackLightOnOffConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlBackLightOnOffCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel getbacklight on/off failed\n");
        goto EXIT;
    }
    pstBackLightCfg->bEn = stMhalPnlBackLightOnOffCfg.bEn;

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlBackLightLevelConfig_t stMhalPnlBackLightLevelCfg;

    stMhalPnlBackLightLevelCfg.u16Level = pstBackLightCfg->u16Level;
    if(MhalPnlSetBackLightLevelConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlBackLightLevelCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel set BackLightlevel failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stBackLightCfg, pstBackLightCfg, sizeof(MI_PANEL_BackLightConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_GetBackLightLevel(MI_PANEL_BackLightConfig_t *pstBackLightCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlBackLightLevelConfig_t stMhalPnlBackLightLevelCfg;

    if(MhalPnlGetBackLightLevelConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlBackLightLevelCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel get BackLightlevel failed\n");
        goto EXIT;
    }
    pstBackLightCfg->u16Level = stMhalPnlBackLightLevelCfg.u16Level;

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetSscConfig(MI_PANEL_SscConfig_t *pstSscCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlSscConfig_t stMhalPnlSscConfig;

    stMhalPnlSscConfig.bEn = pstSscCfg->bEn;
    stMhalPnlSscConfig.u16Step = pstSscCfg->u16Step;
    stMhalPnlSscConfig.u16Span = pstSscCfg->u16Span;
    if(MhalPnlSetSscConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlSscConfig) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel set sscconfig failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stSscCfg, pstSscCfg, sizeof(MI_PANEL_SscConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetMipiDsiConfig(MI_PANEL_MipiDsiConfig_t *pstMipiDsiCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlMipiDsiConfig_t stMhalPnlMipiDsiCfg;

    stMhalPnlMipiDsiCfg.u8HsTrail = pstMipiDsiCfg->u8HsTrail;
    stMhalPnlMipiDsiCfg.u8HsPrpr = pstMipiDsiCfg->u8HsPrpr;
    stMhalPnlMipiDsiCfg.u8HsZero = pstMipiDsiCfg->u8HsZero;
    stMhalPnlMipiDsiCfg.u8ClkHsPrpr = pstMipiDsiCfg->u8ClkHsPrpr;
    stMhalPnlMipiDsiCfg.u8ClkHsExit = pstMipiDsiCfg->u8ClkHsExit;
    stMhalPnlMipiDsiCfg.u8ClkTrail = pstMipiDsiCfg->u8ClkTrail;
    stMhalPnlMipiDsiCfg.u8ClkZero = pstMipiDsiCfg->u8ClkZero;
    stMhalPnlMipiDsiCfg.u8ClkHsPost = pstMipiDsiCfg->u8ClkHsPost;
    stMhalPnlMipiDsiCfg.u8DaHsExit = pstMipiDsiCfg->u8DaHsExit;
    stMhalPnlMipiDsiCfg.u8ContDet = pstMipiDsiCfg->u8ContDet;
    stMhalPnlMipiDsiCfg.u8Lpx = pstMipiDsiCfg->u8Lpx;
    stMhalPnlMipiDsiCfg.u8TaGet = pstMipiDsiCfg->u8TaGet;
    stMhalPnlMipiDsiCfg.u8TaSure = pstMipiDsiCfg->u8TaSure;
    stMhalPnlMipiDsiCfg.u8TaGo = pstMipiDsiCfg->u8TaGo;
    stMhalPnlMipiDsiCfg.u16Hactive = pstMipiDsiCfg->u16Hactive;
    stMhalPnlMipiDsiCfg.u16Hpw = pstMipiDsiCfg->u16Hpw;
    stMhalPnlMipiDsiCfg.u16Hbp = pstMipiDsiCfg->u16Hbp;
    stMhalPnlMipiDsiCfg.u16Hfp = pstMipiDsiCfg->u16Hfp;
    stMhalPnlMipiDsiCfg.u16Vactive = pstMipiDsiCfg->u16Vactive;
    stMhalPnlMipiDsiCfg.u16Vpw = pstMipiDsiCfg->u16Vpw;
    stMhalPnlMipiDsiCfg.u16Vbp = pstMipiDsiCfg->u16Vbp;
    stMhalPnlMipiDsiCfg.u16Vfp = pstMipiDsiCfg->u16Vfp;
    stMhalPnlMipiDsiCfg.u16Bllp = pstMipiDsiCfg->u16Bllp;
    stMhalPnlMipiDsiCfg.u16Fps = pstMipiDsiCfg->u16Fps;
    stMhalPnlMipiDsiCfg.enLaneNum = (pstMipiDsiCfg->enLaneNum == E_MI_PNL_MIPI_DSI_LANE_NONE ? E_MHAL_PNL_MIPI_DSI_LANE_NONE :
                                  pstMipiDsiCfg->enLaneNum == E_MI_PNL_MIPI_DSI_LANE_1 ? E_MHAL_PNL_MIPI_DSI_LANE_1 :
                                  pstMipiDsiCfg->enLaneNum == E_MI_PNL_MIPI_DSI_LANE_2 ? E_MHAL_PNL_MIPI_DSI_LANE_2 :
                                  pstMipiDsiCfg->enLaneNum == E_MI_PNL_MIPI_DSI_LANE_3 ? E_MHAL_PNL_MIPI_DSI_LANE_3 :
                                  E_MHAL_PNL_MIPI_DSI_LANE_4);
    stMhalPnlMipiDsiCfg.enFormat = (pstMipiDsiCfg->enformat == E_MI_PNL_MIPI_DSI_RGB565 ? E_MHAL_PNL_MIPI_DSI_RGB565 :
                                 pstMipiDsiCfg->enformat == E_MI_PNL_MIPI_DSI_RGB666 ? E_MHAL_PNL_MIPI_DSI_RGB666 :
                                 pstMipiDsiCfg->enformat == E_MI_PNL_MIPI_DSI_LOOSELY_RGB666 ? E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666 :
                                 E_MHAL_PNL_MIPI_DSI_RGB888);

    stMhalPnlMipiDsiCfg.enCtrl = (pstMipiDsiCfg->enCtrl == E_MI_PNL_MIPI_DSI_CMD_MODE ? E_MHAL_PNL_MIPI_DSI_CMD_MODE :
                               pstMipiDsiCfg->enCtrl == E_MI_PNL_MIPI_DSI_SYNC_PULSE ? E_MHAL_PNL_MIPI_DSI_SYNC_PULSE :
                               pstMipiDsiCfg->enCtrl == E_MI_PNL_MIPI_DSI_SYNC_EVENT ? E_MHAL_PNL_MIPI_DSI_SYNC_EVENT :
                               E_MHAL_PNL_MIPI_DSI_BURST_MODE);
    if(MhalPnlSetMipiDsiConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlMipiDsiCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel set mipi dsi failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stMipiDsiCfg, pstMipiDsiCfg, sizeof(MI_PANEL_MipiDsiConfig_t));
    _gstPanelDevCtx.bSetMipiDsiCfg = TRUE;

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlTimingConfig_t stMhalPnlTimingCfg;

    stMhalPnlTimingCfg.u16HSyncWidth = pstTimingCfg->u16HSyncWidth;
    stMhalPnlTimingCfg.u16HSyncBackPorch = pstTimingCfg->u16HSyncBackPorch;
    stMhalPnlTimingCfg.u16HSyncFrontPorch = pstTimingCfg->u16HSyncFrontPorch;
    stMhalPnlTimingCfg.u16VSyncWidth = pstTimingCfg->u16VSyncWidth;
    stMhalPnlTimingCfg.u16VSyncBackPorch = pstTimingCfg->u16VSyncBackPorch;
    stMhalPnlTimingCfg.u16VSyncFrontPorch = pstTimingCfg->u16VSyncFrontPorch;
    stMhalPnlTimingCfg.u16HStart = pstTimingCfg->u16HStart;
    stMhalPnlTimingCfg.u16VStart = pstTimingCfg->u16VStart;
    stMhalPnlTimingCfg.u16HActive = pstTimingCfg->u16Width;
    stMhalPnlTimingCfg.u16VActive = pstTimingCfg->u16Height;
    stMhalPnlTimingCfg.u16HTotal = pstTimingCfg->u16HTotal;
    stMhalPnlTimingCfg.u16VTotal = pstTimingCfg->u16VTotal;
    stMhalPnlTimingCfg.u16Dclk = pstTimingCfg->u16Dclk;
    if(MhalPnlSetTimingConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlTimingCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel settimingconfig failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stTimingCfg, pstTimingCfg, sizeof(MI_PANEL_TimingConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetDrvCurrentConfig(MI_PANEL_DrvCurrentConfig_t *pstDrvCurrentCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlDrvCurrentConfig_t stMhalPnlDrvCurrentCfg;

    stMhalPnlDrvCurrentCfg.u16Val = pstDrvCurrentCfg->u16DrvCurrent;
    if(MhalPnlSetDrvCurrentConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlDrvCurrentCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel setdrvcurrentconfig failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stDrvCurrentCfg, pstDrvCurrentCfg, sizeof(MI_PANEL_DrvCurrentConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetOutputPattern(MI_PANEL_TestPatternConfig_t *pstTestPatternCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlTestPatternConfig_t stMhalTestPatternCfg;

    stMhalTestPatternCfg.bEn = pstTestPatternCfg->bEn;
    stMhalTestPatternCfg.u16R = pstTestPatternCfg->u16R;
    stMhalTestPatternCfg.u16G = pstTestPatternCfg->u16G;
    stMhalTestPatternCfg.u16B = pstTestPatternCfg->u16B;
    if(MhalPnlSetTestPatternConfig(_gstPanelDevCtx.pHalCtx, &stMhalTestPatternCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel setoutputpattern failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stTestPatternCfg, pstTestPatternCfg, sizeof(MI_PANEL_TestPatternConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_SetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlParamConfig_t stMhalPnlParamCfg;

    stMhalPnlParamCfg.pPanelName = pstParamCfg->pPanelName;
    stMhalPnlParamCfg.u8Dither = pstParamCfg->u8Dither;
    stMhalPnlParamCfg.eLinkType = (pstParamCfg->eLinkType == E_MI_PNL_LINK_TTL ? E_MHAL_PNL_LINK_TTL:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_LVDS ? E_MHAL_PNL_LINK_LVDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_RSDS ? E_MHAL_PNL_LINK_RSDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_MINILVDS ? E_MHAL_PNL_LINK_MINILVDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_ANALOG_MINILVDS ? E_MHAL_PNL_LINK_ANALOG_MINILVDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_DIGITAL_MINILVDS ? E_MHAL_PNL_LINK_DIGITAL_MINILVDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_MFC ? E_MHAL_PNL_LINK_MFC:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_DAC_I ? E_MHAL_PNL_LINK_DAC_I:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_DAC_P ? E_MHAL_PNL_LINK_DAC_P:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_PDPLVDS ? E_MHAL_PNL_LINK_PDPLVDS:
                                   pstParamCfg->eLinkType == E_MI_PNL_LINK_EXT ? E_MHAL_PNL_LINK_EXT:
                                   E_MHAL_PNL_LINK_MIPI_DSI);
    stMhalPnlParamCfg.u8DualPort = pstParamCfg->u8DualPort;
    stMhalPnlParamCfg.u8SwapPort = pstParamCfg->u8SwapPort;
    stMhalPnlParamCfg.u8SwapOdd_ML = pstParamCfg->u8SwapOdd_ML;
    stMhalPnlParamCfg.u8SwapEven_ML = pstParamCfg->u8SwapEven_ML;
    stMhalPnlParamCfg.u8SwapOdd_RB = pstParamCfg->u8SwapOdd_RB;
    stMhalPnlParamCfg.u8SwapEven_RB = pstParamCfg->u8SwapEven_RB;
    stMhalPnlParamCfg.u8SwapLVDS_POL = pstParamCfg->u8SwapLVDS_POL;
    stMhalPnlParamCfg.u8SwapLVDS_CH = pstParamCfg->u8SwapLVDS_CH;
    stMhalPnlParamCfg.u8PDP10BIT = pstParamCfg->u8PDP10BIT;
    stMhalPnlParamCfg.u8LVDS_TI_MODE = pstParamCfg->u8LVDS_TI_MODE;
    stMhalPnlParamCfg.u8DCLKDelay = pstParamCfg->u8DCLKDelay;
    stMhalPnlParamCfg.u8InvDCLK = pstParamCfg->u8InvDCLK;
    stMhalPnlParamCfg.u8InvDE = pstParamCfg->u8InvDE;
    stMhalPnlParamCfg.u8InvHSync = pstParamCfg->u8InvHSync;
    stMhalPnlParamCfg.u8InvVSync = pstParamCfg->u8InvVSync;
    stMhalPnlParamCfg.u8DCKLCurrent = pstParamCfg->u8DCKLCurrent;
    stMhalPnlParamCfg.u8DECurrent = pstParamCfg->u8DECurrent;
    stMhalPnlParamCfg.u8ODDDataCurrent = pstParamCfg->u8ODDDataCurrent;
    stMhalPnlParamCfg.u8EvenDataCurrent = pstParamCfg->u8EvenDataCurrent;
    stMhalPnlParamCfg.u16OnTiming1 = pstParamCfg->u16OnTiming1;
    stMhalPnlParamCfg.u16OnTiming2 = pstParamCfg->u16OnTiming2;
    stMhalPnlParamCfg.u16OffTiming1 = pstParamCfg->u16OffTiming1;
    stMhalPnlParamCfg.u16OffTiming2 = pstParamCfg->u16OffTiming2;
    stMhalPnlParamCfg.u16HSyncWidth = pstParamCfg->u16HSyncWidth;
    stMhalPnlParamCfg.u16HSyncBackPorch = pstParamCfg->u16HSyncBackPorch;
    stMhalPnlParamCfg.u16VSyncWidth = pstParamCfg->u16VSyncWidth;
    stMhalPnlParamCfg.u16VSyncBackPorch = pstParamCfg->u16VSyncBackPorch;
    stMhalPnlParamCfg.u16HStart = pstParamCfg->u16HStart;
    stMhalPnlParamCfg.u16VStart = pstParamCfg->u16VStart;
    stMhalPnlParamCfg.u16Width = pstParamCfg->u16Width;
    stMhalPnlParamCfg.u16Height = pstParamCfg->u16Height;
    stMhalPnlParamCfg.u16MaxHTotal = pstParamCfg->u16MaxHTotal;
    stMhalPnlParamCfg.u16HTotal = pstParamCfg->u16HTotal;
    stMhalPnlParamCfg.u16MinHTotal = pstParamCfg->u16MinHTotal;
    stMhalPnlParamCfg.u16MaxVTotal = pstParamCfg->u16MaxVTotal;
    stMhalPnlParamCfg.u16VTotal = pstParamCfg->u16VTotal;
    stMhalPnlParamCfg.u16MinVTotal = pstParamCfg->u16MinVTotal;
    stMhalPnlParamCfg.u16MaxDCLK = pstParamCfg->u16MaxDCLK;
    stMhalPnlParamCfg.u16DCLK = pstParamCfg->u16DCLK;
    stMhalPnlParamCfg.u16MinDCLK = pstParamCfg->u16MinDCLK;
    stMhalPnlParamCfg.u16SpreadSpectrumStep = pstParamCfg->u16SpreadSpectrumStep;
    stMhalPnlParamCfg.u16SpreadSpectrumSpan = pstParamCfg->u16SpreadSpectrumSpan;
    stMhalPnlParamCfg.u8DimmingCtl = pstParamCfg->u8DimmingCtl;
    stMhalPnlParamCfg.u8MaxPWMVal = pstParamCfg->u8MaxPWMVal;
    stMhalPnlParamCfg.u8MinPWMVal = pstParamCfg->u8MinPWMVal;
    stMhalPnlParamCfg.u8DeinterMode = pstParamCfg->u8DeinterMode;
    stMhalPnlParamCfg.ePanelAspectRatio = (pstParamCfg->ePanelAspectRatio == E_MI_PNL_ASPECT_RATIO_4_3 ? E_MHAL_PNL_ASPECT_RATIO_4_3:
                                           pstParamCfg->ePanelAspectRatio == E_MI_PNL_ASPECT_RATIO_WIDE ? E_MHAL_PNL_ASPECT_RATIO_WIDE:
                                           E_MHAL_PNL_ASPECT_RATIO_OTHER);
    stMhalPnlParamCfg.u16LVDSTxSwapValue = pstParamCfg->u16LVDSTxSwapValue;
    stMhalPnlParamCfg.eTiBitMode = (pstParamCfg->eTiBitMode == E_MI_PNL_TI_10BIT_MODE ? E_MHAL_PNL_TI_10BIT_MODE:
                                    pstParamCfg->eTiBitMode == E_MI_PNL_TI_8BIT_MODE ? E_MHAL_PNL_TI_8BIT_MODE:
                                    E_MHAL_PNL_TI_6BIT_MODE);
    stMhalPnlParamCfg.eOutputFormatBitMode = (pstParamCfg->eOutputFormatBitMode == E_MI_PNL_OUTPUT_10BIT_MODE ? E_MHAL_PNL_OUTPUT_10BIT_MODE:
                                              pstParamCfg->eOutputFormatBitMode == E_MI_PNL_OUTPUT_6BIT_MODE ? E_MHAL_PNL_OUTPUT_6BIT_MODE:
                                              E_MHAL_PNL_OUTPUT_8BIT_MODE);
    stMhalPnlParamCfg.u8SwapOdd_RG = pstParamCfg->u8SwapOdd_RG;
    stMhalPnlParamCfg.u8SwapEven_RG = pstParamCfg->u8SwapEven_RG;
    stMhalPnlParamCfg.u8SwapOdd_GB = pstParamCfg->u8SwapOdd_GB;
    stMhalPnlParamCfg.u8SwapEven_GB = pstParamCfg->u8SwapEven_GB;
    stMhalPnlParamCfg.u8DoubleClk = pstParamCfg->u8DoubleClk;
    stMhalPnlParamCfg.u32MaxSET = pstParamCfg->u32MaxSET;
    stMhalPnlParamCfg.u32MinSET = pstParamCfg->u32MinSET;
    stMhalPnlParamCfg.eOutTimingMode = (pstParamCfg->eOutTimingMode == E_MI_PNL_CHG_DCLK ? E_MHAL_PNL_CHG_DCLK:
                                        pstParamCfg->eOutTimingMode == E_MI_PNL_CHG_HTOTAL ? E_MHAL_PNL_CHG_HTOTAL:
                                        E_MHAL_PNL_CHG_VTOTAL);
    stMhalPnlParamCfg.u8NoiseDith = pstParamCfg->u8NoiseDith;

    if(MhalPnlSetParamConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlParamCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel set panel param failed\n");
        goto EXIT;
    }
    memcpy(&_gstPanelDevCtx.stParamCfg, pstParamCfg, sizeof(MI_PANEL_ParamConfig_t));

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_GetPanelParam(MI_PANEL_ParamConfig_t *pstParamCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlParamConfig_t stMhalPnlParamCfg;

    if(MhalPnlGetParamConfig(_gstPanelDevCtx.pHalCtx, &stMhalPnlParamCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel get panel param failed\n");
        goto EXIT;
    }
    memcpy((void*)pstParamCfg->pPanelName, stMhalPnlParamCfg.pPanelName, sizeof(pstParamCfg->pPanelName));
    pstParamCfg->u8Dither = stMhalPnlParamCfg.u8Dither;
    pstParamCfg->eLinkType = (stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_TTL ? E_MI_PNL_LINK_TTL:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_LVDS ? E_MI_PNL_LINK_LVDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_RSDS ? E_MI_PNL_LINK_RSDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_MINILVDS ? E_MI_PNL_LINK_MINILVDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_ANALOG_MINILVDS ? E_MI_PNL_LINK_ANALOG_MINILVDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_DIGITAL_MINILVDS ? E_MI_PNL_LINK_DIGITAL_MINILVDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_MFC ? E_MI_PNL_LINK_MFC:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_DAC_I ? E_MI_PNL_LINK_DAC_I:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_DAC_P ? E_MI_PNL_LINK_DAC_P:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_PDPLVDS ? E_MI_PNL_LINK_PDPLVDS:
                              stMhalPnlParamCfg.eLinkType == E_MHAL_PNL_LINK_EXT ? E_MI_PNL_LINK_EXT:
                              E_MI_PNL_LINK_MIPI_DSI);
    pstParamCfg->u8DualPort = stMhalPnlParamCfg.u8DualPort;
    pstParamCfg->u8SwapPort = stMhalPnlParamCfg.u8SwapPort;
    pstParamCfg->u8SwapOdd_ML = stMhalPnlParamCfg.u8SwapOdd_ML;
    pstParamCfg->u8SwapEven_ML = stMhalPnlParamCfg.u8SwapEven_ML;
    pstParamCfg->u8SwapOdd_RB = stMhalPnlParamCfg.u8SwapOdd_RB;
    pstParamCfg->u8SwapEven_RB = stMhalPnlParamCfg.u8SwapEven_RB;
    pstParamCfg->u8SwapLVDS_POL = stMhalPnlParamCfg.u8SwapLVDS_POL;
    pstParamCfg->u8SwapLVDS_CH = stMhalPnlParamCfg.u8SwapLVDS_CH;
    pstParamCfg->u8PDP10BIT = stMhalPnlParamCfg.u8PDP10BIT;
    pstParamCfg->u8LVDS_TI_MODE = stMhalPnlParamCfg.u8LVDS_TI_MODE;
    pstParamCfg->u8DCLKDelay = stMhalPnlParamCfg.u8DCLKDelay;
    pstParamCfg->u8InvDCLK = stMhalPnlParamCfg.u8InvDCLK;
    pstParamCfg->u8InvDE = stMhalPnlParamCfg.u8InvDE;
    pstParamCfg->u8InvHSync = stMhalPnlParamCfg.u8InvHSync;
    pstParamCfg->u8InvVSync = stMhalPnlParamCfg.u8InvVSync;
    pstParamCfg->u8DCKLCurrent = stMhalPnlParamCfg.u8DCKLCurrent;
    pstParamCfg->u8DECurrent = stMhalPnlParamCfg.u8DECurrent;
    pstParamCfg->u8ODDDataCurrent = stMhalPnlParamCfg.u8ODDDataCurrent;
    pstParamCfg->u8EvenDataCurrent = stMhalPnlParamCfg.u8EvenDataCurrent;
    pstParamCfg->u16OnTiming1 = stMhalPnlParamCfg.u16OnTiming1;
    pstParamCfg->u16OnTiming2 = stMhalPnlParamCfg.u16OnTiming2;
    pstParamCfg->u16OffTiming1 = stMhalPnlParamCfg.u16OffTiming1;
    pstParamCfg->u16OffTiming2 = stMhalPnlParamCfg.u16OffTiming2;
    pstParamCfg->u16HSyncWidth = stMhalPnlParamCfg.u16HSyncWidth;
    pstParamCfg->u16HSyncBackPorch = stMhalPnlParamCfg.u16HSyncBackPorch;
    pstParamCfg->u16VSyncWidth = stMhalPnlParamCfg.u16VSyncWidth;
    pstParamCfg->u16VSyncBackPorch = stMhalPnlParamCfg.u16VSyncBackPorch;
    pstParamCfg->u16HStart = stMhalPnlParamCfg.u16HStart;
    pstParamCfg->u16VStart = stMhalPnlParamCfg.u16VStart;
    pstParamCfg->u16Width = stMhalPnlParamCfg.u16Width;
    pstParamCfg->u16Height = stMhalPnlParamCfg.u16Height;
    pstParamCfg->u16MaxHTotal = stMhalPnlParamCfg.u16MaxHTotal;
    pstParamCfg->u16HTotal = stMhalPnlParamCfg.u16HTotal;
    pstParamCfg->u16MinHTotal = stMhalPnlParamCfg.u16MinHTotal;
    pstParamCfg->u16MaxVTotal = stMhalPnlParamCfg.u16MaxVTotal;
    pstParamCfg->u16VTotal = stMhalPnlParamCfg.u16VTotal;
    pstParamCfg->u16MinVTotal = stMhalPnlParamCfg.u16MinVTotal;
    pstParamCfg->u16MaxDCLK = stMhalPnlParamCfg.u16MaxDCLK;
    pstParamCfg->u16DCLK = stMhalPnlParamCfg.u16DCLK;
    pstParamCfg->u16MinDCLK = stMhalPnlParamCfg.u16MinDCLK;
    pstParamCfg->u16SpreadSpectrumStep = stMhalPnlParamCfg.u16SpreadSpectrumStep;
    pstParamCfg->u16SpreadSpectrumSpan = stMhalPnlParamCfg.u16SpreadSpectrumSpan;
    pstParamCfg->u8DimmingCtl = stMhalPnlParamCfg.u8DimmingCtl;
    pstParamCfg->u8MaxPWMVal = stMhalPnlParamCfg.u8MaxPWMVal;
    pstParamCfg->u8MinPWMVal = stMhalPnlParamCfg.u8MinPWMVal;
    pstParamCfg->u8DeinterMode = stMhalPnlParamCfg.u8DeinterMode;
    pstParamCfg->ePanelAspectRatio = (stMhalPnlParamCfg.ePanelAspectRatio == E_MHAL_PNL_ASPECT_RATIO_4_3 ? E_MI_PNL_ASPECT_RATIO_4_3:
                                      stMhalPnlParamCfg.ePanelAspectRatio == E_MHAL_PNL_ASPECT_RATIO_WIDE ? E_MI_PNL_ASPECT_RATIO_WIDE:
                                      E_MI_PNL_ASPECT_RATIO_OTHER);
    pstParamCfg->u16LVDSTxSwapValue = stMhalPnlParamCfg.u16LVDSTxSwapValue;
    pstParamCfg->eTiBitMode = (stMhalPnlParamCfg.eTiBitMode == E_MHAL_PNL_TI_10BIT_MODE ? E_MI_PNL_TI_10BIT_MODE:
                               stMhalPnlParamCfg.eTiBitMode == E_MHAL_PNL_TI_8BIT_MODE ? E_MI_PNL_TI_8BIT_MODE:
                               E_MI_PNL_TI_6BIT_MODE);
    pstParamCfg->eOutputFormatBitMode = (stMhalPnlParamCfg.eOutputFormatBitMode == E_MHAL_PNL_OUTPUT_10BIT_MODE ? E_MI_PNL_OUTPUT_10BIT_MODE:
                                         stMhalPnlParamCfg.eOutputFormatBitMode == E_MHAL_PNL_OUTPUT_6BIT_MODE ? E_MI_PNL_OUTPUT_6BIT_MODE:
                                         E_MI_PNL_OUTPUT_8BIT_MODE);
    pstParamCfg->u8SwapOdd_RG = stMhalPnlParamCfg.u8SwapOdd_RG;
    pstParamCfg->u8SwapEven_RG = stMhalPnlParamCfg.u8SwapEven_RG;
    pstParamCfg->u8SwapOdd_GB = stMhalPnlParamCfg.u8SwapOdd_GB;
    pstParamCfg->u8SwapEven_GB = stMhalPnlParamCfg.u8SwapEven_GB;
    pstParamCfg->u8DoubleClk = stMhalPnlParamCfg.u8DoubleClk;
    pstParamCfg->u32MaxSET = stMhalPnlParamCfg.u32MaxSET;
    pstParamCfg->u32MinSET = stMhalPnlParamCfg.u32MinSET;
    pstParamCfg->eOutTimingMode = (stMhalPnlParamCfg.eOutTimingMode == E_MHAL_PNL_CHG_DCLK ? E_MI_PNL_CHG_DCLK:
                                   stMhalPnlParamCfg.eOutTimingMode == E_MHAL_PNL_CHG_HTOTAL ? E_MI_PNL_CHG_HTOTAL:
                                   E_MI_PNL_CHG_VTOTAL);
    pstParamCfg->u8NoiseDith = stMhalPnlParamCfg.u8NoiseDith;

EXIT:
    return s32Ret;
}

MI_S32 MI_PANEL_IMPL_GetTimingConfig(MI_PANEL_TimingConfig_t *pstTimingCfg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MhalPnlTimingConfig_t stMhalTimingCfg;

    if(MhalPnlGetTimingConfig(_gstPanelDevCtx.pHalCtx, &stMhalTimingCfg) != TRUE)
    {
        s32Ret = E_MI_ERR_FAILED;
        DBG_ERR("mi panel Get timing config failed\n");
        goto EXIT;
    }
    pstTimingCfg->u16HSyncWidth = stMhalTimingCfg.u16HSyncWidth;
    pstTimingCfg->u16HSyncBackPorch = stMhalTimingCfg.u16HSyncBackPorch;
    pstTimingCfg->u16HSyncFrontPorch = stMhalTimingCfg.u16HSyncFrontPorch;
    pstTimingCfg->u16VSyncWidth = stMhalTimingCfg.u16VSyncWidth;
    pstTimingCfg->u16VSyncBackPorch = stMhalTimingCfg.u16VSyncBackPorch;
    pstTimingCfg->u16VSyncFrontPorch = stMhalTimingCfg.u16VSyncFrontPorch;
    pstTimingCfg->u16HStart = stMhalTimingCfg.u16HStart;
    pstTimingCfg->u16VStart = stMhalTimingCfg.u16VStart;
    pstTimingCfg->u16Height = stMhalTimingCfg.u16HActive;
    pstTimingCfg->u16Width = stMhalTimingCfg.u16VActive;
    pstTimingCfg->u16HTotal = stMhalTimingCfg.u16HTotal;
    pstTimingCfg->u16VTotal = stMhalTimingCfg.u16VTotal;
    pstTimingCfg->u16Dclk = stMhalTimingCfg.u16Dclk;

EXIT:
    return MI_SUCCESS;
}

