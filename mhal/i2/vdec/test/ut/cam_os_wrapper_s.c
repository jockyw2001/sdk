////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_wrapper.c
/// @brief     Cam OS Wrapper Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "time.h"
#include "sys_sys.h"
#include "sys_sys_math64.h"
#include "sys_sys_tools.h"
#include "sys_sys_core.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_sys_isw_uart.h"
#include "sys_rtk_hp.h"
#include "hal_drv_util.h"
#include "sys_time.h"
#include "cam_os_wrapper_s.h"
#include "cam_os_util_list_s.h"
#include "cam_os_util_list_s.h"

#define __RTK_US_RESOLUTION_TIMER__
#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192

typedef void *CamOsThreadEntry_t(void *);

typedef struct
{
    MsTaskId_e eHandleObj;
    CamOsThreadEntry_t *pfnEntry;
    void *pArg;
    Ms_Flag_t tExitFlag;
    void *pStack;
} CamOsThreadHandleRtk_t, *pCamOsThreadHandleRtk;

typedef struct
{
    u32 nInited;
    Ms_Mutex_t tMutex;
} CamOsMutexRtk_t, *pCamOsMutexRtk;

typedef struct
{
    Ms_Flag_t tFlag;  /* or semaphore if supported */
    Ms_Mutex_t tMutex;
    u32 nSemval;
} CamOsTsemRtk_t, *pCamOsTsemRtk;

static Ms_Mutex_t _gtSelfInitLock = {0};

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(Ms_Flag_t)+4, "CamOsMutex_t size define not enough!");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemRtk_t), "CamOsTsem_t size define not enough!");

#elif defined(CAM_OS_LINUX_USER)
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdbool.h>
#include <mdrv_msys_io.h>
#include <mdrv_msys_io_st.h>
#include "mdrv_verchk.h"
#include <sys/mman.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include "time.h"
#include "cam_os_util_list_s.h"
#include "cam_os_wrapper_s.h"

typedef struct
{
    u32 nInited;
    pthread_mutex_t tMutex;
} CamOsMutexLU_t, *pCamOsMutexLU;

typedef struct
{
    pthread_cond_t tCondition;
    pthread_mutex_t tMutex;
    u32 nSemval;
} CamOsTsemLU_t, *pCamOsTsemLU;

// TODO remove extern pthread_setname_np and pthread_getname_np
extern int pthread_setname_np(pthread_t tTargetThread, const char *szTargetName);
extern int pthread_getname_np(pthread_t tThread, char *szName, size_t nLen);

static pthread_mutex_t _gtSelfInitLock = PTHREAD_MUTEX_INITIALIZER;

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(pthread_mutex_t)+4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLU_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");

#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <ms_msys.h>
#include <ms_platform.h>
#include "cam_os_wrapper.h"
#include "cam_os_util_list.h"

#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192

typedef struct
{
    u32 nInited;
    struct mutex tMutex;
} CamOsMutexLK_t, *pCamOsMutexLK;

typedef struct
{
    struct semaphore tSem;
    struct mutex tMutex;
    u32 nSemval;
} CamOsTsemLK_t, *pCamOsTsemLK;

typedef s32 CamOsThreadEntry_t(void *);

static DEFINE_MUTEX(_gtSelfInitLock);

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(struct mutex)+4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLK_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");
_Static_assert(sizeof(CamOsAtomic_t) == sizeof(atomic_t), "CamOsAtomic_t size define not enough!");

#endif

#define RIU_BASE_ADDR           0x1F000000
#define RIU_MEM_SIZE_OFFSET     0x2025A4
#define RIU_CHIP_ID_OFFSET      0x003C00

#define ASSIGN_POINTER_VALUE(a, b) if((a))*(a)=(b)

typedef struct MemoryList_t
{
    struct CamOsListHead_t tList;
    void *pPtr;
    void *pMemifoPtr;
    char *szName;
    u32  nSize;
} MemoryList_t;

static MemoryList_t _gtMemList;
static CamOsMutex_t _gtMemLock;

static s32 _gnDmemDbgListInited = 0;

void CamOsPrintf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    va_list tArgs;
    char nLineStr[256];

    va_start(tArgs, szFmt);
    vsprintf(nLineStr, szFmt, tArgs);
    send_msg(nLineStr);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_USER)
    va_list tArgs;

    va_start(tArgs, szFmt);
    vfprintf(stderr, szFmt, tArgs);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_KERNEL)
    va_list tArgs;

    va_start(tArgs, szFmt);
    vprintk(szFmt, tArgs);
    va_end(tArgs);
#endif
}

#ifdef CAM_OS_RTK
static char* _CamOsAdvance(char* pBuf) {

    char* pNewBuf = pBuf;

    /* Skip over nonwhite space */
    while ((*pNewBuf != ' ')  && (*pNewBuf != '\t') &&
            (*pNewBuf != '\n') && (*pNewBuf != '\0'))
    {
        pNewBuf++;
    }

    /* Skip white space */
    while ((*pNewBuf == ' ')  || (*pNewBuf == '\t') ||
            (*pNewBuf == '\n') || (*pNewBuf == '\0'))
    {
        pNewBuf++;
    }

    return pNewBuf;
}

static s32 _CamOsVsscanf(char* szBuf, char* szFmt, va_list tArgp)
{
    char*    pFmt;
    char*    pBuf;
    char*    pnSval;
    u32*     pnU32Val;
    s32*     pnS32Val;
    u64*     pnU64Val;
    s64*     pnS64Val;
    double*  pdbDval;
    float*   pfFval;
    s32      nCount = 0;

    pBuf = szBuf;

    for (pFmt = szFmt; *pFmt; pFmt++)
    {
        if (*pFmt == '%')
        {
            pFmt++;
            if (strncmp (pFmt, "u", 1) == 0)
            {
                pnU32Val = va_arg(tArgp, u32 *);
                sscanf(pBuf, "%u", pnU32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "d", 1) == 0)
            {
                pnS32Val = va_arg(tArgp, s32 *);
                sscanf(pBuf, "%d", pnS32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "llu", 3) == 0)
            {
                pnU64Val = va_arg(tArgp, u64 *);
                sscanf(pBuf, "%llu", pnU64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "lld", 3) == 0)
            {
                pnS64Val = va_arg(tArgp, s64 *);
                sscanf(pBuf, "%lld", pnS64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "f", 1) == 0)
            {
                pfFval = va_arg(tArgp, float *);
                sscanf(pBuf, "%f", pfFval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "lf", 2) == 0)
            {
                pdbDval = va_arg(tArgp, double *);
                sscanf(pBuf, "%lf", pdbDval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if (strncmp (pFmt, "s", 1) == 0)
            {
                pnSval = va_arg(tArgp, char *);
                sscanf(pBuf, "%s", pnSval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else
            {
                CamOsPrintf("_CamOsVsscanf error: unsupported format (\%%s)\n", pFmt);
            }
        }
    }

    return nCount;
}

static s32 _CamOsGetString(char* szBuf, s32 nMaxLen,  s32 nEcho)
{
    s32 nLen;
    static char ch = '\0';

    nLen = 0;
    while (1)
    {
        szBuf[nLen] = get_char();

        // To ignore one for (\r,\n) or (\n, \r) pair
        if ((szBuf[nLen]=='\n' && ch=='\r') || (szBuf[nLen]=='\r' && ch=='\n'))
        {
            ch = '\0';
            continue;
        }
        ch = szBuf[nLen];
        if (ch=='\n' || ch=='\r')
        {
            if (nEcho)
                CamOsPrintf("\n");
            break;
        }
        if (nLen < (nMaxLen - 1))
        {
            if (ch == '\b') /* Backspace? */
            {
                if (nLen <= 0)
                    CamOsPrintf("\007");
                else
                {
                    CamOsPrintf("\b \b");
                    nLen --;
                }
                continue;
            }
            nLen++;
        }
        if (nEcho)
            CamOsPrintf("%c", ch);
    }
    szBuf[nLen] = '\0';
    return nLen;
}

static s32 _CamOsVfscanf(const char *szFmt, va_list tArgp)
{
    s32 nCount;
    char szCommandBuf[128];

    _CamOsGetString(szCommandBuf, sizeof(szCommandBuf), 1);

    nCount = _CamOsVsscanf(szCommandBuf, (char *)szFmt, tArgp);
    return nCount;
}
#endif

s32 CamOsScanf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    s32 nCount = 0;
    va_list tArgp;

    va_start (tArgp, szFmt);
    nCount = _CamOsVfscanf (szFmt, tArgp);
    va_end (tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_USER)
    s32 nCount = 0;
    va_list tArgp;

    va_start (tArgp, szFmt);
    nCount = vfscanf (stdin, szFmt, tArgp);
    va_end (tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

s32 CamOsGetChar(void)
{
#ifdef CAM_OS_RTK
    s32 Ret;
    Ret = get_char();
    CamOsPrintf("\n");
    return Ret;
#elif defined(CAM_OS_LINUX_USER)
    return getchar();
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

void CamOsMsSleep(u32 nMsec)
{
#ifdef CAM_OS_RTK
    MsSleep(RTK_MS_TO_TICK(nMsec));
#elif defined(CAM_OS_LINUX_USER)
    usleep((useconds_t)nMsec * 1000);
#elif defined(CAM_OS_LINUX_KERNEL)
    msleep(nMsec);
#endif
}

void CamOsGetTimeOfDay(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    if (ptRes)
    {
        SysTimeGetUTCSeconds(&ptRes->nSec);
        ptRes->nNanoSec = 0;
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if (ptRes)
    {
        gettimeofday(&tTV, NULL);
        ptRes->nSec = tTV.tv_sec;
        ptRes->nNanoSec = tTV.tv_usec*1000;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timeval tTv;
    if (ptRes)
    {
        do_gettimeofday(&tTv);
        ptRes->nSec = tTv.tv_sec;
        ptRes->nNanoSec = tTv.tv_usec*1000;
    }
#endif
}

void CamOsSetTimeOfDay(const CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    struct tm * tTm;
    vm_rtcTimeFormat_t tLocalTime;
    if (ptRes)
    {
        tTm = localtime ((time_t *)&ptRes->nSec);

        tLocalTime.Year = tTm->tm_year;
        tLocalTime.Month = tTm->tm_mon+1;
        tLocalTime.Day = tTm->tm_mday;
        tLocalTime.DayOfWeek = tTm->tm_wday? tTm->tm_wday : 7;
        tLocalTime.Hour = tTm->tm_hour;
        tLocalTime.Minute = tTm->tm_min;
        tLocalTime.Second = tTm->tm_sec;

        SysTimeSetTime(&tLocalTime);
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if (ptRes)
    {
        tTV.tv_sec = ptRes->nSec;
        tTV.tv_usec = ptRes->nNanoSec/1000;
        settimeofday(&tTV, NULL);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timespec tTs;
    if (ptRes)
    {
        tTs.tv_sec = ptRes->nSec;
        tTs.tv_nsec = ptRes->nNanoSec;
        do_settimeofday(&tTs);
    }
#endif
}

void CamOsGetMonotonicTime(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
#ifdef __RTK_US_RESOLUTION_TIMER__
    u64 nUs;
    if (ptRes)
    {
        nUs = DrvTimerStdaTimerTick2UsEx(RtkGetOsTickExt());
        ptRes->nSec = nUs / 1000000;
        ptRes->nNanoSec = (nUs % 1000000) * 1000;
    }
#else
    u32 nMs;
    if (ptRes)
    {
        nMs = VM_RTK_TICK_TO_MS(MsGetOsTick());
        ptRes->nSec = nMs / 1000;
        ptRes->nNanoSec = (nMs % 1000) * 1000000;
    }
#endif
#elif defined(CAM_OS_LINUX_USER)
    clock_gettime(CLOCK_MONOTONIC, (struct timespec *)ptRes);
#elif defined(CAM_OS_LINUX_KERNEL)
    getrawmonotonic((struct timespec *)ptRes);
#endif
}

#ifdef CAM_OS_RTK
static void _CamOSThreadEntry(void *pEntryData)
{
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)pEntryData;

    ptTaskHandle->pfnEntry(ptTaskHandle->pArg);

    MsFlagSetbits(&ptTaskHandle->tExitFlag, 0x00000001);
}

static void _CamOsThreadEmptyParser(vm_msg_t *ptMessage)
{

}
#endif

CamOsRet_e CamOsThreadCreate(CamOsThread *pThread,
                             CamOsThreadAttrb_t *ptAttrb,
                             void *(*pfnStartRoutine)(void *),
                             void *pArg)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = NULL;
    MsTaskCreateArgs_t tTaskArgs = {0};
    u32 nPrio = 100;
    u32 nStkSz = CAM_OS_THREAD_STACKSIZE_DEFAULT;

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
        {
            nPrio = ptAttrb->nPriority * 2;
        }
        nStkSz = (ptAttrb->nStackSize)? ptAttrb->nStackSize : CAM_OS_THREAD_STACKSIZE_DEFAULT;
    }

    *pThread = (void*) - 1;
    do
    {
        if(!(ptTaskHandle = MsCallocateMem(sizeof(CamOsThreadHandleRtk_t))))
        {
            CamOsPrintf("%s : Allocate ptHandle fail\n\r", __FUNCTION__);
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        ptTaskHandle->pfnEntry = pfnStartRoutine;
        ptTaskHandle->pArg   = pArg;
        if(!(ptTaskHandle->pStack = MsAllocateMem((nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT)))
        {
            CamOsPrintf("%s : Allocate stack fail\n\r", __FUNCTION__);
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        memset(&ptTaskHandle->tExitFlag, 0, sizeof(Ms_Flag_t));
        MsFlagInit(&ptTaskHandle->tExitFlag);
        //VEN_TEST_CHECK_RESULT((pTaskHandle->exit_flag.FlagId >> 0)  && (pTaskHandle->exit_flag.FlagState == RTK_FLAG_INITIALIZED));

        tTaskArgs.Priority = (nPrio >= 0 && nPrio <= 200) ? nPrio : 100;
        tTaskArgs.StackSize = (nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT;
        tTaskArgs.pStackTop = (u32*)ptTaskHandle->pStack;
        tTaskArgs.AppliInit = &_CamOSThreadEntry;
        tTaskArgs.AppliParser = _CamOsThreadEmptyParser;
        tTaskArgs.pInitArgs = ptTaskHandle;
        tTaskArgs.TaskId = &ptTaskHandle->eHandleObj;
        tTaskArgs.ImmediatelyStart = TRUE;
        tTaskArgs.TimeSliceMax = 10;
        tTaskArgs.TimeSliceLeft = 10;
        strncpy(tTaskArgs.TaskName, "CamOsWrp", sizeof(tTaskArgs.TaskName));

        if(MS_OK != MsCreateTask(&tTaskArgs))
        {
            CamOsPrintf("%s : Create task fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        *pThread = ptTaskHandle;
    }
    while(0);

    if(!*pThread)
    {
        if(ptTaskHandle)
        {
            if(ptTaskHandle->pStack)
            {
                MsReleaseMemory(ptTaskHandle->pStack);
            }
            MsFlagDestroy(&ptTaskHandle->tExitFlag);
            MsReleaseMemory(ptTaskHandle);
        }
    }

    //CamOsPrintf("%s get taskid: %d(%s)  priority: %d\n\r", __FUNCTION__, (u32)ptTaskHandle->eHandleObj, tTaskArgs.TaskName, tTaskArgs.Priority);
#elif defined(CAM_OS_LINUX_USER)
    struct sched_param tSched;
    pthread_t tThreadHandle = NULL;
    pthread_attr_t tAttr;
    if(ptAttrb != NULL)
    {
        do {
            pthread_attr_init(&tAttr);

            if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
            {
                pthread_attr_getschedparam(&tAttr, &tSched);
                pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED);
                pthread_attr_setschedpolicy(&tAttr, SCHED_RR);
                tSched.sched_priority = ptAttrb->nPriority;
                if (0 != pthread_attr_setschedparam(&tAttr, &tSched))
                {
                    CamOsPrintf("%s: pthread_attr_setschedparam failed\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                    break;
                }
            }

            if(0 != ptAttrb->nStackSize)
            {
                if (0 != pthread_attr_setstacksize(&tAttr, (size_t) ptAttrb->nStackSize))
                {
                    eRet = CAM_OS_FAIL;
                    CamOsPrintf("%s pthread_attr_setstacksize failed\n\r", __FUNCTION__);
                    break;
                }
            }
            pthread_create(&tThreadHandle, &tAttr, pfnStartRoutine, pArg);

        } while (0);
        pthread_attr_destroy(&tAttr);
    }
    else
    {
        pthread_create(&tThreadHandle, NULL, pfnStartRoutine, pArg);
    }

    *pThread = (CamOsThread *)tThreadHandle;
    CamOsPrintf("%s: done(%d)\n\r", __FUNCTION__, eRet);

#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle;
    struct sched_param tSche = { .sched_priority = 0 };
    u32 nStkSz = CAM_OS_THREAD_STACKSIZE_DEFAULT;

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
        {
            tSche.sched_priority = ptAttrb->nPriority;
        }
        nStkSz = ptAttrb->nStackSize;
    }
    tpThreadHandle = kthread_run((CamOsThreadEntry_t *)pfnStartRoutine, pArg, "CAMOS");
    sched_setscheduler(tpThreadHandle, SCHED_RR, &tSche);
    *pThread = (CamOsThread *)tpThreadHandle;
#endif

    return eRet;
}

CamOsRet_e CamOsThreadJoin(CamOsThread thread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)thread;
    if(ptTaskHandle)
    {
        MsFlagWait(&ptTaskHandle->tExitFlag, 0x00000001, RTK_FLAG_WAITMODE_AND | RTK_FLAG_WAITMODE_CLR);
        MsFlagDestroy(&ptTaskHandle->tExitFlag);
        MsDeleteTask(ptTaskHandle->eHandleObj);

        if(ptTaskHandle->pStack)
        {
            MsReleaseMemory(ptTaskHandle->pStack);
        }
        MsReleaseMemory(ptTaskHandle);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)thread;
    if(tThreadHandle)
    {
        pthread_join(tThreadHandle, NULL);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

CamOsRet_e CamOsThreadStop(CamOsThread thread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)thread;
    if(tpThreadHandle)
    {
        if(0 != kthread_stop((struct task_struct *)thread))
        {
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadShouldStop(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    if(kthread_should_stop())
    {
        eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_FAIL;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadSetName(CamOsThread thread, const char *szName)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)thread;
    if(ptTaskHandle && szName)
    {
        MsSetTaskName(ptTaskHandle->eHandleObj, szName);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)thread;
    if(tThreadHandle)
    {
        pthread_setname_np(tThreadHandle, szName);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif

    return eRet;
}

CamOsRet_e CamOsThreadGetName(CamOsThread thread, char *szName, u32 nLen)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)thread;
    if(ptTaskHandle && szName)
    {
        MsGetTaskName(ptTaskHandle->eHandleObj, szName, nLen);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    pthread_t tThreadHandle = (pthread_t)thread;
    if(tThreadHandle)
    {
        pthread_getname_np(tThreadHandle, szName, nLen);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

u32 CamOsThreadGetID()
{
#ifdef CAM_OS_RTK
    return MsCurrTask();
#elif defined(CAM_OS_LINUX_USER)
    return (u32)syscall(__NR_gettid);
#elif defined(CAM_OS_LINUX_KERNEL)
    return current->tgid;
#endif
}

CamOsRet_e CamOsMutexInit(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            MsMutexLock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(CUS_OS_OK != MsInitMutex(&ptHandle->tMutex))
                {
                    CamOsPrintf("%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            MsMutexUnlock(&_gtSelfInitLock);
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            pthread_mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
                {
                    fprintf(stderr, "%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            pthread_mutex_unlock(&_gtSelfInitLock);
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                mutex_init(&ptHandle->tMutex);
                ptHandle->nInited = 0x55AA5AA5;
            }
            mutex_unlock(&_gtSelfInitLock);
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexDestroy(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        ptHandle->nInited = 0;
        pthread_mutex_destroy(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexLock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            MsMutexLock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(CUS_OS_OK != MsInitMutex(&ptHandle->tMutex))
                {
                    CamOsPrintf("%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            MsMutexUnlock(&_gtSelfInitLock);
        }

        if(CUS_OS_OK != MsMutexLock(&ptHandle->tMutex))
        {
            CamOsPrintf("%s : Lock mutex fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            pthread_mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
                {
                    fprintf(stderr, "%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            pthread_mutex_unlock(&_gtSelfInitLock);
        }

        if(0 != (nErr = pthread_mutex_lock(&ptHandle->tMutex)))
        {
            fprintf(stderr, "%s : Lock mutex fail, err %d\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                mutex_init(&ptHandle->tMutex);
                ptHandle->nInited = 0x55AA5AA5;
            }
            mutex_unlock(&_gtSelfInitLock);
        }

        mutex_lock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexUnlock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            MsMutexLock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(CUS_OS_OK != MsInitMutex(&ptHandle->tMutex))
                {
                    CamOsPrintf("%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            MsMutexUnlock(&_gtSelfInitLock);
        }

        if(CUS_OS_OK != MsMutexUnlock(&ptHandle->tMutex))
        {
            CamOsPrintf("%s : Unlock mutex fail\n\r", __FUNCTION__);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            pthread_mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
                {
                    fprintf(stderr, "%s : Init mutex fail\n\r", __FUNCTION__);
                    eRet = CAM_OS_FAIL;
                }
                ptHandle->nInited = 0x55AA5AA5;
            }
            pthread_mutex_unlock(&_gtSelfInitLock);
        }

        if(0 != (nErr = pthread_mutex_unlock(&ptHandle->tMutex)))
        {
            fprintf(stderr, "%s : Unlock mutex fail, err %d\n\r", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if (ptHandle->nInited != 0x55AA5AA5)
        {
            mutex_lock(&_gtSelfInitLock);
            if (ptHandle->nInited != 0x55AA5AA5)
            {
                mutex_init(&ptHandle->tMutex);
                ptHandle->nInited = 0x55AA5AA5;
            }
            mutex_unlock(&_gtSelfInitLock);
        }

        mutex_unlock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemInit(CamOsTsem_t *ptTsem, u32 nVal)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsFlagInit(&ptHandle->tFlag);
        MsInitMutex(&ptHandle->tMutex);
        ptHandle->nSemval = nVal;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    pthread_condattr_t cattr;
    if(ptHandle)
    {
        if (0 != pthread_condattr_init(&cattr) ||
            0 != pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC)) {
            return CAM_OS_FAIL;
        }

        if(0 != pthread_cond_init(&ptHandle->tCondition, &cattr))
        {
            return CAM_OS_FAIL;
        }
        if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
        {
            return CAM_OS_FAIL;
        }
        ptHandle->nSemval = nVal;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        sema_init(&ptHandle->tSem, nVal);
        mutex_init(&ptHandle->tMutex);
        ptHandle->nSemval = nVal;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemDeinit(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsFlagDestroy(&ptHandle->tFlag);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_cond_destroy(&ptHandle->tCondition);
        pthread_mutex_destroy(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

void CamOsTsemUp(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsMutexLock(&ptHandle->tMutex);
        ptHandle->nSemval++;
        MsMutexUnlock(&ptHandle->tMutex);
        MsFlagSetbits(&ptHandle->tFlag, 0x00000001);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_mutex_lock(&ptHandle->tMutex);
        ptHandle->nSemval++;
        pthread_cond_signal(&ptHandle->tCondition);
        pthread_mutex_unlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        mutex_lock(&ptHandle->tMutex);
        ptHandle->nSemval++;
        mutex_unlock(&ptHandle->tMutex);
        up(&ptHandle->tSem);
    }
#endif
}

void CamOsTsemDown(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsMutexLock(&ptHandle->tMutex);
        while(ptHandle->nSemval == 0)
        {
            MsMutexUnlock(&ptHandle->tMutex);
            MsFlagWait(&ptHandle->tFlag, 0x00000001, MS_FLAG_WAITMODE_OR);
            MsMutexLock(&ptHandle->tMutex);
        }
        if(ptHandle->nSemval > 0)
        {
            ptHandle->nSemval--;
        }
        MsMutexUnlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_mutex_lock(&ptHandle->tMutex);
        while(ptHandle->nSemval == 0)
        {
            pthread_cond_wait(&ptHandle->tCondition, &ptHandle->tMutex);
        }
        ptHandle->nSemval--;
        pthread_mutex_unlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        down(&ptHandle->tSem);
        mutex_lock(&ptHandle->tMutex);
        ptHandle->nSemval--;
        mutex_unlock(&ptHandle->tMutex);
    }
#endif
}

CamOsRet_e CamOsTsemTimedDown(CamOsTsem_t *ptTsem, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    s32 nErr = 0xFFFFFFFF;
    if(ptHandle)
    {
        MsMutexLock(&ptHandle->tMutex);
        while(ptHandle->nSemval == 0)
        {
            MsMutexUnlock(&ptHandle->tMutex);
            nErr = MsFlagTimedWait(&ptHandle->tFlag, 1, (MS_FLAG_WAITMODE_OR), RTK_MS_TO_TICK(nMsec));
            /*time out*/
            MsMutexLock(&ptHandle->tMutex);
            if(!nErr)
            {
                break;
            }
        }
        if(nErr && ptHandle->nSemval > 0)
        {
            ptHandle->nSemval--;
        }
        MsMutexUnlock(&ptHandle->tMutex);
        if(!nErr)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    s32 nErr = 0;
    struct timespec tFinalTime;
    struct timespec tCurrTime;
    s64 nNanoDelay = 0;

    if(ptHandle)
    {
        clock_gettime(CLOCK_MONOTONIC, &tCurrTime);

        nNanoDelay = (nMsec * 1000000LL) + tCurrTime.tv_nsec;
        tFinalTime.tv_sec = tCurrTime.tv_sec + (nNanoDelay / 1000000000LL);
        tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

        pthread_mutex_lock(&ptHandle->tMutex);
        while(ptHandle->nSemval == 0)
        {
            nErr = pthread_cond_timedwait(&ptHandle->tCondition, &ptHandle->tMutex,
                                          &tFinalTime);
            if(nErr != 0)
            {
                break;
            }
        }
        if(nErr == 0)
        {
            ptHandle->nSemval--;
        }
        pthread_mutex_unlock(&ptHandle->tMutex);

        if(!nErr)
            eRet = CAM_OS_OK;
        else if(nErr == ETIMEDOUT)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_FAIL;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    s32 nErr = 0;
    if(ptHandle)
    {
        nErr = down_timeout(&ptHandle->tSem, msecs_to_jiffies(nMsec));
        mutex_lock(&ptHandle->tMutex);
        if(ptHandle->nSemval > 0)
        {
            ptHandle->nSemval--;
        }
        mutex_unlock(&ptHandle->tMutex);
        if(nErr)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTsemSignal(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsFlagSetbits(&ptHandle->tFlag, 0x00000001);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_mutex_lock(&ptHandle->tMutex);
        pthread_cond_signal(&ptHandle->tCondition);
        pthread_mutex_unlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        up(&ptHandle->tSem);
    }
#endif
}

void CamOsTsemWait(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsFlagWait(&ptHandle->tFlag, 0x00000001, MS_FLAG_WAITMODE_OR);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_mutex_lock(&ptHandle->tMutex);
        pthread_cond_wait(&ptHandle->tCondition, &ptHandle->tMutex);
        pthread_mutex_unlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        down(&ptHandle->tSem);
    }
#endif
}

CamOsRet_e CamOsTsemTimedWait(CamOsTsem_t *ptTsem, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    s32 nErr = 0;
    if(ptHandle)
    {
        nErr = MsFlagTimedWait(&ptHandle->tFlag, 1, (MS_FLAG_WAITMODE_OR), RTK_MS_TO_TICK(nMsec));
        if(!nErr)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    s32 nErr = 0;
    struct timespec tFinalTime;
    struct timespec tCurrTime;
    s64 nNanoDelay = 0;

    if(ptHandle)
    {
        clock_gettime(CLOCK_MONOTONIC, &tCurrTime);

        nNanoDelay = (nMsec * 1000000LL) + tCurrTime.tv_nsec;
        tFinalTime.tv_sec = tCurrTime.tv_sec + (nNanoDelay / 1000000000LL);
        tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

        pthread_mutex_lock(&ptHandle->tMutex);

        nErr = pthread_cond_timedwait(&ptHandle->tCondition, &ptHandle->tMutex,
                                      &tFinalTime);

        pthread_mutex_unlock(&ptHandle->tMutex);

        if(!nErr)
            eRet = CAM_OS_OK;
        else if(nErr == ETIMEDOUT)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_FAIL;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    s32 nErr = 0;
    if(ptHandle)
    {
        nErr = down_timeout(&ptHandle->tSem, msecs_to_jiffies(nMsec));
        if(nErr)
            eRet = CAM_OS_TIMEOUT;
        else
            eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

u32 CamOsTsemGetValue(CamOsTsem_t *ptTsem)
{
    s32 eRet = 0;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        eRet = ptHandle->nSemval;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        eRet = ptHandle->nSemval;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        eRet = ptHandle->nSemval;
    }
#endif
    return eRet;
}

void CamOsTsemReset(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        MsMutexLock(&ptHandle->tMutex);
        ptHandle->nSemval = 0;
        MsMutexUnlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        pthread_mutex_lock(&ptHandle->tMutex);
        ptHandle->nSemval = 0;
        pthread_mutex_unlock(&ptHandle->tMutex);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        sema_init(&ptHandle->tSem, 0);
        mutex_lock(&ptHandle->tMutex);
        ptHandle->nSemval = 0;
        mutex_unlock(&ptHandle->tMutex);
    }
#endif
}

void* CamOsMemAlloc(u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsAllocateMem(nSize);
#elif defined(CAM_OS_LINUX_USER)
    return malloc(nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    return kzalloc(nSize, GFP_KERNEL);
#endif
}

void* CamOsMemCalloc(u32 nNum, u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsCallocateMem(nNum * nSize);
#elif defined(CAM_OS_LINUX_USER)
    return calloc(nNum, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    return kzalloc(nSize, GFP_KERNEL);
#endif
}

void* CamOsMemRealloc(void* pPtr, u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsMemoryReAllocate(pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    return realloc(pPtr, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    void *pAddr = kzalloc(nSize, GFP_KERNEL);
    if(pPtr && pAddr)
    {
        memcpy(pAddr, pPtr, nSize);
        kfree(pPtr);
    }
    return pAddr;
#endif
}

void CamOsMemRelease(void* pPtr)
{
#ifdef CAM_OS_RTK
    if(pPtr)
    {
        MsReleaseMemory(pPtr);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(pPtr)
    {
        free(pPtr);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(pPtr)
    {
        kfree(pPtr);
    }
#endif
}

static s32 _CheckDmemInfoListInited(void)
{
    CamOsMutexLock(&_gtMemLock);
    if(!_gnDmemDbgListInited)
    {
        memset(&_gtMemList, 0, sizeof(MemoryList_t));
        CAM_OS_INIT_LIST_HEAD(&_gtMemList.tList);

        _gnDmemDbgListInited = 1;
    }
    CamOsMutexUnlock(&_gtMemLock);

    return 0;
}

CamOsRet_e CamOsDirectMemAlloc(const char* szName,
                               u32 nSize,
                               void** ppVirtPrt,
                               void** ppPhysPtr,
                               void** ppMiuPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    u8 nAllocSucc = TRUE;
    void *pNonCachePtr = NULL;

    pNonCachePtr = MsAllocateNonCacheMemExt(nSize, 12);
    nAllocSucc &= MsIsHeapMemory(pNonCachePtr);

    if((u32)pNonCachePtr & ((1 << 6) - 1))
    {
        nAllocSucc &= FALSE;
        MsReleaseMemory(pNonCachePtr);
    }

    if(nAllocSucc == TRUE)
    {
        ASSIGN_POINTER_VALUE(ppVirtPrt, pNonCachePtr);
        ASSIGN_POINTER_VALUE(ppPhysPtr, MsVA2PA(pNonCachePtr));
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)HalUtilPHY2MIUAddr((u32)*ppPhysPtr));
    }
    else
    {
        ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
        ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
        ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
        eRet = CAM_OS_FAIL;
    }

    /*CamOsPrintf("%s    0x%08X  0x%08X  0x%08X\r\n",
            __FUNCTION__,
            (u32)*ppVirtPrt,
            (u32)*ppPhysPtr,
            (u32)*ppMiuPtr);*/

    _CheckDmemInfoListInited();

    CamOsMutexLock(&_gtMemLock);
    MemoryList_t* ptNewEntry = (MemoryList_t*) MsAllocateMem(sizeof(MemoryList_t));
    ptNewEntry->pPtr = pNonCachePtr;
    ptNewEntry->pMemifoPtr = NULL;
    ptNewEntry->szName = (char *)MsAllocateMem(strlen(szName));
    if (ptNewEntry->szName)
        strcpy(ptNewEntry->szName, szName);
    ptNewEntry->nSize = nSize;
    CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
    CamOsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysFd = -1;
    s32 nMemFd = -1;
    MSYS_DMEM_INFO * ptMsysMem = NULL;
    unsigned char* pMmapPtr = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                fprintf(stderr, "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC | O_DIRECT)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC | O_DIRECT)))
        {
            fprintf(stderr, "%s open /dev/mem failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        ptMsysMem = (MSYS_DMEM_INFO *) malloc(sizeof(MSYS_DMEM_INFO));
        MSYS_ADDR_TRANSLATION_INFO tAddrInfo;
        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);
        FILL_VERCHK_TYPE(*ptMsysMem, ptMsysMem->VerChk_Version,
                         ptMsysMem->VerChk_Size, IOCTL_MSYS_VERSION);

        ptMsysMem->length = nSize;
        snprintf(ptMsysMem->name, sizeof(ptMsysMem->name), "%s", szName);

        if(ioctl(nMsysFd, IOCTL_MSYS_REQUEST_DMEM, ptMsysMem))
        {
            ptMsysMem->length = 0;
            fprintf(stderr, "%s [%s][%d]Request Direct Memory Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        if (ptMsysMem->length < nSize)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr,"%s [%s]Request Direct Memory Failed!! because dmem size <%d>smaller than <%d>\n",
                    __FUNCTION__, szName, ptMsysMem->length, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        tAddrInfo.addr = ptMsysMem->phys;
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptMsysMem->phys);
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s][%d]IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)tAddrInfo.addr);
        pMmapPtr = mmap(0, ptMsysMem->length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        nMemFd, ptMsysMem->phys);
        if(pMmapPtr == (void *) - 1)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s failed!! physAddr<0x%x> size<0x%x> errno<%d, %s> \r\n",
                    __FUNCTION__,
                    (u32)ptMsysMem->phys,
                    (u32)ptMsysMem->length, errno, strerror(errno));
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppVirtPrt, pMmapPtr);

        fprintf(stderr, "%s <%s> physAddr<0x%x> size<%d> kvirt<0x%x>\r\n",
                __FUNCTION__,
                szName, (u32)ptMsysMem->phys,
                (u32)ptMsysMem->length,
                (u32)ptMsysMem->kvirt);

        CamOsMutexLock(&_gtMemLock);
        MemoryList_t* ptNewEntry = (MemoryList_t*) malloc(sizeof(MemoryList_t));
        ptNewEntry->pPtr = pMmapPtr;
        ptNewEntry->pMemifoPtr = (void *) ptMsysMem;
        ptNewEntry->szName = strdup(szName);
        ptNewEntry->nSize = ptMsysMem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        CamOsMutexUnlock(&_gtMemLock);
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
    if(nMemFd >= 0)
    {
        close(nMemFd);
    }
    printf("%s <%s> physAddr<0x%x> size<%d> kvirt<0x%x>\r\n",
            __FUNCTION__,
            szName, (u32)ptMsysMem->phys,
            (u32)ptMsysMem->length,
            (u32)ptMsysMem->kvirt);

#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *ptDmem = NULL;
    MemoryList_t* ptNewEntry;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
    ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
    ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                printk(KERN_WARNING "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 == (ptDmem = (MSYS_DMEM_INFO *)kzalloc(sizeof(MSYS_DMEM_INFO), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s kzalloc MSYS_DMEM_INFO fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        snprintf(ptDmem->name, 15, szName);
        ptDmem->length = nSize;

        if(0 != msys_request_dmem(ptDmem))
        {
            printk(KERN_WARNING "%s msys_request_dmem fail\n", __FUNCTION__);
            kfree(ptDmem);
            eRet = CAM_OS_FAIL;
            break;
        }

        ASSIGN_POINTER_VALUE(ppVirtPrt, (void *)(uintptr_t)ptDmem->kvirt);
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)Chip_Phys_to_MIU(ptDmem->phys));
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptDmem->phys);

        printk(KERN_INFO "%s <%s> physAddr<0x%08X> size<%d>  \r\n",
               __FUNCTION__,
               szName,
               (u32)ptDmem->phys,
               (u32)ptDmem->length);

        CamOsMutexLock(&_gtMemLock);
        ptNewEntry = (MemoryList_t*) kzalloc(sizeof(MemoryList_t), GFP_KERNEL);
        ptNewEntry->pPtr = (void *)(uintptr_t)ptDmem->kvirt;
        ptNewEntry->pMemifoPtr = (void *) ptDmem;
        ptNewEntry->szName = (char *)kzalloc(strlen(szName), GFP_KERNEL);
        if (ptNewEntry->szName)
            strcpy(ptNewEntry->szName, szName);
        ptNewEntry->nSize = ptDmem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        CamOsMutexUnlock(&_gtMemLock);
    }
    while(0);
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemAllocEx(const char* szName,
                               u32 nSize,
                               void** ppVirtPrt,
                               void** ppPhysPtr,
                               void** ppMiuPtr,
                               void** ppKvirtPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    u8 nAllocSucc = TRUE;
    void *pNonCachePtr = NULL;

    pNonCachePtr = MsAllocateNonCacheMemExt(nSize, 12);
    nAllocSucc &= MsIsHeapMemory(pNonCachePtr);

    if((u32)pNonCachePtr & ((1 << 6) - 1))
    {
        nAllocSucc &= FALSE;
        MsReleaseMemory(pNonCachePtr);
    }

    if(nAllocSucc == TRUE)
    {
        ASSIGN_POINTER_VALUE(ppVirtPrt, pNonCachePtr);
        ASSIGN_POINTER_VALUE(ppPhysPtr, MsVA2PA(pNonCachePtr));
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)HalUtilPHY2MIUAddr((u32)*ppPhysPtr));
    }
    else
    {
        ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
        ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
        ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
        eRet = CAM_OS_FAIL;
    }

    /*CamOsPrintf("%s    0x%08X  0x%08X  0x%08X\r\n",
            __FUNCTION__,
            (u32)*ppVirtPrt,
            (u32)*ppPhysPtr,
            (u32)*ppMiuPtr);*/

    _CheckDmemInfoListInited();

    CamOsMutexLock(&_gtMemLock);
    MemoryList_t* ptNewEntry = (MemoryList_t*) MsAllocateMem(sizeof(MemoryList_t));
    ptNewEntry->pPtr = pNonCachePtr;
    ptNewEntry->pMemifoPtr = NULL;
    ptNewEntry->szName = (char *)MsAllocateMem(strlen(szName));
    if (ptNewEntry->szName)
        strcpy(ptNewEntry->szName, szName);
    ptNewEntry->nSize = nSize;
    CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
    CamOsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysFd = -1;
    s32 nMemFd = -1;
    MSYS_DMEM_INFO * ptMsysMem = NULL;
    unsigned char* pMmapPtr = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                fprintf(stderr, "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))  //O_DIRECT)))  //O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/mem failed (%d)!!\n", __FUNCTION__, errno);
            eRet = CAM_OS_FAIL;
            break;
        }

        ptMsysMem = (MSYS_DMEM_INFO *) malloc(sizeof(MSYS_DMEM_INFO));
        MSYS_ADDR_TRANSLATION_INFO tAddrInfo;
        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);
        FILL_VERCHK_TYPE(*ptMsysMem, ptMsysMem->VerChk_Version,
                         ptMsysMem->VerChk_Size, IOCTL_MSYS_VERSION);

        ptMsysMem->length = nSize;
        snprintf(ptMsysMem->name, sizeof(ptMsysMem->name), "%s", szName);

        if(ioctl(nMsysFd, IOCTL_MSYS_REQUEST_DMEM, ptMsysMem))
        {
            ptMsysMem->length = 0;
            fprintf(stderr, "%s [%s][%d]Request Direct Memory Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        if (ptMsysMem->length < nSize)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr,"%s [%s]Request Direct Memory Failed!! because dmem size <%d>smaller than <%d>\n",
                    __FUNCTION__, szName, ptMsysMem->length, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        tAddrInfo.addr = ptMsysMem->phys;
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptMsysMem->phys);
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s][%d]IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        //sylvia
       /* if(ioctl(nMsysFd, IOCTL_MSYS_FLUSH_MEMORY, 1))
        {
             fprintf(stderr, "%s [%s]IOCTL_MSYS_FLUSH_MEMORY Failed!!\n", __FUNCTION__, szName);
        }*/

        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)tAddrInfo.addr);
        pMmapPtr = mmap(0, ptMsysMem->length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        nMemFd, ptMsysMem->phys);
        if(pMmapPtr == (void *) - 1)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s failed!! physAddr<0x%x> size<0x%x> errno<%d, %s> \r\n",
                    __FUNCTION__,
                    (u32)ptMsysMem->phys,
                    (u32)ptMsysMem->length, errno, strerror(errno));
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppVirtPrt, pMmapPtr);
        ASSIGN_POINTER_VALUE(ppKvirtPtr, (void *)(uintptr_t)ptMsysMem->kvirt);

        fprintf(stderr, "%s <%s> physAddr<0x%x> size<%d> kvirt<0x%x>\r\n",
                __FUNCTION__,
                szName, (u32)ptMsysMem->phys,
                (u32)ptMsysMem->length,
                (u32)ptMsysMem->kvirt);

        CamOsMutexLock(&_gtMemLock);
        MemoryList_t* ptNewEntry = (MemoryList_t*) malloc(sizeof(MemoryList_t));
        ptNewEntry->pPtr = pMmapPtr;
        ptNewEntry->pMemifoPtr = (void *) ptMsysMem;
        ptNewEntry->szName = strdup(szName);
        ptNewEntry->nSize = ptMsysMem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        CamOsMutexUnlock(&_gtMemLock);
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
    if(nMemFd >= 0)
    {
        close(nMemFd);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *ptDmem = NULL;
    MemoryList_t* ptNewEntry;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
    ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
    ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                printk(KERN_WARNING "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPrt, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 == (ptDmem = (MSYS_DMEM_INFO *)kzalloc(sizeof(MSYS_DMEM_INFO), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s kzalloc MSYS_DMEM_INFO fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        snprintf(ptDmem->name, 15, szName);
        ptDmem->length = nSize;

        if(0 != msys_request_dmem(ptDmem))
        {
            printk(KERN_WARNING "%s msys_request_dmem fail\n", __FUNCTION__);
            kfree(ptDmem);
            eRet = CAM_OS_FAIL;
            break;
        }

        ASSIGN_POINTER_VALUE(ppVirtPrt, (void *)(uintptr_t)ptDmem->kvirt);
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)Chip_Phys_to_MIU(ptDmem->phys));
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptDmem->phys);

        printk(KERN_INFO "%s <%s> physAddr<0x%08X> size<%d>  \r\n",
               __FUNCTION__,
               szName,
               (u32)ptDmem->phys,
               (u32)ptDmem->length);

        CamOsMutexLock(&_gtMemLock);
        ptNewEntry = (MemoryList_t*) kzalloc(sizeof(MemoryList_t), GFP_KERNEL);
        ptNewEntry->pPtr = (void *)(uintptr_t)ptDmem->kvirt;
        ptNewEntry->pMemifoPtr = (void *) ptDmem;
        ptNewEntry->szName = (char *)kzalloc(strlen(szName), GFP_KERNEL);
        if (ptNewEntry->szName)
            strcpy(ptNewEntry->szName, szName);
        ptNewEntry->nSize = ptDmem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        CamOsMutexUnlock(&_gtMemLock);
    }
    while(0);
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemRelease(void* pVirtPtr, u32 nSize)
{
    CamOsRet_e eRet = CAM_OS_OK;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;
#ifdef CAM_OS_RTK
    if(pVirtPtr)
    {
        MsReleaseMemory(pVirtPtr);
        //CamOsPrintf("%s do release\n\r", __FUNCTION__);

        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr == pVirtPtr)
            {
                if(ptTmp->szName)
                    MsReleaseMemory(ptTmp->szName);
                CAM_OS_LIST_DEL(ptPos);
                MsReleaseMemory(ptTmp);
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysfd = -1;
    s32 nErr = 0;
    MSYS_DMEM_INFO *pMsysMem = NULL;

    if(pVirtPtr)
    {
        do
        {
            if(0 > (nMsysfd = open("/dev/msys", O_RDWR | O_SYNC )))
            {
                fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
                eRet = CAM_OS_FAIL;
                break;
            }

            _CheckDmemInfoListInited();

            CamOsMutexLock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    pMsysMem = (MSYS_DMEM_INFO *) ptTmp->pMemifoPtr;
                    break;
                }
            }
            CamOsMutexUnlock(&_gtMemLock);
            if(pMsysMem == NULL)
            {
                fprintf(stderr, "%s find Msys_DMEM_Info node failed!! <0x%08X>  \r\n", __FUNCTION__, (u32)pVirtPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            if (nSize != pMsysMem->length)
            {
                nErr = munmap((void *)pVirtPtr, pMsysMem->length);
                if (0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s> \r\n",
                            __FUNCTION__, (u32)pVirtPtr, (u32)pMsysMem->length, nErr, errno, strerror(errno));
                }
            }
            else
            {
                nErr = munmap((void *)pVirtPtr, nSize);
                if(0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s> \r\n",
                            __FUNCTION__, (u32)pVirtPtr, (u32)nSize, nErr, errno, strerror(errno));
                }
            }

            if(ioctl(nMsysfd, IOCTL_MSYS_RELEASE_DMEM, pMsysMem))
            {
                fprintf(stderr, "%s : IOCTL_MSYS_RELEASE_DMEM error physAddr<0x%x>\n", __FUNCTION__, (u32)pMsysMem->phys);
                eRet = CAM_OS_FAIL;
                break;
            }
            if(pMsysMem)
            {
                free(pMsysMem);
                pMsysMem = NULL;
            }
            CamOsMutexLock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    if(ptTmp->szName)
                        free(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    free(ptTmp);
                }
            }
            CamOsMutexUnlock(&_gtMemLock);
        }
        while(0);

        if(nMsysfd >= 0)
        {
            close(nMsysfd);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *tpDmem = NULL;

    if(pVirtPtr)
    {
        do
        {
            _CheckDmemInfoListInited();

            CamOsMutexLock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                //printk("search tmp->ptr: %08X  %s\n", (u32)ptTmp->pPtr, ptTmp->szName);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    tpDmem = ptTmp->pMemifoPtr;
                    //printk("search(2) pdmem->name: %s\n", tpDmem->name);
                    break;
                }
            }
            CamOsMutexUnlock(&_gtMemLock);
            if(tpDmem == NULL)
            {
                printk(KERN_WARNING "%s find Msys_DMEM_Info node failed!! <0x%08X>  \r\n", __FUNCTION__, (u32)pVirtPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            msys_release_dmem(tpDmem);

            if(tpDmem)
            {
                kfree(tpDmem);
                tpDmem = NULL;
            }
            CamOsMutexLock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPtr == pVirtPtr)
                {
                    if(ptTmp->szName)
                        kfree(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    kfree(ptTmp);
                }
            }
            CamOsMutexUnlock(&_gtMemLock);
        }
        while(0);
    }
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemFlush(void* pVirtPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    if(pVirtPtr)
    {
        eRet = CAM_OS_FAIL;

        _CheckDmemInfoListInited();

        CamOsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPtr == pVirtPtr)
            {
                eRet = CAM_OS_OK;
                sys_flush_data_cache_buffer((u32)MsVA2PA(ptTmp->pPtr), ptTmp->nSize);
            }
        }
        CamOsMutexUnlock(&_gtMemLock);
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysFd = -1;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(ioctl(nMsysFd, IOCTL_MSYS_FLUSH_MEMORY, 1))
        {
            fprintf(stderr, "%s IOCTL_MSYS_FLUSH_MEMORY Failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    __cpuc_flush_kern_all();
    Chip_Flush_Memory();
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemStat(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;
#ifdef CAM_OS_RTK
    _CheckDmemInfoListInited();

    CamOsMutexLock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            CamOsPrintf("%s memory allocated %p %s\n", __FUNCTION__, ptTmp->pPtr, ptTmp->szName);
        }
    }
    CamOsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    _CheckDmemInfoListInited();

    CamOsMutexLock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            fprintf(stderr, "%s memory allocated 0x%08X %s\n", __FUNCTION__, (u32)ptTmp->pPtr, ptTmp->szName);
        }
    }
    CamOsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    _CheckDmemInfoListInited();

    CamOsMutexLock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pPtr)
        {
            printk(KERN_WARNING "%s memory allocated 0x%08X %s\n", __FUNCTION__, (u32)ptTmp->pPtr, ptTmp->szName);
        }
    }
    CamOsMutexUnlock(&_gtMemLock);
#endif
    return eRet;
}

void* CamOsDirectMemPhysToMiu(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilPHY2MIUAddr((u32)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysFd = -1;
    void *nMiuAddr = 0;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__);
            break;
        }
        nMiuAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }

    return nMiuAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)(uintptr_t)Chip_Phys_to_MIU((uintptr_t)pPtr);
#endif
}

void* CamOsDirectMemMiuToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilMIU2PHYAddr((u32)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    s32 nMsysFd = -1;
    void *nPhysAddr = 0;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_MIU_TO_PHYS, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_MIU_TO_PHYS Failed!!\n", __FUNCTION__);
            break;
        }
        nPhysAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }

    return nPhysAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)(uintptr_t)Chip_MIU_to_Phys((uintptr_t)pPtr);
#endif
}

void* CamOsDirectMemPhysToVirt(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsPA2VA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement PhysToVirt in linux user space.
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)phys_to_virt((unsigned long)pPtr);
#endif
}

void* CamOsDirectMemVirtToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsVA2PA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement VirtToPhys in linux user space.
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)virt_to_phys(pPtr);
#endif
}

#if 0
CamOsRet_e CamOsPropertySet(const char *szKey, const char *szValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    s32(*dlsym_property_set)(const char *szKey, const char *szValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error \n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set = dlsym(pLibHandle, "property_set");
        if(NULL == dlsym_property_set)
        {
            fprintf(stderr, "%s : dlsym property_set failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set(szKey, szValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

CamOsRet_e CamOsPropertyGet(const char *szKey, char *szValue, const char *szDefaultValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    s32(*dlsym_property_get)(const char *szKey, char *szValue, const char *szDefaultValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error \n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        dlsym_property_get = dlsym(pLibHandle, "property_get");
        if(NULL == dlsym_property_get)
        {
            fprintf(stderr, "%s : dlsym property_get failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_get(szKey, szValue, szDefaultValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}
#endif

u64 CamOsMathDivU64(u64 nDividend, u64 nDivisor, u64 *pRemainder)
{
#ifdef CAM_OS_RTK
    return div64_u64_rem(nDividend, nDivisor, pRemainder);
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    return div64_u64_rem(nDividend, nDivisor, pRemainder);
#endif
}

s64 CamOsMathDivS64(s64 nDividend, s64 nDivisor, s64 *pRemainder)
{
#ifdef CAM_OS_RTK
    s64 nQuotient = div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    s64 nQuotient = div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#endif
}

u32 CamOsCopyFromUpperLayer(void *pTo, const void *pFrom, u32 nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_from_user(pTo, pFrom, nLen);
#endif
}

u32 CamOsCopyToUpperLayer(void *pTo, const void * pFrom, u32 nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_to_user(pTo, pFrom, nLen);
#endif
}

s32 CamOsAtomicRead(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_read((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

void CamOsAtomicSet(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        atomic_set((atomic_t *)ptAtomic, nValue);
    }
#endif
}

s32 CamOsAtomicAddReturn(s32 nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicSubReturn(s32 nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicSubAndTest(s32 nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_and_test(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicIncReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return __sync_fetch_and_add(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return __sync_fetch_and_add(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicDecReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return __sync_fetch_and_sub(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return __sync_fetch_and_sub(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicIncAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicDecAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicAddNegative(s32 nValue, CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if (ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_USER)
    if (ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_negative(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

CamOsMemSize_e CamOsPhysMemSize(void)
{
#ifdef CAM_OS_RTK
    return (CamOsMemSize_e)((*(volatile u32 *)(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    s32 nMemFd = -1;
    u32 nRegs = 0;
    u32 nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CamOsPrintf("%s open /dev/mem failed!!\n", __FUNCTION__);
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ|PROT_WRITE, MAP_SHARED , nMemFd, (RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CamOsPrintf("%s mmap failed!!\n", __FUNCTION__);
            break;
        }
        else
        {
            nRegs = *(u32 *)(map_addr + RIU_MEM_SIZE_OFFSET%nPageSize);
            if (munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    CamOsPrintf("CamOsPhysMemSize: %d\n", (nRegs & 0xF000) >> 12);

    return (CamOsMemSize_e)((nRegs & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemSize_e eMemSize;
    void *pRegs;

    if ((pRegs = ioremap(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET, 4)) != NULL)
    {
        eMemSize = (CamOsMemSize_e)((readl(pRegs) & 0xF000)>>12);
        iounmap(pRegs);
    }
    else
    {
        printk(KERN_WARNING "%s ioremap fail\n", __FUNCTION__);
    }

    return eMemSize;
#endif
}

u32 CamOsChipId(void)
{
#ifdef CAM_OS_RTK
    return *(volatile u32 *)(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    s32 nMemFd = -1;
    u32 nRegs = 0;
    u32 nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CamOsPrintf("%s open /dev/mem failed!!\n", __FUNCTION__);
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ|PROT_WRITE, MAP_SHARED , nMemFd, (RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CamOsPrintf("%s mmap failed!!\n", __FUNCTION__);
            break;
        }
        else
        {
            nRegs = *(u32 *)(map_addr + RIU_CHIP_ID_OFFSET%nPageSize);
            if (munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    CamOsPrintf("CamOsPhysMemSize: %d\n", nRegs & 0x3FFFFF);

    return nRegs & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_KERNEL)
    u32 nId;
    void *pRegs;

    if ((pRegs = ioremap(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET, 4)) != NULL)
    {
        nId = readl(pRegs) & 0x3FFFFF;
        iounmap(pRegs);
    }
    else
    {
        printk(KERN_WARNING "%s ioremap fail\n", __FUNCTION__);
    }

    return nId;
#endif
}
