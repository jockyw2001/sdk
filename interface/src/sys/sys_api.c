#ifndef __KERNEL__
#include <unistd.h>
#endif

#include "mi_syscall.h"
#include "mi_print.h"
#include "mi_sys_datatype.h"
#include "sys_ioctl.h"

MI_MODULE_DEFINE(sys)

MI_S32 MI_SYS_Init()
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 u32Value = 0;
    s32Ret = MI_SYSCALL(MI_SYS_INIT, &u32Value);
    return s32Ret;
}

MI_S32 MI_SYS_Exit()
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 u32Value = 0;
    s32Ret = MI_SYSCALL(MI_SYS_EXIT, &u32Value);
    return s32Ret;
}

MI_S32 MI_SYS_BindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort ,MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPortArg_t stChnPortArg;
    memcpy(&stChnPortArg.stSrcChnPort , pstSrcChnPort , sizeof(MI_SYS_ChnPort_t));
    memcpy(&stChnPortArg.stDstChnPort, pstDstChnPort , sizeof(MI_SYS_ChnPort_t));
    stChnPortArg.u32DstFrmrate = u32DstFrmrate;
    stChnPortArg.u32SrcFrmrate = u32SrcFrmrate;
    s32Ret = MI_SYSCALL(MI_SYS_BIND_CHN_PORT, &stChnPortArg);
    return s32Ret;
}

MI_S32 MI_SYS_UnBindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPortArg_t stChnPortArg;
    memcpy(&stChnPortArg.stSrcChnPort , pstSrcChnPort , sizeof(MI_SYS_ChnPort_t));
    memcpy(&stChnPortArg.stDstChnPort, pstDstChnPort , sizeof(MI_SYS_ChnPort_t));
    s32Ret = MI_SYSCALL(MI_SYS_UNBIND_CHN_PORT, &stChnPortArg);
    return s32Ret;
}

MI_S32 MI_SYS_GetVersion (MI_SYS_Version_t *pstVersion)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_SYS_GET_VERSION, pstVersion);
    return s32Ret;
}

MI_S32 MI_SYS_GetCurPts (MI_U64 *pu64Pts)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_SYS_GET_CUR_PTS, pu64Pts);
    return s32Ret;
}

MI_S32 MI_SYS_InitPtsBase (MI_U64 u64PtsBase)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_SYS_INIT_PTS_BASE, &u64PtsBase);
    return s32Ret;
}

MI_S32 MI_SYS_SyncPts (MI_U64 u64Pts)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_SYS_SYNC_PTS, &u64Pts);
    return s32Ret;
}

MI_S32 MI_SYS_Mmap(MI_PHY phyAddr, MI_U32 u32Size , void **ppVirtualAddress , MI_BOOL bCache)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Mmap_t stMmap;
    stMmap.phyAddr = phyAddr;
    stMmap.u32Size = u32Size;
    stMmap.bCache = bCache;
    s32Ret = MI_SYSCALL(MI_SYS_MMAP, &stMmap);
    *ppVirtualAddress = stMmap.pVirtualAddress;
    return s32Ret;
}

MI_S32 MI_SYS_Munmap(void *pVirtualAddress, MI_U32 u32Size)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Munmap_t stMunmap;
    stMunmap.pVirtualAddress = pVirtualAddress;
    stMunmap.u32Size = u32Size;
    s32Ret = MI_SYSCALL(MI_SYS_MUNMAP, &stMunmap);
    return s32Ret;
}

MI_S32 MI_SYS_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetReg_t stSetReg;
    stSetReg.u32RegAddr = u32RegAddr;
    stSetReg.u16Value = u16Value;
    stSetReg.u16Mask = u16Mask;
    s32Ret = MI_SYSCALL(MI_SYS_SET_REG, &stSetReg);
    return s32Ret;
}

MI_S32 MI_SYS_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetReg_t stGetReg;
    stGetReg.u32RegAddr = u32RegAddr;
    s32Ret = MI_SYSCALL(MI_SYS_GET_REG, &stGetReg);
    *pu16Value = stGetReg.u16Value;
    return s32Ret;
}

MI_S32 MI_SYS_SetChnMMAConf (MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId,MI_U8 *pu8MMAHeapName)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetChnPortMMAConf_t stMmaConf;
    stMmaConf.eModId = eModId;
    stMmaConf.u32DevId = u32DevId;
    stMmaConf.u32ChnId = u32ChnId;
    memcpy(&stMmaConf.u8MMAHeapName , pu8MMAHeapName, MAX_MMA_HEAP_NAME_LENGTH);
    s32Ret = MI_SYSCALL(MI_SYS_SET_CHN_MMA_CONF, &stMmaConf);
    return s32Ret;
}
#if 1
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })
#endif
MI_S32 MI_SYS_GetChnMMAConf (MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId, void  *pu8MMAHeapName, MI_U32 u32Length)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetChnPortMMAConf_t stMmaConf;
    stMmaConf.eModId = eModId;
    stMmaConf.u32DevId = u32DevId;
    stMmaConf.u32ChnId = u32ChnId;

    stMmaConf.u32Length = u32Length;
    s32Ret = MI_SYSCALL(MI_SYS_GET_CHN_MMA_CONF, &stMmaConf);
    memcpy(pu8MMAHeapName , stMmaConf.u8Data , min(u32Length,(MI_U32)MAX_MMA_HEAP_NAME_LENGTH) );
    return s32Ret;
}

MI_S32 MI_SYS_ChnInputPortGetBuf (MI_SYS_ChnPort_t *pstChnPort ,MI_SYS_BufConf_t *pstBufConf, MI_SYS_BufInfo_t *pstBufInfo , MI_SYS_BUF_HANDLE *pBufHandle , MI_S32 s32TimeOutMs)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnInputPortGetBuf_t stChnPortGetBuf;
    stChnPortGetBuf.s32TimeOutMs = s32TimeOutMs;
    memcpy(&stChnPortGetBuf.stChnPort , pstChnPort , sizeof(MI_SYS_ChnPort_t));
    memcpy(&stChnPortGetBuf.stBufConf , pstBufConf , sizeof(MI_SYS_BufConf_t));
    stChnPortGetBuf.u32ExtraFlags = MI_SYS_MAP_VA;
    s32Ret = MI_SYSCALL(MI_SYS_CHN_INPUT_PORT_GET_BUF, &stChnPortGetBuf);
    memcpy(pstBufInfo , &stChnPortGetBuf.stBufInfo , sizeof(MI_SYS_BufInfo_t));
    *pBufHandle = stChnPortGetBuf.BufHandle;
    return s32Ret;
}

MI_S32 MI_SYS_ChnInputPortPutBuf (MI_SYS_BUF_HANDLE bufHandle ,  MI_SYS_BufInfo_t *pstBufInfo , MI_BOOL bDropBuf)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnInputPortPutBuf_t stChnPortPutBuf;
    stChnPortPutBuf.BufHandle = bufHandle;
    stChnPortPutBuf.stBufInfo = *pstBufInfo;
    stChnPortPutBuf.bDropBuf = bDropBuf;
    s32Ret = MI_SYSCALL(MI_SYS_CHN_INPUT_PORT_PUT_BUF, &stChnPortPutBuf);
    return s32Ret;
}

MI_S32 MI_SYS_ChnOutputPortGetBuf (MI_SYS_ChnPort_t *pstChnPort,MI_SYS_BufInfo_t *pstBufInfo ,MI_SYS_BUF_HANDLE *pBufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnOutputPortGetBuf_t stChnPortGetBuf;
    memcpy(&stChnPortGetBuf.stChnPort , pstChnPort , sizeof(MI_SYS_ChnPort_t));
    stChnPortGetBuf.u32ExtraFlags = MI_SYS_MAP_VA | MI_SYS_MAP_CPU_READ;

    s32Ret = MI_SYSCALL(MI_SYS_CHN_OUTPUT_PORT_GET_BUF, &stChnPortGetBuf);
    memcpy(pstBufInfo , &stChnPortGetBuf.stBufInfo , sizeof(MI_SYS_BufInfo_t));
    *pBufHandle = stChnPortGetBuf.BufHandle;
    return s32Ret;
}

MI_S32 MI_SYS_ChnOutputPortPutBuf (MI_SYS_BUF_HANDLE bufHandle)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(MI_SYS_CHN_OUTPUT_PORT_PUT_BUF, &bufHandle);
    return s32Ret;
}

MI_S32 MI_SYS_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_SetChnOutputPortDepth_t stChnPortDepth;
    memcpy(&stChnPortDepth.stChnPort , pstChnPort , sizeof(MI_SYS_ChnPort_t));
    stChnPortDepth.u32BufQueueDepth = u32BufQueueDepth;
    stChnPortDepth.u32UserFrameDepth = u32UserFrameDepth;
    s32Ret = MI_SYSCALL(MI_SYS_SET_CHN_OUTPUT_PORT_DEPTH, &stChnPortDepth);
    return s32Ret;
}

MI_S32 MI_SYS_GetFd(MI_SYS_ChnPort_t *pstChnPort , MI_S32 *s32Fd)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_GetFd_t stGetFd;
    stGetFd.stChnPort = *pstChnPort;

#ifndef __KERNEL__
    s32Ret=MI_SYSCALL(MI_SYS_GET_FD, &stGetFd);
    *s32Fd = stGetFd.s32Fd;
#else
    *s32Fd = -1;
#endif

    return s32Ret;
}

MI_S32 MI_SYS_CloseFd(MI_S32 s32ChnPortFd)
{
    MI_S32 s32Fd = MI_SUCCESS;
#ifndef __KERNEL__
    if (close(s32ChnPortFd))
    {
        s32Fd = E_MI_ERR_FAILED;
    }
#endif
    return s32Fd;
}

MI_S32 MI_SYS_ChnPortInjectBuf(MI_SYS_BUF_HANDLE hBufHandle ,MI_SYS_ChnPort_t *pstChnPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPortInjectBuf_t stChnBufBlk;
    stChnBufBlk.BufHandle = hBufHandle;
    stChnBufBlk.stChnPort = *pstChnPort;

    s32Ret = MI_SYSCALL(MI_SYS_CHN_PORT_INJECT_BUF , &stChnBufBlk);
    return s32Ret;
}

MI_S32 MI_SYS_GetBindbyDest (MI_SYS_ChnPort_t *pstDstChnPort, MI_SYS_ChnPort_t *pstSrcChnPort)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_ChnPortArg_t stChnPortArg;
    memcpy(&stChnPortArg.stDstChnPort , pstDstChnPort , sizeof(MI_SYS_ChnPort_t));
    s32Ret = MI_SYSCALL(MI_SYS_GET_BIND_BY_DEST, &stChnPortArg);
    if (s32Ret == MI_SUCCESS)
    {
        memcpy(pstSrcChnPort, &stChnPortArg.stSrcChnPort, sizeof(MI_SYS_ChnPort_t));
    }
    return s32Ret;
}

MI_S32 MI_SYS_MMA_Alloc(MI_U8 *szMMAHeapName, MI_U32  blkSize ,MI_PHY *pu64PhyAddr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Mma_Alloc_t stMmaAlloc;
    memset(stMmaAlloc.szMMAHeapName, 0, MI_SYS_MMA_HEAP_NAME_MAX_LEN);

    //fill input
    if (szMMAHeapName)
        strncpy(stMmaAlloc.szMMAHeapName, szMMAHeapName, MI_SYS_MMA_HEAP_NAME_MAX_LEN);

    stMmaAlloc.blkSize = blkSize;
    s32Ret =   MI_SYSCALL(MI_SYS_MMA_ALLOC,&stMmaAlloc);
    //fill output
    if (s32Ret == MI_SUCCESS)
    {
        *pu64PhyAddr = stMmaAlloc.pu64PhyAddr;
    }
    return s32Ret;
}

MI_S32 MI_SYS_MMA_Free(MI_PHY  phyAddr)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_Mma_Free_t stMmaFree;
    stMmaFree.phyAddr = phyAddr;
    s32Ret =  MI_SYSCALL(MI_SYS_MMA_FREE,&stMmaFree);
    return s32Ret;
}

#if 0
MI_VOID * MI_SYS_Mmap(MI_U64 u64PhyAddr, MI_U32 u32Size,MI_BOOL map_va_cacheable)
{
    MI_SYSCALL(MI_SYS_MMAP,);
}
#endif

MI_S32 MI_SYS_FlushInvCache(void *pVirtualAddress, MI_U32 u32Length)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_FlushInvCache_t stFlushInvCache;
    stFlushInvCache.pVirtualAddress = pVirtualAddress;
    stFlushInvCache.length= u32Length;
    s32Ret =  MI_SYSCALL(MI_SYS_FLUSH_INV_CACHE,&stFlushInvCache);
    return s32Ret;
}

#if 0
MI_S32  MI_SYS_Munmap(MI_VOID  *pVirtualAddress, MI_U32 u32Size)
{
    MI_SYSCALL(MI_SYS_MMA_MUNMAP,);

}
#endif

MI_S32 MI_SYS_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId, MI_VB_PoolListConf_t  stPoolListConf)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    static MI_SYS_ConfDevPubPools_t stConfDevPubPools;
    stConfDevPubPools.eModule = eModule;
    stConfDevPubPools.stPoolListConf = stPoolListConf;
    stConfDevPubPools.u32DevId = u32DevId;
    s32Ret =   MI_SYSCALL(MI_SYS_CONF_DEV_PUB_POOLS,&stConfDevPubPools);
    return s32Ret;
}

MI_S32 MI_SYS_ReleaseDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_RelDevPubPools_t stRelDevPubPools;
    stRelDevPubPools.eModule = eModule;
    stRelDevPubPools.u32DevId = u32DevId;
    s32Ret =   MI_SYSCALL(MI_SYS_REL_DEV_PUB_POOLS,&stRelDevPubPools);
    return s32Ret;
}

MI_S32 MI_SYS_ConfGloPubPools(MI_VB_PoolListConf_t  stPoolListConf)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    static MI_SYS_ConfGloPubPools_t stConfGloPubPools;
    stConfGloPubPools.stPoolListConf = stPoolListConf;
    s32Ret =   MI_SYSCALL(MI_SYS_CONF_GLO_PUB_POOLS,&stConfGloPubPools);
    return s32Ret;
}

MI_S32 MI_SYS_ReleaseGloPubPools(void)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 no_use = 0;
    s32Ret =   MI_SYSCALL(MI_SYS_REL_GLO_PUB_POOLS,&no_use);
    return s32Ret;
}

MI_S32 MI_SYS_Set_Vdec_UseVbPool_Flag(MI_U32 u32Value)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    s32Ret = MI_SYSCALL(E_MI_SYS_CMD_VDEC_USE_VBPOOL, &u32Value);
    return s32Ret;
}
#ifndef __KERNEL__
#ifdef DEBUG_YUV_USER_API

/*
return :
FILE_HANDLE type in fact is a pointer,
NULL:fail
not NULL:success
*/
FILE_HANDLE open_yuv_file(const char *pathname, int open_for_write)
{
    FILE_HANDLE filehandle;
    // mm_segment_t fs = get_fs();
    //set_fs(KERNEL_FS);

    //"w":means only write.If file exist,will clear size to 0,and if file not exist,will  create file.
    //"r":means only read,and file must exist.
    filehandle = fopen(pathname, open_for_write?"w":"r");
    //set_fs(fs);
    if(filehandle == NULL)
    {
        printf("%s:%d fopen fail\n",__FUNCTION__,__LINE__);
    }
    return (FILE_HANDLE)filehandle;
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
    int i;

    if(filehandle == NULL)
    {
        printf("%s:%d filehandle is NULL invalid parameter!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    if(framedata.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && framedata.ePixelFormat !=E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        {
            printf("%s:%d ePixelFormat %d not support!\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
            return -1;
        }

    width = framedata.u16Width;
    height = framedata.u16Height;

    switch (framedata.ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(fread((char *)dst_buf, 1, width*2,filehandle) != width*2)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            return 0;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(fread((char *)dst_buf, 1, width*1, filehandle) != width*1)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            dst_buf = framedata.pVirAddr[1];
            for( i=0;i<height; i++)
            {
                if(fread( (char *)dst_buf, 1, width*1,filehandle) != width*1)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[1];
            }

        return 0;
    }

ERR_RET:
    printf("%s:%d fail\n",__FUNCTION__,__LINE__);
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

    if(!filehandle)
    {
        printf("%s:%d filehandle is NULL,invalid parameter!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    if(framedata.ePixelFormat != E_MI_SYS_PIXEL_FRAME_YUV422_YUYV && framedata.ePixelFormat !=E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        printf("%s:%d ePixelFormat %d not support!\n",__FUNCTION__,__LINE__,framedata.ePixelFormat);
        return -1;
    }
    width = framedata.u16Width;
    height = framedata.u16Height;

    switch (framedata.ePixelFormat)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(fwrite((char *)dst_buf, 1 , width*2,filehandle) != width*2)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            return 0;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            dst_buf = framedata.pVirAddr[0];
            for( i=0;i<height; i++)
            {
                if(fwrite((char *)dst_buf,1 , width,filehandle) != width)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[0];
            }
            dst_buf = framedata.pVirAddr[1];
            for( i=0;i<height; i++)
            {
                if(fwrite( (char *)dst_buf,1 , width,filehandle) != width)
                    goto ERR_RET;
                dst_buf+=framedata.u32Stride[1];
            }
            return 0;
    }
ERR_RET:
    printf("%s:%d fail\n",__FUNCTION__,__LINE__);
    return -1;
}
void close_yuv_file(FILE_HANDLE filehandle)
{
    fclose(filehandle);
}

//reset to file begin
/*
return value:
0:success
-1:fail
*/
int reset_yuv_file(FILE_HANDLE filehandle)
{
    off_t res ;

    //return value of fseek:0 means success,and -1 means fail.
    res = fseek(filehandle, 0, SEEK_SET);//reset to file begin
    return res;
}

/*
return value:
0:success
-1:fail
*/
int is_in_yuv_file_end(FILE_HANDLE filehandle)
{
    off_t res_cur ;
    off_t res_end ;
    res_cur = fseek(filehandle, 0, SEEK_CUR);//get and save current pos.
    if (res_cur <0)
    {
        printf("%s:%d  fail,error",__FUNCTION__,__LINE__);
        return 0;//do nothing,return false.
    }
    res_end = fseek(filehandle, 0, SEEK_END);//get end pos.
    fseek(filehandle, res_cur, SEEK_SET);//after seek end ,and before return , restore to current pos


    if (res_end < 0)
    {
        printf("%s:%d  fail,error",__FUNCTION__,__LINE__);
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
#else
/*
do nothing
*/
#endif

EXPORT_SYMBOL(MI_SYS_Init);
EXPORT_SYMBOL(MI_SYS_Exit);
EXPORT_SYMBOL(MI_SYS_BindChnPort);
EXPORT_SYMBOL(MI_SYS_UnBindChnPort);
EXPORT_SYMBOL(MI_SYS_GetBindbyDest);
EXPORT_SYMBOL(MI_SYS_GetVersion);
EXPORT_SYMBOL(MI_SYS_GetCurPts);
EXPORT_SYMBOL(MI_SYS_InitPtsBase);
EXPORT_SYMBOL(MI_SYS_SyncPts);
EXPORT_SYMBOL(MI_SYS_Mmap);
EXPORT_SYMBOL(MI_SYS_Munmap);
EXPORT_SYMBOL(MI_SYS_SetReg);
EXPORT_SYMBOL(MI_SYS_GetReg);
EXPORT_SYMBOL(MI_SYS_SetChnMMAConf);
EXPORT_SYMBOL(MI_SYS_GetChnMMAConf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortDepth);
EXPORT_SYMBOL(MI_SYS_GetFd);
EXPORT_SYMBOL(MI_SYS_CloseFd);
EXPORT_SYMBOL(MI_SYS_ChnPortInjectBuf);
EXPORT_SYMBOL(MI_SYS_MMA_Alloc);
EXPORT_SYMBOL(MI_SYS_MMA_Free);
EXPORT_SYMBOL(MI_SYS_FlushInvCache);
EXPORT_SYMBOL(MI_SYS_ConfDevPubPools);
EXPORT_SYMBOL(MI_SYS_ReleaseDevPubPools);
EXPORT_SYMBOL(MI_SYS_ConfGloPubPools);
EXPORT_SYMBOL(MI_SYS_ReleaseGloPubPools);

#ifndef __KERNEL__
#ifdef DEBUG_YUV_USER_API
EXPORT_SYMBOL(open_yuv_file);
EXPORT_SYMBOL(read_yuv_file);
EXPORT_SYMBOL(write_yuv_file);
EXPORT_SYMBOL(close_yuv_file);
EXPORT_SYMBOL(reset_yuv_file);
EXPORT_SYMBOL(is_in_yuv_file_end);
#endif
#else
/*
do nothing
*/
#endif
