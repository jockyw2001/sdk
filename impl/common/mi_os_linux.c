#include "mi_os.h"
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/math64.h>
#include "mi_common_macro.h"

typedef struct {
    struct task_struct *task;
    void (*pfnStartRoutine)(void*);
    void *pArg;
} _MI_OS_ThreadWrapper_t;

#define static_assert _Static_assert

static_assert(sizeof(MI_OS_Mutex_t) >= sizeof(struct mutex), "fail mutex size check");
static_assert(sizeof(MI_OS_Semaphore_t) >= sizeof(struct semaphore), "fail semaphore size check");
static_assert(sizeof(MI_OS_WaitCond_t) >= sizeof(struct completion), "fail completion size check");

MI_S32 MI_OS_Printf(const char *szFmt, ...){
    va_list ap;

    va_start(ap, szFmt);
    vprintk(szFmt, ap);
    va_end(ap);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MsSleep(MI_U32 nMsec){
    msleep(nMsec);
    return MI_SUCCESS;
}

MI_S32 MI_OS_GetTimeOfDay(MI_OS_TimeSpec_t *ptRes){
    struct timespec ts;
    ktime_get_real_ts(&ts);
    ptRes->u32Sec = ts.tv_sec;
    ptRes->u32NanoSec = ts.tv_nsec;
    return MI_SUCCESS;
}

MI_S32 MI_OS_SetTimeOfDay(const MI_OS_TimeSpec_t *ptRes){
    struct timespec ts = {
        .tv_sec = ptRes->u32Sec,
        .tv_nsec = ptRes->u32NanoSec,
    };
    do_settimeofday(&ts);
    return MI_SUCCESS;
}

MI_S32 MI_OS_GetMonotonicTime(MI_OS_TimeSpec_t *ptRes){
    struct timespec ts;
    ktime_get_ts(&ts);
    ptRes->u32Sec = ts.tv_sec;
    ptRes->u32NanoSec = ts.tv_nsec;
    return MI_SUCCESS;
}

static int _MI_OS_ThreadWrapper(void *arg){
    _MI_OS_ThreadWrapper_t *cw = arg;
    cw->pfnStartRoutine(cw->pArg);
    kfree(cw);
    return 0;
}

MI_S32 MI_OS_ThreadCreate(MI_OS_Thread_t *pThread, MI_OS_ThreadAttr_t *ptAttrb, void (*pfnStartRoutine)(void*), void *pArg){
    _MI_OS_ThreadWrapper_t *cw = kzalloc(sizeof(*cw), GFP_KERNEL);
    if(!cw){
        printk(KERN_ERR "[%s] can't alloc memory\n", __func__);
        return MI_OS_ERR_ALLOC;
    }
    cw->task = kthread_create(_MI_OS_ThreadWrapper, cw, "cam_os");
    if(!cw->task){
        printk(KERN_ERR "[%s] can't create thread\n", __func__);
        kfree(cw);
        return MI_OS_ERR_FAILED;
    }
    if(ptAttrb->u16Priority > 0 && ptAttrb->u16Priority < 100){
        struct sched_param param = { .sched_priority = ptAttrb->u16Priority };
        sched_setscheduler_nocheck(cw->task, SCHED_NORMAL, &param);
    }
    if(ptAttrb->u16StackSize > 0){
        printk(KERN_ERR "[%s] don't set kernel thread stack\n", __func__);
    }
    cw->pfnStartRoutine = pfnStartRoutine;
    cw->pArg = pArg;
    *pThread = cw;
    wake_up_process(cw->task);
    return 0;
}

MI_S32 MI_OS_ThreadStop(MI_OS_Thread_t thread){
    _MI_OS_ThreadWrapper_t *cw = thread;
    kthread_stop(cw->task);
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadShouldStop(MI_BOOL *bStop){
    *bStop = kthread_should_stop();
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadSetName(const char *szName){
    snprintf(current->comm, sizeof(current->comm), "%s", szName);
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadGetName(char *szName, u32 nLen){
    strncpy(szName, current->comm, nLen-1);
    szName[nLen-1] = '\0';
    return MI_SUCCESS;
}

MI_S32 MI_OS_ThreadGetId(MI_U32 *u32Id){
    *u32Id = current->pid;
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexInit(MI_OS_Mutex_t *ptMutex){
    mutex_init((struct mutex*)ptMutex);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexDestroy(MI_OS_Mutex_t *ptMutex){
    mutex_destroy((struct mutex*)ptMutex);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexLock(MI_OS_Mutex_t *ptMutex){
    mutex_lock_killable((struct mutex*)ptMutex);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MutexUnlock(MI_OS_Mutex_t *ptMutex){
    mutex_unlock((struct mutex*)ptMutex);
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreInit(MI_OS_Semaphore_t *ptTsem, MI_U32 nVal){
    sema_init((struct semaphore*)ptTsem, nVal);
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreDestroy(MI_OS_Semaphore_t *ptTsem){
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreUp(MI_OS_Semaphore_t *ptTsem){
    up((struct semaphore*)ptTsem);
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreDown(MI_OS_Semaphore_t *ptTsem){
    down_killable((struct semaphore*)ptTsem);
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreTimedDown(MI_OS_Semaphore_t *ptTsem, MI_U32 nMsec){
    if(down_timeout((struct semaphore*)ptTsem, msecs_to_jiffies(nMsec))
        == -ETIME){
        return MI_OS_ERR_TIMEOUT;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_SemaphoreGetValue(MI_OS_Semaphore_t *ptTsem, MI_U32 *value){
    struct semaphore *sem = (struct semaphore*)ptTsem;
    *value = sem->count;
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondInit(MI_OS_WaitCond_t *pCond){
    struct completion *com = (struct completion *)pCond;
    init_completion(com);
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondDestroy(MI_OS_WaitCond_t *pCond){
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondSignal(MI_OS_WaitCond_t *pCond){
    struct completion *com = (struct completion *)pCond;
    complete(com);
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCond(MI_OS_WaitCond_t *pCond){
    struct completion *com = (struct completion *)pCond;
    wait_for_completion_killable(com);
    return MI_SUCCESS;
}

MI_S32 MI_OS_WaitCondTimed(MI_OS_WaitCond_t *pCond, MI_U32 nMsec){
    struct completion *com = (struct completion *)pCond;
    if(wait_for_completion_killable_timeout(com, msecs_to_jiffies(nMsec)) == 0){
        return MI_OS_ERR_TIMEOUT;
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemAlloc(u32 nSize, void **ppPtr){
    *ppPtr = kmalloc(nSize, GFP_KERNEL);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemCalloc(u32 nNum, u32 nSize, void **ppPtr){
    *ppPtr = kcalloc(nNum, nSize, GFP_KERNEL);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MemRelease(void *ptr){
    kfree(ptr);
    return MI_SUCCESS;
}

MI_S32 MI_OS_MathDivU64(MI_U64 nDividend, MI_U64 nDivisor, MI_U64 *pRemainder, MI_U64 *pQuot){
    *pQuot = div64_u64(nDividend, nDivisor);
    if(pRemainder){
        *pRemainder = nDividend - (*pQuot * nDivisor);
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_MathDivS64(MI_S64 nDividend, MI_S64 nDivisor, MI_S64 *pRemainder, MI_S64 *pQuot){
    *pQuot = div64_s64(nDividend, nDivisor);
    if(pRemainder){
        *pRemainder = nDividend - (*pQuot * nDivisor);
    }
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicRead(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = atomic_read((atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSet(MI_OS_Atomic_t *ptAtomic, MI_S32 nValue){
    atomic_set((atomic_t*)ptAtomic, nValue);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicAddReturn(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = atomic_add_return(nValue, (atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSubReturn(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = atomic_sub_return(nValue, (atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicSubAndTest(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    *bValue = atomic_sub_and_test(nValue, (atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicIncReturn(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = atomic_inc_return((atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicDecReturn(MI_OS_Atomic_t *ptAtomic, MI_S32 *s32Value){
    *s32Value = atomic_dec_return((atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicIncAndTest(MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    *bValue = atomic_inc_and_test((atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicDecAndTest(MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    *bValue = atomic_dec_and_test((atomic_t*)ptAtomic);
    return MI_SUCCESS;
}

MI_S32 MI_OS_AtomicAddNegative(MI_S32 nValue, MI_OS_Atomic_t *ptAtomic, MI_BOOL *bValue){
    *bValue = atomic_add_negative(nValue, (atomic_t*)ptAtomic);
    return MI_SUCCESS;
}
