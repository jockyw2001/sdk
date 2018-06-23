
#include "mdrv_mfe_io.h"
#include "mdrv_rqct_io.h"
#include "drv_mfe_kernel.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"
#include "mhal_mfe.h"
#include "mhal_ut_wrapper.h"
#ifdef SUPPORT_CMDQ_SERVICE
#include "mhal_cmdq.h"
#endif
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include "ms_msys.h"

MODULE_LICENSE("GPL");

typedef struct
{
#if defined(__KERNEL__) || defined(CAM_OS_LINUX_USER)
    int             i_major;
    int             i_minor;
    struct cdev     m_cdev; // character device
    struct device   m_dev;  // device node
    struct device*  p_dev;  // platform device
    int             i_irq;
#endif
    void*       mdev;
} MfeLinuxDev_t;

MfeLinuxDev_t gDevice = {0};

#ifdef SUPPORT_CMDQ_SERVICE
MHAL_CMDQ_CmdqInterface_t *stCmdQInf;
#endif

static unsigned int mhal_mode = 1;
module_param(mhal_mode, uint, S_IRUGO|S_IWUSR);

static int  MfeOpen(struct inode*, struct file*);
static int  MfeRelease(struct inode*, struct file*);
static long MfeIoctl(struct file*, unsigned int, unsigned long);

int _VencParamCopyFromUser(MHAL_VENC_IDX type, MHAL_VENC_Param_t* param, char * VencBuf)
{
    MHAL_VENC_InternalBuf_t* pInternalBuf;

    switch(type)
    {
        case E_MHAL_VENC_IDX_STREAM_ON:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_InternalBuf_t));
            pInternalBuf = (MHAL_VENC_InternalBuf_t*)VencBuf;

            // Replace virtual address that assign from user space.
            pInternalBuf->pu8IntrAlVirBuf = CamOsDirectMemPhysToVirt((void *)(uintptr_t)pInternalBuf->phyIntrAlPhyBuf);
            //CamOsPrintf("pInternalBuf->IntrAlVirBuf: 0x%08X\n", pInternalBuf->IntrAlVirBuf);
            break;
        case E_MHAL_VENC_264_RESOLUTION:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_Resoluton_t));
            break;
        case E_MHAL_VENC_264_CROP:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_CropCfg_t));
            break;
        case E_MHAL_VENC_264_REF:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamRef_t));
            break;
        case E_MHAL_VENC_264_VUI:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Vui_t));
            break;
        case E_MHAL_VENC_264_DBLK:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Dblk_t));
            break;
        case E_MHAL_VENC_264_ENTROPY:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Entropy_t));
            break;
        case E_MHAL_VENC_264_TRANS:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Trans_t));
            break;
        case E_MHAL_VENC_264_INTRA_PRED:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264IntraPred_t));
            break;
        case E_MHAL_VENC_264_INTER_PRED:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264InterPred_t));
            break;
        case E_MHAL_VENC_264_I_SPLIT_CTL:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamSplit_t));
            break;
        case E_MHAL_VENC_264_ROI:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_RoiCfg_t));
            break;
        case E_MHAL_VENC_264_RC:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_RcInfo_t));
            break;
        case E_MHAL_VENC_264_USER_DATA:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_UserData_t));
            break;
        case E_MHAL_VENC_264_FRAME_LOST:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamFrameLost_t));
            break;
        case E_MHAL_VENC_264_FRAME_CFG:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_SuperFrameCfg_t));
            break;
        case E_MHAL_VENC_264_RC_PRIORITY:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_RcPriorityCfg_t));
            break;
        default:
            break;
    }

    return 0;
}

int _VencParamCopyToUser(MHAL_VENC_IDX type, MHAL_VENC_Param_t* param, char * VencBuf)
{
    switch(type)
    {
        case E_MHAL_VENC_IDX_STREAM_ON:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_InternalBuf_t));
            break;
        case E_MHAL_VENC_264_RESOLUTION:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_Resoluton_t));
            break;
        case E_MHAL_VENC_264_CROP:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_CropCfg_t));
            break;
        case E_MHAL_VENC_264_REF:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamRef_t));
            break;
        case E_MHAL_VENC_264_VUI:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Vui_t));
            break;
        case E_MHAL_VENC_264_DBLK:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Dblk_t));
            break;
        case E_MHAL_VENC_264_ENTROPY:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Entropy_t));
            break;
        case E_MHAL_VENC_264_TRANS:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Trans_t));
            break;
        case E_MHAL_VENC_264_INTRA_PRED:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264IntraPred_t));
            break;
        case E_MHAL_VENC_264_INTER_PRED:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264InterPred_t));
            break;
        case E_MHAL_VENC_264_I_SPLIT_CTL:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamSplit_t));
            break;
        case E_MHAL_VENC_264_ROI:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RoiCfg_t));
            break;
        case E_MHAL_VENC_264_RC:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RcInfo_t));
            break;
        case E_MHAL_VENC_264_FRAME_LOST:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamFrameLost_t));
            break;
        case E_MHAL_VENC_264_FRAME_CFG:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_SuperFrameCfg_t));
            break;
        case E_MHAL_VENC_264_RC_PRIORITY:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RcPriorityCfg_t));
            break;
        default:
            break;
    }

    return 0;
}

static irqreturn_t _MfeIsr(int irq, void* priv)
{
    MfeLinuxDev_t* device = (MfeLinuxDev_t*)priv;

    if (!MfeDevIsrFnx(device->mdev))
        return IRQ_HANDLED;
    return IRQ_HANDLED;
}

static struct file_operations MfeFops = {
    .owner = THIS_MODULE,
    .open = MfeOpen,
    .release = MfeRelease,
    .unlocked_ioctl = MfeIoctl,
};

static int
MfeOpen(
    struct inode*   inode,
    struct file*    file)
{
    MfeLinuxDev_t* device = container_of(inode->i_cdev,MfeLinuxDev_t,m_cdev);

    if ((void *)MHAL_MFE_CreateInstance(device->mdev, &file->private_data))
        return -EINVAL;

    if (!file->private_data)
        return -EINVAL;

    return 0;
}

static int
MfeRelease(
    struct inode*   inode,
    struct file*    file)
{
    MHAL_MFE_DestroyInstance(file->private_data);
    file->private_data = NULL;
    return 0;
}

static long
MfeIoctl(
    struct file*    file,
    unsigned int    cmd,
    unsigned long   arg)
{
    mmfe_ctx* mctx = file->private_data;
    size_t n = _IOC_SIZE(cmd);
    char buf[512];
    long err = -EFAULT;
    void __user *uptr = (void __user *)arg;
    unsigned int v = MMFEIF_VERSION_ID;
    int type = _IOC_TYPE(cmd);

    if (type != MAGIC_RQCT && type != MAGIC_MMFE)
        return -EINVAL;
    switch (cmd)
    {
    case IOCTL_RQCT_VERSION:
        v = RQCTIF_VERSION_ID;
    case IOCTL_MMFE_VERSION:
        if (access_ok(ACCESS_WRITE,uptr,sizeof(unsigned int)))
            err = __put_user(v,(unsigned int*)uptr);
        break;
    default:
        if ((_IOC_DIR(cmd) & _IOC_WRITE) && CamOsCopyFromUpperLayer(buf, uptr, n))
            break;
        if (cmd == IOCTL_MMFE_MHAL_SET_PARAM)
        {
            char Vencbuf[512];

            VencParamUT *pVencParamUT= (VencParamUT *)buf;

            memset(Vencbuf,0,sizeof(Vencbuf));

            _VencParamCopyFromUser(pVencParamUT->type, (MHAL_VENC_Param_t* )pVencParamUT->param, Vencbuf);

            MHAL_MFE_SetParam((MHAL_VENC_INST_HANDLE)mctx, pVencParamUT->type,(MHAL_VENC_Param_t* ) Vencbuf);
        }
        else if (cmd == IOCTL_MMFE_MHAL_GET_PARAM)
        {
            char Vencbuf[512];

            VencParamUT *pVencParamUT= (VencParamUT *)buf;

            memset(Vencbuf,0,sizeof(Vencbuf));

            _VencParamCopyFromUser(pVencParamUT->type, (MHAL_VENC_Param_t* )pVencParamUT->param, Vencbuf);

            MHAL_MFE_GetParam((MHAL_VENC_INST_HANDLE)mctx, pVencParamUT->type, (MHAL_VENC_Param_t* )Vencbuf);

            _VencParamCopyToUser(pVencParamUT->type, (MHAL_VENC_Param_t* )pVencParamUT->param, Vencbuf);

        }
        else if (cmd == IOCTL_MMFE_MHAL_ENCODE_ONE_FRAME)
        {
#ifdef SUPPORT_CMDQ_SERVICE
            MHAL_VENC_InOutBuf_t *stVencInOut = (MHAL_VENC_InOutBuf_t *)buf;
            if (stVencInOut->pCmdQ)
                stVencInOut->pCmdQ = stCmdQInf;
#endif
            MHAL_MFE_EncodeOneFrame((void *)mctx, (MHAL_VENC_InOutBuf_t *)buf);
        }
        else if (cmd == IOCTL_MMFE_MHAL_ENCODE_FRAME_DONE)
        {
            MHAL_MFE_EncodeFrameDone((void *)mctx, (MHAL_VENC_EncResult_t *)buf);
        }
        else if (cmd == IOCTL_MMFE_MHAL_QUERY_BUFSIZE)
        {
            MHAL_MFE_QueryBufSize((MHAL_VENC_INST_HANDLE)mctx, (MHAL_VENC_InternalBuf_t *)buf);
        }
        else if (cmd == IOCTL_MMFE_MHAL_KICKOFF_CMDQ)
        {
#ifdef SUPPORT_CMDQ_SERVICE
            stCmdQInf->MHAL_CMDQ_KickOffCmdq(stCmdQInf);
#endif
        }
        else
        {
            if (0 > (err = MfeCtxActions(mctx, cmd, buf)))
                break;
        }
        if ((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
            break;
        err = 0;
        break;
    }

    return err;
}

static void MfeDevRelease(struct device* dev) {}

void _MfeSetHeader(VOID* header, MS_U32 size);

static int
MfeProbe(
    struct platform_device* pdev)
{
    int err = 0;
    dev_t dev;
    int major, minor = 0;
#ifdef SUPPORT_CMDQ_SERVICE
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
#endif
    MHAL_VENC_ParamInt_t param;

    CamOsPrintf("MFE ko Probe :  %s - %s\n", __DATE__, __TIME__);

    do
    {
        if (MHAL_MFE_CreateDevice(0, &gDevice.mdev))
            break;

        if (0 > (err = alloc_chrdev_region(&dev, minor, MMFE_STREAM_NR+1, "mstar_mmfe")))
            break;
        major = MAJOR(dev);
        gDevice.i_major = major;
        gDevice.i_minor = minor;
        cdev_init(&gDevice.m_cdev, &MfeFops);
        gDevice.m_cdev.owner = THIS_MODULE;
        if (0 > (err = cdev_add(&gDevice.m_cdev, MKDEV(major,minor), 1)))
            break;
        gDevice.m_dev.devt = MKDEV(major,minor);
        gDevice.m_dev.class = msys_get_sysfs_class();
        gDevice.m_dev.parent = NULL;
        gDevice.m_dev.release = MfeDevRelease;
        dev_set_name(&gDevice.m_dev, "%s", "mmfe");
        if (0 > (err = device_register(&gDevice.m_dev)))
            break;

        _MfeSetHeader(&param, sizeof(param));
        MHAL_MFE_GetDevConfig(gDevice.mdev, E_MHAL_VENC_HW_IRQ_NUM, &param);

        gDevice.i_irq = param.u32Val;
        if (0 != (err = request_irq(gDevice.i_irq, _MfeIsr, IRQF_SHARED, "_MfeIsr", &gDevice)))
            break;

        err = -EINVAL;

#ifdef SUPPORT_CMDQ_SERVICE
        _MfeSetHeader(&param, sizeof(param));
        MHAL_MFE_GetDevConfig(gDevice.mdev, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param);

        stCmdqBufDesp.u32CmdqBufSize = param.u32Val;
        stCmdqBufDesp.u32CmdqBufSizeAlign = 16;
        stCmdqBufDesp.u32MloadBufSize = 0;
        stCmdqBufDesp.u16MloadBufSizeAlign = 16;
        stCmdQInf = MHAL_CMDQ_GetSysCmdqService(0, &stCmdqBufDesp, FALSE);
        CamOsPrintf("Call MHAL_CMDQ_GetSysCmdqService: 0x%08X 0x%08X\n", stCmdQInf, stCmdQInf->MHAL_CMDQ_CheckBufAvailable);
#endif

        return 0;
    }
    while (0);

    cdev_del(&gDevice.m_cdev);
    MHAL_MFE_DestroyDevice(gDevice.mdev);

    CamOsPrintf("MFE ko Probe Fail\n");

    return err;
}

static int
MfeRemove(
    struct platform_device* pdev)
{
    int devno = MKDEV(gDevice.i_major,0);

    CamOsPrintf("MFE ko Remove:  %s - %s\n", __DATE__, __TIME__);

    free_irq(gDevice.i_irq, &gDevice);

    cdev_del(&gDevice.m_cdev);

    device_unregister(&gDevice.m_dev);

    MHAL_MFE_DestroyDevice(gDevice.mdev);

    unregister_chrdev_region(devno, MMFE_STREAM_NR+1);

#ifdef SUPPORT_CMDQ_SERVICE
    CamOsPrintf("Call MHAL_CMDQ_ReleaseSysCmdqService\n");
    MHAL_CMDQ_ReleaseSysCmdqService(0);
#endif

    return 0;
}

static const struct of_device_id MfeMatchTables[] = {
    { .compatible = "mstar,mfe" },
    {},
};

static struct platform_driver MfePdrv = {
    .probe = MfeProbe,
    .remove = MfeRemove,
    .driver = {
        .name = "mmfe",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(MfeMatchTables),
    }
};

static int  __init mmfe_init(void)
{
    if (!mhal_mode)
        return platform_driver_register(&MfePdrv);
    else
        return 0;
}

static void __exit mmfe_exit(void)
{
    if (!mhal_mode)
        platform_driver_unregister(&MfePdrv);
}

module_init(mmfe_init);
module_exit(mmfe_exit);
