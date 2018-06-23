#include "mi_device.h"
#include "mi_common_internal.h"

#include <linux/module.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_mtx);
static struct class *device_class;
static int device_major;
static struct proc_dir_entry *root;
#define MAX_MINORS 64 /* max module number */
static DECLARE_BITMAP(device_minors, MAX_MINORS);

int MI_DEVICE_Register(MI_DEVICE_Object_t *device)
{
    int err = 0, i;

    mutex_lock(&device_mtx);
    i = find_first_zero_bit(device_minors, MAX_MINORS);
    if (i >= MAX_MINORS) {
        err = -EBUSY;
        goto out;
    }
    device->minor = MAX_MINORS - i - 1;
    set_bit(i, device_minors);

    device->this_device =
        device_create(device_class, NULL, MKDEV(device_major, device->minor), device, "mi_%s", device->name);

    if (IS_ERR(device->this_device)) {
        err = PTR_ERR(device->this_device);
        goto out;
    }

    list_add(&device->list, &device_list);
out:
    mutex_unlock(&device_mtx);
    return err;
}

EXPORT_SYMBOL(MI_DEVICE_Register);

void MI_DEVICE_Unregister(MI_DEVICE_Object_t *device){
    int i = MAX_MINORS - device->minor - 1;

    if (WARN_ON(list_empty(&device->list)))
        return;

    mutex_lock(&device_mtx);

    list_del(&device->list);

    device_destroy(device_class, MKDEV(device_major, device->minor));
    if (i < MAX_MINORS && i >= 0)
        clear_bit(i, device_minors);
    mutex_unlock(&device_mtx);
}
EXPORT_SYMBOL(MI_DEVICE_Unregister);

static int MI_DEVICE_Open(struct inode *inode, struct file *filp){
    int minor = iminor(inode);
    MI_DEVICE_Object_t *o;
    MI_DEVICE_File_t *f = kzalloc(sizeof(*f), GFP_KERNEL);
    if(!f){
        return -ENOMEM;
    }

    mutex_lock(&device_mtx);

    list_for_each_entry(o, &device_list, list) {
        if (o->minor == minor) {
            f->stClient.pstDevice = o;
            f->stClient.tid = current->pid;
            f->stClient.pid = current->tgid;
            filp->private_data = f;
            break;
        }
    }

    mutex_unlock(&device_mtx);
    if(o->connected){
        o->connected(&f->stClient);
    }
    return 0;
}

static int MI_DEVICE_Release(struct inode *inode, struct file *filp){
    MI_DEVICE_File_t *f = filp->private_data;
    filp->private_data = NULL;
    if(f->stClient.pstDevice->disconnected){
        f->stClient.pstDevice->disconnected(&f->stClient);
    }
    kfree(f);
    return 0;
}

static long MI_DEVICE_Ioctl(struct file *filp, unsigned int cmd, unsigned long ptr){
    MI_DEVICE_File_t *f = filp->private_data;
    int findex = _IOC_NR(cmd);
    long rval = -EIO;
    if(_IOC_TYPE(cmd) == 'i' && findex >= 0 && findex < f->stClient.pstDevice->table_size){
        MI_DEVICE_Context_t env = {.eFrom = E_MI_COMMON_CALL_FROM_IOCTL};
        if(ptr){
            struct {
                int len;
                unsigned long long ptr;
            } tr;
            void *arg = NULL;
            copy_from_user(&tr, (void*)ptr, sizeof(tr));
            if(tr.len > _IOC_SIZE(cmd)){
                printk(KERN_ERR "write cmd(0x%08x) overflow!", cmd);
                return -EINVAL;
            }

            if(tr.len > 4096){
                printk(KERN_WARNING "write cmd(0x%08x) Send Big Data size(%d)!", cmd, tr.len);
            }

            if(_IOC_DIR(cmd) & _IOC_WRITE){
                if(tr.len == 0){
                    printk(KERN_ERR "write cmd(0x%08x) send null data!", cmd);
                    return -EINVAL;
                }
                arg = memdup_user((void*)(long)tr.ptr, tr.len);
                if(!arg)
                    return -ENOMEM;
            }else if(_IOC_DIR(cmd) & _IOC_READ){
                arg = kmalloc(tr.len, GFP_KERNEL);
                if(!arg)
                    return -ENOMEM;
            }else{
                printk(KERN_ERR "send a buffer to cmd(0x%08x) with_IOC_TYPE_NONE!\n", cmd);
                return -EINVAL;
            }
            env.pstClient = &f->stClient;
            env.strModuleName = MACRO_TO_STRING(EXTRA_MODULE_NAME);
            rval = f->stClient.pstDevice->ioctl_table[findex](&env, arg);
            if(_IOC_DIR(cmd) & _IOC_READ){
                copy_to_user((void*)(long)tr.ptr, arg, tr.len);
            }
            kfree(arg);
        }else{
            env.pstClient = &f->stClient;
            env.strModuleName = MACRO_TO_STRING(EXTRA_MODULE_NAME);
            rval = f->stClient.pstDevice->ioctl_table[findex](&env, NULL);
        }
        if(env.pstMutex != NULL){
            mutex_unlock(env.pstMutex);
        }
    }
    return rval;
}

static const struct file_operations fops = {
    .owner      = THIS_MODULE,
    .open       = MI_DEVICE_Open,
    .release    = MI_DEVICE_Release,
    .unlocked_ioctl = MI_DEVICE_Ioctl,
    .llseek     = noop_llseek,
};

static ssize_t number_write(struct file *filp, const char __user *buf, size_t size, loff_t *off) {
    int n;
    if(kstrtouint_from_user(buf, size, 0, &n) == 0){
        MI_DBG_LEVEL_e *level = PDE_DATA(file_inode(filp));
        *level = n;
        return size;
    }
    return -EINVAL;
}

static ssize_t number_show(struct seq_file *m, void *v) {
    seq_printf(m, "%d\n", *(MI_DBG_LEVEL_e*)m->private);
    return 0;
}

static int number_open(struct inode *inode, struct file *filp){
    return single_open(filp, number_show, PDE_DATA(inode));
}

const struct file_operations mi_device_number_ops = {
    .owner      = THIS_MODULE,
    .open       = number_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .write      = number_write,
    .release    = single_release,
};
EXPORT_SYMBOL(mi_device_number_ops);

static ssize_t version_write(struct file *filp, const char __user *buf, size_t size, loff_t *off) {
    //do nothing
    return size;
}

static ssize_t version_show(struct seq_file *m, void *v) {
    seq_printf(m,"%s\n",(char *)m->private);
    return 0;
}

static int version_open(struct inode *inode, struct file *filp){
    return single_open(filp, version_show, PDE_DATA(inode));
}

const struct file_operations mi_device_version_ops = {
    .owner      = THIS_MODULE,
    .open       = version_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .write      = version_write,
    .release    = single_release,
};
EXPORT_SYMBOL(mi_device_version_ops);

struct proc_dir_entry *MI_DEVICE_GetProcModuleDir(void){
    return root;
}
EXPORT_SYMBOL(MI_DEVICE_GetProcModuleDir);

static int __init core_module_init(void){
    int err = 0;
    device_class = class_create(THIS_MODULE, "mi");
    err = PTR_ERR(device_class);
    if (IS_ERR(device_class))
        goto fail_class_create;

    err = -EIO;
    device_major = register_chrdev(0, "mi", &fops);
    if(device_major <= 0)
        goto fail_register_chrdev;

    root = proc_mkdir("mi_modules", NULL);
    if(!root)
        goto fail_create_mi_modules;

    return 0;

fail_create_mi_modules:
    printk(KERN_ERR "failed create /proc/mi_modules\n");
    unregister_chrdev(device_major, "mi");
fail_register_chrdev:
    printk(KERN_ERR "unable to get mi device\n");
    class_destroy(device_class);
fail_class_create:
    printk(KERN_ERR "fail create class\n");
    return err;
}
module_init(core_module_init)

static void __exit core_module_exit(void){
    proc_remove(root);
    unregister_chrdev(device_major, "mi");
    class_destroy(device_class);
}
module_exit(core_module_exit)

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");
