#ifndef _MI_COMMON_INTERNEL_H_
#define _MI_COMMON_INTERNEL_H_
#include "mi_common_macro.h"

typedef enum {
    E_MI_COMMON_CALL_FROM_API,
    E_MI_COMMON_CALL_FROM_IOCTL,
} MI_COMMON_CallFrom_e;

typedef struct MI_COMMON_Client_s {
    struct MI_DEVICE_Object_s *pstDevice;
    int pid;
    int tid;
    void *private;
} MI_COMMON_Client_t;

typedef struct MI_DEVICE_Context_s {
    const char *strModuleName;
    MI_COMMON_CallFrom_e eFrom;
    struct mutex *pstMutex;
    MI_COMMON_Client_t *pstClient;
} MI_DEVICE_Context_t;

#endif /* _MI_COMMON_INTERNEL_H_ */
