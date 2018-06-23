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

/**********************************************************************
 Copyright (c) 2006-2009 MStar Semiconductor, Inc.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 MStar Semiconductor Inc. and be kept in strict confidence
 (MStar Confidential Information) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of MStar Confidential
 Information is unlawful and strictly prohibited. MStar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.

* Class : mapi_ftech
* File  : mapi_fetch.h
**********************************************************************/


/*@ <IncludeGuard> @*/
#ifndef _MAPI_FETCH_H_
#define _MAPI_FETCH_H_
/*@ </IncludeGuard> @*/

#include <stdio.h>

//#include "mapi_types.h"
//#include "mapi_syscfg_table.h"
//#include "iniparser.h"
//#include "mapi_display_timing_datatype.h"
#include "mi_common.h"
#include "mi_syscfg_datatype.h"
#define bool char
#define true 1
#define false 0

typedef struct
{
    //system.ini
    char ModelName[128];
    char PanelName[128];
    MI_U8 PQBinDefault;    //if PQBinDefault==1, use the PQ binary file; if PQBinDefault ==0, use the customer pq binary file.
    MI_U8 PQPathName[64];
    int GammaTableNo;
    int TotalGammaTableNo;
    MI_U8 TunerSelectNo;
    MI_U8 TunerSAWType;
    MI_U8 AudioAmpSelect;
    MI_BOOL UrsaEanble;
    MI_U8 UrsaSelect;
    MI_BOOL MEMCPanelEnable;
    MI_U8 MEMCPanelSelect;
    bool bGammabinflags;
    bool bPqBypassSupported;
    MI_U8 u8AVSyncDelay;
    bool bMirrorVideo;
    MI_U8 u8MirrorType;
    bool bEnable3DOverScan;
    bool bHbbtvDelayInitFlag;
    bool bUseCustomerScreenMuteColor;
    MI_U8 u8NoSignalColor;
    MI_U8 u8FrameColorRU;
    MI_U8 u8FrameColorGY;
    MI_U8 u8FrameColorBV;
    bool bNandHdcpEnable;
    bool bSPIHdcpEnable;
    MI_U8 u8HdcpSPIBank;
    MI_U16 u16HdcpSPIOffset;
    bool bEEPROMHdcpEnable;
    MI_U8 u8HdcpEEPROMAddr;
    char pHDCPKeyFileName[128];
    bool bSPIMacEnable;
    MI_U8 u8MacSPIBank;
    MI_U16 u16MacSPIOffset;
    bool bLocalDIMMINGEnable;
    MI_U8 u8LocalDIMMINGPanelSelect;
    char BoardName[32];
    char SoftWareVer[32];
    char SysPanelName[64];
    bool bPipPairInfo[MAPI_INPUT_SOURCE_NUM+1][MAPI_INPUT_SOURCE_NUM+1];
    bool bPopPairInfo[MAPI_INPUT_SOURCE_NUM+1][MAPI_INPUT_SOURCE_NUM+1];
    bool bTravelingPairInfo[MAPI_INPUT_SOURCE_NUM+1][MAPI_INPUT_SOURCE_NUM+1];
    bool bPanel4K2KModeNeedCmd;
    MI_U8  clk_en;
    MI_U8  bypass_en;
    MI_U8  invAlpha;
    MI_U8  hsync_vfde;
    MI_U8  hfde_vfde;
    MI_U32 OsdcLpllType;
}IniInfo;

/// section filter number config
typedef struct
{
    MI_U16 u16Section1kFilterNumber;
    MI_U16 u16Section4kFilterNumber;
    MI_U16 u16Section64kFilterNumber;
}MAPI_SECTION_FILTER_CONFIG;

#define MSPI_DEVICE_MAX_NUM  10
#define MODELINFO_4K2K_NUM 10
#define DEMOD_INFO_BUFFER_SIZE 1024
#define MODULE_PARAMETER_SIZE (20)
#define MST_FEATURE_NUM_MAX (50)
#define CUS_FEATURE_NUM_MAX (25)
#define TEMP_FEATURE_NUM_MAX (25)
#define DMDTYPE_MAX (30)

#define MODULE_PARAMETER_FEATURE_SIZE (128)
#define MODULE_VALUE_FEATURE_SIZE     (128)

//////////////// End Module Parameter////////////////

//////////////// For 3D convert rule////////////////


/// This class is to set/get the system information. So it uses private constructor/destructor since it should only have static functions.
    /// Define aspect ratio type

    //-------------------------------------------------------------------------------------------------
    /// Set Input source mux table
    /// @param pTemp    \b IN: pointer to input mux table
    /// @param u8Num    \b IN: number of pTemp array
    //-------------------------------------------------------------------------------------------------
    void SetInputMux(const MAPI_VIDEO_INPUTSRCTABLE* const pTemp, MI_U8 u8Num);

    //-------------------------------------------------------------------------------------------------
    /// Get Input source mux table
    /// @return pointer to input source mux table
    //-------------------------------------------------------------------------------------------------
    const MAPI_VIDEO_INPUTSRCTABLE* GetInputMuxInfo();

    //-------------------------------------------------------------------------------------------------
    /// Get Dtv type of route path
    /// @param u8RouteIndex \b IN: index of route (0~(MAXROUTECOUNT-1))
    /// @return MI_U8 bitmapping of DTV type
    //-------------------------------------------------------------------------------------------------
    MI_U8   GetRouteTVMode(MI_U8 u8RouteIndex);

    //-------------------------------------------------------------------------------------------------
    /// Get IP enable bits
    /// @return MI_U16 bitmap of IP enable or not
    //-------------------------------------------------------------------------------------------------
    MI_U16 get_IPEnableType();

    //-------------------------------------------------------------------------------------------------
    /// Get STB system type
    /// @return STB system type
    //-------------------------------------------------------------------------------------------------
    MI_U8 get_STB_SystemType();

    //-------------------------------------------------------------------------------------------------
    /// Set HDMI analog info
    /// @param pHDMITxAnalogInfo   \b IN: pointer to HDMI analog info
    /// @param uCfgTblNum   \b IN: table number
    //-------------------------------------------------------------------------------------------------
    void SetHDMITxAnalogInfo(const HDMITx_Analog_Param_t* const pHDMITxAnalogInfo, MI_U8 uCfgTblNum);

    //-------------------------------------------------------------------------------------------------
    /// To get VD capture window mode
    /// @return                 \b OUT: VD capture window mode
    //-------------------------------------------------------------------------------------------------
    EN_VD_CAPTURE_WINDOW_MODE GetVDCaptureWinMode();

    //-------------------------------------------------------------------------------------------------
    /// To Set VD capture window mode
    /// @param enVDCaptureWinMode \b IN: VD capture window mode
    /// @return                 \b OUT: none
    //-------------------------------------------------------------------------------------------------
    void SetVDCaptureWinMode(EN_VD_CAPTURE_WINDOW_MODE  enVDCaptureWinMode);

    //-------------------------------------------------------------------------------------------------
    /// Set audio configuration
    /// @param u8AudioMuxInfoSize       \b IN: number of audio mux info array
    /// @param u8AudioPathInfoSize      \b IN: number of audio path info array
    /// @param u8AudioOutputTypeInfo    \b IN: number of audio output type info array
    /// @param pAudioMuxInfo                \b IN: pointer to audio mux info
    /// @param pAudioPathInfo               \b IN: pointer to audio path info
    /// @param pAudioOutputTypeInfo     \b IN: pointer to audio output type info
    /// @param pAudioDefaultInit            \b IN: pointer to audio default init value
    /// @return pointer to serial DMX table
    //-------------------------------------------------------------------------------------------------
    void SetAudioInputMuxCfg(MI_U8 u8AudioMuxInfoSize, MI_U8 u8AudioPathInfoSize, MI_U8 u8AudioOutputTypeInfo, const AudioMux_t* const pAudioMuxInfo, const AudioMux_t* const pAudioPathInfo, const AudioOutputType_t * const pAudioOutputTypeInfo, const AudioDefualtInit_t* const pAudioDefaultInit, MI_U32 BOARD_AUDIO_MICROPHONE_PATH);



    //-------------------------------------------------------------------------------------------------
    /// Get audio default init value
    /// @return pointer to audio default init value
    //-------------------------------------------------------------------------------------------------
    const AudioDefualtInit_t *GetAudioDefaultInit();

    //-------------------------------------------------------------------------------------------------
    /// Get audio input source mux table
    /// @return pointer to audio input source mux table
    //-------------------------------------------------------------------------------------------------
    const AudioMux_t* GetAudioInputMuxInfo();

    //-------------------------------------------------------------------------------------------------
    /// Get audio path mux table
    /// @return pointer to udio path mux table
    //-------------------------------------------------------------------------------------------------
    const AudioPath_t* GetAudioPathInfo();

    //-------------------------------------------------------------------------------------------------
    /// Get audio path microphone channel
    /// @return emum to audio path microphone channel
    //-------------------------------------------------------------------------------------------------
    MI_U32 GetAudioMicPathInfo();

    //-------------------------------------------------------------------------------------------------
    /// Get audio output path mux table
    /// @return pointer to audio output path mux table
    //-------------------------------------------------------------------------------------------------
    const AudioOutputType_t* GetAudioOutputTypeInfo();

    //------------------------------------------------------------------------------
    /// Set gamma table configuration
    /// @param pGammaTableInfo \b IN: pointer to gamma table information
    /// @param u8TblCnt   \b IN: number of gamma table configurations
    /// @param u8Default  \b IN: default gamma table index
    //------------------------------------------------------------------------------
    void SetGammaTableCfg(GAMMA_TABLE_t* pGammaTableInfo[], MI_U8 u8TblCnt, MI_U8 u8Default);

    //-------------------------------------------------------------------------------------------------
    /// Get default gamma table index
    /// @return default gamma table index
    //-------------------------------------------------------------------------------------------------
    MI_U8 GetDefaultGammaIdx();

    //-------------------------------------------------------------------------------------------------
    /// Get DLC info table count
    /// @return : DLC info table count
    //-------------------------------------------------------------------------------------------------
    MI_U8 GetDLCTableCount(void);

    //-------------------------------------------------------------------------------------------------
    /// Set DLC info table count
    /// @param u8DlcTableCount    \b IN: DLC table count
    //-------------------------------------------------------------------------------------------------
    void SetDLCTableCount(MI_U8 u8DlcTableCount);

    void SetDLCInfo(MAPI_XC_DLC_init *Board_DLC_init[], MI_U8 u8TblCnt);

    void GetDLCInfo(MAPI_XC_DLC_init *Board_DLC_init, MI_U8 u8TblCnt);

    //-------------------------------------------------------------------------------------------------
    /// Get Color matrix info
    /// @param
    //-------------------------------------------------------------------------------------------------
    void SetColorMatrix(MAPI_COLOR_MATRIX *pMatrx);

    void GetColorMatrix(MAPI_COLOR_MATRIX *pMatrx);

    //-------------------------------------------------------------------------------------------------
    /// To get PQ binary file's path  from customer.ini
    /// @return                 \b OUT: PQ binary file's path
    //-------------------------------------------------------------------------------------------------
    MI_U8 * GetPQPathName(void);
    //-------------------------------------------------------------------------------------------------
    /// To get PQ binary file's path  from customer.ini
    /// @param ps8ini_strval    \b IN: pointer to PQPath name
    /// @param len              \b IN: the leng og PQPath name
    //-------------------------------------------------------------------------------------------------
    void SetPQPathName(char* ps8ini_strval, MI_S32 len);
    //-------------------------------------------------------------------------------------------------
    /// Get gamma table
    /// @param u8Idx                   IN: Gamma table index
    /// @return pointer to gamma table
    //-------------------------------------------------------------------------------------------------
    GAMMA_TABLE_t* GetGammaTableInfo(MI_U8 u8Idx);

    //------------------------------------------------------------------------------
    /// Set panel configuration
    /// @param pstPanelInfo \b IN: pointer to panel information
    /// @param iPanelSize   \b IN: number of panel configurations
    /// @param u16LVDS_Output_type  \b IN: LVDS output type
    /// @param ptPanelBacklightPWMInfo  \b IN: pointer to PanelBacklightPWMInfo
    //------------------------------------------------------------------------------
    void SetPanelInfoCfg(PanelInfo_t * pstPanelInfo, int iPanelSize, MI_U16 u16LVDS_Output_type, PanelBacklightPWMInfo * ptPanelBacklightPWMInfo);

    //------------------------------------------------------------------------------
    /// Set panel configuration
    /// @param bEnablePanelModPvddCfg \b IN: enable panel MOD PVDD
    /// @param bPanelModPvddType   \b IN: panel MOD PVDD type
    /// @return TRUE: set success, FALSE: set fail
    //------------------------------------------------------------------------------
    MI_BOOL SetPanelModPvddInfo(MI_BOOL bEnablePanelModPvddCfg, MI_BOOL bPanelModPvddType);

    //-------------------------------------------------------------------------------------------------
    ///  Set Customer PQC fg
    /// @param pPQCustomerBinFilePath : PQ Customer Bin File Path
    /// @param enWinType : windows type
    //-------------------------------------------------------------------------------------------------
    void SetCustomerPQCfg(char * pPQCustomerBinFilePath, MAPI_PQ_WIN enWinType);

    //-------------------------------------------------------------------------------------------------
    ///  Get Customer PQC fg
    /// @param enWinType : windows type
    /// @return current PQ path name
    //-------------------------------------------------------------------------------------------------
    const char *GetCustomerPQCfg(MAPI_PQ_WIN enWinType);

    //-------------------------------------------------------------------------------------------------
    /// Set PQ Auto NR Param
    /// @param  pParam               \b IN: pointer to PQ Auto NR Param
    /// @return None
    //-------------------------------------------------------------------------------------------------
    void  SetPQAutoNRParam(const MAPI_AUTO_NR_INIT_PARAM* const pParam);

    //-------------------------------------------------------------------------------------------------
    /// Get PQ Auto NR Param
    /// @return pointer to PQ Auto NR Param
    //-------------------------------------------------------------------------------------------------
    MAPI_AUTO_NR_INIT_PARAM* GetPQAutoNRParam();

    //-------------------------------------------------------------------------------------------------
    /// Set SAW Type from board define
    /// @param  SawType               \b IN: SAW Type
    /// @return None
    //-------------------------------------------------------------------------------------------------
    void  SetSAWType(SawArchitecture SawType);

    //-------------------------------------------------------------------------------------------------
    /// Get SAW Type from board define
    /// @return SawType
    //-------------------------------------------------------------------------------------------------
    SawArchitecture GetSAWType(void);

    //-------------------------------------------------------------------------------------------------
    /// Set Video Mirror mode flag
    /// @param bMirrorEnable     \b: IN: enable mirror mode or not
    /// @param u8MirrorType     \b: IN: Mirror type 0:MIRROR_NORMAL, 1:MIRROR_H_ONLY, 2:MIRROR_V_ONLY, 3:MIRROR_HV,
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetVideoMirrorCfg(MI_BOOL bMirrorEnable , MI_U8 u8MirrorType);

    //-------------------------------------------------------------------------------------------------
    /// Get Video Mirror Flag
    /// @return the Video Mirror Flag (0:MIRROR_DISABLE, 1:MIRROR_ENABLE)
    //-------------------------------------------------------------------------------------------------
    MI_BOOL GetMirrorVideoFlag();

    //-------------------------------------------------------------------------------------------------
    /// Get Video Mirror Mode
    /// @return the Video Mirror Mode (0:MIRROR_NORMAL, 1:MIRROR_H_ONLY, 2:MIRROR_V_ONLY, 3:MIRROR_HV)
    //-------------------------------------------------------------------------------------------------
    MI_U8 GetMirrorVideoMode();

    //-------------------------------------------------------------------------------------------------
    /// Set Freerun Config
    /// @param b3D         \b: IN: 3D or 2D
    /// @param bEnable     \b: IN: Enable Freerun or not
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetFreerunCfg(MI_BOOL b3D, MI_BOOL bEnable);

    //-------------------------------------------------------------------------------------------------
    /// Get Freerun Flag
    /// @param b3D         \b: IN: 3D or 2D
    /// @return the Freerun Flag
    //-------------------------------------------------------------------------------------------------
    MI_BOOL GetFreerunFlag(MI_BOOL b3D);

    //-------------------------------------------------------------------------------------------------
    /// Set FBL mode's threshold. if vde*hde*framerate >= u32Threshold, the video will go fbl mode
    /// @param u32Threshold     \b: IN: FBL mode's threshold
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetFBLModeThreshold(MI_U32 u32Threshold);

    //-------------------------------------------------------------------------------------------------
    /// Get FBL mode's threshold
    /// @return threshold
    //-------------------------------------------------------------------------------------------------
    MI_U32 GetFBLModeThreshold();

    //-------------------------------------------------------------------------------------------------
    /// Set Video information configuration
    /// @param enVideoNum   \b IN: video source type
    /// @param u8ResNum     \b IN: number of video info
    /// @param pstVideoInfo     \b IN: pointer to Video info
    /// @param bHotPlugInverse  \b IN: hot plug inverse flag
    /// @param bPqBypassSupported
    /// @param u32HDMI5VDetectGPIOSelect   \b IN: HDMI 5V Detect GPIO Select
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetVideoInfoCfg(VideoInfo_t enVideoNum, MI_U8 u8ResNum, const ST_MAPI_VIDEO_WINDOW_INFO *pstVideoInfo, MI_U32 u32HotPlugInverse, MI_BOOL bPqBypassSupported, MI_U32 u32Hdmi5vDetectGpioSelect);

    //-------------------------------------------------------------------------------------------------
    /// get resolution info table size
    /// @return pointer to resolution info
    //-------------------------------------------------------------------------------------------------
    const ResolutionInfoSize* GetResolutionInfo();

    //-------------------------------------------------------------------------------------------------
    /// If m_bPqBypassSupported enable
    /// @return bool to tell if m_bPqBypassSupported is true.
    //-------------------------------------------------------------------------------------------------
    MI_BOOL GetPqBypassSupported();

    //-------------------------------------------------------------------------------------------------
    /// Get Video windows table infor
    /// @param E_VideoInfo \b video type of  window
    /// @return ST_MAPI_VIDEO_WINDOW_INFO  table of video window infor
    //-------------------------------------------------------------------------------------------------
    const ST_MAPI_VIDEO_WINDOW_INFO** GetVideoWinInfo(VideoInfo_t E_VideoInfo);

    //-------------------------------------------------------------------------------------------------
    /// Get dispout device input capability
    /// @param capability name : ex , Dispout_Cap:VB1_2V,Dispout_Cap:VB1_4V2O
    /// @return support or not
    //-------------------------------------------------------------------------------------------------
    MI_U32 GetDispoutDeviceInputCap(char* capName );

    //-------------------------------------------------------------------------------------------------
    /// Set Blue Screen Flag
    /// @param bUseCustomerScreenMuteColor
    /// @return bool to tell if SetUseCustomerScreenMuteColorFlag is true
    //-------------------------------------------------------------------------------------------------
    MI_BOOL SetUseCustomerScreenMuteColorFlag(MI_BOOL bUseCustomerScreenMuteColor);

    //-------------------------------------------------------------------------------------------------
    /// Get Blue Screen Flag
    /// @return bool to tell if m_bUseCustomerScreenMuteColor is true.
    //-------------------------------------------------------------------------------------------------
    MI_BOOL GetUseCustomerScreenMuteColorFlag();

    //-------------------------------------------------------------------------------------------------
    /// Set Customer Screen Mute Color Type
    /// @param u8ScreenMuteColorType   \b IN: screen mute color type
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SeCustomerScreenMuteColorType(MI_U8 u8ScreenMuteColorType);

    //-------------------------------------------------------------------------------------------------
    /// Get Customer Screen Mute Color Type
    /// @return screen mute color type.
    //-------------------------------------------------------------------------------------------------
    MI_U8 GeCustomerScreenMuteColorType(void);

    //-------------------------------------------------------------------------------------------------
    /// Set Customer Frame Color Type
    /// @param u8FrameColorR   \b IN: Frame color R
    /// @param u8FrameColorG   \b IN: Frame color G
    /// @param u8FrameColorB   \b IN: Frame color B
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetCustomerFrameColorType(MI_U8 u8FrameColorR, MI_U8 u8FrameColorG, MI_U8 u8FrameColorB);

    //-------------------------------------------------------------------------------------------------
    /// Get Customer Frame Color Type
    /// @return screen frame color type, R8bit+G 8bit+B 8bit
    //-------------------------------------------------------------------------------------------------
    MI_U32 GetCustomerFrameColorType(void);

    //-------------------------------------------------------------------------------------------------
    /// Get AMP init bin path
    /// @return 		  \b OUT: AMP init bin path
    //-------------------------------------------------------------------------------------------------
    const char *GetAMPBinPath();
    //-------------------------------------------------------------------------------------------------
    /// Set AMP init bin path
    /// @param path
    /// @return none
    //-------------------------------------------------------------------------------------------------
    void SetAMPBinPath(char *path);
    //-------------------------------------------------------------------------------------------------
    /// SetTspSectionFilterConfig
    /// @param rCfg \b IN: config for section filter number
    /// @return TRUE: setting is ok, FALSE: setting is failed
    //-------------------------------------------------------------------------------------------------
    MI_BOOL SetTspSectionFilterConfig(MAPI_SECTION_FILTER_CONFIG *rCfg);
    //-------------------------------------------------------------------------------------------------
    /// GetTspSectionFilterConfig
    /// @return : config for section filter number
    //-------------------------------------------------------------------------------------------------
    const MAPI_SECTION_FILTER_CONFIG *GetTspSectionFilterConfig();

    //-------------------------------------------------------------------------------------------------
    /// Set Current Mode
    ///@param enCurrentMode IN: the mode of current
    /// @return : None
    //-------------------------------------------------------------------------------------------------
    void SetCurrentMode(EN_CURRENT_MODE_TYPE enCurrentMode);

    //-------------------------------------------------------------------------------------------------
    /// Set Current Mode
    ///@param enCurrentMode IN: the mode of current
    /// @return : E_CURRENT_MODE_FULL.
    /// @return : E_CURRENT_MODE_HALF.
    /// @return : E_CURRENT_MODE_QUART.
    //-------------------------------------------------------------------------------------------------
    EN_CURRENT_MODE_TYPE GetCurrentMode(void);

    //-------------------------------------------------------------------------------------------------
    /// Set Bw customer Mode bCusModeEn
    /// @param bCusModeEn IN: 1 enable bw customer mode,0 disable bw customer mode
    /// @return : None
    //-------------------------------------------------------------------------------------------------
    void SetBwCusMode(MI_BOOL bCusModeEn);

    //-------------------------------------------------------------------------------------------------
    /// Set Bw customer Mode bCusModeEn
    /// @param None
    /// @return MI_BOOL : 1 enable bw customer mode,0 disable bw customer mode
    //-------------------------------------------------------------------------------------------------
    MI_BOOL GetBwCusMode(void);

#define ISFBL_EN()        ((mapi_syscfg_fetch::get_IPEnableType())&(_FBL_ENABLE))
#define ISAUDIOSIF_EN()   ((mapi_syscfg_fetch::get_IPEnableType())&(_AUDIO_SIF_ENABLE))
#define ISPRE_SCALE_EN()  ((mapi_syscfg_fetch::get_IPEnableType())&(_PRE_SCALE_DOWN_ENABLE))
#define ISCCIR_VEOUT_EN() ((mapi_syscfg_fetch::get_IPEnableType())&(_CCIR_VEOUT_ENABLE))
#define ISSCART_OUT_EN()  ((mapi_syscfg_fetch::get_IPEnableType())&(_SCART_OUT_ENABLE))
#define ISMM_FRC_EN()     ((mapi_syscfg_fetch::get_IPEnableType())&(_MM_FRC_ENABLE))
#define ISMARCOVISION_EN()((mapi_syscfg_fetch::get_IPEnableType())&(_MARCOVISION_ENABLE))
///BOARD_IP_ENABLE control - MVOP SW Control Mode
#define ISMVOP_SW_MODE_EN() ((mapi_syscfg_fetch::get_IPEnableType())&(_MVOP_SW_MODE_ENABLE))

#define MST_FEATURE_FLAG(Index) mapi_syscfg_fetch::GetMstarFeatureFlag(Index)
#define CUS_FEATURE_FLAG(Index) mapi_syscfg_fetch::GetCustomerFeatureFlag(Index)
#define TEMP_FEATURE_FLAG(Index) mapi_syscfg_fetch::GetTempFeatureFlag(Index)

void LoadConfig(void);
void DumpConfig(void);
extern MI_SYSCFG_Config_t *m_pstSysCfgBuf;

#endif
