#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/poll.h>
#include "mi_common_datatype.h"
#include "mi_common_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_print.h"
#include "mi_sys_internal.h"
#include "drv_proc_ext_k.h"

extern struct proc_dir_entry *g_pCOMMON_proc;



static DEFINE_SEMAPHORE(s_commoninfo_proc_mutex_lock);
typedef struct mi_sys_CommonInfoProcfsOps_s
{

    MI_S32 (*read)(MI_SYS_DEBUG_HANDLE_t  handle);
    MI_S32 (*write)(MI_SYS_DEBUG_HANDLE_t  handle,const char **args, int count);

} mi_sys_CommonInfoProcfsOps_t;

typedef struct COMMONINFO_PROC_ITEM_S
{
    u8 entry_name[MAX_ENTRY_NAME_LEN+1];
    struct proc_dir_entry *entry;
    DRV_PROC_READ_FN read;
    DRV_PROC_WRITE_FN write;
    mi_sys_CommonInfoProcfsOps_t Ops;
}COMMONINFO_PROC_ITEM_S;

static COMMONINFO_PROC_ITEM_S s_commoninfo_proc_items[MAX_PROC_ENTRIES];



static DEFINE_SEMAPHORE(s_commonraw_proc_mutex_lock);
typedef struct mi_sys_CommonRawProcfsOps_s
{
    MI_S32 (*read)(char *buf, MI_U64 off, size_t len);
    MI_S32 (*write)(const char *buf, MI_U64 off, size_t count);

} mi_sys_CommonRawProcfsOps_t;

typedef struct COMMONRAW_PROC_ITEM_S
{
    u8 entry_name[MAX_ENTRY_NAME_LEN+1];
    struct proc_dir_entry *entry;
    mi_sys_CommonRawProcfsOps_t Ops;
}COMMONRAW_PROC_ITEM_S;

static COMMONRAW_PROC_ITEM_S s_commonraw_proc_items[MAX_PROC_ENTRIES];


MI_S32 MI_COMMON_Print(const char *fmt, ...){
    va_list args;
    int i;
    va_start(args, fmt);
    i = vprintk(fmt, args);
    va_end(args);
    return i;
}
EXPORT_SYMBOL(MI_COMMON_Print);

MI_S32 MI_COMMON_Info(const char *fmt, ...){
    char buf[256];
    va_list args;
    int i;
    sprintf(buf, "%s%s", KERN_INFO, fmt);
    va_start(args, fmt);
    i = vprintk(buf, args);
    va_end(args);
    return i;
}
EXPORT_SYMBOL(MI_COMMON_Info);

MI_S32 MI_COMMON_Warning(const char *fmt, ...){
    char buf[256];
    va_list args;
    int i;
    sprintf(buf, "%s%s", KERN_WARNING, fmt);
    va_start(args, fmt);
    i = vprintk(buf, args);
    va_end(args);
    return i;
}
EXPORT_SYMBOL(MI_COMMON_Warning);

MI_S32 MI_COMMON_Error(const char *fmt, ...){
    char buf[256];
    va_list args;
    int i;
    sprintf(buf, "%s%s", KERN_ERR, fmt);
    va_start(args, fmt);
    i = vprintk(buf, args);
    va_end(args);
    return i;
}
EXPORT_SYMBOL(MI_COMMON_Error);

MI_S32 _MI_SYS_IMPL_CommonInfo_ReadProc(struct seq_file* q, void* v)
{
    COMMONINFO_PROC_ITEM_S *item = q->private;
    MI_SYS_DEBUG_HANDLE_t  handle;
    //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    MI_SYS_BUG_ON(item->read != _MI_SYS_IMPL_CommonInfo_ReadProc);
    //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_seq_write;
    //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    if(item->Ops.read)
    {
        //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
        item->Ops.read(handle);
    }
    else
    {
        printk("not realize read proc\n");
    }

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_IMPL_CommonInfo_WriteProc(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos)
{
    char szBuf[250];
    size_t buf_copied_size = -1;//default value not be 0,because return 0 will cause loop.
    const char *args[32] = {NULL};
    int c = 0;
    MI_S32 rval;
    char *p = szBuf, tc;

    struct seq_file* q = file->private_data;

    COMMONINFO_PROC_ITEM_S *item = q->private;
    
    MI_SYS_DEBUG_HANDLE_t  handle;
    handle.file = (void *)q;
    handle.OnPrintOut = OnPrintOut_linux_vprintk;
    MI_SYS_BUG_ON(item->write != _MI_SYS_IMPL_CommonInfo_WriteProc);
    buf_copied_size = min(count, (sizeof(szBuf)-1));
    if(copy_from_user(szBuf, user_buf, buf_copied_size))
    {
        DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        return -EFAULT;
    }


    do{
        p += strspn(p, " \t\r\f\v");
        if(*p == '\n')
            break;
        args[c++] = p;
        p += strcspn(p, " \t\n\r\f\v");
        tc = *p;
        *p = '\0';
        p++;
    }while(tc != '\n' && (c < 32));

    if(32 == c)
    {
        DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        return -EFAULT;
    }

    rval = item->Ops.write(handle,args,c);
    return rval;

}

ssize_t commoninfo_proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    struct seq_file *s = file->private_data;
    COMMONINFO_PROC_ITEM_S *item = s->private;

    if (item->write)
        return item->write(file, user_buf, count, ppos);

    return -ENOSYS;

}

static int commoninfo_proc_open(struct inode *inode, struct file *file)
{
    COMMONINFO_PROC_ITEM_S *item = PDE_DATA(inode);
    int ret;
    //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    if (item && item->read)
    {

        //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
        ret = single_open(file, item->read, item);

        //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
        return ret;
    }

    //printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    return -ENOSYS;
}


static const struct file_operations _mi_commoninfo_debug_fops = {
    .owner      = THIS_MODULE,
    .open       = commoninfo_proc_open,
    .read       = seq_read,
    .write      = commoninfo_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

COMMON_PROC_PARAM_S s_stCommonInfoProc =
{
    .pfnReadProc      = _MI_SYS_IMPL_CommonInfo_ReadProc,
    .pfnWriteProc      = _MI_SYS_IMPL_CommonInfo_WriteProc,

};

MI_S32 MI_COMMON_SetupDebugFile(const char *name,MI_S32 (*write)(MI_SYS_DEBUG_HANDLE_t  handle,const char **args, int count), MI_S32 (*read)(MI_SYS_DEBUG_HANDLE_t  handle)){
    struct  proc_dir_entry  *entry;
    int i;
    if ((NULL == name) || (strlen(name) > MAX_ENTRY_NAME_LEN))
    {
        return MI_ERR_SYS_FAILED;
    }
    down(&s_commoninfo_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! s_commoninfo_proc_items[i].entry)
            break;

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_commoninfo_proc_mutex_lock);
        DBG_INFO("ERROR: add proc entry %s over LIMIT:%#x\n",  name, MAX_PROC_ENTRIES);
        return MI_ERR_SYS_FAILED;
    }
    strncpy(s_commoninfo_proc_items[i].entry_name, name, sizeof(s_commoninfo_proc_items[i].entry_name) - 1);

    s_commoninfo_proc_items[i].read = s_stCommonInfoProc.pfnReadProc;
    s_commoninfo_proc_items[i].write = s_stCommonInfoProc.pfnWriteProc;
    s_commoninfo_proc_items[i].Ops.read = read;
    s_commoninfo_proc_items[i].Ops.write = write;
    entry =  proc_create_data(name, 0, g_pCOMMON_proc, &_mi_commoninfo_debug_fops,&s_commoninfo_proc_items[i]);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  %s.\n",name);
        up(&s_commoninfo_proc_mutex_lock);
        return MI_ERR_SYS_FAILED;
    }
    
    s_commoninfo_proc_items[i].entry = entry;
    up(&s_commoninfo_proc_mutex_lock);    
    //printk("######## debug code %s \n",__FUNCTION__);
    return MI_SUCCESS;
}
EXPORT_SYMBOL(MI_COMMON_SetupDebugFile);

static int commonraw_proc_open(struct inode *inode, struct file *file)
{
    COMMONRAW_PROC_ITEM_S *item = PDE_DATA(inode);
    //printk("in %s %d\n",__FUNCTION__,__LINE__);
    file->private_data = item;//save it as file private data.
    return MI_SUCCESS;
}

ssize_t commonraw_proc_read(struct file *file, char __user *user_buf, size_t size, loff_t *ppos)
{
    COMMONRAW_PROC_ITEM_S *item = file->private_data;
    void *tmp_buf;
    int ret;
    //printk("in %s:%d\n",__FUNCTION__,__LINE__);

    if(!item->Ops.read)
    {
        printk("not register read!! for %s\n",__FUNCTION__);
        return -1;
    }

    tmp_buf = kmalloc(size, GFP_KERNEL | __GFP_NOWARN);
	if (!tmp_buf && size > PAGE_SIZE)
		tmp_buf = vmalloc(size);

    if(!tmp_buf)
    {
        printk("in %s:%d\n",__FUNCTION__,__LINE__);
        return -ENOMEM;
    }

    ret = item->Ops.read(tmp_buf, *ppos, size);

    if (copy_to_user(user_buf, tmp_buf, ret))//here use ret of item->Ops.read,not use size.and ret <= size
    {
        kvfree(tmp_buf);
        printk("in %s:%d \n",__FUNCTION__,__LINE__);
        goto Error;
    }
    //printk("in %s:%d  size=0x%x\n",__FUNCTION__,__LINE__,size);
    kvfree(tmp_buf);

    return ret;//here should return success ,not bytes copied

Error:
    printk("in %s:%d\n",__FUNCTION__,__LINE__);
    return ret ;
}

ssize_t commonraw_proc_write(struct file *file,
		const char __user *user_buf, size_t size, loff_t *ppos)
{
    COMMONRAW_PROC_ITEM_S *item = file->private_data;
    void *tmp_buf;
    int ret;
    tmp_buf = kmalloc(size, GFP_KERNEL | __GFP_NOWARN);
	if (!tmp_buf && size > PAGE_SIZE)
		tmp_buf = vmalloc(size);

    if(!tmp_buf)
        return -ENOMEM;

    if(copy_from_user(tmp_buf, user_buf, size))
    {
        DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        kvfree(tmp_buf);
        return -EFAULT;
    }

    if(item->Ops.write)
    {
        //printk("in %s:%d   item->Ops.write=%p\n",__FUNCTION__,__LINE__,item->Ops.write);
        ret = item->Ops.write(tmp_buf, *ppos, size);
        //printk("in %s:%d\n",__FUNCTION__,__LINE__);
        kvfree(tmp_buf);
        //printk("in %s:%d\n",__FUNCTION__,__LINE__);
        if(ret != size)
        {
            DBG_ERR("%s :%d \n",__FUNCTION__,__LINE__);
        }
        //printk("in %s:%d\n",__FUNCTION__,__LINE__);

        return ret;    
    }
    else
    {
        printk("not register write!! for %s\n",__FUNCTION__);
        kvfree(tmp_buf);
        return -1;
    }

}

static const struct file_operations _mi_commonraw_debug_fops = {
    .owner      = THIS_MODULE,
    .open       = commonraw_proc_open,
    .read       = commonraw_proc_read,
    .write      = commonraw_proc_write,
};


MI_S32 MI_COMMON_SetupDebugRawFile(const char *name, MI_S32 (*write)(const char *buf, MI_U64 off, size_t count), MI_S32 (*read)(char *buf, MI_U64 off, size_t len)){
    struct  proc_dir_entry  *entry;
    int i;
    //printk(" debug code %s start\n",__FUNCTION__);

    down(&s_commonraw_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! s_commonraw_proc_items[i].entry)
            break;

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_commonraw_proc_mutex_lock);
        DBG_INFO("ERROR: add proc entry %s over LIMIT:%#x\n",  name, MAX_PROC_ENTRIES);
        return MI_ERR_SYS_FAILED;
    }
    strncpy(s_commonraw_proc_items[i].entry_name, name, sizeof(s_commonraw_proc_items[i].entry_name) - 1);

    s_commonraw_proc_items[i].Ops.write = write;
    s_commonraw_proc_items[i].Ops.read = read;
    
    //printk(" debug code %s write=%p,Ops.write=%p\n",__FUNCTION__,write,s_commonraw_proc_items[i].Ops.write);
    entry =  proc_create_data(name, 0, g_pCOMMON_proc, &_mi_commonraw_debug_fops,&s_commonraw_proc_items[i]);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  .\n");
        up(&s_commonraw_proc_mutex_lock); 
        return MI_ERR_SYS_FAILED;
    }
    s_commonraw_proc_items[i].entry = entry;
    up(&s_commonraw_proc_mutex_lock);    

   //printk("######## debug code %s\n",__FUNCTION__);
   return MI_SUCCESS;
}
EXPORT_SYMBOL(MI_COMMON_SetupDebugRawFile);

struct proc_dir_entry *MI_COMMON_RealGetSelfDir(struct proc_dir_entry *proc_dir_entry)
{
    return proc_dir_entry;
}
EXPORT_SYMBOL(MI_COMMON_RealGetSelfDir);



typedef struct {
    MI_COMMON_PollFile_t stPollFile;
    wait_queue_head_t stPollHead;
    struct file *filp;
    MI_COMMON_PollFlag_e (*fState)(struct MI_COMMON_PollFile_s *);
    void (*fRelease)(struct MI_COMMON_PollFile_s *);
} MI_COMMON_PollFileWrapper_t;

MI_S32 MI_COMMON_WakeUpClient(MI_COMMON_PollFile_t *pstPollFile){
    MI_COMMON_PollFileWrapper_t *f = container_of(pstPollFile, MI_COMMON_PollFileWrapper_t, stPollFile);
    wake_up_interruptible(&f->stPollHead);
    return 0;
}
EXPORT_SYMBOL(MI_COMMON_WakeUpClient);

static unsigned int MI_COMMON_Poll(struct file *filp, poll_table *wait){
    MI_COMMON_PollFileWrapper_t *f = filp->private_data;
    unsigned int req_events = poll_requested_events(wait);
    poll_wait(filp, &f->stPollHead, wait);
    if(f->fState){
        unsigned int mask = 0;
        MI_COMMON_PollFlag_e e = f->fState(&f->stPollFile);
        if(e & E_MI_COMMON_FRAME_READY_FOR_READ){
            mask |= POLLIN;
        }
        if(e & E_MI_COMMON_BUFFER_READY_FOR_WRITE){
            mask |= POLLOUT;
        }
        return req_events & mask;
    }
    return POLLERR;
}

static int MI_COMMON_PollRelease(struct inode *inode, struct file *filp){
    MI_COMMON_PollFileWrapper_t *f = filp->private_data;
    if(f){
        if(f->fRelease){
            f->fRelease(&f->stPollFile);
        }

        kfree(f);
    }
    return 0;
}

static const struct file_operations _stPoll = {
    .release   = MI_COMMON_PollRelease,
    .poll      = MI_COMMON_Poll,
};

MI_S32 MI_COMMON_GetPollNamedFd(MI_COMMON_PollFile_t **ppstPollFile, MI_COMMON_PollFlag_e (*fState)(MI_COMMON_PollFile_t*), void (*fRelease)(MI_COMMON_PollFile_t*), const char *name, unsigned int *pfd){
    MI_COMMON_PollFileWrapper_t *f;
    int fd;
    if(!ppstPollFile)
        return E_MI_ERR_FAILED;
    fd = get_unused_fd();
    *pfd = -1;
    if(fd < 0)
        return E_MI_ERR_FAILED;
    if(*ppstPollFile == NULL){
        f = kzalloc(sizeof(MI_COMMON_PollFileWrapper_t), GFP_KERNEL);
        if(!f){
            put_unused_fd(fd);
            return E_MI_ERR_FAILED;
        }
        f->filp = anon_inode_getfile(name, &_stPoll, f, O_RDWR);
        if(!f->filp){
            put_unused_fd(fd);
            kfree(f);
            return E_MI_ERR_FAILED;
        }
        init_waitqueue_head(&f->stPollHead);
        f->fState = fState;
        f->fRelease = fRelease;
        *ppstPollFile = &f->stPollFile;
    }else{
        f = container_of(*ppstPollFile, MI_COMMON_PollFileWrapper_t, stPollFile);
        get_file(f->filp);
    }
    fd_install(fd, f->filp);
    *pfd = fd;
    return 0;
}
EXPORT_SYMBOL(MI_COMMON_GetPollNamedFd);
