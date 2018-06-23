#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "mi_print.h"

#include "mi_sys_internal.h"
#include "mi_sys_impl.h"

#include "mi_sys_buf_mgr.h"

#ifdef MI_SYS_PROC_FS_DEBUG
#include "drv_proc_ext.h"
#endif


#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_allocator_proc_create(MI_U8 *allocator_proc_name,mi_sys_AllocatorProcfsOps_t *Ops,void *allocator_private_data)
{
    return _MI_SYS_IMPL_Allocator_PROC_CREATE(allocator_proc_name,Ops,allocator_private_data);
}
void mi_sys_allocator_proc_remove_allocator(MI_U8 *entry_name)
{
    _MI_SYS_IMPL_Proc_Remove_Allocator(entry_name);
}
#endif


MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, mi_sys_ModuleDevBindOps_t *pstModuleBindOps , void *pUsrData
                                                #ifdef MI_SYS_PROC_FS_DEBUG
                                                ,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
                                                ,struct proc_dir_entry *proc_dir_entry
                                                #endif
                                                )
{
    return MI_SYS_IMPL_RegisterDev(pstMouleInfo,pstModuleBindOps,pUsrData
                                              #ifdef MI_SYS_PROC_FS_DEBUG
                                              ,pstModuleProcfsOps
                                              ,proc_dir_entry
                                              #endif
                                  );
}
MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle)
{
    return MI_SYS_IMPL_UnRegisterDev(miSysDrvHandle);
}
#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_RegistCommand(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                    MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData),
                    MI_SYS_DRV_HANDLE hHandle)
{
    return MI_SYS_IMPL_RegistCommand(u8Cmd, u8MaxPara, fpExecCmd, hHandle);
}
#endif
MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, struct mi_sys_ChnBufInfo_s *pstChnBufInfo)
{
    return MI_SYS_IMPL_GetChnBufInfo(miSysDrvHandle , u32ChnId , pstChnBufInfo);
}

MI_S32 mi_sys_DevTaskIterator(MI_SYS_DRV_HANDLE miSysDrvHandle, mi_sys_TaskIteratorCallBack pfCallBack,void *pUsrData)
{
    return MI_SYS_IMPL_DevTaskIterator(miSysDrvHandle,pfCallBack,pUsrData);
}

MI_S32 mi_sys_DropTask(mi_sys_ChnTaskInfo_t *pstTask)
{
    return MI_SYS_IMPL_DropTask(pstTask);
}

MI_S32 mi_sys_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask)
{
    return MI_SYS_IMPL_PrepareTaskOutputBuf(pstTask);
}

MI_S32 mi_sys_FinishAndReleaseTask(mi_sys_ChnTaskInfo_t *pstTask)
{
    return MI_SYS_IMPL_FinishAndReleaseTask(pstTask);
}

MI_S32 mi_sys_FinishAndReleaseTask_LL(mi_sys_ChnTaskInfo_t *pstTask)
{
    return MI_SYS_IMPL_FinishAndReleaseTask_LL(pstTask);
}

MI_S32 mi_sys_RewindTask(mi_sys_ChnTaskInfo_t *pstTask)
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

MI_SYS_BufInfo_t *mi_sys_GetInputPortBuf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32Flags)
{
    return MI_SYS_IMPL_GetInputPortBuf(miSysDrvHandle, u32ChnId , u32PortId, u32Flags);
}

MI_S32 mi_sys_FinishBuf(MI_SYS_BufInfo_t *buf)
{
    return MI_SYS_IMPL_FinishBuf(buf);
}

MI_S32 mi_sys_FinishBuf_LL(MI_SYS_BufInfo_t *buf)
{
    return MI_SYS_IMPL_FinishBuf_LL(buf);
}

MI_S32 mi_sys_RewindBuf(MI_SYS_BufInfo_t *buf)
{
    return MI_SYS_IMPL_RewindBuf(buf);
}

MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32blkSize ,MI_PHY *phyAddr)
{
    return MI_SYS_IMPL_MmaAlloc(u8MMAHeapName,u32blkSize,phyAddr);
}

MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr)
{
    return MI_SYS_IMPL_MmaFree(phyAddr);
}

MI_SYS_BufInfo_t* mi_sys_InnerAllocBufFromVbPool(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32blkSize, MI_PHY *phyAddr)
{
    return MI_SYS_IMPL_InnerAllocBufFromVbPool(miSysDrvHandle, u32ChnId, u32blkSize, phyAddr);
}

MI_S32 mi_sys_FreeInnerVbPool(MI_SYS_BufInfo_t *pBufAllocation)
{
    return MI_SYS_IMPL_FreeInnerVbPool(pBufAllocation);
}

MI_S32 mi_sys_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId , MI_VB_PoolListConf_t  stPoolListConf)
{
    return MI_SYS_IMPL_ConfDevPubPools(eModule, u32DevId , stPoolListConf);
}

MI_S32 mi_sys_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId)
{
    return MI_SYS_IMPL_RelDevPubPools(eModule, u32DevId);
}


void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache)
{
    return MI_SYS_IMPL_Vmap(u64PhyAddr ,u32Size ,  bCache);
}

void mi_sys_UnVmap(void *pVirtAddr)
{
    return MI_SYS_IMPL_UnVmap(pVirtAddr);
}

MI_S32 mi_sys_VFlushInvCache(void *pVirtAddr, MI_U32 u32Size)
{
    return MI_SYS_IMPL_FlushInvCache(pVirtAddr, u32Size);
}

MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, mi_sys_Allocator_t *pstUserAllocator)
{
    return MI_SYS_IMPL_SetInputPortUsrAllocator(miSysDrvHandle,u32ChnId,u32PortId,pstUserAllocator);
}

MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, mi_sys_Allocator_t *pstUserAllocator)
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

MI_S32 mi_sys_SetOutputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    return MI_SYS_IMPL_SetOutputPortBufExtConf(miSysDrvHandle, u32ChnId , u32PortId, pstBufExtraConf);
}

MI_S32 mi_sys_SetInputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf)
{
    return MI_SYS_IMPL_SetInputPortBufExtConf(miSysDrvHandle, u32ChnId , u32PortId, pstBufExtraConf);
}

MI_S32 mi_sys_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    return MI_SYS_IMPL_EnableInputPort(miSysDrvHandle,u32ChnId,u32PortId);
}

MI_S32 mi_sys_SetInputPortSidebandMsg(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_U64 u64SidebandMsg)
{
    return MI_SYS_IMPL_SetInputPortSidebandMsg(miSysDrvHandle,u32ChnId,u32PortId, u64SidebandMsg);
}

MI_S32 mi_sys_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId)
{
    return MI_SYS_IMPL_DisableInputPort(miSysDrvHandle,u32ChnId,u32PortId);
}

MI_S32 mi_sys_MmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle)
{
    return MI_SYS_IMPL_MmapBufToUser(miSysDrvHandle, pstBufInfo, ppBufHandle);
}

MI_S32 mi_sys_UnmapBufToUser(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo)
{
    return MI_SYS_IMPL_UnmapBufToUser(miSysDrvHandle, pBufHandle, ppstBufInfo);
}
MI_S32 mi_sys_EnsureOutportBkRefFifoDepth (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth)
{
    return MI_SYS_IMPL_EnsureOutportBKRefFifoDepth(miSysDrvHandle,u32ChnId,u32PortId,u32BkRefFifoDepth);
}

MI_PHY mi_sys_Cpu2Miu_BusAddr(MI_PHY cpu_addr)
{
    return MI_SYS_IMPL_Cpu2Miu_BusAddr(cpu_addr);
}
unsigned long long mi_sys_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr)
{
    return MI_SYS_IMPL_Miu2Cpu_BusAddr(miu_phy_addr);
}

MI_SYS_BufferAllocation_t* mi_sys_AllocMMALowLevelBufAllocation(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_SYS_BufConf_t *pstBufConfig)
{
     return MI_SYS_IMPL_AllocMMALowLevelBufAllocation(miSysDrvHandle, u32ChnId, pstBufConfig);
}

MI_S32 mi_sys_NotifyPreProcessBuf(MI_SYS_BufInfo_t *buf)
{
    return MI_SYS_IMPL_OutputBuf_Pre_Process_Notify(buf);
}

MI_S32 mi_sys_EnableLowlatencyPreProcessMode(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_BOOL bEnable, MI_U32 delayMs)
{
    return MI_SYS_IMPL_EnableLowlatencyPreProcessMode(miSysDrvHandle,u32ChnId,u32PortId, bEnable, delayMs);
}

MI_U32 mi_sys_Get_Vdec_VBPool_UseStatus(void)
{
    return MI_SYS_IMPL_Get_VDEC_VBPool_Flag();
}


#ifdef DEBUG_YUV_KERN_API
/*
return :
FILE_HANDLE type in fact is a pointer,
NULL:fail
not NULL:success
*/
FILE_HANDLE open_yuv_file(const char *pathname, int open_for_write)
{
    FILE_HANDLE fp;
    fp = filp_open(pathname, open_for_write?(O_WRONLY|O_CREAT|O_TRUNC):O_RDONLY,0644);
    if (IS_ERR(fp))//result of filp_open,use if(IS_ERR(fp)),not use if(PTR_ERR(fp)),not use if(!fp)
    {
        DBG_ERR("Open File Faild   PTR_ERR_fp=%d\n",PTR_ERR(fp));//base if(IS_ERR(fp)) is true,use PTR_ERR(fp) to show errno
        return NULL;
    }
    fp->f_op->llseek(fp, 0, SEEK_SET);
    return fp;
}

/*
return value:
0:success
-1:fail
*/
int read_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata)
{
    int width, height;
    char *dst_buf;
    int i, n;
    mm_segment_t fs;

    if(IS_ERR(filehandle) )//for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n",__FUNCTION__,__LINE__,PTR_ERR(filehandle));//base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return -1;
    }
    if(framedata.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && framedata.ePixelFormat !=E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        printk("%s:%d ePixelFormat %d not support!\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
        return -1;
    }

    width = framedata.u16Width;
    height = framedata.u16Height;

    fs = get_fs();
    set_fs(KERNEL_DS);
    n = 0;
    switch (framedata.ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:

            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                filehandle->f_op->llseek(filehandle, 0L, SEEK_CUR);
                if((n = filehandle->f_op->read(filehandle, dst_buf, width*2, &(filehandle->f_pos))) != width*2)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            set_fs(fs);
            return 0;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(vfs_read(filehandle, (char *)dst_buf, width*1, &filehandle->f_pos) != width*1)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            dst_buf = framedata.pVirAddr[1];
            for( i=0;i<height; i++)
            {
                if(vfs_read(filehandle, (char *)dst_buf, width*1, &filehandle->f_pos) != width*1)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[1];
            }
            set_fs(fs);
            return 0;
        default:
            printk("%s:%d   not support framedata.ePixelFormat=%d\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
            return -1;
    }

ERR_RET:
    set_fs(fs);
    printk("%s:%d fail\n",__FUNCTION__,__LINE__);
    return -1;
}

/*
return value:
0:success
-1:fail
*/
int write_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata)
{
    int width, height;
    char *dst_buf;
    int i;
    mm_segment_t fs = get_fs();

    if(IS_ERR(filehandle)) //for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n",__FUNCTION__,__LINE__,PTR_ERR(filehandle));//base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return -1;
    }
    if(framedata.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && framedata.ePixelFormat !=E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        printk("%s:%d ePixelFormat %d not support!\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
        return -1;
    }
    width = framedata.u16Width;
    height = framedata.u16Height;
    set_fs(KERNEL_DS);
    switch (framedata.ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width*2, &filehandle->f_pos) != width*2)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            return 0;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width, &filehandle->f_pos) != width)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
                dst_buf = framedata.pVirAddr[1];
            for( i=0;i<height/2; i++)
            {
                if(vfs_write(filehandle, (char *)dst_buf, width, &filehandle->f_pos) != width)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[1];
            }
            set_fs(fs);
            return 0;
        default:
            printk("%s:%d   not support framedata.ePixelFormat=%d\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
            return -1;
    }
ERR_RET:
    set_fs(fs);
    printk("%s:%d fail\n",__FUNCTION__,__LINE__);
    return -1;
}

void close_yuv_file(FILE_HANDLE filehandle)
{

    if(IS_ERR(filehandle)) //for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n",__FUNCTION__,__LINE__,PTR_ERR(filehandle));//base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return;
    }

    filp_close(filehandle, NULL);
}

//reset to file begin
/*
return value:
0:success
-1:fail
*/
int reset_yuv_file(FILE_HANDLE filehandle)
{
    off_t retval;
    loff_t res ;

    if(IS_ERR(filehandle)) //for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n",__FUNCTION__,__LINE__,PTR_ERR(filehandle));//base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return -1;
    }

    retval = -EINVAL;
    res = vfs_llseek(filehandle, 0, SEEK_SET);//reset to file begin
    retval = res;
    if (res != (loff_t)retval)
    retval = -EOVERFLOW;    /* LFS: should only happen on 32 bit platforms */
    return retval;
}

/*
return value:
0:success
-1:fail
*/
int is_in_yuv_file_end(FILE_HANDLE filehandle)
{
    off_t retval;
    loff_t res_cur ;
    loff_t res_end ;

    if(IS_ERR(filehandle)) //for "struct file* ",use IS_ERR(filehandle)
    {
        printk("%s:%d filehandle is NULL PTR_ERR_fp=%ld\n",__FUNCTION__,__LINE__,PTR_ERR(filehandle));//base if(IS_ERR(filehandle)) is true,use PTR_ERR(filehandle) to show errno
        return 0;//here jsut return false
    }


    retval = -EINVAL;
    res_cur = vfs_llseek(filehandle, 0, SEEK_CUR);//get and save current pos.
    retval = res_cur;
    if (res_cur != (loff_t)retval)
    {
        retval = -EOVERFLOW;	/* LFS: should only happen on 32 bit platforms */
        return 0;//do nothing,return false.
    }
    res_end = vfs_llseek(filehandle, 0, SEEK_END);//get end pos.
    vfs_llseek(filehandle, res_cur, SEEK_SET);//after seek end ,and before return , restore to current pos

    retval = res_end;
    if (res_end != (loff_t)retval)
    {
        retval = -EOVERFLOW;	/* LFS: should only happen on 32 bit platforms */
        return 0;
    }
    if(res_cur == res_end)
    {
        return 1;//current pos in end .
    }
    else
    {
        return 0;
    }
}
#endif

EXPORT_SYMBOL(mi_sys_RegisterDev);
EXPORT_SYMBOL(mi_sys_UnRegisterDev);
EXPORT_SYMBOL(mi_sys_GetChnBufInfo);
EXPORT_SYMBOL(mi_sys_DevTaskIterator);
EXPORT_SYMBOL(mi_sys_PrepareTaskOutputBuf);
EXPORT_SYMBOL(mi_sys_FinishAndReleaseTask);
EXPORT_SYMBOL(mi_sys_FinishAndReleaseTask_LL);
EXPORT_SYMBOL(mi_sys_RewindTask);
EXPORT_SYMBOL(mi_sys_WaitOnInputTaskAvailable);
EXPORT_SYMBOL(mi_sys_GetOutputPortBuf);
EXPORT_SYMBOL(mi_sys_GetInputPortBuf);
EXPORT_SYMBOL(mi_sys_FinishBuf);
EXPORT_SYMBOL(mi_sys_FinishBuf_LL);
EXPORT_SYMBOL(mi_sys_RewindBuf);
EXPORT_SYMBOL(mi_sys_MMA_Alloc);
EXPORT_SYMBOL(mi_sys_MMA_Free);
EXPORT_SYMBOL(mi_sys_Vmap);
EXPORT_SYMBOL(mi_sys_UnVmap);
EXPORT_SYMBOL(mi_sys_VFlushInvCache);
EXPORT_SYMBOL(mi_sys_SetInputPortUsrAllocator);
EXPORT_SYMBOL(mi_sys_SetPeerOutputPortCusAllocator);
EXPORT_SYMBOL(mi_sys_MmapBufToUser);
EXPORT_SYMBOL(mi_sys_UnmapBufToUser);
EXPORT_SYMBOL(mi_sys_DisableChannel);
EXPORT_SYMBOL(mi_sys_EnableChannel);
EXPORT_SYMBOL(mi_sys_EnableOutputPort);
EXPORT_SYMBOL(mi_sys_DisableOutputPort);
EXPORT_SYMBOL(mi_sys_EnableInputPort);
EXPORT_SYMBOL(mi_sys_DisableInputPort);
EXPORT_SYMBOL(mi_sys_EnsureOutportBkRefFifoDepth);
EXPORT_SYMBOL(mi_sys_Cpu2Miu_BusAddr);
EXPORT_SYMBOL(mi_sys_Miu2Cpu_BusAddr);
EXPORT_SYMBOL(mi_sys_SetInputPortSidebandMsg);
EXPORT_SYMBOL(mi_sys_AllocMMALowLevelBufAllocation);
EXPORT_SYMBOL(mi_sys_NotifyPreProcessBuf);
EXPORT_SYMBOL(mi_sys_EnableLowlatencyPreProcessMode);
EXPORT_SYMBOL(mi_sys_InnerAllocBufFromVbPool);
EXPORT_SYMBOL(mi_sys_FreeInnerVbPool);
EXPORT_SYMBOL(mi_sys_ConfDevPubPools);
EXPORT_SYMBOL(mi_sys_RelDevPubPools);
EXPORT_SYMBOL(mi_sys_SetInputPortBufExtConf);
EXPORT_SYMBOL(mi_sys_SetOutputPortBufExtConf);
EXPORT_SYMBOL(mi_sys_Get_Vdec_VBPool_UseStatus);
EXPORT_SYMBOL(mi_sys_DropTask);

#ifdef MI_SYS_PROC_FS_DEBUG
EXPORT_SYMBOL(mi_sys_RegistCommand);
#endif
#ifdef DEBUG_YUV_KERN_API
EXPORT_SYMBOL(open_yuv_file);
EXPORT_SYMBOL(read_yuv_file);
EXPORT_SYMBOL(write_yuv_file);
EXPORT_SYMBOL(close_yuv_file);
EXPORT_SYMBOL(reset_yuv_file);
EXPORT_SYMBOL(is_in_yuv_file_end);
#endif

