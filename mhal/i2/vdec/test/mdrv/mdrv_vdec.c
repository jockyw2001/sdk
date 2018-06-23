//===================================================================================================

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <MsTypes.h>
//#include <apiVDEC_EX.h>
//#include <apiVDEC_EX_v2.h>
//#include "vdec_ex_private.h"
#include "vdec_api.h"
#include "mdrv_vdec_io.h"
#include "mdrv_vdec_io_st.h"

////////////////////////////////////////////////////////////////////////////////
void CamOsDebug(const char *szFmt, ...)
{
    va_list tArgs;

    va_start(tArgs, szFmt);
    vprintk(szFmt, tArgs);
    va_end(tArgs);
}

void* CamOsMemCalloc(u32 nNum, u32 nSize)
{
    return kzalloc(nSize, GFP_KERNEL);
}

////////////////////////////////////////////////////////////////////////////////

typedef struct _driver_private_data
{
  int  flag;
} driver_private_data;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
#define CONV_VDEC_RET(x)   ((x==E_VDEC_EX_OK) ? 0 : -1)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int mdrv_vdec_open(struct inode *inode, struct file *filp)
{
    driver_private_data *dpd;

     dpd = (driver_private_data *)filp->private_data;
     if (!dpd)
     {
         dpd = (driver_private_data *)CamOsMemCalloc(1, sizeof(driver_private_data));
         filp->private_data = dpd;
    }
    return 0;
}

int mdrv_vdec_release(struct inode *inode, struct file *filp)
{
    driver_private_data *dpd;

    CamOsDebug("mdrv_vdec_release !\n");
    dpd = (driver_private_data *)filp->private_data;
    return 0;
}

#define COPY_FROM_USER(type,x,y)  if (copy_from_user(x, (type __user *)y, sizeof(type))) \
                                   {                             \
                                       return -EFAULT;          \
                                   }


long mdrv_vdec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    driver_private_data *dpd = (driver_private_data*)filp->private_data;
    VDEC_EX_Result eRet = E_VDEC_EX_OK;
    IOCTL_PARAM_VDEC_GET_FREE_STREAM_t tGetFreeStreamParam;
    IOCTL_PARAM_VDEC_INIT_t tInitParam;
    IOCTL_PARAM_VDEC_PRESET_CONTROL_t tPreCtrlParam;
    IOCTL_PARAM_VDEC_SET_CONTROL_t tSetCtrlParam;
    IOCTL_PARAM_VDEC_GET_CONTROL_t tGetCtrlParam;
    IOCTL_PARAM_VDEC_MAPINFO_t     tMapInfo;
    IOCTL_PARAM_VDEC_UNMAPINFO_t   tUnMapInfo;
    IOCTL_PARAM_VDEC_JPDSetVerificationMode_t tJpdVerif;

    // Should we do that?
    if(dpd == NULL)
    {
      CamOsDebug("dpd NULL !!\n");
       return -EFAULT;
    }
    //expire = PollSampleGetExpireFromUser(arg);
    switch(cmd)
    {
        case IOCTL_CMD_VDEC_EX_GET_FREE_STREAM:
            if(copy_from_user(&tGetFreeStreamParam,
                              (IOCTL_PARAM_VDEC_GET_FREE_STREAM_t __user *)arg,
                              sizeof(IOCTL_PARAM_VDEC_GET_FREE_STREAM_t)) )
            {
               return -EFAULT;
            }

            eRet = mdrv_MApi_VDEC_EX_GetFreeStream(&tGetFreeStreamParam.tParam);
            if (eRet == E_VDEC_EX_OK)
            {
                if(copy_to_user((IOCTL_PARAM_VDEC_GET_FREE_STREAM_t __user *)arg,
                    &tGetFreeStreamParam, sizeof(IOCTL_PARAM_VDEC_GET_FREE_STREAM_t)))
                {
                    return -EFAULT;
                }
            }
            break;

        case IOCTL_CMD_VDEC_EX_INIT:
            if(copy_from_user(&tInitParam,
                               (IOCTL_PARAM_VDEC_INIT_t __user *)arg,
                               sizeof(IOCTL_PARAM_VDEC_INIT_t)) )
            {
                return -EFAULT;
            }
            eRet = mdrv_MApi_VDEC_EX_Init(&tInitParam.tStreamId, &tInitParam.tParam);
             //ToDo: check if copy_to_user is needed
            break;

        case IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL:
            if(copy_from_user(&tPreCtrlParam,
                               (IOCTL_PARAM_VDEC_PRESET_CONTROL_t __user *)arg,
                               sizeof(IOCTL_PARAM_VDEC_PRESET_CONTROL_t)) )
            {
                return -EFAULT;
            }
            eRet = mdrv_MApi_VDEC_EX_PreSetControl(&tPreCtrlParam.tStreamId, tPreCtrlParam.eCmdId, &tPreCtrlParam.tParam);

            break;

       case IOCTL_CMD_VDEC_EX_SET_CONTROL:
           if(copy_from_user(&tSetCtrlParam,
                              (IOCTL_PARAM_VDEC_SET_CONTROL_t __user *)arg,
                              sizeof(IOCTL_PARAM_VDEC_SET_CONTROL_t)) )
           {
               return -EFAULT;
           }
           eRet = mdrv_MApi_VDEC_EX_SetControl(&tSetCtrlParam.tStreamId, tSetCtrlParam.eCmdId, &tSetCtrlParam.tParam);

           break;

        case IOCTL_CMD_VDEC_EX_GET_CONTROL:
            if(copy_from_user(&tGetCtrlParam,
                               (IOCTL_PARAM_VDEC_GET_CONTROL_t __user *)arg,
                               sizeof(IOCTL_PARAM_VDEC_GET_CONTROL_t)) )
            {
                return -EFAULT;
            }
            eRet = mdrv_MApi_VDEC_EX_GetControl(&tGetCtrlParam.tStreamId, tGetCtrlParam.eCmdId, &tGetCtrlParam.tParam);

            if (eRet == E_VDEC_EX_OK)
            {
                if(copy_to_user((IOCTL_PARAM_VDEC_GET_CONTROL_t __user *)arg,
                    &tGetCtrlParam, sizeof(IOCTL_PARAM_VDEC_GET_CONTROL_t)))
                {
                    return -EFAULT;
                }
            }
            break;

        case IOCTL_CMD_VDEC_EX_DOMMAP:
            if(copy_from_user(&tMapInfo,
                               (IOCTL_PARAM_VDEC_MAPINFO_t __user *)arg,
                               sizeof(IOCTL_PARAM_VDEC_MAPINFO_t)) )
            {
                return -EFAULT;
            }
            eRet = mdrv_MApi_VDEC_EX_DoMmap((VDEC_MAPINFO_t *)&tMapInfo);
            break;

        case IOCTL_CMD_VDEC_EX_DOUNMAP:
             if(copy_from_user(&tUnMapInfo,
                                (IOCTL_PARAM_VDEC_UNMAPINFO_t __user *)arg,
                                sizeof(IOCTL_PARAM_VDEC_UNMAPINFO_t)) )
             {
                 CamOsDebug("IOCTL_CMD_VDEC_EX_DOUNMAP: copy_from_user failed !\n");
                 return -EFAULT;
             }
             eRet = mdrv_MApi_VDEC_EX_DoUnMap(&tUnMapInfo.tInfo);
             break;

        case IOCTL_CMD_VDEC_JPDSetVerificationMode:
              if(copy_from_user(&tJpdVerif,
                                 (IOCTL_PARAM_VDEC_JPDSetVerificationMode_t __user *)arg,
                                 sizeof(IOCTL_PARAM_VDEC_JPDSetVerificationMode_t)) )
              {
                  CamOsDebug("IOCTL_CMD_VDEC_EX_DOUNMAP: copy_from_user failed !\n");
                  return -EFAULT;
              }
              MApi_JPEG_SetVerificationMode(tJpdVerif.eVerificationMode);
              break;

         default:
            return -EFAULT; // UTOPIA_STATUS_NOT_SUPPORTED;
    }

    return CONV_VDEC_RET(eRet);
}

//#ifndef CAM_OS_RTK //CAM_OS_LINUX_KERNEL
// Kernel interface
static struct file_operations mdrv_vdec_fops = {
    .owner   =   THIS_MODULE,
    .open    = mdrv_vdec_open,
    .release = mdrv_vdec_release,
    .unlocked_ioctl = mdrv_vdec_ioctl,
};


#define CHRDEV_NAME "mdrv_vdec"
static struct class *chrdev_class = NULL;
static struct device *chrdev_device = NULL;
static dev_t chrdev_devno;
static struct cdev chrdev_cdev;

static __init int mdrv_vdec_init(void)
{
    int ret = 0, err = 0;

    printk(KERN_ALERT "mdrv_vdec_init!\n");

    // alloc character device number
    ret = alloc_chrdev_region(&chrdev_devno, 0, 1, CHRDEV_NAME);
    if (ret) {
        printk(KERN_ALERT " alloc_chrdev_region failed!\n");
        goto PROBE_ERR;
    }
    printk(KERN_ALERT " major:%d minor:%d\n", MAJOR(chrdev_devno), MINOR(chrdev_devno));

    cdev_init(&chrdev_cdev, &mdrv_vdec_fops);
    chrdev_cdev.owner = THIS_MODULE;
    // add a character device
    err = cdev_add(&chrdev_cdev, chrdev_devno, 1);
    if (err) {
        printk(KERN_ALERT " cdev_add failed!\n");
        goto PROBE_ERR;
    }

    // create the device class
    chrdev_class = class_create(THIS_MODULE, CHRDEV_NAME);
    if (IS_ERR(chrdev_class)) {
        printk(KERN_ALERT " class_create failed!\n");
        goto PROBE_ERR;
    }

    // create the device node in /dev
    chrdev_device = device_create(chrdev_class, NULL, chrdev_devno,
        NULL, CHRDEV_NAME);
    if (NULL == chrdev_device) {
        printk(KERN_ALERT " device_create failed!\n");
        goto PROBE_ERR;
    }

    printk(KERN_ALERT " mdrv_vdec_init ok!\n");
    return 0;

PROBE_ERR:
    if (err)
        cdev_del(&chrdev_cdev);
    if (ret)
        unregister_chrdev_region(chrdev_devno, 1);
    return -1;
}

static __exit void mdrv_vdec_exit(void)
{
    printk(KERN_ALERT " mdrv_vdec_exit!\n");

    cdev_del(&chrdev_cdev);
    unregister_chrdev_region(chrdev_devno, 1);

    device_destroy(chrdev_class, chrdev_devno);
    class_destroy(chrdev_class);
    //return 0;
}

module_init(mdrv_vdec_init);
module_exit(mdrv_vdec_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("ms vdec driver");
MODULE_LICENSE("GPL");

//#endif
