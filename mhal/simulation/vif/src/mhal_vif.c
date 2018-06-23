//#include <linux/types.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/spinlock_types.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/interrupt.h>
#include "mhal_vif.h"

//#include "../../../../interface/include/common/mi_common.h"
//#include "../../../../interface/include/sys/mi_sys.h"
//#include "../../../../interface/include/vif/mi_vif.h"
//#include "../../../../interface/include/vif/mi_vif_datatype.h"
//#include "../../../../interface/include/gfx/mi_gfx.h"

//#include "mi_vif.h"
//#include "mi_vif_datatype.h"
//#include "mi_vif_impl.h"
//#include "mi_common.h"
//#include "mi_print.h"
//#include "mi_sys_internal.h"

typedef struct mhal_vif_PortContext_s
{
    MS_BOOL bEnable;
    MS_U32 u32HalBufferHold;
    MS_U32 u32VbFail;
    MHal_VIF_ChnCfg_t stChnCfg;
    struct list_head stRingBufferList;
} MHal_VIF_PortContext_t;

typedef struct mhal_vif_ChnContext_s
{
    MHal_VIF_PortContext_t stPortCtx[MHAL_VIF_MAX_CHN_OUTPORT];
} MHal_VIF_ChnContext_t;

typedef struct MHal_VIF_BuffListElm_s
{
    struct list_head list;
    MS_U32 nMiPriv;
} MHal_VIF_BuffListElm_t;

static MHal_VIF_ChnContext_t _astHalVifChnCtx[MHAL_VIF_MAX_PHYCHN_NUM];
static struct kmem_cache *mhal_vif_buffer_cachep;
static spinlock_t _mhal_vif_BuffListLock;


MS_S32 MHal_VIF_Init(void)
{
    MHal_VIF_CHN u32VifChn;
    MHal_VIF_PORT u32ChnPort;

    mhal_vif_buffer_cachep = kmem_cache_create("mhal_vif_buffer_cachep", sizeof(MHal_VIF_BuffListElm_t), 0, SLAB_PANIC, NULL);
    spin_lock_init(&_mhal_vif_BuffListLock);

    for(u32VifChn = 0 ; u32VifChn < MHAL_VIF_MAX_PHYCHN_NUM ; u32VifChn ++)
    {
        for(u32ChnPort = 0 ; u32ChnPort < MHAL_VIF_MAX_CHN_OUTPORT ; u32ChnPort ++)
        {
            INIT_LIST_HEAD(&_astHalVifChnCtx[u32VifChn].stPortCtx[u32ChnPort].stRingBufferList);
        }
    }

    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_Deinit(void)
{
    MHal_VIF_CHN u32VifChn;
    MHal_VIF_PORT u32ChnPort;
    MHal_VIF_BuffListElm_t *pstBufTrackInfo;

    spin_lock(&_mhal_vif_BuffListLock);

    for(u32VifChn = 0 ; u32VifChn < MHAL_VIF_MAX_PHYCHN_NUM ; u32VifChn ++)
    {
        for(u32ChnPort = 0 ; u32ChnPort < MHAL_VIF_MAX_CHN_OUTPORT ; u32ChnPort ++)
        {
            struct list_head* plist = &(_astHalVifChnCtx[u32VifChn].stPortCtx[u32ChnPort].stRingBufferList);

            while(!list_empty(plist))
            {
                pstBufTrackInfo = list_first_entry(plist, MHal_VIF_BuffListElm_t, list);
                list_del_init(&pstBufTrackInfo->list);
                kmem_cache_free(mhal_vif_buffer_cachep, pstBufTrackInfo);
            }
        }
    }

    //printk("%s %d\n",__FUNCTION__,__LINE__);
    spin_unlock(&_mhal_vif_BuffListLock);
    //printk("%s %d\n",__FUNCTION__,__LINE__);
    kmem_cache_destroy(mhal_vif_buffer_cachep);
    //printk("%s %d\n", __FUNCTION__, __LINE__);
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_Reset(void)
{
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_DevSetConfig(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr)
{
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_DevEnable(MHal_VIF_DEV u32VifDev)
{
    // TODO
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_DevDisable(MHal_VIF_DEV u32VifDev)
{
    // TODO
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_ChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[0];
    memcpy(&pstPortCtx->stChnCfg, pstAttr, sizeof(MHal_VIF_ChnCfg_t));
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_ChnEnable(MHal_VIF_CHN u32VifChn)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[0];
    pstPortCtx->bEnable = TRUE;
    INIT_LIST_HEAD(&pstPortCtx->stRingBufferList);
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_ChnDisable(MHal_VIF_CHN u32VifChn)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[0];
    struct list_head* plist = &pstPortCtx->stRingBufferList;
    MHal_VIF_BuffListElm_t *pstBufTrackInfo;

    pstPortCtx->bEnable = FALSE;
    BUG_ON(!list_empty(plist));

#if 0
    spin_lock(&_mhal_vif_BuffListLock);

    while(!list_empty(plist))
    {
        //printk("%s %d\n", __FUNCTION__, __LINE__);
        pstBufTrackInfo = list_first_entry(plist, MHal_VIF_BuffListElm_t, list);
        //printk("%s %d pstBufTrackInfo:%p\n", __FUNCTION__, __LINE__, pstBufTrackInfo);
        list_del_init(&pstBufTrackInfo->list);
        //printk("%s %d\n", __FUNCTION__, __LINE__);
        kmem_cache_free(mhal_vif_buffer_cachep, pstBufTrackInfo);
        //printk("%s %d\n", __FUNCTION__, __LINE__);
    }

    spin_unlock(&_mhal_vif_BuffListLock);
#endif
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_ChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_SubChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_SubChnCfg_t *pstAttr)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[1];
    memcpy(&pstPortCtx->stChnCfg, pstAttr, sizeof(MHal_VIF_SubChnCfg_t));
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_SubChnEnable(MHal_VIF_CHN u32VifChn)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[1];
    pstPortCtx->bEnable = TRUE;
    INIT_LIST_HEAD(&pstPortCtx->stRingBufferList);
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_SubChnDisable(MHal_VIF_CHN u32VifChn)
{
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_SubChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_QueueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, const MHal_VIF_RingBufElm_t *ptFbInfo)
{
    struct list_head* plist = &(_astHalVifChnCtx[u32VifChn].stPortCtx[u32ChnPort].stRingBufferList);
    MHal_VIF_BuffListElm_t* pstBufTrackInfo;

    //printk("u32VifChn=%d u32ChnPort=%d ptFbInfo=%p\n",u32VifChn,u32ChnPort,ptFbInfo);
    spin_lock(&_mhal_vif_BuffListLock);
    pstBufTrackInfo = kmem_cache_zalloc(mhal_vif_buffer_cachep, GFP_KERNEL);
    pstBufTrackInfo->nMiPriv = ptFbInfo->nMiPriv;
    list_add_tail(&pstBufTrackInfo->list, plist);
    spin_unlock(&_mhal_vif_BuffListLock);
    return MHAL_SUCCESS;
}

static MS_S32 mal_vif_fpsLookUp[E_MHAL_VIF_FRAMERATE_MAX] = {23, 27, 29, 30, 25};
// 8/32 = 25%     25fps    1
// 4/32 = 12.5%   12.5fps  0.5
// 2/32 = 6.25%   6.25fps  0.25
// 1/32 = 3.125%  3.125fps 0.125
// 6/32 = 18.75%  3.125fps 0.75

MS_S32 MHal_VIF_QueryFrames(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MS_U32 *pNumBuf)
{
    MHal_VIF_PortContext_t* pstPortCtx = &_astHalVifChnCtx[u32VifChn].stPortCtx[u32ChnPort];
    *pNumBuf = ((get_random_int() & 0x1F) > mal_vif_fpsLookUp[pstPortCtx->stChnCfg.eFrameRate]);
    //printk("%s %d\n", __FUNCTION__, *pNumBuf);
    return MHAL_SUCCESS;
}

MS_S32 MHal_VIF_DequeueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_RingBufElm_t *ptFbInfo)
{
    struct list_head* plist = &_astHalVifChnCtx[u32VifChn].stPortCtx[u32ChnPort].stRingBufferList;
    MHal_VIF_BuffListElm_t *pstBufTrackInfo;

    //printk("%s %d\n", __FUNCTION__, __LINE__);

    spin_lock(&_mhal_vif_BuffListLock);

    if(list_empty(plist))
    {
        //printk(KERN_ERR"mhal queue is empty\n");
        spin_unlock(&_mhal_vif_BuffListLock);
        return MHAL_FAILURE;
    }

    pstBufTrackInfo = list_first_entry(plist, MHal_VIF_BuffListElm_t, list);
    list_del_init(&pstBufTrackInfo->list);
    ptFbInfo->nMiPriv = pstBufTrackInfo->nMiPriv;
    ptFbInfo->nPTS = jiffies;

    kmem_cache_free(mhal_vif_buffer_cachep, pstBufTrackInfo);
    spin_unlock(&_mhal_vif_BuffListLock);

    //printk("%s %d\n", __FUNCTION__, __LINE__);
    return MHAL_SUCCESS;
}
MS_S32 MHal_VIF_StopMCU()
{
    return MHAL_SUCCESS;
}
EXPORT_SYMBOL(MHal_VIF_StopMCU);

EXPORT_SYMBOL(MHal_VIF_Init);
EXPORT_SYMBOL(MHal_VIF_Deinit);
EXPORT_SYMBOL(MHal_VIF_Reset);
EXPORT_SYMBOL(MHal_VIF_DevSetConfig);
EXPORT_SYMBOL(MHal_VIF_DevEnable);
EXPORT_SYMBOL(MHal_VIF_DevDisable);
EXPORT_SYMBOL(MHal_VIF_ChnSetConfig);
EXPORT_SYMBOL(MHal_VIF_ChnEnable);
EXPORT_SYMBOL(MHal_VIF_ChnDisable);
EXPORT_SYMBOL(MHal_VIF_ChnQuery);
EXPORT_SYMBOL(MHal_VIF_SubChnSetConfig);
EXPORT_SYMBOL(MHal_VIF_SubChnEnable);
EXPORT_SYMBOL(MHal_VIF_SubChnDisable);
EXPORT_SYMBOL(MHal_VIF_SubChnQuery);
EXPORT_SYMBOL(MHal_VIF_QueueFrameBuffer);
EXPORT_SYMBOL(MHal_VIF_QueryFrames);
EXPORT_SYMBOL(MHal_VIF_DequeueFrameBuffer);
