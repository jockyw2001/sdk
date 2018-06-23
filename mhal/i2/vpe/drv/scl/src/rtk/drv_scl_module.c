////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
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
/*=============================================================*/
// Include files
/*=============================================================*/
#include "vm_types.ht"
#include "sys_traces.ho"

#include "sys_sys_isw_cli.h"
#include "sys_sys_isw_uart.h"
#include "sys_sys.h"

#include "sys_sys_isw_uart.h"
#include "sys_MsWrapper_cus_os_msg.h"
#include "sys_MsWrapper_cus_os_timer.h"
#include "sys_MsWrapper_cus_os_mem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_int_pub.h"
#include "sys_MsWrapper_cus_os_int_ctrl.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"

#include "vm_stdio.ho"

#include "kernel.h"
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "mhal_vpe.h"
#include "hal_scl_util.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_dma_m.h"

// definition
/*=============================================================*/
#define TEST_INST_MAX 64

#define ISPINPUTWIDTH 736
#define ISPINPUTHEIGHT 240
#define MAXINPUTWIDTH 1920
#define MAXINPUTHEIGHT 1080
#define MAXCROPWIDTH(w) ((w)/3*2)
#define MAXCROPHEIGHT(h) ((h)/3*2)
#define MAXCROPX(w) ((w)/4)
#define MAXCROPY(h) ((h)/4)
#define MAXSTRIDEINPUTWIDTH 352
#define MAXSTRIDEINPUTHEIGHT 288
#define MAXOUTPUTDISPWIDTH 352
#define MAXOUTPUTDISPHEIGHT 288
#define FHDOUTPUTWIDTH 1920
#define FHDOUTPUTHEIGHT 1080
#define XGAOUTPUTWIDTH 1440
#define XGAOUTPUTHEIGHT 900
#define HDOUTPUTWIDTH 1280
#define HDOUTPUTHEIGHT 720
#define VGAOUTPUTWIDTH 640
#define VGAOUTPUTHEIGHT 360
#define CIFOUTPUTWIDTH 352
#define CIFOUTPUTHEIGHT 288
#define QXGAOUTPUTWIDTH 2048
#define QXGAOUTPUTHEIGHT 1536
#define WQHDOUTPUTWIDTH 2560
#define WQHDOUTPUTHEIGHT 1440
#define QSXGAOUTPUTWIDTH 2560
#define QSXGAOUTPUTHEIGHT 2048
#define QFHDOUTPUTWIDTH8 3840
#define QFHDOUTPUTHEIGHT8 2160
#define FKUOUTPUTWIDTH 4096
#define FKUOUTPUTHEIGHT 2160
#define MAXOUTPUTSTRIDEH 1920
#define MAXOUTPUTSTRIDEV 1080
#define MAXOUTPUTportH MAXOUTPUTSTRIDEH/4
#define MAXOUTPUTportV MAXOUTPUTSTRIDEV/4
#define CH0INWIDTH FHDOUTPUTWIDTH
#define CH1INWIDTH ISPINPUTWIDTH
#define CH2INWIDTH VGAOUTPUTWIDTH
#define CH3INWIDTH HDOUTPUTWIDTH
#define CH0INHEIGHT FHDOUTPUTHEIGHT
#define CH1INHEIGHT ISPINPUTHEIGHT
#define CH2INHEIGHT VGAOUTPUTHEIGHT
#define CH3INHEIGHT HDOUTPUTHEIGHT
#define CH0OUTWIDTH HDOUTPUTWIDTH
#define CH1OUTWIDTH VGAOUTPUTWIDTH
#define CH2OUTWIDTH CIFOUTPUTWIDTH
#define CH3OUTWIDTH XGAOUTPUTWIDTH
#define CH0OUTHEIGHT HDOUTPUTHEIGHT
#define CH1OUTHEIGHT VGAOUTPUTHEIGHT
#define CH2OUTHEIGHT CIFOUTPUTHEIGHT
#define CH3OUTHEIGHT XGAOUTPUTHEIGHT




static MHalVpeSclWinSize_t stMaxWin[TEST_INST_MAX];
static MHalVpeSclCropConfig_t stCropCfg[TEST_INST_MAX];
static MHalVpeSclInputSizeConfig_t stInputpCfg[TEST_INST_MAX];
static MHalVpeSclOutputSizeConfig_t stOutputCfg[TEST_INST_MAX];
static MHalVpeSclDmaConfig_t stOutputDmaCfg[TEST_INST_MAX];
//static MHalVpeSclOutputMDwinConfig_t stMDwinCfg[TEST_INST_MAX];
static MHalVpeSclOutputBufferConfig_t stpBuffer[TEST_INST_MAX];
static MHalVpeIqConfig_t stIqCfg[TEST_INST_MAX];
static MHalVpeIqOnOff_t stIqOnCfg[TEST_INST_MAX];
static MHalVpeIqWdrRoiReport_t stRoiReport[TEST_INST_MAX];
static MHalVpeIqWdrRoiHist_t stHistCfg[TEST_INST_MAX];
static u32 u32Ctx[TEST_INST_MAX];
static u32 IdNum[TEST_INST_MAX];
static u32 u32IqCtx[TEST_INST_MAX];
static u32 IdIqNum[TEST_INST_MAX];
static char sg_scl_yc_name[TEST_INST_MAX][16];
static char KEY_DMEM_SCL_VPE_YC[20] = "VPE_YC";
static u32 sg_scl_yc_addr[TEST_INST_MAX];
static u32 sg_scl_yc_size[TEST_INST_MAX];
static u32 *sg_scl_yc_viraddr[TEST_INST_MAX];
//isp
static char sg_Isp_yc_name[TEST_INST_MAX][16];
static char KEY_DMEM_Isp_VPE_YC[20] = "Isp_YC";
static u32 sg_Isp_yc_addr[TEST_INST_MAX];
static u32 sg_Isp_yc_size[TEST_INST_MAX];
static u32 *sg_Isp_yc_viraddr[TEST_INST_MAX];
static u32 u32IspCtx[TEST_INST_MAX];
static u32 IdIspNum[TEST_INST_MAX];
static MHalVpeIspInputConfig_t stIspInputCfg[TEST_INST_MAX];
static MHalVpeIspRotationConfig_t stRotCfg[TEST_INST_MAX];
static MHalVpeIspVideoInfo_t stVdinfo[TEST_INST_MAX];
//static MHAL_CMDQ_CmdqInterface_t *pCmdqCfg=NULL;
static u8 u8glevel[TEST_INST_MAX];


/*=============================================================*/
// Variable definition
/*=============================================================*/
static char _szVpeTestAHelpTxt[] = "vpehvsp";
static char _szVpeTestAUsageTxt[] = "vpehvsp a|b|c";

static int _gTestInit = 0;
/*=============================================================*/
// Global Variable definition
/*=============================================================*/

int _VpeHvspTest(CLI_t *pCli, char *p);

CliParseToken_t g_atVpeMenuTbl[] =
{
    {"vpehvsp",      _szVpeTestAHelpTxt,      _szVpeTestAUsageTxt,_VpeHvspTest,     NULL},
    PARSE_TOKEN_DELIMITER
};

static u8 _mdrv_Scl_Changebuf2hex(int u32num)
{
    u8 u8level = 0;
    if(u32num==10)
    {
        u8level = 1;
    }
    else if(u32num==48)
    {
        u8level = 0;
    }
    else if(u32num==49)
    {
        u8level = 0x1;
    }
    else if(u32num==50)
    {
        u8level = 0x2;
    }
    else if(u32num==51)
    {
        u8level = 0x3;
    }
    else if(u32num==52)
    {
        u8level = 0x4;
    }
    else if(u32num==53)
    {
        u8level = 0x5;
    }
    else if(u32num==54)
    {
        u8level = 0x6;
    }
    else if(u32num==55)
    {
        u8level = 0x7;
    }
    else if(u32num==56)
    {
        u8level = 0x8;
    }
    else if(u32num==57)
    {
        u8level = 0x9;
    }
    else if(u32num==65)
    {
        u8level = 0xa;
    }
    else if(u32num==66)
    {
        u8level = 0xb;
    }
    else if(u32num==67)
    {
        u8level = 0xc;
    }
    else if(u32num==68)
    {
        u8level = 0xd;
    }
    else if(u32num==69)
    {
        u8level = 0xe;
    }
    else if(u32num==70)
    {
        u8level = 0xf;
    }
    else if(u32num==97)
    {
        u8level = 0xa;
    }
    else if(u32num==98)
    {
        u8level = 0xb;
    }
    else if(u32num==99)
    {
        u8level = 0xc;
    }
    else if(u32num==100)
    {
        u8level = 0xd;
    }
    else if(u32num==101)
    {
        u8level = 0xe;
    }
    else if(u32num==102)
    {
        u8level = 0xf;
    }
    return u8level;
}

void _UTest_CleanInst(void)
{
    int i;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdNum[i])
        {
            if(sg_scl_yc_viraddr[i])
            {
                DrvSclOsDirectMemFree(sg_scl_yc_name[i],sg_scl_yc_size[i],(void*)sg_scl_yc_viraddr[i],(DrvSclOsDmemBusType_t)sg_scl_yc_addr[i]);
                sg_scl_yc_viraddr[i] = NULL;
            }
            MHalVpeDestroySclInstance((void *)u32Ctx[i]);
            IdNum[i] = 0;
        }
        if(IdIqNum[i])
        {
            MHalVpeDestroyIqInstance((void *)u32IqCtx[i]);
            IdIqNum[i] = 0;
        }
        if(IdIspNum[i])
        {
            if(sg_Isp_yc_viraddr[i])
            {
                DrvSclOsDirectMemFree(sg_Isp_yc_name[i],sg_Isp_yc_size[i],(void*)sg_Isp_yc_viraddr[i],(DrvSclOsDmemBusType_t)sg_Isp_yc_addr[i]);
                sg_Isp_yc_viraddr[i] = NULL;
            }
            MHalVpeDestroyIspInstance((void *)u32IspCtx[i]);
            IdIspNum[i] = 0;
        }
    }
    MHalVpeDeInit();
}

int _UTest_GetEmptyIdx(void)
{
    int idx=-1,i;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdNum[i]==0)
        {
            IdNum[i] = 1;
            idx = i;
            break;
        }
    }
    return idx;
}
int _UTest_GetIqEmptyIdx(void)
{
    int idx = -1,i;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdIqNum[i]==0)
        {
            IdIqNum[i] = 1;
            idx = i;
            break;
        }
    }
    return idx;
}
void _UTestClosePatternGen(MHAL_CMDQ_CmdqInterface_t *pCmdqinf)
{
    pCmdqinf->MHAL_CMDQ_WriteRegCmdqMask(pCmdqinf,0x152580,0,0x8000);
    pCmdqinf->MHAL_CMDQ_WriteRegCmdqMask(pCmdqinf,0x1525E0,0,0x1);
}

void _DrvSclVpeModuleInit(void)
{
    //int s32Ret;

    DrvSclOsClkConfig_t stClkCfg;

    DrvSclOsSetSclIrqIDFormSys(E_DRV_SCLOS_SCLIRQ_SC0);
    DrvSclOsSetCmdqIrqIDFormSys(E_DRV_SCLOS_CMDQIRQ_CMDQ0);
    //clk enable
    stClkCfg.ptIdclk = DrvSclOsClkGetClk(0);
    stClkCfg.ptFclk1 = DrvSclOsClkGetClk(1);
    stClkCfg.ptFclk2 = DrvSclOsClkGetClk(2);
    stClkCfg.ptOdclk = DrvSclOsClkGetClk(3);
    DrvSclOsClkSetConfig(E_DRV_SCLOS_CLK_ID_HVSP1, stClkCfg);
}
void _check_sclproc_storeNaming(u32 u32Id)
{
    sg_scl_yc_name[u32Id][0] = 48+(((u32Id&0xFFF)%1000)/100);
    sg_scl_yc_name[u32Id][1] = 48+(((u32Id&0xFFF)%100)/10);
    sg_scl_yc_name[u32Id][2] = 48+(((u32Id&0xFFF)%10));
    sg_scl_yc_name[u32Id][3] = '_';
    sg_scl_yc_name[u32Id][4] = '\0';
    DrvSclOsStrcat(sg_scl_yc_name[u32Id],KEY_DMEM_SCL_VPE_YC);
}
void _check_Ispproc_storeNaming(u32 u32Id)
{
    sg_Isp_yc_name[u32Id][0] = 48+(((u32Id&0xFFF)%1000)/100);
    sg_Isp_yc_name[u32Id][1] = 48+(((u32Id&0xFFF)%100)/10);
    sg_Isp_yc_name[u32Id][2] = 48+(((u32Id&0xFFF)%10));
    sg_Isp_yc_name[u32Id][3] = '_';
    sg_Isp_yc_name[u32Id][4] = '\0';
    DrvSclOsStrcat(sg_Isp_yc_name[u32Id],KEY_DMEM_Isp_VPE_YC);
}
void _UTest_SetInputCfgISP422Mode(int idx, u16 u16Width, u16 u16Height)
{
    stIspInputCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stIspInputCfg[idx].enInType = E_MHAL_ISP_INPUT_YUV422;
    stIspInputCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
    stIspInputCfg[idx].u32Height = u16Height;
    stIspInputCfg[idx].u32Width = u16Width;
    MHalVpeIspInputConfig((void *)u32IspCtx[idx], &stIspInputCfg[idx]);
    MHalVpeIspRotationConfig((void *)u32IspCtx[idx],&stRotCfg[idx]);
    stInputpCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stInputpCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
    stInputpCfg[idx].u16Height = u16Height;
    stInputpCfg[idx].u16Width = u16Width;
    MHalVpeSclInputConfig((void *)u32Ctx[idx], &stInputpCfg[idx]);
}
void _UTest_SetInputCfgISPMode(int idx, u16 u16Width, u16 u16Height)
{
    stIspInputCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stIspInputCfg[idx].enInType = E_MHAL_ISP_INPUT_YUV420;
    stIspInputCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stIspInputCfg[idx].u32Height = u16Height;
    stIspInputCfg[idx].u32Width = u16Width;
    MHalVpeIspInputConfig((void *)u32IspCtx[idx], &stIspInputCfg[idx]);
    MHalVpeIspRotationConfig((void *)u32IspCtx[idx],&stRotCfg[idx]);
    stInputpCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stInputpCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stInputpCfg[idx].u16Height = u16Height;
    stInputpCfg[idx].u16Width = u16Width;
    MHalVpeSclInputConfig((void *)u32Ctx[idx], &stInputpCfg[idx]);
}
void _UTest_SetInputCfgPatMode(int idx, u16 u16Width, u16 u16Height)
{
    stInputpCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stInputpCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_ARGB8888;
    stInputpCfg[idx].u16Height = u16Height;
    stInputpCfg[idx].u16Width = u16Width;
    MHalVpeSclInputConfig((void *)u32Ctx[idx], &stInputpCfg[idx]);
}
void _UTest_SetCropCfg(int idx, u16 u16Width, u16 u16Height)
{
    stCropCfg[idx].bCropEn = (idx%2) ? 1 :0;
    stCropCfg[idx].stCropWin.u16Height = (idx%2) ? MAXCROPHEIGHT(u16Height) : u16Height;
    stCropCfg[idx].stCropWin.u16Width = (idx%2) ? MAXCROPWIDTH(u16Width) : u16Width;
    stCropCfg[idx].stCropWin.u16X = (idx%2) ? MAXCROPX(u16Width) : 0;
    stCropCfg[idx].stCropWin.u16Y = (idx%2) ? MAXCROPY(u16Height) : 0;
    MHalVpeSclCropConfig((void *)u32Ctx[idx], &stCropCfg[idx]);
}
void _UTest_SetM2MDmaCfg(int idx, MHalPixelFormat_e enColor, MHalVpeDmaPort_e enPort)
{
    stM2MOutputDmaCfg[idx].enCompress = E_MHAL_COMPRESS_MODE_NONE;
    stM2MOutputDmaCfg[idx].enFormat = enColor;
    stM2MOutputDmaCfg[idx].enPort = enPort;
    MHalVpeSclDmaPortConfig((void *)u32M2MCtx[idx], &stM2MOutputDmaCfg[idx]);
}
void _UTest_SetDmaCfg(int idx, MHalPixelFormat_e enColor, MHalVpeDmaPort_e enPort)
{
    stOutputDmaCfg[idx].enCompress = E_MHAL_COMPRESS_MODE_NONE;
    stOutputDmaCfg[idx].enFormat = enColor;
    stOutputDmaCfg[idx].enPort = enPort;
    MHalVpeSclDmaPortConfig((void *)u32Ctx[idx], &stOutputDmaCfg[idx]);
}
void _UTest_SetOutputSizeCfg(int idx, MHalVpeDmaPort_e enPort, u16 u16Width, u16 u16Height)
{
    stOutputCfg[idx].enOutPort = enPort;
    stOutputCfg[idx].u16Height = u16Height;
    stOutputCfg[idx].u16Width = u16Width;
    MHalVpeSclPortSizeConfig((void *)u32Ctx[idx], &stOutputCfg[idx]);
}
void _UTest_SetProcessCfg(int idx, MHalVpeDmaPort_e enPort, u32 u32AddrY,bool bEn,u32 u32Stride)
{
    stpBuffer[idx].stCfg[enPort].bEn = bEn;
    stpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[0] = u32Stride;
    stpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[0] = u32AddrY;
    stpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[1] = u32AddrY+(stOutputCfg[idx].u16Width*stOutputCfg[idx].u16Height);
}
void _UTest_SetRoiCfg(int idx, MHAL_CMDQ_CmdqInterface_t *pCfg )
{
    int i;
    DrvSclOsMemset(&stHistCfg[idx],0,sizeof(MHalVpeIqWdrRoiHist_t));
    stHistCfg[idx].bEn = 1;
    stHistCfg[idx].enPipeSrc = E_MHAL_IQ_ROISRC_BEFORE_WDR;
    stHistCfg[idx].u8WinCount = 4;
    for(i=0;i<4;i++)
    {
        stHistCfg[idx].stRoiCfg[idx].bEnSkip = 0;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccX[0] = 100+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccY[0] = 100+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccX[1] = 200+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccY[1] = 100+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccX[2] = 200+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccY[2] = 200+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccX[3] = 100+idx*100;
        stHistCfg[idx].stRoiCfg[idx].u16RoiAccY[3] = 200+idx*100;
    }
    MHalVpeIqSetWdrRoiHist((void *)u32IqCtx[idx], &stHistCfg[idx]);
    MHalVpeIqSetWdrRoiMask((void *)u32IqCtx[idx],0, pCfg);
}
typedef enum
{
    SET_DEF = 0,
    SET_EG,      ///< ID_1
    SET_ES,      ///< ID_2
    SET_ES2,      ///< ID_2
    SET_CON,      ///< ID_2
    SET_UV,      ///< ID_2
    SET_NR,      ///< ID_2
    SET_all,      ///< ID_2
    SET_TYPE,      ///< ID_4
} UTest_SetIqCfg_e;
void _UTest_SetIqCfg(int idx,UTest_SetIqCfg_e enType)
{
    if(enType==SET_DEF)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
    }
    else if(enType==SET_CON)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
        stIqCfg[idx].u8Contrast= 128;
    }
    else if(enType==SET_all)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8NRY_SF_STR = 192;
        stIqCfg[idx].u8NRY_TF_STR = 192;
        stIqCfg[idx].u8NRC_SF_STR = 192;
        stIqCfg[idx].u8NRC_TF_STR = 192;
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
        stIqCfg[idx].u8Contrast= 128;
        stIqCfg[idx].u8NRY_BLEND_MOTION_TH = 0;
        stIqCfg[idx].u8NRY_BLEND_STILL_TH = 1;
        stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_STILL_WEI = 0;
    }
    else if(enType==SET_NR)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8NRY_SF_STR = 255;
        stIqCfg[idx].u8NRY_TF_STR = 255;
        stIqCfg[idx].u8NRC_SF_STR = 255;
        stIqCfg[idx].u8NRC_TF_STR = 255;
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
    }
    else if(enType==SET_ES)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8NRY_SF_STR = 255;
        stIqCfg[idx].u8NRY_TF_STR = 0;
        stIqCfg[idx].u8NRY_BLEND_MOTION_TH = 0;
        stIqCfg[idx].u8NRY_BLEND_STILL_TH = 1;
        stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_STILL_WEI = 0;
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
    }
    else if(enType==SET_ES2)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8NRY_SF_STR = 255;
        stIqCfg[idx].u8NRY_TF_STR = 0;
        stIqCfg[idx].u8NRY_BLEND_MOTION_TH = 0;
        stIqCfg[idx].u8NRY_BLEND_STILL_TH = 1;
        stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = 16;
        stIqCfg[idx].u8NRY_BLEND_STILL_WEI = 16;
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
    }
    else
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8EdgeGain[0] = 128;
        stIqCfg[idx].u8EdgeGain[1] = 192;
        stIqCfg[idx].u8EdgeGain[2] = 255;
        stIqCfg[idx].u8EdgeGain[3] = 255;
        stIqCfg[idx].u8EdgeGain[4] = 255;
        stIqCfg[idx].u8EdgeGain[5] = 255;
    }
    MHalVpeIqConfig((void *)u32IqCtx[idx], &stIqCfg[idx]);
}
void _UTest_SetIqOnOff(int idx,UTest_SetIqCfg_e enType)
{
    if(enType==SET_DEF)
    {
        DrvSclOsMemset(&stIqOnCfg[idx],0,sizeof(MHalVpeIqOnOff_t));
    }
    else if(enType==SET_EG)
    {
        stIqOnCfg[idx].bEdgeEn = 1;
    }
    else if(enType==SET_ES)
    {
        stIqOnCfg[idx].bESEn = 1;
    }
    else if(enType==SET_CON)
    {
        stIqOnCfg[idx].bContrastEn= 1;
    }
    else if(enType==SET_UV)
    {
        stIqOnCfg[idx].bUVInvert= 1;
    }
    else if(enType==SET_NR)
    {
        stIqOnCfg[idx].bNREn= 1;
    }
    else if(enType==SET_all)
    {
        stIqOnCfg[idx].bNREn= 1;
        stIqOnCfg[idx].bContrastEn= 1;
        stIqOnCfg[idx].bESEn = 1;
        stIqOnCfg[idx].bEdgeEn = 1;
    }
    MHalVpeIqOnOff((void *)u32IqCtx[idx], &stIqOnCfg[idx]);
}
void _UTest_SetLevelSize(u8 u8level,u16 *u16wsize ,u16 *u16hsize)
{
    if(u8level == 0)
    {
        *u16wsize = FHDOUTPUTWIDTH;
        *u16hsize = FHDOUTPUTHEIGHT;
    }
    else if(u8level == 1)
    {
        *u16wsize = XGAOUTPUTWIDTH;
        *u16hsize = XGAOUTPUTHEIGHT;
    }
    else if(u8level == 2)
    {
        *u16wsize = HDOUTPUTWIDTH;
        *u16hsize = HDOUTPUTHEIGHT;
    }
    else if(u8level == 3)
    {
        *u16wsize = VGAOUTPUTWIDTH;
        *u16hsize = VGAOUTPUTHEIGHT;
    }
    else if(u8level == 4)
    {
        *u16wsize = CIFOUTPUTWIDTH;
        *u16hsize = CIFOUTPUTHEIGHT;
    }
    else if(u8level == 5)
    {
        *u16wsize = QXGAOUTPUTWIDTH;
        *u16hsize = QXGAOUTPUTHEIGHT;
    }
    else if(u8level == 6)
    {
        *u16wsize = WQHDOUTPUTWIDTH;
        *u16hsize = WQHDOUTPUTHEIGHT;
    }
    else if(u8level == 7)
    {
        *u16wsize = QSXGAOUTPUTWIDTH;
        *u16hsize = QSXGAOUTPUTHEIGHT;
    }
    else if(u8level == 8)
    {
        *u16wsize = QFHDOUTPUTWIDTH8;
        *u16hsize = QFHDOUTPUTHEIGHT8;
    }
    else if(u8level == 9)
    {
        *u16wsize = FKUOUTPUTWIDTH;
        *u16hsize = FKUOUTPUTHEIGHT;
    }
    else
    {
        *u16wsize = FHDOUTPUTWIDTH;
        *u16hsize = FHDOUTPUTHEIGHT;
    }
}
void _UTest_SetLevelInSize(u8 u8level,u16 *u16wsize ,u16 *u16hsize)
{
    if(u8level == 0)
    {
        *u16wsize = FHDOUTPUTWIDTH;
        *u16hsize = FHDOUTPUTHEIGHT;
    }
    else if(u8level == 1)
    {
        *u16wsize = ISPINPUTWIDTH;
        *u16hsize = ISPINPUTHEIGHT;
    }
    else if(u8level == 2)
    {
        *u16wsize = HDOUTPUTWIDTH;
        *u16hsize = HDOUTPUTHEIGHT;
    }
    else if(u8level == 3)
    {
        *u16wsize = VGAOUTPUTWIDTH;
        *u16hsize = VGAOUTPUTHEIGHT;
    }
    else if(u8level == 4)
    {
        *u16wsize = CIFOUTPUTWIDTH;
        *u16hsize = CIFOUTPUTHEIGHT;
    }
    else if(u8level == 5)
    {
        *u16wsize = QXGAOUTPUTWIDTH;
        *u16hsize = QXGAOUTPUTHEIGHT;
    }
    else if(u8level == 6)
    {
        *u16wsize = WQHDOUTPUTWIDTH;
        *u16hsize = WQHDOUTPUTHEIGHT;
    }
    else if(u8level == 7)
    {
        *u16wsize = QSXGAOUTPUTWIDTH;
        *u16hsize = QSXGAOUTPUTHEIGHT;
    }
    else if(u8level == 8)
    {
        *u16wsize = QFHDOUTPUTWIDTH8;
        *u16hsize = QFHDOUTPUTHEIGHT8;
    }
    else if(u8level == 9)
    {
        *u16wsize = FKUOUTPUTWIDTH;
        *u16hsize = FKUOUTPUTHEIGHT;
    }
    else
    {
        *u16wsize = FHDOUTPUTWIDTH;
        *u16hsize = FHDOUTPUTHEIGHT;
    }
}

void _DrvSclVpeModuleDeInit(void)
{
    DrvSclOsClkConfig_t stClkCfg;
    SCL_DBG(SCL_DBG_LV_MDRV_IO(), "[SCLDMA_1] %s\n",__FUNCTION__);
    stClkCfg.ptIdclk = DrvSclOsClkGetClk(0);
    stClkCfg.ptFclk1 = DrvSclOsClkGetClk(1);
    stClkCfg.ptFclk2 = DrvSclOsClkGetClk(2);
    stClkCfg.ptOdclk = DrvSclOsClkGetClk(3);
    MDrvSclHvspIdclkRelease((MDrvSclHvspClkConfig_t *)&stClkCfg);
    MDrvSclDmaClkClose((MDrvSclDmaClkConfig_t *)&stClkCfg);
}
int  _UTest_CreateSclInstance(int idx, u16 u16Width, u16 u16Height)
{
    int a = 1;
    MHalVpeSclCreate_t stCfg;
    stMaxWin[idx].u16Height = u16Height;
    stMaxWin[idx].u16Width = u16Width;
    stCfg.enSclInstance = E_MHAL_VPE_SCL_MAIN;
    stCfg.enInputPortMode = E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME;
    stCfg.enOutmode = E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE;
    stCfg.stMaxWin.u16Height = u16Height;
    stCfg.stMaxWin.u16Width = u16Width;
    if(!MHalVpeCreateSclInstance(NULL, &stCfg,(void *)&u32Ctx[idx]))
    {
        SCL_ERR( "[VPE]create Fail%s\n",sg_scl_yc_name[idx]);
        return 0;
    }
    MHalVpeSclDbgLevel((void *)&a);
    return 1;
}

ssize_t  _UTest_CreateIqInstance(int idx)
{
    int a = 0;
    DrvSclOsMemset(&stRoiReport[idx],0,sizeof(MHalVpeIqWdrRoiReport_t));
    MHalVpeCreateIqInstance(NULL , &stMaxWin[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,(void *)&u32IqCtx[idx]);
    MHalVpeIqDbgLevel((void *)&a);
    _UTest_SetIqCfg(idx,SET_DEF);
    _UTest_SetIqOnOff(idx,SET_DEF);
    return 1;
}

ssize_t  _UTest_CreateIspInstance(const MHalAllocPhyMem_t *pstAlloc ,void **pCtx)
{
	int a = 0xFF;

	if(!MHalVpeCreateIspInstance(pstAlloc, E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,pCtx))
    {
        SCL_ERR( "[VPE]create ISP Inst Fail\n");
        return 0;
    }

    MHalVpeIspDbgLevel((void *)&a);
    return 1;
}
void _UTest_AllocateMem(int idx,u32 size)
{
    _check_sclproc_storeNaming(idx);
    sg_scl_yc_size[idx] = size;
    sg_scl_yc_viraddr[idx] = DrvSclOsDirectMemAlloc(sg_scl_yc_name[idx], sg_scl_yc_size[idx],
        (DrvSclOsDmemBusType_t *)&sg_scl_yc_addr[idx]);
}
void _UTest_AllocateIspMem(int idx,u32 size)
{
    _check_Ispproc_storeNaming(idx);
    sg_Isp_yc_size[idx] = size;
    sg_Isp_yc_viraddr[idx] = DrvSclOsDirectMemAlloc(sg_Isp_yc_name[idx], sg_Isp_yc_size[idx],
        (DrvSclOsDmemBusType_t *)&sg_Isp_yc_addr[idx]);
    SCL_ERR("[%s]vir:%lx ,phy:%lx",__FUNCTION__,(u32)sg_Isp_yc_viraddr[idx],sg_Isp_yc_addr[idx]);
}

bool _UTest_PutFile2Buffer(int idx,char *path,u32 size)
{
    return 1;
}
MHalVpeWaitDoneType_e _UTest_GetWaitFlag(u8 u8level)
{
    MHalVpeWaitDoneType_e enType;
    if(u8level ==3)
    {
        enType = E_MHAL_VPE_WAITDONE_MDWINONLY;
    }
    else if(u8level ==8||u8level ==9)
    {
        enType = E_MHAL_VPE_WAITDONE_DMAANDMDWIN;
    }
    else
    {
        enType = E_MHAL_VPE_WAITDONE_DMAONLY;
    }
    return enType;
}
void UT_multiport_multisize(u8 u8level,u8 u8Count)
{
#if 1
	if(u8level||u8Count){}
#else
    u32 u32Addr;
    u32 u32IspinH;
    u32 u32IspinV;
    u32 u32ScOutH;
    u32 u32ScOutV;
    int idx;
    int i;
    u32 u32CMDQIrq;
    _UTest_CleanInst();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    //writefp = _UTest_OpenFile("/tmp/Scout_ch0.bin",O_WRONLY|O_CREAT,0777);
    //writefp2 = _UTest_OpenFile("/tmp/Scout_ch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level>2)
    {
        //writefp3 = _UTest_OpenFile("/tmp/Scout_ch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        //writefp4 = _UTest_OpenFile("/tmp/Scout_ch3.bin",O_WRONLY|O_CREAT,0777);
    }
    for(idx=0;idx<u8level;idx++)
    {
        u32IspinH = (idx ==0) ? CH0INWIDTH : (idx ==1) ? CH1INWIDTH : (idx ==2) ? CH2INWIDTH : CH3INWIDTH;
        u32IspinV = (idx ==0) ? CH0INHEIGHT : (idx ==1) ? CH1INHEIGHT: (idx ==2) ? CH2INHEIGHT : CH3INHEIGHT;
        u32ScOutH = (idx ==0) ? CH0OUTWIDTH : (idx ==1) ? CH1OUTWIDTH :(idx ==2) ? CH2OUTWIDTH : CH3OUTWIDTH;
        u32ScOutV = (idx ==0) ? CH0OUTHEIGHT: (idx ==1) ? CH1OUTHEIGHT: (idx ==2) ? CH2OUTHEIGHT : CH3OUTHEIGHT;
        _UTest_CreateSclInstance(idx,u32IspinH,u32IspinV);
        IdNum[idx] = 1;
        _UTest_CreateIspInstance(NULL ,(void *)&u32IspCtx[idx]);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx);
        IdIqNum[idx] = 1;
        _UTest_SetIqCfg(idx,SET_all);
        _UTest_SetIqOnOff(idx,SET_all);
        _UTest_AllocateMem(idx,(u32)u32ScOutH*u32ScOutV*2);
        _UTest_AllocateIspMem(idx,(u32)u32IspinH*u32IspinV*3/2);
        u32Addr = sg_scl_yc_addr[idx];
        if(idx ==0)
        {
            //if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin_ch0.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==1)
        {
            //if(!_UTest_PutFile2Buffer2(idx,"/tmp/Ispin_ch1.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==2)
        {
            //if(!_UTest_PutFile2Buffer3(idx,"/tmp/Ispin_ch2.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==3)
        {
            //if(!_UTest_PutFile2Buffer4(idx,"/tmp/Ispin_ch3.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        if(sg_scl_yc_viraddr[idx])
        {
            _UTest_SetInputCfgISPMode(idx, u32IspinH, u32IspinV);
            _UTest_SetCropCfg(idx, u32IspinH, u32IspinV);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,idx,u32ScOutH, u32ScOutV);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            //  1,2,3,4,5,6,  8,9,10,11,12,13,  , 15    =13
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                u32Addr,(idx==0) ? 1 : 0,u32ScOutH*2);
            // 0, 2,  4,   6,  8    10,    12,    14        = 8
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                u32Addr,(idx==1) ? 1 : 0,u32ScOutH*2);
            //  1,2  ,4,5,  7,8,   10,11,    13,14,15   =11
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                u32Addr,(idx==2) ? 1 : 0,u32ScOutH*2);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                u32Addr,(idx==3) ? 1 : 0,u32ScOutH*2);
            MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
            stVdinfo[idx].u32Stride[0] = u32IspinH;
            stVdinfo[idx].u32Stride[1] = u32IspinH;
            stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[idx];
            stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[idx]+((u32)(u32IspinH*u32IspinV));
            MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
            MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,
                (idx==3) ? E_MHAL_VPE_WAITDONE_MDWINONLY : E_MHAL_VPE_WAITDONE_DMAONLY);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
        }
    }
    if(pCmdqCfg)
    {
        do
        {
            DrvSclOsDelayTask(5);
            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
        }
        while(!(u32CMDQIrq&0x800));
    }
    DrvSclOsDelayTask(1000);
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],CH0OUTWIDTH*CH0OUTHEIGHT*2);
    _UTest_WriteFile(writefp2,(char *)sg_scl_yc_viraddr[1],CH1OUTWIDTH*CH1OUTHEIGHT*2);
    if(writefp3 && sg_scl_yc_viraddr[2])
    {
        _UTest_WriteFile(writefp3,(char *)sg_scl_yc_viraddr[2],CH2OUTWIDTH*CH2OUTHEIGHT*2);
    }
    if(writefp4 && sg_scl_yc_viraddr[3])
    {
        _UTest_WriteFile(writefp4,(char *)sg_scl_yc_viraddr[3],CH3OUTWIDTH*CH3OUTHEIGHT*2);
    }
    if(u8Count>1)
    {
        for(i=1;i<u8Count;i++)
        {
            for(idx=0;idx<4;idx++)
            {
                if(idx ==0)
                {
                    if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin_ch0.yuv",(u32)(CH0INWIDTH*CH0INHEIGHT*3/2)))
                    {
                        return ;
                    }
                }
                else if(idx ==1)
                {
                    if(!_UTest_PutFile2Buffer2(idx,"/tmp/Ispin_ch1.yuv",(u32)(CH1INWIDTH*CH1INHEIGHT*3/2)))
                    {
                        return ;
                    }
                }
                else if(idx ==2)
                {
                    if(!_UTest_PutFile2Buffer3(idx,"/tmp/Ispin_ch2.yuv",(u32)(CH2INWIDTH*CH2INHEIGHT*3/2)))
                    {
                        return ;
                    }
                }
                else if(idx ==3)
                {
                    if(!_UTest_PutFile2Buffer4(idx,"/tmp/Ispin_ch3.yuv",(u32)(CH3INWIDTH*CH3INHEIGHT*3/2)))
                    {
                        return ;
                    }
                }
                MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,
                    (idx==3) ? E_MHAL_VPE_WAITDONE_MDWINONLY : E_MHAL_VPE_WAITDONE_DMAONLY);
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                if(pCmdqCfg)
                {
                    do
                    {
                        DrvSclOsDelayTask(5);
                        pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                    }
                    while(!(u32CMDQIrq&0x800));
                }
            }
            DrvSclOsDelayTask(1000);
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],CH0OUTWIDTH*CH0OUTHEIGHT*2);
            _UTest_WriteFile(writefp2,(char *)sg_scl_yc_viraddr[1],CH1OUTWIDTH*CH1OUTHEIGHT*2);
            if(writefp3 && sg_scl_yc_viraddr[2])
            {
                _UTest_WriteFile(writefp3,(char *)sg_scl_yc_viraddr[2],CH2OUTWIDTH*CH2OUTHEIGHT*2);
            }
            if(writefp4 && sg_scl_yc_viraddr[3])
            {
                _UTest_WriteFile(writefp4,(char *)sg_scl_yc_viraddr[3],CH3OUTWIDTH*CH3OUTHEIGHT*2);
            }
        }
    }
#endif
}

MHAL_CMDQ_CmdqInterface_t* _UTest_AllocateCMDQ(void)
{
    MHAL_CMDQ_BufDescript_t stCfg;
    MHAL_CMDQ_CmdqInterface_t *pCfg=NULL;
    stCfg.u16MloadBufSizeAlign = 32;
    stCfg.u32CmdqBufSizeAlign = 32;
    stCfg.u32CmdqBufSize = 0x40000;
    stCfg.u32MloadBufSize = 0x10000;
//#if defined(SCLOS_TYPE_LINUX_TESTCMDQ)
    pCfg = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE,&stCfg,0);
//#endif
    return pCfg;
}

int vpe_test_store(const char *buf)
{
    const char *str = buf;
    int i;
    int idx;
    u8 u8level;
    static MHAL_CMDQ_CmdqInterface_t *pCmdqCfg=NULL;
    u32 u32Time;
    u32 u32CMDQIrq;
    static u8 u8Count = 1;
    static u8 u8Rot = 0;
    static u16 u16swsize = FHDOUTPUTWIDTH;
    static u16 u16shsize = FHDOUTPUTHEIGHT;
    static u16 u16IspInwsize = ISPINPUTWIDTH;
    static u16 u16IspInhsize = ISPINPUTHEIGHT;
    static u16 u16PatInwsize = MAXINPUTWIDTH;
    static u16 u16PatInhsize = MAXINPUTHEIGHT;
    static u8 u8Str[10];

    if(!_gTestInit)
    {
        _gTestInit = 1;
        DrvSclOsMemset(stMaxWin,0,sizeof(MHalVpeSclWinSize_t)*TEST_INST_MAX);
        DrvSclOsMemset(stCropCfg,0,sizeof(MHalVpeSclCropConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stInputpCfg,0,sizeof(MHalVpeSclInputSizeConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stOutputCfg,0,sizeof(MHalVpeSclOutputSizeConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stOutputDmaCfg,0,sizeof(MHalVpeSclDmaConfig_t)*TEST_INST_MAX);
//        DrvSclOsMemset(stMDwinCfg,0,sizeof(MHalVpeSclOutputMDwinConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stpBuffer,0,sizeof(MHalVpeSclOutputBufferConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(u32Ctx,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(u32IqCtx,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(u32IspCtx,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(IdIqNum,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(IdIspNum,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(IdNum,0,sizeof(u32)*TEST_INST_MAX);
        DrvSclOsMemset(stIspInputCfg,0,sizeof(MHalVpeIspInputConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stRotCfg,0,sizeof(MHalVpeIspRotationConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stVdinfo,0,sizeof(MHalVpeIspVideoInfo_t)*TEST_INST_MAX);
        DrvSclOsMemset(stIqCfg,0,sizeof(MHalVpeIqConfig_t)*TEST_INST_MAX);
        DrvSclOsMemset(stIqOnCfg,0,sizeof(MHalVpeIqOnOff_t)*TEST_INST_MAX);
        DrvSclOsMemset(stRoiReport,0,sizeof(MHalVpeIqWdrRoiReport_t)*TEST_INST_MAX);
        DrvSclOsMemset(stHistCfg,0,sizeof(MHalVpeIqWdrRoiHist_t)*TEST_INST_MAX);
    }

    if(NULL!=buf)
    {
        u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
        u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
        u8Str[0] = u8Str[0];
        SCL_ERR( "[VPE]u8level = %d-%d\n",u8level,(int)*str);
        if((int)*str == 48)    //Test 0
        {
            _UTest_CleanInst();
            SCL_ERR( "[VPE]Set %d disable all test\n",(int)*str);
        }
        if((int)*str == 49)    //Test 1
        {
            for(i = 0;i<TEST_INST_MAX;i++)
            {
                idx = _UTest_GetEmptyIdx();
                _UTest_CreateSclInstance(idx,MAXOUTPUTportH,MAXOUTPUTportV);
            }
            //_UTest_AllocateMem(idx,MAXINPUTWIDTH*MAXINPUTHEIGHT*2);
            SCL_ERR( "[VPE]Set %d create 1 inst Debug level on %s\n",(int)*str,sg_scl_yc_name[idx]);
        }
        else if((int)*str == 50)  //Test 2
        {
            for(i = 0;i<TEST_INST_MAX;i++)
            {
                if(IdNum[i])
                {
                    _UTest_SetInputCfgPatMode(i, u16PatInwsize, u16PatInhsize);
                    _UTest_SetCropCfg(i, u16PatInwsize, u16PatInhsize);
                }
            }
            SCL_ERR( "[VPE]Set %d Set all input/crop\n",(int)*str);
        }
        else if((int)*str == 51)  //Test 3
        {
            MHalPixelFormat_e enType;
            for(i = 0;i<TEST_INST_MAX;i++)
            {
                if(IdNum[i])
                {
                    enType = (u8level == 1) ? E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420: E_MHAL_PIXEL_FRAME_YUV422_YUYV;
                    _UTest_SetDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT0);
                    _UTest_SetDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT1);
                    _UTest_SetDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT2);
                    _UTest_SetDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT3);
                }
            }
            SCL_ERR( "[VPE]Set %d Set all Dma config\n",(int)*str);
        }
        else if((int)*str == 52)  //Test 4
        {
            _UTest_SetLevelSize(u8level,&u16swsize,&u16shsize);
            for(i = 0;i<TEST_INST_MAX;i++)
            {
                if(IdNum[i])
                {
                    _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                }
            }
            SCL_ERR( "[VPE]Set %d Set all Dma output size %hu/%hu\n",(int)*str,u16swsize,u16shsize);

        }
        else if((int)*str == 53)  //Test 5
        {
            for(i = 0;i<TEST_INST_MAX;i++)
            {
                if(IdNum[i])
                {
                    _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[i],
                        (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7) ? 1 :0,0);
                    _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[i],
                        (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,0);
                    _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[i],
                        (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,0);
                    _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[i],0,stOutputCfg[i].u16Width*2);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[i],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[i]));
                    MHalVpeSclProcess((void *)u32Ctx[i],NULL, &stpBuffer[i]);
                    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Test5_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                        stOutputCfg[i].u16Width,stOutputCfg[i].u16Height,i,(u16)(u32Time&0xFFFF)
                        ,sg_scl_yc_addr[i]+MIU0_BASE,(u32)(stOutputCfg[i].u16Width*stOutputCfg[i].u16Height*2));
                }
            }
            SCL_ERR( "[VPE]Set %d Set all Dma Process\n",(int)*str);
        }
        else if((int)*str == 54)  //Test 6
        {
            if(IdNum[u8level])
            {
                _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[u8level],(u8level%3 ==2) ? 1 :0,0);
                _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[u8level],(u8level%3 ==1) ? 1 :0,0);
                _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[u8level],(u8level%3 ==0) ? 1 :0,0);
                _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[u8level],0,stOutputCfg[u8level].u16Width*2);
                MHalVpeSclSetWaitDone((void *)u32Ctx[u8level],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[u8level]));
                MHalVpeSclProcess((void *)u32Ctx[u8level],NULL, &stpBuffer[u8level]);
                SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Test6_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                    stOutputCfg[u8level].u16Width,stOutputCfg[u8level].u16Height,u8level,(u16)(u32Time&0xFFFF)
                    ,sg_scl_yc_addr[u8level]+MIU0_BASE,(u32)(stOutputCfg[u8level].u16Width*stOutputCfg[u8level].u16Height*2));
            }
            SCL_ERR( "[VPE]Set %d Set for level:%hhu Dma Process\n",(int)*str,u8level);
        }
        else if((int)*str == 55)//Test 7
        {
            _UTest_SetLevelSize(u8level,&u16swsize,&u16shsize);
            SCL_ERR("[VPE]Set OutPutSize:(%hu,%hu) \n",u16swsize,u16shsize);
        }
        else if((int)*str == 56)//Test 8
        {
            _UTest_SetLevelInSize(u8level,&u16PatInwsize,&u16PatInhsize);
            SCL_ERR("[VPE]Set Pat InPutSize:(%hu,%hu) \n",u16PatInwsize,u16PatInhsize);
        }
        else if((int)*str == 57)//Test9
        {
            _UTest_SetLevelInSize(u8level,&u16IspInwsize,&u16IspInhsize);
            SCL_ERR("[VPE]Set ISP InPutSize:(%hu,%hu) \n",u16IspInwsize,u16IspInhsize);
        }
        else if((int)*str == 65)  //Test A
        {
            idx = _UTest_GetEmptyIdx();
            //MHAL_CMDQ_GetSysCmdqService()
            _UTest_CreateSclInstance(idx,u16PatInwsize,u16PatInhsize);
            _UTest_AllocateMem(idx,(u32)(u16swsize*u16shsize*2));
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgPatMode(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetCropCfg(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    (u8level ==3 || u8level ==8),u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],NULL, &stpBuffer[idx]);
                u8glevel[idx] = u8level;
            }
            MHalVpeSclSetWaitDone((void *)u32Ctx[idx],NULL,_UTest_GetWaitFlag(u8level));
            //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],u16swsize*u16shsize*2);
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestA_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2));
        }
        else if((int)*str == 'a')  //Test a
        {
            idx = _UTest_GetEmptyIdx();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            _UTest_CreateSclInstance(idx,u16PatInwsize,u16PatInhsize);
            _UTest_AllocateMem(idx,(u32)(u16swsize*u16shsize*2));
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgPatMode(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetCropCfg(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    (u8level ==3 || u8level ==8),u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                u8glevel[idx] = u8level;
            }
            if(pCmdqCfg)
            {
                _UTestClosePatternGen(pCmdqCfg);
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                do
                {
                    DrvSclOsDelayTask(5);
                    pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                }
                while(!(u32CMDQIrq&0x800));
            }
            //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],u16swsize*u16shsize*2);
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testa_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2));
        }
        else if((int)*str == 66)  //Test B
        {
            idx = _UTest_GetEmptyIdx();
            _UTest_CreateSclInstance(idx,u16PatInwsize,u16PatInhsize);
            _UTest_AllocateMem(idx,(u32)(u16swsize*u16shsize*2*4));
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgPatMode(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetCropCfg(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],1,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx]+(u32)u16swsize*u16shsize*2,1,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx]+(u32)u16swsize*u16shsize*2*2,1,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx]+(u32)u16swsize*u16shsize*2*3,1,u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                u8glevel[idx] = 9;
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
            }
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestB_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2*4));
        }
        else if((int)*str == 'b')  //Test b
        {
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            idx = _UTest_GetEmptyIdx();
            _UTest_CreateSclInstance(idx,u16PatInwsize,u16PatInhsize);
            _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2);
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgPatMode(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetCropCfg(idx, u16PatInwsize, u16PatInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8|| u8level ==9) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7|| u8level ==9) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7|| u8level ==9) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    (u8level ==3 || u8level ==8|| u8level ==9),u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],0,u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                u8glevel[idx] = u8level;
                if(pCmdqCfg)
                {
                    _UTestClosePatternGen(pCmdqCfg);
                    pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                    do
                    {
                        DrvSclOsDelayTask(5);
                        pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                    }
                    while(!(u32CMDQIrq&0x800));
                }
                //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
                //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],u16swsize*u16shsize*2);
            }
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testb_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2));
        }
        else if((int)*str == 67)  //Test C
        {
            idx = _UTest_GetIqEmptyIdx();
            _UTest_CreateIqInstance(idx);
            _UTest_SetRoiCfg(idx,pCmdqCfg);
            MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
            MHalVpeIqGetWdrRoiHist((void *)u32IqCtx[idx], &stRoiReport[idx]);
            SCL_ERR("[VPE]stRoiReport %u %u %u %u\n",
                stRoiReport[idx].u32Y[0],stRoiReport[idx].u32Y[1],stRoiReport[idx].u32Y[2],stRoiReport[idx].u32Y[3]);

            SCL_ERR( "[VPE]Set IQ inst Direct%d\n",(int)*str);
        }
        else if((int)*str == 'D')  //Test D
        {
            idx = _UTest_GetEmptyIdx();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            _UTest_CreateSclInstance(idx,u16IspInwsize,u16IspInhsize);
            _UTest_CreateIspInstance(NULL ,(void *)&u32IspCtx[idx]);
            IdIspNum[idx] = 1;
            _UTest_CreateIqInstance(idx);
            IdIqNum[idx] = 1;
            MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
            _UTest_SetIqCfg(idx,SET_all);
            _UTest_SetIqOnOff(idx,SET_all);
            _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2);
            _UTest_AllocateIspMem(idx,(u32)u16IspInwsize*u16IspInhsize*3/2);
            if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
            {
                return 0;
            }
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgISPMode(idx, u16IspInwsize, u16IspInhsize);
                _UTest_SetCropCfg(idx, u16IspInwsize, u16IspInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                MHalVpeIqProcess((void *)u32IqCtx[idx],pCmdqCfg);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    (u8level ==3 || u8level ==8) ? 1 :0,u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                stVdinfo[idx].u32Stride[0] = u16IspInwsize;
                stVdinfo[idx].u32Stride[1] = u16IspInwsize;
                stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[idx];
                stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[idx]+((u32)(u16IspInwsize*u16IspInhsize));
                MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                u8glevel[idx] = u8level;
            }
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestD_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2));
        }
        else if((int)*str == 'd')  //Test d
        {
            if(IdIspNum[u8level])
            {
                if(!_UTest_PutFile2Buffer(u8level,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                {
                    return 0;
                }
                MHalVpeIqRead3DNRTbl((void *)u32IqCtx[u8level]);
                MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
                MHalVpeSclProcess((void *)u32Ctx[u8level],pCmdqCfg, &stpBuffer[u8level]);
                MHalVpeIspProcess((void *)u32IspCtx[u8level],pCmdqCfg, &stVdinfo[u8level]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[u8level],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[u8level]));
                SCL_ERR( "[VPE]Set MHAL_CMDQ_KickOffCmdq\n");
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                if(pCmdqCfg)
                {
                    do
                    {
                        DrvSclOsDelayTask(5);
                        pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                    }
                    while(!(u32CMDQIrq&0x800));
                }
                SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testd_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                    stOutputCfg[u8level].u16Width,stOutputCfg[u8level].u16Height,u8level,(u16)(u32Time&0xFFFF)
                    ,sg_scl_yc_addr[u8level]+MIU0_BASE,(u32)(stOutputCfg[u8level].u16Width*stOutputCfg[u8level].u16Height*2));
            }
        }
        else if((int)*str == 69)  //Test E
        {
            if(IdIqNum[u8level])
            {
                MHalVpeIqRead3DNRTbl((void *)u32IqCtx[u8level]);
                SCL_ERR("[VPE]get DNR report \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'e')  //Test e
        {
            if(IdIqNum[u8level])
            {
                MHalVpeIqGetWdrRoiHist((void *)u32IqCtx[u8level], &stRoiReport[u8level]);
                SCL_ERR("[VPE]stRoiReport %u %u %u %u\n",
                    stRoiReport[u8level].u32Y[0],stRoiReport[u8level].u32Y[1],stRoiReport[u8level].u32Y[2],stRoiReport[u8level].u32Y[3]);
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'F')  //Test F
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_DEF);
                _UTest_SetIqOnOff(idx,SET_DEF);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config Default \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'G')  //Test G
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_EG);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config Edge \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'g')  //Test g
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqOnOff(idx,SET_EG);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set ONOff Edge \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'H')  //Test H
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_ES);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config Es \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'f')  //Test f
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_ES2);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config Es2 \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'h')  //Test h
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqOnOff(idx,SET_ES);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set ONOff Es \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'I')  //Test I
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_CON);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config contrast \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'i')  //Test i
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqOnOff(idx,SET_CON);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set ONOff contrast \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'J')  //Test J
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_UV);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config uv \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'j')  //Test j
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqOnOff(idx,SET_UV);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set ONOff uv \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'N')  //Test N
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqCfg(idx,SET_NR);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set Config uv \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'n')  //Test n
        {
            if(IdIqNum[u8level])
            {
                idx = u8level;
                _UTest_SetIqOnOff(idx,SET_NR);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                SCL_ERR("[VPE]Set ONOff uv \n");
            }
            SCL_ERR( "[VPE]Set %d\n",(int)*str);
        }
        else if((int)*str == 'P')  //Test P
        {
            u8Count = u8level;
            SCL_ERR("[VPE]Set Write count:%hhu \n",u8Count);
        }
        else if((int)*str == 'p')  //Test p
        {
            idx = _UTest_GetEmptyIdx();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            _UTest_CreateSclInstance(idx,u16IspInwsize,u16IspInhsize);
            _UTest_CreateIspInstance(NULL ,(void *)&u32IspCtx[idx]);
            IdIspNum[idx] = 1;
            _UTest_CreateIqInstance(idx);
            IdIqNum[idx] = 1;
            MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
            _UTest_SetIqCfg(idx,SET_NR);
            _UTest_SetIqOnOff(idx,SET_NR);
            _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2*3);
            _UTest_AllocateIspMem(idx,(u32)u16IspInwsize*u16IspInhsize*3/2);
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            if(sg_scl_yc_viraddr[idx])
            {
                _UTest_SetInputCfgISPMode(idx, u16IspInwsize, u16IspInhsize);
                _UTest_SetCropCfg(idx, u16IspInwsize, u16IspInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx]+(u32)u16swsize*u16shsize*2,
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx]+(u32)u16swsize*u16shsize*4,
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    0,u16swsize*2);
                u8glevel[idx] = u8level;
                for(i=0;i<u8Count;i++)
                {
                    if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                    {
                        return 0;
                    }
                    MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                    MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                    MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                    stVdinfo[idx].u32Stride[0] = u16IspInwsize;
                    stVdinfo[idx].u32Stride[1] = u16IspInhsize;
                    stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[idx];
                    stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[idx]+((u32)(u16IspInwsize*u16IspInhsize));
                    MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                    if(pCmdqCfg)
                    {
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                   // _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],(u32)u16swsize*u16shsize*2*3);
                }
                SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testp_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                    u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                    ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2*3));
            }
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
        }
        else if((int)*str == 'R')  //Test R
        {
            if(IdIspNum[u8level])
            {
                stRotCfg[u8level].enRotType = u8Rot;
                MHalVpeIspRotationConfig((void *)u32IspCtx[u8level],&stRotCfg[u8level]);
            }
            SCL_ERR("[VPE]Set Write level Rot:%hhu \n",u8Rot);
        }
        else if((int)*str == 'r')  //Test r
        {
            u8Rot = u8level;
            SCL_ERR("[VPE]Set Write Rot:%hhu \n",u8Rot);
        }
        else if((int)*str == 'S')  //Test S
        {
            //disable all
            _UTest_CleanInst();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            for(idx=0;idx<16;idx++)
            {
                _UTest_CreateSclInstance(idx,MAXSTRIDEINPUTWIDTH,MAXSTRIDEINPUTHEIGHT);
                IdNum[idx] = 1;
                _UTest_CreateIqInstance(idx);
                IdIqNum[idx] = 1;
                MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
                if(idx==0)
                {
                    _UTest_AllocateMem(idx,(u32)MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,1,0x1);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,0,0x1);
                }
                else
                {
                    if(pCmdqCfg)
                    {
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                    }
                }
                if(sg_scl_yc_viraddr[0])
                {
                    _UTest_SetInputCfgPatMode(idx, MAXSTRIDEINPUTWIDTH, MAXSTRIDEINPUTHEIGHT);
                    _UTest_SetCropCfg(idx, MAXSTRIDEINPUTWIDTH, MAXSTRIDEINPUTHEIGHT);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetIqCfg(idx,SET_NR);
                    _UTest_SetIqOnOff(idx,SET_NR);
                    MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==0)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==1)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==2)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==3)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                    u8glevel[idx] = u8level;
                }
            }
            if(pCmdqCfg)
            {
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                do
                {
                    DrvSclOsDelayTask(5);
                    pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                }
                while(!(u32CMDQIrq&0x800));
            }
            DrvSclOsDelayTask(1000);
            //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
            if(u8Count>1)
            {
                for(i=1;i<u8Count;i++)
                {
                    for(idx=0;idx<16;idx++)
                    {
                        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                        MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                    }
                    if(pCmdqCfg)
                    {
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                    //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
                }
            }
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestS_%dx%d_16chin1_%hx.bin a:0x%lx++0x%lx \n",
                MAXOUTPUTSTRIDEH,MAXOUTPUTSTRIDEV,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2));
        }
        else if((int)*str == 's')  //Test s
        {
            //disable all
            _UTest_CleanInst();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            for(idx=0;idx<16;idx++)
            {
                _UTest_CreateSclInstance(idx,u16IspInwsize,u16IspInhsize);
                IdNum[idx] = 1;
                _UTest_CreateIspInstance(NULL ,(void *)&u32IspCtx[idx]);
                IdIspNum[idx] = 1;
                _UTest_CreateIqInstance(idx);
                IdIqNum[idx] = 1;
                MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
                if(idx==0)
                {
                    _UTest_AllocateMem(idx,(u32)MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
                    _UTest_AllocateIspMem(idx,(u32)u16IspInwsize*u16IspInhsize*3/2);
                }
                else
                {
                    if(pCmdqCfg)
                    {
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                }
                if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                {
                    return 0;
                }
                if(sg_scl_yc_viraddr[0])
                {
                    _UTest_SetInputCfgISPMode(idx, u16IspInwsize, u16IspInhsize);
                    _UTest_SetCropCfg(idx, u16IspInwsize, u16IspInhsize);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,MAXOUTPUTportH, MAXOUTPUTportV);
                    _UTest_SetIqCfg(idx,SET_NR);
                    _UTest_SetIqOnOff(idx,SET_NR);
                    MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==0)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==1)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==2)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                        sg_scl_yc_addr[0]+(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTportV*2*(idx/4))+(u32)((idx%4)*MAXOUTPUTportH*2),
                        (u8level ==3)? 1 : 0,MAXOUTPUTSTRIDEH*2);
                    MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                    stVdinfo[idx].u32Stride[0] = u16IspInwsize;
                    stVdinfo[idx].u32Stride[1] = u16IspInwsize;
                    stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[0];
                    stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[0]+((u32)(u16IspInwsize*u16IspInhsize));
                    MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                    u8glevel[idx] = u8level;
                }
            }
            if(pCmdqCfg)
            {
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                do
                {
                    DrvSclOsDelayTask(5);
                    pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                }
                while(!(u32CMDQIrq&0x800));
            }
            //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
            if(u8Count>1)
            {
                for(i=1;i<u8Count;i++)
                {
                    for(idx=0;idx<16;idx++)
                    {
                        if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                        {
                            return 0;
                        }
                        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                        MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                        MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        if(pCmdqCfg)
                        {
                            do
                            {
                                DrvSclOsDelayTask(5);
                                pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                            }
                            while(!(u32CMDQIrq&0x800));
                        }
                    }
                    //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
                }
            }
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestS_%dx%d_16chin1_%hx.bin a:0x%lx++0x%lx \n",
                MAXOUTPUTSTRIDEH,MAXOUTPUTSTRIDEV,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2));
        }
        else if((int)*str == 'M') //Test M
        {
            u16 inputhsize[16] = {352,480,176,192,416,640,256,360,288,360,244,256,240,480,256,192};
            u16 inputvsize[16] = {288,360,244,256,240,480,256,192,352,480,176,192,416,640,256,360};
            u32 u32Addr;
            //disable all
            _UTest_CleanInst();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            for(idx=0;idx<16;idx++)
            {
                _UTest_CreateSclInstance(idx,inputhsize[idx],inputvsize[idx]);
                IdNum[idx] = 1;
                _UTest_CreateIqInstance(idx);
                IdIqNum[idx] = 1;
                u16swsize = MAXOUTPUTportH;
                u16shsize = MAXOUTPUTportV;
                if(idx==0)
                {
                    _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2*31);
                    u32Addr = sg_scl_yc_addr[0];
                }
                else
                {
                    pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                }
                if(sg_scl_yc_viraddr[0])
                {
                    _UTest_SetInputCfgPatMode(idx, inputhsize[idx], inputvsize[idx]);
                    _UTest_SetCropCfg(idx, inputhsize[idx], inputvsize[idx]);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                    _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                    _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                    MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                    //  1,2,3,4,5,6,  8,9,10,11,12,13,  , 15    =13
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                        u32Addr,(idx%7) ? 1 : 0,u16swsize*2);
                    if((idx%7))
                    {
                        u32Addr = u32Addr+(u32)(u16swsize*u16shsize*2);
                    }
                    // 0, 2,  4,   6,  8    10,    12,    14        = 8
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                        u32Addr,(idx%2) ? 0 : 1,u16swsize*2);
                    if(!(idx%2))
                    {
                        u32Addr = u32Addr+(u32)(u16swsize*u16shsize*2);
                    }
                    //  1,2  ,4,5,  7,8,   10,11,    13,14,15   =11
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                        u32Addr,(idx%3) ? 1 : 0,u16swsize*2);
                    if((idx%3))
                    {
                        u32Addr = u32Addr+(u32)(u16swsize*u16shsize*2);
                    }
                    _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                        u32Addr,0,u16swsize*2);
                    MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,1,0x1);
                    pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,0,0x1);
                }
            }
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            if(pCmdqCfg)
            {
                do
                {
                    DrvSclOsDelayTask(5);
                    pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                }
                while(!(u32CMDQIrq&0x800));
            }
            //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
            if(u8Count>1)
            {
                for(i=1;i<u8Count;i++)
                {
                    for(idx=0;idx<16;idx++)
                    {
                        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                        MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                        pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
                        pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
                        pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,1,0x1);
                        pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,0,0x1);
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                    }
                    if(pCmdqCfg)
                    {
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                    //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
                }
            }
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestM_%dx%d_multiport_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(u16swsize*u16shsize*2*31));
        }
        else if((int)*str == 'm') //Test m
        {
            //u16 inputhsize[16] = {352,480,176,192,416,640,256,360,288,360,244,256,240,480,256,192};
            //u16 inputvsize[16] = {288,360,244,256,240,480,256,192,352,480,176,192,416,640,256,360};
            u32 u32Addr = 0;
            //disable all
            if(u8level>=2)
            {
                UT_multiport_multisize(u8level,u8Count);
            }
            else
            {
                _UTest_CleanInst();
                if(pCmdqCfg==NULL)
                {
                    pCmdqCfg = _UTest_AllocateCMDQ();
                }
                //writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
                u16swsize = MAXOUTPUTportH;
                u16shsize = MAXOUTPUTportV;
                //idx=0;
                for(idx=0;idx<16;idx++)
                {
                    _UTest_CreateSclInstance(idx,u16IspInwsize,u16IspInhsize);
                    IdNum[idx] = 1;
                    _UTest_CreateIspInstance(NULL ,(void *)&u32IspCtx[idx]);
                    IdIspNum[idx] = 1;
                    _UTest_CreateIqInstance(idx);
                    IdIqNum[idx] = 1;
                    if(idx==0)
                    {
                        _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2*31);
                        _UTest_AllocateIspMem(idx,(u32)u16IspInwsize*u16IspInhsize*3/2);
                        u32Addr = sg_scl_yc_addr[0];
                    }
                    else
                    {
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                    }
                    if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                    {
                        return 0;
                    }
                    if(sg_scl_yc_viraddr[0])
                    {
                        _UTest_SetInputCfgISPMode(idx, u16IspInwsize, u16IspInhsize);
                        _UTest_SetCropCfg(idx, u16IspInwsize, u16IspInhsize);
                        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,u16swsize, u16shsize);
                        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,u16swsize, u16shsize);
                        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,u16swsize, u16shsize);
                        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,u16swsize, u16shsize);
                        MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                        //  1,2,3,4,5,6,  8,9,10,11,12,13,  , 15    =13
                        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                            u32Addr,(idx%7) ? 1 : 0,u16swsize*2);
                        if((idx%7))
                        {
                            u32Addr = u32Addr+(u16swsize*u16shsize*2);
                        }
                        // 0, 2,  4,   6,  8    10,    12,    14        = 8
                        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                            u32Addr,(idx%2) ? 0 : 1,u16swsize*2);
                        if(!(idx%2))
                        {
                            u32Addr = u32Addr+(u16swsize*u16shsize*2);
                        }
                        //  1,2  ,4,5,  7,8,   10,11,    13,14,15   =11
                        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                            u32Addr,(idx%3) ? 1 : 0,u16swsize*2);
                        if((idx%3))
                        {
                            u32Addr = u32Addr+(u32)(u16swsize*u16shsize*2);
                        }
                        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                            u32Addr,0,u16swsize*2);
                        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                        stVdinfo[idx].u32Stride[0] = u16IspInwsize;
                        stVdinfo[idx].u32Stride[1] = u16IspInwsize;
                        stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[0];
                        stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[0]+((u32)(u16IspInwsize*u16IspInhsize));
                        MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                    }
                }
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                DrvSclOsDelayTask(1000);
                //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
                if(u8Count>1)
                {
                    for(i=1;i<u8Count;i++)
                    {
                        for(idx=0;idx<16;idx++)
                        {
                            if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                            {
                                return 0;
                            }
                            MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                            MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                            MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                            MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        }
                        if(pCmdqCfg)
                        {
                            do
                            {
                                DrvSclOsDelayTask(5);
                                pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                            }
                            while(!(u32CMDQIrq&0x800));
                        }
                        //_UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
                    }
                }
                SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestM_%dx%d_multiport_%hx.bin a:0x%lx++0x%lx \n",
                    u16swsize,u16shsize,(u16)(u32Time&0xFFFF)
                    ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(u16swsize*u16shsize*2*31));
            }
        }
        else if((int)*str == 'W')  //Test W
        {
            if(IdIspNum[u8level])
            {
                idx=0;
                MHalVpeSclDbgLevel((void *)&idx);
                while(1)
                {
                    if(!_UTest_PutFile2Buffer(u8level,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                    {
                        return 0;
                    }
                    MHalVpeIqRead3DNRTbl((void *)u32IqCtx[u8level]);
                    MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
                    MHalVpeSclProcess((void *)u32Ctx[u8level],pCmdqCfg, &stpBuffer[u8level]);
                    MHalVpeIspProcess((void *)u32IspCtx[u8level],pCmdqCfg, &stVdinfo[u8level]);
                    MHalVpeSclSetWaitDone((void *)u32Ctx[u8level],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[u8level]));
                    pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                    if(pCmdqCfg)
                    {
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                }
            }
            SCL_ERR( "[VPE]Set %d Set all Dma Process\n",(int)*str);
        }
        else if((int)*str == 'w')  //Test w
        {
            idx=0;
            MHalVpeSclDbgLevel((void *)&idx);
            while(1)
            {
                for(idx=0;idx<=u8level;idx++)
                {
                    if(IdIspNum[idx])
                    {
                        if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
                        {
                            return 0;
                        }
                        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
                        MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                        MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[idx]));
                        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                        if(pCmdqCfg)
                        {
                            do
                            {
                                DrvSclOsDelayTask(5);
                                pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                            }
                            while(!(u32CMDQIrq&0x800));
                        }
                    }
                }
            }
        }

    }

    return 0;
}

int _VpeHvspTest(CLI_t *pCli, char *p)
{
    int ParamCnt;
    char *src = NULL;

    ParamCnt = CliTokenCount(pCli);
    if(ParamCnt == 1)
    {
        pCli->tokenLvl++;
        src = CliTokenPop(pCli);
        vpe_test_store(src);
    }

    return eCLI_PARSE_OK;
}
