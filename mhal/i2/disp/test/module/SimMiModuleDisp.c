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
#include "apiXC.h"
#include "apiXC_EX.h"
#include "apiHDMITx.h"
#include "apiPNL_EX.h"
#include "drvTVEncoder.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
#include "mhal_hdmitx.h"
#include "drv_scl_mgwin_io_st.h"
#include "drv_scl_mgwin_io_wrapper.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define SIMMI_DBG(dbglv, _fmt, _args...)             \
    do                                             \
    if(dbglv)                                      \
    {                                              \
            printk(KERN_INFO _fmt, ## _args);       \
    }while(0)

#define SCL_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)


#define SCL_ERR(_fmt, _args...)       printk(KERN_ERR _fmt, ## _args)

#define SIMMI_ALIGN(x,y) ( x & ~(y-1))

#define SIMMI_SPRINTF(str, _fmt, _args...) \
    do {                                   \
        char tmpstr[1024];                 \
        sprintf(tmpstr, _fmt, ## _args);   \
        strcat(str, tmpstr);               \
    }while(0)

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


typedef struct
{
    MS_U32 u32DevcieId[E_SIMMI_MODULE_DEVICE_ID_NUM];
    MS_U32 u32VideoLayerId[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM];
    void *pDeviceCtx[E_SIMMI_MODULE_DEVICE_ID_NUM];
    void *pVideoLayerCtx[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM];
    void *pInputPortCtx[E_SIMMI_MODULE_VIDEOLAYER_ID_NUM][16];
}SimMiModuleCtxConfig_t;

typedef struct
{
    MSYS_DMEM_INFO stDmem;
    MS_BOOL bUsed;
}SimMiModuleDmemConfig_t;


typedef struct
{
    MS_BOOL bHVSwap;
    PNL_EX_PanelType stOrignPnlCfg;
}SimMiModulePanelConfig_t;

typedef struct
{
    MS_U32 u32XC;
    MS_U32 u32VE;
    MS_U32 u32Hdmitx;
    MS_U32 u32Mgwin[4];
    MS_U32 u32MhalDisp;
}SimMiModuleDbgMsgConfig_t;

typedef struct
{
    MS_U32 u32DeviceId;
    MS_U32 u32VideoLyaerId;
    MS_U32 u32InputPortFlag;
    MS_U32 u32InputPortId[16];
    MHAL_DISP_VideoFrameData_t stVideoFrame[16];
    MS_U32 u32BufferStart;
    MS_U32 u32BufferSize;
    MS_U8  u8Trigger;
}SimMiModuleIsrConfig_t;

extern MS_U16 u16XCDbgLevel;

#ifdef MI_ENABLE
extern MS_S32 mi_sys_MMA_Alloc(MS_U8 *u8MMAHeapName, MS_U32 u32Size ,MS_PHY *phyAddr);
extern MS_S32 mi_sys_MMA_Free(MS_PHY phyAddr);
#endif

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------

SimMiModuleCtxConfig_t gstModeuleCtx;
SimMiModulePanelConfig_t gstPnlHVSwapCfg[E_SIMMI_MODULE_PNL_TIMING_MAX_NUM];
SimMiModuleIsrConfig_t gstIsrCfg[E_SIMMI_MODULE_DEVICE_ID_NUM];


SimMiModuleDbgMsgConfig_t gstDbgMsgCfg;
//-------------------------------------------------------------------------------------------------
// Pirvate Function
//-------------------------------------------------------------------------------------------------
SimMiModuleDmemConfig_t stDmemCfg[20];


MS_S32 _SimMiModuleDispAllocDmem(MS_U8 *name, MS_U32 size, unsigned long long *pu64PhyAddr)
{
    MS_S32 sRet = 0;
    static MS_U8 u8NullNameIdx = 0;
    MSYS_DMEM_INFO dmem;
    MS_U8 u8Name[16][16] =
    {
        "DISP_MEM_00", "DISP_MEM_01", "DISP_MEM_02", "DISP_MEM_03",
        "DISP_MEM_04", "DISP_MEM_05", "DISP_MEM_06", "DISP_MEM_07",
        "DISP_MEM_08", "DISP_MEM_09", "DISP_MEM_0A", "DISP_MEM_0B",
        "DISP_MEM_0C", "DISP_MEM_0D", "DISP_MEM_0E", "DISP_MEM_0F",
    };

    *pu64PhyAddr = 0;
    if(name == NULL)
    {
        memcpy(dmem.name, u8Name[u8NullNameIdx], strlen(u8Name[u8NullNameIdx])+1);
        u8NullNameIdx = (u8NullNameIdx + 1) & 0x0F;
        printk(KERN_EMERG "NULL Name =%s\n", dmem.name);
    }
    else
    {
        memcpy(dmem.name,name,strlen(name)+1);
    }

    dmem.length = size;
    if(0 != msys_request_dmem(&dmem))
    {
        *pu64PhyAddr = 0;
        sRet = 1;
    }
    else
    {
        MS_U8 i;

        for(i=0; i<20; i++)
        {
            if(stDmemCfg[i].bUsed == 0)
            {
                stDmemCfg[i].bUsed = 1;
                memcpy(&stDmemCfg[i].stDmem, &dmem, sizeof(MSYS_DMEM_INFO));
                sRet = 0;
                *pu64PhyAddr = Chip_Phys_to_MIU(dmem.phys);
                printk(KERN_EMERG "%s %d, Addr=%llx\n", __FUNCTION__, __LINE__, *pu64PhyAddr);
                break;
            }
        }

    }
    return sRet;
}

MS_S32 _SimMiModuleDispFreeDmem(unsigned long long u64PhyAddr)
{
    MSYS_DMEM_INFO dmem;
    MS_U8 i;
    MS_S32 sRet = 1;

    for(i=0; i<20; i++)
    {
        if(stDmemCfg[i].bUsed  == 1 && stDmemCfg[i].stDmem.phys == u64PhyAddr)
        {
            memcpy(&dmem, &stDmemCfg[i].stDmem, sizeof(MSYS_DMEM_INFO));
            msys_release_dmem(&dmem);
            stDmemCfg[i].bUsed = 0;
            sRet = 0;
            break;
        }
    }
    return sRet;
}




MHAL_DISP_DeviceTiming_e _SimMiModuleDispTransTiming(SimMiModuleOuptTiming_e enTiming)
{
    MHAL_DISP_DeviceTiming_e eMhalDispTiming;

    eMhalDispTiming = (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_NTSC        ) ? E_MHAL_DISP_OUTPUT_NTSC :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_PAL         ) ? E_MHAL_DISP_OUTPUT_PAL :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_480P        ) ? E_MHAL_DISP_OUTPUT_480P60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_576P        ) ? E_MHAL_DISP_OUTPUT_576P50 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_720P50      ) ? E_MHAL_DISP_OUTPUT_720P50 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_720P60      ) ? E_MHAL_DISP_OUTPUT_720P60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P24     ) ? E_MHAL_DISP_OUTPUT_1080P24 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P25     ) ? E_MHAL_DISP_OUTPUT_1080P25 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P30     ) ? E_MHAL_DISP_OUTPUT_1080P30 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P50     ) ? E_MHAL_DISP_OUTPUT_1080P50 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P60     ) ? E_MHAL_DISP_OUTPUT_1080P60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_4K2K_30P    ) ? E_MHAL_DISP_OUTPUT_3840x2160_30 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_640x480_60  ) ? E_MHAL_DISP_OUTPUT_640x480_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_800x600_60  ) ? E_MHAL_DISP_OUTPUT_800x600_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1024x768_60 ) ? E_MHAL_DISP_OUTPUT_1024x768_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1280x1024_60) ? E_MHAL_DISP_OUTPUT_1280x1024_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1366x768_60 ) ? E_MHAL_DISP_OUTPUT_1366x768_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1440x900_60 ) ? E_MHAL_DISP_OUTPUT_1440x900_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1280x800_60 ) ? E_MHAL_DISP_OUTPUT_1280x800_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1680x1050_60) ? E_MHAL_DISP_OUTPUT_1680x1050_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1600x1200_60) ? E_MHAL_DISP_OUTPUT_1600x1200_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1920x1200_60) ? E_MHAL_DISP_OUTPUT_1920x1200_60 :
                      (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_USER)         ? E_MHAL_DISP_OUTPUT_USER :
                                                                                E_MHAL_DISP_OUTPUT_MAX;

    return eMhalDispTiming;
}

MHAL_DISP_PixelFormat_e _SimMiModuleDispTransPixelFormat(SimMiModulePixelFormat_e enPixelFormat)
{
    MHAL_DISP_PixelFormat_e enMhalDispPixelFormat;

    enMhalDispPixelFormat = (enPixelFormat == E_SIMMI_MODULE_PIXEL_FORMAT_ARGB8888) ? E_MHAL_DISP_PIXEL_FRAME_ARGB8888 :
                            (enPixelFormat == E_SIMMI_MODULE_PIXEL_FORMAT_YUV422)   ? E_MHAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422 :
                            (enPixelFormat == E_SIMMI_MODULE_PIXEL_FORMAT_RGB565)   ? E_MHAL_DISP_PIXEL_FRAME_RGB565 :
                                                                                      E_MHAL_DISP_PIXEL_FRAME_YUV_MST_420;
    return enMhalDispPixelFormat;
}

MHalHdmitxCDType_e _SimMiModuleHdmitxTransColorDepth(SimMiModuleHdmitxColorDepthType_e enColorDepth)
{
    MHalHdmitxCDType_e enCD;

    enCD = enColorDepth == E_SIMMI_MODULE_HDMITX_CD_8_BIT  ? E_MHAL_HDMITX_CD_8 :
           enColorDepth == E_SIMMI_MODULE_HDMITX_CD_10_BIT ? E_MHAL_HDMITX_CD_10 :
           enColorDepth == E_SIMMI_MODULE_HDMITX_CD_12_BIT ? E_MHAL_HDMITX_CD_12 :
           enColorDepth == E_SIMMI_MODULE_HDMITX_CD_16_BIT ? E_MHAL_HDMITX_CD_16 :
                                                             E_MHAL_HDMITX_CD_NUM;
   return enCD;
}

MHalHdmitxColorType_e _SimMiModuleHdmitxTransColor(SimMiModuleHdmitxColorType_e enColorType)
{
    MHalHdmitxColorType_e enColor;
    enColor = enColorType == E_SIMMI_MODULE_HDMITX_COLOR_RGB444 ? E_MHAL_HDMITX_COLOR_RGB444 :
              enColorType == E_SIMMI_MODULE_HDMITX_COLOR_YUV444 ? E_MHAL_HDMITX_COLOR_YUV444 :
              enColorType == E_SIMMI_MODULE_HDMITX_COLOR_YUV422 ? E_MHAL_HDMITX_COLOR_YUV422 :
                                                                  E_MHAL_HDMITX_COLOR_NUM;
    return enColor;
}


int _SimMiModuleIoctlSetHdmitxConfig(struct file *filp, unsigned long arg)
{
    SimMiModuleDispHdmitxConfig_t stHdmitxCfg;
    if(copy_from_user(&stHdmitxCfg, (SimMiModuleDispHdmitxConfig_t __user *)arg, sizeof(SimMiModuleDispHdmitxConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
#ifdef MHAL_HDMITX_EN
        MHalHdmitxCDType_e  enCD;
        MHalHdmitxColorType_e  enOutColor;
        MHalHdmitxColorType_e  enInColor;


        enCD = _SimMiModuleHdmitxTransColorDepth(stHdmitxCfg.enColorDepth);
        enOutColor = _SimMiModuleHdmitxTransColor(stHdmitxCfg.enOutColorType);
        enInColor  = _SimMiModuleHdmitxTransColor(stHdmitxCfg.enInColorType);

        if(enCD == E_MHAL_HDMITX_CD_NUM || enOutColor == E_MHAL_HDMITX_COLOR_NUM || enInColor == E_MHAL_HDMITX_COLOR_NUM)
        {
                return -EFAULT;
        }

        MHalHdmitxChangeColorDepth(enCD);
        MHalHdmitxChangeOutColor(enOutColor);
        MHalHdmitxChangeInColor(enInColor);
#endif
        return 0;
    }
}

int _SiMiModuleTestPattern(u32 u32Info)
{
#define PARSING_TEST_PATTERN_TYPE(x)    ( TEST_PATTERN_TYPE(x) == TEST_PATTERN_IP2      ? "IP2" : \
                                          TEST_PATTERN_TYPE(x) == TEST_PATTERN_OPM      ? "OPM" : \
                                          TEST_PATTERN_TYPE(x) == TEST_PATTERN_OP1      ? "OP1" : \
                                          TEST_PATTERN_TYPE(x) == TEST_PATTERN_VTRACK   ? "VTRACK" :  \
                                          TEST_PATTERN_TYPE(x) == TEST_PATTERN_HDMITX   ? "HDMITX" : \
                                                                                          "UNKNOWN" )



#define TEST_PATTERN_IP2        0x00010000
#define TEST_PATTERN_OPM        0x00020000
#define TEST_PATTERN_OP1        0x00040000
#define TEST_PATTERN_VTRACK     0x00080000
#define TEST_PATTERN_HDMITX     0x00100000

#define TEST_PATTERN_TYPE(x)    (x & 0xFFFF0000)
#define TEST_PATTERN_VAL(x)     (x & 0x000000FF)
#define TEST_PATTERN_DEVICE(x)  ((x >> 8) & 0xFF)

    MS_U8 u8PatternVlaue;
    MS_U8 u8PatternDevice;
    DrvSclMgwinIoRegisterConfig_t stRegConfig;

    u8PatternVlaue = TEST_PATTERN_VAL(u32Info);
    u8PatternDevice = TEST_PATTERN_DEVICE(u32Info);

    SIMMI_DBG(1, "==============================================================\n");
    SIMMI_DBG(1, "Type=%s, Device=%d, Val=%d \n", PARSING_TEST_PATTERN_TYPE(u32Info), u8PatternDevice, u8PatternVlaue);
    SIMMI_DBG(1, "==============================================================\n");

    memset(&stRegConfig, 0, sizeof(DrvSclMgwinIoRegisterConfig_t));
    switch(TEST_PATTERN_TYPE(u32Info))
    {
        case TEST_PATTERN_IP2:
            stRegConfig.u32Addr = (u8PatternDevice == 0) ? 0x130220 : 0x138220;
            stRegConfig.u16Msk  = 0x0200;
            stRegConfig.u16Val  = (u8PatternVlaue == 0) ? 0x0000 : stRegConfig.u16Msk;
            break;

        case TEST_PATTERN_OPM:
            stRegConfig.u32Addr = (u8PatternDevice == 0) ? 0x132020 : 0x13A020;
            stRegConfig.u16Msk = 0x8000;
            stRegConfig.u16Val  = (u8PatternVlaue == 0) ? 0x0000 : stRegConfig.u16Msk;
            break;

        case TEST_PATTERN_OP1:
            stRegConfig.u32Addr = (u8PatternDevice == 0) ? 0x13203E : 0x13A03E;
            stRegConfig.u16Msk  = 0x0400;
            stRegConfig.u16Val  = (u8PatternVlaue == 0) ? 0x0000 : stRegConfig.u16Msk;
            break;

        case TEST_PATTERN_VTRACK:
            stRegConfig.u32Addr = (u8PatternDevice == 0) ? 0x132F40 : 0x139032;
            stRegConfig.u16Msk  = (u8PatternDevice == 0) ? 0x0002 : 0x0002;
            stRegConfig.u16Val  = (u8PatternVlaue == 0)  ? 0x0000 : stRegConfig.u16Msk;
            break;

        case TEST_PATTERN_HDMITX:
            stRegConfig.u32Addr = 0x172D60;
            stRegConfig.u16Msk = 0x03;
            stRegConfig.u16Val = (u8PatternVlaue) & 0x03;
            break;

        default:
            return -EFAULT;
    }

    if(stRegConfig.u32Addr)
    {
        _DrvSclMgwinIoSetRegisterConfig(NULL, &stRegConfig);
    }

    return 0;
}

int _SimMiModuleDebugMsic(u16 u16Info)
{
#define PARSING_CMD_TYPE(x)     ( CMD_TYPE(x) == MISC_CMD_DEVICE     ? "DEVICE " : \
                                  CMD_TYPE(x) == MISC_CMD_VIDEOLAYER ? "VideoLayer " : \
                                  CMD_TYPE(x) == MISC_CMD_INPUTORT   ? "InputPort " : \
                                                                       "Unknown")

#define PARSING_CMD_OPT(x)      ( CMD_OPT(x) == MISC_CMD_OPT_CREATE  ? "Create" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_DESTROY ? "Destroy" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_ENABLE  ? "Enabel" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_DISABLE ? "Disable" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_BIND    ? "Bind" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_UNBIND  ? "UnBind" : \
                                  CMD_OPT(x) == MISC_CMD_OPT_FLIP    ? "Flip" : \
                                                                       "Unknown")



#define MISC_CMD_DEVICE         (0x1000)
#define MISC_CMD_VIDEOLAYER     (0x2000)
#define MISC_CMD_INPUTORT       (0x3000)

#define MISC_CMD_OPT_CREATE     (0x0100)
#define MISC_CMD_OPT_DESTROY    (0x0200)
#define MISC_CMD_OPT_ENABLE     (0x0300)
#define MISC_CMD_OPT_DISABLE    (0x0400)
#define MISC_CMD_OPT_BIND       (0x0500)
#define MISC_CMD_OPT_UNBIND     (0x0700)
#define MISC_CMD_OPT_FLIP       (0x0800)

#define CMD_TYPE(x)         (x & 0xF000)
#define CMD_OPT(x)          (x & 0x0F00)
#define CMD_INST_NUM(x)     (x & 0x00FF)

    MHAL_DISP_AllocPhyMem_t StAlloc;
    u8 InstNum, idx;
    u16 u16VideLayerNum;
    u16 u16InputPortNum[2];
    u16 VideoLayerIdx, InputPortIdx;

#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    InstNum = CMD_INST_NUM(u16Info);

    SIMMI_DBG(1, "==============================================================\n");
    SIMMI_DBG(1, "%x, %s %s , Num=%d\n", u16Info, PARSING_CMD_TYPE(u16Info), PARSING_CMD_OPT(u16Info), InstNum);
    SIMMI_DBG(1, "==============================================================\n");

    switch(CMD_TYPE(u16Info))
    {
        case MISC_CMD_DEVICE:
            if(InstNum > 2)
            {
                SCL_ERR("only 2 device(%d) \n", InstNum);
                return -EFAULT;
            }


            if(CMD_OPT(u16Info) == MISC_CMD_OPT_CREATE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(MHAL_DISP_DeviceCreateInstance(&StAlloc, idx, &gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "%s %d, CreaetInstance fail i=%d, id=%d\n", __FUNCTION__, __LINE__, idx, idx);
                        return -EFAULT;
                    }

                    gstModeuleCtx.u32DevcieId[idx] = idx;
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DESTROY)
            {

                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pDeviceCtx[idx])
                    {
                        if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                        {
                            SIMMI_DBG(1, "Device Destory fail\n");
                            return -EFAULT;
                        }
                       gstModeuleCtx.pDeviceCtx[idx] = NULL;
                    }
                }

            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_ENABLE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pDeviceCtx[idx])
                    {
                        if(MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], TRUE) == FALSE)
                        {
                            SIMMI_DBG(1, "Device Destory fail\n");
                            return -EFAULT;
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DISABLE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pDeviceCtx[idx])
                    {
                        if(MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], FALSE) == FALSE)
                        {
                            SIMMI_DBG(1, "Device Destory fail\n");
                            return -EFAULT;
                        }
                    }
                }
            }
            break;

        case MISC_CMD_VIDEOLAYER:
            if(InstNum > 2)
            {
                SCL_ERR("only 2 VideoLayer(%d) \n", InstNum);
                return -EFAULT;
            }

            if(CMD_OPT(u16Info) == MISC_CMD_OPT_CREATE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, idx ,&gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "%s %d, CreateVideoLayer fail i=%d, id=%d\n", __FUNCTION__, __LINE__, idx, idx);
                        return -EFAULT;
                    }
                    gstModeuleCtx.u32VideoLayerId[idx] = idx;

                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DESTROY)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pVideoLayerCtx[idx])
                    {
                        if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                        {
                            SIMMI_DBG(1, "VideoLayer Destory fail\n");
                            return -EFAULT;
                        }
                        gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
                    }

                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_ENABLE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pVideoLayerCtx[idx])
                    {
                        if(MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], TRUE) == FALSE)
                        {
                            SIMMI_DBG(1, "VideoLayer Enable fail\n");
                            return -EFAULT;
                        }
                    }

                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DISABLE)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pVideoLayerCtx[idx])
                    {
                        if(MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], FALSE) == FALSE)
                        {
                            SIMMI_DBG(1, "VideoLayer Enable fail\n");
                            return -EFAULT;
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_BIND)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pVideoLayerCtx[idx] && gstModeuleCtx.pDeviceCtx[idx])
                    {
                        if(MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[idx],  gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                        {
                            SIMMI_DBG(1, "VideoLayer Bind Fail\n");
                            return -EFAULT;
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_UNBIND)
            {
                for(idx=0; idx<InstNum; idx++)
                {
                    if(gstModeuleCtx.pVideoLayerCtx[idx] && gstModeuleCtx.pDeviceCtx[idx])
                    {
                        if(MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[idx],  gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                        {
                            SIMMI_DBG(1, "VideoLayer UnBind Fail\n");
                            return -EFAULT;
                        }
                    }
                }
            }

            break;

        case MISC_CMD_INPUTORT:
            if(InstNum > 32)
            {
                SCL_ERR("only 16 VideoLyaer(%d) \n", InstNum);
                return -EFAULT;
            }

            if(InstNum <=16)
            {
                u16VideLayerNum = 1;
                u16InputPortNum[0] = InstNum;
            }
            else
            {
                u16VideLayerNum = 2;
                u16InputPortNum[0] = 16;
                u16InputPortNum[1] = InstNum - 16;
            }

            if(CMD_OPT(u16Info) == MISC_CMD_OPT_CREATE)
            {
                for(VideoLayerIdx = 0; VideoLayerIdx < u16VideLayerNum; VideoLayerIdx++)
                {
                    for(InputPortIdx = 0 ; InputPortIdx < u16InputPortNum[VideoLayerIdx]; InputPortIdx++)
                    {
                        if(gstModeuleCtx.pVideoLayerCtx[VideoLayerIdx])
                        {
                            if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[VideoLayerIdx], InputPortIdx, &gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx]) == FALSE)
                            {
                                SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d %d)\n", __FUNCTION__, __LINE__, VideoLayerIdx, InputPortIdx);
                                return -EFAULT;
                            }
                        }
                        else
                        {
                            SIMMI_DBG(1, "%s %d, CreateInputPort fail , VideoLyaer NULL (%d %d)\n", __FUNCTION__, __LINE__, VideoLayerIdx, InputPortIdx);
                            return -EFAULT;
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DESTROY)
            {
                for(VideoLayerIdx = 0; VideoLayerIdx < u16VideLayerNum; VideoLayerIdx++)
                {
                    for(InputPortIdx = 0 ; InputPortIdx < u16InputPortNum[VideoLayerIdx]; InputPortIdx++)
                    {
                        if(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx])
                        {
                            if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx]) == FALSE)
                            {
                                SIMMI_DBG(1, "%s %d, Destroy InputPort fail (%d %d)\n", __FUNCTION__, __LINE__, VideoLayerIdx, InputPortIdx);
                                return -EFAULT;
                            }
                            else
                            {
                                gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx] = NULL;
                            }
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_ENABLE)
            {
                for(VideoLayerIdx = 0; VideoLayerIdx < u16VideLayerNum; VideoLayerIdx++)
                {
                    for(InputPortIdx = 0 ; InputPortIdx < u16InputPortNum[VideoLayerIdx]; InputPortIdx++)
                    {
                        if(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx])
                        {
                            if(MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx], TRUE) == FALSE)
                            {
                                SIMMI_DBG(1, "%s %d, Destroy InputPort Enable (%d %d)\n", __FUNCTION__, __LINE__, VideoLayerIdx, InputPortIdx);
                                return -EFAULT;
                            }
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_DISABLE)
            {
                for(VideoLayerIdx = 0; VideoLayerIdx < u16VideLayerNum; VideoLayerIdx++)
                {
                    for(InputPortIdx = 0 ; InputPortIdx < u16InputPortNum[VideoLayerIdx]; InputPortIdx++)
                    {
                        if(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx])
                        {
                            if(MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx], FALSE) == FALSE)
                            {
                                SIMMI_DBG(1, "%s %d, Destroy InputPort Enable (%d %d)\n", __FUNCTION__, __LINE__, VideoLayerIdx, InputPortIdx);
                                return -EFAULT;
                            }
                        }
                    }
                }
            }
            else if(CMD_OPT(u16Info) == MISC_CMD_OPT_FLIP)
            {
                MHAL_DISP_VideoFrameData_t stVideoFrameData;

                stVideoFrameData.ePixelFormat = E_MHAL_DISP_PIXEL_FRAME_YUV_MST_420;
                stVideoFrameData.eCompressMode = E_MHAL_DISP_COMPRESS_MODE_NONE;
                stVideoFrameData.u32Width = 640;
                stVideoFrameData.u32Height = 320;
                stVideoFrameData.aPhyAddr[0] = 0x24701100;
                stVideoFrameData.au32Stride[0] = 320 * 3 /2;

                for(VideoLayerIdx = 0; VideoLayerIdx < u16VideLayerNum; VideoLayerIdx++)
                {
                    for(InputPortIdx = 0 ; InputPortIdx < u16InputPortNum[VideoLayerIdx]; InputPortIdx++)
                    {
                        MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[VideoLayerIdx][InputPortIdx], &stVideoFrameData);
                    }
                }
            }

            break;

    }

    return 0;
}


int _SimMiModuleIoctlSetDebugLevelConfig(struct file *filp, unsigned long arg)
{
    SimMiModuleDispDebguConfig_t stDebugCfg;

    if(copy_from_user(&stDebugCfg, (SimMiModuleDispDebguConfig_t __user *)arg, sizeof(SimMiModuleDispDebguConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        SIMMI_DBG(1, "%s %d, debug(%x %lx)\n", __FUNCTION__, __LINE__, stDebugCfg.enType, stDebugCfg.u32Flag);
        switch(stDebugCfg.enType)
        {
            case E_SIMMI_MODULE_DEBUG_LEVEL_XC:
                MApi_XC_EX_SetDbgLevel(stDebugCfg.u32Flag);
                break;

            case E_SIMMI_MODULE_DEBUG_LEVEL_HDMITX:
    #ifdef MHAL_HDMITX_EN
                MApi_HDMITx_SetDbgLevel(stDebugCfg.u32Flag);
    #endif
                break;
            case E_SIMMI_MODULE_DEBUG_LEVEL_MHAL_DISP:
                {
                    MS_U32 u32MhalDispLevel = stDebugCfg.u32Flag;
                    MHAL_DISP_DbgLevel(&u32MhalDispLevel);
                }
                break;

            case E_SIMMI_MODULE_DEBUG_LEVEL_VE:
                {
                    MDrv_VE_SetDbgLevel(stDebugCfg.u32Flag ? 1 : 0);
                }
                break;

            case E_SIMMI_MODULE_DEBUG_LEVEL_MGWIN:
                {
                    DrvSclMgwinIoDebugConfig_t stDbgCfg;
                    stDbgCfg.u8DbgOpt = (MS_U8)((stDebugCfg.u32Flag >> 8) & 0xFF);
                    stDbgCfg.u8DbgLvl = (MS_U8)(stDebugCfg.u32Flag & 0xFF);
                    _DrvSclMgwinIoSetDebugConfig(0, &stDbgCfg);
                }
                break;

            case E_SIMMI_MODULE_DEBUG_LEVEL_MISC:
                _SimMiModuleDebugMsic(stDebugCfg.u32Flag);

                break;

            case E_SIMMI_MODULE_DEBUG_LEVEL_TEST_PATTERN:
                _SiMiModuleTestPattern(stDebugCfg.u32Flag);
                break;

            default:
                return -EFAULT;
        }
    }

    return 0;
}


int _SimMiModuleIoctlDispPictureConfig(struct file *filp, unsigned long arg)
{
    SimMiModuleDispPictureConfig_t stColorCfg;
    MHAL_DISP_HdmiParam_t stHdmiParam;
    MHAL_DISP_VgaParam_t  stVgaParam;

    if(copy_from_user(&stColorCfg, (SimMiModuleDispPictureConfig_t __user *)arg, sizeof(SimMiModuleDispPictureConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        if(stColorCfg.enInterface == E_SIMMI_MODULE_OUTPUT_HDMI)
        {
            stHdmiParam.stCsc.eCscMatrix    = (MHAL_DISP_CscMattrix_e)stColorCfg.enCscType;
            stHdmiParam.stCsc.u32Contrast   = stColorCfg.u32Contrast;
            stHdmiParam.stCsc.u32Hue        = stColorCfg.u32Hue;
            stHdmiParam.stCsc.u32Saturation = stColorCfg.u32Saturation;
            stHdmiParam.stCsc.u32Luma       = stColorCfg.u32Brightness;

            if(gstModeuleCtx.pDeviceCtx[0] && gstModeuleCtx.u32DevcieId[0] == 0)
            {
                MHAL_DISP_DeviceSetHdmiParam(gstModeuleCtx.pDeviceCtx[0], &stHdmiParam);
            }
            else
            {
                SIMMI_DBG(1, "%s %d, Device Instant NULL\n", __FUNCTION__, __LINE__);
            }

        }
        else if(stColorCfg.enInterface == E_SIMMI_MODULE_OUTPUT_VGA)
        {
            stVgaParam.stCsc.eCscMatrix    = (MHAL_DISP_CscMattrix_e)stColorCfg.enCscType;
            stVgaParam.stCsc.u32Contrast   = stColorCfg.u32Contrast;
            stVgaParam.stCsc.u32Hue        = stColorCfg.u32Hue;
            stVgaParam.stCsc.u32Saturation = stColorCfg.u32Saturation;
            stVgaParam.stCsc.u32Luma       = stColorCfg.u32Brightness;
            stVgaParam.u32Sharpness        = stColorCfg.u32Sharpness;
            stVgaParam.u32Gain             = stColorCfg.u32Gain;

            if(gstModeuleCtx.pDeviceCtx[0] && gstModeuleCtx.u32DevcieId[0] == 1)
            {
                MHAL_DISP_DeviceSetVgaParam(gstModeuleCtx.pDeviceCtx[0], &stVgaParam);
            }
            else if(gstModeuleCtx.pDeviceCtx[1] && gstModeuleCtx.u32DevcieId[1] == 1)
            {
                MHAL_DISP_DeviceSetVgaParam(gstModeuleCtx.pDeviceCtx[1], &stVgaParam);
            }
        }

    }

    return 0;
}

extern PNL_EX_PanelType * tPanelIndexTbl[E_SIMMI_MODULE_PNL_TIMING_MAX_NUM];

#define HVSWAP(H,V,X) \
{ \
    X=H;  \
    H=V; \
    V=X; \
}


static irqreturn_t _SimMiModuleDispDevIsr(int eIntNum, void* pstDevParam)
{
    MHAL_DISP_IRQFlag_t stIrqFlag;
    if(pstDevParam == NULL)
    {
        SIMMI_DBG(1, "NULL Pointer: dev_id!!!\n");
    }
    else
    {
        SimMiModuleIsrConfig_t *pIsrCfg = (SimMiModuleIsrConfig_t *)pstDevParam;

        pIsrCfg->u8Trigger = 1;

        MHAL_DISP_GetDevIrqFlag(  gstModeuleCtx.pDeviceCtx[pIsrCfg->u32DeviceId], &stIrqFlag);
        MHAL_DISP_ClearDevInterrupt(gstModeuleCtx.pDeviceCtx[pIsrCfg->u32DeviceId], &stIrqFlag);
    }



    return IRQ_HANDLED;

}

static int _SimMiModuleDispFlipThread(void *arg)
{
    static MS_U32 u32Timer[2] = {0, 0};
    static MS_U32 u32Idx[2] = {0, 0};
    SimMiModuleIsrConfig_t *pIsrCfg = (SimMiModuleIsrConfig_t *)arg;
    MS_U8 i;
    MS_U32 u32BufferAddr;

    for(;;)
    {
        if (kthread_should_stop())
        {
            break;
        }

        if(pIsrCfg->u8Trigger)
        {
            MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[pIsrCfg->u32VideoLyaerId] );
            for(i=0;i<16; i++)
            {
                if( pIsrCfg->u32InputPortFlag & (1<<i))
                {

                    u32BufferAddr = pIsrCfg->stVideoFrame[i].aPhyAddr[0] + pIsrCfg->stVideoFrame[i].u32Height * pIsrCfg->stVideoFrame[i].au32Stride[0];

                    if(u32BufferAddr < (pIsrCfg->u32BufferStart + pIsrCfg->u32BufferSize))
                    {
                        pIsrCfg->stVideoFrame[i].aPhyAddr[0] = u32BufferAddr;
                    }
                    else
                    {
                        pIsrCfg->stVideoFrame[i].aPhyAddr[0] = pIsrCfg->u32BufferStart;
                    }

                    MHAL_DISP_InputPortFlip(
                        gstModeuleCtx.pInputPortCtx[pIsrCfg->u32VideoLyaerId][pIsrCfg->u32InputPortId[i]],
                        &pIsrCfg->stVideoFrame[i]);
                }
            }

            MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[pIsrCfg->u32VideoLyaerId] );
            pIsrCfg->u8Trigger = 0;

            if(MsOS_GetSystemTime() - u32Timer[pIsrCfg->u32DeviceId] > 999)
            {
                SIMMI_DBG(0, "Isr Idx=%d\n", u32Idx[pIsrCfg->u32DeviceId]);
                u32Timer[pIsrCfg->u32DeviceId] = MsOS_GetSystemTime();
                u32Idx[pIsrCfg->u32DeviceId] = 0;
            }
            u32Idx[pIsrCfg->u32DeviceId]++;
        }
    }

    return 0;
}

void _SimMiModulePnlHVSwap(MS_BOOL bHVSwap, SimMiModuleOuptTiming_e enTiming)
{
    SimMiModulePnlTiming_e enPnlTiming;

    enPnlTiming = (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_480P        ) ? E_SIMMI_MODULE_PNL_TIMING_480P          :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_576P        ) ? E_SIMMI_MODULE_PNL_TIMING_576P          :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_720P50      ) ? E_SIMMI_MODULE_PNL_TIMING_720P_50       :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_720P60      ) ? E_SIMMI_MODULE_PNL_TIMING_720P_60       :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P24     ) ? E_SIMMI_MODULE_PNL_TIMING_1080P_24      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P25     ) ? E_SIMMI_MODULE_PNL_TIMING_1080P_25      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P30     ) ? E_SIMMI_MODULE_PNL_TIMING_1080P_30      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P50     ) ? E_SIMMI_MODULE_PNL_TIMING_1080P_50      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1080P60     ) ? E_SIMMI_MODULE_PNL_TIMING_1080P_60      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_4K2K_30P    ) ? E_SIMMI_MODULE_PNL_TIMING_3840X2160P_30 :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_640x480_60  ) ? E_SIMMI_MODULE_PNL_TIMING_640X480P      :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_800x600_60  ) ? E_SIMMI_MODULE_PNL_TIMING_800X600P_60   :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1024x768_60 ) ? E_SIMMI_MODULE_PNL_TIMING_1024X768P_60  :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1280x1024_60) ? E_SIMMI_MODULE_PNL_TIMING_1280X1024P_60 :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1366x768_60 ) ? E_SIMMI_MODULE_PNL_TIMING_1366X768P_60  :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1440x900_60 ) ? E_SIMMI_MODULE_PNL_TIMING_1440X900P_60  :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1280x800_60 ) ? E_SIMMI_MODULE_PNL_TIMING_1280X800P_60  :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1680x1050_60) ? E_SIMMI_MODULE_PNL_TIMING_1680X1050P_60 :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1600x1200_60) ? E_SIMMI_MODULE_PNL_TIMING_1600X1200P_60 :
                  (enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_1920x1200_60) ? E_SIMMI_MODULE_PNL_TIMING_1920X1200P_60 :
                                                                            E_SIMMI_MODULE_PNL_TIMING_MAX_NUM;




    if((enPnlTiming != E_SIMMI_MODULE_PNL_TIMING_MAX_NUM) && (bHVSwap != gstPnlHVSwapCfg[enPnlTiming].bHVSwap))
    {
        PNL_EX_PanelType * pPnlType = tPanelIndexTbl[enPnlTiming];
        MS_U8 u8val;
        MS_U16 u16val;


       SIMMI_DBG(1, "(%d %d) (%d %d) (%d %d) (%d %d) (%d %d %d) (%d %d %d)\n",
           pPnlType->m_ucPanelHSyncWidth, pPnlType->m_ucPanelHSyncBackPorch,
           pPnlType->m_ucPanelVSyncWidth, pPnlType->m_ucPanelVBackPorch,
           pPnlType->m_wPanelHStart, pPnlType->m_wPanelVStart,
           pPnlType->m_wPanelWidth, pPnlType->m_wPanelHeight,
           pPnlType->m_wPanelMaxHTotal, pPnlType->m_wPanelHTotal, pPnlType->m_wPanelMinHTotal,
           pPnlType->m_wPanelMaxVTotal, pPnlType->m_wPanelVTotal, pPnlType->m_wPanelMinVTotal);

       if(bHVSwap == 1)
       {
            memcpy(&gstPnlHVSwapCfg[enPnlTiming].stOrignPnlCfg, pPnlType, sizeof(PNL_EX_PanelType));


            HVSWAP(pPnlType->m_ucPanelHSyncWidth,     pPnlType->m_ucPanelVSyncWidth, u8val);
            HVSWAP(pPnlType->m_ucPanelHSyncBackPorch, pPnlType->m_ucPanelVBackPorch, u8val);

            pPnlType->m_wPanelHStart = pPnlType->m_ucPanelHSyncWidth + pPnlType->m_ucPanelHSyncBackPorch;
            pPnlType->m_wPanelVStart = pPnlType->m_ucPanelVSyncWidth + pPnlType->m_ucPanelVBackPorch;

            HVSWAP(pPnlType->m_wPanelWidth,  pPnlType->m_wPanelHeight, u16val);

            HVSWAP(pPnlType->m_wPanelMaxHTotal, pPnlType->m_wPanelMaxVTotal, u16val);
            HVSWAP(pPnlType->m_wPanelHTotal,    pPnlType->m_wPanelVTotal, u16val);
            HVSWAP(pPnlType->m_wPanelMinHTotal, pPnlType->m_wPanelMinVTotal, u16val);

        }
        else
        {
            memcpy(pPnlType, &gstPnlHVSwapCfg[enPnlTiming].stOrignPnlCfg, sizeof(PNL_EX_PanelType));
        }

        SIMMI_DBG(1, "(%d %d) (%d %d) (%d %d) (%d %d) (%d %d %d) (%d %d %d)\n",
            pPnlType->m_ucPanelHSyncWidth, pPnlType->m_ucPanelHSyncBackPorch,
            pPnlType->m_ucPanelVSyncWidth, pPnlType->m_ucPanelVBackPorch,
            pPnlType->m_wPanelHStart, pPnlType->m_wPanelVStart,
            pPnlType->m_wPanelWidth, pPnlType->m_wPanelHeight,
            pPnlType->m_wPanelMaxHTotal, pPnlType->m_wPanelHTotal, pPnlType->m_wPanelMinHTotal,
            pPnlType->m_wPanelMaxVTotal, pPnlType->m_wPanelVTotal, pPnlType->m_wPanelMinVTotal);

        gstPnlHVSwapCfg[enPnlTiming].bHVSwap = bHVSwap;
    }
}


int _SimMiModuleDispAttach(SimMiModuleDispConfig_t stDispCfg)
{
    MHAL_DISP_AllocPhyMem_t StAlloc;
    MS_U16 i,idx, interfaceidx;
    static MS_BOOL bCreate = 0;
    static MS_BOOL bAttachedDone = 0;
    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;
    MS_U8 u8InputPortId;

#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    if(stDispCfg.u8DeivceNum == 2)
    {

        if(bCreate == 0)
        {
            // Create Device Context
            if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[0].enDeviceId, &gstModeuleCtx.pDeviceCtx[0]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreaetInstance fail id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stDeviceCfg[0].enDeviceId);
                return -EFAULT;
            }
            gstModeuleCtx.u32DevcieId[0] = stDispCfg.stDeviceCfg[0].enDeviceId;

            if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[1].enDeviceId, &gstModeuleCtx.pDeviceCtx[1]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreaetInstance fail id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stDeviceCfg[1].enDeviceId);
                return -EFAULT;
            }
            gstModeuleCtx.u32DevcieId[1] = stDispCfg.stDeviceCfg[1].enDeviceId;


            // Create VideoLayer Context
            if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[0].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[0]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreateVideoLayer fail id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stVideoLayerCfg[0].enVideoLayerId);
                return -EFAULT;
            }
            gstModeuleCtx.u32VideoLayerId[0] = stDispCfg.stVideoLayerCfg[0].enVideoLayerId;

            if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[1].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[1]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreateVideoLayer fail  id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stVideoLayerCfg[1].enVideoLayerId);
                return -EFAULT;
            }
            gstModeuleCtx.u32VideoLayerId[1] = stDispCfg.stVideoLayerCfg[1].enVideoLayerId;

            // Create InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[0] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[0][i].u8PortId;
                    if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[0], u32PortIdx, &gstModeuleCtx.pInputPortCtx[0][u32PortIdx]) == FALSE)
                    {
                        SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d) PortIdx=%d\n", __FUNCTION__, __LINE__,  i, u32PortIdx);
                        return -EFAULT;
                    }
                }
            }

            bCreate = 1;

        }

#ifdef MHAL_HDMITX_EN
        if(stDispCfg.stDeviceCfg[0].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
        {
            MHalHdmitxInit();
        }
#endif


        MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[0], gstModeuleCtx.pDeviceCtx[0]);

        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[0], FALSE); // mute VideoLayer0

        for(i=0; i<16; i++)
        {
            if(stDispCfg.u16InputPortFlag[0] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[0][i].u8PortId;
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[0][u8InputPortId], FALSE);
            }
        }

        MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], FALSE); // Mute Device 0

        for(idx=0; idx<stDispCfg.u8DeivceNum; idx++)
        {
            if(stDispCfg.stDeviceCfg[idx].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
            {
                for(interfaceidx = E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1 ; interfaceidx < E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM; interfaceidx++)
                {
                    MS_U32 u32Interface = 0;
                    stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[interfaceidx]);

                    if(stTimingInfo.eTimeType != E_MHAL_DISP_OUTPUT_MAX)
                    {
                        if(stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_PAL ||  stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_NTSC )
                        {
                            u32Interface |= MHAL_DISP_INTF_CVBS;
                            MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                            MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_CVBS, &stTimingInfo);

                        }
                        else
                        {
                            u32Interface |= MHAL_DISP_INTF_HDMI;
                            MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                            MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_HDMI, &stTimingInfo);
        #ifdef MHAL_HDMITX_EN
                            MHalHdmitxChangeResolution(stTimingInfo.eTimeType);
        #endif

                        }

                    }
                }
            }
            else
            {
                stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1]);


                MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_VGA);
                MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx],
                                                MHAL_DISP_INTF_VGA,
                                                &stTimingInfo);
            }

            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], TRUE);
        }

        stVidLayerAttr.stVidLayerSize.u32Width  = stDispCfg.stVideoLayerCfg[0].u16Width;
        stVidLayerAttr.stVidLayerSize.u32Height = stDispCfg.stVideoLayerCfg[0].u16Height;
        stVidLayerAttr.stVidLayerDispWin.u16X   = stDispCfg.stVideoLayerCfg[0].stDispRect.u16X;
        stVidLayerAttr.stVidLayerDispWin.u16Y   = stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y;
        stVidLayerAttr.stVidLayerDispWin.u16Width  = stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width;
        stVidLayerAttr.stVidLayerDispWin.u16Height = stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height;
        stVidLayerAttr.ePixFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.enPixelFormat[0]);
        MHAL_DISP_VideoLayerSetAttr(gstModeuleCtx.pVideoLayerCtx[0], &stVidLayerAttr);
        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[0], TRUE);

        if(bAttachedDone)
        {
            MHAL_DISP_DeviceDetach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
        }

        MHAL_DISP_DeviceAttach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
        bAttachedDone = 1;

        MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[0]);

        for(i=0; i<16; i++)
        {
            if(stDispCfg.u16InputPortFlag[0] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[0][i].u8PortId;


                stInportAttr.stDispWin.u16X       = stDispCfg.stInputPortCfg[0][u8InputPortId].u16X;
                stInportAttr.stDispWin.u16Y       = stDispCfg.stInputPortCfg[0][u8InputPortId].u16Y;
                stInportAttr.stDispWin.u16Height  = stDispCfg.stInputPortCfg[0][u8InputPortId].u16Height;
                stInportAttr.stDispWin.u16Width   = stDispCfg.stInputPortCfg[0][u8InputPortId].u16Width;
                MHAL_DISP_InputPortSetAttr(gstModeuleCtx.pInputPortCtx[0][u8InputPortId], &stInportAttr);

                stVideoFrameData.eCompressMode = 0;
                stVideoFrameData.ePixelFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.stInputPortCfg[0][u8InputPortId].enPixelFormat);

                stVideoFrameData.u32Width  = stDispCfg.stInputPortCfg[0][u8InputPortId].u16Width;
                stVideoFrameData.u32Height = stDispCfg.stInputPortCfg[0][u8InputPortId].u16Height;

                stVideoFrameData.au32Stride[0] = stDispCfg.stInputPortCfg[0][u8InputPortId].u32Stride;
                stVideoFrameData.aPhyAddr[0]   = stDispCfg.stInputPortCfg[0][u8InputPortId].u32BaseAddr;
                MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[0][u8InputPortId], &stVideoFrameData);
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[0][u8InputPortId], TRUE);
            }
        } // for(i=0; i<16; i++)

        MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[0]);


        SIMMI_DBG(1, "-----------------------------------------------------------\n");
        SIMMI_DBG(1, "       Delay %d Sec\n", stDispCfg.u32DemoTimeSec);
        SIMMI_DBG(1, "-----------------------------------------------------------\n");


        if(stDispCfg.u32DemoTimeSec != 112233)
        {
            msleep(stDispCfg.u32DemoTimeSec*1000);


            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], FALSE);
            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[1], FALSE);

            MHAL_DISP_DeviceDetach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
            bAttachedDone = 0;

            MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[0], gstModeuleCtx.pDeviceCtx[0]);

            // Destory InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[0] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[0][i].u8PortId;

                    if(gstModeuleCtx.pInputPortCtx[0][u32PortIdx])
                    {
                        if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[0][u32PortIdx]) == FALSE)
                        {
                            SIMMI_DBG(1, "InputPort Destory Fail, i=%d, port=%d\n", i, u32PortIdx);
                            return -EFAULT;
                        }
                    }
                    gstModeuleCtx.pInputPortCtx[0][u32PortIdx] = NULL;
                }
            }



        #ifdef MHAL_HDMITX_EN
                MHalHdmitxPowerOff(0);
        #endif

            // Desotry Device Context
            for(idx=0; idx<stDispCfg.u8DeivceNum; idx++)
            {
                // Desotry VideoLayer Context
                if(gstModeuleCtx.pVideoLayerCtx[idx])
                {
                    if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "VideoLayer Destory fail\n");
                        return -EFAULT;
                    }
                    gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
                }

                if(gstModeuleCtx.pDeviceCtx[idx])
                {
                    if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "Device Destory fail\n");
                        return -EFAULT;
                    }
                   gstModeuleCtx.pDeviceCtx[idx] = NULL;
                }
            }
            bCreate = 0;
        }

    }
    else
    {
        SIMMI_DBG(1, "DeviceNum != VideoLayerNum (%d != %d)\n", stDispCfg.u8DeivceNum, stDispCfg.u8VideoLayerNum);
    }

    return 0;

}


int _SimMiModuleDisp32MgwinAttach(SimMiModuleDispConfig_t stDispCfg)
{
    MHAL_DISP_AllocPhyMem_t StAlloc;
    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;
    //MS_U32 u32DevIrq[E_SIMMI_MODULE_DEVICE_ID_NUM];
    //MHAL_DISP_IRQFlag_t stIrqFlag;
    MS_U16 i,idx, interfaceidx;
    static MS_BOOL bCreate = FALSE;
    MS_U8 u8InputPortId;
    static MS_BOOL bAttachedDone = 0;


    if(stDispCfg.u8VideoLayerNum != 2 && stDispCfg.u8DeivceNum != 2)
    {
        return -EFAULT;
    }

#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    if(bCreate == FALSE)
    {
        // Create Device Context
        if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[0].enDeviceId, &gstModeuleCtx.pDeviceCtx[0]) == FALSE)
        {
            SIMMI_DBG(1, "%s %d, CreaetInstance fail id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stDeviceCfg[0].enDeviceId);
            return -EFAULT;
        }
        gstModeuleCtx.u32DevcieId[0] = stDispCfg.stDeviceCfg[0].enDeviceId;

        if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[1].enDeviceId, &gstModeuleCtx.pDeviceCtx[1]) == FALSE)
        {
            SIMMI_DBG(1, "%s %d, CreaetInstance fail id=%d\n", __FUNCTION__, __LINE__, stDispCfg.stDeviceCfg[1].enDeviceId);
            return -EFAULT;
        }
        gstModeuleCtx.u32DevcieId[1] = stDispCfg.stDeviceCfg[1].enDeviceId;

        for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
        {
            // Create VideoLayer Context
            if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreateVideoLayer fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId);
                return -EFAULT;
            }
            gstModeuleCtx.u32VideoLayerId[idx] = stDispCfg.stVideoLayerCfg[idx].enVideoLayerId;

            // Create InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                    if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[idx], u32PortIdx, &gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                    {
                        SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d %d) PortIdx=%d\n", __FUNCTION__, __LINE__, idx, i, u32PortIdx);
                        return -EFAULT;
                    }
                }
            }
        }

#ifdef MHAL_HDMITX_EN
        if(stDispCfg.stDeviceCfg[0].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
        {
            MHalHdmitxInit();
        }
#endif
        bCreate = TRUE;
    }

    if(bAttachedDone)
    {
        MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[0], gstModeuleCtx.pDeviceCtx[0]);
        MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[1], gstModeuleCtx.pDeviceCtx[0]);
    }

    for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
    {

        MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[0]);
        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], FALSE);

        for(i=0;i<16;i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], FALSE);
            }
        }
    }



    for(idx=0; idx<stDispCfg.u8DeivceNum; idx++)
    {
        if(stDispCfg.stDeviceCfg[idx].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
        {
            for(interfaceidx = E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1 ; interfaceidx < E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM; interfaceidx++)
            {
                MS_U32 u32Interface = 0;
                stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[interfaceidx]);

                if(stTimingInfo.eTimeType != E_MHAL_DISP_OUTPUT_MAX)
                {
                    if(stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_PAL ||  stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_NTSC )
                    {
                        u32Interface |= MHAL_DISP_INTF_CVBS;
                        MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                        MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_CVBS, &stTimingInfo);

                    }
                    else
                    {
                        u32Interface |= MHAL_DISP_INTF_HDMI;
                        MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                        MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_HDMI, &stTimingInfo);
    #ifdef MHAL_HDMITX_EN
                        MHalHdmitxChangeResolution(stTimingInfo.eTimeType);
    #endif
                    }
                }
            }
        }
        else
        {
            stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1]);

            MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_VGA);
            MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx],
                                            MHAL_DISP_INTF_VGA,
                                            &stTimingInfo);
        }
        MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], TRUE);
    }


    for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
    {
        stVidLayerAttr.stVidLayerSize.u32Width  = stDispCfg.stVideoLayerCfg[idx].u16Width;
        stVidLayerAttr.stVidLayerSize.u32Height = stDispCfg.stVideoLayerCfg[idx].u16Height;
        stVidLayerAttr.stVidLayerDispWin.u16X   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16X;
        stVidLayerAttr.stVidLayerDispWin.u16Y   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Y;
        stVidLayerAttr.stVidLayerDispWin.u16Width  = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Width;
        stVidLayerAttr.stVidLayerDispWin.u16Height = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Height;
        stVidLayerAttr.ePixFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.enPixelFormat[idx]);
        MHAL_DISP_VideoLayerSetAttr(gstModeuleCtx.pVideoLayerCtx[idx], &stVidLayerAttr);
        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], TRUE);

        for(i=0;i<16;i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
            }
        }

        MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[idx]);
        for(i=0; i<16; i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                stInportAttr.stDispWin.u16X       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16X;
                stInportAttr.stDispWin.u16Y       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Y;
                stInportAttr.stDispWin.u16Height  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;
                stInportAttr.stDispWin.u16Width   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                MHAL_DISP_InputPortSetAttr(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stInportAttr);

                stVideoFrameData.eCompressMode = 0;
                stVideoFrameData.ePixelFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.stInputPortCfg[idx][u8InputPortId].enPixelFormat);

                stVideoFrameData.u32Width  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                stVideoFrameData.u32Height = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;

                stVideoFrameData.au32Stride[0] = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32Stride;
                stVideoFrameData.aPhyAddr[0]   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32BaseAddr;
                MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stVideoFrameData);
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
            }
        } // for(i=0; i<16; i++)

        MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[idx]);
    }

    if(bAttachedDone)
    {
        MHAL_DISP_DeviceDetach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
    }

    MHAL_DISP_DeviceAttach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
    bAttachedDone = 1;


    SIMMI_DBG(1, "-----------------------------------------------------------\n");
    SIMMI_DBG(1, "       Delay %d Sec\n", stDispCfg.u32DemoTimeSec);
    SIMMI_DBG(1, "-----------------------------------------------------------\n");

    if(stDispCfg.u32DemoTimeSec != 112233)
    {
        msleep(stDispCfg.u32DemoTimeSec*1000);


        MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], FALSE);
        MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[1], FALSE);

        MHAL_DISP_DeviceDetach(gstModeuleCtx.pDeviceCtx[0], gstModeuleCtx.pDeviceCtx[1]);
        bAttachedDone = 0;

        for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
        {
            MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[0]);

            // Destory InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                    if(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx])
                    {
                        if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                        {
                            SIMMI_DBG(1, "InputPort Destory Fail, i=%d, port=%d\n", i, u32PortIdx);
                            return -EFAULT;
                        }
                    }
                    gstModeuleCtx.pInputPortCtx[idx][u32PortIdx] = NULL;
                }
            }

             // Desotry VideoLayer Context
             if(gstModeuleCtx.pVideoLayerCtx[idx])
             {
                 if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                 {
                     SIMMI_DBG(1, "VideoLayer Destory fail\n");
                     return -EFAULT;
                 }
                 gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
             }
        }

    #ifdef MHAL_HDMITX_EN
        MHalHdmitxPowerOff(0);
    #endif

        // Desotry Device Context
        for(idx=0; idx<stDispCfg.u8DeivceNum; idx++)
        {
            if(gstModeuleCtx.pDeviceCtx[idx])
            {
                if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                {
                    SIMMI_DBG(1, "Device Destory fail\n");
                    return -EFAULT;
                }
               gstModeuleCtx.pDeviceCtx[idx] = NULL;
            }
        }
        bCreate = 0;

    }
    return 0;
}

int _SimMiModuleDisp32Mgwin(SimMiModuleDispConfig_t stDispCfg)
{
    MHAL_DISP_AllocPhyMem_t StAlloc;
    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;
    MS_U32 u32DevIrq[E_SIMMI_MODULE_DEVICE_ID_NUM];
    MHAL_DISP_IRQFlag_t stIrqFlag;
    MS_U16 i,idx, interfaceidx;
    static MS_BOOL bCreate = FALSE;
    MS_U8 u8InputPortId;

    if(stDispCfg.u8VideoLayerNum != 2)
    {
        return -EFAULT;
    }


#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    if(bCreate == FALSE)
    {
        if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[0].enDeviceId, &gstModeuleCtx.pDeviceCtx[0]) == FALSE)
        {
            SIMMI_DBG(1, "%s %d, CreaetInstance fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stDeviceCfg[0].enDeviceId);
            return -EFAULT;
        }

        gstModeuleCtx.u32DevcieId[0] = stDispCfg.stDeviceCfg[0].enDeviceId;

        for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
        {
            // Create VideoLayer Context
            if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
            {
                SIMMI_DBG(1, "%s %d, CreateVideoLayer fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId);
                return -EFAULT;
            }
            gstModeuleCtx.u32VideoLayerId[idx] = stDispCfg.stVideoLayerCfg[idx].enVideoLayerId;

            // Create InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                    if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[idx], u32PortIdx, &gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                    {
                        SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d %d) PortIdx=%d\n", __FUNCTION__, __LINE__, idx, i, u32PortIdx);
                        return -EFAULT;
                    }
                }
            }
        }

    #ifdef MHAL_HDMITX_EN
        if(stDispCfg.stDeviceCfg[0].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
        {
            MHalHdmitxInit();
        }
    #endif
        bCreate = TRUE;
    }

    for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
    {
        MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[0]);
        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], FALSE);

        for(i=0;i<16;i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], FALSE);
            }
        }
    }

    MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], FALSE);

    for(interfaceidx = E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1 ; interfaceidx < E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM; interfaceidx++)
    {
        MS_U32 u32Interface = 0;
        stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[0].enTiming[interfaceidx]);

        if(stTimingInfo.eTimeType != E_MHAL_DISP_OUTPUT_MAX)
        {
            if(stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_PAL ||  stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_NTSC )
            {
                u32Interface |= MHAL_DISP_INTF_CVBS;
                MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[0], u32Interface);
                MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[0], MHAL_DISP_INTF_CVBS, &stTimingInfo);
            }
            else
            {
                u32Interface |= MHAL_DISP_INTF_HDMI;
                MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[0], u32Interface);
                MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[0], MHAL_DISP_INTF_HDMI, &stTimingInfo);
            #ifdef MHAL_HDMITX_EN
                MHalHdmitxChangeResolution(stTimingInfo.eTimeType);
            #endif

            }
        }
    }

    MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], TRUE);

    MHAL_DISP_GetDevIrq(gstModeuleCtx.pDeviceCtx[0], &u32DevIrq[0]);

    for(idx=0; idx<stDispCfg.u8VideoLayerNum ; idx++)
    {
        stVidLayerAttr.stVidLayerSize.u32Width  = stDispCfg.stVideoLayerCfg[idx].u16Width;
        stVidLayerAttr.stVidLayerSize.u32Height = stDispCfg.stVideoLayerCfg[idx].u16Height;
        stVidLayerAttr.stVidLayerDispWin.u16X   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16X;
        stVidLayerAttr.stVidLayerDispWin.u16Y   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Y;
        stVidLayerAttr.stVidLayerDispWin.u16Width  = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Width;
        stVidLayerAttr.stVidLayerDispWin.u16Height = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Height;
        stVidLayerAttr.ePixFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.enPixelFormat[idx]);
        MHAL_DISP_VideoLayerSetAttr(gstModeuleCtx.pVideoLayerCtx[idx], &stVidLayerAttr);
        MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], TRUE);

        for(i=0;i<16;i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
            }
        }

        MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[idx]);
        for(i=0; i<16; i++)
        {
            if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
            {
                u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                stInportAttr.stDispWin.u16X       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16X;
                stInportAttr.stDispWin.u16Y       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Y;
                stInportAttr.stDispWin.u16Height  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;
                stInportAttr.stDispWin.u16Width   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                MHAL_DISP_InputPortSetAttr(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stInportAttr);

                stVideoFrameData.eCompressMode = 0;
                stVideoFrameData.ePixelFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.stInputPortCfg[idx][u8InputPortId].enPixelFormat);

                stVideoFrameData.u32Width  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                stVideoFrameData.u32Height = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;

                stVideoFrameData.au32Stride[0] = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32Stride;
                stVideoFrameData.aPhyAddr[0]   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32BaseAddr;
                MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stVideoFrameData);
                MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
            }
        } // for(i=0; i<16; i++)

        MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[idx]);
    }


    MHAL_DISP_EnableDevIrq(gstModeuleCtx.pDeviceCtx[0], u32DevIrq[0], 1);

    MHAL_DISP_GetDevIrqFlag(gstModeuleCtx.pDeviceCtx[0], &stIrqFlag);

    SIMMI_DBG(1, "-----------------------------------------------------------\n");
    SIMMI_DBG(1, "Irq_Msk=%x, Irq_Flag=%x\n", stIrqFlag.u32IrqMask, stIrqFlag.u32IrqFlag);
    SIMMI_DBG(1, "-----------------------------------------------------------\n");

    MHAL_DISP_ClearDevInterrupt(gstModeuleCtx.pDeviceCtx[0], (void *)&stIrqFlag);


    SIMMI_DBG(1, "-----------------------------------------------------------\n");
    SIMMI_DBG(1, "       Delay %d Sec\n", stDispCfg.u32DemoTimeSec);
    SIMMI_DBG(1, "-----------------------------------------------------------\n");



    if(stDispCfg.u32DemoTimeSec != 112233)
    {
        msleep(stDispCfg.u32DemoTimeSec*1000);

        MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[0], FALSE);

        MHAL_DISP_EnableDevIrq(gstModeuleCtx.pDeviceCtx[0], u32DevIrq[0], 0);

        for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
        {
            MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[0]);


            // Destory InpuPort Context
            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                    if(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx])
                    {
                        if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                        {
                            SIMMI_DBG(1, "InputPort Destory Fail, i=%d, port=%d\n", i, u32PortIdx);
                            return -EFAULT;
                        }
                    }
                    gstModeuleCtx.pInputPortCtx[idx][u32PortIdx] = NULL;
                }
            }

             // Desotry VideoLayer Context
             if(gstModeuleCtx.pVideoLayerCtx[idx])
             {
                 if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                 {
                     SIMMI_DBG(1, "VideoLayer Destory fail\n");
                     return -EFAULT;
                 }
                 gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
             }
        }

        #ifdef MHAL_HDMITX_EN
                MHalHdmitxPowerOff(0);
        #endif

            // Desotry Device Context
            if(gstModeuleCtx.pDeviceCtx[0])
            {
                if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[0]) == FALSE)
                {
                    SIMMI_DBG(1, "Device Destory fail\n");
                    return -EFAULT;
                }
               gstModeuleCtx.pDeviceCtx[0] = NULL;
            }

        bCreate = FALSE;
    }

    return 0;
}

int _SimMiModuleDispLcd(SimMiModuleDispConfig_t stDispCfg)
{
    MHAL_DISP_AllocPhyMem_t StAlloc;
    MS_U16 i,idx, interfaceidx;
    static MS_BOOL bCreate[E_SIMMI_MODULE_DEVICE_ID_NUM] = {0, 0};

    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_SyncInfo_t stSynInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;
    MS_U8 u8InputPortId;

#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    if(stDispCfg.u8DeivceNum != 1)
    {
        SIMMI_DBG(1, "%s %d, DeviceNum > \n", __FUNCTION__, __LINE__);
        return 1;
    }

    if(stDispCfg.u8VideoLayerNum == stDispCfg.u8DeivceNum)
    {
        for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
        {
            if(bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] == 0)
            {

                // Create Device Context
                if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[idx].enDeviceId, &gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                {
                    SIMMI_DBG(1, "%s %d, CreaetInstance fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stDeviceCfg[idx].enDeviceId);
                    return -EFAULT;
                }
                gstModeuleCtx.u32DevcieId[idx] = stDispCfg.stDeviceCfg[idx].enDeviceId;


                // Create VideoLayer Context
                if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                {
                    SIMMI_DBG(1, "%s %d, CreateVideoLayer fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId);
                    return -EFAULT;
                }
                gstModeuleCtx.u32VideoLayerId[idx] = stDispCfg.stVideoLayerCfg[idx].enVideoLayerId;

                // Create InpuPort Context
                for(i=0; i<16; i++)
                {
                    if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                    {
                        MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                        if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[idx], u32PortIdx, &gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                        {
                            SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d %d) PortIdx=%d\n", __FUNCTION__, __LINE__, idx, i, u32PortIdx);
                            return -EFAULT;
                        }
                    }
                }

                bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] = 1;

            }

        #ifdef MHAL_HDMITX_EN
            MHalHdmitxInit();
        #endif

            MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[idx]);
            MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], FALSE);


            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                    MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], FALSE);
                }
            }

            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], FALSE);

            if(stDispCfg.stDeviceCfg[idx].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
            {
                interfaceidx = E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1;

                {
                    MS_U32 u32Interface = 0;

                    stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[interfaceidx]);

                    if(stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_USER)
                    {
                        u32Interface |= MHAL_DISP_INTF_LCD;

                        memset(&stSynInfo, 0, sizeof(MHAL_DISP_SyncInfo_t));
                        stSynInfo.u16Vact = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Vactive;
                        stSynInfo.u16Vbb  = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Vbp;
                        stSynInfo.u16Vpw  = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Vpw;
                        stSynInfo.u16Vfb  =
                            stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Vtt - stSynInfo.u16Vact - stSynInfo.u16Vbb - stSynInfo.u16Vpw;


                        stSynInfo.u16Hact = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Hactive;
                        stSynInfo.u16Hbb  = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Hbp;
                        stSynInfo.u16Hpw  = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Hpw;
                        stSynInfo.u16Hfb  =
                            stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Htt - stSynInfo.u16Hact - stSynInfo.u16Hbb - stSynInfo.u16Hpw;

                        stSynInfo.u32FrameRate = stDispCfg.stDeviceCfg[idx].stUserTimingCfg[0].u16Fps;
                        stTimingInfo.pstSyncInfo = &stSynInfo;

                        MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                        MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_LCD, &stTimingInfo);

                    #ifdef MHAL_HDMITX_EN
                        MHalHdmitxChangeResolution(E_MHAL_DISP_OUTPUT_1080P60);
                    #endif
                    }
                    else
                    {
                        u32Interface |= MHAL_DISP_INTF_LCD;
                        MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                        MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_LCD, &stTimingInfo);

                    #ifdef MHAL_HDMITX_EN
                        MHalHdmitxChangeResolution(stTimingInfo.eTimeType);
                    #endif
                    }
                }
            }

            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], TRUE);

            stVidLayerAttr.stVidLayerSize.u32Width  = stDispCfg.stVideoLayerCfg[idx].u16Width;
            stVidLayerAttr.stVidLayerSize.u32Height = stDispCfg.stVideoLayerCfg[idx].u16Height;
            stVidLayerAttr.stVidLayerDispWin.u16X   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16X;
            stVidLayerAttr.stVidLayerDispWin.u16Y   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Y;
            stVidLayerAttr.stVidLayerDispWin.u16Width  = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Width;
            stVidLayerAttr.stVidLayerDispWin.u16Height = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Height;
            stVidLayerAttr.ePixFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.enPixelFormat[idx]);
            MHAL_DISP_VideoLayerSetAttr(gstModeuleCtx.pVideoLayerCtx[idx], &stVidLayerAttr);


            MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], TRUE);


            MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[idx]);

            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;


                    stInportAttr.stDispWin.u16X       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16X;
                    stInportAttr.stDispWin.u16Y       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Y;
                    stInportAttr.stDispWin.u16Height  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;
                    stInportAttr.stDispWin.u16Width   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                    MHAL_DISP_InputPortSetAttr(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stInportAttr);

                    stVideoFrameData.eCompressMode = 0;
                    stVideoFrameData.ePixelFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.stInputPortCfg[idx][u8InputPortId].enPixelFormat);

                    stVideoFrameData.u32Width  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                    stVideoFrameData.u32Height = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;

                    stVideoFrameData.au32Stride[0] = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32Stride;
                    stVideoFrameData.aPhyAddr[0]   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32BaseAddr;
                    MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stVideoFrameData);
                    MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
                }
            } // for(i=0; i<16; i++)

            MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[idx]);
        } // idx


        SIMMI_DBG(1, "-----------------------------------------------------------\n");
        SIMMI_DBG(1, "       Delay %d Sec\n", stDispCfg.u32DemoTimeSec);
        SIMMI_DBG(1, "-----------------------------------------------------------\n");


        if(stDispCfg.u32DemoTimeSec != 112233)
        {
            msleep(stDispCfg.u32DemoTimeSec*1000);

            for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
            {

                MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], FALSE);

                MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[idx]);

                // Destory InpuPort Context
                for(i=0; i<16; i++)
                {
                    if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                    {
                        MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                        if(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx])
                        {
                            if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                            {
                                SIMMI_DBG(1, "InputPort Destory Fail, i=%d, port=%d\n", i, u32PortIdx);
                                return -EFAULT;
                            }
                        }
                        gstModeuleCtx.pInputPortCtx[idx][u32PortIdx] = NULL;
                    }
                }

                // Desotry VideoLayer Context
                if(gstModeuleCtx.pVideoLayerCtx[idx])
                {
                    if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "VideoLayer Destory fail\n");
                        return -EFAULT;
                    }
                    gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
                }


                #ifdef MHAL_HDMITX_EN
                    MHalHdmitxPowerOff(0);
                #endif

                // Desotry Device Context
                if(gstModeuleCtx.pDeviceCtx[idx])
                {
                    if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "Device Destory fail\n");
                        return -EFAULT;
                    }
                   gstModeuleCtx.pDeviceCtx[idx] = NULL;
                }

                bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] = 0;
            }

        }
    }
    else
    {
        SIMMI_DBG(1, "DeviceNum != VideoLayerNum (%d != %d)\n", stDispCfg.u8DeivceNum, stDispCfg.u8VideoLayerNum);
    }

    return 0;
}


int _SimMiModuleDispNormlCase(SimMiModuleDispConfig_t stDispCfg)
{
    MHAL_DISP_AllocPhyMem_t StAlloc;
    MS_U16 i,idx, interfaceidx;
    MS_U32 u32DevIrq[E_SIMMI_MODULE_DEVICE_ID_NUM];
    static MS_BOOL bCreate[E_SIMMI_MODULE_DEVICE_ID_NUM] = {0, 0};
    static MS_BOOL bBind[E_SIMMI_MODULE_DEVICE_ID_NUM] = {0, 0};
    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;
    MS_U8 u8InputPortId;
    MS_S32 s32IrqRet;
    static struct task_struct *pFlipTask[E_SIMMI_MODULE_DEVICE_ID_NUM];

#ifdef MI_ENABLE
    StAlloc.alloc = mi_sys_MMA_Alloc;
    StAlloc.free = mi_sys_MMA_Free;
#else
    StAlloc.alloc =  _SimMiModuleDispAllocDmem;
    StAlloc.free =  _SimMiModuleDispFreeDmem;
#endif

    if(stDispCfg.u8VideoLayerNum == stDispCfg.u8DeivceNum)
    {
        for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
        {
            if(bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] == 0)
            {
                // Create Device Context
                if(MHAL_DISP_DeviceCreateInstance(&StAlloc, stDispCfg.stDeviceCfg[idx].enDeviceId, &gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                {
                    SIMMI_DBG(1, "%s %d, CreaetInstance fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stDeviceCfg[idx].enDeviceId);
                    return -EFAULT;
                }
                gstModeuleCtx.u32DevcieId[idx] = stDispCfg.stDeviceCfg[idx].enDeviceId;

                // Create VideoLayer Context
                if(MHAL_DISP_VideoLayerCreateInstance(&StAlloc, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId ,&gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                {
                    SIMMI_DBG(1, "%s %d, CreateVideoLayer fail i=%d, id=%d\n", __FUNCTION__, __LINE__, i, stDispCfg.stVideoLayerCfg[idx].enVideoLayerId);
                    return -EFAULT;
                }
                gstModeuleCtx.u32VideoLayerId[idx] = stDispCfg.stVideoLayerCfg[idx].enVideoLayerId;

                // Create InpuPort Context
                for(i=0; i<16; i++)
                {
                    if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                    {
                        MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                        if(MHAL_DISP_InputPortCreateInstance(&StAlloc, gstModeuleCtx.pVideoLayerCtx[idx], u32PortIdx, &gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                        {
                            SIMMI_DBG(1, "%s %d, CreateInputPort fail (%d %d) PortIdx=%d\n", __FUNCTION__, __LINE__, idx, i, u32PortIdx);
                            return -EFAULT;
                        }
                    }
                }

                if(stDispCfg.bIsr)
                {
                    MHAL_DISP_GetDevIrq(gstModeuleCtx.pDeviceCtx[idx], &u32DevIrq[idx]);

                    gstIsrCfg[idx].u32DeviceId = gstModeuleCtx.u32DevcieId[idx];
                    gstIsrCfg[idx].u32VideoLyaerId = gstModeuleCtx.u32VideoLayerId[idx];
                    gstIsrCfg[idx].u32InputPortFlag = stDispCfg.u16InputPortFlag[idx];

                    gstIsrCfg[idx].u32BufferStart = stDispCfg.u32ImageBufferAddr;
                    gstIsrCfg[idx].u32BufferSize  = stDispCfg.u32ImageBufferSize;

                    for(i=0; i<16; i++)
                    {
                        if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                        {
                            gstIsrCfg[idx].u32InputPortId[i] = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                        }
                    }

                    s32IrqRet = request_irq(u32DevIrq[idx], _SimMiModuleDispDevIsr, IRQF_SHARED | IRQF_ONESHOT, "simmi_disp_isr", &gstIsrCfg[idx]);

                    pFlipTask[idx] = kthread_create(_SimMiModuleDispFlipThread, &gstIsrCfg[idx], "simmi_disp_thread");
                    wake_up_process(pFlipTask[idx]);

                    if(s32IrqRet)
                    {
                        SIMMI_DBG(1, "%s %d, CreateIsr  fail, dev=%d\n", __FUNCTION__, __LINE__, idx);
                    }
                }

            #ifdef MHAL_HDMITX_EN
                if(stDispCfg.stDeviceCfg[idx].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
                {
                    MHalHdmitxInit();
                }
            #endif

                bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] = 1;
            }

            if(bBind[idx] == 0)
            {
                MHAL_DISP_VideoLayerBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[idx]);
                MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], FALSE);
                bBind[idx] = 1;
            }

            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;
                    MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], FALSE);
                }
            }

            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], FALSE);

            if(stDispCfg.stDeviceCfg[idx].enDeviceId == E_SIMMI_MODULE_DEVICE_ID_0)
            {
                for(interfaceidx = E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1 ; interfaceidx < E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_NUM; interfaceidx++)
                {
                    MS_U32 u32Interface = 0;
                    stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[interfaceidx]);

                    // HVSwap,
                    _SimMiModulePnlHVSwap(stDispCfg.bHVSwap, stDispCfg.stDeviceCfg[idx].enTiming[interfaceidx]);

                    if(stTimingInfo.eTimeType != E_MHAL_DISP_OUTPUT_MAX)
                    {
                        if(stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_PAL ||  stTimingInfo.eTimeType == E_MHAL_DISP_OUTPUT_NTSC )
                        {
                            u32Interface |= MHAL_DISP_INTF_CVBS;
                            MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                            MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_CVBS, &stTimingInfo);

                        }
                        else
                        {
                            u32Interface |= MHAL_DISP_INTF_HDMI;
                            MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], u32Interface);
                            MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_HDMI, &stTimingInfo);
                        #ifdef MHAL_HDMITX_EN
                            MHalHdmitxChangeResolution(stTimingInfo.eTimeType);
                        #endif
                        }
                    }
                }
            }
            else
            {
                stTimingInfo.eTimeType = _SimMiModuleDispTransTiming(stDispCfg.stDeviceCfg[idx].enTiming[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1]);

                // HVSwap,
                _SimMiModulePnlHVSwap(stDispCfg.bHVSwap, stDispCfg.stDeviceCfg[idx].enTiming[E_SIMMI_MODULE_DEVICE_OUTPUT_INTERFACE_ID_1]);

                MHAL_DISP_DeviceAddOutInterface(gstModeuleCtx.pDeviceCtx[idx], MHAL_DISP_INTF_VGA);
                MHAL_DISP_DeviceSetOutputTiming(gstModeuleCtx.pDeviceCtx[idx],
                                                MHAL_DISP_INTF_VGA,
                                                &stTimingInfo);
            }

            MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], TRUE);

            stVidLayerAttr.stVidLayerSize.u32Width  = stDispCfg.stVideoLayerCfg[idx].u16Width;
            stVidLayerAttr.stVidLayerSize.u32Height = stDispCfg.stVideoLayerCfg[idx].u16Height;
            stVidLayerAttr.stVidLayerDispWin.u16X   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16X;
            stVidLayerAttr.stVidLayerDispWin.u16Y   = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Y;
            stVidLayerAttr.stVidLayerDispWin.u16Width  = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Width;
            stVidLayerAttr.stVidLayerDispWin.u16Height = stDispCfg.stVideoLayerCfg[idx].stDispRect.u16Height;
            stVidLayerAttr.ePixFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.enPixelFormat[idx]);
            MHAL_DISP_VideoLayerSetAttr(gstModeuleCtx.pVideoLayerCtx[idx], &stVidLayerAttr);

            MHAL_DISP_VideoLayerEnable(gstModeuleCtx.pVideoLayerCtx[idx], TRUE);

            MHAL_DISP_InputPortAttrBegin(gstModeuleCtx.pVideoLayerCtx[idx]);

            for(i=0; i<16; i++)
            {
                if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                {
                    u8InputPortId = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                    stInportAttr.stDispWin.u16X       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16X;
                    stInportAttr.stDispWin.u16Y       = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Y;
                    stInportAttr.stDispWin.u16Height  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;
                    stInportAttr.stDispWin.u16Width   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                    MHAL_DISP_InputPortSetAttr(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stInportAttr);

                    stVideoFrameData.eCompressMode = 0;
                    stVideoFrameData.ePixelFormat = _SimMiModuleDispTransPixelFormat(stDispCfg.stInputPortCfg[idx][u8InputPortId].enPixelFormat);

                    stVideoFrameData.u32Width  = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Width;
                    stVideoFrameData.u32Height = stDispCfg.stInputPortCfg[idx][u8InputPortId].u16Height;

                    stVideoFrameData.au32Stride[0] = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32Stride;
                    stVideoFrameData.aPhyAddr[0]   = stDispCfg.stInputPortCfg[idx][u8InputPortId].u32BaseAddr;

                    if(stDispCfg.bIsr)
                    {
                        memcpy(&gstIsrCfg[idx].stVideoFrame[i], &stVideoFrameData, sizeof(MHAL_DISP_VideoFrameData_t));
                    }

                    MHAL_DISP_InputPortFlip(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], &stVideoFrameData);
                    MHAL_DISP_InputPortEnable(gstModeuleCtx.pInputPortCtx[idx][u8InputPortId], TRUE);
                }
            } // for(i=0; i<16; i++)

            MHAL_DISP_InputPortAttrEnd(gstModeuleCtx.pVideoLayerCtx[idx]);
        } // idx

        SIMMI_DBG(1, "-----------------------------------------------------------\n");
        SIMMI_DBG(1, "       Delay %d Sec\n", stDispCfg.u32DemoTimeSec);
        SIMMI_DBG(1, "-----------------------------------------------------------\n");

        if(stDispCfg.bIsr)
        {
            for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
            {
                MHAL_DISP_EnableDevIrq(gstModeuleCtx.pDeviceCtx[idx], u32DevIrq[idx], 1);
            }
        }

    #if 0
        //MDrv_VE_SetWSS525Data(1, 0xFFFFFFFF);

        //MDrv_VE_SetWSSData(1, 0xFFFF);

        MDrv_VE_SetTtxBuffer(0x24701100, 0x1000);
        MDrv_VE_SetVbiTtxActiveLines(0xFF);
        MDrv_VE_EnableTtx(1);
    #endif

        if(stDispCfg.u32DemoTimeSec != 112233)
        {
            msleep(stDispCfg.u32DemoTimeSec*1000);

            for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
            {
                MHAL_DISP_EnableDevIrq(gstModeuleCtx.pDeviceCtx[idx], u32DevIrq[idx], 0);

                msleep(500);

                if(stDispCfg.bIsr)
                {
                    kthread_stop(pFlipTask[idx]);
                    free_irq(u32DevIrq[idx], &gstIsrCfg[idx]);
                }
            }

            for(idx=0; idx<stDispCfg.u8VideoLayerNum; idx++)
            {
                MHAL_DISP_DeviceEnable(gstModeuleCtx.pDeviceCtx[idx], FALSE);

                MHAL_DISP_VideoLayerUnBind(gstModeuleCtx.pVideoLayerCtx[idx], gstModeuleCtx.pDeviceCtx[idx]);
                bBind[idx] = 0;

                // Destory InpuPort Context
                for(i=0; i<16; i++)
                {
                    if(stDispCfg.u16InputPortFlag[idx] & (1<<i))
                    {
                        MS_U32 u32PortIdx = stDispCfg.stInputPortCfg[idx][i].u8PortId;

                        if(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx])
                        {
                            if(MHAL_DISP_InputPortDestroyInstance(gstModeuleCtx.pInputPortCtx[idx][u32PortIdx]) == FALSE)
                            {
                                SIMMI_DBG(1, "InputPort Destory Fail, i=%d, port=%d\n", i, u32PortIdx);
                                return -EFAULT;
                            }
                        }
                        gstModeuleCtx.pInputPortCtx[idx][u32PortIdx] = NULL;
                    }
                }

                // Desotry VideoLayer Context
                if(gstModeuleCtx.pVideoLayerCtx[idx])
                {
                    if(MHAL_DISP_VideoLayerDestoryInstance(gstModeuleCtx.pVideoLayerCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "VideoLayer Destory fail\n");
                        return -EFAULT;
                    }
                    gstModeuleCtx.pVideoLayerCtx[idx] = NULL;
                }

            #ifdef MHAL_HDMITX_EN
                MHalHdmitxPowerOff(0);
            #endif

                // Desotry Device Context
                if(gstModeuleCtx.pDeviceCtx[idx])
                {
                    if(MHAL_DISP_DeviceDestroyInstance(gstModeuleCtx.pDeviceCtx[idx]) == FALSE)
                    {
                        SIMMI_DBG(1, "Device Destory fail\n");
                        return -EFAULT;
                    }
                   gstModeuleCtx.pDeviceCtx[idx] = NULL;
                }

                bCreate[stDispCfg.stDeviceCfg[idx].enDeviceId] = 0;
            }

        }
    }
    else
    {
        SIMMI_DBG(1, "DeviceNum != VideoLayerNum (%d != %d)\n", stDispCfg.u8DeivceNum, stDispCfg.u8VideoLayerNum);
    }

    return 0;

}

int _SimMiModuleIoctlDispConfig(struct file *filp, unsigned long arg)
{
    SimMiModuleDispConfig_t stDispCfg;
    if(copy_from_user(&stDispCfg, (SimMiModuleDispConfig_t __user *)arg, sizeof(SimMiModuleDispConfig_t)))
    {
        return -EFAULT;
    }
    else
    {

        if(stDispCfg.bMgwin32En && stDispCfg.bAttached)
        {
            _SimMiModuleDisp32MgwinAttach(stDispCfg);
        }
        else if(stDispCfg.bMgwin32En)
        {
            return _SimMiModuleDisp32Mgwin(stDispCfg);
        }
        else if(stDispCfg.bAttached)
        {
            return _SimMiModuleDispAttach(stDispCfg);
        }
        else if(stDispCfg.bLCD)
        {
            return _SimMiModuleDispLcd(stDispCfg);
        }
        else
        {
            return _SimMiModuleDispNormlCase(stDispCfg);
        }
    }

    return 0;
}

MS_BOOL _SimMiMoudleInitDemCfg(void)
{
    memset(&stDmemCfg, 0, sizeof(SimMiModuleDmemConfig_t)*20);
    memset(&gstModeuleCtx, 0, sizeof(SimMiModuleCtxConfig_t));
    memset(&gstPnlHVSwapCfg, 0, sizeof(SimMiModulePanelConfig_t)*E_SIMMI_MODULE_PNL_TIMING_MAX_NUM);
    memset(&gstDbgMsgCfg, 0, sizeof(SimMiModuleDbgMsgConfig_t));
    return TRUE;
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

void _SimMiModuleDbgmsgShow(char *buf)
{
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "XC:: %08x\n", gstDbgMsgCfg.u32XC);
    SIMMI_SPRINTF(buf, "  SetWindows: 0x0001\n");
    SIMMI_SPRINTF(buf, "   SetTiming: 0x0002\n");
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "HDMITx:: %08x\n", gstDbgMsgCfg.u32Hdmitx);
    SIMMI_SPRINTF(buf, "  HDMITx: 0x0001\n");
    SIMMI_SPRINTF(buf, "    HDCP: 0x0002\n");
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "VE:: %08x\n", gstDbgMsgCfg.u32VE);
    SIMMI_SPRINTF(buf, "  ON: 0x0001\n");
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "Mgwin::\n");
    SIMMI_SPRINTF(buf, "        IOCTL: %08x\n", gstDbgMsgCfg.u32Mgwin[0]);
    SIMMI_SPRINTF(buf, "         MDRV: %08x\n", gstDbgMsgCfg.u32Mgwin[1]);
    SIMMI_SPRINTF(buf, "     PRIORITY: %08x\n", gstDbgMsgCfg.u32Mgwin[2]);
    SIMMI_SPRINTF(buf, "  DRVSCLMGWIN: %08x\n", gstDbgMsgCfg.u32Mgwin[3]);
    SIMMI_SPRINTF(buf, "        Frame: --01\n");
    SIMMI_SPRINTF(buf, "       SubWin: --02\n");
    SIMMI_SPRINTF(buf, "===========================\n");
    SIMMI_SPRINTF(buf, "mhal_disp:: %08x\n", gstDbgMsgCfg.u32MhalDisp);
    SIMMI_SPRINTF(buf, "       FUNC: 0x00000001\n");
    SIMMI_SPRINTF(buf, "  InputPort: 0x00000002\n");
    SIMMI_SPRINTF(buf, "       Mute: 0x00000004\n");
    SIMMI_SPRINTF(buf, "  Interrupt: 0x00000008\n");
    SIMMI_SPRINTF(buf, "         XC: 0x00000010\n");
    SIMMI_SPRINTF(buf, "  CheckFunc: 0x00000020\n");
    SIMMI_SPRINTF(buf, "       Flip: 0x00000040\n");
}



void _SimMiModuleDbgmsgStore(char *buf)
{
    char *argv[100];
    int argc;
    char *del = " ";
    MS_U32 u32DbgLevel;
    int ret;

    argc = _SimMiModuleSplit(argv, buf, del);
    if(argc == 2)
    {
        ret = kstrtol(argv[2], 16, (long *)&u32DbgLevel);

        if(strcmp(argv[1], "mhal_disp") == 0)
        {
            MHAL_DISP_DbgLevel(&u32DbgLevel);
            gstDbgMsgCfg.u32MhalDisp = u32DbgLevel;

        }
        else if(strcmp(argv[1], "XC") == 0)
        {
            MApi_XC_EX_SetDbgLevel(u32DbgLevel);
            gstDbgMsgCfg.u32XC = u32DbgLevel;

        }
        else if(strcmp(argv[1], "HDMITx") == 0)
        {
        #ifdef MHAL_HDMITX_EN
            MApi_HDMITx_SetDbgLevel(u32DbgLevel);
        #endif
            gstDbgMsgCfg.u32Hdmitx = u32DbgLevel;
        }
        else if(strcmp(argv[1], "VE") == 0)
        {
            MDrv_VE_SetDbgLevel(u32DbgLevel ? 1 : 0);
            gstDbgMsgCfg.u32VE = u32DbgLevel;
        }
        else
        {

        }
    }
    else if(argc == 3)
    {

       ret = kstrtol(argv[3], 16, (long *)&u32DbgLevel);

        if(strcmp(argv[1], "Mgwin") == 0)
        {
            DrvSclMgwinIoDebugConfig_t stDbgCfg;

            stDbgCfg.u8DbgOpt = (strcmp(argv[2], "IOCTL") == 0)    ? 0x00 :
                                (strcmp(argv[2], "MDRV") == 0)     ? 0x01 :
                                (strcmp(argv[2], "PRIORITY") == 0) ? 0x02 :
                                (strcmp(argv[2], "DRV") == 0)      ? 0x03 :
                                                                     0x04 ;

            if(stDbgCfg.u8DbgOpt < 0x04)
            {
                stDbgCfg.u8DbgLvl = (MS_U8)(u32DbgLevel & 0xFF);
                _DrvSclMgwinIoSetDebugConfig(0, &stDbgCfg);
                gstDbgMsgCfg.u32Mgwin[stDbgCfg.u8DbgOpt] = u32DbgLevel & 0xFF;
            }
        }


    }

}

