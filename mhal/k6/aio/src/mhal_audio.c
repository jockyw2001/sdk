#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
///#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include "apiAUDIO.h"
#include "drvAUDIO_if.h"
#include "drvAUDIO.h"
#include "drvSYS.h"

#include <linux/version.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#include <mstar/mstar_chip.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#include <mstar_chip.h>
#else
#error not support this kernel version
#endif

#include "mhal_audio.h"

#define AI_FLAG 0x0
#define AO_FLAG 0x10000
#define AUDIO_HW_DMAREADER_1              "HW DMA Reader1"
#define AUDIO_HW_DMAREADER_2              "HW DMA Reader2"
#define PHYADDR_TO_MIU_OFFSET(phy_addr) \
    (phy_addr >= ARM_MIU2_BASE_ADDR)?((MS_PHY)(phy_addr - ARM_MIU2_BASE_ADDR)):((phy_addr >= (MS_PHY)ARM_MIU1_BASE_ADDR)?((MS_PHY)(phy_addr - ARM_MIU1_BASE_ADDR)):((MS_PHY)(phy_addr - ARM_MIU0_BASE_ADDR)))

/**
*  @file mhal_audio.h
*  @brief audio driver APIs
*/

/**
* \brief Init audio HW
* \return 0 => success, <0 => error number
*/
typedef enum
{
    E_MHAL_AUDIO_SRC_NONE,
    E_MHAL_AUDIO_SRC_SPEAKER,
    E_MHAL_AUDIO_SRC_HP,
    E_MHAL_AUDIO_SRC_LINEOUT,
    E_MHAL_AUDIO_SRC_SCART,
    E_MHAL_AUDIO_SRC_SPDIF,
    E_MHAL_AUDIO_SRC_ARC,
    E_MHAL_AUDIO_SRC_HDMI,
    E_MHAL_AUDIO_SRC_I2SOUT,
    E_MHAL_AUDIO_SRC_MAX
}MHAL_AUDIO_Source_e;

typedef enum
{
    E_MHAL_AUDIO_SRCTYPE_MAIN,
    E_MHAL_AUDIO_SRCTYPE_SUB,
    E_MHAL_AUDIO_SRCTYPE_MAX
}MHAL_AUDIO_SourceType_e;

typedef struct MHAL_AUDIO_Dev_s
{
    struct list_head list;
    MS_U32 u32DevId;
    MHAL_AUDIO_I2sCfg_t stI2sCfg;
    MHAL_AUDIO_PcmCfg_t stPcmCfg;
    MS_S32 s32PcmDevId;
    MS_U8 *pu8PcmDmaPtr;
    MHAL_AUDIO_SourceType_e eSourceType;
    MS_BOOL bStart;
    MS_BOOL bOpen;
}MHAL_AUDIO_Dev_t;
static LIST_HEAD(gAioDevList);
DEFINE_SEMAPHORE(_gstMhalAudioSem);
static MS_BOOL gbSrcType[E_MHAL_AUDIO_SRCTYPE_MAX];

static AUDIO_OUT_INFO gstOutputInfo = {.SpeakerOut = AUDIO_NULL_OUTPUT,
                                       .HpOut = AUDIO_NULL_OUTPUT,
                                       .MonitorOut = AUDIO_AUOUT0_OUTPUT,
                                       .ScartOut = AUDIO_NULL_OUTPUT,
                                       .SpdifOut = AUDIO_NULL_OUTPUT,
                                       .ArcOut = AUDIO_NULL_OUTPUT,
                                       .HDMIOut = AUDIO_HDMI_OUTPUT};

static AUDIO_PATH_INFO gstPathInfo = {.SpeakerOut = AUDIO_PATH_NULL,
                                      .HpOut = AUDIO_PATH_NULL,
                                      .MonitorOut = AUDIO_T3_PATH_AUOUT0,
                                      .ScartOut = AUDIO_PATH_NULL,
                                      .SpdifOut = AUDIO_PATH_NULL,
                                      .ArcOut = AUDIO_PATH_NULL,
                                      .HDMIOut = AUDIO_T3_PATH_HDMI};

static MS_U16 gu16DefaultVolumeTable[] = // Default volume mapping table
{   //    1          2              3               4           5               6               7           8               9           10
    0x7F00, //  00
    0x4700, 0x4400, 0x4100, 0x3E00, 0x3C00, 0x3A00, 0x3800, 0x3600, 0x3400, 0x3200, //  10
    0x3000, 0x2E00, 0x2D00, 0x2C00, 0x2B00, 0x2A00, 0x2900, 0x2800, 0x2700, 0x2600, //  20
    0x2500, 0x2400, 0x2300, 0x2200, 0x2100, 0x2000, 0x1F00, 0x1E40, 0x1E00, 0x1D40, //  30
    0x1D00, 0x1C40, 0x1C00, 0x1B40, 0x1B00, 0x1A40, 0x1A00, 0x1940, 0x1900, 0x1840, //  40
    0x1800, 0x1740, 0x1700, 0x1640, 0x1600, 0x1540, 0x1520, 0x1500, 0x1460, 0x1440, //  50
    0x1420, 0x1400, 0x1360, 0x1340, 0x1320, 0x1300, 0x1260, 0x1240, 0x1220, 0x1200, //  60
    0x1160, 0x1140, 0x1120, 0x1100, 0x1060, 0x1040, 0x1020, 0x1000, 0x0F70, 0x0F60, //  70
    0x0F50, 0x0F40, 0x0F30, 0x0F20, 0x0F10, 0x0F00, 0x0E70, 0x0E60, 0x0E50, 0x0E40, //  80
    0x0E30, 0x0E20, 0x0E10, 0x0E00, 0x0D70, 0x0D60, 0x0D50, 0x0D40, 0x0D30, 0x0D20, //  90
    0x0D10, 0x0D00, 0x0C70, 0x0C60, 0x0C50, 0x0C40, 0x0C30, 0x0C20, 0x0C10, 0x0C00  //  100
};

static MHAL_AUDIO_Dev_t * _MHAL_AUDIO_GetDev(MS_U32 u32DevId)
{
    MHAL_AUDIO_Dev_t *pstAioDev  =NULL;

    list_for_each_entry(pstAioDev, &gAioDevList, list)
    {
        if (u32DevId == pstAioDev->u32DevId)
        {
            break;
        }
    }
    if (&pstAioDev->list == &gAioDevList)
    {
        pstAioDev = kmalloc(sizeof(MHAL_AUDIO_Dev_t), GFP_KERNEL);
        BUG_ON(!pstAioDev);
        memset(pstAioDev, 0, sizeof(MHAL_AUDIO_Dev_t));
        pstAioDev->u32DevId = u32DevId;
        list_add_tail(&pstAioDev->list, &gAioDevList);
    }

    return pstAioDev;
}

static void _MHAL_AUDIO_PcmOpen(MS_U32 u32DevId, MHAL_AUDIO_SourceType_e eSrcType)
{
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    AUDIO_PCM_INFO_t stPcmInfo;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    memset(&stPcmInfo, 0, sizeof(AUDIO_PCM_INFO_t));
    stPcmInfo.u32StructVersion = AUDIO_PCM_INFO_VERSION;
    stPcmInfo.u32StructSize = sizeof(AUDIO_PCM_INFO_t);
    pstAioDev->eSourceType = eSrcType;
    if (pstAioDev->eSourceType == E_MHAL_AUDIO_SRCTYPE_MAIN)
    {
        strcpy((char *)stPcmInfo.u8Name, AUDIO_HW_DMAREADER_1);
    }
    else
    {
        strcpy((char *)stPcmInfo.u8Name, AUDIO_HW_DMAREADER_2);
    }
    stPcmInfo.u32Channel = pstAioDev->stPcmCfg.u16Channels;

    stPcmInfo.u8MultiChFlag = FALSE;
    switch (pstAioDev->stPcmCfg.eRate)
    {
        case E_MHAL_AUDIO_RATE_8K:
        {
            stPcmInfo.u32SampleRate = 8000;
        }
        break;
        case E_MHAL_AUDIO_RATE_16K:
        {
            stPcmInfo.u32SampleRate = 16000;
        }
        break;
        case E_MHAL_AUDIO_RATE_32K:
        {
            stPcmInfo.u32SampleRate = 32000;
        }
        break;
        case E_MHAL_AUDIO_RATE_48K:
        {
            stPcmInfo.u32SampleRate = 48000;
        }
        break;
        default:
             BUG();
             break;
    }
    switch (pstAioDev->stPcmCfg.eWidth)
    {
        case E_MHAL_AUDIO_BITWIDTH_16:
        {
            stPcmInfo.u32BitWidth = 16;
        }
        break;
        case E_MHAL_AUDIO_BITWIDTH_24:
        {
            stPcmInfo.u32BitWidth = 24;
        }
        break;
        default:
            BUG();
            break;
    }
    stPcmInfo.u32BufferDuration = 100;
    //stPcmInfo.u32BufferDuration = 300; //300ms
    stPcmInfo.u32Weighting = 100;
    stPcmInfo.u32Volume = 96;
    stPcmInfo.u8MixingFlag = FALSE;

    printk("[mhal audio]WAV channel %d\n", stPcmInfo.u32Channel);
    printk("[mhal audio]WAV sampleRate %d\n", stPcmInfo.u32SampleRate);
    printk("[mhal audio]WAV bw %d\n", stPcmInfo.u32BitWidth);
    printk("[mhal audio]SRC Type %d\n", eSrcType);
    printk("[mhal audio]Dev id %x\n", u32DevId);

    pstAioDev->s32PcmDevId = MApi_AUDIO_PCM_Open((void*)&stPcmInfo);
    printk("PCM ID %d\n", pstAioDev->s32PcmDevId);
    if (!pstAioDev->stPcmCfg.pu8DmaArea)
    {
        up(&_gstMhalAudioSem);
        printk("pu8DmaArea is NULL!!!!.\n");
        return;
    }
    MApi_AUDIO_PCM_Start(pstAioDev->s32PcmDevId);
    pstAioDev->pu8PcmDmaPtr = pstAioDev->stPcmCfg.pu8DmaArea;
    pstAioDev->bOpen = TRUE;
    up(&_gstMhalAudioSem);


}

static void _MHAL_AUDIO_PcmClose(MS_U32 u32DevId)
{
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (pstAioDev->bOpen)
    {
        MApi_AUDIO_PCM_Stop(pstAioDev->s32PcmDevId);
        MApi_AUDIO_PCM_Close(pstAioDev->s32PcmDevId);
        gbSrcType[pstAioDev->eSourceType] = FALSE;
        pstAioDev->bOpen = FALSE;
    }
    up(&_gstMhalAudioSem);
}
static void _MHAL_AUDIO_SetInternalVolum(MHAL_AUDIO_Source_e eSrc, MS_S16 s16Vol)
{
    MS_U8 u8GainInteger;

    if (s16Vol < -114 || s16Vol > 12)
    {
        printk("error audio volume! %d \n", s16Vol);
    }
    u8GainInteger = 12 -s16Vol; //this is hard code  for audio vol range -114~12db

    switch (eSrc)
    {
        case E_MHAL_AUDIO_SRC_I2SOUT:
        case E_MHAL_AUDIO_SRC_SPEAKER:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.SpeakerOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_HP:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.HpOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_LINEOUT:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.MonitorOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_SCART:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.ScartOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_SPDIF:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.SpdifOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_ARC:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.ArcOut, u8GainInteger, 0);
        }
        break;
        case E_MHAL_AUDIO_SRC_HDMI:
        {
            MApi_AUDIO_SetAbsoluteVolume(gstPathInfo.HDMIOut, u8GainInteger, 0);
        }
        break;
        default:
            BUG();
            break;
    }

}
static void _MHAL_AUDIO_SetMute(MHAL_AUDIO_Source_e eSrc, MS_BOOL bMute)
{
    switch (eSrc)
    {
        case E_MHAL_AUDIO_SRC_I2SOUT:
        case E_MHAL_AUDIO_SRC_SPEAKER:
        {
            MApi_AUDIO_SetMute(gstPathInfo.SpeakerOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_HP:
        {
            MApi_AUDIO_SetMute(gstPathInfo.HpOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_LINEOUT:
        {
            MApi_AUDIO_SetMute(gstPathInfo.MonitorOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_SCART:
        {
            MApi_AUDIO_SetMute(gstPathInfo.ScartOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_SPDIF:
        {
            MApi_AUDIO_SetMute(gstPathInfo.SpdifOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_ARC:
        {
            MApi_AUDIO_SetMute(gstPathInfo.ArcOut, bMute);
        }
        break;
        case E_MHAL_AUDIO_SRC_HDMI:
        {
            MApi_AUDIO_SetMute(gstPathInfo.HDMIOut, bMute);
        }
        break;
        default:
            BUG();
            break;
    }

}
static void _MHAL_AUDIO_SetSrc(MHAL_AUDIO_Source_e eSrc, MHAL_AUDIO_SourceType_e eSrcType)
{
    OUTPUT_SOURCE_INFO stOutputSourceInfo;

    MApi_AUDIO_GetOutputSourceInfo(&stOutputSourceInfo);

    switch (eSrc)
    {
        case E_MHAL_AUDIO_SRC_I2SOUT:
        case E_MHAL_AUDIO_SRC_SPEAKER:
        {
            stOutputSourceInfo.SpeakerOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_HP:
        {
            stOutputSourceInfo.HpOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_LINEOUT:
        {
            stOutputSourceInfo.MonitorOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_SCART:
        {
            stOutputSourceInfo.ScartOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_SPDIF:
        {
            stOutputSourceInfo.SpdifOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_ARC:
        {
            stOutputSourceInfo.ArcOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
        }
        break;
        case E_MHAL_AUDIO_SRC_HDMI:
        {
            stOutputSourceInfo.HDMIOut = (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAIN)?E_CONNECT_MAIN:E_CONNECT_SUB;
            MApi_AUDIO_SetOutputSourceInfo(&stOutputSourceInfo);
            MApi_AUDIO_HDMI_TX_SetMode(HDMI_OUT_PCM);
        }
        break;
        default:
            BUG();
            break;
    }
}

static void _MHAL_AUDIO_ConnectSrc(void)
{
    MApi_AUDIO_SetOutConnectivity();
}

static MHAL_AUDIO_SourceType_e _MHAL_AUDIO_GetSrcType(MHAL_AUDIO_DEV AoutDevId)
{
    MHAL_AUDIO_SourceType_e eRetSrcType = E_MHAL_AUDIO_SRCTYPE_MAX;

    switch(AoutDevId)
    {
        case 0: //lineout
        case 1: //i2sout
        case 2: //hdmiout
        {
            if (gbSrcType[E_MHAL_AUDIO_SRCTYPE_MAIN] == FALSE)
            {
                eRetSrcType = E_MHAL_AUDIO_SRCTYPE_MAIN;
                gbSrcType[E_MHAL_AUDIO_SRCTYPE_MAIN] = TRUE;
            }
            else if (gbSrcType[E_MHAL_AUDIO_SRCTYPE_SUB] == FALSE)
            {
                eRetSrcType = E_MHAL_AUDIO_SRCTYPE_SUB;
                gbSrcType[E_MHAL_AUDIO_SRCTYPE_SUB] = TRUE;
            }
        }
        break;
        case 3:     //ALL path
        {
            if (gbSrcType[E_MHAL_AUDIO_SRCTYPE_MAIN] == FALSE)
            {
                eRetSrcType = E_MHAL_AUDIO_SRCTYPE_MAIN;
                gbSrcType[E_MHAL_AUDIO_SRCTYPE_MAIN] = TRUE;
            }
        }
        break;
        default:
            eRetSrcType = E_MHAL_AUDIO_SRCTYPE_MAX;
            break;
    }
    return eRetSrcType;
}
static char * _MHAL_AUDIO_ReadLine(MS_U8 *buf, MS_U32 len, struct file *fp)
{
    int ret = 0;
    int i = 0;

    ret = fp->f_op->read(fp, buf, len, &(fp->f_pos));
    if (ret <= 0)
    {
        return NULL;
    }

    while(i < ret)
    {
        if (buf[i++] == '\n')
        {
            break;
        }
    }

    if (i < ret)
    {
        fp->f_op->llseek(fp, (i - ret), SEEK_CUR);
    }

    if(i < len)
    {
        buf[i] = 0;
    }

    return buf;
}

static int _MHAL_AUDIO_GetMiu(MS_U8 *dst, MS_U8 *value, struct file *fp)
{
    char buf[512];
    char *pos = NULL;
    while ( (pos = _MHAL_AUDIO_ReadLine(buf, sizeof(buf), fp)) )
    {
        char *p = strstr(pos, dst);
        if (!p)
        {
            continue;
        }

        pos = p;
        p = strstr(pos, "MIU0");
        if (p)
        {
            *value = 0;
            return 1;
        }

        p = strstr(pos, "MIU1");
        if (p)
        {
            *value = 1;
            return 1;
        }

        p = strstr(pos, "MIU2");
        if (p)
        {
            *value = 2;
            return 1;
        }
    }

    return 0;
}

static int _MHAL_AUDIO_GetValue(MS_U8 *dst, MS_U32 *value, struct file *fp)
{
    char buf[512];
    char *pos = NULL;
    while ( (pos = _MHAL_AUDIO_ReadLine(buf, sizeof(buf), fp)) )
    {
        char *p = NULL;
        if ( (p = strstr(pos, dst)) )
        {
            char *start = NULL, *end = NULL;
            start = strstr(p, "0x");
            if (!start)
            {
                pos = NULL;
                break;
            }

            end = strchr(start, ' ');
            if (!end)
            {
                end = strchr(start, '\n');
            }

            if (!end)
            {
                end = strchr(start, '/');
            }

            if (!end)
            {
                pos = NULL;
                break;
            }

            *end = 0;
            *value = simple_strtol(start, NULL, 16);
            return 1;
        }
        else
        {
            continue;
        }
    }

    return 0;
}

static MS_S32 _MHAL_AUDIO_MmapParser(MS_U8 *name, MS_U32 *addr, MS_U32 *size, MS_U8 *miu)
{
    struct file *fp = NULL;
    mm_segment_t fs;
    MS_S32 ret = MHAL_FAILURE;
    MS_U8 targe[64];

    if ((strlen(name) + strlen("_MEMORY_TYPE")) >= sizeof(targe))
    {
        printk("name over flow\n");
        return 0;
    }

    fp =filp_open("/config/mmap.ini",O_RDONLY,0644);
    if (IS_ERR(fp))
    {
        return 0;
    }

    fs =get_fs();
    set_fs(KERNEL_DS);
    ///N+1 m, all cpu address and size
    fp->f_op->llseek(fp, 0, SEEK_SET);
    sprintf(targe, "%s_ADR", name);
    if (!_MHAL_AUDIO_GetValue(targe, addr, fp))
    {
        printk("Can't Find ADDR\n");
        goto _end;
    }

    sprintf(targe, "%s_LEN", name);
    if (!_MHAL_AUDIO_GetValue(targe, size, fp))
    {
        printk("Can't Find LEN\n");
        goto _end;
    }

    sprintf(targe, "%s_MEMORY_TYPE", name);
    if (!_MHAL_AUDIO_GetMiu(targe, miu, fp))
    {
        printk("Can't Find MIU\n");
        goto _end;
    }

    ret = MHAL_SUCCESS;
_end:
    set_fs(fs);
    filp_close(fp,NULL);
    return ret;
}
///mmap parser end

MS_S32 MHAL_AUDIO_Init(void *pdata)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U32 u32Addr = 0;
    MS_U32 u32Size = 0;
    MS_U8 u8Miu = 0;
    MHAL_AUDIO_MmapCfg_t *pstMmapConfig = (MHAL_AUDIO_MmapCfg_t *)pdata;

    MsOS_Init();
    MsOS_MPool_Init();
    MDrv_SYS_GlobalInit();

    u32Addr = pstMmapConfig->u32Addr;
    u32Size = pstMmapConfig->u32Size;
    u8Miu   = pstMmapConfig->u8MiuNo;
    printk("[MHAL AUDIO]:Parse mmap success! addr %x, size %x, miu %d\n", u32Addr, u32Size, u8Miu);

    if(!MsOS_MPool_Mapping_Dynamic(u8Miu, PHYADDR_TO_MIU_OFFSET(u32Addr), u32Size, TRUE))
    {
        printk("MsOS_MPool_Mapping_Dynamic E_MMAP_ID_MAD_R2_ADDR fail!\n");
        if(!MsOS_MPool_Mapping_Dynamic(u8Miu, PHYADDR_TO_MIU_OFFSET(u32Addr), u32Size, FALSE))
        {
            printk("MsOS_MPool_Mapping_Dynamic E_MMAP_ID_MAD_R2_ADDR fail!\n");
        }
    }

    MApi_AUDIO_WritePreInitTable();
    //======================================
    // Set output info
    //======================================
    MApi_AUDIO_SetOutputInfo(&gstOutputInfo);
    //======================================
    // Set path info
    //======================================
    MApi_AUDIO_SetPathInfo(&gstPathInfo);
    MDrv_AUDIO_SetDspBaseAddr(DSP_ADV, 0 , u32Addr); //addr E_MMAP_ID_MAD_R2. need refine

    MApi_AUDIO_Initialize();
#if 0

    MApi_AUDIO_InputSwitch(AUDIO_I2S_INPUT, E_AUDIO_GROUP_SUB);  //Capture CH5
#else
    //MApi_AUDIO_InputSwitch(AUDIO_I2S_INPUT, E_AUDIO_GROUP_MAIN);  //Capture CH5

    // Mic In
    MApi_AUDIO_InputSwitch(AUDIO_AUMIC_INPUT, E_AUDIO_GROUP_MAIN);  //Capture CH5
    // Line In
    //MApi_AUDIO_InputSwitch(AUDIO_AUIN0_INPUT, E_AUDIO_GROUP_MAIN);  //Capture CH5
    MApi_AUDIO_InputSwitch(AUDIO_DSP4_DMARD_INPUT, E_AUDIO_GROUP_SUB); //reader2 CH7 & reader1 CH8

    MApi_AUDIO_SetMute(AUDIO_PATH_4, TRUE);

    //mute Capture CH5 , if not, the Capture data will mix to CH8
    MApi_AUDIO_SetMute(AUDIO_T3_PATH_MIXER_MAIN, TRUE);

#endif
    _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_LINEOUT, TRUE);
    _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_I2SOUT, TRUE);
    _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_HDMI, TRUE);

    return s32Ret;
}

/**
* \brief Config audio I2S output device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ConfigI2sOut(MHAL_AUDIO_DEV AoutDevId, MHAL_AUDIO_I2sCfg_t *pstI2sConfig)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    AUDIO_I2S_MODE_VALUE eValue = I2S_FORMAT_STANDARD;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    BUG_ON(!pstI2sConfig);

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    up(&_gstMhalAudioSem);
    BUG_ON(!pstAioDev);

    memcpy(&pstAioDev->stI2sCfg, pstI2sConfig, sizeof(MHAL_AUDIO_I2sCfg_t));

    switch (pstI2sConfig->eFmt)
    {
        case E_MHAL_AUDIO_I2S_FMT_I2S:
        {
            eValue = I2S_FORMAT_STANDARD;
        }
        break;
        case E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY:
        {
            eValue = I2S_FORMAT_LEFT_JUSTIFIED;
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_RXMODE, eValue);

    switch (pstI2sConfig->eMode)
    {
        case E_MHAL_AUDIO_MODE_I2S_MASTER:
        {
            eValue = I2S_MASTER_MODE;
        }
        break;
        case E_MHAL_AUDIO_MODE_I2S_SLAVE:
        {
            eValue = I2S_SLAVE_MODE;
        }
        break;
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
        {
            ;//Fix me
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_TXMODE, eValue);

    switch (pstI2sConfig->eWidth)
    {
        case E_MHAL_AUDIO_BITWIDTH_16:
        {
            eValue = I2S_WORD_WIDTH_16BIT;
        }
        break;
        case E_MHAL_AUDIO_BITWIDTH_24:
        {
            eValue = I2S_WORD_WIDTH_24BIT;
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_WORD_WIDTH, eValue);

    return s32Ret;
}

/**
* \brief Config audio I2S input device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ConfigI2sIn(MHAL_AUDIO_DEV AinDevId, MHAL_AUDIO_I2sCfg_t *pstI2sConfig)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    AUDIO_I2S_MODE_VALUE eValue = I2S_MASTER_MODE;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    BUG_ON(!pstI2sConfig);


    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    up(&_gstMhalAudioSem);
    BUG_ON(!pstAioDev);

    memcpy(&pstAioDev->stI2sCfg, pstI2sConfig, sizeof(MHAL_AUDIO_I2sCfg_t));

    switch (pstI2sConfig->eMode)
    {
        case E_MHAL_AUDIO_MODE_I2S_MASTER:
        {
            eValue = I2S_MASTER_MODE;
        }
        break;
        case E_MHAL_AUDIO_MODE_I2S_SLAVE:
        {
            eValue = I2S_SLAVE_MODE;
        }
        break;
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
        {
            ;//Fix me
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_RXMODE, eValue);

    switch (pstI2sConfig->eFmt)
    {
        case E_MHAL_AUDIO_I2S_FMT_I2S:
        {
            eValue = I2S_FORMAT_STANDARD;
        }
        break;
        case E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY:
        {
            eValue = I2S_FORMAT_LEFT_JUSTIFIED;
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_RX_JUSTIFIED_MODE, eValue);

    switch (pstI2sConfig->eWidth)
    {
        case E_MHAL_AUDIO_BITWIDTH_16:
        {
            eValue = I2S_WORD_WIDTH_16BIT;
        }
        break;
        case E_MHAL_AUDIO_BITWIDTH_24:
        {
            eValue = I2S_WORD_WIDTH_24BIT;
        }
        break;
        default:
            BUG();
            break;
    }
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_RX_WORD_WIDTH, eValue);
    MApi_AUDIO_I2S_SetMode(AUDIO_I2S_RX_CLOCK, I2S_SAMPLE_RATE_48K);

    return s32Ret;
}

/**
* \brief Config audio PCM output device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ConfigPcmOut(MHAL_AUDIO_DEV AoutDevId, MHAL_AUDIO_PcmCfg_t *ptDmaConfig)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    BUG_ON(!ptDmaConfig);

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);

    memcpy(&pstAioDev->stPcmCfg, ptDmaConfig, sizeof(MHAL_AUDIO_PcmCfg_t));
    up(&_gstMhalAudioSem);

    return s32Ret;
}

/**
* \brief Config audio PCM input device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ConfigPcmIn(MHAL_AUDIO_DEV AinDevId, MHAL_AUDIO_PcmCfg_t *ptDmaConfig)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    BUG_ON(!ptDmaConfig);

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);

    memcpy(&pstAioDev->stPcmCfg, ptDmaConfig, sizeof(MHAL_AUDIO_PcmCfg_t));
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Open audio PCM output device, should be called after configPcmOut
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_OpenPcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U32 u32DevId = 0;
    MHAL_AUDIO_SourceType_e eSrcType = E_MHAL_AUDIO_SRCTYPE_MAX;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;

    eSrcType = _MHAL_AUDIO_GetSrcType(AoutDevId);
    if (eSrcType == E_MHAL_AUDIO_SRCTYPE_MAX)
    {
        printk("[mhal audio]Resource busy!!\n");
        return MHAL_FAILURE;
    }
    _MHAL_AUDIO_PcmOpen(u32DevId, eSrcType);
    switch(AoutDevId)
    {
        case 0: //lineout
        {
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_LINEOUT, eSrcType);
            _MHAL_AUDIO_ConnectSrc();
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_LINEOUT, FALSE);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_LINEOUT, 0);
        }
        break;
        case 1: //i2sout
        {
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_I2SOUT, eSrcType);
            _MHAL_AUDIO_ConnectSrc();
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_I2SOUT, FALSE);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_I2SOUT, 0);
        }
        break;
        case 2: //hdmiout
        {
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_HDMI, eSrcType);
            _MHAL_AUDIO_ConnectSrc();
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_HDMI, FALSE);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_HDMI, 0);
        }
        break;
        case 3:     //ALL path
        {
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_LINEOUT, eSrcType);
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_I2SOUT, eSrcType);
            _MHAL_AUDIO_SetSrc(E_MHAL_AUDIO_SRC_HDMI, eSrcType);
            _MHAL_AUDIO_ConnectSrc();
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_LINEOUT, FALSE);
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_I2SOUT, FALSE);
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_HDMI, FALSE);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_LINEOUT, 0);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_I2SOUT, 0);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_HDMI, 0);
        }
        break;
        default:
            printk("Devide id is error %d!!\n", AoutDevId);
            s32Ret = MHAL_FAILURE;
            break;
    }

    return s32Ret;
}


/**
* \brief Open audio PCM input device, should be called after configPcmIn
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_OpenPcmIn(MHAL_AUDIO_DEV AinDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U32 u32DevId = 0;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);

    switch (AinDevId)
    {
        case 0:
        {
            MApi_AUDIO_SPDIF_ChannelStatus_CTRL(SPDIF_CS_CategoryCode, SPDIF_CS_Category_General);
            if (MApi_AUDIO_PCMCapture_Init(E_DEVICE0, E_CAPTURE_CH5) != TRUE)
            {
                printk("MApi_AUDIO_PCMCapture_Init fail !\n");
                up(&_gstMhalAudioSem);

                return MHAL_FAILURE;
            }
            pstAioDev->u32DevId = E_DEVICE0;
        }
        break;
        default:
            printk("Error device id !\n");
            break;
    }
    pstAioDev->bOpen = TRUE;

    up(&_gstMhalAudioSem);

    return s32Ret;
}

/**
* \brief Close audio PCM output device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ClosePcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U32 u32DevId = 0;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    up(&_gstMhalAudioSem);

    switch(AoutDevId)
    {
        case 0: //lineout
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_LINEOUT, TRUE);
            break;
        case 1: //i2sout
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_I2SOUT, TRUE);
            break;
        case 2: //hdmiout
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_HDMI, TRUE);
            break;
        case 3:     //ALL path
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_LINEOUT, TRUE);
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_I2SOUT, TRUE);
            _MHAL_AUDIO_SetMute(E_MHAL_AUDIO_SRC_HDMI, TRUE);
            break;
        default:
            return MHAL_FAILURE;
            break;
    }
    _MHAL_AUDIO_PcmClose(pstAioDev->u32DevId);

    return s32Ret;
}

/**
* \brief Close audio PCM input device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ClosePcmIn(MHAL_AUDIO_DEV AinDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MS_U32 u32DevId = 0;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (pstAioDev->bOpen)
    {
        pstAioDev->bOpen = FALSE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}

/**
* \brief Start audio PCM output device, start playback
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_StartPcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;
    MS_U32 u32WriteSize = 0;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (!pstAioDev->bStart)
    {
        u32WriteSize = pstAioDev->pu8PcmDmaPtr - pstAioDev->stPcmCfg.pu8DmaArea;
        if (u32WriteSize)
        {
            MApi_AUDIO_PCM_Write(pstAioDev->s32PcmDevId, pstAioDev->stPcmCfg.pu8DmaArea, u32WriteSize);
            pstAioDev->pu8PcmDmaPtr = pstAioDev->stPcmCfg.pu8DmaArea;
        }
        pstAioDev->bStart = TRUE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Start audio PCM input device, start recording
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_StartPcmIn(MHAL_AUDIO_DEV AinDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (!pstAioDev->bStart)
    {
        if (MApi_AUDIO_PCMCapture_Start(pstAioDev->u32DevId) != TRUE)
        {
            printk("MApi_AUDIO_PCMCapture_Start fail !\n");
            return MHAL_FAILURE;
        }
        pstAioDev->bStart = TRUE;
    }
    up(&_gstMhalAudioSem);


    return s32Ret;
}


/**
* \brief Stop audio PCM output device, stop playback
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_StopPcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (pstAioDev->bStart)
    {
        pstAioDev->bStart = FALSE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Stop audio PCM input device, stop recording
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_StopPcmIn(MHAL_AUDIO_DEV AinDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (pstAioDev->bStart)
    {
        if (MApi_AUDIO_PCMCapture_Stop(pstAioDev->u32DevId) != TRUE)
        {
            printk("MApi_AUDIO_PCMCapture_Stop fail !\n");
            return MHAL_FAILURE;
        }
        pstAioDev->bStart = FALSE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Pause audio PCM output device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_PausePcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Resume audio PCM output device
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_ResumePcmOut(MHAL_AUDIO_DEV AoutDevId)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    up(&_gstMhalAudioSem);

    return s32Ret;
}


/**
* \brief Read audio raw data from PCM input device(period unit)
* \return value>0 => read data bytes, value<0 => error number
*/
MS_S32 MHAL_AUDIO_ReadDataIn(MHAL_AUDIO_DEV AinDevId, VOID *pRdBuffer, MS_U32 u32Size, MS_BOOL bBlock)
{
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    BUG_ON(!pRdBuffer);

    u32DevId = (AinDevId & 0xFFFF) | AI_FLAG;

    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (MApi_AUDIO_PCMCapture_Read(pstAioDev->u32DevId, pRdBuffer, u32Size) != TRUE)
    {
        up(&_gstMhalAudioSem);

        //Because it is read in a polling mode, so can't read the data often.
        //return 0 means no data can get

        return 0;
       // return MHAL_FAILURE;
    }
    up(&_gstMhalAudioSem);

    return u32Size;
}

/**
* \brief Write audio raw data to PCM output device(period unit)
* \return value>0 => write data bytes, value<0 => error number
*/
MS_S32  MHAL_AUDIO_WriteDataOut(MHAL_AUDIO_DEV AoutDevId, VOID *pWrBuffer, MS_U32 u32Size, MS_BOOL bBlock)
{
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;
    MS_U32 u32WriteSize = 0;

    BUG_ON(!pWrBuffer);
    if (!u32Size)
    {
        printk("Size is zero!!!\n");
        return MHAL_FAILURE;
    }
    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    if (!pstAioDev->stPcmCfg.pu8DmaArea)
    {
        up(&_gstMhalAudioSem);
        printk("pu8DmaArea not config write buffer failed!!!\n");
        return MHAL_FAILURE;
    }
    if (pstAioDev->bStart)
    {
        u32WriteSize = MApi_AUDIO_PCM_Write(pstAioDev->s32PcmDevId, pWrBuffer, u32Size);
    }
    else
    {
        if (pstAioDev->pu8PcmDmaPtr + u32Size > pstAioDev->stPcmCfg.pu8DmaArea + pstAioDev->stPcmCfg.u32BufferSize)
        {
            u32WriteSize = pstAioDev->pu8PcmDmaPtr - pstAioDev->stPcmCfg.pu8DmaArea;
            u32WriteSize = MApi_AUDIO_PCM_Write(pstAioDev->s32PcmDevId, pstAioDev->stPcmCfg.pu8DmaArea, u32WriteSize);
            //printk("write dev %d size %d\n", pstAioDev->s32PcmDevId, u32WriteSize);
            pstAioDev->pu8PcmDmaPtr = pstAioDev->stPcmCfg.pu8DmaArea;
            memcpy(pstAioDev->pu8PcmDmaPtr, pWrBuffer, u32Size);
            pstAioDev->pu8PcmDmaPtr += u32Size;
        }
        else
        {
            memcpy(pstAioDev->pu8PcmDmaPtr, pWrBuffer, u32Size);
            pstAioDev->pu8PcmDmaPtr += u32Size;
        }
        u32WriteSize = u32Size;
    }
    up(&_gstMhalAudioSem);

    return u32WriteSize;
}


/**
* \brief Set output path gain
* \return 0 => success, <0 => error number
*/
MS_S32 MHAL_AUDIO_SetGainOut(MHAL_AUDIO_DEV AoutDevId, MS_S16 s16Gain)
{
    MS_S32 s32Ret = MHAL_SUCCESS;
    MHAL_AUDIO_Dev_t *pstAioDev = NULL;
    MS_U32 u32DevId = 0;

    u32DevId = (AoutDevId & 0xFFFF) | AO_FLAG;
    down(&_gstMhalAudioSem);
    pstAioDev = _MHAL_AUDIO_GetDev(u32DevId);
    BUG_ON(!pstAioDev);
    if (!pstAioDev->bOpen)
    {
        printk("[mhal audio] pcm out do not open!\n");
        up(&_gstMhalAudioSem);

        return MHAL_FAILURE;
    }
    up(&_gstMhalAudioSem);

    switch(AoutDevId)
    {
        case 0: //lineout
        {
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_LINEOUT, s16Gain);
        }
        break;
        case 1: //i2sout
        {
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_I2SOUT, s16Gain);
        }
        break;
        case 2: //hdmiout
        {
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_HDMI, s16Gain);
        }
        break;
        case 3:     //ALL path
        {
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_LINEOUT, s16Gain);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_I2SOUT, s16Gain);
            _MHAL_AUDIO_SetInternalVolum(E_MHAL_AUDIO_SRC_HDMI, s16Gain);
        }
        break;
        default:
            printk("Devide id is error %d!!\n", AoutDevId);
            s32Ret = MHAL_FAILURE;
            break;
    }
    return s32Ret;
}

MS_BOOL MHAL_AUDIO_IsPcmOutXrun(MHAL_AUDIO_DEV AoutDevId)
{
    MS_BOOL bRet=FALSE;

    return bRet;
}

MS_BOOL MHAL_AUDIO_IsPcmInXrun(MHAL_AUDIO_DEV AinDevId)
{
    MS_BOOL bRet=FALSE;

    return bRet;
}

EXPORT_SYMBOL(MHAL_AUDIO_Init);
EXPORT_SYMBOL(MHAL_AUDIO_ConfigI2sOut);
EXPORT_SYMBOL(MHAL_AUDIO_ConfigI2sIn);
EXPORT_SYMBOL(MHAL_AUDIO_ConfigPcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_ConfigPcmIn);
EXPORT_SYMBOL(MHAL_AUDIO_OpenPcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_OpenPcmIn);
EXPORT_SYMBOL(MHAL_AUDIO_ClosePcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_ClosePcmIn);
EXPORT_SYMBOL(MHAL_AUDIO_StartPcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_StartPcmIn);
EXPORT_SYMBOL(MHAL_AUDIO_StopPcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_StopPcmIn);
EXPORT_SYMBOL(MHAL_AUDIO_PausePcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_ResumePcmOut);
EXPORT_SYMBOL(MHAL_AUDIO_ReadDataIn);
EXPORT_SYMBOL(MHAL_AUDIO_WriteDataOut);
EXPORT_SYMBOL(MHAL_AUDIO_SetGainOut);
EXPORT_SYMBOL(MHAL_AUDIO_IsPcmOutXrun);
EXPORT_SYMBOL(MHAL_AUDIO_IsPcmInXrun);

