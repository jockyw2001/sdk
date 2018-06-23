//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "mi_sys_log_impl.h"
#include "mi_print.h"
typedef struct MI_SYS_LogBuffer_s
{
    void* pStartVirAddr;
    MI_PHY phyStartPhyAddr;
    MI_U32 u32Size;
}MI_SYS_LogBuffer_t;


char *DebugLevelName[MI_DBG_ALL+1] = {
    "NONE",
    "ERR",
    "WRN",
    "INFO",
    "ALL",
};

                
#define MAX_FILENAME_LENTH 256

static MI_SYS_LogConfigInfo_t *_gpstLogConfigInfo = NULL;
static MI_SYS_LogBufferInfo_t _gstLogBufferInfo;
static MI_SYS_LogBuffer_t _gstLogBuffer;
static struct task_struct *gpLogUdiskTask = NULL;
static MI_U8 _gu8LogInit = 0;
static MI_BOOL _gbSetLogFileFlag = FALSE;
static char g_szPathBuf[MAX_FILENAME_LENTH] = {0};
static char *UdiskLogFile = g_szPathBuf;
static char g_szStorePathBuf[MAX_FILENAME_LENTH] = "/mnt";
char *StorePath = g_szStorePathBuf;


DEFINE_SEMAPHORE(_gLogFileMutex);
DEFINE_SPINLOCK(_gLogFileLock);

#define LOG_FILE_LOCK()   down_interruptible(&_gLogFileMutex)
#define LOG_FILE_UNLOCK() up(&_gLogFileMutex)
#define LOG_MAX_TRACE_LEN 256
#define LOG_CONFIG_BUF_SIZE   (sizeof(MI_SYS_LogConfigInfo_t) * E_MI_MODULE_ID_MAX)

struct file* FileOpen(const MI_S8* ps8FileName, MI_S32 s32Flags)
{
    struct file *pFile = NULL;

    if (NULL == ps8FileName)
    {
        return NULL;
    }

    if (s32Flags == 0)
    {
        s32Flags = O_RDONLY;
    }
    else
    {
        s32Flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    pFile = filp_open(ps8FileName, s32Flags | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    return (IS_ERR(pFile)) ? NULL : pFile;
}


void FileClose(struct file * pFile)
{
    if ( NULL != pFile )
    {
        filp_close(pFile, NULL);
    }

    return;
}

MI_S32 FileRead(struct file * pFile,  MI_U8* ps8Buf, MI_U32 u32Len)
{
    MI_S32 s32ReadLen = 0;
    mm_segment_t stOld_FS = {0};

    if (pFile == NULL || NULL == ps8Buf)
    {
        return -ENOENT; /* No such file or directory */
    }

    if (pFile->f_op->read == NULL)
    {
        return -ENOSYS; /* Function not implemented */
    }

    if (((pFile->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) != 0)
    {
        return -EACCES; /* Permission denied */
    }

    /* saved the original file space */
    stOld_FS = get_fs();

    /* extend to the kernel data space */
    set_fs(KERNEL_DS);

    s32ReadLen = pFile->f_op->read(pFile, ps8Buf, u32Len, &pFile->f_pos);

    /* Restore the original file space */
    set_fs(stOld_FS);

    return s32ReadLen;
}


MI_S32 FileWrite(struct file* pFile, MI_S8* ps8Buf, MI_U32 u32Len , MI_BOOL bFlag)
{
    MI_S32 s32WriteLen = 0;
    mm_segment_t stOld_FS = {0};

    if (pFile == NULL || ps8Buf == NULL)
    {
        return -ENOENT; /* No such file or directory */
    }

    if (pFile->f_op->write == NULL)
    {
        return -ENOSYS; /* Function not implemented */
    }

    if (((pFile->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
    {
        return -EACCES; /* Permission denied */
    }

    stOld_FS = get_fs();
    set_fs(KERNEL_DS);
    if(bFlag)
        pFile->f_op->llseek(pFile, 0, SEEK_SET);
    
    s32WriteLen = pFile->f_op->write(pFile, ps8Buf, u32Len, &pFile->f_pos);

    set_fs(stOld_FS);

    return s32WriteLen;
}


MI_S32 FileLseek(struct file *pFile, MI_S32 s32Offset, MI_S32 s32Whence)
{
    MI_S32 s32Ret;

    loff_t res = vfs_llseek(pFile, s32Offset, s32Whence);
    s32Ret = res;
    if (res != (loff_t)s32Ret)
        s32Ret = -EOVERFLOW;

    return s32Ret;
}

static int SeperateString(char *s, char **left, char **right)
{
    char *p = s;
    /* find '=' */
    while(*p != '\0' && *p++ != '=');

    if (*--p != '=')
        return -1;

    /* seperate left from right vaule by '=' */
    *p = '\0';
    *left = s;
    *right = p + 1;
    return 0;
}

static int SearchMod(char *s)
{
    int i= 0;
    int cnt = (int)E_MI_MODULE_ID_MAX;

    for (i = 0; i < cnt; i++){
        if (!strncasecmp(_gpstLogConfigInfo[i].u8ModName, s, sizeof(_gpstLogConfigInfo[i].u8ModName)))
            return i;
    }
    return -1;
}

static char *StripString(char *s, char *d)
{
    char *p = d;
    do{
        if (*s == '\n')
            *s = '\0';
        if (*s != ' ')
            *p++ = *s;
    }while(*s++ != '\0');
    return d;
}

static char *strlwr(char *s)
{
    char *str;
    str = s;
    while(*str != '\0')
    {
        if(*str >= 'A' && *str <= 'Z')
        {
            *str += 'a'-'A';
        }
        str++;
    }
    return s;
}

MI_DBG_LEVEL_e GetModuleLevel(MI_U8* szProcName)
{
    MI_DBG_LEVEL_e eLevel = MI_LOG_LEVEL_DEFAULT;
    char path[256];
    struct file *pFile = NULL;
    MI_S8 s8buf = 0;
    
    snprintf(path, sizeof(path), "/sys/module/%s/parameters/debug_level", strlwr(szProcName));
    
    pFile = FileOpen(path , 1);
    if(pFile)
    {
        FileRead(pFile , &s8buf , 1);
        FileClose(pFile);
        eLevel = (MI_DBG_LEVEL_e)(s8buf - 48);
    }
    else
    {
        eLevel = MI_LOG_LEVEL_DEFAULT;
    }
    return eLevel;
}

MI_S32 SetModuleLevel(MI_U8* szProcName , int level)
{
    char path[256];
    struct file *pFile = NULL;
    MI_S8 s8buf = 0;

    snprintf(path, sizeof(path), "/sys/module/%s/parameters/debug_level", strlwr(szProcName));
    pFile = FileOpen(path , 1);
    if(pFile)
    {
        s8buf = 48 + level;
        FileWrite(pFile , &s8buf , 1 , 1);
        FileClose(pFile);
        return 0;
    }
    return -1;
}

MI_S32 MI_SYS_LOG_IMPL_Snprintf(MI_U8 *pu8Str, MI_U32 u32Len, const MI_U8 *pszFormat, ...)
{
    MI_S32 s32Len = 0;
    va_list stArgs = {0};

    va_start(stArgs, pszFormat);
    s32Len = vsnprintf(pu8Str, u32Len, pszFormat, stArgs);
    va_end(stArgs);

    return s32Len;
}

MI_S32 MI_SYS_LOG_IMPL_Vnprintf(MI_U8 *pszStr, MI_U32 ulLen, const MI_U8 *pszFormat, va_list stVAList)
{
    return vsnprintf(pszStr, ulLen, pszFormat, stVAList);
}


MI_S32 _MI_SYS_LOG_IMPL_ConfigInfoInit(void)
{
    int ModuleIdx = 0;
    if(MI_SUCCESS != mi_sys_MMA_Alloc(NULL,LOG_CONFIG_BUF_SIZE,&_gstLogBuffer.phyStartPhyAddr))
    {
       DBG_ERR("mma alloc fail\n");
       return MI_ERR_SYS_FAILED;
    }

    _gstLogBuffer.pStartVirAddr = mi_sys_Vmap(_gstLogBuffer.phyStartPhyAddr,LOG_CONFIG_BUF_SIZE,FALSE);
    if(!_gstLogBuffer.pStartVirAddr)
    {
       DBG_ERR("call mi_sys_Vmap fail\n");
       return MI_ERR_SYS_FAILED;
    }

    memset(_gstLogBuffer.pStartVirAddr , 0 , LOG_CONFIG_BUF_SIZE);

    _gpstLogConfigInfo = (MI_SYS_LogConfigInfo_t *)_gstLogBuffer.pStartVirAddr;

    for(ModuleIdx = 0 ; ModuleIdx < (int)E_MI_MODULE_ID_MAX ; ModuleIdx ++)
    {
       _gpstLogConfigInfo[ModuleIdx].eLogLevel = MI_LOG_LEVEL_DEFAULT;
       _gpstLogConfigInfo[ModuleIdx].eLogPrintPos = E_MI_SYS_LOG_OUTPUT_SERIAL;
       MI_SYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[ModuleIdx].u8ModName, sizeof(_gpstLogConfigInfo[ModuleIdx].u8ModName), "modulemax");
    }
    
    return MI_SUCCESS;
}

MI_S32 _MI_SYS_LOG_IMPL_LogBufferInit(void)
{
    memset(&_gstLogBufferInfo ,  0 , sizeof(_gstLogBufferInfo));

    _gstLogBufferInfo.u32BufSize = 256 * DEBUG_MSG_BUF_SIZE;

    init_waitqueue_head(&(_gstLogBufferInfo.wqNoData));
    sema_init(&_gstLogBufferInfo.semWrite, 1);
    if(MI_SUCCESS != mi_sys_MMA_Alloc(NULL,256 * DEBUG_MSG_BUF_SIZE,&_gstLogBufferInfo.phyStartAddrPhy))
    {
        DBG_ERR("mma alloc fail\n");
        return MI_ERR_SYS_FAILED;
    }

    _gstLogBufferInfo.pu8StartAddrVir = mi_sys_Vmap(_gstLogBufferInfo.phyStartAddrPhy,256 * DEBUG_MSG_BUF_SIZE ,FALSE);
    if(!_gstLogBufferInfo.pu8StartAddrVir)
    {
        DBG_ERR("call mi_sys_Vmap fail\n");
        return MI_ERR_SYS_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 LogUdiskSave(const MI_S8* pFileName, MI_S8* pData, MI_U32 u32DataLen)
{
    MI_S32 s32WriteLen = 0;
    struct file* pFile = NULL;

    pFile = FileOpen(pFileName, 1);
    if(pFile == NULL)
    {
        DBG_ERR("FileOpen %s failure..............\n", pFileName);
        return MI_ERR_SYS_FAILED;
    }

    s32WriteLen = FileWrite(pFile, pData, u32DataLen , FALSE);

    FileClose(pFile);

    return MI_SUCCESS;
}

static void LOGBufferReset(void)
{
    unsigned long flags;

    spin_lock_irqsave(&_gLogFileLock, flags);
    _gstLogBufferInfo.u32ReadAddr = _gstLogBufferInfo.u32WriteAddr;
    _gstLogBufferInfo.u32ResetFlag++;
    spin_unlock_irqrestore(&_gLogFileLock, flags);
}

static MI_SYS_LogOutputPos_e GetLogPrintMode(void)
{
    MI_SYS_LogOutputPos_e ePos = 0;

    if (0 == _gu8LogInit)
    {
        return E_MI_SYS_LOG_OUTPUT_SERIAL;
    }
    if (_gbSetLogFileFlag == TRUE)
    {
       ePos = E_MI_SYS_LOG_OUTPUT_UDISK;
    }
    else
        ePos = E_MI_SYS_LOG_OUTPUT_SERIAL;

    return ePos;
}
inline MI_U32 GetTimeMs(void)
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return (((MI_U32)tv.tv_sec)*1000 + ((MI_U32)tv.tv_usec)/1000);
}

extern MI_S32 MI_SYS_LOG_IMPL_WriteBuf(MI_U8 *pu8Buf,  MI_U32 u32MsgLen, MI_U32 u32UserOrKer);

static MI_S32 MI_SYS_LOG_IMPL_PrintLogToBuf(const char *format, ...)
{
    char    log_str[LOG_MAX_TRACE_LEN] = {0};
    MI_U32  MsgLen = 0;
    va_list args;

    if (0 == _gu8LogInit)
    {
        DBG_ERR("log device not init!\n");
        return MI_ERR_SYS_FAILED;
    }

    va_start(args, format);
    MsgLen = MI_SYS_LOG_IMPL_Vnprintf(log_str, LOG_MAX_TRACE_LEN-1, format, args);
    va_end(args);

    if (MsgLen >= (LOG_MAX_TRACE_LEN-1))
    {
        log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        log_str[LOG_MAX_TRACE_LEN-2] = '\n';
        log_str[LOG_MAX_TRACE_LEN-3] = '.';
        log_str[LOG_MAX_TRACE_LEN-4] = '.';
        log_str[LOG_MAX_TRACE_LEN-5] = '.';
    }

    return MI_SYS_LOG_IMPL_WriteBuf((MI_U8 *)log_str, MsgLen, MSG_FROM_KERNEL);
}



void MI_SYS_LOG_IMPL_PrintLog(const char *format, ...)
{
    va_list args;
    MI_U32  u32MsgLen = 0;
    char    log_str[LOG_MAX_TRACE_LEN]={'a'};

    {
        log_str[LOG_MAX_TRACE_LEN-1] = 'b';
        log_str[LOG_MAX_TRACE_LEN-2] = 'c';

        va_start(args, format);
        u32MsgLen = MI_SYS_LOG_IMPL_Vnprintf(log_str, LOG_MAX_TRACE_LEN, format, args);
        va_end(args);

        if (u32MsgLen >= LOG_MAX_TRACE_LEN)
        {
            log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
            log_str[LOG_MAX_TRACE_LEN-2] = '\n';
            log_str[LOG_MAX_TRACE_LEN-3] = '.';
            log_str[LOG_MAX_TRACE_LEN-4] = '.';
            log_str[LOG_MAX_TRACE_LEN-5] = '.';
        }
        /* log module has Initialized. */
        if (_gu8LogInit)
        {
            MI_SYS_LogOutputPos_e ePos = GetLogPrintMode();
            
            switch(ePos)
            {
                case E_MI_SYS_LOG_OUTPUT_SERIAL:
                     MI_PRINT(log_str);
                    break;
                case E_MI_SYS_LOG_OUTPUT_NETWORK:
                case E_MI_SYS_LOG_OUTPUT_UDISK:
                    MI_SYS_LOG_IMPL_PrintLogToBuf(log_str);
                    break;
            }
        }
        else /* log module has not Initialized. */
        {
            MI_PRINT(log_str);
        }
    }

}

EXPORT_SYMBOL(MI_SYS_LOG_IMPL_PrintLog);

MI_S32 MI_SYS_LOG_IMPL_WriteBuf(MI_U8 *pu8Buf,  MI_U32 u32MsgLen, MI_U32 u32UserOrKer)
{
    MI_U32 u32CopyLen1;
    MI_U32 u32CopyLen2;
    MI_U32 u32NewWriteAddr;
    
    if (0 == _gstLogBufferInfo.u32BufSize)
    {
        return MI_SUCCESS;
    }
    down(&_gstLogBufferInfo.semWrite);
    if(_gstLogBufferInfo.u32WriteAddr < _gstLogBufferInfo.u32ReadAddr)
    {
        if ((_gstLogBufferInfo.u32ReadAddr - _gstLogBufferInfo.u32WriteAddr)
              < DEBUG_MSG_BUF_SIZE)
        {
            LOGBufferReset();
        }
    }
    else
    {
        if ((_gstLogBufferInfo.u32WriteAddr - _gstLogBufferInfo.u32ReadAddr)
              > (_gstLogBufferInfo.u32BufSize - DEBUG_MSG_BUF_SIZE))
        {
            LOGBufferReset();
        }
    }

    if ((u32MsgLen + _gstLogBufferInfo.u32WriteAddr) >= _gstLogBufferInfo.u32BufSize)
    {
        u32CopyLen1 = _gstLogBufferInfo.u32BufSize - _gstLogBufferInfo.u32WriteAddr;
        u32CopyLen2 = u32MsgLen - u32CopyLen1;
        u32NewWriteAddr = u32CopyLen2;
    }
    else
    {
        u32CopyLen1 = u32MsgLen;
        u32CopyLen2 = 0;
        u32NewWriteAddr = u32MsgLen + _gstLogBufferInfo.u32WriteAddr;
    }

    if(u32CopyLen1 > 0)
    {
        if(MSG_FROM_KERNEL == u32UserOrKer)
        {
            memcpy((_gstLogBufferInfo.pu8StartAddrVir + _gstLogBufferInfo.u32WriteAddr),pu8Buf, u32CopyLen1);
        }
        else
        {
            if(copy_from_user((_gstLogBufferInfo.u32WriteAddr+_gstLogBufferInfo.pu8StartAddrVir),
                    pu8Buf, u32CopyLen1))
            {
                DBG_ERR("copy_from_user error\n");
            }
        }
    }
    if(u32CopyLen2 > 0)
    {
        if(MSG_FROM_KERNEL == u32UserOrKer)
        {
            memcpy(_gstLogBufferInfo.pu8StartAddrVir, (pu8Buf + u32CopyLen1), u32CopyLen2);
        }
        else
        {
            if(copy_from_user(_gstLogBufferInfo.pu8StartAddrVir,
                    (pu8Buf + u32CopyLen1), u32CopyLen2))
            {
                DBG_ERR("copy_from_user error\n");
            }
        }
    }

    _gstLogBufferInfo.u32WriteAddr = u32NewWriteAddr;

    if (_gbSetLogFileFlag != TRUE)
    {
        wake_up_interruptible(&_gstLogBufferInfo.wqNoData);
    }
    up(&_gstLogBufferInfo.semWrite);

    return MI_SUCCESS;
}


MI_S32 MI_SYS_LOG_IMPL_ReadBuf(MI_U8 *pu8Buf,  MI_U32 u32BufLen, MI_U32 *pu32CopyLen, MI_BOOL bKernelCopy)
{
    MI_SYS_LogBufferInfo_t stCurryMsgInfo;
    MI_U32 u32BufUsedLen;
    MI_U32 u32DataLen1;
    MI_U32 u32DataLen2;
    MI_U32 u32CopyLen;
    MI_U32 u32NewReadAddr;
    unsigned long flags;

    if (0 == _gstLogBufferInfo.u32BufSize)
    {
        DBG_ERR("Log Buffer size is 0, Please confige the Buffer size\n");
        return MI_ERR_SYS_FAILED;
    }

    if(_gstLogBufferInfo.u32WriteAddr == _gstLogBufferInfo.u32ReadAddr)
    {
        if (_gbSetLogFileFlag == TRUE)
        {
            return MI_ERR_SYS_FAILED;
        }
        else
        {
            wait_event_interruptible(_gstLogBufferInfo.wqNoData,
                    (_gstLogBufferInfo.u32WriteAddr != _gstLogBufferInfo.u32ReadAddr));
        }
    }

    spin_lock_irqsave(&_gLogFileLock, flags);
    memcpy(&stCurryMsgInfo,  &_gstLogBufferInfo, sizeof(_gstLogBufferInfo));
    spin_unlock_irqrestore(&_gLogFileLock, flags);

    if(stCurryMsgInfo.u32WriteAddr < stCurryMsgInfo.u32ReadAddr)
    {
        u32BufUsedLen = stCurryMsgInfo.u32BufSize - stCurryMsgInfo.u32ReadAddr
                    + stCurryMsgInfo.u32WriteAddr;
        u32DataLen1 =  stCurryMsgInfo.u32BufSize - stCurryMsgInfo.u32ReadAddr;
        u32DataLen2 = stCurryMsgInfo.u32WriteAddr;
    }
    else
    {
        u32BufUsedLen = stCurryMsgInfo.u32WriteAddr - stCurryMsgInfo.u32ReadAddr;
        u32DataLen1 = u32BufUsedLen;
        u32DataLen2 = 0;
    }

    if (u32BufLen <= (u32DataLen1 + u32DataLen2))
    {
        u32CopyLen = u32BufLen;
    }
    else
    {
        u32CopyLen = u32DataLen1 + u32DataLen2;
    }

    if (u32DataLen1 >= u32CopyLen)
    {
        if (bKernelCopy == FALSE)
        {
            if(copy_to_user(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32CopyLen))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32CopyLen);
        }

        u32NewReadAddr = stCurryMsgInfo.u32ReadAddr + u32CopyLen;
    }
    else
    {
        if (bKernelCopy == FALSE)
        {
            if(copy_to_user(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32DataLen1))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32DataLen1);
        }

        if (bKernelCopy == FALSE)
        {
            if(copy_to_user((pu8Buf + u32DataLen1), stCurryMsgInfo.pu8StartAddrVir, (u32CopyLen - u32DataLen1)))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy((pu8Buf + u32DataLen1), stCurryMsgInfo.pu8StartAddrVir, (u32CopyLen - u32DataLen1));
        }

        u32NewReadAddr = u32CopyLen - u32DataLen1;
    }

    *pu32CopyLen = u32CopyLen;

    if (u32NewReadAddr >= stCurryMsgInfo.u32BufSize)
        u32NewReadAddr = 0;

    spin_lock_irqsave(&_gLogFileLock, flags);
    if (stCurryMsgInfo.u32ResetFlag == _gstLogBufferInfo.u32ResetFlag)
    {
        _gstLogBufferInfo.u32ReadAddr = u32NewReadAddr;
    }
    spin_unlock_irqrestore(&_gLogFileLock, flags);

    return MI_SUCCESS;
}

MI_S32 MI_SYS_LOG_IMPL_LogAddModule(const MI_U8* szProcName, MI_ModuleId_e eModuleID)
{
    if (NULL == _gpstLogConfigInfo || eModuleID >= E_MI_MODULE_ID_MAX)
    {
        return MI_ERR_SYS_FAILED;
    }
    
    MI_SYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[eModuleID].u8ModName, sizeof(_gpstLogConfigInfo[eModuleID].u8ModName),"%s", szProcName);
    
    _gpstLogConfigInfo[eModuleID].eLogLevel = GetModuleLevel(strlwr(_gpstLogConfigInfo[eModuleID].u8ModName));
    
    return MI_SUCCESS;
}

MI_S32 MI_SYS_LOG_IMPL_LogRemoveModule(const MI_U8* szProcName, MI_ModuleId_e eModuleID)
{

    if (NULL == _gpstLogConfigInfo || eModuleID >= E_MI_MODULE_ID_MAX)
    {
        return MI_ERR_SYS_FAILED;
    }

    _gpstLogConfigInfo[eModuleID].eLogLevel = E_MI_ERR_LEVEL_ERROR;
    MI_SYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[eModuleID].u8ModName, sizeof(_gpstLogConfigInfo[eModuleID].u8ModName), "ModuleMax");

    return MI_SUCCESS;
}

extern void _MI_SYS_IMPL_ModuleIdToPrefixName(MI_ModuleId_e eModuleId , char *prefix_name);
extern MI_S32 _MI_SYS_LOG_IMPL_LogUdiskInit(const MI_U8* pUdiskFolder);
MI_S32 MI_SYS_LOG_IMPL_Init(void)
{
    MI_U32 u32ModuleId = 0;
    MI_U8 u8ModuleName[10] = {0};
  
    _MI_SYS_LOG_IMPL_ConfigInfoInit();
    _MI_SYS_LOG_IMPL_LogBufferInit();
    _MI_SYS_LOG_IMPL_LogUdiskInit(UdiskLogFile);

    while(u32ModuleId < E_MI_MODULE_ID_MAX)
    {
        _MI_SYS_IMPL_ModuleIdToPrefixName((MI_ModuleId_e)u32ModuleId , u8ModuleName);
        MI_SYS_LOG_IMPL_LogAddModule(u8ModuleName , (MI_ModuleId_e)u32ModuleId);
        u32ModuleId ++;
    }

    _gu8LogInit = 1;

    return MI_SUCCESS;
}

MI_S32 _MI_SYS_LOG_IMPL_LogBufferExit(void)
{
    if(NULL != _gstLogBufferInfo.pu8StartAddrVir)
    {
        mi_sys_UnVmap(_gstLogBufferInfo.pu8StartAddrVir);
        mi_sys_MMA_Free(_gstLogBufferInfo.phyStartAddrPhy);
        _gstLogBufferInfo.pu8StartAddrVir = NULL;
        _gstLogBufferInfo.phyStartAddrPhy = 0;
    }
    return MI_SUCCESS;
}


MI_S32 _MI_SYS_LOG_IMPL_ConfigInfoExit(void)
{
    if (NULL != _gstLogBuffer.pStartVirAddr)
    {
        mi_sys_UnVmap(_gstLogBuffer.pStartVirAddr);
        mi_sys_MMA_Free(_gstLogBuffer.phyStartPhyAddr);
        _gstLogBuffer.pStartVirAddr = NULL;
        _gstLogBuffer.phyStartPhyAddr = 0;
        _gpstLogConfigInfo = NULL;
    }
    
    return MI_SUCCESS;
}
MI_S32 MI_SYS_LOG_IMPL_Exit(void)
{
    MI_U32 u32ModuleId = 0;
    MI_U8 u8ModuleName[10] = {0};
    
    while(u32ModuleId < E_MI_MODULE_ID_MAX)
    {
        _MI_SYS_IMPL_ModuleIdToPrefixName((MI_ModuleId_e)u32ModuleId , u8ModuleName);
        MI_SYS_LOG_IMPL_LogRemoveModule(u8ModuleName , (MI_ModuleId_e)u32ModuleId);
        u32ModuleId ++;
    } 
    _gu8LogInit = 0;

    _MI_SYS_LOG_IMPL_LogBufferExit();
    _MI_SYS_LOG_IMPL_ConfigInfoExit();

    return MI_SUCCESS;
}


int MI_SYS_LOG_IMPL_ProcRead(struct seq_file *s, void *pArg)
{
    MI_U32 i;
    MI_U8 u8Level;
    MI_U32 u32Total = E_MI_MODULE_ID_MAX;

    if (0 == _gu8LogInit)
    {
        seq_printf(s,"    Log module not init\n");
        return 0;
    }
    seq_printf(s,"---------------- Log Path ------------------------\n");
    seq_printf(s,"log path:  %s\n", UdiskLogFile);

    seq_printf(s,"---------------- Store Path ----------------------\n");
    seq_printf(s,"store path:  %s\n", StorePath);

    seq_printf(s,"---------------- Module Log Level ----------------\n");
    seq_printf(s,"Log module\t  Level\n");
    seq_printf(s,"--------------------------\n");
    for (i = 0; i < u32Total; i++)
    {
        if (strncmp(_gpstLogConfigInfo[i].u8ModName, "modulemax", 10))
        {
            _gpstLogConfigInfo[i].eLogLevel = GetModuleLevel(_gpstLogConfigInfo[i].u8ModName);
            u8Level = _gpstLogConfigInfo[i].eLogLevel;
            seq_printf(s,"%-16s  %d(%s)\n",
                _gpstLogConfigInfo[i].u8ModName, u8Level, DebugLevelName[u8Level]);
        }
    }

    seq_printf(s,"\nhelp example:\n");
    seq_printf(s,"echo mi_sys=2 > /proc/mi_modules/mi_log_info \n");
    seq_printf(s,"echo mi_vdisp=1 > /proc/mi_modules/mi_log_info \n");
    seq_printf(s,"echo log=/mnt > /proc/mi_modules/mi_log_info \n");
    seq_printf(s,"echo storepath=/mnt > /proc/mi_modules/mi_log_info \n");

    return 0;
}




ssize_t MI_SYS_LOG_IMPL_ProcWrite( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    char m[MAX_FILENAME_LENTH] = {0};
    char d[MAX_FILENAME_LENTH] = {0};
    size_t len = MAX_FILENAME_LENTH;
    char *left, *right;
    int idx, level;
    int nRet = 0;
    if (*ppos >= MAX_FILENAME_LENTH)
        return -EFBIG;

    len = min(len, count);
    if(copy_from_user(m, buf, len ))
        return -EFAULT;

    if (0 == _gu8LogInit)
    {
        DBG_ERR("    Log module not init!\n");
        goto out;
    }

    StripString(m, d);

    /* echo help info to current terinmal */
    if (!strncasecmp("help", m, 4))
    {
        printk("To modify the level, use command line in shell: \n");
        printk("    echo module_name = level_number > /proc/msp/log\n");
        printk("    level_number: 0-fatal, 1-error, 2-warning, 3-info\n");
        printk("    example: 'echo HI_DEMUX=3 > /proc/msp/log'\n");
        printk("    will change log levle of module \"HI_DEMUX\" to 3, then, \n");
        printk("all message with level higher than \"info\" will be printed.\n");
        printk("Use 'echo \"all = x\" > /proc/msp/log' to change all modules.\n");

        printk("\n\nTo modify the log path, use command line in shell: \n");
        printk("Use 'echo \"log = x\" > /proc/msp/log' to set log path.\n");
        printk("Use 'echo \"log = /dev/null\" > /proc/msp/log' to close log udisk output.\n");
        printk("    example: 'echo log=/home > /proc/msp/log'\n");

        printk("\n\nTo modify the debug file store path, use command line in shell: \n");
        printk("Use 'echo \"storepath = x\" > /proc/msp/log' to set debug file path.\n");
        printk("    example: 'echo storepath=/tmp > /proc/msp/log'\n");
    }

    if (SeperateString(d, &left, &right)){
        DBG_WRN("string is unkown!\n");
        goto out;
    }

    if (!strncasecmp("log", left, 4))
    {
        if (strlen(right) >= sizeof(g_szPathBuf))
        {
            DBG_ERR("    Log path length is over than %d!\n",sizeof(g_szPathBuf));
            goto out;
        }

        nRet = LOG_FILE_LOCK();

        memset(g_szPathBuf, 0, sizeof(g_szPathBuf));
        memcpy(g_szPathBuf, right, strlen(right));

        if ( memcmp(g_szPathBuf, "/dev/null", strlen("/dev/null")) != 0 )
        {
            _gbSetLogFileFlag = TRUE;
        }
        else
        {
            _gbSetLogFileFlag = FALSE;
        }

        _gpstLogConfigInfo->u8UdiskFlag = (MI_U8)_gbSetLogFileFlag;

        UdiskLogFile = g_szPathBuf;

        LOG_FILE_UNLOCK();

        DBG_INFO("set log path is g_szPathBuf = %s\n", UdiskLogFile);

        goto out;
    }
    else if (!strncasecmp("storepath", left, strlen("storepath")+1))
    {
        if (strlen(right) >= sizeof(g_szStorePathBuf))
        {
            DBG_ERR("    Store path length is over than %d!\n",sizeof(g_szStorePathBuf));
            goto out;
        }

        nRet = LOG_FILE_LOCK();

        memset(g_szStorePathBuf, 0, sizeof(g_szStorePathBuf));
        memcpy(g_szStorePathBuf, right, strlen(right));

        StorePath = g_szStorePathBuf;

        LOG_FILE_UNLOCK();

        DBG_ERR("set log path is StorePath = %s\n", g_szStorePathBuf);

        goto out;
    }
    else
    {
        level = simple_strtol(right, NULL, 10);
        if (!level && *right != '0'){
            DBG_WRN("invalid value!\n");
            goto out;
        }
        if (!strncasecmp("all", left, 4)){
            int i = 0;
            MI_U32 u32Total = E_MI_MODULE_ID_MAX;
            for (i = 0; i < u32Total; i++)
            {
                _gpstLogConfigInfo[i].eLogLevel = (MI_DBG_LEVEL_e)level;
                SetModuleLevel(strlwr(_gpstLogConfigInfo[i].u8ModName) , level);
            }
            goto out;
        }

        idx = SearchMod(left);
        if (-1 == idx){
            DBG_WRN("%s not found in array!\n", left);
            return count;
        }
        _gpstLogConfigInfo[idx].eLogLevel = (MI_DBG_LEVEL_e)level;
        if(!SetModuleLevel(strlwr(_gpstLogConfigInfo[idx].u8ModName) , level))
        {
            goto out;
        }
        return count;
    }

out:
    *ppos = len;
    return len;
}


int LogUdiskWriteThread(void* pArg)
{
    MI_U8 szBuf[700] = {0};
    MI_U32 u32ReadLen = 0;
    MI_U32 s32Ret = 0;
    MI_U8 szFileName[MAX_FILENAME_LENTH] = {0};
    MI_BOOL bSetFileFlag = FALSE;

    while (1)
    {
        s32Ret = LOG_FILE_LOCK();

        bSetFileFlag = _gbSetLogFileFlag;

        MI_SYS_LOG_IMPL_Snprintf(szFileName, sizeof(szFileName)-1, "%s/mi.log", (const MI_S8*)UdiskLogFile);

        LOG_FILE_UNLOCK();

        set_current_state(TASK_INTERRUPTIBLE);

        if(kthread_should_stop())
        {
            break;
        }

        if ( bSetFileFlag == FALSE)
        {
            msleep(10);
            continue;
        }

        memset(szBuf, 0, sizeof(szBuf));

        s32Ret = MI_SYS_LOG_IMPL_ReadBuf(szBuf, sizeof(szBuf)-1, &u32ReadLen, TRUE);
        if (s32Ret == MI_SUCCESS)
        {
            LogUdiskSave((const MI_S8*)szFileName, szBuf, u32ReadLen);
        }

        msleep(100);
    }

    return 0;
}


MI_S32 _MI_SYS_LOG_IMPL_LogUdiskInit(const MI_U8* pUdiskFolder)
{
    int err;

    if (pUdiskFolder == NULL )
    {
        return MI_ERR_SYS_NOT_PERM;
    }

    if (gpLogUdiskTask == NULL)
    {
        gpLogUdiskTask = kthread_create(LogUdiskWriteThread, (void*)pUdiskFolder, "miLodUdiskTask");
        if(IS_ERR(gpLogUdiskTask))
        {
            DBG_ERR("create log Udisk write thread failed\n");

            err = PTR_ERR(gpLogUdiskTask);
            gpLogUdiskTask = NULL;

            return err;
        }

        wake_up_process(gpLogUdiskTask);
    }

    return MI_SUCCESS;
}

