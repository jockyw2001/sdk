#include "mi_syscfg.h"
#include "mi_common_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kmod.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
//#include <mstar/mstar_chip.h>
#include "mi_sys_internal.h"

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40)
#include <mstar/mstar_chip.h>
#elif LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
#include <mdrv_miu.h>
#include <mdrv_mma_heap.h>
#include <mstar_chip.h>
#else
#error not support this kernel version
#endif

#include "mhal_common.h"
#include "mhal_cmdq.h"

static MI_SYSCFG_MemoryInfo_t _stMemoryInfo;

static struct {
    const char *name;
    MI_SYSCFG_MmapInfo_t mmapInfo;
} _stMmapInfo[100];

static MI_S32 MI_SYSCFG_SetMmap(const char *buf, MI_U64 off, MI_S32 v){
    char name[32];
    unsigned int u32[8];
    //printk("in %s:%d   off=0x%llx\n",__FUNCTION__,__LINE__,off);
    if(off >= _stMemoryInfo.u32MmapItemsNum){
        printk("in %s:%d  \n",__FUNCTION__,__LINE__);
        return -ENOKEY;
    }
    sscanf(buf, "%s%u%u%u%u%u%u%u%u", name,
        u32+0, u32+1, u32+2, u32+3, u32+4, u32+5, u32+6, u32+7);
    //printk("in %s:%d   buf=%s\n",__FUNCTION__,__LINE__,buf);
    _stMmapInfo[off].name = kstrdup(name, GFP_KERNEL);
    _stMmapInfo[off].mmapInfo.u8Gid = u32[0];
    _stMmapInfo[off].mmapInfo.u32Addr = u32[1];
    _stMmapInfo[off].mmapInfo.u32Size = u32[2];
    _stMmapInfo[off].mmapInfo.u8Layer = u32[3];
    _stMmapInfo[off].mmapInfo.u32Align = u32[4];
    _stMmapInfo[off].mmapInfo.u32MemoryType = u32[5];
    _stMmapInfo[off].mmapInfo.u8MiuNo = u32[6];
    _stMmapInfo[off].mmapInfo.u8CMAHid = u32[7];
    return v;
}

static MI_S32 MI_SYSCFG_ShowMmap(char *buf, MI_U64 off, MI_S32 v){
    //printk("in %s:%d\n",__FUNCTION__,__LINE__);
    if(off >= _stMemoryInfo.u32MmapItemsNum){
        printk("in %s:%d\n",__FUNCTION__,__LINE__);
        return -ENOKEY;
    }
    sprintf(buf, "%s:GID=%d,Addr=0x%x,Size=0x%x,Layer=%d,Align=0x%x,MemoryType=0x%x,MiuNo=%d,CMAID=%d\n",
                _stMmapInfo[off].name,
                _stMmapInfo[off].mmapInfo.u8Gid,
                _stMmapInfo[off].mmapInfo.u32Addr,
                _stMmapInfo[off].mmapInfo.u32Size,
                _stMmapInfo[off].mmapInfo.u8Layer,
                _stMmapInfo[off].mmapInfo.u32Align,
                _stMmapInfo[off].mmapInfo.u32MemoryType,
                _stMmapInfo[off].mmapInfo.u8MiuNo,
                _stMmapInfo[off].mmapInfo.u8CMAHid);
    //printk("in %s:%d buf=%s\n",__FUNCTION__,__LINE__,buf);
    return v;
}

static MI_S32 MI_SYSCFG_SetMemory(MI_SYS_DEBUG_HANDLE_t  handle,const char **args, MI_S32 count){
    //printk("%s:%d\n",__FUNCTION__,__LINE__);
    if(count == 7){
        MI_U32 u32;
        kstrtou32(args[0], 0, &u32);
        _stMemoryInfo.u32TotalSize = u32;
        kstrtou32(args[1], 0, &u32);
        _stMemoryInfo.u32Miu0Size = u32;
        kstrtou32(args[2], 0, &u32);
        _stMemoryInfo.u32Miu1Size = u32;
        kstrtou32(args[3], 0, &u32);
        _stMemoryInfo.u32MiuBoundary = u32;
        kstrtou32(args[4], 0, &u32);
        _stMemoryInfo.u32MmapItemsNum = u32;
        kstrtou32(args[5], 0, &u32);
        _stMemoryInfo.bIs4kAlign = u32;
        kstrtou32(args[6], 0, &u32);
        _stMemoryInfo.bMiu1Enable = u32;
        return count;
    }
    return -EINVAL;
}

static MI_S32 MI_SYSCFG_ShowMemory(MI_SYS_DEBUG_HANDLE_t  handle){
    //printk("%s:%d\n",__FUNCTION__,__LINE__);
    if(_stMemoryInfo.u32MmapItemsNum > 0){
        return handle.OnPrintOut(handle, "u32TotalSize:0x%x\n"
                "u32Miu0Size:0x%x\n"
                "u32Miu1Size:0x%x\n"
                "u32MiuBoundary:0x%x\n"
                "u32MmapItemsNum:%u\n"
                "bIs4kAlign:%d\n"
                "bMiu1Enable:%d\n",
                _stMemoryInfo.u32TotalSize,
                _stMemoryInfo.u32Miu0Size,
                _stMemoryInfo.u32Miu1Size,
                _stMemoryInfo.u32MiuBoundary,
                _stMemoryInfo.u32MmapItemsNum,
                _stMemoryInfo.bIs4kAlign,
                _stMemoryInfo.bMiu1Enable);
    }
    return -ENOKEY;
}

unsigned long long _MI_SYSCFG_IMPL_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr)
{
    if(miu_phy_addr >= (MI_PHY)ARM_MIU2_BASE_ADDR)
    {
        return (MI_PHY)(miu_phy_addr + ARM_MIU2_BUS_BASE-ARM_MIU2_BASE_ADDR);
    }
    else if(miu_phy_addr >= (MI_PHY)ARM_MIU1_BASE_ADDR)
    {
        return  (MI_PHY)(miu_phy_addr + ARM_MIU1_BUS_BASE-ARM_MIU1_BASE_ADDR);
    }
    else if(miu_phy_addr >= (MI_PHY)ARM_MIU0_BASE_ADDR)
    {
        return (MI_PHY)(miu_phy_addr + ARM_MIU0_BUS_BASE-ARM_MIU0_BASE_ADDR);
    }
    else
    {
        printk("miu_phy_addr=0x%llx\n",miu_phy_addr);
        BUG();
    }
    return (MI_PHY)-1ULL;
}

void * _MI_SYSCFG_IMPL_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size , MI_BOOL bCache)
{
    struct vm_struct *area;
    unsigned long pfn, count;
    unsigned long addr;
    unsigned long long cpu_bus_addr;
    int err;
    pgprot_t pgprot;
    BUG_ON(!u32Size);

    if (bCache)
        pgprot = PAGE_KERNEL;
    else
        pgprot = pgprot_writecombine(PAGE_KERNEL);

    cpu_bus_addr = _MI_SYSCFG_IMPL_Miu2Cpu_BusAddr(u64PhyAddr);

    pfn = __phys_to_pfn(cpu_bus_addr);
    count = PAGE_ALIGN(u32Size) / PAGE_SIZE;

    area = get_vm_area_caller((count << PAGE_SHIFT), VM_MAP, __builtin_return_address(0));
    if(!area)
    {
        printk(KERN_DEBUG"get_vm_area_caller failed\n");
        return NULL;
    }

    area->phys_addr = cpu_bus_addr;
    addr = (unsigned long)area->addr;

    err = ioremap_page_range(addr, addr + count*PAGE_SIZE,  cpu_bus_addr,pgprot);
    if(err)
    {
        vunmap((void *)addr);
        return NULL;
    }
    return (void*)addr;
}

void MI_SYSCFG_InitCmdqMmapInfo(void)
{
    //cmdq
    MI_SYSCFG_MmapInfo_t *pstMmap = NULL;
    char *pKernelVa = NULL;
    MHAL_CMDQ_Mmap_Info_t stCmdqMmap = {0};
    const char *name ="E_MMAP_ID_CMDQ";
    MI_BOOL brt = FALSE;
    brt = MI_SYSCFG_GetMmapInfo(name, (const MI_SYSCFG_MmapInfo_t **)&pstMmap);

    if (brt)
    {
        stCmdqMmap.u8CmdqMmapGid = pstMmap->u8Gid;// Mmap ID
        stCmdqMmap.u8CmdqMmapLayer = pstMmap->u8Layer;                       // Memory Layer
        stCmdqMmap.u8CmdqMmapMiuNo = pstMmap->u8MiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
        stCmdqMmap.u8CmdqMmapCMAHid = pstMmap->u8CMAHid;                      // Memory CMAHID
        stCmdqMmap.u32CmdqMmapPhyAddr = pstMmap->u32Addr;                      // phy Memory Address
        //stCmdqMmap.u32CmdqMmapVirAddr = 0;                       // vir Memory Address
        stCmdqMmap.u32CmdqMmapSize = pstMmap->u32Size;                       // Memory Size
        stCmdqMmap.u32CmdqMmapAlign = pstMmap->u32Align;                      // Memory Align
        stCmdqMmap.u32CmdqMmapMemoryType = pstMmap->u32MemoryType;
        pKernelVa = _MI_SYSCFG_IMPL_Vmap(stCmdqMmap.u32CmdqMmapPhyAddr, stCmdqMmap.u32CmdqMmapSize, 0);
        stCmdqMmap.u32CmdqMmapVirAddr = (MS_U32)pKernelVa;

        stCmdqMmap.u8MloadMmapGid = 0;                         // Mmap ID
        stCmdqMmap.u8MloadMmapLayer = 0;                        // Memory Layer
        stCmdqMmap.u8MloadMmapMiuNo = 0;                        // 0: MIU0 / 1: MIU1 / 2: MIU2
        stCmdqMmap.u8MloadMmapCMAHid = 0;                       // Memory CMAHID
        stCmdqMmap.u32MloadMmapPhyAddr = 0;                        //phy Memory Address
        stCmdqMmap.u32MloadMmapVirAddr = 0;                        //vir Memory Address
        stCmdqMmap.u32MloadMmapSize = 0;                       // Memory Size
        stCmdqMmap.u32MloadMmapAlign = 0;                      // Memory Align
        stCmdqMmap.u32MloadMmapMemoryType = 0;
        printk(KERN_DEBUG"cmdq: u32Addr=%#x, u32Size=%#x pKernelVa=%p.\n", stCmdqMmap.u32CmdqMmapPhyAddr, stCmdqMmap.u32CmdqMmapSize, pKernelVa);

    }
    else
    {
        printk(KERN_DEBUG"MI_SYSCFG_InitCmdqMmapInfo error: not found mmap id:%s.\n", name);
    }

    name = "E_MMAP_ID_VPE_MLOAD";

    brt = MI_SYSCFG_GetMmapInfo(name, (const MI_SYSCFG_MmapInfo_t **)&pstMmap);

    if (brt)
    {
        stCmdqMmap.u8MloadMmapGid = pstMmap->u8Gid;                        // Mmap ID
        stCmdqMmap.u8MloadMmapLayer = pstMmap->u8Layer;                    // Memory Layer
        stCmdqMmap.u8MloadMmapMiuNo = pstMmap->u8MiuNo;                    // 0: MIU0 / 1: MIU1 / 2: MIU2
        stCmdqMmap.u8MloadMmapCMAHid = pstMmap->u8CMAHid;                      // Memory CMAHID
        stCmdqMmap.u32MloadMmapPhyAddr = pstMmap->u32Addr;                      // phy Memory Address
        //stCmdqMmap.u32MloadMmapVirAddr = 0;                        //vir Memory Address
        stCmdqMmap.u32MloadMmapSize = pstMmap->u32Size;                       // Memory Size
        stCmdqMmap.u32MloadMmapAlign = pstMmap->u32Align;                      // Memory Align
        stCmdqMmap.u32MloadMmapMemoryType = pstMmap->u32MemoryType;

        pKernelVa = _MI_SYSCFG_IMPL_Vmap(stCmdqMmap.u32MloadMmapPhyAddr, stCmdqMmap.u32MloadMmapSize, 0);
        stCmdqMmap.u32MloadMmapVirAddr = (MS_U32)pKernelVa;
        printk(KERN_DEBUG "mload: u32Addr=%#x, u32Size=%#x pKernelVa=%p.\n", stCmdqMmap.u32MloadMmapPhyAddr, stCmdqMmap.u32MloadMmapSize, pKernelVa);
        MHAL_CMDQ_InitCmdqMmapInfo(&stCmdqMmap);
    }
    else
    {
        printk(KERN_DEBUG "MI_SYSCFG_InitCmdqMmapInfo error: not found mmap id:%s.\n", name);
        MHAL_CMDQ_InitCmdqMmapInfo(&stCmdqMmap);
    }

}
EXPORT_SYMBOL(MI_SYSCFG_InitCmdqMmapInfo);

MI_BOOL MI_SYSCFG_GetMmapInfo(const char *name, const MI_SYSCFG_MmapInfo_t **ppstMmap){
    int i;
    for(i = 0; i < _stMemoryInfo.u32MmapItemsNum; ++i){
        if(strcmp(name, _stMmapInfo[i].name) == 0){
            *ppstMmap = &_stMmapInfo[i].mmapInfo;
            return true;
        }
    }
    return false;
}
EXPORT_SYMBOL(MI_SYSCFG_GetMmapInfo);

MI_BOOL MI_SYSCFG_GetMemoryInfo(const MI_SYSCFG_MemoryInfo_t **ppstMem){
    if(_stMemoryInfo.u32MmapItemsNum == 0){
        return false;
    }
    *ppstMem = &_stMemoryInfo;
    return true;
}
EXPORT_SYMBOL(MI_SYSCFG_GetMemoryInfo);

void MI_SYSCFG_SetupMmapLoader(void){
    char *argv[] = {"/config/load_mmap", "/config/mmap.ini", NULL};
    static char *envp[] = {
                            "HOME=/",
                            "TERM=linux",
                            "PATH=/sbin:/usr/sbin:/bin:/usr/bin",
                            "LD_LIBRARY_PATH=/lib",
                            NULL
                          };

    MI_COMMON_AddDebugRawFile("mmap_info",/* 1024, */MI_SYSCFG_SetMmap, MI_SYSCFG_ShowMmap);
    MI_COMMON_AddDebugFile("memory_info", MI_SYSCFG_SetMemory, MI_SYSCFG_ShowMemory);
    call_usermodehelper("/config/config_tool", argv, envp, UMH_WAIT_PROC);
}
