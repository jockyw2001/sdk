///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
#include <linux/types.h>
#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/miscdevice.h>


#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "hal_cmdq_dbg.h"
#include "cam_os_wrapper.h"
#include "drv_cmdq.h"
#include "drvcmdq_io.h"
#include "hal_cmdq_util.h"
#if defined(HAL_I3_SIMULATE)
#define MDRV_DUMMY_REGISTER          0x1123f0
#elif defined(HAL_K6_SIMULATE)
#define MDRV_DUMMY_REGISTER          0x1123f0
#else
u32 MDRV_DUMMY_REGISTER = 0x1235f0;
u32 MDRV_HWDUMMY_REGISTER = 0x1235EE;

#endif
u32                       u32SelectCmdq = E_MHAL_CMDQ_ID_VPE;

int MdrvCmdqIoOpen(struct inode *inode, struct file *filp);
int MdrvCmdqIoRelease(struct inode *inode, struct file *filp);
long MdrvCmdqIoIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);

#if defined(HAL_K6_SIMULATE) || defined(HAL_I3_SIMULATE)
extern void MDrvCmdqAllocateBuffer(void *pPriv);
extern void MDrvCmdqReleaseBuffer(void *pPriv);
#endif

#if defined(HAL_I2_SIMULATE) || defined(CAM_OS_RTK)
extern void MDrvCmdqAllocateBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo);
extern void MDrvCmdqReleaseBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo);
#endif
#if  defined(HAL_I2_SIMULATE)
//MHAL_CMDQ_Mmap_Info_t gCmdqTestMmapInfo;
#endif
struct task_struct*   pThread1;
struct task_struct*   pThread2;
int    gthreadgo=0;
static struct file_operations _gDevFops =
{
    .owner = THIS_MODULE,
    .open = MdrvCmdqIoOpen,
    .release = MdrvCmdqIoRelease,
    .unlocked_ioctl = MdrvCmdqIoIoctl,
};

static struct miscdevice _gCmdqMiscDev =
{
    .minor      = 255,
    .name       = "cmdq",
    .fops       = &_gDevFops,
};
MHAL_CMDQ_CmdqInterface_t      *gpCmdqInterface[E_MHAL_CMDQ_ID_MAX];

int MdrvCmdqIoOpen(struct inode *inode, struct file *filp)
{
    int ret = 0;

    _CDMQ_PROFILE_INIT;

    //DIP_ASSERT(_gDipDev.refCnt>=0);

    return ret;
}

int MdrvCmdqIoRelease(struct inode *inode, struct file *filp)
{
    _CDMQ_PROFILE_INIT;

    return 0;
}
int UT_GetTask1(void *arg)
{
    MHAL_CMDQ_BufDescript_t    tCmdqBufDesp;
    MHAL_CMDQ_CmdqInterface_t      *pCmdqInterface;
    memset(&tCmdqBufDesp, 0x0, sizeof(MHAL_CMDQ_BufDescript_t));
    tCmdqBufDesp.u32CmdqBufSizeAlign = 16;
    tCmdqBufDesp.u32CmdqBufSize  = 16384;
    tCmdqBufDesp.u32MloadBufSize = 0;
    tCmdqBufDesp.u16MloadBufSizeAlign = 0;
    while(gthreadgo)
    {
        pCmdqInterface = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0, &tCmdqBufDesp,    false);

        msleep(1000);
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC0);
    }
    return 0;
}
int UT_GetTask2(void *arg)
{
    MHAL_CMDQ_BufDescript_t    tCmdqBufDesp;
    MHAL_CMDQ_CmdqInterface_t      *pCmdqInterface;
    memset(&tCmdqBufDesp, 0x0, sizeof(MHAL_CMDQ_BufDescript_t));

    tCmdqBufDesp.u32CmdqBufSizeAlign = 16;
    tCmdqBufDesp.u32CmdqBufSize  = 16384;
    tCmdqBufDesp.u32MloadBufSize = 0;
    tCmdqBufDesp.u16MloadBufSizeAlign = 0;
    while(gthreadgo)
    {
        pCmdqInterface = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC1, &tCmdqBufDesp,    false);
         msleep(1000);
        MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_H265_VENC1);
    }
    return 0;
}

long MdrvCmdqIoIoctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{

    s32                       nTstArg = 0;
    u32                       u32Regval;
    u16                       u16Regval;

    //CmdqInterfacePriv_t * pPrivData = (CmdqInterfacePriv_t *)gpCmdqInterface->ctx;



    if(copy_from_user(&nTstArg, (int __user *)u32Arg, sizeof(int)))
    {
        return -EFAULT;
    }


    switch(u32Cmd)
    {
        /*dummy register test*/
        case IOCTL_CMDQ_TEST_1:
            CMDQ_LOG("%s %d\n", __func__, __LINE__);
            if(nTstArg == 0)
            {
#if  defined(HAL_I2_SIMULATE)
                u32                        i;
#endif
                MS_BOOL                    bForceRiu;
                MHAL_CMDQ_BufDescript_t    tCmdqBufDesp;
#if 0
#if  defined(HAL_I2_SIMULATE)
                CMDQ_LOG("MHAL_CMDQ_InitCmdqMmapInfo\n");
                memset(&gCmdqTestMmapInfo,0x0,sizeof(MHAL_CMDQ_Mmap_Info_t));
                MDrvCmdqAllocateBuffer(&gCmdqTestMmapInfo);
                MHAL_CMDQ_InitCmdqMmapInfo(&gCmdqTestMmapInfo);
#endif
#endif

                bForceRiu = false;
                memset(&tCmdqBufDesp, 0x0, sizeof(MHAL_CMDQ_BufDescript_t));
                tCmdqBufDesp.u32CmdqBufSizeAlign = 16;
                tCmdqBufDesp.u32CmdqBufSize  = 4096;
                tCmdqBufDesp.u32MloadBufSize = 4096;
                tCmdqBufDesp.u16MloadBufSizeAlign = 16;
#if  defined(HAL_I2_SIMULATE)
                for(i = 0 ; i<E_MHAL_CMDQ_ID_MAX;i++)
                {
                    gpCmdqInterface[i] = MHAL_CMDQ_GetSysCmdqService(i, &tCmdqBufDesp, bForceRiu);
                    if(gpCmdqInterface[i] == NULL)
                        CMDQ_ERR("get_sys_cmdq_service cmdq(%d) fail\n",i);
                     CMDQ_ERR("get_sys_cmdq_service cmdq(%d) success\n",i);

                }
#else
                gpCmdqInterface[E_MHAL_CMDQ_ID_VPE]  = MHAL_CMDQ_GetSysCmdqService(E_MHAL_CMDQ_ID_VPE, &tCmdqBufDesp, bForceRiu);

                if(gpCmdqInterface[E_MHAL_CMDQ_ID_VPE] == NULL)
                    CMDQ_ERR("get_sys_cmdq_service cmdq(%d) fail\n",E_MHAL_CMDQ_ID_VPE);
#endif

            }
            else if(nTstArg == 1)
            {
                MDrvCmdDbgWriteDummy(gpCmdqInterface[u32SelectCmdq]->pCtx, 0x55aa);
                if(MDrvCmdDbgReadDummy(gpCmdqInterface[u32SelectCmdq]->pCtx) != 0x55aa)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                    CMDQ_LOG("dummy register write error(0x%x)\n", u16Regval);
                }
                else
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                    CMDQ_LOG("dummy register write success(0x%x)\n", u16Regval);

                }
                //(*((volatile u16*)(0xfd246be0))) = (u16)(0xaabb);
                //W2BYTE(pPrivData->nCmdqRiuBase,0x123578,0xaabb);
                //CMDQ_LOG("dummy register read data(0x%x)\n", (*(volatile u16*)(0xfd246be0)));

            }
            else if(nTstArg == 2)
            {
                //u32 nRegval;
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x),select cmdq=%d\n", u32Regval,u32SelectCmdq);
                MDrvCmdqDumpContext();
            }
            else if(nTstArg == 3)/*WRITE COMMAND TEST*/
            {
                u32 nDumAddr;
                /*write command test*/
                CMDQ_LOG("%s %d\n", __func__, __LINE__);
                nDumAddr = MDrvGetDummyRegisterRiuAddr(gpCmdqInterface[u32SelectCmdq]->pCtx);
                CMDQ_LOG("%s %d riu address=0x%08x\n", __func__, __LINE__, nDumAddr);
                //for(i=0;i<7;i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], (MDRV_DUMMY_REGISTER), 0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], 0xdede);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                CMDQ_LOG("%s %d\n", __func__, __LINE__);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                CMDQ_LOG("dummy register read(0x%x)\n", u16Regval);
                CMDQ_LOG("dummy register read data(0x%x)\n", (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)));
                //(*((volatile u16*)(0x130000UL+(0x36<<8)+(0x27)+0xfd000000))) =  (u16)(0xaabb);
                (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xbb);
                CMDQ_LOG("dummy register read data(0x%x)\n", (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)));
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                CMDQ_LOG("dummy register read(0x%x)\n", u16Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                // CMDQ_LOG("dummy register read data(0x%x)\n", (*(volatile u16*)((0x13364e<<1)+0xfd200000)));
            }
            else if(nTstArg == 4)/*CMDQ OVER FLOW TEST*/
            {
                u32 i, num = 0;
                for(i = 0; i < 500; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                CMDQ_LOG("%s %d\n", __func__, __LINE__);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = 500; i < 600; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                CMDQ_LOG("dummy register read(0x%x)\n", u16Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 5)//test poll eq command
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xcc, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00, 1);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xbb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
#if 1
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

#endif
            }
            else if(nTstArg == 6)//test poll neq command
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00, 0);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xbb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xef00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
            }
            else if(nTstArg == 7)//reset engine
            {
                CMDQ_LOG("reset cmdq engine\n");
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

                msleep(1);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqResetEngine(gpCmdqInterface[u32SelectCmdq]);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

            }
            else if(nTstArg == 8)//ring buffer test for multi round
            {
                u32 i = 0, j, num = 0;
                for(j = 0 ; j < 5; j++)
                {
                    i = j * 700;
                    if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 500)) > 0)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                        for(i = j * 700; i < (j * 700 + 500); i++)
                        {
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        }
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                        CMDQ_LOG("%s %d\n", __func__, __LINE__);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    }
                    /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                    if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                        for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                        {
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                            //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                        }
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                    }
                    if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 100)) > 0)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                        for(i = (j * 700 + 500) + 100; i < ((j * 700 + 500) + 100) + 100; i++)
                        {
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                            //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                        }
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                    }
                    CMDQ_LOG("dummy register read(0x%x)\n", u16Regval);
                }
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 9)//abord command test
            {
                u32 i = 0, j = 0, num = 0;
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 500)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = j * 700; i < (j * 700 + 500); i++)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    }
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                    CMDQ_LOG("%s %d\n", __func__, __LINE__);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                }
                /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                    //gpCmdqInterface->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAbortBuffer(gpCmdqInterface[u32SelectCmdq]);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 10)//abord command test ,then do again
            {
                u32 i = 0, j = 0, num = 0;
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 500)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = j * 700; i < (j * 700 + 500); i++)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    }
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                    CMDQ_LOG("%s %d\n", __func__, __LINE__);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                }
                /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                    //gpCmdqInterface->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAbortBuffer(gpCmdqInterface[u32SelectCmdq]);
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 11)//idle function check
            {
                u32 i = 0, j = 0, num = 0;
                MS_BOOL    bidel;
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 500)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("11read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = j * 700; i < (j * 700 + 500); i++)
                    {
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    }
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                    CMDQ_LOG("%s %d\n", __func__, __LINE__);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_IsCmdqEmptyIdle(gpCmdqInterface[u32SelectCmdq], &bidel);
                    CMDQ_LOG("%s %d idle=0x%x\n", __func__, __LINE__, bidel);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("22read irq status(0x%08x)(%d)\n", u32Regval, num);

                }

                do
                {
                    CMDQ_LOG("%s %d\n", __func__, __LINE__);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("33read irq status(0x%08x)(%d)\n", u32Regval, num);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_IsCmdqEmptyIdle(gpCmdqInterface[u32SelectCmdq], &bidel);
                    CMDQ_LOG("%s %d idle=0x%x\n", __func__, __LINE__, bidel);
                    if(bidel)
                    {
                        for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                        {
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                            //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                        }
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                        CMDQ_LOG("44 read irq status(0x%08x)\n", u32Regval);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    }
                    else
                    {
                        CMDQ_LOG("CMDQ_IsCmdqEmptyIdle is not idel\n");
                        msleep(1);

                    }
                }
                while(!bidel);

                /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("55 read irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("66 read irq status(0x%08x)\n", u32Regval);
                    //gpCmdqInterface->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAbortBuffer(gpCmdqInterface[u32SelectCmdq]);
                if((num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CheckBufAvailable(gpCmdqInterface[u32SelectCmdq], 300)) > 0)
                {
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("77 ad irq status(0x%08x)(%d)\n", u32Regval, num);
                    for(i = (j * 700 + 500); i < (j * 700 + 500) + 100; i++)
                    {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                    CMDQ_LOG("88 read irq status(0x%08x)\n", u32Regval);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                }
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 12)//riu mode test
            {
                MS_BOOL                    bForceRiu;
                MHAL_CMDQ_BufDescript_t    tCmdqBufDesp;


                bForceRiu = true;
                memset(&tCmdqBufDesp, 0x0, sizeof(MHAL_CMDQ_BufDescript_t));
                gpCmdqInterface[u32SelectCmdq] = MHAL_CMDQ_GetSysCmdqService(u32SelectCmdq, &tCmdqBufDesp, bForceRiu);

                if(gpCmdqInterface[u32SelectCmdq] == NULL)
                    CMDQ_ERR("get_sys_cmdq_service fail\n");

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], MDRV_DUMMY_REGISTER, 0xaabb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                CMDQ_LOG("11dummy register(0x%08x)\n", u16Regval);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], MDRV_DUMMY_REGISTER, 0xccdd);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadDummyRegCmdq(gpCmdqInterface[u32SelectCmdq], &u16Regval);
                CMDQ_LOG("22dummy register(0x%08x)\n", u16Regval);

            }
            else if(nTstArg == 13)//menuload mode test
            {
                char * buff;
                MS_PHYADDR phyaddr[5];
                MS_PHYADDR wphyaddr[5];
                buff = kmalloc(1024, GFP_KERNEL);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[0]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[0]);
                CMDQ_LOG("menuload addr0=0x%x\n", (u32)phyaddr[0]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[1]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[1]);
                CMDQ_LOG("menuload addr1=0x%x\n", (u32)phyaddr[1]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[2]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[2]);
                CMDQ_LOG("menuload addr2=0x%x\n", (u32)phyaddr[2]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface[u32SelectCmdq], wphyaddr[0]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface[u32SelectCmdq], wphyaddr[1]);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 2048, 16, &phyaddr[3]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[3]);

                CMDQ_LOG("menuload addr3=0x%x\n", (u32)phyaddr[3]);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface[u32SelectCmdq], wphyaddr[2]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface[u32SelectCmdq], wphyaddr[3]);
                kfree(buff);
            }
            else if(nTstArg == 14)//menuload mode test wait buffer
            {
                char * buff;
                MS_PHYADDR phyaddr[5];
                MS_PHYADDR wphyaddr[5];
                buff = kmalloc(1024, GFP_KERNEL);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[0]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[0]);
                CMDQ_LOG("menuload addr0=0x%x\n", (u32)phyaddr[0]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[1]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[1]);
                CMDQ_LOG("menuload addr1=0x%x\n", (u32)phyaddr[1]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 1024, 16, &phyaddr[2]);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface[u32SelectCmdq], &wphyaddr[2]);
                CMDQ_LOG("menuload addr2=0x%x\n", (u32)phyaddr[2]);
                //gpCmdqInterface->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface,wphyaddr[0]);
                //gpCmdqInterface->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface,wphyaddr[1]);

                if(gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_MloadCopyBuf(gpCmdqInterface[u32SelectCmdq], buff, 2048, 16, &phyaddr[3]) != 0)
                    CMDQ_LOG("menuload copy fail addr3=0x%x\n", (u32)phyaddr[3]);
                //gpCmdqInterface->MHAL_CMDQ_GetNextMloadRingBufWritePtr(gpCmdqInterface,&wphyaddr[3]);

                //CMDQ_LOG("menuload addr3=0x%x\n",(u32)phyaddr[3]);

                //gpCmdqInterface->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface,wphyaddr[2]);
                //gpCmdqInterface->MHAL_CMDQ_UpdateMloadRingBufReadPtr(gpCmdqInterface,wphyaddr[3]);
                kfree(buff);
            }
            else if(nTstArg == 15)//wait command success test
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x1234,0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x8000, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_REG_DUMMY_TRIG);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x5678, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x0, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)),0x9ABC, 0xFFFF);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
#if 1
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

#endif
            }
            else if(nTstArg == 16)
            {
#if  defined(HAL_I2_SIMULATE)
                u32   i;
                for(i = 0 ; i< E_MHAL_CMDQ_ID_MAX ; i++)
                {
                   MHAL_CMDQ_ReleaseSysCmdqService(i);
                }
#if 0
                MDrvCmdqReleaseBuffer(&gCmdqTestMmapInfo);
#endif
#else
                MHAL_CMDQ_ReleaseSysCmdqService(E_MHAL_CMDQ_ID_VPE);
#if  defined(HAL_I2_SIMULATE)
 //             MDrvCmdqReleaseBuffer(&gCmdqTestMmapInfo);
#endif

#endif
            }
            else if(nTstArg == 17)
            {
               u32SelectCmdq = E_MHAL_CMDQ_ID_DIVP;
               MDRV_DUMMY_REGISTER = 0x123CF0;
               MDRV_HWDUMMY_REGISTER = 0x123CEE;
               CMDQ_LOG("select cmdq=%d\n",u32SelectCmdq);
            }
            else if(nTstArg == 18)
            {
               u32SelectCmdq = E_MHAL_CMDQ_ID_H265_VENC0;
               MDRV_DUMMY_REGISTER = 0x123DF0;
               MDRV_HWDUMMY_REGISTER = 0x123DEE;
               CMDQ_LOG("select cmdq=%d\n",u32SelectCmdq);
            }
            else if(nTstArg == 19)
            {
               u32SelectCmdq = E_MHAL_CMDQ_ID_H265_VENC1;
               MDRV_DUMMY_REGISTER = 0x123EF0;
               MDRV_HWDUMMY_REGISTER = 0x123EEE;
               CMDQ_LOG("select cmdq=%d\n",u32SelectCmdq);
            }
            else if(nTstArg == 20)
            {
               u32SelectCmdq = E_MHAL_CMDQ_ID_H264_VENC0;
               MDRV_DUMMY_REGISTER = 0x122FF0;
               MDRV_HWDUMMY_REGISTER = 0x122FEE;
               CMDQ_LOG("select cmdq=%d\n",u32SelectCmdq);
            }
            else if(nTstArg == 21)
            {
               u32SelectCmdq = E_MHAL_CMDQ_ID_VPE;
               MDRV_DUMMY_REGISTER = 0x1235F0;
               MDRV_HWDUMMY_REGISTER = 0x1235EE;
               CMDQ_LOG("select cmdq=%d\n",u32SelectCmdq);
            }
            else if(nTstArg == 22)//wait command success test FOR TIME OUT
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x1234,0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x0000, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_REG_DUMMY_TRIG);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x5678, 0xFFFF);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
#if 1
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

#endif
            }
            else if(nTstArg == 23)
            {
#if  defined(HAL_I2_SIMULATE)
                CMDQ_LOG("MHAL_CMDQ_InitCmdqMmapInfo 23\n");
                //memset(&gCmdqTestMmapInfo,0x0,sizeof(MHAL_CMDQ_Mmap_Info_t));
                //MDrvCmdqAllocateBuffer(&gCmdqTestMmapInfo);
               // MHAL_CMDQ_InitCmdqMmapInfo(&gCmdqTestMmapInfo);
#endif
            }
            else if(nTstArg == 24)
            {
#if  defined(HAL_I2_SIMULATE)
                //MDrvCmdqReleaseBuffer(&gCmdqTestMmapInfo);
#endif
            }
            else if(nTstArg == 25)/*WAIT for multi event , timeout*/
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x1234,0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x8000, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq],(E_MHAL_CMDQEVE_REG_DUMMY_TRIG|E_MHAL_CMDQEVE_CORE1_MFE_TRIG));
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x5678, 0xFFFF);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

            }
            else if(nTstArg == 26)/*WAIT for trigger same test*/
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x1234,0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x8000, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq],(E_MHAL_CMDQEVE_REG_DUMMY_TRIG));
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq],((MDRV_HWDUMMY_REGISTER)),0x0000, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x5678, 0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq],(E_MHAL_CMDQEVE_REG_DUMMY_TRIG));
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)),0xcdef, 0xFFFF);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

            }
            else if(nTstArg == 27)/*test hw dummy irq*/
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x1234,0xFFFF);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x8000, 0xFFFF);
                if(u32SelectCmdq != E_MHAL_CMDQ_ID_H265_VENC0)
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_REG_DUMMY_TRIG);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x5678, 0xFFFF);
                //gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_HWDUMMY_REGISTER)),0x0, 0xFFFF);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
            }
            else if(nTstArg == 28)/*test tag debug*/
            {
                u32 i;
                for(i = 0; i < 500; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,)
                CMDQ_LOG("%s %d\n", __func__, __LINE__);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                /*set 300 to check block mode , set 100 for multi fire& buffer over flow*/
                for(i = 500; i < 600; i++)
                {
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), i);
                        //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                (*(volatile u16*)((0x1525ca * 2) + 0xfd000000)) = (u16)(0x33);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdqByTag(gpCmdqInterface[u32SelectCmdq],0x33);
                msleep(1000);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);

                for(i = 600; i < 700; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)),   i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdqByTag(gpCmdqInterface[u32SelectCmdq],0x55);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 29)//test poll eq command timeout
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xcc, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00, 1);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xbb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
#if 1
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                //(*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

#endif
            }
            else if(nTstArg == 30)//test poll neq command timeout
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00, 0);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xbb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                //(*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xef00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
            }
            else if(nTstArg == 31)//test poll neq command timeout
            {
                //CamOsTimespec_t ptRes;
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdDelay(gpCmdqInterface[u32SelectCmdq],10000);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00,1);


                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);


                msleep(5);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);

                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
            }
            else if(nTstArg == 32)//no round print cmd
            {
                u32 i;
                for(i = 0; i < 450; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);

                for(i = 0; i < 100; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_SC_TRIG013);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(5);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 33)//no round print cmd
            {
                u32 i;
                for(i = 0; i < 500; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_SC_TRIG013);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(5);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 34)//direct mode
            {
                u32 i;
                for(i = 0; i < 100; i++)
                {
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    //gpCmdqInterface->write_reg_cmdq(gpCmdqInterface,((0x1235f0)),0xaa);
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                    //gpCmdqInterface->cmdq_poll_reg_bits(gpCmdqInterface,((0x1235f0)),0xaa,0xffff,1);
                }
                //gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqAddWaitEventCmd(gpCmdqInterface[u32SelectCmdq], E_MHAL_CMDQEVE_SC_TRIG013);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(5);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
            }
            else if(nTstArg == 35)//direct mode
            {
                pThread1 =  kthread_create(UT_GetTask1,NULL, "thest1");
                pThread2 =  kthread_create(UT_GetTask2,NULL, "thest2");
                gthreadgo =1;
                wake_up_process(pThread1);
                wake_up_process(pThread2);
            }
            else if(nTstArg == 36)//direct mode
            {
                gthreadgo =0;
            }
			else if(nTstArg == 37)//direct mode
            {
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xaa, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xcc, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_CmdqPollRegBits_ByTime(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xff00, 0xff00, 1,100000);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMask(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0xbb, 0xff);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval) ;
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
#if 1
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                msleep(1);
                (*(volatile u16*)((MDRV_DUMMY_REGISTER * 2) + 0xfd000000)) = (u16)(0xff00);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);
                MDrvCmdqPrintfCrashCommand(gpCmdqInterface[u32SelectCmdq]->pCtx);
                msleep(5);
                gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_ReadStatusCmdq(gpCmdqInterface[u32SelectCmdq], &u32Regval);
                CMDQ_LOG("read irq status(0x%08x)\n", u32Regval);

#endif
            }
            else if(nTstArg == 38)  /*CMDQ Non-Multiple CMD Write TEST*/
            {
#define _CDMQ_UT_BUFFER_SIZE    500
#define _CDMQ_UT_LOOP_COUNT     2
                u32 i = 0;
                u32 j = 0;

                CMDQ_LOG("%s %d\n", __func__, __LINE__);

                for(j=0;j<_CDMQ_UT_LOOP_COUNT;j++)
                {
                    for(i = 0; i < _CDMQ_UT_BUFFER_SIZE; i++)
                    {
                        if(i%2)
                        {
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        }
                        else {
                            gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdq(gpCmdqInterface[u32SelectCmdq], ((MDRV_DUMMY_REGISTER)), 0x0 + i);
                        }
                    }
                    gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                }
            }
            else if(nTstArg == 39)  /*CMDQ Multiple CMD Write TEST*/
            {
                MHAL_CMDQ_MultiCmdBuf_t *pbuf;
                u32 i, num = 0;
                u32 j = 0;

                CMDQ_LOG("%s %d\n", __func__, __LINE__);

                pbuf = kmalloc(sizeof(MHAL_CMDQ_MultiCmdBuf_t)*_CDMQ_UT_BUFFER_SIZE, GFP_KERNEL);
                for(i = 0; i < _CDMQ_UT_BUFFER_SIZE; i++)
                {
                    pbuf[i].u32RegAddr = MDRV_DUMMY_REGISTER;
                    pbuf[i].u32RegValue = i;
                }

                for(j=0;j<_CDMQ_UT_LOOP_COUNT;j++)
                {
                    u32 num_remain = _CDMQ_UT_BUFFER_SIZE;

                    do
                    {
                        num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMulti(gpCmdqInterface[u32SelectCmdq], (pbuf+_CDMQ_UT_BUFFER_SIZE-num_remain), num_remain);
                        num_remain -= num;
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                        if(num_remain)
                        {
                            CMDQ_LOG("remain(%d)\n", num_remain);
                        }
                    } while(num_remain);
                }
            }
            else if(nTstArg == 40)  /*CMDQ Multiple CMD Write with MASK TEST*/
            {
                MHAL_CMDQ_MultiCmdBufMask_t *pbuf;
                u32 i, num = 0;
                u32 j = 0;

                CMDQ_LOG("%s %d\n", __func__, __LINE__);

                pbuf = kmalloc(sizeof(MHAL_CMDQ_MultiCmdBuf_t)*_CDMQ_UT_BUFFER_SIZE, GFP_KERNEL);
                for(i = 0; i < _CDMQ_UT_BUFFER_SIZE; i++)
                {
                    pbuf[i].u32RegAddr = MDRV_DUMMY_REGISTER;
                    pbuf[i].u16RegValue = i | 0xF000;
                    pbuf[i].u16Mask = (0x7FFF);
                }

                for(j=0;j<_CDMQ_UT_LOOP_COUNT;j++)
                {
                    u32 num_remain = _CDMQ_UT_BUFFER_SIZE;

                    do
                    {
                        num = gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_WriteRegCmdqMaskMulti(gpCmdqInterface[u32SelectCmdq], (pbuf+_CDMQ_UT_BUFFER_SIZE-num_remain), num_remain);
                        num_remain -= num;
                        gpCmdqInterface[u32SelectCmdq]->MHAL_CMDQ_KickOffCmdq(gpCmdqInterface[u32SelectCmdq]);
                        if(num_remain)
                        {
                            CMDQ_LOG("remain(%d)\n", num_remain);
                        }
                    } while(num_remain);
                }
            }
            else
            {
                CMDQ_LOG("no support(%d)\n", nTstArg);
            }
            break;
        default:
            CMDQ_LOG("no support test(%d-%d)\n", u32Cmd, nTstArg);
    }

    return 0;
}

int _MdrvCmdqIoInit(void)
{

    int ret;

    _CDMQ_PROFILE_INIT;

    ret = misc_register(&_gCmdqMiscDev);
    if(ret != 0)
    {
        CMDQ_LOG("cannot register miscdev on minor=11 (err=%d)\n", ret);
    }
    return ret;
}


void _MdrvCmdqIoExit(void)
{
    misc_deregister(&_gCmdqMiscDev);
}

module_init(_MdrvCmdqIoInit);
module_exit(_MdrvCmdqIoExit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("ms cmdq driver");
MODULE_LICENSE("GPL");
