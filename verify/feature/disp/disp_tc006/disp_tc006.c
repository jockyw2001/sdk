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
#include <fcntl.h>


#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

///#define MI_PRINT(fmt, args...)
#define MI_PRINT DBG_INFO
#define DBG_ERR(fmt, args...) //printf(fmt, ##args)
#define DBG_INFO(fmt, args...) //printf(fmt, ##args)

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        MI_PRINT("[%d]exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%d)exec function pass\n", __LINE__);\
    }

#define MIDISPCHECKRESULT(result)\
    if(result != MI_SUCCESS)\
    {\
        MI_PRINT("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

static MI_BOOL _bHandlerThreadExit = FALSE;
static char aSaveFilePath[256];

static void* _mi_disp_HandlerThread(void *argv)
{
    char filename[256];
    int cap_count = 0;
    //prctl(PR_SET_NAME,(unsigned long)"MiDispHandlerThread");
    while(!_bHandlerThreadExit)
    {
        FILE *fp;
        int off = 0;
        MI_U8 u8CapScreen = 0;
        MI_DISP_VideoFrame_t stCapFrame;
        memset(&stCapFrame, 0, sizeof(MI_DISP_VideoFrame_t));
        scanf("%d", &u8CapScreen);
        int fd;
        char *p = NULL;
        if(u8CapScreen == 1)
        {
            //fp = fopen(aSaveFilePath, "wb+");

            printf("[%s %d]  cap start\n", __FUNCTION__, __LINE__);
            if (MI_DISP_GetScreenFrame(0, &stCapFrame) != MI_DISP_SUCCESS)
            {
                continue;
            }
            cap_count++;
            printf("Screen: W: %d X H: %d %p.\n", stCapFrame.u32Width, stCapFrame.u32Height, stCapFrame.pavirAddr);
            sprintf(filename, "%s/cap_output_%uX%u_yuv422_%d.yuv", aSaveFilePath, stCapFrame.u32Width, stCapFrame.u32Height, cap_count);
            fd = open(filename, O_WRONLY|O_CREAT, 0777);
            if (fd < 0)
            {
                printf("dest_file: %s.\n", filename);
                perror("open");
                return NULL;
            }

            printf("Cap Screen, file path : %s\n", filename);
            p = stCapFrame.pavirAddr;
            while(off < stCapFrame.u32Size)
            {
                //off += fwrite(stCapFrame.pavirAddr+off, 1, stCapFrame.u32Size-off, fp);

                off += write(fd, p, stCapFrame.u32Size - off);
                printf("[%s %d] Off = %d \n", __FUNCTION__, __LINE__, off);
            }
            printf("[%s %d] Cap done, Save in file: %s \n", __FUNCTION__, __LINE__, filename);
            //fclose(fp);
            close(fd);
            MI_DISP_ReleaseScreenFrame(0, &stCapFrame);
            sync();
        }

    }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

    if(argc < 5)
    {
        printf("%s: <YUV 422 filePath> <width> <heigt> <source file FrameCount>\n", argv[0]);
        printf("It will auto gen output yuv file.\n");
        return 0;
    }
    int i = 0;
    for(i = 0; i < argc; i++)
    {
        DBG_INFO("Argument %d is %s./n", i, argv[i]);
    }
    char filePath[256];
    strcpy(filePath, argv[1]);
    MI_U16 u16Width = atoi(argv[2]);
    MI_U16 u16Height = atoi(argv[3]);
    MI_U64 u64Times = atoi(argv[4]);
    sprintf(aSaveFilePath, "%s/", dirname(argv[1]));

    printf("File Path:%s, width = %d, height = %d , FrameCount = %d, targeFile: %s , CapSavePath: %s\n", filePath, u16Width, u16Height, u64Times, filePath, aSaveFilePath);
    MIDISPCHECKRESULT(MI_SYS_Init());

    MI_SYS_ChnPort_t stDispChn0InputPort0;
    stDispChn0InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispChn0InputPort0.u32DevId = 0;
    stDispChn0InputPort0.u32ChnId = 0;
    stDispChn0InputPort0.u32PortId = 0;
    // Test Device
    MI_DISP_DEV DispDev = 0;

    MI_DISP_PubAttr_t stPubAttr;

    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);
    mi_disp_hdmiInit();

    MI_DISP_LAYER DispLayer = 0;

    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = u16Width;
    stLayerAttr.stVidLayerSize.u16Height= u16Height;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("[%s %d]Get Video Layer Size [%d, %d] !!!\n", __FUNCTION__, __LINE__, stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("[%s %d]Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", __FUNCTION__, __LINE__,\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width    = u16Width;
    stInputPortAttr.stDispWin.u16Height = u16Height;
    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS)

    pthread_attr_t thread_attr;
    pthread_t tid = 0;
    pthread_attr_init(&thread_attr);
    pthread_create(&tid, &thread_attr, _mi_disp_HandlerThread, NULL);


    int fd =0;
    if(TRUE == test_disp_OpenSourceFile(filePath, &fd))
    {
        while(1)
        {
            int n = 0;
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChn0InputPort0,&stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream = FALSE;
                DBG_INFO(" [%s %d] virAddr = %p  !!!!!\n", __FUNCTION__, __LINE__,stBufInfo.stFrameData.pVirAddr[0]);
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];

                DBG_INFO(" [%s %d] u32Stride = %d  \n", __FUNCTION__, __LINE__, stBufInfo.stFrameData.u32Stride[0]);
                DBG_INFO(" [%s %d] size = %d  \n", __FUNCTION__, __LINE__, size);

                if (test_disp_GetOneYuv422Frame(fd, 0, stBufInfo.stFrameData.pVirAddr[0], u16Width, u16Height, stBufInfo.stFrameData.u32Stride[0]) == 1)
                {
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                }
                else
                {
                    test_disp_FdRewind(fd);
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                }

                u64Times--;
                if(u64Times == 0)
                {
                    break;
                }
            }
            else
            {
                DBG_INFO("get buf fail\n");
            }
            usleep(100 * 1000);
        }
    }
    if(fd > 0)
    {
        _bHandlerThreadExit = TRUE;
        test_disp_CloseFd(fd);
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }
    DBG_INFO("disp test end\n");

    return MI_SUCCESS;
}
