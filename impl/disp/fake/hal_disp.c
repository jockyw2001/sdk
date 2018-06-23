#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include "mhal_common.h"
#define __MHAL_DISP_C__

#include "mhal_disp.h"

#include "mi_sys.h"
#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif


struct halPort_t;
struct halLayer_s;
struct halDevice_s;

typedef struct halDevice_s{
    struct semaphore  stMutex;
    struct list_head layerList;
    u32 u32DeviceId;
    const MHAL_DISP_AllocPhyMem_t *pstAlloc;
    void *pDevBinder;
} halDevice_t;

typedef struct halLayer_s{
    struct list_head layerSelf;
    struct semaphore stMutex;
    struct list_head portList;
    u32 u32LayerId;
    const MHAL_DISP_AllocPhyMem_t *pstAlloc;
    MS_BOOL  bEnable;

    halDevice_t *pDev;
    u32 u32PortNum;
} halLayer_t;

typedef struct halPort_s{
    struct list_head portSelf;
    u32 u32PortId;
    const MHAL_DISP_AllocPhyMem_t *pstAlloc;
    halLayer_t *pLayer;
} halPort_t;



typedef struct {
    struct list_head devList;
    u32 u32DevNum;
    u32 u32LayerNum;
} halDispManager_t;

#define HAL_DISP_DEV_MAX   (2)
#define HAL_DISP_LAYER_MAX (2)
#define HAL_DISP_PORT_MAX  (32)

halDispManager_t _gMHAL_DISP_Mgt = {
    .u32DevNum   = 0,
    .u32LayerNum = 0,
};

#define testDev(pDev) do {\
    DBG_ERR("%s()@line%d pDev: %p u32DeviceId: %d.\n", __func__, __LINE__, (pDev), (pDev)->u32DeviceId);\
} while(0)

#define testLayer(pLayer) do {\
        halDevice_t *pDev = (pLayer)->pDev;\
        if (pDev != NULL) \
        {\
            DBG_ERR("%s()@line%d pLayer: %p u32LayerId: %d bindder to pDev: %p u32DeviceId: %d.\n",__func__, __LINE__,  (pLayer), (pLayer)->u32LayerId, (pDev), (pDev)->u32DeviceId);\
        }\
        else \
        {\
           DBG_ERR("%s()@line%d pLayer: %p u32LayerId: %d .\n",__func__, __LINE__,  (pLayer), (pLayer)->u32LayerId);\
        }\
    } while(0)

#define testPort(pPort) do {\
            halLayer_t *pLayer = (pPort)->pLayer;\
            halDevice_t *pDev = (pLayer)->pDev;\
            if ((pDev) != NULL)\
            {\
                DBG_ERR("%s()@line%d pPort: %p, u32PortId: %d pLayer: %p u32LayerId: %d bindder to pDev: %p u32DeviceId: %d.\n", __func__, __LINE__,  (pPort), (pPort)->u32PortId, (pLayer), (pLayer)->u32LayerId, (pDev), (pDev)->u32DeviceId);\
            }\
            else\
            {\
                DBG_ERR("%s()@line%d pPort: %p, u32PortId: %d pLayer: %p u32LayerId: %d .\n", __func__, __LINE__,  (pPort), (pPort)->u32PortId, (pLayer), (pLayer)->u32LayerId);\
            }\
        } while(0)
#define DBG_ERR(fmt, args...) printk(fmt, ##args)
#define DBG_INFO(fmt, args...) printk(fmt, ##args)

MS_BOOL MHAL_DISP_DeviceCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, const u32 u32DeviceId, void **pDevCtx)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = NULL;

    if ((NULL == pstAlloc) || ( NULL == pDevCtx))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    if (_gMHAL_DISP_Mgt.u32DevNum < HAL_DISP_DEV_MAX)
    {
        pDev = kmalloc(sizeof(*pDev), GFP_KERNEL);
        if (pDev == NULL)
        {

            DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
            return bRet;
        }

        memset(pDev, 0, sizeof(*pDev));
        pDev->u32DeviceId = u32DeviceId;
        pDev->pstAlloc    = pstAlloc;
        pDev->pDevBinder  = NULL;
        INIT_LIST_HEAD(&pDev->layerList);
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        init_MUTEX(&pDev->stMutex);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        sema_init(&pDev->stMutex, 1);
#endif
        *pDevCtx = pDev;
        _gMHAL_DISP_Mgt.u32DevNum++;
        DBG_INFO("%s()@line%d sucess pstAlloc: %p u32DeviceId: %d, *pDevCtx: %p !!!\n", __func__, __LINE__, pstAlloc, u32DeviceId, *pDevCtx);
        bRet = TRUE;
    }
    else
    {
        DBG_ERR("%s()@line%d already max device Failed!!!\n", __func__, __LINE__);
        bRet = FALSE;
    }

    return bRet;
}

MS_BOOL MHAL_DISP_DeviceDestroyInstance(void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if (NULL == pDevCtx )
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    testDev(pDev);
    DBG_INFO("%s()@line%d sucess pDevCtx: %p !!!\n", __func__, __LINE__, pDevCtx);
    kfree(pDevCtx);
    _gMHAL_DISP_Mgt.u32DevNum--;

    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceEnable(void *pDevCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if (NULL == pDevCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d pDevCtx: %p bEnable: %d sucess!!!\n", __func__, __LINE__, pDevCtx, bEnable);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceAttach(const void *pSrcDevCtx, const void *pDstDevCtx)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pSrcDev = (halDevice_t *)pSrcDevCtx;
    halDevice_t *pDstDev = (halDevice_t *)pDstDevCtx;

    if ((NULL == pSrcDevCtx) || (NULL == pDstDevCtx))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pSrcDev);
    testDev(pDstDev);

    DBG_INFO("%s()@line%d sucess pSrcDevCtx: %p, pDstDevCtx: %p !!!\n", __func__, __LINE__, pSrcDevCtx, pDstDevCtx);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_DeviceDetach(const void *pSrcDevCtx, const void *pDstDevCtx)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pSrcDev = (halDevice_t *)pSrcDevCtx;
    halDevice_t *pDstDev = (halDevice_t *)pDstDevCtx;

    if ((NULL == pSrcDevCtx) || (NULL == pDstDevCtx ))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pSrcDev);
    testDev(pDstDev);

    DBG_INFO("%s()@line%d sucess pSrcDevCtx: %p pDstDevCtx: %p !!!\n", __func__, __LINE__, pSrcDevCtx, pDstDevCtx);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_DeviceSetBackGroundColor(void *pDevCtx, const u32 u32BgColor)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if (NULL == pDevCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess pDevCtx:%p u32BgColor: 0x%x !!!\n", __func__, __LINE__, pDevCtx, u32BgColor);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_DeviceAddOutInterace(void *pDevCtx, const u32 u32Interface)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if (NULL == pDevCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess: pDevCtx: %p u32Interface: 0x%x!!!\n", __func__, __LINE__, pDevCtx, u32Interface);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceSetOutputTiming(void *pDevCtx, const MS_U32 u32Interface, const MHAL_DISP_DeviceTimingInfo_t *pstTimingInfo)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pDevCtx) || (NULL == pstTimingInfo))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess pDevCtx: %p pstTimingInfo: %p !!!\n", __func__, __LINE__, pDevCtx, pstTimingInfo);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceSetVgaParam(void *pDevCtx, const MHAL_DISP_VgaParam_t *pstVgaInfo)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pDevCtx) || (pstVgaInfo == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess pDevCtx: %p pstVgaInfo: %p !!!\n", __func__, __LINE__, pDevCtx, pstVgaInfo);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceSetHdmiParam(void *pDevCtx, const MHAL_DISP_HdmiParam_t *pstHdmiInfo)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pDevCtx) || (pstHdmiInfo  == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess pDevCtx: %p pstHdmiInfo: %p  !!!\n", __func__, __LINE__, pDevCtx, pstHdmiInfo);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_DeviceSetCvbsParam(void *pDevCtx, const MHAL_DISP_CvbsParam_t *pstCvbsInfo)
{
    MS_BOOL bRet = FALSE;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pDevCtx) || (pstCvbsInfo  == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);

    DBG_INFO("%s()@line%d sucess pDevCtx: %p pstCvbsInfo: %p !!!\n", __func__, __LINE__, pDevCtx, pstCvbsInfo);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_VideoLayerCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, const u32 u32LayerId, void **pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t *pLayer = NULL;


    if ((NULL == pstAlloc) || (NULL == pVidLayerCtx))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    if (_gMHAL_DISP_Mgt.u32LayerNum < HAL_DISP_LAYER_MAX)
    {
        pLayer = kmalloc(sizeof(*pLayer), GFP_KERNEL);
        if (pLayer == NULL)
        {

            DBG_ERR("%s()@line%d alloc Failed!!!\n", __func__, __LINE__);
            return bRet;
        }

        memset(pLayer, 0, sizeof(*pLayer));
        pLayer->u32LayerId  = u32LayerId;
        pLayer->pstAlloc    = pstAlloc;
        pLayer->pDev        = NULL;
        *pVidLayerCtx = pLayer;
        _gMHAL_DISP_Mgt.u32LayerNum++;
        INIT_LIST_HEAD(&pLayer->portList);
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
        init_MUTEX(&pLayer->stMutex);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
        sema_init(&pLayer->stMutex, 1);
#endif
        *pVidLayerCtx = pLayer;

        //test
        //testLayer(pLayer);
        DBG_INFO("%s()@line%d sucess pstAlloc: %p u32LayerId: %d pVidLayerCtx: %p !!!\n", __func__, __LINE__, pstAlloc, u32LayerId, pVidLayerCtx);
        bRet = TRUE;
    }
    else
    {
        DBG_ERR("%s()@line%d already max layer Failed !!!\n", __func__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}
MS_BOOL MHAL_DISP_VideoLayerDestoryInstance(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess!!!\n", __func__, __LINE__);
    kfree(pVidLayerCtx);
    _gMHAL_DISP_Mgt.u32LayerNum--;
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_VideoLayerEnable(void *pVidLayerCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);
    pLayer->bEnable = bEnable;
    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p bEnbale: %d!!!\n", __func__, __LINE__, pVidLayerCtx, bEnable);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_VideoLayerBind(void *pVidLayerCtx, void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pVidLayerCtx) || (NULL == pDevCtx))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);
    testLayer(pLayer);
    pLayer->pDev = pDev;
    down(&pDev->stMutex);
    list_add(&pLayer->layerSelf, &pDev->layerList);
    up(&pDev->stMutex);
    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p, pDevCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx, pDevCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_VideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;
    halDevice_t *pDev = (halDevice_t *)pDevCtx;

    if ((NULL == pVidLayerCtx) || (pDevCtx == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testDev(pDev);
    testLayer(pLayer);
    down(&pDev->stMutex);
    list_del(&pLayer->layerSelf);
    up(&pDev->stMutex);
    pLayer->pDev = NULL;
    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p, pDevCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx, pDevCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_VideoLayerSetAttr(void *pVidLayerCtx, const MHAL_DISP_VideoLayerAttr_t *pstAttr)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if ((NULL == pVidLayerCtx) || (pstAttr == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p pstAttr: %p !!!\n", __func__, __LINE__, pVidLayerCtx, pstAttr);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_VideoLayerSetCompress(void *pVidLayerCtx, const MHAL_DISP_CompressAttr_t* pstCompressAttr)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if ((NULL == pVidLayerCtx) || (NULL == pstCompressAttr))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p pstCompressAttr: %p !!!\n", __func__, __LINE__, pVidLayerCtx, pstCompressAttr);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_VideoLayerSetPriority(void *pVidLayerCtx, const u32 u32Priority)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_VideoLayerBufferFire(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_VideoLayerCheckBufferFired(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx);
    bRet = TRUE;
    return bRet;
}
//MS_BOOL MHAL_DISP_InputPortCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, void *pVidLayerCtx, void **pCtx)
MS_BOOL MHAL_DISP_InputPortCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, void *pVidLayerCtx, const MS_U32 u32PortId, void **pCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;
    halPort_t *pPort = NULL;

    if ((NULL == pstAlloc) || (NULL == pVidLayerCtx) || (NULL == pCtx))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    if (pLayer->u32PortNum < HAL_DISP_PORT_MAX)
    {
        pPort = kmalloc(sizeof(*pPort), GFP_KERNEL);
        if (pPort == NULL)
        {

            DBG_ERR("%s()@line%d alloc Failed!!!\n", __func__, __LINE__);
            return bRet;
        }

        memset(pPort, 0, sizeof(*pPort));
        pPort->u32PortId  = pLayer->u32PortNum++;
        pPort->pstAlloc    = pstAlloc;
        pPort->pLayer      = pVidLayerCtx;
        *pCtx = pPort;

        DBG_INFO("%s()@line%d sucess pstAlloc: %p , pVidLayerCtx: %p, *pCtx: %p !!!\n", __func__, __LINE__, pstAlloc, pVidLayerCtx, *pCtx);
        bRet = TRUE;
    }
    else
    {
        DBG_ERR("%s()@line%d alloc failed !!!\n", __func__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}
MS_BOOL MHAL_DISP_InputPortDestroyInstance(const void *pCtx)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pCtx;
    halLayer_t  *pLayer = NULL;

    if (NULL == pCtx )
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }
    pLayer = pPort->pLayer;
    pLayer->u32PortNum--;

    //test
    testPort(pPort);
    DBG_INFO("%s()@line%d sucess pCtx: %p !!!\n", __func__, __LINE__, pCtx);
    kfree(pCtx);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_InputPortEnable(void *pInputPortCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pInputPortCtx;

    if (NULL == pInputPortCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testPort(pPort);

    DBG_INFO("%s()@line%d sucess pInputPortCtx: %p !!!\n", __func__, __LINE__, pInputPortCtx);
    bRet = TRUE;
    return bRet;
}
MS_BOOL MHAL_DISP_InputPortSetAttr(void *pInputPortCtx, const MHAL_DISP_InputPortAttr_t *pstAttr)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pInputPortCtx;

    if ((NULL == pInputPortCtx) || (NULL == pstAttr))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testPort(pPort);

    DBG_INFO("%s()@line%d sucess pInputPortCtx: %p pstAttr: %p !!!\n", __func__, __LINE__, pInputPortCtx, pstAttr);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_InputPortShow(void *pInputPortCtx)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pInputPortCtx;

    if (NULL == pInputPortCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testPort(pPort);

    DBG_INFO("%s()@line%d sucess: pInputPortCtx %p!!!\n", __func__, __LINE__, pInputPortCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_InputPortHide(void *pInputPortCtx)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pInputPortCtx;

    if (NULL == pInputPortCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testPort(pPort);

    DBG_INFO("%s()@line%d sucess pInputPortCtx: %p !!!\n", __func__, __LINE__, pInputPortCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_InputPortAttrBegin(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_InputPortAttrEnd(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    halLayer_t  *pLayer = (halLayer_t *)pVidLayerCtx;

    if (NULL == pVidLayerCtx )
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testLayer(pLayer);

    DBG_INFO("%s()@line%d sucess pVidLayerCtx: %p !!!\n", __func__, __LINE__, pVidLayerCtx);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_InputPortFlip(void *pInputPortCtx, MHAL_DISP_VideoFrameData_t *pstVideoFrameBuffer)
{
    MS_BOOL bRet = FALSE;
    halPort_t *pPort = (halPort_t*)pInputPortCtx;

    if ((NULL == pInputPortCtx) || (pstVideoFrameBuffer == NULL))
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    //test
    testPort(pPort);

    DBG_INFO("%s()@line%d sucess pInputPortCtx: %p pstVideoFrameBuffer: %p !!!\n", __func__, __LINE__, pInputPortCtx, pstVideoFrameBuffer);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DbgLevel(void *p)
{
    MS_BOOL bRet = FALSE;

    if (NULL == p)
    {
        DBG_ERR("%s()@line%d pointer Failed!!!\n", __func__, __LINE__);
        return FALSE;

    }

    DBG_INFO("%s()@line%d sucess p: %p dbg level -> %d!!!\n", __func__, __LINE__, p, *(u32*)p);
    bRet = TRUE;
    return bRet;
}

MS_BOOL MHAL_DISP_DeviceAddOutInterface(void *pDevCtx, const MS_U32 u32Interface)
{
    return TRUE;
}

MS_BOOL MHAL_DISP_ClearDevInterrupt(void *pDevCtx, void* pData)
{
    return TRUE;

}

MS_BOOL MHAL_DISP_InitMmapConfig(MHAL_DISP_MmapType_e eMmType, MHAL_DISP_MmapInfo_t* pstMmapInfo)
{
    return TRUE;
}
MS_BOOL MHAL_DISP_GetDevIrq(void *pDevCtx, MS_U32* pu32DevIrq)
{
    MS_BOOL bRet = TRUE;
    *pu32DevIrq = 100;
    return bRet;
}

MI_S32 mi_divp_CaptureTiming(mi_divp_CaptureInfo_t* pstCapInfo)
{
    pstCapInfo = NULL;
    return 0;
}

MS_BOOL MHAL_DISP_InitPanelConfig(MHAL_DISP_PanelConfig_t* pstPanelConfig, MS_U8 u8Size)
{
    return FALSE;
}
