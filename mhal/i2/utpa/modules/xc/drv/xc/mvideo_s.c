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
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#ifdef STELLAR

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
#include "drv_sc_display.h"

//Add for A5
#include "mhal_dip.h"


// Tobe refined
#include "xc_hwreg_utility2.h"
#include "mhal_mux.h"
#include "mhal_hdmi.h"
#include "mhal_ip.h"
#include "mhal_sc.h"
#include "mhal_s_disp_ctl.h"

// extern functions
extern MS_BOOL MDrv_XC_SetIOMapBase_i(void* pInstance);

// Local functions
MS_BOOL _MDrv_XC_S_ADC_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
        // ADC
        case E_XC_S_ADC_SCART_OVERLAY_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_ADC_ScartOverlay_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_ADC_GAIN_SET:
			if(pbuf != NULL && u32BufSize == sizeof(APIXC_AdcGainOffsetSetting))
			{
				return Hal_XC_S_ADC_Gain_SET(pInstance,(APIXC_AdcGainOffsetSetting *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_ADC_GAIN_GET:
			if(pbuf != NULL && u32BufSize == sizeof(APIXC_AdcGainOffsetSetting))
			{
				return Hal_XC_S_ADC_Gain_GET(pInstance,(APIXC_AdcGainOffsetSetting *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_ADC_OFFSET_SET:
			if(pbuf != NULL && u32BufSize == sizeof(APIXC_AdcGainOffsetSetting))
			{
				return Hal_XC_S_ADC_Offset_SET(pInstance,(APIXC_AdcGainOffsetSetting *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_ADC_OFFSET_GET:
			if(pbuf != NULL && u32BufSize == sizeof(APIXC_AdcGainOffsetSetting))
			{
				return Hal_XC_S_ADC_Offset_GET(pInstance,(APIXC_AdcGainOffsetSetting *)pbuf);
			}
			else
			{
				return bret;
			}
			break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_HDMI_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// HDMI
		case E_XC_S_HDMI_HDE_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_HDMI_HDE_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_HDMI_HTT_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_HDMI_HTT_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_HDMI_VDE_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_HDMI_VDE_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_HDMI_VTT_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_HDMI_VTT_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_HDMI_DDC_CHANNEL_EN:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_HDMI_DDCChannel_EN(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_HDMI_HPD_SET:
		    break;
		case E_XC_S_HDMI_SYNC_MODE_SET:
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_PCMode_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// PCMode
		case E_XC_S_PCMODE_HRES_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PCMode_HResolution_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PCMODE_VRES_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PCMode_VResolution_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PCMODE_HTT_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PCMode_Htotal_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_XC_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
        // XC
		case E_XC_S_XC_OPEN:
			// [Not implement yet]
			break;
		case E_XC_S_XC_CLOSE:
			// [Not implement yet]
			break;
		case E_XC_S_XC_FB_SIZE_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_FrameBufferNumber_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_FB_SIZE_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_FrameBufferNumber_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_RW_POINT_DIFF_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_RwPointDiff_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_RW_POINT_DIFF_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_RwPointDiff_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_OPW_OFF_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_OpwOff_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_OPW_OFF_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_OpwOff_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_INTERLACE_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_InterlaceStatus_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_FIELD_DETECT_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_XC_FieldDetect_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_DISP_DE_WIN_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_WINDOW_TYPE))
			{
				return Hal_XC_S_XC_DispDeWin_GET(pInstance,(MS_WINDOW_TYPE *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_XC_FORCE_NO_AUTO_FIT_SET:
			break;
		case E_XC_S_XC_IOMAP_BASE_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return MDrv_XC_SetIOMapBase_i(pInstance);
			}
			else
			{
				return bret;
			}
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_Pattern_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// Pattern
		case E_XC_S_PATTERN_OP1_RGB_SET:
			// [Not implement yet]
			break;
		case E_XC_S_PATTERN_OP2_RGB_SET:
			// [Not implement yet]
			break;
		case E_XC_S_PATTERN_IP1_RGB_SET:
			// [Not implement yet]
			break;
		case E_XC_S_PATTERN_OP_SET:
			// [Not implement yet]
			break;
		case E_XC_S_PATTERN_VOP_SET:
			// [Not implement yet]
			break;
		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_DS_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		case E_XC_S_DS_STATUS_GET:
			if(pbuf != NULL && u32BufSize == sizeof(DSLOAD_TYPE))
			{
				return Hal_XC_S_DS_Status_GET(pInstance,(DSLOAD_TYPE *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_DS_SCALING_WIN_SET:
			// [Not implement yet]
			break;
		case E_XC_S_DS_GST_PROCESS_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_GST_PROCESS_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_DS_GST_PROCESS_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_GST_PROCESS_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_DS_SEAMLESS_PLAY_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_GST_PROCESS_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_DS_SEAMLESS_PLAY_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_GST_PROCESS_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_DS_CAPTURE_PROCESS_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_CAPTURE_PROCESS_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;
		case E_XC_S_DS_CAPTURE_PROCESS_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_DS_CAPTURE_PROCESS_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
			break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_PIP_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// PIP / Smart Zoom
		case E_XC_S_PIP_SMART_ZOOM_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PIP_SmartZoom_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PIP_SMART_ZOOM_GET:
				if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PIP_SmartZoom_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
             case  E_XC_S_PIP_WINDOW_ENABLE:
                    if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
                        {
                                MS_XC_CUS_CTRL_PARA *ptempbuf = pbuf;
                                MDrv_XC_EnablePIP(pInstance, ptempbuf->bParam);
                        }
                        else
                        {
                                return bret;
                        }
                        break;
		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_3D_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// 3D
		case E_XC_S_3D_KR3D_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_3D_KR3DMode_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_3D_KR3D_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_3D_KR3DMode_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_PVR_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// PVR
		case E_XC_S_PVR_DUAL_WIN_EN:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PVR_DualWinForAPVR_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PVR_DUAL_WIN_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PVR_DualWinForAPVR_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_PQ_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// PQ
		case E_XC_S_PQ_NR_DEMO_GET:
			break;
		case E_XC_S_PQ_HSC_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_PQ_PARA))
			{
				return Hal_XC_S_PQ_PicSetHSC_SET(pInstance,(MS_XC_CUS_PQ_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PQ_BT2020_SET:
			break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}

MS_BOOL _MDrv_XC_S_PNL_Ctrl(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

    switch(u32Cmd)
    {
		// PNL
		case E_XC_S_PNL_PACKER_CHECK_SET:
			break;
		case E_XC_S_PNL_SKIP_INIT_FLAG_SET:
			break;
		case E_XC_S_PNL_STR_EN:
			// [Not implement yet]
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_STR_EN(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PNL_PANEL_INCH_SET:
			// [Not implement yet]
			break;
		case E_XC_S_PNL_CUS_FLAG_SET:
			break;
		case E_XC_S_PNL_PRE_EMPHASIS_LEVEL_SET:
			break;
		case E_XC_S_PNL_VREG_LEVEL_SET:
			break;
		case E_XC_S_PNL_OUT_PE_CURRENT_SET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_OutPECurrent_SET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PNL_OUT_PE_CURRENT_PARTIAL_SET:
			// [Not implement yet]
		    break;
		case E_XC_S_PNL_HSTART_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_PanelHStart_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PNL_VSTART_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_PanelVStart_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PNL_PANEL_HEIGHT_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_PanelHeight_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;
		case E_XC_S_PNL_PANEL_WIDTH_GET:
			if(pbuf != NULL && u32BufSize == sizeof(MS_XC_CUS_CTRL_PARA))
			{
				return Hal_XC_S_PNL_PanelWidth_GET(pInstance,(MS_XC_CUS_CTRL_PARA *)pbuf);
			}
			else
			{
				return bret;
			}
		    break;

		default:
			printf("Unknow Handle ID!\n");
			break;
    }

    return bret;
}


//-------------------------------------------------------------------------------------------------
/// MApi_XC_S_Disp_Ctrl_U2              a func for special case which need to add fucntion to match either middle ware layer or direct api
/// @param  pInstance                   \b IN: u2 framework instance
/// @param  u32Cmd                      \b IN: command ID
/// @param  pbuf                        \b IN: input structure
/// @param  u32BufSize                  \b IN: sizeof input structure
//-------------------------------------------------------------------------------------------------
MS_BOOL MApi_XC_S_Disp_Ctrl_U2(void* pInstance,MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    MS_BOOL bret = FALSE;

	if(u32Cmd > E_XC_S_ADC_CMD_START && u32Cmd < E_XC_S_ADC_CMD_END)
	{
		bret = _MDrv_XC_S_ADC_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_HDMI_CMD_START && u32Cmd < E_XC_S_HDMI_CMD_END)
	{
		bret = _MDrv_XC_S_HDMI_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_PCMODE_CMD_START && u32Cmd < E_XC_S_PCMODE_CMD_END)
	{
		bret = _MDrv_XC_S_PCMode_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_XC_CMD_START && u32Cmd < E_XC_S_XC_CMD_END)
	{
		bret = _MDrv_XC_S_XC_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_PATTERN_CMD_START && u32Cmd < E_XC_S_PATTERN_CMD_END)
	{
		bret = _MDrv_XC_S_Pattern_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_DS_CMD_START && u32Cmd < E_XC_S_DS_CMD_END)
	{
		bret = _MDrv_XC_S_DS_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_PIP_CMD_START && u32Cmd < E_XC_S_PIP_CMD_END)
	{
		bret = _MDrv_XC_S_PIP_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_3D_CMD_START && u32Cmd < E_XC_S_3D_CMD_END)
	{
		bret = _MDrv_XC_S_3D_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_PQ_CMD_START && u32Cmd < E_XC_S_PQ_CMD_END)
	{
		bret = _MDrv_XC_S_PQ_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
	else if(u32Cmd > E_XC_S_PNL_CMD_START && u32Cmd < E_XC_S_PNL_CMD_END)
	{
		bret = _MDrv_XC_S_PNL_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
	}
        else if(u32Cmd > E_XC_S_PVR_CMD_START && u32Cmd < E_XC_S_PVR_CMD_END)
        {
                bret = _MDrv_XC_S_PVR_Ctrl(pInstance, u32Cmd, pbuf, u32BufSize);
        }
	else
	{
		printf("Unknow Handle ID!\n");
	}
    return bret;
}
#ifndef UTOPIAXP_REMOVE_WRAPPER
MS_BOOL MApi_XC_S_Disp_Ctrl(MS_U32 u32Cmd,void *pbuf,MS_U32 u32BufSize)
{
    if (pu32XCInst == NULL)
    {
        printf("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return E_APIXC_RET_FAIL;
    }
    stXC_CMD_S_DISP_CTRL XCArgs;
    XCArgs.u32Cmd = u32Cmd;
    XCArgs.pbuf = pbuf;
    XCArgs.u32BufSize = u32BufSize;
    XCArgs.bReturn = FALSE;
    if(UtopiaIoctl(pu32XCInst, E_XC_CMD_S_DISP_CTRL, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
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
#endif
