#ifndef _MI_DEVICE_H_
#define _MI_DEVICE_H_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/major.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include "mi_common.h"
#include "mi_common_internal.h"

#define AUTO_LOCK(e, m) do{ \
    MI_DEVICE_Context_t *_env = e;  \
    mutex_lock(&(m)); \
    _env->pstMutex = &(m); \
}while(0)

typedef struct {
    MI_COMMON_Client_t stClient;
} MI_DEVICE_File_t;

typedef MI_S32 (*MI_DEVICE_WrapperFunction_t)(struct MI_DEVICE_Context_s *, void*);

typedef struct MI_DEVICE_Object_s {
    struct list_head list;
    struct proc_dir_entry *dir;
    const char *name;
    int minor;
    struct device *this_device;
    MI_DEVICE_WrapperFunction_t *ioctl_table;
    int table_size;
    void (*connected)(MI_COMMON_Client_t *);
    void (*disconnected)(MI_COMMON_Client_t *);
} MI_DEVICE_Object_t;

extern int MI_DEVICE_Register(MI_DEVICE_Object_t *mi);
extern void MI_DEVICE_Unregister(MI_DEVICE_Object_t *mi);

extern MI_DEVICE_Object_t *MI_DEVICE_Instance(void);

#define MIDEVICE_NAME EXTRA_MODULE_NAME##__midevice

#define MI_DEVICE_DEFINE(_init, _ioctl, _ioc_size, _c, _d)                                  \
MI_DEVICE_WrapperFunction_t MI_DEVICE_WrapperFunction(int cmd){                             \
    if(_ioc_size > _IOC_NR(cmd)){                                                           \
        return ((MI_DEVICE_WrapperFunction_t*)_ioctl)[_IOC_NR(cmd)];                        \
    }                                                                                       \
    return NULL;                                                                            \
}                                                                                           \
static MI_DEVICE_Object_t MIDEVICE_NAME = {                                                 \
    .name = MACRO_TO_STRING(EXTRA_MODULE_NAME),                                             \
    .ioctl_table = _ioctl,                                                                  \
    .table_size = _ioc_size,                                                                \
    .connected = _c,                                                                        \
    .disconnected = _d,                                                                     \
};                                                                                          \
static MI_DBG_LEVEL_e debug_level = MI_DBG_WRN;                                             \
static struct proc_dir_entry *debug_level_file;                                             \
static struct proc_dir_entry *module_version_file;                                          \
MI_DBG_LEVEL_e MI_DEVICE_GetDebugLevel(void){return debug_level;}                           \
MI_DEVICE_Object_t *MI_DEVICE_Instance(void){return &MIDEVICE_NAME;}                        \
struct proc_dir_entry *MI_DEVICE_GetSelfDir(void){return MIDEVICE_NAME.dir;}                \
module_param(debug_level,uint,0644);                                                        \
static int __init EXTRA_MODULE_NAME##__module_init(void){                                   \
    extern struct proc_dir_entry *MI_DEVICE_GetProcModuleDir(void);                         \
    extern const struct file_operations mi_device_number_ops;                               \
    extern const struct file_operations mi_device_version_ops;                               \
    INIT_LIST_HEAD(&MIDEVICE_NAME.list);                                                    \
    MIDEVICE_NAME.dir = proc_mkdir("mi_"MACRO_TO_STRING(EXTRA_MODULE_NAME), MI_DEVICE_GetProcModuleDir()); \
    debug_level_file = proc_create_data("debug_level", 0640, MIDEVICE_NAME.dir, &mi_device_number_ops, &debug_level);  \
    module_version_file = proc_create_data("module_version_file",0640,MIDEVICE_NAME.dir,&mi_device_version_ops,&MACRO_TO_STRING(MSTAR_MODULE_VERSION));   \
    _init(MACRO_TO_STRING(EXTRA_MODULE_NAME));                                              \
    return MI_DEVICE_Register(&MIDEVICE_NAME);                                              \
}                                                                                           \
module_init(EXTRA_MODULE_NAME##__module_init)                                               \
static void __exit EXTRA_MODULE_NAME##__module_exit(void){                                  \
    proc_remove(debug_level_file);                                                          \
    proc_remove(MIDEVICE_NAME.dir);                                                         \
    MI_DEVICE_Unregister(&MIDEVICE_NAME);                                                   \
}                                                                                           \
module_exit(EXTRA_MODULE_NAME##__module_exit)
#endif /* _MI_DEVICE_H_ */
