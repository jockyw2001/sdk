
#include "mdrv_mhe_io.h"
#include "mdrv_rqct_io.h"
#include "drv_mhe_kernel.h"
#include "drv_mhe_ctx.h"
#include "drv_mhe_dev.h"
#include "mhal_mhe.h"
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
} MheLinuxDev_t;

MheLinuxDev_t gDevice = {0};

#ifdef SUPPORT_CMDQ_SERVICE
MHAL_CMDQ_CmdqInterface_t *stCmdQInf;
#endif

static unsigned int mhal_mode = 1;
module_param(mhal_mode, uint, S_IRUGO|S_IWUSR);

static unsigned int core_id = 0;
module_param(core_id, uint, S_IRUGO|S_IWUSR);

unsigned int clock_mode = 0; //0: 576Mhz, 1:480Mhz
module_param(clock_mode, uint, S_IRUGO|S_IWUSR);

#ifdef MODULE_PARAM_SUPPORT
unsigned int tmvp_enb = 0;
module_param(tmvp_enb, uint, S_IRUGO | S_IWUSR);

unsigned int fdc_dbg = 0;
module_param(fdc_dbg, uint, S_IRUGO | S_IWUSR);
#endif

static int  MheOpen(struct inode*, struct file*);
static int  MheRelease(struct inode*, struct file*);
static long MheIoctl(struct file*, unsigned int, unsigned long);

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
        case E_MHAL_VENC_265_RESOLUTION:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_Resoluton_t));
            break;
        case E_MHAL_VENC_265_CROP:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_CropCfg_t));
            break;
        case E_MHAL_VENC_265_REF:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamRef_t));
            break;
        case E_MHAL_VENC_265_VUI:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Vui_t));
            break;
        case E_MHAL_VENC_265_DBLK:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Dblk_t));
            break;
        case E_MHAL_VENC_265_ENTROPY:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Entropy_t));
            break;
        case E_MHAL_VENC_265_TRANS:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264Trans_t));
            break;
        case E_MHAL_VENC_265_INTRA_PRED:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264IntraPred_t));
            break;
        case E_MHAL_VENC_265_INTER_PRED:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamH264InterPred_t));
            break;
        case E_MHAL_VENC_265_I_SPLIT_CTL:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamSplit_t));
            break;

        case E_MHAL_VENC_265_ROI:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_RoiCfg_t));
            break;
        case E_MHAL_VENC_265_RC:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_RcInfo_t));
            break;
        case E_MHAL_VENC_265_FRAME_LOST:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_ParamFrameLost_t));
            break;
        case E_MHAL_VENC_265_FRAME_CFG:
            CamOsCopyFromUpperLayer(VencBuf, param, sizeof(MHAL_VENC_SuperFrameCfg_t));
            break;
        case E_MHAL_VENC_265_RC_PRIORITY:
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
        case E_MHAL_VENC_265_RESOLUTION:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_Resoluton_t));
            break;
        case E_MHAL_VENC_265_CROP:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_CropCfg_t));
            break;
        case E_MHAL_VENC_265_REF:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamRef_t));
            break;
        case E_MHAL_VENC_265_VUI:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Vui_t));
            break;
        case E_MHAL_VENC_265_DBLK:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Dblk_t));
            break;
        case E_MHAL_VENC_265_ENTROPY:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Entropy_t));
            break;
        case E_MHAL_VENC_265_TRANS:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264Trans_t));
            break;
        case E_MHAL_VENC_265_INTRA_PRED:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264IntraPred_t));
            break;
        case E_MHAL_VENC_265_INTER_PRED:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamH264InterPred_t));
            break;
        case E_MHAL_VENC_265_I_SPLIT_CTL:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamSplit_t));
            break;

        case E_MHAL_VENC_265_ROI:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RoiCfg_t));
            break;
        case E_MHAL_VENC_265_RC:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RcInfo_t));
            break;
        case E_MHAL_VENC_265_FRAME_LOST:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_ParamFrameLost_t));
            break;
        case E_MHAL_VENC_265_FRAME_CFG:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_SuperFrameCfg_t));
            break;
        case E_MHAL_VENC_265_RC_PRIORITY:
            CamOsCopyToUpperLayer(param, VencBuf, sizeof(MHAL_VENC_RcPriorityCfg_t));
            break;
        default:
            break;
    }

    return 0;
}

static irqreturn_t _MheIsr(int irq, void* priv)
{
    MheLinuxDev_t* device = (MheLinuxDev_t*)priv;

    if(!MheDevIsrFnx(device->mdev))
        return IRQ_HANDLED;
    return IRQ_HANDLED;
}


static struct file_operations MheFops =
{
    .owner = THIS_MODULE,
    .open = MheOpen,
    .release = MheRelease,
    .unlocked_ioctl = MheIoctl,
};

static int
MheOpen(
    struct inode*   inode,
    struct file*    file)
{
    MheLinuxDev_t* device = container_of(inode->i_cdev, MheLinuxDev_t, m_cdev);

    if((void *)MHAL_MHE_CreateInstance(device->mdev, &file->private_data))
        return -EINVAL;

    if(!file->private_data)
        return -EINVAL;

    return 0;
}

static int
MheRelease(
    struct inode*   inode,
    struct file*    file)
{
    MHAL_MHE_DestroyInstance(file->private_data);
    file->private_data = NULL;
    return 0;
}

static long
MheIoctl(
    struct file*    file,
    unsigned int    cmd,
    unsigned long   arg)
{
    mmhe_ctx* mctx = file->private_data;
    size_t n = _IOC_SIZE(cmd);
    char buf[512];
    long err = -EFAULT;
    void __user *uptr = (void __user *)arg;
    unsigned int v = MMHEIF_VERSION_ID;
    int type = _IOC_TYPE(cmd);


    if(type != MAGIC_RQCT && type != MAGIC_MMHE)
        return -EINVAL;

    switch(cmd)
    {
        case IOCTL_RQCT_VERSION:
            v = RQCTIF_VERSION_ID;
        case IOCTL_MMHE_VERSION:
            if(access_ok(ACCESS_WRITE, uptr, sizeof(unsigned int)))
                err = __put_user(v, (unsigned int*)uptr);
            break;
        default:
            if((_IOC_DIR(cmd) & _IOC_WRITE) && CamOsCopyFromUpperLayer(buf, uptr, n))
                break;
            if(cmd == IOCTL_MMHE_MHAL_SET_PARAM)
            {
                char Vencbuf[512];

                VencParamUT *pVencParamUT = (VencParamUT *)buf;

                memset(Vencbuf, 0, sizeof(Vencbuf));

                _VencParamCopyFromUser(pVencParamUT->type, (MHAL_VENC_Param_t*)pVencParamUT->param, Vencbuf);

                err = MHAL_MHE_SetParam((MHAL_VENC_INST_HANDLE)mctx, pVencParamUT->type, (MHAL_VENC_Param_t*) Vencbuf);
            }
            else if(cmd == IOCTL_MMHE_MHAL_GET_PARAM)
            {
                char Vencbuf[512];

                VencParamUT *pVencParamUT = (VencParamUT *)buf;

                memset(Vencbuf, 0, sizeof(Vencbuf));

                _VencParamCopyFromUser(pVencParamUT->type, (MHAL_VENC_Param_t*)pVencParamUT->param, Vencbuf);

                err = MHAL_MHE_GetParam((MHAL_VENC_INST_HANDLE)mctx, pVencParamUT->type, (MHAL_VENC_Param_t*)Vencbuf);

                _VencParamCopyToUser(pVencParamUT->type, (MHAL_VENC_Param_t*)pVencParamUT->param, Vencbuf);

            }
            else if(cmd == IOCTL_MMHE_MHAL_ENCODE_ONE_FRAME)
            {
#ifdef SUPPORT_CMDQ_SERVICE
                MHAL_VENC_InOutBuf_t *stVencInOut = (MHAL_VENC_InOutBuf_t *)buf;
                if(stVencInOut->pCmdQ)
                    stVencInOut->pCmdQ = stCmdQInf;
#endif
                err = MHAL_MHE_EncodeOneFrame((void *)mctx, (MHAL_VENC_InOutBuf_t *)buf);
            }
            else if(cmd == IOCTL_MMHE_MHAL_ENCODE_FRAME_DONE)
            {
                err = MHAL_MHE_EncodeFrameDone((void *)mctx, (MHAL_VENC_EncResult_t *)buf);
            }
            else if(cmd == IOCTL_MMHE_MHAL_QUERY_BUFSIZE)
            {
                err = MHAL_MHE_QueryBufSize((MHAL_VENC_INST_HANDLE)mctx, (MHAL_VENC_InternalBuf_t *)buf);
            }
            else if(cmd == IOCTL_MMHE_MHAL_KICKOFF_CMDQ)
            {
#ifdef SUPPORT_CMDQ_SERVICE
                stCmdQInf->MHAL_CMDQ_KickOffCmdq(stCmdQInf);
                err = 0;
#else
                err = 0;
#endif
            }
            else
            {
                if(0 > (err = MheCtxActions(mctx, cmd, buf)))
                    break;
            }
            if((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
                break;

            //err = 0;
            break;
    }

    return err;
}

static void MheDevRelease(struct device* dev) {}

void _MheSetHeader(VOID* header, MS_U32 size);

static int
MheProbe(
    struct platform_device* pdev)
{
    int err = 0;
    dev_t dev;
    int major, minor = 0;
#ifdef SUPPORT_CMDQ_SERVICE
    MHAL_CMDQ_BufDescript_t stCmdqBufDesp;
#endif
    MHAL_VENC_ParamInt_t param;

    CamOsPrintf("MHE ko Probe :  %s - %s\n", __DATE__, __TIME__);

    do
    {
        if(MHAL_MHE_CreateDevice(core_id, &gDevice.mdev))
            break;

        if(0 > (err = alloc_chrdev_region(&dev, minor, MMHE_STREAM_NR + 1, "mstar_mmhe")))
            break;
        major = MAJOR(dev);
        gDevice.i_major = major;
        gDevice.i_minor = minor;
        cdev_init(&gDevice.m_cdev, &MheFops);
        gDevice.m_cdev.owner = THIS_MODULE;
        if(0 > (err = cdev_add(&gDevice.m_cdev, MKDEV(major, minor), 1)))
            break;
        gDevice.m_dev.devt = MKDEV(major, minor);
        gDevice.m_dev.class = msys_get_sysfs_class();
        gDevice.m_dev.parent = NULL;
        gDevice.m_dev.release = MheDevRelease;
        dev_set_name(&gDevice.m_dev, "%s", "mmhe");
        if(0 > (err = device_register(&gDevice.m_dev)))
            break;

        _MheSetHeader(&param, sizeof(param));
        MHAL_MHE_GetDevConfig(gDevice.mdev, E_MHAL_VENC_HW_IRQ_NUM, &param);

        gDevice.i_irq = param.u32Val;

        if(0 != (err = request_irq(gDevice.i_irq, _MheIsr, IRQF_SHARED, "_MheIsr", &gDevice)))
            break;

        err = -EINVAL;

#ifdef SUPPORT_CMDQ_SERVICE
        _MheSetHeader(&param, sizeof(param));
        MHAL_MHE_GetDevConfig(gDevice.mdev, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &param);

        stCmdqBufDesp.u32CmdqBufSize = param.u32Val;
        stCmdqBufDesp.u32CmdqBufSizeAlign = 16;
        stCmdqBufDesp.u32MloadBufSize = 0;
        stCmdqBufDesp.u16MloadBufSizeAlign = 16;
        stCmdQInf = MHAL_CMDQ_GetSysCmdqService(0, &stCmdqBufDesp, FALSE);
        CamOsPrintf("Call MHAL_CMDQ_GetSysCmdqService: 0x%08X 0x%08X\n", stCmdQInf, stCmdQInf->MHAL_CMDQ_CheckBufAvailable);
#endif

#ifdef MODULE_PARAM_SUPPORT
        if(tmvp_enb > 1)
            CamOsPrintf("MHE tmvp_enb setting error !!\n");
        else
            CamOsPrintf("[%s %d] MHE tmvp_enb  = %d !!\n", __FUNCTION__, __LINE__, tmvp_enb);

        if(fdc_dbg > 2)
            CamOsPrintf("MHE fdc_dbg setting error !!\n");
        else
            CamOsPrintf("[%s %d] MHE fdc_dbg  = %d !!\n", __FUNCTION__, __LINE__, fdc_dbg);
#endif

        return 0;
    }
    while(0);

    cdev_del(&gDevice.m_cdev);
    MHAL_MHE_DestroyDevice(gDevice.mdev);

    CamOsPrintf("MHE ko Probe Fail\n");

    return err;
}

static int
MheRemove(
    struct platform_device* pdev)
{
    int devno = MKDEV(gDevice.i_major, 0);

    CamOsPrintf("MHE ko Remove:  %s - %s\n", __DATE__, __TIME__);

    free_irq(gDevice.i_irq, &gDevice);

    cdev_del(&gDevice.m_cdev);

    device_unregister(&gDevice.m_dev);

    MHAL_MHE_DestroyDevice(gDevice.mdev);

    unregister_chrdev_region(devno, MMHE_STREAM_NR + 1);

#ifdef SUPPORT_CMDQ_SERVICE
    CamOsPrintf("Call MHAL_CMDQ_ReleaseSysCmdqService\n");
    MHAL_CMDQ_ReleaseSysCmdqService(0);
#endif

    return 0;
}

static const struct of_device_id MheMatchTables[] =
{
    { .compatible = "mstar,mhe" },
    {},
};

static struct platform_driver MhePdrv =
{
    .probe = MheProbe,
    .remove = MheRemove,
    .driver = {
        .name = "mmhe",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(MheMatchTables),
    }
};

static int  __init mmhe_init(void)
{
    if (!mhal_mode)
        return platform_driver_register(&MhePdrv);
    else
        return 0;
}

static void __exit mmhe_exit(void)
{
    if (!mhal_mode)
        platform_driver_unregister(&MhePdrv);
}

module_init(mmhe_init);
module_exit(mmhe_exit);
