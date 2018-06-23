#include "mi_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

static pthread_key_t _stThreadKey;
static pthread_once_t _stThreadOnce = PTHREAD_ONCE_INIT;

typedef struct {
    pthread_t tid;
    void (*pfnStartRoutine)(void*);
    void *pArg;
    MI_BOOL bShouldStop;
} _MI_OS_ThreadWrapper_t;

#define static_assert _Static_assert
static_assert(sizeof(MI_OS_Mutex_t) >= sizeof(pthread_mutex_t), "fail mutex size check");
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} MI_OS_WaitCondWrapper_t;
static_assert(sizeof(MI_OS_WaitCond_t) >= sizeof(MI_OS_WaitCondWrapper_t), "fail wait condition size check");
typedef struct {
    MI_OS_WaitCondWrapper_t wc;
    MI_S32 s32Value;
} MI_OS_SemaphoreWrapper_t;
static_assert(sizeof(MI_OS_Semaphore_t) >= sizeof(MI_OS_SemaphoreWrapper_t), "fail semaphore size check");

MI_S32 MI_OS_Printf(const char *szFmt, ...){
    va_list ap;

    va_start(ap, szFmt);
    vprintf(szFmt, ap);
    va_end(ap);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MsSleep(MI_U32 nMsec){
    usleep(nMsec*1000);
    return MI_SUCCESS;
}

MI_S32 MI_OS_GetTimeOfDay(MI_OS_TimeSpec_t *ptRes){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ptRes->u32Sec = ts.tv_sec;
    ptRes->u32NanoSec = ts.tv_nsec;
    return MI_SUCCESS;
}

MI_S32 MI_OS_SetTimeOfDay(const MI_OS_TimeSpec_t *ptRes){
    struct timespec ts = {
        .tv_sec = ptRes->u32Sec,
        .tv_nsec = ptRes->u32NanoSec,
    };
    clock_settime(CLOCK_REALTIME, &ts);
    return MI_SUCCESS;
}

MI_S32 MI_OS_GetMonotonicTime(MI_OS_TimeSpec_t *ptRes){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ptRes->u32Sec = ts.tv_sec;
    ptRes->u32NanoSec = ts.tv_nsec;
    return MI_SUCCESS;
}

static void _MI_OS_ThreadKeyDestroy(void *arg){
    free(arg);
}

static void _MI_OS_ThreadKeyCreate(void){
    pthread_key_create(&_stThreadKey, _MI_OS_ThreadKeyDestroy);
}

static void *_MI_OS_ThreadWrapper(void *arg){
    _MI_OS_ThreadWrapper_t *cw = arg;
    pthread_once(&_stThreadOnce, _MI_OS_ThreadKeyCreate);
    pthread_setspecific(_stThreadKey, cw);
    cw->pfnStartRoutine(cw->pArg);
    return arg;
}

MI_S32 MI_OS_ThreadCreate(MI_OS_Thread_t *pThread, MI_OS_ThreadAttr_t *ptAttr, void (*pfnStartRoutine)(void*), void *pArg){
    int rval;
    pthread_attr_t attr;
    _MI_OS_ThreadWrapper_t *cw;
    rval = pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if(ptAttr->u16Priority > 0 && ptAttr->u16Priority < 100){
        struct sched_param param = {.sched_priority = ptAttr->u16Priority};
        rval = pthread_attr_setschedparam(&attr, &param);
        if(rval != 0){
            printf("[%s] set schedparam failed!(%s)\n", __func__, strerror(rval));
        }
    }
    if(ptAttr->u16StackSize > 0){
        rval = pthread_attr_setstacksize(&attr, ptAttr->u16StackSize);
        if(rval != 0){
            printf("[%s] set stacksize failed!(%s)\n", __func__, strerror(rval));
        }
    }
    cw = malloc(sizeof(_MI_OS_ThreadWrapper_t));
    if(!cw){
        printf("[%s] malloc object failed!\n", __func__);
        return E_MI_ERR_NOMEM;
    }
    cw->pfnStartRoutine = pfnStartRoutine;
    cw->pArg = pArg;
    cw->bShouldStop = false;
    rval = pthread_create(&cw->tid, &attr, _MI_OS_ThreadWrapper, cw);
    if(rval != 0){
        printf("[%s] create thread failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    *pThread = cw;
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadStop(MI_OS_Thread_t thread){
    _MI_OS_ThreadWrapper_t *cw = thread;
    void *arg;
    cw->bShouldStop = true;
    int rval = pthread_join(cw->tid, &arg);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadShouldStop(MI_BOOL *bStop){
    _MI_OS_ThreadWrapper_t *cw = pthread_getspecific(_stThreadKey);
    *bStop = cw->bShouldStop;
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadSetName(const char *szName){
    prctl(PR_SET_NAME, (unsigned long)szName);
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadGetName(char *szName, MI_U32 nLen){
    prctl(PR_GET_NAME, (unsigned long)szName);
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadGetId(MI_U32 *u32Id){
    *u32Id = syscall(SYS_gettid);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexInit(MI_OS_Mutex_t *ptMutex){
    int rval = pthread_mutex_init((pthread_mutex_t*)ptMutex, NULL);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexDestroy(MI_OS_Mutex_t *ptMutex){
    int rval = pthread_mutex_destroy((pthread_mutex_t*)ptMutex);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexLock(MI_OS_Mutex_t *ptMutex){
    int rval = pthread_mutex_lock((pthread_mutex_t*)ptMutex);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexUnlock(MI_OS_Mutex_t *ptMutex){
    int rval = pthread_mutex_unlock((pthread_mutex_t*)ptMutex);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(rval));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreInit(MI_OS_Semaphore_t *ptTsem, MI_U32 nVal){
    int rval = sem_init((sem_t*)ptTsem, 0, nVal);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreDestroy(MI_OS_Semaphore_t *ptTsem){
    int rval = sem_destroy((sem_t*)ptTsem);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreUp(MI_OS_Semaphore_t *ptTsem){
    int rval = sem_post((sem_t*)ptTsem);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreDown(MI_OS_Semaphore_t *ptTsem){
    int rval = sem_wait((sem_t*)ptTsem);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreTimedDown(MI_OS_Semaphore_t *ptTsem, MI_U32 nMsec){
    struct timespec ts;
    div_t d = div(nMsec, 1000);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += d.quot;
    ts.tv_nsec += d.rem * 1000*1000;
    if(ts.tv_nsec > 1000*1000*1000){
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000*1000*1000;
    }
    int rval = sem_timedwait((sem_t*)ptTsem, &ts);
    if(rval != 0){
        if(errno != ETIMEDOUT){
            printf("[%s] failed!(%s)\n", __func__, strerror(errno));
            return MI_OS_ERR_FAILED;
        }
        return MI_OS_ERR_TIMEOUT;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreGetValue(MI_OS_Semaphore_t *ptTsem, MI_U32 *u32Value){
    int rval, value;
    rval = sem_getvalue((sem_t*)ptTsem, &value);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    *u32Value = value;
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondInit(MI_OS_WaitCond_t *pCond){
    MI_OS_WaitCondWrapper_t *wcw = (MI_OS_WaitCondWrapper_t*)pCond;
    int rval = pthread_mutex_init(&wcw->mutex, NULL);
    if(rval != 0){
        printf("[%s] init mutex failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    rval = pthread_cond_init(&wcw->cond, &attr);
    pthread_condattr_destroy(&attr);
    if(rval != 0){
        printf("[%s] init cond failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondSignal(MI_OS_WaitCond_t *pCond){
    MI_OS_WaitCondWrapper_t *wcw = (MI_OS_WaitCondWrapper_t*)pCond;
    int rval = pthread_mutex_lock(&wcw->mutex);
    if(rval != 0){
        printf("[%s] mutex lock failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    rval = pthread_cond_signal(&wcw->cond);
    if(rval != 0){
        printf("[%s] cond signal failed!(%s)\n", __func__, strerror(errno));
        pthread_mutex_unlock(&wcw->mutex);
        return MI_OS_ERR_FAILED;
    }
    rval = pthread_mutex_unlock(&wcw->mutex);
    if(rval != 0 && errno != EAGAIN){
        printf("[%s] mutex unlock failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCond(MI_OS_WaitCond_t *pCond){
    MI_OS_WaitCondWrapper_t *wcw = (MI_OS_WaitCondWrapper_t*)pCond;
    int rval = pthread_cond_wait(&wcw->cond, &wcw->mutex);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondTimed(MI_OS_WaitCond_t *pCond, MI_U32 nMsec){
    MI_OS_WaitCondWrapper_t *wcw = (MI_OS_WaitCondWrapper_t*)pCond;
    struct timespec ts;
    div_t d = div(nMsec, 1000);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += d.quot;
    ts.tv_nsec += d.rem * 1000*1000;
    if(ts.tv_nsec > 1000*1000*1000){
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000*1000*1000;
    }
    int rval = pthread_cond_timedwait(&wcw->cond, &wcw->mutex, &ts);
    if(rval != 0){
        printf("[%s] failed!(%s)\n", __func__, strerror(errno));
        return MI_OS_ERR_FAILED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemAlloc(MI_U32 nSize, void **pPtr){
    *pPtr = malloc(nSize);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemCalloc(MI_U32 nNum, MI_U32 nSize, void **pPtr){
    *pPtr = calloc(nNum, nSize);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemRelease(void *ptr){
    free(ptr);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MathDivU64(MI_U64 nDividend, MI_U64 nDivisor, MI_U64 *pRemainder, MI_U64 *pQuot){
    lldiv_t lld = lldiv(nDividend, nDivisor);
    if(pRemainder){
        *pRemainder = lld.rem;
    }
    if(pQuot){
        *pQuot = lld.quot;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MathDivS64(MI_S64 nDividend, MI_S64 nDivisor, MI_S64 *pRemainder, MI_S64 *pQuot){
    return MI_OS_MathDivU64(nDividend, nDivisor, pRemainder, pQuot);
}

MI_S32 MI_OS_AtomicRead(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    __sync_synchronize();
    *s32Value = ptAtomic->s32Counter;
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSet(MI_OS_Atomic_t *ptAtomic, MI_S32 nValue){
    MI_S32 oValue;
    do{
        oValue = ptAtomic->s32Counter;
    }while(!__sync_bool_compare_and_swap(&ptAtomic->s32Counter, oValue, nValue));
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicAddReturn(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = __sync_add_and_fetch(&ptAtomic->s32Counter, nValue);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSubReturn(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = __sync_sub_and_fetch(&ptAtomic->s32Counter, nValue);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSubAndTest(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    *bValue = __sync_sub_and_fetch(&ptAtomic->s32Counter, nValue) == 0;
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicIncReturn(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    return MI_OS_AtomicAddReturn(1, ptAtomic, s32Value);
}

MI_S32 MI_OS_AtomicDecReturn(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    return MI_OS_AtomicSubReturn(1, ptAtomic, s32Value);
}

MI_S32 MI_OS_AtomicIncAndTest(MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    MI_S32 value;
    MI_OS_AtomicIncReturn(ptAtomic, &value);
    *bValue = value == 0;
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicDecAndTest(MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    MI_S32 value;
    MI_OS_AtomicDecReturn(ptAtomic, &value);
    *bValue = value == 0;
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicAddNegative(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    MI_S32 value;
    MI_OS_AtomicAddReturn(nValue, ptAtomic, &value);
    *bValue = value < 0;
    return MI_SUCCESS;
}
