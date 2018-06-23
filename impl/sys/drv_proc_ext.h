#ifndef _DRV_PROC_EXT_H_
#define _DRV_PROC_EXT_H_

#include "mi_sys_internal.h"

#ifdef MI_SYS_PROC_FS_DEBUG
#include <linux/semaphore.h>


MI_S32 _MI_SYS_IMPL_Drv_Proc_Init(void);
void _MI_SYS_IMPL_Drv_Proc_Exit(void);
MI_S32 _MI_SYS_IMPL_Allocator_PROC_CREATE(MI_U8 *allocator_proc_name,mi_sys_AllocatorProcfsOps_t *Ops,void *allocator_private_data);
void _MI_SYS_IMPL_Proc_Remove_Allocator(MI_U8 *entry_name);
#endif
#endif
