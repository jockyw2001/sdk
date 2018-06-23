
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include <pthread.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
//#include <linux/delay.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <linux/netlink.h>
#include <linux/fs.h>

#include "mdrv_msys_io.h"
#include "mdrv_msys_io_st.h"

#include "SimMiTest.h"
#include "drv_scl_verchk.h"
#include "SimMiModuleIoctl.h"
#include "SimMiModuleIoctlDataType.h"
//---------------------------------------------------------------
// define
//---------------------------------------------------------------

#define DISP_ALIGN(x,y)     ( x & ~(y-1))
#define DISP_MIN(x,y)       ( (x) < (y) ? x : y)

typedef struct
{
    int Case;
    unsigned long long MiuAddr;
    unsigned long long VirAddr;
    unsigned long long PhyAddr;
    unsigned long long Size;
    unsigned char *pVirAddr;
    char *Name;
    unsigned short ImageWidth;
    unsigned short ImageHeight;
    int DeviceId;
    int LayerId;
    SimMiModuleOuptTiming_e enTiming1;
    SimMiModuleOuptTiming_e enTiming2;
    SimMiModulePixelFormat_e enPixelFmt;
    unsigned short InputPortXNum;
    unsigned short InputPortYNum;
    unsigned long DelayTime;
    unsigned short u16Hpw;
    unsigned short u16Hbp;
    unsigned short u16Hactive;
    unsigned short u16Htt;
    unsigned short u16Vpw;
    unsigned short u16Vbp;
    unsigned short u16Vactive;
    unsigned short u16Vtt;
    unsigned short u16Fps;
}DispTestCaseConfig_t;

typedef struct
{
    SimMiModuleOuputInterface_e enInterface;
    SimMiModulePictureCSCType_e CscType;
    int u32Contrast;
    int u32Brightness;
    int u32Hue;
    int u32Saturation;
    int u32Sharpness;
    int u32Gain;
}DispTestPictureConfig_t;
//---------------------------------------------------------------
// enum
//---------------------------------------------------------------

extern g_FD[E_DRV_ID_NUM];

void UsagePrintf()
{
    printf("=========================================================================================================================\n");
    printf("Debug Level \n");
    printf("dbg [xc/ve/hdmitx/mhal_disp] [level] \n");
    printf("=========================================================================================================================\n");
    printf("Hdmitx Config \n");
    printf("hdmitx [CD] [IN COLOR] [OUT COLOR] \n");
    printf("    [CD]: 8 10 12 16\n");
    printf(" [COLOR]: rgb444, yuv444, yuv422\n");
    printf("=========================================================================================================================\n");
    printf("picture Config \n");
    printf("[INTERFACE] [Contrast] [Brightness] [Hue] [Saturation] [Sharpness] [Gain] [CSC]\n");
    printf("=========================================================================================================================\n");
    printf("DISP Test Case \n");
    printf("Usage:: case filename imagewidth imageheight");
    printf("case 0 : (FPGA) DISP0(480P, NTSC),   layer0(720x480), inputport0, 720x240, yuv420\n");
    printf("case 1 : (FPGA) DISP1(480P, NULL),   layer0(720x480), inputport1, 720x240, yuv420\n");
    printf("case 2 : (FPGA) DISP0(1080P, NTSC),  layer0(720x480), inpuport0,  720x240, yuv420\n");
    printf("case 3 : (FPGA) DISP0(480P, NTSC),   layer1(720x480), inpurpot0,  720x240, yuv420\n");
    printf("=========================================================================================================================\n");
    printf("Usage:: case filename imagewidth imageheight deviceid, layerid \n");
    printf("case 4:  (FPGA)  720P + NTSC + yuv420\n");
    printf("=========================================================================================================================\n");
    printf("Usage:: case, filename, pixelformat, imagewidth, imageheight, deviceid, layerid, timing1, timing2, xwin, ywin \n");
    printf("case 5: General case for disp0/disp1 \n");
    printf("case 16: ISR + Stream file\n");
    printf("=========================================================================================================================\n");
    printf("Usage:: case, filename, pixelformat, imagewidth, imageheight, 0, 0, timing1, timing2, 1 ,1\n");
    printf("case 6: attach \n");
    printf("case 7: HDMITx + VE + VGA \n");
    printf("case 8: source scaling\n");
    printf("case 10: MGwin0 + MGwin1 -> disp0 \n");
    printf("Case 11: H/V Swap + source scaling\n");
    printf("Case 12: MGwin0 + MGwin1 -> Device0 attach Device1 \n");
    printf("Case 13: CreateIsr\n");
    printf("=========================================================================================================================\n");
    printf("Usage:: case address pixelformat, imagewidth, imageheight, device layerid, timing1, timing2, xwin, ywin \n");
    printf("case 9: \n");
    printf("=========================================================================================================================\n");
    printf("Usage lcd config \n");
    printf("lcd filename pixelformat, imagewidth, imageheight, Hpw, Hbp, Hactive, Htt, Vpw, Vbp, Vactive, Vtt, Fps \n");
    printf("lcd filename pixelformat, imagewidth, imageheight, 0, 0, timing1, NULL, 1 1\n");
    printf("case 15:  LCD \n");
    printf("=========================================================================================================================\n");

}

SimMiModuleHdmitxColorType_e Parsing_Hdmitx_Color(char *pColor)
{
    SimMiModuleHdmitxColorType_e enColor;

    enColor = (strcmp(pColor, "rgb444") == 0) ? E_SIMMI_MODULE_HDMITX_COLOR_RGB444 :
              (strcmp(pColor, "yuv444") == 0) ? E_SIMMI_MODULE_HDMITX_COLOR_YUV444 :
              (strcmp(pColor, "yuv422") == 0) ? E_SIMMI_MODULE_HDMITX_COLOR_YUV422 :
                                                E_SIMMI_MODULE_HDMITX_COLOR_NUM;

    return enColor;
}

SimMiModuleHdmitxColorDepthType_e Parsing_Hdmitx_ColorDepth(char *pColorDepth)
{
    SimMiModuleHdmitxColorDepthType_e enColorDepth;
    enColorDepth  = (strcmp(pColorDepth, "8") == 0)  ? E_SIMMI_MODULE_HDMITX_CD_8_BIT :
                    (strcmp(pColorDepth, "10") == 0) ? E_SIMMI_MODULE_HDMITX_CD_10_BIT :
                    (strcmp(pColorDepth, "12") == 0) ? E_SIMMI_MODULE_HDMITX_CD_12_BIT :
                    (strcmp(pColorDepth, "16") == 0) ? E_SIMMI_MODULE_HDMITX_CD_16_BIT :
                                                       E_SIMMI_MODULE_HDMITX_CD_NUM;

    return enColorDepth;
}

SimMiModuleOuputInterface_e Parsing_Interface(char *pInterface)
{
    SimMiModuleOuputInterface_e enInterface;
    enInterface  =  (strcmp(pInterface, "hdmi") == 0) ? E_SIMMI_MODULE_OUTPUT_HDMI :
                    (strcmp(pInterface, "vga")  == 0) ? E_SIMMI_MODULE_OUTPUT_VGA :
                    (strcmp(pInterface, "cvbs") == 0) ? E_SIMMI_MODULE_OUTPUT_CVBS :
                                                        E_SIMMI_MODULE_OUTPUT_NUM;
    return enInterface;
}

SimMiModuleOuptTiming_e Parsing_Timing(char *pTiming)
{
    SimMiModuleOuptTiming_e enTiming;

    enTiming =  (strcmp(pTiming, "NTSC")      == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_NTSC         :
                (strcmp(pTiming, "PAL")       == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_PAL          :
                (strcmp(pTiming, "480P")      == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_480P         :
                (strcmp(pTiming, "576P")      == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_576P         :
                (strcmp(pTiming, "720P50")    == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_720P50       :
                (strcmp(pTiming, "720P60")    == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_720P60       :
                (strcmp(pTiming, "1080P24")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P24      :
                (strcmp(pTiming, "1080P25")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P25      :
                (strcmp(pTiming, "1080P30")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P30      :
                (strcmp(pTiming, "1080P50")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P50      :
                (strcmp(pTiming, "1080P60")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P60      :
                (strcmp(pTiming, "4K2K30")    == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_4K2K_30P     :
                (strcmp(pTiming, "640x480")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_640x480_60   :
                (strcmp(pTiming, "800x600")   == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_800x600_60   :
                (strcmp(pTiming, "1024x768")  == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1024x768_60  :
                (strcmp(pTiming, "1280x1024") == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1280x1024_60 :
                (strcmp(pTiming, "1366x768")  == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1366x768_60  :
                (strcmp(pTiming, "1440x900")  == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1440x900_60  :
                (strcmp(pTiming, "1280x800")  == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1280x800_60  :
                (strcmp(pTiming, "1680x1050") == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1680x1050_60 :
                (strcmp(pTiming, "1600x1200") == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1600x1200_60 :
                (strcmp(pTiming, "1920x1200") == 0) ? E_SIMMI_MODULE_OUTPUT_TIMING_1920x1200_60 :
                (strcmp(pTiming, "USER") == 0)      ? E_SIMMI_MODULE_OUTPUT_TIMING_USER :
                                                      E_SIMMI_MODULE_OUTPUT_TIMING_NUM;

    if(enTiming == E_SIMMI_MODULE_OUTPUT_TIMING_NUM)
    {
        printf("Unknow Timing: %s\n", pTiming);
    }

    return enTiming;
}

SimMiModulePixelFormat_e Parsing_PixelFormat(char *pPixelFormat)
{
    SimMiModulePixelFormat_e enPixelFormat;

    enPixelFormat = (strcmp(pPixelFormat, "argb8888") == 0) ? E_SIMMI_MODULE_PIXEL_FORMAT_ARGB8888 :
                    (strcmp(pPixelFormat, "rgb565")   == 0) ? E_SIMMI_MODULE_PIXEL_FORMAT_RGB565 :
                    (strcmp(pPixelFormat, "yuv422")   == 0) ? E_SIMMI_MODULE_PIXEL_FORMAT_YUV422 :
                    (strcmp(pPixelFormat, "yuv420")   == 0) ? E_SIMMI_MODULE_PIXEL_FORMAT_YUV420 :
                                                              E_SIMMI_MODULE_PIXEL_FORMAT_NUM;

    if(enPixelFormat == E_SIMMI_MODULE_PIXEL_FORMAT_NUM)
    {
        printf("Known Fromat=%s \n", enPixelFormat);
    }

    return enPixelFormat;
}
SimMiModulePictureCSCType_e Parsing_CscType(char *pCsc)
{
    SimMiModulePictureCSCType_e enCsc;

    if(strcmp(pCsc, "601to709") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_BT601_TO_BT709;
    }
    else if(strcmp(pCsc, "709to601") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_BT709_TO_BT601;
    }
    else if(strcmp(pCsc, "709toRGB") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_BT709_TO_RGB_PC;
    }
    else if(strcmp(pCsc, "601torgb") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_BT601_TO_RGB_PC;
    }
    else if(strcmp(pCsc, "rgbto709") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_RGB_TO_BT709_PC;
    }
    else if(strcmp(pCsc, "rgbto601") == 0)
    {
        enCsc = E_SIMMI_MODULE_MATRIX_RGB_TO_BT601_PC;
    }
    else
    {
        enCsc = E_SIMMI_MODULE_MATRIX_BYPASS;
    }
    return enCsc;
}

//----------------------------------------------------------------------------------
// Test Function
//----------------------------------------------------------------------------------
u32 Test_DISP_TransStrideSize(SimMiModulePixelFormat_e enPixelFmt, u32 u32Stride)
{
    u32 u32StrideSize;

    switch(enPixelFmt)
    {
        case E_SIMMI_MODULE_PIXEL_FORMAT_ARGB8888:
            u32StrideSize = u32Stride * 4;
            break;

        case E_SIMMI_MODULE_PIXEL_FORMAT_YUV422:
        case E_SIMMI_MODULE_PIXEL_FORMAT_RGB565:
            u32StrideSize = u32Stride * 2;
            break;

        default:
        case E_SIMMI_MODULE_PIXEL_FORMAT_YUV420:
            u32StrideSize = u32Stride * 3 / 2;
            break;
    }
    return u32StrideSize;
}

void Test_DISP_GetSizeByTiming(SimMiModuleOuptTiming_e enTiming, bool bHVSwap, u16 *pu16Width, u16 *pu16Height)
{
    switch(enTiming)
    {
        default:
        case E_SIMMI_MODULE_OUTPUT_TIMING_NTSC:
            *pu16Width  = 720;
            *pu16Height = 480;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_PAL:
            *pu16Width  = 720;
            *pu16Height = 576;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_480P:
            *pu16Width  = 720;
            *pu16Height = 480;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_576P:
            *pu16Width  = 720;
            *pu16Height = 576;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_720P50:
        case E_SIMMI_MODULE_OUTPUT_TIMING_720P60:
            *pu16Width  = 1280;
            *pu16Height = 720;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1080P24:
        case E_SIMMI_MODULE_OUTPUT_TIMING_1080P25:
        case E_SIMMI_MODULE_OUTPUT_TIMING_1080P30:
        case E_SIMMI_MODULE_OUTPUT_TIMING_1080P50:
        case E_SIMMI_MODULE_OUTPUT_TIMING_1080P60:
            *pu16Width  = 1920;
            *pu16Height = 1080;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_4K2K_30P:
            *pu16Width  = 3840;
            *pu16Height = 2160;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_640x480_60:
            *pu16Width  = 640;
            *pu16Height = 480;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_800x600_60:
            *pu16Width  = 800;
            *pu16Height = 600;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1024x768_60:
            *pu16Width  = 1024;
            *pu16Height = 768;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1280x1024_60:
            *pu16Width  = 1280;
            *pu16Height = 1024;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1366x768_60:
            *pu16Width  = 1366;
            *pu16Height = 768;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1440x900_60:
            *pu16Width  = 1440;
            *pu16Height = 900;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1280x800_60:
            *pu16Width  = 1280;
            *pu16Height = 800;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1680x1050_60:
            *pu16Width  = 1680;
            *pu16Height = 1050;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1600x1200_60:
            *pu16Width  = 1600;
            *pu16Height = 1200;
            break;
        case E_SIMMI_MODULE_OUTPUT_TIMING_1920x1200_60:
            *pu16Width  = 1920;
            *pu16Height = 1200;
            break;
    }

    if(bHVSwap)
    {
        u16 u16Tmp;
        u16Tmp= *pu16Width;
        *pu16Width = *pu16Height;
        *pu16Height = u16Tmp;
    }
}

int Test_DISP_ReadBinToMemory(char *FileName, unsigned char *DstMem, unsigned int Size, unsigned long *pFileSize)
{
    FILE *fp;
    int FileLen;
    int ret = 1;

    fp =fopen(FileName, "rb");

    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        FileLen=ftell(fp);
        fseek(fp, 0, SEEK_SET);

        SIM_MI_DBG("%s %d, FileName:%s, Size=%x\n", __FUNCTION__, __LINE__, FileName, FileLen);

        if(FileLen > Size)
        {
            ret = 0;

            SIM_MI_DBG("%s %d, FileName:%s is too big, fileSize=%d, MemSize=%d\n", __FUNCTION__, __LINE__, FileName, FileLen, Size);
        }
        else
        {
    	    fread(DstMem, FileLen, 1, fp);
    	    *pFileSize = FileLen;
    	    ret = 1;
    	}

    	fclose(fp);
    }
    else
    {
        ret = 0;
        printf("Read %s Fail\n", FileName);
    }
    return ret;
}


void Test_DISP_Debug(char *pType, unsigned long debuglevel)
{
    SimMiModuleDispDebguConfig_t stDebugCfg;

    stDebugCfg.enType = (strcmp(pType, "hdmitx") == 0)  ? E_SIMMI_MODULE_DEBUG_LEVEL_HDMITX :
                        (strcmp(pType, "xc") == 0)      ? E_SIMMI_MODULE_DEBUG_LEVEL_XC :
                        (strcmp(pType, "ve") == 0)      ? E_SIMMI_MODULE_DEBUG_LEVEL_VE :
                        (strcmp(pType, "mgwin") == 0)   ? E_SIMMI_MODULE_DEBUG_LEVEL_MGWIN :
                        (strcmp(pType, "misc") == 0)    ? E_SIMMI_MODULE_DEBUG_LEVEL_MISC :
                        (strcmp(pType, "pattern") == 0) ? E_SIMMI_MODULE_DEBUG_LEVEL_TEST_PATTERN :
                                                          E_SIMMI_MODULE_DEBUG_LEVEL_MHAL_DISP;
    stDebugCfg.u32Flag = debuglevel;


    if(ioctl(g_FD[E_DRV_ID_SIMMI_MODULE], IOCTL_SIMMI_SET_DEBUG_LEVEL_CONFIG, &stDebugCfg))
    {
        printf("%s %d, IOCTL fail\n", __FUNCTION__, __LINE__ );
    }

}

void Test_DISP_Picture(DispTestPictureConfig_t stCfg)
{
    SimMiModuleDispPictureConfig_t stPictureCfg;


    stPictureCfg.enInterface = stCfg.enInterface;
    stPictureCfg.enCscType = stCfg.CscType;
    stPictureCfg.u32Contrast = stCfg.u32Contrast;
    stPictureCfg.u32Brightness = stCfg.u32Brightness;
    stPictureCfg.u32Hue        = stCfg.u32Hue;
    stPictureCfg.u32Saturation = stCfg.u32Saturation;
    stPictureCfg.u32Sharpness  = stCfg.u32Sharpness;
    stPictureCfg.u32Gain       = stCfg.u32Gain;
    printf("==================================================================================\n");
    printf("PICTURE: Contrast:%d, Brightness:%d, Hue:%d, Sat:%d, Sharpness:%d, Gain:%d, CSC:%d\n",
        stPictureCfg.u32Contrast, stPictureCfg.u32Brightness,
        stPictureCfg.u32Hue, stPictureCfg.u32Saturation,
        stPictureCfg.u32Sharpness, stPictureCfg.u32Gain,
        stPictureCfg.enCscType);
    printf("==================================================================================\n");
    usleep(100);

    if(ioctl(g_FD[E_DRV_ID_SIMMI_MODULE], IOCTL_SIMMI_SET_PICTURE_CONFIG, &stPictureCfg))
    {
        printf("IOCTL Fail\n");
    }
}



void Test_DISP_Hdmitx(SimMiModuleHdmitxColorDepthType_e enColorDepth, SimMiModuleHdmitxColorType_e enInColor, SimMiModuleHdmitxColorType_e enOutColor)
{
    SimMiModuleDispHdmitxConfig_t stHdmitxCfg;

    stHdmitxCfg.enColorDepth = enColorDepth;
    stHdmitxCfg.enOutColorType = enOutColor;
    stHdmitxCfg.enInColorType = enInColor;

    if(ioctl(g_FD[E_DRV_ID_SIMMI_MODULE], IOCTL_SIMMI_SET_HDMITX_CONFIG, &stHdmitxCfg))
    {
        printf("IOCTL Fail\n");
    }
}


void Test_DISP_Case(DispTestCaseConfig_t *pCfg)
{
    SimMiModuleDispConfig_t stDispCfg;
    int bParamSet = 1;
    int x, y;
    unsigned long FileSize= 0;

    memset(&stDispCfg, 0, sizeof(SimMiModuleDispConfig_t));
    printf("==================================================================================\n");
    printf("Case_%d, FileName=%s, Format=%d (%d %d), ID:(%d %d) Timing(%d %d) XY(%d %d)\n",
        pCfg->Case, pCfg->Name,
        pCfg->enPixelFmt, pCfg->ImageWidth, pCfg->ImageHeight, pCfg->DeviceId, pCfg->LayerId,
        pCfg->enTiming1, pCfg->enTiming2, pCfg->InputPortXNum, pCfg->InputPortYNum);
    printf("==================================================================================\n");
    usleep(100);

    switch(pCfg->Case)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            if(pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0)
            {
                printf("Fail to read bin \n");
                bParamSet = 0;
            }
            else
            {
                stDispCfg.u8DeivceNum = 1;
                stDispCfg.u8VideoLayerNum = 1;

                stDispCfg.stDeviceCfg[0].enDeviceId = pCfg->Case == 1 ?  E_SIMMI_MODULE_DEVICE_ID_1 : E_SIMMI_MODULE_DEVICE_ID_0;

                stDispCfg.stDeviceCfg[0].enInterface[0] = pCfg->Case == 1 ? E_SIMMI_MODULE_OUTPUT_VGA : E_SIMMI_MODULE_OUTPUT_HDMI;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = pCfg->Case == 2 ? E_SIMMI_MODULE_OUTPUT_TIMING_1080P60 : E_SIMMI_MODULE_OUTPUT_TIMING_480P;

                stDispCfg.stDeviceCfg[0].enInterface[1] = E_SIMMI_MODULE_OUTPUT_CVBS;
                stDispCfg.stDeviceCfg[0].enTiming[1]    = E_SIMMI_MODULE_OUTPUT_TIMING_NTSC;

                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = pCfg->Case == 3 ?  E_SIMMI_MODULE_VIDEOLAYER_ID_1 : E_SIMMI_MODULE_VIDEOLAYER_ID_0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = 720;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = 480;
                stDispCfg.stVideoLayerCfg[0].u16Width = 720;
                stDispCfg.stVideoLayerCfg[0].u16Height = 480;

                stDispCfg.u16InputPortFlag[0] = 1;
                stDispCfg.stInputPortCfg[0][0].u8PortId = 0;
                stDispCfg.stInputPortCfg[0][0].enPixelFormat = E_SIMMI_MODULE_PIXEL_FORMAT_YUV420;
                stDispCfg.stInputPortCfg[0][0].u16X = 0;
                stDispCfg.stInputPortCfg[0][0].u16Y = 0;
                stDispCfg.stInputPortCfg[0][0].u16Height = 240;
                stDispCfg.stInputPortCfg[0][0].u16Width = 720;
                stDispCfg.stInputPortCfg[0][0].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[0][0].u32Stride =  Test_DISP_TransStrideSize(E_SIMMI_MODULE_PIXEL_FORMAT_YUV420, 240);
                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;
            }
            break;
        case 4:
        {
            if(pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0)
            {
                printf("Fail to read bin \n");
                bParamSet = 0;
            }
            else
            {
                stDispCfg.u8DeivceNum = 1;
                stDispCfg.u8VideoLayerNum = 1;

                stDispCfg.stDeviceCfg[0].enDeviceId = pCfg->DeviceId == 1 ?  E_SIMMI_MODULE_DEVICE_ID_1 : E_SIMMI_MODULE_DEVICE_ID_0;

                stDispCfg.stDeviceCfg[0].enInterface[0] = pCfg->DeviceId == 1 ? E_SIMMI_MODULE_OUTPUT_VGA : E_SIMMI_MODULE_OUTPUT_HDMI;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = E_SIMMI_MODULE_OUTPUT_TIMING_480P;

                stDispCfg.stDeviceCfg[0].enInterface[1] = E_SIMMI_MODULE_OUTPUT_CVBS;
                stDispCfg.stDeviceCfg[0].enTiming[1]    = E_SIMMI_MODULE_OUTPUT_TIMING_NTSC;

                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = pCfg->LayerId == 1 ?  E_SIMMI_MODULE_VIDEOLAYER_ID_1 : E_SIMMI_MODULE_VIDEOLAYER_ID_0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = 720;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = 480;
                stDispCfg.stVideoLayerCfg[0].u16Width = 720;
                stDispCfg.stVideoLayerCfg[0].u16Height = 480;

                stDispCfg.u16InputPortFlag[0] = 1;
                stDispCfg.stInputPortCfg[0][0].u8PortId = 0;
                stDispCfg.stInputPortCfg[0][0].enPixelFormat = E_SIMMI_MODULE_PIXEL_FORMAT_YUV420;
                stDispCfg.stInputPortCfg[0][0].u16X = 0;
                stDispCfg.stInputPortCfg[0][0].u16Y = 0;
                stDispCfg.stInputPortCfg[0][0].u16Height = pCfg->ImageHeight;
                stDispCfg.stInputPortCfg[0][0].u16Width = pCfg->ImageWidth;
                stDispCfg.stInputPortCfg[0][0].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[0][0].u32Stride = Test_DISP_TransStrideSize(E_SIMMI_MODULE_PIXEL_FORMAT_YUV420, pCfg->ImageWidth);
                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;
            }
            break;

        }
        case 5:
        case 9:
        case 16:
        {
            if(pCfg->Case == 5 || pCfg->Case == 16)
            {
                if(pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0)
                {
                    printf("Fail to read bin \n");
                    bParamSet = 0;
                }
            }
            else
            {
                if(pCfg->Name == NULL)
                {
                    bParamSet == 0;
                }
                else
                {
                    pCfg->MiuAddr = Parsing_HexNum(pCfg->Name);
                    FileSize =  Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth) * pCfg->ImageHeight;
                }
            }


            if(bParamSet)
            {
                unsigned short u16TimingWidth, u16TimingHeight;
                unsigned short u16InputPortWidth, u16InputPortHeight;
                unsigned char  u8InputPortCnt;
                unsigned short x, y;

                stDispCfg.u8DeivceNum = 1;
                stDispCfg.u8VideoLayerNum = 1;
                stDispCfg.bAttached = 0;

                stDispCfg.stDeviceCfg[0].enDeviceId = pCfg->DeviceId == 1 ?  E_SIMMI_MODULE_DEVICE_ID_1 : E_SIMMI_MODULE_DEVICE_ID_0;

                stDispCfg.stDeviceCfg[0].enInterface[0] = pCfg->DeviceId == 1 ? E_SIMMI_MODULE_OUTPUT_VGA : E_SIMMI_MODULE_OUTPUT_HDMI;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = pCfg->enTiming1;

                stDispCfg.stDeviceCfg[0].enInterface[1] = E_SIMMI_MODULE_OUTPUT_CVBS;
                stDispCfg.stDeviceCfg[0].enTiming[1]    = pCfg->enTiming2;;

                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = pCfg->LayerId == 1 ?  E_SIMMI_MODULE_VIDEOLAYER_ID_1 : E_SIMMI_MODULE_VIDEOLAYER_ID_0;

                Test_DISP_GetSizeByTiming(pCfg->enTiming1, 0, &u16TimingWidth, &u16TimingHeight);

                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = u16TimingHeight;
                stDispCfg.stVideoLayerCfg[0].u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].u16Height = u16TimingHeight;

                u16InputPortWidth  = DISP_ALIGN( DISP_MIN(pCfg->ImageWidth,  (u16TimingWidth / pCfg->InputPortXNum)), 16);
                u16InputPortHeight = DISP_ALIGN( DISP_MIN(pCfg->ImageHeight, (u16TimingHeight/ pCfg->InputPortYNum)), 2);
                u8InputPortCnt = 0;

                stDispCfg.u16InputPortFlag[0] = 0;
                for(x = 0; x < pCfg->InputPortXNum; x++)
                {
                    for(y = 0; y < pCfg->InputPortYNum; y++)
                    {
                        stDispCfg.u16InputPortFlag[0] |=  (1<<u8InputPortCnt);
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u8PortId = u8InputPortCnt;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].enPixelFormat = pCfg->enPixelFmt;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16X = 0 + x*u16InputPortWidth;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Y = 0 + y*u16InputPortHeight;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Height = u16InputPortHeight;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Width = u16InputPortWidth;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32BaseAddr = pCfg->MiuAddr;
                        stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32Stride = Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth);
                        u8InputPortCnt++;
                    }
                }
                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;

                stDispCfg.u32ImageBufferAddr = pCfg->MiuAddr;
                stDispCfg.u32ImageBufferSize = FileSize;

                stDispCfg.bAttached  = 0;
                stDispCfg.bMgwin32En = 0;
                stDispCfg.bIsr       = pCfg->Case == 16 ? 1 : 0;
                stDispCfg.bHVSwap    = 0;
                stDispCfg.bLCD       = 0;
            }
            break;
        }

        case 6:
        case 7:
        case 10:
        case 12:
        case 13:
        {
            if((pCfg->Case != 11) && (pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0))
            {
                printf("Fail to read bin \n");
                bParamSet = 0;
            }
            else
            {
                unsigned short u16TimingWidth, u16TimingHeight;
                unsigned char  u8InputPortCnt;
                unsigned short x, y;

                stDispCfg.u8DeivceNum     = (pCfg->Case == 6 || pCfg->Case == 7 || pCfg->Case == 12) ? 2 : 1;
                stDispCfg.u8VideoLayerNum = (pCfg->Case == 6 || pCfg->Case == 7 || pCfg->Case == 12) ? 2 : 1;

                stDispCfg.stDeviceCfg[0].enDeviceId = E_SIMMI_MODULE_DEVICE_ID_0;
                stDispCfg.stDeviceCfg[1].enDeviceId = pCfg->Case == 10 ? E_SIMMI_MODULE_DEVICE_ID_0 : E_SIMMI_MODULE_DEVICE_ID_1;

                stDispCfg.stDeviceCfg[0].enInterface[0] = E_SIMMI_MODULE_OUTPUT_HDMI;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = pCfg->enTiming1;
                stDispCfg.stDeviceCfg[0].enInterface[1] = E_SIMMI_MODULE_OUTPUT_CVBS;
                stDispCfg.stDeviceCfg[0].enTiming[1]    = pCfg->Case == 10 ?  pCfg->enTiming2 : E_SIMMI_MODULE_OUTPUT_TIMING_NTSC;

                if(pCfg->Case == 6 || pCfg->Case == 7 || pCfg->Case == 12)
                {
                    stDispCfg.stDeviceCfg[1].enInterface[0] = E_SIMMI_MODULE_OUTPUT_VGA;
                    stDispCfg.stDeviceCfg[1].enTiming[0]    = pCfg->enTiming2;
                }

                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = E_SIMMI_MODULE_VIDEOLAYER_ID_0;
                stDispCfg.stVideoLayerCfg[1].enVideoLayerId = E_SIMMI_MODULE_VIDEOLAYER_ID_1;

                u8InputPortCnt = 0;

                Test_DISP_GetSizeByTiming(pCfg->enTiming1, 0, &u16TimingWidth, &u16TimingHeight);
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = u16TimingHeight;
                stDispCfg.stVideoLayerCfg[0].u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].u16Height = u16TimingHeight;
                stDispCfg.u16InputPortFlag[0] = 1;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u8PortId = u8InputPortCnt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].enPixelFormat = pCfg->enPixelFmt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16X = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Y = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Height = pCfg->ImageHeight;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Width = pCfg->ImageWidth;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32Stride = Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth );

                Test_DISP_GetSizeByTiming((pCfg->Case == 10)  ? pCfg->enTiming1 : pCfg->enTiming2, 0, &u16TimingWidth, &u16TimingHeight);
                stDispCfg.stVideoLayerCfg[1].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[1].stDispRect.u16Y =  0;
                stDispCfg.stVideoLayerCfg[1].stDispRect.u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[1].stDispRect.u16Height = u16TimingHeight;
                stDispCfg.stVideoLayerCfg[1].u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[1].u16Height = u16TimingHeight;
                stDispCfg.u16InputPortFlag[1] = 1;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u8PortId = u8InputPortCnt;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].enPixelFormat = pCfg->enPixelFmt;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u16X = (pCfg->Case == 10 || pCfg->Case == 12) ?  50 : 0;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u16Y = (pCfg->Case == 10 || pCfg->Case == 12) ?  50 : 0;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u16Height = pCfg->ImageHeight;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u16Width = pCfg->ImageWidth;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[1][u8InputPortCnt].u32Stride = Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth);

                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;

                stDispCfg.u32ImageBufferAddr = pCfg->MiuAddr;
                stDispCfg.u32ImageBufferSize = FileSize;

                stDispCfg.bAttached  = (pCfg->Case == 6  || pCfg->Case == 12) ? 1 : 0;
                stDispCfg.bMgwin32En = (pCfg->Case == 10 || pCfg->Case == 12) ? 1 : 0;
                stDispCfg.bIsr       = (pCfg->Case == 13) ? 1 : 0;
                stDispCfg.bHVSwap = 0;
                stDispCfg.bLCD = 0;
            }

            break;
        }

        case 8:
        case 11:
        {

            if(pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0)
            {
                printf("Fail to read bin \n");
                bParamSet = 0;
            }
            else
            {
                unsigned short u16TimingWidth, u16TimingHeight;
                unsigned short u16InputPortWidth, u16InputPortHeight;
                unsigned char  u8InputPortCnt;
                unsigned short x, y;
                bool    bHVSap = pCfg->Case == 11 ? 1 : 0;

                stDispCfg.u8DeivceNum = 1;
                stDispCfg.u8VideoLayerNum = 1;

                stDispCfg.stDeviceCfg[0].enDeviceId = pCfg->DeviceId == 1 ?  E_SIMMI_MODULE_DEVICE_ID_1 : E_SIMMI_MODULE_DEVICE_ID_0;

                stDispCfg.stDeviceCfg[0].enInterface[0] = pCfg->DeviceId == 1 ? E_SIMMI_MODULE_OUTPUT_VGA:  E_SIMMI_MODULE_OUTPUT_HDMI;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = pCfg->enTiming1;
                stDispCfg.stDeviceCfg[0].enInterface[1] = E_SIMMI_MODULE_OUTPUT_CVBS;
                stDispCfg.stDeviceCfg[0].enTiming[1]    = pCfg->enTiming2;

                stDispCfg.stDeviceCfg[1].enInterface[0] = E_SIMMI_MODULE_OUTPUT_VGA;
                stDispCfg.stDeviceCfg[1].enTiming[0]    = pCfg->enTiming2;


                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = pCfg->LayerId == 1 ?  E_SIMMI_MODULE_VIDEOLAYER_ID_1 : E_SIMMI_MODULE_VIDEOLAYER_ID_0;;

                u8InputPortCnt = 0;

                Test_DISP_GetSizeByTiming(pCfg->enTiming1, bHVSap, &u16TimingWidth, &u16TimingHeight);
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = u16TimingHeight; // for XC display window
                stDispCfg.stVideoLayerCfg[0].u16Width = pCfg->ImageWidth;
                stDispCfg.stVideoLayerCfg[0].u16Height = pCfg->ImageHeight; // for xc capture windows
                stDispCfg.u16InputPortFlag[0] = 1;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u8PortId = u8InputPortCnt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].enPixelFormat = pCfg->enPixelFmt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16X = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Y = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Height = pCfg->ImageHeight;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Width = pCfg->ImageWidth;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32Stride = Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth);

                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;

                stDispCfg.u32ImageBufferAddr = pCfg->MiuAddr;
                stDispCfg.u32ImageBufferSize = FileSize;

                stDispCfg.bAttached = 0;
                stDispCfg.bHVSwap = bHVSap;
                stDispCfg.bLCD = 0;
            }

            break;
        }

        case 15:
        {
            if(pCfg->Name == NULL || Test_DISP_ReadBinToMemory(pCfg->Name, pCfg->pVirAddr, pCfg->Size, &FileSize) == 0)
            {
                printf("Fail to read bin \n");
                bParamSet = 0;
            }
            else
            {
                unsigned short u16TimingWidth, u16TimingHeight;
                unsigned short u16InputPortWidth, u16InputPortHeight;
                unsigned char  u8InputPortCnt;
                unsigned short x, y;
                bool    bHVSap = 0;

                stDispCfg.u8DeivceNum = 1;
                stDispCfg.u8VideoLayerNum = 1;

                stDispCfg.stDeviceCfg[0].enDeviceId = E_SIMMI_MODULE_DEVICE_ID_0;

                stDispCfg.stDeviceCfg[0].enInterface[0] = E_SIMMI_MODULE_OUTPUT_LCD;
                stDispCfg.stDeviceCfg[0].enTiming[0]    = pCfg->enTiming1;

                if(pCfg->enTiming1 == E_SIMMI_MODULE_OUTPUT_TIMING_USER)
                {
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Hpw     = pCfg->u16Hpw;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Hbp     = pCfg->u16Hbp;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Hactive = pCfg->u16Hactive;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Htt     = pCfg->u16Htt;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Vpw     = pCfg->u16Vpw;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Vbp     = pCfg->u16Vbp;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Vactive = pCfg->u16Vactive;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Vtt     = pCfg->u16Vtt;
                    stDispCfg.stDeviceCfg[0].stUserTimingCfg[0].u16Fps     = pCfg->u16Fps;
                }

                stDispCfg.stVideoLayerCfg[0].enVideoLayerId = pCfg->LayerId == 1 ?  E_SIMMI_MODULE_VIDEOLAYER_ID_1 : E_SIMMI_MODULE_VIDEOLAYER_ID_0;;

                u8InputPortCnt = 0;

                if(pCfg->enTiming1 == E_SIMMI_MODULE_OUTPUT_TIMING_USER)
                {
                    u16TimingWidth = pCfg->u16Hactive;
                    u16TimingHeight = pCfg->u16Vactive;
                }
                else
                {
                    Test_DISP_GetSizeByTiming(pCfg->enTiming1, bHVSap, &u16TimingWidth, &u16TimingHeight);
                }
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16X = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Y = 0;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Width = u16TimingWidth;
                stDispCfg.stVideoLayerCfg[0].stDispRect.u16Height = u16TimingHeight; // for XC display window
                stDispCfg.stVideoLayerCfg[0].u16Width = pCfg->ImageWidth;
                stDispCfg.stVideoLayerCfg[0].u16Height = pCfg->ImageHeight; // for xc capture windows
                stDispCfg.u16InputPortFlag[0] = 1;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u8PortId = u8InputPortCnt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].enPixelFormat = pCfg->enPixelFmt;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16X = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Y = 0;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Height = pCfg->ImageHeight;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u16Width = pCfg->ImageWidth;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32BaseAddr = pCfg->MiuAddr;
                stDispCfg.stInputPortCfg[0][u8InputPortCnt].u32Stride = Test_DISP_TransStrideSize(pCfg->enPixelFmt, pCfg->ImageWidth);

                stDispCfg.u32DemoTimeSec = pCfg->DelayTime;

                stDispCfg.u32ImageBufferAddr = pCfg->MiuAddr;
                stDispCfg.u32ImageBufferSize = FileSize;

                stDispCfg.bAttached = 0;
                stDispCfg.bHVSwap = bHVSap;
                stDispCfg.bLCD = 1;
            }

            break;
        }

        default:
            bParamSet = 0;
            break;
    }


    if(bParamSet)
    {
        if(ioctl(g_FD[E_DRV_ID_SIMMI_MODULE], IOCTL_SIMMI_DISP_CONFIG, &stDispCfg))
        {
            printf("IOCTL Fail\n");
        }
    }
    else
    {
        UsagePrintf();
    }
}


int Test_DISP_Init(DispTestCaseConfig_t *pstDispCaseCfg)
{
    #define BUFF_SIZE  (3840*2016*2)

    MSYS_DMEM_INFO stMsysMemInfo;
    MSYS_ADDR_TRANSLATION_INFO stMsysTransInfo;

    stMsysMemInfo = _AllocateDmem("DISP", BUFF_SIZE );

    if(stMsysMemInfo.kvirt == 0)
    {
        SIM_MI_DBG("%s:++++++++++++++++ allocate memory fail ++++++++++++++++\n", __FUNCTION__);
        stMsysTransInfo.addr = 0x6040000;
    }
    else
    {
        stMsysTransInfo = _TranslationPhyToMIU(stMsysMemInfo);
    }

    SIM_MI_DBG("%s %llx, MIU=%x \n",  __FUNCTION__, __LINE__, stMsysTransInfo.addr);

    pstDispCaseCfg->MiuAddr = stMsysTransInfo.addr;
    pstDispCaseCfg->PhyAddr = stMsysMemInfo.phys;
    pstDispCaseCfg->VirAddr = stMsysMemInfo.kvirt;
    pstDispCaseCfg->Size = BUFF_SIZE;

    pstDispCaseCfg->pVirAddr = mmap(NULL, stMsysMemInfo.length, PROT_READ|PROT_WRITE,MAP_SHARED, g_FD[E_DRV_ID_MEM_MODULE], (stMsysMemInfo.phys));

    if(pstDispCaseCfg->pVirAddr == NULL)
    {
        printf("mmap fail\n");
        return 0;
    }
    else
    {
        return 1;
    }
}


void Test_DISP(int argc, char *argv[])
{

    DispTestCaseConfig_t stDispCaseCfg;
    int i;

    Open_Device(E_DRV_ID_SIMMI_MODULE);
    Open_Device(E_DRV_ID_MSYS_MODULE);
    Open_Device(E_DRV_ID_MEM_MODULE);

    if(argc < 2)
    {
        UsagePrintf();
        return ;
    }

    memset(&stDispCaseCfg, 0, sizeof(DispTestCaseConfig_t));

    if(strcmp(argv[1], "picture") == 0)
    {
        if(argc == 10)
        {
            DispTestPictureConfig_t stPictureCfg;

            stPictureCfg.enInterface = Parsing_Interface(argv[2]);
            stPictureCfg.u32Contrast = Parsing_Num(argv[3]);
            stPictureCfg.u32Brightness = Parsing_Num(argv[4]);
            stPictureCfg.u32Hue        = Parsing_Num(argv[5]);
            stPictureCfg.u32Saturation = Parsing_Num(argv[6]);
            stPictureCfg.u32Sharpness  = Parsing_Num(argv[7]);
            stPictureCfg.u32Gain       = Parsing_Num(argv[8]);
            stPictureCfg.CscType       = Parsing_CscType(argv[9]);
            Test_DISP_Picture(stPictureCfg);
        }
        else
        {
            printf("=========================================================================================================================\n");
            printf("picture Config \n");
            printf("[INTERFACE] [Contrast] [Brightness] [Hue] [Saturation] [Sharpness] [Gain] [CSC]\n");
            printf("[INTERFACE]: hdmi, vga\n");
            printf("[CSC]: 601t0709, 709to601, 601toRGB, 709toRGB, RGBto601, RGBto709, BYPASS\n");
            printf("=========================================================================================================================\n");
        }
    }
    else if(strcmp(argv[1], "dbg") == 0)
    {
        if(argc == 4)
        {
            Test_DISP_Debug(argv[2], Parsing_HexNum(argv[3]));
        }
        else
        {
            printf("hdmitx [Level]\n");
            printf("        hdmitx:0x0001, hdcp:0x0002\n");
            printf("===============================================\n");
            printf("xc [Level]\n");
            printf("        SetWindow:0x0001, SetTiming:0x0002\n");
            printf("===============================================\n");
            printf("ve [Level]\n");
            printf("        Enable:0x0001\n");
            printf("===============================================\n");
            printf("mgwin [Level]\n");
            printf("    Opt:[15:8] + Level:[7:0]\n");
            printf("       IOCTL       :0000\n");
            printf("       MDRV        :0100\n");
            printf("       PRIORITY    :0200\n");
            printf("       DRVSCLMGWIN :0300\n");
            printf("             FRAME :  01\n");
            printf("            SubWin :  02\n");
            printf("        FPGASCRIPT :  04\n");
            printf("===============================================\n");
            printf("mhal_disp [Level]\n");
            printf("        FUNC     :00000001\n");
            printf("        InputPort:00000002\n");
            printf("        Mute     :00000004\n");
            printf("        Interrupt:00000008\n");
            printf("        XC       :00000010\n");
            printf("        CheckFunc:00000020\n");
            printf("        Flip     :00000040\n");
            printf("===============================================\n");
            printf("pattern [Level]\n");
            printf("    Type:[31:16]\n");
            printf("        IP2      :00010000\n");
            printf("        OPM      :00020000\n");
            printf("        OP1      :00040000\n");
            printf("        Vtrack   :00080000\n");
            printf("        Hdmitx   :00100000\n");
            printf("    device[15:8]\n");
            printf("        Device0  :00000000\n");
            printf("        Device1  :00000100\n");
            printf("    vlaue[7:0]\n");
        }
    }
    else if(strcmp(argv[1], "hdmitx") == 0)
    {
        if(argc == 5)
        {
            Test_DISP_Hdmitx(Parsing_Hdmitx_ColorDepth(argv[2]), Parsing_Hdmitx_Color(argv[3]), Parsing_Hdmitx_Color(argv[4]));
        }
        else
        {
            printf("hdmitx [color depth] [in color type] [out color type]\n");
            printf("[color depth]:: 8, 10, 12, 16\n");
            printf("[color type] :: rgb444, yuv444, yuv422 \n");
        }
    }
    else if(strcmp(argv[1], "lcd") == 0)
    {
        if(argc == 13)
        {
            if(Test_DISP_Init(&stDispCaseCfg) == 0)
            {
                return;
            }

            stDispCaseCfg.Case = 15;
            stDispCaseCfg.Name = argv[2];
            stDispCaseCfg.enPixelFmt = Parsing_PixelFormat(argv[3]);
            stDispCaseCfg.ImageWidth = Parsing_Num(argv[4]);
            stDispCaseCfg.ImageHeight = Parsing_Num(argv[5]);
            stDispCaseCfg.DeviceId   = 0; //Parsing_Num(argv[6]);
            stDispCaseCfg.LayerId    = Parsing_Num(argv[7]);
            stDispCaseCfg.enTiming1  = Parsing_Timing(argv[8]);
            stDispCaseCfg.enTiming2  = Parsing_Timing(argv[9]);
            stDispCaseCfg.InputPortXNum = 1; //Parsing_Num(argv[10]);
            stDispCaseCfg.InputPortYNum = 1; //Parsing_Num(argv[11]);
            stDispCaseCfg.DelayTime = Parsing_Num(argv[12]);
            Test_DISP_Case(&stDispCaseCfg);
        }
        else if(argc == 16)
        {   // case 15

            if(Test_DISP_Init(&stDispCaseCfg) == 0)
            {
                return;
            }

            stDispCaseCfg.Case = 15;
            stDispCaseCfg.Name = argv[2];
            stDispCaseCfg.enPixelFmt = Parsing_PixelFormat(argv[3]);
            stDispCaseCfg.ImageWidth = Parsing_Num(argv[4]);
            stDispCaseCfg.ImageHeight = Parsing_Num(argv[5]);
            stDispCaseCfg.DeviceId   = 0;
            stDispCaseCfg.LayerId    = 0;
            stDispCaseCfg.enTiming1  = E_SIMMI_MODULE_OUTPUT_TIMING_USER;

            stDispCaseCfg.u16Hpw     = Parsing_Num(argv[6]);
            stDispCaseCfg.u16Hbp     = Parsing_Num(argv[7]);
            stDispCaseCfg.u16Hactive = Parsing_Num(argv[8]);
            stDispCaseCfg.u16Htt     = Parsing_Num(argv[9]);
            stDispCaseCfg.u16Vpw     = Parsing_Num(argv[10]);
            stDispCaseCfg.u16Vbp     = Parsing_Num(argv[11]);
            stDispCaseCfg.u16Vactive = Parsing_Num(argv[12]);
            stDispCaseCfg.u16Vtt     = Parsing_Num(argv[13]);
            stDispCaseCfg.u16Fps     = Parsing_Num(argv[14]);
            stDispCaseCfg.InputPortXNum = 1;
            stDispCaseCfg.InputPortYNum = 1;
            stDispCaseCfg.DelayTime = Parsing_Num(argv[15]);

            Test_DISP_Case(&stDispCaseCfg);
        }
        else
        {
            printf("lcd config \n");
            printf("lcd filename pixelformat, imagewidth, imageheight, Hpw, Hbp, Hactive, Htt, Vpw, Vbp, Vactive, Vtt, Fps \n");
            printf("lcd filename pixelformat, imagewidth, imageheight, 0, 0, timing1, NULL, 1 1\n");
            printf("case 15:  LCD \n");
            printf("=========================================================================================================================\n");

        }


    }
    else if(argc == 5 || argc == 6)
    {   // case 0 1 2 3
        if(Test_DISP_Init(&stDispCaseCfg) == 0)
        {
            return;
        }
        stDispCaseCfg.Case = Parsing_Num(argv[1]);

        stDispCaseCfg.Name = argv[2];
        stDispCaseCfg.ImageWidth = Parsing_Num(argv[3]);
        stDispCaseCfg.ImageHeight = Parsing_Num(argv[4]);

        stDispCaseCfg.DelayTime = argc == 5 ? 5 : Parsing_Num(argv[5]);

        Test_DISP_Case(&stDispCaseCfg);
    }
    else if(argc == 7 || argc == 8)
    {   // case 4
        if(Test_DISP_Init(&stDispCaseCfg) == 0)
        {
            return;
        }

        stDispCaseCfg.Case = Parsing_Num(argv[1]);

        stDispCaseCfg.Name = argv[2];
        stDispCaseCfg.ImageWidth = Parsing_Num(argv[3]);
        stDispCaseCfg.ImageHeight = Parsing_Num(argv[4]);
        stDispCaseCfg.DeviceId   = Parsing_Num(argv[5]);
        stDispCaseCfg.LayerId    = Parsing_Num(argv[6]);

        stDispCaseCfg.DelayTime = argc == 7 ? 5 : Parsing_Num(argv[7]);

        Test_DISP_Case(&stDispCaseCfg);
    }
    else if(argc == 12 || argc == 13)
    {   // case 5 6 7 8 9 11
        if(Test_DISP_Init(&stDispCaseCfg) == 0)
        {
            return;
        }

        stDispCaseCfg.Case = Parsing_Num(argv[1]);

        stDispCaseCfg.Name = argv[2];

        stDispCaseCfg.enPixelFmt = Parsing_PixelFormat(argv[3]);
        stDispCaseCfg.ImageWidth = Parsing_Num(argv[4]);
        stDispCaseCfg.ImageHeight = Parsing_Num(argv[5]);
        stDispCaseCfg.DeviceId   = Parsing_Num(argv[6]);
        stDispCaseCfg.LayerId    = Parsing_Num(argv[7]);
        stDispCaseCfg.enTiming1  = Parsing_Timing(argv[8]);
        stDispCaseCfg.enTiming2  = Parsing_Timing(argv[9]);
        stDispCaseCfg.InputPortXNum = Parsing_Num(argv[10]);
        stDispCaseCfg.InputPortYNum = Parsing_Num(argv[11]);

        stDispCaseCfg.DelayTime = argc == 12 ? 5 : Parsing_Num(argv[12]);

        Test_DISP_Case(&stDispCaseCfg);
    }
    else
    {

        UsagePrintf();
    }

}

