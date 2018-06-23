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
#include "drv_scl_os.h"
#include "drv_scl_dbg.h"
#include "hal_scl_util.h"

#include "drv_scl_hvsp_io_st.h"
#include "drv_scl_hvsp_io_wrapper.h"
#include "drv_scl_dma_io_st.h"
#include "drv_scl_dma_io_wrapper.h"
#include "drv_scl_vip_io_st.h"
#include "drv_scl_vip_io_wrapper.h"
#include "drv_scl_verchk.h"
#include "drv_scl_vip_io.h"


/*Kernel VIP API function */
s32 DrvSclVipIoOpen(DrvSclVipIoIdType_e enSlcVipId)
{
    s32 s32Handler;
    if(DrvSclOsGetProbeInformation(E_DRV_SCLOS_INIT_VIP) == 0)
    {
        _DrvSclVipIoInit();
        DrvSclOsSetProbeInformation(E_DRV_SCLOS_INIT_VIP);
    }
    s32Handler = _DrvSclVipIoOpen(enSlcVipId);
    if(!_DrvSclVipIoCreateIqInst(&s32Handler))
    {
        SCL_ERR("[SCLMOD]%s Create Inst Fail\n",__FUNCTION__);
        return -1;
    }
    return s32Handler;
}

DrvSclVipIoErrType_e DrvSclVipIoRelease(s32 s32Handler)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }

    if(!_DrvSclVipIoDestroyIqInst(&s32Handler))
    {
        SCL_ERR("[SCLMOD]%s Destroy Inst Fail\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_FAULT;
    }
    return _DrvSclVipIoRelease(s32Handler);
}

DrvSclVipIoErrType_e DrvSclVipIoSetPeakingConfig(s32 s32Handler, DrvSclVipIoPeakingConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetPeakingConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetDlcHistogramConfig(s32Handler, pstCfg);
}
DrvSclVipIoErrType_e DrvSclVipIoGetDlcHistogramConfig(s32 s32Handler, DrvSclVipIoDlcHistogramReport_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetDlcHistogramConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetDlcConfig(s32 s32Handler, DrvSclVipIoDlcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetDlcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e  DrvSclVipIoSetLceConfig(s32 s32Handler,DrvSclVipIoLceConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetLceConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetUvcConfig(s32 s32Handler, DrvSclVipIoUvcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetUvcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIhcConfig(s32 s32Handler,DrvSclVipIoIhcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIhcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIccConfig(s32 s32Handler, DrvSclVipIoIccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIccConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIhcIceAdpYConfig(s32 s32Handler,DrvSclVipIoIhcIccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIhcIceAdpYConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetIbcConfig(s32 s32Handler,DrvSclVipIoIbcConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetIbcConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetFccConfig(s32 s32Handler,DrvSclVipIoFccConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetFccConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetNlmConfig(s32 s32Handler,DrvSclVipIoNlmConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetNlmConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAckConfig(s32 s32Handler, DrvSclVipIoAckConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAckConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAipConfig(s32 s32Handler, DrvSclVipIoAipConfig_t *pstIoConfig)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAipConfig(s32Handler, pstIoConfig);
}

DrvSclVipIoErrType_e DrvSclVipIoSetAipSramConfig(s32 s32Handler, DrvSclVipIoAipSramConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetAipSramConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetVipConfig(s32 s32Handler,DrvSclVipIoConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetVipConfig(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiHist_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetRoiConfig(s32Handler,pstCfg);
    return eErrType;
}

DrvSclVipIoErrType_e DrvSclVipIoGetWdrRoiHistConfig(s32 s32Handler,DrvSclVipIoWdrRoiReport_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoGetWdrHistogram(s32Handler,pstCfg);
    return eErrType;
}

DrvSclVipIoErrType_e DrvSclVipIoSetWdrRoiMask(s32 s32Handler,DrvSclVipIoSetMaskOnOff_t *pstCfg)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetMaskOnOff(s32Handler, pstCfg);
    return eErrType;
}
DrvSclVipIoErrType_e DrvSclVipIoSetMcnrConfig(s32 s32Handler, DrvSclVipIoMcnrConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoSetMcnrConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoReqmemConfig(s32 s32Handler, DrvSclVipIoReqMemConfig_t *pstIoCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoReqmemConfig(s32Handler, pstIoCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetVersion(s32 s32Handler,DrvSclVipIoVersionConfig_t *pstCfg)
{
    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetVersion(s32Handler, pstCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetNRHistogram(s32 s32Handler, DrvSclVipIoNrHist_t *pstIOCfg)
{
	if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetNRHistogram(s32Handler, pstIOCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoGetPrivateIdConfig(s32 s32Handler, DrvSclVipIoPrivateIdConfig_t *pstIOCfg)
{
	if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    return _DrvSclVipIoGetPrivateIdConfig(s32Handler, pstIOCfg);
}

DrvSclVipIoErrType_e DrvSclVipIoctlInstFlip(s32 s32Handler)
{
    DrvSclVipIoErrType_e eErrType;

    if(!_DrvSclVipIsOpend(s32Handler))
    {
        SCL_ERR("[SCL]%s Invalid handle\n",__FUNCTION__);
        return E_DRV_SCLVIP_IO_ERR_INVAL;
    }
    eErrType = _DrvSclVipIoSetFlip(s32Handler);
    return eErrType;
}
