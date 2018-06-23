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
#include "apiXC_EX.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
//#include "mhal_disp.c"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SIMMI_MODULE_COUNT    1
#define DRV_SIMMI_MODULE_NAME     "mhal_disp"
#define DRV_SIMMI_MODULE_MAJOR    0xea
#define DRV_SIMMI_MODULE_MINOR    0x87

#define SIMMI_DBG(dbglv, _fmt, _args...)             \
    do                                             \
    if(dbglv)                                      \
    {                                              \
            printk(KERN_INFO _fmt, ## _args);       \
    }while(0)

#define SIMMI_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)


#define SIMMI_ERR(_fmt, _args...)       printk(KERN_ERR _fmt, ## _args)

#define SIMMI_ALIGN(x,y) ( x & ~(y-1))

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
int DrvSimMiModuleOpen(struct inode *inode, struct file *filp);
int DrvSimMiModuleRelease(struct inode *inode, struct file *filp);
long DrvSimMiModuleIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
static int DrvSimMiModuleProbe(struct platform_device *pdev);
static int DrvSimMiModuleRemove(struct platform_device *pdev);
static int DrvSimMiModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvSimMiModuleResume(struct platform_device *dev);
static unsigned int DrvSimMiModulePoll(struct file *filp, struct poll_table_struct *wait);
static void DrvSimMiModuleDeviceRelease(struct device *device);

int _SimMiModuleIoctlDispConfig(struct file *filp, unsigned long arg);
int _SimMiModuleIoctlSetDebugLevelConfig(struct file *filp, unsigned long arg);
int _SimMiModuleIoctlSetHdmitxConfig(struct file *filp, unsigned long arg);
MS_BOOL _SimMiMoudleInitDemCfg(void);
int _SimMiModuleIoctlDispPictureConfig(struct file *filp, unsigned long arg);


void _SimMiModuleDbgmsgStore(char *buf);
void _SimMiModuleDbgmsgShow(char *buf);

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
static DrvSclSimMiModuleDevice_t _tSimHiDevice =
{
    .s32Major = DRV_SIMMI_MODULE_MAJOR,
    .s32Minor = DRV_SIMMI_MODULE_MINOR,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SIMMI_MODULE_NAME, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSimMiModuleOpen,
        .release = DrvSimMiModuleRelease,
        .unlocked_ioctl = DrvSimMiModuleIoctl,
        .poll = DrvSimMiModulePoll,
    },
};

static struct class * _tSimHiClass = NULL;
static char * SimHiClassName = "SimMi";


static const struct of_device_id _SimHiMatchTable[] =
{
    { .compatible = "mstar,SimMiModule" },
    {}
};

static struct platform_driver stDrvSimHiPlatformDriver =
{
    .probe      = DrvSimMiModuleProbe,
    .remove     = DrvSimMiModuleRemove,
    .suspend    = DrvSimMiModuleSuspend,
    .resume     = DrvSimMiModuleResume,
    .driver =
    {
        .name   = DRV_SIMMI_MODULE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_SimHiMatchTable),
    },
};

static u64 u64SimHi_DmaMask = 0xffffffffUL;

static struct platform_device stDrvSimHiPlatformDevice =
{
    .name = DRV_SIMMI_MODULE_NAME,
    .id = 0,
    .dev =
    {
        .release =  DrvSimMiModuleDeviceRelease,
        .dma_mask = &u64SimHi_DmaMask,
        .coherent_dma_mask = 0xffffffffUL
    }
};


static ssize_t dbgmsg_call_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        _SimMiModuleDbgmsgStore((char *)buf);

        return n;
    }

    return 0;
}
static ssize_t dbgmsg_call_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char str[1024*3];

    _SimMiModuleDbgmsgShow(str);

    return sprintf(buf, "%s", str);

    //return sprintf(buf,"enter string\n");
}

static DEVICE_ATTR(dbgmsg, 0644, dbgmsg_call_show, dbgmsg_call_store);



//-------------------------------------------------------------------------------------------------
// Pirvate Function
//-------------------------------------------------------------------------------------------------


//==============================================================================
long DrvSimMiModuleIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int err = 0;
    int retval = 0;

    if(_tSimHiDevice.refCnt <= 0)
    {
        SIMMI_ERR( "[SimMiModule] HVSP1IO_IOCTL refCnt =%d!!! \n", _tSimHiDevice.refCnt);
        return -EFAULT;
    }
    /* check u32Cmd valid */
    if(IOCTL_SIMMI_MAGIC == _IOC_TYPE(u32Cmd))
    {
        if(_IOC_NR(u32Cmd) >= IOCTL_SIMMI_MAX_NR)
        {
            SIMMI_ERR( "[SimMiModule] IOCtl NR Error!!! (Cmd=%x)\n",u32Cmd);
            return -ENOTTY;
        }
    }
    else
    {
        SIMMI_ERR( "[SimMiModule] IOCtl MAGIC Error!!! (Cmd=%x)\n",u32Cmd);
        return -ENOTTY;
    }

    /* verify Access */
    if (_IOC_DIR(u32Cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    else if (_IOC_DIR(u32Cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    if (err)
    {
        return -EFAULT;
    }
    /* not allow query or command once driver suspend */
    switch(u32Cmd)
    {
        case IOCTL_SIMMI_DISP_CONFIG:
            retval = _SimMiModuleIoctlDispConfig(filp, u32Arg);
            break;

        case IOCTL_SIMMI_SET_DEBUG_LEVEL_CONFIG:
            retval = _SimMiModuleIoctlSetDebugLevelConfig(filp, u32Arg);
            break;

        case IOCTL_SIMMI_SET_HDMITX_CONFIG:
            retval = _SimMiModuleIoctlSetHdmitxConfig(filp, u32Arg);
            break;

        case IOCTL_SIMMI_SET_PICTURE_CONFIG:
            retval = _SimMiModuleIoctlDispPictureConfig(filp, u32Arg);
            break;

        default:
            SIMMI_ERR( "[SimMi] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
	  }

    return retval;
}


static unsigned int DrvSimMiModulePoll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int ret = 0;
    SIMMI_ERR("[SimMiModule]start %s ret=%x\n",__FUNCTION__,ret);
    return ret;
}


static int DrvSimMiModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);


    return ret;
}

static int DrvSimMiModuleResume(struct platform_device *dev)
{
    int ret = 0;

    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);


    return ret;
}


int DrvSimMiModuleOpen(struct inode *inode, struct file *filp)
{
    int ret = 0;
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);

    SIMMI_ASSERT(_tSimHiDevice.refCnt>=0);



    _tSimHiDevice.refCnt++;

    return ret;
}


int DrvSimMiModuleRelease(struct inode *inode, struct file *filp)
{
    int ret = 0;

    SIMMI_DBG(1, "[SCLDMA_DIPR] %s\n",__FUNCTION__);


    _tSimHiDevice.refCnt--;
    SIMMI_ASSERT(_tSimHiDevice.refCnt>=0);
    return ret;
}

void* DrvSimMiModuleDirectMemAlloc(char* name, MS_U32 size, dma_addr_t *addr)
{

    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length = size;
    if(0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }

    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);

}

MS_U32 DrvSimMiModulePa2Miu(dma_addr_t addr)
{
    return Chip_Phys_to_MIU(addr);
}


typedef struct
{
    void *pCtx;
}DrvSimMiModuelInputPortCtx;



static int DrvSimMiModuleProbe(struct platform_device *pdev)
{
#if 0
    MHAL_DISP_AllocPhyMem_t StAlloc;
    void *pDevCtx;
    void *pVidLayerCtx;
    DrvSimMiModuelInputPortCtx stInputPortCtx[16];
    MS_U32 u32DmemMiuAddr;
    MS_U32 i, x, y;
    MS_U32 Xwin, Ywin;

    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    MHAL_DISP_VideoLayerAttr_t stVidLayerAttr;
    MHAL_DISP_VgaParam_t  stVgaParam;
    MHAL_DISP_HdmiParam_t stHdmiParam;
    MHAL_DISP_CvbsParam_t stCvbsParam;
    MHAL_DISP_InputPortAttr_t stInportAttr;
    MHAL_DISP_VideoFrameData_t stVideoFrameData;



    #define DISP_WIDTH  720
    #define DISP_HEIGHT 480

    #define INPUT_PORT_WIDTH    180
    #define INPUT_PORT_HEIGHT   120

    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);

    u32DmemMiuAddr = 0x6040000;
    printk(KERN_EMERG "-------------------------- MIU ADDR =0x%08x -------------------------- \n", u32DmemMiuAddr);

    //memset(pMgwinVirAddr, 0x80, u32DmemMiuSize);
    //Device Create
    MHAL_DISP_DeviceCreateInstance(&StAlloc, 0, &pDevCtx);

    //VidLayer create
    MHAL_DISP_VideoLayerCreateInstance(&StAlloc, 0, &pVidLayerCtx);

    //Inputport
    for(i=0; i<16; i++)
    {
        MHAL_DISP_InputPortCreateInstance(&StAlloc, pVidLayerCtx, i, &stInputPortCtx[i].pCtx);
    }

    MHAL_DISP_DeviceSetBackGroundColor(pDevCtx, 0);

    MHAL_DISP_DeviceAddOutInterface(pDevCtx, MHAL_DISP_INTF_HDMI);


    //VidLayer Bind
    MHAL_DISP_VideoLayerBind(pVidLayerCtx, pDevCtx);

    stTimingInfo.eTimeType = E_MHAL_DISP_OUTPUT_480P60;
    MHAL_DISP_DeviceSetOutputTiming(pDevCtx, MHAL_DISP_INTF_HDMI, &stTimingInfo);

    stVidLayerAttr.ePixFormat = E_MHAL_DISP_PIXEL_FRAME_YUV422_YUYV;
    stVidLayerAttr.stVidLayerSize.u32Width = DISP_WIDTH;
    stVidLayerAttr.stVidLayerSize.u32Height = DISP_HEIGHT;

    stVidLayerAttr.stVidLayerDispWin.u16X = 0;
    stVidLayerAttr.stVidLayerDispWin.u16Y = 0;
    stVidLayerAttr.stVidLayerDispWin.u16Width  = DISP_WIDTH;
    stVidLayerAttr.stVidLayerDispWin.u16Height = DISP_HEIGHT;
    //MHAL_DISP_VideoLayerSetAttr(pVidLayerCtx, &stVidLayerAttr);

    Xwin = 4;
    Ywin = 4;
    for(x=0; x< Xwin; x++)
    {
        for(y=0; y<Ywin; y++)
        {

            stInportAttr.stDispWin.u16X       = 0 + x*SIMMI_ALIGN(DISP_WIDTH/Xwin, 16);
            stInportAttr.stDispWin.u16Y       = 0 + y*SIMMI_ALIGN(DISP_HEIGHT/Ywin, 2);
            stInportAttr.stDispWin.u16Height  =  SIMMI_ALIGN(DISP_HEIGHT/Ywin, 2);
            stInportAttr.stDispWin.u16Width   =  SIMMI_ALIGN(DISP_WIDTH/Xwin, 16);
            //MHAL_DISP_InputPortSetAttr(stInputPortCtx[x + y*Ywin].pCtx, &stInportAttr);
        }
    }

    stVideoFrameData.eCompressMode = 0;
    stVideoFrameData.ePixelFormat = E_MHAL_DISP_PIXEL_FRAME_YUV422_YUYV;
    //stVideoFrameData.u32Width  = SIMMI_ALIGN((DISP_WIDTH/2), 16);
    //stVideoFrameData.u32Height = SIMMI_ALIGN(DISP_HEIGHT, 2);
    stVideoFrameData.u32Width  = SIMMI_ALIGN(INPUT_PORT_WIDTH, 16);
    stVideoFrameData.u32Height = SIMMI_ALIGN(INPUT_PORT_HEIGHT, 2);

    stVideoFrameData.au32Stride[0] = SIMMI_ALIGN(INPUT_PORT_WIDTH, 16);
    for(i=0; i<Xwin*Ywin; i++)
    {
        stVideoFrameData.aPhyAddr[0] = u32DmemMiuAddr + i * (INPUT_PORT_WIDTH * INPUT_PORT_HEIGHT * 4);
        printk(KERN_EMERG "-------------------------- %d %08x -------------------------- \n",i, stVideoFrameData.aPhyAddr[0]);
        //MHAL_DISP_InputPortFlip(stInputPortCtx[i].pCtx, &stVideoFrameData);
        //MHAL_DISP_InputPortEnable(stInputPortCtx[i].pCtx, TRUE);
    }


    MHAL_DISP_DeviceEnable(pDevCtx, TRUE);

    stVgaParam.stCsc.u32Luma = 50;
    stVgaParam.stCsc.u32Hue  = 50;
    stVgaParam.stCsc.u32Saturation = 50;
    stVgaParam.stCsc.u32Contrast = 50;
    stVgaParam.u32Sharpness = 50;
    stVgaParam.u32Gain = 50;
    //MHAL_DISP_DeviceSetVgaParam(pDevCtx, &stVgaParam);

    stHdmiParam.stCsc.u32Luma = 50;
    stHdmiParam.stCsc.u32Hue  = 50;
    stHdmiParam.stCsc.u32Saturation = 50;
    stHdmiParam.stCsc.u32Contrast = 50;
    //MHAL_DISP_DeviceSetHdmiParam(pDevCtx, &stHdmiParam);

    stCvbsParam.bEnable = TRUE;
    //MHAL_DISP_DeviceSetCvbsParam(pDevCtx, &stCvbsParam);




    // UnBind
    //MHAL_DISP_VideoLayerUnBind(pVidLayerCtx, pDevCtx);

    //Inputport Destory
    for(i=0; i<Xwin*Ywin; i++)
    {
        //MHAL_DISP_InputPortDestroyInstance(stInputPortCtx[i].pCtx);
    }

    //VidLayer Destory
    //MHAL_DISP_VideoLayerDestoryInstance(pVidLayerCtx);

    //Device Destory
    //MHAL_DISP_DeviceDestroyInstance(pDevCtx);
#else
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);
    _SimMiMoudleInitDemCfg();

#endif

    return 0;
}
static int DrvSimMiModuleRemove(struct platform_device *pdev)
{
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);

    return 0;
}


static void DrvSimMiModuleDeviceRelease(struct device *device)
{
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
int _DrvSimMiModuleInit(void)
{
    int ret = 0;
    int s32Ret;
    dev_t  dev;
    //struct device_node *np;

    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);
    //np = of_find_compatible_node(NULL, NULL, "mstar,hvsp1");
    //if (np)
    //{
    //  SIMMI_DBG(1, "Find scl dts node\n");
    //  stDrvSimHiPlatformDevice.dev.of_node = of_node_get(np);
    //  of_node_put(np);
    //}
    //else
    //{
    //    return -ENODEV;
    //}

    if(_tSimHiDevice.s32Major)
    {
        dev     = MKDEV(_tSimHiDevice.s32Major, _tSimHiDevice.s32Minor);
        s32Ret  = register_chrdev_region(dev, DRV_SIMMI_MODULE_COUNT, DRV_SIMMI_MODULE_NAME);
    }
    else
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tSimHiDevice.s32Minor, DRV_SIMMI_MODULE_COUNT, DRV_SIMMI_MODULE_NAME);
        _tSimHiDevice.s32Major  = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        SIMMI_ERR( "[SimMiModule] Unable to get major %d\n", _tSimHiDevice.s32Major);
        return s32Ret;
    }

    cdev_init(&_tSimHiDevice.cdev, &_tSimHiDevice.fops);
    if (0 != (s32Ret= cdev_add(&_tSimHiDevice.cdev, dev, DRV_SIMMI_MODULE_COUNT)))
    {
        SIMMI_ERR( "[SimMiModule] Unable add a character device\n");
        unregister_chrdev_region(dev, DRV_SIMMI_MODULE_COUNT);
        return s32Ret;
    }

    _tSimHiClass = class_create(THIS_MODULE, SimHiClassName);
    if(IS_ERR(_tSimHiClass))
    {
        printk(KERN_ERR "Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        _tSimHiDevice.pDev = device_create(_tSimHiClass, NULL, dev,NULL, DRV_SIMMI_MODULE_NAME);
        _tSimHiDevice.pDev->dma_mask=&u64SimHi_DmaMask;
        _tSimHiDevice.pDev->coherent_dma_mask=u64SimHi_DmaMask;
    }

    ret = platform_driver_register(&stDrvSimHiPlatformDriver);

    if (!ret)
    {
        ret = platform_device_register(&stDrvSimHiPlatformDevice);
        if (ret)    // if register device fail, then unregister the driver.
        {
            platform_driver_unregister(&stDrvSimHiPlatformDriver);
            SIMMI_ERR( "[SimMiModule] platform_driver_register failed\n");

        }
        else
        {
            SIMMI_DBG(1, "[SimMiModule] platform_driver_register success\n");
        }
    }

    ret = device_create_file(_tSimHiDevice.pDev, &dev_attr_dbgmsg);

    if(ret != 0)
    {
        SIMMI_ERR( "[LKM] device_create_file failed\n");
    }



    return ret;
}
void _DrvSimMiModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    SIMMI_DBG(1, "[SimMiModule] %s\n",__FUNCTION__);

    cdev_del(&_tSimHiDevice.cdev);
    device_destroy(_tSimHiClass, MKDEV(_tSimHiDevice.s32Major, _tSimHiDevice.s32Minor));
    class_destroy(_tSimHiClass);
    unregister_chrdev_region(MKDEV(_tSimHiDevice.s32Major, _tSimHiDevice.s32Minor), DRV_SIMMI_MODULE_COUNT);
    platform_device_unregister(&stDrvSimHiPlatformDevice);
    platform_driver_unregister(&stDrvSimHiPlatformDriver);
}


module_init(_DrvSimMiModuleInit);
module_exit(_DrvSimMiModuleExit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("mstar scldma ioctrl driver");
MODULE_LICENSE("GPL");
