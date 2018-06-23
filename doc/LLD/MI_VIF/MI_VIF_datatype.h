#ifndef _MI_COMMON_VIF_H_
#define _MI_COMMON_VIF_H_


#define MI_VIF_MAX_DEV_NUM   4
#define MI_VIF_MAX_WAY_NUM_PER_DEV   4
#define MI_VIF_MAX_PHYCHN_NUM (MI_VIF_MAX_DEV_NUM*MI_VIF_MAX_WAY_NUM_PER_DEV)


/* interface mode of video input */
/*
BT 601: 16位数据传输；21芯；Y、U、V信号同时传输。并行数据，行场同步有单独输出;
BT 656:  8位数据传输； 9芯，不需要同步信号；串行视频传输；传输速率是601的2倍；
         先传Y，后传UV。656输出的是串行数据，行场同步信号嵌入在数据流中

656只是数据传输接口而已，可以说是作为601的一个传输方式。
简单的说ITU-R BT.601是"演播室数字电视编码参数"标准,而ITU-R BT.656 则是ITU-R BT.601附件A中的数字接口标准,
用于主要数字视频设备(包括芯 片)之间采用27Mhz/s并口或243Mb/s串行接口的数字传输接口标准.

601号建议

    它规定了取样频率与取样结构。
        例如：在4：2：2等级的编码中，规定亮度信号和色差信号的取样频率分别为13.5MHZ和6.75MHZ   ,
        取样结构为正交结构,即按行、场、帧重复,每行中的R-Y和B-Y取样与奇次(1,3,5……)Y的取样同位置，
        即取样结构是固定的，取样点在电视屏幕上的相对位置不变。
    它规定了编码方式。
        对亮度信号和两个色差信号进行线性PCM编码，每个取样点取8比特量化。
        同时，规定在数字编码时，不使用A/D转换的整个动态范围，只给亮度信号分配220个量化级，        黑电平对应于量化级16，白电平对应于量化级235。
        为每个色差信号分配224个量化级，色差信号的零电平对应于量化级128。

以4：2：2编码标准为例，其比特流为：13.5×8+6.75×8×2 = 216Mb/S。
若采用4：4：4编码方式，即对复合信号直接编码，其抽样频率取为13.3×8=106.4 Mb/S
*/

typedef enum
{
    /*
    输入数据的协议符合标准BT.656 协议，端口    数据输入模式为亮度色度复合模式，分量模    式为单分量。
    */
    E_MI_VIF_MODE_BT656,
    /*
    如果是Sensor，则为VI_MODE_DIGITAL_CAMERA    输入数据的协议为Digital camera 协议，
    端口  数据输入模式为亮度色度复合模式，分量模    式为单分量
    */
    E_MI_VIF_MODE_DIGITAL_CAMERA,
    /*
    BT.1120 协议（BT.656+双分量），端口数据输入模式为亮度色度分离模式，分量模式为双分量。
    */
    E_MI_VIF_MODE_BT1120_STANDARD,
    /*
    BT.1120 interleave 模式，端口数据输入模式为亮度色度分离模式，分量模式为双分量。
    */
    E_MI_VIF_MODE_BT1120_INTERLEAVED,
    E_MI_VIF_MODE_MAX
} MI_VIF_IntfMode_e;

typedef enum
{
    /*
    1 路复合工作模式。
    */
    E_MI_VIF_WORK_MODE_1MULTIPLEX,
    /*
    2 路复合工作模式，输入数据的协议必须为标准 BT.656 协议。
    */
    E_MI_VIF_WORK_MODE_2MULTIPLEX,
    /*
    4 路复合工作模式，输入数据的协议必须为标准 BT.656 协议。
    */
    E_MI_VIF_WORK_MODE_4MULTIPLEX,
    VIF_WORK_MODE_MAX
} MI_VIF_WorkMode_e;

typedef enum
{
    E_MI_VIF_SCAN_INTERLACED,   /*隔行*/
    E_MI_VIF_SCAN_PROGRESSIVE,  /*逐行*/
    E_MI_VIF_SCAN_MAX
} MI_VIF_ScanMode_e;

typedef enum
{
    E_MI_VIF_FRAMERATE_FULL,
    E_MI_VIF_FRAMERATE_HALF,
    E_MI_VIF_FRAMERATE_QUARTER,
    E_MI_VIF_FRAMERATE_OCTANT,
    E_MI_VIF_FRAMERATE_THREE_QUARTERS,
    E_MI_VIF_FRAMERATE_MAX
} MI_VIF_FrameRate_e;

/*
输入数据顺序（仅支持 YUV 格式）

VI_INPUT_DATA_VUVU  YUV 数据通过分离模式输入时，C 分量的输入排列顺序为 VUVU。
VI_INPUT_DATA_UVUV  YUV 数据通过分离模式输入时，C 分量的输入排列顺序为 UVUV。
VI_INPUT_DATA_UYVY  YUV 数据通过复合模式输入时，顺序为UYVY。
VI_INPUT_DATA_VYUY  YUV 数据通过复合模式输入时，顺序为VYUY。
VI_INPUT_DATA_YUYV  YUV 数据通过复合模式输入时，顺序为YUYV。
VI_INPUT_DATA_YVYU  YUV 数据通过复合模式输入时，顺序为YVYU。
*/
typedef enum
{
    /*The input sequence of the second component(only contains u and v) in BT.1120 mode */
    E_MI_VIF_INPUT_DATA_VUVU = 0,
    E_MI_VIF_INPUT_DATA_UVUV,

    /* The input sequence for yuv */
    E_MI_VIF_INPUT_DATA_UYVY = 0,
    E_MI_VIF_INPUT_DATA_VYUY,
    E_MI_VIF_INPUT_DATA_YUYV,
    E_MI_VIF_INPUT_DATA_YVYU,
    E_MI_VIF_DATA_YUV_MAX
} MI_VIF_DataYuvSeq_e;


typedef enum
{
    // 时钟单沿模式，且VI 设备在上升沿采样
    E_MI_VIF_CLK_EDGE_SINGLE_UP,
    // 时钟单沿模式，且VI 设备在下降沿采样
    E_MI_VIF_CLK_EDGE_SINGLE_DOWN,
    // 前端送过来双沿数据时，VI 进行双沿采样
    E_MI_VIF_CLK_EDGE_DOUBLE,
    E_MI_VIF_CLK_EDGE_MAX
} MI_VIF_ClkEdge_e;

/*
此枚举需要与  VI_DEV_ATTR_EX_S 中的分量掩码    au32CompMask[2]配合设置

视频输入设备Dev0 有16 根数据线，用于与外部视频源对接，
对接时可能仅使用16bit 数据线中的一部分（由硬件连线决定）。
例如Dev0 输入8bit 的YUV 数据，camera 与VI 对接的是16bit 数据线的前8bit 数据线（即bit0~bit7 对应的数据线），
此时配置输入数据为单分量，分量掩码为0xFF000000

如果视频数据是双分量输入，则需要将设备属性配为双分量，并根据管脚的实际连接情况指定每个分量的分量掩码。
例如Dev0 设为BT.1120、双分量输入（亮度分量和色度分量各占用8bit），则分量掩码配置为0xFF000000 和0xFF0000。
*/

typedef enum
{
    // 输入数据为单分量
    E_MI_VIF_COMP_MODE_SINGLE,
    // 输入数据为双分量
    E_MI_VIF_COMP_MODE_DOUBLE,
    E_MI_VIF_COMP_MODE_MAX
} MI_VIF_CompMode_e;

/*
如果Y 与C 分量是通过同一组数据线输入，则为E_MI_VIF_COMBINE_COMPOSITE，如BT.656；
如果Y 与C 分量是通过不同的数据线输入，则为E_MI_VIF_COMBINE_SEPARATE  ，如BT.1120。
*/
typedef enum
{
    // 复合模式
    E_MI_VIF_COMBINE_COMPOSITE,
    // 分离模式
    E_MI_VIF_COMBINE_SEPARATE,
    E_MI_VIF_COMBINE_MAX
} MI_VIF_ConbineMode_e;

typedef enum
{
    /*
        垂直同步翻转模式，即翻转一次表示一场。BT601 模式下表示    场号，DC 模式下表示行有效信号
    */
    E_MI_VIF_VSYNC_FIELD,
    /*
        垂直同步脉冲模式，即一个脉冲到来表示新的一帧或一场
    */
    E_MI_VIF_VSYNC_PULSE
} MI_VIF_Vsync_e;

typedef enum
{
    E_MI_VIF_VSYNC_NEG_HIGH,
    E_MI_VIF_VSYNC_NEG_LOW
} MI_VIF_VsyncNeg_e;

typedef enum
{
    // 数据有效信号。
    E_MI_VIF_HSYNC_VALID_SINGNAL,
    // 水平同步脉冲信号，即一个脉冲到来表示新一行的开始。
    E_MI_VIF_HSYNC_PULSE
} MI_VIF_Hsync_e;

typedef enum
{
    //高电平表示数据有效；
    E_MI_VIF_HSYNC_NEG_HIGH,
    E_MI_VIF_HSYNC_NEG_LOW
} MI_VIF_HsyncNeg_e;

typedef enum
{
    /* DC 模式下使用，表示垂直有效同步标识 */
    E_MI_VIF_VSYNC_NORM_PULSE,
    /* DC 模式下使用，表示垂直同步时序行有效信号 */
    E_MI_VIF_VSYNC_VALID_SINGAL,
} MI_VIF_VsyncValid_e;

typedef enum
{
    E_MI_VIF_VSYNC_VALID_NEG_HIGH,
    E_MI_VIF_VSYNC_VALID_NEG_LOW
} MI_VIF_VsyncValidNeg_e;

typedef struct MI_VIF_TimingBlank_s
{
    // 水平前消隐区宽度
    MI_U32 u32HsyncHfb;
    // 水平有效宽度
    MI_U32 u32HsyncAct;
    // 水平后消隐区宽度
    MI_U32 u32HsyncHbb;
    // 帧图像或隔行输入时奇场图像的垂直前消隐区高度
    MI_U32 u32VsyncVfb;
    // 帧图像或隔行输入时奇场垂直有效高度
    MI_U32 u32VsyncVact;
    // 帧图像或隔行输入时奇场垂直后消隐区高度
    MI_U32 u32VsyncVbb;
    // 隔行输入时偶场垂直前消隐区高度（帧输入时无效）
    MI_U32 u32VsyncVbfb;
    // 隔行输入时偶场垂直有效高度（帧输入时无效）
    MI_U32 u32VsyncVbact;
    // 隔行输入时偶场垂直后消隐区高度（帧输入时无效）
    MI_U32 u32VsyncVbbb;
} MI_VIF_TimingBlank_t;

/* synchronization information about the BT.601 or DC timing */
/*
BT.601 模式下，垂直同步信号只需配置 enVsync 和 enVsyncNeg 即可，
它们分别表示管脚 VI_P_VSYNC_FIELD 的垂直同步信号类型和垂直同步信号的极性，两者决定了采用何种垂直同步时序。

enVsync 配置为 VI_VSYNC_FIELD，BT.601 模式下表示场号。
此时，如果enVsyncNeg 配置为 VI_VSYNC_NEG_HIGH，则高电平表示偶场，低电平表示奇场；
如果 enVsyncNeg 配置为 VI_VSYNC_NEG_LOW，则低电平表示偶场， 高电平表示奇场。

enVsync 配置为 VI_VSYNC_PULSE，表示垂直同步脉冲，即一个脉冲到来表示新的一场或一帧。
此时，如果 enVsyncNeg 配置为 VI_VSYNC_NEG_HIGH， 则正脉冲表示垂直同步脉冲；
如果 enVsyncNeg 配置为VI_VSYNC_NEG_LOW，则负脉冲表示垂直同步脉冲。

DC 模式下，垂直同步信号需要配置 enVsync、enVsyncValid 和enVsyncValidNeg 三项，
它们分别表示管脚 VI_P_VSYNC_FIELD 的垂直同步信号类型、垂直同步信号标识和垂直有效信号极性。
enVsyncValidNeg 只有在 enVsyncValid 配为VI_VSYNC_VALID_SINGAL 时有效。

enVsync 配置为 VI_VSYNC_PULSE，表示垂直时序脉冲方式。如果在垂直消隐区内 hsync 信号仍有效，
则 enVsyncValid 需配置为VI_VSYNC_VALID_SINGAL 选择垂直有效标识，
同时 enVsyncValidNeg 需要配置为 VI_VSYNC_VALID_NEG_HIGH（或VI_VSYNC_VALID_NEG_LOW），
以表示高（低）电平有效，其他情况不需关注此属性。
如果输入时序为垂直时序行有效方式，只需将 enVsync 配置为VI_VSYNC_FIELD 即可。

此时，如果 enHsyncNeg 配置为VI_HSYNC_NEG_HIGH，则正脉冲表示水平同步脉冲；
如果 enHsyncNeg 配置为 VI_HSYNC_NEG_LOW，则负脉冲表示水平同步脉冲。
*/
/*
    同步时序配置，BT.601 和 DC 模式时必须配置，其它模式时无效

    VI_VSYNC_E             enVsync;  定义视频设备输入数据的垂直同步信号类型
        VI_VSYNC_FIELD
            垂直同步翻转模式，即翻转一次表示一场。BT601 模式下表示场号，DC 模式下表示行有效信号
        VI_VSYNC_PULSE
            垂直同步脉冲模式，即一个脉冲到来表示新的一帧或一场

    VI_VSYNC_NEG_E         enVsyncNeg;
        VI_VSYNC_NEG_HIGH
            若 VI_VSYNC_E = VI_VSYNC_FIELD，则表示偶数场的vsync 信号为高电平
            若 VI_VSYNC_E = VI_VSYNC_PULSE，则表示正脉冲表示 vsync 同步脉冲
        VI_VSYNC_NEG_LOW
            若 VI_VSYNC_E = VI_VSYNC_FIELD，则表示偶数场的vsync 信号为低电平
            若 VI_VSYNC_E = VI_VSYNC_PULSE，则表示负脉冲表示 vsync 同步脉冲

    VI_HSYNC_E             enHsync;  定义视频设备输入数据的水平同步信号类型
        VI_HSYNC_VALID_SINGNAL
            水平同步数据有效信号
        VI_HSYNC_PULSE
            水平同步脉冲信号

    VI_HSYNC_NEG_E         enHsyncNeg;  定义视频设备输入数据水平同步信号的极性
        VI_HSYNC_NEG_HIGH
            若 VI_HSYNC_E = VI_HSYNC_VALID_SINGNAL，则高电平表示有效数据；
            若 VI_HSYNC_E = VI_HSYNC_PULSE，则正脉冲表示同步脉冲。
        VI_HSYNC_NEG_LOW
            若 VI_HSYNC_E = VI_HSYNC_VALID_SINGNAL，则低电平表示有效数据；
            若 VI_HSYNC_E = VI_HSYNC_PULSE，则负脉冲表示同步脉冲。

    // 不支持，删除
    VI_VSYNC_VALID_E       enVsyncValid;  定义视频设备输入数据的垂直有效同步信号类型
        VI_VSYNC_NORM_PULSE            DC 模式下使用，表示垂直有效同步标识
        VI_VSYNC_VALID_SINGAL          DC 模式下使用，表示垂直同步时序行有效信号

    // 不支持，删除
    VI_VSYNC_VALID_NEG_E   enVsyncValidNeg; 定义视频设备输入数据水平同步信号的极性,只有在 VI_VSYNC_VALID_E = VI_VSYNC_NORM_SINGAL 时，该项才有效
        VI_VSYNC_VALID_NEG_HIGH
            若 VI_VSYNC_VALID_E =  VI_VSYNC_NORM_SINGAL，则表示高电平为有效信号。
        VI_VSYNC_VALID_NEG_LOW
            若 VI_VSYNC_VALID_E =  VI_VSYNC_NORM_SINGAL，则表示低电平为有效信号。

    // 不支持，删除
    VI_TIMING_BLANK_S      stTimingBlank;  视频设备输入时序的消隐信息
        HI_U32 u32HsyncHfb     水平前消隐区宽度
        HI_U32 u32HsyncAct     水平有效宽度
        HI_U32 u32HsyncHbb     水平后消隐区宽度
        HI_U32 u32VsyncVfb     帧图像或隔行输入时奇场图像的垂直前消隐区高度
        HI_U32 u32VsyncVact    帧图像或隔行输入时奇场垂直有效高度
        HI_U32 u32VsyncVbb     帧图像或隔行输入时奇场垂直后消隐区高度
        HI_U32 u32VsyncVbfb    隔行输入时偶场垂直前消隐区高度（帧输入时无效）
        HI_U32 u32VsyncVbact   隔行输入时偶场垂直有效高度（帧输入时无效）
        HI_U32 u32VsyncVbbb    隔行输入时偶场垂直后消隐区高度（帧输入时无效）
    */

typedef struct MI_VIF_SyncCfg_s
{
    /* 垂直同步信号类型 */
    MI_VIF_Vsync_e  eVsync;
    /* 垂直同步信号的极性 */
    MI_VIF_VsyncNeg_e   eVsyncNeg;
    /* 管脚 VI_P_HSYNC_VD 的水平同步信号类型 */
    MI_VIF_Hsync_e  eHsync;
    /* 管脚 VI_P_HSYNC_VD 的水平同步信号的极性 */
    MI_VIF_HsyncNeg_e   eHsyncNeg;
    /* 垂直有效同步信号类型 */
    MI_VIF_VsyncValid_e  eVsyncValid;
    /* 垂直有效同步信号的极性 */
    MI_VIF_VsyncValidNeg_e eVsyncValidNeg;
    /* 输入时序的消隐信息 */
    MI_VIF_TimingBlank_t  stTimingBlank;
} MI_VIF_SyncCfg_t;

typedef enum
{
    // 输入数据类型为YUV，VI 前端一般接的是AD
    E_MI_VI_DATA_TYPE_YUV,
    // 输入数据类型为RGB，VI 前端一般接的是Sensor
    E_MI_VI_DATA_TYPE_RGB,
    E_MI_VI_DATA_TYPE_MAX
} MI_VIF_DataType_e;

typedef struct MI_VIF_DevAttr_s
{
    MI_VIF_IntfMode_e       eIntfMode;
    MI_VIF_WorkMode_e       eWorkMode;

    /*
    分量掩码配置。
    当 enInputMode = VI_INPUT_MODE_BT656 且 enCompMode =VI_COMP_MODE_DOUBLE，
    enCombineMode =VI_COMBINE_SEPARATE 时，
    形成VI_DEV_ATTR_S 中的时序，此时需要配置 Y 和 C 的分量掩码，其他模式时只需配置单分量掩码。
    */
    /*
    stDevAttr.au32CompMask[0] = 0xFF000000;
    stDevAttr.au32CompMask[1] = 0x00FF0000;

    分量掩码配置。
    当enIntfMode = VI_MODE_BT1120_STANDARD 时，需要配置Y    和C 的分量掩码，其他模式时只需配置单分量掩码。

    分量掩码配置。
    当enInputMode = VI_INPUT_MODE_BT656 且enCompMode = VI_COMP_MODE_DOUBLE，enCombineMode =VI_COMBINE_SEPARATE 时，
    形成VI_DEV_ATTR_S 中的时序，此时需要配置Y 和C 的分量掩码，其他模式时只需配置单分量掩码。
    */
    MI_U32                  au32CompMask[2];
    MI_VIF_ClkEdge_e        eClkEdge;

    /*
    分量掩码配置。
    当 enInputMode = VI_INPUT_MODE_BT656 且 enCompMode =VI_COMP_MODE_DOUBLE，
    enCombineMode =VI_COMBINE_SEPARATE 时，
    形成VI_DEV_ATTR_S 中的时序，此时需要配置 Y 和 C 的分量掩码，其他模式时只需配置单分量掩码。
    */
    MI_S32                  as32AdChnId[4];
    /* DC模式时以下必配，其它模式时无效*/
    MI_VIF_DataYuvSeq_e     eDataSeq;
    MI_VIF_SyncCfg_t        stSynCfg;
    /*
    输入数据类型，Sensor 输入一般为 RGB，AD 输入一般为 YUV
    */
    MI_VIF_DataType_e       eInputDataType;
    /*
    因为走线约束等硬件原因，有可能出现 AD/Sensor 的数据线与数据线连接数据高低位反接，
    比如，AD_DATA0 与 VIU_DATA7 连接，AD_DATA7 与 VIU_DATA0 连接，以此类推。
    当 AD/Sensor 管脚与 VI 管脚正向连接时，取 bDataRev= HI_FALSE；当反向连接时，取 bDataRev= HI_TRUE
    */
    MI_BOOL                 bDataRev;
} MI_VIF_DevAttr_t;

typedef struct MI_VIF_BindAttr_s
{
    MI_U32 u32VifDev;
    MI_U32 u32VifWay;
} MI_VIF_BindAttr_t;

// 视频输入捕获图像的帧场选择
typedef enum
{
    // 选择顶场
    E_MI_VIF_CAPSEL_TOP,
    // 选择底场（若采集单场时推荐使用）
    E_MI_VIF_CAPSEL_BOTTOM,
    // 选择顶底两场
    E_MI_VIF_CAPSEL_BOTH,
    E_MI_VIF_CAPSEL_MAX
} MI_VIF_Capsel_e;

typedef enum
{
    E_MI_VIF_PIXEL_FORMAT_RGB_1BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_2BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_4BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_8BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_444,

    E_MI_VIF_PIXEL_FORMAT_RGB_4444,
    E_MI_VIF_PIXEL_FORMAT_RGB_555,
    E_MI_VIF_PIXEL_FORMAT_RGB_565,
    E_MI_VIF_PIXEL_FORMAT_RGB_1555,

    /*  9 reserved */
    E_MI_VIF_PIXEL_FORMAT_RGB_888,
    E_MI_VIF_PIXEL_FORMAT_RGB_8888,

    E_MI_VIF_PIXEL_FORMAT_RGB_PLANAR_888,
    E_MI_VIF_PIXEL_FORMAT_RGB_BAYER_8BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_BAYER_10BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_BAYER_12BPP,
    E_MI_VIF_PIXEL_FORMAT_RGB_BAYER_14BPP,

    E_MI_VIF_PIXEL_FORMAT_RGB_BAYER,         /* 16 bpp */

    E_MI_VIF_PIXEL_FORMAT_YUV_A422,
    E_MI_VIF_PIXEL_FORMAT_YUV_A444,

    E_MI_VIF_PIXEL_FORMAT_YUV_PLANAR_422,
    E_MI_VIF_PIXEL_FORMAT_YUV_PLANAR_420,

    E_MI_VIF_PIXEL_FORMAT_YUV_PLANAR_444,

    E_MI_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    E_MI_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    E_MI_VIF_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    E_MI_VIF_PIXEL_FORMAT_UYVY_PACKAGE_422,
    E_MI_VIF_PIXEL_FORMAT_YUYV_PACKAGE_422,
    E_MI_VIF_PIXEL_FORMAT_VYUY_PACKAGE_422,
    E_MI_VIF_PIXEL_FORMAT_YCbCr_PLANAR,

    E_MI_VIF_PIXEL_FORMAT_YUV_400,

    E_MI_VIF_PIXEL_FORMAT_MAX
} MI_VIF_PixelFormat_e;


typedef struct MI_Rect_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_Rect_t;

typedef struct MI_Size_s
{
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_Size_t;

typedef struct MI_VIF_ChnPortAttr_s
{
    /*
    采集区域起始坐标用于配置需要采集的矩形图像相对于原始图像起始点的位置。
    stCapRect 的宽与高为静态属性， 其他项为动态属性；无次属性
    stCapRect 是针对原始图像进行裁剪；
    stCapRect 中 s32X 和u32Width 必须 2 对齐；
                s32Y 和u32Height 逐行采集时必须 2 对齐
                s32Y 和u32Height 隔行采集时必须 4 对齐
    */
    MI_Rect_t              stCapRect;

    // stDestSize 的等于stCapRect,或者宽高都减少1/2
    MI_Size_t              stDestSize;

    /*
    抽场选择用于在原始图像为隔行输入时，用户可选择仅捕获其中的一场
    为防止图像上下抖动现象，仅支持捕获两场或底场
    */
    MI_VIF_Capsel_e        eCapSel;

    // 扫描模式用于在原始图像为隔行/逐行输入
    MI_VIF_ScanMode_e      eScanMode;

    // 输出格式 enPixFormat 可以动态改变,       支持 semi-planar4:2:2 和 semi-planar4:2:0
    MI_VIF_PixelFormat_e   ePixFormat;

    //不支持
    MI_BOOL                bMirror;

    //不支持
    MI_BOOL                bFlip;

    MI_U32              u32FrameRate;
} MI_VIF_ChnPortAttr_t;

typedef struct MI_VIF_ChnPortStat_s
{
    // 通道是否使能
    MI_BOOL bEnable;
    // 中断计数
    // can not support ?
    MI_U32  u32IntCnt;
    // 每10 秒的平均帧率，该值不一定精确
    MI_U32  u32FrmRate;
    // 中断丢失计数,该结构体的中断丢失计数一直在增加，说明 VI 工作出现异常
    // can not support?
    MI_U32  u32LostInt;
    // 获取VB 失败计数
    MI_U32  u32VbFail;
    // 图像宽度
    MI_U32  u32PicWidth;
    // 图像高度
    MI_U32  u32PicHeight;
} MI_VIF_ChnPortStat_t;



#endif //_MI_COMMON_VIF_H_

