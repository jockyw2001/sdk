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
#define __DRV_SCL_VIP_IO_WRAPPER_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_verchk.h"
#include "drv_scl_vip_m_st.h"
#include "drv_scl_vip_m.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_ctx_m.h"
#include "drv_scl_ctx_st.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define AIP_PARSING(x)  ( x == E_MDRV_SCLVIP_AIP_YEE                     ?    "AIP_YEE"           : \
                          x == E_MDRV_SCLVIP_AIP_YEE_AC_LUT              ?    "AIP_YEE_AC_LUT"    : \
                          x == E_MDRV_SCLVIP_AIP_MXNR                    ?    "AIP_MXNR"          : \
                          x == E_MDRV_SCLVIP_AIP_UVADJ                   ?    "AIP_UVADJ"         : \
                          x == E_MDRV_SCLVIP_AIP_XNR                     ?    "AIP_XNR"           : \
                          x == E_MDRV_SCLVIP_AIP_YCUVM                   ?    "AIP_YCUVM"         : \
                          x == E_MDRV_SCLVIP_AIP_COLORTRAN               ?    "AIP_COLORTRAN"     : \
                          x == E_MDRV_SCLVIP_AIP_GAMMA                   ?    "AIP_GAMMA"         : \
                          x == E_MDRV_SCLVIP_AIP_422TO444                ?    "AIP_422TO444"      : \
                          x == E_MDRV_SCLVIP_AIP_YUVTORGB                ?    "AIP_YUVTORGB"      : \
                          x == E_MDRV_SCLVIP_AIP_GM10TO12                ?    "AIP_GM10TO12"      :\
                          x == E_MDRV_SCLVIP_AIP_CCM                     ?    "AIP_CCM"           : \
                          x == E_MDRV_SCLVIP_AIP_HSV                     ?    "AIP_HSV"           : \
                          x == E_MDRV_SCLVIP_AIP_GM12TO10                ?    "AIP_GM12TO10"      : \
                          x == E_MDRV_SCLVIP_AIP_RGBTOYUV                ?    "AIP_RGBTOYUV"      : \
                          x == E_MDRV_SCLVIP_AIP_444TO422                ?    "AIP_444TO422"      : \
                          x == E_MDRV_SCLVIP_AIP_PFC                     ?    "AIP_PFC"           : \
                          x == E_MDRV_SCLVIP_AIP_WDR                     ?    "AIP_WDR"           : \
                          x == E_MDRV_SCLVIP_AIP_ARBSHP                  ?    "AIP_ARBSHP"        : \
                          x == E_MDRV_SCLVIP_AIP_FORVPECFG               ?    "AIP_VPECFG"        : \
                          x == E_MDRV_SCLVIP_AIP_FORVPEONOFF             ?    "AIP_VPEONOFF"      : \
                          x == E_MDRV_SCLVIP_AIP_FORVPEPROCESS           ?    "AIP_VPEPROCESS"    : \
                          x == E_MDRV_SCLVIP_AIP_FORVPEPROCESSNE         ?    "AIP_VPEPROCESSNE"  : \
                                                                            "UNKNOWN")
#if 0

#define DRV_SCLHVSP_IO_LOCK_MUTEX(x)    \
        SCL_ERR("+++ [MUTEX_LOCK][%s]_1_[%d] \n", __FUNCTION__, __LINE__); \
        DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER); \
        SCL_ERR("+++ [MUTEX_LOCK][%s]_2_[%d] \n", __FUNCTION__, __LINE__);

#define DRV_SCLHVSP_IO_UNLOCK_MUTEX(x)  \
        SCL_ERR("--- [MUTEX_LOCK][%s]   [%d] \n", __FUNCTION__, __LINE__); \
        DrvSclOsReleaseMutex(x);



#else
#define DRV_SCLVIP_IO_LOCK_MUTEX(x)    DrvSclOsObtainMutex(x, SCLOS_WAIT_FOREVER)
#define DRV_SCLVIP_IO_UNLOCK_MUTEX(x)  DrvSclOsReleaseMutex(x)

#endif

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    s32 s32Id;
    MDrvSclCtxLockConfig_t *pLockCfg;
    MDrvSclCtxConfig_t  *pCmdqCtx;
    MDrvSclCtxIdType_e enCtxId;
}DrvSclVipIoCtxConfig_t;

typedef struct
{
    s32 s32Handle;
    DrvSclVipIoCtxConfig_t stCtxCfg;
} DrvSclVipHandleConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvSclVipHandleConfig_t _gstSclVipHandler[DRV_SCLVIP_HANDLER_MAX];
s32 _s32SclVipIoHandleMutex = -1;
DrvSclVipIoFunctionConfig_t _gstSclVipIoFunc;
MDrvSclCtxNrBufferGlobalSet_t *gstNrGlobalSet;
char KEY_DMEM_VIP_MCNR_YC[20] = "VIP_MCNR_YC";
void *gpvNrBuffer=NULL;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

void _DrvSclVipIoSetGlobal(MDrvSclCtxCmdqConfig_t *pvCtx)
{
    gstNrGlobalSet = &(((MDrvSclCtxGlobalSet_t*)(pvCtx->pgstGlobalSet))->stNrBufferCfg);
}
void _DrvSclVipIoMemNaming(DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg)
{
    gstNrGlobalSet->u8McnrYCMName[0] = 48+(((pstSclVipCtxCfg->s32Id&0xFFF)%1000)/100);
    gstNrGlobalSet->u8McnrYCMName[1] = 48+(((pstSclVipCtxCfg->s32Id&0xFFF)%100)/10);
    gstNrGlobalSet->u8McnrYCMName[2] = 48+(((pstSclVipCtxCfg->s32Id&0xFFF)%10));
    gstNrGlobalSet->u8McnrYCMName[3] = '_';
    gstNrGlobalSet->u8McnrYCMName[4] = '\0';
    DrvSclOsStrcat(gstNrGlobalSet->u8McnrYCMName,KEY_DMEM_VIP_MCNR_YC);
}
bool  _DrvSclVipIoCheckModifyMemSize(DrvSclVipIoReqMemConfig_t  *stReqMemCfg)
{
    if( (stReqMemCfg->u16Vsize & (15)) || (stReqMemCfg->u16Pitch & (15)))
    {
        SCL_ERR(
            "[HVSP] Size must be align 16, Vsize=%d, Pitch=%d\n", stReqMemCfg->u16Vsize, stReqMemCfg->u16Pitch);
        if((stReqMemCfg->u16Pitch & (15)) && (stReqMemCfg->u16Vsize & (15)))
        {
            stReqMemCfg->u32MemSize = (((stReqMemCfg->u16Vsize / 16) + 1) * 16) * ((stReqMemCfg->u16Pitch / 16) + 1) * 16 * 4;
        }
        else if(stReqMemCfg->u16Pitch & (15))
        {
            stReqMemCfg->u32MemSize = (stReqMemCfg->u16Vsize) * ((stReqMemCfg->u16Pitch / 16) + 1) * 16 * 4;
        }
        else if(stReqMemCfg->u16Vsize & (15))
        {
            stReqMemCfg->u32MemSize = (((stReqMemCfg->u16Vsize / 16) + 1) * 16) * stReqMemCfg->u16Pitch * 4;
        }
    }
    else
    {
        stReqMemCfg->u32MemSize = (stReqMemCfg->u16Vsize) * (stReqMemCfg->u16Pitch) * 4;
    }
    if((DrvSclOsGetSclFrameBufferNum()) == 1)
    {
        stReqMemCfg->u32MemSize = stReqMemCfg->u32MemSize / 2 ;
    }
    gstNrGlobalSet->u32McnrSize = stReqMemCfg->u32MemSize;
    if((DrvSclOsGetSclFrameBufferNum()) == 1)
    {
        if((u32)(stReqMemCfg->u16Vsize * stReqMemCfg->u16Pitch * 2) > stReqMemCfg->u32MemSize)
        {
            SCL_ERR( "[VIP] Memory size is too small, Vsize*Pitch*2=%lx, MemSize=%lx\n",
                     (u32)(stReqMemCfg->u16Vsize * stReqMemCfg->u16Pitch * 2), stReqMemCfg->u32MemSize);
            return FALSE;
        }
    }
    else if((u32)(stReqMemCfg->u16Vsize * stReqMemCfg->u16Pitch * 4) > stReqMemCfg->u32MemSize)
    {
        SCL_ERR( "[VIP] Memory size is too small, Vsize*Pitch*4=%lx, MemSize=%lx\n",
                 (u32)(stReqMemCfg->u16Vsize * stReqMemCfg->u16Pitch * 4), stReqMemCfg->u32MemSize);
        return FALSE;
    }
    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_NORMAL,
            "[VIP], Vsize=%d, Pitch=%d, Size=%lx\n", stReqMemCfg->u16Vsize, stReqMemCfg->u16Pitch, stReqMemCfg->u32MemSize);
    return TRUE;
}
void* _DrvSclVipIoAllocDmem(char* name, u32 size, DrvSclOsDmemBusType_t *addr)
{
    void *pDmem = NULL;;

    pDmem = DrvSclOsDirectMemAlloc(name, size, addr);

    if(pDmem)
    {
        gstNrGlobalSet->u32McnrReleaseSize = gstNrGlobalSet->u32McnrSize;
    }

    return pDmem;
}
void _DrvSclVipIoFreeDmem(const char* name, unsigned int size, void *virt, u32 addr)
{
    DrvSclOsDirectMemFree(name, size, virt, addr);
}
void _DrvSclVipIoMemFreeYCMbuffer(void)
{
    if(gstNrGlobalSet->pvMcnrYCMVirAddr != 0)
    {
        SCL_ERR("[VIP] YC free %lx\n",(u32)gstNrGlobalSet->pvMcnrYCMVirAddr);
        SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_NORMAL, "[VIP] YC free\n");
        _DrvSclVipIoFreeDmem(gstNrGlobalSet->u8McnrYCMName,
                            PAGE_ALIGN(gstNrGlobalSet->u32McnrSize),
                            gstNrGlobalSet->pvMcnrYCMVirAddr,
                            gstNrGlobalSet->PhyMcnrYCMAddr);

        gstNrGlobalSet->pvMcnrYCMVirAddr = 0;
        gstNrGlobalSet->PhyMcnrYCMAddr = 0;
        DrvSclOsSetSclFrameBufferAlloced(DrvSclOsGetSclFrameBufferAlloced()&(~E_DRV_SCLOS_FBALLOCED_YCM));
    }
}
bool _DrvSclVipIoMemAllocate(void)
{

    SCL_DBG(SCL_DBG_LV_VIP()&EN_DBGMG_VIPLEVEL_NORMAL, "[VIP] allocate memory\n");
    if (!(gstNrGlobalSet->pvMcnrYCMVirAddr = _DrvSclVipIoAllocDmem(gstNrGlobalSet->u8McnrYCMName,
                                      PAGE_ALIGN(gstNrGlobalSet->u32McnrSize),
                                      &gstNrGlobalSet->PhyMcnrYCMAddr)))
    {
        SCL_ERR( "%s: unable to allocate YC memory %lx\n", __FUNCTION__,(u32)gstNrGlobalSet->PhyMcnrYCMAddr);
        return 0;
    }
    else
    {
        DrvSclOsSetSclFrameBufferAlloced((DrvSclOsGetSclFrameBufferAlloced()|E_DRV_SCLOS_FBALLOCED_YCM));
    }

    SCL_ERR( "[VIP]: MCNR YC: Phy:%x  Vir:%lx\n", gstNrGlobalSet->PhyMcnrYCMAddr, (u32)gstNrGlobalSet->pvMcnrYCMVirAddr);

    return 1;
}
void _DrvSclVipFrameBufferMemoryAllocate(void)
{
    if(DrvSclOsGetSclFrameBufferAlloced() == 0)
    {
        _DrvSclVipIoMemAllocate();
    }
    else if(DrvSclOsGetSclFrameBufferAlloced() != 0 &&
        gstNrGlobalSet->u32McnrSize > gstNrGlobalSet->u32McnrReleaseSize)
    {
        _DrvSclVipIoMemFreeYCMbuffer();
        DrvSclOsSetSclFrameBufferAlloced(E_DRV_SCLOS_FBALLOCED_NON);
        _DrvSclVipIoMemAllocate();
    }
}
void _DrvSclVipIoFillIPMStructForDriver(DrvSclVipIoReqMemConfig_t *pstReqMemCfg,MDrvSclVipIpmConfig_t *stIPMCfg)
{
    stIPMCfg->u16Height = pstReqMemCfg->u16Vsize;
    stIPMCfg->u16Width  = pstReqMemCfg->u16Pitch;
    stIPMCfg->u32MemSize = pstReqMemCfg->u32MemSize;
    stIPMCfg->enCeType = pstReqMemCfg->enCeType;
    stIPMCfg->enRW       = E_MDRV_SCLVIP_MCNR_NON;
    gstNrGlobalSet->u32McnrSize   = pstReqMemCfg->u32MemSize;
    if(gstNrGlobalSet->PhyMcnrYCMAddr)
    {
        stIPMCfg->u32YCMPhyAddr = (gstNrGlobalSet->PhyMcnrYCMAddr);
    }
}
#if defined(SCLOS_TYPE_LINUX_DEBUG)
void _DrvSclVipIoLogConfigStruct(MDrvSclVipSetPqConfig_t *stSetPQCfg)
{
    u8 u8offset;
    u8 *pu8value = NULL;
    for(u8offset = 0;u8offset < stSetPQCfg->u32StructSize;u8offset++)
    {
        pu8value = stSetPQCfg->pPointToCfg + u8offset*1;
        SCL_DBG(SCL_DBG_LV_VIP_LOG(), "[VIP] offset%hhd:%hhx \n",u8offset,*pu8value);

    }
}
#endif
void _DrvSclVipIoLogConfigByIP(DrvSclVipIoConfigType_e enVIPtype,u8 *pPointToCfg)
{
#if defined(SCLOS_TYPE_LINUX_DEBUG)
    MDrvSclVipSetPqConfig_t stSetPQCfg;
    DrvSclOsMemset(&stSetPQCfg,0,sizeof(MDrvSclVipSetPqConfig_t));
    MDrvSclVipFillBasicStructSetPqCfg(enVIPtype,pPointToCfg,&stSetPQCfg);
    _DrvSclVipIoLogConfigStruct(&stSetPQCfg);
#endif
}

void _DrvSclVipIoFillVersionChkStruct
(u32 u32StructSize, u32 u32VersionSize, u32 *pVersion,MDrvSclVipVersionChkConfig_t *stVersion)
{
    stVersion->u32StructSize  = (u32)u32StructSize;
    stVersion->u32VersionSize = (u32)u32VersionSize;
    stVersion->pVersion      = (u32 *)pVersion;
}

s32 _DrvSclVipIoVersionCheck(MDrvSclVipVersionChkConfig_t *stVersion)
{
    if ( CHK_VERCHK_HEADER(stVersion->pVersion) )
    {
        if( CHK_VERCHK_MAJORVERSION_LESS( stVersion->pVersion, DRV_SCLVIP_VERSION) )
        {

            VERCHK_ERR("[VIP] Version(%04lx) < %04x!!! \n",
                *(stVersion->pVersion) & VERCHK_VERSION_MASK,
                DRV_SCLVIP_VERSION);

            return -1;
        }
        else
        {
            if( CHK_VERCHK_SIZE( &stVersion->u32VersionSize, stVersion->u32StructSize) == 0 )
            {
                VERCHK_ERR("[VIP] Size(%04lx) != %04lx!!! \n",
                    stVersion->u32StructSize,
                    stVersion->u32VersionSize);

                return -1;
            }
            else
            {
                SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_ELSE, "[VIP] Size(%ld) \n",stVersion->u32StructSize );
                return VersionCheckSuccess;
            }
        }
    }
    else
    {
        VERCHK_ERR("[VIP] No Header !!! \n");
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
}


u8 _DrvSclVipIoGetIdOpenTime(DrvSclVipIoIdType_e enVipId)
{
    s16 i = 0;
    u8 u8Cnt = 0;
    for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
    {
        if( _gstSclVipHandler[i].s32Handle != -1)
        {
            u8Cnt ++;
        }
    }
    return u8Cnt;
}
DrvSclVipIoCtxConfig_t *_DrvSclVipIoGetCtxConfig(s32 s32Handler)
{
    s16 i;
    s16 s16Idx = -1;
    DrvSclVipIoCtxConfig_t * pCtxCfg;

    //DRV_SCLVIP_IO_LOCK_MUTEX(_s32SclVipIoHandleMutex);

    for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
    {
        if(_gstSclVipHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        pCtxCfg = NULL;
    }
    else
    {
        pCtxCfg = &_gstSclVipHandler[s16Idx].stCtxCfg;
    }

    //DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);
    return pCtxCfg;
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void _DrvSclVipIoKeepCmdqFunction(DrvSclOsCmdqInterface_t *pstCmdq)
{
    if(pstCmdq)
    {
        MDrvSclCtxKeepCmdqFunction(pstCmdq);
    }
}
bool _DrvSclVipIoDeInit(void)
{
    DrvSclOsClearProbeInformation(E_DRV_SCLOS_INIT_VIP);
    if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_ALL) == 0)
    {
        MDrvSclVipDelete(1);
    }
    else
    {
        MDrvSclVipDelete(0);
    }
    DrvSclOsMemFree(gpvNrBuffer);
    gpvNrBuffer =NULL;
    if(_s32SclVipIoHandleMutex != -1)
    {
         DrvSclOsDeleteMutex(_s32SclVipIoHandleMutex);
         _s32SclVipIoHandleMutex = -1;
    }
    return TRUE;
}
bool _DrvSclVipIoInit(void)
{
    u16 i, start, end;
    MDrvSclVipInitConfig_t stVipInitCfg;
    MDrvSclCtxConfig_t *pvCfg;
    DrvSclOsMemset(&stVipInitCfg,0,sizeof(MDrvSclVipInitConfig_t));
    if(_s32SclVipIoHandleMutex == -1)
    {
        _s32SclVipIoHandleMutex = DrvSclOsCreateMutex(E_DRV_SCLOS_FIFO, "SCLVIP_IO", SCLOS_PROCESS_SHARED);
        if(_s32SclVipIoHandleMutex == -1)
        {
            SCL_ERR("%s %d, Create Mutex Fail\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
    }

    start = 0;
    end = start + DRV_SCLVIP_HANDLER_MAX;

    for(i = start; i < end; i++)
    {
        _gstSclVipHandler[i].s32Handle = -1;
        _gstSclVipHandler[i].stCtxCfg.s32Id = -1;
        _gstSclVipHandler[i].stCtxCfg.pLockCfg = NULL;
    }

    //Ctx Init
    if( MDrvSclCtxInit() == FALSE)
    {
        SCL_ERR("%s %d, Init Ctx\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    gpvNrBuffer = DrvSclOsMemalloc(DRV_SCLVIP_IO_NR_SIZE,GFP_KERNEL);
    DrvSclOsMemset(&_gstSclVipIoFunc, 0, sizeof(DrvSclVipIoFunctionConfig_t));
    _gstSclVipIoFunc.DrvSclVipIoSetPeakingConfig        = _DrvSclVipIoSetPeakingConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetDlcHistogramConfig       = _DrvSclVipIoSetDlcHistogramConfig;
    _gstSclVipIoFunc.DrvSclVipIoGetDlcHistogramConfig      = _DrvSclVipIoGetDlcHistogramConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetDlcConfig          = _DrvSclVipIoSetDlcConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetLceConfig         = _DrvSclVipIoSetLceConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetUvcConfig     = _DrvSclVipIoSetUvcConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetIhcConfig    = _DrvSclVipIoSetIhcConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetIccConfig       = _DrvSclVipIoSetIccConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetIhcIceAdpYConfig      = _DrvSclVipIoSetIhcIceAdpYConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetIbcConfig          = _DrvSclVipIoSetIbcConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetFccConfig      = _DrvSclVipIoSetFccConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetNlmConfig   = _DrvSclVipIoSetNlmConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetAckConfig            = _DrvSclVipIoSetAckConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetMcnrConfig         = _DrvSclVipIoSetMcnrConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetAipConfig       = _DrvSclVipIoSetAipConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetAipSramConfig    = _DrvSclVipIoSetAipSramConfig;
    _gstSclVipIoFunc.DrvSclVipIoSetVipConfig          = _DrvSclVipIoSetVipConfig;
    _gstSclVipIoFunc.DrvSclVipIoGetVersion       = _DrvSclVipIoGetVersion;
    //ToDo Init
    pvCfg = MDrvSclCtxGetDefaultCtx();
    stVipInitCfg.pvCtx = (void *)&(pvCfg->stCtx);
    if( MDrvSclVipInit(&stVipInitCfg) == 0)
    {
        return -EFAULT;
    }
    return TRUE;
}
bool _DrvSclVipIsOpend(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;

    for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
    {
        if(_gstSclVipHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        return false;
    }

    return true;
}
s32 _DrvSclVipIoOpen(DrvSclVipIoIdType_e enSclVipId)
{
    MDrvSclCtxIdType_e enCtxId;
    s32 s32Handle = -1;
    s16 s16Idx = -1;
    s16 i ;
    enCtxId = E_MDRV_SCL_CTX_ID_SC_VIP;
    DRV_SCLVIP_IO_LOCK_MUTEX(_s32SclVipIoHandleMutex);
    for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
    {
        if(_gstSclVipHandler[i].s32Handle == -1)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        s32Handle = -1;
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
    }
    else
    {
        s32Handle = s16Idx | DRV_SCLVIP_HANDLER_PRE_FIX ;
        _gstSclVipHandler[s16Idx].s32Handle = s32Handle ;
        _gstSclVipHandler[s16Idx].stCtxCfg.enCtxId= enCtxId;
        _gstSclVipHandler[s16Idx].stCtxCfg.s32Id = s32Handle;
        _gstSclVipHandler[s16Idx].stCtxCfg.pLockCfg = MDrvSclCtxGetLockConfig(enCtxId);
    }
    DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);

    return s32Handle;
}



DrvSclVipIoErrType_e _DrvSclVipIoRelease(s32 s32Handler)
{
    s16 s16Idx = -1;
    s16 i ;
    u16 u16loop = 0;
    DrvSclVipIoErrType_e eRet = TRUE;
    DRV_SCLVIP_IO_LOCK_MUTEX(_s32SclVipIoHandleMutex);
    for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
    {
        if(_gstSclVipHandler[i].s32Handle == s32Handler)
        {
            s16Idx = i;
            break;
        }
    }

    if(s16Idx == -1)
    {
        SCL_ERR( "[HVSP]   %s %d \n", __FUNCTION__, __LINE__);
        eRet = E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    else
    {
        //ToDo Free Ctx

        _gstSclVipHandler[s16Idx].s32Handle = -1;
        _gstSclVipHandler[s16Idx].stCtxCfg.s32Id = -1;
        _gstSclVipHandler[s16Idx].stCtxCfg.pLockCfg = NULL;
        for(i = 0; i < DRV_SCLVIP_HANDLER_MAX; i++)
        {
            if( _gstSclVipHandler[i].s32Handle != -1)
            {
                u16loop = 1;
                break;
            }
        }
        if(!u16loop)
        {
            MDrvSclVipReSetHw();
        }
        eRet = E_DRV_SCLVIP_IO_ERR_OK;
    }
    DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);

    return eRet;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetPeakingConfig(s32 s32Handler, DrvSclVipIoPeakingConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoPeakingConfig_t),
                                              (pstCfg->VerChk_Size),
                                              &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_PEAKING_CONFIG, (u8 *)pstCfg);

    if(!MDrvSclVipSetPeakingConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoDlcHistogramConfig_t),
                                              (pstCfg->VerChk_Size),
                                              &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_DLC_HISTOGRAM_CONFIG, (u8 *)pstCfg);
    if(!MDrvSclVipSetHistogramConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoGetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramReport_t *pstCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    if(!MDrvSclVipGetDlcHistogramReport((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetDlcConfig(s32 s32Handler, DrvSclVipIoDlcConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoDlcConfig_t),
                                              (pstCfg->VerChk_Size),
                                              &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_DLC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetDlcConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e  _DrvSclVipIoSetLceConfig(s32 s32Handler, DrvSclVipIoLceConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoLceConfig_t),
                                              (pstCfg->VerChk_Size),
                                              &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);

    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_LCE_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetLceConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetUvcConfig(s32 s32Handler, DrvSclVipIoUvcConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoUvcConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_UVC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetUvcConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetIhcConfig(s32 s32Handler, DrvSclVipIoIhcConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoIhcConfig_t),
                                              (pstCfg->VerChk_Size),
                                              &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_IHC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetIhcConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetIccConfig(s32 s32Handler, DrvSclVipIoIccConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoIccConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_ICC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetICEConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetIhcIceAdpYConfig(s32 s32Handler, DrvSclVipIoIhcIccConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoIhcIccConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_IHCICC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetIhcICCADPYConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetIbcConfig(s32 s32Handler, DrvSclVipIoIbcConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoIbcConfig_t),
        (pstCfg->VerChk_Size),
        &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_IBC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetIbcConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetFccConfig(s32 s32Handler, DrvSclVipIoFccConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoFccConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_FCC_CONFIG,(u8 *)pstCfg);
    if(!MDrvSclVipSetFccConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetNlmConfig(s32 s32Handler, DrvSclVipIoNlmConfig_t *pstIoCfg)
{
    MDrvSclVipNlmConfig_t stCfg;
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipNlmConfig_t));
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoNlmConfig_t),
                                              (pstIoCfg->VerChk_Size),
                                              &(pstIoCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    DrvSclOsMemcpy(&stCfg, pstIoCfg, sizeof(MDrvSclVipNlmConfig_t));

    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_NLM_CONFIG,(u8 *)pstIoCfg);
    if(!MDrvSclVipSetNlmConfig((void *)&stCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }

    if(stCfg.stSRAM.bEn)
    {
        MDrvSclVipSetNlmSramConfig(stCfg.stSRAM);
    }
    else if(!stCfg.stSRAM.bEn && stCfg.stSRAM.u32viradr)
    {
        MDrvSclVipSetNlmSramConfig(stCfg.stSRAM);
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetAckConfig(s32 s32Handler, DrvSclVipIoAckConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoAckConfig_t),
                                               (pstCfg->VerChk_Size),
                                               &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_ACK_CONFIG,(u8 *)pstCfg);

    if(!MDrvSclVipSetAckConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetMcnrConfig(s32 s32Handler, DrvSclVipIoMcnrConfig_t *pstIoCfg)
{
    MDrvSclVipMcnrConfig_t stCfg;
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipMcnrConfig_t));
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoMcnrConfig_t),
                                               (pstIoCfg->VerChk_Size),
                                               &(pstIoCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    else
    {
        stCfg.u32Viraddr = pstIoCfg->u32Viraddr;
        stCfg.bEnMCNR = pstIoCfg->bEnMCNR;
        stCfg.bEnCIIR = pstIoCfg->bEnCIIR;
        pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
        if(pstSclVipCtxCfg == NULL)
        {
            SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
            return E_DRV_SCLVIP_IO_ERR_INVAL;
        }
        enCtxId = pstSclVipCtxCfg->enCtxId;
        MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    }

    if(!MDrvSclVipSetMcnrConfig((void *)&stCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig)
{
    MDrvSclVipAipConfig_t stCfg;
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipAipConfig_t));
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoAipConfig_t),
                                               (pstIoConfig->VerChk_Size),
                                               &(pstIoConfig->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    else
    {
        stCfg.u32Viraddr = pstIoConfig->u32Viraddr;
        stCfg.u16AIPType = (u16)pstIoConfig->enAIPType;
        pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
        if(pstSclVipCtxCfg == NULL)
        {
            SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
            return E_DRV_SCLVIP_IO_ERR_INVAL;
        }
        enCtxId = pstSclVipCtxCfg->enCtxId;
        MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    }

    SCL_DBG(SCL_DBG_LV_IOCTL()&EN_DBGMG_IOCTLEVEL_VIP, "[VIP] IOCTL_NUM:: == %s ==  \n", (AIP_PARSING(stCfg.u16AIPType)));
    //if(pstIoConfig->enAIPType != E_DRV_SCLVIP_IO_AIP_FORVPEPROCESS)
    {
        if(!MDrvSclVipSetAipConfig(&stCfg))
        {
            MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
            return E_DRV_SCLVIP_IO_ERR_FAULT;
        }
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoSetFlip(s32 s32Handler)
{
    MDrvSclCtxIdType_e enCtxId;
    MDrvSclCtxCmdqConfig_t *pvCtx;
    bool bWaitMload = 0;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    //wdr buffer handler
    bWaitMload = MDrvSclVipSetWdrMloadConfig();
    //context switch
    pvCtx = MDrvSclCtxGetConfigCtx(enCtxId);
    MDrvSclCtxFire(pvCtx);
    if(bWaitMload)
    {
        MDrvSclCtxSetMload(E_MDRV_SCL_CTX_MLOAD_ID_WDR,pvCtx);
        MDrvSclCtxFireMload(bWaitMload,pvCtx);
    }
    // add poll mload down
    //if(bWaitMload)
    //{
    //    MDrvSclCtxSetWaitMload();
    //}
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
{
    MDrvSclVipAipSramConfig_t stCfg;
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipAipSramConfig_t));
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoAipSramConfig_t),
        (pstIoCfg->VerChk_Size),
        &(pstIoCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    else
    {
        stCfg.u32Viraddr = pstIoCfg->u32Viraddr;
        stCfg.enAIPType = pstIoCfg->enAIPType;
        pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
        if(pstSclVipCtxCfg == NULL)
        {
            SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
            return E_DRV_SCLVIP_IO_ERR_INVAL;
        }
        enCtxId = pstSclVipCtxCfg->enCtxId;
        MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    }

    if(!MDrvSclVipSetAipSramConfig(&stCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}


DrvSclVipIoErrType_e _DrvSclVipIoSetVipConfig(s32 s32Handler, DrvSclVipIoConfig_t *pstCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoConfig_t),
                                                      (pstCfg->VerChk_Size),
                                                      &(pstCfg->VerChk_Version),&stVersion);
    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    _DrvSclVipIoLogConfigByIP(E_DRV_SCLVIP_IO_CONFIG,(u8 *)pstCfg);

    if(!MDrvSclVipSetVipOtherConfig((void *)pstCfg))
    {
        MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoGetPrivateIdConfig(s32 s32Handler, DrvSclVipIoPrivateIdConfig_t *pstIOCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;

    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);

    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    pstIOCfg->s32Id = s32Handler;

    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoSetRoiConfig(s32 s32Handler, DrvSclVipIoWdrRoiHist_t *pstCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    MDrvSclVipWdrRoiHist_t stCfg;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipWdrRoiHist_t));
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    DrvSclOsMemcpy(&stCfg,pstCfg,sizeof(MDrvSclVipWdrRoiHist_t));
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    MDrvSclVipSetRoiConfig((void *)&stCfg);
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoSetMaskOnOff(s32 s32Handler, DrvSclVipIoSetMaskOnOff_t *pstCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    MDrvSclVipSetMaskOnOff_t stCfg;
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipSetMaskOnOff_t));
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    DrvSclOsMemcpy(&stCfg,pstCfg,sizeof(MDrvSclVipSetMaskOnOff_t));
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    MDrvSclVipSetMaskOnOff((void *)&stCfg);
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoReqmemConfig(s32 s32Handler, DrvSclVipIoReqMemConfig_t*pstReqMemCfg)
{
    MDrvSclVipIpmConfig_t stIPMCfg;
    DrvSclVipIoErrType_e eRet = E_DRV_SCLVIP_IO_ERR_OK;
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    void *pvCtx;
    if(pstReqMemCfg->u32MemSize == 0)
    {
        return E_DRV_SCLVIP_IO_NOT_ALLOCATE;
    }
    DrvSclOsMemset(&stIPMCfg,0,sizeof(MDrvSclVipIpmConfig_t));
     _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoReqMemConfig_t),
                 pstReqMemCfg->VerChk_Size,
                 &pstReqMemCfg->VerChk_Version,&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[VIP]   %s %d \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);

    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    if(pstReqMemCfg->u32MemSize == 0)
    {
        SCL_ERR( "[VIP]   %s %d,Not to allocate Nr Buffer\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    pvCtx = (void *)MDrvSclCtxGetConfigCtx(enCtxId);
    _DrvSclVipIoSetGlobal((MDrvSclCtxCmdqConfig_t*)pvCtx);
    _DrvSclVipIoMemNaming(pstSclVipCtxCfg);
    DrvSclOsSetSclFrameBufferNum(DNR_BUFFER_MODE);
    _DrvSclVipIoCheckModifyMemSize(pstReqMemCfg);
    _DrvSclVipFrameBufferMemoryAllocate();
    _DrvSclVipIoFillIPMStructForDriver(pstReqMemCfg,&stIPMCfg);
    stIPMCfg.pvCtx = pvCtx;
    if(MDrvSclVipSetInitIpmConfig(&stIPMCfg))
    {
        eRet = E_DRV_SCLVIP_IO_ERR_OK;
    }
    else
    {
        eRet = E_DRV_SCLVIP_IO_ERR_FAULT;
    }

    if(DrvSclOsGetSclFrameBufferAlloced() == 0)
    {
        eRet = E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return eRet;
}



DrvSclVipIoErrType_e _DrvSclVipIoReqWdrMloadBuffer(s32 s32Handler)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    MDrvSclVipReqWdrMloadBuffer(pstSclVipCtxCfg->s32Id);
    MDrvSclVipSetMultiSensorConfig(1);
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoFreeWdrMloadBuffer(s32 s32Handler)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    MDrvSclVipFreeWdrMloadBuffer(pstSclVipCtxCfg->s32Id);
    MDrvSclVipSetMultiSensorConfig(0);
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoGetWdrHistogram(s32 s32Handler, DrvSclVipIoWdrRoiReport_t *pstIOCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    MDrvSclVipWdrRoiReport_t stCfg;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);
    DrvSclOsMemset(&stCfg,0,sizeof(MDrvSclVipWdrRoiReport_t));
    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    MDrvSclVipGetWdrHistogram((void *)&stCfg);
    DrvSclOsMemcpy(pstIOCfg,&stCfg,sizeof(DrvSclVipIoWdrRoiReport_t));
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoGetNRHistogram(s32 s32Handler, DrvSclVipIoNrHist_t *pstIOCfg)
{
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxIdType_e enCtxId;
    void *pvNrBuffer;
    bool bRet = E_DRV_SCLVIP_IO_ERR_OK;
    pstSclVipCtxCfg = _DrvSclVipIoGetCtxConfig(s32Handler);

    if(pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[VIP]   %s %d, Ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    enCtxId = pstSclVipCtxCfg->enCtxId;
    MDrvSclCtxSetLockConfig(s32Handler,enCtxId);
    if(gpvNrBuffer)
    {
        pvNrBuffer = gpvNrBuffer;
    }
    else
    {
        gpvNrBuffer = DrvSclOsMemalloc(DRV_SCLVIP_IO_NR_SIZE,GFP_KERNEL);
        pvNrBuffer = gpvNrBuffer;
    }
    MDrvSclVipGetNRHistogram(pvNrBuffer);
    if(DrvSclOsCopyToUser((void *)pstIOCfg->u32Viraddr, (void *)pvNrBuffer, DRV_SCLVIP_IO_NR_SIZE))
    {
        DrvSclOsMemcpy((void *)pstIOCfg->u32Viraddr, (void *)pvNrBuffer, DRV_SCLVIP_IO_NR_SIZE);
    }
    MDrvSclCtxSetUnlockConfig(s32Handler,enCtxId);
    return bRet;
}
DrvSclVipIoErrType_e _DrvSclVipIoCreateInstConfig(s32 s32Handler, DrvSclVipIoLockConfig_t *pstIoInCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;
    MDrvSclCtxInstConfig_t stCtxInst;
    DrvSclOsMemset(&stCtxInst,0,sizeof(MDrvSclCtxInstConfig_t));
    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoLockConfig_t),
                                              (pstIoInCfg->VerChk_Size),
                                              &(pstIoInCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLVIP]   %s  %d, version fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    pstSclVipCtxCfg =_DrvSclVipIoGetCtxConfig(s32Handler);

    if( pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[SCLVIP]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[SCLVIP]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    DRV_SCLVIP_IO_LOCK_MUTEX(_s32SclVipIoHandleMutex);
    //alloc Ctx handler
    stCtxInst.ps32IdBuf = pstIoInCfg->ps32IdBuf;
    stCtxInst.u8IdNum = pstIoInCfg->u8BufSize;
    pstSclVipCtxCfg->pCmdqCtx = MDrvSclCtxAllocate(E_MDRV_SCL_CTX_ID_SC_VIP,&stCtxInst);
    //alloc VIP Ctx
    if(pstSclVipCtxCfg->pCmdqCtx == NULL)
    {
        SCL_ERR("%s %d::Allocate Ctx Fail\n", __FUNCTION__, __LINE__);
        DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);

        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }



    DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);

    return E_DRV_SCLVIP_IO_ERR_OK;
}
DrvSclVipIoErrType_e _DrvSclVipIoDestroyInstConfig(s32 s32Handler, DrvSclVipIoLockConfig_t *pstIoInCfg)
{
    MDrvSclVipVersionChkConfig_t stVersion;
    DrvSclVipIoCtxConfig_t *pstSclVipCtxCfg;

    _DrvSclVipIoFillVersionChkStruct(sizeof(DrvSclVipIoLockConfig_t),
                                              (pstIoInCfg->VerChk_Size),
                                              &(pstIoInCfg->VerChk_Version),&stVersion);

    if(_DrvSclVipIoVersionCheck(&stVersion))
    {
        SCL_ERR( "[SCLVIP]   %s  %d, version fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    pstSclVipCtxCfg =_DrvSclVipIoGetCtxConfig(s32Handler);

    if( pstSclVipCtxCfg == NULL)
    {
        SCL_ERR( "[SCLVIP]   %s  %d, ctx fail\n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    if(pstIoInCfg->u8BufSize == 0 || pstIoInCfg->ps32IdBuf == NULL || pstIoInCfg->u8BufSize > MDRV_SCL_CTX_CLIENT_ID_MAX)
    {
        SCL_ERR( "[SCLVIP]   %s  %d, \n", __FUNCTION__, __LINE__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    DRV_SCLVIP_IO_LOCK_MUTEX(_s32SclVipIoHandleMutex);
    //free Ctx handler
    MDrvSclCtxSetLockConfig(s32Handler,pstSclVipCtxCfg->pCmdqCtx->stCtx.enCtxId);
    _DrvSclVipIoSetGlobal((MDrvSclCtxCmdqConfig_t*)&(pstSclVipCtxCfg->pCmdqCtx->stCtx));
    _DrvSclVipIoMemFreeYCMbuffer();
    MDrvSclCtxSetUnlockConfig(s32Handler,pstSclVipCtxCfg->pCmdqCtx->stCtx.enCtxId);
    MDrvSclCtxFree(pstSclVipCtxCfg->pCmdqCtx);
    //free VIP Ctx
    pstSclVipCtxCfg->pCmdqCtx = NULL;

    DRV_SCLVIP_IO_UNLOCK_MUTEX(_s32SclVipIoHandleMutex);

    return E_DRV_SCLVIP_IO_ERR_OK;
}

DrvSclVipIoErrType_e _DrvSclVipIoGetVersion(s32 s32Handler, DrvSclVipIoVersionConfig_t *pstCfg)
{
    DrvSclVipIoErrType_e ret = E_DRV_SCLVIP_IO_ERR_OK;
    if (CHK_VERCHK_HEADER( &(pstCfg->VerChk_Version)) )
    {
        if( CHK_VERCHK_MAJORVERSION_LESS( &(pstCfg->VerChk_Version), DRV_SCLVIP_VERSION) )
        {

            VERCHK_ERR("[VIP] Version(%04lx) < %04x!!! \n",
                pstCfg->VerChk_Version & VERCHK_VERSION_MASK,
                DRV_SCLVIP_VERSION);

            ret = E_DRV_SCLVIP_IO_ERR_INVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE( &(pstCfg->VerChk_Size), sizeof(DrvSclVipIoVersionConfig_t)) == 0 )
            {
                VERCHK_ERR("[VIP] Size(%04x) != %04lx!!! \n",
                    sizeof(DrvSclVipIoVersionConfig_t),
                    (pstCfg->VerChk_Size));

                ret = E_DRV_SCLVIP_IO_ERR_INVAL;
            }
            else
            {
                DrvSclVipIoVersionConfig_t stCfg;

                stCfg = FILL_VERCHK_TYPE(stCfg, stCfg.VerChk_Version, stCfg.VerChk_Size, DRV_SCLVIP_VERSION);
                stCfg.u32Version = DRV_SCLVIP_VERSION;
                DrvSclOsMemcpy(pstCfg, &stCfg, sizeof(DrvSclVipIoVersionConfig_t));
                ret = E_DRV_SCLVIP_IO_ERR_OK;
            }
        }
    }
    else
    {
        VERCHK_ERR("[VIP] No Header !!! \n");
        SCL_ERR( "[VIP]   %s  %d\n", __FUNCTION__, __LINE__);
        ret = E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    return ret;
}
