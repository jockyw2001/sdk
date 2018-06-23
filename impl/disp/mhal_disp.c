#define KERNEL_MODE 1
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/irqreturn.h>
#include <linux/list.h>
#include <linux/delay.h>

#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
#include "MsTypes.h"
#include "apiXC.h"
#include "apiPNL.h"
#include "apiDAC.h"
#include "drvPQ.h"
#include "drvMVOP.h"
#include "apiHDMITx.h"
#include "drvDAC.h"
#include "mdrv_dac_tbl.h"
#include "MsIRQ.h"

#define HAL_INPUTPORT_MAX 1
#define HAL_LAYER_MAX 1
#define HAL_DEV_MAX 1

#define MHAL_DISP_CHECK_NULLPOINTER(pointer)\
    {\
        if(pointer == NULL)\
        {\
            printk("NULL pointer!!!\n");\
            goto EXIT;\
        }\
    }
#define MHAL_DISP_CHECK_INVAILDDEV(Dev)\
    {\
        if(Dev >= HAL_DEV_MAX)\
        {\
            printk("[%s %d]Invild Display Dev %d!!!\n", __FUNCTION__, __LINE__, Dev);\
            goto EXIT;\
        }\
    }
#define MHAL_DISP_CHECK_INVAILDLAYER(Layer)\
    {\
        if(Layer >= HAL_LAYER_MAX)\
        {\
            printk("[%s %d]Invild Video layer %d!!!\n", __FUNCTION__, __LINE__, Layer);\
            goto EXIT;\
        }\
    }

#define MHAL_DISP_CHECK_INVAILDINPUTPORT(Port)\
    {\
        if(Port >= HAL_INPUTPORT_MAX)\
        {\
            printk("[%s %d] Invild Input port %d!!!\n", __FUNCTION__, __LINE__, Port);\
            goto EXIT;\
        }\
    }

typedef enum
{
  E_CHIP_MIU_0 = 0,
  E_CHIP_MIU_1,
  E_CHIP_MIU_2,
  E_CHIP_MIU_3,
  E_CHIP_MIU_NUM,
} CHIP_MIU_ID;

#define ARM_CLOCK_FREQ              900000000
#define MIPS_CLOCK_FREQ             12000000    //for FPGA Platform
#define AEON_CLOCK_FREQ             240000000
#define XTAL_CLOCK_FREQ             12000000

//* MIU ADDR */
#define HAL_MIU0_BASE               0x00000000UL
#if defined(__AEONR2__)
#define HAL_MIU1_BASE               0x40000000UL // 1512MB
#else
#define HAL_MIU1_BASE               0x80000000UL // 1512MB
#define HAL_MIU2_BASE               0xC0000000UL //
#endif

/* BUS ADDR */
#define HAL_MIU0_BUS_BASE           0x20000000UL // MIU0 Low 256MB
#define HAL_MIU1_BUS_BASE           0x60000000UL // MIU1 Low 256MB
#define HAL_MIU2_BUS_BASE           0xE0000000UL // MIU2 Low 256MB

#define MHAL_DISP_MVOP_VSYNC 0x01
#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < HAL_MIU1_BASE) \
                                                        {MiuSel = E_CHIP_MIU_0; Offset = PhysAddr;} \
                                                     else \
                                                         {MiuSel = E_CHIP_MIU_1; Offset = PhysAddr - HAL_MIU1_BASE;}
#define HAL_DISP_DEV0_IRQ 211   //Mvop vsync Int

typedef struct MHAL_DISP_InputPortInstance_s
{
    MS_U32 u32PortId;
    MS_BOOL bEnable;
    MS_U32 u32BindedLayerID;
    MHalDispVidWinRect_t stDispWin;                     /* rect of video out chn */

    MS_PHYADDR aphyCurYUVAddr[3];
    struct list_head stInputPortNode;
#if(KERNEL_MODE)
    struct semaphore stPortMutex;
#else
    pthread_mutex_t stPortMutex;
#endif
}MHAL_DISP_InputPortInstance_t;

typedef struct MHAL_DISP_LayerInstance_s
{
    MS_U32 u32LayerId;
    MS_BOOL bEnable;
    MHalDispVidWinRect_t  stVidLayerDispWin;   /* Display resolution */
    MS_U32 u32Width;//Video layer width
    MS_U32 u32Height;//Video layer height
    MHalDispPixelFormat_e ePixFormat;         /* Pixel format of the video layer */
    MS_BOOL bEnableBatch;
    MS_U32 u32Device;
    struct list_head stInputPortList;
    struct list_head stLayerNode;
#if(KERNEL_MODE)
    struct semaphore stLayerMutex;
#else
    pthread_mutex_t stLayerMutex;
#endif

}MHAL_DISP_LayerInstance_t;

typedef struct MHAL_DISP_DevInstance_s
{
    MS_U32 u32DevId;
    MS_BOOL bEnable;
    MS_U32 u32BgColor;
    MS_U32 u32VoutDevice;
    MS_U32 u32Device;
    struct list_head stBindedLayer;
#if(KERNEL_MODE)
    struct semaphore stDevMutex;
#else
    pthread_mutex_t stDevMutex;
#endif
}MHAL_DISP_DevInstance_t;


typedef enum
{
    E_DAC_TABTYPE_INIT,
    E_DAC_TABTYPE_INIT_GPIO,
    E_DAC_TABTYPE_INIT_SC,
    E_DAC_TABTYPE_INIT_MOD,
    E_DAC_TABTYPE_INIT_HDGEN,
    E_DAC_TABTYPE_INIT_HDMITX_8BIT,
    E_DAC_TABTYPE_INIT_HDMITX_8BIT_DIVIDER,
    E_DAC_TABTYPE_INIT_HDMITX_10BIT,
    E_DAC_TABTYPE_INIT_HDMITX_10BIT_DIVIDER,
    E_DAC_TABTYPE_INIT_HDMITX_12BIT,
    E_DAC_TABTYPE_INIT_HDMITX_12BIT_DIVIDER,
    E_DAC_TABTYPE_INIT_HDMITX_16BIT,
    E_DAC_TABTYPE_INIT_HDMITX_16BIT_DIVIDER,
    E_DAC_TABTYPE_GAMMA,
    E_DAC_TABTYPE_SC1_INIT = 14,
    E_DAC_TABTYPE_SC1_INIT_SC = 15
} EN_DAC_TAB_TYPE;

static MHAL_DISP_DevInstance_t* pstDevInstance[HAL_DEV_MAX] = {NULL};
static MHAL_DISP_LayerInstance_t* pstLayerInstance[HAL_LAYER_MAX] = {NULL};
static MHAL_DISP_InputPortInstance_t* pstInputPortInstance[HAL_LAYER_MAX][HAL_INPUTPORT_MAX] = {{NULL}};
static MS_BOOL abDevExisted[HAL_DEV_MAX] = {FALSE};
static MS_S32  s32MVOP_Event_id = -1;
static MHalDispPanelConfig_t* _pstPanelConfig;
static MS_U8 u8PanelSize = 0;
static MHalDispMmapInfo_t stMmapInfo[E_MHAL_DISP_MMAP_MAX];

MS_BOOL _MHalDispGetPanelInfo(MHalDispDeviceTiming_e eDeviceTiming, PanelType* pstPanelInfo)
{
    MS_BOOL bFound = FALSE;
    MS_U8 u8Count = 0;
    MHAL_DISP_CHECK_NULLPOINTER(pstPanelInfo);

    for(u8Count = 0; u8Count < u8PanelSize; u8Count++)
    {
       if((_pstPanelConfig[u8Count].bValid == TRUE) && (_pstPanelConfig[u8Count].eTiming == eDeviceTiming))
       {
           printk("[%s %d] eTiming = %d found!!!!\n", __FUNCTION__, __LINE__, _pstPanelConfig[u8Count].eTiming);
           *pstPanelInfo = _pstPanelConfig[u8Count].stPanelAttr;
           bFound = TRUE;
       }
    }

EXIT:
    return bFound;
}

MS_BOOL _MHalDispPanelInit(void)
{
    MS_BOOL bRet = FALSE;
    MS_U8 *pDACTable = NULL;
    //Initialize panel
    //Load default panel params
    PanelType stPanelInfo;
    memset(&stPanelInfo, 0, sizeof(PanelType));
    // TODO: Tommy Remove later, if user set already
    if(!_MHalDispGetPanelInfo(E_MHAL_DISP_OUTPUT_1080P60, &stPanelInfo))
    {
        bRet = FALSE;
        goto EXIT;
    }

    bRet = MApi_PNL_Init(&stPanelInfo);
    if(bRet == FALSE)
    {
        //DBG_ERR("MApi_PNL_Init Fail!!!\n");
        goto EXIT;
    }

    MApi_DAC_Init();

    if(MApi_PNL_IOMapBaseInit() == FALSE)
    {
        //DBG_ERR("Panel IO map base Init fail!!\n");
        bRet = FALSE;
    }
    // dump DAC general/scaler/HDGEN register tables

    MApi_DAC_SetYPbPrOutputTiming(RES_1920x1080P_60Hz);

    MApi_DAC_SetClkInv(TRUE, TRUE);
    //Note: your dac table must be consistent with your output type
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT);
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_SC);
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_HDGEN);

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL _MHalDispScalerInit(void)
{
    MS_BOOL bRet = FALSE;
    //Initialize XC
    XC_INITDATA sXC_InitData;
    XC_INITDATA *pstXC_InitData = &sXC_InitData;
    MS_U64 u64MenuloadAddr = stMmapInfo[E_MHAL_DISP_MMAP_XC_MENULOAD].u32Addr;
    MS_U32 u32MenuloadSize = stMmapInfo[E_MHAL_DISP_MMAP_XC_MENULOAD].u32Size;

    //pstXC_InitData->u32Main_FB_Start_Addr = pstAlloc->alloc(MainFBSize);
    //pstXC_InitData->u32Main_FB_Size = MainFBSize;
    pstXC_InitData->bMirror = FALSE;

    pstXC_InitData->u32Main_FB_Start_Addr = stMmapInfo[E_MHAL_DISP_MMAP_XC_MAIN].u32Addr;
    pstXC_InitData->u32Main_FB_Size = stMmapInfo[E_MHAL_DISP_MMAP_XC_MAIN].u32Size;

    //panel params
    pstXC_InitData->stPanelInfo.u16HStart = g_IPanel.HStart();      // DE H start
    pstXC_InitData->stPanelInfo.u16VStart = g_IPanel.VStart();
    pstXC_InitData->stPanelInfo.u16Width  = g_IPanel.Width();
    pstXC_InitData->stPanelInfo.u16Height = g_IPanel.Height();
    pstXC_InitData->stPanelInfo.u16HTotal = g_IPanel.HTotal();
    pstXC_InitData->stPanelInfo.u16VTotal = g_IPanel.VTotal();
    pstXC_InitData->stPanelInfo.u16DefaultVFreq = g_IPanel.DefaultVFreq();
    pstXC_InitData->stPanelInfo.u8LPLL_Mode = g_IPanel.LPLL_Mode();
    pstXC_InitData->stPanelInfo.eLPLL_Type = (E_XC_PNL_LPLL_TYPE)g_IPanel.LPLL_Type();
    pstXC_InitData->stPanelInfo.enPnl_Out_Timing_Mode = (E_XC_PNL_OUT_TIMING_MODE)g_IPanel.OutTimingMode();
    pstXC_InitData->stPanelInfo.u16DefaultHTotal = g_IPanel.HTotal();
    pstXC_InitData->stPanelInfo.u16DefaultVTotal = g_IPanel.VTotal();
    pstXC_InitData->stPanelInfo.u32MinSET = g_IPanel.MinSET();
    pstXC_InitData->stPanelInfo.u32MaxSET = g_IPanel.MaxSET();
    MsOS_MPool_Init();
    //MDrv_SYS_GlobalInit();

    if(MApi_XC_Init(pstXC_InitData, sizeof(XC_INITDATA)) == FALSE)
    {
        //DBG_ERR("XC_Init failed because of InitData wrong, please update header file and compile again\n");
        bRet = FALSE;
        goto EXIT;
    }
    //Enable Menuload
    //MI_U64 u64MenuloadAddr = pstAlloc->alloc(MenuloadSize);

    MApi_XC_MLoad_Init(u64MenuloadAddr, u32MenuloadSize);
    //MApi_XC_MLoad_Enable(TRUE);
    MDrv_MVOP_Init();
    bRet = TRUE;
EXIT:
    return bRet;
}

#if(KERNEL_MODE)
// TODO: Tommy pq bin  放在指定位置

#else
#define PQ_FILE_NAME_LENGTH 128
static char pm_PQBinFilePathName[PQ_FILE_NAME_LENGTH];
static char pm_PQBinFilePathName[PQ_FILE_NAME_LENGTH] = {0};
static u8* m_pPqBinBufAddr[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

u8* Open_PQ_Bin(u8 * pName)
{
    FILE *pFile = NULL;
    long lFileSize;
    MS_U32 u32Rst;
    const char * Name;
    u8 *pu8Buffer;

    Name = (const char *)pName;
    pFile = fopen(Name, "rb");
    if(pFile == NULL)
    {
        printf("Open %s Failed\n", pName);
        return NULL;
    }

    fseek(pFile, 0, SEEK_END);
    lFileSize = ftell(pFile);

    if (lFileSize <= 0)
    {
        fclose(pFile);
        return NULL;
    }

    rewind(pFile);
    pu8Buffer = (u8*) malloc(lFileSize);
    if(pu8Buffer == NULL)
    {
        printf("out of memory \n");
        fclose(pFile);
        return NULL;
    }

    u32Rst = fread(pu8Buffer, 1, lFileSize, pFile);
    if(u32Rst != (u32)lFileSize)
    {
        printf("Read Main.bin Error %d %ld\n", u32Rst, lFileSize);
        free(pu8Buffer);
        fclose(pFile);
        return NULL;
    }

    fclose(pFile);
   //printf("mapi_pql_customer::Open_PQ_Bin:open name:%s Bufaddress=%p\n",pName,pu8Buffer);

    return pu8Buffer;
}

MS_BOOL SetCustomerPQBinFileName(const char * const pPQCustomerBinFileName)
{
    FILE *pFile = NULL;
    u16 u16CusPQBinLen = 0;
    u16 u16CusPQPathLen = 0;

    char *pqBinFolderPathName = NULL;

    if((pqBinFolderPathName == NULL) || (pPQCustomerBinFileName == NULL))
    {
        return false;
    }
    u16CusPQPathLen = strlen(pqBinFolderPathName);
    u16CusPQBinLen = strlen(pPQCustomerBinFileName);

    if((u16CusPQBinLen + u16CusPQPathLen) >= PQ_FILE_NAME_LENGTH)
    {
        printf("Invalid PQ Bin Path, PQ Bin path is too long, Return False !!!!\n");
        return false;
    }
    memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
    strncpy(pm_PQBinFilePathName, pqBinFolderPathName, u16CusPQPathLen);
    strncat(pm_PQBinFilePathName, pPQCustomerBinFileName, u16CusPQBinLen);

    pFile = fopen(pm_PQBinFilePathName, "rb");
    if(pFile == NULL)
    {
        return false;
    }
    else
    {
        fclose(pFile);
        return true;
    }
}
#endif

MS_BOOL _MHalDispPqInit(void)
{
    MS_BOOL bRet = FALSE;
    u16 u16PnlWidth;
    //pq init
    PNL_ApiExtStatus stPNLStatus;
    MS_PQ_Init_Info sXC_PQ_InitData;
    PQ_Function_Info function_Info;
    MS_U32 u32InitDataLen = 0;
    memset(&function_Info, 0, sizeof(function_Info));
    memset(&sXC_PQ_InitData, 0, sizeof(MS_PQ_Init_Info));

    MDrv_BW_LoadInitTable();
    memset(&stPNLStatus, 0, sizeof(PNL_ApiExtStatus));
    stPNLStatus.u32ApiStatusEx_Version = API_PNLSTATUS_EX_VERSION;
    stPNLStatus.u16ApiStatusEX_Length = sizeof(PNL_ApiExtStatus);

    if(MApi_PNL_GetStatusEx(&stPNLStatus) == FALSE)
    {
        //DBG_ERR("MApi_PNL_GetStatusEx return error\n");
        bRet = FALSE;
        goto EXIT;
    }
    u16PnlWidth = stPNLStatus.u16DEHEnd - stPNLStatus.u16DEHStart + 1;
    if(u16PnlWidth <= 1366)
    {
        sXC_PQ_InitData.u16PnlWidth = 1366;
    }
    else if(u16PnlWidth == 1920)
    {
        sXC_PQ_InitData.u16PnlWidth = 1920;
    }
    else if (u16PnlWidth == 3840)
    {
        sXC_PQ_InitData.u16PnlWidth = 3840;
    }
    else
    {
        sXC_PQ_InitData.u16PnlWidth = 1920;
    }
    sXC_PQ_InitData.u16PnlHeight = g_IPanel.Height();
    sXC_PQ_InitData.u16Pnl_vtotal = stPNLStatus.u16VTotal;

#if (!KERNEL_MODE)
    u8 *pu8MainBuf = NULL;
    u8 *pu8MainExBuf = NULL ;
    const char DefaltPath[] = "/config/pq/";
    const char MainBinName[]     = "Main.bin";
    const char MainExBinName[]   = "Main_Ex.bin";
    const char MainColorBinName[]= "Main_Color.bin";
    const char CustomerPath[] = "/Customer/pq/";
    FILE *pFile = NULL;

    // if Main.bin exists in /Customer/pq, read it
    memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
    strncpy(pm_PQBinFilePathName, CustomerPath, strlen(CustomerPath));
    strncat(pm_PQBinFilePathName, MainBinName, strlen(MainBinName));
    pFile = fopen(pm_PQBinFilePathName, "r");
    if(pFile == NULL)
    {
        if(SetCustomerPQBinFileName(MainBinName) == false)
        {
            // read from default path
            memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
            strncpy(pm_PQBinFilePathName, DefaltPath, strlen(DefaltPath));
            strncat(pm_PQBinFilePathName, MainBinName, strlen(MainBinName));
        }
        if(m_pPqBinBufAddr[PQ_BIN_STD_MAIN] == NULL)
        {
            pu8MainBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
            m_pPqBinBufAddr[PQ_BIN_STD_MAIN] = pu8MainBuf;
        }
        else
        {
            pu8MainBuf = m_pPqBinBufAddr[PQ_BIN_STD_MAIN];
        }
    }
    else
    {
        // read from /Customer/pq
        fclose(pFile);
        pFile = NULL;
        if(m_pPqBinBufAddr[PQ_BIN_STD_MAIN] == NULL)
        {
            pu8MainBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
            m_pPqBinBufAddr[PQ_BIN_STD_MAIN] = pu8MainBuf;
        }
        else
        {
            pu8MainBuf = m_pPqBinBufAddr[PQ_BIN_STD_MAIN];
        }
    }
    printf("Load PQ bin file, PQ_BIN_STD_MAIN path [%s]\n", pm_PQBinFilePathName);

    // if Main_Ex.bin exists in /Customer/pq, read it
    memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
    strncpy(pm_PQBinFilePathName, CustomerPath, strlen(CustomerPath));
    strncat(pm_PQBinFilePathName, MainExBinName, strlen(MainExBinName));
    pFile = fopen(pm_PQBinFilePathName, "r");
    if(pFile == NULL)
    {
        if(SetCustomerPQBinFileName(MainExBinName) == false)
        {
            // read from default path
            memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
            strncpy(pm_PQBinFilePathName, DefaltPath, strlen(DefaltPath));
            strncat(pm_PQBinFilePathName, MainExBinName, strlen(MainExBinName));
        }
        if(m_pPqBinBufAddr[PQ_BIN_EXT_MAIN] == NULL)
        {
            pu8MainExBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
            m_pPqBinBufAddr[PQ_BIN_EXT_MAIN] = pu8MainExBuf;
        }
        else
        {
            pu8MainExBuf = m_pPqBinBufAddr[PQ_BIN_EXT_MAIN];
        }
    }
    else
    {
        // read from /Customer/pq
        fclose(pFile);
        pFile = NULL;
        if(m_pPqBinBufAddr[PQ_BIN_EXT_MAIN] == NULL)
        {
            pu8MainExBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
            m_pPqBinBufAddr[PQ_BIN_EXT_MAIN] = pu8MainExBuf;
        }
        else
        {
            pu8MainExBuf = m_pPqBinBufAddr[PQ_BIN_EXT_MAIN];
        }

    }
    printf("Load PQ bin file, PQ_BIN_EXT_MAIN path [%s]\n", pm_PQBinFilePathName);

    if(pu8MainBuf && pu8MainExBuf)
    {
        sXC_PQ_InitData.u8PQBinCnt = 2;
        sXC_PQ_InitData.stPQBinInfo[0].u8PQID = (u8) PQ_BIN_STD_MAIN;
        sXC_PQ_InitData.stPQBinInfo[0].PQBin_PhyAddr = (MS_PHYADDR) pu8MainBuf;
        sXC_PQ_InitData.stPQBinInfo[1].u8PQID = (u8) PQ_BIN_EXT_MAIN;
        sXC_PQ_InitData.stPQBinInfo[1].PQBin_PhyAddr = (MS_PHYADDR)pu8MainExBuf;
    }

    u8 *pu8MainCustomerBuf = NULL;
    sXC_PQ_InitData.u8PQBinCustomerCnt = 0 ;
    // if Main_Color.bin exists in /Customer/pq, read it
    memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
    strncpy(pm_PQBinFilePathName, CustomerPath, strlen(CustomerPath));
    strncat(pm_PQBinFilePathName, MainColorBinName, strlen(MainColorBinName));
    pFile = fopen(pm_PQBinFilePathName, "r");
    if(pFile == NULL)
    {
        if(SetCustomerPQBinFileName(MainColorBinName) == false)
        {
            // read from default path
            memset(pm_PQBinFilePathName, 0, PQ_FILE_NAME_LENGTH);
            strncpy(pm_PQBinFilePathName, DefaltPath, strlen(DefaltPath));
            strncat(pm_PQBinFilePathName, MainColorBinName, strlen(MainColorBinName));
        }
        pFile = fopen(pm_PQBinFilePathName, "r");
        if (pFile != NULL)
        {
            fclose(pFile);
            pFile = NULL;
            if(m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN] == NULL)
            {
                pu8MainCustomerBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
                m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN] = pu8MainCustomerBuf;
            }
            else
            {
                pu8MainCustomerBuf = m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN];
            }
        }
    }
    else
    {
        // read from /Customer/pq
        fclose(pFile);
        pFile = NULL;
        if(m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN] == NULL)
        {
            pu8MainCustomerBuf = Open_PQ_Bin((u8 *)pm_PQBinFilePathName);
            m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN] = pu8MainCustomerBuf;
        }
        else
        {
            pu8MainCustomerBuf = m_pPqBinBufAddr[PQ_BIN_CUSTOMER_MAIN];
        }
    }

    if(pu8MainCustomerBuf)
    {
        printf("Load PQ bin file, PQ_BIN_CUSTOMER_MAIN path [%s]\n", pm_PQBinFilePathName);
        sXC_PQ_InitData.u8PQBinCustomerCnt = 1 ;
        sXC_PQ_InitData.stPQBinInfo[4].u8PQID = (u8) PQ_BIN_CUSTOMER_MAIN;
        sXC_PQ_InitData.stPQBinInfo[4].PQBin_PhyAddr = (MS_PHYADDR) pu8MainCustomerBuf;
    }
#else
    //Utopia pq driver
#endif

    function_Info.pq_deside_srctype = MDrv_PQ_DesideSrcType;
    function_Info.pq_disable_filmmode = MDrv_PQ_DisableFilmMode;
    function_Info.pq_load_scalingtable = MDrv_PQ_LoadScalingTable;
    function_Info.pq_set_420upsampling = MDrv_PQ_Set420upsampling;
    function_Info.pq_set_csc = MDrv_PQ_SetCSC;
    function_Info.pq_set_memformat = MDrv_PQ_SetMemFormat;
    function_Info.pq_set_modeInfo = MDrv_PQ_Set_ModeInfo;
    function_Info.pq_get_memyuvfmt = MDrv_PQ_Get_MemYUVFmt;
    function_Info.pq_ioctl = MDrv_PQ_IOCTL;

    MApi_XC_PQ_LoadFunction(&function_Info , u32InitDataLen);
    MDrv_PQ_Init(&sXC_PQ_InitData);

    bRet = TRUE;
EXIT:
    return bRet;

}

MS_BOOL MHalDispInitPanelConfig(MHalDispPanelConfig_t* pstPanelConfig, MS_U8 u8Size)
{
    MS_U8 u8Count = 0;
    MHAL_DISP_CHECK_NULLPOINTER(pstPanelConfig);
    if(u8Size > E_MHAL_DISP_OUTPUT_MAX)
    {
        u8Size = E_MHAL_DISP_OUTPUT_MAX;
    }
    _pstPanelConfig = kmalloc((sizeof(MHalDispPanelConfig_t) * u8Size), GFP_KERNEL);
    memset(_pstPanelConfig, 0, (sizeof(MHalDispPanelConfig_t) * u8Size));
    memcpy(_pstPanelConfig, pstPanelConfig, (sizeof(MHalDispPanelConfig_t) * u8Size));
    u8PanelSize = u8Size;
    for(u8Count = 0; u8Count < u8Size; u8Count++)
    {
        printk("[%s %d] Timing = %d, dev = %d, bValid = %d \n", __FUNCTION__, __LINE__, _pstPanelConfig[u8Count].eTiming, _pstPanelConfig[u8Count].u32OutputDev, _pstPanelConfig[u8Count].bValid == TRUE);
    }
EXIT:
    return TRUE;
}

MS_BOOL MHalDispInitMmapConfig(MHalDispMmapType_e eMmType, MHalDispMmapInfo_t* pstMmapInfo)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_CHECK_NULLPOINTER(pstMmapInfo);
    if(eMmType >= E_MHAL_DISP_MMAP_MAX)
    {
        bRet = FALSE;
        goto EXIT;
    }
    stMmapInfo[eMmType].u32Addr = pstMmapInfo->u32Addr;
    stMmapInfo[eMmType].u32Align = pstMmapInfo->u32Align;
    stMmapInfo[eMmType].u8MiuNo = pstMmapInfo->u8MiuNo;
    stMmapInfo[eMmType].u8CMAHid = pstMmapInfo->u8CMAHid;
    stMmapInfo[eMmType].u8Gid = pstMmapInfo->u8Gid;
    stMmapInfo[eMmType].u8Layer = pstMmapInfo->u8Layer;
    stMmapInfo[eMmType].u32MemoryType = pstMmapInfo->u32MemoryType;
    stMmapInfo[eMmType].u32Size = pstMmapInfo->u32Size;
EXIT:
    return TRUE;
}

MS_BOOL _MHalDispDevInstanceInit(MHAL_DISP_DevInstance_t* pstDevIns, const MS_U32 u32DeviceId)
{
    pstDevIns->bEnable = FALSE;
    pstDevIns->u32BgColor = 0;
    INIT_LIST_HEAD(&(pstDevIns->stBindedLayer));
    pstDevIns->u32DevId = u32DeviceId;
    return TRUE;
}

MS_BOOL _MhalDispInitDevInt(MS_U32 u32Dev)
{
    MS_BOOL bRet = FALSE;
    if(u32Dev != 0)
    {
        printk("[%s %d] Not Support Dev %d\n", __FUNCTION__, __LINE__, u32Dev);
        bRet = FALSE;
        return bRet;
    }
    printk("[%s %d] \n", __FUNCTION__, __LINE__);
    if(!MsOS_DisableInterrupt(E_INT_IRQ_DC))
    {
        //DBG_ERR("[%s %d]Disable E_INT_IRQ_DC Fail!\n", __FUNCTION__, __LINE__);
    }
    if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_NONE))
    {
        //DBG_ERR("[%s %d]Clear Mvop Int Fail!\n", __FUNCTION__, __LINE__);
    }
    if(!MsOS_EnableInterrupt(E_INT_IRQ_DC))
    {
        //DBG_ERR("[%s %d]Enable Mvop IRQ Fail!\n", __FUNCTION__, __LINE__);
    }
    if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_VSYNC))
    {
        //DBG_ERR("[%s %d]Enable Mvop Int Fail!\n", __FUNCTION__, __LINE__);
    }
    bRet = TRUE;
    return bRet;
}

MS_BOOL _MhalDispEnableInterrupt(MS_U32 u32DevId)
{
    MS_BOOL bRet = FALSE;

    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
    if(u32DevId == 0)
    {
        if(!MsOS_EnableInterrupt(E_INT_IRQ_DC))
        {
            //DBG_ERR("[%s %d]Enable Mvop IRQ Fail!\n", __FUNCTION__, __LINE__);
        }
        if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_VSYNC))
        {
            //DBG_ERR("[%s %d]Enable Mvop Int Fail!\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        printk("[%s %d]Not Support Dev %d\n", __FUNCTION__, __LINE__, u32DevId);
    }
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispDeviceCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, const MS_U32 u32DeviceId, void **pDevCtx)
{
    MS_BOOL bRet = FALSE;
    //DBG_ERR("Disable Display Dev %d !\n", u32DeviceId);
    MHAL_DISP_CHECK_NULLPOINTER(pstAlloc);
    MHAL_DISP_CHECK_INVAILDDEV(u32DeviceId);

    if(abDevExisted[u32DeviceId] == FALSE)
    {
        _MHalDispPanelInit();
        _MHalDispScalerInit();
        _MHalDispPqInit();
        _MhalDispInitDevInt(u32DeviceId);
        //Save Dev instance
#if(KERNEL_MODE)
        pstDevInstance[u32DeviceId] = (MHAL_DISP_DevInstance_t*)kmalloc(sizeof(MHAL_DISP_DevInstance_t), GFP_KERNEL);
#else
        pstDevInstance[u32DeviceId] = (MHAL_DISP_DevInstance_t*)malloc(sizeof(MHAL_DISP_DevInstance_t));
#endif
        memset(pstDevInstance[u32DeviceId], 0, sizeof(MHAL_DISP_DevInstance_t));
#if(KERNEL_MODE)
        init_MUTEX(&(pstDevInstance[u32DeviceId]->stDevMutex));
        down(&(pstDevInstance[u32DeviceId]->stDevMutex));
#else
        pthread_mutex_init(&(pstDevInstance[u32DeviceId]->stDevMutex), NULL);
        pthread_mutex_lock(&(pstDevInstance[u32DeviceId]->stDevMutex));
#endif
        _MHalDispDevInstanceInit(pstDevInstance[u32DeviceId], u32DeviceId);
        *pDevCtx = (void *)(pstDevInstance[u32DeviceId]);
        abDevExisted[u32DeviceId] = TRUE;
#if(KERNEL_MODE)
        up(&(pstDevInstance[u32DeviceId]->stDevMutex));
#else
        pthread_mutex_unlock(&(pstDevInstance[u32DeviceId]->stDevMutex));
#endif
    }
    else
    {
        //DBG_ERR("Dev%d Instance Already Created!\n", u32DeviceId);
    }
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispDeviceDestroyInstance(void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceDestroyInstance !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);

    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);

    if(abDevExisted[u32DevId] == TRUE)
    {
        //Deinit Scaler
        MApi_XC_Exit();
        //Deinit pq
        MDrv_PQ_Exit();
        //Deinit Mvop
        // TODO: Tommy : Need add disable mvop int
        MDrv_MVOP_Exit();
        //Destory Dev instance
        abDevExisted[u32DevId] = FALSE;
        pstDevInstance[u32DevId] = NULL;
#if(KERNEL_MODE)
        kfree(pstTmpDev);
#else
        free(pstTmpDev);
#endif
    }
    else
    {
        //DBG_ERR("Dev%d Instance Already Destoryed!\n", u32DevId);
    }
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispDeviceEnable(void *pDevCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceEnable !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);

    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif
    MApi_XC_EnableWindow(bEnable, MAIN_WINDOW);
    pstTmpDev->bEnable = bEnable;
#if(KERNEL_MODE)
    up(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispDeviceSetBackGroundColor(void *pDevCtx, const MS_U32 u32BgColor)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    u8 u8MuteColor = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceSetBackGroundColor !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);

    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);

#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    u8MuteColor = ((u32BgColor)&0x00003)|(((u32BgColor>>8)<<2)&0x1C) |(((u32BgColor>>16)<<5)&0xE0);
    MApi_XC_SetDispWindowColor(u8MuteColor, MAIN_WINDOW);
    pstTmpDev->u32BgColor = u32BgColor;
#if(KERNEL_MODE)
    up(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL _MHalDispDeviceSetPanelDclk(PanelType* pstPanelInfo, MHalDispDeviceTiming_e eDeviceTiming)
{
    MS_BOOL bRet = FALSE;
    u16 u16PanelDclock = 0;
    MHAL_DISP_CHECK_NULLPOINTER(pstPanelInfo);
    //DBG_ERR("_MHalDispDeviceSetPanelDclk !!!\n");
    u16PanelDclock = pstPanelInfo->m_dwPanelDCLK;

    MApi_PNL_ForceSetPanelHStart(pstPanelInfo->m_wPanelHStart,TRUE);

    switch(eDeviceTiming)
    {
        case E_MHAL_DISP_OUTPUT_3840x2160_30:
        case E_MHAL_DISP_OUTPUT_3840x2160_60:
            bRet = MApi_PNL_ForceSetPanelDCLK(u16PanelDclock , TRUE);
            break;
        default:
            bRet = MApi_PNL_ForceSetPanelDCLK(u16PanelDclock , FALSE);
        break;
    }
EXIT:
    return bRet;
}

MS_BOOL _MHalDispDeviceChangePanelType(PanelType* pstPanelInfo)
{
    MS_BOOL bRet = FALSE;
    XC_PANEL_INFO stPanelInfo;
    memset(&stPanelInfo, 0, sizeof(stPanelInfo));
    //DBG_ERR("_MHalDispDeviceChangePanelType !!!\n");
    //Change panel type
    MApi_PNL_SkipTimingChange(FALSE);
    if(!MApi_PNL_ChangePanelType(pstPanelInfo))
    {
        bRet = FALSE;
        goto EXIT;
    }
    //Change xc panel params
    stPanelInfo.u16HStart = g_IPanel.HStart();
    stPanelInfo.u16VStart = g_IPanel.VStart();
    stPanelInfo.u16Width = g_IPanel.Width();
    stPanelInfo.u16Height = g_IPanel.Height();
    stPanelInfo.u16HTotal = g_IPanel.HTotal();
    stPanelInfo.u16VTotal  = g_IPanel.VTotal();
    stPanelInfo.u16DefaultVFreq = g_IPanel.DefaultVFreq();
    stPanelInfo.u8LPLL_Mode = g_IPanel.LPLL_Mode();

    stPanelInfo.enPnl_Out_Timing_Mode = (E_XC_PNL_OUT_TIMING_MODE)g_IPanel.OutTimingMode();

    stPanelInfo.u16DefaultHTotal = g_IPanel.HTotal();
    stPanelInfo.u16DefaultVTotal = g_IPanel.VTotal();
    stPanelInfo.u32MinSET = g_IPanel.MinSET();
    stPanelInfo.u32MaxSET = g_IPanel.MaxSET();

    stPanelInfo.eLPLL_Type = (E_XC_PNL_LPLL_TYPE) g_IPanel.LPLL_Type();

    MApi_XC_ChangePanelType(&stPanelInfo);

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL _MHalDispDeviceSetDacTable(MHalDispDeviceTiming_e eDeviceTiming)
{
    MS_BOOL bRet = TRUE;
    MS_U8 *pDACTable = NULL;
    E_OUTPUT_VIDEO_TIMING_TYPE eTimingType = E_RES_1920x1080P_60Hz;
    switch(eDeviceTiming)
    {
        case E_MHAL_DISP_OUTPUT_PAL:
            eTimingType = E_RES_720x576I_50Hz;
            break;
        case E_MHAL_DISP_OUTPUT_NTSC:
            eTimingType = E_RES_720x480I_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_960H_PAL:
            //eTimingType = E_RES_720x576I_50Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_960H_NTSC:
            //eTimingType = E_RES_720x480I_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1080P24:
            eTimingType = E_RES_1920x1080P_24Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080P25:
            eTimingType = E_RES_1920x1080P_25Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080P30:
            eTimingType = E_RES_1920x1080P_30Hz;
            break;
        case E_MHAL_DISP_OUTPUT_720P50:
            eTimingType = E_RES_1280x720P_50Hz;
            break;
        case E_MHAL_DISP_OUTPUT_720P60:
            eTimingType = E_RES_1280x720P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080I50:
            eTimingType = E_RES_1920x1080I_50Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080I60:
            eTimingType = E_RES_1920x1080I_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080P50:
            eTimingType = E_RES_1920x1080P_50Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1080P60:
            eTimingType = E_RES_1920x1080P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_576P50:
            eTimingType = E_RES_720x576P_50Hz;
            break;
        case E_MHAL_DISP_OUTPUT_480P60:
            eTimingType = E_RES_720x480P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_640x480_60:
            eTimingType = E_RES_640x480P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_800x600_60:
            //eTimingType = E_RES_1280x720P_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1024x768_60:
            eTimingType = E_RES_1024x768P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1280x1024_60:
            eTimingType = E_RES_1280x1024P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1366x768_60:
            //eTimingType = E_RES_1920x1080P_50Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1440x900_60:
            eTimingType = E_RES_1440x900P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1280x800_60:
            //eTimingType = E_RES_720x576P_50Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1680x1050_60:
            //eTimingType = E_RES_720x480P_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1920x2160_30:
            //eTimingType = E_RES_640x480P_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_1600x1200_60:
            eTimingType = E_RES_1600x1200P_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_1920x1200_60:
            //eTimingType = E_RES_1024x768P_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1440_30:
            //eTimingType = E_RES_1280x1024P_60Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_2560x1600_60:
            //eTimingType = E_RES_1920x1080P_50Hz;
            bRet = FALSE;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_30:
            eTimingType = E_RES_3840x2160P_30Hz;
            break;
        case E_MHAL_DISP_OUTPUT_3840x2160_60:
            eTimingType = E_RES_3840x2160P_60Hz;
            break;
        default:
            bRet = FALSE;
            break;
    }

    MApi_DAC_SetYPbPrOutputTiming(eTimingType);
    MApi_DAC_SetClkInv(TRUE, TRUE);
    MApi_DAC_DumpTable(pDACTable, E_DAC_TABTYPE_INIT);           // dump DAC general register tab
    MApi_DAC_DumpTable(pDACTable, E_DAC_TABTYPE_INIT_SC); // dump DAC scaler register tab
    MApi_DAC_DumpTable(pDACTable, E_DAC_TABTYPE_INIT_MOD);       // dump DAC MOD register tab
    bRet = TRUE;
    return bRet;
}


MS_BOOL _MHalDispDeviceSetTxTiming(MHalDispDeviceTiming_e eDeviceTiming)
{
    MS_BOOL bRet = FALSE;
    MS_U8* pDACTable = NULL;
    HDMITX_OUTPUT_MODE eHDMITxMode = HDMITX_HDMI;
    HDMITX_VIDEO_TIMING eHDMITxTiming;
    E_OUTPUT_VIDEO_TIMING_TYPE eTimingType;
    HDMITX_VIDEO_COLOR_FORMAT eOutColorFmt = HDMITX_VIDEO_COLOR_YUV444;
    HDMITX_VIDEO_COLOR_FORMAT eInColorFmt  = HDMITX_VIDEO_COLOR_YUV444;
    HDMITX_VIDEO_COLORDEPTH_VAL eEDIDColorDepth = HDMITX_VIDEO_CD_NoID;

    switch(eDeviceTiming)
    {
        case E_MHAL_DISP_OUTPUT_1080P60:
            eTimingType = E_RES_1920x1080P_60Hz;
            eHDMITxTiming = HDMITX_RES_1920x1080p_60Hz;
            break;
        case E_MHAL_DISP_OUTPUT_720P60:
            eTimingType = E_RES_1280x720P_60Hz;
            eHDMITxTiming = HDMITX_RES_1280x720p_60Hz;
            break;
        default:
            bRet = FALSE;
            goto EXIT;
            break;
    }

    MApi_DAC_SetClkInv(TRUE, TRUE);
    MApi_DAC_SetYPbPrOutputTiming(eTimingType);

    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_HDMITX_8BIT_Divider);
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_HDMITX_8BIT);

    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT);               // dump DAC general register tab
    // mask sc dac table, because it had loaded in mapi_display_timing::SetDacOutputMode
    //MApi_DAC_DumpTable(pDACTable, E_DEVICE_DAC_TABTYPE_INIT_SC);         // dump DAC scaler register tab
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_MOD);       // dump DAC MOD register tab
    MApi_DAC_DumpTable(pDACTable, DAC_TABTYPE_INIT_HDGEN); // dump DAC HDGEN register tab

    //When change timing,should reset Colorformat,because  some TV only support YUV420 in 4k50/60
    MApi_HDMITx_SetHDMITxMode_CD(eHDMITxMode, eEDIDColorDepth);
    MApi_HDMITx_SetColorFormat(eInColorFmt, eOutColorFmt);
    MApi_HDMITx_SetVideoOutputTiming(eHDMITxTiming);

    MApi_HDMITx_Exhibit();
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL _MHalDispDeviceSetPanelInfo(MHalDispDeviceTiming_e eDeviceTiming)
{
    MS_BOOL bRet = FALSE;
    PanelType stPanelInfo;
    memset(&stPanelInfo, 0, sizeof(PanelType));
    if(!_MHalDispGetPanelInfo(eDeviceTiming, &stPanelInfo))
    {
        printk("[%s %d] Not found timing = %d\n", __FUNCTION__, __LINE__, eDeviceTiming);
        bRet = FALSE;
        goto EXIT;
    }

    if(!_MHalDispDeviceSetPanelDclk(&stPanelInfo, eDeviceTiming))
    {
        bRet = FALSE;
        goto EXIT;
    }
    if(!_MHalDispDeviceChangePanelType(&stPanelInfo))
    {
        bRet = FALSE;
        goto EXIT;
    }

    if(!_MHalDispDeviceSetDacTable(eDeviceTiming))
    {
        bRet = FALSE;
        goto EXIT;
    }

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispDeviceAddOutInterface(void *pDevCtx, const MS_U32 u32Interface)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceAddOutInterace !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);

    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    if(u32Interface & 0x1)
    {
        pstTmpDev->u32Device |= 0x1;
        //DBG_ERR("HDMI Added!!!\n");
    }
    else
    {
        //DBG_ERR("Non Support this device !!!\n");
        bRet = FALSE;
        goto EXIT;
    }
#if(KERNEL_MODE)
    up(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}


MS_BOOL MHalDispDeviceSetOutputTiming(void *pDevCtx, const MS_U32 u32Interface, const MHalDispDeviceTimingInfo_t *pstTimingInfo)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceSetOutputTiming !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);

    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif


    if(u32Interface & 0x1)
    {
        switch(pstTimingInfo->eTimeType)
        {
            case E_MHAL_DISP_OUTPUT_PAL:
            case E_MHAL_DISP_OUTPUT_NTSC:
            case E_MHAL_DISP_OUTPUT_960H_PAL:              /* ITU-R BT.1302 960 x 576 at 50 Hz (interlaced)*/
            case E_MHAL_DISP_OUTPUT_960H_NTSC:             /* ITU-R BT.1302 960 x 480 at 60 Hz (interlaced)*/
            case E_MHAL_DISP_OUTPUT_MAX:
            case E_MHAL_DISP_OUTPUT_USER:
            {
                //DBG_ERR("HDMI Not support timing type !!!\n");
                break;
            }
            default:
            {
                _MHalDispDeviceSetPanelInfo(pstTimingInfo->eTimeType);
                break;
            }

        }
    }
    else
    {
        //DBG_ERR("Non Support this device !!!\n");
    }
#if(KERNEL_MODE)
    up(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}

static void _MhalDispMvopIsr(InterruptNum eIntNum)
{
    if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_NONE))
    {
        //DBG_ERR("[%s %d]Clear Mvop Int Fail!\n", __FUNCTION__, __LINE__);
    }
    MsOS_SetEvent(s32MVOP_Event_id, MHAL_DISP_MVOP_VSYNC);
    MDrv_MVOP_EnableInterrupt(E_MVOP_INT_VSYNC);
    MsOS_EnableInterrupt(E_INT_IRQ_DC);
}

MS_BOOL _MhalDispCreateMvopEvent(void)
{
     // create MVOP event
     if(s32MVOP_Event_id < 0)
    {
        s32MVOP_Event_id = MsOS_CreateEventGroup("HalDisp_MVOP_Event");
        if(!MsOS_DisableInterrupt(E_INT_IRQ_DC))
        {
            //DBG_ERR("[%s %d]Disable E_INT_IRQ_DC Fail!\n", __FUNCTION__, __LINE__);
        }
        if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_NONE))
        {
            //DBG_ERR("[%s %d]Clear Mvop Int Fail!\n", __FUNCTION__, __LINE__);
        }
        if(!MsOS_AttachInterrupt(E_INT_IRQ_DC, (InterruptCb)_MhalDispMvopIsr))
        {
            //DBG_ERR("[%s %d]Clear Mvop IQR Fail!\n", __FUNCTION__, __LINE__);
        }
        if(!MsOS_EnableInterrupt(E_INT_IRQ_DC))
        {
            //DBG_ERR("[%s %d]Enable Mvop IRQ Fail!\n", __FUNCTION__, __LINE__);
        }
        if(!MDrv_MVOP_EnableInterrupt(E_MVOP_INT_VSYNC))
        {
            //DBG_ERR("[%s %d]Enable Mvop Int Fail!\n", __FUNCTION__, __LINE__);
        }
    }
     //DBG_ERR("[%s %d]s32MVOP_Event_id = %d!\n", __FUNCTION__, __LINE__, s32MVOP_Event_id);
    return TRUE;
}

MS_BOOL MHalDispClearDevInterrupt(void *pDevCtx, void* pData)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    //DBG_ERR("MHalDispDeviceAddOutInterace !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);
    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
    //bEnable = *(MS_BOOL*)pData;

    MDrv_MVOP_ClearInterrupt(TRUE);
    MDrv_MVOP_ClearInterrupt(FALSE);
    MsOS_EnableInterrupt(E_INT_IRQ_DC);

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispGetDevIrq(void *pDevCtx, MS_U32* pu32DevIrq)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32DevId = 0;
    MHAL_DISP_DevInstance_t* pstTmpDev = NULL;
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pu32DevIrq);
    pstTmpDev = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId = pstTmpDev->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);
    *pu32DevIrq = HAL_DISP_DEV0_IRQ;
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, const MS_U32 u32LayerId, void **pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    //DBG_ERR("MHalDispVideoLayerCreateInstance Layer ID = %d!!!\n", u32LayerId);
    MHAL_DISP_CHECK_NULLPOINTER(pstAlloc);
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
    //Save Dev instance
#if(KERNEL_MODE)
    pstLayerInstance[u32LayerId] = kmalloc(sizeof(MHAL_DISP_LayerInstance_t), GFP_KERNEL);
#else
    pstLayerInstance[u32LayerId] = malloc(sizeof(MHAL_DISP_LayerInstance_t));
#endif
    memset(pstLayerInstance[u32LayerId], 0, sizeof(MHAL_DISP_LayerInstance_t));
#if(KERNEL_MODE)
    init_MUTEX(&(pstLayerInstance[u32LayerId]->stLayerMutex));
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_init(&(pstLayerInstance[u32LayerId]->stLayerMutex), NULL);
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif
    pstLayerInstance[u32LayerId]->u32LayerId = u32LayerId;
    INIT_LIST_HEAD(&(pstLayerInstance[u32LayerId]->stLayerNode));
    INIT_LIST_HEAD(&(pstLayerInstance[u32LayerId]->stInputPortList));
    *pVidLayerCtx = pstLayerInstance[u32LayerId];

    //_MhalDispCreateMvopEvent();

#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerDestoryInstance(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispDeviceDestroyInstance !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    kfree(pstLayerIns);
#else
    free(pstLayerIns);
#endif
    pstLayerInstance[u32LayerId] = NULL;

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL _MHalDispSetVideoLayerMux(MHAL_DISP_LayerInstance_t *pVidLayerCtx, MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32LayerId = 0;
    s16 s16PathId = 0;
    XC_MUX_PATH_INFO PathInfo;
    //DBG_ERR("_MHalDispSelectVideoLayerMux !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    u32LayerId = pVidLayerCtx->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);

    memset(&PathInfo, 0, sizeof(XC_MUX_PATH_INFO));
    // create path
    PathInfo.Path_Type = PATH_TYPE_SYNCHRONOUS;
    //Just for mvop path
    PathInfo.src = INPUT_SOURCE_STORAGE;
    PathInfo.dest = OUTPUT_SCALER_MAIN_WINDOW;
    if(bEnable == TRUE)
    {
        s16PathId = MApi_XC_Mux_CreatePath(&PathInfo, sizeof(XC_MUX_PATH_INFO));
        if(s16PathId == -1)
        {
            //DBG_ERR(" Create path fail src = %d  dest = %d, your structure has wrong size with library \n", PathInfo.src , PathInfo.dest);
            bRet = FALSE;
            goto EXIT;
        }
        else
        {
            MApi_XC_Mux_EnablePath((s16)s16PathId);
        }

        MApi_XC_SetInputSource(INPUT_SOURCE_STORAGE, MAIN_WINDOW);
    }
    else
    {
        MApi_XC_Mux_DeletePath(INPUT_SOURCE_STORAGE, OUTPUT_SCALER_MAIN_WINDOW);
    }
    bRet = TRUE;
EXIT:
    return bRet;
}


MS_BOOL MHalDispVideoLayerEnable(void *pVidLayerCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispDeviceDestroyInstance !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

    if(!_MHalDispSetVideoLayerMux(pstLayerIns, bEnable))
    {
        //DBG_ERR("_MHalDispSetVideoLayerMux Fail !!!\n");
        bRet = FALSE;
        goto UP_LAYER_MUTEX;
    }

    MApi_XC_DisableInputSource(!bEnable, MAIN_WINDOW);
    MApi_XC_GenerateBlackVideo(TRUE, MAIN_WINDOW);
    pstLayerInstance[u32LayerId]->bEnable = bEnable;
    bRet = TRUE;
UP_LAYER_MUTEX:
#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerBind(void *pVidLayerCtx, void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_DevInstance_t* pstDevIns = NULL;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32DevId = 0;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBind !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);
    pstDevIns = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId  = pstDevIns->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif


    list_add_tail(&(pstLayerInstance[u32LayerId]->stLayerNode), &(pstDevInstance[u32DevId]->stBindedLayer));
#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
    up(&(pstDevInstance[u32DevId]->stDevMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_DevInstance_t* pstDevIns = NULL;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32DevId = 0;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerUnBind !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pDevCtx);
    pstDevIns = (MHAL_DISP_DevInstance_t*)pDevCtx;
    u32DevId  = pstDevIns->u32DevId;
    MHAL_DISP_CHECK_INVAILDDEV(u32DevId);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstDevInstance[u32DevId]->stDevMutex));
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstDevInstance[u32DevId]->stDevMutex));
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif
    list_add_tail(&(pstLayerInstance[u32LayerId]->stLayerNode), &(pstDevInstance[u32DevId]->stBindedLayer));
#if(KERNEL_MODE)
    up(&(pstDevInstance[u32DevId]->stDevMutex));
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
    pthread_mutex_unlock(&(pstDevInstance[u32DevId]->stDevMutex));
#endif
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerSetAttr(void *pVidLayerCtx, const MHalDispVideoLayerAttr_t *pstAttr)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    MS_BOOL bHS = TRUE;
    MS_BOOL bXCGen = TRUE;
    MVOP_Handle stMvopHd = { E_MVOP_MODULE_MAIN };
    MVOP_InputSel eInputCfg = MVOP_INPUT_DRAM;
    MVOP_InputCfg stMvopInputCfg;
    MVOP_VidStat stMvopVidSt;
    memset(&stMvopVidSt, 0 , sizeof(MVOP_VidStat));

    //DBG_ERR("MHalDispVideoLayerSetAttr !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pstAttr);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

    if(pstLayerIns->bEnable != TRUE)
    {
        pstLayerInstance[u32LayerId]->stVidLayerDispWin = pstAttr->stVidLayerDispWin;
        pstLayerInstance[u32LayerId]->u32Width = pstAttr->stVidLayerSize.u32Width;
        pstLayerInstance[u32LayerId]->u32Height= pstAttr->stVidLayerSize.u32Height;
        pstLayerInstance[u32LayerId]->ePixFormat = pstAttr->ePixFormat;

        MDrv_MVOP_Enable(FALSE);
        //Set Mvop Input config
        stMvopInputCfg.u16HSize = pstAttr->stVidLayerSize.u32Width;
        stMvopInputCfg.u16VSize = pstAttr->stVidLayerSize.u32Height;
        stMvopInputCfg.u32YOffset = 0;
        stMvopInputCfg.u32UVOffset = 0;
        stMvopInputCfg.bProgressive = TRUE;
        stMvopInputCfg.bUV7bit = FALSE;
        stMvopInputCfg.bField = FALSE;
        stMvopInputCfg.u16StripSize = stMvopInputCfg.u16HSize;

        stMvopInputCfg.bSD = FALSE;
        stMvopInputCfg.bYUV422 = TRUE;
        stMvopInputCfg.bDramRdContd = TRUE;
        stMvopInputCfg.b422pack = TRUE;
        if(E_MVOP_OK != MDrv_MVOP_SetInputCfg(eInputCfg, &stMvopInputCfg))
        {
           //DBG_ERR("Set MVOP input config failed!");
           bRet = FALSE;
           goto UP_LAYER_MUTEX;
        }
        //Set Mvop output Config
        MDrv_MVOP_SetCommand(&stMvopHd, E_MVOP_CMD_SET_HANDSHAKE_MODE, &bHS);
        MDrv_MVOP_SetCommand(&stMvopHd, E_MVOP_CMD_SET_XC_GEN_TIMING, &bXCGen);

        //stMvopVidSt.u16HorSize = pstAttr->stVidLayerDispWin.u16Width;
        //stMvopVidSt.u16VerSize = pstAttr->stVidLayerDispWin.u16Height;
        stMvopVidSt.u16HorSize = pstAttr->stVidLayerSize.u32Width;
        stMvopVidSt.u16VerSize = pstAttr->stVidLayerSize.u32Height;

        stMvopVidSt.u16FrameRate = g_IPanel.DefaultVFreq()*100;
        printf("[%s %d] mvop frame = %d  \n", __FUNCTION__, __LINE__, stMvopVidSt.u16FrameRate);
        stMvopVidSt.u8AspectRate = 0;
        stMvopVidSt.u8Interlace = FALSE;

        if(E_MVOP_OK != MDrv_MVOP_SetOutputCfg(&stMvopVidSt, FALSE))
        {
            //DBG_ERR("Set MVOP output config failed!\n");
            bRet = FALSE;
            goto UP_LAYER_MUTEX;
        }
        MDrv_MVOP_Enable(TRUE); // Enable MVOP, video pass to Scaler

        bRet = TRUE;
    }
    else
    {
        //DBG_ERR("Can't Set Layer Attr when Layer Enabled!!!!\n");
    }
UP_LAYER_MUTEX:
#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif
EXIT:
    return bRet;
}

MS_BOOL MHalDispVideoLayerBufferFire(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBufferFire !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

    if(pstLayerIns->bEnable)
    {
        //Mvop Always Fetch data in next vsync
        bRet = TRUE;
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }
#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

EXIT:
    return bRet;
}

// TODO: 增加check fire ok 的flow
MS_BOOL MHalDispVideoLayerCheckBufferFired(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    MS_BOOL bMatch = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    MS_U64 lumaAddr = 0;
    MS_U64 chromAddr = 0;
    MS_U64 tmplumaAddr = 0;
    MS_U64 tmpchromAddr = 0;
    MS_U32 u32PortId = 0;
    struct list_head *pstPos;
    MHAL_DISP_InputPortInstance_t *pstTmpPort;

    //DBG_ERR("MHalDispVideoLayerCheckBufferFired !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif
    if(pstLayerIns->bEnable)
    {
        list_for_each(pstPos, &(pstLayerIns->stInputPortList))
        {
            pstTmpPort = list_entry(pstPos, MHAL_DISP_InputPortInstance_t, stInputPortNode);
            u32PortId = pstTmpPort->u32PortId;
            MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
            MDrv_MVOP_GetBaseAdd(&lumaAddr, &chromAddr);
            tmplumaAddr = pstTmpPort->aphyCurYUVAddr[0]>>3;
            tmpchromAddr = (pstTmpPort->aphyCurYUVAddr[0] + 16)>>3;
            if((tmplumaAddr == lumaAddr)
                &&(tmpchromAddr == chromAddr)
            )
            {
                bMatch = TRUE;
            }
            else
            {
                //DBG_ERR("%s %d lumaAddr = 0x%llu, chromAddr = 0x%llu, Mached Failed!!!\n", __FUNCTION__, __LINE__, lumaAddr, chromAddr);
            }
        }
        //MsOS_WaitEvent(s32MVOP_Event_id, MHAL_DISP_MVOP_VSYNC, &u32Events, E_OR_CLEAR, 30);
        if(bMatch)
        {
            bRet = TRUE;
        }

        //Mvop Always Fetch data in next vsync
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }
    bRet = TRUE;
#if(KERNEL_MODE)
    up(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerInstance[u32LayerId]->stLayerMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortCreateInstance(const MHAL_DISP_AllocPhyMem_t *pstAlloc, void *pVidLayerCtx, const MS_U32 u32PortId, void **pCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispInputPortCreateInstance !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pstAlloc);
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
#endif

    //Save Dev instance
#if(KERNEL_MODE)
    pstInputPortInstance[u32LayerId][u32PortId] = kmalloc(sizeof(MHAL_DISP_InputPortInstance_t), GFP_KERNEL);
#else
    pstInputPortInstance[u32LayerId][u32PortId] = malloc(sizeof(MHAL_DISP_InputPortInstance_t));
#endif
    memset(pstInputPortInstance[u32LayerId][u32PortId], 0, sizeof(MHAL_DISP_InputPortInstance_t));
#if(KERNEL_MODE)
    init_MUTEX(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex));
    down(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex));
#else
    pthread_mutex_init(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex), NULL);
    pthread_mutex_lock(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex));
#endif


    INIT_LIST_HEAD(&(pstInputPortInstance[u32LayerId][u32PortId]->stInputPortNode));
    pstInputPortInstance[u32LayerId][u32PortId]->u32PortId = u32PortId;
    list_add_tail(&(pstInputPortInstance[u32LayerId][u32PortId]->stInputPortNode), &(pstLayerIns->stInputPortList));
    pstInputPortInstance[u32LayerId][u32PortId]->u32BindedLayerID = pstLayerIns->u32LayerId;
    *pCtx = pstInputPortInstance[u32LayerId][u32PortId];
#if(KERNEL_MODE)
    up(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex));
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstInputPortInstance[u32LayerId][u32PortId]->stPortMutex));
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif
    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortDestroyInstance(const void *pInputPortCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32PortId = 0;
    MS_U32 u32LayerId = 0;
    struct list_head *pstPos;
    struct list_head *pstTmp;
    MHAL_DISP_InputPortInstance_t *pstTmpPort;

    //DBG_ERR("MHalDispInputPortDestroyInstance !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);

    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
    u32LayerId = pstPortIns->u32BindedLayerID;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
    pstLayerIns = pstLayerInstance[u32LayerId];

#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif
    list_for_each_safe(pstPos, pstTmp, &(pstLayerInstance[u32LayerId]->stInputPortList))
    {
        pstTmpPort = list_entry(pstPos, MHAL_DISP_InputPortInstance_t, stInputPortNode);
        if(pstTmpPort->u32PortId == u32PortId)
        {
            list_del(&(pstTmpPort->stInputPortNode));
        }
    }
#if(KERNEL_MODE)
    kfree(pstInputPortInstance[u32LayerId][u32PortId]);
#else
    free(pstInputPortInstance[u32LayerId][u32PortId]);
#endif
    pstInputPortInstance[u32LayerId][u32PortId] = NULL;

#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif

    bRet = TRUE;
EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortFlip(void *pInputPortCtx, MHalDispVideoFrameData_t *pstVideoFrameBuffer)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MS_U32 u32PortId = 0;
    //DBG_ERR("MHalDispInputPortFlip !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pstVideoFrameBuffer);
    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);

#if(KERNEL_MODE)
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif
    // TODO: Tommy: Just support 422 currently.
    // Need add support YUV420 in future.
    if(pstPortIns->bEnable)
    {
        s8 s8PreMiuSelect = -1;
        s8 s8MiuSelect = 0;
        MS_U32 lumaAddr = pstVideoFrameBuffer->PhyAddr[0];
        MS_U32 chromAddr = pstVideoFrameBuffer->PhyAddr[0];
        _phy_to_miu_offset(s8MiuSelect, lumaAddr, lumaAddr);
        //DBG_ERR("%d lumaAddr = 0x%x, chromAddr = 0x%x!!!\n", __LINE__, lumaAddr,chromAddr );

        if(s8MiuSelect != s8PreMiuSelect)
        {
            MDrv_MVOP_MiuSwitch(s8MiuSelect);
            s8PreMiuSelect = s8MiuSelect;
        }
        // TODO: Tommy: need Change mvop address in vblanking or use DS change mvop base address.
        MDrv_MVOP_SetBaseAdd(lumaAddr, chromAddr, TRUE, TRUE);
        pstPortIns->aphyCurYUVAddr[0] = lumaAddr;
        pstPortIns->aphyCurYUVAddr[1] = chromAddr;
        bRet = TRUE;
    }
    else
    {
        //DBG_ERR("Port not enabled!!!\n");
    }
#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortEnable(void *pInputPortCtx, const MS_BOOL bEnable)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32PortId = 0;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispInputPortEnable !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);
    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
    u32LayerId = pstPortIns->u32BindedLayerID;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
    pstLayerIns = pstLayerInstance[u32LayerId];
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif

    if(!(pstLayerIns->bEnable))
    {
        bRet = FALSE;
        goto UP_PORT_MUTEX;
    }

    // TODO: Tommy Check Attr batch, Return directly
    if (pstLayerIns->bEnableBatch == TRUE)
    {
        bRet = TRUE;
        goto UP_PORT_MUTEX;
    }

    if(bEnable)
    {
        MVOP_Timing stMVOPTiming;
        XC_SETWIN_INFO stSetWindowInfo;
        memset(&stSetWindowInfo, 0, sizeof(XC_SETWIN_INFO));
        stSetWindowInfo.enInputSourceType = INPUT_SOURCE_STORAGE;
        stSetWindowInfo.stCapWin.x = 0;
        stSetWindowInfo.stCapWin.y = 0;
        stSetWindowInfo.stCapWin.width = pstLayerIns->u32Width;
        stSetWindowInfo.stCapWin.height = pstLayerIns->u32Height;

        if(MDrv_MVOP_GetOutputTiming(&stMVOPTiming) != E_MVOP_OK)
        {
            //DBG_ERR("MDrv_MVOP_GetOutputTiming Fail !!!\n");
            bRet = FALSE;
            goto UP_PORT_MUTEX;
        }

        stSetWindowInfo.bHDuplicate    = stMVOPTiming.bHDuplicate;
        stSetWindowInfo.u16InputVTotal = stMVOPTiming.u16V_TotalCount;
        stSetWindowInfo.bInterlace     = stMVOPTiming.bInterlace;
        if(stSetWindowInfo.bInterlace)
        {
            stSetWindowInfo.u16InputVFreq = ((u16)stMVOPTiming.u16ExpFrameRate * 2) / 100;
        }
        else
        {
            stSetWindowInfo.u16InputVFreq = (u16)stMVOPTiming.u16ExpFrameRate / 100;
        }
        stSetWindowInfo.stCropWin.x = 0;
        stSetWindowInfo.stCropWin.y = 0;
        stSetWindowInfo.stCropWin.width = pstLayerIns->u32Width;
        stSetWindowInfo.stCropWin.height = pstLayerIns->u32Height;

        stSetWindowInfo.stDispWin.x = pstLayerIns->stVidLayerDispWin.u16X;
        stSetWindowInfo.stDispWin.y = pstLayerIns->stVidLayerDispWin.u16Y;
        stSetWindowInfo.stDispWin.width = pstLayerIns->stVidLayerDispWin.u16Width;
        stSetWindowInfo.stDispWin.height = pstLayerIns->stVidLayerDispWin.u16Height;
        stSetWindowInfo.bHCusScaling = FALSE;
        stSetWindowInfo.bVCusScaling = FALSE;
        if(MApi_XC_SetWindow(&stSetWindowInfo, sizeof(XC_SETWIN_INFO), MAIN_WINDOW) == FALSE)
        {
            //DBG_ERR("MApi_XC_SetWindow failed!!!\n");
            bRet = FALSE;
            goto UP_PORT_MUTEX;
        }
        pstPortIns->bEnable = TRUE;
        MApi_XC_GenerateBlackVideo(FALSE, MAIN_WINDOW);
    }
    else
    {
        MApi_XC_GenerateBlackVideo(TRUE, MAIN_WINDOW);
        pstPortIns->bEnable = FALSE;
    }
    bRet = TRUE;
UP_PORT_MUTEX:
#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispInputCheckDispWin(MHAL_DISP_InputPortInstance_t *pstPortIns, MHAL_DISP_LayerInstance_t *pstLayerIns)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_CHECK_NULLPOINTER(pstPortIns);
    MHAL_DISP_CHECK_NULLPOINTER(pstLayerIns);
    if((pstPortIns->stDispWin.u16X <= pstLayerIns->stVidLayerDispWin.u16X)
        || (pstPortIns->stDispWin.u16Y <= pstLayerIns->stVidLayerDispWin.u16Y)
        || (pstPortIns->stDispWin.u16Width <= pstLayerIns->stVidLayerDispWin.u16Width)
        || (pstPortIns->stDispWin.u16Height <= pstLayerIns->stVidLayerDispWin.u16Height)
        )
    {
        bRet = TRUE;
    }
EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortSetAttr(void *pInputPortCtx, const MHalDispInputPortAttr_t *pstAttr)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32PortId = 0;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispInputPortEnable !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pstAttr);
    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
    u32LayerId = pstPortIns->u32BindedLayerID;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
    pstLayerIns = pstLayerInstance[u32LayerId];
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif


    if(MHalDispInputCheckDispWin(pstPortIns, pstLayerIns) != TRUE)
    {
        //DBG_ERR("MHalDispInputCheckDispWin failed!!!\n");
        bRet = FALSE;
        goto UP_PORT_MUTEX;
    }
    pstInputPortInstance[u32LayerId][u32PortId]->stDispWin = pstAttr->stDispWin;
    bRet = TRUE;
UP_PORT_MUTEX:
#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortShow(void *pInputPortCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MS_U32 u32PortId = 0;
    //DBG_ERR("MHalDispInputPortShow !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);
    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
#if(KERNEL_MODE)
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif
    MApi_XC_GenerateBlackVideo(FALSE, MAIN_WINDOW);
    bRet = TRUE;
#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
#endif

EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortHide(void *pInputPortCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_InputPortInstance_t* pstPortIns = NULL;
    MS_U32 u32PortId = 0;
    //DBG_ERR("MHalDispInputPortHide !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pInputPortCtx);
    pstPortIns = (MHAL_DISP_InputPortInstance_t*)pInputPortCtx;
    u32PortId = pstPortIns->u32PortId;
    MHAL_DISP_CHECK_INVAILDINPUTPORT(u32PortId);
#if(KERNEL_MODE)
    down(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_lock(&(pstPortIns->stPortMutex));
#endif

    MApi_XC_GenerateBlackVideo(TRUE, MAIN_WINDOW);
    bRet = TRUE;
#if(KERNEL_MODE)
    up(&(pstPortIns->stPortMutex));
#else
    pthread_mutex_unlock(&(pstPortIns->stPortMutex));
#endif

EXIT:
    return bRet;
}

// Video layer: Set/Get compress
MS_BOOL MHalDispVideoLayerSetCompress(void *pVidLayerCtx, const MHalDispCompressAttr_t* pstCompressAttr)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBufferFire !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);
    MHAL_DISP_CHECK_NULLPOINTER(pstCompressAttr);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
#endif

    if(pstLayerIns->bEnable)
    {
        //DBG_ERR("Not Support!!!\n");
        bRet = TRUE;
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }
#if(KERNEL_MODE)
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif

EXIT:
    return bRet;
}

// Video layer: Set/Get display priority
MS_BOOL MHalDispVideoLayerSetPriority(void *pVidLayerCtx, const MS_U32 u32Priority)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBufferFire !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
#endif
    if(pstLayerIns->bEnable)
    {
        //DBG_ERR("Not Support!!!\n");
        bRet = TRUE;
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }
#if(KERNEL_MODE)
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif
EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortAttrBegin(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBufferFire !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);
#if(KERNEL_MODE)
    down(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_lock(&(pstLayerIns->stLayerMutex));
#endif
    if(pstLayerIns->bEnable)
    {
        pstLayerIns->bEnableBatch = TRUE;
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }
    bRet = TRUE;
#if(KERNEL_MODE)
    up(&(pstLayerIns->stLayerMutex));
#else
    pthread_mutex_unlock(&(pstLayerIns->stLayerMutex));
#endif
EXIT:
    return bRet;
}

MS_BOOL MHalDispInputPortAttrEnd(void *pVidLayerCtx)
{
    MS_BOOL bRet = FALSE;
    MHAL_DISP_LayerInstance_t* pstLayerIns = NULL;
    MS_U32 u32LayerId = 0;
    //DBG_ERR("MHalDispVideoLayerBufferFire !!!\n");
    MHAL_DISP_CHECK_NULLPOINTER(pVidLayerCtx);

    pstLayerIns = (MHAL_DISP_LayerInstance_t*)pVidLayerCtx;
    u32LayerId = pstLayerIns->u32LayerId;
    MHAL_DISP_CHECK_INVAILDLAYER(u32LayerId);

    if(pstLayerIns->bEnable)
    {
        if(pstLayerIns->bEnableBatch == TRUE)
        {
            struct list_head *pstPos;
            MHAL_DISP_InputPortInstance_t *pstTmpPort;
            list_for_each(pstPos, &(pstLayerInstance[u32LayerId]->stInputPortList))
            {
                pstTmpPort = list_entry(pstPos, MHAL_DISP_InputPortInstance_t, stInputPortNode);
                if(!MHalDispInputPortEnable(pstTmpPort, TRUE))
                {
                    //DBG_ERR("MHalDispInputPortEnable Fail!!!\n");
                    bRet = FALSE;
                    continue;
                }
                bRet = TRUE;
            }

        }
    }
    else
    {
        //DBG_ERR("Layer not enabled!!!\n");
    }

EXIT:
    return bRet;

}

MS_BOOL MHalDispSetDbgLevel(void)
{
    //DBG_ERR("MHalDispSetDbgLevel !!!\n");
    MApi_XC_SetDbgLevel(0x83);
    return TRUE;
}

MS_BOOL MHalDispSetPanelParams(void)
{
    //DBG_ERR("MHalDispSetDbgLevel !!!\n");
    MApi_XC_SetDbgLevel(0x83);
    return TRUE;
}

MS_BOOL MHalDispDeviceSetCvbsParam(void *pDevCtx, const MHalDispCvbsParam_t *pstCvbsInfo)
{
    return TRUE;
}

MS_BOOL MHalDispDeviceSetHdmiParam(void *pDevCtx, const MHalDispHdmiParam_t *pstHdmiInfo)
{
    return TRUE;
}

MS_BOOL MHalDispDeviceSetVgaParam(void *pDevCtx, const MHalDispVgaParam_t *pstVgaInfo)
{
    return TRUE;
}

 MS_BOOL MHalDispDeviceAttach(const void *pSrcDevCtx, const void *pDstDevCtx)

{
    return TRUE;
}

MS_BOOL MHalDispDeviceDetach(const void *pSrcDevCtx, const void *pDstDevCtx)
{
    return TRUE;
}

