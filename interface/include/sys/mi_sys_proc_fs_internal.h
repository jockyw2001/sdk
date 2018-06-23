#ifndef _MI_SYS_PROC_FS_INTERNEL_H_
#define _MI_SYS_PROC_FS_INTERNEL_H_
#include "mi_common_macro.h"
#include <linux/proc_fs.h>

typedef enum {
    E_MI_COMMON_POLL_NOT_READY         = (0x0)     ,
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} MI_COMMON_PollFlag_e;

typedef struct MI_COMMON_PollFile_s {
    void *private;
} MI_COMMON_PollFile_t;

#define MI_COMMON_GetPollFd(__ppstPollFile, __fState, __fRelease, __pfd) do{ \
    MI_S32 MI_COMMON_GetPollNamedFd(MI_COMMON_PollFile_t **, MI_COMMON_PollFlag_e (*)(MI_COMMON_PollFile_t*), void (*)(MI_COMMON_PollFile_t*), const char *, unsigned int *); \
    MI_COMMON_GetPollNamedFd(__ppstPollFile, __fState, __fRelease, MACRO_TO_STRING(EXTRA_MODULE_NAME)"_poll", __pfd); \
}while(0)

MI_S32 MI_COMMON_WakeUpClient(MI_COMMON_PollFile_t *pstClient);
//struct kobject *MI_DEVICE_GetDebugObject(void);

#define MI_COMMON_AddDebugFile(__name, __write, __read) do{ \
    MI_S32 MI_COMMON_SetupDebugFile(const char *, MI_S32 (*)(MI_SYS_DEBUG_HANDLE_t ,const char **, MI_S32), MI_S32 (*)(MI_SYS_DEBUG_HANDLE_t)); \
    MI_COMMON_SetupDebugFile(__name, __write, __read); \
}while(0)

#define MI_COMMON_AddDebugRawFile(__name, __write, __read) do{ \
    MI_S32 MI_COMMON_SetupDebugRawFile(const char *,  MI_S32 (*)(const char *, MI_U64, MI_S32), MI_S32 (*)(char *, MI_U64, MI_S32)); \
    MI_COMMON_SetupDebugRawFile(__name, __write, __read); \
}while(0)

#define MI_COMMON_GetSelfDir ({ \
    extern struct proc_dir_entry *MI_DEVICE_GetSelfDir(void); \
    struct proc_dir_entry *MI_COMMON_RealGetSelfDir(struct proc_dir_entry *);\
    MI_COMMON_RealGetSelfDir(MI_DEVICE_GetSelfDir());\
})

#endif /* _MI_SYS_PROC_FS_INTERNEL_H_ */

