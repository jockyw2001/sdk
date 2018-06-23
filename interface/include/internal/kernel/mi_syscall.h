#ifndef _MI_SYSCALL_H_
#define _MI_SYSCALL_H_
#include <linux/module.h>
#include "mi_common.h"
#include "mi_common_macro.h"
#include "mi_common_internal.h"

#define MI_MODULE_DEFINE(name)

#define MI_SYSCALL(_cmd, _p) ({ \
		typedef MI_S32 (*MI_DEVICE_WrapperFunction_t)(MI_DEVICE_Context_t *, void*); \
		extern MI_DEVICE_WrapperFunction_t MI_DEVICE_WrapperFunction(int); \
        MI_S32 _rval = 0; \
        MI_DEVICE_Context_t _env = { \
                .eFrom = E_MI_COMMON_CALL_FROM_API,         \
        }; \
        MI_DEVICE_WrapperFunction_t _func = MI_DEVICE_WrapperFunction(_cmd); \
		_rval = _func(&_env, _p); \
        if(_env.pstMutex){ \
            mutex_unlock(_env.pstMutex); \
        } \
        _rval; \
    })

#define MI_SYSCALL_VOID(_cmd) ({ \
		typedef MI_S32 (*MI_DEVICE_WrapperFunction_t)(MI_DEVICE_Context_t *, void *); \
		extern MI_DEVICE_WrapperFunction_t MI_DEVICE_WrapperFunction(int); \
        MI_S32 _rval = 0; \
        MI_DEVICE_Context_t _env = { \
                .eFrom = E_MI_COMMON_CALL_FROM_API,         \
        }; \
        MI_DEVICE_WrapperFunction_t _func = MI_DEVICE_WrapperFunction(_cmd); \
		_rval = _func(&_env, NULL); \
        if(_env.pstMutex){ \
            mutex_unlock(_env.pstMutex); \
        } \
        _rval; \
    })
#endif /* _MI_SYSCALL_H_ */
