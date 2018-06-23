#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#include <sstream>

#include "mi_sys.h"
#include "mi_venc.h"
#include "st_common.h"
#include "st_vpe.h"
#include "st_vif.h"
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"

#include "Mdb_Integrate.h"

#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
#endif
#define RTSP_LISTEN_PORT    554
static MI_S32 St_SkipAndWriteVpeFrame(int fd, MI_U32 u32VpePortId, MI_U16 byPassFrame)
{
    MI_SYS_ChnPort_t stVpeChnOutputPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_U16  u16BufInfoStride =0;
    MI_U16  u16BufInfoHeight =0;
    MI_U32  u32FrameSize =0;
    int offset =0;
    int by_pass_frame = byPassFrame;


    stVpeChnOutputPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnOutputPort.u32DevId = 0;
    stVpeChnOutputPort.u32ChnId = 0;
    stVpeChnOutputPort.u32PortId = u32VpePortId;

    while (1)
    {
        if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stVpeChnOutputPort , &stBufInfo,&hHandle))
        {
            // Add user write buffer to file
            u16BufInfoStride  = stBufInfo.stFrameData.u32Stride[0];
            u16BufInfoHeight = stBufInfo.stFrameData.u16Height;
            u32FrameSize = u16BufInfoStride*u16BufInfoHeight;
            // put frame
            printf("getbuf sucess, size(%dx%d), stride(%d, %d, %d), Pixel %d, viraddr(%p, %p, %p)\n", stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height,
            stBufInfo.stFrameData.u32Stride[0], stBufInfo.stFrameData.u32Stride[1], stBufInfo.stFrameData.u32Stride[2], stBufInfo.stFrameData.ePixelFormat,
            stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.pVirAddr[1], stBufInfo.stFrameData.pVirAddr[2]);
            if (by_pass_frame)
            {
                printf("########By pass cnt %d ###########\n", by_pass_frame);
                by_pass_frame--;
                STCHECKRESULT(MI_SYS_ChnOutputPortPutBuf(hHandle));
            }
            else
            {
                printf("########start to write file!!!!!!!!###########\n");
                STCHECKRESULT(ST_Write_OneFrame(fd, offset, (char *)stBufInfo.stFrameData.pVirAddr[0], u16BufInfoStride, u16BufInfoStride, u16BufInfoHeight));
                offset += u32FrameSize;

                if(stBufInfo.stFrameData.ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                {
                    STCHECKRESULT(ST_Write_OneFrame(fd, offset, (char *)stBufInfo.stFrameData.pVirAddr[1], u16BufInfoStride, u16BufInfoStride, u16BufInfoHeight/2));
                    offset += u32FrameSize/2;
                }
                    STCHECKRESULT(MI_SYS_ChnOutputPortPutBuf(hHandle));
                printf("########End to write file!!!!!!!!###########\n");
                break;
            }
        }
        usleep(33 * 1000);
    }

    return MI_SUCCESS;
}
static MI_S32 St_SkipAndWriteVencData(int fd, MI_VENC_CHN VeChn, MI_U16 byPassFrame)
{
    MI_SYS_ChnPort_t stVencChnOutputPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_BOOL bWriteFile = FALSE;
    MI_S32 s32Ret;
    int by_pass_frame = byPassFrame;

    memset(&stVencChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&hHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    stVencChnOutputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnOutputPort.u32ChnId = 0;
    stVencChnOutputPort.u32PortId = 0;
    s32Ret = MI_VENC_GetChnDevid(VeChn, &stVencChnOutputPort.u32DevId);
    if (s32Ret != MI_SUCCESS)
    {
        printf("Venc get dev error!\n");
        return s32Ret;
    }
    while (1)
    {
        if (!by_pass_frame)
        {
            STCHECKRESULT(MI_VENC_RequestIdr(0, TRUE));
        }
        if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stVencChnOutputPort , &stBufInfo, &hHandle))
        {
            if (stBufInfo.eBufType != E_MI_SYS_BUFDATA_RAW)
            {
                printf("Data type error !");
                STCHECKRESULT(MI_SYS_ChnOutputPortPutBuf(hHandle));

                break;
            }
            if (by_pass_frame)
            {
                printf("######## Raw data By pass cnt %d ###########\n", by_pass_frame);
                by_pass_frame--;
                if (!by_pass_frame)
                {
                    STCHECKRESULT(MI_VENC_RequestIdr(0, TRUE));
                }
                STCHECKRESULT(MI_SYS_ChnOutputPortPutBuf(hHandle));
            }
            else
            {
                printf("########start to write file!!!!!!!!###########\n");
                write(fd, (void *)stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32ContentSize);
                STCHECKRESULT(MI_SYS_ChnOutputPortPutBuf(hHandle));
                printf("########End to write file!!!!!!!!###########\n");
                break;
            }

        }
        usleep(33 * 1000);
    }

    return MI_SUCCESS;
}

std::map<std::string, Live555RTSPServer*> Mdb_Integrate::mapRTSPServer;

Mdb_Integrate::Mdb_Integrate()
{
    PREPARE_COMMAND("vpevifinit", &Mdb_Integrate::VpeVifInit, 0);
    PREPARE_COMMAND("vpevifdeinit", &Mdb_Integrate::VpeVifDeinit, 0);
    PREPARE_COMMAND("vpecreateport", &Mdb_Integrate::VpeCreatePort, 4);
    PREPARE_COMMAND("vpedestroyport", &Mdb_Integrate::VpeDestroyPort, 1);
    PREPARE_COMMAND("vpewritefile", &Mdb_Integrate::VpeWriteFile, 3);

    PREPARE_COMMAND("vencinit", &Mdb_Integrate::VencInit, 5);
    PREPARE_COMMAND("vencdeinit", &Mdb_Integrate::VencDeinit, 2);
    PREPARE_COMMAND("vencwritefile", &Mdb_Integrate::VencWriteFile, 3);
    PREPARE_COMMAND("vencinjectfrm", &Mdb_Integrate::VencInjectFrame, 5);

    PREPARE_COMMAND("rtspstart", &Mdb_Integrate::RtspServerStart, 2);
    PREPARE_COMMAND("rtspstop", &Mdb_Integrate::RtspServerStop, 1);
}
Mdb_Integrate::~Mdb_Integrate()
{
}
void Mdb_Integrate::ShowWelcome(std::string &strOut)
{
    strOut.assign("Welcome to integrate module test\n");
}
void Mdb_Integrate::VpeWriteFile(std::vector<std::string> &inStrings, std::string &strOut)
{
    int fd = 0;

    MDB_EXPECT_OK("ST_OpenDestFile", strOut, ST_OpenDestFile(inStrings[0].c_str(), &fd), MI_SUCCESS);
    MDB_EXPECT_OK("St_SkipAndWriteVpeFrame", strOut, St_SkipAndWriteVpeFrame(fd, Atoi(inStrings[1]), Atoi(inStrings[2])), MI_SUCCESS);
    ST_CloseFile(fd);

}
void Mdb_Integrate::VpeVifInit(std::vector<std::string> &inStrings, std::string &strOut)
{
    ST_Sys_BindInfo_t stVpeVifBindInfo;
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;

    memset(&stVpeVifBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
    memset(&stVpeChannelInfo, 0x0, sizeof(ST_VPE_ChannelInfo_t));
    MDB_EXPECT_OK("ST_Sys_Init", strOut, ST_Sys_Init(), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vif_CreateDev", strOut, ST_Vif_CreateDev(0, SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME), MI_SUCCESS);
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = 1920;
    stVifPortInfoInfo.u32RectHeight = 1080;
    stVifPortInfoInfo.u32DestWidth = 1920;
    stVifPortInfoInfo.u32DestHeight = 1080;
    stVifPortInfoInfo.ePixFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    MDB_EXPECT_OK("ST_Vif_CreatePort", strOut, ST_Vif_CreatePort(0, 0, &stVifPortInfoInfo), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vif_StartPort", strOut, ST_Vif_StartPort(0, 0), MI_SUCCESS);
    /************************************************
    Step4:  init VPE
    *************************************************/
    stVpeChannelInfo.u16VpeMaxW = 1920;
    stVpeChannelInfo.u16VpeMaxH = 1080;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 0;
    stVpeChannelInfo.u16VpeCropH = 0;
    stVpeChannelInfo.eRunningMode = E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
    stVpeChannelInfo.eFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    MDB_EXPECT_OK("ST_Vpe_CreateChannel", strOut, ST_Vpe_CreateChannel(0, &stVpeChannelInfo), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vpe_StartChannel", strOut, ST_Vpe_StartChannel(0), MI_SUCCESS);

    /************************************************
    Step7:  Bind VIF->VPE
    *************************************************/
    memset(&stVpeVifBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stVpeVifBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stVpeVifBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stVpeVifBindInfo.stSrcChnPort.u32ChnId = 0;
    stVpeVifBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stVpeVifBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeVifBindInfo.stDstChnPort.u32DevId = 0;
    stVpeVifBindInfo.stDstChnPort.u32ChnId = 0;
    stVpeVifBindInfo.stDstChnPort.u32PortId = 0;
    stVpeVifBindInfo.u32SrcFrmrate = 30;
    stVpeVifBindInfo.u32DstFrmrate = 30;
    MDB_EXPECT_OK("ST_Sys_Bind", strOut, ST_Sys_Bind(&stVpeVifBindInfo), MI_SUCCESS);
}
void Mdb_Integrate::VpeVifDeinit(std::vector<std::string> &inStrings, std::string &strOut)
{
    ST_Sys_BindInfo_t stVpeVifBindInfo;

    memset(&stVpeVifBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stVpeVifBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stVpeVifBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stVpeVifBindInfo.stSrcChnPort.u32ChnId = 0;
    stVpeVifBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stVpeVifBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeVifBindInfo.stDstChnPort.u32DevId = 0;
    stVpeVifBindInfo.stDstChnPort.u32ChnId = 0;
    stVpeVifBindInfo.stDstChnPort.u32PortId = 0;
    stVpeVifBindInfo.u32SrcFrmrate = 30;
    stVpeVifBindInfo.u32DstFrmrate = 30;
    MDB_EXPECT_OK("ST_Sys_UnBind", strOut, ST_Sys_UnBind(&stVpeVifBindInfo), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vpe_DestroyChannel", strOut, ST_Vpe_DestroyChannel(0), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vif_StopPort", strOut, ST_Vif_StopPort(0, 0), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Vif_DisableDev", strOut, ST_Vif_DisableDev(0), MI_SUCCESS);
    MDB_EXPECT_OK("ST_Sys_Exit", strOut, ST_Sys_Exit(), MI_SUCCESS);
}
void Mdb_Integrate::VpeCreatePort(std::vector<std::string> &inStrings, std::string &strOut)
{
    ST_VPE_PortInfo_t stVpePortInfo;
    MI_SYS_ChnPort_t stVpeChnPort;
    MI_VPE_PORT VpePort = 0;

    memset(&stVpeChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stVpePortInfo, 0x0, sizeof(ST_VPE_PortInfo_t));
    stVpePortInfo.DepVpeChannel = 0;
    stVpePortInfo.ePixelFormat = (MI_SYS_PixelFormat_e)Atoi(inStrings[2]);
    stVpePortInfo.u16OutputWidth = Atoi(inStrings[0]);
    stVpePortInfo.u16OutputHeight = Atoi(inStrings[1]);
    VpePort = Atoi(inStrings[3]);
    MDB_EXPECT_OK("ST_Vpe_CreatePort", strOut, ST_Vpe_CreatePort(VpePort, &stVpePortInfo), MI_SUCCESS); //default support port0 --->>> vdisp    
    stVpeChnPort.eModId = E_MI_MODULE_ID_VPE;
    stVpeChnPort.u32DevId = 0;
    stVpeChnPort.u32ChnId = 0;
    stVpeChnPort.u32PortId = VpePort;
    MDB_EXPECT_OK("MI_SYS_SetChnOutputPortDepth", strOut, MI_SYS_SetChnOutputPortDepth(&stVpeChnPort, 1, 4), MI_SUCCESS);
}
void Mdb_Integrate::VpeDestroyPort(std::vector<std::string> &inStrings, std::string &strOut)
{
    MI_VPE_PORT VpePort = 0;

    VpePort = Atoi(inStrings[0]);
    MDB_EXPECT_OK("ST_Vpe_StopPort", strOut, ST_Vpe_StopPort(0, VpePort), MI_SUCCESS);
}
void Mdb_Integrate::VencWriteFile(std::vector<std::string> &inStrings, std::string &strOut)
{
    int fd = 0;

    MDB_EXPECT_OK("ST_OpenDestFile", strOut, ST_OpenDestFile(inStrings[0].c_str(), &fd), MI_SUCCESS);
    MDB_EXPECT_OK("St_SkipAndWriteVencData", strOut, St_SkipAndWriteVencData(fd, Atoi(inStrings[1]), Atoi(inStrings[2])), MI_SUCCESS);
    ST_CloseFile(fd);
}
void Mdb_Integrate::VencInit(std::vector<std::string> &inStrings, std::string &strOut)
{
    MI_VENC_ChnAttr_t stVencAttr;
    ST_Sys_BindInfo_t stVpeVencBindInfo;
    MI_SYS_ChnPort_t stChnVencChnPort;
    MI_VENC_ParamJpeg_t stJpegPara;
    MI_U32 u32VencDevId = 0, s32Ret;
    MI_U32 u32Width = Atoi(inStrings[0]);
    MI_U32 u32Height = Atoi(inStrings[1]);
    MI_VENC_CHN VeChn =  Atoi(inStrings[2]);

    memset(&stVencAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stVpeVencBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
    memset(&stChnVencChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    switch (Atoi(inStrings[3]))
    {
        case 0: //JPEG
        {
            stVencAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stVencAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stVencAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = stVencAttr.stVeAttr.stAttrJpeg.u32PicWidth = u32Width;
            stVencAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = stVencAttr.stVeAttr.stAttrJpeg.u32PicHeight = u32Height;
            stVencAttr.stVeAttr.stAttrJpeg.bByFrame = TRUE;
        }
        break;
        case 1: //H264
        {
            stVencAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stVencAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2; // not support B frame
            stVencAttr.stVeAttr.stAttrH264e.u32PicWidth = stVencAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32Width;
            stVencAttr.stVeAttr.stAttrH264e.u32PicHeight = stVencAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32Height;
            stVencAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
            stVencAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stVencAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
            stVencAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stVencAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
            stVencAttr.stRcAttr.stAttrH264FixQp.u32IQp = 36;
            stVencAttr.stRcAttr.stAttrH264FixQp.u32PQp = 36;
        }
        break;
        case 2: //H265
        {
            stVencAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stVencAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2; // not support B frame
            stVencAttr.stVeAttr.stAttrH265e.u32PicWidth = stVencAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = u32Width;
            stVencAttr.stVeAttr.stAttrH265e.u32PicHeight = stVencAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = u32Height;
            stVencAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;
            stVencAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum= 30;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32Gop = 30;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate=1024*1024;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32MaxQp=45;
            stVencAttr.stRcAttr.stAttrH265Vbr.u32MinQp=25;
        }
        break;
        default:
            printf("error format!\n");
            return;
    }
    MDB_EXPECT_OK("MI_VENC_CreateChn", strOut, MI_VENC_CreateChn(VeChn, &stVencAttr), MI_SUCCESS);
    if(stVencAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        memset(&stJpegPara, 0, sizeof(MI_VENC_ParamJpeg_t));
        MI_VENC_GetJpegParam(0, &stJpegPara);
        stJpegPara.u32Qfactor = 30;
        MI_VENC_SetJpegParam(0, &stJpegPara);
    }
    stChnVencChnPort.u32ChnId = VeChn;
    stChnVencChnPort.u32PortId = 0;
    MDB_EXPECT_OK("MI_VENC_GetChnDevid", strOut, MI_VENC_GetChnDevid(stChnVencChnPort.u32ChnId, &u32VencDevId), MI_SUCCESS);
    stChnVencChnPort.u32DevId = u32VencDevId;
    stChnVencChnPort.eModId = E_MI_MODULE_ID_VENC;
    //This was set to (5, 10) and might be too big for kernel
    MDB_EXPECT_OK("MI_SYS_SetChnOutputPortDepth", strOut, MI_SYS_SetChnOutputPortDepth(&stChnVencChnPort, 1, 1), MI_SUCCESS);
    MDB_EXPECT_OK("MI_VENC_StartRecvPic", strOut, MI_VENC_StartRecvPic(VeChn), MI_SUCCESS);
    if (Atoi(inStrings[4]))
    {
        stVpeVencBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stVpeVencBindInfo.stSrcChnPort.u32DevId = 0;
        stVpeVencBindInfo.stSrcChnPort.u32ChnId = 0;
        stVpeVencBindInfo.stSrcChnPort.u32PortId = VeChn;
        stVpeVencBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stVpeVencBindInfo.stDstChnPort.u32DevId = u32VencDevId;
        stVpeVencBindInfo.stDstChnPort.u32ChnId = VeChn;
        stVpeVencBindInfo.stDstChnPort.u32PortId = 0;
        stVpeVencBindInfo.u32SrcFrmrate = 30;
        stVpeVencBindInfo.u32DstFrmrate = 30;
        MDB_EXPECT_OK("ST_Sys_Bind", strOut, ST_Sys_Bind(&stVpeVencBindInfo), MI_SUCCESS);
    }
}
void Mdb_Integrate::VencDeinit(std::vector<std::string> &inStrings, std::string &strOut)
{
    MI_VENC_CHN VeChn = 0;
    MI_U32 u32VencDevId = 0;

    VeChn = Atoi(inStrings[0]);
    if (Atoi(inStrings[1]))
    {
        ST_Sys_BindInfo_t stVpeVencBindInfo;

        MDB_EXPECT_OK("MI_VENC_GetChnDevid", strOut, MI_VENC_GetChnDevid(VeChn, &u32VencDevId), MI_SUCCESS);
        memset(&stVpeVencBindInfo, 0, sizeof(ST_Sys_BindInfo_t));
        stVpeVencBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
        stVpeVencBindInfo.stSrcChnPort.u32DevId = 0;
        stVpeVencBindInfo.stSrcChnPort.u32ChnId = 0;
        stVpeVencBindInfo.stSrcChnPort.u32PortId = VeChn;
        stVpeVencBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stVpeVencBindInfo.stDstChnPort.u32DevId = u32VencDevId;
        stVpeVencBindInfo.stDstChnPort.u32ChnId = VeChn;
        stVpeVencBindInfo.stDstChnPort.u32PortId = 0;
        stVpeVencBindInfo.u32SrcFrmrate = 30;
        stVpeVencBindInfo.u32DstFrmrate = 30;
        MDB_EXPECT_OK("ST_Sys_UnBind", strOut, ST_Sys_UnBind(&stVpeVencBindInfo), MI_SUCCESS);
    }
    MDB_EXPECT_OK("MI_VENC_StopRecvPic", strOut, MI_VENC_StopRecvPic(VeChn), MI_SUCCESS);
    MDB_EXPECT_OK("MI_VENC_DestroyChn", strOut, MI_VENC_DestroyChn(VeChn), MI_SUCCESS);

    return;
}
void Mdb_Integrate::VencInjectFrame(std::vector<std::string> &inStrings, std::string &strOut)
{
    MI_SYS_BufConf_t stBufConf;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_U32 u32YSize = 0;
    MI_U8 *pData = NULL, *pDataFrom = NULL, *pDataTo = NULL;
    MI_U16 u16Line = 0;
    MI_U16 u16InjectCnt = 0, i = 0;
    MI_SYS_ChnPort_t stChnVencChnPort;

    int fd = 0;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    memset(&stChnVencChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.u16Width = Atoi(inStrings[1]);
    stBufConf.stFrameCfg.u16Height = Atoi(inStrings[2]);
    fd = open(inStrings[0].c_str(), O_RDONLY);
    if (fd <0 )
    {
        strOut += "Open file error!\n";
        return;
    }
    pData = ( MI_U8 *)malloc(stBufConf.stFrameCfg.u16Width * stBufConf.stFrameCfg.u16Height * 3 /2);
    ASSERT(pData);
    read(fd, pData, stBufConf.stFrameCfg.u16Width * stBufConf.stFrameCfg.u16Height * 3 /2);
    close(fd);

    pDataFrom = pData;
    stChnVencChnPort.u32ChnId = Atoi(inStrings[3]);
    stChnVencChnPort.u32PortId = 0;
    MDB_EXPECT_OK("MI_VENC_GetChnDevid", strOut, MI_VENC_GetChnDevid(stChnVencChnPort.u32ChnId, &stChnVencChnPort.u32DevId), MI_SUCCESS);
    stChnVencChnPort.eModId = E_MI_MODULE_ID_VENC;
    u16InjectCnt = Atoi(inStrings[4]);
    for (i = 0; i < u16InjectCnt; i++)
    {
        s32Ret = MI_SYS_ChnInputPortGetBuf(&stChnVencChnPort, &stBufConf, &stBufInfo, &hHandle, 3000);
        if (s32Ret == MI_SUCCESS)
        {
            u32YSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            MI_SYS_FlushInvCache(stBufInfo.stFrameData.pVirAddr[0], u32YSize);
            MI_SYS_FlushInvCache(stBufInfo.stFrameData.pVirAddr[1], u32YSize >> 1);
            pDataTo = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
            for (u16Line = 0; u16Line < stBufInfo.stFrameData.u16Height; u16Line++)
            {
                memcpy(pDataTo, pDataFrom, stBufInfo.stFrameData.u16Width);
                pDataTo += stBufInfo.stFrameData.u32Stride[0];
                pDataFrom += stBufInfo.stFrameData.u16Width;
            }
            pDataTo = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[1];
            for (u16Line = 0; u16Line < stBufInfo.stFrameData.u16Height / 2; u16Line++)
            {
                memcpy(pDataTo, pDataFrom, stBufInfo.stFrameData.u16Width);
                pDataTo += stBufInfo.stFrameData.u32Stride[0];
                pDataFrom += stBufInfo.stFrameData.u16Width;
            }
            MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo , FALSE);
        }

    }
    free(pData);
}

#define STREAM_TITTLE "main_stream"
void* Mdb_Integrate::OpenStream(char const* szStreamName, void* arg)
{
    MI_U32 i = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    char *pStr = (char *)szStreamName;
    std::string str;
    MI_VENC_CHN *pChnNum = NULL;

    if (strncmp(STREAM_TITTLE, szStreamName, strlen(STREAM_TITTLE)))
    {
        printf("Not found %s\n", STREAM_TITTLE);

        return NULL;
    }
    pStr += strlen(STREAM_TITTLE);
    str = *pStr;
    pChnNum = (MI_VENC_CHN *)malloc(sizeof(MI_VENC_CHN));
    ASSERT(pChnNum);
    *pChnNum = Atoi(str);
    s32Ret = MI_VENC_RequestIdr(*pChnNum, TRUE);
    if (MI_SUCCESS != s32Ret)
    {
        printf("request IDR fail, error:%x\n", s32Ret);

        return NULL;
    }
    printf("Open stream %s\n", szStreamName);

    return (void *)pChnNum;
}

int Mdb_Integrate::VideoReadStream(void* handle, unsigned char* ucpBuf, int BufLen, struct timeval *p_Timestamp, void* arg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_CHN chnNum = 0;

    memset(&stStream, 0, sizeof(stStream));
    memset(&stPack, 0, sizeof(stPack));

    ASSERT(handle);
    chnNum = *((MI_VENC_CHN *)handle);
    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    s32Ret = MI_VENC_Query(chnNum, &stStat);
    if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
    {
        return 0;
    }

    s32Ret = MI_VENC_GetStream(chnNum, &stStream, 40);
    if (MI_SUCCESS == s32Ret)
    {
        len = stStream.pstPack[0].u32Len;
        memcpy(ucpBuf,stStream.pstPack[0].pu8Addr,len);

        MI_VENC_ReleaseStream(chnNum, &stStream);

        return len;
    }

    return 0;
}

int Mdb_Integrate::CloseStream(void* handle, void* arg)
{
    ASSERT(handle);
    printf("Close stream %s%d\n", STREAM_TITTLE, *((MI_VENC_CHN *)handle));
    free((MI_VENC_CHN *)handle);

    return 0;
}

void Mdb_Integrate::RtspServerStart(std::vector<std::string> &inStrings, std::string &strOut)
{
    unsigned int rtspServerPortNum = RTSP_LISTEN_PORT;
    int iRet = 0;
    char* urlPrefix = NULL;
    ServerMediaSession* mediaSession = NULL;
    ServerMediaSubsession* subSession = NULL;
    Live555RTSPServer *pRTSPServer = NULL;

    if (!strstr(inStrings[0].c_str(), STREAM_TITTLE))
    {
        printf("Not found %s\n", STREAM_TITTLE);

        return;
    }
    pRTSPServer = new Live555RTSPServer();
    if (pRTSPServer == NULL)
    {
        printf("malloc error\n");
        return;
    }

    iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
    while (iRet < 0)
    {
        rtspServerPortNum++;

        if (rtspServerPortNum > 65535)
        {
            printf("Failed to create RTSP server: %s\n", pRTSPServer->getResultMsg());
            delete pRTSPServer;
            pRTSPServer = NULL;
            return;
        }

        iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
    }

    urlPrefix = pRTSPServer->rtspURLPrefix();
    printf("=================URL===================\n");
    printf("%s%s\n", urlPrefix, inStrings[0].c_str());
    printf("=================URL===================\n");


    pRTSPServer->createServerMediaSession(mediaSession,
                                          inStrings[0].c_str(),
                                          NULL, NULL);

    if (Atoi(inStrings[1]) == 1) // H264
    {
        subSession = WW_H264VideoFileServerMediaSubsession::createNew(
                                    *(pRTSPServer->GetUsageEnvironmentObj()),
                                    inStrings[0].c_str(),
                                    OpenStream,
                                    VideoReadStream,
                                    CloseStream, 30);
    }
    else if (Atoi(inStrings[1]) == 2) // H265
    {
        subSession = WW_H265VideoFileServerMediaSubsession::createNew(
                                    *(pRTSPServer->GetUsageEnvironmentObj()),
                                    inStrings[0].c_str(),
                                    OpenStream,
                                    VideoReadStream,
                                    CloseStream, 30);
    }
    else
    {
        Print(strOut, "Not support!\n", PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT);

        return;
    }

    pRTSPServer->addSubsession(mediaSession, subSession);
    pRTSPServer->addServerMediaSession(mediaSession);


    pRTSPServer->Start();

    mapRTSPServer[inStrings[0]] = pRTSPServer;
    Print(strOut, "rtsp start ok!\n", PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT);

}

void Mdb_Integrate::RtspServerStop(std::vector<std::string> &inStrings, std::string &strOut)
{
    std::map<std::string, Live555RTSPServer*>::iterator iter;

    if (!strstr(inStrings[0].c_str(), STREAM_TITTLE))
    {
        printf("Not found %s\n", STREAM_TITTLE);

        return;
    }
    iter = mapRTSPServer.find(inStrings[0]);
    if(iter != mapRTSPServer.end())
    {
        iter->second->Join();
        delete iter->second;
        iter->second = NULL;
        mapRTSPServer.erase(iter);
        Print(strOut, "rtsp stop ok!\n", PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT);
    }
    else
    {
        Print(strOut, "Not found string!\n", PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT);
    }
}


