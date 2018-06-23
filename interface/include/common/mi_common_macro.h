#ifndef _MI_COMMON_MACRO_H_
#define _MI_COMMON_MACRO_H_

#if 0
#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/delay.h>
#define mprint(fmt, arg...) printk(KERN_ERR "<kernel macro>" fmt, ##arg)
#define mwait(ms) msleep(ms)
#else
#include <string.h>
#include <unistd.h>
#define mprint(fmt, arg...) printf("<app macro>" fmt, ##arg)
#define mwait(ms) usleep(ms*1000)
#endif
#else
#define mprint(fmt, arg...)
#define mwait(ms)
#endif

#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)

///ASCII color code
#define ASCII_COLOR_RED                          "\033[1;31m"
#define ASCII_COLOR_WHITE                        "\033[1;37m"
#define ASCII_COLOR_YELLOW                       "\033[1;33m"
#define ASCII_COLOR_BLUE                         "\033[1;36m"
#define ASCII_COLOR_GREEN                        "\033[1;32m"
#define ASCII_COLOR_END                          "\033[0m"

typedef enum
{
    MI_DBG_NONE = 0,
    MI_DBG_ERR,
    MI_DBG_WRN,
    MI_DBG_API,
    MI_DBG_INFO,
    MI_DBG_ALL
}MI_DBG_LEVEL_e;

#endif /* _MI_COMMON_MACRO_H_ */
