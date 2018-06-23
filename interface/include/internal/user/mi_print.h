#ifndef _MI_PRINT_H_
#define _MI_PRINT_H_
#include "mi_common_macro.h"

MI_DBG_LEVEL_e _MI_PRINT_GetDebugLevel(const char *name);

#if MI_DBG
// mdules who want to use DBG_INFO for debugging should define DBG_LEVEL by itself.
#define MI_PRINT(fmt, args...)      ({do{printf(fmt, ##args);}while(0);})
#define DBG_INFO(fmt, args...)      ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_INFO){printf(ASCII_COLOR_GREEN"[MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_WRN(fmt, args...)       ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_WRN){printf(ASCII_COLOR_YELLOW"[MI WRN ]: %s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_ERR(fmt, args...)       ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_ERR){printf(ASCII_COLOR_RED"[MI ERR ]: %s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_ENTER(fmt, args...)     ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_API){printf(ASCII_COLOR_BLUE">>>%s[%d] \n",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_EXIT_OK(fmt, args...)   ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_API){printf(ASCII_COLOR_BLUE"<<<%s[%d] \n",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_EXIT_ERR(fmt, args...)  ({do{if(_MI_PRINT_GetDebugLevel(MACRO_TO_STRING(EXTRA_MODULE_NAME))>=MI_DBG_ERR){printf(ASCII_COLOR_RED"<<<%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
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

#ifndef __BIONIC__
#define BACKTRACE(_size_) \
	do {	\
		int j = 0, nptrs = 0;	\
		void *buffer[_size_];	\
		char **strings;	\
		nptrs = backtrace(buffer, _size_);	\
		strings = backtrace_symbols(buffer, nptrs);	\
		for (j = 0; j < nptrs; j++)	\
		printf("[Backtrace] %s\n", strings[j]);	\
		free(strings);	\
		} while(0)
#else
#define BACKTRACE(_size_)
#endif

#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            *(int*)0 = 0;                                                                        \
        }                                                                                   \
    } while (0)
#endif

#define CHECK_TIME 30 //ms

#define FUNCTION_IN_TIME      struct  timeval in_time;gettimeofday(&in_time,NULL);

#define RETURN(ret) \
    do{\
        struct  timeval out_time;\
        gettimeofday(&out_time,NULL);\
        int timecost = (out_time.tv_sec - in_time.tv_sec)*1000 + (out_time.tv_usec -in_time.tv_usec)/1000;\
        printf("[Warning][Function] %s cost time %d ms!!!\n" , __FUNCTION__ ,timecost);\
        return ret;\
    }while(0)

#endif /* _MI_PRINT_H_ */
