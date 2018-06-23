#ifndef _MI_SYS_MMA_MIU_PROTECT_IMPL_H_
#define _MI_SYS_MMA_MIU_PROTECT_IMPL_H_
#include <linux/list.h>
#include <linux/mutex.h>

#include <linux/version.h>
#include <linux/kernel.h>
//#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
//#include <mstar/mstar_chip.h>
//#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
//#include <../../drivers/mstar/cpu/include/mstar/mstar_chip.h>
//#else
//#error not support this kernel version
//#endif

#include <linux/printk.h>
#include <linux/bug.h>
#include <linux/slab.h>
#include <linux/gfp.h>

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#include <mach/memory.h>
#include <mstar/mstar_chip.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#include <mstar_chip.h>
//nothing
#else
#error not support this kernel version
#endif

#include "mi_sys_internal.h"
#include "mi_print.h"

#ifdef MI_SYS_PROC_FS_DEBUG
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#endif

#define MIU_BLOCK_NUM 4
#define KERN_CHUNK_NUM 3
#define MIU_PROTECT_ENABLE 1
#define MIU_PROTECT_DISABLE 0

int deleteKRange(unsigned long long start_cpu_bus_pa, unsigned long length);
int addKRange(unsigned long long start_cpu_bus_pa, unsigned long length);
extern unsigned short *g_kernel_protect_client_id;
void init_glob_miu_kranges(void);

#ifdef MI_SYS_PROC_FS_DEBUG
void dump_miu_and_lx_info(struct seq_file *m);
ssize_t miu_protect_write(struct file *file, const char __user *user_buf, size_t size, loff_t *ppos);
int kprotect_status(struct seq_file *m, void *v);
int miu_protect_open(struct inode *inode, struct file *file);
#endif
#endif
