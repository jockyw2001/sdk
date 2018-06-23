//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

#define DBG_INFO(fmt, args...)      //({do{printf("\n [MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);usleep(10000);})

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        DBG_INFO("exec function failed\n");\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("exec function pass\n");\
    }

#define MIDISPCHECKRESULT(result)\
    if(result != MI_SUCCESS)\
    {\
        DBG_INFO("exec function failed\n");\
        return 1;\
    }\
    else\
    {\
        DBG_INFO("exec function pass\n");\
    }

#define MAXPORTNUM 16

static MI_BOOL _gbStop = FALSE;

enum{
    //HDMI and VGA CSC
    E_DISP_TEST_SET_NULL =0,
    E_DISP_TEST_SET_PAUSE,
    E_DISP_TEST_SET_STEP,
    E_DISP_TEST_SET_RESUME,
    E_DISP_TEST_SET_HIDE,
    E_DISP_TEST_SET_SHOW,

    E_DISP_TEST_SET_SHOW_PARA,
    E_DISP_TEST_SET_RET,

    E_DISP_TEST_SET_MAX,
} test_disp_TestStatusOp_e;

enum {
    E_DISP_TEST_SET_RET_PASS,
    E_DISP_TEST_SET_RET_FAIL,
    E_DISP_TEST_SET_RET_EXIT,
} test_vpe_TestStatusRet_e;

typedef struct test_disp_TestPortStatus_s
{
    int PortStatus;
    pthread_t thread;
    pthread_attr_t attr;
    MI_U8 u8PortNum;
    MI_U8 u8PortId;
    MI_U16 u16Width;
    MI_U16 u16Height;
    int srcFd;
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_U32 u32SendStremTime;
    MI_DISP_LAYER DispLayer;
    MI_DISP_INPUTPORT LayerInputPort;
} test_disp_TestPortStatus_t;

typedef int(*test_func)(test_disp_TestPortStatus_t *);

typedef struct {
    const char *desc;
    test_func  func;
    int        next_index;
    MI_BOOL    bEnd;
} test_disp_TestStatusMenu_t;

pthread_mutex_t seq_lock = PTHREAD_MUTEX_INITIALIZER;
test_disp_TestPortStatus_t stPortStatus[MAXPORTNUM];

static int test_DISP_SetEnd(int id)
{
    return E_DISP_TEST_SET_RET_EXIT;
}
static int getChoice(void)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    return atoi(buffer);
}

static int test_DISP_SetPause(test_disp_TestPortStatus_t *pDispPortStatus)
{
    int id;
    printf("\n Set Pause Entry your choice Port 0~%d\n", pDispPortStatus[0].u8PortNum);
    id = getChoice();
    if(id< pDispPortStatus[id].u8PortNum)
        pDispPortStatus[id].LayerInputPort = id;
    else
    {
        printf("MaxPort Num %d, input id is %d\n", pDispPortStatus[id].u8PortNum, id);
    }

    pDispPortStatus[id].PortStatus = E_DISP_TEST_SET_PAUSE;
    ExecFunc(MI_DISP_PauseInputPort(pDispPortStatus[id].DispLayer, pDispPortStatus[id].LayerInputPort),MI_SUCCESS);

    return E_DISP_TEST_SET_RET_PASS;
}

static int test_DISP_SetResume(test_disp_TestPortStatus_t *pDispPortStatus)
{
    int id;

    printf("\n Set Resume Entry your choice Port 0~%d\n", pDispPortStatus[0].u8PortNum);
    id = getChoice();
    if(id< pDispPortStatus[id].u8PortNum)
        pDispPortStatus[id].LayerInputPort = id;
    else
    {
        printf("MaxPort Num %d, input id is %d\n", pDispPortStatus[id].u8PortNum, id);
    }

    if(pDispPortStatus[id].PortStatus == E_DISP_TEST_SET_STEP)
    {
        pDispPortStatus[id].u32SendStremTime = 33;
    }

    pDispPortStatus[id].PortStatus = E_DISP_TEST_SET_RESUME;
    ExecFunc(MI_DISP_ResumeInputPort (pDispPortStatus[id].DispLayer, pDispPortStatus[id].LayerInputPort),MI_SUCCESS);
    return E_DISP_TEST_SET_RET_PASS;
}

static int test_DISP_SetHidePort(test_disp_TestPortStatus_t *pDispPortStatus)
{
    int id;

    printf("\n Set Hide Entry your choice Port 0~%d\n", pDispPortStatus[0].u8PortNum);
    id = getChoice();
    if(id< pDispPortStatus[id].u8PortNum)
        pDispPortStatus[id].LayerInputPort = id;
    else
    {
        printf("MaxPort Num %d, input id is %d\n", pDispPortStatus[id].u8PortNum, id);
    }

    pDispPortStatus[id].PortStatus = E_DISP_TEST_SET_HIDE;
    ExecFunc(MI_DISP_HideInputPort(pDispPortStatus[id].DispLayer, pDispPortStatus[id].LayerInputPort),MI_SUCCESS);
    return E_DISP_TEST_SET_RET_PASS;
}

static int test_DISP_SetShowPort(test_disp_TestPortStatus_t *pDispPortStatus)
{
    int id;

    printf("\n Set Show Entry your choice Port 0~%d\n", pDispPortStatus[0].u8PortNum);
    id = getChoice();
    if(id< pDispPortStatus[id].u8PortNum)
        pDispPortStatus[id].LayerInputPort = id;
    else
    {
        printf("MaxPort Num %d, input id is %d\n", pDispPortStatus[id].u8PortNum, id);
    }

    pDispPortStatus[id].PortStatus = E_DISP_TEST_SET_SHOW;
    ExecFunc(MI_DISP_ShowInputPort(pDispPortStatus[id].DispLayer, pDispPortStatus[id].LayerInputPort),MI_SUCCESS);
    return E_DISP_TEST_SET_RET_PASS;
}

static int test_DISP_SetStepPort(test_disp_TestPortStatus_t *pDispPortStatus)
{
    int id;

    printf("\n Set Show Entry your choice Port 0~%d\n", pDispPortStatus[0].u8PortNum);
    id = getChoice();
    if(id< pDispPortStatus[id].u8PortNum)
        pDispPortStatus[id].LayerInputPort = id;
    else
    {
        printf("MaxPort Num %d, input id is %d\n", pDispPortStatus[id].u8PortNum, id);
    }

    pDispPortStatus[id].PortStatus = E_DISP_TEST_SET_STEP;

    ExecFunc(MI_DISP_StepInputPort(pDispPortStatus[id].DispLayer, pDispPortStatus[id].LayerInputPort),MI_SUCCESS);

    return E_DISP_TEST_SET_RET_PASS;
}

static test_disp_TestStatusMenu_t _gTestDispSetMode[] = {
    // PARAM
    [E_DISP_TEST_SET_PAUSE]      = {"SET_PORT_PAUSE",    test_DISP_SetPause,       E_DISP_TEST_SET_PAUSE, FALSE},
    [E_DISP_TEST_SET_STEP]       = {"SET_PORT_STEP",     test_DISP_SetStepPort,    E_DISP_TEST_SET_PAUSE, FALSE},
    [E_DISP_TEST_SET_RESUME]     = {"SET_PORT_RESUME",   test_DISP_SetResume,      E_DISP_TEST_SET_PAUSE, FALSE},
    [E_DISP_TEST_SET_HIDE]       = {"SET_PORT_HIDE",     test_DISP_SetHidePort,    E_DISP_TEST_SET_PAUSE, FALSE},
    [E_DISP_TEST_SET_SHOW]       = {"SET_PORT_SHOW",     test_DISP_SetShowPort,    E_DISP_TEST_SET_PAUSE, TRUE},

    //[E_DISP_TEST_CSC_SHOW_PARA]  = {"SHOW_PARA",         test_DISP_CscShowPara,     E_DISP_TEST_SET_PAUSE, TRUE },
};

static int showMenu(int index)
{
    int i = 0;
    test_disp_TestStatusMenu_t *pstMenu = NULL;
    printf("\n");
    do
    {
        pstMenu = &_gTestDispSetMode[index + i];
        printf("[%d]: %s.\n", i, pstMenu->desc);
        i++;
        if (pstMenu->bEnd == TRUE)
        {
            break;
        }
    }while(1);
    printf("\n[%d]: %s.\n", i, "Quit");
    printf("Enter you Choice: \n");
    return i;
}

static void * Test_DISP_SetMode (void *pData)
{
    test_disp_TestPortStatus_t *pDispPortStatus = (test_disp_TestPortStatus_t *)pData;
    int i = 0;
    int index = 1;
    int opt = 0;
    int max = 0;

    test_disp_TestStatusMenu_t *pstMenu = NULL;
    printf("Welcome: DISP Mode test.\n");
    while(1)
    {
        max = showMenu(index);
        opt = getChoice();
        if ((opt < 0) || (opt > max))
        {
            printf("Invalid input option !!.\n");
            continue;
        }
        else
        {
            if (opt == max)
            {
                break;
            }
            else
            {
                if((index + opt) < (sizeof(_gTestDispSetMode)/sizeof(_gTestDispSetMode[0])))
                {
                    pstMenu = &_gTestDispSetMode[index + opt];

                    if (pstMenu->func != NULL)
                    {
                        pstMenu->func(pDispPortStatus);
                    }
                    else
                        printf("func is NULL");

                    index = pstMenu->next_index;
                }
            }
        }
    }

    _gbStop = TRUE;
    return 0;
}

static void *test_Disp_SendStream(void *args)
{
    MI_SYS_BUF_HANDLE hHandle;
    int framesize = 0;
     test_disp_TestPortStatus_t *pDispPortStatus = (test_disp_TestPortStatus_t *)args;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    struct timeval stTv;

    MI_SYS_ChnPort_t stDispChnInputPort;
    MI_U16 u16Height=0,u16Width=0;
    int srcFd = pDispPortStatus->srcFd;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;

    stDispChnInputPort.eModId = E_MI_MODULE_ID_DISP;
    stDispChnInputPort.u32DevId = 0;
    stDispChnInputPort.u32ChnId = 0;
    stDispChnInputPort.u32PortId = pDispPortStatus->u8PortId;

    while(_gbStop == FALSE)
    {
        gettimeofday(&stTv, NULL);

        stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
        stBufConf.stFrameCfg.u16Width = pDispPortStatus->u16Width;
        stBufConf.stFrameCfg.u16Height = pDispPortStatus->u16Height;
        stBufConf.stFrameCfg.eFormat = pDispPortStatus->ePixelFormat;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

        gettimeofday(&stGetBuffer, NULL);
        if(pDispPortStatus->PortStatus == E_DISP_TEST_SET_PAUSE)
        {
            continue;
        }
        else if(pDispPortStatus->PortStatus == E_DISP_TEST_SET_STEP)
        {
            pDispPortStatus->u32SendStremTime += 10;
            pDispPortStatus->PortStatus = E_DISP_TEST_SET_NULL;
        }

        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChnInputPort, &stBufConf,&stBufInfo,&hHandle, -1))
        {
            stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
            stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
            stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
            stBufInfo.bEndOfStream = FALSE;

            framesize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            u16Height = stBufInfo.stFrameData.u16Height;
            u16Width  = stBufInfo.stFrameData.u16Width;
            DBG_INFO(" u32PortId = %d, u32Stride = %d, framesize = %d, virAddr = %p  ! \n",
                pDispPortStatus->u8PortId, stBufInfo.stFrameData.u32Stride[0], framesize, stBufInfo.stFrameData.pVirAddr[0]);
            //usleep(100 * 1000);

            gettimeofday(&stReadFile, NULL);
            pthread_mutex_lock(&seq_lock);
            if (test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], u16Width, u16Height, stBufInfo.stFrameData.u32Stride[0]) == 1)
            {
                gettimeofday(&stFlushData, NULL);

                gettimeofday(&stPutBuffer, NULL);
                MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);

                gettimeofday(&stRelease, NULL);
                DBG_INFO(" %d : %d : %d: %d: %d\n", (stGetBuffer.tv_sec*1000000 + stGetBuffer.tv_usec), (stReadFile.tv_sec*1000000 + stReadFile.tv_usec),
                    (stFlushData.tv_sec*1000000 + stFlushData.tv_usec), (stPutBuffer.tv_sec*1000000 + stPutBuffer.tv_usec),
                    (stRelease.tv_sec*1000000 + stRelease.tv_usec));
            }
            else
            {
                //DBG_INFO(" _test_disp_GetOneFrame fail.\n");
                test_disp_FdRewind(srcFd);
                MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
            }
            pthread_mutex_unlock(&seq_lock);
        }
        else
        {
            DBG_INFO("get buf fail\n");
        }

        usleep(pDispPortStatus->u32SendStremTime * 1000);
    }

    return 0;
}

//pause and resume
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_BOOL bsleep = true;
    MI_U32 u32PortId =0;
    MI_U32 u32Toleration = 100;

    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    // Test Device
    MI_DISP_PubAttr_t stPubAttr;
    int i = 0;
    int srcFd = 0;
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;

    if(argc < 7)
    {
        printf("Test Pattern: [disp filePath pchFormat  width heigt FrameCount PortNum]\n");
        return 0;
    }

    MI_DISP_DEV DispDev = atoi(argv[1]);
    MI_DISP_LAYER DispLayer = atoi(argv[2]);
    const char* filePath = argv[3];
    MI_U16 u16Width = atoi(argv[4]);
    MI_U16 u16Height = atoi(argv[5]);
    MI_U32 u32PortNum = atoi(argv[6]);

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    for(i = 0; i < argc; i++)
    {
        DBG_INFO("Argument %d is %s.\n", i, argv[i]);
    }

    if (TRUE != test_disp_OpenSourceFile(filePath, &srcFd))
    {
        printf("open sourcefile Fail \n");
        return FALSE;
    }

    for(i=0;i<u32PortNum;i++)
    {
        stPortStatus[i].u8PortNum = u32PortNum;
        stPortStatus[i].u16Width = u16Width;
        stPortStatus[i].u16Height = u16Height;
        stPortStatus[i].ePixelFormat = ePixelFormat;
        stPortStatus[i].u32SendStremTime = 33;
        stPortStatus[i].srcFd = srcFd;
        stPortStatus[i].u8PortId = i;
    }

    DBG_INFO("File Path:%s, format: %s(%u), width = %d, height = %d , FrameCount = %llu, port number = %u.\n", filePath, ePixelFormat,ePixelFormat,u16Width, u16Height,u32PortNum);

    MIDISPCHECKRESULT(MI_SYS_Init());

    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = 1920;
    stLayerAttr.stVidLayerSize.u16Height= 1080;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    stLayerAttr.ePixFormat = ePixelFormat;

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    mi_disp_hdmiInit();

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", \
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);

    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    ExecFunc(MI_DISP_SetVideoLayerAttrBegin(DispLayer), MI_DISP_SUCCESS);
    if(u32PortNum <= 4)
    {
        u16Avg = 2;
    }
    else if(u32PortNum <= 9)
    {
        u16Avg = 3;
    }
    else if(u32PortNum <= 16)
    {
        u16Avg = 4;
    }

    for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
    {
        stInputPortAttr[u32PortId].stDispWin.u16Width = u16Width;
        stInputPortAttr[u32PortId].stDispWin.u16Height = u16Height;

        u16Row = u32PortId / u16Avg;
        u16Column = u32PortId - u16Row * u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16X = u16Column * u16Width;
        stInputPortAttr[u32PortId].stDispWin.u16Y = u16Row * u16Height;

        ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId])), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId])), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS)
    }
    ExecFunc(MI_DISP_SetVideoLayerAttrEnd(DispLayer), MI_DISP_SUCCESS);

    pthread_t thread;
    pthread_attr_t attr;

    int s = pthread_attr_init(&attr);
    if (s != 0)
        perror("pthread_attr_init");

    pthread_create(&thread, &attr, Test_DISP_SetMode, &stPortStatus);

    for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
    {
        int s = pthread_attr_init(&stPortStatus[u32PortId].attr);
        if (s != 0)
            perror("pthread_attr_init");

        pthread_create(&stPortStatus[u32PortId].thread, &stPortStatus[u32PortId].attr, test_Disp_SendStream, &stPortStatus[u32PortId]);
    }

    pthread_join(thread, NULL);
    s = pthread_attr_destroy(&attr);
    if (s != 0)
        perror("pthread_attr_destroy");


    for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
    {
        pthread_join(stPortStatus[u32PortId].thread, NULL);

        s = pthread_attr_destroy(&stPortStatus[u32PortId].attr);
        if (s != 0)
            perror("pthread_attr_destroy");
    }
EXIT_TEST:

    DBG_INFO(" Prepare to Exit. \n");

    if(srcFd > 0)
    {
        test_disp_CloseFd(srcFd);
        for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
        {
            ExecFunc(MI_DISP_DisableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS);
        }
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }

    usleep(2*1000*1000);
    DBG_INFO("####### disp test110 end ##############\n");

    return MI_SUCCESS;
}
