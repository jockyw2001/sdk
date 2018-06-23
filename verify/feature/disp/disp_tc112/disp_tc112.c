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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_vpe.h"
#include "mi_hdmi.h"
#include "../mi_disp_test.h"

//#define DISP_FPGA_TEST
///#define MI_PRINT(fmt, args...)
#define MI_PRINT printf
#define DBG_INFO(fmt, args...)      ({do{printf("[MI INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);usleep(10 * 1000);}while(0);})

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        MI_PRINT("[%d]exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%d)exec function pass\n", __LINE__);\
    }

#define MIDISPCHECKRESULT(result)\
    if(result != MI_SUCCESS)\
    {\
        MI_PRINT("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        MI_PRINT("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

static MI_BOOL _test_disp_GetOneFrame(int srcFd, char *pData, int yuvSize)
{
    if (read(srcFd, pData, yuvSize) < yuvSize)
    {
        lseek(srcFd, 0, SEEK_SET);
        if (read(srcFd, pData, yuvSize) < yuvSize)
        {
            DBG_INFO(" read file error.\n");
            return FALSE;
        }
    }

    return TRUE;
}

static MI_BOOL _gbStop = FALSE;

typedef int(*test_func)(int);

typedef struct {
    const char *desc;
    test_func  func;
    int        next_index;
    MI_BOOL    bEnd;
} test_disp_TestCscMenu_t;

#define TEST_DISP_VALID_INDEX (0xffff)
#define ENTER_TEST() printf("Enty test.\n")
#define EXIT_TEST()  printf("Exit test.\n")
enum{
    //HDMI and VGA CSC
    E_DISP_TEST_CSC_SET_CSCMARTIX =0,
    E_DISP_TEST_CSC_SET_LUMA,
    E_DISP_TEST_CSC_SET_CONTRAST,
    E_DISP_TEST_CSC_SET_HUE,
    E_DISP_TEST_CSC_SET_SATURATION,

    //only Vga set
    E_DISP_TEST_VGA_SET_GAIN,
    E_DISP_TEST_VGA_SET_SHARPNESS,

    E_DISP_TEST_CSC_SHOW_PARA,
    E_DISP_TEST_CSC_SET_RET,

    E_DISP_TEST_CSC_MAX,
} test_disp_TestCscOp_e;

enum {
    E_DISP_TEST_CSC_RET_PASS,
    E_DISP_TEST_CSC_RET_FAIL,
    E_DISP_TEST_CSC_RET_EXIT,
} test_vpe_TestIqRet_e;

static int test_DISP_CscEnd(int id)
{
    return E_DISP_TEST_CSC_RET_EXIT;
}
static int getChoice(void)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    return atoi(buffer);
}
static int test_DISP_CscSetMartix(int DispDev)
{
    int id;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Martix Entry your choice:0~6\n");
    printf("[1] MATRIX_BYPASS: do not change color space \n");
    printf("[2] MATRIX_BT601_TO_BT709: change color space from BT.601 to BT.709 \n");
    printf("[3] MATRIX_BT709_TO_BT601: change color space from BT.709 to BT.601 \n");
    printf("[4] MATRIX_BT601_TO_RGB_PC: change color space from BT.601 to RGB \n");
    printf("[5] MATRIX_BT709_TO_RGB_PC: change color space from BT.709 to RGB \n");
    printf("[6] MATRIX_RGB_TO_BT601_PC: change color space from RGB to BT.601 \n");
    printf("[7] MATRIX_RGB_TO_BT709_PC: change color space from RGB to BT.709 \n");

    id = getChoice();
    if (0<id && id <=7)
    {
        if(DispDev == 0)
        {
            ExecFunc(MI_DISP_GetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
            stHdmiParam.stCsc.eCscMatrix = (MI_DISP_CscMattrix_e)(id -1);
            ExecFunc(MI_DISP_SetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.stCsc.eCscMatrix = (MI_DISP_CscMattrix_e)(id -1);
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 1~7\n");
    }

    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_CscSetLuma(int DispDev)
{
    int id;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Luma Entry your choice: 0 ~ 100.\n");
    id = getChoice();
    if (0<id && id <101)
    {
        if(DispDev == 0)
        {
            ExecFunc(MI_DISP_GetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
            stHdmiParam.stCsc.u32Luma = id;
            ExecFunc(MI_DISP_SetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.stCsc.u32Luma = id;
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 0~100\n");
    }
    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_CscSetContrast(int DispDev)
{
    int id;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Contrast Entry your choice: 0 ~ 100.\n");
    id = getChoice();
    if (0<id && id <101)
    {
        if(DispDev == 0)
        {
            ExecFunc(MI_DISP_GetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
            stHdmiParam.stCsc.u32Contrast = id;
            ExecFunc(MI_DISP_SetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.stCsc.u32Contrast = id;
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 0~100\n");
    }
    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_CscSetHue(int DispDev)
{
    int id;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));


    printf("\n Set Hue Entry your choice: 0 ~ 100.\n");
    id = getChoice();
    if (0<id && id <101)
    {
        if(DispDev == 0)
        {
            ExecFunc(MI_DISP_GetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
            stHdmiParam.stCsc.u32Hue = id;
            ExecFunc(MI_DISP_SetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.stCsc.u32Hue = id;
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 0~100\n");
    }
    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_CscSetSaturation(int DispDev)
{
    int id;
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Staturation Entry your choice: 0 ~ 100.\n");
    id = getChoice();
    if (0<id && id < 101)
    {
        if(DispDev == 0)
        {
            ExecFunc(MI_DISP_GetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
            stHdmiParam.stCsc.u32Saturation = id;
            ExecFunc(MI_DISP_SetHdmiParam(DispDev,&stHdmiParam), MI_SUCCESS);
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.stCsc.u32Saturation = id;
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 0~100\n");
    }
    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_SetVgaGain(int DispDev)
{
    int id;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Vga gain Entry your choice: 0 ~ 64.\n");
    id = getChoice();
    if (0<id && id < 65)
    {
        if(DispDev == 0)
        {
            printf("ERR HDMI Cant Set Gain\n");
        }
        else
        {
            ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
            stVgaParam.u32Gain = id;
            ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        }
    }
    else
    {
        printf("Param Set Err,Please Set 0~64\n");
    }
    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_SetVgaSharpness(int DispDev)
{
    int id;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    printf("\n Set Vga Sharpness Entry your choice.\n");
    id = getChoice();

    if(DispDev == 0)
    {
        printf("ERR HDMI Cant Set Sharpness\n");
    }
    else
    {
        ExecFunc(MI_DISP_GetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
        stVgaParam.u32Sharpness = id;
        ExecFunc(MI_DISP_SetVgaParam(DispDev,&stVgaParam), MI_SUCCESS);
    }

    return E_DISP_TEST_CSC_RET_PASS;
}

static int test_DISP_CscShowPara(int DispDev)
{
    char *Martix;
    MI_DISP_Csc_t stCsctmp;

    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_VgaParam_t stVgaParam;
    memset(&stHdmiParam, 0, sizeof(stHdmiParam));
    memset(&stVgaParam, 0, sizeof(stVgaParam));

    memset(&stCsctmp,0,sizeof(stCsctmp));

    printf("\n");

    if(DispDev ==0)
    {
        ExecFunc(MI_DISP_GetHdmiParam(DispDev, &stHdmiParam), MI_SUCCESS);
        stCsctmp = stHdmiParam.stCsc;
        printf("HDMI Csc Parameters:\n");
    }
    else
    {
        ExecFunc(MI_DISP_GetVgaParam(DispDev, &stVgaParam), MI_SUCCESS);
        stCsctmp = stVgaParam.stCsc;
        printf("VGA Csc Parameters:\n");
    }

    switch(stCsctmp.eCscMatrix)
    {
        case E_MI_DISP_CSC_MATRIX_BYPASS:
            Martix = "E_MI_DISP_CSC_MATRIX_BYPASS";
            break;
        case E_MI_DISP_CSC_MATRIX_BT601_TO_BT709:
            Martix = "E_MI_DISP_CSC_MATRIX_BT601_TO_BT709";
            break;
        case E_MI_DISP_CSC_MATRIX_BT709_TO_BT601:
            Martix = "E_MI_DISP_CSC_MATRIX_BT709_TO_BT601";
            break;
        case E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC:
            Martix = "E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC";
            break;
        case E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC:
            Martix = "E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC";
            break;
        case E_MI_DISP_CSC_MATRIX_RGB_TO_BT601_PC:
            Martix = "E_MI_DISP_CSC_MATRIX_RGB_TO_BT601_PC";
            break;
        case E_MI_DISP_CSC_MATRIX_RGB_TO_BT709_PC:
            Martix = "E_MI_DISP_CSC_MATRIX_RGB_TO_BT709_PC";
            break;
        default :
            {
                printf("eCscMatrix Param %d error \n", stHdmiParam.stCsc.eCscMatrix);
                Martix = "ERROR";
            }
            break;
     }

    printf("\t.CscMatrix     : %s.\n", Martix);
    printf("\t.u32Contrast   : %d.\n", stCsctmp.u32Contrast);
    printf("\t.u32Hue        : %d.\n", stCsctmp.u32Hue);
    printf("\t.u32Luma       : %d.\n", stCsctmp.u32Luma);
    printf("\t.u32Saturation : %d.\n", stCsctmp.u32Saturation);
    if(DispDev == 1)
    {
        printf("\t.u32Gain       : %d.\n", stVgaParam.u32Gain);
        printf("\t.u32Sharpness  : %d.\n", stVgaParam.u32Sharpness);
    }
    printf("\n");

    return E_DISP_TEST_CSC_RET_PASS;
}

static test_disp_TestCscMenu_t _gTestDispCsc[] = {
    // PARAM
    [E_DISP_TEST_CSC_SET_CSCMARTIX]    = {"SET_CSC_MARTIX",    test_DISP_CscSetMartix,    E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_CSC_SET_LUMA]         = {"SET_CSC_LUMA",      test_DISP_CscSetLuma,      E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_CSC_SET_CONTRAST]     = {"SET_CSC_CONTRAST",  test_DISP_CscSetContrast,  E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_CSC_SET_HUE]          = {"SET_CSC_HUE",       test_DISP_CscSetHue,       E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_CSC_SET_SATURATION]   = {"SET_CSC_SATURATION",test_DISP_CscSetSaturation,E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_VGA_SET_GAIN]   =       {"SET_VGA_GAIN",      test_DISP_SetVgaGain,      E_DISP_TEST_CSC_SET_CSCMARTIX, FALSE},
    [E_DISP_TEST_VGA_SET_SHARPNESS]   =  {"SET_VGA_SHARPNESS", test_DISP_SetVgaSharpness,E_DISP_TEST_CSC_SET_CSCMARTIX,  FALSE},
    [E_DISP_TEST_CSC_SHOW_PARA]        = {"SHOW_PARA",         test_DISP_CscShowPara,     E_DISP_TEST_CSC_SET_CSCMARTIX, TRUE },
};

static int showMenu(int index)
{
    int i = 0;
    test_disp_TestCscMenu_t *pstMenu = NULL;
    printf("\n");
    do
    {
        pstMenu = &_gTestDispCsc[index + i];
        printf("[%d]: %s.\n", i, pstMenu->desc);
        i++;
        if (pstMenu->bEnd == TRUE)
        {
            break;
        }
    }while(1);
    printf("\n[%d]: %s.\n", i, "Quit");
    printf("Enter you Choice: \n");
    return i;
}

static void * test_vpe_TestIq (void *pData)
{
    MI_U8 DispDev = *((MI_U8 *)pData);
    int i = 0;
    int index = 0;
    int opt = 0;
    int max = 0;

    test_disp_TestCscMenu_t *pstMenu = NULL;
    printf("Welcome: DISP CSC test.\n");
    while(1)
    {
        max = showMenu(index);
        opt = getChoice();
        if ((opt < 0) || (opt > max))
        {
            printf("Invalid input option !!.\n");
            continue;
        }
        if (opt == max)
        {
            break;
        }
        pstMenu = &_gTestDispCsc[index + opt];
        if (pstMenu->func != NULL)
        {
            pstMenu->func(DispDev);
        }
        index = pstMenu->next_index;
    }

    _gbStop = TRUE;
    return 0;
}

//reset csc
#include <sys/time.h>
int main(int argc, const char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_U32 u32FrameNum;
    MI_BOOL bsleep = true;
    MI_BOOL bReset = false;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    memset(&stTv, 0, sizeof(stTv));

    time_t stTime = 0;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

    if(argc < 6)
    {
        printf("@@@@@ Test Pattern @@@@:\n [disp Dev layer filePath width heigt]\n");
        return 0;
    }
    int i = 0;
    for(i = 0; i < argc; i++)
    {
        DBG_INFO("Argument %d is %s./n", i, argv[i]);
    }
    MI_DISP_DEV DispDev = atoi(argv[1]);
    MI_DISP_LAYER DispLayer = atoi(argv[2]);
    const char* filePath = argv[3];
    MI_U16 u16Width = atoi(argv[4]);
    MI_U16 u16Height = atoi(argv[5]);

    DBG_INFO("File Path:%s, width = %d, height = %d \n", filePath, u16Width, u16Height);

    MIDISPCHECKRESULT(MI_SYS_Init());

    MI_SYS_ChnPort_t stDispChn0InputPort0;
    stDispChn0InputPort0.eModId = E_MI_MODULE_ID_DISP;
    stDispChn0InputPort0.u32DevId = DispDev;
    stDispChn0InputPort0.u32ChnId = 0;
    stDispChn0InputPort0.u32PortId = 0;
    // Test Device

    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_HdmiParam_t stHdmiParam;

    memset(&stPubAttr, 0, sizeof(stPubAttr));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_1080P60;
    if(DispDev == 0)
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    }
    else if(DispDev == 1)
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
    }
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_DISP_SUCCESS);

    mi_disp_hdmiInit();

    MI_U32 u32Toleration = 100000;
    MI_DISP_CompressAttr_t stCompressAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr, stLayerAttrBackup;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stVidLayerSize.u16Width = 1920;
    stLayerAttr.stVidLayerSize.u16Height= 1080;
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width    = 1920;
    stLayerAttr.stVidLayerDispWin.u16Height = 1080;
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    ExecFunc(MI_DISP_BindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetVideoLayerAttr(DispLayer, &stLayerAttr), MI_DISP_SUCCESS);
    DBG_INFO("Get Video Layer Size [%d, %d] !!!\n", stLayerAttr.stVidLayerSize.u16Width, stLayerAttr.stVidLayerSize.u16Height);
    DBG_INFO("Get Video Layer DispWin [%d, %d, %d, %d] !!!\n",\
        stLayerAttr.stVidLayerDispWin.u16X, stLayerAttr.stVidLayerDispWin.u16Y, stLayerAttr.stVidLayerDispWin.u16Width, stLayerAttr.stVidLayerDispWin.u16Height);

    ExecFunc(MI_DISP_EnableVideoLayer(DispLayer), MI_DISP_SUCCESS);

    MI_DISP_INPUTPORT LayerInputPort = 0;

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    stInputPortAttr.stDispWin.u16Width    = u16Width;
    stInputPortAttr.stDispWin.u16Height = u16Height;

    ExecFunc(MI_DISP_SetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_GetInputPortAttr(DispLayer, LayerInputPort, &stInputPortAttr), MI_DISP_SUCCESS);
    ExecFunc(MI_DISP_EnableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
    ExecFunc(MI_DISP_SetInputPortSyncMode(DispLayer, LayerInputPort, E_MI_DISP_SYNC_MODE_FREE_RUN), MI_SUCCESS);

    pthread_t thread;
    pthread_attr_t attr;

    int s = pthread_attr_init(&attr);
    if (s != 0)
        perror("pthread_attr_init");

    pthread_create(&thread, &attr, test_vpe_TestIq, &DispDev);

    int srcFd = 0;
    if (TRUE == test_disp_OpenSourceFile(filePath, &srcFd))
    {
        while(_gbStop == FALSE)
        {
            int n = 0;
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            gettimeofday(&stTv, NULL);

            stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
            stBufConf.stFrameCfg.u16Width = u16Width;
            stBufConf.stFrameCfg.u16Height = u16Height;
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

            gettimeofday(&stGetBuffer, NULL);
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stDispChn0InputPort0,&stBufConf,&stBufInfo,&hHandle, -1))
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;

                stBufInfo.bEndOfStream = FALSE;
                int size = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
                u16Height = stBufInfo.stFrameData.u16Height;
                u16Width  = stBufInfo.stFrameData.u16Width;
                gettimeofday(&stReadFile, NULL);

                if (test_disp_GetOneYuv422Frame(srcFd, 0, stBufInfo.stFrameData.pVirAddr[0], stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.u32Stride[0])== TRUE)
                {
                    gettimeofday(&stFlushData, NULL);
                    gettimeofday(&stPutBuffer, NULL);

                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                    gettimeofday(&stRelease, NULL);
                }
                else
                {
                    //DBG_INFO(" test_disp_GetOneFrame fail.\n");
                    test_disp_FdRewind(srcFd);
                    MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , TRUE);
                }

            }
            else
            {
                DBG_INFO("get buf fail\n");
            }
            usleep(30 * 1000);
        }
        pthread_join(thread, NULL);
        s = pthread_attr_destroy(&attr);
        if (s != 0)
        perror("pthread_attr_destroy");
    }
    else
    {
        DBG_INFO(" open file fail. \n");
    }

    DBG_INFO(" Prepare to Exit. \n");
    if(srcFd > 0)
    {
        test_disp_CloseFd(srcFd);
        ExecFunc(MI_DISP_DisableInputPort(DispLayer, LayerInputPort), MI_SUCCESS);
        ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_DISP_SUCCESS);
        ExecFunc(MI_DISP_Disable(DispDev), MI_DISP_SUCCESS);
    }

    DBG_INFO("####### disp test103 end ##############\n");

    return MI_SUCCESS;
}
