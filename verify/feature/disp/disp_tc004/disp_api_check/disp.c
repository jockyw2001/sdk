#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mi_disp.h"

#define ExecFunc(func, _ret_) \
    printf("%d Start test: %s\n", __LINE__, #func);\
    if (func != _ret_)\
    {\
        printf("DISP_TEST [%d] %s exec function failed\n",__LINE__, #func);\
        return 1;\
    }\
    else\
    {\
        printf("DISP_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);

#define  TEST_VGA (FALSE)
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_U32 i = 0;
    MI_U32 u32Delay    = 60;
    MI_BOOL bLayerTest = FALSE;
    MI_BOOL bPortTest  = FALSE;

    if (argc > 1)
    {
        bLayerTest = TRUE;
        printf("%s %s.\n", argv[0], argv[1]);
    }

    if (argc > 2)
    {
        bPortTest  = TRUE;
        printf("%s %s %s.\n", argv[0], argv[1], argv[2]);
    }

    // Test Device
    MI_DISP_DEV DispDev = 0;

    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VgaParam_t stVgaParam;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_CvbsParam_t stCvbsParam;

    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P30;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
#if TEST_VGA
    memset(&stVgaParam, 0, sizeof(stVgaParam));
    ExecFunc(MI_DISP_GetVgaParam(DispDev, &stVgaParam), MI_DISP_SUCCESS);
    stVgaParam.u32Gain = 100;
    ExecFunc(MI_DISP_SetVgaParam(DispDev, &stVgaParam), MI_DISP_SUCCESS);
#endif
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    ExecFunc(MI_DISP_GetHdmiParam(DispDev, &stHdmiParam), MI_DISP_SUCCESS);
    stHdmiParam.stCsc.u32Hue = 0x12;
    stHdmiParam.stCsc.u32Luma= 0x34;
    stHdmiParam.stCsc.u32Saturation = 0x56;
    ExecFunc(MI_DISP_SetHdmiParam(DispDev, &stHdmiParam), MI_DISP_SUCCESS);

    memset(&stCvbsParam, 0, sizeof(stCvbsParam));
    ExecFunc(MI_DISP_GetCvbsParam(DispDev, &stCvbsParam), MI_DISP_SUCCESS);
    stCvbsParam.bEnable = FALSE;
    ExecFunc(MI_DISP_SetCvbsParam(DispDev, &stCvbsParam), MI_DISP_SUCCESS);

    stCvbsParam.bEnable = TRUE;
    ExecFunc(MI_DISP_SetCvbsParam(DispDev, &stCvbsParam), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

#if 0
MI_S32 MI_DISP_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
MI_S32 MI_DISP_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev);
#endif
    MI_DISP_LAYER DispLayer = 0;


    if (bLayerTest)
    {
        MI_U32 u32Toleration = 0x87654321;
        MI_DISP_CompressAttr_t stCompressAttr;
        MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
        memset(&stLayerAttr, 0, sizeof(stLayerAttr));

        stLayerAttr.stVidLayerSize.u16Width = 1280;
        stLayerAttr.stVidLayerSize.u16Height= 720;
        stLayerAttr.stVidLayerDispWin.u16X = 0;
        stLayerAttr.stVidLayerDispWin.u16Y = 0;
        stLayerAttr.stVidLayerDispWin.u16Width  = 1920;
        stLayerAttr.stVidLayerDispWin.u16Height = 1080;
        ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);


        memset(&stCompressAttr, 0, sizeof(stCompressAttr));
      #if 0
	  ExecFunc(MI_DISP_GetVideoLayerCompressAttr(DispLayer, &stCompressAttr), MI_DISP_SUCCESS);
        stCompressAttr.bSupportCompress = TRUE;
        ExecFunc(MI_DISP_SetVideoLayerCompressAttr(DispLayer, &stCompressAttr), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetVideoLayerCompressAttr(DispLayer, &stCompressAttr), MI_DISP_SUCCESS);

        stCompressAttr.bSupportCompress = FALSE;
        ExecFunc(MI_DISP_SetVideoLayerCompressAttr(DispLayer, &stCompressAttr), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetVideoLayerCompressAttr(DispLayer, &stCompressAttr), MI_DISP_SUCCESS);
#endif

        ExecFunc(MI_DISP_SetPlayToleration(DispLayer, u32Toleration), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetPlayToleration(DispLayer, &u32Toleration), MI_DISP_SUCCESS);



        ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);
      #if 0
	  MI_U32 u32Priority = 0;
        ExecFunc(MI_DISP_GetVideoLayerPriority(DispLayer, &u32Priority), MI_DISP_SUCCESS);
        u32Priority += 2;
        ExecFunc(MI_DISP_SetVideoLayerPriority(DispLayer, u32Priority), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetVideoLayerPriority(DispLayer, &u32Priority), MI_DISP_SUCCESS);
#endif


        // Start change VideoLayerSize
        memset(&stLayerAttrBackup, 0, sizeof(stLayerAttrBackup));
        ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttrBackup), MI_DISP_SUCCESS);
        u32Delay = 3;
        while (u32Delay--)
        {
            stLayerAttr.stVidLayerSize.u16Width -= u32Delay*30;
            stLayerAttr.stVidLayerSize.u16Height-= u32Delay*30;
            stLayerAttr.stVidLayerDispWin.u16Width  -= u32Delay*80;
            stLayerAttr.stVidLayerDispWin.u16Height -= u32Delay*80;
            ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
            printf("[%d] sleep\n", u32Delay);
            sleep(3);
        }
        ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttrBackup), MI_DISP_SUCCESS);
    }
    MI_DISP_INPUTPORT LayerInputPort = 0;

    if (bPortTest)
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;
        memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
        stInputPortAttr.stDispWin.u16Width  = 1280;
        stInputPortAttr.stDispWin.u16Height = 720;
        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, LayerInputPort, E_MI_DISP_SYNC_MODE_CHECK_PTS), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, LayerInputPort, E_MI_DISP_SYNC_MODE_FREE_RUN), MI_DISP_SUCCESS);

        MI_DISP_QueryChannelStatus_t stStatus;
        memset(&stStatus, 0, sizeof(stStatus));
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_PauseInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_ResumeInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_StepInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_ShowInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_HideInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        ExecFunc(MI_DISP_SetVideoLayerAttrBegin(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_QueryInputPortStat(DispLayer, LayerInputPort, &stStatus), MI_DISP_SUCCESS);

        u32Delay = 3;
        MI_DISP_Position_t stDispPos;
        memset(&stDispPos, 0, sizeof(stDispPos));
        while (u32Delay--)
        {
            ExecFunc(MI_DISP_SetInputPortDispPos(DispLayer, LayerInputPort, &stDispPos), MI_DISP_SUCCESS);
            stDispPos.u16X += u32Delay*20;
            stDispPos.u16Y += u32Delay*20;
            ExecFunc(MI_DISP_SetInputPortDispPos(DispLayer, LayerInputPort, &stDispPos), MI_DISP_SUCCESS);

            sleep(1);
        }

        ExecFunc(MI_DISP_SetVideoLayerAttrEnd(DispLayer), MI_DISP_SUCCESS);
    }

    u32Delay = 6;
    while (u32Delay--)
    {
        sleep(1);
        printf("[%d] sleep\n", u32Delay);
    }

    // End Test port
endi_test_port:
    if (bPortTest)
    {
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_DISP_SUCCESS);
    }

    // End Test Layer
end_test_layer:
    if (bLayerTest)
    {
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    }


    // End Test Device
end_test_device:

    ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    printf("%s()@line %d pass exit\n", __func__, __LINE__);
    return 0;
}
