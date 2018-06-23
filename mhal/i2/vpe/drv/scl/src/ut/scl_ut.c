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
#if defined(SCLOS_TYPE_LINUX_TEST)
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include "ms_platform.h"
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "drv_scl_vip_ioctl.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_verchk.h"
#include "mhal_vpe.h"
#include "drv_scl_vip_io.h"
typedef struct
{
    bool bEn;
    u8 u8level;
    u32 u32FrameSize;
    s32 Id;
    bool bSet;
    u16 u16fence;
    u32 u32buffer[3];
}UTestTask_t;
#define TEST_INST_MAX 64
#define M2MTEST_INST_MAX 16
#define VPE_IQAPI_TEST 0
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


static MHalVpeSclWinSize_t stM2MMaxWin[M2MTEST_INST_MAX];
static MHalVpeSclInputSizeConfig_t stM2MInputpCfg[M2MTEST_INST_MAX];
static MHalVpeSclOutputSizeConfig_t stM2MOutputCfg[M2MTEST_INST_MAX];
static MHalVpeSclDmaConfig_t stM2MOutputDmaCfg[M2MTEST_INST_MAX];
static MHalVpeSclOutputSizeConfig_t stM2MinputCfg[M2MTEST_INST_MAX];
static MHalVpeSclDmaConfig_t stM2MinputDmaCfg[M2MTEST_INST_MAX];
static MHalVpeSclOutputBufferConfig_t stM2MpBuffer[M2MTEST_INST_MAX];
static u32 u32M2MCtx[M2MTEST_INST_MAX];
static u32 IdM2MNum[M2MTEST_INST_MAX];
static char sg_m2m_yc_name[M2MTEST_INST_MAX][16];
static char KEY_DMEM_M2M_VPE_YC[20] = "VPE_M2M";
static u32 sg_m2m_yc_addr[M2MTEST_INST_MAX];
static u32 sg_m2m_yc_size[M2MTEST_INST_MAX];
static u32 *sg_m2m_yc_viraddr[M2MTEST_INST_MAX];
static u8 u8gM2Mlevel[M2MTEST_INST_MAX];


static MHalVpeSclWinSize_t stMaxWin[TEST_INST_MAX];
static MHalVpeSclCropConfig_t stCropCfg[TEST_INST_MAX];
static MHalVpeSclInputSizeConfig_t stInputpCfg[TEST_INST_MAX];
static MHalVpeSclOutputSizeConfig_t stOutputCfg[TEST_INST_MAX];
static MHalVpeSclDmaConfig_t stOutputDmaCfg[TEST_INST_MAX];
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
static MHalVpeIspVideoInfo_t stVdinfo[TEST_INST_MAX];
static struct file *readfp = NULL;
static struct file *writefp = NULL;
static struct file *readfp2 = NULL;
static struct file *writefp2 = NULL;
static struct file *readfp3 = NULL;
static struct file *writefp3 = NULL;
static struct file *readfp4 = NULL;
static struct file *writefp4 = NULL;
static MHAL_CMDQ_CmdqInterface_t *pCmdqCfg=NULL;
static MHAL_CMDQ_CmdqInterface_t *pM2MCmdqCfg=NULL;
static u8 gu8M2MCount = 1;
UTestTask_t gstUtDispTaskCfg;
UTestTask_t gstUtM2MTaskCfg;
static u8 u8glevel[TEST_INST_MAX];
static u8 gu8Count = 1;
static u16 gu16swsize = FHDOUTPUTWIDTH;
static u16 gu16shsize = FHDOUTPUTHEIGHT;
static u16 gu16IspInwsize = ISPINPUTWIDTH;
static u16 gu16IspInhsize = ISPINPUTHEIGHT;
static u16 gu16PatInwsize = MAXINPUTWIDTH;
static u16 gu16PatInhsize = MAXINPUTHEIGHT;
static u16 gu16M2MInwsize = MAXINPUTWIDTH;
static u16 gu16M2MInhsize = MAXINPUTHEIGHT;
static u16 gu16M2Mwsize = MAXINPUTWIDTH;
static u16 gu16M2Mhsize = MAXINPUTHEIGHT;
static u8 gu8Str[10];
u32 u32EnISR[2] = {0,0};
#endif
//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------
#if defined(SCLOS_TYPE_LINUX_TEST)
u8 _mdrv_Scl_Changebuf2hex(int u32num);
void _UTest_FdRewind(struct file *fp);
struct file *_UTest_OpenFile(char *path,int flag,int mode);
int _UTest_WriteFile(struct file *fp,char *buf,int writelen);
int _UTest_CloseFile(struct file *fp);
void UTest_init(void)
{
    DrvSclOsMemset(stMaxWin,0,sizeof(MHalVpeSclWinSize_t)*TEST_INST_MAX);
    DrvSclOsMemset(stM2MMaxWin,0,sizeof(MHalVpeSclWinSize_t)*M2MTEST_INST_MAX);
    DrvSclOsMemset(stCropCfg,0,sizeof(MHalVpeSclCropConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stInputpCfg,0,sizeof(MHalVpeSclInputSizeConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stOutputCfg,0,sizeof(MHalVpeSclOutputSizeConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stOutputDmaCfg,0,sizeof(MHalVpeSclDmaConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stpBuffer,0,sizeof(MHalVpeSclOutputBufferConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stM2MInputpCfg,0,sizeof(MHalVpeSclInputSizeConfig_t)*M2MTEST_INST_MAX);
    DrvSclOsMemset(stM2MOutputCfg,0,sizeof(MHalVpeSclOutputSizeConfig_t)*M2MTEST_INST_MAX);
    DrvSclOsMemset(stM2MOutputDmaCfg,0,sizeof(MHalVpeSclDmaConfig_t)*M2MTEST_INST_MAX);
    DrvSclOsMemset(stM2MpBuffer,0,sizeof(MHalVpeSclOutputBufferConfig_t)*M2MTEST_INST_MAX);
    DrvSclOsMemset(u32Ctx,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(u32M2MCtx,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(u32IqCtx,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(u32IspCtx,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(IdIqNum,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(IdIspNum,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(IdNum,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(IdM2MNum,0,sizeof(u32)*TEST_INST_MAX);
    DrvSclOsMemset(stIspInputCfg,0,sizeof(MHalVpeIspInputConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stVdinfo,0,sizeof(MHalVpeIspVideoInfo_t)*TEST_INST_MAX);
    DrvSclOsMemset(stIqCfg,0,sizeof(MHalVpeIqConfig_t)*TEST_INST_MAX);
    DrvSclOsMemset(stIqOnCfg,0,sizeof(MHalVpeIqOnOff_t)*TEST_INST_MAX);
    DrvSclOsMemset(stRoiReport,0,sizeof(MHalVpeIqWdrRoiReport_t)*TEST_INST_MAX);
    DrvSclOsMemset(stHistCfg,0,sizeof(MHalVpeIqWdrRoiHist_t)*TEST_INST_MAX);

}
int _UTest_ReadFile(struct file *fp,char *buf,int readlen)
{
    off_t u32current;
    off_t u32end;
    u32current = fp->f_op->llseek(fp,0,SEEK_CUR);
    u32end = fp->f_op->llseek(fp,0,SEEK_END);
    if ((u32end - u32current) < readlen)
    {
        SCL_DBGERR("[%s]length err\n",__FUNCTION__);
        return -1;
    }
    else
    {
        SCL_DBGERR("[%s]u32current :%lx u32end:%lx\n",__FUNCTION__,u32current,u32end);
    }
    fp->f_op->llseek(fp,u32current,SEEK_SET);
    if (fp->f_op && fp->f_op->read)
    {
        fp->f_op->read(fp,buf,readlen, &fp->f_pos);
        return 1;
    }
    else
    {
        SCL_ERR("[%s]read err\n",__FUNCTION__);
        return -1;
    }
}
void _check_m2mproc_storeNaming(u32 u32Id)
{
    sg_m2m_yc_name[u32Id][0] = 48+(((u32Id&0xFFF)%1000)/100);
    sg_m2m_yc_name[u32Id][1] = 48+(((u32Id&0xFFF)%100)/10);
    sg_m2m_yc_name[u32Id][2] = 48+(((u32Id&0xFFF)%10));
    sg_m2m_yc_name[u32Id][3] = '_';
    sg_m2m_yc_name[u32Id][4] = '\0';
    DrvSclOsStrcat(sg_m2m_yc_name[u32Id],KEY_DMEM_M2M_VPE_YC);
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
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        if(IdM2MNum[i])
        {
            if(sg_m2m_yc_viraddr[i])
            {
                DrvSclOsDirectMemFree(sg_m2m_yc_name[i],sg_m2m_yc_size[i],(void*)sg_m2m_yc_viraddr[i],(DrvSclOsDmemBusType_t)sg_m2m_yc_addr[i]);
                sg_m2m_yc_viraddr[i] = NULL;
            }
            MHalVpeDestroySclInstance((void *)u32M2MCtx[i]);
            IdM2MNum[i] = 0;
        }
    }
    if(readfp)
    {
        _UTest_CloseFile(readfp);
        readfp = NULL;
    }
    if(writefp)
    {
        _UTest_CloseFile(writefp);
        writefp = NULL;
    }
    MHalVpeDeInit();
}
bool _UTest_PutFile2BufferAddr(char *path,u32 size,u32* u32Addr)
{
    if(!readfp)
    readfp = _UTest_OpenFile(path,O_RDONLY,0);
    if(readfp)
    {
        if(_UTest_ReadFile(readfp,(char *)u32Addr,size)==1)
        {
            DrvSclOsDirectMemFlush((u32)u32Addr,size);
            DrvSclOsWaitForCpuWriteToDMem();
        }
        else
        {
            _UTest_FdRewind(readfp);
            if(_UTest_ReadFile(readfp,(char *)u32Addr,size)==1)
            {
                DrvSclOsDirectMemFlush((u32)u32Addr,size);
                DrvSclOsWaitForCpuWriteToDMem();
            }
            else
            {
                SCL_ERR("Test Fail\n");
                return 0;
            }
        }
    }
    else
    {
        SCL_ERR("Test Fail\n");
        return 0;
    }
    return 1;
}
bool _UTest_PutFile2Buffer(int idx,char *path,u32 size)
{
    if(!readfp)
    readfp = _UTest_OpenFile(path,O_RDONLY,0);
    if(readfp)
    {
        if(_UTest_ReadFile(readfp,(char *)sg_Isp_yc_viraddr[idx],size)==1)
        {
            DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
            DrvSclOsWaitForCpuWriteToDMem();
        }
        else
        {
            _UTest_FdRewind(readfp);
            if(_UTest_ReadFile(readfp,(char *)sg_Isp_yc_viraddr[idx],size)==1)
            {
                DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
                DrvSclOsWaitForCpuWriteToDMem();
            }
            else
            {
                SCL_ERR("Test Fail\n");
                return 0;
            }
        }
    }
    else
    {
        SCL_ERR("Test Fail\n");
        return 0;
    }
    return 1;
}
bool _UTest_PutFile2Buffer2(int idx,char *path,u32 size)
{
    if(!readfp2)
    readfp2 = _UTest_OpenFile(path,O_RDONLY,0);
    if(readfp2)
    {
        if(_UTest_ReadFile(readfp2,(char *)sg_Isp_yc_viraddr[idx],size)==1)
        {
            DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
            DrvSclOsWaitForCpuWriteToDMem();
        }
        else
        {
            _UTest_FdRewind(readfp2);
            if(_UTest_ReadFile(readfp2,(char *)sg_Isp_yc_viraddr[idx],size)==1)
            {
                DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
                DrvSclOsWaitForCpuWriteToDMem();
            }
            else
            {
                SCL_ERR("Test Fail\n");
                return 0;
            }
        }
    }
    else
    {
        SCL_ERR("Test Fail\n");
        return 0;
    }
    return 1;
}
bool _UTest_PutFile2Buffer3(int idx,char *path,u32 size)
{
    if(!readfp3)
    readfp3 = _UTest_OpenFile(path,O_RDONLY,0);
    if(readfp3)
    {
        if(_UTest_ReadFile(readfp3,(char *)sg_Isp_yc_viraddr[idx],size)==1)
        {
            DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
            DrvSclOsWaitForCpuWriteToDMem();
        }
        else
        {
            _UTest_FdRewind(readfp3);
            if(_UTest_ReadFile(readfp3,(char *)sg_Isp_yc_viraddr[idx],size)==1)
            {
                DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
                DrvSclOsWaitForCpuWriteToDMem();
            }
            else
            {
                SCL_ERR("Test Fail\n");
                return 0;
            }
        }
    }
    else
    {
        SCL_ERR("Test Fail\n");
        return 0;
    }
    return 1;
}
bool _UTest_PutFile2Buffer4(int idx,char *path,u32 size)
{
    if(!readfp4)
    readfp4 = _UTest_OpenFile(path,O_RDONLY,0);
    if(readfp4)
    {
        if(_UTest_ReadFile(readfp4,(char *)sg_Isp_yc_viraddr[idx],size)==1)
        {
            DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
            DrvSclOsWaitForCpuWriteToDMem();
        }
        else
        {
            _UTest_FdRewind(readfp4);
            if(_UTest_ReadFile(readfp4,(char *)sg_Isp_yc_viraddr[idx],size)==1)
            {
                DrvSclOsDirectMemFlush((u32)sg_Isp_yc_viraddr[idx],size);
                DrvSclOsWaitForCpuWriteToDMem();
            }
            else
            {
                SCL_ERR("Test Fail\n");
                return 0;
            }
        }
    }
    else
    {
        SCL_ERR("Test Fail\n");
        return 0;
    }
    return 1;
}
MHAL_CMDQ_CmdqInterface_t* _UTest_AllocateCMDQ(void)
{
    MHAL_CMDQ_BufDescript_t stCfg;
    MHAL_CMDQ_CmdqInterface_t *pCfg=NULL;
    stCfg.u16MloadBufSizeAlign = 32;
    stCfg.u32CmdqBufSizeAlign = 32;
    stCfg.u32CmdqBufSize = 0x80000;
    stCfg.u32MloadBufSize = 0x10000;
#if defined(SCLOS_TYPE_LINUX_TESTCMDQ)
    pCfg = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE,&stCfg,0);
#endif
    return pCfg;
}
MHAL_CMDQ_CmdqInterface_t* _UTest_AllocateCMDQM2M(void)
{
    MHAL_CMDQ_BufDescript_t stCfg;
    MHAL_CMDQ_CmdqInterface_t *pCfg=NULL;
    stCfg.u16MloadBufSizeAlign = 32;
    stCfg.u32CmdqBufSizeAlign = 32;
    stCfg.u32CmdqBufSize = 0x10000;
    stCfg.u32MloadBufSize = 0x10000;
#if defined(SCLOS_TYPE_LINUX_TESTCMDQ)
    pCfg = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC1,&stCfg,0);
#endif
    return pCfg;
}
ssize_t  _UTest_CreateSclSubInstance(int idx, u16 u16Width, u16 u16Height,MHalSclInputPortMode_e enType)
{
    int a = 0xFF;
    MHalVpeSclCreate_t stCfg;
    stM2MMaxWin[idx].u16Height = u16Height;
    stM2MMaxWin[idx].u16Width = u16Width;
    stCfg.enSclInstance = E_MHAL_VPE_SCL_SUB;
    stCfg.enInputPortMode = enType;
    stCfg.enOutmode = E_MHAL_SCL_OUTPUT_MODE_PORT2_M2M_MODE;
    stCfg.enTopIn = E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE;
    stCfg.stMaxWin.u16Height = u16Height;
    stCfg.stMaxWin.u16Width = u16Width;
    MHalVpeInit(NULL,NULL);
    if(!MHalVpeCreateSclInstance(NULL, &stCfg,(void *)&u32M2MCtx[idx]))
    {
        SCL_ERR( "[VPE]create Fail%s\n",sg_scl_yc_name[idx]);
        return 0;
    }
    MHalVpeSclDbgLevel((void *)&a);
    MHalVpeIspDbgLevel((void *)&a);
    return 1;
}
ssize_t  _UTest_CreateSclInstance
(int idx, u16 u16Width, u16 u16Height,MHalVpeTopInputMode_e enType,MHalSclOutputMode_e enOutmode)
{
    int a = 0xFF;
    MHalVpeSclCreate_t stCfg;
    stMaxWin[idx].u16Height = u16Height;
    stMaxWin[idx].u16Width = u16Width;
    stCfg.enSclInstance = E_MHAL_VPE_SCL_MAIN;
    stCfg.enTopIn = enType;
    stCfg.enInputPortMode = E_MHAL_SCL_MAIN_INPUT_MODE_REAL_TIME;
    stCfg.enOutmode = enOutmode;
    stCfg.stMaxWin.u16Height = u16Height;
    stCfg.stMaxWin.u16Width = u16Width;
    MHalVpeInit(NULL,NULL);
    if(!MHalVpeCreateSclInstance(NULL, &stCfg,(void *)&u32Ctx[idx]))
    {
        SCL_ERR( "[VPE]create Fail%s\n",sg_scl_yc_name[idx]);
        return 0;
    }
    MHalVpeSclDbgLevel((void *)&a);
    MHalVpeIspDbgLevel((void *)&a);
    return 1;
}

ssize_t  _UTest_CreateIspInstance(void **pCtx,MHalVpeTopInputMode_e enType)
{
	int a = 0xFF;
    MhalVpeIspCreate_t stCfg;
    stCfg.eInput = enType;
    stCfg.eHDRMode = E_MHAL_HDR_TYPE_OFF;
	if(!MHalVpeCreateIspInstance(NULL,&stCfg,pCtx))
    {
        SCL_ERR( "[VPE]create ISP Inst Fail\n");
        return 0;
    }

    MHalVpeIspDbgLevel((void *)&a);
    return 1;
}
void _UTest_AllocateM2MMem(int idx,u32 size)
{
    _check_m2mproc_storeNaming(idx);
    sg_m2m_yc_size[idx] = size;
    sg_m2m_yc_viraddr[idx] = DrvSclOsDirectMemAlloc(sg_m2m_yc_name[idx], sg_m2m_yc_size[idx],
        (DrvSclOsDmemBusType_t *)&sg_m2m_yc_addr[idx]);
    DrvSclOsMemset(sg_m2m_yc_viraddr[idx],0,size);
}
void _UTest_AllocateMem(int idx,u32 size)
{
    _check_sclproc_storeNaming(idx);
    sg_scl_yc_size[idx] = size;
    sg_scl_yc_viraddr[idx] = DrvSclOsDirectMemAlloc(sg_scl_yc_name[idx], sg_scl_yc_size[idx],
        (DrvSclOsDmemBusType_t *)&sg_scl_yc_addr[idx]);
    DrvSclOsMemset(sg_scl_yc_viraddr[idx],0,size);
}
void _UTest_AllocateIspMem(int idx,u32 size)
{
    _check_Ispproc_storeNaming(idx);
    sg_Isp_yc_size[idx] = size;
    sg_Isp_yc_viraddr[idx] = DrvSclOsDirectMemAlloc(sg_Isp_yc_name[idx], sg_Isp_yc_size[idx],
        (DrvSclOsDmemBusType_t *)&sg_Isp_yc_addr[idx]);
    DrvSclOsMemset(sg_Isp_yc_viraddr[idx],0,size);
    SCL_ERR("[%s]vir:%lx ,phy:%lx",__FUNCTION__,(u32)sg_Isp_yc_viraddr[idx],sg_Isp_yc_addr[idx]);
}
void _UTest_SetInputCfgISP422Mode(int idx, u16 u16Width, u16 u16Height)
{
    stIspInputCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stIspInputCfg[idx].enInType = E_MHAL_ISP_INPUT_YUV422;
    stIspInputCfg[idx].ePixelFormat = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
    stIspInputCfg[idx].u32Height = u16Height;
    stIspInputCfg[idx].u32Width = u16Width;
    MHalVpeIspInputConfig((void *)u32IspCtx[idx], &stIspInputCfg[idx]);
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
void _UTest_SetInputCfgM2MMode(int idx, u16 u16Width, u16 u16Height)
{
    stM2MInputpCfg[idx].eCompressMode = E_MHAL_COMPRESS_MODE_NONE;
    stM2MInputpCfg[idx].ePixelFormat = stM2MInputpCfg[idx].ePixelFormat;
    stM2MInputpCfg[idx].u16Height = u16Height;
    stM2MInputpCfg[idx].u16Width = u16Width;
    MHalVpeSclInputConfig((void *)u32M2MCtx[idx], &stM2MInputpCfg[idx]);
}
void _UTest_SetCropCfg(int idx, u16 u16Width, u16 u16Height)
{
#if 0
    stCropCfg[idx].bCropEn = (idx%2) ? 1 :0;
    stCropCfg[idx].stCropWin.u16Height = (idx%2) ? MAXCROPHEIGHT(u16Height) : u16Height;
    stCropCfg[idx].stCropWin.u16Width = (idx%2) ? MAXCROPWIDTH(u16Width) : u16Width;
    stCropCfg[idx].stCropWin.u16X = (idx%2) ? MAXCROPX(u16Width) : 0;
    stCropCfg[idx].stCropWin.u16Y = (idx%2) ? MAXCROPY(u16Height) : 0;
    #else
    stCropCfg[idx].bCropEn = 0;
    stCropCfg[idx].stCropWin.u16Height = u16Height;
    stCropCfg[idx].stCropWin.u16Width =u16Width;
    stCropCfg[idx].stCropWin.u16X = 0;
    stCropCfg[idx].stCropWin.u16Y = 0;
    #endif
    MHalVpeSclCropConfig((void *)u32Ctx[idx], &stCropCfg[idx]);
}
void _UTest_SetDmaCfg(int idx, MHalPixelFormat_e enColor, MHalVpeDmaPort_e enPort)
{
    stOutputDmaCfg[idx].enCompress = E_MHAL_COMPRESS_MODE_NONE;
    stOutputDmaCfg[idx].enFormat = enColor;
    stOutputDmaCfg[idx].enPort = enPort;
    MHalVpeSclDmaPortConfig((void *)u32Ctx[idx], &stOutputDmaCfg[idx]);
}
void _UTest_SetM2MDmaCfg(int idx, MHalPixelFormat_e enColor, MHalVpeDmaPort_e enPort)
{
    stM2MOutputDmaCfg[idx].enCompress = E_MHAL_COMPRESS_MODE_NONE;
    stM2MOutputDmaCfg[idx].enFormat = enColor;
    stM2MOutputDmaCfg[idx].enPort = enPort;
    MHalVpeSclDmaPortConfig((void *)u32M2MCtx[idx], &stM2MOutputDmaCfg[idx]);
}
void _UTest_SetM2MinDmaCfg(int idx, MHalPixelFormat_e enColor, MHalVpeDmaPort_e enPort)
{
    stM2MinputDmaCfg[idx].enCompress = E_MHAL_COMPRESS_MODE_NONE;
    stM2MinputDmaCfg[idx].enFormat = enColor;
    stM2MinputDmaCfg[idx].enPort = enPort;
    stM2MInputpCfg[idx].ePixelFormat = enColor;
    MHalVpeSclDmaPortConfig((void *)u32M2MCtx[idx], &stM2MinputDmaCfg[idx]);
}
void _UTest_SetM2MOutputSizeCfg(int idx, MHalVpeDmaPort_e enPort, u16 u16Width, u16 u16Height)
{
    stM2MOutputCfg[idx].enOutPort = enPort;
    stM2MOutputCfg[idx].u16Height = u16Height;
    stM2MOutputCfg[idx].u16Width = u16Width;
    MHalVpeSclPortSizeConfig((void *)u32M2MCtx[idx], &stM2MOutputCfg[idx]);
}
void _UTest_SetM2MInputSizeCfg(int idx, MHalVpeDmaPort_e enPort, u16 u16Width, u16 u16Height)
{
    stM2MinputCfg[idx].enOutPort = enPort;
    stM2MinputCfg[idx].u16Height = u16Height;
    stM2MinputCfg[idx].u16Width = u16Width;
    stM2MInputpCfg[idx].u16Height = u16Height;
    stM2MInputpCfg[idx].u16Width = u16Width;
    MHalVpeSclPortSizeConfig((void *)u32M2MCtx[idx], &stM2MinputCfg[idx]);
}
void _UTest_SetOutputSizeCfg(int idx, MHalVpeDmaPort_e enPort, u16 u16Width, u16 u16Height)
{
    stOutputCfg[idx].enOutPort = enPort;
    stOutputCfg[idx].u16Height = u16Height;
    stOutputCfg[idx].u16Width = u16Width;
    MHalVpeSclPortSizeConfig((void *)u32Ctx[idx], &stOutputCfg[idx]);
}
int _UTest_GetM2MEmptyIdx(void)
{
    int idx=-1,i;
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        if(IdM2MNum[i]==0)
        {
            IdM2MNum[i] = 1;
            idx = i;
            break;
        }
    }
    return idx;
}
int _UTest_GetEmptyIdx(void)
{
    int idx,i;
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
    int idx,i;
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
void _UTest_SetM2MInProcessCfg(int idx, MHalVpeDmaPort_e enPort, u32 u32AddrY,bool bEn,u32 u32Stride)
{
    stM2MpBuffer[idx].stCfg[enPort].bEn = bEn;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[0] = u32Stride;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[1] = u32Stride;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[0] = u32AddrY;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[1] = u32AddrY+(stM2MInputpCfg[idx].u16Width*stM2MInputpCfg[idx].u16Height);
}
void _UTest_SetM2MProcessCfg(int idx, MHalVpeDmaPort_e enPort, u32 u32AddrY,bool bEn,u32 u32Stride)
{
    stM2MpBuffer[idx].stCfg[enPort].bEn = bEn;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[0] = u32Stride;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[1] = u32Stride;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[0] = u32AddrY;
    stM2MpBuffer[idx].stCfg[enPort].stBufferInfo.u64PhyAddr[1] = u32AddrY+(stM2MOutputCfg[idx].u16Width*stM2MOutputCfg[idx].u16Height);
}
void _UTest_SetProcessCfg(int idx, MHalVpeDmaPort_e enPort, u32 u32AddrY,bool bEn,u32 u32Stride)
{
    stpBuffer[idx].stCfg[enPort].bEn = bEn;
    stpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[0] = u32Stride;
    stpBuffer[idx].stCfg[enPort].stBufferInfo.u32Stride[1] = u32Stride;
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
        stHistCfg[idx].stRoiCfg[i].bEnSkip = 0;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccX[0] = 100+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccY[0] = 100+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccX[1] = 200+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccY[1] = 100+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccX[2] = 200+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccY[2] = 200+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccX[3] = 100+i*100;
        stHistCfg[idx].stRoiCfg[i].u16RoiAccY[3] = 200+i*100;
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
    else if(enType==SET_NR)
    {
        DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
        stIqCfg[idx].u8NRY_SF_STR = 150;
        stIqCfg[idx].u8NRY_TF_STR = 150;
        stIqCfg[idx].u8NRC_SF_STR = 150;
        stIqCfg[idx].u8NRC_TF_STR = 150;
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
void _UTest_SetIqCfgYEE(int idx, u8 *data)
{

    DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
    stIqCfg[idx].u8NRY_SF_STR = 0;
    stIqCfg[idx].u8NRY_TF_STR = 0;
    stIqCfg[idx].u8NRC_SF_STR = 0;
    stIqCfg[idx].u8NRC_TF_STR = 0;
    stIqCfg[idx].u8NRY_BLEND_MOTION_TH = 0;
    stIqCfg[idx].u8NRY_BLEND_STILL_TH = 1;
    stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = 0;
    stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = 0;
    stIqCfg[idx].u8NRY_BLEND_STILL_WEI = 0;/**/
    stIqCfg[idx].u8EdgeGain[0] = data[0];
    stIqCfg[idx].u8EdgeGain[1] = data[1];
    stIqCfg[idx].u8EdgeGain[2] = data[2];
    stIqCfg[idx].u8EdgeGain[3] = data[3];
    stIqCfg[idx].u8EdgeGain[4] = data[4];
    stIqCfg[idx].u8EdgeGain[5] = data[5];
    SCL_ERR( "[VPE]Val %d, %d, %d, %d, %d, %d\n",data[0],data[1],data[2],data[3],data[4],data[5]);
    MHalVpeIqConfig((void *)u32IqCtx[idx], &stIqCfg[idx]);
}

void _UTest_SetIqCfgMCNR(int idx, u8 *data)
{
    DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
    stIqCfg[idx].u8NRY_SF_STR = data[0];
    stIqCfg[idx].u8NRY_TF_STR = data[1];
    stIqCfg[idx].u8NRC_SF_STR = data[2];
    stIqCfg[idx].u8NRC_TF_STR = data[3];
    stIqCfg[idx].u8NRY_BLEND_MOTION_TH = data[4]/25;
    stIqCfg[idx].u8NRY_BLEND_STILL_TH = data[5]/25;
    stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = data[6]/25;
    stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = data[7]/25;
    stIqCfg[idx].u8NRY_BLEND_STILL_WEI = data[8]/25;
    stIqCfg[idx].u8EdgeGain[0] = 0;
    stIqCfg[idx].u8EdgeGain[1] = 20;
    stIqCfg[idx].u8EdgeGain[2] = 80;
    stIqCfg[idx].u8EdgeGain[3] = 120;
    stIqCfg[idx].u8EdgeGain[4] = 160;
    stIqCfg[idx].u8EdgeGain[5] = 160;
   SCL_ERR( "[VPE]Val %d, %d, %d, %d, %d, %d, %d, %d, %d\n",data[0]/25,data[1]/25,data[2]/25,data[3]/25,data[4]/25,data[5]/25,data[6]/25,data[7]/25,data[8]/25);

    MHalVpeIqConfig((void *)u32IqCtx[idx], &stIqCfg[idx]);
}
void _UTest_SetIqCfgWDR(int idx, u8 *data)
{
    DrvSclOsMemset(&stIqCfg[idx],0,sizeof(MHalVpeIqConfig_t));
    stIqCfg[idx].u8NRY_SF_STR = 0;
    stIqCfg[idx].u8NRY_TF_STR = 0;
    stIqCfg[idx].u8NRC_SF_STR = 0;
    stIqCfg[idx].u8NRC_TF_STR = 0;
    stIqCfg[idx].u8NRY_BLEND_MOTION_TH = 0;
    stIqCfg[idx].u8NRY_BLEND_STILL_TH = 1;
    stIqCfg[idx].u8NRY_BLEND_MOTION_WEI = 0;
    stIqCfg[idx].u8NRY_BLEND_OTHER_WEI = 0;
    stIqCfg[idx].u8NRY_BLEND_STILL_WEI = 0;/**/
    stIqCfg[idx].u8EdgeGain[0] = 0;
    stIqCfg[idx].u8EdgeGain[1] = 20;
    stIqCfg[idx].u8EdgeGain[2] = 80;
    stIqCfg[idx].u8EdgeGain[3] = 120;
    stIqCfg[idx].u8EdgeGain[4] = 160;
    stIqCfg[idx].u8EdgeGain[5] = 160;
    stIqCfg[idx].u8Contrast= data[0];
    SCL_ERR( "[VPE]Val %d,\n",data[0]);

    MHalVpeIqConfig((void *)u32IqCtx[idx], &stIqCfg[idx]);
}
void _UTest_SetIqCfgES(int idx, u8 *data)
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
    }
    MHalVpeIqOnOff((void *)u32IqCtx[idx], &stIqOnCfg[idx]);
}
void _UTest_SetIqOnOff1(int idx,UTest_SetIqCfg_e enType, int en)
{
    if(enType==SET_DEF)
    {
        DrvSclOsMemset(&stIqOnCfg[idx],0,sizeof(MHalVpeIqOnOff_t));
    }
    else if(enType==SET_EG)
    {
        stIqOnCfg[idx].bEdgeEn = en;
    }
    else if(enType==SET_ES)
    {
        stIqOnCfg[idx].bESEn = en;
    }
    else if(enType==SET_CON)
    {
        stIqOnCfg[idx].bContrastEn= en;
    }
    else if(enType==SET_UV)
    {
        stIqOnCfg[idx].bUVInvert= en;
    }
    else if(enType==SET_NR)
    {
        stIqOnCfg[idx].bNREn= en;
    }
    else if(enType==SET_all)
    {
        stIqOnCfg[idx].bNREn= en;
        stIqOnCfg[idx].bContrastEn= en;
        stIqOnCfg[idx].bESEn = en;
    }
    MHalVpeIqOnOff((void *)u32IqCtx[idx], &stIqOnCfg[idx]);
}

ssize_t  _UTest_CreateIqInstance(int idx,MHalVpeTopInputMode_e enType)
{
    int a = 0;
    DrvSclOsMemset(&stRoiReport[idx],0,sizeof(MHalVpeIqWdrRoiReport_t));
    MHalVpeCreateIqInstance(NULL , &stMaxWin[idx],enType,(void *)&u32IqCtx[idx]);
    MHalVpeIqDbgLevel((void *)&a);
    _UTest_SetIqCfg(idx,SET_DEF);
    _UTest_SetIqOnOff(idx,SET_DEF);
    return 1;
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
void _UTest_SetISPProcess(int idx,u16 stride,u16 vsize)
{
    stVdinfo[idx].u32Stride[0] = stride;
    stVdinfo[idx].u32Stride[1] = stride;
    stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[idx];
    stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[idx]+((u32)(stride*vsize));
    MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);

}
void UT_multiport_multisize(u8 u8level,u8 u8Count)
{
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
    writefp = _UTest_OpenFile("/tmp/Scout_ch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_ch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_ch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_ch3.bin",O_WRONLY|O_CREAT,0777);
    }
    for(idx=0;idx<u8level;idx++)
    {
        u32IspinH = (idx ==0) ? CH0INWIDTH : (idx ==1) ? CH1INWIDTH : (idx ==2) ? CH2INWIDTH : CH3INWIDTH;
        u32IspinV = (idx ==0) ? CH0INHEIGHT : (idx ==1) ? CH1INHEIGHT: (idx ==2) ? CH2INHEIGHT : CH3INHEIGHT;
        u32ScOutH = (idx ==0) ? CH0OUTWIDTH : (idx ==1) ? CH1OUTWIDTH :(idx ==2) ? CH2OUTWIDTH : CH3OUTWIDTH;
        u32ScOutV = (idx ==0) ? CH0OUTHEIGHT: (idx ==1) ? CH1OUTHEIGHT: (idx ==2) ? CH2OUTHEIGHT : CH3OUTHEIGHT;
        _UTest_CreateSclInstance
        (idx,u32IspinH,u32IspinV,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        IdNum[idx] = 1;
        _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        _UTest_SetIqCfg(idx,SET_all);
        _UTest_SetIqOnOff(idx,SET_all);
        _UTest_AllocateMem(idx,(u32)u32ScOutH*u32ScOutV*2);
        _UTest_AllocateIspMem(idx,(u32)u32IspinH*u32IspinV*3/2);
        u32Addr = sg_scl_yc_addr[idx];
        if(idx ==0)
        {
            if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin_ch0.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==1)
        {
            if(!_UTest_PutFile2Buffer2(idx,"/tmp/Ispin_ch1.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==2)
        {
            if(!_UTest_PutFile2Buffer3(idx,"/tmp/Ispin_ch2.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
            {
                return ;
            }
        }
        else if(idx ==3)
        {
            if(!_UTest_PutFile2Buffer4(idx,"/tmp/Ispin_ch3.yuv",(u32)(u32IspinH*u32IspinV*3/2)))
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
            _UTest_SetISPProcess(idx,u32IspinH,u32IspinV);
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
}
void UT_DispTask(void *arg)
{
    u32 u32CMDQIrq;
    while(gstUtDispTaskCfg.bEn)
    {
        _UTest_PutFile2Buffer(gstUtDispTaskCfg.u8level,"/tmp/Ispin.yuv",(u32)gstUtDispTaskCfg.u32FrameSize);
        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[gstUtDispTaskCfg.u8level]);
        MHalVpeIqProcess((void *)u32IqCtx[gstUtDispTaskCfg.u8level], pCmdqCfg);
        MHalVpeSclProcess((void *)u32Ctx[gstUtDispTaskCfg.u8level],pCmdqCfg, &stpBuffer[gstUtDispTaskCfg.u8level]);
        MHalVpeIspProcess((void *)u32IspCtx[gstUtDispTaskCfg.u8level],pCmdqCfg, &stVdinfo[gstUtDispTaskCfg.u8level]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[gstUtDispTaskCfg.u8level],pCmdqCfg,
            _UTest_GetWaitFlag(u8glevel[gstUtDispTaskCfg.u8level]));
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
        DrvSclOsDelayTask(1000);
    }
}
void _UTest_CreateM2M_Task(u8 u8level,u32 u32Framesize,TaskEntry pf)
{
    gstUtM2MTaskCfg.bEn = 1;
    gstUtM2MTaskCfg.u8level = u8level;
    gstUtM2MTaskCfg.u32FrameSize = u32Framesize;
    gstUtM2MTaskCfg.bSet = 0;
    gstUtM2MTaskCfg.u16fence = 0;
    gstUtM2MTaskCfg.Id  = DrvSclOsCreateTask(pf,(MS_U32)NULL,TRUE,(char*)"UTM2MTask");
    if (gstUtM2MTaskCfg.Id == -1)
    {
        SCL_ERR("[UT]%s create task fail\n", __FUNCTION__);
        return;
    }
}
void _UTest_Create_Task(u8 u8level,u32 u32Framesize,TaskEntry pf)
{
    gstUtDispTaskCfg.bEn = 1;
    gstUtDispTaskCfg.u8level = u8level;
    gstUtDispTaskCfg.u32FrameSize = u32Framesize;
    gstUtDispTaskCfg.bSet = 0;
    gstUtDispTaskCfg.u16fence = 0;
    gstUtDispTaskCfg.Id  = DrvSclOsCreateTask(pf,(MS_U32)NULL,TRUE,(char*)"UTDISPTask");
    if (gstUtDispTaskCfg.Id == -1)
    {
        SCL_ERR("[UT]%s create task fail\n", __FUNCTION__);
        return;
    }
}
void _UTest_Destroy_Task(void)
{
    if (gstUtDispTaskCfg.bEn)
    {
        gstUtDispTaskCfg.bEn = 0;
        DrvSclOsDeleteTask(gstUtDispTaskCfg.Id);
    }
    if (gstUtM2MTaskCfg.bEn)
    {
        gstUtM2MTaskCfg.bEn = 0;
        DrvSclOsDeleteTask(gstUtM2MTaskCfg.Id);
    }
}
void _UTestClosePatternGen(MHAL_CMDQ_CmdqInterface_t *pCmdqinf)
{
    pCmdqinf->MHAL_CMDQ_WriteRegCmdqMask(pCmdqinf,0x152580,0,0x8000);
    pCmdqinf->MHAL_CMDQ_WriteRegCmdqMask(pCmdqinf,0x1525E0,0,0x1);
}
irqreturn_t _UTCallback(int eIntNum, void* dev_id)
{
    MHalVpeIrqStatus_e eIrqstst;
    unsigned int  nIrqNum = 0 ;
    int u32int = 0;
    MHalVpeSclGetIrqNum(&nIrqNum,E_MHAL_SCL_IRQ_M2M_MODE);
    if(nIrqNum==eIntNum)
    {
        u32int = 1;
    }
    eIrqstst = MHalVpeSclCheckIrq(u32EnISR[u32int]) ;

    if (eIrqstst != E_MHAL_SCL_IRQ_NONE)
    {
        MHalVpeSclClearIrq(u32EnISR[u32int]);
    }
    if(eIrqstst & E_MHAL_SCL_IRQ_FRAMEDONE)
    {
        SCL_ERR( "[VPE]FRAMEDONE %d\n",u32int);
        //spin_lock_irq
        if(u32int)
        {
            if(gstUtM2MTaskCfg.bEn)
            {
                gstUtM2MTaskCfg.bSet = 1;
                DrvSclOsSetTaskWork(gstUtM2MTaskCfg.Id);
            }
        }
        else
        {
            if(gstUtDispTaskCfg.bEn)
            {
                gstUtDispTaskCfg.bSet = 1;
                DrvSclOsSetTaskWork(gstUtDispTaskCfg.Id);
            }
        }
    }
    return IRQ_HANDLED;
}
void _UTestEnableImi(bool bEn,u8 u8level)
{
    MHalVpeSclOutputIMIConfig_t stCfg;
    stCfg.bEn = bEn;
    stCfg.enFormat = E_MHAL_PIXEL_FRAME_YUV422_YUYV;
    stCfg.enOutPort = E_MHAL_SCL_OUTPUT_PORT0;
    MHalVpeSclImiPortConfig((void *)u32Ctx[u8level],&stCfg);
}
void _UTestEnableIntr(MHalVpeIrqEnableMode_e eMode)
{
    unsigned int  nIrqNum = 0 ;
    u32 u32int;
    MHalVpeSclGetIrqNum(&nIrqNum,eMode);
    SCL_ERR("[DRVSCLIRQ]nIrqNum:(%d)\n", nIrqNum);
    if(eMode & E_MHAL_SCL_IRQ_M2M_MODE || eMode & E_MHAL_SCL_IRQ_OFFM2M)
    {
        u32int = 1;
    }
    else
    {
        u32int = 0;
    }
    if(eMode&E_MHAL_SCL_IRQ_OFFM2M)
    {
        if(u32EnISR[u32int])
        {
            DrvSclOsDetachInterrupt(nIrqNum);
            DrvSclOsDisableInterrupt(nIrqNum);
            MHalVpeSclEnableIrq(eMode);
            u32EnISR[u32int] = 0;
        }
    }
    else if(eMode)
    {
        if(!u32EnISR[u32int])
        {
            if(DrvSclOsAttachInterrupt(nIrqNum, (InterruptCb)_UTCallback ,IRQF_DISABLED, "SCLINTR"))
            {
                SCL_ERR("[DRVSCLIRQ]%s(%d):: Request IRQ Fail\n", __FUNCTION__, __LINE__);
                return ;
            }
            else
            {
                u32EnISR[u32int] = eMode;
            }
            DrvSclOsDisableInterrupt(nIrqNum);
            DrvSclOsEnableInterrupt(nIrqNum);
            MHalVpeSclEnableIrq(eMode);
        }
    }
    else
    {
        if(u32EnISR[u32int])
        {
            DrvSclOsDetachInterrupt(nIrqNum);
            DrvSclOsDisableInterrupt(nIrqNum);
            MHalVpeSclEnableIrq(eMode);
            u32EnISR[u32int] = 0;
        }
    }
}
void UTest_Case0(void)
{
    _UTest_Destroy_Task();
    _UTestEnableIntr(E_MHAL_SCL_IRQ_OFF);
    _UTest_CleanInst();
    SCL_ERR( "[VPE]Set disable all test\n");
}
void M2MUTest_Case0(void)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_OFFM2M);
    _UTestEnableIntr(E_MHAL_SCL_IRQ_OFF);
    _UTest_Destroy_Task();
    _UTest_CleanInst();
    SCL_ERR( "[VPE]Set disable all test\n");
}
void M2MUTest_Case1(void)
{
    int i;
    int idx;
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        idx = _UTest_GetM2MEmptyIdx();
        _UTest_CreateSclSubInstance(idx,MAXOUTPUTportH,MAXOUTPUTportV,E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME);
    }
    //_UTest_AllocateMem(idx,MAXINPUTWIDTH*MAXINPUTHEIGHT*2);
    SCL_ERR( "[VPE]Set create 1 inst Debug level on %s\n",sg_scl_yc_name[idx]);
}
void M2MUTest_Case2(u8 u8level)
{
    if(IdM2MNum[u8level])
    {
        _UTest_SetInputCfgM2MMode(u8level, gu16M2MInwsize, gu16M2MInhsize);
    }
    SCL_ERR( "[VPE]Set Set all input/crop\n");
}
void M2MUTest_Case3(u8 u8level)
{
    int i;
    MHalPixelFormat_e enType;
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        if(u8level <= 1)
        {
            if(IdM2MNum[i])
            {
                enType = (u8level == 1) ? E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420: E_MHAL_PIXEL_FRAME_YUV422_YUYV;
                _UTest_SetM2MDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetM2MDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetM2MDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetM2MDmaCfg(i,enType,E_MHAL_SCL_OUTPUT_PORT3);
            }
        }
        if(u8level > 1)
        {
            if(u32M2MCtx[i])
            {
                stM2MInputpCfg[i].ePixelFormat = (u8level == 2) ? E_MHAL_PIXEL_FRAME_YUV422_YUYV :
                                                 (u8level == 3) ? E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420 :
                                                 E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422;
                MHalVpeSclInputConfig((void *)u32M2MCtx[i], &stM2MInputpCfg[i]);
            }
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma config\n");
}
void M2MUTest_Case4(u8 u8level)
{
    int i;
    _UTest_SetLevelSize(u8level,&gu16M2Mwsize,&gu16M2Mhsize);
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        if(IdM2MNum[i])
        {
            _UTest_SetM2MOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT0,gu16M2Mwsize, gu16M2Mhsize);
            _UTest_SetM2MOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT1,gu16M2Mwsize, gu16M2Mhsize);
            _UTest_SetM2MOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT2,gu16M2Mwsize, gu16M2Mhsize);
            _UTest_SetM2MOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT3,gu16M2Mwsize, gu16M2Mhsize);
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma output size %hu/%hu\n",gu16swsize,gu16shsize);
}
void M2MUTest_Case5(u32 u32Time)
{
    int i;
    for(i = 0;i<M2MTEST_INST_MAX;i++)
    {
        if(IdM2MNum[i])
        {
            _UTest_SetM2MProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT0,sg_m2m_yc_addr[i],
                (u8gM2Mlevel[i] ==0 || u8gM2Mlevel[i] ==4 || u8gM2Mlevel[i] ==5 || u8gM2Mlevel[i] ==7|| u8gM2Mlevel[i] ==8) ? 1 :0,0);
            _UTest_SetM2MProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT1,sg_m2m_yc_addr[i],
                (u8gM2Mlevel[i] ==1 || u8gM2Mlevel[i] ==4 || u8gM2Mlevel[i] ==6 || u8gM2Mlevel[i] ==7) ? 1 :0,0);
            _UTest_SetM2MProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT2,sg_m2m_yc_addr[i],
                (u8gM2Mlevel[i] ==2 || u8gM2Mlevel[i] ==5 || u8gM2Mlevel[i] ==6 || u8gM2Mlevel[i] ==7) ? 1 :0,0);
            _UTest_SetM2MProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT3,sg_m2m_yc_addr[i],
            (u8gM2Mlevel[i] ==3 || u8gM2Mlevel[i] ==8),stM2MOutputCfg[i].u16Width*2);
            MHalVpeSclProcess((void *)u32M2MCtx[i],pM2MCmdqCfg, &stM2MpBuffer[i]);
            MHalVpeSclSetWaitDone((void *)u32M2MCtx[i],pM2MCmdqCfg,_UTest_GetWaitFlag(u8gM2Mlevel[i]));
            if(pM2MCmdqCfg)
            {
                pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            }
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma Process\n");
}
void M2MUTest_Case6(u8 u8level,u32 u32Time)
{
    if(IdM2MNum[u8level])
    {
        _UTest_SetM2MProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT0,sg_m2m_yc_addr[u8level],
        (u8gM2Mlevel[u8level] ==0 || u8gM2Mlevel[u8level] ==4 || u8gM2Mlevel[u8level] ==5 || u8gM2Mlevel[u8level] ==7|| u8gM2Mlevel[u8level] ==8) ? 1 :0,0);
        _UTest_SetM2MProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT1,sg_m2m_yc_addr[u8level],
        (u8gM2Mlevel[u8level] ==1 || u8gM2Mlevel[u8level] ==4 || u8gM2Mlevel[u8level] ==6 || u8gM2Mlevel[u8level] ==7) ? 1 :0,0);
        _UTest_SetM2MProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT2,sg_m2m_yc_addr[u8level],
        (u8gM2Mlevel[u8level] ==2 || u8gM2Mlevel[u8level] ==5 || u8gM2Mlevel[u8level] ==6 || u8gM2Mlevel[u8level] ==7) ? 1 :0,0);
        _UTest_SetM2MProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT3,sg_m2m_yc_addr[u8level],
        (u8gM2Mlevel[u8level] ==3 || u8gM2Mlevel[u8level] ==8),stM2MOutputCfg[u8level].u16Width*2);
        MHalVpeSclProcess((void *)u32M2MCtx[u8level],pM2MCmdqCfg, &stM2MpBuffer[u8level]);
        MHalVpeSclSetWaitDone((void *)u32M2MCtx[u8level],pM2MCmdqCfg,_UTest_GetWaitFlag(u8gM2Mlevel[u8level]));
    }
    SCL_ERR( "[VPE]Set Set for level:%hhu Dma Process\n",u8level);
}
void M2MUTest_Case7(u8 u8level)
{
    _UTest_SetLevelSize(u8level,&gu16M2Mwsize,&gu16M2Mhsize);
    SCL_ERR("[VPE]Set M2M OutPutSize:(%hu,%hu) \n",gu16M2Mwsize,gu16M2Mhsize);

}
void M2MUTest_Case8(u8 u8level)
{
    _UTest_SetLevelInSize(u8level,&gu16M2MInwsize,&gu16M2MInhsize);
    SCL_ERR("[VPE]Set M2M InPutSize:(%hu,%hu) \n",gu16M2MInwsize,gu16M2MInhsize);
}
void M2MUTest_CaseI(u8 u8level)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_PORT2_M2M);
}
void M2MUTest_CaseJ(u8 u8level)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_CAM_FRAMEBUF_3PORT);
}
void M2MUTest_CaseP(u8 u8level)
{
    gu8M2MCount = u8level;
}
void M2MUTest_Casei(u8 u8level)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_OFFM2M);
}
void _UTest_SetSubCfg(u32 M2MInaddr,u32 M2MOutaddr,int M2Midx)
{
    u16 wsize;
    u16 hsize;
    if(M2Midx%4 == 0)
    {
        wsize = gu16M2Mwsize;
        hsize = gu16M2Mhsize;
    }
    else if(M2Midx%4 == 1)
    {
        wsize = gu16M2Mwsize;
        hsize = gu16M2Mhsize/2;
    }
    else if(M2Midx%4 == 2)
    {
        wsize = gu16M2Mwsize/2;
        hsize = gu16M2Mhsize;
    }
    else
    {
        wsize = gu16M2Mwsize/2;
        hsize = gu16M2Mhsize/2;
    }
    if(sg_m2m_yc_viraddr[M2Midx])
    {
        _UTest_SetInputCfgM2MMode(M2Midx, gu16swsize, gu16shsize);
        _UTest_SetM2MinDmaCfg(M2Midx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422,E_MHAL_M2M_INPUT_PORT);
        _UTest_SetM2MDmaCfg(M2Midx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetM2MInputSizeCfg(M2Midx,E_MHAL_M2M_INPUT_PORT,gu16swsize, gu16shsize);
        _UTest_SetM2MOutputSizeCfg(M2Midx,E_MHAL_SCL_OUTPUT_PORT2,wsize, hsize);
        _UTest_SetM2MInProcessCfg(M2Midx,E_MHAL_M2M_INPUT_PORT,M2MInaddr,1,gu16swsize);
        _UTest_SetM2MProcessCfg(M2Midx,E_MHAL_SCL_OUTPUT_PORT2,M2MOutaddr,1,wsize*2);
    }
}
int _UTest_InitMainDev(void)
{
    int idx;
    idx = _UTest_GetEmptyIdx();
    _UTest_AllocateMem(idx,(u32)(gu16swsize*gu16shsize*2)*3);
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16PatInwsize,gu16PatInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_CAM_MODE,E_MHAL_SCL_OUTPUT_MODE_3PORT_2LEVEL_MODE);
    return idx;
}
int _UTest_InitSubDev(void)
{
    int idx;
    u32 memsize;
    idx = _UTest_GetM2MEmptyIdx();
    if(idx%4 == 0)
    {
        memsize = (gu16M2Mwsize*gu16M2Mhsize*2)*3;
    }
    else if(idx%4 == 1)
    {
        memsize = (gu16M2Mwsize*gu16M2Mhsize)*3;
    }
    else if(idx%4 == 2)
    {
        memsize = (gu16M2Mwsize*gu16M2Mhsize)*3;
    }
    else
    {
        memsize = (gu16M2Mwsize*gu16M2Mhsize/2)*3;
    }
    _UTest_AllocateM2MMem(idx,(u32)memsize);
    _UTest_CreateSclSubInstance
    (idx,gu16M2MInwsize,gu16M2MInhsize,E_MHAL_SCL_SUB_INPUT_MODE_RSC_TIME);
    return idx;
}
void _UTest_SetMainCfg(u32 Outaddr,int idx)
{
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgPatMode(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetCropCfg(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,Outaddr,1,gu16swsize);
    }
}
void UT_M2MProcessTaskCmdq(void *arg)
{
    u16 u16fence = 0;
    u16 u16orifence = 0;
    u16 count = 0;
    u16 u16count = 0;
    int i,j,k;
    u32 M2Maddr[4][3];
    u32* M2MViraddr[4][3];
    int M2Midx[4];
    int dev[4];
    u16 u16M2Mfence;
    u16 u16M2Mfence2 = 0;
    struct file *writefplocal = NULL;
    while(gstUtDispTaskCfg.bEn)
    {
        for(j=0;j<gstUtDispTaskCfg.u8level;j++)
        {
            M2Midx[j] = _UTest_InitSubDev();
            dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
        }
        for(i = 0;i<3;i++)
        {
            for(j=0;j<gstUtDispTaskCfg.u8level;j++)
            {
                M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
                M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
            }
        }
        while(gstUtDispTaskCfg.bEn)
        {
            SCL_ERR( "[VPE]UT M2MTask\n");
            do
            {
                //DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                if(u16count > 2000 || (!u32EnISR[0]&&!u32EnISR[1]))
                {
                    SCL_ERR( "[VPE]UT M2Msleep\n");
                    break;
                }
                u16count++;
            }
            while(u16fence==u16orifence);
            u16count = 0;
            count = u16fence - u16orifence;
            k= u16orifence %3;
            u16orifence = u16fence;
            for(i=0;i<count;i++)
            {
                for(j=0;j<gstUtDispTaskCfg.u8level;j++)
                {
                    _UTest_SetSubCfg(gstUtDispTaskCfg.u32buffer[k],M2Maddr[j][k],M2Midx[j]);
                    MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg, &stM2MpBuffer[M2Midx[j]]);
                    MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                    if(pM2MCmdqCfg)
                    {
                        pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,gstUtDispTaskCfg.u8level*i+j+1);
                        pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
                    }
                }
                k++;
                if(k>2)
                {
                    k=0;
                }
            }
            if(pM2MCmdqCfg)
            {
                u16M2Mfence2 = 0;
                do
                {
                    //DrvSclOsDelayTask(1);
                    pM2MCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pM2MCmdqCfg,&u16M2Mfence);
                    if(u16M2Mfence && u16M2Mfence%gstUtDispTaskCfg.u8level==0 && u16M2Mfence != u16M2Mfence2 )
                    {
                        gstUtDispTaskCfg.u16fence+= ((u16M2Mfence-u16M2Mfence2)/gstUtDispTaskCfg.u8level);
                        u16M2Mfence2 = u16M2Mfence;
                        SCL_ERR("u16M2Mfence:%hx u16fence:%hx\n",u16M2Mfence,gstUtDispTaskCfg.u16fence);
                    }
                }
                while(u16M2Mfence!=(gstUtDispTaskCfg.u8level*(count-1)+j));
                u16M2Mfence = 0;
                pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,0);
                pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            }
            for(i=0;i<count;i++)
            {
                if(k==0)
                {
                    k=2;
                }
                else
                {
                    k--;
                }
            }
            for(i=0;i<count;i++)
            {
                for(j=0;j<gstUtDispTaskCfg.u8level;j++)
                {
                    if(j==0)
                    {
                        writefplocal = writefp;
                    }
                    else if (j==1)
                    {
                        writefplocal = writefp2;
                    }
                    else if (j==2)
                    {
                        writefplocal = writefp3;
                    }
                    else
                    {
                        writefplocal = writefp4;
                    }
                    _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][k],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
                }
                k++;
            }
            if(!gstUtDispTaskCfg.bSet || (!u32EnISR[0]&&!u32EnISR[1]))
            {
                DrvSclOsSleepTaskWork(gstUtDispTaskCfg.Id);
            }
            //spin_lock_irq
            gstUtDispTaskCfg.bSet = 0;
        }
    }
}
void UT_M2MProcessTask(void *arg)
{
    u16 u16fence = 0;
    u16 u16orifence = 0;
    u16 count = 0;
    u16 u16count = 0;
    int i,j,k;
    u32 M2Maddr[4][3];
    u32* M2MViraddr[4][3];
    int M2Midx[4];
    int dev[4];
    struct file *writefplocal = NULL;
    while(gstUtDispTaskCfg.bEn)
    {
        for(j=0;j<gstUtDispTaskCfg.u8level;j++)
        {
            M2Midx[j] = _UTest_InitSubDev();
            dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
        }
        for(i = 0;i<3;i++)
        {
            for(j=0;j<gstUtDispTaskCfg.u8level;j++)
            {
                M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
                M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
            }
        }
        while(gstUtDispTaskCfg.bEn)
        {
            SCL_ERR( "[VPE]UT M2MTask\n");
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                if(u16count > 2000 || (!u32EnISR[0]&&!u32EnISR[1]))
                {
                    SCL_ERR( "[VPE]UT M2Msleep\n");
                    break;
                }
                u16count++;
            }
            while(u16fence==u16orifence);
            u16count = 0;
            count = u16fence - u16orifence;
            k= u16orifence %3;
            u16orifence = u16fence;
            for(i=0;i<count;i++)
            {
                for(j=0;j<gstUtDispTaskCfg.u8level;j++)
                {
                    _UTest_SetSubCfg(gstUtDispTaskCfg.u32buffer[k],M2Maddr[j][k],M2Midx[j]);
                    if(j==0)
                    {
                        writefplocal = writefp;
                    }
                    else if (j==1)
                    {
                        writefplocal = writefp2;
                    }
                    else if (j==2)
                    {
                        writefplocal = writefp3;
                    }
                    else
                    {
                        writefplocal = writefp4;
                    }
                    MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],NULL, &stM2MpBuffer[M2Midx[j]]);
                    MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],NULL,E_MHAL_VPE_WAITDONE_DMAONLY);
                    _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][k],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
                }
                k++;
                if(k>2)
                {
                    k=0;
                }
                gstUtDispTaskCfg.u16fence++;
            }
            if(!gstUtDispTaskCfg.bSet || (!u32EnISR[0]&&!u32EnISR[1]))
            {
                DrvSclOsSleepTaskWork(gstUtDispTaskCfg.Id);
            }
            //spin_lock_irq
            gstUtDispTaskCfg.bSet = 0;
        }
    }
}
void M2MUTest_CaseA(u8 u8level)
{
    int idx;
    int i;
    int count;
    int M2Midx;
    u16 u16fence = 0;
    u32 M2Maddr[3];
    u32 Scladdr[3];
    u32* M2MViraddr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout2.bin",O_WRONLY|O_CREAT,0777);
    M2Midx = _UTest_InitSubDev();
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        M2Maddr[i] = sg_m2m_yc_addr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i;
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        M2MViraddr[i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i);
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<u8level;count++)
    {
        //add sub
        _UTest_SetSubCfg(Scladdr[i],M2Maddr[i],M2Midx);
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,1,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        _UTest_WriteFile(writefp,(char *)SclViraddr[i],gu16swsize*gu16shsize*2);
        MHalVpeSclProcess((void *)u32M2MCtx[M2Midx],NULL, &stM2MpBuffer[M2Midx]);
        MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx],NULL,E_MHAL_VPE_WAITDONE_DMAONLY);
        _UTest_WriteFile(writefp2,(char *)M2MViraddr[i],gu16M2Mwsize*gu16M2Mhsize*2);
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Casea(u8 u8level)
{
    int idx;
    int i;
    int count;
    int M2Midx;
    u16 u16fence = 0;
    u16 u16M2Mfence = 0;
    u32 M2Maddr[3];
    u32 Scladdr[3];
    u32* M2MViraddr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp2 = _UTest_OpenFile("/tmp/Scout2.bin",O_WRONLY|O_CREAT,0777);
    M2Midx = _UTest_InitSubDev();
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        M2Maddr[i] = sg_m2m_yc_addr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i;
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        M2MViraddr[i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i);
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<u8level;count++)
    {
        //add sub
        _UTest_SetSubCfg(Scladdr[i],M2Maddr[i],M2Midx);
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        MHalVpeSclProcess((void *)u32M2MCtx[M2Midx],pM2MCmdqCfg, &stM2MpBuffer[M2Midx]);
        MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx],pM2MCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pM2MCmdqCfg)
        {
            pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,count+1);
            pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pM2MCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pM2MCmdqCfg,&u16M2Mfence);
            }
            while(u16M2Mfence!=(count+1));
        }
        _UTest_WriteFile(writefp2,(char *)M2MViraddr[i],gu16M2Mwsize*gu16M2Mhsize*2);
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Caseb(u8 u8level)
{
    int idx;
    int i,j;
    int dev[4];
    int count;
    int M2Midx[4];
    u16 u16fence = 0;
    u16 u16M2Mfence = 0;
    u32 M2Maddr[4][3];
    u32 Scladdr[3];
    u32* M2MViraddr[4][3];
    u32* SclViraddr[3];
    struct file *writefplocal = NULL;
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    for(j=0;j<u8level;j++)
    {
        M2Midx[j] = _UTest_InitSubDev();
        dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
    }
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        for(j=0;j<u8level;j++)
        {
            M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
            M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
        }
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        //add sub
        for(j=0;j<u8level;j++)
        {
            _UTest_SetSubCfg(Scladdr[i],M2Maddr[j][i],M2Midx[j]);
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        for(j=0;j<u8level;j++)
        {
            MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg, &stM2MpBuffer[M2Midx[j]]);
            MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
            if(pM2MCmdqCfg)
            {
                pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,u8level*count+j+1);
                pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            }
        }
        if(pM2MCmdqCfg)
        {
            do
            {
                DrvSclOsDelayTask(1);
                pM2MCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pM2MCmdqCfg,&u16M2Mfence);
            }
            while(u16M2Mfence!=(u8level*count+j));
        }
        for(j=0;j<u8level;j++)
        {
            if(j==0)
            {
                writefplocal = writefp;
            }
            else if (j==1)
            {
                writefplocal = writefp2;
            }
            else if (j==2)
            {
                writefplocal = writefp3;
            }
            else
            {
                writefplocal = writefp4;
            }
            _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][i],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_CaseB(u8 u8level)
{
    int idx;
    int i,j;
    int dev[4];
    int count;
    int M2Midx[4];
    u16 u16fence = 0;
    u32 M2Maddr[4][3];
    u32 Scladdr[3];
    u32* M2MViraddr[4][3];
    u32* SclViraddr[3];
    struct file *writefplocal = NULL;
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    for(j=0;j<u8level;j++)
    {
        M2Midx[j] = _UTest_InitSubDev();
        dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
    }
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        for(j=0;j<u8level;j++)
        {
            M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
            M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
        }
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        //add sub
        for(j=0;j<u8level;j++)
        {
            _UTest_SetSubCfg(Scladdr[i],M2Maddr[j][i],M2Midx[j]);
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,1,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152502,0,0x1);
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        for(j=0;j<u8level;j++)
        {
            if(j==0)
            {
                writefplocal = writefp;
            }
            else if (j==1)
            {
                writefplocal = writefp2;
            }
            else if (j==2)
            {
                writefplocal = writefp3;
            }
            else
            {
                writefplocal = writefp4;
            }
            MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],NULL, &stM2MpBuffer[M2Midx[j]]);
            MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],NULL,E_MHAL_VPE_WAITDONE_DMAONLY);
            _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][i],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_CaseC(u8 u8level)
{
    int idx;
    int i;
    int count;
    int M2Midx;
    u16 u16fence = 0;
    u32 M2Maddr[3];
    u32 Scladdr[3];
    u32* M2MViraddr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    M2Midx = _UTest_InitSubDev();
    idx = _UTest_InitMainDev();
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout2.bin",O_WRONLY|O_CREAT,0777);
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    for(i = 0;i<3;i++)
    {
        M2Maddr[i] = sg_m2m_yc_addr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i;
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        M2MViraddr[i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i);
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<u8level;count++)
    {
        if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        //add sub
        _UTest_SetSubCfg(Scladdr[i],M2Maddr[i],M2Midx);
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        _UTest_WriteFile(writefp,(char *)SclViraddr[i],gu16swsize*gu16shsize*2);
        MHalVpeSclProcess((void *)u32M2MCtx[M2Midx],NULL, &stM2MpBuffer[M2Midx]);
        MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx],NULL,E_MHAL_VPE_WAITDONE_DMAONLY);
        _UTest_WriteFile(writefp2,(char *)M2MViraddr[i],gu16M2Mwsize*gu16M2Mhsize*2);
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Casec(u8 u8level)
{
    int idx;
    int i;
    int count;
    int M2Midx;
    u16 u16fence = 0;
    u16 u16M2Mfence = 0;
    u32 M2Maddr[3];
    u32 Scladdr[3];
    u32* M2MViraddr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp2 = _UTest_OpenFile("/tmp/Scout2.bin",O_WRONLY|O_CREAT,0777);
    M2Midx = _UTest_InitSubDev();
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    idx = _UTest_InitMainDev();
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    for(i = 0;i<3;i++)
    {
        M2Maddr[i] = sg_m2m_yc_addr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i;
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        M2MViraddr[i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx] + (gu16M2Mwsize*gu16M2Mhsize*2)*i);
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<u8level;count++)
    {
        if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        //add sub
        _UTest_SetSubCfg(Scladdr[i],M2Maddr[i],M2Midx);
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        MHalVpeSclProcess((void *)u32M2MCtx[M2Midx],pM2MCmdqCfg, &stM2MpBuffer[M2Midx]);
        MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx],pM2MCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pM2MCmdqCfg)
        {
            pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,count+1);
            pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pM2MCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pM2MCmdqCfg,&u16M2Mfence);
            }
            while(u16M2Mfence!=(count+1));
        }
        _UTest_WriteFile(writefp2,(char *)M2MViraddr[i],gu16M2Mwsize*gu16M2Mhsize*2);
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_CaseD(u8 u8level)
{
    int idx;
    int i,j;
    int dev[4];
    int count;
    int M2Midx[4];
    u16 u16fence = 0;
    u32 M2Maddr[4][3];
    u32 Scladdr[3];
    u32* M2MViraddr[4][3];
    u32* SclViraddr[3];
    struct file *writefplocal = NULL;
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    for(j=0;j<u8level;j++)
    {
        M2Midx[j] = _UTest_InitSubDev();
        dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
    }
    idx = _UTest_InitMainDev();
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    for(i = 0;i<3;i++)
    {
        for(j=0;j<u8level;j++)
        {
            M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
            M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
        }
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        //add sub
        for(j=0;j<u8level;j++)
        {
            _UTest_SetSubCfg(Scladdr[i],M2Maddr[j][i],M2Midx[j]);
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        for(j=0;j<u8level;j++)
        {
            if(j==0)
            {
                writefplocal = writefp;
            }
            else if (j==1)
            {
                writefplocal = writefp2;
            }
            else if (j==2)
            {
                writefplocal = writefp3;
            }
            else
            {
                writefplocal = writefp4;
            }
            MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],NULL, &stM2MpBuffer[M2Midx[j]]);
            MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],NULL,E_MHAL_VPE_WAITDONE_DMAONLY);
            _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][i],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Cased(u8 u8level)
{
    int idx;
    int i,j;
    int dev[4];
    int count;
    int M2Midx[4];
    u16 u16fence = 0;
    u16 u16M2Mfence = 0;
    u32 M2Maddr[4][3];
    u32 Scladdr[3];
    u32* M2MViraddr[4][3];
    u32* SclViraddr[3];
    struct file *writefplocal = NULL;
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    for(j=0;j<u8level;j++)
    {
        M2Midx[j] = _UTest_InitSubDev();
        dev[j] = (M2Midx[j]%4==0) ? 1 : (M2Midx[j]%4==1) ? 2 : (M2Midx[j]%4==2) ? 2 : 4 ;
    }
    idx = _UTest_InitMainDev();
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    for(i = 0;i<3;i++)
    {
        for(j=0;j<u8level;j++)
        {
            M2Maddr[j][i] = sg_m2m_yc_addr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i;
            M2MViraddr[j][i] = (u32*)((u32)sg_m2m_yc_viraddr[M2Midx[j]] + (gu16M2Mwsize*gu16M2Mhsize*2)/dev[j]*i);
        }
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
    }
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        //add sub
        for(j=0;j<u8level;j++)
        {
            _UTest_SetSubCfg(Scladdr[i],M2Maddr[j][i],M2Midx[j]);
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(u16fence!=(count+1));
        }
        for(j=0;j<u8level;j++)
        {
            MHalVpeSclProcess((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg, &stM2MpBuffer[M2Midx[j]]);
            MHalVpeSclSetWaitDone((void *)u32M2MCtx[M2Midx[j]],pM2MCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
            if(pM2MCmdqCfg)
            {
                pM2MCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pM2MCmdqCfg,u8level*count+j+1);
                pM2MCmdqCfg->MHAL_CMDQ_KickOffCmdq(pM2MCmdqCfg);
            }
        }
        if(pM2MCmdqCfg)
        {
            do
            {
                DrvSclOsDelayTask(1);
                pM2MCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pM2MCmdqCfg,&u16M2Mfence);
            }
            while(u16M2Mfence!=(u8level*count+j));
        }
        for(j=0;j<u8level;j++)
        {
            if(j==0)
            {
                writefplocal = writefp;
            }
            else if (j==1)
            {
                writefplocal = writefp2;
            }
            else if (j==2)
            {
                writefplocal = writefp3;
            }
            else
            {
                writefplocal = writefp4;
            }
            _UTest_WriteFile(writefplocal,(char *)M2MViraddr[j][i],gu16M2Mwsize*gu16M2Mhsize*2/dev[j]);
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_CaseE(u8 u8level)
{
    int idx;
    int i;
    int count;
    u16 u16fence = 0;
    u32 Scladdr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
        gstUtDispTaskCfg.u32buffer[i] = Scladdr[i];
    }
    _UTest_Create_Task(u8level,(u32)(gu16M2MInwsize*gu16M2MInwsize*2),(TaskEntry)UT_M2MProcessTask);
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            if(count==gu8M2MCount -1)
            {
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            }
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            while(1)
            {
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                //if no buffer
                if(count+1!=gstUtDispTaskCfg.u16fence+3)
                {
                    break;
                }
            }
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Casee(u8 u8level)
{
    int idx;
    int i;
    int count;
    u16 u16fence = 0;
    u32 Scladdr[3];
    u32* SclViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    idx = _UTest_InitMainDev();
    for(i = 0;i<3;i++)
    {
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
        gstUtDispTaskCfg.u32buffer[i] = Scladdr[i];
    }
    _UTest_Create_Task(u8level,(u32)(gu16M2MInwsize*gu16M2MInwsize*2),(TaskEntry)UT_M2MProcessTaskCmdq);
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            if(count==gu8M2MCount -1)
            {
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x152580,0,0x8000);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1525E0,0,0x1);
            }
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            while(1)
            {
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                //if no buffer
                if(count+1!=gstUtDispTaskCfg.u16fence+3)
                {
                    break;
                }
            }
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_CaseF(u8 u8level)
{
    int idx;
    int i;
    int count;
    u16 u16fence = 0;
    u32 Scladdr[3];
    u32* SclViraddr[3];
    u32 Ispaddr[3];
    u32* IspViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    idx = _UTest_InitMainDev();
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*9/2);
    for(i = 0;i<3;i++)
    {
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
        Ispaddr[i] = sg_Isp_yc_addr[idx] + (gu16IspInwsize*gu16IspInhsize*3/2)*i;
        IspViraddr[i] = (u32*)((u32)sg_Isp_yc_viraddr[idx] + (gu16IspInwsize*gu16IspInhsize*3/2)*i);
        gstUtDispTaskCfg.u32buffer[i] = Scladdr[i];
    }
    _UTest_Create_Task(u8level,(u32)(gu16M2MInwsize*gu16M2MInwsize*2),(TaskEntry)UT_M2MProcessTask);
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        if(!_UTest_PutFile2BufferAddr("/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2),IspViraddr[i]))
        {
            return ;
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        stVdinfo[idx].u32Stride[0] = gu16IspInwsize;
        stVdinfo[idx].u32Stride[1] = gu16IspInwsize;
        stVdinfo[idx].u64PhyAddr[0] = Ispaddr[i];
        stVdinfo[idx].u64PhyAddr[1] = Ispaddr[i]+((u32)(gu16IspInwsize*gu16IspInhsize));
        MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            while(1)
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                //if no buffer
                if(count+1!=gstUtDispTaskCfg.u16fence+3)
                {
                    break;
                }
            }
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void M2MUTest_Casef(u8 u8level)
{
    int idx;
    int i;
    int count;
    u16 u16fence = 0;
    u32 Scladdr[3];
    u32* SclViraddr[3];
    u32 Ispaddr[3];
    u32* IspViraddr[3];
    gu16swsize = gu16M2MInwsize;
    gu16shsize = gu16M2MInhsize;
    // add sub pipe
    if(pM2MCmdqCfg==NULL)
    {
        pM2MCmdqCfg = _UTest_AllocateCMDQM2M();
    }
    writefp = _UTest_OpenFile("/tmp/Scout_M2mch0.bin",O_WRONLY|O_CREAT,0777);
    writefp2 = _UTest_OpenFile("/tmp/Scout_M2mch1.bin",O_WRONLY|O_CREAT,0777);
    if(u8level<2)
    {
        u8level = 2;
    }
    if(u8level>2)
    {
        writefp3 = _UTest_OpenFile("/tmp/Scout_M2mch2.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>3)
    {
        writefp4 = _UTest_OpenFile("/tmp/Scout_M2mch3.bin",O_WRONLY|O_CREAT,0777);
    }
    if(u8level>4)
    {
        if(writefp)
        {
            _UTest_CloseFile(writefp);
            writefp = NULL;
        }
        if(writefp2)
        {
            _UTest_CloseFile(writefp2);
            writefp2 = NULL;
        }
        if(writefp3)
        {
            _UTest_CloseFile(writefp3);
            writefp3 = NULL;
        }
        if(writefp4)
        {
            _UTest_CloseFile(writefp4);
            writefp4 = NULL;
        }
        return ;
    }
    idx = _UTest_InitMainDev();
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*9/2);
    for(i = 0;i<3;i++)
    {
        Scladdr[i] = sg_scl_yc_addr[idx] + (gu16swsize*gu16shsize*2)*i;
        SclViraddr[i] = (u32*)((u32)sg_scl_yc_viraddr[idx] + (gu16swsize*gu16shsize*2)*i);
        Ispaddr[i] = sg_Isp_yc_addr[idx] + (gu16IspInwsize*gu16IspInhsize*3/2)*i;
        IspViraddr[i] = (u32*)((u32)sg_Isp_yc_viraddr[idx] + (gu16IspInwsize*gu16IspInhsize*3/2)*i);
        gstUtDispTaskCfg.u32buffer[i] = Scladdr[i];
    }
    _UTest_Create_Task(u8level,(u32)(gu16M2MInwsize*gu16M2MInwsize*2),(TaskEntry)UT_M2MProcessTaskCmdq);
    i =0;
    for(count = 0;count<gu8M2MCount;count++)
    {
        if(!_UTest_PutFile2BufferAddr("/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2),IspViraddr[i]))
        {
            return ;
        }
        // add main pipe
        _UTest_SetMainCfg(Scladdr[i],idx);
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        u8glevel[idx] = 0;
        stVdinfo[idx].u32Stride[0] = gu16IspInwsize;
        stVdinfo[idx].u32Stride[1] = gu16IspInwsize;
        stVdinfo[idx].u64PhyAddr[0] = Ispaddr[i];
        stVdinfo[idx].u64PhyAddr[1] = Ispaddr[i]+((u32)(gu16IspInwsize*gu16IspInhsize));
        MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
        if(pCmdqCfg)
        {
            pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,count+1);
            pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            while(1)
            {
                SCL_DBGERR("u16fence:%hx u16fence:%hx\n",u16fence,gstUtDispTaskCfg.u16fence);
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                //if no buffer
                if(count+1!=gstUtDispTaskCfg.u16fence+3)
                {
                    break;
                }
            }
        }
        i++;
        if(i>2)
        {
            i =0;
        }
    }
}
void UTest_Case1(void)
{
    int i;
    int idx;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        idx = _UTest_GetEmptyIdx();
        _UTest_CreateSclInstance
        (idx,MAXOUTPUTportH,MAXOUTPUTportV,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    }
    //_UTest_AllocateMem(idx,MAXINPUTWIDTH*MAXINPUTHEIGHT*2);
    SCL_ERR( "[VPE]Set create 1 inst Debug level on %s\n",sg_scl_yc_name[idx]);
}
void UTest_Case2(void)
{
    int i;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdNum[i])
        {
            _UTest_SetInputCfgPatMode(i, gu16PatInwsize, gu16PatInhsize);
            _UTest_SetCropCfg(i, gu16PatInwsize, gu16PatInhsize);
        }
    }
    SCL_ERR( "[VPE]Set Set all input/crop\n");
}
void UTest_Case3(u8 u8level)
{
    int i;
    MHalPixelFormat_e enType;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(u8level <= 1)
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
        if(u8level > 1)
        {
            if(u32IspCtx[i])
            {
                stIspInputCfg[i].enInType = (u8level == 2) ? E_MHAL_ISP_INPUT_YUV422 :
                                            (u8level == 3) ?E_MHAL_ISP_INPUT_YUV420 : E_MHAL_ISP_INPUT_YUV420;
                stIspInputCfg[i].ePixelFormat = (u8level == 2) ? E_MHAL_PIXEL_FRAME_YUV422_YUYV :
                                            (u8level == 3) ? E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420 : E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                MHalVpeIspInputConfig((void *)u32IspCtx[i], &stIspInputCfg[i]);
            }
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma config\n");
}
void UTest_Case4(u8 u8level)
{
    int i;
    _UTest_SetLevelSize(u8level,&gu16swsize,&gu16shsize);
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdNum[i])
        {
            _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(i,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma output size %hu/%hu\n",gu16swsize,gu16shsize);
}
void UTest_Case5(u32 u32Time)
{
    int i;
    for(i = 0;i<TEST_INST_MAX;i++)
    {
        if(IdNum[i])
        {
            _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[i],
                (u8glevel[i] ==0 || u8glevel[i] ==4 || u8glevel[i] ==5 || u8glevel[i] ==7|| u8glevel[i] ==8) ? 1 :0,0);
            _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[i],
                (u8glevel[i] ==1 || u8glevel[i] ==4 || u8glevel[i] ==6 || u8glevel[i] ==7) ? 1 :0,0);
            _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[i],
                (u8glevel[i] ==2 || u8glevel[i] ==5 || u8glevel[i] ==6 || u8glevel[i] ==7) ? 1 :0,0);
            _UTest_SetProcessCfg(i,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[i],
            (u8glevel[i] ==3 || u8glevel[i] ==8),stOutputCfg[i].u16Width*2);
            MHalVpeSclProcess((void *)u32Ctx[i],pCmdqCfg, &stpBuffer[i]);
            MHalVpeSclSetWaitDone((void *)u32Ctx[i],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[i]));
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Test5_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                stOutputCfg[i].u16Width,stOutputCfg[i].u16Height,i,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[i]+MIU0_BASE,(u32)(stOutputCfg[i].u16Width*stOutputCfg[i].u16Height*2));
            if(pCmdqCfg)
            {
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            }
        }
    }
    SCL_ERR( "[VPE]Set Set all Dma Process\n");
}
void UT_RealTimeTask(void *arg)
{
    while(gstUtDispTaskCfg.bEn)
    {
        SCL_ERR( "[VPE]UT RealTimeTask\n");
        if(!gstUtDispTaskCfg.bSet)
        {
            DrvSclOsSleepTaskWork(gstUtDispTaskCfg.Id);
        }
        DrvSclOsDelayTask(1000);
        //spin_lock_irq
        gstUtDispTaskCfg.bSet = 0;
        UTest_Case5(((u32)DrvSclOsGetSystemTimeStamp()));
    }
}
void UTest_Case6(u8 u8level,u32 u32Time)
{
    if(IdNum[u8level])
    {
        _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[u8level],(u8level%3 ==2) ? 1 :0,0);
        _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[u8level],(u8level%3 ==1) ? 1 :0,0);
        _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[u8level],(u8level%3 ==0) ? 1 :0,0);
        _UTest_SetProcessCfg(u8level,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[u8level],0,stOutputCfg[u8level].u16Width*2);
        MHalVpeSclProcess((void *)u32Ctx[u8level],pCmdqCfg, &stpBuffer[u8level]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[u8level],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[u8level]));
        SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Test6_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
            stOutputCfg[u8level].u16Width,stOutputCfg[u8level].u16Height,u8level,(u16)(u32Time&0xFFFF)
            ,sg_scl_yc_addr[u8level]+MIU0_BASE,(u32)(stOutputCfg[u8level].u16Width*stOutputCfg[u8level].u16Height*2));
    }
    SCL_ERR( "[VPE]Set Set for level:%hhu Dma Process\n",u8level);
}
void UTest_Case7(u8 u8level)
{
    _UTest_SetLevelSize(u8level,&gu16swsize,&gu16shsize);
    SCL_ERR("[VPE]Set OutPutSize:(%hu,%hu) \n",gu16swsize,gu16shsize);

}
void UTest_Case8(u8 u8level)
{
    _UTest_SetLevelInSize(u8level,&gu16PatInwsize,&gu16PatInhsize);
    SCL_ERR("[VPE]Set Pat InPutSize:(%hu,%hu) \n",gu16PatInwsize,gu16PatInhsize);
}
void UTest_Case9(u8 u8level)
{
    _UTest_SetLevelInSize(u8level,&gu16IspInwsize,&gu16IspInhsize);
    SCL_ERR("[VPE]Set ISP InPutSize:(%hu,%hu) \n",gu16IspInwsize,gu16IspInhsize);
}
void UTest_CaseA(u8 u8level,u32 u32Time)
{
    int idx;
    idx = _UTest_GetEmptyIdx();
    //MHAL_CMDQ_GetSysCmdqService()
    _UTest_CreateSclInstance
    (idx,gu16PatInwsize,gu16PatInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_AllocateMem(idx,(u32)(gu16swsize*gu16shsize*2));
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgPatMode(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetCropCfg(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8),gu16swsize*2);
        MHalVpeSclProcess((void *)u32Ctx[idx],NULL, &stpBuffer[idx]);
        u8glevel[idx] = u8level;
    }
    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],NULL,_UTest_GetWaitFlag(u8level));
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],gu16swsize*gu16shsize*2);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestA_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));

}
void UTest_Casea(u8 u8level,u32 u32Time)
{
    int idx;
    u32 u32CMDQIrq;
    idx = _UTest_GetEmptyIdx();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16PatInwsize,gu16PatInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_AllocateMem(idx,(u32)(gu16swsize*gu16shsize*2));
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgPatMode(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetCropCfg(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8),gu16swsize*2);
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
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],gu16swsize*gu16shsize*2);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testa_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));

}
void UTest_CaseB(u8 u8level,u32 u32Time)
{
    int idx;
    idx = _UTest_GetEmptyIdx();
    //MHAL_CMDQ_GetSysCmdqService()
    _UTest_CreateSclInstance
    (idx,gu16PatInwsize,gu16PatInhsize,E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_AllocateMem(idx,(u32)(gu16swsize*gu16shsize*2));
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    _UTest_Create_Task(u8level,(u32)(gu16PatInwsize*gu16PatInhsize*3/2),(TaskEntry)UT_RealTimeTask);
    _UTestEnableIntr(E_MHAL_SCL_IRQ_CAM_REALTIME_3PORT);
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgPatMode(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetCropCfg(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8),gu16swsize*2);
        MHalVpeSclProcess((void *)u32Ctx[idx],NULL, &stpBuffer[idx]);
        u8glevel[idx] = u8level;
    }
    MHalVpeSclSetWaitDone((void *)u32Ctx[idx],NULL,_UTest_GetWaitFlag(u8level));
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],gu16swsize*gu16shsize*2);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestB_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));
}
void UTest_Caseb(u8 u8level,u32 u32Time)
{
    int idx;
    u32 u32CMDQIrq;
    idx = _UTest_GetEmptyIdx();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16PatInwsize,gu16PatInhsize,E_MHAL_VPE_INPUT_MODE_REALTIME_CAM_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_AllocateMem(idx,(u32)(gu16swsize*gu16shsize*2));
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    _UTest_Create_Task(u8level,(u32)(gu16PatInwsize*gu16PatInhsize*3/2),(TaskEntry)UT_RealTimeTask);
    _UTestEnableIntr(E_MHAL_SCL_IRQ_CAM_REALTIME_3PORT);
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgPatMode(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetCropCfg(idx, gu16PatInwsize, gu16PatInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8),gu16swsize*2);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
        u8glevel[idx] = u8level;
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
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],gu16swsize*gu16shsize*2);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testb_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));

}
void UTest_CaseC(void)
{
    int idx;
    idx = _UTest_GetIqEmptyIdx();
    _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    _UTest_SetRoiCfg(idx,pCmdqCfg);
    MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
    MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
    MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
    MHalVpeIqGetWdrRoiHist((void *)u32IqCtx[idx], &stRoiReport[idx]);
    SCL_ERR("[VPE]stRoiReport %u %u %u %u\n",
        stRoiReport[idx].u32Y[0],stRoiReport[idx].u32Y[1],stRoiReport[idx].u32Y[2],stRoiReport[idx].u32Y[3]);
}
void UTest_Casec(u8 u8level,u32 u32Time)
{
    int idx;
    idx = _UTest_GetEmptyIdx();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIqNum[idx] = 1;
    MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
    _UTest_SetIqCfg(idx,SET_all);
    _UTest_SetIqOnOff(idx,SET_all);
    _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2);
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*2);
    if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*2)))
    {
        return ;
    }
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgISP422Mode(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        MHalVpeIqProcess((void *)u32IqCtx[idx],pCmdqCfg);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8) ? 1 :0,gu16swsize*2);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
        u8glevel[idx] = u8level;
    }
    pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestD_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));
}
void UTest_CaseD(u8 u8level,u32 u32Time)
{
    int idx;
    idx = _UTest_GetEmptyIdx();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIqNum[idx] = 1;
    MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
    _UTest_SetIqCfg(idx,SET_all);
    _UTest_SetIqOnOff(idx,SET_all);
    _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2);
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
    {
        return ;
    }
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        MHalVpeIqProcess((void *)u32IqCtx[idx],pCmdqCfg);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            (u8level ==3 || u8level ==8) ? 1 :0,gu16swsize*2);
        MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
        _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
        MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
        u8glevel[idx] = u8level;
    }
    pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestD_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2));
}
void UTest_Cased(u8 u8level,u32 u32Time)
{
    u32 u32CMDQIrq;
    u32 u32timepre,u32timepost;
    if(IdIspNum[u8level])
    {
        if(!_UTest_PutFile2Buffer(u8level,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[u8level]);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        MHalVpeSclProcess((void *)u32Ctx[u8level],pCmdqCfg, &stpBuffer[u8level]);
        MHalVpeIspProcess((void *)u32IspCtx[u8level],pCmdqCfg, &stVdinfo[u8level]);
        MHalVpeSclSetWaitDone((void *)u32Ctx[u8level],pCmdqCfg,_UTest_GetWaitFlag(u8glevel[u8level]));
        SCL_ERR( "[VPE]Set MHAL_CMDQ_KickOffCmdq\n");
        u32timepre = ((u32)DrvSclOsGetSystemTimeStamp());
        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
        if(pCmdqCfg)
        {
            do
            {
                DrvSclOsDelayTaskUs(100);
                pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
            }
            while(!(u32CMDQIrq&0x800));
        }
        u32timepost = ((u32)DrvSclOsGetSystemTimeStamp());
        SCL_ERR("[time]diff:%lu\n",u32timepost-u32timepre);
        SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testd_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
            stOutputCfg[u8level].u16Width,stOutputCfg[u8level].u16Height,u8level,(u16)(u32Time&0xFFFF)
            ,sg_scl_yc_addr[u8level]+MIU0_BASE,(u32)(stOutputCfg[u8level].u16Width*stOutputCfg[u8level].u16Height*2));
    }

}
void UTest_CaseE(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        MHalVpeIqRead3DNRTbl((void *)u32IqCtx[u8level]);
        SCL_ERR("[VPE]get DNR report \n");
    }

}
void UTest_Casee(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        MHalVpeIqGetWdrRoiHist((void *)u32IqCtx[u8level], &stRoiReport[u8level]);
        SCL_ERR("[VPE]stRoiReport %u %u %u %u\n",
            stRoiReport[u8level].u32Y[0],stRoiReport[u8level].u32Y[1],stRoiReport[u8level].u32Y[2],stRoiReport[u8level].u32Y[3]);
    }

}
void UTest_CaseI(u8 u8level)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_DVR_FRAMEBUF_4PORT);
}
void UTest_Casei(u8 u8level)
{
    _UTestEnableIntr(E_MHAL_SCL_IRQ_OFF);
}
void UTest_CaseJ(u8 u8level)
{
    if(u32Ctx[u8level])
    {
        _UTestEnableImi(1,u8level);
    }
}
void UTest_Casej(u8 u8level)
{
    if(u32Ctx[u8level])
    {
        _UTestEnableImi(0,u8level);
    }
}
void UTest_CaseF(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_DEF);
        _UTest_SetIqOnOff(u8level,SET_DEF);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config Default \n");
    }

}
void UTest_CaseG(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_EG);
        _UTest_SetIqOnOff(u8level,SET_EG);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config Edge \n");
    }

}
void UTest_CaseH(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_ES);
        _UTest_SetIqOnOff(u8level,SET_ES);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config Es \n");
    }
}
void IqUTest_CaseI(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_CON);
        _UTest_SetIqOnOff(u8level,SET_CON);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config contrast \n");
    }

}
void IqUTest_CaseJ(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_UV);
        _UTest_SetIqOnOff(u8level,SET_UV);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config uv \n");
    }

}
void UTest_CaseN(u8 u8level)
{
    if(IdIqNum[u8level])
    {
        _UTest_SetIqCfg(u8level,SET_NR);
        _UTest_SetIqOnOff(u8level,SET_NR);
        MHalVpeIqProcess((void *)u32IqCtx[u8level], pCmdqCfg);
        SCL_ERR("[VPE]Set Config NR \n");
    }
}
void UTest_CaseP(u8 u8level)
{
    gu8Count = u8level;
    SCL_ERR("[VPE]Set Write count:%hhu \n",gu8Count);
}
void UTest_Casep(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u32 u32CMDQIrq;
    idx = _UTest_GetEmptyIdx();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    _UTest_CreateSclInstance
    (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
    _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIspNum[idx] = 1;
    _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
    IdIqNum[idx] = 1;
    MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
    _UTest_SetIqCfg(idx,SET_NR);
    _UTest_SetIqOnOff(idx,SET_NR);
    _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2*3);
    _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    if(sg_scl_yc_viraddr[idx])
    {
        _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
        _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
        _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
            (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx]+(u32)gu16swsize*gu16shsize*2,
            (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx]+(u32)gu16swsize*gu16shsize*4,
            (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,gu16swsize*2);
        _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
            0,gu16swsize*2);
        u8glevel[idx] = u8level;
        for(i=0;i<gu8Count;i++)
        {
            if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
            {
                return ;
            }
            MHalVpeIqRead3DNRTbl((void *)u32IqCtx[idx]);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
            _UTest_SetISPProcess(idx,gu16IspInwsize,gu16IspInhsize);
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
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],(u32)gu16swsize*gu16shsize*2*3);
        }
        SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\Testp_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
            gu16swsize,gu16shsize,idx,(u16)(u32Time&0xFFFF)
            ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2*3));
    }
}
void UTest_CaseK(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u64 u64Time;
    u64 u64TimeDiff;
    u32 Ispaddr[5];
    u32 idxM[5];
    u32* IspViraddr[5];
    u16 u16fence = 0;
    u16 u16Fps = 30;
    u16 u16Channel = 4;
    u16 u16InputCount = 5;
    u16 u16Count ;
    u16Count = u16Fps * u16Channel;
    gu16IspInwsize = FHDOUTPUTWIDTH;
    gu16IspInhsize = FHDOUTPUTHEIGHT;
    gu16swsize = FHDOUTPUTWIDTH;
    gu16shsize = FHDOUTPUTHEIGHT;
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    for(i=0;i<u16Channel;i++)
    {
        idx = _UTest_GetEmptyIdx();
        idxM[i] = idx;
        _UTest_CreateSclInstance
        (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
        if(u8level==1)
        {
            _UTest_SetIqCfg(idx,SET_NR);
            _UTest_SetIqOnOff(idx,SET_NR);
        }
        else if(u8level==2)
        {
            _UTest_SetIqCfg(idx,SET_all);
            _UTest_SetIqOnOff(idx,SET_all);
        }
        else
        {
            _UTest_SetIqCfg(idx,SET_DEF);
            _UTest_SetIqOnOff(idx,SET_DEF);
        }
        _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2+gu16swsize*gu16shsize/2+352*288*2);
        if(sg_scl_yc_viraddr[idx])
        {
            _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_MST_420,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,352, 288);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize/2, gu16shsize/2);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],1,gu16swsize);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx]+(u32)gu16swsize*gu16shsize*5/2,
                1 ,352);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx]+(u32)gu16swsize*gu16shsize*2,
                1,gu16swsize/2);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                0,gu16swsize*2);
        }
    }
    _UTest_AllocateIspMem(0,(u32)gu16IspInwsize*gu16IspInhsize*3/2*5);
    for(i=0;i<u16InputCount;i++)
    {
        Ispaddr[i] = sg_Isp_yc_addr[0] + (gu16IspInwsize*gu16IspInhsize*3/2)*i;
        IspViraddr[i] = (u32*)((u32)sg_Isp_yc_viraddr[0] + (gu16IspInwsize*gu16IspInhsize*3/2)*i);
        if(!_UTest_PutFile2BufferAddr("/mnt/Ispin_1920x1080_5.yuv",
            (u32)(gu16IspInwsize*gu16IspInhsize*3/2),IspViraddr[i]))
        {
            return ;
        }
    }
    u64Time = DrvSclOsGetSystemTimeStamp();
    while(1)
    {
        for(i=0;i<u16Count;i++)
        {
            MHalVpeIqProcess((void *)u32IqCtx[idxM[i%u16Channel]], pCmdqCfg);
            MHalVpeSclProcess((void *)u32Ctx[idxM[i%u16Channel]],pCmdqCfg, &stpBuffer[idxM[i%u16Channel]]);
            stVdinfo[idxM[i%u16Channel]].u32Stride[0] = FHDOUTPUTWIDTH;
            stVdinfo[idxM[i%u16Channel]].u32Stride[1] = FHDOUTPUTWIDTH;
            stVdinfo[idxM[i%u16Channel]].u64PhyAddr[0] = Ispaddr[i%u16InputCount];
            stVdinfo[idxM[i%u16Channel]].u64PhyAddr[1] = Ispaddr[i%u16InputCount]+((u32)(FHDOUTPUTWIDTH*FHDOUTPUTHEIGHT));
            MHalVpeIspProcess((void *)u32IspCtx[idxM[i%u16Channel]],pCmdqCfg, &stVdinfo[idxM[i%u16Channel]]);
            MHalVpeSclSetWaitDone((void *)u32Ctx[idxM[i%u16Channel]],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
            if(pCmdqCfg)
            {
                pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,i+1);
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
            }
        }
        if(pCmdqCfg)
        {
            do
            {
                DrvSclOsDelayTask(5);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(!(u16fence==u16Count));
        }
        u64TimeDiff = DrvSclOsGetSystemTimeStamp() - u64Time;
        SCL_ERR("[UT]process %d frame time :@%llu\n",u16Count,u64TimeDiff);
        while(1)
        {
            if(u64TimeDiff <1000000 && u64TimeDiff >995000)
            {
                DrvSclOsDelayTask(1);
            }
            else if(u64TimeDiff <1000000 && u64TimeDiff >900000)
            {
                DrvSclOsDelayTask(5);
            }
            else if(u64TimeDiff <1000000 && u64TimeDiff >500000)
            {
                DrvSclOsDelayTask(50);
            }
            else
            {
                break;
            }
            u64TimeDiff = DrvSclOsGetSystemTimeStamp() - u64Time;
        }
        u64Time += 1000000;
        SCL_ERR("[UT]u64TimeDiff :@%llu u64Time:@%llu\n",u64TimeDiff,u64Time);
    }
}
void UTest_CaseL(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u64 u64Time;
    u64 u64TimeDiff;
    u32 Ispaddr[5];
    u32* IspViraddr[5];
    u32 idxM[8];
    u32 idxS[8];
    u16 u16fence = 0;
    u16 u16FpsMain = 15;
    u16 u16FpsSub = 30;
    u16 u16ChannelMain = 8;
    u16 u16ChannelSub = 8;
    u16 u16InputCount = 5;
    u16 u16CountMain ;
    u16 u16CountSub ;
    u16 u16CountAll ;
    u16CountMain = u16FpsMain * u16ChannelMain;
    u16CountSub = u16FpsSub * u16ChannelSub;
    u16CountAll = u16CountMain+u16CountSub;
    gu16IspInwsize = FHDOUTPUTWIDTH;
    gu16IspInhsize = FHDOUTPUTHEIGHT;
    gu16swsize = FHDOUTPUTWIDTH;
    gu16shsize = FHDOUTPUTHEIGHT;
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    for(i=0;i<u16ChannelMain;i++)
    {
        idx = _UTest_GetEmptyIdx();
        idxM[i] = idx;
        _UTest_CreateSclInstance
        (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
        if(u8level==1)
        {
            _UTest_SetIqCfg(idx,SET_NR);
            _UTest_SetIqOnOff(idx,SET_NR);
        }
        else if(u8level==2)
        {
            _UTest_SetIqCfg(idx,SET_all);
            _UTest_SetIqOnOff(idx,SET_all);
        }
        else
        {
            _UTest_SetIqCfg(idx,SET_DEF);
            _UTest_SetIqOnOff(idx,SET_DEF);
        }
        _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2);
        if(sg_scl_yc_viraddr[idx])
        {
            _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_MST_420,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],1,gu16swsize);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,0,0,0);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,0,0,0);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,0,0,0);
        }
    }
    for(i=0;i<u16ChannelSub;i++)
    {
        idx = _UTest_GetEmptyIdx();
        idxS[i] = idx;
        _UTest_CreateSclInstance
        (idx,gu16IspInwsize/2,gu16IspInhsize/2,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
        if(u8level==1)
        {
            _UTest_SetIqCfg(idx,SET_NR);
            _UTest_SetIqOnOff(idx,SET_NR);
        }
        else if(u8level==2)
        {
            _UTest_SetIqCfg(idx,SET_all);
            _UTest_SetIqOnOff(idx,SET_all);
        }
        else
        {
            _UTest_SetIqCfg(idx,SET_DEF);
            _UTest_SetIqOnOff(idx,SET_DEF);
        }
        _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize/2);
        if(sg_scl_yc_viraddr[idx])
        {
            _UTest_SetInputCfgISPMode(idx, gu16IspInwsize/2, gu16IspInhsize/2);
            _UTest_SetCropCfg(idx, gu16IspInwsize/2, gu16IspInhsize/2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV_MST_420,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize/2, gu16shsize/2);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,0,0,0);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,0,0,0);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,0,0,0);
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],1,gu16swsize*3/4);
        }
    }
    _UTest_AllocateIspMem(0,(u32)gu16IspInwsize*gu16IspInhsize*3/2*5);
    for(i=0;i<u16InputCount;i++)
    {
        Ispaddr[i] = sg_Isp_yc_addr[0] + (gu16IspInwsize*gu16IspInhsize*3/2)*i;
        IspViraddr[i] = (u32*)((u32)sg_Isp_yc_viraddr[0] + (gu16IspInwsize*gu16IspInhsize*3/2)*i);
        if(!_UTest_PutFile2BufferAddr("/mnt/Ispin_1920x1080_5.yuv",
            (u32)(gu16IspInwsize*gu16IspInhsize*3/2),IspViraddr[i]))
        {
            return ;
        }
    }
    while(1)
    {
        u64Time = DrvSclOsGetSystemTimeStamp();
        for(i=0;i<u16CountAll;i++)
        {
            if(i%3)
            {
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026A2,0x1f,0xFFFF);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x8,0x8);
                MHalVpeSclProcess((void *)u32Ctx[idxS[i%u16ChannelSub]],pCmdqCfg, &stpBuffer[idxS[i%u16ChannelSub]]);
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x8);
                MHalVpeIqProcess((void *)u32IqCtx[idxS[i%u16ChannelSub]], pCmdqCfg);
                stVdinfo[idxS[i%u16ChannelSub]].u32Stride[0] = gu16IspInwsize/2;
                stVdinfo[idxS[i%u16ChannelSub]].u32Stride[1] = gu16IspInwsize/2;
                stVdinfo[idxS[i%u16ChannelSub]].u64PhyAddr[0] = Ispaddr[i%u16InputCount];
                stVdinfo[idxS[i%u16ChannelSub]].u64PhyAddr[1] = Ispaddr[i%u16InputCount]+((u32)(FHDOUTPUTWIDTH*FHDOUTPUTHEIGHT));
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x8,0x8);
                MHalVpeIspProcess((void *)u32IspCtx[idxS[i%u16ChannelSub]],pCmdqCfg, &stVdinfo[idxS[i%u16ChannelSub]]);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x0);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idxS[i%u16ChannelSub]],pCmdqCfg,E_MHAL_VPE_WAITDONE_MDWINONLY);
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x8);
            }
            else
            {
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026A2,0x1f,0xFFFF);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x8,0x8);
                MHalVpeSclProcess((void *)u32Ctx[idxM[i%u16ChannelMain]],pCmdqCfg, &stpBuffer[idxM[i%u16ChannelMain]]);
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x8);
                MHalVpeIqProcess((void *)u32IqCtx[idxM[i%u16ChannelMain]], pCmdqCfg);
                stVdinfo[idxM[i%u16ChannelMain]].u32Stride[0] = FHDOUTPUTWIDTH;
                stVdinfo[idxM[i%u16ChannelMain]].u32Stride[1] = FHDOUTPUTWIDTH;
                stVdinfo[idxM[i%u16ChannelMain]].u64PhyAddr[0] = Ispaddr[i%u16InputCount];
                stVdinfo[idxM[i%u16ChannelMain]].u64PhyAddr[1] = Ispaddr[i%u16InputCount]+((u32)(FHDOUTPUTWIDTH*FHDOUTPUTHEIGHT));
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x8,0x8);
                MHalVpeIspProcess((void *)u32IspCtx[idxM[i%u16ChannelMain]],pCmdqCfg, &stVdinfo[idxM[i%u16ChannelMain]]);
                pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x8);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idxM[i%u16ChannelMain]],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
                //pCmdqCfg->MHAL_CMDQ_WriteRegCmdqMask(pCmdqCfg,0x1026CE,0x0,0x8);
            }
            if(pCmdqCfg)
            {
                pCmdqCfg->MHAL_CMDQ_WriteDummyRegCmdq(pCmdqCfg,i+1);
                SCL_DBGERR("[UT]fence:%x\n",i+1);
                pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                SCL_DBGERR("[UT]read fence:%x\n",u16fence);
            }
            /*
            if(pCmdqCfg)
            {
                do
                {
                    DrvSclOsDelayTask(1);
                    pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
                }
                while(!(u16fence==i+1));
            }*/
        }
        if(pCmdqCfg)
        {
            do
            {
                DrvSclOsDelayTask(1);
                pCmdqCfg->MHAL_CMDQ_ReadDummyRegCmdq(pCmdqCfg,&u16fence);
            }
            while(!(u16fence==u16CountAll));
        }
        u64TimeDiff = DrvSclOsGetSystemTimeStamp() - u64Time;
        SCL_ERR("[UT]process %d frame time :@%llu\n",u16CountAll,u64TimeDiff);
    }
}
void UTest_CaseS(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u32 u32CMDQIrq;
    //disable all
    _UTest_CleanInst();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    for(idx=0;idx<16;idx++)
    {
        _UTest_CreateSclInstance
        (idx,MAXSTRIDEINPUTWIDTH,MAXSTRIDEINPUTHEIGHT,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        IdNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
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
            _UTest_SetIqCfg(idx,SET_DEF);
            _UTest_SetIqOnOff(idx,SET_DEF);
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
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
    if(gu8Count>1)
    {
        for(i=1;i<gu8Count;i++)
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
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
        }
    }
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestS_%dx%d_16chin1_%hx.bin a:0x%lx++0x%lx \n",
        MAXOUTPUTSTRIDEH,MAXOUTPUTSTRIDEV,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2));

}
void UTest_Cases(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u32 u32CMDQIrq;
    //disable all
    _UTest_CleanInst();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    for(idx=0;idx<16;idx++)
    {
        _UTest_CreateSclInstance
        (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        IdNum[idx] = 1;
        _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIspNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        MHalVpeIqSetDnrTblMask((void *)u32IqCtx[idx],0, pCmdqCfg);
        if(idx==0)
        {
            _UTest_AllocateMem(idx,(u32)MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
            _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
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
        if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
        {
            return ;
        }
        if(sg_scl_yc_viraddr[0])
        {
            _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,MAXOUTPUTportH, MAXOUTPUTportV);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,MAXOUTPUTportH, MAXOUTPUTportV);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,MAXOUTPUTportH, MAXOUTPUTportV);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,MAXOUTPUTportH, MAXOUTPUTportV);
            _UTest_SetIqCfg(idx,SET_DEF);
            _UTest_SetIqOnOff(idx,SET_DEF);
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
            stVdinfo[idx].u32Stride[0] = gu16IspInwsize;
            stVdinfo[idx].u32Stride[1] = gu16IspInwsize;
            stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[0];
            stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[0]+((u32)(gu16IspInwsize*gu16IspInhsize));
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
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
    if(gu8Count>1)
    {
        for(i=1;i<gu8Count;i++)
        {
            for(idx=0;idx<16;idx++)
            {
                if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
                {
                    return ;
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
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2);
        }
    }
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestS_%dx%d_16chin1_%hx.bin a:0x%lx++0x%lx \n",
        MAXOUTPUTSTRIDEH,MAXOUTPUTSTRIDEV,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(MAXOUTPUTSTRIDEH*MAXOUTPUTSTRIDEV*2));
}
void UTest_CaseM(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u32 u32CMDQIrq;
    u16 inputhsize[16] = {352,480,176,192,416,640,256,360,288,360,244,256,240,480,256,192};
    u16 inputvsize[16] = {288,360,244,256,240,480,256,192,352,480,176,192,416,640,256,360};
    u32 u32Addr;
    //disable all
    _UTest_CleanInst();
    if(pCmdqCfg==NULL)
    {
        pCmdqCfg = _UTest_AllocateCMDQ();
    }
    writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
    for(idx=0;idx<16;idx++)
    {
        _UTest_CreateSclInstance
        (idx,inputhsize[idx],inputvsize[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
        IdNum[idx] = 1;
        _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
        IdIqNum[idx] = 1;
        gu16swsize = MAXOUTPUTportH;
        gu16shsize = MAXOUTPUTportV;
        if(idx==0)
        {
            _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2*31);
            u32Addr = sg_scl_yc_addr[0];
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
        if(sg_scl_yc_viraddr[0])
        {
            _UTest_SetInputCfgPatMode(idx, inputhsize[idx], inputvsize[idx]);
            _UTest_SetCropCfg(idx, inputhsize[idx], inputvsize[idx]);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
            _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
            _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            //  1,2,3,4,5,6,  8,9,10,11,12,13,  , 15    =13
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                u32Addr,(idx%7) ? 1 : 0,gu16swsize*2);
            if((idx%7))
            {
                u32Addr = u32Addr+(u32)(gu16swsize*gu16shsize*2);
            }
            // 0, 2,  4,   6,  8    10,    12,    14        = 8
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                u32Addr,(idx%2) ? 0 : 1,gu16swsize*2);
            if(!(idx%2))
            {
                u32Addr = u32Addr+(u32)(gu16swsize*gu16shsize*2);
            }
            //  1,2  ,4,5,  7,8,   10,11,    13,14,15   =11
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                u32Addr,(idx%3) ? 1 : 0,gu16swsize*2);
            if((idx%3))
            {
                u32Addr = u32Addr+(u32)(gu16swsize*gu16shsize*2);
            }
            _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                u32Addr,0,gu16swsize*2);
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
    _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
    if(gu8Count>1)
    {
        for(i=1;i<gu8Count;i++)
        {
            for(idx=0;idx<16;idx++)
            {
                if(pCmdqCfg)
                {
                    do
                    {
                        DrvSclOsDelayTask(5);
                        pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                    }
                    while(!(u32CMDQIrq&0x800));
                }
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
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
        }
    }
    SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestM_%dx%d_multiport_%hx.bin a:0x%lx++0x%lx \n",
        gu16swsize,gu16shsize,(u16)(u32Time&0xFFFF)
        ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2*31));

}
void UTest_Casem(u8 u8level,u32 u32Time)
{
    int idx;
    int i;
    u32 u32CMDQIrq;
    //u16 inputhsize[16] = {352,480,176,192,416,640,256,360,288,360,244,256,240,480,256,192};
    //u16 inputvsize[16] = {288,360,244,256,240,480,256,192,352,480,176,192,416,640,256,360};
    u32 u32Addr;
    //disable all
    if(u8level>=2)
    {
        UT_multiport_multisize(u8level,gu8Count);
    }
    else
    {
        _UTest_CleanInst();
        if(pCmdqCfg==NULL)
        {
            pCmdqCfg = _UTest_AllocateCMDQ();
        }
        writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
        gu16swsize = MAXOUTPUTportH;
        gu16shsize = MAXOUTPUTportV;
        //idx=0;
        for(idx=0;idx<16;idx++)
        {
            _UTest_CreateSclInstance
            (idx,gu16IspInwsize,gu16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
            IdNum[idx] = 1;
            _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
            IdIspNum[idx] = 1;
            _UTest_CreateIqInstance(idx,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
            IdIqNum[idx] = 1;
            if(idx==0)
            {
                _UTest_AllocateMem(idx,(u32)gu16swsize*gu16shsize*2*31);
                _UTest_AllocateIspMem(idx,(u32)gu16IspInwsize*gu16IspInhsize*3/2);
                u32Addr = sg_scl_yc_addr[0];
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
            if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
            {
                return ;
            }
            if(sg_scl_yc_viraddr[0])
            {
                _UTest_SetInputCfgISPMode(idx, gu16IspInwsize, gu16IspInhsize);
                _UTest_SetCropCfg(idx, gu16IspInwsize, gu16IspInhsize);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT0);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT1);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT2);
                _UTest_SetDmaCfg(idx,E_MHAL_PIXEL_FRAME_YUV422_YUYV,E_MHAL_SCL_OUTPUT_PORT3);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,gu16swsize, gu16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,gu16swsize, gu16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,gu16swsize, gu16shsize);
                _UTest_SetOutputSizeCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,gu16swsize, gu16shsize);
                MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
                //  1,2,3,4,5,6,  8,9,10,11,12,13,  , 15    =13
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,
                    u32Addr,(idx%7) ? 1 : 0,gu16swsize*2);
                if((idx%7))
                {
                    u32Addr = u32Addr+(gu16swsize*gu16shsize*2);
                }
                // 0, 2,  4,   6,  8    10,    12,    14        = 8
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,
                    u32Addr,(idx%2) ? 0 : 1,gu16swsize*2);
                if(!(idx%2))
                {
                    u32Addr = u32Addr+(gu16swsize*gu16shsize*2);
                }
                //  1,2  ,4,5,  7,8,   10,11,    13,14,15   =11
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,
                    u32Addr,(idx%3) ? 1 : 0,gu16swsize*2);
                if((idx%3))
                {
                    u32Addr = u32Addr+(u32)(gu16swsize*gu16shsize*2);
                }
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,
                    u32Addr,0,gu16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                stVdinfo[idx].u32Stride[0] = gu16IspInwsize;
                stVdinfo[idx].u32Stride[1] = gu16IspInwsize;
                stVdinfo[idx].u64PhyAddr[0] = sg_Isp_yc_addr[0];
                stVdinfo[idx].u64PhyAddr[1] = sg_Isp_yc_addr[0]+((u32)(gu16IspInwsize*gu16IspInhsize));
                MHalVpeIspProcess((void *)u32IspCtx[idx],pCmdqCfg, &stVdinfo[idx]);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,E_MHAL_VPE_WAITDONE_DMAONLY);
            }
        }
        pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);
        DrvSclOsDelayTask(1000);
        DrvSclOsDirectMemFlush((u32)sg_scl_yc_viraddr[0],gu16swsize*gu16shsize*2*31);
        _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
        if(gu8Count>1)
        {
            for(i=1;i<gu8Count;i++)
            {
                for(idx=0;idx<16;idx++)
                {
                    if(pCmdqCfg)
                    {
                        do
                        {
                            DrvSclOsDelayTask(5);
                            pCmdqCfg->MHAL_CMDQ_ReadStatusCmdq(pCmdqCfg,(unsigned int*)&u32CMDQIrq);
                        }
                        while(!(u32CMDQIrq&0x800));
                    }
                    if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
                    {
                        return ;
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
                _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[0],MAXOUTPUTportH*MAXOUTPUTportV*2*31);
            }
        }
        SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestM_%dx%d_multiport_%hx.bin a:0x%lx++0x%lx \n",
            gu16swsize,gu16shsize,(u16)(u32Time&0xFFFF)
            ,sg_scl_yc_addr[0]+MIU0_BASE,(u32)(gu16swsize*gu16shsize*2*31));
    }
}
void UTest_CaseW(u8 u8level,u32 u32Time)
{
    int idx;
    u32 u32CMDQIrq;
    if(IdIspNum[u8level])
    {
        idx=0;
        MHalVpeSclDbgLevel((void *)&idx);
        while(1)
        {
            if(!_UTest_PutFile2Buffer(u8level,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
            {
                return ;
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

}
void UTest_Casew(u8 u8level,u32 u32Time)
{
    int idx;
    u32 u32CMDQIrq;
    idx=0;
    MHalVpeSclDbgLevel((void *)&idx);
    while(1)
    {
        for(idx=0;idx<=u8level;idx++)
        {
            if(IdIspNum[idx])
            {
                if(!_UTest_PutFile2Buffer(0,"/tmp/Ispin.yuv",(u32)(gu16IspInwsize*gu16IspInhsize*3/2)))
                {
                    return ;
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
void UTest_CaseT(u8 u8level,u32 u32Time)
{
    if(IdIspNum[u8level])
    {
        _UTest_Create_Task(u8level,(u32)(gu16IspInwsize*gu16IspInhsize*3/2),(TaskEntry)UT_DispTask);
    }
    SCL_ERR( "[VPE]Create_Task\n");
}
void UTest_Caset(u8 u8level,u32 u32Time)
{
    if(IdIspNum[u8level])
    {
        _UTest_Destroy_Task();
    }
    SCL_ERR( "[VPE]Destroy_Task\n");
}
void UTest_CaseIQ(u8 u8level,u32 u32Time,const char *str)
{
    int idx;
    gu16PatInwsize = MAXINPUTWIDTH;
    gu16PatInhsize = MAXINPUTHEIGHT;
    if((int)*str >= 48 && (int)*str <= 57)    //Test 0~9
    {
        gu8Str[gu8Count-1] = ((int)*str - 48)*25;
        SCL_ERR( "[VPE]Set [%d]:%d\n",gu8Count-1, gu8Str[gu8Count-1]);
        gu8Count++;
    }
    else if((int)*str == 'C')  //Test
    {
        gu8Count=1;
    }
    else if((int)*str == 'E')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgYEE(idx,gu8Str);
            _UTest_SetIqOnOff(idx,SET_EG);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'S')  //Test S
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgES(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_ES,1);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 's')  //Test S
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgES(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_ES,0);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'N')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgMCNR(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_NR,1);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'n')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgMCNR(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_NR,0);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'W')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgWDR(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_CON,1);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
     else if((int)*str == 'w')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfgWDR(idx,gu8Str);
            _UTest_SetIqOnOff1(idx,SET_CON,0);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'U')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfg(idx,SET_UV);
            _UTest_SetIqOnOff(idx,SET_UV);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'A')  //Test
    {
        if(IdIqNum[u8level])
        {
            idx = u8level;
            _UTest_SetIqCfg(idx,SET_all);
            _UTest_SetIqOnOff(idx,SET_all);
            MHalVpeIqProcess((void *)u32IqCtx[idx], pCmdqCfg);
            SCL_ERR("[VPE]Set Config Default \n");
        }
        SCL_ERR( "[VPE]Set %d\n",(int)*str);
    }
    else if((int)*str == 'F')  //Test F
    {
        UTest_CaseF(u8level);
    }
    else if((int)*str == 'G')  //Test G
    {
        UTest_CaseG(u8level);
    }
    else if((int)*str == 'H')  //Test H
    {
        UTest_CaseH(u8level);
    }
    else if((int)*str == 'I')  //Test I
    {
        IqUTest_CaseI(u8level);
    }
    else if((int)*str == 'J')  //Test J
    {
        IqUTest_CaseJ(u8level);
    }
    else if((int)*str == 'M')  //Test M
    {
        UTest_CaseN(u8level);
    }
    else if((int)*str == 'D')  //Test D
    {
        UTest_CaseD(u8level,u32Time);
    }
    else if((int)*str == 'd')  //Test d
    {
        UTest_Cased(u8level,u32Time);
    }
}
ssize_t check_testm2m_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    const char *str = buf;
    u8 u8level;
    u32 u32Time;

    if(NULL!=buf)
    {
        u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
        u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
        gu8Str[0] = gu8Str[0];
        if((int)*str == 48)    //Test 0
        {
            M2MUTest_Case0();
        }
        if((int)*str == 49)    //Test 1
        {
            M2MUTest_Case1();
        }
        else if((int)*str == 50)  //Test 2
        {
            M2MUTest_Case2(u8level);
        }
        else if((int)*str == 51)  //Test 3
        {
            M2MUTest_Case3(u8level);
        }
        else if((int)*str == 52)  //Test 4
        {
            M2MUTest_Case4(u8level);
        }
        else if((int)*str == 53)  //Test 5
        {
            M2MUTest_Case5(u32Time);
        }
        else if((int)*str == 54)  //Test 6
        {
            M2MUTest_Case6(u8level,u32Time);
        }
        else if((int)*str == 55)//Test 7
        {
            M2MUTest_Case7(u8level);
        }
        else if((int)*str == 56)//Test 8
        {
            M2MUTest_Case8(u8level);
        }
        else if((int)*str == 'A')//TestA
        {
            M2MUTest_CaseA(u8level);
        }
        else if((int)*str == 'a')//TestA
        {
            M2MUTest_Casea(u8level);
        }
        else if((int)*str == 'B')//TestB
        {
            M2MUTest_CaseB(u8level);
        }
        else if((int)*str == 'b')//Testb
        {
            M2MUTest_Caseb(u8level);
        }
        else if((int)*str == 'C')//TestC
        {
            M2MUTest_CaseC(u8level);
        }
        else if((int)*str == 'c')//Testc
        {
            M2MUTest_Casec(u8level);
        }
        else if((int)*str == 'D')//TestD
        {
            M2MUTest_CaseD(u8level);
        }
        else if((int)*str == 'd')//Testd
        {
            M2MUTest_Cased(u8level);
        }
        else if((int)*str == 'E')//Teste
        {
            M2MUTest_CaseE(u8level);
        }
        else if((int)*str == 'e')//Testd
        {
            M2MUTest_Casee(u8level);
        }
        else if((int)*str == 'F')//Teste
        {
            M2MUTest_CaseF(u8level);
        }
        else if((int)*str == 'f')//Testd
        {
            M2MUTest_Casef(u8level);
        }
        else if((int)*str == 'I')//TestI
        {
            M2MUTest_CaseI(u8level);
        }
        else if((int)*str == 'i')//Testi
        {
            M2MUTest_Casei(u8level);
        }
        else if((int)*str == 'J')//TestJ
        {
            M2MUTest_CaseJ(u8level);
        }
        else if((int)*str == 'P')//TestP
        {
            M2MUTest_CaseP(u8level);
        }
        return n;
    }

    return 0;
}

ssize_t check_testm2m_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    end = end;
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 0: Close All Sub Inst\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 1: Create Scl_Sub Inst Max Input Size(%d,%d)\n",MAXOUTPUTportH,MAXOUTPUTportV);
    str += DrvSclOsScnprintf(str, end - str, "Test 2: Set All Input/Crop Config bEn:by idx size(,,,)\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 3: Set All Dma config by level output 0:422 1:420 input :3:422 4:420 5:RGB...\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 4: Set all Sub Inst Dma output size by level 0:FHD 1:XGA 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 5: Set for All Sub Inst Process\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 6: Set for level Sub Inst Process\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 7: Set level Sub Inst Dma output size level 0:FHD 1:XGA 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 8: Set level Sub Inst Dma input size level 0:FHD 1:736x240 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test I: open Sub Inst Isr\n");
    str += DrvSclOsScnprintf(str, end - str, "Test i: Close Sub Inst Isr\n");
    str += DrvSclOsScnprintf(str, end - str, "Test A: Single Trigger mode\n");
    str += DrvSclOsScnprintf(str, end - str, "Test a: CMdq Single Trigger mode\n");
    str += DrvSclOsScnprintf(str, end - str, "Test B: Multiinst Trigger mode\n");
    str += DrvSclOsScnprintf(str, end - str, "Test b: Cmdq Multiinst Trigger mode\n");
    str += DrvSclOsScnprintf(str, end - str, "Test C: Single Trigger mode+ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "Test c: CMdq Single Trigger mode+ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "Test D: Multiinst Trigger mode+ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "Test d: Cmdq Multiinst Trigger mode+ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "Test E: Trigger mode+VPE pat\n");
    str += DrvSclOsScnprintf(str, end - str, "Test e: Cmdq Trigger mode+VPE pat\n");
    str += DrvSclOsScnprintf(str, end - str, "Test F: Trigger mode+VPE ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "Test f: Cmdq Trigger mode+VPE ISP\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    return (str - buf);
}
ssize_t check_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    const char *str = buf;
    u8 u8level;
    u32 u32Time;

    if(NULL!=buf)
    {
        u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
        u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
        gu8Str[0] = gu8Str[0];

        if((int)*str == 48)    //Test 0
        {
            UTest_Case0();
        }
        if((int)*str == 49)    //Test 1
        {
            UTest_Case1();
        }
        else if((int)*str == 50)  //Test 2
        {
            UTest_Case2();
        }
        else if((int)*str == 51)  //Test 3
        {
            UTest_Case3(u8level);
        }
        else if((int)*str == 52)  //Test 4
        {
            UTest_Case4(u8level);
        }
        else if((int)*str == 53)  //Test 5
        {
            UTest_Case5(u32Time);
        }
        else if((int)*str == 54)  //Test 6
        {
            UTest_Case6(u8level,u32Time);
        }
        else if((int)*str == 55)//Test 7
        {
            UTest_Case7(u8level);
        }
        else if((int)*str == 56)//Test 8
        {
            UTest_Case8(u8level);
        }
        else if((int)*str == 57)//Test9
        {
            UTest_Case9(u8level);
        }
        else if((int)*str == 65)  //Test A
        {
            UTest_CaseA(u8level,u32Time);
        }
        else if((int)*str == 'a')  //Test a
        {
            UTest_Casea(u8level,u32Time);
        }
        else if((int)*str == 66)  //Test B
        {
            UTest_CaseB(u8level,u32Time);
        }
        else if((int)*str == 'b')  //Test b
        {
            UTest_Caseb(u8level,u32Time);
        }
        else if((int)*str == 67)  //Test C
        {
            UTest_CaseC();
        }
        else if((int)*str == 'c')  //Test c
        {
            UTest_Casec(u8level,u32Time);
        }
        else if((int)*str == 'D')  //Test D
        {
            UTest_CaseD(u8level,u32Time);
        }
        else if((int)*str == 'd')  //Test d
        {
            UTest_Cased(u8level,u32Time);
        }
        else if((int)*str == 69)  //Test E
        {
            UTest_CaseE(u8level);
        }
        else if((int)*str == 'e')  //Test e
        {
            UTest_Casee(u8level);
        }
        else if((int)*str == 'I')  //Test I
        {
            UTest_CaseI(u8level);
        }
        else if((int)*str == 'i')  //Test I
        {
            UTest_Casei(u8level);
        }
        else if((int)*str == 'J')  //Test J
        {
            UTest_CaseJ(u8level);
        }
        else if((int)*str == 'j')  //Test j
        {
            UTest_Casej(u8level);
        }
        else if((int)*str == 'P')  //Test P
        {
            UTest_CaseP(u8level);
        }
        else if((int)*str == 'p')  //Test p
        {
            UTest_Casep(u8level,u32Time);
        }
        else if((int)*str == 'S')  //Test S
        {
            UTest_CaseS(u8level,u32Time);
        }
        else if((int)*str == 's')  //Test s
        {
            UTest_Cases(u8level,u32Time);
        }
        else if((int)*str == 'M') //Test M
        {
            UTest_CaseM(u8level,u32Time);
        }
        else if((int)*str == 'm') //Test m
        {
            UTest_Casem(u8level,u32Time);
        }
        else if((int)*str == 'W')  //Test W
        {
            UTest_CaseW(u8level,u32Time);
        }
        else if((int)*str == 'w')  //Test w
        {
            UTest_Casew(u8level,u32Time);
        }
        else if((int)*str == 'T')  //Test T
        {
            UTest_CaseT(u8level,u32Time);
        }
        else if((int)*str == 't')  //Test T
        {
            UTest_Caset(u8level,u32Time);
        }
        else if((int)*str == 'K')  //Test T
        {
            UTest_CaseK(u8level,u32Time);
        }
        else if((int)*str == 'L')  //Test T
        {
            UTest_CaseL(u8level,u32Time);
        }
        //if(pCmdqCfg)
        //pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);

        return n;
    }

    return 0;
}

ssize_t check_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    end = end;
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 0: Close All Inst\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 1: Create Scl Inst Max Input Size(%d,%d)\n",MAXOUTPUTportH,MAXOUTPUTportV);
    str += DrvSclOsScnprintf(str, end - str, "Test 2: Set All Input/Crop Config bEn:by idx size(,,,)\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 3: Set All Dma config by level output 0:422 1:420 input :3:422 4:420 5:RGB...\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 4: Set all Dma output size by level 0:FHD 1:XGA 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 5: Set for All Dma Process\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 6: Set for level Dma Process\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 7: Set level Dma output size level 0:FHD 1:XGA 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 8: Set level Pat input size level 0:FHD 1:736x240 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test 9: Set level Isp input size level 0:FHD 1:736x240 2:HD 3:VGA 4:CIF 5:3M 6:4M 7:5M 8:s4K2K 9:4K2K\n");
    str += DrvSclOsScnprintf(str, end - str, "Test A: PT All Flow level port\n");
    str += DrvSclOsScnprintf(str, end - str, "Test a: PT All Flow CMDQ level port\n");
    str += DrvSclOsScnprintf(str, end - str, "Test B: PT RealTime Mode All Flow level port\n");
    str += DrvSclOsScnprintf(str, end - str, "Test b: PT RealTime Mode All Flow CMDQ level port\n");
    str += DrvSclOsScnprintf(str, end - str, "Test C: Create IQ Inst\n");
    str += DrvSclOsScnprintf(str, end - str, "Test c: ISP All Flow level port 422mode //tmp//Ispin.yuv\n");
    str += DrvSclOsScnprintf(str, end - str, "Test D: ISP All Flow level port //tmp//Ispin.yuv\n");
    str += DrvSclOsScnprintf(str, end - str, "Test d: ISP All Flow level port Process\n");
    str += DrvSclOsScnprintf(str, end - str, "Test E: Get Nr by Level\n");
    str += DrvSclOsScnprintf(str, end - str, "Test e: Get Roi by Level\n");
    str += DrvSclOsScnprintf(str, end - str, "Test I: open Main Isr\n");
    str += DrvSclOsScnprintf(str, end - str, "Test i: Close Main Isr\n");
    str += DrvSclOsScnprintf(str, end - str, "Test J: open Imi\n");
    str += DrvSclOsScnprintf(str, end - str, "Test j: Close Imi\n");
    str += DrvSclOsScnprintf(str, end - str, "Test K0: port0 FHD @30x4ch port1 CIF @30x4ch port2 QHD @30x4(zero)\n");
    str += DrvSclOsScnprintf(str, end - str, "Test K1: Test K+3DNR\n");
    str += DrvSclOsScnprintf(str, end - str, "Test K2: Test K+3DNR+IQ\n");
    str += DrvSclOsScnprintf(str, end - str, "Test L0: input FHD port0 FHD @15x8ch input QHD port3 QHD @30x8ch\n");
    str += DrvSclOsScnprintf(str, end - str, "Test L1: Test L+3DNR\n");
    str += DrvSclOsScnprintf(str, end - str, "Test L2: Test L+3DNR+IQ\n");
    str += DrvSclOsScnprintf(str, end - str, "Test M: PT  16inst multiport random input/crop 256x256 output\n");
    str += DrvSclOsScnprintf(str, end - str, "Test m: ISP 16inst multiport random input/crop 256x256 output\n");
    str += DrvSclOsScnprintf(str, end - str, "Test P: ISP All Flow level port write file count\n");
    str += DrvSclOsScnprintf(str, end - str, "Test p: ISP All Flow level port write file \n");
    str += DrvSclOsScnprintf(str, end - str, "Test R: ISP Rotate by level inst\n");
    str += DrvSclOsScnprintf(str, end - str, "Test r: ISP Set Rotate angle by level\n");
    str += DrvSclOsScnprintf(str, end - str, "Test S: PT  16inst port 0 Stride 1 frame 256x256x16\n");
    str += DrvSclOsScnprintf(str, end - str, "Test s: ISP 16inst port 0 Stride 1 frame 256x256x16\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    return (str - buf);
}

#if defined(USE_USBCAM)
ssize_t check_iqtest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    const char *str = buf;
    int idx,i;
    u8 u8level;
    u32 u32Time,u32CMDQIrq;
    static u16 u16swsize = FHDOUTPUTWIDTH;
    static u16 u16shsize = FHDOUTPUTHEIGHT;
    static u16 u16IspInwsize = ISPINPUTWIDTH;
    static u16 u16IspInhsize = ISPINPUTHEIGHT;
    static u8 u8Str[10];

    if(NULL!=buf)
    {
        u8level = _mdrv_Scl_Changebuf2hex((int)*(str+1));
        u32Time = ((u32)DrvSclOsGetSystemTimeStamp());
        u8Str[0] = u8Str[0];

        if((int)*str == 'A')  //Test A
        {
#if 1
            DrvSclVipIoPeakingConfig_t tTmpcfg;
            DrvSclVipIoDlcHistogramConfig_t t1Tmpcfg;
            DrvSclVipIoDlcConfig_t t2Tmpcfg;
            DrvSclVipIoLceConfig_t t3Tmpcfg;
            DrvSclVipIoUvcConfig_t t4Tmpcfg;
            DrvSclVipIoIhcConfig_t t5Tmpcfg;
            DrvSclVipIoIccConfig_t t6Tmpcfg;
            DrvSclVipIoIhcIccConfig_t t7Tmpcfg;
            DrvSclVipIoIbcConfig_t    t8Tmpcfg;
            DrvSclVipIoFccConfig_t    t9Tmpcfg;
            DrvSclVipIoNlmConfig_t t10Tmpcfg;
            DrvSclVipIoAckConfig_t t11Tmpcfg;
            DrvSclVipIoSetMaskOnOff_t t12Tmpcfg;
            DrvSclVipIoWdrRoiHist_t   t13Tmpcfg;
            DrvSclVipIoConfig_t         t14Tmpcfg;
            DrvSclVipIoAipConfig_t       t15Tmpcfg;
            DrvSclVipIoAipSramConfig_t   t16Tmpcfg;
            void                         *tmpmem;
            DrvSclVipIoMcnrConfig_t      t17Tmpcfg;
            DrvSclVipIoReqMemConfig_t    stReqMemCfg;
#endif
            idx = _UTest_GetEmptyIdx();
            if(pCmdqCfg==NULL)
            {
                pCmdqCfg = _UTest_AllocateCMDQ();
            }
            _UTest_CreateSclInstance
            (idx,u16IspInwsize,u16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
            _UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
            IdIspNum[idx] = 1;
            u32IqCtx[idx] = DrvSclVipIoOpen(E_DRV_SCLVIP_IO_ID_1);
            IdIqNum[idx] = 1;
            _UTest_AllocateMem(idx,(u32)u16swsize*u16shsize*2);
            _UTest_AllocateIspMem(idx,(u32)u16IspInwsize*u16IspInhsize*3/2);//YUV420
            if(!_UTest_PutFile2Buffer(idx,"/tmp/Ispin.yuv",(u32)(u16IspInwsize*u16IspInhsize*3/2)))
            {
                return 0;
            }
            writefp = _UTest_OpenFile("/tmp/Scout.bin",O_WRONLY|O_CREAT,0777);
            if(sg_scl_yc_viraddr[idx])
            {
                //MHalVpeSetIrqCallback(NULL);
                //MHalVpeSclEnableIrq(1);
                _DrvSclVipIoKeepCmdqFunction(pCmdqCfg);
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
#if 1
                memset(&tTmpcfg,0xaa,sizeof(DrvSclVipIoPeakingConfig_t));
                FILL_VERCHK_TYPE(tTmpcfg,tTmpcfg.VerChk_Version,tTmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                //printk("[CMDQ]IQ peak bnum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                DrvSclVipIoSetPeakingConfig(u32IqCtx[idx],&tTmpcfg);
                //printk("[CMDQ]IQ peak mnum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ peak anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                //MHalVpeIqProcess((void *)u32IqCtx[idx],pCmdqCfg);

                memset(&t1Tmpcfg,0x11,sizeof(DrvSclVipIoDlcHistogramConfig_t));
                FILL_VERCHK_TYPE(t1Tmpcfg,t1Tmpcfg.VerChk_Version,t1Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetDlcHistogramConfig(u32IqCtx[idx],&t1Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 111 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));



                //set dlc
                memset(&t2Tmpcfg,0x22,sizeof(DrvSclVipIoDlcConfig_t));
                FILL_VERCHK_TYPE(t2Tmpcfg,t2Tmpcfg.VerChk_Version,t2Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetDlcConfig(u32IqCtx[idx],&t2Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 222 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //set lce
                memset(&t3Tmpcfg,0x33,sizeof(DrvSclVipIoLceConfig_t));
                FILL_VERCHK_TYPE(t3Tmpcfg,t3Tmpcfg.VerChk_Version,t3Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetLceConfig(u32IqCtx[idx],&t3Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 333 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //Uvc
                memset(&t4Tmpcfg,0x44,sizeof(DrvSclVipIoUvcConfig_t));
                FILL_VERCHK_TYPE(t4Tmpcfg,t4Tmpcfg.VerChk_Version,t4Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetUvcConfig(u32IqCtx[idx],&t4Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 444 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //IHC
                memset(&t5Tmpcfg,0x55,sizeof(DrvSclVipIoIhcConfig_t));
                FILL_VERCHK_TYPE(t5Tmpcfg,t5Tmpcfg.VerChk_Version,t5Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetIhcConfig(u32IqCtx[idx],&t5Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 555 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //ICC
                memset(&t6Tmpcfg,0x66,sizeof(DrvSclVipIoIccConfig_t));
                FILL_VERCHK_TYPE(t6Tmpcfg,t6Tmpcfg.VerChk_Version,t6Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetIccConfig(u32IqCtx[idx],&t6Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 666 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));

                //IhcIceAdpYConfig
                memset(&t7Tmpcfg,0x77,sizeof(DrvSclVipIoIhcIccConfig_t));
                FILL_VERCHK_TYPE(t7Tmpcfg,t7Tmpcfg.VerChk_Version,t7Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetIhcIceAdpYConfig(u32IqCtx[idx],&t7Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 777 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //Ibc
                memset(&t8Tmpcfg,0x88,sizeof(DrvSclVipIoIbcConfig_t));
                FILL_VERCHK_TYPE(t8Tmpcfg,t8Tmpcfg.VerChk_Version,t8Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetIbcConfig(u32IqCtx[idx],&t8Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 888 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));

                //FCC
                memset(&t9Tmpcfg,0x99,sizeof(DrvSclVipIoFccConfig_t));
                FILL_VERCHK_TYPE(t9Tmpcfg,t9Tmpcfg.VerChk_Version,t9Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetFccConfig(u32IqCtx[idx],&t9Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ 999 anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //nlm

                memset(&t10Tmpcfg,0xaa,sizeof(DrvSclVipIoNlmConfig_t));
                FILL_VERCHK_TYPE(t10Tmpcfg,t10Tmpcfg.VerChk_Version,t10Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetNlmConfig(u32IqCtx[idx],&t10Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ aa anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));

                //ack
                memset(&t11Tmpcfg,0xbb,sizeof(DrvSclVipIoAckConfig_t));
                FILL_VERCHK_TYPE(t11Tmpcfg,t11Tmpcfg.VerChk_Version,t11Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetAckConfig(u32IqCtx[idx],&t11Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ bb anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));


                //wdr mask
                memset(&t12Tmpcfg,0xCC,sizeof(DrvSclVipIoSetMaskOnOff_t));
                //FILL_VERCHK_TYPE(t12Tmpcfg,t12Tmpcfg.VerChk_Version,t12Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                t12Tmpcfg.enMaskType = E_DRV_SCLVIP_IO_MASK_WDR;
                DrvSclVipIoSetWdrRoiMask(u32IqCtx[idx],&t12Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ cc anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));

                //WDR ROI
                memset(&t13Tmpcfg,0xdd,sizeof(DrvSclVipIoWdrRoiHist_t));
                //FILL_VERCHK_TYPE(t13Tmpcfg,t13Tmpcfg.VerChk_Version,t13Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetWdrRoiHistConfig(u32IqCtx[idx],&t13Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ dd anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));

                //VIP
                memset(&t14Tmpcfg,0xcc,sizeof(DrvSclVipIoConfig_t));
                FILL_VERCHK_TYPE(t14Tmpcfg,t14Tmpcfg.VerChk_Version,t14Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                DrvSclVipIoSetVipConfig(u32IqCtx[idx],&t14Tmpcfg);
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ cc anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                //DrvSclVipIoSetVipConfig(s32 s32Handler,DrvSclVipIoConfig_t *pstCfg)
                //AIP
                tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
                memset(tmpmem,0xff,sizeof(tmpmem));
                for(i = E_DRV_SCLVIP_IO_AIP_YEE ; i < E_DRV_SCLVIP_IO_AIP_NUM ; i++)
                {

                    memset(&t15Tmpcfg,0xdd,sizeof(DrvSclVipIoAipConfig_t));
                    FILL_VERCHK_TYPE(t15Tmpcfg,t15Tmpcfg.VerChk_Version,t15Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                    t15Tmpcfg.enAIPType = i;
                    t15Tmpcfg.u32Viraddr = (u32)tmpmem;
                    DrvSclVipIoSetAipConfig(u32IqCtx[idx],&t15Tmpcfg);
                    //DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig)
                }
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ dd anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                DrvSclOsMemFree(tmpmem);

                //AIP sram
                tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
                memset(tmpmem,0xaa,sizeof(tmpmem));
                for(i = E_DRV_SCLVIP_IO_AIP_SRAM_WDR_TBL ; i <= E_DRV_SCLVIP_IO_AIP_SRAM_WDR_TBL ; i++)
                {
                    memset(&t16Tmpcfg,0xee,sizeof(DrvSclVipIoAipSramConfig_t));
                    FILL_VERCHK_TYPE(t16Tmpcfg,t16Tmpcfg.VerChk_Version,t16Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                    t16Tmpcfg.enAIPType = i;
                    t16Tmpcfg.u32Viraddr = (u32)tmpmem;
                    DrvSclVipIoSetAipSramConfig(u32IqCtx[idx],&t16Tmpcfg);
                    //DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
                }
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                //printk("[CMDQ]IQ ee anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                DrvSclOsMemFree(tmpmem);

                tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
                memset(&t17Tmpcfg,0xee,sizeof(DrvSclVipIoAipSramConfig_t));
                FILL_VERCHK_TYPE(t17Tmpcfg,t17Tmpcfg.VerChk_Version,t17Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                FILL_VERCHK_TYPE(stReqMemCfg,stReqMemCfg.VerChk_Version,stReqMemCfg.VerChk_Size,DRV_SCLVIP_VERSION);
                stReqMemCfg.u16Pitch = 1920;
                stReqMemCfg.u16Vsize = 1080;
                stReqMemCfg.enCeType = E_DRV_SCLVIP_IO_UCMCE_8;
                stReqMemCfg.u32MemSize = 1920*1080*2;
                t17Tmpcfg.u32Viraddr = (u32)tmpmem;
                _DrvSclVipIoReqmemConfig(u32IqCtx[idx],&stReqMemCfg);
                DrvSclVipIoSetMcnrConfig(u32IqCtx[idx],&t17Tmpcfg);
                    //DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
                //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
                DrvSclOsMemFree(tmpmem);

#endif
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT0,sg_scl_yc_addr[idx],
                    (u8level ==0 || u8level ==4 || u8level ==5 || u8level ==7 || u8level ==8) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT1,sg_scl_yc_addr[idx],
                    (u8level ==1 || u8level ==4 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT2,sg_scl_yc_addr[idx],
                    (u8level ==2 || u8level ==5 || u8level ==6 || u8level ==7) ? 1 :0,u16swsize*2);
                _UTest_SetProcessCfg(idx,E_MHAL_SCL_OUTPUT_PORT3,sg_scl_yc_addr[idx],
                    (u8level ==3 || u8level ==8) ? 1 :0,u16swsize*2);
                MHalVpeSclProcess((void *)u32Ctx[idx],pCmdqCfg, &stpBuffer[idx]);
                //printk("[CMDQ]IQ sclprocess anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
                _UTest_SetISPProcess(idx,u16IspInwsize,u16IspInhsize);
                MHalVpeSclSetWaitDone((void *)u32Ctx[idx],pCmdqCfg,_UTest_GetWaitFlag(u8level));
                u8glevel[idx] = u8level;

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
            _UTest_WriteFile(writefp,(char *)sg_scl_yc_viraddr[idx],(u32)u16swsize*u16shsize*2);
            SCL_ERR( "[VPE]Set inst Direct%d\n",(int)*str);
            SCL_ERR( "data.save.binary \\\\hcnas01\\garbage\\paul-pc.wang\\frame\\TestD_%dx%d_%d_%hx.bin a:0x%lx++0x%lx \n",
                u16swsize,u16shsize,idx,(u16)(u32Time&0xFFFF)
                ,sg_scl_yc_addr[idx]+MIU0_BASE,(u32)(u16swsize*u16shsize*2));
        }
        else if((int)*str == 'a')  //Test a
        {
            void                         *tmpmem;
            DrvSclVipIoAipConfig_t       t15Tmpcfg;
            DrvSclVipIoAipSramConfig_t   t16Tmpcfg;
            DrvSclVipIoMcnrConfig_t      t17Tmpcfg;
            DrvSclVipIoReqMemConfig_t    stReqMemCfg;

            idx = _UTest_GetEmptyIdx();

            //_UTest_CreateSclInstance
            //(idx,u16IspInwsize,u16IspInhsize,E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE,E_MHAL_SCL_OUTPUT_MODE_4PORT_2LEVEL_MODE);
            //_UTest_CreateIspInstance((void *)&u32IspCtx[idx],E_MHAL_VPE_INPUT_MODE_FRAMEBUF_DVR_MODE);
            //IdIspNum[idx] = 1;
            u32IqCtx[idx] = DrvSclVipIoOpen(E_DRV_SCLVIP_IO_ID_1);
            tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
            //AIP
            memset(tmpmem,0xff,sizeof(tmpmem));
            for(i = E_DRV_SCLVIP_IO_AIP_YEE ; i < E_DRV_SCLVIP_IO_AIP_NUM ; i++)
            {

                memset(&t15Tmpcfg,0xdd,sizeof(DrvSclVipIoAipConfig_t));
                FILL_VERCHK_TYPE(t15Tmpcfg,t15Tmpcfg.VerChk_Version,t15Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                t15Tmpcfg.enAIPType = i;
                t15Tmpcfg.u32Viraddr = (u32)tmpmem;
                DrvSclVipIoSetAipConfig(u32IqCtx[idx],&t15Tmpcfg);
                //DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig)
            }
            //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
            //printk("[CMDQ]IQ dd anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
            DrvSclOsMemFree(tmpmem);




            //AIP sram
            tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
            memset(tmpmem,0xaa,sizeof(tmpmem));
            for(i = E_DRV_SCLVIP_IO_AIP_SRAM_WDR_TBL ; i <= E_DRV_SCLVIP_IO_AIP_SRAM_WDR_TBL ; i++)
            {
                memset(&t16Tmpcfg,0xee,sizeof(DrvSclVipIoAipSramConfig_t));
                FILL_VERCHK_TYPE(t16Tmpcfg,t16Tmpcfg.VerChk_Version,t16Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
                t16Tmpcfg.enAIPType = i;
                t16Tmpcfg.u32Viraddr = (u32)tmpmem;
                DrvSclVipIoSetAipSramConfig(u32IqCtx[idx],&t16Tmpcfg);
                //DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
            }
            //DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
            //printk("[CMDQ]IQ ee anum=%d\n",pCmdqCfg->MHAL_CMDQ_GetCurrentCmdqNumber(pCmdqCfg));
            DrvSclOsMemFree(tmpmem);

            tmpmem = DrvSclOsMemalloc(16*1024,GFP_KERNEL);
            memset(&t17Tmpcfg,0xee,sizeof(DrvSclVipIoAipSramConfig_t));
            FILL_VERCHK_TYPE(t17Tmpcfg,t17Tmpcfg.VerChk_Version,t17Tmpcfg.VerChk_Size,DRV_SCLVIP_VERSION);
            FILL_VERCHK_TYPE(stReqMemCfg,stReqMemCfg.VerChk_Version,stReqMemCfg.VerChk_Size,DRV_SCLVIP_VERSION);
            stReqMemCfg.u16Pitch = 1920;
            stReqMemCfg.u16Vsize = 1080;
            stReqMemCfg.enCeType = E_DRV_SCLVIP_IO_UCMCE_8;
            stReqMemCfg.u32MemSize = 1920*1080*2;
            t17Tmpcfg.u32Viraddr = (u32)tmpmem;
            //printk("u32IqCtx[idx] =%lx\n",u32IqCtx[idx]);
            DrvSclVipIoReqmemConfig(u32IqCtx[idx],&stReqMemCfg);
            DrvSclVipIoSetMcnrConfig(u32IqCtx[idx],&t17Tmpcfg);
                //DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
            DrvSclVipIoctlInstFlip(u32IqCtx[idx]);
            DrvSclOsMemFree(tmpmem);

            IdNum[i] = 0;
            DrvSclVipIoRelease(u32IqCtx[idx]);

        }
        else
        {
                UTest_CaseIQ(u8level,u32Time,str);
        }
        //if(pCmdqCfg)
        //pCmdqCfg->MHAL_CMDQ_KickOffCmdq(pCmdqCfg);


        return n;
    }

    return 0;
}
 ssize_t check_iqtest_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    end = end;
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    str += DrvSclOsScnprintf(str, end - str, "Test F: Set Config Default\n");
    str += DrvSclOsScnprintf(str, end - str, "Test G: Set Config Edge\n");
    str += DrvSclOsScnprintf(str, end - str, "Test H: Set Config Es\n");
    str += DrvSclOsScnprintf(str, end - str, "Test I: Set Config contrast\n");
    str += DrvSclOsScnprintf(str, end - str, "Test J: Set Config uv\n");
    str += DrvSclOsScnprintf(str, end - str, "Test N: Set Config Nr\n");
    str += DrvSclOsScnprintf(str, end - str, "Test n: Set ON Nr\n");
    str += DrvSclOsScnprintf(str, end - str, "========================SCL UNIT TEST======================\n");
    return (str - buf);
}


#endif// for Iq
#endif// for Test
