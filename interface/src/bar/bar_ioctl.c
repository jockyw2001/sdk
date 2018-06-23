#include "mi_device.h"
#include "mi_bar_internal.h"
#include "mi_common_internal.h"
#include "mi_syscfg.h"
#include "bar_impl.h"
#include "mi_print.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_sys_internal.h"

static MI_DEVICE_WrapperFunction_t ioctl_table[BAR_NR] = {
#if INTERFACE_FOO == 1
    [BAR_NR_CALL_FUNCTION] = impl_call_bar_function,
#endif
};

static MI_S32 sysfs_query_mmap(MI_SYS_DEBUG_HANDLE_t  handle,const char **args, MI_S32 count){
	if(count == 1){
		const MI_SYSCFG_MmapInfo_t *mmap;
		if(MI_SYSCFG_GetMmapInfo(args[0], &mmap)){
			MI_PRINT("%s:GID=%d,Addr=0x%x,Size=0x%x,Layer=%d,Align=0x%x,MemoryType=0x%x,MiuNo=%d,CMAID=%d\n", args[0], mmap->u8Gid, mmap->u32Addr, mmap->u32Size, mmap->u8Layer, mmap->u32Align, mmap->u32MemoryType, mmap->u8MiuNo, mmap->u8CMAHid);
			return count;
		}
	}
	return -EINVAL;
}

static void mi_wrapper_init(const char *name){
    MI_PRINT("module [%s] init\n", name);
    MI_COMMON_AddDebugFile("query_mmap", sysfs_query_mmap, NULL);
}

static void mi_wrapper_connected(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_wrapper_disconnected(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");

MI_DEVICE_DEFINE(mi_wrapper_init,
               ioctl_table, BAR_NR,
               mi_wrapper_connected, mi_wrapper_disconnected);
