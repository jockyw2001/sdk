#include "mi_foo.h"
#include "foo_ioctl.h"
#include "mi_syscall.h"
#include "mi_print.h"

MI_MODULE_DEFINE(foo)

MI_S32 call_foo_function(int arg1, int arg2, int *rval){
    MI_S32 result;
    mi_foo_call_foo_function_t foo = {
        .arg1 = arg1,
        .arg2 = arg2,
    };
    result = MI_SYSCALL(FOO_CALL_FUNCTION, &foo);
    *rval = foo.rval;
    return result;
}
EXPORT_SYMBOL(call_foo_function);

#ifndef __KERNEL__
MI_S32 foo_openpollfd(int *fd, int i){
    mi_foo_openpollfd_t foo = {
        .i = i,
    };
    MI_SYSCALL(FOO_SETUP_POLLFD, &foo);
    *fd = foo.fd;
    return 0;
}

MI_S32 foo_wake(int i){
    mi_foo_poll_id_t id = {i};
    return MI_SYSCALL(FOO_POLLFD_WAKE, &id);
}

MI_S32 foo_ack(int i){
    mi_foo_poll_id_t id = {i};
    return MI_SYSCALL(FOO_POLLFD_ACK, &id);
}
#endif
