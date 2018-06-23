#ifndef _MI_SYSCALL_H_
#define _MI_SYSCALL_H_
#include "mi_common_macro.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define MI_MODULE_DEFINE(__name) \
static int _MI_WRAPPER_DEVICE_Open(void){ \
    static int fd = -1; \
    if(fd == -1){ \
        fd = open("/dev/mi_"MACRO_TO_STRING(EXTRA_MODULE_NAME), O_RDWR); \
    } \
    if(fd < 0){ \
        printf("[%s] failed to open %s!(%s)\n", MACRO_TO_STRING(EXTRA_MODULE_NAME), "/dev/mi/"MACRO_TO_STRING(EXTRA_MODULE_NAME), strerror(errno)); \
    } \
    return fd; \
}

#define MI_SYSCALL(__cmd, __p) ({ \
    struct {  \
        int __len; \
        unsigned long long __ptr; \
    } __tr = {_IOC_SIZE(__cmd), (long)__p}; \
    int __rval = ioctl(_MI_WRAPPER_DEVICE_Open(), __cmd, &__tr);\
    if(__rval == -1){ \
        printf("[%s] failed to ioctl 0x%08x!(%s)\n", MACRO_TO_STRING(EXTRA_MODULE_NAME), __cmd, strerror(errno)); \
    } \
    __rval; \
	})

#define MI_SYSCALL_VOID(__cmd) ({ \
    int __rval = ioctl(_MI_WRAPPER_DEVICE_Open(), __cmd, NULL);\
    if(__rval == -1){ \
        printf("[%s] failed to ioctl 0x%08x!(%s)\n", MACRO_TO_STRING(EXTRA_MODULE_NAME), __cmd, strerror(errno)); \
    } \
    __rval; \
	})

#define EXPORT_SYMBOL(e)
#endif /* _MI_SYSCALL_H_ */
