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
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

#define DBG_INFO(fmt, args...)      ({do{printf("\n [MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);usleep(10000);})

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


static MI_BOOL _mi_disp_GetFrame(int srcFd, char *pData, int yuvSize)
{
    if (read(srcFd, pData, yuvSize) < yuvSize)
    {
        lseek(srcFd, 0, SEEK_SET);
        //rewind(srcFd);
        if (read(srcFd, pData, yuvSize) < yuvSize)
        {
            printf(" [%s %d] read file error.\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }

    return TRUE;
}

//change video display window and position
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_BOOL bsleep = true;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    MI_DISP_LAYER DispLayer = 0;
    MI_U32 u32Toleration = 100;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    // Test Device
    MI_DISP_DEV DispDev = 0;
    MI_DISP_PubAttr_t stPubAttr;
    int i = 0;
    int srcFd = 0;
    int framesize = 0;
    MI_U32 u32PortId =0;
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    const char* filePath = argv[1];
    const char* pchFormat = argv[2];
    MI_U16 u16Width = atoi(argv[3]);
    MI_U16 u16Height = atoi(argv[4]);
    MI_U64 u64Times = atoi(argv[5]);
    MI_U32 u32PortNum = atoi(argv[6]);

    MI_U32 u32LayerDispX = atoi(argv[7]);
    MI_U32 u32LayerDispY = atoi(argv[8]);
    MI_U32 u32LayerDispWidth = atoi(argv[9]);
    MI_U32 u32LayerDispHeight = atoi(argv[10]);

    time_t stTime = 0;
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;
    MI_U32 u32Index = 0;

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stTv, 0, sizeof(stTv));
    memset(&stBufInfo , 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0, sizeof(MI_SYS_BufConf_t));
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    if(argc < 6)
    {
        DBG_INFO("Test Pattern: [disp filePath format FrameWidth FrameHeigt FrameCount LayerX LayerY LayerWidth LayerHeight]\n");
        return 0;
    }

    for(i = 0; i < argc; i++)
    {
        DBG_INFO("Argument %d is %s.\n", i, argv[i]);
    }

    if(strncmp(pchFormat, "yuv422", 6)  == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV. \n");
    }
    else if(strncmp(pchFormat, "argb", 4) == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888. \n");
    }
    else if(strncmp(pchFormat, "rgb565", 6) == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565. \n");
    }
    else
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420. \n");
    }

    DBG_INFO("File Path:%s, format: %s(%u), width = %d, height = %d , FrameCount = %llu, portnumber = %u.\n", filePath, pchFormat,ePixelFormat,u16Width, u16Height, u64Times,u32PortNum);

    MIDISPCHECKRESULT(MI_SYS_Init());

    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_480P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = 720;
    stLayerAttr.stVidLayerSize.u16Height = 480;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = 720;
    stLayerAttr.stVidLayerDispWin.u16Height = 480;
    stLayerAttr.ePixFormat = ePixelFormat;

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    //mi_disp_hdmiInit();

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
CHNANGE_VIDEO_LAYER_ATTR:
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", \
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    usleep(100 * 1000);
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
        stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
        stDispChnInputPort[u32PortId].u32DevId = 0;
        stDispChnInputPort[u32PortId].u32ChnId = 0;
        stDispChnInputPort[u32PortId].u32PortId = u32PortId;

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

    if (TRUE == test_disp_OpenSourceFile(filePath, &srcFd))
    {
        while(1)
        {
            gettimeofday(&stTv, NULL);

            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            stBufConf.stFrameCfg.eFormat = ePixelFormat;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
            {
                gettimeofday(&stGetBuffer, NULL);
                if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChnInputPort[u32PortId],&stBufConf,&stBufInfo,&hHandle, -1))
                {
                    stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                    stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                    stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                    stBufInfo.bEndOfStream = FALSE;

                    framesize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                    u16Height = stBufInfo.stFrameData.u16Height;
                    u16Width  = stBufInfo.stFrameData.u16Width;
                    DBG_INFO(" u32PortId = %d, u32Stride = %d, framesize = %d, virAddr = %p  ! \n",
                        u32PortId, stBufInfo.stFrameData.u32Stride[0], framesize, stBufInfo.stFrameData.pVirAddr[0]);
                    //usleep(100 * 1000);

                    gettimeofday(&stReadFile, NULL);
                    if (_mi_disp_GetFrame(srcFd, stBufInfo.stFrameData.pVirAddr[0], framesize) == 1)
                    {
                        gettimeofday(&stFlushData, NULL);

                        gettimeofday(&stPutBuffer, NULL);
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);

                        gettimeofday(&stRelease, NULL);
                        DBG_INFO(" %d : %d : %d: %d: %d\n", (stGetBuffer.tv_sec*1000000 + stGetBuffer.tv_usec), (stReadFile.tv_sec*1000000 + stReadFile.tv_usec),
                            (stFlushData.tv_sec*1000000 + stFlushData.tv_usec), (stPutBuffer.tv_sec*1000000 + stPutBuffer.tv_usec),
                            (stRelease.tv_sec*1000000 + stRelease.tv_usec));

                        u64Times--;
                        if(u64Times == 4)
                        {
                            usleep(10*1000*1000);
                            stLayerAttr.stVidLayerDispWin.u16X = u32LayerDispX;
                            stLayerAttr.stVidLayerDispWin.u16Y = u32LayerDispY;
                            stLayerAttr.stVidLayerDispWin.u16Width = u32LayerDispWidth;
                            stLayerAttr.stVidLayerDispWin.u16Height = u32LayerDispHeight;

                            for(u32Index = 0; u32Index < u32PortNum; u32Index ++)
                            {
                                MI_DISP_DisableInputPort(DispLayer, u32InputPort[u32Index]);
                                usleep(100*1000);
                            }
                            MI_DISP_DisableVideoLayer(DispLayer);

                            goto CHNANGE_VIDEO_LAYER_ATTR;
                        }
                        usleep(100 * 1000);
                        if(u64Times == 0)
                        {
                            DBG_INFO("  u64Times = %llu \n", u64Times);
                            usleep(100 * 1000);
                            goto EXIT_TEST;
                        }
                    }
                    else
                    {
                        DBG_INFO(" _test_disp_GetOneFrame fail.\n");
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                    }
                }
                else
                {
                    DBG_INFO("get buf fail\n");
                }
            }
            usleep(100 * 1000);
        }
    }
    else
    {
        DBG_INFO(" open file fail. \n");
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

    usleep(5*1000*1000);
    DBG_INFO("####### disp test107 end ##############\n");

    while(0)
    {
        if(bsleep)
        {
            bsleep = false;
            DBG_INFO(" sleep. \n");
        }
        usleep(100*1000);
    }
    return MI_SUCCESS;
}
