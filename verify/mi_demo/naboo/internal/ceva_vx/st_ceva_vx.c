#include <pthread.h>
#include <sys/prctl.h>
#include <errno.h>
#include <assert.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <pthread.h>


#include "VX/vx.h"
#include "VX/vxu.h"
#include <VX/vx_ceva_kernels.h>
#include <VX/vx_ceva_nodes.h>
#include "VX/vx_lib_debug.h"
#include "VX/vx_helper.h"
#include "VX/vx_api.h"
#include "ceva_boot.h"
#include "st_common.h"
#include "st_ceva_vx.h"
#include "i2c.h"


#define ST_CEVA_VX_GET_PA_BUF
//#define ST_CEVA_VX_DEBUG_LOG
//#define ST_CEVA_VX_DUMP_FILE
#define ST_CEVA_VX_STAT_TIME
//#define PROC_BY_MEMCPY


#define CEVA_XM6_DEV_NAME       "/dev/ceva_linkdrv_xm6_0"
#define SHARE_MEM_SIZE          1920*1088*2       // align by 4*1024


#define CHECK_HANDLE_VALID(handle) do{ \
    if (handle < 0) \
    { \
        CEVA_VX_LOG_ERR("invalid handle"); \
        return -1;  \
    } \
) while(0);

#define CHECK_ALL_ITEMS(array, iter, status, label) { \
    status = VX_SUCCESS; \
    for ((iter) = 0; (iter) < dimof(array); (iter)++) { \
        if ((array)[(iter)] == 0) { \
            CEVA_VX_LOG_ERR("Item %u in "#array" is null!\n", (iter)); \
            assert((array)[(iter)] != 0); \
            status = VX_ERROR_NOT_SUFFICIENT; \
                                        } \
                    } \
    if (status != VX_SUCCESS) { \
        goto label; \
                    } \
}


// stat time
typedef struct ST_CalcSpendTime_s
{
    struct timeval stStartTmVal;
    MI_U32 u32TotalTime;
    MI_U32 u32MaxSpendTime;
    MI_U32 u32MinSpendTime;
    MI_U32 u32CurSpendTime;
}ST_CalcSpendTime_t;

static MI_U32 StatSpendTime(struct timeval *pstTimeStart)
{
    struct timeval stCurTime;
    gettimeofday(&stCurTime, NULL);

    return 1000000*(stCurTime.tv_sec - pstTimeStart->tv_sec) + (stCurTime.tv_usec - pstTimeStart->tv_usec);
}

void ST_InitialTimeSpend(ST_CalcSpendTime_t *pCalcTimeSpend)
{
    pCalcTimeSpend->u32TotalTime = 0;
    pCalcTimeSpend->u32MaxSpendTime = 0;
    pCalcTimeSpend->u32MinSpendTime = 0xffffffff;
    pCalcTimeSpend->u32CurSpendTime = 0;
}

void ST_CalcTimeSpend(ST_CalcSpendTime_t *pCalcSpendTime)
{
    pCalcSpendTime->u32CurSpendTime = StatSpendTime(&pCalcSpendTime->stStartTmVal);
    if (pCalcSpendTime->u32MaxSpendTime < pCalcSpendTime->u32CurSpendTime)
        pCalcSpendTime->u32MaxSpendTime = pCalcSpendTime->u32CurSpendTime;

    if (pCalcSpendTime->u32MinSpendTime > pCalcSpendTime->u32CurSpendTime)
        pCalcSpendTime->u32MinSpendTime = pCalcSpendTime->u32CurSpendTime;

    pCalcSpendTime->u32TotalTime += pCalcSpendTime->u32CurSpendTime;
}

// ceva vx
typedef MI_S32 CEVA_VX_DEV_HANDLE;

typedef struct {
    vx_context context;
    vx_graph graph;
    vx_node nodes[1];
    vx_image images[2];
    vx_scalar scale;
} ST_CEVA_VX_ProcHandle_t;

typedef enum
{
    E_CEVA_VX_RES_640X480,
    E_CEVA_VX_RES_1920X1080,
    E_CEVA_VX_RES_3840X2160,
    E_CEVA_VX_RES_MAX_NUM
}ST_CEVA_VX_Resolution_e;

typedef struct
{
    MI_BOOL bFilled;
    MI_U32 u32Size;         // max size
    MI_U32 u32Length;       // fill length
    MI_PHY phyAddr;
    void *pVirAddr;
    pthread_mutex_t mtxShareMem;
}ST_CEVA_VX_ShareMem_t;

//static struct boot_config_t _gBootConfig;
static ST_CEVA_VX_ImageAttr_t _gstImageAttr;
static MI_SYS_ChnPort_t _gstSinkChnPort;
static MI_SYS_ChnPort_t _gstInjectChnPort;
static pthread_t _gVxThread = NULL;
static MI_BOOL _gbVxThreadExit = FALSE;
static MI_S32 _gs32TestCnt = 0;
static MI_BOOL _gbLogOn = TRUE;
static ST_CEVA_VX_ShareMem_t _gstShareMem;

//static ceva_boot_info _gstBootInfo = {"image.bin", // DSP image
//                                0x5FB00000,  // DSP code address (physical)
//                                0x500000,    // DSP code size
//                                0x1000000,   // External heap
//                                0x200000,    // Share memory
//                                0,           // Working buffer
//                                0};          // Custom information


#define CEVA_VX_LOG_ERR(fmt, args...) {printf("\033[1;31m");printf(fmt, ##args);printf("\033[0m");}
#define CEVA_VX_LOG_INFO(fmt, args...) {if(_gbLogOn) {printf("\033[1;34m");printf(fmt, ##args);printf("\033[0m");}}


MI_S32 ST_CEVA_VX_DspBootUp(const MI_S8 *pDspIniPath)
{
    MI_S32 s32Ret = CEVABOOT_API_SUCCESS;

//    s32Ret= ceva_boot_up_dsp(&_gstBootInfo);
    s32Ret= ceva_boot_up_dsp(pDspIniPath);

    if (s32Ret != CEVABOOT_API_SUCCESS)
    {
        CEVA_VX_LOG_ERR("can't boot up DSP, ret is %d\n", s32Ret);
    }

    return s32Ret;
}

MI_S32 ST_CEVA_VX_DspShutDown(void)
{
    MI_S32 s32Ret = CEVABOOT_API_SUCCESS;

//    s32Ret = ceva_shut_down_dsp(&_gstBootInfo);
    s32Ret = ceva_shut_down_dsp();

    if (s32Ret != CEVABOOT_API_SUCCESS)
    {
        CEVA_VX_LOG_ERR("can't shut down DSP, ret is %d\n", s32Ret);
    }

    return s32Ret;
}

vx_status _CEVA_VX_SetImagePatchAddr(vx_imagepatch_addressing_t*  addr, vx_uint32 width, vx_uint32 height, vx_uint32 depth)
{
    vx_status status = VX_SUCCESS;

    addr->dim_x = width;
    addr->dim_y = height;

    if( (depth%8) != 0 ) {
        status = VX_FAILURE;
        CEVA_VX_LOG_ERR("Address pixels failed in image patch (%d)\n", status);
        return status;
    }
    addr->stride_x = (depth/8)*sizeof(vx_uint8);
    addr->stride_y = (depth/8)*sizeof(vx_uint8);
    addr->scale_x = VX_SCALE_UNITY;
    addr->scale_y = VX_SCALE_UNITY;
    addr->step_x = 1;
    addr->step_y = 1;

#ifdef ST_CEVA_VX_DEBUG_LOG
    printf("/////_CEVA_VX_SetImagePatchAddr : %p/////\n",addr);
    printf("dim_x : %d\n",addr->dim_x);
    printf("dim_y: %d\n",addr->dim_y);
    printf("stride_x : %d\n",addr->stride_x);
    printf("stride_y : %d\n",addr->stride_y);
    printf("scale_x  : %d\n",addr->scale_x);
    printf("scale_y  : %d\n",addr->scale_y);
    printf("step_x   : %d\n",addr->step_x);
    printf("step_y   : %d\n",addr->step_y);
#endif

    return status;
}

//===========================================================================================
vx_status _ST_CEVA_VX_Init(ST_CEVA_VX_ProcHandle_t *pHandle, vx_uint32 width, vx_uint32 height, vx_uint32 depth)
{
    vx_status status = VX_FAILURE;
    vx_uint32 tile_w = 128;
    vx_uint32 tile_h = 60;
    vx_uint32 i = 0;
    vx_int32 kernel_size = 3;
//    void *input[1] = {NULL};
//    void *output[1] = {NULL};
    MI_PHY input[1] = {0};
    MI_PHY output[1] = {0};

    vx_imagepatch_addressing_t src_addr_pixel, dst_addr_pixel;

    memset(pHandle, 0, sizeof(*pHandle));

    // create context
    pHandle->context = vxCreateContext();
    if ((status = vxGetStatus((vx_reference)pHandle->context)) != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxCreateContext failed, vxGetStatus: %d\n", status);
        return status;
    }

    status = vxLoadKernels(pHandle->context, "openvx-debug");
    status |= vxLoadKernels(pHandle->context, "openvx-cevacv");
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxLoadKernels failed, status: %d\n", status);
        goto EXIT_0;
    }

    pHandle->graph = vxCreateGraph(pHandle->context);
    if ((status = vxGetStatus((vx_reference)pHandle->graph)) != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxCreateGraph failed, status: %d\n");
        goto EXIT_0;
    }

    //adress pixel
    status =  _CEVA_VX_SetImagePatchAddr(&src_addr_pixel, width, height, depth);
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vx_ceva_set_imagepatch_addr src failed, status: %d\n");
        goto EXIT_1;
    }

    status =  _CEVA_VX_SetImagePatchAddr(&dst_addr_pixel, width, height, depth);
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vx_ceva_set_imagepatch_addr dst failed, status: %d\n");
        goto EXIT_1;
    }

//    pHandle->images[0] = vxCreateImageFromHandle(pHandle->context, VX_DF_IMAGE_U8, &src_addr_pixel, input, VX_IMPORT_TYPE_HOST_PHYS);
//    pHandle->images[1] = vxCreateImageFromHandle(pHandle->context, VX_DF_IMAGE_U8, &dst_addr_pixel, output, VX_IMPORT_TYPE_HOST_PHYS);
    pHandle->images[0] = vxCreateImageFromHandle(pHandle->context, VX_DF_IMAGE_U8, &src_addr_pixel, (void**)input, VX_IMPORT_TYPE_HOST_MIU);
    pHandle->images[1] = vxCreateImageFromHandle(pHandle->context, VX_DF_IMAGE_U8, &dst_addr_pixel, (void**)output, VX_IMPORT_TYPE_HOST_MIU);

    CHECK_ALL_ITEMS(pHandle->images, i, status, EXIT_1);

    //set tile size
    for (i = 0; i < dimof(pHandle->images); i++)
    {
        vxSetImageAttribute(pHandle->images[i], VX_IMAGE_ATTRIBUTE_TILE_WIDTH, &tile_w, sizeof(tile_w));
        vxSetImageAttribute(pHandle->images[i], VX_IMAGE_ATTRIBUTE_TILE_HEIGHT, &tile_h, sizeof(tile_h));
    }

    // create node
    pHandle->nodes[0] = vxCevaCVGaussianNode(pHandle->graph, pHandle->images[0], pHandle->images[1], kernel_size);
    status = vxGetStatus((vx_reference)pHandle->nodes[0]);
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxCevaCVGaussianNode failed, status: %d\n");
        goto EXIT_2;
    }

    vx_border_mode_t border = { VX_BORDER_MODE_CONSTANT, 0 };
    status |= vxSetNodeAttribute(pHandle->nodes[0], VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(vx_border_mode_t));
    if (status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxSetNodeAttribute failed, status: %d\n");
        goto EXIT_3;
    }

    status = vxVerifyGraph(pHandle->graph);
    if (status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxVerifyGraph failed, status: %d\n");
        goto EXIT_3;
    }

    return status;

EXIT_3:
    for (i = 0; i < dimof(pHandle->nodes); i++)
    {
        vxReleaseNode(&pHandle->nodes[i]);
    }

EXIT_2:
    for (i = 0; i < dimof(pHandle->images); i++)
    {
        vxReleaseImage(&pHandle->images[i]);
    }

EXIT_1:
    vxReleaseGraph(&pHandle->graph);

EXIT_0:
    vxReleaseContext(&pHandle->context);

    return status;
}

void _ST_CEVA_VX_Release(ST_CEVA_VX_ProcHandle_t *pHandle)
{
    int i;

    for (i = 0; i < dimof(pHandle->nodes); i++)
    {
        vxReleaseNode(&pHandle->nodes[i]);
    }

    for (i = 0; i < dimof(pHandle->images); i++)
    {
        vxReleaseImage(&pHandle->images[i]);
    }

    vxReleaseGraph(&pHandle->graph);

    vxReleaseContext(&pHandle->context);
}

// input: src miu_addr, output: dst miu_addr
vx_status _ST_CEVA_VX_Process(ST_CEVA_VX_ProcHandle_t *pHandle, MI_PHY *input, MI_PHY *output)
{
    vx_status status = VX_FAILURE;

    status = vxSetImageAttribute(pHandle->images[0], VX_IMAGE_ATTRIBUTE_BUFFER_HANDLE, (void*)input, sizeof(vx_uint8*));
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxSetImageAttribute input failed, status: %d\n");
        return status;
    }

    status = vxSetImageAttribute(pHandle->images[1], VX_IMAGE_ATTRIBUTE_BUFFER_HANDLE, (void*)output, sizeof(vx_uint8*));
    if(status != VX_SUCCESS){
        CEVA_VX_LOG_ERR("vxSetImageAttribute output failed, status: %d\n");
        return status;
    }

    CEVA_VX_LOG_INFO("start vxProcessGraph\n");
    status = vxProcessGraph(pHandle->graph);
    CEVA_VX_LOG_INFO("end vxProcessGraph\n");

    return status;
}

void *_CEVA_VX_WorkThreadToUsr(void *argv)
{
    ST_CEVA_VX_ShareMem_t *pShareMem = (ST_CEVA_VX_ShareMem_t*)argv;
    vx_status status;
    ST_CEVA_VX_ProcHandle_t handle;
    MI_S32 s32Fd = 0;
    MI_S32 s32Ret;
    MI_S32 s32TimeOutMs = 100;
    struct pollfd aFd[1] = { {s32Fd, POLLIN|POLLERR} };
    MI_SYS_BufInfo_t stSinkBufInfo;
    MI_SYS_BUF_HANDLE hSinkBufHandle;
    MI_SYS_BufConf_t stInjectBufConf;
    void *pOutVirAddr;
    void *pOutVirtY;
    void *pOutVirtUV;
    MI_S32 s32Planar0Size = 0;
    MI_S32 s32Planar1Size = 0;

#ifdef ST_CEVA_VX_DUMP_FILE
    FILE *pGetBufFile = NULL;
    FILE *pPutBufFile = NULL;
    char szGetBufFilePath[50];
    char szPutBufFilePath[50];
    MI_S32 s32FileIndex = 0;
#endif

    if (_gstImageAttr.eFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
        && _gstImageAttr.eFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        CEVA_VX_LOG_ERR("Only support NV12 & NV16, format %d not support\n", _gstImageAttr.eFormat);
        return NULL;
    }

    _gstImageAttr.src[0] = NULL;

    MI_SYS_SetChnOutputPortDepth(&_gstSinkChnPort, 2, 7);

    s32Ret = MI_SYS_GetFd(&_gstSinkChnPort, &s32Fd);
    if(MI_SUCCESS != s32Ret || s32Fd < 0)
    {
        CEVA_VX_LOG_ERR("MI_SYS_GetFd fail\n");
        return NULL;
    }

    aFd[0].fd = s32Fd;

    // ceva_vx init
    status = _ST_CEVA_VX_Init(&handle, _gstImageAttr.width, _gstImageAttr.height, _gstImageAttr.depth);
    if (status != VX_SUCCESS)
    {
        CEVA_VX_LOG_ERR("Can't init ceva sample, return %d\n", status);
        return NULL;
    }

    while(1)
    {
        if (_gbVxThreadExit)
            break;

#ifdef ST_CEVA_VX_DUMP_FILE
        s32FileIndex++;
#endif

retry:

        // recv date
        s32Ret = poll(aFd, 1, s32TimeOutMs);
        if (s32Ret>0 && (aFd[0].revents & POLLIN))
        {
            //CEVA_VX_LOG_INFO("Thread: recv data !!!\n");
        }
        else
        {
            goto retry;
        }

        // process image
        // 1. get vpe outport Buf
#ifdef ST_CEVA_VX_GET_PA_BUF
        s32Ret = MI_SYS_ChnOutputPortGetPABuf(&_gstSinkChnPort, &stSinkBufInfo, &hSinkBufHandle);
#else
        s32Ret = MI_SYS_ChnOutputPortGetBuf(&_gstSinkChnPort, &stSinkBufInfo, &hSinkBufHandle);
#endif

        if (MI_SUCCESS == s32Ret)
        {
            if (_gstImageAttr.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
            {
                s32Planar0Size = stSinkBufInfo.stFrameData.u32Stride[0] * stSinkBufInfo.stFrameData.u16Height;
                s32Planar1Size = stSinkBufInfo.stFrameData.u32Stride[1] * stSinkBufInfo.stFrameData.u16Height / 2;
            }
            else
            {
                s32Planar0Size = stSinkBufInfo.stFrameData.u32Stride[0] * stSinkBufInfo.stFrameData.u16Height;
                s32Planar1Size = stSinkBufInfo.stFrameData.u32Stride[1] * stSinkBufInfo.stFrameData.u16Height;
            }

            CEVA_VX_LOG_INFO("bufSize=%d s32Planar0Size=%d s32Planar1Size=%d\n", stSinkBufInfo.stFrameData.u32BufSize, s32Planar0Size, s32Planar1Size);

#ifdef ST_CEVA_VX_GET_PA_BUF
            s32Ret = MI_SYS_Mmap(stSinkBufInfo.stFrameData.phyAddr[0], stSinkBufInfo.stFrameData.u32BufSize, &pOutVirAddr, FALSE);
            if (MI_SUCCESS != s32Ret)
            {
                CEVA_VX_LOG_ERR("mmap sink port buf failed\n");
                goto put_sinkPort_buf;
            }

            pOutVirtY = pOutVirAddr;
            if (_gstImageAttr.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                pOutVirtUV = pOutVirAddr + stSinkBufInfo.stFrameData.u32BufSize * 2/3;      // nv12
            else
                pOutVirtUV = pOutVirAddr + stSinkBufInfo.stFrameData.u32BufSize * 1/2;      // nv16
#else
            pOutVirtY = stSinkBufInfo.stFrameData.pVirAddr[0];
            pOutVirtUV = stSinkBufInfo.stFrameData.pVirAddr[1];
            pOutVirAddr = pOutVirtY;
#endif

#ifdef ST_CEVA_VX_DUMP_FILE
            // dump sink port buf
            memset(szGetBufFilePath, 0, sizeof(szGetBufFilePath));
            sprintf(szGetBufFilePath, "GetBuf_%d.yuv", s32FileIndex);
            pGetBufFile = fopen(szGetBufFilePath, "ab+");
            if (pGetBufFile)
            {
                fwrite(pOutVirAddr, stSinkBufInfo.stFrameData.u32BufSize, 1, pGetBufFile);
                fclose(pGetBufFile);
                pGetBufFile = NULL;
            }
#endif

            _gstImageAttr.src[0] = stSinkBufInfo.stFrameData.phyAddr[0];

            while (1)
            {
                if (_gbVxThreadExit)
#ifdef ST_CEVA_VX_GET_PA_BUF
                    goto umap_sinkPort_Buf;
#else
                    goto put_sinkPort_buf;
#endif

                pthread_mutex_lock(&pShareMem->mtxShareMem);
                if (!_gstShareMem.bFilled)
                {
                    memset(_gstShareMem.pVirAddr, 0, SHARE_MEM_SIZE);
                    _gstShareMem.u32Length = 0;

                    status = _ST_CEVA_VX_Process(&handle, _gstImageAttr.src, _gstImageAttr.dst);
                    if (status != VX_SUCCESS)
                    {
                        CEVA_VX_LOG_ERR("vx process failed : %d\n", status);

                        memset(_gstShareMem.pVirAddr, 0, SHARE_MEM_SIZE);
                        pthread_mutex_unlock(&pShareMem->mtxShareMem);

#ifdef ST_CEVA_VX_GET_PA_BUF
                        goto umap_sinkPort_Buf;
#else
                        goto put_sinkPort_buf;
#endif
                    }

                    memcpy(_gstShareMem.pVirAddr+(pOutVirtUV-pOutVirtY), pOutVirtUV, s32Planar1Size);

                    _gstShareMem.u32Length = stSinkBufInfo.stFrameData.u32BufSize;
                    _gstShareMem.bFilled = TRUE;

#ifdef ST_CEVA_VX_DUMP_FILE
                    // dump inject port buf
                    memset(szPutBufFilePath, 0, sizeof(szPutBufFilePath));
                    sprintf(szPutBufFilePath, "PutBuf_%d.yuv", s32FileIndex);
                    pPutBufFile = fopen(szPutBufFilePath, "ab+");
                    if (pPutBufFile)
                    {
                        fwrite(_gstShareMem.pVirAddr, _gstShareMem.u32Length, 1, pPutBufFile);
                        fclose(pPutBufFile);
                        pPutBufFile = NULL;
                    }
#endif
                    pthread_mutex_unlock(&pShareMem->mtxShareMem);

                    break;
                }
                pthread_mutex_unlock(&pShareMem->mtxShareMem);

                usleep(1000);
            }

#ifdef ST_CEVA_VX_GET_PA_BUF
umap_sinkPort_Buf:
            MI_SYS_Munmap(pOutVirAddr, stSinkBufInfo.stFrameData.u32BufSize);
#endif

put_sinkPort_buf:
            MI_SYS_ChnOutputPortPutBuf(hSinkBufHandle);

        }
        else
        {
            CEVA_VX_LOG_ERR("Get output port buf failed\n");
        }
    }

    _ST_CEVA_VX_Release(&handle);

    MI_SYS_CloseFd(s32Fd);

    return NULL;
}

void *_CEVA_VX_WorkThread(void *argv)
{
    vx_status status;
    ST_CEVA_VX_ProcHandle_t handle;

    MI_S32 s32TimeOutMs = 100;
    MI_S32 s32Fd = 0;
    struct pollfd aFd[1] = { {s32Fd, POLLIN|POLLERR} };

    MI_S32 s32Ret;
    MI_SYS_BufInfo_t stSinkBufInfo, stInjectBufInfo;
    MI_SYS_BUF_HANDLE hSinkBufHandle, hInjectBufHandle;
    MI_SYS_BufConf_t stInjectBufConf;

    // user virAddr
    void *pOutVirAddr;
    void *pOutVirtY;
    void *pOutVirtUV;
    void *pInVirAddr;
    void *pInVirtY;
    void *pInVirtUV;

    MI_S32 s32Planar0Size = 0;
    MI_S32 s32Planar1Size = 0;

#ifdef ST_CEVA_VX_DUMP_FILE
    FILE *pGetBufFile = NULL;
    FILE *pPutBufFile = NULL;
    char szGetBufFilePath[50];
    char szPutBufFilePath[50];
#endif

#ifdef ST_CEVA_VX_STAT_TIME
    ST_CalcSpendTime_t stCalTmSpend;
    MI_S32 s32TestCnt = 0;
    MI_S32 s32FailCnt = 0;
    MI_U32 u32FirstTime = 0;

    ST_InitialTimeSpend(&stCalTmSpend);
#endif

    if (_gstImageAttr.eFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
        && _gstImageAttr.eFormat != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        CEVA_VX_LOG_ERR("Only support NV12 & NV16, format %d not support\n", _gstImageAttr.eFormat);
        return NULL;
    }

    _gstImageAttr.src[0] = NULL;
    _gstImageAttr.dst[0] = NULL;

    CEVA_VX_LOG_INFO("Sink port info: modId=%d devId=%d chnId=%d portId=%d\n", _gstSinkChnPort.eModId, _gstSinkChnPort.u32DevId
           , _gstSinkChnPort.u32ChnId, _gstSinkChnPort.u32PortId);
    CEVA_VX_LOG_INFO("Inject port info: modId=%d devId=%d chnId=%d portId=%d\n", _gstInjectChnPort.eModId, _gstInjectChnPort.u32DevId
           , _gstInjectChnPort.u32ChnId, _gstInjectChnPort.u32PortId);

    MI_SYS_SetChnOutputPortDepth(&_gstSinkChnPort, 2, 7);

    s32Ret = MI_SYS_GetFd(&_gstSinkChnPort, &s32Fd);
    if(MI_SUCCESS != s32Ret || s32Fd < 0)
    {
        CEVA_VX_LOG_ERR("MI_SYS_GetFd fail\n");
        return NULL;
    }

    aFd[0].fd = s32Fd;

    // init inject bufConf
    stInjectBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInjectBufConf.stFrameCfg.eFormat = _gstImageAttr.eFormat;
    stInjectBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInjectBufConf.stFrameCfg.u16Width = _gstImageAttr.width;
    stInjectBufConf.stFrameCfg.u16Height = _gstImageAttr.height;

    // ceva_vx init
    status = _ST_CEVA_VX_Init(&handle, _gstImageAttr.width, _gstImageAttr.height, _gstImageAttr.depth);
    if (status != VX_SUCCESS)
    {
        CEVA_VX_LOG_ERR("Can't init ceva sample, return %d\n", status);
        return NULL;
    }

    while(1)
    {
        if (_gbVxThreadExit)
            break;

#ifdef ST_CEVA_VX_STAT_TIME
        if (_gs32TestCnt > 0)
        {
            if (s32TestCnt >= _gs32TestCnt)
            {
                _gbVxThreadExit = TRUE;
                continue;
            }
        }

        s32TestCnt++;
//        CEVA_VX_LOG_INFO("current try: %d\n", s32TestCnt);
#endif

retry:
        // recv date
        s32Ret = poll(aFd, 1, s32TimeOutMs);
        if (s32Ret>0 && (aFd[0].revents & POLLIN))
        {
            //CEVA_VX_LOG_INFO("Thread: recv data !!!\n");
        }
        else
        {
            goto retry;
        }

        // process image
        // 1. get vpe outport Buf
#ifdef ST_CEVA_VX_GET_PA_BUF
        s32Ret = MI_SYS_ChnOutputPortGetPABuf(&_gstSinkChnPort, &stSinkBufInfo, &hSinkBufHandle);
#else
        s32Ret = MI_SYS_ChnOutputPortGetBuf(&_gstSinkChnPort, &stSinkBufInfo, &hSinkBufHandle);
#endif

        if (MI_SUCCESS == s32Ret)
        {
            // 2. get venc inputport buf
retry_get_inputport_buf:
#ifdef ST_CEVA_VX_GET_PA_BUF
            s32Ret = MI_SYS_ChnInputPortGetPABuf(&_gstInjectChnPort, &stInjectBufConf, &stInjectBufInfo, &hInjectBufHandle, s32TimeOutMs);
#else
            s32Ret = MI_SYS_ChnInputPortGetBuf(&_gstInjectChnPort, &stInjectBufConf, &stInjectBufInfo, &hInjectBufHandle, s32TimeOutMs);
#endif

            if (MI_SUCCESS == s32Ret)
            {
                // 3. memcpy uv data
                CEVA_VX_LOG_INFO("sink buf size: %d, inject buf size: %d\n", stSinkBufInfo.stFrameData.u32BufSize, stInjectBufInfo.stFrameData.u32BufSize);

                CEVA_VX_LOG_INFO("Sinkbuf: pa0=%llx pa1=%llx va0=%p va1=%p, stride0=%d, stride1=%d, w=%d, h=%d\n"
                    , stSinkBufInfo.stFrameData.phyAddr[0], stSinkBufInfo.stFrameData.phyAddr[1], stSinkBufInfo.stFrameData.pVirAddr[0]
                    , stSinkBufInfo.stFrameData.pVirAddr[1], stSinkBufInfo.stFrameData.u32Stride[0], stSinkBufInfo.stFrameData.u32Stride[1]
                    , stSinkBufInfo.stFrameData.u16Width, stSinkBufInfo.stFrameData.u16Height);

                if (_gstImageAttr.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                {
                    s32Planar0Size = stSinkBufInfo.stFrameData.u32Stride[0] * stSinkBufInfo.stFrameData.u16Height;
                    s32Planar1Size = stSinkBufInfo.stFrameData.u32Stride[1] * stSinkBufInfo.stFrameData.u16Height / 2;
                }
                else
                {
                    s32Planar0Size = stSinkBufInfo.stFrameData.u32Stride[0] * stSinkBufInfo.stFrameData.u16Height;
                    s32Planar1Size = stSinkBufInfo.stFrameData.u32Stride[1] * stSinkBufInfo.stFrameData.u16Height;
                }

#ifdef ST_CEVA_VX_GET_PA_BUF
                s32Ret = MI_SYS_Mmap(stSinkBufInfo.stFrameData.phyAddr[0], stSinkBufInfo.stFrameData.u32BufSize, &pOutVirAddr, FALSE);
                if (MI_SUCCESS != s32Ret)
                {
                    CEVA_VX_LOG_ERR("mmap sink port buf failed\n");
                    goto put_inputport_buf;
                }

                pOutVirtY = pOutVirAddr;
                if (_gstImageAttr.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                    pOutVirtUV = pOutVirAddr + stSinkBufInfo.stFrameData.u32BufSize * 2/3;
                else
                    pOutVirtUV = pOutVirAddr + stSinkBufInfo.stFrameData.u32BufSize / 2;

                s32Ret = MI_SYS_Mmap(stInjectBufInfo.stFrameData.phyAddr[0], stInjectBufInfo.stFrameData.u32BufSize, &pInVirAddr, FALSE);
                if (MI_SUCCESS != s32Ret)
                {
                    CEVA_VX_LOG_ERR("mmap inject port buf failed\n");
                    goto unmap_sink_buf;
                }

                pInVirtY = pInVirAddr;
                if (_gstImageAttr.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                    pInVirtUV = pInVirAddr + stInjectBufInfo.stFrameData.u32BufSize * 2/3;
                else
                    pInVirtUV = pInVirAddr + stInjectBufInfo.stFrameData.u32BufSize / 2;
#else
                pOutVirtY = stSinkBufInfo.stFrameData.pVirAddr[0];
                pOutVirtUV = stSinkBufInfo.stFrameData.pVirAddr[1];
                pOutVirAddr = pOutVirtY;

                pInVirtY = stInjectBufInfo.stFrameData.pVirAddr[0];
                pInVirtUV = stInjectBufInfo.stFrameData.pVirAddr[1];
                pInVirAddr = pInVirtY;
#endif

#ifndef PROC_BY_MEMCPY      // proc on ceva_vx
                memcpy(pInVirtUV, pOutVirtUV, s32Planar1Size);
                MI_SYS_FlushInvCache(pInVirtUV, s32Planar1Size);

#ifdef ST_CEVA_VX_DUMP_FILE
                // dump get buf to file
                memset(szGetBufFilePath, 0, sizeof(szGetBufFilePath));
                sprintf(szGetBufFilePath, "GetBuf_%d.yuv", s32TestCnt);
                pGetBufFile = fopen(szGetBufFilePath, "ab+");
                if (pGetBufFile)
                {
                    fwrite(pOutVirAddr, stSinkBufInfo.stFrameData.u32BufSize, 1, pGetBufFile);
                    fclose(pGetBufFile);
                    pGetBufFile = NULL;
                }
#endif

                // memcpy uv data
                _gstImageAttr.src[0] = stSinkBufInfo.stFrameData.phyAddr[0];
                _gstImageAttr.dst[0] = stInjectBufInfo.stFrameData.phyAddr[0];

#ifdef ST_CEVA_VX_STAT_TIME
                gettimeofday(&stCalTmSpend.stStartTmVal, NULL);
#endif

                // ceva_vx_process_image y data
                status = _ST_CEVA_VX_Process(&handle, _gstImageAttr.src, _gstImageAttr.dst);
                if (status != VX_SUCCESS)
                {
                    CEVA_VX_LOG_ERR("vx process failed : %d\n", status);
                    s32FailCnt++;
                }
                else
                {
#ifdef ST_CEVA_VX_STAT_TIME
                    ST_CalcTimeSpend(&stCalTmSpend);
                    CEVA_VX_LOG_INFO("%2d: %d us\n", s32TestCnt, stCalTmSpend.u32CurSpendTime);

                    if (s32TestCnt == 1)
                    {
                        u32FirstTime = stCalTmSpend.u32CurSpendTime;
                        ST_InitialTimeSpend(&stCalTmSpend);
                    }
#endif

#ifdef ST_CEVA_VX_DUMP_FILE
                    // dump put buf to file
                    memset(szPutBufFilePath, 0, sizeof(szPutBufFilePath));
                    sprintf(szPutBufFilePath, "PutBuf_%d.yuv", s32TestCnt);
                    pPutBufFile = fopen(szPutBufFilePath, "ab+");
                    if (pPutBufFile)
                    {
                        fwrite(pInVirAddr, stInjectBufInfo.stFrameData.u32BufSize, 1, pPutBufFile);
                        fclose(pPutBufFile);
                        pPutBufFile = NULL;
                    }
#endif
                }

#else           // proc by memcpy for test
                memcpy(pInVirtY, pOutVirtY, s32Planar0Size);
                memcpy(pInVirtUV, pOutVirtUV, s32Planar1Size);
                MI_SYS_FlushInvCache(pInVirtY, s32Planar0Size);
                MI_SYS_FlushInvCache(pInVirtUV, s32Planar1Size);
#endif

#ifdef ST_CEVA_VX_GET_PA_BUF
                MI_SYS_Munmap(pInVirAddr, stInjectBufInfo.stFrameData.u32BufSize);
unmap_sink_buf:
                MI_SYS_Munmap(pOutVirAddr, stSinkBufInfo.stFrameData.u32BufSize);
#endif

put_inputport_buf:
                // 7. put venc inputport buf
                MI_SYS_ChnInputPortPutBuf(hInjectBufHandle, &stInjectBufInfo, FALSE);
            }
            else
            {
                CEVA_VX_LOG_INFO("Get input buf failed\n");
                goto retry_get_inputport_buf;
            }

put_output_buf:
            // 8. put vpe outputport buf
            MI_SYS_ChnOutputPortPutBuf(hSinkBufHandle);
        }
        else
        {
            CEVA_VX_LOG_ERR("get outputport buf failed\n");
        }
    }

#ifdef ST_CEVA_VX_STAT_TIME
    if (s32TestCnt - s32FailCnt > 1)
        CEVA_VX_LOG_INFO("Test %dx%d: avgTime= %u us, maxTime= %u us, minTime= %u us, firstTime= %d us, testCnt= %d, failCnt= %d\n"
                         , _gstImageAttr.width, _gstImageAttr.height, stCalTmSpend.u32TotalTime/(s32TestCnt-s32FailCnt-1)
                         , stCalTmSpend.u32MaxSpendTime, stCalTmSpend.u32MinSpendTime, u32FirstTime, s32TestCnt, s32FailCnt);
#endif

#ifdef ST_CEVA_VX_DUMP_FILE
    if (pGetBufFile)
    {
        fclose(pGetBufFile);
        pGetBufFile = NULL;
    }

    if (pPutBufFile)
    {
        fclose(pPutBufFile);
        pPutBufFile = NULL;
    }
#endif

    // ceva_vx release
    _ST_CEVA_VX_Release(&handle);

close_fd:
    MI_SYS_CloseFd(s32Fd);

    return NULL;
}

void ST_CEVA_VX_SetImageAttr(ST_CEVA_VX_ImageAttr_t *pstImageAttr, MI_SYS_ChnPort_t stSinkPort, MI_SYS_ChnPort_t stInjectPort)
{
    memset(&_gstImageAttr, 0, sizeof(ST_CEVA_VX_ImageAttr_t));
    memcpy(&_gstImageAttr, pstImageAttr, sizeof(ST_CEVA_VX_ImageAttr_t));
    _gstSinkChnPort = stSinkPort;
    _gstInjectChnPort = stInjectPort;

    CEVA_VX_LOG_INFO("ST_CEVA_VX_SetImageAttr ok\n");
}

MI_S32 ST_CEVA_VX_Start(MI_BOOL bInjectToUsr)
{
    vx_status status;
    pthread_attr_t thread_attr;
    struct sched_param schedule_param;
    MI_S32 s32Ret = 0;

    _gbVxThreadExit = FALSE;

    CEVA_VX_LOG_INFO("create ceva vx process thread\n");
    pthread_attr_init(&thread_attr);
    schedule_param.sched_priority = 50;     //99;
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    pthread_attr_setschedparam(&thread_attr, &schedule_param);

    if (bInjectToUsr)
    {
        s32Ret = MI_SYS_MMA_Alloc(NULL, SHARE_MEM_SIZE, &_gstShareMem.phyAddr);
        if (s32Ret != MI_SUCCESS)
        {
            CEVA_VX_LOG_ERR("MI_SYS_MMA_Alloc failed\n");
            return -1;
        }

        CEVA_VX_LOG_INFO("Alloc shareMem phyAddr=0x%llx\n", _gstShareMem.phyAddr);

        s32Ret = MI_SYS_Mmap(_gstShareMem.phyAddr, SHARE_MEM_SIZE, &_gstShareMem.pVirAddr, FALSE);
        if (s32Ret != MI_SUCCESS)
        {
            CEVA_VX_LOG_ERR("MI_SYS_Mmap failed\n");
            return -1;
        }

        CEVA_VX_LOG_INFO("Map shareMem virAddr=%p\n", _gstShareMem.pVirAddr);

        _gstShareMem.u32Size = SHARE_MEM_SIZE;
        _gstShareMem.u32Length = 0;
        _gstShareMem.bFilled = FALSE;
        _gstImageAttr.dst[0] = _gstShareMem.phyAddr;
        pthread_mutex_init(&_gstShareMem.mtxShareMem, NULL);

        CEVA_VX_LOG_INFO("create thread _CEVA_VX_WorkThreadToUsr\n");
        pthread_create(&_gVxThread, &thread_attr, _CEVA_VX_WorkThreadToUsr, &_gstShareMem);
    }
    else
    {
        CEVA_VX_LOG_INFO("create thread _CEVA_VX_WorkThread\n");
        pthread_create(&_gVxThread, &thread_attr, _CEVA_VX_WorkThread, NULL);
    }

    if (!_gVxThread)
    {
        CEVA_VX_LOG_ERR("create ceva_vx thread failed\n");
        return -1;
    }

    CEVA_VX_LOG_INFO("ceva_vx init ok\n");
    return 0;
}

MI_S32 ST_CEVA_VX_Stop(MI_BOOL bInjectToUsr)
{
    _gbVxThreadExit = TRUE;

    if (bInjectToUsr)
    {
        if (_gVxThread)
        {
            pthread_join(_gVxThread, NULL);
            _gVxThread = NULL;
        }

        pthread_mutex_destroy(&_gstShareMem.mtxShareMem);
        MI_SYS_Munmap(_gstShareMem.pVirAddr, SHARE_MEM_SIZE);
        MI_SYS_MMA_Free(_gstShareMem.phyAddr);
        memset(&_gstShareMem, 0, sizeof(ST_CEVA_VX_ShareMem_t));
    }
    else
    {
        if (_gVxThread)
        {
            pthread_join(_gVxThread, NULL);
            _gVxThread = NULL;
        }
    }

    CEVA_VX_LOG_INFO("ceva vx process thread exit\n");

    return 0;
}

MI_U32 ST_CEVA_VX_GetOutputBuf(void *pUsrBuf)
{
    MI_U32 u32Length = 0;

#ifdef ST_CEVA_VX_DUMP_FILE
    FILE *pFile = NULL;
    char szFilePath[50];
    static MI_S32 s32FileIndex = 0;
#endif

    while (1)
    {
        pthread_mutex_lock(&_gstShareMem.mtxShareMem);
        if (_gstShareMem.bFilled && _gstShareMem.u32Length > 0)
        {
            memcpy(pUsrBuf, _gstShareMem.pVirAddr, _gstShareMem.u32Length);
            u32Length = _gstShareMem.u32Length;
            _gstShareMem.bFilled = FALSE;

#ifdef ST_CEVA_VX_DUMP_FILE
            // dump uvc buf
            if (u32Length > 0)
            {
                s32FileIndex++;
                memset(szFilePath, 0, sizeof(szFilePath));
                sprintf(szFilePath, "TestGetBuf_%d.yuv", s32FileIndex);
                pFile = fopen(szFilePath, "ab+");
                if (pFile)
                {
                    fwrite(pUsrBuf, u32Length, 1, pFile);
                    fclose(pFile);
                    pFile = NULL;
                }
            }
#endif
            pthread_mutex_unlock(&_gstShareMem.mtxShareMem);
            break;
        }
        pthread_mutex_unlock(&_gstShareMem.mtxShareMem);

        CEVA_VX_LOG_INFO("usr get buf failed, try again\n");
        usleep(5000);
    }


    return u32Length;
}

void ST_CEVA_VX_SwitchLog(void)
{
    _gbLogOn = !_gbLogOn;
}





