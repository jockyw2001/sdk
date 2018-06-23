#ifndef DRV_MLOAD_H_
#define DRV_MLOAD_H_

#include<mload_common.h>
#include<mdrv_mload.h>
#include <mhal_common.h>
#include<mhal_cmdq.h>

#if 0
#undef false
#define false 0
#undef true
#define true  1
#define SUCCESS 0
#define FAIL -1
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

#define ML_DOUBLE_BUFFER 0

#undef offsetof
#define offsetof(S,F)           (u32) (((u8*)&(((S *) 0)->F))-((u8*)0))
#undef member_size
#define member_size(type, member) sizeof(((type *)0)->member)

typedef struct
{
  short mload_rdata[3];
} ISP_MLOAD_OUTPUT;

typedef struct {
    //DMA_MEM_INFO base_dmem;
	u32         length;
    void*       pVirAddr;
    u32         u32physAddr;
    u32         u32RingBufWp;
    u32         u32RingBufRp;
    u8          uBufferIdx;//current buffer
    u32         u4Base[2];//dram double buffer
    u32         bDirty[SCL_MLOAD_ID_NUM];
    //u32         tblSize[eMLOAD_ID_NUM]; // 16bytes alignment size
    u32         tblOffset[SCL_MLOAD_ID_NUM];// memory offset in dma memory address
    u32         tblRealSize[SCL_MLOAD_ID_NUM];// real iq table size
    u32         tblSramWidth[SCL_MLOAD_ID_NUM];// Logical size in ip side
    IspMloadDomain_e  eTarget[SCL_MLOAD_ID_NUM];
} MLOAD_IQ_MEM;

int DrvIsp_MLoadInit(MLOAD_HANDLE *handle,MLOAD_ATTR attr);
int DrvIsp_MLoadTableSet(MLOAD_HANDLE handle, IspMloadID_e idx, const u16 *table);
const u16* DrvIsp_MLoadTableGet(MLOAD_HANDLE handle, IspMloadID_e idx);
MloadCmdqIf_t* DrvIsp_MLoadChangeCmdQ(MLOAD_HANDLE *handle,MHAL_CMDQ_CmdqInterface_t *pNewCmdQ);
int DrvIsp_MLoadApply(MLOAD_HANDLE handle,IspMloadDomain_e domain);
int DrvScl_MLoadApply(MLOAD_HANDLE handle);
int DrvIsp_MLoadFreeBuffer(MLOAD_HANDLE handle);
void DrvIsp_MLoadDump(MLOAD_HANDLE handle,IspMloadID_e ID, short *buf);
void DrvScl_MLoadDump(MLOAD_HANDLE handle,IspMloadID_e ID, short *buf);
void DrvIsp_MloadUpdateWp(MLOAD_HANDLE handle);
void DrvIsp_MloadUpdateRp(MLOAD_HANDLE handle);
int DrvScl_MLoadRead(int nDataWidth,int id,int sram_offset,ISP_MLOAD_OUTPUT *output);
int DrvIsp_MLoadRead(int nDataWidth,int id,int sram_offset,ISP_MLOAD_OUTPUT *output);
#endif //DRV_MLOAD_H_
