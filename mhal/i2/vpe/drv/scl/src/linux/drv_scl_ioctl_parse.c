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
#define __DRV_SCL_IOCTL_PARSE_C__

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

//#include <linux/cdev.h>
//#include <linux/interrupt.h>
//#include <linux/poll.h>
//#include <linux/module.h>
//#include <linux/moduleparam.h>
//#include <linux/init.h>
//#include <linux/platform_device.h>
//#include <linux/blkdev.h>
//#include <linux/delay.h>
//#include <linux/err.h>
//#include <linux/slab.h>
//#include <linux/version.h>
//#include <linux/gpio.h>
//#include <linux/irq.h>
//#include <linux/sched.h>
//#include <linux/wait.h>


//#include <linux/of.h>
//#include <linux/of_irq.h>
//#include <linux/of_address.h>
#include "ms_platform.h"
#include "ms_msys.h"

#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_ioctl.h"
#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_io_wrapper.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_dma_io_wrapper.h"
#include "drv_scl_vip_ioctl.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_verchk.h"
#include "drv_scl_vip_io.h"
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define CMD_PARSING(x)  (x==IOCTL_SCLHVSP_SET_IN_CONFIG          ?  "IOCTL_SCL_SET_IN_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_OUT_CONFIG         ?  "IOCTL_SCL_SET_OUT_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_SCALING_CONFIG     ?  "IOCTL_SCL_SET_SCALING_CONFIG" : \
                         x==IOCTL_SCLHVSP_REQ_MEM_CONFIG         ?  "IOCTL_SCL_REQ_MEM_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_MISC_CONFIG        ?  "IOCTL_SCL_SET_MISC_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_POST_CROP_CONFIG   ?  "IOCTL_SCL_SET_POST_CROP_CONFIG" : \
                         x==IOCTL_SCLHVSP_GET_PRIVATE_ID_CONFIG  ?  "IOCTL_SCL_GET_PRIVATE_ID_CONFIG" : \
                         x==IOCTL_SCLHVSP_GET_INFORM_CONFIG      ?  "IOCTL_SCL_GET_INFORM_CONFIG" : \
                         x==IOCTL_SCLHVSP_RELEASE_MEM_CONFIG     ?  "IOCTL_SCL_RELEASE_MEM_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_ROTATE_CONFIG      ?  "IOCTL_SCL_SET_ROTATE_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_LOCK_CONFIG        ?  "IOCTL_SCL_SET_LOCK_CONFIG" : \
                         x==IOCTL_SCLHVSP_SET_UNLOCK_CONFIG      ?  "IOCTL_SCL_SET_UNLOCK_CONFIG" : \
                         x==IOCTL_SCLDMA_SET_IN_BUFFER_CONFIG         ?  "IOCTL_SCL_SET_IN_BUFFER_CONFIG" :  \
                         x==IOCTL_SCLDMA_SET_OUT_BUFFER_CONFIG        ?  "IOCTL_SCL_SET_OUT_BUFFER_CONFIG" :  \
                         x==IOCTL_SCLDMA_CREATE_INST_CONFIG              ?  "IOCTL_SCL_CREATE_INST_CONFIG" :\
                         x==IOCTL_SCLDMA_DESTROY_INST_CONFIG            ?  "IOCTL_SCL_DESTROY_INST_CONFIG" :\
                         x==IOCTL_SCLDMA_BUFFER_QUEUE_HANDLE_CONFIG   ?  "IOCTL_SCL_BUFFER_QUEUE_HANDLE_CONFIG" :\
                         x==IOCTL_SCLDMA_GET_INFORMATION_CONFIG       ?  "IOCTL_SCL_GET_INFORMATION_CONFIG" : \
                         "UNKNOWN")
#define CMDVIP_PARSING(x)  ( x == IOCTL_SCLVIP_SET_MCNR_CONFIG                           ?    "IOCTL_SCLVIP_SET_MCNR_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_PEAKING_CONFIG                       ?    "IOCTL_SCLVIP_SET_PEAKING_CONFIG"                               : \
                             x == IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG                 ?    "IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG"                 : \
                             x == IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT                 ?    "IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT"                 : \
                             x == IOCTL_SCLVIP_SET_DLC_CONFIG                           ?    "IOCTL_SCLVIP_SET_DLC_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_LCE_CONFIG                           ?    "IOCTL_SCLVIP_SET_LCE_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_UVC_CONFIG                           ?    "IOCTL_SCLVIP_SET_UVC_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_IHC_CONFIG                           ?    "IOCTL_SCLVIP_SET_IHC_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_ICE_CONFIG                           ?    "IOCTL_SCLVIP_SET_ICE_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG                 ?    "IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG"                 : \
                             x == IOCTL_SCLVIP_SET_IBC_CONFIG                           ?    "IOCTL_SCLVIP_SET_IBC_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_FCC_CONFIG                           ?    "IOCTL_SCLVIP_SET_FCC_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_ACK_CONFIG                           ?    "IOCTL_SCLVIP_SET_ACK_CONFIG"                           : \
                             x == IOCTL_SCLVIP_SET_NLM_CONFIG                           ?    "IOCTL_SCLVIP_SET_NLM_CONFIG"                           : \
                             x == IOCTL_VIP_SET_AIP_SRAM_CONFIG                      ?    "IOCTL_VIP_SET_AIP_SRAM_CONFIG"                      : \
                             x == IOCTL_SCLVIP_SET_VIP_CONFIG                           ?    "IOCTL_SCLVIP_SET_VIP_CONFIG"                           : \
                             x == IOCTL_VIP_SET_AIP_CONFIG                           ?    "IOCTL_VIP_SET_AIP_CONFIG"                           : \
                             x == IOCTL_SCLHVSP_SET_VTRACK_CONFIG                        ?    "IOCTL_SCLHVSP_SET_VTRACK_CONFIG"                        : \
                             x == IOCTL_SCLHVSP_SET_VTRACK_ONOFF_CONFIG                  ?    "IOCTL_SCLHVSP_SET_VTRACK_ONOFF_CONFIG"                  : \
                             x == IOCTL_SCLVIP_REGUEST_MEM                            ?    "IOCTL_SCLVIP_REGUEST_MEM"                           : \
                             "UNKNOWN")


//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvSclHvspIoFunctionConfig_t _gstSclHvspIoFunc;
extern DrvSclDmaIoFunctionConfig_t _gstSclDmaFunc;


//-------------------------------------------------------------------------------------------------
// Pirvate Function
//-------------------------------------------------------------------------------------------------
#if defined(SCLOS_TYPE_LINUX_IOCTL)
int _DrvSclDmaTransErrType(DrvSclDmaIoErrType_e eErrType)
{
    int ret;

    switch(eErrType)
    {
        case E_DRV_SCLDMA_IO_ERR_OK:
            ret = 0;
            break;
        case E_DRV_SCLDMA_IO_ERR_INVAL:
            ret = -EINVAL;
            break;
        case E_DRV_SCLDMA_IO_ERR_MULTI:
            ret = -EFAULT;
            break;

        default:
        case E_DRV_SCLDMA_IO_ERR_FAULT:
            ret = -EFAULT;
            break;
    }
    return ret;
}



int _DrvSclDmaIoctlSetInBufferConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoBufferConfig_t stIODMABufferCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoSetInBufferConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIODMABufferCfg, (DrvSclDmaIoBufferConfig_t __user *)arg, sizeof(DrvSclDmaIoBufferConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);
        eErrType = _gstSclDmaFunc.DrvSclDmaIoSetInBufferConfig(s32Handler, &stIODMABufferCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }
}

int _DrvSclDmaIoctlSetOutBufferConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoBufferConfig_t stIODMABufferCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoSetOutBufferConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIODMABufferCfg, (DrvSclDmaIoBufferConfig_t __user *)arg, sizeof(DrvSclDmaIoBufferConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);
        eErrType = _gstSclDmaFunc.DrvSclDmaIoSetOutBufferConfig(s32Handler, &stIODMABufferCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }
}
int _DrvSclDmaIoctlInstProcessConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoProcessConfig_t stIoCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoInstProcessConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIoCfg, (DrvSclDmaIoProcessConfig_t __user *)arg, sizeof(DrvSclDmaIoProcessConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);
        eErrType = _gstSclDmaFunc.DrvSclDmaIoInstProcessConfig(s32Handler, &stIoCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }
}
int _DrvSclDmaIoctlInstFlip(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    eErrType = _DrvSclDmaIoInstFlip(s32Handler);
    return _DrvSclDmaTransErrType(eErrType);
}
int _DrvSclDmaIoctlCreateInstConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoLockConfig_t stIoLockCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoCreateInstConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIoLockCfg, (DrvSclDmaIoLockConfig_t __user *)arg, sizeof(DrvSclDmaIoLockConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclDmaFunc.DrvSclDmaIoCreateInstConfig(s32Handler, &stIoLockCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }
}

int _DrvSclDmaIoctlDestroyInstConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoLockConfig_t stIoUnlocCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoDestroyInstConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIoUnlocCfg, (DrvSclDmaIoLockConfig_t __user *)arg, sizeof(DrvSclDmaIoLockConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclDmaFunc.DrvSclDmaIoDestroyInstConfig(s32Handler, &stIoUnlocCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }
}

int _DrvSclDmaIoctlSetExternalInputConfig(struct file *filp, unsigned long arg)
{
    DrvSclDmaIoExternalInputConfig_t stIoExtInputCfg;

    if(_gstSclDmaFunc.DrvSclDmaIoSetExternalInputConfig == NULL)
    {
        SCL_ERR("%s %d:: Null Func\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIoExtInputCfg, (DrvSclDmaIoExternalInputConfig_t __user *)arg, sizeof(DrvSclDmaIoExternalInputConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclDmaIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclDmaFunc.DrvSclDmaIoSetExternalInputConfig(s32Handler, &stIoExtInputCfg);
        return _DrvSclDmaTransErrType(eErrType);
    }

}

int _DrvSclHvspIoctlTransErrType(DrvSclHvspIoErrType_e eErrType)
{
    int ret;

    switch(eErrType)
    {
        case E_DRV_SCLHVSP_IO_ERR_OK:
            ret = 0;
            break;
        case E_DRV_SCLHVSP_IO_ERR_INVAL:
            ret = -EINVAL;
            break;
        case E_DRV_SCLHVSP_IO_ERR_MULTI:
            ret = -EFAULT;
            break;

        default:
        case E_DRV_SCLHVSP_IO_ERR_FAULT:
            ret = -EFAULT;
            break;
    }
    return ret;
}



int _DrvSclHvspIoctlSetInputConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoInputConfig_t stIOInCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetInputConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIOInCfg, (DrvSclHvspIoInputConfig_t __user *)arg, sizeof(DrvSclHvspIoInputConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);
        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetInputConfig(s32Handler, &stIOInCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}

int _DrvSclHvspIoctlSetOutputConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoOutputConfig_t stOutCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetOutputConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stOutCfg, (DrvSclHvspIoOutputConfig_t __user *)arg, sizeof(DrvSclHvspIoOutputConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetOutputConfig(s32Handler, &stOutCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}


int _DrvSclHvspIoctlSetScalingConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoScalingConfig_t stIOSclCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetScalingConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIOSclCfg, (DrvSclHvspIoScalingConfig_t __user *)arg, sizeof(DrvSclHvspIoScalingConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);
        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetScalingConfig(s32Handler, &stIOSclCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}
int _DrvSclHvspIoctlSetMiscConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoMiscConfig_t stIOMiscCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetMiscConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIOMiscCfg, (DrvSclHvspIoMiscConfig_t __user *)arg, sizeof(DrvSclHvspIoMiscConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetMiscConfig(s32Handler, &stIOMiscCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}

int _DrvSclHvspIoctlSetPostCropConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoPostCropConfig_t stIOPostCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetPostCropConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIOPostCfg, (DrvSclHvspIoPostCropConfig_t __user *)arg, sizeof(DrvSclHvspIoPostCropConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetPostCropConfig(s32Handler, &stIOPostCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}

int _DrvSclHvspIoctlGetInformConfig(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoScInformConfig_t stIOInfoCfg;
    DrvSclHvspIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    int Ret = 0;

    if(_gstSclHvspIoFunc.DrvSclHvspIoGetInformConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    eErrType = _gstSclHvspIoFunc.DrvSclHvspIoGetInformConfig(s32Handler, &stIOInfoCfg);

    if(eErrType == E_DRV_SCLHVSP_IO_ERR_OK)
    {
        if(copy_to_user((DrvSclHvspIoScInformConfig_t __user *)arg, &stIOInfoCfg, sizeof(DrvSclHvspIoScInformConfig_t)))
        {
            Ret = -EFAULT;
        }
    }
    else
    {
        Ret = _DrvSclHvspIoctlTransErrType(eErrType);
    }

    return Ret;
}

int _DrvSclHvspIoctlSetRotateCfg(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoRotateConfig_t stIoCfg;

    if(_gstSclHvspIoFunc.DrvSclHvspIoSetRotateConfig == NULL)
    {
        SCL_ERR("%s %d: Null Function\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    if(copy_from_user(&stIoCfg, (DrvSclHvspIoRotateConfig_t __user *)arg, sizeof(DrvSclHvspIoRotateConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclHvspIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _gstSclHvspIoFunc.DrvSclHvspIoSetRotateConfig(s32Handler, &stIoCfg);
        return _DrvSclHvspIoctlTransErrType(eErrType);
    }
}

int _mdrv_ms_hvsp_io_set_vtrack_config(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoVtrackConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclHvspIoVtrackConfig_t __user *)arg, sizeof(DrvSclHvspIoVtrackConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _DrvSclHvspIoSetVtrackConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _mdrv_ms_hvsp_io_set_vtrack_onoff_config(struct file *filp, unsigned long arg)
{
    DrvSclHvspIoVtrackOnOffConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclHvspIoVtrackOnOffConfig_t __user *)arg, sizeof(DrvSclHvspIoVtrackOnOffConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _DrvSclHvspIoSetVtrackOnOffConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}
int _mdrv_ms_vip_trans_errtype(DrvSclVipIoErrType_e eErrType)
{
    int ret;

    switch(eErrType)
    {
        case E_DRV_SCLVIP_IO_ERR_OK:
            ret = 0;
            break;
        case E_DRV_SCLVIP_IO_ERR_INVAL:
            ret = -EINVAL;
            break;
        case E_DRV_SCLVIP_IO_ERR_MULTI:
            ret = -EFAULT;
            break;

        default:
        case E_DRV_SCLVIP_IO_ERR_FAULT:
            ret = -EIO;
            break;
    }
    return ret;
}

int _DrvSclVipIoctlSetPeakingConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoPeakingConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoPeakingConfig_t __user *)arg, sizeof(DrvSclVipIoPeakingConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetPeakingConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetDlcHistogramConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoDlcHistogramConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoDlcHistogramConfig_t __user *)arg, sizeof(DrvSclVipIoDlcHistogramConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetDlcHistogramConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlGetDlcHistogramConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoDlcHistogramReport_t stCfg;
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    int Ret = 0;

    eErrType = DrvSclVipIoGetDlcHistogramConfig(s32Handler, &stCfg);


    if(eErrType == E_DRV_SCLVIP_IO_ERR_OK)
    {
        if (copy_to_user( (DrvSclVipIoDlcHistogramReport_t __user *)arg, &stCfg, sizeof(DrvSclVipIoDlcHistogramReport_t) ))
        {
            Ret= -EFAULT;
        }
    }
    else
    {
        Ret = _mdrv_ms_vip_trans_errtype(eErrType);
    }

    return Ret;
}

int _DrvSclVipIoctlSetDlcConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoDlcConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoDlcConfig_t __user *)arg, sizeof(DrvSclVipIoDlcConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetDlcConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetLceConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoLceConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoLceConfig_t __user *)arg, sizeof(DrvSclVipIoLceConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetLceConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetUvcConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoUvcConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoUvcConfig_t __user *)arg, sizeof(DrvSclVipIoUvcConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetUvcConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}


int _DrvSclVipIoctlSetIhcConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoIhcConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoIhcConfig_t __user *)arg, sizeof(DrvSclVipIoIhcConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetIhcConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetIccConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoIccConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoIccConfig_t __user *)arg, sizeof(DrvSclVipIoIccConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetIccConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlIhcIceAdpYConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoIhcIccConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoIhcIccConfig_t __user *)arg, sizeof(DrvSclVipIoIhcIccConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetIhcIceAdpYConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetIbcConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoIbcConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoIbcConfig_t __user *)arg, sizeof(DrvSclVipIoIbcConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetIbcConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetFccConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoFccConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoFccConfig_t __user *)arg, sizeof(DrvSclVipIoFccConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetFccConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetNlmConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoNlmConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoNlmConfig_t __user *)arg, sizeof(DrvSclVipIoNlmConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetNlmConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetAckConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoAckConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoAckConfig_t __user *)arg, sizeof(DrvSclVipIoAckConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetAckConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}


int _DrvSclVipIoctlSetMcnrConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoMcnrConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoMcnrConfig_t __user *)arg, sizeof(DrvSclVipIoMcnrConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetMcnrConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }

}

int _DrvSclVipIoctlSetAipConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoAipConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoAipConfig_t __user *)arg, sizeof(DrvSclVipIoAipConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetAipConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlSetAipSramConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoAipSramConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoAipSramConfig_t __user *)arg, sizeof(DrvSclVipIoAipSramConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetAipSramConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}



int _DrvSclVipIoctlSetVipConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoConfig_t stCfg;

    if(copy_from_user(&stCfg, (DrvSclVipIoConfig_t __user *)arg, sizeof(DrvSclVipIoConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetVipConfig(s32Handler, &stCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }

}


int _DrvSclVipIoctlGetVersion(struct file *filp, unsigned long arg)
{

    DrvSclVipIoVersionConfig_t stIoVersionCfg;
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    int Ret = 0;

    eErrType = DrvSclVipIoGetVersion(s32Handler, &stIoVersionCfg);

    if(eErrType == E_DRV_SCLVIP_IO_ERR_OK)
    {
        if(copy_to_user((DrvSclVipIoVersionConfig_t __user *)arg,   &stIoVersionCfg, sizeof(DrvSclVipIoVersionConfig_t)))
        {
            Ret = -EFAULT;
        }
    }
    else
    {
        Ret = _mdrv_ms_vip_trans_errtype(eErrType);
    }

    return Ret;
}
int _DrvSclVipIoctlGetPrivateIdConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoPrivateIdConfig_t stCfg;
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    int Ret = 0;

    eErrType = DrvSclVipIoGetPrivateIdConfig(s32Handler, &stCfg);

    if(eErrType == E_DRV_SCLVIP_IO_ERR_OK)
    {
        if(copy_to_user((DrvSclVipIoPrivateIdConfig_t __user *)arg, &stCfg, sizeof(DrvSclVipIoPrivateIdConfig_t)))
        {
            Ret = -EFAULT;
        }
    }
    else
    {
        Ret = _mdrv_ms_vip_trans_errtype(eErrType);
    }

    return Ret;
}
int _DrvSclVipIoctlCreateInstConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoLockConfig_t stIoLockCfg;

    if(copy_from_user(&stIoLockCfg, (DrvSclVipIoLockConfig_t __user *)arg, sizeof(DrvSclVipIoLockConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _DrvSclVipIoCreateInstConfig(s32Handler, &stIoLockCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlDestroyInstConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoLockConfig_t stIoUnlocCfg;

    if(copy_from_user(&stIoUnlocCfg, (DrvSclVipIoLockConfig_t __user *)arg, sizeof(DrvSclVipIoLockConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = _DrvSclVipIoDestroyInstConfig(s32Handler, &stIoUnlocCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}
int _DrvSclVipIoctlGetRoiConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoWdrRoiReport_t stIoCfg;
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    int Ret = 0;

    eErrType = DrvSclVipIoGetWdrRoiHistConfig(s32Handler, &stIoCfg);
    if(eErrType == E_DRV_SCLVIP_IO_ERR_OK)
    {
        if (copy_to_user( (DrvSclVipIoWdrRoiReport_t __user *)arg, &stIoCfg, sizeof(DrvSclVipIoWdrRoiReport_t) ))
        {
            Ret= -EFAULT;
        }
    }
    else
    {
        Ret = _mdrv_ms_vip_trans_errtype(eErrType);
    }
    return Ret;
}
int _DrvSclVipIoctlGetNrHistogram(struct file *filp, unsigned long arg)
{
    DrvSclVipIoNrHist_t stIoCfg;
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    if(copy_from_user(&stIoCfg, (DrvSclVipIoNrHist_t __user *)arg, sizeof(DrvSclVipIoNrHist_t)))
    {
        return -EFAULT;
    }
    eErrType = DrvSclVipIoGetNRHistogram(s32Handler, &stIoCfg);
    return _mdrv_ms_vip_trans_errtype(eErrType);
}
int _DrvSclVipIoctlSetMaskOnOff(struct file *filp, unsigned long arg)
{
    DrvSclVipIoSetMaskOnOff_t stIoCfg;

    if(copy_from_user(&stIoCfg, (DrvSclVipIoSetMaskOnOff_t __user *)arg, sizeof(DrvSclVipIoSetMaskOnOff_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetWdrRoiMask(s32Handler, &stIoCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}
int _DrvSclVipIoctlSetRoiConfig(struct file *filp, unsigned long arg)
{
    DrvSclVipIoWdrRoiHist_t stIoCfg;

    if(copy_from_user(&stIoCfg, (DrvSclVipIoWdrRoiHist_t __user *)arg, sizeof(DrvSclVipIoWdrRoiHist_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoSetWdrRoiHistConfig(s32Handler, &stIoCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlReqMemory(struct file *filp, unsigned long arg)
{
	DrvSclVipIoReqMemConfig_t stIoCfg;

    if(copy_from_user(&stIoCfg, (DrvSclVipIoReqMemConfig_t __user *)arg, sizeof(DrvSclVipIoReqMemConfig_t)))
    {
        return -EFAULT;
    }
    else
    {
        DrvSclVipIoErrType_e eErrType;
        s32 s32Handler = *((s32 *)filp->private_data);

        eErrType = DrvSclVipIoReqmemConfig(s32Handler, &stIoCfg);
        return _mdrv_ms_vip_trans_errtype(eErrType);
    }
}

int _DrvSclVipIoctlInstFlip(struct file *filp, unsigned long arg)
{
    DrvSclVipIoErrType_e eErrType;
    s32 s32Handler = *((s32 *)filp->private_data);
    eErrType = DrvSclVipIoctlInstFlip(s32Handler);
    return _mdrv_ms_vip_trans_errtype(eErrType);
}
#endif


bool _DrvSclVipIoCreateIqInst(s32 *ps32Handle)
{
    DrvSclVipIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclVipIoLockConfig_t));
    stIoInCfg.ps32IdBuf = ps32Handle;
    stIoInCfg.u8BufSize = 1;
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclVipIoCreateInstConfig(*ps32Handle,&stIoInCfg))
    {
        SCL_ERR("[SCLMOD]%s Create Inst Fail\n",__FUNCTION__);
        return 0;
    }
    _DrvSclVipIoReqWdrMloadBuffer(*ps32Handle);
    return 1;
}
bool _DrvSclVipIoDestroyIqInst(s32 *s32Handle)
{
    DrvSclVipIoLockConfig_t stIoInCfg;
    DrvSclOsMemset(&stIoInCfg,0,sizeof(DrvSclVipIoLockConfig_t));
    stIoInCfg.ps32IdBuf = s32Handle;
    stIoInCfg.u8BufSize = 1;
    //free wdr mload
    _DrvSclVipIoFreeWdrMloadBuffer(*s32Handle);
    FILL_VERCHK_TYPE(stIoInCfg, stIoInCfg.VerChk_Version, stIoInCfg.VerChk_Size, DRV_SCLDMA_VERSION);
    if(_DrvSclVipIoDestroyInstConfig(*s32Handle,&stIoInCfg))
    {
        SCL_ERR("[VPE]%s Destroy Inst Fail\n",__FUNCTION__);
        return 0;
    }
    return 1;
}

/*Kernel VIP API function */
s32 DrvSclVipIoOpen(DrvSclVipIoIdType_e enSlcVipId)
{
    s32 s32Handler;
    if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_VIP) == 0)
    {
        _DrvSclVipIoInit();
        DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_VIP);
    }
    s32Handler = _DrvSclVipIoOpen(enSlcVipId);
    if(!_DrvSclVipIoCreateIqInst(&s32Handler))
    {
        SCL_ERR("[SCLMOD]%s Create Inst Fail\n",__FUNCTION__);
        return -1;
    }
    return s32Handler;
}

DrvSclVipIoErrType_e DrvSclVipIoRelease(s32 s32Handler)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    if(!_DrvSclVipIoDestroyIqInst(&s32Handler))
    {
        SCL_ERR("[SCLMOD]%s Destroy Inst Fail\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    return _DrvSclVipIoRelease(s32Handler);
}

DrvSclVipIoErrType_e DrvSclVipIoSetPeakingConfig(s32 s32Handler, DrvSclVipIoPeakingConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetPeakingConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetDlcHistogramConfig(s32Handler, pstCfg);
}
DrvSclVipIoErrType_e DrvSclVipIoGetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramReport_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetDlcHistogramConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetDlcConfig(s32 s32Handler, DrvSclVipIoDlcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetDlcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e  DrvSclVipIoSetLceConfig(s32 s32Handler,DrvSclVipIoLceConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetLceConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetUvcConfig(s32 s32Handler, DrvSclVipIoUvcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetUvcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIhcConfig(s32 s32Handler,DrvSclVipIoIhcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIhcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIccConfig(s32 s32Handler, DrvSclVipIoIccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIccConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIhcIceAdpYConfig(s32 s32Handler,DrvSclVipIoIhcIccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIhcIceAdpYConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIbcConfig(s32 s32Handler,DrvSclVipIoIbcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIbcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetFccConfig(s32 s32Handler,DrvSclVipIoFccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetFccConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetNlmConfig(s32 s32Handler,DrvSclVipIoNlmConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetNlmConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAckConfig(s32 s32Handler, DrvSclVipIoAckConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAckConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAipConfig(s32Handler, pstIoConfig);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAipSramConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetVipConfig(s32 s32Handler,DrvSclVipIoConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetVipConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiHist_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetRoiConfig(s32Handler,pstCfg);
    return eErrType;
}

DrvSclVipIoErrType_e DrvSclVipIoGetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiReport_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoGetWdrHistogram(s32Handler,pstCfg);
    return eErrType;
}

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiMask(s32 s32Handler,DrvSclVipIoSetMaskOnOff_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetMaskOnOff(s32Handler, pstCfg);
    return eErrType;
}
DrvSclVipIoErrType_e DrvSclVipIoSetMcnrConfig(s32 s32Handler, DrvSclVipIoMcnrConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetMcnrConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoReqmemConfig(s32 s32Handler, DrvSclVipIoReqMemConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoReqmemConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetVersion(s32 s32Handler,DrvSclVipIoVersionConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetVersion(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetNRHistogram(s32 s32Handler, DrvSclVipIoNrHist_t *pstIOCfg)
{
	if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetNRHistogram(s32Handler, pstIOCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetPrivateIdConfig(s32 s32Handler, DrvSclVipIoPrivateIdConfig_t *pstIOCfg)
{
	if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetPrivateIdConfig(s32Handler, pstIOCfg);
}

u8 DrvSclIrqGetSclProcessCnt(void);
DrvSclVipIoErrType_e DrvSclVipIoctlInstFlip(s32 s32Handler)
{
    DrvSclVipIoErrType_e eErrType;
    static u8 su8SclprocessCnt = 0;
    u8 u8SclprocessCnt = 0;
    static u8 u8SclprocessEqCnt = 0;
    u8SclprocessCnt = DrvSclIrqGetSclProcessCnt();
    if(u8SclprocessCnt == su8SclprocessCnt && u8SclprocessEqCnt)
    {
        SCL_DBGERR("[SCL]%s process cnt = twice\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_OK;
    }
    else if(u8SclprocessCnt == su8SclprocessCnt)
    {
        u8SclprocessEqCnt = 1;
    }
    else
    {
        u8SclprocessEqCnt = 0;
        su8SclprocessCnt = u8SclprocessCnt;
    }
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetFlip(s32Handler);
    return eErrType;
}
//----------------------------------------------------------------------------------------------

#if defined(SCLOS_TYPE_LINUX_IOCTL)
//==============================================================================
long DrvSclVipIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
    /* not allow query or command once driver suspend */
    SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_VIP, "[VIP] IOCTL_NUM:: == %s ==  \n", (CMDVIP_PARSING(u32Cmd)));
    switch(u32Cmd)
    {
        case IOCTL_SCLVIP_SET_MCNR_CONFIG:
			retval = _DrvSclVipIoctlSetMcnrConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_PEAKING_CONFIG:
			retval = _DrvSclVipIoctlSetPeakingConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_DLC_HISTOGRAM_CONFIG:
			retval = _DrvSclVipIoctlSetDlcHistogramConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_GET_DLC_HISTOGRAM_REPORT:
			retval = _DrvSclVipIoctlGetDlcHistogramConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_DLC_CONFIG:
			retval = _DrvSclVipIoctlSetDlcConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_LCE_CONFIG:
			retval = _DrvSclVipIoctlSetLceConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_UVC_CONFIG:
			retval = _DrvSclVipIoctlSetUvcConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_IHC_CONFIG:
			retval = _DrvSclVipIoctlSetIhcConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_ICE_CONFIG:
			retval = _DrvSclVipIoctlSetIccConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_IHC_ICE_ADP_Y_CONFIG:
			retval = _DrvSclVipIoctlIhcIceAdpYConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_IBC_CONFIG:
			retval = _DrvSclVipIoctlSetIbcConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_FCC_CONFIG:
			retval = _DrvSclVipIoctlSetFccConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_ACK_CONFIG:
			retval = _DrvSclVipIoctlSetAckConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_NLM_CONFIG:
 			retval = _DrvSclVipIoctlSetNlmConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_VIP_CONFIG:
			retval = _DrvSclVipIoctlSetVipConfig(filp, u32Arg);
            break;
        case IOCTL_VIP_SET_AIP_CONFIG:
			retval = _DrvSclVipIoctlSetAipConfig(filp, u32Arg);
            break;
        case IOCTL_VIP_SET_AIP_SRAM_CONFIG:
			retval = _DrvSclVipIoctlSetAipSramConfig(filp, u32Arg);
            break;
        case IOCTL_VIP_GET_VERSION_CONFIG:
			retval = _DrvSclVipIoctlGetVersion(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_GET_PRIVATE_ID_CONFIG:
			retval = _DrvSclVipIoctlGetPrivateIdConfig(filp, u32Arg);
            break;

        case IOCTL_SCLVIP_CREATE_INST_CONFIG:
			retval = _DrvSclVipIoctlCreateInstConfig(filp, u32Arg);
            break;

        case IOCTL_SCLVIP_DESTROY_INST_CONFIG:
			retval = _DrvSclVipIoctlDestroyInstConfig(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_SET_ROI_CONFIG:
			retval = _DrvSclVipIoctlSetRoiConfig(filp, u32Arg);
            break;

        case IOCTL_SCLVIP_SET_MASK_ONOFF:
			retval = _DrvSclVipIoctlSetMaskOnOff(filp, u32Arg);
            break;
        case IOCTL_SCLVIP_GET_ROI_CONFIG:
			retval = _DrvSclVipIoctlGetRoiConfig(filp, u32Arg);
            break;

        case IOCTL_SCLVIP_GET_NR_HISTGRAM:
			retval = _DrvSclVipIoctlGetNrHistogram(filp, u32Arg);
            break;
        case IOCTL_SCLDMA_FILP_INST:
            retval = _DrvSclVipIoctlInstFlip(filp, u32Arg);
            break;
		case IOCTL_SCLVIP_REGUEST_MEM:
			retval = _DrvSclVipIoctlReqMemory(filp, u32Arg);
            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            SCL_ERR( "[VIP] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
    }
    return retval;
}

//-------------------------------------------------------------------------------------------------
// Public Function
//-------------------------------------------------------------------------------------------------
int DrvSclVpeIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
    switch(u32Cmd)
    {
        case IOCTL_VPE_TEST_CASE:
            SCL_ERR( "[SCLVPE] VPE IOCtl Unit Test\n ");
            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            SCL_ERR( "[SCLDMA] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
    }
    return retval;

}

int DrvSclM2MIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
    switch(u32Cmd)
    {
        case IOCTL_SCLDMA_SET_IN_BUFFER_CONFIG:
            retval = _DrvSclDmaIoctlSetInBufferConfig(filp, u32Arg);
            break;
        case IOCTL_SCLDMA_SET_OUT_BUFFER_CONFIG:
            retval = _DrvSclDmaIoctlSetOutBufferConfig(filp, u32Arg);
            break;

        case IOCTL_SCLDMA_INST_PROCESS_CONFIG:
            retval = _DrvSclDmaIoctlInstProcessConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_SET_IN_CONFIG:
            retval = _DrvSclHvspIoctlSetInputConfig(filp, u32Arg);
            break;
        case IOCTL_SCLHVSP_SET_SCALING_CONFIG:
            retval = _DrvSclHvspIoctlSetScalingConfig(filp, u32Arg);
            break;
        case IOCTL_SCLDMA_FILP_INST:
            retval = _DrvSclDmaIoctlInstFlip(filp, u32Arg);
            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            SCL_ERR( "[SCLDMA] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
    }
    return retval;

}

int DrvSclDmaIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;


    switch(u32Cmd)
    {
        case IOCTL_SCLDMA_SET_IN_BUFFER_CONFIG:
            retval = _DrvSclDmaIoctlSetInBufferConfig(filp, u32Arg);
            break;
        case IOCTL_SCLDMA_SET_OUT_BUFFER_CONFIG:
            retval = _DrvSclDmaIoctlSetOutBufferConfig(filp, u32Arg);
            break;
        case IOCTL_SCLDMA_FILP_INST:
            retval = _DrvSclDmaIoctlInstFlip(filp, u32Arg);
            break;

        case IOCTL_SCLDMA_CREATE_INST_CONFIG:
            retval = _DrvSclDmaIoctlCreateInstConfig(filp, u32Arg);
            break;

        case IOCTL_SCLDMA_DESTROY_INST_CONFIG:
            retval = _DrvSclDmaIoctlDestroyInstConfig(filp, u32Arg);
            break;

        case IOCTL_SCLDMA_INST_PROCESS_CONFIG:
            retval = _DrvSclDmaIoctlInstProcessConfig(filp, u32Arg);
            break;

        default:  /* redundant, as cmd was checked against MAXNR */
            SCL_ERR( "[SCLDMA] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
    }

    return retval;

}

int DrvSclHvspIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;


    switch(u32Cmd)
    {
        case IOCTL_SCLHVSP_SET_IN_CONFIG:
            retval = _DrvSclHvspIoctlSetInputConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_SET_OUT_CONFIG:
            retval = _DrvSclHvspIoctlSetOutputConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_SET_SCALING_CONFIG:
            retval = _DrvSclHvspIoctlSetScalingConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_SET_MISC_CONFIG:
            retval = _DrvSclHvspIoctlSetMiscConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_SET_POST_CROP_CONFIG:
            retval = _DrvSclHvspIoctlSetPostCropConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_GET_PRIVATE_ID_CONFIG:
            retval = _DrvSclHvspIoctlGetPrivateIdConfig(filp, u32Arg);
            break;

        case IOCTL_SCLHVSP_GET_INFORM_CONFIG:
            retval = _DrvSclHvspIoctlGetInformConfig(filp, u32Arg);
            break;
        case IOCTL_SCLHVSP_SET_ROTATE_CONFIG:
            retval = _DrvSclHvspIoctlSetRotateCfg(filp, u32Arg);
            break;
        case IOCTL_SCLHVSP_SET_VTRACK_CONFIG:
            retval = _mdrv_ms_hvsp_io_set_vtrack_config(filp, u32Arg);
            break;
        case IOCTL_SCLHVSP_SET_VTRACK_ONOFF_CONFIG:
            retval = _mdrv_ms_hvsp_io_set_vtrack_onoff_config(filp, u32Arg);
            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            SCL_ERR( "[SCLHVSP] ERROR IOCtl number %x\n ",u32Cmd);
            retval = -ENOTTY;
            break;
    }

    return retval;
}
#endif

int DrvSclIoctlParse(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
#if defined(SCLOS_TYPE_LINUX_IOCTL)
    SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_SC1, "[SCL %lx] IOCTL_NUM:: == %s ==  \n", (*((s32 *)filp->private_data) & 0xFFFF), (CMD_PARSING(u32Cmd)));

    if((*((s32 *)filp->private_data) & HANDLER_PRE_MASK)==SCLDMA_HANDLER_PRE_FIX )
    {
        DrvSclDmaIoctlParse(filp, u32Cmd, u32Arg);
    }
    else if ((*((s32 *)filp->private_data) & HANDLER_PRE_MASK)==SCLHVSP_HANDLER_PRE_FIX )
    {
        DrvSclHvspIoctlParse(filp, u32Cmd, u32Arg);
    }
    else if ((*((s32 *)filp->private_data) & HANDLER_PRE_MASK)==SCLVIP_HANDLER_PRE_FIX )
    {
        DrvSclVipIoctlParse(filp, u32Cmd, u32Arg);
    }
    else if ((*((s32 *)filp->private_data) & HANDLER_PRE_MASK)==SCLVPE_HANDLER_PRE_FIX )
    {
        //ToDo Get Ioctl handler by device id, not general
        DrvSclVpeIoctlParse(filp, u32Cmd, u32Arg);
    }
    else if ((*((s32 *)filp->private_data) & HANDLER_PRE_MASK)==SCLM2M_HANDLER_PRE_FIX )
    {
        //ToDo Get Ioctl handler by device id, not general
        DrvSclM2MIoctlParse(filp, u32Cmd, u32Arg);
    }
    else
    {
        BUG();
    }
#endif
    return retval;
}

//==============================================================================
#if defined (SCLOS_TYPE_LINUX_KERNEL)
EXPORT_SYMBOL(DrvSclVipIoOpen);
EXPORT_SYMBOL(DrvSclVipIoRelease);
EXPORT_SYMBOL(DrvSclVipIoSetMcnrConfig);
EXPORT_SYMBOL(DrvSclVipIoReqmemConfig);
EXPORT_SYMBOL(DrvSclVipIoSetPeakingConfig);
EXPORT_SYMBOL(DrvSclVipIoSetDlcHistogramConfig);
EXPORT_SYMBOL(DrvSclVipIoGetDlcHistogramConfig);
EXPORT_SYMBOL(DrvSclVipIoSetDlcConfig);
EXPORT_SYMBOL(DrvSclVipIoSetLceConfig);
EXPORT_SYMBOL(DrvSclVipIoSetUvcConfig);
EXPORT_SYMBOL(DrvSclVipIoSetIhcConfig);
EXPORT_SYMBOL(DrvSclVipIoSetIccConfig);
EXPORT_SYMBOL(DrvSclVipIoSetIhcIceAdpYConfig);
EXPORT_SYMBOL(DrvSclVipIoSetIbcConfig);
EXPORT_SYMBOL(DrvSclVipIoSetFccConfig);
EXPORT_SYMBOL(DrvSclVipIoSetNlmConfig);
EXPORT_SYMBOL(DrvSclVipIoSetAckConfig);
EXPORT_SYMBOL(DrvSclVipIoSetAipConfig);
EXPORT_SYMBOL(DrvSclVipIoSetAipSramConfig);
EXPORT_SYMBOL(DrvSclVipIoSetVipConfig);
EXPORT_SYMBOL(DrvSclVipIoSetWdrRoiHistConfig);
EXPORT_SYMBOL(DrvSclVipIoGetWdrRoiHistConfig);
EXPORT_SYMBOL(DrvSclVipIoSetWdrRoiMask);
EXPORT_SYMBOL(DrvSclVipIoGetVersion);
EXPORT_SYMBOL(DrvSclVipIoctlInstFlip);
EXPORT_SYMBOL(DrvSclVipIoGetNRHistogram);
#endif
