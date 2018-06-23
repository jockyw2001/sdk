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

* Class : mapi_fetch
* File  : mapi_fetch.cpp
**********************************************************************/

// headers of itself
#include "systeminfo.h"
#include "mapi_syscfg_fetch.h"

#include "mapi_utility.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/syscall.h>

#define SYSINFO_DEBUG 0
#define SYS_INI_PATH_FILENAME          "/config/sys.ini"
#define BOOTLOGO_IN_MBOOT_CFG_PATH "/proc/cmdline"
#define BOOTLOGO_IN_MBOOT_STR "BOOTLOGO_IN_MBOOT"
#define MI_FLASH_ENV_VALUE_LEN_MAX 2048
#define CMDLINE_SIZE MI_FLASH_ENV_VALUE_LEN_MAX

#define ASSERT(e) if(!(e)){printf("assert at %s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);printf("\n");*(int*)0 = 0;}
#define MAPI_PRINTF(fmt, arg...) printf(fmt, ##arg)

//Syscfg Buf
static MI_SYSCFG_Config_t m_stSysCfgBuf;
MI_SYSCFG_Config_t *m_pstSysCfgBuf = &m_stSysCfgBuf;

void LoadConfig(void){
    SetSystemInfo();
    int fd = open("/proc/mi_modules/common/config_info", O_WRONLY);
    int off = 0, total = sizeof(m_stSysCfgBuf);
    while(off < total){
        off += write(fd, (void*)m_pstSysCfgBuf + off, sizeof(m_stSysCfgBuf)-off);
    }
    close(fd);
}

void DumpConfig(void){
    MI_SYSCFG_Config_t config;
    int fd = open("/proc/mi_modules/common/config_info", O_RDONLY);
    int off = 0, total = sizeof(config);
    while(off < total){
        off += read(fd, (void*)&config + off, sizeof(config)-off);
    }
    close(fd);
    printf("panel size:%d\n", config.m_PanelConf.nSize);
}

void SetInputMux(const MAPI_VIDEO_INPUTSRCTABLE* const pTemp, MI_U8 u8Num)
{
    m_pstSysCfgBuf->m_MuxConf.nSize = u8Num;

    if(u8Num != 0)
    {
       memcpy(m_pstSysCfgBuf->m_pU32MuxInfo, pTemp, sizeof(MAPI_VIDEO_INPUTSRCTABLE)*u8Num);
    }


#if (SYSINFO_DEBUG == 1)
    {
        int i;
        MAPI_VIDEO_INPUTSRCTABLE *ptr = (MAPI_VIDEO_INPUTSRCTABLE*)m_pstSysCfgBuf->m_pU32MuxInfo;
        if(ptr != NULL)
        {
            MAPI_PRINTF("------------------InputMuxCfg Start-2--------------------\n");
            MAPI_PRINTF("InputMuxTable Size = %d\n", m_pstSysCfgBuf->m_MuxConf.nSize);
            for(i = 0; i < (int)m_pstSysCfgBuf->m_MuxConf.nSize; i++)
                MAPI_PRINTF("MuxInfo[%d] = (%lu, %lu, %lu)\n", i, ptr[i].u32EnablePort, \
                       ptr[i].u32Port[0], ptr[i].u32Port[1]);
            MAPI_PRINTF("------------------InputMuxCfg End----------------------\n\n");
        }
    }
#endif

}

void SetHDMITxAnalogInfo(const HDMITx_Analog_Param_t* const pHDMITxAnalogInfo, MI_U8 uCfgTblNum)
{

    ASSERT(HDMITX_ANALOG_INFO_MAX_NUM > uCfgTblNum);

    memcpy(m_pstSysCfgBuf->m_pHdmiTxAnalogInfo, pHDMITxAnalogInfo, sizeof(HDMITx_Analog_Param_t)*uCfgTblNum);

#if (SYSINFO_DEBUG == 1)
    HDMITx_Analog_Param_t *pInfo = NULL;
    MAPI_PRINTF("\033[36m--------------------HDMITxAnalogInfo Start------------------\033[m\n");
    pInfo = (HDMITx_Analog_Param_t*)&(m_pstSysCfgBuf->m_pHdmiTxAnalogInfo[0].HDMITx_Attr);
    MAPI_PRINTF("\033[36mHDMITxAnalogInfo (HD): %x %x %x %x %x %x\033[m\n", pInfo->tm_txcurrent, pInfo->tm_pren2, pInfo->tm_precon,
                pInfo->tm_pren, pInfo->tm_tenpre, pInfo->tm_ten);
    pInfo = (HDMITx_Analog_Param_t*)&(m_pstSysCfgBuf->m_pHdmiTxAnalogInfo[1].HDMITx_Attr);
    MAPI_PRINTF("\033[36mHDMITxAnalogInfo (Deep HD): %x %x %x %x %x %x\033[m\n", pInfo->tm_txcurrent, pInfo->tm_pren2, pInfo->tm_precon,
                pInfo->tm_pren, pInfo->tm_tenpre, pInfo->tm_ten);
    MAPI_PRINTF("\033[36m--------------------HDMITxAnalogInfo End------------------\033[m\n");
#endif
}

EN_VD_CAPTURE_WINDOW_MODE GetVDCaptureWinMode()
{
    return m_pstSysCfgBuf->m_VDCaptureWinMode;
}

void SetVDCaptureWinMode(EN_VD_CAPTURE_WINDOW_MODE  enVDCaptureWinMode)
{
    m_pstSysCfgBuf->m_VDCaptureWinMode = enVDCaptureWinMode;

#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("VDCaptureWinMode = %s\n", m_pstSysCfgBuf->m_VDCaptureWinMode ? "Mode_1135" : "Mode1135_1P5");
#endif
}

void SetAudioInputMuxCfg(MI_U8 u8AudioMuxInfoSize, MI_U8 u8AudioPathInfoSize, MI_U8 u8AudioOutputTypeInfo,
                                            const AudioMux_t* const pAudioMuxInfo, const AudioMux_t* const pAudioPathInfo, const AudioOutputType_t * const pAudioOutputTypeInfo,
                                            const AudioDefualtInit_t* const pAudioDefaultInit, MI_U32 BOARD_AUDIO_MICROPHONE_PATH)
{
    MI_U8 nTablesize = 0, nTablesize1 = 0, nTablesize2 = 0;

    // Someone will use the MAPI_AUDIO_INPUT_SOURCE_TYPE enum to access the array,
    // so the array size need not less than enum number
    if (u8AudioMuxInfoSize < MAPI_AUDIO_SOURC_NUM)
    {
        MAPI_PRINTF("ERROR: Audio Mux Info Size is too small\n");
        ASSERT(0);
    }

    // Someone will use the MAPI_AUDIO_PATH_TYPE enum to access the array,
    // so the array size need not less than enum number
    if (u8AudioPathInfoSize < MAPI_AUDIO_PATH_NUM)
    {
        MAPI_PRINTF("ERROR: Audio Path Info Size is too small\n");
        ASSERT(0);
    }

    // Someone will use the MAPI_AUDIO_OUTPUT_TYPE enum to access the array,
    // so the array size need not less than enum number
    if (u8AudioOutputTypeInfo < MAPI_AUDIO_OUTPUT_NUM)
    {
        MAPI_PRINTF("ERROR: Audio Output Type Info Size is too small\n");
        ASSERT(0);
    }

    nTablesize = u8AudioMuxInfoSize;
    if(nTablesize != 0)
    {
        ASSERT(AUDIO_INPUT_MAX_NUM > nTablesize);
        ASSERT(pAudioMuxInfo);
        memcpy(m_pstSysCfgBuf->m_pAudioMuxInfo, pAudioMuxInfo, sizeof(AudioMux_t)*nTablesize);
    }

    nTablesize1 = u8AudioPathInfoSize;
    if(nTablesize1 != 0)
    {
        ASSERT(AUDIO_PATH_MAX_NUM > nTablesize1);
        ASSERT(pAudioPathInfo);
        memcpy(m_pstSysCfgBuf->m_pAudioPathInfo, pAudioPathInfo, sizeof(AudioPath_t)*nTablesize1);
    }


    nTablesize2 = u8AudioOutputTypeInfo;
    if(nTablesize2 != 0)
    {
        ASSERT(AUDIO_OUTPUT_MAX_NUM > nTablesize2);
        ASSERT(pAudioOutputTypeInfo);
        memcpy(m_pstSysCfgBuf->m_pAudioOutputTypeInfo, pAudioOutputTypeInfo, sizeof(AudioOutputType_t)*nTablesize2);
    }
    ASSERT(pAudioDefaultInit);
    memcpy(&m_pstSysCfgBuf->m_AudioDefaultInit, pAudioDefaultInit, sizeof(AudioDefualtInit_t));

    m_pstSysCfgBuf->m_AudioMicPath.u32Path = BOARD_AUDIO_MICROPHONE_PATH;

#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("-------------------AudioInputMuxCfg Start-------------------\n");
    if(m_pstSysCfgBuf->m_pAudioMuxInfo != NULL)
    {
        MAPI_PRINTF("AudioInputMux Size = %u\n", nTablesize);
        for(MI_U8 i = 0; i < nTablesize; i++)
        {
            MAPI_PRINTF("AudioMuxInfo[%d] = 0x%02lx\n", i, m_pstSysCfgBuf->m_pAudioMuxInfo[i].u32Port);
        }
    }

    MAPI_PRINTF("\n");
    if(m_pstSysCfgBuf->m_pAudioOutputTypeInfo != NULL)
    {
        MAPI_PRINTF("AudioPathInfo Size = %u\n", nTablesize1);
        for(MI_U8 i = 0; i < nTablesize1; i++)
        {
            MAPI_PRINTF("AudioPathInfo[%d] = 0x%02lx\n", i, m_pstSysCfgBuf->m_pAudioPathInfo[i].u32Path);
        }
    }

    MAPI_PRINTF("\n");
    if(m_pstSysCfgBuf->m_pAudioOutputTypeInfo != NULL)
    {
        MAPI_PRINTF("AudioOutputInfo Size = %u\n", nTablesize2);
        for(MI_U8 i = 0; i < nTablesize2; i++)
        {
            MAPI_PRINTF("AudioOutputTypeInfo[%d] = 0x%02lx\n", i, m_pstSysCfgBuf->m_pAudioOutputTypeInfo[i].u32Output);
        }
    }

    MAPI_PRINTF("Default audio Src = %d, Path = %d, Output = %d\n", m_AudioDeafultInit.eAudioSrc, m_AudioDeafultInit.eAudioPath, m_AudioDeafultInit.eAudioPath);
    MAPI_PRINTF("-------------------AudioInputMuxCfg End==-------------------\n\n");
#endif
}

const AudioMux_t* GetAudioInputMuxInfo()
{
    return m_pstSysCfgBuf->m_pAudioMuxInfo;
}

const AudioPath_t* GetAudioPathInfo()
{
    return m_pstSysCfgBuf->m_pAudioPathInfo;
}

MI_U32 GetAudioMicPathInfo()
{
    return m_pstSysCfgBuf->m_AudioMicPath.u32Path;
}

const AudioOutputType_t* GetAudioOutputTypeInfo()
{
    return m_pstSysCfgBuf->m_pAudioOutputTypeInfo;
}

void SetGammaTableCfg(GAMMA_TABLE_t* pGammaTableInfo[], MI_U8 u8TblCnt, MI_U8 u8Default)
{

    m_pstSysCfgBuf->m_GammaConf.nGammaTableSize = u8TblCnt;

    ASSERT(GAMMA_TABLE_MAX_NUM > m_pstSysCfgBuf->m_GammaConf.nGammaTableSize);
    ASSERT(pGammaTableInfo);

    MI_U8 i;
    for (i = 0; i < u8TblCnt; i++)
    {
        memcpy(&m_pstSysCfgBuf->m_pGammaTableInfo[i],  pGammaTableInfo[i], sizeof(GAMMA_TABLE_t));
    }

    m_pstSysCfgBuf->m_u8DefaultGammaIdx = u8Default;


#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("--------------------GammaTableCfg Start------------------\n");
    MAPI_PRINTF("Gamma Table size = %u\n", m_pstSysCfgBuf->m_GammaConf.nGammaTableSize);
    if(m_pstSysCfgBuf->m_pGammaTableInfo != NULL)
    {
        for(int j = 0 ; j < m_pstSysCfgBuf->m_GammaConf.nGammaTableSize ; j++)
        {
            for(MI_U16 i = 0; i < GammaArrayMAXSize; i++)
            {
                MAPI_PRINTF("GammaTableInfo_R[%d] = 0x%02x \n", i, m_pstSysCfgBuf->m_pGammaTableInfo[j].NormalGammaR[i]);
            }
            MAPI_PRINTF("\n");
            for(MI_U16 i = 0; i < GammaArrayMAXSize; i++)
            {
                MAPI_PRINTF("GammaTableInfo_G[%d] = 0x%02x  \n", i, m_pstSysCfgBuf->m_pGammaTableInfo[j].NormalGammaG[i]);
            }
            MAPI_PRINTF("\n");
            for(MI_U16 i = 0; i < GammaArrayMAXSize; i++)
            {
                MAPI_PRINTF("GammaTableInfo_B[%d] = 0x%02x  \n", i, m_pstSysCfgBuf->m_pGammaTableInfo[j].NormalGammaB[i]);
            }
            MAPI_PRINTF("\n");
        }
    }
    MAPI_PRINTF("--------------------GammaTableCfg End--------------------\n\n");
#endif
}

MI_U8 GetDefaultGammaIdx()
{
    return m_pstSysCfgBuf->m_u8DefaultGammaIdx;
}

void SetDLCTableCount(MI_U8 u8DlcTableCount)
{
    m_pstSysCfgBuf->m_u8DLCTableCount = u8DlcTableCount;
}

MI_U8 GetDLCTableCount()
{
    return m_pstSysCfgBuf->m_u8DLCTableCount;
}

void SetDLCInfo(MAPI_XC_DLC_init *Board_DLC_init[], MI_U8 u8TblCnt)
{
    MI_U8 i;
    MAPI_XC_DLC_init *p = m_pstSysCfgBuf->Board_DLC_init;

    ASSERT(u8TblCnt <= DLCTABLE_MAX_NUM);
    for (i = 0; i < u8TblCnt; i++)
    {
        memcpy((p+i), Board_DLC_init[i], sizeof(MAPI_XC_DLC_init));
    }
}

void GetDLCInfo(MAPI_XC_DLC_init *Board_DLC_init, MI_U8 u8TblCnt)
{
    ASSERT(u8TblCnt <= DLCTABLE_MAX_NUM);
    memcpy(Board_DLC_init, &(m_pstSysCfgBuf->Board_DLC_init[u8TblCnt]), sizeof(MAPI_XC_DLC_init));
}

void SetColorMatrix(MAPI_COLOR_MATRIX *pMatrx)
{
    memcpy(&m_pstSysCfgBuf->m_stMatrx, pMatrx, sizeof(MAPI_COLOR_MATRIX));
}

void SetPQPathName(char* ps8ini_strval, MI_S32 len)
{

    ASSERT(PATH_MAX_NUM > len);
    memset(m_pstSysCfgBuf->m_PQPathName , 0 , len + 1);
    memcpy(m_pstSysCfgBuf->m_PQPathName ,(void*)ps8ini_strval, len);
}

MI_U8 * GetPQPathName(void)
{
    return m_pstSysCfgBuf->m_PQPathName;
}


GAMMA_TABLE_t* GetGammaTableInfo(MI_U8 u8Idx)
{
    return &m_pstSysCfgBuf->m_pGammaTableInfo[u8Idx];
}

void SetPanelInfoCfg(PanelInfo_t* pstPanelInfo, int iPanelSize, MI_U16 u16LVDS_Output_type, PanelBacklightPWMInfo* ptPanelBacklightPWMInfo)
{

    ASSERT((0 <= iPanelSize) && (PANEL_MAX_NUM >= iPanelSize));

    m_pstSysCfgBuf->m_PanelConf.nSize = iPanelSize;
    m_pstSysCfgBuf->m_u16LVDS_Output_type = u16LVDS_Output_type;

    if(0 < m_pstSysCfgBuf->m_PanelConf.nSize)
    {
        MAPI_PRINTF("************************Panel Table Start*****************************\n");
        MAPI_PRINTF("mapi_base::m_PanelConf.nSize = %d \n", m_pstSysCfgBuf->m_PanelConf.nSize);

        int i;
        for(i = 0 ; i < m_pstSysCfgBuf->m_PanelConf.nSize ; i++)
        {
            memcpy(&(m_pstSysCfgBuf->m_pPanelInfo[i]), &(pstPanelInfo[i]), sizeof(PanelInfo_t));
            MAPI_PRINTF("index %d name %s timing %d hdmitx %d\n", i, m_pstSysCfgBuf->m_pPanelInfo[i].PanelAttr.pPanelName,
                    m_pstSysCfgBuf->m_pPanelInfo[i].eTiming, m_pstSysCfgBuf->m_pPanelInfo[i].bHdmiTx);
        }
        MAPI_PRINTF("************************Panel Table End*******************************\n");

        int i32Len = strlen(pstPanelInfo[0].PanelAttr.pPanelName);
        ASSERT(NAME_MAX_NUM > i32Len);
        strcpy(m_pstSysCfgBuf->m_strPanelName, pstPanelInfo[0].PanelAttr.pPanelName);
        m_pstSysCfgBuf->m_pCurrentPanelInfo = m_pstSysCfgBuf->m_pPanelInfo;
        MAPI_PRINTF("m_strPanelName = %s\n", m_pstSysCfgBuf->m_strPanelName);
    }
    else
    {
        MAPI_PRINTF("war panel size if zero\n");
        ASSERT(0);
    }

    ASSERT(ptPanelBacklightPWMInfo);
    memcpy(&(m_pstSysCfgBuf->m_PanelBacklightPWMInfo), ptPanelBacklightPWMInfo, sizeof(PanelBacklightPWMInfo));

#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("------------------SetPanelBlacklightPWM Start--------------------\n");
    MAPI_PRINTF(" PWMPort = %d,\n u32PeriodPWM = %ld,\n u32DutyPWM = %ld,\n u16DivPWM = %d,\n bPolPWM = %d,\n u8MaxPWMvalue = %d,\n u8MinPWMvalue = %d,\n ", \
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u8PWMPort,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u32PeriodPWM,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u32DutyPWM,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u16DivPWM,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.bPolPWM,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u16MaxPWMvalue,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.u16MinPWMvalue,
                m_pstSysCfgBuf->m_PanelBacklightPWMInfo.bBakclightFreq2Vfreq);
    MAPI_PRINTF("------------------SetPanelBlacklightPWM End----------------------\n\n");
#endif


#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("--------------------PanelCfg Start-------------------\n");
    MAPI_PRINTF("Current Panel = %s\n", pName);
    MAPI_PRINTF("Panel table size = %d\n", m_pstSysCfgBuf->m_PanelConf.nSize);
    for(int i = 0; i < m_pstSysCfgBuf->m_PanelConf.nSize ; i++)
    {
        MAPI_PanelType *ptr = &(m_pstSysCfgBuf->m_pPanelInfo[i].PanelAttr);
        MAPI_PRINTF("Table Index[%d] = (%s, %u, %d, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, ", i, ptr->m_pPanelName, ptr->bPanelDither, ptr->m_ePanelLinkType\
               , ptr->m_bPanelDualPort, ptr->m_bPanelSwapPort, ptr->m_bPanelSwapOdd_ML, ptr->m_bPanelSwapEven_ML, \
               ptr->m_bPanelSwapOdd_RB, ptr->m_bPanelSwapEven_RB, ptr->m_bPanelSwapLVDS_POL, ptr->m_bPanelSwapLVDS_CH, \
               ptr->m_bPanelPDP10BIT, ptr->m_bPanelLVDS_TI_MODE, ptr->m_ucPanelDCLKDelay, ptr->m_bPanelInvDCLK, \
               ptr->m_bPanelInvDE, ptr->m_bPanelInvHSync, ptr->m_bPanelInvVSync);
        MAPI_PRINTF("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, %u, %d, %d, %u, %u, %u, %u, %u, 0x%lx, 0x%lx, %d)\n", \
               ptr->m_ucPanelDCKLCurrent, ptr->m_ucPanelDECurrent, ptr->m_ucPanelODDDataCurrent, \
               ptr->m_ucPanelEvenDataCurrent, ptr->m_wPanelOnTiming1, ptr->m_wPanelOnTiming2, ptr->m_wPanelOffTiming1, ptr->m_wPanelOffTiming2, \
               ptr->m_ucPanelHSyncWidth, ptr->m_ucPanelHSyncBackPorch, ptr->m_ucPanelVSyncWidth, ptr->m_ucPanelVBackPorch, \
               ptr->m_wPanelHStart, ptr->m_wPanelVStart, ptr->m_wPanelWidth, ptr->m_wPanelHeight, \
               ptr->m_wPanelMaxHTotal, ptr->m_wPanelHTotal, ptr->m_wPanelMinHTotal, \
               ptr->m_wPanelMaxVTotal, ptr->m_wPanelVTotal, ptr->m_wPanelMinVTotal, \
               ptr->m_dwPanelMaxDCLK, ptr->m_dwPanelDCLK, ptr->m_dwPanelMinDCLK, \
               ptr->m_wSpreadSpectrumStep, ptr->m_wSpreadSpectrumSpan, \
               ptr->m_ucDimmingCtl, ptr->m_ucMaxPWMVal, ptr->m_ucMinPWMVal, \
               ptr->m_bPanelDeinterMode, ptr->m_ucPanelAspectRatio, \
               ptr->m_u16LVDSTxSwapValue, ptr->m_ucTiBitMode, ptr->m_ucOutputFormatBitMode, \
               ptr->m_bPanelSwapOdd_RG, ptr->m_bPanelSwapEven_RG, ptr->m_bPanelSwapOdd_GB, ptr->m_bPanelSwapEven_GB, \
               ptr->m_bPanelDoubleClk, ptr->m_dwPanelMaxSET, ptr->m_dwPanelMinSET, ptr->m_ucOutTimingMode);
            MAPI_PRINTF("Hdr: %u %u %u %u %u %u %u %u \n",ptr->u16tRx, ptr->u16tRy, ptr->u16tGx, ptr->u16tGy, ptr->u16tBx, ptr->u16tBy, ptr->u16tWx, ptr->u16tWy);
    }
    MAPI_PRINTF("--------------------PanelCfg End---------------------\n\n");
#endif
}

const PanelBacklightPWMInfo* GetPanelBacklightPWMInfo()
{
    return &m_pstSysCfgBuf->m_PanelBacklightPWMInfo;
}

MI_BOOL SetPanelModPvddInfo(MI_BOOL bEnablePanelModPvddCfg, MI_BOOL bPanelModPvddType)
{
    m_pstSysCfgBuf->m_PanelModPvddPowerInfo.bEnabled=bEnablePanelModPvddCfg;
    m_pstSysCfgBuf->m_PanelModPvddPowerInfo.bPanelModPvddPowerType=bPanelModPvddType;
    return TRUE;
}

const PanelModPvddPowerInfo* GetPanelModPvddPowerInfo()
{
    return &m_pstSysCfgBuf->m_PanelModPvddPowerInfo;
}

MI_U16 GetLVDSOutputType()
{
    return m_pstSysCfgBuf->m_u16LVDS_Output_type;
}

void SetPQAutoNRParam(const MAPI_AUTO_NR_INIT_PARAM* const pParam)
{
    ASSERT(pParam);
    memcpy(&m_pstSysCfgBuf->m_AutoNrParam, pParam, sizeof(MAPI_AUTO_NR_INIT_PARAM));
}

MAPI_AUTO_NR_INIT_PARAM* GetPQAutoNRParam()
{
    return &m_pstSysCfgBuf->m_AutoNrParam;
}

void SetSAWType(SawArchitecture SawType)
{
    if(SAW_NUMS>SawType)
    {
        m_pstSysCfgBuf->enSawType = SawType;
    }
    else
    {
        ASSERT(0);
    }
}

SawArchitecture GetSAWType()
{
    return m_pstSysCfgBuf->enSawType;
}



void SetCustomerPQCfg(char * pPQCustomerBinFilePath, MAPI_PQ_WIN enWinType)
{
    if(pPQCustomerBinFilePath){
        int i32Len = strlen(pPQCustomerBinFilePath);
        ASSERT(PATH_MAX_NUM > i32Len);

        if ( enWinType == MAPI_PQ_MAIN_WINDOW )
        {
            strcpy(m_pstSysCfgBuf->pMainPQPath,  pPQCustomerBinFilePath);
        }
        else if ( enWinType == MAPI_PQ_SUB_WINDOW )
        {
            strcpy(m_pstSysCfgBuf->pSubPQPath,  pPQCustomerBinFilePath);
        }
    }
}

void SetVideoMirrorCfg(MI_BOOL bMirrorEnable,MI_U8 u8MirrorMode)
{
    m_pstSysCfgBuf->m_bMirrorVideo=bMirrorEnable;
    m_pstSysCfgBuf->m_u8MirrorMode= u8MirrorMode;
}

MI_U8 getMirrorVideoMode()
{
    return m_pstSysCfgBuf->m_u8MirrorMode;
}

MI_BOOL GetMirrorVideoFlag()
{
    return m_pstSysCfgBuf->m_bMirrorVideo;
}

void SetFreerunCfg(MI_BOOL b3D, MI_BOOL bEnable)
{
    m_pstSysCfgBuf->m_bEnableFreerun[b3D] = bEnable;
}

MI_BOOL GetFreerunFlag(MI_BOOL b3D)
{
    return m_pstSysCfgBuf->m_bEnableFreerun[b3D];
}

void SetFBLModeThreshold(MI_U32 u32Threshold)
{
    m_pstSysCfgBuf->m_u32FBLThreshold = u32Threshold;
}

MI_U32 GetFBLModeThreshold()
{
    return m_pstSysCfgBuf->m_u32FBLThreshold;
}

void SetVideoInfoCfg(VideoInfo_t enVideoNum, MI_U8 u8ResNum, const ST_MAPI_VIDEO_WINDOW_INFO* pstVideoInfo, MI_U32 u32HotPlugInverse, MI_BOOL bPqBypassSupported, MI_U32 u32Hdmi5vDetectGpioSelect)
{
    int i = 0, j = 0;

    ASSERT(WINDOW_TIMMING_NUM > u8ResNum);
    ASSERT((WINDOW_TYPE_NUM > enVideoNum) && (MAXRESOLUTIONSIZE > enVideoNum));
    int i32MaxAr = E_AR_MAX;
    ASSERT(WINDOW_TIMMING_NUM >= i32MaxAr);

    m_pstSysCfgBuf->m_ResoSize[enVideoNum].nResolutionSize = u8ResNum;

    for(i = 0 ; i < u8ResNum ; i++)
    {
        for(j = 0; j < E_AR_MAX ; j++)
        {
            ASSERT(pstVideoInfo);
            memcpy((char*)&(m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j]), (const char*)pstVideoInfo, sizeof(ST_MAPI_VIDEO_WINDOW_INFO));
            pstVideoInfo++;
        }
    }

#if (SYSINFO_DEBUG == 1)
    MAPI_PRINTF("------------------VideoInfoCfg Start--------------------\n");

    switch(enVideoNum)
    {
        case E_DTV:
            MAPI_PRINTF("DTV Video Info Table\n");
            break;
        default:
            break;
    }

    for(i = 0 ; i < m_pstSysCfgBuf->m_ResoSize[enVideoNum].nResolutionSize ; i++)
    {
        MAPI_PRINTF("//Resolution[%d]:\n", i);
        MAPI_PRINTF("//******************\n");
        for(j = 0; j < E_AR_MAX ; j++)
        {
            MAPI_PRINTF("(0x%02x, 0x%02x, %u, %u, %u, %u)//",
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u16H_CapStart,
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u16V_CapStart,
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u8HCrop_Left,
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u8HCrop_Right,
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u8VCrop_Up,
                                m_pstSysCfgBuf->m_pVideoWinInfo[enVideoNum][i][j].u8VCrop_Down);
            switch(j)
            {
                case mapi_video_datatype::E_AR_DEFAULT:
                    MAPI_PRINTF("Default \n");
                    break;
                case mapi_video_datatype::E_AR_16x9:
                    MAPI_PRINTF("16:9 \n");
                    break;
                case mapi_video_datatype::E_AR_4x3:
                    MAPI_PRINTF("4:3 \n");
                    break;
                case mapi_video_datatype::E_AR_AUTO:
                    MAPI_PRINTF("Auto \n");
                    break;
                case mapi_video_datatype::E_AR_Panorama:
                    MAPI_PRINTF("Panorama \n");
                    break;
                case mapi_video_datatype::E_AR_JustScan:
                    MAPI_PRINTF("JustScan \n");
                    break;
                case mapi_video_datatype::E_AR_Zoom1:
                    MAPI_PRINTF("Zoom1 \n");
                    break;
                case mapi_video_datatype::E_AR_Zoom2:
                    MAPI_PRINTF("Zoom2 \n");
                    break;
                case mapi_video_datatype::E_AR_4x3_PanScan:
                    MAPI_PRINTF("4x3_PanScan \n");
                    break;
                case mapi_video_datatype::E_AR_4x3_LetterBox:
                    MAPI_PRINTF("4x3_LetterBox \n");
                    break;
                case mapi_video_datatype::E_AR_16x9_PillarBox:
                    MAPI_PRINTF("16x9_PillarBox \n");
                    break;
                default:
                    break;
            }

        }
        MAPI_PRINTF("//******************\n\n");
    }
    MAPI_PRINTF("======================================================\n");
#endif

}

const ResolutionInfoSize* GetResolutionInfo()
{
    return m_pstSysCfgBuf->m_ResoSize;
}

MI_BOOL GetPqBypassSupported()
{
    return m_pstSysCfgBuf->m_bPqBypassSupported;
}

const ST_MAPI_VIDEO_WINDOW_INFO** GetVideoWinInfo(VideoInfo_t eVideoInfo)
{

    MAPI_PRINTF("eVideoInfo = %d\n", eVideoInfo);
    ASSERT(MAXRESOLUTIONSIZE > eVideoInfo);
    ASSERT(0 != m_pstSysCfgBuf->m_ResoSize[eVideoInfo].nResolutionSize);

    return (const ST_MAPI_VIDEO_WINDOW_INFO**)(&(m_pstSysCfgBuf->m_pVideoWinInfo[eVideoInfo]));
}

MI_BOOL SetUseCustomerScreenMuteColorFlag(MI_BOOL bUseCustomerScreenMuteColor)
{
    m_pstSysCfgBuf->m_bUseCustomerScreenMuteColor = bUseCustomerScreenMuteColor;
    return TRUE;
}

void SeCustomerScreenMuteColorType(MI_U8 u8ScreenMuteColorType)
{
    if (u8ScreenMuteColorType >=  E_SCREEN_MUTE_NUMBER)
    {
        MAPI_PRINTF("Set customer screen mute color type error, change to default color type\n");
        m_pstSysCfgBuf->m_u8CustomerScreenMuteColorType = E_SCREEN_MUTE_BLACK;
    }
    else
    {
        m_pstSysCfgBuf->m_u8CustomerScreenMuteColorType = u8ScreenMuteColorType;
    }
}

void SetCustomerFrameColorType(MI_U8 u8FrameColorR, MI_U8 u8FrameColorG, MI_U8 u8FrameColorB)
{
    m_pstSysCfgBuf->m_u8CustomerFrameColorType[0] = u8FrameColorR;
    m_pstSysCfgBuf->m_u8CustomerFrameColorType[1] = u8FrameColorG;
    m_pstSysCfgBuf->m_u8CustomerFrameColorType[2] = u8FrameColorB;
}

void SetAMPBinPath(char *path)
{
    if(NULL != path)
    {
        int i32Len = strlen(path);
        ASSERT(PATH_MAX_NUM > i32Len);
        strcpy(m_pstSysCfgBuf->pAmpInitBinPath, path);
    }
}
#if 0
void ModifyI2CDevCfg(MI_U32 u32gID, MI_U8 u8i2c_bus, MI_U8 u8slave_id)
{
    if(m_pstSysCfgBuf->m_i2cconfig.n_i2c_device != 0)
    {
        for(MI_U32 i = 0; i < m_pstSysCfgBuf->m_i2cconfig.n_i2c_device; i++)
        {
            if(i == u32gID)
            {
                MAPI_PRINTF("Orig I2C Device[%d] ID = %u SlaveId = %u, Bus = %u\n", i, \
                m_pstSysCfgBuf->m_pI2CDevices[i].gID, m_pstSysCfgBuf->m_pI2CDevices[i].slave_id, \
                m_pstSysCfgBuf->m_pI2CDevices[i].i2c_bus);

                MAPI_PRINTF("==>\n");
                m_pstSysCfgBuf->m_pI2CDevices[i].slave_id = u8slave_id;
                m_pstSysCfgBuf->m_pI2CDevices[i].i2c_bus = u8i2c_bus;

                MAPI_PRINTF("Orig I2C Device[%d] ID = %u SlaveId = %u, Bus = %u\n", i, \
                m_pstSysCfgBuf->m_pI2CDevices[i].gID, m_pstSysCfgBuf->m_pI2CDevices[i].slave_id, \
                m_pstSysCfgBuf->m_pI2CDevices[i].i2c_bus);
            }
        }
    }
}
#endif

void SetCurrentMode(EN_CURRENT_MODE_TYPE enCurrentMode)
{
    m_pstSysCfgBuf->m_CurrentMode = enCurrentMode;
    return ;
}

void SetBwCusMode(MI_BOOL bCusModeEn)
{
    m_pstSysCfgBuf->m_bBwCusMode = bCusModeEn;
    return;
}

#if 0
void SetVb1ChannelOrder(EN_MAPI_VB1_CHANNELORDER_TYPE cotype, MI_U16 u16Order[4])
{
    switch(cotype)
    {
        case E_MAPI_VB1_CHANNELORDER_8V:
            memcpy(m_pstSysCfgBuf->m_u16Vb18VChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb18VChannelOrder));
            break;
        case E_MAPI_VB1_CHANNELORDER_4V:
            memcpy(m_pstSysCfgBuf->m_u16Vb14VChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb14VChannelOrder));
            break;
        case E_MAPI_VB1_CHANNELORDER_2V:
            memcpy(m_pstSysCfgBuf->m_u16Vb12VChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb12VChannelOrder));
            break;
        case E_MAPI_VB1_CHANNELORDER_1V:
            memcpy(m_pstSysCfgBuf->m_u16Vb11VChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb11VChannelOrder));
            break;
        case E_MAPI_VB1_CHANNELORDER_4O:
            memcpy(m_pstSysCfgBuf->m_u16Vb14OChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb14OChannelOrder));
            break;
        case E_MAPI_VB1_CHANNELORDER_2O:
            memcpy(m_pstSysCfgBuf->m_u16Vb12OChannelOrder, u16Order, sizeof(m_pstSysCfgBuf->m_u16Vb12OChannelOrder));
            break;
        default:
            MAPI_PRINTF("SetVb1ChannelOrder fail,cotype = %d\n", cotype);
            break;
    }
}
MI_BOOL GetVB1ChannelOrder(mapi_display_timing_datatype::EN_DISPLAYTIMING_RES_TYPE enVideo, mapi_display_timing_datatype::EN_DISPLAYTIMING_RES_TYPE enOsd, MI_U16 u16Order[4])
{
    memset(u16Order, 0, sizeof(MI_U16)*4);
    const PanelInfo_t *pPanelInfo = GetPanelInfo(0, MAPI_FALSE);
    //pPanelInfo = GetPanelInfo(0, MAPI_FALSE);
    //pPanelInfo = GetCurrentPanelInfo();
    if (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_DEFAULT)
    {
        enVideo = pPanelInfo->eTiming;
    }

    if ((enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_480I) || (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_480P))
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[3];
    }
    else if ((enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_576I) || (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_576P))
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb11VChannelOrder[3];
    }
    else if ((enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_720P_50) || (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_720P_60))
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[3];
    }
    else if (enVideo >= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_1080P_24 && enVideo <= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_2205P_24)/// 2k1k
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb12VChannelOrder[3];
    }
    else if (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_24 || (enVideo >= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_24 && enVideo <= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_30))
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[3];
    }
    else if (enVideo >= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_50 && enVideo <= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_60)
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb18VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb18VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb18VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb18VChannelOrder[3];
    }
    else if (enVideo == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4096P_24)
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb14VChannelOrder[3];
    }

    if (enOsd >= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_1080P_24 && enOsd <= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_1080P_60)
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb12OChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb12OChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb12OChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb12OChannelOrder[3];
    }
    else if (enOsd == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_24 || (enOsd >= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_24 && enOsd <= mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4K2KP_30))
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[3];
    }
    else if (enOsd == mapi_display_timing_datatype::DISPLAYTIMING_DACOUT_4096P_24)
    {
        u16Order[0] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[0];
        u16Order[1] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[1];
        u16Order[2] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[2];
        u16Order[3] |= m_pstSysCfgBuf->m_u16Vb14OChannelOrder[3];
    }

    if ((u16Order[0] == 0) && (u16Order[1] == 0) && (u16Order[2] == 0) && (u16Order[3] == 0))
    {
        return FALSE;
    }

    return TRUE;
}
#endif
