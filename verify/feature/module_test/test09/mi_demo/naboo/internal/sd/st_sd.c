#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_sd.h"
#include "st_sd.h"
#include "st_common.h"

MI_S32 ST_SD_Init(void)
{
    printf("ST SD Init!!!\n");
	return MI_SUCCESS;
}

MI_S32 ST_SD_Exit(void)
{
    printf("ST SD Exit!!!\n");
	return MI_SUCCESS;
}

MI_S32 ST_SD_CreateChannel(MI_SD_CHANNEL SDChannel, ST_SD_ChannelInfo_t *pstChannelInfo)
{
    MI_SD_ChannelAttr_t stChannelSDAttr;

    memset(&stChannelSDAttr, 0, sizeof(MI_SD_ChannelAttr_t));
    stChannelSDAttr.u16MaxW = pstChannelInfo->u16SDMaxW;
    stChannelSDAttr.u16MaxH = pstChannelInfo->u16SDMaxH;

    stChannelSDAttr.stCropRect.u16X = pstChannelInfo->u32X;
    stChannelSDAttr.stCropRect.u16Y = pstChannelInfo->u32Y;
    stChannelSDAttr.stCropRect.u16Width = pstChannelInfo->u16SDCropW;
    stChannelSDAttr.stCropRect.u16Height = pstChannelInfo->u16SDCropH; //output size???
    ExecFunc(MI_SD_CreateChannel(SDChannel, &stChannelSDAttr), MI_SD_OK);
    ExecFunc(MI_SD_StartChannel (SDChannel), MI_SD_OK);

    return MI_SUCCESS;
}

//MI_S32 ST_SD_StartChannel(MI_SD_CHANNEL SDChannel)
//{
//    ExecFunc(MI_SD_StartChannel (SDChannel), MI_SD_OK);

//    return MI_SUCCESS;
//}

//MI_S32 ST_SD_StopChannel(MI_SD_CHANNEL SDChannel)
//{
//    ExecFunc(MI_SD_StopChannel(SDChannel), MI_SD_OK);

//    return MI_SUCCESS;
//}

MI_S32 ST_SD_DestroyChannel(MI_SD_CHANNEL SDChannel)
{
    ExecFunc(MI_SD_StopChannel(SDChannel), MI_SD_OK);
    ExecFunc(MI_SD_DestroyChannel(SDChannel), MI_SD_OK);
    return MI_SUCCESS;
}

MI_S32 ST_SD_CreatePort(ST_SD_PortInfo_t *pstPortInfo)
{
    MI_SD_OuputPortAttr_t stSDMode;
    MI_SYS_ChnPort_t stChnPort;

    memset(&stSDMode, 0, sizeof(stSDMode));
    ExecFunc(MI_SD_GetOutputPortAttr(pstPortInfo->DepSDChannel, &stSDMode), MI_SD_OK);
    stSDMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stSDMode.ePixelFormat = pstPortInfo->ePixelFormat;
    stSDMode.u16Width = pstPortInfo->u16OutputWidth;
    stSDMode.u16Height= pstPortInfo->u16OutputHeight;
    ExecFunc(MI_SD_SetOutputPortAttr(pstPortInfo->DepSDChannel, &stSDMode), MI_SD_OK);

    stChnPort.eModId = E_MI_MODULE_ID_SD;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = pstPortInfo->DepSDChannel;
    stChnPort.u32PortId = 0;
    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 5), 0);

    return MI_SUCCESS;
}

MI_S32 ST_SD_StopPort(MI_SD_CHANNEL SDChannel, MI_SD_PORT SDPort)
{
    printf("ST SD StopPort!!!\n");

    return MI_SUCCESS;
}
