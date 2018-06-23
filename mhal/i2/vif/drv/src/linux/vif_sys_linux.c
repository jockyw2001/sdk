#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/i2c.h>
//#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <asm/io.h>
//#include <linux/clk-provider.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <ms_platform.h>
#include <asm/string.h>
#include <asm/div64.h>
#include <vif_common.h>

#include "drv_vif.h"
#include "hal_vif.h"
#include <mdrv_vif_io.h>
#include "8051.h"

#include "hal_dma.h"
#include "arch/infinity2_reg_padtop1.h"
#include "arch/infinity2_reg_block_ispsc.h"
#include "arch/infinity2_reg_isp_dma_irq.h"
#include "infinity2/irqs.h"

#define REG_R(base,offset) (*(unsigned short*)(base+(offset*4)))
#define REG_W(base,offset,val) ((*(unsigned short*)((base)+ (offset*4)))=(val))
#define MAILBOX_HEART_REG (0x6D)
#define MAILBOX_CONCTRL_REG (0x6E)
#define MAILBOX_STATE_REG (0x6F)

extern void *g_ptVIF;
extern void *g_ptVIF2;
extern void *g_BDMA;
extern void *g_MCU8051;
extern void *g_PMSLEEP;
extern void *g_MAILBOX;
extern void *g_TOPPAD1;
extern void *g_ISP_ClkGen;

extern volatile VifRingBufShm_t *SHM_Ring;
extern void Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size);

static int mstar_vif_probe(struct platform_device *pdev);
static int mstar_vif_remove(struct platform_device *pdev);


#if 1
/* Match table for of_platform binding */
static const struct of_device_id mstar_vif_of_match[] = {
	{ .compatible = "mstar,vif", 0},
	{},
};
MODULE_DEVICE_TABLE(of, mstar_vif_of_match);
#endif

static struct platform_driver mstar_vif_driver = {
	.probe   = mstar_vif_probe,
	.remove  = mstar_vif_remove,
	.driver  = {
		.name  = "mstar-vif",
		.owner = THIS_MODULE,
		.of_match_table = mstar_vif_of_match,
	},
};

struct miscdevice vif_dev;
static int vif_open(struct inode *inode, struct file *fp);
static int vif_release(struct inode *inode, struct file *fp);
static ssize_t vif_fread(struct file *fp, char __user *buf, size_t size, loff_t *ppos);
static ssize_t vif_fwrite(struct file *fp,const char __user *buf, size_t size, loff_t *ppos);
static long vif_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
struct file_operations vif_fops =
{
    .owner = THIS_MODULE,
    .open = vif_open,
    .release = vif_release,
    .read = vif_fread,
    .write = vif_fwrite,
    .unlocked_ioctl = vif_ioctl,
};


#define IPC_RAM_SIZE (64*1024)
#define TIMER_RATIO 12

#if IPC_DMA_ALLOC_COHERENT
extern u32 IPCRamPhys;
extern char *IPCRamVirtAddr;
#else
extern unsigned long IPCRamPhys;
extern void *IPCRamVirtAddr;
#endif

struct vif_dev_data{
    struct work_struct isp_dma_wq;

    /*
    *   Four line count status in each channel
    *   [0]Main Y, [1]Main C, [2]Sub Y, [3]Sub C
    */
    u8 line_cnt_status[VIF_CHANNEL_NUM];

    /*
    *   bit[0] for channel 0, bit[1] for channel 1...
    *   16 channel maximum
    */
    u16 vif_frame_start_status;

    u8 isr_run_cb;

    spinlock_t lock;

    u8 inited;

};

struct vif_dev_data vif_dev_ctx; // ToDo:use kmalloc

void isp_dma_do_work(struct work_struct *work)
{
    struct vif_dev_data *data = container_of(work, struct vif_dev_data, isp_dma_wq);
    u32 i;
    unsigned long flags;
    //pr_info("[do_work] %x %x\n", (u32)data->line_cnt_status[0], (u32)data->vif_frame_start_status);

    spin_lock_irqsave(&data->lock, flags);

    for (i = 0; i < VIF_CHANNEL_NUM; i++) {
        if (data->vif_frame_start_status & (1 << i)) {
            DrvVif_DoVifFrameStartCb(i);
        }

        if (data->line_cnt_status[i] & WDMA_LINE_CNT_MAIN_Y) {
            DrvVif_DoDmaLineCntCb(i);
        }
        data->line_cnt_status[i] = 0;
    }
    data->vif_frame_start_status = 0;

    spin_unlock_irqrestore(&data->lock, flags);
}

u8 DrvVif_FakeIrq(irqreturn_t func)
{
    return true;
}
EXPORT_SYMBOL(DrvVif_FakeIrq);

// MCU control function

void DrvVif_dumpInfo(void) {
    int idx=0, sub_idx=0;
    int bound = VIF_PORT_NUM>>1;
    printk("\n\n\n");
    for (idx=0; idx<bound; ++idx) {
        printk("  ");
        for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
            if (SHM_Ring[idx].nReadIdx == sub_idx) {
                printk("R ");
            } else if (SHM_Ring[idx].nWriteIdx == sub_idx){
                printk("W ");
            } else {
                printk("  ");
            }
        }
        printk("   readIdx=%d writeidx=%d     ",SHM_Ring[idx].nReadIdx,SHM_Ring[idx].nWriteIdx);

        for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
            if (SHM_Ring[idx+bound].nReadIdx == sub_idx) {
                printk("R ");
            } else if (SHM_Ring[idx+bound].nWriteIdx == sub_idx){
                printk("W ");
            } else {
                printk("  ");
            }
        }
        printk("   readIdx=%d writeidx=%d \n",SHM_Ring[idx+bound].nReadIdx,SHM_Ring[idx+bound].nWriteIdx);

        printk("< ");
        for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
            /* Print ring buffer status */

            switch(SHM_Ring[idx].data[sub_idx].nStatus) {
                case E_VIF_BUF_EMPTY:
                    printk("E ");
                break;
                case E_VIF_BUF_FILLING:
                    printk("F ");
                break;
                case E_VIF_BUF_READY:
                    printk("R ");
                break;
                case E_VIF_BUF_INVALID:
                    printk("I ");
                break;
                default:
                break;
            }
        }

        printk(" >                        < ");
        for (sub_idx=0; sub_idx<VIF_RING_QUEUE_SIZE; ++sub_idx) {
            /* Print ring buffer status */

            switch(SHM_Ring[idx+bound].data[sub_idx].nStatus) {
                case E_VIF_BUF_EMPTY:
                    printk("E ");
                break;
                case E_VIF_BUF_FILLING:
                    printk("F ");
                break;
                case E_VIF_BUF_READY:
                    printk("R ");
                break;
                case E_VIF_BUF_INVALID:
                    printk("I ");
                break;
                default:
                break;
            }
        }

        printk(" >\n");
    }
}

/*******************************************************************************************8*/


#if 0
void DrvVif_WaitFSInts(VIF_CHANNEL_e ch)
{
    rtk_flag_value_t event_flag = 0;

	switch(ch)
	{
		case VIF_CHANNEL_0:
			event_flag = VIF_CH0_FRAME_START_INTS;
			break;
		case VIF_CHANNEL_1:
			event_flag = VIF_CH1_FRAME_START_INTS;
			break;
		case VIF_CHANNEL_2:
			event_flag = VIF_CH2_FRAME_START_INTS;
			break;
		default:
			UartSendTrace("[%s] err, over VIF chanel number \n", __func__);
	}

    event_flag = MsFlagWait(&_ints_event_flag, event_flag, MS_FLAG_WAITMODE_AND | MS_FLAG_WAITMODE_CLR);
    UartSendTrace("###[%s], event_flag = %#x, _ints_event_flag = %#x \n", __func__, event_flag, _ints_event_flag.FlagValue);
}

void DrvVif_WaitFEInts(VIF_CHANNEL_e ch)
{
    rtk_flag_value_t event_flag = 0;
	switch(ch)
	{
		case VIF_CHANNEL_0:
			event_flag = VIF_CH0_FRAME_END_INTS;
			break;
		case VIF_CHANNEL_1:
			event_flag = VIF_CH1_FRAME_END_INTS;
			break;
		case VIF_CHANNEL_2:
			event_flag = VIF_CH2_FRAME_END_INTS;
			break;
		default:
			UartSendTrace("[%s] err, over VIF chanel number \n", __func__);
	}
    event_flag = MsFlagWait(&_ints_event_flag, event_flag, MS_FLAG_WAITMODE_AND | MS_FLAG_WAITMODE_CLR);
    UartSendTrace("###[%s], event_flag = %#x, _ints_event_flag = %#x \n", __func__, event_flag, _ints_event_flag.FlagValue);
}
#endif
#if 0
void DrvVif_FS_Task_Init(void* pDummy)
{
    while(1)
    {
        DrvVif_WaitFSInts(VIF_CHANNEL_0);
    }
}

void DrvVif_FS_Parser(vm_msg_t* pMessage)
{

}

void DrvVif_FE_Task_Init(void* pDummy)
{
    while(1)
    {
        DrvVif_WaitFEInts(VIF_CHANNEL_0);
    }
}

void DrvVif_FE_Parser(vm_msg_t* pMessage)
{

}

int DrvVif_CreateFSTask(void)
{
    MsTaskCreateArgs_t TaskArgs;

    if(u8VifFSTaskId != 0xFF)
	{
		UartSendTrace("VIF_FS_TASK Task Create Fail, Taskid = %d \n", u8VifFSTaskId);
		return -VIF_FAIL;
    }
	UartSendTrace("VIF_FS_TASK Task Create\n");

    // Note the task name space is 8-1 chars
    strcpy(TaskArgs.TaskName, "VIF_FS_TASK");

    TaskArgs.AppliInit = DrvVif_FS_Task_Init;
    TaskArgs.AppliParser = DrvVif_FS_Parser;
    TaskArgs.ImmediatelyStart = TRUE;
    TaskArgs.StackSize = VIF_TASK_STACK_SIZE;
    TaskArgs.Priority = CUS58_PRIO;
    TaskArgs.pInitArgs = NULL;
    TaskArgs.TaskId = &u8VifFSTaskId;
    TaskArgs.pStackTop = MsAllocateMem(VIF_TASK_STACK_SIZE);
    TaskArgs.TimeSliceLeft = RTK_MS_TO_TICK(5);
    TaskArgs.TimeSliceMax = RTK_MS_TO_TICK(5);
    pVifFSStackTop = TaskArgs.pStackTop;

    if(MS_OK != MsCreateTask(&TaskArgs))
    {
        MsReleaseMemory(pVifFSStackTop);
        UartSendTrace("VIF_FS_TASK Task Create Fail!\n");
        return -VIF_FAIL;
    }

    return VIF_SUCCESS;
}

int DrvVif_DeleteFSTask(void)
{
	MsDeleteTask(u8VifFSTaskId);
	MsReleaseMemory(pVifFSStackTop);
	u8VifFSTaskId = 0xFF;
    return VIF_SUCCESS;
}

int DrvVif_CreateFETask(void)
{
    MsTaskCreateArgs_t TaskArgs;

    if(u8VifFETaskId != 0xFF)
	{
		UartSendTrace("VIF_FE_TASK Task Create Fail, Taskid = %d \n", u8VifFETaskId);
		return -VIF_FAIL;
    }
	UartSendTrace("VIF_FE_TASK Task Create\n");

    // Note the task name space is 8-1 chars
    strcpy(TaskArgs.TaskName, "VIF_FE_TASK");

    TaskArgs.AppliInit = DrvVif_FE_Task_Init;
    TaskArgs.AppliParser = DrvVif_FE_Parser;
    TaskArgs.ImmediatelyStart = TRUE;
    TaskArgs.StackSize = VIF_TASK_STACK_SIZE;
    TaskArgs.Priority = CUS58_PRIO;
    TaskArgs.pInitArgs = NULL;
    TaskArgs.TaskId = &u8VifFETaskId;
    TaskArgs.pStackTop = MsAllocateMem(VIF_TASK_STACK_SIZE);
    TaskArgs.TimeSliceLeft = RTK_MS_TO_TICK(5);
    TaskArgs.TimeSliceMax = RTK_MS_TO_TICK(5);
    pVifFEStackTop = TaskArgs.pStackTop;

    if(MS_OK != MsCreateTask(&TaskArgs))
    {
        MsReleaseMemory(pVifFEStackTop);
        UartSendTrace("VIF_FE_TASK Task Create Fail!\n");
        return -VIF_FAIL;
    }

    return VIF_SUCCESS;
}

int DrvVif_DeleteFETask(void)
{
	MsDeleteTask(u8VifFETaskId);
	MsReleaseMemory(pVifFEStackTop);
	u8VifFETaskId = 0xFF;
    return VIF_SUCCESS;
}

void DrvVif_ClearInt(volatile u32 *clear, VIF_INTERRUPT_e int_num)
{
    *clear |= (0x01 << int_num);
}
#endif

#if 0
static irqreturn_t vif_isr(int irq, void* priv)
{
#if 0
    //infinity2_reg_ispsc_vif *pvif_handle = g_ptVIF;
    unsigned int status, i;

    //pr_info("1vif irq mask 0x%x\n", pvif_handle->reg_vif_ch0_c_irq_mask);
    //pr_info("1vif irq force 0x%x\n", pvif_handle->reg_vif_ch0_c_irq_force);
    //pr_info("1vif irq status 0x%x\n", pvif_handle->reg_vif_ch0_irq_final_status);
    //pr_info("1vif irq clr 0x%x\n", pvif_handle->reg_vif_ch0_c_irq_clr);

    status = HalVif_IrqFinalStatus(0);

    HalVif_IrqForce(0, 0x0);   //for testing

    HalVif_IrqMask(0, 0xFF);

    HalVif_IrqClr1(0, status);

    for(i = 0; i < VIF_INTERRUPT_FINAL_STATUS_MAX; i++){
        if(status & (1<<i)){
            //pr_info("run int status cb 0x%x\n", i);
            DrvVif_DoIntStatusCB(0, i);
        }
    }
    HalVif_IrqUnMask(0, 0xFF);
#endif
    return IRQ_HANDLED;
}
#endif

static irqreturn_t isp_dma_isr(int irq, void* priv)
{
    struct vif_dev_data *data = (struct vif_dev_data *)priv;
    u32 i;

    //DrvVif_WDMA_ISR(priv);

    spin_lock(&data->lock);
    /* Read line count status */
    for (i = 0; i < VIF_CHANNEL_NUM; i++) {

        data->line_cnt_status[i] |= DrvVif_LineCntIrqFinalStatus(i);

        /* clear this interrupt  */
        DrvVif_LineCntIrqMask(i, data->line_cnt_status[i], 1);
        DrvVif_LineCntIrqClr(i, data->line_cnt_status[i]);
        DrvVif_LineCntIrqMask(i, data->line_cnt_status[i], 0);
    }

    /* Read vif frame start status*/
#if 0
    data->vif_frame_start_status |= DrvVif_FrameStartIrqFinalStatus();

    DrvVif_FrameStartIrqMask(data->vif_frame_start_status, 1);
    DrvVif_FrameStartIrqClr(data->vif_frame_start_status);
    DrvVif_FrameStartIrqMask(data->vif_frame_start_status, 0);
#endif

    if (data->isr_run_cb) {
        //pr_info("isr_run_cb!\n");
        for (i = 0; i < VIF_CHANNEL_NUM; i++) {
            if (data->vif_frame_start_status & (1 << i)) {
                DrvVif_DoVifFrameStartCb(i);
            }

            if (data->line_cnt_status[i] & WDMA_LINE_CNT_MAIN_Y) {
                DrvVif_DoDmaLineCntCb(i);
            }
            data->line_cnt_status[i] = 0;
        }
        data->vif_frame_start_status = 0;
    } else {
        schedule_work(&vif_dev_ctx.isp_dma_wq);
    }

    spin_unlock(&data->lock);

    return IRQ_HANDLED;
}

static int vif_open(struct inode *inode, struct file *fp)
{
    return 0;
}

static int vif_release(struct inode *inode, struct file *fp)
{
    return 0;
}
static ssize_t vif_fread(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t vif_fwrite(struct file *fp,const char __user *buf, size_t size, loff_t *ppos)
{
    int rc = 0;
    int cmd = 0;
    sscanf(buf, "%d", &cmd);
    //pr_info("buf %s\n",buf);
    switch(cmd) {
        case 1:
        {
            rc = DrvVif_stopMCU();
            if (rc) {
                pr_err("Init stop MCU fail\n");
                goto VIF_FAIL;
            }
            pr_info("Stop MCU done!\n");
        }
        break;
        case 2:
        {
            rc = DrvVif_startMCU();
            if (rc) {
                pr_err("Start MCU fail\n");
                goto VIF_FAIL;
            }
            pr_info("Start MCU done!\n");
        }
        break;
        case 3:
        {
            rc = DrvVif_pollingMCU();
            if (rc) {
                pr_err("Polling MCU fail\n");
                goto VIF_FAIL;
            }
            pr_info("Polling MCU done!\n");
        }
        break;
        case 4:
        {
            rc = DrvVif_pauseMCU();
            if (rc) {
                pr_err("Pause MCU fail\n");
                goto VIF_FAIL;
            }
            pr_info("Pause MCU done!\n");
        }
        break;

        //Ring buffer queue/dequeue
        case 10:
        {
            #if 0
             MHal_VIF_RingBufElm_t temp = { {0x2E600000,0x2E620000}, {720, 720}, 0, 0, 720, 240, 264960, E_VIF_BUF_INVALID, 0};
             MHal_VIF_PORT u32ChnPort = 0;
             u32 channel = 0;
             u32 main_addr=0, sub_addr=0;
             sscanf(buf, "%*d %d %d %x %x", &channel, &u32ChnPort, &main_addr, &sub_addr);
             temp.u64PhyAddr[0] = main_addr;
             temp.u64PhyAddr[1] = sub_addr;
             printk("main addr %x, sub addr %x\n",main_addr,sub_addr);
             DrvVif_QueueFrameBuffer(channel, u32ChnPort,&temp);
             #endif
        }
        break;
        case 11:
        {
             u32 channel = 0;
             u32 num = 0;
             MHal_VIF_PORT u32ChnPort = 0;
             DrvVif_QueryFrames(channel, u32ChnPort,&num);
             pr_info("Query number of READY frame = %d\n",num);
        }
        break;
        case 12:
        {
             //MHal_VIF_RingBufElm_t temp = {0};
             MHal_VIF_RingBufElm_t temp;
             u32 num = 0;
             u8 idx = 0;
             u32 channel = 0;
             MHal_VIF_PORT u32ChnPort = 0;
             DrvVif_QueryFrames(channel, u32ChnPort,&num);
             memset(&temp,0,sizeof(MHal_VIF_RingBufElm_t));
             for (idx = 0;idx<num;idx++) {
                 DrvVif_DequeueFrameBuffer(channel, u32ChnPort, &temp);
                 pr_info("Dequeue buffer info: AddrY=0x%x AddrC=0x%x width=%d height=%d pitch=%d nPTS=0x%x\n",
                         (u32)temp.u64PhyAddr[0], (u32)temp.u64PhyAddr[1], temp.nCropW, temp.nCropH, temp.u32Stride[0], temp.nPTS);
             }
             pr_info("Query number of READY frame = %d\n",num);
             pr_info("Pre_idx = %d\n",SHM_Ring[channel].pre_nReadIdx);
        }
        break;
        case 13:
        {
             MHal_VIF_PORT u32ChnPort = 0;
             u32 channel = 0;
             u32 eFrameRate;
             sscanf(buf, "%*d %d %d %d", &channel, &u32ChnPort, &eFrameRate);
             DrvVif_setChnFPSBitMask(channel, u32ChnPort, eFrameRate, NULL);
        }
        break;
        case 254:
        {
             DrvVif_dumpInfo();
        }
        break;

        case 300:
        {
            vif_dev_ctx.isr_run_cb = 0;
            pr_info("Callback function run in isr context %d\n",vif_dev_ctx.isr_run_cb);
        }
        break;

        case 301:
        {
            vif_dev_ctx.isr_run_cb = 1;
            pr_info("Callback function run in isr context %d\n",vif_dev_ctx.isr_run_cb);
        }
        break;
        case 302:
        {
            DrvVif_ShowRegistedCb();
        }
        break;
        case 303:
        {
            extern void DriVif_SensorInit(u32 uCamID);
            DriVif_SensorInit(0);
        }
        break;

        default:
            pr_err("Invalid command[%d]!!",cmd);
        break;
    }
    return size;
VIF_FAIL:
    //unInitialSHMRing();
    return size;
}

static long vif_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
        VIF_CMD_st VifCmd;

	if (_IOC_TYPE(cmd) != VIF_IOCTL_MAGIC) return -ENOTTY;
	switch(cmd)
	{
            case IOCTL_VIF_MCU_DEV_SET_CFG:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_SET_CFG\n");
                ret = DrvVif_DevSetConfig(VifCmd.MCU_DevSetConfig.u32VifDev,(MHal_VIF_DevCfg_t *) &(VifCmd.MCU_DevSetConfig.pstDevAttr), NULL);
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_DEV_ENABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_DEV_ENABLE\n");
                ret = DrvVif_DevEnable(VifCmd.MCU_DevEnable.u32VifDev);
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_DEV_DISABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_DEV_DISABLE\n");
                ret = DrvVif_DevDisable(VifCmd.MCU_DevDisable.u32VifDev);
                return ret;
            }
            break;

            case IOCTL_VIF_MCU_CHN_SET_CFG:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_SET_CFG :%d\n",VifCmd.MCU_ChnSetConfig.pstAttr.stCapRect.u16Width);
                ret = DrvVif_ChnSetConfig(VifCmd.MCU_ChnSetConfig.u32VifChn, (MHal_VIF_ChnCfg_t *)&(VifCmd.MCU_ChnSetConfig.pstAttr));
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_CHN_ENABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_ENABLE\n");
                ret = DrvVif_ChnEnable(VifCmd.MCU_ChnEnable.u32VifChn);
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_CHN_DISABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_DISABLE\n");
                ret = DrvVif_ChnDisable(VifCmd.MCU_ChnEnable.u32VifChn);
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_CHN_QUERY:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_QUERY\n");
                ret = DrvVif_ChnQuery(VifCmd.MCU_ChnQuery.u32VifChn,(MHal_VIF_ChnStat_t *) &(VifCmd.MCU_ChnQuery.pstStat));
                return ret;
            }
            break;

            case IOCTL_VIF_SUB_MCU_CHN_SET_CFG:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_SUB_MCU_CHN_SET_CFG\n");
                ret = DrvVif_SubChnSetConfig(VifCmd.MCU_SubChnSetConfig.u32VifChn,(MHal_VIF_SubChnCfg_t *) &(VifCmd.MCU_SubChnSetConfig.pstAttr));
                return ret;
            }
            break;
            case IOCTL_VIF_SUB_MCU_CHN_ENABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_ENABLE\n");
                ret = DrvVif_SubChnEnable(VifCmd.MCU_SubChnEnable.u32VifChn);
                return ret;
            }
            break;
            case IOCTL_VIF_SUB_MCU_CHN_DISABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_DISABLE\n");
                ret = DrvVif_SubChnDisable(VifCmd.MCU_SubChnDisable.u32VifChn);
                return ret;
            }
            break;
            case IOCTL_VIF_SUB_MCU_CHN_QUERY:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_CHN_QUERY\n");
                ret = DrvVif_SubChnQuery(VifCmd.MCU_SubChnQuery.u32VifChn, (MHal_VIF_ChnStat_t *)&(VifCmd.MCU_SubChnQuery.pstStat));
                return ret;
            }
            break;

            case IOCTL_VIF_MCU_QUEUE_FRAME:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_QUEUE_FRAME\n");
                ret = DrvVif_QueueFrameBuffer(VifCmd.MCU_Queue_Frame.u32VifChn, VifCmd.MCU_Queue_Frame.u32ChnPort, (MHal_VIF_RingBufElm_t *)&(VifCmd.MCU_Queue_Frame.ptFbInfo));
                return ret;
            }
            break;
            case IOCTL_VIF_MCU_QUERY_FRAME:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_QUERY_FRAME\n");
                ret = DrvVif_QueryFrames(VifCmd.MCU_Query_Frame.u32VifChn, VifCmd.MCU_Query_Frame.u32ChnPort,(u32 *)&(VifCmd.MCU_Query_Frame.pNumBuf));
				pr_info("ready frame num:%d\n",VifCmd.MCU_Query_Frame.pNumBuf);

                if(copy_to_user((VIF_CMD_st *)arg, &VifCmd, sizeof(VifCmd.MCU_Query_Frame)))
                    return -EFAULT;

                return ret;
            }
            break;
            case IOCTL_VIF_MCU_DEQUEUE_FRAME:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_DEQUEUE_FRAME\n");

                //Chip_Flush_Cache_Range((unsigned long)phys_to_virt(VifCmd.MCU_DeQueue_Frame.ptFbInfo.u64PhyAddr[0]), VifCmd.MCU_DeQueue_Frame.ptFbInfo.nCropW*VifCmd.MCU_DeQueue_Frame.ptFbInfo.nCropH);
                //Chip_Flush_Cache_Range((unsigned long)phys_to_virt(VifCmd.MCU_DeQueue_Frame.ptFbInfo.u64PhyAddr[1]), ((VifCmd.MCU_DeQueue_Frame.ptFbInfo.nCropW + 31)/32)*32 * VifCmd.MCU_DeQueue_Frame.ptFbInfo.nCropH);

                ret = DrvVif_DequeueFrameBuffer(VifCmd.MCU_DeQueue_Frame.u32VifChn, VifCmd.MCU_DeQueue_Frame.u32ChnPort,(MHal_VIF_RingBufElm_t *)&(VifCmd.MCU_DeQueue_Frame.ptFbInfo));

                if(copy_to_user((VIF_CMD_st *)arg, &VifCmd, sizeof(VifCmd.MCU_DeQueue_Frame)))
                    return -EFAULT;

                return ret;
            }
            break;
            case IOCTL_VIF_MCU_TEST_FLOW:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                pr_info("IOCTL_VIF_MCU_TEST_FLOW\n");
                //ret = DrvVif_DequeueFrameBuffer(VifCmd.MCU_DeQueue_Frame.u32VifChn, VifCmd.MCU_DeQueue_Frame.u32ChnPort,VifCmd.MCU_DeQueue_Frame.ptFbInfo);
                return ret;
            }
            break;
            case IOCTL_VIF_CONFIG_PARALLEL_IF:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                //pr_info("IOCTL_VIF_MCU_TEST_FLOW\n");
                //ret = DrvVif_DequeueFrameBuffer(VifCmd.MCU_DeQueue_Frame.u32VifChn, VifCmd.MCU_DeQueue_Frame.u32ChnPort,VifCmd.MCU_DeQueue_Frame.ptFbInfo);
                ret = DrvVif_ConfigParallelIF(  VifCmd.ParallelCfg_t.u32VifChn,
                                                VifCmd.ParallelCfg_t.svif_sensor_in_format,
                                                VifCmd.ParallelCfg_t.PixDepth,
                                                VifCmd.ParallelCfg_t.PclkPol,
                                                VifCmd.ParallelCfg_t.VsyncPol,
                                                VifCmd.ParallelCfg_t.HsyncPol,
                                                VifCmd.ParallelCfg_t.RstPol,
                                                VifCmd.ParallelCfg_t.CropX,
                                                VifCmd.ParallelCfg_t.CropY,
                                                VifCmd.ParallelCfg_t.CropW,
                                                VifCmd.ParallelCfg_t.CropH,
                                                VifCmd.ParallelCfg_t.CropEnable,
                                                VifCmd.ParallelCfg_t.Mclk
                      );
                return ret;
            }
            break;
            case IOCTL_VIF_CONFIG_MIPI_IF:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;

                //pr_info("IOCTL_VIF_MCU_TEST_FLOW\n");
                //ret = DrvVif_DequeueFrameBuffer(VifCmd.MCU_DeQueue_Frame.u32VifChn, VifCmd.MCU_DeQueue_Frame.u32ChnPort,VifCmd.MCU_DeQueue_Frame.ptFbInfo);
                ret = DrvVif_ConfigMipiIF(  VifCmd.MipiCfg_t.u32VifChn,
                                            VifCmd.MipiCfg_t.svif_sensor_in_format,
                                            VifCmd.MipiCfg_t.PixDepth,
                                            //VifCmd.MipiCfg_t.PclkPol,
                                            //VifCmd.MipiCfg_t.VsyncPol,
                                            //VifCmd.MipiCfg_t.HsyncPol,
                                            VifCmd.MipiCfg_t.RstPol,
                                            VifCmd.MipiCfg_t.CropX,
                                            VifCmd.MipiCfg_t.CropY,
                                            VifCmd.MipiCfg_t.CropW,
                                            VifCmd.MipiCfg_t.CropH,
                                            VifCmd.MipiCfg_t.CropEnable,
                                            VifCmd.MipiCfg_t.Mclk,
                                            VifCmd.MipiCfg_t.Hdr
                      );
                return ret;
            }
            break;

            case IOCTL_VIF_RAW_STORE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;
                pr_info("DMA base=0x%X\n",VifCmd.RawStoreCfg_t.BufMiuAddr);
                ret = DrvVif_RawStore(  VifCmd.RawStoreCfg_t.u32VifChn,
                                        VifCmd.RawStoreCfg_t.BufMiuAddr,
                                        VifCmd.RawStoreCfg_t.ImgW,
                                        VifCmd.RawStoreCfg_t.ImgH
                                     );
                return ret;
            }
            break;

            case IOCTL_VIF_RAW_DMA_ENABLE:
            {
                memset(&VifCmd,0,sizeof(VIF_CMD_st));
                if(copy_from_user(&VifCmd, (VIF_CMD_st *)arg, sizeof(VIF_CMD_st)))
                    return -EFAULT;
                pr_info("Ch %d Raw DMA enable=0x%X\n",VifCmd.RawDmaEnable_t.u32VifChn,
                                                                            VifCmd.RawDmaEnable_t.uEn);
                DrvVif_DmaEnable(VifCmd.RawDmaEnable_t.u32VifChn,VifCmd.RawDmaEnable_t.uEn);
                HalDma_DmaMaskEnable(VifCmd.RawDmaEnable_t.u32VifChn, 0);
                return ret;
            }
            break;
            default:
                pr_err("Invalid ioctl command\n");
	}
    return 0;
}
static int DrvVif_Init_Common(struct platform_device *pdev)
{
    struct resource *res;
    int rc = 0;
    int i;
    //int irq_vif = INT_IRQ_83_VIF_INT + 32 + 4;
    int irq_isp_dma = INT_IRQ_119_ISP_DMA_INT + 32;
    int proval = 0;
    int mcu_enable = 1;
    static u8 inited = 0;
#if 0
    struct clk **vif_clks;
	int num_parents, i,rc = 0;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    vif_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

    //enable all clk
    for(i = 0; i < num_parents; i++)
    {
        vif_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(vif_clks[i]))
        {
            pr_err( "[%s] Fail to get clk!\n", __func__);
            kfree(vif_clks);
            return -1;
        }
        else
        {
            clk_prepare_enable(vif_clks[i]);
            if(i == 0)
	            clk_set_rate(vif_clks[i], 12000000);
        }
    }
    kfree(vif_clks);
#endif
    if (vif_dev_ctx.inited) {
        return 0;
    }
    pr_info("vif driver probe\n");

    if (!inited) {

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        g_ptVIF = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_ptVIF: %p\n", g_ptVIF);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
        g_ptVIF2 = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_ptVIF2: %p\n", g_ptVIF2);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
        g_BDMA = devm_ioremap_resource(&pdev->dev, res);
        pr_info("BDMA: %p\n", g_BDMA);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
        g_MCU8051 = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_MCU8051: %p\n", g_MCU8051);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 4);
        g_PMSLEEP = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_PMSLEEP: %p\n", g_PMSLEEP);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 5);
        g_MAILBOX = devm_ioremap_resource(&pdev->dev, res);
        pr_info("MAILBOX: %p\n", g_MAILBOX);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 6);
        g_TOPPAD1 = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_TOPPAD1: %p\n", g_TOPPAD1);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 7);
        g_ISP_ClkGen = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_ISP_ClkGen: %p\n", g_ISP_ClkGen);

        res = platform_get_resource(pdev, IORESOURCE_MEM, 8);
        g_CLKGEN2 = devm_ioremap_resource(&pdev->dev, res);
        pr_info("g_CLKGEN2: %p\n", g_CLKGEN2);
    }

    of_property_read_u32(pdev->dev.of_node, "ccir0_clk_mode", &proval);
    pr_info("ccir0_clk_mode:%d\n",proval);
    HalVif_CcirPclkPadConfig(0,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir1_clk_mode", &proval);
    pr_info("ccir1_clk_mode:%d\n",proval);
    HalVif_CcirPclkPadConfig(1,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir2_clk_mode", &proval);
    pr_info("ccir2_clk_mode:%d\n",proval);
    HalVif_CcirPclkPadConfig(2,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir3_clk_mode", &proval);
    pr_info("ccir3_clk_mode:%d\n",proval);
    HalVif_CcirPclkPadConfig(3,proval);


    of_property_read_u32(pdev->dev.of_node, "ccir0_ctrl_mode", &proval);
    pr_info("ccir0_ctrl_mode:%d\n",proval);
    HalVif_CcirCtrlPadConfig(0,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir1_ctrl_mode", &proval);
    pr_info("ccir1_ctrl_mode:%d\n",proval);
    HalVif_CcirCtrlPadConfig(1,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir2_ctrl_mode", &proval);
    pr_info("ccir2_ctrl_mode:%d\n",proval);
    HalVif_CcirCtrlPadConfig(2,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir3_ctrl_mode", &proval);
    pr_info("ccir3_ctrl_mode:%d\n",proval);
    HalVif_CcirCtrlPadConfig(3,proval);


    of_property_read_u32(pdev->dev.of_node, "ccir0_8b_mode", &proval);
    pr_info("ccir0_8b_mode:%d\n",proval);
    HalVif_Ccir8BitDataPadConfig(0,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir1_8b_mode", &proval);
    pr_info("ccir1_8b_mode:%d\n",proval);
    HalVif_Ccir8BitDataPadConfig(1,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir2_8b_mode", &proval);
    pr_info("ccir2_8b_mode:%d\n",proval);
    HalVif_Ccir8BitDataPadConfig(2,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir3_8b_mode", &proval);
    pr_info("ccir3_8b_mode:%d\n",proval);
    HalVif_Ccir8BitDataPadConfig(3,proval);


    of_property_read_u32(pdev->dev.of_node, "ccir0_16b_mode", &proval);
    pr_info("ccir0_16b_mode:%d\n",proval);
    HalVif_Ccir16BitDataPadConfig(0,proval);
    of_property_read_u32(pdev->dev.of_node, "ccir2_16b_mode", &proval);
    pr_info("ccir2_16b_mode:%d\n",proval);
    HalVif_Ccir16BitDataPadConfig(2,proval);
#if 0
    if (0 == request_irq(irq_vif, vif_isr, IRQF_TRIGGER_NONE, "vif interrupt", 0))
        pr_info("[VIF] vif irq interrupt registered #%d\n", irq_vif);
    else
        pr_err("[VIF] failed to request vif irq #%d\n",irq_vif);
#endif
    if (0 == request_irq(irq_isp_dma, isp_dma_isr, IRQF_TRIGGER_NONE, "isp dma interrupt", &vif_dev_ctx))
        pr_info("[VIF] isp dma interrupt registered #%d\n", irq_isp_dma);
    else
        pr_err("[VIF] failed to request isp dma #%d\n",irq_isp_dma);


    INIT_WORK(&vif_dev_ctx.isp_dma_wq, isp_dma_do_work);
    spin_lock_init(&vif_dev_ctx.lock);

    if (!inited) {
        vif_dev.minor = MISC_DYNAMIC_MINOR;
        vif_dev.name = "vif";
        vif_dev.fops = &vif_fops;
        vif_dev.parent = &pdev->dev;
        misc_register(&vif_dev);
    }
#if IPC_DMA_ALLOC_COHERENT
    IPCRamVirtAddr = dma_alloc_coherent(NULL, IPC_RAM_SIZE, &IPCRamPhys, GFP_KERNEL);
    pr_info("[DMA coherent] IPCRamVirtAddr:%p, IPCRamPhys:%#x, size:%#x\n", IPCRamVirtAddr, IPCRamPhys, IPC_RAM_SIZE);
#endif

    of_property_read_u32(pdev->dev.of_node, "mcu", &mcu_enable);
    mcu_enable = 1;
    if(mcu_enable)
    {
#if IPC_DMA_ALLOC_COHERENT
        pr_info("IPCRamVirtAddr allocated by DMA coherent!!\n");
#else
        of_property_read_u32(pdev->dev.of_node, "IPCRamPhys", &proval);
        IPCRamPhys = proval;
        IPCRamVirtAddr = ioremap(IPCRamPhys, 0x10000);
        VIF_DEBUG("IPCRamVirtAddr: %p IPCRamPhys: 0x%lx\n", IPCRamVirtAddr, IPCRamPhys);
#endif

        // Write 8051 Share memory Address to mailbox
        REG_W(g_MAILBOX, MAILBOX_HEART_REG, ((Chip_Phys_to_MIU(IPCRamPhys + MCU_WINDOWS0_OFFSET) >> 16) & 0xFFFF));
        REG_W(g_MAILBOX, MAILBOX_STATE_REG, 0x0);
        REG_W(g_MAILBOX, MAILBOX_CONCTRL_REG, 0x0);

        //copy 8051 bin file
        memset(IPCRamVirtAddr,0,IPC_RAM_SIZE);

        if(sizeof(hk51) <= 0x6000){
            memcpy(IPCRamVirtAddr,hk51,sizeof(hk51));

            rc = DrvVif_MCULoadBin();

            if(rc == 0)
                VIF_DEBUG("mcu version: %s\n", mcu_commit);

        }
        else
            VIF_DEBUG("mcu code size over: %s\n");


        rc = initialSHMRing();

        if(rc==-1)
            return -1;

        rc = DrvVif_stopMCU();
        if (rc) {
            pr_err("Init stop MCU fail\n");
        }

        rc = DrvVif_startMCU();
        if (rc) {
            pr_err("Start MCU fail\n");
        }

        rc = DrvVif_pollingMCU();
        if (rc) {
            pr_err("Polling MCU fail\n");
        }

    }

    // Set VIF CLK
    DrvVif_CLK();

    // initial ISR mask
    //DrvVif_SetDefaultIntsMask();

    // reg base addr maapping assign
    if (!inited) {
        DrvVif_SetVifChanelBaseAddr();
    }
    HalVif_GlobalInit();
    //DrvVif_Init();
    if (!inited) {
        HalDma_Init();
    }
    for(i=0;i<VIF_CHANNEL_NUM;i++) {
        HalDma_DmaMaskEnable(i,1);
        HalDma_DisableIrq(i);
    }

    //DrvVif_WdmaFiFoFullHandlerInit();

    DrvVif_CtxInit();
#if 0
    DrvVif_SetDmaLineCntInt(VIF_CHANNEL_0, 1);
    DrvVif_SetDmaLineCnt(VIF_CHANNEL_0, 0x400);
    DrvVif_SetVifFrameStartInt((1<<VIF_CHANNEL_0), 1);

    DrvVif_RegisterFrameStartCb(0,(void *)my_cb_fs, 0, NULL);
    DrvVif_RegisterLineCntCb(0,(void *)my_cb_lc, 0, NULL);
    DrvVif_RegisterLineCntCbTest(0, my_cb);
#endif
    inited = 1;
    vif_dev_ctx.inited = 1;
    return 0;
}

static int mstar_vif_probe(struct platform_device *pdev)
{

    return DrvVif_Init_Common(pdev);
}

s32 DrvVif_Init(void)
{
    int ret = E_HAL_VIF_SUCCESS;
    struct device_node *dev_node;
    struct platform_device *pdev;


    dev_node = of_find_compatible_node(NULL, NULL, "mstar,vif");
    if (!dev_node) {
        return -ENODEV;
    }

    pdev = of_find_device_by_node(dev_node);
    if (!pdev) {
        of_node_put(dev_node);
        return -ENODEV;
    }

    if (DrvVif_Init_Common(pdev)) {
        ret = E_HAL_VIF_ERROR;
    }

    return ret;
}
EXPORT_SYMBOL(DrvVif_Init);

static int DrvVif_Deinit_Common(struct platform_device *pdev)
{
    //int irq_vif = INT_IRQ_83_VIF_INT + 32 + 4;
    int irq_isp_dma = INT_IRQ_119_ISP_DMA_INT + 32;

#if 0
    devm_iounmap(&pdev->dev, g_ptVIF);
    devm_iounmap(&pdev->dev, g_ptVIF2);
    devm_iounmap(&pdev->dev, g_BDMA);
    devm_iounmap(&pdev->dev, g_MCU8051);
    devm_iounmap(&pdev->dev, g_PMSLEEP);
    devm_iounmap(&pdev->dev, g_MAILBOX);
    devm_iounmap(&pdev->dev, g_TOPPAD1);
    devm_iounmap(&pdev->dev, g_ISP_ClkGen);
#endif
    iounmap(g_CLKGEN2);
#if IPC_DMA_ALLOC_COHERENT
    dma_free_coherent(NULL, IPC_RAM_SIZE, IPCRamVirtAddr, IPCRamPhys);
#else
    iounmap(IPCRamVirtAddr);
#endif
    //free_irq(irq_vif, NULL);
    free_irq(irq_isp_dma, &vif_dev_ctx);
    vif_dev_ctx.inited = 0;
    return 0;
}

static int mstar_vif_remove(struct platform_device *pdev)
{
    return DrvVif_Deinit_Common(pdev);
}

s32 DrvVif_Deinit(void)
{
    int ret = E_HAL_VIF_SUCCESS;

    struct device_node *dev_node;
    struct platform_device *pdev;

    dev_node = of_find_compatible_node(NULL, NULL, "mstar,vif");
    if (!dev_node) {
        return -ENODEV;
    }

    pdev = of_find_device_by_node(dev_node);
    if (!pdev) {
        of_node_put(dev_node);
        return -ENODEV;
    }

    if (DrvVif_Deinit_Common(pdev)) {
        ret = E_HAL_VIF_ERROR;
}

    return ret;
}
EXPORT_SYMBOL(DrvVif_Deinit);

static int __init mstar_vif_init_driver(void)
{
	return platform_driver_register(&mstar_vif_driver);
}

static void __exit mstar_vif_exit_driver(void)
{
	platform_driver_unregister(&mstar_vif_driver);
}

subsys_initcall(mstar_vif_init_driver);
module_exit(mstar_vif_exit_driver);

MODULE_DESCRIPTION("MStar VIF driver");
MODULE_AUTHOR("MSTAR");
MODULE_LICENSE("GPL");
