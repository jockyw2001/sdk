#include <linux/sched.h>
#include <linux/slab.h>
#include "mi_device.h"
#include "foo_ioctl.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_common_internal.h"
#include "foo.h"
#include "vif.h"
#include "vpe.h"
#include "ai.h"
#include "ao.h"
#include "vdec.h"
#include "venc.h"
#include "divp.h"
#include "disp.h"
#include "mi_print.h"

static MI_S32 impl_call_foo_function(MI_DEVICE_Context_t *env, void *ptr){
    mi_foo_call_foo_function_t *arg = ptr;
    return mi_call_foo_function(arg->arg1, arg->arg2, &arg->rval);
}

static MI_COMMON_PollFile_t *files[32];

static void impl_pollfd_release(MI_COMMON_PollFile_t *c){
    MI_PRINT("release\n");
}

static MI_COMMON_PollFlag_e impl_pollfd_state(MI_COMMON_PollFile_t *f){
    return (MI_COMMON_PollFlag_e)f->private;
}

static MI_S32 impl_setup_pollfd(MI_DEVICE_Context_t *env, void *ptr){
    mi_foo_openpollfd_t *arg = ptr;
    MI_COMMON_GetPollFd(files + arg->i, impl_pollfd_state, impl_pollfd_release, &arg->fd);
    return 0;
}

static MI_S32 impl_pollfd_wake(MI_DEVICE_Context_t *env, void *ptr){
    mi_foo_poll_id_t *arg = ptr;
    files[arg->i]->private = (void*)(E_MI_COMMON_FRAME_READY_FOR_READ|E_MI_COMMON_BUFFER_READY_FOR_WRITE);
    MI_COMMON_WakeUpClient(files[arg->i]);
    return 0;
}

static MI_S32 impl_pollfd_ack(MI_DEVICE_Context_t *env, void *ptr){
    mi_foo_poll_id_t *arg = ptr;
    files[arg->i]->private = (void*)E_MI_COMMON_POLL_NOT_READY;
    return 0;
}

static MI_DEVICE_WrapperFunction_t ioctl_table[FOO_NR] = {
    [FOO_NR_CALL_FUNCTION] = impl_call_foo_function,
    [FOO_NR_SETUP_POLLFD] = impl_setup_pollfd,
    [FOO_NR_POLLFD_WAKE] = impl_pollfd_wake,
    [FOO_NR_POLLFD_ACK] = impl_pollfd_ack,
};

#if 0
static MI_S32 sysfs_example_write(const char **args, MI_S32 count){
    MI_S32 i;
    for(i = 0; i < count; ++i){
        MI_PRINT("arg[%d] = (%s)\n", i, args[i]);
    }
    return count;
}

static MI_S32 sysfs_example_read(char *buf, MI_S32 count){
    return sprintf(buf, "hello world!(%d)\n", count);
}

static MI_S32 sysfs_vif_init(const char **args, MI_S32 count){
    vif_init();
    return count;
}

static MI_S32 sysfs_vif_isr(const char **args, MI_S32 count){
    vif_isr();
    return count;
}

static MI_S32 sysfs_vpe_init(const char **args, MI_S32 count){
    vpe_init();
    return count;
}

static MI_S32 sysfs_vpe_isr(const char **args, MI_S32 count){
    vpe_isr();
    return count;
}

static MI_S32 sysfs_vpe_add_task(const char **args, MI_S32 count){
    if(count == 1){
        int v = -1;
        kstrtoint(args[0], 0, &v);
        vpe_add_task(v);
        return count;
    }
    return -EINVAL;
}

static MI_S32 sysfs_vdec_init(const char **args, MI_S32 count){
    vdec_init();
    return count;
}
static MI_S32 sysfs_vdec_send_input(const char **args, MI_S32 count){
    if(count == 1){
        int v = 0;
        kstrtoint(args[0], 0, &v);
        vdec_send_input(v);
        return count;
    }
    return -EINVAL;
}

static MI_S32 sysfs_disp_init(const char **args, MI_S32 count){
    disp_init();
    return count;
}

static MI_S32 sysfs_disp_isr(const char **args, MI_S32 count){
    if(count == 1){
        int v = 0;
        kstrtoint(args[0], 0, &v);
        disp_isr(v);
        return count;
    }
    return -EINVAL;
}
#endif
static void mi_wrapper_init(const char *name){
    MI_PRINT("module [%s] init\n", name);
    vif_init();
    vpe_init();
    disp_init();
    divp_init();
    vdec_init();
    ai_init();
    ao_init();
    venc_init();
 #if 0
    MI_COMMON_AddDebugFile("example", sysfs_example_write, sysfs_example_read);
    MI_COMMON_AddDebugFile("vif_init", sysfs_vif_init, NULL);
    MI_COMMON_AddDebugFile("vif_isr", sysfs_vif_isr, NULL);
    MI_COMMON_AddDebugFile("vpe_init", sysfs_vpe_init, NULL);
    MI_COMMON_AddDebugFile("vpe_isr", sysfs_vpe_isr, NULL);
    MI_COMMON_AddDebugFile("vpe_add_task", sysfs_vpe_add_task, NULL);
    MI_COMMON_AddDebugFile("vdec_init", sysfs_vdec_init, NULL);
    MI_COMMON_AddDebugFile("vdec_send_input", sysfs_vdec_send_input, NULL);
    MI_COMMON_AddDebugFile("disp_init", sysfs_disp_init, NULL);
    MI_COMMON_AddDebugFile("disp_isr", sysfs_disp_isr, NULL);
#endif
}

static void mi_wrapper_connected(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] connected\n", client->pid);
}

static void mi_wrapper_disconnected(MI_COMMON_Client_t *client){
    MI_PRINT("client [%d] disconnected\n", client->pid);
    memset(files, 0, sizeof(files));
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("colin.hu <colin.hu@mstarsemi.com>");

MI_DEVICE_DEFINE(mi_wrapper_init,
               ioctl_table, FOO_NR,
               mi_wrapper_connected, mi_wrapper_disconnected);
