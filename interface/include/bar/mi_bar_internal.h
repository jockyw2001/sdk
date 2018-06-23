#ifndef _MI_BAR_INTERNAL_H_
#define _MI_BAR_INTERNAL_H_
typedef enum {
#if INTERFACE_FOO == 1
    BAR_NR_CALL_FUNCTION,
#endif
    BAR_NR,
} BAR_FUNCTION_E;

#include <linux/ioctl.h>
#if INTERFACE_FOO == 1
#define BAR_CALL_FUNCTION _IO('i', BAR_NR_CALL_FUNCTION)
#endif
#endif /* _MI_BAR_INTERNAL_H_ */
