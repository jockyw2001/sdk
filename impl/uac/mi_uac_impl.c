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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_uac_impl.c
/// @brief  uac module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/platform_device.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"

#include "mi_uac.h"
#include "mi_alsa.h"
#include "mi_uac_impl.h"

static uac_module_t _uac_module;
static DEFINE_SEMAPHORE(module_sem);

MI_S32 mi_uac_impl_alsa_init(void)
{
#if UAC_IMPL_DBG
    DBG_INFO("UAC ALSA Init\n");
#endif
    if(alsa_card_uac_init()<0){
        DBG_ERR("UAC IMPL: failed to setup alsa device \n");
        return -1;
    }
    return MI_SUCCESS;
}

MI_S32 mi_uac_impl_alsa_exit(void)
{
#if UAC_IMPL_DBG
    DBG_INFO("UAC ALSA Exit\n");
#endif
    if(alsa_card_uac_exit()<0){
        DBG_ERR("UAC IMPL: failed to setup alsa device \n");
        return -1;
    }
    return MI_SUCCESS;
}

MI_S32 MI_UAC_IMPL_Init(void)
{
    MI_S8  i = 0;
    MI_S32 ret = -1;
#if UAC_IMPL_DBG
    DBG_INFO("UAC Module Init\n");
#endif
    down(&module_sem);
    if(_uac_module.binited){
        DBG_ERR("UAC IMPL: inited already\n");
        ret=MI_UAC_ERR_MOD_INITED;
        goto exit;
    }

    memset(&_uac_module,0,sizeof(_uac_module));
    for(i=0;i<UAC_MAX_DEVICE_NUM;i++){
        _uac_module.dev[i].status = UAC_DEVICE_UNINIT;
        mutex_init(&_uac_module.dev[i].mtx);
    }

    _uac_module.binited = 1;
    ret=MI_SUCCESS;
exit:
    up(&module_sem);
    return 0;
}

MI_S32 MI_UAC_IMPL_Exit(void)
{
    MI_S8 i=0;
    MI_S32 ret=-1;
    uac_device_t *dev=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("UAC Module Exit\n");
#endif
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }

    for(i=0;i<UAC_MAX_DEVICE_NUM;i++){
        dev=&(_uac_module.dev[i]);
        if(UAC_DEVICE_START == dev->status){
            DBG_INFO("Device %d not stopped\n", i);
            up(&module_sem);
            MI_UAC_IMPL_StopDev(i);
            down(&module_sem);
        }

        if(UAC_DEVICE_UNINIT != dev->status){
            DBG_INFO("Device %d not closed\n", i);
            up(&module_sem);
            MI_UAC_IMPL_CloseDevice(i);
            down(&module_sem);
        }
    }

    for(i=0;i<UAC_MAX_DEVICE_NUM;i++){
        mutex_destroy(&_uac_module.dev[i].mtx);
    }
    _uac_module.binited=0;
    ret=MI_SUCCESS;
exit:
    up(&module_sem);
    return 0;
}

//for AI Bind Capture InputPort
static MI_S32 uac_OnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    uac_device_t *dev=(uac_device_t *)pUsrData;
    uac_inputport_t *inputport;
    int ret=-1;
#if UAC_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!UAC_VALID_MODID(pstChnCurPort->eModId)
       || !UAC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !UAC_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !UAC_VALID_INPUTPORTID(pstChnCurPort->u32PortId)){
        DBG_ERR("Invalid cur port\n");
        return -1;
    }
    if(UAC_CAPTURE_DEV!=pstChnCurPort->u32DevId)
    {
        DBG_ERR("Is not a Capture Channel\n");
        return -1;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        goto exit;
    }
    mutex_lock(&dev->mtx);
    inputport= &dev->channel[pstChnCurPort->u32ChnId].inputport[pstChnCurPort->u32PortId];
    if(dev->status==UAC_DEVICE_UNINIT){
        DBG_ERR("Device not open\n");
        goto exit_device;
    }
    MI_SYS_BUG_ON(inputport->bbind);
//TODO
    inputport->bindport=*pstChnPeerPort;
    inputport->bbind=1;

    if(inputport->status==UAC_INPUTPORT_ENABLED){
//TODO
    }
    ret=0;
    goto exit_device;

exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
//For AI UnBind Capture InputPort
static MI_S32 uac_OnUnBindInputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    uac_device_t *dev=(uac_device_t *)pUsrData;
    int ret=-1;
    uac_inputport_t *inputport;
#if UAC_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!UAC_VALID_MODID(pstChnCurPort->eModId)
       || !UAC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !UAC_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !UAC_VALID_INPUTPORTID(pstChnCurPort->u32PortId)){
        DBG_ERR("Invalid cur port\n");
        return -1;
    }
    if(UAC_CAPTURE_DEV!=pstChnCurPort->u32DevId)
    {
        DBG_ERR("Is not a Capture Channel\n");
        return -1;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        goto exit;
    }

    mutex_lock(&dev->mtx);
    inputport=&dev->channel[pstChnCurPort->u32ChnId].inputport[pstChnCurPort->u32PortId];
    if(dev->status==UAC_DEVICE_UNINIT){
        DBG_ERR("Device not open\n");
        goto exit_device;
    }
    if(!inputport->bbind){
        DBG_ERR("Input port not bound\n");
        goto exit_device;
    }
//TODO
    memset(&inputport->bindport, 0, sizeof(MI_SYS_ChnPort_t));
    inputport->bbind=0;

    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
/* For AO Bind Playback Outputport */
static MI_S32 uac_OnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    uac_device_t *dev=(uac_device_t *)pUsrData;
    uac_outputport_t *outputport;
    int ret=-1;
#if UAC_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!UAC_VALID_MODID(pstChnCurPort->eModId)
       || !UAC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !UAC_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !UAC_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId)){
        return -1;
    }
    if(UAC_PLAYBACK_DEV!=pstChnCurPort->u32DevId)
    {
        DBG_ERR("Is not a PlayBack Channel\n");
        return -1;
    }
    down(&module_sem);
    if(!_uac_module.binited)
        goto exit;

    mutex_lock(&dev->mtx);
    if(dev->status==UAC_DEVICE_UNINIT || dev->status==UAC_DEVICE_START)
        goto exit_device;
    outputport = &dev->channel[pstChnCurPort->u32ChnId].outputport[pstChnCurPort->u32PortId];

    outputport->bbind=1;
    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}
/* For AO UnBind Playback Outputport */
static MI_S32 uac_OnUnBindOutputPort(MI_SYS_ChnPort_t *pstChnCurPort, MI_SYS_ChnPort_t *pstChnPeerPort ,void *pUsrData)
{
    uac_device_t *dev=(uac_device_t *)pUsrData;
    uac_outputport_t *outputport;
    int ret=-1;
#if UAC_IMPL_DBG
    DBG_INFO("\n");
#endif
    if(!UAC_VALID_MODID(pstChnCurPort->eModId)
       || !UAC_VALID_DEVID(pstChnCurPort->u32DevId)
       || !UAC_VALID_CHNID(pstChnCurPort->u32ChnId)
       || !UAC_VALID_OUTPUTPORTID(pstChnCurPort->u32PortId)){
        return -1;
    }
    if(UAC_PLAYBACK_DEV!=pstChnCurPort->u32DevId)
    {
        DBG_ERR("Is not a PlayBack Channel\n");
        return -1;
    }
    down(&module_sem);
    if(!_uac_module.binited)
        goto exit;

    mutex_lock(&dev->mtx);
    if(dev->status==UAC_DEVICE_UNINIT || dev->status==UAC_DEVICE_START)
        goto exit_device;
    outputport = &dev->channel[pstChnCurPort->u32ChnId].outputport[pstChnCurPort->u32PortId];
    outputport->bbind=0;
    ret=0;
exit_device:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

#if(MI_UAC_PROCFS_DEBUG == 1)
MI_S32 mi_uac_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
    uac_device_t *dev = NULL;
    dev = &(_uac_module.dev[u32DevId]);

    handle.OnPrintOut(handle, "\n");
    handle.OnPrintOut(handle, "=====================Private UAC%d Info ==========================\n", u32DevId);
    handle.OnPrintOut(handle, "%11d\n", dev->wakeevent);

    return MI_SUCCESS;
}

MI_S32 mi_uac_OnDumpInputPortAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, void *pUsrData)
{
   // MI_S32 i = 0;
    uac_device_t *dev = NULL;
    //uac_inputport_t *inputport = NULL;
    if (!UAC_VALID_DEVID(u32DevId))
    {
        DBG_ERR("Invalid DevId=%d\n", u32DevId);
        return MI_UAC_ERR_INVALID_DEVID;
    }

    dev = &(_uac_module.dev[u32DevId]);

    handle.OnPrintOut(handle, "------------------- Input Port Info --------------------------------\n");

    return MI_SUCCESS;
}

MI_S32 mi_uac_OnDumpOutPortAttr(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData)
{
    uac_device_t *dev = NULL;
    dev = &(_uac_module.dev[u32DevId]);
    handle.OnPrintOut(handle, "------------------- Output Port Info --------------------------------\n");

    return MI_SUCCESS;
}
#endif
MI_S32 MI_UAC_IMPL_OpenDevice(MI_UAC_DEV DevId)
{
    MI_S8  i = 0,j = 0;
    MI_S32 ret = -1;
    uac_device_t *dev=NULL;
    uac_inputport_t  *inputport=NULL;
    uac_outputport_t *outputport=NULL;
    mi_sys_ModuleDevInfo_t modinfo;
    mi_sys_ModuleDevBindOps_t bindops;
#ifdef MI_SYS_PROC_FS_DEBUG
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    memset(&pstModuleProcfsOps,0x00,sizeof(mi_sys_ModuleDevProcfsOps_t));
#endif
    memset(&modinfo,0x00,sizeof(mi_sys_ModuleDevInfo_t));
    memset(&bindops,0x00,sizeof(mi_sys_ModuleDevBindOps_t));
#if UAC_IMPL_DBG
    DBG_INFO("Uac Open device %d\n", DevId);
#endif
    down(&module_sem);
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }

    dev=&(_uac_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(dev->status != UAC_DEVICE_UNINIT){
        DBG_ERR("Device Opened already\n");
        ret=MI_UAC_ERR_DEV_OPENED;
        goto exit_mtx;
    }
    dev->wakeevent=0; //if necessully

/* input port capture dev */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_INPUTPORT_NUM;i++){
                inputport = &dev->channel[j].inputport[i];
                inputport->inputport.eModId    = E_MI_MODULE_ID_UAC;
                inputport->inputport.u32DevId  = DevId;
                inputport->inputport.u32ChnId  = j;
                inputport->inputport.u32PortId = i;
                inputport->status = UAC_INPUTPORT_INIT;
            }
        }
        modinfo.eModuleId = E_MI_MODULE_ID_UAC;
        modinfo.u32DevId  = DevId;
        modinfo.u32InputPortNum  = UAC_TOTAL_INPUTPORT_NUM;
        modinfo.u32OutputPortNum = 0;
        modinfo.u32DevChnNum     = UAC_TOTAL_CHANNEL_NUM;

        bindops.OnBindInputPort        = uac_OnBindInputPort;
        bindops.OnUnBindInputPort      = uac_OnUnBindInputPort;
        bindops.OnBindOutputPort       = NULL;
        bindops.OnUnBindOutputPort     = NULL;
        bindops.OnOutputPortBufRelease =NULL;
#ifdef MI_SYS_PROC_FS_DEBUG
        memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_UAC_PROCFS_DEBUG == 1)
        pstModuleProcfsOps.OnDumpDevAttr       = mi_uac_OnDumpDevAttr;
        pstModuleProcfsOps.OnDumpChannelAttr   = NULL;
        pstModuleProcfsOps.OnDumpInputPortAttr = mi_uac_OnDumpInputPortAttr;
        pstModuleProcfsOps.OnDumpOutPortAttr   = NULL;
        pstModuleProcfsOps.OnHelp              = NULL;
#else
        pstModuleProcfsOps.OnDumpDevAttr       = NULL;
        pstModuleProcfsOps.OnDumpChannelAttr   = NULL;
        pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
        pstModuleProcfsOps.OnDumpOutPortAttr   = NULL;
        pstModuleProcfsOps.OnHelp              = NULL;
#endif
#endif
    }
/* output port playback dev */
    if(UAC_PLAYBACK_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_OUTPUTPORT_NUM;i++){
                outputport = &dev->channel[j].outputport[i];
                outputport->outputport.eModId    = E_MI_MODULE_ID_UAC;
                outputport->outputport.u32DevId  = DevId;
                outputport->outputport.u32ChnId  = j;
                outputport->outputport.u32PortId = i;
                outputport->status = UAC_OUTPUTPORT_INIT;
            }
        }
        modinfo.eModuleId        = E_MI_MODULE_ID_UAC;
        modinfo.u32DevId         = DevId;
        modinfo.u32InputPortNum  = 0;
        modinfo.u32OutputPortNum = UAC_TOTAL_OUTPUTPORT_NUM;
        modinfo.u32DevChnNum     = UAC_TOTAL_CHANNEL_NUM;

        bindops.OnBindInputPort        = NULL;
        bindops.OnUnBindInputPort      = NULL;
        bindops.OnBindOutputPort       = uac_OnBindOutputPort;
        bindops.OnUnBindOutputPort     = uac_OnUnBindOutputPort;
        bindops.OnOutputPortBufRelease =NULL;
#ifdef MI_SYS_PROC_FS_DEBUG
        memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
#if(MI_UAC_PROCFS_DEBUG == 1)
        pstModuleProcfsOps.OnDumpDevAttr       = mi_uac_OnDumpDevAttr;
        pstModuleProcfsOps.OnDumpChannelAttr   = NULL;
        pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
        pstModuleProcfsOps.OnDumpOutPortAttr   = mi_uac_OnDumpOutPortAttr;
        pstModuleProcfsOps.OnHelp              = NULL;
#else
        pstModuleProcfsOps.OnDumpDevAttr       = NULL;
        pstModuleProcfsOps.OnDumpChannelAttr   = NULL;
        pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
        pstModuleProcfsOps.OnDumpOutPortAttr   = NULL;
        pstModuleProcfsOps.OnHelp              = NULL;
#endif
#endif
    }

    dev->devhandle = mi_sys_RegisterDev(&modinfo, &bindops, dev
                                         #ifdef MI_SYS_PROC_FS_DEBUG
                                         , &pstModuleProcfsOps
                                         ,MI_COMMON_GetSelfDir
                                         #endif
                                         );
    if(NULL==dev->devhandle){
        DBG_ERR("Register Module Device fail\n");
        ret=MI_UAC_ERR_FAIL;
        goto exit_mtx;
    }
    dev->status=UAC_DEVICE_INIT;
    ret=MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_UAC_IMPL_CloseDevice(MI_UAC_DEV DevId)
{
    MI_S32 ret = -1;
    MI_U32 i = 0,j = 0;
    uac_device_t *dev=NULL;
    uac_inputport_t  *inputport=NULL;
    uac_outputport_t *outputport=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("Uac Close device %d\n", DevId);
#endif
    down(&module_sem);
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    if(!_uac_module.binited){
        DBG_ERR("Module not init\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(UAC_DEVICE_INIT!=dev->status && UAC_DEVICE_STOP!=dev->status){
        DBG_ERR("Device not stopped: %d\n", dev->status);
        ret=MI_UAC_ERR_DEV_NOT_STOP;
        goto exit_mtx;
    }
/* input port capture dev */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_INPUTPORT_NUM;i++){
                inputport = &dev->channel[j].inputport[i];
                if(inputport->status == UAC_INPUTPORT_ENABLED){
                    DBG_ERR("Inputport %d is still enabled\n", i);
                    ret=MI_UAC_ERR_PORT_NOT_DISABLE;
                    continue;
                }
                if(inputport->bbind) {
                   DBG_ERR("Inputport %d is still bound\n", i);
                   ret=MI_UAC_ERR_PORT_NOT_UNBIND;
                   continue;
                }
                if(inputport->status != UAC_INPUTPORT_UNINIT)
                    inputport->status = UAC_INPUTPORT_UNINIT;
            }
        }
    }
/* output port playback dev */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_OUTPUTPORT_NUM;i++){
                outputport = &dev->channel[j].outputport[i];
                if(outputport->status==UAC_OUTPUTPORT_ENABLED){
                    DBG_ERR("Playback Outputport %d is still enabled\n", i);
                    ret=MI_UAC_ERR_PORT_NOT_DISABLE;
                    continue;
                }
                if(outputport->bbind) {
                   DBG_ERR("Playback Outputport %d is still bound\n", i);
                   ret=MI_UAC_ERR_PORT_NOT_UNBIND;
                   continue;
                }
                if(outputport->status != UAC_OUTPUTPORT_UNINIT)
                    outputport->status = UAC_OUTPUTPORT_UNINIT;
            }
        }
    }

    MI_SYS_BUG_ON(dev->devhandle==NULL);
    mi_sys_UnRegisterDev(dev->devhandle);
    dev->status=UAC_DEVICE_UNINIT;
    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_UAC_IMPL_Get_CapturePortAttr(MI_UAC_DEV DevId,
                MI_UAC_CHN ChnId,
                MI_UAC_PORT PortId,
                MI_UAC_InputPortAttr_t *pstInputPortAttr)
{
    uac_device_t *dev=NULL;
    uac_inputport_t *inputport=NULL;
    MI_S32 ret = -1;
#if UAC_IMPL_DBG
    DBG_INFO("Uac GetInputPortAttr %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    if(!UAC_VALID_CHNID(ChnId) ||
       !UAC_VALID_INPUTPORTID(PortId)){
        DBG_ERR("Invalid Param ChnId=%d,PortId=%d\n",ChnId,PortId);
        return MI_UAC_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    inputport = &dev->channel[ChnId].inputport[PortId];
    mutex_lock(&dev->mtx);
    if(UAC_INPUTPORT_UNINIT>=inputport->status){
        DBG_ERR("Input port not inited\n");
        ret=MI_UAC_ERR_NOT_CONFIG;
        goto exit_mtx;
    }
    *pstInputPortAttr = inputport->attr;
    return MI_SUCCESS;

exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    /* default hw params */
    pstInputPortAttr->access   = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
    pstInputPortAttr->format   = SNDRV_PCM_FORMAT_S16_LE;
    pstInputPortAttr->rate     = 16000;
    pstInputPortAttr->channels = 1;
    up(&module_sem);
    return ret;
}

MI_S32 MI_UAC_IMPL_Get_PlaybackPortAttr(MI_UAC_DEV DevId,
               MI_UAC_CHN ChnId,
               MI_UAC_PORT PortId,
               MI_UAC_OutputPortAttr_t *pstOutputPortAttr)
{
    uac_device_t *dev=NULL;
    uac_outputport_t *outputport=NULL;
    int ret=-1;
#if UAC_IMPL_DBG
    DBG_INFO("Uac Get Outputport: dev=%d, port=%d\n",
            DevId, PortId);
#endif
    if(!UAC_VALID_DEVID(DevId) ){
        DBG_ERR("Invalid Id ,DevId=%d\n", DevId);
        return MI_UAC_ERR_INVALID_DEVID;
    }
    if(!UAC_VALID_CHNID(ChnId) ||
       !UAC_VALID_OUTPUTPORTID(PortId)){
        DBG_ERR("Invalid Param ChnId=%d,PortId=%d\n",ChnId,PortId);
        return MI_UAC_ERR_ILLEGAL_PARAM;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    outputport = &dev->channel[ChnId].outputport[PortId];
    mutex_lock(&dev->mtx);
    if(UAC_OUTPUTPORT_UNINIT>=outputport->status){
        DBG_ERR("Output port not inited\n");
        ret=MI_UAC_ERR_NOT_CONFIG;
        goto exit_mtx;
    }
    *pstOutputPortAttr = outputport->attr;
    return MI_SUCCESS;

exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    /* default hw params */
    pstOutputPortAttr->access   = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
    pstOutputPortAttr->format   = SNDRV_PCM_FORMAT_S16_LE;
    pstOutputPortAttr->rate     = 16000;
    pstOutputPortAttr->channels = 1;
    up(&module_sem);
    return ret;
}

/*
 *  Enable Capture InputPort || Playback InputPort
 */
MI_S32 MI_UAC_IMPL_StartDev(MI_UAC_DEV DevId)
{
    MI_S32 i = 0,j = 0;
    MI_S32 ret = -1;
    uac_device_t     *dev=NULL;
    uac_inputport_t  *inputport=NULL;
    uac_outputport_t *outputport=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("Uac StartDev %d\n", DevId);
#endif
    down(&module_sem);
    if(!UAC_VALID_DEVID(DevId))
    {
        DBG_ERR("Invalid Device Id Or Port Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);

    mutex_lock(&dev->mtx);
    if(UAC_DEVICE_UNINIT==dev->status)
    {
        DBG_ERR("Device not open\n");
        ret=MI_UAC_ERR_NOT_CONFIG;
        goto exit_mtx;
    }
    if(UAC_DEVICE_START == dev->status){
        DBG_INFO("Device is already Opened\n");
        goto done;
    }

/* capture channel */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_INPUTPORT_NUM;i++){
                inputport  = &dev->channel[j].inputport[i];
                if(UAC_INPUTPORT_INIT==inputport->status ||
                   UAC_INPUTPORT_DISABLED==inputport->status)
                {
                    mi_sys_EnableChannel(dev->devhandle,dev->channel[j].chnId);
                    mi_sys_EnableInputPort(dev->devhandle,
                                    dev->channel[j].chnId,
                                    inputport->inputport.u32PortId);
                    inputport->status = UAC_INPUTPORT_ENABLED;
                }
            }
        }
    }
/* playback channel */
    if(UAC_PLAYBACK_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_OUTPUTPORT_NUM;i++){
                outputport = &dev->channel[j].outputport[i];
                if(UAC_OUTPUTPORT_INIT==outputport->status ||
                   UAC_OUTPUTPORT_DISABLED==outputport->status)
                {
                    mi_sys_EnableChannel(dev->devhandle,dev->channel[j].chnId);
                    mi_sys_EnableOutputPort(dev->devhandle,
                                    dev->channel[j].chnId,
                                    outputport->outputport.u32PortId);
                    outputport->status = UAC_OUTPUTPORT_ENABLED;
                }
            }
        }
    }
done:
    dev->status = UAC_DEVICE_START;
    ret = MI_SUCCESS;
exit_mtx:
   mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

MI_S32 MI_UAC_IMPL_StopDev(MI_UAC_DEV DevId)
{
    MI_S32 i = 0,j = 0;
    MI_S32 ret = -1;
    uac_device_t *dev=NULL;
    uac_inputport_t *inputport=NULL;
    uac_outputport_t *outputport=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("Uac StopDev %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    mutex_lock(&dev->mtx);
    if(UAC_DEVICE_UNINIT==dev->status){
        DBG_ERR("Device not open\n");
        ret=MI_UAC_ERR_DEV_NOT_OPEN;
        goto exit_mtx;
    }

    if(UAC_DEVICE_STOP == dev->status){
       DBG_INFO("Device is already stopped\n");
       goto done;
    }

/* capture channel */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_INPUTPORT_NUM;i++){
                inputport  = &dev->channel[j].inputport[i];
                if(UAC_INPUTPORT_ENABLED==inputport->status)
                {
                    mi_sys_DisableChannel(dev->devhandle,dev->channel[j].chnId);
                    mi_sys_DisableInputPort(dev->devhandle,
                                    dev->channel[j].chnId,
                                    inputport->inputport.u32PortId);
                    inputport->status = UAC_INPUTPORT_DISABLED;
                }
            }
        }
    }
/* playback channel */
    if(UAC_CAPTURE_DEV==DevId){
        for(j=0;j<UAC_TOTAL_CHANNEL_NUM;j++){
            for(i=0;i<UAC_TOTAL_OUTPUTPORT_NUM;i++){
                outputport = &dev->channel[j].outputport[i];
                if(UAC_OUTPUTPORT_ENABLED==outputport->status)
                {
                    mi_sys_DisableChannel(dev->devhandle,dev->channel[j].chnId);
                    mi_sys_DisableOutputPort(dev->devhandle,
                                    dev->channel[j].chnId,
                                    outputport->outputport.u32PortId);
                    outputport->status = UAC_OUTPUTPORT_DISABLED;
                }
            }
        }
    }
done:
    dev->status = UAC_DEVICE_STOP;
    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

s32 mi_uac_playback_hw_params(struct snd_pcm_hw_params *hw_params)
{
    MI_S32 ret    = -1;
    MI_S32 DevId  = UAC_PLAYBACK_DEV;
    MI_S32 ChnId  = 0;
    MI_S32 PortId = 0;
    uac_device_t  *dev=NULL;
    uac_outputport_t *outputport=NULL;
    MI_UAC_OutputPortAttr_t *attr=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("mi_uac_playback_hw_params %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    outputport = &dev->channel[ChnId].outputport[PortId];
    mutex_lock(&dev->mtx);
    if(dev->status <= UAC_DEVICE_UNINIT){
        DBG_ERR("Device is not Opened\n");
        ret=MI_UAC_ERR_DEV_OPENED;
        goto exit_mtx;
    }
    {
        attr = &outputport->attr;
        attr->access   = params_access(hw_params);
        attr->format   = params_format(hw_params);
        attr->rate     = params_rate(hw_params);
        attr->channels = params_channels(hw_params);
#if UAC_IMPL_DBG
    DBG_INFO("params_access   = %d\n", params_access(hw_params));
    DBG_INFO("params_format   = %d\n", params_format(hw_params));
    DBG_INFO("params_rate     = %d\n", params_rate(hw_params));
    DBG_INFO("params_channels = %d\n", params_channels(hw_params));
#endif
    }
    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

s32 mi_uac_capture_hw_params(struct snd_pcm_hw_params *hw_params)
{
    MI_S32 ret    = -1;
    MI_S32 DevId  = UAC_CAPTURE_DEV;
    MI_S32 ChnId  = 0;
    MI_S32 PortId = 0;
    uac_device_t  *dev=NULL;
    uac_inputport_t *inputport=NULL;
    MI_UAC_InputPortAttr_t *attr=NULL;
#if UAC_IMPL_DBG
    DBG_INFO("mi_uac_playback_hw_params %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        DBG_ERR("Invalid Device Id\n");
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        DBG_ERR("Module not inited\n");
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    inputport = &dev->channel[ChnId].inputport[PortId];
    mutex_lock(&dev->mtx);
    if(dev->status <= UAC_DEVICE_UNINIT){
        DBG_ERR("Device is not Opened\n");
        ret=MI_UAC_ERR_DEV_OPENED;
        goto exit_mtx;
    }
    {
        attr = &inputport->attr;
        attr->access   = params_access(hw_params);
        attr->format   = params_format(hw_params);
        attr->rate     = params_rate(hw_params);
        attr->channels = params_channels(hw_params);
#if UAC_IMPL_DBG
    DBG_INFO("params_access   = %d\n", params_access(hw_params));
    DBG_INFO("params_format   = %d\n", params_format(hw_params));
    DBG_INFO("params_rate     = %d\n", params_rate(hw_params));
    DBG_INFO("params_channels = %d\n", params_channels(hw_params));
#endif
    }
    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
    return ret;
}

s32 mi_uac_capture(void *from,u32 length)
{
    MI_S32 ret    = -1;
    MI_S32 DevId  = UAC_CAPTURE_DEV;
    MI_S32 ChnId  = 0;
    MI_S32 PortId = 0;
    uac_device_t  *dev=NULL;
    uac_inputport_t *inputport=NULL;
    MI_SYS_BufInfo_t *bufinfo=NULL;
    MI_U64 buf_map_flags = MI_SYS_MAP_VA;
#if UAC_IMPL_DBG
    DBG_INFO("Uac mi_uac_capture %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    inputport = &dev->channel[ChnId].inputport[PortId];
    mutex_lock(&dev->mtx);
    if(UAC_DEVICE_START!=dev->status){
        ret=MI_UAC_ERR_DEV_NOT_OPEN;
        goto exit_mtx;
    }
    if(UAC_INPUTPORT_ENABLED!=inputport->status){
        ret=MI_UAC_ERR_PORT_NOT_DISABLE;
        goto exit_mtx;
    } else
    {

        /*
        No need set buf_map_flags MI_SYS_MAP_CPU_READ.
        Since the buf is clean & invalid.
        */
        bufinfo = mi_sys_GetInputPortBuf(dev->devhandle,ChnId,PortId,buf_map_flags);
        if(NULL!=bufinfo)
        {
#if UAC_IMPL_DBG
            DBG_INFO("%s getlen %d needlen %d\n",__func__,bufinfo->stRawData.u32BufSize,length);
#endif
            if(bufinfo->stRawData.u32ContentSize == length)
                memcpy(from,bufinfo->stRawData.pVirAddr,length);
            mi_sys_FinishBuf(bufinfo);
        } else
        {
            ret=MI_UAC_ERR_PORT_NOT_DISABLE;
            goto exit_mtx;
        }
    }

    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
#ifdef UAC_IMPL_DBG
    DBG_INFO("%s return value %d\n",__func__,ret);
#endif
    return ret;
}

s32 mi_uac_playback(void *to,u32 length)
{
    MI_S32 ret    = -1;
    MI_S32 DevId  = UAC_PLAYBACK_DEV;
    MI_S32 ChnId  = 0;
    MI_S32 PortId = 0;
    uac_device_t  *dev=NULL;
    uac_outputport_t *outputport=NULL;
    MI_SYS_BufInfo_t *bufinfo=NULL;
    MI_SYS_BufConf_t  Buf_config;

    memset(&Buf_config,0x00,sizeof(MI_SYS_BufConf_t));

#if UAC_IMPL_DBG
    DBG_INFO("Uac mi_uac_playback %d\n", DevId);
#endif
    if(!UAC_VALID_DEVID(DevId)){
        ret = MI_UAC_ERR_INVALID_DEVID;
        goto exit;
    }
    down(&module_sem);
    if(!_uac_module.binited){
        ret=MI_UAC_ERR_MOD_NOT_INIT;
        goto exit;
    }
    dev=&(_uac_module.dev[DevId]);
    outputport = &dev->channel[ChnId].outputport[PortId];
    mutex_lock(&dev->mtx);
    if(UAC_DEVICE_START!=dev->status){
        ret=MI_UAC_ERR_DEV_NOT_OPEN;
        goto exit_mtx;
    }
    if( UAC_OUTPUTPORT_ENABLED!=outputport->status)
    {
        ret=MI_UAC_ERR_PORT_NOT_DISABLE;
        goto exit_mtx;
    } else
    {
        Buf_config.eBufType         = E_MI_SYS_BUFDATA_RAW;
        Buf_config.u64TargetPts     = 100;
        Buf_config.stRawCfg.u32Size = length;
        Buf_config.u32Flags         = MI_SYS_MAP_VA;

        /*
        Flag : only MI_SYS_MAP_VA
        No need to set MI_SYS_MAP_CPU_READ & MI_SYS_MAP_CPU_WRITE flag.
        Since output is picked by cpu(user).
        */

#ifdef UAC_IMPL_DBG
        DBG_INFO("%s Put a Buf to Mi_Sys length %d\n",__func__,length);
#endif
        bufinfo = mi_sys_GetOutputPortBuf(dev->devhandle,ChnId,0,&Buf_config,FALSE);
        if(NULL == bufinfo){
              DBG_WRN("Get Output port buffer of Channel %d From Sys fail \n", ChnId);
        } else {
              memcpy(bufinfo->stRawData.pVirAddr,to,length);
              mi_sys_FinishBuf(bufinfo);
        }
    }
    ret = MI_SUCCESS;
exit_mtx:
    mutex_unlock(&dev->mtx);
exit:
    up(&module_sem);
#ifdef UAC_IMPL_DBG
    DBG_INFO("%s return value %d\n",__func__,ret);
#endif
    return ret;
}
