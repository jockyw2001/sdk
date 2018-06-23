#ifndef _MI_FOO_H_
#define _MI_FOO_H_
#include "mi_common.h"
MI_S32 call_foo_function(int arg1, int arg2, int *rval);
MI_S32 foo_openpollfd(int *fd, int i);
MI_S32 foo_wake(int i);
MI_S32 foo_ack(int i);
#endif
