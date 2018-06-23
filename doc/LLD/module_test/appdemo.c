
void app_flow_sample()
{
    MI_SYS_ChnPort_t stDVIPChnPortInfo;
    memset(&stDVIPChnPortInfo, 0x0, sizeof(MI_SYS_ChnPort_t));
    stDVIPChnPortInfo.eModuleID = E_MI_SYS_MODULE_ID_DIVP;
    stDVIPChnPortInfo.u32DevID = 0;
    stDVIPChnPortInfo.u32ChnIdx = 1;
    stDVIPChnPortInfo.u32PortIdx = 3;
    ///config output port by driver
    MI_SYS_ChnPort_t stChnPortInfo;
    memset(&stChnPortInfo, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPortInfo.eModuleID = E_MI_SYS_MODULE_ID_VPE;
    stChnPortInfo.u32DevID = 0;
    stChnPortInfo.u32ChnIdx = 1;
    stChnPortInfo.u32PortIdx = 3;

#define PER_FRAME_SIZE (1920 * 1080 * 1.5)
    MI_U16 u16FrameWidth = 1920;
    MI_U16 u16FrameHeight = 1080;
    MI_SYS_PixelFormat_e eFrameFormat = E_MI_FRAME_FORMAT_YUV422_YUYV;
    MI_U32 u32PerFrameSize = u16FrameWidth * u16FrameHeight * 1.5;///pixel size = 1.5
    MI_U8 u8FrameNum = 8;
    MI_SYS_ChnOutputPortCfgInfo_t stChnCfgInfo;
    memset(&stChnCfgInfo, 0x0, sizeof(MI_SYS_ChnOutputPortCfgInfo_t));
    stChnCfgInfo.u32FrameRate = 25;
    stChnCfgInfo.u32GetUsrFrameDepth = 5;///by frame
    stChnCfgInfo.u32ChnBindFrameDepth = u8FrameNum;
    stChnCfgInfo.eFormat = eFrameFormat;
    stChnCfgInfo.u32Width = u16FrameWidth;
    stChnCfgInfo.u32Height = u16FrameHeight;
    stChnCfgInfo.u32Stride = u16FrameWidth * 1;
    MI_SYS_ConfigOutputPort(&stChnPortInfo, &stChnCfgInfo);

    MI_SYS_ChnPortInfo_t stVDECChnPortInfo;
    memset(&stVDECChnPortInfo, 0x0, sizeof(MI_SYS_ChnPortInfo_t));
    stVDECChnPortInfo.eModuleID = E_MI_SYS_MODULE_ID_VDEC;
    stVDECChnPortInfo.u32DevID = 0;
    stVDECChnPortInfo.u32ChnIdx = 1;
    stVDECChnPortInfo.u32PortIdx = 1;
    stVDECChnPortInfo.ePortType = MI_SYS_CHN_PORT_TYPE_OUTPUT;
    MI_SYS_BindChn(&stDVIPChnPortInfo, &stVDECChnPortInfo);
}

