#include <linux/kernel.h>
#include <linux/slab.h>

#include "mi_print.h"

#include "mi_sys_internal.h"
#include "mi_sys_impl.h"

#include "mi_sys_buf_mgr.h"


#define MI_SYS_MALLOC(x) kmalloc(x, GFP_ATOMIC)
#define MI_SYS_FREE(x) kfree(x)



MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(struct mi_sys_ModuleDevInfo_s *pstMouleInfo, struct mi_sys_ModuleDevBindOps_s *pstModuleBindOps , void *pUsrData)
{
    return MI_SYS_IMPL_RegisterDev(pstMouleInfo,pstModuleBindOps,pUsrData);
}

MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
	return MI_SYS_IMPL_UnRegisterDev(miSysDrvHandle);
}

MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, struct mi_sys_ChnBufInfo_s *pstChnBufInfo)
{
	return MI_SYS_IMPL_GetChnBufInfo(miSysDrvHandle , u32ChnId , pstChnBufInfo);
}

MI_S32 mi_sys_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData)
{
	return MI_SYS_IMPL_DevTaskIterator(miSysDrvHandle,pfCallBack,pUsrData);
}

MI_S32 mi_sys_PrepareTaskOutputBuf(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	return MI_SYS_IMPL_PrepareTaskOutputBuf(pstTask);
}


MI_S32 mi_sys_FinishTaskBuf(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	return MI_SYS_IMPL_FinishAndReleaseTask(pstTask);
}

MI_S32 mi_sys_RewindTaskBuf(struct mi_sys_ChnTaskInfo_s *pstTask)
{
	return MI_SYS_IMPL_RewindTask(pstTask);
}

MI_S32 mi_sys_WaitOnInputTaskAvailable(MI_SYS_DRV_HANDLE miSysDrvHandle , MI_S32 u32TimeOutMs)
{
	return MI_SYS_IMPL_WaitOnInputTaskAvailable(miSysDrvHandle,u32TimeOutMs);
}

MI_SYS_BufInfo_t *mi_sys_GetOutputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct MI_SYS_BufConf_s *pstBufConfig , MI_BOOL *pbBlockedByRateCtrl)
{
	return MI_SYS_IMPL_GetOutputPortBuf(miSysDrvHandle,u32ChnId,u32PortId, pstBufConfig , pbBlockedByRateCtrl);
}

MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId)
{
	return MI_SYS_IMPL_GetInputPortBuf(miSysDrvHandle, u32ChnId , u32PortId);
}

MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t *buf)
{
	return MI_SYS_IMPL_FinishBuf(buf);
}

MI_S32 mi_sys_RewindBuf(struct MI_SYS_BufInfo_s *buf)
{
 	return MI_SYS_IMPL_RewindBuf(buf);
}

MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32blkSize ,MI_PHY *phyAddr)
{
	return MI_SYS_IMPL_MMA_Alloc(u8MMAHeapName,u32blkSize,phyAddr);
}

MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr)
{
	return MI_SYS_IMPL_MMA_Free(phyAddr);
}

void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache)
{
	return MI_SYS_IMPL_Vmap(u64PhyAddr ,u32Size ,  bCache);
}

void mi_sys_UnVmap(void *pVirtAddr)
{
	return MI_SYS_IMPL_UnVmap(pVirtAddr);
}

MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator)
{
	return MI_SYS_IMPL_SetInputPortUsrAllocator(miSysDrvHandle,u32ChnId,u32PortId,pstUserAllocator);
}

MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator)
{
	return MI_SYS_IMPL_SetPeerOutputPortCusAllocator(miSysDrvHandle,u32ChnId,u32PortId,pstUserAllocator);
}

MI_S32 mi_sys_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
	return MI_SYS_IMPL_DisableChannel(miSysDrvHandle,u32ChnId);
}
MI_S32 mi_sys_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId)
{
	return MI_SYS_IMPL_EnableChannel(miSysDrvHandle,u32ChnId);
}

MI_S32 mi_sys_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	return MI_SYS_IMPL_EnableOutputPort(miSysDrvHandle,u32ChnId,u32PortId);
}
MI_S32 mi_sys_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
	return MI_SYS_IMPL_DisableOutputPort(miSysDrvHandle,u32ChnId,u32PortId);
}


EXPORT_SYMBOL(mi_sys_RegisterDev);
EXPORT_SYMBOL(mi_sys_UnRegisterDev);
EXPORT_SYMBOL(mi_sys_GetChnBufInfo);
EXPORT_SYMBOL(mi_sys_DevTaskIterator);
EXPORT_SYMBOL(mi_sys_PrepareTaskOutputBuf);
EXPORT_SYMBOL(mi_sys_FinishTaskBuf);
EXPORT_SYMBOL(mi_sys_RewindTaskBuf);
EXPORT_SYMBOL(mi_sys_WaitOnInputTaskAvailable);
EXPORT_SYMBOL(mi_sys_GetOutputPortBuf);
EXPORT_SYMBOL(mi_sys_GetInputPortBuf);
EXPORT_SYMBOL(mi_sys_FinishBuf);
EXPORT_SYMBOL(mi_sys_RewindBuf);
EXPORT_SYMBOL(mi_sys_MMA_Alloc);
EXPORT_SYMBOL(mi_sys_MMA_Free);
EXPORT_SYMBOL(mi_sys_Vmap);
EXPORT_SYMBOL(mi_sys_UnVmap);
EXPORT_SYMBOL(mi_sys_SetInputPortUsrAllocator);
EXPORT_SYMBOL(mi_sys_SetPeerOutputPortCusAllocator);

