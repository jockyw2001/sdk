#include "bar_impl.h"
#include "mi_print.h"

MI_S32 impl_call_bar_function(MI_DEVICE_Context_t *env, void *ptr){
    MI_S32 rval = -1;
    //call_foo_function(104, 755, &rval);
    DBG_ERR("<%s> %d %d\n", __func__, env->eFrom, rval);
    return rval;
}
