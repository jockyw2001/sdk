#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include "drv_vif.h"

#include <MsTypes.h>
#include "mhal_vif.h"

/* new API */
MS_S32 MHal_VIF_Init(void)
{
    return DrvVif_Init();
}
EXPORT_SYMBOL(MHal_VIF_Init);

MS_S32 MHal_VIF_Deinit(void)
{
    return DrvVif_Deinit();
}
EXPORT_SYMBOL(MHal_VIF_Deinit);

MS_S32 MHal_VIF_Reset(void)
{
    return DrvVif_Reset();
}
EXPORT_SYMBOL(MHal_VIF_Reset);

MS_S32 MHal_VIF_DevSetConfig(MHal_VIF_DEV u32VifDev, MHal_VIF_DevCfg_t *pstDevAttr, MHalAllocPhyMem_t *pstAlloc)
{
    return DrvVif_DevSetConfig(u32VifDev, pstDevAttr, pstAlloc);
}
EXPORT_SYMBOL(MHal_VIF_DevSetConfig);

MS_S32 MHal_VIF_DevEnable(MHal_VIF_DEV u32VifDev)
{
    return DrvVif_DevEnable(u32VifDev);
}
EXPORT_SYMBOL(MHal_VIF_DevEnable);

MS_S32 MHal_VIF_DevDisable(MHal_VIF_DEV u32VifDev)
{
    return DrvVif_DevDisable(u32VifDev);
}
EXPORT_SYMBOL(MHal_VIF_DevDisable);

MS_S32 MHal_VIF_ChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnCfg_t *pstAttr)
{
    return DrvVif_ChnSetConfig(u32VifChn, pstAttr);
}
EXPORT_SYMBOL(MHal_VIF_ChnSetConfig);

MS_S32 MHal_VIF_ChnEnable(MHal_VIF_CHN u32VifChn)
{
    return DrvVif_ChnEnable(u32VifChn);
}
EXPORT_SYMBOL(MHal_VIF_ChnEnable);

MS_S32 MHal_VIF_ChnDisable(MHal_VIF_CHN u32VifChn)
{
    return DrvVif_ChnDisable(u32VifChn);
}
EXPORT_SYMBOL(MHal_VIF_ChnDisable);

MS_S32 MHal_VIF_ChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{
    return DrvVif_ChnQuery(u32VifChn, pstStat);
}
EXPORT_SYMBOL(MHal_VIF_ChnQuery);

MS_S32 MHal_VIF_SubChnSetConfig(MHal_VIF_CHN u32VifChn, MHal_VIF_SubChnCfg_t *pstAttr)
{
    return DrvVif_SubChnSetConfig(u32VifChn, pstAttr);
}
EXPORT_SYMBOL(MHal_VIF_SubChnSetConfig);

MS_S32 MHal_VIF_SubChnEnable(MHal_VIF_CHN u32VifChn)
{
    return DrvVif_SubChnEnable(u32VifChn);
}
EXPORT_SYMBOL(MHal_VIF_SubChnEnable);

MS_S32 MHal_VIF_SubChnDisable(MHal_VIF_CHN u32VifChn)
{
    return DrvVif_SubChnDisable(u32VifChn);
}
EXPORT_SYMBOL(MHal_VIF_SubChnDisable);

MS_S32 MHal_VIF_SubChnQuery(MHal_VIF_CHN u32VifChn, MHal_VIF_ChnStat_t *pstStat)
{
    return DrvVif_SubChnQuery(u32VifChn, pstStat);
}
EXPORT_SYMBOL(MHal_VIF_SubChnQuery);

MS_S32 MHal_VIF_QueueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, const MHal_VIF_RingBufElm_t *ptFbInfo)
{
    return DrvVif_QueueFrameBuffer(u32VifChn, u32ChnPort, ptFbInfo);
}
EXPORT_SYMBOL(MHal_VIF_QueueFrameBuffer);

MS_S32 MHal_VIF_QueryFrames(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MS_U32 *pNumBuf)
{
    return DrvVif_QueryFrames(u32VifChn, u32ChnPort, pNumBuf);
}
EXPORT_SYMBOL(MHal_VIF_QueryFrames);

MS_S32 MHal_VIF_DequeueFrameBuffer(MHal_VIF_CHN u32VifChn, MHal_VIF_PORT u32ChnPort, MHal_VIF_RingBufElm_t *ptFbInfo)
{
    return DrvVif_DequeueFrameBuffer(u32VifChn, u32ChnPort, ptFbInfo);
}
EXPORT_SYMBOL(MHal_VIF_DequeueFrameBuffer);

MS_S32 MHal_VIF_StartMCU()
{

	int rc = 0;

	rc = DrvVif_stopMCU();
	if (rc) {
		pr_err("Init stop MCU fail\n");
	}

	rc = DrvVif_startMCU();
	if (rc) {
		pr_err("Start MCU fail\n");
	}

	rc = DrvVif_pollingMCU();
	if (rc) {
		pr_err("Polling MCU fail\n");
	}

    return rc;
}
EXPORT_SYMBOL(MHal_VIF_StartMCU);

MS_S32 MHal_VIF_StopMCU()
{
    return DrvVif_stopMCU();
}
EXPORT_SYMBOL(MHal_VIF_StopMCU);

MS_S32 MHal_VIF_RegisterEventCallback(MHal_VIF_CHN u32VifChn, MHal_VIF_EventType_e eEventType, MHal_VIF_EventCallback_fp CallBack, u32 Param, void *pPrivateData)
{
    MS_S32 ret = MHAL_SUCCESS;
    switch (eEventType) {
        case E_VIF_EVENT_FRAME_START:
            DrvVif_RegisterFrameStartCb(u32VifChn, (void *)CallBack, Param, pPrivateData);
            break;
        case E_VIF_EVENT_LINE_COUNT_HIT0:
            DrvVif_RegisterLineCntCb(u32VifChn, (void *)CallBack, Param, pPrivateData);
            break;
        case E_VIF_EVENT_FRAME_END:
            DrvVif_RegisterFrameEndCb(u32VifChn, (void *)CallBack, Param, pPrivateData);
            break;
        default:
            ret = MHAL_FAILURE;
            break;
    }

    return ret;
}
