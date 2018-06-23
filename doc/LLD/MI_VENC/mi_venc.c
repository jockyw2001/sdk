//#include <linux/stddef.h>

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#include "mi_venc.h"
#include "mi_sys_internal.h"
#include "mi_common.h"

#include "mhal_venc.h"

#define WITH_DEV (1) /**< Set 1 Use a series of structure with Dev for example.
						  Use mi_sys_ModuleDevInfo_t instead of mi_sys_ModuleInfo_t. */

///TODO:ENABLE
#define DOWN(x) if(0){down(x);}
#define UP(x) if(0){up(x);}

#define _IS_VALID_VENC_CHANNEL(chanId)\
		(((VENC_CHN)chanId) < VENC_MAX_CHN_NUM)

#define MAX_OUTPUT_ES_SIZE (200*1024)

//-------------------------------------------------------------------------------------------------
//  Missing define for current code. As other modules pushing their code, this section should
//  not be needed.
//-------------------------------------------------------------------------------------------------
typedef struct cmdq_buffer_descript_s {
	MI_U32 uCmdBufSize;;
	MI_U32 uCmdBufSizeAlign;
	MI_U32 uMloadBufSize;
	MI_U32 uMloadBufSizeAlign;
} cmdq_buffer_descript_t;


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct MI_VENC_RegDevPriData_s
{
	MI_U32 u32Version;
} MI_VENC_RegDevPriData_t;

#if 1
typedef struct MI_VENC_MemBufInfo_s
{
	MI_PHY phyAddr;
	void *pVirAddr;
	MI_U32 u32BufSize;
} MI_VENC_MemBufInfo_t;
#else
typedef VencOut MI_VENC_MemBufInfo_t;
#endif

#define VENC_EXTRA_BUFFER (0)
typedef struct MI_VENC_DevRes_s MI_VENC_DevRes_t;
typedef struct
{
#if VENC_EXTRA_BUFFER
	MI_VENC_MemBufInfo_t stCpuMemInfo;
	MI_VENC_MemBufInfo_t stEsMemInfo;
	//MI_VENC_MemBufInfo_t stFrameMemInfo;
#endif
    MI_VENC_MemBufInfo_t stRefMemInfo;///<Internal reference frames memory
    MI_VENC_MemBufInfo_t stAlMemInfo;///<Al stands for Algorithm
	MI_BOOL bCreate;
	MI_BOOL bStart;
	MI_BOOL bSelected;///< If any buffer is selected for processing

	MI_VENC_ChnAttr_t stChnAttr;
	MI_VENC_DevRes_t *pstDevRes;///< Quick access to resources
	MHalVencInstHandle hInst;
	//VENC_StreamId stVENCStreamId;
	struct semaphore semLock;
} MI_VENC_ChnRes_t;

typedef struct MI_VENC_DevRes_s {
	//MI
	MI_BOOL bInitFlag;
	MI_BOOL bWorkTaskRun;
	MI_BOOL bIrqTaskRun;
	MI_SYS_DRV_HANDLE hMiDev;
	MI_VENC_ChnRes_t *astChnRes[VENC_MAX_CHN_NUM_PER_MODULE];

	//Linux
	struct task_struct *ptskWork;
	struct task_struct *ptskIrq;
	///XXX Should not be needed but don't know how to use allocator
	wait_queue_head_t stWorkWaitQueueHead;
	wait_queue_head_t stIrqWaitQueueHead;
	wait_queue_head_t *wait_queue;
	struct list_head * todo_task_list;
	struct list_head * working_task_list;
	struct mutex* list_mutex;

	//HAL and command queue
	///XXX because MI_VENC_CHN is input from MI user.
	///Here should be total 16x4 channels here.
	MHalVencDevHandle hHalDev;
	cmd_mload_interface *cmdq;
	MS_U32 u32IrqNum;
	MHalVencDrv_t *stDrv;
	//MI_VENC_ChnAttr_t astChnInfo[VENC_MAX_CHN_NUM];
} MI_VENC_DevRes_t;

//This enumeration is chip-dependent
typedef enum
{
	E_MI_VENC_DEV_MHE0, //support CMD_Q
	E_MI_VENC_DEV_MHE1, //support CMD_Q
	E_MI_VENC_DEV_MFE0, //support CMD_Q, MFE1 does not support JPEG
	E_MI_VENC_DEV_JPEG, //with CPU
	E_MI_VENC_DEV_BUTT  //number of devices
} MI_VENC_Dev_e;

DECLARE_WAIT_QUEUE_HEAD(isr_queue_mhe0);
LIST_HEAD(todo_task_list_mhe0);
LIST_HEAD(working_task_list_mhe0);
DEFINE_MUTEX(working_list_sem_mhe0);

DECLARE_WAIT_QUEUE_HEAD(isr_queue_mhe1);
LIST_HEAD(todo_task_list_mhe1);
LIST_HEAD(working_task_list_mhe1);
DEFINE_MUTEX(working_list_sem_mhe1);

//... MFE0/1 and JPEG


typedef struct MI_VENC_Res_s
{
	MI_BOOL bInitFlag;
	MI_VENC_ChnRes_t astChnRes[VENC_MAX_CHN_NUM];
	MI_VENC_DevRes_t devs[E_MI_VENC_DEV_BUTT];
} MI_VENC_Res_t;
static MI_VENC_Res_t _ModRes;//Module Resource


//DECLARE_WAIT_QUEUE_HEAD(venc_isr_waitqueue);
//DECLARE_WAIT_QUEUE_HEAD(venc_isr_queue);
//LIST_HEAD(VENC_todo_task_list);
//LIST_HEAD(VENC_working_task_list);
//DEFINE_MUTEX(VENC_working_list_sem);

static inline is_fence_LE(MI_U16 fence1, MI_U16 fence2)
{
    if(fence1<=fence2)
        return 1;
    if(fence2+(0xFFFF-fence1) <0x7FFFF)
        return 1;
    return 0;
}

//triggered from each CMDQ
void VENC_ISR_Proc_Thread(void *data)
{
	MI_VENC_DevRes_t *pDevRes = (MI_VENC_DevRes_t *)data;
	cmd_mload_interface *cmdinf = pDevRes->cmdq;// get_sys_cmdq_service(CMDQ_ID_VENC);
	MI_RESULT mi_ret;

	while(pDevRes->bIrqTaskRun)
	{
		interruptible_sleep_on_timeout(&pDevRes->stIrqWaitQueueHead, 10);
		while(!list_empty(&pDevRes->working_task_list/*VENC_working_task_list*/));
		{
			mi_sys_ChnTaskInfo_t *pstChnTask;
			MI_VENC_ChnRes_t *pstChnRes;
			MHAL_ErrCode_e err;
			VencEncResult stEncResult;
			int loop_cnt = 0;
			pstChnTask = container_of(&pDevRes->working_task_list.next/*VENC_working_task_list.next*/,
					mi_sys_ChnTaskInfo_t, list);

			//double check for CMDQ idle.
			if(!cmdinf->is_cmdq_empty_idle(cmdinf->ctx))
			{
			    break;
			}

            pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
			DOWN(&pstChnRes->semLock);
			pstChnRes->bSelected = FALSE;
            UP(&pstChnRes->semLock);
			DOWN(pDevRes->list_mutex);//down(&VENC_working_list_sem);
			list_del(&pstChnTask->list);
			UP(pDevRes->list_mutex);//up(&VENC_working_list_sem);

			//HAL driver check the result status from IP ISR or IP registers.
			//It would do rate control if any.
			err = MHAL_VENC_EncDone(pstChnRes->hInst, &stEncResult);
			if (err == MI_OK) {
                mi_sys_FinishAndReleaseTask(pstChnTask);
			}
			else {
			    mi_sys_CancelTaskBuf(pstChnTask);
			}
		}
	}
}

//should not be used any more
void VENC_ISR(void *data)
{
	//wake_up(&venc_isr_queue);
}

void CMDQ0_ISR(void* data)
{
	wake_up(_ModRes.devs[E_MI_VENC_DEV_MHE0].stWorkWaitQueueHead);
}

void CMDQ1_ISR(void* data)
{
	wake_up(_ModRes.devs[E_MI_VENC_DEV_MHE1].stWorkWaitQueueHead);
}

void CMDQ2_ISR(void* data)
{
	wake_up(_ModRes.devs[E_MI_VENC_DEV_MFE0].stWorkWaitQueueHead);
}

void CMDQ3_ISR(void* data)
{
	wake_up(_ModRes.devs[E_MI_VENC_DEV_JPEG].stWorkWaitQueueHead);
}

static MI_RESULT _MI_VENC_FindDevResFromId(VENC_CHN VeChn, MI_VENC_DevRes_t **pDevRes)
{
	MI_VENC_ChnRes_t *pChnRes;
	pChnRes = &_ModRes.astChnRes[VeChn];

	DOWN(&pChnRes->semLock);
	if(pChnRes->bCreate && pChnRes->bStart) {
		*pDevRes = pChnRes->pstDevRes;
	} else {
		*pDevRes = NULL;
		UP(&pChnRes->semLock);
		return MI_FAILURE;//not found
	}
	UP(&pChnRes->semLock);
	return MI_SUCCESS;
}

typedef struct
{
	int totalAddedTask;
}VENC_Iterator_WorkInfo;

mi_sys_TaskIteratorCBAction_e MI_VENC_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
	int i;
	int valid_output_port_cnt = 0;
	VENC_Iterator_WorkInfo *workInfo = (VENC_Iterator_WorkInfo *)pUsrData;
	MI_VENC_DevRes_t *pDevRes = NULL;
	MI_VENC_ChnRes_t *pChnRes = NULL;
	MI_RESULT ret = MI_FAILURE;
	MI_SYS_BufConf_t *pstOutCfg;

	ret = _MI_VENC_FindDevResFromId(pstTaskInfo->u32ChnId, &pDevRes);
	if (ret == MI_SUCCESS) {//input format is not correct
	    if (pstTaskInfo->astInputPortBufInfo[0].eBufType != E_MI_SYS_BUFDATA_FRAME) {
	        ret = MI_FAILURE;
	    }
	}
	if (ret == MI_SUCCESS) {
	}
	if (ret != MI_SUCCESS) {
        // Drop input buffer because the channel is not available for now.
        mi_sys_FinishAndReleaseTask(pstTaskInfo);
        DBG_EXIT_ERR("Ch %d is not available. Drop frame directly.\n", pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
	}

	//avoid selecting the same channel for encoding dependency
	pChnRes = _ModRes.astChnRes + pstTaskInfo->u32ChnId;
	if (pChnRes->bSelected) {
	    return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
	}

	//prepare output buffer
	pstOutCfg = pstTaskInfo->astOutputPortPerfBufConfig + 0;//pin 0
	pstOutCfg->u64TargetPts = pstTaskInfo->astInputPortBufInfo[0]->u64Pts;
	pstOutCfg->eBufType     = E_MI_SYS_BUFDATA_RAW;
	pstOutCfg->stRawCfg     = MAX_OUTPUT_ES_SIZE;
	if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_SUCCESS)
		return MI_SYS_ITERATOR_SKIP_CONTINUTE;

	for( i=0;i<pstTaskInfo->u32OutputPortNum; i++)
	{
		BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

		if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i])
			valid_output_port_cnt++;
		else if(pstTaskInfo->astOutputPortBufInfo[i])
			valid_output_port_cnt++;
	}


	//check if lacking of output buffer
	if(pstTaskInfo->u32OutputPortNum && valid_output_port_cnt==0)
		return MI_SYS_ITERATOR_SKIP_CONTINUTE;

	//add to to-do list
	if (pstTaskInfo->miSysDrvHandle == pDevRes->hMiDev) {
	    //list_add_tail(&pstTaskInfo->list, &VENC_todo_task_list);
	    DOWN(&pChnRes->semLock);
        pChnRes->bSelected = MI_TRUE;
        UP(&pChnRes->semLock);
	    DOWN(&pDevRes->list_mutex);
	    list_add_tail(&pstTaskInfo->list, pDevRes->todo_task_list);
	    UP(&pDevRes->list_mutex);
	}

	//stop iteration if exceed device capacity
	if(++workInfo->totalAddedTask >= VENC_MAX_CHN_NUM_PER_MODULE)
		return MI_SYS_ITERATOR_ACCEPT_STOP;
	else
		return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
}

void MI_VENC_SortHandleChnListOrder(LIST_HEAD list)
{
	///sort handle channel  order
}

#if 0
MI_VENC_Process_TASK(mi_sys_ChnTaskInfo_t *pstTask, cmd_mload_interface *cmdinf, MI_U16 fence)
{
	ISP_process(pstTask, cmdinf);
	isp_XC_process(pstTask, cmdinf);
	MDWIN_process(pstTask, cmdinf);
	OSD_process(pstTask,cmdinf);
	cmdinf->cmdq_add_wait_event_cmd(cmdinf, CMDQ_EVENT_ISP_SC_ISR);
}
#endif

static MI_S32 _MI_VENC_IMPL_InjectBuffer(VENC_CHN VdecChn, MI_SYS_BufInfo_t *pstBufInfo)
{
//	MI_SYS_BufInfo_t *pstBufInfo = NULL;
	//add frame to HAL


}

void VencWorkThread(void *data)
{

	MI_U16 fence = 0;
	MI_VENC_DevRes_t *pDevRes = (MI_VENC_DevRes_t *)data;
	cmd_mload_interface *cmdinf = pDevRes->cmdq;// get_sys_cmdq_service(CMDQ_ID_VENC);
	MI_SYS_BufInfo_t *pstBufInfo = NULL;
	//MI_BOOL bBlockedByRateControl = MI_FALSE;

	while(pDevRes->bWorkTaskRun)
	{

		VENC_Iterator_WorkInfo workinfo;
		MI_BOOL bPushFrameOK = MI_FALSE;
		struct list_head pos,n;
		MHalVencInstHandle hInst = NULL;
		MI_VENC_ChnRes_t *pstChnRes = NULL;
		MHAL_ErrCode_e err;

		workinfo.totalAddedTask = 0;

		mi_sys_DevTaskIterator(pDevRes->hMiDev, MI_VENC_TaskIteratorCallBK, &workinfo);
		if(list_empty(&pDevRes->todo_task_list))
		{
			schedule();
			mi_sys_WaitOnInputTaskAvailable(pDevRes->hMiDev, 0, 100);
			continue;
		}

		list_for_each_entry_safe(&pos, &n, &pDevRes->todo_task_list, next)
		{
			mi_sys_ChnTaskInfo_t *pstChnTask;
			int loop_cnt = 0;
			VencInOutBuf stEncBufs;
			MI_SYS_BufInfo_t *pstInBuf, *pstOutBuf;

			pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, list);
			pstChnRes = _ModRes.astChnRes + pstChnTask->u32ChnId;
			if (!_IS_VALID_VENC_CHANNEL(pstChnTask->u32ChnId)) {
			    //should not goes here
			    BUG_ON(pstChnTask->u32ChnId);
			    break;
			}

			//prepare in out buffer
			pstInBuf = pstChnTask->astInputPortBufInfo + 0;//pin 0
            pstOutBuf = pstChnTask->astOutputPortBufInfo + 0;//pin 0
			stEncBufs.InputYUVBuf = pstInBuf->stFrameData.pVirAddr[0];
            stEncBufs.InputYUVBuf2 = pstInBuf->stFrameData.pVirAddr[1];
            stEncBufs.InputYUVBuf3 = pstInBuf->stFrameData.pVirAddr[2];
            stEncBufs.hCmdQ = cmdinf;
            stEncBufs.InputYUVBuf2Size =
                    stEncBufs.InputYUVBuf3Size = 0; //ignored and be calculated in driver
            stEncBufs.OutputBuf = pstOutBuf->stRawData.pVirAddr;
            stEncBufs.OutputBufSize = pstOutBuf->stRawData.u32BufSize;

			err = MHAL_VENC_EncodeOneFrame(pstChnRes->hInst, stEncBufs);
			if (err != MI_OK) {
			    mi_sys_FinishAndReleaseTask(pstChnTask);
			    continue;
			}

			bPushFrameOK = MI_TRUE;

			if (cmdinf) {
			    //while(!cmdinf->ensure_cmdbuf_available(cmdinf, 0x400, 0x1000))
			    while(!cmdinf->check_buf_available(cmdinf, 0x400, 0x1000))
			    {
			        interruptible_sleep_on_timeout(&pDevRes->stIrqWaitQueueHead, 2);
			        loop_cnt++;
			        if(loop_cnt>1000)
			            BUG();//engine hang
			    }
			    cmdinf->write_dummy_reg_cmdq(cmdinf, 0x01);
			    cmdinf->kick_off_cmdq(cmdinf);
			}

			list_del(pstChnTask->list);

			//down(&VENC_working_list_sem);
			//list_add_tail(&pstChnTask->list, &VENC_working_task_list);
			//up(&VENC_working_list_sem);
			DOWN(pDevRes->list_mutex);
			list_add_tail(&pstChnTask->list, &pDevRes->working_task_list);
			UP(pDevRes->list_mutex);
		}
	}
}

void VENCWorkThreadNoCmdQ(void *data)
{
	//TODO
}

#if WITH_DEV
static MI_S32 _MI_VENC_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
	return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnUnbindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
	return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
	return MI_SUCCESS;
}

static MI_S32 _MI_VENC_OnUnbindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
	return MI_SUCCESS;
}
#else
MI_RESULT VencOnBindH265ChnOutputCb(MI_SYS_ChnInputPortCfgInfo_t *pstChnPortCfgInfo)
{
	if (UNSUPPORT(pstChnPortCfgInfo->eFormat) || UNSUPPORT(pstChnPortCfgInfo->u32Stride * pstChnPortCfgInfo->u32Height))
	{
		printf("can't binder channel, capacity limited, unsupported XXXX\n");
		return MI_ERR_FAILED;
	}

	return MI_OK;
}

MI_RESULT VencOnUnBindH265ChnOutputCb(void)
{
	if (PORT_BUSY)
	{
		printf("unbind fail\n");
		return MI_ERR_FAILED;
	}

	return MI_OK;
}

MI_RESULT VencOnBindH264ChnOutputCb(MI_SYS_ChnInputPortCfgInfo_t *pstChnPortCfgInfo)
{
	if (UNSUPPORT(pstChnPortCfgInfo->eFormat) || UNSUPPORT(pstChnPortCfgInfo->u32Stride * pstChnPortCfgInfo->u32Height))
	{
		printf("can't binder channel, capacity limited, unsupported XXXX\n");
		return MI_ERR_FAILED;
	}

	return MI_OK;
}

MI_RESULT VencOnUnBindH264ChnOutputCb(void)
{
	if (PORT_BUSY)
	{
		printf("unbind fail\n");
		return MI_ERR_FAILED;
	}

	return MI_OK;
}
#endif

void* JPEG_CONTEXT = NULL;//TODO read jpeg context in mi_venc
static void _MI_VENC_IMPL_InitRes(void)
{
    MI_U32 i;
	memset(&_ModRes, 0x0, sizeof(_ModRes));
	for (i = 0; i < E_MI_VENC_DEV_BUTT; ++i) {
	    init_waitqueue_head(&_ModRes.devs[i].stIrqWaitQueueHead);
	    init_waitqueue_head(&_ModRes.devs[i].stWorkWaitQueueHead);
    }
	for (i = 0; i < VENC_MAX_CHN_NUM; ++i) {
        sema_init(&_ModRes.astChnRes[i].semLock, 1);
    }
	//TODO initial each HAL module?
}

MHAL_ErrCode_e MHalVencMheCreateDevice(void *pOsDev, void** ppBase, int *pSize, MHalVencDevHandle *phDev)
{
	return MHAL_FAILURE;
}

MHAL_ErrCode_e MHalVencMheDestroyDevice(MHalVencDevHandle hDev)
{
	return MHAL_FAILURE;
}

MHalVencDrv_t mhe0_drvfn = {
	.CreateDevice      = MHalVencMheCreateDevice,
	.DestroyDevice     = MHalVencMheDestroyDevice,
};

MHalVencDrv_t dummy_drvfn = {
	.CreateDevice      = MHalVencMheCreateDevice,
	.DestroyDevice     = MHalVencMheDestroyDevice
};

#define CMDQ_ID_NONE (0xFF)//command queue is not supported
static MI_CMDQ_ID _CMDQ_ID[E_MI_VENC_DEV_BUTT] =
{
    CMDQ_ID_H265_VENC0,//E_MI_VENC_DEV_MHE0
    CMDQ_ID_H265_VENC1,//E_MI_VENC_DEV_MHE1
    CMDQ_ID_H264_VENC0,//E_MI_VENC_DEV_MFE0
    CMDQ_ID_NONE       //E_MI_VENC_DEV_JPEG
};

static MI_RESULT _MI_VENC_CreateDevice(MI_VENC_Dev_e dev_type)
{
#if WITH_DEV
	mi_sys_ModuleDevBindOps_t stVENCOps;
	mi_sys_ModuleDevInfo_t stModInfo;
#else
	mi_sys_ModuleBindOps_t stVENCOps;
	mi_sys_ModuleInfo_t stModInfo;
#endif
	MI_VENC_DevRes_t *pDevRes;
	MHalVencParamInt_t stParamInt;
	MI_CMDQ_ID cmdqId;

	MI_VENC_DevRes_t stNulDev;
	void* pBase;
	MHalVencDevHandle hDev;
	MHAL_ErrCode_e err;
	MI_VENC_DevRes_t *pDevRes;
	cmdq_buffer_descript_t stCmdqBufDesp;

    DBG_ENTER();
	if (dev_type >= E_MI_VENC_DEV_BUTT) {
		return MI_FAILURE;
	}

	pDevRes = _ModRes.devs + dev_type;
	if (pDevRes->bInitFlag) {
        DBG_WRN("Already Init\n");
        return MI_SUCCESS;
	}

	err = MHalVencMheCreateDevice(NULL/*pOsDev*/, &pBase, &hDev);
	if (err != MHAL_SUCCESS) {
		return MI_FAILURE;
	}

	pDevRes->hHalDev = hDev;

	// Register kernel IRQ and Enable IRQ
	err = MHAL_VENC_GetDevConfig(hDev, E_MHAL_VENC_HW_IRQ_NUM, &stParamInt);
	if (err != MHAL_SUCCESS) {
		return MI_FAILURE;
	}
	pDevRes->u32IrqNum = stParamInt.nVal;
	request_irq(pDevRes->u32IrqNum);

	//command queue
	err = MHAL_VENC_GetDevConfig(hDev, E_MHAL_VENC_HW_CMDQ_BUF_LEN, &stParamInt);
	if (err != MHAL_SUCCESS) {
		return MI_FAILURE;
	}
	stCmdqBufDesp.uCmdBufSize = stParamInt.nSize;
	stCmdqBufDesp.uCmdBufSizeAlign = 16;
	stCmdqBufDesp.uMloadBufSize = 0;//0x1000;
	stCmdqBufDesp.uMloadBufSizeAlign = 0;
	cmdqId = _CMDQ_ID[dev_type];
	if (CMDQ_ID_NONE == cmdqId) {
	    pDevRes->cmdq = NULL;
	} else {
	    pDevRes->cmdq = get_sys_cmdq_service(cmdqId, &stCmdqBufDesp, MI_FALSE);
	    if (pDevRes->cmdq == NULL) {
	        return MI_FAILURE;
	    }
	}

	memset(&stVENCOps, 0x0, sizeof(stVENCOps));
	memset(&stModInfo, 0x0, sizeof(stModInfo));
	stModInfo.u32DevId = dev_type;
	stModInfo.u32DevChnNum = 16;
	stModInfo.u32InputPortNum = 1;
	stModInfo.u32OutputPortNum = 1;
#if WITH_DEV
	stModInfo.eModuleId = E_MI_SYS_MODULE_ID_VENC;
	stVENCOps.OnBindInputPort = _MI_VENC_OnBindInputPort;
	stVENCOps.OnUnBindInputPort = _MI_VENC_OnUnbindInputPort;
	stVENCOps.OnBindOutputPort = _MI_VENC_OnBindOutputPort;
	stVENCOps.OnUnBindOutputPort = _MI_VENC_OnUnbindOutputPort;
#else
	stModInfo.u32ModuleId = E_MI_SYS_MODULE_ID_VENC;
	stVENCOps.OnBindChn = VencOnBindH265ChnOutputCb;
	stVENCOps.OnUnBindChn = VencOnUnBindH265ChnOutputCb;
#endif
	pDevRes->hMiDev = mi_sys_RegisterDev(&stModInfo, &stVENCOps);
	if (pDevRes->hMiDev == NULL) {
		return MI_FAILURE;
	}

	pDevRes->ptskWork = kthread_create(VencWorkThread, pDevRes, "VencWorkThread");
	pDevRes->ptskIrq = thread_create(VENC_ISR_Proc_Thread, pDevRes, "VENC_ISR_Proc_Thread");

	wake_up_process(pDevRes->ptskWork);
	pDevRes->bWorkTaskRun = TRUE;
	wake_up_process(pDevRes->ptskIrq);
	pDevRes->bIrqTaskRun = TRUE;

	pDevRes->bInitFlag = TRUE;

	DBG_EXIT_OK();
	return MI_SUCCESS;
}


void MI_VENC_Init()
{
	//TBD: init _ModRes()
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

	_MI_VENC_CreateDevice(E_MI_VENC_DEV_MHE0);
	_MI_VENC_CreateDevice(E_MI_VENC_DEV_MHE1);
	_MI_VENC_CreateDevice(E_MI_VENC_DEV_MFE0);


	//_MI_VENC_CreateDevice(E_MI_VENC_DEV_JPEG);//RIU mode is not designed
	kthread_create(VENCWorkThreadNoCmdQ, JPEG_CONTEXT);

	request_irq(VENC_ISR_IDX, VENC_ISR, NULL);

	_ModRes.devs[E_MI_VENC_DEV_JPEG].cmdq = NULL;


}

static MI_RESULT _MI_VENC_FindAvailableDevChn(MI_VENC_DevRes_t *pDevRes, MI_VENC_ChnRes_t **pOutChn)
{
	int i;

	if (pDevRes == NULL)
		return MI_FAILURE;

	//find first available MI_VENC_ChnRes_t
	for (i = 0; i < VENC_MAX_CHN_NUM_PER_MODULE; ++i) {
		if ((pDevRes->astChnRes[i] != NULL) &&
			(pDevRes->astChnRes[i]->bCreate == FALSE)) {
			*pOutChn = pDevRes[i];
			return MI_SUCCESS;
		}
	}

	*pOutChn = NULL;
	return MI_FAILURE;
}

static MI_RESULT _MI_VENC_CalInBufSize(MI_VENC_ChnAttr_t *pstAttr, MI_U32 *pSize)
{
	MI_VENC_AttrH265_t *h265Attr;

	switch (pstAttr->stVeAttr.enType) {
	case E_MI_VENC_MODTYPE_H265E:
		h265Attr = &pstAttr->stVeAttr.stAttrH265e;
		if (h265Attr->u32BufSize != 0) {
			*pSize = h265Attr->u32BufSize;
		} else {
			*pSize = h265Attr->u32MaxPicWidth * h265Attr->u32MaxPicHeight * h265Attr->u32RefNum;
		}
		break;
	case E_MI_VENC_MODTYPE_H264E:
		pSize = 50 * 1024;
		break;
	case E_MI_VENC_MODTYPE_JPEGE:
		pSize = 300 * 1024;
		break;
	default:
		return MI_ERR_FAILED;
		break;
	}

	return MI_OK;
}

static MI_RESULT _MI_VENC_CalOutBufSize(MI_VENC_ChnAttr_t *pstAttr, MI_U32 *pSize)
{
	switch (pstAttr->stVeAttr.enType) {
	case E_MI_VENC_MODTYPE_H265E:
		pSize = 50 * 1024;
		break;
	case E_MI_VENC_MODTYPE_H264E:
		pSize = 50 * 1024;
		break;
	case E_MI_VENC_MODTYPE_JPEGE:
		pSize = 300 * 1024;
		break;
	default:
		return MI_ERR_FAILED;
		break;
	}

	return MI_OK;
}

static MI_RESULT _MI_VENC_FreeChnMemory(MI_VENC_ChnRes_t *pstChnRes)
{
    if (pstChnRes) {
        #if  VENC_EXTRA_BUFFER
        if (pstChnRes->stEsMemInfo.phyAddr) {
            mi_sys_MMA_Free(pstChnRes->stEsMemInfo.phyAddr);
            pstChnRes->stEsMemInfo.phyAddr = NULL;
        }
        if (pstChnRes->stFrameMemInfo.phyAddr) {
            mi_sys_MMA_Free(pstChnRes->stFrameMemInfo.phyAddr);
            pstChnRes->stFrameMemInfo.phyAddr = NULL;
        }
        #endif
        if (pstChnRes->stAlMemInfo.phyAddr) {
            mi_sys_MMA_Free(pstChnRes->stAlMemInfo.phyAddr);
            pstChnRes->stAlMemInfo.phyAddr = NULL;
        }
        if (pstChnRes->stAlMemInfo.pVirAddr) {
            mi_sys_UnVmap(pstChnRes->stAlMemInfo.pVirAddr);
            pstChnRes->stAlMemInfo.pVirAddr = NULL;
        }
        if (pstChnRes->stRefMemInfo.phyAddr) {
            mi_sys_MMA_Free(pstChnRes->stRefMemInfo.phyAddr);
            pstChnRes->stRefMemInfo.phyAddr = NULL;
        }
        return MI_SUCCESS;
    }
    return MI_FAILURE;
}

static MI_VENC_ModType_e _MI_VENC_ChooseIP(VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
    //simple implementation
    if (VeChn & 1) {
        return E_MI_VENC_DEV_MHE1;
    }
    return E_MI_VENC_DEV_MHE0;
    //by MB numbers
}

static MI_RESULT _MHAL_VENC_ConfigInstance(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_ChnAttr_t *pstAttr)
{
    MHAL_ErrCode_e err;
    MHAL_VENC_IDX eIdxRes, eIdxRc;
    MHalVencParam_t *pstParamRes = NULL, *pstParamRc = NULL;
    VEncResolution stVEncResolution;
    VENCRcInfo stVENCRcInfo;

    if (pstChnRes == NULL || pstAttr == NULL)
        return MI_FAILURE;

    if (pstChnRes->bStart) {
        return MI_FAILURE;//already started!
    }

    switch (pstAttr->stVeAttr.enType) {
    case E_MI_VENC_MODTYPE_H265E:
        break;
    case E_MI_VENC_MODTYPE_H264E:
        //if (pstAttr->stVeAttr.enType)//TODO check enType
        eIdxRes = E_MHAL_VENC_264_RESOLUTION;
        pstParamRes = (MHalVencParam_t*)&(stVEncResolution);
        MHAL_VENC_INIT_PARAM(VEncResolution);
        stVEncResolution.nWidth  = pstAttr->stVeAttr.stAttrH264e.u32MaxPicWidth;
        stVEncResolution.nHeight = pstAttr->stVeAttr.stAttrH264e.u32MaxPicHeight;
        stVEncResolution.stFmt   = E_MI_VENC_FMT_NV12;

        eIdxRc = E_MHAL_VENC_264_RC;
        pstParamRc = (MHalVencParam_t*)&(VENCRcInfo);
        MHAL_VENC_INIT_PARAM(VENCRcInfo);
        switch (pstAttr->stRcAttr.eRcMode) {
        case E_MI_VENC_RC_MODE_H264FIXQP:
            stVENCRcInfo.RcMode = VENC_RC_MODE_H264FIXQP;
            stVENCRcInfo.AttrH264FixQp.SrcFrmRate = pstAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRate;
            stVENCRcInfo.AttrH264FixQp.Gop        = pstAttr->stRcAttr.stAttrH264FixQp.u32Gop;
            stVENCRcInfo.AttrH264FixQp.IQp        = pstAttr->stRcAttr.stAttrH264FixQp.u32IQp;
            stVENCRcInfo.AttrH264FixQp.PQp        = pstAttr->stRcAttr.stAttrH264FixQp.u32PQp;
            break;
        default:
            return MI_FAILURE;//not supported yet.
            break;
        }
        break;
    case E_MI_VENC_MODTYPE_JPEGE:
        break;
    default:
        return MI_FAILURE;
        break;
    }
    err = MHAL_VENC_SetParam(pstChnRes->hInst, eIdxRes, pstParamRes);
    if (err != MI_OK) {
        return MI_FAILURE;
    }
    err = MHAL_VENC_SetParam(pstChnRes->hInst, eIdxRc, pstParamRc);
    if (err != MI_OK) {
        return MI_FAILURE;
    }

    return MI_SUCCESS;
}

//Set runtime parameters
static MI_RESULT _MHAL_VENC_SetRtParam(MI_VENC_ChnRes_t *pstChnRes, MI_VENC_ChnAttr_t *pstAttr)
{
    //TBD
    return MI_SUCCESS;
}

MI_RESULT MI_VENC_CreateChn(VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr)
{
	MI_VENC_DevRes_t *pstDevRes = NULL;
	MI_VENC_ChnRes_t *pstChnRes = NULL;
	MI_U32 u32BufSize;
	MI_RESULT ret = MI_FAILURE;
	MHAL_ErrCode_e err;
	VencInternalBuf stVencInternalBuf;

	if (!_IS_VALID_VENC_CHANNEL(VeChn)) {
		return MI_FAILURE;
	}

	switch (pstAttr->stVeAttr.enType) {
	case E_MI_VENC_MODTYPE_H265E:
		//H.265 load balancer. refine later with MB number
	    pstDevRes = _ModRes.devs + _MI_VENC_ChooseIP(VeChn, pstAttr);
		break;
	case E_MI_VENC_MODTYPE_H264E:
		pstDevRes = _ModRes.devs + E_MI_VENC_DEV_MFE0;
		break;
	case E_MI_VENC_MODTYPE_JPEGE:
		pstDevRes = _ModRes.devs + E_MI_VENC_DEV_JPEG;
		break;
	default:
		return MI_ERR_FAILED;
		break;
	}

	ret = _MI_VENC_FindAvailableDevChn(pstDevRes, &pstChnRes);
	if (ret != MI_OK) {
		return MI_ERR_FAILED;
	}
	DOWN(&pstChnRes->semLock);

	err = MHAL_VENC_CreateInstance(pstDevRes->hHalDev, &pstChnRes->hInst);
	if (err != MI_OK) {
        goto _Exit;
	}

	//allocate in buffer
#if  VENC_EXTRA_BUFFER
	ret = _MI_VENC_CalOutBufSize(pstAttr, &pstChnRes->stEsMemInfo.u32BufSize);
	if (ret != MI_OK) {
        goto _Exit;
	}
	ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stEsMemInfo.u32BufSize, &pChnRes->stEsMemInfo.pVirAddr);
	if (ret != 0)
		goto _Exit;

	//allocate out buffer
	ret = _MI_VENC_CalInBufSize(pstAttr, &pstChnRes->stFrameMemInfo.u32BufSize);
	if (ret != MI_OK)
		goto _Exit;
    ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stFrameMemInfo.u32BufSize, &pstChnRes->stFrameMemInfo.pVirAddr);
    if (ret != 0)
        goto _Exit;
#endif

    //==== Set other dynamic parameters
    ret = _MHAL_VENC_ConfigInstance(pstDevRes, pstAttr);
    if (ret != MI_SUCCESS)
        goto _Exit;

    //allocate internal buffer size, such as refer/current YUV buffers
    MHAL_VENC_INIT_PARAM(VencInternalBuf);
    //set virtual buffer to 0 for un-map
    pstChnRes->stAlMemInfo.pVirAddr = NULL;
	err = MHAL_VENC_QueryBufSize(pstDevRes->hHalDev, &stVencInternalBuf);
    if (err != MI_OK) {
        goto _Exit;
    }
    pstChnRes->stAlMemInfo.u32BufSize = stVencInternalBuf.IntrAlBufSize;
    pstChnRes->stRefMemInfo.u32BufSize = stVencInternalBuf.IntrRefBufSize;

    ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stAlMemInfo.u32BufSize, &pstChnRes->stAlMemInfo.phyAddr);
    if (ret != 0)
        goto _Exit;
    ret = mi_sys_MMA_Alloc(NULL, pstChnRes->stRefMemInfo.u32BufSize, &pstChnRes->stRefMemInfo.phyAddr);
    if (ret != 0)
        goto _Exit;

    pstChnRes->stAlMemInfo.pVirAddr = mi_sys_Vmap(pstChnRes->stAlMemInfo.phyAddr, pstChnRes->stAlMemInfo.u32BufSize,
            FALSE/*non-cached*/);
    if (pstChnRes->stAlMemInfo.pVirAddr == NULL)
        goto _Exit;

    stVencInternalBuf.IntrAlPhyBuf  = pstChnRes->stAlMemInfo.phyAddr;
    stVencInternalBuf.IntrAlVirBuf  = pstChnRes->stAlMemInfo.pVirAddr;
    stVencInternalBuf.IntrRefPhyBuf = pstChnRes->stRefMemInfo.phyAddr;

    err = MHAL_VENC_SetParam(pstChnRes->hInst, E_MHAL_VENC_IDX_STREAM_ON, &stVencInternalBuf);
    if (err != MI_OK) {
        goto _Exit;
    }

    //==== Set other dynamic parameters
    ret = _MHAL_VENC_SetRtParam(pstDevRes, pstAttr);
    if (ret != MI_SUCCESS)
        goto _Exit;

	///save channel attribute
	pstChnRes->stChnAttr = *pstAttr;
	pstChnRes->bCreate = TRUE;

	UP(&pstChnRes->semLock);

	return MI_OK;

_Exit:
    (void)_MI_VENC_FreeChnMemory(pstChnRes);
	UP(&pstChnRes->semLock);

	return MI_FAILURE;
}

MI_RESULT MI_VENC_DestroyChn(VENC_CHN VeChn)
{
    MI_VENC_DevRes_t *pstDevRes = NULL;
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    MI_U32 u32BufSize;
    MI_RESULT ret = MI_FAILURE;
    MHAL_ErrCode_e err;
    VencInternalBuf stVencInternalBuf;

    ret = MI_VENC_StopRecvPic(VeChn);
    if (ret == MI_FAILURE) {
        return ret;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (pstChnRes) {
        DOWN(&pstChnRes->semLock);
        err = MHAL_VENC_SetParam(pstChnRes->hInst, E_MHAL_VENC_IDX_STREAM_OFF, NULL);
        if (err != MI_OK) {
            goto _ExitWithChn;
        }
        err = MHAL_VENC_DestroyInstance(pstChnRes->hInst);
        if (err != MI_OK) {
            goto _ExitWithChn;
        }
        pstChnRes->bCreate = FALSE;
        (void)_MI_VENC_FreeChnMemory(pstChnRes);
        UP(&pstChnRes->semLock);
        return MI_SUCCESS;
    }
    //remove list?
    return MI_FAILURE;//TODO
_ExitWithChn:
    UP(&pstChnRes->semLock);
    return MI_FAILURE;
}

MI_RESULT MI_VENC_StartRecvPicEx(VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    if (!_IS_VALID_VENC_CHANNEL(VeChn)) {
        return MI_FAILURE;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (pstChnRes->bCreate) {
        pstChnRes->bStart = TRUE;
    }
    return MI_SUCCESS;
}

MI_RESULT MI_VENC_StartRecvPic(VENC_CHN VeChn)
{
	return MI_VENC_StartRecvPicEx(VeChn, NULL);
}

MI_RESULT MI_VENC_StopRecvPic(VENC_CHN VeChn)
{
    MI_VENC_ChnRes_t *pstChnRes = NULL;
    if (!_IS_VALID_VENC_CHANNEL(VeChn)) {
        return MI_FAILURE;
    }

    pstChnRes = _ModRes.astChnRes + VeChn;
    if (pstChnRes->bCreate) {
        pstChnRes->bStart = FALSE;
    }
    return MI_SUCCESS;
}

static MI_RESULT _MI_VENC_FindChnResFromId(VENC_CHN VeChn, MI_VENC_ChnRes_t *pChnRes)
{
	//FIXME 48 channels structure
	*pChnRes = &_ModRes.devs[E_MI_VENC_DEV_MHE0][VeChn];
	return MI_SUCCESS;
}


MI_RESULT MI_VENC_GetStream(VENC_CHN VeChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec)
{
	MHAL_ErrCode_e err;
	MI_RESULT ret = MI_FAILURE;
	MI_VENC_ChnRes_t *pChnRes = NULL;
	MHal_OutBuf out;
	MI_SYS_BufInfo_t *pstMiSysBuf;

	ret = _MI_VENC_FindChnResFromId(VeChn, &pChnRes);
	if (ret != MI_OK) {
		return MI_ERR_FAILED;
	}

	if(s32MilliSec != 0 /*non-blocking*/) {
	/**@remark Need select() mechanism for s32MilliSec timeout
	 */
		return MI_FAILURE;
	}

	mi_sys_SetInputPortUsrAllocator()

	pstMiSysBuf = mi_sys_GetOutputPortBuf();

	if (pstMiSysBuf) {
		//TODO convert MI_SYS_BufInfo_t to MI_VENC_Stream_t
		*pstStream = *pstMiSysBuf;

	}
	/*
	err = MHalVencGetOutBuf(pChnRes->hInst, &out);
	if (err == MHAL_SUCCESS) {
		//TODO convert out to pstStream
		*pstStream = out;
	}*/
	return MI_SUCCESS;
}

MI_RESULT MI_VENC_ReleaseStream(VENC_CHN VeChn, MI_VENC_Stream_t *pstStream)
{
	XXX revise
	MHAL_ErrCode_e err;
	MI_RESULT ret = MI_FAILURE;
	MI_VENC_ChnRes_t *pChnRes = NULL;
	MHal_OutBuf out;
	MI_SYS_BufInfo_t *pstMiSysBuf;

	ret = _MI_VENC_FindChnResFromId(VeChn, &pChnRes);
	if (ret != MI_OK) {
		return MI_ERR_FAILED;
	}

	//TODO convert from MI_VENC_Stream_t to MI_SYS_BufInfo_t
	pstMiSysBuf
	mi_sys_FinishBuf(pstMiSysBuf);
	/*
	//TODO convert pstStream out
	out = *pstStream;

	err = MHalVencReleaseOutBuf(pChnRes->hInst, &out);
	if (err == MHAL_SUCCESS) {
	}*/
	return MI_SUCCESS;
}

MI_RESULT MI_VENC_InsertUserData(VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len)
{
	return MI_SUCCESS;
}
