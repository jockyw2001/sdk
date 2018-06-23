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
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h> 
#include <linux/kthread.h>
#include <err.h> 

#include "mi_sys_internal.h"
#include "mi_display.h"



#define DBG_INFO(fmt, args...)      ({do{{printf(ASCII_COLOR_GREEN);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})
#define DBG_ERR(fmt, args...)       ({do{{printf(ASCII_COLOR_RED"[MI ERR ]: %s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}}while(0);})

typedef enum
{
    MI_OK = 0x0,                   ///< succeeded
    MI_FAIL,                  ///< Error
}MI_RESULT;

typedef struct mi_disp_InputPortStatus_s
{
    MI_DISP_VidWinRect_t stDispWin;
    MI_BOOL bPause;
    MI_BOOL bEnable;
    MI_DISP_SyncMode_e eMode;

    //Check PTS
    MI_U64 u64FiredTimeStamp;
    MI_U64 u64DiffPts;
    MI_BOOL bFrameBefore;

    //Hal layer instance
    void* apInputObjs;

    struct semaphore stPortMutex; 
}mi_disp_InputPortStatus_t;

typedef struct mi_disp_LayerStatus_s
{
   //Video Layer properities
   MI_U8 u8LayerID;
   MI_BOOL  bLayerEnabled;
   MI_U32     u32LayerWidth;
   MI_U32     u32LayerHeight;
   MI_DISP_PixelFormat_e    ePixFormat;         /* Pixel format of the video layer */
   MI_DISP_VidWinRect_t     stVidLayerDispWin;                    /* Display resolution */
   MI_BOOL bCompress;
   MI_U32 u32Priority;
   MI_U32 u32Toleration;
   MI_U8 u8BindedDevID;

   //Input port status in video Layer
   mi_disp_InputPortStatus_t astPortStatus[MI_DISP_INPUTPORT_MAX]; 

   //Input port pending buffer
   struct mutex stDispLayerPendingQueueMutex;
   list_head stPortPendingBufQueue[MI_DISP_INPUTPORT_MAX];
   
   MI_SYS_BufInfo_t *pstOnScreenBufInfo[MI_DISP_INPUTPORT_MAX];
   MI_SYS_BufInfo_t *pstCurrentFiredBufInfo[MI_DISP_INPUTPORT_MAX];

   //Hal layer instance
   void* apLayerObjs;

   //Layer List node
   struct list_head stLayerNode;

   struct semaphore stLayerMutex; 
}mi_disp_LayerStatus_t;

typedef enum
{
    MI_DISP_INTFACE_CVBS,
    MI_DISP_INTFACE_YPBPR,
    MI_DISP_INTFACE_VGA,
    MI_DISP_INTFACE_BT656,
    MI_DISP_INTFACE_BT1120,
    MI_DISP_INTFACE_HDMI,
    MI_DISP_INTFACE_LCD,
    MI_DISP_INTFACE_BT656_H,
    MI_DISP_INTFACE_BT656_L,
    MI_DISP_INTFACE_BT656_L,
    MI_DISP_INTFACE_MAX,
}MI_DISP_Interface_e;

#define MI_DISP_OUTDEVICE_MAX 9
typedef struct MI_DISP_DEV_Status_s
{
   MI_BOOL bDISPEnabled;
   wait_queue_head_t  stWaitQueueHead;
   MI_U32 u32DevId;
   MI_SYS_DRV_HANDLE hDevSysHandle;

   //Binded layer list, Use List for extented
   struct list_head stBindedLayer;
   // TODO: layer size 统一放在dev 中处理
   MI_U32 u32SrcW; //layer width
   MI_U32 u32SrcH; //layer height
   
   MI_U32 u32BgColor;
   MI_DISP_Interface_e eIntfType;
   MI_DISP_IntfSync_e eDeviceTiming[MI_DISP_OUTDEVICE_MAX];

   //Hdmi or Vga
   MI_DISP_CscMattrix_e eCscMatrix;
   MI_U32 u32Luma;                     /* luminance:   0 ~ 100 default: 50 */
   MI_U32 u32Contrast;                 /* contrast :   0 ~ 100 default: 50 */
   MI_U32 u32Hue;                      /* hue      :   0 ~ 100 default: 50 */
   MI_U32 u32Saturation;               /* saturation:  0 ~ 100 default: 50 */

   //vga only
   MI_U32 u32Gain;                          /* current gain of VGA signals. [0, 64). default:0x30 */
   MI_U32 u32Sharpness;                /* For VGA signals*/

   //cvbs
   MI_BOOL bCvbsEnable;

   // Hal Dev instance
   void* pstDevObj;

   //Dev List node
   struct list_head stDevNode;

   //Dev Task
   struct task_struct* pstWorkTask;
   struct task_struct* pstIsrTask;

   //Check pts
   MI_U64 u64LastIntTimeStamp;
   MI_U64 u64CurIntTimeStamp;
   MI_U64 u64VsyncInterval;

   struct semaphore stDevMutex; 
}mi_disp_DevStatus_t;

static mi_disp_DevStatus_t* pstDevice0Param = NULL;
static mi_disp_DevStatus_t* pstDevice1Param = NULL;
static mi_disp_DevStatus_t astDevStatus[MI_DISP_DEV_MAX];
static mi_disp_LayerStatus_t astLayerParams[MI_DISP_LAYER_MAX];

// TODO: 收在dev / layer 里面
static MI_BOOL bCteateInstance[MI_DISP_LAYER_MAX] = {FALSE};

typedef struct MI_DISP_PortPendingBuf_s
{
    MI_SYS_BufInfo_t* pstInputBuffer;
    list_head stPortPendingBufNode;
}MI_DISP_PortPendingBuf_t;

#define MI_DISP_CHECK_NULLPOINTER(pointer)\
    {\
        if(pointer == NULL)\
        {\
            DBG_ERR("NULL pointer!!!\n");\
            goto EXIT;\
        }\
    }
#define MI_DISP_CHECK_INVAILDDEV(Dev)\
    {\
        if(DispDev > MI_DISP_DEV_MAX)\
        {\
            DBG_ERR("Invild Display Dev!!!\n");\
            goto EXIT;\
        }\
    }
#define MI_DISP_CHECK_INVAILDLAYER(Layer)\
        {\
            if(Layer > MI_DISP_LAYER_MAX)\
            {\
                DBG_ERR("Invild Display Layer!!!\n");\
                goto EXIT;\
            }\
        }
#define MI_DISP_CHECK_INVAILDINPUTPORT(Port)\
            {\
                if(Port > MI_DISP_INPUTPORT_MAX)\
                {\
                    DBG_ERR("Invild Input Port!!!\n");\
                    goto EXIT;\
                }\
            }

MI_S32  MI_DISP_Enable(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("Enable Display Dev %d !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    down(&(astDevStatus[DispDev].stDevMutex));

    if(HalDispDeviceEnable(&apDevObjs[DispDev], TRUE) != TRUE)
    {
        DBG_ERR("Hal Enable Display Dev Fail!!!\n");
        goto UP_DEV_MUTEX;
    }

    astDevStatus[DispDev].bDISPEnabled = TRUE;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT:
    return s32Ret;
}

MI_S32  MI_DISP_Disable(MI_DISP_DEV DispDev)
{
    MI_s32 s32Ret = MI_FAIL;
    DBG_INFO("Disable Display Dev %d !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);

    down(&(astDevStatus[DispDev].stDevMutex));
    if(!list_empty(&(astDevStatus[DispDev].stBindedLayer))
    {
        struct list_head *pstPos = NULL;
        struct mi_disp_LayerStatus_t *pstTmpLayer = NULL;
        list_for_each(pstPos, &(astDevStatus[u8DevId].stBindedLayer))
        {
            pstTmpLayer = list_entry(pstPos, struct mi_disp_LayerStatus_t, stLayerNode);
            down(&(pstTmpLayer->stLayerMutex));
            if(pstTmpLayer->bLayerEnabled)
            {
                up(&(pstTmpLayer->stLayerMutex));
                goto UP_DEV_MUTEX;
            }
            up(&(pstTmpLayer->stLayerMutex));
        }
    }
    if(HalDispDeviceEnable(&(astDevStatus[DispDev].pstDevObj), FALSE) != TRUE)
    {
        DBG_ERR("Hal Disable Display Dev Fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].bDISPEnabled = FALSE;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT:
    return s32Ret;
    
}


MI_S32  MI_DISP_SetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_s32 s32Ret = MI_FAIL;
    HalDispDeviceTimingInfo_t stDevTimingInfo;
    DBG_INFO("Set Display dev %d, Public Attrs !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstPubAttr);
    down(&(astDevStatus[DispDev].stDevMutex));
    if(HalDispDeviceSetBackGroundColor(&(astDevStatus[DispDev].pstDevObj), pstPubAttr->u32BgColor) != TRUE)
    {
        DBG_ERR("Hal Set backcolor fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].u32BgColor = pstPubAttr->u32BgColor;
    
    if(HalDispDeviceAddOutInterace(&(astDevStatus[DispDev].pstDevObj), pstPubAttr->u32IntfType) != TRUE)
    {
        DBG_ERR("Hal Add Output device fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].u32IntfType;
    memset(&stDevTimingInfo, 0, sizeof(HalDispDeviceTimingInfo_t));
    stDevTimingInfo.eDeviceType = pstPubAttr->eIntfType;
    stDevTimingInfo.eTimeType = pstPubAttr->eIntfSync;
    stDevTimingInfo.pstSyncInfo = pstPubAttr->stSyncInfo;
    if(!HalDispDeviceSetOutputTiming(&(astDevStatus[DispDev].pstDevObj), &stDevTimingInfo));
    {
        DBG_ERR("Hal Set Output Timing fail!!!\n");
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].eDeviceTiming[stDevTimingInfo.eTimeType] = pstPubAttr->eIntfSync;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT:
    return s32Ret;
}

MI_S32  MI_DISP_GetPubAttr(MI_DISP_DEV DispDev, const MI_DISP_PubAttr_t *pstPubAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    MI_U32 u32BackColor = 0;
    MI_U32 u32Interface = 0;
    HalDispDeviceTimingInfo_t stDevTimingInfo;
    memset(&stDevTimingInfo, 0, sizeof(HalDispDeviceTimingInfo_t));
    DBG_INFO("Get Display dev %d, Public Attrs !\n", DispDev);
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstPubAttr);

    down(&(astDevStatus[DispDev].stDevMutex));
    if(astDevStatus[DispDev].bDISPEnabled == TRUE)
    {
        pstPubAttr->u32BgColor = astDevStatus[DispDev].u32BgColor;
        pstPubAttr->u32IntfType = astDevStatus[DispDev].u32IntfType;
    }
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT:
    return s32Ret;

}

MI_S32 MI_DISP_DeviceAttach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("Attach Dev %d to Dest Dev %d !\n", DispSrcDev, DispDstDev);
    MI_DISP_CHECK_INVAILDDEV(DispSrcDev);
    MI_DISP_CHECK_NULLPOINTER(DispDstDev);
    down(&(astDevStatus[DispSrcDev].stDevMutex));
    down(&(astDevStatus[DispDstDev].stDevMutex));
    if(!(astDevStatus[DispSrcDev].bDISPEnabled && astDevStatus[DispDstDev].bDISPEnabled))
    {
        DBG_ERR("Dest or Src Dev not Enabled!!!\n!!!");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    if(!list_empty(&(astDevStatus[DispDstDev].stBindedLayer))
    {
        DBG_ERR("Layer binded DstDev already!!!\n!!!");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }

    if(!HalDispDeviceAttach(&(astDevStatus[DispSrcDev].pstDevObj), &(astDevStatus[DispDstDev].pstDevObj))
    {
        DBG_ERR("Hal Attach fail!!!\n");
        s32Ret = MI_FAIL
        goto UP_DEV_MUTEX;
    }
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispSrcDev].stDevMutex));
    up(&(astDevStatus[DispDstDev].stDevMutex));
EXIT:
    return s32Ret;   
}

MI_S32 MI_DISP_DeviceDetach(MI_DISP_DEV DispSrcDev, MI_DISP_DEV DispDstDev)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("Attach Dev %d to Dest Dev %d !\n", DispSrcDev, DispDstDev);
    MI_DISP_CHECK_INVAILDDEV(DispSrcDev);
    MI_DISP_CHECK_NULLPOINTER(DispDstDev);
    down(&(astDevStatus[DispSrcDev].stDevMutex));
    down(&(astDevStatus[DispDstDev].stDevMutex));

    if(!(astDevStatus[DispSrcDev].bDISPEnabled && astDevStatus[DispDstDev].bDISPEnabled))
    {
        DBG_ERR("Dest or Src Dev not Enabled!!!\n!!!");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }

    if(HalDispDeviceDeattach(apDevObjs[DispSrcDev], apDevObjs[DispDstDev]))
    {
        DBG_ERR("Hal Attach fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispSrcDev].stDevMutex));
    up(&(astDevStatus[DispDstDev].stDevMutex));
EXIT:
    return s32Ret;
}

MI_S32  MI_DISP_EnableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_FAIL;
    HalAllocPhyMem_t stHalAlloc;
    MI_U8 u8PortCount = 0;
    DBG_INFO("MI_DISP_EnableVideoLayer, VideoLayer = %d !\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    //
    mi_sys_enableChannel();
    if(!HalDispVideoLayerEnable(&(astLayerParams[DispLayer].apLayerObjs), TRUE))
    {
        DBG_ERR("Enable Video Layer Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;
    }
    astLayerParams[DispLayer].bLayerEnabled = TRUE;

    stHalAlloc.free = MI_MMA_Free;
    stHalAlloc.alloc = MI_MMA_Alloc;
    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        if(!HalDispInputPortCreateInstance(&stHalAlloc, &(astLayerParams[DispLayer].apLayerObjs), &(astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs)))
        {
            DBG_ERR("InputPort Create Instance fail!!!\n");
            continue;
        }
    }
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:
    return s32Ret;

}

MI_S32  MI_DISP_DisableVideoLayer(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_FAIL;
    MI_U8 u8PortCount = 0;
    DBG_INFO("MI_DISP_EnableVideoLayer, VideoLayer = %d !\n", DispLayer);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    down(&(astLayerParams[DispLayer].stLayerMutex));

    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        if(!HalDispInputPortDestroyInstance(&(astLayerParams[DispLayer].astPortStatus[u8PortCount].apInputObjs)))
        {
            DBG_ERR("InputPort Destory Instance fail!!!\n");
            continue;
        }
    }
    if(bCteateInstance[DispLayer] == TRUE)
    {
        if(!HalDispVideoLayerEnable(&(astLayerParams[DispLayer].apLayerObjs), FALSE))
        {
            DBG_ERR("Disable Video Layer Fail!!!\n");
            s32Ret = MI_FAIL;
            goto UP_LAYER_MUTEX;
        }
        bCteateInstance[DispLayer] == FALSE;
    }
    astLayerParams[DispLayer].bLayerEnabled = FALSE;

    if(!HalDispVideoLayerDestoryInstance(&(astLayerParams[DispLayer].apLayerObjs)))
    {
        DBG_ERR("Layer Destory Instance fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;

    }
    // TODO: sys 开出关闭channel 的接口，内部清掉Input port info
    mi_sys_disableChannel();
    for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
    {
        memset(&(astLayerParams[DispLayer].astPortStatus[u8PortCount]), 0, sizeof(mi_disp_InputPortStatus_s));
        astLayerParams[DispLayer].pstOnScreenBufInfo[u8PortCount] = NULL;
        astLayerParams[DispLayer].pstCurrentFiredBufInfo[u8PortCount] = NULL;
        down(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
        if(!list_empty(astLayerParams[DispLayer].stPortPendingBufQueue[u8PortCount]))
        {
            list_head *pstPendingBufferPos;
            MI_DISP_PortPendingBuf_t *pstTmpPendingBuf;
            list_for_each(pstPendingBufferPos, &(astLayerParams[DispLayer].stPortPendingBufQueue[u8PortCount]))
            {
                pstTmpPendingBuf = list_entry(pstPendingBufferPos, struct MI_DISP_PortPendingBuf_t, stPortPendingBufNode);
                list_del(pstTmpPendingBuf);
                MI_SYS_BufInfo_t* pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                kfree(pstTmpPendingBuf);
            }
        }
        up(&(astLayerParams[DispLayer].stDispLayerPendingQueueMutex));
    }//Go through input port in Video Layer

UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:
    return s32Ret;
}

MI_S32  MI_DISP_SetVideoLayerCompressAttr(MI_DISP_LAYER DispLayer, const MI_DISP_CompressAttr_t *pstCompressAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispCompressAttr_t stHalDispCompressAttr;
    DBG_INFO("MI_DISP_SetVideoLayerCompressAttr, bEnable = %d !\n", pstCompressAttr->bSupportCompress);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstCompressAttr);

    memset(&stHalDispCompressAttr, 0, sizeof(HalDispCompressAttr_t));
    stHalDispCompressAttr.bEnbale = pstCompressAttr.bSupportCompress;
    down(&(astLayerParams[DispLayer].stLayerMutex));
    if(!HalDispVideoLayerSetCompress(&(astLayerParams[DispLayer].apLayerObjs), &stHalDispCompressAttr))
    {
        DBG_ERR("Disable Video Layer Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;
    }
    astLayerParams[DispLayer].bCompress = pstCompressAttr.bSupportCompress;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:
    return s32Ret;
}

MI_S32  MI_DISP_GetVideoLayerCompressAttr(MI_DISP_LAYER DispLayer, const MI_DISP_CompressAttr_t *pstCompressAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetVideoLayerCompressAttr, bEnable = %d !\n", pstCompressAttr->bSupportCompress);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstCompressAttr);

    down(&(astLayerParams[DispLayer].stLayerMutex));
    pstCompressAttr->bSupportCompress = astLayerParams[DispLayer].bCompress;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_SetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispVideoLayerAttr_t stHalLayerAttr;
    DBG_INFO("MI_DISP_SetVideoLayerAttr!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstLayerAttr);
    down(&(astDevStatus[u32DevId].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));

    // TODO: 收在dev enable 里面
    if(bCteateInstance[DispLayer] == FALSE)
    {
        HalAllocPhyMem_t stHalAlloc;
        stHalAlloc.free = MI_MMA_Free;
        stHalAlloc.alloc = MI_MMA_Alloc;
        if(!HalDispVideoLayerCreateInstance(&stHalAlloc, DispLayer, &(astLayerParams[DispLayer].apLayerObjs)))
        {
            DBG_ERR("HalDispDeviceCreateInstance fail!!!\n");
            s32Ret = MI_FAIL;
            goto UP_DEV_LAYER_MUTEX;
        }
        bCteateInstance[DispLayer] = TRUE;
    }

    memset(&stHalLayerAttr, 0, sizeof(HalDispVideoLayerAttr_t));
    stHalLayerAttr = *((HalDispVideoLayerAttr_t*)pstLayerAttr);
    if(!HalDispVideoLayerSetAttr(&(astLayerParams[DispLayer].apLayerObjs), &stHalLayerAttr))
    {
        DBG_ERR("HalDispVideoLayerSetAttr Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_LAYER_MUTEX;
    }

    //Init Layer list
    INIT_LIST_HEAD(astLayerParams[DispLayer].stLayerList);
    astLayerParams[DispLayer].u32LayerWidth = pstLayerAttr->stVidLayerSize.u32Width;
    astLayerParams[DispLayer].u32LayerHeight = pstLayerAttr->stVidLayerSize.u32Height;
    astLayerParams[DispLayer].ePixFormat = pstLayerAttr->ePixFormat;
    astLayerParams[DispLayer].stVidLayerDispWin = pstLayerAttr->stVidLayerDispWin;
    s32Ret = MI_OK;
UP_DEV_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
    up(&(astDevStatus[u32DevId].stDevMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_GetVideoLayerAttr(MI_DISP_LAYER DispLayer, const MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetVideoLayerAttr!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstLayerAttr);

    down(&(astDevStatus[u32DevId].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));

    pstLayerAttr->stVidLayerSize.u32Width = astLayerParams[DispLayer].u32LayerWidth;
    pstLayerAttr->stVidLayerSize.u32Height = astLayerParams[DispLayer].u32LayerHeight;
    pstLayerAttr->ePixFormat = astLayerParams[DispLayer].ePixFormat;
    pstLayerAttr->stVidLayerDispWin = astLayerParams[DispLayer].stVidLayerDispWin;
    s32Ret = MI_OK;
UP_DEV_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
    up(&(astDevStatus[u32DevId].stDevMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_BindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_BindVideoLayer!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);

    down(&(astDevStatus[DispDev].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));

    if(!HalDispVideoLayerBind(&(astLayerParams[DispLayer].apLayerObjs), &(astDevStatus[DispDev].pstDevObj)))
    {
        DBG_ERR("Bind Video Layer  Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_LAYER_MUTEX;
    }

    list_add_tail(&(astLayerParams[DispLayer].stLayerList), &(astDevStatus[DispDev].stBindedLayer));
    astLayerParams[DispLayer].u8BindedDevID = DispDev;
    s32Ret = MI_OK;
UP_DEV_LAYER_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}
MI_S32  MI_DISP_UnBindVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_FAIL;
    struct list_head *pstPos;
    struct mi_disp_LayerStatus_t *pstTmpLayer;  
    DBG_INFO("MI_DISP_UnBindVideoLayer!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);

    down(&(astDevStatus[DispDev].stDevMutex));
    down(&(astLayerParams[DispLayer].stLayerMutex));
    if(!HalDispVideoLayerUnBind(&(astLayerParams[DispLayer].apLayerObjs), &(astDevStatus[DispDev].pstDevObj)))
    {
        DBG_ERR("Unbind Video Layer  Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_LAYER_MUTEX;
    }
    list_for_each(pstPos, &(astDevStatus[DispDev].stBindedLayer))
    {
        pstTmpLayer = list_entry(pstPos, struct mi_disp_LayerStatus_t, stLayerList);
        if(pstTmpLayer->u8LayerID == DispLayer)
        {
            list_del(pstTmpLayer.stLayerList);
        }
    }
    //Unbinded 
    astLayerParams[DispLayer].u8BindedDevID = 0xff;
UP_DEV_LAYER_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_SetVideoLayerPriority(MI_DISP_LAYER DispLayer, MI_U32 u32Priority)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetVideoLayerPriority!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    if(!HalDispVideoLayerSetPriority(&(astLayerParams[DispLayer].apLayerObjs), u32Priority)
    {
        DBG_ERR("Set Video Layer priority Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;
    }

    astLayerParams[DispLayer].u32Priority = u32Priority;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}
MI_S32  MI_DISP_GetVideoLayerPriority(MI_DISP_LAYER DispLayer, MI_U32 *pu32Priority)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetVideoLayerPriority!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pu32Priority);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    *pu32Priority = astLayerParams[DispLayer].u32Priority;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_SetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 u32Toleration)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetPlayToleration!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    astLayerParams[DispLayer].u32Toleration = u32Toleration;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}
MI_S32  MI_DISP_GetPlayToleration(MI_DISP_LAYER DispLayer, MI_U32 *pu32Toleration)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetPlayToleration!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pu32Toleration);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    *pu32Toleration = astLayerParams[DispLayer].u32Toleration;
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}

static MI_BOOL bTimeOut = FALSE;
void mi_disp_GetScreenFrameTimer(void *data)
{
    DBG_INFO("mi_disp_GetScreenFrameTimer!\n");
    MI_U32 u32MilliSec = *((MI_U32*)data);
    msleep(u32MilliSec);
    bTimeOut = TRUE;
}

typedef struct mi_divp_CaptureInfo_s
{
    MI_PHY u32BufAddr[3];//output
    MI_PHY u32BufSize;//output
    MI_U16 u16Width;//output
    MI_U16 u16Height;//output
    MI_U16 u16Stride[3];//output(byte)
    MI_SYS_PixelFormat_e ePxlFmt;//output
    mi_divp_CapStage_e stCapStage;//input
    MI_DIVP_Rotate_e eRota;//input
    mi_divp_Mirror_t stMirror;//input
    mi_divp_Window_t stCapWin;//input
    MI_BOOL bReturnValue;
    void (*pFun)(void *);
} mi_divp_CaptureInfo_t;

static MI_BOOL bCapFrameDone = FALSE;

void mi_disp_GetCapFrameStatus(void)
{
    bCapFrameDone = TRUE;
}

typedef void (*pFun)(void *);

static MI_SYS_DRV_HANDLE hSysHandle[MI_DISP_DEV_MAX];
MI_S32 MI_DISP_GetScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame, MI_U32  u32MilliSec)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetScreenFrame!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_NULLPOINTER(pstVFrame);

    bTimeOut = FALSE;
    bCapFrameDone = FALSE;
    mi_divp_CaptureInfo_t stCapInfo;
    memset(&stCapInfo, 0, sizeof(mi_divp_CaptureInfo_t));
    stCapInfo.ePxlFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    stCapInfo.stCapStage = E_MI_DIVP_CAP_STAGE_OUTPUT_WITH_OSD;
    //Get Sys buffer
    MI_U64 u64PhyAddr = 0;
    // TODO:  size 考虑到对齐问题
    MI_MMA_Alloc(NULL, u32OutputTimingSize, &u64PhyAddr);
    //
    stCapInfo.eDispId = astLayerParams[DispLayer].u8BindedDevID;
    stCapInfo.u16Height = OutputTimingHeight;
    stCapInfo.u16Width = OutputTimingWidth;
    stCapInfo.u32BufAddr = u64PhyAddr;
    stCapInfo.u32BufSize = u32OutputTimingSize;
    // TODO: Tommy
    if(!mi_divp_CaptureTiming(&stCapInfo, mi_disp_GetCapFrameStatus))
    {
        DBG_ERR("Divp Cap Timing fail!!!\n");
        return MI_FAIL;
    }
    do
    {
        if(bTimeOut == TRUE)
        {
            break;
            eRet = MI_FAIL;
        }
        msleep(1);
    }while(bCapFrameDone == TRUE)

    // TODO: sky
    
    pstVFrame->aphyAddr[0] = stCapInfo.u32BufAddr;
    pstVFrame->aphyAddr[1] = stCapInfo.u32BufAddr;
    pstVFrame->aphyAddr[2] = stCapInfo.u32BufAddr;
    pstVFrame->ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    pstVFrame->pavirAddr[0] = 0;
    pstVFrame->pavirAddr[1] = 0;
    pstVFrame->pavirAddr[2] = 0;
    pstVFrame->u32Height = OutputTimingHeight;
    pstVFrame->u32Width = OutputTimingWidth;
    pstVFrame->u64pts = 0;
    return eRet;
}

MI_S32  MI_DISP_ReleaseScreenFrame(MI_DISP_LAYER DispLayer, MI_DISP_VideoFrame_t *pstVFrame)
{
    DBG_INFO("MI_DISP_ReleaseScreenFrame!\n");

    if(DispLayer > MI_DISP_LAYER_MAX)
    {
        DBG_ERR("Invild Video Layer!!!\n");
        return MI_FAIL;
    }
    if(pstVFrame == NULL)
    {
        DBG_ERR("NULL Pointer\n!!!");
        return MI_FAIL;
    }

    MI_MMA_Free(pstVFrame->aphyAddr);
    return MI_OK;
}

MI_S32  MI_DISP_SetVideoLayerAttrBegin(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetVideoLayerAttrBegin!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    down(&(astLayerParams[DispLayer].stLayerMutex));
    if(!HalDispInputPortAttrBegin(&(astLayerParams[DispLayer].apLayerObjs)))
    {
        DBG_ERR("HalDispInputPortAttrBegin Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;
    }
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}
// TODO: 在doc 中描述使用Case
MI_S32  MI_DISP_SetVideoLayerAttrEnd(MI_DISP_LAYER DispLayer)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetVideoLayerAttrEnd!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);

    down(&(astLayerParams[DispLayer].stLayerMutex));
    if(!HalDispInputPortAttrEnd(&(astLayerParams[DispLayer].apLayerObjs)))
    {
        DBG_ERR("HalDispInputPortAttrEnd Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_LAYER_MUTEX;
    }
    s32Ret = MI_OK;
UP_LAYER_MUTEX:
    up(&(astLayerParams[DispLayer].stLayerMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_SetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispInputPortAttr_t stHalInputPortAttr;
    DBG_INFO("MI_DISP_SetInputPortAttr!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstInputPortAttr);

    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    memset(&stHalInputPortAttr, 0, sizeof(HalDispInputPortAttr_t));
    stHalInputPortAttr.stDispWin = pstInputPortAttr->stDispWin;
    if(!HalDispInputPortSetAttr(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), &stHalInputPortAttr))
    {
        DBG_ERR("HalDispInputPortSetAttr!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin = pstInputPortAttr->stDispWin;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT:    
    return s32Ret;
}

MI_S32  MI_DISP_GetInputPortAttr(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetInputPortAttr!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstInputPortAttr);

    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    pstInputPortAttr->stDispWin = astInputPortStatus[DispLayer][LayerInputPort].stDispWin;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT:    
    return s32Ret;
}


MI_S32  MI_DISP_EnableInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_EnableInputPort!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);

    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(!HalDispInputPortEnable(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), TRUE))
    {
        DBG_ERR("HalDispInputPortEnable Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable = TRUE;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}
MI_S32  MI_DISP_DisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_DisableInputPort!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(!HalDispInputPortEnable(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs), FALSE))
    {
        DBG_ERR("HalDispInputPortEnable Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }

    //
    DeinitInputPortResource;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_SetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, const MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispInputPortAttr_t stHalInputPortAttr;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstDispPos);
    memset(&stHalInputPortAttr, 0, sizeof(HalDispInputPortAttr_t));
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    stHalInputPortAttr.stDispWin = astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin;
    stHalInputPortAttr.stDispWin.u16X = pstDispPos->u16X;
    stHalInputPortAttr.stDispWin.u16Y = pstDispPos->u16Y;
    if(!HalDispInputPortSetAttr(apInputObjs[DispLayer][LayerInputPort], &stHalInputPortAttr))
    {
        DBG_ERR("HalDispInputPortSetAttr Fail!!!\n");
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16X = pstDispPos->u16X;
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16Y = pstDispPos->u16Y;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_GetInputPortDispPos(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_Position_t *pstDispPos)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstDispPos);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    pstDispPos->u16X = astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16X;
    pstDispPos->u16Y = astLayerParams[DispLayer].astPortStatus[LayerInputPort].stDispWin.u16Y;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_PauseInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DBG_ERR("Input port Not enable!!!\n");
        goto UP_PORT_MUTEX;
    }
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].bPause = TRUE;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_ResumeInputPort (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DBG_ERR("Input port Not enable!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }

    astLayerParams[DispLayer].astPortStatus[LayerInputPort].bPause = FALSE;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_StepInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    DBG_ERR("Not Support!!!\n");
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_RefreshInputPort( MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortDispPos!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    DBG_ERR("Not Support!!!\n");
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_ShowInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_ShowInputPort!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);

    // TODO: 增加flow，有合适buffer 去打开Gwin
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    while(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] != NULL)
    {
        if(!HalDispInputPortShow(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs)))
        {
            DBG_ERR("HalDispInputPortShow Fail!!!\n");
            s32Ret = MI_FAIL;
            goto UP_PORT_MUTEX;
        }
    }
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_HideInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_HideInputPort!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    if(!HalDispInputPortHide(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].apInputObjs)))
    {
        DBG_ERR("HalDispInputPortHide Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_U64  MI_DISP_GetInputPortPts(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_U64 *pu64InputPortPts)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetInputPortPts!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pu64InputPortPts);

    // TODO: last frame pts
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort] != NULL)
    {
        *pu64InputPortPts = astLayerParams[DispLayer].pstOnScreenBufInfo[LayerInputPort].u64Pts;
    }
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_SetInputPortSyncMode (MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_SyncMode_e eMode)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortSyncMode!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);

    if((eMode <= E_MI_DISP_SYNC_MODE_INVALID) || (eMode >= E_MI_DISP_SYNC_MODE_NUM))
    {
        DBG_ERR("Invaild sync mode!!!\n");
        s32Ret = MI_FAIL;
        goto EXIT;
    }
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
    if(astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable != TRUE)
    {
        DBG_ERR("Input port Not enable!!!\n");
        s32Ret = MI_FAIL;
        goto UP_PORT_MUTEX;
    }
    // TODO: 在doc 中增加使用场景,Default check pts， 增加参数有效性检查
    astLayerParams[DispLayer].astPortStatus[LayerInputPort].eMode = eMode;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_QueryInputPortStat(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_QueryChanelStatus_t *pstStatus)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_SetInputPortSyncMode!\n");
    MI_DISP_CHECK_INVAILDLAYER(DispLayer);
    MI_DISP_CHECK_INVAILDINPUTPORT(LayerInputPort);
    MI_DISP_CHECK_NULLPOINTER(pstStatus);
    down(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));

    pstStatus.bEnable = astLayerParams[DispLayer].astPortStatus[LayerInputPort].bEnable;
    s32Ret = MI_OK;
UP_PORT_MUTEX:
    up(&(astLayerParams[DispLayer].astPortStatus[LayerInputPort].stPortMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_GetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = MI_FAIL;
    DBG_INFO("MI_DISP_GetVgaParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstVgaParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    if(!(astDevStatus[DispDev].u32IntfType != MI_DISP_INTF_VGA))
    {
        DBG_ERR("Not Support Vga output device In device %d\n!!!", DispDev);
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    pstVgaParam->stCsc.eCscMatrix = astDevStatus[DispDev].eCscMatrix;
    pstVgaParam->stCsc.u32Luma = astDevStatus[DispDev].u32Luma;
    pstVgaParam->stCsc.u32Contrast = astDevStatus[DispDev].u32Contrast;
    pstVgaParam->stCsc.u32Hue = astDevStatus[DispDev].u32Hue;
    pstVgaParam->stCsc.u32Saturation = astDevStatus[DispDev].u32Saturation;
    pstVgaParam->u32Gain = astDevStatus[DispDev].u32Gain;
    pstVgaParam->u32Sharpness = astDevStatus[DispDev].u32Sharpness;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_SetVgaParam(MI_DISP_DEV DispDev, MI_DISP_VgaParam_t *pstVgaParam)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispVgaParam_t stHalVgaParams;
    DBG_INFO("MI_DISP_SetVgaParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstVgaParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    if(!(astDevStatus[DispDev].u32IntfType != MI_DISP_INTF_VGA))
    {
        DBG_ERR("Not Support Vga output device In device %d\n!!!", DispDev);
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }

    memset(&stHalVgaParams, 0, sizeof(HalDispVgaParam_t));
    stHalVgaParams.stCsc.eCscMatrix = pstVgaParam->stCsc.eCscMatrix;
    stHalVgaParams.stCsc.u32Contrast = pstVgaParam->stCsc.u32Contrast;
    stHalVgaParams.stCsc.u32Hue = pstVgaParam->stCsc.u32Hue;
    stHalVgaParams.stCsc.u32Luma = pstVgaParam->stCsc.u32Luma;
    stHalVgaParams.stCsc.u32Saturation = pstVgaParam->stCsc.u32Saturation;
    stHalVgaParams.u32Gain = pstVgaParam->u32Gain;
    stHalVgaParams.u32Sharpness = pstVgaParam->u32Sharpness;
    if(!(HalDispDeviceSetVgaParam(&(astDevStatus[DispDev].pstDevObj), &stHalVgaParams))
    {
        DBG_ERR("HalDispDeviceSetVgaParam Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].eCscMatrix = pstVgaParam->stCsc.eCscMatrix;
    astDevStatus[DispDev].u32Luma = pstVgaParam->stCsc.u32Luma;
    astDevStatus[DispDev].u32Contrast = pstVgaParam->stCsc.u32Contrast;
    astDevStatus[DispDev].u32Hue = pstVgaParam->stCsc.u32Hue;
    astDevStatus[DispDev].u32Saturation = pstVgaParam->stCsc.u32Saturation;
    astDevStatus[DispDev].u32Gain = pstVgaParam->u32Gain;
    astDevStatus[DispDev].u32Sharpness = pstVgaParam->u32Sharpness;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_GetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispHdmiParam_t stHalHdmiParams;
    DBG_INFO("MI_DISP_GetHdmiParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstHdmiParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    if(!(astDevStatus[DispDev].u32IntfType != MI_DISP_INTF_HDMI))
    {
        DBG_ERR("Not Support HDMI output device In device %d\n!!!", DispDev);
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }

    pstHdmiParam->stCsc.eCscMatrix = astDevStatus[DispDev].eCscMatrix;
    pstHdmiParam->stCsc.u32Luma = astDevStatus[DispDev].u32Luma;
    pstHdmiParam->stCsc.u32Contrast = astDevStatus[DispDev].u32Contrast;
    pstHdmiParam->stCsc.u32Hue = astDevStatus[DispDev].u32Hue;
    pstHdmiParam->stCsc.u32Saturation = astDevStatus[DispDev].u32Saturation;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_SetHdmiParam(MI_DISP_DEV DispDev, MI_DISP_HdmiParam_t *pstHdmiParam)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispHdmiParam_t stHalHdmiParams;
    DBG_INFO("MI_DISP_GetHdmiParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstHdmiParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    if(!(astDevStatus[DispDev].u32IntfType != MI_DISP_INTF_HDMI))
    {
        DBG_ERR("Not Support HDMI output device In device %d\n!!!", DispDev);
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }

    memset(&stHalHdmiParams, 0, sizeof(HalDispHdmiParam_t));
    stHalHdmiParams.stCsc.eCscMatrix = pstHdmiParam->stCsc.eCscMatrix;
    stHalHdmiParams.stCsc.u32Contrast = pstHdmiParam->stCsc.u32Contrast;
    stHalHdmiParams.stCsc.u32Hue = pstHdmiParam->stCsc.u32Hue;
    stHalHdmiParams.stCsc.u32Luma = pstHdmiParam->stCsc.u32Luma;
    stHalHdmiParams.stCsc.u32Saturation = pstHdmiParam->stCsc.u32Saturation;
    if(!(HalDispDeviceSetHdmiParam(&(astDevStatus[DispDev].pstDevObj), &stHalHdmiParams))
    {
        DBG_ERR("HalDispDeviceSetHdmiParam\n!!!");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].eCscMatrix = pstHdmiParam->stCsc.eCscMatrix;
    astDevStatus[DispDev].u32Luma = pstHdmiParam->stCsc.u32Luma;
    astDevStatus[DispDev].u32Contrast = pstHdmiParam->stCsc.u32Contrast;
    astDevStatus[DispDev].u32Hue = pstHdmiParam->stCsc.u32Hue;
    astDevStatus[DispDev].u32Saturation = pstHdmiParam->stCsc.u32Saturation;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_GetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = MI_FAIL;
    HalDispHdmiParam_t stHalHdmiParams;
    DBG_INFO("MI_DISP_GetCvbsParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstCvbsParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    pstCvbsParam->bEnable = astDevStatus[DispDev].bCvbsEnable;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_S32  MI_DISP_SetCvbsParam(MI_DISP_DEV DispDev, MI_DISP_CvbsParam_t *pstCvbsParam)
{
    MI_S32 s32Ret = MI_FAIL;
    MI_DISP_CvbsParam_t stHalCvbsParams;
    DBG_INFO("MI_DISP_SetCvbsParam !!!\n");
    MI_DISP_CHECK_INVAILDDEV(DispDev);
    MI_DISP_CHECK_NULLPOINTER(pstCvbsParam);
    down(&(astDevStatus[DispDev].stDevMutex));

    memset(&stHalCvbsParams, 0, sizeof(MI_DISP_CvbsParam_t));
    stHalCvbsParams.bEnable = pstCvbsParam->bEnable;
    if(!(HalDispDeviceSetCvbsParam(&(astDevStatus[DispDev].pstDevObj), &stHalCvbsParams))
    {
        DBG_ERR("HalDispDeviceSetCvbsParam Fail!!!\n");
        s32Ret = MI_FAIL;
        goto UP_DEV_MUTEX;
    }
    astDevStatus[DispDev].bCvbsEnable = pstCvbsParam->bEnable;
    s32Ret = MI_OK;
UP_DEV_MUTEX:
    up(&(astDevStatus[DispDev].stDevMutex));
EXIT: 
    return s32Ret;
}

MI_BOOL mi_display_SuitableDispWin(MI_SYS_BufInfo_t* pstFrameBufInfo, mi_disp_LayerStatus_t* pstLayerInfo)
{
    if((pstFrameBufInfo == NULL) || pstLayerInfo)
    {
        DBG_ERR("NULL Pointer, mi_display_SuitableDispWin!!!\n");
        return FALSE;
    }
    MI_BOOL bSuitable = FALSE;
    bSuitable = (pstFrameBufInfo->Buf_Info.dataFrame.u32Width == pstLayerInfo->u32LayerWidth)
        && (pstFrameBufInfo->Buf_Info.dataFrame.u32Height == pstLayerInfo->u32LayerHeight);
    return bSuitable;
}

MI_U64 mi_display_ResetPts(MI_U64 u64FiredTimeStamp, MI_U64 u64FramePts, MI_BOOL* bBefore)
{
    MI_U64 u64Diff = 0;
    if(bBefore == NULL)
    {
        DBG_ERR("NULL Pointer, mi_display_CalDiffPts!!!\n");
        return FALSE;
    }

    if(u64FiredTimeStamp > u64FramePts)
    {
        u64Diff = u64FiredTimeStamp - u64FramePts;
        *bBefore = TRUE;
    }
    else
    {
        u64Diff = u64FramePts - u64FiredTimeStamp;            
        *bBefore = FALSE;
    }
    return u64Diff;
}

MI_U64 mi_display_CalVsyncInterval(MI_U64 u64CurIntTimeStamp, MI_U64 u64LastIntTimeStamp, MI_U64 u64BaseInterval)
{
    MI_U64 u64VsyncInterval = 0;
    if(u64CurIntTimeStamp = u64LastIntTimeStamp)
    {
        u64VsyncInterval = u64BaseInterval;
    }
    else
    {
        //Current interval 3/4, history interval 1/4
        u64VsyncInterval =  ((u64CurIntTimeStamp - u64LastIntTimeStamp) * 3 + u64VsyncInterval) >> 2;
    }
    return u64VsyncInterval;
}

typedef enum
{
    E_MI_DISP_FRAME_NORMAL,
    E_MI_DISP_FRAME_DROP,
    E_MI_DISP_FRAME_SHOW_LAST,
}MI_DISP_HandleFrame_e;

MI_DISP_HandleFrame_e mi_display_CheckInputPortPts(MI_SYS_BufInfo_t* pstFrameBuf, mi_disp_InputPortStatus_s* pstPortStatus, mi_disp_DevStatus_t* pstDevStatus, MI_U32 TolerationMs)
{
    MI_U64 u64NextPts = 0;
    if(pstPortStatus->u64FiredDiff == 0)
    {
        pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstPortStatus->u64FiredTimeStamp, pstFrameBuf->u64FramePts, &(pstPortStatus->bFramePtsBefore));
    }
    pstDevStatus->u64VsyncInterval = ((pstDevStatus->u64CurrentIntTimeStamp - pstDevStatus->u64LastIntTimeStamp) * 3 + pstDevStatus->u64VsyncInterval) >> 2;
    if(pstPortStatus->bFramePtsBefore == TRUE)
    {
        u64NextPts = pstDevStatus->u64LastIntTimeStamp + pstPortStatus->u64FiredDiff + pstDevStatus->u64VsyncInterval;
    }
    else
    {
        u64NextPts = pstDevStatus->u64LastIntTimeStamp - pstPortStatus->u64FiredDiff + pstDevStatus->u64VsyncInterval;
    }

    if(pstFrameBuf->u64FramePts > u64NextPts + TolerationMs)
    {
        pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstPortStatus->u64FiredTimeStamp, pstFrameBuf->u64FramePts, &(pstPortStatus->bFramePtsBefore));
        return E_MI_DISP_FRAME_SHOW_LAST;
    }
    else if(pstFrameBuf->u64FramePts < u64NextPts - TolerationMs)
    {
        pstPortStatus->u64FiredDiff = mi_display_ResetPts(pstPortStatus->u64FiredTimeStamp, pstFrameBuf->u64FramePts, &(pstPortStatus->bFramePtsBefore));
        return E_MI_DISP_FRAME_NORMAL;
    }
    else if((pstFrameBuf->u64FramePts < u64NextPts) || (pstFrameBuf->u64FramePts > u64NextPts - TolerationMs))
    {
        return E_MI_DISP_FRAME_DROP;
    }
    else if((pstFrameBuf->u64FramePts > u64NextPts) || (pstFrameBuf->u64FramePts < u64NextPts + TolerationMs))
    {
        return E_MI_DISP_FRAME_NORMAL;
    }
    return E_MI_DISP_FRAME_NORMAL;
}

void Disp_Dev_ISR_Thread(void *pdata)
{
    MI_U8 u8DevId = 0;
    mi_disp_DevStatus_t* pstDispDevParam = (mi_disp_DevStatus_t*)data;
    if(pdata == NULL)
    {
        DBG_ERR("NULL Pointer, Disp_Dev_Work_Thread Return!!!\n");
        return;
    }
    while(1)
    {
        interruptible_sleep_on_timeout(&(pstDispDevParam->stWaitQueueHead), 10);
        down(&(pstDispDevParam->stDevMutex));
        u8DevId = pstDispDevParam->u32DevId;
        if(!pstDispDevParam->bDISPEnabled)
        {
            continue;
        }

        if(!list_empty(pstDispDevParam->stBindedVideoLayer)
        {
            struct list_head *pstPos = NULL;
            struct mi_disp_LayerStatus_t *pstTmpLayer = NULL;
            MI_U8 u8LayerID = 0;
            list_for_each(pstPos, &(astDevStatus[u8DevId].stBindedLayer))
            {
                MI_U8 u8PortCount = 0;
                pstTmpLayer = list_entry(pstPos, struct mi_disp_LayerStatus_t, stLayerNode);
                u8LayerID = pstTmpLayer->u8LayerID;
                down(&(pstTmpLayer->stLayerMutex));
                if(!HalDispVideoLayerCheckBufferFired(apLayerObjs[u8LayerID]))
                {
                    up(&(pstTmpLayer->stLayerMutex));
                    continue;
                }

                for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
                {
                   //Update Screen  buffer
                    if(pstTmpLayer->pstCurrentFiredBufInfo[u8PortCount])
                    {
                        pstTmpLayer->pstOnScreenBufInfo[u8PortCount] = pstTmpLayer->pstOnScreenBufInfo[u8PortCount];
                        pstTmpLayer->pstCurrentFiredBufInfo[u8PortCount] = NULL;
                    }
                    if((!pstTmpLayer->astPortStatus[u8PortCount].bEnable) && (pstTmpLayer->pstOnScreenBufInfo[u8PortCount]))
                    {
                        MI_SYS_ReleaseInputPortBuf(pstTmpLayer->pstOnScreenBufInfo[u8PortCount]);
                        pstTmpLayer->pstOnScreenBufInfo[u8PortCount] = NULL;
                    }
                    if(!list_empty(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                    {
                        list_head *pstPendingBufferPos;
                        MI_DISP_PortPendingBuf_t *pstTmpPendingBuf;
                        down(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                        list_for_each(pstPendingBufferPos, &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                        {
                            MI_DISP_HandleFrame_e eHandleFrame = E_MI_DISP_FRAME_NORMAL;
                            pstTmpPendingBuf = list_entry(pstPendingBufferPos, struct MI_DISP_PortPendingBuf_t, stPortPendingBufNode);
                            list_del(pstTmpPendingBuf);
                            MI_SYS_BufInfo_t* pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                            kfree(pstTmpPendingBuf);
                            if(astLayerParams[u8LayerID].astPortStatus[u8PortCount].bPause)
                            {
                                if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_OK)
                                {
                                    DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                }
                                up(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                                continue;
                            }
                            eHandleFrame = mi_display_CheckInputPortPts(pstTmpInputBuffer, &(astLayerParams[u8LayerID].astPortStatus[u8PortCount], pstDispDevParam, astLayerParams[u8LayerID].u32Toleration))
                            if(eHandleFrame == E_MI_DISP_FRAME_DROP)
                            {
                                if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_OK)
                                {
                                    DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                }
                                up(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                                continue;
                            }
                            else if(eHandleFrame == E_MI_DISP_FRAME_SHOW_LAST)
                            {
                                while(pstPendingBufferPos->next != &(pstTmpLayer->stPortPendingBufQueue[u8PortCount]))
                                {
                                    pstPendingBufferPos = pstPendingBufferPos->next;
                                    pstTmpPendingBuf = list_entry(pstPendingBufferPos, struct MI_DISP_PortPendingBuf_t, stPortPendingBufNode);
                                    list_del(pstTmpPendingBuf);
                                    pstTmpInputBuffer = pstTmpPendingBuf->pstInputBuffer;
                                    kfree(pstTmpPendingBuf);
                                    if(mi_sys_FinishBuf(pstTmpInputBuffer) != MI_OK)
                                    {
                                        DBG_ERR("mi_sys_FinishBuf Fail!!!\n");
                                    }
                                    break;
                                }
                            }
                            else if(eHandleFrame = E_MI_DISP_FRAME_NORMAL)
                            {
                                //Normal case
                            }
                                
                            if(mi_display_SuitableDispWin(pstTmpInputBuffer))
                            {
                                struct timespec sttime;
                                HalDispVideoFrameData_t stHalFrameData;
                                memset(&sttime, 0, sizeof(&sttime));
                                memset(&stHalFrameData, 0, sizeof(HalDispVideoFrameData_t));
                                stHalFrameData.eCompressMode = pstTmpInputBuffer->Buf_Info.dataFrame.eCompressMode;
                                stHalFrameData.ePixelFormat = pstTmpInputBuffer->Buf_Info.dataFrame.ePixelFormat;
                                stHalFrameData.PhyAddr[0] = pstTmpInputBuffer->Buf_Info.dataFrame.u64PhyAddr[0];
                                stHalFrameData.PhyAddr[1] = pstTmpInputBuffer->Buf_Info.dataFrame.u64PhyAddr[1];
                                stHalFrameData.PhyAddr[2] = pstTmpInputBuffer->Buf_Info.dataFrame.u64PhyAddr[2];
                                stHalFrameData.u32Height = pstTmpInputBuffer->Buf_Info.dataFrame.u32Height;
                                stHalFrameData.u32Width = pstTmpInputBuffer->Buf_Info.dataFrame.u32Width;
                                stHalFrameData.u32Stride[0] = pstTmpInputBuffer->Buf_Info.dataFrame.u32Stride[0];
                                stHalFrameData.u32Stride[1] = pstTmpInputBuffer->Buf_Info.dataFrame.u32Stride[1];
                                stHalFrameData.u32Stride[2] = pstTmpInputBuffer->Buf_Info.dataFrame.u32Stride[2];
                                if(!HalDispInputPortFlip(apInputObjs[u8LayerID][u8PortCount], &stHalFrameData))
                                {
                                    DBG_ERR("mi_sys_FinishBuf Failed, the buffer Droped!!!\n");
                                }
                                do_posix_clock_monotonic_gettime(&sttime);
                                astLayerParams[u8LayerID][u8PortCount].u64FiredTimeStamp = sttime.tv_sec * 1000 * 1000 + (sttime.tv_nsec / 1000);
                                break;
                            }
                            MI_SYS_ReleaseInputPortBuf(pstTmpPendingBuf->pstTmpInputBuffer);
                            up(&(pstTmpLayer->stDispLayerPendingQueueMutex));
                        }
                     }
                }
                up(&(pstTmpLayer->stLayerMutex));
            }
        }
        up(&(pstDispDevParam->stDevMutex));
    }
}

void Disp_Dev_Work_Thread(void *pdata)
{
    MI_U8 u8DevId = 0;
    mi_disp_DevStatus_t* pstDispDevParam = NULL
    if(pdata == NULL)
    {
        DBG_ERR("NULL Pointer, Disp_Dev_Work_Thread Return!!!\n");
        return;
    }
    
    pstDispDevParam = (mi_disp_DevStatus_t*)data;
    u8DevId = pstDispDevParam->u32DevId;
    while(1)
    {
        interruptible_sleep_on_timeout(&pstDispDevParam->stWaitQueueHead, 10);
        if(mi_sys_WaitOnInputTaskAvailable(pstDispDevParam->hDevSysHandle, 16) != MI_OK)
        {
            DBG_ERR("mi_sys_WaitOnInputTaskAvailable Fail!!!\n");
            continue;
        }
        down(&(pstDispDevParam->stDevMutex));
        if(!pstDispDevParam->bDISPEnabled)
        {
            up(&(pstDispDevParam->stDevMutex));
            continue;
        }

        if(!list_empty(pstDispDevParam->stBindedVideoLayer)
        {
            struct list_head *pstPos = NULL;
            struct mi_disp_LayerStatus_t *pstTmpLayer = NULL;
            MI_U8 u8LayerID = 0;            
            MI_BOOL bAvaliable = FALSE;
            list_for_each(pstPos, &(astDevStatus[u8DevId].stBindedLayer))
            {
                pstTmpLayer = list_entry(pstPos, struct mi_disp_LayerStatus_t, stLayerNode);
                u8LayerID = pstTmpLayer->u8LayerID;
                MI_U8 u8PortCount = 0;
                down(&(pstTmpLayer->stLayerMutex));
                for(u8PortCount = 0; u8PortCount < MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX; u8PortCount++)
                {                    
                    down(&(astLayerParams[u8LayerID].astPortStatus[u8PortCount].stPortMutex));
                    if(astLayerParams[u8LayerID].astPortStatus[u8PortCount].bEnable)
                    {
                        MI_SYS_ChnPort_t stChnInfo;
                        memset(&stChnInfo, 0, sizeof(MI_SYS_ChnPort_t));
                        stChnInfo.eModId = E_MI_SYS_MODULE_ID_DISP;
                        stChnInfo.s32ChnId = u8DevId;//Disp module Chn Equals Dev;
                        stChnInfo.s32DevId = u8DevId;
                        stChnInfo.s32PortId = u8PortCount;
                        MI_SYS_BufInfo_t *pstBufInfo = NULL;
                        
                        while((pstBufInfo = MI_SYS_GetInputPortBuf(&stChnInfo)) != NULL)
                        {
                            //Allocate pending buffer node
                            MI_DISP_PortPendingBuf_t* pstPortPendingBuf = NULL;
                            //增加出错处理， 使用kmalloc_cache
                            pstPortPendingBuf = kmem_cache_alloc(sizeof(MI_DISP_PortPendingBuf_t), GFP_KERNEL);
                            if(pstPortPendingBuf != NULL)
                            {
                                pstPortPendingBuf->pstInputBuffer.pstBufInfo;
                                INIT_LIST_HEAD(&(pstPortPendingBuf->stPortPendingBufNode));
                                down(&(astLayerParams[u8LayerID]->stDispLayerPendingQueueMutex));
                                list_add(&(astPortPendingBuf[u8LayerID][u8PortCount].stPortPendingBufNode), astLayerParams[u8LayerID]->stPortPendingBufQueue[u8PortCount]);
                                up(&(astLayerParams[u8LayerID]->stDispLayerPendingQueueMutex));
                                bAvaliable = TRUE;
                            }
                            else
                            {
                                DBG_ERR("kmem_cache_alloc Fail!!!\n");
                                mi_sys_RewindBuf(pstBufInfo);
                            }
                        }
                    }
                    up(&(astLayerParams[u8LayerID].astPortStatus[u8PortCount].stPortMutex));
                }//Go through input port in Video Layer
                up(&(pstTmpLayer->stLayerMutex));
            }//Go through All binded Video layer
        }// Check Binded list empty     
        if(bAvaliable)
        {
            wake_up(&pstDispDevParam->stWaitQueueHead);
        }
        up(&(pstDispDevParam->stDevMutex));
    }
} 

void MI_DISP_ISR(void *pdata)
{
    mi_disp_DevStatus_t *pstDispDevParam = NULL;
    if(pdata == NULL)
    {
        DBG_ERR("NULL Pointer, Disp_Dev_Work_Thread Return!!!\n");
        return;
    }
    pstDispDevParam = (mi_disp_DevStatus_t*)data;
    if(pstDispDevParam->bDISPEnabled)
    {
        wake_up(&pstDispDevParam->stWaitQueueHead);
        //记录中断的确切时间
        struct timespec sttime;
        memset(&sttime, 0, sizeof(sttime));
        do_posix_clock_monotonic_gettime(&sttime);
        if(pstDispDevParam->u64VsyncInterval == 0)
        {
            pstDispDevParam->u64CurrentIntTimeStamp = sttime.tv_sec * 1000 * 1000 + (sttime.tv_nsec / 1000);
            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
            pstDispDevParam->u64VsyncInterval = 20 * 1000;
        }
        else
        {
            pstDispDevParam->u64LastIntTimeStamp = pstDispDevParam->u64CurrentIntTimeStamp;
            pstDispDevParam->u64CurrentIntTimeStamp = sttime.tv_sec * 1000 * 1000 + (sttime.tv_nsec / 1000);
        }
    }
};

MI_s32 mi_disp_InitDevStatus(mi_disp_DevStatus_t* pstDevStatus, MI_U32 u32DevId)
{
    if(pstDevStatus == NULL)
    {
        DBG_ERR("mi_disp_InitDevStatus Fail !!!\n");
        return MI_FAIL; 
    }
    INIT_LIST_HEAD(pstDevStatus->stBindedLayer);
    pstDevStatus->bDISPEnabled = FALSE;
    DECLARE_WAIT_QUEUE_HEAD (pstDevStatus->stWaitQueueHead);
    pstDevStatus->u32DevId = u32DevId;
    pstDevStatus->u32SrcW = 0; //layer width
    pstDevStatus->u32SrcH = 0; //layer height
    pstDevStatus->u32BgColor = 0;
    pstDevStatus->u32Luma = 50;
    pstDevStatus->u32Contrast = 50;
    pstDevStatus->u32Hue = 50;
    pstDevStatus->u32Saturation = 50;
    pstDevStatus->u32Gain = 0;
    pstDevStatus->u32Sharpness = 0;
    pstDevStatus->bCvbsEnable = FALSE;
    pstDevStatus->pstDevObj = NULL;
    INIT_LIST_HEAD(pstDevStatus->stDevNode);
    pstDevStatus->pstWorkTask = NULL;
    pstDevStatus->pstIsrTask = NULL;
    init_MUTEX(&(pstDevStatus->stDevMutex));
    return MI_OK;
}

//Internal function 
static MI_BOOL _bDispInit = FALSE;

MI_S32 mi_disp_Init(void)
{
    //Register Dev 0
    MI_S32 s32Err = 0;
    MI_U32 u32DevID = 0;
    MI_U8 u8Count = 0;
    MI_SYS_ModuleBindOps_t stDISPOps;
    MI_SYS_ModuleInfo_t stModInfo;
    HalAllocPhyMem_t stHalAlloc;
    MI_SYS_DRV_HANDLE hDevSysHandle;
    if(_bDispInit)
    {
        DBG_INFO("MI_DISPLAY already Inited, return ok !\n");
        return MI_OK;
    }

    pstDevice0Param = &(astDevStatus[u32DevID]);
    if(pstDevice0Param == NULL)
    {
        DBG_ERR("Kmalloc Device0 memeory fail!!!\n");
        return MI_FAIL;
    }
    memset(pstDevice0Param, 0, sizeof(mi_disp_DevStatus_t));

    stDISPOps.OnBindChn = NULL; //DISPOnBindChnOutputCallback;
    stDISPOps.OnUnBindChn = NULL; //DISPOnUnBindChnOutputCallback;
    memset(&stModInfo, 0x0, sizeof(MI_SYS_ModuleInfo_t));
    stModInfo.u32ModuleId = E_MI_SYS_MODULE_ID_DISP;
    stModInfo.u32DevId = u32DevID;
    stModInfo.u32DevChnNum = 1;
    stModInfo.u32InputPortNum = MI_DISP_INPUTPORT_MAX;
    stModInfo.u32OutputPortNum = 0;

    for(u8Count = 0; u8Count < MI_DISP_INPUTPORT_MAX; u8Count++)
    {
       sprintf(stModInfo.au8OutputPortName[u8Count],  "DISP0_WINPORT%02d", u8Count);
    }
    hDevSysHandle = MI_SYS_RegisterDev(&stModInfo, &stDISPOps)
    pstDevice0Param->hDevSysHandle = hDevSysHandle;

    if(mi_disp_InitDevStatus(pstDevice0Param, u32DevID) != MI_OK)
    {
        DBG_ERR("mi_disp_InitDevStatus Fail!!!\n");
        return MI_FAIL;
    }
    
    stHalAlloc.free = MI_MMA_Free;
    stHalAlloc.alloc = MI_MMA_Alloc;
    if(!HalDispDeviceCreateInstance(&stHalAlloc, u32DevID, &(astDevStatus[u32DevID].pstDevObj)))
    {
        DBG_ERR("Dev%d Create Instance fail!!!\n", u32DevID);
        return MI_FAIL;
    }

    //Register Dev 1
    u32DevID = 1;
    stModInfo.u32DevId = u32DevID;
    pstDevice1Param = &(astDevStatus[u32DevID]);
    if(pstDevice1Param == NULL)
    {
        DBG_ERR("Kmalloc Device1 memeory fail!!!\n");
        return MI_FAIL;
    }

    for(u8Count = 0; u8Count < MI_DISP_INPUTPORT_MAX; u8Count++)
    {
       sprintf(stModInfo.au8OutputPortName[u8Count],  "DISP1_WINPORT%02d", u8Count);
    }
    hDevSysHandle = MI_SYS_RegisterDev(&stModInfo, &stDISPOps);
    pstDevice1Param->hDevSysHandle = hDevSysHandle;

    if(mi_disp_InitDevStatus(pstDevice1Param, u32DevID) != MI_OK)
    {
        DBG_ERR("mi_disp_InitDevStatus Fail!!!\n");
        return MI_FAIL;
    }
    
    if(!HalDispDeviceCreateInstance(&stHalAlloc, u32DevID, &(astDevStatus[u32DevID].pstDevObj)))
    {
        DBG_ERR("Dev%d Create Instance fail!!!\n", u32DevID);
        return MI_FAIL;
    }

    //Creater Work/ISR thread
    pstDevice0Param->pstWorkTask = kthread_create(Disp_Dev_Work_Thread, pstDevice0Param, "Dev0WorkThread");
    if(IS_ERR(pstDevice0Param->pstWorkTask ))
    {
        DBG_ERR("Unable to start kernel thread.\n");
        s32Err = PTR_ERR(pstDevice0Param->pstWorkTask );
        pstDevice0Param->pstWorkTask  = NULL;
        return s32Err;
    }
    wake_up_process(pstDevice0Param->pstWorkTask );

    pstDevice0Param->pstIsrTask = kthread_create(Disp_Dev_ISR_Thread, pstDevice0Param, "Dev1WorkThread");
    if(IS_ERR(pstDevice0Param->pstIsrTask))
    {
        DBG_ERR("Unable to start kernel thread.\n");
        s32Err = PTR_ERR(pstDevice0Param->pstIsrTask);
        pstDevice0Param->pstIsrTask = NULL;
        return s32Err;
    }
    wake_up_process(pstDevice0Param->pstIsrTask);

    pstDevice1Param->pstWorkTask = kthread_create(Disp_Dev_Work_Thread, pstDevice1Param);
    if(IS_ERR(pstDevice1Param->pstWorkTask))
    {
        DBG_ERR("Unable to start kernel thread.\n");
        s32Err = PTR_ERR(pstDevice1Param->pstWorkTask);
        pstDevice1Param->pstWorkTask = NULL;
        return s32Err;
    }
    wake_up_process(pstDevice1Param->pstWorkTask);

    pstDevice1Param->pstIsrTask = kthread_create(Disp_Dev_ISR_Thread, pstDevice1Param);
    if(IS_ERR(pstDevice1Param->pstIsrTask))
    {
        DBG_ERR("Unable to start kernel thread.\n");
        s32Err = PTR_ERR(pstDevice1Param->pstIsrTask);
        pstDevice1Param->pstIsrTask = NULL;
        return s32Err;
    }
    wake_up_process(pstDevice1Param->pstIsrTask);

    request_irq(DISP_DEV0_ISR_IDX, MI_DISP_ISR, pstDevice0Param);
    request_irq(DISP_DEV1_ISR_IDX, MI_DISP_ISR, pstDevice1Param);
    _bDispInit = TRUE;

    return MI_OK;
}

