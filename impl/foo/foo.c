#include "foo.h"
#include "mi_print.h"
//#include "hal.h"

MI_S32 mi_call_foo_function(int a, int b, int *rval){
    DBG_INFO("<%s> %d, %d\n", __func__, a, b);
//    *rval = hal_add(a, b);
    return 0;
}
