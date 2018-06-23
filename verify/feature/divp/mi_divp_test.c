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
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#ifndef MI_DIVP_FPGA_TEST
#include "mi_sys.h"
#include "mi_vdec_datatype.h"
#include "mi_vdec.h"
#endif
#include "mi_common_datatype.h"
#include "mi_divp_datatype.h"
#include "mi_divp.h"
#include "mi_disp.h"
#include "mi_vpe.h"
#include "mi_hdmi.h"

#define VDEC_CHN_MAX (16)
#define VESFILE_READER_BATCH (1024*1024)
#ifdef MI_DIVP_FPGA_TEST
typedef enum
{
    E_MI_VDEC_DB_MODE_H264_H265   = 0x00,
    E_MI_VDEC_DB_MODE_VP9         = 0x01,
    E_MI_VDEC_DB_MODE_MAX
} mi_vdec_DbMode_e; // Decoder Buffer Mode

typedef struct mi_vdec_DbInfo_s
{
    MI_BOOL bDbEnable;           // Decoder Buffer Enable
    MI_U8   u8DbSelect;          // Decoder Buffer select
    MI_BOOL bHMirror;
    MI_BOOL bVMirror;
    MI_BOOL bUncompressMode;
    MI_BOOL bBypassCodecMode;
    mi_vdec_DbMode_e eDbMode;        // Decoder Buffer mode
    MI_U16 u16StartX;
    MI_U16 u16StartY;
    MI_U16 u16HSize;
    MI_U16 u16VSize;
    MI_PHY phyDbBase;          // Decoder Buffer base addr
    MI_U16 u16DbPitch;         // Decoder Buffer pitch
    MI_U8  u8DbMiuSel;         // Decoder Buffer Miu select
    MI_PHY phyLbAddr;          // Lookaside buffer addr
    MI_U8  u8LbSize;           // Lookaside buffer size
    MI_U8  u8LbTableId;        // Lookaside buffer table Id
} mi_vdec_DbInfo_t; // Decoder Buffer Info

typedef enum
{
    E_MI_VDEC_FRAME_TYPE_I = 0,
    E_MI_VDEC_FRAME_TYPE_P,
    E_MI_VDEC_FRAME_TYPE_B,
    E_MI_VDEC_FRAME_TYPE_OTHER,
    E_MI_VDEC_FRAME_TYPE_NUM
} mi_vdec_FrameType_e;

typedef struct mi_vdec_FrameInfoExt_s
{
    MI_PHY phyLumaAddr2bit;                   // physical address of Luma 2bit buffer
    MI_PHY phyChromaAddr2bit;                 // physical address of Chroma 2bit buffer
    MI_U8  u8LumaBitDepth;                    // Luma Frame bitdepth, support 8~10bits now
    MI_U8  u8ChromaBitDepth;                  // Chroma Frame bitdepth, support 8~10bits now
    MI_U16 u16Pitch2bit;                      // pitch of 2bits frame buffer
    MI_SYS_FrameTileMode_e eFrameTileMode;  // Frame tile mode
    MI_SYS_FrameScanMode_e eFrameScanMode;  // Frame scan mode
} mi_vdec_FrameInfoExt_t;

typedef struct mi_vdec_DispInfo_s
{
    MI_U16 u16HorSize;
    MI_U16 u16VerSize;
    MI_U32 u32FrameRate;
    MI_U8  u8Interlace;
    MI_U16 u16CropRight;
    MI_U16 u16CropLeft;
    MI_U16 u16CropBottom;
    MI_U16 u16CropTop;
    MI_U16 u16Pitch;
} mi_vdec_DispInfo_t;

typedef struct
{
    MI_PHY phyLumaAddr;                           // frame buffer base + the start offset of current displayed luma data. Unit: byte.
    MI_PHY phyChromaAddr;                       // frame buffer base + the start offset of current displayed chroma data. Unit: byte.
    MI_U32 u32TimeStamp;                         // Time stamp(DTS, PTS) of current displayed frame. Unit: ms (todo: 90khz)
    MI_U32 u32IdL;                                    // low part of ID number
    MI_U32 u32IdH;                                   // high part of ID number
    MI_U16 u16Pitch;                                   // pitch
    MI_U16 u16Width;                                  // width
    MI_U16 u16Height;                                 // hight
    mi_vdec_FrameType_e eFrameType;    //< Frame type: I, P, B frame
    MI_SYS_FieldType_e eFieldType;         //< Field type: Top, Bottom, Both
} mi_vdec_FrameInfo_t;

typedef struct mi_vdec_DispFrame_s
{
    mi_vdec_FrameInfo_t stFrmInfo;    //< frame information
    MI_U32 u32PriData;                         //< firmware private data
    MI_U32 u32Idx;                               //< index used by apiVDEC to manage VDEC_DispQ[][]
    mi_vdec_FrameInfoExt_t stFrmInfoExt;        // Frame Info Extend
    mi_vdec_DbInfo_t stDbInfo;
    mi_vdec_DispInfo_t stDispInfo;
    MI_SYS_PixelFormat_e ePixelFrm;
    MI_U64 u64FastChnId;
} mi_vdec_DispFrame_t;
#endif

#ifndef MI_DIVP_FPGA_TEST
typedef struct mi_vdec_PushDataInfo_s
{
    MI_U32 u32ChnNum;
    MI_VDEC_CHN aVdecChn[VDEC_CHN_MAX];
    MI_U32 u32Ms;
} mi_vdec_PushDataInfo_t;
#endif

#define DBG_INFO(fmt, args...)      ({do{printf("\n [MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);/*usleep(1000000)*/;})

#define BIND_DIVP_VPE (0)
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

#define CHN_FRAME_NUM (5)
#define CHN_NUM (32)
#define CHN_BUF_DEPTH (2)
#define CHN_MAX_WIDTH (4096)
#define CHN_MAX_HEIGHT (2160)

MI_SYS_ChnPort_t gstVdecOutputPort[CHN_NUM];
MI_SYS_ChnPort_t gstDivpOutputPort[CHN_NUM];
MI_SYS_ChnPort_t gstDivpInputPort[CHN_NUM];
MI_SYS_ChnPort_t gstDispInputPort[CHN_NUM];

MI_SYS_BufConf_t gstInputBufConf[CHN_NUM];
MI_SYS_BufInfo_t gstInputBufInfo[CHN_NUM];
MI_SYS_BufInfo_t gstOutputBufInfo[CHN_NUM];
static MI_BOOL _bChnEnable[CHN_NUM];
MI_SYS_BUF_HANDLE ghInputBufHdl = 0;
MI_SYS_BUF_HANDLE ghOutputBufHdl = 0;
MI_U32 gu32CaseId = 0;
static pthread_t _gthrGetDivpOutputBuffer;

MI_U16 gu16FrameNumber = 0;
char gachInputFileName[128];
char gachOutputFileName[128];
MI_U16 gu16Lenth = 128;

static pthread_t _thrPushEsData;
static pthread_t _thrGetBuf[VDEC_CHN_MAX];
static MI_BOOL _bVdecChnEnable[VDEC_CHN_MAX];
static MI_BOOL _bPushData = FALSE;
static MI_S32 _hEsFile[VDEC_CHN_MAX];
static MI_VDEC_CodecType_e _aeCodecType[VDEC_CHN_MAX];
static MI_SYS_PixelFormat_e _geOutPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
#define MI_U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])

////////////////////////////////////////////////vdec related start//////////////////////////////////////////////
#ifndef MI_DIVP_FPGA_TEST
MI_U64 get_pts(MI_U32 u32FrameRate)
{
    if (0 == u32FrameRate)
    {
        return (MI_U64)(-1);
    }

    return (MI_U64)(90 * 1000 / u32FrameRate);
}

void *push_frame(void * pData)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hSysBuf;

    MI_U8 u8Index = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U32 u32Len = 0;
    MI_U32 u32FrameLen = 0;
    MI_U64 u64Pts = 0;
    MI_U8 au8Header[16] = {0};
    MI_U32 u32Pos = 0;
    struct timeval stTv;
    MI_VDEC_ChnStat_t stChnStat;
    mi_vdec_PushDataInfo_t * pstPushDataInfo = (mi_vdec_PushDataInfo_t *)pData;
    MI_U32 u32Ms = pstPushDataInfo->u32Ms;
    MI_VDEC_CHN VdecChn = 0;
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = VdecChn;
    stChnPort.u32PortId = 0;

    DBG_INFO("########## u32Ms = %d ##############\n\n", u32Ms);
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    memset(&stTv, 0, sizeof(stTv));
    stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufConf.u64TargetPts = 0;
    pu8Buf = malloc(VESFILE_READER_BATCH);
    while (1)
    {
        usleep(u32Ms * 1000);

        for(u8Index = 0; u8Index < pstPushDataInfo->u32ChnNum; u8Index ++)
        {
            VdecChn = pstPushDataInfo->aVdecChn[u8Index];
            stChnPort.u32ChnId = VdecChn;

            if(_bVdecChnEnable[VdecChn])
            {
                memset(au8Header, 0, 16);
                u32Pos = lseek(_hEsFile[VdecChn], 0L, SEEK_CUR);
                u32Len = read(_hEsFile[VdecChn], au8Header, 16);
                if(u32Len <= 0)
                {
                    lseek(_hEsFile[VdecChn], 0, SEEK_SET);
                    continue;
                }

                u32FrameLen = MI_U32VALUE(au8Header, 4);
                if(u32FrameLen > VESFILE_READER_BATCH)
                {
                    lseek(_hEsFile[VdecChn], 0, SEEK_SET);
                    continue;
                }

                u32Len = read(_hEsFile[VdecChn], pu8Buf, u32FrameLen);
                if(u32Len <= 0)
                {
                    lseek(_hEsFile[VdecChn], 0, SEEK_SET);
                    continue;
                }

                gettimeofday(&stTv, NULL);
                stBufConf.stRawCfg.u32Size = u32Len;
                stBufConf.u64TargetPts = stTv.tv_sec*1000 + stTv.tv_usec/1000;
                memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
                hSysBuf = MI_HANDLE_NULL;
                if (MI_SUCCESS != MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf, &stBufInfo, &hSysBuf, -1))
                {
                    lseek(_hEsFile[VdecChn], u32Pos, SEEK_SET);
                    continue;
                }

                memcpy(stBufInfo.stRawData.pVirAddr, pu8Buf, u32Len);

                stBufInfo.eBufType = E_MI_SYS_BUFDATA_RAW;
                stBufInfo.bEndOfStream = FALSE;
                stBufInfo.stRawData.bEndOfFrame = TRUE;
                stBufInfo.stRawData.u32ContentSize = u32Len;
                if (MI_SUCCESS != MI_SYS_ChnInputPortPutBuf(hSysBuf, &stBufInfo, false))
                {
                    lseek(_hEsFile[VdecChn], u32Pos, SEEK_SET);
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
                    continue;
                }

                memset(&stChnStat, 0x0, sizeof(stChnStat));
                MI_VDEC_GetChnStat(VdecChn, &stChnStat);
                if(((7264 != gu32CaseId) && (7265 != gu32CaseId) && (stChnStat.u32DecodeStreamFrames < 500))
                    || (((7264 == gu32CaseId) || (7265 == gu32CaseId) || (17 == gu32CaseId)) && (stChnStat.u32DecodeStreamFrames < 12)))
                {
                    if((17 != gu32CaseId) && (19 != gu32CaseId))
                    {
                        DBG_INFO("#####_hEsFile[%d] = %p. Frame Dec:%d\n", VdecChn, _hEsFile[VdecChn],stChnStat.u32DecodeStreamFrames);
                    }
                }
                else
                {
                    _bPushData = FALSE;
                    DBG_INFO("########### stop push_frame ###########\n");
                    return;
                }
            }
        }
    }
    DBG_INFO("\n\n");
    usleep(3000000);
    free(pu8Buf);
    return NULL;
}

void create_push_ES_data_thread(mi_vdec_PushDataInfo_t* pstPushDataInfo)
{
    if (pthread_create(&_thrPushEsData, NULL, push_frame, pstPushDataInfo))
    {
        DBG_INFO(" create Vdec push data thread fail.\n\n");
    }
    else
    {
        DBG_INFO("create Vdec push data thread OK.\n");
        usleep(100);
    }
}

void destroy_push_ES_data_thread(mi_vdec_PushDataInfo_t* pstPushDataInfo)
{
    MI_U8 u8Index = 0;
    MI_VDEC_CHN VdecChn = 0;
    DBG_INFO(" destroy_push_ES_data_thread.\n");
    for(u8Index = 0; u8Index < pstPushDataInfo->u32ChnNum; u8Index ++)
    {
        VdecChn = pstPushDataInfo->aVdecChn[u8Index];
        if (_bVdecChnEnable[VdecChn])
        {
            _bVdecChnEnable[VdecChn] = 0;
        }

        if (_hEsFile[VdecChn] >= 0)
        {
            close(_hEsFile[VdecChn]);
        }
    }

    if (_thrPushEsData)
    {
        DBG_INFO("wait push_frame thread end.\n");
        pthread_join(_thrPushEsData, NULL);
        DBG_INFO("push_frame thread deatroyed.\n");
    }
}
#endif
////////////////////////////////////////////////vdec related end//////////////////////////////////////////////

MI_S32 mi_divp_test_CreateChn(MI_DIVP_CHN DivpChn, MI_U32 u32MaxWidth, MI_U32 u32MaxHeight, MI_DIVP_DiType_e eDiType)
{
    MI_S32 s32Ret = -1;
    MI_DIVP_ChnAttr_t stAttr;
    memset(&stAttr, 0, sizeof(stAttr));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = eDiType;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u32MaxWidth;
    stAttr.stCropRect.u16Height = u32MaxHeight;
    stAttr.u32MaxWidth = u32MaxWidth;
    stAttr.u32MaxHeight = u32MaxHeight;

    s32Ret = MI_DIVP_CreateChn(DivpChn, &stAttr);
    DBG_INFO("\n s32Ret = %d", s32Ret);
    return s32Ret;
}

MI_S32 mi_divp_test_DestroyChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_DestroyChn(DivpChn), MI_SUCCESS);

    return s32Ret;
}

MI_S32 mi_divp_test_SetChnAttr(MI_DIVP_CHN DivpChn, MI_DIVP_ChnAttr_t* pstAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_SetChnAttr(DivpChn, pstAttr), MI_SUCCESS);

    return s32Ret;
}

MI_S32 mi_divp_test_GetChnAttr(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_ChnAttr_t stAttr;
    memset(&stAttr, 0, sizeof(stAttr));

    ExecFunc(MI_DIVP_GetChnAttr(DivpChn, &stAttr), MI_SUCCESS);
    DBG_INFO("DivpChn = %d, bHorMirror = %d, bVerMirror = %d, eDiType = %d, eRotateType = %d, eTnrLevel = %d, \n  u32MaxWidth = %u, u32MaxHeight = %u, crop(x, y, w, h) = (%u, %u, %u, %u). s32Ret = 0x%x.\n",
        DivpChn, stAttr.bHorMirror, stAttr.bVerMirror, stAttr.eDiType, stAttr.eRotateType, stAttr.eTnrLevel,
        stAttr.u32MaxWidth, stAttr.u32MaxHeight,  stAttr.stCropRect.u16X,
        stAttr.stCropRect.u16Y, stAttr.stCropRect.u16Width, stAttr.stCropRect.u16Height, s32Ret);

    return s32Ret;
}

MI_S32 mi_divp_test_StartChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_StartChn(DivpChn), MI_SUCCESS);
    _bChnEnable[DivpChn] = TRUE;
    return s32Ret;
}

MI_S32 mi_divp_test_StopChn(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_StopChn(DivpChn), MI_SUCCESS);
    _bChnEnable[DivpChn] = FALSE;
    return s32Ret;
}

MI_S32 mi_divp_test_SetOutputPortAttr(MI_DIVP_CHN DivpChn, MI_DIVP_OutputPortAttr_t* pstOutputPortAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_SetOutputPortAttr(DivpChn, pstOutputPortAttr), MI_SUCCESS);

    return s32Ret;
}

MI_S32 mi_divp_test_GetOutputPortAttr(MI_DIVP_CHN DivpChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));

    ExecFunc(MI_DIVP_GetOutputPortAttr(DivpChn, &stOutputPortAttr), MI_SUCCESS);
    DBG_INFO("DivpChn = %d. port Width = %u, Height = %u, pixelformat = %u, eCompressMode = %d, s32Ret = 0x%x\n",
        DivpChn, stOutputPortAttr.u32Width, stOutputPortAttr.u32Height, stOutputPortAttr.ePixelFormat, stOutputPortAttr.eCompMode, s32Ret);

    return s32Ret;
}

MI_S32 mi_divp_CreateChannel(MI_DIVP_CHN DivpChn, MI_U16 u16Width, MI_U16 u16Height, MI_DIVP_DiType_e eDiType)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    mi_divp_test_CreateChn(DivpChn, u16Width, u16Height, eDiType);
    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;//E_MI_SYS_PIXEL_FRAME_YUV_MST_420;//E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;//
    stOutputPortAttr.u32Width = u16Width;
    stOutputPortAttr.u32Height = u16Height;
    mi_divp_test_SetOutputPortAttr(DivpChn, &stOutputPortAttr);
    mi_divp_test_StartChn(DivpChn);

    MI_SYS_SetChnOutputPortDepth(&gstDivpOutputPort[DivpChn], 2, 5);
    return s32Ret;
}

MI_S32 mi_divp_DestroyChannel(MI_DIVP_CHN u32ChnId)
{

    MI_S32 s32Ret = MI_SUCCESS;
    ExecFunc(MI_DIVP_StopChn(u32ChnId), MI_SUCCESS);
    ExecFunc(MI_DIVP_DestroyChn(u32ChnId), MI_SUCCESS);
    return s32Ret;
}

MI_BOOL mi_divp_LoadFileByName(char* pchFileName, void* pVirtAddr, MI_U32 u32Size)
{
    FILE* pFile =NULL;
    MI_U32 u32ReadSize = 0;
    DBG_INFO("load pFileName = %s", pchFileName);

    pFile =fopen(pchFileName, "rb");
    if (pFile == NULL)
    {
        DBG_INFO("create file error\n");
        return;
    }

    u32ReadSize = fread(pVirtAddr, 1, u32Size, pFile);
    DBG_INFO("u32ReadSize = %d. \n", u32ReadSize);
    fclose(pFile);
}
MI_BOOL mi_divp_SaveFileByName(char* pchFileName, void* pVirtAddr, MI_U32 u32Size)
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

MI_BOOL mi_divp_SaveFile(char* pchFileName, void* pVirtAddr, MI_U32 u32Stride,
    MI_U16 u16Width, MI_U16 u16Height, MI_SYS_PixelFormat_e ePixelFormat)
{
    MI_BOOL bRet = FALSE;
    FILE* pFile =NULL;
    MI_U32 u32WriteSize = 0;
    MI_U32 u32LineNum = 0;
    MI_U32 u32BytesPerLine = 0;
    MI_U32 u32Index = 0;

    DBG_INFO("save pchFileName = %s, pVirtAddr = %p, u32Stride = %u, u16Width = %u, u16Height = %u, ePixelFormat = %u.\n \n", pchFileName,
         pVirtAddr, u32Stride, u16Width, u16Height, ePixelFormat);
    pFile =fopen(pchFileName, "w+");
    if (pFile == NULL)
    {
        DBG_INFO("create file error.\n");
        return bRet;
    }

    if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
    {
        u32LineNum = u16Height;
        u32BytesPerLine = u16Width * 2;
    }
    else if(E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == ePixelFormat)
    {
        u32LineNum = u16Height * 3 / 2;
        u32BytesPerLine = u16Width;
    }
    else if(E_MI_SYS_PIXEL_FRAME_YUV_MST_420 == ePixelFormat)
    {
        u32LineNum = u16Height;
        u32BytesPerLine = u16Width * 3 /2;
    }

    for (u32Index = 0; u32Index < u32LineNum; u32Index ++)
    {
        u32WriteSize += fwrite(pVirtAddr + u32Index * u32Stride, 1, u32BytesPerLine, pFile);
    }

    if(u32WriteSize == u32BytesPerLine * u32LineNum)
    {
        bRet = TRUE;
    }
    DBG_INFO("u32WriteSize = %d. bRet = %u\n", u32WriteSize, bRet);
    fclose(pFile);

    return bRet;
}

MI_U32 mi_divp_GetOutputFrameInfo(MI_U32 u32ChnId, MI_U64 u64FrameId, MI_SYS_BufInfo_t* pstBufInfo, char* PFileName)
{
    MI_U32 u32FrameSize = 0;
    if(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == pstBufInfo->stFrameData.ePixelFormat)
    {
        sprintf(PFileName, "/mnt/test_results/case%d_chn%d_frame%lld_%dx%d_stride%u_YUV422.raw", gu32CaseId,
              u32ChnId, u64FrameId, pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height, pstBufInfo->stFrameData.u32Stride[0]);
        u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height;
    }
    else if(E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == pstBufInfo->stFrameData.ePixelFormat)
    {
        sprintf(PFileName, "/mnt/test_results/case%d_chn%d_frame%lld_%dx%d_stride%u_YUV420.raw", gu32CaseId,
              u32ChnId, u64FrameId, pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height, pstBufInfo->stFrameData.u32Stride[0]);
        u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height * 3 / 2;
    }
    else if(E_MI_SYS_PIXEL_FRAME_YUV_MST_420 == pstBufInfo->stFrameData.ePixelFormat)
    {
        sprintf(PFileName, "/mnt/test_results/case%d_chn%d_frame%lld_%dx%d_stride%u_MST420.raw", gu32CaseId,
              u32ChnId, u64FrameId, pstBufInfo->stFrameData.u16Width, pstBufInfo->stFrameData.u16Height, pstBufInfo->stFrameData.u32Stride[0]);
        u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height;
    }

    return u32FrameSize;
}

void* mi_divp_GetDivpOutputFrameData(void* p)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hSysBufHdl;
    MI_U32 u32CheckSum = 0;
    MI_U32 u32ChnId = 0;
    char achOutputFilename[256] = {0};
    MI_U64 u64FrameNumber[CHN_NUM] = {0};
    MI_U32 u32LastCaseId = 0;
    MI_U32 u32FrameSize = 0;

#ifdef MI_DIVP_FPGA_TEST
    MI_U32 u32FrmNum = 0;
#endif
    DBG_INFO(".\n\n");
    while (1)
    {
        usleep(5 *1000);
        if ((17 != gu32CaseId) && (19 != gu32CaseId))
        {
            for(u32ChnId = 0; u32ChnId < CHN_NUM; u32ChnId ++)
            {
                if((15 != gu32CaseId) || ((15 == gu32CaseId) && (u32ChnId > 15)))
                {
                    if(_bChnEnable[u32ChnId])
                    {
                        hSysBufHdl = MI_HANDLE_NULL;
                        memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
                        memset(achOutputFilename, '\0', 256);
                        if (
#if BIND_DIVP_VPE
                            (23 != gu32CaseId) &&
                            (13 != gu32CaseId) &&
#endif
                            (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(&gstDivpOutputPort[u32ChnId], &stBufInfo, &hSysBufHdl)))
                        {
                            continue;
                        }
                        else
                        {
#ifdef MI_DIVP_FPGA_TEST
                            u32FrmNum ++;
                            DBG_INFO("u32FrameNumber = %u.\n", u32FrmNum);
#endif
                            DBG_INFO("\n @@ @@ @@ buffer phyaddr = 0x%llx, VirAddr[0] = 0x%lx, VirAddr[1] = 0x%lx, framesize = 0x%x. @@ @@ @@ \n",
                                  stBufInfo.stFrameData.phyAddr[0], stBufInfo.stFrameData.pVirAddr[0],  stBufInfo.stFrameData.pVirAddr[0],
                                  (stBufInfo.stFrameData.u32Stride[0] + stBufInfo.stFrameData.u32Stride[1]
                                  + stBufInfo.stFrameData.u32Stride[2])  * stBufInfo.stFrameData.u16Height);

                        }

#ifndef MI_DIVP_FPGA_TEST
                        if (stBufInfo.eBufType != E_MI_SYS_BUFDATA_FRAME)
                        {
                            DBG_INFO("error eBufType:%d\n", stBufInfo.eBufType);
                        }
                        else
                        {
                            u32FrameSize =mi_divp_GetOutputFrameInfo(u32ChnId, u64FrameNumber[u32ChnId], &stBufInfo, achOutputFilename);
                            if((gu32CaseId <= 7) || ((gu32CaseId >= 10) && (gu32CaseId <= 16)) || (gu32CaseId == 7264) || (gu32CaseId == 7265) )
                            {
                                mi_divp_SaveFile(achOutputFilename, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u32Stride[0],
                                      stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.ePixelFormat);
                            }
                            else if(((gu32CaseId == 9) || (gu32CaseId == 8264) || (gu32CaseId == 8265)) && (0 == u64FrameNumber[u32ChnId] % 3))
                            {
                                mi_divp_SaveFile(achOutputFilename, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u32Stride[0],
                                      stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.ePixelFormat);
                            }
                            else
                            {
                                DBG_INFO("don't save file: %s .\n\n",achOutputFilename);
                            }
                            u64FrameNumber[u32ChnId] ++;
                        }
#endif

                        if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(hSysBufHdl))
                        {
                            DBG_INFO("MI_SYS_ChnOutputPortPutBuf fail.\n\n");
                        }
                    }
                }
            }
        }

        if(0 == gu32CaseId)
        {
            memset(u64FrameNumber, 0, sizeof(u64FrameNumber));
        }
    }

    return NULL;
}

MI_BOOL mi_divp_GetDivpInputFrameDataYuv422(FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
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

MI_BOOL mi_divp_GetDivpInputFrameData420(FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
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

MI_BOOL mi_divp_GetDivpInputFrameData(MI_SYS_PixelFormat_e ePixelFormat, FILE *pInputFile, MI_SYS_BufInfo_t* pstBufInfo)
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
            if(!mi_divp_GetDivpInputFrameData420(pInputFile, pstBufInfo))
            {
                bRet = FALSE;
                DBG_INFO(" read input frame data failed!.\n");
            }
        }
        else if(ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
        {
            if(!mi_divp_GetDivpInputFrameDataYuv422(pInputFile, pstBufInfo))
            {
                bRet = FALSE;
                DBG_INFO(" read input frame data failed!.\n");
            }
        }
    }

    DBG_INFO(" bRet = %u.\n", bRet);
    return bRet;
}

MI_S32 mi_disp_hdmiInit(void)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;

    stInitParam.pCallBackArgs = NULL;
    stInitParam.pfnHdmiEventCallback = NULL;

    MI_HDMI_Init(&stInitParam);

    MI_HDMI_Open(eHdmi);
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
    stAttr.stAudioAttr.bEnableAudio = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel = 0;
    stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
    stAttr.stVideoAttr.bEnableVideo = TRUE;
    stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
    stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
    stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    MI_HDMI_SetAttr(eHdmi, &stAttr);

    MI_HDMI_Start(eHdmi);
    return MI_SUCCESS;
}

MI_S32 mi_divp_hdmiDeInit(void)
{
    MI_HDMI_DeviceId_e eHdmi = E_MI_HDMI_ID_0;
    MI_HDMI_Stop(eHdmi);
    MI_HDMI_Close(eHdmi);
    MI_HDMI_DeInit();
    return MI_SUCCESS;
}

MI_S32 MI_DIVP_TestCase_1(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = u16InputWidth;
    stAttr.u32MaxHeight = u16InputHeight;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 1 one frame start ############## \n");
    mi_divp_CreateChannel(u32ChnId, u16InputWidth, u16InputHeight, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

        ///get data from file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    usleep(4*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);

    DBG_INFO("########## test case 1 one frame end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_2(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BufInfo_t stOutputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 2 multi channel without crop start ############## \n");

    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
    }

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    while(u32FrameNumber < u16FrameNumber)
    {
        for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
        {
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
            {
                DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                        u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                        stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

                ///get data from file.
                if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
                {
                    fclose(pInputFile);
                    DBG_INFO(" read input frame data failed!.\n");
                    return MI_DIVP_ERR_FAILED;
                }

                //set buffer to divp
                if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
                {
                    u32FrameNumber ++;
                    usleep(2000000);
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
                }
                else
                {
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
                }
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
            }
        }
    }

    fclose(pInputFile);
    usleep(2*1000*1000);
    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        usleep(4*1000*1000);
        mi_divp_DestroyChannel(u32ChnId);
    }
    DBG_INFO("########## test case 2 multi channel  without crop end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_3(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_DIVP_CHN u32ChnId = 15;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_3D;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_MIDDLE;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = u16InputWidth;
    stAttr.u32MaxHeight = u16InputHeight*2;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 3 3D DI start ############## \n");

    mi_divp_CreateChannel(u32ChnId, u16InputWidth, u16InputHeight*2, E_MI_DIVP_DI_TYPE_3D);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
        stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
        usleep(100);
        DBG_INFO("get DI buffer buffer phyaddr = 0x%llx,\n\n",stInputBufInfo.stFrameData.phyAddr[0]);
        usleep(100);
    }
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n", pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    while(u32FrameNumber < u16FrameNumber)
    {
        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
        {
            DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                    u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                    stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

            if(u32FrameNumber % 2)
            {
                stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_TOP;
            }
            else
            {
                stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_BOTTOM;
            }
            stInputBufInfo.u64Pts = 0x12345600ULL + u32FrameNumber;
            DBG_INFO("eFieldType = %d, u64Pts = 0x%llx.\n", stInputBufInfo.stFrameData.eFieldType, stInputBufInfo.u64Pts);

            ///get data from input file.
            if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
            {
                fclose(pInputFile);
                DBG_INFO(" read input frame data failed!.\n");
                return MI_DIVP_ERR_FAILED;
            }

            //set buffer to divp
            if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
            {
                DBG_INFO("prepare to exit.\n");
                usleep(1* 1000 * 1000);
                DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
            }
        }
        else
        {
            DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
        }
        u32FrameNumber ++;
        usleep(1000000);
    }

    DBG_INFO("prepare to exit.\n");
    usleep(8* 1000 * 1000);
    fclose(pInputFile);

    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 3 3D DI end ##############\n\n");
    return s32Ret;
}

#ifdef MI_DIVP_FPGA_TEST
static mi_vdec_DispFrame_t* pstVdecFrameInfo = NULL;
#endif
MI_S32 MI_DIVP_TestCase_4(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
#ifdef MI_DIVP_FPGA_TEST
    MI_U32 u32FrameDataSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BufInfo_t stOutputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    MI_SYS_BUF_HANDLE hOutputBufHdl = 0;
    FILE *pInputFile = NULL;
    pchFileName = "/data/h265_320x256_mfdec.bin";
    void *pvirFramAddr = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    pstVdecFrameInfo = (mi_vdec_DispFrame_t*)malloc(sizeof(mi_vdec_DispFrame_t));
    usleep(100);
    DBG_INFO("### pstVdecFrameInfo = %p.#### sizeof(mi_vdec_DispFrame_t) = %u. \n", pstVdecFrameInfo, sizeof(mi_vdec_DispFrame_t));
    usleep(100);
    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 320;
    stAttr.stCropRect.u16Height = 256;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_META;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;

    DBG_INFO("########## test case 4 MFdec start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    fseek(pInputFile, 0, SEEK_END);//set seek start point
    u32FrameDataSize = ftell(pInputFile);//return position
    fseek(pInputFile, 0, SEEK_SET);//set seek start point
    stInputBufConf.stMetaCfg.u32Size = u32FrameDataSize;
    DBG_INFO(" input frame data u32FrameDataSize = %u.\n", u32FrameDataSize);

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {

        pstVdecFrameInfo->stFrmInfo.phyLumaAddr = stInputBufInfo.stMetaData.phyAddr;
        pstVdecFrameInfo->stFrmInfo.phyChromaAddr  = stInputBufInfo.stMetaData.phyAddr + 320 * 256;
        pstVdecFrameInfo->stFrmInfo.u16Width = 320;
        pstVdecFrameInfo->stFrmInfo.u16Height = 256;
        pstVdecFrameInfo->stFrmInfo.u16Pitch = 320 * 3 / 2;
        pstVdecFrameInfo->ePixelFrm = E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265;
        pstVdecFrameInfo->stFrmInfoExt.eFrameTileMode = E_MI_SYS_FRAME_TILE_MODE_32x16;
        pstVdecFrameInfo->stFrmInfo.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
        pstVdecFrameInfo->stFrmInfoExt.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

        pstVdecFrameInfo->stDbInfo.bDbEnable = TRUE;           // Decoder Buffer Enable
        pstVdecFrameInfo->stDbInfo.u8DbSelect = 0;          // Decoder Buffer select
        pstVdecFrameInfo->stDbInfo.bHMirror = FALSE;
        pstVdecFrameInfo->stDbInfo.bVMirror = FALSE;
        pstVdecFrameInfo->stDbInfo.bUncompressMode = FALSE;
        pstVdecFrameInfo->stDbInfo.bBypassCodecMode = FALSE;
        pstVdecFrameInfo->stDbInfo.u16StartX = 0;
        pstVdecFrameInfo->stDbInfo.u16StartY = 0;
        pstVdecFrameInfo->stDbInfo.u16HSize = 320;
        pstVdecFrameInfo->stDbInfo.u16VSize = 256;
        pstVdecFrameInfo->stDbInfo.phyDbBase = stInputBufInfo.stMetaData.phyAddr + 320 * 256 * 3 / 2;          // Decoder Buffer base addr
        pstVdecFrameInfo->stDbInfo.u16DbPitch = 320 * 3 / 2;        // Decoder Buffer pitch
        pstVdecFrameInfo->stDbInfo.u8DbMiuSel = 0;         // Decoder Buffer Miu select
        pstVdecFrameInfo->stDbInfo.phyLbAddr = 0;          // Lookaside buffer addr
        pstVdecFrameInfo->stDbInfo.u8LbSize = 0;          // Lookaside buffer size
        pstVdecFrameInfo->stDbInfo.u8LbTableId = 0;       // Lookaside buffer table Id

        stInputBufInfo.stMetaData.pVirAddr = pstVdecFrameInfo;
            DBG_INFO("stInputBufInfo.stMetaData.pVirAddr = %p. \n", stInputBufInfo.stMetaData.pVirAddr);

        //get buffer virtual address
        if(MI_SYS_Mmap(stInputBufInfo.stMetaData.phyAddr, u32FrameDataSize, &pvirFramAddr, TRUE))
        {
            DBG_INFO(" MI_SYS_Mmap failed!.\n");
        }
        else
        {
            DBG_INFO("pvirFramAddr = %p. \n", pvirFramAddr);
        }

        ///get data from file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }
        stInputBufInfo.bEndOfStream = TRUE;
        stInputBufInfo.stMetaData.eDataFromModule = E_MI_MODULE_ID_VDEC;

        MI_SYS_FlushInvCache(pvirFramAddr, u32FrameDataSize);

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    free(pstVdecFrameInfo);
    usleep(2*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 4 MFdec end ##############\n\n");
#endif
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_5(MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber)//resolution change
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_U32 u32InputId = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_U32 u32InputFrameSize[4] = {0};
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf[4];
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    char* pchInputFilename[4];
    FILE *pInputFile[4] = {NULL};

    //pchOutputFilename = malloc(128);
    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

#ifndef MI_DIVP_FPGA_TEST
    pchInputFilename[0] = "/mnt/YUV422/1920_1080_yuy422.yuv";
    pchInputFilename[1] = "/mnt/YUV422/1280_720_yuv422.yuv";
    pchInputFilename[2] = "/mnt/YUV422/640_480_yuy422.yuv";
    pchInputFilename[3] = "/mnt/YUV422/352_288_yuy422.yuv";
#else //need refine CYS
    pchInputFilename[0] = "/data/snake_128x192.yuv422";
    pchInputFilename[1] = "/data/fish_256x256.yuv422";
    pchInputFilename[2] = "/data/LbRw_352x288.yuv422";
    pchInputFilename[3] = "/data/triangle_256x128.yuv422";
#endif

    for(u32InputId = 0; u32InputId < 4; u32InputId ++)
    {
        pInputFile[u32InputId] =fopen(pchInputFilename[u32InputId ], "rb");
        if (pInputFile[u32InputId] == NULL)
        {
            DBG_INFO("open file error! file name = %s, u32InputId = %u.\n", pInputFile[u32InputId], u32InputId);
            return;
        }
        else
        {
            DBG_INFO("open file pFileName = %s, u32InputId = %u. pInputFile = %p\n", pchInputFilename[u32InputId], u32InputId, pInputFile[u32InputId]);
        }
    }

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 352;
    stAttr.stCropRect.u16Height = 288;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

#ifndef MI_DIVP_FPGA_TEST
    stInputBufConf[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[0].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[0].u64TargetPts = 0x12340000;
    stInputBufConf[0].stFrameCfg.u16Width = 1920;
    stInputBufConf[0].stFrameCfg.u16Height = 1080;
    stInputBufConf[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[1].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[1].u64TargetPts = 0x12340000;
    stInputBufConf[1].stFrameCfg.u16Width = 1280;
    stInputBufConf[1].stFrameCfg.u16Height = 720;
    stInputBufConf[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[2].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[2].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[2].u64TargetPts = 0x12340000;
    stInputBufConf[2].stFrameCfg.u16Width = 640;
    stInputBufConf[2].stFrameCfg.u16Height = 480;
    stInputBufConf[2].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[2].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[3].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[3].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[3].u64TargetPts = 0x12340000;
    stInputBufConf[3].stFrameCfg.u16Width = 352;
    stInputBufConf[3].stFrameCfg.u16Height = 288;
    stInputBufConf[3].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[3].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
#else //need refine CYS
    stInputBufConf[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[0].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[0].u64TargetPts = 0x12340000;
    stInputBufConf[0].stFrameCfg.u16Width = 128;
    stInputBufConf[0].stFrameCfg.u16Height = 192;
    stInputBufConf[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[1].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[1].u64TargetPts = 0x12340002;
    stInputBufConf[1].stFrameCfg.u16Width = 256;
    stInputBufConf[1].stFrameCfg.u16Height = 256;
    stInputBufConf[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[2].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[2].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[2].u64TargetPts = 0x12340004;
    stInputBufConf[2].stFrameCfg.u16Width = 352;
    stInputBufConf[2].stFrameCfg.u16Height = 288;
    stInputBufConf[2].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[2].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    stInputBufConf[3].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[3].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[3].u64TargetPts = 0x12340006;
    stInputBufConf[3].stFrameCfg.u16Width = 256;
    stInputBufConf[3].stFrameCfg.u16Height = 128;
    stInputBufConf[3].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[3].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
#endif
    DBG_INFO("########## test case 5 resolution change start ############## \n");

    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    while(u32FrameNumber < u16FrameNumber)
    {
        for(u32InputId = 0; u32InputId < 4; u32InputId ++)
        {
            stAttr.stCropRect.u16Width = stInputBufConf[u32InputId].stFrameCfg.u16Width;
            stAttr.stCropRect.u16Height = stInputBufConf[u32InputId].stFrameCfg.u16Height;
            DBG_INFO("crop W = %d, H = %d, u32InputId = %d",stAttr.stCropRect.u16Width, stAttr.stCropRect.u16Height, u32InputId);
            mi_divp_test_SetChnAttr(u32ChnId, &stAttr);

            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf[u32InputId],&stInputBufInfo,&hInputBufHdl, 1))
            {
                DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                        u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                        stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

                ///get data from file.
                if(!mi_divp_GetDivpInputFrameData(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV, pInputFile[u32InputId], &stInputBufInfo))
                {
                    for(u32InputId = 0; u32InputId < 4; u32InputId ++)
                    {
                        fclose(pInputFile[u32InputId]);
                    }
                    DBG_INFO(" read input frame data failed!.\n");
                    return MI_DIVP_ERR_FAILED;
                }

                //set buffer to divp
                if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
                {
                    u32FrameNumber ++;
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
                    usleep(2*1000*1000);
                }
                else
                {
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
                }
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
            }
            usleep(1000000);
        }
    }

    for(u32InputId = 0; u32InputId < 4; u32InputId ++)
    {
        fclose(pInputFile[u32InputId]);
    }

    usleep(8*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 5 resolution change end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_26(MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber)//I /P mode change
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_U32 u32InputFrameSize[2] = {0};
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;
    MI_U32 u32Num = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf[2];
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    char* pchInputFilename[5];
    FILE *pInputFile[5] = {NULL};
    MI_U16 u16Index = 0;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    pchInputFilename[0] = "/mnt/YUV422/1920_1080_yuy422.yuv";//1080P
    pchInputFilename[1] = "/mnt/YUV422/1920X540_yuy422.yuv";//1080I
    pchInputFilename[2] = "/mnt/YUV422/1920X540_yuy422.yuv";//1080I
    pchInputFilename[3] = "/mnt/YUV422/1920X540_yuy422.yuv";//1080I
    pchInputFilename[4] = "/mnt/YUV422/1920X540_yuy422.yuv";//1080I

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 352;
    stAttr.stCropRect.u16Height = 288;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[0].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[0].u64TargetPts = 0x12340000;
    stInputBufConf[0].stFrameCfg.u16Width = 736;
    stInputBufConf[0].stFrameCfg.u16Height = 240;
    stInputBufConf[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    u32InputFrameSize[0] = stInputBufConf[0].stFrameCfg.u16Width * stInputBufConf[0].stFrameCfg.u16Height* 3 / 2;

    stInputBufConf[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[1].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[1].u64TargetPts = 0x12340000;
    stInputBufConf[1].stFrameCfg.u16Width = 1920;
    stInputBufConf[1].stFrameCfg.u16Height = 540;
    stInputBufConf[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    u32InputFrameSize[1] = stInputBufConf[1].stFrameCfg.u16Width * stInputBufConf[1].stFrameCfg.u16Height * 2;

    DBG_INFO("########## test case 26 I  P mode change start ############## \n");

    mi_divp_CreateChannel(u32ChnId, 1920, 1080, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    for(u32Num = 0; u32Num < 5; u32Num ++)
    {
        pInputFile[u32Num] = fopen(pchInputFilename[u32Num], "rb");
        if(NULL == pInputFile[u32Num])
        {
             DBG_INFO("open file fail. FileName = %s, u32InputFrameSize[%d] = %d", pchInputFilename[u32Num], u32Num, u32InputFrameSize[u32Num]);
        }
        else
        {
             DBG_INFO("open file OK. FileName = %s, u32InputFrameSize[%d] = %d", pchInputFilename[u32Num], u32Num, u32InputFrameSize[u32Num]);
        }
    }

    while(u32FrameNumber < 8)
    {
        if(u32FrameNumber / 2 == 0)
        {
            stAttr.eDiType = E_MI_DIVP_DI_TYPE_3D;
        }
        else
        {
            stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
        }

        stAttr.stCropRect.u16Width = stInputBufConf[u32FrameNumber % 2].stFrameCfg.u16Width;
        stAttr.stCropRect.u16Height = stInputBufConf[u32FrameNumber % 2].stFrameCfg.u16Height;
        DBG_INFO("crop W = %d, H = %d",stAttr.stCropRect.u16Width, stAttr.stCropRect.u16Height);
        s32Ret = MI_DIVP_SetChnAttr(u32ChnId, &stAttr);

        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf[u32FrameNumber % 2],&stInputBufInfo,&hInputBufHdl, 1))
        {
            DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx, u32InputFrameSize[%d] = %d. @@\n",
                    u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                    stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2],u32InputFrameSize[u32FrameNumber/2]);

            if(u32FrameNumber % 2 == 0)
            {
                if(u32FrameNumber % 4)
                {
                    stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_TOP;
                }
                else
                {
                    stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_BOTTOM;
                }
                stInputBufInfo.u64Pts = 0x12345600ULL + u32FrameNumber;
            }

            ///get data from file.
            if(u32FrameNumber % 2 == 0)
            {
                if(!mi_divp_GetDivpInputFrameData(E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420, pInputFile[u16Index], &stInputBufInfo))
                {
                    for(u32Num = 0; u32Num < 5; u32Num ++)
                    {
                        fclose(pInputFile[u32Num]);
                    }
                    DBG_INFO(" read input frame data failed!.\n");
                    return MI_DIVP_ERR_FAILED;
                }
            }
            else
            {
                if(!mi_divp_GetDivpInputFrameData(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV, pInputFile[u16Index], &stInputBufInfo))
                {
                    for(u32Num = 0; u32Num < 5; u32Num ++)
                    {
                        fclose(pInputFile[u32Num]);
                    }
                    DBG_INFO(" read input frame data failed!.\n");
                    return MI_DIVP_ERR_FAILED;
                }
            }

            //set buffer to divp
            if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
            }
        }
        else
        {
            DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
        }
        u32FrameNumber ++;
        usleep(400*1000);
    }

    for(u32Num = 0; u32Num < 5; u32Num ++)
    {
        fclose(pInputFile[u32Num]);
    }

    usleep(4*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 26 I  P mode change end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_6(MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber)//I /P mode change
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_U32 u32InputFrameSize[4] = {0};
    MI_U32 u32Num = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf[4];
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    char* pchInputFilename[4];
    FILE *pInputFile[4] = {NULL};
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

#ifndef MI_DIVP_FPGA_TEST
    pchInputFilename[0] = "/mnt/YUV422/1920_1080_yuy422.yuv";//1080P
    pchInputFilename[1] = "/mnt/3DDI_736x240_YUV422_6_field.raw";//480I
    pchInputFilename[2] = "/mnt/YUV422/352_288_yuy422.yuv";//288P
    pchInputFilename[3] = "/mnt/3DDI_736x240_NV12_4field.raw";//480I
#else //need refine CYS
    pchInputFilename[0] = "/data/1920_1080_yuy422.yuv";//1080P
    pchInputFilename[1] = "/data/1920X540_yuy422.yuv";//1080I
    pchInputFilename[2] = "/data/720x288_yuv422.yuv";//576I
    pchInputFilename[3] = "/data/352_288_yuy422.yuv";//288P
#endif

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 352;
    stAttr.stCropRect.u16Height = 288;
    stAttr.u32MaxWidth = 1920;
    stAttr.u32MaxHeight = 1080;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf[0].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[0].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[0].u64TargetPts = 0x12340000;
    stInputBufConf[0].stFrameCfg.u16Width = 1920;
    stInputBufConf[0].stFrameCfg.u16Height = 1080;
    stInputBufConf[0].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[0].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    u32InputFrameSize[0] = stInputBufConf[0].stFrameCfg.u16Width * stInputBufConf[0].stFrameCfg.u16Height* 2;

    stInputBufConf[1].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[1].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[1].u64TargetPts = 0x12340000;
    stInputBufConf[1].stFrameCfg.u16Width = 736;
    stInputBufConf[1].stFrameCfg.u16Height = 240;
    stInputBufConf[1].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf[1].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    u32InputFrameSize[1] = stInputBufConf[1].stFrameCfg.u16Width * stInputBufConf[1].stFrameCfg.u16Height * 2;

    stInputBufConf[2].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[2].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[2].u64TargetPts = 0x12340000;
    stInputBufConf[2].stFrameCfg.u16Width = 352;
    stInputBufConf[2].stFrameCfg.u16Height = 288;
    stInputBufConf[2].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf[2].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    u32InputFrameSize[2] = stInputBufConf[2].stFrameCfg.u16Width * stInputBufConf[2].stFrameCfg.u16Height* 2;

    stInputBufConf[3].eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf[3].u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf[3].u64TargetPts = 0x12340000;
    stInputBufConf[3].stFrameCfg.u16Width = 736;
    stInputBufConf[3].stFrameCfg.u16Height = 240;
    stInputBufConf[3].stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf[3].stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    u32InputFrameSize[3] = stInputBufConf[3].stFrameCfg.u16Width * stInputBufConf[3].stFrameCfg.u16Height * 3 / 2;

    DBG_INFO("########## test case 6 I  P mode changestart ############## \n");

    mi_divp_CreateChannel(u32ChnId, 1920, 1080, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    for(u32Num = 0; u32Num < 4; u32Num ++)
    {
        pInputFile[u32Num] = fopen(pchInputFilename[u32Num], "rb");
        if(NULL == pInputFile[u32Num])
        {
             DBG_INFO("open file fail. FileName = %s, u32InputFrameSize[%d] = %d", pchInputFilename[u32Num], u32Num, u32InputFrameSize[u32Num]);
        }
        else
        {
             DBG_INFO("open file OK. FileName = %s, u32InputFrameSize[%d] = %d", pchInputFilename[u32Num], u32Num, u32InputFrameSize[u32Num]);
        }
    }

    while(u32FrameNumber < 16)
    {
        if((u32FrameNumber/4 == 1) || (u32FrameNumber/4 == 3))
        {
            stAttr.eDiType = E_MI_DIVP_DI_TYPE_3D;
        }
        else
        {
            stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
        }

#ifdef MI_DIVP_FPGA_TEST
        usleep(1000*1000);
#endif

        stAttr.stCropRect.u16Width = stInputBufConf[u32FrameNumber/4].stFrameCfg.u16Width;
        stAttr.stCropRect.u16Height = stInputBufConf[u32FrameNumber/4].stFrameCfg.u16Height;
        DBG_INFO("crop W = %d, H = %d",stAttr.stCropRect.u16Width, stAttr.stCropRect.u16Height);
        s32Ret = MI_DIVP_SetChnAttr(u32ChnId, &stAttr);

        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf[u32FrameNumber/4],&stInputBufInfo,&hInputBufHdl, 1))
        {
            DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx, u32InputFrameSize[%d] = %d. @@\n",
                    u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                    stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2],u32InputFrameSize[u32FrameNumber/4]);

            if((u32FrameNumber > 3 && u32FrameNumber < 8) || (u32FrameNumber > 11 && u32FrameNumber < 16))
            {
                if(u32FrameNumber % 2)
                {
                    stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_TOP;
                }
                else
                {
                    stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_BOTTOM;
                }
                stInputBufInfo.u64Pts = 0x12345600ULL + u32FrameNumber;
            }

            ///get data from file.
            if(!mi_divp_GetDivpInputFrameData(stInputBufInfo.stFrameData.ePixelFormat, pInputFile[u32FrameNumber/4], &stInputBufInfo))
            {
                for(u32Num = 0; u32Num < 4; u32Num ++)
                {
                    fclose(pInputFile[u32Num]);
                }
                DBG_INFO(" read input frame data failed!.\n");
                return MI_DIVP_ERR_FAILED;
            }

            //set buffer to divp
            if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
            }
        }
        else
        {
            DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
        }
        u32FrameNumber ++;
        usleep(2000*1000);
    }

    for(u32Num = 0; u32Num < 4; u32Num ++)
    {
        fclose(pInputFile[u32Num]);
    }

    usleep(4*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 6 I  P mode change end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_7(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_U32 u32InputFrameSize = 0;
    MI_U32 u32WriteSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;
    MI_U32 u32Num = 0;
    MI_U32 u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 10;
    stAttr.stCropRect.u16Y = 10;
    stAttr.stCropRect.u16Width = 64;
    stAttr.stCropRect.u16Height = 64;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;

    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 7 start ############## \n");

    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
    ///get data from file.
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("create file error. pFileName = %s\n", pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("open input file name : %s.\n", pchFileName);
    }

    while((10 + u32Num * (12 + 32) + 64 < u16InputHeight) && (10 + u32Num * (10 + 32) + 128 < u16InputWidth) && (u32Num < u16FrameNumber))
    {
        stAttr.stCropRect.u16X = 10 + u32Num * 32;
        stAttr.stCropRect.u16Y = 10 + u32Num * 32;
        stAttr.stCropRect.u16Width = 128 + u32Num * 10;
        stAttr.stCropRect.u16Height = 64 + u32Num * 12;
        s32Ret = MI_DIVP_SetChnAttr(u32ChnId, &stAttr);

        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
        {
            DBG_INFO("@@ nChnId = %u, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                    u32ChnId, stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                    stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

            ///get data from file.
            if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
            {
                fclose(pInputFile);
                DBG_INFO(" read input frame data failed!.\n");
                return MI_DIVP_ERR_FAILED;
            }

            //set buffer to divp
            if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
            }
        }
        else
        {
            DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
        }
        usleep(2000000);
        u32Num ++;
    }

    fclose(pInputFile);
    usleep(10*1000*1000);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 7 end ##############\n\n");
    return s32Ret;
}

#ifndef MI_DIVP_FPGA_TEST
MI_S32 MI_DIVP_TestCase_7_264()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN VdecChn = 0;
    MI_U32 u32Num = 0;
    MI_DIVP_CHN u32ChnId = 0;
    char* pFileName = "/mnt/720p30H264.es";
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;

    mi_vdec_PushDataInfo_t stPushDataInfo;
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.aVdecChn[0] = VdecChn;
    stPushDataInfo.u32ChnNum = 1;
    stPushDataInfo.u32Ms = 1000;
    DBG_INFO("########## test case 7_264 start ############## \n");

    _hEsFile[VdecChn] = open(pFileName, O_RDONLY, 0);
    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1280;
    stAttr.stCropRect.u16Height = 720;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputPortAttr.u32Width = 1280;
    stOutputPortAttr.u32Height = 720;

    memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H264;

    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
    stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize = 1024 * 1024;
    stVdecChnAttr.u32PicWidth = 1280;
    stVdecChnAttr.u32PicHeight = 720;
    stVdecChnAttr.u32Priority = 0;


    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
    {
        s32Ret = MI_VDEC_StartChn(VdecChn);
        _bVdecChnEnable[VdecChn] = 1;
    }
    else
    {
        DBG_INFO("MI_VDEC_CreateChn failed. \n\n");
    }

    MI_SYS_BindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0], 30, 30);

    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while((u32Num * 20 + u32Num * 32 + 64 < 720) && (u32Num * 20 + u32Num * 64 + 128 < 1280) && (u32Num < 20))
    {
        stAttr.stCropRect.u16X = 10 + u32Num * 32;
        stAttr.stCropRect.u16Y = 10 + u32Num * 32;
        stAttr.stCropRect.u16Width = 128 + u32Num * 12;
        stAttr.stCropRect.u16Height = 64 + u32Num * 10;

        stOutputPortAttr.u32Width = stAttr.stCropRect.u16Width;
        stOutputPortAttr.u32Height = stAttr.stCropRect.u16Height;

        s32Ret = MI_DIVP_SetChnAttr(u32ChnId, &stAttr);
        s32Ret = MI_DIVP_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
        usleep(1000000);
        u32Num ++;
    }

    while(_bPushData)
    {
        usleep(50000);
    }

    destroy_push_ES_data_thread(&stPushDataInfo);
    MI_SYS_UnBindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0]);
    usleep(5000000);
    MI_VDEC_StopChn(VdecChn);
    MI_VDEC_DestroyChn(VdecChn);
    mi_divp_DestroyChannel(u32ChnId);

    DBG_INFO("########## test case 7_264 end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_7_265()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN VdecChn = 0;
    char InputCmd[256] = {0};
    MI_U32 u32Num = 0;
    MI_DIVP_CHN u32ChnId = 0;
    char* pFileName = "/mnt/1080p30H265.es";
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;

    mi_vdec_PushDataInfo_t stPushDataInfo;
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.aVdecChn[0] = VdecChn;
    stPushDataInfo.u32ChnNum = 1;
    stPushDataInfo.u32Ms = 1000;
    DBG_INFO("########## test case 7_265 start ############## \n");
    _hEsFile[VdecChn] = open(pFileName, O_RDONLY, 0);

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1920;
    stAttr.stCropRect.u16Height = 1080;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputPortAttr.u32Width = 1920;
    stOutputPortAttr.u32Height = 1080;

    memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H265;

    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
    stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize = 1024 * 1024;
    stVdecChnAttr.u32PicWidth = 1920;
    stVdecChnAttr.u32PicHeight = 1080;
    stVdecChnAttr.u32Priority = 0;

    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
    {
        s32Ret = MI_VDEC_StartChn(VdecChn);
        _bVdecChnEnable[VdecChn] = 1;
    }
    else
    {
        DBG_INFO("MI_VDEC_CreateChn failed. \n\n");
    }

    MI_SYS_BindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0], 30, 30);

    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while((u32Num * 20 + u32Num * 32 + 64 < 1080) && (u32Num * 20 + u32Num * 64 + 128 < 1920) && (u32Num < 20))
    {
        stAttr.stCropRect.u16X = 10 + u32Num * 32;
        stAttr.stCropRect.u16Y = 10 + u32Num * 32;
        stAttr.stCropRect.u16Width = 128 + u32Num * 10;
        stAttr.stCropRect.u16Height = 64 + u32Num * 12;

        stOutputPortAttr.u32Width = stAttr.stCropRect.u16Width;
        stOutputPortAttr.u32Height = stAttr.stCropRect.u16Height;

        s32Ret = MI_DIVP_SetChnAttr(u32ChnId, &stAttr);
        s32Ret = MI_DIVP_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
        usleep(1000000);
        u32Num ++;
    }
    //usleep(5000000);

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);
    usleep(5000000);
    MI_SYS_UnBindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0]);
    MI_VDEC_StopChn(VdecChn);
    MI_VDEC_DestroyChn(VdecChn);
    mi_divp_DestroyChannel(u32ChnId);

    DBG_INFO("########## test case 7_265 end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_8_264()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN VdecChn = 0;
    char* pFileName = "/mnt/ES/720p30H264.es";
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_S32 u32ChnId = 0;
    mi_vdec_PushDataInfo_t stPushDataInfo;
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.aVdecChn[0] = VdecChn;
    stPushDataInfo.u32ChnNum = 1;
    stPushDataInfo.u32Ms = 2000;
    DBG_INFO("########## test case 8_264 start ############## \n");
    _hEsFile[VdecChn] = open(pFileName, O_RDONLY, 0);

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1280;
    stAttr.stCropRect.u16Height = 720;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputPortAttr.u32Width = 1280;
    stOutputPortAttr.u32Height = 720;

    memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H264;

    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
    stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize = 1024 * 1024;
    stVdecChnAttr.u32PicWidth = 1280;
    stVdecChnAttr.u32PicHeight = 720;
    stVdecChnAttr.u32Priority = 0;

    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
    {
        s32Ret = MI_VDEC_StartChn(VdecChn);
        _bVdecChnEnable[VdecChn] = 1;
    }
    else
    {
        DBG_INFO("MI_VDEC_CreateChn failed. \n\n");
    }

    MI_SYS_BindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0], 30, 30);

    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);
    MI_SYS_UnBindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0]);
    usleep(5000000);
    MI_VDEC_StopChn(VdecChn);
    MI_VDEC_DestroyChn(VdecChn);
    mi_divp_DestroyChannel(u32ChnId);

    DBG_INFO("########## test case 8_264 end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_8_265()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_CHN VdecChn = 0;
    char InputCmd[256] = { 0 };
    char* pFileName = "/mnt/ES/1080p30H265.es";
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_S32 u32ChnId = 0;
    mi_vdec_PushDataInfo_t stPushDataInfo;
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.aVdecChn[0] = VdecChn;
    stPushDataInfo.u32ChnNum = 1;
    stPushDataInfo.u32Ms = 2000;
    DBG_INFO("########## test case 8_265 start ############## \n");
    _hEsFile[VdecChn] = open(pFileName, O_RDONLY, 0);

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1920;
    stAttr.stCropRect.u16Height = 1080;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputPortAttr.u32Width = 1280;
    stOutputPortAttr.u32Height = 720;

    memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H265;

    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
    stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize = 1024 * 1024;
    stVdecChnAttr.u32PicWidth = 1920;
    stVdecChnAttr.u32PicHeight = 1080;
    stVdecChnAttr.u32Priority = 0;

    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
    {
        s32Ret = MI_VDEC_StartChn(VdecChn);
        _bVdecChnEnable[VdecChn] = 1;
    }
    else
    {
        DBG_INFO("MI_VDEC_CreateChn failed. \n\n");
    }

    MI_SYS_BindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0], 30, 30);

    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);
    usleep(5000000);
    MI_SYS_UnBindChnPort(&gstVdecOutputPort[0], &gstDivpInputPort[0]);
    MI_VDEC_StopChn(VdecChn);
    MI_VDEC_DestroyChn(VdecChn);
    mi_divp_DestroyChannel(u32ChnId);

    DBG_INFO("########## test case 8_265 end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_9(MI_U32 u32ChnNum,MI_VDEC_CodecType_e eCodecType1,
    MI_U32 u32Resolution1, MI_VDEC_CodecType_e eCodecType2, MI_U32 u32Resolution2)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Ret1 = E_MI_ERR_FAILED;
    MI_S32 s32Ret2 = E_MI_ERR_FAILED;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN VdecChn = 0;
    MI_U8 u8Index = 0;
    MI_U32 u32ChnId = 0;
    MI_U32 u32Resolution = 720;
    MI_DIVP_ChnAttr_t stAttr;
    char InputCmd[256] = {0};
    char aFileName[256] = {0};
    mi_vdec_PushDataInfo_t stPushDataInfo;
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.u32Ms = 2000;
    stPushDataInfo.u32ChnNum = u32ChnNum;

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1280;
    stAttr.stCropRect.u16Height = 720;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputPortAttr.u32Width = 1920;
    stOutputPortAttr.u32Height = 1080;
    DBG_INFO("########## test case 9 H264 and H265  4 channel start ############## \n");
    DBG_INFO(" u32ChnNum = %u, eCodecType1 = %u,u32Resolution1 = %u, eCodecType2 = %u, u32Resolution2 = %u.\n",
        u32ChnNum, eCodecType1 ,u32Resolution1, eCodecType2, u32Resolution2);
    for(u8Index = 0; u8Index < u32ChnNum; u8Index ++)
    {
        u32ChnId = u8Index;
        VdecChn = u8Index;
        memset(aFileName, '\0', sizeof(aFileName));
        memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
        stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnAttr.u32BufSize = 1024 * 1024;
        stVdecChnAttr.u32Priority = 0;

        if(0 == u8Index % 2)
        {
            stVdecChnAttr.eCodecType = eCodecType1;
            u32Resolution = u32Resolution1;
        }
        else
        {
            stVdecChnAttr.eCodecType = eCodecType2;
            u32Resolution = u32Resolution2;
        }

        if(720 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }
        else if(1080 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1920;
            stVdecChnAttr.u32PicHeight = 1080;
            stAttr.stCropRect.u16Width = 1920;
            stAttr.stCropRect.u16Height = 1080;
        }
        else if(480 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 640;
            stVdecChnAttr.u32PicHeight = 480;
            stAttr.stCropRect.u16Width = 640;
            stAttr.stCropRect.u16Height = 480;
        }
        else
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }

        if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H264.es");
        }
        else if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H265.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H264.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H265.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H264.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H265.es");
        }

        mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
        _hEsFile[VdecChn] = open(aFileName, O_RDONLY, 0);
        if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
        {
            DBG_INFO("MI_VDEC_CreateChn VdecChn = %d. \n\n", VdecChn);
            s32Ret = MI_VDEC_StartChn(VdecChn);
            if(MI_SUCCESS != s32Ret)
            {
                DBG_INFO("MI_VDEC_StartChn failed. \n\n");
                continue;
            }
            else
            {
                _bVdecChnEnable[VdecChn] = 1;
                MI_SYS_SetChnOutputPortDepth(&gstVdecOutputPort[VdecChn], 2, 5);
            }
        }
        else
        {
            DBG_INFO("MI_VDEC_CreateChn failed.  VdecChn = %d. \n\n", VdecChn);
        }

        MI_SYS_BindChnPort(&gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index], 30, 30);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

        stPushDataInfo.aVdecChn[u8Index] = VdecChn;
        DBG_INFO("open file : %s\n", aFileName);
    }
    DBG_INFO("u32ChnNum = %u \n\n", u32ChnNum);
    usleep(2000000);
    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);

    for (u8Index = 0; u8Index < u32ChnNum; u8Index++)
    {
        close(_hEsFile[u8Index]);
        MI_SYS_UnBindChnPort( &gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index]);
        MI_VDEC_StopChn(u8Index);
        MI_VDEC_DestroyChn(u8Index);
    }

    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 9 H264 and H265  4 channel end ##############\n\n");
    return s32Ret;
}


MI_S32 MI_DIVP_TestCase_19(MI_U32 u32ChnNum,MI_VDEC_CodecType_e eCodecType1,
    MI_U32 u32Resolution1, MI_VDEC_CodecType_e eCodecType2, MI_U32 u32Resolution2)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Ret1 = E_MI_ERR_FAILED;
    MI_S32 s32Ret2 = E_MI_ERR_FAILED;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN VdecChn = 0;
    MI_U8 u8Index = 0;
    MI_U32 u32ChnId = 0;
    MI_U32 u32Resolution = 720;
    MI_DIVP_ChnAttr_t stAttr;
    char InputCmd[256] = {0};
    char aFileName[256] = {0};
    mi_vdec_PushDataInfo_t stPushDataInfo;
    MI_DISP_DEV DispDev = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    MI_U32 u32PortId =0;
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;

    MI_U16 u16DivpOutWidth = 640;
    MI_U16 u16DivpOutHeight = 480;
    MI_SYS_PixelFormat_e eDivpOutPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.u32Ms = 30;
    stPushDataInfo.u32ChnNum = u32ChnNum;

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1280;
    stAttr.stCropRect.u16Height = 720;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eDivpOutPxlFmt;
    stOutputPortAttr.u32Width = u16DivpOutWidth;
    stOutputPortAttr.u32Height = u16DivpOutHeight;
    DBG_INFO("########## test case 19 Vdec->Divp->Disp channel start ############## \n");
    DBG_INFO(" u32ChnNum = %u, eCodecType1 = %u,u32Resolution1 = %u, eCodecType2 = %u, u32Resolution2 = %u.\n",
        u32ChnNum, eCodecType1 ,u32Resolution1, eCodecType2, u32Resolution2);

    if(u32ChnNum <= 1)
    {
        u16Avg = 1;
    }
    else if(u32ChnNum <= 4)
    {
        u16Avg = 2;
    }
    else if(u32ChnNum <= 9)
    {
        u16Avg = 3;
    }
    else if(u32ChnNum <= 16)
    {
        u16Avg = 4;
    }
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = u16DivpOutWidth * u16Avg;
    stLayerAttr.stVidLayerSize.u16Height= u16DivpOutHeight * u16Avg;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    stLayerAttr.ePixFormat = eDivpOutPxlFmt;

    MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    MI_DISP_Enable(DispDev);

    mi_disp_hdmiInit();
    MI_HDMI_Attr_t stHdmiAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);

    MI_DISP_BindVideoLayer(DispLayer, DispDev);
    MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
    MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    MI_DISP_EnableVideoLayer(DispLayer);

    MI_DISP_SetVideoLayerAttrBegin(DispLayer);
    for(u32PortId = 0; u32PortId < u32ChnNum; u32PortId ++)
    {
        stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
        stDispChnInputPort[u32PortId].u32DevId = 0;
        stDispChnInputPort[u32PortId].u32ChnId = 0;
        stDispChnInputPort[u32PortId].u32PortId = u32PortId;

        stInputPortAttr[u32PortId].stDispWin.u16Width = u16DivpOutWidth;
        stInputPortAttr[u32PortId].stDispWin.u16Height = u16DivpOutHeight;

        u16Row = u32PortId / u16Avg;
        u16Column = u32PortId - u16Row * u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16X = u16Column * u16DivpOutWidth;
        stInputPortAttr[u32PortId].stDispWin.u16Y = u16Row * u16DivpOutHeight;

        DBG_INFO(">>>>>>>  u32PortId = %u, disp win(%u, %u, %u, %u) <<<<<<<<<< \n", u32PortId,stInputPortAttr[u32PortId].stDispWin.u16X,
            stInputPortAttr[u32PortId].stDispWin.u16Y, stInputPortAttr[u32PortId].stDispWin.u16Width, stInputPortAttr[u32PortId].stDispWin.u16Height);
        MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_GetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]);
    }
    MI_DISP_SetVideoLayerAttrEnd(DispLayer);

    for(u8Index = 0; u8Index < u32ChnNum; u8Index ++)
    {
        u32ChnId = u8Index;
        VdecChn = u8Index;
        memset(aFileName, '\0', sizeof(aFileName));
        memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
        stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnAttr.u32BufSize = 1024 * 1024;
        stVdecChnAttr.u32Priority = 0;

        if(0 == u8Index % 2)
        {
            stVdecChnAttr.eCodecType = eCodecType1;
            u32Resolution = u32Resolution1;
        }
        else
        {
            stVdecChnAttr.eCodecType = eCodecType2;
            u32Resolution = u32Resolution2;
        }

        if(720 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }
        else if(1080 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1920;
            stVdecChnAttr.u32PicHeight = 1080;
            stAttr.stCropRect.u16Width = 1920;
            stAttr.stCropRect.u16Height = 1080;
        }
        else if(480 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 640;
            stVdecChnAttr.u32PicHeight = 480;
            stAttr.stCropRect.u16Width = 640;
            stAttr.stCropRect.u16Height = 480;
        }
        else
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }

        if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H264.es");
        }
        else if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H265.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H264.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H265.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H264.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H265.es");
        }

        _hEsFile[VdecChn] = open(aFileName, O_RDONLY, 0);
        mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
        if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
        {
            DBG_INFO("MI_VDEC_CreateChn VdecChn = %d. \n\n", VdecChn);
            s32Ret = MI_VDEC_StartChn(VdecChn);
            if(MI_SUCCESS != s32Ret)
            {
                DBG_INFO("MI_VDEC_StartChn failed. \n\n");
                continue;
            }
            else
            {
                _bVdecChnEnable[VdecChn] = 1;
                MI_SYS_SetChnOutputPortDepth(&gstVdecOutputPort[VdecChn], 2, 5);
            }
        }
        else
        {
            DBG_INFO("MI_VDEC_CreateChn failed.  VdecChn = %d. \n\n", VdecChn);
        }

        MI_SYS_BindChnPort(&gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index], 30, 30);
        MI_SYS_BindChnPort(&gstDivpOutputPort[u8Index], &gstDispInputPort[u8Index], 30, 30);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

        stPushDataInfo.aVdecChn[u8Index] = VdecChn;
        DBG_INFO("open file : %s\n", aFileName);
    }
    DBG_INFO("u32ChnNum = %u \n\n", u32ChnNum);
    usleep(2000000);
    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);

    for (u8Index = 0; u8Index < u32ChnNum; u8Index++)
    {
        close(_hEsFile[u8Index]);
        MI_SYS_UnBindChnPort( &gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index]);
        MI_SYS_UnBindChnPort(&gstDivpOutputPort[u8Index], &gstDispInputPort[u8Index]);
        MI_VDEC_StopChn(u8Index);
        MI_VDEC_DestroyChn(u8Index);
        mi_divp_DestroyChannel(u8Index);
        MI_DISP_DisableInputPort(DispLayer, u32InputPort[u8Index]);
    }
    MI_DISP_DisableVideoLayer(DispLayer);
    MI_DISP_UnBindVideoLayer(DispLayer, DispDev);
    mi_divp_hdmiDeInit();
    MI_DISP_Disable(DispDev);

    DBG_INFO("########## test case 19 Vdec->Divp->Disp end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_17(MI_U32 u32ChnNum,MI_VDEC_CodecType_e eCodecType1,
    MI_U32 u32Resolution1, MI_VDEC_CodecType_e eCodecType2, MI_U32 u32Resolution2)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Ret1 = E_MI_ERR_FAILED;
    MI_S32 s32Ret2 = E_MI_ERR_FAILED;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN VdecChn = 0;
    MI_U8 u8Index = 0;
    MI_U32 u32ChnId = 0;
    MI_U32 u32Resolution = 720;
    MI_DIVP_ChnAttr_t stAttr;
    char InputCmd[256] = {0};
    char aFileName[256] = {0};
    mi_vdec_PushDataInfo_t stPushDataInfo;
    MI_DISP_DEV DispDev = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    MI_U32 u32PortId =0;
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;

    MI_U16 u16DivpOutWidth = 640;
    MI_U16 u16DivpOutHeight = 480;
    MI_SYS_PixelFormat_e eDivpOutPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    memset(&stPubAttr, 0, sizeof(stPubAttr));
    memset(&stPushDataInfo, 0, sizeof(stPushDataInfo));
    stPushDataInfo.u32Ms = 1000;
    stPushDataInfo.u32ChnNum = u32ChnNum;

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = 1280;
    stAttr.stCropRect.u16Height = 720;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eDivpOutPxlFmt;
    stOutputPortAttr.u32Width = u16DivpOutWidth;
    stOutputPortAttr.u32Height = u16DivpOutHeight;
    DBG_INFO("########## test case 17 Vdec->Divp->Disp and capture start ############## \n");
    DBG_INFO(" u32ChnNum = %u, eCodecType1 = %u,u32Resolution1 = %u, eCodecType2 = %u, u32Resolution2 = %u.\n",
        u32ChnNum, eCodecType1 ,u32Resolution1, eCodecType2, u32Resolution2);

    if(u32ChnNum <= 1)
    {
        u16Avg = 1;
    }
    else if(u32ChnNum <= 4)
    {
        u16Avg = 2;
    }
    else if(u32ChnNum <= 9)
    {
        u16Avg = 3;
    }
    else if(u32ChnNum <= 16)
    {
        u16Avg = 4;
    }
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = u16DivpOutWidth * u16Avg;
    stLayerAttr.stVidLayerSize.u16Height= u16DivpOutHeight * u16Avg;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    stLayerAttr.ePixFormat = eDivpOutPxlFmt;

    MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    MI_DISP_Enable(DispDev);

    mi_disp_hdmiInit();
    MI_HDMI_Attr_t stHdmiAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);

    MI_DISP_BindVideoLayer(DispLayer, DispDev);
    MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
    MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    MI_DISP_EnableVideoLayer(DispLayer);

    MI_DISP_SetVideoLayerAttrBegin(DispLayer);
    for(u32PortId = 0; u32PortId < u32ChnNum; u32PortId ++)
    {
        stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
        stDispChnInputPort[u32PortId].u32DevId = 0;
        stDispChnInputPort[u32PortId].u32ChnId = 0;
        stDispChnInputPort[u32PortId].u32PortId = u32PortId;

        stInputPortAttr[u32PortId].stDispWin.u16Width = u16DivpOutWidth;
        stInputPortAttr[u32PortId].stDispWin.u16Height = u16DivpOutHeight;

        u16Row = u32PortId / u16Avg;
        u16Column = u32PortId - u16Row * u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16X = u16Column * u16DivpOutWidth;
        stInputPortAttr[u32PortId].stDispWin.u16Y = u16Row * u16DivpOutHeight;

        DBG_INFO(">>>>>>>  u32PortId = %u, disp win(%u, %u, %u, %u) <<<<<<<<<< \n", u32PortId,stInputPortAttr[u32PortId].stDispWin.u16X,
            stInputPortAttr[u32PortId].stDispWin.u16Y, stInputPortAttr[u32PortId].stDispWin.u16Width, stInputPortAttr[u32PortId].stDispWin.u16Height);
        MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_GetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]);
    }
    MI_DISP_SetVideoLayerAttrEnd(DispLayer);

    for(u8Index = 0; u8Index < u32ChnNum; u8Index ++)
    {
        u32ChnId = u8Index;
        VdecChn = u8Index;
        memset(aFileName, '\0', sizeof(aFileName));
        memset(&stVdecChnAttr, 0x0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 7;
        stVdecChnAttr.eVideoMode = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnAttr.u32BufSize = 1024 * 1024;
        stVdecChnAttr.u32Priority = 0;

        if(0 == u8Index % 2)
        {
            stVdecChnAttr.eCodecType = eCodecType1;
            u32Resolution = u32Resolution1;
        }
        else
        {
            stVdecChnAttr.eCodecType = eCodecType2;
            u32Resolution = u32Resolution2;
        }

        if(720 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }
        else if(1080 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 1920;
            stVdecChnAttr.u32PicHeight = 1080;
            stAttr.stCropRect.u16Width = 1920;
            stAttr.stCropRect.u16Height = 1080;
        }
        else if(480 == u32Resolution)
        {
            stVdecChnAttr.u32PicWidth = 640;
            stVdecChnAttr.u32PicHeight = 480;
            stAttr.stCropRect.u16Width = 640;
            stAttr.stCropRect.u16Height = 480;
        }
        else
        {
            stVdecChnAttr.u32PicWidth = 1280;
            stVdecChnAttr.u32PicHeight = 720;
            stAttr.stCropRect.u16Width = 1280;
            stAttr.stCropRect.u16Height = 720;
        }

        if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H264.es");
        }
        else if((720 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/720p30H265.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H264.es");
        }
        else if((1080 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/1080p30H265.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H264 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H264.es");
        }
        else if((480 == u32Resolution) && (E_MI_VDEC_CODEC_TYPE_H265 == stVdecChnAttr.eCodecType))
        {
            sprintf(aFileName, "/mnt/480p30H265.es");
        }

        mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
        _hEsFile[VdecChn] = open(aFileName, O_RDONLY, 0);
        if(MI_SUCCESS == MI_VDEC_CreateChn(VdecChn, &stVdecChnAttr))
        {
            DBG_INFO("MI_VDEC_CreateChn VdecChn = %d. \n\n", VdecChn);
            s32Ret = MI_VDEC_StartChn(VdecChn);
            if(MI_SUCCESS != s32Ret)
            {
                DBG_INFO("MI_VDEC_StartChn failed. \n\n");
                continue;
            }
            else
            {
                _bVdecChnEnable[VdecChn] = 1;
                MI_SYS_SetChnOutputPortDepth(&gstVdecOutputPort[VdecChn], 2, 5);
            }
        }
        else
        {
            DBG_INFO("MI_VDEC_CreateChn failed.  VdecChn = %d. \n\n", VdecChn);
        }

        MI_SYS_BindChnPort(&gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index], 30, 30);
        MI_SYS_BindChnPort(&gstDivpOutputPort[u8Index], &gstDispInputPort[u8Index], 30, 30);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

        stPushDataInfo.aVdecChn[u8Index] = VdecChn;
        DBG_INFO("open file : %s\n", aFileName);
    }
    DBG_INFO("u32ChnNum = %u \n\n", u32ChnNum);
    usleep(2000000);
    create_push_ES_data_thread(&stPushDataInfo);
    _bPushData = TRUE;

    usleep(8*1000*1000);
    {
        MI_U8 u8CapScreen = 0;
        MI_U32 u32FrameSize = 720 * 1280 * 2;
        char* pchFileName = "/mnt/test_results/cap.raw";
        MI_DISP_VideoFrame_t stCapFrame;
        memset(&stCapFrame, 0, sizeof(MI_DISP_VideoFrame_t));

        DBG_INFO(" prepare to get capture  frame.\n");
        usleep(1*1000*1000);

        if (MI_DISP_GetScreenFrame(0, &stCapFrame) != MI_DISP_SUCCESS)
        {
            DBG_INFO("capture Screen failed!\n");
        }
        DBG_INFO("Screen: W = %d, H = %d, phyAddr = 0x%llx, pavirAddr = %p.\n", stCapFrame.u32Width,
            stCapFrame.u32Height, stCapFrame.aphyAddr, stCapFrame.pavirAddr);

        mi_divp_SaveFile(pchFileName, stCapFrame.pavirAddr, stCapFrame.u32Stride,
                                    stCapFrame.u32Width, stCapFrame.u32Height, stCapFrame.ePixelFormat);

        DBG_INFO(" prepare to release capture  frame.\n");
        usleep(3*1000*1000);

        MI_DISP_ReleaseScreenFrame(0, &stCapFrame);
    }

    while(_bPushData)
    {
        usleep(50000);
    }
    destroy_push_ES_data_thread(&stPushDataInfo);

    for (u8Index = 0; u8Index < u32ChnNum; u8Index++)
    {
        close(_hEsFile[u8Index]);
        MI_SYS_UnBindChnPort( &gstVdecOutputPort[u8Index], &gstDivpInputPort[u8Index]);
        MI_SYS_UnBindChnPort(&gstDivpOutputPort[u8Index], &gstDispInputPort[u8Index]);
        MI_VDEC_StopChn(u8Index);
        MI_VDEC_DestroyChn(u8Index);
        mi_divp_DestroyChannel(u8Index);
        MI_DISP_DisableInputPort(DispLayer, u32InputPort[u8Index]);
    }
    MI_DISP_DisableVideoLayer(DispLayer);
    MI_DISP_UnBindVideoLayer(DispLayer, DispDev);
    mi_divp_hdmiDeInit();
    MI_DISP_Disable(DispDev);

    DBG_INFO("########## test case 19 Vdec->Divp->Disp capture end ##############\n\n");
    return s32Ret;
}
#endif

MI_S32 MI_DIVP_TestCase_10(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));

    stAttr.bHorMirror = true;
    stAttr.bVerMirror = true;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_90;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 10 HV mirror start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

        ///get data from input file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 10 HV mirror end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_11(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));

    stAttr.bHorMirror = true;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 11 H mirror start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

        ///get data from input file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 11 H mirror end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_12(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = true;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 12 V mirror start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

        ///get data from input file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 12 V mirror end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_CreatVpeChannel(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort, MI_SYS_WindowRect_t *pstCropWin, MI_SYS_WindowRect_t *pstDispWin)
{
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_SYS_WindowRect_t stCropWin;
    stChannelVpeAttr.u16MaxW = 1920;
    stChannelVpeAttr.u16MaxH = 1080;
    stChannelVpeAttr.bNrEn= FALSE;
    stChannelVpeAttr.bEdgeEn= FALSE;
    stChannelVpeAttr.bEsEn= FALSE;
    stChannelVpeAttr.bContrastEn= FALSE;
    stChannelVpeAttr.bUvInvert= FALSE;
    stChannelVpeAttr.ePixFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    ExecFunc(MI_VPE_CreateChannel(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelAttr(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    stChannelVpeAttr.bContrastEn = FALSE;
    stChannelVpeAttr.bNrEn = FALSE;
    ExecFunc(MI_VPE_SetChannelAttr(VpeChannel, &stChannelVpeAttr), MI_VPE_OK);

    ExecFunc(MI_VPE_GetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
    stCropWin.u16X = pstCropWin->u16X;
    stCropWin.u16Y = pstCropWin->u16Y;
    stCropWin.u16Width = pstCropWin->u16Width;
    stCropWin.u16Height = pstCropWin->u16Height;
    ExecFunc(MI_VPE_SetChannelCrop(VpeChannel, &stCropWin), MI_VPE_OK);
    MI_VPE_PortMode_t stVpeMode;
    memset(&stVpeMode, 0, sizeof(stVpeMode));
    ExecFunc(MI_VPE_GetPortMode(VpeChannel, VpePort, &stVpeMode), MI_VPE_OK);
    stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stVpeMode.u16Width = pstDispWin->u16Width;
    stVpeMode.u16Height= pstDispWin->u16Height;
    ExecFunc(MI_VPE_SetPortMode(VpeChannel, VpePort, &stVpeMode), MI_VPE_OK);
    ExecFunc(MI_VPE_EnablePort(VpeChannel, VpePort), MI_VPE_OK);
    ExecFunc(MI_VPE_StartChannel (VpeChannel), MI_VPE_OK);
}

MI_S32 MI_DIVP_TestCase_23(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_DIVP_CHN u32ChnId = 15;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    MI_U32 u32ReadSize = 0;
    FILE *pInputFile = NULL;
#if BIND_DIVP_VPE
    MI_VPE_CHANNEL VpeChannel = 0;
    MI_VPE_PORT VpePort = 0;
    MI_SYS_WindowRect_t stVpeCropWin;
    MI_SYS_WindowRect_t stVpeDispWin;
    MI_SYS_ChnPort_t stVpeInputPort;
    MI_SYS_ChnPort_t stVpeOutputPort;
    stVpeCropWin.u16X = 0;
    stVpeCropWin.u16Y = 0;
    stVpeCropWin.u16Width = 736;
    stVpeCropWin.u16Height = 480;

    stVpeDispWin.u16X = 0;
    stVpeDispWin.u16Y = 0;
    stVpeDispWin.u16Width = 720;
    stVpeDispWin.u16Height = 480;

    stVpeInputPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeInputPort.u32DevId = 0;
    stVpeInputPort.u32ChnId = VpeChannel;
    stVpeInputPort.u32PortId = VpePort;

    stVpeOutputPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeOutputPort.u32DevId = 0;
    stVpeOutputPort.u32ChnId = VpeChannel;
    stVpeOutputPort.u32PortId = VpePort;
#endif
    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_3D;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_MIDDLE;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
#if BIND_DIVP_VPE
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
#else
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
#endif
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 23 FPGA 3D DI start ############## \n");

    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        mi_divp_CreateChannel(u32ChnId, u16InputWidth, u16InputHeight, E_MI_DIVP_DI_TYPE_3D);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
    }

#if BIND_DIVP_VPE
    MI_DIVP_CreatVpeChannel(VpeChannel, VpePort, &stVpeCropWin, &stVpeDispWin);
    MI_SYS_SetChnOutputPortDepth(&stVpeOutputPort, 2, 5);
    MI_SYS_BindChnPort(&gstDivpOutputPort[u32ChnId], &stVpeInputPort, 30, 30);
#endif

    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n", pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    while(u32FrameNumber < u16FrameNumber)
    {
        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
        {
            DBG_INFO("@@ nChnId = 0, buffer  phyaddr[0] = 0x%llx, phyaddr[1] = 0x%llx, phyaddr[2] = 0x%llx @@\n",
                    stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.phyAddr[1],stInputBufInfo.stFrameData.phyAddr[2]);
            DBG_INFO("@@ nChnId = 0, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                    stInputBufInfo.stFrameData.pVirAddr[0], stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);
            DBG_INFO("@@ nChnId = 0, buffer u32Stride[0] = %u, u32Stride[1] = %u, u32Stride[2] = %u @@\n",
                    stInputBufInfo.stFrameData.u32Stride[0], stInputBufInfo.stFrameData.u32Stride[1],stInputBufInfo.stFrameData.u32Stride[2]);


            if(u32FrameNumber % 2)
            {
                stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_TOP;
            }
            else
            {
                stInputBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_BOTTOM;
            }
            stInputBufInfo.u64Pts = 0x12345600ULL + u32FrameNumber;
            DBG_INFO("eFieldType = %d, u64Pts = 0x%llx.\n", stInputBufInfo.stFrameData.eFieldType, stInputBufInfo.u64Pts);

            ///get data from input file.
            if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
            {
                fclose(pInputFile);
                DBG_INFO(" read input frame data failed!.\n");
                return MI_DIVP_ERR_FAILED;
            }

            //set buffer to divp
            if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
            {
                DBG_INFO("prepare to exit.\n");
                usleep(5* 1000 * 1000);
                DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
            }
        }
        else
        {
            DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
        }
        u32FrameNumber ++;
        usleep(1000000);
    }

#if BIND_DIVP_VPE
    MI_SYS_UnBindChnPort(&gstDivpOutputPort[15], &stVpeInputPort);
#endif

    DBG_INFO("prepare to exit.\n");
    usleep(8* 1000 * 1000);
    fclose(pInputFile);

    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        mi_divp_DestroyChannel(u32ChnId);
    }
    DBG_INFO("########## test case 23 FPGA 3D DI end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_13(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;
#if BIND_DIVP_VPE
    MI_VPE_CHANNEL VpeChannel = 0;
    MI_VPE_PORT VpePort = 0;
    MI_SYS_WindowRect_t stVpeCropWin;
    MI_SYS_WindowRect_t stVpeDispWin;
    MI_SYS_ChnPort_t stVpeInputPort;
    MI_SYS_ChnPort_t stVpeOutputPort;
    stVpeCropWin.u16X = 0;
    stVpeCropWin.u16Y = 0;
    stVpeCropWin.u16Width = u16OutputWidth;
    stVpeCropWin.u16Height = u16OutputHeight;

    stVpeDispWin.u16X = 0;
    stVpeDispWin.u16Y = 0;
    stVpeDispWin.u16Width = 720;
    stVpeDispWin.u16Height = 480;

    stVpeInputPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeInputPort.u32DevId = 0;
    stVpeInputPort.u32ChnId = VpeChannel;
    stVpeInputPort.u32PortId = VpePort;

    stVpeOutputPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeOutputPort.u32DevId = 0;
    stVpeOutputPort.u32ChnId = VpeChannel;
    stVpeOutputPort.u32PortId = VpePort;
#endif

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_90;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;
#if BIND_DIVP_VPE
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
#else
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
#endif

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 13 rotate 90 start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

#if BIND_DIVP_VPE
    MI_DIVP_CreatVpeChannel(VpeChannel, VpePort, &stVpeCropWin, &stVpeDispWin);
    MI_SYS_SetChnOutputPortDepth(&stVpeOutputPort, 2, 5);
    MI_SYS_BindChnPort(&gstDivpOutputPort[u32ChnId], &stVpeInputPort, 30, 30);
#endif

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer  phyaddr[0] = 0x%llx, phyaddr[1] = 0x%llx, phyaddr[2] = 0x%llx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.phyAddr[1],stInputBufInfo.stFrameData.phyAddr[2]);
        DBG_INFO("@@ nChnId = 0, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.pVirAddr[0], stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);
        DBG_INFO("@@ nChnId = 0, buffer u32Stride[0] = %u, u32Stride[1] = %u, u32Stride[2] = %u @@\n",
                stInputBufInfo.stFrameData.u32Stride[0], stInputBufInfo.stFrameData.u32Stride[1],stInputBufInfo.stFrameData.u32Stride[2]);

        ///get data from input file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

#if BIND_DIVP_VPE
    MI_SYS_UnBindChnPort(&gstDivpOutputPort[15], &stVpeInputPort);
#endif

    DBG_INFO("prepare to exit.\n");
    usleep(8* 1000 * 1000);
    fclose(pInputFile);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 13 rotate 90 end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_14(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_270;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 14 rotate 270 start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer  phyaddr[0] = 0x%llx, phyaddr[1] = 0x%llx, phyaddr[2] = 0x%llx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.phyAddr[1],stInputBufInfo.stFrameData.phyAddr[2]);
        DBG_INFO("@@ nChnId = 0, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.pVirAddr[0], stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);
        DBG_INFO("@@ nChnId = 0, buffer u32Stride[0] = %u, u32Stride[1] = %u, u32Stride[2] = %u @@\n",
                stInputBufInfo.stFrameData.u32Stride[0], stInputBufInfo.stFrameData.u32Stride[1],stInputBufInfo.stFrameData.u32Stride[2]);

        ///get data from input file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }

        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    DBG_INFO("prepare to exit.\n");
    usleep(8* 1000 * 1000);
    fclose(pInputFile);
    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 14 rotate 270 end ############## \n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_15(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32FrameNumber = 0;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;
    MI_DISP_DEV DispDev = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT u32InputPort[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    MI_SYS_ChnPort_t stDispChnInputPort[16];
    MI_U32 u32PortId =0;
    MI_DISP_InputPortAttr_t stInputPortAttr[16];
    MI_U16 u16Row = 0;
    MI_U16 u16Column = 0;
    MI_U16 u16Avg = 0;
    MI_U16 u16InputPortNum = 0;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));
    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 15 32 channel start ############## \n");
    if(u32ChnNum < 17)
    {
        if(u32ChnNum <= 1)
        {
            u16Avg = 1;
        }
        else if(u32ChnNum <= 4)
        {
            u16Avg = 2;
        }
        else if(u32ChnNum <= 9)
        {
            u16Avg = 3;
        }
        else if(u32ChnNum <= 16)
        {
            u16Avg = 4;
        }

    }
    else
    {
        u16Avg = 4;
    }

    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;

    stLayerAttr.stVidLayerSize.u16Width = 1920;
    stLayerAttr.stVidLayerSize.u16Height= 1080;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    stLayerAttr.ePixFormat = eOutPxlFmt;

    MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    MI_DISP_Enable(DispDev);

    mi_disp_hdmiInit();
    MI_HDMI_Attr_t stHdmiAttr;
    MI_HDMI_DeviceId_e eHdmi = 0;
    MI_HDMI_GetAttr(eHdmi,&stHdmiAttr);
    stHdmiAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
    MI_HDMI_SetAttr(eHdmi,&stHdmiAttr);

    MI_DISP_BindVideoLayer(DispLayer, DispDev);
    MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
    MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n", stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);
    MI_DISP_EnableVideoLayer(DispLayer);
    if(u32ChnNum < 16)
    {
        u16InputPortNum = u32ChnNum;
    }
    else
    {
        u16InputPortNum = 16;
    }
    MI_DISP_SetVideoLayerAttrBegin(DispLayer);
    for(u32PortId = 0; u32PortId < u16InputPortNum; u32PortId ++)
    {
        stDispChnInputPort[u32PortId].eModId = E_MI_MODULE_ID_DISP;
        stDispChnInputPort[u32PortId].u32DevId = 0;
        stDispChnInputPort[u32PortId].u32ChnId = 0;
        stDispChnInputPort[u32PortId].u32PortId = u32PortId;

        stInputPortAttr[u32PortId].stDispWin.u16Width = 1920 / u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16Height = 1080 / u16Avg;

        u16Row = u32PortId / u16Avg;
        u16Column = u32PortId - u16Row * u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16X = u16Column * 1920 / u16Avg;
        stInputPortAttr[u32PortId].stDispWin.u16Y = u16Row * 1080 / u16Avg;

        DBG_INFO(">>>>>>>  u32PortId = %u, disp win(%u, %u, %u, %u) <<<<<<<<<< \n", u32PortId,stInputPortAttr[u32PortId].stDispWin.u16X,
            stInputPortAttr[u32PortId].stDispWin.u16Y, stInputPortAttr[u32PortId].stDispWin.u16Width, stInputPortAttr[u32PortId].stDispWin.u16Height);
        MI_DISP_SetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_GetInputPortAttr(DispLayer, u32InputPort[u32PortId], &(stInputPortAttr[u32PortId]));
        MI_DISP_EnableInputPort(DispLayer, u32InputPort[u32PortId]);
    }
    MI_DISP_SetVideoLayerAttrEnd(DispLayer);

    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        stAttr.bHorMirror = false;
        stAttr.bVerMirror = false;
        stAttr.stCropRect.u16X = 0;
        stAttr.stCropRect.u16Y = 0;
        stAttr.stCropRect.u16Width = u16InputWidth;
        stAttr.stCropRect.u16Height = u16InputHeight;
        if(u32ChnId < 16)
        {
            stOutputPortAttr.u32Width = 1920 / u16Avg;
            stOutputPortAttr.u32Height = 1080 / u16Avg;
        }
        else
        {
            stOutputPortAttr.u32Width = 256;
            stOutputPortAttr.u32Height = 256;
        }
        mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
        mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
        mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);
        if(u32ChnId < 16)
        {
            MI_SYS_BindChnPort(&gstDivpOutputPort[u32ChnId], &gstDispInputPort[u32ChnId], 30, 30);
        }
    }

    usleep(5*1000*1000);
    DBG_INFO("open file %s \n", pchFileName);
    usleep(5*1000*1000);
    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    while(u32FrameNumber < u16FrameNumber)
    {
        for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
        {
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[u32ChnId],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
            {
                DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                        stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                        stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

                ///get data from file.
                if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
                {
                    fclose(pInputFile);
                    DBG_INFO(" read input frame data failed!.\n");
                    return MI_DIVP_ERR_FAILED;
                }

                //set buffer to divp
                if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
                {
                    u32FrameNumber ++;
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf OK.\n");
                }
                else
                {
                    DBG_INFO("MI_SYS_ChnInputPortPutBuf fail.\n");
                }
            }
            else
            {
                DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
            }
            usleep(1000000);
        }
    }

    fclose(pInputFile);

    usleep(3000000);
    for(u32ChnId = 0; (u32ChnId < u32ChnNum && u32ChnId < 16); u32ChnId ++)
    {
        MI_SYS_UnBindChnPort(&gstDivpOutputPort[u32ChnId], &gstDispInputPort[u32ChnId]);

        MI_DISP_DisableInputPort(DispLayer, u32InputPort[u32ChnId]);
    }
    MI_DISP_DisableVideoLayer(DispLayer);
    MI_DISP_UnBindVideoLayer(DispLayer, DispDev);
    mi_divp_hdmiDeInit();
    MI_DISP_Disable(DispDev);

    for(u32ChnId = 0; u32ChnId < u32ChnNum; u32ChnId ++)
    {
        mi_divp_DestroyChannel(u32ChnId);
    }
    DBG_INFO("########## test case 15 32 channel end ##############\n\n");
    return s32Ret;
}

MI_S32 MI_DIVP_TestCase_16(char* pchFileName, MI_U16 u16InputWidth, MI_U16 u16InputHeight,
    MI_SYS_PixelFormat_e eInPxlFmt, MI_U16 u16OutputWidth, MI_U16 u16OutputHeight,
    MI_SYS_PixelFormat_e eOutPxlFmt, MI_U16 u16FrameNumber , MI_U32 u32ChnNum)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_DIVP_CHN u32ChnId = 0;
    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_BufConf_t stInputBufConf;
    MI_SYS_BufInfo_t stInputBufInfo;
    MI_SYS_BUF_HANDLE hInputBufHdl = 0;
    FILE *pInputFile = NULL;

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    memset(&stInputBufConf, 0, sizeof(stInputBufConf));
    memset(&stInputBufInfo, 0, sizeof(stInputBufInfo));

    stAttr.bHorMirror = false;
    stAttr.bVerMirror = false;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.stCropRect.u16X = 100;
    stAttr.stCropRect.u16Y = 100;
    stAttr.stCropRect.u16Width = u16InputWidth - 200;
    stAttr.stCropRect.u16Height = u16InputHeight - 200;
    stAttr.u32MaxWidth = CHN_MAX_WIDTH;
    stAttr.u32MaxHeight = CHN_MAX_HEIGHT;;

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat = eOutPxlFmt;
    stOutputPortAttr.u32Width = u16OutputWidth;
    stOutputPortAttr.u32Height = u16OutputHeight;

    stInputBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stInputBufConf.u32Flags = 0x80000000;   //0 or MI_SYS_MAP_VA=0x80000000
    stInputBufConf.u64TargetPts = 0x12340000;
    stInputBufConf.stFrameCfg.u16Width = u16InputWidth;
    stInputBufConf.stFrameCfg.u16Height = u16InputHeight;
    stInputBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stInputBufConf.stFrameCfg.eFormat = eInPxlFmt;

    DBG_INFO("########## test case 16 refrsh channel start ############## \n");
    mi_divp_CreateChannel(u32ChnId, CHN_MAX_WIDTH, CHN_MAX_HEIGHT, E_MI_DIVP_DI_TYPE_OFF);
    mi_divp_test_SetChnAttr(u32ChnId, &stAttr);
    mi_divp_test_SetOutputPortAttr(u32ChnId, &stOutputPortAttr);

    ///read input video stream
    pInputFile =fopen(pchFileName, "rb");
    //src_fd = open(pchFileName, O_RDONLY);
    //if (src_fd < 0)
    if (pInputFile == NULL)
    {
        DBG_INFO("open input file error. pchFileName = %s\n",pchFileName);
        return MI_DIVP_ERR_FAILED;
    }
    else
    {
        DBG_INFO("input file name : %s.\n", pchFileName);
    }

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&gstDivpInputPort[0],&stInputBufConf,&stInputBufInfo,&hInputBufHdl, 1))
    {
        DBG_INFO("@@ nChnId = 0, buffer phyaddr = 0x%llx, pVirAddr[0] = 0x%lx, pVirAddr[1] = 0x%lx, pVirAddr[2] = 0x%lx @@\n",
                stInputBufInfo.stFrameData.phyAddr[0],stInputBufInfo.stFrameData.pVirAddr[0],
                stInputBufInfo.stFrameData.pVirAddr[1],stInputBufInfo.stFrameData.pVirAddr[2]);

        ///get data from file.
        if(!mi_divp_GetDivpInputFrameData(eInPxlFmt, pInputFile, &stInputBufInfo))
        {
            fclose(pInputFile);
            DBG_INFO(" read input frame data failed!.\n");
            return MI_DIVP_ERR_FAILED;
        }


        //set buffer to divp
        if(MI_SUCCESS == MI_SYS_ChnInputPortPutBuf (hInputBufHdl,  &stInputBufInfo, false))
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf OK.\n");
        }
        else
        {
            DBG_INFO(" MI_SYS_ChnInputPortPutBuf fail.\n");
        }
    }
    else
    {
        DBG_INFO("MI_SYS_ChnInputPortGetBuf fail.\n");
    }

    fclose(pInputFile);
    usleep(10000000);
    DBG_INFO("refresh channel. \n");
    usleep(10000000);

    stAttr.stCropRect.u16X = 0;
    stAttr.stCropRect.u16Y = 0;
    stAttr.stCropRect.u16Width = u16InputWidth;
    stAttr.stCropRect.u16Height = u16InputHeight;
    MI_DIVP_SetChnAttr(u32ChnId, &stAttr);
    MI_DIVP_RefreshChn(u32ChnId);
    usleep(4000000);

    mi_divp_DestroyChannel(u32ChnId);
    DBG_INFO("########## test case 16 refrsh channel end ############## \n");
    return s32Ret;
}

MI_S32 mi_divp_init()
{
    MI_S32 s32Ret = MI_DIVP_ERR_FAILED;
    MI_DIVP_CHN u32ChnId = 0;
    memset(_thrGetBuf, 0x0, sizeof(_thrGetBuf));

    for(u32ChnId = 0; u32ChnId < CHN_NUM; u32ChnId ++)
    {
        gstDispInputPort[u32ChnId].eModId = E_MI_MODULE_ID_DISP;
        gstDispInputPort[u32ChnId].u32DevId = 0;
        gstDispInputPort[u32ChnId].u32ChnId = 0;
        gstDispInputPort[u32ChnId].u32PortId = u32ChnId;

        gstDivpInputPort[u32ChnId].eModId = E_MI_MODULE_ID_DIVP;
        gstDivpInputPort[u32ChnId].u32DevId = 0;
        gstDivpInputPort[u32ChnId].u32ChnId = u32ChnId;
        gstDivpInputPort[u32ChnId].u32PortId = 0;

        gstDivpOutputPort[u32ChnId].eModId = E_MI_MODULE_ID_DIVP;
        gstDivpOutputPort[u32ChnId].u32DevId = 0;
        gstDivpOutputPort[u32ChnId].u32ChnId = u32ChnId;
        gstDivpOutputPort[u32ChnId].u32PortId = 0;

        gstVdecOutputPort[u32ChnId].eModId = E_MI_MODULE_ID_VDEC;
        gstVdecOutputPort[u32ChnId].u32DevId = 0;
        gstVdecOutputPort[u32ChnId].u32ChnId = u32ChnId;
        gstVdecOutputPort[u32ChnId].u32PortId = 0;
    }

    if (pthread_create(&_gthrGetDivpOutputBuffer, NULL, mi_divp_GetDivpOutputFrameData, NULL))
    {
        DBG_INFO(" create mi_divp_GetDivpOutputFrameData thread fail.\n");
    }
    else
    {
        DBG_INFO(" create mi_divp_GetDivpOutputFrameData thread success!.\n");
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

MI_U32 mi_divp_GetCodecType(char* pchCodecType)
{
    MI_VDEC_CodecType_e eCodecType = E_MI_VDEC_CODEC_TYPE_H264;

    if(strncmp(pchCodecType, "h264", 4)  == 0)
    {
        eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
        printf(" [%s %d] eCodecType = E_MI_VDEC_CODEC_TYPE_H264 . \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pchCodecType, "h265", 4) == 0)
    {
        eCodecType = E_MI_VDEC_CODEC_TYPE_H265;
        printf(" [%s %d] eCodecType = E_MI_VDEC_CODEC_TYPE_H265 . \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else
    {
        eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
        printf(" [%s %d] eCodecType = E_MI_VDEC_CODEC_TYPE_H264 . \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }

    return eCodecType;
}

MI_U32 mi_divp_GetResolution(char* pchResolution)
{
    MI_U32 u32Resolution = 720;

    if(strncmp(pchResolution, "720p", 4)  == 0)
    {
        u32Resolution = 720;
        printf(" [%s %d] u32Resolution = 720. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pchResolution, "1080p", 4) == 0)
    {
        u32Resolution = 1080;
        printf(" [%s %d] u32Resolution = 1080. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pchResolution, "480p", 4) == 0)
    {
        u32Resolution = 480;
        printf(" [%s %d] u32Resolution = 480. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else
    {
        u32Resolution = 720;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }

    return u32Resolution;
}

MI_SYS_PixelFormat_e mi_divp_GetPixelFormat(char* pChPixelFormat)
{
    MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

    if(strncmp(pChPixelFormat, "yuv422", 6)  == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pChPixelFormat, "argb", 4) == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pChPixelFormat, "rgb565", 6) == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else if(strncmp(pChPixelFormat, "mst420", 6)  == 0)
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }
    else
    {
        ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        printf(" [%s %d]ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420. \n", __FUNCTION__, __LINE__);
        usleep(100 * 1000);
    }

    return ePixelFormat;
}

int main(int argc, const char *argv[])
{
    char InputCmd[512] = { 0 };
    char tempCmd[256] = { 0 };
    char achFilePath[256] = {0};
    MI_U16 u16InputWidth = 0;
    MI_U16 u16InputHeight = 0;
    MI_U16 u16OutputWidth = 0;
    MI_U16 u16OutputHeight = 0;
    MI_U16 u16TestcaseId = 0;
    MI_U16 u16Index = 0;
    char* pchStart = NULL;
    MI_U16 u16ParaNum = 0;
    MI_VDEC_CodecType_e eCodecType1 = E_MI_VDEC_CODEC_TYPE_H264;
    MI_VDEC_CodecType_e eCodecType2 = E_MI_VDEC_CODEC_TYPE_H264;
    MI_U32 u32Resolution1 = 720;
    MI_U32 u32Resolution2 = 720;
    MI_SYS_PixelFormat_e eInPxlFmt = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    MI_U32 u32ChnNum = 0;

    MI_SYS_Init();
    ExecFunc(mi_divp_init(), MI_SUCCESS);

    while (1)
    {
        DBG_INFO(" please input test case ID or exit:\n");
        DBG_INFO(" test case 1: show one frame.\n");
        DBG_INFO(" test case 2: multi channel with out crop.\n");
        DBG_INFO(" test case 3: 3D DI.\n");
        DBG_INFO(" test case 4: MFdec\n");
        DBG_INFO(" test case 5: resolution change.\n");
        DBG_INFO(" test case 6: I/P mode change.\n");
        DBG_INFO(" test case 7: Electronic amplification.\n");
        DBG_INFO(" test case 8: .\n");
        DBG_INFO(" test case 9: Vdec->DIVP multi channel\n");
        DBG_INFO(" test case 10: HV mirror.\n");
        DBG_INFO(" test case 11: H mirror.\n");
        DBG_INFO(" test case 12: V mirror.\n");
        DBG_INFO(" test case 13: rotate 90.\n");
        DBG_INFO(" test case 14: rotate 270.\n");
        DBG_INFO(" test case 15: multi channel: 0~15 connect to disp, 16~31 write to memery.\n");
        DBG_INFO(" test case 16: refresh channel.\n");
        DBG_INFO(" test case 17: capture screen with H264 or H265 in\n");
        DBG_INFO(" test case 19: Vdec->DIVP->Disp multi channel\n");
        DBG_INFO(" test case 23: @@ FPGA @@ 3D DI.\n");
        DBG_INFO(" test case 26: @@ FPGA @@ I/P mode change.\n");

        fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
        if (strncmp(InputCmd, "exit", 4) == 0)
        {
            printf("prepare to exit!\n\n");
            break;
        }
        else if (strncmp(InputCmd, "debug", 5) == 0)
        {
            printf("prepare to debug and exit!\n\n");
            return 0;
        }
        else
        {
            pchStart = &InputCmd[0];
            gu32CaseId = atoi(InputCmd);
            u16TestcaseId = gu32CaseId;
            DBG_INFO(" InputCmd test case ID = %u.\n", gu32CaseId);
            DBG_INFO("\n please send test case parameters: \n");

            if((1 == gu32CaseId) || (2 == gu32CaseId) || (3 == gu32CaseId)  || (4 == gu32CaseId) || (7 == gu32CaseId) || (10 == gu32CaseId)
                 || (11 == gu32CaseId) || (12 == gu32CaseId) || (13 == gu32CaseId) || (14 == gu32CaseId) || (15 == gu32CaseId)
                 ||(16 == gu32CaseId)  || (23 == gu32CaseId))
            {
                DBG_INFO("\n inputfilepath inputwidth inputheight inputformat outputwidth outputheight Outputformat framenumber channelnumber\n");
                fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
                DBG_INFO(" InputCmd = %s. strlen(InputCmd) = %u.\n", InputCmd, strlen(InputCmd));

                for(u16Index = 0; u16Index < strlen(InputCmd); u16Index ++)
                {
                    if((InputCmd[u16Index] == ' ') && (0 == u16ParaNum))
                    {
                        memcpy(achFilePath, pchStart, (&InputCmd[u16Index] - pchStart));
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" achFilePath = %s. \n", achFilePath);
                    }
                    else if((InputCmd[u16Index] == ' ') && (1 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16InputWidth = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16InputWidth = %u.\n", u16InputWidth);
                    }
                    else if((InputCmd[u16Index] == ' ') && (2 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16InputHeight = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16InputHeight = %u.\n", u16InputHeight);
                    }
                    else if((InputCmd[u16Index] == ' ') && (3 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        eInPxlFmt = mi_divp_GetPixelFormat(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" eInPxlFmt = %u.\n", eInPxlFmt);
                    }
                    else if((InputCmd[u16Index] == ' ') && (4 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16OutputWidth = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16OutputWidth = %u.\n", u16OutputWidth);
                    }
                    else if((InputCmd[u16Index] == ' ') && (5 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16OutputHeight = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16OutputHeight = %u.\n", u16OutputHeight);
                    }
                    else if((InputCmd[u16Index] == ' ') && (6 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        _geOutPxlFmt = mi_divp_GetPixelFormat(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" eOutPxlFmt = %d. \n", _geOutPxlFmt);
                    }
                    else if((InputCmd[u16Index] == ' ') && (7 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        gu16FrameNumber = atoi(tempCmd);
                        DBG_INFO(" u16FrameNumber = %u.\n", gu16FrameNumber);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                    }
                    else if((InputCmd[u16Index] == ' ') && (8 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u32ChnNum = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u32ChnNum = %u.\n", u32ChnNum);
                    }
                }
            }
            else if((5 == gu32CaseId) || (6 == gu32CaseId) || (26 == gu32CaseId))
            {
                DBG_INFO("\n outputwidth outputheight Outputformat framenumber\n");
                fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
                DBG_INFO(" InputCmd = %s. strlen(InputCmd) = %u.\n", InputCmd, strlen(InputCmd));

                for(u16Index = 0; u16Index < strlen(InputCmd); u16Index ++)
                {
                    if((InputCmd[u16Index] == ' ') && (0 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16OutputWidth = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16OutputWidth = %u.\n", u16OutputWidth);
                    }
                    else if((InputCmd[u16Index] == ' ') && (1 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u16OutputHeight = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u16OutputHeight = %u.\n", u16OutputHeight);
                    }
                    else if((InputCmd[u16Index] == ' ') && (2 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        _geOutPxlFmt = mi_divp_GetPixelFormat(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" eOutPxlFmt = %d. \n", _geOutPxlFmt);
                    }
                    else if((InputCmd[u16Index] == ' ') && (3 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        gu16FrameNumber = atoi(tempCmd);
                        DBG_INFO(" u16FrameNumber = %u.\n", gu16FrameNumber);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                    }
                }
            }
            else if((9 == gu32CaseId) ||(17 == gu32CaseId) ||  (19 == gu32CaseId))
            {
                DBG_INFO("\n ChnNum, CodecType1, resolution1, CodecType2, reolution2 \n");
                fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
                DBG_INFO(" InputCmd = %s. strlen(InputCmd) = %u.\n", InputCmd, strlen(InputCmd));

                for(u16Index = 0; u16Index < strlen(InputCmd); u16Index ++)
                {
                    if((InputCmd[u16Index] == ' ') && (0 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u32ChnNum = atoi(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u32ChnNum = %u.\n", u32ChnNum);
                    }
                    else if((InputCmd[u16Index] == ' ') && (1 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        eCodecType1 = mi_divp_GetCodecType(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" eCodecType1 = %u.\n", eCodecType1);
                    }
                    else if((InputCmd[u16Index] == ' ') && (2 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u32Resolution1 = mi_divp_GetResolution(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u32Resolution1 = %d. \n", u32Resolution1);
                    }
                    else if((InputCmd[u16Index] == ' ') && (3 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        eCodecType2 = mi_divp_GetCodecType(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" eCodecType2 = %u.\n", eCodecType2);
                    }
                    else if((InputCmd[u16Index] == ' ') && (4 == u16ParaNum))
                    {
                        memcpy(tempCmd, pchStart, (&InputCmd[u16Index] - pchStart));
                        u32Resolution2 = mi_divp_GetResolution(tempCmd);
                        memset(tempCmd, 0, sizeof(tempCmd));
                        pchStart = &InputCmd[u16Index + 1];
                        u16ParaNum ++;
                        DBG_INFO(" u32Resolution2 = %d. \n", u32Resolution2);
                    }
                }
            }

            memset(tempCmd, 0, sizeof(tempCmd));
            memset(InputCmd, 0, sizeof(InputCmd));
            u16ParaNum = 0;

            DBG_INFO("TestcaseId = %u, filepath = %s, inputwidth = %u, inputheight = %u, eInPxlFmt = %u.\n outputwidth = %u, outputheight = %u, eOutPxlFmt = %u, framenumber = %u, u32ChnNum = %u.\n",
                u16TestcaseId, achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            usleep(100*1000);

            if(1 == u16TestcaseId)
            {
                MI_DIVP_TestCase_1(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(2 == u16TestcaseId)
            {
                MI_DIVP_TestCase_2(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(3 == u16TestcaseId)
            {
                MI_DIVP_TestCase_3(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(4 == u16TestcaseId)
            {
                MI_DIVP_TestCase_4(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(5 == u16TestcaseId)
            {
                MI_DIVP_TestCase_5(u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber);
            }
            else if(6 == u16TestcaseId)
            {
                MI_DIVP_TestCase_6(u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber);
            }
            else if(7 == u16TestcaseId)
            {
                MI_DIVP_TestCase_7(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
#ifndef MI_DIVP_FPGA_TEST
            else if(7264 == u16TestcaseId)
            {
                MI_DIVP_TestCase_7_264();
            }
            else if(7265 == u16TestcaseId)
            {
                MI_DIVP_TestCase_7_265();
            }
            else if(8264 == u16TestcaseId)
            {
                MI_DIVP_TestCase_8_264();
            }
            else if(8265 == u16TestcaseId)
            {
                MI_DIVP_TestCase_8_265();
            }
            else if(9 == u16TestcaseId)
            {
                MI_DIVP_TestCase_9(u32ChnNum, eCodecType1, u32Resolution1, eCodecType2, u32Resolution2);
            }
            else if(17 == u16TestcaseId)
            {
                MI_DIVP_TestCase_17(u32ChnNum, eCodecType1, u32Resolution1, eCodecType2, u32Resolution2);
            }
            else if(19 == u16TestcaseId)
            {
                MI_DIVP_TestCase_19(u32ChnNum, eCodecType1, u32Resolution1, eCodecType2, u32Resolution2);
            }
#endif
            else if(10 == u16TestcaseId)
            {
                MI_DIVP_TestCase_10(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(11 == u16TestcaseId)
            {
                MI_DIVP_TestCase_11(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(12 == u16TestcaseId)
            {
                MI_DIVP_TestCase_12(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(13 == u16TestcaseId)
            {
                MI_DIVP_TestCase_13(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(14 == u16TestcaseId)
            {
                MI_DIVP_TestCase_14(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(15 == u16TestcaseId)
            {
                MI_DIVP_TestCase_15(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(16 == u16TestcaseId)
            {
                MI_DIVP_TestCase_16(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(23 == u16TestcaseId)
            {
                MI_DIVP_TestCase_23(achFilePath, u16InputWidth, u16InputHeight, eInPxlFmt,
                    u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber, u32ChnNum);
            }
            else if(26 == u16TestcaseId)
            {
                MI_DIVP_TestCase_26(u16OutputWidth, u16OutputHeight, _geOutPxlFmt, gu16FrameNumber);
            }
        }
        memset(achFilePath, 0, sizeof(achFilePath));
        usleep(2000000);
    }

    return 0;
}
