#include "drv_proc_ext.h"
#include "drv_proc_ext_k.h"

#include "mi_sys_log_impl.h"
#include <linux/delay.h>

#ifdef MI_SYS_PROC_FS_DEBUG

struct idr idr_delay_worker;

struct workqueue_struct *mi_sys_debug_workqueue;

static DEFINE_SEMAPHORE(s_proc_mutex_lock);
static DEFINE_SEMAPHORE(s_allocator_proc_mutex_lock);

struct proc_dir_entry *g_pCMPI_proc = NULL;
struct proc_dir_entry *g_pMstar_proc = NULL;
struct proc_dir_entry *g_pMma_proc = NULL;
struct proc_dir_entry *g_pLog_proc = NULL;
struct proc_dir_entry *g_pCOMMON_proc = NULL;

static DRV_PROC_ITEM_S s_proc_items[MAX_PROC_ENTRIES];
static ALLOCATOR_PROC_ITEM_S s_allocator_proc_items[MAX_PROC_ENTRIES];

extern COMMON_PROC_PARAM_S s_stAllocatorProc;
MI_S32 _MI_SYS_IMPL_Allocator_ReadProc(struct seq_file* q, void* v);
typedef struct MI_SYS_CmdList_s
{
    DRV_PROC_EXEC_CMD_t stExecCmd;
    struct list_head stCmdList;
}MI_SYS_CmdList_t;

static struct list_head *_MI_SYS_IMPL_ListFind(struct list_head *head, void *key, struct list_head *(*list_find_fp)(struct list_head *, void*))
{
    struct list_head *pos = NULL;
    struct list_head *pos_next = NULL;

    pos = head->next;
    while(pos != head)
    {
        pos_next = list_find_fp(pos, key);
        if (pos_next == pos)
        {
            break;
        }
        pos = pos_next;
    }
    return pos;
}
static struct list_head *_MI_SYS_IMPL_CmdListCmp(struct list_head *pstList, void* key)
{
    MI_SYS_CmdList_t *pstCmdList = NULL;
    MI_U8 *pu8CmdName = (MI_U8 *)key;

    MI_SYS_BUG_ON(!pstList);
    MI_SYS_BUG_ON(!pu8CmdName);

    pstCmdList = list_entry(pstList, MI_SYS_CmdList_t, stCmdList);

    return strcmp(pu8CmdName, pstCmdList->stExecCmd.pu8CmdName)?(pstCmdList->stCmdList.next):(&pstCmdList->stCmdList);
}

static void _MI_SYS_IMPL_Proc_AddCmd(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *), MI_U8 *entry_name)
{
    int i;
    MI_SYS_CmdList_t *pstCmdList;

    MI_SYS_BUG_ON(!u8Cmd);
    MI_SYS_BUG_ON(!fpExecCmd);
    MI_SYS_BUG_ON(!entry_name);

    down(&s_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
    {
        if (! strncmp(s_proc_items[i].entry_name, entry_name, sizeof(s_proc_items[i].entry_name)))
            break;
    }

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_proc_mutex_lock);
        DBG_INFO("Not find the entry:%s\n", entry_name);
        return ;
    }
    pstCmdList = kmalloc(sizeof(MI_SYS_CmdList_t), GFP_KERNEL);
    MI_SYS_BUG_ON(!pstCmdList);
    memset(pstCmdList, 0, sizeof(MI_SYS_CmdList_t));
    pstCmdList->stExecCmd.pu8CmdName = kmalloc(strlen(u8Cmd) + 1, GFP_KERNEL);
    MI_SYS_BUG_ON(!pstCmdList->stExecCmd.pu8CmdName);
    strcpy(pstCmdList->stExecCmd.pu8CmdName, u8Cmd);
    pstCmdList->stExecCmd.fpExecCmd = fpExecCmd;
    pstCmdList->stExecCmd.u8MaxPara = u8MaxPara;
    list_add_tail(&pstCmdList->stCmdList, &s_proc_items[i].stCmdListHead);

    up(&s_proc_mutex_lock);

    return;
}
static void _MI_SYS_IMPL_Proc_ClearCmd(MI_U8 *entry_name)
{
    MI_SYS_CmdList_t *pstCmdList;
    MI_SYS_CmdList_t *pstNCmdList;
    int i;

    MI_SYS_BUG_ON(!entry_name);

    down(&s_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
    {
        if (! strncmp(s_proc_items[i].entry_name, entry_name, sizeof(s_proc_items[i].entry_name)))
            break;
    }

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_proc_mutex_lock);
        DBG_INFO("Not find the entry:%s\n", entry_name);
        return ;
    }

    list_for_each_entry_safe(pstCmdList, pstNCmdList, &s_proc_items[i].stCmdListHead, stCmdList)
    {
        list_del(&pstCmdList->stCmdList);
        kfree(pstCmdList->stExecCmd.pu8CmdName);
        kfree(pstCmdList);
    }

    up(&s_proc_mutex_lock);
    return;

}
static void _MI_SYS_IMPL_Proc_FindCmd(MI_U8 *u8Cmd, MI_U8 *entry_name, DRV_PROC_EXEC_CMD_t *pstExecCmd)
{
    MI_SYS_CmdList_t *pstCmdList;
    struct list_head *pstList;
    int i;

    MI_SYS_BUG_ON(!u8Cmd);
    MI_SYS_BUG_ON(!entry_name);
    MI_SYS_BUG_ON(!pstExecCmd);

    memset(pstExecCmd, 0, sizeof(DRV_PROC_EXEC_CMD_t));
    down(&s_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
    {
        if (! strncmp(s_proc_items[i].entry_name, entry_name, strlen(entry_name)+1))
            break;
    }

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_proc_mutex_lock);
        DBG_INFO("Not find the entry:%s\n", entry_name);
        return;
    }

    pstList = _MI_SYS_IMPL_ListFind(&s_proc_items[i].stCmdListHead, u8Cmd, _MI_SYS_IMPL_CmdListCmp);
    if (pstList != &s_proc_items[i].stCmdListHead)
    {
        pstCmdList = list_entry(pstList, MI_SYS_CmdList_t, stCmdList);
        *pstExecCmd = pstCmdList->stExecCmd;
    }
    up(&s_proc_mutex_lock);
    return;
}
static int _MI_SYS_IMPL_Dev_Proc_Open(struct inode *inode, struct file *file)
{
    DRV_PROC_ITEM_S *item = PDE_DATA(inode);

    if (item && item->read)
        return single_open(file, item->read, item);

    return -ENOSYS;
}
static ssize_t _MI_SYS_IMPL_Dev_Proc_Write(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file *s = file->private_data;
    DRV_PROC_ITEM_S *item = s->private;

    if (item->write)
        return item->write(file, buf, count, ppos);

    return -ENOSYS;
}

static struct file_operations Dev_proc_ops __attribute__((unused))	 = {
    .owner   = THIS_MODULE,
    .open    = _MI_SYS_IMPL_Dev_Proc_Open,
    .read    = seq_read,
    .write   = _MI_SYS_IMPL_Dev_Proc_Write,
    .llseek  = seq_lseek,
    .release = single_release,
};

static DRV_PROC_ITEM_S *_MI_SYS_IMPL_Proc_AddDev(char *entry_name, DRV_PROC_EX_S* pFnOp, mi_sys_ModuleDevProcfsOps_t * Ops,struct proc_dir_entry *dir_entry)
{
    struct proc_dir_entry *entry;
    int i;
#ifdef DEBUG_COMMON_Proc
    mi_sys_ModuleDevProcfsOps_t *tmp_pstModuleProcfsOps;
#endif

    if ((NULL == entry_name) || (strlen(entry_name) > MAX_ENTRY_NAME_LEN))
    {
        return NULL;
    }

    down(&s_proc_mutex_lock);

    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! s_proc_items[i].entry)
            break;

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_proc_mutex_lock);
        DBG_INFO("ERROR: add proc entry %s over LIMIT:%#x\n",  entry_name, MAX_PROC_ENTRIES);
        return NULL;
    }
    strncpy(s_proc_items[i].entry_name, entry_name, strlen(entry_name));
    s_proc_items[i].entry_name[strlen(entry_name)] = '\0';

    if (pFnOp != NULL)
    {
        s_proc_items[i].read = pFnOp->fnRead;
        s_proc_items[i].write = pFnOp->fnWrite;
    }
    else
    {
        s_proc_items[i].read = NULL;
        s_proc_items[i].write = NULL;
    }

    memcpy(&s_proc_items[i].Ops,Ops,sizeof(mi_sys_ModuleDevProcfsOps_t));
#ifdef DEBUG_COMMON_Proc
    tmp_pstModuleProcfsOps = (mi_sys_ModuleDevProcfsOps_t *)Ops;
    printk("%s:%d pstProcItem info %p  %p %p  %p %p\n",__FUNCTION__,__LINE__,tmp_pstModuleProcfsOps
                                                ,tmp_pstModuleProcfsOps->OnDumpDevAttr,tmp_pstModuleProcfsOps->OnDumpChannelAttr
                                                ,tmp_pstModuleProcfsOps->OnDumpInputPortAttr,tmp_pstModuleProcfsOps->OnDumpOutPortAttr);
#endif

    if(dir_entry != NULL)
    {
        entry = proc_create_data(entry_name, 0, dir_entry, &Dev_proc_ops, &s_proc_items[i]);
        s_proc_items[i].dir_entry = dir_entry;
    }
    else
    {
        if(0 != strncmp(entry_name,"mi_fb",strlen("mi_fb")))//entry_name may be xx0,xx1,xx2
        {
            up(&s_proc_mutex_lock);
            DBG_ERR("fail,error! this case only support mi_fbxxx!!!\n");
            MI_SYS_BUG();
            return NULL;
        }
        entry = proc_create_data(entry_name, 0, g_pCMPI_proc, &Dev_proc_ops, &s_proc_items[i]);
        s_proc_items[i].dir_entry = g_pCMPI_proc;
    }
    if (!entry)
    {
        up(&s_proc_mutex_lock);
        return NULL;
    }

    s_proc_items[i].entry = entry;
    INIT_LIST_HEAD(&s_proc_items[i].stCmdListHead);
#ifdef DEBUG_COMMON_Proc
    printk("%s:%d pstProcItem info %p  %p %p  %p %p\n",__FUNCTION__,__LINE__,tmp_pstModuleProcfsOps
                                                ,tmp_pstModuleProcfsOps->OnDumpDevAttr,tmp_pstModuleProcfsOps->OnDumpChannelAttr
                                                ,tmp_pstModuleProcfsOps->OnDumpInputPortAttr,tmp_pstModuleProcfsOps->OnDumpOutPortAttr);
#endif


    up(&s_proc_mutex_lock);

    DBG_INFO("add: i:%#x, entry_name:%s\n", i, s_proc_items[i].entry_name);

    return &s_proc_items[i];

}

static void _MI_SYS_IMPL_Proc_RemoveDev(char *entry_name)
{
    int i;
    int id;
    struct queue_dump_buf_cmd_info *dump_buf_cmd_info;

    down(&s_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
    {
        if (! strncmp(s_proc_items[i].entry_name, entry_name, strlen(entry_name)+1))
            break;
    }

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_proc_mutex_lock);
        DBG_INFO("Not find the entry:%s\n", entry_name);
        return ;
    }

    //if have valid dump_buffer workers related to this entry_name device,force stop them.
    rcu_read_lock();
    idr_for_each_entry(&idr_delay_worker, dump_buf_cmd_info, id)
    {
        MI_SYS_BUG_ON(dump_buf_cmd_info->idr_num != id);
        if(s_proc_items[i].pstModDev == dump_buf_cmd_info->pstModDev)
        {
            dump_buf_cmd_info->force_stop = TRUE;
        }
    }
    rcu_read_unlock();

RETRY_WAIT:
    msleep(100);
    rcu_read_lock();
    idr_for_each_entry(&idr_delay_worker, dump_buf_cmd_info, id)
    {
        MI_SYS_BUG_ON(dump_buf_cmd_info->idr_num != id);
        if(s_proc_items[i].pstModDev == dump_buf_cmd_info->pstModDev)
        {
            rcu_read_unlock();
            goto RETRY_WAIT;
        }
    }
    rcu_read_unlock();
    //all workers related to this entry_name device have been finished.

    remove_proc_entry(s_proc_items[i].entry_name, s_proc_items[i].dir_entry);
    s_proc_items[i].entry = NULL;
    s_proc_items[i].dir_entry = NULL;

    up(&s_proc_mutex_lock);
}

static DRV_PROC_INTFPARAM  procparam = {
    .addDevfun = _MI_SYS_IMPL_Proc_AddDev,
    .rmvDevfun = _MI_SYS_IMPL_Proc_RemoveDev,
    .addDevCmdfun = _MI_SYS_IMPL_Proc_AddCmd,
    .clearDevCmdfun = _MI_SYS_IMPL_Proc_ClearCmd,
    .findDevCmdfun = _MI_SYS_IMPL_Proc_FindCmd,
};

static ssize_t mi_global_info_show(struct seq_file *m,void *v)
{
    static char mstar_mi_sys_version_string[] = MACRO_TO_STRING(MSTAR_MODULE_VERSION);

    seq_printf(m,"miu_and_lx_info:\n");
    dump_miu_and_lx_info(m);

    seq_printf(m,"PAGE_OFFSET - the virtual address of the start of the kernel image\n");
    seq_printf(m,"TASK_SIZE - the maximum size of a user space task\n");
    //different kernel&sdk config may have different VMALLOC_START,
    //for debug feture issues easier,here dump value of VMALLOC_START.
    seq_printf(m,"%15s%15s%15s%15s\n","PAGE_OFFSET","TASK_SIZE"
                                         ,"VMALLOC_START","VMALLOC_END");
    seq_printf(m,"%15lx%15lx%15lx%15lx\n",PAGE_OFFSET,TASK_SIZE
                                                ,VMALLOC_START,VMALLOC_END);

    seq_printf(m,"%s\n",mstar_mi_sys_version_string);

    return MI_SUCCESS;
}

int mi_global_info_open(struct inode *inode, struct file *file)
{
    single_open(file,mi_global_info_show,PDE_DATA(inode));
    return MI_SUCCESS;
}


static int mi_log_info_open(struct inode *inode, struct file *file)
{
    single_open(file,MI_SYS_LOG_IMPL_ProcRead,PDE_DATA(inode));
    return MI_SUCCESS;
}


static const struct file_operations mi_global_info_fops = {
    .owner   = THIS_MODULE,
    .open = mi_global_info_open,
    .read = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations mi_log_info_fops = {
    .owner   = THIS_MODULE,
    .open = mi_log_info_open,
    .read = seq_read,
    .write = MI_SYS_LOG_IMPL_ProcWrite,
    .llseek  = seq_lseek,
    .release = single_release,
};


static const struct file_operations miu_protect_fops = {
    .owner      = THIS_MODULE,
    .open       = miu_protect_open,
    .read       = seq_read,
    .write      = miu_protect_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static const struct file_operations mi_dump_buffer_delay_worker_fops = {
        .owner      = THIS_MODULE,
        .open       = mi_dump_buffer_delay_worker_open,
        .read       = seq_read,
        .write      = mi_dump_buffer_delay_worker_write,
        .llseek     = seq_lseek,
        .release    = single_release,
};

static int mi_BufQueueStatus_open(struct inode *inode, struct file *file)
{
    single_open(file,MI_SYS_BufQueueStatus_IMPL_ProcRead,PDE_DATA(inode));
    return MI_SUCCESS;
}

static const struct file_operations mi_BufQueueStatus_fops = {
    .owner   = THIS_MODULE,
    .open = mi_BufQueueStatus_open,
    .read = seq_read,
    .write = MI_SYS_BufQueueStatus_IMPL_ProcWrite,
    .llseek  = seq_lseek,
    .release = single_release,
};

ssize_t allocator_proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    struct seq_file *s = file->private_data;
    ALLOCATOR_PROC_ITEM_S *item = s->private;

    if (item->write)
        return item->write(file, user_buf, count, ppos);

    return -ENOSYS;

}
static int allocator_proc_open(struct inode *inode, struct file *file)
{
    ALLOCATOR_PROC_ITEM_S *item = PDE_DATA(inode);
    int ret;
    if (item && item->read)
    {
        ret = single_open(file, item->read, item);
        //printk("item %p    item->read=%p  s_stAllocatorProc.pfnReadProc=%p   _MI_SYS_IMPL_Allocator_ReadProc=%p\n",item,item->read,s_stAllocatorProc.pfnReadProc,_MI_SYS_IMPL_Allocator_ReadProc);
        return ret;
    }

    printk("in  %s:%d\n",__FUNCTION__,__LINE__);
    return -ENOSYS;
}


static const struct file_operations allocator_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = allocator_proc_open,
    .read       = seq_read,
    .write      = allocator_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

MI_S32 _MI_SYS_IMPL_Allocator_PROC_CREATE(MI_U8 *allocator_proc_name,mi_sys_AllocatorProcfsOps_t *Ops,void *allocator_private_data)
{
    struct  proc_dir_entry  *entry;
    int i;
    if ((NULL == allocator_proc_name) || (strlen(allocator_proc_name) > MAX_ENTRY_NAME_LEN))
    {
        return MI_ERR_SYS_FAILED;
    }
    down(&s_allocator_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! s_allocator_proc_items[i].entry)
            break;

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_allocator_proc_mutex_lock);
        DBG_INFO("ERROR: add proc entry %s over LIMIT:%#x\n",  allocator_proc_name, MAX_PROC_ENTRIES);
        return MI_ERR_SYS_FAILED;
    }
    strncpy(s_allocator_proc_items[i].entry_name, allocator_proc_name, strlen(allocator_proc_name));
    s_allocator_proc_items[i].entry_name[strlen(allocator_proc_name)]='\0';

    s_allocator_proc_items[i].read = s_stAllocatorProc.pfnReadProc;
    s_allocator_proc_items[i].write = s_stAllocatorProc.pfnWriteProc;
    s_allocator_proc_items[i].allocator_private_data = (void *)allocator_private_data;
    memcpy(&s_allocator_proc_items[i].Ops,Ops,sizeof(mi_sys_AllocatorProcfsOps_t));
    //printk("item.read %p   s_stAllocatorProc.pfnReadProc %p\n",s_allocator_proc_items[i].read,s_stAllocatorProc.pfnReadProc);
    entry =  proc_create_data(allocator_proc_name,0,g_pMma_proc,&allocator_proc_fops,&s_allocator_proc_items[i]);
    if (!entry)
    {
        up(&s_allocator_proc_mutex_lock);
        printk(KERN_ERR "failed  to  create  procfs  file  %s.\n",allocator_proc_name);
        return MI_ERR_SYS_FAILED;
    }
    s_allocator_proc_items[i].entry = entry;
    up(&s_allocator_proc_mutex_lock);
    return MI_SUCCESS;
}

void _MI_SYS_IMPL_Proc_Remove_Allocator(MI_U8 *allocator_proc_name)
{
    int i;
    down(&s_allocator_proc_mutex_lock);
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
    {
        if (! strncmp(s_allocator_proc_items[i].entry_name, allocator_proc_name, max(strlen(s_allocator_proc_items[i].entry_name),strlen(allocator_proc_name))))
            break;
    }

    if (MAX_PROC_ENTRIES == i)
    {
        up(&s_allocator_proc_mutex_lock);
        DBG_INFO("Not find the entry:%s\n", allocator_proc_name);
        return ;
    }

    remove_proc_entry(allocator_proc_name, g_pMma_proc);
    s_allocator_proc_items[i].entry = NULL;
    up(&s_allocator_proc_mutex_lock);
}

MI_S32 _MI_SYS_IMPL_Drv_Proc_Init(void)
{
    extern struct proc_dir_entry *MI_DEVICE_GetProcModuleDir(void);
    struct  proc_dir_entry  *entry;
    memset(s_proc_items, 0x00, sizeof(s_proc_items));
    _MI_SYS_IMPL_Drv_Proc_RegisterParam(&procparam);
    idr_init(&idr_delay_worker);

    mi_sys_debug_workqueue = create_workqueue("mi_sys_debug wq");
    if(mi_sys_debug_workqueue == NULL)
        return MI_ERR_SYS_NOMEM;

    //create  /proc/Mstar/
    g_pMstar_proc = proc_mkdir("Mstar", NULL);

    //create  /proc/Mstar/mi_modules/
    //g_pCMPI_proc = proc_mkdir("mi_modules", g_pMstar_proc);
    g_pCMPI_proc = MI_DEVICE_GetProcModuleDir();

    //symlink  /proc/Mstar/mi_modules/  and  /proc/mi_modules
    //proc_symlink("mi_modules", NULL, "Mstar/mi_modules");

    //create  /proc/mi_modules/mi_dump_buffer_delay_worker
    entry = proc_create_data("mi_dump_buffer_delay_worker",0,g_pCMPI_proc,&mi_dump_buffer_delay_worker_fops,NULL);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  mi_dump_buffer_delay_worker.\n");
        return MI_ERR_SYS_FAILED;
    }

    g_pCOMMON_proc = proc_mkdir("common", g_pCMPI_proc);

    /*
    create  /proc/mi_modules/mi_global_info
    */
    entry =  proc_create_data("mi_global_info", 0, g_pCMPI_proc, &mi_global_info_fops,NULL);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  mi_global_info.\n");
        return MI_ERR_SYS_FAILED;
    }

    entry =  proc_create("mi_log_info", 0666, g_pCMPI_proc, &mi_log_info_fops);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  mi_log_info.\n");
        return MI_ERR_SYS_FAILED;
    }

    //create  /proc/mi_modules/mi_BufQueueStatus
    entry =  proc_create("mi_bufqueue_status", 0666, g_pCMPI_proc, &mi_BufQueueStatus_fops);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  mi_BufQueueStatus.\n");
        return MI_ERR_SYS_FAILED;
    }

    //create  /proc/mi_modules/mi_sys_mma/
    g_pMma_proc = proc_mkdir("mi_sys_mma",g_pCMPI_proc);

    entry =  proc_create_data("miu_protect",0,g_pMma_proc,&miu_protect_fops,NULL);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  miu_protect.\n");
        return MI_ERR_SYS_FAILED;
    }

    return MI_SUCCESS;
}

void _MI_SYS_IMPL_Drv_Proc_Exit(void)
{
    _MI_SYS_IMPL_Drv_Proc_UnRegisterParam();

    idr_destroy(&idr_delay_worker);

    //remove  /proc/mi_modules/mi_dump_buffer_delay_worker
    remove_proc_entry("mi_dump_buffer_delay_worker",g_pCMPI_proc);

    //remove  /proc/mi_modules/mi_sys_mma/mi_protect
    remove_proc_entry("mi_protect",g_pMma_proc);

    //remove  /proc/mi_modules/mi_sys_mma/
    remove_proc_entry("mi_sys_mma",g_pCMPI_proc);

    //remove  /proc/mi_modules/mi_BufQueueStatus
    remove_proc_entry("mi_BufQueueStatus",g_pCMPI_proc);

    //remove  /proc/mi_modules/mi_log_info
    remove_proc_entry("mi_log_info",g_pCMPI_proc);

    //remove  /proc/mi_modules/mi_global_info
    remove_proc_entry("mi_global_info",g_pCMPI_proc);

    //remove  /proc/mi_modules/common/
    remove_proc_entry("common",g_pCOMMON_proc);

    //remove  /proc/mi_modules/
    remove_proc_entry("mi_modules", NULL);

    //remove  /proc/Mstar/mi_modules/
    remove_proc_entry("mi_modules", g_pMstar_proc);

    //remove  /proc/Mstar/
    remove_proc_entry("Mstar", NULL);

    return;
}
#endif

