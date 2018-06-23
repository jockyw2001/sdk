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
#include "mi_divp_datatype.h"
#include "mi_divp.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_hdmi.h"

#define DBG_INFO(fmt, args...)      ({do{printf("\n [MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);usleep(10000);})
#define BIND_DIVP (1)

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

            DBG_INFO(" cap start\n");
            if (MI_DISP_GetScreenFrame(0, &stCapFrame) != MI_DISP_SUCCESS)
            {
                continue;
            }
            cap_count++;
            DBG_INFO("Screen: W: %d X H: %d %p.\n", stCapFrame.u32Width, stCapFrame.u32Height, stCapFrame.pavirAddr);
            sprintf(filename, "%s/cap_output_%uX%u_yuv422_%d.yuv", aSaveFilePath, stCapFrame.u32Width, stCapFrame.u32Height, cap_count);
            fd = open(filename, O_WRONLY|O_CREAT, 0777);
            if (fd < 0)
            {
                DBG_INFO("dest_file: %s.\n", filename);
                perror("open");
                return NULL;
            }

            DBG_INFO("Cap Screen, file path : %s\n", filename);
            p = stCapFrame.pavirAddr;
            while(off < stCapFrame.u32Size)
            {
                //off += fwrite(stCapFrame.pavirAddr+off, 1, stCapFrame.u32Size-off, fp);

                off += write(fd, p, stCapFrame.u32Size - off);
                DBG_INFO("Off = %d \n", off);
            }
            DBG_INFO("Cap done, Save in file: %s \n", filename);
            //fclose(fp);
            close(fd);
            MI_DISP_ReleaseScreenFrame(0, &stCapFrame);
            sync();
        }

    }
    pthread_exit(NULL);
    return NULL;
}

static MI_BOOL _test_disp_GetOneFrame(int srcFd, char *pData, int yuvSize)
{
    if (read(srcFd, pData, yuvSize) < yuvSize)
    {
        lseek(srcFd, 0, SEEK_SET);
        if (read(srcFd, pData, yuvSize) < yuvSize)
        {
            DBG_INFO(" read file error.\n");
            return FALSE;
        }
    }

    return TRUE;
}
MI_BOOL mi_disp_GetInputFrameDataYuv422(FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
{
    MI_BOOL bRet = FALSE;
    MI_U32 u32ReadSize = 0;
    MI_U32 u32LineNum = 0;
    MI_U32 u32BytesPerLine = 0;
    MI_U32 u32Index = 0;
    MI_U32 u32FrameDataSize = 0;

    if (pInputFile == NULL)
    {
        DBG_INFO("create file error.\n");
        return bRet;
    }

    if(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == pstBufInfo->stFrameData.ePixelFormat)
    {
        u32LineNum = pstBufInfo->stFrameData.u16Height;
        u32BytesPerLine = pstBufInfo->stFrameData.u16Width * 2;
        u32FrameDataSize = u32BytesPerLine * u32LineNum;
        DBG_INFO("u16Width = %u, u16Height = %u, u32BytesPerLine = %u, u32LineNum = %u. u32FrameDataSize = %u \n",
            pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height, u32BytesPerLine, u32LineNum, u32FrameDataSize);
    }
    else
    {
        DBG_INFO("######  error ######. ePixelFormat = %u\n", pstBufInfo->stFrameData.ePixelFormat);
        return bRet;
    }

    for (u32Index = 0; u32Index < u32LineNum; u32Index ++)
    {
        u32ReadSize += fread(pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], 1, u32BytesPerLine, pInputFile);
    }

    if(u32ReadSize == u32FrameDataSize)
    {
        bRet = TRUE;
    }
    else if(u32ReadSize < u32FrameDataSize)
    {
        fseek(pInputFile, 0, SEEK_SET);
        u32ReadSize = 0;

        for (u32Index = 0; u32Index < u32LineNum; u32Index ++)
        {
            u32ReadSize += fread(pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], 1, u32BytesPerLine, pInputFile);
        }

        if(u32ReadSize == u32FrameDataSize)
        {
            bRet = TRUE;
        }
        else
        {
            DBG_INFO("read file error. u32ReadSize = %u. \n", u32ReadSize);
            bRet = FALSE;
        }
    }

    DBG_INFO("u32ReadSize = %d. bRet = %u\n", u32ReadSize, bRet);
    return bRet;
}

MI_BOOL mi_disp_GetInputFrameData420(FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
{
    MI_BOOL bRet = FALSE;
    MI_U32 u32LineNum = 0;
    MI_U32 u32BytesPerLine = 0;
    MI_U32 u32Index = 0;
    MI_U32 u32FrameDataSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;

    if (pInputFile == NULL)
    {
        DBG_INFO("create file error.\n");
        return bRet;
    }

    if(E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == pstBufInfo->stFrameData.ePixelFormat)
    {
        u32LineNum = pstBufInfo->stFrameData.u16Height * 3 / 2;
        u32BytesPerLine = pstBufInfo->stFrameData.u16Width;
        u32FrameDataSize = u32BytesPerLine * u32LineNum;
        DBG_INFO("u16Width = %u, u16Height = %u, u32BytesPerLine = %u, u32LineNum = %u. u32FrameDataSize = %u \n",
            pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height, u32BytesPerLine, u32LineNum, u32LineNum);
    }
    else
    {
        DBG_INFO("######  error ######. bRet = %u\n", bRet);
        return bRet;
    }

    DBG_INFO(" read input frame data : pInputFile = %p, pstBufInfo = %p.\n", pInputFile, pstBufInfo);
    for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height; u32Index ++)
    {
        u32YSize += fread(pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], 1, u32BytesPerLine, pInputFile);
    }

    for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height / 2; u32Index ++)
    {
        u32UVSize += fread(pstBufInfo->stFrameData.pVirAddr[1] + u32Index * pstBufInfo->stFrameData.u32Stride[1], 1, u32BytesPerLine, pInputFile);
    }

    if(u32YSize + u32UVSize == u32FrameDataSize)
    {
        bRet = TRUE;
    }
    else if(u32YSize + u32UVSize < u32FrameDataSize)
    {
        fseek(pInputFile, 0, SEEK_SET);
        u32YSize = 0;
        u32UVSize = 0;

        for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height; u32Index ++)
        {
            u32YSize += fread(pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], 1, u32BytesPerLine, pInputFile);
        }

        for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height / 2; u32Index ++)
        {
            u32UVSize += fread(pstBufInfo->stFrameData.pVirAddr[1] + u32Index * pstBufInfo->stFrameData.u32Stride[1], 1, u32BytesPerLine, pInputFile);
        }

        if(u32YSize + u32UVSize == u32FrameDataSize)
        {
            bRet = TRUE;
        }
        else
        {
            DBG_INFO(" read file error. u32YSize = %u, u32UVSize = %u. \n", u32YSize, u32UVSize);
            bRet = FALSE;
        }
    }

    DBG_INFO(" u32YSize = %u, u32UVSize = %u. bRet = %u\n", u32YSize, u32UVSize, bRet);

    return bRet;
}

MI_BOOL mi_disp_GetInputFrameData(MI_SYS_PixelFormat_e ePixelFormat, FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
{
    MI_BOOL bRet = TRUE;
    if((NULL == pInputFile) || (NULL == pstBufInfo))
    {
        bRet = FALSE;
        DBG_INFO(" read input frame data failed! pInputFile = %p, pstBufInfo = %p.\n", pInputFile, pstBufInfo);
    }
    else
    {
        DBG_INFO(" read input frame data : pInputFile = %p, pstBufInfo = %p.\n", pInputFile, pstBufInfo);
        if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
        {
            if(!mi_disp_GetInputFrameData420(pInputFile, pstBufInfo))
            {
                bRet = FALSE;
                DBG_INFO(" read input frame data failed!.\n");
            }
        }
        else if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
        {
            if(!mi_disp_GetInputFrameDataYuv422(pInputFile, pstBufInfo))
            {
                bRet = FALSE;
                DBG_INFO(" read input frame data failed!.\n");
            }
        }
    }

    DBG_INFO(" bRet = %u.\n", bRet);
    return bRet;
}

#if BIND_DIVP
MI_S32 mi_divp_test_CreateChn(MI_DIVP_CHN DivpChn, MI_U16 u16Width, MI_U16 u16Height)
{
    MI_S32 s32Ret = -1;
    MI_DIVP_ChnAttr_t stAttr;
    memset(&stAttr, 0, sizeof(stAttr));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16Width;
    stAttr.stCropRect.u16Height = u16Height;
    stAttr.u32MaxWidth = 1920;
    stAttr.u32MaxHeight = 1080;

    s32Ret = MI_DIVP_CreateChn(DivpChn, &stAttr);
    DBG_INFO("\n s32Ret = %d", s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_test_DestroyChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = -1;
    s32Ret = MI_DIVP_DestroyChn(DivpChn);
    DBG_INFO("\n s32Ret = %d", s32Ret);

    return s32Ret;
}

MI_S32 mi_divp_test_StartChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = -1;
    s32Ret = MI_DIVP_StartChn(DivpChn);
    DBG_INFO("\n s32Ret = %d", s32Ret);

    return s32Ret;
}

MI_S32 mi_divp_test_StopChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = -1;

    s32Ret = MI_DIVP_StopChn(DivpChn);
    DBG_INFO("\n s32Ret = %d", s32Ret);

    return s32Ret;
}

MI_S32 mi_divp_test_SetOutputPortAttr(MI_DIVP_CHN DivpChn, MI_SYS_PixelFormat_e eOutPixelFormat, MI_U16 u16Width, MI_U16 u16Height)
{
    MI_S32 s32Ret = -1;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPixelFormat;//E_MI_SYS_PIXEL_FRAME_YUV_MST_420;//E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;//
    stOutputPortAttr.u32Width = u16Width;
    stOutputPortAttr.u32Height = u16Height;

    s32Ret = MI_DIVP_SetOutputPortAttr(DivpChn, &stOutputPortAttr);
    DBG_INFO("\n s32Ret = %d", s32Ret);

    return s32Ret;
}
#endif

MI_BOOL mi_disp_SaveFileByName(char* pchFileName, void* pVirtAddr, MI_U32 u32Size)
{
    FILE* pFile =NULL;
    MI_U32 u32WriteSize = 0;
    DBG_INFO("save pchFileName = %s\n \n", pchFileName);


    pFile =fopen(pchFileName, "w+");
    if (pFile == NULL)
    {
        DBG_INFO("create file error.\n");
        return;
    }

    u32WriteSize = fwrite(pVirtAddr, 1, u32Size, pFile);
    DBG_INFO("u32WriteSize = %d. \n", u32WriteSize);
    fclose(pFile);
}

MI_S32 mi_disp_hdmiDeInit(void)
{
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_Stop(eHdmi);
    MI_HDMI_Close(eHdmi);
    MI_HDMI_DeInit();
    return MI_SUCCESS;
}
//capture screen
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_BOOL bsleep = true;
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
    FILE *pInputFile = NULL;
    int framesize = 0;
    MI_U32 u32PortId =0;
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    const char* filePath = argv[1];
    const char* pchFormat = argv[2];
    MI_U16 u16Width = atoi(argv[3]);
    MI_U16 u16Height = atoi(argv[4]);
    MI_U64 u64Times = atoi(argv[5]);
    MI_U32 u32PortNum = atoi(argv[6]);
#if BIND_DIVP
    const char* pchOutFrmt = argv[7];
    MI_SYS_PixelFormat_e eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_DIVP_CHN DivpChn = 0;
    MI_SYS_ChnPort_t stDivpOutputPort;
    MI_SYS_ChnPort_t stDivpInputPort;
#endif
    time_t stTime = 0;
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;
    MI_U16 u16Frame = u64Times / 2;
    MI_BOOL bprint = TRUE;
    char* pchFileName = "/mnt/test_results/cap.raw";

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stBufInfo , 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0, sizeof(MI_SYS_BufConf_t));
    memset(&stPubAttr, 0, sizeof(stPubAttr));

#if BIND_DIVP
    stDivpOutputPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpOutputPort.u32ChnId = 0;
    stDivpOutputPort.u32DevId = 0;
    stDivpOutputPort.u32PortId = 0;

    stDivpInputPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpInputPort.u32ChnId = 0;
    stDivpInputPort.u32DevId = 0;
    stDivpInputPort.u32PortId = 0;
#endif

    if(argc < 6)
    {
        DBG_INFO("Test Pattern: [disp filePath width heigt FrameCount]\n");
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
    else if(strncmp(pchFormat, "mst420", 6)  == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420. \n");
    }
    else
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420. \n");
    }

#if BIND_DIVP
    if(strncmp(pchOutFrmt, "yuv422", 6)  == 0)
    {
        eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        DBG_INFO("eOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV. \n");
    }
    else if(strncmp(pchOutFrmt, "argb", 4) == 0)
    {
        eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;
        DBG_INFO("eOutPixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888. \n");
    }
    else if(strncmp(pchOutFrmt, "rgb565", 6) == 0)
    {
        eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565;
        DBG_INFO("eOutPixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565. \n");
    }
    else if(strncmp(pchOutFrmt, "mst420", 6)  == 0)
    {
        eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        DBG_INFO("ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420. \n");
    }
    else
    {
        eDivpOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        DBG_INFO("eOutPixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420. \n");
    }
    DBG_INFO("File Path:%s, inputformat: %s(%u), width = %d, height = %d , FrameCount = %llu, port number = %u. eDivpOutPixelFormat = %u\n", filePath, pchFormat,ePixelFormat,u16Width, u16Height, u64Times,u32PortNum,eDivpOutPixelFormat);
#else
    DBG_INFO("File Path:%s, inputformat: %s(%u), width = %d, height = %d , FrameCount = %llu, port number = %u.\n", filePath, pchFormat,ePixelFormat,u16Width, u16Height, u64Times,u32PortNum);
#endif

#if BIND_DIVP
    mi_divp_test_CreateChn(DivpChn, u16Width, u16Height);
    mi_divp_test_SetOutputPortAttr(DivpChn, eDivpOutPixelFormat, u16Width, u16Height);
    mi_divp_test_StartChn(DivpChn);
#endif

    MI_SYS_Init();

    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = u16Width;
    stLayerAttr.stVidLayerSize.u16Height= u16Height;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
#if BIND_DIVP
    stLayerAttr.ePixFormat = eDivpOutPixelFormat;
#else
    stLayerAttr.ePixFormat = ePixelFormat;
#endif

    stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
    stDispChnInputPort[u32PortId].u32DevId = 0;
    stDispChnInputPort[u32PortId].u32ChnId = 0;
    stDispChnInputPort[u32PortId].u32PortId = u32PortId;

    stInputPortAttr[u32PortId].stDispWin.u16Width = u16Width;
    stInputPortAttr[u32PortId].stDispWin.u16Height = u16Height;
    stInputPortAttr[u32PortId].stDispWin.u16X = 0;
    stInputPortAttr[u32PortId].stDispWin.u16Y = 0;

    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    mi_disp_hdmiInit();
    MI_HDMI_Attr_t stHdmiAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);

    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n",stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n",\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);

    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId])), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId])), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS)

#if BIND_DIVP
    MI_SYS_BindChnPort(&stDivpOutputPort, &stDispChnInputPort[0], 30, 30);
#endif

    pInputFile =fopen(filePath, "rb");
    {
        while(1)
        {
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            stBufConf.u64TargetPts = 0x1234;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            stBufConf.stFrameCfg.eFormat = ePixelFormat;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
            {
#if BIND_DIVP
                if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDivpInputPort,&stBufConf,&stBufInfo,&hHandle, -1))
#else
                if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChnInputPort[u32PortId],&stBufConf,&stBufInfo,&hHandle, -1))
#endif
                {
                    stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                    stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                    stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
                    stBufInfo.bEndOfStream = FALSE;
                    if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
                    {
                        framesize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                    }
                    else if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_MST_420)
                    {
                        framesize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                    }
                    u16Height = stBufInfo.stFrameData.u16Height;
                    u16Width  = stBufInfo.stFrameData.u16Width;
                    DBG_INFO(" u32PortId = %d, u32Stride = %d, framesize = %d, virAddr = %p  ! \n",
                        u32PortId, stBufInfo.stFrameData.u32Stride[0], framesize, stBufInfo.stFrameData.pVirAddr[0]);
                    //usleep(100 * 1000);

                    if (mi_disp_GetInputFrameData(ePixelFormat, pInputFile, &stBufInfo) == TRUE)
                    {
                        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);

                        //capture screen
                        if(u64Times == u16Frame)
                        {
                            MI_U8 u8CapScreen = 0;
                            MI_DISP_VideoFrame_t stCapFrame;
                            memset(&stCapFrame, 0, sizeof(MI_DISP_VideoFrame_t));

                            DBG_INFO(" prepare to get capture  frame.\n");
                            usleep(1*1000*1000);

                            if (MI_DISP_GetScreenFrame(0, &stCapFrame) != MI_DISP_SUCCESS)
                            {
                                DBG_INFO("capture Screen failed!\n");
                                continue;
                            }
                            DBG_INFO("Screen: W = %d, H = %d, phyAddr = 0x%llx, pavirAddr = %p.\n", stCapFrame.u32Width,
                                stCapFrame.u32Height, stCapFrame.aphyAddr, stCapFrame.pavirAddr);

                            framesize = 720 * 1280 * 2;
                            mi_disp_SaveFileByName(pchFileName, stCapFrame.pavirAddr, framesize);

                            DBG_INFO(" prepare to release capture  frame.\n");
                            usleep(10*1000*1000);

                            MI_DISP_ReleaseScreenFrame(0, &stCapFrame);
                        }

                        u64Times--;
                        if(u64Times == 0)
                        {
                            DBG_INFO(" u64Times = %llu \n", u64Times);
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

EXIT_TEST:

    DBG_INFO(" Prepare to Exit. \n");

    while(0)
    {
        if(bprint)
        {
            bprint = FALSE;
            DBG_INFO("while 1 sleep. \n");
        }
        usleep(100 * 1000);
    }
    usleep(20*1000*1000);

    {
#if BIND_DIVP
        MI_SYS_UnBindChnPort(&stDivpOutputPort, &stDispChnInputPort[0]);
        mi_divp_test_StopChn(DivpChn);
        mi_divp_test_DestroyChn(DivpChn);
#endif

        fclose(pInputFile);
        for(u32PortId = 0; u32PortId < u32PortNum; u32PortId ++)
        {
            ExecFunc(MI_DISP_DisableInputPort(DispLayer, u32InputPort[u32PortId]), MI_SUCCESS);
        }
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(mi_disp_hdmiDeInit(), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }

    usleep(5*1000*1000);
    DBG_INFO("####### disp test106 end ##############\n");

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
