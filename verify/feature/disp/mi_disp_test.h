#ifndef __MI_DISP_TEST_HEADER__
#define __MI_DISP_TEST_HEADER__
#include "mi_vpe.h"
#include "mi_divp.h"

#define DISP_TEST_001_DESC ("MI_DISP show signal input window")
#define DISP_TEST_002_DESC ("MI_DISP show 16 input window")
#define DISP_TEST_003_DESC ("MI_DISP switch 4 input to 9 input window")
#define DISP_TEST_004_DESC ("MI_DISP switch timing")
#define DISP_TEST_005_DESC ("MI_DISP Homologous output")
#define DISP_TEST_006_DESC ("MI_DISP Capture screen")
#define DISP_TEST_007_DESC ("MI_DISP Change layer size & pos")
#define DISP_TEST_008_DESC ("MI_DISP Change input port pos")
#define DISP_TEST_009_DESC ("MI_DISP batch input port changes")
#define DISP_TEST_010_DESC ("MI_DISP input port pause")
#define DISP_TEST_011_DESC ("MI_DISP input port Zoom")
#define DISP_TEST_012_DESC ("MI_DISP Set HDMI VGA CSC")
#define DISP_TEST_013_DESC ("MI_DISP 2 layer overlap")


#define DISP_TEST_INFO(fmt, args...)  //printf(fmt, ##args)
#define DISP_TEST_DBG(fmt, args...) //printf(fmt, ##args)
#define DISP_TEST_ERROR(fmt, args...) printf(fmt, ##args)
void test_disp_ShowFrameInfo (const char *s, MI_SYS_FrameData_t *pstFrameInfo);
MI_BOOL test_disp_OpenSourceFile(const char *pFileName, int *pSrcFd);
void test_disp_FdRewind(int srcFd);
void test_disp_CloseFd(int fd);

MI_S32 mi_disp_test_SetDevPubAttr(MI_DISP_DEV DispChn, MI_DISP_PubAttr_t* pstDevAttr);
MI_S32 mi_disp_test_SetDevBgColor(MI_DISP_DEV DispChn, MI_U32 u32BgColor);
MI_S32 mi_disp_test_SetOutPutTiming(MI_DISP_DEV DispChn, MI_DISP_Interface_e eIntfType, MI_DISP_OutputTiming_e eOuttiming, MI_DISP_SyncInfo_t* pstSyncInfo);
MI_S32 mi_disp_test_EnableDev(MI_DISP_DEV DispChn, MI_BOOL bEnable);
MI_S32 mi_disp_test_SetVideoLayerAttr(MI_DISP_DEV DispChn, MI_DISP_VideoLayerAttr_t* pstLayerAttr);
MI_S32 mi_disp_test_SetVideoLayerDispWin(MI_DISP_DEV DispChn, MI_U16 u16X, MI_U16 u16Y, MI_U16 u16Width, MI_U16 u16Height);
MI_S32 mi_disp_test_SetVideoLayerSize(MI_DISP_DEV DispChn, MI_U16 u16Width, MI_U16 u16Height);
MI_S32 mi_disp_test_EnableVideoLayer(MI_DISP_LAYER VideoLayer, MI_BOOL bEnable);
MI_S32 mi_disp_test_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev, MI_BOOL bBind);
MI_S32 mi_divp_test_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t* pstPortAttr);
MI_S32 mi_divp_test_SetInputPortRect(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_U16 u16x, MI_U16 u16y, MI_U16 u16Width, MI_U16 u16height);
MI_S32 mi_disp_test_EnableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_BOOL bEnable);
MI_S32 mi_disp_hdmiInit(void);
MI_S32 test_disp_GetOneFrame(int srcFd, int offset, char *pData, int yuvSize);
MI_S32 test_disp_GetOneFrameYUV420ByStride(int srcFd, char *pYData, char *pUvData, int ySize, int uvSize, int height, int width, int yStride, int uvStride);
MI_S32 test_disp_GetOneYuv422Frame(int srcFd, int offset, char *pData, int width, int height, int yStride);
MI_S32 test_disp_PutOneFrame(int dstFd, int offset, char *pDataFrame, int line_offset, int line_size, int lineNumber);
MI_S32 test_disp_CreatVpeChannel(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort, MI_SYS_WindowRect_t *pstCropWin, MI_SYS_WindowRect_t *pstDispWin);
MI_S32 test_Disp_DestroyVpeChannel(MI_VPE_CHANNEL VpeChannel, MI_VPE_PORT VpePort);
MI_S32 test_disp_vpeUnBinderDisp(MI_U32 VpeOutputPort, MI_U32 DispInputPort);
MI_S32 test_disp_vpeBinderDisp(MI_U32 VpeOutputPort, MI_U32 DispInputPort);
MI_S32 test_disp_CreateDivpChannel(MI_DIVP_CHN DivpChn, MI_SYS_WindowRect_t *pstCapWin, MI_SYS_WindowRect_t *pstDispWin, MI_DIVP_DiType_e eDiType);
MI_S32 test_disp_DestroyDivpChannel(MI_DIVP_CHN u32ChnId);

#endif
