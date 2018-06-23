#ifndef _MI_SYS_LOG_H_
#define _MI_SYS_LOG_H_

#include "mi_sys_internal.h"
#include "mi_sys_datatype.h"
#include "mi_common.h"
#include "mi_common_macro.h"


#define DEBUG_MSG_BUF_SIZE   (1024 * 4)

#define MSG_FROM_USER   0
#define MSG_FROM_KERNEL 1
#define MI_LOG_LEVEL_DEFAULT E_MI_ERR_LEVEL_ERROR

typedef struct MI_SYS_LogBufferInfo_S
{
    MI_PHY phyStartAddrPhy; /*start physic address*/    /*CNcomment:»º³åÇøÆðÊ¼ÎïÀíµØÖ·*/
    MI_U8 *pu8StartAddrVir; /*start virtual address*/    /*CNcomment:»º³åÇøÆðÊ¼ÐéÄâµØÖ·*/
    MI_U32 u32BufSize;      /*buffer size*/                /*CNcomment:»º³åÇø´óÐ¡*/
    MI_U32 u32WriteAddr;    /*write offset*/            /*CNcomment:Ð´µØÖ·Æ«ÒÆ*/
    MI_U32 u32ReadAddr;     /*read offset*/                /*CNcomment:¶ÁµØÖ·Æ«ÒÆ*/
    MI_U32 u32ResetFlag;    /*reset count*/                /*CNcomment:¸´Î»´ÎÊý*/
    MI_U32 u32WriteCount;   /*write count*/                /*CNcomment:Ð´Èë´ÎÊý*/
    wait_queue_head_t wqNoData;    /*no wait queque*/    /*CNcomment:Ã»ÓÐÊý¾ÝµÈ´ý¶ÓÁÐ*/
    struct semaphore semWrite;     /*write semaphore*/    /*CNcomment:Ð´bufferÐÅºÅÁ¿*/
}MI_SYS_LogBufferInfo_t;

typedef enum
{
    E_MI_SYS_LOG_OUTPUT_SERIAL = 0,
    E_MI_SYS_LOG_OUTPUT_NETWORK,
    E_MI_SYS_LOG_OUTPUT_UDISK,
}MI_SYS_LogOutputPos_e;


#define LOG_OUTPUT_POS_DEFAULT (E_MI_SYS_LOG_OUTPUT_SERIAL)

/*structure of mode log level */
/*CNcomment: Ä£¿é´òÓ¡¼¶±ð¿ØÖÆÐÅÏ¢½á¹¹ */
typedef struct MI_SYS_LogConfigInfo_S
{
    MI_U8 u8ModName[16+12];     /*mode name 16 + '_' 1 + pid 10 */
    MI_DBG_LEVEL_e eLogLevel;    /*log level*//*CNcomment:  Ä£¿é´òÓ¡¼¶±ð¿ØÖÆ */
    MI_SYS_LogOutputPos_e eLogPrintPos;      /*log output location, 0:serial port; 1:network;2:u-disk*//*CNcomment:  Ä£¿é´òÓ¡Î»ÖÃ¿ØÖÆ 0:´®¿Ú 1:ÍøÂç 2:UÅÌ */
    MI_U8 u8UdiskFlag;        /* u-disk log flag */
    MI_U8 reserved;    /* u-disk log flag */
}MI_SYS_LogConfigInfo_t;

typedef struct MI_SYS_LogBufRead_S
{
    MI_PHY  pHyAddr;
    MI_U32  u32BufLen;
    MI_U32  u32CopyedLen;
}MI_SYS_LogBufRead_t;

typedef struct MI_SYS_LogBufWrite_S
{
    MI_PHY  pHyAddr;
    MI_U32  u32BufLen;
}MI_SYS_LogBufWrite_t;

int MI_SYS_LOG_IMPL_ProcRead(struct seq_file *s, void *pArg);
    
ssize_t MI_SYS_LOG_IMPL_ProcWrite( struct file * file,  const char __user * buf,
                         size_t count, loff_t *ppos);

MI_S32 MI_SYS_LOG_IMPL_Init(void);

MI_S32 MI_SYS_LOG_IMPL_Exit(void);

void MI_SYS_LOG_IMPL_PrintLog(const char *fmt, ...);

#endif
