#ifndef _DRV_PROC_EXT_K_H_
#define _DRV_PROC_EXT_K_H_

#include "mi_sys_internal.h"


#ifdef MI_SYS_PROC_FS_DEBUG
#include "mi_print.h"
#include "mi_sys_impl.h"
#include "mi_sys_mma_heap_impl.h"
#include "mi_sys_mma_miu_protect_impl.h"
#include "mi_sys_meta_impl.h"
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/hardirq.h>
#include <linux/delay.h>


//#define DEBUG_COMMON_Proc  //do not delete this,default not def.

#define COMMON_STRING_SKIP_BLANK(str)   \
    while (str[0] == ' ')           \
    {                               \
        (str)++;                    \
    }

#define COMMON_STRING_SKIP_NON_BLANK(str)       \
    while (str[0] != ' ' && str[0] != '\0') \
    {								\
        (str)++;					\
    }

#define MAX_ENTRY_NAME_LEN (31)
#define MAX_PROC_ENTRIES 256
#define PROC_DUMP_BUFFER_COMMAND_PARAMETER_NUM 6

typedef MI_S32 (*DRV_PROC_READ_FN)(struct seq_file *, void *);
typedef MI_S32 (*DRV_PROC_WRITE_FN)(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos);
typedef MI_S32 (*DRV_PROC_IOCTL_FN)(struct seq_file *, MI_U32 cmd, MI_U32 arg);
typedef struct tagCOMMON_PROC_REG_PARAM_S
{
    MI_SYS_MOD_DEV_t *pstModDev;
    mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps;
}COMMON_PROC_REG_PARAM_S;

typedef struct tagPROCEX
{
    DRV_PROC_READ_FN fnRead;
    DRV_PROC_WRITE_FN fnWrite;
}DRV_PROC_EX_S;

typedef struct struCMPI_PROC_ITEM
{
    u8 entry_name[MAX_ENTRY_NAME_LEN+1];
    struct proc_dir_entry *entry;
    struct list_head stCmdListHead;
    DRV_PROC_READ_FN read;
    DRV_PROC_WRITE_FN write;
    mi_sys_ModuleDevProcfsOps_t Ops;//N.B. here not use (mi_sys_ModuleDevProcfsOps_t *) !!!!
    MI_SYS_MOD_DEV_t *pstModDev;
    struct proc_dir_entry *dir_entry;
}DRV_PROC_ITEM_S;

typedef struct DRV_PROC_EXEC_CMD_s
{
    MI_U8 *pu8CmdName;
    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *);
    MI_U8 u8MaxPara;
}DRV_PROC_EXEC_CMD_t;

typedef enum
{
    E_MI_MODULE_DUMP_BUFFER_INVALID = 0,
    E_MI_MODULE_DUMP_BUFFER_START = 1,
    E_MI_MODULE_DUMP_BUFFER_END = 2,
    E_MI_MODULE_DUMP_BUFFER_STATE_MAX,
}MI_DUMP_BUFFER_START_STOP_FLAG;

typedef enum
{
    E_MI_MODULE_DUMP_BUFFER_Queue_UsrInject = 1,
    E_MI_MODULE_DUMP_BUFFER_Queue_BindInput = 2,
    E_MI_MODULE_DUMP_BUFFER_Queue_GetFifo = 3,
    E_MI_MODULE_DUMP_BUFFER_Queue_TYPE_MAX,
}MI_DUMP_BUFFER_Queue_TYPE;

typedef enum
{
    E_MI_MODULE_DUMP_BUFFER_INPUT_PORT = 1,
    E_MI_MODULE_DUMP_BUFFER_OUTPUT_PORT = 2,
    E_MI_MODULE_DUMP_BUFFER_PORT_TYPE_MAX,
}MI_DUMP_BUFFER_PORT_TYPE;

struct queue_dump_buf_cmd_info
{
	struct delayed_work		queue_dump_buf_data_wq;
    int idr_num;
    bool force_stop;
    MI_SYS_MOD_DEV_t *pstModDev;
    char module_name[32];
    MI_U32  u32DevId;
    MI_U32 u32ChnId;
    MI_DUMP_BUFFER_PORT_TYPE port_type;//input port or output port.
    MI_U32 port_id;
    MI_DUMP_BUFFER_Queue_TYPE Queue_name;//for which input/output port queue
    char path[256];//target file saved in which dir,here path is absolute path

    //if valid,end with dump these bufnum finished
    //-1 means invalid
    MI_S32 bufnum;

    //if valid,end with time finish
    MI_S32 time;

    MI_DUMP_BUFFER_START_STOP_FLAG start_end;//valid value is 1 and 2,1 means start,2 means stop,and 2 will be set by outside
    void *private;//other value that callback in work needed.
};

typedef void(*PROC_RemoveDev_Fun)(char *);
typedef DRV_PROC_ITEM_S *(*PROC_AddDev_Fun)(char *, DRV_PROC_EX_S*, mi_sys_ModuleDevProcfsOps_t *,struct proc_dir_entry *);
typedef void (*PROC_AddCommand_Fun)(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *), MI_U8 *entry_name);
typedef void (*PROC_ClearCommand_Fun)(MI_U8 *entry_name);
typedef void (*PROC_FindCommand_Fun)(MI_U8 *u8Cmd, MI_U8 *entry_name, DRV_PROC_EXEC_CMD_t *pstExecCmd);

typedef struct struCDev_Proc_IntfParam{
    PROC_AddDev_Fun    addDevfun;
    PROC_RemoveDev_Fun    rmvDevfun;
    PROC_AddCommand_Fun   addDevCmdfun;
    PROC_ClearCommand_Fun clearDevCmdfun;
    PROC_FindCommand_Fun findDevCmdfun;
}DRV_PROC_INTFPARAM;

typedef struct tagCOMMON_PROC_PARAM_S
{
    DRV_PROC_READ_FN pfnReadProc;
    DRV_PROC_WRITE_FN pfnWriteProc;
} COMMON_PROC_PARAM_S;


typedef struct struALLOCATOR_PROC_ITEM
{
    u8 entry_name[MAX_ENTRY_NAME_LEN+1];
    struct proc_dir_entry *entry;
    DRV_PROC_READ_FN read;
    DRV_PROC_WRITE_FN write;
    mi_sys_AllocatorProcfsOps_t Ops;
    void *allocator_private_data;
}ALLOCATOR_PROC_ITEM_S;

ssize_t MI_SYS_BufQueueStatus_IMPL_ProcWrite( struct file * file,  const char __user * buf,size_t count, loff_t *ppos);
int MI_SYS_BufQueueStatus_IMPL_ProcRead(struct seq_file *s, void *pArg);
MI_S32  OnPrintOut_linux_vprintk(MI_SYS_DEBUG_HANDLE_t  handle, char *data,...);
MI_S32  OnPrintOut_linux_seq_write(MI_SYS_DEBUG_HANDLE_t  handle, char *data,...);
void _MI_SYS_IMPL_ModuleIdToPrefixName(MI_ModuleId_e eModuleId , char *prefix_name);

MI_S32  OnWriteOut_linux(MI_SYS_DEBUG_HANDLE_t  handle, char *data,size_t len);
MI_S32  OnPrintOut_linux(MI_SYS_DEBUG_HANDLE_t  handle, char *data,...);
MI_S32 _MI_SYS_IMPL_Drv_Proc_RegisterParam(DRV_PROC_INTFPARAM *param);
void _MI_SYS_IMPL_Drv_Proc_UnRegisterParam(void);
DRV_PROC_ITEM_S* _MI_SYS_IMPL_Drv_Proc_AddDev(MI_U8 *entry_name ,DRV_PROC_EX_S* pfnOpt, void * data,struct proc_dir_entry *proc_dir_entry);
void _MI_SYS_IMPL_Drv_Proc_RemoveDev(char *entry_name);
void _MI_SYS_IMPL_Common_RegProc(MI_SYS_MOD_DEV_t *pstModDev,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps,struct proc_dir_entry *proc_dir_entry);
void _MI_SYS_IMPL_Common_UnRegProc(MI_SYS_MOD_DEV_t *pstModDev);
void _MI_SYS_IMPL_CommonRegCmd(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *), MI_SYS_MOD_DEV_t *pstModDev);
void _MI_SYS_IMPL_CommonClearCmd(MI_SYS_MOD_DEV_t *pstModDev);
int mi_dump_buffer_delay_worker_open(struct inode *inode, struct file *file);
ssize_t mi_dump_buffer_delay_worker_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
#endif
#endif
