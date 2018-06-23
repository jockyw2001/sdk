#ifndef _MI_PRINT_H_
#define _MI_PRINT_H_
#include "mi_common.h"
#include "mi_common_macro.h"


void MI_SYS_LOG_IMPL_PrintLog(const char *fmt, ...);

MI_DBG_LEVEL_e MI_DEVICE_GetDebugLevel(void);

MI_S32 MI_COMMON_Print(const char *fmt, ...);
MI_S32 MI_COMMON_Info(const char *fmt, ...);
MI_S32 MI_COMMON_Warning(const char *fmt, ...);
MI_S32 MI_COMMON_Error(const char *fmt, ...);

#define _CHECKPOINT_ DBG_INFO("==== check point ====\n");

#if MI_DBG
#define MI_PRINT(fmt,args...)      ({do{MI_COMMON_Print(fmt, ##args);}while(0);})
#define DBG_INFO(fmt, args...)     ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_INFO){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_GREEN"[MI INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__,##args);}}while(0);})
#define DBG_WRN(fmt, args...)      ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_WRN){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_YELLOW"[MI WRN ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})
#define DBG_ERR(fmt, args...)      ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_ERR){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"[MI ERR ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})
#define DBG_ENTER(fmt, args...)    ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_API){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_BLUE">>>%s[%d] \n" fmt ASCII_COLOR_END,__FUNCTION__,__LINE__,##args);}}while(0);})
#define DBG_EXIT_OK(fmt, args...)  ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_API){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_BLUE"<<<%s[%d] \n" fmt ASCII_COLOR_END,__FUNCTION__,__LINE__,##args);}}while(0);})
#define DBG_EXIT_ERR(fmt, args...) ({do{if(MI_DEVICE_GetDebugLevel()>=MI_DBG_ERR){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED"<<<%s[%d] " fmt ASCII_COLOR_END,__FUNCTION__,__LINE__,##args);}}while(0);})
#else
#define MI_PRINT(fmt, args...)
#define DBG_INFO(fmt, args...)
#define DBG_PPID_INFO(fmt, args...)
#define DBG_WRN(fmt, args...)
#define DBG_ERR(fmt, args...)
#define DBG_ENTER(fmt, args...)
#define DBG_EXIT_OK(fmt, args...)
#define DBG_EXIT_ERR(fmt, args...)
#endif

#endif /* _MI_PRINT_H_ */
