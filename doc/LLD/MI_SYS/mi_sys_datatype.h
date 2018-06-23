#ifndef _MI_SYS_DATATYPE_H_
#define _MI_SYS_DATATYPE_H_

#define MI_SYS_MAX_INPUT_PORT_CNT 64
#define MI_SYS_MAX_OUTPUT_PORT_CNT 8
#define MI_SYS_MAX_CHN_CNT 128
#define MI_SYS_INVALID_PTS 0xffffffffffffffffULL

#define MI_VB_BLK_HANDLE_INVALID   (-1)
#define MI_VB_POOL_HANDLE_INVALID  (-1)
#define MI_VB_POOL_LIST_MAX_CNT    (64)
#define MI_MAX_MMA_HEAP_LENGTH     (64)

typedef MI_S32 MI_VBPOOL_HANDLE;
typedef MI_S32 MI_VBBLK_HANDLE;

typedef  struct MI_VB_BufBlkInfo_s
{
  MI_VBPOOL_HANDLE poolHandle;
  MI_U32 u32OffsetInVBPool;
  MI_U32 u32BlkSize;
  MI_PHY phySicalAddr;
  void *pVirtualAddress;
} MI_VB_BufBlkInfo_t;

typedef struct MI_VB_PoolConf_s
{
   MI_U32 u32BlkSize;
   MI_U32 u32BlkCnt;
   MI_U8  u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
}MI_VB_PoolConf_t;

typedef struct MI_VB_PoolListConf_s
{
     MI_U32 u32PoolListCnt;
     MI_VB_PoolConf_t stPoolConf[MI_VB_POOL_LIST_MAX_CNT];
} MI_VB_PoolListConf_t;

typedef enum
{
    E_MI_SYS_MODULE_ID_VDEC = 0,
    E_MI_SYS_MODULE_ID_VENC,
    E_MI_SYS_MODULE_ID_DISP,
    E_MI_SYS_MODULE_ID_VIF,
    E_MI_SYS_MODULE_ID_AI,
    E_MI_SYS_MODULE_ID_AO,
    E_MI_SYS_MODULE_ID_RGN,
    E_MI_SYS_MODULE_ID_VPE,
    E_MI_SYS_MODULE_ID_DIVP,
    E_MI_SYS_MODULE_ID_GFX,
    E_MI_SYS_MODULE_ID_IVE,
    E_MI_SYS_MODULE_ID_IAE,
    E_MI_SYS_MODULE_ID_MD,
    E_MI_SYS_MODULE_ID_OD,
    E_MI_SYS_MODULE_ID_VDF,
    E_MI_SYS_MODULE_ID_VDISP,
    E_MI_SYS_MODULE_ID_MAX,
} MI_SYS_MODULE_ID_e;


typedef enum
{

    E_MI_SYS_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MI_SYS_PIXEL_FRAME_ARGB8888,
    E_MI_SYS_PIXEL_FRAME_ABGR8888,

    E_MI_SYS_PIXEL_FRAME_RGB565,
    E_MI_SYS_PIXEL_FRAME_ARGB1555,
    E_MI_SYS_PIXEL_FRAME_I2,
    E_MI_SYS_PIXEL_FRAME_I4,
    E_MI_SYS_PIXEL_FRAME_I8,
    
    E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    E_MI_SYS_PIXEL_FRAME_YUV_MST_420,
    
    
    //vdec mstar private video format
    E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE1_H264,
    E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE2_H265,
    E_MI_SYS_PIXEL_FRAME_YC420_MSTTILE3_H265,
    E_MI_SYS_PIXEL_FRAME_FORMAT_MAX,
} MI_SYS_PixelFormat_e;

typedef enum
{
    E_MI_SYS_COMPRESS_MODE_NONE,//no compress
    E_MI_SYS_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_MI_SYS_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_MI_SYS_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_MI_SYS_COMPRESS_MODE_BUTT, //number
}MI_SYS_CompressMode_e;


typedef enum
{
    E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE = 0x0,  // progessive.
    E_MI_SYS_FRAME_SCAN_MODE_INTERLACE   = 0x1,  // interlace.
    E_MI_SYS_FRAME_SCAN_MODE_MAX,
} MI_SYS_FrameScanMode_e;


typedef enum
{
    E_MI_SYS_FRAME_TILE_MODE_NONE = 0,
    E_MI_SYS_FRAME_TILE_MODE_16x16,      // tile mode 16x16
    E_MI_SYS_FRAME_TILE_MODE_16x32,      // tile mode 16x32
    E_MI_SYS_FRAME_TILE_MODE_32x16,      // tile mode 32x16
    E_MI_SYS_FRAME_TILE_MODE_32x32,      // tile mode 32x32
    E_MI_SYS_FRAME_TILE_MODE_MAX
} MI_SYS_FrameTileMode_e;

typedef enum
{
    E_MI_SYS_FIELDTYPE_NONE,        //< no field.
    E_MI_SYS_FIELDTYPE_TOP,           //< Top field only.
    E_MI_SYS_FIELDTYPE_BOTTOM,    //< Bottom field only.
    E_MI_SYS_FIELDTYPE_BOTH,        //< Both fields.
    E_MI_SYS_FIELDTYPE_NUM
} MI_SYS_FieldType_e;


typedef enum
{
    E_MI_SYS_BUFALLOC_MODE_VBPOOL,
    E_MI_SYS_BUFALLOC_MODE_KMALLOC,
} MI_SYS_BufAllocMode_e;


typedef enum
{
    E_MI_SYS_BUFDATA_RAW = 0,
    E_MI_SYS_BUFDATA_FRAME,
    E_MI_SYS_BUFDATA_META,
} MI_SYS_BufDataType_e;

typedef enum
{
    E_MI_SYS_ROTATE_NONE, //Rotate 0 degrees
    E_MI_SYS_ROTATE_90, //Rotate 90 degrees
    E_MI_SYS_ROTATE_180, //Rotate 180 degrees
    E_MI_SYS_ROTATE_270, //Rotate 270 degrees
    E_MI_SYS_ROTATE_NUM,
}MI_SYS_Rotate_e;


typedef struct MI_SYS_ChnPort_s
{
    MI_SYS_MODULE_ID_e  eModId;
    MI_S32 s32DevId;
    MI_S32 s32ChnId;
    MI_S32 s32PortId;
}MI_SYS_ChnPort_t;

typedef struct MI_SYS_WindowRect_s
{
    MI_U16 u16X;
    MI_U16 u16Y;
    MI_U16 u16Width;
    MI_U16 u16Height;
}MI_SYS_WindowRect_t;



typedef struct MI_SYS_RawData_s
{
    void* pVirAddr;            /* 码流地址 */
    MI_PHY   u64PhyAddr;    /* 物理地址 */
    MI_U32 u32BufSize;         /*当前分配 的buf 大小*/
    
    MI_U32 u32ContenttSize;            /* 实际使用的大小 */
    MI_BOOL bEndOfFrame;    /* 当前帧是否结束 */

} MI_SYS_RawData_t;

typedef struct MI_SYS_MetaData_s
{
    void* pVirAddr;            /* 码流地址 */
    MI_PHY   u64PhyAddr;    /* 物理地址 */

    MI_U32 u32Size;
    MI_U32 u32ExtraData;    /*driver special flag*/
    MI_SYS_MODULE_ID_e eDataFromModule;
} MI_SYS_MetaData_t;

typedef  struct  MI_SYS_FrameData_s
{
    MI_SYS_FrameTileMode_e eTileMode;
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_SYS_FrameScanMode_e eFrameScanMode;
    MI_SYS_FieldType_e eFieldType;
    MI_SYS_WindowRect_t stWindowRect;
  
    MI_U16 u16Width;
    MI_U16 u16Height;

    void* pVirAddr[3];
    MI_PHY u64PhyAddr[3];
    MI_U32 u32Stride[3];

} MI_SYS_FrameData_t;


typedef  struct  MI_SYS_BufInfo_s
{
    MI_SYS_BufDataType_e eBufType;
    MI_U64 u64Pts;
    MI_BOOL bEndOfStream;    /* 是否发完所有码流 */

     MI_BOOL bUsrBuf;
    union
    {
        MI_SYS_RawData_t stRawData;
        MI_SYS_FrameData_t stFrameData;
        MI_SYS_MetaData_t stMetaData;
    };
} MI_SYS_BufInfo_t;


typedef struct MI_SYS_BufFrameConfig_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
    MI_SYS_FrameScanMode_e eFrameScanMode;
    MI_SYS_PixelFormat_e eFormat;
}MI_SYS_BufFrameConfig_t;

typedef struct MI_SYS_BufRawConfig_s
{
    MI_U32 u32Size;
}MI_SYS_BufRawConfig_t;

typedef struct MI_SYS_MetaDataConfig_s
{
    MI_SYS_BufAllocMode_e eBufAllocMode;

    MI_U32 u32Size;
}MI_SYS_MetaDataConfig_t;


typedef struct
{
   MI_SYS_BufDataType_e eBufType;
   ///FIFO = MI_SYS_INVALID_PTS
   MI_U64 u64TargetPts;
   union
   {
       MI_SYS_BufFrameConfig_t stFrameCfg;
       MI_SYS_BufRawConfig_t stRawCfg;
       MI_SYS_MetaDataConfig_t stMetaCfg;
   };
}MI_SYS_BufConf_t;


typedef struct MI_SYS_Version_S
{
    MI_U8 u8Version[64];
}MI_SYS_Version_t;

#endif ///_MI_SYS_DATATYPE_H_