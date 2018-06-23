#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#include "cam_os_wrapper.h"
#include "MsCommon.h"
#include "apiXC.h"
#include "apiXC_DWIN_v2.h"
#include "mhal_divp_datatype.h"
#include "mhal_divp.h"
#include "mhal_dip.h"
#include "mdrv_divp.h"
#include "divp_proc.h"

static u32 gInitDivpProc = 0;
static s32 g_s32ShowChanInfoId = -1;

struct proc_dir_entry *gpRootDivpDir;
extern u32 gDipDbgLvl;
extern DivpPrivateDevData g_stDipRes[MAX_DIP_WINDOW];
extern MS_BOOL g_bDipMfdecFPEn;
extern MS_U8      g_u8DipMfdecFPData;

#define _XC_RIU_BASE          (0xFD000000)
#define REG_SCALER_BASE             (0x130000UL)
#define REG_MFDEC_BASE                      (0x112200UL)

#define _PK_L_(bank, addr)   (((MS_U16)(bank) << 8) | (MS_U16)((addr)*2))
#define RIU_READ_2BYTE(addr)        ( READ_WORD( _XC_RIU_BASE + (addr) ) )

#define SCRead2BYTE(u32Id, u32Reg )    ( RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) ) << 1 ))

#define Read2Byte( u32Reg )    (RIU_READ_2BYTE((u32Reg)<<1))

void Divp_Proc_Dump_Reg(struct seq_file *m, MS_U16 BankNo, MS_U16 RegOffset, MS_U32 len)
{
    MS_U32 addr, l_offset;
    MS_U8 l_buf[128], token_buf[8];
    MS_U32 start;

    start = _PK_L_(BankNo, RegOffset);

    for (addr = start, l_offset = 0; addr < (start + (len * 2)); addr += 2)
    {
        if ((addr & 0xFF) == 0)
        {
            seq_printf(m, "BK x%02X :\n", (addr >> 8));
        }
        if ((addr & 0xF) == 0)
        {
            l_offset = 0;
            snprintf(token_buf, 8, "%02X:", ((0xFF & addr) >> 1));
            strcpy(l_buf + l_offset, token_buf);
            l_offset += strlen(token_buf);
        }

        snprintf(token_buf, 8, " %04X", SCRead2BYTE(0, addr));
        strcpy(l_buf + l_offset, token_buf);
        l_offset += strlen(token_buf);

        if ((addr & 0xF) == 0xE)
        {
            seq_printf(m, "%s\n", l_buf);
        }
    }
    seq_printf(m, "Dump End\n");
}

void Divp_Proc_MFDEC_Dump_Reg(struct seq_file *m)
{
    MS_U32 addr, l_offset;
    MS_U8 l_buf[128], token_buf[8];
    MS_U32 start;

    start = REG_MFDEC_BASE;

    for (addr = start, l_offset = 0; addr < (start + (0x80 * 2)); addr += 2)
    {
        if ((addr & 0xFF) == 0)
        {
            seq_printf(m, "BK x%04X :\n", (addr >> 8));
        }
        if ((addr & 0xF) == 0)
        {
            l_offset = 0;
            snprintf(token_buf, 8, "%02X:", ((0xFF & addr) >> 1));
            strcpy(l_buf + l_offset, token_buf);
            l_offset += strlen(token_buf);
        }

        snprintf(token_buf, 8, " %04X", Read2Byte(addr));
        strcpy(l_buf + l_offset, token_buf);
        l_offset += strlen(token_buf);

        if ((addr & 0xF) == 0xE)
        {
            seq_printf(m, "%s\n", l_buf);
        }
    }
    seq_printf(m, "Dump End\n");
}

static int DivpDbglvlProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "0: disbale, 1: level 1, 2: level 2, 4: level 3, 7: all\n");
    seq_printf(m, "dbglvl = 0x%08x\n", gDipDbgLvl);
    return 0;
}

static int DivpDbglvlProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, DivpDbglvlProcShow, NULL);
}

static ssize_t DivpDbglvlProcWrite (struct file *file,
                                    const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[] = "0x00000000\n";
    size_t len = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
        return count;
    buf[len] = 0;
    if (sscanf(buf, "%li", &val) != 1)
        printk(": %s is not in hex or decimal form.\n", buf);
    else
        gDipDbgLvl = val;

    return strnlen(buf, len);
}

static const struct file_operations DivpDbglvlProcOps =
{
    .owner		= THIS_MODULE,
    .open		= DivpDbglvlProcOpen,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
    .write		= DivpDbglvlProcWrite,
};

static int DivpDevInfoProcShow(struct seq_file *m, void *v)
{
    int i;
    DivpPrivateDevData *dev = &g_stDipRes[0];

    seq_printf(m, "-- Total DIVP channel num : %d ---------------------------------------------\n", dev->u8NumOpening);

    seq_printf(m, " ==========================================================================\n");
    seq_printf(m, "  Channel   InW   InH   InFt  OutW OutH OutFt   FrmCnt  MFEn DiEn  fps time\n");
    seq_printf(m, " --------------------------------------------------------------------------\n");
    for(i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        if(!dev->ctx[i])
            continue;

        seq_printf(m, "   %2d       %4d  %4d  %2d    %4d %4d %2d    %8d  %d    %d     %2d  %4d\n",
                   dev->ctx[i]->u8ChannelId,
                   dev->ctx[i]->pstInputInfo.u16InputWidth,
                   dev->ctx[i]->pstInputInfo.u16InputHeight,
                   dev->ctx[i]->pstInputInfo.ePxlFmt,
                   dev->ctx[i]->pstOutputInfo.u16OutputWidth,
                   dev->ctx[i]->pstOutputInfo.u16OutputHeight,
                   dev->ctx[i]->pstOutputInfo.ePxlFmt,
                   dev->ctx[i]->u32CurFrameCnt,
                   dev->ctx[i]->pstInputInfo.stMfdecInfo.bDbEnable,
                   dev->ctx[i]->bIsDiEnable,
                   dev->ctx[i]->fps,
                   dev->ctx[i]->time_elapsed);
    }

    seq_printf(m, "-- DivpDevInfoProcShow End ------------------------------------------------\n");
    return 0;
}

static int DivpDevInfoProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, DivpDevInfoProcShow, NULL);
}

static const struct file_operations DivpDevInfoProcOps =
{
    .owner		= THIS_MODULE,
    .open		= DivpDevInfoProcOpen,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
};

static int DivpShowChanInfoProcShow(struct seq_file *m, void *v)
{
    DivpPrivateDevData *dev = &g_stDipRes[0];
    int nChnID = g_s32ShowChanInfoId;
    int i, frm_start;

    if ((nChnID < 0) || (nChnID >= MAX_CHANNEL_NUM))
    {
        seq_printf(m, "Please specify correct channel ID by below command: (0 ~ %d)\n  echo [ChanId] > /proc/divp/chan_info\n", (MAX_CHANNEL_NUM - 1));
    }
    else
    {
        if(!dev->ctx[nChnID])
        {
            seq_printf(m, "!!!!!!!!!! Channel %d was NOT active !!!!!!!!!!\n", nChnID);
        }
        else
        {
            seq_printf(m, "==============================================================\n");
            seq_printf(m, "---------- Show information channel ID: %d  ----------\n", nChnID);
            seq_printf(m, "---------------- Input Information  ----------------\n");
            seq_printf(m, "3DDI=%d, ROT=%d\n", dev->ctx[nChnID]->stAttr.eDiType, dev->ctx[nChnID]->stAttr.eRotateType);
            seq_printf(m, "2P_Enable=%d\n", dev->ctx[nChnID]->pstInputInfo.b2P_Enable);
            seq_printf(m, "DIVP Crop (X, Y, W, H) = (%d, %d, %d, %d)\n", dev->ctx[nChnID]->pstInputInfo.stCropWin.u16X, dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Y,
                       dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Width, dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Height);
            seq_printf(m, "Addr0=0x%x, Addr1=0x%x\n", (MS_U32)(dev->ctx[nChnID]->pstInputInfo.u64BufAddr[0]), (MS_U32)(dev->ctx[nChnID]->pstInputInfo.u64BufAddr[1]));
            seq_printf(m, "    Resolution : %d x %d\n", dev->ctx[nChnID]->pstInputInfo.u16InputWidth, dev->ctx[nChnID]->pstInputInfo.u16InputHeight);
            seq_printf(m, "    Stride=%d, PxlFmt=%d, TileMode= %d\n", dev->ctx[nChnID]->pstInputInfo.u16Stride[0], dev->ctx[nChnID]->pstInputInfo.ePxlFmt, dev->ctx[nChnID]->pstInputInfo.eTileMode);
            if(dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bDbEnable == TRUE)
            {
                seq_printf(m, "MFDEC enable=%d, bypass=%d\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bDbEnable, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bBypassCodecMode);
                seq_printf(m, "    DbMode=%d, DbBaseAddr=0x%x\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.eDbMode, (MS_U32)dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u64DbBaseAddr);
                seq_printf(m, "    MFDEC Crop(X, Y, W, H) =(%d, %d, %d, %d)\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16StartX, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16StartY,
                           dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16Width, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16Height);
            }
            if(dev->ctx[nChnID]->pstInputInfo.eScanMode != E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE)
            {
                seq_printf(m, "3DDI ScanMode=%d, DiMode=%d\n", dev->ctx[nChnID]->pstInputInfo.eScanMode, dev->ctx[nChnID]->pstInputInfo.stDiSettings.eDiMode);
                seq_printf(m, "    FieldType=%d, FieldOrderType=%d\n", dev->ctx[nChnID]->pstInputInfo.stDiSettings.eFieldType, dev->ctx[nChnID]->pstInputInfo.stDiSettings.eFieldOrderType);
                seq_printf(m, "    DiAddr=x%x, DiSize=x%x\n", (int)dev->ctx[nChnID]->u64AllocAddr, dev->ctx[nChnID]->u32AllocSize);
                seq_printf(m, "    == The last %d frame information =============\n", MAX_DEBUG_FRAME_NUM);
                seq_printf(m, "    frm Bob Field BufIdx RdTop RdBot WrTop WrBot \n");
                seq_printf(m, "    ---------------------------------------------\n");
                frm_start = g_stDivpDiDebugInfo[nChnID].u8NextFrame;
                for(i = frm_start; i < (frm_start + MAX_DEBUG_FRAME_NUM); i++)
                {
                    seq_printf(m, "     %d   %d    %s    %d     %d     %d     %d     %d\n",
                               (i % MAX_DEBUG_FRAME_NUM),
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].bBobEn,
                               ((g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8TB_Field == 1) ? "T" : "B"),
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIBufIndex,
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIRdTopIndex,
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIRdBotIndex,
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIWrTopIndex,
                               g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIWrBotIndex);
                }
                seq_printf(m, "    -- 3DDI Info end ----------------------------\n");
            }
            seq_printf(m, "---------------- Output Information  ----------------\n");
            seq_printf(m, "Addr=0x%x\n", (MS_U32)(dev->ctx[nChnID]->pstOutputInfo.u64BufAddr[0]));
            seq_printf(m, "    Resolution : %d x %d\n", dev->ctx[nChnID]->pstOutputInfo.u16OutputWidth, dev->ctx[nChnID]->pstOutputInfo.u16OutputHeight);
            seq_printf(m, "    Stride=%d, PxlFmt=%d\n", dev->ctx[nChnID]->pstOutputInfo.u16Stride[0], dev->ctx[nChnID]->pstOutputInfo.ePxlFmt);
            seq_printf(m, "==============================================================\n");
        }
    }

    return 0;
}

static int DivpShowChanInfoProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, DivpShowChanInfoProcShow, NULL);
}

static ssize_t DivpShowChanInfoProcWrite (struct file *file,
        const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[] = "0x00000000\n";
    size_t len = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
        return count;
    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
        printk(": %s is not in hex or decimal form.\n", buf);
    else
        g_s32ShowChanInfoId = val;

    return strnlen(buf, len);
}

static const struct file_operations DivpShowChanInfoProcOps =
{
    .owner		= THIS_MODULE,
    .open		= DivpShowChanInfoProcOpen,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
    .write		= DivpShowChanInfoProcWrite,
};

static int DivpDumpFinalCmdProcShow(struct seq_file *m, void *v)
{
    DivpPrivateDevData *dev = &g_stDipRes[0];
    int nChnID = dev->u8FinalChannelId;
    int i, frm_start;

    if(!dev->ctx[nChnID])
    {
        seq_printf(m, "!!!!!!!!!! Final Command %d was NOT active !!!!!!!!!!\n", nChnID);
    }
    else
    {
        seq_printf(m, "==============================================================\n");
        seq_printf(m, "---------- Final information channel ID: %d  ----------\n", nChnID);
        seq_printf(m, "---------------- Input Information  ----------------\n");
        seq_printf(m, "3DDI=%d, ROT=%d\n", dev->ctx[nChnID]->stAttr.eDiType, dev->ctx[nChnID]->stAttr.eRotateType);
        seq_printf(m, "2P_Enable=%d\n", dev->ctx[nChnID]->pstInputInfo.b2P_Enable);
        seq_printf(m, "DIVP Crop (X, Y, W, H) = (%d, %d, %d, %d)\n", dev->ctx[nChnID]->pstInputInfo.stCropWin.u16X, dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Y,
                   dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Width, dev->ctx[nChnID]->pstInputInfo.stCropWin.u16Height);
        seq_printf(m, "Addr0=0x%x, Addr1=0x%x\n", (MS_U32)(dev->ctx[nChnID]->pstInputInfo.u64BufAddr[0]), (MS_U32)(dev->ctx[nChnID]->pstInputInfo.u64BufAddr[1]));
        seq_printf(m, "    Resolution : %d x %d\n", dev->ctx[nChnID]->pstInputInfo.u16InputWidth, dev->ctx[nChnID]->pstInputInfo.u16InputHeight);
        seq_printf(m, "    Stride=%d, PxlFmt=%d, TileMode= %d\n", dev->ctx[nChnID]->pstInputInfo.u16Stride[0], dev->ctx[nChnID]->pstInputInfo.ePxlFmt, dev->ctx[nChnID]->pstInputInfo.eTileMode);
        if(dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bDbEnable == TRUE)
        {
            seq_printf(m, "MFDEC enable=%d, bypass=%d\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bDbEnable, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bBypassCodecMode);
            seq_printf(m, "    DbMode=%d, DbBaseAddr=0x%x\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.eDbMode, (MS_U32)dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u64DbBaseAddr);
            seq_printf(m, "    MFDEC Crop(X, Y, W, H) =(%d, %d, %d, %d)\n", dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16StartX, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16StartY,
                       dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16Width, dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.u16Height);
        }
        if(dev->ctx[nChnID]->pstInputInfo.eScanMode != E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE)
        {
            seq_printf(m, "3DDI ScanMode=%d, DiMode=%d\n", dev->ctx[nChnID]->pstInputInfo.eScanMode, dev->ctx[nChnID]->pstInputInfo.stDiSettings.eDiMode);
            seq_printf(m, "    FieldType=%d, FieldOrderType=%d\n", dev->ctx[nChnID]->pstInputInfo.stDiSettings.eFieldType, dev->ctx[nChnID]->pstInputInfo.stDiSettings.eFieldOrderType);
            seq_printf(m, "    DiAddr=x%x, DiSize=x%x\n", (int)dev->ctx[nChnID]->u64AllocAddr, dev->ctx[nChnID]->u32AllocSize);
            seq_printf(m, "    == The last %d frame information =============\n", MAX_DEBUG_FRAME_NUM);
            seq_printf(m, "    frm Bob Field BufIdx RdTop RdBot WrTop WrBot \n");
            seq_printf(m, "    ---------------------------------------------\n");
            frm_start = g_stDivpDiDebugInfo[nChnID].u8NextFrame;
            for(i = frm_start; i < (frm_start + MAX_DEBUG_FRAME_NUM); i++)
            {
                seq_printf(m, "     %d   %d    %s    %d     %d     %d     %d     %d\n",
                           (i % MAX_DEBUG_FRAME_NUM),
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].bBobEn,
                           ((g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8TB_Field == 1) ? "T" : "B"),
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIBufIndex,
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIRdTopIndex,
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIRdBotIndex,
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIWrTopIndex,
                           g_stDivpDiDebugInfo[nChnID].stDebugInfo[i % MAX_DEBUG_FRAME_NUM].u8DIWrBotIndex);
            }
            seq_printf(m, "    -- 3DDI Info end ----------------------------\n");
        }
        seq_printf(m, "---------------- Output Information  ----------------\n");
        seq_printf(m, "Addr=0x%x\n", (MS_U32)(dev->ctx[nChnID]->pstOutputInfo.u64BufAddr[0]));
        seq_printf(m, "    Resolution : %d x %d\n", dev->ctx[nChnID]->pstOutputInfo.u16OutputWidth, dev->ctx[nChnID]->pstOutputInfo.u16OutputHeight);
        seq_printf(m, "    Stride=%d, PxlFmt=%d\n", dev->ctx[nChnID]->pstOutputInfo.u16Stride[0], dev->ctx[nChnID]->pstOutputInfo.ePxlFmt);
        seq_printf(m, "==============================================================\n");

        Divp_Proc_Dump_Reg(m, 0x34, 0x00, 0x80);
        Divp_Proc_Dump_Reg(m, 0x36, 0x00, 0x80);
        if(dev->ctx[nChnID]->pstInputInfo.eScanMode != E_MHAL_DIVP_SCAN_MODE_PROGRESSIVE)
        {
            Divp_Proc_Dump_Reg(m, 0x43, 0x00, 0x80);
            Divp_Proc_Dump_Reg(m, 0x47, 0x00, 0x80);
        }
        Divp_Proc_Dump_Reg(m, 0x49, 0x00, 0x80);

        if(dev->ctx[nChnID]->pstInputInfo.stMfdecInfo.bDbEnable == TRUE)
            Divp_Proc_MFDEC_Dump_Reg(m);

        if(dev->ctx[nChnID]->pstOutputInfo.ePxlFmt == E_MHAL_DIVP_PIXEL_FORMAT_YUV_MST_420)
        {
            Divp_Proc_Dump_Reg(m, 0xF5, 0x00, 0x80);
            Divp_Proc_Dump_Reg(m, 0xF6, 0x00, 0x80);
        }

    }

    return 0;
}

static int DivpDumpFinalCmdProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, DivpDumpFinalCmdProcShow, NULL);
}

static const struct file_operations DivpDumpFinalCmdProcOps =
{
    .owner		= THIS_MODULE,
    .open		= DivpDumpFinalCmdProcOpen,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
};

static int DivpMFDECFixedPatProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "usage: echo [enable] [fp_data(Dec)] > /proc/divp/mfdec_fp\n");
    seq_printf(m, "MfdecFPEn = %d, MfdecFPData = 0x%x\n", g_bDipMfdecFPEn, g_u8DipMfdecFPData);
    return 0;
}

static int DivpMFDECFixedPatProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, DivpMFDECFixedPatProcShow, NULL);
}

static ssize_t DivpMFDECFixedPatProcWrite (struct file *file,
        const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[] = "0x00000000\n";
    size_t len = min(sizeof(buf) - 1, count);
    unsigned long val[2];

    if (copy_from_user(buf, buffer, len))
        return count;
    buf[len] = 0;

    if (sscanf(buf, "%li %li", &val[0], &val[1]) == 2)
    {
        g_bDipMfdecFPEn = (MS_BOOL)(val[0]);
        g_u8DipMfdecFPData = val[1];
    }
    else
        printk(": %s is not in hex or decimal form.\n", buf);

    return strnlen(buf, len);
}

static const struct file_operations DivpMFDECFixedPatProcOps =
{
    .owner		= THIS_MODULE,
    .open		= DivpMFDECFixedPatProcOpen,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
    .write		= DivpMFDECFixedPatProcWrite,
};

void DivpProcInit(void)
{
    struct proc_dir_entry *pde;

    if(gInitDivpProc)
        return;

    printk("%s %d\n", __FUNCTION__, __LINE__);
    gInitDivpProc = 1;

    gpRootDivpDir = proc_mkdir("divp", NULL);
    if (!gpRootDivpDir)
    {
        printk("[DIVP] Can not create proc\n");
        return;
    }

    pde = proc_create("dbglvl", S_IRUGO, gpRootDivpDir, &DivpDbglvlProcOps);
    if (!pde)
        goto out_dbglvl;
    pde = proc_create("dev_info", S_IRUGO, gpRootDivpDir, &DivpDevInfoProcOps);
    if (!pde)
        goto out_dev_info;
    pde = proc_create("chan_info", S_IRUGO, gpRootDivpDir, &DivpShowChanInfoProcOps);
    if (!pde)
        goto out_chan_info;
    pde = proc_create("finalcmd", S_IRUGO, gpRootDivpDir, &DivpDumpFinalCmdProcOps);
    if (!pde)
        goto out_finaldump;
    pde = proc_create("mfdec_fp", S_IRUGO, gpRootDivpDir, &DivpMFDECFixedPatProcOps);
    if (!pde)
        goto out_mfdec_fp;
    return ;

out_mfdec_fp:
    remove_proc_entry("finalcmd", gpRootDivpDir);
out_finaldump:
    remove_proc_entry("chan_info", gpRootDivpDir);
out_chan_info:
    remove_proc_entry("dev_info", gpRootDivpDir);
out_dev_info:
    remove_proc_entry("dbglvl", gpRootDivpDir);
out_dbglvl:
    return ;
}
void DivpProcDeInit(void)
{
    if(!gInitDivpProc)
        return;

    printk("%s %d\n", __FUNCTION__, __LINE__);
    gInitDivpProc = 0;
    remove_proc_entry("dbglvl", gpRootDivpDir);
    remove_proc_entry("dev_info", gpRootDivpDir);
    remove_proc_entry("chan_info", gpRootDivpDir);
    remove_proc_entry("finalcmd", gpRootDivpDir);
    remove_proc_entry("mfdec_fp", gpRootDivpDir);
    remove_proc_entry("divp", NULL);
}
