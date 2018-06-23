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

//#include <linux/pfn.h>
//#include <linux/errno.h>
//#include <linux/kernel.h>
//#include <linux/kthread.h>
//#include <linux/mm.h>
//#include <linux/slab.h>
//#include <linux/vmalloc.h>          /* seems do not need this */
//#include <linux/delay.h>
//#include <linux/interrupt.h>
//#include <linux/module.h>
//#include <asm/uaccess.h>
//#include <linux/fs.h>
//#include <asm/io.h>
//#include <asm/string.h>
//#include <linux/clk.h>
//#include <linux/clk-provider.h>

#include <linux/cdev.h>
//#include <linux/interrupt.h>
//#include <linux/poll.h>
#include <linux/module.h>
//#include <linux/moduleparam.h>
//#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
//#include <linux/delay.h>
//#include <linux/err.h>
//#include <linux/slab.h>
//#include <linux/version.h>
//#include <linux/gpio.h>
//#include <linux/irq.h>
//#include <linux/sched.h>
//#include <linux/wait.h>


#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "ms_platform.h"
#include "ms_msys.h"

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_m.h"

#include "drv_scl_ioctl.h"
#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_io_wrapper.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_dma_io_wrapper.h"
#include "drv_scl_vip_ioctl.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_m2m_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_m2m_io_wrapper.h"
#include "drv_scl_verchk.h"
#include "mhal_vpe.h"
#include "drv_scl_vip_io.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_SCL_DEVICE_COUNT    1
#define DRV_SCL_DEVICE_NAME     "mscl"
#define DRV_SCL_DEVICE_MINOR    0x10
#define DRV_SCLVIP_DEVICE_NAME     "msclvip"
#define DRV_SCLVIP_DEVICE_MINOR    0x11
#define DRV_SCLHVSP_DEVICE_NAME1     "msclhvsp1"
#define DRV_SCLHVSP_DEVICE_MINOR1    0x12
#define DRV_SCLHVSP_DEVICE_NAME2     "msclhvsp2"
#define DRV_SCLHVSP_DEVICE_MINOR2    0x13
#define DRV_SCLHVSP_DEVICE_NAME3     "msclhvsp3"
#define DRV_SCLHVSP_DEVICE_MINOR3    0x14
#define DRV_SCLHVSP_DEVICE_NAME4     "msclhvsp4"
#define DRV_SCLHVSP_DEVICE_MINOR4   0x15
#define DRV_SCLDMA_DEVICE_NAME1     "mscldma1"
#define DRV_SCLDMA_DEVICE_MINOR1    0x16
#define DRV_SCLDMA_DEVICE_NAME2     "mscldma2"
#define DRV_SCLDMA_DEVICE_MINOR2    0x17
#define DRV_SCLDMA_DEVICE_NAME3     "mscldma3"
#define DRV_SCLDMA_DEVICE_MINOR3    0x18
#define DRV_SCLDMA_DEVICE_NAME4     "mscldma4"
#define DRV_SCLDMA_DEVICE_MINOR4   0x19
#define DRV_SCLM2M_DEVICE_NAME     "msclm2m"
#define DRV_SCLM2M_DEVICE_MINOR    0x1a
#define DRV_SCL_DEVICE_MAJOR    0xea
//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------

int DrvSclModuleOpen(struct inode *inode, struct file *filp);
int DrvSclModuleRelease(struct inode *inode, struct file *filp);
long DrvSclModuleIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
static int DrvSclModuleProbe(struct platform_device *pdev);
static int DrvSclModuleRemove(struct platform_device *pdev);
static int DrvSclModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvSclModuleResume(struct platform_device *dev);
static unsigned int DrvSclModulePoll(struct file *filp, struct poll_table_struct *wait);
extern int DrvSclIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
extern int DrvSclM2MIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
extern long DrvSclVipIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
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
    struct device *devicenode;
}DrvSclModuleDevice_t;

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern u8  gbdbgmessage[EN_DBGMG_NUM_CONFIG];

static DrvSclModuleDevice_t _tSclDevice =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCL_DEVICE_MINOR,
    .refCnt = 0,
    .devicenode = NULL,
    .cdev =
    {
        .kobj = {.name= DRV_SCL_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclVipDevice =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLVIP_DEVICE_MINOR,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLVIP_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclM2MDevice =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLM2M_DEVICE_MINOR,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLM2M_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};

static DrvSclModuleDevice_t _tSclHvsp4Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLHVSP_DEVICE_MINOR4,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLHVSP_DEVICE_NAME4, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclHvsp3Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLHVSP_DEVICE_MINOR3,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLHVSP_DEVICE_NAME3, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclHvsp2Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLHVSP_DEVICE_MINOR2,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLHVSP_DEVICE_NAME2, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclHvsp1Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLHVSP_DEVICE_MINOR1,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLHVSP_DEVICE_NAME1, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclDma1Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLDMA_DEVICE_MINOR1,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLDMA_DEVICE_NAME1, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclDma2Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLDMA_DEVICE_MINOR2,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLDMA_DEVICE_NAME2, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclDma3Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLDMA_DEVICE_MINOR3,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLDMA_DEVICE_NAME3, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};
static DrvSclModuleDevice_t _tSclDma4Device =
{
    .s32Major = DRV_SCL_DEVICE_MAJOR,
    .s32Minor = DRV_SCLDMA_DEVICE_MINOR4,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name= DRV_SCLDMA_DEVICE_NAME4, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = DrvSclModuleOpen,
        .release = DrvSclModuleRelease,
        .unlocked_ioctl = DrvSclModuleIoctl,
        .poll = DrvSclModulePoll,
    },
};



static struct class * _tSclHvspClass = NULL;
static char * SclHvspClassName = "m_sclhvsp_1_class";


static const struct of_device_id _SclMatchTable[] =
{
    { .compatible = "mstar,sclhvsp1_i2" },
    {}
};

static struct platform_driver stDrvSclPlatformDriver =
{
    .probe      = DrvSclModuleProbe,
    .remove     = DrvSclModuleRemove,
    .suspend    = DrvSclModuleSuspend,
    .resume     = DrvSclModuleResume,
    .driver =
    {
        .name   = DRV_SCL_DEVICE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_SclMatchTable),
    },
};

static u64 u64SclHvsp_DmaMask = 0xffffffffUL;

static struct platform_device stDrvSclPlatformDevice =
{
    .name = DRV_SCL_DEVICE_NAME,
    .id = 0,
    .dev =
    {
        .of_node = NULL,
        .dma_mask = &u64SclHvsp_DmaMask,
        .coherent_dma_mask = 0xffffffffUL
    }
};
//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------
u8 _mdrv_Scl_Changebuf2hex(int u32num)
{
    u8 u8level;
    if(u32num==10)
    {
        u8level = 1;
    }
    else if(u32num==48)
    {
        u8level = 0;
    }
    else if(u32num==49)
    {
        u8level = 0x1;
    }
    else if(u32num==50)
    {
        u8level = 0x2;
    }
    else if(u32num==51)
    {
        u8level = 0x3;
    }
    else if(u32num==52)
    {
        u8level = 0x4;
    }
    else if(u32num==53)
    {
        u8level = 0x5;
    }
    else if(u32num==54)
    {
        u8level = 0x6;
    }
    else if(u32num==55)
    {
        u8level = 0x7;
    }
    else if(u32num==56)
    {
        u8level = 0x8;
    }
    else if(u32num==57)
    {
        u8level = 0x9;
    }
    else if(u32num==65)
    {
        u8level = 0xa;
    }
    else if(u32num==66)
    {
        u8level = 0xb;
    }
    else if(u32num==67)
    {
        u8level = 0xc;
    }
    else if(u32num==68)
    {
        u8level = 0xd;
    }
    else if(u32num==69)
    {
        u8level = 0xe;
    }
    else if(u32num==70)
    {
        u8level = 0xf;
    }
    else if(u32num==97)
    {
        u8level = 0xa;
    }
    else if(u32num==98)
    {
        u8level = 0xb;
    }
    else if(u32num==99)
    {
        u8level = 0xc;
    }
    else if(u32num==100)
    {
        u8level = 0xd;
    }
    else if(u32num==101)
    {
        u8level = 0xe;
    }
    else if(u32num==102)
    {
        u8level = 0xf;
    }
    return u8level;
}
void _mdrv_Scl_SetInputTestPatternAndTgen(u16 Width,u16 Height)
{
    MDrvSclHvspMiscConfig_t stHvspMiscCfg;
#if defined(SCLOS_TYPE_LINUX_TEST)
    u8 u8InputTgenSetBuf[200] =
    {
        0x25, 0x15, 0x80, 0x03, 0xFF,// 4
        0x25, 0x15, 0x81, 0x80, 0xFF,// 9
        0x25, 0x15, 0x82, 0x30, 0xFF,
        0x25, 0x15, 0x83, 0x30, 0xFF,
        0x25, 0x15, 0x84, 0x10, 0xFF,
        0x25, 0x15, 0x85, 0x10, 0xFF,
        0x25, 0x15, 0x86, 0x02, 0xFF,
        0x25, 0x15, 0x87, 0x00, 0xFF,
        0x25, 0x15, 0x88, 0x21, 0xFF,
        0x25, 0x15, 0x89, 0x0C, 0xFF,
        0x25, 0x15, 0xE0, 0x01, 0xFF,
        0x25, 0x15, 0xE1, 0x00, 0xFF,
        0x25, 0x15, 0xE2, 0x01, 0xFF,
        0x25, 0x15, 0xE3, 0x00, 0xFF,
        0x25, 0x15, 0xE4, 0x03, 0xFF,
        0x25, 0x15, 0xE5, 0x00, 0xFF,
        0x25, 0x15, 0xE6, 0x05, 0xFF,
        0x25, 0x15, 0xE7, 0x00, 0xFF,
        0x25, 0x15, 0xE8, 0x3C, 0xFF,//94
        0x25, 0x15, 0xE9, 0x04, 0xFF,//99
        0x25, 0x15, 0xEA, 0x05, 0xFF,
        0x25, 0x15, 0xEB, 0x00, 0xFF,
        0x25, 0x15, 0xEC, 0x3C, 0xFF,//114
        0x25, 0x15, 0xED, 0x04, 0xFF,//119
        0x25, 0x15, 0xEE, 0xFF, 0xFF,//124
        0x25, 0x15, 0xEF, 0x08, 0xFF,//129
        0x25, 0x15, 0xF2, 0x04, 0xFF,
        0x25, 0x15, 0xF3, 0x00, 0xFF,
        0x25, 0x15, 0xF4, 0x7F, 0xFF,
        0x25, 0x15, 0xF5, 0x00, 0xFF,
        0x25, 0x15, 0xF6, 0xA8, 0xFF,
        0x25, 0x15, 0xF7, 0x00, 0xFF,
        0x25, 0x15, 0xF8, 0x27, 0xFF,//164
        0x25, 0x15, 0xF9, 0x08, 0xFF,//169
        0x25, 0x15, 0xFA, 0xA8, 0xFF,
        0x25, 0x15, 0xFB, 0x00, 0xFF,
        0x25, 0x15, 0xFC, 0x27, 0xFF,//184
        0x25, 0x15, 0xFD, 0x08, 0xFF,//189
        0x25, 0x15, 0xFE, 0xFF, 0xFF,//194
        0x25, 0x15, 0xFF, 0x0B, 0xFF,//199
    };
#else
    unsigned char u8InputTgenSetBuf[300] =
    {
        0x18, 0x12, 0x80, 0x03, 0xFF,// 4
        0x18, 0x12, 0x81, 0x80, 0xFF,// 9
        0x18, 0x12, 0x82, 0x30, 0xFF,
        0x18, 0x12, 0x83, 0x30, 0xFF,
        0x18, 0x12, 0x84, 0x10, 0xFF,
        0x18, 0x12, 0x85, 0x10, 0xFF,
        0x18, 0x12, 0x86, 0x02, 0xFF,
        0x18, 0x12, 0x87, 0x00, 0xFF,
        0x18, 0x12, 0x88, 0x21, 0xFF,
        0x18, 0x12, 0x89, 0x0C, 0xFF,
        0x18, 0x12, 0xE0, 0x01, 0xFF,
        0x18, 0x12, 0xE1, 0x00, 0xFF,
        0x18, 0x12, 0xE2, 0x01, 0xFF,
        0x18, 0x12, 0xE3, 0x00, 0xFF,
        0x18, 0x12, 0xE4, 0x03, 0xFF,
        0x18, 0x12, 0xE5, 0x00, 0xFF,
        0x18, 0x12, 0xE6, 0x05, 0xFF,
        0x18, 0x12, 0xE7, 0x00, 0xFF,
        0x18, 0x12, 0xE8, 0x3C, 0xFF,//94
        0x18, 0x12, 0xE9, 0x04, 0xFF,//99
        0x18, 0x12, 0xEA, 0x05, 0xFF,
        0x18, 0x12, 0xEB, 0x00, 0xFF,
        0x18, 0x12, 0xEC, 0x3C, 0xFF,//114
        0x18, 0x12, 0xED, 0x04, 0xFF,//119
        0x18, 0x12, 0xEE, 0xFF, 0xFF,//124
        0x18, 0x12, 0xEF, 0x08, 0xFF,//129
        0x18, 0x12, 0xF2, 0x04, 0xFF,
        0x18, 0x12, 0xF3, 0x00, 0xFF,
        0x18, 0x12, 0xF4, 0x7F, 0xFF,
        0x18, 0x12, 0xF5, 0x00, 0xFF,
        0x18, 0x12, 0xF6, 0xA8, 0xFF,
        0x18, 0x12, 0xF7, 0x00, 0xFF,
        0x18, 0x12, 0xF8, 0x27, 0xFF,//164
        0x18, 0x12, 0xF9, 0x08, 0xFF,//169
        0x18, 0x12, 0xFA, 0xA8, 0xFF,
        0x18, 0x12, 0xFB, 0x00, 0xFF,
        0x18, 0x12, 0xFC, 0x27, 0xFF,//184
        0x18, 0x12, 0xFD, 0x08, 0xFF,//189
        0x18, 0x12, 0xFE, 0xFF, 0xFF,//194
        0x18, 0x12, 0xFF, 0x0B, 0xFF,//199
        0x21, 0x12, 0xE0, 0x01, 0x01,//vip
        0x1E, 0x12, 0x70, 0x00, 0xFF,
        0x1E, 0x12, 0x71, 0x04, 0x07,
        0x1E, 0x12, 0x72, 0x00, 0xFF,
        0x1E, 0x12, 0x73, 0x00, 0x01,
        0x1E, 0x12, 0x74, 0x00, 0xFF,
        0x1E, 0x12, 0x75, 0x04, 0x07,
        0x1E, 0x12, 0x76, 0x00, 0xFF,
        0x1E, 0x12, 0x77, 0x00, 0x01,
        0x1E, 0x12, 0x78, 0x00, 0xFF,
        0x1E, 0x12, 0x79, 0x04, 0x07,
        0x1E, 0x12, 0x7A, 0x00, 0xFF,
        0x1E, 0x12, 0x7B, 0x00, 0x01,
        0x1E, 0x12, 0x7C, 0x00, 0xFF,
        0x1E, 0x12, 0x7D, 0x04, 0x07,
        0x1E, 0x12, 0x7E, 0x00, 0xFF,
        0x1E, 0x12, 0x7F, 0x00, 0x01,
    };
#endif
    // Input tgen setting
    u8InputTgenSetBuf[93]  = (u8)((0x4+Height)&0x00FF);
    u8InputTgenSetBuf[98]  = (u8)(((0x4+Height)&0xFF00)>>8);
    u8InputTgenSetBuf[113] = (u8)((0x4+Height)&0x00FF);
    u8InputTgenSetBuf[118] = (u8)(((0x4+Height)&0xFF00)>>8);
    u8InputTgenSetBuf[123] = 0xFF;
    u8InputTgenSetBuf[128] = 0x08;
    u8InputTgenSetBuf[163] = (u8)((0xA7+Width)&0x00FF);
    u8InputTgenSetBuf[168] = (u8)(((0xA7+Width)&0xFF00)>>8);
    u8InputTgenSetBuf[183] = (u8)((0xA7+Width)&0x00FF);
    u8InputTgenSetBuf[188] = (u8)(((0xA7+Width)&0xFF00)>>8);
    u8InputTgenSetBuf[193] = 0xFF;
    u8InputTgenSetBuf[198] = 0x0B;

    stHvspMiscCfg.u8Cmd     = 0;
    stHvspMiscCfg.u32Size   = sizeof(u8InputTgenSetBuf);
    stHvspMiscCfg.u32Addr   = (u32)u8InputTgenSetBuf;
    MDrvSclHvspSetMiscConfigForKernel(&stHvspMiscCfg);
}

void _mdrv_Scl_OpenInputTestPatternAndTgen(bool ball)
{
    MDrvSclHvspSetPatTgenStatus(ball);
}

void _mdrv_Scl_OpenTestPatternByISPTgen(bool bDynamic, bool ball)
{
    MDrvSclHvspMiscConfig_t stHvspMiscCfg;
    u8 input_tgen_buf[] =
    {
        0x25, 0x15, 0x80, 0x03, 0xFF,// 4
        0x25, 0x15, 0x81, 0x80, 0xFF,// 9
        0x25, 0x15, 0x82, 0x30, 0xFF,
        0x25, 0x15, 0x83, 0x30, 0xFF,
        0x25, 0x15, 0x84, 0x10, 0xFF,
        0x25, 0x15, 0x85, 0x10, 0xFF,
        0x25, 0x15, 0x86, 0x02, 0xFF,
        0x25, 0x15, 0x87, 0x00, 0xFF,
        0x25, 0x15, 0x88, 0x21, 0xFF,
        0x25, 0x15, 0x89, 0x0C, 0xFF,
    };
    if(bDynamic)
    {
        input_tgen_buf[3]   = (u8)0x7;
    }
    stHvspMiscCfg.u8Cmd     = ball ? E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINSTALL : E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINST;
    stHvspMiscCfg.u32Size   = sizeof(input_tgen_buf);
    stHvspMiscCfg.u32Addr   = (u32)input_tgen_buf;
    MDrvSclHvspSetMiscConfigForKernel(&stHvspMiscCfg);

}
void _mdrv_Scl_CloseTestPatternByISPTgen(void)
{
    MDrvSclHvspMiscConfig_t stHvspMiscCfg;
    u8 input_tgen_buf[] =
    {
        0x25, 0x15, 0x80, 0x00, 0xFF,// 4
        0x25, 0x15, 0x81, 0x00, 0xFF,// 9
        0x25, 0x15, 0x86, 0x02, 0xFF,
        0x25, 0x15, 0x88, 0x20, 0xFF,
        0x25, 0x15, 0x89, 0x0C, 0xFF,
        0x25, 0x15, 0xE0, 0x00, 0xFF,
        0x25, 0x15, 0xE1, 0x00, 0xFF,
    };
    stHvspMiscCfg.u8Cmd     = E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINSTALL;
    stHvspMiscCfg.u32Size   = sizeof(input_tgen_buf);
    stHvspMiscCfg.u32Addr   = (u32)input_tgen_buf;
    MDrvSclHvspSetMiscConfigForKernel(&stHvspMiscCfg);

}


static ssize_t ptgen_call_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        if((int)*str == 49)    //input 1  echo 1 >ptgen_call
        {
            SCL_ERR( "[HVSP1]ptgen all static OK %d\n",(int)*str);
            _mdrv_Scl_OpenTestPatternByISPTgen(E_MDRV_SCLHVSP_CALLPATGEN_STATIC,1);
        }
        else if((int)*str == 48)  //input 0  echo 0 >ptgen_call
        {
            SCL_ERR( "[HVSP1]ptgen_call_close %d\n",(int)*str);
            _mdrv_Scl_CloseTestPatternByISPTgen();
        }
        else if((int)*str == 50)  //input 2
        {
            SCL_ERR( "[HVSP1]ptgen all dynamic %d\n",(int)*str);
            _mdrv_Scl_OpenTestPatternByISPTgen(E_MDRV_SCLHVSP_CALLPATGEN_DYNAMIC,1);
        }
        else if((int)*str == 51)  //input 3
        {
            SCL_ERR( "[HVSP1]ptgen inst static %d\n",(int)*str);
            _mdrv_Scl_OpenTestPatternByISPTgen(E_MDRV_SCLHVSP_CALLPATGEN_STATIC,0);
        }
        else if((int)*str == 52)  //input 4
        {
            SCL_ERR( "[HVSP1]ptgen inst dynamic %d\n",(int)*str);
            _mdrv_Scl_OpenTestPatternByISPTgen(E_MDRV_SCLHVSP_CALLPATGEN_DYNAMIC,0);
        }
        else if((int)*str == 53)  //input 5
        {
            SCL_ERR( "[HVSP1]ptgen all dynamic by sclself%d\n",(int)*str);
            _mdrv_Scl_OpenInputTestPatternAndTgen(1);
        }
        else if((int)*str == 54)  //input 6
        {
            SCL_ERR( "[HVSP1]ptgen inst dynamic by sclself%d\n",(int)*str);
            _mdrv_Scl_OpenInputTestPatternAndTgen(0);
        }
        return n;
    }

    return 0;
}
static ssize_t ptgen_call_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf,"0:close\n1:open all static ptgen\n2:open all dynamic ptgen\n3:open inst static ptgen\n4:open inst dynamic ptgen\n5:open all dynamic self\n6:open inst dynamic self\n");
}

static DEVICE_ATTR(ptgen,0644, ptgen_call_show, ptgen_call_store);
#if defined(SCLOS_TYPE_LINUX_DEBUG)

MDrvSclHvspIdType_e _DrvSclModuleGetDev2Type(struct device *dev)
{
    MDrvSclHvspIdType_e enType = E_MDRV_SCLHVSP_ID_1;
    if(((u32)dev) == ((u32)_tSclHvsp1Device.devicenode))
    {
        enType = E_MDRV_SCLHVSP_ID_1;
    }
    else if(((u32)dev) == ((u32)_tSclHvsp2Device.devicenode))
    {
        enType = E_MDRV_SCLHVSP_ID_2;
    }
    else if(((u32)dev) == ((u32)_tSclHvsp3Device.devicenode))
    {
        enType = E_MDRV_SCLHVSP_ID_2;
    }
    else if(((u32)dev) == ((u32)_tSclHvsp4Device.devicenode))
    {
        enType = E_MDRV_SCLHVSP_ID_2;
    }
    else
    {
        enType = E_MDRV_SCLHVSP_ID_1;
    }
    return enType;
}
ssize_t MDrvSclVipVipShow(char *buf);
static ssize_t check_IQ_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return MDrvSclVipVipShow(buf);
}

static DEVICE_ATTR(IQ,0444, check_IQ_show,NULL);

static ssize_t check_SCIQ_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    MDrvSclHvspIdType_e enType = _DrvSclModuleGetDev2Type(dev);
    MDrvSclHvspScIqStore(buf,enType);
    return n;
}
static ssize_t check_SCIQ_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    MDrvSclHvspIdType_e enType = _DrvSclModuleGetDev2Type(dev);
    return MDrvSclHvspScIqShow(buf,enType);
}

static DEVICE_ATTR(SCIQ,0644, check_SCIQ_show, check_SCIQ_store);
#endif
static ssize_t check_proc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return MDrvSclHvspProcShow(buf);
}
static ssize_t check_proc_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        u8 u8level;
        if(((int)*(str+1))>=48)//LF :line feed
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
            u8level |= (_mdrv_Scl_Changebuf2hex((int)*(str))<<4);
        }
        else
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str));
        }
        MDrvSclHvspSetProcInst(u8level);
        SCL_ERR( "[HVSP1]Set Inst %d\n",(int)u8level);
    }
    return n;
}
static DEVICE_ATTR(proc,0644, check_proc_show, check_proc_store);
void _UTest_FdRewind(struct file *fp)
{
    SCL_DBGERR("[%s]open fp:%lx %lx\n",__FUNCTION__,(u32)fp->f_op->llseek,(u32)fp->f_op);
    fp->f_op->llseek(fp,0,SEEK_SET);
}
struct file *_UTest_OpenFile(char *path,int flag,int mode)
{
    struct file *fp=NULL;

    fp = filp_open(path, flag, mode);
    SCL_ERR("[%s]open fp:%lx\n",__FUNCTION__,(u32)fp);
    if ((s32)fp != -1)
    {
        _UTest_FdRewind(fp);
        return fp;
    }
    else
    {
        SCL_ERR("[%s]open fail\n",__FUNCTION__);
        return NULL;
    }
}
int _UTest_WriteFile(struct file *fp,char *buf,int writelen)
{
    if (fp->f_op && fp->f_op->read)
    {
        return fp->f_op->write(fp,buf,writelen, &fp->f_pos);
    }
    else
    {
        return -1;
    }
}

int _UTest_CloseFile(struct file *fp)
{
    filp_close(fp,NULL);
    return 0;
}
#if defined(SCLOS_TYPE_LINUX_TEST)
ssize_t check_test_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t check_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n);
static DEVICE_ATTR(test,0644, check_test_show, check_test_store);
ssize_t check_testm2m_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t check_testm2m_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n);
static DEVICE_ATTR(m2mtest,0644, check_testm2m_show, check_testm2m_store);
void UTest_init(void);
#if defined(USE_USBCAM)
ssize_t check_iqtest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n);
ssize_t check_iqtest_show(struct device *dev, struct device_attribute *attr, char *buf);
static DEVICE_ATTR(iqtest,0644, check_iqtest_show, check_iqtest_store);

#endif
#endif
void DrvSclIrqResetSclHistInts(void);
void DrvSclIrqPrintSclHistInts(u8 u8Flag);
static ssize_t check_ints_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        u8 u8level = 0;
        if(((int)*(str+1))>=48)//LF :line feed
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
            u8level |= (_mdrv_Scl_Changebuf2hex((int)*(str))<<4);
        }
        else
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str));
        }
        if((int)*str == 48)    //input 1  echo 0 >
        {
            DrvSclIrqResetSclHistInts();
        }
        if(u8level>0)    //input 1  echo 1 >
        {
            DrvSclIrqPrintSclHistInts(u8level);
        }
        return n;
    }
    return 0;
}
static ssize_t check_ints_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return MDrvSclHvspIntsShow(buf);
}
static DEVICE_ATTR(ints,0644, check_ints_show, check_ints_store);


static ssize_t check_dbgmg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return MDrvSclHvspDbgmgFlagShow(buf);
}

static ssize_t check_dbgmg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        u8 u8level;
        SCL_ERR( "[HVSP1]check_dbgmg_store OK %d\n",(int)*str);
        SCL_ERR( "[HVSP1]check_dbgmg_store level %d\n",(int)*(str+1));
        SCL_ERR( "[HVSP1]check_dbgmg_store level2 %d\n",(int)*(str+2));
        if(((int)*(str+2))>=48)//LF :line feed
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str+2));
            u8level |= (_mdrv_Scl_Changebuf2hex((int)*(str+1))<<4);
        }
        else
        {
            u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
        }

        if((int)*str == 48)    //input 1  echo 0 >
        {
            Reset_DBGMG_FLAG();
            MHalVpeIqDbgLevel(&gbdbgmessage[EN_DBGMG_VPEIQ_CONFIG]);
            MHalVpeSclDbgLevel(&gbdbgmessage[EN_DBGMG_VPESCL_CONFIG]);
            MHalVpeIspDbgLevel(&gbdbgmessage[EN_DBGMG_VPEISP_CONFIG]);
        }
        else if((int)*str == 49)    //input 1  echo 1 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_MDRV_CONFIG,u8level);
        }
        else if((int)*str == 50)    //input 1  echo 2 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_IOCTL_CONFIG,u8level);
        }
        else if((int)*str == 51)    //input 1  echo 3 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_HVSP_CONFIG,u8level);
        }
        else if((int)*str == 52)    //input 1  echo 4 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_SCLDMA_CONFIG,u8level);
        }
        else if((int)*str == 53)    //input 1  echo 5 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_PNL_CONFIG,u8level);
        }
        else if((int)*str == 54)    //input 1  echo 6 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_VIP_CONFIG,u8level);
        }
        else if((int)*str == 55)    //input 1  echo 7 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_DRVPQ_CONFIG,u8level);
        }
        else if((int)*str == 56)    //input 1  echo 8 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_CTX_CONFIG,u8level);
        }
        else if((int)*str == 57)    //input 1  echo 9 >
        {
            Set_DBGMG_FLAG(EN_DBGMG_VPESCL_CONFIG,u8level);
            MHalVpeSclDbgLevel(&gbdbgmessage[EN_DBGMG_VPESCL_CONFIG]);
        }
        else if((int)*str == 65)    //input 1  echo A >
        {
            Set_DBGMG_FLAG(EN_DBGMG_VPEIQ_CONFIG,u8level);
            MHalVpeIqDbgLevel(&gbdbgmessage[EN_DBGMG_VPEIQ_CONFIG]);
        }
        else if((int)*str == 66)    //input 1  echo B >
        {
            Set_DBGMG_FLAG(EN_DBGMG_DRVHVSP_CONFIG,u8level);
        }
        else if((int)*str == 67)    //input 1  echo C >
        {
            Set_DBGMG_FLAG(EN_DBGMG_DRVSCLDMA_CONFIG,u8level);
        }
        else if((int)*str == 68)    //input 1  echo D >
        {
            Set_DBGMG_FLAG(EN_DBGMG_DRVSCLIRQ_CONFIG,u8level);
        }
        else if((int)*str == 69)    //input 1  echo E >
        {
            Set_DBGMG_FLAG(EN_DBGMG_VPEISP_CONFIG,u8level);
            MHalVpeIspDbgLevel(&gbdbgmessage[EN_DBGMG_VPEISP_CONFIG]);
        }
        else if((int)*str == 70)    //input 1  echo F >
        {
            Set_DBGMG_FLAG(EN_DBGMG_DRVVIP_CONFIG,u8level);
        }
        else if((int)*str == 71)    //input 1  echo G >
        {
            Set_DBGMG_FLAG(EN_DBGMG_PRIORITY_CONFIG,1);
        }
        else if((int)*str == 72)    //input 1  echo H >
        {
            Set_DBGMG_FLAG(EN_DBGMG_UTILITY_CONFIG,u8level);
        }
        else if((int)*str == 73) // input 1 echo I >
        {
            MDrvSclHvspDbgmgDumpShow(u8level);
        }
        return n;
    }

    return 0;
}

static DEVICE_ATTR(dbgmg,0644, check_dbgmg_show, check_dbgmg_store);
#if defined(SCLOS_TYPE_LINUX_DEBUG)
static ssize_t check_lock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return MDrvSclHvspLockShow(buf);
}
static ssize_t check_lock_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        if((int)*str == 76)    //input 1  echo L >
        {
            if(!DrvSclOsReleaseMutexAll())
            {
                SCL_DBGERR("[HVSP]!!!!!!!!!!!!!!!!!!! HVSP Release Mutex fail\n");
            }
        }
        return n;
    }
    return 0;
}

static DEVICE_ATTR(mutex,0644, check_lock_show, check_lock_store);
u32 gu32SaveAddr = 0;
void *gpSaveVirAddr = NULL;
u32 gu32SaveSize = 0;
bool gbioremap = 0;
bool gSavepath[20] = "/tmp/Scout.bin";
static ssize_t savebin_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct file *wfp = NULL;
    SCL_ERR("[SCL]Size :%lx ,VirAddr:%lx ,Addr:%lx path:%s\n",gu32SaveSize,(u32)gpSaveVirAddr,(u32)gu32SaveAddr,gSavepath);
    wfp = _UTest_OpenFile(gSavepath,O_WRONLY|O_CREAT,0777);
    if(wfp && gpSaveVirAddr && gu32SaveSize)
    {
        _UTest_WriteFile(wfp,(char *)gpSaveVirAddr,gu32SaveSize);
        _UTest_CloseFile(wfp);
    }
    return 0;
}
static ssize_t savebin_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        int idx = 0;
        u32 u32Level;
        u8 u8off = 7;
        if((int)*str == 'a')
        {
            str++;
            idx++;
            u8off = 7;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu32SaveAddr |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]gu32SaveAddr OK %lx\n",gu32SaveAddr);
            str++;
        }
        if((int)*str == 's')
        {
            str++;
            idx = 0;
            u8off = 0;
            while(1)
            {
                if((((int)*(str+idx))<48) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            u8off--;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu32SaveSize |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]Size OK %lx\n",gu32SaveSize);
            str++;
        }
        if((int)*str == 'r')
        {
            str++;
            gpSaveVirAddr = DrvSclOsDirectMemMap((gu32SaveAddr),gu32SaveSize);
            /*
            if(gu32SaveAddr < 0x2F800000+0x1000000)
            {
                gpSaveVirAddr = (void*)(gu32SaveAddr+(0xC0000000-0x1000000));
            }
            else
            {
                gpSaveVirAddr = ioremap((gu32SaveAddr+0x20000000),gu32SaveSize);
                gbioremap = 1;
            }*/
            SCL_ERR("[SCL]Save bin VirAddr: %lx\n",(u32)gpSaveVirAddr);
            str++;
        }
        if((int)*str == 'p')
        {
            str++;
            u8off = 0;
            idx = 0;
            while(1)
            {
                if((((int)*(str+idx))<33) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            for(idx=0;idx<u8off;idx++)
            {
                gSavepath[idx] = *(str+idx);
            }
            SCL_ERR("[SCL]idx OK %s\n",gSavepath);
        }
        if((int)*str == 'u')
        {
            DrvSclOsDirectMemUnmap((u32)gpSaveVirAddr);
            //gpSaveVirAddr = ioremap((gu32SaveAddr+0x20000000),gu32SaveSize);
            /*
            if(gbioremap)
            {
                iounmap(gpSaveVirAddr);
                gbioremap = 0;
                gpSaveVirAddr = NULL;
                gu32SaveAddr = 0;
                gu32SaveSize = 0;
            }
            else
            {
                gpSaveVirAddr = NULL;
                gu32SaveAddr = 0;
                gu32SaveSize = 0;
            }*/
        }
        return n;
    }
    return 0;
}

static DEVICE_ATTR(savebin,0644, savebin_show, savebin_store);
#endif
u32 gu32RegAddr = 0x151e;
u16 gu16RegVal = 0;
u16 gu16RegMsk = 0xFFFF;
u16 gu16RegOffset = 0;
bool gbAllBank = 1;
static ssize_t Reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32 u32RegAddr = 0;
    u32RegAddr = ((gu16RegOffset<<1) | (gu32RegAddr<<8));
    if(u32RegAddr)
    {
        return MDrvSclHvspRegShow(buf,u32RegAddr,&gu16RegVal,gbAllBank);
    }
    else
    {
        return 0;
    }
}
static ssize_t Reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        const char *str = buf;
        int idx = 0;
        u32 u32Level;
        static u32 u32Inst = 0xFF;
        u8 u8off = 0;
        MDrvSclHvspMiscConfig_t stHvspMiscCfg;
        u8 input_tgen_buf[10];
        if((int)*str == 'a')//addr
        {
            str++;
            idx++;
            u8off = 3;
            gu32RegAddr = 0;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu32RegAddr |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]gu32SaveAddr OK %lx\n",gu32RegAddr);
            str++;

        }
        if((int)*str == 'o')//offset
        {
            str++;
            idx = 0;
            u8off = 0;
            gu16RegOffset = 0;
            while(1)
            {
                if((((int)*(str+idx))<48) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            u8off--;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu16RegOffset |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]RegOffset OK %hx\n",gu16RegOffset);
            str++;
        }
        if((int)*str == 'v')//value
        {
            str++;
            idx = 0;
            u8off = 0;
            gu16RegVal = 0;
            while(1)
            {
                if((((int)*(str+idx))<48) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            u8off--;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu16RegVal |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]RegVal OK %hx\n",gu16RegVal);
            str++;
        }
        if((int)*str == 'm')//mask
        {
            str++;
            idx = 0;
            u8off = 0;
            gu16RegMsk = 0;
            while(1)
            {
                if((((int)*(str+idx))<48) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            u8off--;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    gu16RegMsk |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]RegMsk OK %hx\n",gu16RegMsk);
            str++;
        }
        if((int)*str == 'b')//b all = 1
        {
            str++;
            gbAllBank = 1;
            str++;
        }
        if((int)*str == 's')//b all = 0
        {
            str++;
            gbAllBank = 0;
            str++;
        }
        if((int)*str == 'i')//write reg
        {
            str++;
            idx = 0;
            u8off = 0;
            u32Inst = 0;
            while(1)
            {
                if((((int)*(str+idx))<48) || idx >100  )//LF :line feed
                {
                    break;
                }
                else
                {
                    u8off++;
                    idx++;
                }
            }
            u8off--;
            while(1)
            {
                if((((int)*(str))<48) || idx >100  )//LF :line feed
                {
                    str++;
                    break;
                }
                else
                {
                    u32Level = (u32)_mdrv_Scl_Changebuf2hex((int)*(str));
                    u32Inst |= (u32Level<< (u8off*4));
                    if(u8off)
                    {
                        u8off--;
                        str++;
                        idx++;
                    }
                    else
                    {
                        str++;
                        idx++;
                        break;
                    }
                }
            }
            SCL_ERR("[SCL]u32Inst OK %lx\n",u32Inst);
            MDrvSclHvspSetProcInst(u32Inst);
            str++;
        }
        if((int)*str == 'w')//write reg
        {
            str++;
            stHvspMiscCfg.u8Cmd     = (u32Inst==0xFF) ? E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINSTALL : E_MDRV_SCLHVSP_MISC_CMD_SET_REG_BYINST;
            stHvspMiscCfg.u32Size   = sizeof(input_tgen_buf);
            stHvspMiscCfg.u32Addr   = (u32)input_tgen_buf;
            input_tgen_buf[0] = gu32RegAddr&0xFF;
            input_tgen_buf[1] = (gu32RegAddr>>8)&0xFF;
            input_tgen_buf[2] = (gu16RegOffset&0xFF)*2;
            input_tgen_buf[3] = gu16RegVal&0xFF;
            input_tgen_buf[4] = gu16RegMsk&0xFF;
            input_tgen_buf[5] = gu32RegAddr&0xFF;
            input_tgen_buf[6] = (gu32RegAddr>>8)&0xFF;
            input_tgen_buf[7] = ((gu16RegOffset&0xFF)*2)+1;
            input_tgen_buf[8] = (gu16RegVal>>8)&0xFF;
            input_tgen_buf[9] = (gu16RegMsk>>8)&0xFF;
            MDrvSclHvspSetMiscConfigForKernel(&stHvspMiscCfg);
            SCL_ERR("[SCL]Write inst:%lx OK\n",u32Inst);
            str++;
        }
        return n;
    }
    return 0;
}

static DEVICE_ATTR(reg,0644, Reg_show, Reg_store);

//==============================================================================

long DrvSclModuleIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int err = 0;
    int retval = 0;
    if(filp->private_data == NULL)
    {
        SCL_ERR( "[SCL] IO_IOCTL private_data =NULL!!! \n");
        return -EFAULT;
    }
    /* check u32Cmd valid */
    if(IOCTL_SCL_MAGIC == _IOC_TYPE(u32Cmd))
    {
        if(_IOC_NR(u32Cmd) >= IOCTL_SCL_MAX_NR)
        {
            SCL_ERR( "[SCL] IOCtl NR Error!!! (Cmd=%x)\n",u32Cmd);
            return -ENOTTY;
        }
    }
    else if(IOCTL_VIP_MAGIC == _IOC_TYPE(u32Cmd))
    {
        if(_IOC_NR(u32Cmd) >= IOCTL_SCL_MAX_NR)
        {
            SCL_ERR( "[SCL] IOCtl NR Error!!! (Cmd=%x)\n",u32Cmd);
            return -ENOTTY;
        }
    }
    else
    {
        SCL_ERR( "[SCL] IOCtl MAGIC Error!!! (Cmd=%x)\n",u32Cmd);
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

    retval = DrvSclIoctlParse(filp, u32Cmd, u32Arg);
    return retval;
}


static unsigned int DrvSclModulePoll(struct file *filp, struct poll_table_struct *wait)
{
    return 0;
}


static int DrvSclModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    MDrvSclHvspSuspendResumeConfig_t stHvspSuspendResumeCfg;
    int ret = 0;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[HVSP_1] %s\n",__FUNCTION__);

    stHvspSuspendResumeCfg.u32IRQNum = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0);
    if(MDrvSclHvspSuspend(E_MDRV_SCLHVSP_ID_1, &stHvspSuspendResumeCfg))
    {
        ret = 0;
    }
    else
    {
        ret = -EFAULT;
    }

    return ret;
}

static int DrvSclModuleResume(struct platform_device *dev)
{
    MDrvSclHvspSuspendResumeConfig_t stHvspSuspendResumeCfg;
    int ret = 0;

    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[HVSP1] %s\n",__FUNCTION__);

    stHvspSuspendResumeCfg.u32IRQNum     = DrvSclOsGetIrqIDSCL(E_DRV_SCLOS_SCLIRQ_SC0);;
    MDrvSclHvspResume(E_MDRV_SCLHVSP_ID_1, &stHvspSuspendResumeCfg);

    return ret;
}
void _DrvSclModuleSetRefCnt(u32 u32DevNum, bool bAdd)
{
    switch(u32DevNum)
    {
        case DRV_SCL_DEVICE_MINOR:
            if(bAdd)
            {
                _tSclDevice.refCnt++;
            }
            else
            {
                if(_tSclDevice.refCnt)
                {
                    _tSclDevice.refCnt--;
                }
            }
        break;
        case DRV_SCLM2M_DEVICE_MINOR:
            if(bAdd)
            {
                _tSclM2MDevice.refCnt++;
            }
            else
            {
                if(_tSclM2MDevice.refCnt)
                {
                    _tSclM2MDevice.refCnt--;
                }
            }
        break;
        case DRV_SCLVIP_DEVICE_MINOR:
            if(bAdd)
            {
                _tSclVipDevice.refCnt++;
            }
            else
            {
                if(_tSclVipDevice.refCnt)
                {
                    _tSclVipDevice.refCnt--;
                }
            }
        break;
        case DRV_SCLHVSP_DEVICE_MINOR1:
            if(bAdd)
            {
                _tSclHvsp1Device.refCnt++;
            }
            else
            {
                if(_tSclHvsp1Device.refCnt)
                {
                    _tSclHvsp1Device.refCnt--;
                }
            }
        break;
        case DRV_SCLHVSP_DEVICE_MINOR2:
            if(bAdd)
            {
                _tSclHvsp2Device.refCnt++;
            }
            else
            {
                if(_tSclHvsp2Device.refCnt)
                {
                    _tSclHvsp2Device.refCnt--;
                }
            }
        break;
        case DRV_SCLHVSP_DEVICE_MINOR3:
            if(bAdd)
            {
                _tSclHvsp3Device.refCnt++;
            }
            else
            {
                if(_tSclHvsp3Device.refCnt)
                {
                    _tSclHvsp3Device.refCnt--;
                }
            }
        break;
        case DRV_SCLHVSP_DEVICE_MINOR4:
            if(bAdd)
            {
                _tSclHvsp4Device.refCnt++;
            }
            else
            {
                if(_tSclHvsp4Device.refCnt)
                {
                    _tSclHvsp4Device.refCnt--;
                }
            }
        break;
        case DRV_SCLDMA_DEVICE_MINOR1:
            if(bAdd)
            {
                _tSclDma1Device.refCnt++;
            }
            else
            {
                if(_tSclDma1Device.refCnt)
                {
                    _tSclDma1Device.refCnt--;
                }
            }
        break;
        case DRV_SCLDMA_DEVICE_MINOR2:
            if(bAdd)
            {
                _tSclDma2Device.refCnt++;
            }
            else
            {
                if(_tSclDma2Device.refCnt)
                {
                    _tSclDma2Device.refCnt--;
                }
            }
        break;
        case DRV_SCLDMA_DEVICE_MINOR3:
            if(bAdd)
            {
                _tSclDma3Device.refCnt++;
            }
            else
            {
                if(_tSclDma3Device.refCnt)
                {
                    _tSclDma3Device.refCnt--;
                }
            }
        break;
        case DRV_SCLDMA_DEVICE_MINOR4:
            if(bAdd)
            {
                _tSclDma4Device.refCnt++;
            }
            else
            {
                if(_tSclDma4Device.refCnt)
                {
                    _tSclDma4Device.refCnt--;
                }
            }
        break;
    }
}
s32 _DrvSclModuleOpenDevice(u32 u32DevNum)
{
    s32 s32Handler = -1;
    switch(u32DevNum)
    {
        case DRV_SCL_DEVICE_MINOR:
                s32Handler = SCLVPE_HANDLER_PRE_FIX;
        break;
        case DRV_SCLM2M_DEVICE_MINOR:
        break;
        case DRV_SCLVIP_DEVICE_MINOR:
			s32Handler = DrvSclVipIoOpen(E_DRV_SCLVIP_IO_ID_1);
        break;
        case DRV_SCLHVSP_DEVICE_MINOR1:
            s32Handler = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_1);
        break;
        case DRV_SCLHVSP_DEVICE_MINOR2:
            s32Handler = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_2);
        break;
        case DRV_SCLHVSP_DEVICE_MINOR3:
            s32Handler = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_3);
        break;
        case DRV_SCLHVSP_DEVICE_MINOR4:
            s32Handler = _DrvSclHvspIoOpen(E_DRV_SCLHVSP_IO_ID_4);
        break;
        case DRV_SCLDMA_DEVICE_MINOR1:
            s32Handler = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_1);
        break;
        case DRV_SCLDMA_DEVICE_MINOR2:
            s32Handler = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_2);
        break;
        case DRV_SCLDMA_DEVICE_MINOR3:
            s32Handler = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_3);
        break;
        case DRV_SCLDMA_DEVICE_MINOR4:
            s32Handler = _DrvSclDmaIoOpen(E_DRV_SCLDMA_IO_ID_4);
        break;
    }
    return s32Handler;
}
u32 _DrvSclModuleCloseDevice(u32 u32DevNum, s32 s32Handler)
{
    u32 u32Ret = 0;
    switch(u32DevNum)
    {
        case DRV_SCL_DEVICE_MINOR:

        break;
        case DRV_SCLM2M_DEVICE_MINOR:
        break;
        case DRV_SCLVIP_DEVICE_MINOR:
			u32Ret = DrvSclVipIoRelease(s32Handler);
        break;
        case DRV_SCLHVSP_DEVICE_MINOR1:
        case DRV_SCLHVSP_DEVICE_MINOR2:
        case DRV_SCLHVSP_DEVICE_MINOR3:
        case DRV_SCLHVSP_DEVICE_MINOR4:
            u32Ret = _DrvSclHvspIoRelease(s32Handler);
            break;
        case DRV_SCLDMA_DEVICE_MINOR1:
        case DRV_SCLDMA_DEVICE_MINOR2:
        case DRV_SCLDMA_DEVICE_MINOR3:
        case DRV_SCLDMA_DEVICE_MINOR4:
            u32Ret = _DrvSclDmaIoRelease(s32Handler);
            break;
    }
    return u32Ret;
}

int DrvSclModuleOpen(struct inode *inode, struct file *filp)
{
    int ret = 0;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s %lx\n",__FUNCTION__,(u32)inode->i_rdev);

    SCL_ASSERT(_tSclDevice.refCnt>=0);

    if(filp->private_data == NULL)
    {

        filp->private_data = DrvSclOsVirMemalloc(sizeof(s32));

        if(filp->private_data == NULL)
        {
            SCL_ERR("[SCLHVSP_1] %s %d, allocate memory fail\n", __FUNCTION__, __LINE__);
            ret = -EFAULT;
        }
        else
        {
            s32 s32Handler = -1;
            s32Handler = _DrvSclModuleOpenDevice((u32)(inode->i_rdev&0xFF));
            if(s32Handler != -1)
            {
                *((s32 *)filp->private_data) = s32Handler;
            }
            else
            {
                SCL_ERR("[HVSP1] %s %d, handler error fail\n", __FUNCTION__, __LINE__);
                ret = -EFAULT;
                DrvSclOsVirMemFree(filp->private_data);
            }
        }
    }
    if(!ret)
    {
        _DrvSclModuleSetRefCnt((u32)(inode->i_rdev&0xFF),1);
    }

    return ret;
}


int DrvSclModuleRelease(struct inode *inode, struct file *filp)
{
    int ret = 0;
    s32 s32Handler;

    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);

    s32Handler = *((s32 *)filp->private_data);

    if( _DrvSclModuleCloseDevice((u32)(inode->i_rdev&0xFF),s32Handler) == E_DRV_SCLHVSP_IO_ERR_OK)
    {
        DrvSclOsVirMemFree(filp->private_data);
        filp->private_data = NULL;
    }
    else
    {
        ret = -EFAULT;
        SCL_ERR("[SCLHVSP_1] Release Fail\n");
    }

    _DrvSclModuleSetRefCnt((u32)(inode->i_rdev&0xFF),0);
    SCL_ASSERT(_tSclDevice.refCnt>=0);
    return ret;
}
void _DrvSclVpeModuleDeInit(void)
{
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLDMA_1] %s\n",__FUNCTION__);
    if(_tSclDevice.cdev.count)
    {
        cdev_del(&_tSclDevice.cdev);
    }
    stDrvSclPlatformDevice.dev.of_node=NULL;
    _tSclHvspClass = NULL;
    //ToDo
    //device_unregister(_tSclDevice.devicenode);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor));
    //class_destroy(_tSclHvspClass);
    //unregister_chrdev_region(MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor), DRV_SCL_DEVICE_COUNT);
}
void _DrvSclVpeModuleInit(void)
{
    int s32Ret;
    dev_t  dev;
    if(_tSclDevice.s32Major)
    {
        dev     = MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor);
        //s32Ret  = register_chrdev_region(dev, DRV_SCL_DEVICE_COUNT, DRV_SCL_DEVICE_NAME);
        if(!_tSclHvspClass)
        {
            _tSclHvspClass = msys_get_sysfs_class();
            if(!_tSclHvspClass)
            {
                _tSclHvspClass = class_create(THIS_MODULE, SclHvspClassName);
            }

        }
        else
        {
            cdev_init(&_tSclDevice.cdev, &_tSclDevice.fops);
            if (0 != (s32Ret= cdev_add(&_tSclDevice.cdev, dev, DRV_SCL_DEVICE_COUNT)))
            {
                SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
            }
        }
        //ToDo
        if(_tSclDevice.devicenode==NULL)
        {
            _tSclDevice.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCL_DEVICE_NAME);
            //create device
            device_create_file(_tSclDevice.devicenode, &dev_attr_proc);
            device_create_file(_tSclDevice.devicenode, &dev_attr_dbgmg);
#if defined(SCLOS_TYPE_LINUX_DEBUG)
            device_create_file(_tSclDevice.devicenode, &dev_attr_mutex);
            device_create_file(_tSclDevice.devicenode, &dev_attr_savebin);
#endif
            device_create_file(_tSclDevice.devicenode, &dev_attr_reg);
            device_create_file(_tSclDevice.devicenode, &dev_attr_ptgen);
            device_create_file(_tSclDevice.devicenode, &dev_attr_ints);
#if defined(SCLOS_TYPE_LINUX_TEST)
            device_create_file(_tSclDevice.devicenode, &dev_attr_test);
            device_create_file(_tSclDevice.devicenode, &dev_attr_m2mtest);
#if defined(USE_USBCAM)
            device_create_file(_tSclDevice.devicenode, &dev_attr_iqtest);
#endif
#if defined(SCLOS_TYPE_LINUX_DEBUG)
            if(_tSclHvsp1Device.devicenode==NULL)
            {
                dev     = MKDEV(_tSclHvsp1Device.s32Major, _tSclHvsp1Device.s32Minor);
                _tSclHvsp1Device.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCLHVSP_DEVICE_NAME1);
                device_create_file(_tSclHvsp1Device.devicenode, &dev_attr_SCIQ);
            }
            if(_tSclHvsp2Device.devicenode==NULL)
            {
                dev     = MKDEV(_tSclHvsp2Device.s32Major, _tSclHvsp2Device.s32Minor);
                _tSclHvsp2Device.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCLHVSP_DEVICE_NAME2);
                device_create_file(_tSclHvsp2Device.devicenode, &dev_attr_SCIQ);
            }
            if(_tSclHvsp3Device.devicenode==NULL)
            {
                dev     = MKDEV(_tSclHvsp3Device.s32Major, _tSclHvsp3Device.s32Minor);
                _tSclHvsp3Device.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCLHVSP_DEVICE_NAME3);
                device_create_file(_tSclHvsp3Device.devicenode, &dev_attr_SCIQ);
            }
            if(_tSclHvsp4Device.devicenode==NULL)
            {
                dev     = MKDEV(_tSclHvsp4Device.s32Major, _tSclHvsp4Device.s32Minor);
                _tSclHvsp4Device.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCLHVSP_DEVICE_NAME4);
                device_create_file(_tSclHvsp4Device.devicenode, &dev_attr_SCIQ);
            }
            if(_tSclVipDevice.devicenode==NULL)
            {
                dev     = MKDEV(_tSclVipDevice.s32Major, _tSclVipDevice.s32Minor);
                _tSclVipDevice.devicenode = device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCLVIP_DEVICE_NAME);
                device_create_file(_tSclVipDevice.devicenode, &dev_attr_IQ);
            }
#endif
#endif
        }

        if(stDrvSclPlatformDevice.dev.of_node==NULL)
        {
            stDrvSclPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "mstar,sclhvsp1_i2");
        }
        if(stDrvSclPlatformDevice.dev.of_node==NULL)
        {
            SCL_ERR("[VPE INIT] Get Device mode Fail!!\n");
        }
        DrvSclOsSetSclIrqIDFormSys(&stDrvSclPlatformDevice,0,E_DRV_SCLOS_SCLIRQ_SC0);
        DrvSclOsSetSclIrqIDFormSys(&stDrvSclPlatformDevice,1,E_DRV_SCLOS_SCLIRQ_SC1);
    }
}

#if CONFIG_OF
static int DrvSclModuleProbe(struct platform_device *pdev)
{
    //unsigned char ret;
    int s32Ret;
    dev_t  dev[E_DRV_SCLOS_DEV_MAX];
    //struct resource *res_irq;
    //struct device_node *np;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s:%d\n",__FUNCTION__,__LINE__);
    if(_tSclDevice.s32Major)
    {
        dev[E_DRV_SCLOS_DEV_VIP]     = MKDEV(_tSclVipDevice.s32Major, _tSclVipDevice.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_VIP], DRV_SCL_DEVICE_COUNT, DRV_SCLVIP_DEVICE_NAME);
        dev[E_DRV_SCLOS_DEV_HVSP_1]     = MKDEV(_tSclHvsp1Device.s32Major, _tSclHvsp1Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_1], DRV_SCL_DEVICE_COUNT, DRV_SCLHVSP_DEVICE_NAME1);
        dev[E_DRV_SCLOS_DEV_HVSP_2]     = MKDEV(_tSclHvsp2Device.s32Major, _tSclHvsp2Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_2], DRV_SCL_DEVICE_COUNT, DRV_SCLHVSP_DEVICE_NAME2);
        dev[E_DRV_SCLOS_DEV_HVSP_3]     = MKDEV(_tSclHvsp3Device.s32Major, _tSclHvsp3Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_3], DRV_SCL_DEVICE_COUNT, DRV_SCLHVSP_DEVICE_NAME3);
        dev[E_DRV_SCLOS_DEV_HVSP_4]     = MKDEV(_tSclHvsp4Device.s32Major, _tSclHvsp4Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_4], DRV_SCL_DEVICE_COUNT, DRV_SCLHVSP_DEVICE_NAME4);
        dev[E_DRV_SCLOS_DEV_DMA_1]     = MKDEV(_tSclDma1Device.s32Major, _tSclDma1Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_1], DRV_SCL_DEVICE_COUNT, DRV_SCLDMA_DEVICE_NAME1);
        dev[E_DRV_SCLOS_DEV_DMA_2]     = MKDEV(_tSclDma2Device.s32Major, _tSclDma2Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_2], DRV_SCL_DEVICE_COUNT, DRV_SCLDMA_DEVICE_NAME2);
        dev[E_DRV_SCLOS_DEV_DMA_3]     = MKDEV(_tSclDma3Device.s32Major, _tSclDma3Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_3], DRV_SCL_DEVICE_COUNT, DRV_SCLDMA_DEVICE_NAME3);
        dev[E_DRV_SCLOS_DEV_DMA_4]     = MKDEV(_tSclDma4Device.s32Major, _tSclDma4Device.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_4], DRV_SCL_DEVICE_COUNT, DRV_SCLDMA_DEVICE_NAME4);
        dev[E_DRV_SCLOS_DEV_M2M]     = MKDEV(_tSclM2MDevice.s32Major, _tSclM2MDevice.s32Minor);
        //s32Ret  = register_chrdev_region(dev[E_DRV_SCLOS_DEV_M2M], DRV_SCL_DEVICE_COUNT, DRV_SCLM2M_DEVICE_NAME);
    }
    else
    {
        s32Ret                  = alloc_chrdev_region(&dev[E_DRV_SCLOS_DEV_HVSP_1], _tSclDevice.s32Minor, DRV_SCL_DEVICE_COUNT, DRV_SCL_DEVICE_NAME);
        _tSclDevice.s32Major  = MAJOR(dev[E_DRV_SCLOS_DEV_HVSP_1]);
    }
    //if (0 > s32Ret)
    //{
    //    SCL_ERR( "[SCLHVSP_1] Unable to get major %d\n", _tSclDevice.s32Major);
    //    return s32Ret;
    //}

    cdev_init(&_tSclVipDevice.cdev, &_tSclVipDevice.fops);
    cdev_init(&_tSclM2MDevice.cdev, &_tSclM2MDevice.fops);
    cdev_init(&_tSclHvsp1Device.cdev, &_tSclHvsp1Device.fops);
    cdev_init(&_tSclHvsp2Device.cdev, &_tSclHvsp2Device.fops);
    cdev_init(&_tSclHvsp3Device.cdev, &_tSclHvsp3Device.fops);
    cdev_init(&_tSclHvsp4Device.cdev, &_tSclHvsp4Device.fops);
    cdev_init(&_tSclDma1Device.cdev, &_tSclDma1Device.fops);
    cdev_init(&_tSclDma2Device.cdev, &_tSclDma2Device.fops);
    cdev_init(&_tSclDma3Device.cdev, &_tSclDma3Device.fops);
    cdev_init(&_tSclDma4Device.cdev, &_tSclDma4Device.fops);
    if (0 != (s32Ret= cdev_add(&_tSclVipDevice.cdev, dev[E_DRV_SCLOS_DEV_VIP], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_VIP], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclM2MDevice.cdev, dev[E_DRV_SCLOS_DEV_M2M], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_M2M], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclHvsp1Device.cdev, dev[E_DRV_SCLOS_DEV_HVSP_1], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_1], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclHvsp2Device.cdev, dev[E_DRV_SCLOS_DEV_HVSP_2], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_2], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclHvsp3Device.cdev, dev[E_DRV_SCLOS_DEV_HVSP_3], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_3], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclHvsp4Device.cdev, dev[E_DRV_SCLOS_DEV_HVSP_4], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_HVSP_4], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclDma1Device.cdev, dev[E_DRV_SCLOS_DEV_DMA_1], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_1], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclDma2Device.cdev, dev[E_DRV_SCLOS_DEV_DMA_2], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_2], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclDma3Device.cdev, dev[E_DRV_SCLOS_DEV_DMA_3], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_3], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    if (0 != (s32Ret= cdev_add(&_tSclDma4Device.cdev, dev[E_DRV_SCLOS_DEV_DMA_4], DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev[E_DRV_SCLOS_DEV_DMA_4], DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }
    _tSclHvspClass = msys_get_sysfs_class();
    if(!_tSclHvspClass)
    {
        _tSclHvspClass = class_create(THIS_MODULE, SclHvspClassName);
    }
    if(IS_ERR(_tSclHvspClass))
    {
        printk(KERN_WARNING"Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        _tSclVipDevice.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_VIP],NULL, DRV_SCLVIP_DEVICE_NAME);
        _tSclM2MDevice.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_M2M],NULL, DRV_SCLM2M_DEVICE_NAME);
        _tSclHvsp1Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_HVSP_1],NULL, DRV_SCLHVSP_DEVICE_NAME1);
        _tSclHvsp2Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_HVSP_2],NULL, DRV_SCLHVSP_DEVICE_NAME2);
        _tSclHvsp3Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_HVSP_3],NULL, DRV_SCLHVSP_DEVICE_NAME3);
        _tSclHvsp4Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_HVSP_4],NULL, DRV_SCLHVSP_DEVICE_NAME4);
        _tSclDma1Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_DMA_1],NULL, DRV_SCLDMA_DEVICE_NAME1);
        _tSclDma2Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_DMA_2],NULL, DRV_SCLDMA_DEVICE_NAME2);
        _tSclDma3Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_DMA_3],NULL, DRV_SCLDMA_DEVICE_NAME3);
        _tSclDma4Device.devicenode = device_create(_tSclHvspClass, NULL, dev[E_DRV_SCLOS_DEV_DMA_4],NULL, DRV_SCLDMA_DEVICE_NAME4);

        //_tSclDevice.devicenode->dma_mask=&u64SclHvsp_DmaMask;
        //_tSclDevice.devicenode->coherent_dma_mask=u64SclHvsp_DmaMask;
    }
    //probe
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);
    stDrvSclPlatformDevice.dev.of_node = pdev->dev.of_node;

    //create device
#if defined(SCLOS_TYPE_LINUX_DEBUG)
    device_create_file(_tSclHvsp1Device.devicenode, &dev_attr_SCIQ);
    device_create_file(_tSclHvsp2Device.devicenode, &dev_attr_SCIQ);
    device_create_file(_tSclHvsp3Device.devicenode, &dev_attr_SCIQ);
    device_create_file(_tSclHvsp4Device.devicenode, &dev_attr_SCIQ);
    device_create_file(_tSclVipDevice.devicenode, &dev_attr_IQ);
#endif
    Reset_DBGMG_FLAG();
    _DrvSclVpeModuleInit();
    _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_1);
    _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_2);
    _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_3);
    _DrvSclHvspIoInit(E_DRV_SCLHVSP_IO_ID_4);
    _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_1);
    _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_2);
    _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_3);
    _DrvSclDmaIoInit(E_DRV_SCLDMA_IO_ID_4);
    _DrvSclVipIoInit();
    _DrvM2MIoInit();
    DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_ALL);
#if defined(SCLOS_TYPE_LINUX_TEST)
    UTest_init();
#endif
    return 0;
}

static int DrvSclModuleRemove(struct platform_device *pdev)
{
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLDMA_1] %s\n",__FUNCTION__);
    _DrvSclVipIoDeInit();
    _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_4);
    _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_3);
    _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_2);
    _DrvSclDmaIoDeInit(E_DRV_SCLDMA_IO_ID_1);
    _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_4);
    _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_3);
    _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_2);
    _DrvSclHvspIoDeInit(E_DRV_SCLHVSP_IO_ID_1);
    _DrvSclVpeModuleDeInit();
    //ToDo
    device_unregister(_tSclDevice.devicenode);
    cdev_del(&_tSclHvsp1Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclHvsp1Device.s32Major, _tSclHvsp1Device.s32Minor));
    device_unregister(_tSclHvsp1Device.devicenode);
    cdev_del(&_tSclHvsp2Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclHvsp2Device.s32Major, _tSclHvsp2Device.s32Minor));
    device_unregister(_tSclHvsp2Device.devicenode);
    cdev_del(&_tSclHvsp3Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclHvsp3Device.s32Major, _tSclHvsp3Device.s32Minor));
    device_unregister(_tSclHvsp3Device.devicenode);
    cdev_del(&_tSclHvsp4Device.cdev);
    device_unregister(_tSclHvsp4Device.devicenode);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclHvsp4Device.s32Major, _tSclHvsp4Device.s32Minor));
    cdev_del(&_tSclDma1Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDma1Device.s32Major, _tSclDma1Device.s32Minor));
    device_unregister(_tSclDma1Device.devicenode);
    cdev_del(&_tSclDma2Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDma2Device.s32Major, _tSclDma2Device.s32Minor));
    device_unregister(_tSclDma2Device.devicenode);
    cdev_del(&_tSclDma3Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDma3Device.s32Major, _tSclDma3Device.s32Minor));
    device_unregister(_tSclDma3Device.devicenode);
    cdev_del(&_tSclDma4Device.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDma4Device.s32Major, _tSclDma4Device.s32Minor));
    device_unregister(_tSclDma4Device.devicenode);
    cdev_del(&_tSclVipDevice.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclVipDevice.s32Major, _tSclVipDevice.s32Minor));
    device_unregister(_tSclVipDevice.devicenode);
    cdev_del(&_tSclM2MDevice.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclM2MDevice.s32Major, _tSclM2MDevice.s32Minor));
    device_unregister(_tSclM2MDevice.devicenode);
    //cdev_del(&_tSclDevice.cdev);
    //device_destroy(_tSclHvspClass, MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor));
    //class_destroy(_tSclHvspClass);
    //unregister_chrdev_region(MKDEV(_tSclHvsp1Device.s32Major, _tSclHvsp1Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclHvsp2Device.s32Major, _tSclHvsp2Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclHvsp3Device.s32Major, _tSclHvsp3Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclHvsp4Device.s32Major, _tSclHvsp4Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclDma1Device.s32Major, _tSclDma1Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclDma2Device.s32Major, _tSclDma2Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclDma3Device.s32Major, _tSclDma3Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclDma4Device.s32Major, _tSclDma4Device.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclVipDevice.s32Major, _tSclVipDevice.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclM2MDevice.s32Major, _tSclM2MDevice.s32Minor), DRV_SCL_DEVICE_COUNT);
    //unregister_chrdev_region(MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor), DRV_SCL_DEVICE_COUNT);
    return 0;
}
#else
static int DrvSclModuleProbe(struct platform_device *pdev)
{
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);

    return 0;
}
static int DrvSclModuleRemove(struct platform_device *pdev)
{
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);

    return 0;
}

#endif


//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
#if CONFIG_OF
int _DrvSclModuleInit(void)
{
    int ret = 0;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s:%d\n",__FUNCTION__,__LINE__);

    ret = platform_driver_register(&stDrvSclPlatformDriver);
    if (!ret)
    {
        SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] platform_driver_register success\n");
    }
    else
    {
        SCL_ERR( "[SCLHVSP_1] platform_driver_register failed\n");
        platform_driver_unregister(&stDrvSclPlatformDriver);
    }


    return ret;
}
void _DrvSclModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);
    platform_driver_unregister(&stDrvSclPlatformDriver);
}
#else

int _DrvSclModuleInit(void)
{
    int ret = 0;
    int s32Ret;
    dev_t  dev;
    //struct device_node *np;

    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);
    //np = of_find_compatible_node(NULL, NULL, "mstar,hvsp1");
    //if (np)
    //{
    //  SCL_DBG(SCL_DBG_LV_MDRV_IO(), "Find scl dts node\n");
    //  stDrvSclPlatformDevice.dev.of_node = of_node_get(np);
    //  of_node_put(np);
    //}
    //else
    //{
    //    return -ENODEV;
    //}

    if(_tSclDevice.s32Major)
    {
        dev     = MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor);
        s32Ret  = register_chrdev_region(dev, DRV_SCL_DEVICE_COUNT, DRV_SCL_DEVICE_NAME);
    }
    else
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tSclDevice.s32Minor, DRV_SCL_DEVICE_COUNT, DRV_SCL_DEVICE_NAME);
        _tSclDevice.s32Major  = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        SCL_ERR( "[SCLHVSP_1] Unable to get major %d\n", _tSclDevice.s32Major);
        return s32Ret;
    }

    cdev_init(&_tSclDevice.cdev, &_tSclDevice.fops);
    if (0 != (s32Ret= cdev_add(&_tSclDevice.cdev, dev, DRV_SCL_DEVICE_COUNT)))
    {
        SCL_ERR( "[SCLHVSP_1] Unable add a character device\n");
        unregister_chrdev_region(dev, DRV_SCL_DEVICE_COUNT);
        return s32Ret;
    }

    _tSclHvspClass = class_create(THIS_MODULE, SclHvspClassName);
    if(IS_ERR(_tSclHvspClass))
    {
        printk(KERN_WARNING"Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        device_create(_tSclHvspClass, NULL, dev,NULL, DRV_SCL_DEVICE_NAME);
    }

    ret = platform_driver_register(&stDrvSclPlatformDriver);

    if (!ret)
    {
        ret = platform_device_register(&stDrvSclPlatformDevice);
        if (ret)    // if register device fail, then unregister the driver.
        {
            platform_driver_unregister(&stDrvSclPlatformDriver);
            SCL_ERR( "[SCLHVSP_1] platform_driver_register failed\n");

        }
        else
        {
            SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] platform_driver_register success\n");
        }
    }


    return ret;
}
void _DrvSclModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLHVSP_1] %s\n",__FUNCTION__);

    cdev_del(&_tSclDevice.cdev);
    device_destroy(_tSclHvspClass, MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor));
    class_destroy(_tSclHvspClass);
    unregister_chrdev_region(MKDEV(_tSclDevice.s32Major, _tSclDevice.s32Minor), DRV_SCL_DEVICE_COUNT);
    platform_driver_unregister(&stDrvSclPlatformDriver);
}

#endif
EXPORT_SYMBOL(_DrvSclVpeModuleInit);
EXPORT_SYMBOL(_DrvSclVpeModuleDeInit);

module_init(_DrvSclModuleInit);
module_exit(_DrvSclModuleExit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("mstar sclhvsp ioctrl driver");
MODULE_LICENSE("GPL");
