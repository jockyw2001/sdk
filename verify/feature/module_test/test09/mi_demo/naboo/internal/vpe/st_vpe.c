#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_vpe.h"
#include "st_vpe.h"
#include "st_common.h"

MI_S32 ST_Vpe_Init(void)
{
    printf("ST Vpe Init!!!\n");
	return MI_SUCCESS;
}

MI_S32 ST_Vpe_Exit(void)
{
    printf("ST Vpe Exit!!!\n");
	return MI_SUCCESS;
}

MI_S32 ST_Vpe_CreateChannel(MI_VPE_CHANNEL VpeChannel, ST_VPE_ChannelInfo_t *pstChannelInfo)
{
    MI_VPE_ChannelAttr_t stChannelVpssAttr;
    MI_SYS_WindowRect_t stCropWin;
    memset(&stChannelVpssAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
    memset(&stCropWin, 0, sizeof(MI_SYS_WindowRect_t));
    stChannelVpssAttr.u16MaxW = pstChannelInfo->u16VpeMaxW;
    stChannelVpssAttr.u16MaxH = pstChannelInfo->u16VpeMaxH;
    stChannelVpssAttr.bNrEn= FALSE;
    stChannelVpssAttr.bEdgeEn= FALSE;
    stChannelVpssAttr.bEsEn= FALSE;
    stChannelVpssAttr.bContrastEn= FALSE;
    stChannelVpssAttr.bUvInvert= FALSE;
    stChannelVpssAttr.ePixFmt = pstChannelInfo->eFormat;
    stChannelVpssAttr.eRunningMode = pstChannelInfo->eRunningMode;
    stChannelVpssAttr.eHDRType = pstChannelInfo->eHDRType;
    ExecFunc(MI_VPE_CreateChannel(VpeChannel, &stChannelVpssAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelAttr(VpeChannel, &stChannelVpssAttr), MI_VPE_OK);

    stChannelVpssAttr.bContrastEn = TRUE;
    stChannelVpssAttr.bNrEn = TRUE;
    ExecFunc(MI_VPE_SetChannelAttr(VpeChannel, &stChannelVpssAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
    stCropWin.u16X = pstChannelInfo->u32X;
    stCropWin.u16Y = pstChannelInfo->u32Y;
    stCropWin.u16Width = pstChannelInfo->u16VpeCropW;
    stCropWin.u16Height = pstChannelInfo->u16VpeCropH; //output size???

    ExecFunc(MI_VPE_SetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
#if 0
    MI_VPE_ChannelPara_t stVpeParam; //default 3dnr Para
    stVpeParam.u8NrcSfStr = 80;
    stVpeParam.u8NrySfStr = 80;
    stVpeParam.u8NrcTfStr = 60;
    stVpeParam.u8NryTfStr = 60;
    stVpeParam.u8NryBlendMotionTh = 4;
    stVpeParam.u8NryBlendMotionWei = 16;
    stVpeParam.u8NryBlendOtherWei = 4;
    stVpeParam.u8NryBlendStillTh = 8;
    stVpeParam.u8NryBlendStillWei = 0;
    stVpeParam.u8Contrast = 128;
    stVpeParam.u8EdgeGain[0] = 0;
    stVpeParam.u8EdgeGain[1] = 20;
    stVpeParam.u8EdgeGain[2] = 80;
    stVpeParam.u8EdgeGain[3] = 120;
    stVpeParam.u8EdgeGain[4] = 160;
    stVpeParam.u8EdgeGain[5] = 160;

    ExecFunc(MI_VPE_SetChannelParam(VpeChannel, &stVpeParam), MI_VPE_OK);
#endif
    return MI_SUCCESS;
}
MI_VPE_RunningMode_e ST_Vpe_GetRunModeByWorkMode(VIF_AD_WORK_MODE_E e_WorkMode)
{
    switch (e_WorkMode)
    {
        case SAMPLE_VI_MODE_1_D1:
        case SAMPLE_VI_MODE_16_D1:
        case SAMPLE_VI_MODE_16_960H:
        case SAMPLE_VI_MODE_2_720P:
        case SAMPLE_VI_MODE_1_1080P:
        case SAMPLE_VI_MODE_8_D1:
        case SAMPLE_VI_MODE_1_720P:
        case SAMPLE_VI_MODE_16_Cif:
        case SAMPLE_VI_MODE_16_2Cif:
        case SAMPLE_VI_MODE_16_D1Cif:
        case SAMPLE_VI_MODE_1_D1Cif:
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_8_2Cif:
            return E_MI_VPE_RUNNING_MODE_FRAMEBUF_DVR_MODE;
        case SAMPLE_VI_MODE_MIPI_1_1080P_VPE:
        case SAMPLE_VI_MODE_MIPI_1_1080P_VENC:
            return E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
        default:
            ST_ERR("not support this work mode, mode:%d\n", e_WorkMode);
            return E_MI_VPE_RUNNING_MODE_INVALID;
    }
}
MI_S32 ST_Vpe_StartChannel(MI_VPE_CHANNEL VpeChannel)
{
    ExecFunc(MI_VPE_StartChannel (VpeChannel), MI_VPE_OK);

    return MI_SUCCESS;
}

MI_S32 ST_Vpe_StopChannel(MI_VPE_CHANNEL VpeChannel)
{
    ExecFunc(MI_VPE_StopChannel(VpeChannel), MI_VPE_OK);

    return MI_SUCCESS;
}

MI_S32 ST_Vpe_DestroyChannel(MI_VPE_CHANNEL VpeChannel)
{
    ExecFunc(MI_VPE_DestroyChannel(VpeChannel), MI_VPE_OK);

    return MI_SUCCESS;
}

MI_S32 ST_Vpe_CreatePort(MI_VPE_PORT VpePort, ST_VPE_PortInfo_t *pstPortInfo)
{
    MI_VPE_PortMode_t stVpeMode;
    MI_SYS_ChnPort_t stChnPort;

    memset(&stVpeMode, 0, sizeof(stVpeMode));
    ExecFunc(MI_VPE_GetPortMode(pstPortInfo->DepVpeChannel, VpePort, &stVpeMode), MI_VPE_OK);
    stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stVpeMode.ePixelFormat = pstPortInfo->ePixelFormat;
    stVpeMode.u16Width = pstPortInfo->u16OutputWidth;
    stVpeMode.u16Height= pstPortInfo->u16OutputHeight;
    ExecFunc(MI_VPE_SetPortMode(pstPortInfo->DepVpeChannel, VpePort, &stVpeMode), MI_VPE_OK);

    stChnPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = pstPortInfo->DepVpeChannel;
    stChnPort.u32PortId = VpePort;
    ExecFunc(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 5), 0);

    ExecFunc(MI_VPE_EnablePort(pstPortInfo->DepVpeChannel, VpePort), MI_VPE_OK);

    return MI_SUCCESS;
}

MI_S32 ST_Vpe_StopPort(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort)
{
    ExecFunc(MI_VPE_DisablePort(VpeChannel, VpePort), MI_VPE_OK);

    return MI_SUCCESS;
}