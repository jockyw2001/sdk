#ifndef DRVOPTEE_PRIVATE_H
#define DRVOPTEE_PRIVATE_H

#include "ULog.h"
#include "MsCommon.h"

#if defined (MSOS_TYPE_LINUX)
#include "tee_client_api.h"
int get_optee_version(void);
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/types.h>
#endif

MS_U32 _MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context);
MS_U32 _MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin);
void _MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context);
void _MDrv_SYS_TEEC_Close(TEEC_Session *session);
MS_U32 _MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin);

#endif
