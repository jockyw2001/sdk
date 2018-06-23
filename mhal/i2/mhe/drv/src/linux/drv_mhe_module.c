
#include "mdrv_mhe_io.h"
#include "mdrv_rqct_io.h"
#include "drv_mhe_kernel.h"
#include "drv_mhe_ctx.h"
#include "drv_mhe_dev.h"
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int  MheOpen(struct inode*, struct file*);
static int  MheRelease(struct inode*, struct file*);
static long MheIoctl(struct file*, unsigned int, unsigned long);

static irqreturn_t _MheIsr(int irq, void* priv)
{
    mmhe_dev* mdev = (mmhe_dev*)priv;

    if(!MheDevIsrFnx(mdev))
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
    mmhe_dev* mdev = container_of(inode->i_cdev, mmhe_dev, m_cdev);
    mmhe_ctx* mctx;
    if(!(mctx = MheCtxAcquire(mdev)))
        return -ENOMEM;
    if(0 <= MheDevRegister(mdev, mctx))
    {
        file->private_data = mctx;
        return 0;
    }
    mctx->release(mctx);
    return -EINVAL;
}

static int
MheRelease(
    struct inode*   inode,
    struct file*    file)
{
    mmhe_dev* mdev = container_of(inode->i_cdev, mmhe_dev, m_cdev);
    mmhe_ctx* mctx = file->private_data;
    MheDevUnregister(mdev, mctx);
    file->private_data = NULL;
    mctx->release(mctx);
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
    char buf[256];
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
            if(0 > (err = MheCtxActions(mctx, cmd, buf)))
                break;
            if((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
                break;
            err = 0;
            break;
    }

    return err;
}

static int MrqcOpen(struct inode*, struct file*);
static int MrqcRelease(struct inode*, struct file*);
static long MrqcIoctl(struct file*, unsigned int, unsigned long);
static ssize_t MrqcRead(struct file*, char __user*, size_t, loff_t*);

static struct file_operations MrqcFops =
{
    .owner = THIS_MODULE,
    .open = MrqcOpen,
    .release = MrqcRelease,
    .unlocked_ioctl = MrqcIoctl,
    .read = MrqcRead,
};

static int
MrqcOpen(
    struct inode*   inode,
    struct file*    file)
{
    int err = -EINVAL;
    mmhe_rqc* mrqc = container_of(inode->i_cdev, mmhe_rqc, m_cdev);
    mmhe_ctx* mctx = NULL;
    CamOsMutexLock(mrqc->p_mutex);
    if(NULL != (mctx = mrqc->p_mctx))
    {
        mctx->adduser(mctx);
        file->private_data = mctx;
        err = 0;
    }
    CamOsMutexUnlock(mrqc->p_mutex);
    return err;
}

static int
MrqcRelease(
    struct inode*   inode,
    struct file*    file)
{
    mmhe_ctx* mctx = file->private_data;
    mctx->release(mctx);
    return 0;
}

static long
MrqcIoctl(
    struct file*    file,
    unsigned int    cmd,
    unsigned long   arg)
{
    size_t n = _IOC_SIZE(cmd);
    char buf[256];
    long err = -EFAULT;
    void __user *uptr = (void __user *)arg;
    void* mctx = file->private_data;
    unsigned int v = RQCTIF_VERSION_ID;

    if(_IOC_TYPE(cmd) != MAGIC_RQCT)
        return -EINVAL;
    switch(cmd)
    {
        case IOCTL_RQCT_VERSION:
            if(access_ok(ACCESS_WRITE, uptr, sizeof(unsigned int)))
                err = __put_user(v, (unsigned int*)uptr);
            break;
        default:
            if((_IOC_DIR(cmd) & _IOC_WRITE) && CamOsCopyFromUpperLayer(buf, uptr, n))
                break;
            if(0 > (err = MheRqcIoctls(mctx, cmd, buf)))
                break;
            if((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
                break;
            err = 0;
            break;
    }

    return err;
}

static ssize_t
MrqcRead(
    struct file*    file,
    char __user*    buff,
    size_t          size,
    loff_t*         fpos)
{
    char line[RQCT_LINE_SIZE];
    long err = -EFAULT;
    void __user *uptr = (void __user*)buff;
    void* mctx = file->private_data;
    size_t lsize = MheRqcPrint(mctx, line, RQCT_LINE_SIZE);
    if(size > lsize)
        size = lsize;
    if(size > 0 && CamOsCopyToUpperLayer(uptr, line, size))
        return (ssize_t)err;
    return (ssize_t)size;
}

static void MheDevRelease(struct device* dev) {}

static ssize_t MheTmrStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    int i, thresh = 0;
    if(0 != kstrtol(buff, 10, (long int*)&thresh))
        return -EINVAL;
    mdev->i_thresh = thresh;
    for(i = 0; i < MMHE_STREAM_NR; i++)
        mdev->i_counts[i][0] = mdev->i_counts[i][1] = mdev->i_counts[i][2] = mdev->i_counts[i][3] = mdev->i_counts[i][4] = 0;
    return n;
}

static ssize_t MheTmrPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    int i;
    for(i = 0; i < MMHE_STREAM_NR; i++)
        str += scnprintf(str, end - str, "inst-%d:%8d/%5d/%5d/%8d/%8d\n", i, mdev->i_counts[i][0], mdev->i_counts[i][1], mdev->i_counts[i][2], mdev->i_counts[i][3], mdev->i_counts[i][4]);
    str += scnprintf(str, end - str, "thresh:%8d\n", mdev->i_thresh);
    return (str - buff);
}

static DEVICE_ATTR(tmr, 0644, MheTmrPrint, MheTmrStore);

static ssize_t MheClkStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    struct clk* clock = mdev->p_clocks[0];
    struct clk* ck = NULL;
    int id = 0;
    if(0 != kstrtol(buff, 10, (long int*)&id))
        return -EINVAL;
    ck = clk_get_parent_by_index(clock, id);
    if(ck == NULL)
        mdev->i_ratehz = mdev->i_clkidx = -1;
    else
    {
        mdev->i_clkidx = id;
        mdev->i_ratehz = clk_get_rate(ck);
    }
    return n;
}

static ssize_t MheClkPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    if(mdev->i_clkidx < 0)
        str += scnprintf(str, end - str, "clkidx:%3d(dynamic)\n", mdev->i_clkidx);
    else
        str += scnprintf(str, end - str, "clkidx:%3d(%3d mhz)\n", mdev->i_clkidx, mdev->i_ratehz / 1000000);
    return (str - buff);
}

static DEVICE_ATTR(clk, 0644, MheClkPrint, MheClkStore);

char* RqctMheComment(int);

static ssize_t MheRctStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    int id = 0;
    char* brief;
    if(0 != kstrtol(buff, 10, (long int*)&id))
        return -EINVAL;
    if((brief = RqctMheComment(id)) && *brief)
        mdev->i_rctidx = id;
    return n;
}

static ssize_t MheRctPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmhe_dev* mdev = container_of(dev, mmhe_dev, m_dev);
    char* brief;
    int j;
    for(j = 0; (brief = RqctMheComment(j)); j++)
        if(*brief)
            str += scnprintf(str, end - str, "(%c)[%d]%s\n", j == mdev->i_rctidx ? '*' : ' ', j, brief);
    return (str - buff);
}

static DEVICE_ATTR(rct, 0644, MheRctPrint, MheRctStore);

static irqreturn_t _MheIsr(int irq, void* priv);

mhve_ios* MheIosAcquire(char* tags);

static int
MheProbe(
    struct platform_device* pdev)
{
    int i, err = 0;
    dev_t dev;
    mmhe_dev* mdev = NULL;
    mhve_ios* mios = NULL;
    mhve_reg mregs;
    struct resource* res;
    struct clk* clock;
    int major, minor = 0;

    CamOsPrintf("MHE ko Probe :  %s - %s\n", __DATE__, __TIME__);

    if(0 > (err = alloc_chrdev_region(&dev, minor, MMHE_STREAM_NR + 1, "mstar_mmhe")))
        return err;
    major = MAJOR(dev);
    do
    {
        mmhe_rqc* mrqc;
        err = -ENOMEM;
        if(!(mdev = kzalloc(sizeof(mmhe_dev), GFP_KERNEL)))
            break;
        CamOsMutexInit(&mdev->m_mutex);
        CamOsTsemInit(&mdev->tGetBitsSem, 1);
        CamOsTsemInit(&mdev->m_wqh, MMHE_DEV_STATE_IDLE);
        mrqc = mdev->m_regrqc;
        while(minor < MMHE_STREAM_NR)
        {
            cdev_init(&mrqc[minor].m_cdev, &MrqcFops);
            mrqc[minor].m_cdev.owner = THIS_MODULE;
            if(0 > (err = cdev_add(&mrqc[minor].m_cdev, MKDEV(major, minor), 1)))
                break;
            mrqc[minor].p_mutex = &mdev->m_mutex;
            minor++;
        }
        if(minor < MMHE_STREAM_NR)
            break;
        mdev->i_major = major;
        mdev->i_minor = minor;
        cdev_init(&mdev->m_cdev, &MheFops);
        mdev->m_cdev.owner = THIS_MODULE;
        if(0 > (err = cdev_add(&mdev->m_cdev, MKDEV(major, minor), 1)))
            break;
        mdev->m_dev.devt = MKDEV(major, minor);
        mdev->m_dev.class = msys_get_sysfs_class();
        mdev->m_dev.parent = NULL;
        mdev->m_dev.release = MheDevRelease;
        dev_set_name(&mdev->m_dev, "%s", "mmhe");
        if(0 > (err = device_register(&mdev->m_dev)))
            break;
        if(!(mdev->p_asicip = MheIosAcquire("mhe")))
            break;
        mios = mdev->p_asicip;
        mdev->p_dev = &pdev->dev;
        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        mregs.i_id = 0;
        mregs.base = (void*)IO_ADDRESS(res->start);
        mregs.size = (int)(res->end - res->start);
        mios->set_bank(mios, &mregs);
        res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
        mdev->i_irq = (int)res->start;
        if(0 != (err = request_irq(mdev->i_irq, _MheIsr, IRQF_SHARED, "_MheIsr", mdev)))
            break;
        err = -EINVAL;
        clock = clk_get(&pdev->dev, "CKG_mhe");
        if(IS_ERR(clock))
            break;
        mdev->p_clocks[0] = clock;
        mdev->i_clkidx = 0;
        mdev->i_ratehz = clk_get_rate(clk_get_parent_by_index(clock, mdev->i_clkidx));
        for(i = 1; i < MMHE_CLOCKS_NR; i++)
        {
            clock = of_clk_get(pdev->dev.of_node, i);
            if(IS_ERR(clock))
                break;
            mdev->p_clocks[i] = clock;
        }
        mdev->i_rctidx = 1;
        dev_set_drvdata(&pdev->dev, mdev);

        device_create_file(&mdev->m_dev, &dev_attr_tmr);
        device_create_file(&mdev->m_dev, &dev_attr_clk);
        device_create_file(&mdev->m_dev, &dev_attr_rct);
//      CamOsPrintf("mmhe built at %s on %s\n",__TIME__,__DATE__);
        return 0;
    }
    while(0);

    if(mdev)
    {
        mhve_ios* mios = mdev->p_asicip;
        cdev_del(&mdev->m_cdev);
        if(mios)
            mios->release(mios);
        CamOsMemRelease(mdev);
    }

    return err;
}

static int
MheRemove(
    struct platform_device* pdev)
{
    mmhe_dev* mdev = dev_get_drvdata(&pdev->dev);
    mhve_ios* mios = mdev->p_asicip;
    int devno = MKDEV(mdev->i_major, 0);
    int i = 0;

    CamOsPrintf("MHE ko Remove:  %s - %s\n", __DATE__, __TIME__);

    free_irq(mdev->i_irq, mdev);

    clk_put(mdev->p_clocks[0]);

    while(i < MMHE_STREAM_NR)
        cdev_del(&mdev->m_regrqc[i++].m_cdev);
    cdev_del(&mdev->m_cdev);

    device_unregister(&mdev->m_dev);

    if(mios)
        mios->release(mios);
    kfree(mdev);

    dev_set_drvdata(&pdev->dev, NULL);

    unregister_chrdev_region(devno, MMHE_STREAM_NR + 1);

    return 0;
}

static int
MheSuspend(
    struct platform_device* pdev,
    pm_message_t            state)
{
    mmhe_dev* mdev = dev_get_drvdata(&pdev->dev);

    return MheDevSuspend(mdev);
}

static int
MheResume(
    struct platform_device* pdev)
{
    mmhe_dev* mdev = dev_get_drvdata(&pdev->dev);

    return MheDevResume(mdev);
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
    .suspend = MheSuspend,
    .resume = MheResume,
    .driver = {
        .name = "mmhe",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(MheMatchTables),
    }
};

static int  __init mmhe_init(void)
{
    return platform_driver_register(&MhePdrv);
}

static void __exit mmhe_exit(void)
{
    platform_driver_unregister(&MhePdrv);
}

module_init(mmhe_init);
module_exit(mmhe_exit);
