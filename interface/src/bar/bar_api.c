#include "mi_bar.h"
#include "mi_bar_internal.h"
#include "mi_syscall.h"
#include "mi_print.h"
MI_MODULE_DEFINE(bar)

MI_S32 call_bar_function(){
#if INTERFACE_FOO == 1
    return MI_SYSCALL_VOID(BAR_CALL_FUNCTION);
#else
    return -1;
#endif
}
EXPORT_SYMBOL(call_bar_function);
