/*
 * file_access.c
 *
 *  Created on: Nov 28, 2017
 *      Author: giggs.huang
 */
#if defined(CONFIG_ARCH_INFINITY2)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

mm_segment_t oldfs;

void InitKernelEnv(void)
{
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    //set_fs(oldfs);
}

struct file *OpenFile(char *path,int flag,int mode)
{
    struct file *fp;

    InitKernelEnv();

    fp=filp_open(path, flag, mode);
    if (fp) return fp;
    else return NULL;
}

int ReadFile(struct file *fp,char *buf,int readlen)
{
    if (fp->f_op && fp->f_op->read)
        return fp->f_op->read(fp,buf,readlen, &fp->f_pos);
    else
        return -1;
}

int WriteFile(struct file *fp,char *buf,int writelen)
{
    ssize_t writeBytes = 0;
    loff_t pos = 0;
    writeBytes = vfs_write(fp, (char*)buf, writelen, &pos);
    return (int)writeBytes;
/*
    if (fp->f_op && fp->f_op->write)
        return fp->f_op->write(fp,buf,writelen, &fp->f_pos);
    else
        return -1;*/
}

int CloseFile(struct file *fp)
{
    filp_close(fp,NULL);
    set_fs(oldfs);
    return 0;
}
#endif
