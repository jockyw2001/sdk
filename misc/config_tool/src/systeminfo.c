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

// headers of standard C libs
typedef unsigned char MS_U8;
typedef unsigned short MS_U16;
typedef unsigned int MS_U32;
typedef short MS_S16;
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include "dirent.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "mapi_utility.h"
#include "mapi_syscfg_fetch.h"
#include "systeminfo.h"
#include "board.h"

//#include "OverScan.h"
IniInfo SysIniBlock={"","", 1, {0}, 0, 0, 0xFF,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0x00,0x00,0x00,0,0,0,0,0,0,"",0,0,0,0,0,{0},{0},{0}, {{0}}, {{0}}, {{0}}, 0};
#define MI_PRINT(fmt, arg...) printf(fmt, ##arg)
#define SYSTEM_INFO_ERR(fmt, arg...)         MI_PRINT(fmt, ##arg)
#define SYSTEM_INFO_DBG(fmt, arg...)         MI_PRINT(fmt, ##arg)
#define SYSTEM_INFO_IFO(fmt, arg...)         MI_PRINT(fmt, ##arg)
#define SYSTEM_INFO_FLOW(fmt, arg...)        MI_PRINT(fmt, ##arg)

#define DLC_INI_UPDATE_PATH "/config/DLC/DLC.ini"
#define COLORMATRIX_INI_UPDATE_PATH "/config/ColorMatrix/ColorMatrix.ini"
#define I2C_BIN_PATH "/config/I2C_BIN/I2C.bin"
#define MOUNT_FOLDER "/Customer/UpdatePQSetting"
#define UPDATE_PQ_LOG "/Customer/UpdatePQSetting/updatePQ.log"
#define DLC_UPDATE_SOURCE "/Customer/UpdatePQSetting/DLC.ini"
#define COLORMATRIX_UPDATE_SOURCE "/Customer/UpdatePQSetting/ColorMatrix.ini"
#define GAMMA0_UPDATE_SOURCE "/Customer/UpdatePQSetting/gamma0.ini"
#define BANDWIDTH_REG_TABLE_FILE "Bandwidth_RegTable.bin"
#define PQ_MAIN_FILE "Main.bin"
#define PQ_MAIN_TEXT_FILE "Main_Text.bin"
#define PQ_MAIN_EX_FILE "Main_Ex.bin"
#define PQ_MAIN_EX_TEXT_FILE "Main_Ex_Text.bin"
#define PQ_SUB_FILE "Sub.bin"
#define PQ_SUB_TEXT_FILE "Sub_Text.bin"
#define PQ_SUB_EX_FILE "Sub_Ex.bin"
#define PQ_SUB_EX_TEXT_FILE "Sub_Ex_Text.bin"
#define UPDATE_PQ_FILES_COUNT 12

#define CusBackupModelMode  1
#define CusBackupPanelMode  2
#define CusBackupDLCMode  3
#define CusBackupColorMatrixMode  4
#define CusBackupPcMode  5

#define PANEL_NAME_LEN  32
#define PROC_CMD_LINE               "/proc/cmdline"
#define CMD_LINE_SIZE 4096
#define MI_FLASH_ENV_VALUE_LEN_MAX CMD_LINE_SIZE
#define ASSERT(e) if(!(e)){printf("assert at %s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);printf("\n");*(int*)0 = 0;}

static const char *m_pModelName = "/config/model/Customer_1.ini";
static dictionary *m_pCustomerini;
static dictionary *m_pBoardini;
static dictionary *m_pDCLIni;
static dictionary *m_pMatrixIni;
static dictionary *m_pPanelini;
static MI_U16 m_u16PanelCount;

typedef MI_BOOL BOOL;

typedef struct
{
    bool bHdmiTx;
    EN_DISPLAYTIMING_RES_TYPE enTiming;
    char cName[32];
    char cPath[128];
} PanelPath;

PanelPath g_PanelPath[] =
{
// panel
    {false,DISPLAYTIMING_DACOUT_480I, "panel:m_p480_60IPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_480P, "panel:m_p480_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_576I, "panel:m_p576_50IPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_576P, "panel:m_p576_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_720P_50, "panel:m_p720_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_720P_60, "panel:m_p720_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080P_24, "panel:m_p2K1K_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080P_25, "panel:m_p2K1K_25PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080P_30, "panel:m_p2K1K_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080I_50, "panel:m_p2K1K_50IPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080P_50, "panel:m_p2K1K_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080I_60, "panel:m_p2K1K_60IPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1080P_60, "panel:m_p2K1K_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1440P_50, "panel:m_p1440_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1470P_24, "panel:m_p1470_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1470P_30, "panel:m_p1470_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_1470P_60, "panel:m_p1470_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_2205P_24, "panel:m_p2205_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_2K2KP_24, "panel:m_p2K2K_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_2K2KP_25, "panel:m_p2K2K_25PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_2K2KP_30, "panel:m_p2K2K_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_2K2KP_60, "panel:m_p2K2K_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K540P_240, "panel:m_p4K540_240PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K1KP_30, "panel:m_p4K1K_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K1KP_60, "panel:m_p4K1K_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K1KP_120, "panel:m_p4K1K_120PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_24, "panel:m_p4K2K_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_25, "panel:m_p4K2K_25PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_30, "panel:m_p4K2K_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_50, "panel:m_p4K2K_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_60, "panel:m_p4K2K_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4096P_24, "panel:m_p4096_24PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4096P_25, "panel:m_p4096_25PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4096P_30, "panel:m_p4096_30PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4096P_50, "panel:m_p4096_50PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4096P_60, "panel:m_p4096_60PPanelName",{0}},
    {false,DISPLAYTIMING_DACOUT_4K2KP_120, "panel:m_p4K2K_120PPanelName",{0}},
    {false,DISPLAYTIMING_VGAOUT_1024x768P_60, "panel:m_p768_60PPanelName",{0}},
    {false,DISPLAYTIMING_VGAOUT_1280x1024P_60, "panel:m_p1024_60PPanelName",{0}},
    {false,DISPLAYTIMING_VGAOUT_1440x900P_60, "panel:m_p900_60PPanelName",{0}},
    {false,DISPLAYTIMING_VGAOUT_1600x1200P_60, "panel:m_p1200_60PPanelName",{0}},
// hdmiTx
    {true,DISPLAYTIMING_DACOUT_480I, "hdmitx:m_p480_60IPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_480P, "hdmitx:m_p480_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_576I, "hdmitx:m_p576_50IPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_576P, "hdmitx:m_p576_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_720P_50, "hdmitx:m_p720_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_720P_60, "hdmitx:m_p720_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080P_24, "hdmitx:m_p2K1K_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080P_25, "hdmitx:m_p2K1K_25PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080P_30, "hdmitx:m_p2K1K_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080I_50, "hdmitx:m_p2K1K_50IPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080P_50, "hdmitx:m_p2K1K_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080I_60, "hdmitx:m_p2K1K_60IPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1080P_60, "hdmitx:m_p2K1K_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1440P_50, "hdmitx:m_p1440_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1470P_24, "hdmitx:m_p1470_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1470P_30, "hdmitx:m_p1470_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_1470P_60, "hdmitx:m_p1470_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_2205P_24, "hdmitx:m_p2205_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_2K2KP_24, "hdmitx:m_p2K2K_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_2K2KP_25, "hdmitx:m_p2K2K_25PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_2K2KP_30, "hdmitx:m_p2K2K_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_2K2KP_60, "hdmitx:m_p2K2K_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K540P_240, "hdmitx:m_p4K540_240PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K1KP_30, "hdmitx:m_p4K1K_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K1KP_60, "hdmitx:m_p4K1K_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K1KP_120, "hdmitx:m_p4K1K_120PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K2KP_24, "hdmitx:m_p4K2K_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K2KP_25, "hdmitx:m_p4K2K_25PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K2KP_30, "hdmitx:m_p4K2K_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K2KP_50, "hdmitx:m_p4K2K_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4K2KP_60, "hdmitx:m_p4K2K_60PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4096P_24, "hdmitx:m_p4096_24PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4096P_25, "hdmitx:m_p4096_25PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4096P_30, "hdmitx:m_p4096_30PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4096P_50, "hdmitx:m_p4096_50PPanelName",{0}},
    {true,DISPLAYTIMING_DACOUT_4096P_60, "hdmitx:m_p4096_60PPanelName",{0}},
};

inline ssize_t mmc_read(int fd, unsigned char *buf, size_t count,int offset)
{
    ssize_t _n =0;
    ssize_t n;

    do {
        n = pread(fd, buf+_n, count,offset+_n);
    } while (n < 0 && errno == EINTR);
    _n += n;

    return _n;
}

static MI_BOOL GetPanelNameFromCmdline(char* pcName)
{
    #define KEYWORD_TO_DAC_4096P60 "resolution=DACOUT_4096X2160P_60"
    #define KEYWORD_TO_DAC_4096P50 "resolution=DACOUT_4096X2160P_50"
    #define KEYWORD_TO_DAC_4096P30 "resolution=DACOUT_4096X2160P_30"
    #define KEYWORD_TO_DAC_4096P25 "resolution=DACOUT_4096X2160P_25"
    #define KEYWORD_TO_DAC_4096P24 "resolution=DACOUT_4096X2160P_24"
    #define KEYWORD_TO_DAC_4K2KP60 "resolution=DACOUT_4K2KP_60"
    #define KEYWORD_TO_DAC_4K2KP50 "resolution=DACOUT_4K2KP_50"
    #define KEYWORD_TO_DAC_4K2KP30 "resolution=DACOUT_4K2KP_30"
    #define KEYWORD_TO_DAC_4K2KP25 "resolution=DACOUT_4K2KP_25"
    #define KEYWORD_TO_DAC_4K2KP24 "resolution=DACOUT_4K2KP_24"
    #define KEYWORD_TO_DAC_1080P24 "resolution=DACOUT_1080P_24"
    #define KEYWORD_TO_DAC_1080P25 "resolution=DACOUT_1080P_25"
    #define KEYWORD_TO_DAC_1080P30 "resolution=DACOUT_1080P_30"
    #define KEYWORD_TO_DAC_1080P50 "resolution=DACOUT_1080P_50"
    #define KEYWORD_TO_DAC_1080P60 "resolution=DACOUT_1080P_60"
    #define KEYWORD_TO_DAC_1080I50 "resolution=DACOUT_1080I_50"
    #define KEYWORD_TO_DAC_1080I60 "resolution=DACOUT_1080I_60"
    #define KEYWORD_TO_DAC_720P50 "resolution=DACOUT_720P_50"
    #define KEYWORD_TO_DAC_720P60 "resolution=DACOUT_720P_60"
    #define KEYWORD_TO_DAC_576P50 "resolution=DACOUT_576P_50"
    #define KEYWORD_TO_DAC_576I50 "resolution=DACOUT_576I_50"
    #define KEYWORD_TO_DAC_480P60 "resolution=DACOUT_480P_60"
    #define KEYWORD_TO_DAC_480I60 "resolution=DACOUT_480I_60"
    #define KEYWORD_TO_DAC_768P60 "resolution=DACOUT_1024X768P_60"
    #define KEYWORD_TO_DAC_1024P60 "resolution=DACOUT_1280X1024P_60"
    #define KEYWORD_TO_DAC_900P60 "resolution=DACOUT_1440X900P_60"
    #define KEYWORD_TO_DAC_1200P60 "resolution=DACOUT_1600X1200P_60"


    #define KEYWORD_TO_HDMITX_4K2KP30 "resolution=HDMITX_RES_4K2Kp_30Hz"
    #define KEYWORD_TO_HDMITX_4K1KP60 "resolution=HDMITX_RES_4K1Kp_60Hz"
    #define KEYWORD_TO_HDMITX_4K2KP25 "resolution=HDMITX_RES_4K2Kp_25Hz"
    #define KEYWORD_TO_HDMITX_1080P50 "resolution=HDMITX_RES_1920x1080p_50Hz"
    #define KEYWORD_TO_HDMITX_1080P60 "resolution=HDMITX_RES_1920x1080p_60Hz"
    #define KEYWORD_TO_HDMITX_1080I50 "resolution=HDMITX_RES_1920x1080i_50Hz"
    #define KEYWORD_TO_HDMITX_1080I60 "resolution=HDMITX_RES_1920x1080i_60Hz"
    #define KEYWORD_TO_HDMITX_720P50 "resolution=HDMITX_RES_1280x720p_50Hz"
    #define KEYWORD_TO_HDMITX_720P60 "resolution=HDMITX_RES_1280x720p_60Hz"
    #define KEYWORD_TO_HDMITX_576P50 "resolution=HDMITX_RES_720x576p"
    #define KEYWORD_TO_HDMITX_576I50 "resolution=HDMITX_RES_720x576i"
    #define KEYWORD_TO_HDMITX_480P60 "resolution=HDMITX_RES_720x480p"
    #define KEYWORD_TO_HDMITX_480I60 "resolution=HDMITX_RES_720x480i"

    #define KEYWORD_TO_RAPTORS_480P60  "resolution=RAPTORS_RES_720x480p"
    #define KEYWORD_TO_RAPTORS_576P50  "resolution=RAPTORS_RES_720x576p"
    #define KEYWORD_TO_RAPTORS_720P50  "resolution=RAPTORS_RES_1280x720p_50Hz"
    #define KEYWORD_TO_RAPTORS_720P60  "resolution=RAPTORS_RES_1280x720p_60Hz"
    #define KEYWORD_TO_RAPTORS_1080P50 "resolution=RAPTORS_RES_1920x1080p_50Hz"
    #define KEYWORD_TO_RAPTORS_1080P60 "resolution=RAPTORS_RES_1920x1080p_60Hz"
    #define KEYWORD_TO_RAPTORS_4K2KP24 "resolution=RAPTORS_RES_4K2Kp_24Hz"
    #define KEYWORD_TO_RAPTORS_4K2KP25 "resolution=RAPTORS_RES_4K2Kp_25Hz"
    #define KEYWORD_TO_RAPTORS_4K2KP30 "resolution=RAPTORS_RES_4K2Kp_30Hz"
    #define KEYWORD_TO_RAPTORS_4K2KP50 "resolution=RAPTORS_RES_4K2Kp_50Hz"
    #define KEYWORD_TO_RAPTORS_4K2KP60 "resolution=RAPTORS_RES_4K2Kp_60Hz"
    #define KEYWORD_TO_RAPTORS_4096P24 "resolution=RAPTORS_RES_4096p_24Hz"
    #define KEYWORD_TO_RAPTORS_4096P25 "resolution=RAPTORS_RES_4096p_25Hz"
    #define KEYWORD_TO_RAPTORS_4096P30 "resolution=RAPTORS_RES_4096p_30Hz"
    #define KEYWORD_TO_RAPTORS_4096P50 "resolution=RAPTORS_RES_4096p_50Hz"
    #define KEYWORD_TO_RAPTORS_4096P60 "resolution=RAPTORS_RES_4096p_60Hz"


    FILE *cmdLine;
    char cmdLineBuf[CMD_LINE_SIZE];
    const char *strPanelName = NULL;

    memset(cmdLineBuf,0,sizeof(cmdLineBuf));
    cmdLine=fopen(PROC_CMD_LINE, "r");

    if(cmdLine != NULL)
    {
        fgets(cmdLineBuf, CMD_LINE_SIZE - 1, cmdLine);
        fclose(cmdLine);
        if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4096P60))
        {
            strPanelName = "DACOUT_4096X2160P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4096P50))
        {
            strPanelName = "DACOUT_4096X2160P_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4096P30))
        {
            strPanelName = "DACOUT_4096X2160P_30";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4096P25))
        {
            strPanelName = "DACOUT_4096X2160P_25";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4096P24))
        {
            strPanelName = "DACOUT_4096X2160P_24";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4K2KP60))
        {
            strPanelName = "DACOUT_4K2KP_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4K2KP50))
        {
            strPanelName = "DACOUT_4K2KP_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4K2KP30))
        {
            strPanelName = "DACOUT_4K2KP_30";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4K2KP25))
        {
            strPanelName = "DACOUT_4K2KP_25";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_4K2KP24))
        {
            strPanelName = "DACOUT_4K2KP_24";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080P24))
        {
            strPanelName = "DACOUT_1080P_24";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080P25))
        {
            strPanelName = "DACOUT_1080P_25";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080P30))
        {
            strPanelName = "DACOUT_1080P_30";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080P50))
        {
            strPanelName = "DACOUT_1080P_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080P60))
        {
            strPanelName = "DACOUT_1080P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080I50))
        {
           strPanelName = "DACOUT_1080I_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1080I60))
        {
           strPanelName = "DACOUT_1080I_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_720P50))
        {
            strPanelName = "DACOUT_720P_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_720P60))
        {
           strPanelName = "DACOUT_720P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_576P50))
        {
            strPanelName = "DACOUT_576P_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_576I50))
        {
            strPanelName = "DACOUT_576I_50";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_480P60))
        {
            strPanelName = "DACOUT_480P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_480I60))
        {
            strPanelName = "DACOUT_480I_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_768P60))
        {
            strPanelName = "DACOUT_1024X768P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1024P60))
        {
            strPanelName = "DACOUT_1280X1024P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_900P60))
        {
            strPanelName = "DACOUT_1440X900P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_DAC_1200P60))
        {
            strPanelName = "DACOUT_1600X1200P_60";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_4K2KP30))
        {
            strPanelName = "HDMITX_4K2K_30P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_4K1KP60))
        {
            strPanelName = "HDMITX_4K1K_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_4K2KP25))
        {
            strPanelName = "HDMITX_4K2K_25P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_1080P50))
        {
            strPanelName = "HDMITX_1080_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_1080P60))
        {
            strPanelName = "HDMITX_1080_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_1080I50))
        {
            strPanelName = "HDMITX_1080_50I";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_1080I60))
        {
            strPanelName = "HDMITX_1080_60I";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_720P50))
        {
           strPanelName = "HDMITX_720_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_720P60))
        {
            strPanelName = "HDMITX_720_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_576P50))
        {
            strPanelName = "HDMITX_576_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_576I50))
        {
            //strPanelName = "HDMITX_576_50I";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_480P60))
        {
            strPanelName = "HDMITX_480_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_HDMITX_480I60))
        {
            //strPanelName = "HDMITX_480_60I";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_480P60))
        {
            strPanelName = "HDMITX_VB1_480_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_576P50))
        {
            strPanelName = "HDMITX_VB1_576_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_720P50))
        {
            strPanelName = "HDMITX_VB1_720_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_720P60))
        {
            strPanelName = "HDMITX_VB1_720_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_1080P50))
        {
            strPanelName = "HDMITX_VB1_1080_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_1080P60))
        {
            strPanelName = "HDMITX_VB1_1080_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4K2KP24))
        {
            strPanelName = "HDMITX_VB1_4K2K_24P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4K2KP25))
        {
            strPanelName = "HDMITX_VB1_4K2K_25P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4K2KP30))
        {
            strPanelName = "HDMITX_VB1_4K2K_30P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4K2KP50))
        {
            strPanelName = "HDMITX_VB1_4K2K_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4K2KP60))
        {
            strPanelName = "HDMITX_VB1_4K2K_60P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4096P24))
        {
            strPanelName = "HDMITX_VB1_4096_24P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4096P25))
        {
            strPanelName = "HDMITX_VB1_4096_25P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4096P30))
        {
            strPanelName = "HDMITX_VB1_4096_30P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4096P50))
        {
            strPanelName = "HDMITX_VB1_4096_50P";
        }
        else if(strstr(cmdLineBuf, KEYWORD_TO_RAPTORS_4096P60))
        {
            strPanelName = "HDMITX_VB1_4096_60P";
        }
        else
        {
            SYSTEM_INFO_ERR("Error, panel name not found!\n");
        }
    }
    if(strPanelName)
    {
        strcpy(pcName,strPanelName);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
// Function: PreLoadSystemIni
// Describion: Pre-load the INI File (Customer.ini) to SysIniBlock
//-----------------------------------------------------------------------------
BOOL PreLoadSystemIni(void)
{
    char * pu8ini_strval=NULL;

    /* Load the system ini file */
   // ASSERT(m_pSystemini);

    int iModelNameLen =(int)strlen(m_pModelName);

    memset(SysIniBlock.ModelName, 0, iModelNameLen + 1);
    memcpy(SysIniBlock.ModelName, m_pModelName, iModelNameLen);

    ASSERT(m_pCustomerini);

    char dPanelName[128] = "/config/panel/";
    char sPanelName[128] = {};
    char ePanelName[5] = ".ini";
    char *pPanelName = NULL;
    if(GetPanelNameFromCmdline(sPanelName)==TRUE)
    {
        pPanelName = strcat(dPanelName,(strcat(sPanelName,ePanelName)));
    }
    else
    {
        pPanelName = iniparser_getstr(m_pCustomerini, "panel:m_pPanelName");
    }
    /* Assign the panel name to SysIniBlock */
    if(pPanelName != NULL)
    {
        int iPanelNameLen = (int)strlen(pPanelName);
        memset(SysIniBlock.PanelName, 0, iPanelNameLen + 1);
        memcpy(SysIniBlock.PanelName, pPanelName, iPanelNameLen);
    }


    /* Read and assign the gamma bin file flags  number to SysIniBlock */
    SysIniBlock.bGammabinflags = iniparser_getint(m_pCustomerini, "GAMMA_BIN:bGammaBinFlags", -1);

    /* Read and assign the gamma table number to SysIniBlock */
    SysIniBlock.GammaTableNo = iniparser_getint(m_pCustomerini, "panel:gammaTableNo", -1);

    /* Read and assign the tuner select number to SysIniBlock */
    SysIniBlock.TunerSelectNo = iniparser_getint(m_pCustomerini, "Tuner:TunerSelectNo", -1);

    /* Read and assign the tuner SAW type to SysIniBlock */
    SysIniBlock.TunerSAWType = iniparser_getint(m_pCustomerini, "Tuner:TunerSawType", -1);

    /* Read abd assign the audio amplifier number to SysIniBlock */
    SysIniBlock.AudioAmpSelect = iniparser_getint(m_pCustomerini, "AudioAmp:AudioAmpSelect", -1);

    /* Read abd assign the ursa enable to SysIniBlock */
    SysIniBlock.UrsaEanble= iniparser_getboolean(m_pCustomerini, "Ursa:UrsaEnable", 0);
    /* Read abd assign the Ursa number to SysIniBlock */
    SysIniBlock.UrsaSelect = iniparser_getint(m_pCustomerini, "Ursa:UrsaSelect", -1);
    /* Read abd assign the MEMC Panel enable to SysIniBlock */
    SysIniBlock.MEMCPanelEnable= iniparser_getboolean(m_pCustomerini, "MEMCPanel:MEMCPanelEnable", 0);
    /* Read abd assign the MEMC Panel number to SysIniBlock */
    SysIniBlock.MEMCPanelSelect = iniparser_getint(m_pCustomerini, "MEMCPanel:MEMCPanelSelect", -1);
    /* Read abd assign the PQ use default value  to SysIniBlock */
    SysIniBlock.PQBinDefault= iniparser_getint(m_pCustomerini, "panel:bPQUseDefaultValue", -1);

    /* Read and assign the bPqBypassSupported to SysIniBlock */
    SysIniBlock.bPqBypassSupported= iniparser_getboolean(m_pCustomerini, "MISC:bPqBypassSupported", 0);

    /* Read and assign the AVSYnc delay value*/
    SysIniBlock.u8AVSyncDelay = iniparser_getint(m_pCustomerini, "MISC:AVSyncDelay", 0);

    /* Read and assign the bMirrorVideo to SysIniBlock */
    SysIniBlock.bMirrorVideo = iniparser_getboolean(m_pCustomerini, "MISC_MIRROR_CFG:MIRROR_VIDEO", 0);

    /* Read and assign the u8MirrorType to SysIniBlock */
    SysIniBlock.u8MirrorType= iniparser_getint(m_pCustomerini, "MISC_MIRROR_CFG:MIRROR_VIDEO_TYPE", 3);

    /* Read and assign the hbbtvDelayInit to SysIniBlock */
    SysIniBlock.bHbbtvDelayInitFlag = iniparser_getboolean(m_pCustomerini, "hbbtv:hbbtvDelayInitFlag", 1);

    // for customer blue screen
    /* Read and assign the bIsBlueScreenOn to SysIniBlock */
    SysIniBlock.bUseCustomerScreenMuteColor = iniparser_getboolean(m_pCustomerini, "panel:bIsBlueScreenOn", 0);
    SysIniBlock.u8NoSignalColor = iniparser_getint(m_pCustomerini, "panel:u8NoSignalColor", 0);
    SysIniBlock.u8FrameColorRU = iniparser_getint(m_pCustomerini, "panel:u8FrameColorRU", 0x00);
    SysIniBlock.u8FrameColorGY = iniparser_getint(m_pCustomerini, "panel:u8FrameColorGY", 0x00);
    SysIniBlock.u8FrameColorBV = iniparser_getint(m_pCustomerini, "panel:u8FrameColorBV", 0x00);

    //for storage MAC config
    /* Read and assign the bIsSPIMacEnable to SysIniBlock */
    SysIniBlock.bSPIMacEnable = iniparser_getboolean(m_pCustomerini, "StorageMAC:bSPIMacEnable", 0);

    /* Read and assign the MacSPIBank to SysIniBlock */
    SysIniBlock.u8MacSPIBank = iniparser_getint(m_pCustomerini, "StorageMAC:MacSPIBank", -1);

    /* Read and assign the HdcpSPIOffset to SysIniBlock */
    SysIniBlock.u16MacSPIOffset = iniparser_getint(m_pCustomerini, "StorageMAC:MacSPIOffset", -1);

    /* Read abd assign the path of customer PQ binary file to SysIniBlock */
    pu8ini_strval = iniparser_getstr(m_pCustomerini, "panel:PQBinPathName");
    ASSERT(pu8ini_strval);
    int len = strlen(pu8ini_strval);
    if(len>=64)
    {
        ASSERT(0);
    }
    memset(SysIniBlock.PQPathName , 0 , len + 1);
    memcpy(SysIniBlock.PQPathName ,(void*)pu8ini_strval, len);
    SetPQPathName(pu8ini_strval, len);

    return TRUE;
}


BOOL iniparser_getU8array(dictionary * pdic, const char * pkey, const MS_U16 u16OutDataLen, MS_U8 * pOutDataVal)
{
    char * pIniString;
    MS_U16 len=0;
    char pVal[10]={0};
    MS_U16 read_index=0;
    MS_U16 write_index=0;
    MS_U16 check_x=0;

    MS_U16 u16OutDataIndex=0;
    MS_U8 carry=0;
    MS_U32 temp_value=0;
    MS_U32 temp_pOutDataVal=0;

    if((pdic==NULL) || (pkey==NULL) || (pOutDataVal==NULL))
    {
        ASSERT(0);
    }
    memset(pOutDataVal, 0, u16OutDataLen);

    pIniString = iniparser_getstr(pdic, pkey);

    if(pIniString==NULL)
    {
        //ASSERT(0);
        return FALSE;
    }
    len = strlen(pIniString);

    //
    //skip space
    //
    while((pIniString[read_index] == ' ') && (read_index<len))
    {
        read_index++;
    }

    //
    //seek the start of array '{'
    //
    if((pIniString[read_index] == '{'))
    {
        read_index++;
    }
    else
    {
        SYSTEM_INFO_ERR("ERROR: Wrong volumn curve table in ini file :: 1 \n");
        ASSERT(0);
    }

    //
    //skip space
    //
    while((pIniString[read_index] == ' ') && (read_index<len))
    {
        read_index++;
    }

    //
    // read string and parser data, then store data
    //
    while((pIniString[read_index] != '}') && (read_index<len))
    {
        //check 0~9
        if(!((pIniString[read_index]>='0') && (pIniString[read_index]<='9')))
        {
            SYSTEM_INFO_ERR("ERROR: Wrong volumn curve table in ini file ::  2 - %c  \n",pIniString[read_index]);
            ASSERT(0);
        }

        //check 0~9, but not ',' or '}'
        write_index=0;
        while(((pIniString[read_index]>='0') && (pIniString[read_index]<='9')) || ((pIniString[read_index] != ',') && (pIniString[read_index] != '}')))
        {

            pVal[write_index]= pIniString[read_index];
            write_index++;
            read_index++;

            check_x=0;
            while((pIniString[read_index] == 'x') || ((pIniString[read_index]>='0') && (pIniString[read_index]<='9')) || ((pIniString[read_index]>='a') && (pIniString[read_index]<='f'))  || (pIniString[read_index]== 'X') || ((pIniString[read_index]>='A') && (pIniString[read_index]<='F')))
            {
                pVal[write_index]= pIniString[read_index];
                write_index++;
                read_index++;
                check_x++;
                if((pIniString[read_index] == 'x') && (check_x !=1))
                {
                    SYSTEM_INFO_ERR("ERROR: Wrong volumn curve table in ini file ::  3 \n");
                    ASSERT(0);
                }
            }
             //skip space
            while((pIniString[read_index] == ' '))
            {
                read_index++;
            }
        }

        //check ','
        if(pIniString[read_index] == ',')
        {
            pVal[write_index]='\0';
            read_index++;
        }
        else if(pIniString[read_index] == '}')
        {
            pVal[write_index]='\0';
        }
        else
        {
            SYSTEM_INFO_ERR("ERROR: Wrong volumn curve table in ini file :: 3 - %c  \n", pIniString[read_index]);
            ASSERT(0);
        }

        //
        //  transfer string into U8 and store it
        //
        write_index=0;
        if(pVal[write_index+1]=='x')
        {
            carry = 16;
            write_index=write_index+2;
        }
        else
        {
            carry = 10;
        }
        temp_pOutDataVal=0;
        while(pVal[write_index]!='\0')
        {
            if(((pVal[write_index]>='0') && (pVal[write_index]<='9')))
                temp_value = (pVal[write_index] - '0' );
            else if(((pVal[write_index]>='A') && (pVal[write_index]<='F')))
                temp_value = (pVal[write_index]-'A' +10 );
            else if(((pVal[write_index]>='a') && (pVal[write_index]<='f')))
                temp_value = (pVal[write_index] -'a' +10 );

            temp_pOutDataVal=temp_value +temp_pOutDataVal*carry;
            write_index++;
        }
        //check overflow
        if(temp_pOutDataVal > 0xFF)
        {
            ASSERT(0);
        }
        pOutDataVal[u16OutDataIndex]=temp_pOutDataVal;
        u16OutDataIndex++;

        //
        //skip space
        //
        while((pIniString[read_index] == ' ') && (read_index<len))
        {
            read_index++;
        }
    }

    if(read_index>=len)
    {
        SYSTEM_INFO_ERR("ERROR: Wrong volumn curve table in ini file :: 5\n");
        ASSERT(0);
    }

    return TRUE;
}

void LoadIniFile(void)
{
    char *pDLCName=NULL;
    char *pMatrixName=NULL;
    char *pBoardName = NULL;
    m_pCustomerini = iniparser_load(m_pModelName);
    ASSERT(m_pCustomerini);

    //board
    pBoardName =iniparser_getstr(m_pCustomerini, "board:m_pBoardName");
    ASSERT(pBoardName);
    m_pBoardini = iniparser_load(pBoardName);
    ASSERT(m_pBoardini);

    //DLC
    m_pDCLIni = iniparser_load((char *)DLC_INI_UPDATE_PATH);
    if(m_pDCLIni == NULL)
    {
        pDLCName  =iniparser_getstr(m_pCustomerini, "DLC:m_pDLCName");
        ASSERT(pDLCName);
        m_pDCLIni = iniparser_load(pDLCName);
        ASSERT(m_pDCLIni);
    }

    //color matrix
    m_pMatrixIni = iniparser_load((char *)COLORMATRIX_INI_UPDATE_PATH);
    if(m_pMatrixIni == NULL)
    {
        pMatrixName  =iniparser_getstr(m_pCustomerini , "ColorMatrix:MatrixName");
        ASSERT(pMatrixName);
        m_pMatrixIni = iniparser_load(pMatrixName);
        ASSERT(m_pMatrixIni);
    }

#if 0
    //wait SDK CL 568471 release
    MI_SECTION_FILTER_CONFIG demuxSectionFilterCfg = {0};
    demuxSectionFilterCfg.u16Section1kFilterNumber = iniparser_getunsignedint(m_pCustomerini, "TSP:Section1kFilterNumber", 0);
    demuxSectionFilterCfg.u16Section4kFilterNumber = iniparser_getunsignedint(m_pCustomerini, "TSP:Section4kFilterNumber", 0);
    demuxSectionFilterCfg.u16Section64kFilterNumber = iniparser_getunsignedint(m_pCustomerini, "TSP:Section64kFilterNumber", 0);
    mapi_syscfg_fetch::SetTspSectionFilterConfig(demuxSectionFilterCfg);
#endif
}

BOOL LoadPanelInfo(dictionary* pPanelini, MAPI_PanelType* stPanelInfo)
{
    strncpy((char*)stPanelInfo->pPanelName, iniparser_getstr(pPanelini, "panel:m_pPanelName"), PANEL_NAME_LEN);

    stPanelInfo->bPanelDither          = iniparser_getboolean(pPanelini, "panel:m_bPanelDither", TRUE);
    stPanelInfo->ePanelLinkType        = (MAPI_APIPNL_LINK_TYPE)iniparser_getint(pPanelini, "panel:m_ePanelLinkType", E_MAPI_LINK_LVDS);
    stPanelInfo->bPanelDualPort        = iniparser_getboolean(pPanelini, "panel:m_bPanelDualPort", TRUE);
    stPanelInfo->bPanelSwapPort        = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapPort", TRUE);
    stPanelInfo->bPanelSwapOdd_ML      = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapOdd_ML", TRUE);
    stPanelInfo->bPanelSwapEven_ML     = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapEven_ML", TRUE);
    stPanelInfo->bPanelSwapOdd_RB      = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapOdd_RB", TRUE);
    stPanelInfo->bPanelSwapEven_RB     = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapEven_RB", TRUE);

    //when mapi_display.cpp set MApi_PNL_SkipTimingChange(TRUE), here panel setting just be skip, can't setting to utopia
    stPanelInfo->bPanelSwapLVDS_POL    = iniparser_getboolean(m_pBoardini, "PanelRelativeSetting:m_bPANEL_SWAP_LVDS_POL", TRUE);
    stPanelInfo->bPanelSwapLVDS_CH     = iniparser_getboolean(m_pBoardini, "PanelRelativeSetting:m_bPANEL_SWAP_LVDS_CH", TRUE);
    stPanelInfo->bPanelPDP10BIT        = iniparser_getboolean(m_pBoardini, "PanelRelativeSetting:m_bPANEL_PDP_10BIT", TRUE);
    stPanelInfo->bPanelLVDS_TI_MODE    = iniparser_getboolean(pPanelini, "panel:m_bPanelLVDS_TI_MODE", TRUE);

    stPanelInfo->ucPanelDCLKDelay      = iniparser_getint(pPanelini, "panel:m_ucPanelDCLKDelay", 0);
    stPanelInfo->bPanelInvDCLK         = iniparser_getboolean(pPanelini, "panel:m_bPanelInvDCLK", TRUE);
    stPanelInfo->bPanelInvDE           = iniparser_getboolean(pPanelini, "panel:m_bPanelInvDE", TRUE);
    stPanelInfo->bPanelInvHSync        = iniparser_getboolean(pPanelini, "panel:m_bPanelInvHSync", TRUE);
    stPanelInfo->bPanelInvVSync        = iniparser_getboolean(pPanelini, "panel:m_bPanelInvVSync", TRUE);

    stPanelInfo->ucPanelDCKLCurrent    = iniparser_getint(pPanelini, "panel:m_ucPanelDCKLCurrent", 0);
    stPanelInfo->ucPanelDECurrent      = iniparser_getint(pPanelini, "panel:m_ucPanelDECurrent", 0);
    stPanelInfo->ucPanelODDDataCurrent = iniparser_getint(pPanelini, "panel:m_ucPanelODDDataCurrent", 0);
    stPanelInfo->ucPanelEvenDataCurrent= iniparser_getint(pPanelini, "panel:m_ucPanelEvenDataCurrent", 0);

    stPanelInfo->wPanelOnTiming1       = iniparser_getint(pPanelini, "panel:m_wPanelOnTiming1", 0);
    stPanelInfo->wPanelOnTiming2       = iniparser_getint(pPanelini, "panel:m_wPanelOnTiming2", 0);
    stPanelInfo->wPanelOffTiming1      = iniparser_getint(pPanelini, "panel:m_wPanelOffTiming1", 0);
    stPanelInfo->wPanelOffTiming2      = iniparser_getint(pPanelini, "panel:m_wPanelOffTiming2", 0);

    stPanelInfo->ucPanelHSyncWidth     = iniparser_getint(pPanelini, "panel:m_ucPanelHSyncWidth", 0);
    stPanelInfo->ucPanelHSyncBackPorch = iniparser_getint(pPanelini, "panel:m_ucPanelHSyncBackPorch", 0);

    stPanelInfo->ucPanelVSyncWidth     = iniparser_getint(pPanelini, "panel:m_ucPanelVSyncWidth", 0);
    stPanelInfo->ucPanelVBackPorch     = iniparser_getint(pPanelini, "panel:m_ucPanelVBackPorch", 0);

    stPanelInfo->wPanelHStart          = iniparser_getint(pPanelini, "panel:m_wPanelHStart", 0);
    stPanelInfo->wPanelVStart          = iniparser_getint(pPanelini, "panel:m_wPanelVStart", 0);

    stPanelInfo->wPanelWidth           = iniparser_getint(pPanelini, "panel:m_wPanelWidth", 0);
    stPanelInfo->wPanelHeight          = iniparser_getint(pPanelini, "panel:m_wPanelHeight", 0);

    stPanelInfo->wPanelMaxHTotal       = iniparser_getint(pPanelini, "panel:m_wPanelMaxHTotal", 0);
    stPanelInfo->wPanelHTotal          = iniparser_getint(pPanelini, "panel:m_wPanelHTotal", 0);
    stPanelInfo->wPanelMinHTotal       = iniparser_getint(pPanelini, "panel:m_wPanelMinHTotal", 0);

    stPanelInfo->wPanelMaxVTotal       = iniparser_getint(pPanelini, "panel:m_wPanelMaxVTotal", 0);
    stPanelInfo->wPanelVTotal          = iniparser_getint(pPanelini, "panel:m_wPanelVTotal", 0);
    stPanelInfo->wPanelMinVTotal       = iniparser_getint(pPanelini, "panel:m_wPanelMinVTotal", 0);

    // Panel DCLK (U16 data type), used to break DCLK up bound 255.
    stPanelInfo->u16PanelMaxDCLK        = iniparser_getint(pPanelini, "panel:m_dwPanelMaxDCLK", 0);
    stPanelInfo->u16PanelDCLK           = iniparser_getint(pPanelini, "panel:m_dwPanelDCLK", 0);
    stPanelInfo->u16PanelMinDCLK        = iniparser_getint(pPanelini, "panel:m_dwPanelMinDCLK", 0);

    stPanelInfo->dwPanelMaxDCLK        = iniparser_getint(pPanelini, "panel:m_dwPanelMaxDCLK", 0);
    stPanelInfo->dwPanelDCLK           = iniparser_getint(pPanelini, "panel:m_dwPanelDCLK", 0);
    stPanelInfo->dwPanelMinDCLK        = iniparser_getint(pPanelini, "panel:m_dwPanelMinDCLK", 0);

    stPanelInfo->wSpreadSpectrumStep   = iniparser_getint(pPanelini, "panel:m_wSpreadSpectrumStep", 0);
    stPanelInfo->wSpreadSpectrumSpan   = iniparser_getint(pPanelini, "panel:m_wSpreadSpectrumSpan", 0);

    stPanelInfo->ucDimmingCtl          = iniparser_getint(pPanelini, "panel:m_ucDimmingCtl", 0);
    stPanelInfo->ucMaxPWMVal           = iniparser_getint(pPanelini, "panel:m_ucMaxPWMVal", 0);
    stPanelInfo->ucMinPWMVal           = iniparser_getint(pPanelini, "panel:m_ucMinPWMVal", 0);

    stPanelInfo->stCfdAttribute.u8ColorFormat = iniparser_getint(pPanelini, "CFD:ColorFormat", 3);
    stPanelInfo->stCfdAttribute.u8ColorDataFormat = iniparser_getint(pPanelini, "CFD:ColorDataFormat", 0);
    stPanelInfo->stCfdAttribute.bIsFullRange = iniparser_getint(pPanelini, "CFD:FullRange", 1);

    stPanelInfo->stCfdAttribute.u16tRx = iniparser_getint(pPanelini, "CFD:Rx", 32000);
    stPanelInfo->stCfdAttribute.u16tRy = iniparser_getint(pPanelini, "CFD:Ry", 16500);
    stPanelInfo->stCfdAttribute.u16tGx = iniparser_getint(pPanelini, "CFD:Gx", 15000);
    stPanelInfo->stCfdAttribute.u16tGy = iniparser_getint(pPanelini, "CFD:Gy", 30000);
    stPanelInfo->stCfdAttribute.u16tBx = iniparser_getint(pPanelini, "CFD:Bx", 7500);
    stPanelInfo->stCfdAttribute.u16tBy = iniparser_getint(pPanelini, "CFD:By", 3000);
    stPanelInfo->stCfdAttribute.u16tWx = iniparser_getint(pPanelini, "CFD:Wx", 15635);
    stPanelInfo->stCfdAttribute.u16tWy = iniparser_getint(pPanelini, "CFD:Wy", 16450);
    stPanelInfo->stCfdAttribute.u16MaxLuminance = iniparser_getint(pPanelini, "CFD:MaxLuminance", 100);
    stPanelInfo->stCfdAttribute.u16MedLuminance = iniparser_getint(pPanelini, "CFD:MedLuminance", 50);
    stPanelInfo->stCfdAttribute.u16MinLuminance = iniparser_getint(pPanelini, "CFD:MinLuminance", 500);
#if CFD_PANEL_VERSION >= 1
    stPanelInfo->stCfdAttribute.bLinearRgb = iniparser_getint(pPanelini, "CFD:LinearRGB", 0);
    stPanelInfo->stCfdAttribute.bCustomerColorPrimaries = iniparser_getint(pPanelini, "CFD:CustomerColorPrimaries", 0);
    stPanelInfo->stCfdAttribute.u16SourceWx = iniparser_getint(pPanelini, "CFD:SourceWx", 15635);
    stPanelInfo->stCfdAttribute.u16SourceWy = iniparser_getint(pPanelini, "CFD:SourceWy", 16450);
#endif

    stPanelInfo->bPanelDeinterMode     = iniparser_getboolean(pPanelini, "panel:m_bPanelDeinterMode", TRUE);

    stPanelInfo->ucPanelAspectRatio    = (MAPI_PNL_ASPECT_RATIO)iniparser_getint(pPanelini, "panel:m_ucPanelAspectRatio", 0);
    stPanelInfo->u16LVDSTxSwapValue    = (iniparser_getint(m_pBoardini, "PanelRelativeSetting:m_u16LVDS_PN_SWAP_H", 0) << 8) | (iniparser_getint(m_pBoardini, "PanelRelativeSetting:m_u16LVDS_PN_SWAP_L", 0));

#ifdef TI_BIT_MODE
    stPanelInfo->ucTiBitMode           = (MAPI_APIPNL_TIBITMODE)TI_BIT_MODE;
#else
    stPanelInfo->ucTiBitMode           = (MAPI_APIPNL_TIBITMODE)iniparser_getint(pPanelini, "panel:m_ucTiBitMode", -1);
#endif

    stPanelInfo->ucOutputFormatBitMode = (MAPI_APIPNL_OUTPUTFORMAT_BITMODE)iniparser_getint(pPanelini, "panel:m_ucOutputFormatBitMode", 0);
    stPanelInfo->bPanelSwapOdd_RG      = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapOdd_RG", TRUE);
    stPanelInfo->bPanelSwapEven_RG     = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapEven_RG", TRUE);
    stPanelInfo->bPanelSwapOdd_GB      = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapOdd_GB", TRUE);
    stPanelInfo->bPanelSwapEven_GB     = iniparser_getboolean(pPanelini, "panel:m_bPanelSwapEven_GB", TRUE);

    stPanelInfo->bPanelDoubleClk       = iniparser_getboolean(pPanelini, "panel:m_bPanelDoubleClk", TRUE);
    stPanelInfo->dwPanelMaxSET         = iniparser_getint(pPanelini, "panel:m_dwPanelMaxSET", 0);
    stPanelInfo->dwPanelMinSET         = iniparser_getint(pPanelini, "panel:m_dwPanelMinSET", 0);

    stPanelInfo->bPanel3DFreerunFlag     = iniparser_getboolean(pPanelini, "panel:bPanel3DFreerunFlag", 0);
    stPanelInfo->bPanel2DFreerunFlag     = iniparser_getboolean(pPanelini, "panel:bPanel2DFreerunFlag", 0);
    stPanelInfo->bPanelReverseFlag       = iniparser_getboolean(pPanelini, "panel:bPanelReverseFlag", 0);
    stPanelInfo->bSGPanelFlag            = iniparser_getboolean(pPanelini, "panel:bSGPanelFlag", 0);
    stPanelInfo->bXCOutput120hzSGPanelFlag  = iniparser_getboolean(pPanelini, "panel:bXCOutput120hzSGPanelFlag", 0);
    stPanelInfo->ucOutTimingMode       = (MAPI_APIPNL_OUT_TIMING_MODE)iniparser_getint(pPanelini, "panel:m_ucOutTimingMode", 0);

    //stPanelInfo->m_bPanelNoiseDith       = iniparser_getboolean(PanelInfo, "panel:m_bPanelNoiseDith", -1);

    stPanelInfo->bMirrorMode            = (MI_U16)iniparser_getint(pPanelini, "panel:mirrorMode", 0);
    stPanelInfo->bMirrorModeH           = (MI_U16)iniparser_getint(pPanelini, "panel:mirrorModeH", 0);
    stPanelInfo->bMirrorModeV           = (MI_U16)iniparser_getint(pPanelini, "panel:mirrorModeV", 0);

    return TRUE;
}

BOOL SetPanelInfo(void)
{
    char *pPanelName=NULL;

    char dPanelName[128] = "/config/panel/";
    char sPanelName[128] = {};
    char ePanelName[5] = ".ini";
    if(GetPanelNameFromCmdline(sPanelName)==TRUE)
    {
        pPanelName = strcat(dPanelName,(strcat(sPanelName,ePanelName)));
    }
    else
    {
        pPanelName =iniparser_getstr(m_pCustomerini, "panel:m_pPanelName");
    }
    ASSERT(pPanelName);
    m_pPanelini = iniparser_load(pPanelName);
    m_u16PanelCount ++;
    unsigned int count;

    for(count =0;count< (sizeof(g_PanelPath)/sizeof(g_PanelPath[0]));count++)
    {
        char *cPath = iniparser_getstr(m_pCustomerini, g_PanelPath[count].cName);
        if (cPath != NULL && strlen(cPath)!= 0)
        {
            strcpy(g_PanelPath[count].cPath,cPath);
            SYSTEM_INFO_DBG("Get panel path. bHdmiTx %d, timing %x, panel ini is %s, path is %s.\n",\
                g_PanelPath[count].bHdmiTx, g_PanelPath[count].enTiming, g_PanelPath[count].cName,\
                g_PanelPath[count].cPath);
            m_u16PanelCount++;
        }
    }

    char *pSysPanelName = iniparser_getstr(m_pPanelini, "panel:m_pPanelName");
    if(pSysPanelName == NULL)
        SysIniBlock.SysPanelName[0]='\0';
    else
    {
        memset(SysIniBlock.SysPanelName ,0 ,64);
        strncpy(SysIniBlock.SysPanelName, pSysPanelName,strlen(pSysPanelName));
        SysIniBlock.SysPanelName[63]='\0';
    }
    SYSTEM_INFO_DBG("SysPanelName : %s \n", SysIniBlock.SysPanelName);

    unsigned int PanelCount = 0;
    ASSERT(m_pBoardini);
    MI_U16 u16LVDS_Output_typ = iniparser_getint(m_pBoardini, "PanelRelativeSetting:m_u16BOARD_LVDS_CONNECT_TYPE", 0);

    ASSERT(m_pPanelini);

    PanelInfo_t pstPanelInfo[m_u16PanelCount];

    //Load default panel info

    memset((void*)pstPanelInfo, 0, sizeof(PanelInfo_t)*m_u16PanelCount);
    LoadPanelInfo(m_pPanelini, &pstPanelInfo[0].PanelAttr);
    pstPanelInfo[0].u16PanelLinkExtType = (MI_U16)iniparser_getint(m_pPanelini, "panel:m_ePanelLinkExtType", E_MAPI_LINK_EXT);
    pstPanelInfo[PanelCount++].eTiming = DISPLAYTIMING_DACOUT_DEFAULT;

    //Load the panel info list
    for(count =0;count< (sizeof(g_PanelPath)/sizeof(g_PanelPath[0]));count++)
    {
        if(strlen(g_PanelPath[count].cPath)!=0)
        {
            SYSTEM_INFO_DBG("Loading, bHdmiTx %d, timing %x, panel ini is %s, path is %s.\n",\
                g_PanelPath[count].bHdmiTx, g_PanelPath[count].enTiming, g_PanelPath[count].cName,\
                g_PanelPath[count].cPath);
            dictionary *pPanelini = NULL;
            pPanelini = iniparser_load(g_PanelPath[count].cPath);
            if (pPanelini == NULL)
            {
                MI_PRINT("Cannot open panel ini file: %s\n", g_PanelPath[count].cPath);
                continue;
            }

            LoadPanelInfo(pPanelini, &pstPanelInfo[PanelCount].PanelAttr);
            pstPanelInfo[PanelCount].u16PanelLinkExtType = (MI_U16)iniparser_getint(pPanelini, "panel:m_ePanelLinkExtType", E_MAPI_LINK_EXT);
            pstPanelInfo[PanelCount].eTiming = g_PanelPath[count].enTiming;
            pstPanelInfo[PanelCount].bHdmiTx = g_PanelPath[count].bHdmiTx;
            iniparser_freedict(pPanelini);
            PanelCount ++;
        }
    }


    //panel backlight PWM information
    PanelBacklightPWMInfo ptPanelBacklightPWMInfo;
    memset(&ptPanelBacklightPWMInfo,0,sizeof(PanelBacklightPWMInfo));
    ptPanelBacklightPWMInfo.u32PeriodPWM              = iniparser_getint(m_pPanelini, "panel:u32PeriodPWM", 0);
    ptPanelBacklightPWMInfo.u32DutyPWM                = iniparser_getint(m_pPanelini, "panel:u32DutyPWM", 0);
    ptPanelBacklightPWMInfo.u16DivPWM                 = iniparser_getint(m_pPanelini, "panel:u16DivPWM", 0);
    ptPanelBacklightPWMInfo.u16MaxPWMvalue            = iniparser_getint(m_pPanelini, "panel:u16MaxPWMvalue", 0);
    ptPanelBacklightPWMInfo.u16MinPWMvalue            = iniparser_getint(m_pPanelini, "panel:u16MinPWMvalue", 0);
    ptPanelBacklightPWMInfo.u8PWMPort = iniparser_getint(m_pBoardini, "PanelRelativeSetting:m_u8BOARD_PWM_PORT", 0);
    ptPanelBacklightPWMInfo.bPolPWM                      = iniparser_getboolean(m_pPanelini, "panel:bPolPWM", TRUE);
    ptPanelBacklightPWMInfo.bBakclightFreq2Vfreq         = iniparser_getboolean(m_pPanelini, "panel:bBakclightFreq2Vfreq", FALSE);

    SetFreerunCfg(FALSE, pstPanelInfo[0].PanelAttr.bPanel2DFreerunFlag);
    SetFreerunCfg(TRUE, pstPanelInfo[0].PanelAttr.bPanel3DFreerunFlag);
    SYSTEM_INFO_DBG("%s <#> Got backup panel info\n\n", __FUNCTION__);

    //mapi system set pnl.
    SetPanelInfoCfg(pstPanelInfo, m_u16PanelCount, u16LVDS_Output_typ, &ptPanelBacklightPWMInfo);
    //Customer PQ
    char *pMainPQPath;
    char *pSubPQPath;

    pMainPQPath = iniparser_getstring(m_pPanelini, "CUSTOMER_PQ:Main_pq_path", NULL);
    pSubPQPath = iniparser_getstring(m_pPanelini, "CUSTOMER_PQ:Sub_pq_path", NULL);

    SetCustomerPQCfg(pMainPQPath, MAPI_PQ_MAIN_WINDOW);
    SetCustomerPQCfg(pSubPQPath, MAPI_PQ_SUB_WINDOW);
    SetFBLModeThreshold(iniparser_getunsignedint(m_pBoardini, "FBL_SETTINGS:FBL_THRESHOLD", MAX_XC_CLOCK));
    return TRUE;
}

BOOL LoadGammaBinInfo(void)
{
    char GammaBinIniName[MAX_BUFFER + 1] ="";
    MI_U8  gammabinfileTotle;
    MI_U8  gammabinfileIndex;
    char *pGammaBinIniName =NULL;
    FILE *fp;
    int filelen;

    gammabinfileTotle = iniparser_getint(m_pCustomerini, "GAMMA_BIN:gammabinfileTotle",-1);
    gammabinfileIndex = iniparser_getint(m_pCustomerini, "GAMMA_BIN:gammabinfileIndex",-1);
    MI_PRINT("\r\n  gammabinfileTotle = %d",gammabinfileTotle);
    MI_PRINT("\r\n  gammabinfileIndex = %d",gammabinfileIndex);

    if(gammabinfileIndex> gammabinfileTotle)
    {
        SYSTEM_INFO_ERR("Can't open Gamma file!\n");
        return FALSE;
    }
    snprintf(GammaBinIniName, MAX_BUFFER, "GAMMA_BIN:GAMMA_FILE_%d", gammabinfileIndex);
    pGammaBinIniName = iniparser_getstring(m_pCustomerini, GammaBinIniName, NULL);
    MI_PRINT("pGammaBinIniName = %s",pGammaBinIniName);

    /* check the Gamma file */
    if((pGammaBinIniName == NULL) || (strlen(pGammaBinIniName) == 0))
    {
        SYSTEM_INFO_ERR("Can't open Gamma bin file!\n");
        return FALSE;
    }

    GAMMA_TABLE_t   *Board_GammaTableInfo[1];
    Board_GammaTableInfo[0] = (GAMMA_TABLE_t *) malloc(sizeof(GAMMA_TABLE_t));
    ASSERT(Board_GammaTableInfo[0]);
    memset(Board_GammaTableInfo[0], 0, sizeof(GAMMA_TABLE_t));

    fp = fopen(pGammaBinIniName, "r");
    if (fp == NULL)
    {
        MI_PRINT("Gamma Bin file open error\n");
    }
    else
    {
        fseek(fp,0,SEEK_END);
        filelen = ftell(fp);
        if (filelen != (GammaArrayMAXSize * 3))
        {
            MI_PRINT("Gamma Bin file length error\n");
        }
        else
        {
            fseek(fp,0,SEEK_SET);
            filelen = fread((char *)Board_GammaTableInfo[0]->NormalGammaR,sizeof(char),GammaArrayMAXSize,fp);
            if (filelen == GammaArrayMAXSize)
            {
                fseek(fp,GammaArrayMAXSize,SEEK_SET);
                filelen = fread((char *)Board_GammaTableInfo[0]->NormalGammaG,sizeof(char),GammaArrayMAXSize,fp);
                if (filelen == GammaArrayMAXSize)
                {
                    fseek(fp,(GammaArrayMAXSize * 2),SEEK_SET);
                    filelen = fread((char *)Board_GammaTableInfo[0]->NormalGammaB,sizeof(char),GammaArrayMAXSize,fp);
                }
            }
        }
        fclose(fp);

    }
    SetGammaTableCfg(Board_GammaTableInfo, 1, 0);
    return TRUE;
}

void ParserStringToArray(const char *inputStr, const char ***strArray, int *count)
{
    int leftIndex = -1, rightIndex = -2;
    int i;

    for (i = 0; i < (int)strlen(inputStr); i++)
    {
        if (*(inputStr+i) == '{')
        {
            leftIndex = i + 1;
            break;
        }
    }

    for (i = strlen(inputStr)-1; i >= 0; i--)
    {
        if (*(inputStr+i) == '}')
        {
            rightIndex = i - 1;
            break;
        }
    }

    if (leftIndex > rightIndex)
    {
        return;
    }

    int len = rightIndex - leftIndex + 1;
    *count = 0;
    i = 0;
    const char *digit = "0123456789xabcdefABCDEF";
    while(i < len){
        i += strcspn(inputStr+leftIndex+i, digit);
        if(isdigit(inputStr[i]))
            *count += 1;
        i += strspn(inputStr+leftIndex+i, digit);
    }
    const char **token = malloc(sizeof(char*)*(*count+1)+len+1);
    memset(token, 0, sizeof(char*)*(*count+1)+len+1);
    char *str = (char*)(token+(*count+1));
    strncpy(str, inputStr + leftIndex, len);
    str[len] = '\0';
    i = 0;
    char *pch = strtok(str, " ,");
    while (pch != NULL)
    {
        token[i++] = pch;
        pch = strtok(NULL, " ,");
    }
    *strArray = token;
}

BOOL ParserGammaTable(GAMMA_TABLE_t *pGAMMA_TABLE_t, const char *pparameter, MS_U8 which_rgb)
{
    if (pparameter == NULL)
    {
        ASSERT(0);
    }

    const char **strArray = NULL;
    int len;
    ParserStringToArray(pparameter, &strArray, &len);

    if (len > GammaArrayMAXSize)
    {
        MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
        ASSERT(0);
    }

    unsigned int gamma_table_index;
    for (gamma_table_index = 0; gamma_table_index < len; gamma_table_index++)
    {
        if (strArray[gamma_table_index] == NULL)
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        int temp_value = 0;

        int filledResult = sscanf(strArray[gamma_table_index], "%x", &temp_value);

        if (filledResult <= 0)
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        if ((temp_value < 0x00) || (temp_value > 0xFF))
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        if(which_rgb=='r')
        {
            pGAMMA_TABLE_t->NormalGammaR[gamma_table_index] = (MI_U8)temp_value;
        }
        else if(which_rgb=='g')
        {
            pGAMMA_TABLE_t->NormalGammaG[gamma_table_index] = (MI_U8)temp_value;
        }
        else if(which_rgb=='b')
        {
            pGAMMA_TABLE_t->NormalGammaB[gamma_table_index] = (MI_U8)temp_value;
        }

    }
    free(strArray);

    return TRUE;
}



BOOL ParseSetGammaTableCfg(int iGammaIdx)
{
    char GammaTableSectionItemName[] = "gamma_table_1:parameter_r";
    const MI_U8 u8MaxGammaTable = 10; // maximum number of Gamma table
    GAMMA_TABLE_t   *Board_GammaTableInfo[u8MaxGammaTable];
    const char *pparameter;
    unsigned char u8Cnt;

    if((iGammaIdx>u8MaxGammaTable) || (iGammaIdx<0))
    {
        MI_PRINT("WRONG GAMMA TABLE NO FROM INI FILE\n");
        ASSERT(0);
    }

    ASSERT(m_pPanelini);

    SYSTEM_INFO_DBG("%s:::::::::: %u\n", __FILE__, __LINE__);

    for (u8Cnt = 0; u8Cnt < u8MaxGammaTable; u8Cnt++)
    {
        SYSTEM_INFO_DBG("Parsing GAMMA[%d]\n", u8Cnt);
    //table no.
        GammaTableSectionItemName[12]= '0' + u8Cnt;

        GammaTableSectionItemName[24]= 'r';
        pparameter           = iniparser_getstr(m_pPanelini, GammaTableSectionItemName);
        if (pparameter == NULL)
        {
            break;
        }
        SysIniBlock.TotalGammaTableNo++;
        Board_GammaTableInfo[u8Cnt] = (GAMMA_TABLE_t *)malloc(sizeof(GAMMA_TABLE_t));
        ASSERT(Board_GammaTableInfo[u8Cnt]);

        ParserGammaTable(Board_GammaTableInfo[u8Cnt], pparameter, 'r');

    GammaTableSectionItemName[24]= 'g';
    pparameter           = iniparser_getstr(m_pPanelini, GammaTableSectionItemName);
        ParserGammaTable(Board_GammaTableInfo[u8Cnt], pparameter, 'g');

    GammaTableSectionItemName[24]= 'b';
    pparameter           = iniparser_getstr(m_pPanelini, GammaTableSectionItemName);
        ParserGammaTable(Board_GammaTableInfo[u8Cnt], pparameter, 'b');
    }

    SetGammaTableCfg(Board_GammaTableInfo, u8Cnt, iGammaIdx);

    unsigned char i;
    for (i = 0; i < u8Cnt; i++)
    {
        free(Board_GammaTableInfo[i]);
    }
    return TRUE;

}

BOOL SetAMPInfo(void)
{
    char *AmpBinPath = NULL;
    AmpBinPath = iniparser_getstring(m_pCustomerini, "AudioAmp:AmpBinPath", NULL);
    /* check the Gamma file */
        SYSTEM_INFO_IFO("AmpBinPath = %s",AmpBinPath);
        SetAMPBinPath(AmpBinPath);
    return TRUE;

}

#if 1
BOOL ParserDCLCurve(MAPI_XC_DLC_init *pBoard_DLC_init, const char *pparameter, E_MAPI_DLC_PURE_INIT_CURVE whichcurve)
{
    const unsigned int curve_arrary_num = (whichcurve == E_MAPI_DLC_INIT_HISTOGRAM_LIMIT_CURVE ? DLC_HISTOGRAM_LIMIT_CURVE_ARRARY_NUM : 16);

    if (pparameter==NULL)
    {
        ASSERT(0);
    }

    const char **strArray;
    int len;
    ParserStringToArray(pparameter, &strArray, &len);

    if (len > curve_arrary_num)
    {
        MI_PRINT("WRONG DCL TABLE FORMAT FROM INI FILE\n\n");
        ASSERT(0);
    }

    unsigned int dlc_table_index;
    for (dlc_table_index = 0; dlc_table_index < len; dlc_table_index++)
    {
        if (strArray[dlc_table_index] == NULL)
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        int temp_value = 0;
        int filledResult = sscanf(strArray[dlc_table_index], "%x", &temp_value);

        if (filledResult <= 0)
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        if ((temp_value < 0x00) || (temp_value > 0xFF))
        {
            MI_PRINT("WRONG GAMMA TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        if(whichcurve==E_MAPI_DLC_INIT_LUMA_CURVE)
        {
            pBoard_DLC_init->DLC_MFinit_Ex.ucLumaCurve[dlc_table_index] = (MI_U8)temp_value;
        }
        else if(whichcurve==E_MAPI_DLC_INIT_LUMA_CURVE2_A)
        {
            pBoard_DLC_init->DLC_MFinit_Ex.ucLumaCurve2_a[dlc_table_index] = (MI_U8)temp_value;
        }
        else if(whichcurve==E_MAPI_DLC_INIT_LUMA_CURVE2_B)
        {
            pBoard_DLC_init->DLC_MFinit_Ex.ucLumaCurve2_b[dlc_table_index] = (MI_U8)temp_value;
        }
        else if(whichcurve==E_MAPI_DLC_INIT_HISTOGRAM_LIMIT_CURVE)
        {
            pBoard_DLC_init->DLC_MFinit_Ex.ucDlcHistogramLimitCurve[dlc_table_index] = temp_value;
        }

    }
    free(strArray);

    return TRUE;
}
#endif

BOOL ParseSetDLCInfo(void)
{
    const MI_U8 u8MaxDLCTable = 10; // maximum number of DLC table
    MAPI_XC_DLC_init board_DLC_init[u8MaxDLCTable];
    MAPI_XC_DLC_init *Board_DLC_init[u8MaxDLCTable];
    const char *pparameter = NULL;

    const MI_U8 u8bufSize = 50; // number of DLC.ini member name length
    char DLCTableSectionItemName[u8bufSize];
    MI_U8 u8Cnt;

    for (u8Cnt = 0; u8Cnt < u8MaxDLCTable; u8Cnt++){
        Board_DLC_init[u8Cnt] = board_DLC_init+u8Cnt;
    }

    for (u8Cnt = 0; u8Cnt < u8MaxDLCTable; u8Cnt++)
    {
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:tLumaCurve", u8Cnt);
        pparameter = iniparser_getstr(m_pDCLIni, DLCTableSectionItemName);
        if (pparameter == NULL)
        {
            break;
        }
        ParserDCLCurve(Board_DLC_init[u8Cnt], pparameter, E_MAPI_DLC_INIT_LUMA_CURVE);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:tLumaCurve2_a", u8Cnt);
        pparameter = iniparser_getstr(m_pDCLIni, DLCTableSectionItemName);
        ParserDCLCurve(Board_DLC_init[u8Cnt], pparameter, E_MAPI_DLC_INIT_LUMA_CURVE2_A);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:tLumaCurve2_b", u8Cnt);
        pparameter = iniparser_getstr(m_pDCLIni, DLCTableSectionItemName);
        ParserDCLCurve(Board_DLC_init[u8Cnt], pparameter, E_MAPI_DLC_INIT_LUMA_CURVE2_B);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:tDlcHistogramLimitCurve", u8Cnt);
        pparameter = iniparser_getstr(m_pDCLIni, DLCTableSectionItemName);
        ParserDCLCurve(Board_DLC_init[u8Cnt], pparameter, E_MAPI_DLC_INIT_HISTOGRAM_LIMIT_CURVE);

        //Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u32DLC_MFinit_Ex_Version= mapi_pql::GetDLCVersionInfo();

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_L_L_U", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_L_L_U               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,3);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_L_L_D", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_L_L_D               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_L_H_U", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_L_H_U               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,3);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_L_H_D", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_L_H_D               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,3);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_S_L_U", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_S_L_U               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,128);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_S_L_D", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_S_L_D               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,128);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_S_H_U", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_S_H_U               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,128);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.u8_S_H_D", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.u8_S_H_D               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,128);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcPureImageMode", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcPureImageMode     = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,2);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcLevelLimit", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcLevelLimit        = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcAvgDelta", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcAvgDelta          = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,15);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcAvgDeltaStill", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcAvgDeltaStill     = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcFastAlphaBlending", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcFastAlphaBlending = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,31);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcYAvgThresholdL", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcYAvgThresholdL    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,5);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcYAvgThresholdH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcYAvgThresholdH    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,200);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcBLEPoint", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcBLEPoint          = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,48);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcWLEPoint", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcWLEPoint          = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,48);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.bEnableBLE", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.bEnableBLE             = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.bEnableWLE", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.bEnableWLE             = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcYAvgThresholdM", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcYAvgThresholdM    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,70);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcCurveMode", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcCurveMode         = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,2);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcCurveModeMixAlpha", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcCurveModeMixAlpha = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,80);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcAlgorithmMode", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcAlgorithmMode     = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcSepPointH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcSepPointH         = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,188);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcSepPointL", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcSepPointL         = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,80);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBleStartPointTH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBleStartPointTH   = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,640);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBleEndPointTH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBleEndPointTH     = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,256);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcCurveDiff_L_TH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcCurveDiff_L_TH    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,56);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcCurveDiff_H_TH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcCurveDiff_H_TH    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,148);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBLESlopPoint_1", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBLESlopPoint_1    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1028);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBLESlopPoint_2", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBLESlopPoint_2    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1168);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBLESlopPoint_3", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBLESlopPoint_3    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1260);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBLESlopPoint_4", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBLESlopPoint_4    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1370);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcBLESlopPoint_5", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcBLESlopPoint_5    = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1440);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.uwDlcDark_BLE_Slop_Min", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.uwDlcDark_BLE_Slop_Min = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1200);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcCurveDiffCoringTH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcCurveDiffCoringTH = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,2);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcAlphaBlendingMin", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcAlphaBlendingMin  = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcAlphaBlendingMax", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcAlphaBlendingMax  = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,128);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcFlicker_alpha", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcFlicker_alpha     = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,96);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcYAVG_L_TH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcYAVG_L_TH         = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,56);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcYAVG_H_TH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcYAVG_H_TH         = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,136);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcDiffBase_L", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcDiffBase_L        = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,4);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcDiffBase_M", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcDiffBase_M        = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,14);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucDlcDiffBase_H", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucDlcDiffBase_H        = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,20);

        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.bCGCCGainCtrl", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.bCGCCGainCtrl          = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucCGCCGain_offset", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucCGCCGain_offset      = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucCGCYCslope", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucCGCYCslope           = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucCGCChroma_GainLimitH", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucCGCChroma_GainLimitH = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,0);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucCGCChroma_GainLimitL", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucCGCChroma_GainLimitL = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,1);
        snprintf(DLCTableSectionItemName, u8bufSize, "DLC_%d:g_DlcParameters.ucCGCYth", u8Cnt);
        Board_DLC_init[u8Cnt]->DLC_MFinit_Ex.ucCGCYth               = iniparser_getint(m_pDCLIni, DLCTableSectionItemName,8);
    }
    SetDLCTableCount(u8Cnt);
    SetDLCInfo(Board_DLC_init,u8Cnt);

    return TRUE;
}

#if 1
BOOL ParserColorCorrectionMatrix(MS_S16 *pS16Matrix, const char *pparameter, MS_U8 u8ItemCnt)
{

    if (pparameter==NULL)
    {
        ASSERT(0);
    }

    const char **strArray;
    int len;
    ParserStringToArray(pparameter, &strArray, &len);

    if (len > u8ItemCnt)
    {
        MI_PRINT("WRONG ColorCorrectionMatrix TABLE FORMAT FROM INI FILE\n\n");
        ASSERT(0);
    }

    unsigned int index;
    for (index = 0; index < len; index++)
    {
        if (strArray[index] == NULL)
        {
            MI_PRINT("WRONG ColorCorrectionMatrix TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        int temp_value = 0;
        int filledResult = sscanf(strArray[index], "%x", &temp_value);

        if (filledResult <= 0)
        {
            MI_PRINT("WRONG ColorCorrectionMatrix TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        if ((temp_value < -32768) || (temp_value > 32767))
        {
            MI_PRINT("WRONG ColorCorrectionMatrix TABLE FORMAT FROM INI FILE\n\n");
            ASSERT(0);
        }

        pS16Matrix[index] = (MI_S16)temp_value;

    }
    free(strArray);

    return TRUE;
}
#endif

BOOL ParseSetColorMatrix(void)
{
    const char *pparameter=NULL;
    MS_S16 s16tmpMatrix[32];
    MAPI_COLOR_MATRIX stMatrix;

    ASSERT(m_pMatrixIni);

    memset(s16tmpMatrix, 0, sizeof(s16tmpMatrix));

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorSpace:tSDTVYuv2rgb");
    if (pparameter != NULL)
    {
        SYSTEM_INFO_DBG("Parsing tSDTVYuv2rgb\n");
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 9);
        memcpy(stMatrix.s16SDYuv2Rgb, s16tmpMatrix, sizeof(stMatrix.s16SDYuv2Rgb));
    }
    else
    {
        ASSERT(0);
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorSpace:tHDTVYuv2rgb");
    if (pparameter != NULL)
    {
        SYSTEM_INFO_DBG("Parsing tSDTVYuv2rgb\n");
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 9);
        memcpy(stMatrix.s16HDYuv2Rgb, s16tmpMatrix, sizeof(stMatrix.s16HDYuv2Rgb));
    }
    else
    {
        ASSERT(0);
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tDefaultColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16Default, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        ASSERT(0);
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tHDTVColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16HDTV, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16HDTV, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tSDTVColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16SDTV, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16SDTV, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }


    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tATVColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16ATV, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16ATV, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }


    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tSDYPbPrColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16SdYPbPr, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16SdYPbPr, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }


    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tHDYPbPrColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16HdYPbPr, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16HdYPbPr, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }


    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tHDHdmiColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16HdHdmi, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16HdHdmi, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tSDHdmiColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16SdHdmi, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16SdHdmi, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tAVColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16AV, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16AV, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }


    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tSVColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16SV, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16SV, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }

    pparameter = iniparser_getstr(m_pMatrixIni, "ColorCorrection:tVgaColorCorrectionMatrix");
    if (pparameter != NULL)
    {
        ParserColorCorrectionMatrix(s16tmpMatrix, pparameter, 32);
        memcpy(stMatrix.s16Vga, s16tmpMatrix, sizeof(s16tmpMatrix));
    }
    else
    {
        memcpy(stMatrix.s16Vga, stMatrix.s16Default, sizeof(stMatrix.s16Default));
    }

    SetColorMatrix(&stMatrix);
    return TRUE;
}

void SetSystemInfo(void)
{
    LoadIniFile();
    // Read customer.ini, DLC.ini and panel.ini and check each of them's customer is correct or not.
    // If the checksum is not correct, it will be recoveried.

    ASSERT(m_pCustomerini);

//ini file lenght is too large.   CheckVersionAndSetDefaultValue(PanelIniType);
//to do....    CheckVersionAndSetDefaultValue(DCLIniType);
//#ifndef BOARD_ALL_INI_DISABLE
    /* Pre-load the INI File (Customer.ini) to SysIniBlock */
    PreLoadSystemIni();
//#endif

    SYSTEM_INFO_DBG("  = SetPQAutoNRParam = \n");
    MAPI_AUTO_NR_INIT_PARAM stPara;
    stPara.u8DebugLevel =(MI_U8)iniparser_getint(m_pCustomerini, "MISC_PQ_NR_CFG:DEBUG_LEVEL", 0);
    stPara.u16AutoNr_L2M_Thr = (MI_U16)iniparser_getint(m_pCustomerini, "MISC_PQ_NR_CFG:LOW2MID_THR", 0);
    stPara.u16AutoNr_M2L_Thr = (MI_U16)iniparser_getint(m_pCustomerini, "MISC_PQ_NR_CFG:MID2LOW_THR", 0);
    stPara.u16AutoNr_M2H_Thr = (MI_U16)iniparser_getint(m_pCustomerini, "MISC_PQ_NR_CFG:MID2HIGH_THR", 0);
    stPara.u16AutoNr_H2M_Thr = (MI_U16)iniparser_getint(m_pCustomerini, "MISC_PQ_NR_CFG:HIGH2MID_THR", 0);
    SetPQAutoNRParam(&stPara);

#if 0
    MI_PRINT("--------------------System.ini START-------------------\n");
    MI_PRINT("Panel:[%s]\n", SysIniBlock.PanelName);
    MI_PRINT("Panel:[%s]\n", SysIniBlock.PanelName);
    MI_PRINT("GammaTableNo:[%d]\n", SysIniBlock.GammaTableNo);
    MI_PRINT("--------------------System.ini  END-------------------\n");
#endif
    SYSTEM_INFO_DBG(" ======== SYSTEM_INFO_ENABLE Start ==========\n");

    //panel.ini
    SYSTEM_INFO_DBG("  = SetPanelInfo= \n");
    SetPanelInfo();

    SYSTEM_INFO_DBG("  = SetGammaTableCfg = \n");
    if(SysIniBlock.bGammabinflags)
        LoadGammaBinInfo();
    else
        ParseSetGammaTableCfg(SysIniBlock.GammaTableNo);

    SYSTEM_INFO_DBG("  = SetDLCInfo = \n");
    ParseSetDLCInfo();

    SYSTEM_INFO_DBG("  = SetColorMatrix = \n");
    ParseSetColorMatrix();
    //board.h

    SetHDMITxAnalogInfo(astHdmiAnalogCfg, HDMITX_INFO_SIZE);

    SYSTEM_INFO_DBG("  = SetSAWType = \n");
    SetSAWType(BOARD_SAW_TYPE);

    SYSTEM_INFO_DBG("  = SetMirrorFlag = \n");
    SetVideoMirrorCfg(SysIniBlock.bMirrorVideo,SysIniBlock.u8MirrorType);

    // for customer blue screen
    SYSTEM_INFO_DBG("  = SetUseCustomerScreenMuteColorFlag = \n");
    SetUseCustomerScreenMuteColorFlag(SysIniBlock.bUseCustomerScreenMuteColor);
    SYSTEM_INFO_DBG("  = SetUseCustomerScreenMuteColor = \n");
    SeCustomerScreenMuteColorType(SysIniBlock.u8NoSignalColor);
    SetCustomerFrameColorType(SysIniBlock.u8FrameColorRU, SysIniBlock.u8FrameColorGY, SysIniBlock.u8FrameColorBV);

    SYSTEM_INFO_DBG(" = SetAMPInfo\n");
    SetAMPInfo();

    SYSTEM_INFO_DBG(" = SetCIModelCount\n");

    // set current mode
    SYSTEM_INFO_DBG(" =Set Current Mode =\n");
    SetCurrentMode(BOARD_DAC_CURRENT_MODE);

    SYSTEM_INFO_DBG(" ======== SYSTEM_INFO_ENABLE End ==========\n");

#if 0
    // Load channel order setting.
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:8V_Order", m_u16Vb18VChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_8V, m_u16Vb18VChannelOrder);
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:4V_Order", m_u16Vb14VChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_4V, m_u16Vb14VChannelOrder);
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:2V_Order", m_u16Vb12VChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_2V, m_u16Vb12VChannelOrder);
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:1V_Order", m_u16Vb11VChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_1V, m_u16Vb11VChannelOrder);
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:4O_Order", m_u16Vb14OChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_4O, m_u16Vb14OChannelOrder);
    ParseVb1ChannelOrder(m_pCustomerini, "VB1_Connector:2O_Order", m_u16Vb12OChannelOrder);
    SetVb1ChannelOrder(E_MAPI_VB1_CHANNELORDER_2O, m_u16Vb12OChannelOrder);

    SystemInfoInited();
    FreeIniFile();
#endif
}

#if 0
void SystemInfo::ParseVb1ChannelOrder(dictionary *pCustomerIni, const char *pKey, MI_U16 u16Order[4])
{
    memset(u16Order, 0, sizeof(MI_U16)*4);

    MI_U8 channelOrder[7];
    memset(channelOrder, 0, sizeof(channelOrder));

    if (iniparser_getU8array(pCustomerIni, pKey, 7, channelOrder) == TRUE)
    {
        u16Order[0] = (u16Order[0] << 8) + channelOrder[0];
        u16Order[0] = (u16Order[0] << 8) + channelOrder[1];
        u16Order[1] = (u16Order[1] << 8) + channelOrder[2];
        u16Order[1] = (u16Order[1] << 8) + channelOrder[3];
        u16Order[2] = (u16Order[2] << 8) + channelOrder[4];
        u16Order[2] = (u16Order[2] << 8) + channelOrder[5];
        u16Order[3] = (u16Order[3] << 8) + channelOrder[6];
    }
}
#endif
