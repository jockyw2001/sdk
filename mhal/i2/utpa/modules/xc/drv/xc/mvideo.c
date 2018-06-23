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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// file    Mvideo.c
/// @brief  Main API in XC library
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define  MVIDEO_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#else
#include <string.h>
#include <stdlib.h>
#endif
//#include <assert.h>

#include "UFO.h"

#include "MsCommon.h"
#include "MsVersion.h"
#include "MsOS.h"
#include "mhal_xc_chip_config.h"
#include "utopia.h"
#include "utopia_dapi.h"

// Internal Definition
#include "drvXC_IOPort.h"
#include "apiXC.h"
#include "apiXC_DWIN.h"
#include "apiXC_Adc.h"
#include "apiXC_Auto.h"
#include "drv_sc_display.h"
#include "drv_sc_isr.h"
#include "apiXC_PCMonitor.h"
#include "apiXC_ModeParse.h"
#include "drvXC_HDMI_if.h"
#include "mvideo_context.h"
#include "drv_sc_ip.h"
#if (LD_ENABLE==1)
#include "mdrv_ld.h"
#include "mdrv_ldalgo.h"
#endif
#include "mdrv_sc_3d.h"
#include "drv_sc_menuload.h"
#include "drvXC_ADC_Internal.h"
#include "mdrv_frc.h"
#include "mhal_frc.h"
#include "mhal_sc.h"
#include "mhal_dynamicscaling.h"
#include "XC_private.h"
#include "apiXC_v2.h"
#include "drvXC_HDMI_Internal.h"

#include "xc_Analog_Reg.h"
#include "hwreg_ipmux.h"

#include "drv_sc_scaling.h"
#include "drv_sc_mux.h"
#include "drv_sc_DIP_scaling.h"
#include "mdrv_sc_dynamicscaling.h"
#include "drvscaler_nr.h"
#include "drvMMIO.h"
#include "mhal_pip.h"

#include "drvPQ.h"
//Add for A5
#include "mhal_dip.h"

// Tobe refined
#include "xc_hwreg_utility2.h"
#include "mhal_mux.h"
#include "mhal_hdmi.h"
#include "mhal_ip.h"
#include "mhal_sc.h"
#include "mhal_menuload.h"

#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
#ifdef MSOS_TYPE_LINUX_KERNEL
#include "drvCMAPool_v2.h"
#else
#include "drvCMAPool.h"
#endif
#include "msos/linux/mdrv_cma_pool_st.h"
#include "halCHIP.h"
#endif
#endif
#include "drvSYS.h"

//mvop for xc all mvop API
#include "drvMVOP.h"
#include "tee_client_api.h"
#include "drvMIU.h"

#ifdef STELLAR
#include "apiXC_cus.h"
#include "mhal_s_disp_ctl.h"
#endif

#if (defined MSOS_TYPE_LINUX) || (defined ANDROID)
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h> // O_RDWR
#include <poll.h>
#endif

#include "mdrv_xc_io.h"
#include "mdrv_xc_st.h"
/*
    XC_INITDATA_VERSION                      current version : 1    //1: Add an field eScartIDPort_Sel for port selection
*/


//PQ_Function_Info    s_PQ_Function_Info = {0, 0, 0, 0, 0, 0, 0, 0, 0};
PQ_Function_Info    s_PQ_Function_Info[2];//MAX_XC_DEVICE_NUM // = {{0, 0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0, 0}};

MS_U8 MApi_Pnl_Get_LPLL_Type(void);
MS_BOOL MApi_PNL_Get_TCON_Capability(void);
//void MApi_PNL_CalExtLPLLSETbyDClk(MS_U32 ldHz);
#ifdef PATCH_TCON_BRING_UP
const char* MDrv_PNL_GetName(void);
#endif
#if (defined(MSOS_TYPE_LINUX_KERNEL) && defined(CONFIG_MSTAR_XC_HDR_SUPPORT))
extern MS_BOOL MDrv_XC_CFDControl(ST_KDRV_XC_CFD_CONTROL_INFO *pstKdrvCFDCtrlInfo);
#endif
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
#ifdef MSOS_TYPE_LINUX
static MS_S32   _s32FdScaler = -1;
#endif
#endif
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define VERIFY_MVIDEO_FPGA  0

#ifndef XC_HLINEARSCALING_VER
#define XC_HLINEARSCALING_VER 0
#endif

#ifndef ENABLE_DI_8LB_MODE
#define ENABLE_DI_8LB_MODE 0
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL
#define DELAY_FRAME_NUM 5
#endif

#ifdef MSOS_TYPE_OPTEE
#define MAX_OPTEE_ERROR_CNT 3
#endif

#if defined (__aarch64__)
#define VERSION_COMPATIBLE_CHECK(pstInput,stValid,copy_Length,version_arg,length_arg,current_version) do { \
    if(pstInput == NULL)\
        return E_APIXC_RET_FAIL;\
    /*new AP + old lib, only the length corresponding to old lib has meaning.*/\
    if((pstInput)->version_arg > (current_version))\
    {\
        /*We consider compatible operation from this version, so reject the info init when version invalid*/\
        printf("[%s: %d]: this old version XC lib has only length:%lu driver status!!\n",\
                     __FUNCTION__, __LINE__, sizeof(stValid));\
        /*we still copy the min size of both structure, but AP should take care of it.*/\
        (copy_Length) = sizeof(stValid);\
    }\
    \
    /*old AP + new lib, driver shouldn't access to the space which doesn't exist in old structure*/\
    if(((pstInput)->version_arg < (current_version)) || ((pstInput)->length_arg < sizeof(stValid)))\
    {\
        /*We consider compatible operation from this version, so reject the info init when version invalid*/\
        printf("[%s: %d]: new version XC lib shouldn't access to the space which doesn't exist in old structure!!\n",\
        __FUNCTION__, __LINE__);\
        (copy_Length) = (pstInput)->length_arg;\
    }\
}while(0)
#else
#define VERSION_COMPATIBLE_CHECK(pstInput,stValid,copy_Length,version_arg,length_arg,current_version) do { \
    if(pstInput == NULL)\
        return E_APIXC_RET_FAIL;\
    /*new AP + old lib, only the length corresponding to old lib has meaning.*/\
    if((pstInput)->version_arg > (current_version))\
    {\
        /*We consider compatible operation from this version, so reject the info init when version invalid*/\
        printf("[%s: %d]: this old version XC lib has only length:%u driver status!!\n",\
                     __FUNCTION__, __LINE__, sizeof(stValid));\
        /*we still copy the min size of both structure, but AP should take care of it.*/\
        (copy_Length) = sizeof(stValid);\
    }\
    \
    /*old AP + new lib, driver shouldn't access to the space which doesn't exist in old structure*/\
    if(((pstInput)->version_arg < (current_version)) || ((pstInput)->length_arg < sizeof(stValid)))\
    {\
        /*We consider compatible operation from this version, so reject the info init when version invalid*/\
        printf("[%s: %d]: new version XC lib shouldn't access to the space which doesn't exist in old structure!!\n",\
        __FUNCTION__, __LINE__);\
        (copy_Length) = (pstInput)->length_arg;\
    }\
}while(0)
#endif
//#define SUPPORT_HDMI20_420
//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MSIF_Version _api_xc_version = {
    .DDI = { XC_API_VERSION },
};

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 u32SetWindowTime;
#endif

extern void MApi_XC_ACE_GetColorMatrix_U2(void *pInstance, MS_BOOL eWindow, MS_U16* pu16Matrix);
extern void MApi_XC_ACE_SetColorMatrix_U2( void *pInstance, MS_BOOL eWindow, MS_U16* pu16Matrix);

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

E_APIXC_ReturnValue MApi_XC_Enable_LockFreqOnly_U2(void* pInstance,MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;

    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[E_XC_POOL_ID_INTERNAL_VARIABLE],(void**)(&pXCResourcePrivate));

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_POOL_ID_INTERNAL_VARIABLE);
    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bEnableLockFreqOnly = bEnable;
    bRet = TRUE;
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_POOL_ID_INTERNAL_VARIABLE);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}

E_APIXC_ReturnValue MApi_XC_Enable_LockFreqOnly(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Enable_LockFreqOnly XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_ENABLE_LOCKFREQONLY, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }

}

#if(XC_CONTEXT_ENABLE)
MS_U8 _XC_Client[] = {"Linux XC driver"};

static MS_BOOL _MApi_XC_Init_Context(void *pInstance, MS_BOOL *pbFirstDrvInstant)
{
    MS_BOOL bRet = TRUE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    *pbFirstDrvInstant = pXCResourcePrivate->stdrvXC_MVideo.bNeedInitShared;

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return bRet;
}
#endif

//-------------------------------------------------------------------------------------------------
/// Exit the XC
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_Exit_U2(void* pInstance)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    if (_XC_Mutex == -1)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return TRUE;
    }
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

#if ((HW_DESIGN_4K2K_VER == 6) && FRC_INSIDE)
        MS_U8 u8LPLL_Type =(MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type;

        if (IsVBY1_16LANE(u8LPLL_Type))
        {
            MHal_XC_PixelShiftStatusChange(pInstance, EN_XC_PIXEL_SHIFT_DISABLE);
        }
        // Maserati mode
        if ((u8LPLL_Type >= E_XC_PNL_LPLL_VBY1_10BIT_16LANE)&&(u8LPLL_Type <= E_XC_PNL_LPLL_VBY1_8BIT_16LANE))
        {
            // case 4: no FSC + FRC
            if(MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)
            {
                _MLOAD_ENTRY(pInstance);
            }
            else
            {
                printf("mload is not enable, please check !\n");
            }
            MHal_FRC_Set_Pipe_Delay_Reset(pInstance, DISABLE, TRUE);

            MHal_FRC_Set_TimingStable(pInstance, TRUE);
            MHal_FRC_ByPass_Enable(pInstance, FALSE);  // enable FRC
            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_1, TRUE);

            MHal_FRC_Set_Composer_User_Mode(pInstance, E_FRC_COMPOSER_SOURCE_MODE_VIP, TRUE);
            MHal_FRC_Set_FSC_DE_Selection(pInstance, E_FRC_FSC_SOURCE_MODE_MDE);
            MHal_FRC_TGEN_Enable_Lock_Source(pInstance, DISABLE, E_FRC_TGEN_LOCK_SOURCE_MODE_FROM_TGEN, TRUE);
            MDrv_XC_MLoad_Fire(pInstance, TRUE);
            if(MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)
            {
                _MLOAD_RETURN(pInstance);
            }
            else
            {
                printf("mload is not enable, please check !\n");
            }
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = TRUE;
        }
        else
        {
            // For Manhattan mode
            // no FSC+ no FRC
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
            if(MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)
            {
                _MLOAD_ENTRY(pInstance);
            }
            else
            {
                printf("mload is not enable, please check !\n");
            }
            MHal_FRC_Set_Pipe_Delay_Reset(pInstance, DISABLE, TRUE);
            MHal_FRC_AdjustGOPPosition(pInstance);
            MHal_FRC_ByPass_Enable(pInstance, TRUE); // disable FRC
            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_1, TRUE);

            MHal_FRC_Set_Composer_User_Mode(pInstance, E_FRC_COMPOSER_SOURCE_MODE_VIP, TRUE);
            MHal_FRC_Set_FSC_DE_Selection(pInstance, E_FRC_FSC_SOURCE_MODE_MDE);
            MHal_FRC_TGEN_Enable_Lock_Source(pInstance, DISABLE, E_FRC_TGEN_LOCK_SOURCE_MODE_FROM_TGEN, TRUE);
            MDrv_XC_MLoad_Fire(pInstance, TRUE);
            if(MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)
            {
                _MLOAD_RETURN(pInstance);
            }
            else
            {
                printf("mload is not enable, please check !\n");
            }
        }
#endif
    _XC_ENTRY(pInstance);
#ifdef K3_U2
    if(psXCInstPri->u32DeviceID == E_XC_DEVICE0)
    {
        if(pXCResourcePrivate->stdrvXC_MVideo._stXC_ApiInfo.u8MaxWindowNum > 1)
        {
            // DISABLE FIClk1
            MDrv_WriteRegBit(REG_CKG_FICLK_F1, ENABLE, CKG_FICLK_F1_GATED);                // Enable clock

            // DISABLE IDClk1
            MDrv_WriteRegBit(REG_CKG_IDCLK1, ENABLE, CKG_IDCLK1_GATED);

            // DISABLE SIDClk1
            MDrv_WriteRegBit(REG_CKG_SIDCLK1, ENABLE, CKG_SIDCLK1_GATED);
        }

        // DISABLE FIClk2
        MDrv_WriteRegBit(REG_CKG_FICLK_F2, ENABLE, CKG_FICLK_F2_GATED);                    // Enable clock

        // DISABLE FClk
        MDrv_WriteRegBit(REG_CKG_FCLK, ENABLE, CKG_FCLK_GATED);                            // Enable clock

        // DISABLE IDClk2
        MDrv_WriteRegBit(REG_CKG_IDCLK2, ENABLE, CKG_IDCLK2_GATED);                        // Enable clock

        // DISABLE SIDClk2
        MDrv_WriteRegBit(REG_CKG_SIDCLK2, ENABLE, CKG_SIDCLK2_GATED);
    }
    else // pDeviceId->u32Id == E_XC_DEVICE1
    {
        // DISABLE FIClk2
        MDrv_WriteRegBit(REG_CKG_SC1_FECLK_F2, ENABLE, CKG_SC1_FECLK_F2_GATED);                    // Enable clock

        // DISABLE FClk
        MDrv_WriteRegBit(REG_CKG_SC1_FCLK, ENABLE, CKG_SC1_FCLK_GATED);                            // Enable clock

        // DISABLE IDClk2
        MDrv_WriteRegBit(REG_CKG_SC1_IDCLK2, ENABLE, CKG_SC1_IDCLK2_GATED);                        // Enable clock

        // DISABLE SIDClk2
        MDrv_WriteRegBit(REG_CKG_SC1_SIDCLK2, ENABLE, CKG_SC1_SIDCLK2_GATED);
    }
#else

#if(HW_DESIGN_4K2K_VER == 4)
    if(psXCInstPri->u32DeviceID == E_XC_DEVICE0)
    {
        if(pXCResourcePrivate->stdrvXC_MVideo._stXC_ApiInfo.u8MaxWindowNum > 1)
        {
            // DISABLE FIClk1
            MDrv_WriteRegBit(REG_CKG_FICLK_F1, ENABLE, CKG_FICLK_F1_GATED);                // Enable clock

            // DISABLE IDClk1
            MDrv_WriteRegBit(REG_CKG_IDCLK1, ENABLE, CKG_IDCLK1_GATED);
        }

            // DISABLE EDCLK_F2
        Hal_SC_set_edclk(pInstance, 0x00, DISABLE, MAIN_WINDOW);
            // DISABLE EDCLK_F1
        Hal_SC_set_edclk(pInstance, 0x00, DISABLE, SUB_WINDOW);

        // DISABLE FICLK2_F2
        Hal_SC_set_ficlk2(pInstance, 0x00, DISABLE, MAIN_WINDOW);

        // DISABLE FIClk_F2
        MDrv_WriteRegBit(REG_CKG_FICLK_F2, ENABLE, CKG_FICLK_F2_GATED);                    // Enable clock

        // DISABLE FClk
        MDrv_WriteRegBit(REG_CKG_FCLK, ENABLE, CKG_FCLK_GATED);                            // Enable clock

        // DISABLE IDClk2
        MDrv_WriteRegBit(REG_CKG_IDCLK2, ENABLE, CKG_IDCLK2_GATED);                        // Enable clock
    }
    else
    {
#ifdef MONACO_SC2_PATCH
        // DISABLE FIClk_F2
        MDrv_WriteRegBit(REG_CKG_S2_FICLK2_F2, ENABLE, CKG_FICLK2_F2_GATED);                    // Enable clock
#endif
        // DISABLE FClk
        MDrv_WriteRegBit(REG_CKG_S2_FCLK, ENABLE, CKG_S2_FCLK_GATED);                            // Enable clock

        // DISABLE IDClk2
        MDrv_WriteRegBit(REG_CKG_S2_IDCLK2, ENABLE, CKG_S2_IDCLK2_GATED);                        // Enable clock

    }
#else
    if(pXCResourcePrivate->stdrvXC_MVideo._stXC_ApiInfo.u8MaxWindowNum > 1)
    {
        // DISABLE FIClk1
        MDrv_WriteRegBit(REG_CKG_FICLK_F1, ENABLE, CKG_FICLK_F1_GATED);                // Enable clock

        // DISABLE IDClk1
        MDrv_WriteRegBit(REG_CKG_IDCLK1, ENABLE, CKG_IDCLK1_GATED);
    }

        // DISABLE EDCLK_F2
    Hal_SC_set_edclk(pInstance, 0x00, DISABLE, MAIN_WINDOW);
        // DISABLE EDCLK_F1
    Hal_SC_set_edclk(pInstance, 0x00, DISABLE, SUB_WINDOW);

    // DISABLE FICLK2_F2
    Hal_SC_set_ficlk2(pInstance, 0x00, DISABLE, MAIN_WINDOW);

    // DISABLE FIClk_F2
    MDrv_WriteRegBit(REG_CKG_FICLK_F2, ENABLE, CKG_FICLK_F2_GATED);                    // Enable clock

    // DISABLE FClk
    MDrv_WriteRegBit(REG_CKG_FCLK, ENABLE, CKG_FCLK_GATED);                            // Enable clock

    // DISABLE IDClk2
    MDrv_WriteRegBit(REG_CKG_IDCLK2, ENABLE, CKG_IDCLK2_GATED);                        // Enable clock
#endif
#endif
    _XC_RETURN(pInstance);

//DISABLE Interrupt
#ifndef MSOS_TYPE_LINUX_KERNEL
    if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
    {
        MsOS_DisableInterrupt(E_INT_IRQ_DISP);
    }
    else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
    {
        MsOS_DisableInterrupt(E_INT_IRQ_EC_BRIDGE);
        MsOS_DisableInterrupt(E_INT_IRQ_DISP1);
    }
#endif

    // Release ISR
#ifdef MSOS_TYPE_LINUX_KERNEL
    if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
    {
        MsOS_DetachInterrupt_Shared(E_INT_IRQ_DISP);
    }
    else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
    {
        MsOS_DetachInterrupt_Shared(E_INT_IRQ_EC_BRIDGE);
        MsOS_DetachInterrupt_Shared(E_INT_IRQ_DISP1);
    }
#else
    if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
    {
        MsOS_DetachInterrupt(E_INT_IRQ_DISP);
    }
    else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
    {
        MsOS_DetachInterrupt(E_INT_IRQ_EC_BRIDGE);
        MsOS_DetachInterrupt(E_INT_IRQ_DISP1);
    }
#endif

    // Clear internal variable
    mvideo_sc_variable_init(pInstance, TRUE, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData);

    _XC_ENTRY(pInstance);
    memset(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, 0 , sizeof(XC_INITDATA) );

    #if 0 //(FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    MHal_XC_FRCR2_IP_PatchClose(pInstance, MAIN_WINDOW);
    MHal_XC_FRCR2_OP_PatchClose(pInstance, MAIN_WINDOW);
    #endif

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // For XC re-init
    MsOS_DeleteMutex(_XC_Mutex);
    _XC_Mutex = -1;

    MsOS_DeleteMutex(_XC_ISR_Mutex);
    _XC_ISR_Mutex = -1;
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
#if (HW_DESIGN_4K2K_VER == 7)
    if(psXCInstPri->u32DeviceID == 0)
#endif
    {
        if ((pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable == TRUE) ||
            (pXCResourcePrivate->sthal_Optee.op_tee_xc[SUB_WINDOW].isEnable == TRUE))
        {
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
            return TRUE;
        }
        MHal_XC_Release_CMA(pInstance, CMA_XC_SELF_MEM, MAIN_WINDOW);
        MApi_CMA_Pool_Release(gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u32HeapID);
        if (gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID != XC_INVALID_HEAP_ID)
        {
            MHal_XC_Release_CMA(pInstance, CMA_XC_COBUFF_MEM, MAIN_WINDOW);
            MApi_CMA_Pool_Release(gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID);
        }
    }
#endif
#endif
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return TRUE;

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Exit(void)
{

    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_EXIT XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_EXIT, (void*)&XCArgs) != 0)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
MS_BOOL MDrv_XC_IsPNLYUVOutput(void *pInstance)
{
    //Pnl output is affected by input signal, so change to xc driver to report it as below
    return MHal_XC_IsPNLYUVOutput(pInstance);
}

MS_BOOL MDrv_XC_Is2K2KToFrc(void *pInstance)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
#if FRC_INSIDE
    // For Einstein FRC 4K2K case, for non-4K2K input, need to sent 2Kx2K to FRC
    if ((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultHTotal> 3000) &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultVTotal> 2000) &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultVFreq > 500) &&  // 4K2K 60 out
        (pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_2K1K))
    {
        bRet = TRUE;
    }
#endif
    return bRet;
}

MS_BOOL MDrv_XC_SetIOMapBase_i(void* pInstance)
{
    MS_VIRT _XCRIUBaseAddress = 0, _PMRIUBaseAddress = 0, _XCEXTRIUBaseAddress = 0;
    MS_PHY u32NonPMBankSize = 0, u32PMBankSize = 0;
    #ifdef SUPPORT_XC_EXT_REG
    MS_PHY u32EXTBankSize = 0;
    #endif
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");


    if(MDrv_MMIO_GetBASE( &_XCRIUBaseAddress, &u32NonPMBankSize, MS_MODULE_XC ) != TRUE)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init GetBASE failure\n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return FALSE;
    }
    else
    {
        //printf("MApi_XC_Init GetBase success %lx\n", _XCRIUBaseAddress);
    }

    if(MDrv_MMIO_GetBASE( &_PMRIUBaseAddress, &u32PMBankSize, MS_MODULE_PM ) != TRUE)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init Get PM BASE failure\n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return FALSE;
    }
    else
    {
        //printf("MApi_XC_Init GetPMBase success %lx\n", _PMRIUBaseAddress);
    }

    #ifdef SUPPORT_XC_EXT_REG
    if(MDrv_MMIO_GetBASE( &_XCEXTRIUBaseAddress, &u32EXTBankSize, MS_MODULE_EXTRIU ) != TRUE)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init Get EXT BASE failure\n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        //return FALSE;
    }
    else
    {
        //printf("MApi_XC_Init GetPMBase success %lx\n", _PMRIUBaseAddress);
    }
    #endif

    //printf("XC RIU base=%lx\n", _XCRIUBaseAddress);
    MDrv_XC_init_riu_base(_XCRIUBaseAddress, _PMRIUBaseAddress, _XCEXTRIUBaseAddress);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return TRUE;
}

//This API only designed for DFB calling.
//Supernova involved xc lib only by initializing this function.
//So we need to create mutext in this API.
//it's useless for new dfb-xc design
MS_BOOL MDrv_XC_SetIOMapBase_U2(void* pInstance)
{
    MS_BOOL bReturn = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo._bDFBInit = TRUE; //This flag is to compatible old dfb version.
                      //new dfb will not call this func,
                      //instead of set u32InitDataLen=0 of MApi_XC_Init()
    printf("MDrv_XC_SetIOMapBase(), do nothing, it's assumed DFB init case!\n");
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MDrv_XC_SetIOMapBase(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_CMD_SET_IOMAPBASE XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_IOMAPBASE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
void MApi_XC_PreInit_FRC(void *pInstance, XC_PREINIT_INFO_t *pPanelInfo)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    ///patch
    ///g_XC_Pnl_Misc.FRCInfo.u8FRC3DPanelType = E_XC_3D_PANEL_SHUTTER;
    memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc, pPanelInfo, sizeof(XC_PREINIT_INFO_t));
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

E_APIXC_ReturnValue MApi_XC_PreInit_U2( void* pInstance, E_XC_PREINIT_t eType ,void* para, MS_U32 u32Length)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    XC_PREINIT_INFO_t* stFRCInfo = NULL;

    switch(eType)
    {
        case E_XC_PREINIT_FRC:
            if(u32Length != sizeof(XC_PREINIT_INFO_t))
            {
                printf("Error size of parameters\n");
                return E_APIXC_RET_FAIL;
            }
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            MApi_XC_PreInit_FRC(pInstance, (XC_PREINIT_INFO_t *)para);
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        break;
        case E_XC_PREINIT_FRC_3D_TYPE:
            if(u32Length != sizeof(XC_PREINIT_INFO_t))
            {
                printf("Error size of parameters\n");
                return E_APIXC_RET_FAIL;
            }
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
            stFRCInfo = (XC_PREINIT_INFO_t *)para;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8FRC3DPanelType = stFRCInfo->FRCInfo.u8FRC3DPanelType;
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        break;
        case E_XC_PREINIT_FRC_ADDRESS:
            if(u32Length != sizeof(XC_FRC_ADDRESS_INFO))
            {
                printf("Error size of parameters\n");
                return E_APIXC_RET_FAIL;
            }
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            MDrv_FRC_SetMemoryAddress(pInstance, (XC_FRC_ADDRESS_INFO *)para);
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        break;
        case E_XC_PREINIT_HDMITX_COLORDEPTH:
            #if (HW_DESIGN_HDMITX_VER == 2)
            if(u32Length != sizeof(EN_XC_HDMITX_COLORDEPTH))
            {
                printf("Error size of parameters\n");
                return E_APIXC_RET_FAIL;
            }
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
            pXCResourcePrivate->stdrvXC_MVideo_Context.enHDMITXColorDepth = *(EN_XC_HDMITX_COLORDEPTH *)para;
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            #else
            printf("Not support assign color depth for HDMI Tx \n");
            return E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
            #endif
        break;
        default:
        case E_XC_PREINIT_NULL:

        break;
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_PreInit( E_XC_PREINIT_t eType ,void* para, MS_U32 u32Length)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);
        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_PREINIT XCArgs;
    XCArgs.eType = eType;
    XCArgs.para = para;
    XCArgs.u32Length = u32Length;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_PREINIT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
static MS_BOOL _MApi_XC_Init_WithoutCreateMutex(void *pInstance, XC_INITDATA *pXC_InitData, MS_U32 u32InitDataLen)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange = 0;
    if(pXCResourcePrivate->stdrvXC_MVideo._bSkipSWReset == FALSE)
    {
        _XC_ENTRY(pInstance);
        MDrv_SC_SetSkipWaitVsync(pInstance, MAIN_WINDOW, TRUE);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_SC_SetSkipWaitVsync(pInstance, SUB_WINDOW , TRUE);
#endif
        MDrv_XC_SoftwareReset(pInstance, REST_IP_ALL, MAIN_WINDOW);

        // adc init
        MDrv_ADC_init(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32XTAL_Clock/1000UL
                      ,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.bIsShareGround
                      ,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.eScartIDPort_Sel);

        //mvop init
        //MBoot can not init MVOP here(MBoot does not register MVOP)
#if (!(defined(CONFIG_MBOOT)) && !(defined(MSOS_TYPE_OPTEE)) && !(defined(CHIP_I2)))
        MDrv_MVOP_Init();
#endif

#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        Hal_SC_enable_window(pInstance, FALSE,SUB_WINDOW);
#endif

        MDrv_XC_DisableInputSource(pInstance, ENABLE, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_DisableInputSource(pInstance, ENABLE, SUB_WINDOW);
#endif
#if 0
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
        MS_BOOL ret= FALSE;

        if (
#if (HW_DESIGN_4K2K_VER == 7)
            (psXCInstPri->u32DeviceID == 0) &&
#endif
            pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState != E_POWER_SUSPEND)
        {
#if (HW_DESIGN_4K2K_VER == 6)
            gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u32HeapID = ION_XC_HEAP_ID;
#if defined(CHIP_MONET)
            gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID = XC_DUAL_MIU_HID;
#else
            gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID = XC_INVALID_HEAP_ID;
#endif
            gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u64AddrHeapOffset = 0;
            gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u64AddrHeapOffset = 0;
#endif
            /* 1. do CMA_POOL_INIT */
            CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_id     = gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u32HeapID;
            CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].flags       = CMA_FLAG_MAP_VMA;
            MsOS_DelayTaskUs(1);
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], CMA self heapid = %d!\n",__func__, __LINE__,CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_id);

            ret = MApi_CMA_Pool_Init(&CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM]);

            if(ret == FALSE)
            {
                printf("\033[35m   Function = %s, Line = %d, CMA_POOL_INIT ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
                while(1);
                ////assert(0);
            }
            else
            {
                //printf("\033[35m  Function = %s, Line = %d, get pool_handle_id is %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].pool_handle_id);
                //printf("\033[35m  Function = %s, Line = %d, get miu is %u\033[m\n", __PRETTY_FUNCTION__,__LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].miu);
                //printf("\033[35m  Function = %s, Line = %d, get heap_miu_start_offset is 0x%lx\033[m\n", __PRETTY_FUNCTION__, __LINE__, (MS_U32)CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_miu_start_offset);
                //printf("\033[35m  Function = %s, Line = %d, get heap_length is 0x%X\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_length);
            }

            if (gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID != XC_INVALID_HEAP_ID)
            {
                /* 1. do CMA_POOL_INIT */
                CMA_Pool_Init_PARAM[CMA_XC_COBUFF_MEM].heap_id     = gSrcInfo[eWindow].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID;
                CMA_Pool_Init_PARAM[CMA_XC_COBUFF_MEM].flags       = CMA_FLAG_MAP_VMA;
                MsOS_DelayTaskUs(1);
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], CMA cobuff heapid = %d!\n",__func__, __LINE__,CMA_Pool_Init_PARAM[CMA_XC_COBUFF_MEM].heap_id);
                ret = MApi_CMA_Pool_Init(&CMA_Pool_Init_PARAM[CMA_XC_COBUFF_MEM]);

                if(ret == FALSE)
                {
                    printf("\033[35m   Function = %s, Line = %d, CMA_POOL_INIT ERROR!!\033[m\n", __PRETTY_FUNCTION__, __LINE__);
                    while(1);
                    //assert(0);
                }
                else
                {
                    //printf("\033[35m  Function = %s, Line = %d, get pool_handle_id is %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].pool_handle_id);
                    //printf("\033[35m  Function = %s, Line = %d, get miu is %u\033[m\n", __PRETTY_FUNCTION__,__LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].miu);
                    //printf("\033[35m  Function = %s, Line = %d, get heap_miu_start_offset is 0x%lx\033[m\n", __PRETTY_FUNCTION__, __LINE__, (MS_U32)CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_miu_start_offset);
                    //printf("\033[35m  Function = %s, Line = %d, get heap_length is 0x%X\033[m\n", __PRETTY_FUNCTION__, __LINE__, CMA_Pool_Init_PARAM[CMA_XC_SELF_MEM].heap_length);
                }
            }
        }
#endif
#endif
#endif

        // default frame buffer address init
        MDrv_XC_SetFrameBufferAddress(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr,
                                      pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_SetFrameBufferAddress(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr,
                                      pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size, SUB_WINDOW);
#endif
        // default frcm frame buffer address init
        MDrv_XC_SetFRCMFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Size, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_SetFRCMFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Size, SUB_WINDOW);
#endif

#ifdef UFO_XC_SUPPORT_DUAL_MIU
        // default xc dual miu frame buffer address init
        MDrv_XC_SetDualFrameBufferAddress(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Dual_FB_Start_Addr,
                                      pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Dual_FB_Size, MAIN_WINDOW);
#endif
        MDrv_XC_init_fbn_win(pInstance, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_init_fbn_win(pInstance, SUB_WINDOW);
#endif

        if (pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState != E_POWER_RESUME)
        {
            // in MApi_XC_SetPowerState_U2 function will set mcdi,so here is not needed
            //HW initial settting of MCDi is turned on (the address of MCDi is 0).
            //Driver should turn it off before Fclk on, otherwise it will start write whole memory.
            MDrv_SC_EnableMCDI(pInstance, DISABLE,E_XC_MCDI_BOTH); // Main Window
            MDrv_SC_EnableMCDI(pInstance, DISABLE,E_XC_MCDI_SUB_BOTH); // Sub Window
        }
        if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.bEnableIPAutoCoast)
        {
            // enable IP auto coast
            MDrv_XC_EnableIPAutoCoast(pInstance, ENABLE);
        }

        /**
            * !!! Attention !!! The ISR interface is different from Chakra2 and Utopia.
            * In Chakra2, there are two para needed, MHAL_SavedRegisters and vector.
            * In Utopia, there is no parameter.
            */
        if((pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState == E_POWER_MECHANICAL)
#if 0
#ifndef MSOS_TYPE_LINUX_KERNEL
        || (pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState == E_POWER_SUSPEND)
#endif
#endif
        )
        {
#ifdef MSOS_TYPE_LINUX_KERNEL
            if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
            {
                MsOS_AttachInterrupt_Shared(E_INT_IRQ_DISP,(InterruptCb) MDrv_SC_isr);
            }
            else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
            {
                MsOS_AttachInterrupt_Shared(E_INT_IRQ_DISP1,(InterruptCb) MDrv_SC1_isr);
            }
#else
            if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
            {
                MsOS_AttachInterrupt(E_INT_IRQ_DISP,(InterruptCb) MDrv_SC_isr);
            }
            else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
            {
                MsOS_AttachInterrupt(E_INT_IRQ_DISP1,(InterruptCb) MDrv_SC1_isr);
            }
#endif
        }
#ifndef CHIP_I2
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            MsOS_EnableInterrupt(E_INT_IRQ_DISP);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_EnableInterrupt(E_INT_IRQ_DISP1);
        }
#endif

        // Enable XC interrupts
        //MDrv_WriteByte(BK_SELECT_00, REG_BANK_VOP);
        //MDrv_WriteRegBit(L_BK_VOP(0x25), ENABLE, BIT(7));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_25_L, 0x80, BIT(7));

        // Daten FixMe, move from panel init to here, need to check why
        //MDrv_WriteByte(BK_SELECT_00, REG_BANK_S_VOP);
        //MDrv_WriteRegBit(L_BK_S_VOP(0x56), 1, BIT(1));  //Using new ovs_ref
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK0F_56_L, 0x02, BIT(1));

        //MDrv_WriteByte(BK_SELECT_00, REG_BANK_IP1F2);
        //MDrv_WriteByteMask(L_BK_IP1F2(0x09), 0x02, BIT(3) | BIT(2) | BIT(1) | BIT(0));  //YLock
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK01_09_L, 0x02, BIT(3) | BIT(2) | BIT(1) | BIT(0));
        //Extend the pixel of bottom A session at the right side [7:0]
        //Extend the pixel of bottom B session at the left side[15:8] for PIP issue
        SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK20_14_L, 0x2020);

        //HSYNC Tolerance for Mode Change
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK01_1D_L, 0x02, 0x1F);

        //----------------------------
        // Enable clock
        //----------------------------

        // Enable sub window clock
#if 0
        if(_stXC_ApiInfo.u8MaxWindowNum > 1)
        {
            // Enable FIClk1
            MDrv_WriteByteMask(REG_CKG_FICLK_F1, CKG_FICLK_F1_FLK, CKG_FICLK_F1_MASK);      // select FClk first, if pre-scaling down -> change to CKG_FICLK_F1_IDCLK1
            MDrv_WriteRegBit(REG_CKG_FICLK_F1, DISABLE, CKG_FICLK_F1_INVERT);               // Not Invert
            MDrv_WriteRegBit(REG_CKG_FICLK_F1, DISABLE, CKG_FICLK_F1_GATED);                // Enable clock

            // Enable IDClk1
            MDrv_WriteByteMask(REG_CKG_IDCLK1, CKG_IDCLK1_XTAL, CKG_IDCLK1_MASK);           // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_IDCLK1, DISABLE, CKG_IDCLK1_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_IDCLK1, DISABLE, CKG_IDCLK1_GATED);                    // Enable clock
        }
#endif
#ifdef K3_U2
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            // Enable FIClk2
            MDrv_WriteByteMask(REG_CKG_FICLK_F2, CKG_FICLK_F2_FLK, CKG_FICLK_F2_MASK);          // select FClk first, if pre-scaling down -> change to CKG_FICLK_F1_IDCLK2
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_GATED);                    // Enable clock

            // Enable FClk
            MDrv_WriteByteMask(REG_CKG_FCLK, CKG_FCLK_170MHZ, CKG_FCLK_MASK);                   // select 170MHz, if not enough (for example, scaling bandwidth not enough), increase this
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_INVERT);                           // Not Invert
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_GATED);                            // Enable clock

            // Enable IDClk2
            MDrv_WriteByteMask(REG_CKG_IDCLK2, CKG_IDCLK2_XTAL, CKG_IDCLK2_MASK);               // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_GATED);                        // Enable clock

            // Enable SIDClk2
            MDrv_WriteRegBit(REG_CKG_SIDCLK2, DISABLE, CKG_SIDCLK2_INVERT);
            MDrv_WriteRegBit(REG_CKG_SIDCLK2, DISABLE, CKG_SIDCLK2_GATED);
        }
        else //E_XC_DEVICE1 == pDeviceId->u32Id
        {
            // Enable FIClk2
            MDrv_WriteByteMask(REG_CKG_SC1_FECLK_F2, CKG_SC1_FECLK_F2_SC1_FCLK, CKG_SC1_FECLK_F2_MASK);          // select FClk first, if pre-scaling down -> change to CKG_FICLK_F1_IDCLK2
            MDrv_WriteRegBit(REG_CKG_SC1_FECLK_F2, DISABLE, CKG_SC1_FECLK_F2_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_SC1_FECLK_F2, DISABLE, CKG_SC1_FECLK_F2_GATED);                    // Enable clock

            // Enable FClk
            MDrv_WriteByteMask(REG_CKG_SC1_FCLK, CKG_SC1_FCLK_123MHZ, CKG_SC1_FCLK_MASK);                   // select 170MHz, if not enough (for example, scaling bandwidth not enough), increase this
            MDrv_WriteRegBit(REG_CKG_SC1_FCLK, DISABLE, CKG_SC1_FCLK_INVERT);                           // Not Invert
            MDrv_WriteRegBit(REG_CKG_SC1_FCLK, DISABLE, CKG_SC1_FCLK_GATED);                            // Enable clock

            // Enable IDClk2
            MDrv_WriteByteMask(REG_CKG_SC1_IDCLK2, CKG_SC1_IDCLK2_XTAL, CKG_SC1_IDCLK2_MASK);               // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_SC1_IDCLK2, DISABLE, CKG_SC1_IDCLK2_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_SC1_IDCLK2, DISABLE, CKG_SC1_IDCLK2_GATED);                        // Enable clock
            MDrv_Write2ByteMask(REG_CKG_SC1_FECLK2_F2, CKG_SC1_FECLK2_F2_SC1_FCLK, CKG_SC1_FECLK2_F2_MASK);  // enable ficlk2_f2

            // Enable SIDClk2
            MDrv_WriteRegBit(REG_CKG_SC1_SIDCLK2, DISABLE, CKG_SC1_SIDCLK2_INVERT);
            MDrv_WriteRegBit(REG_CKG_SC1_SIDCLK2, DISABLE, CKG_SC1_SIDCLK2_GATED);
        }
#else

#if(HW_DESIGN_4K2K_VER == 7)
        if(psXCInstPri->u32DeviceID == E_XC_DEVICE0)
        {
            // Enable FIClk2
            MDrv_WriteByteMask(REG_CKG_FICLK_F2, CKG_FICLK_F2_FLK, CKG_FICLK_F2_MASK);          // select FClk first, if pre-scaling down -> change to CKG_FICLK_F1_IDCLK2
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_GATED);                    // Enable clock

            // Enable FClk
            MDrv_WriteByteMask(REG_CKG_FCLK, CKG_FCLK_DEFAULT, CKG_FCLK_MASK);                   // select 170MHz, if not enough (for example, scaling bandwidth not enough), increase this
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_INVERT);                           // Not Invert
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_GATED);                            // Enable clock

            // Enable IDClk2
            MDrv_WriteByteMask(REG_CKG_IDCLK2, CKG_IDCLK2_XTAL, CKG_IDCLK2_MASK);               // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_GATED);                        // Enable clock

            // Enable EDClk
            Hal_SC_set_edclk(pInstance, CKG_EDCLK_F2_345MHZ, ENABLE, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
            Hal_SC_set_edclk(pInstance, CKG_EDCLK_F1_345MHZ, ENABLE, SUB_WINDOW);
#endif

            // Enable FMClk
            Hal_SC_set_fmclk(psXCInstPri, ENABLE);

            // Enable FIClk2_f2
            Hal_SC_set_ficlk2(pInstance, CKG_FICLK2_F2_FCLK, ENABLE, MAIN_WINDOW);

            //Enable ODCLK
            MDrv_WriteRegBit(REG_CKG_ODCLK, DISABLE, CKG_ODCLK_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_ODCLK, DISABLE, CKG_ODCLK_GATED);
        }
#else

#if(HW_DESIGN_4K2K_VER == 4 || HW_DESIGN_4K2K_VER == 6)
        if(psXCInstPri->u32DeviceID == E_XC_DEVICE0)
#endif
        {
            // Enable FIClk2
            MDrv_WriteByteMask(REG_CKG_FICLK_F2, CKG_FICLK_F2_FLK, CKG_FICLK_F2_MASK);          // select FClk first, if pre-scaling down -> change to CKG_FICLK_F1_IDCLK2
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_FICLK_F2, DISABLE, CKG_FICLK_F2_GATED);                    // Enable clock

            // Enable FClk
            MDrv_WriteByteMask(REG_CKG_FCLK, CKG_FCLK_DEFAULT, CKG_FCLK_MASK);                   // select 170MHz, if not enough (for example, scaling bandwidth not enough), increase this
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_INVERT);                           // Not Invert
            MDrv_WriteRegBit(REG_CKG_FCLK, DISABLE, CKG_FCLK_GATED);                            // Enable clock

            // Enable IDClk2
            MDrv_WriteByteMask(REG_CKG_IDCLK2, CKG_IDCLK2_XTAL, CKG_IDCLK2_MASK);               // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_IDCLK2, DISABLE, CKG_IDCLK2_GATED);                        // Enable clock

            // Enable EDClk
            Hal_SC_set_edclk(pInstance, CKG_EDCLK_F2_345MHZ, ENABLE, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
            Hal_SC_set_edclk(pInstance, CKG_EDCLK_F1_345MHZ, ENABLE, SUB_WINDOW);
#endif

            // Enable FMClk
            Hal_SC_set_fmclk(psXCInstPri, ENABLE);

            // Enable FIClk2_f2
            Hal_SC_set_ficlk2(pInstance, CKG_FICLK2_F2_FCLK, ENABLE, MAIN_WINDOW);
        }
#if(HW_DESIGN_4K2K_VER == 4 || HW_DESIGN_4K2K_VER == 6)
        else
        {
            // Enable FIClk2
            MDrv_WriteByteMask(REG_CKG_S2_FICLK2_F2, CKG_S2_FICLK2_F2_FCLK, CKG_S2_FICLK2_F2_MASK);          // select FClk first, if pre-scaling down -> change to CKG_S2_FICLK_F1_IDCLK2
            MDrv_WriteRegBit(REG_CKG_S2_FICLK2_F2, DISABLE, CKG_S2_FICLK2_F2_INVERT);                   // Not Invert
            MDrv_WriteRegBit(REG_CKG_S2_FICLK2_F2, DISABLE, CKG_S2_FICLK2_F2_GATED);                    // Enable clock
            // Enable FClk
            MDrv_WriteByteMask(REG_CKG_S2_FCLK, CKG_S2_FCLK_DEFAULT, CKG_S2_FCLK_MASK);                   // select 170MHz, if not enough (for example, scaling bandwidth not enough), increase this
            MDrv_WriteRegBit(REG_CKG_S2_FCLK, DISABLE, CKG_S2_FCLK_INVERT);                           // Not Invert
            MDrv_WriteRegBit(REG_CKG_S2_FCLK, DISABLE, CKG_S2_FCLK_GATED);                            // Enable clock

            // Enable IDClk2
            MDrv_WriteByteMask(REG_CKG_S2_IDCLK2, CKG_S2_IDCLK2_XTAL, CKG_S2_IDCLK2_MASK);               // initial to XTAL first, will be changed when switch input source
            MDrv_WriteRegBit(REG_CKG_S2_IDCLK2, DISABLE, CKG_S2_IDCLK2_INVERT);                       // Not Invert
            MDrv_WriteRegBit(REG_CKG_S2_IDCLK2, DISABLE, CKG_S2_IDCLK2_GATED);                        // Enable clock
#if(HW_DESIGN_4K2K_VER == 6)
            // copy from halpnl.c init xc clk condition, to set the subwindow Odclk.
            if(MDrv_XC_IsSupportPipPatchUsingSc1MainAsSc0Sub())
            {
                if( ( E_XC_PNL_LPLL_VBY1_10BIT_16LANE == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type )||
                    ( E_XC_PNL_LPLL_VBY1_8BIT_16LANE == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) ||
                    ( E_XC_PNL_LPLL_VBY1_10BIT_1LANE == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) ||
                    ( E_XC_PNL_LPLL_VBY1_8BIT_1LANE == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) ||
                    ( E_XC_PNL_LPLL_TTL == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) ||
                    (( E_XC_PNL_LPLL_LVDS == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) &&
                    ( E_XC_MOD_OUTPUT_SINGLE == pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u8LPLL_Mode )) ||
                    (( E_XC_PNL_LPLL_HS_LVDS == (MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type ) &&
                    ( E_XC_MOD_OUTPUT_SINGLE == pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u8LPLL_Mode)))
                {
                    MDrv_WriteByteMask(REG_CKG_S2_ODCLK, 0x04, CKG_S2_ODCLK_MASK);
                }
                else
                {
                    MDrv_WriteByteMask(REG_CKG_S2_ODCLK, 0x0C, CKG_S2_ODCLK_MASK);
                }
            }
#endif

#ifdef MONACO_SC2_PATCH
            // Enable reg_sc2_odclk_div_en
            MDrv_WriteByteMask(0x101E2C , 0x0E, 0x0E);
            MDrv_WriteByteMask(0x101E30 , 0x11, 0x11);
#endif
        }
#endif
#endif

#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
        // Enable SPT4K_L2
        MDrv_WriteByteMask(REG_CKG_S2_GOP_HDR, CKG_S2_GOP_HDR_EDCLK, CKG_S2_GOP_HDR_MASK);
        MDrv_WriteRegBit(REG_CKG_S2_GOP_HDR, DISABLE, CKG_S2_GOP_HDR_INVERT); // Not Invert
        MDrv_WriteRegBit(REG_CKG_S2_GOP_HDR, DISABLE, CKG_S2_GOP_HDR_GATED);

        MDrv_WriteRegBit(REG_CKG_S2_MECLK, DISABLE, CKG_S2_MECLK_INVERT); // Not Invert
        MDrv_WriteRegBit(REG_CKG_S2_MECLK, DISABLE, CKG_S2_MECLK_GATED);

        MDrv_WriteRegBit(REG_CKG_S2_MGCLK, DISABLE, CKG_S2_MGCLK_INVERT); // Not Invert
        MDrv_WriteRegBit(REG_CKG_S2_MGCLK, DISABLE, CKG_S2_MGCLK_GATED);

        Hal_SC_HDR_CLK_Setting(pInstance);
#endif
#endif

#endif
        MDrv_SC_SetDisplay_LineBuffer_Mode(pInstance, ENABLE);
        MDrv_SC_SetDisplay_Start_Mode(pInstance, ENABLE);

        //Set line buffer merge address
        //This value will not changed. It is depend on line buffer size of main window.
        MDrv_SC_Set_LB_MergeAddress(pInstance);

#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        //For pip, need to enable sub line buffer first
        MDrv_XC_FilLineBuffer(pInstance, ENABLE, SUB_WINDOW);
#endif

        // Set PAFRC mixed with noise dither disable---Move the control to pnl mod init
        //SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK24_3F_L, 0x0, 0x8);

        //extra fetch between main and sub
        //if the setting value is too big, hw cannot catch up
        //by CC, it's safe to set 1366x768 panel the same with 1920x1080 panel.
        Hal_SC_Set_extra_fetch_line(pInstance, 0x05);//H:extra fetch line
        Hal_SC_Set_extra_adv_line(pInstance, 0x03);  //V:extra advance line

        MDrv_SC_SetSkipWaitVsync(pInstance, MAIN_WINDOW, FALSE);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_SC_SetSkipWaitVsync(pInstance, SUB_WINDOW , FALSE);
#endif

        // enable osd layer blending with main window
#if (VIDEO_OSD_SWITCH_VER > 1)
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_23_L, BIT(6), BIT(6));
#endif
#ifdef K3_U2
        //Mantis issue of 0266525 . Need to toggle IP write mask field count clear before input V sync .
        //Main window
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_07_L, BIT(4), BIT(4) |BIT(5) |BIT(6));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_07_L, 0, BIT(7));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_07_L, BIT(7), BIT(7));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_07_L, 0, BIT(7));
        //Sub window
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_47_L, BIT(4), BIT(4) |BIT(5) |BIT(6));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_47_L, 0, BIT(7));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_47_L, BIT(7), BIT(7));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_47_L, 0, BIT(7));
#endif
        //if mweEnable not be called, sub win will be open default, we need to black it at first.
        //so that to avoid garbage when opening sub win.
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_19_L, BIT(5), BIT(5));
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        gSrcInfo[SUB_WINDOW].bBlackscreenEnabled = TRUE;
#endif
#if SUPPORT_SEAMLESS_ZAPPING
        // for seamless zapping, this bit will maximize IP motion detection inside scaler
        // this motion setting will NOT override PQ's setting.
        // no need to be controlled by Qmap
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_45_L, BIT(9) , BIT(9));
#endif

        // this should only turned on when Top Bottom input with PreV scaling down enabled.
        // but the hardware's default value is ON so we turned off when inited.
        HAL_SC_Set_vsd_3D_autofactor_reset(pInstance, DISABLE, MAIN_WINDOW);

        // Enable IPM tune after DS feature
        MDrv_XC_EnableIPMTuneAfterDS(pInstance, ENABLE);

        //enable F2 IPM wbank freeze alginment mode //mantis:0747407
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_06_L, BIT(12) , BIT(12));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK32_06_L, BIT(12) , BIT(12));

        _XC_RETURN(pInstance);
    }
    else
    {
        // default frame buffer address init
        MDrv_XC_SetFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_SetFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size, SUB_WINDOW);
#endif
        // default frcm frame buffer address init
        MDrv_XC_SetFRCMFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Size, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
        MDrv_XC_SetFRCMFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Size, SUB_WINDOW);
#endif

#ifdef UFO_XC_SUPPORT_DUAL_MIU
        MDrv_XC_SetDualFrameBufferAddressSilently(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Dual_FB_Start_Addr,
                                              pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Dual_FB_Size, MAIN_WINDOW);
#endif

        //when from mboot to AP, there is an OS switch, so we need to do interrupt handle again.
        /**
         * !!! Attention !!! The ISR interface is different from Chakra2 and Utopia.
         * In Chakra2, there are two para needed, MHAL_SavedRegisters and vector.
         * In Utopia, there is no parameter.
         */
#ifdef MSOS_TYPE_LINUX_KERNEL
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            MsOS_AttachInterrupt_Shared(E_INT_IRQ_DISP,(InterruptCb) MDrv_SC_isr);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_AttachInterrupt_Shared(E_INT_IRQ_DISP1,(InterruptCb) MDrv_SC1_isr);
        }
#else
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            MsOS_AttachInterrupt(E_INT_IRQ_DISP,(InterruptCb) MDrv_SC_isr);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_AttachInterrupt(E_INT_IRQ_DISP1,(InterruptCb) MDrv_SC1_isr);
        }
#endif

#ifndef CHIP_I2
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            MsOS_EnableInterrupt(E_INT_IRQ_DISP);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_EnableInterrupt(E_INT_IRQ_DISP1);
        }
#endif

    }

#if FRC_INSIDE
    // In A5 new feature,pre-arbiter can support (IPM_W and IPS_R)/(IPM_R and IPS_W)/(OP_R and OPW)  use the same MIU request client
    //Others, total 6 MIU request client

    // Current setting is : Turn off the max request function of pre-arbiter(2 to 1)
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_63_L,0x0A00); // 0xFF00 // miu merge arbiter IP0
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_68_L,0x0A00); // 0xFF00 // miu merge arbiter IP1
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_6A_L,0x1000); // 0xFF00 // miu merge arbiter OP
    MHal_FRC_3DLR_Select(pInstance, FALSE);
#endif

    //for HDMI AV mute, when AVMUTE = 1, HDMI will notice to XC to disable the input source on Main/Sub
    Hal_SC_Enable_AVMute(pInstance, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
    Hal_SC_Enable_AVMute(pInstance, SUB_WINDOW);
#endif

#if (HW_DESIGN_4K2K_VER == 4)
    // for HW Auto No signal can control FRCM freeze
    Hal_SC_set_frcm_to_freeze(pInstance, ENABLE, MAIN_WINDOW);
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
    Hal_SC_set_frcm_to_freeze(pInstance, ENABLE, SUB_WINDOW);
#endif

    // for 3D 6Tap
    Hal_SC_3D_set_top_win_6Tap(pInstance, ENABLE);
#endif

    Hal_SC_Init(pInstance);

    Hal_SC_set_T3D_H_size(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
#ifdef CONFIG_MSTAR_SRAMPD
    // To avoid mantis 1082875: boot logo flash issue, remove  SRAM PD for LD from XC_init to PNL_init
    //MHal_XC_Set_LD_SRAM_Power_Down(pInstance, TRUE);
    Hal_SC_EnableCLK_for_SUB(pInstance, FALSE);
    Hal_SC_Sub_SRAM_PowerDown_Control(pInstance,FALSE);
    MHal_XC_Set_ADC_SRAM_Power_Down(pInstance, TRUE);
#endif
    if(!(MHal_XC_Init_Patch(pInstance, ENABLE, MAIN_WINDOW)))
    {
        printf("no need to patch\n");
    }
//    MApi_XC_SetCompressionMode(ENABLE, MAIN_WINDOW);
    return TRUE;
}
#ifdef SUPPORT_BWD
static void MDrv_SC_BWR_mem_cfg_map_bpp(XC_RESOURCE_PRIVATE* pXCResourcePrivate, SCALER_WIN eWindow)
{
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_1].u16BWR_MEM_CFG_VALUE = 0x0;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_1].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_16;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_2].u16BWR_MEM_CFG_VALUE = 0x4;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_2].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_20;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_3].u16BWR_MEM_CFG_VALUE = 0x5;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_3].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_20;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_4].u16BWR_MEM_CFG_VALUE = 0x6;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_4].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_20;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_5].u16BWR_MEM_CFG_VALUE = 0x8;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_5].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_24;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_6].u16BWR_MEM_CFG_VALUE = 0x9;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_6].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_24;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_7].u16BWR_MEM_CFG_VALUE = 0xA;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_7].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_24;

    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_8].u16BWR_MEM_CFG_VALUE = 0xB;
    gSrcInfo[eWindow].Status2.stMemCfgMap[E_XC_BWR_MEM_CONFIG_8].eBitPerPixelType = E_XC_BWR_MEM_CFG_BPP_24;
}
#endif
//-------------------------------------------------------------------------------------------------
/// Initialize the XC
/// If the length is 0, it won't do anything except the mutex and share memory. Usually it is the action of DFB
/// @param  pXC_InitData                  \b IN: the Initialized Data
/// @param  u32InitDataLen                \b IN: the length of the initialized data
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_Init_U2(void* pInstance, XC_INITDATA *pXC_InitData, MS_U32 u32InitDataLen)
{
    char wordISR[] = {"_XC_ISR_Mutex"};


#if(XC_CONTEXT_ENABLE)
    MS_BOOL bFirstInstance = TRUE;
#endif
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    MS_U8 u8VerCharIndex = 0 ;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

#if 0
    printf("[XC,Version]\33[0;36m ");
    for (; u8VerCharIndex < 8 ; u8VerCharIndex++)
        printf("%c",_api_xc_version.DDI.change[u8VerCharIndex]);
    printf("\n \33[m");
#endif

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
#ifdef MSOS_TYPE_LINUX_KERNEL
    init_waitqueue_head(&pXCResourcePrivate->stXC_Utility._XC_EventQueue);
#endif

#if(XC_CONTEXT_ENABLE)
    _MApi_XC_Init_Context(pInstance, &bFirstInstance);
    if (!bFirstInstance)
    {
        //The XC instance already exists
        //return TRUE;
    }

    #ifdef STELLAR//[LM17A_BRINGUP_FIXME]
    XC_INITDATA zeroXC;
    memset((void*)(&zeroXC),0,sizeof(XC_INITDATA));
    if(!bFirstInstance && pXC_InitData && strncmp((void*)pXC_InitData,(void*)(&zeroXC),sizeof(XC_INITDATA)) == 0)
    {
        printf("MMM  %s bypass bFirstInstance:%d\n",__FUNCTION__,bFirstInstance);
         _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return TRUE;
    }
    #endif////[LM17A_BRINGUP_FIXME]

    if (&(pXCResourcePrivate->stdrvXC_MVideo._SContext) == NULL || &(pXCResourcePrivate->stdrvXC_MVideo._SCShared) == NULL)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "[MAPI XC][%06d] create context fail\n", __LINE__);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return FALSE;
    }

#endif

    if (_XC_Mutex == -1)
    {
        _XC_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, "_XC_Mutex", MSOS_PROCESS_SHARED);
        if (_XC_Mutex == -1)
        {
            printf("[MAPI XC]_XC_Mutex create mutex fail\n");
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "[MAPI XC][%06d] create mutex fail\n", __LINE__);
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            return FALSE;
        }
    }

    if (_XC_ISR_Mutex == -1)
    {
        _XC_ISR_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, wordISR, MSOS_PROCESS_SHARED);
        if (_XC_ISR_Mutex == -1)
        {
            printf("[MAPI XC]_XC_ISR_Mutex create mutex fail\n");
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "[MAPI XC][%06d] create ISR mutex fail\n", __LINE__);
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            return FALSE;
        }
    }

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // This should be the first function. get MMIO base
    if (MDrv_XC_SetIOMapBase_i(pInstance) != TRUE)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init MDrv_XC_SetIOMapBase() failure\n");
    }

    MDrv_XC_SetDeviceOffset(pInstance);


#ifdef STELLAR
    S_XC_CUS_CTRL_PARA stuff;
    if(Hal_XC_S_DS_GST_PROCESS_GET(pInstance, &stuff))
    {
        if(stuff.bParam)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Skip the following init in Gstreamer process\n");
            return TRUE;
        }
    }

    if(Hal_XC_S_DS_CAPTURE_PROCESS_GET(pInstance, &stuff))
    {
        if(stuff.bParam)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Skip the following init in Capture process\n");
            return TRUE;
        }
    }
#endif

    // If the length is 0, it won't do anything except the mutex and share memory. Usually it is the action of DFB
    // if the _bDFBInit is TRUE, it also indicates DFB init case
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    if((0 == u32InitDataLen) || (TRUE == pXCResourcePrivate->stdrvXC_MVideo._bDFBInit))
    {
        pXCResourcePrivate->stdrvXC_MVideo._bDFBInit = FALSE;

        mvideo_sc_variable_init(pInstance, bFirstInstance, NULL);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init(): the u32InitDataLen is zero or DFB init case.\n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return TRUE;
    }
    MDrv_XC_FPLLCusReset(pInstance);

    pXCResourcePrivate->stdrvXC_MVideo_Context._bEnableEuro = FALSE;//Euro HDTV support flag
#ifndef MSOS_TYPE_OPTEE
    if(MApi_PNL_Get_TCON_Capability())
    {
        if (pXC_InitData->stPanelInfo.eLPLL_Type >= E_XC_PNL_LPLL_EXT)
        {
            pXC_InitData->stPanelInfo.eLPLL_Type = (E_XC_PNL_LPLL_TYPE)MApi_Pnl_Get_LPLL_Type();
        }
        printf("%s, %d, pXC_InitData->stPanelInfo.eLPLL_Type=%d\n", __FUNCTION__, __LINE__, pXC_InitData->stPanelInfo.eLPLL_Type);
    }
#endif
    #ifdef MSOS_TYPE_LINUX_KERNEL
    if(is_compat_task())
    {
        memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, pXC_InitData, sizeof(XC_INITDATA));
    }
    else
    {
        memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, pXC_InitData, ((u32InitDataLen>=sizeof(XC_INITDATA))? sizeof(XC_INITDATA):u32InitDataLen));
    }
    #else
    memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, pXC_InitData, ((u32InitDataLen>=sizeof(XC_INITDATA))? sizeof(XC_INITDATA):u32InitDataLen));
    #endif
    pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32InitDataLen_Suspend = u32InitDataLen;
#if defined (__aarch64__)
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "Main_FB(start, size)=(%lx, %lu)\n", pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr,
                                                                             pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "Sub_FB (start, size)=(%lx, %lu)\n", pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr,
                                                                             pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size);
#else
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "Main_FB(start, size)=(%tx, %tu)\n", (ptrdiff_t)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr,
                                                                             (ptrdiff_t)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "Sub_FB (start, size)=(%tx, %tu)\n", (ptrdiff_t)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr,
                                                                             (ptrdiff_t)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size);
#endif

    // Scart ID port selection is available after version 1.
    if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32XC_version < 1)
    {
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.eScartIDPort_Sel = E_XC_SCARTID_NONE;
    }

#ifdef STELLAR //Avoid xc ipm/opm write cause memory hit // sync from previous projects
    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_07_L, BIT(0)|BIT(1)  , BIT(0)|BIT(1) ); //ipm_rreq_off_f2, ipm_wreq_off_f2
    MDrv_Write2ByteMask(MIU0_G6_REQUEST_MASK, 0, BIT(1));//MIU opm Unmask // miu0 group6 client1
#endif

    mvideo_sc_variable_init(pInstance, bFirstInstance, pXC_InitData);

    // For MI multi init case,we should confirm enPrevPowerState = E_POWER_MECHANICAL
    // after STR
    pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState = E_POWER_MECHANICAL;

    _MApi_XC_Init_WithoutCreateMutex(pInstance, pXC_InitData, u32InitDataLen);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    //MApi_XC_SetDbgLevel(XC_DBGLEVEL_SETTIMING|XC_DBGLEVEL_SETWINDOW);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    //map memory config to bit per pix
#ifdef SUPPORT_BWD
    MDrv_SC_BWR_mem_cfg_map_bpp(pXCResourcePrivate, MAIN_WINDOW);
#endif
    pXCResourcePrivate->stdrvXC_MVideo.bModuleInited = TRUE;

    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Init(XC_INITDATA *pXC_InitData, MS_U32 u32InitDataLen)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_INIT XCArgs;
    XCArgs.pXC_InitData = pXC_InitData;
    XCArgs.u32InitDataLen = u32InitDataLen;
    XCArgs.bReturnValue = FALSE;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get the XC init config
/// @param  pXC_InitData                  \b IN: the Initialized Data
/// @return @ref E_APIXC_ReturnValue
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_GetConfig_U2(void* pInstance, XC_INITDATA *pXC_InitData)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(pXCResourcePrivate->stdrvXC_MVideo.bModuleInited)
    {
        memcpy(pXC_InitData, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, sizeof(XC_INITDATA));
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return E_APIXC_RET_OK;
    }
    else
    {
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return E_APIXC_RET_FAIL;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_GetConfig(XC_INITDATA *pXC_InitData)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }

    stXC_GET_CONFIG XCArgs;
    XCArgs.pXC_InitData = pXC_InitData;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_CONFIG, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("XC engine GET_CONFIG Ioctl fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MDrv_XC_Init_MISC(void *pInstance, XC_INITMISC *pXC_Init_Misc, MS_U32 u32InitMiscDataLen)
{

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    MST_PANEL_INFO_t stFRCPanelInfo;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    if(u32InitMiscDataLen != sizeof(XC_INITMISC))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return E_APIXC_RET_FAIL;
    }

#if (SUPPORT_IMMESWITCH == 0)
    if (pXC_Init_Misc->u32MISC_A & E_XC_INIT_MISC_A_IMMESWITCH )
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return E_APIXC_RET_FAIL;
    }
#endif

#if (SUPPORT_DVI_AUTO_EQ == 0)
    if (pXC_Init_Misc->u32MISC_A & E_XC_INIT_MISC_A_DVI_AUTO_EQ)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return E_APIXC_RET_FAIL;
    }
#endif

#if (FRC_INSIDE == 0)
    if ((pXC_Init_Misc->u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE) || (pXC_Init_Misc->u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_60HZ))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return E_APIXC_RET_FAIL;
    }
#endif

#if FRC_INSIDE
    #if (HW_SUPPORT_FRC_DUAL_MIU == 0)
        if (pXC_Init_Misc->u32MISC_A & E_XC_INIT_MISC_A_FRC_DUAL_MIU)
        {
            pXC_Init_Misc->u32MISC_A &= ~E_XC_INIT_MISC_A_FRC_DUAL_MIU;
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "HW NOT support FRC dual MIU! Keep single MIU HERE!\n");
        }
    #endif
#endif

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    // Start only when all setting ready.
    memset(&stFRCPanelInfo, 0, sizeof(MST_PANEL_INFO_t));

    memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc, pXC_Init_Misc, u32InitMiscDataLen);
    pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32InitMiscDataLen_Suspend = u32InitMiscDataLen;

    if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FAST_GET_VFREQ)
    {
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFast_Get_VFreq = TRUE;
    }
    else
    {
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFast_Get_VFreq = FALSE;
    }

    #if (FRC_INSIDE)
    if(((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE)
        || (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_60HZ)
        || (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_4K1K_120HZ)
        || (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_4K_HALFK_240HZ)
        || (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_KEEP_OP_4K2K))
        #if (HW_DESIGN_4K2K_VER == 6)
        && (TRUE == MHal_FRC_IsSupportFRC_byEfuse(pInstance))
        #endif
        )
    {
        if (FALSE == pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited)
        {
            MHal_CLKGEN_FRC_Init(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u8LPLL_Mode);
            MDrv_FRC_PNLInfo_Transform(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo,
                                   &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc,
                                   &stFRCPanelInfo);

            if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.bFRC) // init ready?! if yes, continue
            {
                MDrv_FRC_Init(pInstance, &stFRCPanelInfo, &(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo));
                gSrcInfo[MAIN_WINDOW].Status2.bInFRCMode = TRUE; //Store FRC enable state in share memory for multi-process
                gSrcInfo[SUB_WINDOW].Status2.bInFRCMode = gSrcInfo[MAIN_WINDOW].Status2.bInFRCMode;
                MHal_FRC_3DLR_Select(pInstance, TRUE);
            }

            // when OUTPUT is 4k2k@60Hz then we turn on FRC
            // in other case , we stil do FRC init but we choose to bypass FRC
            if ((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultVFreq < 600)
                && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width == 3840)
                && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height == 2160))
            {
                MDrv_FRC_ByPass_Enable(pInstance, TRUE);
            }
            else
            {
                MDrv_FRC_ByPass_Enable(pInstance, FALSE);
            }

            pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited = TRUE;
        }

        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_KEEP_OP_4K2K)
        {
            // SW patch here, if AP set keep OP as 4K2K, need to adjust DE and Htt before FRC setting is 4K2K
            if (SC_R2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK10_0C_L) < 3000
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB//follow HW_DESIGN_4K2K_VER = 6 rule
               && (psXCInstPri->u32DeviceID == 0)
#endif
            )
            {
                if(MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)
                {
                    _MLOAD_ENTRY(pInstance);

                    MDrv_XC_MLoad_WriteCmd(pInstance, REG_SC_BK10_0C_L, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal, VOP_HTT_MASK);
                    MDrv_XC_MLoad_WriteCmd(pInstance, REG_SC_BK10_05_L, (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart+pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width - 1), VOP_DE_HEND_MASK);

                    MDrv_XC_MLoad_Fire(pInstance, TRUE);
                    _MLOAD_RETURN(pInstance);
                }
                else
                {
                    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK10_0C_L, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal);
                    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK10_05_L, (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart+pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width - 1));
                }
            }
        }

        //printf("\n## FRC %s mode\n",g_XC_Pnl_Misc.FRCInfo.bFRC?"NORMAL":"BYPASS");
        //printf("## FRC LPLL mode =%d\n\n",g_XC_InitData.stPanelInfo.u8LPLL_Mode);
        //printf("[%s][%d]\n",__FUNCTION__,__LINE__);
    }
    else
    {
        MDrv_FRC_ByPass_Enable(pInstance, TRUE);
    }
    #endif
#if (HW_DESIGN_4K2K_VER == 4)
    if (((psXCInstPri->u32DeviceID == 0) && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_LEGACY_MODE))
         ||(psXCInstPri->u32DeviceID == 1))
    {
        MDrv_SC_Enable_LegacyMode(pInstance, TRUE);
    }
    else
    {
        MDrv_SC_Enable_LegacyMode(pInstance, FALSE);
    }
#endif
    //printf("[%s][%d]\n",__FUNCTION__,__LINE__);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return E_APIXC_RET_OK;
}

E_APIXC_ReturnValue MApi_XC_Init_MISC_U2(void* pInstance, XC_INITMISC *pXC_Init_Misc, MS_U32 u32InitMiscDataLen)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    eReturn = MDrv_XC_Init_MISC(pInstance, pXC_Init_Misc, u32InitMiscDataLen);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return eReturn;
}
E_APIXC_ReturnValue MApi_XC_Init_MISC(XC_INITMISC *pXC_Init_Misc, MS_U32 u32InitMiscDataLen)
{
    if (pu32XCInst == NULL
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        || pu32XCInst_1 == NULL
#endif
        )
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_INIT_MISC XCArgs;
    XCArgs.pXC_Init_Misc = pXC_Init_Misc;
    XCArgs.u32InitMiscDataLen = u32InitMiscDataLen;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if((UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        || (UtopiaIoctl(pu32XCInst_1, E_XC_CMD_INIT_MISC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
#endif
        )
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
E_APIXC_ReturnValue MDrv_XC_GetMISCStatus(void* pInstance, XC_INITMISC *pXC_Init_Misc)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    memcpy(pXC_Init_Misc,&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc,sizeof(XC_INITMISC));

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return E_APIXC_RET_OK;
}
E_APIXC_ReturnValue MApi_XC_GetMISCStatus_U2(void* pInstance, XC_INITMISC *pXC_Init_Misc)
{
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;

    eReturn = MDrv_XC_GetMISCStatus(pInstance, pXC_Init_Misc);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    return eReturn;
}
E_APIXC_ReturnValue MApi_XC_GetMISCStatus(XC_INITMISC *pXC_Init_Misc)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }
    static XC_INITMISC tmp_Init_Misc;
    stXC_GET_MISC_STATUS XCArgs;
    memcpy(&tmp_Init_Misc,pXC_Init_Misc,sizeof(XC_INITMISC));
    XCArgs.pXC_Init_Misc = &tmp_Init_Misc;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MISC_STATUS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        memcpy(pXC_Init_Misc,&tmp_Init_Misc,sizeof(XC_INITMISC));
        return XCArgs.eReturnValue;
    }
}
E_APIXC_ReturnValue MApi_XC_GetChipCaps_U2(void* pInstance, E_XC_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;

    eReturn = MDrv_XC_GetChipCaps(pInstance, eCapType, pRet, ret_size);

    return eReturn;
}
E_APIXC_ReturnValue MApi_XC_GetChipCaps(E_XC_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size)
{

    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_CHIP_CAPS XCArgs;
    XCArgs.eCapType = eCapType;
    XCArgs.pRet = pRet;
    XCArgs.ret_size = ret_size;
    XCArgs.eReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_CHIPCAPS, (void*)&XCArgs) != 0)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
MS_U32 MApi_XC_GetCapability_U2(void* pInstance, MS_U32 u32Id)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    MS_U32 u32Val = 0;

    switch (u32Id)
    {
        case E_XC_SUPPORT_IMMESWITCH:
            MDrv_XC_GetChipCaps(pInstance, E_XC_IMMESWITCH,&u32Val,sizeof(u32Val));
            break;

        case E_XC_SUPPORT_DVI_AUTO_EQ:
            MDrv_XC_GetChipCaps(pInstance, E_XC_DVI_AUTO_EQ,&u32Val,sizeof(u32Val));
            break;

        case  E_XC_SUPPORT_FRC_INSIDE:
            MDrv_XC_GetChipCaps(pInstance ,E_XC_FRC_INSIDE,&u32Val,sizeof(u32Val));
            break;
        default:
            u32Val = 0;
            break;
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u32Val;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_GetCapability(MS_U32 u32Id)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_GET_CAPABILITY XCArgs;
    XCArgs.u32Id = u32Id;
    XCArgs.u32ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_CAPABILITY, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u32ReturnValue;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
/// Enable sub window clock.
//-------------------------------------------------------------------------------------------------
void MApi_XC_EnableCLK_for_SUB(void *pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_XC_EnableCLK_for_SUB(pInstance, bEnable);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

//-------------------------------------------------------------------------------------------------
/// Enable DIP window clock.
//-------------------------------------------------------------------------------------------------
void MApi_XC_EnableCLK_for_DIP(void *pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_SC_EnableCLK_for_DIP(pInstance, bEnable);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

//-------------------------------------------------------------------------------------------------
/// This function will change panel type dynamically
/// @param  pstPanelInfo      \b IN: the new panel type
//-------------------------------------------------------------------------------------------------
void MApi_XC_ChangePanelType_U2(void* pInstance, XC_PANEL_INFO *pstPanelInfo)
{
    XC_PANEL_INFO_EX stPanelInfoEx;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    //Reset information used by ex panel info
    memset(&stPanelInfoEx, 0, sizeof(XC_PANEL_INFO_EX));
    stPanelInfoEx.u16VFreq = PANEL_INFO_EX_INVALID_ALL;
    stPanelInfoEx.u32PanelInfoEx_Version = PANEL_INFO_EX_VERSION;
    stPanelInfoEx.u16PanelInfoEX_Length = sizeof(XC_PANEL_INFO_EX);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if (psXCInstPri->u32DeviceID == 1)
    {
        /// not support
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return;
    }
    CHECK_DEVICE1_INSTANCE();
    XC_INSTANCE_PRIVATE *psXCInstPri_1 = NULL;
    UtopiaInstanceGetPrivate(g_pDevice1Instance, (void**)&psXCInstPri_1);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate_1 = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri_1->u32DeviceID)],(void**)(&pXCResourcePrivate_1));
#endif
    MDrv_XC_SetExPanelInfo(pInstance, FALSE, &stPanelInfoEx);
    _XC_ENTRY(pInstance);
    MDrv_XC_GetDefaultHVSyncInfo(pInstance);
    _XC_RETURN(pInstance);

#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    MDrv_XC_SetExPanelInfo(g_pDevice1Instance, FALSE, &stPanelInfoEx);
    _XC_ENTRY(g_pDevice1Instance);
    MDrv_XC_GetDefaultHVSyncInfo(g_pDevice1Instance);
    _XC_RETURN(g_pDevice1Instance);
#endif

    if (pstPanelInfo->eLPLL_Type == E_XC_PNL_LPLL_EXT)
    {
        pstPanelInfo->eLPLL_Type = (E_XC_PNL_LPLL_TYPE)MApi_Pnl_Get_LPLL_Type();
    }
    //printf("%s, %d, pstPanelInfo->eLPLL_Type=%d\n", __FUNCTION__, __LINE__, pstPanelInfo->eLPLL_Type);

    memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo, pstPanelInfo, sizeof(XC_PANEL_INFO));
    memcpy(&(gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming), &(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo), sizeof(XC_PANEL_TIMING));
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    memcpy(&pXCResourcePrivate_1->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo, pstPanelInfo, sizeof(XC_PANEL_INFO));
    memcpy(&(gSrcInfo_1[MAIN_WINDOW].Status2.stXCPanelDefaultTiming), &(pXCResourcePrivate_1->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo), sizeof(XC_PANEL_TIMING));
#endif
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_ChangePanelType(XC_PANEL_INFO *pstPanelInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_CHANGE_PANELTYPE XCArgs;
    XCArgs.pstPanelInfo = pstPanelInfo;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_CHANGE_PANELTYPE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// This function will let scaler driver know if to decide best pre-scaling down ratio based on FB size of not.
/// Please set this before call MApi_XC_SetWindow()
/// @param  bEnable      \b IN: enable this feature or not
//-------------------------------------------------------------------------------------------------
void MApi_XC_SetAutoPreScaling_U2(void* pInstance, MS_BOOL bEnable, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    gSrcInfo[eWindow].Status2.bAutoBestPreScaling = bEnable;
    //printf("MApi_XC_SetAutoPreScaling(): obsolete interface\n");
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetAutoPreScaling(MS_BOOL bEnable, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_AUTO_PRESCALING XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_AUTO_PRESCALING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// This function will enable/diable output black pattern
/// @param  bEnable      \b IN: TRUE: enable, FALSE: otherwise
//-------------------------------------------------------------------------------------------------
void MApi_XC_Set_BLSK_U2(void* pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    MDrv_SC_Set_BLSK(pInstance, bEnable);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Set_BLSK(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BLACKSCREEN XCArgs;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BLACKSCREEN, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif

void MApi_XC_GenerateBlackVideoForBothWin_U2( void* pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    if(E_MLOAD_ENABLED == MDrv_XC_MLoad_GetStatus(pInstance))
    {
        MDrv_SC_GenerateBlackVideoForBothWin(pInstance, bEnable);
    }
    else
    {
        MDrv_XC_WaitOutputVSync(pInstance, 1, 50, MAIN_WINDOW);
        MDrv_SC_GenerateBlackVideo(pInstance, bEnable, MAIN_WINDOW);
        MDrv_XC_WaitOutputVSync(pInstance, 1, 50, SUB_WINDOW);
        MDrv_SC_GenerateBlackVideo( pInstance, bEnable, SUB_WINDOW);
    }
    gSrcInfo[MAIN_WINDOW].bBlackscreenEnabled = bEnable;
    gSrcInfo[SUB_WINDOW].bBlackscreenEnabled = bEnable;
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_GenerateBlackVideoForBothWin( MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BOTHWINDOW_BLACKVIDEO XCArgs;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BOTHWINDOW_BLACKVIDEO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
//-------------------------------------------------------------------------------------------------
/// This function will enable/diable output black pattern by mode
/// @param  bEnable      \b IN: TRUE: enable, FALSE: otherwise
/// @param  eWindow    \b IN: which window we are going to query
/// @param  eMode        \b IN: which black video mode we are going to use
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_GenerateBlackVideoByMode_U2( void* pInstance, MS_BOOL bEnable, SCALER_WIN eWindow, EN_XC_BLACK_VIDEO_MODE eMode )
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);

    if(eWindow < MAX_WINDOW)
    {
        eRet = MDrv_SC_GenerateBlackVideoByMode(pInstance, bEnable, eWindow, eMode);
        gSrcInfo[eWindow].bBlackscreenEnabled = bEnable;
    }
    else
    {
        // For now not supporting memsync black for both window
        if(eMode == E_XC_BLACK_VIDEO_MEMSYNC)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "Not supporting both window memsync black video, using basic mode\n");
        }

        MDrv_SC_GenerateBlackVideoForBothWin(pInstance, bEnable);
        gSrcInfo[MAIN_WINDOW].bBlackscreenEnabled = gSrcInfo[SUB_WINDOW].bBlackscreenEnabled = bEnable;
        eRet = E_APIXC_RET_OK;
    }

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_GenerateBlackVideoByMode ( MS_BOOL bEnable, SCALER_WIN eWindow, EN_XC_BLACK_VIDEO_MODE eMode )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_BLACKVIDEO_BYMODE XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eWindow = eWindow;
    XCArgs.eMode = eMode;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BLACKVIDEO_BYMODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#endif
//-------------------------------------------------------------------------------------------------
/// This function will enable/diable output black pattern
/// @param  bEnable      \b IN: TRUE: enable, FALSE: otherwise
//-------------------------------------------------------------------------------------------------
void MApi_XC_GenerateBlackVideo_U2( void* pInstance, MS_BOOL bEnable, SCALER_WIN eWindow )
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    //wait VSync and do setting in Vblanking to avoid broken picture

    if(MDrv_XC_MLoad_GetStatus(pInstance) != E_MLOAD_ENABLED)
    {
        MDrv_XC_WaitOutputVSync(pInstance, 1, 50, MAIN_WINDOW);
    }

    if ( eWindow < MAX_WINDOW )
    {
        MDrv_SC_GenerateBlackVideo(pInstance, bEnable, eWindow );
        gSrcInfo[eWindow].bBlackscreenEnabled = bEnable;
    }
    else
    {
        MDrv_SC_GenerateBlackVideoForBothWin(pInstance, bEnable);
        gSrcInfo[MAIN_WINDOW].bBlackscreenEnabled = gSrcInfo[SUB_WINDOW].bBlackscreenEnabled = bEnable;
    }

    #if TEST_NEW_DYNAMIC_NR   //Why turn on/of DNR here, but not in SetBlueScreen?
    // DNR should be turned on only if video is avaliable.m
    if (mvideo_sc_is_enable_3dnr(pInstance, MAIN_WINDOW))
    {
        //printk("Enable 3DNR %bu (flag is 0x%x\n", !bEnable, g_SrcInfo.u16ColorFinetuneFlag & FINETUNE_3DNR);
        MDrv_Scaler_Enable3DNR(pInstance, !bEnable, MAIN_WINDOW);
    }
    #endif

#if 0
    // Enable FPD function when source is MVOP ouput & "Not" in FBL mode.
    {
        if( (IsSrcTypeDTV(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType) ) &&
            (!MDrv_XC_IsCurrentFrameBufferLessMode()))
        {
            MApi_XC_set_FD_Mask( !bEnable );
        }
    }
#endif
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_GenerateBlackVideo ( MS_BOOL bEnable, SCALER_WIN eWindow )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BLACKVIDEO XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BLACKVIDEO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Query whether current XC is black video enabled or not
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_IsBlackVideoEnable_U2( void* pInstance, SCALER_WIN eWindow )
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    MS_BOOL bEnable = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    bEnable = gSrcInfo[eWindow].bBlackscreenEnabled;
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bEnable;
}

MS_BOOL MApi_XC_IsBlackVideoEnable( SCALER_WIN eWindow )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_CHECK_BLACKVIDEO_ENABLE XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CHECK_BLACKVIDEO_ENABLE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// wait for input sync
/// @param  u8NumVSyncs        \b IN: the number of VSync we are going to wait
/// @param  u16Timeout         \b IN: time out
/// @return @ref MS_U8 retun the number of left Vsync if timeout
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_WaitInputVSync_U2(void* pInstance, MS_U8 u8NumVSyncs, MS_U16 u16Timeout, SCALER_WIN eWindow)
{
    MS_U8 u8InputVsyncTime;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);
    u8InputVsyncTime = MDrv_XC_wait_input_vsync(pInstance, u8NumVSyncs, u16Timeout, eWindow);
    //return MDrv_XC_wait_input_vsync(u8NumVSyncs, u16Timeout, eWindow);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u8InputVsyncTime;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U8 MApi_XC_WaitInputVSync(MS_U8 u8NumVSyncs, MS_U16 u16Timeout, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0xFF;
    }

    stXC_SET_WAIT_INPUT_VSYNC XCArgs;
    XCArgs.u8NumVSyncs = u8NumVSyncs;
    XCArgs.u16Timeout = u16Timeout;
    XCArgs.eWindow = eWindow;
    XCArgs.u8ReturnValue = 0xFF;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_WAIT_INPUT_VSYNC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0xFF;
    }
    else
    {
        return XCArgs.u8ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// wait for output sync
/// @param  u8NumVSyncs        \b IN: the number of VSync we are going to wait
/// @param  u16Timeout         \b IN: time out
/// @return @ref MS_U8 retun the number of left Vsync if timeout
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_WaitOutputVSync_U2(void* pInstance, MS_U8 u8NumVSyncs, MS_U16 u16Timeout, SCALER_WIN eWindow)
{
    MS_U8 u8OutputVsyncTime;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    u8OutputVsyncTime = MDrv_XC_wait_output_vsync(pInstance, u8NumVSyncs, u16Timeout, eWindow);
    //return MDrv_XC_wait_output_vsync(u8NumVSyncs, u16Timeout, eWindow);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u8OutputVsyncTime;
}

MS_U8 MApi_XC_WaitOutputVSync(MS_U8 u8NumVSyncs, MS_U16 u16Timeout, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0xFF;
    }

    stXC_SET_WAIT_OUTPUT_VSYNC XCArgs;
    XCArgs.u8NumVSyncs = u8NumVSyncs;
    XCArgs.u16Timeout = u16Timeout;
    XCArgs.eWindow = eWindow;
    XCArgs.u8ReturnValue = 0xFF;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_WAIT_OUTPUT_VSYNC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0xFF;
    }
    else
    {
        return XCArgs.u8ReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// wait for output sync
/// @param  u8NumVSyncs        \b IN: the number of VSync we are going to wait
/// @param  u16Timeout         \b IN: time out
/// @return @ref MS_U8 retun the number of left Vsync if timeout
//-------------------------------------------------------------------------------------------------
MS_U8 MDrv_XC_WaitOutputVSync(void *pInstance, MS_U8 u8NumVSyncs, MS_U16 u16Timeout, SCALER_WIN eWindow)
{
    MS_U8 u8OutputVsyncTime;

    if(MDrv_SC_GetForceWrite(eWindow == SUB_WINDOW ? E_SUB_WINDOW_MUTE : E_MAIN_WINDOW_MUTE))
    {
        u8OutputVsyncTime = 0;
    }
    else
    {
        u8OutputVsyncTime = MDrv_XC_wait_output_vsync(pInstance, u8NumVSyncs, u16Timeout, eWindow);
    }
    return u8OutputVsyncTime;
}

void msAPI_Scaler_Field_Detect(void *pInstance, INPUT_SOURCE_TYPE_t enInputSrcType, EN_VIDEO_SCAN_TYPE enScanType, SCALER_WIN eWindow)
{

    MS_U8 reg_IP1F2_1D, reg_IP1F2_23;
    MS_U16 reg_IP1F2_21;
    MS_U16 u16Vtt = 0;

    // Note:
    // IP1F2_1D[13]: Enable auto no signal filter mode.
    // This functionality is to improve timing detection stability.

    if(IsSrcTypeDigitalVD(enInputSrcType))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "VD \n");
        reg_IP1F2_1D = 0xA1;
        reg_IP1F2_21 = 0x0403;
        reg_IP1F2_23 = 0x30;
    }
    else if(IsSrcTypeDTV(enInputSrcType) || IsSrcTypeStorage(enInputSrcType))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "DTV ");
        if (enScanType == SCAN_INTERLACE)
        {
            reg_IP1F2_1D = 0xA1;
            reg_IP1F2_21 = 0x0403;
            reg_IP1F2_23 = 0x30;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Interlace\n");
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0400;
            reg_IP1F2_23 = 0x00;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Progressive\n");
        }
    }
    else if (IsSrcTypeHDMI(enInputSrcType))
    {

        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "HDMI ");
        if ((enScanType == SCAN_INTERLACE)
            || (MDrv_SC_GetInterlaceInPModeStatus(pInstance, eWindow)
               && (MDrv_XC_GetForceiSupportType(pInstance) == ForceI_SW)))
        {
            //HDMI can get i/p mode info. from header,so we dont have to set i/p mode forecely.
            //reg_IP1F2_21 = 0x0003;      // enable DE -> no field invert, disable DE -> use field invert

            //sw patch: HDMI 720*480i->720*400p , v de will become twice
            //0x20 means that xc ip will detect this timing as i mode if( VTT < (0x20<<4) )
            //it will be more faster to detect i mode
            //But in this case, 720*400p will be treat as i mode, and then v de will become twice
            //So, we decrease the threshold to zero.
            //reg_IP1F2_23 = 0x20;
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0000;      // enable DE -> no field invert, disable DE -> use field invert
            reg_IP1F2_23 = 0x00;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Interlace\n");

            u16Vtt = MDrv_SC_ip_get_verticaltotal(pInstance, eWindow);
            if((u16Vtt/2)%2 )
            {
                reg_IP1F2_21 |= BIT(8);
            }
            else
            {
                reg_IP1F2_21 &= ~BIT(8);
            }
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0000;
            reg_IP1F2_23 = 0x00;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Progressive\n");
        }

        reg_IP1F2_21 |= BIT(12);

        if( (MDrv_XC_GetHdmiSyncMode(pInstance) == HDMI_SYNC_HV)
          &&(enScanType == SCAN_INTERLACE) )
        {
            XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
            UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
            MS_U16 u16Cur_IP1F2_21Val = SC_R2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK01_21_L);
            if( u16Cur_IP1F2_21Val&0xC000 )
            {
                if( reg_IP1F2_21&BIT(8) )
                {
                    reg_IP1F2_21 &= ~BIT(8);
                }
                else
                {
                    reg_IP1F2_21 |= BIT(8);
                }
                //printf("daniel_test patch REG_SC_BK01_21_L: FieldInvert\n");
            }
            else
            {
            }
        }

    }
    else if(IsSrcTypeVga(enInputSrcType))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "DSUB ");
        if (enScanType == SCAN_INTERLACE)
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0103;
            reg_IP1F2_23 = 0x10;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Interlace\n");
        }
        else
        {
            reg_IP1F2_1D = 0x21;
            reg_IP1F2_21 = 0x0000;
            reg_IP1F2_23 = 0x00;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "-> Progressive\n");
        }
    }
    else if (IsSrcTypeCapture(enInputSrcType)
            ||IsSrcTypeSC0DI(enInputSrcType))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "DIP ");
        reg_IP1F2_21 = 0x0101;//IP1 sometimes will detect opcapture to interlace,we force it to progressive
        reg_IP1F2_1D = 0xA1;
        reg_IP1F2_23 = 0x08; //0x20  // Vtt will use "IP1F2_23 * 16" as lower bound.
    }
    else // YPbPr
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "YPbPr ");
        if(MDrv_XC_ip_GetEuroHdtvStatus(pInstance, MAIN_WINDOW) == EURO_AUS_HDTV_NORMAL)
        {
            reg_IP1F2_21 = 0x0100;
        }
        else
        {
            reg_IP1F2_21 = 0x0000;
        }
        reg_IP1F2_1D = 0xA1;
        reg_IP1F2_23 = 0x08; //0x20  // Vtt will use "IP1F2_23 * 16" as lower bound.
    }


    MDrv_SC_setfield(pInstance, reg_IP1F2_1D, reg_IP1F2_21, reg_IP1F2_23, eWindow);

}

//-------------------------------------------------------------------------------------------------
/// Get the specific window's sync status
/// @param  eCurrentSrc             \b IN: the current input source
/// @param  sXC_Sync_Status         \b OUT:store the sync status
/// @param  eWindow                 \b IN: which window we are going to get
//-------------------------------------------------------------------------------------------------
void MApi_XC_GetSyncStatus_U2(void* pInstance, INPUT_SOURCE_TYPE_t eCurrentSrc, XC_IP_SYNC_STATUS *sXC_Sync_Status, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    MDrv_XC_GetSyncStatus(pInstance, eCurrentSrc, sXC_Sync_Status, eWindow);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_GetSyncStatus(INPUT_SOURCE_TYPE_t eCurrentSrc, XC_IP_SYNC_STATUS *sXC_Sync_Status, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_GET_SYNC_STATUS XCArgs;
    XCArgs.eCurrentSrc = eCurrentSrc;
    XCArgs.sXC_Sync_Status = sXC_Sync_Status;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_SYNC_STATUS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
void MDrv_XC_GetSyncStatus(void *pInstance, INPUT_SOURCE_TYPE_t eCurrentSrc, XC_IP_SYNC_STATUS *sXC_Sync_Status, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    E_MUX_INPUTPORT _enPort = MDrv_XC_Mux_GetHDMIPort(pInstance, eCurrentSrc);

    MDrv_XC_ip_get_sync_status(pInstance, sXC_Sync_Status, _enPort , eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

//-------------------------------------------------------------------------------------------------
/// Set input source type
/// @param  enInputSourceType      \b IN: which input source typewe are going to set
/// @param  eWindow                \b IN: which window we are going to set
/// @return void
//-------------------------------------------------------------------------------------------------
void MApi_XC_SetInputSource_U2( void* pInstance, INPUT_SOURCE_TYPE_t enInputSourceType,SCALER_WIN eWindow )
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if( pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[eWindow] == enInputSourceType)
    {
        printf("MApi_XC_SetInputSource[Win %u] return because old src %d -> %d\n"
                , eWindow
                , pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[eWindow], enInputSourceType);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return;
    }
    else
    {
        pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[eWindow] = enInputSourceType;
    }

#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
    MDrv_XC_HDR_SetInputSource(pInstance, enInputSourceType);
#endif
#endif

    MS_U8 u8InputSrcSel, u8SyncSel, u8VideoSel=0, u8isYPbPr, u8HankShakingMD;

    //Disable new mode interlaced detect before set input source
    MDrv_XC_SetNewModeInterlacedDetect(pInstance, DISABLE, 0, DISABLE, FALSE, FALSE, eWindow);

    MDrv_SC_ForceInterlaceInPMode(pInstance, DISABLE,eWindow);
    //IP1 sometimes will detect opcapture to interlace,we force it to progressive
    Hal_SC_ip_set_user_def_interlace_status(pInstance, DISABLE, DISABLE,eWindow);

    _XC_ENTRY(pInstance);

#if SUPPORT_IP_HDMI_FOR_HV_MODE
    // Should disable "pixel repetition" for other source
    if(!(IsSrcTypeHDMI(enInputSourceType) || IsSrcTypeDVI(enInputSourceType)))
    {
        Hal_SC_ip_set_fir_down_sample_divider(pInstance, DISABLE,0x00,eWindow); //Reset Pixel repetition, when switch hv mode
    }
#endif

    if(IsSrcTypeVga(enInputSourceType))
    {
        u8InputSrcSel = IP_ANALOG1;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = FALSE;
        u8HankShakingMD = FALSE;
        MApi_XC_ADC_SwitchSource(pInstance,1); // Force ADC to reset after source switch
    }
    else if(IsSrcTypeYPbPr(enInputSourceType))
    {
        u8InputSrcSel = IP_ANALOG1;
        u8SyncSel = (SYNC_ON_GREEN << 1) | SOG;
        u8isYPbPr = TRUE;
        u8HankShakingMD = FALSE;
        MApi_XC_ADC_SwitchSource(pInstance,1); // Force ADC to reset after source switch
    }
    else if(IsSrcTypeHDMI(enInputSourceType))
    {
#if SUPPORT_IP_HDMI_FOR_HV_MODE
        // set to DE mode to sync with IP_HDMI below, to make system status sync.
        // to fix hdmi 720p-->atv-->720p, then MDrv_XC_Switch_DE_HV_Mode_By_Timing()
        // won't set IP_VIDEO.
        //first time bootup and do 3d conversion, sub window will be set to hdmi also.
        //and sub window will override the HV mode of main. wrong
        //HV mode will be taken as a whole system feature, not main/sub window's
        //beside, we have only one HDMI engine.
        if(!((eWindow == SUB_WINDOW) && (IsSrcTypeHDMI(pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[MAIN_WINDOW]))))
        {
            MDrv_XC_SetHdmiSyncMode(pInstance, HDMI_SYNC_DE);
        }
        u8InputSrcSel = IP_HDMI;
        u8VideoSel = IP_HDMI;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
#else
        if( MDrv_XC_GetHdmiSyncMode(pInstance) == HDMI_SYNC_DE)
        {
            u8InputSrcSel = IP_HDMI;
            u8VideoSel = IP_HDMI;
            u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        }
        else
        {
            u8InputSrcSel = IP_VIDEO;
            u8VideoSel = IP_MST_VD_A;
            u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        }
#endif
        u8isYPbPr = TRUE;
        u8HankShakingMD = FALSE;

    }
    else if(IsSrcTypeDigitalVD(enInputSourceType))
    {
        u8InputSrcSel = IP_VIDEO;
        u8VideoSel = IP_MST_VD_A;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = TRUE;
        u8HankShakingMD = FALSE;
    }
    #ifdef __EXTVD
    else if(IsUseExtVDPort(enInputPortType))
    {
        u8InputSrcSel = IP_VIDEO;
        u8VideoSel = IP_CCIR656_A;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = TRUE;
        u8HankShakingMD = FALSE;
    }
    #endif
    else if(IsSrcTypeDTV(enInputSourceType) || (IsSrcTypeStorage(enInputSourceType)))
    {
        u8InputSrcSel = IP_VIDEO;
        u8VideoSel = IP_MST_VD_A;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = TRUE;
        u8HankShakingMD = TRUE;
    }
    else if(IsSrcTypeCapture(enInputSourceType))
    {
        u8InputSrcSel = IP_DVI; // DIP uses IP_HDTV
        u8VideoSel = IP_CCIR656_A;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = FALSE;
        u8HankShakingMD = FALSE;
        MDrv_SC_ip_set_input_source(pInstance, u8InputSrcSel, u8SyncSel, u8VideoSel, u8isYPbPr, eWindow);
        MDrv_SC_ip_set_handshaking_md(pInstance, u8HankShakingMD, eWindow);
        MDrv_SC_ip_set_input_sync_reference_edge( pInstance, TAILING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
        //IP1 sometimes will detect opcapture to interlace,we force it to progressive
        Hal_SC_ip_set_user_def_interlace_status(pInstance, ENABLE, DISABLE, eWindow);
        //Hal_SC_Enable_VE_BypassPath(pInstance,FALSE);
        gSrcInfo[eWindow].enInputSourceType = enInputSourceType;
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return;
    }
    else if(IsSrcTypeSC0DI(enInputSourceType))
    {
        u8InputSrcSel = IP_DVI; // DIP uses IP_HDTV
        u8VideoSel = IP_CCIR656_A;
        u8SyncSel = (AUTO_DETECT << 1) | CSYNC;
        u8isYPbPr = FALSE;
        u8HankShakingMD = FALSE;
        MDrv_SC_ip_set_input_source(pInstance, u8InputSrcSel, u8SyncSel, u8VideoSel, u8isYPbPr, eWindow);
        MDrv_SC_ip_set_handshaking_md(pInstance, u8HankShakingMD, eWindow);
        MDrv_SC_ip_set_input_sync_reference_edge( pInstance, TAILING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
        //IP1 sometimes will detect opcapture to interlace,we force it to progressive
        Hal_SC_ip_set_user_def_interlace_status(pInstance, ENABLE, DISABLE, eWindow);
        Hal_SC_Enable_VE_BypassPath(pInstance,TRUE);
        enInputSourceType = INPUT_SOURCE_SCALER_OP;
        gSrcInfo[eWindow].enInputSourceType = enInputSourceType;
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return;
    }
    else
    {
        printf("Unknow Input Type \n");
        //main: vga, switch to browser(storage), select sub source to ypbpr
        //sub display garbage. because main's source select is still in vga status, conflict.
        //Thus, we need to set main to a resevered on: IP_HDTV in mapi_video_base::finalize()
        // of MApi_XC_SetInputSource(INPUT_SOURCE_NONE, SDK2DriverScalerWinTypeTrans(m_enDstWin));
        u8InputSrcSel = IP_HDTV; //by CC, treat IP_HDTV as reserved
        u8VideoSel = 0;
        u8SyncSel = 0;
        u8isYPbPr = 0;
        u8HankShakingMD = 0;
        MDrv_SC_ip_set_input_source(pInstance, u8InputSrcSel, u8SyncSel, u8VideoSel, u8isYPbPr, eWindow);
        //IP1 sometimes will detect opcapture to interlace,we force it to progressive
        if( IsSrcTypeCapture(enInputSourceType)
            ||IsSrcTypeSC0DI(enInputSourceType))
        {
            Hal_SC_ip_set_user_def_interlace_status(pInstance, ENABLE, DISABLE, eWindow);
        }
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return;
    }

    MDrv_SC_ip_set_input_source(pInstance, u8InputSrcSel, u8SyncSel, u8VideoSel, u8isYPbPr, eWindow);
    if(IsSrcTypeVga(enInputSourceType))
    {
        MDrv_SC_ip_set_image_wrap(pInstance,  DISABLE, DISABLE, eWindow );
    }
    else
    {
        MDrv_SC_ip_set_image_wrap(pInstance,  ENABLE, ENABLE, eWindow );
    }
    MDrv_SC_ip_set_handshaking_md(pInstance, u8HankShakingMD, eWindow);
    MDrv_SC_ip_set_de_bypass_mode(pInstance, DISABLE, eWindow); // fix HDMI <-> component switch problems

    if(IsSrcTypeVga(enInputSourceType))
    {
        MDrv_SC_ip_set_input_sync_reference_edge(pInstance, LEADING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_input_vsync_delay(pInstance, DELAY_QUARTER_HSYNC, eWindow );
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
        MDrv_ip_set_input_10bit(pInstance, ENABLE, eWindow );

        MDrv_SC_ip_set_coast_input(pInstance, PC_SOURCE, eWindow );
        MDrv_SC_ip_set_ms_filter(pInstance, ENABLE,0,eWindow);
        MDrv_SC_ip_set_coast_window(pInstance, 0x0C, 0x0C, eWindow );
        MDrv_SC_ip_set_input_sync_sample_mode(pInstance, NORMAL_MODE, eWindow );
        MDrv_SC_ip_set_post_glitch_removal(pInstance, DISABLE, 0x00, eWindow );
    }
    else if(IsSrcTypeYPbPr(enInputSourceType))
    {
        MDrv_SC_ip_set_input_sync_reference_edge(pInstance, LEADING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_input_vsync_delay(pInstance, NO_DELAY, eWindow );
        MDrv_SC_ip_set_ms_filter(pInstance, ENABLE,0,eWindow);
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
        MDrv_ip_set_input_10bit(pInstance, ENABLE, eWindow );

        MDrv_SC_ip_set_coast_input(pInstance, COMPONENT_VIDEO, eWindow );
        MDrv_SC_ip_set_coast_window(pInstance, 0x0C, 0x0C, eWindow );
        MDrv_SC_ip_set_input_sync_sample_mode(pInstance, GLITCH_REMOVAL, eWindow );
        MDrv_SC_ip_set_post_glitch_removal(pInstance, ENABLE, 0x01, eWindow );

    #if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        MDrv_XC_mux_SetScalerIndexInformationToADC(pInstance,(psXCInstPri->u32DeviceID),enInputSourceType);
    #endif

    }
    else if(IsSrcTypeHDMI(enInputSourceType))
    {
        MDrv_SC_ip_set_input_sync_reference_edge(pInstance, LEADING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_input_vsync_delay(pInstance, DELAY_QUARTER_HSYNC, eWindow );
#ifdef K3_U2
#else
        Hal_HDMI_Set_YUV422to444_Bypass(TRUE);
#endif

        if( MDrv_XC_GetHdmiSyncMode(pInstance) == HDMI_SYNC_DE)
        {
            MDrv_SC_ip_set_input_vsync_delay(pInstance, DELAY_QUARTER_HSYNC, eWindow );
            MDrv_SC_ip_set_de_only_mode(pInstance, ENABLE, eWindow );
            MDrv_SC_ip_set_de_bypass_mode(pInstance, DISABLE, eWindow);
            MDrv_SC_ip_set_input_sync_sample_mode(pInstance, NORMAL_MODE, eWindow );
            MDrv_SC_ip_set_post_glitch_removal(pInstance, ENABLE, 0x07, eWindow );
#ifdef K3_U2
            Hal_HDMI_Set_YUV422to444_Bypass(TRUE);
#endif
        }
        else
        {
            MDrv_SC_ip_set_input_vsync_delay(pInstance, NO_DELAY, eWindow );
            MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
            // Use DE as capture window
            MDrv_SC_ip_set_de_bypass_mode(pInstance, ENABLE, eWindow);
            // Disable scaling for get correct DE.
            //MDrv_SC_Enable_PreScaling(FALSE, FALSE, eWindow);
            // Set full range capture size for de-bypass mode
            //MDrv_SC_SetFullRangeCapture(eWindow);
            MDrv_SC_ip_set_input_sync_sample_mode(pInstance, GLITCH_REMOVAL, eWindow );
            MDrv_SC_ip_set_post_glitch_removal(pInstance, ENABLE, 0x01, eWindow );
#ifdef K3_U2
            Hal_HDMI_Set_YUV422to444_Bypass(FALSE);
#endif
        }

        MDrv_ip_set_input_10bit(pInstance, ENABLE, eWindow );

        MDrv_SC_ip_set_ms_filter(pInstance, DISABLE,0,eWindow);

        //MDrv_SC_ip_set_coast_input( PC_SOURCE, eWindow );
        MDrv_SC_ip_set_ms_filter(pInstance, DISABLE,0,eWindow);
        //MDrv_SC_ip_set_coast_window( 0x0C, 0x0C, eWindow );
        MDrv_SC_ip_set_input_sync_sample_mode(pInstance, NORMAL_MODE, eWindow );
        MDrv_SC_ip_set_post_glitch_removal(pInstance, ENABLE, 0x07, eWindow );
    }
    else if(IsSrcTypeDigitalVD(enInputSourceType)
    #ifdef __EXTVD
    | IsUseExtVDPort(enInputPortType)
    #endif
    )
    {
        MDrv_SC_ip_set_input_sync_reference_edge(pInstance, TAILING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_input_vsync_delay(pInstance, DELAY_QUARTER_HSYNC, eWindow );
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );

        #ifdef __EXTVD
        if(IsUseExtVDPort(enInputPortType))
        {
            MS_U16 u16tmp;
            MDrv_ip_set_input_10bit(pInstance, DISABLE, eWindow );

            //MDrv_WriteByte(BK_SELECT_00, REG_BANK_IP1F2);
            //MDrv_WriteByte(L_BK_IP1F2(0x08), (MDrv_ReadByte(L_BK_IP1F2(0x08)) & 0x7F));
            //u16tmp = SC_R2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK01_08_L);
            //SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK01_08_L, u16tmp & 0x007F, 0x00FF);
        }
        else
        #endif

        {
            MDrv_ip_set_input_10bit(pInstance, ENABLE, eWindow );
        }
        //MDrv_SC_ip_set_coast_input( PC_SOURCE, eWindow );
        MDrv_SC_ip_set_ms_filter(pInstance, DISABLE,0,eWindow);
        //MDrv_SC_ip_set_coast_window( 0x0C, 0x0C, eWindow );
        MDrv_SC_ip_set_input_sync_sample_mode(pInstance, NORMAL_MODE, eWindow );
        MDrv_SC_ip_set_post_glitch_removal(pInstance, DISABLE, 0x00, eWindow );
    }
    else //DTV
    {
        //set HSYNC reference to leading edge, the capature start will be 1/2 Hsync when input source is from DC0/DC1
        MDrv_SC_ip_set_input_sync_reference_edge(pInstance, LEADING_EDGE, TAILING_EDGE, eWindow );
        MDrv_SC_ip_set_input_vsync_delay(pInstance, DELAY_QUARTER_HSYNC, eWindow );
        MDrv_SC_ip_set_de_only_mode(pInstance, DISABLE, eWindow );
        MDrv_ip_set_input_10bit(pInstance, ENABLE, eWindow );

        //MDrv_SC_ip_set_coast_input( PC_SOURCE, eWindow );
        MDrv_SC_ip_set_ms_filter(pInstance, DISABLE,0,eWindow);
        //MDrv_SC_ip_set_coast_window( 0x0C, 0x0C, eWindow );
        MDrv_SC_ip_set_input_sync_sample_mode(pInstance, NORMAL_MODE, eWindow );
        MDrv_SC_ip_set_post_glitch_removal(pInstance, DISABLE, 0x00, eWindow );
    }

    // set  h0027 [2] : true to enable DE only mode Glitch Protect for position
    // to fix HDMI 720P YUV422 HStart error & HDE lack of 1
    if (IsSrcTypeHDMI(enInputSourceType))
        MDrv_SC_ip_set_DE_Mode_Glitch(pInstance, 0x24 , eWindow ); // VSync glitch removal with line less than 2(DE only)
    else
        MDrv_SC_ip_set_DE_Mode_Glitch(pInstance, 0x04 , eWindow ); // CHANNEL_ATTRIBUTE function control

    msAPI_Scaler_Field_Detect(pInstance, enInputSourceType, SCAN_AUTO, eWindow );

    MDrv_XC_reset_ip(pInstance, eWindow );

    gSrcInfo[eWindow].enInputSourceType = enInputSourceType;

#if 0
    if(g_bIMMESWITCH_DVI_POWERSAVING)
    {
        if((enInputSourceType < INPUT_SOURCE_HDMI || enInputSourceType > INPUT_SOURCE_HDMI4))
            MDrv_DVI_ForceAllPortsEnterPS();
        else
            u8PSDVIStableCount[enInputSourceType - INPUT_SOURCE_HDMI] = 0;
    }
#endif
    /// no delay
    gSrcInfo[eWindow].stStatusnodelay.enInputSourceType = enInputSourceType;
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");




}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetInputSource( INPUT_SOURCE_TYPE_t enInputSourceType,SCALER_WIN eWindow )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_INPUTSOURCE XCArgs;
    XCArgs.enInputSourceType = enInputSourceType;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_INPUTSOURCE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
static MS_BOOL _MDrv_XC_Update_CFDInfo(void *pInstance, INPUT_SOURCE_TYPE_t enInputSourceType, XC_InternalStatus *pSrcInfo, SCALER_WIN eWindow, MS_BOOL bUseYUVSpace)
{

#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)

///Add CFD control
///Update Stream info into CFD
    PQ_INPUT_SOURCE_TYPE enPQSourceType=PQ_INPUT_SOURCE_NONE;
    MS_BOOL bisHDmode=FALSE;

    /// Trans Input Source
    if(IsSrcTypeVga(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_VGA; // VGA
    }
    else if(IsSrcTypeATV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_TV; // TV
    }
    else if(IsSrcTypeDTV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_DTV; //DTV
    }
    else if(IsSrcTypeAV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_CVBS; // AV
    }
    else if(IsSrcTypeScart(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_SCART; // SCART
    }
    else if(IsSrcTypeSV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_SVIDEO; // SV
    }
    else if(IsSrcTypeHDMI(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_HDMI; // HDMI
    }
    else if(IsSrcTypeYPbPr(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_YPBPR; // COMP
    }
    else if(IsSrcTypeJpeg(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_JPEG; // JPEG
    }
    else if(IsSrcTypeStorage(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_STORAGE; // Storage
    }
    else if(IsSrcTypeCapture(enInputSourceType)
            ||IsSrcTypeSC0DI(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_DTV; // Temperarily use DTV
    }
    else if(enInputSourceType == INPUT_SOURCE_KTV)
    {
        enPQSourceType = PQ_INPUT_SOURCE_KTV; // KTV
    }
    else
    {
        MS_ASSERT(0);
    }

    /// Trans is HD mode or not
    if(pSrcInfo->stCapWin.height < 600)
    {
        bisHDmode=FALSE;
    }
    else
    {
        bisHDmode=TRUE;
    }

    /// Send into Kernel CFD
    ST_KDRV_XC_CFD_FIRE stUpdateInfo;
    memset(&stUpdateInfo, 0, sizeof(ST_KDRV_XC_CFD_FIRE));
    stUpdateInfo.u32Version=KDRV_XC_CFD_FIRE_VERSION;
    stUpdateInfo.u16Length=sizeof(ST_KDRV_XC_CFD_FIRE);
    stUpdateInfo.u8Win=eWindow;
    stUpdateInfo.u8InputSource=enPQSourceType;
    stUpdateInfo.bIsRgbBypass=(!bUseYUVSpace);
    stUpdateInfo.bIsHdMode=bisHDmode;

#if (defined MSOS_TYPE_LINUX) || (defined ANDROID)

    ST_KDRV_XC_CFD_CONTROL_INFO stCFDCtrlInfo;
    memset(&stCFDCtrlInfo, 0, sizeof(ST_KDRV_XC_CFD_CONTROL_INFO));
    stCFDCtrlInfo.enCtrlType = E_KDRV_XC_CFD_CTRL_UPDATE_INFO;
    stCFDCtrlInfo.pParam = &stUpdateInfo;
    stCFDCtrlInfo.u32ParamLen = sizeof(ST_KDRV_XC_CFD_FIRE);

    if(0 > _s32FdScaler)
    {
        _s32FdScaler = open("/dev/scaler", O_RDWR);

        if(0 > _s32FdScaler)
        {
            printf("Scaler device not opened!!!!\n");
        }
    }

    if (ioctl(_s32FdScaler, MDRV_XC_IOC_CFDCONCTRL, &stCFDCtrlInfo))
    {
        printf("MDRV_XC_IOC_CFDCONCTRL UPDATE INFO fail!!!!\n");
    }

    if (stCFDCtrlInfo.u16ErrCode != 0)
    {
        printf("MDRV_XC_IOC_CFDCONCTRL UPDATE INFO fail, errCode: %d\n", stCFDCtrlInfo.u16ErrCode);
    }
#endif
#if (defined(MSOS_TYPE_LINUX_KERNEL) && defined(CONFIG_MSTAR_XC_HDR_SUPPORT))

    ST_KDRV_XC_CFD_CONTROL_INFO stCFDCtrlInfo;
    memset(&stCFDCtrlInfo, 0, sizeof(ST_KDRV_XC_CFD_CONTROL_INFO));
    stCFDCtrlInfo.enCtrlType = E_KDRV_XC_CFD_CTRL_UPDATE_INFO;
    stCFDCtrlInfo.pParam = &stUpdateInfo;
    stCFDCtrlInfo.u32ParamLen = sizeof(ST_KDRV_XC_CFD_FIRE);
    MDrv_XC_CFDControl(&stCFDCtrlInfo);
#endif
#endif
#endif
    return TRUE;
}
static void msAPI_Scaler_SetMode(void *pInstance, INPUT_SOURCE_TYPE_t enInputSourceType, XC_InternalStatus *pSrcInfo, SCALER_WIN eWindow )
{
#ifdef UFO_XC_FB_LEVEL
    MS_BOOL bFBL =  pSrcInfo->eFBLevel == E_XC_FB_LEVEL_FBL ? TRUE : FALSE;
#else
    MS_BOOL bFBL = pSrcInfo->bFBL;
#endif

    MS_BOOL bInterlace = pSrcInfo->bInterlace;
    MS_BOOL bUseYUVSpace = FALSE;
    MS_U8 interlace_type = 0;
    XC_INITMISC stXC_Init_Misc;
    memset(&stXC_Init_Misc,0,sizeof(XC_INITMISC));

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    //----------------------------------------------------
    // Setup Memory format
    //----------------------------------------------------
    if(pXCResourcePrivate->stdrvXC_MVideo.s_eMemFmt[eWindow] == E_MS_XC_MEM_FMT_AUTO)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "E_MS_XC_MEM_FMT_AUTO\n");
        if(!bInterlace &&
          ((IsSrcTypeHDMI(enInputSourceType) && !pXCResourcePrivate->stdrv_HDMI.enStatus_info.b_St_HDMI_Mode) || // DVI plugged into HDMI connector
          IsSrcTypeVga(enInputSourceType)  ) )
        {   // always keep memory foramt to 444 for corresponding to Y/C post-scaling filter
            pSrcInfo->bMemFmt422 = FALSE;
        }
        else
        {   // if input source is 422, HW will convert to 444 automatically
            pSrcInfo->bMemFmt422 = TRUE;
        }
    }
    else
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "E_MS_XC_MEM_FMT_422\n");
        if(pXCResourcePrivate->stdrvXC_MVideo.s_eMemFmt[eWindow] == E_MS_XC_MEM_FMT_422)
            pSrcInfo->bMemFmt422 = TRUE;
        else
            pSrcInfo->bMemFmt422 = FALSE;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR); //K3_PQ

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MemFmt422=%u\n", (MS_U16)pSrcInfo->bMemFmt422);
#if ((HW_DESIGN_4K2K_VER == 4)||(HW_DESIGN_4K2K_VER == 6)||(HW_DESIGN_4K2K_VER == 7)||(HW_DESIGN_4K2K_VER == 1))
#if defined(UFO_XC_SET_DSINFO_V0)
    if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
#else
    if(!MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
#endif
    {
#ifdef ENABLE_TV_SC2_PQ
        if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc == NULL)
                bUseYUVSpace = 0;
            else
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                bUseYUVSpace = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW,
                                             pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
#else
                bUseYUVSpace = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc(eWindow == SUB_WINDOW ? PQ_SUB_WINDOW : PQ_MAIN_WINDOW,
                                             pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
#endif
        }
#ifdef ENABLE_TV_SC2_PQ
        else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc_ex == NULL)
                bUseYUVSpace = 0;
            else
                bUseYUVSpace = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc_ex(psXCInstPri->u32DeviceID, eWindow == SUB_WINDOW ? PQ_SUB_WINDOW : PQ_MAIN_WINDOW,
                                              pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
        }
#endif

#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
        if ((pXCResourcePrivate->stdrvXC_Display._bEnableHDR == ENABLE) && IsSrcTypeHDMI(enInputSourceType))
        {
            // since Dolby HDR will fake as RGB but actually YUV content, we have to set our color space as YUV.
            pSrcInfo->bUseYUVSpace = TRUE;
        }
        else
#endif
#endif
        {
            if (bUseYUVSpace)
            {
                pSrcInfo->bUseYUVSpace = TRUE;
             }
            else
            {
                pSrcInfo->bUseYUVSpace = FALSE;
            }
        }

#ifdef ENABLE_TV_SC2_PQ
        if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt == NULL)
                pSrcInfo->bMemYUVFmt =  TRUE;
            else
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                pSrcInfo->bMemYUVFmt = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW,
                                          pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
#else
                pSrcInfo->bMemYUVFmt = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt(eWindow == SUB_WINDOW ? PQ_SUB_WINDOW : PQ_MAIN_WINDOW,
                                          pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
#endif
        }
#ifdef ENABLE_TV_SC2_PQ
        else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt_ex == NULL)
                pSrcInfo->bMemYUVFmt =  TRUE;
            else
                pSrcInfo->bMemYUVFmt = s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt_ex(psXCInstPri->u32DeviceID, eWindow == SUB_WINDOW ? PQ_SUB_WINDOW : PQ_MAIN_WINDOW,
                                          pSrcInfo->bForceRGBin == ENABLE ? PQ_FOURCE_COLOR_RGB : PQ_FOURCE_COLOR_DEFAULT);
        }
#endif

        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "bMemYUVFmt=%u, RGBin=%u bFBL=%u\n", pSrcInfo->bMemYUVFmt, pSrcInfo->bForceRGBin, pSrcInfo->bFBL);

        if((pSrcInfo->bMemYUVFmt != TRUE) && (eWindow == SUB_WINDOW)) //IP_CSC output is RGB, so skip 444 to 422
        {
            pSrcInfo->bMemFmt422 = FALSE;//Now, only care about Sub case to reduce side effect
        }

        MDrv_XC_GetMISCStatus(pInstance, &stXC_Init_Misc);
        if (stXC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_PQ_SKIP_PCMODE_NEWFLOW)
        {
            //if(HDMI/DVI RGB) set to 444
            if(!pSrcInfo->bMemYUVFmt && ( IsSrcTypeHDMI(enInputSourceType) || IsSrcTypeDVI(enInputSourceType) ))
            {
                pSrcInfo->bMemFmt422 = FALSE;
            }
        }
        //-------------------------------------------
        // Set De-interlaced mode and Memory format
        //-------------------------------------------
        //printf("==> Set PQ MEMORY Mode: %d\n", eWindow);

#ifdef K3_U2
        // Tempararily not dumping MADi PQ table in seamless zapping
        if((Hal_SC_GetSeamlessZappingStatus(pInstance, eWindow)) && (Hal_SC_get_freezeimg(pInstance, eWindow)))
        {
            Hal_SC_SetPX2MemFormat(pInstance, eWindow);
            interlace_type = PQ_DEINT_3DDI_HISTORY;
            pSrcInfo->u8BitPerPixel = 24;

            //PQ_IP_MemFormat_Main 422MF
            HAL_SC_set_memory_bit_fmt(pInstance,FALSE,FALSE);
            //444 to 422 (PQ_IP_444To422_Main) (on)
            HAL_SC_ip_set_444to422_filter_mod(pInstance,TRUE);
            //422To444(PQ_IP_422To444_Main on)
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_0C_L, 0x80 ,0x0080);
        }
        else
        {
            if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID) // SC0
            {
                if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat == NULL)
                    interlace_type = 0;
                else
                    interlace_type = (MS_U8) s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat(
                        eWindow == SUB_WINDOW ?
                        PQ_SUB_WINDOW :
                        PQ_MAIN_WINDOW,
                        pSrcInfo->bMemFmt422,
                        bFBL,
                        &(pSrcInfo->u8BitPerPixel));
            }
            else // SC1
            {
                Hal_SC_SetPX2MemFormat(pInstance, eWindow);
                interlace_type = PQ_DEINT_3DDI_HISTORY;
                pSrcInfo->u8BitPerPixel = 24;
            }
        }
#else
#ifdef ENABLE_TV_SC2_PQ
        if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat == NULL)
            {
#if (HW_DESIGN_4K2K_VER == 7)
                // Temperarily add for SC1 to load MADi settings
                if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
                {
                    interlace_type = Hal_SC1_LoadMADi(pInstance, eWindow, &(pSrcInfo->u8BitPerPixel));
                }
                else
#endif
                {
                    interlace_type = 0;
                }
            }
            else
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                interlace_type = (MS_U8) s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat(
                    (psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW,
                    pSrcInfo->bMemFmt422,
                    bFBL,
                    &(pSrcInfo->u8BitPerPixel));
#else
                interlace_type = (MS_U8) s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat(
                    eWindow == SUB_WINDOW ?
                    PQ_SUB_WINDOW :
                    PQ_MAIN_WINDOW,
                    pSrcInfo->bMemFmt422,
                    bFBL,
                    &(pSrcInfo->u8BitPerPixel));
#endif
        }
#ifdef ENABLE_TV_SC2_PQ
        else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
        {
            if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat_ex == NULL)
                interlace_type = 0;
            else
                interlace_type = (MS_U8) s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat_ex(psXCInstPri->u32DeviceID,
                    eWindow == SUB_WINDOW ?
                    PQ_SUB_WINDOW :
                    PQ_MAIN_WINDOW,
                    pSrcInfo->bMemFmt422,
                    bFBL,
                    &(pSrcInfo->u8BitPerPixel));
        }
#endif
    }
#endif
#if SUPPORT_SEAMLESS_ZAPPING
    Hal_SC_SetPX2MemFormat(pInstance, eWindow);
    if (IsVMirrorMode(eWindow))
    {
        MS_U16 u16IPM3DLRFlags = SC_R2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_30_L);
        // 2D mode
        if(MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_MODE_NONE || MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow) == E_XC_3D_INPUT_MODE_NONE)
        {
            if(Hal_SC_IsPX2MemFormat(pInstance, eWindow))
            {
                if((u16IPM3DLRFlags&BIT(4)) == 0)
                {
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_30_L, BIT(4), BIT(4));//F2 IPM 3D LR invert
                }
            }
            else
            {
                if((u16IPM3DLRFlags&BIT(4)) == BIT(4))
                {
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_30_L, 0, BIT(4));//F2 IPM 3D LR invert
                }
            }
        }
    }
#endif
#endif

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR); //K3_PQ
#if ((HW_DESIGN_4K2K_VER == 4)||(HW_DESIGN_4K2K_VER == 6)||(HW_DESIGN_4K2K_VER == 7)||(HW_DESIGN_4K2K_VER == 1))
#if defined(UFO_XC_SET_DSINFO_V0)
    if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
#else
    if(!MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
#endif
    {
        pSrcInfo->eDeInterlaceMode = (MS_DEINTERLACE_MODE)interlace_type;
    }
#endif
    //printf("DI mode =%x, bitPerPixel=%d\n",pSrcInfo->eDeInterlaceMode, pSrcInfo->u8BitPerPixel);
    if(bFBL && (pSrcInfo->stDispWin.height != pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height))
    {
        if(SUPPORT_SPECIAL_FRAMELOCK == FALSE)
        {
            MDrv_SC_set_std_display_window(pInstance, eWindow);  //only support full screen
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "[FBL]Set to std_display_window(panel size, full screen) \n");
        }
    }

    _XC_ENTRY(pInstance);

    // set field shift line  // set shift line between even/odd field
    MDrv_SC_set_shift_line(pInstance, pSrcInfo->bFBL, pSrcInfo->eDeInterlaceMode, eWindow);

    //------------------------------------
    // Setup Delay line
    //------------------------------------
    /* move to MDrv_SC_Set_LockFreeze_Point()
    if(bFBL)
    {
        pSrcInfo->u8DelayLines = 1;
        //printf("[FBL]Set delay line\n");
    }
    else if( (pSrcInfo->eDeInterlaceMode == MS_DEINT_2DDI_AVG) ||
             (pSrcInfo->eDeInterlaceMode == MS_DEINT_2DDI_BOB))
    {
        pSrcInfo->u8DelayLines = 1;    // 5
    }
    else
    {
        pSrcInfo->u8DelayLines = 2;
    }

    // Setup delay line
    Hal_SC_set_delayline( pSrcInfo->u8DelayLines, eWindow );           // set delay line for trigger point, for 2DDI
    */
#ifdef UFO_XC_SET_DSINFO_V0
    if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
    {
        msAPI_Scaler_Field_Detect(pInstance, enInputSourceType, (pSrcInfo->bInterlace?SCAN_INTERLACE:SCAN_PROGRESSIVE), eWindow );
    }
#else
    msAPI_Scaler_Field_Detect(pInstance, enInputSourceType, (pSrcInfo->bInterlace?SCAN_INTERLACE:SCAN_PROGRESSIVE), eWindow );
#endif

    //--------------
    // PIP related
    //--------------
    if(PIP_SUPPORTED)
    {
        if(eWindow == MAIN_WINDOW)
        {
            if(pSrcInfo->bInterlace == TRUE)
            {
                SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK20_11_L, BIT(13), BIT(13));
            }
            else
            {
#if SUPPORT_SEAMLESS_ZAPPING
                // If memory format is PX2 progressive mode, set to use interlace line buffer setting
                if(Hal_SC_IsPX2MemFormat(pInstance, eWindow))
                {
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK20_11_L, BIT(13), BIT(13));
                }
                else
#endif
                {
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK20_11_L, 0, BIT(13));
                }
            }
        }
    }

    //Update CFD info into CFD driver
    _MDrv_XC_Update_CFDInfo(pInstance,enInputSourceType,pSrcInfo,eWindow,bUseYUVSpace);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
}

//This function is used in internal (as driver), so do not set _XC_SEMAPHORE_ENTRY
void _MDrv_XC_Set_PQ_SourceData(void *pInstance,
    SCALER_WIN eWindow,
    INPUT_SOURCE_TYPE_t enInputSourceType,
    XC_InternalStatus *pstXC_WinTime_Info)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    PQ_WIN ePQWin;
    PQ_INPUT_SOURCE_TYPE enPQSourceType=PQ_INPUT_SOURCE_NONE;
    MS_PQ_Mode_Info stPQModeInfo;

    switch (eWindow)
    {
    default:
    case MAIN_WINDOW:
        ePQWin = PQ_MAIN_WINDOW;
        break;

    case SUB_WINDOW:
        ePQWin = PQ_SUB_WINDOW;
        break;
    }

    if(IsSrcTypeVga(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_VGA; // VGA
    }
    else if(IsSrcTypeATV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_TV; // TV
    }
    else if(IsSrcTypeDTV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_DTV; //DTV
    }
    else if(IsSrcTypeAV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_CVBS; // AV
    }
    else if(IsSrcTypeScart(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_SCART; // SCART
    }
    else if(IsSrcTypeSV(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_SVIDEO; // SV
    }
    else if(IsSrcTypeHDMI(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_HDMI; // HDMI
    }
    else if(IsSrcTypeYPbPr(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_YPBPR; // COMP
    }
    else if(IsSrcTypeJpeg(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_JPEG; // JPEG
    }
    else if(IsSrcTypeStorage(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_STORAGE; // Storage
    }
    else if(IsSrcTypeCapture(enInputSourceType)
            ||IsSrcTypeSC0DI(enInputSourceType))
    {
        enPQSourceType = PQ_INPUT_SOURCE_DTV; // Temperarily use DTV
    }
    else if(enInputSourceType == INPUT_SOURCE_KTV)
    {
        enPQSourceType = PQ_INPUT_SOURCE_KTV; // KTV
    }
    else
    {
        MS_ASSERT(0);
    }

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "PQSetSource:Input src=%u, win=%u,\n", enInputSourceType, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "PQSetSource:   PQ src=%u, win=%u\n", enPQSourceType, eWindow);

    stPQModeInfo.bFBL               = pstXC_WinTime_Info->bFBL;
    stPQModeInfo.bInterlace         = pstXC_WinTime_Info->bInterlace;
    stPQModeInfo.u16input_hsize     = pstXC_WinTime_Info->stCapWin.width;
    stPQModeInfo.u16input_vsize     = pstXC_WinTime_Info->stCapWin.height;
    stPQModeInfo.u16input_vfreq     = pstXC_WinTime_Info->u16InputVFreq;
    stPQModeInfo.u16input_vtotal    = pstXC_WinTime_Info->u16InputVTotal;
    stPQModeInfo.u16ouput_vfreq     = pstXC_WinTime_Info->u16InputVFreq;
    stPQModeInfo.u16display_hsize   = pstXC_WinTime_Info->stDispWin.width;
    stPQModeInfo.u16display_vsize   = pstXC_WinTime_Info->stDispWin.height;
    stPQModeInfo.u16cropwin_hsize   = pstXC_WinTime_Info->stCropWin.width;
    stPQModeInfo.u16cropwin_vsize   = pstXC_WinTime_Info->stCropWin.height;

#ifdef ENABLE_TV_SC2_PQ
    if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
    {
#ifdef UFO_XC_SET_DSINFO_V0
        if ((s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo != NULL) && (!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD))
#else
        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo != NULL)
#endif
        {
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW, enPQSourceType, &stPQModeInfo);
#else
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo(ePQWin, enPQSourceType, &stPQModeInfo);
#endif
        }
    }
#ifdef ENABLE_TV_SC2_PQ
    else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
    {
#ifdef UFO_XC_SET_DSINFO_V0
        if ((s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo_ex != NULL) && (!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD))
#else
        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo_ex != NULL)
#endif
        {
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo_ex(psXCInstPri->u32DeviceID, ePQWin, enPQSourceType, &stPQModeInfo);
        }
    }
#endif

#ifdef ENABLE_TV_SC2_PQ
    if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
    {
#ifdef UFO_XC_SET_DSINFO_V0
        if ((s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype != NULL) && (!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD))
#else
        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype != NULL)
#endif
        {
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW, enPQSourceType);
            UNUSED(ePQWin);
#else
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype(ePQWin, enPQSourceType);
#endif
        }
    }
#ifdef ENABLE_TV_SC2_PQ
    else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
    {
#ifdef UFO_XC_SET_DSINFO_V0
        if ((s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype_ex != NULL) && (!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD))
#else
        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype_ex != NULL)
#endif
        {
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype_ex(psXCInstPri->u32DeviceID, ePQWin, enPQSourceType);
        }
    }
#endif

}

///////////////////////////////////////////////////
/// ADC API
///////////////////////////////////////////////////
void MDrv_XC_ADC_GetPQADCSamplingInfo(void *pInstance, INPUT_SOURCE_TYPE_t enSourceType, XC_InternalStatus *pSrcInfo, PQ_ADC_SAMPLING_INFO *pstPQADCSamplingInfo, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    //MS_U16 u16Width;//, u16RealWidth;
    if ( IsSrcTypeVga(enSourceType) || IsSrcTypeYPbPr(enSourceType) )
    {
        if(pSrcInfo->bHDuplicate)
        {
#ifdef ENABLE_TV_SC2_PQ
            if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
            {
                if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl)
                {
                    pstPQADCSamplingInfo->enPQSourceType = IsSrcTypeVga(enSourceType) ? PQ_INPUT_SOURCE_VGA : PQ_INPUT_SOURCE_YPBPR;
                    pstPQADCSamplingInfo->stPQModeInfo.bFBL               = pSrcInfo->bFBL;
                    pstPQADCSamplingInfo->stPQModeInfo.bInterlace         = pSrcInfo->bInterlace;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_hsize     = pSrcInfo->stCapWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vsize     = pSrcInfo->stCapWin.height;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vfreq     = pSrcInfo->u16InputVFreq;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vtotal    = pSrcInfo->u16InputVTotal;
                    pstPQADCSamplingInfo->stPQModeInfo.u16ouput_vfreq     = pSrcInfo->u16InputVFreq;
                    pstPQADCSamplingInfo->stPQModeInfo.u16display_hsize   = pSrcInfo->stDispWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16display_vsize   = pSrcInfo->stDispWin.height;
                    pstPQADCSamplingInfo->stPQModeInfo.u16cropwin_hsize   = pSrcInfo->stCropWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16cropwin_vsize   = pSrcInfo->stCropWin.height;

#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                    if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW, E_PQ_IOCTL_ADC_SAMPLING, (void *)pstPQADCSamplingInfo, sizeof(PQ_ADC_SAMPLING_INFO)) == FALSE)
#else
                    if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl((PQ_WIN)eWindow, E_PQ_IOCTL_ADC_SAMPLING, (void *)pstPQADCSamplingInfo, sizeof(PQ_ADC_SAMPLING_INFO)) == FALSE)
#endif
                    {
                        pstPQADCSamplingInfo->u16ratio = 2;
                    }
                }
                else
                {
                    pstPQADCSamplingInfo->u16ratio = 2;
                }
            }
#ifdef ENABLE_TV_SC2_PQ
            else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
            {
                if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl_ex)
                {
                    pstPQADCSamplingInfo->enPQSourceType = IsSrcTypeVga(enSourceType) ? PQ_INPUT_SOURCE_VGA : PQ_INPUT_SOURCE_YPBPR;
                    pstPQADCSamplingInfo->stPQModeInfo.bFBL               = pSrcInfo->bFBL;
                    pstPQADCSamplingInfo->stPQModeInfo.bInterlace         = pSrcInfo->bInterlace;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_hsize     = pSrcInfo->stCapWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vsize     = pSrcInfo->stCapWin.height;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vfreq     = pSrcInfo->u16InputVFreq;
                    pstPQADCSamplingInfo->stPQModeInfo.u16input_vtotal    = pSrcInfo->u16InputVTotal;
                    pstPQADCSamplingInfo->stPQModeInfo.u16ouput_vfreq     = pSrcInfo->u16InputVFreq;
                    pstPQADCSamplingInfo->stPQModeInfo.u16display_hsize   = pSrcInfo->stDispWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16display_vsize   = pSrcInfo->stDispWin.height;
                    pstPQADCSamplingInfo->stPQModeInfo.u16cropwin_hsize   = pSrcInfo->stCropWin.width;
                    pstPQADCSamplingInfo->stPQModeInfo.u16cropwin_vsize   = pSrcInfo->stCropWin.height;

                    if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl_ex(psXCInstPri->u32DeviceID, (PQ_WIN)eWindow, E_PQ_IOCTL_ADC_SAMPLING, (void *)&stPQADCSamplingInfo, sizeof(PQ_ADC_SAMPLING_INFO)) == FALSE)
                    {
                        pstPQADCSamplingInfo->u16ratio = 2;
                    }
                }
                else
                {
                    pstPQADCSamplingInfo->u16ratio = 2;
                }
            }
#endif
            //u16RealWidth = u16Width / pstPQADCSamplingInfo->u16ratio;
        }
    }
}

static void _MDrv_XC_ADC_SetMode(void *pInstance, INPUT_SOURCE_TYPE_t enSourceType, XC_InternalStatus *pSrcInfo, PQ_ADC_SAMPLING_INFO *pstPQADCSamplingInfo, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    //MS_U16 u16Width;//, u16RealWidth;
    MS_U16 u16PixClk = 0, u16RealPixClk = 0; // Add the initial value
    MS_U16 u16HorizontalTotal;
    XC_IP_SYNC_STATUS sXC_Sync_Status;


    if ( IsSrcTypeVga(enSourceType) || IsSrcTypeYPbPr(enSourceType) )
    {
        // get sync status
        _XC_ENTRY(pInstance);
        MDrv_XC_GetSyncStatus(pInstance, enSourceType, &sXC_Sync_Status, eWindow);
        _XC_RETURN(pInstance);

        // Set Hsync polarity
        MDrv_XC_ADC_hpolarity_setting(pInstance, sXC_Sync_Status.u8SyncStatus & XC_MD_HSYNC_POR_BIT);

        // Set user default phase
        MDrv_XC_ADC_SetPhaseEx(pInstance, pSrcInfo->u16DefaultPhase); // setting ADC phase

        //Sog Filter behavior is before the ADC double sampling, so for Sog Filter, we don't need to use
        //double horizontal total for the bandwidth calculation.
        _XC_ENTRY(pInstance);
        u16RealPixClk = MDrv_XC_cal_pixclk(pInstance, pSrcInfo->u16DefaultHtotal, eWindow);
        _XC_RETURN(pInstance);

        MDrv_XC_ADC_Reload(pInstance, enSourceType, u16RealPixClk, pSrcInfo->u16DefaultHtotal, pstPQADCSamplingInfo->u16ratio);

        // Get Htt
        if(pSrcInfo->bHDuplicate)
        {
            u16HorizontalTotal = pSrcInfo->u16DefaultHtotal * pstPQADCSamplingInfo->u16ratio; // for better quality
        }
        else
        {
            u16HorizontalTotal = pSrcInfo->u16DefaultHtotal;
        }

        // Calculate pixel clock, the unit is MHz
        _XC_ENTRY(pInstance);
        u16PixClk = MDrv_XC_cal_pixclk(pInstance, u16HorizontalTotal, eWindow);
        _XC_RETURN(pInstance);

        if (IsSrcTypeVga(enSourceType))
        {
            // ADC reset, Patch PLL unlock problem.
            MDrv_XC_ADC_reset(pInstance, REST_ADC);
        }
        else
        {
            MDrv_XC_ADC_clamp_placement_setting(pInstance, u16PixClk);
        }

        MDrv_XC_ADC_vco_ctrl(pInstance, IsSrcTypeHDMI(enSourceType), u16PixClk);
    }
    else
    {
        MDrv_XC_ADC_Reload(pInstance, enSourceType, u16RealPixClk, pSrcInfo->u16DefaultHtotal, pstPQADCSamplingInfo->u16ratio);
    }
}

void MDrv_XC_ADC_SetMode(void *pInstance, INPUT_SOURCE_TYPE_t enSourceType, XC_InternalStatus *pSrcInfo, PQ_ADC_SAMPLING_INFO *pstPQADCSamplingInfo, SCALER_WIN eWindow)
{
    PQ_ADC_SAMPLING_INFO stPQADCSamplingInfo;

    // Give a initial setting.
    memset(&stPQADCSamplingInfo,0,sizeof(PQ_ADC_SAMPLING_INFO));
    if ( IsSrcTypeVga(enSourceType) || IsSrcTypeYPbPr(enSourceType) )
    {
        if(pSrcInfo->bHDuplicate)
        {
            stPQADCSamplingInfo = *pstPQADCSamplingInfo;

        }
        else
        {
            stPQADCSamplingInfo.u16ratio = 1;
        }
    }
    _MDrv_XC_ADC_SetMode(pInstance, enSourceType, pSrcInfo, &stPQADCSamplingInfo, eWindow);
}

void MApi_XC_ADC_SwitchSource(void* pInstance, MS_BOOL bUpdateStatus)
{
    MDrv_ADC_SourceSwitch(pInstance,bUpdateStatus);
}

static MS_BOOL _Mdrv_XC_CopySetWinInfo(void *pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    if(pstXC_SetWin_Info == NULL)
    {
        return FALSE;
    }
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    gSrcInfo[eWindow].enInputSourceType     = pstXC_SetWin_Info->enInputSourceType;

    gSrcInfo[eWindow].stCapWin.x            = pstXC_SetWin_Info->stCapWin.x;
    gSrcInfo[eWindow].stCapWin.y            = pstXC_SetWin_Info->stCapWin.y;
    gSrcInfo[eWindow].stCapWin.width        = pstXC_SetWin_Info->stCapWin.width;
    gSrcInfo[eWindow].stCapWin.height       = pstXC_SetWin_Info->stCapWin.height;

    gSrcInfo[eWindow].stDispWin.x           = pstXC_SetWin_Info->stDispWin.x;
    gSrcInfo[eWindow].stDispWin.y           = pstXC_SetWin_Info->stDispWin.y;
    gSrcInfo[eWindow].stDispWin.width       = pstXC_SetWin_Info->stDispWin.width;
    gSrcInfo[eWindow].stDispWin.height      = pstXC_SetWin_Info->stDispWin.height;

    gSrcInfo[eWindow].stCropWin.x           = pstXC_SetWin_Info->stCropWin.x;
    gSrcInfo[eWindow].stCropWin.y           = pstXC_SetWin_Info->stCropWin.y;
    gSrcInfo[eWindow].stCropWin.width       = pstXC_SetWin_Info->stCropWin.width;
    gSrcInfo[eWindow].stCropWin.height      = pstXC_SetWin_Info->stCropWin.height;

    gSrcInfo[eWindow].bInterlace            = pstXC_SetWin_Info->bInterlace;
    gSrcInfo[eWindow].bHDuplicate           = pstXC_SetWin_Info->bHDuplicate;
    gSrcInfo[eWindow].u16InputVFreq         = pstXC_SetWin_Info->u16InputVFreq;
    gSrcInfo[eWindow].u16InputVTotal        = pstXC_SetWin_Info->u16InputVTotal;
    gSrcInfo[eWindow].u16DefaultHtotal      = pstXC_SetWin_Info->u16DefaultHtotal;
    if(pstXC_SetWin_Info->u16DefaultPhase != 0)
    {
        gSrcInfo[eWindow].u16DefaultPhase       = pstXC_SetWin_Info->u16DefaultPhase;
    }
    else
    {
        gSrcInfo[eWindow].u16DefaultPhase       = pstXC_SetWin_Info->u8DefaultPhase;//compatible for old AP code
    }
    gSrcInfo[eWindow].bHCusScaling          = pstXC_SetWin_Info->bHCusScaling;
    gSrcInfo[eWindow].u16HCusScalingSrc     = pstXC_SetWin_Info->u16HCusScalingSrc;
    gSrcInfo[eWindow].u16HCusScalingDst     = pstXC_SetWin_Info->u16HCusScalingDst;
    gSrcInfo[eWindow].bVCusScaling          = pstXC_SetWin_Info->bVCusScaling;
    gSrcInfo[eWindow].u16VCusScalingSrc     = pstXC_SetWin_Info->u16VCusScalingSrc;
    gSrcInfo[eWindow].u16VCusScalingDst     = pstXC_SetWin_Info->u16VCusScalingDst;
    gSrcInfo[eWindow].bDisplayNineLattice   = pstXC_SetWin_Info->bDisplayNineLattice;

    return TRUE;
}

static void _MDrv_XC_CheckCropWin(void *pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    if(pstXC_SetWin_Info == NULL)
    {
        printf("Invalid Crop Window access\n");
        return;
    }

    if(pstXC_SetWin_Info->stCropWin.width > pstXC_SetWin_Info->stCapWin.width)
    {
        pstXC_SetWin_Info->stCropWin.width = pstXC_SetWin_Info->stCapWin.width;
    }

    if(pstXC_SetWin_Info->stCropWin.height > pstXC_SetWin_Info->stCapWin.height)
    {
        pstXC_SetWin_Info->stCropWin.height = pstXC_SetWin_Info->stCapWin.height;
    }

    if(pstXC_SetWin_Info->stCropWin.x > (pstXC_SetWin_Info->stCapWin.width - pstXC_SetWin_Info->stCropWin.width))
    {
        pstXC_SetWin_Info->stCropWin.x = pstXC_SetWin_Info->stCapWin.width - pstXC_SetWin_Info->stCropWin.width;
    }
    if(pstXC_SetWin_Info->stCropWin.y > (pstXC_SetWin_Info->stCapWin.height - pstXC_SetWin_Info->stCropWin.height))
    {
        pstXC_SetWin_Info->stCropWin.y = pstXC_SetWin_Info->stCapWin.height - pstXC_SetWin_Info->stCropWin.height;
    }
}

static void _MDrv_XC_CheckDisplayWin(void *pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    if(eWindow >= MAX_WINDOW)
    {
        printf("Invalid window!\n");
    }
    if(pstXC_SetWin_Info == NULL)
    {
        printf("Invalid Display Window access\n");
        return;
    }

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

#if (XC_SUPPORT_2P_MODE == TRUE)
    if (psXCInstPri->u32DeviceID == E_XC_DEVICE1)
    {
        // sc1 composer sc0 need use 2p in hstart and width
        pstXC_SetWin_Info->stDispWin.x = ((pstXC_SetWin_Info->stDispWin.x + 1) & ~0x01);
        pstXC_SetWin_Info->stDispWin.width = (pstXC_SetWin_Info->stDispWin.width  & ~0x01);
    }
#endif

    if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID && XC_DEVICE1_IS_INTERLACE_OUT)
    {
        pstXC_SetWin_Info->stDispWin.y /= 2;
        pstXC_SetWin_Info->stDispWin.height /= 2;
    }

    // TGEN does not allow display window height <= 5 or it cannot decide VDE_End
    if (pstXC_SetWin_Info->stDispWin.height <= 5)
    {
        pstXC_SetWin_Info->stDispWin.height = 5;
    }


    if ((pstXC_SetWin_Info->stDispWin.x
           + pstXC_SetWin_Info->stDispWin.width
           + pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart)
         > pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal)
    {
        pstXC_SetWin_Info->stDispWin.width = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal
                                             - pstXC_SetWin_Info->stDispWin.x
                                             - pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart;
        printf("Display Window size is exceeded Htotal, display window horizontal end is limited to Htotal\n");
    }

    if ((pstXC_SetWin_Info->stDispWin.y
           + pstXC_SetWin_Info->stDispWin.height
           + pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart)
        > pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VTotal)
    {
        pstXC_SetWin_Info->stDispWin.height = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VTotal
                                             - pstXC_SetWin_Info->stDispWin.y
                                             - pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart;
        printf("Display Window size is exceeded Vtotal, display window vertical end is limited to Vtotal\n");
    }

    //patch for monaco,monet 3D SC HW limitation, if input is SBS,PA, CB, capture window's width must be align by 4
    if((E_XC_3D_INPUT_SIDE_BY_SIDE_HALF == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW))
        || (E_XC_3D_INPUT_PIXEL_ALTERNATIVE == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW))
        || (E_XC_3D_INPUT_CHECK_BORAD == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW)))
    {
        pstXC_SetWin_Info->stCapWin.width &= ~0x03;
    }

    // do OverScan to avoid black border is shown
    if ((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN)
    {
        pstXC_SetWin_Info->stDispWin.x -= pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange/2;
        pstXC_SetWin_Info->stDispWin.y -= pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange/2;
        pstXC_SetWin_Info->stDispWin.width += pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange;
        pstXC_SetWin_Info->stDispWin.height += pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange;
    }

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    // FRC 3D enable: SC transforms to TB, FRC transforms to FA/LA, 3D mode is FA/LA in SDK.
    if(MHal_FRC_IsEnableFRC3D(pInstance,MAIN_WINDOW))
    {
        if((MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_TOP_BOTTOM)
            &&(pXCResourcePrivate->stdrvXC_3D._bLAToTB))
        {
            pstXC_SetWin_Info->stDispWin.y /= 2;
        }
    }
#endif
}

static void _MDrv_XC_CheckCaptureWin(void *pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    MS_BOOL bFramePacking = FALSE;
    if(pstXC_SetWin_Info == NULL)
    {
        printf("Invalid Capture Window access\n");
        return;
    }

#if (XC_SUPPORT_2P_MODE == TRUE)
    // 2p mode need width as even
    pstXC_SetWin_Info->stCapWin.width = ((pstXC_SetWin_Info->stCapWin.width + 1) & ~0x01);
#endif

    if(pstXC_SetWin_Info->bInterlace)
    {
        //The component 1080i can't frame lock when capture start is odd.
        {
            // the Capture start should be even
            if(pstXC_SetWin_Info->stCapWin.y%2)
            {
                pstXC_SetWin_Info->stCapWin.y += 1;
                pstXC_SetWin_Info->stCapWin.height -= 1; //Y+1, so height-1 to keep not capture outside
            }
        }

        // the Capture height should be even
        if(pstXC_SetWin_Info->stCapWin.height%2)
        {
            pstXC_SetWin_Info->stCapWin.height -= 1;
        }
    }

    //patch for monaco 3D SC HW limitation, if input is SBS, 2D_HW, PA, CB, capture window's width must be align by 4
    #if(HW_DESIGN_3D_VER == 5) //monaco mode
        if((E_XC_3D_INPUT_SIDE_BY_SIDE_HALF == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW))
            || (E_XC_3D_INPUT_PIXEL_ALTERNATIVE == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW))
            || (E_XC_3D_INPUT_CHECK_BORAD == MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW)))
        {
            pstXC_SetWin_Info->stCapWin.width &= ~0x03;
        }
    #endif

    //if input is LA or TB, capture.height must be even.
    if((E_XC_3D_INPUT_LINE_ALTERNATIVE == MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow))
        || (E_XC_3D_INPUT_TOP_BOTTOM == MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow)))
    {
        bFramePacking = MDrv_SC_3D_IsFramePaking(pInstance,
                                                 pstXC_SetWin_Info->stCapWin.width,
                                                 pstXC_SetWin_Info->stCapWin.height,
                                                 pstXC_SetWin_Info->bInterlace);
        if(!bFramePacking)
        {
            pstXC_SetWin_Info->stCapWin.height &= ~0x01;
        }
    }

#if(HW_DESIGN_4K2K_VER == 7) //Kano Patch
    if((E_XC_DEVICE1 == psXCInstPri->u32DeviceID) &&
        (IsSrcTypeCapture(gSrcInfo[eWindow].enInputSourceType)
        ||IsSrcTypeSC0DI(gSrcInfo[eWindow].enInputSourceType)))
    {
        // H2V2 enabled
        if(SC_R2BYTEMSK(E_XC_DEVICE0, REG_SC_BK6A_20_L, BIT(0)) == BIT(0))
        {
            // When Kano H2V2 is enabled, H sync and DE will randomly shift 1 pixel
            // Need to switch to DE bypass mode to make sure video is always aligned
            // Attention, in DE bypass mode, capture window setting will be useless
            //This is SW patch for H2V2SD, U02 does not need
            if((H2V2_PIXEL_SHIFT_PATCH_USING_IP_DE_BYPASS_MODE == TRUE) && (MDrv_SYS_GetChipRev() == 0x00))
            {
                Hal_SC_ip_set_de_bypass_mode(pInstance, TRUE, eWindow);
            }
            pstXC_SetWin_Info->stCapWin.x = (pstXC_SetWin_Info->stCapWin.x / 2) - 59; // H_DE has 60 pixels delay
            pstXC_SetWin_Info->stCapWin.y /= 2;
            pstXC_SetWin_Info->stCapWin.width /= 2;
            pstXC_SetWin_Info->stCapWin.height /= 2;
        }
        // H2V2 bypassed
        else
        {
            //This is SW patch for H2V2SD, U02 does not need
            if((H2V2_PIXEL_SHIFT_PATCH_USING_IP_DE_BYPASS_MODE == TRUE) && (MDrv_SYS_GetChipRev() == 0x00))
            {
                Hal_SC_ip_set_de_bypass_mode(pInstance, FALSE, eWindow);
            }
            pstXC_SetWin_Info->stCapWin.x += 11; // H_DE has 11 pixels delay
        }
    }
#endif
}

MS_BOOL MDrv_XC_Get_VirtualBox_Info(void *pInstance,XC_VBOX_INFO *pstXC_VboxInfo)
{
    MS_U8 u8FrameCnt = 0;

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));


    if(pstXC_VboxInfo)
    {
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
#if(HW_DESIGN_4K2K_VER == 7) // Kano
        if((psXCInstPri->u32DeviceID == 0))
#endif
        {
            if ((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM] + gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM]) != 0)
                pstXC_VboxInfo->u32FBSize[0] = (gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM] + gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM]);
            else if ((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM] + gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM]) != 0)
                pstXC_VboxInfo->u32FBSize[0] = (gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM] + gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM]);
        }
#else
        pstXC_VboxInfo->u32FBSize[0] = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size;
#endif
#else
        pstXC_VboxInfo->u32FBSize[0] = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size;
#endif
        pstXC_VboxInfo->u32FBSize[1] = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size;
        pstXC_VboxInfo->u16VBox_PanelHsize = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width;
        pstXC_VboxInfo->u16VBox_PanelVsize = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height;

        // temp for sub
        pstXC_VboxInfo->u16VBox_Htotal[1] = 1920;
        pstXC_VboxInfo->u16VBox_Vtotal[1] = 1080;

        if(XC_SUPPORT_4K2K)
        {
                if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_SAVE_MEM_MODE)
                {
                    // in 4K2K case, the Madi mode is BK12_02 = 0x300 = 16bit/pixels
                    u8FrameCnt = (pstXC_VboxInfo->u32FBSize[0]) / (pstXC_VboxInfo->u16VBox_PanelHsize*pstXC_VboxInfo->u16VBox_PanelVsize * 16/8);
                }
                else
                {
                    // in 4K2K case, the Madi mode is P_Mode10 = 20bit/pixels
                    u8FrameCnt = (pstXC_VboxInfo->u32FBSize[0]) / (pstXC_VboxInfo->u16VBox_PanelHsize*pstXC_VboxInfo->u16VBox_PanelVsize * 20/8);
                }
                if(u8FrameCnt >= 2)
                {
            #if FRC_INSIDE
                /// FRC Enable
                    if(pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited)
                    {
                        #if(HW_DESIGN_4K2K_VER == 6)
                            if (pstXC_VboxInfo->bInterface[0] == FALSE)
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                            }
                            else
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                            }
                        #else
                            if(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_4K2K)
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                            }
                            else //if(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_2K1K)) and others
                            {
                                // 2K2K only for Napoli with FRC_On
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080*2;
                            }
                        #endif
                    }
                    else  /// FRC Disable
                    {
                        #if(HW_DESIGN_4K2K_VER == 6) || (HW_DESIGN_4K2K_VER == 7)
                            if (pstXC_VboxInfo->bInterface[0] == FALSE)
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                            }
                            else
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                            }
                        #else
                            if((pstXC_VboxInfo->u16Vfreq[0] <= 300)&&(pstXC_VboxInfo->bInterface[0] == FALSE))
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;//1920; //3840;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;//1080; //2160;
                            }
                            else
                            {
                                pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                                pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                            }
                        #endif
                    }
            #else
                if(!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_LEGACY_MODE) && (pstXC_VboxInfo->bInterface[0] == FALSE) )
                {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                }
                else if(!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_LEGACY_MODE) && (pstXC_VboxInfo->bInterface[0] == TRUE) )// DI 300MHz
                {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                }
                else // exp : Nike, Monaco_legacy_mode
                {
                    if((pstXC_VboxInfo->u16Vfreq[0] <= 300)&&(pstXC_VboxInfo->bInterface[0] == FALSE))
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                    }
                    else
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                    }
                }
        #endif
                }
                else
                 {
            #if (FRC_INSIDE &&(HW_DESIGN_4K2K_VER != 6))
                if(pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited
                     &&(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_2K1K))
                    {
                        // 2K2K only for Napoli with FRC_On
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080*2;
                    }
                    else
            #endif

            #if(HW_DESIGN_4K2K_VER == 6)
                    if(pstXC_VboxInfo->bInterface[0] == FALSE)//p mode//Monet: RFBl/FBL
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                    }
                    else//i mode
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                    }
            #else
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                    }
            #endif
                 }
            }
            else
            {
                pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
            }
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

E_APIXC_ReturnValue MApi_XC_Get_VirtualBox_Info_U2(void* pInstance, XC_VBOX_INFO *pstXC_VboxInfo)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    bRet = MDrv_XC_Get_VirtualBox_Info(pInstance,pstXC_VboxInfo);

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Get_VirtualBox_Info(XC_VBOX_INFO *pstXC_VboxInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Get_VBOX_Info XCArgs;
    XCArgs.pstXC_VboxInfo = pstXC_VboxInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_VBOX_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
MS_BOOL MDrv_XC_ConfigCMA(void *pInstance, XC_CMA_CONFIG *pstXC_CMA_Config, XC_CMA_CLIENT enCmaClient, MS_U32 u32DataLen, SCALER_WIN eWindow)
{
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
#if(HW_DESIGN_4K2K_VER == 7) // Kano
    if(psXCInstPri->u32DeviceID != 0)
    {
        return TRUE;
    }
#endif
    if (enCmaClient >= CMA_XC_MEM_NUM)
    {
        printf("[%s,%5d] XC CMA CMA Client error!\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    if (u32DataLen == sizeof(XC_CMA_CONFIG))
    {
        memcpy(&gSrcInfo[eWindow].Status2.stXCConfigCMA[enCmaClient], pstXC_CMA_Config, u32DataLen);
    }
    else
    {
        printf("[%s,%5d] XC CMA u32DataLen error!\n",__FUNCTION__,__LINE__);
        return FALSE;
    }
#endif
#endif
    return TRUE;
}

MS_BOOL MApi_XC_ConfigCMA_U2(void *pInstance, XC_CMA_CONFIG *pstXC_CMA_Config, XC_CMA_CLIENT enCmaClient, MS_U32 u32DataLen, SCALER_WIN eWindow)
{
    MS_BOOL bRet = FALSE;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    bRet = MDrv_XC_ConfigCMA(pInstance, pstXC_CMA_Config, enCmaClient, u32DataLen, eWindow);

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_ConfigCMA(XC_CMA_CONFIG *pstXC_CMA_Config, XC_CMA_CLIENT enCmaClient, MS_U32 u32DataLen, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
      }
#endif

    stXC_CONFIG_CMA XCArgs;
    XCArgs.pstXC_CMA_Config = pstXC_CMA_Config;
    XCArgs.enCmaClient = enCmaClient;
    XCArgs.u32InitDataLen = u32DataLen;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CONFIG_CMA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif
#if (defined (ANDROID)&& defined(UFO_XC_DS_PQ))
extern void MDrv_PQ_set_imode_PQDS(MS_BOOL imode);
#endif
//-------------------------------------------------------------------------------------------------
/// Set the specific window
/// @param  pstXC_SetWin_Info      \b IN: the information of the window setting
/// @param  u32InitDataLen         \b IN: the length of the pstXC_SetWin_Info
/// @param  eWindow                \b IN: which window we are going to set
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
static MS_BOOL _MApi_XC_SetWindow(void *pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, MS_U32 u32InitDataLen, MS_BOOL bDualWindow, SCALER_WIN eWindow)
{
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
u32SetWindowTime = MsOS_GetSystemTime();
#endif
#ifdef VIDEO_4K_I_PATCH
    if((pstXC_SetWin_Info->stCapWin.height > VIDEO_4K_I_BIG_HEIGHT)&&(pstXC_SetWin_Info->bInterlace))
    {
        pstXC_SetWin_Info->stCapWin.height -= 4;
    }
#endif

#if VERIFY_MVIDEO_FPGA
    MS_BOOL binterlace =FALSE;
#endif
//    MS_BOOL bEnableHighSpeedFclk = FALSE;
    MS_U32 u32DisPOddCheck = 0x00;
    MS_BOOL bRet = TRUE;

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    MS_BOOL bChangeDisplaySize = FALSE;
    MS_U8 u8LPLL_Type =(MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type;
#endif

    // Need clear dual mode enable
#ifdef UFO_XC_SUPPORT_DUAL_MIU
    //MDrv_XC_EnableMiuDualMode(pInstance, FALSE, 2, 2, eWindow);
#endif
#if 0 //jsut for test for maserati dual miu.
#ifndef DONT_USE_CMA
#ifdef UFO_XC_SUPPORT_DUAL_MIU
        if (eWindow == MAIN_WINDOW)
        {
            if (IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType)) //&& ((pstXC_SetWin_Info->stCapWin.width > 1920) && (pstXC_SetWin_Info->stCapWin.height > 1080)))
            {
                MDrv_XC_EnableMiuDualMode(pInstance, TRUE, 2, 2, MAIN_WINDOW);
            }
            else
            {
                MDrv_XC_EnableMiuDualMode(pInstance, FALSE, 2, 2, eWindow);
            }
        }
        else
        {
            MDrv_XC_EnableMiuDualMode(pInstance, FALSE, 2, 2, eWindow);
        }
#endif
#endif
#endif


    MDrv_SC_SetPQHSDFlag(pInstance, FALSE); //Reset variable


#ifdef K3_U2
    // Prepare seamless zapping, reset commands in next DS index
    if((Hal_SC_GetSeamlessZappingStatus(pInstance, eWindow)) && (Hal_SC_get_freezeimg(pInstance, eWindow)))
    {
        Hal_SC_PrepareSeamlessZapping(pInstance, eWindow);
    }
#endif
#ifdef UFO_XC_HDMI_4K2K_DMA
    if(IsSrcTypeHDMI(gSrcInfo[eWindow].enInputSourceType) && (pstXC_SetWin_Info->stCapWin.width >= 3840) && (pstXC_SetWin_Info->stCapWin.height >= 2160))
    {
        MHal_XC_HDR_Control(pInstance, E_XC_HDR_CTRL_INIT_DMA_HDMI, (void *)(&pXCResourcePrivate->stdrvXC_Display._stDMAInitInfo_Hdmi));
    }
    else
    {
        MHal_XC_HDR_Control(pInstance, E_XC_HDR_CTRL_INIT_DMA, (void *)(&pXCResourcePrivate->stdrvXC_Display._stDMAInitInfo));
    }
#endif
#ifdef VDEC_4K2K_PATCH_MODIFY_INFO
    if(IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType)||IsSrcTypeDTV(gSrcInfo[eWindow].enInputSourceType))
    {
        if( pstXC_SetWin_Info->stCapWin.width>=2048 )
        {
            pstXC_SetWin_Info->stCapWin.width=pstXC_SetWin_Info->stCapWin.width /2;
            pstXC_SetWin_Info->stCapWin.x=pstXC_SetWin_Info->stCapWin.x /2;
        }

        if( pstXC_SetWin_Info->stCropWin.width>=2048 )
        {
            pstXC_SetWin_Info->stCropWin.width=pstXC_SetWin_Info->stCropWin.width /2;
            pstXC_SetWin_Info->stCropWin.x=pstXC_SetWin_Info->stCropWin.x /2;
        }
    }
#endif

#ifdef VIDEO_4K_I_PATCH
    if((pstXC_SetWin_Info->stCropWin.height > VIDEO_4K_I_HEIGHT)&&(pstXC_SetWin_Info->bInterlace == TRUE))
    {
         pstXC_SetWin_Info->stCropWin.height /= 2;
    }
#endif

    // we've to support interface changed without re-compile user application
    // please specify new added interface default value here
    //_u16XCDbgSwitch_InternalUseOnly = (XC_DBGLEVEL_SETWINDOW|XC_DGBLEVEL_CROPCALC|XC_DBGLEVEL_SETTIMING);
    if(u32InitDataLen != sizeof(XC_SETWIN_INFO))
    {
        // customer application different version with our driver
        // start from customized pre H/V scaling
        gSrcInfo[eWindow].Status2.bPreHCusScaling = FALSE;
        gSrcInfo[eWindow].Status2.bPreVCusScaling = FALSE;
        gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc = 0;
        gSrcInfo[eWindow].Status2.u16PreHCusScalingDst = 0;
        gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc = 0;
        gSrcInfo[eWindow].Status2.u16PreVCusScalingDst = 0;
    }
    else
    {
        gSrcInfo[eWindow].Status2.bPreHCusScaling = pstXC_SetWin_Info->bPreHCusScaling;
        gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc = pstXC_SetWin_Info->u16PreHCusScalingSrc;
        gSrcInfo[eWindow].Status2.u16PreHCusScalingDst = pstXC_SetWin_Info->u16PreHCusScalingDst;

        if(gSrcInfo[eWindow].Status2.u16PreHCusScalingDst > gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"H can't do pre scaling up, change to auto prescaling\n");
            gSrcInfo[eWindow].Status2.bPreHCusScaling = FALSE;
            gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc = 0;
            gSrcInfo[eWindow].Status2.u16PreHCusScalingDst = 0;
        }

        if (gSrcInfo[eWindow].Status2.bPreHCusScaling)
        {
            if (gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc == 0 || gSrcInfo[eWindow].Status2.u16PreHCusScalingDst == 0)
            {
                gSrcInfo[eWindow].Status2.bPreHCusScaling = FALSE;
                gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc = 0;
                gSrcInfo[eWindow].Status2.u16PreHCusScalingDst = 0;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"PreHCus Src or Dst can not be 0 \n");
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                return FALSE;
            }
        }
        else
        {
            gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc = 0;
            gSrcInfo[eWindow].Status2.u16PreHCusScalingDst = 0;
        }

        gSrcInfo[eWindow].Status2.bPreVCusScaling = pstXC_SetWin_Info->bPreVCusScaling;
        gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc = pstXC_SetWin_Info->u16PreVCusScalingSrc;
        gSrcInfo[eWindow].Status2.u16PreVCusScalingDst = pstXC_SetWin_Info->u16PreVCusScalingDst;

        if(gSrcInfo[eWindow].Status2.u16PreVCusScalingDst > gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"V can't do pre scaling up, change to auto prescaling\n");
            gSrcInfo[eWindow].Status2.bPreVCusScaling = FALSE;
            gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc = 0;
            gSrcInfo[eWindow].Status2.u16PreVCusScalingDst = 0;
        }

        if (gSrcInfo[eWindow].Status2.bPreVCusScaling)
        {
            if (gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc == 0 || gSrcInfo[eWindow].Status2.u16PreVCusScalingDst == 0  )
            {
                gSrcInfo[eWindow].Status2.bPreVCusScaling = FALSE;
                gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc = 0;
                gSrcInfo[eWindow].Status2.u16PreVCusScalingDst = 0;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"PreVCus Src or Dst can not be 0 \n");
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                return FALSE;
            }
        }
        else
        {
            gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc = 0;
            gSrcInfo[eWindow].Status2.u16PreVCusScalingDst = 0;
        }
    }

    _MDrv_XC_CheckCaptureWin(pInstance, pstXC_SetWin_Info, eWindow);
    _MDrv_XC_CheckCropWin(pInstance, pstXC_SetWin_Info, eWindow);
    _MDrv_XC_CheckDisplayWin(pInstance, pstXC_SetWin_Info, eWindow);

    #if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    if (IsVBY1_16LANE(u8LPLL_Type))
    {
        // For 120Hz BW request, need to send video size to FRC,
        // FRC would reduce BW by different case
#if (defined (ANDROID))
#else
        Hal_XC_FRC_R2_Set_InputFrameSize(pInstance, pstXC_SetWin_Info->stCropWin.width, pstXC_SetWin_Info->stCropWin.height);
#endif
    }
    #endif

    // copy user data to internal data
    // here is a patch first, will remove after seperate Internal XC status with set window information
    //u32CopyLen = sizeof(XC_SETWIN_INFO) - (sizeof(MS_BOOL)*2) - (sizeof(MS_U16)*4);
    //memcpy(&gSrcInfo[eWindow], pstXC_SetWin_Info, u32CopyLen);
    //do not use memcopy(), strongly depend on structure definition
    _Mdrv_XC_CopySetWinInfo(pInstance, pstXC_SetWin_Info, eWindow);

    bRet = MDrv_SC_3D_FormatValidation(pInstance, &gSrcInfo[eWindow], eWindow);
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (((pXCResourcePrivate->stdrvXC_3D._eInput3DMode[eWindow] != E_XC_3D_INPUT_MODE_NONE)
            && (pXCResourcePrivate->stdrvXC_3D._eOutput3DMode != E_XC_3D_OUTPUT_MODE_NONE))
            || (pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange==0 && pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange==0))
        {
            MHal_XC_PixelShiftStatusChange(pInstance, EN_XC_PIXEL_SHIFT_DISABLE);
        }
#endif

    //for 3D handle invalid display window
    if(E_XC_3D_OUTPUT_CHECKBOARD_HW == MDrv_XC_Get_3D_Output_Mode(pInstance))
    {
        gSrcInfo[eWindow].stDispWin.y &= ~0x1;
        gSrcInfo[eWindow].stDispWin.x &= ~0x1;
        gSrcInfo[eWindow].stDispWin.height &= ~0x1;
        gSrcInfo[eWindow].stDispWin.width &= ~0x1;
    }
    else if((E_XC_3D_OUTPUT_LINE_ALTERNATIVE == MDrv_XC_Get_3D_Output_Mode(pInstance))
            || (E_XC_3D_OUTPUT_LINE_ALTERNATIVE_HW == MDrv_XC_Get_3D_Output_Mode(pInstance))
            || ((E_XC_3D_OUTPUT_TOP_BOTTOM == MDrv_XC_Get_3D_Output_Mode(pInstance)) && MDrv_SC_3D_Is_LR_Sbs2Line(pInstance)))
    {
        gSrcInfo[eWindow].stDispWin.y &= ~0x1;
        gSrcInfo[eWindow].stDispWin.height &= ~0x1;
    }
    else if(E_XC_3D_OUTPUT_PIXEL_ALTERNATIVE_HW == MDrv_XC_Get_3D_Output_Mode(pInstance))
    {
        gSrcInfo[eWindow].stDispWin.x &= ~0x1;
        gSrcInfo[eWindow].stDispWin.width &= ~0x1;
    }
    else if((E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF == MDrv_XC_Get_3D_Output_Mode(pInstance))
            || (E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF_HW == MDrv_XC_Get_3D_Output_Mode(pInstance)))
    {
        gSrcInfo[eWindow].stDispWin.width &= ~0x1;
    }
    else if((E_XC_3D_OUTPUT_TOP_BOTTOM == MDrv_XC_Get_3D_Output_Mode(pInstance))
            || (E_XC_3D_OUTPUT_TOP_BOTTOM_HW == MDrv_XC_Get_3D_Output_Mode(pInstance)))
    {
        gSrcInfo[eWindow].stDispWin.height &= ~0x1;
    }

    // assert check
    MS_ASSERT((gSrcInfo[eWindow].bHCusScaling==TRUE)?((gSrcInfo[eWindow].u16HCusScalingSrc!=0)&&(gSrcInfo[eWindow].u16HCusScalingDst!=0)):1);
    MS_ASSERT((gSrcInfo[eWindow].bVCusScaling==TRUE)?((gSrcInfo[eWindow].u16VCusScalingSrc!=0)&&(gSrcInfo[eWindow].u16VCusScalingDst!=0)):1);
    MS_ASSERT((gSrcInfo[eWindow].Status2.bPreHCusScaling==TRUE)?((gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc!=0)&&(gSrcInfo[eWindow].Status2.u16PreHCusScalingDst!=0)):1);
    MS_ASSERT((gSrcInfo[eWindow].Status2.bPreVCusScaling==TRUE)?((gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc!=0)&&(gSrcInfo[eWindow].Status2.u16PreVCusScalingDst!=0)):1);

    // The total pixel conut of display window can not be odd.
    u32DisPOddCheck = gSrcInfo[eWindow].stDispWin.width * gSrcInfo[eWindow].stDispWin.height;
    // if u32DisPOddCheck is odd, that means both of disp.width and disp.height is odd.
    // we adjust width to even.
    if (u32DisPOddCheck & 0x01)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width > gSrcInfo[eWindow].stDispWin.width )
        {
            gSrcInfo[eWindow].stDispWin.width = ( gSrcInfo[eWindow].stDispWin.width + 1 ) & ~0x1;
        }
    }

    if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_4K_HALFK_240HZ)
    {
        gSrcInfo[eWindow].stDispWin.height = gSrcInfo[eWindow].stDispWin.height/4;
    }
    else if(MDrv_XC_Is2K2KToFrc(pInstance))
    {
        gSrcInfo[eWindow].stDispWin.x = gSrcInfo[eWindow].stDispWin.x/2;
        gSrcInfo[eWindow].stDispWin.width = gSrcInfo[eWindow].stDispWin.width/2;
    }

    #if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    // For Manhattan, FSC+FRC
    // Only main path support FSC+FSC, so not change FSC/FRC status for sub setwindow
    if (eWindow == MAIN_WINDOW)
    {
        if (MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_MODE_NONE)
        {
            // Maserati mode
            if (IsVBY1_16LANE(u8LPLL_Type))
            {
                if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_SKIP_SR)
                {
                    // case 4: no FSC + FRC
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = TRUE; // Maserati mode, FRC always need
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
                }
                else
                {
                    // case 5: FSC + FRC
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = TRUE; // Maserati mode, FRC always need
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = TRUE;
                }
            }
            else // Manhattan mode
            if ((MHal_FRC_IsSupportFRC_byEfuse(pInstance))
               &&(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.bFRC && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE))
               && (MHal_FRC_Check_Condition(pInstance))
               &&!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_SKIP_SR)
                )
            {
                // MM 24/25/30fps case, need MEMC, so use case3
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = TRUE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = TRUE;
            }
            else if (!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_SKIP_SR))
            {
                // SD/FHD input case, always do SR, so use case2
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = TRUE;
            }
            else
            {
                // 4K 50/60 case, no need to do SR and MEMC, so use case1
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
            }
        }
        else
        {
            ///3D Maserati mode or manhattan mode use FRC 3D
            if ((MHal_FRC_IsEnableFRC3D(pInstance, MAIN_WINDOW)
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                || Hal_SC_3D_IsDualViewMode(pInstance,eWindow)
#endif
                //Frc cant do memc under RGB source.Memc will bypass if RGB source in.
                //Under memc bypass mode it cant send 3D LR flag so that RGB 3D should disable FRC enable flag.
                ) && gSrcInfo[MAIN_WINDOW].bUseYUVSpace
                && (!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_SKIP_SR)))
            {
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = TRUE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = TRUE;
            }
            else if (!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_SKIP_SR))
            {
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = TRUE;
            }
            else
            {
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
            }
        }

        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled)
        {
            if (MHal_FRC_IsFHDToFSC(pInstance, eWindow))
            {
                // FHD to FSC case, need to make post-scaling to displayH/2 + displayV/2 first
                gSrcInfo[eWindow].stDispWin.x = gSrcInfo[eWindow].stDispWin.x/2;
                gSrcInfo[eWindow].stDispWin.width = gSrcInfo[eWindow].stDispWin.width/2;
                gSrcInfo[eWindow].stDispWin.y = gSrcInfo[eWindow].stDispWin.y/2;
                gSrcInfo[eWindow].stDispWin.height = gSrcInfo[eWindow].stDispWin.height/2;
                bChangeDisplaySize = TRUE;

                // for custom scaling case
                if(gSrcInfo[eWindow].bHCusScaling)
                {
                    gSrcInfo[eWindow].u16HCusScalingDst = gSrcInfo[eWindow].u16HCusScalingDst/2;
                }
                if(gSrcInfo[eWindow].bVCusScaling)
                {
                    gSrcInfo[eWindow].u16VCusScalingDst = gSrcInfo[eWindow].u16VCusScalingDst/2;
                }
            }
        }
    }
    #endif

    // if V mirror, need to re-cal display window but Ninelattice!!
    if(IsVMirrorMode(eWindow)&&(!gSrcInfo[eWindow].bDisplayNineLattice))
    {
        MS_U32 u32TempHeight = gSrcInfo[eWindow].stDispWin.height;

        if(((MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_TOP_BOTTOM)
                    || (MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_TOP_BOTTOM_HW))
                && ((MDrv_XC_Get_3D_HW_Version() > 1)
                    || ((MDrv_XC_Get_3D_HW_Version() == 1) && (!MDrv_SC_3D_Is_LR_Sbs2Line(pInstance)))))
        {
            gSrcInfo[eWindow].stDispWin.y = gSrcInfo[eWindow].stDispWin.y * 2;
        }

        //for ver3d =1, display window will be half when pass into driver,so here need double it
        if(((MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_TOP_BOTTOM)
                || (MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_TOP_BOTTOM_HW))
            && (MDrv_XC_Get_3D_HW_Version() == 1))
        {
            u32TempHeight *= 2;
        }

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled && bChangeDisplaySize)
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height/2 > ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight ) )
            {
                gSrcInfo[eWindow].stDispWin.y =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height/2 - ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight );
            }
            else
            {
                //pixshift case will do overscan,changed display.y&Height,y + Height >stPanelInfo.u16Height,so need not error handling
                if( !((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN))
                {
                    gSrcInfo[eWindow].stDispWin.y = 0;
                }
            }
        }
        else
#endif
        if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height > ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight ) )
        {
            gSrcInfo[eWindow].stDispWin.y =
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height - ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight );
        }
        else
        {
            //pixshift case will do overscan,changed display.y&Height,y + Height >stPanelInfo.u16Height,so need not error handling
            if( !((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN))
            {
                gSrcInfo[eWindow].stDispWin.y = 0;
            }
        }

        if(((MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_TOP_BOTTOM)
                    || (MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_TOP_BOTTOM_HW))
                && ((MDrv_XC_Get_3D_HW_Version() > 1)
                    || ((MDrv_XC_Get_3D_HW_Version() == 1) && (!MDrv_SC_3D_Is_LR_Sbs2Line(pInstance)))))
        {
            gSrcInfo[eWindow].stDispWin.y /= 2;

            //adjust main/sub display window position
            //for ver>=2, this logic is at MDrv_XC_3D_LoadReg
            if(MDrv_XC_Get_3D_HW_Version() == 1)
            {
                if(eWindow == MAIN_WINDOW)
                {
                    gSrcInfo[eWindow].stDispWin.y += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height / 2;
                }
                else
                {
                    if(gSrcInfo[eWindow].stDispWin.y >= pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height / 2)
                    {
                        gSrcInfo[eWindow].stDispWin.y -= pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height / 2;
                    }
                }
            }
        }
    }
    // if H mirror, need to re-cal display window but Ninelattice!!
    if(IsHMirrorMode(eWindow)&&(!gSrcInfo[eWindow].bDisplayNineLattice))
    {
        MS_U32 u32TempWidth = gSrcInfo[eWindow].stDispWin.width;

        if(((MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
               || (MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF_HW))
            && (MDrv_XC_Get_3D_HW_Version() >= 1))
        {
            gSrcInfo[eWindow].stDispWin.x = gSrcInfo[eWindow].stDispWin.x * 2;
        }

        //for ver3d =1, display window will be half when pass into driver,so here need double it
        if(((MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
                || (MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF_HW))
            && (MDrv_XC_Get_3D_HW_Version() == 1))
        {
            u32TempWidth *= 2;
        }

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled && bChangeDisplaySize)
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2 > ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth ) )
            {
                gSrcInfo[eWindow].stDispWin.x =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2 - ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth );
            }
            else
            {
                //pixshift case will do overscan,changed display.x&width,x + width >stPanelInfo.u16Width,so need not error handling
                if( !((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN))
                {
                    gSrcInfo[eWindow].stDispWin.x = 0;
                }
            }
        }
        else
#endif
        if(MDrv_XC_Is2K2KToFrc(pInstance))
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2 > ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth ) )
            {
                gSrcInfo[eWindow].stDispWin.x =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2 - ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth );
            }
            else
            {
                //pixshift case will do overscan,changed display.x&width,x + width >stPanelInfo.u16Width,so need not error handling
                if( !((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN))
                {
                    gSrcInfo[eWindow].stDispWin.x = 0;
                }
            }
        }
        else
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width > ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth ) )
            {
                gSrcInfo[eWindow].stDispWin.x =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width - ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth );
            }
            else
            {
                //pixshift case will do overscan,changed display.x&width,x + width >stPanelInfo.u16Width,so need not error handling
                if( !((pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature & PIXEL_SHIFT_DO_OVERSCAN )== PIXEL_SHIFT_DO_OVERSCAN))
                {
                    gSrcInfo[eWindow].stDispWin.x = 0;
                }
            }
        }

        if(((MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
                || (MDrv_XC_Get_3D_Output_Mode(pInstance)== E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF_HW))
            &&(MDrv_XC_Get_3D_HW_Version() >= 1))
        {
            gSrcInfo[eWindow].stDispWin.x /= 2;

            //adjust main/sub display window position for ver1
            //for ver>=2, this logic is at MDrv_XC_3D_LoadReg
            if(MDrv_XC_Get_3D_HW_Version() == 1)
            {
                if(eWindow == MAIN_WINDOW)
                {
                    gSrcInfo[eWindow].stDispWin.x += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2;
                }
                else
                {
                    if(gSrcInfo[eWindow].stDispWin.x >= pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2)
                    {
                        gSrcInfo[eWindow].stDispWin.x -= pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width / 2;
                    }
                }
            }
        }
    }

    MDrv_SC_3D_AdjustHShift(pInstance, &gSrcInfo[eWindow], eWindow);

    if(gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[DS] MDrv_XC_GetDynamicScalingStatus():%d MDrv_XC_Is_DSForceIndexEnabled():%d\n",MDrv_XC_GetDynamicScalingStatus(pInstance),MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow));

        if(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow) )
        {
            gSrcInfo[eWindow].stCapWin.width = pstXC_SetWin_Info->stCapWin.width;
            gSrcInfo[eWindow].stCapWin.height = pstXC_SetWin_Info->stCapWin.height;
            gSrcInfo[eWindow].stCropWin.width = pstXC_SetWin_Info->stCropWin.width;
            gSrcInfo[eWindow].stCropWin.height = pstXC_SetWin_Info->stCropWin.height;
        }
        //do FW DS pixshift,update pixshift display window
        if((pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange != 0) ||(pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange != 0))
        {
            gSrcInfo[eWindow].stDispWin.x += pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange/2;
            gSrcInfo[eWindow].stDispWin.y += pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange/2;
            gSrcInfo[eWindow].stDispWin.x += pXCResourcePrivate->stdrvXC_MVideo.s8HPixelShift;
            gSrcInfo[eWindow].stDispWin.y += pXCResourcePrivate->stdrvXC_MVideo.s8VPixelShift;
        }
    }
    else
    {
        //reset pixel shift preH and preV data,SW DS pixshift need not reset
        pXCResourcePrivate->stdrvXC_MVideo.s8HPixelShift = 0;
        pXCResourcePrivate->stdrvXC_MVideo.s8VPixelShift = 0;
    }
    gSrcInfo[eWindow].stDispWin.x += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart;
    gSrcInfo[eWindow].stDispWin.y += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart;

    // dump debug msg
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= SetWindow Start (Window : %u, src: %u) =============\n",eWindow, gSrcInfo[eWindow].enInputSourceType);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CapWin  x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCapWin.x, gSrcInfo[eWindow].stCapWin.y, gSrcInfo[eWindow].stCapWin.width, gSrcInfo[eWindow].stCapWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CropWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCropWin.x, gSrcInfo[eWindow].stCropWin.y, gSrcInfo[eWindow].stCropWin.width, gSrcInfo[eWindow].stCropWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"DispWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stDispWin.x,gSrcInfo[eWindow].stDispWin.y, gSrcInfo[eWindow].stDispWin.width, gSrcInfo[eWindow].stDispWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Panel   x: %4u  y: %4u  w: %4u  h: %4u \n",
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"HMirror/VMirror/Interlace/Hdup = %u/%u/%u/%u\n", IsHMirrorMode(eWindow),IsVMirrorMode(eWindow), pstXC_SetWin_Info->bInterlace, pstXC_SetWin_Info->bHDuplicate);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"H/V total = (%u, %u), VFreq = %u\n", pstXC_SetWin_Info->u16DefaultHtotal, pstXC_SetWin_Info->u16InputVTotal, pstXC_SetWin_Info->u16InputVFreq);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Pre  H cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].Status2.bPreHCusScaling, gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc, gSrcInfo[eWindow].Status2.u16PreHCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Pre  V cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].Status2.bPreVCusScaling, gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc, gSrcInfo[eWindow].Status2.u16PreVCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Post H cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].bHCusScaling, gSrcInfo[eWindow].u16HCusScalingSrc, gSrcInfo[eWindow].u16HCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Post V cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].bVCusScaling, gSrcInfo[eWindow].u16VCusScalingSrc, gSrcInfo[eWindow].u16VCusScalingDst);

#if VERIFY_MVIDEO_FPGA  //non-de-interlace function
    if(gSrcInfo[eWindow].bInterlace)
    {
        gSrcInfo[eWindow].bInterlace = 0;
        binterlace =TRUE;//gSrcInfo[eWindow].u16V_DisSize /= 2;
    }
#endif

#ifdef XC_SUPPORT_2STEP_SCALING
    #ifdef UFO_XC_SUPPORT_2STEP_SCALING
    MDrv_XC_SetIsSupport2StepScalingFlag(FALSE);
    if(MDrv_ReadByte(REG_CHIP_REVISION) >= 1)//Monet U02 fix .
    {
        if((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal > 3000)          // 4K output
         &&(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VTotal > 2000)
         &&(MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_MODE_NONE)          //3D can not enable 2-step scaling. Hardware limitation
         &&(MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow) == E_XC_3D_INPUT_MODE_NONE))
        {
            if( (gSrcInfo[eWindow].stCapWin.width < 1440)
                && (gSrcInfo[eWindow].stDispWin.width == pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width)
                )
            {
                MDrv_XC_SetIsSupport2StepScalingFlag(TRUE);
            }
            else
            {
                MDrv_XC_SetIsSupport2StepScalingFlag(FALSE);
            }
        }
    }
    #endif
#endif


    gSrcInfo[eWindow].Status2.u16VirtualBox_Width = 0;
    gSrcInfo[eWindow].Status2.u16VirtualBox_Height = 0;

    XC_VBOX_INFO pstXCVBox;
    memset(&pstXCVBox, 0, sizeof(XC_VBOX_INFO));
    pstXCVBox.bInterface[eWindow] = gSrcInfo[eWindow].bInterlace;
    pstXCVBox.u16Vfreq[eWindow] = pstXC_SetWin_Info->u16InputVFreq;

    gSrcInfo[eWindow].u16InputVFreq = pstXC_SetWin_Info->u16InputVFreq;

#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
    if (
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
#ifdef UFO_XC_SUPPORT_SUB_CMA
    ((psXCInstPri->u32DeviceID==0)||(psXCInstPri->u32DeviceID==1))
#else
    (psXCInstPri->u32DeviceID==0)
#endif
#elif (HW_DESIGN_4K2K_VER == 7)
    (psXCInstPri->u32DeviceID == 0) && (MAIN_WINDOW==eWindow)
#else
    (MAIN_WINDOW==eWindow)
#endif

#if defined(UFO_XC_SET_DSINFO_V0)
    && (!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
#else
    && (! MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
#endif
    && (pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].isEnable == FALSE))
    {
        MS_BOOL bIsGetCMA[CMA_XC_MEM_NUM] = {FALSE, };
        MS_U32 au32CMAMemSCMSize[CMA_XC_MEM_NUM] = {0, };
        MS_U32 au32CMAMemFRCMSize[CMA_XC_MEM_NUM] = {0, };
        MS_U32 u32DualMiuMemSize = 0;

        if (MDrv_XC_IsInputSourceDisabled(pInstance, MAIN_WINDOW) == TRUE)
        {
            MHal_XC_CMAPatchClose(pInstance, MAIN_WINDOW);
            bNeedDeattachCMAISR = TRUE;
        }
        else
        {
            MHal_XC_CMAPatch(pInstance, MAIN_WINDOW);
        }

        #ifdef CONFIG_MSTAR_SRAMPD
        // control SRAM PD here
        if (MHal_XC_Is_SRAM_PowerDown(pInstance) == TRUE)
        {
            // enable SRAM here
            MHal_XC_SRAM_PowerDown_Control(pInstance, FALSE);
        }
        #endif

        MHal_XC_Get_CMA_UsingCondition(pInstance, bIsGetCMA, au32CMAMemSCMSize, au32CMAMemFRCMSize, &u32DualMiuMemSize, CMA_XC_MEM_NUM, eWindow);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], bIsGetCMA[self]=%d bIsGetCMA[cobuff]=%d \n",__func__,__LINE__,bIsGetCMA[CMA_XC_SELF_MEM],bIsGetCMA[CMA_XC_COBUFF_MEM]);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], au32CMAMemSCMSize[self]=%x au32CMAMemSCMSize[cobuff]=%x u32DualMiuMemSize=%x\n",__func__,__LINE__,au32CMAMemSCMSize[CMA_XC_SELF_MEM],au32CMAMemSCMSize[CMA_XC_COBUFF_MEM],u32DualMiuMemSize);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], u32PreCMAMemSCMSize[self]=%x u32PreCMAMemSCMSize[cobuff]=%x \n",__func__,__LINE__,gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM],gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM]);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], u32PreCMAMemFRCMSize[self]=%x u32PreCMAMemFRCMSize[cobuff]=%x \n",__func__,__LINE__,gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM],gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM]);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], u32PreCMAMemDualSCMSize=%x \n",__func__, __LINE__, gSrcInfo[eWindow].Status2.u32PreCMAMemDualSCMSize);

        if(  ((bIsGetCMA[CMA_XC_SELF_MEM] == TRUE)&&((au32CMAMemSCMSize[CMA_XC_SELF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM])||(au32CMAMemFRCMSize[CMA_XC_SELF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM])))
           ||((bIsGetCMA[CMA_XC_COBUFF_MEM] == TRUE)&&((u32DualMiuMemSize!=gSrcInfo[eWindow].Status2.u32PreCMAMemDualSCMSize)||(au32CMAMemSCMSize[CMA_XC_COBUFF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM])||(au32CMAMemFRCMSize[CMA_XC_COBUFF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM]))))
        {
            Hal_SC_set_freezeimg(pInstance, ENABLE, MAIN_WINDOW);
            MS_PHY u32CMAAddr[CMA_XC_MEM_NUM] = {0, };
            MS_BOOL bCMAChanges[CMA_XC_MEM_NUM] = {FALSE,};

            if (bIsGetCMA[CMA_XC_SELF_MEM] == TRUE)
            {
                if ((au32CMAMemSCMSize[CMA_XC_SELF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM])||(au32CMAMemFRCMSize[CMA_XC_SELF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM]))
                {
                    MHal_XC_Release_CMA(pInstance, CMA_XC_SELF_MEM, MAIN_WINDOW);
                    u32CMAAddr[CMA_XC_SELF_MEM] = MHal_XC_Get_CMA_Addr(pInstance, CMA_XC_SELF_MEM,  au32CMAMemSCMSize[CMA_XC_SELF_MEM]+au32CMAMemFRCMSize[CMA_XC_SELF_MEM], eWindow);
                    gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM] = au32CMAMemSCMSize[CMA_XC_SELF_MEM];
                    gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM] = au32CMAMemFRCMSize[CMA_XC_SELF_MEM];
                    bCMAChanges[CMA_XC_SELF_MEM] = TRUE;
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], u32CMAAddr[CMA_XC_SELF_MEM]=%x \n",__func__, __LINE__, (MS_U32)u32CMAAddr[CMA_XC_SELF_MEM]);
                }
            }
            else
            {
                MHal_XC_Release_CMA(pInstance, CMA_XC_SELF_MEM, MAIN_WINDOW);
                bCMAChanges[CMA_XC_SELF_MEM] = FALSE;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], Release self buff!\n",__func__, __LINE__);
            }

            if (bIsGetCMA[CMA_XC_COBUFF_MEM] == TRUE)
            {
                if ((u32DualMiuMemSize!=gSrcInfo[eWindow].Status2.u32PreCMAMemDualSCMSize)
                   ||(au32CMAMemSCMSize[CMA_XC_COBUFF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM])
                   ||(au32CMAMemFRCMSize[CMA_XC_COBUFF_MEM]!=gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM]))
                {
                        MHal_XC_Release_CMA(pInstance, CMA_XC_COBUFF_MEM, MAIN_WINDOW);
                        if (u32DualMiuMemSize != 0)
                        {
                            u32CMAAddr[CMA_XC_COBUFF_MEM] = MHal_XC_Get_CMA_Addr(pInstance, CMA_XC_COBUFF_MEM, u32DualMiuMemSize, eWindow);
                        }
                        else
                        {
                            u32CMAAddr[CMA_XC_COBUFF_MEM] = MHal_XC_Get_CMA_Addr(pInstance, CMA_XC_COBUFF_MEM, au32CMAMemSCMSize[CMA_XC_COBUFF_MEM]+au32CMAMemFRCMSize[CMA_XC_COBUFF_MEM], eWindow);
                        }
                    gSrcInfo[eWindow].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM] = au32CMAMemSCMSize[CMA_XC_COBUFF_MEM];
                    gSrcInfo[eWindow].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM] = au32CMAMemFRCMSize[CMA_XC_COBUFF_MEM];
                    gSrcInfo[eWindow].Status2.u32PreCMAMemDualSCMSize = u32DualMiuMemSize;
                    bCMAChanges[CMA_XC_COBUFF_MEM] = TRUE;
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], u32CMAAddr[CMA_XC_SELF_MEM]=%x \n",__func__, __LINE__, (MS_U32)u32CMAAddr[CMA_XC_COBUFF_MEM]);
                }
            }
            else
            {
                MHal_XC_Release_CMA(pInstance, CMA_XC_COBUFF_MEM, MAIN_WINDOW);
                bCMAChanges[CMA_XC_COBUFF_MEM] = FALSE;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d][CMA], Release cobuff buff!\n",__func__, __LINE__);
            }
            XC_CMA_CLIENT enXCCMAClient = CMA_XC_MEM_NUM;
            if (u32DualMiuMemSize != 0)
            {
                enXCCMAClient = CMA_XC_SELF_MEM;
            }
            else
            {
                if (bIsGetCMA[CMA_XC_COBUFF_MEM] == TRUE)
                {
                    enXCCMAClient = CMA_XC_COBUFF_MEM;
                }
                else
                {
                    enXCCMAClient = CMA_XC_SELF_MEM;
                }
            }
#if (HW_DESIGN_4K2K_VER == 4)
            if(au32CMAMemSCMSize[enXCCMAClient] == 0)
            {
                // default frcm frame buffer address init
                MDrv_XC_SetFRCMFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], eWindow);
                // default frame buffer address init
                MDrv_XC_SetFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], eWindow);
            }
            else
#endif
            {
                if(bCMAChanges[enXCCMAClient] == TRUE)
                {
#if (HW_DESIGN_4K2K_VER == 4)
                    // default frcm frame buffer address init
                    MDrv_XC_SetFRCMFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient]+au32CMAMemSCMSize[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], eWindow);
#endif
                    // default frame buffer address init
                    MDrv_XC_SetFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemSCMSize[enXCCMAClient], eWindow);
                }
            }
#ifdef UFO_XC_SUPPORT_DUAL_MIU
            if (u32DualMiuMemSize != 0)
            {
                // default dual frame buffer address init
                MDrv_XC_SetDualFrameBufferAddress(pInstance, u32CMAAddr[CMA_XC_COBUFF_MEM], u32DualMiuMemSize, eWindow);
            }
#endif
#if (HW_DESIGN_4K2K_VER == 4)
            if ((MDrv_XC_Is_DSForceIndexEnabled(pInstance, MAIN_WINDOW)||MDrv_XC_GetDynamicScalingStatus(pInstance))
                && IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType))
            {
                // MM source and DS on
                if(!gSrcInfo[eWindow].bInterlace)
                {
                    //DS ON , 4K2K case
                    Hal_SC_frcmw_Memory_Write_Request(pInstance, DISABLE, MAIN_WINDOW);
                }
                else
                {
                    Hal_SC_frcmw_Memory_Write_Request(pInstance, ENABLE, MAIN_WINDOW);
                }
            }
            else if ((gSrcInfo[eWindow].stCapWin.width > 3000)
                &&(gSrcInfo[eWindow].stCapWin.height> 1050))
            {
                Hal_SC_frcmw_Memory_Write_Request(pInstance, DISABLE, MAIN_WINDOW);
            }
            else
            {
                Hal_SC_frcmw_Memory_Write_Request(pInstance, ENABLE, MAIN_WINDOW);
            }
#endif
            Hal_SC_set_freezeimg(pInstance, DISABLE, MAIN_WINDOW);
        }
    }
    if (pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].isEnable == TRUE)
    {
        #ifdef CONFIG_MSTAR_SRAMPD
        // control SRAM PD here
        if (MHal_XC_Is_SRAM_PowerDown(pInstance) == TRUE)
        {
            // enable SRAM here
            MHal_XC_SRAM_PowerDown_Control(pInstance, FALSE);
        }
        #endif
    }
#endif
#endif

    // Get VB size from MVOP Timing
    if(IsSrcTypeDTV(gSrcInfo[eWindow].enInputSourceType) ||
        IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType))
    {
        MVOP_Timing stMVOPTiming;

        memset(&stMVOPTiming, 0, sizeof(MVOP_Timing));

#if !defined(CHIP_I2)
        if(eWindow == MAIN_WINDOW)
        {
            MDrv_MVOP_GetOutputTiming(&stMVOPTiming);
        }
        else
        {
            MDrv_MVOP_SubGetOutputTiming(&stMVOPTiming);
        }
#endif
        if (MDrv_XC_Get_VirtualBox_Info(pInstance,&pstXCVBox) == TRUE)
        {
            if(stMVOPTiming.u16Width > pstXCVBox.u16VBox_Htotal[eWindow])
            {
                gSrcInfo[eWindow].Status2.u16VirtualBox_Width = pstXCVBox.u16VBox_Htotal[eWindow];
            }
            else
            {
                gSrcInfo[eWindow].Status2.u16VirtualBox_Width = stMVOPTiming.u16Width;
            }

            if(stMVOPTiming.u16Height > pstXCVBox.u16VBox_Vtotal[eWindow])
            {
                gSrcInfo[eWindow].Status2.u16VirtualBox_Height = pstXCVBox.u16VBox_Vtotal[eWindow];
            }
            else
            {
                gSrcInfo[eWindow].Status2.u16VirtualBox_Height = stMVOPTiming.u16Height;
            }
        }
        else
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d] [DS] MDrv_XC_Get_VirtualBox_Info error!! u16VBox_Htotal:%d u16VBox_Vtotal:%d  \n",__func__,__LINE__,gSrcInfo[eWindow].Status2.u16VirtualBox_Width,gSrcInfo[eWindow].Status2.u16VirtualBox_Height);
        }

        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d] [DS] u16VBox_Htotal:%d u16VBox_Vtotal:%d  \n",__func__,__LINE__,gSrcInfo[eWindow].Status2.u16VirtualBox_Width,gSrcInfo[eWindow].Status2.u16VirtualBox_Height);
    }
#if 0
    if (MDrv_XC_Get_VirtualBox_Info(pInstance,&pstXCVBox) == TRUE)
    {
        gSrcInfo[eWindow].Status2.u16VirtualBox_Width = pstXCVBox.u16VBox_Htotal[eWindow];
        gSrcInfo[eWindow].Status2.u16VirtualBox_Height = pstXCVBox.u16VBox_Vtotal[eWindow];

        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d] [DS] u16VBox_Htotal:%d u16VBox_Vtotal:%d  \n",__func__,__LINE__,gSrcInfo[eWindow].Status2.u16VirtualBox_Width,gSrcInfo[eWindow].Status2.u16VirtualBox_Height);
    }
    else
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d] [DS] MDrv_XC_Get_VirtualBox_Info error!! u16VBox_Htotal:%d u16VBox_Vtotal:%d  \n",__func__,__LINE__,gSrcInfo[eWindow].Status2.u16VirtualBox_Width,gSrcInfo[eWindow].Status2.u16VirtualBox_Height);
    }
#endif

#if ((HW_DESIGN_4K2K_VER == 4)||(HW_DESIGN_4K2K_VER == 6)||(HW_DESIGN_4K2K_VER == 7))
    XC_InternalStatus SrcInfo;
    memcpy(&SrcInfo,&gSrcInfo[eWindow],sizeof(XC_InternalStatus));

    if(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow) )
    {
        if(  !gSrcInfo[eWindow].bInterlace
           ||(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_SAVE_MEM_MODE))
        {
             if(MDrv_XC_Is_SupportSWDS(pInstance,eWindow))
             {
                 SrcInfo.stCapWin.width     = gSrcInfo[eWindow].stCapWin.width;
                 SrcInfo.stCapWin.height    = gSrcInfo[eWindow].stCapWin.height;
             }
             else
             {
                 SrcInfo.stCapWin.width     = SrcInfo.Status2.u16VirtualBox_Width;
                 SrcInfo.stCapWin.height    = SrcInfo.Status2.u16VirtualBox_Height;
              }
        }
        else
        {
            SrcInfo.stCapWin.width     = gSrcInfo[eWindow].stCropWin.width;
            SrcInfo.stCapWin.height    = gSrcInfo[eWindow].stCropWin.height;
        }
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[%s,%5d] [DS] [PQSrcData] width:%d height:%d  \n",__func__,__LINE__,SrcInfo.stCapWin.width,SrcInfo.stCapWin.height);
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    _MDrv_XC_Set_PQ_SourceData(pInstance, eWindow, gSrcInfo[eWindow].enInputSourceType, &SrcInfo);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#else
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // Scaler must update source before width / height checking.
    _MDrv_XC_Set_PQ_SourceData(pInstance, eWindow, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow]);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#endif

    //if HW 3d ver > 1 in 3d case, sub PQ source type don't set, because there is no setwin(sub). So load first row in PQ table in quality_map_sub.
    //it need set sub PQ source type the same with Main's source type.
    if((MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow) != E_XC_3D_INPUT_MODE_NONE)
        && (MDrv_XC_Get_3D_HW_Version() > 1)
        && (eWindow == MAIN_WINDOW))
    {
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        _MDrv_XC_Set_PQ_SourceData(pInstance, SUB_WINDOW, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow]);
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    }

    // Capture width & height can not be 0 !!
    if (gSrcInfo[eWindow].stCapWin.width == 0 || gSrcInfo[eWindow].stCapWin.height == 0 ||
        gSrcInfo[eWindow].stCropWin.width == 0 || gSrcInfo[eWindow].stCropWin.height == 0 ||
        gSrcInfo[eWindow].stDispWin.width == 0 || gSrcInfo[eWindow].stDispWin.height == 0 )
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Cap/Crop/Disp width or height can not be 0 \n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return FALSE;
    }

    if(eWindow == SUB_WINDOW)
    {
        //For PIP, enable IP Read/write
        _XC_ENTRY(pInstance);
        MDrv_XC_SetScalerMemoryRequest(pInstance, TRUE, E_XC_MEMORY_IP_READ_REQUEST, SUB_WINDOW);
        MDrv_XC_SetScalerMemoryRequest(pInstance, TRUE, E_XC_MEMORY_IP_WRITE_REQUEST, SUB_WINDOW);
        _XC_RETURN(pInstance);
    }


    if (IsSrcTypeVga(gSrcInfo[eWindow].enInputSourceType)   ||
        IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) ||
        IsSrcTypeScart(gSrcInfo[eWindow].enInputSourceType) )
    {
        if(!((E_XC_3D_INPUT_MODE_NONE != MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow))
              && (SUB_WINDOW == eWindow)))// don't set adc mode when set sub window at 3d
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"ADC Set Mode \n");
            PQ_ADC_SAMPLING_INFO stPQADCSamplingInfo;
            memset(&stPQADCSamplingInfo,0,sizeof(PQ_ADC_SAMPLING_INFO));
            if ( IsSrcTypeVga(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) )
            {
                if(gSrcInfo[eWindow].bHDuplicate)
                {
                    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                    MDrv_XC_ADC_GetPQADCSamplingInfo(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], &stPQADCSamplingInfo, eWindow);
                    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
                }
            }

            MDrv_XC_ADC_SetMode(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], &stPQADCSamplingInfo, eWindow);
        }
    }
    else
    {
        MDrv_XC_ADC_SCART_RGB_setMode(pInstance,FALSE);
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

#ifdef ADC_BYPASS_YPBPR_MACRO_VISION_PATCH
    // patch for china player
    // there are some undefined signal between sync and DE in YPbPr fullHD
    // so we ignore this signal with coast window
    if(IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) && (!pstXC_SetWin_Info->bInterlace))
    {
        //MS_U16 u16ADC_PC_Clk = 0;
        //u16ADC_PC_Clk = MDrv_XC_ADC_GetPcClock(pInstance);
        //if(u16ADC_PC_Clk>2635 && u16ADC_PC_Clk<2645 )
        if((1920 == gSrcInfo[eWindow].stCapWin.width) && (1080 == gSrcInfo[eWindow].stCapWin.height))
        {
            MDrv_SC_ip_set_coast_window(pInstance,0x0F, 0x0F, eWindow );
        }
    }
#endif

    msAPI_Scaler_SetMode(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], eWindow);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#if ENABLE_REQUEST_FBL
    if(!(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow)) )
    {
        if ( (MDrv_XC_IsRequestFrameBufferLessMode(pInstance) && MDrv_XC_IsCurrentRequest_FrameBufferLessMode(pInstance))
             && (!MDrv_XC_IsCurrentFrameBufferLessMode(pInstance)) )
        {
            printf("====No Crop for RFBL==\n");

            gSrcInfo[eWindow].stCropWin.x = 0;
            gSrcInfo[eWindow].stCropWin.y = 0;
            gSrcInfo[eWindow].stCropWin.width = gSrcInfo[eWindow].stCapWin.width;
            gSrcInfo[eWindow].stCropWin.height= gSrcInfo[eWindow].stCapWin.height;
        }
    }
#endif

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"UCNR enabled: %s \n", (MDrv_XC_Get_OPWriteOffEnable(pInstance, eWindow) == TRUE) ? "NO" : "YES");

#ifdef ENABLE_SCALING_WO_MUTE
    if(DISABLE == gSrcInfo[eWindow].bBlackscreenEnabled)
    {
        MDrv_SC_Adjust_Prescaling_Ratio(pInstance, eWindow);
    }
#endif

#ifdef UFO_XC_SET_DSINFO_V0
    if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
    {
        MDrv_SC_set_2p_mode(pInstance, pstXC_SetWin_Info, eWindow);
    }
#else
    MDrv_SC_set_2p_mode(pInstance, pstXC_SetWin_Info, eWindow);
#endif

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    MDrv_SC_set_prescaling_ratio(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], eWindow );
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CapWin  x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCapWin.x, gSrcInfo[eWindow].stCapWin.y, gSrcInfo[eWindow].stCapWin.width, gSrcInfo[eWindow].stCapWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CropWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCropWin.x, gSrcInfo[eWindow].stCropWin.y, gSrcInfo[eWindow].stCropWin.width, gSrcInfo[eWindow].stCropWin.height);
    // SCMI miu dual mode Phase1: Dual enable under hdmirx 4k2k, dual bw ration 2:2
    // User should judge when enable dual by call dual enable api. So remove auto start dual enable.
#if 0
#ifndef DONT_USE_CMA
#ifdef UFO_XC_SUPPORT_DUAL_MIU
    if (eWindow == MAIN_WINDOW)
    {
        if (IsSrcTypeHDMI(gSrcInfo[eWindow].enInputSourceType) && ((gSrcInfo[(eWindow)].u16H_SizeAfterPreScaling> 1920) && (gSrcInfo[(eWindow)].u16V_SizeAfterPreScaling> 1080)))
        {
            MDrv_XC_EnableMiuDualMode(pInstance, TRUE, 2, 2, MAIN_WINDOW);
        }
    }
#endif
#endif
#endif

    MDrv_SC_set_crop_window(pInstance, &gSrcInfo[eWindow], eWindow );

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CapWin  x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCapWin.x, gSrcInfo[eWindow].stCapWin.y, gSrcInfo[eWindow].stCapWin.width, gSrcInfo[eWindow].stCapWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CropWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCropWin.x, gSrcInfo[eWindow].stCropWin.y, gSrcInfo[eWindow].stCropWin.width, gSrcInfo[eWindow].stCropWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"DispWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stDispWin.x,gSrcInfo[eWindow].stDispWin.y, gSrcInfo[eWindow].stDispWin.width, gSrcInfo[eWindow].stDispWin.height);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    MDrv_SC_set_postscaling_ratio(pInstance, &gSrcInfo[eWindow], eWindow );

#ifdef ENABLE_TV_SC2_PQ
    if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
    {
        MS_BOOL bTmpFBL = FALSE;
#ifdef UFO_XC_FB_LEVEL
        if((gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_FBL) || (gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_RFBL_DI))
        {
            bTmpFBL = TRUE;
        }
#else
        bTmpFBL = gSrcInfo[eWindow].bFBL;
#endif
        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling != NULL)
        {
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling(
                            (psXCInstPri->u32DeviceID == 1) ?
                            PQ_SUB_WINDOW :
                            PQ_MAIN_WINDOW,
                            bTmpFBL,
                            gSrcInfo[eWindow].bPreV_ScalingDown,
                            gSrcInfo[eWindow].ScaledCropWin.y);
#else
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling(
                            eWindow == SUB_WINDOW ?
                            PQ_SUB_WINDOW :
                            PQ_MAIN_WINDOW,
                            bTmpFBL,
                            gSrcInfo[eWindow].bPreV_ScalingDown,
                            gSrcInfo[eWindow].ScaledCropWin.y);
#endif
        }
    }
#ifdef ENABLE_TV_SC2_PQ
    else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
    {
            MS_BOOL bTmpFBL = FALSE;
#ifdef UFO_XC_FB_LEVEL
        if((gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_FBL) || (gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_RFBL_DI))
        {
            bTmpFBL = TRUE;
        }
#else
        bTmpFBL = gSrcInfo[eWindow].bFBL;
#endif

        if (s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling_ex != NULL)
        {
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling_ex(psXCInstPri->u32DeviceID,
                            eWindow == SUB_WINDOW ?
                            PQ_SUB_WINDOW :
                            PQ_MAIN_WINDOW,
                            bTmpFBL,
                            gSrcInfo[eWindow].bPreV_ScalingDown,
                            gSrcInfo[eWindow].ScaledCropWin.y);
        }
    }
#endif
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    // FIXME: direct use variable instead of stDBreg?
    _XC_ENTRY(pInstance);

    //for 3D handle invalid crop window,or else LR status may be inverse.
    if(E_XC_3D_INPUT_CHECK_BORAD == MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow))
    {
        gSrcInfo[eWindow].ScaledCropWin.y &= ~0x1;
        gSrcInfo[eWindow].ScaledCropWin.x &= ~0x1;
        gSrcInfo[eWindow].ScaledCropWin.height &= ~0x1;
        gSrcInfo[eWindow].ScaledCropWin.width &= ~0x1;
    }
    else if(E_XC_3D_INPUT_LINE_ALTERNATIVE == MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow))
    {
        gSrcInfo[eWindow].ScaledCropWin.y &= ~0x1;
        gSrcInfo[eWindow].ScaledCropWin.height &= ~0x1;
    }
    else if(E_XC_3D_INPUT_PIXEL_ALTERNATIVE == MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow))
    {
        gSrcInfo[eWindow].ScaledCropWin.x &= ~0x1;
        gSrcInfo[eWindow].ScaledCropWin.width &= ~0x1;
    }

#if (HW_DESIGN_4K2K_VER == 4)
    // temp solution for monaco
    // MDrv_SC_set_fetch_number_limit will change by MDrv_SC_set_frcm_fetch_number_limit when it stable
    // but function name always is MDrv_SC_set_fetch_number_limit
    MDrv_SC_set_frcm_fetch_number_limit(pInstance, &gSrcInfo[eWindow], eWindow );
#else
    MDrv_SC_set_fetch_number_limit(pInstance, &gSrcInfo[eWindow], eWindow );
#endif
    MDrv_sc_set_capture_window(pInstance, eWindow );

#if VERIFY_MVIDEO_FPGA //non-de-interlace function=> celear second frame half garbage
    if(binterlace)
    {
        gSrcInfo[eWindow].stDispWin.height/= 2;
        gSrcInfo[eWindow].u16InputVTotal /= 2;
    }
#endif

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    // For Manhattan, FSC+FRC
    if ((eWindow == MAIN_WINDOW) &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled))
    {
        if (bChangeDisplaySize)
        {
            // FHD to FSC case, but bk10 display window need to be original size
            gSrcInfo[eWindow].stDispWin.x = gSrcInfo[eWindow].stDispWin.x*2 - pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart;
            gSrcInfo[eWindow].stDispWin.width = gSrcInfo[eWindow].stDispWin.width*2;
            gSrcInfo[eWindow].stDispWin.y = gSrcInfo[eWindow].stDispWin.y*2 - pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart;
            gSrcInfo[eWindow].stDispWin.height = gSrcInfo[eWindow].stDispWin.height*2;

            // for custom scaling case
            if(gSrcInfo[eWindow].bHCusScaling)
            {
                gSrcInfo[eWindow].u16HCusScalingDst = gSrcInfo[eWindow].u16HCusScalingDst*2;
            }
            if(gSrcInfo[eWindow].bVCusScaling)
            {
                gSrcInfo[eWindow].u16VCusScalingDst = gSrcInfo[eWindow].u16VCusScalingDst*2;
            }
        }
    }
#endif

#ifdef PATCH_TCON_BRING_UP
        if((((MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type) == E_XC_PNL_LPLL_EPI28_12P)
          && !(strcmp(panelName_UD_VB1_8LANE_DRDEPI,MDrv_PNL_GetName())))
        {
            gSrcInfo[eWindow].stDispWin.y = gSrcInfo[eWindow].stDispWin.y+1;
        }
#endif

    // FIXME: direct use variable instead of stDBreg?
    MDrv_SC_set_display_window(pInstance,  eWindow );

    #ifdef MONACO_SC2_PATCH
    // patch!!
    if(psXCInstPri->u32DeviceID == 1)
    {
        Hal_SC_set_de_window(pInstance ,&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo);
    }
    #endif

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Physical DispWin HStart: %4u  HEnd: %4u  Vstart: %4u  VEnd: %4u \n",
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16H_DisStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16H_DisEnd,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16V_DisStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16V_DisEnd);

    // Adjust FCLK by different case
#ifdef UFO_XC_FB_LEVEL
    if((gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_FBL)  || (gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_RFBL_DI))//FIXME
#else
    if (gSrcInfo[eWindow].bFBL)
#endif
    {
        MDrv_SC_set_Fclk(pInstance, EN_FCLK_FBL);
    }
    else
    {
        MDrv_SC_set_Fclk(pInstance, EN_FCLK_NORMAL);
    }

    // Adjust display line buffer start mode by different case
#ifdef UFO_XC_FB_LEVEL
    if((gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_FBL)  || (gSrcInfo[eWindow].eFBLevel == E_XC_FB_LEVEL_RFBL_DI))
#else
    if (gSrcInfo[eWindow].bFBL || gSrcInfo[eWindow].bR_FBL)
#endif
    {
        MDrv_SC_SetDisplay_Start_Mode(pInstance, DISABLE);
    }
    else
    {
        MDrv_SC_SetDisplay_Start_Mode(pInstance, ENABLE);
    }

    if (bDualWindow == TRUE)
    {
        if (eWindow == MAIN_WINDOW)
        {
            memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.stMainDBreg, &pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, sizeof(SC_SWDB_INFO));
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualMainWinDone = TRUE;
        }
        else
        {
            memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.stSubDBreg
                   , &pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, sizeof(SC_SWDB_INFO));
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualSubWinDone = TRUE;
        }

        if (pXCResourcePrivate->stdrvXC_MVideo._bSetDualMainWinDone
            && pXCResourcePrivate->stdrvXC_MVideo._bSetDualSubWinDone)
        {
            MDrv_SC_dual_sw_db(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.stMainDBreg
                               , &pXCResourcePrivate->stdrvXC_MVideo_Context.stSubDBreg);
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualMainWinDone = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualSubWinDone = FALSE;
        }
    }
    else
    {
    #ifndef STELLAR
        if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
        {
            MDrv_SC_sw_db(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, eWindow );
        }
    #endif


#ifdef STELLAR
#if (defined (ANDROID))
#if (defined (ANDROID)&& defined(UFO_XC_DS_PQ))
        MDrv_PQ_set_imode_PQDS(pstXC_SetWin_Info->bInterlace);
#endif
        MDrv_SWDS_AddCmd(pInstance,&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg,eWindow);
        MDrv_SWDS_Fire(pInstance,eWindow);
#else

#ifdef UFO_XC_SET_DSINFO_V0
        //printf("[%s,%5d]MDrv_XC_GetDynamicScalingStatus():%d MDrv_XC_Is_DSForceIndexEnabled():%d  \n",__FUNCTION__,__LINE__,MDrv_XC_GetDynamicScalingStatus(pInstance),MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow));
        //printf("[%s,%5d]bUpdate_DS_CMD():%d  \n",__FUNCTION__,__LINE__,gSrcInfo[MAIN_WINDOW].Status2.bUpdate_DS_CMD);
        if(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow))
        {
            if(gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)//update ds cmd
            {
                //printf("[%s,%5d]  MDrv_SWDS_AddCmd  \n",__FUNCTION__,__LINE__);
                MDrv_SWDS_AddCmd(pInstance,&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg,eWindow);
                MDrv_SWDS_Fire(pInstance,eWindow);
            }
            else
            {
                //printf("[%s,%5d]  MDrv_SC_sw_db  \n",__FUNCTION__,__LINE__);
                MDrv_SC_sw_db(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, eWindow);
            }
        }
        else
#endif
        {
            //printf("[%s,%5d]  MDrv_SC_sw_db  \n",__FUNCTION__,__LINE__);
            MDrv_SC_sw_db(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, eWindow);
        }
#endif //STELLAR


    #endif




    }

#ifndef STELLAR

#if (SUPPORT_3D_DS == 0)
#if ((HW_DESIGN_4K2K_VER == 1)||(HW_DESIGN_4K2K_VER == 4)||(HW_DESIGN_4K2K_VER == 6)||(HW_DESIGN_4K2K_VER == 7))
    if(gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
    {
#if (defined (ANDROID)&& defined(UFO_XC_DS_PQ))
        MDrv_PQ_set_imode_PQDS(pstXC_SetWin_Info->bInterlace);
#endif
        MDrv_SWDS_AddCmd(pInstance,&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg,eWindow);
        MDrv_SWDS_Fire(pInstance,eWindow);
    }
#endif
#endif

#endif

    //For VIP peaking setting
    MDrv_XC_VIP_Peaking_Setting(pInstance, eWindow);

#if SUPPORT_SEAMLESS_ZAPPING
    if(INPUT_SOURCE_DTV == gSrcInfo[eWindow].enInputSourceType)
    {
        //When HDMI 4K2K open compress mode change to PX2 mode need to toggle the compress flag clear
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK08_40_L, BIT(0), BIT(0));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK08_40_L, 0, BIT(0));
    }
#endif

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if(bRet == FALSE)
    {
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        MDrv_XC_3D_LoadReg(pInstance, MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow), MDrv_XC_Get_3D_Output_Mode(pInstance), &gSrcInfo[eWindow], eWindow);
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if(!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_HDMITX_ENABLE)
            && pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled)
        {
            Hal_SC_3D_Enable_FRC_LR_Flag(pInstance, FALSE); //disable FRC LR  flag
            MDrv_XC_FRC_Set_Input3DFormat(pInstance, E_XC_3D_INPUT_MODE_NONE);
            MDrv_XC_FRC_R2_Set_InputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
            MDrv_XC_FRC_R2_Set_OutputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
        }
#endif
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    }

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 3)) // only Napoli need this
    // SW patch for MM 4K BW issue
    if ((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal > 3000)
        && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VTotal > 2000)
        && (pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16OPMFetch > 1920)
        && (pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16VLen > 1080))
    {
        /// for BW issue
        if ((IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType))||(IsSrcTypeHDMI(gSrcInfo[eWindow].enInputSourceType)))
        {
            // SC
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_02_L, BIT(14), BIT(14));  // turn off ip read for bw issue
        }
        else
        {
#ifdef UFO_XC_FB_LEVEL
            if(gSrcInfo[MAIN_WINDOW].eFBLevel == E_XC_FB_LEVEL_FB)//FIXME
#else
            if(!gSrcInfo[MAIN_WINDOW].bFBL)
#endif
            {
                SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_02_L, 0x00, BIT(14));
            }
        }
    }
    else
    {
#ifdef UFO_XC_FB_LEVEL
        if(gSrcInfo[MAIN_WINDOW].eFBLevel == E_XC_FB_LEVEL_FB)//FIXME
#else
        if(!gSrcInfo[MAIN_WINDOW].bFBL)
#endif
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_02_L, 0x00, BIT(14));
        }
    }
    // SC opm line buffer lens
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_24_L, 0xA03E); // OP read request
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_68_L, 0x3E00); // IPM arbiter
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_63_L, 0x3F00); // IPM arbiter
    SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_22_L, 0x2020); // IPM request length

#endif

    #if (HW_DESIGN_4K2K_VER == 4)
    //SW patch for HDMI 4K@60 BW issue, mantis: 782590
    if((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HTotal > 3000)          // 4K output
        && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VTotal > 2000)
        && (IsSrcTypeHDMI(gSrcInfo[eWindow].enInputSourceType))                                         // HDMI
        && (pstXC_SetWin_Info->stCapWin.width >= 0x780)                                                 // 4K2K@50,60 input
        && (pstXC_SetWin_Info->stCapWin.height >= 0x870)
        && (pstXC_SetWin_Info->u16InputVFreq >= 490))
    {
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK32_24_L, 0x0008, 0x00FF);  // OP read request
    }
    else
    {
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK32_24_L, 0x0020, 0x00FF);
    }

    MS_U16 u16Height  = SC_R2BYTEMSK(0, REG_SC_BK01_06_L, 0x1FFF);
    MS_U16 u16Vtt     = SC_R2BYTEMSK(0, REG_SC_BK01_1F_L, 0x1FFF);
    if (  (pstXC_SetWin_Info->enInputSourceType == INPUT_SOURCE_VGA)
        &&(u16Vtt - u16Height <= 5))
    {
        MDrv_XC_MLoad_set_IP_trig_p(pInstance, 0x02, 0x03);
        MDrv_XC_MemControl_Switch_Method_By_Vcnt(pInstance, FALSE, 0x0);
    }
    else if(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow) )
    {
        MDrv_XC_MLoad_set_IP_trig_p(pInstance, 0x01, 0x02);
        MDrv_XC_MemControl_Switch_Method_By_Vcnt(pInstance, FALSE, 0x00);
        MDrv_XC_edclk_pd_control(pInstance, FALSE, 0x00, 0x00);
    }

#if(HW_DESIGN_3D_VER == 5) //monaco/muji mode
   //several lines of garbage on the top FPI to SBSH
   else if ((MApi_XC_Get_3D_Input_Mode(MAIN_WINDOW) == E_XC_3D_INPUT_FRAME_PACKING)
         && (pstXC_SetWin_Info->stCapWin.height == DOUBLEHD_1080X2I_VSIZE))
    {
        MS_U16 u16TrainTriggerPoint = 0;
        MS_U16 u16DispTriggerPoint = 0;
        MDrv_XC_MLoad_get_IP_trig_p(pInstance, &u16TrainTriggerPoint, &u16DispTriggerPoint);
        if((u16TrainTriggerPoint != 0x00) && (u16DispTriggerPoint != 0x01))
        {
            MDrv_XC_MLoad_get_IP_trig_p(pInstance, &pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_AUTO_TUNE_AREA_TRIG], &pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_DISP_AREA_TRIG]);
            MDrv_XC_MLoad_set_IP_trig_p(pInstance, 0x0000, 0x0001);
        }
    }
#endif

    else
    {
        MS_U16 u16TrainTriggerPoint = 0;
        MS_U16 u16DispTriggerPoint = 0;
        MDrv_XC_MLoad_get_IP_trig_p(pInstance, &u16TrainTriggerPoint, &u16DispTriggerPoint);

        if ((pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_AUTO_TUNE_AREA_TRIG] != u16TrainTriggerPoint)
            || (pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_DISP_AREA_TRIG] != u16DispTriggerPoint))
        {
            MDrv_XC_MemControl_Switch_Method_By_Vcnt(pInstance, TRUE, 0x04);
            MDrv_XC_MLoad_set_IP_trig_p(pInstance, pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_AUTO_TUNE_AREA_TRIG], pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_DISP_AREA_TRIG]);
        }
    }

    if( !(MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow)) )
    {
        // not need open edclk control when vblanking is little
        if (u16Vtt - u16Height <= 24)
        {
            MDrv_XC_edclk_pd_control(pInstance, FALSE, 0x00, 0x00);
        }
        else
        {
            MDrv_XC_edclk_pd_control(pInstance, TRUE, 0x04, 0x08);
        }
    }

    //DTV 1080i patch for mantis:0735773
    MDrv_XC_DTVPatch(pInstance,eWindow);

#endif

    if((((MDrv_XC_Is_DSForceIndexEnabled(pInstance, MAIN_WINDOW)||MDrv_XC_GetDynamicScalingStatus(pInstance)) && (IS_DS_4K2K(eWindow)) && (gSrcInfo[eWindow].u16InputVFreq > 330)) //UHD DS case
           ||(IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType) && IS_INPUT_4K2K(eWindow) && (gSrcInfo[eWindow].u16InputVFreq > 330))))
    {
#if (defined(USE_MVOP_VS_TO_GEN_VS_TO_OPVS) && USE_MVOP_VS_TO_GEN_VS_TO_OPVS == TRUE)
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID,REG_SC_BK02_10_L, BIT(2), BIT(2));
#endif

#if (defined(PATCH_ENABLE_MVOP_HANDSHAKE_MODE_WHEN_BW_NOT_ENOUGH_IN_120HZ_PANEL))
#if (PATCH_ENABLE_MVOP_HANDSHAKE_MODE_WHEN_BW_NOT_ENOUGH_IN_120HZ_PANEL == TRUE)
        if(IsVBY1_16LANE(u8LPLL_Type))
        {
            //MVOP
            MS_BOOL bHSK = TRUE;
            MVOP_Handle stMvopHd = { E_MVOP_MODULE_MAIN };
            MDrv_MVOP_SetCommand(&stMvopHd, E_MVOP_CMD_SET_HANDSHAKE_MODE, &bHSK);
            //XC  0x1302_10 = 0x25
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID,REG_SC_BK02_10_L, 0x0025, 0xFFFF);
        }
#endif
#endif
    }
    else
    {
#if (defined(USE_MVOP_VS_TO_GEN_VS_TO_OPVS) && USE_MVOP_VS_TO_GEN_VS_TO_OPVS == TRUE)
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID,REG_SC_BK02_10_L, 0, BIT(2));
#endif

#if (defined(PATCH_ENABLE_MVOP_HANDSHAKE_MODE_WHEN_BW_NOT_ENOUGH_IN_120HZ_PANEL))
#if (PATCH_ENABLE_MVOP_HANDSHAKE_MODE_WHEN_BW_NOT_ENOUGH_IN_120HZ_PANEL == TRUE)
        //MVOP
        MS_BOOL bHSK = FALSE;
        MVOP_Handle stMvopHd = { E_MVOP_MODULE_MAIN };
        MDrv_MVOP_SetCommand(&stMvopHd, E_MVOP_CMD_SET_HANDSHAKE_MODE, &bHSK);
        //XC  0x1302_10 = 0x00
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID,REG_SC_BK02_10_L, 0x0000, 0xFFFF);
#endif
#endif
    }

#if (HW_DESIGN_4K2K_VER == 6)
    if(IsSrcTypeDTV(gSrcInfo[MAIN_WINDOW].enInputSourceType) || (IsSrcTypeStorage(gSrcInfo[MAIN_WINDOW].enInputSourceType)))//MVOP source
    {
        if(SC_R2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_10_L, BIT(2)) == 0)
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_5D_L, 0x1018 , 0xFFFF);//return to HW default value
        }
    }
#endif

    //control HSK mode and GenTimingFromXC mode
    if(MDrv_SC_CheckMuteStatusByRegister(pInstance, eWindow))
    {
        Hal_SC_MVOP_HandShake_Control(pInstance,eWindow);
        Hal_SC_MVOP_Gen_Timing_From_XC_Control(pInstance,eWindow);
    }

#if (HW_DESIGN_4K2K_VER == 7)  //Kano Patch
    if((psXCInstPri->u32DeviceID==0) && (eWindow == MAIN_WINDOW))
    {
        //for 60in/24out tearing issue, HW says XC FRC case need enable BK11_05[9]. but need more test to confirm this solution is OK.
        //BK11_05[9] use to protect write too fast.
        if((((gSrcInfo[eWindow].enInputSourceType)==INPUT_SOURCE_STORAGE) || ((gSrcInfo[eWindow].enInputSourceType)==INPUT_SOURCE_STORAGE2))
            && gSrcInfo[eWindow].bInterlace)
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK11_05_L, BIT(9) , BIT(9));
        }
        else
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK11_05_L, 0 , BIT(9));
        }
    }
#endif
    if (((pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable == TRUE) ||
         (pXCResourcePrivate->sthal_Optee.op_tee_xc[SUB_WINDOW].isEnable == TRUE)) &&
        ((gSrcInfo[eWindow].Status2.eMirrorMode == MIRROR_H_ONLY) ||
         (gSrcInfo[eWindow].Status2.eMirrorMode == MIRROR_V_ONLY) ||
         (gSrcInfo[eWindow].Status2.eMirrorMode == MIRROR_HV)))
    {
        XC_OPTEE_HANDLER st_xc_handler;
        memset(&st_xc_handler,0,sizeof(XC_OPTEE_HANDLER));
        st_xc_handler.eWindow = eWindow;
        MDrv_XC_OPTEE_Update(pInstance, E_XC_OPTEE_UPDATE_HANDLER, st_xc_handler);
    }
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    u32SetWindowTime = MsOS_GetSystemTime()-u32SetWindowTime;
#endif
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Set the specific window
/// @param  pstXC_SetWin_Info      \b IN: the information of the window setting
/// @param  u32InitDataLen         \b IN: the length of the pstXC_SetWin_Info
/// @param  eWindow                \b IN: which window we are going to set
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_SetWindow_U2(void* pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, MS_U32 u32InitDataLen, SCALER_WIN eWindow)
{
    MS_BOOL bRet = FALSE;
    MS_BOOL bEnable8LBMode = FALSE;
    XC_INITMISC stXC_Init_Misc;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    if (pstXC_SetWin_Info == NULL)
    {
        printf("[%s,%5d]Invalid parameter !\n",__FUNCTION__,__LINE__);
        return FALSE;
    }
#ifdef MSOS_TYPE_LINUX_KERNEL
    /*
    ** Save setwindow info for str
    */
    void *pModule;
    UtopiaInstanceGetModule(pInstance, &pModule);
    XC_REGS_SAVE_AREA *pXCResourceStr = NULL;
    UtopiaModuleGetSTRPrivate(pModule, (void**)&pXCResourceStr);
    if (pstXC_SetWin_Info != NULL)
    {
        memcpy(&pXCResourceStr->pstXC_SetWin_Info[eWindow],pstXC_SetWin_Info,sizeof(XC_SETWIN_INFO));
    }
#endif
    _SETWINDOW_ENTRY();

    PSTXC_DS_CMDCNT.u16CMDCNT_IPM =0;
    PSTXC_DS_CMDCNT.u16CMDCNT_IPS =0;
    PSTXC_DS_CMDCNT.u16CMDCNT_OPM =0;
    PSTXC_DS_CMDCNT.u16CMDCNT_OPS =0;

    memset(&stXC_Init_Misc,0,sizeof(XC_INITMISC));

    if(MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
    {
        g_bSWDSGenScenario = TRUE;
    }
    else
    {
        g_bSWDSGenScenario = FALSE;
    }

#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    //Only SC0 can set HDR (MDrv_XC_HDR_SetWindow can only set sc0 to HDR)
    if ((psXCInstPri->u32DeviceID == 0) && (eWindow == MAIN_WINDOW) && (pstXC_SetWin_Info != NULL))
    {
        if (gSrcInfo[eWindow].bR_FBL)
        {
            MDrv_XC_HDR_SetWindow(pInstance, pstXC_SetWin_Info->stCropWin);
        }
        else
        {
            MDrv_XC_HDR_SetWindow(pInstance, pstXC_SetWin_Info->stCapWin);
        }
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
#endif
#endif

    if (_MApi_XC_SetWindow(pInstance, pstXC_SetWin_Info, sizeof(XC_SETWIN_INFO), FALSE, eWindow) == FALSE)
    {
        goto SETWINDOW_EXIT;
    }

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    MDrv_XC_3D_LoadReg(pInstance, MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow), MDrv_XC_Get_3D_Output_Mode(pInstance), &gSrcInfo[eWindow], eWindow);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    if(!(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_B & E_XC_INIT_MISC_B_HDMITX_ENABLE)
        && pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled)
    {
        if(pXCResourcePrivate->stdrvXC_3D._bFRC3DEnabled )//FRC 3D enabled
        {
            MDrv_XC_FRC_Set_Input3DFormat(pInstance, pXCResourcePrivate->stdrvXC_3D._eFrcInput3DMode);

            if(E_XC_3D_OUTPUT_FRAME_ALTERNATIVE == pXCResourcePrivate->stdrvXC_3D._eFrcOutput3DMode)//frc output FA
            {
                Hal_SC_3D_Enable_FRC_LR_Flag(pInstance, TRUE); //use FRC LR flag
            }
            else
            {
                Hal_SC_3D_Enable_FRC_LR_Flag(pInstance, FALSE); //use SC LR flag
            }

            if(MHal_FRC_IsEnableFRC3D(pInstance, MAIN_WINDOW))
            {
                if(IS_OUTPUT_4K1K_120HZ_PANEL() || IS_OUTPUT_4K0_5K_240HZ_PANEL())
                {
                    MDrv_XC_FRC_R2_Set_InputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height*2);
                    MDrv_XC_FRC_R2_Set_OutputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
                }
            }
        }
        else
        {
            Hal_SC_3D_Enable_FRC_LR_Flag(pInstance, FALSE); //disable FRC LR  flag
            MDrv_XC_FRC_Set_Input3DFormat(pInstance, E_XC_3D_INPUT_MODE_NONE);
            MDrv_XC_FRC_R2_Set_InputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
            MDrv_XC_FRC_R2_Set_OutputFrameSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width, \
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
        }
    }
#endif

#if (SUPPORT_3D_DS == 1)
#if ((HW_DESIGN_4K2K_VER == 4)||(HW_DESIGN_4K2K_VER == 6))
#ifdef STELLAR
     if(MDrv_XC_Is_SupportSWDS(pInstance) && (MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, eWindow) ))
#else
    if(MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
#endif
    {
#if (defined (ANDROID)&& defined(UFO_XC_DS_PQ))
        MDrv_PQ_set_imode_PQDS(pstXC_SetWin_Info->bInterlace);
#endif
        MDrv_SWDS_AddCmd(pInstance,&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg,eWindow);
        MDrv_SWDS_Fire(pInstance,eWindow);
    }
#endif
#endif
    bRet = TRUE;

    //all return should come here, because this code will enable opm write limit addr protect
SETWINDOW_EXIT:
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    memcpy(&(gSrcInfo[eWindow].stStatusnodelay.stDispWin), &(gSrcInfo[eWindow].stDispWin), sizeof(MS_WINDOW_TYPE));

    if(eWindow == MAIN_WINDOW)
    {
    #if (ENABLE_OPM_WRITE_SUPPORTED)
        MS_PHY u32OPWLimitBase = 0x00;
        u32OPWLimitBase = ((MDrv_XC_GetIPMBase(pInstance, 0, eWindow)*BYTE_PER_WORD + MDrv_XC_GetDNRBufSize(pInstance, eWindow)) / BYTE_PER_WORD - 1) ;
        _XC_ENTRY(pInstance);
        MDrv_SC_set_opm_write_limit(pInstance, TRUE, 0 , u32OPWLimitBase , eWindow);
        _XC_RETURN(pInstance);
#ifdef UFO_XC_SUPPORT_DUAL_MIU
        _XC_ENTRY(pInstance);
        u32OPWLimitBase = ((MDrv_XC_GetDualIPMBase(pInstance, 0, eWindow)*BYTE_PER_WORD + MDrv_XC_GetDualDNRBufSize(pInstance, eWindow)) / BYTE_PER_WORD - 1) ;
        MDrv_SC_set_dual_opm_write_limit(pInstance, FALSE, 0 , u32OPWLimitBase , eWindow);
        _XC_RETURN(pInstance);
#endif

    #endif
    }

//there is xc performance issue for napoli hw, so need enable 8 LB mode by designer's advice
#if ENABLE_DI_8LB_MODE
    bEnable8LBMode = ((MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow) == E_XC_3D_INPUT_MODE_NONE)
                     && (MDrv_XC_Get_3D_Output_Mode(pInstance) == E_XC_3D_OUTPUT_MODE_NONE)
                     && MDrv_XC_Is2K2KToFrc(pInstance)
                     && (((gSrcInfo[MAIN_WINDOW].stCapWin.width == DOUBLEHD_1080X2P_HSIZE)
                             && (gSrcInfo[MAIN_WINDOW].stCapWin.height > (DOUBLEHD_1080X2P_VSIZE - DOUBLEHD_1080X2P_GARBAGE_VSIZE - 10))
                             && (!IsSrcTypeStorage(gSrcInfo[eWindow].enInputSourceType))) // MM case, enable UC costs too much BW, make video temp garbage, so not enable UC for MM
                         || (MApi_XC_Is_SubWindowEanble()//For dualview case: enable DI8LB to eliminate bubble for scaling performance issue.
                             && (gSrcInfo[SUB_WINDOW].stDispWin.height ==1080)&& (gSrcInfo[SUB_WINDOW].stDispWin.width ==1920))));
#endif

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);


#if (HW_DESIGN_4K2K_VER == 6)
// For HW limitation, we need disable UC feature in Src * 1.5 > Dst(Post Horizontal Scaling)case   (OPW is 1P engine)
    if(((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width == 3840) && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height == 2160))&&
       (IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeHDMI(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeDVI(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeDTV(gSrcInfo[eWindow].enInputSourceType))&&
       (gSrcInfo[eWindow].u16H_SizeAfterPreScaling * 15 >  gSrcInfo[eWindow].stDispWin.width * 10))
    {
        if((MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED) && (gSrcInfo[MAIN_WINDOW].bBlackscreenEnabled == FALSE))
        {
            _MLOAD_ENTRY(pInstance);
            MDrv_XC_MLoad_WriteCmd(pInstance, REG_SC_BK2A_02_L, 0x0, BIT(15));
            MDrv_XC_MLoad_Fire(pInstance, TRUE);
            _MLOAD_RETURN(pInstance);
        }
        else
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK2A_02_L, 0x0 ,BIT(15));//disable UC feature
        }
    }
   else
#endif
   {
#ifdef ENABLE_TV_SC2_PQ
    if (psXCInstPri->u32DeviceID == PQ_XC_ID_0)
#endif
    {
        if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl)
        {
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl((psXCInstPri->u32DeviceID == 1)?PQ_SUB_WINDOW:PQ_MAIN_WINDOW,
                               E_PQ_IOCTL_SET_UCFEATURE,
                               (void *)&bEnable8LBMode,
                               sizeof(MS_BOOL));
#else
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl((PQ_WIN)eWindow,
                                       E_PQ_IOCTL_SET_UCFEATURE,
                                       (void *)&bEnable8LBMode,
                                       sizeof(MS_BOOL));
#endif
        }
    }
#ifdef ENABLE_TV_SC2_PQ
    else if (psXCInstPri->u32DeviceID == PQ_XC_ID_1)
    {
        if(s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl_ex)
        {
            s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl_ex(psXCInstPri->u32DeviceID, (PQ_WIN)eWindow,
                                       E_PQ_IOCTL_SET_UCFEATURE,
                                       (void *)&bEnable8LBMode,
                                       sizeof(MS_BOOL));
        }
    }
#endif
   }

#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    // For Manhattan, FSC+FRC
    // 1. load FSC Qmap here
    // 2. load FRC 3D table here
    // 3. send R2 cmd to FRC
    // 4. control FRC clk
    // 5. control ODCLK2, composer, and TGEN lock_src
    if (eWindow == MAIN_WINDOW)
    {
        if ((pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled) && (psXCInstPri->u32DeviceID != 1))
        {
            // for DS case, it would update MDE at DS cmd
            // for other case, if AP only call setwindow without setpaneltiming, need to update MDE here
#if defined(UFO_XC_SET_DSINFO_V0)
            if(!gSrcInfo[eWindow].Status2.bUpdate_DS_CMD)
#else
            if(!MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
#endif
            {
                static MS_BOOL bIsFhdToFsc = TRUE;
                if (bIsFhdToFsc == MHal_FRC_IsFHDToFSC(pInstance, eWindow))
                {
                    //commands move to mload
                    //MDrv_FRC_UpdateMDE(pInstance, gSrcInfo[MAIN_WINDOW].stDispWin);
                }
                else
                {
                    MS_BOOL bMLoad = (MDrv_XC_MLoad_GetStatus(pInstance) == E_MLOAD_ENABLED)?TRUE:FALSE;
                    if(bMLoad)
                    {
                        _MLOAD_ENTRY(pInstance);
                    }

                    MST_PANEL_INFO_t stFRCPanelInfo;
                    MDrv_FRC_PNLInfo_Transform(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo,&pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc, &stFRCPanelInfo);
                    MDrv_FRC_Tx_SetTgen(pInstance, &stFRCPanelInfo);

                    MS_BOOL bLockFromIP = Hal_SC_IsLockFromIP(pInstance, eWindow);

                    //for Maserati 120Hz
                    MS_U8 u8LPLL_Type =(MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type;
                    if(IsVBY1_16LANE(u8LPLL_Type))
                    {
                        // case 5: FSC + FRC
                        if (MHal_FRC_IsFHDToFSC(pInstance, eWindow))
                        {
                            // FHD case, ODCLK2 as ODCLK/4
                            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_4, bMLoad);
                        }
                        else
                        {
                            // 4K 50/60 case, ODLCK2 as ODCLK
                            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_1, bMLoad);
                        }
                    }
                    else
                    {
                        if (MHal_FRC_IsFHDToFSC(pInstance, eWindow))
                        {
                            // FHD case, ODCLK2 as ODCLK/4
                            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_4, bMLoad);
                        }
                        else if ((gSrcInfo[eWindow].u16InputVFreq < 330) && bLockFromIP)
                        {
                            // FRC enable, sync from IP case
                            // 4K 24/25/30 case, ODCLK2 as ODCLK/2 for power saving
                            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_2, bMLoad);
                        }
                        else
                        {
                            // 4K 50/60 case, ODLCK2 as ODCLK
                            MHal_FRC_Set_STGEN_ODCLK(pInstance, E_FRC_STGEN_ODCLK_1, bMLoad);
                        }
                    }

                    if (MHal_FRC_IsFHDToFSC(pInstance, MAIN_WINDOW))
                    {
                        if (bMLoad == TRUE)
                        {
                            MDrv_XC_MLoad_WriteCommand_NonXC(pInstance, 0x1423, 0x26, 0x1000, 0xFFFF);
                        }
                        else
                        {
                            MDrv_Write2ByteMask(REG_FSC_BK23_26, 0x1000, 0x1000);
                        }
                    }
                    else
                    {
                        if (bMLoad == TRUE)
                        {
                            MDrv_XC_MLoad_WriteCommand_NonXC(pInstance, 0x1423, 0x26, 0x0000, 0xFFFF);
                        }
                        else
                        {
                            MDrv_Write2ByteMask(REG_FSC_BK23_26, 0x0000, 0x1000);
                        }
                    }

                    bIsFhdToFsc = MHal_FRC_IsFHDToFSC(pInstance, eWindow);

                    MDrv_XC_MLoad_Fire(pInstance, TRUE);

                    if(bMLoad)
                    {
                        _MLOAD_RETURN(pInstance);
                    }

                }

                MDrv_FRC_Set_3D_QMap(pInstance, E_FRC_PNL_TYPE_2D, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_MODE_NONE); // load FRC 3D table

            }
        }
    }

#endif

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    Hal_SC_ControlMloadTrig(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

#if (HW_DESIGN_4K2K_VER == 7) //Kano Patch
    //[FIXME]If DS case, Kano need to disable REG_SC_BK00_03_L [12]
    //but MDrv_XC_3D_LoadReg will enable REG_SC_BK00_03_L [12]
    //so we disable it here temporary
    if(MDrv_XC_GetDynamicScalingStatus(pInstance))
    {
        SC_W2BYTEMSK(0, REG_SC_BK00_03_L, 0x0000, 0x1000);
    }
#endif

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= SetWindow Done (Window : %u, src: %u) =============\n",eWindow, gSrcInfo[eWindow].enInputSourceType);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    _SETWINDOW_RETURN();

    if(g_bSWDSGenScenario)
    {
        g_bSWDSGenScenario = FALSE;
    }



    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetWindow(XC_SETWIN_INFO *pstXC_SetWin_Info, MS_U32 u32InitDataLen, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        || pu32XCInst_1 == NULL
#endif
        )
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }
#if (defined (ANDROID))
    static XC_INITMISC tmp_Init_Misc;
    stXC_GET_MISC_STATUS XCArgs1;
    XCArgs1.pXC_Init_Misc = &tmp_Init_Misc;
    XCArgs1.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MISC_STATUS, (void*)&XCArgs1) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }

    if(!(tmp_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_IS_ANDROID))
    {
        tmp_Init_Misc.u32MISC_A |= E_XC_INIT_MISC_A_IS_ANDROID;

        stXC_INIT_MISC XCArgs2;
        XCArgs2.pXC_Init_Misc = &tmp_Init_Misc;
        XCArgs2.u32InitMiscDataLen = sizeof(XC_INITMISC);
        XCArgs2.eReturnValue = E_APIXC_RET_FAIL;

        if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Obtain XC engine fail\n");
            return E_APIXC_RET_FAIL;
        }
    }
#else
    static XC_INITMISC tmp_Init_Misc;
    stXC_GET_MISC_STATUS XCArgs1;
    XCArgs1.pXC_Init_Misc = &tmp_Init_Misc;
    XCArgs1.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MISC_STATUS, (void*)&XCArgs1) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }

    if(tmp_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_IS_ANDROID)
    {
        tmp_Init_Misc.u32MISC_A = tmp_Init_Misc.u32MISC_A & (~E_XC_INIT_MISC_A_IS_ANDROID);
        stXC_INIT_MISC XCArgs2;
        XCArgs2.pXC_Init_Misc = &tmp_Init_Misc;
        XCArgs2.u32InitMiscDataLen = sizeof(XC_INITMISC);
        XCArgs2.eReturnValue = E_APIXC_RET_FAIL;

#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        if(eWindow == SUB_WINDOW)
        {
            if(UtopiaIoctl(pu32XCInst_1, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
            {
                printf("Obtain XC engine fail\n");
                return E_APIXC_RET_FAIL;
            }
        }
        else
        {
            if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
            {
                printf("Obtain XC engine fail\n");
                return E_APIXC_RET_FAIL;
            }
        }
#else
        if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Obtain XC engine fail\n");
            return E_APIXC_RET_FAIL;
        }
#endif

    }
#endif
    stXC_SET_WINDOW XCArgs;
    XCArgs.pstXC_SetWin_Info = pstXC_SetWin_Info;
    XCArgs.u32InitDataLen = u32InitDataLen;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_WINDOW, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
#ifndef DISABLE_3D_FUNCTION
static MS_BOOL _MDrv_XC_3D_LoadReg(void *pInstance, E_XC_3D_INPUT_MODE eInputMode,
                           E_XC_3D_OUTPUT_MODE eOutputMode,
                           XC_InternalStatus *pSrcInfo,
                           SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if (eWindow == MAIN_WINDOW)
    {
        memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, &pXCResourcePrivate->stdrvXC_MVideo_Context.stMainDBreg, sizeof(SC_SWDB_INFO));
    }
    else
    {
        memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg, &pXCResourcePrivate->stdrvXC_MVideo_Context.stSubDBreg, sizeof(SC_SWDB_INFO));
    }
    return MDrv_XC_3D_LoadReg(pInstance, eInputMode, eOutputMode, pSrcInfo, eWindow);

}
#endif

MS_BOOL MApi_XC_SetDualWindow_U2(void* pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info_Main, XC_SETWIN_INFO *pstXC_SetWin_Info_Sub)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _SETWINDOW_ENTRY();

    if (_MApi_XC_SetWindow(pInstance, pstXC_SetWin_Info_Main, sizeof(XC_SETWIN_INFO), TRUE, MAIN_WINDOW) == FALSE)
    {
        goto SETWINDOW_EXIT;
    }

    if (_MApi_XC_SetWindow(pInstance, pstXC_SetWin_Info_Sub, sizeof(XC_SETWIN_INFO), TRUE, SUB_WINDOW) == FALSE)
    {
        goto SETWINDOW_EXIT;
    }
#ifndef DISABLE_3D_FUNCTION
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _MDrv_XC_3D_LoadReg(pInstance, MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW), MDrv_XC_Get_3D_Output_Mode(pInstance), &gSrcInfo[MAIN_WINDOW], MAIN_WINDOW);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
#endif
    bRet = TRUE;
    _SETWINDOW_RETURN();

    //all return should come here, because this code will enable opm write limit addr protect
SETWINDOW_EXIT:
    {
    #if (ENABLE_OPM_WRITE_SUPPORTED)
        MS_PHY u32OPWLimitBase = 0x00;
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        u32OPWLimitBase = ((MDrv_XC_GetIPMBase(pInstance, 0, MAIN_WINDOW)*BYTE_PER_WORD + MDrv_XC_GetDNRBufSize(pInstance, MAIN_WINDOW)) / BYTE_PER_WORD - 1) ;
        _XC_ENTRY(pInstance);
        MDrv_SC_set_opm_write_limit(pInstance, TRUE, 0 , u32OPWLimitBase , MAIN_WINDOW);
        _XC_RETURN(pInstance);
#ifdef UFO_XC_SUPPORT_DUAL_MIU
        _XC_ENTRY(pInstance);
        if (IsEnableDualMode(MAIN_WINDOW))
        {
            u32OPWLimitBase = ((MDrv_XC_GetDualIPMBase(pInstance, 0, MAIN_WINDOW)*BYTE_PER_WORD + MDrv_XC_GetDualDNRBufSize(pInstance, MAIN_WINDOW)) / BYTE_PER_WORD - 1) ;
            MDrv_SC_set_dual_opm_write_limit(pInstance, TRUE, 0 , u32OPWLimitBase , MAIN_WINDOW);
        }
        _XC_RETURN(pInstance);
#endif
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    #endif
    }

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= Set Dual Window Done =============\n");
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    if (bRet == FALSE)
    {
        _SETWINDOW_RETURN();
    }
    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetDualWindow(XC_SETWIN_INFO *pstXC_SetWin_Info_Main, XC_SETWIN_INFO *pstXC_SetWin_Info_Sub)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_SET_DUALWINDOW XCArgs;
    XCArgs.pstXC_SetWin_Info_Main = pstXC_SetWin_Info_Main;
    XCArgs.pstXC_SetWin_Info_Sub = pstXC_SetWin_Info_Sub;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_DUALWINDOW, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
#if FRC_INSIDE
void Mapi_XC_FRC_Set_User_MemoryFMT(void *pInstance, MS_U8 u8Mode, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo.bEnableUserMode = bEnable;
    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = u8Mode;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

MS_U8 Mapi_XC_FRC_Get_MemoryFMT(void *pInstance) //NO_USE
{
    MS_U8 u8Val = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    u8Val = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return u8Val;
}

void Mapi_XC_FRC_Set_MemoryFMT(void *pInstance, E_XC_3D_INPUT_MODE e3dInputMode,
                                E_XC_3D_OUTPUT_MODE e3dOutputMode,
                                E_XC_3D_PANEL_TYPE e3dPanelType)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(pXCResourcePrivate->stdrvXC_MVideo.bEnableUserMode == FALSE)
    {
        // RGB and DVI
        if(0) //(IsSrcTypeVga(enInputSourceType) ||
           //(IsSrcTypeHDMI(enInputSourceType)&(!g_HdmiPollingStatus.bIsHDMIMode)))
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "### is RGB Space\n");
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_RGB_10_SPECIAL;
        }
        else
        {
            if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.bFRC)
            {
                if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
                {
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### E_XC_3D_OUTPUT_FRAME_ALTERNATIVE\n");
                    if((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_960x1080)||
                       (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_1280x720))
                    {
                        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### is YUV Space, 960x1080\n");
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_YUV_10_SPECIAL;
                    }
                    //else if((g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_1920x1080)||
                    //   (g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_1920x540))
                    else
                    {
                        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### is YC Space, 1920x1080/1920x540\n");
                        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_YC_10_SPECIAL;
                    }
                }
                else if(e3dOutputMode == E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
                {
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF\n");
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### is YUV Space, SBS 960x1080\n");
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_YUV_10_SPECIAL;
                }
                else
                {
                    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### is YC Space, others\n");
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_YC_10_SPECIAL;
                }
            }
            else
            {
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u8IpMode = FRC_IP_MEM_IP_YC_10_SPECIAL;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"### is YC Space, bypass\n");
            }
        }
    }
    else
    {
        printf("\n\n@@Use User define for debug\n\n");
    }
#if 1
    if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
        &&(e3dInputMode == E_XC_3D_INPUT_FRAME_PACKING)
        &&(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_1280x720))
    {
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16PanelWidth = 1280;
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16PanelHeigh= 720;
    }
    else
    {
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16PanelWidth = 1920;
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_240HZ)
        {
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16PanelHeigh = 540;
        }
        else
        {
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16PanelHeigh = 1080;
        }
    }
#endif
    MDrv_FRC_SetMemFormat(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo);

    if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.bFRC)
    {
        if((pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u83D_FI_out == E_XC_3D_OUTPUT_FI_960x1080)&&
           (e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE))
        {
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u16FB_YcountLinePitch /= 2;
        }
        MDrv_FRC_OPM_SetBaseOfset(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo, e3dOutputMode);
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

//-------------------------------------------------------------------------------------------------
/// set 3D convert format type
/// @param  e3dInputMode                \b IN: the input format
/// @param  e3dOutputMode               \b IN: the format we want to convert to
/// @param  e3dPanelType                \b IN: the panel type.
//-------------------------------------------------------------------------------------------------
void MApi_XC_FRC_Set_3D_Mode(void *pInstance, E_XC_3D_INPUT_MODE e3dInputMode,
                                       E_XC_3D_OUTPUT_MODE e3dOutputMode,
                                       E_XC_3D_PANEL_TYPE e3dPanelType)
{


    MS_U8 u8FRC_Pnl_type = E_FRC_PNL_TYPE_PASSIVE;
    MS_U8 u83D_FI_out = E_XC_3D_OUTPUT_FI_MODE_NONE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if (e3dPanelType == E_XC_3D_PANEL_SHUTTER)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_240HZ)
        {
            u8FRC_Pnl_type = E_FRC_PANEL_3D_TYPE_ACTIVE_240_1920_540;
        }
        else
        {
            u8FRC_Pnl_type = E_FRC_PNL_TYPE_ACTIVE;
        }
    }
    else if(e3dPanelType == E_XC_3D_PANEL_PELLICLE)
    {
        u8FRC_Pnl_type = E_FRC_PNL_TYPE_PASSIVE;
    }
    else
    {
        u8FRC_Pnl_type = E_FRC_PNL_TYPE_2D;
    }

    u83D_FI_out = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.u83D_FI_out;

    switch(e3dInputMode)
    {
        case E_XC_3D_INPUT_MODE_NONE:
        case E_XC_3D_INPUT_NORMAL_2D_HW:
            MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, e3dInputMode, e3dOutputMode, u83D_FI_out);
        break;

        default:
        case E_XC_3D_INPUT_FRAME_PACKING:
            if(e3dOutputMode == E_XC_3D_OUTPUT_TOP_BOTTOM)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_FRAME_PACKING, E_XC_3D_OUTPUT_TOP_BOTTOM, u83D_FI_out);
            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_FRAME_PACKING, E_XC_3D_OUTPUT_FRAME_ALTERNATIVE, u83D_FI_out);
            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_FRAME_PACKING, E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF, u83D_FI_out);
            }
            else if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_L)||(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_R))
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_1920x1080);
            }
            else
            {
                printf("\n##[%s]1. Please check the 3D input and out mode\n\n", __FUNCTION__);
            }
        break;
        case E_XC_3D_INPUT_SIDE_BY_SIDE_HALF:
            if(e3dOutputMode == E_XC_3D_OUTPUT_TOP_BOTTOM)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_SIDE_BY_SIDE_HALF, E_XC_3D_OUTPUT_TOP_BOTTOM, u83D_FI_out);

            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_SIDE_BY_SIDE_HALF, E_XC_3D_OUTPUT_FRAME_ALTERNATIVE, u83D_FI_out);

            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_SIDE_BY_SIDE_HALF, E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF, u83D_FI_out);
            }
            else if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_L)||(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_R))
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_1920x1080);
            }
            else
            {
                printf("\n##[%s]2. Please check the 3D input and out mode\n\n", __FUNCTION__);
            }

        break;
        case E_XC_3D_INPUT_SIDE_BY_SIDE_FULL:
            if(e3dOutputMode == E_XC_3D_OUTPUT_TOP_BOTTOM)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_SIDE_BY_SIDE_FULL, E_XC_3D_OUTPUT_TOP_BOTTOM, u83D_FI_out);

            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_SIDE_BY_SIDE_FULL, E_XC_3D_OUTPUT_FRAME_ALTERNATIVE, u83D_FI_out);
            }
            else if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_L)||(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_R))
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_1920x1080);
            }
            else
            {
                printf("\n##[%s]2. Please check the 3D input and out mode\n\n", __FUNCTION__);
            }
        break;
        case E_XC_3D_INPUT_TOP_BOTTOM:
            if(e3dOutputMode == E_XC_3D_OUTPUT_TOP_BOTTOM)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_TOP_BOTTOM, E_XC_3D_OUTPUT_TOP_BOTTOM, u83D_FI_out);
            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_TOP_BOTTOM, E_XC_3D_OUTPUT_FRAME_ALTERNATIVE, u83D_FI_out);

            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_OUTPUT_TOP_BOTTOM, E_XC_3D_OUTPUT_SIDE_BY_SIDE_HALF, u83D_FI_out);

            }
            else if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_L)||(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_R))
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_1920x1080);
            }
            else
            {
                printf("\n##[%s]3. Please check the 3D input and out mode\n\n", __FUNCTION__);
            }
        break;
        case E_XC_3D_INPUT_FRAME_ALTERNATIVE:
            if(e3dOutputMode == E_XC_3D_OUTPUT_TOP_BOTTOM)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_FRAME_ALTERNATIVE, E_XC_3D_OUTPUT_TOP_BOTTOM, u83D_FI_out);
            }
            else if(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_ALTERNATIVE)
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_FRAME_ALTERNATIVE, E_XC_3D_OUTPUT_FRAME_ALTERNATIVE, u83D_FI_out);
            }
            else if((e3dOutputMode == E_XC_3D_OUTPUT_FRAME_L)||(e3dOutputMode == E_XC_3D_OUTPUT_FRAME_R))
            {
                MDrv_FRC_Set_3D_QMap(pInstance, u8FRC_Pnl_type, E_XC_3D_INPUT_MODE_NONE, E_XC_3D_OUTPUT_MODE_NONE, E_XC_3D_OUTPUT_FI_1920x1080);
            }
            else
            {
                printf("\n##[%s]1. Please check the 3D input and out mode\n\n", __FUNCTION__);
            }
        break;


    }

    switch(e3dOutputMode)
    {
        //for 2D to 3D use

        // ParaCnt=4, P0 = 2D to 3D ConversionMode(0=Disable, 1=Enable)
        //                  P1 = 3D Depth Gain(0~31)
        //                  P2 = 3D Depth Offset(0~127)
        //                  P3 = Artificial Gain(0~15)

        case E_XC_3D_OUTPUT_CHECKBOARD_HW:
        case E_XC_3D_OUTPUT_LINE_ALTERNATIVE_HW:
        case E_XC_3D_OUTPUT_PIXEL_ALTERNATIVE_HW:
        case E_XC_3D_OUTPUT_FRAME_L_HW:
        case E_XC_3D_OUTPUT_FRAME_R_HW:
            if(!Hal_XC_FRC_R2_Set_2DTo3DMode(1, 0x13, 0x69, 0x0E))
            {
                printf("Enable 2DTo3D Failed\n");
            }
        break;

        default:
            if(!Hal_XC_FRC_R2_Set_2DTo3DMode(0, 0x13, 0x69, 0x8))
            {
                printf("Disable 2DTo3D Failed\n");
            }
        break;
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#endif
extern MS_U16 _MDrv_SC_GetOutputVPeriod(void *pInstance);
MS_BOOL MApi_XC_FRC_BypassMFC_U2(void* pInstance, MS_BOOL bEnable)
{
#if FRC_INSIDE
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MS_U8 u8Mode = FRC_IP_MEM_IP_RGB_10_SPECIAL;
    E_XC_3D_INPUT_MODE e3dInputMode = E_XC_3D_INPUT_MODE_NONE;
    E_XC_3D_OUTPUT_MODE e3dOutputMode = E_XC_3D_OUTPUT_MODE_NONE;

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    E_XC_3D_PANEL_TYPE e3dPanelType = MDrv_XC_Get_3D_Panel_Type(pInstance);
    MS_U16 u16DelayCount = 0;

    FRC_R2_CMD_PARAMETER_t tFRC_R2_Para;

    memset(&tFRC_R2_Para, 0, sizeof(FRC_R2_CMD_PARAMETER_t));

    ///Current spec doesn't have bypass MFC in 3D mode, this is only for PC RGB bypass mode
    if((e3dInputMode != MDrv_XC_Get_3D_Input_Mode(pInstance, MAIN_WINDOW))||(e3dOutputMode != MDrv_XC_Get_3D_Output_Mode(pInstance)))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return FALSE;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    // for Agate, Freeze memory
    W2BYTEMSK(L_FRC_SC134(0x07), BIT(0), BIT(0));

    if(bEnable)
    {
        printf("@@@ MFC Bypass Enable\n");
        printf("@@@1,Set MFC CMD \n");

        tFRC_R2_Para.p1=0x0F;
        tFRC_R2_Para.p2=0;
        tFRC_R2_Para.p3=0;

        MDrv_XC_SendCmdToFRC(pInstance, 0x20, 3, tFRC_R2_Para );
        MDrv_Write2ByteMask(0x3020C0, 0x00, BIT(0));  // IPM CSC oFF
        // for Agate
        MDrv_Write2ByteMask(0x302660, 0x00, BIT(8));  // data flow don't go through the MFC section
        MDrv_Write2ByteMask(0x302C80, 0x00, BIT(0)); // MFC Stage disable
        MDrv_Write2ByteMask(0x30290E, 0x00, 0xFFFF); // MFC disable
        W2BYTEMSK(L_FRC_SC134(0x07), BIT(1), BIT(1));
        //Disable 6R mode
        W2BYTEMSK(L_FRC_SC134(0x19), 0x00, BIT(8));


        MDrv_Write2ByteMask(0x303002, 0x00, BIT(3));   // OP2 CSC oFF
        u16DelayCount = 7;
        u8Mode = FRC_IP_MEM_IP_RGB_10_SPECIAL;
    }
    else
    {
        printf("@@@ MFC Bypass Disable\n");
        printf("@@@1,Set MFC CMD \n");

        tFRC_R2_Para.p1=0x02;
        tFRC_R2_Para.p2=0;
        tFRC_R2_Para.p3=0;

        MDrv_XC_SendCmdToFRC(pInstance, 0x20, 3, tFRC_R2_Para );

        MDrv_Write2ByteMask(0x3020C0, BIT(0), BIT(0));
        // for Agate
        MDrv_Write2ByteMask(0x302660, BIT(8), BIT(8));
        MDrv_Write2ByteMask(0x302C80, BIT(0), BIT(0));
        MDrv_Write2ByteMask(0x30290E, 0x1D2F, 0xFFFF);
        W2BYTEMSK(L_FRC_SC134(0x07), 0, BIT(1));
        //Enable 6R mode
        W2BYTEMSK(L_FRC_SC134(0x19), BIT(8), BIT(8));

        MDrv_Write2ByteMask(0x303002, BIT(3), BIT(3));
        u16DelayCount = 10;
        u8Mode = FRC_IP_MEM_IP_YUV_10_SPECIAL;
    }
    printf("@@@2,Load MFC table\n");
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    Mapi_XC_FRC_Set_User_MemoryFMT(pInstance, u8Mode, bEnable);
    MDrv_XC_FRC_SetWindow(pInstance, e3dInputMode, e3dOutputMode, e3dPanelType);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // for Agate, need to wait the memory write finish
    u16DelayCount *= 20;//_MDrv_SC_GetOutputVPeriod(); //get delay time
    printf("Enable Delay=%u\n", u16DelayCount);
    MsOS_DelayTask(u16DelayCount);

    // for Agate, un Freeze memory
    W2BYTEMSK(L_FRC_SC134(0x07), 0x00, BIT(0));

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return TRUE;
#else
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return FALSE;
#endif

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_FRC_BypassMFC(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_FRC_BYPASS XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_FRC_BYPASS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
void MApi_XC_FRC_Mute_U2(void* pInstance, MS_BOOL bEnable)
{
    MDrv_FRC_Mute(pInstance, bEnable);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_Mute(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_FRC_MUTE XCArgs;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_FRC_MUTE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
void MDrv_XC_FRC_SetWindow(void *pInstance, E_XC_3D_INPUT_MODE e3dInputMode,
                           E_XC_3D_OUTPUT_MODE e3dOutputMode,
                           E_XC_3D_PANEL_TYPE e3dPanelType)
{
#if FRC_INSIDE
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Pnl_Misc.FRCInfo.bFRC &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultHTotal> 3000) &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultVTotal> 2000) &&
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16DefaultVFreq > 500) &&  // 4K2K panel
        (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_FRC_INSIDE_4K1K_120HZ) &&
        (E_XC_3D_OUTPUT_TOP_BOTTOM == MDrv_XC_Get_3D_Output_Mode(pInstance)))
    {
        // For FRC inside chip, ex.Einstein/Napoli
        // 4K1K FA output case, FRC only can handle TB in to FA out
        // FRC input is TB, output is FA
        e3dInputMode = E_XC_3D_INPUT_TOP_BOTTOM;
        e3dOutputMode = E_XC_3D_OUTPUT_FRAME_ALTERNATIVE;
    }

    Mapi_XC_FRC_Set_MemoryFMT(pInstance, e3dInputMode, e3dOutputMode, e3dPanelType);
    MApi_XC_FRC_Set_3D_Mode(pInstance, e3dInputMode, e3dOutputMode, e3dPanelType);
    if( (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width == 3840)
        && (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height == 2160) )
    {
        printf("@@@ %s: 4k2k patch\n", __FUNCTION__);
        //FRC IPM/OPM
        MDrv_FRC_IPM_SetOffset(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
        MDrv_FRC_IPM_SetFetchNum(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
        MDrv_FRC_OPM_SetOffset(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
        MDrv_FRC_OPM_SetFetchNum(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
        //FRC OP HSU/VSU
        MDrv_FRC_HSU_SetScalingSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width,
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width);
        MDrv_FRC_VSU_SetScalingSize(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height,
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
        //force RGB 444
        MDrv_FRC_CSC_SelectPath(pInstance, E_FRC_COLOR_PATH_RGB_444);
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
#endif
}

void MApi_XC_FRC_SetWindow_U2(void* pInstance,
                             E_XC_3D_INPUT_MODE e3dInputMode,
                             E_XC_3D_OUTPUT_MODE e3dOutputMode,
                             E_XC_3D_PANEL_TYPE e3dPanelType)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_XC_FRC_SetWindow(pInstance, e3dInputMode, e3dOutputMode, e3dPanelType);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_SetWindow(E_XC_3D_INPUT_MODE e3dInputMode,
                           E_XC_3D_OUTPUT_MODE e3dOutputMode,
                           E_XC_3D_PANEL_TYPE e3dPanelType)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_FRC_WINDOW XCArgs;
    XCArgs.e3dInputMode = e3dInputMode;
    XCArgs.e3dOutputMode = e3dOutputMode;
    XCArgs.e3dPanelType = e3dPanelType;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FRC_WINDOW, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set report window
/// @param  bEnable        \b IN: enable or disable report window
/// @param  Window         \b IN: the window position and size
/// @param  u8Color        \b IN: report window's color
//-------------------------------------------------------------------------------------------------
void MApi_XC_SetRepWindow_U2(void* pInstance, MS_BOOL bEnable,MS_WINDOW_TYPE Window,MS_U8 u8Color)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    MDrv_SC_rep_window(pInstance,bEnable,Window,u8Color);
    memcpy(&pXCResourcePrivate->stdrvXC_MVideo_Context.g_RepWindow, &Window, sizeof(MS_WINDOW_TYPE));
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetRepWindow(MS_BOOL bEnable,MS_WINDOW_TYPE Window,MS_U8 u8Color)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_REPORT_WINDOW XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.Window = Window;
    XCArgs.u8Color = u8Color;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_REPORT_WINDOW, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// update display window registers with input window
/// @param  eWindow                          \b IN: Main or sub window
/// @param  pdspwin                          \b IN: window info that will be setted to registers
//-------------------------------------------------------------------------------------------------
void    MApi_XC_SetDispWinToReg_U2(void* pInstance, MS_WINDOW_TYPE *pstDspwin, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    MDrv_XC_set_dispwin_to_reg(pInstance, eWindow, pstDspwin);
    memcpy(&gSrcInfo[eWindow].stDispWin, pstDspwin, sizeof(MS_WINDOW_TYPE));
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
    if ((pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled) && (psXCInstPri->u32DeviceID != 1))
    {
        MDrv_FRC_UpdateMDE(pInstance, gSrcInfo[eWindow].stDispWin);
    }
#endif
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void    MApi_XC_SetDispWinToReg(MS_WINDOW_TYPE *pstDspwin, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_DISPLAY_WIN_TO_REG XCArgs;
    XCArgs.pstDspwin = pstDspwin;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_DISPLAY_WIN_TO_REG, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// get current display window registers setting
/// @param  eWindow                          \b IN : Main or sub window
/// @param  pdspwin                          \b OUT: Pointer for ouput disp window register
//-------------------------------------------------------------------------------------------------
void    MApi_XC_GetDispWinFromReg_U2(void* pInstance, MS_WINDOW_TYPE *pstDspwin, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);
    MDrv_XC_get_dispwin_from_reg(pInstance, eWindow, pstDspwin);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void    MApi_XC_GetDispWinFromReg(MS_WINDOW_TYPE *pstDspwin, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }
    static MS_WINDOW_TYPE tmp;
    memcpy(&tmp,pstDspwin,sizeof(MS_WINDOW_TYPE));
    stXC_GET_DISPLAY_WIN_FROM_REG XCArgs;
    XCArgs.pstDspwin = &tmp;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_DISPLAY_WIN_FROM_REG, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        memcpy(pstDspwin,&tmp,sizeof(MS_WINDOW_TYPE));
        return;
    }
}
#endif
/// MUX API
///////////////////////////////////////////////////

/**
 * InputSource Mux is a controller which handles all of the video path.
 * Before using video source, user have to create a data path between source and sink.
 * For example, DTV -> MainWindow or DTV -> SubWindow.
 */

/////////////////////////////////////////
// Configuration
/////////////////////////////////////////


/********************************************************************************/
/*                 Static Variable and Functions (Private)                      */
/********************************************************************************/

/********************************************************************************/
/*                 Functions                                                    */
/********************************************************************************/

//////////////////////////////////////
//    Data Query Functions
//////////////////////////////////////
void MApi_XC_Mux_GetPortMappingMatrix_U2(void *pInstance,XC_MUX_INPUTSRCTABLE* mapping_tab,MS_U32 length)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    memcpy(pXCResourcePrivate->stdrvXC_MVideo.mapping_tab,mapping_tab,length);
    pXCResourcePrivate->stdrvXC_MVideo.matrix_length = (length / sizeof(XC_MUX_INPUTSRCTABLE));
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Mux_GetPortMappingMatrix(XC_MUX_INPUTSRCTABLE mapping_tab[50],MS_U32 length)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return ;
        }
      }
#endif

    stXC_GET_MUX_MAPPINGTAB XCArgs;
    XCArgs.mapping_tab = mapping_tab;
    XCArgs.length = length;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MUX_MAPPINGTAB, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get input port from input source type
/// @param  src                 \b IN: the query based on the input source type
/// @param  port_ids            \b OUT: the port ID we get
/// @param  u8port_count        \b OUT: how many ports we get
//-------------------------------------------------------------------------------------------------
void MDrv_XC_Mux_GetPort(void *pInstance, INPUT_SOURCE_TYPE_t src, E_MUX_INPUTPORT* port_ids , MS_U8* u8port_count )
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    * u8port_count = pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[src].u32EnablePort ;
    port_ids[0] = pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[src].u32Port[0];
    port_ids[1] = pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[src].u32Port[1];
}

//-------------------------------------------------------------------------------------------------
/// Get input port from input HDMI source type
/// @param  src                 \b IN: the query based on the input HDMI type
/// @return @ref E_MUX_INPUTPORT
//-------------------------------------------------------------------------------------------------
E_MUX_INPUTPORT MApi_XC_Mux_GetHDMIPort_U2( void* pInstance, INPUT_SOURCE_TYPE_t src )
{
    E_MUX_INPUTPORT ePort = INPUT_PORT_NONE_PORT;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    ePort = MDrv_XC_Mux_GetHDMIPort(pInstance, src);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return ePort;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_MUX_INPUTPORT MApi_XC_Mux_GetHDMIPort( INPUT_SOURCE_TYPE_t src )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return INPUT_PORT_NONE_PORT;
    }

    stXC_GET_MUX_HDMIPORT XCArgs;
    XCArgs.src = src;
    XCArgs.eReturnValue = INPUT_PORT_NONE_PORT;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MUX_HDMIPORT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return INPUT_PORT_NONE_PORT;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set Support MHL Path Info
/// @param u8MhlSupportInfo               \b IN:  MHL Support Info
///@return @ref MS_BOOL True if query success.
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Mux_SetSupportMhlPathInfo_U2(void* pInstance, MS_U8 u8MhlSupportInfo)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo_Context._u8MhlSupportInfo = u8MhlSupportInfo;
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Mux_SetSupportMhlPathInfo(MS_U8 u8MhlSupportInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_MUX_SUPPORT_MHL_PATHINFO XCArgs;
    XCArgs.u8MhlSupportInfo = u8MhlSupportInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_SUPPORT_MHL_PATHINFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set MHL Hot Plug Inverse Info
/// @param bIsMhlHotPlugInverse               \b IN:  MHL Hot Plug Inverse Info
///@return @ref MS_BOOL True if query success.
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Mux_SetMhlHotPlugInverseInfo_U2(void* pInstance, MS_BOOL bIsMhlHotPlugInverse)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo_Context.g_bIsMhlHotPlugInverse = bIsMhlHotPlugInverse;
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Mux_SetMhlHotPlugInverseInfo(MS_BOOL bIsMhlHotPlugInverse)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_MUX_MHL_HOTPLUG_INVERSE_INFO XCArgs;
    XCArgs.bIsMhlHotPlugInverse = bIsMhlHotPlugInverse;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_MHL_HOTPLUG_INVERSE_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//////////////////////////////////////
//    Set path & power
//////////////////////////////////////

static void _MApi_XC_Mux_SetPath(void *pInstance, INPUT_SOURCE_TYPE_t enInputSourceType, E_DEST_TYPE enOutputType)
{
    MS_U8 u8Port_count = 0;
    E_MUX_INPUTPORT enPorts[3] = {INPUT_PORT_NONE_PORT, INPUT_PORT_NONE_PORT, INPUT_PORT_NONE_PORT};
    MS_U8 MS_U8index = 0 ;
    MS_U8 dest = (MS_U8)enOutputType;
    // Get source ports.

    MDrv_XC_Mux_GetPort(pInstance, enInputSourceType , enPorts , &u8Port_count );
    // Initial : Turn off Y/C mux
    // This is only for one VD
    if ( IsSrcTypeDigitalVD(enInputSourceType) && enOutputType != OUTPUT_CVBS1 && enOutputType != OUTPUT_CVBS2 )
    {
        MDrv_XC_mux_turnoff_ymux(pInstance);
        MDrv_XC_mux_turnoff_cmux(pInstance);
    }

    // For backward compatibility ( Remove if app layer ready )
    // Because we can not change the API, we add a check here for old behavior.
    if ( IsSrcTypeVga(enInputSourceType) && u8Port_count == 1)
    {
        MDrv_XC_mux_set_sync_port_by_dataport(pInstance, enPorts[0]);
    }

    for (  ;  MS_U8index < u8Port_count ; MS_U8index++ )
    {
        //printf(" Port index: %d  Port type: %d \n", MS_U8index ,enPorts[MS_U8index]);
        MDrv_XC_mux_dispatch(pInstance, enPorts[MS_U8index],(E_MUX_OUTPUTPORT)dest);
    }

    //printf(" Output port: %d \n", enOutputType);
    // for AV source with the G channel of RGB
    if(!IsSrcTypeVga(enInputSourceType) && !IsSrcTypeYPbPr(enInputSourceType))
    {// VGA and YPbPr have SOG control flow in PCmonitor
        if(IsSrcTypeAV(enInputSourceType))
        {
            if((INPUT_PORT_YMUX_G0 == enPorts[0])||(INPUT_PORT_YMUX_G1 == enPorts[0]) || (INPUT_PORT_YMUX_G2 == enPorts[0]))
            {
                // AV source with the G channel of RGB

                // Signal be received from G channel to SOG HW
                // no Signal in SOG: SOG discharge (E_ADC_ISOG_STANDBY_MODE)
                // signal in SOG   : SOG charge    (E_ADC_ISOG_NORMAL_MODE)
                MDrv_ADC_ISOG_SetDetectMode(pInstance, E_ADC_ISOG_NORMAL_MODE);
            }
            else
            {
                MDrv_ADC_ISOG_SetDetectMode(pInstance, E_ADC_ISOG_STANDBY_MODE);
            }
        }
        else
        {
            MDrv_ADC_ISOG_SetDetectMode(pInstance, E_ADC_ISOG_STANDBY_MODE);
        }
    }

}

//////////////////////////////////////
//    Path Managerment Functions
//////////////////////////////////////

/*!
 *  Check path is valid or not
 *  @param src The input source of a path
 *  @param dest The destination of a path
 *  @return BOOL true if path is valid.
 */
MS_BOOL _MApi_XC_Mux_CheckValidPath(void *pInstance, INPUT_SOURCE_TYPE_t src , E_DEST_TYPE dest)
{
    // After T3, there is no spec about SV -> CVBS out. (HW design)
    if ( IsSrcTypeSV(src) && ( dest == OUTPUT_CVBS1 || dest == OUTPUT_CVBS2 ) )
    {
        return Hal_XC_SVOutput_GetCaps(pInstance);
    }
    return TRUE;
}

/*!
 *  Search a patch and return the path index if the desire path founded
 *  @param src The input source of a path
 *  @param dest The destination of a path
 *  @return Path Id stored in Mux Controller
 */
static MS_S16 _MApi_XC_Mux_SearchPath(void *pInstance, INPUT_SOURCE_TYPE_t src, E_DEST_TYPE dest)
{
    MS_S16 i;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    for ( i = 0 ; i < MAX_DATA_PATH_SUPPORTED; i++)
    {
        if ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src == src &&
             pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest == dest &&
             (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & DATA_PATH_USING)
           )
        {
            return i;
        }
    }
    return -1; // No maching path
}

static MS_S16 _MApi_XC_Mux_GetAvailablePath(void *pInstance, E_PATH_TYPE e_type)
{
    MS_S16 index = 0, boundary = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if ( e_type == PATH_TYPE_ASYNCHRONOUS )
    {
        index = MAX_SYNC_DATA_PATH_SUPPORTED ;
        boundary = MAX_DATA_PATH_SUPPORTED ;
    }
    else
    {
        boundary = MAX_SYNC_DATA_PATH_SUPPORTED ;
    }

    for (; index < boundary; index++)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[index].u8PathStatus & DATA_PATH_USING)
        {
            continue;
        }
        else
        {
            return index;
        }
    }

    return -1; // No maching path

}

// Return sources connected with Main window or Sub window
static MS_U8 _MApi_XC_Mux_GetUsingSources(void *pInstance, INPUT_SOURCE_TYPE_t* src_array )
{
    MS_U8 i = 0, j = 0 , available_source_count = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    for ( ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        // Retrieve sources which is using right now.
        if (  ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ( DATA_PATH_USING ) ) != ( DATA_PATH_USING ) )
            continue;

        // Check if source on path i already in src_array
        for ( j = 0 ; j < available_source_count ; j++ )
        {
            if ( src_array[j] == pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src )
                break;
        }

        if ( j >= available_source_count )
        {
            // no same source in src_array
            // only return source which connected with main or sub window
            if ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest != OUTPUT_CVBS1 &&
                 pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest != OUTPUT_CVBS2 &&
                 pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest != OUTPUT_SCALER_DWIN)
            {
                src_array[available_source_count] = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src ;
                available_source_count++;
            }
        }
    }
    return available_source_count;
}
// Public Functions

//-------------------------------------------------------------------------------------------------
/// Initialize Mux before using Mux controller
/// @param  input_source_to_input_port      \b IN: a function to map from input source to input port
//-------------------------------------------------------------------------------------------------
void MApi_XC_Mux_Init_U2(void* pInstance, void (*input_source_to_input_port)(INPUT_SOURCE_TYPE_t src_ids , E_MUX_INPUTPORT* port_ids , MS_U8* u8port_count ) )
{
    MS_U16 i;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount = 0;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.input_source_to_input_port = input_source_to_input_port;
    for ( i = 0 ; i < MAX_DATA_PATH_SUPPORTED; i++)
    {
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus = 0;
    }

    for(i=0; i<INPUT_SOURCE_NUM; i++)
    {
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[i] = 0;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Mux_Init(void (*input_source_to_input_port)(INPUT_SOURCE_TYPE_t src_ids , E_MUX_INPUTPORT* port_ids , MS_U8* u8port_count ) )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MUX_INIT XCArgs;
    XCArgs.input_source_to_input_port = input_source_to_input_port;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_INIT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Monitor the source of Mux
/// @param  bRealTimeMonitorOnly      \b IN: define whether do the monitor routine only for real time
//-------------------------------------------------------------------------------------------------
void MApi_XC_Mux_SourceMonitor_U2(void* pInstance, MS_BOOL bRealTimeMonitorOnly)
{
    MS_S16 i;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    for ( i = 0 ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        if (  ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
              == ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
        {
            // execute path_thread()
            if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].path_thread != NULL)
            {
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].path_thread(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src, bRealTimeMonitorOnly);
                _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            }

            // execute destination periodic handler
            if(((pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ENABLE_PERIODIC_HANDLER) == ENABLE_PERIODIC_HANDLER) &&
               (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_periodic_handler != NULL))
            {
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_periodic_handler(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src, bRealTimeMonitorOnly );
                _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            }
        }
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    }

    // This is patch for imme-switch. It can be fix in the feature.
    Hal_DVI_IMMESWITCH_PS_SW_Path();

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Mux_SourceMonitor(MS_BOOL bRealTimeMonitorOnly)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MUX_SOURCE_MONITOR XCArgs;
    XCArgs.bRealTimeMonitorOnly = bRealTimeMonitorOnly;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_SOURCE_MONITOR, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// A path is disabled after creating it (\ref MApi_XC_Mux_CreatePath)
/// If a path is not enabled,   \ref MApi_XC_Mux_SourceMonitor will bypass it.
/// @param PathId               \b IN: PathId The Path you want to enable
/// @return 1 if enable successfuly. Return -1 if enable fail
//-------------------------------------------------------------------------------------------------
MS_S16 MApi_XC_Mux_EnablePath_U2(void* pInstance, MS_U16 PathId)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    if ( PathId >= MAX_DATA_PATH_SUPPORTED )
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1;
    }
    else
    {
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].u8PathStatus |= DATA_PATH_ENABLE;
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return 1;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_S16 MApi_XC_Mux_EnablePath(MS_U16 PathId)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return -1;
    }

    stXC_SET_MUX_ENABLE_PATH XCArgs;
    XCArgs.PathId = PathId;
    XCArgs.s16ReturnValue = -1;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_ENABLE_PATH, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return -1;
    }
    else
    {
        return XCArgs.s16ReturnValue;
    }
}
#endif

#if (ENABLE_NONSTD_INPUT_MCNR==1)

extern MS_U8 MDrv_AVD_GetRegValue(MS_U16 u16Addr);
#define BK_AFEC_CC    (0x35CC)

static void _MApi_XC_Sys_Detect_UnStd_Input_Status_Init(void)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal = FALSE;
    pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt = 0;
}

static void _MApi_XC_CVBSUnStdISR(SC_INT_SRC eIntNum, void * pParam)
{
    UNUSED(eIntNum); UNUSED(pParam);
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if((MDrv_AVD_GetRegValue(BK_AFEC_CC) & 0x18) == 0x18)        // non-standard input
    {
        pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt = 0;

        if (pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal == FALSE)
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_27_L, 0x01, 0x01);
            pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal = TRUE;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "_MApi_XC_CVBSUnStdISR: unStdSignal\n");
        }
    }
    else
    {
        if(pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal == TRUE)
        {
            pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt++;

            if(pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt > 10)
            {
                SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK12_27_L, 0x00, 0x01);
                pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt = 0;
                pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal = FALSE;
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "_MApi_XC_CVBSUnStdISR: StdSignal\n");
            }
        }
    }
}

#endif

static void _Mdrv_XC_Mux_DeletePath(void* pInstance, MS_S16 PathId)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].u8PathStatus = 0;

    if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount > 0)
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount--;
}

static void _Mdrv_XC_Mux_DecreaseSrcCreateTime(void* pInstance, INPUT_SOURCE_TYPE_t src, MS_BOOL bIsIncrease)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if (bIsIncrease)
    {
        // Increate src create time
        if(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[src] < 0xFFFF)
        {
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[src]++;
        }
    }
    else
    {
        // Decrease src create time
        if(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[src] > 0)
        {
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[src]--;
        }
    }
}


//-------------------------------------------------------------------------------------------------
/// delete a path from Mux Controller.
/// @param src              \b IN: Type of input source of a path you want to delete
/// @param dest             \b IN: Type of destination of a path you want to delete
/// @return 1 if delete successfuly. Return -1 if delete fail
//-------------------------------------------------------------------------------------------------
MS_S16 MApi_XC_Mux_DeletePath_U2(void* pInstance, INPUT_SOURCE_TYPE_t src, E_DEST_TYPE dest)
{
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if (dest == OUTPUT_SCALER_SUB_WINDOW)
    {
        pInstance = g_pDevice1Instance;
        dest = OUTPUT_SCALER2_MAIN_WINDOW;
    }
#endif
    // Maximum number of sources is the number of pathes
    INPUT_SOURCE_TYPE_t _InputSource[MAX_SYNC_DATA_PATH_SUPPORTED] ;
    MS_U8 _source_count = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);

    MS_S16 PathId = _MApi_XC_Mux_SearchPath(pInstance, src,dest);

    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"=========== MApi_XC_Mux_DeletePath(ID 0x%x) is called =========== \n", PathId);
    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Source : %d   Destination : %d \n",src,dest);

    if ( PathId != -1 )
    {
        MS_U8 u8Index = 0;

        // Initial _InputSource
        for (; u8Index < MAX_SYNC_DATA_PATH_SUPPORTED; u8Index++)
        {
            _InputSource[u8Index] = INPUT_SOURCE_NONE;
        }

        // Clear status.
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        _Mdrv_XC_Mux_DeletePath(g_pDevice0Instance, PathId);
        _Mdrv_XC_Mux_DeletePath(g_pDevice1Instance, PathId);
#else
        _Mdrv_XC_Mux_DeletePath(pInstance, PathId);
#endif
        // Update ADC setting for multi-source.
        if ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount > 0 )
        {
            _source_count = _MApi_XC_Mux_GetUsingSources(pInstance, _InputSource);

            XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Total using source current %d \n",_source_count);

            MDrv_XC_ADC_SetInputSource(pInstance, _InputSource, _source_count);
        }

        if ( IsSrcTypeHDMI(src) || IsSrcTypeDVI(src)  )
        {
            // Power down all HDMI power
            // because HDMI cannot PIP with HDMI, so we can power down all the HDMIs
            Hal_SC_mux_set_dvi_mux(pInstance, 0xFF, HAL_OUTPUTPORT_NONE_PORT);
        }

#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA == TRUE)
        if (
#if (HW_DESIGN_4K2K_VER == 7)
            (psXCInstPri->u32DeviceID == 0) &&
            (dest == OUTPUT_SCALER_MAIN_WINDOW) &&
#endif
#ifndef UFO_XC_SUPPORT_SUB_CMA
            (dest != OUTPUT_SCALER2_MAIN_WINDOW) &&
#endif
            (pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable == FALSE))
        {
            MHal_XC_Release_CMA(pInstance, CMA_XC_SELF_MEM, MAIN_WINDOW);
            MHal_XC_Release_CMA(pInstance, CMA_XC_COBUFF_MEM, MAIN_WINDOW);
        }
#endif
#endif
        //Disable SUB_Window CLK
        if(dest == OUTPUT_SCALER_SUB_WINDOW)
        {
            MApi_XC_EnableCLK_for_SUB(pInstance, DISABLE);
            MDrv_XC_Enable_Extra_Request(pInstance, DISABLE);
        }
        else if(dest == OUTPUT_SCALER_DWIN)
        {
            MApi_XC_EnableCLK_for_DIP(pInstance, DISABLE);
        }

        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"=========== Path deleted success =========== \n");

        // Decrease src create time
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(g_pDevice0Instance, src, FALSE);
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(g_pDevice0Instance, src, FALSE);
#else
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(pInstance, src, FALSE);
#endif
        if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[src] == 0)
        {
            if ( IsSrcTypeYPbPr(src) || IsSrcTypeVga(src)  )
            {
                // ADC have to set as free run
                MDrv_XC_ADC_Set_Freerun(pInstance, TRUE);
            }
        }

#if (ENABLE_NONSTD_INPUT_MCNR == 1)
        MS_U8 i;

        for(i=0; i<_source_count; i++)
        {
            if(IsSrcTypeATV(_InputSource[i]))
            {
                break;
            }
        }

        if(i==_source_count)
        {
            // there is no more ATV input, de-attach ISR
            MDrv_XC_InterruptDeAttach(pInstance, XC_INT_VSINT, _MApi_XC_CVBSUnStdISR, &pXCResourcePrivate->stdrvXC_MVideo.u8UnStdOutput[MAIN_WINDOW]);
        }
#endif

        // We clear Input source variable here. To reset scaler IP again if same source connected again.
        if (dest == OUTPUT_SCALER_MAIN_WINDOW)
        {
            pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[MAIN_WINDOW] = INPUT_SOURCE_NONE;
        }
        else if (dest == OUTPUT_SCALER_SUB_WINDOW)
        {
            pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[SUB_WINDOW] = INPUT_SOURCE_NONE;
        }

        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return TRUE;
    }
    else
    {
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"This path is not exist\n");
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"=========== Path deleted fail =========== \n");
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_S16 MApi_XC_Mux_DeletePath(INPUT_SOURCE_TYPE_t src, E_DEST_TYPE dest)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return -1;
    }

    stXC_SET_MUX_DELETE_PATH XCArgs;
    XCArgs.src = src;
    XCArgs.dest = dest;
    XCArgs.s16ReturnValue = -1;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_DELETE_PATH, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return -1;
    }
    else
    {
        return XCArgs.s16ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Trigger Sync Event on Mux Controller.
/// The Mux Controller will pass src and *para to each Sync Event Handler (refer synchronous_event_handler of MUX_DATA_PATH also).
///  ( Delegate Function:
///    Send necessary parameter to SyncEventHandler of each synchronous_path which source is 'src' )
/// @param src          \b IN: The input source which triggers Sync Event
/// @param para         \b IN: A pointer points to parameters which need to pass to Event Handler
//-------------------------------------------------------------------------------------------------
void MApi_XC_Mux_TriggerPathSyncEvent_U2( void* pInstance, INPUT_SOURCE_TYPE_t src , void* para)
{
    MS_S16 i;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    for ( i = 0 ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        if (  ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
              != ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            continue;
        }

        // send 'src' to event handler of each path
        if ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src == src &&
             pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].synchronous_event_handler != NULL)
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].synchronous_event_handler(src, para);
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        }
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Mux_TriggerPathSyncEvent( INPUT_SOURCE_TYPE_t src , void* para)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MUX_TRIGGER_PATH_SYNC_EVENT XCArgs;
    XCArgs.src = src;
    XCArgs.para = para;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_TRIGGER_PATH_SYNC_EVENT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
/*!
 *  Trigger Sync Event on Mux Controller.
 *  The Mux Controller will pass src and *para to each Sync Event Handler (refer to synchronous_event_handler
 *   of MUX_DATA_PATH also).
 *
 *  ( Delegate Function:
 *    Send necessary parameter to SyncEventHandler of each synchronous_path which source is 'src' )
 *  @param src The input source which triggers Sync Event
 *  @param para A pointer points to parameters which need to pass to Event Handler
 */
//-------------------------------------------------------------------------------------------------
/// Set the specific window
/// @param  src             \b IN: the source type for handler
/// @param para         \b IN: A pointer points to parameters which need to pass to Event Handler
//-------------------------------------------------------------------------------------------------
void MApi_XC_Mux_TriggerDestOnOffEvent_U2( void* pInstance, INPUT_SOURCE_TYPE_t src , void* para)
{
    MS_S16 i;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    for ( i = 0 ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        if (  ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
              != ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            continue;
        }

        // send 'src' to event handler of each path
        if ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src == src &&
             pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_on_off_event_handler != NULL)
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_on_off_event_handler(src, para);
            _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        }
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Mux_TriggerDestOnOffEvent( INPUT_SOURCE_TYPE_t src , void* para)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MUX_TRIGGER_DEST_ONOFF_EVENT XCArgs;
    XCArgs.src = src;
    XCArgs.para = para;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_TRIGGER_DEST_ONOFF_EVENT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// This function was used to enable/disable the destination periodic handler
/// After enabled periodic hander, Mux Controller will pass the parameters to this handler and execute it periodically
/// @param  src             \b IN: the source type for handler
/// @param  bEnable         \b IN: Enable/Disable the priodic handler.
/// @return 1 if successfuly. Return -1 if failed.
//-------------------------------------------------------------------------------------------------
MS_S16 MApi_XC_Mux_OnOffPeriodicHandler_U2( void* pInstance, INPUT_SOURCE_TYPE_t src, MS_BOOL bEnable)
{
    MS_S16 i;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    for ( i = 0 ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        if (  ( pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
              != ( DATA_PATH_ENABLE | DATA_PATH_USING ) )
        {
            continue;
        }

        // send 'src' to event handler of each path
        if(bEnable)
        {
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus |= ENABLE_PERIODIC_HANDLER;
        }
        else
        {
            pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus &= ~ENABLE_PERIODIC_HANDLER;
        }
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return i;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_S16 MApi_XC_Mux_OnOffPeriodicHandler( INPUT_SOURCE_TYPE_t src, MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return -1;
    }

    stXC_SET_MUX_ONOFF_PERIODIC_HANDLER XCArgs;
    XCArgs.src = src;
    XCArgs.bEnable = bEnable;
    XCArgs.s16ReturnValue = -1;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_ONOFF_PERIODIC_HANDLER, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return -1;
    }
    else
    {
        return XCArgs.s16ReturnValue;
    }
}
#endif
typedef enum
{
    E_OnPathAdd = 0,
    E_OnPathDel= 1,
    E_PathMax,
}E_PathStatus;


static void _Mdrv_XC_Mux_CreatePath(void* pInstance, XC_MUX_PATH_INFO* Path_Info, MS_S16 PathId)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount++;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].e_PathType = Path_Info->Path_Type;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].src = Path_Info->src;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest = Path_Info->dest;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].u8PathStatus = DATA_PATH_USING;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].path_thread = Path_Info->path_thread;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].synchronous_event_handler = Path_Info->SyncEventHandler;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest_on_off_event_handler = Path_Info->DestOnOff_Event_Handler;
    pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest_periodic_handler = Path_Info->dest_periodic_handler;

#if (HW_DESIGN_4K2K_VER == 7)
//temp make a default map matrix for Kano
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_DTV].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_DTV].u32Port[0] = INPUT_PORT_MVOP;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_DTV2].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_DTV2].u32Port[0] = INPUT_PORT_MVOP2;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_STORAGE].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_STORAGE].u32Port[0] = INPUT_PORT_MVOP;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_STORAGE2].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_STORAGE2].u32Port[0] = INPUT_PORT_MVOP2;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_HDMI].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_HDMI].u32Port[0] = INPUT_PORT_DVI0;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_SCALER_OP].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_SCALER_OP].u32Port[0] = INPUT_PORT_SCALER_OP;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_SC0_DI].u32EnablePort = 1;
    pXCResourcePrivate->stdrvXC_MVideo.mapping_tab[INPUT_SOURCE_SC0_DI].u32Port[0] = INPUT_PORT_SCALER_DI;
#ifdef MSOS_TYPE_LINUX_KERNEL
    //for msAPI usage, we set dest_periodic_handler by utopia PQ's MDrv_PQ_AdaptiveTuning
    if(psXCInstPri->u32DeviceID == 0)
    {
    #ifdef CHIP_I2
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest_periodic_handler = NULL;
    #else
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest_periodic_handler = MDrv_PQ_AdaptiveTuning;
    #endif
    }
    else
    {
        pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[PathId].dest_periodic_handler = NULL;
    }
#endif
#endif

}
//-------------------------------------------------------------------------------------------------
/// Create a path in Mux Controller.
/// @param  Path_Info               \b IN: the information of the path
/// @param  u32InitDataLen          \b IN: the length of the Path_Info
/// @return @ref MS_S16 return the path id, or -1 when any error is happened
//-------------------------------------------------------------------------------------------------
MS_S16 MApi_XC_Mux_CreatePath_U2(void* pInstance, XC_MUX_PATH_INFO* Path_Info , MS_U32 u32InitDataLen)
{
#ifdef MSOS_TYPE_LINUX_KERNEL
    if(!is_compat_task())
    {
        if(u32InitDataLen != sizeof(XC_MUX_PATH_INFO) || Path_Info == NULL)
        {
            // ASSERT when driver is not initiallized
            MS_ASSERT(0);
            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
            return -1;
        }
    }
#else
    if(u32InitDataLen != sizeof(XC_MUX_PATH_INFO) || Path_Info == NULL)
    {
        // ASSERT when driver is not initiallized
        MS_ASSERT(0);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1;
    }
#endif
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if (Path_Info->dest == OUTPUT_SCALER_SUB_WINDOW)
    {
        pInstance = g_pDevice1Instance;
        Path_Info->dest = OUTPUT_SCALER2_MAIN_WINDOW;
    }
#endif
    MS_S16 PathId;
    MS_U8 u8Index=0;

    // Maximum number of sources is the number of pathes
    INPUT_SOURCE_TYPE_t _InputSource[MAX_SYNC_DATA_PATH_SUPPORTED] ;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    MS_U8 _source_count = 0;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);

    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"=========== MApi_XC_Mux_CreatePath is called =========== \n");
    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Source : %d   Destination : %d \n",Path_Info->src,Path_Info->dest);

    if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount >= MAX_DATA_PATH_SUPPORTED)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"There is no avaliable path. Current Total Path Count : %d ",pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount );
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1; // No avaliable path.
    }

    if ( _MApi_XC_Mux_SearchPath(pInstance, Path_Info->src,Path_Info->dest) != -1)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Path already exist\n");
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1; // Path exist
    }

    if ( _MApi_XC_Mux_CheckValidPath(pInstance, Path_Info->src, Path_Info->dest) == FALSE)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Path is invalid\n");
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1; // Path is invalid
    }


    if(_XC_Mutex == -1)
    {
        MS_ASSERT(0);
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"XC dirver have to be initiallized first \n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return -1;
    }

    //Add one path
    PathId= _MApi_XC_Mux_GetAvailablePath(pInstance, Path_Info->Path_Type);

    if ( PathId == -1 )
    {
        XC_LOG_TRACE(XC_DBGLEVEL_MUX,"There is no avaliable path. Current Total Path Count : %d ",pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u8PathCount );
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return -1; // no available path
    }

    // Path create.
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    _Mdrv_XC_Mux_CreatePath(g_pDevice0Instance, Path_Info, PathId);
    _Mdrv_XC_Mux_CreatePath(g_pDevice1Instance, Path_Info, PathId);
#else
    _Mdrv_XC_Mux_CreatePath(pInstance, Path_Info, PathId);
#endif
    // Set physical registers after path created.
    _MApi_XC_Mux_SetPath(pInstance, Path_Info->src,Path_Info->dest);
    if ((pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable == TRUE) ||
        (pXCResourcePrivate->sthal_Optee.op_tee_xc[SUB_WINDOW].isEnable == TRUE))
    {
        XC_OPTEE_MUX_DATA st_xc_mux_data;
        MDrv_XC_OPTEE_Mux(pInstance, E_XC_OPTEE_SET_MUX, st_xc_mux_data);
    }

    // Initial _InputSource
    for (; u8Index < MAX_SYNC_DATA_PATH_SUPPORTED; u8Index++)
    {
        _InputSource[u8Index] = INPUT_SOURCE_NONE;
    }

    // Update ADC setting for multi-source.
    _source_count = _MApi_XC_Mux_GetUsingSources(pInstance, _InputSource);

    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"Total using source current %d \n",_source_count);

    MDrv_XC_ADC_SetInputSource(pInstance, _InputSource, _source_count);

    // Check if need calibration or not
    if(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.u16SrcCreateTime[Path_Info->src] == 0)
    {
        // calibrate input source
        MDrv_XC_ADC_Source_Calibrate(pInstance, MDrv_XC_ADC_ConvertSrcToADCSrc(pInstance, Path_Info->src));

        if(IsSrcTypeYPbPr(Path_Info->src))
        {
            MDrv_XC_ADC_SetSoGCal(pInstance);
            MsOS_DelayTask(10);
        }

        if ( IsSrcTypeYPbPr(Path_Info->src) ||
             IsSrcTypeVga(Path_Info->src)  )
        {
            // ADC have to set as free run
            MDrv_XC_ADC_Set_Freerun(pInstance, TRUE);
        }

    }

    // Increate src create time
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(g_pDevice0Instance, Path_Info->src, TRUE);
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(g_pDevice0Instance, Path_Info->src, TRUE);
#else
        _Mdrv_XC_Mux_DecreaseSrcCreateTime(pInstance, Path_Info->src, TRUE);
#endif
    //Enable Sub_Window CLK
    if(Path_Info->dest == OUTPUT_SCALER_SUB_WINDOW)
    {
        // Disable Pre-Scaling for ATV pink issue
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK04_05_L, 0x00 , BIT(15) );
        MApi_XC_EnableCLK_for_SUB(pInstance, ENABLE);
    }
    else if(Path_Info->dest == OUTPUT_SCALER_MAIN_WINDOW)
    {
        // Disable Pre-Scaling for ATV pink issue
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_05_L, 0x00 , BIT(15) );
    }
    else if(Path_Info->dest == OUTPUT_SCALER_DWIN)
    {
        // Enable DIP window clock
        MApi_XC_EnableCLK_for_DIP(pInstance, ENABLE);
    }


    _XC_RETURN(pInstance);

    XC_LOG_TRACE(XC_DBGLEVEL_MUX,"=========== Path created success =========== \n");

#if (ENABLE_NONSTD_INPUT_MCNR == 1)
    if(IsSrcTypeATV(Path_Info->src) &&
      (!MApi_XC_InterruptIsAttached(XC_INT_VSINT, _MApi_XC_CVBSUnStdISR, &pXCResourcePrivate->stdrvXC_MVideo.u8UnStdOutput[MAIN_WINDOW])))
    {
        // Re-init state machine
        _MApi_XC_Sys_Detect_UnStd_Input_Status_Init();

        // Attach ISR
        MDrv_XC_InterruptAttach(pInstance, XC_INT_VSINT, _MApi_XC_CVBSUnStdISR, &pXCResourcePrivate->stdrvXC_MVideo.u8UnStdOutput[MAIN_WINDOW]);
    }
#endif
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return PathId;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_S16 MApi_XC_Mux_CreatePath(XC_MUX_PATH_INFO* Path_Info , MS_U32 u32InitDataLen)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return -1;
    }

    stXC_SET_MUX_CREATE_PATH XCArgs;
    XCArgs.Path_Info = Path_Info;
    XCArgs.u32InitDataLen = u32InitDataLen;
    XCArgs.s16ReturnValue = -1;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MUX_CREATE_PATH, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return -1;
    }
    else
    {
        return XCArgs.s16ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Map input source to VD Ymux port
/// @param  u8InputSourceType      \b IN: input source type
/// @return @ref MS_U8
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_MUX_MapInputSourceToVDYMuxPORT_U2( void* pInstance, INPUT_SOURCE_TYPE_t u8InputSourceType )
{
    MS_U8 u8Return = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    u8Return = MDrv_XC_MUX_MapInputSourceToVDYMuxPORT(pInstance, u8InputSourceType );
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return u8Return;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U8 MApi_XC_MUX_MapInputSourceToVDYMuxPORT( INPUT_SOURCE_TYPE_t u8InputSourceType )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_MUX_INPUTSOURCE2VDYMUXPORT XCArgs;
    XCArgs.u8InputSourceType = u8InputSourceType;
    XCArgs.u8ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MUX_INPUTSOURCE2VDYMUXPORT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u8ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get Paths in driver.
/// @param  Paths      \b OUT: Path information
/// @return @ref MS_U8
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_Mux_GetPathInfo_U2(void* pInstance, XC_MUX_PATH_INFO* Paths)
{
    MS_S16 i;
    MS_U8 count;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    for ( i = 0, count = 0 ; i < MAX_SYNC_DATA_PATH_SUPPORTED ; i++)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & DATA_PATH_USING)
        {
            Paths[count].src = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src;
            Paths[count].dest = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest;
            Paths[count].path_thread = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].path_thread;
            Paths[count].SyncEventHandler =  pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].synchronous_event_handler;
            Paths[count].DestOnOff_Event_Handler =  pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_on_off_event_handler;
            Paths[count].dest_periodic_handler =  pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_periodic_handler;
            count++;
            //printf(" Path Id: [%d]  Source: %d   Destination: %d \n",i, s_InputSource_Mux_attr.Mux_DataPaths[i].src,
            //s_InputSource_Mux_attr.Mux_DataPaths[i].dest);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if (Paths[count].dest == OUTPUT_SCALER2_MAIN_WINDOW)
            {
                Paths[count].dest = OUTPUT_SCALER_SUB_WINDOW;
            }
#endif
        }
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return count;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U8 MApi_XC_Mux_GetPathInfo(XC_MUX_PATH_INFO* Paths)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_MUX_PATHINFO XCArgs;
    XCArgs.Paths = Paths;
    XCArgs.u8ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MUX_PATHINFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u8ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get XC library version
/// @param  ppVersion                  \b OUT: store the version in this member
/// @return @ref E_APIXC_ReturnValue
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_GetLibVer_U2(void* pInstance, const MSIF_Version **ppVersion)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    if (!ppVersion)
    {
        return E_APIXC_RET_FAIL;
    }

    *ppVersion = &_api_xc_version;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}

E_APIXC_ReturnValue MApi_XC_GetLibVer(const MSIF_Version **ppVersion)
{
    //This function may be called before creating pu32XCInst
    if (!ppVersion)
    {
        return E_APIXC_RET_FAIL;
    }

    *ppVersion = &_api_xc_version;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;

    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_LIB_VER XCArgs;
    XCArgs.ppVersion = ppVersion;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_LIB_VERSION, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// Get XC Information
/// @return @ref XC_ApiInfo returnthe XC information in this member
//-------------------------------------------------------------------------------------------------
XC_ApiInfo * MApi_XC_GetInfo_U2(void* pInstance)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    MDrv_XC_GetInfo(pInstance, &pXCResourcePrivate->stdrvXC_MVideo._stXC_ApiInfo);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return &pXCResourcePrivate->stdrvXC_MVideo._stXC_ApiInfo;
}

const XC_ApiInfo * MApi_XC_GetInfo(void)
{
    stXC_CMD_GET_INFO XCArgs;
    static XC_ApiInfo Getinfo;

    XCArgs.stReturnValue = &Getinfo;

    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return XCArgs.stReturnValue;
    }

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return XCArgs.stReturnValue; // return meaningless value because get instance failed.
    }
    else
    {
        return XCArgs.stReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// Extended interface for Getting XC Status of specific window(Main/Sub)
/// @param  pDrvStatusEx                  \b OUT: store the status
/// @param  eWindow                     \b IN: which window(Main/Sub) is going to get status
/// @return @ref MS_U16 return the copied length of input structure
//-------------------------------------------------------------------------------------------------
MS_U16 MApi_XC_GetStatusEx_U2(void* pInstance, XC_ApiStatusEx *pDrvStatusEx, SCALER_WIN eWindow)
{
    XC_ApiStatusEx DrvStatusEx;
    MS_U16 u16CopiedLength = sizeof(XC_ApiStatusEx);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    if((NULL == pDrvStatusEx) || (pDrvStatusEx->u16ApiStatusEX_Length == 0))
    {
        //We consider compatible operation from version2 , so reject the info init when version invalid
        XC_LOG_TRACE(XC_DBGLEVEL_SETTIMING, "MApi_XC_GetStatusEx: Null paramter or Wrong u16ApiStatusEX_Length!!\n")

        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return 0;
    }

    // the version control is coming in with version 1
    if(pDrvStatusEx->u32ApiStatusEx_Version < 1)
    {
        //We consider compatible operation from version1 , so reject the info init when version invalid
        XC_LOG_TRACE(XC_DBGLEVEL_SETTIMING, "MApi_XC_GetStatusEx: please check your u32ApiStatusEx_Version, it should not set to 0!!\n")
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return 0;
    }
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    memset(&DrvStatusEx, 0, sizeof(DrvStatusEx));
    VERSION_COMPATIBLE_CHECK(pDrvStatusEx, DrvStatusEx, u16CopiedLength, u32ApiStatusEx_Version, u16ApiStatusEX_Length, API_STATUS_EX_VERSION);

    DrvStatusEx.u32ApiStatusEx_Version = API_STATUS_EX_VERSION; //Return the actual version of the returned value for APP to use
    DrvStatusEx.u16ApiStatusEX_Length = u16CopiedLength; //Return the actual length of the returned value for APP to know whick info is valid

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    //----------------------
    // Customer setting
    //----------------------
    DrvStatusEx.enInputSourceType = gSrcInfo[eWindow].enInputSourceType;

    //----------------------
    // Window
    //----------------------
    memcpy(&DrvStatusEx.stCapWin, &gSrcInfo[eWindow].stCapWin, sizeof(MS_WINDOW_TYPE));
    memcpy(&DrvStatusEx.stCropWin, &gSrcInfo[eWindow].stCropWin, sizeof(MS_WINDOW_TYPE));
    /* real crop win in memory */
    memcpy(&DrvStatusEx.ScaledCropWin, &gSrcInfo[eWindow].ScaledCropWin, sizeof(MS_WINDOW_TYPE));
    //Assign display window for AP layer
    memcpy(&DrvStatusEx.stDispWin, &gSrcInfo[eWindow].stDispWin, sizeof(MS_WINDOW_TYPE));

    //stXCPanelDefaultTiming.u16HStart only be set to MAIN_WINDOW
    //Sub window do not set this value, use MAIN_WINDOW if set sub.
    if(DrvStatusEx.stDispWin.x >= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16HStart)
    {
        DrvStatusEx.stDispWin.x -= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16HStart;
    }
    if(DrvStatusEx.stDispWin.y >= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16VStart)
    {
        DrvStatusEx.stDispWin.y -= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16VStart;
    }

    if(MDrv_XC_Is2K2KToFrc(pInstance))
    {
        DrvStatusEx.stDispWin.x = DrvStatusEx.stDispWin.x * 2;
        DrvStatusEx.stDispWin.width= DrvStatusEx.stDispWin.width * 2;
    }

    //----------------------
    // Timing
    //----------------------
    DrvStatusEx.bInterlace = gSrcInfo[eWindow].bInterlace;
    DrvStatusEx.bHDuplicate = gSrcInfo[eWindow].bHDuplicate;
    DrvStatusEx.u16InputVFreq = gSrcInfo[eWindow].u16InputVFreq;
    DrvStatusEx.u16InputVTotal = gSrcInfo[eWindow].u16InputVTotal;
    DrvStatusEx.u16DefaultHtotal = gSrcInfo[eWindow].u16DefaultHtotal;
    DrvStatusEx.u8DefaultPhase = (MS_U8)gSrcInfo[eWindow].u16DefaultPhase;
    DrvStatusEx.u16DefaultPhase = gSrcInfo[eWindow].u16DefaultPhase;

    //----------------------
    // customized scaling
    //----------------------
    DrvStatusEx.bHCusScaling = gSrcInfo[eWindow].bHCusScaling;
    DrvStatusEx.u16HCusScalingSrc = gSrcInfo[eWindow].u16HCusScalingSrc;
    DrvStatusEx.u16HCusScalingDst = gSrcInfo[eWindow].u16HCusScalingDst;
    DrvStatusEx.bVCusScaling = gSrcInfo[eWindow].bVCusScaling;
    DrvStatusEx.u16VCusScalingSrc = gSrcInfo[eWindow].u16VCusScalingSrc;
    DrvStatusEx.u16VCusScalingDst = gSrcInfo[eWindow].u16VCusScalingDst;

    DrvStatusEx.bPreHCusScaling = gSrcInfo[eWindow].Status2.bPreHCusScaling;
    DrvStatusEx.u16PreHCusScalingSrc = gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc;
    DrvStatusEx.u16PreHCusScalingDst = gSrcInfo[eWindow].Status2.u16PreHCusScalingDst;
    DrvStatusEx.bPreVCusScaling = gSrcInfo[eWindow].Status2.bPreVCusScaling;
    DrvStatusEx.u16PreVCusScalingSrc = gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc;
    DrvStatusEx.u16PreVCusScalingDst = gSrcInfo[eWindow].Status2.u16PreVCusScalingDst;

    //--------------
    // 9 lattice
    //--------------
    DrvStatusEx.bDisplayNineLattice = gSrcInfo[eWindow].bDisplayNineLattice;

    //----------------------
    // XC internal setting
    //----------------------

    /* scaling ratio */
    DrvStatusEx.u16H_SizeAfterPreScaling = gSrcInfo[eWindow].u16H_SizeAfterPreScaling;
    DrvStatusEx.u16V_SizeAfterPreScaling = gSrcInfo[eWindow].u16V_SizeAfterPreScaling;
    DrvStatusEx.bPreV_ScalingDown = gSrcInfo[eWindow].bPreV_ScalingDown;


    /* others */
    DrvStatusEx.u32Op2DclkSet = gSrcInfo[eWindow].u32Op2DclkSet;

    /* Video screen status */
    DrvStatusEx.bBlackscreenEnabled = gSrcInfo[eWindow].bBlackscreenEnabled;
    DrvStatusEx.bBluescreenEnabled = gSrcInfo[eWindow].bBluescreenEnabled;
    DrvStatusEx.u16VideoDark = gSrcInfo[eWindow].u16VideoDark;

    DrvStatusEx.u16V_Length = gSrcInfo[eWindow].u16V_Length;
    DrvStatusEx.u16BytePerWord = gSrcInfo[eWindow].u16BytePerWord;
    DrvStatusEx.u16OffsetPixelAlignment = gSrcInfo[eWindow].u16OffsetPixelAlignment;
    DrvStatusEx.u8BitPerPixel = gSrcInfo[eWindow].u8BitPerPixel;

    DrvStatusEx.eDeInterlaceMode = gSrcInfo[eWindow].eDeInterlaceMode;
    DrvStatusEx.u8DelayLines = gSrcInfo[eWindow].u8DelayLines;
    DrvStatusEx.bMemFmt422 = gSrcInfo[eWindow].bMemFmt422;
    DrvStatusEx.eMemory_FMT = gSrcInfo[eWindow].eMemory_FMT;

    DrvStatusEx.bForceNRoff = gSrcInfo[eWindow].bForceNRoff;
    DrvStatusEx.bEn3DNR = gSrcInfo[eWindow].bEn3DNR;
    DrvStatusEx.bUseYUVSpace = gSrcInfo[eWindow].bUseYUVSpace;
    DrvStatusEx.bMemYUVFmt = gSrcInfo[eWindow].bMemYUVFmt;
    DrvStatusEx.bForceRGBin = gSrcInfo[eWindow].bForceRGBin;
    DrvStatusEx.bLinearMode = gSrcInfo[eWindow].bLinearMode;

    // frame lock related
    // only main can select FB/FBL because panel output timing depends on main window, sub will always use FB
    DrvStatusEx.bFBL = gSrcInfo[eWindow].bFBL;
    DrvStatusEx.bFastFrameLock = gSrcInfo[eWindow].bFastFrameLock;
    DrvStatusEx.bDoneFPLL = MDrv_Scaler_GetFPLLDoneStatus(pInstance);
    DrvStatusEx.bEnableFPLL = gSrcInfo[eWindow].bEnableFPLL;
    DrvStatusEx.bFPLL_LOCK = gSrcInfo[eWindow].bFPLL_LOCK;

    // Capture_Memory
    DrvStatusEx.u32IPMBase0 = gSrcInfo[eWindow].u32IPMBase0;
    DrvStatusEx.u32IPMBase1 = gSrcInfo[eWindow].u32IPMBase1;
    DrvStatusEx.u32IPMBase2 = gSrcInfo[eWindow].u32IPMBase2;
    DrvStatusEx.u16IPMOffset = gSrcInfo[eWindow].u16IPMOffset;
    DrvStatusEx.u16IPMFetch = gSrcInfo[eWindow].u16IPMFetch;

    //HSizeChangedBeyondPQ status
    DrvStatusEx.bPQSetHSD = !(gSrcInfo[eWindow].bHCusScaling || gSrcInfo[eWindow].Status2.bPreHCusScaling
                                || MDrv_XC_GetHSizeChangeManuallyFlag(pInstance, eWindow));

    DrvStatusEx.bIsHWDepthAdjSupported = MDrv_XC_3D_IsHWDepthAdjSupported(pInstance, MDrv_XC_Get_3D_Input_Mode(pInstance, eWindow),
                                                                          MDrv_XC_Get_3D_Output_Mode(pInstance),
                                                                          eWindow);
    DrvStatusEx.bIs2LineMode = MDrv_SC_3D_Is2TapModeSupportedFormat(pInstance, &gSrcInfo[eWindow], eWindow);

    DrvStatusEx.bIsPNLYUVOutput = MDrv_XC_IsPNLYUVOutput(pInstance);

    //if inputSource is HDMI, get the HDMI pixel repetition info.
    if(IsSrcTypeHDMI(DrvStatusEx.enInputSourceType))
    {
        DrvStatusEx.u8HDMIPixelRepetition = MDrv_HDMI_avi_infoframe_info(_BYTE_5) & 0x0F;
    }
    else
    {
        DrvStatusEx.u8HDMIPixelRepetition = 0;
    }

    // FRCM memory info
    DrvStatusEx.u32FRCMBase0 = MDrv_XC_Get_FRCM_W_BaseAddr(pInstance,0,eWindow);
    DrvStatusEx.u32FRCMBase1 = MDrv_XC_Get_FRCM_W_BaseAddr(pInstance,1,eWindow);
    DrvStatusEx.u32FRCMBase2 = MDrv_XC_Get_FRCM_W_BaseAddr(pInstance,2,eWindow);

    #ifdef UFO_XC_ZORDER
    //VIDEO and the relative position of OSD.
    if(E_APIXC_RET_OK != MDrv_SC_GetVideoOnOSD(pInstance, &(DrvStatusEx.enZOrderIndex), eWindow) )
    {
        printf("%s, %d, MDrv_SC_GetVideoOnOSD fail!\n", __FUNCTION__, __LINE__);
    }
    #endif

    // FSC && FRC settings
    #if FRC_INSIDE
        #if (HW_DESIGN_4K2K_VER == 6) // For Manhattan, FSC+FRC
            DrvStatusEx.bFSCEnabled = pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled;
            DrvStatusEx.bFRCEnabled = pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled;
        #else
            DrvStatusEx.bFSCEnabled = FALSE;
            DrvStatusEx.bFRCEnabled = FALSE;
        #endif
    #else
        DrvStatusEx.bFSCEnabled = FALSE;
        DrvStatusEx.bFRCEnabled = FALSE;
    #endif
    DrvStatusEx.u16PanelInterfaceType= (MS_U16)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type;
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    //shorter Ap structure with longer lib structure, we should handle it to avoid access-violation.
    memcpy(pDrvStatusEx, &DrvStatusEx, u16CopiedLength);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return u16CopiedLength;
}

MS_U16 MApi_XC_GetStatusEx(XC_ApiStatusEx *pDrvStatusEx, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_STATUS_EX XCArgs;
    XCArgs.pDrvStatusEx = pDrvStatusEx;
    XCArgs.eWindow = eWindow;
    XCArgs.u16ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_STATUS_EX, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u16ReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// Get XC Status of specific window(Main/Sub)
/// @param  pDrvStatus                  \b OUT: store the status
/// @param  eWindow                     \b IN: which window(Main/Sub) is going to get status
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_XC_GetStatus(void *pInstance, XC_ApiStatus *pDrvStatus, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    //----------------------
    // Customer setting
    //----------------------
    pDrvStatus->enInputSourceType = gSrcInfo[eWindow].enInputSourceType;

    //----------------------
    // Window
    //----------------------
    memcpy(&pDrvStatus->stCapWin, &gSrcInfo[eWindow].stCapWin, sizeof(MS_WINDOW_TYPE));
    memcpy(&pDrvStatus->stCropWin, &gSrcInfo[eWindow].stCropWin, sizeof(MS_WINDOW_TYPE));
    /* real crop win in memory */
    memcpy(&pDrvStatus->ScaledCropWin, &gSrcInfo[eWindow].ScaledCropWin, sizeof(MS_WINDOW_TYPE));
    //Assign display window for AP layer
    memcpy(&pDrvStatus->stDispWin, &gSrcInfo[eWindow].stDispWin, sizeof(MS_WINDOW_TYPE));

    //stXCPanelDefaultTiming.u16HStart only be set to MAIN_WINDOW
    //Sub window do not set this value, use MAIN_WINDOW if set sub.
    if(pDrvStatus->stDispWin.x >= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16HStart)
    {
        pDrvStatus->stDispWin.x -= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16HStart;
    }
    if(pDrvStatus->stDispWin.y >= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16VStart)
    {
        pDrvStatus->stDispWin.y -= gSrcInfo[MAIN_WINDOW].Status2.stXCPanelDefaultTiming.u16VStart;
    }

    //----------------------
    // Timing
    //----------------------
    pDrvStatus->bInterlace = gSrcInfo[eWindow].bInterlace;
    pDrvStatus->bHDuplicate = gSrcInfo[eWindow].bHDuplicate;
    pDrvStatus->u16InputVFreq = gSrcInfo[eWindow].u16InputVFreq;
    pDrvStatus->u16InputVTotal = gSrcInfo[eWindow].u16InputVTotal;
    pDrvStatus->u16DefaultHtotal = gSrcInfo[eWindow].u16DefaultHtotal;
    pDrvStatus->u8DefaultPhase = (MS_U8)gSrcInfo[eWindow].u16DefaultPhase;

    //----------------------
    // customized scaling
    //----------------------
    pDrvStatus->bHCusScaling = gSrcInfo[eWindow].bHCusScaling;
    pDrvStatus->u16HCusScalingSrc = gSrcInfo[eWindow].u16HCusScalingSrc;
    pDrvStatus->u16HCusScalingDst = gSrcInfo[eWindow].u16HCusScalingDst;
    pDrvStatus->bVCusScaling = gSrcInfo[eWindow].bVCusScaling;
    pDrvStatus->u16VCusScalingSrc = gSrcInfo[eWindow].u16VCusScalingSrc;
    pDrvStatus->u16VCusScalingDst = gSrcInfo[eWindow].u16VCusScalingDst;

    //--------------
    // 9 lattice
    //--------------
    pDrvStatus->bDisplayNineLattice = gSrcInfo[eWindow].bDisplayNineLattice;

    //----------------------
    // XC internal setting
    //----------------------

    /* scaling ratio */
    pDrvStatus->u16H_SizeAfterPreScaling = gSrcInfo[eWindow].u16H_SizeAfterPreScaling;
    pDrvStatus->u16V_SizeAfterPreScaling = gSrcInfo[eWindow].u16V_SizeAfterPreScaling;
    pDrvStatus->bPreV_ScalingDown = gSrcInfo[eWindow].bPreV_ScalingDown;


    /* others */
    pDrvStatus->u32Op2DclkSet = gSrcInfo[eWindow].u32Op2DclkSet;

    /* Video screen status */
    pDrvStatus->bBlackscreenEnabled = gSrcInfo[eWindow].bBlackscreenEnabled;
    pDrvStatus->bBluescreenEnabled = gSrcInfo[eWindow].bBluescreenEnabled;
    pDrvStatus->u16VideoDark = gSrcInfo[eWindow].u16VideoDark;

    pDrvStatus->u16V_Length = gSrcInfo[eWindow].u16V_Length;
    pDrvStatus->u16BytePerWord = gSrcInfo[eWindow].u16BytePerWord;
    pDrvStatus->u16OffsetPixelAlignment = gSrcInfo[eWindow].u16OffsetPixelAlignment;
    pDrvStatus->u8BitPerPixel = gSrcInfo[eWindow].u8BitPerPixel;

    pDrvStatus->eDeInterlaceMode = gSrcInfo[eWindow].eDeInterlaceMode;
    pDrvStatus->u8DelayLines = gSrcInfo[eWindow].u8DelayLines;
    pDrvStatus->bMemFmt422 = gSrcInfo[eWindow].bMemFmt422;
    pDrvStatus->eMemory_FMT = gSrcInfo[eWindow].eMemory_FMT;

    pDrvStatus->bForceNRoff = gSrcInfo[eWindow].bForceNRoff;
    pDrvStatus->bEn3DNR = gSrcInfo[eWindow].bEn3DNR;
    pDrvStatus->bUseYUVSpace = gSrcInfo[eWindow].bUseYUVSpace;
    pDrvStatus->bMemYUVFmt = gSrcInfo[eWindow].bMemYUVFmt;
    pDrvStatus->bForceRGBin = gSrcInfo[eWindow].bForceRGBin;
    pDrvStatus->bLinearMode = gSrcInfo[eWindow].bLinearMode;

    // frame lock related
    // only main can select FB/FBL because panel output timing depends on main window, sub will always use FB
    pDrvStatus->bFBL = gSrcInfo[eWindow].bFBL;
    pDrvStatus->bFastFrameLock = gSrcInfo[eWindow].bFastFrameLock;
    pDrvStatus->bDoneFPLL = MDrv_Scaler_GetFPLLDoneStatus(pInstance);
    pDrvStatus->bEnableFPLL = gSrcInfo[eWindow].bEnableFPLL;
    pDrvStatus->bFPLL_LOCK = gSrcInfo[eWindow].bFPLL_LOCK;

    // Capture_Memory
    pDrvStatus->u32IPMBase0 = gSrcInfo[eWindow].u32IPMBase0;
    pDrvStatus->u32IPMBase1 = gSrcInfo[eWindow].u32IPMBase1;
    pDrvStatus->u32IPMBase2 = gSrcInfo[eWindow].u32IPMBase2;
    pDrvStatus->u16IPMOffset = gSrcInfo[eWindow].u16IPMOffset;
    pDrvStatus->u16IPMFetch = gSrcInfo[eWindow].u16IPMFetch;

#ifdef STELLAR
    pDrvStatus->u16OutputVFreqAfterFRC =gSrcInfo[eWindow].Status2.u16OutputVFreqAfterFRC;
#endif

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return TRUE;
}

MS_BOOL MApi_XC_GetStatus_U2(void* pInstance, XC_ApiStatus *pDrvStatus, SCALER_WIN eWindow)
{
    MS_BOOL bReturn = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    bReturn = MDrv_XC_GetStatus(pInstance, pDrvStatus, eWindow);
    if(MDrv_XC_Is2K2KToFrc(pInstance))
    {
        pDrvStatus->stDispWin.x = pDrvStatus->stDispWin.x * 2;
        pDrvStatus->stDispWin.width= pDrvStatus->stDispWin.width * 2;
    }
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bReturn;
}

MS_BOOL MApi_XC_GetStatus(XC_ApiStatus *pDrvStatus, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_GET_STATUS XCArgs;
    XCArgs.pDrvStatus = pDrvStatus;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_STATUS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}

MS_BOOL MApi_XC_GetStatusNodelay_U2(void* pInstance, ST_XC_APISTATUSNODELAY *pDrvStatus, SCALER_WIN eWindow)
{
    MS_U16 u16CopiedLength = sizeof(ST_XC_APISTATUSNODELAY);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
#if 0
    if((NULL == pDrvStatus) || (pDrvStatus->u16ApiStatusEX_Length == 0))
    {
        //We consider compatible operation from version2 , so reject the info init when version invalid
        XC_LOG_TRACE(XC_DBGLEVEL_SETTIMING, "MApi_XC_GetStatusEx: Null paramter or Wrong u16ApiStatusEX_Length!!\n")

        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return 0;
    }

    // the version control is coming in with version 1
    if(pDrvStatus->u32ApiStatusEx_Version < 1)
    {
        //We consider compatible operation from version1 , so reject the info init when version invalid
        XC_LOG_TRACE(XC_DBGLEVEL_SETTIMING, "MApi_XC_GetStatusEx: please check your u32ApiStatusEx_Version, it should not set to 0!!\n")
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return 0;
    }
#endif
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    //----------------------
    // Customer setting
    //----------------------
    pDrvStatus->enInputSourceType = gSrcInfo[eWindow].stStatusnodelay.enInputSourceType;

    //----------------------
    // Window
    //----------------------
    //Assign display window for AP layer
    memcpy(&pDrvStatus->stDispWin, &gSrcInfo[eWindow].stStatusnodelay.stDispWin, sizeof(MS_WINDOW_TYPE));

    if(pDrvStatus->stDispWin.x >= gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16HStart)
    {
        pDrvStatus->stDispWin.x -= gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16HStart;
    }
    if(pDrvStatus->stDispWin.y >= gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16VStart)
    {
        pDrvStatus->stDispWin.y -= gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16VStart;
    }


    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return u16CopiedLength;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_GetStatusNodelay(ST_XC_APISTATUSNODELAY *pDrvStatus, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    ST_XC_GET_STATUS_NODELAY XCArgs;
    XCArgs.pDrvStatus = pDrvStatus;
    XCArgs.eWindow = eWindow;
    XCArgs.u16ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_STATUS_NODELAY, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.u16ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set XC debug level
/// @param  u16DbgSwitch                \b IN: turn on/off debug switch
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_SetDbgLevel_U2(void* pInstance, MS_U16 u16DbgSwitch)
{
    _u16XCDbgSwitch_InternalUseOnly = u16DbgSwitch;

    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetDbgLevel(MS_U16 u16DbgSwitch)
{
    XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
    INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_SET_DBG_LEVEL XCArgs;
    XCArgs.u16DbgSwitch = u16DbgSwitch;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_DBG_LEVEL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get scaler output vertical frequency
/// @return @ref MS_U16 return output vertical frequency x 100
//-------------------------------------------------------------------------------------------------
MS_U16 MApi_XC_GetOutputVFreqX100_U2(void* pInstance)
{
    MS_U16 u16OutputVfreqX100;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    u16OutputVfreqX100 = MDrv_SC_GetOutputVFreqX100(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u16OutputVfreqX100;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U16 MApi_XC_GetOutputVFreqX100(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_OUTPUT_VFREQX100 XCArgs;
    XCArgs.u16ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_OUTPUT_VFREQX100, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u16ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get OP1 output vertical frequency
/// @return @ref MS_U16 return output vertical frequency x 100
//-------------------------------------------------------------------------------------------------
MS_U16 MApi_XC_GetOP1OutputVFreqX100_U2(void* pInstance)
{
    MS_U16 u16OutputVfreqX100;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    u16OutputVfreqX100 = MDrv_SC_GetOP1OutputVFreqX100(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u16OutputVfreqX100;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U16 MApi_XC_GetOP1OutputVFreqX100(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_OP1_OUTPUT_VFREQX100 XCArgs;
    XCArgs.u16ReturnValue = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_OP1_OUTPUT_VFREQX100, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u16ReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set NR on/off
/// @param bEn              \b IN: enable/disable NR
/// @param eWindow          \b IN: which window(Main/Sub) is going to get status
//-------------------------------------------------------------------------------------------------
void MApi_XC_Set_NR_U2(void* pInstance, MS_BOOL bEn, SCALER_WIN eWindow)
{
#if TEST_NEW_DYNAMIC_NR
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    MDrv_Scaler_EnableNR(pInstance, bEn, eWindow);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Set_NR(MS_BOOL bEn, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_NR XCArgs;
    XCArgs.bEn = bEn;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_NR, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
extern __attribute__((weak)) void MDrv_PQ_DesideSrcType(PQ_WIN eWindow, PQ_INPUT_SOURCE_TYPE enInputSourceType);
extern __attribute__((weak)) void MDrv_PQ_DisableFilmMode(PQ_WIN eWindow, MS_BOOL bOn);
extern __attribute__((weak)) MS_BOOL MDrv_PQ_LoadScalingTable(PQ_WIN eWindow,
                                                                MS_U8 eXRuleType,
                                                                MS_BOOL bPreV_ScalingDown,
                                                                MS_BOOL bInterlace,
                                                                MS_BOOL bColorSpaceYUV,
                                                                MS_U16 u16InputSize,
                                                                MS_U16 u16SizeAfterScaling);
extern __attribute__((weak)) void MDrv_PQ_Set420upsampling(PQ_WIN eWindow,
                                                            MS_BOOL bFBL,
                                                            MS_BOOL bPreV_ScalingDown,
                                                            MS_U16 u16V_CropStart);
extern __attribute__((weak)) MS_BOOL MDrv_PQ_SetCSC(PQ_WIN eWindow, PQ_FOURCE_COLOR_FMT enForceColor);
extern __attribute__((weak)) MS_BOOL MDrv_PQ_Get_MemYUVFmt(PQ_WIN eWindow, PQ_FOURCE_COLOR_FMT enForceColor);
extern __attribute__((weak)) PQ_DEINTERLACE_MODE MDrv_PQ_SetMemFormat(PQ_WIN eWindow, MS_BOOL bMemFmt422, MS_BOOL bFBL, MS_U8 *pu8BitsPerPixel);
extern __attribute__((weak)) void MDrv_PQ_Set_ModeInfo(PQ_WIN eWindow, PQ_INPUT_SOURCE_TYPE enInputSourceType, MS_PQ_Mode_Info *pstPQModeInfo);
extern __attribute__((weak)) MS_BOOL MDrv_PQ_IOCTL(PQ_WIN eWindow, MS_U32 u32Flag, void *pBuf, MS_U32 u32BufSize);
#endif
//-------------------------------------------------------------------------------------------------
/// Create a function for PQ in SC.
/// @param  PQ_Function_Info               \b IN: the information of the function
/// @param  u32InitDataLen          \b IN: the length of the PQ_Function_Info
//-------------------------------------------------------------------------------------------------
void MApi_XC_PQ_LoadFunction_U2(void* pInstance, PQ_Function_Info* function_Info , MS_U32 u32InitDataLen)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

#if (HW_DESIGN_4K2K_VER == 7)
//Kano sc1 do not load pq function now
    if(psXCInstPri->u32DeviceID == 1)
    {
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_disable_filmmode = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_load_scalingtable = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt= NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo = NULL;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl = NULL;
        return;
    }
#endif

        // Path create.
#ifdef MSOS_TYPE_LINUX_KERNEL
        UNUSED(function_Info);
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype = MDrv_PQ_DesideSrcType;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_disable_filmmode = MDrv_PQ_DisableFilmMode;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_load_scalingtable = MDrv_PQ_LoadScalingTable;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling = MDrv_PQ_Set420upsampling;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc = MDrv_PQ_SetCSC;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt= MDrv_PQ_Get_MemYUVFmt;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat = MDrv_PQ_SetMemFormat;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo = MDrv_PQ_Set_ModeInfo;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl = MDrv_PQ_IOCTL;
#else
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype = function_Info->pq_deside_srctype;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_disable_filmmode = function_Info->pq_disable_filmmode;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_load_scalingtable = function_Info->pq_load_scalingtable;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling = function_Info->pq_set_420upsampling;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc = function_Info->pq_set_csc;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt= function_Info->pq_get_memyuvfmt;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat = function_Info->pq_set_memformat;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo = function_Info->pq_set_modeInfo;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl = function_Info->pq_ioctl;
#endif

#ifdef ENABLE_TV_SC2_PQ
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_deside_srctype_ex = function_Info->pq_deside_srctype_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_disable_filmmode_ex = function_Info->pq_disable_filmmode_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_load_scalingtable_ex = function_Info->pq_load_scalingtable_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_420upsampling_ex = function_Info->pq_set_420upsampling_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_csc_ex = function_Info->pq_set_csc_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_get_memyuvfmt_ex = function_Info->pq_get_memyuvfmt_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_memformat_ex = function_Info->pq_set_memformat_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_set_modeInfo_ex = function_Info->pq_set_modeInfo_ex;
        s_PQ_Function_Info[psXCInstPri->u32DeviceID].pq_ioctl_ex = function_Info->pq_ioctl_ex;
#endif

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}

void MApi_XC_PQ_LoadFunction(PQ_Function_Info* function_Info , MS_U32 u32InitDataLen)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return;
        }
    }

    stXC_SET_PQ_LOADFUNCTION XCArgs;
    XCArgs.function_Info = function_Info;
    XCArgs.u32InitDataLen = u32InitDataLen;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_PQ_LOADFUNCTION, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}

void MApi_XC_SetOSD2VEMode_U2(void* pInstance, EN_VOP_SEL_OSD_XC2VE_MUX  eVOPSelOSD_MUX)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_XC_SetOSD2VEmode(pInstance, eVOPSelOSD_MUX);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetOSD2VEMode(EN_VOP_SEL_OSD_XC2VE_MUX  eVOPSelOSD_MUX)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_OSD2VEMODE XCArgs;
    XCArgs.eVOPSelOSD_MUX = eVOPSelOSD_MUX;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OSD2VEMODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
void MApi_XC_IP2_PreFilter_Enable_U2(void* pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);
    MDrv_SC_IP2_PreFilter_Enable(pInstance, bEnable);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_IP2_PreFilter_Enable(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_IP2_PREFILTER_ENABLE XCArgs;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_IP2_PREFILTER_ENABLE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
void MApi_XC_KeepPixelPointerAppear_U2(void* pInstance, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    Hal_SC_enable_cursor_report(pInstance, bEnable);
    pXCResourcePrivate->stdrvXC_MVideo.s_bKeepPixelPointerAppear = bEnable;
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_KeepPixelPointerAppear(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_KEEP_PIXELPOINTER_APPEAR XCArgs;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_KEEP_PIXELPOINTER_APPEAR, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
E_APIXC_GET_PixelRGB_ReturnValue MApi_XC_Get_Pixel_RGB_U2(void* pInstance, XC_Get_Pixel_RGB *pData, SCALER_WIN eWindow)
{
    E_APIXC_GET_PixelRGB_ReturnValue enRet;

    MS_WINDOW_TYPE DispWin;
    MS_WINDOW_TYPE DEWin;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    Hal_SC_get_disp_de_window(pInstance, &DEWin);

    if(pData->enStage == E_XC_GET_PIXEL_STAGE_AFTER_DLC)
        MDrv_XC_get_dispwin_from_reg(pInstance, eWindow,&DispWin );
    else
        MDrv_XC_get_dispwin_from_reg(pInstance, MAIN_WINDOW, &DispWin);

    pData->u16x += DEWin.x;
    pData->u16y += DEWin.y;

    if(pData->u16x > DEWin.x + DEWin.width ||
       pData->u16y > DEWin.y + DEWin.height)
    {
        enRet = E_GET_PIXEL_RET_OUT_OF_RANGE;
    }
    else
    {
        if(Hal_SC_get_pixel_rgb(pInstance, pData))
            enRet = E_GET_PIXEL_RET_OK;
        else
            enRet = E_GET_PIXEL_RET_FAIL;
    }

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return enRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_GET_PixelRGB_ReturnValue MApi_XC_Get_Pixel_RGB(XC_Get_Pixel_RGB *pData, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_GET_PIXEL_RET_OUT_OF_RANGE;
    }

    stXC_GET_PIXELRGB XCArgs;
    XCArgs.pData = pData;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_GET_PIXEL_RET_OUT_OF_RANGE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_PIXELRGB, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_GET_PIXEL_RET_OUT_OF_RANGE;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
MS_BOOL MApi_XC_ReportPixelInfo_U2(void* pInstance, MS_XC_REPORT_PIXELINFO *pstRepPixInfo)
{
    MS_BOOL enRet = FALSE;
    MS_WINDOW_TYPE stDEWin;
    MS_WINDOW_TYPE stRepWin;
    memset(&stDEWin, 0, sizeof(MS_WINDOW_TYPE));
    memset(&stRepWin, 0, sizeof(MS_WINDOW_TYPE));
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if(NULL == pstRepPixInfo)
    {
        printf("MApi_XC_ReportPixelInfo: pstRepPixInfo is NULL!!\n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return FALSE;
    }

    _XC_ENTRY(pInstance);

    //backup
    stRepWin.x = pstRepPixInfo->u16XStart;
    stRepWin.width = pstRepPixInfo->u16XEnd - pstRepPixInfo->u16XStart + 1;
    stRepWin.y = pstRepPixInfo->u16YStart;
    stRepWin.height = pstRepPixInfo->u16YEnd - pstRepPixInfo->u16YStart + 1;

    Hal_SC_get_disp_de_window(pInstance, &stDEWin);

    if((pstRepPixInfo->u16XStart > pstRepPixInfo->u16XEnd)
        || (pstRepPixInfo->u16XStart >= stDEWin.width)
        || (pstRepPixInfo->u16XEnd >= stDEWin.width)
        || (pstRepPixInfo->u16YStart > pstRepPixInfo->u16YEnd)
        || (pstRepPixInfo->u16YStart >= stDEWin.height)
        || (pstRepPixInfo->u16YEnd >= stDEWin.height)
      )
    {
        printf("Attention! MApi_XC_ReportPixelInfo: Invalid Param! Report Window=(%u,%u,%u,%u), DE Window=(%u,%u,%u,%u)\n",
                pstRepPixInfo->u16XStart, pstRepPixInfo->u16XEnd, pstRepPixInfo->u16YStart, pstRepPixInfo->u16YEnd,
                stDEWin.x, stDEWin.x + stDEWin.width - 1, stDEWin.y, stDEWin.y + stDEWin.height - 1);
        _XC_RETURN(pInstance);
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return FALSE;
    }

    //add de
    /*printf("! MApi_XC_ReportPixelInfo: Report Window=(%u,%u,%u,%u), DE Window=(%u,%u,%u,%u)\n",
                pstRepPixInfo->u16XStart, pstRepPixInfo->u16XEnd, pstRepPixInfo->u16YStart, pstRepPixInfo->u16YEnd,
                stDEWin.x, stDEWin.x + stDEWin.width - 1, stDEWin.y, stDEWin.y + stDEWin.height - 1);*/

    //if H mirror
    if(IsHMirrorMode(MAIN_WINDOW) || (MDrv_ReadRegBit(REG_MVOP_MIRROR, BIT(1)) == BIT(1)))
    {
        pstRepPixInfo->u16XStart = stDEWin.width - pstRepPixInfo->u16XEnd - 1;
        pstRepPixInfo->u16XEnd = stRepWin.width + pstRepPixInfo->u16XStart - 1;
    }

    pstRepPixInfo->u16XStart += stDEWin.x;
    pstRepPixInfo->u16XEnd += stDEWin.x;

    //if V mirror
    if(IsVMirrorMode(MAIN_WINDOW) || (MDrv_ReadRegBit(REG_MVOP_MIRROR, BIT(0)) == BIT(0)))
    {
        pstRepPixInfo->u16YStart = stDEWin.height - pstRepPixInfo->u16YEnd -1;
        pstRepPixInfo->u16YEnd = stRepWin.height + pstRepPixInfo->u16YStart - 1;
    }

    pstRepPixInfo->u16YStart += stDEWin.y;
    pstRepPixInfo->u16YEnd += stDEWin.y;

    /*printf("%s[%d]! MApi_XC_ReportPixelInfo: Report Window=(%u,%u,%u,%u), DE Window=(%u,%u,%u,%u)\n",
                __FUNCTION__,__LINE__, pstRepPixInfo->u16XStart, pstRepPixInfo->u16XEnd, pstRepPixInfo->u16YStart, pstRepPixInfo->u16YEnd,
                stDEWin.x, stDEWin.x + stDEWin.width - 1, stDEWin.y, stDEWin.y + stDEWin.height - 1);*/

    enRet = Hal_XC_ReportPixelInfo(pInstance, pstRepPixInfo);
    //restore for user
    pstRepPixInfo->u16XStart = stRepWin.x;
    pstRepPixInfo->u16XEnd = stRepWin.x + stRepWin.width - 1;
    pstRepPixInfo->u16YStart = stRepWin.y;
    pstRepPixInfo->u16YEnd = stRepWin.y + stRepWin.height -1;

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance, E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return enRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_ReportPixelInfo(MS_XC_REPORT_PIXELINFO *pstRepPixInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_GET_REPORT_PIXEL_INFO XCArgs;
    XCArgs.pstRepPixInfo = pstRepPixInfo;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_REPORT_PIXEL_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
///This is an Obsolete functions, try use MApi_XC_Set_MemFmtEx
MS_BOOL MApi_XC_Set_MemFmt_U2(void* pInstance, MS_XC_MEM_FMT eMemFmt)
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    if(TRUE == MDrv_XC_Set_MemFmtEx(pInstance, eMemFmt, MAIN_WINDOW))
    {
        bret = MDrv_XC_Set_MemFmtEx(pInstance, eMemFmt, SUB_WINDOW);
    }
    else
    {
        bret = FALSE;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Set_MemFmt(MS_XC_MEM_FMT eMemFmt)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_SET_MEMORY_FORMAT XCArgs;
    XCArgs.eMemFmt = eMemFmt;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MEMORY_FORMAT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
MS_BOOL MDrv_XC_Set_MemFmtEx(void *pInstance, MS_XC_MEM_FMT eMemFmt, SCALER_WIN eWindow)
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(eMemFmt <= E_MS_XC_MEM_FMT_422)
    {
        pXCResourcePrivate->stdrvXC_MVideo.s_eMemFmt[eWindow] = eMemFmt;
        bret = TRUE;
    }
    else
    {
        bret = FALSE;
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;
}

MS_BOOL MApi_XC_Set_MemFmtEx_U2(void* pInstance, MS_XC_MEM_FMT eMemFmt, SCALER_WIN eWindow)
{
    MS_BOOL bReturn = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bReturn = MDrv_XC_Set_MemFmtEx(pInstance, eMemFmt, eWindow);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bReturn;
}

MS_BOOL MApi_XC_Set_MemFmtEx(MS_XC_MEM_FMT eMemFmt, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }

    stXC_SET_MEMORY_FORMAT_EX XCArgs;
    XCArgs.eMemFmt = eMemFmt;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MEMORY_FORMAT_EX, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
MS_BOOL MApi_XC_IsRequestFrameBufferLessMode_U2(void* pInstance)
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    bret = MDrv_XC_IsRequestFrameBufferLessMode(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_IsRequestFrameBufferLessMode(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_CHECK_REQUEST_FRAMEBUFFERLESS_MODE XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CHECK_REQUEST_FRAMEBUFFERLESS_MODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set Flag to bypass software reset in MApi_XC_Init()
/// @param  bFlag               \b IN: TRUE: initialize XC by skipping SW reset; FALSE: initialize XC in normal case
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SkipSWReset_U2(void* pInstance, MS_BOOL bFlag)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo._bSkipSWReset = bFlag;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SkipSWReset(MS_BOOL bFlag)
{
    XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
    INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_SET_SKIP_SWRESET XCArgs;
    XCArgs.bFlag = bFlag;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_SKIP_SWRESET, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Select which DE to vop
/// @param  bFlag               \b IN: @ref E_OP2VOP_DE_SEL
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_OP2VOPDESel_U2(void* pInstance, E_OP2VOP_DE_SEL eVopDESel)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    eRet = Hal_SC_OP2VOPDESel(pInstance, eVopDESel);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_OP2VOPDESel(E_OP2VOP_DE_SEL eVopDESel)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OP2VOP_DE_SELECTION XCArgs;
    XCArgs.eVopDESel = eVopDESel;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OP2VOP_DE_SELECTION, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set Video and OSD Layer at Scaler VOP
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetOSDLayer_U2(void* pInstance, E_VOP_OSD_LAYER_SEL  eVOPOSDLayer, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);

    eRet = MDrv_XC_SetOSDLayer(pInstance, eVOPOSDLayer, eWindow);
    if(eRet == E_APIXC_RET_OK)
    {
        gSrcInfo[eWindow].u16OSDLayer = eVOPOSDLayer;
    }

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetOSDLayer(E_VOP_OSD_LAYER_SEL  eVOPOSDLayer, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OSDLAYER XCArgs;
    XCArgs.eVOPOSDLayer = eVOPOSDLayer;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OSDLAYER, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get Video and OSD Layer Enum at Scaler VOP
/// @return Enum value
//-------------------------------------------------------------------------------------------------
E_VOP_OSD_LAYER_SEL MApi_XC_GetOSDLayer_U2(void* pInstance, SCALER_WIN eWindow)
{
    E_VOP_OSD_LAYER_SEL eSelect = E_VOP_LAYER_RESERVED;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);

    eSelect = (gSrcInfo[eWindow].u16OSDLayer == E_VOP_LAYER_RESERVED) ? MDrv_XC_GetOSDLayer(pInstance, eWindow) : gSrcInfo[eWindow].u16OSDLayer;

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eSelect;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_VOP_OSD_LAYER_SEL MApi_XC_GetOSDLayer(SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_VOP_LAYER_RESERVED;
    }

    stXC_GET_OSDLAYER XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_VOP_LAYER_RESERVED;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_OSDLAYER, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_VOP_LAYER_RESERVED;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set Constant Alpha Value of Video
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetVideoAlpha_U2(void* pInstance, MS_U8 u8Val, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    eRet = MDrv_XC_SetVideoAlpha(pInstance, u8Val, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetVideoAlpha(MS_U8 u8Val, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VIDEO_ALPHA XCArgs;
    XCArgs.u8Val = u8Val;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VIDEO_ALPHA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get Constant Alpha Value of Video
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_GetVideoAlpha_U2(void* pInstance, MS_U8 *pu8Val, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    eRet = MDrv_XC_GetVideoAlpha(pInstance, pu8Val, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_GetVideoAlpha(MS_U8 *pu8Val, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_VIDEO_ALPHA XCArgs;
    XCArgs.pu8Val = pu8Val;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_VIDEO_ALPHA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Get field packing mode support status
/// @return TRUE(success) or FALSE(fail)
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_IsFieldPackingModeSupported_U2(void* pInstance)
{
    MS_BOOL bRet = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    bRet = MDrv_XC_Get_SCMI_Type();
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bRet;
}

MS_BOOL MApi_XC_IsFieldPackingModeSupported(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_CHECK_FIELDPACKINGMODE_SUPPORTED XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CHECK_FIELDPACKINGMODE_SUPPORTED, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
// Skip Wait Vsync
/// @param eWindow               \b IN: Enable
/// @param Skip wait Vsync      \b IN: Disable wait Vsync
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SkipWaitVsync_U2( void* pInstance, MS_BOOL eWindow,MS_BOOL bIsSkipWaitVsyn)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_SC_SetSkipWaitVsync(pInstance, eWindow, bIsSkipWaitVsyn);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SkipWaitVsync( MS_BOOL eWindow,MS_BOOL bIsSkipWaitVsyn)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SKIP_WAIT_VSYNC XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.bIsSkipWaitVsyn = bIsSkipWaitVsyn;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SKIP_WAIT_VSYNC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set XC CMA Heap ID
/// @param  u8CMAHeapID            \b IN: CMA Heap ID
/// @param  eWindow                 \b IN: which window we are going to set
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetCMAHeapID_U2( void* pInstance, MS_U8 u8CMAHeapID, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_SC_SetCMAHeapID(pInstance, u8CMAHeapID, eWindow);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetCMAHeapID(MS_U8 u8CMAHeapID, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_CMA_HEAP_ID XCArgs;
    XCArgs.u8CMAHeapID = u8CMAHeapID;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_CMA_HEAP_ID, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set the VGA SOG on or off
/// @param  bEnable                \b IN: bEnable =1, Turn on the VGA SOG; bEnable =0, Turn off the VGA SOG
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetVGASogEn_U2(void* pInstance, MS_BOOL bVGASogEn)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_XC_SetVGASogEn(pInstance, bVGASogEn);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}

E_APIXC_ReturnValue MApi_XC_SetVGASogEn(MS_BOOL bVGASogEn)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VGA_SOG_ENABLE XCArgs;
    XCArgs.bVGASogEn = bVGASogEn;
    XCArgs.eReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VGA_SOG_ENABLE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}

//-------------------------------------------------------------------------------------------------
/// Get pixel shift
/// @param  ps8HOffset               \b OUT: pixel shift H
/// @param  ps8VOffset               \b OUT: pixel shift V
/// @return E_APIXC_RET_OK
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_GetPixelShift_U2(void* pInstance, MS_S8 *ps8HOffset, MS_S8 *ps8VOffset)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    *ps8HOffset = pXCResourcePrivate->stdrvXC_MVideo.s8HPixelShift;
    *ps8VOffset = pXCResourcePrivate->stdrvXC_MVideo.s8VPixelShift;

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue  MApi_XC_GetPixelShift(MS_S8 *ps8HOffset, MS_S8 *ps8VOffset)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_PIXEL_SHIFT XCArgs;
    XCArgs.ps8HOffset = ps8HOffset;
    XCArgs.ps8VOffset = ps8VOffset;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_PIXEL_SHIFT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }

}
#endif
//-------------------------------------------------------------------------------------------------
/// Set pixel shift
/// @param  s8H               \b IN: pixel shift H
/// @param  s8V               \b IN: pixel shift V
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetPixelShift_U2(void* pInstance, MS_S8 s8H, MS_S8 s8V)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    // save pixshift offset for SW DS.
   if(MDrv_XC_Is_SupportSWDS(pInstance, MAIN_WINDOW) && (MDrv_XC_GetDynamicScalingStatus(pInstance) || MDrv_XC_Is_DSForceIndexEnabled(pInstance, MAIN_WINDOW)))
   {
        pXCResourcePrivate->stdrvXC_MVideo.s8HPixelShift = s8H;
        pXCResourcePrivate->stdrvXC_MVideo.s8VPixelShift = s8V;
        _XC_RETURN(pInstance);
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return E_APIXC_RET_OK;
    }
    MS_BOOL bEnablePS=TRUE;
    MS_S8 s8CheckHData=0;
    MS_S8 s8CheckVData=0;
    UNUSED(s8CheckHData);
    UNUSED(s8CheckVData);

    s8CheckHData = (MS_S8) pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange / 2;
    s8CheckVData = (MS_S8) pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange / 2;

    if(pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange==0 && pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange==0)
    {
        //close pixel shift ( if H range == 0 && V range ==0 )
        bEnablePS = FALSE;
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        MHal_XC_PixelShiftStatusChange(pInstance, EN_XC_PIXEL_SHIFT_DISABLE);
#endif
    }
    //else if( (s8H > s8CheckHData || s8H < ((-1)*s8CheckHData) )  ||  (s8V > s8CheckVData || s8V < ((-1)*s8CheckVData) )  )
    //{
    //    bEnablePS = FALSE;
    //}
    else
    {
        bEnablePS = TRUE;
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        MHal_XC_PixelShiftStatusChange(pInstance, EN_XC_PIXEL_SHIFT_ENABLE);
#endif
    }
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[pixel shift] shift H=%d (%d ~ %d)\n",s8H,((-1)*s8CheckHData),s8CheckHData);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[pixel shift] shift V=%d (%d ~ %d)\n",s8V,((-1)*s8CheckVData),s8CheckVData);

    if(bEnablePS)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[pixel shift] do pixel shift...\n");
        eRet = MDrv_XC_SetPixelShift(pInstance, s8H, s8V);
    }
    else
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"[pixel shift] not support!!!\n");
    }
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetPixelShift(MS_S8 s8H, MS_S8 s8V)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_PIXEL_SHIFT XCArgs;
    XCArgs.s8H = s8H;
    XCArgs.s8V = s8V;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_PIXEL_SHIFT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_SetPixelShiftFeatures_U2(void* pInstance,
                                                     MS_U16 u16HPixelShiftRange,
                                                     MS_U16 u16VPixelShiftRange,
                                                     PIXEL_SHIFT_FEATURE ePixelShiftFeature)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange = 2 * u16HPixelShiftRange;
    pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange = 2 * u16VPixelShiftRange;
    pXCResourcePrivate->stdrvXC_MVideo.gePixelShiftFeature = ePixelShiftFeature;

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetPixelShiftFeatures(MS_U16 u16HPixelShiftRange,
                                                  MS_U16 u16VPixelShiftRange,
                                                  PIXEL_SHIFT_FEATURE ePixelShiftFeature)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_PIXELSHIFT_FEATURES XCArgs;
    XCArgs.u16HPixelShiftRange = u16HPixelShiftRange;
    XCArgs.u16VPixelShiftRange = u16VPixelShiftRange;
    XCArgs.ePixelShiftFeature = ePixelShiftFeature;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_PIXEL_SHIFT_FEATURES, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set the specific window for traveling mode(VE capture function)
/// @param  pstXC_SetWin_Info      \b IN: the information of the window setting
/// @param  u32InitDataLen         \b IN: the length of the pstXC_SetWin_Info
/// @param  eWindow                \b IN: which window we are going to set
/// @return @ref MS_BOOL
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_SetTravelingWindow_U2(void* pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, MS_U32 u32InitDataLen, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    //The component 1080i can't frame lock when capture start is odd.
    if(pstXC_SetWin_Info->bInterlace)
    {
        // the Capture start should be even
        if(pstXC_SetWin_Info->stCapWin.y%2)
        {
            pstXC_SetWin_Info->stCapWin.y += 1;
        }
        pstXC_SetWin_Info->stCapWin.height = pstXC_SetWin_Info->stCapWin.height & (~0x01);
    }
    pstXC_SetWin_Info->stCapWin.width = pstXC_SetWin_Info->stCapWin.width & (~0x01);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    // copy user data to internal data
    // here is a patch first, will remove after seperate Internal XC status with set window information
    //u32CopyLen = sizeof(XC_SETWIN_INFO) - (sizeof(MS_BOOL)*2) - (sizeof(MS_U16)*4);
    //memcpy(&gSrcInfo[eWindow], pstXC_SetWin_Info, u32CopyLen);
    //do not use memcpy(), strongly depend on structure definition
    _Mdrv_XC_CopySetWinInfo(pInstance, pstXC_SetWin_Info, eWindow);

    gSrcInfo[eWindow].stDispWin.x += gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16HStart;
    gSrcInfo[eWindow].stDispWin.y += gSrcInfo[eWindow].Status2.stXCPanelDefaultTiming.u16VStart;

    // dump debug msg
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= SetTravelingWindow Start (Window : %u, src: %u) =============\n",eWindow, gSrcInfo[eWindow].enInputSourceType);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CapWin  x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCapWin.x, gSrcInfo[eWindow].stCapWin.y, gSrcInfo[eWindow].stCapWin.width, gSrcInfo[eWindow].stCapWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CropWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCropWin.x, gSrcInfo[eWindow].stCropWin.y, gSrcInfo[eWindow].stCropWin.width, gSrcInfo[eWindow].stCropWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"DispWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stDispWin.x,gSrcInfo[eWindow].stDispWin.y, gSrcInfo[eWindow].stDispWin.width, gSrcInfo[eWindow].stDispWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Panel   x: %4u  y: %4u  w: %4u  h: %4u \n",
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width,
                                       pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Mirror/Interlace/Hdup = %u/%u/%u\n", IsVMirrorMode(eWindow), pstXC_SetWin_Info->bInterlace, pstXC_SetWin_Info->bHDuplicate);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"H/V total = (%u, %u), VFreq = %u\n", pstXC_SetWin_Info->u16DefaultHtotal, pstXC_SetWin_Info->u16InputVTotal, pstXC_SetWin_Info->u16InputVFreq);

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // Scaler must update source before width / height checking.
    _MDrv_XC_Set_PQ_SourceData(pInstance, eWindow, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow]);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    // Capture width & height can not be 0 !!
    if ((gSrcInfo[eWindow].stCapWin.width < 0x20) || (gSrcInfo[eWindow].stCapWin.height < 0x20) ||
        (gSrcInfo[eWindow].stCapWin.x > 0x7FF)    || (gSrcInfo[eWindow].stCapWin.y > 0x7FF))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Cap Width/Height can not be smaller than 0x20, or X/Y can not bigger than 0x7FF \n");
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return FALSE;
    }
/* //Useless code
#if ENABLE_REQUEST_FBL
    if(s_PQ_Function_Info.pq_ioctl)
    {
        s_PQ_Function_Info.pq_ioctl((PQ_WIN)eWindow, E_PQ_IOCTL_RFBL_CTRL, NULL, NULL);
    }
#endif
*/
    _XC_ENTRY(pInstance);
    MDrv_sc_set_capture_window(pInstance, eWindow );//Fine tune capture window
    Hal_SC_ip_set_capture_h_start(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16H_CapStart, eWindow); //Set the tuned capture window
    Hal_SC_ip_set_capture_v_start(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16V_CapStart, eWindow);
    Hal_SC_ip_set_capture_h_size (pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16H_CapSize , eWindow);
    Hal_SC_ip_set_capture_v_size (pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u16V_CapSize , eWindow);
    _XC_RETURN(pInstance);

    if (IsSrcTypeVga(gSrcInfo[eWindow].enInputSourceType)   ||
        IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) ||
        IsSrcTypeScart(gSrcInfo[eWindow].enInputSourceType) )
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"ADC Set Mode \n");
        PQ_ADC_SAMPLING_INFO stPQADCSamplingInfo;
        memset(&stPQADCSamplingInfo,0,sizeof(PQ_ADC_SAMPLING_INFO));
        if ( IsSrcTypeVga(gSrcInfo[eWindow].enInputSourceType) || IsSrcTypeYPbPr(gSrcInfo[eWindow].enInputSourceType) )
        {
            if(gSrcInfo[eWindow].bHDuplicate)
            {
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                MDrv_XC_ADC_GetPQADCSamplingInfo(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], &stPQADCSamplingInfo, eWindow);
                _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            }
        }

        MDrv_XC_ADC_SetMode(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], &stPQADCSamplingInfo, eWindow);
    }
    else
    {
        MDrv_XC_ADC_SCART_RGB_setMode(pInstance,FALSE);
    }
    memcpy(&gSrcInfo[eWindow].stStatusnodelay.stDispWin, &gSrcInfo[eWindow].stDispWin, sizeof(MS_WINDOW_TYPE));
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    msAPI_Scaler_SetMode(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], eWindow);
/*
    if((s_PQ_Function_Info.pq_set_csc == NULL) || (s_PQ_Function_Info.pq_set_memformat == NULL))
    {
        Hal_SC_set_csc(TRUE, eWindow);
        Hal_SC_set_444To422(ENABLE, eWindow);
        gSrcInfo[eWindow].bUseYUVSpace = TRUE;
        gSrcInfo[eWindow].bMemYUVFmt = TRUE;
    }
*/
    MDrv_SC_set_2p_mode(pInstance, pstXC_SetWin_Info, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= SetTravelingWindow Done (Window : %u, src: %u) =============\n",eWindow, gSrcInfo[eWindow].enInputSourceType);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetTravelingWindow(XC_SETWIN_INFO *pstXC_SetWin_Info, MS_U32 u32InitDataLen, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

#if (defined (ANDROID))
    static XC_INITMISC tmp_Init_Misc;
    stXC_GET_MISC_STATUS XCArgs1;
    XCArgs1.pXC_Init_Misc = &tmp_Init_Misc;
    XCArgs1.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MISC_STATUS, (void*)&XCArgs1) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }

    if(!(tmp_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_IS_ANDROID))
    {
        tmp_Init_Misc.u32MISC_A |= E_XC_INIT_MISC_A_IS_ANDROID;

        stXC_INIT_MISC XCArgs2;
        XCArgs2.pXC_Init_Misc = &tmp_Init_Misc;
        XCArgs2.u32InitMiscDataLen = sizeof(XC_INITMISC);
        XCArgs2.eReturnValue = E_APIXC_RET_FAIL;

        if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Obtain XC engine fail\n");
            return E_APIXC_RET_FAIL;
        }
    }
#else
    static XC_INITMISC tmp_Init_Misc;
    stXC_GET_MISC_STATUS XCArgs1;
    XCArgs1.pXC_Init_Misc = &tmp_Init_Misc;
    XCArgs1.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MISC_STATUS, (void*)&XCArgs1) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }

    if(tmp_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_IS_ANDROID)
    {
        tmp_Init_Misc.u32MISC_A = tmp_Init_Misc.u32MISC_A & (~E_XC_INIT_MISC_A_IS_ANDROID);
        stXC_INIT_MISC XCArgs2;
        XCArgs2.pXC_Init_Misc = &tmp_Init_Misc;
        XCArgs2.u32InitMiscDataLen = sizeof(XC_INITMISC);
        XCArgs2.eReturnValue = E_APIXC_RET_FAIL;

        if(UtopiaIoctl(pu32XCInst, E_XC_CMD_INIT_MISC, (void*)&XCArgs2) != UTOPIA_STATUS_SUCCESS)
        {
            printf("Obtain XC engine fail\n");
            return E_APIXC_RET_FAIL;
        }
    }
#endif

    stXC_SET_TRAVELING_WINDOW XCArgs;
    XCArgs.pstXC_SetWin_Info = pstXC_SetWin_Info;
    XCArgs.u32InitDataLen = u32InitDataLen;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_TRAVELING_WINDOW, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Combine_MLoadEn
/// @param  bEnable               \b IN: whether to menuload
/// Do an example when you want to menuload OSDLayer :
/// MApi_XC_Combine_MLoadEn(TRUE)
/// call xc function
/// MApi_XC_Combine_MLoadEn(FALSE)
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Combine_MLoadEn_U2(void* pInstance, MS_BOOL bEn)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    MDrv_XC_Combine_MLoadEn(pInstance, bEn);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Combine_MLoadEn(MS_BOOL bEn)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_MLOAD_COMBINE stXCArgs;
    stXCArgs.bEn = bEn;
    stXCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_MLOAD_COMBINE, (void*)&stXCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return stXCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetVideoOnOSD
/// @param  E_VDO_ON_OSD_LAYER               \b IN: set video show on which osd layer
/// @param  eWindow                                     \b IN: set main or sub video data to memory
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetVideoOnOSD_U2(void* pInstance, E_VIDEO_ON_OSD_LAYER enlayer, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue bRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    bRet = MDrv_SC_SetVideoOnOSD(pInstance, enlayer, eWindow);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bRet;

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetVideoOnOSD(E_VIDEO_ON_OSD_LAYER enlayer, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VIDEO_ON_OSD XCArgs;
    XCArgs.enlayer = enlayer;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VIDEO_ON_OSD, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetOSDLayerBlending
/// @param  u8Layer               \b IN: which osd layer will blend
/// @param  bEnable               \b IN: whether to blend
/// @param  eWindow               \b IN: which window blend with
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetOSDLayerBlending_U2(void* pInstance, MS_U8 u8Layer, MS_BOOL bEnable, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue bRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    bRet = MDrv_SC_SetOSDLayerBlending(pInstance, u8Layer, bEnable, eWindow);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetOSDLayerBlending(MS_U8 u8Layer, MS_BOOL bEnable, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OSD_LAYER_BLENDING stXCArgs;
    stXCArgs.u8Layer = u8Layer;
    stXCArgs.bEnable = bEnable;
    stXCArgs.eWindow = eWindow;
    stXCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OSD_LAYER_BLEANDING, (void*)&stXCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return stXCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetOSDLayerAlpha
/// @param  u8Layer                 \b IN: which osd layer will blend
/// @param  u16Alpha                \b IN: blending alpha
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetOSDLayerAlpha_U2(void* pInstance, MS_U8 u8Layer, MS_U8 u8Alpha)
{
    E_APIXC_ReturnValue bRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    bRet = MDrv_SC_SetOSDLayerAlpha(pInstance, u8Layer, u8Alpha);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetOSDLayerAlpha(MS_U8 u8Layer, MS_U8 u8Alpha)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OSD_LAYER_ALPHA stXCArgs;
    stXCArgs.u8Layer = u8Layer;
    stXCArgs.u8Alpha = u8Alpha;
    stXCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OSD_LAYER_ALPHA, (void*)&stXCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return stXCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetOSDBlendingFormula
/// @param  enOsdIndex                              \b IN: osd index
/// @param  enType                                  \b IN: osd blending type
/// @param  eWindow                                 \b IN: blending with main or sub video
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetOSDBlendingFormula_U2(void* pInstance, E_XC_OSD_INDEX enOsdIndex, E_XC_OSD_BlENDING_TYPE enType, SCALER_WIN eWindow)
{
    E_APIXC_ReturnValue bRet = E_APIXC_RET_OK;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);
    bRet = MDrv_SC_SetOSDBlendingFormula(pInstance, enOsdIndex, enType, eWindow);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetOSDBlendingFormula(E_XC_OSD_INDEX enOsdIndex, E_XC_OSD_BlENDING_TYPE enType, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OSD_BLENDING_FORMULA XCArgs;
    XCArgs.enOsdIndex = enOsdIndex;
    XCArgs.enType = enType;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_OSD_BLENDING_FORMULA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetScaling
/// @param  bEnable                               \b IN: set enable or disable
/// @param  eScaling_type                      \b IN: set scaling type, pre-scaling, post-scaling, both-scaling
/// @param  eVector_type                       \b IN: set H vector, v vector, and HV vector
/// @param  eWindow                             \b IN: set main or sub video data to memory
//-------------------------------------------------------------------------------------------------
void MApi_XC_SetScaling_U2( void* pInstance, MS_BOOL bEnable, E_XC_SCALING_TYPE eScaling_type, E_XC_VECTOR_TYPE eVector_type, SCALER_WIN eWindow )
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    if(eScaling_type == E_XC_PRE_SCALING)
    {
        if(eWindow == MAIN_WINDOW)
        {
            switch(eVector_type)
            {
                case E_XC_H_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_05_L, bEnable<<15 , BIT(15) );
                    break;
                case E_XC_V_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_09_L, bEnable<<15 , BIT(15) );
                    break;
                default:
                case E_XC_HV_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_05_L, bEnable<<15 , BIT(15) );
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK02_09_L, bEnable<<15 , BIT(15) );
                    break;
            }
        }
        else
        {
            switch(eVector_type)
            {
                case E_XC_H_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK04_05_L, bEnable<<15 , BIT(15) );
                    break;
                case E_XC_V_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK04_09_L, bEnable<<15 , BIT(15) );
                    break;
                default:
                case E_XC_HV_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK04_05_L, bEnable<<15 , BIT(15) );
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK04_09_L, bEnable<<15 , BIT(15) );
                    break;
            }
        }
    }
    else if(eScaling_type == E_XC_POST_SCALING)
    {
        if(eWindow == MAIN_WINDOW)
        {
            switch(eVector_type)
            {
                case E_XC_H_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_08_L, bEnable<<8 , BIT(8) );
                    break;
                case E_XC_V_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_0A_L, bEnable<<8 , BIT(8) );
                    break;
                default:
                case E_XC_HV_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_08_L, bEnable<<8 , BIT(8) );
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_0A_L, bEnable<<8 , BIT(8) );
                    break;
            }
        }
        else
        {
            switch(eVector_type)
            {
                case E_XC_H_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_28_L, bEnable<<8 , BIT(8) );
                    break;
                case E_XC_V_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_2A_L, bEnable<<8 , BIT(8) );
                    break;
                default:
                case E_XC_HV_VECTOR:
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_28_L, bEnable<<8 , BIT(8) );
                    SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_2A_L, bEnable<<8 , BIT(8) );
                    break;
            }
        }
    }
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetScaling( MS_BOOL bEnable, E_XC_SCALING_TYPE eScaling_type, E_XC_VECTOR_TYPE eVector_type, SCALER_WIN eWindow )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_SCALING XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eScaling_type = eScaling_type;
    XCArgs.eVector_type = eVector_type;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_SCALING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetMCDIBufferAddress
/// @param  u32FBAddress                              \b IN: frame buffer base addr
/// @param  u32FBSize                                   \b IN: frame buffer size
/// @param  eType                                         \b IN: set frame buffer for MCDI ME1 or ME2
//-------------------------------------------------------------------------------------------------
void MApi_XC_SetMCDIBufferAddress_U2(void* pInstance, MS_PHY u32FBAddress, MS_PHY u32FBSize, E_XC_MCDI_TYPE eType)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    MDrv_SC_SetMCDIBufferAddress(pInstance, u32FBAddress, u32FBSize, eType);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_SetMCDIBufferAddress(MS_PHY u32FBAddress, MS_PHY u32FBSize, E_XC_MCDI_TYPE eType)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MCDI_BUFFERADDRESS XCArgs;
    XCArgs.u32FBAddress = u32FBAddress;
    XCArgs.u32FBSize = u32FBSize;
    XCArgs.eType = eType;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MCDI_BUFFERADDRESS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_EnableMCDI
/// @param  bEnable                                      \b IN: enable/disable MCDI
/// @param  eType                                         \b IN: control MCDI ME1 or ME2 or both
//-------------------------------------------------------------------------------------------------
void MApi_XC_EnableMCDI_U2(void* pInstance, MS_BOOL bEnable, E_XC_MCDI_TYPE eType)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    MDrv_SC_EnableMCDI(pInstance, bEnable, eType);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_EnableMCDI(MS_BOOL bEnable, E_XC_MCDI_TYPE eType)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_MCDI_ENABLE XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eType = eType;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_MCDI_ENABLE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SendCmdToFRC
/// @param  u8Cmd                                      \b IN: CMD
/// @param  count                                         \b IN: parameter Counter
/// @param  pFRC_R2_Para                           \b IN: input parameter
/// @return  TRUE if succeed, FALSE if failed
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_SendCmdToFRC_U2( void* pInstance, MS_U8 u8Cmd, MS_U8 count, FRC_R2_CMD_PARAMETER_t pFRC_R2_Para )
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    bret = MDrv_XC_SendCmdToFRC(pInstance, u8Cmd, count, pFRC_R2_Para);

#if (HW_DESIGN_3D_VER == 4)
    // for 2Dto3D case, the FRC FICLK need to be changed
    //E_MAPI_FRC_MB_CMD_SET_2D_TO_3D_MODE = 0x40,  ParaCnt=5,
    //          P0 = 2D to 3D ConversionMode(0=Disable, 1=Enable)
    //          P1 = 3D Depth Gain(0~31)
    //          P2 = 3D Depth Offset(0~127)
    //          P3 = Artificial Gain(0~15)
    //          P4 = L/R Swap (0=Normal, 1=L/R Swap)
    if(u8Cmd == 0x40)
    {
        if(pFRC_R2_Para.p1==ENABLE)
        {
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0004, 0x0004); // clk_idclk_frc
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0000, 0x0002); // Not Invert
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0000, 0x0001); // Enable clock
        }
        else
        {
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0000, 0x0004); // clk_fdclk_frc
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0000, 0x0002); // Not Invert
            W2BYTEMSK(L_CLKGEN1(0x35), 0x0000, 0x0001); // Enable clock
        }
    }
#endif
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SendCmdToFRC( MS_U8 u8Cmd, MS_U8 count, FRC_R2_CMD_PARAMETER_t pFRC_R2_Para )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_SEND_CMD2FRC XCArgs;
    XCArgs.u8Cmd = u8Cmd;
    XCArgs.count = count;
    XCArgs.pFRC_R2_Para = pFRC_R2_Para;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SEND_CMD2FRC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_GetMsgFromFRC
/// @param  u8Cmd                                      \b IN: CMD
/// @param  pu8ParaCount                            \b IN: Counter
/// @param  pu8Para                                     \b IN: input parameter
/// @return  TRUE if succeed, FALSE if failed
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_GetMsgFromFRC_U2(void* pInstance, MS_U8* pu8Cmd, MS_U8* pu8ParaCount, MS_U8* pu8Para)
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    bret = MDrv_XC_GetMsgFromFRC(pInstance, pu8Cmd, pu8ParaCount, pu8Para);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_GetMsgFromFRC(MS_U8* pu8Cmd, MS_U8* pu8ParaCount, MS_U8* pu8Para)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_GET_MSG_FROM_FRC XCArgs;
    XCArgs.pu8Cmd = pu8Cmd;
    XCArgs.pu8ParaCount = pu8ParaCount;
    XCArgs.pu8Para = pu8Para;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_MSG_FROM_FRC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set XC bandwidth saving X-mode
/// @param  bEnable                   \b IN: enable or disable
/// @param  eWindow                 \b IN: which window we are going to enable or disable
//-------------------------------------------------------------------------------------------------
void MApi_XC_Set_BWS_Mode_U2(void* pInstance, MS_BOOL bEnable, SCALER_WIN eWindow)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    Hal_SC_set_bws_mode(pInstance, bEnable, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_Set_BWS_Mode(MS_BOOL bEnable, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BWS_MODE XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BWS_MODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set xc FRC ColorPathCtrl
/// @param  epath_type               \b IN: Select the path
/// @param  bEnable                    \b IN: enable or disable
//-------------------------------------------------------------------------------------------------
void MApi_XC_FRC_ColorPathCtrl_U2( void* pInstance, MS_U16 u16Path_sel, MS_BOOL bEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    if(u16Path_sel & FRC_PATHCTRL_OP2COLORMATRIX)
    {
        MDrv_FRC_OP2_ColorMatrixEn(pInstance, bEnable);
    }
    if(u16Path_sel & FRC_PATHCTRL_OP2CSCDITHER)
    {
        MDrv_FRC_OP2_CscDitherEn(pInstance, bEnable);
    }
    if(u16Path_sel & FRC_PATHCTRL_OP2BRIGHTNESS)
    {
        MDrv_FRC_OP2_BrightnessEn(pInstance, bEnable);
    }
    if(u16Path_sel & FRC_PATHCTRL_OP2CONTRAST)
    {
        MDrv_FRC_OP2_ContrastEn(pInstance, bEnable);
    }
    if(u16Path_sel & FRC_PATHCTRL_OP2NOISEROUND)
    {
        MDrv_FRC_OP2_NoiseRoundEn(pInstance, bEnable);
    }

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_ColorPathCtrl( MS_U16 u16Path_sel, MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_FRC_COLOR_PATH_CONTROL XCArgs;
    XCArgs.u16Path_sel = u16Path_sel;
    XCArgs.bEnable = bEnable;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FRC_COLOR_PATH_CONTROL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set XC FRC_OP2 SetRGBGain
/// @param  u16RedGain                 \b IN: Red Gain
/// @param  u16GreenGain              \b IN: Green Gain
/// @param  u16BlueGain                \b IN: Blue Gain
//-------------------------------------------------------------------------------------------------
void MApi_XC_FRC_OP2_SetRGBGain_U2(void* pInstance, MS_U16 u16RedGain, MS_U16 u16GreenGain, MS_U16 u16BlueGain)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_FRC_OP2_SetRGBGain(pInstance, u16RedGain,  u16GreenGain,  u16BlueGain);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_OP2_SetRGBGain(MS_U16 u16RedGain, MS_U16 u16GreenGain, MS_U16 u16BlueGain)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_FRC_OP2_RGBGAIN XCArgs;
    XCArgs.u16RedGain = u16RedGain;
    XCArgs.u16GreenGain = u16GreenGain;
    XCArgs.u16BlueGain = u16BlueGain;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FRC_OP2_RGBGAIN, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set XC FRC_OP2 SetRGBOffset
/// @param  u16RedOffset                 \b IN: Red Offset
/// @param  u16GreenOffset              \b IN: Green Offset
/// @param  u16BlueOffset                \b IN: Blue Offset
//-------------------------------------------------------------------------------------------------
void MApi_XC_FRC_OP2_SetRGBOffset_U2(void* pInstance, MS_U16 u16RedOffset, MS_U16 u16GreenOffset, MS_U16 u16BlueOffset)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_FRC_OP2_SetRGBOffset(pInstance, u16RedOffset,  u16GreenOffset,  u16BlueOffset);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_OP2_SetRGBOffset(MS_U16 u16RedOffset, MS_U16 u16GreenOffset, MS_U16 u16BlueOffset)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_FRC_OP2_RGBOFFSET XCArgs;
    XCArgs.u16RedOffset = u16RedOffset;
    XCArgs.u16GreenOffset = u16GreenOffset;
    XCArgs.u16BlueOffset = u16BlueOffset;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FRC_OP2_RGBOFFSET, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// set XC FRC_OP2 SetDither
/// @param  u16dither                 \b IN: Dither setting
//-------------------------------------------------------------------------------------------------
void MApi_XC_FRC_OP2_SetDither_U2(void* pInstance, MS_U16 u16dither)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MDrv_FRC_OP2_SetDither(pInstance, u16dither);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_FRC_OP2_SetDither(MS_U16 u16dither)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_FRC_OP2_DITHER XCArgs;
    XCArgs.u16dither = u16dither;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FRC_OP2_DITHER, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_ForceReadFrame
/// @param  bEnable                                 \b IN: enable/disable keep specified frame
/// @param  u16FrameIndex                           \b IN: control specified frame to show
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_ForceReadFrame_U2(void* pInstance, MS_BOOL bEnable, MS_U16 u16FrameIndex)
{
    E_APIXC_ReturnValue bRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);

    bRet = MDrv_SC_ForceReadFrame(pInstance, bEnable, u16FrameIndex);

    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_ForceReadFrame(MS_BOOL bEnable, MS_U16 u16FrameIndex)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_FORCE_READ_FRAME XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.u16FrameIndex = u16FrameIndex;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FORCE_READ_FRAME, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif

E_APIXC_ReturnValue MApi_XC_OSDC_InitSetting_U2(void* pInstance,
                                               E_XC_OSDC_TGEN_Type e_osdctgen_type,
                                               MS_XC_OSDC_TGEN_INFO *pstOC_Tgen_Cus,
                                               MS_XC_OSDC_CTRL_INFO *pstOC_Ctrl)
{

    if(!SUPPORT_OSD_HSLVDS_PATH)
    {
        printf("Please check the osdc capability \n");
        return E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
    }
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    gSrcInfo[MAIN_WINDOW].Status2.bIsInitOSDC = TRUE;

    if(e_osdctgen_type == E_XC_OSDC_TGEN_MANUAL)
    {
        if(NULL == pstOC_Tgen_Cus)
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            return E_APIXC_RET_FAIL_INVALID_PARAMETER;
        }

        memcpy(&gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING, pstOC_Tgen_Cus, sizeof(MS_XC_OSDC_TGEN_INFO));
        memcpy(&gSrcInfo[MAIN_WINDOW].Status2.stXCOSDC_CTRL,  pstOC_Ctrl,     sizeof(MS_XC_OSDC_CTRL_INFO));
    }
    else
    {
        switch(e_osdctgen_type)
        {
            case E_XC_OSDC_TGEN_1366x768:
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0x618;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x326;

                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0x589;

                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = 0x309;
                break;

            case E_XC_OSDC_TGEN_1920x1080:
                if(pstOC_Ctrl->u16OC_Lpll_type == E_XC_PNL_LPLL_HS_LVDS)
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0x7B4; //0x800;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x448; //0x500;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0x7B3;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = 0x447;
                }
                else
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0x7FF; //should be odd(total cycle: htt+1),
                                                                                                   //vby one tx need even
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x500;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0x7B3;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = 0x447;

                }
                break;
            case E_XC_OSDC_TGEN_3840x2160:
                if(pstOC_Ctrl->u16OC_Lpll_type == E_XC_PNL_LPLL_HS_LVDS)
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0xFFF;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x982;//0xA00;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0xF33;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = 0x87F;
                }
                else
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0x10FF;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x900;//0xA00;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0xF33;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = 0x87F;
                }
                break;

            case E_XC_OSDC_TGEN_3840x1080:
                if(pstOC_Ctrl->u16OC_Lpll_type == E_XC_PNL_LPLL_HS_LVDS)
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0xFFF;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x982/2;//0xA00;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0xF33;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = (0x87F+1)/2-1;
                }
                else
                {
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HTotal        = 0x10FF;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VTotal        = 0x900/2;//0xA00;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Hsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_Start = 0x34;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_HframDE_End   = 0xF33;

                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_Start   = 0x04;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_Vsync_End     = 0x06;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_Start = 0x10;
                    gSrcInfo[MAIN_WINDOW].Status2.stXCOSDCTIMING.u16OC_Tgen_VframDE_End   = (0x87F+1)/2-1+8;
                }
                break;

            case E_XC_OSDC_TGEN_MAX:
            default:
                break;
        }
        memcpy(&gSrcInfo[MAIN_WINDOW].Status2.stXCOSDC_CTRL, pstOC_Ctrl, sizeof(MS_XC_OSDC_CTRL_INFO));
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_OSDC_InitSetting(E_XC_OSDC_TGEN_Type e_osdctgen_type,
                                             MS_XC_OSDC_TGEN_INFO *pstOC_Tgen_Cus,
                                             MS_XC_OSDC_CTRL_INFO *pstOC_Ctrl)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_OSDC_INITSETTING XCArgs;
    XCArgs.e_osdctgen_type = e_osdctgen_type;
    XCArgs.pstOC_Tgen_Cus = pstOC_Tgen_Cus;
    XCArgs.pstOC_Ctrl = pstOC_Ctrl;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OSDC_INITSETTING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
void MApi_XC_OSDC_SetOutVfreqx10_U2(void* pInstance, MS_U16 u16Vfreq)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    pXCResourcePrivate->stdrvXC_MVideo.OSDC_FREQ = u16Vfreq;
    printf("OSDC_FREQ=%u\n",pXCResourcePrivate->stdrvXC_MVideo.OSDC_FREQ);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_OSDC_SetOutVfreqx10(MS_U16 u16Vfreq)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_OSDC_SET_OUTPUTVFREQX10 XCArgs;
    XCArgs.u16Vfreq = u16Vfreq;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OSDC_SET_OUTPUTVFREQX10, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_OSDC_Control_U2(void* pInstance, MS_U32 eCtrl_type)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
//    MS_U8 u8LPLL_Type = 0;
//    u8LPLL_Type =(MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type;

    eReturn  = MDrv_XC_OSDC_Control(pInstance, eCtrl_type);

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_OSDC_Control(MS_U32 eCtrl_type)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_OSDC_CONTROL XCArgs;
    XCArgs.eCtrl_type = eCtrl_type;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OSDC_CONTROL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_OSDC_GetDstInfo_U2(void* pInstance, MS_OSDC_DST_DispInfo *pDstInfo, MS_U32 u32SizeofDstInfo)
{
    MS_XC_OSDC_TGEN_INFO stOC_Tgen;
    MS_XC_OSDC_CTRL_INFO stOC_Ctrl;

    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;

    if(NULL == pDstInfo)
    {
        printf("MApi_XC_OSDC_GetDstInfo():pDstInfo is NULL\n");
        return E_APIXC_RET_FAIL;
    }
    else if(u32SizeofDstInfo != sizeof(MS_OSDC_DST_DispInfo))
    {
        printf("MApi_XC_OSDC_GetDstInfo():u16SizeofDstInfo is different from the MS_XC_OSDC_DST_DispInfo defined, check header file!\n");
        return E_APIXC_RET_FAIL;
    }

    // the version control is coming in with version 1
    if(pDstInfo->ODSC_DISPInfo_Version < 1)
    {
        //We consider compatible operation form version1 , so reject the info init when version invalid
        XC_LOG_TRACE(XC_DBGLEVEL_SETTIMING, "MApi_XC_OSDC_GetDstInfo: please check your ODSC_DISPInfo_Version, it should not set to 0!!\n")
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

        return E_APIXC_RET_FAIL;
    }

    if(MDrv_XC_OSDC_GetDataFromRegister(pInstance, &stOC_Tgen, &stOC_Ctrl))
    {
        pDstInfo->VDTOT                = stOC_Tgen.u16OC_Tgen_VTotal;
        pDstInfo->DEVST                = stOC_Tgen.u16OC_Tgen_VframDE_Start;
        pDstInfo->DEVEND               = stOC_Tgen.u16OC_Tgen_VframDE_End;
        pDstInfo->HDTOT                = stOC_Tgen.u16OC_Tgen_HTotal;
        pDstInfo->DEHST                = stOC_Tgen.u16OC_Tgen_HframDE_Start;
        pDstInfo->DEHEND               = stOC_Tgen.u16OC_Tgen_HframDE_End;

        pDstInfo->SYNCHST              = stOC_Tgen.u16OC_Tgen_Hsync_Start;
        pDstInfo->SYNCHEND             = stOC_Tgen.u16OC_Tgen_Hsync_End;
        pDstInfo->SYNCVST              = stOC_Tgen.u16OC_Tgen_Vsync_Start;
        pDstInfo->SYNCVEND             = stOC_Tgen.u16OC_Tgen_Vsync_End;

        pDstInfo->bCLK_EN              = stOC_Ctrl.bOC_ClK_En;
        pDstInfo->bMIXER_BYPASS_EN     = stOC_Ctrl.bOC_Mixer_Bypass_En;
        pDstInfo->bMIXER_INVALPHA_EN   = stOC_Ctrl.bOC_Mixer_InvAlpha_En;
        pDstInfo->bMIXER_HSTVFDEOUT_EN = stOC_Ctrl.bOC_Mixer_Hsync_Vfde_Out;
        pDstInfo->bMIXER_HFDEVFDEOUT_EN= stOC_Ctrl.bOC_Mixer_Hfde_Vfde_Out;

        eRet = E_APIXC_RET_OK;
    }
    else
    {
        pDstInfo->VDTOT                = 0;
        pDstInfo->HDTOT                = 0;
        pDstInfo->DEHST                = 0;
        pDstInfo->DEHEND               = 0;
        pDstInfo->DEVST                = 0;
        pDstInfo->DEVEND               = 0;

        pDstInfo->SYNCHST              = 0;
        pDstInfo->SYNCHEND             = 0;
        pDstInfo->SYNCVST              = 0;
        pDstInfo->SYNCVEND             = 0;

        pDstInfo->bCLK_EN              = DISABLE;
        pDstInfo->bMIXER_BYPASS_EN     = DISABLE;
        pDstInfo->bMIXER_INVALPHA_EN   = DISABLE;
        pDstInfo->bMIXER_HSTVFDEOUT_EN = DISABLE;
        pDstInfo->bMIXER_HFDEVFDEOUT_EN= DISABLE;

        eRet = E_APIXC_RET_FAIL;
    }

    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_OSDC_GetDstInfo(MS_OSDC_DST_DispInfo *pDstInfo, MS_U32 u32SizeofDstInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_OSDC_GET_DESTINATION_INFO XCArgs;
    XCArgs.pDstInfo = pDstInfo;
    XCArgs.u32SizeofDstInfo = u32SizeofDstInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OSDC_GET_DESTINATION_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#define LOGTIMECOST  FALSE
XC_PCMONITOR_STATUS _MApi_XC_PCMonitor_Exe(void* pInstance, SCALER_WIN eWindow, INPUT_SOURCE_TYPE_t enInputSourceType)
{
    MApi_XC_PCMonitor_Restart_U2(pInstance,eWindow);
    MS_U32 u32maxRetryTime = 40;
    XC_PCMONITOR_STATUS enPCMonitorStatus = E_XC_PCMONITOR_STABLE_NOSYNC;
    while ( enPCMonitorStatus != E_XC_PCMONITOR_STABLE_SYNC )
    {
        if(u32maxRetryTime == 0)
        {
            printf("\033[1;31mRetry Time out.Signal Unstable or No Signal!!\033[0m\n");
            break;
        }

        enPCMonitorStatus = MApi_XC_PCMonitor_U2(pInstance,enInputSourceType, eWindow);

        if(enPCMonitorStatus == E_XC_PCMONITOR_STABLE_SYNC)
        {
            printf("Signal Stable!!\n");
            break;
        }

        MsOS_DelayTask(10);
        u32maxRetryTime--;
    }
    return enPCMonitorStatus;
}

MS_U32 MApi_XC_SetPowerState_U2(void* pInstance, EN_POWER_MODE enPowerState)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
#ifndef MSOS_TYPE_OPTEE
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
#else
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
#endif

#if LOGTIMECOST
    MS_U32 u32Begin = MsOS_GetSystemTime();
#endif
#ifndef MSOS_TYPE_OPTEE
    MS_U32 u32Ret = UTOPIA_STATUS_FAIL;
    MS_U8 i = 0;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if (enPowerState == E_POWER_SUSPEND)
    {
        pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState = enPowerState;
        pXCResourcePrivate->stdrvXC_MVideo._bSkipSWReset = FALSE;

#ifdef MSOS_TYPE_LINUX_KERNEL
        /*
        ** Save setwindow info for str
        */
        void *pModule;
        UtopiaInstanceGetModule(pInstance, &pModule);
        XC_REGS_SAVE_AREA *pXCResourceStr = NULL;
        UtopiaModuleGetSTRPrivate(pModule, (void**)&pXCResourceStr);

        MApi_XC_ACE_GetColorMatrix_U2(pInstance,MAIN_WINDOW, pXCResourceStr->u16TmpColorMatrix);
#if (SUPPORT_OP2_TEST_PATTERN >0)
        pXCResourcePrivate->stdrvXC_MVideo.u16OP2_color_3x3 = SC_R2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_2F_L,0xFFFF);
#endif
        MDrv_XC_ADC_GetGainSetting(pInstance, &(pXCResourceStr->stADCSetting));

        pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = TRUE;

        // Because of in linux kernel mode, XC can do STR without XC_init.
        // Get IO_Base again for protecting
        // (NG flow: without XC init => do STR => kernel call XCStr callback
        // => call MApi_XC_SetPowerState_U2
        // => No _XC_RIU_BASE)
        if (_XC_RIU_BASE == 0)
        {
            if (MDrv_XC_SetIOMapBase_i(pInstance) != TRUE)
            {
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "MApi_XC_Init MDrv_XC_SetIOMapBase() failure\n");
            }
            MDrv_XC_SetDeviceOffset(pInstance);
        }
#endif

#if (HW_DESIGN_4K2K_VER == 7)
        //Temp delete path when STR(need to refine msAPI)
        INPUT_SOURCE_TYPE_t oldMainSource = pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[MAIN_WINDOW];
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        MApi_XC_Mux_DeletePath_U2(pInstance, oldMainSource, OUTPUT_SCALER_MAIN_WINDOW);
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#endif

        #if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6) && MANHATTAN_GOP_HW_BUG_PATCH)
        // For manhattan
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
        pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
        _MLOAD_ENTRY(pInstance);
        MHal_FRC_AdjustGOPPosition(pInstance);
        MDrv_XC_MLoad_Fire(pInstance,TRUE);
        _MLOAD_RETURN(pInstance);
        #endif
        MDrv_XC_MLoad_Enable(pInstance,  FALSE);

#if FRC_INSIDE
        // disable FRC_PQ client during suspend flow to avoid MIU hit
        MHal_FRC_Set_TimingStable(pInstance, FALSE);
#endif

        // disable FRC during suspend flow to avoid FRC_R2 MIU hit
        MDrv_FRC_ByPass_Enable(pInstance,  TRUE);

        //DISABLE Interrupt
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
            MsOS_DisableInterrupt(E_INT_IRQ_DISP);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_DisableInterrupt(E_INT_IRQ_EC_BRIDGE);
            MsOS_DisableInterrupt(E_INT_IRQ_DISP1);
        }
        MDrv_SC_set_interrupt(pInstance, SC_INT_VSINT, DISABLE); //Mask Output Vsync for Hanging issue for CPU to bussy (Mantis 1180314)
#if 0
#ifndef MSOS_TYPE_LINUX_KERNEL
        // Release ISR
        if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
        {
        MsOS_DetachInterrupt(E_INT_IRQ_DISP);
        }
        else if(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
        {
            MsOS_DetachInterrupt(E_INT_IRQ_EC_BRIDGE);
            MsOS_DetachInterrupt(E_INT_IRQ_DISP1);
        }
#endif
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL

#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
        if (  ((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM]+gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM])!=0)
            &&((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM]+gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM])!=0))
        {
            // signal stable,but cma buff is not use
            // setwindow will be done when str resume.
            pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
        }

#endif
#endif

#endif
        u32Ret = UTOPIA_STATUS_SUCCESS;
    }
    else if (enPowerState == E_POWER_RESUME)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState == E_POWER_SUSPEND)
        {
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBAddress[MAIN_WINDOW] = 0xFFFFFFFF;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBSize[MAIN_WINDOW] = 0x0;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBAddress[SUB_WINDOW] = 0xFFFFFFFF;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBSize[SUB_WINDOW] = 0x0;
            pXCResourcePrivate->stdrvXC_Scaling._u32PreDualFBAddress[MAIN_WINDOW] = 0xFFFFFFFF;
            pXCResourcePrivate->stdrvXC_Scaling._u32PreDualFBSize[MAIN_WINDOW] = 0x0;
            pXCResourcePrivate->stdrvXC_Scaling._u32PreDualFBAddress[SUB_WINDOW] = 0xFFFFFFFF;
            pXCResourcePrivate->stdrvXC_Scaling._u32PreDualFBSize[SUB_WINDOW] = 0x0;

            #if FRC_INSIDE
            pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited = FALSE;
            #endif

            for(i = 0; i < MAX_WINDOW; i++)
            {
                MDrv_SC_GenerateBlackVideo(pInstance, TRUE, (SCALER_WIN)i);
            }

            _MApi_XC_Init_WithoutCreateMutex(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32InitDataLen_Suspend);

            MDrv_XC_OSDC_Control(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32OSDCCtrltype_Suspend);

            MDrv_XC_Init_MISC(pInstance, &pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32InitMiscDataLen_Suspend);

            if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MLoadBufByteLen_Suspend > 0)
            {
#if (SUPPORT_KERNEL_MLOAD == 1)
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                //dc on should do menuload init
                pXCResourcePrivate->stdrvXC_MVideo_Context.stdrvXC_Menuload[psXCInstPri->u32DeviceID].g_bMLoadInitted = FALSE;
                MApi_XC_MLoad_Init_U2(pInstance,
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MLoadPhyAddr_Suspend,
                pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MLoadBufByteLen_Suspend);
                _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#else
                _MLOAD_ENTRY(pInstance);
                //dc on should do menuload init
                pXCResourcePrivate->stdrvXC_MVideo_Context.stdrvXC_Menuload[psXCInstPri->u32DeviceID].g_bMLoadInitted = FALSE;
                MDrv_XC_MLoad_Init(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MLoadPhyAddr_Suspend);
                _MLOAD_RETURN(pInstance);
#endif
            }

            for(i = 0; i < E_XC_MCDI_MAX; i++)
            {
                if((pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MCDIFBSize_Suspend[i] > 0) && (i < E_XC_MCDI_SUB_BOTH))
                {
                    pXCResourcePrivate->stdrvXC_Display._u32MCDIFBAddress[i]=0xFFFFFFFF;
                    pXCResourcePrivate->stdrvXC_Display._u32MCDIFBSize[i]=0x0;
                    MDrv_SC_SetMCDIBufferAddress(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MCDIFBAddress_Suspend[i],
                                                 pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MCDIFBSize_Suspend[i],
                                                 (E_XC_MCDI_TYPE)i);
                }
                if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_bEnableMCDI_Suspend[i])
                {
                    MDrv_SC_EnableMCDI(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_bEnableMCDI_Suspend[i], (E_XC_MCDI_TYPE)i);
                }
            }

            for(i = 0; i < MAX_WINDOW; i++)
            {
                if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_u8DispWindowColor_Suspend[i] > 0)
                {
                    MDrv_XC_SetDispWindowColor(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u8DispWindowColor_Suspend[i], (SCALER_WIN)i);
                }
            }

            if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32FrameColor_Suspend > 0)
            {
                MDrv_XC_SetFrameColor(pInstance, pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32FrameColor_Suspend, FALSE);
            }

#ifndef MSOS_TYPE_LINUX_KERNEL
            MDrv_SC_SetSkipWaitVsync(pInstance, MAIN_WINDOW, TRUE);
            MDrv_SC_SetSkipWaitVsync(pInstance, SUB_WINDOW , TRUE);
#endif

            for(i = 0; i < MAX_WINDOW; i++)
            {

#ifdef MSOS_TYPE_LINUX_KERNEL
                MDrv_SC_GenerateBlackVideo(pInstance, TRUE, (SCALER_WIN)i);
#else
                MDrv_SC_GenerateBlackVideo(pInstance, gSrcInfo[i].bBlackscreenEnabled, (SCALER_WIN)i);
#endif
            }

#ifndef MSOS_TYPE_LINUX_KERNEL
            MDrv_SC_SetSkipWaitVsync(pInstance, MAIN_WINDOW, FALSE);
            MDrv_SC_SetSkipWaitVsync(pInstance, SUB_WINDOW , FALSE);
#endif

            #if (LD_ENABLE == 1)
            // Local Dimming Recovery procedure
            // In Local Dimming situation, we don't know if AP has a mmap for Local Dimming
            // we can only judge it by checking if it is inited so we will try to resume.
            if (pXCResourcePrivate->stdrv_LD._bMDrvLD_InitFinished == TRUE)
            {
                MDrv_XC_LD_Set_MIUSel(pInstance, pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u8MIU);

                MDrv_XC_LD_SetMemoryAddress(pInstance, pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32LDFReadBuf0Base,
                                            pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32LDFReadBuf1Base,
                                            pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32LDBWriteBuf0Base,
                                            pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32LDBWriteBuf1Base,
                                            pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32Edge2DCoeffBase,
                                            (pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32SPIDataBuf0Base
                                             -pXCResourcePrivate->stdrv_LD.gsLDDramBaseAddr.u32LDBWriteBuf0Base));

                MDrv_XC_LD_Init(pInstance, pXCResourcePrivate->stdrv_LD._enLDPanelType);

                // Must call or otherwise our local dimming report value will be only 1 pixel
                // but actually we want it to be the whole screen so we divided it into 32*18 areas of the panel screen.
                MS_U8 u8LDData[576] = {0};
                MDrv_LD_Get_Value(pInstance, u8LDData, 32, 18);
            }
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
            if(pXCResourcePrivate->stdrvXC_Display._stDMAInitInfo.phyBaseAddr != 0)
            {
                MDrv_XC_HDR_Control(pInstance, E_XC_HDR_CTRL_INIT_DMA, (void*) &pXCResourcePrivate->stdrvXC_Display._stDMAInitInfo);
            }
#endif
#endif
            //pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState = enPowerState;

#ifdef MSOS_TYPE_LINUX_KERNEL
            if(E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
            {
                void *pModule;
                _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
                //****************to do :  add
                /* Get Str Resource*/
                UtopiaInstanceGetModule(pInstance, &pModule);
                XC_REGS_SAVE_AREA *pXCResourceStr = NULL;
                UtopiaModuleGetSTRPrivate(pModule, (void**)&pXCResourceStr);
                /*1.create_mux,*/
                MS_S16 s16PathId;
                INPUT_SOURCE_TYPE_t currentSrc[MAX_WINDOW];
                currentSrc[MAX_WINDOW] = INPUT_SOURCE_NONE;
                //get path data from shm
                memset(&pXCResourceStr->Path_Info[0],0,sizeof(XC_MUX_PATH_INFO)*MAX_DATA_PATH_SUPPORTED);
                for(i=0;i<MAX_DATA_PATH_SUPPORTED;i++)
                {
                    if(pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus & DATA_PATH_USING)
                    {
                        pXCResourceStr->Path_Info[i].Path_Type                 = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].e_PathType;
                        pXCResourceStr->Path_Info[i].src                       = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].src;
                        pXCResourceStr->Path_Info[i].dest                      = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest;
                        pXCResourceStr->Path_Info[i].path_thread               = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].path_thread;
                        pXCResourceStr->Path_Info[i].SyncEventHandler          = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].synchronous_event_handler;
                        pXCResourceStr->Path_Info[i].DestOnOff_Event_Handler   = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_on_off_event_handler;
                        pXCResourceStr->Path_Info[i].dest_periodic_handler     = pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].dest_periodic_handler;
                        pXCResourceStr->u8PathStatus[i]                         =  pXCResourcePrivate->stdrvXC_MVideo.s_InputSource_Mux_attr.Mux_DataPaths[i].u8PathStatus;
                        s16PathId = MApi_XC_Mux_DeletePath_U2(pInstance,pXCResourceStr->Path_Info[i].src, pXCResourceStr->Path_Info[i].dest);
                        if(s16PathId >=0)
                        {
                            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE,"Delete Path Success !!\n");
                            s16PathId = MApi_XC_Mux_CreatePath_U2(pInstance,&(pXCResourceStr->Path_Info[i]), sizeof(XC_MUX_PATH_INFO) );
                            if(s16PathId != -1)
                            {
                                if( MApi_XC_Mux_EnablePath_U2(pInstance,s16PathId) != -1 )
                                {
                                    if(pXCResourceStr->Path_Info[i].dest == OUTPUT_SCALER_MAIN_WINDOW)
                                    {
                                        MApi_XC_SetInputSource_U2(pInstance,pXCResourceStr->Path_Info[i].src, MAIN_WINDOW);
                                        currentSrc[MAIN_WINDOW] = pXCResourceStr->Path_Info[i].src;
                                    }
                                    else if((pXCResourceStr->Path_Info[i].dest == OUTPUT_SCALER_SUB_WINDOW)&&pXCResourceStr->bPIP_enable)
                                    {
                                        MApi_XC_SetInputSource_U2(pInstance,pXCResourceStr->Path_Info[i].src, SUB_WINDOW);
                                        currentSrc[SUB_WINDOW] = pXCResourceStr->Path_Info[i].src;
                                    }
                                    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE," Enable Path Success \n");
                                }
                                else
                                {
                                    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE," Enable Path Error \n");
                                    u32Ret = UTOPIA_STATUS_FAIL;
                                }
                            }else
                            {
                                XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE,"Create Path Error !!\n");
                                u32Ret = UTOPIA_STATUS_FAIL;
                            }
                        }
                        else
                        {
                            XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE,"Delete Path Error !!\n");
                        }


                    }
                }
                MApi_XC_DisableInputSource_U2(pInstance,DISABLE, MAIN_WINDOW);

                //if(pXCResourceStr->bPIP_enable)
                //{
                //    MApi_XC_DisableInputSource_U2(pInstance,DISABLE, SUB_WINDOW);
                //}
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
                if (  ((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_SELF_MEM]+gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_SELF_MEM])==0)
                    &&((gSrcInfo[MAIN_WINDOW].u32PreCMAMemSCMSize[CMA_XC_COBUFF_MEM]+gSrcInfo[MAIN_WINDOW].u32PreCMAMemFRCMSize[CMA_XC_COBUFF_MEM])==0))
                {
                    // setwindow will be done when str resume.
                    pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                }
#endif
#endif
                if(pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr)
                {
                    pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                    // wait xc front module resume done
                    if(IsSrcTypeYPbPr(gSrcInfo[MAIN_WINDOW].enInputSourceType) ||IsSrcTypeVga(gSrcInfo[MAIN_WINDOW].enInputSourceType)
                        ||IsSrcTypeHDMI(gSrcInfo[MAIN_WINDOW].enInputSourceType))
                    {
                        //get signal status
                        XC_IP_SYNC_STATUS  sXC_Sync_Status = {0};
                        MS_U16 u16DelayTime = 0;
                        MS_U16 u16DelayTimeTemp = 0;
                        MS_U16 u16PreVototal = 0,u16PreHperiod = 0;

                        MDrv_XC_GetSyncStatus(pInstance,gSrcInfo[MAIN_WINDOW].enInputSourceType,&sXC_Sync_Status,MAIN_WINDOW);
                        u16DelayTime = (DELAY_FRAME_NUM)*(1000/(gSrcInfo[MAIN_WINDOW].u16InputVFreq/10));

                        while(((sXC_Sync_Status.u8SyncStatus & XC_MD_HSYNC_LOSS_BIT)||(sXC_Sync_Status.u8SyncStatus & XC_MD_VSYNC_LOSS_BIT))
                                    &&(u16DelayTimeTemp < u16DelayTime))
                        {
                            MsOS_DelayTask(1);
                            u16DelayTimeTemp++;
                            MDrv_XC_GetSyncStatus(pInstance,gSrcInfo[MAIN_WINDOW].enInputSourceType,&sXC_Sync_Status,MAIN_WINDOW);
                        }

                        if(u16DelayTimeTemp > u16DelayTime)
                        {
                                printf("[%s,%d] Signal detect  Time Out!\n",__FUNCTION__,__LINE__);
                                pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                        }
                        else
                        {
                            u16PreVototal = pXCResourcePrivate->stdrvXC_IP.sPCMONITOR_status[MAIN_WINDOW].u16Vtotal;
                            u16PreHperiod = pXCResourcePrivate->stdrvXC_IP.sPCMONITOR_status[MAIN_WINDOW].u16Hperiod;

                            if(_MApi_XC_PCMonitor_Exe(pInstance,MAIN_WINDOW, gSrcInfo[MAIN_WINDOW].enInputSourceType) == E_XC_PCMONITOR_STABLE_SYNC)
                            {
                                if((abs(u16PreVototal -pXCResourcePrivate->stdrvXC_IP.sPCMONITOR_status[MAIN_WINDOW].u16Vtotal)>MD_VTOTAL_TORLANCE)
                                  ||((abs(u16PreHperiod -pXCResourcePrivate->stdrvXC_IP.sPCMONITOR_status[MAIN_WINDOW].u16Hperiod)>MD_HPERIOD_TORLANCE)))
                                {
                                    printf("[%s,%d] Timing has changed!\n",__FUNCTION__,__LINE__);
                                    pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                                    pXCResourcePrivate->stdrvXC_MVideo.bTimingUnstableForStr = TRUE;
                                }
                                else
                                {
                                    pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = TRUE;
                                }
                            }
                            else
                            {
                                pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                                pXCResourcePrivate->stdrvXC_MVideo.bTimingUnstableForStr = TRUE;
                            }
                        }
                    }
                    else if(IsSrcTypeAV(gSrcInfo[MAIN_WINDOW].enInputSourceType)||IsSrcTypeATV(gSrcInfo[MAIN_WINDOW].enInputSourceType)||IsSrcTypeDTV(gSrcInfo[MAIN_WINDOW].enInputSourceType))
                    {
                        UtopiaStrWaitCondition("xc", enPowerState, 0);
                        pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = TRUE;
                        if(IsSrcTypeAV(gSrcInfo[MAIN_WINDOW].enInputSourceType))
                        {
                            const char Vd_Key[] = "timing change";
                            char Value[255];
                            if(UTOPIA_STATUS_SUCCESS == (UtopiaStrGetData((char *)&Vd_Key,(char *)&Value)))
                            {
                                if(0 == strcmp(Value,"yes" ))
                                {
                                    printf("[%s,%d] Timing has changed!\n",__FUNCTION__,__LINE__);
                                    pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
                                }
                            }
                        }
                    }

                    if(pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr)
                    {
                        /*2.set_window,*/
                        if(MApi_XC_SetWindow_U2(pInstance,&(pXCResourceStr->pstXC_SetWin_Info[MAIN_WINDOW]), sizeof(XC_SETWIN_INFO), MAIN_WINDOW) ==FALSE)
                        {
                            u32Ret = UTOPIA_STATUS_FAIL;
                        }
                        /*3.set_paneltiming,*/
                        MApi_XC_SetPanelTiming_U2(pInstance,&(pXCResourceStr->pTimingInfo), MAIN_WINDOW);
                        /*4.load_pq,*/
                        MDrv_PQ_LoadSettings(PQ_MAIN_WINDOW);

                        MApi_XC_ACE_SetColorMatrix_U2( pInstance,MAIN_WINDOW, pXCResourceStr->u16TmpColorMatrix);
#if (SUPPORT_OP2_TEST_PATTERN >0)
                        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_2F_L,pXCResourcePrivate->stdrvXC_MVideo.u16OP2_color_3x3,0xFFFF);
#endif
                        /*4.pq delay,*/
                        MS_U16 u16PQdelay = 0;
                        u16PQdelay=MDrv_PQ_GetDelayTime(PQ_MAIN_WINDOW);
                        MsOS_DelayTask(u16PQdelay);
                    }
                }

                //if(pXCResourceStr->bPIP_enable)
                //{
                //    if(MApi_XC_SetWindow_U2(pInstance,&(pXCResourceStr->pstXC_SetWin_Info[SUB_WINDOW]), sizeof(XC_SETWIN_INFO), SUB_WINDOW) ==FALSE)
                //    {
                //        u32Ret = UTOPIA_STATUS_FAIL;
                //    }
                //}
                if(currentSrc[MAIN_WINDOW] == INPUT_SOURCE_VGA)
                    {
                    MApi_XC_ADC_AdjustGainOffset_U2(pInstance, &(pXCResourceStr->stADCSetting));
                }

                /// For CVBS out, recover ADC table
                if(pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr)
                {
                    if(IsSrcTypeATV(gSrcInfo[MAIN_WINDOW].enInputSourceType) || IsSrcTypeDTV(gSrcInfo[MAIN_WINDOW].enInputSourceType) || IsSrcTypeSV(gSrcInfo[MAIN_WINDOW].enInputSourceType))
                    {
                        MApi_XC_ADC_SetCVBSOut_U2(pInstance, TRUE, OUTPUT_CVBS1 , gSrcInfo[MAIN_WINDOW].enInputSourceType, TRUE);
                    }
                }

                _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
            }
#endif
            u32Ret = UTOPIA_STATUS_SUCCESS;
            //enable mload lastly for saving  time when power resume
            //when you need to add code,put them ahead of here
            if(pXCResourcePrivate->stdrvXC_MVideo_Context.g_u32MLoadBufByteLen_Suspend > 0)
            {
                _MLOAD_ENTRY(pInstance);
                MDrv_XC_MLoad_Enable(pInstance,  TRUE);
                _MLOAD_RETURN(pInstance);
            }

#ifdef MSOS_TYPE_LINUX_KERNEL
            for(i = 0; i < MAX_WINDOW; i++)
            {
                if(pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr)
                {
                MDrv_SC_GenerateBlackVideo(pInstance, gSrcInfo[i].bBlackscreenEnabled, (SCALER_WIN)i);
            }

            }
#endif

        }
        else
        {
            printf("[%s,%5d]It is not suspended yet. We shouldn't resume\n",__FUNCTION__,__LINE__);
            u32Ret = UTOPIA_STATUS_FAIL;
        }
        pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState = enPowerState;
    }
    else
    {
        printf("[%s,%5d]Do Nothing: %u\n",__FUNCTION__,__LINE__,enPowerState);
        u32Ret = UTOPIA_STATUS_FAIL;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
#if LOGTIMECOST
    printf("=========[%s,%5d] cost %lu ms========\n", __FUNCTION__, __LINE__, MsOS_GetSystemTime() - u32Begin);
#endif

    return u32Ret;

#else  ////For Optee STR
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));


    if (enPowerState == E_POWER_SUSPEND)
    {
        ///Do nothing for optee suspend
    }
    else if (enPowerState == E_POWER_RESUME)
    {
        if(pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable == TRUE)
        {
            if((E_XC_DEVICE0 == psXCInstPri->u32DeviceID)
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
              ||(E_XC_DEVICE1 == psXCInstPri->u32DeviceID)
#endif
              )
            {
#if 0 //which will be locked by seal side
                Hal_SC_secure_lock(pInstance, MAIN_WINDOW, TRUE, TRUE);
#endif
                /// recorver for FB adress REG

                MDrv_XC_OPTEE_Control(pInstance,E_XC_OPTEE_UPDATE_HANDLER,&pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW]);
            }
        }
        else if(pXCResourcePrivate->sthal_Optee.op_tee_xc[SUB_WINDOW].isEnable == TRUE)
        {
#if 0 //which will be locked by seal side
            Hal_SC_secure_lock(pInstance, SUB_WINDOW, TRUE, TRUE);
#endif
            MDrv_XC_OPTEE_Control(pInstance,E_XC_OPTEE_UPDATE_HANDLER,&pXCResourcePrivate->sthal_Optee.op_tee_xc[SUB_WINDOW]);
        }
    }

    return UTOPIA_STATUS_SUCCESS;

#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_SetPowerState(EN_POWER_MODE enPowerState)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    stXC_SET_POWERSTATE XCArgs;
    XCArgs.enPowerState = enPowerState;
    XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_POWERSTATE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }

#if (MAX_XC_DEVICE_NUM == 2) // Support SC1
    if (pu32XCInst_1 == NULL)
    {
        printf("[%s,%5d] No SC1 instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    XCArgs.enPowerState = enPowerState;
    XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;

    if(UtopiaIoctl(pu32XCInst_1, E_XC_CMD_SET_POWERSTATE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
#endif

    return XCArgs.u32ReturnValue;
}
#endif

//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetByPassOSDVsyncPos
/// @param  u16VsyncStart                              \b IN: OSD vsync start
/// @param  u16VsyncEnd                                \b IN: OSD vsync end
//-------------------------------------------------------------------------------------------------
void MApi_XC_BYPASS_SetOSDVsyncPos_U2(void* pInstance, MS_U16 u16VsyncStart, MS_U16 u16VsyncEnd)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    _XC_ENTRY(pInstance);

    MHal_XC_ByPass_SetOSDVsyncPos(pInstance, u16VsyncStart, u16VsyncEnd);

    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_BYPASS_SetOSDVsyncPos(MS_U16 u16VsyncStart, MS_U16 u16VsyncEnd)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BYPASS_OSDVSYNC_POS XCArgs;
    XCArgs.u16VsyncStart = u16VsyncStart;
    XCArgs.u16VsyncEnd = u16VsyncEnd;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BYPASS_OSDVSYNC_POS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_BYPASS_SetInputSrc
/// @param  bEnable                              \b IN: ENABLE/DISABLE bypass mode
/// @param  input                                \b IN: Select the input source for bypass mode
//-------------------------------------------------------------------------------------------------
void MApi_XC_BYPASS_SetInputSrc_U2(void* pInstance, MS_BOOL bEnable, E_XC_BYPASS_InputSource input)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    MHal_XC_BYPASS_Setinputclk(pInstance, bEnable,input);

    _XC_ENTRY(pInstance);
    MHal_XC_Bypass_SetinputSrc(pInstance, input);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_BYPASS_SetInputSrc(MS_BOOL bEnable, E_XC_BYPASS_InputSource input)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_SET_BYPASS_INPUTSRC XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.input = input;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BYPASS_INPUTSRC, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_SetSeamlessZapping
/// @param  eWindow                 \b IN: which window we are going to enable or disable
/// @param  bEnable                 \b IN: ENABLE/DISABLE seamless zapping
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_SetSeamlessZapping_U2(void* pInstance, SCALER_WIN eWindow, MS_BOOL bEnable)
{
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    if(!SUPPORT_SEAMLESS_ZAPPING)
    {
        XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
        return E_APIXC_RET_FAIL;
    }
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    if(!MDrv_XC_GetDynamicScalingStatus(pInstance))
    {
        if(!bEnable)
        {
            eRet = E_APIXC_RET_OK;
        }
        else
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SEAMLESSZAP, "[%s][%06d] Dynamic scaling is not initialized\n", __FUNCTION__, __LINE__);
            eRet = E_APIXC_RET_FAIL;
        }
    }
    else
    {
        _XC_ENTRY(pInstance);
        if(bEnable)
        {
            MDrv_XC_Set_DSIndexSourceSelect(pInstance, E_XC_DS_INDEX_SCALER, eWindow);
        }
        else
        {
            MDrv_XC_Set_DSIndexSourceSelect(pInstance, E_XC_DS_INDEX_MVOP, eWindow);
        }

        eRet = Hal_SC_SetSeamlessZapping(pInstance, eWindow, bEnable);
        XC_LOG_TRACE(XC_DBGLEVEL_SEAMLESSZAP, "[%s][%06d] set Seamless Zapping[%s] = %x\n", __FUNCTION__, __LINE__, eWindow?"SUB":"MAIN", bEnable);

        _XC_RETURN(pInstance);
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetSeamlessZapping(SCALER_WIN eWindow, MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_SEAMLESS_ZAPPING XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.bEnable = bEnable;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_SEAMLESS_ZAPPING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif

E_APIXC_ReturnValue MApi_XC_GetSeamlessZappingStatus_U2(void* pInstance, SCALER_WIN eWindow, MS_BOOL* pbEnable)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");

    E_APIXC_ReturnValue eReturnValue = E_APIXC_RET_FAIL;

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    *pbEnable = Hal_SC_GetSeamlessZappingStatus(pInstance, eWindow);
    XC_LOG_TRACE(XC_DBGLEVEL_SEAMLESSZAP, "[%s][%06d] get Seamless Zapping[%s] = %x\n", __FUNCTION__, __LINE__, eWindow?"SUB":"MAIN", *pbEnable);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    eReturnValue = E_APIXC_RET_OK;

    return eReturnValue;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_GetSeamlessZappingStatus(SCALER_WIN eWindow, MS_BOOL* pbEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_SEAMLESS_ZAPPING_STATUS XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.pbEnable = pbEnable;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_SEAMLESS_ZAPPING_STATUS, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Vtrack_SetPayloadData
/// @param  u16Timecode                       \b IN: The time code is coded on 16 bits. It is calculated based on the number of days from January 1st.
/// @param  u8ID                              \b IN: Operator ID
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Vtrack_SetPayloadData_U2(void* pInstance, MS_U16 u16Timecode, MS_U8 u8OperatorID)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    _MLOAD_ENTRY(pInstance);
    eReturn = MHal_XC_Vtrack_SetPayloadData(pInstance, u16Timecode, u8OperatorID);
    _MLOAD_RETURN(pInstance);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Vtrack_SetPayloadData(MS_U16 u16Timecode, MS_U8 u8OperatorID)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VTRACK_PAYLOADDATA XCArgs;
    XCArgs.u16Timecode = u16Timecode;
    XCArgs.u8OperatorID = u8OperatorID;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VTRACK_PAYLOADDATA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Vtrack_SetAllPayloadData
/// @param  vtrack_payload                       \b IN: Vtrack payload data
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Vtrack_SetAllPayloadData_U2(void* pInstance, XC_VTRACK_PAYLOAD *vtrack_payload)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    _MLOAD_ENTRY(pInstance);
    eReturn = MHal_XC_Vtrack_SetAllPayloadData(pInstance, vtrack_payload);
    _MLOAD_RETURN(pInstance);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Vtrack_SetAllPayloadData(XC_VTRACK_PAYLOAD *vtrack_payload)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }
    stXC_SET_VTRACK_ALLPAYLOADDATA XCArgs;
    XCArgs.vtrack_payload = vtrack_payload;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VTRACK_ALLPAYLOADDATA, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Vtrack_SetVtrackKey
/// @param  u64VtrackKey                       \b IN: Vtrack Key
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Vtrack_SetVtrackKey_U2(void* pInstance, MS_U64 u64VtrackKey)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    _MLOAD_ENTRY(pInstance);
    eReturn = MHal_XC_Vtrack_SetVtrackKey(pInstance, u64VtrackKey);
    _MLOAD_RETURN(pInstance);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Vtrack_SetVtrackKey(MS_U64 u64VtrackKey)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VTRACK_VTRACKKEY XCArgs;
    XCArgs.u64VtrackKey= u64VtrackKey;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VTRACK_VTRACKKEY, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Vtrack_SetUserDefindedSetting
/// @param  bEnable                              \b IN: ENABLE/DISABLE Customized Setting
/// @param  pu8Setting                           \b IN: pointer to targert user definded setting data (include setting1 to setting3)
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Vtrack_SetUserDefindedSetting_U2(void *pInstance, MS_BOOL bUserDefinded, MS_U8 *pu8Setting)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    _MLOAD_ENTRY(pInstance);
    eReturn = MHal_XC_Vtrack_SetUserDefindedSetting(pInstance, bUserDefinded, pu8Setting);
    _MLOAD_RETURN(pInstance);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Vtrack_SetUserDefindedSetting(MS_BOOL bUserDefinded, MS_U8 *pu8Setting)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VTRACK_USERDEFINED_SETTING XCArgs;
    XCArgs.bUserDefinded = bUserDefinded;
    XCArgs.pu8Setting = pu8Setting;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VTRACK_USERDEFINED_SETTING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Vtrack_Enable
/// @param  u8FrameRate                          \b IN: ENABLE/DISABLE The FrameRateIn is based on the output format of the box
/// @param  bEnable                              \b IN: ENABLE/DISABLE Vtrack
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Vtrack_Enable_U2(void* pInstance, MS_U8 u8FrameRate, MS_BOOL bEnable)
{
    E_APIXC_ReturnValue eReturn = E_APIXC_RET_FAIL;
    _MLOAD_ENTRY(pInstance);
    eReturn = MHal_XC_Vtrack_Enable(pInstance, u8FrameRate, bEnable);
    _MLOAD_RETURN(pInstance);
    return eReturn;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Vtrack_Enable(MS_U8 u8FrameRate, MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_VTRACK_ENABLE XCArgs;
    XCArgs.u8FrameRate = u8FrameRate;
    XCArgs.bEnable = bEnable;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_VTRACK_ENABLE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Pre-set the specific window with PQ info
/// @param  pstXC_SetWin_Info      \b IN: the information of the window setting
/// @param  eWindow                \b IN: which window we are going to set
//-------------------------------------------------------------------------------------------------
void MApi_XC_PreSetPQInfo_U2(void* pInstance, XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    MS_U32 u32DisPOddCheck = 0x00;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    MDrv_SC_SetPQHSDFlag(pInstance, FALSE); //Reset variable

    if(pstXC_SetWin_Info->bInterlace)
    {
        //The component 1080i can't frame lock when capture start is odd.
#ifdef K3_U2
        if(!IsSrcTypeDTV( pstXC_SetWin_Info->enInputSourceType ))
#endif
        {
            // the Capture start should be even
            if(pstXC_SetWin_Info->stCapWin.y%2)
            {
                pstXC_SetWin_Info->stCapWin.y += 1;
                pstXC_SetWin_Info->stCapWin.height -= 1; //Y+1, so height-1 to keep not capture outside
            }
        }

        // the Capture height should be even
        if(pstXC_SetWin_Info->stCapWin.height%2)
        {
            pstXC_SetWin_Info->stCapWin.height -= 1;
        }
    }

    // copy user data to internal data
    // here is a patch first, will remove after seperate Internal XC status with set window information
    //u32CopyLen = sizeof(XC_SETWIN_INFO) - (sizeof(MS_BOOL)*2) - (sizeof(MS_U16)*4);
    //memcpy(&gSrcInfo[eWindow], pstXC_SetWin_Info, u32CopyLen);
    //do not use memcopy(), strongly depend on structure definition
    _Mdrv_XC_CopySetWinInfo(pInstance, pstXC_SetWin_Info, eWindow);

    // The total pixel conut of display window can not be odd.
    u32DisPOddCheck = gSrcInfo[eWindow].stDispWin.width * gSrcInfo[eWindow].stDispWin.height;
    // if u32DisPOddCheck is odd, that means both of disp.width and disp.height is odd.
    // we adjust width to even.
    if (u32DisPOddCheck & 0x01)
    {
        if (pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width > gSrcInfo[eWindow].stDispWin.width )
        {
            gSrcInfo[eWindow].stDispWin.width = ( gSrcInfo[eWindow].stDispWin.width + 1 ) & ~0x1;
        }
    }

    // if mirror, need to re-cal display window but Ninelattice!!
    if((IsVMirrorMode(eWindow) || IsHMirrorMode(eWindow)) &&(!gSrcInfo[eWindow].bDisplayNineLattice))
    {
        MS_U32 u32TempWidth = gSrcInfo[eWindow].stDispWin.width;
        MS_U32 u32TempHeight = gSrcInfo[eWindow].stDispWin.height;

        if(IsHMirrorMode(eWindow))
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width > ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth ) )
            {
                gSrcInfo[eWindow].stDispWin.x =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width - ( gSrcInfo[eWindow].stDispWin.x + u32TempWidth );
            }
            else
            {
                gSrcInfo[eWindow].stDispWin.x = 0;
            }
        }

        if(IsVMirrorMode(eWindow))
        {
            if( pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height > ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight ) )
            {
                gSrcInfo[eWindow].stDispWin.y =
                    pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height - ( gSrcInfo[eWindow].stDispWin.y + u32TempHeight );
            }
            else
            {
                gSrcInfo[eWindow].stDispWin.y = 0;
            }
        }
    }

    MDrv_SC_3D_FormatValidation(pInstance, &gSrcInfo[eWindow], eWindow);
    MDrv_SC_3D_AdjustHShift(pInstance, &gSrcInfo[eWindow], eWindow);

    gSrcInfo[eWindow].stDispWin.x += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart;
    gSrcInfo[eWindow].stDispWin.y += pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart;

    // dump debug msg
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= SetWindow Start (Window : %u, src: %u) =============\n",eWindow, gSrcInfo[eWindow].enInputSourceType);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CapWin  x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCapWin.x, gSrcInfo[eWindow].stCapWin.y, gSrcInfo[eWindow].stCapWin.width, gSrcInfo[eWindow].stCapWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"CropWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stCropWin.x, gSrcInfo[eWindow].stCropWin.y, gSrcInfo[eWindow].stCropWin.width, gSrcInfo[eWindow].stCropWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"DispWin x: %4u  y: %4u  w: %4u  h: %4u \n",gSrcInfo[eWindow].stDispWin.x,gSrcInfo[eWindow].stDispWin.y, gSrcInfo[eWindow].stDispWin.width, gSrcInfo[eWindow].stDispWin.height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Panel   x: %4u  y: %4u  w: %4u  h: %4u \n", pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16HStart
                                                                                  , pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16VStart
                                                                                  , pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width
                                                                                  , pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Mirror/Interlace/Hdup = %u/%u/%u\n", IsVMirrorMode(eWindow), pstXC_SetWin_Info->bInterlace, pstXC_SetWin_Info->bHDuplicate);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"H/V total = (%u, %u), VFreq = %u\n", pstXC_SetWin_Info->u16DefaultHtotal, pstXC_SetWin_Info->u16InputVTotal, pstXC_SetWin_Info->u16InputVFreq);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Pre  H cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].Status2.bPreHCusScaling, gSrcInfo[eWindow].Status2.u16PreHCusScalingSrc, gSrcInfo[eWindow].Status2.u16PreHCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Pre  V cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].Status2.bPreVCusScaling, gSrcInfo[eWindow].Status2.u16PreVCusScalingSrc, gSrcInfo[eWindow].Status2.u16PreVCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Post H cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].bHCusScaling, gSrcInfo[eWindow].u16HCusScalingSrc, gSrcInfo[eWindow].u16HCusScalingDst);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"Post V cus scaling %u (%u -> %u)\n", gSrcInfo[eWindow].bVCusScaling, gSrcInfo[eWindow].u16VCusScalingSrc, gSrcInfo[eWindow].u16VCusScalingDst);
    memcpy(&gSrcInfo[eWindow].stStatusnodelay.stDispWin, &gSrcInfo[eWindow].stDispWin, sizeof(MS_WINDOW_TYPE));
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    // Scaler must update source before width / height checking.
    _MDrv_XC_Set_PQ_SourceData(pInstance, eWindow, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow]);
    msAPI_Scaler_SetMode(pInstance, gSrcInfo[eWindow].enInputSourceType, &gSrcInfo[eWindow], eWindow);

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
void MApi_XC_PreSetPQInfo(XC_SETWIN_INFO *pstXC_SetWin_Info, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return;
    }

    stXC_PRESET_PQINFO XCArgs;
    XCArgs.pstXC_SetWin_Info = pstXC_SetWin_Info;
    XCArgs.eWindow = eWindow;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_PRESET_PQINFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return;
    }
    else
    {
        return;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// Mapi_XC_Is_OP1_TestPattern_Enabled
/// @return MS_BOOL: TRUE - enabled, FALSED - disabled
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_Is_OP1_TestPattern_Enabled_U2(void* pInstance)
{
#ifndef DISABLE_HW_PATTERN_FUNCTION
    #ifdef SUPPORT_HW_TESTPATTERN
        XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
        UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
        XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
        MS_BOOL bReturn = FALSE;
        _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
        UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
        bReturn = pXCResourcePrivate->stdrvXC_MVideo.bIsHWPatternEnabled;
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return bReturn;
    #else
        return FALSE;
    #endif
#else
    return FALSE;
#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Is_OP1_TestPattern_Enabled(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_GET_OP1_TESTPATTERN_ENABLED XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_GET_OP1_TESTPATTERN_ENABLED, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_Set_OP1_TestPattern
/// @param  ePattern                                      \b IN: select test pattern
/// @param  eMode                                         \b IN: choose the patgen mode
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL or E_APIXC_RET_FAIL_FUN_NOT_SUPPORT
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Set_OP1_TestPattern_U2(void* pInstance, EN_XC_OP1_PATTERN ePattern, EN_XC_OP1_PATGEN_MODE eMode)
{
    E_APIXC_ReturnValue enResult = E_APIXC_RET_OK;

#ifndef DISABLE_HW_PATTERN_FUNCTION

#ifdef SUPPORT_HW_TESTPATTERN
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    _XC_ENTRY(pInstance);
    if (FALSE == pXCResourcePrivate->stdrvXC_MVideo.bIsHWPatternEnabled)
    {
        pXCResourcePrivate->stdrvXC_MVideo.bIsInputSrcDisabled = Hal_SC_Is_InputSource_Disable(pInstance, MAIN_WINDOW) >> 7;
        pXCResourcePrivate->stdrvXC_MVideo.bIsMute = SC_R2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_19_L, BIT(1)) >> 1;
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (IsVBY1_16LANE((MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type))
        {
            pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCMute = SC_R2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BKCB_17_L, BIT(1)) >> 1;
        }
#endif
    }

    if (E_XC_OP1_PATTERN_OFF == ePattern)
    {
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_19_L, BIT(1),BIT(1)); //window color en : on
#if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (IsVBY1_16LANE((MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type))
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BKCB_17_L, BIT(1),BIT(1));
        }
#endif

        MDrv_SC_OP1_PatGen_Disable(pInstance);

        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK10_19_L, (pXCResourcePrivate->stdrvXC_MVideo.bIsMute << 1),BIT(1));
  #if (FRC_INSIDE && (HW_DESIGN_4K2K_VER == 6))
        if (IsVBY1_16LANE((MS_U8)pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.eLPLL_Type))
        {
            SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BKCB_17_L, (pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCMute << 1),BIT(1));
        }
#endif
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK01_02_L, (pXCResourcePrivate->stdrvXC_MVideo.bIsInputSrcDisabled << 7),BIT(7));

        pXCResourcePrivate->stdrvXC_MVideo.bIsHWPatternEnabled = FALSE;
    }
    else
    {
        MDrv_SC_Set_OP1_TestPattern(pInstance, ePattern, eMode);
        pXCResourcePrivate->stdrvXC_MVideo.bIsHWPatternEnabled = TRUE;
    }
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
#else
    enResult = E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
#endif

#else
    enResult = E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
#endif


    return enResult;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Set_OP1_TestPattern(EN_XC_OP1_PATTERN ePattern, EN_XC_OP1_PATGEN_MODE eMode)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OP1_TESTPATTERN XCArgs;
    XCArgs.ePattern = ePattern;
    XCArgs.eMode = eMode;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_OP1_TESTPATTERN, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
/// MApi_XC_Set_OP2_Pattern
/// @param  bEnable               \b IN: enable/disable OP2  pattern
/// @param  u16R_Data             \b IN: when enable OP2  patter ,Data Generator for r_cr constant
/// @param  u16G_Data             \b IN: when enable OP2  patter ,Data Generator for g_y constant
/// @param  u16B_Data             \b IN: when enable OP2  patter ,Data Generator for b_cb constant
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL or E_APIXC_RET_FAIL_FUN_NOT_SUPPORT
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_Set_OP2_Pattern_U2(void* pInstance, MS_BOOL bEnable, MS_U16 u16R_Data, MS_U16 u16G_Data ,MS_U16 u16B_Data)
{
    E_APIXC_ReturnValue enResult = E_APIXC_RET_OK;
    #if (SUPPORT_OP2_TEST_PATTERN >0)
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    _XC_ENTRY(pInstance);
    Hal_SC_vop2_Set_TestPattern(pInstance, bEnable,u16R_Data,u16G_Data,u16B_Data);
    _XC_RETURN(pInstance);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Eixt\n");
    #endif
    return enResult;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Set_OP2_Pattern(MS_BOOL bEnable, MS_U16 u16R_Data, MS_U16 u16G_Data ,MS_U16 u16B_Data)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_OP2_TESTPATTERN  XCArgs;
    XCArgs.bEnable = bEnable ;
    XCArgs.u16R_Data = u16R_Data ;
    XCArgs.u16G_Data = u16G_Data;
    XCArgs.u16B_Data = u16B_Data;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_OP2_TESTPATTERN, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
//-------------------------------------------------------------------------------------------------
/// MApi_XC_CheckWhiteBalancePatternModeSupport
/// @param  eWBPatternMode   \b IN: White balance Pattern gen Mode
/// @return MS_BOOL: TRUE - support, FALSED - unsupport
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_CheckWhiteBalancePatternModeSupport_U2(void* pInstance, EN_XC_WB_PATTERN_MODE enWBPatternMode)
{
#ifndef DISABLE_HW_PATTERN_FUNCTION
    #ifdef SUPPORT_HW_TESTPATTERN
    if(enWBPatternMode == E_XC_OP1_PATTERN_MODE)
    {
        return TRUE;
    }
    else
    #endif
    if(SUPPORT_OP2_TEST_PATTERN)
    {
        if(enWBPatternMode == E_XC_OP2_PATTERN_MODE)
        {
            return TRUE;
        }
    }
    return FALSE;
#else
    if(SUPPORT_OP2_TEST_PATTERN)
    {
        if(enWBPatternMode == E_XC_OP2_PATTERN_MODE)
        {
            return TRUE;
        }
    }
    return FALSE;
 #endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_CheckWhiteBalancePatternModeSupport(EN_XC_WB_PATTERN_MODE enWBPatternMode)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_CHECK_WB_TESTPATTERN  XCArgs;
    XCArgs.eWBPattern = enWBPatternMode;
    XCArgs.eReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CHECK_WHITEBALANCE_PATTERN_MODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_SetHLinearScaling_U2(void* pInstance, MS_BOOL bEnable, MS_BOOL bSign, MS_U16 u16Delta, SCALER_WIN eWindow)
{
#if XC_HLINEARSCALING_VER > 0
    MS_U16 u16DeltaInit = 0;

    _XC_ENTRY(pInstance);

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    if(bEnable)
    {
        if(0 == SC_R2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_08_L, BIT(8)))
        {
            printf("Waring: there is no H_postscaling enabled, the HLinearScaling won't work!\n");
        }

        u16DeltaInit = (960 * u16Delta) / 512;

        if((u16DeltaInit < 2) || (u16Delta > 0x7F) || (eWindow != MAIN_WINDOW))
        {
            _XC_RETURN(pInstance);
            XC_LOG_TRACE(XC_DGBLEVEL_CROPCALC,"MApi_XC_SetHLinearScaling(): u16Delta=%u, u16DeltaInit=%u, eWindow=%u\n", u16Delta, u16DeltaInit, eWindow);
            return E_APIXC_RET_FAIL_INVALID_PARAMETER;
        }

        XC_LOG_TRACE(XC_DGBLEVEL_CROPCALC,"MApi_XC_SetHLinearScaling(): u16DeltaInit=%u\n", u16DeltaInit);

        //enable
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, BIT(6), BIT(6));

        //zone 0 width: 0x3C0
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, 1<<12, BIT(12));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_14_L, 0xE0, 0x00FF);
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, 1<<8, BIT(8));

        //zone 1 width: 0x3C0
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, 1<<13, BIT(13));
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_14_L, 0xE0<<8, 0xFF00);
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, 1<<9, BIT(9));

        //zone 0 1 Delta
        SC_W2BYTE(psXCInstPri->u32DeviceID, REG_SC_BK23_16_L, (u16Delta<<8) | (u16Delta));

        //zone 0 1 Delta init
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_15_L, u16DeltaInit<<8, 0x7F00);

        //Delta init Signed
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_15_L, bSign<<15, BIT(15));
    }
    else
    {
        SC_W2BYTEMSK(psXCInstPri->u32DeviceID, REG_SC_BK23_13_L, 0, BIT(6));
    }

    _XC_RETURN(pInstance);

    return E_APIXC_RET_OK;
#else
    return E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetHLinearScaling(MS_BOOL bEnable, MS_BOOL bSign, MS_U16 u16Delta, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_HLINEARSCALING XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.bSign = bSign;
    XCArgs.u16Delta = u16Delta;
    XCArgs.eWindow = eWindow;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_HLINEARSCALING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_EnableT3D_U2(void* pInstance, MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
#ifdef ENABLE_CHIP_BRINGUP
    bEnable = FALSE;
#endif
    _XC_ENTRY(pInstance);
    bRet = MDrv_XC_EnableT3D(pInstance, bEnable);
    _XC_RETURN(pInstance);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_EnableT3D(MS_BOOL bEnable)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_EnableT3D XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_ENABLE_T3D, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_Set_FRC_InputTiming_U2(void* pInstance, E_XC_FRC_InputTiming enFRC_InputTiming)
{
#if FRC_INSIDE
    MS_BOOL bRet = FALSE;

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bRet = MDrv_XC_Set_FRC_InputTiming(pInstance, enFRC_InputTiming);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
#else
    return E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Set_FRC_InputTiming(E_XC_FRC_InputTiming enFRC_InputTiming)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Set_FRC_InputTiming XCArgs;
    XCArgs.enFRC_InputTiming = enFRC_InputTiming;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_FRC_INPUTTIMING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_Get_FRC_InputTiming_U2(void* pInstance, E_XC_FRC_InputTiming *penFRC_InputTiming)
{
#if FRC_INSIDE
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(penFRC_InputTiming)
    {
        *penFRC_InputTiming = pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming;
        bRet = TRUE;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
#else
    return E_APIXC_RET_FAIL_FUN_NOT_SUPPORT;
#endif
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Get_FRC_InputTiming(E_XC_FRC_InputTiming *penFRC_InputTiming)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Get_FRC_InputTiming XCArgs;
    XCArgs.penFRC_InputTiming = penFRC_InputTiming;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_GET_FRC_INPUTTIMING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#if 0
E_APIXC_ReturnValue MApi_XC_Get_VirtualBox_Info_U2(void* pInstance, XC_VBOX_INFO *pstXC_VboxInfo)
{
    MS_BOOL bRet = FALSE;
    MS_U8 u8FrameCnt = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if(pstXC_VboxInfo)
    {
        pstXC_VboxInfo->u32FBSize[0] = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size;
        pstXC_VboxInfo->u32FBSize[1] = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size;
        pstXC_VboxInfo->u16VBox_PanelHsize = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Width;
        pstXC_VboxInfo->u16VBox_PanelVsize = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.stPanelInfo.u16Height;

        // temp for sub
        pstXC_VboxInfo->u16VBox_Htotal[1] = 1920;
        pstXC_VboxInfo->u16VBox_Vtotal[1] = 1080;

        if(XC_SUPPORT_4K2K)
        {
            // in 4K2K case, the Madi mode is P_Mode10 = 20bit/pixels
            u8FrameCnt = (pstXC_VboxInfo->u32FBSize[0]) / (pstXC_VboxInfo->u16VBox_PanelHsize*pstXC_VboxInfo->u16VBox_PanelVsize * 20/8);
            if(u8FrameCnt >= 2)
            {
        #if FRC_INSIDE
                /// FRC Enable
                if(pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited)
                {
                    if(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_4K2K)
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                    }
                    else //if(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_2K1K)) and others
                    {
                        // 2K2K only for Napoli with FRC_On
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080*2;
                    }
                }
                else  /// FRC Disable
                {
                    if((pstXC_VboxInfo->u16Vfreq[0] <= 300)&&(pstXC_VboxInfo->bInterface[0] == FALSE))
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                    }
                    else
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                    }
                }
            #else
                if(0)//(pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_Init_Misc.u32MISC_A & E_XC_INIT_MISC_A_MONACO_MODE)
                {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                }
                else // exp : Nike, Monaco_legacy_mode
                {
                    if((pstXC_VboxInfo->u16Vfreq[0] <= 300)&&(pstXC_VboxInfo->bInterface[0] == FALSE))
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 3840;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 2160;
                    }
                    else
                    {
                        pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                        pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                    }
                }
        #endif
            }
            else
            {
        #if FRC_INSIDE
                if(pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited
                 &&(pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming == E_XC_FRC_InputTiming_2K1K))
                {
                    // 2K2K only for Napoli with FRC_On
                    pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                    pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080*2;
                }
                else
        #endif
                {
                    pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
                    pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
                }
            }
        }
        else
        {
            pstXC_VboxInfo->u16VBox_Htotal[0] = 1920;
            pstXC_VboxInfo->u16VBox_Vtotal[0] = 1080;
        }
        bRet = TRUE;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}

E_APIXC_ReturnValue MApi_XC_Get_VirtualBox_Info(XC_VBOX_INFO *pstXC_VboxInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Get_VBOX_Info XCArgs;
    XCArgs.pstXC_VboxInfo = pstXC_VboxInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_VBOX_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_Set_OSD_Detect_U2(void* pInstance,MS_BOOL bEnable, MS_U32 Threhold)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bRet = Hal_SC_SetOSDDetect(pInstance, bEnable,Threhold);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Set_OSD_Detect(MS_BOOL bEnable, MS_U32 Threhold)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Set_OSD_DETECT XCArgs;
    XCArgs.bEnable = bEnable;
    XCArgs.Threhold = Threhold;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_SET_OSD_DETECT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_Get_OSD_Detect_U2(void* pInstance,MS_BOOL *pbOSD)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bRet = Hal_SC_GetOSDDetect(pInstance, pbOSD);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    if (bRet)
    {
        return E_APIXC_RET_OK;
    }
    else
    {
        return E_APIXC_RET_FAIL;
    }
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_Get_OSD_Detect(MS_BOOL *pbOSD)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Get_OSD_DETECT XCArgs;
    XCArgs.pbOSD = pbOSD;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_GET_OSD_DETECT, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
MS_BOOL MApi_XC_Is2K2KToFrcMode_U2(void* pInstance)
{
    MS_BOOL bRet = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bRet = MDrv_XC_Is2K2KToFrc(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Is2K2KToFrcMode(void)
{
    //return MDrv_XC_Is2K2KToFrc();
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Is2K2KToFrcMode  XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_IS2K2KTOFRCMODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
MS_BOOL MApi_XC_Set_XC_VOP_U2(void * pInstance, MS_XC_VOP_Data stXCVOPData)
{

    MS_BOOL bResult = TRUE;

    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    printf("[%s][%d]pPNLInstancePrivate->u32DeviceID= %tx\n", __FUNCTION__, __LINE__, (ptrdiff_t)psXCInstPri->u32DeviceID);
    if(psXCInstPri->u32DeviceID == 0)
    {
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return FALSE;
    }

    MDrv_XC_Set_VOP(pInstance, &stXCVOPData);

    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);

    return bResult;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_Set_XC_VOP(MS_XC_VOP_Data stXCVOPData)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SetXC_VOP XCArgs;
    XCArgs.stXC_VOPData = stXCVOPData;
    XCArgs.bReturn = FALSE;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_XC_VOP, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain PNL engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturn;
    }

}
#endif
//K3_U2
#if 0
//-------------------------------------------------------------------------------------------------
/// MApi_XC_EX_Set_WhiteBalance_Pattern
/// @param  bEnable                  \b IN: enable/disable white balance test pattern
/// @param  u16R_CR_Data             \b IN: when enable white balance test pattern ,Data Generator for r_cr constant
/// @param  u16G_Y_Data              \b IN: when enable white balance test pattern ,Data Generator for g_y constant
/// @param  u16B_CB_Data             \b IN: when enable white balance test pattern ,Data Generator for b_cb constant
/// @return E_APIXC_RET_OK or E_APIXC_RET_FAIL or E_APIXC_RET_FAIL_FUN_NOT_SUPPORT
//-------------------------------------------------------------------------------------------------
E_APIXC_ReturnValue MApi_XC_EX_Set_WhiteBalance_Pattern_U2(XC_DEVICE_ID *pDeviceId, MS_BOOL bEnable, MS_U16 u16R_CR_Data, MS_U16 u16G_Y_Data ,MS_U16 u16B_CB_Data)
{
    XC_EX_RETURN_VALUE enResult = E_XC_EX_RET_OK;

    W2BYTEMSK(REG_IPMUX_10_L, (bEnable << 9) , BIT(9));
    W2BYTEMSK(REG_IPMUX_10_L, (bEnable << 12), BIT(12));
    W2BYTEMSK(REG_IPMUX_10_L, (bEnable << 13), BIT(13));
    W2BYTEMSK(REG_IPMUX_10_L, (bEnable << 14), BIT(14));

    if(bEnable == TRUE)
    {
        W2BYTEMSK(REG_IPMUX_1D_L, u16R_CR_Data, BMASK(9:0));
        W2BYTEMSK(REG_IPMUX_1E_L, u16G_Y_Data,  BMASK(9:0));
        W2BYTEMSK(REG_IPMUX_1F_L, u16B_CB_Data, BMASK(9:0));
    }

    return enResult;
}

E_APIXC_ReturnValue MApi_XC_Get_FRC_InputTiming(E_XC_FRC_InputTiming *penFRC_InputTiming)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_Get_FRC_InputTiming XCArgs;
    XCArgs.penFRC_InputTiming = penFRC_InputTiming;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_GET_FRC_INPUTTIMING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
/// Get SWDSIndex
/// @return @ref MS_U8 return SWDSIndex
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_XC_GetSWDSIndex_U2(void* pInstance, SCALER_WIN eWindow)
{
    MS_U8 u8SWDSIndex;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if(MDrv_XC_Is_SupportSWDS(pInstance, eWindow))
    {
#if (SUPPORT_KERNEL_DS == 1)
        u8SWDSIndex = MDrv_XC_GetSWDSIndex(pInstance, eWindow);
#else
        u8SWDSIndex = pXCResourcePrivate->sthal_SC.u8DynamicScalingNextIndex[eWindow];
#endif
    }
    else
    {
        u8SWDSIndex = 0;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return u8SWDSIndex;

}
#ifndef UTOPIAXP_REMOVE_WRAPPER
#ifdef MSOS_TYPE_LINUX_KERNEL
MS_U8 MApi_XC_GetSWDSIndex(SCALER_WIN eWindow)
#else
__attribute__((weak)) MS_U8 MApi_XC_GetSWDSIndex(SCALER_WIN eWindow)
#endif
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return 0;
    }

    stXC_GET_SWDSIndex XCArgs;
    XCArgs.u8ReturnValue = 0;
    XCArgs.eWindow = eWindow;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_SWDSIndex, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)  //E_XC_CMD_GET_SWDSIndex
    {
        printf("Obtain XC engine fail\n");
        return 0;
    }
    else
    {
        return XCArgs.u8ReturnValue;
    }

}
#endif
//-------------------------------------------------------------------------------------------------
/// Set DNR base0 address
/// @param  u32FBAddress      \b IN: the framebuffer address
/// @param  u32FBSize         \b IN: the framebuffer size
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_SetDualFrameBufferAddress_U2(void* pInstance, MS_PHY u32FBAddress, MS_PHY u32FBSize, SCALER_WIN eWindow)
{
    MS_BOOL bReturn = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    bReturn = MDrv_XC_SetDualFrameBufferAddress(pInstance, u32FBAddress, u32FBSize, eWindow);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bReturn;
}

#ifdef UFO_XC_SUPPORT_DUAL_MIU
MS_BOOL MApi_XC_EnableMiuDualMode_U2(void* pInstance, MS_BOOL bEnable, MS_U32 u32LeftFactor, MS_U32 u32RightFactor, SCALER_WIN eWindow)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
#ifndef XC_DUAL_MIU_SUPPORT_SUBWIN
    if (eWindow == MAIN_WINDOW)
    {
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
        if (psXCInstPri->u32DeviceID != 0)
        {
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "XC SCMI Dual miu just support main window only.\n");
            return FALSE;
        }
#endif
    }
    else
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "XC SCMI Dual miu just support main window only.\n");
        return FALSE;
    }
#endif

    if(!((u32RightFactor <= 3*u32LeftFactor) && (u32LeftFactor <= 3*u32RightFactor)))
    {
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "error params u32LeftFactor: %d, u32RightFactor: %d.\n", u32LeftFactor, u32RightFactor);
        return FALSE;
    }

    gSrcInfo[eWindow].Status2.stMiuDualModCfg.bEnableMiuDualMod = bEnable;
    gSrcInfo[eWindow].Status2.stMiuDualModCfg.u32LeftFactor = u32LeftFactor;
    gSrcInfo[eWindow].Status2.stMiuDualModCfg.u32RightFactor = u32RightFactor;
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW, "u32LeftFactor: %d, u32RightFactor: %d.\n", u32LeftFactor, u32RightFactor);
    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL  MApi_XC_EnableMiuDualMode(MS_BOOL bEnable, MS_U32 u32LeftFactor, MS_U32 u32RightFactor, SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_SetBwdConfig() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_EnableDualMode XCArgs;
    XCArgs.bEnable       = bEnable;
    XCArgs.u32LeftFactor = u32LeftFactor;
    XCArgs.u32RightFactor= u32RightFactor;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_ENABLE_DUAL_MODE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
#endif

#ifdef SUPPORT_BWD
MS_BOOL MApi_XC_SetBwdConfig_U2(void* pInstance, E_XC_BWD_CONFIG_TYPE eType, void *pstParam, SCALER_WIN eWindow )
{
    MS_BOOL bResult = FALSE;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    bResult = MDrv_SC_setBwdConfig(pInstance, eType, pstParam, eWindow);
      _XC_RETURN(pInstance);
      _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return bResult;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetBwdConfig(E_XC_BWD_CONFIG_TYPE eType, void *pstParam, SCALER_WIN eWindow )
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_SetBwdConfig() first\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    stXC_SetBwdConfig XCArgs;
    XCArgs.eType = eType;
    XCArgs.pstParam = pstParam;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_BWD_CONFIG, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
#endif
MS_BOOL MApi_XC_IsSupport2StepScaling_U2(void* pInstance)
{
    return MDrv_XC_IsSupport2StepScaling();
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_IsSupport2StepScaling(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_IsSupport2StepScaling XCArgs;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_IS_SUPPORT_2_STEP_SCALING, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif


E_APIXC_ReturnValue MApi_XC_SetForceWrite_U2( void* pInstance, EN_MUTE_TYPE enMuteType, MS_BOOL bIsForceWrite)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
   _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_SC_SetForceWrite( enMuteType, bIsForceWrite);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return E_APIXC_RET_OK;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue MApi_XC_SetForceWrite( EN_MUTE_TYPE enMuteType, MS_BOOL bIsForceWrite)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_SET_FORCE_WRITE XCArgs;
    XCArgs.enMuteType = enMuteType;
    XCArgs.bIsForceWrite = bIsForceWrite;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_FORCE_WRITE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");

        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }

}
#endif

MS_BOOL MApi_XC_GetForceWrite_U2( void* pInstance, EN_MUTE_TYPE enMuteType)
{
    MS_BOOL bret = FALSE;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
   _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    bret = MDrv_SC_GetForceWrite( enMuteType);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_GetForceWrite( EN_MUTE_TYPE enMuteType)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_FORCE_WRITE XCArgs;
    XCArgs.enMuteType = enMuteType;
    XCArgs.eReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_FORCE_WRITE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        printf("G eReturnValue = %d\n",(XCArgs.eReturnValue == TRUE));
        return XCArgs.eReturnValue;
    }

}
#endif
E_APIXC_ReturnValue MApi_XC_VideoPlaybackCtrl_U2(void* pInstance,MS_U32 u32Cmd,void *pCmdArgs,MS_U32 u32CmdArgsSize)
{
    MS_BOOL bret = E_APIXC_RET_FAIL;
    switch(u32Cmd)
    {
        case E_XC_VPC_MULTI_SPEED_PLAYBACK_CTRL:
            if(u32CmdArgsSize == sizeof(ST_XC_VPC_MULTI_SPEED_PLAYBACK_CTRL))
            {
                ST_XC_VPC_MULTI_SPEED_PLAYBACK_CTRL *stXCVpcMultiSpeedPlaybackCtrl = (ST_XC_VPC_MULTI_SPEED_PLAYBACK_CTRL *)pCmdArgs;

                _XC_ENTRY(pInstance);
                MDrv_XC_SetForceCurrentReadBank(pInstance, stXCVpcMultiSpeedPlaybackCtrl ->bEnable, stXCVpcMultiSpeedPlaybackCtrl ->eWindow);
                _XC_RETURN(pInstance);
                bret = E_APIXC_RET_OK;
            }
            else
            {
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"E_XC_VPC_MULTI_SPEED_PLAYBACK_I_Mode Struct Size No Match\n");
                return bret;
            }
            break;

        default:
            printf("Unknow Handle ID!\n");
            break;
    }

    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue SYMBOL_WEAK MApi_XC_VideoPlaybackCtrl(MS_U32 u32Cmd,void *pCmdArgs,MS_U32 u32CmdArgsSize)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_VIDEO_PLAYBACK_CTRL XCArgs;
    XCArgs.u32Cmd = u32Cmd;
    XCArgs.pCmdArgs = pCmdArgs;
    XCArgs.u32CmdArgsSize = u32CmdArgsSize;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_VIDEO_PLAYBACK_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain PNL engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#ifdef UFO_XC_AUTO_DOWNLOAD
E_APIXC_ReturnValue MApi_XC_AutoDownload_Config_U2(void* pInstance, XC_AUTODOWNLOAD_CONFIG_INFO *pstConfigInfo)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_AUTODOWNLOAD_CONFIG_INFO stConfigInfo;
    MS_U16 u16CopiedLength = sizeof(stConfigInfo);

    memset(&stConfigInfo, 0, sizeof(stConfigInfo));
    VERSION_COMPATIBLE_CHECK(pstConfigInfo, stConfigInfo, u16CopiedLength, u32ConfigInfo_Version, u16ConfigInfo_Length, AUTODOWNLOAD_CONFIG_INFO_VERSION);
    pstConfigInfo->u32ConfigInfo_Version = AUTODOWNLOAD_CONFIG_INFO_VERSION; /*Return the actual version of the returned value for APP to use*/
    pstConfigInfo->u16ConfigInfo_Length = u16CopiedLength; /*Return the actual length of the returned value for APP to know whick info is valid*/
    memcpy(&stConfigInfo, pstConfigInfo, u16CopiedLength);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    eRet = MDrv_XC_AutoDownload_Config(pInstance, &stConfigInfo);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue SYMBOL_WEAK MApi_XC_AutoDownload_Config(XC_AUTODOWNLOAD_CONFIG_INFO *pstConfigInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_AUTODOWNLOAD_CONFIG_CTRL XCArgs;
    XCArgs.pstConfigInfo = pstConfigInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_AUTODOWNLOAD_CONFIG_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_AutoDownload_Write_U2(void* pInstance, XC_AUTODOWNLOAD_DATA_INFO *pstDataInfo)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_AUTODOWNLOAD_DATA_INFO stDataInfo;
    MS_U16 u16CopiedLength = sizeof(stDataInfo);
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;

#if (defined(CONFIG_PURE_SN))
    XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Write auto download(client: %d, size: %ld).\n",
        pstDataInfo->enClient, pstDataInfo->u32Size);
#else
    XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Write auto download(client: %d, size: %d).\n",
        pstDataInfo->enClient, pstDataInfo->u32Size);
#endif

    XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Write auto download(client: %d).\n", pstDataInfo->enClient);

    memset(&stDataInfo, 0, sizeof(stDataInfo));
    VERSION_COMPATIBLE_CHECK(pstDataInfo, stDataInfo, u16CopiedLength, u32DataInfo_Version, u16DataInfo_Length, AUTODOWNLOAD_DATA_INFO_VERSION);
    pstDataInfo->u32DataInfo_Version = AUTODOWNLOAD_DATA_INFO_VERSION; /*Return the actual version of the returned value for APP to use*/
    pstDataInfo->u16DataInfo_Length = u16CopiedLength; /*Return the actual length of the returned value for APP to know whick info is valid*/
    memcpy(&stDataInfo, pstDataInfo, u16CopiedLength);

    XC_AUTODOWNLOAD_FORMAT_INFO stFormatInfo;
    u16CopiedLength = sizeof(stFormatInfo);
    memset(&stFormatInfo, 0, sizeof(stFormatInfo));
    if (stDataInfo.enClient == E_XC_AUTODOWNLOAD_CLIENT_HDR)
    {
        XC_AUTODOWNLOAD_FORMAT_INFO* pstFormatInfo = (XC_AUTODOWNLOAD_FORMAT_INFO *)stDataInfo.pParam;
#if (defined(CONFIG_PURE_SN))
        XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Write auto download, Size: %ld, SubClient: %d, AddrRange(enable, start, end) = (%d, %td, %td).\n",
            stDataInfo.u32Size, pstFormatInfo->enSubClient, pstFormatInfo->bEnableRange, (ptrdiff_t)pstFormatInfo->u16StartAddr, (ptrdiff_t)pstFormatInfo->u16EndAddr);
#else
        XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Write auto download, Size: %d, SubClient: %d, AddrRange(enable, start, end) = (%d, %td, %td).\n",
            stDataInfo.u32Size, pstFormatInfo->enSubClient, pstFormatInfo->bEnableRange, (ptrdiff_t)pstFormatInfo->u16StartAddr, (ptrdiff_t)pstFormatInfo->u16EndAddr);
#endif

        VERSION_COMPATIBLE_CHECK(pstFormatInfo, stFormatInfo, u16CopiedLength, u32FormatInfo_Version, u16FormatInfo_Length, AUTODOWNLOAD_FORMAT_INFO_VERSION);
        pstFormatInfo->u32FormatInfo_Version = AUTODOWNLOAD_FORMAT_INFO_VERSION; /*Return the actual version of the returned value for APP to use*/
        pstFormatInfo->u16FormatInfo_Length = u16CopiedLength; /*Return the actual length of the returned value for APP to know whick info is valid*/
        memcpy(&stFormatInfo, pstFormatInfo, u16CopiedLength);

        stDataInfo.pParam = &stFormatInfo;
    }

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    eRet = MDrv_XC_AutoDownload_Write(pInstance, &stDataInfo);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue SYMBOL_WEAK MApi_XC_AutoDownload_Write(XC_AUTODOWNLOAD_DATA_INFO *pstDataInfo)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }
    stXC_AUTODOWNLOAD_WRITE_CTRL XCArgs;
    XCArgs.pstDataInfo= pstDataInfo;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_AUTODOWNLOAD_WRITE_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
E_APIXC_ReturnValue MApi_XC_AutoDownload_Fire_U2(void* pInstance, EN_XC_AUTODOWNLOAD_CLIENT enClient)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "Fire auto download(client: %d).\n", enClient);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    eRet = MDrv_XC_AutoDownload_Fire(pInstance, enClient);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return eRet;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
E_APIXC_ReturnValue SYMBOL_WEAK MApi_XC_AutoDownload_Fire(EN_XC_AUTODOWNLOAD_CLIENT enClient)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_AUTODOWNLOAD_FIRE_CTRL XCArgs;
    XCArgs.enClient = enClient;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_AUTODOWNLOAD_FIRE_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
E_APIXC_ReturnValue MApi_XC_HDR_Control_U2(void* pInstance, EN_XC_HDR_CTRL_TYPE enCtrlType, void *pParam)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    E_APIXC_ReturnValue eRet = E_APIXC_RET_FAIL;
    XC_LOG_TRACE(XC_DBGLEVEL_PARAMETER_TRACE, "HDR contrl type: %d.\n", enCtrlType);

    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    eRet = MDrv_XC_HDR_Control(pInstance, enCtrlType, pParam);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");

    return eRet;
}

E_APIXC_ReturnValue SYMBOL_WEAK MApi_XC_HDR_Control(EN_XC_HDR_CTRL_TYPE enCtrlType, void *pParam)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
    }

    stXC_HDR_CTRL XCArgs;
    XCArgs.enCtrlType= enCtrlType;
    XCArgs.pParam = pParam;
    XCArgs.eReturnValue = E_APIXC_RET_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_HDR_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return E_APIXC_RET_FAIL;
    }
    else
    {
        return XCArgs.eReturnValue;
    }
}
#endif
#endif

#ifdef UFO_XC_PQ_PATH
MS_BOOL MApi_XC_GetPQPathStatus_U2(void* pInstance, E_XC_PQ_Path_Type ePqPathType,  MS_U16 u16Width, MS_U16 u16Height)
{
    return MDrv_XC_GetPQPathStatus(pInstance, ePqPathType, u16Width, u16Height);
}

MS_BOOL MApi_XC_GetPQPathStatus(E_XC_PQ_Path_Type ePqPathType, MS_U16 u16Width, MS_U16 u16Height)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GetPqPathStatus XCArgs;
    XCArgs.ePqPathType = ePqPathType;
    XCArgs.u16Height = u16Height;
    XCArgs.u16Width = u16Width;
    XCArgs.bReturnValue = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_PQ_PATH, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif

static const char optee_xc[] = "opteearmtz00";
char *_xc_device = (char *)optee_xc;
TEEC_Context mstar_teec_ctx;
////#define MSTAR_TA_NAME       "mstara_internal_tests.ta"
#define MSTAR_INTERNAL_XC_UUID {0x4dd53ca0, 0x0248, 0x11e6, \
      {0x86, 0xc0, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }
//#define SYS_TEEC_OPERATION_INITIALIZER { 0, 0, { { { 0 } } } }
#define SYS_TEEC_OPERATION_INITIALIZER { 0 }
TEEC_Session session = { 0 };
TEEC_UUID uuid = MSTAR_INTERNAL_XC_UUID;
TEEC_Operation op = SYS_TEEC_OPERATION_INITIALIZER;
extern MS_U32 MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context);;
extern MS_U32 MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin);
extern MS_U32 MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin);
extern void MDrv_SYS_TEEC_Close(TEEC_Session *session);
extern void MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context);

MS_BOOL _MDrv_XC_OPTEE_Send_Cmd(void *pInstance,EN_XC_OPTEE_ACTION action,void *para,MS_U32 u32size)
{
    MS_BOOL bret = FALSE;
#ifdef MSOS_TYPE_LINUX
    MS_U32 ret_orig= 0;
    if (MDrv_SYS_TEEC_InitializeContext(_xc_device, &mstar_teec_ctx) != TEEC_SUCCESS)
    {
        printf("%s %d: Init Context failed\n",__func__,__LINE__);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_teec_ctx);
        return bret;
    }

    if (MDrv_SYS_TEEC_Open(&mstar_teec_ctx, &session, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &ret_orig) != TEEC_SUCCESS)
    {
        printf("%s %d: TEEC Open session failed\n",__func__,__LINE__);
        MDrv_SYS_TEEC_Close(&session);
        MDrv_SYS_TEEC_FinalizeContext(&mstar_teec_ctx);
        return bret;
    }
    op.params[0].tmpref.buffer = para;
    op.params[0].tmpref.size = u32size;
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    if (MDrv_SYS_TEEC_InvokeCmd(&session, action, &op, &ret_orig) != TEEC_SUCCESS)
    {
        printf("%s %d: TEEC Invoke command failed\n",__func__,__LINE__);
    }
    else
    {
        bret = TRUE;
    }
    MDrv_SYS_TEEC_Close(&session);
    MDrv_SYS_TEEC_FinalizeContext(&mstar_teec_ctx);
#endif
    return bret;

}
MS_BOOL MDrv_XC_OPTEE_Control(void *pInstance,EN_XC_OPTEE_ACTION action,XC_OPTEE_HANDLER* pstxc_handler)
{
    MS_BOOL bopteeenable = FALSE;
    ///MS_U32 u32tmp_address = 0;
    ///MS_U32 u32offset = 0;
    MS_U32 u32Index = 0;
    MS_U32 u32Cnt=0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
#ifndef MSOS_TYPE_OPTEE
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
    static MS_BOOL bIsGetCMA[CMA_XC_MEM_NUM] = {FALSE,};
#endif
#endif
#endif

#ifndef MSOS_TYPE_OPTEE
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
#else
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
#endif
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    /*
    ** if SC2 - main as SC0 - sub ,take care this as sub , therefore eWindow id has to be take case specilly
    */
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if (psXCInstPri->u32DeviceID == 1 && pstxc_handler->eWindow == MAIN_WINDOW)
    {
        pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].eWindow = SUB_WINDOW; //if PIP_SUB special case
    }else
#endif
    {
        pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].eWindow = pstxc_handler->eWindow; //if PIP_SUB special case
    }

    //Check XC_OPTEE_HANDLER structure size and version
    pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].version = XC_OPTEE_HANDLER_VERSION;
    pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].length = sizeof(XC_OPTEE_HANDLER);
    printf("Size of XC_OPTEE_HANDLER in REE = %u, xc_handler = %u\n", (unsigned int)sizeof(XC_OPTEE_HANDLER), (unsigned int)(pstxc_handler->length));
    printf("Verion in REE = %d, xc_handler =%d\n", XC_OPTEE_HANDLER_VERSION, pstxc_handler->version);
    if((pstxc_handler->length != sizeof(XC_OPTEE_HANDLER)) || (pstxc_handler->version != XC_OPTEE_HANDLER_VERSION))
    {
        printf("REE and xc_handler(input parameter) not match!\n");
    }

    switch (action)
    {
//********relay SN*********//
        case E_XC_OPTEE_GET_PIPE_ID:
            //get pipe in into shm'
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if (psXCInstPri->u32DeviceID == 1 && pstxc_handler->eWindow == MAIN_WINDOW)
            {
                pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].pipeID =pipeID_SC1_main;//?
                pstxc_handler->pipeID = pipeID_SC1_main;
            }else if(psXCInstPri->u32DeviceID == 0)
#endif
            {
                if(pstxc_handler->eWindow == MAIN_WINDOW)
                {
                    pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].pipeID = pipeID_SC0_main;
                    pstxc_handler->pipeID = pipeID_SC0_main;
                }
                else
                {
                    pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].pipeID = pipeID_SC0_sub;
                    pstxc_handler->pipeID = pipeID_SC0_sub;
                }
            }
            break;
        case E_XC_OPTEE_ENABLE:
            //reset forst
            pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].drams_cnt = 0;
            pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].regs_cnt = 0;

            if (pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].isEnable == TRUE)
            {
                bopteeenable = TRUE;
            }
            pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].isEnable = TRUE;

            //on flag , get cma , set_handler
#ifndef MSOS_TYPE_OPTEE
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
            if(psXCInstPri->u32DeviceID == 0 && pstxc_handler->eWindow == MAIN_WINDOW)
            {
                MS_U32 au32CMAMemSCMSize[CMA_XC_MEM_NUM] = {0, };
                MS_U32 au32CMAMemFRCMSize[CMA_XC_MEM_NUM] = {0, };
                MS_U32 u32DualMiuMemSize = 0;
                XC_CMA_CLIENT enXCCMAClient = CMA_XC_MEM_NUM;
                static MS_PHY u32CMAAddr[CMA_XC_MEM_NUM] = {0, };

                //get cma
                MHal_XC_Get_CMA_UsingCondition(pInstance, bIsGetCMA, au32CMAMemSCMSize, au32CMAMemFRCMSize, &u32DualMiuMemSize, CMA_XC_MEM_NUM, pstxc_handler->eWindow);
                if (u32DualMiuMemSize != 0)
                {
                    enXCCMAClient = CMA_XC_SELF_MEM;
                }
                else
                {
                    if (bIsGetCMA[CMA_XC_COBUFF_MEM] == TRUE)
                    {
                        enXCCMAClient = CMA_XC_COBUFF_MEM;
                    }
                    else
                    {
                        enXCCMAClient = CMA_XC_SELF_MEM;
                    }
                }
                if (bopteeenable == FALSE)
                {
                    u32CMAAddr[enXCCMAClient] = MHal_XC_Get_CMA_Addr(pInstance, enXCCMAClient,  au32CMAMemSCMSize[enXCCMAClient]+au32CMAMemFRCMSize[enXCCMAClient], pstxc_handler->eWindow);
                }
                Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,u32CMAAddr[enXCCMAClient], au32CMAMemSCMSize[enXCCMAClient]);
#if (HW_DESIGN_4K2K_VER == 4)
                Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,u32CMAAddr[enXCCMAClient]+au32CMAMemSCMSize[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient]);
#endif
#if (HW_DESIGN_4K2K_VER == 4)
                if(au32CMAMemSCMSize[enXCCMAClient] == 0)
                {
                    // default frcm frame buffer address init
                    MDrv_XC_SetFRCMFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], pstxc_handler->eWindow);
                    // default frame buffer address init
                    MDrv_XC_SetFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], pstxc_handler->eWindow);
                }
                else
#endif
                {
#if (HW_DESIGN_4K2K_VER == 4)
                    // default frcm frame buffer address init
                    MDrv_XC_SetFRCMFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient]+au32CMAMemSCMSize[enXCCMAClient], au32CMAMemFRCMSize[enXCCMAClient], pstxc_handler->eWindow);
#endif
                    // default frame buffer address init
                    MDrv_XC_SetFrameBufferAddress(pInstance, u32CMAAddr[enXCCMAClient], au32CMAMemSCMSize[enXCCMAClient], pstxc_handler->eWindow);
                }
                gSrcInfo[pstxc_handler->eWindow].u32PreCMAMemSCMSize[enXCCMAClient] = au32CMAMemSCMSize[enXCCMAClient];
                gSrcInfo[pstxc_handler->eWindow].u32PreCMAMemFRCMSize[enXCCMAClient] = 0;

                ///u32offset = _XC_Device_Offset[psXCInstPri->u32DeviceID];
                ///u32tmp_address = u32CMAAddr[enXCCMAClient];
            }
            else
#endif
#endif
            {
                pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].isEnable = TRUE;
                if (pstxc_handler->eWindow == MAIN_WINDOW)
                {
                    ///u32tmp_address = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr;
                    Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Start_Addr, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FB_Size);
#if (HW_DESIGN_4K2K_VER == 4)
                    Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Start_Addr, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Main_FRCM_FB_Size);
#endif
                }
                else if (pstxc_handler->eWindow == SUB_WINDOW)
                {
                    ///u32tmp_address = pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr;
                    Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Start_Addr, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FB_Size);
#if (HW_DESIGN_4K2K_VER == 4)
                    Hal_SC_add_dram_to_shm(pInstance,pstxc_handler->eWindow,pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Start_Addr, pXCResourcePrivate->stdrvXC_MVideo_Context.g_XC_InitData.u32Sub_FRCM_FB_Size);
#endif
                }
                ///u32offset = _XC_Device_Offset[psXCInstPri->u32DeviceID];
            }
#endif
            ///u32tmp_address /=BYTE_PER_WORD;
            Hal_SC_update_to_shm(pInstance, pstxc_handler->eWindow);
#if 0
            if (pstxc_handler->eWindow == MAIN_WINDOW)
            {
            Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK0D_38_L,(BIT(0) | BIT(4)), (BIT(0) | BIT(4)));
            Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_08_L, (u32tmp_address & 0xFFFF), 0xFFFF);
            Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_09_L, (u32tmp_address >> 16), 0xFFFF);
            Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_1C_L, 0x0000, 0xFFFF);
            }
            else if (pstxc_handler->eWindow == SUB_WINDOW)
            {
                Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK0D_38_L,(BIT(1) | BIT(6)), (BIT(1) | BIT(6)));
                Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_48_L, (u32tmp_address & 0xFFFF), 0xFFFF);
                Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_49_L, (u32tmp_address >> 16), 0xFFFF);
                Hal_SC_add_reg_to_shm(pInstance,pstxc_handler->eWindow,REG_SC_BK12_5C_L, 0x0000, 0xFFFF);
            }
#endif
            _MDrv_XC_OPTEE_Send_Cmd(pInstance,E_XC_OPTEE_SYNC_HANDLER,&pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow],sizeof(XC_OPTEE_HANDLER));
            break;
        case E_XC_OPTEE_DISABLE:
            //off flag
            bopteeenable = TRUE;
            if (pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].isEnable == FALSE)
            {
                bopteeenable = FALSE;
            }
            pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].isEnable = FALSE;
#ifndef MSOS_TYPE_OPTEE
            if(psXCInstPri->u32DeviceID == 0 && pstxc_handler->eWindow == MAIN_WINDOW)
            {
#ifndef DONT_USE_CMA
#if (XC_SUPPORT_CMA ==TRUE)
                if (bopteeenable == TRUE)
                {
                    if (bIsGetCMA[CMA_XC_COBUFF_MEM] == TRUE)
                    {
                        MHal_XC_Release_CMA(pInstance, CMA_XC_COBUFF_MEM, MAIN_WINDOW);
                    }
                    else if(bIsGetCMA[CMA_XC_SELF_MEM] == TRUE)
                    {
                        MHal_XC_Release_CMA(pInstance, CMA_XC_SELF_MEM, MAIN_WINDOW);
                    }
                }
#endif
#endif
            }
#else
            UNUSED(bopteeenable);
#endif
            break;
//*******pipe only********//
        case E_XC_OPTEE_SET_HANDLER:
            _MDrv_XC_OPTEE_Send_Cmd(pInstance,E_XC_OPTEE_SYNC_HANDLER,&pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow],sizeof(XC_OPTEE_HANDLER));
            break;
        case E_XC_OPTEE_SYNC_HANDLER:
            memcpy(&pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow],pstxc_handler,sizeof(XC_OPTEE_HANDLER));
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if (psXCInstPri->u32DeviceID == 1 && pstxc_handler->eWindow == MAIN_WINDOW)
            {
                pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].eWindow = SUB_WINDOW; //if PIP_SUB special case
            }else
#endif
            {
                pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].eWindow = pstxc_handler->eWindow; //if PIP_SUB special case
            }
            break;
        case E_XC_OPTEE_GET_HANDLER:
            //get from shm
            memcpy(pstxc_handler,&pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow],sizeof(XC_OPTEE_HANDLER));
            break;
        case E_XC_OPTEE_UPDATE_HANDLER:
            //****set seal directly****//
            memcpy(&pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow],pstxc_handler,sizeof(XC_OPTEE_HANDLER));
            u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].regs_cnt;
            for(u32Index = 0 ; u32Index < u32Cnt ; u32Index++)
            {
                        if (u32Index>= OP_TEE_XC_REG_MAX_CNT)
            {
                    printf("array size overflow for xc part");
                    break;
                }
                MDrv_Write2ByteMask(pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].regs_data[u32Index].bk
                                  , pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].regs_data[u32Index].value
                                  , pXCResourcePrivate->sthal_Optee.op_tee_xc[pstxc_handler->eWindow].regs_data[u32Index].mask);
            }
            break;
        default:
            break;
    }
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return TRUE;
}
MS_BOOL MApi_XC_OPTEE_Control_U2(void *pInstance,EN_XC_OPTEE_ACTION action,XC_OPTEE_HANDLER* xc_handler)
{
    return MDrv_XC_OPTEE_Control(pInstance,action,xc_handler);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_OPTEE_Control(EN_XC_OPTEE_ACTION action,XC_OPTEE_HANDLER* xc_handler)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_OPTEE_CTRL XCArgs;
    XCArgs.action = action;
    XCArgs.xc_handler = xc_handler;
    XCArgs.bReturn = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OPTEE_CTL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturn;
    }
}
#endif
MS_BOOL MDrv_XC_OPTEE_Update(void *pInstance,EN_XC_OPTEE_ACTION action,XC_OPTEE_HANDLER xc_handler)
{
    ///MS_U32 u32tmp_address;
    ///MS_U32 u32offset = 0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(action != E_XC_OPTEE_UPDATE_HANDLER)
        return FALSE;
    //reset first
    // only update register setting, dram buffer lock max
    pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].regs_cnt = 0;

   /// u32tmp_address = pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u32DNRBase0*BYTE_PER_WORD;

    pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].drams_data[0].address = pXCResourcePrivate->stdrvXC_MVideo_Context.stDBreg.u32DNRBase0 * BYTE_PER_WORD;
#if 0
    if(psXCInstPri->u32DeviceID == 0 && xc_handler.eWindow == MAIN_WINDOW)
    {
        //get cma
        u32offset = _XC_Device_Offset[psXCInstPri->u32DeviceID];
    }
    else
    {
        u32offset = _XC_Device_Offset[psXCInstPri->u32DeviceID];
    }
#endif
#if 0
    Hal_SC_add_reg_to_shm(pInstance,xc_handler.eWindow,REG_SC_BK0D_38_L,(BIT(0) | BIT(4)), (BIT(0) | BIT(4)));
    Hal_SC_add_reg_to_shm(pInstance,xc_handler.eWindow,REG_SC_BK12_08_L, (u32tmp_address & 0xFFFF), 0xFFFF);
    Hal_SC_add_reg_to_shm(pInstance,xc_handler.eWindow,REG_SC_BK12_09_L, (u32tmp_address >> 16), 0xFFFF);
    Hal_SC_add_reg_to_shm(pInstance,xc_handler.eWindow,REG_SC_BK12_1C_L, 0x0000, 0xFFFF);
#endif
    Hal_SC_update_to_shm(pInstance, xc_handler.eWindow);
    //************************PRINT DATA*******************************//
    //MS_U32 tmp_x=0;
    //for(tmp_x=0;tmp_x < pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].regs_cnt;tmp_x++)
    //{
    //    printf("\033[1;32m[%s:%d][OPTEE][REG:%d] 0x%x:0x%x:0x%x\033[m\n",__FUNCTION__,__LINE__,tmp_x
    //        ,pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].regs_data[tmp_x].bk
    //        ,pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].regs_data[tmp_x].value
    //        ,pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow].regs_data[tmp_x].mask);
    //}
    //************************END PRINT DATA*******************************//
    _MDrv_XC_OPTEE_Send_Cmd(pInstance,E_XC_OPTEE_UPDATE_HANDLER,&pXCResourcePrivate->sthal_Optee.op_tee_xc[xc_handler.eWindow],sizeof(XC_OPTEE_HANDLER));
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return TRUE;
    //****go on
}
MS_BOOL MDrv_XC_OPTEE_Mux(void *pInstance,EN_XC_OPTEE_ACTION action,XC_OPTEE_MUX_DATA xc_mux_data)
{
    MS_U32 u32Index = 0;
    MS_U32 u32Cnt=0;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

#ifndef MSOS_TYPE_OPTEE
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
#else
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
#endif
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    switch (action)
    {
        case E_XC_OPTEE_SET_MUX:
            pXCResourcePrivate->sthal_Optee.op_tee_mux.bfire = TRUE;
            _MDrv_XC_OPTEE_Send_Cmd(pInstance,E_XC_OPTEE_SYNC_MUX,&pXCResourcePrivate->sthal_Optee.op_tee_mux,sizeof(XC_OPTEE_MUX_DATA));
            pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_cnt = 0;
            pXCResourcePrivate->sthal_Optee.op_tee_mux.bfire = FALSE;
            //printf("\033[1;32m[%s:%d][NORMAL]XC_OPTEE_MUX_DATA=%d\033[m\n",__FUNCTION__,__LINE__,sizeof(XC_OPTEE_MUX_DATA));
            break;
        case E_XC_OPTEE_SYNC_MUX:
            memcpy(&pXCResourcePrivate->sthal_Optee.op_tee_mux,&xc_mux_data,sizeof(XC_OPTEE_MUX_DATA));
            //************************PRINT DATA*******************************//
            //for(tmp_x=0;tmp_x < pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_cnt;tmp_x++)
           // {
           //     printf("\033[1;32m[%s:%d][OPTEE][REG:%d] 0x%x:0x%x:0x%x\033[m\n",__FUNCTION__,__LINE__,tmp_x
           //         ,pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[tmp_x].bk
           //         ,pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[tmp_x].value
           //         ,pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[tmp_x].mask);
           // }
            u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_cnt;
            for(u32Index = 0 ; u32Index < u32Cnt ; u32Index++)
            {

                if (u32Index>= OP_TEE_XC_REG_MAX_CNT)
                {
                    printf("array size overflow for ipmux part");
                    break;
                }
                MDrv_Write2ByteMask(pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].bk
                                  , pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].value
                                  , pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].mask);
            }
            //************************END PRINT DATA*******************************//
            break;
        case E_XC_OPTEE_GET_MUX:
            break;
        default:
            break;
    }
    return TRUE;
}
MS_BOOL MApi_XC_OPTEE_Mux_U2(void *pInstance,EN_XC_OPTEE_ACTION action,XC_OPTEE_MUX_DATA xc_mux_data)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    MDrv_XC_OPTEE_Mux(pInstance,action,xc_mux_data);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_OPTEE_Mux(EN_XC_OPTEE_ACTION action,XC_OPTEE_MUX_DATA xc_mux_data)
{
    if (pu32XCInst == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pu32XCInst, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return FALSE;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
     if (pu32XCInst_1 == NULL)
     {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);

        stXCInstantAttribute.u32DeviceID = 1;
        if(UtopiaOpen(MODULE_XC, &pu32XCInst_1, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            return E_APIXC_RET_FAIL;
        }
      }
#endif
    stXC_OPTEE_MUX XCArgs;
    XCArgs.action = action;
    XCArgs.xc_mux_data = xc_mux_data;
    XCArgs.bReturn = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OPTEE_MUX, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturn;
    }
}
#endif
#ifdef MSOS_TYPE_OPTEE
static EN_XC_OPTEE_TIMER_STATUS status=E_XC_OPTEE_TIMER_TEE_DISABLE;
static MS_U8 u8OpteeErrorCnt = 0;
#endif
static MS_BOOL _Mdrv_XC_OPTEE_CheckRBase_Handler(void * pParam,SCALER_WIN eWindow)
{
#if defined(MSOS_TYPE_OPTEE) && defined(SUPPORT_READLIMIT_1BIT_NS_SW)
    MS_BOOL bRBaseLegal = TRUE;
    void *pInstance = pu32XCInst_private;
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);

    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if(!pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable)
    {
        status = E_XC_OPTEE_TIMER_TEE_DISABLE;
        u8OpteeErrorCnt = 0;
    }

    switch (status)
    {
        case E_XC_OPTEE_TIMER_TEE_DISABLE:
            if(pXCResourcePrivate->sthal_Optee.op_tee_xc[MAIN_WINDOW].isEnable)
            {
                status = E_XC_OPTEE_TIMER_TEE_ENABLE;
            }
            break;
        case E_XC_OPTEE_TIMER_TEE_ENABLE:
            if(Hal_SC_secure_Check_RBase(pInstance) == FALSE)
            {
                u8OpteeErrorCnt++;
                status = E_XC_OPTEE_TIMER_TEE_RBASE_ABNORMAL;
            }
            break;
        case E_XC_OPTEE_TIMER_TEE_RBASE_ABNORMAL:
            if(Hal_SC_secure_Check_RBase(pInstance) == FALSE)
            {
                u8OpteeErrorCnt++;
                if(u8OpteeErrorCnt > MAX_OPTEE_ERROR_CNT)
                {
                    status = E_XC_OPTEE_TIMER_TEE_CRASH;
                }
            }
            else
            {
                status = E_XC_OPTEE_TIMER_TEE_ENABLE;
                u8OpteeErrorCnt = 0;
            }
            break;
        case E_XC_OPTEE_TIMER_TEE_CRASH:
            bRBaseLegal = FALSE;
            break;
        default:
            break;
    }
    return bRBaseLegal;
#else
#ifdef MSOS_TYPE_OPTEE
    UNUSED(status);
    UNUSED(u8OpteeErrorCnt);
#endif
    return TRUE;
#endif
}

MS_BOOL MApi_XC_OPTEE_CheckRBase_U2(void* pInstance,SCALER_WIN eWindow)
{
    return _Mdrv_XC_OPTEE_CheckRBase_Handler(pInstance,eWindow);
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_OPTEE_CheckRBase(SCALER_WIN eWindow)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_OPTEE_CheckRBase XCArgs;
    XCArgs.eWindow = eWindow;
    XCArgs.bReturn = FALSE;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_OPTEE_CHECKRBASE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return FALSE;
    }
    else
    {
        return XCArgs.bReturn;
    }
}
#endif
#ifdef MSOS_TYPE_OPTEE
static MS_U32 _MDrv_XC_PipID_Trans(MS_U32 u32PipeID)
{
    if(u32PipeID == pipeID_SC0_main)
        return MAIN_WINDOW;
    else if (u32PipeID == pipeID_SC0_sub)
        return SUB_WINDOW;
    else if (u32PipeID == pipeID_SC1_main)
        return MAIN_WINDOW;
    else
        return 0xFFFF;
}
#endif
MS_U32 MApi_XC_GetResourceByPipeID_U2(void* pInstance,MS_U32 u32PipeID, MS_U32*  U32RetResNum, RESOURCE* pResource)
{
    /// pResource: return scaler memory information
    /// U32RetResNum: check resource number max
#ifdef MSOS_TYPE_OPTEE
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    MS_U32 eWindow = _MDrv_XC_PipID_Trans(u32PipeID);

    //id[3]: 0, id[2]=address_h, id[1]=address_l, id[0]=length
    // 0: SCM Dram buffer
    pResource[0].ID[0] = ((SC_R4BYTE(psXCInstPri->u32DeviceID, REG_SC_BK0D_42_L) & 0xFFFFFFFF) - (SC_R4BYTE(psXCInstPri->u32DeviceID, REG_SC_BK0D_40_L) & 0xFFFFFFFF))*BYTE_PER_WORD;
    pResource[0].ID[1] = (SC_R4BYTE(psXCInstPri->u32DeviceID, REG_SC_BK12_08_L) & 0xFFFFFFFF)*BYTE_PER_WORD;
    pResource[0].ID[2] = 0;
    pResource[0].ID[3] = 0;
    #if (HW_DESIGN_4K2K_VER == 4)
    //id[3]: 0, id[2]=address_h, id[1]=address_l, id[0]=length
    // 1: FRCM Dram buffer
    pResource[1].ID[0] = pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].drams_data[1].length;
    pResource[1].ID[1] = pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].drams_data[1].address & 0xFFFFFFFF;
    pResource[1].ID[2] = 0;
    pResource[1].ID[3] = 0;
    #else
    UNUSED(eWindow);
    #endif
    if(pResource[0].ID[0]==0 ) //If FBL ,DO not lock
        *U32RetResNum=0;
#endif
    return UTOPIA_STATUS_SUCCESS;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_GetResourceByPipeID(MS_U32 u32PipeID, MS_U32*  U32RetResNum, RESOURCE* pResource)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GetResourceByPipeID XCArgs;
    XCArgs.u32PipeID = u32PipeID;
    XCArgs.U32RetResNum = U32RetResNum;
    XCArgs.pResource = pResource;
    XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_RES_BY_PIPE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.u32ReturnValue;
    }

}
#endif
MS_U32 MApi_XC_ConfigPipe_U2(void* pInstance,MS_U32 u32PipeID, MS_U32 u32SecureDMA, MS_U32 u32OperationMode)
{
#ifdef MSOS_TYPE_OPTEE
    /// u32SecureDMA: memory lock
    /// u32OperationMode: RIU register lock
    MS_U32 u32Index = 0;
    MS_U32 u32Cnt=0;
    MS_U32 eWindow = _MDrv_XC_PipID_Trans(u32PipeID);
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));
    if(Hal_SC_secure_lock(pInstance, eWindow, u32SecureDMA, u32OperationMode) == FALSE)
    {
        return UTOPIA_STATUS_FAIL;
    }
    u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_cnt;

    for(u32Index = 0 ; u32Index < u32Cnt ; u32Index++)
    {
        if (u32Index>= OP_TEE_XC_REG_MAX_CNT)
        {
            printf("array size overflow for xc part");
            return UTOPIA_STATUS_FAIL;
        }
        MDrv_Write2ByteMask(pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Index].bk
                          , pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Index].value
                          , pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Index].mask);
    }
    u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_cnt;
    for(u32Index = 0 ; u32Index < u32Cnt ; u32Index++)
    {
        if (u32Index>= OP_TEE_XC_REG_MAX_CNT)
        {
            printf("array size overflow for ipmux part");
            return UTOPIA_STATUS_FAIL;
        }
        MDrv_Write2ByteMask(pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].bk
                          , pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].value
                          , pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Index].mask);
    }

#endif
    return UTOPIA_STATUS_SUCCESS;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_ConfigPipe(MS_U32 u32PipeID, MS_U32 u32SecureDMA, MS_U32 u32OperationMode)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_ConfigPipe XCArgs;
    XCArgs.u32PipeID = u32PipeID;
    XCArgs.u32SecureDMA = u32SecureDMA;
    XCArgs.u32OperationMode = u32OperationMode;
    XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CONFIG_PIPE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.u32ReturnValue;
    }
}
#endif
MS_U32 MApi_XC_CheckPipe_U2(void* pInstance,MS_U32 u32PipeID, MS_U32 u32SecureDMA, MS_U32 u32OperationMode)
{
#ifdef MSOS_TYPE_OPTEE
    /// check PipeID is correct or not
    /// u32SecureDMA: memory lock
    /// u32OperationMode: RIU register lock
    ///MS_U32 u32Index = 0;
    ///MS_U32 u32Cnt=0;
    MS_U32 eWindow = _MDrv_XC_PipID_Trans(u32PipeID);
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);

    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    if(pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].isEnable == FALSE)
    {
        // if there is not enable return fail
        _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
        return UTOPIA_STATUS_FAIL;
    }
#if 0
    u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_cnt;
    //How to get data correctly
    for(u32Index = 0 ; u32Index < u32Cnt ; u32Cnt++)
    {
        if(pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Cnt].value != (MDrv_Read2Byte( pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Cnt].bk)| pXCResourcePrivate->sthal_Optee.op_tee_xc[eWindow].regs_data[u32Cnt].mask ))
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            return UTOPIA_STATUS_FAIL;
        }
    }
    u32Cnt = pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_cnt;
    for(u32Index = 0 ; u32Index < u32Cnt ; u32Cnt++)
    {
        if( pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Cnt].value != (MDrv_Read2Byte( pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Cnt].bk )| pXCResourcePrivate->sthal_Optee.op_tee_mux.regs_data[u32Cnt].mask ))
        {
            _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            return UTOPIA_STATUS_FAIL;
        }
    }
#endif
#endif
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    return UTOPIA_STATUS_SUCCESS; // success
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_CheckPipe(MS_U32 u32PipeID, MS_U32 u32SecureDMA, MS_U32 u32OperationMode)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_CheckPipe XCArgs;
    XCArgs.u32PipeID = u32PipeID;
    XCArgs.u32SecureDMA = u32SecureDMA;
    XCArgs.u32OperationMode = u32OperationMode;
    XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_CHECK_PIPE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.u32ReturnValue;
    }
}
#endif
MS_BOOL MApi_XC_SetSWDRInfo_U2(void* pInstance, XC_SWDR_INFO *pSWDR_INFO)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    MDrv_XC_SetSWDRInfo(pInstance,pSWDR_INFO);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_SetSWDRInfo(XC_SWDR_INFO *pSWDR_INFO)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_GET_PIXEL_RET_OUT_OF_RANGE;
    }

    stXC_SWDR_INFO XCArgs;
    XCArgs.pSWDR_INFO = pSWDR_INFO;
    XCArgs.bReturnValue = UTOPIA_STATUS_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_SET_SWDR_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
MS_BOOL MApi_XC_GetSWDRInfo_U2(void* pInstance, XC_SWDR_INFO *pSWDR_INFO)
{
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);

    MDrv_XC_GetSWDRInfo(pInstance,pSWDR_INFO);

    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return TRUE;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_GetSWDRInfo(XC_SWDR_INFO *pSWDR_INFO)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_GET_PIXEL_RET_OUT_OF_RANGE;
    }

    stXC_SWDR_INFO XCArgs;
    XCArgs.pSWDR_INFO = pSWDR_INFO;
    XCArgs.bReturnValue = UTOPIA_STATUS_FAIL;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_SWDR_INFO, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.bReturnValue;
    }
}
#endif
#ifdef UFO_XC_SUPPORT_HDMI_DOWNSCALE_OUTPUT_POLICY
MS_U32 MApi_XC_GetHDMIPolicy_U2(void* pInstance)
{
    MS_U32 u32HDMIPolicy = 0;
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Entry\n");
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psXCInstPri);
    _XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR);
    _XC_ENTRY(pInstance);
    u32HDMIPolicy = MDrv_SC_GetHDMIPolicy(pInstance);
    _XC_RETURN(pInstance);
    _XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
    XC_LOG_TRACE(XC_DBGLEVEL_FUNCTION_TRACE, "Exit\n");
    return u32HDMIPolicy;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_U32 MApi_XC_GetHDMIPolicy(void)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }

    stXC_GET_HDMIPOLICY XCArgs;
    XCArgs.u32HDMIPolicyInfo = 0;

    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_GET_HDMIPOLICY, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
    {
        printf("Obtain XC engine fail\n");
        return UTOPIA_STATUS_FAIL;
    }
    else
    {
        return XCArgs.u32HDMIPolicyInfo;
    }
}
#endif
#endif
