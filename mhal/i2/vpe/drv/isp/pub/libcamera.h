#ifndef _LIBCAMERA_H_
#define _LIBCAMERA_H_

#include <mhal_common.h>
#include <mhal_cmdq.h>
#include <hal_isp_cmdq_if.h>

#define LIBCAMERA_MSG(args...)  pr_info(args)

#define LIBCAMERA_DMSG(args...) do{}while(0)
#define LIBCAMERA_EMSG(args...) pr_err(args)
#define LIBCAMERA_VMSG(args...) do{}while(0)

#define MAX_SUPPORT_SENSOR_NUM      (4)
#define ISPMID_MAX_CH MAX_SUPPORT_SENSOR_NUM

typedef enum
{
    eRealTimeModeNoHDR=0,
    eRealTimeModeHDR=1,
    eFrameModeNoHDR=2,
    eFrameModeHDR=3,
    eFrameModeYUV=4,
    eRealTimeModeYUV=5
}CameraMode_e;

typedef enum
{
    eCamerBayerID_Invalid=-1,
    eCamerBayerID_RG=0,
    eCamerBayerID_GR=1,
    eCamerBayerID_BG=2,
    eCamerBayerID_GB=3,
}CameraBayerID_e;

typedef enum
{
    eCameraPixelFormat_Invalid=-1,
    eCameraPixelFormat_Bayer8BPP=0,
    eCameraPixelFormat_Bayer10BPP=1,
    eCameraPixelFormat_Bayer12BPP=2,
    eCameraPixelFormat_Bayer16BPP=3,
    eCameraPixelFormat_YUYV=4,
    eCameraPixelFormat_NV12=5
}CameraPixelFormat_e;

/* Raw fetch control */
typedef enum
{
    eRawFetchNode_P0HEAD = 0,
    eRawFetchNode_P1HEAD = 1
}CameraRawFetchNode_e;

typedef struct
{
    u32 nPitch;
    u32 nWidth;
    u32 nHeight;
    CameraPixelFormat_e ePixelFmt;
    CameraBayerID_e eBayerID;
    u32 nPhysAddr;
}CameraRawFetchCfg_t;

/* Raw store control */
typedef enum
{
    eRawStoreNode_P0HEAD = 0, /* Control by VIF, Do not use */
    eRawStoreNode_P1HEAD = 1, /* Control by VIF, Do not use */
    eRawStoreNode_P0TAIL = 2,
    eRawStoreNode_P1TAIL = 3,
    eRawStoreNode_ISPOUT = 4,
    eRawStoreNode_VDOS   = 5
}CameraRawStoreNode_e;

typedef struct
{
    u32 nPitch;
    u32 nWidth;
    u32 nHeight;
    CameraPixelFormat_e ePixelFmt;
    CameraBayerID_e eBayerID;
    u32 nPhysAddr;
}CameraRawStoreCfg_t;

typedef struct {
    s32 (*alloc)(u8 *pu8Name, u32 size,u64 *phyAddr);
    s32 (*free)(u64 u64PhyAddr);
    void * (*map)(u64 u64PhyAddr, u32 u32Size ,u8 bCache);
    void   (*unmap)(void *pVirtAddr);
    s32 (*flush_cache)(void *pVirtAddr,u32 u32Size);
}CameraMemAlloctor_t;

typedef struct
{
    /* Input size */
    u32 nCropX;
    u32 nCropY;
    u32 nCropWidth;
    u32 nCropHeight;
    CameraPixelFormat_e ePixelFmt;
    CameraBayerID_e eBayerID;
    CameraMode_e eMode;
    CameraMemAlloctor_t *ptMemAlloc;
    /* channel 0~3 */
    u32 nCh;
    /* bayer scaler output size */
    u32 nOutWidth;
    u32 nOutHeight;
    /* bayer scaler output crop*/
    u32 nOutCropX;
    u32 nOutCropY;
    u32 nOutCropWidth;
    u32 nOutCropHeight;
    u32 nHDRFB;   //frame buffer for real-time HDR mode in MIU address.
}CameraOpenAttr_t;

/* For CameraSetControlFlag */
#define CAMERA_CTRL_FLAG_RGB_IQ_EN      0x1
#define CAMERA_CTRL_FLAG_YUV_IQ_EN      0x2
#define CAMERA_CTRL_FLAG_RGB_MLOAD_EN   0x4
#define CAMERA_CTRL_FLAG_YUV_MLOAD_EN   0x8
#define CAMERA_CTRL_FLAG_AE_EN          0x10
#define CAMERA_CTRL_FLAG_AWB_EN         0x20
#define CAMERA_CTRL_FLAG_AF_EN          0x40

int MhalCameraOpen(CameraOpenAttr_t *pAttr);
void MhalCameraClose(u32 nCh);
int CameraContextSwitch(u32 nCh);
int CameraOpen(CameraMode_e eMode);
void CameraClose(void);
int CameraRawFetch(CameraRawFetchNode_e eNode,CameraRawFetchCfg_t *pCfg);
int CameraRawFetchTrigger(CameraRawFetchNode_e eNode);
int CameraRawStore(CameraRawStoreNode_e eNode,CameraRawStoreCfg_t *pCfg);
int CameraRawStoreTrigger(CameraRawStoreNode_e eNode);
void CameraLoadIQFile(u32 nChannel, void **hnd);
void CameraSetSensorFPS(int nCh,unsigned int nFps);
/* */
void CameraSetControlFlag(int nCh,u32 nItem,unsigned char bEn);

IspCmdqIf_t* CameraChangeCmdqIF(IspCmdqIf_t *pNew);
MHAL_CMDQ_CmdqInterface_t* CameraChangeCmdqIF2(MHAL_CMDQ_CmdqInterface_t *pNew);

#endif
