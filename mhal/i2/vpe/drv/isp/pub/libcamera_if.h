#ifndef _LIBCAMERAIF_H_
#define _LIBCAMERAIF_H_

#include <libcamera.h>

typedef struct
{
    int     (*CameraOpen)(CameraMode_e eMode);
    void    (*CameraClose)(void);
    int     (*CameraRawFetch)(CameraRawFetchNode_e eNode,CameraRawFetchCfg_t *pCfg);
    int     (*CameraRawFetchTrigger)(CameraRawFetchNode_e eNode);
    int     (*CameraRawStore)(CameraRawStoreNode_e eNode,CameraRawStoreCfg_t *pCfg);
    int     (*CameraRawStoreTrigger)(CameraRawStoreNode_e eNode);
    void    (*CameraLoadIQFile)(u32 nChannel, void **hnd);
    int     (*MhalCameraOpen)(CameraOpenAttr_t *pAttr);
    IspCmdqIf_t* (*CameraChangeCmdqIF)(IspCmdqIf_t *pNew);
    void    (*MhalCameraClose)(u32 nCh);
    int    (*CameraContextSwitch)(u32 nCh);
    MHAL_CMDQ_CmdqInterface_t* (*MHalChangeCmdqIF)(MHAL_CMDQ_CmdqInterface_t *pNew);
}LibcameraIF_t;

LibcameraIF_t* LibcameraIfGet(void);
void LibcameraIfInit(void(*fpInit)(LibcameraIF_t*));

#endif
