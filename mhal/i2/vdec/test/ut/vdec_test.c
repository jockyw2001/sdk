#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>
#include <MsTypes.h>
#include <mdrv_vdec_io.h>
#include <mdrv_vdec_io_st.h>
#include <apiVDEC.h>
#include "cam_os_wrapper_s.h"
#include "vdec_test.h"

//#define _DEBUG_DUMMY_CACHE
#define _ENABLE_MD5_CALC
#define _ENABLE_RELEASE_FRAME    // this should be defined in normal case
#define _ENABLE_MD5_ERR_DUMP     // dump decoded data on MD5 failure
//#define _ENABLE_GPIO_CONTROL

//#define FRAME_RATE_CONTROL
//#define FRAME_FPS        	30

#include "libmd5/libmd5.h"
#include "libmd5/MD5.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>


#ifdef CHIP_I2_FPGA
#define MAX_FRAME_NUM           10
#else
#define MAX_FRAME_NUM           100
#endif

/*#if defined(CHIP_I2_FPGA) || (_MD5_GOLDEN_GEN == 1)
#define DUMP_DECODE_FRAME_NUM 0
#else
#define DUMP_DECODE_FRAME_NUM 2  //5
#endif
*/
#define MAX_DUMP_FRAME_NUM    10 //5


//=============================================================================
//
//=============================================================================
#define CamDevOpen(x)   open(x, O_RDWR)
#define CamDevIoctl     ioctl
#define CamDevPoll      poll
#define CamDevClose     close
#define CamOsDebug      printf
#define CamOsNoDebug

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif


#if 1
#define ASCII_COLOR_RED     "\033[1;31m"
#define ASCII_COLOR_WHITE   "\033[1;37m"
#define ASCII_COLOR_YELLOW  "\033[1;33m"
#define ASCII_COLOR_BLUE    "\033[1;36m"
#define ASCII_COLOR_GREEN   "\033[1;32m"
#define ASCII_COLOR_END     "\033[0m"

#define FUNC_DBG(fmt, args...) \
  ({if(_gUtDbgLevel>=E_VDEC_EX_DBG_LEVEL_DBG){do{printf(ASCII_COLOR_WHITE"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);}})
#define FUNC_MSG(fmt, args...)  \
  ({if(_gUtDbgLevel>=E_VDEC_EX_DBG_LEVEL_INFO){do{printf(ASCII_COLOR_GREEN"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);}})
#define FUNC_ERR(fmt, args...)  \
  ({if(_gUtDbgLevel>=E_VDEC_EX_DBG_LEVEL_ERR){do{printf(ASCII_COLOR_RED"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);}})
  //({if(_gDbgLevel>=E_VDEC_EX_DBG_LEVEL_ERR){do{printf(ASCII_COLOR_WHITE"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);}})

#define FUNC_INFO_WHITE(fmt, args...)  \
  ({do{printf(ASCII_COLOR_WHITE"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#define FUNC_INFO_GREEN(fmt, args...)  \
  ({do{printf(ASCII_COLOR_GREEN"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#else
#define FUNC_DBG(fmt, args...)
#define FUNC_MSG(fmt, args...)
#define FUNC_ERR(fmt, args...)
#define FUNC_INFO_WHITE(fmt, args...)
#define FUNC_INFO_GREEN(fmt, args...)
#endif

#define MIN_PUSHNUM_H264        7
#define MIN_PUSHNUM_H265        7
#define H264_TILE_X             32
#define H264_TILE_Y             32
#define H265_TILE_X             32
#define H265_TILE_Y             16

#define H265_TILE_X_GRP         64
#define H265_TILE_Y_GRP         32


VDEC_TEST_CFG     gVdecTestCfg = { 0 };

VDEC_EX_DbgLevel  _gDbgLevel = E_VDEC_EX_DBG_LEVEL_ERR;
VDEC_EX_DbgLevel  _gUtDbgLevel = E_VDEC_EX_DBG_LEVEL_NONE;
int              _gEnableMFCodec = FALSE;
int              _gSingleFrameMode = FALSE;
int              _gPerfTestMode = FALSE;
MS_U32           _gu8NumOfVdecChannel = 1;
MS_U32           _gnAutoMode = FALSE;
int              _gnVdecSuccess = TRUE;

VDEC_EX_CodecType _geCodecType[MAX_CHANNEL_NUM] = {E_VDEC_EX_CODEC_TYPE_H264};
char             _gFileName[MAX_CHANNEL_NUM][MAX_PATH] = { 0 };
int              _gnMinPushNum[MAX_CHANNEL_NUM] = { 0 };


//=============================================================================
//   from FW (for MFCodec dump bitlen data
//=============================================================================
#define VPU2PHY_ADDR(VPUAddr, PHYAddr, FwBase)         PHYAddr = ((MS_U32)(VPUAddr) + (MS_U32)(FwBase))

#define HEVC_LEVEL_40
#define MAX_UHD_FRM_NUM          10

#define LEVEL5_MFCODEC_BITLEN_MAX_SIZE (0x88000 * MAX_UHD_FRM_NUM) // MF2.0:(4096/128)*(2176/64)*192*9 allign 512byte ~= 0x1FE000 MF2.5:557056*10 ~= 0x550000
#define LEVEL4_MFCODEC_BITLEN_MAX_SIZE 0x25DA00 // MF2.0:(1920/128)*(1088/64)*192*19 allign 512byte ~= 0xE4000 MF2.5:130560*19 ~= 0x25DA00
#define LEVEL5_MFCODEC30_BITLEN_MAX_SIZE (0x8C000 * MAX_UHD_FRM_NUM) // MF3.0: 573440*10 = 0x578000
#define LEVEL4_MFCODEC30_BITLEN_MAX_SIZE 0x281400  // MF3.0: 138240*19 = 0x281400

#define MEM_ALIGN(addr, align) (((addr)+((align)-1)) & (~((align)-1)))


//=============================================================================
//
//=============================================================================
#define DUMP_FILE_PATH "/tmp"
#define DEV_PATH   "/dev/mdrv_vdec"

#define DO_IOCTL_NCHK      CamDevIoctl

#define DO_IOCTL(nDev,ioctlCode,arg)  if (0 != CamDevIoctl(nDev, ioctlCode, arg)) \
    {  \
  CamOsNoDebug("{Fail]\n\r");  \
        continue; \
    }
#define DO_IOCTL2(nDev,ioctlCode,arg)  if (0 != CamDevIoctl(nDev, ioctlCode, arg)) \
    {  \
  CamOsNoDebug("{Fail]\n\r");  \
        return FALSE; \
    }

#define DO_IOCTL3(nDev,ioctlCode,arg,str)  if (0 != CamDevIoctl(nDev, ioctlCode, arg)) \
    {  \
  CamOsNoDebug("{Fail]%s\n\r", str);  \
        return FALSE; \
    }

static VDEC_StreamId  _gStreamId[MAX_CHANNEL_NUM] = {0};//&tGetStreamId.tParam.streamId;
static int            _gnDev = -1;

//=============================================================================
//   FLow
//=============================================================================

#ifdef CHIP_I2_FPGA
#define DEMO_VESFILE_READER_BATCH        (1024*512)
#else
#define DEMO_VESFILE_READER_BATCH        (3*1024*1024)
#endif

#define DEMO_VESFILE_FRAME_HEADER_LEN    (16)
#define MS_U32VALUE(pu8Data, index)             (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])

typedef unsigned char                          MS_U8;         // 1 byte
typedef unsigned short                         MS_U16;        // 2 bytes
typedef unsigned int                           MS_U32;        // 4 bytes
typedef unsigned long long                     MS_U64;        // 8 bytes
typedef unsigned long long                     MS_PHY;        // 8 bytes
typedef size_t                                   MS_VIRT;       // 4 bytes when 32bit toolchain, 8 bytes when 64bit toolchain.

typedef unsigned char                           MS_BOOL;

typedef enum
{
    E_MAPI_VDEC_TILE_MODE_NONE = 0,
    E_MAPI_VDEC_TILE_MODE_16x16,       // tile mode 16x16
    E_MAPI_VDEC_TILE_MODE_16x32,       // tile mode 16x32
    E_MAPI_VDEC_TILE_MODE_32x16,       // tile mode 32x16
    E_MAPI_VDEC_TILE_MODE_32x32,       // tile mode 32x32
    E_MAPI_VDEC_TILE_MODE_NUM
} EN_MAPI_VDEC_TILE_MODE;

typedef enum
{
    E_MAPI_VDEC_MFCODEC_UNSUPPORT = 0x00,
    E_MAPI_VDEC_MFCODEC_10 = 0x01,           // MFDEC version 1.0
    E_MAPI_VDEC_MFCODEC_20 = 0x05,           // MFDEC version 2.0
    E_MAPI_VDEC_MFCODEC_25 = 0x06,           // MFDEC version 2.5
    E_MAPI_VDEC_MFCODEC_30 = 0x07,           // MFDEC version 3.0
    E_MAPI_VDEC_MFCODEC_DISABLE   = 0xFF,
} EN_MAPI_VDEC_MFCODEC_VERSION;


#define DUMP_DECODE_START_FRAME 10
#define DISP_FRAME_RATE       30

static MS_BOOL   _gStopEsSndThr[MAX_CHANNEL_NUM] = {0};
static MS_BOOL   _gStopEsGetThr[MAX_CHANNEL_NUM] = {0};
static int       _gnDecSucCnt[MAX_CHANNEL_NUM] = {0};
static int       _gnDecFaiCnt[MAX_CHANNEL_NUM] = {0};
static int       _gnDecTotalCnt[MAX_CHANNEL_NUM] = {0};

MS_U32           _gu32InitCnt[MAX_CHANNEL_NUM] = {0};
MS_U32           _gu32DispCnt[MAX_CHANNEL_NUM] = {0};
MS_U32           _gu32BufSize[MAX_CHANNEL_NUM];

//int                frame_cnt = 0;
// for _DBG_Batch_Dump_Files
//MS_PHY _gFrame_luma[MAX_FRAME_NUM];
//MS_PHY _gFrame_chroma[MAX_FRAME_NUM];


//=============================================================================
/* halMpool.h */

#define MALLOC_IOC_MAGIC                'M'

// MALLOC_IOC_MPOOL_INFO
#define MALLOC_IOC_MPOOL_INFO           _IOWR(MALLOC_IOC_MAGIC, 0x00, DevMalloc_MPool_Info_t)
#define MALLOC_IOC_FLUSHDCACHE          _IOR(MALLOC_IOC_MAGIC, 0x01, DevMalloc_MPool_Info_t)
#define MALLOC_IOC_MPOOL_CACHE          _IOR(MALLOC_IOC_MAGIC, 0x03, unsigned int)
#define MALLOC_IOC_MPOOL_SET            _IOR(MALLOC_IOC_MAGIC, 0x04, DevMalloc_MPool_Info_t)
#define MALLOC_IOC_MPOOL_KERNELDETECT   _IOWR(MALLOC_IOC_MAGIC, 0x05, DevMalloc_MPool_Kernel_Info_t)
#define MALLOC_IOC_FLUSHDCACHE_PAVA     _IOR(MALLOC_IOC_MAGIC, 0x07, DevMalloc_MPool_Flush_Info_t)
#define MALLOC_IOC_FLUSHDCACHE_ALL      _IOR(MALLOC_IOC_MAGIC, 0x09, DevMalloc_MPool_Flush_Info_t)
#define MALLOC_IOC_SETWATCHPT           _IOW(MALLOC_IOC_MAGIC, 0x0A, DevMalloc_MPool_Watchpt_Info_t)
#define MALLOC_IOC_GETWATCHPT           _IOR(MALLOC_IOC_MAGIC, 0x0B, char*)


/* MsOS.h */
#define     MSOS_CACHE_BOTH         0
#define     MSOS_NON_CACHE_BOTH     1
#define     MSOS_CACHE_USERSPACE    2
#define     MSOS_NON_CACHE_USERSPACE  3
#define     MSOS_CACHE_KERNEL       4
#define     MSOS_NON_CACHE_KERNEL   5

/* halCHIP.h */
typedef enum
{
    E_CHIP_MIU_0 = 0,
    E_CHIP_MIU_1,
    E_CHIP_MIU_2,
    E_CHIP_MIU_3,
    E_CHIP_MIU_NUM,
} CHIP_MIU_ID;
//* MIU ADDR */
#define HAL_MIU0_BASE               0x00000000UL
#define HAL_MIU1_BASE               0x80000000UL // 1512MB
#define HAL_MIU2_BASE               0xC0000000UL //


#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < HAL_MIU1_BASE) \
                                                        {MiuSel = E_CHIP_MIU_0; Offset = PhysAddr;} \
                                                     else \
                                                         {MiuSel = E_CHIP_MIU_1; Offset = PhysAddr - HAL_MIU1_BASE;}

#define _miu_offset_to_phy(MiuSel, Offset, PhysAddr) if (MiuSel == E_CHIP_MIU_0) \
                                                        {PhysAddr = Offset;} \
                                                     else \
                                                         {PhysAddr = Offset + HAL_MIU1_BASE;}

 /* */
#define MAX_MAPPINGSIZE 200


#define MMAP_NONCACHE   TRUE
#define MMAP_CACHE      FALSE

#ifdef CHIP_K6
#define VDEC_MIU               1
#define FW_BUFFER_OFFSET       0x00000000
#define  VDEC_BUFFER_SIZE      0x07E00000
#define  FW_BUFFER_BASESIZE    0x00100000
#define  FW_BUFFER_CHANSIZE    0x00100000
#define  BS_BUFFER_SIZE        0x00100000   //bitstream buffer size
#else
#define VDEC_MIU                0
#define FW_BUFFER_OFFSET       0x00000000
#define FW_BUFFER_BASESIZE     0x00100000

#define ENABLE_LOW_CPU_BUF
#ifdef ENABLE_LOW_CPU_BUF
#define FW_BUFFER_CHANSIZE     0x00050000
#else
#define FW_BUFFER_CHANSIZE     0x00100000
#endif

#define  BS_BUFFER_SIZE        0x00100000   //bitstream buffer size

#ifdef CHIP_I2_FPGA
#define VDEC_BUFFER_SIZE       0x00C00000   //0x01C00000
#else
#define VDEC_BUFFER_SIZE       0x04000000   //0x02800000  //0x0C000000
#endif
#endif

typedef struct
{
  MS_U32 u32PhyAddr;
  MS_U32 u32Offset;
  MS_U32 u32VirtAddr;
  MS_U32 u32KVirtAddr;
  MS_U32 u32Size;
} DMEM_INFO;

static MS_U32   fw_buffer_phyAddr = 0;
static MS_U32   fw_buffer_offset = 0;
static MS_U32   fw_buffer_virtAddr = 0;
static MS_U32   fw_buffer_kVirtAddr = 0;
static MS_U32   fw_buffer_size = 0;

static MS_U32   vdec_buffer_phyAddr = 0;
static MS_U32   vdec_buffer_offset = 0;
static MS_U32   vdec_buffer_virtAddr = 0;
static MS_U32   vdec_buffer_kVirtAddr = 0;
static MS_U32   vdec_buffer_size = 0;

static DMEM_INFO xcshmBuf = { 0 };



#ifdef CHIP_K6
#define MIU_INTERVAL            0x80000000UL
#define ENABLE_PARTIAL_MAPPING  1
#define DISABLE_PARTIAL_MAPPING 0

 /* defined in halMPool.h */
typedef struct
{
    MS_U64             u64Addr;
    MS_U64             u64Size;
    MS_U64             u64Interval;
    MS_U8              u8MiuSel;
} DevMalloc_MPool_Info_t;


typedef struct
{
    MS_VIRT u64VirtStart;
    MS_VIRT u64VirtEnd;
    MS_U64 u64Phyaddr;
    MS_U64 u64MpoolSize;
    MS_BOOL bIsUsed;
    MS_SIZE s32V2Poff;
    MS_BOOL bNonCache;
    MS_U8 u8MiuSel;
    MS_BOOL bIsDyn;
} MPOOL_INFO;

static MPOOL_INFO mpool_info[MAX_MAPPINGSIZE] = { 0 };
static MS_S32              _s32MPoolFd = -1;

static MS_U8 _MPOOL_DELAY_BINDING(MS_S32 idx)
{
    DevMalloc_MPool_Info_t  stPoolInfo;
    MS_U64                  u64AddrVirt;
    MS_U32                  bCache = (mpool_info[idx].bNonCache) ? 0: 1;
    MS_U8   u8Check_MiuSel = 0;
    MS_U64  u64Check_Offset = 0;

    stPoolInfo.u64Addr = mpool_info[idx].u64Phyaddr;
    stPoolInfo.u64Size = mpool_info[idx].u64MpoolSize;
    stPoolInfo.u64Interval = MIU_INTERVAL;
    stPoolInfo.u8MiuSel = mpool_info[idx].u8MiuSel;

        /* use u64Phyaddr to get MIU, offset */
        _phy_to_miu_offset(u8Check_MiuSel, u64Check_Offset, stPoolInfo.u64Addr);

        if(u8Check_MiuSel != stPoolInfo.u8MiuSel)
        {
                printf("\033[35mFunction = %s, Line = %d, [Error] miu_setting is wrong\033[m\n", __PRETTY_FUNCTION__, __LINE__);
                return FALSE;
        }
        stPoolInfo.u64Addr = u64Check_Offset;

    //prevent race condition cause mpool mapping size modified in the kernel layer
    if (ioctl(_s32MPoolFd, MALLOC_IOC_MPOOL_SET, &stPoolInfo))
    {
        return FALSE;
    }
    if (ioctl(_s32MPoolFd, MALLOC_IOC_MPOOL_CACHE, &bCache))
    {
        return FALSE;
    }
    if ((MS_VIRT)MAP_FAILED == (u64AddrVirt = (MS_VIRT)mmap(0, mpool_info[idx].u64MpoolSize, PROT_READ | PROT_WRITE, MAP_SHARED, _s32MPoolFd, 0)))
    {
        return FALSE;
    }


    mpool_info[idx].u64VirtStart = u64AddrVirt;
    mpool_info[idx].u64VirtEnd = (u64AddrVirt + mpool_info[idx].u64MpoolSize);

        mpool_info[idx].s32V2Poff = mpool_info[idx].u64VirtStart - mpool_info[idx].u64Phyaddr;

    return TRUE;
}

static MS_BOOL _MPool_Mapping_Dynamic(MS_U8 u8MiuSel, MS_SIZE u32Offset, MS_SIZE u32MapSize, MS_U8 u8MapMode)
{
    MS_S32 i,idx=0;
    MS_BOOL     bNonCache =0;

    if(u8MapMode == MSOS_CACHE_BOTH || u8MapMode == MSOS_CACHE_USERSPACE || u8MapMode == MSOS_CACHE_KERNEL )
    {
        bNonCache = 0;
    }
    else
    {
        bNonCache =1;
    }

     if(u8MapMode == MSOS_CACHE_BOTH || u8MapMode == MSOS_NON_CACHE_BOTH || u8MapMode == MSOS_CACHE_USERSPACE || u8MapMode == MSOS_NON_CACHE_USERSPACE)
     {

        for (i = 0; i < MAX_MAPPINGSIZE; i++)
        {
            if(mpool_info[i].bIsUsed == false)
            {
                idx = i;
                break;
            }
        }
        if(i >= MAX_MAPPINGSIZE)
        {
            FUNC_ERR("Not enough MPool, must increase MAX_MAPPINGSIZE!!\n");
            return FALSE;
        }

        mpool_info[idx].bIsUsed = true;
        if(bNonCache)
        {
            mpool_info[idx].bNonCache = MMAP_NONCACHE;
        }
        else
        {
            mpool_info[idx].bNonCache = MMAP_CACHE;
        }
        mpool_info[idx].u64VirtStart = 0;
        mpool_info[idx].u64VirtEnd = 0;
        mpool_info[idx].u8MiuSel = u8MiuSel;
        mpool_info[idx].bIsDyn = true;

        _miu_offset_to_phy(u8MiuSel, u32Offset, mpool_info[idx].u64Phyaddr);

        mpool_info[idx].u64MpoolSize = u32MapSize;
        mpool_info[idx].s32V2Poff = 0;
        _MPOOL_DELAY_BINDING(idx);
    }
    return TRUE;
}

MS_VIRT  MsOS_PA2KSEG1(MS_PHY u64Phys)
{
    int i;

    for(i = 0; i < MAX_MAPPINGSIZE;i ++)
    {
        if((!mpool_info[i].bIsUsed) || (!mpool_info[i].bNonCache))
        {
            continue;   // due to unmap, we can not use break
        }

        // we do PA_REGION check here, to check if pAddrPhys is located in mpool_info[i], to prevent we do mmap for some dynamic_mapping but not used
        if(((mpool_info[i].u64Phyaddr <= u64Phys) && (u64Phys < (mpool_info[i].u64Phyaddr + mpool_info[i].u64MpoolSize))) )
        {
            return (MS_VIRT)(u64Phys + mpool_info[i].s32V2Poff);
        }
   }

   return (MS_VIRT)0;
}
int _Vdec_DoMmap(void)
{
    int ret = TRUE;

    if(!_MPool_Mapping_Dynamic(VDEC_MIU, fw_buffer_offset, fw_buffer_size, MSOS_CACHE_BOTH))
     {
          FUNC_ERR("_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(ret == TRUE && !_MPool_Mapping_Dynamic(VDEC_MIU, fw_buffer_offset, fw_buffer_size, MSOS_NON_CACHE_BOTH))
     {
       FUNC_ERR("_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(!_MPool_Mapping_Dynamic(VDEC_MIU, vdec_buffer_offset, vdec_buffer_size, MSOS_CACHE_BOTH))
      {
       FUNC_ERR("_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }

      if(ret == TRUE && !_MPool_Mapping_Dynamic(VDEC_MIU, vdec_buffer_offset, vdec_buffer_size, MSOS_NON_CACHE_BOTH))
      {
        FUNC_ERR("_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }
      return ret;
}
#else
MS_VIRT  MsOS_PA2KSEG1(MS_PHY u64Phys)
{
    int i;

    if(((MS_PHY)fw_buffer_phyAddr <= u64Phys) && (u64Phys < ((MS_PHY)fw_buffer_offset + fw_buffer_size)) )
    {
        FUNC_DBG("MsOS_PA2KSEG 1: %x-->%x\n", (unsigned int)u64Phys, (unsigned int)(u64Phys + (fw_buffer_virtAddr - fw_buffer_offset)));
        return (MS_VIRT)(u64Phys + (fw_buffer_virtAddr - fw_buffer_offset));
    }
    else if(((MS_PHY)vdec_buffer_phyAddr <= u64Phys) && (u64Phys < ((MS_PHY)vdec_buffer_phyAddr + vdec_buffer_size)) )
    {
      FUNC_DBG("MsOS_PA2KSEG 2: %x-->%x\n", (unsigned int)u64Phys, (unsigned int)(u64Phys + (vdec_buffer_virtAddr - vdec_buffer_phyAddr)));
        return (MS_VIRT)(u64Phys + (vdec_buffer_virtAddr - vdec_buffer_phyAddr));
    }
    else
    {
        FUNC_ERR("MsOS_PA2KSEG1 fail!(%llx)\n", u64Phys);
        return (MS_VIRT)-1;
    }
}
#endif


/* driver layer mapinfo */
int _Vdec_DrvDoMmap(int nDev)
{
    int ret = TRUE;
    IOCTL_PARAM_VDEC_MAPINFO_t drvMapInfo;

    drvMapInfo.u8MiuSel = VDEC_MIU;
    drvMapInfo.u32Offset = fw_buffer_offset;
    drvMapInfo.u32MapSize = fw_buffer_size;
    drvMapInfo.u32KvirtAddr = fw_buffer_kVirtAddr;
    DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_DOMMAP, &drvMapInfo, "VDEC_EX_DOMMAP 1");
    FUNC_MSG("_Vdec_DrvDoMmap: cpu(%x,%x)\n", drvMapInfo.u32Offset , drvMapInfo.u32MapSize );

    drvMapInfo.u8MiuSel = VDEC_MIU;
    drvMapInfo.u32Offset = vdec_buffer_offset;
    drvMapInfo.u32MapSize = vdec_buffer_size;
    drvMapInfo.u32KvirtAddr = vdec_buffer_kVirtAddr;
    DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_DOMMAP, &drvMapInfo, "VDEC_EX_DOMMAP 2");
    FUNC_MSG("_Vdec_DrvDoMmap: buf(%x,%x)\n", drvMapInfo.u32Offset , drvMapInfo.u32MapSize );

    return ret;
}


int _Vdec_DrvDoUnMap(int nDev)
{
    int ret = TRUE;
    IOCTL_PARAM_VDEC_UNMAPINFO_t drvUnMapInfo;

    drvUnMapInfo.tInfo.u32phyStart = fw_buffer_phyAddr;
    drvUnMapInfo.tInfo.u32MapSize   = fw_buffer_size;
    drvUnMapInfo.tInfo.u32KvirtAddr = fw_buffer_kVirtAddr;
    DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_DOUNMAP, &drvUnMapInfo, "DoUnMap 1");
    FUNC_DBG("_Vdec_DrvDoMmap: cpu(%x,%x)\n", drvUnMapInfo.tInfo.u32phyStart , drvUnMapInfo.tInfo.u32MapSize );

    drvUnMapInfo.tInfo.u32phyStart = vdec_buffer_phyAddr;
    drvUnMapInfo.tInfo.u32MapSize   = vdec_buffer_size;
    drvUnMapInfo.tInfo.u32KvirtAddr = vdec_buffer_kVirtAddr;
    DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_DOUNMAP, &drvUnMapInfo, "DoUnMap 2");
    FUNC_DBG("_Vdec_DrvDoMmap: buf(%x,%x)\n", drvUnMapInfo.tInfo.u32phyStart , drvUnMapInfo.tInfo.u32MapSize );

    return ret;
}

//=============================================================================
//   MFCodec Functions
//=============================================================================
MS_U32 MFCodec_blens_size(MS_U32 width, MS_U32 height)
{
    // blens size = (# of 64x64 areas to cover frame with shift up lines)
    //            * (8 mfc tiles per 64x64 area)
    //            * (32 bytes per mfc tile)
    return  MEM_ALIGN(((width + 63) / 64) * (((height + 8) + 63) / 64) * 8 * 32, 512);
}
//=============================================================================
//   MD5 Functions
//=============================================================================
#define MD5_CHK_ALLFRAMES       0xFFFF
int     _gMd5_golden_num[MAX_CHANNEL_NUM] = { 0 };


static void MD5_GetFilename(char *es_fname, char *md5_fname)
{
    char *pch;
    int idx;

    strcpy(md5_fname, es_fname);
    pch = strrchr(md5_fname, '.');
    idx = pch - md5_fname;
    md5_fname[idx] = 0;
    if (_gEnableMFCodec)
    {
        strcat(md5_fname, "_mfc.md5");
    }
    else
    {
        strcat(md5_fname, ".md5");
    }
}

#define MD5_BLOCK_LEN   16
unsigned char md5_golden[MAX_CHANNEL_NUM][MAX_FRAME_NUM][3][MD5_BLOCK_LEN];
unsigned char md5_result[MAX_CHANNEL_NUM][MAX_FRAME_NUM][3][MD5_BLOCK_LEN];

static void dump_md5(MS_U32 setNumber, unsigned char md5[3][MD5_BLOCK_LEN])
{
     int i,j;
     printf("[SWMD5] ");
     for(j = 0; j < setNumber;j++)
     {
          for(i = 0; i < MD5_BLOCK_LEN;i++)
          {
              printf("%02x",md5[j][i]);
          }
          //printf(" ");
      }
      printf("\n");
}

static int MD5_LoadGolden(int channel, char *fname)
{
    FILE *pFile = NULL;
    char line[256], str[8];
    int  i = 0, j, k;
    //int cnt = (gVdecTestCfg.u32MaxFrameNum > MAX_FRAME_NUM) ? MAX_FRAME_NUM : gVdecTestCfg.u32MaxFrameNum;
    int cnt = MAX_FRAME_NUM;

    if ((gVdecTestCfg.u32MaxFrameNum > 0) && (gVdecTestCfg.u32MaxFrameNum < MAX_FRAME_NUM))
    {
      cnt = gVdecTestCfg.u32MaxFrameNum;
    }

    FUNC_MSG("load MD5 from %s\n", fname);
    pFile = fopen(fname, "r");
    if (pFile == NULL)
    {
        printf("open file %s failed\n", fname);
        return -1;
    }

    while (fgets(line, sizeof(line), pFile) && (i < cnt))  //gVdecTestCfg.u32MaxFrameNum))
    {
        //memcpy(&md5_golden[channel][i], line, MD5_YUV_LEN);
        //printf("LINE=%s\n",line);
        for (j = 0; j < 3; j++)
        {
          for (k=0; k < MD5_BLOCK_LEN; k++)
          {
             sscanf(line+(j*MD5_BLOCK_LEN+k)*2, "%2hhX", &md5_golden[channel][i][j][k]);
          }
        }
        FUNC_DBG("reading(%d)MD5_golden= %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", i
               , md5_golden[channel][i][0][0], md5_golden[channel][i][0][1], md5_golden[channel][i][0][2], md5_golden[channel][i][0][3], md5_golden[channel][i][0][4], md5_golden[channel][i][0][5]
               , md5_golden[channel][i][0][6], md5_golden[channel][i][0][7], md5_golden[channel][i][0][8], md5_golden[channel][i][0][9], md5_golden[channel][i][0][10], md5_golden[channel][i][0][11]
               , md5_golden[channel][i][0][12], md5_golden[channel][i][0][13], md5_golden[channel][i][0][14], md5_golden[channel][i][0][15]);
        i++;
    }

    FUNC_DBG("load MD5: num= %d\n", i);
    _gMd5_golden_num[channel] = i;
    fclose(pFile);
    return 0;
}

static FILE * pFile_md5dump = NULL;
static int MD5_DumpInit(char *mfname)
{
    char md5_fullname[MAX_PATH];
    char *fname, *bname;

    if (pFile_md5dump != NULL) return TRUE;

    fname = strdup(mfname);
    bname = basename(fname);
    sprintf(md5_fullname, "%s/%s", DUMP_FILE_PATH, bname);
    FUNC_MSG("save MD5 to %s\n", md5_fullname);

    if ((pFile_md5dump = fopen(md5_fullname, "w")) == NULL)
	{
	    printf("open file %s failed\n", md5_fullname);
	    return FALSE;
	}
}
static int MD5_DumpExit()
{
    if (pFile_md5dump == NULL) return TRUE;
    fclose(pFile_md5dump);
    pFile_md5dump = NULL;
}
static void MD5_DumpResult(unsigned char md5[3][MD5_BLOCK_LEN])
{
    int  j, k;

    if (pFile_md5dump == NULL) return;

    for (j = 0 ; j <3; j++)
    {
      for (k = 0 ; k < MD5_BLOCK_LEN; k++)
      {
        fprintf(pFile_md5dump, "%02X", md5[j][k]);
      }
    }
    fprintf(pFile_md5dump, "\n");
}

static int MD5_VerifyResult(int channel, int frameNo, unsigned char md5[3][MD5_BLOCK_LEN])
{
    int i,j,k;
    int ret = TRUE;

    if ((gVdecTestCfg.eMd5Mode == E_VDEC_MD5_NOT_VERIFY) || _gEnableMFCodec || (_gMd5_golden_num[channel] == 0))
    {
      return TRUE;
    }

    i = frameNo % _gMd5_golden_num[channel];

    if (memcmp(md5, &md5_golden[channel][i], 3*MD5_BLOCK_LEN) != 0)
    {
            FUNC_ERR("MD5_Verify Failed[%d]: %d\n", channel, frameNo);
            printf("MD5_golden= ");
            for(j = 0; j < 3;j++)
            {
                  for(k = 0; k < MD5_BLOCK_LEN; k++)
                  {
                      printf("%02x",md5_golden[channel][i][j][k]);
                  }
                  //printf(" ");
            }
            printf("\n");
            printf("MD5_result= ");
            for(j = 0; j < 3;j++)
            {
                  for(k = 0; k < MD5_BLOCK_LEN; k++)
                  {
                      printf("%02x",md5[j][k]);
                  }
                  //printf(" ");
           }
           printf("\n");
           return FALSE;
    }

    //FUNC_INFO_GREEN("MD5_VerifyResult[%d](%d,%d) (%d, %d) %s\n", channel, start, end, _gMd5_golden_num[channel], _gu32DispCnt[channel],
    FUNC_INFO_GREEN("MD5_VerifyResult[%d](%d) %s\n", channel, _gu32DispCnt[channel],
                                                       (ret)? "success" : "fail");
    return ret;
}


//MD5_Calc(pstNextDispFrm->stFrmInfo.u32LumaAddr,  u32LumaSz,     md5_result[u16ChnNum][_gu32DispCnt[u16ChnNum]]);
//MD5_Calc(pstNextDispFrm->stFrmInfo.u32ChromaAddr, u32ChromaSz, &md5_result[u16ChnNum][_gu32DispCnt[u16ChnNum]][0]+16);
/*int hvd_MIUI_YUV_to_Golden_YUV_md5(MS_U8 *pSrcY, MS_U8 *pSrcC, MS_U8 *pSrcY1, MS_U8 *pSrcC1,
                                   int dst_width, int dst_height,
                                   int src_width, int src1_width,
                                   int depth_y_minus8, int depth_c_minus8, unsigned char md5[3][16],
                                   void  *pDstBuf,  //yuv_buf_planar_st
                                   int tile_width, int tile_height,
                                   int tile_2bit_width)
*/
static void MD5_Calc(MS_PHY phyaddr_luma, MS_PHY phyaddr_chroma, int width, int height,
                       int pitch_w, int tile_w, int tile_h, unsigned char md5[3][16])
{
    MS_U8 *pyBuf  = (MS_U8 *)MsOS_PA2KSEG1(phyaddr_luma);
    MS_U8 *puvBuf = (MS_U8 *)MsOS_PA2KSEG1(phyaddr_chroma);

    hvd_MIUI_YUV_to_Golden_YUV_md5(pyBuf, puvBuf, NULL, NULL,  /*pSrcY, pSrcC, pSrcY1, pSrcC1*/
                                 width, height,
                                 pitch_w, 0,
                                 0, 0, md5,
                                 NULL,
                                 tile_w, tile_h,
                                 0);
}


//=============================================================================
// Help Functions from MI
//=============================================================================
static int PushVideoES(int nDev, MS_U8 *pu8Buf, MS_U32 u32BufLen, MS_U64 u64Pts, MS_U16 u16ChnNum)
{
    MS_U32 u32AvailSize;
    MS_PHY phyAddr;
    MS_VIRT virtAddr;
    VDEC_EX_DecCmd tCmd;
    MS_U32 u32QVac = 0;
    IOCTL_PARAM_VDEC_SET_CONTROL_t     tSetCtrl;
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl;

    memcpy((void*)&tGetCtrl.tStreamId, &_gStreamId[u16ChnNum], sizeof(VDEC_StreamId));
    memcpy((void*)&tSetCtrl.tStreamId, &_gStreamId[u16ChnNum], sizeof(VDEC_StreamId));

    //u32QVac = MApi_VDEC_EX_GetDecQVacancy(pStreamId);
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_DECQ_VACANCY;
    DO_IOCTL2(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);
    u32QVac = tGetCtrl.tParam.decQVacancy.u32Vacancy;
    if(0 > u32QVac)
    {
        FUNC_ERR("Fail: ch=%d\n", u16ChnNum);
        return FALSE;
    }
    FUNC_DBG("Done: MApi_VDEC_EX_GetDecQVacancy[%d]=%d\n", u16ChnNum, u32QVac);

    //MApi_VDEC_EX_GetESBuff(pStreamId, u32BufLen/* u32ReqSize */, &u32AvailSize, &phyAddr),);
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_ES_BUFF;
    tGetCtrl.tParam.esBuffer.u32ReqSize = u32BufLen;
    DO_IOCTL2(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);
    u32AvailSize = tGetCtrl.tParam.esBuffer.u32AvailSize;
    phyAddr = tGetCtrl.tParam.esBuffer.u32Addr;
    if(u32AvailSize < u32BufLen)
    {
        FUNC_ERR("MApi_VDEC_EX_GetESBuff: size not enough\n");
        sleep(2);
        return FALSE;
    }

    virtAddr = MsOS_PA2KSEG1(phyAddr);//MsOS_PA2KSEG1(phyAddr); //MsOS_PA2KSEG1(nDev, phyAddr);
    memcpy((void*)virtAddr, pu8Buf, u32BufLen);
    FUNC_MSG("Done: MApi_VDEC_EX_GetESBuff[%d]=(%d,%d, addr=0x%llx->%lx)%llu\n", u16ChnNum, u32BufLen, u32AvailSize, phyAddr, virtAddr, u64Pts);

    //VdecResultHandler(MApi_VDEC_EX_PushDecQ(pStreamId,&tCmd),"PushDecQ");
    tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_PUSH_DECQ;
    tSetCtrl.tParam.pushDecQ.cmd.u32Size = u32BufLen;
    tSetCtrl.tParam.pushDecQ.cmd.u32StAddr = phyAddr;
    tSetCtrl.tParam.pushDecQ.cmd.u32ID_H = (MS_U32)((u64Pts >> 32) & 0xFFFFFFFF); //should be increased per frame
    tSetCtrl.tParam.pushDecQ.cmd.u32ID_L = (MS_U32)(u64Pts & 0xFFFFFFFF);
    tSetCtrl.tParam.pushDecQ.cmd.u32Timestamp = (MS_U32)(u64Pts & 0xFFFFFFFF);
    DO_IOCTL2(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);

    //FUNC_MSG("Done: (%p,%d)%llu\n",virtAddr,u32BufLen, u64Pts);

    if (_geCodecType[u16ChnNum] != E_VDEC_EX_CODEC_TYPE_MJPEG)
    {
        //VdecResultHandler(MApi_VDEC_EX_FireDecCmd(pStreamId),"FireDecCmd");
        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_FIRE_DEC;
        DO_IOCTL2(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
    }

    return TRUE;
}

MS_U64 CalcNextVideoPts(MS_U32 u32FrameRate)
{
    if(0 == u32FrameRate)
    {
        return (MS_U64)(-1);
    }

    return (MS_U64)(90 * 1000 / u32FrameRate);
}

static int SendEsStream(void *args)
{
    MS_U16 u16ChnNum = (MS_U32)args;
    MS_S32 s32ReadLen;
    MS_U8 *pu8Buf= NULL;
    MS_U32 u32PktSize;
    MS_U64 u64PTS = 0;
    MS_U8 sFrameHeader[DEMO_VESFILE_FRAME_HEADER_LEN] = {0};
    int ret = FALSE;
    int cnt = 0;
    int  fd;
    char sPthreadName[16];
    #ifdef FRAME_RATE_CONTROL
    struct timespec tCurrentTime;
    long long nNextFrameUs=0, nCurrentUs;
    MS_U32    u32SleepUs = 1000000/FRAME_FPS;
    MS_U32    u32CompenUs = 1000000 - u32SleepUs * (FRAME_FPS - 1);
    #endif

    snprintf(sPthreadName, sizeof(sPthreadName), "SendStream%02d", u16ChnNum);
    pthread_setname_np(pthread_self(), sPthreadName);

    fd = open((char *)_gFileName[u16ChnNum], O_RDONLY, 0);
    if (0 > fd)
    {
          FUNC_ERR("can't open file:%s error\n", _gFileName[u16ChnNum]);
          _gStopEsGetThr[u16ChnNum] = TRUE;
          return -1;
    }
    else
    {
        printf("CH%d Open File : %s\n", u16ChnNum, _gFileName[u16ChnNum]);
    }

    pu8Buf = malloc(DEMO_VESFILE_READER_BATCH);
    if(pu8Buf == NULL)
    {
        FUNC_ERR("Error: failed to malloc(%d) for reading VES file!\n", DEMO_VESFILE_READER_BATCH);
        _gStopEsGetThr[u16ChnNum] = TRUE;
        goto VES_READER_EXIT;
    }

    _gu32InitCnt[u16ChnNum] = 0;
    while(FALSE == _gStopEsSndThr[u16ChnNum])
    {
        //memset(sFrameHeader, 0, DEMO_VESFILE_FRAME_HEADER_LEN);
        //long pos = lseek(fd, 0L, SEEK_CUR);
        s32ReadLen = read(fd, sFrameHeader, DEMO_VESFILE_FRAME_HEADER_LEN);
        if(s32ReadLen <= 0)
        {
            //if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_GEN) /* only dump CH0 */
            //{
            //printf("%s(%d) read end of file!\n", __FUNCTION__, __LINE__);
            //_gStopEsSndThr[u16ChnNum] = TRUE;
            //break;
            //}
            lseek(fd, 0, SEEK_SET);
            continue;
        }

        u32PktSize = MS_U32VALUE(sFrameHeader, 4);
        if(u32PktSize > DEMO_VESFILE_READER_BATCH)
        {
            FUNC_ERR("VES frame size(%d KB) more than buffer size(%d KB), please check VES format!!!\n", u32PktSize / 1024, DEMO_VESFILE_READER_BATCH / 1024);
            lseek(fd, 0, SEEK_SET);
            break;
        }
        FUNC_DBG("Done: size=%x (%x)!\n", u32PktSize, DEMO_VESFILE_READER_BATCH);

        //memset(pu8Buf, 0, DEMO_VESFILE_READER_BATCH);
        s32ReadLen = read(fd, pu8Buf, u32PktSize);
        if(s32ReadLen <= 0)
        {
            FUNC_MSG("read end of file!\n");
            lseek(fd, 0, SEEK_SET);
            ret = TRUE;
            //break;
        }
PUSH_RETRY:
        if(!PushVideoES(_gnDev, pu8Buf, (MS_U32)s32ReadLen, u64PTS, u16ChnNum))
        {
            FUNC_DBG("PushVideoES[%d] failed!\n", u16ChnNum);
            //break;  //fail
            //lseek(fd, pos, SEEK_SET);
            if (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG)
                usleep(1000);
            else
                usleep(5000);
            if (TRUE == _gStopEsSndThr[u16ChnNum])
                break;
            goto PUSH_RETRY;
        }

        u64PTS = _gu32InitCnt[u16ChnNum]; //u64PTS + CalcNextVideoPts(DISP_FRAME_RATE);
        _gu32InitCnt[u16ChnNum]++;

#if 1 //sylvia: test mjpeg
        if (_gSingleFrameMode)
#else
        if (_gSingleFrameMode || (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG))
#endif
        {
            if (_gu32InitCnt[u16ChnNum] >= _gnMinPushNum[u16ChnNum])
            {
                FUNC_MSG("[Done]Ch%d %d %d!\n",u16ChnNum, _gu32InitCnt[u16ChnNum], _gnMinPushNum[u16ChnNum]);
                break;
            }
        }

        if (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_H264)
        {
             if ((pu8Buf[4] & 0xF) != 0x5 && (pu8Buf[4] & 0xF) != 0x1)
                 continue;
        }
        else if (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC)
        {
            if ((pu8Buf[4] == 0x40) || (pu8Buf[4] == 0x42) || (pu8Buf[4] == 0x44) || (pu8Buf[4] == 0x4e))
                continue;
        }

        #ifndef FRAME_RATE_CONTROL
        if (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG)
            usleep(1000);
        else
            usleep(5000);
        #else
        cnt++;
        if (nNextFrameUs == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &tCurrentTime);
            nNextFrameUs = (tCurrentTime.tv_sec * 1000000) + (tCurrentTime.tv_nsec / 1000);
        }

        if (cnt >= FRAME_FPS)
        {
            cnt = 0;
            nNextFrameUs += u32CompenUs;
        }
        else
        {
            nNextFrameUs += u32SleepUs;
        }
        clock_gettime(CLOCK_MONOTONIC, &tCurrentTime);
        nCurrentUs = (tCurrentTime.tv_sec * 1000000) + (tCurrentTime.tv_nsec / 1000);
        if (nNextFrameUs > nCurrentUs)
        {
            usleep(nNextFrameUs-nCurrentUs);
        }
        #endif
    }
    //_gu32InitCnt[u16ChnNum] = 0;
VES_READER_EXIT:

    if(pu8Buf)
    {
        free(pu8Buf);
        pu8Buf = NULL;
    }
    close(fd);

    FUNC_MSG("[Finish] CH%d\n", u16ChnNum);
    return ret;
}


#ifdef _DEBUG_DUMMY_CACHE
#define CACHE_SIZE      32768
static char buf1[CACHE_SIZE], buf2[CACHE_SIZE];

static void dummy_use_cache(void)
{
    memcpy(buf2, buf1, CACHE_SIZE);
    memcpy(buf1, buf2, CACHE_SIZE);
    printf("dummy_use_cache...\n");
}
#endif

MS_BOOL _DUMP_BufferToFile(MS_PHY phyaddr, unsigned int length, char * name)
{
    MS_PHY *pVaBuf = (MS_PHY *)MsOS_PA2KSEG1(phyaddr);
    FILE *pFile = NULL;

#ifdef _DEBUG_DUMMY_CACHE
    dummy_use_cache();
#endif

#ifndef _CUNIT_TEST
    pFile = fopen(name, "wb");
    if(pFile == NULL)
    {
        printf("open file %s failed\n", name);
        return FALSE;
    }

    //get file size
    if(fwrite(pVaBuf, 1, length, pFile) != length)
    {
        printf("fread %s failed\n", name);
        fflush(pFile);
        fclose(pFile);
        pFile = NULL;
        return FALSE;
    }
    fflush(pFile);
    fclose(pFile);
    pFile = NULL;
#endif

    printf("dump file(%s) ok ..............[len:%d]\n", name, length);

    return true;
}


static int GetNextDispFrame(int nDev, VDEC_StreamId *pStreamId, MS_U16 u16ChnNum)
{
    VDEC_EX_Result enRst = E_VDEC_EX_FAIL;
    IOCTL_PARAM_VDEC_SET_CONTROL_t     tSetCtrl;
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl;
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl2;
    VDEC_EX_DispFrame *pstNextDispFrm = &tGetCtrl.tParam.nextDispFrame.dispFrm;
    MS_BOOL b10Bit;
    EN_MAPI_VDEC_TILE_MODE eTileMode;
    static MS_U32 u32DumpVdecFrameCount = 0;
    MS_U32 yPitch, yHeight, uvPitch, uvHeight;
    MS_U32 u32Pitch, u32LumaSz, u32ChromaSz;
    char name[256], typeStr[8] = "h264";
    MS_U32 tile_x, tile_y;
    VDEC_EX_FrameInfoExt_v6 *pstFrmInfoExt_v6 = NULL;
    MS_U32  u32bitlenSize = 0;
    MS_VIRT *pVaBuf;
    int i;
    unsigned char md5[3][MD5_BLOCK_LEN];

    //enRst = MApi_VDEC_EX_GetNextDispFrame(pStreamId, &pstNextDispFrm);
    memcpy((void*)&tGetCtrl.tStreamId, pStreamId, sizeof(VDEC_StreamId));
    memcpy((void*)&tSetCtrl.tStreamId, pStreamId, sizeof(VDEC_StreamId));

    //u32QVac = MApi_VDEC_EX_GetDecQVacancy(pStreamId);
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME;
    //DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl, "GET_NEXT_DISP_FRAME;");
    if (0 != CamDevIoctl(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL,  (void*)&tGetCtrl))
    {
        return FALSE;
    }

    if ((pstNextDispFrm->stFrmInfo.u32LumaAddr== 0) || (pstNextDispFrm->stFrmInfo.u32ChromaAddr == 0))
    {
        return FALSE;
    }

    {
        //stNextDispFrm->stFrmInfo.u32LumaAddr;
        //stNextDispFrm->stFrmInfo.u32ChromaAddr;
        //stNextDispFrm->stFrmInfo.u32TimeStamp;
        //stNextDispFrm->stFrmInfo.u32ID_L;
        //stNextDispFrm->stFrmInfo.u32ID_H;
        //stNextDispFrm->stFrmInfo.u16Pitch;
        //stNextDispFrm->stFrmInfo.u16Width;
        //stNextDispFrm->stFrmInfo.u16Height;
        //stNextDispFrm->stFrmInfo.eFrameType;
        //stNextDispFrm->stFrmInfo.eFieldType;
        //stNextDispFrm->u32PriData;
        //stNextDispFrm->u32Idx;
        //eScanMode  = (mapi_vdec_datatype::EN_MAPI_VDEC_SCAN_MODE)((stNextDispFrm->stFrmInfo.u32ID_L >> 19) & 0x03);
        b10Bit = (pstNextDispFrm->stFrmInfo.u32ID_L >> 21) & 0x01;
        FUNC_DBG("[Done]timestamp=%x\n", tGetCtrl.tParam.nextDispFrame.dispFrm.stFrmInfo.u32TimeStamp);
    }

    /*------- check support MFDEC or not ---------*/
    if(sizeof(VDEC_EX_FrameInfoExt_v6) <= 1)
    {
        return TRUE;
    }

    if (_gPerfTestMode)
        goto EXIT_GetNextDispFrame;

    memcpy((void*)&tGetCtrl2.tStreamId, pStreamId, sizeof(VDEC_StreamId));
    pstFrmInfoExt_v6 = &tGetCtrl2.tParam.getNextDispFrameInfoExt.sFrameInfoExt_v6;
    memset(pstFrmInfoExt_v6, 0, sizeof(tGetCtrl2.tParam.getNextDispFrameInfoExt.sFrameInfoExt_v6));
    pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.stVerCtl.u32version = 6;
    pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.stVerCtl.u32size = sizeof(VDEC_EX_FrameInfoExt_v6);

    tGetCtrl2.eCmdId = E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT;
    //DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl2, "CMD GET_NEXT_DISP_FRAME_INFO_EXT;");
    DO_IOCTL_NCHK(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl2, "CMD GET_NEXT_DISP_FRAME_INFO_EXT;");

    //if (E_VDEC_EX_OK == MApi_VDEC_EX_GetControl(pStreamId, E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT, (MS_U32*)&stFrmInfoExt_v6))
    {
        MS_U32 u32MFCodecInfo = pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32MFCodecInfo;
        EN_MAPI_VDEC_MFCODEC_VERSION MfDecVersion = (EN_MAPI_VDEC_MFCODEC_VERSION)(u32MFCodecInfo & 0xff);
        if((MfDecVersion != E_MAPI_VDEC_MFCODEC_UNSUPPORT) && (MfDecVersion != E_MAPI_VDEC_MFCODEC_DISABLE))
        {
            /*stDbInfo[m_VdecDecoderPath].bDbEnable = TRUE;
            stDbInfo[m_VdecDecoderPath].bBypassCodecMode = MAPI_FALSE;
            stDbInfo[m_VdecDecoderPath].bUncompressMode = (u32MFCodecInfo >> 28) & 0x1;
            stDbInfo[m_VdecDecoderPath].u8DbSelect = (MAPI_U8)((u32MFCodecInfo >> 8) & 0x1);
#ifdef MFDEC_HARDWARE_LIMITATION_PATCH
            //Fix me: MFDEC0 share clk with MVOP0, so shouldn't use MFDEC0
            stDbInfo[m_VdecDecoderPath].u8DbSelect = 1;
#endif
            stDbInfo[m_VdecDecoderPath].eDbMode = (mapi_vdec_datatype::EN_MAPI_VDEC_DB_MODE)((u32MFCodecInfo >> 29) & 0x1);
            stDbInfo[m_VdecDecoderPath].u16HSize = stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.sFrameInfo.u16Width;
            stDbInfo[m_VdecDecoderPath].u16VSize = stFrmInfoExt_v6.sFrameInfoExt_v5.sFr                                                                                                                                                                                                                                                                                                                                                                                                                                                         ameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.sFrameInfo.u16Height;
            stDbInfo[m_VdecDecoderPath].phyDbBase = (MAPI_PHY)stFrmInfoExt_v6.sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen;
            stDbInfo[m_VdecDecoderPath].u16DbPitch = (MAPI_U16)((u32MFCodecInfo >> 16) & 0xFF);
            stDbInfo[m_VdecDecoderPath].u8DbMiuSel = (MAPI_U8)((u32MFCodecInfo >> 24) & 0x0F);
            stDbInfo[m_VdecDecoderPath].phyLbAddr = (MAPI_PHY)stFrmInfoExt_v6.u32HTLBEntriesAddr;
            stDbInfo[m_VdecDecoderPath].u8LbSize = stFrmInfoExt_v6.u8HTLBEntriesSize;
            stDbInfo[m_VdecDecoderPath].u8LbTableId = stFrmInfoExt_v6.u8HTLBTableId;
            */
/*#if defined(HEVC_LEVEL_40)
          u32bitlenSize = ((MfDecVersion >= E_MAPI_VDEC_MFCODEC_30) ? LEVEL4_MFCODEC30_BITLEN_MAX_SIZE
                                                                       : LEVEL4_MFCODEC_BITLEN_MAX_SIZE);
#elif defined(HEVC_LEVEL_50)
          u32bitlenSize = ((MfDecVersion >= E_MAPI_VDEC_MFCODEC_30) ? LEVEL5_MFCODEC30_BITLEN_MAX_SIZE
                                                                       : LEVEL5_MFCODEC_BITLEN_MAX_SIZE);
#endif*/
            u32bitlenSize = MFCodec_blens_size(pstNextDispFrm->stFrmInfo.u16Width,
            pstNextDispFrm->stFrmInfo.u16Height);

            eTileMode = (EN_MAPI_VDEC_TILE_MODE)(pstFrmInfoExt_v6->eTileMode);
            FUNC_INFO_WHITE("MFcodec EN: bitlen=(%x,%x),blenSize=%x,ver=%x,tile=%d,id=%d\n",
                     pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen,
                     pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32ChromaMFCbitlen,
                     u32bitlenSize, MfDecVersion, eTileMode,
                     (MS_U8)((u32MFCodecInfo >> 8) & 0x1)
                     );

        }
        else
        {
            //stDbInfo[m_VdecDecoderPath].bDbEnable = MAPI_FALSE;
            //stDbInfo[m_VdecDecoderPath].bBypassCodecMode = TRUE;
            FUNC_MSG("[No MfCodec] ver=%x, tile=%d\n", MfDecVersion, pstFrmInfoExt_v6->eTileMode);
        }
    }


    /*-------------- dump file ----------------*/
    tile_x = (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC) ? H265_TILE_X_GRP : H264_TILE_X;
    tile_y = (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC) ? H265_TILE_Y_GRP : H264_TILE_Y;

    //u32Pitch = (pstNextDispFrm->stFrmInfo.u16Width + tile_x-1) / tile_x * tile_x;
    //u32LumaSz = u32Pitch * ((pstNextDispFrm->stFrmInfo.u16Height+ tile_y-1) / tile_y * tile_y);
    yPitch  = (pstNextDispFrm->stFrmInfo.u16Width + tile_x-1) / tile_x * tile_x;
    yHeight = (pstNextDispFrm->stFrmInfo.u16Height+ tile_y-1) / tile_y * tile_y;
    u32LumaSz = yPitch * yHeight;

    uvPitch  = (pstNextDispFrm->stFrmInfo.u16Width/2 + tile_x-1) / tile_x * tile_x;
    uvHeight = (pstNextDispFrm->stFrmInfo.u16Height/2 + tile_y-1) / tile_y * tile_y;
    u32ChromaSz = uvPitch * uvHeight * 2;

    memset(name, 0x0, sizeof(name));
    FUNC_DBG("u16Width=%d u16Height=%d u32LumaSz=%d \n", pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height, u32LumaSz);

    if(_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC)
        strcpy(typeStr, "h265");
    else if(_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_H264)
        strcpy(typeStr, "h264");
    else if(_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG)
	strcpy(typeStr, "jpg");

    if(_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG)
        u32LumaSz = pstNextDispFrm->stFrmInfo.u16Width * pstNextDispFrm->stFrmInfo.u16Height * 2;

    if(_gu32DispCnt[u16ChnNum] < gVdecTestCfg.u32DumpFrameNum)
    {
        FUNC_INFO_WHITE("[%d:%d]u32LumaAddr=0x%llx u32ChromaAddr=0x%llx \n", u16ChnNum,_gu32DispCnt[u16ChnNum],
                                                                pstNextDispFrm->stFrmInfo.u32LumaAddr, pstNextDispFrm->stFrmInfo.u32ChromaAddr);
        if(_geCodecType[u16ChnNum] != E_VDEC_EX_CODEC_TYPE_MJPEG)
        {
            sprintf(name, "%s/ch%d%sLuma_frame%d_(%d_%d).yuyv", DUMP_FILE_PATH, u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                    pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
            _DUMP_BufferToFile(pstNextDispFrm->stFrmInfo.u32LumaAddr, u32LumaSz, name);
            memset(name, 0x0, sizeof(name));
            sprintf(name, "%s/ch%d%sChroma_frame%d_(%d_%d).yuyv", DUMP_FILE_PATH, u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                    pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
            _DUMP_BufferToFile(pstNextDispFrm->stFrmInfo.u32ChromaAddr, u32ChromaSz, name);

            /* dump MfCodec bitlen */
            if ((u32bitlenSize > 0) && (pstFrmInfoExt_v6 != NULL))
            {
                MS_U32 phyAddr;
                if (pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen)
                {
                       //VPU2PHY_ADDR(pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen, phyAddr, fw_buffer_phyAddr);
                       //FUNC_INFO_GREEN("%x %x %x\n", pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen, phyAddr, fw_buffer_phyAddr );
                        phyAddr = pstFrmInfoExt_v6->sFrameInfoExt_v5.sFrameInfoExt_v4.sFrameInfoExt_v3.sFrameInfoExt.u32LumaMFCbitlen;
                       sprintf(name, "%s/ch%d%sBitlen_%d_(%d_%d).bin", DUMP_FILE_PATH, u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                               pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
                       _DUMP_BufferToFile(phyAddr, u32bitlenSize, name);
                 }
             }
       }
        else
        {
              sprintf(name, "%s/ch%d_%s_frame%d_(%d_%d).yuv", DUMP_FILE_PATH, u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                        pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
               _DUMP_BufferToFile(pstNextDispFrm->stFrmInfo.u32LumaAddr, u32LumaSz, name);
        }

        u32DumpVdecFrameCount++;
    }

    /*-------------- MD5 ----------------*/
#ifdef _ENABLE_MD5_CALC
    #ifdef _ENABLE_GPIO_CONTROL
        system("echo 0 > /sys/class/gpio/gpio53/value");
    #endif

        if (gVdecTestCfg.eMd5Mode != E_VDEC_MD5_NOT_VERIFY)
        {
            MD5_Calc(pstNextDispFrm->stFrmInfo.u32LumaAddr, pstNextDispFrm->stFrmInfo.u32ChromaAddr,
                       pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height, yPitch,
                       (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC) ? H265_TILE_X : H264_TILE_X,
                       (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_HEVC) ? H265_TILE_Y : H264_TILE_Y,
                       md5);
            dump_md5(3, md5);
        }
    #ifdef _ENABLE_GPIO_CONTROL
        system("echo 1 > /sys/class/gpio/gpio53/value");
    #endif


    if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_BYFRAME_VERIFY)
    {
        if (!MD5_VerifyResult(u16ChnNum, _gu32DispCnt[u16ChnNum], md5))
        {
    #ifdef _ENABLE_MD5_ERR_DUMP
            sprintf(name, "%s/run%dch%d%sLuma_frame%d_(%d_%d).yuyv", DUMP_FILE_PATH,gVdecTestCfg.u32IdNum,
                u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                  pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
            _DUMP_BufferToFile(pstNextDispFrm->stFrmInfo.u32LumaAddr, u32LumaSz, name);

            sprintf(name, "%s/run%dch%d%sChroma_frame%d_(%d_%d).yuyv", DUMP_FILE_PATH,gVdecTestCfg.u32IdNum,
                u16ChnNum, typeStr, _gu32DispCnt[u16ChnNum], //u32DumpVdecFrameCount,
                  pstNextDispFrm->stFrmInfo.u16Width, pstNextDispFrm->stFrmInfo.u16Height);
            _DUMP_BufferToFile(pstNextDispFrm->stFrmInfo.u32ChromaAddr, u32ChromaSz, name);
    #endif
            if (!gVdecTestCfg.u32Md5FailGo)
            {
				_gnVdecSuccess = FALSE;
				for (i = 0; i < _gu8NumOfVdecChannel; i++)
				{
				  _gStopEsGetThr[i] = TRUE;
				  _gStopEsSndThr[i] = TRUE;
				}
            }
        }
        else
        {
    #ifdef _CLEAR_BY_FRAME
            memset((void*)MsOS_PA2KSEG1(pstNextDispFrm->stFrmInfo.u32LumaAddr), 0, u32LumaSz);
            memset((void*)MsOS_PA2KSEG1(pstNextDispFrm->stFrmInfo.u32ChromaAddr), 0, u32ChromaSz);
    #endif
        }
    }  /* E_VDEC_MD5_BYFRAME_VERIFY */
    else if ((gVdecTestCfg.eMd5Mode == E_VDEC_MD5_GEN) && (u16ChnNum == 0)) /* support one CH dump only */
    {
        MD5_DumpResult(md5);
    }
#endif /* _ENABLE_MD5_CALC */


EXIT_GetNextDispFrame:
    _gu32DispCnt[u16ChnNum]++;


    /*----------- Release frame -------------------*/

#ifdef _ENABLE_RELEASE_FRAME
    //MApi_VDEC_EX_ReleaseFrame(pStreamId, pstRelFrm);
    tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_RELEASE_FRAME;
    memcpy(&tSetCtrl.tParam.releaseFrame.dispFrm, pstNextDispFrm, sizeof(VDEC_EX_DispFrame));
    DO_IOCTL3(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl, "RELEASE_FRAME");
    FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_RELEASE_FRAME[%d] \n", u16ChnNum);
#endif

    return TRUE;
}

static int GetFrameCnt(VDEC_StreamId *pStreamId, MS_U16 u16ChnNum)
{
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl;

    memcpy((void*)&tGetCtrl.tStreamId, pStreamId, sizeof(VDEC_StreamId));
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_FRAME_CNT;
    DO_IOCTL2(_gnDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);

    return tGetCtrl.tParam.frameCnt.u32Cnt;
}

static int GetErrCnt(VDEC_StreamId *pStreamId, MS_U16 u16ChnNum)
{
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl;

    memcpy((void*)&tGetCtrl.tStreamId, pStreamId, sizeof(VDEC_StreamId));
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_ERR_CNT;
    DO_IOCTL2(_gnDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);

    return tGetCtrl.tParam.errCnt.u32Cnt;
}

static int GetErrCode(VDEC_StreamId *pStreamId, MS_U16 u16ChnNum)
{
    IOCTL_PARAM_VDEC_GET_CONTROL_t     tGetCtrl;

    memcpy((void*)&tGetCtrl.tStreamId, pStreamId, sizeof(VDEC_StreamId));
    tGetCtrl.eCmdId = E_VDEC_EX_USER_CMD_GET_ERR_CODE;
    DO_IOCTL2(_gnDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);

    return tGetCtrl.tParam.errCode.eErrCode;
}

static int GetDecFrame(void *args)
{
    MS_U16 u16ChnNum = (MS_U32)args;
    int nRetryCnt;
    int i;
    struct timespec tStartTime, tEndTime;
    char sPthreadName[16];

    snprintf(sPthreadName, sizeof(sPthreadName), "GetFrame%02d", u16ChnNum);
    pthread_setname_np(pthread_self(), sPthreadName);

#if 0 // sylvia: MJPEG debug
    if (_geCodecType[u16ChnNum] == E_VDEC_EX_CODEC_TYPE_MJPEG)  return 0;
#endif

    _gnDecSucCnt[u16ChnNum] = 0;
    if (!_gPerfTestMode)
        usleep(100000);
    clock_gettime(CLOCK_MONOTONIC, &tStartTime);
    while(FALSE == _gStopEsGetThr[u16ChnNum])
    {
          if (!GetNextDispFrame(_gnDev, &_gStreamId[u16ChnNum], u16ChnNum))
          {
               usleep(10000); //usleep(150000);
               nRetryCnt++;
               if (!_gPerfTestMode)
                   FUNC_DBG("CH%d nRetryCnt=%d GetNextDispFrame fail \n", u16ChnNum,nRetryCnt);
          }
          else
          {
              nRetryCnt = 0;
              _gnDecSucCnt[u16ChnNum]++;
              if (!_gPerfTestMode)
                  FUNC_INFO_GREEN("CH%d dispCnt=%d GetDispCnt=%d GetErrCnt=%d GetErrCode=0x%X\n",
                                   u16ChnNum,
                                  _gnDecSucCnt[u16ChnNum],
                                  GetFrameCnt(&_gStreamId[u16ChnNum], u16ChnNum),
                                  GetErrCnt(&_gStreamId[u16ChnNum], u16ChnNum),
                                  GetErrCode(&_gStreamId[u16ChnNum], u16ChnNum));

               if ( (gVdecTestCfg.u32MaxFrameNum > 0)/* infinitely run */ &&
                 (_gSingleFrameMode || (_gu32DispCnt[u16ChnNum] == gVdecTestCfg.u32MaxFrameNum)) )
              {
                  _gStopEsGetThr[u16ChnNum] = TRUE;
                  _gStopEsSndThr[u16ChnNum] = TRUE;
              }
          }
#if 0
          if ((_gMd5_golden_num[u16ChnNum] != 0) && (_gu32DispCnt[u16ChnNum] >= _gMd5_golden_num[u16ChnNum]))
          {
              _gStopEsGetThr[u16ChnNum] = TRUE;
              _gStopEsSndThr[u16ChnNum] = TRUE;
          }
#endif
        if (_gPerfTestMode)
        {
            clock_gettime(CLOCK_MONOTONIC, &tEndTime);
            if (((tEndTime.tv_sec * 1000 + tEndTime.tv_nsec / 1000000) -
                (tStartTime.tv_sec * 1000 + tStartTime.tv_nsec / 1000000)) > _gPerfTestMode * 1000)
            {
                _gnDecTotalCnt[u16ChnNum] += _gnDecSucCnt[u16ChnNum];
                FUNC_INFO_GREEN("CH%d decode %d frames in %d seconds(total %d)\n", u16ChnNum, _gnDecSucCnt[u16ChnNum], _gPerfTestMode, _gnDecTotalCnt[u16ChnNum]);
                _gnDecSucCnt[u16ChnNum] = 0;
                clock_gettime(CLOCK_MONOTONIC, &tStartTime);
            }
        }

        usleep(5000);
    }


    FUNC_INFO_GREEN("[Finish]CH%d dispCnt=%d GetDispCnt=%d GetErrCnt=%d GetErrCode=0x%X\n", u16ChnNum,
                                                                                            _gnDecSucCnt[u16ChnNum],
                                                                                            GetFrameCnt(&_gStreamId[u16ChnNum], u16ChnNum),
                                                                                            GetErrCnt(&_gStreamId[u16ChnNum], u16ChnNum),
                                                                                            GetErrCode(&_gStreamId[u16ChnNum], u16ChnNum));
    return _gnDecSucCnt[u16ChnNum];
}


//=============================================================================
//
//=============================================================================
int vdec_init()
{
    int nDev = _gnDev;
    int cnt = 0;
    int i = 0;
    MS_U32 u32BufOffset = 0;
    IOCTL_PARAM_VDEC_GET_FREE_STREAM_t tGetStreamId;
    IOCTL_PARAM_VDEC_SET_CONTROL_t     tSetCtrl;
    IOCTL_PARAM_VDEC_PRESET_CONTROL_t  tPreCtrl;
    IOCTL_PARAM_VDEC_INIT_t            tInit;

    for(i = 0; i < _gu8NumOfVdecChannel; i++)
    {
        /****** GetFreeStream ******/
        memset(&tGetStreamId, 0, sizeof(IOCTL_PARAM_VDEC_GET_FREE_STREAM_t));
        tGetStreamId.tParam.u32Size = sizeof(VDEC_StreamId);
        tGetStreamId.tParam.eStreamType = 2;//E_VDEC_EX_MAIN_STREAM; /*2*/
        tGetStreamId.tParam.eCodecType = _geCodecType[i]; /*11: E_VDEC_EX_CODEC_TYPE_H264*/
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_GET_FREE_STREAM, (void*)&tGetStreamId);
        memcpy(&_gStreamId[i], &tGetStreamId.tParam.streamId, sizeof(VDEC_StreamId));
        FUNC_DBG("Stream ID : %d \n", _gStreamId[i].u32Id);

        /****** PreSetControl ******/
        memcpy(&tPreCtrl.tStreamId, &_gStreamId[i], sizeof(VDEC_StreamId));

        //if (_geCodecType[i] != E_VDEC_EX_CODEC_TYPE_MJPEG)
        {
        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_DECODE_MODE;
        VDEC_EX_DecModCfg *ptDecModCfg = &tPreCtrl.tParam.decodeMode.tDecModCfg;
        memset(ptDecModCfg, 0, sizeof(VDEC_EX_DecModCfg));
        ptDecModCfg->eDecMod = E_VDEC_EX_DEC_MODE_DUAL_INDIE;
        ptDecModCfg->u32Arg = E_VDEC_EX_DEC_PIP_SYNC_MAIN_STC;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        }

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_CONNECT_INPUT_TSP;
        tPreCtrl.tParam.connectInputTsp.eInputTSP = E_VDEC_EX_INPUT_TSP_NONE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH;
        VDEC_EX_DynmcDispPath *ptDynmcDispPath = &tPreCtrl.tParam.connectDisplayPath.tDynmcDispPath;
        memset(ptDynmcDispPath, 0x0, sizeof(VDEC_EX_DynmcDispPath));
        ptDynmcDispPath->bConnect  = TRUE;
        ptDynmcDispPath->eMvopPath = E_VDEC_EX_DISPLAY_PATH_NONE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE;
        tPreCtrl.tParam.displayMode.eDispMode = E_VDEC_EX_DISPLAY_MODE_MCU;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        FUNC_DBG("MApi_VDEC_EX_PreSetControl:E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE\n");

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY;
        tPreCtrl.tParam.bitstreamBufferMonopoly.bMonopolyBitstreamBuffer = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        FUNC_DBG("MApi_VDEC_EX_PreSetControl: E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY\n");

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY;
        tPreCtrl.tParam.frameBufferMonopoly.bMonopolyFrameBuffer = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        FUNC_DBG("MApi_VDEC_EX_PreSetControl: E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY\n");

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE;
        tPreCtrl.tParam.dynamicCMAMode.bEnableDynamicCMA = FALSE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        FUNC_DBG("MApi_VDEC_EX_PreSetControl:E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE\n");

        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_MFCODEC_MODE;
        tPreCtrl.tParam.mfCodecMode.eMFCodecMode = _gEnableMFCodec ? E_VDEC_EX_MFCODEC_FORCE_ENABLE : E_VDEC_EX_MFCODEC_FORCE_DISABLE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
        FUNC_DBG("MApi_VDEC_EX_PreSetControl:E_VDEC_EX_USER_CMD_MFCODEC_MODE (mode=%d)\n", tPreCtrl.tParam.mfCodecMode.eMFCodecMode);

#if 0
        ///set dv xc shm info: MApi_VDEC_EX_PreSetControl(E_VDEC_EX_USER_CMD_SET_BUFFER_INFO)
        tPreCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_BUFFER_INFO;
        memset(&tPreCtrl.tParam.bufferInfo.tBufInfo, 0, sizeof(VDEC_EX_BufferInfo));
        tPreCtrl.tParam.bufferInfo.tBufInfo.eType     = E_VDEC_EX_BUFFERTYPE_HDRSHM_MAIN;
        tPreCtrl.tParam.bufferInfo.tBufInfo.phyAddr = xcshmBuf.u32PhyAddr;
        tPreCtrl.tParam.bufferInfo.tBufInfo.szSize = xcshmBuf.u32Size;
        tPreCtrl.tParam.bufferInfo.tBufInfo.u32Config = 0;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_PRE_SET_CONTROL, (void*)&tPreCtrl);
#endif
        /*=============  init ==================*/
        memcpy(&tInit.tStreamId, &_gStreamId[i], sizeof(VDEC_StreamId));
        memset(&tInit.tParam, 0, sizeof(VDEC_EX_InitParam));
        tInit.tParam.u32Version = 0;
        tInit.tParam.eCodecType = _geCodecType[i]; /* E_VDEC_EX_CODEC_TYPE_HEVC(16) or E_VDEC_EX_CODEC_TYPE_H264(11) */
        tInit.tParam.SysConfig.u32FWBinaryAddr = fw_buffer_phyAddr;  //0x80000000;
        tInit.tParam.SysConfig.u32FWBinarySize = fw_buffer_size;  //0x200000 + ((_gu8NumOfVdecChannel - 1) * 0x100000);
        tInit.tParam.SysConfig.u32CodeBufAddr = fw_buffer_phyAddr; //0x80000000;
        tInit.tParam.SysConfig.u32CodeBufSize = fw_buffer_size;  //0x200000 + ((_gu8NumOfVdecChannel - 1) * 0x100000);
        tInit.tParam.SysConfig.u32BitstreamBufAddr = vdec_buffer_phyAddr + u32BufOffset;//0x80200000 + ((_gu8NumOfVdecChannel - 1) * 0x100000) + (i * 0x3000000);
        //tInit.tParam.SysConfig.u32BitstreamBufSize = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 0xC20000:0x100000;
        tInit.tParam.SysConfig.u32BitstreamBufSize = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 0xC20000:BS_BUFFER_SIZE;
#if 0
        //tInit.tParam.SysConfig.u32FrameBufSize = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_H264) ? 0xE00000 : 0x1800000;
        tInit.tParam.SysConfig.u32FrameBufAddr = tInit.tParam.SysConfig.u32BitstreamBufAddr + tInit.tParam.SysConfig.u32BitstreamBufSize+ 0x100000; //0x81A00000 + (i * 0x3000000);
        tInit.tParam.SysConfig.u32FrameBufSize = _gu32BufSize[i];
        tInit.tParam.SysConfig.u32DrvProcBufAddr = tInit.tParam.SysConfig.u32BitstreamBufAddr + tInit.tParam.SysConfig.u32BitstreamBufSize; //0x80200000 + ((_gu8NumOfVdecChannel - 1) * 0x100000) + (i * 0x3000000);
        tInit.tParam.SysConfig.u32DrvProcBufSize = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 0x3FC000:0xA000;
#else  //might be error
        tInit.tParam.SysConfig.u32FrameBufAddr = tInit.tParam.SysConfig.u32BitstreamBufAddr + tInit.tParam.SysConfig.u32BitstreamBufSize; //0x81A00000 + (i * 0x3000000);
        tInit.tParam.SysConfig.u32FrameBufSize = _gu32BufSize[i];
        tInit.tParam.SysConfig.u32DrvProcBufAddr = tInit.tParam.SysConfig.u32BitstreamBufAddr; //0x80200000 + ((_gu8NumOfVdecChannel - 1) * 0x100000) + (i * 0x3000000);
        tInit.tParam.SysConfig.u32DrvProcBufSize = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 0x3FC000:0xA000;
#endif

        tInit.tParam.SysConfig.u32VlcTabBinarySize = 0;
        tInit.tParam.SysConfig.u32VlcBinarySrcAddr = 0x00000000;
        tInit.tParam.SysConfig.eDbgMsgLevel = _gDbgLevel; //E_VDEC_EX_DBG_LEVEL_TRACE;
        tInit.tParam.SysConfig.eFWSourceType = E_VDEC_FW_SOURCE_NONE; /* 0 */
        tInit.tParam.VideoInfo.eSrcMode =  E_VDEC_SRC_MODE_FILE; /* 2 */
        tInit.tParam.VideoInfo.eTimeStampType = E_VDEC_TIME_STAMP_PTS; /* 1 */
        tInit.tParam.VideoInfo.eMJpegScaleFactor = 0;
        tInit.tParam.VideoInfo.bWithoutNalStCode = FALSE;
        tInit.tParam.VideoInfo.u32FrameRate = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 30:0;
        tInit.tParam.VideoInfo.u32FrameRateBase = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) ? 1000:0;
        tInit.tParam.EnableDynaScale = 0;
        tInit.tParam.bDisableDropErrFrame = 1;
        tInit.tParam.bDisableErrConceal = 0;
        tInit.tParam.bRepeatLastField = 1;
        tInit.tParam.u8ErrThreshold = 0;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_INIT, (void*)&tInit);
        u32BufOffset += tInit.tParam.SysConfig.u32BitstreamBufSize + tInit.tParam.SysConfig.u32FrameBufSize;
        FUNC_DBG("MApi_VDEC_EX_Init Done\n");


        /*=============  set control ==================*/
        //DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        memcpy(&tSetCtrl.tStreamId, &_gStreamId[i], sizeof(VDEC_StreamId));

        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_AVSYNC_ON;
        tSetCtrl.tParam.avSyncOn.bOn = FALSE;
        tSetCtrl.tParam.avSyncOn.u32SyncDelay = 0;
        tSetCtrl.tParam.avSyncOn.u16SyncTolerance = 0;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_AVSYNC_ON\n");

        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_BLOCK_DISPLAY;
        tSetCtrl.tParam.setBlockDisplay.bEnable = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_SET_BLOCK_DISPLAY\n");

#if 0 // not support, always return FAIL
        printf("MApi_VDEC_EX_SetControl:E_VDEC_EX_V2_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE\n");
        if(E_VDEC_EX_OK != MApi_VDEC_EX_SetControl(pInfo, E_VDEC_EX_V2_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE, 1/*bEnable*/))
        {
            printf(" [%s]....SetControl AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE : fail \n", __PRETTY_FUNCTION__);
            return MAPI_FALSE;
        }
#endif


        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_ENABLE_ES_BUFF_MALLOC;
        tSetCtrl.tParam.enableEsBufMalloc.bEnable = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_ENABLE_ES_BUFF_MALLOC\n");

        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE;
        tSetCtrl.tParam.setDispOutsideCtrlMode.bEnable = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE\n");

        /*  not support: always return FIAL
         *  tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE;
            tSetCtrl.tParam.setDispFinishMode.bEnable = TRUE;
            DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
            printf("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE\n");
        */
        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_DISABLE_DEBLOCKING;
        tSetCtrl.tParam.setDisableDeblocking.bDisable = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_DISABLE_DEBLOCKING\n");

#if 0
        printf("MApi_VDEC_EX_SetControl:E_VDEC_EX_V2_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE\n");
        if(E_VDEC_EX_OK != MApi_VDEC_EX_SetControl(pInfo, E_VDEC_EX_V2_USER_CMD_AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE, 1/*bEnable*/))
        {
            printf(" [%s]....SetControl AVC_SUPPORT_REF_NUM_OVER_MAX_DPB_SIZE : fail \n", __PRETTY_FUNCTION__);
            return MAPI_FALSE;
        }
#endif

        /*     printf("MApi_VDEC_EX_SetControl:E_VDEC_EX_V2_USER_CMD_ENABLE_ES_BUFF_MALLOC\n");
             if(E_VDEC_EX_OK != MApi_VDEC_EX_EnableESBuffMalloc(pInfo, MAPI_TRUE))
             {
                 printf(" [%s]....SetControl EnableESBuffMalloc : fail \n", __PRETTY_FUNCTION__);
                 return MAPI_FALSE;
             }

             if(E_VDEC_EX_OK != MApi_VDEC_EX_AVSyncOn(pInfo, MAPI_FALSE, 0, 0))
             {
                 printf(" [%s]....SetControl AVSyncOn : fail \n", __PRETTY_FUNCTION__);
                 return MAPI_FALSE;
             }
        */
        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_PLAY ;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_PLAY \n");

#if 0
        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER ;
        tSetCtrl.tParam.showDecodeOrder.bEnable = TRUE;
        DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER \n");
#endif

        tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_SET_FW_DEBUG_LEVEL ;
        tSetCtrl.tParam.fwDebug.u32DebugLevel = VDEC_FW_DBG_LV_ERROR | VDEC_FW_DBG_LV_FLOW;
        DO_IOCTL2(nDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
        FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_SET_FW_DEBUG_LEVEL \n");
    }

    //DO_IOCTL(nDev, IOCTL_CMD_VDEC_EX_GET_CONTROL, (void*)&tGetCtrl);
    /* read bitstream and pushDecQ */
    #ifdef _ENABLE_GPIO_CONTROL
    system("echo 53 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio53/direction");
    #endif
    return 0;
}

int vdec_done(void)
{
    int i;
    int ret = TRUE;
    IOCTL_PARAM_VDEC_SET_CONTROL_t     tSetCtrl;
    char md5_fname[MAX_PATH];
    #ifdef _ENABLE_GPIO_CONTROL
    system("echo 53 > /sys/class/gpio/unexport");
    #endif
    if (_gnDev >= 0)
    {
        for(i = 0; i < _gu8NumOfVdecChannel; i++)
        {
			//if(_geCodecType[i] != E_VDEC_EX_CODEC_TYPE_MJPEG)
			{
			    FUNC_DBG("MApi_VDEC_EX_SetControl: E_VDEC_EX_USER_CMD_EXIT (ch %d)\n", i);
				memcpy(&tSetCtrl.tStreamId, &_gStreamId[i], sizeof(VDEC_StreamId));
				tSetCtrl.eCmdId = E_VDEC_EX_USER_CMD_EXIT;
				DO_IOCTL_NCHK(_gnDev, IOCTL_CMD_VDEC_EX_SET_CONTROL, (void*)&tSetCtrl);
			}

            FUNC_MSG("ch %d: init=%d, isp=%d\n", i,_gu32InitCnt[i], _gu32DispCnt[i]);
        }
    }

    return ret;
}

//=============================================================================
//
//=============================================================================
static void _vdec_test_initVaraibales(VDEC_TEST_CFG *ptCfg)
{
    int i = 0;
    char md5_fname[MAX_PATH];

    memcpy(&gVdecTestCfg, ptCfg, sizeof(VDEC_TEST_CFG));
    /* if ((gVdecTestCfg.u32MaxFrameNum <= 0) || (gVdecTestCfg.u32MaxFrameNum > MAX_FRAME_NUM))
    {
        gVdecTestCfg.u32MaxFrameNum = MAX_FRAME_NUM;
    }*/

    if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_GEN)
    {
        gVdecTestCfg.u32DumpFrameNum = 0;
    }
    else if (gVdecTestCfg.u32DumpFrameNum > MAX_DUMP_FRAME_NUM)
    {
        gVdecTestCfg.u32DumpFrameNum = MAX_DUMP_FRAME_NUM;
    }

    _gu8NumOfVdecChannel = ptCfg->u8NumOfVdecChannel;
    _gSingleFrameMode    = ptCfg->nSingleFrameMode;
    _gPerfTestMode       = ptCfg->nPerfTestMode;
    _gDbgLevel = ptCfg->eDbgLevel;
    _gUtDbgLevel = ptCfg->eUtDbgLevel;
    _gEnableMFCodec = ptCfg->nEnableMFCodec;
    _gnAutoMode = ptCfg->u32AutoMode;

    for (i = 0; i < _gu8NumOfVdecChannel; i ++)
    {
        _geCodecType[i] = ptCfg->tChannelCfg[i].eCodecType;
        strcpy(_gFileName[i], ptCfg->tChannelCfg[i].fileName);

        memset(&_gStreamId[i], 0, sizeof(VDEC_StreamId));
        _gStopEsSndThr[i] = 0;
        _gStopEsGetThr[i] = 0;
        _gnDecSucCnt[i] = 0;
        _gnDecFaiCnt[i] = 0;
        _gnDecTotalCnt[i] = 0;

        _gu32InitCnt[i] = 0;
        _gu32DispCnt[i] = 0;

        if (ptCfg->tChannelCfg[i].u32BufSize == 0)
        {
            _gu32BufSize[i] = (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_H264) ? 0x1000000:0x2000000; //0xE00000 : 0x1800000; //init value
        }
        else
        {
          _gu32BufSize[i] = ptCfg->tChannelCfg[i].u32BufSize;
        }

        if (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_H264)
            _gnMinPushNum[i] =  MIN_PUSHNUM_H264;
        else if (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_HEVC)
            _gnMinPushNum[i] =  MIN_PUSHNUM_H265;
        else
            _gnMinPushNum[i] =  1;

        if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_BYFRAME_VERIFY)
        {
            MD5_GetFilename(_gFileName[i], md5_fname);
            MD5_LoadGolden(i, md5_fname);
        }
    }

    _gnDev = -1;
    //frame_cnt = 0;
}

int Vdec_Test_Init(VDEC_TEST_CFG *ptCfg)
{
    int ret = FALSE;
    int totalSize = 0, i;

    /*---------------------------------------------------------------
     *  global variables
     *---------------------------------------------------------------*/
    _vdec_test_initVaraibales(ptCfg);

    /*---------------------------------------------------------------
      * init MD5_Dump
      *---------------------------------------------------------------*/
    if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_GEN) /* only dump CH0 */
    {
         char md5_fname[MAX_PATH];
         MD5_GetFilename(_gFileName[0], md5_fname);
         MD5_DumpInit(md5_fname);
    }

    if (gVdecTestCfg.u32VdecBufSize == 0)
    {
      gVdecTestCfg.u32VdecBufSize = VDEC_BUFFER_SIZE;
    }

    for (i = 0; i < _gu8NumOfVdecChannel; i++ )
    {
        totalSize +=  _gu32BufSize[i];
    }
    if ((totalSize+BS_BUFFER_SIZE) > gVdecTestCfg.u32VdecBufSize)
    {
        FUNC_ERR("vdec_bufSize=0x%x, needSize=0x%0x\n", gVdecTestCfg.u32VdecBufSize, totalSize);
        return ret;
    }

    /*---------------------------------------------------------------
     * alloc and map buffers
     *---------------------------------------------------------------*/
#ifdef CHIP_K6
    if (0 > (_s32MPoolFd = open("/dev/malloc", O_RDWR)))
    {
        return FALSE;
    }

    fw_buffer_offset  = FW_BUFFER_OFFSET;
    fw_buffer_phyAddr  = MIU_INTERVAL * VDEC_MIU + FW_BUFFER_OFFSET;
    fw_buffer_size    = FW_BUFFER_BASESIZE + FW_BUFFER_CHANSIZE * _gu8NumOfVdecChannel;
    vdec_buffer_offset = fw_buffer_offset + fw_buffer_size;
    vdec_buffer_phyAddr = fw_buffer_phyAddr + fw_buffer_size;
    vdec_buffer_size   = gVdecTestCfg.u32VdecBufSize;
    FUNC_MSG("cpu(%x,%x) buf(%x,%x)\n", fw_buffer_phyAddr, fw_buffer_size,
             vdec_buffer_phyAddr, vdec_buffer_size);

    _Vdec_DoMmap();

    {
        MS_PHY *pVaBuf = (MS_PHY *)MsOS_PA2KSEG1(vdec_buffer_phyAddr);
        memset(pVaBuf, 0, vdec_buffer_size);
    }

    #else /* CHIP_I2 */
    char szDmemName[20];
    CamOsRet_e res;
    void* pVirtPtr;
    void* pKvirtPtr;
    void* pPhysPtr;
    void* pMiuPtr;

    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "VDEC_FW");
    fw_buffer_size    = FW_BUFFER_BASESIZE + FW_BUFFER_CHANSIZE * _gu8NumOfVdecChannel;

    res = CamOsDirectMemAllocEx(szDmemName, fw_buffer_size, &pVirtPtr, &pPhysPtr, &pMiuPtr, &pKvirtPtr);
    if (CAM_OS_OK != res)
    {
        FUNC_ERR("Fail: CamOsDirectMemAlloc failed\n");
        return FALSE;
    }
    fw_buffer_phyAddr = (MS_U32)pPhysPtr - 0x20000000;  //sylvia test
    fw_buffer_virtAddr = (MS_U32)pVirtPtr;
    fw_buffer_kVirtAddr = (MS_U32)pKvirtPtr;  /* FIX ME: if not MIU 0 */
    fw_buffer_offset = (MS_U32)pPhysPtr - 0x20000000;;  /* FIX ME: if not MIU 0 */
    FUNC_INFO_GREEN("fw_buffer=%x(%x,%x) %x\n", fw_buffer_phyAddr, fw_buffer_virtAddr, fw_buffer_kVirtAddr,
             fw_buffer_size
             );

    // allocate vdec buf
    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "VDEC_BUF");
    //vdec_buffer_size    = _gAllocBuf; //VDEC_BUFFER_SIZE;
    vdec_buffer_size    = gVdecTestCfg.u32VdecBufSize; // - fw_buffer_size;

    res = CamOsDirectMemAllocEx(szDmemName, vdec_buffer_size, &pVirtPtr, &pPhysPtr, &pMiuPtr, &pKvirtPtr);
    if (CAM_OS_OK != res)
    {
        CamOsDirectMemRelease((void*)fw_buffer_virtAddr, fw_buffer_size);
        FUNC_ERR("Fail: CamOsDirectMemAlloc failed(size=0x%x)\n", vdec_buffer_size);
        return FALSE;
    }
    vdec_buffer_phyAddr = (MS_U32)pPhysPtr - 0x20000000;;
    vdec_buffer_virtAddr = (MS_U32)pVirtPtr;
    vdec_buffer_kVirtAddr = (MS_U32)pKvirtPtr;
    vdec_buffer_offset = (MS_U32)pPhysPtr - 0x20000000;; /* FIX ME: if not MIU 0 */
    FUNC_INFO_GREEN("vdec_buf=%x(%x,%x) %x\n", vdec_buffer_phyAddr, vdec_buffer_virtAddr, vdec_buffer_kVirtAddr,
             vdec_buffer_size);

#if 0    // alloc xcShm
    memset(szDmemName, 0, sizeof(szDmemName));
    snprintf(szDmemName, sizeof(szDmemName), "VDEC_XCSHM");
    xcshmBuf.u32Size    = 4096;

    res = CamOsDirectMemAllocEx(szDmemName, xcshmBuf.u32Size, &pVirtPtr, &pPhysPtr, &pMiuPtr, &pKvirtPtr);
    if (CAM_OS_OK != res)
    {
        CamOsDirectMemRelease((void*)fw_buffer_virtAddr, fw_buffer_size);
        CamOsDirectMemRelease((void*)vdec_buffer_virtAddr, vdec_buffer_size);
        FUNC_ERR("Fail: CamOsDirectMemAlloc failed(size=0x%x)\n", xcshmBuf.u32Size);
        return FALSE;
    }
    xcshmBuf.u32PhyAddr = (MS_U32)pPhysPtr - 0x20000000;;
    xcshmBuf.u32VirtAddr = (MS_U32)pVirtPtr;
    xcshmBuf.u32KVirtAddr = (MS_U32)pKvirtPtr;
    xcshmBuf.u32Offset = (MS_U32)pPhysPtr - 0x20000000;; /* FIX ME: if not MIU 0 */
    FUNC_INFO_GREEN("xcshm_buf=%x(%x,%x) %x\n", xcshmBuf.u32PhyAddr, xcshmBuf.u32VirtAddr, xcshmBuf.u32KVirtAddr,
        xcshmBuf.u32Size);
#endif    /*alloc xcshn */
#endif

    /*---------------------------------------------------------------
     *  open driver handler
     *---------------------------------------------------------------*/
    _gnDev = CamDevOpen(DEV_PATH);
    if(_gnDev < 0)
    {
         CamOsDebug("CamDevOpen %s failed\n\r", DEV_PATH);
         goto VDEC_INIT_ERR;
    }
    CamOsDebug("CamDevOpen %s fd=%d\n\r", DEV_PATH, _gnDev);

    /****** drv DoMmap *******/
    if (!_Vdec_DrvDoMmap(_gnDev))
    {
        CamDevClose(_gnDev);
        CamOsDebug("_Vdec_DrvDoMmap failed\n\r");
        goto VDEC_INIT_ERR;
    }

    /*---------------------------------------------------------------
     *  vdec init
     *---------------------------------------------------------------*/
    vdec_init();
    ret = TRUE;

VDEC_INIT_ERR:
#ifndef  CHIP_K6
    if (!ret)
    {
        CamOsDirectMemRelease((void*)fw_buffer_virtAddr, fw_buffer_size);
        CamOsDirectMemRelease((void*)vdec_buffer_virtAddr, vdec_buffer_size);
#if 0
        CamOsDirectMemRelease((void*)xcshmBuf.u32VirtAddr, xcshmBuf.u32Size);
#endif
     }
#endif

    return ret;
}

int Vdec_Test_Exit(void)
{
    int ret = vdec_done();

    _Vdec_DrvDoUnMap(_gnDev);

   /*---------------------------------------------------------------
   * free buffers
   *---------------------------------------------------------------*/
#ifdef CHIP_K6
    if (0 < _s32MPoolFd)
    {
        close(_s32MPoolFd);
    }
#else
    if (0 != fw_buffer_phyAddr)
    {
        CamOsDirectMemRelease((void*)fw_buffer_virtAddr, fw_buffer_size);
    }
    if (0 != vdec_buffer_phyAddr)
    {
      CamOsDirectMemRelease((void*)vdec_buffer_virtAddr, vdec_buffer_size);
   }
   if (0 != xcshmBuf.u32PhyAddr)
   {
      CamOsDirectMemRelease((void*)xcshmBuf.u32VirtAddr, xcshmBuf.u32Size);
   }
#endif

    /*---------------------------------------------------------------
     *  close driver handler
     *---------------------------------------------------------------*/
    CamDevClose(_gnDev);

    /*---------------------------------------------------------------
       * Exit MD5_Dump
       *---------------------------------------------------------------*/
    if (gVdecTestCfg.eMd5Mode == E_VDEC_MD5_GEN) /* only dump CH0 */
    {
         MD5_DumpExit();
    }

    return ret;
}

/*
static void _DBG_Batch_Dump_Files()
{
    char name[256];
    char typeStr[] = "h265";
    int u16ChnNum = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        sprintf(name, "%s/ch%d%sLuma_frame%d_(1920_1088).yuyv", DUMP_FILE_PATH, u16ChnNum, typeStr, i);
        _DUMP_BufferToFile(_gFrame_luma[i], 0x1FE000, name);
        sprintf(name, "%s/ch%d%sChroma_frame%d_(1920_1088).yuyv", DUMP_FILE_PATH, u16ChnNum, typeStr, i);
        _DUMP_BufferToFile(_gFrame_chroma[i], 0xFF000, name);
    }
}
*/

int Vdec_Test_Run(void)
{
    int i;
    char inputCmd[256] = {0};
    pthread_t    tEsSndThd[MAX_CHANNEL_NUM];
    pthread_t    tGetDecThd[MAX_CHANNEL_NUM];

    if (_gSingleFrameMode)
    {
        printf("SingleFrameMode...........\n");
        SendEsStream((void *)0);
        GetDecFrame((void *)0);
    }
    else
    {
        for(i = 0; i < _gu8NumOfVdecChannel; i++)
        {
#if 0 //sylvia: test
//            if (_geCodecType[i] == E_VDEC_EX_CODEC_TYPE_MJPEG) continue;
#endif
            _gStopEsSndThr[i] = FALSE;
            pthread_create(&tEsSndThd[i], NULL, (void *)SendEsStream, (void *)(MS_U32)i);
            usleep(10000);//usleep(1000000);
            pthread_create(&tGetDecThd[i], NULL, (void *)GetDecFrame, (void *)(MS_U32)i);
        }

        if (!_gnAutoMode)
        {
            while(1)
            {
                fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
                if((strncmp(inputCmd, "q", 1) == 0) || (strncmp(inputCmd, "exit", 1)) )
                {
                    printf("prepare to exit!\n\n");
                    break;
                }
                /*
                else if (strncmp(inputCmd, "f", 1) == 0)  //only correct for 1080p
                {
                  _DBG_Batch_Dump_Files();
                }*/
            }

            for(i = 0; i < _gu8NumOfVdecChannel; i++)
            {
                _gStopEsSndThr[i] = TRUE;
                _gStopEsGetThr[i] = TRUE;
            }

            for(i = 0; i < _gu8NumOfVdecChannel; i++)
            {
                pthread_join( tEsSndThd[i], NULL);
                FUNC_DBG("tEsSndThd[%d] end\n", i);
                pthread_join( tGetDecThd[i], NULL);
                FUNC_DBG("tGetDecThd[%d] end\n", i);
            }
        }
    }
    return _gnVdecSuccess;
}

//====================================================================================================
/*
hvd_MIUI_YUV_to_Golden_YUV_md5(g_fbm->addr_luma[(info->disp.fb_idx[VOIdx])],
                               g_fbm->addr_chroma[(info->disp.fb_idx[VOIdx])],
                               NULL, NULL,
                               info->width, info->height - info->crop_bottom,
                               g_fbm->pitch_luma, 0,
                               0, 0, md5,
                               NULL,
                               HVD_TILE_WIDTH, HVD_TILE_HEIGHT,
                               0);
*/
int hvd_MIUI_YUV_to_Golden_YUV_md5(MS_U8 *pSrcY, MS_U8 *pSrcC, MS_U8 *pSrcY1, MS_U8 *pSrcC1,
                                   int dst_width, int dst_height,
                                   int src_width, int src1_width,
                                   int depth_y_minus8, int depth_c_minus8, unsigned char md5[3][16],
                                   void /*yuv_buf_planar_st*/ *pDstBuf,
                                   int tile_width, int tile_height,
                                   int tile_2bit_width)
{
    int tile_2bit_height = 64;//tile_height;//64;
    int row_st, col_st, row_st1, col_st1;
    int i, j;
    unsigned char* pbuf;
    unsigned char* pbuf1;
    int dst_uv_pitch = (dst_width + 1) / 2, dst_uv_height = (dst_height + 1) / 2;
    MD5 md5Y, md5U, md5V;

    FUNC_DBG("(0x%X,0x%X)(%d,%d,%d,%d)(%d,%d,%d)\n",pSrcY, pSrcC,dst_width, dst_height, src_width, src1_width,
           md5,tile_width, tile_height,tile_2bit_width);
    FUNC_DBG("Y:(%02x%02x%02x%02x %02x%02x%02x%02x)\n",
                    pSrcY[0],pSrcY[1],pSrcY[2],pSrcY[3],pSrcY[4],pSrcY[5],pSrcY[6],pSrcY[7]);

    if(md5) {
        MD5_construct(&md5Y);
        MD5_construct(&md5U);
        MD5_construct(&md5V);
    }
    pbuf = pSrcY;
    pbuf1 = pSrcY1;
    //FUNC_DBG("dst_height = 0x%X, dst_width = 0x%X\n", dst_height, dst_width);
    for (i=0; i<dst_height; i++) {
#if defined(FPGA_VERIFY)
        if (dst_width > 1280 && ((i & 0x7f) == 0))
        {
            OSWRAP_DEBUG("i = 0x%X\n", i);
        }
#endif
        for (j=0; j<dst_width; j++) {
            short bit8_part,bit2_part=0,out16;
            row_st = (i/tile_height)*tile_height;
            col_st = (j/tile_width)*tile_width*tile_height + (i%tile_height)*tile_width + (j%tile_width);
            bit8_part = pbuf[row_st*src_width + col_st];
            if (depth_y_minus8 != 0) {
                //col_st1 = (j/tile_2bit_width)*tile_2bit_width*tile_height + (i%tile_height)*tile_2bit_width + (j%tile_2bit_width);
                row_st1 = (i/tile_2bit_height)*tile_2bit_height;
                col_st1 = (j/tile_2bit_width)*tile_2bit_width*tile_2bit_height + (i%tile_2bit_height)*tile_2bit_width + (j%tile_2bit_width);
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(2*(col_st1%4)))>>(depth_y_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(2*(col_st1%4)))>>(depth_y_minus8==1?1:0));
                out16 = (bit8_part<<depth_y_minus8) | bit2_part;
                if (md5)
                    MD5_update(&md5Y, (unsigned char*)&out16, 2);
                if (pDstBuf) {
                    //pDstBuf->buf_y[i*(2*dst_width)+2*j] = out16;
                    //pDstBuf->buf_y[i*(2*dst_width)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5Y, (unsigned char*)&bit8_part, 1);
                //if(pDstBuf)
                    //pDstBuf->buf_y[i*(dst_width)+j] = bit8_part;
            }
        }
    }
    pbuf = pSrcC;
    pbuf1 = pSrcC1;
    //FUNC_DBG("dst_uv_height = 0x%X, dst_uv_pitch = 0x%X\n", dst_uv_height, dst_uv_pitch);
    for (i=0; i<dst_uv_height; i++) {
#if defined(FPGA_VERIFY)
        if (dst_uv_pitch > 1280 && ((i & 0x7f) == 0))
        {
            OSWRAP_DEBUG("i = 0x%X\n", i);
        }
#endif
        for (j=0; j<(dst_uv_pitch); j++) {
            short bit8_part,bit2_part=0,out16;
            row_st = (i/tile_height)*tile_height;
            col_st = (j*2/tile_width)*tile_width*tile_height + (i%tile_height)*tile_width + ((j*2)%tile_width);
            bit8_part = pbuf[row_st*src_width + col_st];
            if(depth_c_minus8 != 0) {
                //col_st1 = (j*2/tile_2bit_width)*tile_2bit_width*tile_height + (i%tile_height)*tile_2bit_width + ((2*j)%tile_2bit_width);
                row_st1 = (i/tile_2bit_height)*tile_2bit_height;
                col_st1 = (j*2/tile_2bit_width)*tile_2bit_width*tile_2bit_height + (i%tile_2bit_height)*tile_2bit_width + ((2*j)%tile_2bit_width);
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                out16 = (bit8_part<<depth_c_minus8) | bit2_part;
                if(md5)
                    MD5_update(&md5U, (unsigned char*)&out16, 2);
                if(pDstBuf) {
                    //pDstBuf->buf_u[i*(2*dst_uv_pitch)+2*j] = out16;
                    //pDstBuf->buf_u[i*(2*dst_uv_pitch)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5U, (unsigned char*)&bit8_part, 1);
                //if(pDstBuf)
                    //pDstBuf->buf_u[i*(dst_uv_pitch)+j] = bit8_part;
            }

            bit8_part = pbuf[row_st*src_width + col_st+1];
            if(depth_c_minus8 != 0) {
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(2+4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(2+4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                out16 = (bit8_part<<depth_c_minus8) | bit2_part;
                if(md5)
                    MD5_update(&md5V, (unsigned char*)&out16, 2);
                if(pDstBuf) {
                    //pDstBuf->buf_v[i*(2*dst_uv_pitch)+2*j] = out16;
                    //pDstBuf->buf_v[i*(2*dst_uv_pitch)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5V, (unsigned char*)&bit8_part, 1);
                //if(pDstBuf)
                    //pDstBuf->buf_v[i*(dst_uv_pitch)+j] = bit8_part;
            }
        }
    }
    if(md5) {
        MD5_finalize(&md5Y, md5[0]);
        MD5_finalize(&md5U, md5[1]);
        MD5_finalize(&md5V, md5[2]);
    }
    return 0;
}

#if 0
int mfcodec_MIUI_YUV_to_Golden_YUV_md5(U8 *pSrcY, U8 *pSrcC, U8 *pSrcY1, U8 *pSrcC1,
                                   int dst_width, int dst_height,
                                   int src_width, int src_height, int src_uv_width, int src_uv_height, int src1_width,
                                   int depth_y_minus8, int depth_c_minus8, unsigned char md5[3][16],
                                   yuv_buf_planar_st *pDstBuf,
                                   int tile_width, int tile_height,
                                   int tile_2bit_width, U8* yuv_planar_buffer)
{
    int tile_2bit_height = 64;//tile_height;//64;
    int row_st, col_st, row_st1, col_st1;
    int i, j;
    unsigned char* pbuf;
    unsigned char* pbuf1;
    int dst_uv_pitch = dst_width/2, dst_uv_height = dst_height/2;
    unsigned char *y_ptr = NULL, *u_ptr = NULL, *v_ptr = NULL;

    MD5 md5Y, md5U, md5V;

    OSWRAP_DEBUG("dst_width = %d, dst_height = %d, src_width = %d, src_height = %d, src_uv_width = %d, src_uv_height = %d\n", dst_width, dst_height, src_width, src_height, src_uv_width, src_uv_height);
    OSWRAP_DEBUG("tile_height = %d, tile_width = %d\n", tile_height, tile_width);

    if(md5) {
        MD5_construct(&md5Y);
        MD5_construct(&md5U);
        MD5_construct(&md5V);
    }
    if(yuv_planar_buffer != NULL){
        y_ptr = yuv_planar_buffer;
        u_ptr = yuv_planar_buffer + dst_width * dst_height;
        v_ptr = yuv_planar_buffer + (dst_width * dst_height) * 5 / 4;
    }
    pbuf = pSrcY;
    pbuf1 = pSrcY1;
    for (i=0; i<dst_height; i++) {

#if defined(FPGA_VERIFY)
        if (dst_width > 1280 && ((i & 0x7f) == 0))
        {
            OSWRAP_DEBUG("i = 0x%X\n", i);
        }
#endif

        for (j=0; j<dst_width; j++) {
            short bit8_part,bit2_part=0,out16;
            row_st = (i/tile_height)*tile_height;
            col_st = (j/tile_width)*tile_width*tile_height + (i%tile_height)*tile_width + (j%tile_width);
            bit8_part = pbuf[i*src_width + j];

#if defined(FPGA_VERIFY)
            if ((i & 0xf) == 0x0 && j == 0)
            {
                OSWRAP_DEBUG("i = %d, j = %d\n", i, j);
            }
#endif

            if(depth_y_minus8 != 0) {
                //col_st1 = (j/tile_2bit_width)*tile_2bit_width*tile_height + (i%tile_height)*tile_2bit_width + (j%tile_2bit_width);
                row_st1 = (i/tile_2bit_height)*tile_2bit_height;
                col_st1 = (j/tile_2bit_width)*tile_2bit_width*tile_2bit_height + (i%tile_2bit_height)*tile_2bit_width + (j%tile_2bit_width);
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(2*(col_st1%4)))>>(depth_y_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(2*(col_st1%4)))>>(depth_y_minus8==1?1:0));
                out16 = (bit8_part<<depth_y_minus8) | bit2_part;
                if(md5)
                    MD5_update(&md5Y, (unsigned char*)&out16, 2);
                if(pDstBuf) {
                    pDstBuf->buf_y[i*(2*dst_width)+2*j] = out16;
                    pDstBuf->buf_y[i*(2*dst_width)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5Y, (unsigned char*)&bit8_part, 1);
                if(pDstBuf)
                    pDstBuf->buf_y[i*(dst_width)+j] = bit8_part;
                if(yuv_planar_buffer != NULL)
                    *y_ptr++ = bit8_part;
            }
        }
    }
    pbuf = pSrcC;
    pbuf1 = pSrcC1;
    for (i=0; i<dst_uv_height; i++) {

#if defined(FPGA_VERIFY)
        if (dst_uv_pitch > 1280 && ((i & 0x7f) == 0))
        {
            OSWRAP_DEBUG("i = 0x%X\n", i);
        }
#endif

        for (j=0; j<(dst_uv_pitch); j++) {
            short bit8_part,bit2_part=0,out16;

#if defined(FPGA_VERIFY)
            if ((i & 0xf) == 0x0 && j == 0)
            {
                OSWRAP_DEBUG("i = %d, j = %d\n", i, j);
            }
#endif

            row_st = (i/tile_height)*tile_height;
            col_st = (j*2/tile_width)*tile_width*tile_height + (i%tile_height)*tile_width + ((j*2)%tile_width);
            bit8_part = pbuf[i*(src_width/2) + j];
            if(depth_c_minus8 != 0) {
                //col_st1 = (j*2/tile_2bit_width)*tile_2bit_width*tile_height + (i%tile_height)*tile_2bit_width + ((2*j)%tile_2bit_width);
                row_st1 = (i/tile_2bit_height)*tile_2bit_height;
                col_st1 = (j*2/tile_2bit_width)*tile_2bit_width*tile_2bit_height + (i%tile_2bit_height)*tile_2bit_width + ((2*j)%tile_2bit_width);
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                out16 = (bit8_part<<depth_c_minus8) | bit2_part;
                if(md5)
                    MD5_update(&md5U, (unsigned char*)&out16, 2);
                if(pDstBuf) {
                    pDstBuf->buf_u[i*(2*dst_uv_pitch)+2*j] = out16;
                    pDstBuf->buf_u[i*(2*dst_uv_pitch)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5U, (unsigned char*)&bit8_part, 1);
                if(pDstBuf)
                    pDstBuf->buf_u[i*(dst_uv_pitch)+j] = bit8_part;
                if(yuv_planar_buffer != NULL)
                *u_ptr++ = bit8_part;
            }
            bit8_part = pbuf[src_uv_height*src_uv_width+ i*(src_width/2) + j];
            if(depth_c_minus8 != 0) {
                //bit2_part = 0x3 & ((pbuf1[row_st*src1_width + col_st1/4]>>(2+4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                bit2_part = 0x3 & ((pbuf1[row_st1*src1_width + col_st1/4]>>(2+4*(col_st1/2%2)))>>(depth_c_minus8==1?1:0));
                out16 = (bit8_part<<depth_c_minus8) | bit2_part;
                if(md5)
                    MD5_update(&md5V, (unsigned char*)&out16, 2);
                if(pDstBuf) {
                    pDstBuf->buf_v[i*(2*dst_uv_pitch)+2*j] = out16;
                    pDstBuf->buf_v[i*(2*dst_uv_pitch)+2*j+1] = out16>>8;
                }
            } else {
                if(md5)
                    MD5_update(&md5V, (unsigned char*)&bit8_part, 1);
                if(pDstBuf)
                    pDstBuf->buf_v[i*(dst_uv_pitch)+j] = bit8_part;
                if(yuv_planar_buffer != NULL)
                *v_ptr++ = bit8_part;
            }
        }
    }
    if(md5) {
        MD5_finalize(&md5Y, md5[0]);
        MD5_finalize(&md5U, md5[1]);
        MD5_finalize(&md5V, md5[2]);
    }
    return 0;
}
#endif
