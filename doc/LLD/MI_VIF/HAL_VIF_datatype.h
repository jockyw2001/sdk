typedef enum
{
    E_HAL_VIF_MODE_BT656,
    E_HAL_VIF_MODE_DIGITAL_CAMERA,
    E_HAL_VIF_MODE_BT1120_STANDARD,
    E_HAL_VIF_MODE_BT1120_INTERLEAVED,
    E_HAL_VIF_MODE_MAX
} HalVifIntfMode_e;

typedef enum
{
    E_HAL_VIF_WORK_MODE_1MULTIPLEX,
    E_HAL_VIF_WORK_MODE_2MULTIPLEX,
    E_HAL_VIF_WORK_MODE_4MULTIPLEX,
    E_HAL_VIF_WORK_MODE_MAX
} HalVifWorkMode_e;

typedef enum
{
    E_HAL_VIF_CLK_EDGE_SINGLE_UP,
    E_HAL_VIF_CLK_EDGE_SINGLE_DOWN,
    E_HAL_VIF_CLK_EDGE_DOUBLE,
    E_HAL_VIF_CLK_EDGE_MAX
} HalVifClkEdge_e;

typedef enum
{
    /*The input sequence of the second component(only contains u and v) in BT.1120 mode */
    HAL_VIF_INPUT_DATA_VUVU = 0,
    HAL_VIF_INPUT_DATA_UVUV,

    /* The input sequence for yuv */
    HAL_VIF_INPUT_DATA_UYVY = 0,
    HAL_VIF_INPUT_DATA_VYUY,
    HAL_VIF_INPUT_DATA_YUYV,
    HAL_VIF_INPUT_DATA_YVYU,
    HAL_VIF_DATA_YUV_MAX
} HalVifDataYuvSeq_e;

typedef enum
{
    HAL_VIF_VSYNC_FIELD,
    HAL_VIF_VSYNC_PULSE
} HalVifVsync_e;

typedef enum
{
    HAL_VIF_VSYNC_NEG_HIGH,
    HAL_VIF_VSYNC_NEG_LOW
} HalVifVsyncNeg_e;

typedef enum
{
    HAL_VIF_HSYNC_VALID_SINGNAL,
    HAL_VIF_HSYNC_PULSE
} HalVifHsync_e;

typedef enum
{
    HAL_VIF_HSYNC_NEG_HIGH,
    HAL_VIF_HSYNC_NEG_LOW
} HalVifHsyncNeg_e;

typedef enum
{
    HAL_VIF_VSYNC_NORM_PULSE,
    HAL_VIF_VSYNC_VALID_SINGAL,
} HalVifVsyncValid_e;

typedef enum
{
    HAL_VIF_VSYNC_VALID_NEG_HIGH,
    HAL_VIF_VSYNC_VALID_NEG_LOW
} HalVifVsyncValidNeg_e;

typedef struct MI_VIF_TimingBlank_s
{
    u32 u32HsyncHfb;
    u32 u32HsyncAct;
    u32 u32HsyncHbb;
    u32 u32VsyncVfb;
    u32 u32VsyncVact;
    u32 u32VsyncVbb;
    u32 u32VsyncVbfb;
    u32 u32VsyncVbact;
    u32 u32VsyncVbbb;
} HalVifTimingBlank_t;

typedef struct HalVifSyncCfg_s
{
    HalVifVsync_e         eVsync;
    HalVifVsyncNeg_e      eVsyncNeg;
    HalVifHsync_e         eHsync;
    HalVifHsyncNeg_e      eHsyncNeg;
    HalVifVsyncValid_e    eVsyncValid;
    HalVifVsyncValidNeg_e eVsyncValidNeg;
    HalVifTimingBlank_t   stTimingBlank;
} HalVifSyncCfg_t;

typedef enum
{
    HAL_VI_DATA_TYPE_YUV,
    HAL_VI_DATA_TYPE_RGB,
    HAL_VI_DATA_TYPE_MAX
} HalVifDataType_e;

typedef struct HalVifDevCfg_s
{
    HalVifIntfMode_e       eIntfMode;
    HalVifWorkMode_e       eWorkMode;
    u32                    au32CompMask[2];
    HalVifClkEdge_e        eClkEdge;
    s32                    as32AdChnId[4];
    HalVifDataYuvSeq_e     eDataSeq;
    HalVifSyncCfg_t        stSynCfg;
    HalVifDataType_e       eInputDataType;
    bool                   bDataRev;
} HalVifDevCfg_t;


typedef struct HalRect_s
{
    u32 u32X;
    u32 u32Y;
    u32 u32Width;
    u32 u32Height;
} HalRect_t;

typedef enum
{
    E_HAL_VIF_CAPSEL_TOP,
    E_HAL_VIF_CAPSEL_BOTTOM,
    E_HAL_VIF_CAPSEL_BOTH,
    E_HAL_VIF_CAPSEL_MAX
} HalVifCapsel_e;

typedef enum
{
    E_HAL_VIF_SCAN_INTERLACED,
    E_HAL_VIF_SCAN_PROGRESSIVE,
    E_HAL_VIF_SCAN_MAX
} HalVifScanMode_e;

typedef enum
{
    E_HAL_VIF_PIXEL_FORMAT_RGB_1BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_2BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_4BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_8BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_444,

    E_HAL_VIF_PIXEL_FORMAT_RGB_4444,
    E_HAL_VIF_PIXEL_FORMAT_RGB_555,
    E_HAL_VIF_PIXEL_FORMAT_RGB_565,
    E_HAL_VIF_PIXEL_FORMAT_RGB_1555,

    /*  9 reserved */
    E_HAL_VIF_PIXEL_FORMAT_RGB_888,
    E_HAL_VIF_PIXEL_FORMAT_RGB_8888,

    E_HAL_VIF_PIXEL_FORMAT_RGB_PLANAR_888,
    E_HAL_VIF_PIXEL_FORMAT_RGB_BAYER_8BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_BAYER_10BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_BAYER_12BPP,
    E_HAL_VIF_PIXEL_FORMAT_RGB_BAYER_14BPP,

    E_HAL_VIF_PIXEL_FORMAT_RGB_BAYER,         /* 16 bpp */

    E_HAL_VIF_PIXEL_FORMAT_YUV_A422,
    E_HAL_VIF_PIXEL_FORMAT_YUV_A444,

    E_HAL_VIF_PIXEL_FORMAT_YUV_PLANAR_422,
    E_HAL_VIF_PIXEL_FORMAT_YUV_PLANAR_420,

    E_HAL_VIF_PIXEL_FORMAT_YUV_PLANAR_444,

    E_HAL_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    E_HAL_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    E_HAL_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    E_HAL_VIF_PIXEL_FORMAT_UYVY_PACKAGE_422,
    E_HAL_VIF_PIXEL_FORMAT_YUYV_PACKAGE_422,
    E_HAL_VIF_PIXEL_FORMAT_VYUY_PACKAGE_422,
    E_HAL_VIF_PIXEL_FORMAT_YCbCr_PLANAR,

    E_HAL_VIF_PIXEL_FORMAT_YUV_400,

    E_HAL_VIF_PIXEL_FORMAT_MAX
} HalVifPixelFormat_e;



typedef struct HalVifChnCfg_s
{
    HalRect_t             stCapRect;
    HalVifCapsel_e        eCapSel;
    HalVifScanMode_e      eScanMode;
    HalVifPixelFormat_e   ePixFormat;
    MI_U32                u32FrameRate;
} HalVifChnCfg_t;

typedef struct HalVifChnCfg_s
{
    HalRect_t             stDestSize;
    u32                   u32FrameRate;
} HalVifSubChnCfg_t;

typedef struct HalVifChnStat_s
{
    bool bEnable;
    u32  u32IntCnt;
    u32  u32FrmRate;
    u32  u32LostInt;
    u32  u32VbFail;
    u32  u32PicWidth;
    u32  u32PicHeight;
} HalVifChnStat_t;


/*! @brief VIF API error code*/
typedef enum
{
    E_HAL_VIF_SUCCESS,   /**< operation successful */
    E_HAL_VIF_ERROR,     /**< unspecified failure */
} HalVifErr_e;


/*! @brief ring buffer element*/
typedef struct
{
    U32 nPhyBufAddr; /**< physical buffer address*/
    U16 nCropX;      /**< crop start x*/
    U16 nCropY;      /**< crop start y*/
    U16 nCropW;      /**< crop start width*/
    U16 nCropH;      /**< crop start height*/
    U32 nPTS;        /**< timestamp in 90KHz*/
#define VIF_BUF_EMPTY   0 /**< Vif buffer is ready to write*/
#define VIF_BUF_FILLING 1 /**< DMA occupied */
#define VIF_BUF_READY   2 /**< Image ready */
#define VIF_BUF_INVALID 3 /**< Do not use */
    U8 nStatus;      /**< Vif buffer status*/
} VifRingBufElm_t;

