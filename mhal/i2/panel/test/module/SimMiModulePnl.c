#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");



////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>



#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "./inc/ms_platform.h"
#include "./inc/ms_msys.h"
#include "SimMiModuleIoctl.h"
#include "SimMiModuleIoctlDataType.h"


#include "drv_pnl_verchk.h"
#include "drv_pnl_io_st.h"
#include "drv_pnl_io_wrapper.h"

#include "mhal_pnl.h"
#include "mhal_pnl_datatype.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define SIMMI_DBG(dbglv, _fmt, _args...)             \
    do                                             \
    if(dbglv)                                      \
    {                                              \
            printk(KERN_INFO _fmt, ## _args);       \
    }while(0)

#define SIMI_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)


#define SIMMI_ERR(_fmt, _args...)       printk(KERN_ERR _fmt, ## _args)

#define SIMMI_ALIGN(x,y) ( x & ~(y-1))

#define SIMMI_SPRINTF(str, _fmt, _args...) \
    do {                                   \
        char tmpstr[1024];                 \
        sprintf(tmpstr, _fmt, ## _args);   \
        strcat(str, tmpstr);               \
    }while(0)


#define FLAG_DELAY            0xFE
#define FLAG_END_OF_TABLE     0xFF   // END OF REGISTERS MARKER
//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int s32Major;
    int s32Minor;
    int refCnt;
    struct cdev cdev;
    struct file_operations fops;
    struct device *pDev;
}DrvSclSimMiModuleDevice_t;




//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
s32 gs32Handler = -1;
void *gpPnlCtx = NULL;
MhalPnlLinkType_e genLinkType = E_MHAL_PNL_LINK_LVDS;

extern MhalPnlParamConfig_t  tPanel_InnovLux1280x800[];

extern MhalPnlParamConfig_t  tPanel_720x1280_60[];

extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Pulse_Loosely_RGB666;

extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Sync_Event_Loosely_RGB666;

extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Burst_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Burst_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Burst_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_RM68200_720x1280_4Lane_Burst_Loosely_RGB666;


extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Pulse_Loosely_RGB666;

extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Sync_Event_Loosely_RGB666;

extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Burst_RGB888;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Burst_RGB565;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Burst_RGB666;
extern MhalPnlMipiDsiConfig_t tPanel_HX8394_720x1280_2Lane_Burst_Loosely_RGB666;



//-------------------------------------------------------------------------------------------------
// Pirvate Function
//-------------------------------------------------------------------------------------------------
int _SimMiPnlCreateInstance(void)
{
    if(gpPnlCtx == NULL)
    {
       if( MhalPnlCreateInstance(&gpPnlCtx, genLinkType) == 0)
       {
            SIMMI_ERR("%s %d, MhalPnlCreateInstance Fail\n", __FUNCTION__, __LINE__);
            return 0;
       }

       if(gpPnlCtx == NULL)
       {
            SIMMI_ERR("%s %d, PnlCtx is Null \n", __FUNCTION__, __LINE__);
            return 0;
       }
    }

    return 1;
}

int _SimMiPnlDestroyInstance(void)
{
    if(gpPnlCtx)
    {
        if(MhalPnlDestroyInstance(gpPnlCtx) == 0)
        {
            SIMMI_ERR("%s %d, MhalPnlDestroyInstance Fail\n", __FUNCTION__, __LINE__);
            return 0;
        }

        gpPnlCtx = NULL;
    }

    return 1;
}

int _SimMiPnlSetDbgLevel(int u32Level)
{
    DrvPnlIoDbgLevelConfig_t stIoDbgLvCfg;

    stIoDbgLvCfg.u32Level = u32Level;

    _DrvPnlIoSetDbgLevelConfig(0, &stIoDbgLvCfg);

    return 1;
}


int _SimMiPnlBackLightOnOff(int bEn)
{
    MhalPnlBackLightOnOffConfig_t stBackLightOnOffCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stBackLightOnOffCfg.bEn = bEn;

    if(MhalPnlSetBackLightOnOffConfig(gpPnlCtx, &stBackLightOnOffCfg) == 0)
    {
        SIMMI_ERR("%s %d MhalPnlSetBackLightOnOffConfig fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}

int _SimMiPnlBackLightLevel(int u16Level)
{
    MhalPnlBackLightLevelConfig_t stBackLightLevelCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stBackLightLevelCfg.u16Level = u16Level;

    if(MhalPnlSetBackLightLevelConfig(gpPnlCtx, &stBackLightLevelCfg) == 0)
    {
        SIMMI_ERR("%s %d MhalPnlSetBackLightLevelConfig fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}


int _SimMiPnlPower(int bEn)
{
    MhalPnlPowerConfig_t stPowerCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stPowerCfg.bEn = bEn;

    if(MhalPnlSetPowerConfig(gpPnlCtx, &stPowerCfg) == 0)
    {
        SIMMI_ERR("%s %d MhalPnlSetPowerConfig fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}
int _SimMiPnlDrvCurrent(int u16Val)
{
    MhalPnlDrvCurrentConfig_t stDrvCurrentCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }
    stDrvCurrentCfg.u16Val = u16Val;

    if(MhalPnlSetDrvCurrentConfig(gpPnlCtx, &stDrvCurrentCfg) == 0)
    {
        SIMMI_ERR("%s %d MhalPnlSetDrvCurrentConfig fail\n", __FUNCTION__, __LINE__);
        return 0;
    }
    return 1;

}

int _SimMiPnlTestPattern(bool bEn, u16 u16R, u16 u16G, u16 u16B)
{
    MhalPnlTestPatternConfig_t stTestPatCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }
    stTestPatCfg.bEn = bEn;
    stTestPatCfg.u16R = u16R;
    stTestPatCfg.u16G = u16G;
    stTestPatCfg.u16B = u16B;

    if(MhalPnlSetTestPatternConfig(gpPnlCtx, &stTestPatCfg) == 0)
    {
        SIMMI_ERR("%s %d, MhalPnlSetTestPatternConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}


int _SimMiPnlSetSsc(bool bEn, u16 u16Step, u16 u16Span)
{
    MhalPnlSscConfig_t stSscCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stSscCfg.bEn = bEn;
    stSscCfg.u16Step = u16Step;
    stSscCfg.u16Span = u16Span;

    if(MhalPnlSetSscConfig(gpPnlCtx, &stSscCfg) == 0)
    {
        SIMMI_ERR("%s %d, MhalPnlSetSscConfig Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    return 1;
}

int _SimMiPnlSetParam(char *pPnlName)
{
    MhalPnlParamConfig_t stParamCfg;
    bool bPnlFound = 1;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if(strcmp(pPnlName, "innovlux1280x800") == 0)
    {
        memcpy(&stParamCfg, tPanel_InnovLux1280x800, sizeof(MhalPnlParamConfig_t));
    }
    else
    {
        bPnlFound = 0;
    }
    if(bPnlFound)
    {
        if(MhalPnlSetParamConfig(gpPnlCtx, &stParamCfg) == 0)
        {
            SIMMI_DBG(1, "MhalPnlSetParamConfig fail\n");
        }
    }
    else
    {
        SIMMI_ERR("%s %d, Pnl %s Not Found\n", __FUNCTION__, __LINE__, pPnlName);
        return 0;
    }
    return 1;
}

int _SimMiPnlSetMipiDsiCfg(char *pPnlName, char *pCtrl, char *pFmt)
{
    MhalPnlMipiDsiConfig_t *pstMipiDisCfg = NULL;
    MhalPnlParamConfig_t   *pstParamCfg = NULL;
    MhalPnlMipiDsiFormat_e enFormat;
    MhalPnlMipiDsiCtrlMode_e enCtrl;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if(strcmp(pFmt, "rgb565") == 0)
    {
        enFormat = E_MHAL_PNL_MIPI_DSI_RGB565;
    }
    else if(strcmp(pFmt, "rgb666") == 0)
    {
        enFormat = E_MHAL_PNL_MIPI_DSI_RGB666;
    }
    else if(strcmp(pFmt, "loosely_rgb666") == 0)
    {
        enFormat = E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666;
    }
    else
    {
        enFormat = E_MHAL_PNL_MIPI_DSI_RGB888;
    }

    if(strcmp(pCtrl, "sync_event") == 0)
    {
        enCtrl = E_MHAL_PNL_MIPI_DSI_SYNC_EVENT;
    }
    else if(strcmp(pCtrl, "sync_pulse") == 0)
    {
        enCtrl = E_MHAL_PNL_MIPI_DSI_SYNC_PULSE;
    }
    else
    {
        enCtrl = E_MHAL_PNL_MIPI_DSI_BURST_MODE;
    }

    if(strcmp(pPnlName, "rm68200_720x1280_4Lane") == 0)
    {
        pstParamCfg = tPanel_720x1280_60;

        if(enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_PULSE)
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Pulse_RGB888 :
                                                                      &tPanel_RM68200_720x1280_4Lane_Sync_Pulse_Loosely_RGB666;
        }
        else if(enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_EVENT)
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_RM68200_720x1280_4Lane_Sync_Event_RGB888 :
                                                                      &tPanel_RM68200_720x1280_4Lane_Sync_Event_Loosely_RGB666;
        }
        else
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_RM68200_720x1280_4Lane_Burst_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_RM68200_720x1280_4Lane_Burst_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_RM68200_720x1280_4Lane_Burst_RGB888 :
                                                                      &tPanel_RM68200_720x1280_4Lane_Burst_Loosely_RGB666;
        }
    }
    else if(strcmp(pPnlName, "hx8394_720x1280_2Lane") == 0)
    {
        pstParamCfg = tPanel_720x1280_60;
        if(enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_PULSE)
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Pulse_RGB888 :
                                                                      &tPanel_HX8394_720x1280_2Lane_Sync_Pulse_Loosely_RGB666;
        }
        else if(enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_EVENT)
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_HX8394_720x1280_2Lane_Sync_Event_RGB888 :
                                                                      &tPanel_HX8394_720x1280_2Lane_Sync_Event_Loosely_RGB666;
        }
        else
        {
            pstMipiDisCfg = enFormat == E_MHAL_PNL_MIPI_DSI_RGB666 ?  &tPanel_HX8394_720x1280_2Lane_Burst_RGB666 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB565 ?  &tPanel_HX8394_720x1280_2Lane_Burst_RGB565 :
                            enFormat == E_MHAL_PNL_MIPI_DSI_RGB888 ?  &tPanel_HX8394_720x1280_2Lane_Burst_RGB888 :
                                                                      &tPanel_HX8394_720x1280_2Lane_Burst_Loosely_RGB666;
        }
    }
    else
    {
        pstMipiDisCfg = NULL;
        pstParamCfg = NULL;
    }


    if(pstMipiDisCfg && pstParamCfg)
    {
        if(MhalPnlSetParamConfig(gpPnlCtx, pstParamCfg) == 0)
        {
            SIMMI_ERR("%s %d, MhalPnlSetParamConfig fail\n", __FUNCTION__, __LINE__);
        }
        else
        {
            if(MhalPnlSetMipiDsiConfig(gpPnlCtx, pstMipiDisCfg) == 0)
            {
                SIMMI_ERR("%s %d, MhalPnlSetMipiDsiConfig fail\n", __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        SIMMI_ERR("%s %d, Pnl %s Not Found\n", __FUNCTION__, __LINE__, pPnlName);
        return 0;
    }

    return 1;
}

int _SimmiPnlSetMipiDsiTiming(
 u16 u16Hpw, u16 u16Hbp, u16 u16Hfp, u16 u16Hactive,
 u16 u16Vpw, u16 u16Vbp, u16 u16Vfp, u16 u16Vactive
)
{
    MhalPnlTimingConfig_t stTimingCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stTimingCfg.u16HSyncWidth     = u16Hpw;
    stTimingCfg.u16HSyncBackPorch = u16Hbp;
    stTimingCfg.u16HSyncFrontPorch = u16Hfp;
    stTimingCfg.u16HActive        = u16Hactive;

    stTimingCfg.u16VSyncWidth     = u16Vpw;
    stTimingCfg.u16VSyncBackPorch = u16Vbp;
    stTimingCfg.u16VSyncFrontPorch = u16Vfp;
    stTimingCfg.u16VActive        = u16Vactive;

    if( MhalPnlSetTimingConfig(gpPnlCtx, &stTimingCfg) == 0)
    {
        SIMMI_ERR("%s %d, MhalPnlSetTimingConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}

int _SimMiPnlSetLvdsTiming(
    u16 u16Hpw, u16 u16Hbp, u16 u16Hstart, u16 u16Hactive, u16 u16Htt,
    u16 u16Vpw, u16 u16Vbp, u16 u16Vstart, u16 u16Vactive, u16 u16Vtt,
    u16 u16Dclk)
{
    MhalPnlTimingConfig_t stTimingCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    stTimingCfg.u16HSyncWidth     = u16Hpw;
    stTimingCfg.u16HSyncBackPorch = u16Hbp;
    stTimingCfg.u16HActive        = u16Hactive;
    stTimingCfg.u16HStart         = u16Hstart;
    stTimingCfg.u16HTotal         = u16Htt;

    stTimingCfg.u16VSyncWidth     = u16Vpw;
    stTimingCfg.u16VSyncBackPorch = u16Vbp;
    stTimingCfg.u16VActive        = u16Vactive;
    stTimingCfg.u16VStart         = u16Vstart;
    stTimingCfg.u16VTotal         = u16Vtt;
    stTimingCfg.u16Dclk           = u16Dclk;


    if( MhalPnlSetTimingConfig(gpPnlCtx, &stTimingCfg) == 0)
    {
        SIMMI_ERR("%s %d, MhalPnlSetTimingConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}
int _SimMiPnlGetPower(void)
{
    MhalPnlPowerConfig_t stPowerCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if(MhalPnlGetPowerConfig(gpPnlCtx, &stPowerCfg))
    {
        SIMMI_DBG(1, "Power En=%d\n", stPowerCfg.bEn);
    }
    else
    {
        SIMMI_ERR("%s %d, MhalPnlGetPowerConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}

int _SimMiPnlGetTiming(void)
{
    MhalPnlTimingConfig_t stTimingCfg;
    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if(MhalPnlGetTimingConfig(gpPnlCtx, &stTimingCfg))
    {
        SIMMI_DBG(1, "H(%d %d %d %d %d), V(%d %d %d %d %d) DCLK:%d\n",
            stTimingCfg.u16HSyncWidth, stTimingCfg.u16HSyncBackPorch,
            stTimingCfg.u16HStart, stTimingCfg.u16HActive, stTimingCfg.u16HTotal,
            stTimingCfg.u16VSyncWidth, stTimingCfg.u16VSyncBackPorch,
            stTimingCfg.u16VStart, stTimingCfg.u16VActive, stTimingCfg.u16VTotal,
            stTimingCfg.u16Dclk);
    }
    else
    {
        SIMMI_ERR("%s %d, MhalPnlGetTimingConfig Fail\n", __FUNCTION__, __LINE__);
    }
    return 1;
}


int _SimMiPnlGetBackLight(void)
{
    MhalPnlBackLightLevelConfig_t stBackLightLevelCfg;
    MhalPnlBackLightOnOffConfig_t stBackLightOnOffCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if( MhalPnlGetBackLightOnOffConfig(gpPnlCtx, &stBackLightOnOffCfg))
    {
        SIMMI_DBG(1, "BackLight En=%d\n", stBackLightOnOffCfg.bEn);
    }
    else
    {
        SIMMI_ERR("%s %d, MhalPnlGetBackLightOnOffConfig Fail\n", __FUNCTION__, __LINE__);
    }

    if( MhalPnlGetBackLightLevelConfig(gpPnlCtx, &stBackLightLevelCfg))
    {
        SIMMI_DBG(1, "BackLight Level=%04x\n", stBackLightLevelCfg.u16Level);
    }
    else
    {
        SIMMI_ERR("%s %d, MhalPnlGetBackLightLevelConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}


int _SimMiPnlGetParam(void)
{
    MhalPnlParamConfig_t stParamCfg;

    if( _SimMiPnlCreateInstance() == 0)
    {
        SIMMI_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if(MhalPnlGetParamConfig(gpPnlCtx, &stParamCfg))
    {
        SIMMI_DBG(1, "%s, %d H(%d %d %d %d), V(%d %d %d %d) DCLK(%d)\n",
            stParamCfg.pPanelName,
            stParamCfg.eLinkType,
            stParamCfg.u16HSyncWidth, stParamCfg.u16HSyncBackPorch,
            stParamCfg.u16Width, stParamCfg.u16HTotal,
            stParamCfg.u16VSyncWidth, stParamCfg.u16VSyncBackPorch,
            stParamCfg.u16Height, stParamCfg.u16Height,
            stParamCfg.u16DCLK);

    }
    else
    {
        SIMMI_ERR("%s %d, MhalPnlGetParamConfig Fail\n", __FUNCTION__, __LINE__);
    }

    return 1;
}

//==============================================================================
// Parsing SysFs
//==============================================================================
int _SimMiModuleSplit(char **arr, char *str,  char* del)
{
    char *cur = str;
    char *token = NULL;
    int cnt = 0;


    token = strsep(&cur, del);
    while (token)
    {
        cnt++;
        arr[cnt] = token;
        token = strsep(&cur, del);
    }
    return cnt;
}





void _SimMiModulePnlShow(char *buf)
{
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "lvds [PnlName]\n");
    SIMMI_SPRINTF(buf, "mipi_dsi [PnlName] [Ctrl] [Fmt]\n");
    SIMMI_SPRINTF(buf, "test_pat [bEn] [R] [G] [B]\n");
    SIMMI_SPRINTF(buf, "ssc [En] [step] [span] \n");
    SIMMI_SPRINTF(buf, "power [En]\n");
    SIMMI_SPRINTF(buf, "backlight_en [En]\n");
    SIMMI_SPRINTF(buf, "backlight_lv [Level]\n");
    SIMMI_SPRINTF(buf, "timing\n");
    SIMMI_SPRINTF(buf, "  LVDS,    [Hpw] [HBp] [Hstart] [Hactive] [Htt] [Vpw] [VBp] [Vstart] [Vactive] [Vtt] [DCLK]\n");
    SIMMI_SPRINTF(buf, "  MIPI_DSI [Hactive] [Hpw] [HBp] [HFp] [VActive] [Vpw] [VBp] [VFp] \n");
    SIMMI_SPRINTF(buf, "drv_cur [VAL]");
    SIMMI_SPRINTF(buf, "exit\n");
}

void _SimMiModulePnlStore(char *buf)
{
    char *argv[100];
    int argc;
    char del[] = " ";
    int len;
    int ret;

    argc = _SimMiModuleSplit(argv, buf, del);
    len = strlen(argv[argc]);
    argv[argc][len-1] = '\0';

    if(argc >= 1)
    {
        if(strcmp(argv[1], "exit") == 0)
        {
            _SimMiPnlDestroyInstance();
        }
        else if(strcmp(argv[1], "lvds") == 0)
        {
            if(argc == 2)
            {
                genLinkType = E_MHAL_PNL_LINK_LVDS;
                _SimMiPnlSetParam(argv[2]);
            }
        }
        else if(strcmp(argv[1], "mipi_dsi") == 0)
        {
            genLinkType = E_MHAL_PNL_LINK_MIPI_DSI;
            if(argc == 2)
            {
                _SimMiPnlSetMipiDsiCfg(argv[2], "sync_pulse", "rgb888");
            }
            else if(argc == 4)
            {
                _SimMiPnlSetMipiDsiCfg(argv[2], argv[3], argv[4]);
                SIMMI_DBG(1, "[PnlName] [Ctrl] [Fmt]\n");
            }

        }
        else if(strcmp(argv[1], "ssc") == 0)
        {
            if(argc == 4)
            {
                int bEn;
                int u16Step;
                int u16Span;

                ret = kstrtol(argv[2], 16, (long *)&bEn);
                ret = kstrtol(argv[3], 16, (long *)&u16Step);
                ret = kstrtol(argv[4], 16, (long *)&u16Span);

                _SimMiPnlSetSsc(bEn, u16Step, u16Span);
            }
        }
        else if(strcmp(argv[1], "test_pat") == 0)
        {
            if(argc == 5)
            {
                int val[4];
                ret = kstrtol(argv[2], 16, (long *)&val[0]);
                ret = kstrtol(argv[3], 16, (long *)&val[1]);
                ret = kstrtol(argv[4], 16, (long *)&val[2]);
                ret = kstrtol(argv[5], 16, (long *)&val[3]);
                _SimMiPnlTestPattern(val[0], val[1], val[2], val[3]);
            }
        }
        else if(strcmp(argv[1], "backlight_en") == 0)
        {
            if(argc == 2)
            {
                int val;
                ret = kstrtol(argv[2], 16, (long *)&val);
                _SimMiPnlBackLightOnOff(val);
            }
        }
        else if(strcmp(argv[1], "backlight_lv") == 0)
        {
            if(argc == 2)
            {
                int val;
                ret = kstrtol(argv[2], 16, (long *)&val);
                _SimMiPnlBackLightLevel(val);
            }
        }
        else if(strcmp(argv[1], "power") == 0)
        {
            if(argc == 2)
            {
                int val[4];
                ret = kstrtol(argv[2], 16, (long *)&val[0]);
                _SimMiPnlPower(val[0]);
            }
        }
        else if(strcmp(argv[1], "drv_cur") == 0)
        {
            if(argc == 2)
            {
                int val;
                ret = kstrtol(argv[2], 16, (long *)&val);
                _SimMiPnlDrvCurrent(val);
            }
        }
        else if(strcmp(argv[1], "timing") == 0)
        {
            if(argc == 12)
            {
                int u16Htt, u16Hpw, u16Hbp, u16Hactive, u16Hstart;
                int u16Vtt, u16Vpw, u16Vbp, u16Vactive, u16Vstart;
                int u16Dclk;

                ret = kstrtol(argv[2], 10, (long *)&u16Hpw);
                ret = kstrtol(argv[3], 10, (long *)&u16Hbp);
                ret = kstrtol(argv[4], 10, (long *)&u16Hstart);
                ret = kstrtol(argv[5], 10, (long *)&u16Hactive);
                ret = kstrtol(argv[6], 10, (long *)&u16Htt);
                ret = kstrtol(argv[7], 10, (long *)&u16Vpw);
                ret = kstrtol(argv[8], 10, (long *)&u16Vbp);
                ret = kstrtol(argv[9], 10, (long *)&u16Vstart);
                ret = kstrtol(argv[10], 10, (long *)&u16Vactive);
                ret = kstrtol(argv[11], 10, (long *)&u16Vtt);
                ret = kstrtol(argv[12], 10, (long *)&u16Dclk);

                _SimMiPnlSetLvdsTiming(u16Hpw, u16Hbp, u16Hstart, u16Hactive, u16Htt,
                                   u16Vpw, u16Vbp, u16Vstart, u16Vactive, u16Vtt,
                                   u16Dclk);
            }
            else if(argc == 9)
            {
                int u16Hpw, u16Hbp, u16Hactive, u16Hfp;
                int u16Vpw, u16Vbp, u16Vactive, u16Vfp;

                ret = kstrtol(argv[2], 10, (long *)&u16Hactive);
                ret = kstrtol(argv[3], 10, (long *)&u16Hpw);
                ret = kstrtol(argv[4], 10, (long *)&u16Hbp);
                ret = kstrtol(argv[5], 10, (long *)&u16Hfp);
                ret = kstrtol(argv[6], 10, (long *)&u16Vactive);
                ret = kstrtol(argv[7], 10, (long *)&u16Vpw);
                ret = kstrtol(argv[8], 10, (long *)&u16Vbp);
                ret = kstrtol(argv[9], 10, (long *)&u16Vfp);
                _SimmiPnlSetMipiDsiTiming(u16Hpw, u16Hbp, u16Hfp, u16Hactive, u16Vpw, u16Vbp, u16Vfp, u16Vactive);
            }
        }
        else if(strcmp(argv[1], "get") == 0)
        {
            if(argc == 2)
            {
                if(strcmp(argv[2], "param") == 0)
                {
                    _SimMiPnlGetParam();
                }
                else if(strcmp(argv[2], "timing") == 0)
                {
                    _SimMiPnlGetTiming();
                }
                else if(strcmp(argv[2], "power") == 0)
                {
                    _SimMiPnlGetPower();
                }
                else if(strcmp(argv[2], "backlight") == 0)
                {
                    _SimMiPnlGetBackLight();
                }
            }
        }
        else if(strcmp(argv[1], "dbg") == 0)
        {
            int val;

            if(argc == 2)
            {
                ret = kstrtol(argv[2], 16, (long *)&val);
                _SimMiPnlSetDbgLevel(val);
            }
        }
        else
        {
            SIMMI_DBG(1, "%s %d %s\n", __FUNCTION__, __LINE__, argv[1]);
        }
    }
    else
    {
        SIMMI_DBG(1, "%s %d\n", __FUNCTION__, __LINE__);

    }

}

