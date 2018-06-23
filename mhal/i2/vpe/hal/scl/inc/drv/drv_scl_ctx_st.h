////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
#ifndef __DRV_SC_CTX_ST_H__
#define __DRV_SC_CTX_ST_H__
#include "drv_scl_os.h"
#include "drv_scl_vip_m_st.h"
#include "drv_scl_vip.h"
#include "drv_scl_dma_m.h"
#include "drv_scl_hvsp_m.h"
#include "drv_scl_hvsp_st.h"
#include "drv_scl_dma_st.h"
#include "drv_scl_irq_st.h"
#include "hal_scl_hvsp.h"
typedef struct
{
    //hvspwrapper
    MDrvSclCtxNrBufferGlobalSet_t stNrBufferCfg;
    //dmawrapper
    //vipwrapper
    //pnlwrapper
    //mhvsp
    MDrvSclCtxMhvspGlobalSet_t stMhvspCfg;
    //mdma
    //mvip
    MDrvSclCtxMvipGlobalSet_t stMvipCfg;
    //mpnl
    //hvsp
    MDrvSclCtxHvspGlobalSet_t stHvspCfg;
    //dma
    //vip
    //irq
    //cmdq
    //MDrvSclCtxCmdqGlobalSet_t stCmdqCfg; //nonuse
    //pnl
    //msos
    MDrvSclCtxSclOsGlobalSet_t stSclOsCfg;
    //halhvsp
    MDrvSclCtxHalHvspGlobalSet_t stHalHvspCfg;
    //halscldma
    //halvip
    //halsclirq
    //MDrvSclCtxHalSclIrqGlobalSet_t stHalSclIrqCfg;//only for ISR
    //halcmdq
    //halpnl
    //extern
    DrvSclDmaInfoType_t  stScldmaInfo; //extern
}MDrvSclCtxGlobalSet_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------


#endif
