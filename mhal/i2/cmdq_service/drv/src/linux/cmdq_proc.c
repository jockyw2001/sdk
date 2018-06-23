#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#include "cam_os_wrapper.h"
#include "drv_cmdq.h"
#include "hal_cmdq_dbg.h"
#include "hal_cmdq.h"
#include "cmdq_proc.h"

u32    gInitCmdqProc = 0;
u32    gCmdqDumEngine = 0;

struct proc_dir_entry *gpRootCmdqDir;
extern u32 gCmdqDbgLvl;
extern CmdqInterfacePriv_t gDrvCmdPriv[EN_CMDQ_TYPE_MAX];
extern REG_CMDQCtrl             *_CMDQCtrl[NUMBER_OF_CMDQ_HW];

extern u32 _MDrvCmdqTransPhyBuf2VirBuf(CmdqInterfacePriv_t* pPrivData, u32 PhyBufAddr);
extern void HAL_CMDQ_SetISRMSK(HALCmdqIPSupport_e eIpNum, u32 nMakVal);
extern u16 HAL_CMDQ_GetISRMSK(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///concatenate (reg)->H and (reg)->L
/// @param  reg                     \b IN: concatenate data
//---------------------------------------------------------------------------
u32 CmdqProcReg32R(REG32 *reg)
{
    u32     value;
    value = (reg)->H << 16;
    value |= (reg)->L;
    return value;
}

static int CmdqDbglvlProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "0:disbale 1: lv1 2:lv2 4:lv3 7:all\n");
	seq_printf(m, "dbglvl = 0x%08x\n", gCmdqDbgLvl);
	return 0;
}

static int CmdqDbglvlProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, CmdqDbglvlProcShow, NULL);
}

static ssize_t CmdqDbglvlProcWrite (struct file *file,
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
		gCmdqDbgLvl = val;

	return strnlen(buf, len);
}

static const struct file_operations CmdqDbglvlProcOps = {
	.owner		= THIS_MODULE,
	.open		= CmdqDbglvlProcOpen,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= CmdqDbglvlProcWrite,
};

static void CmdqDumpHalRegProcShow(struct seq_file *m, void *v,u32 eIpNum)
{
    seq_printf(m,"*************************************************************\n");

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Enable, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Enable));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Mode));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_St_Ptr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_End_Ptr));

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Wr_Mi_Wadr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Rd_Mi_Radr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Buf_Radr_Wadr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Buf_Radr_Wadr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_0F_0E, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_0F_0E));

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Length_ReadMode));
    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_1f_12,CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_1f_12));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Mask_Setting));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Wait_Trig));

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Force_Skip));
    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_27_26,CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_27_26));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Tout_Base_Amount));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Mode_Enable));

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_One_Step_Trig));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Replace_Dat2_3, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Replace_Dat2_3));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_En_Clk_Miu));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Debug_Sel, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Debug_Sel));

    //printk("0x%08x-0x%08x\n",(u32)&_CMDQCtrl[eIpNum]->CMDQ_3f_34,CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_3f_34));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Dma_State_Rb_Cmd));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Sel_Decode_State));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Raw_Irq_Final_Irq));

    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Irq_Mask_Irq_Force));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Sw_Trig_Cap_Sel_Irq_Clr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Poll_Ratio_Wait_Time));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Cmd_Ptr_Vld));

    seq_printf(m,"tb final irq = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Final_Irq, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Final_Irq));
    seq_printf(m,"tb irq force = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Force_Mask, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Force_Mask));
    seq_printf(m,"tb irq clr = 0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Irq_Clr, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Trig_Bus_Irq_Clr));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_Hw_dummy));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_dummy));
    seq_printf(m,"0x%08x-0x%08x\n", (u32)&_CMDQCtrl[eIpNum]->CMDQ_dummy, CmdqProcReg32R(&_CMDQCtrl[eIpNum]->CMDQ_dummy));
}

static int CmdqDumpCtxProcShow(struct seq_file *m, void *v)
{
	int i,j;
    u32 s, e, r, w;

    seq_printf(m,"-- Total %d CMDQ HW Ver:%s---------------------------------------------\n", NUMBER_OF_CMDQ_HW,DRVCMDQ_VERSION);
    for(i = 0; i < NUMBER_OF_CMDQ_HW; i++)
    {
        if(!gDrvCmdPriv[i].bUsed)
            continue;

        seq_printf(m," ====================================================== CMDQ #%d ===============\n", i);
        seq_printf(m," Mode: (%d-%d-%d)DRVID(%d-%d)\n",
                 gDrvCmdPriv[i].nCmdMode, gDrvCmdPriv[i].bForceRIU, gDrvCmdPriv[i].bUsed, gDrvCmdPriv[i].CmdqIpId, gDrvCmdPriv[i].nCmdqDrvIpId);

        seq_printf(m," Basis: miu        size         phyadr -     viradr \n");
        seq_printf(m,"        0x%8x      0x%8x        0x%08X - 0x%08X \n",
                 gDrvCmdPriv[i].nCmdqRiuBase, gDrvCmdPriv[i].tCmdqBuf.nBufSize
                 , gDrvCmdPriv[i].tCmdqBuf.BufPhy, gDrvCmdPriv[i].tCmdqBuf.BufVir);

        {

            s = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufStart;
            e = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufEnd;
            w = (u32)gDrvCmdPriv[i].tCmdqInfo.pBufWrite;
            r = HAL_CMDQ_Read_Pointer(i);
            seq_printf(m,"s,e,w,r: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", s, e, w, r);
            seq_printf(m,"fire_w,total_cnt,current_cnt: 0x%08x,%d,%d\n", (u32)gDrvCmdPriv[i].tCmdqInfo.pBufFireWriteHw
                     , gDrvCmdPriv[i].tCmdqInfo.nTotalCmdCount, gDrvCmdPriv[i].tCmdqInfo.nCurCmdCount);
        }

        seq_printf(m," Mload Basis: miu        size         phyadr -     viradr \n");
        seq_printf(m,"              0x%08x     0x%8x        0x%08X - 0x%08X  \n",
                 gDrvCmdPriv[i].nCmdqRiuBase, gDrvCmdPriv[i].tMloadBuf.nBufSize,
                 gDrvCmdPriv[i].tMloadBuf.BufPhy, gDrvCmdPriv[i].tMloadBuf.BufVir);

        {

            s = (u32)gDrvCmdPriv[i].tMloadInfo.pBufStart;
            e = (u32)gDrvCmdPriv[i].tMloadInfo.pBufEnd;
            w = (u32)gDrvCmdPriv[i].tMloadInfo.pBufWrite;
            r = (u32)gDrvCmdPriv[i].tMloadInfo.pBufReadHw;
            seq_printf(m,"Mload s,e,w,r: 0x%8x, 0x%8x, 0x%8x, 0x%8x\n", s, e, w, r);

        }

        CmdqDumpHalRegProcShow(m,v,i);

        seq_printf(m,"-- CMDQ forbidden tag ----------------------------------------------------------\n");

        for(j=0;j<DRVCMD_FORBIDDENTAG_NUM;j++)
        {
            if(gDrvCmdPriv[i].nForbiddenTag[j]==0)
                break;
            if(j%16 ==0)
            {
                seq_printf(m,"forbidden tag=0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x\n"
                           ,gDrvCmdPriv[i].nForbiddenTag[j],gDrvCmdPriv[i].nForbiddenTag[j+1],gDrvCmdPriv[i].nForbiddenTag[j+2],gDrvCmdPriv[i].nForbiddenTag[j+3]
                           ,gDrvCmdPriv[i].nForbiddenTag[j+4],gDrvCmdPriv[i].nForbiddenTag[j+5],gDrvCmdPriv[i].nForbiddenTag[j+6],gDrvCmdPriv[i].nForbiddenTag[j+7]
                           ,gDrvCmdPriv[i].nForbiddenTag[j+8],gDrvCmdPriv[i].nForbiddenTag[j+9],gDrvCmdPriv[i].nForbiddenTag[j+10],gDrvCmdPriv[i].nForbiddenTag[j+11]
                           ,gDrvCmdPriv[i].nForbiddenTag[j+12],gDrvCmdPriv[i].nForbiddenTag[j+13],gDrvCmdPriv[i].nForbiddenTag[j+14],gDrvCmdPriv[i].nForbiddenTag[j+15]);
            }
        }
    }

    seq_printf(m,"-- CMDQ Dump Context ----------------------------------------------------------\n");
	return 0;
}

static int CmdqDumpCtxProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, CmdqDumpCtxProcShow, NULL);
}

static const struct file_operations CmdqDumpCtxProcOps = {
	.owner		= THIS_MODULE,
	.open		= CmdqDumpCtxProcOpen,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int CmdqDumpFinalCmdProcShow(struct seq_file *m, void *v)
{
    CmdqInterfacePriv_t * pPrivData;
    u32 Command_15_0_bit, Command_31_16_bit, Command_55_32_bit, Command_63_56_bit;
    u32 i;
    for(i = 0; i < NUMBER_OF_CMDQ_HW; i++)
    {
        if(!gDrvCmdPriv[i].bUsed)
            continue;

        pPrivData = (CmdqInterfacePriv_t * )&gDrvCmdPriv[i];
        Command_15_0_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_15_0_BIT);
        Command_31_16_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_31_16_BIT);
        Command_55_32_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_55_32_BIT);
        Command_63_56_bit = HAL_CMDQ_Error_Command(pPrivData->nCmdqDrvIpId, CMDQ_CRASH_63_56_BIT);

        seq_printf(m,"[CMDQ_%d]\033[35mCommand _bits: %04x, %04x, %04x, %04x \033[m\n",i
                   , Command_63_56_bit, Command_55_32_bit, Command_31_16_bit, Command_15_0_bit);
    }
    return 0;
}

static int CmdqDumpFinalCmdProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, CmdqDumpFinalCmdProcShow, NULL);
}

static const struct file_operations CmdqDumpFinalCmdProcOps = {
	.owner		= THIS_MODULE,
	.open		= CmdqDumpFinalCmdProcOpen,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int CmdqDumpAllCmdProcShow(struct seq_file *m, void *v)
{
    CmdqInterfacePriv_t * pPrivData;
    u32                 nCmdCnt=0;
    u16                 *pCmdPoint;
    u16                 *pVirLastBufwrite,*pVirStartBufwrite,*pVirEndBufwrite,*pVirBufwrite;


    if(gCmdqDumEngine >= NUMBER_OF_CMDQ_HW)
    {
        printk("[CMDQ]CMDQ num is ill(%d)\n", gCmdqDumEngine);
        return 0;
    }

    if(!gDrvCmdPriv[gCmdqDumEngine].bUsed)
        return 0;

    pPrivData = (CmdqInterfacePriv_t * )&gDrvCmdPriv[gCmdqDumEngine];
    pVirStartBufwrite = (u16*)_MDrvCmdqTransPhyBuf2VirBuf(pPrivData, (u32)pPrivData->tCmdqInfo.pBufStart);

    if((u32)(pPrivData->tCmdqInfo.pBufLastFireWriteHw))
        pVirLastBufwrite  = (u16*)_MDrvCmdqTransPhyBuf2VirBuf(pPrivData,((u32)(pPrivData->tCmdqInfo.pBufLastFireWriteHw)<< DRVCMD_BUFFER_SHIFT));
    else
        pVirLastBufwrite = pVirStartBufwrite;

    pVirEndBufwrite   = (u16*)_MDrvCmdqTransPhyBuf2VirBuf(pPrivData, (u32)pPrivData->tCmdqInfo.pBufEnd);

    if((u32)(pPrivData->tCmdqInfo.pBufFireWriteHw))
        pVirBufwrite      = (u16*)_MDrvCmdqTransPhyBuf2VirBuf(pPrivData, ((u32)(pPrivData->tCmdqInfo.pBufFireWriteHw)<<DRVCMD_BUFFER_SHIFT));
    else
    {
        printk("[CMDQ]CMDQ(%d) no fire yet\n", gCmdqDumEngine);
        return 0;
    }

    if((u32)pVirLastBufwrite < (u32)pVirBufwrite)
    {
        for(pCmdPoint = pVirLastBufwrite ; pCmdPoint < pVirBufwrite;pCmdPoint+=4)
        {
            seq_printf(m,"[CMDQ_%d]\033[35mCommand[%d]: %04x, %04x, %04x, %04x \033[m\n",gCmdqDumEngine
                       ,nCmdCnt,pCmdPoint[0], pCmdPoint[1], pCmdPoint[2],pCmdPoint[3]);
            nCmdCnt++;
        }
    }
    else if((u32)pVirLastBufwrite > (u32)pVirBufwrite)
    {
        for(pCmdPoint = pVirLastBufwrite ; pCmdPoint < pVirEndBufwrite;pCmdPoint+=4)
        {
            seq_printf(m,"[CMDQ_%d]\033[35mCommand[%d]: %04x, %04x, %04x, %04x \033[m\n",gCmdqDumEngine
                       ,nCmdCnt,pCmdPoint[0], pCmdPoint[1], pCmdPoint[2],pCmdPoint[3]);
            nCmdCnt++;
        }

        for(pCmdPoint = pVirStartBufwrite ; pCmdPoint < pVirBufwrite;pCmdPoint+=4)
        {
            seq_printf(m,"[CMDQ_%d]\033[35mCommand[%d]: %04x, %04x, %04x, %04x \033[m\n",gCmdqDumEngine
                       ,nCmdCnt,pCmdPoint[0], pCmdPoint[1], pCmdPoint[2],pCmdPoint[3]);
            nCmdCnt++;
        }
    }
    return 0;
}

static int CmdqDumpAllCmdProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, CmdqDumpAllCmdProcShow, NULL);
}

static ssize_t CmdqDumpAllProcWrite (struct file *file,
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
    {
        if(val >= NUMBER_OF_CMDQ_HW)
        {
            printk("[CMDQ]CMDQ num is ill(%lu)\n", val);
            return strnlen(buf, len);
        }
		gCmdqDumEngine = val;
    }
	return strnlen(buf, len);
}

static const struct file_operations CmdqDumpAllCmdProcOps = {
	.owner		= THIS_MODULE,
	.open		= CmdqDumpAllCmdProcOpen,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= CmdqDumpAllProcWrite,
};

static int CmdqIrqEvtProcShow(struct seq_file *m, void *v)
{
    CmdqInterfacePriv_t * pPrivData;
    u32                 i=0;
    u16                 nIrqMsk;


    for(i=0;i<NUMBER_OF_CMDQ_HW;i++)
    {
        pPrivData = (CmdqInterfacePriv_t * )&gDrvCmdPriv[i];
        if(!pPrivData->bUsed)
            continue;
        nIrqMsk = HAL_CMDQ_GetISRMSK(pPrivData->nCmdqDrvIpId);
        seq_printf(m,"[CMDQ_%d] IrqMask: %04x\n",i,nIrqMsk);
    }

    seq_printf(m,"[CMDQ_%d] echo cmdqId Mask (0x1:poll neq 0x2:poll eq 0x4:wait 0x8:write)\n",gCmdqDumEngine);
    return 0;
}

static int CmdqIrqEvtProcOpen(struct inode *inode, struct file *file)
{
	return single_open(file, CmdqIrqEvtProcShow, NULL);
}

static ssize_t CmdqIrqEvtProcWrite (struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	char buf[] = "0x00000000\n";
	size_t len = min(sizeof(buf) - 1, count);
	unsigned int val;
    CmdqInterfacePriv_t * pPrivData;
    u32                   i=0;
    u32                   nIrqMsk = 0;

	if (copy_from_user(buf, buffer, len))
		return count;
	buf[len] = 0;
	if (sscanf(buf, "%x", &val) != 1)
		printk(": %s is not in hex or decimal form.\n", buf);
	else
    {
        for(i=0;i<NUMBER_OF_CMDQ_HW;i++)
        {
            pPrivData = (CmdqInterfacePriv_t * )&gDrvCmdPriv[i];
            if(!pPrivData->bUsed)
                continue;

            nIrqMsk = HAL_CMDQ_GetISRMSK(pPrivData->nCmdqDrvIpId);

            if(val & 0x1)
                nIrqMsk |= DRVCMDQ_POLLNEQ_TIMEOUT;
            else
                nIrqMsk &= ~DRVCMDQ_POLLNEQ_TIMEOUT;

            if(val & 0x2)
                nIrqMsk |= DRVCMDQ_POLLEQ_TIMEOUT;
            else
                nIrqMsk &= ~DRVCMDQ_POLLEQ_TIMEOUT;

            if(val & 0x4)
                nIrqMsk |= DRVCMDQ_WAIT_TIMEOUT;
            else
                nIrqMsk &= ~DRVCMDQ_WAIT_TIMEOUT;

            if(val & 0x8)
                nIrqMsk |= DRVCMDQ_WRITE_TIMEOUT;
            else
                nIrqMsk &= ~DRVCMDQ_WRITE_TIMEOUT;

            HAL_CMDQ_SetISRMSK(pPrivData->nCmdqDrvIpId,nIrqMsk);
        }
    }
	return strnlen(buf, len);
}

static const struct file_operations CmdqIrqEvtProcOps = {
	.owner		= THIS_MODULE,
	.open		= CmdqIrqEvtProcOpen,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= CmdqIrqEvtProcWrite,
};

void CmdqProcInit(void)
{
    struct proc_dir_entry *pde;

    if(gInitCmdqProc)
        return;
    CMDQ_LOG("%s %d\n",__FUNCTION__,__LINE__);
    gInitCmdqProc =1;

    gpRootCmdqDir = proc_mkdir("cmdq", NULL);
	if (!gpRootCmdqDir)
    {
        CMDQ_ERR("[CMDQ]can not create proc\n");
		return;
    }

	pde = proc_create("dbglvl", S_IRUGO, gpRootCmdqDir, &CmdqDbglvlProcOps);
	if (!pde)
		goto out_dbglvl;
	pde = proc_create("engdump", S_IRUGO,gpRootCmdqDir, &CmdqDumpCtxProcOps);
	if (!pde)
		goto out_engdump;
    pde = proc_create("finalcmd", S_IRUGO,gpRootCmdqDir, &CmdqDumpFinalCmdProcOps);
	if (!pde)
		goto out_finaldump;
    pde = proc_create("dumpcmd", S_IRUGO,gpRootCmdqDir, &CmdqDumpAllCmdProcOps);
	if (!pde)
		goto out_dumpcmd;
    pde = proc_create("irqevt", S_IRUGO,gpRootCmdqDir, &CmdqIrqEvtProcOps);
	if (!pde)
		goto out_irqevt;
	return ;

out_irqevt:
    remove_proc_entry("dumpcmd",gpRootCmdqDir);
out_dumpcmd:
    remove_proc_entry("engdump",gpRootCmdqDir);
out_finaldump:
    remove_proc_entry("engdump",gpRootCmdqDir);
out_engdump:
	remove_proc_entry("dbglvl",gpRootCmdqDir);
out_dbglvl:
    return ;
}
void CmdqProcDeInit(void)
{
    if(!gInitCmdqProc)
        return;
    gInitCmdqProc = 0;
    CMDQ_LOG("%s %d\n",__FUNCTION__,__LINE__);
    remove_proc_entry("dbglvl", gpRootCmdqDir);
    remove_proc_entry("engdump", gpRootCmdqDir);
    remove_proc_entry("finalcmd", gpRootCmdqDir);
    remove_proc_entry("dumpcmd", gpRootCmdqDir);
    remove_proc_entry("irqevt", gpRootCmdqDir);
	remove_proc_entry("cmdq", NULL);
}
