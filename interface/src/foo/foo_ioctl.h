#ifndef _MI_FOO_INTERNAL_H_
#define _MI_FOO_INTERNAL_H_
typedef enum {
    FOO_NR_CALL_FUNCTION,
    FOO_NR_SETUP_POLLFD,
    FOO_NR_POLLFD_WAKE,
    FOO_NR_POLLFD_ACK,
    FOO_NR,
} FOO_FUNCTION_E;

#include <linux/ioctl.h>

typedef struct {
    int arg1;
    int arg2;
    int rval;
} mi_foo_call_foo_function_t;
#define FOO_CALL_FUNCTION _IOWR('i', FOO_NR_CALL_FUNCTION, mi_foo_call_foo_function_t)
typedef struct {
    int i;
    int fd;
} mi_foo_openpollfd_t;
#define FOO_SETUP_POLLFD _IOWR('i', FOO_NR_SETUP_POLLFD, mi_foo_openpollfd_t)
typedef struct {
    int i;
} mi_foo_poll_id_t;
#define FOO_POLLFD_WAKE _IOW('i', FOO_NR_POLLFD_WAKE, mi_foo_poll_id_t)
#define FOO_POLLFD_ACK _IOW('i', FOO_NR_POLLFD_ACK, mi_foo_poll_id_t)

#endif /* _MI_FOO_INTERNAL_H_ */
