//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
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
////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#else
#include <string.h>
#endif

#if defined(REDLION_LINUX_KERNEL_ENVI)
#include "drvHVD_Common.h"
#else
#include "MsCommon.h"
#endif

#include "MsOS.h"
#include "asmCPU.h"


#if (!defined(MSOS_TYPE_NUTTX) && !defined(MSOS_TYPE_OPTEE)) || defined(SUPPORT_X_MODEL_FEATURE)

// Internal Definition
#include "regVPU_EX.h"
#include "halVPU_EX.h"
#include "halCHIP.h"
#include "drvSYS.h"
#if defined(VDEC3)
#include "../../drv/hvd_v3/drvHVD_def.h"
#include "../hvd_v3/fwHVD_if.h"
#include "../mvd_v3/mvd4_interface.h"
#include "../../hal/hvd_v3/halHVD_EX.h"
#else
#include "../../drv/hvd_v3/drvHVD_def.h"
#include "../hvd_v3/fwHVD_if.h"
#include "../mvd_v3/mvd4_interface.h"
#endif
#include "controller.h"

#if (ENABLE_DECOMPRESS_FUNCTION == TRUE)
#include "ms_decompress.h"
#include "ms_decompress_priv.h"
#endif
//#include "../../drv/mbx/apiMBX_St.h"
//#include "../../drv/mbx/apiMBX.h"
#include <drvMBX.h>
#include <apiMBX.h>

#if VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM
#include "drvSERFLASH.h"
#define HVD_FLASHcpy(DESTADDR, SRCADDR, LEN, Flag)  MDrv_SERFLASH_CopyHnd((MS_PHY)(SRCADDR), (MS_PHY)(DESTADDR), (LEN), (Flag), SPIDMA_OPCFG_DEF)
#endif



#ifdef MSOS_TYPE_LINUX_KERNEL
#define VPRINTF printk
#elif defined( MSOS_TYPE_ECOS)
#define VPRINTF diag_printf
#else
#ifndef ANDROID
#define VPRINTF printf
#else
#include <sys/mman.h>
#include <cutils/ashmem.h>
#include <cutils/log.h>
#define VPRINTF ALOGD
#endif
#endif

#ifdef CONFIG_MSTAR_CLKM
#include "drvCLKM.h"
#endif
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define VPU_CTL_INTERFACE_VER   0x00000001  //the interface version of VPU driver

#define VPU_MIU1BASE_ADDR    0x40000000UL   //Notice: this define must be comfirm with designer
#ifdef VDEC3
#define MAX_EVD_BBU_COUNT 4 // This definition is chip-dependent.
#define MAX_HVD_BBU_COUNT 4 // The Chip after Monaco(included) have two EVD BBU, must check this definition when bring up
#define MAX_MVD_SLQ_COUNT 4
#define MAX_SUPPORT_DECODER_NUM 32
#else
#define MAX_SUPPORT_DECODER_NUM 2
#endif
typedef enum
{
    E_VDEC_EX_REE_TO_TEE_MBX_MSG_NULL,
    E_VDEC_EX_REE_TO_TEE_MBX_MSG_FW_LoadCode,
    E_VDEC_EX_REE_TO_TEE_MBX_MSG_GETSHMBASEADDR,
} VDEC_REE_TO_TEE_MBX_MSG_TYPE;


typedef enum
{
    E_VDEC_EX_TEE_TO_REE_MBX_MSG_NULL,
    E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_INVALID,
    E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_NO_TEE,
    E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS,
    E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_FAIL
} VDEC_TEE_TO_REE_MBX_ACK_TYPE;

typedef enum
{
    E_VPU_UART_CTRL_DISABLE = BIT(4),
    E_VPU_UART_CTRL_ERR     = BIT(0),
    E_VPU_UART_CTRL_INFO    = BIT(1),
    E_VPU_UART_CTRL_DBG     = BIT(2),
    E_VPU_UART_CTRL_FW      = BIT(3),
    E_VPU_UART_CTRL_MUST    = BIT(4),
    E_VPU_UART_CTRL_TRACE   = BIT(5),
} VPU_EX_UartCtrl;

typedef struct
{
    HAL_VPU_StreamId eStreamId;
    VPU_EX_DecoderType eDecodertype;
} VPU_EX_Stream;

typedef struct
{
    MS_BOOL bSTCSetMode;
    MS_U32  u32STCIndex;
} VPU_EX_STC;

#define VPU_MSG_ERR(format, args...)                \
    do                                              \
    {                                               \
        if (u32VpuUartCtrl & E_VPU_UART_CTRL_ERR)  \
        {                                           \
            VPRINTF("[VPU][ERR]%s:", __FUNCTION__);  \
            VPRINTF(format, ##args);                 \
        }                                           \
    } while (0)

#define VPU_MSG_DBG(format, args...)                \
    do                                              \
    {                                               \
        if (u32VpuUartCtrl & E_VPU_UART_CTRL_DBG)  \
        {                                           \
            VPRINTF("[VPU][DBG]%s:", __FUNCTION__);  \
            VPRINTF(format, ##args);                 \
        }                                           \
    } while (0)

#define VPU_MSG_INFO(format, args...)               \
    do                                              \
    {                                               \
        if (u32VpuUartCtrl & E_VPU_UART_CTRL_INFO) \
        {                                           \
            VPRINTF("[VPU][INF]%s:", __FUNCTION__);  \
            VPRINTF(format, ##args);                 \
        }                                           \
    } while (0)

//------------------------------ MIU SETTINGS ----------------------------------
//#ifdef EVDR2
#define _MaskMiuReq_VPU_D_RW(m)     _VPU_WriteRegBit(MIU0_REG_RQ0_MASK, m, BIT(5))
#define _MaskMiuReq_VPU_Q_RW(m)     _VPU_WriteRegBit(MIU0_REG_RQ0_MASK, m, BIT(5))
#define _MaskMiuReq_VPU_I_R(m)      _VPU_WriteRegBit(MIU0_REG_RQ0_MASK, m, BIT(3))

#define _MaskMiu1Req_VPU_D_RW(m)    _VPU_WriteRegBit(MIU1_REG_RQ0_MASK, m, BIT(5))
#define _MaskMiu1Req_VPU_Q_RW(m)    _VPU_WriteRegBit(MIU1_REG_RQ0_MASK, m, BIT(5))
#define _MaskMiu1Req_VPU_I_R(m)     _VPU_WriteRegBit(MIU1_REG_RQ0_MASK, m, BIT(3))



#define VPU_D_RW_ON_MIU0            (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(5)) == 0) )
#define VPU_Q_RW_ON_MIU0            (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(5)) == 0) )
#define VPU_I_R_ON_MIU0             (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(3)) == 0) ) //g03
#define VPU_D_RW_ON_MIU1            (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(5)) == BIT(5)) )
#define VPU_Q_RW_ON_MIU1            (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(5)) == BIT(5)) )
#define VPU_I_R_ON_MIU1             (((_VPU_ReadByte(MIU0_REG_SEL0) & BIT(3)) == BIT(3)) ) //g03

//#endif



#define _VPU_MIU_SetReqMask(miu_clients, mask)  \
    do                                          \
    {                                           \
        if (miu_clients##_ON_MIU0 == 1)   \
        {                                       \
            _MaskMiuReq_##miu_clients(mask);     \
        }                                       \
        else                                     \
        {                                       \
            if (miu_clients##_ON_MIU1 == 1)   \
            {                                       \
                _MaskMiu1Req_##miu_clients(mask);    \
            }                                       \
            \
        }                                       \
    } while(0)




#if ENABLE_VPU_MUTEX_PROTECTION
MS_S32 s32VPUMutexID = -1;
MS_U8 _u8VPU_Mutex[] = { "VPU_Mutex" };

#define _HAL_VPU_MutexCreate()  \
    if (s32VPUMutexID < 0)      \
    {                           \
        s32VPUMutexID = MsOS_CreateMutex(E_MSOS_FIFO,(char*)_u8VPU_Mutex, MSOS_PROCESS_SHARED); \
    }

#define _HAL_VPU_MutexDelete()              \
    if (s32VPUMutexID >= 0)                 \
    {                                       \
        MsOS_DeleteMutex(s32VPUMutexID);    \
        s32VPUMutexID = -1;                 \
    }

#define _HAL_VPU_Entry()                                                \
    if (s32VPUMutexID >= 0)                                             \
    {                                                                   \
        if (!MsOS_ObtainMutex(s32VPUMutexID, VPU_DEFAULT_MUTEX_TIMEOUT))       \
        {                                                               \
            VPRINTF("[HAL VPU][%06d] Mutex taking timeout\n", __LINE__); \
        }                                                               \
    }

#define _HAL_VPU_Return(_ret)                   \
    {                                           \
        if (s32VPUMutexID >= 0)                 \
        {                                       \
            MsOS_ReleaseMutex(s32VPUMutexID);   \
        }                                       \
        return _ret;                            \
    }

#define _HAL_VPU_Release()                      \
    {                                           \
        if (s32VPUMutexID >= 0)                 \
        {                                       \
            MsOS_ReleaseMutex(s32VPUMutexID);   \
        }                                       \
    }
#else
#define _HAL_VPU_MutexCreate()
#define _HAL_VPU_MutexDelete()
#define _HAL_VPU_Entry()
#define _HAL_VPU_Return(_ret)       {return _ret;}
#define _HAL_VPU_Release()
#endif

#define VPU_FW_MEM_OFFSET   FW_TASK_SIZE //org: 0x100000UL  // 1M

#ifdef CHIP_I2_FPGA
#define VPU_CMD_TIMEOUT     20000 // 10 sec or FW debug mode
#else
#define VPU_CMD_TIMEOUT     10000   //1000 // 10 sec for FW R2 debug
//#define VPU_CMD_TIMEOUT    1000 // 1 sec
#endif

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct _VPU_HWInitFunc
{
    MS_BOOL (*pfMVDHW_Init)(void);
    MS_BOOL (*pfMVDHW_Deinit)(void);
    MS_BOOL (*pfHVDHW_Init)(MS_U32 u32Arg);
    MS_BOOL (*pfHVDHW_Deinit)(void);
} VPU_HWInitFunc;

typedef struct
{
    MS_U32  u32ApiHW_Version;   //<Version of current structure>
    MS_U16  u16ApiHW_Length;    //<Length of this structure>

    MS_U8   u8Cap_Support_Decoder_Num;

    MS_BOOL bCap_Support_MPEG2;
    MS_BOOL bCap_Support_H263;
    MS_BOOL bCap_Support_MPEG4;
    MS_BOOL bCap_Support_DIVX311;
    MS_BOOL bCap_Support_DIVX412;
    MS_BOOL bCap_Support_FLV;
    MS_BOOL bCap_Support_VC1ADV;
    MS_BOOL bCap_Support_VC1MAIN;

    MS_BOOL bCap_Support_RV8;
    MS_BOOL bCap_Support_RV9;
    MS_BOOL bCap_Support_H264;
    MS_BOOL bCap_Support_AVS;
    MS_BOOL bCap_Support_AVS_PLUS;
    MS_BOOL bCap_Support_MJPEG;
    MS_BOOL bCap_Support_MVC;
    MS_BOOL bCap_Support_VP8;
    MS_BOOL bCap_Support_VP9;
    MS_BOOL bCap_Support_HEVC;

    /*New HW Cap and Feature add in struct at the end*/
} VDEC_HwCap;

//-------------------------------------------------------------------------------------------------
//  Local Functions Prototype
//-------------------------------------------------------------------------------------------------
static MS_BOOL          _VPU_EX_LoadVLCTable(VPU_EX_VLCTblCfg *pVlcCfg, MS_U8 u8FwSrcType);
MS_U8                   _VPU_EX_GetOffsetIdx(MS_U32 u32Id);
static HVD_User_Cmd     _VPU_EX_MapCtrlCmd(VPU_EX_TaskInfo *pTaskInfo);

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern HVD_Return HAL_HVD_EX_SetCmd(MS_U32 u32Id, HVD_User_Cmd eUsrCmd, MS_U32 u32CmdArg);
extern MS_BOOL HAL_MVD_InitHW(VPU_EX_SourceType SourceType, VPU_EX_DecoderType eDecType);
extern MS_BOOL HAL_MVD_DeinitHW(VPU_EX_SourceType SourceType, VPU_EX_DecoderType eDecType);
extern MS_BOOL HAL_HVD_EX_InitHW(MS_U32 u32Id, VPU_EX_DecoderType DecoderType);
extern MS_BOOL HAL_HVD_EX_DeinitHW(MS_U32 u32Id);
extern MS_BOOL HAL_EVD_EX_DeinitHW(MS_U32 u32Id);
extern MS_VIRT HAL_HVD_EX_GetShmAddr(MS_U32 u32Id);
#if SUPPORT_G2VP9 && defined(VDEC3)
extern MS_BOOL HAL_VP9_EX_DeinitHW(void);
#endif
#if defined (__aeon__)
static MS_VIRT u32VPURegOSBase = 0xA0000000UL;
#else
static MS_VIRT u32VPURegOSBase = 0xBF200000UL;
#endif

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#if 0

static MS_BOOL _bVPUPowered = FALSE;
static MS_BOOL _bVPURsted = FALSE;
static MS_BOOL _bVPUSingleMode = FALSE;
static VPU_EX_DecModCfg _stVPUDecMode;

static MS_U8 u8TaskCnt = 0;

static MS_U32 u32VpuUartCtrl = (E_VPU_UART_CTRL_ERR | E_VPU_UART_CTRL_MUST);

//Notice: this function must be consistent with _VPU_EX_GetOffsetIdx()
static VPU_EX_Stream _stVPUStream[] =
{
    {E_HAL_VPU_MAIN_STREAM0, E_VPU_EX_DECODER_NONE},
    {E_HAL_VPU_SUB_STREAM0, E_VPU_EX_DECODER_NONE},
};
static VPU_HWInitFunc stHWInitFunc =
{
    &HAL_MVD_InitHW,
    &HAL_MVD_DeinitHW,
    &HAL_HVD_EX_InitHW,
    &HAL_HVD_EX_DeinitHW,
};

#endif

#if VPU_ENABLE_EMBEDDED_FW_BINARY
static const MS_U8 u8HVD_FW_Binary[] =
{
#include "fwVPU.dat"
};

#if HVD_ENABLE_RV_FEATURE
static const MS_U8 u8HVD_VLC_Binary[] =
{
#include "fwVPU_VLC.dat"
};
#endif
#endif


#ifdef VDEC3
typedef struct
{
    MS_BOOL bTSP;
    MS_U8   u8RegSetting;  //NAL_TBL: BIT(0), ES_BUFFER: BIT(1)
    MS_U32  u32Used;
} BBU_STATE;

typedef struct
{
    MS_BOOL bTSP;
    MS_BOOL bUsedbyMVD;
    MS_U32 u32Used;
} SLQ_STATE;
#endif

typedef struct
{
    MS_BOOL _bVPUPowered;
    MS_BOOL _bVPURsted;
    MS_BOOL _bVPUSingleMode;
    VPU_EX_DecModCfg _stVPUDecMode;
    MS_U8 u8TaskCnt;
    //Notice: this function must be consistent with _VPU_EX_GetOffsetIdx()
#ifdef VDEC3
    VPU_EX_Stream _stVPUStream[MAX_SUPPORT_DECODER_NUM];
#else
    VPU_EX_Stream _stVPUStream[2];
#endif

    VPU_HWInitFunc stHWInitFunc;

    MS_BOOL bVpuExReloadFW;
    MS_BOOL bVpuExLoadFWRlt;
    MS_VIRT  u32VPUSHMAddr;    //PA
    MS_BOOL bEnableVPUSecureMode;

    MS_VIRT  u32FWShareInfoAddr[MAX_SUPPORT_DECODER_NUM];
    MS_BOOL bEnableDymanicFBMode;
    MS_PHY u32DynamicFBAddress;
    MS_U32 u32DynamicFBSize;
#ifdef VDEC3
    MS_VIRT  u32FWCodeAddr;
    MS_VIRT  u32BitstreamAddress[MAX_SUPPORT_DECODER_NUM];

    BBU_STATE stHVD_BBU_STATE[MAX_HVD_BBU_COUNT];
    BBU_STATE stEVD_BBU_STATE[MAX_EVD_BBU_COUNT];
    SLQ_STATE stMVD_SLQ_STATE[MAX_MVD_SLQ_COUNT];

    MS_U8 u8HALId[MAX_SUPPORT_DECODER_NUM];
#endif
    MS_U8 u8ForceRst;
    VPU_EX_STC _stVPUSTCMode[MAX_SUPPORT_DECODER_NUM];
} VPU_Hal_CTX;

//global variables
VPU_Hal_CTX* pVPUHalContext = NULL;
VPU_Hal_CTX gVPUHalContext;
MS_U32 u32VpuUartCtrl = (E_VPU_UART_CTRL_ERR | E_VPU_UART_CTRL_MUST);
MS_BOOL bVPUMbxInitFlag = 0;
MS_U8 u8VPUMbxMsgClass = 0;
MBX_Msg VPUReeToTeeMbxMsg;
MBX_Msg VPUTeeToReeMbxMsg;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

static MS_BOOL _VPU_EX_LoadVLCTable(VPU_EX_VLCTblCfg *pVlcCfg, MS_U8 u8FwSrcType)
{
#if HVD_ENABLE_RV_FEATURE
    if (E_HVD_FW_INPUT_SOURCE_FLASH == u8FwSrcType)
    {
#if VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM
        VPU_MSG_DBG("Load VLC outF2D: dest:0x%lx source:%lx size:%lx\n",
                    pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize);

        if (pVlcCfg->u32BinSize)
        {
            SPIDMA_Dev cpyflag = E_SPIDMA_DEV_MIU1;

            MS_U32 u32Start;
            MS_U32 u32StartOffset;
            MS_U8  u8MiuSel;

            // Get MIU selection and offset from physical address = 0x30000000
            _phy_to_miu_offset(u8MiuSel, u32StartOffset, pVlcCfg->u32FrameBufAddr);


            if(u8MiuSel == E_CHIP_MIU_0)
                cpyflag = E_SPIDMA_DEV_MIU0;
            else if(u8MiuSel == E_CHIP_MIU_1)
                cpyflag = E_SPIDMA_DEV_MIU1;
            else if(u8MiuSel == E_CHIP_MIU_2)
                cpyflag = E_SPIDMA_DEV_MIU2;

            if (!HVD_FLASHcpy(MsOS_VA2PA(pVlcCfg->u32DstAddr), MsOS_VA2PA(pVlcCfg->u32BinAddr), pVlcCfg->u32BinSize, cpyflag))
            {
                VPU_MSG_ERR("HVD_BDMAcpy VLC table Flash 2 DRAM failed: dest:0x%lx src:0x%lx size:0x%lx flag:%lu\n",
                            pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize, (MS_U32) cpyflag);

                return FALSE;
            }
        }
        else
        {
            VPU_MSG_ERR("During copy VLC from Flash to Dram, the source size of FW is zero\n");
            return FALSE;
        }
#else
        VPU_MSG_ERR("driver not enable to use BDMA copy VLC from flash 2 sdram.\n");
        return FALSE;
#endif
    }
    else
    {
        if (E_HVD_FW_INPUT_SOURCE_DRAM == u8FwSrcType)
        {
            if ((pVlcCfg->u32BinAddr != 0) && (pVlcCfg->u32BinSize != 0))
            {
                VPU_MSG_INFO("Load VLC outD2D: dest:0x%lx source:%lx size:%lx\n",
                             (unsigned long)pVlcCfg->u32DstAddr, (unsigned long)pVlcCfg->u32BinAddr, (unsigned long)pVlcCfg->u32BinSize);

#if HVD_ENABLE_BDMA_2_BITSTREAMBUF
                BDMA_Result bdmaRlt;
                MS_VIRT u32DstAdd = 0, u32SrcAdd = 0, u32tabsize = 0;

                u32DstAdd   = pVlcCfg->u32FrameBufAddr + pVlcCfg->u32VLCTableOffset;
                u32SrcAdd   = pVlcCfg->u32BinAddr;
                u32tabsize  = pVlcCfg->u32BinSize;
                //bdmaRlt = MDrv_BDMA_MemCopy(u32SrcAdd, u32DstAdd, SLQ_TBL_SIZE);
                MsOS_FlushMemory();
                bdmaRlt = HVD_dmacpy(u32DstAdd, u32SrcAdd, u32tabsize);

                if (E_BDMA_OK != bdmaRlt)
                {
                    VPU_MSG_ERR("MDrv_BDMA_MemCopy fail in %s(), ret=%x!\n", __FUNCTION__, bdmaRlt);
                }
#else
                HVD_memcpy(pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize);
#endif
            }
            else
            {
                VPU_MSG_ERR
                ("During copy VLC from out Dram to Dram, the source size or virtual address of VLC is zero\n");
                return FALSE;
            }
        }
        else
        {
#if VPU_ENABLE_EMBEDDED_FW_BINARY
#ifdef HVD_CACHE_TO_UNCACHE_CONVERT
            MS_U8 *pu8HVD_VLC_Binary;

            pu8HVD_VLC_Binary = (MS_U8 *) ((MS_U32) u8HVD_VLC_Binary | 0xA0000000);

            VPU_MSG_DBG("Load VLC inD2D: dest:0x%lx source:%lx size:%lx\n",
                        pVlcCfg->u32FrameBufAddr + pVlcCfg->u32VLCTableOffset, ((MS_U32) pu8HVD_VLC_Binary),
                        (MS_U32) sizeof(u8HVD_VLC_Binary));

            HVD_memcpy((void *) (pVlcCfg->u32FrameBufAddr + pVlcCfg->u32VLCTableOffset),
                       (void *) ((MS_U32) pu8HVD_VLC_Binary), sizeof(u8HVD_VLC_Binary));
#else
            VPU_MSG_INFO("Load VLC inD2D: dest:0x%lx source:%lx size:%x\n",
                         (unsigned long)MsOS_VA2PA(pVlcCfg->u32DstAddr), (unsigned long)u8HVD_VLC_Binary,
                         (MS_U32) sizeof(u8HVD_VLC_Binary));

            HVD_memcpy(pVlcCfg->u32DstAddr, ((unsigned long)u8HVD_VLC_Binary), sizeof(u8HVD_VLC_Binary));
#endif
#else
            VPU_MSG_ERR("driver not enable to use embedded VLC binary.\n");
            return FALSE;
#endif
        }
    }
#endif

    return TRUE;
}

//Notice: this function must be consistent with _stVPUStream[]
MS_U8 _VPU_EX_GetOffsetIdx(MS_U32 u32Id)
{
    MS_U8 u8OffsetIdx = 0;
    MS_U8 u8VSidBaseMask = 0xF0;
    HAL_VPU_StreamId eVSidBase = (HAL_VPU_StreamId)(u32Id & u8VSidBaseMask);

    switch ((MS_U32)eVSidBase)
    {
        case E_HAL_VPU_MAIN_STREAM_BASE:
        {
            u8OffsetIdx = 0;
            break;
        }
        case E_HAL_VPU_SUB_STREAM_BASE:
        {
            u8OffsetIdx = 1;
            break;
        }
        case E_HAL_VPU_MVC_STREAM_BASE:
        {
            u8OffsetIdx = 0;
            break;
        }
#ifdef VDEC3
        case E_HAL_VPU_N_STREAM_BASE:
        {
            u8OffsetIdx = u32Id & 0x0F;
            break;
        }
        case E_HAL_VPU_N_STREAM_BASE+0x10:
        {
            u8OffsetIdx = u32Id & 0x3F;
            break;
        }
        case E_HAL_VPU_N_STREAM_BASE+0x20:
        {
            u8OffsetIdx = u32Id & 0x3F;
            break;
        }
        case E_HAL_VPU_N_STREAM_BASE+0x30:
        {
            u8OffsetIdx = u32Id & 0x3F;
            break;
        }
#endif
        default:
        {
            u8OffsetIdx = 0;
            break;
        }
    }

    /*
    VPU_MSG_DBG("u32Id=0x%lx, eVSidBase=0x%x, u8OffsetIdx=0x%x\n",
        u32Id, eVSidBase, u8OffsetIdx);
        */
    return u8OffsetIdx;
}

MS_U8 HAL_VPU_EX_GetOffsetIdx(MS_U32 u32Id)
{
    return _VPU_EX_GetOffsetIdx(u32Id);
}

static void _VPU_EX_Context_Init(void)
{
#ifdef VDEC3
    MS_U8 i;

    for (i = 0; i < MAX_SUPPORT_DECODER_NUM; i++)
    {
        pVPUHalContext->_stVPUStream[i].eStreamId = E_HAL_VPU_N_STREAM0 + i;
        pVPUHalContext->u32FWShareInfoAddr[i] = 0xFFFFFFFFUL;
    }

    for (i = 0; i < MAX_HVD_BBU_COUNT; i++)
    {
        pVPUHalContext->stHVD_BBU_STATE[i].bTSP = FALSE;
        pVPUHalContext->stHVD_BBU_STATE[i].u32Used = 0;
    }

    for (i = 0; i < MAX_EVD_BBU_COUNT; i++)
    {
        pVPUHalContext->stEVD_BBU_STATE[i].bTSP = FALSE;
        pVPUHalContext->stEVD_BBU_STATE[i].u32Used = 0;
    }

    for (i = 0; i < MAX_MVD_SLQ_COUNT; i++)
    {
        pVPUHalContext->stMVD_SLQ_STATE[i].bTSP = FALSE;
        pVPUHalContext->stMVD_SLQ_STATE[i].bUsedbyMVD = FALSE;
        pVPUHalContext->stMVD_SLQ_STATE[i].u32Used = 0;
    }
#else
    pVPUHalContext->_stVPUStream[0].eStreamId = E_HAL_VPU_MAIN_STREAM0;
    pVPUHalContext->_stVPUStream[1].eStreamId = E_HAL_VPU_SUB_STREAM0;
#endif
    pVPUHalContext->bVpuExReloadFW = TRUE;
    pVPUHalContext->u8ForceRst = 0;
}

static HVD_User_Cmd _VPU_EX_MapCtrlCmd(VPU_EX_TaskInfo *pTaskInfo)
{
    HVD_User_Cmd eCmd = E_HVD_CMD_INVALID_CMD;
    MS_U8 u8OffsetIdx = 0;

    if (NULL == pTaskInfo)
    {
        return eCmd;
    }

    u8OffsetIdx = _VPU_EX_GetOffsetIdx(pTaskInfo->u32Id);

    VPU_MSG_INFO("input TaskInfo u32Id=0x%08x eVpuId=0x%x src=0x%x dec=0x%x\n",
                 pTaskInfo->u32Id, pTaskInfo->eVpuId, pTaskInfo->eSrcType, pTaskInfo->eDecType);

#ifdef VDEC3
    if (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
    {
        if (E_VPU_EX_INPUT_TSP == pTaskInfo->eSrcType)
        {
            eCmd = E_NST_CMD_TASK_MVD_TSP;
        }
        else if (E_VPU_EX_INPUT_FILE == pTaskInfo->eSrcType)
        {
            eCmd = E_NST_CMD_TASK_MVD_SLQ;
        }
    }
#else
    if (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
    {
        if (E_VPU_EX_INPUT_TSP == pTaskInfo->eSrcType)
        {
            eCmd = (u8OffsetIdx == 0) ? E_DUAL_CMD_TASK0_MVD_TSP : E_DUAL_CMD_TASK1_MVD_TSP;
        }
        else if (E_VPU_EX_INPUT_FILE == pTaskInfo->eSrcType)
        {
            eCmd = (u8OffsetIdx == 0) ? E_DUAL_CMD_TASK0_MVD_SLQ : E_DUAL_CMD_TASK1_MVD_SLQ;
        }
    }
#endif
#ifdef VDEC3
#if SUPPORT_G2VP9
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_G2VP9 == pTaskInfo->eDecType)
#else
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
#endif
    {
        if (E_VPU_EX_INPUT_TSP == pTaskInfo->eSrcType)
        {
            eCmd = E_NST_CMD_TASK_HVD_TSP;
        }
        else if (E_VPU_EX_INPUT_FILE == pTaskInfo->eSrcType)
        {
            eCmd = E_NST_CMD_TASK_HVD_BBU;
        }
    }
#else
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
    {
        if (E_VPU_EX_INPUT_TSP == pTaskInfo->eSrcType)
        {
            eCmd = (u8OffsetIdx == 0) ? E_DUAL_CMD_TASK0_HVD_TSP : E_DUAL_CMD_TASK1_HVD_TSP;
        }
        else if (E_VPU_EX_INPUT_FILE == pTaskInfo->eSrcType)
        {
            eCmd = (u8OffsetIdx == 0) ? E_DUAL_CMD_TASK0_HVD_BBU : E_DUAL_CMD_TASK1_HVD_BBU;
        }
    }
#endif

    VPU_MSG_INFO("output: eCmd=0x%x offsetIdx=0x%x\n", eCmd, u8OffsetIdx);
    return eCmd;
}

static MS_BOOL _VPU_EX_InitHW(VPU_EX_TaskInfo *pTaskInfo)
{
    if (!pTaskInfo)
    {
        VPU_MSG_ERR("null input\n");
        return FALSE;
    }

    //Check if we need to init MVD HW
    if ((E_VPU_EX_INPUT_TSP == pTaskInfo->eSrcType) ||
            (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType))
    {
        //Init HW
        if (FALSE == HAL_VPU_EX_MVDInUsed())
        {
            if (TRUE != HAL_MVD_InitHW(pTaskInfo->eSrcType, pTaskInfo->eDecType))
            {
                VPU_MSG_ERR("(%d):HAL_MVD_InitHW failed\n", __LINE__);
                return FALSE;
            }
        }
        else
        {
            VPU_MSG_ERR("(%d): do nothing\n", __LINE__);
        }
    }

#if 0  // k6 is wbmvop, no need to open hvd clock for sub diu interface
    //MVD use sub mvop
    if((E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType) &&
#ifdef VDEC3
            (pTaskInfo->u8HalId == 1) )
#else
            (E_HAL_VPU_SUB_STREAM0 == pTaskInfo->eVpuId))
#endif
    {
        VPU_MSG_ERR("Force turn on HVD\n");
        if(!HAL_VPU_EX_HVDInUsed())
        {
            if(E_VPU_DEC_MODE_DUAL_INDIE == pVPUHalContext->_stVPUDecMode.u8DecMod)
            {
                if (!HAL_HVD_EX_InitHW(pTaskInfo->u32Id, pTaskInfo->eDecType))
                {
                    VPU_MSG_ERR("(%d):HAL_HVD_EX_InitHW failed\n", __LINE__);
                    return FALSE;
                }
            }
            else
            {
                VPU_MSG_INFO("%s  MVD 3DTV sub\n", __FUNCTION__);
#ifdef CONFIG_MSTAR_CLKM
                HAL_VPU_EX_SetClkManagement(E_VPU_EX_CLKPORT_HVD, TRUE);
#else
                HAL_HVD_EX_PowerCtrl(pTaskInfo->u32Id, TRUE);
#endif
            }
        }
        else
        {
            VPU_MSG_ERR("(%d): do nothing, HVD already init\n", __LINE__);
        }
    }
#endif

    //Check if we need to init HVD HW
#ifdef VDEC3
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
#else
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
#endif
    {
	#ifdef SUPPORT_MVD
        if (!HAL_VPU_EX_MVDInUsed())
        {
            if (!HAL_MVD_InitHW(pTaskInfo->eSrcType, pTaskInfo->eDecType))
            {
                VPU_MSG_ERR("(%d):HAL_MVD_InitHW failed\n", __LINE__);
                return FALSE;
            }
        }
	#endif

        if (!HAL_HVD_EX_InitHW(pTaskInfo->u32Id, pTaskInfo->eDecType))
        {
            VPU_MSG_ERR("(%d):HAL_HVD_EX_InitHW failed\n", __LINE__);
            return FALSE;
        }
#if SUPPORT_MSVP9
        if (E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
        {
            _VPU_WriteWordMask(REG_CLKGEN1_RESERVERD0, SELECT_CLK_HVD_AEC_P_216, SELECT_CLK_HVD_AEC_P_MASK); //for VP9 dqmem
        }
#endif
    }

#if SUPPORT_G2VP9 && defined(VDEC3)
    if (E_VPU_EX_DECODER_G2VP9 == pTaskInfo->eDecType)
    {
        if (!HAL_VPU_EX_MVDInUsed())
        {
            if (!HAL_MVD_InitHW(pTaskInfo->eSrcType, pTaskInfo->eDecType))
            {
                VPU_MSG_ERR("(%d):HAL_MVD_InitHW failed\n", __LINE__);
                return FALSE;
            }
        }
        if (!HAL_HVD_EX_InitHW(pTaskInfo->u32Id, pTaskInfo->eDecType))
        {
            VPU_MSG_ERR("(%d):HAL_HVD_EX_InitHW failed for VP9\n", __LINE__);
            return FALSE;
        }
    }
#endif

    return TRUE;
}

static MS_U32 _VPU_EX_InClock(MS_U32 u32type)
{
#ifdef CHIP_I2_FPGA
    return 12000000UL;
#else
    switch (u32type)
    {
        case VPU_CLOCK_576MHZ:
            return 576000000UL;
        case VPU_CLOCK_480MHZ:
            return 480000000UL;
        case VPU_CLOCK_432MHZ:
            return 432000000UL;
        case VPU_CLOCK_384MHZ:
            return 384000000UL;
        default:
            return 480000000UL;
    }
#endif
}


//#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
//For REE
#ifdef _ENABLE_VDEC_REE
MS_BOOL HAL_VPU_EX_REE_RegisterMBX(void)
{
//#ifndef MSOS_TYPE_LINUX_KERNEL
#if 1
    MS_U8 ClassNum = 0;
    MBX_Result result;

#if 0
    if (bVPUMbxInitFlag == TRUE)
    {
        return TRUE;
    }
#endif

    if (E_MBX_SUCCESS != MApi_MBX_Init(E_MBX_CPU_MIPS, E_MBX_ROLE_HK, 1000))
    {
        VPU_MSG_ERR("VDEC_TEE MApi_MBX_Init fail\n");
        return FALSE;
    }
    else
    {
        MApi_MBX_Enable(TRUE);
    }

    result = MApi_MBX_QueryDynamicClass(E_MBX_CPU_MIPS_VPE1, "VDEC_TEE", (MS_U8 *)&ClassNum);

    if (E_MBX_SUCCESS != result)
    {
        VPU_MSG_ERR("VDEC_TEE MApi_MBX_QueryDynamicClass fail,result %d\n", (unsigned int)result);
        return FALSE;
    }

    result = MApi_MBX_RegisterMSG(ClassNum, 10);

    if (( E_MBX_SUCCESS != result) && ( E_MBX_ERR_SLOT_AREADY_OPENNED != result ))
    {
        VPU_MSG_ERR("%s fail\n", __FUNCTION__);
        return FALSE;
    }
    else
    {
        bVPUMbxInitFlag = TRUE;
        u8VPUMbxMsgClass = ClassNum;
        return TRUE;
    }
#else
    return FALSE;
#endif
}

//#ifdef MBX_2K
#if 1
VDEC_TEE_TO_REE_MBX_ACK_TYPE _VPU_EX_REE_SendMBXMsg(VDEC_REE_TO_TEE_MBX_MSG_TYPE msg_type)
{
    MBX_Result result;
    VDEC_TEE_TO_REE_MBX_ACK_TYPE u8Index;

    if (pVPUHalContext->bEnableVPUSecureMode == FALSE)
    {
        return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_NO_TEE;
    }

    if (bVPUMbxInitFlag == FALSE)
    {
        return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_INVALID;
    }

    VPUReeToTeeMbxMsg.eRoleID = E_MBX_CPU_MIPS_VPE1;
    VPUReeToTeeMbxMsg.u8Ctrl = 0;
    VPUReeToTeeMbxMsg.eMsgType = E_MBX_MSG_TYPE_INSTANT;
    VPUReeToTeeMbxMsg.u8MsgClass = u8VPUMbxMsgClass;
    VPUReeToTeeMbxMsg.u8Index = msg_type;

    result = MApi_MBX_SendMsg(&VPUReeToTeeMbxMsg);
    if (E_MBX_SUCCESS != result)
    {
        VPRINTF("VDEC_TEE Send MBX fail,result %d\n", (unsigned int)result);
        return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_FAIL;
    }

    // Receive Reply ACK from TEE side.
    memset(&VPUTeeToReeMbxMsg, 0, sizeof(MBX_Msg));

    VPUTeeToReeMbxMsg.u8MsgClass = u8VPUMbxMsgClass;

#if 0 // marked temperarily, wait kernel team to fix MApi_MBX_RecvMsg.
    if(E_MBX_SUCCESS != MApi_MBX_RecvMsg(TEE_MBX_MSG_CLASS, &(TEE_TO_REE_MBX_MSG), 20, MBX_CHECK_INSTANT_MSG))
    {
        VPU_MSG_ERR("VDEC get Secure world ACK fail\n");
        return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_FAIL;
    }
    else
#else
    do
    {
        result = MApi_MBX_RecvMsg(u8VPUMbxMsgClass, &VPUTeeToReeMbxMsg, 2000, MBX_CHECK_INSTANT_MSG);
    }
    while(E_MBX_SUCCESS != result);
#endif
    {
        u8Index = VPUTeeToReeMbxMsg.u8Index;
        VPU_MSG_DBG("VDEC get ACK cmd:%x\n", u8Index);

        if (E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_FAIL == u8Index)
        {
            return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_FAIL;
        }
    }

    return E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS;
}
#endif

MS_BOOL HAL_VPU_EX_REE_SetSHMBaseAddr(MS_U32 U32Type, MS_PHY u32SHMAddr, MS_PHY u32SHMSize, MS_PHY u32MIU1Addr)
{
#ifndef STELLAR
    if(U32Type == SYS_TEEINFO_OSTYPE_NUTTX)
    {
        if(_VPU_EX_REE_SendMBXMsg(E_VDEC_EX_REE_TO_TEE_MBX_MSG_GETSHMBASEADDR) != E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS)
        {
            VPU_MSG_ERR("[Error] VDEC load code in Secure world fail!\n");
            return FALSE;
        }
        else
        {
            MS_U32 u32Start;
            MS_VIRT u32VPUSHMoffset;
            MS_U32 u32VPUSHMsize;

            u32VPUSHMoffset = (VPUTeeToReeMbxMsg.u8Parameters[0] & 0xff) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[1] << 8) & 0xff00) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[2] << 16) & 0xff0000) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[3] << 24) & 0xff000000);
            u32VPUSHMsize =   (VPUTeeToReeMbxMsg.u8Parameters[4] & 0xff) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[5] << 8) & 0xff00) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[6] << 16) & 0xff0000) |
                              ((VPUTeeToReeMbxMsg.u8Parameters[7] << 24) & 0xff000000);

            VPU_MSG_INFO("u32VPUSHMoffset %lx,u32VPUSHMsize %x,miu %d\n", (unsigned long)u32VPUSHMoffset, (unsigned int)u32VPUSHMsize, VPUTeeToReeMbxMsg.u8Parameters[8]);


            if(VPUTeeToReeMbxMsg.u8Parameters[8] == 1)
            {
                _miu_offset_to_phy(E_CHIP_MIU_1, u32VPUSHMoffset, u32Start);
                pVPUHalContext->u32VPUSHMAddr =  u32Start;
            }
            else if(VPUTeeToReeMbxMsg.u8Parameters[8] == 2)
            {
                _miu_offset_to_phy(E_CHIP_MIU_2, u32VPUSHMoffset, u32Start);
                pVPUHalContext->u32VPUSHMAddr =  u32Start;

            }
            else // == 0
            {
                pVPUHalContext->u32VPUSHMAddr = u32VPUSHMoffset;
            }
        }
    }
    else if(U32Type == SYS_TEEINFO_OSTYPE_OPTEE)
    {
        MS_U32 u32Offset;
        if((u32SHMAddr >= u32MIU1Addr) && (u32MIU1Addr != 0))
        {
            u32Offset = u32SHMAddr - u32MIU1Addr;
            _miu_offset_to_phy(E_CHIP_MIU_1, u32Offset, pVPUHalContext->u32VPUSHMAddr);
        }
        else
        {
            pVPUHalContext->u32VPUSHMAddr = u32SHMAddr;
        }
    }
#else
    pVPUHalContext->u32VPUSHMAddr = 0;
#endif

    return TRUE;
}
#endif

MS_BOOL HAL_VPU_Set_MBX_param(MS_U8 u8APIMbxMsgClass)
{
    bVPUMbxInitFlag = TRUE;
    u8VPUMbxMsgClass = u8APIMbxMsgClass;
    return TRUE;
}

void HAL_VPU_EX_ForceSwRst(void)
{
    pVPUHalContext->u8ForceRst = 1;
}

//#endif

MS_BOOL HAL_VPU_EX_GetFWReload(void)
{
    return pVPUHalContext->bVpuExReloadFW;
}

static MS_BOOL _VPU_EX_IsNeedDecompress(MS_VIRT u32SrcAddr)
{
    if(*((MS_U8*)(u32SrcAddr)) == 'V' && *((MS_U8*)(u32SrcAddr + 1)) == 'D'
            && *((MS_U8*)(u32SrcAddr + 2)) == 'E' && *((MS_U8*)(u32SrcAddr + 3)) == 'C'
            && *((MS_U8*)(u32SrcAddr + 4)) == '3' && *((MS_U8*)(u32SrcAddr + 5)) == '1'
            && *((MS_U8*)(u32SrcAddr + 0xe8)) == 'V' && *((MS_U8*)(u32SrcAddr + 0xe9)) == 'D'
            && *((MS_U8*)(u32SrcAddr + 0xea)) == 'E' && *((MS_U8*)(u32SrcAddr + 0xeb)) == 'C'
            && *((MS_U8*)(u32SrcAddr + 0xec)) == '3' && *((MS_U8*)(u32SrcAddr + 0xed)) == '0'
      )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static MS_BOOL _VPU_EX_InitAddressLimiter(VPU_EX_FWCodeCfg *pFWCodeCfg)
{


#if 1  // bypass mode for debug

    // allow non-secure read access
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_LOCK_RD_LAT_CLR, VDR2_D_ACCESS_RANGE0_CFG_LOCK_RD_LAT_CLR);
    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_LOCK_RD_LAT_CLR, VDR2_I_ACCESS_RANGE0_CFG_LOCK_RD_LAT_CLR);

    // allow non-secure write access
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_ALWAYS_PASS_W_ADDR, VDR2_D_ACCESS_RANGE0_CFG_ALWAYS_PASS_W_ADDR);
    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_ALWAYS_PASS_W_ADDR, VDR2_I_ACCESS_RANGE0_CFG_ALWAYS_PASS_W_ADDR);

#else
    MS_PHY u32fwPAStart = MsOS_VA2PA(pFWCodeCfg->u32DstAddr);
    MS_PHY u32fwPAEnd = u32fwPAStart + pFWCodeCfg->u32DstSize;
    MS_PHY u32WriteReplaceAddr = u32fwPAStart + CTL_INFO_ADDR;

    // 128 bits align
    if((u32fwPAStart & 0xF) || (u32fwPAEnd & 0xF) || (u32WriteReplaceAddr & 0xF))
    {
        VPU_MSG_ERR("[Error] address need 128 bits align\n");
        return FALSE;
    }

    u32fwPAStart >>= 4;
    u32fwPAEnd >>= 4;
    u32WriteReplaceAddr >>= 4;

    // configure 1st secure range start address
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_L, (MS_U16)(u32fwPAStart & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_H, (MS_U16)(u32fwPAStart >> 16));
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_WRITE_ADDR0_START, VDR2_D_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_L, (MS_U16)(u32fwPAStart & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_H, (MS_U16)(u32fwPAStart >> 16));
    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_WRITE_ADDR0_START, VDR2_I_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    // configure 1st secure range end address
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_L, (MS_U16)(u32fwPAEnd & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_H, (MS_U16)(u32fwPAEnd >> 16));
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_WRITE_ADDR0_END, VDR2_D_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_L, (MS_U16)(u32fwPAEnd & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_H, (MS_U16)(u32fwPAEnd >> 16));
    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_WRITE_ADDR0_END, VDR2_I_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    // configure replaced address
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_L, (MS_U16)(u32WriteReplaceAddr & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_D_ACCESS_RANGE_ADDR_H, (MS_U16)(u32WriteReplaceAddr >> 16));
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_WRITE_REPLACE_ADDR, VDR2_D_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_L, (MS_U16)(u32WriteReplaceAddr & 0xFFFF));
    _VPU_Write2Byte(REG_VDR2_I_ACCESS_RANGE_ADDR_H, (MS_U16)(u32WriteReplaceAddr >> 16));
    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_WRITE_REPLACE_ADDR, VDR2_I_ACCESS_RANGE0_CFG_WRITE_ADDR_MASK);

    // Enable limit write access
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_REF_ADDR0_RANGE_PASS_W_ADDR, VDR2_D_ACCESS_RANGE0_CFG_REF_ADDR0_RANGE_PASS_W_ADDR);

    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_REF_ADDR0_RANGE_PASS_W_ADDR, VDR2_I_ACCESS_RANGE0_CFG_REF_ADDR0_RANGE_PASS_W_ADDR);

    // Enable 1st limit range
    _VPU_WriteWordMask(REG_VDR2_D_ACCESS_RANGE0_CFG, VDR2_D_ACCESS_RANGE0_CFG_ADDR0_LIMIT_EN, VDR2_D_ACCESS_RANGE0_CFG_ADDR0_LIMIT_EN);

    _VPU_WriteWordMask(REG_VDR2_I_ACCESS_RANGE0_CFG, VDR2_I_ACCESS_RANGE0_CFG_ADDR0_LIMIT_EN, VDR2_I_ACCESS_RANGE0_CFG_ADDR0_LIMIT_EN);

    // Enable One-way bit
    _VPU_WriteWordMask(REG_ONEWAY_B16, ONEWAY_B16, ONEWAY_B16);
    _VPU_WriteWordMask(REG_LOCK_ONEWAY_B16, LOCK_ONEWAY_B16, LOCK_ONEWAY_B16);

#endif

    return TRUE;
}

void HAL_HVD_EX_UartSwitch2FW(MS_BOOL bEnable);

extern MS_U32 VPUClockType;
static MS_BOOL _VPU_EX_InitAll(VPU_EX_NDecInitPara *pInitPara)
{
    MS_PHY u32fwPA = NULL;  //physical address

    VPU_EX_ClockSpeed eClkSpeed;

    struct _ctl_info *ctl_ptr;
    VPU_EX_FWCodeCfg *pFWCodeCfg   = NULL;
    VPU_EX_TaskInfo  *pTaskInfo    = NULL;
    VPU_EX_VLCTblCfg *pVlcCfg      = NULL;

#if (defined(MSOS_TYPE_LINUX)||defined(MSOS_TYPE_LINUX_KERNEL))
#ifdef _ENABLE_VDEC_SECUMODE
    SYS_TEEINFO teemode;
#endif
#endif

    switch (VPUClockType)
    {
        case 576:
            eClkSpeed = VPU_CLOCK_576MHZ;
            break;
        case 480:
            eClkSpeed = VPU_CLOCK_480MHZ;
            break;
        case 432:
            eClkSpeed = VPU_CLOCK_432MHZ;
            break;
        case 384:
        default:
            eClkSpeed = VPU_CLOCK_384MHZ;
            break;
    }

    if (TRUE == HAL_VPU_EX_IsPowered())
    {
        VPU_MSG_DBG("IsPowered\n");
        return TRUE;
    }
    else
    {
        VPU_EX_InitParam VPUInitParams = {eClkSpeed, FALSE, -1, VPU_DEFAULT_MUTEX_TIMEOUT, TRUE};
        //VPU hold
        HAL_VPU_EX_SwRst(FALSE);

        //VPU clock on

        if (VPU_I_R_ON_MIU0)
            VPUInitParams.u8MiuSel = 0;
        else if (VPU_I_R_ON_MIU1)
            VPUInitParams.u8MiuSel = 1;
//        else if (VPU_I_R_ON_MIU2)
//            VPUInitParams.u8MiuSel = 2;

        HAL_VPU_EX_Init(&VPUInitParams);
    }

    if (pInitPara)
    {
        pFWCodeCfg  = pInitPara->pFWCodeCfg;
        pTaskInfo   = pInitPara->pTaskInfo;
        pVlcCfg     = pInitPara->pVLCCfg;

        if(_VPU_EX_InitAddressLimiter(pFWCodeCfg) == FALSE)
        {
            VPU_MSG_ERR("(%d) InitAddressLimiter fail\n", __LINE__);
            return FALSE;
        }
    }
    else
    {
        VPU_MSG_DBG("(%d) NULL para\n", __LINE__);
        return FALSE;
    }

    u32fwPA = MsOS_VA2PA(pFWCodeCfg->u32DstAddr);

#if (defined(MSOS_TYPE_LINUX)||defined(MSOS_TYPE_LINUX_KERNEL))
#if defined(_ENABLE_VDEC_SECUMODE)

    MDrv_SYS_ReadKernelCmdLine();
    MDrv_SYS_GetTEEInfo(&teemode);
    VPRINTF("[VDEC][TEE/OPTEE]%s,TEE_type=%d\n", __FUNCTION__, teemode.OsType);

    if(teemode.OsType == SYS_TEEINFO_OSTYPE_OPTEE)
    {
        if(pVPUHalContext->bEnableVPUSecureMode == FALSE)
        {
            MS_BOOL bAlwaysOpteeLoadFW = FALSE;
            if(FALSE == HAL_VPU_EX_GetCapability((MS_U8 *)("CAP_ALWAYS_OPTEE_LOAD_FW"), NULL, (void *)&bAlwaysOpteeLoadFW))
            {
                bAlwaysOpteeLoadFW = FALSE;
            }

            if(bAlwaysOpteeLoadFW == FALSE)
            {
                VPU_MSG_INFO("%d, Load VDEC f/w code in Normal World\n", __LINE__);

                if (!HAL_VPU_EX_LoadCode(pFWCodeCfg))
                {
                    VPU_MSG_ERR("%d, HAL_VPU_EX_LoadCode fail!\n", __LINE__);
                    return FALSE;
                }
            }
        }
    }
    else
    {
        if(pVPUHalContext->bEnableVPUSecureMode == TRUE)
        {
            if(teemode.OsType == SYS_TEEINFO_OSTYPE_NUTTX)
            {
                VPU_MSG_INFO("Load VDEC f/w code in Secure World\n");

#ifdef _ENABLE_VDEC_REE
                if (FALSE == HAL_VPU_EX_GetFWReload())
                {
                    if (FALSE == pVPUHalContext->bVpuExLoadFWRlt)
                    {
                        VPU_MSG_INFO("Never load fw successfully, load it anyway!\n");
                        if(_VPU_EX_REE_SendMBXMsg(E_VDEC_EX_REE_TO_TEE_MBX_MSG_FW_LoadCode) != E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS)
                        {
                            VPU_MSG_ERR("[Error] VDEC load code in Secure world fail!\n");
                            return FALSE;
                        }
                        pVPUHalContext->bVpuExLoadFWRlt = TRUE;
                    }
                    else
                    {
                        //Check f/w prefix "VDEC30"
                        if (_VPU_EX_IsNeedDecompress(pFWCodeCfg->u32DstAddr) != FALSE)
                        {
                            VPU_MSG_ERR("Wrong prefix: reload fw!\n");
                            if(_VPU_EX_REE_SendMBXMsg(E_VDEC_EX_REE_TO_TEE_MBX_MSG_FW_LoadCode) != E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS)
                            {
                                VPU_MSG_ERR("[Error] VDEC load code in Secure world fail!\n");
                                pVPUHalContext->bVpuExLoadFWRlt = FALSE;
                                return FALSE;
                            }
                        }
                        else
                        {
                            VPU_MSG_INFO("Skip loading fw this time!!!\n");
                        }
                    }
                }
                else
                {
                    if(_VPU_EX_REE_SendMBXMsg(E_VDEC_EX_REE_TO_TEE_MBX_MSG_FW_LoadCode) != E_VDEC_EX_TEE_TO_REE_MBX_ACK_MSG_ACTION_SUCCESS)
                    {
                        VPU_MSG_ERR("[Error] VDEC load code in Secure world fail!\n");
                        pVPUHalContext->bVpuExLoadFWRlt = FALSE;
                        return FALSE;
                    }
                    pVPUHalContext->bVpuExLoadFWRlt = TRUE;
                }
#endif
            }
        }
        else
#endif
#endif
        {
            VPU_MSG_INFO("Load VDEC f/w code in Normal World\n");

            if (!HAL_VPU_EX_LoadCode(pFWCodeCfg))
            {
                VPU_MSG_ERR("HAL_VPU_EX_LoadCode fail!\n");
                return FALSE;
            }
        }
#if (defined(MSOS_TYPE_LINUX)||defined(MSOS_TYPE_LINUX_KERNEL))
#if defined(_ENABLE_VDEC_SECUMODE)
    }
#endif
#endif

    if (pVlcCfg)
    {
        if (!_VPU_EX_LoadVLCTable(pVlcCfg, pFWCodeCfg->u8SrcType))
        {
            VPU_MSG_ERR("HAL_VPU_LoadVLCTable fail!\n");
            return FALSE;
        }
    }

    if (!HAL_VPU_EX_CPUSetting(u32fwPA))
    {
        VPU_MSG_ERR("HAL_VPU_EX_CPUSetting fail!\n");
        return FALSE;
    }

    //Init HW
    if (FALSE == _VPU_EX_InitHW(pTaskInfo))
    {
        VPU_MSG_ERR("(%d): InitHW failed\n", __LINE__);
        //_MVD_INIT_FAIL_RET();
        return FALSE;
    }
    else
    {
        VPU_MSG_DBG("(%d): InitHW success\n", __LINE__);
    }

    //set vpu clock to FW
    ctl_ptr = (struct _ctl_info *)
              MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + CTL_INFO_ADDR);


#if (VPU_ENABLE_IQMEM)
    ctl_ptr->bIQmemEnableIfSupport = TRUE;
#endif

    ctl_ptr->statue = CTL_STU_NONE;
    //notify controller the interface version of VPU driver.
    ctl_ptr->ctl_interface = VPU_CTL_INTERFACE_VER;
    ctl_ptr->vpu_clk = _VPU_EX_InClock(eClkSpeed);
    MsOS_FlushMemory();
    VPU_MSG_DBG("clock speed=0x%x\n", ctl_ptr->vpu_clk);

    //Release VPU: For dual decoder, we only release VPU if it is not released yet.
    if (TRUE == HAL_VPU_EX_IsRsted())
    {
        VPU_MSG_DBG("VPU_IsRsted\n");
        return TRUE;
    }
    else
    {
        HAL_VPU_EX_SwRstRelse();
    }

    return TRUE;
}

static MS_BOOL _VPU_EX_DeinitHW(VPU_EX_TaskInfo *pTaskInfo)
{
    MS_BOOL bRet = FALSE;

#if defined(VDEC3)
    MS_BOOL isEVD = (E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType);
#else
    MS_BOOL isEVD = FALSE ;
#endif
    MS_BOOL isHVD = (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
                    || (E_VPU_EX_DECODER_VP8 == pTaskInfo->eDecType)
                    || (E_VPU_EX_DECODER_MVC == pTaskInfo->eDecType);

#ifdef SUPPORT_MVD
    if (FALSE == HAL_VPU_EX_MVDInUsed())
    {
        bRet = HAL_MVD_DeinitHW(pTaskInfo->eSrcType, pTaskInfo->eDecType);
    }
#endif

    if (TRUE == isHVD)
    {
        bRet = HAL_HVD_EX_DeinitHW(pTaskInfo->u32Id);
    }

#if defined(VDEC3)
    if (TRUE == isEVD)
    {
        bRet = HAL_EVD_EX_DeinitHW(pTaskInfo->u32Id);
    }
#endif

#if SUPPORT_G2VP9 && defined(VDEC3)
    if (FALSE == HAL_VPU_EX_G2VP9InUsed())
    {
        bRet = HAL_VP9_EX_DeinitHW();
    }
#endif
    return bRet;
}

static MS_BOOL _VPU_EX_DeinitAll(VPU_EX_NDecInitPara *pInitPara)
{
    HAL_VPU_EX_SwRst(TRUE);
    _VPU_EX_DeinitHW(pInitPara->pTaskInfo);
    HAL_VPU_EX_DeInit();

    return TRUE;
}

#if 0
static MS_U8 _VPU_EX_GetActiveCodecCnt(void)
{
    MS_U32 i;
    MS_U8  u8ActiveCnt = 0;
    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if (E_VPU_EX_DECODER_NONE != pVPUHalContext->_stVPUStream[i].eDecodertype &&
                E_VPU_EX_DECODER_GET != pVPUHalContext->_stVPUStream[i].eDecodertype &&
                E_VPU_EX_DECODER_GET_MVC != pVPUHalContext->_stVPUStream[i].eDecodertype)
        {
            u8ActiveCnt++;
        }
    }
    if (pVPUHalContext->u8TaskCnt != u8ActiveCnt)
    {
        VPU_MSG_ERR("Err u8TaskCnt(%d) != u8ActiveCnt(%d)\n", pVPUHalContext->u8TaskCnt, u8ActiveCnt);
    }
    VPU_MSG_DBG(" = %d\n", u8ActiveCnt);
    return u8ActiveCnt;
}
#endif

static void _VPU_EX_ClockInv(MS_BOOL bEnable)
{
#ifndef _DISABLE_CLK_CTL
    if (TRUE)
    {
        _VPU_WriteWordMask(REG_TOP_VPU, 0, TOP_CKG_VPU_INV);
    }
    else
    {
        _VPU_WriteWordMask(REG_TOP_VPU, TOP_CKG_VPU_INV, TOP_CKG_VPU_INV);
    }
#endif
}

static void _VPU_EX_ClockSpeed(MS_U32 u32type)
{
#ifndef _DISABLE_CLK_CTL
    switch (u32type)
    {
        case VPU_CLOCK_576MHZ:
        case VPU_CLOCK_480MHZ:
        case VPU_CLOCK_432MHZ:
        case VPU_CLOCK_384MHZ:
            _VPU_WriteWordMask(REG_TOP_VPU, u32type, TOP_CKG_VPU_CLK_MASK);
            break;
        default:
            _VPU_WriteWordMask(REG_TOP_VPU, VPU_CLOCK_480MHZ, TOP_CKG_VPU_CLK_MASK);
            break;
    }
#endif
}
#ifdef HAL_FEATURE_MAU
static MS_BOOL _VPU_EX_MAU_IDLE(void)
{
    if (((_VPU_Read2Byte(MAU1_ARB0_DBG0) & MAU1_FSM_CS_MASK) == MAU1_FSM_CS_IDLE)
            && ((_VPU_Read2Byte(MAU1_ARB1_DBG0) & MAU1_FSM_CS_MASK) == MAU1_FSM_CS_IDLE))
    {
        return TRUE;
    }
    return FALSE;
}
#endif


#if (ENABLE_DECOMPRESS_FUNCTION==TRUE)
static MS_BOOL _VPU_EX_DecompressBin(MS_VIRT u32SrcAddr, MS_U32 u32SrcSize, MS_VIRT u32DestAddr, MS_VIRT u32SlidingAddr)
{
    if(_VPU_EX_IsNeedDecompress(u32SrcAddr))
    {
        ms_VDECDecompressInit((MS_U8*)u32SlidingAddr, (MS_U8*)u32DestAddr);
        ms_VDECDecompress((MS_U8*)u32SrcAddr, u32SrcSize);
        ms_VDECDecompressDeInit();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

MS_BOOL HAL_VPU_EX_SetSingleDecodeMode(MS_BOOL bEnable)
{
    MS_BOOL bRet = TRUE;
    pVPUHalContext->_bVPUSingleMode = bEnable;
    return bRet;
}

MS_BOOL HAL_VPU_EX_SetSTCMode(MS_U32 u32Id, MS_U32 u32STCIndex)
{
    MS_BOOL bRet = TRUE;
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
    pVPUHalContext->_stVPUSTCMode[u8OffsetIdx].bSTCSetMode = TRUE;
    pVPUHalContext->_stVPUSTCMode[u8OffsetIdx].u32STCIndex = u32STCIndex;

    return bRet;
}

MS_BOOL HAL_VPU_EX_SetDecodeMode(VPU_EX_DecModCfg *pstCfg)
{
    MS_BOOL bRet = TRUE;
    MS_U8 i = 0;
    if (pstCfg != NULL)
    {
        pVPUHalContext->_stVPUDecMode.u8DecMod = pstCfg->u8DecMod;
        pVPUHalContext->_stVPUDecMode.u8CodecCnt = pstCfg->u8CodecCnt;
        for (i = 0; ((i < pstCfg->u8CodecCnt) && (i < VPU_MAX_DEC_NUM)); i++)
        {
            pVPUHalContext->_stVPUDecMode.u8CodecType[i] = pstCfg->u8CodecType[i];
        }
        pVPUHalContext->_stVPUDecMode.u8ArgSize = pstCfg->u8ArgSize;
        pVPUHalContext->_stVPUDecMode.u32Arg    = pstCfg->u32Arg;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

//static MS_BOOL bVpuExReloadFW = TRUE;
//static MS_BOOL bVpuExLoadFWRlt = FALSE;
MS_BOOL HAL_VPU_EX_SetFWReload(MS_BOOL bReload)
{
    pVPUHalContext->bVpuExReloadFW = bReload;
    //VPRINTF("%s bVpuExReloadFW = %x\n", __FUNCTION__, bVpuExReloadFW);
    return TRUE;
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
#ifdef VDEC3_FB
MS_BOOL HAL_VPU_EX_LoadVLCTable(VPU_EX_VLCTblCfg *pVlcCfg, MS_U8 u8FwSrcType)
{
#if HVD_ENABLE_RV_FEATURE
    if (E_HVD_FW_INPUT_SOURCE_FLASH == u8FwSrcType)
    {
#if VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM
        VPU_MSG_DBG("Load VLC outF2D: dest:0x%lx source:%lx size:%lx\n",
                    pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize);

        if (pVlcCfg->u32BinSize)
        {
            SPIDMA_Dev cpyflag = E_SPIDMA_DEV_MIU1;

            if (HAL_MIU1_BASE <= MsOS_VA2PA(pVlcCfg->u32DstAddr))
            {
                cpyflag = E_SPIDMA_DEV_MIU1;
            }
            else
            {
                cpyflag = E_SPIDMA_DEV_MIU0;
            }

            if (!HVD_FLASHcpy(MsOS_VA2PA(pVlcCfg->u32DstAddr), MsOS_VA2PA(pVlcCfg->u32BinAddr), pVlcCfg->u32BinSize, cpyflag))
            {
                VPU_MSG_ERR("HVD_BDMAcpy VLC table Flash 2 DRAM failed: dest:0x%lx src:0x%lx size:0x%lx flag:%lu\n",
                            pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize, (MS_U32) cpyflag);

                return FALSE;
            }
        }
        else
        {
            VPU_MSG_ERR("During copy VLC from Flash to Dram, the source size of FW is zero\n");
            return FALSE;
        }
#else
        VPU_MSG_ERR("driver not enable to use BDMA copy VLC from flash 2 sdram.\n");
        return FALSE;
#endif
    }
    else
    {
        if (E_HVD_FW_INPUT_SOURCE_DRAM == u8FwSrcType)
        {
            if ((pVlcCfg->u32BinAddr != 0) && (pVlcCfg->u32BinSize != 0))
            {
                VPU_MSG_INFO("Load VLC outD2D: dest:0x%lx source:%lx size:%lx\n",
                             (unsigned long)pVlcCfg->u32DstAddr, (unsigned long)pVlcCfg->u32BinAddr, (unsigned long)pVlcCfg->u32BinSize);

#if HVD_ENABLE_BDMA_2_BITSTREAMBUF
                BDMA_Result bdmaRlt;

                MsOS_FlushMemory();
                bdmaRlt = HVD_dmacpy(pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize);

                if (E_BDMA_OK != bdmaRlt)
                {
                    VPU_MSG_ERR("MDrv_BDMA_MemCopy fail in %s(), ret=%x!\n", __FUNCTION__, bdmaRlt);
                }
#else
                HVD_memcpy(pVlcCfg->u32DstAddr, pVlcCfg->u32BinAddr, pVlcCfg->u32BinSize);
#endif
            }
            else
            {
                VPU_MSG_ERR
                ("During copy VLC from out Dram to Dram, the source size or virtual address of VLC is zero\n");
                return FALSE;
            }
        }
        else
        {
#if VPU_ENABLE_EMBEDDED_FW_BINARY
#ifdef HVD_CACHE_TO_UNCACHE_CONVERT
            MS_U8 *pu8HVD_VLC_Binary;

            pu8HVD_VLC_Binary = (MS_U8 *) ((MS_U32) u8HVD_VLC_Binary | 0xA0000000);

            VPU_MSG_DBG("Load VLC inD2D: dest:0x%lx source:%lx size:%lx\n",
                        (unsigned long)pVlcCfg->u32DstAddr, (unsigned long) pu8HVD_VLC_Binary),
                        (MS_U32) sizeof(u8HVD_VLC_Binary));

            HVD_memcpy((void *) (pVlcCfg->u32DstAddr),
                       (void *) ((MS_U32) pu8HVD_VLC_Binary), sizeof(u8HVD_VLC_Binary));
#else
            VPU_MSG_INFO("Load VLC inD2D: dest:0x%lx source:%lx size:%x\n",
                         (unsigned long)MsOS_VA2PA(pVlcCfg->u32DstAddr), (unsigned long) u8HVD_VLC_Binary,
                         (MS_U32) sizeof(u8HVD_VLC_Binary));

            HVD_memcpy(pVlcCfg->u32DstAddr, ((MS_VIRT) u8HVD_VLC_Binary), sizeof(u8HVD_VLC_Binary));
#endif
#else
            VPU_MSG_ERR("driver not enable to use embedded VLC binary.\n");
            return FALSE;
#endif
        }
    }
#endif

    return TRUE;
}
#endif


#if (VPU_ENABLE_IQMEM)
MS_BOOL HAL_VPU_EX_IQMem_Init(VPU_EX_NDecInitPara *pInitPara)
{

    struct _ctl_info *ctl_ptr = (struct _ctl_info *)
                                MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + CTL_INFO_ADDR);


    MS_U32 u32Timeout = 20000;

    if (ctl_ptr->u8IQmemCtrl == E_CTL_IQMEM_INIT_NONE)
    {

        HAL_VPU_EX_IQMemSetDAMode(TRUE);


        ctl_ptr->u8IQmemCtrl = E_CTL_IQMEM_INIT_LOADING;

        while (u32Timeout)
        {

            if (ctl_ptr->u8IQmemCtrl == E_CTL_IQMEM_INIT_LOADED)
            {
                break;
            }
            u32Timeout--;
            MsOS_DelayTaskUs(1000);
        }

        HAL_VPU_EX_IQMemSetDAMode(FALSE);


        ctl_ptr->u8IQmemCtrl = E_CTL_IQMEM_INIT_FINISH;

        if (u32Timeout == 0)
        {
            HVD_EX_MSG_ERR("Wait E_CTL_IQMEM_INIT_FINISH timeout !!\n");
            return FALSE;
        }

    }
    return TRUE;
}

#endif



#ifdef VDEC3
MS_BOOL HAL_VPU_EX_TaskCreate(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara, MS_BOOL bFWdecideFB, MS_U32 u32BBUId)
#else
MS_BOOL HAL_VPU_EX_TaskCreate(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara)
#endif
{
    VPU_EX_TaskInfo *pTaskInfo  = pInitPara->pTaskInfo;
    MS_U8 u8Offset              = _VPU_EX_GetOffsetIdx(u32Id);
    HVD_User_Cmd eCmd           = E_HVD_CMD_INVALID_CMD;
    VPU_EX_DecoderType eDecType = E_VPU_EX_DECODER_NONE;
    MS_U32 u32Arg = 0xFFFFFFFF;
    MS_U32 u32Timeout = 0;
    HVD_Return eCtrlRet = E_HVD_RETURN_FAIL;
    MS_U32 u32CmdArg = 0;
    struct _ctl_info *ctl_ptr = (struct _ctl_info *)
                                MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + CTL_INFO_ADDR);
    //char lastStatus = 0xFF;

    _HAL_VPU_Entry();
    //Check FW buffer size
    if (1 == u8Offset)
    {
        MS_VIRT u32MinFWBuffSize = (u8Offset + 1) * VPU_FW_MEM_OFFSET;
        MS_VIRT u32CurFWBuffSize = pInitPara->pFWCodeCfg->u32DstSize;

        if (u32CurFWBuffSize < u32MinFWBuffSize)
        {
            VPU_MSG_ERR("FW BuffSize(0x%lx < 0x%lx) is too small!\n", (unsigned long)u32CurFWBuffSize, (unsigned long)u32MinFWBuffSize);
            _HAL_VPU_Release();
            return FALSE;
        }
    }

    if(( E_HAL_VPU_MVC_STREAM_BASE == (0xFF & u32Id))
            && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[0].eDecodertype)
            && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[1].eDecodertype))
    {
        pVPUHalContext->_stVPUStream[0].eStreamId = E_HAL_VPU_MVC_MAIN_VIEW;
    }
#ifdef VDEC3
    pVPUHalContext->u32FWCodeAddr = MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr);
#endif

    if (0 == pVPUHalContext->u8TaskCnt)
    {
        //No task is created, need to load f/w, etc.
        VPU_MSG_DBG("u8TaskCnt=%d\n", pVPUHalContext->u8TaskCnt);

        if (!_VPU_EX_InitAll(pInitPara))
        {
            VPU_MSG_DBG("(%d) fail to InitAll\n", __LINE__);
            _HAL_VPU_Release();
            return FALSE;
        }

      //if (E_VPU_EX_DECODER_MJPEG != pTaskInfo->eDecType) //sylvia test
      {
        //Check if controller finish initialization: clear mailbox, etc.
        //Need to check it before sending any controller commands!
        u32Timeout = HVD_GetSysTime_ms() + VPU_CMD_TIMEOUT;
        while (CTL_STU_NONE == ctl_ptr->statue)
        {
            if (HVD_GetSysTime_ms() > u32Timeout)
            {
                MS_U32 t = 0;
                VPU_MSG_ERR("Ctl init timeout, st=%x\n", ctl_ptr->statue);
                VPU_MSG_ERR("version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                            ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);
                for (t = 0; t < 30; t++)
                {
                    VPU_MSG_DBG("_pc=0x%x\n", HAL_VPU_EX_GetProgCnt());
                }
                _HAL_VPU_Release();
                return FALSE;
            }

            MsOS_ReadMemory();
        }
      }

        VPU_MSG_INFO("ctl_init_done: version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                     ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);

    }
    else
    {
        if (pVPUHalContext->_bVPUSingleMode)
        {
            //Show error message
            VPRINTF("This task will use dram instead of sram!!!\n");
            VPU_MSG_INFO("VDEC warn: this task will use dram instead of sram!!!\n");
        }

        if (!_VPU_EX_InitHW(pInitPara->pTaskInfo))
        {
            VPU_MSG_DBG("(%d) fail to InitHW\n", __LINE__);
            _HAL_VPU_Release();
            return FALSE;
        }
        if (pInitPara->pVLCCfg)
        {
            if (!_VPU_EX_LoadVLCTable(pInitPara->pVLCCfg, pInitPara->pFWCodeCfg->u8SrcType))
            {
                VPU_MSG_ERR("HAL_VPU_LoadVLCTable fail!\n");
                _HAL_VPU_Release();
                return FALSE;
            }
        }
    }


#if (VPU_ENABLE_IQMEM)

    if( ctl_ptr->bIsIQMEMSupport)
    {

        HAL_VPU_EX_IQMem_Init(pInitPara);
    }
    else
    {
        VPU_MSG_ERR("not support IQMEM\n");
    }

#endif

#ifdef VDEC_FW31_HVD_NONAUTO_BBU
    if (pVPUHalContext->u8TaskCnt == 0)
    {
        ctl_ptr->bEnableHvdNonAutoBBU = TRUE;
        MsOS_FlushMemory();
    }
#endif

#ifdef VDEC3
    if (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
    {
        DISPQ_IN_DRAM *pTemp;
        CMD_QUEUE *pTemp2 ;
        unsigned char* pTemp3;
        MS_U32 u32StartOffset = 0;
        MS_U8  u8MiuSel = 0;
        unsigned int* pVersion;
        VDEC_VBBU *pTemp4 = (VDEC_VBBU *)MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + VBBU_TABLE_START + u8Offset * VPU_FW_MEM_OFFSET);

        memset(pTemp4, 0, sizeof(VDEC_VBBU));

        //*((unsigned int*)(pTemp4->u8Reserved)) = MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr)-HAL_MIU1_BASE;
        _phy_to_miu_offset(u8MiuSel, u32StartOffset, MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr));
        *((unsigned int*)(pTemp4->u8Reserved)) = u32StartOffset;

        pTemp = (DISPQ_IN_DRAM *)MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + DISP_QUEUE_START + u8Offset * VPU_FW_MEM_OFFSET);

        memset(pTemp, 0, sizeof(DISPQ_IN_DRAM));

        pTemp2 = (CMD_QUEUE *)MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + VCOMMANDQ_INFO_START + u8Offset * VPU_FW_MEM_OFFSET);

        memset(pTemp2, 0, sizeof(CMD_QUEUE));

        pTemp2->u32HVD_DISPCMDQ_DRAM_ST_ADDR = VDISP_COMMANDQ_START + u8Offset * VPU_FW_MEM_OFFSET;

        pTemp2->u32HVD_CMDQ_DRAM_ST_ADDR = VNORMAL_COMMANDQ_START + u8Offset * VPU_FW_MEM_OFFSET;

        pTemp3 = (unsigned char*)MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + VDISP_COMMANDQ_START + u8Offset * VPU_FW_MEM_OFFSET);

        memset(pTemp3, 0, 0x2000);

        pVersion = (unsigned int*)MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + OFFSET_BASE + u8Offset * VPU_FW_MEM_OFFSET);

        memset((void*)pVersion, 0, 0x8);

        *pVersion = 1; //0:diu, 1:wb
    }

#endif

#if 1  // For TEE
#ifdef VDEC3
#if SUPPORT_G2VP9
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType ||
            E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_G2VP9 == pTaskInfo->eDecType)
#else
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
#endif
#else
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
#endif
    {
        MS_VIRT u32FWPhyAddr = MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr);

        if (pVPUHalContext->u32FWShareInfoAddr[u8Offset] == 0xFFFFFFFFUL)
        {
            ctl_ptr->u32TaskShareInfoAddr[u8Offset] = 0xFFFFFFFFUL;
        }
        else
        {
            ctl_ptr->u32TaskShareInfoAddr[u8Offset] = pVPUHalContext->u32FWShareInfoAddr[u8Offset] - u32FWPhyAddr;
        }

        MsOS_FlushMemory();
        VPU_MSG_DBG("task share info offset = 0x%x\n", ctl_ptr->u32TaskShareInfoAddr[u8Offset]);

        ///VPRINTF("DRV side,      share info offset = 0x%lx\n", pVPUHalContext->u32FWShareInfoAddr[u8Offset]);
        ///VPRINTF("FW side,  task share info offset = 0x%x\n", ctl_ptr->u32TaskShareInfoAddr[u8Offset]);
    }
#endif

    if ((pVPUHalContext->bEnableDymanicFBMode == TRUE) && (pVPUHalContext->u8TaskCnt == 0))
    {
        ctl_ptr->FB_ADDRESS = pVPUHalContext->u32DynamicFBAddress;
        ctl_ptr->FB_Total_SIZE = pVPUHalContext->u32DynamicFBSize;

        HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_COMMON, 0);

        MsOS_FlushMemory();
    }

    if (pVPUHalContext->_stVPUSTCMode[u8Offset].bSTCSetMode)
    {
        eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_STC_MODE, pVPUHalContext->_stVPUSTCMode[u8Offset].u32STCIndex);
        if (E_HVD_RETURN_SUCCESS != eCtrlRet)
        {
            VPU_MSG_ERR("E_HVD_CMD_STC_MODE NG eCtrlRet=%x\n", eCtrlRet);
        }
    }

    if ((TRUE == pVPUHalContext->_bVPUSingleMode) || (E_VPU_DEC_MODE_SINGLE == pVPUHalContext->_stVPUDecMode.u8DecMod))
    {
        //Issue E_DUAL_CMD_SINGLE_TASK to FW controller
        //arg=1 to get better performance for single task
        u32CmdArg = (pVPUHalContext->_bVPUSingleMode) ? 1 : 0;
        VPU_MSG_DBG("Issue E_DUAL_CMD_SINGLE_TASK to FW controller arg=%x\n", u32CmdArg);
        eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_SINGLE_TASK, u32CmdArg);
        if (E_HVD_RETURN_SUCCESS != eCtrlRet)
        {
            VPU_MSG_ERR("E_DUAL_CMD_SINGLE_TASK NG eCtrlRet=%x\n", eCtrlRet);
        }
    }
    else if (E_VPU_DEC_MODE_DUAL_3D == pVPUHalContext->_stVPUDecMode.u8DecMod)
    {
        if(pVPUHalContext->_stVPUDecMode.u8CodecType[0] != pVPUHalContext->_stVPUDecMode.u8CodecType[1])
        {
            switch (pVPUHalContext->_stVPUDecMode.u32Arg)
            {
                case E_VPU_CMD_MODE_KR3D_INTERLACE:
                    u32CmdArg = CTL_MODE_3DTV;
                    break;
                case E_VPU_CMD_MODE_KR3D_FORCE_P:
                    u32CmdArg = CTL_MODE_3DTV_PROG;
                    break;
                case E_VPU_CMD_MODE_KR3D_INTERLACE_TWO_PITCH:
                    u32CmdArg = CTL_MODE_3DTV_TWO_PITCH;
                    break;
                case E_VPU_CMD_MODE_KR3D_FORCE_P_TWO_PITCH:
                    u32CmdArg = CTL_MODE_3DTV_PROG_TWO_PITCH;
                    break;
                default:
                    u32CmdArg = CTL_MODE_3DTV;
                    VPU_MSG_INFO("%x not defined, use CTL_MODE_3DTV for KR3D\n", pVPUHalContext->_stVPUDecMode.u32Arg);
                    break;
            }
        }
        else
        {
            u32CmdArg = CTL_MODE_3DWMV;
        }
        VPU_MSG_DBG("Issue E_DUAL_CMD_MODE to FW controller arg=%x\n", u32CmdArg);
        eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_MODE, u32CmdArg);
        if (E_HVD_RETURN_SUCCESS != eCtrlRet)
        {
            VPU_MSG_ERR("E_DUAL_CMD_MODE NG eCtrlRet=%x\n", eCtrlRet);
        }
    }
    else if (E_VPU_DEC_MODE_DUAL_INDIE == pVPUHalContext->_stVPUDecMode.u8DecMod)
    {
        if(E_VPU_CMD_MODE_PIP_SYNC_MAIN_STC == pVPUHalContext->_stVPUDecMode.u32Arg)
        {
            u32CmdArg = CTL_MODE_ONE_STC;
        }
        else
        {
            u32CmdArg = (pVPUHalContext->_stVPUDecMode.u32Arg == E_VPU_CMD_MODE_PIP_SYNC_SWITCH) ? CTL_MODE_SWITCH_STC : CTL_MODE_NORMAL;
        }
        VPU_MSG_DBG("Issue E_DUAL_CMD_MODE to FW controller arg=%x\n", u32CmdArg);
        eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_MODE, u32CmdArg);
        if (E_HVD_RETURN_SUCCESS != eCtrlRet)
        {
            VPU_MSG_ERR("E_DUAL_CMD_MODE NG eCtrlRet=%x\n", eCtrlRet);
        }
    }
    else if (E_VPU_DEC_MODE_AUTO_MCU == pVPUHalContext->_stVPUDecMode.u8DecMod)
    {
        u32CmdArg = CTL_MODE_SEC_MCU;
        VPU_MSG_DBG("Issue E_DUAL_CMD_MODE to FW controller arg=%x\n", u32CmdArg);
        eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_CMD_MODE, u32CmdArg);
        if (E_HVD_RETURN_SUCCESS != eCtrlRet)
        {
            VPU_MSG_ERR("E_DUAL_CMD_MODE NG eCtrlRet=%x\n", eCtrlRet);
        }
    }

    eCmd = _VPU_EX_MapCtrlCmd(pTaskInfo);
#if defined(SUPPORT_NEW_MEM_LAYOUT)
    if (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
#ifdef VDEC3
    {
        u32Arg = (u32BBUId << VDEC_BBU_ID_SHIFT) + u8Offset * VPU_FW_MEM_OFFSET + OFFSET_BASE;
    }
#else
        u32Arg = u8Offset * VPU_FW_MEM_OFFSET + OFFSET_BASE;
#endif

#ifdef VDEC3
#if SUPPORT_G2VP9
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_G2VP9 == pTaskInfo->eDecType)
#else
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
#endif
    {
        u32Arg = (u32BBUId << VDEC_BBU_ID_SHIFT) + (u8Offset * VPU_FW_MEM_OFFSET) + HVD_SHARE_MEM_ST_OFFSET;
    }
#else
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
        u32Arg = u8Offset * VPU_FW_MEM_OFFSET + HVD_SHARE_MEM_ST_OFFSET;
#endif
    else
    {
        VPU_MSG_ERR("Can't find eDecType! %d\n", pTaskInfo->eDecType);
        _HAL_VPU_Release();
        return FALSE;
    }
#else
    u32Arg = u8Offset * VPU_FW_MEM_OFFSET;
#endif

    VPU_MSG_INFO("task info: id 0x%x, cmd 0x%x, arg 0x%x, bbuID %d, offset %d \n", u32Id, eCmd, u32Arg, u32BBUId, u8Offset);
    HAL_HVD_EX_SetCmd(u32Id, eCmd, u32Arg);

    MsOS_ReadMemory();
    VPU_MSG_INFO("create task: ctl_ptr addr=0x%x, taskStatus addr=0x%x\n", (unsigned int)ctl_ptr, (unsigned int)&ctl_ptr->task_statue[0]);
    VPU_MSG_INFO("before: version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                 ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);

    u32Timeout = HVD_GetSysTime_ms() + VPU_CMD_TIMEOUT;

    while (CTL_TASK_CMDRDY != ctl_ptr->task_statue[u8Offset])
    {
        if (HVD_GetSysTime_ms() > u32Timeout)
        {
            MS_U32 t = 0;
            VPU_MSG_ERR("Task %d creation timeout\n", u8Offset);
            for (t = 0; t < 30; t++)
            {
                VPU_MSG_DBG("_pc=0x%x\n", HAL_VPU_EX_GetProgCnt());
            }
//#ifndef VDEC3 // FIXME: workaround fw response time is slow sometimes in multiple stream case so far
            pVPUHalContext->bVpuExLoadFWRlt = FALSE; ///error handling
            VPU_MSG_ERR("set bVpuExLoadFWRlt as FALSE\n\n");
            _HAL_VPU_Release();
            return FALSE;
//#endif
        }

#if 0
        // if (ctl_ptr->task_statue[0] != lastStatus)
        {
            VPU_MSG_INFO("pooling: task_status %d t0=%d, t1=%d, t2=%d\n", u8Offset, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1], ctl_ptr->task_statue[2] );
            lastStatus = ctl_ptr->task_statue[0];
        }
#endif

        MsOS_ReadMemory();
    }

    VPU_MSG_INFO("after: version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                 ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);

#ifdef VDEC3
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType || E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
#else
    if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
#endif
    {
        HAL_HVD_EX_SetNalTblAddr(u32Id);
    }

    if (E_VPU_EX_DECODER_MVD == pTaskInfo->eDecType)
    {
        eDecType = E_VPU_EX_DECODER_MVD;
    }
    else if (E_VPU_EX_DECODER_HVD == pTaskInfo->eDecType)
    {
        eDecType = E_VPU_EX_DECODER_HVD;
    }
#ifdef VDEC3
    else if (E_VPU_EX_DECODER_EVD == pTaskInfo->eDecType)
    {
        eDecType = E_VPU_EX_DECODER_EVD;
    }
#if SUPPORT_G2VP9
    else if (E_VPU_EX_DECODER_G2VP9 == pTaskInfo->eDecType)
    {
        eDecType = E_VPU_EX_DECODER_G2VP9;
    }
#endif
#endif
    else
    {
        VPU_MSG_ERR("Can't find eDecType! %d\n", pTaskInfo->eDecType);
        _HAL_VPU_Release();
        return FALSE;
    }

#ifdef VDEC3
    if ((bFWdecideFB == TRUE) && (pVPUHalContext->u8TaskCnt == 0))
    {
        HAL_HVD_EX_SetCmd(u32Id, E_DUAL_R2_CMD_FBADDR, pInitPara->pFBCfg->u32FrameBufAddr);
        HAL_HVD_EX_SetCmd(u32Id, E_DUAL_R2_CMD_FBSIZE, pInitPara->pFBCfg->u32FrameBufSize);
    }
#endif

    if (pTaskInfo->eDecType != eDecType)
    {
        VPU_MSG_ERR("warning pTaskInfo->eDecType=%x not %x\n",
                    pTaskInfo->eDecType, eDecType);
    }
    goto _SAVE_DEC_TYPE;

_SAVE_DEC_TYPE:
    if (pVPUHalContext->_stVPUStream[u8Offset].eStreamId == (u32Id & 0xFF))
    {
        pVPUHalContext->_stVPUStream[u8Offset].eDecodertype = eDecType;
    }
    else
    {
        VPU_MSG_ERR("Cannot save eDecType!!\n");
    }

    (pVPUHalContext->u8TaskCnt)++;
    _HAL_VPU_Release();
    return TRUE;
}

MS_BOOL HAL_VPU_EX_TaskDelete(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara)
{
    HVD_Return eRet;
#ifdef VDEC3
    HVD_User_Cmd eCmd = E_NST_CMD_DEL_TASK;
#else
    HVD_User_Cmd eCmd = E_DUAL_CMD_DEL_TASK;
#endif
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
    MS_U32 u32Timeout       = HVD_GetSysTime_ms() + 3000;

    _HAL_VPU_Entry();
    VPU_MSG_DBG("DecType=%d\n", pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype);

    eRet = HAL_HVD_EX_SetCmd(u32Id, eCmd, u8OffsetIdx);
    if(eRet != E_HVD_RETURN_SUCCESS)
    {
        VPU_MSG_ERR("VPU fail to DEL Task %d\n", eRet);
    }

    {
        struct _ctl_info *ctl_ptr = (struct _ctl_info *)
                                    MsOS_PA2KSEG1(MsOS_VA2PA(pInitPara->pFWCodeCfg->u32DstAddr) + CTL_INFO_ADDR);
        u32Timeout = HVD_GetSysTime_ms() + VPU_CMD_TIMEOUT;

        MsOS_ReadMemory();

        VPU_MSG_DBG("before: version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                    ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);

        while (CTL_TASK_NONE != ctl_ptr->task_statue[u8OffsetIdx])
        {
            if (HVD_GetSysTime_ms() > u32Timeout)
            {
                VPU_MSG_ERR("Task %u deletion timeout\n", u8OffsetIdx);
                pVPUHalContext->bVpuExLoadFWRlt = FALSE; ///error handling
                VPU_MSG_ERR("Set bVpuExLoadFWRlt as FALSE\n");

                if(pVPUHalContext->u8TaskCnt == 1)
                {
                    VPU_MSG_ERR("Due to one task remain, driver can force delete task\n");
                    break;
                }
                else if(pVPUHalContext->u8TaskCnt == 2)
                {
                    VPU_MSG_ERR("Due to two tasks remain, driver can't force delete task\n");
                    break;
                }
                else
                {
                    VPU_MSG_ERR("Task number is not correct\n");
                    _HAL_VPU_Release();
                    return FALSE;
                }
            }

            MsOS_ReadMemory();
        }

        VPU_MSG_DBG("after: version=0x%x, statue=0x%x, last_ctl_cmd=0x%x, last_ctl_arg=0x%x, t0=%d, t1=%d\n",
                    ctl_ptr->verion, ctl_ptr->statue, ctl_ptr->last_ctl_cmd, ctl_ptr->last_ctl_arg, ctl_ptr->task_statue[0], ctl_ptr->task_statue[1]);
    }
#if SUPPORT_EVD
    if (pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype == E_VPU_EX_DECODER_EVD)
    {
        HAL_EVD_EX_ClearTSPInput(u32Id);
    }
#endif

    pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype = E_VPU_EX_DECODER_NONE;
    if( (u8OffsetIdx == 0) && (pVPUHalContext->_stVPUStream[u8OffsetIdx].eStreamId == E_HAL_VPU_MVC_MAIN_VIEW))
    {
        pVPUHalContext->_stVPUStream[u8OffsetIdx].eStreamId = E_HAL_VPU_N_STREAM0;
        pVPUHalContext->_stVPUStream[0].eDecodertype = E_VPU_EX_DECODER_NONE;
        pVPUHalContext->_stVPUStream[1].eDecodertype = E_VPU_EX_DECODER_NONE;
    }

    if (pVPUHalContext->u8TaskCnt)
    {
        (pVPUHalContext->u8TaskCnt)--;
    }
    else
    {
        VPU_MSG_DBG("Warning: u8TaskCnt=0\n");
    }

    if (0 == pVPUHalContext->u8TaskCnt)
    {
        int i;
        VPU_MSG_DBG("u8TaskCnt=%d time to terminate\n", pVPUHalContext->u8TaskCnt);
        _VPU_EX_DeinitAll(pInitPara);
        HAL_VPU_EX_SetSingleDecodeMode(FALSE);
        pVPUHalContext->u32VPUSHMAddr = 0;

        for (i = 0; i < MAX_SUPPORT_DECODER_NUM; i++)
            pVPUHalContext->u32FWShareInfoAddr[i] = 0xFFFFFFFFUL;
    }
    else
    {
        pVPUHalContext->u32FWShareInfoAddr[u8OffsetIdx] = 0xFFFFFFFFUL;
        _VPU_EX_DeinitHW(pInitPara->pTaskInfo);
    }

    _HAL_VPU_Release();
    return TRUE;
}

MS_BOOL HAL_VPU_EX_LoadCode(VPU_EX_FWCodeCfg *pFWCodeCfg)
{
    MS_VIRT u32DestAddr  = pFWCodeCfg->u32DstAddr;
    MS_VIRT u32BinAddr   = pFWCodeCfg->u32BinAddr;
    MS_U32 u32Size      = pFWCodeCfg->u32BinSize;
#if (ENABLE_DECOMPRESS_FUNCTION==TRUE)
    MS_U32 u32DestSize  = pFWCodeCfg->u32DstSize;
#endif

    if (FALSE == HAL_VPU_EX_GetFWReload())
    {
        //VPRINTF("%s bFWReload FALSE!!!\n", __FUNCTION__);
        if (FALSE == pVPUHalContext->bVpuExLoadFWRlt)
        {
            VPU_MSG_INFO("Never load fw successfully, load it anyway!\n");
        }
        else
        {
            //Check f/w prefix "VDEC30"
            if (_VPU_EX_IsNeedDecompress(u32DestAddr) != FALSE)
            {
                VPU_MSG_ERR("Wrong prefix: reload fw!\n");
            }
            else
            {
                VPU_MSG_INFO("Skip loading fw this time!!!\n");
                return TRUE;
            }
        }
    }

    if (E_HVD_FW_INPUT_SOURCE_FLASH == pFWCodeCfg->u8SrcType)
    {
#if VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM
        if (u32Size != 0)
        {
            SPIDMA_Dev cpyflag = E_SPIDMA_DEV_MIU1;


            MS_U32 u32Start;
            MS_U32 u32StartOffset;
            MS_U8  u8MiuSel;

            _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32DestAddr);


            if(u8MiuSel == E_CHIP_MIU_0)
                cpyflag = E_SPIDMA_DEV_MIU0;
            else if(u8MiuSel == E_CHIP_MIU_1)
                cpyflag = E_SPIDMA_DEV_MIU1;
            else if(u8MiuSel == E_CHIP_MIU_2)
                ; ///TODO:  cpyflag = E_SPIDMA_DEV_MIU2;

            if (!HVD_FLASHcpy(MsOS_VA2PA(u32DestAddr), MsOS_VA2PA(u32BinAddr), u32Size, cpyflag))
            {
                goto _load_code_fail;
            }
        }
        else
        {
            goto _load_code_fail;
        }
#else
        goto _load_code_fail;
#endif
    }
    else if (E_HVD_FW_INPUT_SOURCE_DRAM == pFWCodeCfg->u8SrcType)
    {
        if (u32BinAddr != 0 && u32Size != 0)
        {
#if (ENABLE_DECOMPRESS_FUNCTION==TRUE)
            if(_VPU_EX_DecompressBin(u32BinAddr, u32Size, u32DestAddr, u32DestAddr + u32DestSize - WINDOW_SIZE) == TRUE)
            {
                if(_VPU_EX_IsNeedDecompress(u32DestAddr) == FALSE)
                {
                    VPU_MSG_INFO("Decompress ok!!!\n");
                }
                else
                {
                    VPU_MSG_INFO("Decompress fail!!!\n");
                }
            }
            else
#endif
            {
                HVD_memcpy(u32DestAddr, u32BinAddr, u32Size);
            }
        }
        else
        {
            goto _load_code_fail;
        }
    }
    else
    {
#if VPU_ENABLE_EMBEDDED_FW_BINARY
        VPU_MSG_INFO("Load FW inD2D: dest=0x%lx, source=0x%lx, size=%d\n",
                     (unsigned long)u32DestAddr, ((unsigned long) u8HVD_FW_Binary),
                     (MS_U32) sizeof(u8HVD_FW_Binary));

#if (ENABLE_DECOMPRESS_FUNCTION==TRUE)
        if(_VPU_EX_DecompressBin((MS_VIRT)u8HVD_FW_Binary, (MS_U32)sizeof(u8HVD_FW_Binary), u32DestAddr, u32DestAddr + u32DestSize - WINDOW_SIZE) == TRUE)
        {
            if(_VPU_EX_IsNeedDecompress(u32DestAddr) == FALSE)
            {
                VPU_MSG_INFO("Decompress ok!!!\n");
            }
            else
            {
                VPU_MSG_INFO("Decompress fail!!!\n");
            }
        }
        else
#endif
        {
            HVD_memcpy(u32DestAddr, (MS_VIRT)u8HVD_FW_Binary, sizeof(u8HVD_FW_Binary));
        }
#else
        goto _load_code_fail;
#endif
    }

    MAsm_CPU_Sync();
    MsOS_FlushMemory();

    if (FALSE == (*((MS_U8*)(u32DestAddr + 6)) == 'R' && *((MS_U8*)(u32DestAddr + 7)) == '2'))
    {
        VPU_MSG_ERR("FW is not R2 version! _%x_ _%x_\n", *(MS_U8*)(u32DestAddr + 6), *(MS_U8*)(u32DestAddr + 7));
        goto _load_code_fail;
    }

    pVPUHalContext->bVpuExLoadFWRlt = TRUE;
    return TRUE;

_load_code_fail:
    pVPUHalContext->bVpuExLoadFWRlt = FALSE;
    return FALSE;
}

void HAL_VPU_EX_InitRegBase(MS_VIRT u32RegBase)
{
    u32VPURegOSBase = u32RegBase;
}

MS_BOOL HAL_VPU_EX_Init_Share_Mem(void)
{
#if ((defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_ECOS)) && (!defined(SUPPORT_X_MODEL_FEATURE)))

    MS_U32 u32ShmId;
    MS_VIRT u32Addr;
    MS_U32 u32BufSize;


    if (FALSE == MsOS_SHM_GetId( (MS_U8*)"Linux HAL VPU",
                                 sizeof(VPU_Hal_CTX),
                                 &u32ShmId,
                                 &u32Addr,
                                 &u32BufSize,
                                 MSOS_SHM_QUERY))
    {
        if (FALSE == MsOS_SHM_GetId((MS_U8*)"Linux HAL VPU",
                                    sizeof(VPU_Hal_CTX),
                                    &u32ShmId,
                                    &u32Addr,
                                    &u32BufSize,
                                    MSOS_SHM_CREATE))
        {
            VPU_MSG_ERR("[%s]SHM allocation failed!!!use global structure instead!!!\n", __FUNCTION__);
            if(pVPUHalContext == NULL)
            {
                pVPUHalContext = &gVPUHalContext;
                memset(pVPUHalContext, 0, sizeof(VPU_Hal_CTX));
                _VPU_EX_Context_Init();
                VPRINTF("[%s]Global structure init Success!!!\n", __FUNCTION__);
            }
            else
            {
                VPRINTF("[%s]Global structure exists!!!\n", __FUNCTION__);
            }
            //return FALSE;
        }
        else
        {
            memset((MS_U8*)u32Addr, 0, sizeof(VPU_Hal_CTX));
            pVPUHalContext = (VPU_Hal_CTX*)u32Addr; // for one process
            _VPU_EX_Context_Init();
        }
    }
    else
    {
        pVPUHalContext = (VPU_Hal_CTX*)u32Addr; // for another process
    }
#else
    if(pVPUHalContext == NULL)
    {
        pVPUHalContext = &gVPUHalContext;
        memset(pVPUHalContext, 0, sizeof(VPU_Hal_CTX));
        _VPU_EX_Context_Init();
    }
#endif

    return TRUE;

}

HAL_VPU_StreamId HAL_VPU_EX_GetFreeStream(HAL_VPU_StreamType eStreamType)
{
    MS_U32 i = 0;

    _HAL_VPU_MutexCreate();

    _HAL_VPU_Entry();

    if (E_HAL_VPU_MVC_STREAM == eStreamType)
    {
        if((E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[0].eDecodertype) && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[1].eDecodertype))
        {
            pVPUHalContext->_stVPUStream[0].eStreamId = E_HAL_VPU_MVC_MAIN_VIEW;
            pVPUHalContext->_stVPUStream[0].eDecodertype = E_VPU_EX_DECODER_GET_MVC;
            pVPUHalContext->_stVPUStream[1].eDecodertype = E_VPU_EX_DECODER_GET_MVC;
            _HAL_VPU_Release();
            return pVPUHalContext->_stVPUStream[0].eStreamId;       /// Need to check
        }
    }
    else if (E_HAL_VPU_MAIN_STREAM == eStreamType)
    {
        for (i = 0; i < MAX_SUPPORT_DECODER_NUM; i++)
        {
            if ((E_HAL_VPU_MAIN_STREAM_BASE & pVPUHalContext->_stVPUStream[i].eStreamId)
                    && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[i].eDecodertype))
            {
                pVPUHalContext->_stVPUStream[i].eDecodertype = E_VPU_EX_DECODER_GET;
                _HAL_VPU_Release();
                return pVPUHalContext->_stVPUStream[i].eStreamId;
            }
        }
    }
    else if (E_HAL_VPU_SUB_STREAM == eStreamType)
    {
        for (i = 0; i < MAX_SUPPORT_DECODER_NUM; i++)
        {
            if ((E_HAL_VPU_SUB_STREAM_BASE & pVPUHalContext->_stVPUStream[i].eStreamId)
                    && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[i].eDecodertype))
            {
                pVPUHalContext->_stVPUStream[i].eDecodertype = E_VPU_EX_DECODER_GET;
                _HAL_VPU_Release();
                return pVPUHalContext->_stVPUStream[i].eStreamId;
            }
        }
    }
#ifdef VDEC3
    else if (eStreamType >= E_HAL_VPU_N_STREAM && eStreamType < (E_HAL_VPU_N_STREAM + VPU_MAX_DEC_NUM))
    {
#if 1 // bound FW task to main/sub stream
        i = eStreamType - E_HAL_VPU_N_STREAM;
        if (pVPUHalContext->_stVPUStream[i].eDecodertype == E_VPU_EX_DECODER_NONE)
        {
            pVPUHalContext->_stVPUStream[i].eDecodertype = E_VPU_EX_DECODER_GET;
            _HAL_VPU_Release();
            return pVPUHalContext->_stVPUStream[i].eStreamId;
        }
#else // dynamic select FW task id
        for (i = 0; i < MAX_SUPPORT_DECODER_NUM; i++)
        {
            if ((E_HAL_VPU_N_STREAM_BASE & pVPUHalContext->_stVPUStream[i].eStreamId)
                    && (E_VPU_EX_DECODER_NONE == pVPUHalContext->_stVPUStream[i].eDecodertype))
            {
                return pVPUHalContext->_stVPUStream[i].eStreamId;
            }
        }
#endif
    }
#endif

    _HAL_VPU_Release();

    return E_HAL_VPU_STREAM_NONE;
}

MS_BOOL HAL_VPU_EX_ReleaseFreeStream(MS_U8 u8Idx)
{
    _HAL_VPU_Entry();

    if(pVPUHalContext->_stVPUStream[u8Idx].eDecodertype == E_VPU_EX_DECODER_GET_MVC)
    {
        pVPUHalContext->_stVPUStream[0].eDecodertype = E_VPU_EX_DECODER_NONE;
        pVPUHalContext->_stVPUStream[1].eDecodertype = E_VPU_EX_DECODER_NONE;
    }
    else if(pVPUHalContext->_stVPUStream[u8Idx].eDecodertype == E_VPU_EX_DECODER_GET)
    {
        pVPUHalContext->_stVPUStream[u8Idx].eDecodertype = E_VPU_EX_DECODER_NONE;
    }

    _HAL_VPU_Release();

    return TRUE;
}

MS_U8 HAL_VPU_EX_CheckFreeStream(VPU_EX_Original_Stream eStream)
{
    MS_U8 u8Idx = 0;

    //VPRINTF("[NDec][%s][%d] eStream = %d\n", __FUNCTION__, __LINE__, eStream);

    if(eStream == E_VPU_ORIGINAL_MAIN_STREAM)
    {
        if(pVPUHalContext->_stVPUStream[0].eDecodertype == E_VPU_EX_DECODER_NONE)
        {
            //VPRINTF("[NDec][%s][%d] main stream using u8Idx = 0\n", __FUNCTION__, __LINE__);
            return 0;
        }
    }
    else if(eStream == E_VPU_ORIGINAL_SUB_STREAM)
    {
        if(pVPUHalContext->_stVPUStream[1].eDecodertype == E_VPU_EX_DECODER_NONE)
        {
            //VPRINTF("[NDec][%s][%d] sub stream using u8Idx = 1\n", __FUNCTION__, __LINE__);
            return 1;
        }
    }

    for (u8Idx = 0; u8Idx < MAX_SUPPORT_DECODER_NUM; u8Idx++)
    {
        if (pVPUHalContext->_stVPUStream[u8Idx].eDecodertype == E_VPU_EX_DECODER_NONE)
            break;
    }

    if (u8Idx >= MAX_SUPPORT_DECODER_NUM)
    {
        VPU_MSG_ERR("all vpu free streams are occupied \n");
        return 0xFF;
    }

    VPU_MSG_DBG("available vpu free stream %d \n", u8Idx);
    return u8Idx;
}

MS_BOOL HAL_VPU_EX_Init(VPU_EX_InitParam *InitParams)
{
    VPU_MSG_DBG("Inv=%d, clk=%d\n", InitParams->bClockInv, InitParams->eClockSpeed);

    // enable module
    _VPU_EX_ClockInv(InitParams->bClockInv);
    _VPU_EX_ClockSpeed(InitParams->eClockSpeed);
    HAL_VPU_EX_PowerCtrl(TRUE);

#ifdef CONFIG_MSTAR_CLKM
    HAL_VPU_EX_SetClkManagement(E_VPU_EX_CLKPORT_VD_MHEG5, TRUE);
#endif

#ifdef CONFIG_MSTAR_SRAMPD
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, CODEC_SRAM_HVD_R2, CODEC_SRAM_HVD_R2);
    HVD_Delay_ms(1);
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, CODEC_SRAM_HVD_R2_MIU0_BWP, CODEC_SRAM_HVD_R2_MIU0_BWP);
    HVD_Delay_ms(1);
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, CODEC_SRAM_HVD_R2_MIU1_BWP, CODEC_SRAM_HVD_R2_MIU1_BWP);
    HVD_Delay_ms(1);
#endif
#if 1                           //Create VPU's own mutex
    //_HAL_VPU_MutexCreate();
#else
    pVPUHalContext->s32VPUMutexID = InitParams->s32VPUMutexID;
    pVPUHalContext->u32VPUMutexTimeOut = InitParams->u32VPUMutexTimeout;
#endif

    return TRUE;
}

MS_BOOL HAL_VPU_EX_DeInit(void)
{
#if 0
    if (0 != _VPU_EX_GetActiveCodecCnt())
#else
    if (pVPUHalContext->u8TaskCnt != 0)
#endif
    {
        VPU_MSG_DBG("do nothing since codec is active %d\n", pVPUHalContext->u8TaskCnt);
        return TRUE;
    }

    memset(&(pVPUHalContext->_stVPUDecMode), 0, sizeof(VPU_EX_DecModCfg));

#ifndef _DISABLE_CLK_CTL
#ifdef CONFIG_MSTAR_CLKM
    HAL_VPU_EX_SetClkManagement(E_VPU_EX_CLKPORT_VD_MHEG5, FALSE);
#else
    HAL_VPU_EX_PowerCtrl(FALSE);
#endif
#endif

#ifdef CONFIG_MSTAR_SRAMPD
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, ~CODEC_SRAM_HVD_R2, CODEC_SRAM_HVD_R2);
    HVD_Delay_ms(1);
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, ~CODEC_SRAM_HVD_R2_MIU0_BWP, CODEC_SRAM_HVD_R2_MIU0_BWP);
    HVD_Delay_ms(1);
    _VPU_WriteWordMask(REG_CODEC_SRAM_SD_EN, ~CODEC_SRAM_HVD_R2_MIU1_BWP, CODEC_SRAM_HVD_R2_MIU1_BWP);
    HVD_Delay_ms(1);
#endif

    HAL_VPU_EX_SwRelseMAU();
    //_HAL_VPU_MutexDelete();

    return TRUE;
}

void HAL_VPU_EX_PowerCtrl(MS_BOOL bEnable)
{
#ifndef _DISABLE_CLK_CTL
    if (bEnable)
    {
        _VPU_WriteWordMask(REG_TOP_VPU, 0, TOP_CKG_VPU_DIS);
        _VPU_WriteWordMask(REG_TOP_VPU, VPU_ICG_EN, VPU_ICG_EN);
        //_VPU_WriteWordMask( REG_CHIPTOP_DUMMY_CODEC, REG_CHIPTOP_DUMMY_CODEC_ENABLE, REG_CHIPTOP_DUMMY_CODEC_ENABLE);
        pVPUHalContext->_bVPUPowered = TRUE;
    }
    else
    {
        _VPU_WriteWordMask(REG_TOP_VPU, TOP_CKG_VPU_DIS, TOP_CKG_VPU_DIS);
        //_VPU_WriteWordMask( REG_CHIPTOP_DUMMY_CODEC, 0, REG_CHIPTOP_DUMMY_CODEC_ENABLE);
        pVPUHalContext->_bVPUPowered = FALSE;
    }
#endif
}

void HAL_VPU_EX_MIU_RW_Protect(MS_BOOL bEnable)
{
    _VPU_MIU_SetReqMask(VPU_D_RW, bEnable);
    _VPU_MIU_SetReqMask(VPU_Q_RW, bEnable);
    _VPU_MIU_SetReqMask(VPU_I_R, bEnable);
    VPU_EX_TimerDelayMS(1);
}

///-----------------------------------------------------------------------------
/// config AVCH264 CPU
/// @param u32StAddr \b IN: CPU binary code base address in DRAM.
/// @param u8dlend_en \b IN: endian
///     - 1, little endian
///     - 0, big endian
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_CPUSetting(MS_PHY u32StAddr)
{
    MS_BOOL bRet = TRUE;
    MS_U32 u32Offset = 0;
    MS_U16 tempreg = 0;
    MS_U8  u8MiuSel;
    MS_U16 r2_miu_sel;
    //MS_U32 u32TmpStartOffset;

    _phy_to_miu_offset(u8MiuSel, u32Offset, u32StAddr);

    _VPU_Write2Byte(VPU_REG_SPI_BASE,  0xC000);
    _VPU_WriteWordMask( VPU_REG_MIU_LAST , 0 , VPU_REG_MIU_LAST_EN );
    _VPU_WriteWordMask( VPU_REG_CPU_SETTING , 0 , VPU_REG_CPU_SPI_BOOT );
    _VPU_WriteWordMask( VPU_REG_CPU_SETTING , 0 , VPU_REG_CPU_SDRAM_BOOT );
    _VPU_Write2Byte(VPU_REG_DQMEM_MASK_L,  0x8000); // For DQMEM 28KB
    _VPU_Write2Byte(VPU_REG_DQMEM_MASK_H,  0xffff);
    _VPU_Write2Byte(VPU_REG_IO1_BASE,  0xf900); // UART BASE
    _VPU_Write2Byte(VPU_REG_IO2_BASE,  0xf000);
    _VPU_Write2Byte(VPU_REG_DQMEM_BASE_L,  0x0000);
    _VPU_Write2Byte(VPU_REG_DQMEM_BASE_H, 0xf200);
    _VPU_Write2Byte(VPU_REG_CPU_BASE_L, 0x1000);

#if (VPU_ENABLE_IQMEM)
    _VPU_Write2Byte(VPU_REG_IQMEM_BASE_L, (MS_U16)(VPU_IQMEM_BASE & 0x0000ffff));
    _VPU_Write2Byte(VPU_REG_IQMEM_BASE_H, (MS_U16)((VPU_IQMEM_BASE >> 16) & 0xffff));
#endif

#if (VPU_FORCE_MIU_MODE)
    // Data sram base Unit: byte address
    _VPU_Write2Byte(VPU_REG_DCU_SDR_BASE_L, (MS_U16)(u32Offset & 0x0000ffff)) ;
    _VPU_Write2Byte(VPU_REG_DCU_SDR_BASE_H, (MS_U16)((u32Offset >> 16) & 0xffff));
    // Instruction sram base Unit: byte address
    _VPU_Write2Byte(VPU_REG_ICU_SDR_BASE_L, (MS_U16)(u32Offset & 0x0000ffff)) ;
    _VPU_Write2Byte(VPU_REG_ICU_SDR_BASE_H, (MS_U16)((u32Offset >> 16) & 0xffff));

#ifndef HAL_FEATURE_MAU
    r2_miu_sel = (_VPU_Read2Byte(VPU_REG_R2_MI_SEL_BASE) & 0xfff);
#endif
    VPU_MSG_INFO("\033[1;32m[%s] %d  u8MiuSel = %d  r2_miu_sel = 0x%x \033[m\n", __FUNCTION__, __LINE__, u8MiuSel, r2_miu_sel);

    //use force miu mode
    if(u8MiuSel == E_CHIP_MIU_0)
    {
#ifdef HAL_FEATURE_MAU
        _VPU_Write2Byte(MAU1_MIU_SEL, 0x8900);
        _VPU_Write2Byte(MAU1_LV2_0_MIU_SEL, 0x8900);
        _VPU_Write2Byte(MAU1_LV2_1_MIU_SEL, 0x8900);
#else
        _VPU_Write2Byte(VPU_REG_R2_MI_SEL_BASE, r2_miu_sel);//1 Manhattan has no MAU, use this register to select miu
#endif
    }
    else if(u8MiuSel == E_CHIP_MIU_1)
    {
#ifdef HAL_FEATURE_MAU
        _VPU_Write2Byte(MAU1_MIU_SEL, 0x8900);
        _VPU_Write2Byte(MAU1_LV2_0_MIU_SEL, 0x8b00);
        _VPU_Write2Byte(MAU1_LV2_1_MIU_SEL, 0x8900);
#else
        _VPU_Write2Byte(VPU_REG_R2_MI_SEL_BASE, r2_miu_sel | 0x5000);
#endif
    }
    else //miu 2
    {
#ifdef HAL_FEATURE_MAU
        _VPU_Write2Byte(MAU1_MIU_SEL, 0x8b00);
        _VPU_Write2Byte(MAU1_LV2_0_MIU_SEL, 0x8900);
        _VPU_Write2Byte(MAU1_LV2_1_MIU_SEL, 0x8900);
#else
        _VPU_Write2Byte(VPU_REG_R2_MI_SEL_BASE, r2_miu_sel | 0xa000);
#endif
    }
#else
    ///TODO:
#endif


    tempreg = _VPU_Read2Byte(VPU_REG_CONTROL_SET);
    tempreg |= VPU_REG_IO2_EN;
    tempreg |= VPU_REG_QMEM_SPACE_EN;
    _VPU_Write2Byte(VPU_REG_CONTROL_SET, tempreg);

    return bRet;
}

///-----------------------------------------------------------------------------
/// Set IQMem data access mode or instruction fetch mode
/// @param u8dlend_en \b IN: endian
///     - 1, switch to data access mode
///     - 0, switch to instruction fetch mode
///-----------------------------------------------------------------------------
void HAL_VPU_EX_IQMemSetDAMode(MS_BOOL bEnable)
{

    if(bEnable)
    {

        _VPU_Write2Byte(VPU_REG_IQMEM_SETTING, _VPU_Read2Byte(VPU_REG_IQMEM_SETTING) | 0x10);
        _VPU_Write2Byte(VPU_REG_QMEM_OWNER, _VPU_Read2Byte(VPU_REG_QMEM_OWNER) | 0x20);

    }
    else
    {

        _VPU_Write2Byte(VPU_REG_IQMEM_SETTING, _VPU_Read2Byte(VPU_REG_IQMEM_SETTING) & (~0x10));
        _VPU_Write2Byte(VPU_REG_QMEM_OWNER, _VPU_Read2Byte(VPU_REG_QMEM_OWNER) & (~0x20));
    }
}

///-----------------------------------------------------------------------------
/// H.264 SW reset
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
///-----------------------------------------------------------------------------

MS_BOOL HAL_VPU_EX_SwRst(MS_BOOL bCheckMauIdle)
{
    MS_U16 tempreg = 0, tempreg1 = 0;
    MS_U16 idle_cnt;
    tempreg = _VPU_Read2Byte(VPU_REG_CPU_CONFIG);
    tempreg |= VPU_REG_CPU_STALL_EN;
    _VPU_Write2Byte(VPU_REG_CPU_CONFIG, tempreg);

    tempreg = _VPU_Read2Byte(VPU_REG_CPU_SETTING);
    // 0xf means VPU is not stalled
    if (tempreg & 0xf || pVPUHalContext->u8ForceRst == 1)
    {
        pVPUHalContext->u8ForceRst = 0;
        // write R2 RIU registers to select DCU/ICU debug data
        // Writing these registers here provides enough time for them to
        // take effect.
        tempreg1 = _VPU_Read2Byte(VPU_REG_DCU_DBG_SEL);
        tempreg1 |= VPU_REG_DCU_DBG_SEL_0 | VPU_REG_DCU_DBG_SEL_1;
        _VPU_Write2Byte(VPU_REG_DCU_DBG_SEL, tempreg1);
        _VPU_Write2Byte(VPU_REG_ICU_DBG_SEL, 0);

        // wait at least 1ms for VPU_REG_CPU_STALL_EN to take effect
        // This step is important because in the next step we want to make
        // sure "DCU is not replaying when R2 is stalled".
        idle_cnt = 100;
        do
        {
            if (--idle_cnt == 0)
            {
                VPRINTF("VPU_REG_CPU_STALL_EN is not set\n");
                break;
            }
            MsOS_DelayTask(1);
        }
        while ((_VPU_Read2Byte(VPU_REG_CPU_CONFIG) & VPU_REG_CPU_STALL_EN) == 0);
        // check CPU status: DCU should NOT be replaying
        // If R2 has been stalled, we can guarantee that if we found DCU is
        // NOT replaying, it will NOT replay later even CPU is going to issue
        // a load/store instruction.
        idle_cnt = 100;
        while (_VPU_Read2Byte(VPU_REG_CPU_STATUS) & VPU_REG_CPU_D_REPLAY)
        {
            if (--idle_cnt == 0)
            {
                VPRINTF("DCU is replaying\n");
                break;
            }
            MsOS_DelayTask(1);
        }
        // wait 1ms to prevent race condition between (1) DCU is not
        // replaying, and (2) BIU start to doing new job or ICU start to
        // fetch new instruction
        MsOS_DelayTask(1);

        // check BIU should be empty
        idle_cnt = 100;
        while ( (_VPU_Read2Byte(VPU_REG_DCU_STATUS) & VPU_REG_BIU_EMPTY) == 0 )
        {
            if (--idle_cnt == 0)
            {
                VPRINTF("BIU DCU idle time out~~~~~\n");
                break;
            }
            MsOS_DelayTask(1);
        }

        // check CPU is not requesting ICU
        idle_cnt = 100;
        while (_VPU_Read2Byte(VPU_REG_ICU_DBG_DAT0) & VPU_REG_ICPU_REQ)
        {
            if (--idle_cnt == 0)
            {
                VPRINTF("CPU keeps requesting ICU\n");
                break;
            }
            MsOS_DelayTask(1);
        }

        // wait 1ms to avoid race condition of (1) CPU stop requesting ICU, and
        // (2) ISB start to fetch
        MsOS_DelayTask(1);

        // check ISB should be idle
        idle_cnt = 100;
        while ( (_VPU_Read2Byte(VPU_REG_ICU_STATUS) & VPU_REG_ISB_IDLE) == 0 )
        {
            if (--idle_cnt == 0)
            {
                VPRINTF("ISB is busy\n");
                break;
            }
            MsOS_DelayTask(1);
        }
    }
#ifdef HAL_FEATURE_MAU
//MAU has been removed since manhattan, so it is not necessary to check MAU status
    if (bCheckMauIdle)
    {
        MS_U32 mau_idle_cnt = 100;// ms
        while (mau_idle_cnt)
        {
            if (TRUE == _VPU_EX_MAU_IDLE())
            {
                break;
            }
            mau_idle_cnt--;
            MsOS_DelayTask(1);
        }

        if (mau_idle_cnt == 0)
        {
            VPRINTF("MAU idle time out~~~~~\n");
        }
    }
#endif

    // this command set MIU to block R2 (does not ack R2's request)
    HAL_VPU_EX_MIU_RW_Protect(TRUE);

#ifdef HAL_FEATURE_MAU
    // reset MAU
    tempreg1 = _VPU_Read2Byte(MAU1_CPU_RST);
    tempreg1 |= MAU1_REG_SW_RESET;
    _VPU_Write2Byte(MAU1_CPU_RST, tempreg1);
#if defined(UDMA_FPGA_ENVI)
    tempreg = _VPU_Read2Byte(VPU_REG_RESET);
    _VPU_Write2Byte(VPU_REG_RESET, (tempreg & 0xfffd));
#endif
#endif

    // reset R2
    // We should trigger MIU reset before R2 reset. If we set MIU/R2 reset
    // by the same RIU write, the R2 reset signal may reach afifo eralier
    // than MIU reset and afifo write pointer will be reset to position 0.
    // In this case, afifo consider it is not empty because read/write
    // pointer are mismatch and then BIU sends out unpredicted MIU request.
    tempreg = _VPU_Read2Byte(VPU_REG_CPU_SETTING);
    tempreg &= ~VPU_REG_CPU_MIU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
    VPU_EX_TimerDelayMS(1);
    tempreg &= ~VPU_REG_CPU_R2_EN;
    tempreg &= ~VPU_REG_CPU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);

    VPU_EX_TimerDelayMS(1);

    // this command set MIU to accept R2 (can ack R2's request)
    HAL_VPU_EX_MIU_RW_Protect(FALSE);

    pVPUHalContext->_bVPURsted = FALSE;
    return TRUE;
}

/*
MS_BOOL HAL_VPU_EX_SwRst(MS_BOOL bCheckMauIdle)
{
    MS_U16 tempreg = 0, tempreg1 = 0;
#ifndef HAL_FEATURE_MAU
    tempreg = _VPU_Read2Byte(VPU_REG_CPU_CONFIG);
    tempreg |= VPU_REG_CPU_STALL_EN;
    _VPU_Write2Byte(VPU_REG_CPU_CONFIG, tempreg);

     MS_U32 idle_cnt = 100;// ms
     while (idle_cnt)
     {
         if (_VPU_Read2Byte(VPU_REG_ICU_STATUS) & (VPU_REG_ISB_IDLE | VPU_REG_ICU_IDLE))
         {
             break;
         }
         idle_cnt--;
         MsOS_DelayTask(1);
     }

     if (idle_cnt == 0)
     {
         VPRINTF("ISB ICU idle time out~~~~~\n");
     }

    tempreg1 = _VPU_Read2Byte(VPU_REG_DCU_DBG_SEL);
    tempreg1 |= VPU_REG_DCU_DBG_SEL_0;
    tempreg1 |= VPU_REG_DCU_DBG_SEL_1;
    _VPU_Write2Byte(VPU_REG_DCU_DBG_SEL, tempreg1);

    MS_U32 idle_cnt_1 = 100;// ms
     while (idle_cnt_1)
     {
         if (_VPU_Read2Byte(VPU_REG_DCU_STATUS) & (VPU_REG_BIU_EMPTY))
         {
             break;
         }
         idle_cnt_1--;
         MsOS_DelayTask(1);
     }

     if (idle_cnt_1 == 0)
     {
         VPRINTF("BIU DCU idle time out~~~~~\n");
     }

    tempreg = _VPU_Read2Byte(VPU_REG_CPU_SETTING);
    tempreg &= ~VPU_REG_CPU_MIU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
    VPU_EX_TimerDelayMS(1);
    tempreg &= ~VPU_REG_CPU_R2_EN;
    tempreg &= ~VPU_REG_CPU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);

#else
//MAU has been removed since manhattan, so it is not necessary to check MAU status

    if (bCheckMauIdle)
    {
        MS_U32 mau_idle_cnt = 100;// ms
        while (mau_idle_cnt)
        {
            if (TRUE == _VPU_EX_MAU_IDLE())
            {
                break;
            }
            mau_idle_cnt--;
            MsOS_DelayTask(1);
        }

        if (mau_idle_cnt == 0)
        {
            VPRINTF("MAU idle time out~~~~~\n");
        }
    }


    HAL_VPU_EX_MIU_RW_Protect(TRUE);

    tempreg1 = _VPU_Read2Byte(MAU1_CPU_RST);
    tempreg1 |= MAU1_REG_SW_RESET;
    _VPU_Write2Byte(MAU1_CPU_RST, tempreg1);

#if defined(UDMA_FPGA_ENVI)
    tempreg = _VPU_Read2Byte(VPU_REG_RESET);
   _VPU_Write2Byte(VPU_REG_RESET, (tempreg& 0xfffd));
#endif

    tempreg = _VPU_Read2Byte(VPU_REG_CPU_SETTING);
    tempreg &= ~VPU_REG_CPU_R2_EN;
    tempreg &= ~VPU_REG_CPU_SW_RSTZ;
    tempreg &= ~VPU_REG_CPU_MIU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
#endif
    VPU_EX_TimerDelayMS(1);
    HAL_VPU_EX_MIU_RW_Protect(FALSE);

    pVPUHalContext->_bVPURsted = FALSE;
    return TRUE;
}
*/
///-----------------------------------------------------------------------------
/// CPU reset release
///-----------------------------------------------------------------------------
void HAL_VPU_EX_SwRstRelse(void)
{
    MS_U16 tempreg = 0;

    tempreg = _VPU_Read2Byte(VPU_REG_CPU_CONFIG);
    tempreg &= ~VPU_REG_CPU_STALL_EN;
    _VPU_Write2Byte(VPU_REG_CPU_CONFIG, tempreg);

    tempreg = _VPU_Read2Byte(VPU_REG_CPU_SETTING);
    tempreg |= VPU_REG_CPU_MIU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
    VPU_EX_TimerDelayMS(1);
    tempreg |= VPU_REG_CPU_SW_RSTZ;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
    VPU_EX_TimerDelayMS(1);
    tempreg |= VPU_REG_CPU_R2_EN;
    _VPU_Write2Byte(VPU_REG_CPU_SETTING, tempreg);
#ifdef HAL_FEATURE_MAU
    MS_U16 tempreg1 = 0;
    tempreg1 = _VPU_Read2Byte(MAU1_CPU_RST);
    tempreg1 &= ~MAU1_REG_SW_RESET;
    _VPU_Write2Byte(MAU1_CPU_RST, tempreg1);
#endif
    pVPUHalContext->_bVPURsted = TRUE;
}

void HAL_VPU_EX_SwRelseMAU(void)
{

#ifdef HAL_FEATURE_MAU
    MS_U16 tempreg = 0;
    tempreg = _VPU_Read2Byte(MAU1_CPU_RST);
    tempreg &= ~MAU1_REG_SW_RESET;
    _VPU_Write2Byte(MAU1_CPU_RST, tempreg);
#endif
}

MS_U32 HAL_VPU_EX_MemRead(MS_VIRT u32Addr)
{
    MS_U32 u32value = 0;

    return u32value;
}

MS_BOOL HAL_VPU_EX_MemWrite(MS_VIRT u32Addr, MS_U32 u32value)
{
    MS_BOOL bRet = TRUE;

    return bRet;
}

///-----------------------------------------------------------------------------
/// Check AVCH264 Ready or not
/// @return TRUE or FALSE
///     - TRUE, MailBox is free
///     - FALSE, MailBox is busy
/// @param u8MBox \b IN: MailBox to check
///     - AVCH264_HI_MBOX0,
///     - AVCH264_HI_MBOX1,
///     - AVCH264_RISC_MBOX0,
///     - AVCH264_RISC_MBOX1,
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_MBoxRdy(MS_U32 u32type)
{
    MS_BOOL bResult = FALSE;

    switch (u32type)
    {
        case VPU_HI_MBOX0:
            bResult = (_VPU_Read2Byte(VPU_REG_HI_MBOX_RDY) & VPU_REG_HI_MBOX0_RDY) ? FALSE : TRUE;
            break;
        case VPU_HI_MBOX1:
            bResult = (_VPU_Read2Byte(VPU_REG_HI_MBOX_RDY) & VPU_REG_HI_MBOX1_RDY) ? FALSE : TRUE;
            break;
        case VPU_RISC_MBOX0:
            bResult = (_VPU_Read2Byte(VPU_REG_RISC_MBOX_RDY) & VPU_REG_RISC_MBOX0_RDY) ? TRUE : FALSE;
            break;
        case VPU_RISC_MBOX1:
            bResult = (_VPU_Read2Byte(VPU_REG_RISC_MBOX_RDY) & VPU_REG_RISC_MBOX1_RDY) ? TRUE : FALSE;
            break;
        default:
            break;
    }
    return bResult;
}


///-----------------------------------------------------------------------------
/// Read message from AVCH264
/// @return TRUE or FALSE
///     - TRUE, success
///     - FALSE, failed
/// @param u8MBox \b IN: MailBox to read
///     - AVCH264_RISC_MBOX0
///     - AVCH264_RISC_MBOX1
/// @param u32Msg \b OUT: message read
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_MBoxRead(MS_U32 u32type, MS_U32 * u32Msg)
{
    MS_BOOL bResult = TRUE;

    switch (u32type)
    {
        case VPU_HI_MBOX0:
            *u32Msg = ((MS_U32) (_VPU_Read2Byte(VPU_REG_HI_MBOX0_H)) << 16) |
                      ((MS_U32) (_VPU_Read2Byte(VPU_REG_HI_MBOX0_L)));
            break;
        case VPU_HI_MBOX1:
            *u32Msg = ((MS_U32) (_VPU_Read2Byte(VPU_REG_HI_MBOX1_H)) << 16) |
                      ((MS_U32) (_VPU_Read2Byte(VPU_REG_HI_MBOX1_L)));
            break;
        case VPU_RISC_MBOX0:
            *u32Msg = ((MS_U32) (_VPU_Read2Byte(VPU_REG_RISC_MBOX0_H)) << 16) |
                      ((MS_U32) (_VPU_Read2Byte(VPU_REG_RISC_MBOX0_L)));
            break;
        case VPU_RISC_MBOX1:
            *u32Msg = ((MS_U32) (_VPU_Read2Byte(VPU_REG_RISC_MBOX1_H)) << 16) |
                      ((MS_U32) (_VPU_Read2Byte(VPU_REG_RISC_MBOX1_L)));
            break;
        default:
            *u32Msg = 0;
            bResult = FALSE;
            break;
    }
    return bResult;
}

///-----------------------------------------------------------------------------
/// Mailbox from AVCH264 clear bit resest
///-----------------------------------------------------------------------------
void HAL_VPU_EX_MBoxClear(MS_U32 u32type)
{
    switch (u32type)
    {
        case VPU_RISC_MBOX0:
            _VPU_WriteWordMask(VPU_REG_RISC_MBOX_CLR, VPU_REG_RISC_MBOX0_CLR, VPU_REG_RISC_MBOX0_CLR);
            break;
        case VPU_RISC_MBOX1:
            _VPU_WriteWordMask(VPU_REG_RISC_MBOX_CLR, VPU_REG_RISC_MBOX1_CLR, VPU_REG_RISC_MBOX1_CLR);
            break;
        default:
            break;
    }
}

///-----------------------------------------------------------------------------
/// Send message to AVCH264
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
/// @param u8MBox \b IN: MailBox
///     - AVCH264_HI_MBOX0,
///     - AVCH264_HI_MBOX1,
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_MBoxSend(MS_U32 u32type, MS_U32 u32Msg)
{
    MS_BOOL bResult = TRUE;

    VPU_MSG_DBG("type=%u, msg=0x%x\n", u32type, u32Msg);

    switch (u32type)
    {
        case VPU_HI_MBOX0:
        {
            _VPU_Write4Byte(VPU_REG_HI_MBOX0_L, u32Msg);
            _VPU_WriteWordMask(VPU_REG_HI_MBOX_SET, VPU_REG_HI_MBOX0_SET, VPU_REG_HI_MBOX0_SET);
            break;
        }
        case VPU_HI_MBOX1:
        {
            _VPU_Write4Byte(VPU_REG_HI_MBOX1_L, u32Msg);
            _VPU_WriteWordMask(VPU_REG_HI_MBOX_SET, VPU_REG_HI_MBOX1_SET, VPU_REG_HI_MBOX1_SET);
            break;
        }
        default:
        {
            bResult = FALSE;
            break;
        }
    }

    return bResult;
}

MS_U32 HAL_VPU_EX_GetProgCnt(void)
{

    MS_U16 expc_l = 0;
    MS_U16 expc_h = 0;
    expc_l = _VPU_Read2Byte(VPU_REG_EXPC_L) & 0xFFFF;
    expc_h = _VPU_Read2Byte(VPU_REG_EXPC_H) & 0xFFFF;
    return (((MS_U32)expc_h) << 16) | (MS_U32)expc_l;
}

MS_U8 HAL_VPU_EX_GetTaskId(MS_U32 u32Id)
{
    return _VPU_EX_GetOffsetIdx(u32Id);
}

void HAL_VPU_EX_SetShareInfoAddr(MS_U32 u32Id, MS_VIRT u32ShmAddr)
{
    MS_U8 u8Offset = _VPU_EX_GetOffsetIdx(u32Id);

    if (u32ShmAddr == 0)
    {
        pVPUHalContext->u32FWShareInfoAddr[u8Offset] = 0xFFFFFFFFUL;
    }
    else
    {
        if (u8Offset == 0)
        {
            pVPUHalContext->u32FWShareInfoAddr[u8Offset] = u32ShmAddr;
        }
        else if (u8Offset == 1)
        {
            pVPUHalContext->u32FWShareInfoAddr[u8Offset] = u32ShmAddr + TEE_ONE_TASK_SHM_SIZE;
        }
    }

    VPU_MSG_DBG("set PA ShareInfoAddr[%d] = 0x%lx \n", u8Offset, (unsigned long)pVPUHalContext->u32FWShareInfoAddr[u8Offset]);
    return;
}

MS_VIRT HAL_VPU_EX_GetShareInfoAddr(MS_U32 u32Id)
{
    MS_U8 u8Offset = _VPU_EX_GetOffsetIdx(u32Id);

    return pVPUHalContext->u32FWShareInfoAddr[u8Offset];
}

#if defined(VDEC_FW31)
MS_VIRT HAL_VPU_EX_GetVsyncAddrOffset(MS_U32 u32Id)
{
    MS_U8  u8Offset = _VPU_EX_GetOffsetIdx(u32Id);
    MS_VIRT VPUSHMAddr = HAL_VPU_EX_GetSHMAddr();
    MS_VIRT VsyncBridgeOffset = 0;

    if (VPUSHMAddr != 0)  // TEE project
    {
        if ((u8Offset == 0) || (u8Offset == 1))
        {
            VsyncBridgeOffset = VSYNC_BRIDGE_OFFSET;
        }
        else
        {
            VsyncBridgeOffset = VSYNC_BRIDGE_NWAY_OFFSET + (u8Offset - 2) * VSYNC_BRIDGE_INFO_SIZE;
        }
    }
    else  // normal project
    {
        if ((u8Offset == 0) || (u8Offset == 1))
        {
            VsyncBridgeOffset = COMMON_AREA_START + VSYNC_BRIDGE_OFFSET;
        }
        else
        {
            VsyncBridgeOffset = COMMON_AREA_START + VSYNC_BRIDGE_NWAY_OFFSET + (u8Offset - 2) * VSYNC_BRIDGE_INFO_SIZE;
        }
    }

    return VsyncBridgeOffset;
}

MS_VIRT HAL_VPU_EX_GetVsyncExtAddrOffset(MS_U32 u32Id)
{
    MS_U8  u8Offset = _VPU_EX_GetOffsetIdx(u32Id);
    MS_VIRT VPUSHMAddr = HAL_VPU_EX_GetSHMAddr();
    MS_VIRT VsyncBridgeExtOffset = 0;

    if (VPUSHMAddr != 0)  // TEE project
    {
        if ((u8Offset == 0) || (u8Offset == 1))
        {
            VsyncBridgeExtOffset = VSYNC_BRIDGE_EXT_OFFSET;
        }
        else
        {
            VsyncBridgeExtOffset = VSYNC_BRIDGE_EXT_NWAY_OFFSET + (u8Offset - 2) * VSYNC_BRIDGE_INFO_SIZE;
        }
    }
    else  // normal project
    {
        if ((u8Offset == 0) || (u8Offset == 1))
        {
            VsyncBridgeExtOffset = COMMON_AREA_START + VSYNC_BRIDGE_EXT_OFFSET;
        }
        else
        {
            VsyncBridgeExtOffset = COMMON_AREA_START + VSYNC_BRIDGE_EXT_NWAY_OFFSET + (u8Offset - 2) * VSYNC_BRIDGE_INFO_SIZE;
        }
    }

    return VsyncBridgeExtOffset;
}
#endif

MS_BOOL HAL_VPU_EX_IsPowered(void)
{
    return pVPUHalContext->_bVPUPowered;
}

MS_BOOL HAL_VPU_EX_IsRsted(void)
{
    return pVPUHalContext->_bVPURsted;
}

MS_BOOL HAL_VPU_EX_IsEVDR2(void)
{
#ifdef EVDR2
    return TRUE;
#else
    return FALSE;
#endif
}

MS_BOOL HAL_VPU_EX_MVDInUsed(void)
{
    //MVD is in used for MVD or HVD_TSP mode.
    MS_U8 i;
    MS_U8 u8UseCnt = 0;

    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if ((pVPUHalContext->_stVPUStream[i].eDecodertype == E_VPU_EX_DECODER_MVD) ||
#ifdef VDEC3
                (pVPUHalContext->_stVPUStream[i].eDecodertype == E_VPU_EX_DECODER_EVD) ||
#endif
                (pVPUHalContext->_stVPUStream[i].eDecodertype == E_VPU_EX_DECODER_HVD) )
        {
            u8UseCnt++;
        }
    }

    VPU_MSG_DBG("MVD u8UseCnt=%d\n", u8UseCnt);

    if (u8UseCnt != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

MS_BOOL HAL_VPU_EX_HVDInUsed(void)
{
    //HVD is in used for HVD or MVD in sub stream.
    MS_U8 i;
    MS_U8 u8UseCnt = 0;
    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {

        if ((E_VPU_EX_DECODER_HVD == pVPUHalContext->_stVPUStream[i].eDecodertype))
        {
            u8UseCnt++;
        }
    }

    VPU_MSG_DBG("HVD u8UseCnt=%d\n", u8UseCnt);

    if (u8UseCnt != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void HAL_VPU_EX_Mutex_Lock(void)
{
    _HAL_VPU_Entry();
}

void HAL_VPU_EX_Mutex_UnLock(void)
{
    _HAL_VPU_Release();
}

#ifdef VDEC3
MS_BOOL HAL_VPU_EX_EVDInUsed(void)
{
    MS_U8 i;
    MS_U8 u8UseCnt = 0;

    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if (E_VPU_EX_DECODER_EVD == pVPUHalContext->_stVPUStream[i].eDecodertype)
        {
            u8UseCnt++;
        }
    }

    VPU_MSG_DBG("EVD u8UseCnt=%d\n", u8UseCnt);

    if (u8UseCnt != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#if SUPPORT_G2VP9 && defined(VDEC3)
MS_BOOL HAL_VPU_EX_G2VP9InUsed(void)
{
    MS_U8 i;
    MS_U8 u8UseCnt = 0;

    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if (E_VPU_EX_DECODER_G2VP9 == pVPUHalContext->_stVPUStream[i].eDecodertype)
        {
            u8UseCnt++;
        }
    }

    VPU_MSG_DBG("G2 VP9 u8UseCnt=%d\n", u8UseCnt);

    if (u8UseCnt != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif
#endif

//-----------------------------------------------------------------------------
/// @brief \b Function \b Name: MDrv_HVD_EX_SetDbgLevel()
/// @brief \b Function \b Description:  Set debug level
/// @param -elevel \b IN : debug level
//-----------------------------------------------------------------------------
void HAL_VPU_EX_SetDbgLevel(VPU_EX_UartLevel eLevel)
{
    VPU_MSG_INFO("%s eLevel=0x%x\n", __FUNCTION__, eLevel);

    switch (eLevel)
    {
        case E_VPU_EX_UART_LEVEL_ERR:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_ERR;
            break;
        }
        case E_VPU_EX_UART_LEVEL_INFO:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_INFO | E_VPU_UART_CTRL_ERR;
            break;
        }
        case E_VPU_EX_UART_LEVEL_DBG:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_DBG | E_VPU_UART_CTRL_ERR | E_VPU_UART_CTRL_INFO;
            break;
        }
        case E_VPU_EX_UART_LEVEL_TRACE:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_TRACE | E_VPU_UART_CTRL_ERR | E_VPU_UART_CTRL_INFO | E_VPU_UART_CTRL_DBG;
            break;
        }
        case E_VPU_EX_UART_LEVEL_FW:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_DISABLE;
            break;
        }
        default:
        {
            u32VpuUartCtrl = E_VPU_UART_CTRL_DISABLE;
            break;
        }
    }
}

MS_U32 HAL_VPU_EX_GetFWVer(MS_U32 u32Id, VPU_EX_FWVerType eVerType)
{
    HVD_Return eCtrlRet = E_HVD_RETURN_FAIL;
    MS_U32 u32CmdArg = (MS_U32)eVerType;
    MS_U32 u32Version = 0xFFFFFFFF;
    MS_BOOL bRet = false;
    MS_U32 u32TimeOut = 0xFFFFFFFF;

    eCtrlRet = HAL_HVD_EX_SetCmd(u32Id, E_DUAL_VERSION, u32CmdArg);
    if (E_HVD_RETURN_SUCCESS != eCtrlRet)
    {
        VPU_MSG_ERR("E_DUAL_VERSION NG eCtrlRet=%x\n", eCtrlRet);
        return u32Version;
    }

    while(--u32TimeOut)
    {
        if(HAL_VPU_EX_MBoxRdy(VPU_RISC_MBOX0))
        {
            bRet = HAL_VPU_EX_MBoxRead(VPU_RISC_MBOX0, &u32Version);
            if (false == bRet)
            {
                VPU_MSG_ERR("E_DUAL_VERSION NG bRet=%x\n", bRet);
                return u32Version;
            }

            _VPU_WriteWordMask(  VPU_REG_RISC_MBOX_CLR , VPU_REG_RISC_MBOX0_CLR  , VPU_REG_RISC_MBOX0_CLR);
            VPU_MSG_DBG("E_DUAL_VERSION arg=%x u32Version = 0x%x\n", u32CmdArg, u32Version);
            return u32Version;
        }
    }

    VPU_MSG_ERR("get E_DUAL_VERSION=%x timeout", eVerType);

    return u32Version;
}

MS_BOOL HAL_VPU_EX_NotSupportDS(void)
{
    return FALSE;// maserati disable SN DS
}

//-----------------------------------------------------------------------------
/// @brief \b Function \b Name: HAL_VPU_EX_MIU1BASE()
/// @brief \b Function \b Description:  Get VPU MIU base address
/// @return - vpu MIU1 base
//-----------------------------------------------------------------------------
MS_VIRT HAL_VPU_EX_MIU1BASE(void)
{
    return VPU_MIU1BASE_ADDR;
}


MS_VIRT HAL_VPU_EX_GetSHMAddr(void)
{
    if(pVPUHalContext->bEnableVPUSecureMode == FALSE)
    {
        return 0;
    }
    return pVPUHalContext->u32VPUSHMAddr;
}
MS_BOOL HAL_VPU_EX_EnableSecurityMode(MS_BOOL enable)
{
    pVPUHalContext->bEnableVPUSecureMode = enable;
    return TRUE;
}

MS_BOOL HAL_VPU_EX_CHIP_Capability(void* pHWCap)
{
    ((VDEC_HwCap*)pHWCap)->u8Cap_Support_Decoder_Num = 2;

    ((VDEC_HwCap*)pHWCap)->bCap_Support_MPEG2 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_H263 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_MPEG4 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_DIVX311 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_DIVX412 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_FLV = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_VC1ADV = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_VC1MAIN = TRUE;

    ((VDEC_HwCap*)pHWCap)->bCap_Support_RV8 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_RV9 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_H264 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_AVS = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_MJPEG = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_MVC = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_VP8 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_HEVC = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_VP9 = TRUE;
    ((VDEC_HwCap*)pHWCap)->bCap_Support_AVS_PLUS = TRUE;

    return TRUE;
}

//-----------------------------------------------------------------------------
/// @brief \b Function \b Name: HAL_VPU_EX_GetCodecCapInfo()
/// @brief \b Function \b Description:  Get chip codec capability  (for vudu)
/// @return - success/fail
//-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_GetCodecCapInfo( int eCodecType, VDEC_EX_CODEC_CAP_INFO *pCodecCapInfo)
{
#define MAX_CAPABILITY_INFO_NUM 8
#define MAX_CODEC_TYPE_NUM 18

    unsigned int capability[MAX_CODEC_TYPE_NUM][MAX_CAPABILITY_INFO_NUM] =
    {
        //width, height , frmrate,                                                 profile,                                        level,                                          version                          bit rate    reserved2
        {    0,    0,      0,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,         0,        0},//E_HVD_EX_CODEC_TYPE_NONE
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_MP2_MAIN,         E_VDEC_EX_CODEC_LEVEL_MP2_HIGH,     E_VDEC_EX_CODEC_VERSION_NONE,        80,        0},//E_HVD_EX_CODEC_TYPE_MPEG2
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_H263_BASELINE,    E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_H263_1,      40,        0},//E_HVD_EX_CODEC_TYPE_H263
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_MP4_ASP,          E_VDEC_EX_CODEC_LEVEL_MP4_L5,       E_VDEC_EX_CODEC_VERSION_NONE,        40,        0},//E_HVD_EX_CODEC_TYPE_MPEG4
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_DIVX_311,    40,        0},//E_HVD_EX_CODEC_TYPE_DIVX311
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_DIVX_6,      40,        0},//E_HVD_EX_CODEC_TYPE_DIVX412
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_FLV_1,       40,        0},//E_HVD_EX_CODEC_TYPE_FLV
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_VC1_AP,           E_VDEC_EX_CODEC_LEVEL_VC1_L3,       E_VDEC_EX_CODEC_VERSION_NONE,        40,        0},//E_HVD_EX_CODEC_TYPE_VC1_ADV
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_RCV_MAIN,         E_VDEC_EX_CODEC_LEVEL_RCV_HIGH,     E_VDEC_EX_CODEC_VERSION_NONE,        40,        0},//E_HVD_EX_CODEC_TYPE_VC1_MAIN (RCV)
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,        40,        0},//E_HVD_EX_CODEC_TYPE_RV8
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,        40,        0},//E_HVD_EX_CODEC_TYPE_RV9
        { 4096, 2160,     30,     E_VDEC_EX_CODEC_PROFILE_H264_HIP,         E_VDEC_EX_CODEC_LEVEL_H264_5_1,     E_VDEC_EX_CODEC_VERSION_NONE,       135,        0},//E_HVD_EX_CODEC_TYPE_H264
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_AVS_BROADCASTING, E_VDEC_EX_CODEC_LEVEL_AVS_6010860,  E_VDEC_EX_CODEC_VERSION_NONE,        50,        0},//E_HVD_EX_CODEC_TYPE_AVS
        { 1920, 1080,     30,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,        20,        0},//E_HVD_EX_CODEC_TYPE_MJPEG
        { 1920, 1080,     30,     E_VDEC_EX_CODEC_PROFILE_H264_HIP,         E_VDEC_EX_CODEC_LEVEL_H264_5_1,     E_VDEC_EX_CODEC_VERSION_NONE,        80,        0},//E_HVD_EX_CODEC_TYPE_MVC
        { 1920, 1080,     60,     E_VDEC_EX_CODEC_PROFILE_NONE,             E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,        20,        0},//E_HVD_EX_CODEC_TYPE_VP8
        { 4096, 2176,     60,     E_VDEC_EX_CODEC_PROFILE_H265_MAIN_10,     E_VDEC_EX_CODEC_LEVEL_H265_5_1_HT,  E_VDEC_EX_CODEC_VERSION_NONE,       100,        0},//E_HVD_EX_CODEC_TYPE_HEVC
        { 4096, 2176,     60,     E_VDEC_EX_CODEC_PROFILE_VP9_2,            E_VDEC_EX_CODEC_LEVEL_NONE,         E_VDEC_EX_CODEC_VERSION_NONE,       100,        0},//E_HVD_EX_CODEC_TYPE_VP9
    };

    if(eCodecType < MAX_CODEC_TYPE_NUM)
    {
        pCodecCapInfo->u16CodecCapWidth     = capability[eCodecType][0];
        pCodecCapInfo->u16CodecCapHeight    = capability[eCodecType][1];
        pCodecCapInfo->u8CodecCapFrameRate  = capability[eCodecType][2];
        pCodecCapInfo->u8CodecCapProfile    = capability[eCodecType][3];
        pCodecCapInfo->u8CodecCapLevel      = capability[eCodecType][4];
        pCodecCapInfo->u8CodecCapVersion    = capability[eCodecType][5];
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static MS_BOOL copyCapProfileTable(MS_U32 u32CodecCapProfileTableSizeDst, VDEC_EX_PROFILE_CAP_INFO *pCodecCapProfileTableDst  , VDEC_EX_PROFILE_CAP_INFO *pCodecCapProfileTableSrc, MS_U32 u32CodecCapProfileTableSizeSrc)
{
    if(u32CodecCapProfileTableSizeDst < u32CodecCapProfileTableSizeSrc)
        return FALSE;
    else
    {
        memcpy(pCodecCapProfileTableDst, pCodecCapProfileTableSrc, u32CodecCapProfileTableSizeSrc);
        return TRUE;
    }
}
//-----------------------------------------------------------------------------
/// @brief \b Function \b Name: HAL_VPU_EX_GetCodecProfileCapInfo()
/// @brief \b Function \b Description:  Get chip profile capability
/// @return - success/fail
//-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_GetCodecProfileCapInfo(int eCodecType, MS_U32 u32CodecCapProfileTableSize, VDEC_EX_CODEC_PROFILE_CAP_INFO *pCodecProfileCapInfo)
{
    MS_U32 profileEntryNum = 0;

    switch (eCodecType)
    {
        case E_VPU_EX_CODEC_TYPE_MPEG2:
        {
#define MAX_MPEG2_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_MPEG2_PROFILE_NUM + 1] =
            {
                //profile,                           level,                           width,  height,    frmrate,        version,                         bit rate,
                { E_VDEC_EX_CODEC_PROFILE_MP2_MAIN,  E_VDEC_EX_CODEC_LEVEL_MP2_HIGH,  1920,   1080,      60,             E_VDEC_EX_CODEC_VERSION_NONE,    80},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,   E_VDEC_EX_CODEC_LEVEL_NONE,      0,      0,         0,              E_VDEC_EX_CODEC_VERSION_NONE,    0},
            };
            profileEntryNum = MAX_MPEG2_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_H263:
        {
#define MAX_H263_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_H263_PROFILE_NUM + 1] =
            {
                //profile,                                level,                       width,  height,  frmrate,     version,                         bit rate,
                { E_VDEC_EX_CODEC_PROFILE_H263_BASELINE,  E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,          E_VDEC_EX_CODEC_VERSION_H263_1,  40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,        E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,           E_VDEC_EX_CODEC_VERSION_NONE,    0},
            };
            profileEntryNum = MAX_H263_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_MPEG4:
        {
#define MAX_MPEG4_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_MPEG4_PROFILE_NUM + 1] =
            {
                //profile,                          level,                         width,  height,  frmrate,      version,                         bit rate,
                { E_VDEC_EX_CODEC_PROFILE_MP4_ASP,  E_VDEC_EX_CODEC_LEVEL_MP4_L5,  1920,   1080,    60,           E_VDEC_EX_CODEC_VERSION_NONE,    40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,    0,      0,       0,            E_VDEC_EX_CODEC_VERSION_NONE,    0},
            };
            profileEntryNum = MAX_MPEG4_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_DIVX311:
        {
#define MAX_DIVX311_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_DIVX311_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_DIVX_311,    40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_DIVX311_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_DIVX412:
        {
#define MAX_DIVX412_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_DIVX412_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_DIVX_6,      40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_DIVX412_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_FLV:
        {
#define MAX_FLV_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_FLV_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_FLV_1,       40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_FLV_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_VC1_ADV:
        {
#define MAX_VC1_ADV_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_VC1_ADV_PROFILE_NUM + 1] =
            {
                //profile,                          level,                         width,  height,  frmrate,      version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_VC1_AP,   E_VDEC_EX_CODEC_LEVEL_VC1_L3,  1920,   1080,    60,           E_VDEC_EX_CODEC_VERSION_NONE,        40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,    0,      0,       0,            E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_VC1_ADV_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_VC1_MAIN:
        {
#define MAX_VC1_MAIN_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_VC1_MAIN_PROFILE_NUM + 1] =
            {
                //profile,                          level,                           width,  height,  frmrate,    version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_RCV_MAIN, E_VDEC_EX_CODEC_LEVEL_RCV_HIGH,  1920,   1080,    60,         E_VDEC_EX_CODEC_VERSION_NONE,        40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,      0,      0,       0,          E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_VC1_MAIN_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_RV8:
        {
#define MAX_RV8_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_RV8_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_RV8_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_RV9:
        {
#define MAX_RV9_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_RV9_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        40},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_RV9_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_H264:
        {
#define MAX_H264_PROFILE_NUM 5
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_H264_PROFILE_NUM + 1] =
            {
                //profile,                          level,                           width,  height,  frmrate,    version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_H264_HIP, E_VDEC_EX_CODEC_LEVEL_H264_5_1,  4096,   2160,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        135},
                { E_VDEC_EX_CODEC_PROFILE_H264_MP,  E_VDEC_EX_CODEC_LEVEL_H264_5_1,  4096,   2160,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        135},
                { E_VDEC_EX_CODEC_PROFILE_H264_XP,  E_VDEC_EX_CODEC_LEVEL_H264_5_1,  4096,   2160,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        135},
                { E_VDEC_EX_CODEC_PROFILE_H264_BP,  E_VDEC_EX_CODEC_LEVEL_H264_5_1,  4096,   2160,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        135},
                { E_VDEC_EX_CODEC_PROFILE_H264_CBP, E_VDEC_EX_CODEC_LEVEL_H264_5_1,  4096,   2160,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        135},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,      0,      0,       0,          E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_H264_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_AVS:
        {
#define MAX_AVS_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_AVS_PROFILE_NUM + 1] =
            {
                //profile,                                   level,                                 width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_AVS_BROADCASTING,  E_VDEC_EX_CODEC_LEVEL_AVS_6010860,     1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        50},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,           E_VDEC_EX_CODEC_LEVEL_NONE,            0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_AVS_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_MJPEG:
        {
#define MAX_MJPEG_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_MJPEG_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    30,             E_VDEC_EX_CODEC_VERSION_NONE,        20},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_MJPEG_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_MVC:
        {
#define MAX_MVC_PROFILE_NUM 5
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_MVC_PROFILE_NUM + 1] =
            {
                //profile,                           level,                            width,  height,  frmrate,    version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_H264_HIP,  E_VDEC_EX_CODEC_LEVEL_H264_5_1,   1920,   1080,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        80},
                { E_VDEC_EX_CODEC_PROFILE_H264_MP,   E_VDEC_EX_CODEC_LEVEL_H264_5_1,   1920,   1080,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        80},
                { E_VDEC_EX_CODEC_PROFILE_H264_XP,   E_VDEC_EX_CODEC_LEVEL_H264_5_1,   1920,   1080,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        80},
                { E_VDEC_EX_CODEC_PROFILE_H264_BP,   E_VDEC_EX_CODEC_LEVEL_H264_5_1,   1920,   1080,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        80},
                { E_VDEC_EX_CODEC_PROFILE_H264_CBP,  E_VDEC_EX_CODEC_LEVEL_H264_5_1,   1920,   1080,    30,         E_VDEC_EX_CODEC_VERSION_NONE,        80},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,   E_VDEC_EX_CODEC_LEVEL_NONE,       0,      0,       0,          E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_MVC_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_VP8:
        {
#define MAX_VP8_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_VP8_PROFILE_NUM + 1] =
            {
                //profile,                          level,                       width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,     E_VDEC_EX_CODEC_LEVEL_NONE,  1920,   1080,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        20},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,  E_VDEC_EX_CODEC_LEVEL_NONE,  0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_VP8_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_HEVC:
        {
#define MAX_HEVC_PROFILE_NUM 2
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_HEVC_PROFILE_NUM + 1] =
            {
                //profile,                               level,                              width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_H265_MAIN_10,  E_VDEC_EX_CODEC_LEVEL_H265_5_1_HT,  4096,   2176,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        100},
                { E_VDEC_EX_CODEC_PROFILE_H265_MAIN,     E_VDEC_EX_CODEC_LEVEL_H265_5_1_HT,  4096,   2176,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        100},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,       E_VDEC_EX_CODEC_LEVEL_NONE,         0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_HEVC_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_VP9:
        {
#define MAX_VP9_PROFILE_NUM 2
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_VP9_PROFILE_NUM + 1] =
            {
                //profile,                            level,                        width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_VP9_2,      E_VDEC_EX_CODEC_LEVEL_NONE,   4096,   2176,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        100},
                { E_VDEC_EX_CODEC_PROFILE_VP9_0,      E_VDEC_EX_CODEC_LEVEL_NONE,   4096,   2176,    60,             E_VDEC_EX_CODEC_VERSION_NONE,        100},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,    E_VDEC_EX_CODEC_LEVEL_NONE,   0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_VP9_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
        case E_VPU_EX_CODEC_TYPE_HEVC_DV:
        {
#define MAX_HEVC_DV_PROFILE_NUM 1
            VDEC_EX_PROFILE_CAP_INFO codecCapProfileTable[MAX_HEVC_DV_PROFILE_NUM + 1] =
            {
                //profile,                            level,                         width,  height,  frmrate,        version,                             bit rate,
                { E_VDEC_EX_CODEC_PROFILE_NONE,       E_VDEC_EX_CODEC_LEVEL_NONE,    0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
                { E_VDEC_EX_CODEC_PROFILE_INVALID,    E_VDEC_EX_CODEC_LEVEL_NONE,    0,      0,       0,              E_VDEC_EX_CODEC_VERSION_NONE,        0},
            };
            profileEntryNum = MAX_HEVC_DV_PROFILE_NUM + 1;
            return copyCapProfileTable(u32CodecCapProfileTableSize, pCodecProfileCapInfo->pCodecCapProfileTable, codecCapProfileTable, sizeof(VDEC_EX_PROFILE_CAP_INFO) * profileEntryNum);
        }
    }//end of switch

    return FALSE;
}

#ifdef VDEC3
void HAL_VPU_EX_SetBBUSetting(MS_U32 u32Id, MS_U32 u32BBUId, VPU_EX_DecoderType eDecType, MS_U8 u8TypeBit)
{
    BBU_STATE *bbu_state;
    (void) u32Id;

    if (   (eDecType == E_VPU_EX_DECODER_MVD)
            || (eDecType == E_VPU_EX_DECODER_VP8)
#if SUPPORT_G2VP9
            || (eDecType == E_VPU_EX_DECODER_G2VP9)
#endif
       )
    {
        // MVD should not call this function.
        // VP8 and G2_VP9 don't have the concept of BBU, so we just return.
        return;
    }

    switch (eDecType)
    {
        case E_VPU_EX_DECODER_EVD:
            bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
            break;
        case E_VPU_EX_DECODER_HVD:
        case E_VPU_EX_DECODER_RVD:
        case E_VPU_EX_DECODER_MVC:
        default:
            bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
            break;
    }

    bbu_state[u32BBUId].u8RegSetting |= u8TypeBit;
    return;
}

MS_BOOL HAL_VPU_EX_CheckBBUSetting(MS_U32 u32Id, MS_U32 u32BBUId, VPU_EX_DecoderType eDecType, MS_U8 u8TypeBit)
{
    BBU_STATE *bbu_state;
    (void) u32Id;

    if (   (eDecType == E_VPU_EX_DECODER_MVD)
            || (eDecType == E_VPU_EX_DECODER_VP8)
#if SUPPORT_G2VP9
            || (eDecType == E_VPU_EX_DECODER_G2VP9)
#endif
       )
    {
        // MVD should not call this function.
        // VP8 and G2_VP9 don't have the concept of BBU, so we just return TRUE and not set related registers.
        return TRUE;
    }

    switch (eDecType)
    {
        case E_VPU_EX_DECODER_EVD:
            bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
            break;
        case E_VPU_EX_DECODER_HVD:
        case E_VPU_EX_DECODER_RVD:
        case E_VPU_EX_DECODER_MVC:
        default:
            bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
            break;
    }

    return (bbu_state[u32BBUId].u8RegSetting & u8TypeBit);
}

void HAL_VPU_EX_ClearBBUSetting(MS_U32 u32Id, MS_U32 u32BBUId, VPU_EX_DecoderType eDecType)
{
    BBU_STATE *bbu_state;
    (void) u32Id;

    if (   (eDecType == E_VPU_EX_DECODER_MVD)
            || (eDecType == E_VPU_EX_DECODER_VP8)
#if SUPPORT_G2VP9
            || (eDecType == E_VPU_EX_DECODER_G2VP9)
#endif
       )
    {
        // MVD should not call this function.
        // VP8 and G2_VP9 don't have the concept of BBU, so we just return.
        return;
    }

    switch (eDecType)
    {
        case E_VPU_EX_DECODER_EVD:
            bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
            break;
        case E_VPU_EX_DECODER_HVD:
        case E_VPU_EX_DECODER_RVD:
        case E_VPU_EX_DECODER_MVC:
        default:
            bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
            break;
    }

    if (bbu_state[u32BBUId].u32Used == 0)
    {
        bbu_state[u32BBUId].u8RegSetting = 0;
    }

    return;
}

MS_U32 HAL_VPU_EX_GetBBUId(MS_U32 u32Id, VPU_EX_TaskInfo *pTaskInfo, MS_BOOL bShareBBU)
{
    MS_U32 i, max_bbu_cnt;
    BBU_STATE *bbu_state;
    SLQ_STATE *slq_state;
    MS_U8 u8TaskId;
    MS_BOOL bTSP;
    MS_U32 retBBUId = HAL_VPU_INVALID_BBU_ID;

    if(pTaskInfo == NULL)
        return retBBUId;

    slq_state = &pVPUHalContext->stMVD_SLQ_STATE[0];
    u8TaskId = HAL_VPU_EX_GetTaskId(u32Id);

    bTSP = (pTaskInfo->eSrcType == E_VPU_EX_INPUT_TSP);

    pVPUHalContext->u8HALId[u8TaskId] = pTaskInfo->u8HalId;

    /*    HVD_EX_MSG_ERR("[%d] DecType=0x%x \n", u32Id & 0xFF, pTaskInfo->eDecType);
        for(i = 0; i < MAX_MVD_SLQ_COUNT; i++)
            HVD_EX_MSG_ERR("slq_state[%d] u32Used=0x%x bTSP=%x bUsedbyMVD=%x\n",i,slq_state[i].u32Used,slq_state[i].bTSP,slq_state[i].bUsedbyMVD);

        for(i = 0; i < MAX_EVD_BBU_COUNT; i++)
            HVD_EX_MSG_ERR("EVD_BBU_state[%d] u32Used=0x%x bTSP=%x\n",i,pVPUHalContext->stEVD_BBU_STATE[i].u32Used,pVPUHalContext->stEVD_BBU_STATE[i].bTSP);

        for(i = 0; i < MAX_HVD_BBU_COUNT; i++)
            HVD_EX_MSG_ERR("HVD_BBU_state[%d] u32Used=0x%x bTSP=%x\n",i,pVPUHalContext->stHVD_BBU_STATE[i].u32Used,pVPUHalContext->stHVD_BBU_STATE[i].bTSP);
    */
#if 1
    if(pTaskInfo->eDecType == E_VPU_EX_DECODER_MVD) // MVD case
    {
        max_bbu_cnt = MAX_MVD_SLQ_COUNT;
        if (bTSP)
        {
            if ((u8TaskId < MAX_MVD_SLQ_COUNT) && (slq_state[u8TaskId].u32Used == 0))
            {
                VPRINTF("[NDec][0x%x][%d] MVD bTSP, use bbu %d \n", u32Id, u8TaskId, u8TaskId);
                slq_state[u8TaskId].u32Used |= (1 << u8TaskId); // Record the HVD use the TSP parser
                slq_state[u8TaskId].bTSP = TRUE;
                slq_state[u8TaskId].bUsedbyMVD = TRUE;
                return u8TaskId;
            }
        }
        else
        {
            MS_U32 shared_bbu_idx = HAL_VPU_INVALID_BBU_ID;
            MS_U32 avaliable_bbu_idx = HAL_VPU_INVALID_BBU_ID;
            for (i = 0; i < MAX_MVD_SLQ_COUNT; i++)
            {
                if (slq_state[i].u32Used != 0)
                {
                    if (shared_bbu_idx == HAL_VPU_INVALID_BBU_ID && slq_state[i].bTSP == FALSE)
                    {
                        shared_bbu_idx = i; // recored the first used MM bbu for sharing
                    }
                }
                else if (avaliable_bbu_idx == HAL_VPU_INVALID_BBU_ID)
                {
                    avaliable_bbu_idx = i; // recored the first empty bbu
                }
            }

            VPU_MSG_INFO("[NDec][0x%x][%d] MVD bShareBBU %d, shared_bbu %d, avail_bbu %d \n", u32Id, u8TaskId, bShareBBU, shared_bbu_idx, avaliable_bbu_idx);

            if ((bShareBBU == TRUE) && (shared_bbu_idx != HAL_VPU_INVALID_BBU_ID))   // In Nstream mode, first priority is sharing bbu
            {
                slq_state[shared_bbu_idx].u32Used |= (1 << u8TaskId);
                slq_state[shared_bbu_idx].bTSP = FALSE;
                slq_state[shared_bbu_idx].bUsedbyMVD = TRUE;
                VPU_MSG_INFO("[NDec][0x%x][%d] MVD shared_bbu %d \n", u32Id, u8TaskId, shared_bbu_idx);
                return shared_bbu_idx;
            }
            else if (slq_state[u8TaskId].u32Used == FALSE && u8TaskId < max_bbu_cnt)   // 2nd priority is task id
            {
                slq_state[u8TaskId].u32Used |= (1 << u8TaskId);
                slq_state[u8TaskId].bTSP = FALSE;
                slq_state[u8TaskId].bUsedbyMVD = TRUE;

                if (bShareBBU == FALSE)
                {
                    slq_state[u8TaskId].bTSP = TRUE;
                    VPRINTF("[NDec] MVD occupy one BBU_ID \n");
                }
                VPU_MSG_INFO("[NDec][0x%x][%d] MVD u8TaskId %d \n", u32Id, u8TaskId, u8TaskId);
                return u8TaskId;
            }
            else if (avaliable_bbu_idx != HAL_VPU_INVALID_BBU_ID && avaliable_bbu_idx < max_bbu_cnt)   // 3rd priority is avaliable bbu id
            {
                slq_state[avaliable_bbu_idx].u32Used |= (1 << u8TaskId);
                slq_state[avaliable_bbu_idx].bTSP = FALSE;
                slq_state[avaliable_bbu_idx].bUsedbyMVD = TRUE;

                if (bShareBBU == FALSE)
                {
                    slq_state[avaliable_bbu_idx].bTSP = TRUE;
                    VPRINTF("[NDec] MVD occupy one BBU_ID \n");
                }
                VPU_MSG_INFO("[NDec][0x%x][%d] MVD avaliable_bbu_idx %d \n", u32Id, u8TaskId, avaliable_bbu_idx);
                return avaliable_bbu_idx;
            }
            else
            {
                VPU_MSG_ERR("ERROR!!! MVD can't get avaliable BBU ID taskId=%d at %s\n", u8TaskId, __FUNCTION__);
            }
        }
    }
#if SUPPORT_G2VP9
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_G2VP9) // G2_VP9 case
    {
        // G2_VP9 don't have the concept of BBU, so we don't need to record the hardware BBU usage situation
        // Don't care the return value, G2_VP9 will not use it.
        return 0;
    }
#endif
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_VP8) // VP8 case
    {
        // G2_VP8 always use the same BBU, so we don't need to record the hardware BBU usage situation
        // Don't care the return value, VP8 will not use it.
        MS_U8 u8Offset = _VPU_EX_GetOffsetIdx(u32Id);
        return u8Offset;
        //return 0;
    }
    else
    {
        switch (pTaskInfo->eDecType)
        {
            case E_VPU_EX_DECODER_EVD:
                max_bbu_cnt = MAX_EVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
                break;
            case E_VPU_EX_DECODER_HVD:
            case E_VPU_EX_DECODER_RVD:
            case E_VPU_EX_DECODER_MVC:
            default:
                max_bbu_cnt = MAX_HVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
                break;
        }

        // FIXME: TSP assume bbu id = u8TaskId, so it does not support N decode. Use the same logic with MM to support it
        if (bTSP)
        {
            if ((u8TaskId < max_bbu_cnt) && (bbu_state[u8TaskId].u32Used == 0) && (slq_state[u8TaskId].u32Used == 0))
            {
                VPRINTF("[NDec][0x%x][%d] bTSP, use bbu %d \n", u32Id, u8TaskId, u8TaskId);
                bbu_state[u8TaskId].u32Used |= (1 << u8TaskId);
                bbu_state[u8TaskId].bTSP = TRUE;
                slq_state[u8TaskId].u32Used |= (1 << u8TaskId); // Record the HVD use the TSP parser
                slq_state[u8TaskId].bTSP = TRUE;
                slq_state[u8TaskId].bUsedbyMVD = FALSE;
                return u8TaskId;
            }
        }
        else
        {
            MS_U32 shared_bbu_idx = HAL_VPU_INVALID_BBU_ID;
            MS_U32 avaliable_bbu_idx = HAL_VPU_INVALID_BBU_ID;
            for (i = 0; i < max_bbu_cnt; i++)
            {
                if (shared_bbu_idx == HAL_VPU_INVALID_BBU_ID && bbu_state[i].u32Used != 0)
                {
                    if (bbu_state[i].bTSP == FALSE)
                    {
                        shared_bbu_idx = i;
                    }
                }
                else if (avaliable_bbu_idx == HAL_VPU_INVALID_BBU_ID)
                {
                    avaliable_bbu_idx = i;
                }
            }

            VPU_MSG_INFO("[NDec][0x%x][%d] bShareBBU %d, shared_bbu %d, avail_bbu %d \n", u32Id, u8TaskId, bShareBBU, shared_bbu_idx, avaliable_bbu_idx);

            if ((bShareBBU == TRUE) && (shared_bbu_idx != HAL_VPU_INVALID_BBU_ID))   // // In Nstream mode, first priority is sharing bbu
            {
                bbu_state[shared_bbu_idx].u32Used |= (1 << u8TaskId);
                VPU_MSG_INFO("[NDec][0x%x][%d] shared_bbu %d \n", u32Id, u8TaskId, shared_bbu_idx);
                return shared_bbu_idx;
            }
            else if (bbu_state[u8TaskId].u32Used == FALSE && u8TaskId < max_bbu_cnt)   // 2nd priority is task id
            {
                bbu_state[u8TaskId].u32Used |= (1 << u8TaskId);

                if (bShareBBU == FALSE)
                {
                    bbu_state[u8TaskId].bTSP = TRUE;
                    VPRINTF("[NDec] occupy one BBU_ID \n");
                }
                VPU_MSG_INFO("[NDec][0x%x][%d] u8TaskId %d \n", u32Id, u8TaskId, u8TaskId);
                return u8TaskId;
            }
            else if (avaliable_bbu_idx != HAL_VPU_INVALID_BBU_ID && avaliable_bbu_idx < max_bbu_cnt)   // 3rd priority is avaliable bbu id
            {
                bbu_state[avaliable_bbu_idx].u32Used |= (1 << u8TaskId);

                if (bShareBBU == FALSE)
                {
                    bbu_state[avaliable_bbu_idx].bTSP = TRUE;
                    VPRINTF("[NDec] occupy one BBU_ID \n");
                }
                VPU_MSG_INFO("[NDec][0x%x][%d] avaliable_bbu_idx %d \n", u32Id, u8TaskId, avaliable_bbu_idx);
                return avaliable_bbu_idx;
            }
            else
            {
                VPU_MSG_ERR("ERROR!!! can't get avaliable BBU ID taskId=%d at %s\n", u8TaskId, __FUNCTION__);
            }
        }
    }
#else // The following source code is wiser selecting BBU id. Howerver, it need HW to support and we mark it temporarily.
    MS_U32 j;
    MS_BOOL Got = FALSE;
    if(pTaskInfo->eDecType == E_VPU_EX_DECODER_MVD) // MVD case
    {
        for (i = 0; i < MAX_MVD_SLQ_COUNT; i++)
        {
            if(slq_state[i].u32Used != 0)
            {
                if(!bTSP && slq_state[i].bTSP == FALSE) // MVD non-first MM case
                {
                    retBBUId = i;
                    slq_state[retBBUId].u32Used |= (1 << u8TaskId);
                    slq_state[retBBUId].bTSP = bTSP;
                    slq_state[retBBUId].bUsedbyMVD = TRUE;
                    return retBBUId;
                }
            }
            else if(!Got && slq_state[i].u32Used == 0) // MVD first MM or TS case
            {
                if(i < MAX_EVD_BBU_COUNT) // Trend to select used EVD BBU id
                {
                    if(pVPUHalContext->stEVD_BBU_STATE[i].u32Used != 0 && pVPUHalContext->stEVD_BBU_STATE[i].bTSP == FALSE)
                    {
                        Got = TRUE;
                        retBBUId = i;
                    }
                }

                if(!Got && i < MAX_HVD_BBU_COUNT) // Trend to select used HVD BBU id
                {
                    if(pVPUHalContext->stHVD_BBU_STATE[i].u32Used != 0 && pVPUHalContext->stHVD_BBU_STATE[i].bTSP == FALSE)
                    {
                        Got = TRUE;
                        retBBUId = i;
                    }
                }

                if(!Got && retBBUId == HAL_VPU_INVALID_BBU_ID) // if no used EVD BBU id, select the first BBU_ID
                    retBBUId = i;
            }
        }
        if(retBBUId != HAL_VPU_INVALID_BBU_ID)
        {
            slq_state[retBBUId].u32Used |= (1 << u8TaskId);
            slq_state[retBBUId].bTSP = bTSP;
            slq_state[retBBUId].bUsedbyMVD = TRUE;
        }
    }
#if SUPPORT_G2VP9
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_G2VP9) // G2_VP9 case
    {
        // G2_VP9 don't have the concept of BBU, so we don't need to record the hardware BBU usage situation
        // Don't care the return value, G2_VP9 will not use it.
        return 0;
    }
#endif
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_VP8) // VP8 case
    {
        // G2_VP8 always use the same BBU, so we don't need to record the hardware BBU usage situation
        // Don't care the return value, VP8 will not use it.
        return 0;
    }
    else // HVD/EVD case
    {
        switch (pTaskInfo->eDecType)
        {
            case E_VPU_EX_DECODER_EVD:
            case E_VPU_EX_DECODER_G2VP9:
                max_bbu_cnt = MAX_EVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
                break;
            case E_VPU_EX_DECODER_HVD:
            case E_VPU_EX_DECODER_RVD:
            case E_VPU_EX_DECODER_MVC:
            default:
                max_bbu_cnt = MAX_HVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
                break;
        }

        for (i = 0; i < max_bbu_cnt; i++)
        {
            if(bbu_state[i].u32Used != 0)
            {
                if(!bTSP && bbu_state[i].bTSP == FALSE) // HVD/EVD non-first MM case
                {
                    retBBUId = i;
                    bbu_state[retBBUId].u32Used |= (1 << u8TaskId);
                    bbu_state[retBBUId].bTSP = bTSP;
                    return retBBUId;
                }
            }
            else if(bbu_state[i].u32Used == 0) // HVD/EVD first MM or TS case
            {
                if(i < MAX_MVD_SLQ_COUNT)
                {
                    if(!bTSP) //HVD/EVD first MM case
                    {
                        if( slq_state[i].u32Used != 0 && slq_state[i].bUsedbyMVD == TRUE) // HVD/EVD MM will trend to select used MVD SLQ id
                        {
                            retBBUId = i;
                            bbu_state[retBBUId].u32Used |= (1 << u8TaskId);
                            bbu_state[retBBUId].bTSP = bTSP;
                            return retBBUId;
                        }

                        if(retBBUId == HAL_VPU_INVALID_BBU_ID) // if no used MVD SLQ id, select the first BBU_ID
                            retBBUId = i;
                    }
                    else if(slq_state[i].u32Used == 0) //HVD/EVD TSP case, just find a empty slq id
                    {
                        retBBUId = i;
                        bbu_state[retBBUId].u32Used |= (1 << u8TaskId);
                        bbu_state[retBBUId].bTSP = bTSP;
                        slq_state[retBBUId].bUsedbyMVD = FALSE;
                        slq_state[retBBUId].u32Used |= (1 << u8TaskId);
                        slq_state[retBBUId].bTSP = bTSP;
                        return retBBUId;
                    }
                }
            }
        }
        if(retBBUId != HAL_VPU_INVALID_BBU_ID)
        {
            bbu_state[retBBUId].u32Used |= (1 << u8TaskId);
            bbu_state[retBBUId].bTSP = bTSP;
            if(bTSP)
            {
                slq_state[retBBUId].bUsedbyMVD = FALSE;
                slq_state[retBBUId].u32Used |= (1 << u8TaskId);
                slq_state[retBBUId].bTSP = bTSP;
            }
        }
    }
#endif
    return retBBUId;
}

MS_BOOL HAL_VPU_EX_FreeBBUId(MS_U32 u32Id, MS_U32 u32BBUId, VPU_EX_TaskInfo *pTaskInfo)
{
    MS_U32 max_bbu_cnt;
    BBU_STATE *bbu_state;
    MS_U8 u8TaskId;
    MS_BOOL bTSP;
    SLQ_STATE *slq_state = &pVPUHalContext->stMVD_SLQ_STATE[0];

    if(pTaskInfo == NULL)
        return FALSE;
    u8TaskId = HAL_VPU_EX_GetTaskId(u32Id);
    bTSP = (pTaskInfo->eSrcType == E_VPU_EX_INPUT_TSP);

    HVD_EX_MSG_DBG("[%d] DecType=0x%x \n", (int)(u32Id & 0xFF), pTaskInfo->eDecType);
    /*  MS_U32 i;
        for(i = 0; i < MAX_MVD_SLQ_COUNT; i++)
            HVD_EX_MSG_ERR("slq_state[%d] u32Used=0x%x bTSP=%x bUsedbyMVD=%x\n",i,slq_state[i].u32Used,slq_state[i].bTSP,slq_state[i].bUsedbyMVD);

        for(i = 0; i < MAX_EVD_BBU_COUNT; i++)
            HVD_EX_MSG_ERR("EVD_BBU_state[%d] u32Used=0x%x bTSP=%x\n",i,pVPUHalContext->stEVD_BBU_STATE[i].u32Used,pVPUHalContext->stEVD_BBU_STATE[i].bTSP);

        for(i = 0; i < MAX_HVD_BBU_COUNT; i++)
            HVD_EX_MSG_ERR("HVD_BBU_state[%d] u32Used=0x%x bTSP=%x\n",i,pVPUHalContext->stHVD_BBU_STATE[i].u32Used,pVPUHalContext->stHVD_BBU_STATE[i].bTSP);
    */
    if(pTaskInfo->eDecType == E_VPU_EX_DECODER_MVD) // MVD case
    {
        // TO DO
        if(u32BBUId < MAX_MVD_SLQ_COUNT)
        {
            slq_state[u32BBUId].u32Used &= ~(1 << u8TaskId); // Record the HVD use the TSP parser
            slq_state[u32BBUId].bTSP = FALSE;
            slq_state[u32BBUId].bUsedbyMVD = FALSE;
            return TRUE;
        }
    }
#if SUPPORT_G2VP9
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_G2VP9) // G2_VP9 case
    {
        // G2_VP9 don't have the concept of BBU, so we don't need to record the hardware BBU usage situation
        return TRUE;
    }
#endif
    else if(pTaskInfo->eDecType == E_VPU_EX_DECODER_VP8) // VP8 case
    {
        // G2_VP8 always use the same BBU, so we don't need to record the hardware BBU usage situation
        return TRUE;
    }
    else
    {
        switch (pTaskInfo->eDecType)
        {
            case E_VPU_EX_DECODER_EVD:
                max_bbu_cnt = MAX_EVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stEVD_BBU_STATE[0];
                break;
            case E_VPU_EX_DECODER_HVD:
            case E_VPU_EX_DECODER_RVD:
            case E_VPU_EX_DECODER_MVC:
            default:
                max_bbu_cnt = MAX_HVD_BBU_COUNT;
                bbu_state = &pVPUHalContext->stHVD_BBU_STATE[0];
                break;
        }

        if (u32BBUId < max_bbu_cnt)
        {
            bbu_state[u32BBUId].u32Used &= ~(1 << u8TaskId);
            bbu_state[u32BBUId].bTSP = FALSE;

            if (bTSP)
            {
                slq_state[u32BBUId].u32Used &= ~(1 << u8TaskId); // Record the HVD use the TSP parser
                slq_state[u32BBUId].bTSP = FALSE;
                slq_state[u32BBUId].bUsedbyMVD = FALSE;
            }

#if 1
            HAL_VPU_EX_ClearBBUSetting(u32Id, u32BBUId, pTaskInfo->eDecType);
#else
            if (bbu_state[u32BBUId].u32Used == 0)
            {
                bbu_state[u32BBUId].u8RegSetting = 0;  // clear record of both NAL_TBL and ES_BUFFER
            }
#endif

            return TRUE;
        }
    }
    return FALSE;
}

MS_U32 HAL_VPU_EX_GetVBBUVacancy(MS_VIRT u32VBBUAddr)
{
    VDEC_VBBU *pstVBBU = (VDEC_VBBU *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + u32VBBUAddr);
    MS_U32 u32WrPtr;
    MS_U32 u32RdPtr;
    MS_U32 u32Vacancy = 0;

    if (CHECK_NULL_PTR(pstVBBU))
        return 0;
    u32WrPtr = pstVBBU->u32WrPtr;
    u32RdPtr = pstVBBU->u32RdPtr;

    if (u32WrPtr == u32RdPtr)
    {
        u32Vacancy = MAX_VDEC_VBBU_ENTRY_COUNT;
    }
    else if (u32WrPtr > u32RdPtr)
    {
        u32Vacancy = MAX_VDEC_VBBU_ENTRY_COUNT - (u32WrPtr - u32RdPtr);
    }
    else
    {
        u32Vacancy = u32RdPtr - u32WrPtr - 1;
    }

    return u32Vacancy;
}

MS_U32 HAL_VPU_EX_GetInputQueueNum(MS_U32 u32Id)
{
    return MAX_VDEC_VBBU_ENTRY_COUNT;
}

MS_PHY HAL_VPU_EX_GetFWCodeAddr(MS_U32 u32Id)
{
    return pVPUHalContext->u32FWCodeAddr;
}

MS_VIRT HAL_VPU_EX_GetESReadPtr(MS_U32 u32Id, MS_VIRT u32VBBUAddr)
{
    VDEC_VBBU_Entry *stEntry;
    VDEC_VBBU *pstVBBU = (VDEC_VBBU *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + u32VBBUAddr);
#if SUPPORT_G2VP9
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
#endif

    if (CHECK_NULL_PTR(pstVBBU))
        return FALSE;
    MsOS_ReadMemory();

#ifdef STELLAR
    if (pstVBBU->u32RdPtr > MAX_VDEC_VBBU_ENTRY_COUNT)
    {
#ifdef MSOS_TYPE_LINUX_KERNEL
        printk("%s  %d  fatal error\n", __FUNCTION__, __LINE__);
#endif
        return 0;
    }
#endif

    stEntry = (VDEC_VBBU_Entry *) &pstVBBU->stEntry[pstVBBU->u32RdPtr];

    if(NULL == stEntry)
    {
        return 0;
    }

//    ALOGE("JJJ1: %d %d %d", pstVBBU->u32RdPtr, pstVBBU->u32WrPtr, stEntry->u32Offset);
    if (pstVBBU->u32RdPtr == pstVBBU->u32WrPtr)
    {
        return HAL_VPU_EX_GetESWritePtr(u32Id, u32VBBUAddr);
    }
    else
    {
#if SUPPORT_G2VP9
        if (E_VPU_EX_DECODER_G2VP9 == pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype)
        {
            if (stEntry->u32Offset == 0)
                return 0;
            else
                return stEntry->u32Offset - pVPUHalContext->u32BitstreamAddress[u8OffsetIdx];
        }
        else
#endif
        {
            return stEntry->u32Offset;
        }
    }
}

MS_VIRT HAL_VPU_EX_GetESWritePtr(MS_U32 u32Id, MS_VIRT u32VBBUAddr)
{
    MS_U32 u32WrPtr;
    VDEC_VBBU *pstVBBU = (VDEC_VBBU *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + u32VBBUAddr);
    VDEC_VBBU_Entry *stEntry;
#if SUPPORT_G2VP9
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
#endif

    MsOS_ReadMemory();
    if (CHECK_NULL_PTR(pstVBBU))
        return 0;
    u32WrPtr = pstVBBU->u32WrPtr;

#ifdef STELLAR
    if (u32WrPtr > MAX_VDEC_VBBU_ENTRY_COUNT)
    {
#ifdef MSOS_TYPE_LINUX_KERNEL
        printk("%s  %d  fatal error\n", __FUNCTION__, __LINE__);
#endif
        return 0;
    }
#endif

    if (u32WrPtr == 0)
        u32WrPtr = MAX_VDEC_VBBU_ENTRY_COUNT;
    else
        u32WrPtr--;

    stEntry = (VDEC_VBBU_Entry*) &pstVBBU->stEntry[u32WrPtr];

    if(NULL == stEntry)
    {
        return 0;
    }

    //ALOGE("JJJ2: %d %d %d %d", pstVBBU->u32RdPtr, u32WrPtr, stEntry->u32Offset, stEntry->u32Length);
#if SUPPORT_G2VP9
    if (E_VPU_EX_DECODER_G2VP9 == pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype)
    {
        if (stEntry->u32Offset == 0)
            return 0;
        else
            return stEntry->u32Offset + stEntry->u32Length - pVPUHalContext->u32BitstreamAddress[u8OffsetIdx];
    }
    else
#endif
    {
        return stEntry->u32Offset + stEntry->u32Length;
    }
}

MS_BOOL HAL_VPU_EX_Push2VBBU(MS_U32 u32Id, HAL_VPU_EX_PacketInfo *stVpuPkt, MS_VIRT u32VBBUAddr)
{
    MS_U32 u32NewWrPtr;
    VDEC_VBBU_Entry *stEntry;
    VDEC_VBBU *pstVBBU = (VDEC_VBBU *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + u32VBBUAddr);
#if SUPPORT_G2VP9
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
#endif

    if (CHECK_NULL_PTR(pstVBBU) || CHECK_NULL_PTR(stVpuPkt))
        return FALSE;
    MsOS_ReadMemory();

#ifdef STELLAR
    if (pstVBBU->u32WrPtr > MAX_VDEC_VBBU_ENTRY_COUNT)
    {
#ifdef MSOS_TYPE_LINUX_KERNEL
        printk("%s  %d  fatal error\n", __FUNCTION__, __LINE__);
#endif
        return FALSE;
    }
#endif

    stEntry = (VDEC_VBBU_Entry*) &pstVBBU->stEntry[pstVBBU->u32WrPtr];

    u32NewWrPtr = pstVBBU->u32WrPtr + 1;
    if (u32NewWrPtr == (MAX_VDEC_VBBU_ENTRY_COUNT + 1))
    {
        u32NewWrPtr = 0;
    }

    if (u32NewWrPtr == pstVBBU->u32RdPtr) return FALSE;

    stEntry->u32Offset = stVpuPkt->u32Offset;

#if SUPPORT_G2VP9
    if (E_VPU_EX_DECODER_G2VP9 == pVPUHalContext->_stVPUStream[u8OffsetIdx].eDecodertype)
    {
        stEntry->u32Offset += pVPUHalContext->u32BitstreamAddress[u8OffsetIdx];
    }
#endif

    stEntry->u32Length = stVpuPkt->u32Length;
    stEntry->u64TimeStamp = stVpuPkt->u64TimeStamp;
    stEntry->u32ID_H = stVpuPkt->u32ID_H;
    stEntry->u32ID_L = stVpuPkt->u32ID_L;

    MsOS_FlushMemory();//make sure vbbu offset/length already flushed to memory before vbbu wptr advancing

    pstVBBU->u32WrPtr = u32NewWrPtr;

    //ALOGE("JJJ3: %d", pstVBBU->u32WrPtr);

    MsOS_FlushMemory();

    return TRUE;
}

MS_BOOL HAL_VPU_EX_IsVBBUEmpty(MS_VIRT u32VBBUAddr)
{
    VDEC_VBBU *pstVBBU = (VDEC_VBBU *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + u32VBBUAddr);

    if (CHECK_NULL_PTR(pstVBBU))
        return FALSE;
    return pstVBBU->u32RdPtr == pstVBBU->u32WrPtr;
}

///-----------------------------------------------------------------------------
/// specify the command send to Mail box or DRAM
/// @return TRUE or FALSE
///     - TRUE, Mail box
///     - FALSE, Dram
/// @param u32Cmd \b IN: Command is going to be sned
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_IsMailBoxCMD(MS_U32 u32Cmd)
{
    MS_BOOL bResult = TRUE;

    switch (u32Cmd)
    {
            // *********** Runtime action Command
            /*        case E_HVD_CMD_RELEASE_DISPQ:
                    case E_HVD_CMD_UPDATE_DISPQ:
                    case E_HVD_CMD_FLUSH_DEC_Q:
                    case E_HVD_CMD_FLUSH:
                    case E_HVD_CMD_PLAY:
                    case E_HVD_CMD_PAUSE:
                    case E_HVD_CMD_STOP:
                    case E_HVD_CMD_STEP_DECODE:
                    case E_HVD_CMD_SKIP_DEC:
                    case E_HVD_CMD_DISP_I_DIRECT:*/
            // *********** Dual-Stream Create Task Command
        case E_DUAL_CMD_TASK0_HVD_BBU:
        case E_DUAL_CMD_TASK0_HVD_TSP:
        case E_DUAL_CMD_TASK0_MVD_SLQ:
        case E_DUAL_CMD_TASK0_MVD_TSP:
        case E_DUAL_CMD_TASK1_HVD_BBU:
        case E_DUAL_CMD_TASK1_HVD_TSP:
        case E_DUAL_CMD_MODE:
#ifndef _WIN32
        case E_DUAL_CMD_TASK1_MVD_SLQ:
        case E_DUAL_CMD_TASK1_MVD_TSP:
#endif
        case E_DUAL_CMD_DEL_TASK:
        case E_DUAL_CMD_SINGLE_TASK:
        case E_DUAL_VERSION:
        case E_DUAL_R2_CMD_EXIT:
        case E_DUAL_CMD_STC_MODE:
#ifdef VDEC3
        case E_DUAL_R2_CMD_FBADDR:
        case E_DUAL_R2_CMD_FBSIZE:
            // *********** N-Streams
        case E_NST_CMD_TASK_HVD_TSP:
        case E_NST_CMD_TASK_HVD_BBU:
        case E_NST_CMD_TASK_MVD_TSP:
        case E_NST_CMD_TASK_MVD_SLQ:
        case E_NST_CMD_DEL_TASK:
        case E_NST_CMD_COMMON_MASK:
        case E_NST_CMD_COMMON_CMD1:
#endif
        case E_DUAL_CMD_COMMON:
        {
            bResult = TRUE;
        }
        break;
        default:
        {
            bResult = FALSE;
        }
        break;
    }

    return bResult;
}

///-----------------------------------------------------------------------------
/// specify the command send to Mail box or DRAM
/// @return TRUE or FALSE
///     - TRUE, Mail box
///     - FALSE, Dram
/// @param u32Cmd \b IN: Command is going to be sned
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_IsDisplayQueueCMD(MS_U32 u32Cmd)
{
    MS_BOOL bResult = TRUE;

    switch (u32Cmd)
    {
            // *********** Runtime action Command
        case E_HVD_CMD_RELEASE_DISPQ:
        case E_HVD_CMD_UPDATE_DISPQ:
        case E_HVD_CMD_FLUSH_DEC_Q:
        case E_HVD_CMD_PAUSE:
        case E_HVD_CMD_FLUSH:
        case E_HVD_CMD_PLAY:
        case E_HVD_CMD_STOP:
        case E_HVD_CMD_SKIP_DEC:
        case E_HVD_CMD_DISP_I_DIRECT:
        case E_HVD_CMD_STEP_DECODE:
        case E_HVD_CMD_INC_DISPQ_NUM:
        case E_HVD_CMD_DYNAMIC_CONNECT_DISP_PATH:
        case E_HVD_CMD_ADD_REF_DISPQ:
        {
            bResult = TRUE;
        }
        break;
        default:
        {
            bResult = FALSE;
        }
        break;
    }

    return bResult;
}

///-----------------------------------------------------------------------------
/// Send message to HVD stream command queue
/// @return TRUE or FALSE
///     - TRUE, Success
///     - FALSE, Failed
/// @param u32DramAddr \b IN: address to be writen
/// @param u32Msg \b IN: data to be writen
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_DRAMCMDQueueSend(MS_VIRT u32DramAddr, MS_U32 u32Msg)
{
    MS_BOOL bResult = TRUE;

    VPU_MSG_DBG("Send to Command Queue Address=0x%lx, msg=0x%x\n", (unsigned long)u32DramAddr, u32Msg);

    WRITE_LONG(u32DramAddr, u32Msg);

    return bResult;
}

///-----------------------------------------------------------------------------
/// Read task share memory to specify that task command queue is empty or not
/// @return TRUE or FALSE
///     - TRUE, Empty
///     - FALSE, Non empty
/// @param u32Id \b IN: Task information
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_DRAMCMDQueueIsEmpty(void *cmd_queue)
{
//    HVD_ShareMem *pShm = (HVD_ShareMem *) HAL_HVD_EX_GetShmAddr(u32Id);
    CMD_QUEUE *cmd_q = (CMD_QUEUE *)cmd_queue;
    if (!cmd_q)
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return FALSE;
    }

    return cmd_q->u32HVD_STREAM_CMDQ_WD == cmd_q->u32HVD_STREAM_CMDQ_RD;
}

///-----------------------------------------------------------------------------
/// Read task share memory to specify that task command queue is full or not
/// @return TRUE or FALSE
///     - TRUE, Full
///     - FALSE, Non full
/// @param u32Id \b IN: Task information
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_DRAMCMDQueueIsFull(void *cmd_queue)
{
//    HVD_ShareMem *pShm = (HVD_ShareMem *) HAL_HVD_EX_GetShmAddr(u32Id);
    MS_U32 NewWD;
    CMD_QUEUE *cmd_q = (CMD_QUEUE *)cmd_queue;
    if (!cmd_q)
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return TRUE;
    }
    NewWD = cmd_q->u32HVD_STREAM_CMDQ_WD + (HVD_DRAM_CMDQ_CMD_SIZE + HVD_DRAM_CMDQ_ARG_SIZE); //preserve one slot

    if(NewWD >= HVD_CMDQ_DRAM_ST_SIZE)
        NewWD -= HVD_CMDQ_DRAM_ST_SIZE;

    return NewWD == cmd_q->u32HVD_STREAM_CMDQ_RD;
}

MS_U32 HAL_VPU_EX_DRAMStreamCMDQueueSend(MS_U32 u32Id, void *cmd_queue, MS_U8 u8CmdType, MS_U32 u32Msg)
{
    MS_VIRT u32CmdQWdPtr;
    MS_U32 bResult = E_HVD_COMMAND_QUEUE_SEND_FAIL;
    CMD_QUEUE *cmd_q = (CMD_QUEUE *)cmd_queue;
    MS_U8 u8TaskID = HAL_VPU_EX_GetTaskId(u32Id);

#if HVD_ENABLE_MVC
    if (E_HAL_VPU_MVC_STREAM_BASE == u8TaskID)
    {
        u8TaskID = E_HAL_VPU_MAIN_STREAM_BASE;
    }
#endif

    if (CHECK_NULL_PTR(cmd_q))
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return bResult;
    }

    if (CHECK_NULL_PTR((MS_VIRT)cmd_q->u32HVD_CMDQ_DRAM_ST_ADDR))
        return E_HVD_COMMAND_QUEUE_NOT_INITIALED;

    if (HAL_VPU_EX_DRAMCMDQueueIsFull(cmd_q))
        return E_HVD_COMMAND_QUEUE_FULL;
    else
    {
        u32CmdQWdPtr = MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + cmd_q->u32HVD_CMDQ_DRAM_ST_ADDR + cmd_q->u32HVD_STREAM_CMDQ_WD);
    }

    switch (u8CmdType)
    {
        case E_HVD_CMDQ_CMD:
        {
            u32Msg |= (u8TaskID << 24);
            bResult = HAL_VPU_EX_DRAMCMDQueueSend(u32CmdQWdPtr, u32Msg);

            MsOS_FlushMemory();//make sure u32DISPCMDQWdPtr already flushed to memory

            if (bResult)
            {
                cmd_q->u32HVD_STREAM_CMDQ_WD += (HVD_DRAM_CMDQ_CMD_SIZE + HVD_DRAM_CMDQ_ARG_SIZE);

                if (cmd_q->u32HVD_STREAM_CMDQ_WD == HVD_CMDQ_DRAM_ST_SIZE)
                    cmd_q->u32HVD_STREAM_CMDQ_WD = 0;

                bResult = E_HVD_COMMAND_QUEUE_SEND_SUCCESSFUL;
            }
            break;
        }
        case E_HVD_CMDQ_ARG:
        {
            bResult = HAL_VPU_EX_DRAMCMDQueueSend(u32CmdQWdPtr + HVD_DRAM_CMDQ_CMD_SIZE, u32Msg);
            if (bResult)
                bResult = E_HVD_COMMAND_QUEUE_SEND_SUCCESSFUL;
            break;
        }
        default:
        {
            bResult = E_HVD_COMMAND_QUEUE_SEND_FAIL;
            break;
        }
    }

    MsOS_FlushMemory();

    return bResult;
}

///-----------------------------------------------------------------------------
/// Read task share memory to specify that task display command queue is empty or not
/// @return TRUE or FALSE
///     - TRUE, Empty
///     - FALSE, Non empty
/// @param u32Id \b IN: Task information
///-----------------------------------------------------------------------------
MS_BOOL HAL_VPU_EX_DRAMDispCMDQueueIsEmpty(void *cmd_queue)
{
//    HVD_ShareMem *pShm = (HVD_ShareMem *) HAL_HVD_EX_GetShmAddr(u32Id);
    CMD_QUEUE *cmd_q = (CMD_QUEUE *) cmd_queue;
    if (!cmd_q)
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return FALSE;
    }

    return cmd_q->u32HVD_STREAM_DISPCMDQ_WD == cmd_q->u32HVD_STREAM_DISPCMDQ_RD;
}

MS_BOOL HAL_VPU_EX_DRAMDispCMDQueueIsFull(void *cmd_queue)
{
//    HVD_ShareMem *pShm = (HVD_ShareMem *) HAL_HVD_EX_GetShmAddr(u32Id);
    MS_U32 NewWD;
    CMD_QUEUE *cmd_q = (CMD_QUEUE *) cmd_queue;
    if (!cmd_q)
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return TRUE;
    }

    NewWD = cmd_q->u32HVD_STREAM_DISPCMDQ_WD + (HVD_DRAM_CMDQ_CMD_SIZE + HVD_DRAM_CMDQ_ARG_SIZE); //preserve one slot

    if(NewWD >= HVD_DISPCMDQ_DRAM_ST_SIZE)
        NewWD -= HVD_DISPCMDQ_DRAM_ST_SIZE;

    return NewWD == cmd_q->u32HVD_STREAM_DISPCMDQ_RD;
}

MS_U32 HAL_VPU_EX_DRAMStreamDispCMDQueueSend(MS_U32 u32Id, void *cmd_queue, MS_U8 u8CmdType, MS_U32 u32Msg)
{
    HVD_DRAM_COMMAND_QUEUE_SEND_STATUS bResult = E_HVD_COMMAND_QUEUE_SEND_FAIL;
    CMD_QUEUE *cmd_q = (CMD_QUEUE *)cmd_queue;
    MS_U8 u8TaskID = HAL_VPU_EX_GetTaskId(u32Id);
    MS_VIRT u32DISPCMDQWdPtr;


#if HVD_ENABLE_MVC
    if (E_HAL_VPU_MVC_STREAM_BASE == u8TaskID)
    {
        u8TaskID = E_HAL_VPU_MAIN_STREAM_BASE;
    }
#endif

//    HVD_ShareMem *pShm = (HVD_ShareMem *) HAL_HVD_EX_GetShmAddr(u32Id);
    //HVD_EX_MSG_DBG("DP shmAddr=%X u8TaskID = %X u8CmdType = %X u32Msg = %X\n", pShm, u8TaskID, u8CmdType, u32Msg);

    if (CHECK_NULL_PTR(cmd_q))
    {
        VPU_MSG_ERR("Invalid parameter with share memory address=0x%lx %s:%d \n", (unsigned long)cmd_q, __FUNCTION__, __LINE__);
        return bResult;
    }

    if (CHECK_NULL_PTR((MS_VIRT)cmd_q->u32HVD_DISPCMDQ_DRAM_ST_ADDR))
        return E_HVD_COMMAND_QUEUE_NOT_INITIALED;

    if (HAL_VPU_EX_DRAMDispCMDQueueIsFull(cmd_q))
        return E_HVD_COMMAND_QUEUE_FULL;
    else
    {
        u32DISPCMDQWdPtr = MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + cmd_q->u32HVD_DISPCMDQ_DRAM_ST_ADDR + cmd_q->u32HVD_STREAM_DISPCMDQ_WD);
    }

    if (u32DISPCMDQWdPtr == NULL)
    {
        printf("%s pVPUHalContext->u32FWCodeAddr:%x  addr:%x  wd:%x\n", __FUNCTION__, (unsigned int)pVPUHalContext->u32FWCodeAddr,
               (unsigned int)cmd_q->u32HVD_DISPCMDQ_DRAM_ST_ADDR,
               (unsigned int)cmd_q->u32HVD_STREAM_DISPCMDQ_WD);
        return E_HVD_COMMAND_QUEUE_SEND_FAIL;
    }

    // HVD_EX_MSG_DBG("VDispCmdQ_BASE_ADDR=%X PDispCmsQ_BASE_ADDR=%X u32DISPCMDQWdPtr=%X DISPCMDQ_TOTAL_SIZE = %X\n", cmd_q->u32HVD_DISPCMDQ_DRAM_ST_ADDR, pVPUHalContext->u32FWCodeVAddr + cmd_q->u32HVD_DISPCMDQ_DRAM_ST_ADDR, u32DISPCMDQWdPtr,HVD_DISPCMDQ_DRAM_ST_SIZE);

    switch (u8CmdType)
    {
        case E_HVD_CMDQ_CMD:
        {
            u32Msg |= (u8TaskID << 24);
            bResult = HAL_VPU_EX_DRAMCMDQueueSend(u32DISPCMDQWdPtr, u32Msg);

            MsOS_FlushMemory();//make sure u32DISPCMDQWdPtr already flushed to memory

            if (bResult)
            {
                cmd_q->u32HVD_STREAM_DISPCMDQ_WD += (HVD_DRAM_CMDQ_CMD_SIZE + HVD_DRAM_CMDQ_ARG_SIZE);

                if (cmd_q->u32HVD_STREAM_DISPCMDQ_WD == HVD_DISPCMDQ_DRAM_ST_SIZE)
                    cmd_q->u32HVD_STREAM_DISPCMDQ_WD = 0;

                bResult = E_HVD_COMMAND_QUEUE_SEND_SUCCESSFUL;
            }
            break;
        }
        case E_HVD_CMDQ_ARG:
        {
            bResult = HAL_VPU_EX_DRAMCMDQueueSend(u32DISPCMDQWdPtr + HVD_DRAM_CMDQ_CMD_SIZE, u32Msg);
            if (bResult)
                bResult = E_HVD_COMMAND_QUEUE_SEND_SUCCESSFUL;
            break;
        }
        default:
        {
            bResult = E_HVD_COMMAND_QUEUE_SEND_FAIL;
            break;
        }
    }

    MsOS_FlushMemory();

    return bResult;
}

MS_BOOL HAL_VPU_EX_SetBitstreamBufAddress(MS_U32 u32Id, MS_VIRT u32BsAddr)
{
    MS_U8 u8OffsetIdx = _VPU_EX_GetOffsetIdx(u32Id);
    MS_U32 u32StAddr = 0;
    MS_U8 u8TmpMiuSel = 0;

    _phy_to_miu_offset(u8TmpMiuSel, u32StAddr, u32BsAddr);

    pVPUHalContext->u32BitstreamAddress[u8OffsetIdx] = u32StAddr;

    return TRUE;
}
#endif

void HAL_VPU_EX_DynamicFBMode(MS_BOOL bEnable, MS_PHY u32address, MS_U32 u32Size)
{
    pVPUHalContext->bEnableDymanicFBMode = bEnable;

    if(u32address >= HAL_MIU1_BASE)
    {
        pVPUHalContext->u32DynamicFBAddress = u32address - HAL_MIU1_BASE;
    }
    else
    {
        pVPUHalContext->u32DynamicFBAddress = u32address;
    }

    pVPUHalContext->u32DynamicFBSize = u32Size;
}
#ifdef CONFIG_MSTAR_CLKM
void HAL_VPU_EX_SetClkManagement(VPU_EX_ClkPortType eClkPortType, MS_BOOL bEnable)
{
    MS_S32 handle;
    switch(eClkPortType)
    {
        case E_VPU_EX_CLKPORT_MVD:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_mvd");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_MVD_CORE:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_mvd_core");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_MVD_PAS:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_mvd_pas");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_HVD:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_hvd");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_HVD_IDB:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_hvd_idb");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_HVD_AEC:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_hvd_aec");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_HVD_AEC_LITE:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_hvd_aec_lite");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_VP8:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_vp8");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_EVD:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_evd");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_EVD_PPU:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_evd_ppu");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_EVD_LITE:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_evd_lite");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_EVD_PPU_LITE:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_evd_ppu_lite");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "FAST_MODE");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_VD_MHEG5:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_vd_mheg5");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
        case E_VPU_EX_CLKPORT_VD_MHEG5_LITE:
        {
            handle = Drv_Clkm_Get_Handle("g_clk_vd_mheg5_lite");
            if(bEnable)
            {
                Drv_Clkm_Set_Clk_Source(handle, "");
            }
            else
            {
                Drv_Clkm_Clk_Gate_Disable(handle);
            }
            break;
        }
    }
}
#endif  //#ifdef CONFIG_MSTAR_CLKM

MS_SIZE _VPU_EX_GetFrameBufferDefaultSize(VPU_EX_CodecType eCodecType, MS_BOOL bUseSubSample)
{
    MS_SIZE FrameBufferSize = 0;

    (void)bUseSubSample;

    switch(eCodecType)
    {
        case E_VPU_EX_CODEC_TYPE_MPEG2:
        case E_VPU_EX_CODEC_TYPE_H263:
        case E_VPU_EX_CODEC_TYPE_MPEG4:
        case E_VPU_EX_CODEC_TYPE_DIVX311:
        case E_VPU_EX_CODEC_TYPE_DIVX412:
        case E_VPU_EX_CODEC_TYPE_FLV:
            FrameBufferSize = 0x1E00000;
            break;
        case E_VPU_EX_CODEC_TYPE_VC1_ADV:
        case E_VPU_EX_CODEC_TYPE_VC1_MAIN:
            FrameBufferSize = 0x6C00000;
            break;
        case E_VPU_EX_CODEC_TYPE_RV8:
        case E_VPU_EX_CODEC_TYPE_RV9:
            FrameBufferSize = 0x1B00000;
            break;
        case E_VPU_EX_CODEC_TYPE_VP8:
            FrameBufferSize = 0x1500000;
            break;
        case E_VPU_EX_CODEC_TYPE_H264:
            FrameBufferSize = 0x8200000;
            //FrameBufferSize = 0x7A00000;  //UHD 122MB ,5 ref frame
            //FrameBufferSize = 0x7A80000;  //UHD 4K2K 16:19  126.5MB
            //FrameBufferSize = 0x8E00000;  //UHD 4K2K 16:19  142MB
            break;
        case E_VPU_EX_CODEC_TYPE_AVS:
            FrameBufferSize = 0x1B00000;
            break;
        case E_VPU_EX_CODEC_TYPE_MJPEG:
            FrameBufferSize = 0x2800000;
            break;
        case E_VPU_EX_CODEC_TYPE_MVC:
            FrameBufferSize = 0x4200000;
            break;
        case E_VPU_EX_CODEC_TYPE_HEVC_DV:
            FrameBufferSize = 0xB000000;
            break;
        case E_VPU_EX_CODEC_TYPE_HEVC:
#if SUPPORT_MSVP9
        case E_VPU_EX_CODEC_TYPE_VP9:
#endif
            FrameBufferSize = 0xA000000;
            break;
#if !SUPPORT_MSVP9
        case E_VPU_EX_CODEC_TYPE_VP9:
            FrameBufferSize = 0x7800000;
            break;
#endif
        default:
            FrameBufferSize = 0;
            break;
    }

    return FrameBufferSize;
}

MS_SIZE HAL_VPU_EX_GetFrameBufferDefaultSize(VPU_EX_CodecType eCodecType, MS_BOOL bUseSubSample)
{
    return _VPU_EX_GetFrameBufferDefaultSize(eCodecType, bUseSubSample);
}

// To-do: Taking the source type into consideration
MS_BOOL HAL_VPU_EX_GetCMAMemSize(VPU_EX_CodecType eCodecType, VPU_EX_SrcMode eSrcMode,
                                 MS_U64 *offset, MS_SIZE *length, MS_U64 total_length, MS_SIZE unUseSize, MS_BOOL bUseSubSample)
{
    MS_SIZE FrameBufferSize = 0;

    if (!offset || !length)
        return FALSE;

    total_length -= unUseSize;
    VPRINTF("[HAL][%s]:[%d] total_length:%llu, cType:%d, sType:%d\n", __FUNCTION__, __LINE__,
            (unsigned long long)total_length, (int)eCodecType, (int)eSrcMode);

    FrameBufferSize = _VPU_EX_GetFrameBufferDefaultSize(eCodecType, bUseSubSample);

    if(FrameBufferSize == 0)
    {
        return FALSE;
    }
    VPRINTF("[HAL][%s]:[%d] FrameSize:%llu, offset:%llu, length:%llu ", __FUNCTION__, __LINE__,
            (unsigned long long)FrameBufferSize, (unsigned long long)*offset, (unsigned long long)*length);
    if (total_length < FrameBufferSize)
    {
        *offset = unUseSize;
        *length = total_length;
    }
    else  // todo, dual decode case
    {
        *offset = unUseSize;
        *length = FrameBufferSize;
    }
    return TRUE;
}

static int HAL_VPU_EX_strcmp(const char *string1, const char *string2)
{
    int iRet, i;

    i = 0;
    while(string1[i] || string2[i])
    {
        iRet = string1[i] - string2[i];
        if(iRet)
        {
            return iRet;
        }
        i++;
    }
    return 0;
}

// For HAL_VPU_EX_GetCapability: E_CAP_NDEC_FB_SEARCH_LIST/E_CAP_NDEC_BS_SEARCH_LIST
#define VPU_EX_BUFFERTYPE_TYPE_MASK           0x00FF
#define VPU_EX_BUFFERTYPE_BASE_MASK           0xFF00
#define VPU_EX_BUFFERTYPE_GET_BASE_IDX(eType) ((((eType) & VPU_EX_BUFFERTYPE_BASE_MASK) >> 8))
#define VPU_EX_BUFFERTYPE_GET_TYPE_IDX(eType) ((((eType) & VPU_EX_BUFFERTYPE_TYPE_MASK)) - 1)
#define VPU_EX_BUFFERTYPE_BASE_CNT            2
#define VPU_EX_BUFFERTYPE_TYPE_FB_CNT         (E_VPU_EX_BUFFERTYPE_FB_NUM - E_VPU_EX_BUFFERTYPE_FB_BASE - 1)
#define VPU_EX_BUFFERTYPE_TYPE_BS_CNT         (E_VPU_EX_BUFFERTYPE_BS_NUM - E_VPU_EX_BUFFERTYPE_BS_BASE - 1)
#define VPU_EX_BUFFERTYPE_TYPE_RECORD_CNT     MAX(VPU_EX_BUFFERTYPE_TYPE_FB_CNT, VPU_EX_BUFFERTYPE_TYPE_BS_CNT)

typedef enum
{
    E_VPU_EX_ALLOCATOR_STREAMTYPE_NONE = -1,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_MAIN_DTV = 0,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_MAIN_MM,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_SUB_DTV,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_SUB_MM,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_N_DTV,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_N_MM,
    E_VPU_EX_ALLOCATOR_STREAMTYPE_NUM,
} VPU_EX_AllocatorStreamType;

typedef enum
{
    E_VPU_EX_BUFFERTYPE_FB_BASE = 0x0000,          /// 0x0000 : Frame Buffer Type Base
    E_VPU_EX_BUFFERTYPE_FB_MAIN,                   /// 0x0001 : Frame Buffer Main
    E_VPU_EX_BUFFERTYPE_FB_SUB,                    /// 0x0002 : Frame Buffer Sub
    E_VPU_EX_BUFFERTYPE_FB_BW,                     /// 0x0003 : Frame Buffer Bandwidth Balance
    E_VPU_EX_BUFFERTYPE_FB_NUM,                    /// Number of Frame Buffer Type

    E_VPU_EX_BUFFERTYPE_BS_BASE = 0x0100,          /// 0x0100 : Bitstream Buffer Type Base
    E_VPU_EX_BUFFERTYPE_BS_MAIN,                   /// 0x0101 : Bitstream Buffer Main
    E_VPU_EX_BUFFERTYPE_BS_SUB,                    /// 0x0102 : Bitstream Buffer Sub
    E_VPU_EX_BUFFERTYPE_BS_NUM,                    /// Number of Bitstream Buffer Type

    E_VPU_EX_BUFFERTYPE_HDRSHM_BASE = 0x0200,      /// 0x0200 : HDR Share Mem Type Base
    E_VPU_EX_BUFFERTYPE_HDRSHM_MAIN,               /// 0x0201 : HDR Share Mem Main
    E_VPU_EX_BUFFERTYPE_HDRSHM_SUB,                /// 0x0202 : HDR Share Mem Sub
    E_VPU_EX_BUFFERTYPE_HDRSHM_NUM,                /// Number of HDR Share Mem Type

    E_VPU_EX_BUFFERTYPE_NONE = 0xFFFF,             /// Undefine Buffer Type
} VPU_EX_BufferType;

typedef enum
{
    E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE = -1,
    E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_ANY = 0,
    E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD,
    E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL,
    E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NUM,
} VPU_EX_AllocatorAllocateDirection;

typedef struct DLL_PACKED
{
    VPU_EX_BufferType eType;
    VPU_EX_AllocatorAllocateDirection eAllocateDirection;
    MS_U32 u32ExInfo;   //other information for this buffer
} VPU_EX_AllocatorBufferInfo;

static VPU_EX_AllocatorBufferInfo gVPU_EX_AllocatorBufferFBbufferSearchList[E_VPU_EX_ALLOCATOR_STREAMTYPE_NUM][VPU_EX_BUFFERTYPE_TYPE_FB_CNT] =
{
    /* 1st */                                                                     /* 2nd */                                                                     /* 3rd */
    /* Main_DTV */ {{E_VPU_EX_BUFFERTYPE_FB_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}} ,
    /* Main_MM  */ {{E_VPU_EX_BUFFERTYPE_FB_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}} ,
    /* Sub_DTV  */ {{E_VPU_EX_BUFFERTYPE_FB_SUB,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}} ,
    /* Sub_MM   */ {{E_VPU_EX_BUFFERTYPE_FB_SUB,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}} ,
    /* N_DTV    */ {{E_VPU_EX_BUFFERTYPE_FB_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL, 0}, {E_VPU_EX_BUFFERTYPE_NONE,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}} ,
    /* N_MM     */ {{E_VPU_EX_BUFFERTYPE_FB_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL, 0}, {E_VPU_EX_BUFFERTYPE_FB_BW, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL, 0},   {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}}
};

static VPU_EX_AllocatorBufferInfo gVPU_EX_AllocatorBufferBSbufferSearchList[E_VPU_EX_ALLOCATOR_STREAMTYPE_NUM][VPU_EX_BUFFERTYPE_TYPE_BS_CNT] =
{
    /* 1st */                                                                      /* 2nd */
    /* Main_DTV */ {{E_VPU_EX_BUFFERTYPE_BS_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}},
    /* Main_MM  */ {{E_VPU_EX_BUFFERTYPE_BS_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}},
    /* Sub_DTV  */ {{E_VPU_EX_BUFFERTYPE_BS_SUB,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}},
    /* Sub_MM   */ {{E_VPU_EX_BUFFERTYPE_BS_SUB,  E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_HEAD, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}},
    /* N_DTV    */ {{E_VPU_EX_BUFFERTYPE_BS_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}},
    /* N_MM     */ {{E_VPU_EX_BUFFERTYPE_BS_MAIN, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_TAIL, 0}, {E_VPU_EX_BUFFERTYPE_NONE, E_VPU_EX_ALLOCATOR_ALLOCATE_DIRECTION_NONE, 0}}

};

MS_BOOL HAL_VPU_EX_GetCapability(MS_U8 *pu8CmdNameIn, void *pParamIn, void *pParamOut)
{
    typedef enum
    {
        E_CAP_NDEC_FB_MIU_SELECT,
        E_CAP_NDEC_NSTREAM_FB_SIZE,
        E_CAP_NDEC_NSTREAM_BS_SIZE,
        E_CAP_NDEC_SUPPORT_SHAREBBU,
        E_CAP_SUPPORT_ALLOCATOR,
        E_CAP_ALWAYS_OPTEE_LOAD_FW,
        E_CAP_NDEC_FB_SEARCH_LIST,
        E_CAP_NDEC_BS_SEARCH_LIST,

    } VPU_EX_GetCapabilityCmdId;

    typedef struct
    {
        VPU_EX_GetCapabilityCmdId eCmdId;
        MS_U8 u8CmdName[32];

    } VPU_EX_GetCapabilityCmd;

    static VPU_EX_GetCapabilityCmd stCapCmd[] =
    {
        {E_CAP_NDEC_FB_MIU_SELECT,    "CAP_NDEC_FB_MIU_SELECT"},
        {E_CAP_NDEC_NSTREAM_FB_SIZE,  "CAP_NDEC_NSTREAM_FB_SIZE"},
        {E_CAP_NDEC_NSTREAM_BS_SIZE,  "CAP_NDEC_NSTREAM_BS_SIZE"},
        {E_CAP_NDEC_SUPPORT_SHAREBBU, "CAP_NDEC_SUPPORT_SHAREBBU"},
        {E_CAP_SUPPORT_ALLOCATOR,     "CAP_SUPPORT_ALLOCATOR"},
        {E_CAP_ALWAYS_OPTEE_LOAD_FW,  "CAP_ALWAYS_OPTEE_LOAD_FW"},
        {E_CAP_NDEC_FB_SEARCH_LIST,   "CAP_NDEC_FB_SEARCH_LIST"},
        {E_CAP_NDEC_BS_SEARCH_LIST,   "CAP_NDEC_BS_SEARCH_LIST"},
    };

    MS_U32 bRet = FALSE;
    MS_U32 u32TotalCmdCnt = sizeof(stCapCmd) / sizeof(stCapCmd[0]);
    MS_U32 u32CmdCnt;

    for(u32CmdCnt = 0; u32CmdCnt < u32TotalCmdCnt; u32CmdCnt++)
    {
        if(HAL_VPU_EX_strcmp((const char *)(stCapCmd[u32CmdCnt].u8CmdName), (const char *)pu8CmdNameIn) == 0)
        {
            bRet = TRUE;
            switch(stCapCmd[u32CmdCnt].eCmdId)
            {
                case E_CAP_NDEC_FB_MIU_SELECT:
                {
                    VPU_EX_CodecType eCodecType = *((VPU_EX_CodecType *)pParamIn);
                    MS_BOOL *pSupport = (MS_BOOL *)pParamOut;
                    if(eCodecType == E_VPU_EX_CODEC_TYPE_MJPEG ||
                            eCodecType == E_VPU_EX_CODEC_TYPE_VP8)
                    {
                        *pSupport = FALSE;
                        VPU_MSG_ERR("[NDec][ALLOC] Not support frame buffer at different MIU: VP8, MJPEG\n");
                    }
                    else
                    {
                        *pSupport = TRUE;
                    }
                }
                break;

                // FIXME: Patch for waiting dynamic frame buffer and preset task_spec
                case E_CAP_NDEC_NSTREAM_FB_SIZE:
                {
                    MS_SIZE *pSize = (MS_SIZE *)pParamOut;
                    *pSize = 0x3500000;  // 53M
                }
                break;

                // FIXME: Patch for waiting dynamic bitstream buffer
                case E_CAP_NDEC_NSTREAM_BS_SIZE:
                {
                    MS_SIZE *pSize = (MS_SIZE *)pParamOut;
                    *pSize = 0x400000;  // 4M
                }
                break;

                case E_CAP_NDEC_SUPPORT_SHAREBBU:
                {
                    MS_BOOL *pbSupportShareBBU = (MS_BOOL *)pParamOut;
                    *pbSupportShareBBU = TRUE;
                }
                break;

                case E_CAP_SUPPORT_ALLOCATOR:
                {
                    MS_BOOL *pSupport = (MS_BOOL *)pParamOut;
                    *pSupport = TRUE;
                }
                break;

                case E_CAP_ALWAYS_OPTEE_LOAD_FW:
                {
                    MS_BOOL *pSupport = (MS_BOOL *)pParamOut;
                    *pSupport = TRUE;
                }
                break;

                case E_CAP_NDEC_FB_SEARCH_LIST:
                {
                    VPU_EX_AllocatorStreamType eStreamType = *((VPU_EX_AllocatorStreamType *)pParamIn);
                    VPU_EX_AllocatorBufferInfo **ppBufferCandidate = (VPU_EX_AllocatorBufferInfo **)pParamOut;
                    *ppBufferCandidate = gVPU_EX_AllocatorBufferFBbufferSearchList[eStreamType];
                }
                break;

                case E_CAP_NDEC_BS_SEARCH_LIST:
                {
                    VPU_EX_AllocatorStreamType eStreamType = *((VPU_EX_AllocatorStreamType *)pParamIn);
                    VPU_EX_AllocatorBufferInfo **ppBufferCandidate = (VPU_EX_AllocatorBufferInfo **)pParamOut;
                    *ppBufferCandidate = gVPU_EX_AllocatorBufferBSbufferSearchList[eStreamType];
                }
                break;

                default:
                    bRet = FALSE;
                    break;
            }
        }
    }

    return bRet;
}

#define HVD_FHD_MAX_WIDTH   1920
#define HVD_FHD_MAX_HEIGHT  1080
#define HVD_UHD_MAX_WIDTH   3840
#define HVD_UHD_MAX_HEIGHT  2160

static MS_BOOL is_EVD(MS_U32 u32CodecType)
{
    return ((u32CodecType == E_HVD_Codec_HEVC)
            || (u32CodecType == E_HVD_Codec_HEVC_DV)
            || (u32CodecType == E_HVD_Codec_AVS2)
            || (u32CodecType == E_HVD_Codec_VP9));
}

static MS_BOOL is_HVD(MS_U32 u32CodecType)
{
    return ((u32CodecType == E_HVD_Codec_AVC)
            || (u32CodecType == E_HVD_Codec_MVC)
            || (u32CodecType == E_HVD_Codec_AVS)
            || (u32CodecType == E_HVD_Codec_RM)
            || (u32CodecType == E_HVD_Codec_VP8));
}

static MS_U32 _HAL_VPU_EX_get_approximate_framerate(MS_U32 framerate)
{
    if (framerate >= 14500 && framerate <= 15000)
        return 15000;
    else if (framerate >= 23000 && framerate <= 24010)  //because VDplayer rounding to 23 for 23.976
        return 24000;
    else if (framerate >= 29000 && framerate <= 30010)
        return 30000;
    else if (framerate >= 59000 && framerate <= 61000)
        return 60000;
    else
        return framerate;
}

static MS_U32 _HAL_VPU_EX_get_task_loading(MS_U32 Res, MS_U32 FrameRate)
{
    // Res: w * h, FrameRate: fps * 1000, return pixel rate
    FrameRate = _HAL_VPU_EX_get_approximate_framerate(FrameRate) / 1000;
    return Res * FrameRate;
}

static MS_U32 _HAL_VPU_EX_GetEVDloading(MS_U8 u8CurOffset)
{
    MS_U8 i;
    MS_U32 total_Loading = 0;
    HVD_ShareMem *pShm = NULL;
    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if (i != u8CurOffset)
        {
            MS_U32 task_Res;
            MS_U32 task_FrameRate;

            if (pVPUHalContext->u32FWShareInfoAddr[i] == 0xFFFFFFFFUL)
                pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + (i * FW_TASK_SIZE) + HVD_SHARE_MEM_ST_OFFSET);
            else//TEE
                pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWShareInfoAddr[i] + COMMON_AREA_SIZE);

            task_Res = pShm->DispInfo.u16HorSize * pShm->DispInfo.u16VerSize;
            task_FrameRate = pShm->DispInfo.u32FrameRate;

            if (is_EVD(pShm->u32CodecType))
            {
                total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate);
                if (pShm->u32CodecType == E_HVD_Codec_HEVC_DV)//Add EL loading, we only support 1:4 HDR
                    total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate) / 4;
            }
            else if (is_HVD(pShm->u32CodecType) && pShm->IsRunMFcodec)
            {
                // If HVD enable MFCodec, it would share EVD hi-codec decoding time
                total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate);
                if (pShm->u32CodecType == E_HVD_Codec_MVC)//Add second view loading
                    total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate);
            }
        }
    }
    return total_Loading;
}

static MS_U32 _HAL_VPU_EX_GetHVDloading(MS_U8 u8CurOffset)
{
    MS_U8 i;
    MS_U32 total_Loading = 0;
    HVD_ShareMem *pShm = NULL;
    for (i = 0; i < sizeof(pVPUHalContext->_stVPUStream) / sizeof(pVPUHalContext->_stVPUStream[0]); i++)
    {
        if (i != u8CurOffset)
        {
            if (pVPUHalContext->u32FWShareInfoAddr[i] == 0xFFFFFFFFUL)
                pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + (i * FW_TASK_SIZE) + HVD_SHARE_MEM_ST_OFFSET);
            else//TEE
                pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWShareInfoAddr[i] + COMMON_AREA_SIZE);

            if (is_HVD(pShm->u32CodecType))
            {
                MS_U32 task_Res = pShm->DispInfo.u16HorSize * pShm->DispInfo.u16VerSize;
                MS_U32 task_FrameRate = pShm->DispInfo.u32FrameRate;
                total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate);
                if (pShm->u32CodecType == E_HVD_Codec_MVC)//Add second view loading
                    total_Loading += _HAL_VPU_EX_get_task_loading(task_Res, task_FrameRate);
            }
        }
    }
    return total_Loading;
}

void HAL_VPU_EX_TrickSmoothScan(MS_U32 u32Id, VPU_EX_TrickSmoothScan *stTrickSmoothScan)
{
    MS_U8 u8CurOffset = _VPU_EX_GetOffsetIdx(u32Id);
    HVD_ShareMem *pShm = NULL;
    VPRINTF("HAL_VPU_EX_TrickSmoothScan: %d %d\n", stTrickSmoothScan->bEnable, stTrickSmoothScan->eDispSpeed);
    if (pVPUHalContext->u32FWShareInfoAddr[u8CurOffset] == 0xFFFFFFFFUL)
        pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWCodeAddr + (u8CurOffset * FW_TASK_SIZE) + HVD_SHARE_MEM_ST_OFFSET);
    else//TEE
        pShm = (HVD_ShareMem *)MsOS_PA2KSEG1(pVPUHalContext->u32FWShareInfoAddr[u8CurOffset] + COMMON_AREA_SIZE);

    if (stTrickSmoothScan->bEnable == TRUE)
    {
        if (stTrickSmoothScan->eDispSpeed > E_VPU_EX_DISP_SPEED_NORMAL_1X)
        {
            MS_U32 u32HWPixelRate = 0, u32RealPixelRate = 0;
            MS_U32 u32Res = pShm->DispInfo.u16HorSize * pShm->DispInfo.u16VerSize;
            MS_U32 u32FrameRate = pShm->DispInfo.u32FrameRate;

            switch (pShm->u32CodecType)
            {

                case E_HVD_Codec_HEVC:
                {
                    if (E_MSTAR_CHIP_K6LITE == pShm->u16ChipID)//4K@60
                    {
                        u32HWPixelRate = HVD_UHD_MAX_WIDTH * HVD_UHD_MAX_HEIGHT * 60;
                    }
                    u32RealPixelRate = _HAL_VPU_EX_GetEVDloading(u8CurOffset) + _HAL_VPU_EX_get_task_loading(u32Res, u32FrameRate) * stTrickSmoothScan->eDispSpeed;
                }
                break;

                case E_HVD_Codec_AVC:
                {
                    if (E_MSTAR_CHIP_K6LITE == pShm->u16ChipID)//4K@30
                    {
                        u32HWPixelRate = HVD_UHD_MAX_WIDTH * HVD_UHD_MAX_HEIGHT * 30;
                    }

                    u32RealPixelRate = _HAL_VPU_EX_GetHVDloading(u8CurOffset) + _HAL_VPU_EX_get_task_loading(u32Res, u32FrameRate) * stTrickSmoothScan->eDispSpeed;
                }
                break;

                default:
                {
                    stTrickSmoothScan->bEnable = FALSE;
                    goto SMOOTHSCAN_END;
                }

            }
            //VPRINTF("u32RealPixelRate %d, u32HWPixelRate %d\n", u32RealPixelRate, u32HWPixelRate);

            //smoothly scan condition check
            if (u32RealPixelRate <= u32HWPixelRate)
            {
                stTrickSmoothScan->eSmoothScanMode = VPU_EX_SMOOTH_SCAN_HW;
            }
            else if (u32RealPixelRate <= u32HWPixelRate * 4)
            {
                stTrickSmoothScan->eSmoothScanMode = VPU_EX_SMOOTH_SCAN_VDEC_PLUS;
            }
            else
            {
                stTrickSmoothScan->eSmoothScanMode = VPU_EX_SMOOTH_SCAN_UNSUPPORT;
            }
        }
        else
        {
            stTrickSmoothScan->bEnable = FALSE;
        }
    }

SMOOTHSCAN_END:
    if (stTrickSmoothScan->bEnable == FALSE)
    {
        stTrickSmoothScan->eSmoothScanMode = VPU_EX_SMOOTH_SCAN_NONE;
    }
    pShm->u8SmoothScanMode = stTrickSmoothScan->eSmoothScanMode;
    VPRINTF("u8SmoothScanMode %d\n", pShm->u8SmoothScanMode);

    return;
}

#else
#include "halVPU_EX.h"
#include "drvMMIO.h"
#include "../hvd_v3/regHVD_EX.h"
#include "halCHIP.h"
#include "controller.h"

#if defined(MSOS_TYPE_NUTTX)
extern int lib_lowprintf(const char *fmt, ...);
#define PRINTF lib_lowprintf
#elif defined(MSOS_TYPE_OPTEE)
#define PRINTF printf
#endif

#define HVD_LWORD(x)    (MS_U16)((x)&0xffff)
#define HVD_HWORD(x)    (MS_U16)(((x)>>16)&0xffff)

MS_U8 u8FW_Binary[] =
{
#include "fwVPU.dat"
};

MS_U32 u32HVDRegOSBase;

MS_BOOL HAL_VPU_EX_LoadCodeInSecure(MS_VIRT addr)
{
    //PRINTF("do load code,u32DestAddr %x\n",addr);
    memcpy((void*)addr, (void*)u8FW_Binary, sizeof(u8FW_Binary));
    MAsm_CPU_Sync();
    MsOS_FlushMemory();

    if (FALSE == (*((MS_U8*)(addr + 6)) == 'R' && *((MS_U8*)(addr + 7)) == '2'))
    {
        PRINTF("FW is not R2 version! _%x_ _%x_\n", *(MS_U8*)(addr + 6), *(MS_U8*)(addr + 7));
        return FALSE;
    }
    return TRUE;
}

MS_BOOL HAL_VPU_EX_SetLockDownRegister(void* param)
{
#if 1
    MS_PHY u32StAddr_main;
    MS_PHY u32StAddr_sub;
    MS_U32 u32NonPMBankSize = 0;
    VPU_EX_LOCK_DOWN_REGISTER* register_lockdown;

    if(param == NULL)
    {
        return FALSE;
    }

    register_lockdown = (VPU_EX_LOCK_DOWN_REGISTER*)param;

    MDrv_MMIO_GetBASE(&u32HVDRegOSBase, &u32NonPMBankSize, MS_MODULE_HW);

    // ES buffer
    u32StAddr_main = register_lockdown->Bitstream_Addr_Main;
    u32StAddr_sub = register_lockdown->Bitstream_Addr_Sub;


    MS_PHY u32StartOffset;
    MS_U8  u8MiuSel;

    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32StAddr_main);
    u32StAddr_main = u32StartOffset;

    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32StAddr_sub);
    u32StAddr_sub = u32StartOffset;

    //Lock down register
    _HVD_Write2Byte(HVD_REG_ESB_ST_ADDR_L(REG_HVD_BASE), HVD_LWORD(u32StAddr_main >> 3));
    _HVD_Write2Byte(HVD_REG_ESB_ST_ADDR_H(REG_HVD_BASE), HVD_HWORD(u32StAddr_main >> 3));

    _HVD_Write2Byte(HVD_REG_ESB_ST_ADDR_L_BS2(REG_HVD_BASE), HVD_LWORD(u32StAddr_sub >> 3));
    _HVD_Write2Byte(HVD_REG_ESB_ST_ADDR_H_BS2(REG_HVD_BASE), HVD_HWORD(u32StAddr_sub >> 3));
    //~

    // Lock Down
    //_HVD_Write2Byte(HVD_REG_HI_DUMMY_0, (_HVD_Read2Byte(HVD_REG_HI_DUMMY_0) | (HVD_REG_LOCK_REG_ESB_ST_ADR_L_H|HVD_REG_LOCK_REG_ESB_ST_ADR_L_H_BS2)));
    //~
#endif
    return TRUE;
}

MS_U32 HAL_VPU_EX_GetSHM_Offset(void)
{
    return COMMON_AREA_START;
}
#endif
