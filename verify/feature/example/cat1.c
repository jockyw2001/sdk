#include "cat1.h"
#if INTERFACE_FOO == 1
#include "mi_foo.h"
#endif

int cat1(int a, int b){
    int c;
    cat_lib();
#if INTERFACE_FOO == 1
    call_foo_function(a, b, &c);
#endif
    return c;
}
