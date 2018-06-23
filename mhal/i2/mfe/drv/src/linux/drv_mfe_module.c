
#include "mdrv_mfe_io.h"
#include "mdrv_rqct_io.h"
#include "drv_mfe_kernel.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"
#include "hal_mfe_api.h"
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int  MfeOpen(struct inode*, struct file*);
static int  MfeRelease(struct inode*, struct file*);
static long MfeIoctl(struct file*, unsigned int, unsigned long);

static irqreturn_t _MfeIsr(int irq, void* priv)
{
    mmfe_dev* mdev = (mmfe_dev*)priv;

    if (!MfeDevIsrFnx(mdev))
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
    mmfe_dev* mdev = container_of(inode->i_cdev,mmfe_dev,m_cdev);
    mmfe_ctx* mctx;
    if (!(mctx = MfeCtxAcquire(mdev)))
        return -ENOMEM;
    if (0 <= MfeDevRegister(mdev, mctx))
    {
        file->private_data = mctx;
        return 0;
    }
    mctx->release(mctx);
    return -EINVAL;
}

static int
MfeRelease(
    struct inode*   inode,
    struct file*    file)
{
    mmfe_dev* mdev = container_of(inode->i_cdev,mmfe_dev,m_cdev);
    mmfe_ctx* mctx = file->private_data;
    MfeDevUnregister(mdev, mctx);
    file->private_data = NULL;
    mctx->release(mctx);
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
    char buf[256];
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
        if (0 > (err = MfeCtxActions(mctx, cmd, buf)))
            break;
        if ((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
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

static struct file_operations MrqcFops = {
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
    mmfe_rqc* mrqc = container_of(inode->i_cdev,mmfe_rqc,m_cdev);
    mmfe_ctx* mctx = NULL;
    CamOsMutexLock(mrqc->p_mutex);
    if (NULL != (mctx = mrqc->p_mctx))
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
    mmfe_ctx* mctx = file->private_data;
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

    if (_IOC_TYPE(cmd) != MAGIC_RQCT)
        return -EINVAL;
    switch (cmd)
    {
    case IOCTL_RQCT_VERSION:
        if (access_ok(ACCESS_WRITE,uptr,sizeof(unsigned int)))
            err = __put_user(v,(unsigned int*)uptr);
        break;
    default:
        if ((_IOC_DIR(cmd) & _IOC_WRITE) && CamOsCopyFromUpperLayer(buf, uptr, n))
            break;
        if (0 > (err = MfeRqcIoctls(mctx, cmd, buf)))
            break;
        if ((_IOC_DIR(cmd) & _IOC_READ) && CamOsCopyToUpperLayer(uptr, buf, n))
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
    size_t lsize = MfeRqcPrint(mctx, line, RQCT_LINE_SIZE);
    if (size > lsize)
        size = lsize;
    if (size > 0 && CamOsCopyToUpperLayer(uptr, line, size))
        return (ssize_t)err;
    return (ssize_t)size;
}

static void MfeDevRelease(struct device* dev) {}

static ssize_t MfeTmrStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    int i, thresh = 0;
    if (0 != kstrtol(buff, 10, (long int*)&thresh))
        return -EINVAL;
    mdev->i_thresh = thresh;
    for (i = 0; i < MMFE_STREAM_NR; i++)
        mdev->i_counts[i][0] = mdev->i_counts[i][1] = mdev->i_counts[i][2] = mdev->i_counts[i][3] = mdev->i_counts[i][4] = 0;
    return n;
}

static ssize_t MfeTmrPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    int i;
    for (i = 0; i < MMFE_STREAM_NR; i++)
        str += scnprintf(str,end-str,"inst-%d:%8d/%5d/%5d/%8d/%8d\n",i,mdev->i_counts[i][0],mdev->i_counts[i][1],mdev->i_counts[i][2],mdev->i_counts[i][3],mdev->i_counts[i][4]);
    str += scnprintf(str,end-str,"thresh:%8d\n",mdev->i_thresh);
    return (str - buff);
}

static DEVICE_ATTR(tmr,0644,MfeTmrPrint,MfeTmrStore);

static ssize_t MfeClkStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    struct clk* clock = mdev->p_clocks[0];
    struct clk* ck = NULL;
    int id = 0;
    if (0 != kstrtol(buff, 10, (long int*)&id))
        return -EINVAL;
    ck = clk_get_parent_by_index(clock, id);
    if (ck == NULL)
        mdev->i_ratehz = mdev->i_clkidx = -1;
    else
    {
        mdev->i_clkidx = id;
        mdev->i_ratehz = clk_get_rate(ck);
    }
    return n;
}

static ssize_t MfeClkPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    if (mdev->i_clkidx < 0)
        str += scnprintf(str,end-str,"clkidx:%3d(dynamic)\n",mdev->i_clkidx);
    else
        str += scnprintf(str,end-str,"clkidx:%3d(%3d mhz)\n",mdev->i_clkidx,mdev->i_ratehz/1000000);
    return (str - buff);
}

static DEVICE_ATTR(clk,0644,MfeClkPrint,MfeClkStore);

char* RqctMfeComment(int);

static ssize_t MfeRctStore(struct device* dev, struct device_attribute* attr, const char* buff, size_t n)
{
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    int id = 0;
    char* brief;
    if (0 != kstrtol(buff, 10, (long int*)&id))
        return -EINVAL;
    if ((brief = RqctMfeComment(id)) && *brief)
        mdev->i_rctidx = id;
    return n;
}

static ssize_t MfeRctPrint(struct device* dev, struct device_attribute* attr, char* buff)
{
    char* str = buff;
    char* end = buff + PAGE_SIZE;
    mmfe_dev* mdev = container_of(dev,mmfe_dev,m_dev);
    char* brief;
    int j;
    for (j = 0; (brief = RqctMfeComment(j)); j++)
        if (*brief)
            str += scnprintf(str,end-str,"(%c)[%d]%s\n",j==mdev->i_rctidx?'*':' ',j,brief);
    return (str - buff);
}

static DEVICE_ATTR(rct,0644,MfeRctPrint,MfeRctStore);

static int
MfeProbe(
    struct platform_device* pdev)
{
    int i, err = 0;
    dev_t dev;
    mmfe_dev* mdev = NULL;
    mhve_ios* mios = NULL;
    mhve_reg mregs;
    struct resource* res;
    struct clk* clock;
    int major, minor = 0;

    CamOsPrintf("MFE ko Probe :  %s - %s\n", __DATE__, __TIME__);

    if (0 > (err = alloc_chrdev_region(&dev, minor, MMFE_STREAM_NR+1, "mstar_mmfe")))
        return err;
    major = MAJOR(dev);
    do
    {
        mmfe_rqc* mrqc;
        err = -ENOMEM;
        if (!(mdev = kzalloc(sizeof(mmfe_dev), GFP_KERNEL)))
            break;
        CamOsMutexInit(&mdev->m_mutex);
        CamOsTsemInit(&mdev->tGetBitsSem, 1);
        CamOsTsemInit(&mdev->m_wqh, MMFE_DEV_STATE_IDLE);
        mrqc = mdev->m_regrqc;
        while (minor < MMFE_STREAM_NR)
        {
            cdev_init(&mrqc[minor].m_cdev, &MrqcFops);
            mrqc[minor].m_cdev.owner = THIS_MODULE;
            if (0 > (err = cdev_add(&mrqc[minor].m_cdev, MKDEV(major,minor), 1)))
                break;
            mrqc[minor].p_mutex = &mdev->m_mutex;
            minor++;
        }
        if (minor < MMFE_STREAM_NR)
            break;
        mdev->i_major = major;
        mdev->i_minor = minor;
        cdev_init(&mdev->m_cdev, &MfeFops);
        mdev->m_cdev.owner = THIS_MODULE;
        if (0 > (err = cdev_add(&mdev->m_cdev, MKDEV(major,minor), 1)))
            break;
        mdev->m_dev.devt = MKDEV(major,minor);
        mdev->m_dev.class = msys_get_sysfs_class();
        mdev->m_dev.parent = NULL;
        mdev->m_dev.release = MfeDevRelease;
        dev_set_name(&mdev->m_dev, "%s", "mmfe");
        if (0 > (err = device_register(&mdev->m_dev)))
            break;
        if (!(mdev->p_asicip = MfeIosAcquire("mfe5")))
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
        if (0 != (err = request_irq(mdev->i_irq, _MfeIsr, IRQF_SHARED, "_MfeIsr", mdev)))
            break;
        err = -EINVAL;
        clock = clk_get(&pdev->dev, "CKG_mfe");
        if (IS_ERR(clock))
            break;
        mdev->p_clocks[0] = clock;
        mdev->i_clkidx = 0;
        mdev->i_ratehz = clk_get_rate(clk_get_parent_by_index(clock,mdev->i_clkidx));
        for (i = 1; i < MMFE_CLOCKS_NR; i++)
        {
            clock = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(clock))
                break;
            mdev->p_clocks[i] = clock;
        }
        mdev->i_rctidx = 1;
        dev_set_drvdata(&pdev->dev, mdev);

        device_create_file(&mdev->m_dev, &dev_attr_tmr);
        device_create_file(&mdev->m_dev, &dev_attr_clk);
        device_create_file(&mdev->m_dev, &dev_attr_rct);
//      CamOsPrintf("mmfe built at %s on %s\n",__TIME__,__DATE__);
        return 0;
    }
    while (0);

    if (mdev)
    {
        mhve_ios* mios = mdev->p_asicip;
        cdev_del(&mdev->m_cdev);
        if (mios)
            mios->release(mios);
        CamOsMemRelease(mdev);
    }

    return err;
}

static int
MfeRemove(
    struct platform_device* pdev)
{
    mmfe_dev* mdev = dev_get_drvdata(&pdev->dev);
    mhve_ios* mios = mdev->p_asicip;
    int devno = MKDEV(mdev->i_major,0);
    int i = 0;

    CamOsPrintf("MFE ko Remove:  %s - %s\n", __DATE__, __TIME__);

    free_irq(mdev->i_irq, mdev);

    clk_put(mdev->p_clocks[0]);

    while (i < MMFE_STREAM_NR)
        cdev_del(&mdev->m_regrqc[i++].m_cdev);
    cdev_del(&mdev->m_cdev);

    device_unregister(&mdev->m_dev);

    if (mios)
        mios->release(mios);
    kfree(mdev);

    dev_set_drvdata(&pdev->dev, NULL);

    unregister_chrdev_region(devno, MMFE_STREAM_NR+1);

    return 0;
}

static int
MfeSuspend(
    struct platform_device* pdev,
    pm_message_t            state)
{
    mmfe_dev* mdev = dev_get_drvdata(&pdev->dev);

    return MfeDevSuspend(mdev);
}

static int
MfeResume(
    struct platform_device* pdev)
{
    mmfe_dev* mdev = dev_get_drvdata(&pdev->dev);

    return MfeDevResume(mdev);
}

static const struct of_device_id MfeMatchTables[] = {
    { .compatible = "mstar,mfe" },
    {},
};

static struct platform_driver MfePdrv = {
    .probe = MfeProbe,
    .remove = MfeRemove,
    .suspend = MfeSuspend,
    .resume = MfeResume,
    .driver = {
        .name = "mmfe",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(MfeMatchTables),
    }
};

static int  __init mmfe_init(void)
{
    return platform_driver_register(&MfePdrv);
}

static void __exit mmfe_exit(void)
{
    platform_driver_unregister(&MfePdrv);
}

module_init(mmfe_init);
module_exit(mmfe_exit);
