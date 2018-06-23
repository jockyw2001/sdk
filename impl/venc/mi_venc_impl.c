#include "mi_venc_impl.h"
//==== Porting ====

/* This is designed to be removed in the end.
 * Because the /project/ for CamOs K6L is not ready in Alkaid, temporarily
 * define this wrapper in VENC module to be Linux native or Cam OS wrappers.
 */
//#define USE_CAM_OS (0)
#define VENC_PARAM(var,var_type,val) extern var_type var;


#include "mi_print.h"
#include "mi_sys_internal.h"
#include "mi_common.h"
#include "mi_venc.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"

#include "mhal_venc.h"
#include "mi_venc_internal.h"

#include "inc/mhal_venc_dummy.h"
#include "inc/venc_util.h"
#include "mhal_cmdq.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_sys_sideband_msg.h"

#include "../sys/mi_sys_buf_mgr.h"

#include <linux/version.h>
#if !USE_CAM_OS
    #include <linux/semaphore.h>
    #include <linux/list.h>
    #include <linux/sched.h>
    #include <linux/kthread.h>
    #include <linux/slab.h>
    #include <linux/types.h>
    #include <linux/spinlock.h>
    #include <linux/delay.h>
    #include <linux/uaccess.h>
#endif


#if defined(__linux__) && defined(CONFIG_ARCH_INFINITY2)//kernel file system
    #define SUPPORT_KFS (1)//kernel file system
    #include "inc/file_access.h"
#else
    #define SUPPORT_KFS (0)//kernel file system
    #define OpenFile(path, flag, mode) NULL
    #define ReadFile(fp, buf, len) 0
    #define WriteFile(fp, buf, len) 0
    #define CloseFile(fp) 0
#endif

/** @file mi_venc_impl.c
 *
 * MI_VENC does not know:
 * - other MI modules
 *   - which MI module a buffer is "input from" or "output to".
 * - HW specified variables:
 *   - IRQ number
 *   - CMDQ number
 *   - Registers
 *   - driver interface other than MHAL_VENC_Drv_t
 *
 * MI_VENC knows:
 * - MHAL components: These might be automatic or data hiding inside MHAL in the future.
 *   This information should be in astMHalDevConfig
 *   - How many and which driver MI_VENC could access. Through MHAL_VENC_Drv_t
 *   - How many instance could each device support.
 *   - How many core for each driver. This is also known as device ID in MI view.
 * - Which MI DEV ID maps to which stMHalDevConfig.
 * - Which CMDQ ID maps to which MHAL device.
 *
 */
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    #define USE_HW_ENC (0) //K6L
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    #define USE_HW_ENC (1) //i2-FPGA and i2, i3
#else
    #error Unknown Linux version
#endif
#define SKIP_IRQ (0) //Skip HW IRQ, HW IRQ is now tested under i2 FPGA and i3 EVB.
/** The delay between HW IP encoding and triggering next ISR_Proc_Thread.
    This should be effective while SKIP_IRQ == 1. */
#define FPGA_DELAY_MS (1 * 1000)
#define VENC_USE_CMDQ (1) //This is supported only in Infinity

#define MIN_CMDQ_CNT_PER_FRAME 200
#define DBG_INPUT_FRAME (VENC_BOARD == 2 && VENC_FPGA)

#define TASK_CHECK_INTERVAL (50) //How many ms to check the task again.
#define CMDQ_WAIT_TIME_OUT (4) //How many TASK_CHECK_INTERVAL are considered as timeout

//encode device semaphore lock timing tuning
#define TIMING_IN_CALLBACK (3)
#define TIMING_IN_GOT_A_FRAME (4) //have branches dead-lock
#define TIMING_IN_ENCODE_FRAME (9) //FAIL

#define TIMING_ENCODE_FINISH_AND_RELEASE (11)
#define TIMING_ENCODE_DONE (20) //FAIL

#define DEV_DOWN TIMING_IN_GOT_A_FRAME
#define DEV_UP   TIMING_ENCODE_FINISH_AND_RELEASE
//TIMING_IN_ENCODE_FRAME(9) - TIMING_ENCODE_DONE(20) :NG
//TIMING_IN_ENCODE_FRAME(9) - TIMING_ENCODE_FINISH_AND_RELEASE(11) :NG
//TIMING_IN_GOT_A_FRAME(4) - TIMING_ENCODE_FINISH_AND_RELEASE(11): OK

//designed testing setting
/* i3, i2-FGPA
 *   VENC_FPGA == 1, SKIP_IRQ == 0, FPGA_DELAY_MS don't care, VENC_USE_CMDQ == 1,
 *   for 2 channels H264 testing (venc 2 h264 h264) MOCK_HW_FRAME_LEN == 1
 *   others MOCK_HW_FRAME_LEN == 0
 * k6l could be tested with dummy only for now.
 *   VENC_FPGA == 0, SKIP_IRQ == 1
 */


#if USE_CAM_OS
    #define DOWN(x) if(1){CamOsTsemDown(x);}
    #define UP(x) if(1){CamOsTsemUp(x);}
    typedef CamOsTsem_t MI_VENC_Sem_t;
    #define VENC_SEM_INIT CamOsTsemInit
    #define MI_VENC_ThreadShouldStop() (CamOsThreadShouldStop() == CAM_OS_OK)
    #define MI_VENC_WAKE_UP_QUEUE_IF_NECESSARY(waitqueue)\
        do {\
            if(CamOsTcondWaitActive(&(waitqueue))) \
                CamOsTcondSignalAll(&(waitqueue)); \
        }while(0)
    typedef CamOsTcond_t MI_VENC_Wait_t;
    #define MI_VENC_TimedWaitInterruptible CamOsTcondTimedWaitInterruptible
    #define MI_VENC_TimedScheduleInterruptible(waitms) CamOsThreadSchedule(true, waitms)
    #define MI_VENC_InitCond(cond) CamOsTcondInit(cond)

    #define MI_VENC_Thread_t CamOsThread
    #define MI_VENC_ThreadWakeUp CamOsThreadWakeUp
    #define MI_VENC_ThreadStop   CamOsThreadStop
    #define MI_VENC_MsSleep CamOsMsSleep
    #define MI_VENC_CopyFromUser CamOsCopyFromUpperLayer
#else
    #define DOWN(x) if(1){down(x);}
    #define UP(x) if(1){up(x);}
    typedef struct semaphore MI_VENC_Sem_t;
    #define VENC_SEM_INIT sema_init
    #define MI_VENC_ThreadShouldStop() (kthread_should_stop())
    #define MI_VENC_WAKE_UP_QUEUE_IF_NECESSARY(waitqueue) WAKE_UP_QUEUE_IF_NECESSARY(waitqueue)
    typedef wait_queue_head_t MI_VENC_Wait_t;
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
    #if VENC_FPGA
        #define MI_VENC_TimedWaitInterruptible(pWait, cond, waitms)\
            interruptible_sleep_on_timeout(pWait, msecs_to_jiffies(waitms * FPGA_DELAY_MS * 1000));
    #else
        #define MI_VENC_TimedWaitInterruptible(pWait, cond, waitms)\
            interruptible_sleep_on_timeout(pWait, msecs_to_jiffies(waitms));
    #endif
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    #define MI_VENC_TimedWaitInterruptible(pWait, cond, waitms)\
        wait_event_interruptible_timeout(*pWait, cond, msecs_to_jiffies(waitms));
#endif

    #define MI_VENC_TimedScheduleInterruptible schedule_timeout_interruptible
    #define MI_VENC_InitCond(cond) init_waitqueue_head(cond);
    //#define MI_VENC_Thread_t struct task_struct*
    typedef struct task_struct* MI_VENC_Thread_t;
    #define MI_VENC_ThreadWakeUp wake_up_process
    #define MI_VENC_ThreadStop   kthread_stop
    #define MI_VENC_MsSleep msleep
    #define MI_VENC_CopyFromUser copy_from_user
#endif

//==== for list porting ====
#if USE_CAM_OS && 0 //0: Because mi_sys still using Linux list so set 0 to always use Liux list.
    //This section is never full compiled due to MI_SYS
    typedef struct CamOsListHead_t MI_VENC_List_t;
    #define MI_VENC_LIST_IS_EMPTY CAM_OS_LIST_EMPTY
    #define MI_VENC_CONTAINER_OF CAM_OS_CONTAINER_OF
    #define MI_VENC_LIST_NEXT_ENTRY(pos, member) CAM_OS_LIST_NEXT_ENTRY(pos, member)
    #define MI_VENC_LIST_ADD_TAIL(New, head) CAM_OS_LIST_ADD_TAIL(New, head)
    #define MI_VENC_LIST_FOR_EACH_SAFE(pos, n, head) CAM_OS_LIST_FOR_EACH_SAFE(pos, n, head)
    #define MI_VENC_INIT_LIST_HEAD CAM_OS_LIST_HEAD_INIT
#else
    typedef struct list_head MI_VENC_List_t;
    #define MI_VENC_LIST_IS_EMPTY list_empty
    #define MI_VENC_CONTAINER_OF container_of
    #define MI_VENC_LIST_NEXT_ENTRY(pos, member) (pos->member.next)
    #define MI_VENC_LIST_ADD_TAIL(New, head) list_add_tail(New, head)
    #define MI_VENC_LIST_FOR_EACH_SAFE(pos, n, head) list_for_each_safe(pos, n, head)
    #define MI_VENC_INIT_LIST_HEAD INIT_LIST_HEAD
#endif


//==== for kthread porting ====
#if USE_CAM_OS
#define MI_VENC_CreateThreadWrapper(pfnStartRoutine) \
    void * _##pfnStartRoutine (void* pArg) \
    { \
        (void)pfnStartRoutine(pArg); \
        return NULL; \
    }
#define MI_VENC_CreateThread(ptThread, szName, pfnStartRoutine, pArg) \
    _MI_VENC_CreateThread(ptThread, szName, _##pfnStartRoutine, pArg)

MI_S32 _MI_VENC_CreateThread(MI_VENC_Thread_t *ptThread, char* szName, void *(*pfnStartRoutine)(void *), void* pArg)
{
    CamOsThreadAttrb_t stAttr = {0, 0};
    CamOsRet_e eRet;
    eRet = CamOsThreadCreate(ptThread, &stAttr, pfnStartRoutine, pArg);
    if (CAM_OS_OK != eRet)
    {
        return -1;
    }
    return MI_SUCCESS;
}
#else
#define MI_VENC_CreateThreadWrapper(pfnStartRoutine)
MI_S32 MI_VENC_CreateThread(MI_VENC_Thread_t *ptThread, char* szName, int (*pfnStartRoutine)(void *), void* pArg)
{
    *ptThread = kthread_create(pfnStartRoutine, pArg, szName);
    if (IS_ERR(*ptThread))
    {
        DBG_ERR("Fail to create thread VPE/WorkThread.\n");
        return -1;
    }
    return MI_SUCCESS;
}
#endif

#define LOCK_CHNN(pstChnRes)   DOWN(&pstChnRes->semLock)
#define UNLOCK_CHNN(pstChnRes) UP(&pstChnRes->semLock)

#define _IS_VALID_VENC_CHANNEL(chanId)\
        (((MI_VENC_CHN)chanId) < VENC_MAX_CHN_NUM)

//Note that this is not portable if enum order changes but implemented like this to save code
#define _IS_VALID_VENC_MODULE_TYPE(eType)\
        (((eType) > E_MI_VENC_MODTYPE_VENC) && ((eType) <= E_MI_VENC_MODTYPE_MAX))

#define DEFAULT_ES_SIZE (1024*1024) //(2000)
#define MAX_OUTPUT_ES_SIZE (1024*1024) //(2000)
#define LOW_LATENCY_PACK_SIZE (8 * 1024)
#define MAX_USER_DATA_LEN (1024)
#define MAX_ROI_AREA (8)

//==== Frame Lost Strategy and Super Frame
//special debug magic number
#define IS_MORE_DBG_INFO_SUPER_FRM(pstChnRes) (pstChnRes->stSuperFrame.u32SuperBFrmBitsThr == 3)
#define IS_MORE_DBG_WRN_SUPER_FRM(pstChnRes) (pstChnRes->stSuperFrame.u32SuperBFrmBitsThr >= 2\
        && pstChnRes->stSuperFrame.u32SuperBFrmBitsThr <= 3)
#define IS_MORE_DBG_INFO_FRM_LOST(pstChnRes)  (pstChnRes->stFrameLost.bFrmLostOpen == 3)
#define IS_MORE_DBG_WRN_FRM_LOST(pstChnRes)  (pstChnRes->stFrameLost.bFrmLostOpen >= 2\
        && pstChnRes->stFrameLost.bFrmLostOpen <= 3)

#define USE_FL_WINDOW (1) //Set 1 to use Frame Lost sliding window
#define MAX_FRAME_LOST_WINDOW_SIZE (32)

//These debug message is too detail and should only be comprehensive in module testing
#if 1 // MT: Module Test, MTD: Module Test with Device argument
    #define MTD_INFO(pDev, fmt, args...) ({do{if(_stDbgFsInfo[pDev->u32DevId].eDbgLevel>=MI_DBG_INFO){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_GREEN "[MT INFO]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})
    #define MTD_WRN(pDev, fmt, ...)  ({do{if(_stDbgFsInfo[pDev->u32DevId].eDbgLevel>=MI_DBG_WRN) {MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_YELLOW"[MT WRN ]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}}while(0);})
    #define MTD_ERR(pDev, fmt, args...)  ({do{if(_stDbgFsInfo[pDev->u32DevId].eDbgLevel>=MI_DBG_ERR) {MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED   "[MT ERR ]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})

    //Coding convention: User for these macro must have pstDevRes, this saves typing.
    #define MT_INFO(fmt, ...) ({do{if(pstDevRes&&_stDbgFsInfo[pstDevRes->u32DevId].eDbgLevel>=MI_DBG_INFO){MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_GREEN "[MT INFO]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}}while(0);})
    #define MT_WRN(fmt, ...)  ({do{if(pstDevRes&&_stDbgFsInfo[pstDevRes->u32DevId].eDbgLevel>=MI_DBG_WRN) {MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_YELLOW"[MT WRN ]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}}while(0);})
    #define MT_ERR(fmt, ...)  ({do{if(pstDevRes&&_stDbgFsInfo[pstDevRes->u32DevId].eDbgLevel>=MI_DBG_ERR) {MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_RED   "[MT ERR ]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}}while(0);})
#else
    #define MT_ERR(...)
    #define MT_WRN(...)
    #define MT_INFO(...)
#endif

/**
 * TMP debugging for internal engineers, this is allowed to be pushed into trunk temporarily but indented to be
 * removed soon after a certain tasks. An example usage: add debugging message for setting parameter while MHAL
 * engineers revising MHAL driver in different commit.
 */
#define DBG_TMP(fmt, ...) {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_BLUE "[DBG TMP]:`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}while(0);}

//special print which does not control by debug level and is permanent, not like DBG_TMP
#define DBG_MGC(fmt, ...) {do{MI_SYS_LOG_IMPL_PrintLog(ASCII_COLOR_GREEN "`%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##__VA_ARGS__);}while(0);}

/**
 * TMP version control.
 * Fixed to used version 1
 * This is a temporary solution and should be removed later after per structure version control is implemented.
 */
#define MHAL_VENC_SET_TMP_VER(st) (st).stVerCtl.u32Version = 1

#define DOWN_CPU_MODE(cmdinf,sem) if(!cmdinf) { DOWN(&sem); }
#define UP_CPU_MODE(cmdinf,sem) if(!cmdinf) { UP(&sem); }


///Used to note that a module does not support HW command queue.
#define E_MHAL_CMDQ_ID_NONE E_MHAL_CMDQ_ID_MAX
#ifdef CONFIG_ARCH_INFINITY2
    #define E_MHAL_CMDQ_ID0_H265 (E_MHAL_CMDQ_ID_H265_VENC0)
    #define E_MHAL_CMDQ_ID1_H265 (E_MHAL_CMDQ_ID_H265_VENC1)
#else
    #define E_MHAL_CMDQ_ID0_H265 (E_MHAL_CMDQ_ID_NONE)
    #define E_MHAL_CMDQ_ID1_H265 (E_MHAL_CMDQ_ID_NONE)
#endif

#define MHAL_BYTES_PER_CMDQ (8)

static MI_S32 _MI_VENC_DestroyDevice(MI_VENC_Dev_e eDevType);
static MI_S32 _MI_VENC_ConfigInstance(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_ChnAttr_t *pstAttr);
//-------------------------------------------------------------------------------------------------
//  Compilation Options Checking
//-------------------------------------------------------------------------------------------------
#if (SKIP_IRQ == 0) && (USE_HW_ENC == 0)
    //#error SKIP_IRQ must be 1 while USE_HW_ENC is 0
    #undef SKIP_IRQ
    #define SKIP_IRQ (1)
#endif

#if (VENC_USE_CMDQ && !USE_HW_ENC)
    //#error CMDQ is supported only in i3 now.
    #undef VENC_USE_CMDQ
    #define VENC_USE_CMDQ (0)
#endif

/** @bug If pushing too fast in user application, the system hangs. It's likely in MI_SYS.
 *
 */
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//---- The device and channel resource
typedef struct MI_VENC_MemBufInfo_s
{
    MI_PHY phyAddr;
    void *pVirAddr;
    MI_U32 u32BufSize;
} MI_VENC_MemBufInfo_t;

/*                                                                 (SEI, RC, Channel)
 * input task:IDLE->SELECTED->wait device available->PRE_ENCODE->apply frame sync attribute->ENCODE->MHAL_EncOneFrame()->WAIT_ISR
 *  ISR->MHAL_IsrProc()->notify output task
 *  output task:WAIT_ISR--------------->POST_ENCODE->MHAL_EncDone()->ENCODE_DONE->apply frame-sync attribute->IDLE
 *                      \_WAIT_CMDQ_>_/                            \                 (request IDR)
 *                      ^                                           |
 *                      \_REENCODE______________________________<__/              */
typedef enum
{
    MI_VENC_BUFSTATE_IDLE = 0, ///< This value must == 0 in case of memset(0) initialization.
    MI_VENC_BUFSTATE_ENCODE_DONE,
    MI_VENC_BUFSTATE_POST_ENCODE,
    MI_VENC_BUFSTATE_REENCODE = 10,//start of exceptional branch state
    //MI_VENC_BUFSTATE_ISR_DONE,//no channel list traverse in ISR
    MI_VENC_BUFSTATE_WAIT_CMDQ,
    //MI_VENC_BUFSTATE_IN_ISR,//no channel list traverse in ISR
    MI_VENC_BUFSTATE_CMDQ_ERROR,
    MI_VENC_BUFSTATE_WAIT_ISR = 20,
    MI_VENC_BUFSTATE_ENCODE,
    MI_VENC_BUFSTATE_PRE_ENCODE,
    MI_VENC_BUFSTATE_SELECTED,
    MI_VENC_BUFSTATE_MAX
} MI_VENC_BufferState;

typedef struct MI_VENC_ChnRes_s
{
    MI_VENC_MemBufInfo_t stRefMemInfo; ///<Internal reference frames memory
    MI_VENC_MemBufInfo_t stAlMemInfo; ///<Al stands for Algorithm
    MI_BOOL bCreate;
    MI_BOOL bStart;
    MI_U8   u8ChnId;
    //volatile MI_BOOL bSelected; ///< If any buffer is selected for processing
    volatile MI_VENC_BufferState eBufState;
    MI_BOOL bRequestIdr;
    MI_BOOL bEnableIdr;
    MI_BOOL bRequestEnableIdr;///< A user APP requests enable IDR setting. Apply the effect while the frame is done.
    MI_BOOL bByFrame;
    //MI_VENC_ChnStat_t stStat;// use mi_sys_GetChnBufInfo instead.
    MI_U8   au8UserData[MAX_USER_DATA_LEN];
    MI_U32  u32UserDataLen;
    MI_SYS_PixelFormat_e eInputFormat;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_BOOL bPendingChnAttr;
    MI_U32  u32BufSize;//buffer size is different among CODEC types. Make it simple here.
    MI_VENC_DevRes_t *pstDevRes; ///< Quick access to resources
    MHAL_VENC_INST_HANDLE hInst;
    MHAL_VENC_InternalBuf_t stVencInternalBuf;
    MHAL_VENC_RoiCfg_t astRoiCfg[MAX_ROI_AREA]; /**< There are MAX_ROI_AREA sets of configurations.
     Use MHAL to speed up setting for stream on.*/
    MI_VENC_SuperFrameCfg_t stSuperFrame;
    MI_VENC_ParamFrameLost_t stFrameLost;
    struct
    {
        MI_U32 u32IFrmThr;//bytes
        MI_U32 u32PFrmThr;//bytes
        MI_U32 u32GopFrmCnt;
        MI_U32 u32GopPBytes;
        MI_U32 u32GopIBytes;
        MI_U32 u32LastGopBytes;//total bytes
        MI_U8  u8AbortCnt;
#if USE_FL_WINDOW
        MI_U32 u32FrmBytes[MAX_FRAME_LOST_WINDOW_SIZE];
        MI_U32 u32WindowBytes; //total bytes for the window
        MI_U16 u16Idx;
#endif
        MI_S16 s16DropGap;
    } stFLCtx;//Frame Lost Context
    struct
    {
        MI_U32 u32Gop;
        MI_U32 u32TargetBitrate;
        MI_U32 u32SrcFrmRateNum;
        MI_U32 u32SrcFrmRateDen;
    } stCtx;//interpreted context
    MI_VENC_RcPriority_e eRcPriority;
    MI_VENC_Sem_t semLock;

    //Statistic
    MI_U64  u64SeqNum;
    MI_U32  u32InFrameCnt;
    MI_U32  u32InDropCnt;
    MI_U32  u32InBlockCnt;
    MI_U32  u32OutFrameCnt;
    MI_U32  u32OutDropCnt;
    MI_U32  u32ReEncCnt;
    MI_U32  u32MbRate;//macro-block per second
    struct
    {
        MI_U32  u32Kbps;
        MI_U16  u16Fps;
        MI_U16  u16FpsFrac;
    } _stStat;//internal statistic
    MI_VENC_Utilization_t stFps;
    MI_U32  u32UpdateAttrCnt; ///< Updating Channel Attribute counter;
} MI_VENC_ChnRes_t;

typedef struct MI_VENC_DevRes_s
{
    //MI
    MI_BOOL bInitFlag;
    MI_BOOL bWorkTaskRun;
    MI_BOOL bIrqTaskRun;
    MI_SYS_DRV_HANDLE hMiDev;
    MI_VENC_Dev_e eMiDevType;
    MI_VENC_ChnRes_t *astChnRes[MI_VENC_MAX_CHN_NUM_PER_MODULE];
    MI_U32  u32MbRate;//Macro-block per second
    MI_U32  u32DevId; ///< self ID number for quick reference

    //OS-dependent
    MI_VENC_Thread_t stTaskWork;
    MI_VENC_Thread_t stTaskIrq;

    MI_VENC_Wait_t stWorkWaitQueueHead;
    MI_VENC_Wait_t stIrqWaitQueueHead;
    MI_VENC_List_t todo_task_list;
    MI_VENC_List_t working_task_list;
    MI_VENC_Sem_t list_mutex;
    MI_VENC_Sem_t frame_sem;
    raw_spinlock_t working_task_spinlock;//FIXME this is for Linux only

    //HAL and command queue
    MHAL_VENC_DEV_HANDLE hHalDev;
    MHAL_CMDQ_CmdqInterface_t *cmdq;
    MHAL_CMDQ_Id_e eCmdqId;
    MS_U32 u32CmdqSize; ///< Command queue size in bytes for this module
    MS_U32 u32CmdqCnt; ///<  Command queue number. (Total entities)
    MS_U32 u32IrqNum;
    volatile MI_U32 u32WaitCmdq;/**<Use this to control if the command queue
     needs to be rechecked from the working list.*/
    volatile MS_BOOL bFromIsr;
    MHAL_VENC_Drv_t *pstDrv;
    MHAL_VENC_ResolutionAlign_t stAlign;

    //Statistic
    MI_VENC_Utilization_t stUtil;
    MI_VENC_Utilization_t stFps;
    struct
    {
        MI_U16 u16Fps;
        MI_U16 u16FpsFrac;
        MI_U16 au16Util[MI_VENC_UTIL_MAX_END];
        MI_U16 au16PeakUtil[MI_VENC_UTIL_MAX_END];
    } _stStat;//internal statistic
} MI_VENC_DevRes_t;

typedef struct MI_VENC_Res_s
{
    MI_BOOL bInitFlag;
    MI_VENC_ChnRes_t astChnRes[VENC_MAX_CHN_NUM];
    MI_VENC_DevRes_t devs[E_MI_VENC_DEV_MAX];
} MI_VENC_Res_t;
static MI_VENC_Res_t _ModRes; //Module Resource


//---- Module Configuration: Mostly chip platform dependent, such as CMDQ ID for each module.
typedef struct MHAL_VENC_DevConfig_s
{
    struct list_head *pTodoList;
    struct list_head *pWorkingList;
    MHAL_CMDQ_Id_e eCmdqId;
    MI_VENC_PFN_IRQ IRQ; //rename to ISR
    MS_U8 u8CoreId;
    MHAL_VENC_Drv_t *pstDrv;
    char *szName;
    MI_U32 u32MbRate;
} MHAL_VENC_DevConfig_t;

#define DECLARE_DEV_LINUX_RES(dev_name)\
    LIST_HEAD(todo_task_list_##dev_name);\
    LIST_HEAD(working_task_list_##dev_name)

#define REF_INIT_RES(eDev, dev_name, cmdq_id, irq, core, pDrv, mbRate)\
    [eDev].pTodoList = &todo_task_list_##dev_name,\
    [eDev].pWorkingList = &working_task_list_##dev_name,\
    [eDev].szName = #dev_name,\
    [eDev].eCmdqId = cmdq_id,\
    [eDev].IRQ = irq,\
    [eDev].u8CoreId = core,\
    [eDev].pstDrv = pDrv,\
    [eDev].u32MbRate = mbRate

DECLARE_DEV_LINUX_RES(mhe0);
DECLARE_DEV_LINUX_RES(mhe1);
DECLARE_DEV_LINUX_RES(mfe0);
DECLARE_DEV_LINUX_RES(mfe1);
DECLARE_DEV_LINUX_RES(jpeg);
DECLARE_DEV_LINUX_RES(dummy);
MI_VENC_Irqreturn_t VENC_IP_ISR(int irq, void* data);

#define DRV_CORE0 (0)
#define DRV_CORE1 (1)


#define MB_RATE_MHE 539062 //(1920*1088/256*60)=489600, 539,062.5 for 138 MP/s
#define MB_RATE_MFE 664062 //(1920*1088/256*60), 664,062.5 for 170MP/s
#define MB_RATE_JPE (1920*1088/256*60)
#define MHAL_VENC_INST_PER_DEV (8) //assumed instances per device, currently used for CMDQ only

const MHAL_VENC_DevConfig_t astMHalDevConfig[E_MI_VENC_DEV_MAX] =
{
#if 1 //all CMDQ enabled
    REF_INIT_RES(E_MI_VENC_DEV_MHE0,  mhe0,  E_MHAL_CMDQ_ID0_H265,      VENC_IP_ISR, DRV_CORE0, &stDrvMhe, MB_RATE_MHE),
    REF_INIT_RES(E_MI_VENC_DEV_MHE1,  mhe1,  E_MHAL_CMDQ_ID1_H265,      VENC_IP_ISR, DRV_CORE1, &stDrvMhe, MB_RATE_MHE),
    REF_INIT_RES(E_MI_VENC_DEV_MFE0,  mfe0,  E_MHAL_CMDQ_ID_H264_VENC0, VENC_IP_ISR, DRV_CORE0, &stDrvMfe, MB_RATE_MFE),
#else
    REF_INIT_RES(E_MI_VENC_DEV_MHE0,  mhe0,  E_MHAL_CMDQ_ID_NONE,       VENC_IP_ISR, DRV_CORE0, &stDrvMhe, MB_RATE_MHE),
    REF_INIT_RES(E_MI_VENC_DEV_MHE1,  mhe1,  E_MHAL_CMDQ_ID_NONE,       VENC_IP_ISR, DRV_CORE1, &stDrvMhe, MB_RATE_MHE),
    REF_INIT_RES(E_MI_VENC_DEV_MFE0,  mfe0,  E_MHAL_CMDQ_ID_NONE,       VENC_IP_ISR, DRV_CORE0, &stDrvMfe, MB_RATE_MFE),
#endif
    REF_INIT_RES(E_MI_VENC_DEV_MFE1,  mfe1,  E_MHAL_CMDQ_ID_NONE,       VENC_IP_ISR, DRV_CORE1, &stDrvMfe, MB_RATE_MFE),
    REF_INIT_RES(E_MI_VENC_DEV_JPEG,  jpeg,  E_MHAL_CMDQ_ID_NONE,       VENC_IP_ISR, DRV_CORE0, &stDrvJpe, MB_RATE_JPE),
#if CONNECT_DUMMY_HAL
    REF_INIT_RES(E_MI_VENC_DEV_DUMMY, dummy, E_MHAL_CMDQ_ID_NONE,       NULL,        DRV_CORE0, &stDrvDummy, 10000),
#endif
};

/** @brief configuration by each MHAL module
 *
 * These could be different among MHAL modules.
 * Because current they could not be get with MHAL_VENC_GetParam, they are set here.
 */
typedef struct MHAL_VENC_ModuleConfig_s
{
    MS_U32 u32RoiBlock;///< ROI block size granularity in pixels
    MHAL_VENC_IDX eCrop;
} MHAL_VENC_ModuleConfig_t;

static MHAL_VENC_ModuleConfig_t _astMhalModuleCfg[E_MI_VENC_MODTYPE_MAX] =
{
    [E_MI_VENC_MODTYPE_H264E] =
    {
        .u32RoiBlock = 16,
        .eCrop = E_MHAL_VENC_264_CROP,
    },
    [E_MI_VENC_MODTYPE_H265E] =
    {
        .u32RoiBlock = 16,
        .eCrop = E_MHAL_VENC_265_CROP,
    },
    [E_MI_VENC_MODTYPE_JPEGE] =
    {
        .u32RoiBlock = 16,
        .eCrop = E_MHAL_VENC_JPEG_CROP,
    },
};

//info for all device
typedef struct MI_VENC_DbgFsGlobalInfo_s
{
    MI_S32 s32ShowAllDev;
} MI_VENC_DbgFsGlobalInfo_t;

//info for each device
typedef struct MI_VENC_DbgFsInfo_s
{
    MI_DBG_LEVEL_e eDbgLevel;
    MI_S32 s32EnIrq;
    MI_S32 s32EnCmdq;
    MI_S32 s32DumpIn;//dump input buffer
    MI_S32 s32DumpBigIn;//dump input buffer while output buffer is big
    MI_S32 s32Cmdq;
    MI_S32 s32Isr;
} MI_VENC_DbgFsInfo_t;

static MI_VENC_DbgFsGlobalInfo_t _stDbgFsGlobal =
{
    .s32ShowAllDev = TRUE,
};

static MI_VENC_DbgFsInfo_t _stDbgFsInfo[E_MI_VENC_DEV_MAX] =
{
    [E_MI_VENC_DEV_MHE0] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE,
        .s32EnCmdq = TRUE
    },
    [E_MI_VENC_DEV_MHE1] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE,
        .s32EnCmdq = TRUE
    },
    [E_MI_VENC_DEV_MFE0] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE,
        .s32EnCmdq = TRUE
    },
    [E_MI_VENC_DEV_MFE1] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE,
        .s32EnCmdq = FALSE
    },
    [E_MI_VENC_DEV_JPEG] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE,
        .s32EnCmdq = FALSE
    }
#if CONNECT_DUMMY_HAL
    , [E_MI_VENC_DEV_DUMMY] =
    {
        .eDbgLevel = MI_DBG_NONE,
        .s32DumpIn = FALSE,
        .s32DumpBigIn = FALSE
    }
#endif
};

typedef struct MI_VENC_DbgFsCmd_s
{
    char  *szName; ///< c string of the name of this command
    MI_U8 u8MaxArgc; ///< max argument for this command
    ///Command function Sync this function type with mi_sys_RegistCommand
    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData);
    /// PROC FS call back does not link to MI_VENC_DbgFsCmd_t. Point to self here while initializing.
    struct MI_VENC_DbgFsCmd_s **ppstSelf;
} MI_VENC_DbgFsCmd_t;

MI_VENC_DbgFsCmd_t *pstProcDbgLevel = NULL;
MI_VENC_DbgFsCmd_t *pstProcEnIrq = NULL;
MI_VENC_DbgFsCmd_t *pstProcEnCmdq = NULL;
MI_VENC_DbgFsCmd_t *pstProcDumpIn = NULL;
MI_VENC_DbgFsCmd_t *pstProcDumpInBig = NULL;
MI_VENC_DbgFsCmd_t *pstProcShowAll = NULL;
MI_VENC_DbgFsCmd_t *pstProcDmsg = NULL;

#define __MI_LOW_LATENCY_MAGIC_NUM__ 0xA0000000
#define __MI_LOW_LATENCY_VALUE_MASK__  0x0FFFFFFF

#define VENC_FRAME_START 0x01
#define VENC_FRAME_END 0x02

#define FRAME_TYPE_MASK 0xF0
#define FRAME_TYPE_SHIFT 4

typedef enum
{
    _MI_VENC_LOW_LATENCY_BASE,
    _MI_VENC_LOW_LATENCY_DONE,
    _MI_VENC_LOW_LATENCY_CANCEL,
    _MI_VENC_LOW_LATENCY_ONGOING,
} _MI_Venc_Low_Latency_Status_e;

static inline void _mi_set_chntask_lowlatency_read_offset(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U64 val)
{
    pstChnTask->u64Reserved1 = val | __MI_LOW_LATENCY_MAGIC_NUM__;
}

static inline void _mi_set_chntask_lowlatency_read_offset_move(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U64 val)
{
    pstChnTask->u64Reserved1 += val;
}

static inline void _mi_set_chntask_lowlatency_write_offset(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U64 val)
{
    pstChnTask->u64Reserved2 = val | __MI_LOW_LATENCY_MAGIC_NUM__;
}

static inline void _mi_set_chntask_lowlatency_write_offset_move(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U64 val)
{
    pstChnTask->u64Reserved2 += val;
}

static inline void _mi_set_chntask_lowlatency_pack_size(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U64 val)
{
    pstChnTask->u64Reserved3 = val | __MI_LOW_LATENCY_MAGIC_NUM__;
}

static inline void _mi_set_chntask_lowlatency_status(mi_sys_ChnTaskInfo_t *pstChnTask, _MI_Venc_Low_Latency_Status_e val)
{
    pstChnTask->u32Reserved1 = val | __MI_LOW_LATENCY_MAGIC_NUM__;
}

static inline MI_U64 _mi_get_chntask_lowlatency_read_offset(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_SYS_BUG_ON((pstChnTask->u64Reserved1 | __MI_LOW_LATENCY_VALUE_MASK__)
                  == __MI_LOW_LATENCY_VALUE_MASK__);
    return pstChnTask->u64Reserved1 & __MI_LOW_LATENCY_VALUE_MASK__;
}

static inline MI_U64 _mi_get_chntask_lowlatency_write_offset(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_SYS_BUG_ON((pstChnTask->u64Reserved2 | __MI_LOW_LATENCY_VALUE_MASK__)
                  == __MI_LOW_LATENCY_VALUE_MASK__);
    return pstChnTask->u64Reserved2 & __MI_LOW_LATENCY_VALUE_MASK__;
}

static inline MI_U64 _mi_get_chntask_lowlatency_pack_size(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    //MI_SYS_BUG_ON((pstChnTask->u64Reserved3 | __MI_LOW_LATENCY_VALUE_MASK__)
    //    == __MI_LOW_LATENCY_VALUE_MASK__);
    return pstChnTask->u64Reserved3 & __MI_LOW_LATENCY_VALUE_MASK__;
}

static inline MI_U32 _mi_get_chntask_lowlatency_status(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    //MI_SYS_BUG_ON((pstChnTask->u32Reserved1 | __MI_LOW_LATENCY_VALUE_MASK__)
    //    == __MI_LOW_LATENCY_VALUE_MASK__);
    return pstChnTask->u32Reserved1 & __MI_LOW_LATENCY_VALUE_MASK__;
}

#define FenceOfChTask(pstChTask) (pstChTask->u32Reserved0)
#define _VENC_GET_FRAME_SIZE(frame, sz) (sz)

static inline MI_BOOL _MI_VENC_IsFenceL(MI_U16 fence1, MI_U16 fence2)
{
    if(((MI_U16)(fence2 - fence1 - 1)) < 100)
    {
        return TRUE;
    }
    return FALSE;
}

static MI_U16 _MI_VENC_ReadFence(MHAL_CMDQ_CmdqInterface_t *cmdinf)
{
    MI_U16 u16Value = 0;
    if(cmdinf && cmdinf->MHAL_CMDQ_ReadDummyRegCmdq)
        cmdinf->MHAL_CMDQ_ReadDummyRegCmdq(cmdinf, &u16Value);
    return u16Value;
}

static void _MI_VENC_ReportDevFps (MI_VENC_Utilization_t* pstUtil, void* pUser)
{
    MI_VENC_DevRes_t *pstDevRes = (MI_VENC_DevRes_t *) pUser;
    MI_U32 u32Divisor;
    if(pstDevRes == NULL)
        return;
    u32Divisor = pstUtil->d.u32DiffUs;
    if(u32Divisor != 0)
    {
        pstDevRes->_stStat.u16Fps = (MI_U16) ((MI_U64) (pstUtil->d.u32Cnt - 1)
                                              * 1000000 / u32Divisor);
        pstDevRes->_stStat.u16FpsFrac = (MI_U16) ((MI_U64) ((pstUtil->d.u32Cnt - 1)
                                        * 1000000 * 100 / u32Divisor) % 100);
    }
    if(0)
    {
        printk("<%5ld.%06ld> D%02d get %2d.%02d fps\n",
               pstUtil->d.stTimeEnd[0].tv_sec, pstUtil->d.stTimeEnd[0].tv_usec, pstDevRes->u32DevId,
               pstDevRes->_stStat.u16Fps, pstDevRes->_stStat.u16FpsFrac);
    }
}

static void _MI_VENC_ReportChnFps (MI_VENC_Utilization_t* pstUtil, void* pUser)
{
    MI_U32 VeChn = (MI_U32) pUser;
    MI_VENC_ChnRes_t *pstChnRes;
    MI_U32 u32Divisor;

    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        //DBG_ERR("Invalid channel :%d\n", VeChn);
        return;
    }
    pstChnRes = _ModRes.astChnRes + VeChn;
    u32Divisor = pstUtil->d.u32DiffUs;
    if(u32Divisor != 0)
    {
        pstChnRes->_stStat.u16Fps = (MI_U16)((MI_U64) (pstUtil->d.u32Cnt-1) * 1000000 / u32Divisor);
        pstChnRes->_stStat.u16FpsFrac = (MI_U16)((MI_U64) ((pstUtil->d.u32Cnt-1) * 1000000 * 100 / u32Divisor) % 100);
        pstChnRes->_stStat.u32Kbps = pstUtil->d.u32ValueSum * 1000 / u32Divisor;
        if (0)
        {
            printk("<%5ld.%06ld> CH %02d get %2d.%02d fps, %d kbps\n",
                   pstUtil->d.stTimeEnd[0].tv_sec, pstUtil->d.stTimeEnd[0].tv_usec, VeChn,
                   (MI_U32)((MI_U64) (pstUtil->d.u32Cnt-1) * 1000000 / u32Divisor),
                   (MI_U32)((MI_U64) ((pstUtil->d.u32Cnt-1) * 1000000 * 100 / u32Divisor) % 100),
                   pstUtil->d.u32ValueSum * 1000 / u32Divisor);
        }
    }
}

static void _MI_VENC_ReportDevUtil (MI_VENC_Utilization_t* pstUtil, void* pUser)
{
    MI_S32 i;
    MI_VENC_DevRes_t *pstDevRes = (MI_VENC_DevRes_t *) pUser;
    MI_U32 u32Divisor;
    MI_U64 u64Divisor;
    if(pstDevRes == NULL)
        return;

    u32Divisor = pstUtil->d.u32DiffUs;
    if(u32Divisor != 0)
    {
        pstDevRes->_stStat.u16Fps = (MI_U16) ((MI_U64) (pstUtil->d.u32Cnt - 1)
                                              * 1000000 / u32Divisor);
        pstDevRes->_stStat.u16FpsFrac = (MI_U16) ((MI_U64) ((pstUtil->d.u32Cnt - 1)
                                        * 1000000 * 100 / u32Divisor) % 100);
    }
    u64Divisor = pstUtil->d.u64TimeSum;
    for (i = 0; i < MI_VENC_UTIL_MAX_END; ++i)
    {
        if(u64Divisor != 0)
        {
            pstDevRes->_stStat.au16Util[i] = (MI_U16) (pstUtil->d.au64SumDiff[i] * 100 / u64Divisor);
        }
        if(pstDevRes->_stStat.au16Util[i] > pstDevRes->_stStat.au16PeakUtil[i])
        {
            pstDevRes->_stStat.au16PeakUtil[i] = pstDevRes->_stStat.au16Util[i];
        }
    }
    if(0)
    {
        printk("util:%d %d\n",
               pstDevRes->_stStat.au16Util[0],
               pstDevRes->_stStat.au16Util[1]);
    }
}

MI_S32 _MI_VENC_SetRoiCfg(MI_VENC_ChnRes_t *pstChnRes, MI_U32 u32Index)
{
    MS_S32 s32Ret = MI_SUCCESS;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_ROI;
    MI_SYS_BUG_ON(pstChnRes == NULL || u32Index >= MAX_ROI_AREA);

    if(pstChnRes->astRoiCfg[u32Index].stVerCtl.u32Size == sizeof(MHAL_VENC_RoiCfg_t))
    {
        if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            eMhalIndex = E_MHAL_VENC_264_ROI;
        }
        else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            eMhalIndex = E_MHAL_VENC_265_ROI;
        }
        s32Ret = MHAL_VENC_SetParam(pstChnRes, eMhalIndex, &pstChnRes->astRoiCfg[u32Index]);
        if(s32Ret != MI_SUCCESS)
        {
            DBG_ERR("Set Param err 0x%X.\n", s32Ret);
            return MI_ERR_VENC_UNDEFINED;
        }
    }
    return s32Ret;
}

MS_S32 _MI_VENC_ApplyEnableIdr(MI_VENC_ChnRes_t *pstChnRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MS_S32 s32RetIdr;
    MHAL_VENC_EnableIdr_t stEnableIdr;

    MHAL_VENC_INIT_PARAM(MHAL_VENC_EnableIdr_t, stEnableIdr);
    stEnableIdr.bEnable = pstChnRes->bEnableIdr;

    MI_SYS_BUG_ON(pstChnRes->eBufState == MI_VENC_BUFSTATE_IDLE);

    if(pstChnRes->bStart)
    {
        MHAL_VENC_ParamInt_t stDummy;
        MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamInt_t, stDummy);
        s32Ret = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_OFF, (MHAL_VENC_Param_t*) &stDummy);
        if (s32Ret != MI_SUCCESS/*MI_SUCCESS*/)
        {
            DBG_ERR("E_MHAL_VENC_IDX_STREAM_OFF\n");
        }
    }

    s32RetIdr = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_ENABLE_IDR, &stEnableIdr);
    if(s32RetIdr != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32RetIdr);
        //return MI_ERR_VENC_UNDEFINED;
    }

    if(pstChnRes->bStart)
    {
        MI_U32 i;
        s32Ret = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_ON, (MHAL_VENC_Param_t*) &pstChnRes->stVencInternalBuf);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_ERR("E_MHAL_VENC_IDX_STREAM_ON\n");
            return s32Ret;
        }
        for (i = 0; i < MAX_ROI_AREA; ++i)
        {
            s32Ret = _MI_VENC_SetRoiCfg(pstChnRes, i);
            if(s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
        }
    }
    if(s32RetIdr != MI_SUCCESS)
        s32Ret = s32RetIdr;

    return s32Ret;
}

MS_S32 _MI_VENC_InsertUserData(MI_VENC_ChnRes_t *pstChnRes)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MHAL_VENC_UserData_t stUserData;
    MHAL_VENC_IDX eType = E_MHAL_VENC_USER_DATA;


    if(pstChnRes->stChnAttr.stVeAttr.eType != E_MI_VENC_MODTYPE_H264E &&
            pstChnRes->stChnAttr.stVeAttr.eType != E_MI_VENC_MODTYPE_H265E)
    {
        //MI_ERR_VENC_NOT_SUPPORT; is not needed, MODTYPE check should be done in MI_VENC_IMPL_InsertUserData
        return MI_SUCCESS;
    }

    if(pstChnRes->u32UserDataLen == 0)
    {
        return MI_SUCCESS;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_UserData_t, stUserData);

    stUserData.pu8Data = pstChnRes->au8UserData;
    stUserData.u32Len = pstChnRes->u32UserDataLen;
    pstChnRes->u32UserDataLen = 0;

    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eType = E_MHAL_VENC_264_USER_DATA;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eType = E_MHAL_VENC_265_USER_DATA;
    }
    s32Ret = MHAL_VENC_SetParam(pstChnRes, eType, &stUserData);
    if(s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Ret);
        return MI_ERR_VENC_UNDEFINED;
    }
    return s32Ret;
}

static MI_S32 _MI_VENC_MapMemToFile(char *szFileName, MI_PHY phyAddr, MI_U32 u32Size)
{
    struct file *fp;
    fp = OpenFile(szFileName, O_RDWR | O_CREAT | O_SYNC, 0644);
    if(IS_ERR(fp))
    {
        DBG_ERR("file open %p!", fp);
        return -1;
    }
    else if(fp)
    {
        char *ptr;
        ptr = mi_sys_Vmap(phyAddr, u32Size, FALSE);
        if(ptr)
        {
            DBG_WRN("dump:%p sz:%d\n", ptr, u32Size);
            //mi_sys_VFlushInvCache(ptr, u32Size);//even if Vmap is non-cachable, it still need flush...
            (void) WriteFile(fp, ptr, u32Size);
            mi_sys_UnVmap(ptr);
        }
        (void) CloseFile(fp);
    }
    return MI_SUCCESS;
}

MI_BOOL Is_VENC_TASK_LOWLATENCY(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    if(pstChnTask)
    {
        if(_mi_get_chntask_lowlatency_pack_size(pstChnTask) == LOW_LATENCY_PACK_SIZE)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void PreProcessLowLatencyOutputput(mi_sys_ChnTaskInfo_t *pstChnTask, MI_U32 contentSize, MI_U64 u64SidebandMsg)
{
    MI_SYS_BufConf_t  stBufConfig;
    MI_SYS_BufInfo_t *pstTmpBuf;
    MI_U64 read_offset, write_offset;
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)
    static MI_BOOL bFirstSplit = FALSE;
#endif
    stBufConfig.u64TargetPts = pstChnTask->astInputPortBufInfo[0]->u64Pts;
    stBufConfig.eBufType  = E_MI_SYS_BUFDATA_RAW;

    /*
    No need to set MI_SYS_MAP_CPU_READ & MI_SYS_MAP_CPU_WRITE flag.
    Since Output buf is vacant. and output is picked by cpu(User).
    */
    stBufConfig.u32Flags = MI_SYS_MAP_VA;
    stBufConfig.stRawCfg.u32Size = contentSize;

    read_offset = _mi_get_chntask_lowlatency_read_offset(pstChnTask);
    write_offset = _mi_get_chntask_lowlatency_write_offset(pstChnTask);
    /*
        if(read_offset > write_offset)
        {
            printk("read:%llx, write:%llx\n", read_offset, write_offset);
            return;
        }

        if(u64SidebandMsg != VENC_FRAME_END && read_offset == write_offset)
        {
            printk("read too fast: %llx\n", read_offset);
            return;
        }
    */
    pstTmpBuf = mi_sys_GetOutputPortBuf(pstChnTask->miSysDrvHandle, pstChnTask->u32ChnId,
                                        0, &stBufConfig, NULL);

#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)

    if((pstTmpBuf != NULL) && (pstChnTask->astInputPortBufInfo[0] != NULL) && !bFirstSplit)
    {
        mi_sys_buf_mgr_sync_trackinfo(pstChnTask->astInputPortBufInfo[0],pstTmpBuf);
    }
#endif
    if(pstTmpBuf)
    {
        pstTmpBuf->stRawData.u32ContentSize = contentSize;
        MI_SYS_ACK_SIDEBAND_MSG(u64SidebandMsg);
        pstTmpBuf->u64SidebandMsg = u64SidebandMsg;

        /*fix me.
        Can remove FlushCache here, since the outputbuf is not finished.
        it is encoding during picking the buf.
        So, can't make sure the cache is clean & invalid on low latency.
        */
        mi_sys_VFlushInvCache(pstChnTask->astOutputPortBufInfo[0]->stRawData.pVirAddr + read_offset, contentSize);

        memcpy(pstTmpBuf->stRawData.pVirAddr, pstChnTask->astOutputPortBufInfo[0]->stRawData.pVirAddr + read_offset, contentSize);

        mi_sys_FinishBuf(pstTmpBuf);
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1)

        if((unsigned int)u64SidebandMsg == 2)
            bFirstSplit = FALSE; //new frame will come reset flag
        else
            bFirstSplit = TRUE;

#endif
        _mi_set_chntask_lowlatency_read_offset_move(pstChnTask, contentSize);
    }
    else
    {
        DBG_INFO("pstTmpBuf is NULL\n");
    }
}

static __inline MI_S32 _MI_VENC_DoFrameLost(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_EncResult_t *pstEncResult, MI_BOOL bJpeg)
{
    MI_BOOL bDropFrame = FALSE;

    if(IS_MORE_DBG_INFO_FRM_LOST(pstChnRes))
    {
        if((pstEncResult->eStatus & 0x01/*MHVE_FLAGS_FORCEI*/))
        {
            DBG_MGC("CH%2d I:%d\n", pstChnRes->u8ChnId, pstEncResult->u32OutputBufUsed);
        }
        else
        {
            DBG_MGC("CH%2d P:%d\n", pstChnRes->u8ChnId, pstEncResult->u32OutputBufUsed);
        }
    }

    if(pstChnRes->stFrameLost.bFrmLostOpen && (!bJpeg))
    {
        if(pstChnRes->stFLCtx.s16DropGap <= -1)
        {//enable to drop
            MI_U32 u32FrmThr;
            if((pstEncResult->eStatus & 0x01/*MHVE_FLAGS_FORCEI*/))
            {
                u32FrmThr = pstChnRes->stFLCtx.u32IFrmThr;
            }
            else
            {
                MI_U32 u32CurFrameBytes;
#if USE_FL_WINDOW
                pstChnRes->stFLCtx.u32WindowBytes -= pstChnRes->stFLCtx.u32FrmBytes[pstChnRes->stFLCtx.u16Idx];
                pstChnRes->stFLCtx.u32WindowBytes += pstEncResult->u32OutputBufUsed;
                pstChnRes->stFLCtx.u32FrmBytes[pstChnRes->stFLCtx.u16Idx] = pstEncResult->u32OutputBufUsed;
                pstChnRes->stFLCtx.u16Idx++;
                if(frm_lost_win == 0)
                {
                    DBG_ERR("frm_lost_win must > 0\n");
                    frm_lost_win = 1;
                }
                if(pstChnRes->stFLCtx.u16Idx >= frm_lost_win)
                    pstChnRes->stFLCtx.u16Idx = 0;
                u32CurFrameBytes = pstChnRes->stFLCtx.u32WindowBytes / frm_lost_win;
                //DBG_TMP("%d WindowBytes to %d\n", pstChnRes->stFLCtx.u32WindowBytes, u32CurFrameBytes);
#else
                u32CurFrameBytes = pstEncResult->u32OutputBufUsed;
#endif
                u32FrmThr = pstChnRes->stFLCtx.u32PFrmThr;
                //only P frame would drop
                if(u32CurFrameBytes > u32FrmThr)
                {
                    pstChnRes->stFLCtx.s16DropGap = pstChnRes->stFrameLost.u32EncFrmGaps - 1;
                    bDropFrame = TRUE;
                    if(IS_MORE_DBG_WRN_FRM_LOST(pstChnRes))
                        DBG_WRN("CH%2d Frame Lost\n", pstChnRes->u8ChnId);
                }
            }
        }
        else
        {
            pstChnRes->stFLCtx.s16DropGap--;
            if(IS_MORE_DBG_INFO_FRM_LOST(pstChnRes))
                DBG_MGC("--\n");
        }

        if(!bDropFrame)
        {
            if((pstEncResult->eStatus & 0x01/*MHVE_FLAGS_FORCEI*/))
            {
                MI_U32 u32ByteRate;
                if(pstChnRes->stFLCtx.u32GopPBytes != 0)
                {
                    pstChnRes->stFLCtx.u32LastGopBytes = pstChnRes->stFLCtx.u32GopPBytes + pstChnRes->stFLCtx.u32GopIBytes;
                }
                if(pstChnRes->stFLCtx.u32LastGopBytes != 0)
                {
                    u32ByteRate = pstChnRes->stFLCtx.u32LastGopBytes / (pstChnRes->stFLCtx.u32GopFrmCnt + 1/*I frame*/)
                                * pstChnRes->stCtx.u32SrcFrmRateNum / pstChnRes->stCtx.u32SrcFrmRateDen;
                }
                else if(pstChnRes->stCtx.u32TargetBitrate != 0)
                {
                    u32ByteRate = pstChnRes->stCtx.u32TargetBitrate >> 3;
                }
                else
                {//on Fix QP, 2nd I frame could be < 2 x 1st I frame
                    u32ByteRate = pstChnRes->stFrameLost.u32FrmLostBpsThr >> 4;
                }
                if(u32ByteRate == 0)
                {//should not go here
                    DBG_ERR("0 byte rate\n");
                    u32ByteRate = 1;
                }
                pstChnRes->stFLCtx.u32IFrmThr = (MI_U32) (((MI_U64) pstEncResult->u32OutputBufUsed)
                        * (pstChnRes->stFrameLost.u32FrmLostBpsThr >> 3) / u32ByteRate);
                if(IS_MORE_DBG_INFO_FRM_LOST(pstChnRes))
                {
                    DBG_MGC("u32ByteRate:%d (bps:%d)\n", u32ByteRate, u32ByteRate * 8);
                    DBG_MGC("New I.thr:%d  (%d/%d)\n", pstChnRes->stFLCtx.u32IFrmThr,
                            u32ByteRate << 3, pstChnRes->stFrameLost.u32FrmLostBpsThr);
                }
                if(pstChnRes->stFLCtx.u32LastGopBytes != 0 && pstChnRes->stFLCtx.u32GopFrmCnt != 0)
                {
                    pstChnRes->stFLCtx.u32PFrmThr = (MI_U32) (((MI_U64) pstChnRes->stFLCtx.u32GopPBytes)
                            * (pstChnRes->stFrameLost.u32FrmLostBpsThr >> 3) / u32ByteRate
                            / pstChnRes->stFLCtx.u32GopFrmCnt);
                    if(IS_MORE_DBG_INFO_FRM_LOST(pstChnRes))
                    {
                        DBG_MGC("GopBytes:%d Last:%d cnt:%d\n", pstChnRes->stFLCtx.u32GopPBytes, pstChnRes->stFLCtx.u32LastGopBytes, pstChnRes->stFLCtx.u32GopFrmCnt);
                        DBG_MGC("New P.thr:%d\n", pstChnRes->stFLCtx.u32PFrmThr);
                    }
                }
                pstChnRes->stFLCtx.u32GopFrmCnt = 0;
                pstChnRes->stFLCtx.u32GopPBytes = 0;
                pstChnRes->stFLCtx.u32GopIBytes = pstEncResult->u32OutputBufUsed;
            }
            else
            {
                pstChnRes->stFLCtx.u32GopPBytes += pstEncResult->u32OutputBufUsed;
                pstChnRes->stFLCtx.u32GopFrmCnt++;
            }
        }
    }
    return bDropFrame ? MI_ERR_VENC_UNDEFINED : MI_SUCCESS;
}

static __inline MI_S32 _MI_VENC_DoSuperFrame(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_EncResult_t *pstEncResult,
                                             mi_sys_ChnTaskInfo_t *pstChnTask, MI_BOOL bJpeg, MI_BOOL *pbReencode)
{
    MI_S32 err = MI_SUCCESS;
    if(pstChnRes->stSuperFrame.eSuperFrmMode != E_MI_VENC_SUPERFRM_NONE)
    {
        MI_BOOL bSuperFrame = FALSE;
        if(pstChnRes->eRcPriority == E_MI_VENC_RC_PRIORITY_BITRATE_FIRST)
        {
            if(pstChnRes->_stStat.u32Kbps * 1000 < pstChnRes->stCtx.u32TargetBitrate)
            {
                return err;
            }
        }
        if(bJpeg || (pstEncResult->eStatus & 0x01/*MHVE_FLAGS_FORCEI*/))
        {
            if(IS_MORE_DBG_INFO_SUPER_FRM(pstChnRes))
                DBG_MGC("CH%2d I_bit:%d\n", pstChnTask->u32ChnId, pstEncResult->u32OutputBufUsed * 8);//used for parameter tuning/demo
            if((pstEncResult->u32OutputBufUsed * 8) >= pstChnRes->stSuperFrame.u32SuperIFrmBitsThr)
            {
                if(IS_MORE_DBG_WRN_SUPER_FRM(pstChnRes))
                    DBG_WRN("Super I Frame: CH%02d bits:%d\n", pstChnTask->u32ChnId, pstEncResult->u32OutputBufUsed * 8);
                bSuperFrame = TRUE;
            }
        }
        else
        {
            if(IS_MORE_DBG_INFO_SUPER_FRM(pstChnRes))
                DBG_MGC("CH%2d P_bit:%d\n", pstChnTask->u32ChnId, pstEncResult->u32OutputBufUsed * 8);//used for parameter tuning/demo
            if((pstEncResult->u32OutputBufUsed * 8) >= pstChnRes->stSuperFrame.u32SuperPFrmBitsThr)
            {
                if(IS_MORE_DBG_WRN_SUPER_FRM(pstChnRes))
                    DBG_WRN("Super P Frame: CH%02d bits:%d\n", pstChnTask->u32ChnId, pstEncResult->u32OutputBufUsed * 8);
                bSuperFrame = TRUE;
            }
        }
        if(bSuperFrame)
        {
            if(pstChnRes->stSuperFrame.eSuperFrmMode == E_MI_VENC_SUPERFRM_REENCODE)
            {
                *pbReencode = TRUE;
            }
            err = MI_ERR_VENC_UNDEFINED;//set undefined to drop the frame
        }
    }
    return err;
}

//need re-encode or discard
static MI_S32 _MI_VENC_AbortFrame(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_EncResult_t *pstEncResult,
                                           mi_sys_ChnTaskInfo_t *pstChnTask, MI_BOOL bJpeg, MI_BOOL bReencode)
{
    MI_S32 err = MI_SUCCESS;
    MS_S32 s32MhalRet;
    MI_BOOL bShrinkNextFrame = TRUE;

    if(!bJpeg)
    {
        MHAL_VENC_ParamInt_t stDummy;
        MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamInt_t, stDummy);
        s32MhalRet = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_RESTORE, (MHAL_VENC_Param_t*) &stDummy);
        if(s32MhalRet != 0)
        {
            //err = MI_ERR_VENC_UNDEFINED;
            DBG_ERR("Unable to restore at CH%2d err:%X\n", pstChnTask->u32ChnId, s32MhalRet);
        }
    }

    if(pstChnRes->stFLCtx.u8AbortCnt == 0)
    {
        if(bReencode)
            pstChnRes->stFLCtx.u8AbortCnt = reencode_max_cnt;
        else
            pstChnRes->stFLCtx.u8AbortCnt = discard_max_cnt;
    }
    else if(pstChnRes->stFLCtx.u8AbortCnt == 1)
    {
        pstChnRes->stFLCtx.u8AbortCnt = 0;
        DBG_WRN("Fail to re-encode/discard, generate next GOP\n");
        err = MI_VENC_IMPL_RequestIdr(pstChnRes->u8ChnId, TRUE);
        bShrinkNextFrame = FALSE;
        bReencode = FALSE;
    }
    else
    {
        pstChnRes->stFLCtx.u8AbortCnt --;
    }

    //Set to shrink the next frame.
    if(bShrinkNextFrame)
    {
        if(bJpeg)
        {
            MHAL_VENC_RcInfo_t stRc;
            MI_S32 s32Err;
            MI_S32 s32Qdf;
            MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stRc);
            s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
            if(s32Err != MI_SUCCESS)
            {
                DBG_ERR("Get Param err 0x%X.\n", s32Err);
                err = MI_ERR_VENC_UNDEFINED;
            }

            s32Qdf = bReencode ? reencode_dqf : discard_dqf;
            if(stRc.stAttrMJPGRc.u32Qfactor > 100 || stRc.stAttrMJPGRc.u32Qfactor < s32Qdf)
            {
                DBG_ERR("Invalid Q factor:%d\n", stRc.stAttrMJPGRc.u32Qfactor);
                err = MI_ERR_VENC_NOT_CONFIG;
            }
            else
            {
                stRc.stAttrMJPGRc.u32Qfactor -= s32Qdf;
                DBG_WRN("Set new Q factor:%d\n", stRc.stAttrMJPGRc.u32Qfactor);
                s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
                if(s32Err != MI_SUCCESS)
                {
                    DBG_ERR("Set Param err 0x%X.\n", s32Err);
                    err = MI_ERR_VENC_UNDEFINED;
                }
            }
        }
        else
        {
            MHAL_VENC_FrameCfg_t stFrame;
            MHAL_VENC_INIT_PARAM(MHAL_VENC_FrameCfg_t, stFrame);
            if(bReencode)
                stFrame.s8QpOffset = reencode_dqp * (reencode_max_cnt + 1 - pstChnRes->stFLCtx.u8AbortCnt);//FIXME -8, -16
            else
                stFrame.s8QpOffset = discard_dqp * (discard_max_cnt + 1 - pstChnRes->stFLCtx.u8AbortCnt);//FIXME -8, -16
            s32MhalRet = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_FRAME_CFG, (MHAL_VENC_Param_t*) &stFrame);
            if(s32MhalRet != 0)
            {
                DBG_ERR("Unable to set frame cfg at CH%2d err:%X\n", pstChnTask->u32ChnId, s32MhalRet);
            }
        }
    }

    if(bReencode)
    {
        pstChnRes->u32ReEncCnt++;
        mi_sys_RewindTask(pstChnTask);
    }
    else
    {
        pstChnRes->u32OutDropCnt++;
        mi_sys_DropTask(pstChnTask);
    }
    return err;
}

void _MI_VENC_ProcessTaskDone(mi_sys_ChnTaskInfo_t *pstChnTask, MHAL_VENC_EncResult_t *pstEncResult, MHAL_ErrCode_e err)
{
    MI_VENC_DevRes_t *pstDevRes;
    MI_VENC_ChnRes_t *pstChnRes;
    MI_SYS_BufInfo_t *pOutputBuf;
    MI_SYS_BufInfo_t *pstOutBufInfo = NULL;
    MI_SYS_BufInfo_t *pstInBufInfo = NULL;
    MI_BOOL bReencode = FALSE;
    MI_BOOL bJpeg = FALSE; //JPEG
    MI_S32 s32Err;

    MI_BOOL bIsLowLatencyTask = Is_VENC_TASK_LOWLATENCY(pstChnTask);

    if(bIsLowLatencyTask)
    {
        MI_U32 contentSize;
        contentSize = pstEncResult->u32OutputBufUsed - _mi_get_chntask_lowlatency_read_offset(pstChnTask);
        PreProcessLowLatencyOutputput(pstChnTask, contentSize, VENC_FRAME_END);
    }

    if(_mi_get_chntask_lowlatency_status(pstChnTask) == _MI_VENC_LOW_LATENCY_CANCEL || bIsLowLatencyTask)
    {
        pOutputBuf = pstChnTask->astOutputPortBufInfo[0];
        mi_sys_RewindBuf(pOutputBuf);
        pstChnTask->astOutputPortBufInfo[0] = NULL;
    }

    pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
    pstDevRes = pstChnRes->pstDevRes;
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
        bJpeg = TRUE;


    {
        s32Err = _MI_VENC_DoSuperFrame(pstChnRes, pstEncResult, pstChnTask, bJpeg, &bReencode);
        if(s32Err != MI_SUCCESS)
            err = s32Err;
    }

    {
        s32Err = _MI_VENC_DoFrameLost(pstChnRes, pstEncResult, bJpeg);
        if(s32Err != MI_SUCCESS)
            err = s32Err;
    }

    //DOUBLE check drop frame or re-encode policy from MI
    if (err == MI_SUCCESS)
    {
        pstOutBufInfo = pstChnTask->astOutputPortBufInfo[0];
        pstInBufInfo = pstChnTask->astInputPortBufInfo[0];
        if (pstInBufInfo && pstOutBufInfo)
        {
            MT_INFO("Found out port 0, type:%d VA:%p PA:%llX\n", pstChnTask->astOutputPortBufInfo[0]->eBufType,
                    pstOutBufInfo->stRawData.pVirAddr,
                    pstOutBufInfo->stRawData.phyAddr);
            MT_INFO("Found in port 0, type:%d VA:%p PA:%llX\n", pstInBufInfo->eBufType,
                    pstInBufInfo->stRawData.pVirAddr,
                    pstInBufInfo->stRawData.phyAddr);
            pstOutBufInfo->stRawData.u32ContentSize = _VENC_GET_FRAME_SIZE(pstChnRes, pstEncResult->u32OutputBufUsed);
            MT_INFO("raw:%d %d\n", pstChnTask->astOutputPortBufInfo[0]->stRawData.u32BufSize,
                    pstChnTask->astOutputPortBufInfo[0]->stRawData.u32ContentSize);
            //DBG_TMP("sz:%d\n", pstOutBufInfo->stRawData.u32ContentSize);
            if(pstOutBufInfo->stRawData.u32ContentSize == 0)
            {
                DBG_WRN("set buffer full:\n");
            }
            pstChnRes->u64SeqNum++;
            pstOutBufInfo->bUsrBuf = pstInBufInfo->bUsrBuf;
            pstOutBufInfo->u64Pts = pstInBufInfo->u64Pts;
            pstOutBufInfo->u64SidebandMsg = pstInBufInfo->u64SidebandMsg;
            pstOutBufInfo->bEndOfStream = pstInBufInfo->bEndOfStream;
            pstOutBufInfo->stRawData.u64SeqNum = pstChnRes->u64SeqNum;
            if (pstInBufInfo->bEndOfStream)
                DBG_INFO("EOS\n");

#if SUPPORT_KFS
            {
                static MI_U32 u32PrevSize;
                if(_stDbgFsInfo[pstDevRes->u32DevId].s32DumpBigIn &&
                        pstOutBufInfo->stRawData.u32ContentSize >= 2 * u32PrevSize)
                {
                    char szFn[64];
                    MI_U32 u32YuvSize = (pstInBufInfo->stFrameData.u32Stride[0] * pstInBufInfo->stFrameData.u16Height * 3) >> 1;
                    sprintf(szFn, "/tmp/big_%03d.yuv", (MI_U32)(pstChnRes->u64SeqNum - 1));
                    _MI_VENC_MapMemToFile(szFn, pstInBufInfo->stFrameData.phyAddr[0], u32YuvSize);
                }
                u32PrevSize = pstOutBufInfo->stRawData.u32ContentSize;
            }
#endif
        }
        else
        {
            if (!pstInBufInfo)
                DBG_WRN("Null input pointer\n");
            if (!pstOutBufInfo && !bIsLowLatencyTask)
                DBG_WRN("Null output pointer\n");
        }
    }

    if (err == MI_SUCCESS)
    {
        //size return 0 but reports MI_SUCCESS, consider it as big frame
        if (pstOutBufInfo && pstOutBufInfo->stRawData.u32ContentSize == 0)
        {
            DBG_WRN("set buffer full:\n");
            err = MI_ERR_VENC_BUF_FULL;
            {
                _MI_VENC_AbortFrame(pstChnRes, pstEncResult, pstChnTask, bJpeg, bReencode);
            }
            return;
        }
    }

    if (err == MI_SUCCESS)
    {
        pstChnRes->u32OutFrameCnt++;
        pstChnRes->stFLCtx.u8AbortCnt = 0;
        mi_sys_FinishAndReleaseTask(pstChnTask);
    }
    else if(MI_ERR_VENC_UNDEFINED == err)
    {//need re-encode or discard
        MI_S32 s32Err;
        s32Err = _MI_VENC_AbortFrame(pstChnRes, pstEncResult, pstChnTask, bJpeg, bReencode);
        if(s32Err != MI_SUCCESS)
            err = s32Err;
    }
}

MS_S32 MHAL_VENC_GetEngStatus(MI_VENC_DevRes_t *pstDevRes, MHAL_VENC_EncStatus_t *pstEncStatus)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstDevRes == NULL)
    {
        DBG_ERR("Null Pointer.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    if (pstDevRes->pstDrv == NULL)
    {
        DBG_ERR("Uninitialized drivers.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstDevRes->pstDrv;
    if (pstDrv->GetEngStatus)
    {
        s32Ret = pstDrv->GetEngStatus(pstDevRes->hHalDev, pstEncStatus);
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_EncodeCancel(MI_VENC_ChnRes_t *pstChnRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL || pstChnRes->pstDevRes->pstDrv == NULL)
    {
        DBG_ERR("Null Pointer.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->EncodeCancel)
    {
        s32Ret = pstDrv->EncodeCancel(pstChnRes->hInst);
    }
    return s32Ret;

}

MI_S32 _MI_VENC_ApplyPendingChnAttr(MI_VENC_ChnRes_t *pstChnRes, MI_U32 u32ChnId)
{
    MI_S32 s32Err = MI_SUCCESS;

    if(pstChnRes->bPendingChnAttr)
    {
        if(pstChnRes->bStart)
        {
            MI_S32 s32MhalRet;
            MHAL_VENC_ParamInt_t stDummy;
            MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamInt_t, stDummy);
            s32MhalRet = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_OFF, (MHAL_VENC_Param_t*) &stDummy);
            if(s32MhalRet != 0)
            {
                s32Err = MI_ERR_VENC_UNDEFINED;
                DBG_ERR("Unable to temp STREAM off at ch%2d err:%X\n", u32ChnId, s32MhalRet);
            }
        }

        s32Err = _MI_VENC_ConfigInstance(pstChnRes, &pstChnRes->stChnAttr);
        if(s32Err != MI_SUCCESS)
        {
            DBG_ERR("CH%2d Failed to config instance\n", u32ChnId);
        }
        if(pstChnRes->bStart)
        {
            MI_S32 s32MhalRet;
            s32MhalRet = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_ON, (MHAL_VENC_Param_t*) &pstChnRes->stVencInternalBuf);
            if (s32MhalRet != MI_SUCCESS)
            {
                s32Err = MI_ERR_VENC_UNDEFINED;
                DBG_ERR("Unable to temp STREAM on at ch%2d err:%X\n", u32ChnId, s32MhalRet);
            }
        }
        pstChnRes->bPendingChnAttr = FALSE;
    }
    return s32Err;
}
//triggered from each CMDQ
int VENC_ISR_Proc_Thread(void *data)
{
    MI_VENC_DevRes_t *pstDevRes = (MI_VENC_DevRes_t *) data;
    MHAL_CMDQ_CmdqInterface_t *cmdinf = NULL;
    //MI_U32 u32WaitCmdq = 0;//Use this to control if the command queue needs to be rechecked from the working list.
    MI_BOOL bFirstTask;//The first task in the working_task_list
    MI_BOOL bTaskTimeOut = FALSE;

    MI_SYS_BUG_ON(pstDevRes == NULL);

    while(!MI_VENC_ThreadShouldStop())
    {
        MI_VENC_TimedWaitInterruptible(&pstDevRes->stIrqWaitQueueHead, (pstDevRes->bFromIsr == TRUE), TASK_CHECK_INTERVAL);

        bTaskTimeOut = FALSE;
        if(_stDbgFsInfo[pstDevRes->u32DevId].s32EnCmdq == 0)
        {
            //force disabled
            cmdinf = NULL;
        }
        else
        {
            cmdinf = pstDevRes->cmdq;
        }
        if(pstDevRes->bFromIsr == FALSE && (pstDevRes->u32WaitCmdq == 0))
        {
            //signal time-out case, used only for checking thread stopping.
            continue;
        }
        if(pstDevRes->u32WaitCmdq >= 1)
        {
            if(pstDevRes->u32WaitCmdq == 1 && pstDevRes->bFromIsr == FALSE)
            {
                bTaskTimeOut = TRUE;
            }
            if(pstDevRes->bFromIsr == TRUE)
            {
                pstDevRes->u32WaitCmdq = 0;
            }
            else
            {
                pstDevRes->u32WaitCmdq --;
            }
        }
        if((cmdinf == NULL) && (pstDevRes->bFromIsr == FALSE) && (bTaskTimeOut == FALSE))
            continue;
        pstDevRes->bFromIsr = FALSE;

        //mi_venc_RecSwEnd(&pstDevRes->stUtil, 2, 0);
        DOWN(&pstDevRes->list_mutex);
        bFirstTask = TRUE;
        while(!MI_VENC_LIST_IS_EMPTY(&pstDevRes->working_task_list/*VENC_working_task_list*/))
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_VENC_ChnRes_t *pstChnRes;
            MHAL_ErrCode_e err;
            MHAL_VENC_EncResult_t stEncResult;

            MT_INFO("here %p %p\n", pstDevRes->working_task_list, MI_VENC_LIST_NEXT_ENTRY(pstDevRes, working_task_list));
            pstChnTask = MI_VENC_CONTAINER_OF(MI_VENC_LIST_NEXT_ENTRY(pstDevRes, working_task_list),
                                              mi_sys_ChnTaskInfo_t, listChnTask);

            if (!_IS_VALID_VENC_CHANNEL(pstChnTask->u32ChnId))
            {
                DBG_ERR("Invalid channel :%d\n", pstChnTask->u32ChnId);
                UP(&pstDevRes->list_mutex);
                return -1;
            }
            pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
            LOCK_CHNN(pstChnRes);

            MT_INFO("here gets:%p chn[%d]\n", pstChnTask, pstChnTask->u32ChnId);
            if(pstDevRes->eMiDevType == E_MI_VENC_DEV_JPEG)
            {
                if(_mi_get_chntask_lowlatency_status(pstChnTask) == _MI_VENC_LOW_LATENCY_ONGOING)
                {
                    PreProcessLowLatencyOutputput(pstChnTask, _mi_get_chntask_lowlatency_pack_size(pstChnTask), 0);
                    UNLOCK_CHNN(pstChnRes);
                    break;
                }
                else if (_mi_get_chntask_lowlatency_status(pstChnTask) == _MI_VENC_LOW_LATENCY_CANCEL)
                {

                    DBG_WRN("[ISR_PROC] task status: cancel\n");
                }
                else if (_mi_get_chntask_lowlatency_status(pstChnTask) == _MI_VENC_LOW_LATENCY_BASE)
                {
                    DBG_INFO("[ISR_PROC] task status: base\n");
                    UNLOCK_CHNN(pstChnRes);
                    break;
                }
            }
            else
            {
                // Check if current task has not been finished
                if(cmdinf && _MI_VENC_IsFenceL(_MI_VENC_ReadFence(cmdinf), FenceOfChTask(pstChnTask)))
                {
                    MS_BOOL bIdle;
                    cmdinf->MHAL_CMDQ_IsCmdqEmptyIdle(cmdinf, &bIdle);
                    //double check for CMDQ idle.
                    if((bIdle == TRUE) && _MI_VENC_IsFenceL(_MI_VENC_ReadFence(cmdinf), FenceOfChTask(pstChnTask)))
                    {
                        DBG_ERR("invalid fence %04x, %04x!\n", _MI_VENC_ReadFence(cmdinf), pstChnTask->u32Reserved0);
                        pstChnRes->eBufState = MI_VENC_BUFSTATE_CMDQ_ERROR;
                    }
                    else
                    {
                        //Command Queue is doing something else, which is OK.
                        pstChnRes->eBufState = MI_VENC_BUFSTATE_WAIT_CMDQ;
                        if(bFirstTask && bTaskTimeOut)
                        {
                            DBG_ERR("Time-out CmdQ Ch%2d %X %X\n", pstChnTask->u32ChnId, _MI_VENC_ReadFence(cmdinf),
                                    pstChnTask->u32Reserved0);
                            //cmdinf->MHAL_CMDQ_CmdqAbortBuffer(cmdinf);
                            cmdinf->MHAL_CMDQ_CmdqResetEngine(cmdinf);
                            pstChnRes->eBufState = MI_VENC_BUFSTATE_CMDQ_ERROR;
                        }
                        else
                        {
                            /* 1 ms sleep could not be added. On multiple channel case, it sleeps too long, such as 10 ms,
                             * and cause performance drop. Here just read again without sleep */
                            //if(bFirstTask) MI_VENC_MsSleep(1);
                            if(_MI_VENC_IsFenceL(_MI_VENC_ReadFence(cmdinf), FenceOfChTask(pstChnTask)))
                            {
                                //triple read fenced, then start time-out process.
                                //DBG_WRN("Wait CmdQ Ch%2d %X %X\n", pstChnTask->u32ChnId, _MI_VENC_ReadFence(cmdinf), pstChnTask->u32Reserved0);
                                if(pstDevRes->u32WaitCmdq == 0)
                                {
                                    pstDevRes->u32WaitCmdq = CMDQ_WAIT_TIME_OUT;//50x4 times, 200 ms timeout for one frame.
                                }
                                UNLOCK_CHNN(pstChnRes);
                                break;
                            }
                        }
                        //DBG_TMP("OK-LAG CmdQ Ch%2d %X %X\n", pstChnTask->u32ChnId, _MI_VENC_ReadFence(cmdinf), pstChnTask->u32Reserved0);
                    }
                }
            }
            //pstChnRes->bSelected = FALSE;//moved to a later place in this function

            list_del(&pstChnTask->listChnTask);
            bFirstTask = FALSE;

            pstChnRes->eBufState = MI_VENC_BUFSTATE_POST_ENCODE;

            //HAL driver check the result status from IP ISR or IP registers.
            //It would do rate control if any.
            MT_WRN("enc_done\n");
            memset(&stEncResult, 0, sizeof(stEncResult));
            err = MHAL_VENC_EncDone(pstChnRes, &stEncResult);
            pstChnRes->eBufState = MI_VENC_BUFSTATE_ENCODE_DONE;
            MT_WRN("enc_done ret:%d(0x%X) len:%d\n", err, err, stEncResult.u32OutputBufUsed);

            if(pstChnRes->bRequestEnableIdr)
            {
                //because this is non-blocking asynchronous code, no chance to reply the error.
                (void)_MI_VENC_ApplyEnableIdr(pstChnRes);
                pstChnRes->bRequestEnableIdr = FALSE;
            }

            _MI_VENC_ProcessTaskDone(pstChnTask, &stEncResult, err);

#if DEV_UP == TIMING_ENCODE_FINISH_AND_RELEASE
            UP(&pstChnRes->pstDevRes->frame_sem);
#endif
            pstChnRes->eBufState = MI_VENC_BUFSTATE_IDLE;
            UNLOCK_CHNN(pstChnRes);
        }
        UP(&pstDevRes->list_mutex);
    }
    DBG_EXIT_OK();
    return 0;
}
MI_VENC_CreateThreadWrapper(VENC_ISR_Proc_Thread)

MS_S32 _MI_VENC_EncodeAddOutBuff(MI_VENC_DevRes_t *pstDevRes, MHAL_VENC_AddOutBuf_t *pstAddOutBuf)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstDevRes == NULL)
    {
        DBG_ERR("Null Pointer.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    if (pstDevRes->pstDrv == NULL)
    {
        DBG_ERR("Uninitialized drivers.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstDevRes->pstDrv;
    if (pstDrv->EncodeAddOutBuff)
    {
        s32Ret = pstDrv->EncodeAddOutBuff(pstDevRes->hHalDev, pstAddOutBuf);
    }
    return s32Ret;

}

//should not be used any more
MI_VENC_Irqreturn_t VENC_IP_ISR(int irq, void *data)
{
    MI_VENC_DevRes_t *pstDevRes = (MI_VENC_DevRes_t *)data;
    MHAL_VENC_EncStatus_t engStatus;
    MS_S32 s32Ret;

    if (pstDevRes)
    {
        if (pstDevRes->eMiDevType >= E_MI_VENC_DEV_MAX)
        {
            DBG_ERR("Invalid eMiDevType %d\n", pstDevRes->eMiDevType);
            return IRQ_HANDLED;
        }
        if(_stDbgFsInfo[pstDevRes->u32DevId].s32Isr == 1)
        {
            DBG_WRN("Skip IRQ on D:%d\n", pstDevRes->u32DevId);
            //have to clear ISR while faking ISR missing, otherwise it would
            //keep entering this ISR
            s32Ret = MHAL_VENC_GetEngStatus(pstDevRes, &engStatus);
            if(s32Ret == MI_ERR_VENC_UNDEFINED)
            {
                MHAL_VENC_IsrProc(pstDevRes);
            }
            _stDbgFsInfo[pstDevRes->u32DevId].s32Isr = 0;
            return IRQ_HANDLED;
        }

        MT_WRN("%s\n", astMHalDevConfig[pstDevRes->eMiDevType].szName);
        pstDevRes->bFromIsr = TRUE;
    }
    else
    {
        DBG_ERR("Null input pointer.\n");
    }

    s32Ret = MHAL_VENC_GetEngStatus(pstDevRes, &engStatus);

    if(s32Ret != MI_ERR_VENC_UNDEFINED)
    {
        if(engStatus.eEncSts == E_MHAL_VENC_BUF_RDY_UNDER_RUN)
        {
            MI_BOOL bFound = FALSE;
            struct list_head *pos = NULL, *n = NULL;

            //raw_spin_lock(&pstDevRes->working_task_spinlock);
            MI_VENC_LIST_FOR_EACH_SAFE(pos, n, &pstDevRes->working_task_list)
            {
                mi_sys_ChnTaskInfo_t *pstChnTask;
                MHAL_VENC_AddOutBuf_t addOutBuf;
                MI_VENC_ChnRes_t *pstChnRes;

                pstChnTask = MI_VENC_CONTAINER_OF(pos, mi_sys_ChnTaskInfo_t, listChnTask);
                pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
                if(pstChnTask->astInputPortBufInfo[0]->stFrameData.phyAddr[0] == engStatus.phyCurrInBuf)
                {
                    bFound = TRUE;
                    if(Is_VENC_TASK_LOWLATENCY(pstChnTask))
                    {
                        //low latency mode
                        _mi_set_chntask_lowlatency_write_offset_move(pstChnTask, _mi_get_chntask_lowlatency_pack_size(pstChnTask));
                        if(_mi_get_chntask_lowlatency_write_offset(pstChnTask) + _mi_get_chntask_lowlatency_pack_size(pstChnTask)
                                > pstChnTask->astOutputPortBufInfo[0]->stRawData.u32BufSize)
                        {
                            DBG_INFO("[Buff full] task cancel\n");
                            _mi_set_chntask_lowlatency_status(pstChnTask, _MI_VENC_LOW_LATENCY_CANCEL);
                            MHAL_VENC_EncodeCancel(pstChnRes);
                        }
                        else
                        {
                            addOutBuf.u32OutBuffSize = _mi_get_chntask_lowlatency_pack_size(pstChnTask);
                            addOutBuf.phyOutBuff = pstChnTask->astOutputPortBufInfo[0]->stRawData.phyAddr + _mi_get_chntask_lowlatency_write_offset(pstChnTask);
                            _mi_set_chntask_lowlatency_status(pstChnTask, _MI_VENC_LOW_LATENCY_ONGOING);
                            _MI_VENC_EncodeAddOutBuff(pstDevRes, &addOutBuf);
                        }
                    }
                    else
                    {
                        DBG_INFO("[not LL mode] task cancel\n");
                        _mi_set_chntask_lowlatency_status(pstChnTask, _MI_VENC_LOW_LATENCY_CANCEL);
                        MHAL_VENC_EncodeCancel(pstChnRes);
                    }
                    break;
                }
                else
                {
                    DBG_ERR("task addr not equal\n");
                }
            }
            //raw_spin_unlock(&pstDevRes->working_task_spinlock);
            MI_SYS_BUG_ON(!bFound );
        }
        else if(engStatus.eEncSts == E_MHAL_VENC_FRM_DONE)
        {
            struct list_head *pos = NULL, *n = NULL;
            //raw_spin_lock(&pstDevRes->working_task_spinlock);
            MI_VENC_LIST_FOR_EACH_SAFE(pos, n, &pstDevRes->working_task_list)
            {
                mi_sys_ChnTaskInfo_t *pstChnTask;
                pstChnTask = MI_VENC_CONTAINER_OF(pos, mi_sys_ChnTaskInfo_t, listChnTask);
                _mi_set_chntask_lowlatency_status(pstChnTask, _MI_VENC_LOW_LATENCY_DONE);
            }
            //raw_spin_unlock(&pstDevRes->working_task_spinlock);
        }
        else
        {
            DBG_ERR("[Venc eng] other status!\n");
        }
    }
    else
    {
        MHAL_VENC_IsrProc(pstDevRes);
    }

    pstDevRes->bFromIsr = TRUE;
    MI_VENC_WAKE_UP_QUEUE_IF_NECESSARY(_ModRes.devs[pstDevRes->eMiDevType].stIrqWaitQueueHead);
    return IRQ_HANDLED;
}

static MI_S32 _MI_VENC_FindDevResFromId(MI_VENC_CHN VeChn, MI_VENC_DevRes_t **pstDevRes)
{
    MI_VENC_ChnRes_t *pstChnRes;

    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        DBG_ERR("Invalid channel :%d\n", VeChn);
        return MI_ERR_VENC_INVALID_CHNID;
    }
    pstChnRes = &_ModRes.astChnRes[VeChn];

    LOCK_CHNN(pstChnRes);
    if (pstChnRes->bCreate && pstChnRes->bStart)
    {
        *pstDevRes = pstChnRes->pstDevRes;
    }
    else
    {
        *pstDevRes = NULL;
        UNLOCK_CHNN(pstChnRes);
        return MI_ERR_VENC_CHN_NOT_STARTED; //not found
    }
    UNLOCK_CHNN(pstChnRes);
    return MI_SUCCESS;
}

typedef struct
{
    int totalAddedTask;
} VENC_Iterator_WorkInfo;

static MI_S32 _MI_VENC_CalChnBufsize(MI_VENC_ChnRes_t *pstChnRes)
{
    if (pstChnRes == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(pstChnRes->u32BufSize == 0)//auto, decided here in kernel
    {
        switch (pstChnRes->stChnAttr.stVeAttr.eType)
        {
            case E_MI_VENC_MODTYPE_H264E://pixels * 0.5
                pstChnRes->u32BufSize = (pstChnRes->stChnAttr.stVeAttr.stAttrH264e.u32PicWidth
                                         * pstChnRes->stChnAttr.stVeAttr.stAttrH264e.u32PicHeight * 4) >> 3;
                break;
            case E_MI_VENC_MODTYPE_H265E://pixels * 0.5
                pstChnRes->u32BufSize = (pstChnRes->stChnAttr.stVeAttr.stAttrH265e.u32PicWidth
                                         * pstChnRes->stChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 4) >> 3;
                break;
            case E_MI_VENC_MODTYPE_JPEGE://was pixels * 1 but buffer overflow while qfactor = 100.
                //probably need a q-factor based calculation.
                pstChnRes->u32BufSize = (pstChnRes->stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth
                                         * pstChnRes->stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight * 8) >> 3;
                break;
            default:
                pstChnRes->u32BufSize = DEFAULT_ES_SIZE;
                break;
        }
    }
    return MI_SUCCESS;
}

#define TRACE_TASK_GROUP (0)//trace which channel are processed in the same batch for device 0
#if TRACE_TASK_GROUP
    char seq[256];
    int  iseq = 0;
    int  d0=0;
#endif
mi_sys_TaskIteratorCBAction_e MI_VENC_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    int i;
    int frc_blocked_count = 0;
    int allocated_buf_count = 0;

    VENC_Iterator_WorkInfo *workInfo = (VENC_Iterator_WorkInfo *)pUsrData;
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_S32 ret = MI_ERR_VENC_UNDEFINED;
    MI_SYS_BufConf_t *pstOutCfg;
    MI_SYS_BufInfo_t *pstInputPortBufInfo;

    ret = _MI_VENC_FindDevResFromId(pstTaskInfo->u32ChnId, &pstDevRes);
    if (pstDevRes == NULL)
    {
        DBG_ERR("Null res pointer!\n");
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    }
    MT_WRN("Chn:%d\n", pstTaskInfo->u32ChnId);
    pstInputPortBufInfo = pstTaskInfo->astInputPortBufInfo[0];

    if (ret == MI_SUCCESS)
    {
        //input format is not correct
        if(pstInputPortBufInfo->eBufType != E_MI_SYS_BUFDATA_FRAME)
        {
            DBG_WRN("Unsupported in Buf Type %d\n", pstInputPortBufInfo->eBufType);
            ret = MI_ERR_VENC_NOT_SUPPORT;
        }
        else if ((pstInputPortBufInfo->stFrameData.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV_MST_420)
                 && (pstInputPortBufInfo->stFrameData.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                 && (pstInputPortBufInfo->stFrameData.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV))
        {
            DBG_WRN("Unsupported in Pix Type %d\n", pstTaskInfo->astInputPortBufInfo[0]->stFrameData.ePixelFormat);
            ret = MI_ERR_VENC_NOT_PERM;
        }
    }
    if (ret != MI_SUCCESS)
    {
        // Drop input buffer because the channel is not available for now.
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        DBG_EXIT_ERR("Ch %d is not available. Drop frame directly.\n", pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    //avoid selecting the same channel for encoding dependency
    pstChnRes = _ModRes.astChnRes + pstTaskInfo->u32ChnId;

    if(pstChnRes->eBufState != MI_VENC_BUFSTATE_IDLE)//if (pstChnRes->bSelected)
    {
        DBG_INFO("Multiple frames in channel %d\n", pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    //prepare output buffer
    pstOutCfg = pstTaskInfo->astOutputPortPerfBufConfig + 0;//pin 0
    pstOutCfg->u64TargetPts = pstInputPortBufInfo->u64Pts;
    pstOutCfg->eBufType     = E_MI_SYS_BUFDATA_RAW;
    (void)_MI_VENC_CalChnBufsize(pstChnRes);
    pstOutCfg->stRawCfg.u32Size = pstChnRes->u32BufSize;
    pstOutCfg->u32Flags = MI_SYS_MAP_VA;

    if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_SUCCESS)
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;

    if(0)
    {
        if (pstTaskInfo->astOutputPortBufInfo[0])
        {
            MT_INFO("addr:V:%X, P:%llX\n", pstTaskInfo->astOutputPortBufInfo[0]->stRawData.pVirAddr,
                    pstTaskInfo->astOutputPortBufInfo[0]->stRawData.phyAddr);
        }
    }

#if 1
    //for( i=0; i < MI_SYS_MAX_OUTPUT_PORT_CNT; i++)
    i = 0;
    {
        MI_SYS_BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

        if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i])
            frc_blocked_count++;
        else if(pstTaskInfo->astOutputPortBufInfo[i])
            allocated_buf_count++;
    }
    if (allocated_buf_count == 0)
    {
        MT_ERR("Allocated %d output buffers.", allocated_buf_count);
        //for( i=0; i < MI_SYS_MAX_OUTPUT_PORT_CNT; i++)
        i = 0;
        {
            MT_INFO("outCfg[%]: ty:%d pts:%lld\n", i, pstTaskInfo->astOutputPortPerfBufConfig[i].eBufType,
                    pstTaskInfo->astOutputPortPerfBufConfig[i].u64TargetPts);
        }
    }
    else
    {
        MT_WRN("Allocated %d output buffers.", allocated_buf_count);
    }

#elif 0
    //XXX crashes here
    ret = mi_sys_GetChnBufInfo(pstTaskInfo->miSysDrvHandle, pstTaskInfo->u32ChnId, &stChnBufInfo);
    if (ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to get ChnBufInfo(stChnBufInfo)\n");
        //not found, use mi_sys_FinishAndReleaseTask() instead?
        MI_VENC_MOCK(mi_sys_FinishTaskBuf(pstTaskInfo));
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    for( i=0; i < stChnBufInfo.u32OutputPortNum; i++)
    {
        MI_SYS_BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

        if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i])
            frc_blocked_count++;
        else if(pstTaskInfo->astOutputPortBufInfo[i])
            allocated_buf_count++;
    }
#else //work around
    stChnBufInfo.u32OutputPortNum = 1;
    if(pstTaskInfo->astOutputPortBufInfo[0])
    {
        allocated_buf_count++;
    }
#endif

    //we have only one output port
    //disable this check for not having stChnBufInfo.u32OutputPortNum
    //MI_SYS_BUG_ON(frc_blocked_count + allocated_buf_count > stChnBufInfo.u32OutputPortNum);

    if(frc_blocked_count)
    {
        //DBG_WRN("CH%2d FRC dropped an output frame. Check if output frame queue overflows.\n", pstTaskInfo->u32ChnId);
        //drop input frame since we don't need to encode it
        pstChnRes->u32InDropCnt++;
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    //check if lacking of output buffer
    if(allocated_buf_count==0)
    {
#if 0
        DBG_WRN("No output port on channel %d\n", pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
#else
        pstChnRes->u32InDropCnt++;
        pstChnRes->u32InBlockCnt++;
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
#endif
    }

    MT_INFO("task handles pstTaskInfo = %p, and pstDevRes = %p\n", pstTaskInfo->miSysDrvHandle, pstDevRes->hMiDev);
    MT_INFO("ID:%d cnt:%d %d\n", pstTaskInfo->u32ChnId, pstChnRes->semLock.count,
            pstDevRes->list_mutex.count);
    //add to to-do list
    if (pstTaskInfo->miSysDrvHandle == pstDevRes->hMiDev)
    {
        LOCK_CHNN(pstChnRes);
        pstChnRes->eBufState = MI_VENC_BUFSTATE_SELECTED;
        UNLOCK_CHNN(pstChnRes);
        DOWN(&pstDevRes->list_mutex);
        MI_VENC_LIST_ADD_TAIL(&pstTaskInfo->listChnTask, &pstDevRes->todo_task_list);
        UP(&pstDevRes->list_mutex);
    }
    else
    {
        DBG_ERR("task handle = %p, and pstDevRes = %p\n", pstTaskInfo->miSysDrvHandle, pstDevRes->hMiDev);
    }

    //stop iteration if exceed device capacity
    if(++workInfo->totalAddedTask >= MI_VENC_MAX_CHN_NUM_PER_MODULE)
    {
        MT_WRN("STOP\n");
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    }
    else
    {
        //MT_INFO("OK\n");
#if TRACE_TASK_GROUP
        if(pstDevRes->u32DevId == 0)
        {
            seq[iseq++]='0'+pstTaskInfo->u32ChnId;
            d0 = 1;
        }
#endif
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }
}

static MS_S32 _MI_VENC_MoveChnTskToWorking(mi_sys_ChnTaskInfo_t *pstChnTask, MI_VENC_ChnRes_t *pstChnRes)
{
    MI_VENC_DevRes_t *pstDevRes;

    if(pstChnRes == NULL || pstChnRes->pstDevRes == NULL)
        return MI_ERR_VENC_NULL_PTR;
    pstDevRes = pstChnRes->pstDevRes;

    MT_INFO("pstChnTask %p new:%p\n", pstChnTask, pstChnTask->listChnTask);
    list_del(&pstChnTask->listChnTask);

    DOWN(&pstChnRes->pstDevRes->list_mutex);
    MT_INFO("pstChnTask %p new:%p\n", pstChnTask, pstChnTask->listChnTask);
    //MT_WRN("Work List empty? %d\n", list_empty_careful(pstChnRes->pstDevRes->working_task_list));

    MI_VENC_LIST_ADD_TAIL(&pstChnTask->listChnTask, &pstChnRes->pstDevRes->working_task_list);
    //MT_WRN("Work List empty? %d\n", list_empty_careful(pstChnRes->pstDevRes->working_task_list));

    UP(&pstChnRes->pstDevRes->list_mutex);
    return MI_SUCCESS;
}

int VencWorkThread(void *data)
{
    MI_U16 fence = 1;//map to total frame count of each device driver. That counter starts from 1.
    MI_VENC_DevRes_t *pstDevRes = (MI_VENC_DevRes_t *)data;
    MHAL_CMDQ_CmdqInterface_t *cmdinf = NULL;

    MI_SYS_BUG_ON(pstDevRes == NULL);

    while((!MI_VENC_ThreadShouldStop()))
    {
        VENC_Iterator_WorkInfo workinfo;
        MI_BOOL bPushFrameOK = FALSE;
        struct list_head *pos = NULL, *n = NULL;
        MI_VENC_ChnRes_t *pstChnRes = NULL;
        MS_S32 s32Err;
        MS_S32 s32CmdqErr;
        int nCnt = 0;

        workinfo.totalAddedTask = 0;

        if(_stDbgFsInfo[pstDevRes->u32DevId].s32EnCmdq == 0)
        {
            //force disabled
            cmdinf = NULL;
        }
        else
        {
            cmdinf = pstDevRes->cmdq;
        }
        //MT_INFO("pstDevRes->hMiDev:%X\n", pstDevRes->hMiDev);
#if TRACE_TASK_GROUP
        if(pstDevRes->u32DevId == 0)
        {
            iseq = 0;
            seq[iseq++]='G';
            d0 = 0;
        }
#endif
        mi_sys_DevTaskIterator(pstDevRes->hMiDev, MI_VENC_TaskIteratorCallBK, &workinfo);
        if(MI_VENC_LIST_IS_EMPTY(&pstDevRes->todo_task_list))
        {
            if (nCnt++ >= 100)
            {
                DBG_INFO("... waiting on work ...\n");
                nCnt = 0;
            }
            //schedule_timeout_interruptible(1);//was schedule();
            MI_VENC_TimedScheduleInterruptible(1);
            //in Linux wait_event_timeout()
            mi_sys_WaitOnInputTaskAvailable(pstDevRes->hMiDev, TASK_CHECK_INTERVAL);
            continue;
        }
        if(workinfo.totalAddedTask == 0)
        {
            MI_VENC_TimedScheduleInterruptible(1);
            continue;
        }

#if TRACE_TASK_GROUP
        if(pstDevRes->u32DevId == 0)
        {
            seq[iseq++]=' ';
            seq[iseq++]='e';
        }
#endif

        MI_VENC_LIST_FOR_EACH_SAFE(pos, n, &pstDevRes->todo_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            int loop_cnt = 0;
            MHAL_VENC_InOutBuf_t stEncBufs;
            MI_SYS_BufInfo_t *pstInBuf, *pstOutBuf;

            memset(&stEncBufs, 0, sizeof(stEncBufs));
            pstChnTask = container_of(pos, mi_sys_ChnTaskInfo_t, listChnTask);
            pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
            if (!_IS_VALID_VENC_CHANNEL(pstChnTask->u32ChnId))
            {
                //should not goes here
                MI_SYS_BUG_ON(pstChnTask->u32ChnId);
                break;
            }
            MT_INFO("Got a frame!\n");
#if DEV_DOWN == TIMING_IN_GOT_A_FRAME
            DOWN_CPU_MODE(cmdinf, pstChnRes->pstDevRes->frame_sem);
#endif

            //prepare in out buffer
            LOCK_CHNN(pstChnRes);
            pstChnRes->eBufState = MI_VENC_BUFSTATE_PRE_ENCODE;
            pstInBuf = pstChnTask->astInputPortBufInfo[0];//pin 0
            pstOutBuf = pstChnTask->astOutputPortBufInfo[0];//pin 0
            if (pstInBuf == NULL)
            {
                DBG_ERR("Invalid Input buffer, skip frame\n");
                list_del(&pstChnTask->listChnTask);
                UNLOCK_CHNN(pstChnRes);
#if DEV_DOWN == TIMING_IN_GOT_A_FRAME
                UP_CPU_MODE(cmdinf, pstChnRes->pstDevRes->frame_sem);
#endif
                continue;
            }
            if (pstOutBuf == NULL)
            {
                DBG_ERR("Invalid Output buffer, skip frame\n");
                list_del(&pstChnTask->listChnTask);
                UNLOCK_CHNN(pstChnRes);
#if DEV_DOWN == TIMING_IN_GOT_A_FRAME
                UP_CPU_MODE(cmdinf, pstChnRes->pstDevRes->frame_sem);
#endif
                continue;
            }
            if(pstChnRes->bStart != TRUE)
            {
                DBG_WRN("Channel %d Stopped\n", pstChnTask->u32ChnId);
                list_del(&pstChnTask->listChnTask);
                s32Err = mi_sys_FinishAndReleaseTask(pstChnTask);//drop the frame here
                if (s32Err != MI_SUCCESS)
                {
                    DBG_ERR("Encode release frame[%d] Error:%X\n", pstChnTask->u32ChnId, s32Err);
                }
                UNLOCK_CHNN(pstChnRes);
#if DEV_DOWN == TIMING_IN_GOT_A_FRAME
                UP_CPU_MODE(cmdinf, pstChnRes->pstDevRes->frame_sem);
#endif
                continue;
            }
            stEncBufs.phyInputYUVBuf1 = pstInBuf->stFrameData.phyAddr[0];
            stEncBufs.phyInputYUVBuf2 = pstInBuf->stFrameData.phyAddr[1];
            stEncBufs.phyInputYUVBuf3 = pstInBuf->stFrameData.phyAddr[2];
            stEncBufs.pCmdQ = cmdinf;
            stEncBufs.u32InputYUVBuf1Size = pstInBuf->stFrameData.u32Stride[0] * pstInBuf->stFrameData.u16Height;

            //TODO should be for NV12 only
            stEncBufs.u32InputYUVBuf1Size = (stEncBufs.u32InputYUVBuf1Size * 3) >> 1;
            stEncBufs.u32InputYUVBuf2Size = 0; //ignored and be calculated in driver
            stEncBufs.u32InputYUVBuf3Size = 0; //ignored and be calculated in driver
            stEncBufs.phyOutputBuf = pstOutBuf->stRawData.phyAddr;
            stEncBufs.virtOutputBuf = (MS_PHYADDR)(MS_U32)pstOutBuf->stRawData.pVirAddr;
            stEncBufs.pFlushCacheCb = mi_sys_VFlushInvCache;
            if(pstChnRes->bByFrame)
            {
                stEncBufs.u32OutputBufSize = pstOutBuf->stRawData.u32BufSize;
            }
            else
            {
                stEncBufs.u32OutputBufSize = LOW_LATENCY_PACK_SIZE;
                if(LOW_LATENCY_PACK_SIZE >= pstOutBuf->stRawData.u32BufSize)
                {
                    stEncBufs.u32OutputBufSize = pstOutBuf->stRawData.u32BufSize;
                }
                _mi_set_chntask_lowlatency_read_offset(pstChnTask, 0); //set slice read ptr offset as 0
                _mi_set_chntask_lowlatency_write_offset(pstChnTask, 0); //set slice write ptr offset as 0
                _mi_set_chntask_lowlatency_pack_size(pstChnTask, stEncBufs.u32OutputBufSize); //slice size
            }

            stEncBufs.bRequestI = pstChnRes->bRequestIdr;
            pstChnRes->bRequestIdr = FALSE;
            if(pstChnRes->eInputFormat != pstInBuf->stFrameData.ePixelFormat)
            {
                if(pstDevRes->eMiDevType == E_MI_VENC_DEV_JPEG)
                {
                    if(pstInBuf->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV || pstInBuf->stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                    {
                        pstChnRes->eInputFormat = pstInBuf->stFrameData.ePixelFormat;
                        pstChnRes->bPendingChnAttr = TRUE;
                    }
                    else
                    {
                        DBG_WRN("[%s] Dev:%d, yuv format(%d) not support\n", __func__, pstDevRes->eMiDevType, pstInBuf->stFrameData.ePixelFormat);
                    }
                }
                else
                {
                    DBG_WRN("[%s] Dev:%d, yuv format(%d) not support\n", __func__, pstDevRes->eMiDevType, pstInBuf->stFrameData.ePixelFormat);
                }
            }

            MT_INFO("Got a frame MIU: %llX! cmdq:%p\n", stEncBufs.phyInputYUVBuf1, cmdinf);
            MT_INFO("Got a frame %p (mi_sys va)\n", pstInBuf->stFrameData.pVirAddr[0]);
            MT_INFO("Got a frame sz:%d %d!\n", stEncBufs.u32InputYUVBuf1Size, stEncBufs.u32OutputBufSize);

            if (cmdinf)
            {
                MS_U32 u32Size;
                while(pstDevRes->u32CmdqCnt > (u32Size = cmdinf->MHAL_CMDQ_CheckBufAvailable(cmdinf, pstDevRes->u32CmdqCnt)))
                {
                    //command queue buffer size is not enough for this frame
                    MI_VENC_TimedWaitInterruptible(&pstDevRes->stWorkWaitQueueHead, FALSE, (2));
                    loop_cnt++;
                    MI_SYS_BUG_ON(loop_cnt > 1000);//engine hang
                }
            }

            s32Err = _MI_VENC_MoveChnTskToWorking(pstChnTask, pstChnRes);
            if (s32Err != MI_SUCCESS)
            {
                DBG_ERR("List error %X\n", s32Err);
            }
            else
            {
                if(_stDbgFsInfo[pstDevRes->u32DevId].s32DumpIn > 0) //typical condition, dump N frames
                    //if(pstChnRes->u64SeqNum < 3) //dump first 3 frames from start to get the very first frames.
                {
                    char szFn[64];
                    sprintf(szFn, "/tmp/in_d%dc%02d_%03d.yuv", pstDevRes->u32DevId, pstChnTask->u32ChnId,
                            (MI_U32) pstChnRes->u64SeqNum);
                    _MI_VENC_MapMemToFile(szFn, stEncBufs.phyInputYUVBuf1, stEncBufs.u32InputYUVBuf1Size);
                    _stDbgFsInfo[pstDevRes->u32DevId].s32DumpIn--;
                }

                s32Err = _MI_VENC_InsertUserData(pstChnRes);
                s32Err = _MI_VENC_ApplyPendingChnAttr(pstChnRes, pstChnTask->u32ChnId);
                pstChnRes->eBufState = MI_VENC_BUFSTATE_ENCODE;
                //driver need to make sure that CMDQ buffer integrity
#if TRACE_TASK_GROUP
                if(pstDevRes->u32DevId == 0)
                {
                    DBG_TMP("Enc:d0 c%d\n", pstChnRes->u8ChnId);
                    seq[iseq++] = '0' + pstChnRes->u8ChnId;
                    d0 = 1;
                }
#endif
                s32Err = MHAL_VENC_EncodeOneFrame(pstChnRes, &stEncBufs);
                pstChnRes->u32InFrameCnt++;
                pstChnRes->eBufState = MI_VENC_BUFSTATE_WAIT_ISR;
            }

            if (s32Err != MI_SUCCESS)
            {
                //MHAL_CMDQ_CmdqAbortBuffer here?
                pstChnRes->eBufState = MI_VENC_BUFSTATE_IDLE;
                pstChnRes->u32InDropCnt++;
                DBG_ERR("Encode Chn[%d] Error:%X\n", pstChnTask->u32ChnId, s32Err);
                s32Err = mi_sys_FinishAndReleaseTask(pstChnTask);//drop the frame here
                if (s32Err != MI_SUCCESS)
                {
                    DBG_ERR("Encode release frame[%d] Error:%X\n", pstChnTask->u32ChnId, s32Err);
                }
                UNLOCK_CHNN(pstChnRes);
#if DEV_DOWN == TIMING_IN_GOT_A_FRAME
                UP_CPU_MODE(cmdinf, pstChnRes->pstDevRes->frame_sem);
#endif
                continue;
            }

            bPushFrameOK = TRUE;

            if(cmdinf)
            {
                if(0)//set 1 to test CMDQ dummy read busy-bus latency case
                {
                    int i;
                    for (i = 0; i < 500; ++i)
                    {
                        s32CmdqErr = cmdinf->MHAL_CMDQ_WriteDummyRegCmdq(cmdinf, fence-1);
                    }
                }
                if(_stDbgFsInfo[pstDevRes->u32DevId].s32Cmdq == 1)
                {
                    //s32CmdqErr = cmdinf->MHAL_CMDQ_WriteRegCmdq(cmdinf, 0, 0);//this would hang CMDQ
                    s32CmdqErr = cmdinf->MHAL_CMDQ_CmdqPollRegBits(cmdinf, 0x123500, 0, 0x01, 0);
                    if(s32CmdqErr != MHAL_SUCCESS)
                    {
                        DBG_ERR("CMDQ write error %d.\n", s32CmdqErr);
                    }
                    DBG_WRN("Block CMDQ on D:%d CH:%d\n", pstDevRes->u32DevId, pstChnTask->u32ChnId);
                    _stDbgFsInfo[pstDevRes->u32DevId].s32Cmdq = 0;
                }
                s32CmdqErr = cmdinf->MHAL_CMDQ_WriteDummyRegCmdq(cmdinf, fence);
                if(s32CmdqErr == MHAL_SUCCESS)
                {
                    s32CmdqErr = cmdinf->MHAL_CMDQ_KickOffCmdq(cmdinf);
                    if(s32CmdqErr < 0)
                    {
                        DBG_ERR("Kick off error %d.\n", s32CmdqErr);
                    }
                    pstChnTask->u32Reserved0 = fence++;
                }
                else
                {
                    DBG_WRN("WriteDummyRegCmdq:%X\n", s32CmdqErr);
                }
            }

            UNLOCK_CHNN(pstChnRes);
        }
#if TRACE_TASK_GROUP
        if(pstDevRes->u32DevId == 0)
        {
            seq[iseq++] = '\0';
            if(d0==1)
            {
                DBG_TMP("Enc run: %s\n", seq);
            }
        }
#endif
    }
    DBG_EXIT_OK();
    MT_WRN("End thread\n");
    return 0;
}
MI_VENC_CreateThreadWrapper(VencWorkThread)


static MI_S32 _MI_VENC_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnUnbindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnUnbindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort, void *pUsrData)
{
    return MI_SUCCESS;
}

#ifdef MI_SYS_PROC_FS_DEBUG
static MI_S32 _MI_VENC_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_U32 u32MinDevId = E_MI_VENC_DEV_MHE0;
    MI_U32 u32MaxDevId = E_MI_VENC_DEV_MAX - 1;
    MI_U32 i;
    MI_BOOL bUsed[E_MI_VENC_DEV_MAX];//which device has any created channel.
    MI_U32 u32ChnNum;
    MI_VENC_ChnRes_t *pstChnRes;

    for (i = u32MinDevId; i <= u32MaxDevId; ++i)
    {
        bUsed[i] = FALSE;
    }
    if(_stDbgFsGlobal.s32ShowAllDev)
    {
        for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
        {
            //skip empty channel or channel used by other device.
            pstChnRes = _ModRes.astChnRes + u32ChnNum;
            if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
            {
                //assume that this channel is not created
                continue;
            }
            bUsed[pstChnRes->pstDevRes->u32DevId] = TRUE;
        }
        handle.OnPrintOut(handle, "\n------------------------------ All VENC Dev info ------------------------------", u32DevId);
    }
    if(!_stDbgFsGlobal.s32ShowAllDev)
    {
        //display requested device only
        u32MinDevId = u32MaxDevId = u32DevId;
    }

    handle.OnPrintOut(handle, "\n------------------------------- VENC%2d Dev info -------------------------------\n", u32DevId);
    handle.OnPrintOut(handle, "%7s%8s%10s%9s%8s%9s\n",
                              "DevId",
                                  "CmdqId",
                                     "CmdqSize",
                                         "CmdqCnt",
                                            "IrqNum",
                                               "IsrProc");
    for (i = u32MinDevId; i <= u32MaxDevId; ++i)
    {
        if(bUsed[i] == FALSE)
            continue;
        pstDevRes = _ModRes.devs + i;
        handle.OnPrintOut(handle, "%7d%8d%10d%9d%8d%9d\n",
                          pstDevRes->eMiDevType,
                          ((_stDbgFsInfo[i].s32EnCmdq == 0) || (pstDevRes->eCmdqId == E_MHAL_CMDQ_ID_NONE)) ?
                          -1 : pstDevRes->eCmdqId,
                          pstDevRes->u32CmdqSize,
                          pstDevRes->u32CmdqCnt,
                          pstDevRes->u32IrqNum,
                          pstDevRes->bFromIsr);
    }


    handle.OnPrintOut(handle, "%7s%8s%8s%8s%8s%8s%8s%4s\n",
                              "DevId",
                                  "UtilHw",
                                     "UtilMi",
                                        "PeakHw",
                                           "PeakMi",
                                              "FPS",
                                                 "MbRate",
                                                    " \%");
    for (i = u32MinDevId; i <= u32MaxDevId; ++i)
    {
        if(bUsed[i] == FALSE)
            continue;
        pstDevRes = _ModRes.devs + i;
        handle.OnPrintOut(handle, "%7d%8d%8d%8d%8d%5d.%02d%8d%4d\n",
                          pstDevRes->eMiDevType,
                          pstDevRes->_stStat.au16Util[0],
                          pstDevRes->_stStat.au16Util[1],
                          pstDevRes->_stStat.au16PeakUtil[0],//pstDevRes->_stStat.au16Util[2],
                          pstDevRes->_stStat.au16PeakUtil[1],
                          pstDevRes->_stStat.u16Fps,
                          pstDevRes->_stStat.u16FpsFrac,
                          pstDevRes->u32MbRate,
                          pstDevRes->u32MbRate * 100 / astMHalDevConfig[i].u32MbRate);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnDumpChannelAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_U32 u32ChnNum = 0;
    MI_BOOL bPrintHeader = TRUE;

    pstDevRes = _ModRes.devs + u32DevId;

    handle.OnPrintOut(handle, "\n------------------------------- VENC%2d CHN info -------------------------------\n", u32DevId);

    for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
    {
        //skip empty channel or channel used by other device.
        pstChnRes = _ModRes.astChnRes + u32ChnNum;

        if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
        {
            //assume that this channel is not created
            continue;
        }

        if((!_stDbgFsGlobal.s32ShowAllDev) && (pstChnRes->pstDevRes != pstDevRes))
        {
            //Skip this channel it does not point to input device.
            continue;
        }

        if(bPrintHeader)
        {
            handle.OnPrintOut(handle,
                              "%5s%10s%10s%15s %9s %9s%14s\n",
                              "ChnId",
                                  "RefMemPA",
                                      "RefMemVA",
                                          "RefMemBufSize",
                                               "AlMemPA",
                                                   "AlMemVA",
                                                      "AlMemBufSize");
        }

        handle.OnPrintOut(handle, "%5d%10llX%10p%15d %9llX %9p%14d\n",
                          u32ChnNum,
                          pstChnRes->stRefMemInfo.phyAddr,
                          pstChnRes->stRefMemInfo.pVirAddr,
                          pstChnRes->stRefMemInfo.u32BufSize,
                          pstChnRes->stAlMemInfo.phyAddr,
                          pstChnRes->stAlMemInfo.pVirAddr,
                          pstChnRes->stAlMemInfo.u32BufSize);
        bPrintHeader = FALSE;
    }

    bPrintHeader = TRUE;
    for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
    {
        //skip empty channel or channel used by other device.
        pstChnRes = _ModRes.astChnRes + u32ChnNum;

        if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
        {
            //assume that this channel is not created
            continue;
        }

        if((!_stDbgFsGlobal.s32ShowAllDev) && (pstChnRes->pstDevRes != pstDevRes))
        {
            //Skip this channel it does not point to input device.
            continue;
        }

        if(bPrintHeader)
        {
            handle.OnPrintOut(handle,
                              "%5s%7s%8s%11s%10s%8s%9s\n",
                              "ChnId",
                                  "DevId",
                                     "bStart",
                                        "eBufState",
                                            "FrameCnt",
                                                "FPS",
                                                   "kbps");
            bPrintHeader = FALSE;
        }

        handle.OnPrintOut(handle, "%5d%7d%8d%11d%10lld%5d.%02d%9d\n",
                          u32ChnNum,
                          pstChnRes->pstDevRes->eMiDevType,
                          pstChnRes->bStart,
                          pstChnRes->eBufState,
                          pstChnRes->u64SeqNum,
                          pstChnRes->_stStat.u16Fps,
                          pstChnRes->_stStat.u16FpsFrac,
                          pstChnRes->_stStat.u32Kbps);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_BOOL bPrintHeader = TRUE;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_VENC_Attr_t *pstVeAttr;
    MI_VENC_RcAttr_t *pstRcAttr;
    char *szType;
    MI_U32 u32MaxPicWidth = 0;
    MI_U32 u32MaxPicHeight = 0;
    MI_U32 u32PicWidth = 0;
    MI_U32 u32PicHeight = 0;
    MI_U32 u32SrcFrmRateNum = 0;
    MI_U32 u32SrcFrmRateDen = 0;

    MI_U32 u32ChnNum = 0;

    pstDevRes = _ModRes.devs + u32DevId;

    handle.OnPrintOut(handle, "----------------------------- InputPort of dev:%2d -----------------------------\n", u32DevId);

    for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
    {
        if (!_IS_VALID_VENC_CHANNEL(u32ChnNum))
        {
            DBG_ERR("Invalid channel %d\n", u32ChnNum);
            return MI_ERR_VENC_INVALID_CHNID;
        }
        pstChnRes = _ModRes.astChnRes + u32ChnNum;

        pstDevRes = _ModRes.devs + u32DevId;
        if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
        {
            //return MI_SUCCESS;
            continue;
        }
        if((!_stDbgFsGlobal.s32ShowAllDev) && (pstChnRes->pstDevRes != pstDevRes))
        {
            //Skip this channel it does not point to input device.
            continue;
        }

        pstVeAttr = &pstChnRes->stChnAttr.stVeAttr;

        switch(pstVeAttr->eType)
        {
            case E_MI_VENC_MODTYPE_H264E:
                szType = "H264";
                u32PicWidth     = pstVeAttr->stAttrH264e.u32PicWidth;
                u32PicHeight    = pstVeAttr->stAttrH264e.u32PicHeight;
                u32MaxPicWidth  = pstVeAttr->stAttrH264e.u32MaxPicWidth;
                u32MaxPicHeight = pstVeAttr->stAttrH264e.u32MaxPicHeight;
                break;
            case E_MI_VENC_MODTYPE_H265E:
                szType = "H265";
                u32PicWidth     = pstVeAttr->stAttrH265e.u32PicWidth;
                u32PicHeight    = pstVeAttr->stAttrH265e.u32PicHeight;
                u32MaxPicWidth  = pstVeAttr->stAttrH265e.u32MaxPicWidth;
                u32MaxPicHeight = pstVeAttr->stAttrH265e.u32MaxPicHeight;
                break;
            case E_MI_VENC_MODTYPE_JPEGE:
                szType = "JPGE";
                u32PicWidth     = pstVeAttr->stAttrJpeg.u32PicWidth;
                u32PicHeight    = pstVeAttr->stAttrJpeg.u32PicHeight;
                u32MaxPicWidth  = pstVeAttr->stAttrJpeg.u32MaxPicWidth;
                u32MaxPicHeight = pstVeAttr->stAttrJpeg.u32MaxPicHeight;
                break;
            default:
                szType = "????";
                break;
        }

        pstRcAttr = &pstChnRes->stChnAttr.stRcAttr;
        switch(pstRcAttr->eRcMode)
        {
            case E_MI_VENC_RC_MODE_H264FIXQP:
                u32SrcFrmRateNum = pstRcAttr->stAttrH264FixQp.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH264FixQp.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H264CBR:
                u32SrcFrmRateNum = pstRcAttr->stAttrH264Cbr.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH264Cbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H264VBR:
                u32SrcFrmRateNum = pstRcAttr->stAttrH264Vbr.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH264Vbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H265CBR:
                u32SrcFrmRateNum = pstRcAttr->stAttrH265Cbr.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH265Cbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H265VBR:
                u32SrcFrmRateNum = pstRcAttr->stAttrH265Vbr.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH265Vbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H265FIXQP:
                u32SrcFrmRateNum = pstRcAttr->stAttrH265FixQp.u32SrcFrmRateNum;
                u32SrcFrmRateDen = pstRcAttr->stAttrH265FixQp.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_MJPEGFIXQP:
                u32SrcFrmRateNum = (MI_U32)-1;
                u32SrcFrmRateDen = (MI_U32)-1;
                break;
            default:
                u32SrcFrmRateNum = (MI_U32)-2;
                break;
        }

        if(bPrintHeader)
        {
            handle.OnPrintOut(handle,
                              "%5s%7s%8s%12s%6s%6s%10s%9s%10s\n",
                              "ChnId",
                                  "Width",
                                     "Height",
                                        "SrcFrmRate",
                                            "MaxW",
                                               "MaxH",
                                                  "FrameCnt",
                                                      "DropCnt",
                                                         "BlockCnt");
            bPrintHeader = FALSE;
        }
        handle.OnPrintOut(handle, "%5d%7d%8d%6d/%-5d%6d%6d%10d%9d%10d\n", u32ChnNum, u32PicWidth, u32PicHeight, u32SrcFrmRateNum, u32SrcFrmRateDen,
                          u32MaxPicWidth, u32MaxPicHeight, pstChnRes->u32InFrameCnt, pstChnRes->u32InDropCnt, pstChnRes->u32InBlockCnt);
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    MI_BOOL bPrintHeader = TRUE;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_U32 u32ChnNum = 0;
    MI_U32 u32Grp, u32GrpIdx;
    char *szType;

    pstDevRes = _ModRes.devs + u32DevId;
    handle.OnPrintOut(handle, "---------------------------- OutputPort of dev:%2d -----------------------------\n", u32DevId);

    for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
    {
        MI_VENC_Attr_t *pstVeAttr;
        MI_U32 u32BufSize = 0;
        MI_U32 u32Profile = 0;
        MI_BOOL bByFrame = 0;
        MI_U32 u32RefNum = 0;

        if (!_IS_VALID_VENC_CHANNEL(u32ChnNum))
        {
            DBG_ERR("Invalid channel %d\n", u32ChnNum);
            return MI_ERR_VENC_INVALID_CHNID;
        }

        pstChnRes = _ModRes.astChnRes + u32ChnNum;
        pstDevRes = _ModRes.devs + u32DevId;
        if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
        {
            continue;
        }
        if((!_stDbgFsGlobal.s32ShowAllDev) && (pstChnRes->pstDevRes != pstDevRes))
        {
            //Skip this channel it does not point to input device.
            continue;
        }

        pstVeAttr = &pstChnRes->stChnAttr.stVeAttr;

        switch(pstVeAttr->eType)
        {
            case E_MI_VENC_MODTYPE_H264E:
                szType = "H264";
                u32Profile      = pstVeAttr->stAttrH264e.u32Profile;
                u32RefNum       = pstVeAttr->stAttrH264e.u32RefNum;
                bByFrame        = pstVeAttr->stAttrH264e.bByFrame = 1;//supports only by Frame
                //u32BufSize      = pstVeAttr->stAttrH264e.u32BufSize;
                //ignore u32BFrameNum
                break;
            case E_MI_VENC_MODTYPE_H265E:
                szType = "H265";
                u32Profile      = pstVeAttr->stAttrH265e.u32Profile;
                u32RefNum       = pstVeAttr->stAttrH265e.u32RefNum;
                bByFrame        = pstVeAttr->stAttrH265e.bByFrame = 1;//supports only by Frame
                //u32BufSize      = pstVeAttr->stAttrH265e.u32BufSize;
                break;
            case E_MI_VENC_MODTYPE_JPEGE:
                szType = "JPGE";
                bByFrame        = pstVeAttr->stAttrJpeg.bByFrame = 1;//supports only by Frame
                //u32BufSize      = pstVeAttr->stAttrMjpeg.u32BufSize;
                break;
            default:
                szType = "????";
                break;
        }
        u32BufSize = pstChnRes->u32BufSize;

        if(bPrintHeader)
        {
            handle.OnPrintOut(handle,
                              "%5s%7s%9s %9s%8s%10s%10s%9s%10s\n",
                              "ChnId",
                                  "CODEC",
                                     "Profile",
                                         "BufSize",
                                            "RefNum",
                                               "bByFrame",
                                                   "FrameCnt",
                                                       "DropCnt",
                                                          "ReEncCnt");
            bPrintHeader = FALSE;
        }
        handle.OnPrintOut(handle, "%5d%7s%9d %9d%8d%10d%10d%9d%10d\n", u32ChnNum, szType, u32Profile, u32BufSize,
                          u32RefNum, bByFrame, pstChnRes->u32OutFrameCnt, pstChnRes->u32OutDropCnt, pstChnRes->u32ReEncCnt);
    }

    // 2nd section: Rate control
    for (u32Grp = 0; u32Grp < 4; ++u32Grp)
    {

        bPrintHeader = TRUE;
        for (u32ChnNum = 0; u32ChnNum < VENC_MAX_CHN_NUM; u32ChnNum++)
        {
            MI_VENC_RcAttr_t *pstRcAttr;
            char *szRc;
            MI_U32 u32Gop = 0;
            MI_U32 u32StatTime = 0;
            MI_U32 u32MaxBitRate = 0;
            MI_U32 u32MaxQp = 0;
            MI_U32 u32MinQp = 0;
            MI_U32 u32IQp = 0;
            MI_U32 u32PQp = 0;
            MI_U32 u32FlucLevel = 0;

            pstChnRes = _ModRes.astChnRes + u32ChnNum;
            pstDevRes = _ModRes.devs + u32DevId;
            if(pstChnRes->bCreate == FALSE || pstChnRes->pstDevRes == NULL)
            {
                continue;
            }
            if((!_stDbgFsGlobal.s32ShowAllDev) && (pstChnRes->pstDevRes != pstDevRes))
            {
                //Skip this channel it does not point to input device.
                continue;
            }

            pstRcAttr = &pstChnRes->stChnAttr.stRcAttr;
            switch(pstRcAttr->eRcMode)
            {
                case E_MI_VENC_RC_MODE_H264FIXQP:
                case E_MI_VENC_RC_MODE_H265FIXQP:
                    u32GrpIdx = 0;
                    break;
                case E_MI_VENC_RC_MODE_H264CBR:
                case E_MI_VENC_RC_MODE_H265CBR:
                    u32GrpIdx = 1;
                    break;
                case E_MI_VENC_RC_MODE_H264VBR:
                case E_MI_VENC_RC_MODE_H265VBR:
                    u32GrpIdx = 2;
                    break;
                case E_MI_VENC_RC_MODE_MJPEGFIXQP:
                    u32GrpIdx = 3;
                    break;
                case E_MI_VENC_RC_MODE_H264ABR:
                default:
                    u32GrpIdx = 100;
                    break;
            }
            if(u32GrpIdx != u32Grp)
                continue;
            //bPrintHeader = TRUE;
            switch(pstRcAttr->eRcMode)
            {
                case E_MI_VENC_RC_MODE_H264CBR:
                case E_MI_VENC_RC_MODE_H264VBR:
                case E_MI_VENC_RC_MODE_H264FIXQP:
                    szType = "H264";
                    break;
                case E_MI_VENC_RC_MODE_H265CBR:
                case E_MI_VENC_RC_MODE_H265VBR:
                case E_MI_VENC_RC_MODE_H265FIXQP:
                    szType = "H265";
                    break;
                case E_MI_VENC_RC_MODE_MJPEGFIXQP:
                    szType = "MJPG";
                    break;
                default:
                    szType = "????";
                    break;
            }

            switch(pstRcAttr->eRcMode)
            {
                case E_MI_VENC_RC_MODE_H264CBR:
                    u32Gop        = pstRcAttr->stAttrH264Cbr.u32Gop;
                    u32StatTime   = pstRcAttr->stAttrH264Cbr.u32StatTime;
                    u32MaxBitRate = pstRcAttr->stAttrH264Cbr.u32BitRate;
                    u32FlucLevel  = pstRcAttr->stAttrH264Cbr.u32FluctuateLevel;
                    break;
                case E_MI_VENC_RC_MODE_H264VBR:
                    u32Gop        = pstRcAttr->stAttrH264Vbr.u32Gop;
                    u32StatTime   = pstRcAttr->stAttrH264Vbr.u32StatTime;
                    u32MaxBitRate = pstRcAttr->stAttrH264Vbr.u32MaxBitRate;
                    u32MaxQp      = pstRcAttr->stAttrH264Vbr.u32MaxQp;
                    u32MinQp      = pstRcAttr->stAttrH264Vbr.u32MinQp;
                    break;
                case E_MI_VENC_RC_MODE_H264FIXQP:
                    u32Gop        = pstRcAttr->stAttrH264FixQp.u32Gop;
                    u32IQp        = pstRcAttr->stAttrH264FixQp.u32IQp;
                    u32PQp        = pstRcAttr->stAttrH264FixQp.u32PQp;
                    break;
                case E_MI_VENC_RC_MODE_H265CBR:
                    u32Gop        = pstRcAttr->stAttrH265Cbr.u32Gop;
                    u32StatTime   = pstRcAttr->stAttrH265Cbr.u32StatTime;
                    u32MaxBitRate = pstRcAttr->stAttrH265Cbr.u32BitRate;
                    u32FlucLevel  = pstRcAttr->stAttrH265Cbr.u32FluctuateLevel;
                    break;
                case E_MI_VENC_RC_MODE_H265VBR:
                    u32Gop        = pstRcAttr->stAttrH265Vbr.u32Gop;
                    u32StatTime   = pstRcAttr->stAttrH265Vbr.u32StatTime;
                    u32MaxBitRate = pstRcAttr->stAttrH265Vbr.u32MaxBitRate;
                    u32MaxQp      = pstRcAttr->stAttrH265Vbr.u32MaxQp;
                    u32MinQp      = pstRcAttr->stAttrH265Vbr.u32MinQp;
                    break;
                case E_MI_VENC_RC_MODE_H265FIXQP:
                    u32Gop        = pstRcAttr->stAttrH265FixQp.u32Gop;
                    u32IQp        = pstRcAttr->stAttrH265FixQp.u32IQp;
                    u32PQp        = pstRcAttr->stAttrH265FixQp.u32PQp;
                    break;
                case E_MI_VENC_RC_MODE_MJPEGFIXQP:
                    break;
                default:
                    break;
            }

            switch(pstRcAttr->eRcMode)
            {
                case E_MI_VENC_RC_MODE_H264CBR:
                case E_MI_VENC_RC_MODE_H265CBR:
                    szRc = "CBR";
                    if(bPrintHeader)
                    {
                        handle.OnPrintOut(handle, "%5s%8s%5s%11s%10s%16s\n",
                                          "ChnId", "RateCtl", "GOP", "MaxBitrate", "StatTime",
                                          "FluctuateLevel");
                    }
                    handle.OnPrintOut(handle, "%5d%8s%5d%11d%10d%16d\n",
                                      u32ChnNum, szRc, u32Gop,
                                      u32MaxBitRate, u32StatTime, u32FlucLevel);
                    break;
                case E_MI_VENC_RC_MODE_H264VBR:
                case E_MI_VENC_RC_MODE_H265VBR:
                    szRc = "VBR";
                    u32MaxQp      = pstRcAttr->stAttrH264Vbr.u32MaxQp;
                    u32MinQp      = pstRcAttr->stAttrH264Vbr.u32MinQp;
                    if(bPrintHeader)
                    {
                        handle.OnPrintOut(handle, "%5s%8s%5s%11s%10s%7s%7s\n",
                                          "ChnId", "RateCtl", "GOP",
                                          "MaxBitrate", "StatTime", "MaxQp", "MinQp");
                    }
                    handle.OnPrintOut(handle, "%5d%8s%5d%11d%10d%7d%7d\n",
                                      u32ChnNum, szRc, u32Gop,
                                      u32MaxBitRate, u32StatTime, u32MaxQp, u32MinQp);
                    break;
                case E_MI_VENC_RC_MODE_H264FIXQP:
                case E_MI_VENC_RC_MODE_H265FIXQP:
                    szRc   = "FixQP";

                    if(bPrintHeader)
                    {
                        handle.OnPrintOut(handle,
                                          "%5s%8s%5s%6s%6s\n",
                                          "ChnId", "RateCtl", "GOP", "QP.I",
                                          "QP.P");
                    }
                    handle.OnPrintOut(handle, "%5d%8s%5d%6d%6d\n",
                                      u32ChnNum, szRc, u32Gop,
                                      u32IQp, u32PQp);

                    break;
                case E_MI_VENC_RC_MODE_MJPEGFIXQP:
                    szRc   = "FixQP";
                    {
                        MHAL_VENC_RcInfo_t stRc;
                        MI_S32 s32Err;
                        MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stRc);
                        s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
                        if(s32Err != MI_SUCCESS)
                        {
                            DBG_ERR("Get Param err 0x%X.\n", s32Err);
                            return MI_ERR_VENC_UNDEFINED;
                        }

                        if(bPrintHeader)
                        {
                            handle.OnPrintOut(handle, "%5s%9s%9s\n",
                                              "ChnId", "RateCtl", "Qfactor");
                        }
                        handle.OnPrintOut(handle, "%5d%9s%9d\n",
                                          u32ChnNum, szRc, (MI_S16)stRc.stAttrMJPGRc.u32Qfactor);
                    }

                    break;
                default:
                    //szRc   = "????";
                    if(bPrintHeader)
                    {
                        handle.OnPrintOut(handle, "%5s%7s%12s\n",
                                          "ChnId", "CODEC", "BufSize");
                    }
                    handle.OnPrintOut(handle, "%5d%7s%12d\n",
                                      u32ChnNum, szType, -1);
                    break;
            }
            bPrintHeader = FALSE;
        }
    }
    return MI_SUCCESS;
}

static MI_BOOL _MI_VENC_IsHelp(MI_U8 *szArg1)
{
    if(szArg1 == NULL)
        return FALSE;

    if(strcmp("-h", szArg1) == 0 || strcmp("help", szArg1) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

static MI_S32 _MI_VENC_ProcDbgfsInt(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv,
                                    MI_VENC_DbgFsCmd_t * const pstDbgFsCmd, MI_S32 *ps32Value, MI_S32 s32Min, MI_S32 s32Max, char *szHelp)
{
    MI_S32 s32;
    int cnt;
    MI_U8 u8Argc;

    if(ps32Value == NULL)
        return MI_ERR_VENC_NULL_PTR;

    u8Argc = pstDbgFsCmd? pstDbgFsCmd->u8MaxArgc: 100;
    if(argc <= u8Argc || _MI_VENC_IsHelp(argv[1]))
    {
        handle.OnPrintOut(handle, "\n%s %s",
                          pstDbgFsCmd ? pstDbgFsCmd->szName : "", szHelp);
        return MI_SUCCESS;
    }

    //print (get) the current value back.
    if (strcmp("-g", argv[1]) == 0 || strcmp("get", argv[1]) == 0)
    {
        handle.OnPrintOut(handle, "dev:%d %s=%d\n", u32DevId, pstDbgFsCmd->szName, *ps32Value);
        return MI_SUCCESS;
    }

    cnt = sscanf((char*)argv[1], "%d", &s32);
    if(cnt == 1)
    {
        if(s32 < s32Min)
            s32 = s32Min;
        if(s32 > s32Max)
            s32 = s32Max;
        *ps32Value = s32;
        handle.OnPrintOut(handle, "dev:%d set %s to %d\n", u32DevId, pstDbgFsCmd->szName, s32);
    }
    else
    {
        handle.OnPrintOut(handle, "[ERROR] dev:%d argc:%d, argument error\n", u32DevId, argc);
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_ProcDbgLevel(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;
    MI_S32 s32Temp = (MI_S32)_stDbgFsInfo[u32DevId].eDbgLevel;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcDbgLevel, &s32Temp, MI_DBG_NONE, MI_DBG_ALL,
                                   " [0-3]\n"
                                   "\t  0:None\n"
                                   "\t  1:Error\n"
                                   "\t  2:Level 1 + warnings\n"
                                   "\t  3:Level 2 + information\n");

    if(s32Ret == MI_SUCCESS)
        _stDbgFsInfo[u32DevId].eDbgLevel = (MI_DBG_LEVEL_e)s32Temp;

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcEnIrq(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcEnIrq, &_stDbgFsInfo[u32DevId].s32EnIrq, -1, 1,
                                   " [-1, 1]\n"
                                   "\t-1:Not set. Use module default value\n"
                                   "\t 0:Disable IRQ.\n"
                                   "\t 1:Enable IRQ.\n");

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcEnCmdq(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcEnCmdq, &_stDbgFsInfo[u32DevId].s32EnCmdq, -1, 1,
                                   " [-1, 1]\n"
                                   "\t-1:Not set. Use module default value\n"
                                   "\t 0:Disable CmdQ.\n"
                                   "\t 1:Enable CmdQ.\n");

    if (1 == _stDbgFsInfo[u32DevId].s32EnCmdq &&
            E_MHAL_CMDQ_ID_NONE == astMHalDevConfig[u32DevId].eCmdqId)
    {
        handle.OnPrintOut(handle, "But this device does not support CMDQ.\n", u32DevId);
        _stDbgFsInfo[u32DevId].s32EnCmdq = 0;
        return MI_SUCCESS;
    }

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcDumpIn(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcDumpIn, &_stDbgFsInfo[u32DevId].s32DumpIn, 0, 10,
                                   " [0-10]\n"
                                   "\t 0:None\n"
                                   "\t 1-10:dump input buffer for first few frames.\n");
    if(_stDbgFsInfo[u32DevId].s32DumpIn > 0)
    {
        DBG_WRN("dump %d in frames\n", _stDbgFsInfo[u32DevId].s32DumpIn);
    }

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcDumpInBig(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcDumpInBig, &_stDbgFsInfo[u32DevId].s32DumpBigIn, 0, 1,
                                   " [0-1]\n"
                                   "\t 0:None\n"
                                   "\t 1:Dump first N frames while big output bitstream in detected\n");

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcShowAll(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_S32 s32Ret;

    s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv,
                                   //&Config, &variable, MIN value, MAX value
                                   pstProcShowAll, &_stDbgFsGlobal.s32ShowAllDev, 0, 1,
                                   " [0-1]\n"
                                   "\t 0:Show only requested device info.\n"
                                   "\t 1:Show all used device info. (with created channels)\n");

    return s32Ret;
}

static MI_S32 _MI_VENC_ProcDmsg(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MI_VENC_DbgFsCmd_t * const pstDbgFsCmd = pstProcDmsg;
    MI_U8 u8Argc;
    MI_S32 s32Ret = MI_SUCCESS;

    u8Argc = pstDbgFsCmd? pstDbgFsCmd->u8MaxArgc: 1;
#if 0//print out every argument
    {
        int i;
        handle.OnPrintOut(handle, "u8Argc:%d \n", u8Argc);
        for (i = 1; i <= u8Argc; ++i)
        {
            handle.OnPrintOut(handle, "arv[%d]='%s'\n", i, (char*)argv[i]);
        }
    }
#endif
    if(argc <= u8Argc || _MI_VENC_IsHelp(argv[1]))
    {
        handle.OnPrintOut(handle, "\ndmsg [0~1]\n"
                          "\t  0:None\n");
        return MI_SUCCESS;
    }
    if(strcmp("cmdq", argv[1]) == 0)
    {
        s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv + 1,
                                       //&Config, &variable, MIN value, MAX value
                                       pstDbgFsCmd, &_stDbgFsInfo[u32DevId].s32Cmdq, 0, 1,
                                       " [0-1]\n"
                                       "\t 0:...\n"
                                       "\t 1:...\n");
        if(s32Ret == MI_SUCCESS)
        {
            if(_stDbgFsInfo[u32DevId].s32Cmdq > 0 && astMHalDevConfig[u32DevId].eCmdqId == E_MHAL_CMDQ_ID_NONE)
            {
                DBG_ERR("Device[%d] does not support CMDQ\n", u32DevId);
                _stDbgFsInfo[u32DevId].s32Cmdq = 0;
            }
        }
        DBG_TMP("s32Cmdq = %d\n", _stDbgFsInfo[u32DevId].s32Cmdq);
    }
    else if(strcmp("irq", argv[1]) == 0)
    {
        s32Ret = _MI_VENC_ProcDbgfsInt(handle, u32DevId, argc, argv + 1,
                                       //&Config, &variable, MIN value, MAX value
                                       pstDbgFsCmd, &_stDbgFsInfo[u32DevId].s32Isr, 0, 1,
                                       " [0-1]\n"
                                       "\t 0:...\n"
                                       "\t 1:...\n");
        DBG_TMP("s32Isr = %d\n", _stDbgFsInfo[u32DevId].s32Isr);
    }

#if 0
    sscanf(argv[1], "%d", &i);
    _stDbgFsInfo[u32DevId].s32DumpBigIn = (MI_BOOL)i;
    _stDbgFsInfo[u32DevId].s32DumpBigIn = (_stDbgFsInfo[u32DevId].s32DumpBigIn != 0);
    handle.OnPrintOut(handle, "dev:%d argc:%d, Now set dump_in_big to %d\n", u32DevId, argc,
                      _stDbgFsInfo[u32DevId].s32DumpBigIn);
#endif
    return s32Ret;
}

#if 0
static MI_S32 _MI_VENC_ProcUnimplemented(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv,
        void *pUsrData)
{
    if(argc <= 1 || _MI_VENC_IsHelp(argv[1]))
    {
        handle.OnPrintOut(handle, "\nThis is help of unimplemented.\n");
        return MI_SUCCESS;
    }
    handle.OnPrintOut(handle, "This command is not implemented.\n");
    return MI_SUCCESS;
}
#endif

static MI_VENC_DbgFsCmd_t _astDbgFsCmds[] =
{
    {"dbg_level",   1, _MI_VENC_ProcDbgLevel,      &pstProcDbgLevel},
    {"en_irq",      1, _MI_VENC_ProcEnIrq,         &pstProcEnIrq},
    {"en_cmdq",     1, _MI_VENC_ProcEnCmdq,        &pstProcEnCmdq},
    {"dump_in",     1, _MI_VENC_ProcDumpIn,        &pstProcDumpIn},
    {"dump_in_big", 1, _MI_VENC_ProcDumpInBig,     &pstProcDumpInBig},
    {"show_all",    1, _MI_VENC_ProcShowAll,       &pstProcShowAll},
    {"dmsg",        2, _MI_VENC_ProcDmsg,          &pstProcDmsg},
};

static MI_S32 _MI_VENC_OnHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    handle.OnPrintOut(handle, "\n====================== MI_VENC Help =====================\n");
    {
        MI_U32 i;
        MI_U8 *argv[2] = {NULL, "-h"};
        for(i = 0; i < sizeof(_astDbgFsCmds) / sizeof(_astDbgFsCmds[0]); ++i)
        {
            if(_astDbgFsCmds[i].fpExecCmd)
            {
                //MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData);
                argv[0] = _astDbgFsCmds[i].szName;
                handle.OnPrintOut(handle, "\n"ASCII_COLOR_YELLOW"%s"ASCII_COLOR_END" usage: ", argv[0]);
                _astDbgFsCmds[i].fpExecCmd(handle, u32DevId, 2, argv, pUsrData);
            }
        }
    }

    return MI_SUCCESS;
}

#endif

#define UT_FENCE (0)
#if UT_FENCE
struct
{
    MI_U16 u16a;
    MI_U16 u16b;
    MI_BOOL bAns;
} astFenceCases[] =
{
    {0, 1, TRUE},
    {0, 0, FALSE},
    {1, 0, FALSE},
//    {0, 0x7FFF, TRUE},
    {0, 0x7FFF, FALSE},
    {0, 0x8000, FALSE},
    {0, 0x8001, FALSE},
    {0, 0xFFFF, FALSE},
    {1, 0xFFFF, FALSE},
    {0x7FFE, 0x7FFF, TRUE},
    {0x7FFF, 0x8000, TRUE},
    {0x8000, 0x8001, TRUE},
    {0xFFFE, 0xFFFF, TRUE},
    {0xFFFF, 0xFFFF, FALSE},
    {0xFFFF, 0x0000, TRUE},
};
#endif

static void _MI_VENC_IMPL_InitRes(void)
{
    MI_U32 i;
#if UT_FENCE
    MI_U16 u16Fail = 0;
    MI_BOOL bAns;
#endif
    memset(&_ModRes, 0x0, sizeof(_ModRes));
    for(i = 0; i < E_MI_VENC_DEV_MAX; ++i)
    {
        MI_VENC_InitCond(&_ModRes.devs[i].stIrqWaitQueueHead);
        MI_VENC_InitCond(&_ModRes.devs[i].stWorkWaitQueueHead);
        VENC_SEM_INIT(&_ModRes.devs[i].list_mutex, 1);
        raw_spin_lock_init(&_ModRes.devs[i].working_task_spinlock);
    }
    for(i = 0; i < VENC_MAX_CHN_NUM; ++i)
    {
        VENC_SEM_INIT(&_ModRes.astChnRes[i].semLock, 1);
    }

    DBG_INFO("Use CMDQ:%d\n", VENC_USE_CMDQ);

#if UT_FENCE
    DBG_TMP("test 1\n\n");
    for (u16Fail = 0, i = 0; i < sizeof(astFenceCases)/sizeof(astFenceCases[1]); ++i)
    {
        bAns = _MI_VENC_IsFenceL(astFenceCases[i].u16a, astFenceCases[i].u16b);
        if(bAns != astFenceCases[i].bAns)
        {
            u16Fail++;
            DBG_ERR("1(%X,%X) == %d but expected %d\n", astFenceCases[i].u16a, astFenceCases[i].u16b, bAns, astFenceCases[i].bAns);
        }
    }
    DBG_TMP("1 astFenceCases tested, %d failed\n", u16Fail);
#endif
}

static MI_S32 _MI_VENC_CreateDevice(MI_VENC_Dev_e eDevType)
{
    mi_sys_ModuleDevBindOps_t stVencOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    MI_VENC_DevRes_t *pstDevRes;
    MHAL_VENC_ParamInt_t stParamInt;
    MHAL_CMDQ_Id_e cmdqId;
    MI_S32 s32Ret = MI_ERR_VENC_NULL_PTR;
    char szTaskName[64];

    void* pBase;
    MHAL_VENC_DEV_HANDLE hDev = NULL;
    MHAL_ErrCode_e err;
    int size;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
#endif

    //DBG_ENTER();
#if CONNECT_DUMMY_HAL
    if (eDevType > E_MI_VENC_DEV_MAX) //with extra dummy device
#else
    if (eDevType >= E_MI_VENC_DEV_MAX)
#endif
    {
        return MI_ERR_VENC_INVALID_DEVID;
    }

    pstDevRes = _ModRes.devs + eDevType;
    if (pstDevRes->bInitFlag)
    {
        DBG_WRN("Already Initialized\n");
        return MI_SUCCESS;
    }
    pstDevRes->bInitFlag = TRUE;//This should be set here in order to destroy device when failed to create it.

    err = MHAL_VENC_InitDevice(eDevType);
    if (err != MHAL_SUCCESS)
    {
        DBG_ERR("Invalid DEVID %d. Unable to Init Device :%X\n", eDevType, err);
        return MI_ERR_VENC_INVALID_DEVID;
    }
    err = MHAL_VENC_CreateDevice(pstDevRes, eDevType, NULL, &pBase, &size, &hDev);
    if (err != MHAL_SUCCESS)
    {
        DBG_ERR("Invalid DEVID %d. Unable to Create Device :%X\n", eDevType, err);
        return MI_ERR_VENC_INVALID_DEVID;
    }

    pstDevRes->hHalDev = hDev;

    // Register kernel IRQ and Enable IRQ
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamInt_t, stParamInt);
    err = MHAL_VENC_GetDevConfig(pstDevRes, E_MHAL_VENC_HW_IRQ_NUM, &stParamInt);
    if (err != MHAL_SUCCESS)
    {
        DBG_ERR("Unable to config IRQ :%X\n", err);
        return MI_ERR_VENC_NOT_CONFIG;
    }
    pstDevRes->u32IrqNum = stParamInt.u32Val;
    if (pstDevRes->u32IrqNum != MHAL_VENC_DUMMY_IRQ)
    {
#if SKIP_IRQ
        DBG_INFO("Skip request_irq %d for module %d test.\n", pstDevRes->u32IrqNum, eDevType);
        pstDevRes->u32IrqNum = MHAL_VENC_DUMMY_IRQ;
#else
        MI_S32 ret = 0;
        ret = MI_VENC_RequestIrq(pstDevRes->u32IrqNum, astMHalDevConfig[eDevType].IRQ, "VENC-ISR", pstDevRes);
        //ret = request_irq(pstDevRes->u32IrqNum, CMDQ2_ISR, IRQF_SHARED, "VENC-ISR", pstDevRes);
        if (ret != 0)
        {
            DBG_ERR("irq:%d ret %d\n", pstDevRes->u32IrqNum, ret);
            return MI_ERR_VENC_NOT_CONFIG;
        }
#endif
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ResolutionAlign_t, pstDevRes->stAlign);
    err = MHAL_VENC_GetDevConfig(pstDevRes, E_MHAL_VENC_HW_RESOLUTION_ALIGN, &pstDevRes->stAlign);
    if (err != MHAL_SUCCESS)
    {
        DBG_ERR("Unable to config alignment.\n");
        return MI_ERR_VENC_NOT_CONFIG;
    }

    //command queue per module per frame
    err = MHAL_VENC_GetDevConfig(pstDevRes, E_MHAL_VENC_HW_CMDQ_BUF_LEN, (MHAL_VENC_Param_t*)&stParamInt);
    if (err != MHAL_SUCCESS)
    {
        DBG_ERR("Unable to config CMDQ buffer.\n");
        return MI_ERR_VENC_NOT_CONFIG;
    }

    pstDevRes->u32CmdqSize = stParamInt.u32Val;
    cmdqId = astMHalDevConfig[eDevType].eCmdqId;
    pstDevRes->eCmdqId = cmdqId;
    if (E_MHAL_CMDQ_ID_NONE == cmdqId)
    {
        pstDevRes->cmdq = NULL;
        VENC_SEM_INIT(&pstDevRes->frame_sem, 1);
    }
    else
    {
#if VENC_USE_CMDQ
        MHAL_CMDQ_BufDescript_t stReqCmdqBuf;//Required command queue buffer
        MS_U32 u32CmdqAvailCnt;
        MS_U32 u32MinCmdqBufSize;

        stReqCmdqBuf.u32CmdqBufSize = pstDevRes->u32CmdqSize;
        stReqCmdqBuf.u32CmdqBufSizeAlign = 16;
        stReqCmdqBuf.u32MloadBufSize = 0;
        stReqCmdqBuf.u16MloadBufSizeAlign = 0;
        pstDevRes->cmdq = MHAL_CMDQ_GetSysCmdqService(cmdqId, &stReqCmdqBuf, FALSE);
        if (pstDevRes->cmdq == NULL)
        {
            DBG_ERR("Unable to get CMDQ ID:%d\n", cmdqId);
            s32Ret = MI_ERR_VENC_NOT_CONFIG;
            pstDevRes->cmdq = (MHAL_CMDQ_CmdqInterface_t *)1;//to release cmdq service even if cmdq == NULL
            goto __get_cmdq_service_fail;
        }
        VENC_SEM_INIT(&pstDevRes->frame_sem, VENC_MAX_CHN_NUM);

        //Get requested CMDQ count from MHAL
        u32MinCmdqBufSize = stReqCmdqBuf.u32CmdqBufSize / MHAL_BYTES_PER_CMDQ / MHAL_VENC_INST_PER_DEV * 3;//3 frames
        u32CmdqAvailCnt = pstDevRes->cmdq->MHAL_CMDQ_CheckBufAvailable(pstDevRes->cmdq, 20);
        pstDevRes->u32CmdqCnt = u32CmdqAvailCnt;

        if(u32MinCmdqBufSize > u32CmdqAvailCnt)
        {
            DBG_ERR("Minimal required buf %d but available :%d counts\n", u32MinCmdqBufSize, u32CmdqAvailCnt);
            s32Ret = MI_ERR_VENC_NOBUF;
            goto __get_cmdq_avail_buf_fail;
        }
#else
        DBG_INFO("Skip CMDQ %d for module %d testing.\n", cmdqId, eDevType);
        pstDevRes->cmdq = NULL;
        VENC_SEM_INIT(&pstDevRes->frame_sem, 1);
#endif
    }

    memset(&stVencOps, 0x0, sizeof(stVencOps));
    memset(&stModInfo, 0x0, sizeof(stModInfo));
    stModInfo.u32DevId = eDevType;
    stModInfo.u32DevChnNum = VENC_MAX_CHN_NUM;
#if CONNECT_DUMMY_HAL
    if(eDevType == E_MI_VENC_DEV_DUMMY)
        stModInfo.u32DevChnNum = 2;//few channels for dummy is good enough. Save mi sys channel resource
#endif
    stModInfo.u32InputPortNum = 1;
    stModInfo.u32OutputPortNum = 1;
    stModInfo.eModuleId = E_MI_MODULE_ID_VENC;
    stVencOps.OnBindInputPort = _MI_VENC_OnBindInputPort;
    stVencOps.OnUnBindInputPort = _MI_VENC_OnUnbindInputPort;
    stVencOps.OnBindOutputPort = _MI_VENC_OnBindOutputPort;
    stVencOps.OnUnBindOutputPort = _MI_VENC_OnUnbindOutputPort;
    stVencOps.OnOutputPortBufRelease = NULL;
#ifdef MI_SYS_PROC_FS_DEBUG
    memset(&pstModuleProcfsOps, 0, sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_VENC_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = _MI_VENC_OnDumpChannelAttr;
    pstModuleProcfsOps.OnDumpInputPortAttr = _MI_VENC_OnDumpInputPortAttr;
    pstModuleProcfsOps.OnDumpOutPortAttr = _MI_VENC_OnDumpOutPortAttr;
    pstModuleProcfsOps.OnHelp = _MI_VENC_OnHelp;
#endif
    pstDevRes->hMiDev = mi_sys_RegisterDev(&stModInfo, &stVencOps, pstDevRes
#ifdef MI_SYS_PROC_FS_DEBUG
                                           , &pstModuleProcfsOps
                                           , MI_COMMON_GetSelfDir
#endif
                                          );
    if (pstDevRes->hMiDev == NULL)
    {
        s32Ret = MI_ERR_VENC_NOT_CONFIG;
        DBG_ERR("Fail to register dev[%d].", eDevType);
        goto __register_dev_fail;
    }

    MT_INFO("pstDevRes:%p, cmdq:%p \n", pstDevRes, pstDevRes->cmdq);
    MT_INFO("List %p %p\n", pstDevRes->todo_task_list, pstDevRes->working_task_list);
    MI_VENC_INIT_LIST_HEAD(&pstDevRes->todo_task_list);
    MI_VENC_INIT_LIST_HEAD(&pstDevRes->working_task_list);

    //MT_WRN("Work List empty? %d\n", list_empty_careful(pstDevRes->working_task_list));

    sprintf(szTaskName, "VencInTask%d", eDevType);
    if (MI_SUCCESS != MI_VENC_CreateThread(&pstDevRes->stTaskWork, szTaskName, VencWorkThread, pstDevRes))
    {
        DBG_ERR("Fail to create thread VPE/WorkThread.\n");
        goto __create_work_thread_fail;
    }
    sprintf(szTaskName, "VencOutTask%d", eDevType);
    if (MI_SUCCESS != MI_VENC_CreateThread(&pstDevRes->stTaskIrq, szTaskName, VENC_ISR_Proc_Thread, pstDevRes))
    {
        DBG_ERR("Fail to create thread VENC/IsrProcThread.\n");
        goto __create_proc_thread_fail;
    }
    pstDevRes->u32DevId = (MI_U32)eDevType;

    DBG_INFO("VENC DEV[%d] Thread Created.\n", eDevType);

    pstDevRes->bWorkTaskRun = TRUE;
    MI_VENC_ThreadWakeUp(pstDevRes->stTaskWork);
    pstDevRes->bIrqTaskRun = TRUE;
    MI_VENC_ThreadWakeUp(pstDevRes->stTaskIrq);

    mi_venc_InitSw(&pstDevRes->stFps, 30, 0, _MI_VENC_ReportDevFps, pstDevRes);
    mi_venc_InitSw(&pstDevRes->stUtil, 50, 0, _MI_VENC_ReportDevUtil, pstDevRes);

    DBG_EXIT_OK();
    return MI_SUCCESS;

    //kthread_stop(pstDevRes->ptskIrq);
__create_proc_thread_fail:
    MI_VENC_ThreadStop(pstDevRes->stTaskWork);
__create_work_thread_fail:
__register_dev_fail:
    mi_sys_UnRegisterDev(pstDevRes->hMiDev);
    pstDevRes->hMiDev = NULL;
#if VENC_USE_CMDQ
__get_cmdq_avail_buf_fail:
__get_cmdq_service_fail:
    if (pstDevRes->cmdq != NULL)
    {
        MHAL_CMDQ_ReleaseSysCmdqService(pstDevRes->eCmdqId);
        pstDevRes->cmdq = NULL;
    }
#endif
    if(pstDevRes->hHalDev)
    {
        MS_U32 u32DestroyRet;
        u32DestroyRet = MHAL_VENC_DestroyDevice(pstDevRes);
        if (u32DestroyRet != MI_SUCCESS)
        {
            DBG_ERR("Unable to destroy device type:%d %X\n", eDevType, s32Ret);
        }

        pstDevRes->hHalDev = NULL;
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_InitDevice(MI_VENC_Dev_e eDevType)
{
    MS_S32 s32Ret = MI_SUCCESS;
    MHAL_VENC_Drv_t *pstDrv;
    if (eDevType >= E_MI_VENC_DEV_MAX)
    {
        DBG_ERR("Unsupported device type:%d\n", eDevType);
        return MI_ERR_VENC_NOT_SUPPORT;
    }
    pstDrv = astMHalDevConfig[eDevType].pstDrv;
    if(pstDrv->Init)
    {
        s32Ret = pstDrv->Init();
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_CreateDevice(MI_VENC_DevRes_t *pstDevRes, MI_VENC_Dev_e eDevType, void *pOsDev, void** ppBase,
                              int *pSize, MHAL_VENC_DEV_HANDLE *phDev)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    if (pstDevRes == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }

    if (eDevType >= E_MI_VENC_DEV_MAX)
    {
        DBG_ERR("Unsupported device type:%d\n", eDevType);
        return MI_ERR_VENC_NOT_SUPPORT;
    }

    pstDevRes->eMiDevType = eDevType;
    pstDevRes->pstDrv = astMHalDevConfig[eDevType].pstDrv;
    if (pstDevRes->pstDrv->CreateDevice)
    {
        s32Ret = pstDevRes->pstDrv->CreateDevice(astMHalDevConfig[eDevType].u8CoreId, phDev);
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_DestroyDevice(MI_VENC_DevRes_t *pstDevRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    if (pstDevRes == NULL || pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }

    if (pstDevRes->pstDrv->DestroyDevice)
    {
        s32Ret = pstDevRes->pstDrv->DestroyDevice(pstDevRes->hHalDev);
    }
    pstDevRes->eMiDevType = E_MI_VENC_DEV_MAX;
    pstDevRes->hHalDev = NULL;
    pstDevRes->pstDrv = NULL;
    return s32Ret;
}

MS_S32 MHAL_VENC_GetDevConfig(MI_VENC_DevRes_t *pstDevRes, MHAL_VENC_IDX eIdx, MHAL_VENC_Param_t* pstParam)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    if (pstDevRes == NULL || pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }

    //DBG_INFO("stDrvDummy =%p\n", &stDrvDummy);
    //DBG_INFO("GetDevConfig:%p %p %p\n", pstDevRes, pstDevRes->pstDrv,
    //        pstDevRes->pstDrv->GetDevConfig);
    if (pstDevRes->pstDrv->GetDevConfig)
    {
        s32Ret = pstDevRes->pstDrv->GetDevConfig(pstDevRes->hHalDev, eIdx, pstParam);
    }

    return s32Ret;
}

MS_S32 MHAL_VENC_CreateInstance(MI_VENC_DevRes_t *pstDevRes, MI_VENC_ChnRes_t *pstChnRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    if (pstDevRes == NULL || pstDevRes->pstDrv == NULL || pstChnRes == NULL)
    {
        DBG_INFO("Input Null Ptr\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    if (pstChnRes->hInst)
    {
        DBG_ERR("MHAL driver channel has already initialized.\n");
        return MI_ERR_VENC_INVALID_CHNID;
    }

    if (pstDevRes->pstDrv->CreateInstance)
    {
        s32Ret = pstDevRes->pstDrv->CreateInstance(pstDevRes->hHalDev, &(pstChnRes->hInst));
        if (pstChnRes->hInst == NULL)
        {
            DBG_INFO("No Instance Ptr\n");
            return MI_ERR_VENC_NULL_PTR;
        }
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

MS_S32 MHAL_VENC_DestroyInstance(MI_VENC_ChnRes_t *pstChnRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL || pstChnRes->pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }

    pstChnRes->bCreate = FALSE;
    if(pstChnRes->hInst == NULL)
    {
        return MI_SUCCESS;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->DestroyInstance)
    {
        s32Ret = pstDrv->DestroyInstance(pstChnRes->hInst);
        pstChnRes->hInst = NULL;
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_QueryBufSize(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_InternalBuf_t *pstSize)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL || pstChnRes->pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->QueryBufSize)
    {
        //MS_S32 (*QueryBufSize)(MHAL_VENC_INST_HANDLE hInst, MS_U32 *pSize);
        s32Ret = pstDrv->QueryBufSize(pstChnRes->hInst, pstSize);
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_SetParam(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;

    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL)
    {
        DBG_ERR("Null Pointer.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    if (pstChnRes->pstDevRes->pstDrv == NULL)
    {
        DBG_ERR("Uninitialized drivers.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->SetParam)
    {
        s32Ret = pstDrv->SetParam(pstChnRes->hInst, eType, pstParam);
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_GetParam(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_IDX eType, MHAL_VENC_Param_t* pstParam)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL)
    {
        DBG_ERR("Null Pointer.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    if (pstChnRes->pstDevRes->pstDrv == NULL)
    {
        DBG_ERR("Uninitialized drivers.\n");
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->SetParam)
    {
        s32Ret = pstDrv->GetParam(pstChnRes->hInst, eType, pstParam);
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_EncodeOneFrame(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_InOutBuf_t* pInOutBuf)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL || pstChnRes->pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }


    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->EncodeOneFrame)
    {
        //MS_S32 (*EncodeOneFrame)(MHAL_VENC_INST_HANDLE hInst, MHAL_VENC_InOutBuf_t* pInOutBuf);
        if (pInOutBuf != NULL)
        {
#if DBG_INPUT_FRAME
#if 0
            DBG_INFO("Encode Y:%llX,%X U:%llX,%X V:%llX,%X\n",
                     pInOutBuf->phyInputYUVBuf1, pInOutBuf->u32InputYUVBuf1Size,
                     pInOutBuf->phyInputYUVBuf2, pInOutBuf->u32InputYUVBuf2Size,
                     pInOutBuf->phyInputYUVBuf3, pInOutBuf->u32InputYUVBuf3Size);
#else
            DBG_WRN("\ndata.save.binary d:\\venc.y A:0x%llX++0x%X\n",
                    pInOutBuf->phyInputYUVBuf1 | 0x20000000, (pInOutBuf->u32InputYUVBuf1Size << 1) | pInOutBuf->u32InputYUVBuf1Size);
#endif
#endif
        }
#if DEV_DOWN == TIMING_IN_ENCODE_FRAME
        DOWN(&pstChnRes->pstDevRes->frame_sem);
#endif
        mi_venc_RecSwStart(&pstChnRes->pstDevRes->stFps, 0);
        mi_venc_RecSwEnd(&pstChnRes->pstDevRes->stFps, 0, 0);
        mi_venc_RecSwStart(&pstChnRes->pstDevRes->stUtil, 0);
        mi_venc_RecSwStart(&pstChnRes->pstDevRes->stUtil, 1);

        s32Ret = pstDrv->EncodeOneFrame(pstChnRes->hInst, pInOutBuf);
        if (s32Ret != MI_SUCCESS && (s32Ret != MS_VENC_DUMMY_IRQ))
        {
            DBG_INFO("Returns :%d\n", s32Ret);
        }
        else if ((CONNECT_DUMMY_HAL && SKIP_IRQ) || s32Ret == MS_VENC_DUMMY_IRQ)
        {
#if VENC_FPGA && USE_HW_ENC
            MI_VENC_MsSleep(FPGA_DELAY_MS);//for FPGA
#else
            MI_VENC_MsSleep(30);
#endif
            //MHAL_MFE_IsrProc(pstChnRes->pstDevRes->hHalDev);
            s32Ret = MHAL_VENC_IsrProc(pstChnRes->pstDevRes);
            pstChnRes->pstDevRes->bFromIsr = TRUE;
            MI_VENC_WAKE_UP_QUEUE_IF_NECESSARY(pstChnRes->pstDevRes->stIrqWaitQueueHead);
        }
    }
    if(/*(pInOutBuf->pCmdQ != NULL) && */s32Ret == MI_SUCCESS)
    {
        if(pstChnRes->pstDevRes->u32WaitCmdq == 0)
            pstChnRes->pstDevRes->u32WaitCmdq = CMDQ_WAIT_TIME_OUT;
    }
    return s32Ret;
}

MS_S32 MHAL_VENC_IsrProc(MI_VENC_DevRes_t *pstDevRes)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;

    if (pstDevRes == NULL || pstDevRes->pstDrv == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstDevRes->pstDrv;
    if (pstDrv->IsrProc)
    {
        s32Ret = pstDrv->IsrProc(pstDevRes->hHalDev);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_INFO("Returns :%d\n", s32Ret);
        }
    }
    mi_venc_RecSwEnd(&pstDevRes->stUtil, 0, 0);
    //mi_venc_RecSwStart(&pstDevRes->stUtil, 2);

    return s32Ret;
}

MS_S32 MHAL_VENC_EncDone(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_EncResult_t* pstEncRet)
{
    MS_S32 s32Ret = MI_ERR_VENC_UNDEFINED;
    MHAL_VENC_Drv_t *pstDrv;
    if (pstChnRes == NULL || pstChnRes->pstDevRes == NULL || pstChnRes->pstDevRes->pstDrv == NULL)
    {
#if DEV_UP == TIMING_ENCODE_DONE
        UP(&pstChnRes->pstDevRes->frame_sem);
#endif
        return MI_ERR_VENC_NULL_PTR;
    }
    pstDrv = pstChnRes->pstDevRes->pstDrv;
    if (pstDrv->EncodeDone)
    {
        //MS_S32 (*EncodeDone)(MHAL_VENC_INST_HANDLE hInst,MHAL_VENC_EncResult_t* pstEncRet);
        s32Ret = pstDrv->EncodeDone(pstChnRes->hInst, pstEncRet);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_INFO("Returns :%d\n", s32Ret);
        }
    }
    mi_venc_RecSwEnd(&pstChnRes->pstDevRes->stUtil, 1, 0);
    mi_venc_RecSwStart(&pstChnRes->stFps, 0);
    mi_venc_RecSwEnd(&pstChnRes->stFps, 0, pstEncRet->u32OutputBufUsed * 8);

    //pstChnRes->hInst u32OutputBufUsed
#if DEV_UP == TIMING_ENCODE_DONE
    UP(&pstChnRes->pstDevRes->frame_sem);
#endif
    return s32Ret;
}

#if VENC_FPGA || !defined(CONFIG_ARCH_INFINITY2)
#define VALIDATE_I2_RET(s32Ret, label)\
    do {if (s32Ret != MI_SUCCESS) { DBG_WRN("Skip device for FPGA\n"); }} while (0);
#else
#define VALIDATE_I2_RET(s32Ret, label)\
    do {if (s32Ret != MI_SUCCESS) { goto label; }} while (0);
#endif
void MI_VENC_IMPL_Init(void)
{
    MI_S32 s32Ret;
    MI_VENC_Dev_e i;

    if (_ModRes.bInitFlag)
    {
        DBG_INFO("Already Initialized.\n");
        return;
    }
    _MI_VENC_IMPL_InitRes();

    //==== Create Device and Instances ====
    /**
     * E_MI_VENC_DEV_MHE0
     * |-- up to 16 instances
     * E_MI_VENC_DEV_MHE1
     * |-- up to 16 instances
     * E_MI_VENC_DEV_MFE0
     * |-- up to 16 instances
     * E_MI_VENC_DEV_MFE1
     * |-- Not included for now but maintain for extension
     * E_MI_VENC_DEV_JPEG
     * |-- CPU. Should also up to 16 instances
     */
    DBG_ENTER();

#if 1
    for (i = 0; i < E_MI_VENC_DEV_MAX; ++i)
    {
        s32Ret = _MI_VENC_CreateDevice(i);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_ERR("Unable to Create DEV%d.\n", i);
            break;
        }
    }
#else
    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_MHE0);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_mhe0);

    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_MHE1);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_mhe1);

    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_MFE0);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_mfe0);

    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_MFE1);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_mfe1);

    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_JPEG);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_jpeg);

#if CONNECT_DUMMY_HAL
    s32Ret = _MI_VENC_CreateDevice(E_MI_VENC_DEV_DUMMY);
    VALIDATE_I2_RET(s32Ret, __fail_create_dev_mfe0);
#endif
#endif

#ifdef MI_SYS_PROC_FS_DEBUG
    if (s32Ret == MI_SUCCESS)
    {
        MI_U32 i;
        MI_U32 j;
        for(i = 0; i < sizeof(_astDbgFsCmds) / sizeof(_astDbgFsCmds[0]); ++i)
        {
            for(j = 0; j < E_MI_VENC_DEV_MAX; j++)
            {
                mi_sys_RegistCommand(_astDbgFsCmds[i].szName, _astDbgFsCmds[i].u8MaxArgc, _astDbgFsCmds[i].fpExecCmd,
                                     _ModRes.devs[j].hMiDev);
                if(_astDbgFsCmds[i].ppstSelf)
                {
                    *_astDbgFsCmds[i].ppstSelf = _astDbgFsCmds + i;
                }
            }
        }
    }
#endif

    if (s32Ret == MI_SUCCESS)
    {
        _ModRes.bInitFlag = TRUE;
        return;
    }
    else
    {

        for (i = 0; i < E_MI_VENC_DEV_MAX; ++i)
        {
            s32Ret = _MI_VENC_DestroyDevice(i);
            if (s32Ret != MI_SUCCESS)
            {
                DBG_ERR("Unable to destroy DEV%d.\n", i);
            }
        }
        _ModRes.bInitFlag = FALSE;
    }
    return;
}

static MI_S32 _MI_VENC_DestroyDevice(MI_VENC_Dev_e eDevType)
{
    MI_VENC_DevRes_t *pstDevRes;
    MI_S32 s32Ret;
    MI_S32 s32Result = MI_SUCCESS;

    pstDevRes = _ModRes.devs + eDevType;
    if (pstDevRes->hMiDev && pstDevRes->bInitFlag)
    {
        int i, ret, retry;
        MI_BOOL bAllClear;

        retry = 0;
        while (retry++ < 4)
        {
            bAllClear = TRUE;
            for (i = 0; i < MI_VENC_MAX_CHN_NUM_PER_MODULE; ++i)
            {
                if (pstDevRes->astChnRes[i])
                {
                    if (pstDevRes->astChnRes[i]->bStart)
                    {
                        DBG_WRN("CH%d is still running!\n", i);
                        s32Ret = MI_VENC_IMPL_StopRecvPic(i);
                        if (s32Ret != MI_SUCCESS)
                        {
                            DBG_ERR("Unable to stop ch%d\n", i);
                            bAllClear = FALSE;
                        }
                        //return MI_ERR_VENC_CHN_NOT_STOPPED;
                    }
                    if (pstDevRes->astChnRes[i]->bCreate)
                    {
                        DBG_WRN("CH%d is still created!\n", i);
                        s32Ret = MI_VENC_IMPL_DestroyChn(pstDevRes->astChnRes[i]->u8ChnId);
                        if (s32Ret != MI_SUCCESS)
                        {
                            DBG_ERR("Unable to destroy ch%d\n", i);
                            bAllClear = FALSE;
                        }
                        //return MI_ERR_VENC_CHN_NOT_STOPPED;
                    }
                }
            }
            if (bAllClear)
            {
                break;
            }
            else
            {
                DBG_ERR("Sleep 1000\n");
                MI_VENC_MsSleep(1000);
            }
        }

        if (pstDevRes->u32IrqNum != MHAL_VENC_DUMMY_IRQ)
        {
            MI_VENC_FreeIrq(pstDevRes->u32IrqNum, pstDevRes);
            pstDevRes->u32IrqNum = 0;
        }

        if (pstDevRes->cmdq)
        {
            MHAL_CMDQ_ReleaseSysCmdqService(pstDevRes->eCmdqId);
            pstDevRes->cmdq = NULL;
        }

        //stop threads
        //if bWorkTaskRun is used to stop the thread, the thread might stop before
        //kthread_stop and causing kernel dump...
        //pstDevRes->bWorkTaskRun = FALSE;
        //pstDevRes->bIrqTaskRun = FALSE;
        pstDevRes->bFromIsr = TRUE;
        if (pstDevRes->stTaskWork)
        {
            ret = MI_VENC_ThreadStop(pstDevRes->stTaskWork);
            if (ret != 0)
                DBG_WRN("work kthread_stop ret:%d\n", ret);
            //put_task_struct(pstDevRes->ptskWork);
            pstDevRes->stTaskWork = NULL;
        }
        if (pstDevRes->stTaskIrq)
        {
            ret = MI_VENC_ThreadStop(pstDevRes->stTaskIrq);
            if (ret != 0)
                DBG_WRN("irq kthread_stop ret:%d\n", ret);
            //put_task_struct(pstDevRes->ptskIrq);
            pstDevRes->stTaskIrq = NULL;
        }

        //TODO handling list here?
        //if (pstDevRes->todo_task_list && pstDevRes->working_task_list)
        {
            DOWN(&pstDevRes->list_mutex);
            MI_VENC_INIT_LIST_HEAD(&pstDevRes->todo_task_list);
            MI_VENC_INIT_LIST_HEAD(&pstDevRes->working_task_list);
            UP(&pstDevRes->list_mutex);
        }

        if(pstDevRes->hHalDev)
        {
            s32Ret = MHAL_VENC_DestroyDevice(pstDevRes);
            if (s32Ret != MI_SUCCESS)
            {
                DBG_ERR("Unable to destroy device type:%d %X\n", eDevType, s32Ret);
            }
            pstDevRes->hHalDev = NULL;
        }

        //unregister device
        s32Ret = mi_sys_UnRegisterDev(pstDevRes->hMiDev);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_ERR("mi_sys_UnRegisterDev :%X\n", s32Ret);
            s32Result = MI_ERR_VENC_INVALID_DEVID;
        }

        pstDevRes->bInitFlag = FALSE;
    }
    return s32Result;
}

void MI_VENC_IMPL_DeInit(void)
{
    MI_S32 s32Ret;
    MI_VENC_Dev_e i;
    DBG_ENTER();

#if 1
    for (i = 0; i < E_MI_VENC_DEV_MAX; ++i)
    {
        s32Ret = _MI_VENC_DestroyDevice(i);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_ERR("Unable to destroy DEV%d.\n", i);
        }
    }
#else
    s32Ret = _MI_VENC_DestroyDevice(E_MI_VENC_DEV_MHE0);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy E_MI_VENC_DEV_MHE0.\n");
    }
    s32Ret = _MI_VENC_DestroyDevice(E_MI_VENC_DEV_MHE1);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy E_MI_VENC_DEV_MHE1.\n");
    }
    s32Ret = _MI_VENC_DestroyDevice(E_MI_VENC_DEV_MFE0);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy E_MI_VENC_DEV_MFE0.\n");
    }
    s32Ret = _MI_VENC_DestroyDevice(E_MI_VENC_DEV_JPEG);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy E_MI_VENC_DEV_JPEG.\n");
    }
#if CONNECT_DUMMY_HAL

    s32Ret = _MI_VENC_DestroyDevice(E_MI_VENC_DEV_DUMMY);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy dummy.\n");
    }
#endif
#endif
    _ModRes.bInitFlag = FALSE;

    if (s32Ret == MI_SUCCESS)
        DBG_EXIT_OK();
    else
        DBG_EXIT_ERR("err:0x%X\n", s32Ret);
}

#if 0
void _MI_VENC_DumpDevChn(MI_VENC_DevRes_t *pstDevRes)
{
    int i;

    if (pstDevRes == NULL)
    {
        DBG_ERR("Null pointer.\n");
        return;
    }
    for (i = 0; i < MI_VENC_MAX_CHN_NUM_PER_MODULE; ++i)
    {
        DBG_INFO("[%d]=%p\n", i, pstDevRes->astChnRes[i]);
    }
}
#endif

static MI_S32 _MI_VENC_FindAvailableDevChn(MI_VENC_DevRes_t *pstDevRes, MI_VENC_CHN VeChn, MI_VENC_ChnRes_t **ppOutChn)
{
    int i;

    if (pstDevRes == NULL)
        return MI_ERR_VENC_NULL_PTR;

    //_MI_VENC_DumpDevChn(pstDevRes);

    //find first available MI_VENC_CHNRes_t
    for (i = 0; i < MI_VENC_MAX_CHN_NUM_PER_MODULE; ++i)
    {
        if (pstDevRes->astChnRes[i] == NULL)
        {
            //DBG_INFO("Found free chn %d\n", i); MT_INFO("Found free chn %d\n", i);
            pstDevRes->astChnRes[i] = _ModRes.astChnRes + VeChn;
            pstDevRes->astChnRes[i]->pstDevRes = pstDevRes;
            pstDevRes->astChnRes[i]->u8ChnId = VeChn;
            *ppOutChn = pstDevRes->astChnRes[i];
            return MI_SUCCESS;
        }
    }

    DBG_ERR("Unable to find any free channel\n");
    *ppOutChn = NULL;
    return MI_ERR_VENC_UNEXIST;
}

static MI_S32 _MI_VENC_FreeChnMemory(MI_VENC_ChnRes_t *pstChnRes)
{
    if (pstChnRes)
    {
        if (pstChnRes->stAlMemInfo.phyAddr)
        {
            mi_sys_MMA_Free(pstChnRes->stAlMemInfo.phyAddr);
            pstChnRes->stAlMemInfo.phyAddr = NULL;
        }
        if (pstChnRes->stAlMemInfo.pVirAddr)
        {
            mi_sys_UnVmap(pstChnRes->stAlMemInfo.pVirAddr);
            pstChnRes->stAlMemInfo.pVirAddr = NULL;
        }
        if (pstChnRes->stRefMemInfo.phyAddr)
        {
            mi_sys_MMA_Free(pstChnRes->stRefMemInfo.phyAddr);
            pstChnRes->stRefMemInfo.phyAddr = NULL;
        }
        return MI_SUCCESS;
    }
    return MI_ERR_VENC_NULL_PTR;
}

static void _MI_VENC_GetFpsPtr(MI_VENC_ChnAttr_t *pstAttr, MI_U32 **ppu32SrcFrmRateNum, MI_U32 **ppu32SrcFrmRateDen)
{
    //private function, does not check NULL pointer.
    switch (pstAttr->stRcAttr.eRcMode)
    {
        case E_MI_VENC_RC_MODE_H264FIXQP:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264ABR:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH264Abr.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH264Abr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264CBR:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264VBR:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H265FIXQP:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H265CBR:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H265VBR:
            *ppu32SrcFrmRateNum = &pstAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum;
            *ppu32SrcFrmRateDen = &pstAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen;
            break;
        default:
            *ppu32SrcFrmRateNum = NULL;
            *ppu32SrcFrmRateDen = NULL;
            break;
    }
}

static MI_S32 _MI_VENC_VerifyFps(MI_U32 *pu32SrcFrmRateNum, MI_U32 *pu32SrcFrmRateDen)
{
    //private function, does not check NULL pointer.
    if(*pu32SrcFrmRateNum == 0 || *pu32SrcFrmRateDen == 0)
    {
        DBG_ERR("Input invalid FPS:%d/%d. Overwrite it into 30/1\n",
                *pu32SrcFrmRateNum, *pu32SrcFrmRateDen);
        *pu32SrcFrmRateNum = 30;
        *pu32SrcFrmRateDen = 1;
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    return MI_SUCCESS;
}
//sync with get_venc_dev_id (user demo app)
static MI_U32 _MI_VENC_ChooseIP(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
#if 0
    //use mhe0 only
    return E_MI_VENC_DEV_MHE0;
#elif 0 //for mhe1 only SOF
    return E_MI_VENC_DEV_MHE1;
#elif 0
    //simple implementation
    if (VeChn & 1)
    {
        return E_MI_VENC_DEV_MHE1;
    }
    return E_MI_VENC_DEV_MHE0;
#elif 0
    if(pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        return E_MI_VENC_DEV_MFE0 + (VeChn & 1);
    }
    if(pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        return E_MI_VENC_DEV_MHE0 + (VeChn & 1);
    }
    return E_MI_VENC_DEV_MAX;
#else
    //by MB numbers
    MI_VENC_ChnRes_t *pstChnRes;
    MI_U32 u32MbCurChn;
    MI_VENC_DevRes_t* pstDevRes[2];
    MI_U32 *pu32SrcFrmRateNum = NULL, *pu32SrcFrmRateDen = NULL;

    _MI_VENC_GetFpsPtr(pstAttr, &pu32SrcFrmRateNum, &pu32SrcFrmRateDen);
    _MI_VENC_VerifyFps(pu32SrcFrmRateNum, pu32SrcFrmRateDen);
    if(pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        pstDevRes[0] = _ModRes.devs + E_MI_VENC_DEV_MFE0;
        pstDevRes[1] = _ModRes.devs + E_MI_VENC_DEV_MFE1;
        u32MbCurChn = ((pstAttr->stVeAttr.stAttrH264e.u32MaxPicWidth + 15) >> 4) *
                      ((pstAttr->stVeAttr.stAttrH264e.u32MaxPicHeight + 15)>> 4);
        switch (pstAttr->stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_H264FIXQP:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H264ABR:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH264Abr.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH264Abr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H264CBR:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H264VBR:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen;
                break;
            default:
                return E_MI_VENC_DEV_MAX;
                break;
        }
    }
    else if(pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        pstDevRes[0] = _ModRes.devs + E_MI_VENC_DEV_MHE0;
        pstDevRes[1] = _ModRes.devs + E_MI_VENC_DEV_MHE1;
        u32MbCurChn = ((pstAttr->stVeAttr.stAttrH265e.u32MaxPicWidth + 15) >> 4) *
                      ((pstAttr->stVeAttr.stAttrH265e.u32MaxPicHeight + 15)>> 4);
        switch (pstAttr->stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_H265FIXQP:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H265CBR:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen;
                break;
            case E_MI_VENC_RC_MODE_H265VBR:
                u32MbCurChn = u32MbCurChn * pstAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum
                              / pstAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen;
                break;
            default:
                return E_MI_VENC_DEV_MAX;
                break;
        }
    }
    else
        return E_MI_VENC_MODTYPE_MAX;

    pstChnRes = _ModRes.astChnRes + VeChn;
    pstChnRes->u32MbRate = u32MbCurChn;
    if(pstDevRes[1]->u32MbRate < pstDevRes[0]->u32MbRate)
    {
        return pstDevRes[1] - &_ModRes.devs[0];
    }
    return pstDevRes[0] - &_ModRes.devs[0];
#endif
}

/** @brief Convert Rate Control structure from MI to MHAL
 *
 * @param[in] pstMiRc The pointer to MI structure
 * @param[out] pstMhalRc The pinter to output the converted MHAL RC structure
 * @return MI_SUCCESS
 */
static MI_S32 _MI_VENC_ConvertRc(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_RcAttr_t *pstMiRc, MHAL_VENC_RcInfo_t *pstMhalRc)
{
    if (pstMiRc == NULL || pstMhalRc == NULL || pstChnRes == NULL)
        return MI_ERR_VENC_NULL_PTR;

    MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, (*pstMhalRc));
    switch (pstMiRc->eRcMode)
    {
        case E_MI_VENC_RC_MODE_H265CBR:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H265CBR;
            pstMhalRc->stAttrH265Cbr.u32SrcFrmRateNum  = pstMiRc->stAttrH265Cbr.u32SrcFrmRateNum;
            pstMhalRc->stAttrH265Cbr.u32SrcFrmRateDen  = pstMiRc->stAttrH265Cbr.u32SrcFrmRateDen;
            pstMhalRc->stAttrH265Cbr.u32Gop            = pstMiRc->stAttrH265Cbr.u32Gop;
            pstMhalRc->stAttrH265Cbr.u32StatTime       = pstMiRc->stAttrH265Cbr.u32StatTime;
            pstMhalRc->stAttrH265Cbr.u32BitRate        = pstMiRc->stAttrH265Cbr.u32BitRate;
            pstMhalRc->stAttrH265Cbr.u32FluctuateLevel = pstMiRc->stAttrH265Cbr.u32FluctuateLevel;
            pstChnRes->stCtx.u32Gop                    = pstMiRc->stAttrH265Cbr.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate          = pstMiRc->stAttrH265Cbr.u32BitRate;
            pstChnRes->stCtx.u32SrcFrmRateNum          = pstMiRc->stAttrH265Cbr.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen          = pstMiRc->stAttrH265Cbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H265VBR:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H265VBR;
            pstMhalRc->stAttrH265Vbr.u32SrcFrmRateNum  = pstMiRc->stAttrH265Vbr.u32SrcFrmRateNum;
            pstMhalRc->stAttrH265Vbr.u32SrcFrmRateDen  = pstMiRc->stAttrH265Vbr.u32SrcFrmRateDen;
            pstMhalRc->stAttrH265Vbr.u32Gop            = pstMiRc->stAttrH265Vbr.u32Gop;
            pstMhalRc->stAttrH265Vbr.u32MaxBitRate     = pstMiRc->stAttrH265Vbr.u32MaxBitRate;
            pstMhalRc->stAttrH265Vbr.u32MaxQp          = pstMiRc->stAttrH265Vbr.u32MaxQp;
            pstMhalRc->stAttrH265Vbr.u32MinQp          = pstMiRc->stAttrH265Vbr.u32MinQp;
            pstChnRes->stCtx.u32Gop                    = pstMiRc->stAttrH265Vbr.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate          = pstMiRc->stAttrH265Vbr.u32MaxBitRate;
            pstChnRes->stCtx.u32SrcFrmRateNum          = pstMiRc->stAttrH265Vbr.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen          = pstMiRc->stAttrH265Vbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H265FIXQP:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H265FIXQP;
            pstMhalRc->stAttrH265FixQp.u32SrcFrmRateNum = pstMiRc->stAttrH265FixQp.u32SrcFrmRateNum;
            pstMhalRc->stAttrH265FixQp.u32SrcFrmRateDen = pstMiRc->stAttrH265FixQp.u32SrcFrmRateDen;
            pstMhalRc->stAttrH265FixQp.u32Gop           = pstMiRc->stAttrH265FixQp.u32Gop;
            pstMhalRc->stAttrH265FixQp.u32IQp           = pstMiRc->stAttrH265FixQp.u32IQp;
            pstMhalRc->stAttrH265FixQp.u32PQp           = pstMiRc->stAttrH265FixQp.u32PQp;
            pstChnRes->stCtx.u32Gop                     = pstMiRc->stAttrH265FixQp.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate           = 0;
            pstChnRes->stCtx.u32SrcFrmRateNum           = pstMiRc->stAttrH265FixQp.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen           = pstMiRc->stAttrH265FixQp.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264CBR:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H264CBR;
            pstMhalRc->stAttrH264Cbr.u32SrcFrmRateNum  = pstMiRc->stAttrH264Cbr.u32SrcFrmRateNum;
            pstMhalRc->stAttrH264Cbr.u32SrcFrmRateDen  = pstMiRc->stAttrH264Cbr.u32SrcFrmRateDen;
            pstMhalRc->stAttrH264Cbr.u32Gop            = pstMiRc->stAttrH264Cbr.u32Gop;
            pstMhalRc->stAttrH264Cbr.u32StatTime       = pstMiRc->stAttrH264Cbr.u32StatTime;
            pstMhalRc->stAttrH264Cbr.u32BitRate        = pstMiRc->stAttrH264Cbr.u32BitRate;
            pstMhalRc->stAttrH264Cbr.u32FluctuateLevel = pstMiRc->stAttrH264Cbr.u32FluctuateLevel;
            pstChnRes->stCtx.u32Gop                    = pstMiRc->stAttrH264Cbr.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate          = pstMiRc->stAttrH264Cbr.u32BitRate;
            pstChnRes->stCtx.u32SrcFrmRateNum          = pstMiRc->stAttrH264Cbr.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen          = pstMiRc->stAttrH264Cbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264VBR:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H264VBR;
            pstMhalRc->stAttrH264Vbr.u32SrcFrmRateNum  = pstMiRc->stAttrH264Vbr.u32SrcFrmRateNum;
            pstMhalRc->stAttrH264Vbr.u32SrcFrmRateDen  = pstMiRc->stAttrH264Vbr.u32SrcFrmRateDen;
            pstMhalRc->stAttrH264Vbr.u32Gop            = pstMiRc->stAttrH264Vbr.u32Gop;
            pstMhalRc->stAttrH264Vbr.u32MaxBitRate     = pstMiRc->stAttrH264Vbr.u32MaxBitRate;
            pstMhalRc->stAttrH264Vbr.u32MaxQp          = pstMiRc->stAttrH264Vbr.u32MaxQp;
            pstMhalRc->stAttrH264Vbr.u32MinQp          = pstMiRc->stAttrH264Vbr.u32MinQp;
            pstChnRes->stCtx.u32Gop                    = pstMiRc->stAttrH264Vbr.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate          = pstMiRc->stAttrH264Vbr.u32MaxBitRate;
            pstChnRes->stCtx.u32SrcFrmRateNum          = pstMiRc->stAttrH264Vbr.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen          = pstMiRc->stAttrH264Vbr.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_H264FIXQP:
            pstMhalRc->eRcMode = E_MHAL_VENC_RC_MODE_H264FIXQP;
            pstMhalRc->stAttrH264FixQp.u32SrcFrmRateNum = pstMiRc->stAttrH264FixQp.u32SrcFrmRateNum;
            pstMhalRc->stAttrH264FixQp.u32SrcFrmRateDen = pstMiRc->stAttrH264FixQp.u32SrcFrmRateDen;
            pstMhalRc->stAttrH264FixQp.u32Gop           = pstMiRc->stAttrH264FixQp.u32Gop;
            pstMhalRc->stAttrH264FixQp.u32IQp           = pstMiRc->stAttrH264FixQp.u32IQp;
            pstMhalRc->stAttrH264FixQp.u32PQp           = pstMiRc->stAttrH264FixQp.u32PQp;
            pstChnRes->stCtx.u32Gop                     = pstMiRc->stAttrH264FixQp.u32Gop;
            pstChnRes->stCtx.u32TargetBitrate           = 0;
            pstChnRes->stCtx.u32SrcFrmRateNum           = pstMiRc->stAttrH264FixQp.u32SrcFrmRateNum;
            pstChnRes->stCtx.u32SrcFrmRateDen           = pstMiRc->stAttrH264FixQp.u32SrcFrmRateDen;
            break;
        case E_MI_VENC_RC_MODE_MJPEGFIXQP:
            pstMhalRc->eRcMode = E_MHAL_VENC_JPEG_RC;
            {
                MHAL_VENC_RcInfo_t stRc;
                MI_S32 s32Err;
                int i;
                MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stRc);
                s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
                if(s32Err != MI_SUCCESS)
                {
                    DBG_ERR("Get Param err 0x%X.\n", s32Err);
                    return MI_ERR_VENC_UNDEFINED;
                }

                pstMhalRc->stAttrMJPGRc.u32Qfactor = stRc.stAttrMJPGRc.u32Qfactor;
                for(i = 0; i < (sizeof(stRc.stAttrMJPGRc.u8YQt) / sizeof(stRc.stAttrMJPGRc.u8YQt[0])); ++i)
                {
                    pstMhalRc->stAttrMJPGRc.u8YQt[i] = stRc.stAttrMJPGRc.u8YQt[i];
                    pstMhalRc->stAttrMJPGRc.u8CbCrQt[i] = stRc.stAttrMJPGRc.u8CbCrQt[i];
                }
            }
            pstChnRes->stCtx.u32Gop = 0;
            pstChnRes->stCtx.u32TargetBitrate = 0;
            pstChnRes->stCtx.u32SrcFrmRateNum = 1;
            pstChnRes->stCtx.u32SrcFrmRateDen = 1;
            break;
        default:
            DBG_WRN("Unsupported RC mode %d\n", pstMiRc->eRcMode);
            return MI_ERR_VENC_ILLEGAL_PARAM;//not supported yet.
            break;
    }

    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_GetRcIdx(MI_VENC_RcAttr_t *pstMiRc, MHAL_VENC_Idx_e *peMhalIdx)
{
    if (pstMiRc == NULL || peMhalIdx == NULL)
        return MI_ERR_VENC_NULL_PTR;

    switch (pstMiRc->eRcMode)
    {
        case E_MI_VENC_RC_MODE_H265FIXQP:
        case E_MI_VENC_RC_MODE_H265CBR:
        case E_MI_VENC_RC_MODE_H265VBR:
            *peMhalIdx = E_MHAL_VENC_265_RC;
            break;

        case E_MI_VENC_RC_MODE_H264FIXQP:
        case E_MI_VENC_RC_MODE_H264CBR:
        case E_MI_VENC_RC_MODE_H264VBR:
        case E_MI_VENC_RC_MODE_H264ABR:
            *peMhalIdx = E_MHAL_VENC_264_RC;
            break;
        case E_MI_VENC_RC_MODE_MJPEGFIXQP:
            *peMhalIdx = E_MHAL_VENC_JPEG_RC;
            break;
        default:
            DBG_WRN("Unsupported RC mode %d\n", pstMiRc->eRcMode);
            return MI_ERR_VENC_ILLEGAL_PARAM;//not supported yet.
            break;
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_SetMaxResolution(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_ChnAttr_t *pstAttr)
{
    MHAL_ErrCode_e err;
    MHAL_VENC_Resoluton_t stMHalResolution;
    MHAL_VENC_IDX eIdxRes;

    MHAL_VENC_INIT_PARAM(MHAL_VENC_Resoluton_t, stMHalResolution);
    stMHalResolution.eFmt   = E_MHAL_VENC_FMT_NV12;
    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H265E:
            eIdxRes = E_MHAL_VENC_265_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrH265e.u32MaxPicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrH265e.u32MaxPicHeight;
            break;
        case E_MI_VENC_MODTYPE_H264E:
            eIdxRes = E_MHAL_VENC_264_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrH264e.u32MaxPicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrH264e.u32MaxPicHeight;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            eIdxRes = E_MHAL_VENC_JPEG_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight;
            break;
        case E_MI_VENC_MODTYPE_MAX:
            return MI_SUCCESS;
            break;
        default:
            return MI_ERR_VENC_NOT_SUPPORT;
            break;
    }
    err = MHAL_VENC_SetParam(pstChnRes, eIdxRes, &stMHalResolution);
    if (err != MI_SUCCESS)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_ConfigInstance(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_ChnAttr_t *pstAttr)
{
    MHAL_ErrCode_e err;
    MI_S32 s32Ret;
    MHAL_VENC_IDX eIdxRes, eIdxRcExpected, eIdxRcIn;
    MHAL_VENC_Resoluton_t stMHalResolution;
    MHAL_VENC_RcInfo_t stMhalRcInfo;

    if (pstChnRes == NULL || pstAttr == NULL)
        return MI_ERR_VENC_NULL_PTR;

    MHAL_VENC_INIT_PARAM(MHAL_VENC_Resoluton_t, stMHalResolution);
    MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stMhalRcInfo);
    if(pstChnRes->eInputFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        stMHalResolution.eFmt = E_MHAL_VENC_FMT_NV12;
    else if(pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE && pstChnRes->eInputFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
        stMHalResolution.eFmt = E_MHAL_VENC_FMT_YUYV;
    else
        DBG_WRN("[%s]YUV Format not supprot, use NV12 by default\n");

    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H265E:
            eIdxRes = E_MHAL_VENC_265_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrH265e.u32PicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrH265e.u32PicHeight;
            eIdxRcExpected = E_MHAL_VENC_265_RC;
            break;
        case E_MI_VENC_MODTYPE_H264E:
            eIdxRes = E_MHAL_VENC_264_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrH264e.u32PicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrH264e.u32PicHeight;
            eIdxRcExpected = E_MHAL_VENC_264_RC;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            eIdxRes = E_MHAL_VENC_JPEG_RESOLUTION;
            stMHalResolution.u32Width  = pstAttr->stVeAttr.stAttrJpeg.u32PicWidth;
            stMHalResolution.u32Height = pstAttr->stVeAttr.stAttrJpeg.u32PicHeight;
            eIdxRcExpected = E_MHAL_VENC_JPEG_RC;
            break;
#if CONNECT_DUMMY_HAL
        case E_MI_VENC_MODTYPE_MAX:
            return MI_SUCCESS;
            break;
#endif
        default:
            return MI_ERR_VENC_NOT_SUPPORT;
            break;
    }

    s32Ret = _MI_VENC_ConvertRc(pstChnRes, &pstAttr->stRcAttr, &stMhalRcInfo);
    if(MI_SUCCESS == s32Ret)
    {
        s32Ret = _MI_VENC_GetRcIdx(&pstAttr->stRcAttr, &eIdxRcIn);
    }
    if(MI_SUCCESS == s32Ret)
    {
        if(eIdxRcIn != eIdxRcExpected)
        {
            DBG_ERR("Input RC attr type:%d does not match with expected type:%d\n", eIdxRcIn, eIdxRcExpected);
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        err = MHAL_VENC_SetParam(pstChnRes, eIdxRes, &stMHalResolution);
        if (err != MI_SUCCESS)
        {
            DBG_ERR("Unable to set resolution\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }

        err = MHAL_VENC_SetParam(pstChnRes, eIdxRcExpected, &stMhalRcInfo);
        if (err != MI_SUCCESS)
        {
            DBG_ERR("Unable to set RC\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
    }
    //pstChnRes->stStat.eInputFmt = stMHalResolution.eFmt;

    return s32Ret;
}

static MI_S32 _MI_VENC_ValidateResolution(MI_VENC_ChnAttr_t *pstAttr, MI_VENC_DevRes_t* pstDevRes)
{
    MI_U32 u32W = 0, u32H = 0;
    MI_U32 u32WAligned = 0, u32HAligned = 0;
    MI_BOOL bOversize = FALSE;

    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H264E:
            u32W = pstAttr->stVeAttr.stAttrH264e.u32PicWidth ;//& (32-1);
            u32H = pstAttr->stVeAttr.stAttrH264e.u32PicHeight;//& (2-1);
            if(u32W > pstAttr->stVeAttr.stAttrH264e.u32MaxPicWidth ||
                    u32H > pstAttr->stVeAttr.stAttrH264e.u32MaxPicHeight)
            {
                bOversize = TRUE;
            }
            break;
        case E_MI_VENC_MODTYPE_H265E:
            u32W = pstAttr->stVeAttr.stAttrH265e.u32PicWidth ;//& (16-1);
            u32H = pstAttr->stVeAttr.stAttrH265e.u32PicHeight;//& (2-1);
            if(u32W > pstAttr->stVeAttr.stAttrH265e.u32MaxPicWidth ||
                    u32H > pstAttr->stVeAttr.stAttrH265e.u32MaxPicHeight)
            {
                bOversize = TRUE;
            }
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            u32W = pstAttr->stVeAttr.stAttrJpeg.u32PicWidth  ;//& (32-1);
            u32H = pstAttr->stVeAttr.stAttrJpeg.u32PicHeight ;//& (2-1);
            if(u32W > pstAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth ||
                    u32H > pstAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight)
            {
                bOversize = TRUE;
            }
            break;
        default:
            return MI_SUCCESS;
            break;
    }
    if(bOversize)
    {
        DBG_ERR("Resolution requested(%dx%d) exceeds max resolution\n", u32W, u32H);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    if(pstDevRes == NULL)
    {
        DBG_WRN("NULL device\n");
    }
    else
    {
        u32WAligned = u32W & (pstDevRes->stAlign.u32AlignW - 1);
        u32HAligned = u32H & (pstDevRes->stAlign.u32AlignH - 1);
    }
    if(u32WAligned != 0 || u32HAligned != 0)
    {
        DBG_ERR("Invalid Resolution %dx%d, which should align with %dx%d\n", u32W, u32H,
                pstDevRes->stAlign.u32AlignW, pstDevRes->stAlign.u32AlignH);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_AllocBuf(MI_VENC_ChnRes_t *pstChnRes)
{
    MI_VENC_DevRes_t *pstDevRes = pstChnRes->pstDevRes;
    MI_S32 u32Ret;

    //allocate internal buffer size, such as refer/current YUV buffers
    MHAL_VENC_INIT_PARAM(MHAL_VENC_InternalBuf_t, pstChnRes->stVencInternalBuf);

    MT_INFO("query size\n");
    u32Ret = MHAL_VENC_QueryBufSize(pstChnRes, &pstChnRes->stVencInternalBuf);
    if (u32Ret != MI_SUCCESS/*MI_SUCCESS*/)
    {
        DBG_ERR("QueryBufSize\n");
        return MI_ERR_VENC_NOTREADY;
    }
    pstChnRes->stAlMemInfo.u32BufSize = pstChnRes->stVencInternalBuf.u32IntrAlBufSize;
    pstChnRes->stRefMemInfo.u32BufSize = pstChnRes->stVencInternalBuf.u32IntrRefBufSize;
    //some driver, such as JPE might not need any buffer here.
    if(pstChnRes->stVencInternalBuf.u32IntrAlBufSize > 0)
    {
        u32Ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stVencInternalBuf.u32IntrAlBufSize, &pstChnRes->stAlMemInfo.phyAddr);
        if(u32Ret != MI_SUCCESS)
        {
            DBG_ERR("mi_sys_MMA_Alloc\n");
            return MI_ERR_VENC_NOTREADY;
        }
        pstChnRes->stAlMemInfo.pVirAddr = mi_sys_Vmap(pstChnRes->stAlMemInfo.phyAddr, pstChnRes->stAlMemInfo.u32BufSize,
                                          FALSE/*non-cached*/);
        if(pstChnRes->stAlMemInfo.pVirAddr == NULL)
        {
            mi_sys_MMA_Free(pstChnRes->stAlMemInfo.phyAddr);
            pstChnRes->stAlMemInfo.phyAddr = NULL;
            DBG_ERR("null stAlMemInfo.pVirAddr\n");
            return MI_ERR_VENC_NOTREADY;
        }
    }
    if(pstChnRes->stVencInternalBuf.u32IntrRefBufSize > 0)
    {
        u32Ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stVencInternalBuf.u32IntrRefBufSize, &pstChnRes->stRefMemInfo.phyAddr);
        if (u32Ret != MI_SUCCESS)
        {
            _MI_VENC_FreeChnMemory(pstChnRes);
            DBG_ERR("mi_sys_MMA_Alloc 2\n");
            return MI_ERR_VENC_NOTREADY;
        }

        MT_INFO("Vmem pass\n");
    }

    pstChnRes->stVencInternalBuf.phyIntrAlPhyBuf  = pstChnRes->stAlMemInfo.phyAddr;
    pstChnRes->stVencInternalBuf.pu8IntrAlVirBuf  = pstChnRes->stAlMemInfo.pVirAddr;
    pstChnRes->stVencInternalBuf.phyIntrRefPhyBuf = pstChnRes->stRefMemInfo.phyAddr;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_CreateChn(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_S32 ret = MI_ERR_VENC_UNDEFINED;
    MI_SYS_FrameBufExtraConfig_t  stVencInputAlignment;
    MI_U32 u32Dev;

    DBG_INFO("Create Encode Chn :%d\n", VeChn);

    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        DBG_ERR("Invalid Channel\n");
        return MI_ERR_VENC_INVALID_CHNID;
    }

    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        case E_MI_VENC_MODTYPE_H265E:
            //H.265 load balancer. refine later with MB number
            u32Dev = _MI_VENC_ChooseIP(VeChn, pstAttr);
            if(E_MI_VENC_DEV_MAX == u32Dev)
            {
                DBG_ERR("Unable to choose IP on ch%d\n", VeChn);
                return MI_ERR_VENC_ILLEGAL_PARAM;
            }
            pstDevRes = _ModRes.devs + u32Dev;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            pstDevRes = _ModRes.devs + E_MI_VENC_DEV_JPEG;
            break;
#if CONNECT_DUMMY_HAL
        case E_MI_VENC_MODTYPE_MAX:
            pstDevRes = _ModRes.devs + E_MI_VENC_DEV_DUMMY;
            break;
#endif
        default:
            DBG_ERR("Unsupported mod type %d\n", pstAttr->stVeAttr.eType);
            return MI_ERR_VENC_ILLEGAL_PARAM;
            break;
    }

    ret = _MI_VENC_ValidateResolution(pstAttr, pstDevRes);
    if (ret != MI_SUCCESS)
    {
        DBG_ERR("Currently H.264/H.265 W and H must be multiples of 32\n");
        return ret;
    }

    ret = _MI_VENC_FindAvailableDevChn(pstDevRes, VeChn, &pstChnRes);
    if (ret != MI_SUCCESS)
    {
        DBG_ERR("Failed to find channel\n");
        return ret;
    }
    LOCK_CHNN(pstChnRes);

    MT_INFO("Create Instance\n");
    ret = MHAL_VENC_CreateInstance(pstDevRes, pstChnRes);
    if (ret != MI_SUCCESS/*MI_SUCCESS*/)
    {
        DBG_ERR("CreateInstance failed for ch%d\n", VeChn);
        goto _Exit;
    }

    //==== Set other static parameters
    ret = _MI_VENC_SetMaxResolution(pstChnRes, pstAttr);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR("CH%2d Failed to set max resolution\n", VeChn);
        goto _Exit;
    }

    ret = _MI_VENC_AllocBuf(pstChnRes);
    if(ret != MI_SUCCESS)
    {
        goto _Exit;
    }

    pstChnRes->eInputFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    ret = _MI_VENC_ConfigInstance(pstChnRes, pstAttr);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR("CH%2d Failed to config instance\n", VeChn);
        goto _Exit;
    }

    ///save channel attribute
    pstChnRes->stChnAttr = *pstAttr;
    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H265E:
            pstChnRes->u32BufSize = pstAttr->stVeAttr.stAttrH265e.u32BufSize;
            pstChnRes->bByFrame = TRUE;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            pstChnRes->u32BufSize = pstAttr->stVeAttr.stAttrJpeg.u32BufSize;
            pstChnRes->bByFrame = pstAttr->stVeAttr.stAttrJpeg.bByFrame;
            break;
        case E_MI_VENC_MODTYPE_H264E:
        default:
            pstChnRes->u32BufSize = pstAttr->stVeAttr.stAttrH264e.u32BufSize;
            pstChnRes->bByFrame = TRUE;
            break;
    }
    stVencInputAlignment.u16BufHAlignment = pstDevRes->stAlign.u32AlignW;
    //Not u32AlignH, assume HW MB is the same as u32AlignW
    stVencInputAlignment.u16BufVAlignment = pstDevRes->stAlign.u32AlignW;
    stVencInputAlignment.bClearPadding = TRUE;
    ret = mi_sys_SetInputPortBufExtConf(pstDevRes->hMiDev, VeChn, 0, &stVencInputAlignment);
    if(ret != MI_SUCCESS)
    {
        DBG_ERR("Set Input port ext conf %X\n", ret);
    }

    pstDevRes->u32MbRate += pstChnRes->u32MbRate;
    pstChnRes->u32UpdateAttrCnt = 1;
    pstChnRes->bCreate = TRUE;

    pstChnRes->u64SeqNum = 0;
    mi_venc_InitSw(&pstChnRes->stFps, 0, 1000000 + VeChn, _MI_VENC_ReportChnFps, (void*)VeChn/*pstChnRes*/);

    UNLOCK_CHNN(pstChnRes);

    DBG_INFO("Encode Chn%d Created\n", VeChn);

    return MI_SUCCESS;

_Exit:
    ret = MHAL_VENC_DestroyInstance(pstChnRes);
    if (ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to destroy instance %X\n", ret);
    }
    (void)_MI_VENC_FreeChnMemory(pstChnRes);
    UNLOCK_CHNN(pstChnRes);

    DBG_ERR("[NG] Create Encode Chn :%d err:%X\n", VeChn, ret);
    return MI_ERR_VENC_NOTREADY;
}

MI_S32 MI_VENC_IMPL_DestroyChn(MI_VENC_CHN VeChn)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_S32 ret = MI_ERR_VENC_UNDEFINED;

    DBG_ENTER();
    ret = MI_VENC_IMPL_StopRecvPic(VeChn);
    if (ret != MI_SUCCESS)
    {
        DBG_ERR("Unable to stop ch%d %X\n", VeChn, ret);
        DBG_EXIT_ERR("Unable to stop 0x%X\n", ret);
        return ret;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (pstChnRes)
    {
        int i;
        LOCK_CHNN(pstChnRes);
        if(pstChnRes->pstDevRes)
        {
            pstChnRes->pstDevRes->u32MbRate -= pstChnRes->u32MbRate;
            for (i = 0; i < MI_VENC_MAX_CHN_NUM_PER_MODULE; ++i)
            {
                if (pstChnRes->pstDevRes->astChnRes[i] == pstChnRes)
                {
                    pstChnRes->pstDevRes->astChnRes[i] = NULL;
                    break;
                }
            }
            if(i == MI_VENC_MAX_CHN_NUM_PER_MODULE)
            {
                DBG_ERR("Unable to find ch:%d to be destroyed.\n", VeChn);
            }
        }
        ret = MHAL_VENC_DestroyInstance(pstChnRes);
        if (ret != MI_SUCCESS)
        {
            DBG_ERR("Unable to destroy ch%d instance %X\n", VeChn, ret);
            goto _ExitWithChn;
        }
        (void)_MI_VENC_FreeChnMemory(pstChnRes);
        UNLOCK_CHNN(pstChnRes);
        DBG_EXIT_OK();
        return MI_SUCCESS;
    }
    //remove list?
    DBG_EXIT_ERR("Unable to stop %X\n", ret);
    return MI_ERR_VENC_CHN_NOT_STOPPED;
_ExitWithChn:
    DBG_EXIT_ERR("Unable to stop %X\n", ret);
    UNLOCK_CHNN(pstChnRes);
    return MI_ERR_VENC_CHN_NOT_STOPPED;
}

MI_S32 MI_VENC_IMPL_StartRecvPicEx(MI_VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_U32 i;
    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_CHN_NOT_STARTED;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (pstChnRes->bCreate)
    {
        if(pstChnRes->bStart == TRUE)
        {
            DBG_WRN("CH%2d is already started.\n", VeChn);
            return MI_SUCCESS;
        }
        pstChnRes->bStart = TRUE;
    }

    s32Ret = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_ON, (MHAL_VENC_Param_t*) &pstChnRes->stVencInternalBuf);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("E_MHAL_VENC_IDX_STREAM_ON\n");
        return s32Ret;
    }

    for (i = 0; i < MAX_ROI_AREA; ++i)
    {
        s32Ret = _MI_VENC_SetRoiCfg(pstChnRes, i);
        if(s32Ret != MI_SUCCESS)
        {
            return s32Ret;
        }
    }

    if (pstChnRes->pstDevRes && pstChnRes->pstDevRes->hMiDev)
    {
        s32Ret = mi_sys_EnableInputPort(pstChnRes->pstDevRes->hMiDev, VeChn, 0);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("Unable enable input port %X\n", s32Ret);
        }
        if (s32Ret == MI_SUCCESS)
        {
            s32Ret = mi_sys_EnableOutputPort(pstChnRes->pstDevRes->hMiDev, VeChn, 0);
            if (s32Ret != MI_SUCCESS)
                DBG_WRN("Unable enable output port %X", s32Ret);
        }
        if (s32Ret == MI_SUCCESS)
        {
            s32Ret = mi_sys_EnableChannel(pstChnRes->pstDevRes->hMiDev, VeChn);
            if (s32Ret != MI_SUCCESS)
                DBG_WRN("Unable enable output Ch%d %X", VeChn, s32Ret);
        }

        
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("Unable to enable ports for channel[%d] %X\n", VeChn, s32Ret);
        }
        else
        {
            DBG_INFO("ports enabled Chn[%d].\n", VeChn);
        }
    }
    return s32Ret;
}

MI_S32 MI_VENC_IMPL_StartRecvPic(MI_VENC_CHN VeChn)
{
    return MI_VENC_IMPL_StartRecvPicEx(VeChn, NULL);
}

MI_S32 MI_VENC_IMPL_StopRecvPic(MI_VENC_CHN VeChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MS_S32 s32MhalRet = 0;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    //wait for pending HW post processing
    if(0)
    {
        int cnt = 0;
        const MI_U32 u32SleepInterval = 5;
        while((pstChnRes->eBufState != MI_VENC_BUFSTATE_IDLE) && cnt < 1000)
        {
            cnt += u32SleepInterval;
            MI_VENC_MsSleep(u32SleepInterval);
        }
        if(cnt > 0)
        {
            DBG_ERR("cnt:%d\n", cnt);
        }
    }

    if (pstChnRes->pstDevRes && pstChnRes->pstDevRes->hMiDev)
    {
        s32Ret = mi_sys_DisableInputPort(pstChnRes->pstDevRes->hMiDev, VeChn, 0);
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("Unable disable input port %X\n", s32Ret);
        }
        if (s32Ret == MI_SUCCESS)
        {
            s32Ret = mi_sys_DisableOutputPort(pstChnRes->pstDevRes->hMiDev, VeChn, 0);
        }
        if (s32Ret == MI_SUCCESS)
        {
            s32Ret = mi_sys_DisableChannel(pstChnRes->pstDevRes->hMiDev, VeChn);
        }

        //Wait until wall pending input are processed.
        {

            int cnt = 0;
            const MI_U32 u32SleepInterval = 5;
            mi_sys_ChnBufInfo_t stChnBufInfo;
            while(1)
            {
                LOCK_CHNN(pstChnRes);
                s32Ret = mi_sys_GetChnBufInfo(pstChnRes->pstDevRes->hMiDev, VeChn, &stChnBufInfo);
                UNLOCK_CHNN(pstChnRes);
                if (s32Ret != MI_SUCCESS)
                {
                    DBG_ERR("Unable to get ChnBufInfo(stChnBufInfo)\n");
                    return MI_ERR_VENC_NOTREADY;
                }
                if((stChnBufInfo.au16InputPortBindConnectBufPendingCnt[0] +
                        stChnBufInfo.au16InputPortBufHoldByDrv[0] +
                        stChnBufInfo.au16InputPortUserBufPendingCnt[0] > 0) && cnt < 1000)
                {
                    cnt += u32SleepInterval;
                    MI_VENC_MsSleep(u32SleepInterval);
                }
                else
                {
                    break;
                }
            }
        }
    }

    LOCK_CHNN(pstChnRes);
    {
        MHAL_VENC_ParamInt_t stDummy;
        MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamInt_t, stDummy);
        s32MhalRet = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_IDX_STREAM_OFF, (MHAL_VENC_Param_t*) &stDummy);
    }
    if (s32MhalRet != MI_SUCCESS/*MI_SUCCESS*/)
    {
        DBG_ERR("E_MHAL_VENC_IDX_STREAM_OFF\n");
        UNLOCK_CHNN(pstChnRes);
        return s32Ret;
    }
    {
        if (s32Ret != MI_SUCCESS)
        {
            DBG_WRN("Unable to disable ports for channel[%d] %X\n", VeChn, s32Ret);
        }
        else
        {
            DBG_INFO("ports disabled Chn[%d].\n", VeChn);
            pstChnRes->bStart = FALSE;
        }
    }
    UNLOCK_CHNN(pstChnRes);
    return s32Ret;
}

MI_S32 MI_VENC_IMPL_GetStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MI_SYS_BUG_ON(pstStream == NULL);
    switch (pstChnRes->stChnAttr.stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H264E:
            //pstStream->stH264Info.eRefType;
            pstStream->stH264Info.u32UpdateAttrCnt = pstChnRes->u32UpdateAttrCnt;
            break;
        case E_MI_VENC_MODTYPE_H265E:
            //pstStream->stH265Info.eRefType;
            pstStream->stH265Info.u32UpdateAttrCnt = pstChnRes->u32UpdateAttrCnt;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            pstStream->stJpegInfo.u32UpdateAttrCnt = pstChnRes->u32UpdateAttrCnt;
            break;
        case E_MI_VENC_MODTYPE_MAX://for dummy
            break;
        default:
            DBG_ERR("Unknown channel module type %d\n", pstChnRes->stChnAttr.stVeAttr.eType);
            return MI_ERR_VENC_NOT_CONFIG;
            break;
    }

    //data hacking. Embed type into pstStream->stH264Info.eRefType.
    pstStream->stH264Info.eRefType = pstChnRes->stChnAttr.stVeAttr.eType;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_InsertUserData(MI_VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(u32Len > MAX_USER_DATA_LEN)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    if (!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(pstChnRes->stChnAttr.stVeAttr.eType != E_MI_VENC_MODTYPE_H264E &&
            pstChnRes->stChnAttr.stVeAttr.eType != E_MI_VENC_MODTYPE_H265E)
    {
        return MI_ERR_VENC_NOT_SUPPORT;
    }

    MI_VENC_CopyFromUser(pstChnRes->au8UserData, pu8Data, u32Len);
    pstChnRes->u32UserDataLen = u32Len;

    return s32Ret;
}

MI_S32 MI_VENC_IMPL_ResetChn(MI_VENC_CHN VeChn) { return MI_ERR_VENC_NOT_SUPPORT; }

MI_S32 MI_VENC_IMPL_Query(MI_VENC_CHN VeChn, MI_VENC_ChnStat_t *pstStat)
{
    MI_S32 s32MiRet;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    mi_sys_ChnBufInfo_t stChnBufInfo;

    if(NULL == pstStat)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    LOCK_CHNN(pstChnRes);
    s32MiRet = mi_sys_GetChnBufInfo(pstChnRes->pstDevRes->hMiDev, VeChn, &stChnBufInfo);
    if (s32MiRet != MI_SUCCESS)
    {
        DBG_ERR("Unable to get ChnBufInfo(stChnBufInfo)\n");
        UNLOCK_CHNN(pstChnRes);
        return MI_ERR_VENC_NOTREADY;
    }
    UNLOCK_CHNN(pstChnRes);

    /*
    stChnBufInfo.au16InputPortBufHoldByDrv[0];//PutInBuf++; before poll--;
    stChnBufInfo.au16InputPortUserBufPendingCnt[0];//PutInBuf++; before poll--;
    stChnBufInfo.au16InputPortBindConnectBufPendingCnt[0];//0
    stChnBufInfo.au16OutputPortBufInUsrFIFONum[0];//5
    stChnBufInfo.au16OutputPortBufHoldByDrv[0];//PutInBuf++; before poll/GetOutBuf--;
    stChnBufInfo.au16OutputPortBufTotalInUsedNum[0]; //PutInBuf++; put OutBuf--;
    stChnBufInfo.au16OutputPortBufUsrLockedNum[0];//GetOutBuf++; putOutBuf--;
    */

    if (stChnBufInfo.u32InputPortNum == 1)
    {
        pstStat->u32LeftPics = stChnBufInfo.au16InputPortBufHoldByDrv[0];//PutInBuf++; before poll--;
    }
    else
    {
        pstStat->u32LeftPics = (MI_U32)-1;
    }

    if (stChnBufInfo.u32OutputPortNum == 1)
    {
        pstStat->u32LeftStreamFrames = stChnBufInfo.au16OutputPortBufTotalInUsedNum[0]; //PutInBuf++; put OutBuf--;
    }
    else
    {
        pstStat->u32LeftStreamFrames = (MI_U32)-1;
    }
    pstStat->u32CurPacks = pstStat->u32LeftStreamFrames;//byFrame only
    //for StartRecvPicEx
    pstStat->u32LeftEncPics = 0;
    pstStat->u32LeftRecvPics = 0;
    //Unavailable now
    pstStat->u32LeftStreamBytes = (MI_U32)-1;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t* pstAttr)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_VENC_Attr_t stTmpAttr;
    MI_S32 s32Ret;

    if(NULL == pstAttr)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    //check if only dynamic fields are used.
    stTmpAttr = pstAttr->stVeAttr;
    switch (pstAttr->stVeAttr.eType)
    {
        case E_MI_VENC_MODTYPE_H265E:
            stTmpAttr.stAttrH265e.u32PicWidth  = pstChnRes->stChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
            stTmpAttr.stAttrH265e.u32PicHeight = pstChnRes->stChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
            break;
        case E_MI_VENC_MODTYPE_H264E:
            stTmpAttr.stAttrH264e.u32PicWidth  = pstChnRes->stChnAttr.stVeAttr.stAttrH264e.u32PicWidth;
            stTmpAttr.stAttrH264e.u32PicHeight = pstChnRes->stChnAttr.stVeAttr.stAttrH264e.u32PicHeight;
            break;
        case E_MI_VENC_MODTYPE_JPEGE:
            stTmpAttr.stAttrJpeg.u32PicWidth  = pstChnRes->stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth;
            stTmpAttr.stAttrJpeg.u32PicHeight = pstChnRes->stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight;
            break;
        case E_MI_VENC_MODTYPE_MAX:
            return MI_SUCCESS;
            break;
        default:
            return MI_ERR_VENC_NOT_SUPPORT;
            break;
    }
    if(memcmp(&stTmpAttr, &(pstChnRes->stChnAttr.stVeAttr), sizeof(MI_VENC_Attr_t)))
    {
        DBG_ERR("stVeAttr allows only PicWidth/PicHeight change\n");
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    s32Ret = _MI_VENC_ValidateResolution(pstAttr, pstChnRes->pstDevRes);
    if (s32Ret != MI_SUCCESS)
    {
        DBG_ERR("Invalid Video resolution\n");
        return s32Ret;
    }

    LOCK_CHNN(pstChnRes);
    //Now, only width and height might be changed in MI_VENC_Attr_t
    //All MI_VENC_RcAttr_t are considered as dynamic attributes.
    pstChnRes->stChnAttr = *pstAttr;
    pstChnRes->u32UpdateAttrCnt++;
    pstChnRes->bPendingChnAttr = TRUE;
    UNLOCK_CHNN(pstChnRes);

    return s32Ret;
}

MI_S32 MI_VENC_IMPL_GetChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    if(NULL == pstAttr)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    LOCK_CHNN(pstChnRes);
    *pstAttr = pstChnRes->stChnAttr;
    UNLOCK_CHNN(pstChnRes);

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_RequestIdr(MI_VENC_CHN VeChn, MI_BOOL bInstant)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    if(!pstChnRes->bStart)
    {
        return MI_ERR_VENC_CHN_NOT_STARTED;
    }

    pstChnRes->bRequestIdr = TRUE;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_EnableIdr(MI_VENC_CHN VeChn, MI_BOOL bEnableIdr)
{
    MS_S32 s32Err = MI_SUCCESS;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        DBG_WRN("Ch%2d did not be created.\n", VeChn);
        return MI_ERR_VENC_UNEXIST;
    }

    pstChnRes->bEnableIdr = bEnableIdr;
    pstChnRes->bRequestEnableIdr = TRUE;
    return s32Err;
}

MI_S32 MI_VENC_IMPL_SetRoiCfg(MI_VENC_CHN VeChn, MI_VENC_RoiCfg_t *pstVencRoiCfg)
{
    MS_S32 s32Err = MI_SUCCESS;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MHAL_VENC_RoiCfg_t *pstMhalRoi;
    MI_U32 u32Check;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    if(!pstChnRes->bStart)
    {
        return MI_ERR_VENC_NOT_PERM;
    }

    if(pstVencRoiCfg->u32Index >= MAX_ROI_AREA)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    u32Check = pstVencRoiCfg->stRect.u32Top |
               pstVencRoiCfg->stRect.u32Left |
               pstVencRoiCfg->stRect.u32Width |
               pstVencRoiCfg->stRect.u32Height;
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        if((u32Check & (16 - 1)) != 0)
        {
            DBG_ERR("All members in H.264 ROI RECT must be 16-aligned\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        if((u32Check & (32 - 1)) != 0)
        {
            DBG_ERR("All members in H.265 ROI RECT must be 32-aligned\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
    }
    else
    {
        return MI_ERR_VENC_NOT_SUPPORT;
    }

    LOCK_CHNN(pstChnRes);
    pstMhalRoi = pstChnRes->astRoiCfg + pstVencRoiCfg->u32Index;
    MHAL_VENC_INIT_PARAM(MHAL_VENC_RoiCfg_t, *pstMhalRoi);
    pstMhalRoi->stVerCtl.u32Version = 1;//FIXME special
    pstMhalRoi->eRoiMode = E_MHAL_VENC_ROI_MODE_ROIAREA;
    pstMhalRoi->stAttrRoiArea.u32Index     = pstVencRoiCfg->u32Index;
    pstMhalRoi->stAttrRoiArea.bEnable      = pstVencRoiCfg->bEnable;
    pstMhalRoi->stAttrRoiArea.bAbsQp       = pstVencRoiCfg->bAbsQp;
    pstMhalRoi->stAttrRoiArea.s32Qp        = pstVencRoiCfg->s32Qp;
    pstMhalRoi->stAttrRoiArea.stRect.u32H  = pstVencRoiCfg->stRect.u32Height;
    pstMhalRoi->stAttrRoiArea.stRect.u32W  = pstVencRoiCfg->stRect.u32Width;
    pstMhalRoi->stAttrRoiArea.stRect.u32X  = pstVencRoiCfg->stRect.u32Left;
    pstMhalRoi->stAttrRoiArea.stRect.u32Y  = pstVencRoiCfg->stRect.u32Top;


    if(pstChnRes->bStart)
    {
        s32Err = _MI_VENC_SetRoiCfg(pstChnRes, pstVencRoiCfg->u32Index);
    }
    UNLOCK_CHNN(pstChnRes);

    return s32Err;
}

MI_S32 MI_VENC_IMPL_GetRoiCfg(MI_VENC_CHN VeChn, MI_U32 u32Index, MI_VENC_RoiCfg_t *pstVencRoiCfg)
{
    MHAL_VENC_RoiCfg_t stVencRoiCfg;
    MS_S32 s32Err;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_ROI;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstVencRoiCfg)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    if(pstVencRoiCfg->u32Index >= MAX_ROI_AREA)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_RoiCfg_t, stVencRoiCfg);
    MHAL_VENC_SET_TMP_VER(stVencRoiCfg);
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eMhalIndex = E_MHAL_VENC_264_ROI;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eMhalIndex = E_MHAL_VENC_265_ROI;
    }
    stVencRoiCfg.stAttrRoiArea.u32Index = u32Index;
    stVencRoiCfg.eRoiMode = E_MHAL_VENC_ROI_MODE_ROIAREA;
    s32Err = MHAL_VENC_GetParam(pstChnRes, eMhalIndex, &stVencRoiCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstVencRoiCfg->u32Index = stVencRoiCfg.stAttrRoiArea.u32Index;
    pstVencRoiCfg->bEnable = stVencRoiCfg.stAttrRoiArea.bEnable;
    pstVencRoiCfg->s32Qp   = stVencRoiCfg.stAttrRoiArea.s32Qp;
    pstVencRoiCfg->bAbsQp = stVencRoiCfg.stAttrRoiArea.bAbsQp;
    pstVencRoiCfg->stRect.u32Height = stVencRoiCfg.stAttrRoiArea.stRect.u32H;
    pstVencRoiCfg->stRect.u32Width = stVencRoiCfg.stAttrRoiArea.stRect.u32W;
    pstVencRoiCfg->stRect.u32Left = stVencRoiCfg.stAttrRoiArea.stRect.u32X;
    pstVencRoiCfg->stRect.u32Top = stVencRoiCfg.stAttrRoiArea.stRect.u32Y;

    return MI_SUCCESS;
}

static MI_S32 _MI_VENC_GetRoiCfg(MI_VENC_ChnRes_t *pstChnRes, MHAL_VENC_RoiMode_e eRoiMode, MHAL_VENC_RoiCfg_t *pstMhalRoiCfg)
{
    MS_S32 s32Err;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_ROI;

    if(NULL == pstMhalRoiCfg || NULL == pstChnRes)
    {
        return MI_ERR_VENC_NULL_PTR;
    }

    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_RoiCfg_t, *pstMhalRoiCfg);
    MHAL_VENC_SET_TMP_VER(*pstMhalRoiCfg);
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eMhalIndex = E_MHAL_VENC_264_ROI;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eMhalIndex = E_MHAL_VENC_265_ROI;
    }
    pstMhalRoiCfg->eRoiMode = eRoiMode;
    s32Err = MHAL_VENC_GetParam(pstChnRes, eMhalIndex, pstMhalRoiCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t * pstRoiBgFrmRate)
{
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MHAL_VENC_RoiCfg_t stVencRoiCfg;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_ROI;

    if(NULL == pstRoiBgFrmRate)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    s32Err = _MI_VENC_GetRoiCfg(pstChnRes, E_MHAL_VENC_ROI_MODE_BGFPS, &stVencRoiCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Unable to get RoiCfg %X\n", s32Err);
        return s32Err;
    }
    if(!pstChnRes->bStart)
    {
        return MI_ERR_VENC_NOT_PERM;
    }

    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eMhalIndex = E_MHAL_VENC_264_ROI;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eMhalIndex = E_MHAL_VENC_265_ROI;
    }
    else
    {
        DBG_ERR("Only H.264/H.265 are supported.\n");
        return MI_ERR_VENC_NOT_PERM;
    }
    stVencRoiCfg.RoiBgCtl.u32SrcFrmRate = pstRoiBgFrmRate->s32SrcFrmRate;
    stVencRoiCfg.RoiBgCtl.u32DstFrmRate = pstRoiBgFrmRate->s32DstFrmRate;
    s32Err = MHAL_VENC_SetParam(pstChnRes, eMhalIndex, &stVencRoiCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetRoiBgFrameRate(MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate)
{
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MHAL_VENC_RoiCfg_t stVencRoiCfg;

    if(NULL == pstRoiBgFrmRate)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    s32Err = _MI_VENC_GetRoiCfg(pstChnRes, E_MHAL_VENC_ROI_MODE_BGFPS, &stVencRoiCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Unable to get RoiCfg %X\n", s32Err);
        return s32Err;
    }

    pstRoiBgFrmRate->s32SrcFrmRate = stVencRoiCfg.RoiBgCtl.u32SrcFrmRate;
    pstRoiBgFrmRate->s32DstFrmRate = stVencRoiCfg.RoiBgCtl.u32DstFrmRate;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit)
{
    MHAL_VENC_ParamSplit_t stH264ParamSplit;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH264ParamSplit);

    stH264ParamSplit.bSplitEnable = pstSliceSplit->bSplitEnable;
    stH264ParamSplit.u32SliceRowCount = pstSliceSplit->u32SliceRowCount;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_I_SPLIT_CTL, &stH264ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

#if 0 //verified, read-back is not needed.
    //read back for temp debugging.
    DBG_TMP("set done, prepare get\n");
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH264ParamSplit);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_I_SPLIT_CTL, &stH264ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    DBG_TMP("read back en:%d: row:%d\n", stH264ParamSplit.bSplitEnable, stH264ParamSplit.u32SliceRowCount);
#endif

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit)
{
    MHAL_VENC_ParamSplit_t stH264ParamSplit;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstSliceSplit)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH264ParamSplit);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_I_SPLIT_CTL, &stH264ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstSliceSplit->bSplitEnable = stH264ParamSplit.bSplitEnable;
    pstSliceSplit->u32SliceRowCount = stH264ParamSplit.u32SliceRowCount;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred)
{
    MHAL_VENC_ParamH264InterPred_t stH264InterPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264InterPred_t, stH264InterPred);

    stH264InterPred.u32HWSize = pstH264InterPred->u32HWSize;
    stH264InterPred.u32VWSize = pstH264InterPred->u32VWSize;
    stH264InterPred.bInter16x16PredEn = pstH264InterPred->bInter16x16PredEn;
    stH264InterPred.bInter16x8PredEn = pstH264InterPred->bInter16x8PredEn;
    stH264InterPred.bInter8x16PredEn = pstH264InterPred->bInter8x16PredEn;
    stH264InterPred.bInter8x8PredEn = pstH264InterPred->bInter8x8PredEn;
    stH264InterPred.bExtedgeEn = pstH264InterPred->bExtedgeEn;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_INTER_PRED, &stH264InterPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264InterPred_t *pstH264InterPred)
{
    MHAL_VENC_ParamH264InterPred_t stH264InterPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264InterPred)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264InterPred_t, stH264InterPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_INTER_PRED, &stH264InterPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264InterPred->u32HWSize = stH264InterPred.u32HWSize;
    pstH264InterPred->u32VWSize = stH264InterPred.u32VWSize;
    pstH264InterPred->bInter16x16PredEn = stH264InterPred.bInter16x16PredEn;
    pstH264InterPred->bInter16x8PredEn = stH264InterPred.bInter16x8PredEn;
    pstH264InterPred->bInter8x16PredEn = stH264InterPred.bInter8x16PredEn;
    pstH264InterPred->bInter8x8PredEn = stH264InterPred.bInter8x8PredEn;
    pstH264InterPred->bExtedgeEn = stH264InterPred.bExtedgeEn;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred)
{
    MHAL_VENC_ParamH264IntraPred_t stH264IntraPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264IntraPred_t, stH264IntraPred);

    stH264IntraPred.bIntra16x16PredEn = pstH264IntraPred->bIntra16x16PredEn;
    stH264IntraPred.bIntraNxNPredEn = pstH264IntraPred->bIntraNxNPredEn;
    stH264IntraPred.constrained_intra_pred_flag = pstH264IntraPred->constrained_intra_pred_flag;
    stH264IntraPred.bIpcmEn = pstH264IntraPred->bIpcmEn;
    stH264IntraPred.u32Intra16x16Penalty = pstH264IntraPred->u32Intra16x16Penalty;
    stH264IntraPred.u32Intra4x4Penalty = pstH264IntraPred->u32Intra4x4Penalty;
    stH264IntraPred.bIntraPlanarPenalty = pstH264IntraPred->bIntraPlanarPenalty;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_INTRA_PRED, &stH264IntraPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH264IntraPred_t *pstH264IntraPred)
{
    MHAL_VENC_ParamH264IntraPred_t stH264IntraPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264IntraPred)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264IntraPred_t, stH264IntraPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_INTRA_PRED, &stH264IntraPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264IntraPred->bIntra16x16PredEn = stH264IntraPred.bIntra16x16PredEn;
    pstH264IntraPred->bIntraNxNPredEn = stH264IntraPred.bIntraNxNPredEn;
    pstH264IntraPred->constrained_intra_pred_flag = stH264IntraPred.constrained_intra_pred_flag;
    pstH264IntraPred->bIpcmEn = stH264IntraPred.bIpcmEn;
    pstH264IntraPred->u32Intra16x16Penalty = stH264IntraPred.u32Intra16x16Penalty;
    pstH264IntraPred->u32Intra4x4Penalty = stH264IntraPred.u32Intra4x4Penalty;
    pstH264IntraPred->bIntraPlanarPenalty = stH264IntraPred.bIntraPlanarPenalty;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans)
{
    MHAL_VENC_ParamH264Trans_t stH264Trans;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Trans_t, stH264Trans);

    stH264Trans.u32IntraTransMode = pstH264Trans->u32IntraTransMode;
    stH264Trans.u32InterTransMode = pstH264Trans->u32InterTransMode;
    stH264Trans.s32ChromaQpIndexOffset = pstH264Trans->s32ChromaQpIndexOffset;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_TRANS, &stH264Trans);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans)
{
    MHAL_VENC_ParamH264Trans_t stH264Trans;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264Trans)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Trans_t, stH264Trans);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_TRANS, &stH264Trans);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264Trans->u32IntraTransMode = stH264Trans.u32IntraTransMode;
    pstH264Trans->u32InterTransMode = stH264Trans.u32InterTransMode;
    pstH264Trans->s32ChromaQpIndexOffset = stH264Trans.s32ChromaQpIndexOffset;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc)
{
    MHAL_VENC_ParamH264Entropy_t stH264Entropy;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Entropy_t, stH264Entropy);

    stH264Entropy.u32EntropyEncModeI = pstH264EntropyEnc->u32EntropyEncModeI;
    stH264Entropy.u32EntropyEncModeP = pstH264EntropyEnc->u32EntropyEncModeP;
    //DBG_TMP("to :%d\n", stH264Entropy.u32EntropyEncModeI);
    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_ENTROPY, &stH264Entropy);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

#if 0 //verified, read-back is not needed.
    //read back for temp debugging.
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Entropy_t, stH264Entropy);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_ENTROPY, &stH264Entropy);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    //DBG_TMP("DBG read back :%d\n", stH264Entropy.u32EntropyEncModeI);
#endif
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264Entropy(MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc)
{
    MHAL_VENC_ParamH264Entropy_t stH264Entropy;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264EntropyEnc)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Entropy_t, stH264Entropy);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_ENTROPY, &stH264Entropy);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264EntropyEnc->u32EntropyEncModeI = stH264Entropy.u32EntropyEncModeI;
    pstH264EntropyEnc->u32EntropyEncModeP = stH264Entropy.u32EntropyEncModeP;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk)
{
    MHAL_VENC_ParamH264Dblk_t stH264Dblk;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Dblk_t, stH264Dblk);

    stH264Dblk.disable_deblocking_filter_idc = pstH264Dblk->disable_deblocking_filter_idc;
    stH264Dblk.slice_alpha_c0_offset_div2 = pstH264Dblk->slice_alpha_c0_offset_div2;
    stH264Dblk.slice_beta_offset_div2 = pstH264Dblk->slice_beta_offset_div2;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_DBLK, &stH264Dblk);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk)
{
    MHAL_VENC_ParamH264Dblk_t stH264Dblk;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264Dblk)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Dblk_t, stH264Dblk);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_DBLK, &stH264Dblk);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264Dblk->disable_deblocking_filter_idc = stH264Dblk.disable_deblocking_filter_idc;
    pstH264Dblk->slice_alpha_c0_offset_div2 = stH264Dblk.slice_alpha_c0_offset_div2;
    pstH264Dblk->slice_beta_offset_div2 = stH264Dblk.slice_beta_offset_div2;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit)
{
    MHAL_VENC_ParamSplit_t stH265ParamSplit;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH265ParamSplit);

    stH265ParamSplit.bSplitEnable = pstSliceSplit->bSplitEnable;
    stH265ParamSplit.u32SliceRowCount = pstSliceSplit->u32SliceRowCount;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_I_SPLIT_CTL, &stH265ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

#if 0 //verified, read-back is not needed.
    //read back for temp debugging.
    DBG_TMP("en:%d nRows:%d set done, prepare get \n", stH265ParamSplit.bSplitEnable, stH265ParamSplit.u32SliceRowCount);
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH265ParamSplit);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_I_SPLIT_CTL, &stH265ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    DBG_TMP("read back en:%d: row:%d\n", stH265ParamSplit.bSplitEnable, stH265ParamSplit.u32SliceRowCount);
#endif

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit)
{
    MHAL_VENC_ParamSplit_t stH265ParamSplit;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstSliceSplit)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamSplit_t, stH265ParamSplit);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_I_SPLIT_CTL, &stH265ParamSplit);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstSliceSplit->bSplitEnable = stH265ParamSplit.bSplitEnable;
    pstSliceSplit->u32SliceRowCount = stH265ParamSplit.u32SliceRowCount;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred)
{
    MHAL_VENC_ParamH265InterPred_t stH265InterPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265InterPred_t, stH265InterPred);

    stH265InterPred.u32HWSize = pstH265InterPred->u32HWSize;
    stH265InterPred.u32VWSize = pstH265InterPred->u32VWSize;
    stH265InterPred.bInter16x16PredEn = pstH265InterPred->bInter16x16PredEn;
    stH265InterPred.bInter16x8PredEn = pstH265InterPred->bInter16x8PredEn;
    stH265InterPred.bInter8x16PredEn = pstH265InterPred->bInter8x16PredEn;
    stH265InterPred.bInter8x8PredEn = pstH265InterPred->bInter8x8PredEn;
    stH265InterPred.bExtedgeEn = pstH265InterPred->bExtedgeEn;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_INTER_PRED, &stH265InterPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
#if 0
    //read back for temp debugging.
    DBG_TMP("set done, prepare get\n");
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265InterPred_t, stH265InterPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_INTER_PRED, &stH265InterPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    DBG_TMP("read back X:%d Y:%d 16x16PredEn:%d 16x16PredEn:%d 16x8PredEn:%d 8x16PredEn:%d 8x8PredEn:%d  bExtedgeEn:%d\n",
            stH265InterPred.u32HWSize = pstH265InterPred->u32HWSize,
            stH265InterPred.u32VWSize = pstH265InterPred->u32VWSize,
            stH265InterPred.bInter16x16PredEn = pstH265InterPred->bInter16x16PredEn,
            stH265InterPred.bInter16x8PredEn = pstH265InterPred->bInter16x8PredEn,
            stH265InterPred.bInter8x16PredEn = pstH265InterPred->bInter8x16PredEn,
            stH265InterPred.bInter8x8PredEn = pstH265InterPred->bInter8x8PredEn,
            stH265InterPred.bExtedgeEn = pstH265InterPred->bExtedgeEn);
#endif
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265InterPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265InterPred_t *pstH265InterPred)
{
    MHAL_VENC_ParamH265InterPred_t stH265InterPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH265InterPred)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265InterPred_t, stH265InterPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_INTER_PRED, &stH265InterPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH265InterPred->u32HWSize = stH265InterPred.u32HWSize;
    pstH265InterPred->u32VWSize = stH265InterPred.u32VWSize;
    pstH265InterPred->bInter16x16PredEn = stH265InterPred.bInter16x16PredEn;
    pstH265InterPred->bInter16x8PredEn = stH265InterPred.bInter16x8PredEn;
    pstH265InterPred->bInter8x16PredEn = stH265InterPred.bInter8x16PredEn;
    pstH265InterPred->bInter8x8PredEn = stH265InterPred.bInter8x8PredEn;
    pstH265InterPred->bExtedgeEn = stH265InterPred.bExtedgeEn;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred)
{
    MHAL_VENC_ParamH265IntraPred_t stH265IntraPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265IntraPred_t, stH265IntraPred);

    stH265IntraPred.u32Intra32x32Penalty = pstH265IntraPred->u32Intra32x32Penalty;
    stH265IntraPred.u32Intra16x16Penalty = pstH265IntraPred->u32Intra16x16Penalty;
    stH265IntraPred.u32Intra8x8Penalty = pstH265IntraPred->u32Intra8x8Penalty;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_INTRA_PRED, &stH265IntraPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
#if 0
    //read back for temp debugging.
    DBG_TMP("set done, prepare "
            "u32Intra32x32Penalty:%d u32Intra16x16Penalty:%d 8x8PredEn:%d  \n",
            stH265IntraPred.u32Intra32x32Penalty,
            stH265IntraPred.u32Intra16x16Penalty,
            stH265IntraPred.u32Intra8x8Penalty);
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265IntraPred_t, stH265IntraPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_INTRA_PRED, &stH265IntraPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    DBG_TMP("read back u32Intra32x32Penalty:%d u32Intra16x16Penalty:%d 8x8PredEn:%d  \n",
            stH265IntraPred.u32Intra32x32Penalty,
            stH265IntraPred.u32Intra16x16Penalty,
            stH265IntraPred.u32Intra8x8Penalty);
#endif
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265IntraPred(MI_VENC_CHN VeChn, MI_VENC_ParamH265IntraPred_t *pstH265IntraPred)
{
    MHAL_VENC_ParamH265IntraPred_t stH265IntraPred;
    MS_S32 s32Err;
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH265IntraPred)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265IntraPred_t, stH265IntraPred);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_INTRA_PRED, &stH265IntraPred);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH265IntraPred->u32Intra32x32Penalty = stH265IntraPred.u32Intra32x32Penalty;
    pstH265IntraPred->u32Intra16x16Penalty = stH265IntraPred.u32Intra16x16Penalty;
    pstH265IntraPred->u32Intra8x8Penalty = stH265IntraPred.u32Intra8x8Penalty;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans)
{
    MHAL_VENC_ParamH265Trans_t stH265Trans;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Trans_t, stH265Trans);

    stH265Trans.u32IntraTransMode = pstH265Trans->u32IntraTransMode;
    stH265Trans.u32InterTransMode = pstH265Trans->u32InterTransMode;
    stH265Trans.s32ChromaQpIndexOffset = pstH265Trans->s32ChromaQpIndexOffset;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_TRANS, &stH265Trans);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265Trans(MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans)
{
    MHAL_VENC_ParamH265Trans_t stH265Trans;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH265Trans)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Trans_t, stH265Trans);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_TRANS, &stH265Trans);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH265Trans->u32IntraTransMode = stH265Trans.u32IntraTransMode;
    pstH265Trans->u32InterTransMode = stH265Trans.u32InterTransMode;
    pstH265Trans->s32ChromaQpIndexOffset = stH265Trans.s32ChromaQpIndexOffset;

    return MI_SUCCESS;
}


MI_S32 MI_VENC_IMPL_SetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk)
{
    MHAL_VENC_ParamH265Dblk_t stH265Dblk;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Dblk_t, stH265Dblk);

    stH265Dblk.disable_deblocking_filter_idc = pstH265Dblk->disable_deblocking_filter_idc;
    stH265Dblk.slice_tc_offset_div2 = pstH265Dblk->slice_tc_offset_div2;
    stH265Dblk.slice_beta_offset_div2 = pstH265Dblk->slice_beta_offset_div2;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_DBLK, &stH265Dblk);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265Dblk(MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk)
{
    MHAL_VENC_ParamH265Dblk_t stH265Dblk;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH265Dblk)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Dblk_t, stH265Dblk);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_DBLK, &stH265Dblk);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH265Dblk->disable_deblocking_filter_idc = stH265Dblk.disable_deblocking_filter_idc;
    pstH265Dblk->slice_tc_offset_div2 = stH265Dblk.slice_tc_offset_div2;
    pstH265Dblk->slice_beta_offset_div2 = stH265Dblk.slice_beta_offset_div2;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
    MHAL_VENC_RcInfo_t stRc;
    MS_S32 s32Err;
    int i;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stRc);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    stRc.stAttrMJPGRc.u32Qfactor = pstJpegParam->u32Qfactor;
    for (i = 0; i < sizeof(stRc.stAttrMJPGRc.u8YQt)/sizeof(stRc.stAttrMJPGRc.u8YQt[0]); ++i)
    {
        stRc.stAttrMJPGRc.u8YQt[i] = pstJpegParam->au8YQt[i];
        stRc.stAttrMJPGRc.u8CbCrQt[i] = pstJpegParam->au8CbCrQt[i];
    }

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetJpegParam(MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
    MHAL_VENC_RcInfo_t stRc;
    MS_S32 s32Err;
    int i;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstJpegParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_RcInfo_t, stRc);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_JPEG_RC, &stRc);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstJpegParam->u32Qfactor = stRc.stAttrMJPGRc.u32Qfactor;
    pstJpegParam->u32McuPerEcs = 0;
    for(i = 0; i < (sizeof(stRc.stAttrMJPGRc.u8YQt) / sizeof(stRc.stAttrMJPGRc.u8YQt[0])); ++i)
    {
        pstJpegParam->au8YQt[i] = stRc.stAttrMJPGRc.u8YQt[i];
        pstJpegParam->au8CbCrQt[i] = stRc.stAttrMJPGRc.u8CbCrQt[i];
    }

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui)
{
    MHAL_VENC_ParamH264Vui_t stH264Vui;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Vui_t, stH264Vui);
    stH264Vui.stVerCtl.u32Version = 1;//FIXME special

    stH264Vui.stVuiAspectRatio.u16SarHeight = pstH264Vui->stVuiAspectRatio.u16SarHeight;
    stH264Vui.stVuiAspectRatio.u16SarWidth = pstH264Vui->stVuiAspectRatio.u16SarWidth;
#if MHAL_VENC_ParamH264Vui_t_ver == 1
    stH264Vui.stVuiAspectRatio.u8AspectRatioIdc = pstH264Vui->stVuiAspectRatio.u8AspectRatioIdc;
    stH264Vui.stVuiAspectRatio.u8AspectRatioInfoPresentFlag = pstH264Vui->stVuiAspectRatio.u8AspectRatioInfoPresentFlag;
    stH264Vui.stVuiAspectRatio.u8OverscanAppropriateFlag = pstH264Vui->stVuiAspectRatio.u8OverscanAppropriateFlag;
    stH264Vui.stVuiAspectRatio.u8OverscanInfoPresentFlag = pstH264Vui->stVuiAspectRatio.u8OverscanInfoPresentFlag;

    stH264Vui.stVuiTimeInfo.u32NumUnitsInTick = pstH264Vui->stVuiTimeInfo.u32NumUnitsInTick;
    stH264Vui.stVuiTimeInfo.u32TimeScale = pstH264Vui->stVuiTimeInfo.u32TimeScale;
    stH264Vui.stVuiTimeInfo.u8FixedFrameRateFlag = pstH264Vui->stVuiTimeInfo.u8FixedFrameRateFlag;
#endif
    stH264Vui.stVuiTimeInfo.u8TimingInfoPresentFlag = pstH264Vui->stVuiTimeInfo.u8TimingInfoPresentFlag;

    stH264Vui.stVuiVideoSignal.u8VideoSignalTypePresentFlag = pstH264Vui->stVuiVideoSignal.u8VideosignalTypePresentFlag;
    stH264Vui.stVuiVideoSignal.u8VideoFormat = pstH264Vui->stVuiVideoSignal.u8VideoFormat;
    stH264Vui.stVuiVideoSignal.u8VideoFullRangeFlag = pstH264Vui->stVuiVideoSignal.u8VideoFullRangeFlag;
    stH264Vui.stVuiVideoSignal.u8ColourDescriptionPresentFlag = pstH264Vui->stVuiVideoSignal.u8ColourDescriptionPresentFlag;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_264_VUI, &stH264Vui);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH264Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui)
{
    MHAL_VENC_ParamH264Vui_t stH264Vui;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH264Vui)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH264Vui_t, stH264Vui);
    stH264Vui.stVerCtl.u32Version = 1;//FIXME special
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_264_VUI, &stH264Vui);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH264Vui->stVuiAspectRatio.u16SarHeight = stH264Vui.stVuiAspectRatio.u16SarHeight;
    pstH264Vui->stVuiAspectRatio.u16SarWidth = stH264Vui.stVuiAspectRatio.u16SarWidth;
#if MHAL_VENC_ParamH264Vui_t_ver == 1
    pstH264Vui->stVuiAspectRatio.u8AspectRatioIdc = stH264Vui.stVuiAspectRatio.u8AspectRatioIdc;
    pstH264Vui->stVuiAspectRatio.u8AspectRatioInfoPresentFlag = stH264Vui.stVuiAspectRatio.u8AspectRatioInfoPresentFlag;
    pstH264Vui->stVuiAspectRatio.u8OverscanAppropriateFlag = stH264Vui.stVuiAspectRatio.u8OverscanAppropriateFlag;
    pstH264Vui->stVuiAspectRatio.u8OverscanInfoPresentFlag = stH264Vui.stVuiAspectRatio.u8OverscanInfoPresentFlag;

    pstH264Vui->stVuiTimeInfo.u32NumUnitsInTick = stH264Vui.stVuiTimeInfo.u32NumUnitsInTick;
    pstH264Vui->stVuiTimeInfo.u32TimeScale = stH264Vui.stVuiTimeInfo.u32TimeScale;
    pstH264Vui->stVuiTimeInfo.u8FixedFrameRateFlag = stH264Vui.stVuiTimeInfo.u8FixedFrameRateFlag;
#endif
    pstH264Vui->stVuiTimeInfo.u8TimingInfoPresentFlag = stH264Vui.stVuiTimeInfo.u8TimingInfoPresentFlag;

    pstH264Vui->stVuiVideoSignal.u8VideosignalTypePresentFlag = stH264Vui.stVuiVideoSignal.u8VideoSignalTypePresentFlag;
    pstH264Vui->stVuiVideoSignal.u8VideoFormat = stH264Vui.stVuiVideoSignal.u8VideoFormat;
    pstH264Vui->stVuiVideoSignal.u8VideoFullRangeFlag = stH264Vui.stVuiVideoSignal.u8VideoFullRangeFlag;
    pstH264Vui->stVuiVideoSignal.u8ColourDescriptionPresentFlag = stH264Vui.stVuiVideoSignal.u8ColourDescriptionPresentFlag;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui)
{
    MHAL_VENC_ParamH265Vui_t stH265Vui;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Vui_t, stH265Vui);

    stH265Vui.stVuiAspectRatio.u16SarHeight = pstH265Vui->stVuiAspectRatio.u16SarHeight;
    stH265Vui.stVuiAspectRatio.u16SarWidth = pstH265Vui->stVuiAspectRatio.u16SarWidth;
    stH265Vui.stVuiTimeInfo.u8TimingInfoPresentFlag = pstH265Vui->stVuiTimeInfo.u8TimingInfoPresentFlag;

    stH265Vui.stVuiVideoSignal.u8VideoSignalTypePresentFlag = pstH265Vui->stVuiVideoSignal.u8VideosignalTypePresentFlag;
    stH265Vui.stVuiVideoSignal.u8VideoFormat = pstH265Vui->stVuiVideoSignal.u8VideoFormat;
    stH265Vui.stVuiVideoSignal.u8VideoFullRangeFlag = pstH265Vui->stVuiVideoSignal.u8VideoFullRangeFlag;
    stH265Vui.stVuiVideoSignal.u8ColourDescriptionPresentFlag = pstH265Vui->stVuiVideoSignal.u8ColourDescriptionPresentFlag;

    s32Err = MHAL_VENC_SetParam(pstChnRes, E_MHAL_VENC_265_VUI, &stH265Vui);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetH265Vui(MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui)
{
    MHAL_VENC_ParamH265Vui_t stH265Vui;
    MS_S32 s32Err;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstH265Vui)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamH265Vui_t, stH265Vui);
    s32Err = MHAL_VENC_GetParam(pstChnRes, E_MHAL_VENC_265_VUI, &stH265Vui);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstH265Vui->stVuiAspectRatio.u16SarHeight = stH265Vui.stVuiAspectRatio.u16SarHeight;
    pstH265Vui->stVuiAspectRatio.u16SarWidth = stH265Vui.stVuiAspectRatio.u16SarWidth;
    pstH265Vui->stVuiTimeInfo.u8TimingInfoPresentFlag = stH265Vui.stVuiTimeInfo.u8TimingInfoPresentFlag;

    pstH265Vui->stVuiVideoSignal.u8VideosignalTypePresentFlag = stH265Vui.stVuiVideoSignal.u8VideoSignalTypePresentFlag;
    pstH265Vui->stVuiVideoSignal.u8VideoFormat = stH265Vui.stVuiVideoSignal.u8VideoFormat;
    pstH265Vui->stVuiVideoSignal.u8VideoFullRangeFlag = stH265Vui.stVuiVideoSignal.u8VideoFullRangeFlag;
    pstH265Vui->stVuiVideoSignal.u8ColourDescriptionPresentFlag = stH265Vui.stVuiVideoSignal.u8ColourDescriptionPresentFlag;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg)
{
    MHAL_VENC_CropCfg_t stCropCfg;
    MS_S32 s32Err;
    MI_VENC_Rect_t *pstRect;
    MHAL_VENC_IDX eType = E_MAHL_VENC_CROP;
    MI_VENC_DevRes_t *pstDevRes;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(pstChnRes->bStart)
    {
        return MI_ERR_VENC_NOT_PERM;
    }
    pstDevRes = pstChnRes->pstDevRes;
    if (pstDevRes == NULL)
    {
        return MI_ERR_VENC_NOTREADY;
    }


    pstRect = &pstCropCfg->stRect;
#if 1
    if (pstDevRes->stAlign.u32AlignCropW == 0 || pstDevRes->stAlign.u32AlignCropH == 0
            || pstDevRes->stAlign.u32AlignCropX == 0 || pstDevRes->stAlign.u32AlignCropY == 0)
    {
        DBG_ERR("The crop of this CH%d is not set\n", VeChn);
        return MI_ERR_VENC_NOT_SUPPORT;
    }
    if((pstRect->u32Width & (pstDevRes->stAlign.u32AlignCropW - 1)) != 0)
    {
        DBG_ERR("u32Width should be a multiple of %d.\n", pstDevRes->stAlign.u32AlignCropW);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    if((pstRect->u32Height & (pstDevRes->stAlign.u32AlignCropH - 1)) != 0)
    {
        DBG_ERR("u32Height should be a multiple of %d.\n", pstDevRes->stAlign.u32AlignCropH);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    if((pstRect->u32Left & (pstDevRes->stAlign.u32AlignCropX - 1)) != 0)
    {
        DBG_ERR("u32Left should be a multiple of %d.\n", pstDevRes->stAlign.u32AlignCropX);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    if((pstRect->u32Top & (pstDevRes->stAlign.u32AlignCropY - 1)) != 0)
    {
        DBG_ERR("u32Top should be a multiple of %d.\n", pstDevRes->stAlign.u32AlignCropY);
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eType = E_MHAL_VENC_264_CROP;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eType = E_MHAL_VENC_265_CROP;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        eType = E_MHAL_VENC_JPEG_CROP;
    }
    else
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
#else
    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        if(((pstRect->u32Height | pstRect->u32Width) & (16-1)) != 0)
        {
            DBG_ERR("u32Height or u32Width should be a multiple of 16.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        if(((pstRect->u32Top) & (32-1)) != 0)
        {
            DBG_ERR("u32Top should be a multiple of 32.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        if(((pstRect->u32Left) & (256-1)) != 0)
        {
            DBG_ERR("u32Left should be a multiple of 256.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        eType = E_MHAL_VENC_264_CROP;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        if(((pstRect->u32Height | pstRect->u32Width) & (32-1)) != 0)
        {
            DBG_ERR("u32Height or u32Width should be a multiple of 32.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        if(((pstRect->u32Top) & (2-1)) != 0)
        {
            DBG_ERR("u32Top should be a multiple of 2.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        if(((pstRect->u32Left) & (16-1)) != 0)
        {
            DBG_ERR("u32Left should be a multiple of 16.\n");
            return MI_ERR_VENC_ILLEGAL_PARAM;
        }
        eType = E_MHAL_VENC_265_CROP;
    }
    else
    {
        return MI_ERR_VENC_NOT_SUPPORT;
    }
#endif
    MHAL_VENC_INIT_PARAM(MHAL_VENC_CropCfg_t, stCropCfg);

    stCropCfg.bEnable = pstCropCfg->bEnable;
    stCropCfg.stRect.u32X = pstCropCfg->stRect.u32Left;
    stCropCfg.stRect.u32Y = pstCropCfg->stRect.u32Top;
    stCropCfg.stRect.u32W = pstCropCfg->stRect.u32Width;
    stCropCfg.stRect.u32H = pstCropCfg->stRect.u32Height;

    s32Err = MHAL_VENC_SetParam(pstChnRes, eType, &stCropCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetCrop(MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg)
{
    MHAL_VENC_CropCfg_t stCropCfg;
    MS_S32 s32Err;
    MHAL_VENC_IDX eType = E_MAHL_VENC_CROP;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstCropCfg)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    eType = _astMhalModuleCfg[pstChnRes->stChnAttr.stVeAttr.eType].eCrop;


    MHAL_VENC_INIT_PARAM(MHAL_VENC_CropCfg_t, stCropCfg);
    s32Err = MHAL_VENC_GetParam(pstChnRes, eType, &stCropCfg);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstCropCfg->bEnable          = stCropCfg.bEnable;
    pstCropCfg->stRect.u32Left   = stCropCfg.stRect.u32X;
    pstCropCfg->stRect.u32Top    = stCropCfg.stRect.u32Y;
    pstCropCfg->stRect.u32Width  = stCropCfg.stRect.u32W;
    pstCropCfg->stRect.u32Height = stCropCfg.stRect.u32H;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetChnDevid(MI_VENC_CHN VeChn, MI_U32 *pu32Devid)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(pstChnRes->bCreate != TRUE)
    {
        return MI_ERR_VENC_UNEXIST;
    }
    if(pstChnRes->pstDevRes == NULL)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    *pu32Devid = pstChnRes->pstDevRes->u32DevId;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetRefParam(MI_VENC_CHN VeChn, MI_VENC_ParamRef_t * pstRefParam)
{
    MHAL_VENC_ParamRef_t stRefParam;
    MS_S32 s32Err;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_IDX_MAX;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstRefParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eMhalIndex = E_MHAL_VENC_264_REF;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eMhalIndex = E_MHAL_VENC_265_REF;
    }
    else
    {
        return MI_ERR_VENC_NOT_SUPPORT;
    }
    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamRef_t, stRefParam);

    stRefParam.u32Base = pstRefParam->u32Base;
    stRefParam.u32Enhance = pstRefParam->u32Enhance;
    stRefParam.bEnablePred = pstRefParam->bEnablePred;

    s32Err = MHAL_VENC_SetParam(pstChnRes, eMhalIndex, &stRefParam);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Set Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    LOCK_CHNN(pstChnRes);
    s32Err = _MI_VENC_SetMaxResolution(pstChnRes, &pstChnRes->stChnAttr);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("CH%2d Failed to set max resolution\n", VeChn);
    }
    if(s32Err == MI_SUCCESS)
    {
        (void)_MI_VENC_FreeChnMemory(pstChnRes);
        s32Err = _MI_VENC_AllocBuf(pstChnRes);
        if(s32Err != MI_SUCCESS)
        {
            DBG_ERR("CH%2d Failed to re-allocate :%X\n", VeChn, s32Err);
            (void)_MI_VENC_FreeChnMemory(pstChnRes);
        }
    }
    if(s32Err == MI_SUCCESS)
    {
        s32Err = _MI_VENC_ConfigInstance(pstChnRes, &pstChnRes->stChnAttr);
    }
    UNLOCK_CHNN(pstChnRes);

    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Realloc buffer fail\n");
        MI_VENC_IMPL_DestroyChn(VeChn);
    }
    return s32Err;
}

MI_S32 MI_VENC_IMPL_GetRefParam(MI_VENC_CHN VeChn, MI_VENC_ParamRef_t * pstRefParam)
{
    MHAL_VENC_ParamRef_t stRefParam;
    MS_S32 s32Err;
    MHAL_VENC_IDX eMhalIndex = E_MHAL_VENC_IDX_MAX;

    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstRefParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        eMhalIndex = E_MHAL_VENC_264_REF;
    }
    else if(pstChnRes->stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        eMhalIndex = E_MHAL_VENC_265_REF;
    }
    else
    {
        return MI_ERR_VENC_NOT_SUPPORT;
    }

    MHAL_VENC_INIT_PARAM(MHAL_VENC_ParamRef_t, stRefParam);
    s32Err = MHAL_VENC_GetParam(pstChnRes, eMhalIndex, &stRefParam);
    if(s32Err != MI_SUCCESS)
    {
        DBG_ERR("Get Param err 0x%X.\n", s32Err);
        return MI_ERR_VENC_UNDEFINED;
    }

    pstRefParam->u32Base = stRefParam.u32Base;
    pstRefParam->u32Enhance = stRefParam.u32Enhance;
    pstRefParam->bEnablePred = stRefParam.bEnablePred;

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstSuperFrmParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(pstSuperFrmParam->eSuperFrmMode >= E_MI_VENC_SUPERFRM_MAX)
    {
        DBG_ERR("Unsupported mode :%X\n", pstSuperFrmParam->eSuperFrmMode);
        return MI_ERR_VENC_NOT_SUPPORT;
    }
    LOCK_CHNN(pstChnRes);
    pstChnRes->stSuperFrame = *pstSuperFrmParam;
    UNLOCK_CHNN(pstChnRes);

    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetSuperFrameCfg(MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstSuperFrmParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    *pstSuperFrmParam = pstChnRes->stSuperFrame;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstFrmLostParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    //if(pstFrmLostParam->u32FrmLostBpsThr < 64000 || pstFrmLostParam->u32FrmLostBpsThr > 163840000)
    //if(pstFrmLostParam->u32EncFrmGaps > 5 && pstFrmLostParam < pstChnRes->stChnAttr.stRcAttr.stAttrH264Abr.u32Gop)
    if(pstFrmLostParam->eFrmLostMode != E_MI_VENC_FRMLOST_NORMAL)
    {
        DBG_ERR("Only E_MI_VENC_FRMLOST_NORMAL is supported\n");
        return MI_ERR_VENC_NOT_SUPPORT;
    }
    switch(pstChnRes->stChnAttr.stRcAttr.eRcMode)
    {
        //case E_MI_VENC_RC_MODE_H264FIXQP:
        //case E_MI_VENC_RC_MODE_H265FIXQP:
        case E_MI_VENC_RC_MODE_MJPEGFIXQP:
            DBG_ERR("MJPEG FIX QP is unsupported\n");
            return MI_ERR_VENC_NOT_SUPPORT;
            break;
        default:
            break;
    }
    LOCK_CHNN(pstChnRes);
    pstChnRes->stFrameLost = *pstFrmLostParam;
    memset(&pstChnRes->stFLCtx, 0, sizeof(pstChnRes->stFLCtx));
    pstChnRes->stFLCtx.u32IFrmThr =
            pstChnRes->stFLCtx.u32PFrmThr = (MI_U32)-1;//set to max U32
#if USE_FL_WINDOW
    {
        int i;
        MI_U32 u32MaxBytes = ((MI_U32) -1) / (frm_lost_win << 1);
        for (i = 0; i < frm_lost_win; ++i)
        {
            pstChnRes->stFLCtx.u32FrmBytes[i] = u32MaxBytes;
            pstChnRes->stFLCtx.u32WindowBytes += u32MaxBytes;
        }
    }
#endif
    UNLOCK_CHNN(pstChnRes);
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetFrameLostStrategy(MI_VENC_CHN VeChn, MI_VENC_ParamFrameLost_t *pstFrmLostParam)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == pstFrmLostParam)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    *pstFrmLostParam = pstChnRes->stFrameLost;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_SetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == peRcPriority)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if(!pstChnRes->bCreate)
    {
        return MI_ERR_VENC_UNEXIST;
    }

    if(*peRcPriority >= E_MI_VENC_RC_PRIORITY_MAX || *peRcPriority < E_MI_VENC_RC_PRIORITY_BITRATE_FIRST)
    {
        return MI_ERR_VENC_ILLEGAL_PARAM;
    }
    if(pstChnRes->stCtx.u32TargetBitrate == 0)
    {//Fixed QP
        DBG_WRN("Set RC priority under Fixed QP!\n");
    }
    pstChnRes->eRcPriority = *peRcPriority;
    return MI_SUCCESS;
}

MI_S32 MI_VENC_IMPL_GetRcPriority(MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;

    if(NULL == peRcPriority)
    {
        return MI_ERR_VENC_NULL_PTR;
    }
    if(!_IS_VALID_VENC_CHANNEL(VeChn))
    {
        return MI_ERR_VENC_INVALID_CHNID;
    }
    pstChnRes = _ModRes.astChnRes + VeChn;
    *peRcPriority = pstChnRes->eRcPriority;
    return MI_SUCCESS;
}

//---- Implemented in user space ----
MI_S32 MI_VENC_IMPL_SetMaxStreamCnt(MI_VENC_CHN VeChn,MI_U32 u32MaxStrmCnt) { return MI_ERR_VENC_NOT_PERM; }
MI_S32 MI_VENC_IMPL_GetMaxStreamCnt(MI_VENC_CHN VeChn,MI_U32 *pu32MaxStrmCnt) { return MI_ERR_VENC_NOT_PERM; }
MI_S32 MI_VENC_IMPL_GetFd(MI_VENC_CHN VeChn) { return MI_ERR_VENC_NOT_PERM; }
MI_S32 MI_VENC_IMPL_CloseFd(MI_VENC_CHN VeChn) { return MI_ERR_VENC_NOT_PERM; }
MI_S32 MI_VENC_IMPL_ReleaseStream(MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream)  { return MI_ERR_VENC_NOT_PERM; }

//---- Probably not be supported soon, most of them are driver features ----
//MI_ERR_VENC_NOT_SUPPORT as E_MHAL_ERR_NOT_SUPPORT for MHAL_VENC apis
MI_S32 MI_VENC_IMPL_SetRcParam(MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam) { return MI_ERR_VENC_NOT_SUPPORT; }
MI_S32 MI_VENC_IMPL_GetRcParam(MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam) { return MI_ERR_VENC_NOT_SUPPORT; }
MI_S32 MI_VENC_IMPL_SetModParam(MI_VENC_ModParam_t *pstModParam) { return MI_ERR_VENC_NOT_SUPPORT; }
MI_S32 MI_VENC_IMPL_GetModParam(MI_VENC_ModParam_t *pstModParam) { return MI_ERR_VENC_NOT_SUPPORT; }
MI_S32 MI_VENC_IMPL_SetH264IdrPicId(MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg) { return MI_ERR_VENC_NOT_SUPPORT; }
MI_S32 MI_VENC_IMPL_GetH264IdrPicId( MI_VENC_CHN VeChn, MI_VENC_H264IdrPicIdCfg_t* pstH264eIdrPicIdCfg ) { return MI_ERR_VENC_NOT_SUPPORT; }
