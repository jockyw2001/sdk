#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <assert.h>
#include "list.h"
#include "mi_print.h"
#include "mi_shadow.h"
#include "mi_md.h"
#include "mi_od.h"
#include "mi_vdf.h"
#include "mi_vdf_datatype.h"
#include "mi_sys_datatype.h"

/*
 * debug printf -->
 */
#define DBG_LEVEL_FATAL       (1)
#define DBG_LEVEL_ERROR       (2)
#define DBG_LEVEL_WARN        (3)
#define DBG_LEVEL_INFO        (4)
#define DBG_LEVEL_DEBUG       (5)
#define DBG_LEVEL_ENTRY       (6)

#define DBG_LEVEL DBG_LEVEL_WARN

#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define MI_VDF_FUNC_ENTRY()                 ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] In\n"COLOR_NONE,  __FUNCTION__,__LINE__);}}while(0);})
#define MI_VDF_FUNC_EXIT()                  ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] Exit\n"COLOR_NONE,  __FUNCTION__,__LINE__);}}while(0);})
#define MI_VDF_FUNC_ENTRY2(ViChn)           ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] In, ViChn=%d\n"COLOR_NONE,  __FUNCTION__,__LINE__, ViChn);}}while(0);})
#define MI_VDF_FUNC_EXIT2(ViChn)            ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] Exit, ViChn=%d\n"COLOR_NONE,  __FUNCTION__,__LINE__, ViChn);}}while(0);})
#define MI_VDF_FUNC_ENTRY3(ViChn, VdfChn)   ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] In, ViChn=%d, VdfChn=%d\n"COLOR_NONE,  __FUNCTION__,__LINE__, ViChn, VdfChn);}}while(0);})
#define MI_VDF_FUNC_EXIT3(ViChn, VdfChn)    ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"[VDF_ENTRY]:%s[%d] Exit ,ViChn=%d, VdfChn=%d\n"COLOR_NONE,  __FUNCTION__,__LINE__, ViChn, VdfChn);}}while(0);})

#define LOG_ENTRY(fmt, args...)             ({do{if(DBG_LEVEL>=DBG_LEVEL_ENTRY){printf(COLOR_CYAN"VDF_ENTRY]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})
#define LOG_DEBUG(fmt, args...)             ({do{if(DBG_LEVEL>=DBG_LEVEL_DEBUG){printf(COLOR_GREEN"VDF_INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})
#define LOG_INFO(fmt, args...)              ({do{if(DBG_LEVEL>=DBG_LEVEL_INFO) {printf(COLOR_WHITE"VDF_DBG]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})
#define LOG_WARN(fmt, args...)              ({do{if(DBG_LEVEL>=DBG_LEVEL_WARN) {printf(COLOR_YELLOW"VDF_WARN]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})
#define LOG_ERROR(fmt, args...)             ({do{if(DBG_LEVEL>=DBG_LEVEL_ERROR){printf(COLOR_RED"VDF_ERR]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})
#define LOG_FATAL(fmt, args...)             ({do{if(DBG_LEVEL>=DBG_LEVEL_FATAL){printf(COLOR_RED"VDF_FATAL]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(COLOR_NONE);}}while(0);})

#define ENABLE_DEBUG_LOCK   0

#if 0
#define pthread_mutex_lock(a)  \
{   \
    if(ENABLE_DEBUG_LOCK) \
    {\
        LOG_INFO("%s locked.~~ $$\n", #a);   \
    }\
    pthread_mutex_lock(a);  \
}

#define pthread_mutex_trylock(a)  \
{   \
    if(ENABLE_DEBUG_LOCK) \
    {\
        LOG_INFO("%s trylocked.~~ $$\n", #a);   \
    }\
    pthread_mutex_trylock(a);  \
}

#define pthread_mutex_unlock(a)  \
{   \
    if(ENABLE_DEBUG_LOCK) \
    {\
        LOG_INFO("%s unlocked.~~ $$\n", #a);   \
    }\
    pthread_mutex_unlock(a);  \
}
#endif

/*
 * function define -->
 */
#define MD_OD_SINGLE_PTHREAD    0
#define VDF_DBG_LOG_ENABLE      1
#define VDF_READ_RESULT_ENABLE  0

#define VDF_USLEEP_TIME_1MS     (1000 * 1)
#define VDF_USLEEP_TIME_10MS    (1000 * 10)
#define VDF_USLEEP_TIME_20MS    (1000 * 20)
#define VDF_USLEEP_DEFAULT_TIME VDF_USLEEP_TIME_1MS
#define ALIGN_UP(x, align)           (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, a)            (((x) / (a)) * (a))


/*
 * function global variable -->
 */
typedef enum MI_VDF_MDRSTBUF_MODE_E
{
    VDF_MDRSTBUF_MODE_READ     = 0x0,
    VDF_MDRSTBUF_MODE_WRITE    = 0x1,
    VDF_MDRSTBUF_MODE_MAX
} VDF_MDRSTBUF_MODE_e;

typedef struct MI_VDF_IMG_s
{
    void  *pu32PhyAddr;
    MI_U8 *pu8VirAddr;
} MI_VDF_IMG_t;

/*
   VDF_PTHREAD_MUTEX_S结构体用来描述和配置每一个通道属性，管理该通道内MD/OD的检测结果
   */
typedef struct _VDF_PTHREAD_MUTEX_S
{
    MI_U8   u8Enable;                       //创建该通道线程，用来控制该通道内的MD&OD功能
    MI_U16  u16IdxI;                        //用于for循环自增变量
    MI_U16  u16IdxJ;                        //用于for循环自增变量
    MI_U32  u32YImageSize;                  //输入该通道Image的Y数据大小
    MI_U64  u64Pts;                         //输入该通道Image的时间戳，每一帧数据都会更新
    struct timeval tv;

    void *           pstMdRstListTmp;
    MDOBJ_DATA_t*    pstMDObjData;
    MDSAD_DATA_t*    pstMDSadData;
    MI_MD_Result_t*  pstMdRstList;
    MI_OD_Result_t   stOdDetectRst;
    MI_OD_Result_t*  pstOdRstList;
    MI_VDF_MdRstHdlList_t* pstMdRstHdlList;
    MI_VDF_OdRstHdlList_t* pstOdRstHdlList;
    MI_VDF_NodeList_t*     pstVDFNodeTmp;
    MI_VDF_IMG_t       image;
    MI_SYS_BUF_HANDLE hInputBufHandle;
    pthread_mutex_t mutexMDRW;
    pthread_mutex_t mutexODRW;
    pthread_mutex_t mutexRUN;
    pthread_cond_t  condVideoSrcChnY;
    pthread_mutex_t mutexVideoSrcChnY;
} VDF_PTHREAD_MUTEX_S;


typedef struct VDF_ChnAttr_Object_s {
    struct list_head list;
    MI_VDF_CHANNEL vdfChn;
    MI_VDF_ChnAttr_t vdfChn_Attr;
} VDF_ChnAttr_Object_t;

typedef struct MI_SYS_ChnPortList_s
{
    struct list_head list;
    MI_SYS_ChnPort_t srcChnPort;
    MI_SYS_ChnPort_t dtsChnPort;
} MI_SYS_ChnPortList_t;

typedef struct VDF_VirSrcIdexMap_s {
    struct list_head chnPortNode;
    MI_S32 mapNum;
} VDF_VirSrcIdexMap_t;

MI_U8 *RAW_PTR = NULL;
static MI_VDF_MdResult_t MI_VDF_MD_RST_LIST[MI_VDF_SOURCE_NUM_MAX];
static MI_VDF_OdResult_t MI_VDF_OD_RST_LIST[MI_VDF_SOURCE_NUM_MAX];
static MI_VDF_EntryNode_t MI_VDF_ENTRY_MODE_LIST[MI_VDF_SOURCE_NUM_MAX];

static MI_S32 g_width  = MI_VDF_MIN_WIDTH;
static MI_S32 g_height = MI_VDF_MIN_WIDTH;
static MI_U8  g_MDEnable = 0;
static MI_U8  g_ODEnable = 0;
static MI_BOOL g_VdfYuvTaskExit = FALSE;
static MI_BOOL g_VdfAPPTaskExit[MI_VDF_SOURCE_NUM_MAX];
static MI_VDF_NodeList_t* g_pstVDFNode[MI_VDF_CHANNEL_MAX] = { NULL };

MI_SHADOW_RegisterDevParams_t _stVDFRegDevInfo;
MI_SHADOW_HANDLE _hVDFDev = 0;


struct list_head vdfChn_list;
VDF_VirSrcIdexMap_t MAPSourceChn[MI_VDF_SOURCE_NUM_MAX] = {0};

static pthread_t pthreadVDFYUV;
static pthread_t pthreadVDFAPP[MI_VDF_SOURCE_NUM_MAX];
VDF_PTHREAD_MUTEX_S VDF_PTHREAD_MUTEX[MI_VDF_SOURCE_NUM_MAX];


static pthread_mutex_t mutexVDF = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  condVDF  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex_VDF_cfg = PTHREAD_MUTEX_INITIALIZER;

/*
 * function -->
 */
unsigned int _MI_VDF_GetTimeInMs(void)
{
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned int T = (1000000 * (t1.tv_sec) + (t1.tv_nsec) / 1000) / 1000 ;
    return T;
}

void _MI_VDF_Dump_Map_VdfChn(void)
{
    MI_U8 index;
    struct list_head *pos = NULL, *n = NULL;
    MI_SYS_ChnPortList_t *pstPorList = NULL;

    for (index = 0; index < MI_VDF_SOURCE_NUM_MAX; index++)
    {
        LOG_DEBUG("=====MAPSourceChn[%d].mapNum=%d =====\n", index, MAPSourceChn[index].mapNum);
        if (MAPSourceChn[index].mapNum)
        {
            if (!list_empty(&MAPSourceChn[index].chnPortNode)) {
                list_for_each_safe(pos, n, &MAPSourceChn[index].chnPortNode)
                {
                    pstPorList = list_entry(pos, MI_SYS_ChnPortList_t, list);
                    LOG_DEBUG("srcChnPort eModId=%d u32DevId=%d u32ChnId=%d u32PortId=%d, "
                            "dtsChnPort eModId=%d u32DevId=%d u32ChnId=%d u32PortId=%d\n",
                            pstPorList->srcChnPort.eModId, pstPorList->srcChnPort.u32DevId,
                            pstPorList->srcChnPort.u32ChnId, pstPorList->srcChnPort.u32PortId,
                            pstPorList->dtsChnPort.eModId, pstPorList->dtsChnPort.u32DevId,
                            pstPorList->dtsChnPort.u32ChnId, pstPorList->dtsChnPort.u32PortId);
                }
            }
        }
    }
}

MI_S32 _MI_VDF_Is_Map_VdfChn(MI_U8 *idx, MI_VDF_CHANNEL VdfChn, MI_VDF_PORT VdfPort)
{
    MI_S32 ret = -1;
    MI_U8 index;
    struct list_head *pos = NULL, *n = NULL;
    MI_SYS_ChnPortList_t *pstPorList = NULL;

    pthread_mutex_lock(&mutexVDF);
    for (index = 0; index < MI_VDF_SOURCE_NUM_MAX; index++)
    {
        if (MAPSourceChn[index].mapNum)
        {
            // LOG_DEBUG("MAPSourceChn[%d].mapNum:%d\n", index, MAPSourceChn[index].mapNum);
            if (!list_empty(&MAPSourceChn[index].chnPortNode)) {
                list_for_each_safe(pos, n, &MAPSourceChn[index].chnPortNode)
                {
                    pstPorList = list_entry(pos, MI_SYS_ChnPortList_t, list);
                    if (pstPorList->dtsChnPort.u32ChnId == VdfChn &&
                            pstPorList->dtsChnPort.u32PortId == VdfPort)
                    {
                        LOG_DEBUG("find the VdfChn Map!\n");
                        LOG_DEBUG("MAPSourceChn[%d].mapNum:%d\n",index, MAPSourceChn[index].mapNum);
                        LOG_DEBUG("pstPorList->srcChnPort eModId=%d u32DevId=%d u32ChnId=%d u32PortId=%d\n",
                                pstPorList->srcChnPort.eModId, pstPorList->srcChnPort.u32DevId,
                                pstPorList->srcChnPort.u32ChnId, pstPorList->srcChnPort.u32PortId);

                        LOG_DEBUG("pstPorList->dtsChnPort eModId=%d u32DevId=%d u32ChnId=%d u32PortId=%d\n",
                                pstPorList->dtsChnPort.eModId, pstPorList->dtsChnPort.u32DevId,
                                pstPorList->dtsChnPort.u32ChnId, pstPorList->dtsChnPort.u32PortId);

                        *idx = index;
                        pthread_mutex_unlock(&mutexVDF);
                        return TRUE;
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    // LOG_WARN("can not find the VdfChn Map!!!\n");
    pthread_mutex_unlock(&mutexVDF);
    return FALSE;
}

MI_S32 _MI_VDF_Map_VdfChn_To_ViSrcIndex(MI_U8 *idx, MI_SYS_ChnPort_t *dstChnPort, MI_SYS_ChnPort_t *srcChnPort)
{
    MI_S32 ret = -1;
    MI_U8 index;
    struct list_head *pos = NULL, *n = NULL;
    MI_SYS_ChnPortList_t *pstPorList = NULL;
    MI_SYS_ChnPortList_t *pstBindList = NULL;

    // 1. 查找VdfChn Map是否存在,有则直接返回
    if(idx == NULL || dstChnPort == NULL || srcChnPort == NULL)
    {
        return ret;
    }

    if (_MI_VDF_Is_Map_VdfChn(idx, dstChnPort->u32ChnId, dstChnPort->u32PortId))
    {
        return MI_SUCCESS;
    }

    pthread_mutex_lock(&mutexVDF);
    // 2. 是否有同源数据(srcChnPort 相同)，有则追加到该ViSrcIndex列表
    for (index = 0; index < MI_VDF_SOURCE_NUM_MAX; index++)
    {
        if (MAPSourceChn[index].mapNum &&
                !list_empty(&MAPSourceChn[index].chnPortNode))
        {

            list_for_each_safe(pos, n, &MAPSourceChn[index].chnPortNode)
            {
                pstPorList = list_entry(pos, MI_SYS_ChnPortList_t, list);
                if (!memcmp(&pstPorList->srcChnPort, srcChnPort, sizeof(MI_SYS_ChnPort_t)))
                {
                    if (memcmp(&pstPorList->dtsChnPort, dstChnPort, sizeof(MI_SYS_ChnPort_t)) &&
                            pstPorList->list.next == &MAPSourceChn[index].chnPortNode)
                    {
                        pstBindList = (MI_SYS_ChnPortList_t *) malloc(sizeof(MI_SYS_ChnPortList_t));
                        if (!pstBindList)
                        {
                            pthread_mutex_unlock(&mutexVDF);
                            return ret;
                        }
                        memcpy(&pstBindList->srcChnPort, srcChnPort, sizeof(MI_SYS_ChnPort_t));
                        memcpy(&pstBindList->dtsChnPort, dstChnPort, sizeof(MI_SYS_ChnPort_t));
                        list_add_tail(&pstBindList->list, &MAPSourceChn[index].chnPortNode);
                        MAPSourceChn[index].mapNum++;
                        *idx = index;
                        LOG_DEBUG("add to a exist index list, index=%d\n", index);
                        pthread_mutex_unlock(&mutexVDF);
                        return MI_SUCCESS;
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }

    // 3. 没有同源数据，新建一个index
    for (index = 0; index < MI_VDF_SOURCE_NUM_MAX; index++)
    {
        if (MAPSourceChn[index].mapNum)
        {
            continue;
        }
        else
        {
            INIT_LIST_HEAD(&MAPSourceChn[index].chnPortNode);
            pstBindList = (MI_SYS_ChnPortList_t *) malloc(sizeof(MI_SYS_ChnPortList_t));
            if (!pstBindList)
            {
                pthread_mutex_unlock(&mutexVDF);
                return ret;
            }
            memcpy(&pstBindList->srcChnPort, srcChnPort, sizeof(MI_SYS_ChnPort_t));
            memcpy(&pstBindList->dtsChnPort, dstChnPort, sizeof(MI_SYS_ChnPort_t));

            list_add_tail(&pstBindList->list, &MAPSourceChn[index].chnPortNode);
            MAPSourceChn[index].mapNum++;
            *idx = index;
            LOG_DEBUG("add to a new index list, index=%d\n", index);
            pthread_mutex_unlock(&mutexVDF);
            return MI_SUCCESS;
        }
    }
    pthread_mutex_unlock(&mutexVDF);
    return ret;
}

MI_S32 _MI_VDF_GetInputPortBuf_ByViSrcIndex(
        MI_SHADOW_HANDLE hShadow,
        MI_U8 u8ViSrcChn,
        MI_SYS_BufInfo_t *pstBufInfo,
        MI_SYS_BUF_HANDLE *phBufHandle)
{
    MI_S32 ret = -1;
    MI_S32 cnt = 0;
    struct list_head *pos = NULL, *n = NULL;
    MI_SYS_ChnPortList_t *pstPorList = NULL;

    MI_VDF_FUNC_ENTRY2(u8ViSrcChn);

    if(list_empty(&MAPSourceChn[u8ViSrcChn].chnPortNode))
    {
        // LOG_ERROR("The input MAPSourceChn[u8ViSrcChn].chnPortNode is NULL %d\n", u8ViSrcChn);
        return ret;
    }

    // LOG_DEBUG("MAPSourceChn[u8ViSrcChn].chnPortNode %p %p", MAPSourceChn[u8ViSrcChn].chnPortNode.prev, MAPSourceChn[u8ViSrcChn].chnPortNode.next);

    if (u8ViSrcChn >= MI_VDF_SOURCE_NUM_MAX)
    {
        LOG_ERROR("index >= MI_VDF_SOURCE_NUM_MAX\n");
        return ret;
    }

    // 查找Map到当前ViSrcIndex的chn和port,并获取buf
    cnt = MAPSourceChn[u8ViSrcChn].mapNum;

    list_for_each_safe(pos, n, &MAPSourceChn[u8ViSrcChn].chnPortNode)
    {
        pstPorList = list_entry(pos, MI_SYS_ChnPortList_t, list);
        // LOG_DEBUG("%d, %d\n", pstPorList->dtsChnPort.u32ChnId, pstPorList->dtsChnPort.u32PortId);
        // only kepp one buf ref
        ret = MI_SHADOW_GetInputPortBuf(hShadow, pstPorList->dtsChnPort.u32ChnId,
                pstPorList->dtsChnPort.u32PortId, pstBufInfo, phBufHandle);
        if (MI_SUCCESS == ret && cnt > 1)
        {
            MI_SHADOW_FinishBuf(_hVDFDev, *phBufHandle);
            cnt--;
        }
        else
        {
            if (MI_SUCCESS != ret)
                LOG_DEBUG("MI_SHADOW_FinishBuf ret fial:%d cnt:%d\n", ret, cnt);
            else
                LOG_DEBUG("GetInputPortBuf ok, %d, %d\n", pstPorList->dtsChnPort.u32ChnId, pstPorList->dtsChnPort.u32PortId);
        }
    }
    MI_VDF_FUNC_EXIT();
    return ret;
}

MI_S32 _MI_VDF_FinishBuf(MI_SHADOW_HANDLE hShadow, MI_SYS_BUF_HANDLE hBufHandle)
{
    return MI_SHADOW_FinishBuf(hShadow, hBufHandle);  //discard Output port data
}

inline MI_S32 _MI_VDF_RunAndSetRst_MD(MI_U8 u8ViSrcChn, MI_VDF_NodeList_t* pstVDFNode)
{
    MI_VDF_FUNC_ENTRY2(u8ViSrcChn);

    pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));

    //TODO: check if Width * Height * 1.5 = videoFrame.vFrame.bufLen
    pstVDFNode->u32New_t = _MI_VDF_GetTimeInMs(); // get system time in ms

    MI_MD_SetTime(pstVDFNode->phandle,
            ((pstVDFNode->u32New_t > pstVDFNode->u32Old_t) ?
             (pstVDFNode->u32New_t - pstVDFNode->u32Old_t) :
             (0xFFFFFFFF - pstVDFNode->u32Old_t + pstVDFNode->u32New_t)));

    pstVDFNode->u32Old_t = pstVDFNode->u32New_t;

	LOG_DEBUG("[u8ViSrcChn=%d] pstVDFNode->phandle=%p image.pu8VirAddr=%p image.pu32PhyAddr=%p\n",\
            u8ViSrcChn, \
            pstVDFNode->phandle, \
            VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr, \
            VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu32PhyAddr \
            );

    if(-1 == (pstVDFNode->s32Ret = MI_MD_Run(pstVDFNode->phandle, (MI_MD_IMG_t*)&VDF_PTHREAD_MUTEX[u8ViSrcChn].image)))
    {
        //case1: MD 运行错误
        LOG_ERROR("ViSrc=%u VdfChn=0x%x Run MD error\n", u8ViSrcChn, pstVDFNode->VdfChn);
        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return pstVDFNode->s32Ret;
    }
    else if(0 == pstVDFNode->s32Ret)
    {
        //case2: MD 运行正确，没有检测到画面运动
        LOG_INFO("ViSrc=%u VdfChn=0x%x No MD detected!\n", u8ViSrcChn, pstVDFNode->VdfChn);
        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return pstVDFNode->s32Ret;
    }

    pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
    //case3: MD 运行正确，有检测到画面运动
    //Get volume from the MdRstHdlList to backup the MD detect result
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList = MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList;

    //find the MD handle from the Handle list and write the MD result
    // TODO:移到md od里面
    while(NULL != VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList)
    {
        if((pstVDFNode->VdfChn  == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->VdfChn) && \
                (pstVDFNode->phandle == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->pMdHandle))
        {
            break;
        }

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList = (MI_VDF_MdRstHdlList_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->next;
    }

    if(NULL == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList)
    {
        LOG_ERROR("ViSrc=%u Get Result handle list error\n", u8ViSrcChn);
        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return (MI_S32)(-1);
    }

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->pstMdRstList;

    //如果写指针大于buf长度：1)打印警告信息; 2)重新对写指针赋值
    if(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst > pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt)
    {
        LOG_WARN("ViSrc=%d, VdfChn=0x%x write overflow, D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                u8ViSrcChn, pstVDFNode->VdfChn,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt);

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst %= pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt;
    }

    //根据写指针，设定保存MD结果的buf的地址
    LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstMdRstListTmp=%p\n", u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp);
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst * \
                                                     VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u16MdRstBufSize;

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;
    LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstMdRstList=%p(MdRstBufSize=0x%x)\n", u8ViSrcChn,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u16MdRstBufSize);

    //如果要写的地址正在被应用层读取，写指针和读指针重合
    //并且DltData为buff总数时等待应用层读取完数据(设置10ms超时)
    if((1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Enable) && (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Reading) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdRstBufCnt))
    {
        MI_U32 u32PtsStop = 0;
        MI_U32 u32PtsStart = (MI_U32)_MI_VDF_GetTimeInMs();

        do{
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
            usleep(VDF_USLEEP_DEFAULT_TIME);

            if(0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Reading)
            {
                break;
            }

            u32PtsStop = (MI_U32)_MI_VDF_GetTimeInMs();
        }while((u32PtsStop>u32PtsStart) ? ((u32PtsStop-u32PtsStart-10)<0) : ((0xFFFFFFFF-u32PtsStart+u32PtsStop-10)<0));

        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
    }


    //如果写指针对应buf不为空，打印警告信息并覆盖该数据
    //如果写指针和读指针重合，并且DltData为buff总数时自动覆盖最开始写入数据(读指针加一，同时DltData数据减一)
    if((1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Enable) && (0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Reading) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdRstBufCnt))
    {
        LOG_WARN("ViSrc=%d, VdfChn=0x%x the MD detect data(pos=%u) will be covered!\n",
                u8ViSrcChn, pstVDFNode->VdfChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst);
        LOG_DEBUG("VdfChn=0x%x D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                pstVDFNode->VdfChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt);

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst++;
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst %= VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdRstBufCnt;
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData--;
    }

    memset(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList, 0x00, sizeof(MI_MD_Result_t));

    switch(pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
    {
        case MDALG_MODE_FG:
            {
                //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstMdRstList=%p\n", u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp = (MI_U8*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData = (MDOBJ_DATA_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MDOBJ_DATA_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion = (MDOBJ_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                //VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->pu8MdResult = (MI_U8*)pu8MdRstData;
                LOG_DEBUG("pstMdResult=%p ccobj.astRegion=%p MdSadBufferSize=0x%x\n", VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList,
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion, pstVDFNode->stAttr.unAttr.stMdAttr.u16MdSadBufferSize);

                if(pstVDFNode->s32Ret = MI_MD_CCL(pstVDFNode->phandle, &pstVDFNode->stAttr.unAttr.stMdAttr.ccl_ctrl, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData))
                {
                    LOG_ERROR("call MI_MD_CCL() error(0x%x)!\n", pstVDFNode->s32Ret);
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
                    return pstVDFNode->s32Ret;
                }

                if(VDF_DBG_LOG_ENABLE)
                {
                    MI_U16 u16Cnt = 0;
                    MDOBJ_t*pu8MdRstDataTmp = NULL;

                    LOG_INFO("[VDF-MD][VdfChn=0x%x ViChn=%u] pts=0x%llx D_Delt=%d, W_Ptr=%u, R_Ptr=%u, MDObjDataAddr=%p\n", pstVDFNode->VdfChn, u8ViSrcChn,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion);
                    LOG_INFO("RegionNum=0x%x indexofmaxobj=0x%x areaofmaxobj=0x%x areaoftotalobj=0x%x astRegion=%p\n",
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->u8RegionNum, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->indexofmaxobj,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->areaofmaxobj, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->areaoftotalobj);
#if VDF_READ_RESULT_ENABLE
                    pu8MdRstDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion;
                    for(u16Cnt = 0; u16Cnt < VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->u8RegionNum; u16Cnt++)
                    {
                        pu8MdRstDataTmp += u16Cnt;
                        LOG_INFO("u8RegionNum=%d u32Area=0x%02x u16Left=0x%02x u16Righ=0x%02x u16Top=0x%02x u16Bottom=0x%02x\n",
                                pu8MdRstDataTmp->u32Area, pu8MdRstDataTmp->u16Left, pu8MdRstDataTmp->u16Right,
                                pu8MdRstDataTmp->u16Top,  pu8MdRstDataTmp->u16Bottom);
                    }
#endif
                }

                break;
            }

        case MDALG_MODE_SAD:
            {
                //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size) * ResultBufferCount

                LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstMdRstList=%p\n", u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData = (MDSAD_DATA_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MDSAD_DATA_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;
                LOG_DEBUG("[u8ViSrcChn=%d]pstMdResult=%p pstMdSadData=%p pstMdSadData->paddr=%p "
                        "pstMdSadData->stride=%d pstMdSadData->enOutCtrl=%d MdSadBufferSize=0x%x\n", \
                        u8ViSrcChn, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->stride, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->enOutCtrl, \
                        pstVDFNode->stAttr.unAttr.stMdAttr.u16MdSadBufferSize \
                        );
                LOG_DEBUG("[u8ViSrcChn=%d] image.pu8VirAddr=%p image.pu32PhyAddr=%p\n",\
                        u8ViSrcChn, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu32PhyAddr \
                        );

                /* Get MD SAD */
                pstVDFNode->s32Ret = (MI_S32)MI_MD_ComputeImageSAD( \
                        pstVDFNode->phandle, \
                        (const MI_MD_IMG_t*)&VDF_PTHREAD_MUTEX[u8ViSrcChn].image, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData \
                        );

                LOG_DEBUG("[u8ViSrcChn:%d] Get MD SAD (0x%x, %p %p %p)!\n", \
                        u8ViSrcChn, \
                        pstVDFNode->s32Ret, \
                        pstVDFNode->phandle, \
                        &VDF_PTHREAD_MUTEX[u8ViSrcChn].image, \
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData \
                        );

                if(pstVDFNode->s32Ret)
                {
                    LOG_ERROR("Get MD SAD error(0x%x)!\n", pstVDFNode->s32Ret);
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
                    return pstVDFNode->s32Ret;
                }

                if(VDF_DBG_LOG_ENABLE)
                {
                    LOG_INFO("[VDF-MD][VdfChn=0x%x ViChn=%u] pts=0x%llx D_Delt=%d, W_Ptr=%u, R_Ptr=%u, MDSadDataAddr=%p\n", pstVDFNode->VdfChn, u8ViSrcChn,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr);
                    LOG_INFO("[VDF-MD][VdfChn=0x%x ViChn=%u] paddr=%p stride=%u enOutCtrlD_Delt=%d\n", pstVDFNode->VdfChn, u8ViSrcChn,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->stride,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->enOutCtrl);

#if VDF_READ_RESULT_ENABLE
                    MI_U16 u16Cnt = 0;
                    MI_U32 buffer_size, col, row;
                    MI_U8* pstMdSadDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr;

                    pstMdSadDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr;

                    col = (g_width >> 1) >> (pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
                    row = g_height >> (pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
                    buffer_size = col * row * (2 - pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl);

                    for(u16Cnt = 1; u16Cnt < buffer_size; u16Cnt++)
                    {
                        if(0 == u16Cnt%16) printf("\n");
                        else if(0 == u16Cnt%8) printf(" ");
                        else printf("0x%02x ",*pstMdSadDataTmp++);
                    }
                    printf("\n");
#endif
                }
                break;
            }

        case MDALG_MODE_SAD_FG:
            {
                //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount

                LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstMdRstList=%p\n", u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData = (MDSAD_DATA_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MDSAD_DATA_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += sizeof(MDOBJ_DATA_t);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;

                LOG_DEBUG("pstMdResult=%p pu8MdRstData=%p pstMdSadData->paddr=%p MdSadBufferSize=0x%x\n",
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData,
                        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr, pstVDFNode->stAttr.unAttr.stMdAttr.u16MdSadBufferSize);
                if(pstVDFNode->s32Ret = MI_MD_ComputeImageSAD(pstVDFNode->phandle, (const MI_MD_IMG_t*)&VDF_PTHREAD_MUTEX[u8ViSrcChn].image, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData))
                {
                    LOG_ERROR("Get MD SAD error(0x%x)!\n", pstVDFNode->s32Ret);
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
                    return pstVDFNode->s32Ret;
                }

                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp += pstVDFNode->stAttr.unAttr.stMdAttr.u16MdSadBufferSize;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp -= sizeof(MDOBJ_t) * 255;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstListTmp;
                LOG_DEBUG("ccobj.astRegion=%p\n", VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion);

                if(pstVDFNode->s32Ret = MI_MD_CCL(pstVDFNode->phandle,
                            &pstVDFNode->stAttr.unAttr.stMdAttr.ccl_ctrl,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData))
                {
                    LOG_ERROR("call MI_MD_CCL() error(0x%x)!\n", pstVDFNode->s32Ret);
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
                    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
                    return pstVDFNode->s32Ret;
                }

                if(VDF_DBG_LOG_ENABLE)
                {
                    MI_U16 u16Cnt = 0;
                    MI_U8* pstMdSadDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr;
                    LOG_INFO("[VDF-MD][VdfChn=0x%x ViChn=%u] pts=0x%llx D_Delt=%d, W_Ptr=%u, R_Ptr=%u, MDSadDataAddr=%p\n", pstVDFNode->VdfChn, u8ViSrcChn,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr);
#if VDF_READ_RESULT_ENABLE
                    MI_U32 buffer_size, col, row;

                    LOG_INFO("[VDF-MD][VdfChn=0x%x ViChn=%u] paddr=%p stride=%u enOutCtrlD_Delt=%d\n", pstVDFNode->VdfChn, u8ViSrcChn,
                            pstMdSadDataTmp, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->stride, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->enOutCtrl);

                    pstMdSadDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDSadData->paddr;

                    col = (g_width >> 1) >> (pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
                    row = g_height >> (pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
                    buffer_size = col * row * (2 - pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl);

                    for(u16Cnt = 1; u16Cnt < buffer_size; u16Cnt++)
                    {
                        if(0 == u16Cnt%16) printf("\n");
                        else if(0 == u16Cnt%8) printf(" ");
                        else printf("0x%02x ",*pstMdSadDataTmp++);
                    }
#endif
                    LOG_INFO("RegionNum=0x%x indexofmaxobj=0x%x areaofmaxobj=0x%x areaoftotalobj=0x%x MDObjDataAddr=%p\n",
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->u8RegionNum, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->indexofmaxobj,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->areaofmaxobj, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->areaoftotalobj,
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion);
#if VDF_READ_RESULT_ENABLE
                    MDOBJ_t* pu8MdObjDataTmp = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->astRegion;
                    for(u16Cnt = 0; u16Cnt < VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMDObjData->u8RegionNum; u16Cnt++)
                    {
                        pu8MdObjDataTmp += u16Cnt;
                        LOG_INFO("u32Area=0x%x u16Left=0x%02x u16Righ=0x%02x u16Top=0x%02x u16Bottom=0x%02x\n",
                                pu8MdObjDataTmp->u32Area, pu8MdObjDataTmp->u16Left, pu8MdObjDataTmp->u16Right,
                                pu8MdObjDataTmp->u16Top,  pu8MdObjDataTmp->u16Bottom);
                    }
#endif
                }

                break;
            }

        default:
            LOG_WARN("Input the wrong md_alg_mode(%d)\n",
                    pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
            return (MI_S32)(-1);
    }

    //take care of this control bit, if not set APP will not get the result
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u8Enable = 1;
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstList->u64Pts = VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts;

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst++;
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst %= pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt;

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData++;
    if(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData > pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt)
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData = pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt;

    if(VDF_DBG_LOG_ENABLE)
    {
        LOG_INFO("VdfChn=0x%x D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                pstVDFNode->VdfChn,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->s8MdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstMdRstHdlList->u8MdReadPst,
                pstVDFNode->stAttr.unAttr.stMdAttr.u8MdBufCnt);
    }

    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));

    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));

    MI_VDF_FUNC_EXIT();

    return (MI_S32)0;
}


inline MI_S32 _MI_VDF_RunAndSetRst_OD(MI_U8 u8ViSrcChn, MI_VDF_NodeList_t* pstVDFNode)
{
    MI_VDF_FUNC_ENTRY();

    //TODO: check if Width * Height * 1.5 = videoFrame.vFrame.bufLen
    pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));

    if(-1 == (pstVDFNode->s32Ret = MI_OD_Run(pstVDFNode->phandle, (MI_OD_IMG_t*)&VDF_PTHREAD_MUTEX[u8ViSrcChn].image)))
    {
        //case1: OD 运行错误
        LOG_ERROR("ViSrc=%d run OD error\n", u8ViSrcChn);
        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return pstVDFNode->s32Ret;
    }
    else if(0 == pstVDFNode->s32Ret)
    {
        //case2: OD 运行正确，没有检测到画面运动
        if(VDF_DBG_LOG_ENABLE)
            LOG_DEBUG("ViSrc=%u No OD detected!\n", __func__, __LINE__, u8ViSrcChn);

        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return pstVDFNode->s32Ret;
    }

    //case3: OD 运行正确，有检测到画面运动
    //deal with the OD detect result
    memset(&VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst, 0x00, sizeof(MI_OD_Result_t));
    switch(pstVDFNode->stAttr.unAttr.stOdAttr.stOdStaticParamsIn.div)
    {
        case OD_WINDOW_1X1:
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8WideDiv  = 1;
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8HightDiv = 1;
            break;

        case OD_WINDOW_2X2:
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8WideDiv  = 2;
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8HightDiv = 2;
            break;

        case OD_WINDOW_3X3:
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8WideDiv  = 3;
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8HightDiv = 3;
            break;

        default:
            LOG_ERROR("ViSrc=%u VdfChn=0x%x Get the wrong window Divs(%d)\n", u8ViSrcChn,
                    pstVDFNode->VdfChn, pstVDFNode->stAttr.unAttr.stOdAttr.stOdStaticParamsIn.div);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
            return -1;
    }

    //take care:
    //  1, The number of divisions of window in horizontal direction must smaller than 3(u16W_div<=3)
    //  2, The number of divisions of window in vertical direction must smaller than 3(u16H_div<=3)
    for(VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxJ = 0;
            VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxJ < VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8HightDiv;
            VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxJ++)
    {
        for(VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxI = 0;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxI < VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8WideDiv;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxI++)
        {
            VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst.u8RgnAlarm[VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxJ][VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxI] =
                MI_OD_GetWindowResult(pstVDFNode->phandle, VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxI, VDF_PTHREAD_MUTEX[u8ViSrcChn].u16IdxJ);
        }
    }


    //add the OD detect result to the list
    pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW));

    //Get volume from the OdRstHdlList to backup the OD detect result
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList = MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList;

    //find the OD handle from the Handle list and write the OD result
    while(NULL != VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList)
    {
        if((pstVDFNode->VdfChn  == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->VdfChn) && \
                (pstVDFNode->phandle == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->pOdHandle))
        {
            break;
        }

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList = (MI_VDF_OdRstHdlList_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->next;
    }

    if(NULL == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList)
    {
        LOG_ERROR("ViSrc=%u Get Result handle list error\n", u8ViSrcChn);
        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        return (MI_S32)(-1);
    }

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->pstOdResultList;

    //如果写指针大于buf长度：1)打印警告信息; 2)重新对写指针赋值
    if(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst > pstVDFNode->stAttr.unAttr.stOdAttr.u8OdBufCnt)
    {
        LOG_WARN("ViSrc=%d, VdfChn=0x%x write overflow, D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                u8ViSrcChn, pstVDFNode->VdfChn,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst,
                pstVDFNode->stAttr.unAttr.stOdAttr.u8OdBufCnt);

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst %= pstVDFNode->stAttr.unAttr.stOdAttr.u8OdBufCnt;
    }

    //根据写指针，设定保存OD结果的buf的地址
    LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstOdRstList=%p\n", u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList);
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList += VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst;
    LOG_DEBUG("VDF_PTHREAD_MUTEX[%d].pstOdRstList=%p(sizeof(MI_OD_Result_t)=0x%x)\n", u8ViSrcChn,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList, sizeof(MI_OD_Result_t));

    //如果要写的地址正在被应用层读取，写指针和读指针重合
    //并且DltData为buff总数时等待应用层读取完数据(设置10ms超时)
    if((1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Enable) && (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Reading) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt))
    {
        MI_U32 u32PtsStop = 0;
        MI_U32 u32PtsStart = (MI_U32)_MI_VDF_GetTimeInMs();

        do{
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW));
            usleep(VDF_USLEEP_DEFAULT_TIME);

            if(0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Reading)
            {
                break;
            }

            u32PtsStop = (MI_U32)_MI_VDF_GetTimeInMs();
        }while((u32PtsStop>u32PtsStart) ? ((u32PtsStop-u32PtsStart-10)<0) : ((0xFFFFFFFF-u32PtsStart+u32PtsStop-10)<0));

        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW));
    }


    //如果写指针对应buf不为空，打印警告信息并覆盖该数据
    //如果写指针和读指针重合，并且DltData为buff总数时自动覆盖最开始写入数据(读指针加一，同时DltData数据减一)
    if((1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Enable) && (0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Reading) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst) &&
            (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt))
    {
        LOG_WARN("ViSrc=%d, VdfChn=0x%x the OD detect data(pos=%u) will be covered!\n",
                u8ViSrcChn, pstVDFNode->VdfChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst);
        LOG_DEBUG("VdfChn=0x%x D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                pstVDFNode->VdfChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst,
                pstVDFNode->stAttr.unAttr.stOdAttr.u8OdBufCnt);

        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst++;
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst %= VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt;
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData--;
    }

    memset(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList, 0x00, sizeof(MI_OD_Result_t));
    memcpy(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList, &VDF_PTHREAD_MUTEX[u8ViSrcChn].stOdDetectRst, sizeof(MI_OD_Result_t));

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u64Pts = VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts;

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst++;
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst %= VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt;

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData++;
    if(VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData > VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt)
    {
        VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt;
    }

    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8DataLen  = VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData;

    //take care of this control bit, if not set APP will not get the result
    VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8Enable = 1;
    LOG_INFO("VDF_PTHREAD_MUTEX[%d].pstOdRstList=%p, u64Pts=0x%llx\n", u8ViSrcChn,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList,VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u64Pts);

#if (VDF_DBG_LOG_ENABLE)
    LOG_DEBUG("[VdfChn=0x%x, hdl=%p ViSrc=%d] pts=0x%llx DataLen=%d (%d, %d)] Get OD-Rst data:\n",
            pstVDFNode->VdfChn, pstVDFNode->phandle, u8ViSrcChn,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u64Pts,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8DataLen,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8WideDiv,
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8HightDiv);
    LOG_DEBUG("{(%d %d %d) (%d %d %d) (%d %d %d)}\n",
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[0][0], VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[0][1],
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[0][2], VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[1][0],
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[1][1], VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[1][2],
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[2][0], VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[2][1],
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstList->u8RgnAlarm[2][2]);
#endif

    if(VDF_DBG_LOG_ENABLE)
    {
        LOG_DEBUG("VdfChn=0x%x D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                pstVDFNode->VdfChn,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->s8OdDeltData,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdWritePst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdReadPst,
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList->u8OdRstBufCnt);
    }


    if(NULL == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstOdRstHdlList)
    {
        LOG_ERROR("VdfChn=0x%x Cannot find the OD Result handle list\n", pstVDFNode->VdfChn);
    }


    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW));
    pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));

    MI_VDF_FUNC_EXIT();
    return 0;
}


void* vdf_YUV_task(void *argu)
{
    MI_S32 ret = 0;
    MI_U8  u8ViSrcChn = 0;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHandle;


    MI_VDF_FUNC_ENTRY();

    LOG_INFO("VDF_YUV_TASK+\n");


    while(FALSE == g_VdfYuvTaskExit)
    {
        //TODO:fixme
#if 0
        ret = pthread_mutex_trylock(&mutexVDF);

        if(ret == EBUSY)
        {
            usleep(VDF_USLEEP_DEFAULT_TIME);
            pthread_mutex_unlock(&mutexVDF);
            continue;
        }
        else if(ret != 0)
        {
            LOG_DEBUG("mutexVDF ret == %d\n", ret);
            usleep(VDF_USLEEP_DEFAULT_TIME);
            pthread_mutex_unlock(&mutexVDF);
            continue;
        }
#endif
        for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
        {
            MI_U16 u32PortId = 0;
            static MI_U16 time_dis = 0;
            static MI_U16 times = 0;

            // 1st, get the video src channel image
            if(MI_SUCCESS != MI_SHADOW_WaitOnInputTaskAvailable(_hVDFDev, 20))
            {
                /*
                   time_dis++;
                   if(0 == (time_dis%100))
                   {
                   LOG_DEBUG("SHADOW wait buffer time out(%d)\n",time_dis);
                   usleep(VDF_USLEEP_TIME_10MS);
                   }
                   */
                continue;
            }

            memset(&stInputBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

            if(MI_SUCCESS != _MI_VDF_GetInputPortBuf_ByViSrcIndex(_hVDFDev, u8ViSrcChn, &stInputBufInfo, &hInputBufHandle))
            {
                // LOG_DEBUG("get shadow buffer failed\n");
                continue;
            }
            if(0 == (times%100))
                LOG_DEBUG("MI_SHADOW_GetInputPortBuf() times = %d\n", times++);

            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
            //todo fixme
#if 0
            ret = pthread_mutex_trylock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));  // TODO:fixme
            if(EBUSY == ret)
            {
                LOG_WARN("[VDF_YUV_TASK] ViSrc(%u) busy\n", u8ViSrcChn);
                usleep(VDF_USLEEP_DEFAULT_TIME);
                continue;
            }
            else if(0 != ret)
            {
                LOG_WARN("[VDF_YUV_TASK] ViSrc(%u) ret=%d\n", ret);
            }
            else
#endif
            {
                VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr = (MI_U8 *)stInputBufInfo.stFrameData.pVirAddr[0];
                VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu32PhyAddr = (void  *)((MI_U32)stInputBufInfo.stFrameData.phyAddr[0]);
                VDF_PTHREAD_MUTEX[u8ViSrcChn].u64Pts = stInputBufInfo.u64Pts;

                VDF_PTHREAD_MUTEX[u8ViSrcChn].u32YImageSize = stInputBufInfo.stFrameData.u16Width * stInputBufInfo.stFrameData.u16Height;
                VDF_PTHREAD_MUTEX[u8ViSrcChn].hInputBufHandle = hInputBufHandle;

                pthread_cond_signal(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].condVideoSrcChnY));
                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
            }

            //在vdf_APP_task()里面调用MI_SHADOW_FinishBuf()释放buffer
            //MI_SHADOW_FinishBuf(_hVDFDev, hInputBufHandle);  //discard Output port data
            // usleep(VDF_USLEEP_TIME_10MS);
        }
        // todo fixme
#if 0
        pthread_mutex_unlock(&mutexVDF);
#endif
        usleep(VDF_USLEEP_DEFAULT_TIME);
    }

    LOG_INFO("VDF_YUV_TASK-\n");

    MI_VDF_FUNC_EXIT();
    return NULL;
}


void* vdf_APP_task(void *argu)
{
    MI_U8 u8ViSrcChn = 0;

    MI_VDF_FUNC_ENTRY2((MI_U8)((MI_U32)argu & 0xFF));

    u8ViSrcChn = (MI_U8)((MI_U32)argu & 0xFF);

    if((u8ViSrcChn < 0) && (u8ViSrcChn > MI_VDF_SOURCE_NUM_MAX))
    {
        LOG_ERROR("Get the ViSrc(%d) is out of range\n", u8ViSrcChn);
        return NULL;
    }

    // TODO:fixme
    if(NULL == (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList))
    {
        LOG_ERROR("ViSrc=%u Get the VDF Node list is NULL\n", u8ViSrcChn);
        return NULL;
    }

    LOG_INFO("VDF_APP_TASK(%u)+\n", u8ViSrcChn);

    while(FALSE == g_VdfAPPTaskExit[u8ViSrcChn])
    {
        if(NULL == (VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList))
        {
            LOG_WARN("Get the VDF Node list(%d) is NULL\n", u8ViSrcChn);
            usleep(VDF_USLEEP_DEFAULT_TIME);
        }

        // TODO:get buffer lock
        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));

        while(NULL == VDF_PTHREAD_MUTEX[u8ViSrcChn].hInputBufHandle) {
            pthread_cond_wait(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].condVideoSrcChnY), \
                    &(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
        }

        if(TRUE == g_VdfAPPTaskExit[u8ViSrcChn])
        {
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
            break; //break the 1st while loop
        }


        while(NULL != VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp)
        {
            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameCnt++;
            LOG_DEBUG("pstVDFNode[%d]->u8FrameInterval=%d, pstVDFNode[%d]->u8FrameCnt=%d\n",
                    u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameInterval,
                    u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameCnt);

            if((0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameInterval) || \
                    (0 == ((VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameCnt % \
                            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameInterval))))
            {
                VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->u8FrameCnt = 0;

                if((1 == g_MDEnable) && (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].u8Enable)                        &&   \
                        (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.unAttr.stMdAttr.u8Enable)       &&   \
                        (E_MI_VDF_WORK_MODE_MD == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.enWorkMode) &&   \
                        (u8ViSrcChn == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.unAttr.stMdAttr.u8SrcChnNum))
                {
                    _MI_VDF_RunAndSetRst_MD(u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp);
                }

                if((1 == g_ODEnable) && (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].u8Enable)                        &&   \
                        (1 == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.unAttr.stOdAttr.u8Enable)       &&   \
                        (E_MI_VDF_WORK_MODE_OD == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.enWorkMode) &&   \
                        (u8ViSrcChn == VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->stAttr.unAttr.stOdAttr.u8SrcChnNum))
                {
                    _MI_VDF_RunAndSetRst_OD(u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp);
                }
            }

            VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp = (MI_VDF_NodeList_t*)VDF_PTHREAD_MUTEX[u8ViSrcChn].pstVDFNodeTmp->next;
            usleep(VDF_USLEEP_DEFAULT_TIME);
        }

        _MI_VDF_FinishBuf(_hVDFDev, VDF_PTHREAD_MUTEX[u8ViSrcChn].hInputBufHandle);  //discard Output port data

        static MI_U32 times = 0;
        LOG_DEBUG("call MI_SHADOW_FinishBuf() ok, VDF_PTHREAD_MUTEX[%d].hInputBufHandle=%p, times=%d\n\n",
                u8ViSrcChn, VDF_PTHREAD_MUTEX[u8ViSrcChn].hInputBufHandle, times++);
        VDF_PTHREAD_MUTEX[u8ViSrcChn].hInputBufHandle = NULL;

        pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
    }

    LOG_INFO("VDF_APP_TASK(%u)-\n", u8ViSrcChn);

    MI_VDF_FUNC_EXIT2((MI_U8)((MI_U32)argu & 0xFF));
    return NULL;
}


MI_S32 _MI_VDF_CheckChnValid(MI_VDF_CHANNEL VdfChn)
{
    MI_U8 u8ViSrcChn = 0;
    MI_VDF_NodeList_t* pstVDFNode_tmp = NULL;

    MI_VDF_FUNC_ENTRY();

    if((VdfChn < 0) || (VdfChn >= MI_VDF_CHANNEL_MAX))
    {
        LOG_ERROR("The input VdfChn is Out of Range\n");
        return -1;
    }

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        if(NULL == (pstVDFNode_tmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList))
        {
            continue;
        }

        do
        {
            if(VdfChn == pstVDFNode_tmp->VdfChn)
            {
                MI_VDF_FUNC_EXIT();
                return 0;
            }

            pstVDFNode_tmp = (MI_VDF_NodeList_t*)pstVDFNode_tmp->next;

        }
        while(NULL != pstVDFNode_tmp);
    }

    MI_VDF_FUNC_EXIT();
    return -1;
}


MI_S32 _MI_VDF_AddVDFNode( void* phandle, MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr)
{
    MI_VDF_NodeList_t* pstVDFNode_tmp = NULL;


    MI_VDF_FUNC_ENTRY2(pstAttr->unAttr.stMdAttr.u8SrcChnNum);

    pthread_mutex_lock(&mutex_VDF_cfg);     //TODO:fixme

    if((NULL == pstAttr) || (NULL == phandle))
    {
        LOG_ERROR("The input Pointer is NULL\n");
        pthread_mutex_unlock(&mutex_VDF_cfg);
        return -1;
    }

    if((E_MI_VDF_WORK_MODE_MD != pstAttr->enWorkMode) && (E_MI_VDF_WORK_MODE_OD != pstAttr->enWorkMode))
    {
        LOG_ERROR("Get the wrong Work Mode(%d)\n", pstAttr->enWorkMode);
        pthread_mutex_unlock(&mutex_VDF_cfg);
        return -1;
    }

    if((NULL == g_pstVDFNode[VdfChn]) && (-1 == _MI_VDF_CheckChnValid(VdfChn)))
    {
        if(NULL == (g_pstVDFNode[VdfChn] = (MI_VDF_NodeList_t*)malloc(sizeof(MI_VDF_NodeList_t))))
        {
            LOG_ERROR("malloc buffer for New VDF NODE error\n");
            pthread_mutex_unlock(&mutex_VDF_cfg);
            return -1;
        }
    }

    memset(g_pstVDFNode[VdfChn], 0x00, sizeof(MI_VDF_NodeList_t));
    g_pstVDFNode[VdfChn]->phandle  = phandle;
    g_pstVDFNode[VdfChn]->VdfChn   = VdfChn;
    g_pstVDFNode[VdfChn]->u32Old_t = _MI_VDF_GetTimeInMs();
    g_pstVDFNode[VdfChn]->u32New_t = g_pstVDFNode[VdfChn]->u32Old_t;
    memcpy(&g_pstVDFNode[VdfChn]->stAttr, pstAttr, sizeof(MI_VDF_ChnAttr_t));
    g_pstVDFNode[VdfChn]->next = NULL;

    if(E_MI_VDF_WORK_MODE_MD == pstAttr->enWorkMode)
    {
        MI_U32 u32DataTmp = 0;
        MI_S32 buffer_size, col, row;

        col = g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.width  >> \
              (g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
        row = g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.height >> \
              (g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
        buffer_size = col * row * (2 - g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl);

        //Set MD Result buf size
        switch(g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
        {
            case MDALG_MODE_FG:
                {
                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize = sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255);
                    u32DataTmp = g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize % sizeof(MI_U32);
                    if(u32DataTmp)
                    {
                        g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize += (sizeof(MI_U32) - u32DataTmp);
                    }

                    break;
                }

            case MDALG_MODE_SAD:
                {
                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size) * ResultBufferCount
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize = sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size;
                    u32DataTmp = g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize % sizeof(MI_U32);
                    if(u32DataTmp)
                    {
                        g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize += (sizeof(MI_U32) - u32DataTmp);
                    }
                    break;
                }

            case MDALG_MODE_SAD_FG:
                {
                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize = sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + \
                                                                                      sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255);
                    u32DataTmp = g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize % sizeof(MI_U32);
                    if(u32DataTmp)
                    {
                        g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u16MdSadBufferSize += (sizeof(MI_U32) - u32DataTmp);
                    }
                    break;
                }

            default:
                LOG_WARN("Input the wrong md_alg_mode(%d)\n", \
                        g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
                break;
        }

        g_pstVDFNode[VdfChn]->u8SrcChnNum     = pstAttr->unAttr.stMdAttr.u8SrcChnNum;
        g_pstVDFNode[VdfChn]->u8FrameInterval = pstAttr->unAttr.stMdAttr.u8VDFIntvl;
    }
    else if(E_MI_VDF_WORK_MODE_OD == pstAttr->enWorkMode)
    {
        g_pstVDFNode[VdfChn]->u8SrcChnNum     = pstAttr->unAttr.stOdAttr.u8SrcChnNum;
        g_pstVDFNode[VdfChn]->u8FrameInterval = pstAttr->unAttr.stOdAttr.u8VDFIntvl;
    }

    // add the new node to the tail of VDF Node list
    if(E_MI_VDF_WORK_MODE_MD == pstAttr->enWorkMode)
    {
        if(NULL == (pstVDFNode_tmp = MI_VDF_ENTRY_MODE_LIST[pstAttr->unAttr.stMdAttr.u8SrcChnNum].pstVdfNodeList))
        {
            MI_VDF_ENTRY_MODE_LIST[pstAttr->unAttr.stMdAttr.u8SrcChnNum].pstVdfNodeList = g_pstVDFNode[VdfChn];
            LOG_DEBUG("VdfChn=0x%x, phandle=0x%x pstNewVDFNode=%p\n", VdfChn, (MI_U32*)phandle, g_pstVDFNode[VdfChn]);
            pthread_mutex_unlock(&mutex_VDF_cfg);
            MI_VDF_FUNC_EXIT2(pstAttr->unAttr.stMdAttr.u8SrcChnNum);
            return 0;
        }
    }
    else if(E_MI_VDF_WORK_MODE_OD == pstAttr->enWorkMode)
    {
        if(NULL == (pstVDFNode_tmp = MI_VDF_ENTRY_MODE_LIST[pstAttr->unAttr.stOdAttr.u8SrcChnNum].pstVdfNodeList))
        {
            MI_VDF_ENTRY_MODE_LIST[pstAttr->unAttr.stOdAttr.u8SrcChnNum].pstVdfNodeList = g_pstVDFNode[VdfChn];
            LOG_DEBUG("VdfChn=0x%x, phandle=0x%x pstNewVDFNode=%p\n", VdfChn, (MI_U32*)phandle, g_pstVDFNode[VdfChn]);
            pthread_mutex_unlock(&mutex_VDF_cfg);
            MI_VDF_FUNC_EXIT2(pstAttr->unAttr.stMdAttr.u8SrcChnNum);
            return 0;
        }
    }

    while(NULL != pstVDFNode_tmp)
    {
        LOG_DEBUG("VdfChn=0x%x, phandle=0x%x pstVDFNode_tmp=%p\n", pstVDFNode_tmp->VdfChn, pstVDFNode_tmp->phandle, pstVDFNode_tmp);
        if(NULL == pstVDFNode_tmp->next)
        {
            pstVDFNode_tmp->next = g_pstVDFNode[VdfChn];
            break;
        }

        pstVDFNode_tmp = (MI_VDF_NodeList_t*)pstVDFNode_tmp->next;
    }

    pthread_mutex_unlock(&mutex_VDF_cfg);

    MI_VDF_FUNC_EXIT2(pstAttr->unAttr.stMdAttr.u8SrcChnNum);
    return 0;
}


MI_S32 _MI_VDF_GetVDFNode(MI_VDF_CHANNEL VdfChn, MI_VDF_NodeList_t** pstVDFNode)
{
    MI_S32 ret = -1;
    MI_U8 u8ViSrcChn = 0;
    MI_VDF_NodeList_t* pstVDFNode_tmp = NULL;

    MI_VDF_FUNC_ENTRY();


    if(NULL == pstVDFNode)
    {
        LOG_ERROR("The input Pointer is NULL\n");
        return ret;
    }

    if(0 != _MI_VDF_CheckChnValid(VdfChn))
    {
        LOG_ERROR("check VdfChn(0x%x) error\n", VdfChn);
        return ret;
    }

    pthread_mutex_lock(&mutex_VDF_cfg);

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        if(NULL == (pstVDFNode_tmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList))
        {
            continue;
        }

        do{
            if(VdfChn == pstVDFNode_tmp->VdfChn)
            {
                *pstVDFNode = pstVDFNode_tmp;
                LOG_DEBUG("VdfChn=0x%x, phandle=0x%x pstVDFNode_tmp=%p\n", pstVDFNode_tmp->VdfChn, pstVDFNode_tmp->phandle, pstVDFNode_tmp);
                pthread_mutex_unlock(&mutex_VDF_cfg);
                MI_VDF_FUNC_EXIT();
                ret = 0;
                return ret;
            }

            LOG_DEBUG("VdfChn=0x%x, phandle=0x%x pstVDFNode_tmp=%p\n", pstVDFNode_tmp->VdfChn, pstVDFNode_tmp->phandle, pstVDFNode_tmp);

            pstVDFNode_tmp = pstVDFNode_tmp->next;

        }while(NULL != pstVDFNode_tmp);
    }

    *pstVDFNode = NULL;
    pthread_mutex_unlock(&mutex_VDF_cfg);

    return ret;
}


MI_S32 _MI_VDF_DelVDFNode(MI_VDF_CHANNEL VdfChn)
{
    MI_S32 ret = -1;
    MI_U8 u8ViSrcChn = 0;
    MI_VDF_NodeList_t* pVdfNode_tmp = NULL;
    MI_VDF_NodeList_t* pVdfNode_pro = NULL;

    MI_VDF_FUNC_ENTRY();

    if(0 != _MI_VDF_CheckChnValid(VdfChn))
    {
        LOG_ERROR("The input VdfChn(0x%x) is invalid\n", VdfChn);
        return ret;
    }

    pthread_mutex_lock(&mutex_VDF_cfg);

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        if(NULL == (pVdfNode_tmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList))
        {
            continue;
        }

        // check the 1st node
        if(VdfChn == MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList->VdfChn)
        {
            MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList = pVdfNode_tmp->next;
            free(pVdfNode_tmp);
            pthread_mutex_unlock(&mutex_VDF_cfg);
            MI_VDF_FUNC_EXIT();
            ret = 0;
            return ret;
        }

        pVdfNode_pro = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList;
        pVdfNode_tmp = pVdfNode_pro->next;

        while(NULL != pVdfNode_tmp)
        {
            if(VdfChn == pVdfNode_tmp->VdfChn)
            {
                pVdfNode_pro = pVdfNode_tmp->next;
                free(pVdfNode_tmp);
                pthread_mutex_unlock(&mutex_VDF_cfg);
                MI_VDF_FUNC_EXIT();
                ret = 0;
                return ret;
            }

            pVdfNode_pro = pVdfNode_tmp;
            pVdfNode_tmp = pVdfNode_tmp->next;
        }
    }

    pthread_mutex_unlock(&mutex_VDF_cfg);

    return ret;
}

MI_S32 _MI_VDF_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    MI_S32 ret = -1;
    MI_U8  u8ViSrcChn = 0;
    struct list_head *pos = NULL, *n = NULL;
    VDF_ChnAttr_Object_t *pstVdfChnAttrObj= NULL;

    MI_VDF_FUNC_ENTRY();
    LOG_DEBUG("VDF Get On Input Event, cur(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) \
            peer(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) pUsrData:%p\n",
            pstChnCurryPort->eModId,
            pstChnCurryPort->u32DevId,
            pstChnCurryPort->u32ChnId,
            pstChnCurryPort->u32PortId,
            pstChnPeerPort->eModId,
            pstChnPeerPort->u32DevId,
            pstChnPeerPort->u32ChnId,
            pstChnPeerPort->u32PortId,
            pUsrData);

    if(!pstChnCurryPort && !pstChnPeerPort)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return ret;
    }

    // 1. 查找当前chn对应的vdfChn_list成员并建立映射
    // 2. 根据绑定关系修改参数u8SrcChnNum
    if(list_empty(&vdfChn_list))
    {
        LOG_ERROR("The input vdfChn_list is NULL\n");
        return ret;
    }

    list_for_each_safe(pos, n, &vdfChn_list)
    {
        pstVdfChnAttrObj = list_entry(pos, VDF_ChnAttr_Object_t, list);

        LOG_DEBUG("pstAttrObj->vdfChn:%d\n", pstVdfChnAttrObj->vdfChn);
        LOG_DEBUG("pstAttrObj->vdfChn_Attr.enWorkMode:%d\n", pstVdfChnAttrObj->vdfChn_Attr.enWorkMode);

        if (pstVdfChnAttrObj->vdfChn == pstChnCurryPort->u32ChnId)
        {

            if (MI_SUCCESS != _MI_VDF_Map_VdfChn_To_ViSrcIndex(&u8ViSrcChn, pstChnCurryPort, pstChnPeerPort))
                return ret;

            switch(pstVdfChnAttrObj->vdfChn_Attr.enWorkMode)
            {
                case E_MI_VDF_WORK_MODE_MD:
                    pstVdfChnAttrObj->vdfChn_Attr.unAttr.stMdAttr.u8SrcChnNum = u8ViSrcChn;
                    break;

                case E_MI_VDF_WORK_MODE_OD:
                    pstVdfChnAttrObj->vdfChn_Attr.unAttr.stOdAttr.u8SrcChnNum = u8ViSrcChn;
                    break;

                default:
                    LOG_WARN("Input the wrong Work Mode(%d)\n", pstVdfChnAttrObj->vdfChn_Attr.enWorkMode);
                    break;
            }
#if (DBG_LEVEL >= DBG_LEVEL_DEBUG)
            _MI_VDF_Dump_Map_VdfChn();
#endif
            break;
        }
    }

    return ret;
}

MI_S32 _MI_VDF_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    LOG_DEBUG("(%d)VDF Get On output Event, cur(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) \
            peer(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) pUsrData:%p\n",
            pstChnCurryPort->eModId,
            pstChnCurryPort->u32DevId,
            pstChnCurryPort->u32ChnId,
            pstChnCurryPort->u32PortId,
            pstChnPeerPort->eModId,
            pstChnPeerPort->u32DevId,
            pstChnPeerPort->u32ChnId,
            pstChnPeerPort->u32PortId,
            pUsrData);
    return E_MI_ERR_FAILED;
}

MI_S32 _MI_VDF_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    LOG_DEBUG("VDF Get On un input Event, cur(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) \
            peer(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) pUsrData:%p\n",
            pstChnCurryPort->eModId,
            pstChnCurryPort->u32DevId,
            pstChnCurryPort->u32ChnId,
            pstChnCurryPort->u32PortId,
            pstChnPeerPort->eModId,
            pstChnPeerPort->u32DevId,
            pstChnPeerPort->u32ChnId,
            pstChnPeerPort->u32PortId,
            pUsrData);
    return MI_SUCCESS;
}

MI_S32 _MI_VDF_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    LOG_DEBUG("VDF Get On Un Output Event, cur(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) \
            peer(eModId:%d, u32DevId:%d, u32ChnId:%d, u32PortId:%d) pUsrData:%p\n",
            pstChnCurryPort->eModId,
            pstChnCurryPort->u32DevId,
            pstChnCurryPort->u32ChnId,
            pstChnCurryPort->u32PortId,
            pstChnPeerPort->eModId,
            pstChnPeerPort->u32DevId,
            pstChnPeerPort->u32ChnId,
            pstChnPeerPort->u32PortId,
            pUsrData);
    return MI_SUCCESS;
}

MI_S32 MI_VDF_Init(void)
{
    MI_S32 ret = -1;
    MI_S32 index = 0;
    MI_U8 u8ViSrcChn = 0;

    MI_VDF_FUNC_ENTRY();


    g_MDEnable = 0;
    g_ODEnable = 0;
    g_VdfYuvTaskExit = FALSE;

    memset(MI_VDF_MD_RST_LIST, 0x00, sizeof(MI_VDF_MD_RST_LIST));
    memset(MI_VDF_OD_RST_LIST, 0x00, sizeof(MI_VDF_OD_RST_LIST));
    memset(MI_VDF_ENTRY_MODE_LIST, 0x00, sizeof(MI_VDF_ENTRY_MODE_LIST));

    // init attr list
    INIT_LIST_HEAD(&vdfChn_list);

    // init MAPSourceChn list
    for (index = 0; index < MI_VDF_SOURCE_NUM_MAX; index++)
    {
        INIT_LIST_HEAD(&MAPSourceChn[index].chnPortNode);
        g_pstVDFNode[index] = NULL;
    }

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        g_VdfAPPTaskExit[u8ViSrcChn] = FALSE;
        pthread_mutex_init(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW), NULL);
        pthread_mutex_init(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW), NULL);
        pthread_mutex_init(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN), NULL);
        pthread_mutex_init(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY), NULL);
        pthread_cond_init (&(VDF_PTHREAD_MUTEX[u8ViSrcChn].condVideoSrcChnY), NULL);
    }

#if (VDF_DBG_LOG_ENABLE)
    printf("===== This libmi_vdf.so is only run on MSR650 platform [");
    printf(__DATE__);
    printf("  ");
    printf(__TIME__);
    printf("] =====\n");
#endif //#if (VDF_DBG_LOG_ENABLE)

    //用户调用下面接口函数设置shadow buff缓存帧大小
    //MI_SYS_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth)
    memset(&_stVDFRegDevInfo, 0x00, sizeof(MI_SHADOW_RegisterDevParams_t));
    _stVDFRegDevInfo.stModDevInfo.eModuleId = E_MI_MODULE_ID_VDF;
    _stVDFRegDevInfo.stModDevInfo.u32DevId = 0;
    _stVDFRegDevInfo.stModDevInfo.u32DevChnNum = MI_VDF_CHANNEL_MAX;
    _stVDFRegDevInfo.stModDevInfo.u32InputPortNum = 1;
    _stVDFRegDevInfo.stModDevInfo.u32OutputPortNum = 0;
    _stVDFRegDevInfo.OnBindInputPort = _MI_VDF_OnBindInputPort;
    _stVDFRegDevInfo.OnBindOutputPort =_MI_VDF_OnBindOutputPort;
    _stVDFRegDevInfo.OnUnBindInputPort = _MI_VDF_OnUnBindInputPort;
    _stVDFRegDevInfo.OnUnBindOutputPort = _MI_VDF_OnUnBindOutputPort;

    if(MI_SUCCESS != (ret = MI_SHADOW_RegisterDev(&_stVDFRegDevInfo, &_hVDFDev)))
    {
        LOG_ERROR("call MI_SHADOW_RegisterDev() fail!\n", __func__, __LINE__);
        return ret;
    }

    pthread_create(&pthreadVDFYUV, NULL, vdf_YUV_task, NULL);
    pthread_setname_np(pthreadVDFYUV, "VDF_YUV_Task");

    MI_VDF_FUNC_EXIT();
    ret = 0;
    return ret;
}


MI_S32 MI_VDF_Uninit(void)
{
    MI_S32 ret = -1;
    MI_U8  u8ViSrcChn = 0;
    MI_VDF_NodeList_t* pstVDFNode_tmp = NULL;
    MI_VDF_NodeList_t* pstVDFNode_tmp_next = NULL;

    MI_VDF_FUNC_ENTRY();


    //1st: stop vdf_YUV_task & vdf_APP_task
    while(FALSE == g_VdfYuvTaskExit)
    {
        pthread_mutex_lock(&mutexVDF);
        // todo fix me
#if 0
        pthread_cond_signal(&condVDF);
#endif
        pthread_mutex_unlock(&mutexVDF);

        LOG_DEBUG("wait for VDF TASK exit\n");

        usleep(VDF_USLEEP_TIME_10MS);
    }

    pthread_join(pthreadVDFYUV, NULL);

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        while(FALSE == g_VdfAPPTaskExit[u8ViSrcChn])
        {
            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
            pthread_cond_signal(&VDF_PTHREAD_MUTEX[u8ViSrcChn].condVideoSrcChnY);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));

            g_VdfAPPTaskExit[u8ViSrcChn] = TRUE;
            usleep(VDF_USLEEP_TIME_10MS);
        }

        pthread_join(pthreadVDFAPP[u8ViSrcChn], NULL);

        if(VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr)
        {
            free(VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr);
            VDF_PTHREAD_MUTEX[u8ViSrcChn].image.pu8VirAddr = NULL;
        }
    }

    //2nd: de-init and free the MD/OD node
    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        pstVDFNode_tmp = MI_VDF_ENTRY_MODE_LIST[u8ViSrcChn].pstVdfNodeList;

        while(NULL != pstVDFNode_tmp)
        {
            if((NULL != (pstVDFNode_tmp->phandle)) && \
                    (E_MI_VDF_WORK_MODE_MD == pstVDFNode_tmp->stAttr.enWorkMode))
            {
                MI_MD_Uninit(pstVDFNode_tmp->phandle);
            }

            if((NULL != (pstVDFNode_tmp->phandle)) && \
                    (E_MI_VDF_WORK_MODE_OD == pstVDFNode_tmp->stAttr.enWorkMode))
            {
                MI_OD_Uninit(pstVDFNode_tmp->phandle);
            }

            pstVDFNode_tmp_next = pstVDFNode_tmp->next;
            free(pstVDFNode_tmp);
            pstVDFNode_tmp = pstVDFNode_tmp_next;
        }
    }

    //3st: free the MD/OD resault buffer
    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        if(NULL != MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList)
        {
            MI_VDF_MdRstHdlList_t* pstMdRstHandleList = NULL;
            MI_VDF_MdRstHdlList_t* pstMdRstHandleList_next = NULL;

            pstMdRstHandleList = MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList;

            while(NULL != pstMdRstHandleList)
            {
                pstMdRstHandleList_next = pstMdRstHandleList->next;
                free(pstMdRstHandleList->pstMdRstList);
                free(pstMdRstHandleList);
                pstMdRstHandleList = pstMdRstHandleList_next;
            }

            MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList = NULL;
        }

        if(NULL != MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList)
        {
            MI_VDF_OdRstHdlList_t* pstOdRstHandleList = NULL;
            MI_VDF_OdRstHdlList_t* pstOdRstHandleList_next = NULL;

            pstOdRstHandleList = MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList;

            while(NULL != pstOdRstHandleList)
            {
                pstOdRstHandleList_next = pstOdRstHandleList->next;
                free(pstOdRstHandleList->pstOdResultList);
                free(pstOdRstHandleList);
                pstOdRstHandleList = pstOdRstHandleList_next;
            }

            MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList = NULL;
        }
    }

    memset(MI_VDF_MD_RST_LIST, 0x00, sizeof(MI_VDF_MD_RST_LIST));
    memset(MI_VDF_MD_RST_LIST, 0x00, sizeof(MI_VDF_OD_RST_LIST));

    for(u8ViSrcChn = 0; u8ViSrcChn < MI_VDF_SOURCE_NUM_MAX; u8ViSrcChn++)
    {
        g_VdfAPPTaskExit[u8ViSrcChn] = TRUE;
        pthread_mutex_destroy(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexMDRW));
        pthread_mutex_destroy(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexODRW));
        pthread_mutex_destroy(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexRUN));
        pthread_mutex_destroy(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].mutexVideoSrcChnY));
        pthread_cond_destroy(&(VDF_PTHREAD_MUTEX[u8ViSrcChn].condVideoSrcChnY));
    }

    MI_VDF_FUNC_EXIT();
    ret = 0;
    return ret;
}


MI_S32 _MI_VDF_CreateChn_MD(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr)
{
    MI_S32 ret = -1;
    MI_U8  u8ViSrcChn = 0;
    MI_U8  u8RstBufCnt = 0;
    MD_HANDLE pMDHandle = NULL;
    MI_MD_static_param_t   stMDStaticParam;
    MI_VDF_MdRstHdlList_t* pstMdRstHdlListTmp = NULL;
    MI_VDF_MdRstHdlList_t* pstMdRstHdlListNew = NULL;
    MI_S32 buffer_size, col, row;
    MI_U32 u32MdRstBufSize = 0;
    MI_SYS_FrameBufExtraConfig_t stBufExtraConf;


    u8ViSrcChn  = pstAttr->unAttr.stMdAttr.u8SrcChnNum;
    u8RstBufCnt = pstAttr->unAttr.stMdAttr.u8MdBufCnt;

    MI_VDF_FUNC_ENTRY3(u8ViSrcChn, VdfChn);

    if((u8RstBufCnt < 0) || (u8RstBufCnt > MI_VDF_MD_RST_BUF_NUM_MAX))
    {
        LOG_WARN("The input MD Result Buffer Counter(%d) is out of range, set Default %d\n",
                u8RstBufCnt, MI_VDF_MD_RST_BUF_NUM_MAX);
    }

    col = pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.width  >> \
          (pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
    row = pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.height >> \
          (pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.mb_size + 2);
    buffer_size = col * row * (2 - pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl);

    // set input port buffer extconf.
    switch(pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.mb_size)
    {
        case MDMB_MODE_MB_8x8:
        stBufExtraConf.u16BufHAlignment = 8 * 16;   // width
        stBufExtraConf.u16BufVAlignment = 8;        // height
        break;

        case MDMB_MODE_MB_16x16:
        stBufExtraConf.u16BufHAlignment = 16 * 16;   // width
        stBufExtraConf.u16BufVAlignment = 16;        // height
        break;

        case MDMB_MODE_MB_4x4:
        default:
        stBufExtraConf.u16BufHAlignment = 4 * 16;   // width
        stBufExtraConf.u16BufVAlignment = 4;        // height
        break;
    }
    stBufExtraConf.bClearPadding = FALSE;
    MI_SHADOW_SetInputPortBufExtConf(_hVDFDev, VdfChn, 0, &stBufExtraConf);

    memcpy(&stMDStaticParam, &pstAttr->unAttr.stMdAttr.stMdStaticParamsIn, sizeof(MI_MD_static_param_t));
    if(MDALG_MODE_SAD_FG == stMDStaticParam.md_alg_mode)
    {
        stMDStaticParam.md_alg_mode = MDALG_MODE_SAD;
    }

    if(NULL == (pMDHandle = MI_MD_Init(&stMDStaticParam, (MI_MD_param_t *)&pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn)))
    {
        LOG_ERROR("do MD initial error\n");
        return ret;
    }

    // add the MD Entry in VDF Node list
    if(0 != _MI_VDF_AddVDFNode(pMDHandle, VdfChn, pstAttr))
    {
        LOG_ERROR("Add MD handle to list error\n");
        MI_MD_Uninit(pMDHandle);
        return ret;
    }


    //malloc buffer for MD Result Handle list
    pstMdRstHdlListNew = (MI_VDF_MdRstHdlList_t*)malloc(sizeof(MI_VDF_MdRstHdlList_t));
    if(NULL == pstMdRstHdlListNew)
    {
        MI_MD_Uninit(pMDHandle);
        _MI_VDF_DelVDFNode(VdfChn);
        LOG_ERROR("Malloc buffer for MD Result Handle list struct error\n");
        return ret;
    }

    memset(pstMdRstHdlListNew, 0x00, sizeof(MI_VDF_MdRstHdlList_t));
    pstMdRstHdlListNew->VdfChn        = VdfChn;
    pstMdRstHdlListNew->pMdHandle     = pMDHandle;
    pstMdRstHdlListNew->u8MdRstBufCnt = u8RstBufCnt;
    pstMdRstHdlListNew->next          = NULL;

    //Malloc buffer for MD Result
    switch(pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
    {
        case MDALG_MODE_FG:
            {
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                pstMdRstHdlListNew->u16MdRstBufSize = sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255);
                if(pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32))
                {
                    pstMdRstHdlListNew->u16MdRstBufSize += (sizeof(MI_U32) - pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32));
                }

                pstMdRstHdlListNew->pstMdRstList = (MI_MD_Result_t*)malloc(pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                if(NULL == pstMdRstHdlListNew->pstMdRstList)
                {
                    LOG_ERROR("Malloc buffer for MD result error\n");
                    free(pstMdRstHdlListNew);
                    pstMdRstHdlListNew = NULL;
                    MI_MD_Uninit(pMDHandle);
                    _MI_VDF_DelVDFNode(VdfChn);
                    return ret;
                }
                else
                {
                    memset(pstMdRstHdlListNew->pstMdRstList, 0x00, pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                    LOG_DEBUG("pstMdRstHdlListNew->pstMdRstList=%p, u16MdRstBufSize(*u8RstBufNum)=0x%x\n",
                            pstMdRstHdlListNew->pstMdRstList, pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                }

                break;
            }

        case MDALG_MODE_SAD:
            {
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size) * ResultBufferCount
                pstMdRstHdlListNew->u16MdRstBufSize = sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size;
                if(pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32))
                {
                    pstMdRstHdlListNew->u16MdRstBufSize += (sizeof(MI_U32) - pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32));
                }

                pstMdRstHdlListNew->pstMdRstList = (MI_MD_Result_t*)malloc(pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                if(NULL == pstMdRstHdlListNew->pstMdRstList)
                {
                    LOG_ERROR("Malloc buffer for MD result error\n");
                    free(pstMdRstHdlListNew);
                    pstMdRstHdlListNew = NULL;
                    MI_MD_Uninit(pMDHandle);
                    _MI_VDF_DelVDFNode(VdfChn);
                    return ret;
                }
                else
                {
                    memset(pstMdRstHdlListNew->pstMdRstList, 0x00, pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                    LOG_DEBUG("sizeof(MI_MD_Result_t)=0x%x, sizeof(MDSAD_DATA_t)=0x%x, buffer_size=0x%x\n",
                            sizeof(MI_MD_Result_t), sizeof(MDSAD_DATA_t), buffer_size);
                    LOG_DEBUG("pstMdRstHdlListNew->pstMdRstList=%p, u16MdRstBufSize(*u8RstBufNum)=0x%x\n",
                            pstMdRstHdlListNew->pstMdRstList,pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                }
                break;
            }

        case MDALG_MODE_SAD_FG:
            {
                //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                pstMdRstHdlListNew->u16MdRstBufSize = sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255);
                if(pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32))
                {
                    pstMdRstHdlListNew->u16MdRstBufSize += (sizeof(MI_U32) - pstMdRstHdlListNew->u16MdRstBufSize % sizeof(MI_U32));
                }

                pstMdRstHdlListNew->pstMdRstList = (MI_MD_Result_t*)malloc(pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                if(NULL == pstMdRstHdlListNew->pstMdRstList)
                {
                    LOG_ERROR("Malloc buffer for MD result error\n");
                    free(pstMdRstHdlListNew);
                    pstMdRstHdlListNew = NULL;
                    MI_MD_Uninit(pMDHandle);
                    _MI_VDF_DelVDFNode(VdfChn);
                    return ret;
                }
                else
                {
                    memset(pstMdRstHdlListNew->pstMdRstList, 0x00, pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                    LOG_DEBUG("pstMdRstHdlListNew->pstMdRstList=%p, size=0x%x\n", pstMdRstHdlListNew->pstMdRstList,pstMdRstHdlListNew->u16MdRstBufSize * u8RstBufCnt);
                }

                break;
            }

        default:
            LOG_WARN("Input the wrong md_alg_mode(%d)\n",
                    pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
            free(pstMdRstHdlListNew);
            pstMdRstHdlListNew = NULL;
            MI_MD_Uninit(pMDHandle);
            _MI_VDF_DelVDFNode(VdfChn);
            return ret;
    }


    if(NULL == MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList)
    {
        MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList = pstMdRstHdlListNew;
    }
    else
    {
        pstMdRstHdlListTmp = MI_VDF_MD_RST_LIST[u8ViSrcChn].pstMdRstHdlList;

        //当指向下一个MdRstHdl的指针不为NULL时，进入while循环直到next为NULL时退出来
        while(NULL != pstMdRstHdlListTmp->next)
        {

            LOG_DEBUG("VdfChn=0x%x, pMDHandle=0x%x, pstMdRstHdlListTmp=%p\n",
                    pstMdRstHdlListTmp->VdfChn, pstMdRstHdlListTmp->pMdHandle, pstMdRstHdlListTmp);

            pstMdRstHdlListTmp = (MI_VDF_MdRstHdlList_t*)pstMdRstHdlListTmp->next;
        }

        // add new buf to the tail of the list
        pstMdRstHdlListTmp->next = pstMdRstHdlListNew;
    }

    LOG_DEBUG("Create VDF Node: ViSrcChn=%d md_hdl=%p MdRstHdl=%p\n", u8ViSrcChn, pMDHandle, pstMdRstHdlListNew);

    MI_VDF_FUNC_EXIT3(u8ViSrcChn, VdfChn);

    ret = 0;
    return ret;
}


MI_S32 _MI_VDF_CreateChn_OD(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr)
{
    MI_S32 ret = -1;
    MI_U8  u8ViSrcChn = 0;
    MI_U8  u8RstBufCnt = 0;
    OD_HANDLE pODHandle = NULL;
    MI_VDF_OdRstHdlList_t* pstOdRstHdlListTmp = NULL;
    MI_VDF_OdRstHdlList_t* pstOdRstHdlListNew = NULL;

    MI_OD_param_t stOdDynamicParamsIn;
    MI_OD_static_param_t stOdStaticParamsIn;

    u8ViSrcChn = pstAttr->unAttr.stOdAttr.u8SrcChnNum;
    u8RstBufCnt = pstAttr->unAttr.stOdAttr.u8OdBufCnt;

    MI_VDF_FUNC_ENTRY3(u8ViSrcChn, VdfChn);

    stOdStaticParamsIn.inImgW   = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.inImgW;
    stOdStaticParamsIn.inImgH   = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.inImgH;
    stOdStaticParamsIn.nClrType = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.nClrType;
    stOdStaticParamsIn.div      = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.div;
    stOdStaticParamsIn.alpha    = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.alpha;
    stOdStaticParamsIn.M        = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.M;
    stOdStaticParamsIn.MotionSensitivity = pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.MotionSensitivity;
    memcpy(&stOdStaticParamsIn.roi_od, &pstAttr->unAttr.stOdAttr.stOdStaticParamsIn.roi_od, sizeof(ODROI_t));
    //memcpy(&stOdStaticParamsIn, &pstAttr->unAttr.stOdAttr.stOdStaticParamsIn, sizeof(MI_OD_static_param_t));

    stOdDynamicParamsIn.min_duration   = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.min_duration;
    stOdDynamicParamsIn.tamper_blk_thd = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd;
    stOdDynamicParamsIn.thd_tamper     = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper;
    //memcpy(&stOdDynamicParamsIn, &pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn, sizeof(MI_OD_param_t));

    if(NULL == (pODHandle = MI_OD_Init(&stOdStaticParamsIn, &stOdDynamicParamsIn)))
    {
        LOG_ERROR("do OD initial error\n");
        return ret;
    }

    if(MI_OD_RET_SUCCESS != MI_OD_SetAttr(pODHandle, &stOdDynamicParamsIn))
    {
        LOG_ERROR("Set OD attribute error\n");
        return ret;
    }

    // add the OD handle in VDF list
    if(0 != _MI_VDF_AddVDFNode(pODHandle, VdfChn, pstAttr))
    {
        LOG_ERROR("Add OD handle to list error\n");
        MI_OD_Uninit(pODHandle);
        return ret;
    }

    //Malloc buffer for OD Handle list
    pstOdRstHdlListNew = (MI_VDF_OdRstHdlList_t*)malloc(sizeof(MI_VDF_OdRstHdlList_t));
    if(NULL == pstOdRstHdlListNew)
    {
        MI_OD_Uninit(pODHandle);
        _MI_VDF_DelVDFNode(VdfChn);
        LOG_ERROR("malloc buf for OD Result Handle list error\n");
        return ret;
    }

    memset(pstOdRstHdlListNew, 0x00, sizeof(MI_VDF_OdRstHdlList_t));
    pstOdRstHdlListNew->pOdHandle     = pODHandle;
    pstOdRstHdlListNew->VdfChn        = VdfChn;
    pstOdRstHdlListNew->u8OdRstBufCnt = u8RstBufCnt;
    pstOdRstHdlListNew->next          = NULL;

    //Malloc buffer for OD Result
    pstOdRstHdlListNew->pstOdResultList = (MI_OD_Result_t*)malloc(sizeof(MI_OD_Result_t) * u8RstBufCnt);
    if(NULL == pstOdRstHdlListNew->pstOdResultList)
    {
        LOG_ERROR("malloc buffer for OD result error\n");
        MI_OD_Uninit(pODHandle);
        _MI_VDF_DelVDFNode(VdfChn);
        free(pstOdRstHdlListNew);
        pstOdRstHdlListNew = NULL;
        return ret;
    }

    memset(pstOdRstHdlListNew->pstOdResultList, 0x00, sizeof(MI_OD_Result_t) * u8RstBufCnt);

    if(NULL == MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList)
    {
        MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList = pstOdRstHdlListNew;
    }
    else
    {
        pstOdRstHdlListTmp = MI_VDF_OD_RST_LIST[u8ViSrcChn].pstOdRstHdlList;

        while(NULL != pstOdRstHdlListTmp->next)
        {
            LOG_DEBUG("VdfChn=0x%x, pODHandle=0x%x, pstOdRstHdlListTmp=%p\n",
                    pstOdRstHdlListTmp->VdfChn, pstOdRstHdlListTmp->pOdHandle, pstOdRstHdlListTmp);
            pstOdRstHdlListTmp = (MI_VDF_OdRstHdlList_t*)pstOdRstHdlListTmp->next;
        }

        // add new buf to the tail of the list
        pstOdRstHdlListTmp->next = pstOdRstHdlListNew;
    }

    LOG_INFO("Create VDF Node: ViSrcChn=%d, od_hdl=%p OdRstHdl=%p\n", u8ViSrcChn, pODHandle, pstOdRstHdlListNew);

    MI_VDF_FUNC_EXIT3(u8ViSrcChn, VdfChn);

    ret = 0;
    return ret;
}

MI_S32 _MI_VDF_CreateViSrcThread(MI_VDF_CHANNEL VdfChn)
{
    MI_S32 ret = -1;
    MI_U8  u8ViSrcChn = 0;
    MI_U8  u8RstBufCnt = 0;
    struct list_head *pos = NULL, *n = NULL;
    VDF_ChnAttr_Object_t *pstVdfChnAttrObj= NULL;

    MI_VDF_FUNC_ENTRY2(VdfChn);

    // 1. 读取当前chn对应的vdfChn_list成员
    if(list_empty(&vdfChn_list))
    {
        LOG_ERROR("The input vdfChn_list is NULL \n");
        return ret;
    }

    list_for_each_safe(pos, n, &vdfChn_list)
    {
        pstVdfChnAttrObj = list_entry(pos, VDF_ChnAttr_Object_t, list);
        if(pstVdfChnAttrObj->vdfChn == VdfChn)
        {
            break;
        }
        else
        {
            pstVdfChnAttrObj = NULL;
        }
    }

    if(!pstVdfChnAttrObj)
    {
        LOG_ERROR("pstVdfChnAttrObj is NULL \n");
        return ret;
    }

    // 2.根据pstVdfChnAttrObj参数创建MD/OD
    switch(pstVdfChnAttrObj->vdfChn_Attr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            u8ViSrcChn = pstVdfChnAttrObj->vdfChn_Attr.unAttr.stMdAttr.u8SrcChnNum;
            u8RstBufCnt = pstVdfChnAttrObj->vdfChn_Attr.unAttr.stMdAttr.u8MdBufCnt;
            // prepare the VDF-MD result list
            if((u8ViSrcChn < 0) || (u8ViSrcChn >= MI_VDF_SOURCE_NUM_MAX))
            {
                LOG_ERROR("The input Source ViSrcChn(%d) is out of range\n", u8ViSrcChn);
                goto EXIT_FAIL;
            }
            if((u8RstBufCnt < 0) || (u8RstBufCnt > MI_VDF_MD_RST_BUF_NUM_MAX))
            {
                goto EXIT_FAIL;
            }

            pthread_mutex_lock(&mutexVDF);

            if(0 != (ret = _MI_VDF_CreateChn_MD(pstVdfChnAttrObj->vdfChn, &pstVdfChnAttrObj->vdfChn_Attr)))
            {
                LOG_ERROR("call _MI_VDF_CreateChn_MD() error!\n");
                pthread_mutex_unlock(&mutexVDF);
                goto EXIT_FAIL;
            }

            pthread_mutex_unlock(&mutexVDF);
            break;

        case E_MI_VDF_WORK_MODE_OD:
            u8ViSrcChn = pstVdfChnAttrObj->vdfChn_Attr.unAttr.stOdAttr.u8SrcChnNum;
            u8RstBufCnt = pstVdfChnAttrObj->vdfChn_Attr.unAttr.stOdAttr.u8OdBufCnt;

            if((u8ViSrcChn < 0) || (u8ViSrcChn >= MI_VDF_SOURCE_NUM_MAX))
            {
                LOG_ERROR("The input Source ViSrcChn(%d) is out of range\n", u8ViSrcChn);
                goto EXIT_FAIL;
            }

            if((u8RstBufCnt < 0) || (u8RstBufCnt > MI_VDF_OD_RST_BUF_NUM_MAX))
            {
                LOG_ERROR("The input OD buff Num(%d) is out of range\n", u8RstBufCnt);
                goto EXIT_FAIL;
            }

            pthread_mutex_lock(&mutexVDF);
            if(0 != (ret = _MI_VDF_CreateChn_OD(pstVdfChnAttrObj->vdfChn, &pstVdfChnAttrObj->vdfChn_Attr)))
            {
                LOG_ERROR("call _MI_VDF_CreateChn_OD() error!\n");
                pthread_mutex_unlock(&mutexVDF);
                goto EXIT_FAIL;
            }
            pthread_mutex_unlock(&mutexVDF);
            break;

        default:
            LOG_WARN("Input the wrong Work Mode(%d)\n", pstVdfChnAttrObj->vdfChn_Attr.enWorkMode);
            break;
    }

    // 3.根据u8ViSrcChn创建线程
    pthread_mutex_lock(&mutexVDF);      // TODO:fixme
    //start the APP thread of the video source channel
    if(0 == VDF_PTHREAD_MUTEX[u8ViSrcChn].u8Enable)
    {
        //MI_U32 argu = 0;
        MI_S8 cmdbuff[32];

        memset(cmdbuff, 0x00, sizeof(cmdbuff));
        snprintf(cmdbuff, sizeof(cmdbuff), "vdf_APP_task_%u", u8ViSrcChn);

        //argu = u8ViSrcChn;

        LOG_INFO("start %s\n", cmdbuff);
        pthread_create(&(pthreadVDFAPP[u8ViSrcChn]), NULL, vdf_APP_task, (void *)((MI_U32)u8ViSrcChn));
        pthread_setname_np(pthreadVDFAPP[u8ViSrcChn] , cmdbuff);

        VDF_PTHREAD_MUTEX[u8ViSrcChn].u8Enable = 1;
    }
    pthread_mutex_unlock(&mutexVDF);
    MI_VDF_FUNC_EXIT();
    return MI_SUCCESS;

EXIT_FAIL:
    return ret;
}

MI_S32 MI_VDF_CreateChn(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr)
{
    MI_S32 ret = -1;
    VDF_ChnAttr_Object_t *pstAttrObj = NULL;

    MI_VDF_FUNC_ENTRY();

    // TODO: 检查线程是否需要同步
    if(NULL == pstAttr)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return ret;
    }

    if(0 == _MI_VDF_CheckChnValid(VdfChn))
    {
        LOG_ERROR("The input VdfChn(0x%x) has been defined already\n", VdfChn);
        return ret;
    }

    // add vdfChn attr to attr list.
    pstAttrObj = (VDF_ChnAttr_Object_t *)malloc(sizeof(VDF_ChnAttr_Object_t));
    if (pstAttrObj)
    {
        memcpy(&pstAttrObj->vdfChn_Attr, pstAttr, sizeof(MI_VDF_ChnAttr_t));
        pstAttrObj->vdfChn = VdfChn;
        list_add_tail(&pstAttrObj->list, &vdfChn_list);
        LOG_INFO("malloc size:%d addr:%p succes!!! \n", sizeof(VDF_ChnAttr_Object_t), pstAttrObj);
        LOG_INFO("pstAttrObj->vdfChn:%d\n", pstAttrObj->vdfChn);
        LOG_INFO("pstAttrObj->vdfChn_Attr.enWorkMode:%d\n", pstAttrObj->vdfChn_Attr.enWorkMode);

    }
    else
    {
        LOG_ERROR("malloc size: %d fail!!! \n", sizeof(VDF_ChnAttr_Object_t));
        return ret;
    }

    if ( MI_SHADOW_EnableChannel(_hVDFDev, VdfChn) ||
            MI_SHADOW_EnableInputPort(_hVDFDev, VdfChn, 0))
        return ret;

    struct list_head *pos = NULL, *n = NULL;
    VDF_ChnAttr_Object_t *pstVdfChnAttrObj= NULL;


    //TODO: fixe me!!
    list_for_each_safe(pos, n, &vdfChn_list)
    {
        pstVdfChnAttrObj = list_entry(pos, VDF_ChnAttr_Object_t, list);
        LOG_INFO("_MI_VDF_OnBindInputPort: pstAttrObj->vdfChn:%d\n", pstVdfChnAttrObj->vdfChn);
        LOG_INFO("_MI_VDF_OnBindInputPort: pstAttrObj->vdfChn_Attr.enWorkMode:%d\n", pstVdfChnAttrObj->vdfChn_Attr.enWorkMode);
    }

    MI_VDF_FUNC_EXIT();

    return MI_SUCCESS;
}


MI_S32 MI_VDF_DestroyChn(MI_VDF_CHANNEL VdfChn)
{
    MI_VDF_FUNC_ENTRY();


    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    //1st: un-init the MD/OD func
    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            //TBD:stop the MD
            pthread_mutex_lock(&mutexVDF);
            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));
            MI_MD_Uninit(g_pstVDFNode[VdfChn]->phandle);
            _MI_VDF_DelVDFNode(VdfChn);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));
            pthread_mutex_unlock(&mutexVDF);

            break;

        case E_MI_VDF_WORK_MODE_OD:
            //TBD:stop the OD
            pthread_mutex_lock(&mutexVDF);
            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));

            MI_OD_Uninit(g_pstVDFNode[VdfChn]->phandle);
            _MI_VDF_DelVDFNode(VdfChn);

            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));
            pthread_mutex_unlock(&mutexVDF);

            break;

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    //2nd: free the MD/OD resault buffer
    if(E_MI_VDF_WORK_MODE_MD == g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        MI_VDF_MdRstHdlList_t* pstMdRstHdlList_Next = NULL;

        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList
            = MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList;

        // check the 1st node
        if((MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->VdfChn == VdfChn) && \
                (MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pMdHandle == g_pstVDFNode[VdfChn]->phandle))
        {
            MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList
                = MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->next;

            free(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList);
            free(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));
            MI_VDF_FUNC_EXIT();
            return 0;
        }

        pstMdRstHdlList_Next = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->next;

        while(NULL != pstMdRstHdlList_Next)
        {
            if((pstMdRstHdlList_Next->VdfChn == VdfChn) && \
                    (pstMdRstHdlList_Next->pMdHandle == g_pstVDFNode[VdfChn]->phandle))
            {
                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->next = pstMdRstHdlList_Next->next;
                free(pstMdRstHdlList_Next->pstMdRstList);
                free(pstMdRstHdlList_Next);
                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));
                MI_VDF_FUNC_EXIT();
                return 0;
            }

            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList = pstMdRstHdlList_Next;
            pstMdRstHdlList_Next = pstMdRstHdlList_Next->next;
        }
    }
    else if(E_MI_VDF_WORK_MODE_OD == g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        MI_VDF_OdRstHdlList_t* pstOdRstHdlList_Next = NULL;

        pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));

        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
            = MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList;

        // check the 1st node
        if((MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->VdfChn == VdfChn) && \
                (MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pOdHandle == g_pstVDFNode[VdfChn]->phandle))
        {
            MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
                = MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->next;

            free(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pstOdResultList);
            free(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList);
            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));
            MI_VDF_FUNC_EXIT();
            return 0;
        }

        pstOdRstHdlList_Next = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->next;

        while(NULL != pstOdRstHdlList_Next)
        {
            if((pstOdRstHdlList_Next->VdfChn == VdfChn) && \
                    (pstOdRstHdlList_Next->pOdHandle == g_pstVDFNode[VdfChn]->phandle))
            {
                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->next = pstOdRstHdlList_Next->next;
                free(pstOdRstHdlList_Next->pstOdResultList);
                free(pstOdRstHdlList_Next);
                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));
                MI_VDF_FUNC_EXIT();
                return 0;
            }

            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList = (MI_VDF_OdRstHdlList_t*)pstOdRstHdlList_Next;
            pstOdRstHdlList_Next = (MI_VDF_OdRstHdlList_t*)pstOdRstHdlList_Next->next;
        }
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_SetChnAttr(MI_VDF_CHANNEL VdfChn, const MI_VDF_ChnAttr_t* pstAttr)
{
    MI_VDF_FUNC_ENTRY();

    if(NULL == pstAttr)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return -1;
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            {
                MI_MD_param_t  stMdDynamicParam;

                pthread_mutex_lock(&mutexVDF);

                memset(&stMdDynamicParam, 0x00, sizeof(MI_MD_param_t));

                stMdDynamicParam.sensitivity = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.sensitivity;
                stMdDynamicParam.learn_rate  = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.learn_rate;
                stMdDynamicParam.md_thr      = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.md_thr;
                stMdDynamicParam.obj_num_max = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max;

                if(MI_MD_RET_SUCCESS == MI_MD_SetParam(g_pstVDFNode[VdfChn]->phandle, &stMdDynamicParam))
                {
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.sensitivity;
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.learn_rate  = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.learn_rate;
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.md_thr      = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.md_thr;
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max = pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max;
                }
                else
                {
                    LOG_ERROR("Set MD param error! In: sensitivity=%u, learn_rate=%u, md_thr=%u, obj_num_max=%u\n",
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.sensitivity,
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.learn_rate,
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.md_thr,
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max);
                }
                pthread_mutex_unlock(&mutexVDF);
                break;
            }

        case E_MI_VDF_WORK_MODE_OD:
            {
                MI_OD_param_t stOdDynamicParamsIn;

                pthread_mutex_lock(&mutexVDF);

                stOdDynamicParamsIn.min_duration   = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.min_duration;
                stOdDynamicParamsIn.tamper_blk_thd = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd;
                stOdDynamicParamsIn.thd_tamper     = pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper;

                if(MI_OD_RET_SUCCESS == MI_OD_SetAttr(g_pstVDFNode[VdfChn]->phandle, &stOdDynamicParamsIn))
                {
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = stOdDynamicParamsIn.min_duration;
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = stOdDynamicParamsIn.tamper_blk_thd;
                    g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = stOdDynamicParamsIn.thd_tamper;
                }
                else
                {
                    LOG_ERROR("Set OD param error! In: thd_tamper=%u, tamper_blk_thd=%u, min_duration=%u\n",
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper,
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd,
                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.stOdDynamicParamsIn.min_duration);
                }
                pthread_mutex_unlock(&mutexVDF);
                break;
            }

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n",g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_GetChnAttr(MI_VDF_CHANNEL VdfChn, MI_VDF_ChnAttr_t* pstAttr)
{
    MI_VDF_FUNC_ENTRY();

    if(NULL == pstAttr)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return -1;
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            {
                MI_MD_param_t    stMdDynamicParam_tmp;

                memset(&stMdDynamicParam_tmp, 0x00, sizeof(MI_MD_param_t));

                if(MI_RET_SUCESS == MI_MD_GetParam(g_pstVDFNode[VdfChn]->phandle, &stMdDynamicParam_tmp))
                {
                    pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = stMdDynamicParam_tmp.sensitivity;
                    pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.learn_rate  = stMdDynamicParam_tmp.learn_rate;
                    pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.md_thr      = stMdDynamicParam_tmp.md_thr;
                    pstAttr->unAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max = stMdDynamicParam_tmp.obj_num_max;
                }
                else
                {
                    LOG_ERROR("Get VdfChn(0x%x) Window Parames error!\n", VdfChn);
                    return -1;
                }
                break;
            }

        case E_MI_VDF_WORK_MODE_OD:
            {
                MI_OD_param_t    stOdDynamicParam_tmp;

                memset(&stOdDynamicParam_tmp, 0x00, sizeof(MI_OD_param_t));

                if(MI_OD_RET_SUCCESS == MI_OD_GetAttr(g_pstVDFNode[VdfChn]->phandle, &stOdDynamicParam_tmp))
                {
                    pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = stOdDynamicParam_tmp.thd_tamper;
                    pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = stOdDynamicParam_tmp.tamper_blk_thd;
                    pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = stOdDynamicParam_tmp.min_duration;
                }
                else
                {
                    LOG_ERROR("Get VdfChn(0x%x) Parames error!\n", VdfChn);
                    return -1;
                }

                /*if(MI_OD_RET_SUCCESS != (ret = MI_OD_GetDetectWindowSize(phandle, uint16_t* st_x, uint16_t* st_y, uint16_t* div_w, uint16_t* div_h)))
                  {
                  LOG_ERROR("Get VdfChn(0x%x) Parames error!\n", VdfChn);
                  return -1;
                  }
                  else
                  {
                  pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = stOdDynamicParam_tmp.thd_tamper;
                  pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = stOdDynamicParam_tmp.tamper_blk_thd;
                  pstAttr->unAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = stOdDynamicParam_tmp.min_duration;
                  }*/
                break;
            }

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_EnableSubWindow(MI_VDF_CHANNEL VdfChn, MI_U8 u8Col, MI_U8 u8Row, MI_U8 u8Enable)
{
    MI_U8 idx = 0;


    MI_VDF_FUNC_ENTRY2(VdfChn);     // TODO:fixme

    // 0. wait onbindinputport callback map vdfchn
    while (!_MI_VDF_Is_Map_VdfChn(&idx, VdfChn, 0))
    {
        usleep(VDF_USLEEP_DEFAULT_TIME);
    }
    LOG_DEBUG("_MI_VDF_Is_Map_VdfChn == TRUE!, VdfChn=%d \n", VdfChn);

    // 1. creat workthread by ViSrcIndex
    if (MI_SUCCESS != _MI_VDF_CreateViSrcThread(VdfChn))
    {
        LOG_ERROR("_MI_VDF_CreateViSrcThread failed!\n");
    }

    // 2. enable subwindows
    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));

            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u8Enable = u8Enable;

            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));
            break;

        case E_MI_VDF_WORK_MODE_OD:

            pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));

            g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.u8Enable = u8Enable;

            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexRUN));
            break;

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_Run(MI_VDF_WorkMode_e      enWorkMode)
{
    MI_VDF_FUNC_ENTRY();

    switch(enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            g_MDEnable = 1;

            break;

        case E_MI_VDF_WORK_MODE_OD:
            g_ODEnable = 1;

            break;

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_Stop(MI_VDF_WorkMode_e      enWorkMode)
{
    MI_VDF_FUNC_ENTRY();

    switch(enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            g_MDEnable = 0;

            break;

        case E_MI_VDF_WORK_MODE_OD:
            g_ODEnable = 0;

            break;

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_GetResult(MI_VDF_CHANNEL VdfChn, MI_VDF_Result_t* pstVdfResult, MI_S32 s32MilliSec)
{
    MI_VDF_FUNC_ENTRY2(VdfChn);

    if(NULL == pstVdfResult)
    {
        LOG_ERROR("The input pointer(MI_VDF_Result_t*) is NULL\n");
        return -1;
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            {
                //Get the MD detect result from the list
                if(NULL == MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    LOG_ERROR("VdfChn=0x%x  Get MD Result handle list error\n", VdfChn);
                    break;
                }

                //find the MD handle from the Handle list
                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList
                    = MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList;

                while(NULL != VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    if((g_pstVDFNode[VdfChn]->VdfChn == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->VdfChn) && \
                            (g_pstVDFNode[VdfChn]->phandle == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pMdHandle))
                    {
                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList;

                        //如果没有数据可取，则等待设定的时间
                        if((0 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData) && (0 < s32MilliSec))
                        {
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].tv.tv_sec  = s32MilliSec / 1000;
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].tv.tv_usec = (s32MilliSec % 1000) * 1000;

                            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

                            //wait for write MD detect result to the list
                            // TODO:fixme
                            select(0, NULL, NULL, NULL, &VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u8SrcChnNum].tv);

                            if (0 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData)
                            {
                                return -1;
                            }
                            else
                            {
                                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));
                            }
                        }

                        if(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst
                                < VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdRstBufCnt)
                        {
                            switch(g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
                            {
                                case MDALG_MODE_FG:
                                    {
                                        //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                                        //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            += (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u16MdRstBufSize
                                                    * VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                                            = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDObjData
                                            = (MDOBJ_DATA_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        LOG_DEBUG("pstMdResult=%p MdReadPst=%d MdWritePst=%d MdDeltData=%d\n",
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading   = 1; //设置此标志位，写操作时不能覆盖该buff数据
                                        pstVdfResult->unVdfResult.stMdResult.u64Pts      = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u64Pts;
                                        pstVdfResult->unVdfResult.stMdResult.u8Enable    = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Enable;
                                        pstVdfResult->unVdfResult.stMdResult.u8Reading   = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultSad = NULL;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultObj = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDObjData;
                                        break;
                                    }

                                case MDALG_MODE_SAD:
                                    {
                                        //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                                        //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size) * ResultBufferCount
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            += (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u16MdRstBufSize
                                                    * VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                                            = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDSadData
                                            = (MDSAD_DATA_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        /*
                                           LOG_DEBUG("pstMdResult=%p MdReadPst=%d MdWritePst=%d MdDeltData=%d\n",
                                           VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList,
                                           VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                                           VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                                           VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData);
                                           */
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading = 1; //设置此标志位，写操作时不能覆盖该buff数据
                                        pstVdfResult->unVdfResult.stMdResult.u64Pts    = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u64Pts;
                                        pstVdfResult->unVdfResult.stMdResult.u8Enable  = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Enable;
                                        pstVdfResult->unVdfResult.stMdResult.u8Reading = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultSad = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDSadData;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultObj = NULL;
                                        break;
                                    }

                                case MDALG_MODE_SAD_FG:
                                    {
                                        //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                                        //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                            += (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u16MdRstBufSize
                                                    * VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                                            = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp += sizeof(MI_MD_Result_t);
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDSadData
                                            = (MDSAD_DATA_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp += sizeof(MDSAD_DATA_t);
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDObjData
                                            = (MDOBJ_DATA_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                        LOG_DEBUG("pstMdResult=%p MdReadPst=%d MdWritePst=%d MdDeltData=%d\n",
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading   = 1; //设置此标志位，写操作时不能覆盖该buff数据
                                        pstVdfResult->unVdfResult.stMdResult.u64Pts    = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u64Pts;
                                        pstVdfResult->unVdfResult.stMdResult.u8Enable  = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Enable;
                                        pstVdfResult->unVdfResult.stMdResult.u8Reading = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Reading;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultSad = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDSadData;
                                        pstVdfResult->unVdfResult.stMdResult.pstMdResultObj = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMDObjData;
                                        break;
                                    }

                                default:
                                    LOG_WARN("Input the wrong md_alg_mode(%d)\n",
                                            g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
                                    break;
                            }
                        }
                        else
                        {
                            /*
                               LOG_INFO("VdfChn(0x%x) D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n", VdfChn,
                               VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData,
                               VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                               VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                               g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.u8MdBufCnt);
                               */
                        }

                        break; //break the while loop
                    }

                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList
                        = (MI_VDF_MdRstHdlList_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->next;
                }

                if(NULL == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    LOG_INFO("VdfChn=%d Cannot find the MD Result handle list! g_pstVDFNode[VdfChn]->u8SrcChnNum:%d\n", VdfChn, g_pstVDFNode[VdfChn]->u8SrcChnNum);
                }

                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

                break; //breake the case:E_MI_VDF_WORK_MODE_MD
            }

        case E_MI_VDF_WORK_MODE_OD:
            {
                //Get the OD detect result from the list
                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
                    = MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList;

                if(NULL == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
                {
                    LOG_ERROR("VdfChn=0x%x  Get OD Result handle list error\n", VdfChn);
                    break;
                }

                //find the OD handle from the Handle list
                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));

                while(NULL != VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
                {
                    if((g_pstVDFNode[VdfChn]->VdfChn  == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->VdfChn) && \
                            (g_pstVDFNode[VdfChn]->phandle == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pOdHandle))
                    {
                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList
                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pstOdResultList;

                        //如果没有数据可取，则等待设定的时间
                        if((0 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData) && (0 < s32MilliSec))
                        {
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].tv.tv_sec  = s32MilliSec / 1000;
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].tv.tv_usec = (s32MilliSec % 1000) * 1000;

                            pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));

                            //wait for write OD detect result to the list
                            select(0, NULL, NULL, NULL, &VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].tv);

                            if (0 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData)
                            {
                                return -1;
                            }
                            else
                            {
                                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));
                            }
                        }

                        if(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst
                                > VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdRstBufCnt)
                        {
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst
                                %= g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.u8OdBufCnt;
                        }

                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList
                            += VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst;

                        if((0 < VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData) &&
                                (1 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8Enable) &&
                                (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst
                                 < VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdRstBufCnt))
                        {

                            memcpy(&pstVdfResult->unVdfResult.stOdResult, \
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList, \
                                    sizeof(MI_OD_Result_t));
#if (VDF_DBG_LOG_ENABLE)
                            LOG_DEBUG("[VdfChn=0x%x, hdl=%p ViSrc=%d] pts=0x%llx DataLen=%d (%d, %d)] Get OD-Rst data:\n",
                                    VdfChn, g_pstVDFNode[VdfChn]->phandle, g_pstVDFNode[VdfChn]->stAttr.unAttr.stOdAttr.u8SrcChnNum,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u64Pts,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8DataLen,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8WideDiv,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8HightDiv);
                            LOG_DEBUG("{(%d %d %d) (%d %d %d) (%d %d %d)}\n",
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[0][0],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[0][1],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[0][2],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[1][0],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[1][1],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[1][2],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[2][0],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[2][1],
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8RgnAlarm[2][2]);
#endif
                        }
                        else
                        {
                            LOG_DEBUG("VdfChn(0x%x) D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n", VdfChn,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdWritePst,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdRstBufCnt);
                        }

                        break; //break the while loop
                    }

                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
                        = (MI_VDF_OdRstHdlList_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->next;
                }

                if(NULL == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
                {
                    LOG_INFO("VdfChn=%d Cannot find the OD Result handle list! g_pstVDFNode[VdfChn]->u8SrcChnNum:%d\n", VdfChn, g_pstVDFNode[VdfChn]->u8SrcChnNum);
                }

                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));

                break; //breake the case:E_MI_VDF_WORK_MODE_OD
            }

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", pstVdfResult->enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}


MI_S32 MI_VDF_PutResult(MI_VDF_CHANNEL VdfChn, MI_VDF_Result_t* pstVdfResult)
{
    MI_VDF_FUNC_ENTRY2(VdfChn);

    if(NULL == pstVdfResult)
    {
        LOG_ERROR("The input pointer(MI_VDF_RESULT_S*) is NULL\n");
        return -1;
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            {
                //Get the MD detect result from the list
                if(NULL == MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    LOG_ERROR("VifSrc=%u MD Get result Handle list fail\n", g_pstVDFNode[VdfChn]->u8SrcChnNum);
                    break;
                }

                //find the MD handle from the Handle list
                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList = MI_VDF_MD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList;

                while(NULL != VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    if((0 < VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData) && \
                            (g_pstVDFNode[VdfChn]->VdfChn  == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->VdfChn) && \
                            (g_pstVDFNode[VdfChn]->phandle == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pMdHandle))
                    {
                        //LOG_DEBUG("pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode:%d\n", pstVDFNode->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
                        switch(g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
                        {
                            case MDALG_MODE_FG:
                            case MDALG_MODE_SAD:
                            case MDALG_MODE_SAD_FG:
                                {
                                    //在_MI_VDF_CreateChn_MD()函数中申请保存MD检测结果的内存大小如下
                                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDOBJ_DATA_t) + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + buffer_size) * ResultBufferCount
                                    //MdRstBufSize = (sizeof(MI_MD_Result_t) + sizeof(MDSAD_DATA_t) + sizeof(MDOBJ_DATA_t) + buffer_size + (sizeof(MDOBJ_t) * 255)) * ResultBufferCount
                                    //Step1:获取MD运算结果链表的首地址
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                                        = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->pstMdRstList;
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                        = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList;

                                    //Step2:从MD运算结果链表中找到目标节点的地址
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp
                                        += (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u16MdRstBufSize
                                                * VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst);
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList
                                        = (MI_MD_Result_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstListTmp;

                                    LOG_DEBUG("pstMdResult=%p MdReadPst=%d MdWritePst=%d MdDeltData=%d\n",
                                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList,
                                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData);

                                    if((1 == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u8Enable) &&
                                            (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u64Pts == pstVdfResult->unVdfResult.stMdResult.u64Pts))
                                    {
                                        //pu8MdRstData->u8Enable = 0;
                                        memset(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList, 0x00, \
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u16MdRstBufSize);

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst++;
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst
                                            %= VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdRstBufCnt;

                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData--;
                                        if(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData < 0)
                                        {
                                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData = 0;
                                        }

                                        LOG_DEBUG("pstMdResult=%p MdReadPst=%d MdWritePst=%d MdDeltData=%d\n",
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdReadPst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->u8MdWritePst,
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->s8MdDeltData);
                                    }
                                    else
                                    {
                                        LOG_WARN("the MD Result(Pts=0x%x) has been released or covered!\n",
                                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstList->u64Pts);
                                    }
                                    break;
                                }

                            default:
                                LOG_WARN("Input the wrong md_alg_mode(%d)\n",
                                        g_pstVDFNode[VdfChn]->stAttr.unAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
                                break;
                        }

                        break; //break the while loop
                    }

                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList
                        = (MI_VDF_MdRstHdlList_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList->next;
                }

                if(NULL == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstMdRstHdlList)
                {
                    LOG_INFO("VdfChn=%d Cannot find the MD Result handle list! g_pstVDFNode[VdfChn]->u8SrcChnNum:%d\n", VdfChn, g_pstVDFNode[VdfChn]->u8SrcChnNum);
                }

                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexMDRW));

                break; //break the case:E_MI_VDF_WORK_MODE_MD
            }

        case E_MI_VDF_WORK_MODE_OD:
            //Get the OD detect result from the list
            if(NULL != MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
            {
                //find the MD handle from the Handle list
                pthread_mutex_lock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));

                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
                    = MI_VDF_OD_RST_LIST[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList;

                while(NULL != VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
                {
                    if((0 < VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData) && \
                            (g_pstVDFNode[VdfChn]->VdfChn  == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->VdfChn) && \
                            (g_pstVDFNode[VdfChn]->phandle == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pOdHandle))
                    {
                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList
                            = VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->pstOdResultList;

                        LOG_INFO("pstOdResult=%p sizeof(MI_OD_Result_t)=0x%x\n",
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList, sizeof(MI_OD_Result_t));

                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList
                            += VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst;

                        LOG_INFO("pstOdResult=%p DataLen=%u enable=%u pts=0x%llx\n", \
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList,
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8DataLen,
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8Enable,
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u64Pts);

                        if((VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8Enable == 1) &&
                                (VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u64Pts == pstVdfResult->unVdfResult.stOdResult.u64Pts))
                        {
                            memset(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList, 0x00, sizeof(MI_OD_Result_t));
                            LOG_INFO("pstOdResult=%p DataLen=%u enable=%u pts=0x%llx\n", \
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8DataLen,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u8Enable,
                                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstList->u64Pts);

                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst++;
                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst
                                %= VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdRstBufCnt;

                            VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData--;
                            if(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData < 0)
                            {
                                VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData = 0;
                            }

                            if(VDF_DBG_LOG_ENABLE)
                            {
                                LOG_INFO("OD D_Delt=%d, W_Ptr=%u, R_Ptr=%u, buf_Size=%u\n",
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->s8OdDeltData,
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdWritePst,
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdReadPst,
                                        VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->u8OdRstBufCnt);
                            }
                        }

                        break; //break the while loop
                    }

                    VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList
                        = (MI_VDF_OdRstHdlList_t*)VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList->next;
                }

                if(NULL == VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].pstOdRstHdlList)
                {
                    LOG_INFO("VdfChn=%d Cannot find the OD Result handle list! g_pstVDFNode[VdfChn]->u8SrcChnNum:%d\n", VdfChn, g_pstVDFNode[VdfChn]->u8SrcChnNum);
                }

                // TODO: fixme!
                pthread_mutex_unlock(&(VDF_PTHREAD_MUTEX[g_pstVDFNode[VdfChn]->u8SrcChnNum].mutexODRW));
            }
            else
            {
                LOG_ERROR("VivSrc=%u OD Get result Handle list fail\n", g_pstVDFNode[VdfChn]->u8SrcChnNum);
            }

            break; //break the case:E_MI_VDF_WORK_MODE_OD

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();

    return 0;
}


MI_S32 MI_VDF_GetLibVersion(MI_VDF_CHANNEL VdfChn, MI_U32* u32VDFVersion)
{
    MI_VDF_FUNC_ENTRY();

    if(NULL == u32VDFVersion)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return -1;
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        if(0 != _MI_VDF_GetVDFNode(VdfChn, &g_pstVDFNode[VdfChn]))
        {
            LOG_ERROR("get VdfChn(0x%x) Node info error\n", VdfChn);
            return -1;
        }
    }

    if(NULL == g_pstVDFNode[VdfChn])
    {
        LOG_ERROR("get VdfChn Node info error\n");
        return -1;
    }

    *u32VDFVersion = 0;

    switch(g_pstVDFNode[VdfChn]->stAttr.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
            MI_MD_GetLibVersion(g_pstVDFNode[VdfChn]->phandle);
            *u32VDFVersion = MI_MD_GetLibVersion();
            break;

        case E_MI_VDF_WORK_MODE_OD:
            *u32VDFVersion = MI_OD_GetLibVersion();
            break;

        default:
            LOG_WARN("get the wrong WorkMode(%d)\n", g_pstVDFNode[VdfChn]->stAttr.enWorkMode);
            return -1;
    }

    MI_VDF_FUNC_EXIT();
    return 0;
}

MI_S32 MI_VDF_Query(MI_VDF_CHANNEL VdfChn, MI_VDF_ChnStat_t *pstChnState)
{
    MI_S32 ret = 0;

    MI_VDF_FUNC_ENTRY();

    if(NULL == pstChnState)
    {
        LOG_ERROR("Input parameter is NULL\n");
        return -1;
    }

    if(0 != _MI_VDF_CheckChnValid(VdfChn))
    {
        LOG_ERROR("The input VdfChn(%d) is invalid\n", VdfChn);

        MI_VDF_FUNC_EXIT();
    }

    MI_VDF_FUNC_EXIT();

    return ret;
}

// TODO:fix me!
#if 0
MI_S32 _MI_VDF_CheckStrideMatch(MI_SYS_BufInfo_t *pstInputBufInfo, MI_VDF_ChnAttr_t *pstAttr)
{
    MI_S32 ret = FALSE;
    MI_SYS_FrameBufExtraConfig_t stBufExtraConf;
    MI_U32 tWidth;

    assert(pstAttr);
    assert(pstInputBufInfo);

    MI_SYS_BufInfo_t stInputBufInfo;

    if (E_MI_VDF_WORK_MODE_MD == pstAttr->enWorkMode)
    {
        switch (pstAttr->unAttr.stMdAttr.stMdStaticParamsIn.mb_size)
        {
            case MDMB_MODE_MB_8x8:
            stBufExtraConf.u16BufHAlignment = 8 * 16;   // width
            stBufExtraConf.u16BufVAlignment = 8;        // height
            break;

            case MDMB_MODE_MB_16x16:
            stBufExtraConf.u16BufHAlignment = 16 * 16;   // width
            stBufExtraConf.u16BufVAlignment = 16;        // height
            break;

            case MDMB_MODE_MB_4x4:
            default:
            stBufExtraConf.u16BufHAlignment = 4 * 16;   // width
            stBufExtraConf.u16BufVAlignment = 4;        // height
            break;
        }
    }

    tWidth = ALIGN_UP(pstInputBufInfo->stFrameData.u16Height, stBufExtraConf.u16BufVAlignment);
    if (pstInputBufInfo.stFrameData.u32Stride[0] == tWidth)
    {
        ret = TRUE;
        LOG_WARN("u32Stride=%d, tWidth=%d, u32BufSize=%d\n", \
            pstInputBufInfo.stFrameData.u32Stride[0], \
            tWidth, \
            pstInputBufInfo.stFrameData.u32BufSize
            );
    }

    return ret;
}
#endif
