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


#include <mi_vpe.h>
#include <HAL_VPE.h>

#define MI_VPE_MAX_INPUTPORT_NUM             (1)
#define VPE_PROC_WAIT                        (10)
#define VPE_WORK_THREAD_WAIT                 (2)
#define VPE_OUTPUT_PORT_FOR_MDWIN            (3)
#define MI_VPE_TASK_3DNR_UPDATE              (1ul << 0)
#define MI_VPE_TASK_ROI_UPDATE               (1ul << 1)
// TODO: Need Dummy register
#define MI_VPE_FENCE_REGISTER                (0x12345678)
#define MI_VPE_3DNR_MAX_WAITING_NUM          (32)
#define FHD_SIZE                             ((1920*1088))
#define MI_VPE_FRAME_PER_BURST_CMDQ          (32)

#define MI_VPE_CHECK_CHNN_SUPPORTED(VpeCh)   (((VpeCh) >= 0) && ((VpeCh) < (MI_VPE_MAX_CHANNEL_NUM)))
#define MI_VPE_CHECK_PORT_SUPPORTED(PortNum) (((PortNum) >= 0) && ((PortNum) < (MI_VPE_MAX_PORT_NUM)))
#define MI_VPE_CHECK_CHNN_VALID(VpeCh)       (MI_VPE_CHECK_CHNN_SUPPORTED((VpeCh)) && (_gVpeDevInfo.stChnnInfo[VpeCh].bCreated == TRUE))
#define GET_VPE_CHNN_PTR(VpeCh)              (&_gVpeDevInfo.stChnnInfo[(VpeCh)])
#define GET_VPE_PORT_PTR(VpeCh, VpePort)     (&_gVpeDevInfo.stChnnInfo[(VpeCh)].stOutPortInfo[(VpePort)])
#define GET_VPE_DEV_PTR()                    (&_gVpeDevInfo)
#define  MI_VPE_CHNN_DYNAMIC_ATTR_CHANGED(pstOldAttr, pstNewAttr) \
        (((pstOldAttr)->bContrastEn != (pstNewAttr)->bContrastEn)\
            || ((pstOldAttr)->bEdgeEn != (pstNewAttr)->bEdgeEn)\
            || ((pstOldAttr)->bEsEn != (pstNewAttr)->bEsEn)\
            || ((pstOldAttr)->bNrEn != (pstNewAttr)->bNrEn)\
            || ((pstOldAttr)->bUvInvert != (pstNewAttr)->bUvInvert)\
            || ((pstOldAttr)->ePixFmt != (pstNewAttr)->ePixFmt)\
        )
#define  MI_VPE_CHNN_STATIC_ATTR_CHANGED(pstOldAttr, pstNewAttr) \
        (((pstOldAttr)->u16MaxW != (pstNewAttr)->u16MaxW)\
            || ((pstOldAttr)->u16MaxH != (pstNewAttr)->u16MaxH)\
        )
#define MI_VPE_CHNN_PRAMS_CHANGED(pstOld, pstNew)\
        (((pstOld)->u8NrcSfStr != (pstNew)-> u8NrcSfStr) \
        || ((pstOld)->u8NrcTfStr != (pstNew)-> u8NrcTfStr) \
        || ((pstOld)->u8NrySfStr != (pstNew)-> u8NrySfStr) \
        || ((pstOld)->u8NryTfStr != (pstNew)-> u8NryTfStr) \
        || ((pstOld)->u8NryBlendMotionTh != (pstNew)-> u8NryBlendMotionTh) \
        || ((pstOld)->u8NryBlendStillTh != (pstNew)-> u8NryBlendStillTh) \
        || ((pstOld)->u8NryBlendMotionWei != (pstNew)-> u8NryBlendMotionWei) \
        || ((pstOld)->u8NryBlendOtherWei != (pstNew)-> u8NryBlendOtherWei) \
        || ((pstOld)->u8NryBlendStillWei != (pstNew)-> u8NryBlendStillWei) \
        || ((pstOld)->u8EdgeGain[0] != (pstNew)-> u8EdgeGain[0]) \
        || ((pstOld)->u8EdgeGain[1] != (pstNew)-> u8EdgeGain[1]) \
        || ((pstOld)->u8EdgeGain[2] != (pstNew)-> u8EdgeGain[2]) \
        || ((pstOld)->u8EdgeGain[3] != (pstNew)-> u8EdgeGain[3]) \
        || ((pstOld)->u8EdgeGain[4] != (pstNew)-> u8EdgeGain[4]) \
        || ((pstOld)->u8EdgeGain[5] != (pstNew)-> u8EdgeGain[5]) \
        || ((pstOld)->u8EdgeGain[6] != (pstNew)-> u8EdgeGain[6]) \
        || ((pstOld)->u8Contrast != (pstNew)-> u8Contrast)\
        )
#define GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask) (&(pstChnTask)->astInputPortBufInfo[0].stFrameData)

typedef struct MI_VPE_OutPortInfo_s {
    MI_BOOL           bEnable;
    MI_U32            u32OverlayMask;
    MI_VPE_PortMode_t stPortMode;
    MI_SYS_ChnPort_t  stPeerOutputPortInfo; // Port binder to: Just for debug
} mi_vpe_OutPortInfo_t;

typedef enum
{
    E_MI_VPE_CHANNEL_STATUS_INITED = 0,
    E_MI_VPE_CHANNEL_STATUS_START,
    E_MI_VPE_CHANNEL_STATUS_STOP,
    E_MI_VPE_CHANNEL_STATUS_NUM,
} mi_vpe_ChannelStatus_e;


typedef enum
{
    E_MI_VPE_3DNR_STATUS_INVALID = 0,
    E_MI_VPE_3DNR_STATUS_IDLE,
    E_MI_VPE_3DNR_STATUS_NEED_UPDATE,
    E_MI_VPE_3DNR_STATUS_RUNNING,
    E_MI_VPE_3DNR_STATUS_UPDATED,
    E_MI_VPE_CHANNEL_STATUS_NUM,
} mi_vpe_3DNrStatus_e;


typedef struct {
    mi_vpe_3DNrStatus_e eStatus;
    MI_VPE_CHANNEL VpeCh;
    MI_U32  u32WaitScriptNum;
} mi_vpe_3DNRUpdateStatus_t;

typedef struct {
    MI_U16 u16Width;
    MI_U16 u16Height;
} mi_vpe_SourceVideoInfo_t;

typedef struct MI_VPE_ChanneInfo_s {
    struct list_head         list;                               // Device active list
    MI_BOOL                  bCreated;                           // Channel: Created
    void                     *pIspCtx;                           // HAL layer: ISP context pointer
    void                     *pIqCtx;                            // HAL layer: IQ context pointer
    void                     *pSclCtx;                           // HAL layer: SCL context pointer
    MI_VPE_ChannelAttr_t     stChnnAttr;                         // Channel attribution
    MI_VPE_ChannelPara_t     stChnnPara;                         // Channel parameter
    mi_vpe_ChannelStatus_e   eStatus;                            // Channel status
    MI_SYS_CompressMode_e    eCompressMode;                      // Channel Input Compress mode
    mi_vpe_SourceVideoInfo_t stSrcWin;                           // Chanel input source size
    MI_SYS_WindowRect_t      stCropWin;                          // Channel user setting crop window
    MI_SYS_WindowRect_t      stRealCrop;                         // Channel real crop window
    MI_SYS_Rotate_e          eRotationType;                      // Channel rotation type
    MI_SYS_ChnPort_t         stPeerInputPortInfo;                // Channel binder to: Just for debug
    mi_vpe_OutPortInfo_t     stOutPortInfo[MI_VPE_MAX_PORT_NUM]; // Information of output
    MI_PHY                   u64PhyAddrOffset[3];                // Channel address offset for sw crop window.
    MI_VPE_CHANNEL           VpeCh;
} mi_vpe_ChannelInfo_t;

typedef enum {
    E_MI_VPE_ROI_STATUS_INVALID = 0,
    E_MI_VPE_ROI_STATUS_IDLE,        //---> free
    E_MI_VPE_ROI_STATUS_NEED_UPDATE, // user call ROI
    E_MI_VPE_ROI_STATUS_RUNNING,     // ROI in cmdQ
    E_MI_VPE_ROI_STATUS_UPDATED,     // ISR ROI finish
    E_MI_VPE_ROI_STATUS_NUM,
} mi_vpe_RoiStatus_e;

typedef struct {
    mi_vpe_RoiStatus_e  eRoiStatus;
    MI_VPE_CHANNEL      VpeCh;
    MI_VPE_RegionInfo_t stRegion; // user region
} mi_vpe_RoiInfo_t;

typedef struct {
    mi_vpe_ChannelInfo_t      stChnnInfo[MI_VPE_MAX_CHANNEL_NUM];// All channel information
    MI_SYS_DRV_HANDLE         hDevSysHandle;                     // Handle for MI_SYS device
    MI_BOOL                   bInited;                           // Weather MI_VPE already inited.
    cmd_mload_interface       *pstCmdMloadInfo;                  // Command Queue/Menuload interface
    unsigned int              uVpeIrqNum;                        // VPE irq number
    struct task_struct        *pstWorkThread;                    // Kernel thread for VPEWorkThread
    struct task_struct        *pstProcThread;                    // Kernel thread for VPE IRQ bottom
    MI_U32                    u32ChannelCreatedNum;              // Total channel number for user created
//    mi_vpe_RoiInfo_t          stRoiInfo;          // Roi information
    mi_vpe_RoiStatus_e        eRoiStatus;
    mi_vpe_3DNRUpdateStatus_t st3DNRUpdate;                      // 3DNR update status
    MI_U64                    u64TotalCreatedArea;               // Created channel total area
} mi_vpe_DevInfo_t;

typedef struct
{
   int totalAddedTask;
} mi_vpe_IteratorWorkInfo_t;

static mi_vpe_DevInfo_t _gVpeDevInfo = {
    .bInited = FALSE,
    .hDevSysHandle = MI_HANDLE_NULL,
    .uVpeIrqNum    = 0,
    .pstWorkThrad  = NULL,
    .pstProcThread = NULL,
    .u32ChannelCreatedNum = 0,
    .eRoiStatus    = E_MI_VPE_ROI_STATUS_IDLE,
    .st3DNRUpdate  = {
        .eStatus = E_MI_VPE_3DNR_STATUS_IDLE,
        .VpeCh    = 0,
    },
};

#define MI_VPE_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstVpeChAttr) (!(!(((pstVpeChAttr)->u32MaxW > MI_VPE_CHANNEL_MAX_WIDTH) && ((pstVpeChAttr)->u32MaxW * (pstVpeChAttr)->u32MaxH) > (MI_VPE_CHANNEL_MAX_WIDTH *MI_VPE_CHANNEL_MAX_HEIGHT))\
            && (((pstDevInfo)->u64TotalCreatedArea + ((pstVpeChAttr)->u32MaxW * (pstVpeChAttr)->u32MaxH) <= 8*FHD_SIZE))\
            && (((pstDevInfo)->u32ChannelCreatedNum +  1) < MI_VPE_MAX_CHANNEL_NUM)\
            ))

DECLARE_WAIT_QUEUE_HEAD(vpe_isr_waitqueue);
LIST_HEAD(VPE_roi_task_list);
DECLARE_MUTEX(VPE_roi_task_list_sem);

LIST_HEAD(VPE_todo_task_list);
LIST_HEAD(VPE_working_task_list);
DECLARE_MUTEX(VPE_working_list_sem);
LIST_HEAD(VPE_active_channel_list);
DECLARE_MUTEX(VPE_active_channel_list_sem);
MI_S32 MI_VPE_ATOM_CreateChannel(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_SUPPORTED(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

        // Check created ?
        if (pstChnnInfo->bCreated == TRUE)
        {
            s32Ret = MI_ERR_VPE_EXIST;
            DBG_EXIT_ERR("Channel id: %d already created.\n", VpeCh);
            return s32Ret;
        }
        else if (MI_VPE_CHNN_ATTR_OUT_OF_CAPS(pstDevInfo, pstVpeChAttr))
        {
            DBG_WRN("Channel id: %d MaxW: %d MaxH: %d out of hardware Caps !!!\n", VpeCh, pstVpeChAttr->u16MaxW, pstVpeChAttr->u16MaxH);
        }
        // Create channel
        HalAllocPhyMem_t stAlloc;
        // SCL handle
        HalVpeSclWinSize_t stMaxWin;
        stAlloc.alloc = mi_sys_MMA_Alloc;
        stAlloc.free  = mi_sys_MMA_Free;
        stMaxWin.u16Width = pstChnnInfo->stChnnAttr.u32MaxH = pstVpeChAttr->u32MaxW;
        stMaxWin.u16Height= pstChnnInfo->stChnnAttr.u32MaxW = pstVpeChAttr->u32MaxH;
        HalVpeCreateSclInstance(&stAlloc, &stMaxWin,  &pstChnnInfo->pSclCtx);
        // IQ handle
        HalVpeCreateIqInstance(&stAlloc, &pstChnnInfo->pIqCtx);
        // ISP handle
        HalVpeCreateIspInstance(&stAlloc, &pstChnnInfo->pIspCtx);

        // HalVpeIspInputConfig
        HalVpeIqOnOff_t stCfg;
        memset(&stCfg, 0, sizeof(stCfg));
        stCfg.bNREn       = pstChnnInfo->stChnnAttr.bNrEn       = pstVpeChAttr->bNrEn;
        stCfg.bEdgeEn     = pstChnnInfo->stChnnAttr.bEdgeEn     = pstVpeChAttr->bEdgeEn;
        stCfg.bESEn       = pstChnnInfo->stChnnAttr.bEsEn       = pstVpeChAttr->bEsEn;
        stCfg.bContrastEn = pstChnnInfo->stChnnAttr.bContrastEn = pstVpeChAttr->bContrastEn;
        stCfg.bUVInvert   = pstChnnInfo->stChnnAttr.bUvInvert   = pstVpeChAttr->bUvInvert;
        HalVpeIqOnOff(pstChnnInfo->pIspCtx, &stCfg);

        HalVpeIspInputConfig_t stIspCfg;
        memset(&stIspCfg, 0, sizeof(stIspCfg));
        // Init input compress mode
        stIspCfg.eCompressMode = pstChnnInfo->eCompressMode = E_HAL_COMPRESS_MODE_NONE;

        // Init input width/height
        // Cap Window
        pstChnnInfo->u64PhyAddrOffset[0] = 0;
        pstChnnInfo->u64PhyAddrOffset[1] = 0;
        pstChnnInfo->u64PhyAddrOffset[2] = 0;
        stIspCfg.u32Width       = pstChnnInfo->stSrcWin.u16Width = pstVpeChAttr->u32MaxW;
        stIspCfg.u32Height      = pstChnnInfo->stSrcWin.u16Height= pstVpeChAttr->u32MaxH;
        // init input pixel format
        stIspCfg.ePixelFormat   = pstChnnInfo->stChnnAttr.ePixFmt = pstVpeChAttr->ePixFmt;
        HalVpeIspInputConfig(pstChnnInfo->pIspCtx, &stIspCfg);

        // Init internal parameter
        pstChnnInfo->eRotationType = E_MI_SYS_ROTATE_NONE;
        memset(&pstChnnInfo->stPeerInputPortInfo, 0, sizeof(pstChnnInfo->stPeerInputPortInfo));
        memset(&pstChnnInfo->u32LumaData, 0, sizeof(pstChnnInfo->u32LumaData));
        memset(&pstChnnInfo->stOutPortInfo, 0, sizeof(pstChnnInfo->stOutPortInfo));
        pstDevInfo->u32ChannelCreatedNum++;
        pstChnnInfo->bCreated = TRUE;
        pstChnnInfo->VpeCh    = VpeCh;
        pstDevInfo->u64TotalCreatedArea += pstChnnInfo->u32MaxW * pstChnnInfo->u32MaxH;
        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_DestroyChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

        HalVpeDestroyIspInstance(pstChnnInfo->pIspCtx);
        HalVpeDestroyIqInstance(pstChnnInfo->pIqCtx);
        HalVpeDestroySclInstance(pstChnnInfo->pSclCtx);
        pstDevInfo->u32ChannelCreatedNum--;
        pstDevInfo->u64TotalCreatedArea -= pstChnnInfo->u32MaxW * pstChnnInfo->u32MaxH;
        memset(pstChnnInfo, 0, sizeof(*pstChnnInfo));

        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        memcpy(pstVpeChAttr, _gVpeDevInfo.stChnnInfo[VpeCh].stChnnAttr, sizeof(*pstVpeChAttr));
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetChannelAttr(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelAttr_t *pstVpeChAttr)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        if (MI_VPE_CHNN_STATIC_ATTR_CHANGED(&pstChnnInfo->stChnnAttr, pstVpeChAttr))
        {
            s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
            DBG_EXIT_ERR("Channel id: %d illegal params. Try to change static params(MaxW/MaxH).\n", VpeCh);
        }
        else if (MI_VPE_CHNN_DYNAMIC_ATTR_CHANGED(&pstChnnInfo->stChnnAttr, pstVpeChAttr))
        {
            HalVpeIqOnOff_t stCfg;
            memset(&stCfg, 0, sizeof(stCfg));
            stCfg.bContrastEn = pstVpeChAttr->bContrastEn;
            stCfg.bEdgeEn     = pstVpeChAttr->bEdgeEn;
            stCfg.bESEn       = pstVpeChAttr->bEsEn;
            stCfg.bNREn       = pstVpeChAttr->bNrEn;
            stCfg.bUVInvert   = pstVpeChAttr->bUvInvert;
            if (TRUE == HalVpeIqOnOff(pstChnnInfo->pIqCtx, &stCfg))
            {
                pstChnnInfo->stChnnAttr.bContrastEn = pstVpeChAttr->bContrastEn;
                pstChnnInfo->stChnnAttr.bEdgeEn     = pstVpeChAttr->bEdgeEn;
                pstChnnInfo->stChnnAttr.bEsEn       = pstVpeChAttr->bEsEn;
                pstChnnInfo->stChnnAttr.bNrEn       = pstVpeChAttr->bNrEn;
                pstChnnInfo->stChnnAttr.bUvInvert   = pstVpeChAttr->bUvInvert;

                HalVpeIspInputConfig_t stIspCfg;
                memset(&stIspCfg, 0, sizeof(stIspCfg));
                stIspCfg.eCompressMode = pstChnnInfo->eCompressMode;
                stIspCfg.u32Width      = pstChnnInfo->stCapWin.u16Width;
                stIspCfg.u32Height     = pstChnnInfo->stCapWin.u16Height;
                stIspCfg.ePixelFormat  = pstChnnInfo->stChnnAttr.ePixFmt = pstVpeChAttr->ePixFmt;
                HalVpeIspInputConfig(pstChnnInfo->pIspCtx, &stIspCfg);

                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                DBG_EXIT_ERR("Channel id: %d illegal params.\n", VpeCh);
            }
        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}


MI_S32 MI_VPE_ATOM_StartChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        pstChnnInfo->eStatus = E_MI_VPE_CHANNEL_STATUS_START;
        // Add channel to active list
        down(&VPE_active_channel_list_sem);
        list_add_tail(&pstChnnInfo->list, &VPE_active_channel_list);
        up(&VPE_active_channel_list_sem);

        // TODO: Notify to MI_SYS
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    DBG_EXIT_OK();

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_StopChannel(MI_VPE_CHANNEL VpeCh)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        pstChnnInfo->eStatus = E_MI_VPE_CHANNEL_STATUS_STOP;

        // Remove channel from active list
        down(&VPE_active_channel_list_sem);
        list_del(&pstChnnInfo->list);
        up(&VPE_active_channel_list_sem);

        // TODO: Notify to MI_SYS
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        if (MI_VPE_CHNN_PRAMS_CHANGED(&pstChnnInfo->stChnnPara, pstVpeParam))
        {
            HalVpeIqConfig_t stCfg;
            stCfg.u8Contrast = pstVpeParam->u8Contrast;
            stCfg.u8EdgeGain[0] = pstVpeParam->u8EdgeGain[0];
            stCfg.u8EdgeGain[1] = pstVpeParam->u8EdgeGain[1];
            stCfg.u8EdgeGain[2] = pstVpeParam->u8EdgeGain[2];
            stCfg.u8EdgeGain[3] = pstVpeParam->u8EdgeGain[3];
            stCfg.u8EdgeGain[4] = pstVpeParam->u8EdgeGain[4];
            stCfg.u8EdgeGain[5] = pstVpeParam->u8EdgeGain[5];
            stCfg.u8NRC_SF_STR  = pstVpeParam->u8NrcSfStr;
            stCfg.u8NRC_TF_STR  = pstVpeParam->u8NrcTfStr;
            stCfg.u8NRY_BLEND_MOTION_TH = pstVpeParam->u8NryBlendMotionTh;
            stCfg.u8NRY_BLEND_MOTION_WEI= pstVpeParam->u8NryBlendMotionWei;
            stCfg.u8NRY_BLEND_OTHER_WEI = pstVpeParam->u8NryBlendOtherWei;
            stCfg.u8NRY_BLEND_STILL_TH  = pstVpeParam->u8NryBlendStillTh;
            stCfg.u8NRY_BLEND_STILL_WEI = pstVpeParam->u8NryBlendStillWei;
            stCfg.u8NRY_SF_STR          = pstVpeParam->u8NrySfStr;
            stCfg.u8NRY_TF_STR          = pstVpeParam->u8NryTfStr;
            if (TRUE == HalVpeIqConfig(pstChnnInfo->pIqCtx, &stCfg))
            {
                pstChnnInfo->stChnnPara.u8Contrast = pstVpeParam->u8Contrast;
                pstChnnInfo->stChnnPara.u8EdgeGain[0] = pstVpeParam->u8EdgeGain[0];
                pstChnnInfo->stChnnPara.u8EdgeGain[1] = pstVpeParam->u8EdgeGain[1];
                pstChnnInfo->stChnnPara.u8EdgeGain[2] = pstVpeParam->u8EdgeGain[2];
                pstChnnInfo->stChnnPara.u8EdgeGain[3] = pstVpeParam->u8EdgeGain[3];
                pstChnnInfo->stChnnPara.u8EdgeGain[4] = pstVpeParam->u8EdgeGain[4];
                pstChnnInfo->stChnnPara.u8EdgeGain[5] = pstVpeParam->u8EdgeGain[5];
                pstChnnInfo->stChnnPara.u8NrcSfStr = pstVpeParam->u8NrcSfStr;
                pstChnnInfo->stChnnPara.u8NrcTfStr = pstVpeParam->u8NrcTfStr;
                pstChnnInfo->stChnnPara.u8NryBlendMotionTh = pstVpeParam->u8NryBlendMotionTh;
                pstChnnInfo->stChnnPara.u8NryBlendMotionWei = pstVpeParam->u8NryBlendMotionWei;
                pstChnnInfo->stChnnPara.u8NryBlendOtherWei = pstVpeParam->u8NryBlendOtherWei;
                pstChnnInfo->stChnnPara.u8NryBlendStillTh = pstVpeParam->u8NryBlendStillTh;
                pstChnnInfo->stChnnPara.u8NryBlendStillWei = pstVpeParam->u8NryBlendStillWei;
                pstChnnInfo->stChnnPara.u8NrySfStr = pstVpeParam->u8NrySfStr;
                pstChnnInfo->stChnnPara.u8NryTfStr = pstVpeParam->u8NryTfStr;
                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                DBG_EXIT_ERR("Channel id: %d illegal params.\n", VpeCh);
            }

        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        memcpy(pstVpeParam, &pstChnnInfo->stChnnPara, sizeof(*pstVpeParam));
        s32Ret = MI_VPE_OK;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);

        if ((pstCropInfo->u16X != pstChnnInfo->stCropWin.u16X)
            || (pstCropInfo->u16Y != pstChnnInfo->stCropWin.u16Y)
            || (pstCropInfo->u16Width !=  pstChnnInfo->stCropWin.u16Width)
            || (pstCropInfo->u16Height!=  pstChnnInfo->stCropWin.u16Height)
            )
        {
#if 0
            HalVpeSclCropConfig_t stCropWin;
            memset(&stCropWin, 0, sizeof(stCropWin));
            stCropWin.bCropEn = TRUE;
            stCropWin.stCropWin.u16X = pstCropInfo.u16X;
            stCropWin.stCropWin.u16Y = pstCropInfo.u16Y;
            stCropWin.stCropWin.u16Width  = pstCropInfo.u16Width;
            stCropWin.stCropWin.u16Height = pstCropInfo.u16Height;

            if (TRUE == HalVpeSclCropConfig(pstChnnInfo->pSclCtx, &stCropWin))
            {
                pstChnnInfo->stCropWin.u16X = pstCropInfo.u16X;
                pstChnnInfo->stCropWin.u16Y = pstCropInfo.u16Y;
                pstChnnInfo->stCropWin.u16Width  = pstCropInfo.u16Width;
                pstChnnInfo->stCropWin.u16Height = pstCropInfo.u16Height;

                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                DBG_EXIT_ERR("Channel id: %d illegal params.\n", VpeCh);
            }
#else
            pstChnnInfo->stCropWin.u16X = pstCropInfo.u16X;
            pstChnnInfo->stCropWin.u16Y = pstCropInfo.u16Y;
            pstChnnInfo->stCropWin.u16Width  = pstCropInfo.u16Width;
            pstChnnInfo->stCropWin.u16Height = pstCropInfo.u16Height;

#endif
        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetChannelCrop(MI_VPE_CHANNEL VpeCh,  MI_SYS_WindowRect_t *pstCropInfo)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        memcpy(pstCropInfo, &pstChnnInfo->stCropWin, sizeof(*pstCropInfo));
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}


typedef struct ROI_Task_s
{
   struct list_head      list;
    MI_VPE_RegionInfo_t  *pstRegion; // IN
    wait_queuque queue;
    mi_vpe_RoiStatus_e   eRoiStatus;
    MI_U32               u32LumaData[ROI_WINDOW_MAX]; // Channel luma region data
    mi_vpe_ChannelInfo_t *pstChnnInfo;
} ROI_Task_t;

MI_S32 MI_VPE_ATOM_GetChannelRegionLuma(MI_VPE_CHANNEL VpeCh, MI_VPE_RegionInfo_t *pstRegionInfo, MI_U32 *pu32LumaData,MI_S32 s32MilliSec)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;

    ROI_Task_t stRoiTask;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_DevInfo_t *pstDevInfo      = GET_VPE_DEV_PTR();
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        MI_U32 u32RegionNum = (pstRegionInfo->u32RegionNum > ROI_WINDOW_MAX) ? ROI_WINDOW_MAX : pstRegionInfo->u32RegionNum;
        memset(&stRoiTask, 0, sizeof(stRoiTask));
        stRoiTask.pstRegion   = pstRegionInfo;
        stRoiTask.eRoiStatus  = E_MI_VPE_ROI_STATUS_NEED_UPDATE;
        stRoiTask.pstChnnInfo = pstChnnInfo;
        // Tommy: support multi-channel ROI queue ??
        down(&VPE_roi_task_list_sem);
        list_add_tail(&stRoiTask, VPE_roi_task_list);
        up(&VPE_roi_task_list_sem);

waiting_roi_running:
        interruptible_sleep_on_timeout(&stRoiTask->queue, msecs_to_jiffies(s32MilliSec));
        if (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_RUNNING)
        {
            goto waiting_roi_running;
        }
        else
        {
            down(&VPE_roi_task_list_sem);
            list_del(&stRoiTask, VPE_roi_task_list);
            up(&VPE_roi_task_list_sem);

            if (stRoiTask.eRoiStatus == E_MI_VPE_ROI_STATUS_UPDATED)
            {
                // Copy ROI to user
                memcpy(pu32LumaData, &stRoiTask.u32LumaData, u32RegionNum*sizeof(MI_U32));
                pstDevInfo->stRoiInfo.eRoiStatus = E_MI_VPE_ROI_STATUS_IDLE;
                s32Ret = MI_VPE_OK;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_BUSY;
                DBG_EXIT_ERR("Channel %s is busy.\n", VpeCh);
            }
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_VPE_RotationType_e eType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        if (eType != pstChnnInfo->eRotationType)
        {
            HalVpeIspRotationConfig_t stRotation;
            stRotation.enRotType = (HalVpeIspRotationType_e)eType;
            if (TRUE == HalVpeIspRotationConfig(pstChnnInfo->pIspCtx, &stRotation))
            {
                s32Ret = MI_VPE_OK;
                pstChnnInfo->eRotationType = eType;
                DBG_EXIT_OK();
            }
            else
            {
                s32Ret = MI_ERR_VPE_NOT_SUPPORT;
                DBG_EXIT_ERR("Channel %d not support Roation: %d.\n", VpeCh, eType);
            }
        }
        else
        {
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetChannelRotation(MI_VPE_CHANNEL VpeCh,  MI_VPE_RotationType_e *pType)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(VpeCh);
        pType = pstChnnInfo->eRotationType;
        DBG_EXIT_OK();
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_EnablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            pstOutPortInfo->bEnable = TRUE;
            // TODO: Notify MI_SYS
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_DisablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            // TODO: Notify MI_SYS
            pstOutPortInfo->bEnable = FALSE;
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetPortOverlay(MI_VPE_CHANNEL VpeCh,MI_VPE_PORT VpePort, MI_U32 u32OverlayMask)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            if (pstOutPortInfo->u32OverlayMask != u32OverlayMask)
            {
                pstOutPortInfo->u32OverlayMask = u32OverlayMask;
            }
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetPortOverlay(MI_VPE_CHANNEL VpeCh,MI_VPE_PORT VpePort, MI_U32 *pu32OverlayMask)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            *pu32OverlayMask = pstOutPortInfo->u32OverlayMask;
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_SetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_ChannelInfo_t  *pstChnnInfo    = GET_VPE_CHNN_PTR(VpeCh);
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            s32Ret = MI_VPE_OK;
            if ((pstVpeMode->u16Width != pstOutPortInfo->stPortMode.u16Width)
                || (pstVpeMode->u16Height!= pstOutPortInfo->stPortMode.u16Height))
            {
                HalVpeSclOutputSizeConfig_t stOutput;
                // Need check as foo ?
                // memset(&stOutput, 0, sizeof(stOutput));
                stOutput.enOutPort = (HalVpeSclOutputPort_e)VpePort;
                stOutput.u16Width  = pstVpeMode->u16Width;
                stOutput.u16Height = pstVpeMode->u16Height;
                if (TRUE == HalVpeSclOutputSizeConfig(pstChnnInfo->pSclCtx, &stOutput))
                {
                    pstOutPortInfo->stPortMode.u16Width = pstVpeMode->u16Width;
                    pstOutPortInfo->stPortMode.u16Height= pstVpeMode->u16Height;

                    DBG_EXIT_OK();
                }
                else
                {
                    s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                    DBG_EXIT_ERR("Ch: %d port %d set output size fail.\n", VpeCh, VpePort);
                }
            }

            // Tommy: Check Caps: Pixel format per frame change in ISP ????
            if (s32Ret == MI_VPE_OK)
            {
                if ((pstOutPortInfo->stPortMode.ePixelFormat != pstVpeMode->ePixelFormat)
                    ||(pstOutPortInfo->stPortMode.eCompressMode != pstVpeMode->eCompressMode))
                {
                    HalVpeSclOutputDmaConfig_t stDmaCfg;
                    // Need check as foo?
                    // memset(&stDmaCfg, 0, sizeof(stDmaCfg));
                    stDmaCfg.enOutPort = (HalVpeSclOutputPort_e)VpePort;
                    if (VpePort == VPE_OUTPUT_PORT_FOR_MDWIN)
                    {
                        stDmaCfg.enOutFormat = E_HAL_PIXEL_FRAME_YUV_MST_420;
                        stDmaCfg.enCompress  = E_HAL_COMPRESS_MODE_NONE;
                    }
                    else
                    {
                        stDmaCfg.enOutFormat = pstVpeMode.ePixelFormat;
                        stDmaCfg.enCompress  = pstVpeMode.eCompressMode;
                    }

                    if (TRUE == HalVpeSclOutputDmaConfig(pstChnnInfo->pSclCtx, &stDmaCfg))
                    {
                        pstOutPortInfo->stPortMode.ePixelFormat = pstVpeMode->ePixelFormat;
                        pstOutPortInfo->stPortMode.eCompressMode= pstVpeMode->eCompressMode;
                        DBG_EXIT_OK();
                    }
                    else
                    {
                        s32Ret = MI_ERR_VPE_ILLEGAL_PARAM;
                        DBG_EXIT_ERR("Ch: %d port %d set output size fail.\n", VpeCh, VpePort);
                    }
                }

                pstOutPortInfo->stPortMode = *pstVpeMode;
            }
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

MI_S32 MI_VPE_ATOM_GetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode)
{
    MI_S32 s32Ret = MI_ERR_VPE_BUSY;
    DBG_ENTER();

    if (MI_VPE_CHECK_CHNN_VALID(VpeCh))
    {
        if (MI_VPE_CHECK_PORT_SUPPORTED(VpePort))
        {
            mi_vpe_OutPortInfo_t *pstOutPortInfo = GET_VPE_PORT_PTR(VpeCh, VpePort);
            *pstVpeMode = pstOutPortInfo->stPortMode;
            s32Ret = MI_VPE_OK;
            DBG_EXIT_OK();
        }
        else
        {
            s32Ret = MI_ERR_VPE_INVALID_PORTID;
            DBG_EXIT_ERR("Invalid port id: %d.\n", VpePort);
        }
    }
    else
    {
        s32Ret = MI_ERR_VPE_INVALID_CHNID;
        DBG_EXIT_ERR("Invalid channel id: %d.\n", VpeCh);
    }

    return s32Ret;
}

static inline MI_BOOL _MI_VPE_IsFenceLe(MI_U16 u16Fence1, MI_U16 u16Fence2)
{
   if(u16Fence1<=u16Fence2)
        return TRUE;
   if(u16Fence2+(0xFFFF-u16Fence1) <0x7FFFF)
        return TRUE;

   return FALSE;
}

static MI_U16 _MI_VPE_ReadFence(void)
{
    MI_U16 u16Value = 0;
    // Tommy: MI_SYS open internal read register ???
    MI_SYS_GetReg(MI_VPE_FENCE_REGISTER, &u16Value);
    return u16Value;
}

static void VPE_ISR_Proc_Thread(void *data)
{
    mi_vpe_DevInfo_t *pstDevInfo  = (mi_vpe_DevInfo_t *)data;
    cmd_mload_interface *cmdinf = pstDevInfo->pstCmdMloadInfo;

    while(1)
    {
        interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(VPE_PROC_WAIT));
        while(!list_empty(&VPE_working_task_list));
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            pstChnTask = container_of(&VPE_working_task_list.next, mi_sys_ChnTaskInfo_t, cur_list);
            // Task has not been finished yet.
            if(_MI_VPE_IsFenceLe(_MI_VPE_ReadFence(), pstChnTask->u32Reserved0))
            {
                if(!cmdinf->is_cmdq_empty_idle(cmdinf->ctx)) //cmdQ is Running
                {
                    break;
                }
                else // cmdQ is stop
                {
                    DBG_ERR("invalid fence %04x, %04x!\n", _MI_VPE_ReadFence(), pstChnTask->u32Reserved0);
                    BUG();
                }
            }

            // Task already finished.
            down(&VPE_working_list_sem);
            // Remove task from working list
            list_del(&pstChnTask->list);
            up(&VPE_working_list_sem);
            // Release menuload ring buffer
            cmdinf->update_mload_ringbuf_read_ptr(cmdinf, pstChnTask->u64Reserved0);

            // Check Task is 3DNR upate
            if (pstChnTask->u32Reserved1 & MI_VPE_TASK_3DNR_UPDATE)
            {
                _MI_VPE_3DnrProcessFinish(pstDevInfo, &pstDevInfo->stChnnInfo[pstChnTask->u32ChnId]);
            }

            // Check Task is ROI update
            if (pstChnTask->u32Reserved1 & MI_VPE_TASK_ROI_UPDATE)
            {
                _MI_VPE_RoiProcessFinish(pstDevInfo, &pstDevInfo->stChnnInfo[pstChnTask->u32ChnId], pstChnTask);
            }

            // Finish task to mi_sys
            // Tommy: add description for mi_sys_FinishTaskBuf  life cycle for: pstChnTask
            mi_sys_FinishTaskBuf(pstChnTask);
        }
    }

}

irqreturn_t VPE_ISR(int irq, void *data)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)data;

    if (HalVpeSclCheckIrq() == TRUE)
    {
        HalVpeSclClearIrq();
        wake_up(&vpe_isr_waitqueue);
    }

    return IRQ_HANDLED;
}

static mi_sys_TaskIteratorCBAction_e _MI_VPE_TaskIteratorCallBK(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData)
{
    int i;
    int valid_output_port_cnt = 0;
    mi_vpe_IteratorWorkInfo_t *workInfo = (mi_vpe_IteratorWorkInfo_t *)pUsrData;
    mi_vpe_ChannelInfo_t *pstChnnInfo = GET_VPE_CHNN_PTR(pstTaskInfo->u32ChnId);

    // Check Channel stop or created ??
    if ((pstChnnInfo->bCreated == FALSE) ||
        (pstChnnInfo->eStatus != E_MI_VPE_CHANNEL_STATUS_START)
        )
    {
        // Drop can not process input buffer
        mi_sys_FinishTaskBuf(pstTaskInfo);
        DBG_EXIT_ERR("Ch %d is not create or Ch %d is stop. Drop frame directly.\n",
                        pstTaskInfo->u32ChnId, pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    if (pstTaskInfo->astInputPortBufInfo[0].eBufType != E_MI_SYS_BUFDATA_FRAME)
    {
        // Drop can not process input buffer
        mi_sys_FinishTaskBuf(pstTaskInfo);
        DBG_EXIT_ERR("Ch %d is not support buffer pixel format.\n",
                        pstTaskInfo->u32ChnId);
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
    }

    for (i = 0; i < MI_VPE_MAX_PORT_NUM; i++)
    {
        pstTaskInfo->astOutputPortPerfBufConfig[i].u64TargetPts = pstTaskInfo->astInputPortBufInfo[0].u64Pts;
        pstTaskInfo->astOutputPortPerfBufConfig[i].eBufType     = E_MI_SYS_BUFDATA_FRAME;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.eFormat = pstChnnInfo->stOutPortInfo[i].stPortMode.ePixelFormat;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Width = pstChnnInfo->stOutPortInfo[i].stPortMode.u16Width;
        pstTaskInfo->astOutputPortPerfBufConfig[i].stFrameCfg.u16Height= pstChnnInfo->stOutPortInfo[i].stPortMode.u16Height;
    }

    if(mi_sys_PrepareTaskOutputBuf(pstTaskInfo) != MI_OK)
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;

    for(i = 0; i < pstTaskInfo->u32OutputPortNum; i++)
    {
        BUG_ON(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i] && pstTaskInfo->astOutputPortBufInfo[i]);

        if(pstTaskInfo->bOutputPortMaskedByFrmrateCtrl[i])
            valid_output_port_cnt++;
        else if(pstTaskInfo->astOutputPortBufInfo[i])
            valid_output_port_cnt++;
    }

    //check if lack of output buf
    if(pstTaskInfo->u32OutputPortNum && valid_output_port_cnt==0)
    {
        return MI_SYS_ITERATOR_SKIP_CONTINUTE;
    }

    list_add_tail(&pstTaskInfo->list, &VPE_todo_task_list);

    //we at most process 32 batches at one time
    if(++workInfo->totalAddedTask >= MI_VPE_FRAME_PER_BURST_CMDQ)
        return MI_SYS_ITERATOR_ACCEPT_STOP;
    else
        return MI_SYS_ITERATOR_ACCEPT_CONTINUTE;
}


// TODO: implement by region ownner
extern MI_S32 VPE_osd_process(MI_REG_CmdInfo_t CmdInfo, cmd_mload_interface *cmdinf);

//-------------------------------------------------
// ROI Defination:
//   Crop Window
//    *----------------------------------------*
//    |                                        |
//    |   AccX[0]AccY[0]   AccX[1]AccY[1]      |
//    |   *---------------------*              |
//    |   |    ROI region       |              |
//    |   |                     |              |
//    |   *---------------------*              |
//    |   AccX[2]AccY[2]   AccX[3]AccY[3]      |
//    *----------------------------------------*
//--------------------------------------------------
static inline void _MI_VPE_RoiCalcRegionToHal(const MI_SYS_WindowRect_t *pstCropInfo, const MI_SYS_WindowRect_t *pstRegion, HalVpeIqWdrRoiConfig_t *pstHalRoi)
{
    pstHalRoi->u16RoiAccX[0] = pstRegion->u16X;
    pstHalRoi->u16RoiAccY[0] = pstRegion->u16Y;

    pstHalRoi->u16RoiAccX[1] = pstRegion->u16X + pstRegion->u16Width;
    pstHalRoi->u16RoiAccY[1] = pstRegion->u16Y;


    pstHalRoi->u16RoiAccX[2] = pstRegion->u16X;
    pstHalRoi->u16RoiAccY[2] = pstRegion->u16Y + (pstRegion->u16Height * pstCropInfo->u16Width);

    pstHalRoi->u16RoiAccX[1] = pstRegion->u16X + pstRegion->u16Width;
    pstHalRoi->u16RoiAccY[1] = pstRegion->u16Y + (pstRegion->u16Height * pstCropInfo->u16Width);
}

static MI_BOOL _MI_VPE_RoiGetTask(mi_vpe_DevInfo_t *pstDevInfo, ROI_Task_t *pstRoiTask, list_head *pListHead, struct semaphore *pSemphore)
{
    MI_BOOL bRet = FALSE;
    list_head pos, n;
    ROI_Task_t *pstRoi = NULL;
    if ((pstDevInfo->eRoiStatus == E_MI_VPE_ROI_STATUS_IDLE)
        || (pstDevInfo->eRoiStatus == E_MI_VPE_ROI_STATUS_UPDATED)
        )
    {
        up(pSemphore);
        list_for_each_safe(&pos, &n, pListHead)
        {
            pstRoi = container_of(&pos, ROI_Task_t, list);
            if ((pstRoi->eRoiStatus == E_MI_VPE_ROI_STATUS_NEED_UPDATE)
                && (pstRoi->pstChnnInfo->VpeCh == pstTask->u32ChnId))
            {
                pstRoi->eRoiStatus   = E_MI_VPE_ROI_STATUS_RUNNING;
                pstDevInfo->eRoiStatus = E_MI_VPE_ROI_STATUS_RUNNING;
                bRet = TRUE;
                break;
            }
        }
        down(pSemphore);
    }
    return bRet;
}

static MI_BOOL _MI_VPE_RoiProcessTaskStart(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo, MI_SYS_FrameData_t *pstBuffFrame, ROI_Task_t *pstRoiTask)
{
    // ROI region set by MI API: MI_VPE_ATOM_GetChannelRegionLuma
    HalVpeIqWdrRoiHist_t stCfg;
    HalVpeIqWdrRoiConfig_t stIqRoiCfg[ROI_WINDOW_MAX];
    MI_SYS_WindowRect_t *pstRegion = NULL;
    MI_SYS_FrameData_t *pstFrameData = NULL;
    int i = 0;

    stCfg.bEn = TRUE;
    stCfg.u8WinCount = (MI_U8)(pstRoiTask->pstRegion->u32RegionNum & 0xff);
    stCfg.enPipeSrc  = E_HAL_IQ_ROISRC_AFTER_WDR;

    pstRegion = pstRoiTask->pstRegion->pstRegion;
    // ROI base on Crop Window
    for (i = 0; i < sizeof(stIqRoiCfg)/sizeof(stIqRoiCfg[0]); i++)
    {
       if (i < pstRoiTask->pstRegion->u32RegionNum)
       {
           _MI_VPE_RoiCalcRegionToHal(pstBuffFrame, pstRegion, &stIqRoiCfg[i]);
           stIqRoiCfg[i].bEnSkip = FALSE;
       }
       else
       {
           stIqRoiCfg[i].bEnSkip = TRUE;
       }
       stCfg.pstRoiCfg[i] = &stIqRoiCfg[i];
       pstRegion++;
    }
    HalVpeIqSetWdrRoiHist(pstChnnInfo->pIqCtx, &stCfg);
    pstTask->u64Reserved1 = pstRoiTask;
    pstTask->u32Reserved1 |= MI_VPE_TASK_ROI_UPDATE;
    // register release for ROI
    HalVpeIqSetWdrRoiMask(pstChnnInfo->pIqCtx, FALSE, pstDevInfo->pstCmdMloadInfo);
}

static void _MI_VPE_RoiProcessTaskEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    // Mask ROI register update.
    HalVpeIqSetWdrRoiMask(pstChnnInfo->pIqCtx, TRUE, pstDevInfo->pstCmdMloadInfo);
}

static void _MI_VPE_RoiProcessFinish(mi_vpe_DevInfo_t * pstDevInfo, mi_vpe_ChannelInfo_t * pstChnnInfo, mi_sys_ChnTaskInfo_t *pstTask)
{
    HalVpeIqWdrRoiReport_t stRoi;
    memset(&stRoi, 0, sizeof(stRoi));
    if (TRUE == HalVpeIqGetWdrRoiHist(pstChnnInfo->pIqCtx, &stRoi))
    {
        // 1. for user API need check region max support in HW.
        // 2. HW caps: put in HAL.
        // Notify ROI Data already
        //pstDevInfo->stRoiInfo.eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
        //wake_up(&roi_task->vpe_roi_waitqueue);
        ROI_Task_t *pstRoiTask = (ROI_Task_t *)pstTask->u64Reserved1;
        if (pstRoiTask != NULL)
        {
            pstRoiTask->u32LumaData[0] = stRoi.u32Y[0];
            pstRoiTask->u32LumaData[1] = stRoi.u32Y[1];
            pstRoiTask->u32LumaData[2] = stRoi.u32Y[2];
            pstRoiTask->u32LumaData[3] = stRoi.u32Y[3];
            pstRoiTask->eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
            wake_up(&pstRoiTask->queue);
        }
        pstDevInfo->stRoiInfo.eRoiStatus = E_MI_VPE_ROI_STATUS_UPDATED;
    }
}

static void _MI_VPE_3DnrSwitchChannel(mi_vpe_DevInfo_t *pstDevInfo, list_head *pListHead, struct semaphore *pSemphore)
{
    MI_BOOL bFind = FALSE;
    mi_vpe_ChannelInfo_t *pstChInfo = NULL;
    list_head pos, n;

    if ((E_MI_VPE_3DNR_STATUS_IDLE == pstDevInfo->st3DNRUpdate.eStatus)
        || (E_MI_VPE_3DNR_STATUS_UPDATED == pstDevInfo->st3DNRUpdate.eStatus)
        || ((E_MI_VPE_3DNR_STATUS_NEED_UPDATE == pstDevInfo->st3DNRUpdate.eStatus)
            && (pstDevInfo->st3DNRUpdate.u32WaitScriptNum < MI_VPE_3DNR_MAX_WAITING_NUM))
        )
    {
        up(pSemphore);
        list_for_each_safe(&pos, &n, pListHead)
        {
            pstChInfo = container_of(&pos, mi_vpe_ChannelInfo_t, list);
            if (pstChInfo->VpeCh == pstDevInfo->st3DNRUpdate.VpeCh)
            {
                if (list_is_last(&pos, pListHead)) // End of active list, need goto list head.
                {
                    pstChInfo = container_of(&(pListHead->next), mi_vpe_ChannelInfo_t, list);
                }
                else // Find next channel need 3DNR update
                {
                    pstChInfo = container_of(&n, mi_vpe_ChannelInfo_t, list);
                }
                bFind = TRUE;
                break;
            }
        }
        down(pSemphore);

        if (bFind == FALSE) // Can not find channel, just start from list head.
        {
            pstChInfo = container_of(&(pListHead->next), mi_vpe_ChannelInfo_t, list);
        }
        pstDevInfo->st3DNRUpdate.VpeCh = pstChInfo->VpeCh;
        pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_NEED_UPDATE;
        pstDevInfo->st3DNRUpdate.u32WaitScriptNum = 0;
    }

}

static MI_BOOL _MI_VPE_3DnrProcessStart(mi_vpe_DevInfo_t *pstDevInfo)
{
    MI_BOOL bNeed3DnrUpdate = FALSE;
    if ((pstDevInfo->st3DNRUpdate.eStatus == E_MI_VPE_3DNR_STATUS_NEED_UPDATE) && (pstDevInfo->st3DNRUpdate.VpeCh == pstTask->u32ChnId))
    {
        pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_RUNNING;
        bNeed3DnrUpdate = TRUE;
        // Register release for 3DNR
        HalVpeIqSetDnrTblMask(pstChnnInfo->pIqCtx, FALSE, pstDevInfo->pstCmdMloadInfo);
    }
    return bNeed3DnrUpdate;
}

static void _MI_VPE_3DnrProcessEnd(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    // Mask 3D NR register update.
    HalVpeIqSetDnrTblMask(pstChnnInfo->pIqCtx, TRUE, pstDevInfo->pstCmdMloadInfo);
}

static void _MI_VPE_3DnrProcessFinish(mi_vpe_DevInfo_t *pstDevInfo, mi_vpe_ChannelInfo_t *pstChnnInfo)
{
    // HAL read 3D NR register
    HalVpeSclRead3DNRTbl(pstChnnInfo->pSclCtx);
    // update 3D NR finish
    pstDevInfo->st3DNRUpdate.eStatus = E_MI_VPE_3DNR_STATUS_UPDATED;
}

MI_VPE_Process_TASK(mi_sys_ChnTaskInfo_t *pstTask, mi_vpe_DevInfo_t *pstDevInfo, MI_SYS_FrameData_t *pstBuffFrame)
{
    int i = 0;
    MI_BOOL bNeedRoi, bNeed3DnrUpdate;
    MI_BOOL bUseMDWIN = FALSE;
    mi_vpe_ChannelInfo_t *pstChnnInfo = &pstDevInfo->stChnnInfo[pstTask->u32ChnId];
    HalVpeSclOutputBufferConfig_t stVpeOutputBuffer;
    HalVpeIspVideoInfo_t stIspVideoInfo;
    list_head pos, n;
    ROI_Task_t *pstRoiTask = NULL;

    // Init reserved info
    pstTask->u32Reserved1 = 0;

    // OSD region process
    // TODO: Need add region OSD API.
    VPE_osd_process(MI_REG_CmdInfo_t CmdInfo, cmd_mload_interface * cmdinf);

    // Check ROI
    bNeedRoi = _MI_VPE_RoiGetTask(pstDevInfo, pstRoiTask, &VPE_roi_task_list, &VPE_roi_task_list_sem);
    if (bNeedRoi == TRUE)
    {
        _MI_VPE_RoiProcessTaskStart(pstTask, pstDevInfo, pstChnnInfo, pstRoiTask);
    }

    // Check 3DNR
    bNeed3DnrUpdate = _MI_VPE_3DnrProcessStart(pstDevInfo);

    // Update SCL OutputBuffer
    memset(&stVpeOutputBuffer, 0, sizeof(stVpeOutputBuffer));
    for (i = 0; i < pstChnnInfo->u32OutputPortNum ; i++)
    {
        mi_vpe_OutPortInfo_t *pstPortInfo = NULL;
        MI_U32 u32PortId = pstTask->au32OutputPortId[i];
        pstPortInfo =  &pstChnnInfo->stOutPortInfo[u32PortId];
        // Port enable
        if (pstPortInfo->bEnable == TRUE)
        {
            // Tommy: need add check output buffer --> disp window change ??
            if ((pstTask->astOutputPortBufInfo[u32PortId].stFrameData.u16Width != pstPortInfo->stPortMode.u16Height)
                || (pstTask->astOutputPortBuffInf[u32PortId].stFrameData.u16Height != pstPortInfo->stPortMode.u16Width)
                )
            {
                pstPortInfo->stPortMode.u16Width = pstTask->astOutputPortBufInfo[u32PortId].stFrameData.u16Width;
                pstPortInfo->stPortMode.u16Height = pstTask->astOutputPortBuffInf[u32PortId].stFrameData.u16Height;
                HalVpeSclOutputSizeConfig_t stOutputSize;
                memset(&stOutputSize, 0, sizeof(stOutputSize));
                stOutputSize.enOutPort = (HalVpeSclOutputPort_e)(u32PortId);
                stOutputSize.u16Width  = pstPortInfo->stPortMode.u16Width;
                stOutputSize.u16Height = pstPortInfo->stPortMode.u16Height;
                HalVpeSclOutputSizeConfig(pstChnnInfo->pSclCtx, &stOutputSize);
            }

            // Port MDWIN enable
            if (u32PortId == VPE_OUTPUT_PORT_FOR_MDWIN)
            {
                bUseMDWIN = TRUE;
            }

            // Enable
            stVpeOutputBuffer.stCfg[u32PortId].bEn = TRUE;
            // Address
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u32Stride[0] = pstTask->astOutputPortBufInfo[i].stFrameData.u32Stride[0];
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u32Stride[1] = pstTask->astOutputPortBufInfo[i].stFrameData.u32Stride[1];
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u32Stride[2] = pstTask->astOutputPortBufInfo[i].stFrameData.u32Stride[2];
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u64PhyAddr[0] = pstTask->astOutputPortBufInfo[i].stFrameData.u64PhyAddr[0];
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u64PhyAddr[1] = pstTask->astOutputPortBufInfo[i].stFrameData.u64PhyAddr[1];
            stVpeOutputBuffer.stCfg[u32PortId].stBufferInfo.u64PhyAddr[2] = pstTask->astOutputPortBufInfo[i].stFrameData.u64PhyAddr[2];
        }
        else
        {
            stVpeOutputBuffer.stCfg[u32PortId].bEn = FALSE;
        }
    }
    HalVpeSclProcess(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo, &stVpeOutputBuffer);

    // Update IQ context
    HalVpeIqProcess(pstChnnInfo->pIqCtx,   pstDevInfo->pstCmdMloadInfo);

    // Update ISP input buffer address
    memset(&stIspVideoInfo, 0, sizeof(stIspVideoInfo));
    // input buffer address
    stIspVideoInfo.u32Stride[0] = pstBuffFrame->u32Stride[0];
    stIspVideoInfo.u32Stride[1] = pstBuffFrame->u32Stride[1];
    stIspVideoInfo.u32Stride[2] = pstBuffFrame->u32Stride[2];
    stIspVideoInfo.u64PhyAddr[0] = pstBuffFrame->u64PhyAddr[0] + pstChnnInfo->u64PhyAddrOffset[0];
    stIspVideoInfo.u64PhyAddr[1] = pstBuffFrame->u64PhyAddr[1] + pstChnnInfo->u64PhyAddrOffset[1];
    stIspVideoInfo.u64PhyAddr[2] = pstBuffFrame->u64PhyAddr[2] + pstChnnInfo->u64PhyAddrOffset[2];
    HalVpeIspProcess(pstChnnInfo->pIspCtx, pstDevInfo->pstCmdMloadInfo, &stIspVideoInfo);

    // Wait MDWIN Done
    if (bUseMDWIN == TRUE)
    {
        HalVpeSclSetWaitMdwinDone(pstChnnInfo->pSclCtx, pstDevInfo->pstCmdMloadInfo);
    }

    if (bNeedRoi == TRUE)
    {
        _MI_VPE_RoiProcessTaskEnd(pstDevInfo, pstChnnInfo);
    }

    if (bNeed3DnrUpdate == TRUE)
    {
        _MI_VPE_3DnrProcessEnd(pstDevInfo, pstChnnInfo);
    }
}

static MI_BOOL _MI_VPE_CheckInputChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
    MI_SYS_FrameData_t *pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);
    // Check source window changed ?
    if ((pstChnInfo->stSrcWin.u16Width == pstBuffFrame->u16Width)
        && (pstChnInfo->stSrcWin.u16Height == pstBuffFrame->u16Height)
        &&(pstChnInfo->stChnnAttr.ePixFmt) == pstBuffFrame->ePixelFormat)
        )
    {
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
        pstChnInfo->stSrcWin.u16Width = pstBuffFrame->u16Width;
        pstChnInfo->stSrcWin.u16Height= pstBuffFrame->u16Height;
        pstChnInfo->stChnnAttr.ePixFmt = pstBuffFrame->ePixelFormat;
    }

    return bRet;
}

static MI_BOOL _MI_VPE_CheckCropChanged(mi_sys_ChnTaskInfo_t *pstChnTask)
{
    MI_BOOL bRet = FALSE;
    MI_SYS_WindowRect_t stRealCrop = {0,0,0,0};
    mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);

    // Calculator real cropWindow
    if ((pstChnInfo->stCropWin.u16X + pstChnInfo->stCropWin.u16Width) > pstChnInfo->stSrcWin.u16Width)
    {
        bRet = TRUE;
        stRealCrop.u16X     = 0;
        stRealCrop.u16Width = pstChnInfo->stSrcWin.u16Width;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16X     = pstChnInfo->stCropWin.u16X;
        stRealCrop.u16Width = pstChnInfo->stCropWin.u16Width;

    }
    if ((pstChnInfo->stCropWin.u16Y + pstChnInfo->stCropWin.u16Height) > pstChnInfo->stSrcWin.u16Height)
    {
        bRet = TRUE;
        stRealCrop.u16Y     = 0;
        stRealCrop.u16Height = pstChnInfo->stSrcWin.u16Height;
    }
    else
    {
        bRet = FALSE;
        stRealCrop.u16Y     = pstChnInfo->stCropWin.u16Y;
        stRealCrop.u16Height = pstChnInfo->stCropWin.u16Height;
    }

    // Change Crop window Changed?
    if ((bRet == TRUE) && ((stRealCrop.u16X != pstChnInfo->stCropWin.u16X)
        || (stRealCrop.u16Width != pstChnInfo->stCropWin.u16Width)
        || (stRealCrop.u16Y != pstChnInfo->stCropWin.u16Y)
        || (stRealCrop.u16Height != pstChnInfo->stCropWin.u16Height)
        ))
    {
         pstChnInfo->stRealCrop.u16X = stRealCrop.u16X;
         pstChnInfo->stRealCrop.u16Width = stRealCrop.u16Width;
         pstChnInfo->stRealCrop.u16Y = stRealCrop.u16Y;
         pstChnInfo->stRealCrop.u16Height = stRealCrop.u16Height;

        bRet = TRUE;
    }

    return bRet;
}

#define MIU_BURST_BITS        (256)
#define YUV422_PIXEL_ALIGN    (2)
#define YUV422_BYTE_PER_PIXEL (2)

#define YUV420_PIXEL_ALIGN    (2)
#define YUV420_BYTE_PER_PIXEL (1)


static MI_BOOL _MI_VPE_CalcCropInfo(mi_vpe_ChannelInfo_t *pstChnInfo, MI_SYS_WindowRect_t *pstHwCrop, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = TRUE;
    int left_top_x, left_top_y, right_bottom_x, right_bottom_y;
    MI_U32 u32PixelAlign = 0, u32PixelPerBytes = 0;
    switch (pstChnInfo->stChnnAttr->ePixFmt)
    {
    case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        u32PixelAlign    = YUV422_PIXEL_ALIGN;
        u32PixelPerBytes = YUV422_BYTE_PER_PIXEL;
        break;
    case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        u32PixelAlign    = YUV420_PIXEL_ALIGN;
        u32PixelPerBytes = YUV420_BYTE_PER_PIXEL;
        break;
    default:
        bRet = FALSE;
    }

    if (bRet == TRUE)
    {
        left_top_y = ALIGN_DOWN(pstChnInfo->stRealCropWin.u16Y, u32PixelAlign);
        left_top_x = ALIGN_DOWN(pstChnInfo->stRealCropWin.u16X, (MIU_BURST_BITS/(u32PixelPerBytes*8)));
        right_bottom_y = ALIGN_UP(pstChnInfo->stRealCropWin.u16Y + pstChnInfo->stRealCropWin.u16Height, u32PixelAlign);
        right_bottom_x = ALIGN_UP(pstChnInfo->stRealCropWin.u16X + pstChnInfo->v.u16Width, (MIU_BURST_BITS/(u32PixelPerBytes*8)));

        pstSwCrop->u16X = left_top_x;
        pstSwCrop->u16Width = right_bottom_x-left_top_x;
        pstSwCrop->u16Y = left_top_y;
        pstSwCrop->u16Heigth = right_bottom_y-left_top_y;

        pstHwCrop->u16X = pstChnInfo->stCropWin.u16X-left_top_x;
        pstHwCrop->u16Y = pstChnInfo->stCropWin.u16Y-left_top_y;

        pstHwCrop->u16Width = pstChnInfo->stCropWin.u16Width;
        pstHwCrop->u16Heigth = pstChnInfo->stCropWin.u16Height;
    }
    else
    {
        DBG_WRN("UnSupport pixel format: %d.\n", pstChnInfo->stChnnAttr->ePixFmt);
    }

    return bRet;
}

static MI_BOOL _MI_VPE_UpdateCropAddress(mi_sys_ChnTaskInfo_t *pstChnTask, MI_SYS_WindowRect_t *pstSwCrop)
{
    MI_BOOL bRet = FALSE;
    MI_PHY u64PhyAddr = 0;
    if (pstChnTask != NULL)
    {
        mi_vpe_ChannelInfo_t *pstChnInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
        MI_SYS_FrameData_t *pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);

        switch (pstBuffFrame->ePixelFormat)
        {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
        {
            MI_U32 u32BytePerPixel = 2;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]) * u32BytePerPixel;
            u64PhyAddr += pstSwCrop->u16X * u32BytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[1] = 0;
            pstChnInfo->u64PhyAddrOffset[2] = 0;

        }
        break;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
        {
            MI_U32 u32LumaBytePerPixel = 1;
            u64PhyAddr =  (pstSwCrop->u16Y * pstBuffFrame->u32Stride[0]) * u32LumaBytePerPixel;
            u64PhyAddr += pstSwCrop->u16X * u32LumaBytePerPixel;
            pstChnInfo->u64PhyAddrOffset[0] += u64PhyAddr;

            u64PhyAddr =   (pstSwCrop->u16Y/2 * pstBuffFrame->u32Stride[0]);
            u64PhyAddr +=  pstSwCrop->u16X; // TODO: tommy: Check UV whether need /2 ?
            pstChnInfo->u64PhyAddrOffset[1] = u64PhyAddr;
            pstChnInfo->u64PhyAddrOffset[2] = 0;
        }
        break;
        default:
            bRet = FALSE;
        }
    }

    return bRet;
}

#define REPEAT_MAX_NUMBER (2)

void VPEWorkThread(void *data)
{
    MI_U16 fence = 0;
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)data;
    cmd_mload_interface *cmdinf = pstDevInfo->pstCmdMloadInfo;
    while(1)
    {
        mi_vpe_IteratorWorkInfo_t workinfo;
        list_head pos,n;
        workinfo.totalAddedTask = 0;

        mi_sys_DevTaskIterator(pstDevInfo->hDevSysHandle, _MI_VPE_TaskIteratorCallBK, &workinfo);
        if(list_empty(&VPE_todo_task_list))
        {
            schedule();
            // Tommy: Need description for this API behavior: eg: input buffer will return immediately.
            mi_sys_WaitOnInputTaskAvailable(E_MI_SYS_MODULE_ID_VPE, 0, 100);
            continue;
        }

        // 3DNR Channel switch
        _MI_VPE_3DnrSwitchChannel(pstDevInfo, &VPE_active_channel_list, &VPE_active_channel_list_sem);

        list_for_each_safe(&pos, &n, &VPE_todo_task_list)
        {
            mi_sys_ChnTaskInfo_t *pstChnTask;
            MI_PHY menuload_buf_ring_beg_addr;
            MI_PHY menuload_buf_ring_end_addr;
            int loop_cnt = 0;
            int repeatNum = 1;
            HalVpeSclInputSizeConfig_t stCfg;
            MI_SYS_FrameData_t *pstBuffFrame = NULL;

            mi_vpe_ChannelInfo_t *pstChInfo = GET_VPE_CHNN_PTR(pstChnTask->u32ChnId);
            pstChnTask = container_of(&pos, mi_sys_ChnTaskInfo_t, list);

            pstBuffFrame = GET_VPE_BUFF_FRAME_FROM_TASK(pstChnTask);
            // 1. Check input size change ??
            if (_MI_VPE_CheckInputChanged(pstChnTask)== TRUE)
            {
                // Set SCL input
                // Tommy: Think about repeat: input size change /sw crop
                // Need check with HAL wheather add Hal API for get NR reference Buffer.
                repeatNum = REPEAT_MAX_NUMBER;
                memset(&stCfg, 0, sizeof(stCfg));
                stCfg.u32Height = pstChInfo->stSrcWin.u16Height;
                stCfg.u32Width  = pstChInfo->stSrcWin.u16Width;
                stCfg.eCompressMode = pstChInfo->eCompressMode;
                stCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                HalVpeSclInputConfig(pstChInfo->pSclCtx, &stCfg);
            }

            // 2. sw crop
            // Check Crop
            // TODO: Tommy check crop need ???
            if (_MI_VPE_CheckCropChanged(pstChnTask) == TRUE)
            {
                MI_SYS_WindowRect_t stHwCrop;
                MI_SYS_WindowRect_t stSwCrop;
                HalVpeSclCropConfig_t stHwCropCfg;
                _MI_VPE_CalcCropInfo(pstChnTask, &stHwCrop, &stSwCrop);
                _MI_VPE_UpdateCropAddress(pstChnTask, &stSwCrop);

                // Set SCL input <-- SW crop
                repeatNum = REPEAT_MAX_NUMBER;
                memset(&stCfg, 0, sizeof(stCfg));
                stCfg.u32Height = stSwCrop.u16Height;
                stCfg.u32Width  = stSwCrop.u16Width;
                stCfg.eCompressMode = pstChInfo->eCompressMode;
                stCfg.ePixelFormat  = pstChInfo->stChnnAttr.ePixFmt;
                HalVpeSclInputConfig(pstChInfo->pSclCtx, &stCfg);

                // Set Crop info <- HW crop
                memset(&stHwCropCfg, 0, sizeof(stHwCropCfg));
                stHwCropCfg.bCropEn = TRUE;
                stHwCropCfg.stCropWin.u16X = stHwCrop.u16X;
                stHwCropCfg.stCropWin.u16Y = stHwCrop.u16Y;
                stHwCropCfg.stCropWin.u16Width = stHwCrop.u16Width;
                stHwCropCfg.stCropWin.u16Height = stHwCrop.u16Height;
                HalVpeSclCropConfig(pstChInfo->pSclCtx, &stHwCropCfg);
            }
            pstDevInfo->st3DNRUpdate.u32WaitScriptNum++;

            for (;repeatNum > 0; repeatNum--)
            {
                while(!cmdinf->ensure_cmdbuf_available(cmdinf, 0x400, 0x1000))
                {
                    interruptible_sleep_on_timeout(&vpe_isr_waitqueue, msecs_to_jiffies(VPE_WORK_THREAD_WAIT));
                    loop_cnt++;
                    if(loop_cnt>1000)
                        BUG();//engine hang
                }

                MI_VPE_Process_TASK(pstChnTask, cmdinf, pstBuffFrame);
                menuload_buf_ring_end_addr = cmdinf->get_menuload_ringbuf_write_ptr(cmdinf);
                cmdinf->write_reg_cmdq_mask(cmdinf, MI_VPE_FENCE_REGISTER, fence, 0xffff);
                cmdinf->kick_off_cmdq(cmdinf);
                pstChnTask->u32Reserved0 = fence++;
                pstChnTask->u64Reserved0 = cmdinf->get_next_mload_ringbuf_write_ptr(cmdinf, 0x1000);;
            }

            list_del(pstChnTask->list);
            down(&VPE_working_list_sem);
            list_add_tail(&pstChnTask->list, &VPE_working_task_list);
            up(&VPE_working_list_sem);

        }
    }
}

static MI_RESULT _MI_VPE_OnBindChnnInputputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    if ((pstChnCurryPort->eModId == E_MI_SYS_MODULE_ID_VPE)    // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->s32ChnId) // Check support Channel
#if 0
        && ((pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VIF) // Check supported input module ?
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VPE))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stPeerInputPortInfo = *pstChnPeerPort;
        return MI_OK;
    }
    else
    {
        return MI_ERR_NOT_SUPPORT;
    }
}

static MI_RESULT _MI_VPE_OnUnBindChnnInputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    if ((pstChnCurryPort->eModId == E_MI_SYS_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->s32ChnId)   // Check support Channel
#if 0
    && ((pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VIF)      // Check supported output module
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VPE))
#endif
        )
    {
        // Need check Chnn Busy ???
        // if(list_empty(&VPE_todo_task_list))
        // ... ...
        memset(&pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stPeerInputPortInfo, 0, sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stPeerInputPortInfo));
        return MI_OK;
    }
    else
    {
        return MI_ERR_NOT_SUPPORT;
    }
}

static MI_RESULT _MI_VPE_OnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    if ((pstChnCurryPort->eModId == E_MI_SYS_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->s32ChnId)   // Check support Channel
        && MI_VPE_CHECK_PORT_SUPPORTED(pstChnCurryPort->s32PortId)  // Check support Port Id
#if 0
    && ((pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DISP))
#endif
        )
    {
        pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stOutPortInfo[pstChnCurryPort->s32PortId].stPeerOutputPortInfo = *pstChnPeerPort;
        return MI_OK;
    }
    else
    {
        return MI_ERR_NOT_SUPPORT;
    }
}

static MI_RESULT _MI_VPE_OnUnBindChnnOutputCallback(MI_SYS_ChnPort_t *pstChnCurryPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    mi_vpe_DevInfo_t *pstDevInfo = (mi_vpe_DevInfo_t *)pUsrData;
    if ((pstChnCurryPort->eModId == E_MI_SYS_MODULE_ID_VPE)         // Check Callback information ok ?
        && MI_VPE_CHECK_CHNN_SUPPORTED(pstChnCurryPort->s32ChnId)   // Check support Channel
        && MI_VPE_CHECK_PORT_SUPPORTED(pstChnCurryPort->s32PortId)  // Check support Port Id
#if 0 // No use
    && ((pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DIVP)     // supported input module
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VENC)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_VDISP)
            || (pstChnPeerPort->eModId == E_MI_SYS_MODULE_ID_DISP))
#endif
        )
    {
        memset (&pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stOutPortInfo[pstChnCurryPort->s32PortId].stPeerOutputPortInfo, 0,
            sizeof(pstDevInfo->stChnnInfo[pstChnCurryPort->s32ChnId].stOutPortInfo[pstChnCurryPort->s32PortId].stPeerOutputPortInfo));
        return MI_OK;
    }
    else
    {
        return MI_ERR_NOT_SUPPORT;
    }

    // Check Busy ??
    // if(list_empty(&VPE_working_task_list))
 }

static MI_S32 _MI_VPE_Init(void)
{
    MI_S32 s32Ret = MI_ERR_VPE_NULL_PTR;
    mi_sys_ModuleDevBindOps_t stVPEPOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();

    if (pstDevInfo->bInited == TRUE)
    {
        return MI_ERR_VPE_EXIST;
    }

    // Get VPE IRQ
    if (HalVpeSclGetIrqNum(&pstDevInfo->uVpeIrqNum) == FALSE)
    {
        return MI_ERR_VPE_NOT_SUPPORT;
    }

    // VPE register to mi_sys
    stVPEPOps.OnBindInputPort    = _MI_VPE_OnBindChnnInputputCallback;
    stVPEPOps.OnUnBindInputPort  = _MI_VPE_OnUnBindChnnInputCallback;
    stVPEPOps.OnBindOutputPort   = _MI_VPE_OnBindChnnOutputCallback;
    stVPEPOps.OnUnBindOutputPort = _MI_VPE_OnUnBindChnnOutputCallback;
    memset(&stModInfo, 0x0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.u32ModuleId      = E_MI_SYS_MODULE_ID_VPE;
    stModInfo.u32DevId         = 0;
    stModInfo.u32DevChnNum     = MI_VPE_MAX_CHANNEL_NUM;
    stModInfo.u32InputPortNum  = MI_VPE_MAX_INPUTPORT_NUM;
    stModInfo.u32OutputPortNum = MI_VPE_MAX_PORT_NUM;
    pstDevInfo->hDevSysHandle   = mi_sys_RegisterDev(&stModInfo, &stVPEPOps, &pstDevInfo);

    // Get cmdQ service
    pstDevInfo->pstCmdMloadInfo = get_sys_cmdq_service(CMDQ_ID_VPE, TRUE);
    if (pstDevInfo->pstCmdMloadInfo == NULL)
    {
        goto __get_cmd_service_fail;
    }

    // Create work thread
    pstDevInfo->pstWorkThread = kthread_create(VPEWorkThread, pstDevInfo, "VPE/WorkThread");
    if (IS_ERR(pstDevInfo->pstWorkThread))
    {
        goto __create_work_thread_fail;
    }

    // Create IRQ Bottom handler
    pstDevInfo->pstProcThread = kthread_create(VPE_ISR_Proc_Thread, pstDevInfo, "VPE/IsrProcThread");
    if (IS_ERR(pstDevInfo->pstProcThread))
    {
        goto __create_proc_thread_fail;
    }

    // Register kernel IRQ
    if (0 > (request_irq(pstDevInfo->uVpeIrqNum, VPE_ISR, IRQF_SHARED | IRQF_ONESHOT, "VPE-IRQ", pstDevInfo))
    {
        goto __register_vpe_irq_fail:
    }

    // Enable VPE IRQ
    HalVpeSclEnableIrq(TRUE);
    pstDevInfo->bInited = TRUE;
    s32Ret = MI_VPE_OK;

    return s32Ret;

__register_vpe_irq_fail:
    kthread_stop(pstDevInfo->pstProcThread);
__create_proc_thread_fail:
    kthread_stop(pstDevInfo->pstWorkThread);
__create_work_thread_fail:
    pstDevInfo->pstCmdMloadInfo = NULL;
__get_cmd_service_fail:
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;

    return s32Ret;
}

static void _MI_VPE_DeInit(void)
{
    mi_vpe_DevInfo_t *pstDevInfo = GET_VPE_DEV_PTR();
    HalVpeSclEnableIrq(FALSE);
    pstDevInfo->bInited = FALSE;
    free_irq(pstDevInfo->uVpeIrqNum);
    kthread_stop(pstDevInfo->pstProcThread);
    kthread_stop(pstDevInfo->pstWorkThread);
    pstDevInfo->pstCmdMloadInfo = NULL;
    mi_sys_UnRegisterDev(pstDevInfo->hDevSysHandle);
    pstDevInfo->hDevSysHandle = NULL;
}

module_init(_MDrv_XC_ModuleInit);
module_exit(_MI_VPE_DeInit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MI VPE");
MODULE_LICENSE("GPL");
