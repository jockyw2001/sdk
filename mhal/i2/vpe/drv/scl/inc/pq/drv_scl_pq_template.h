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

#include "hal_utility.h"
#ifndef _DRVPQ_TEMPLATE_H_
#define _DRVPQ_TEMPLATE_H_

#define PQ_INSTALL_FUNCTION


#ifndef UNUSED //to avoid compile warnings...
#define UNUSED(var) (void)((var) = (var))
#endif

#if(PQ_ONLY_SUPPORT_BIN == 0)
static PQTABLE_INFO _Scl_PQTableInfo;
#define MAX_IP_NUM  (PQ_IP_NUM_Main)
static u8 _u8SclPQTabIdx[MAX_IP_NUM];    // store all TabIdx of all IPs

#if (ENABLE_PQ_MLOAD)

static bool _bSclMLoadEn = FALSE;

#define MLOAD_MAX_CMD   30
static u32 _u32SclMLoadCmd[MLOAD_MAX_CMD];
static u16 _u16SclMLoadMsk[MLOAD_MAX_CMD];
static u16 _u16SclMLoadVal[MLOAD_MAX_CMD];
static u16 _u16SclMLoadCmdCnt = 0;
#endif

#if(ENABLE_PQ_LOAD_TABLE_INFO)
static MS_PQ_LOAD_TABLE_INFO g_PQLoadTableInfo;
#endif
#endif


#if (ENABLE_PQ_CMDQ)
#define CMDQ_MAX_CMD_SIZE 512
static u8 gu8debugflag = 0;
#endif


#if (PQ_ENABLE_CHECK == 0)
    #define PQ_REG_FUNC_READ( u32Reg)  MApi_XC_ReadByte(u32Reg)
    #define PQ_REG_FUNC( u32Reg, u8Value, u8Mask )   \
        do{\
        HalUtilityWBYTEMSK( u32Reg, u8Value, u8Mask );\
        }while(0)\

    #define PQ_REG_FUNC_xc( u32Reg, u8Value, u8Mask ) \
    do{ \
            if(u32Reg %2){ \
                    MApi_XC_W2BYTEMSK(u32Reg-1, ((u16)u8Value)<<8, ((u16)u8Mask)<<8); \
                }else{ \
                    MApi_XC_W2BYTEMSK(u32Reg, u8Value, u8Mask); \
                }\
        }while(0)

    #define PQ_REG_MLOAD_WRITE_CMD(u32Reg,u8Value,u8Mask) \
            do{ \
                if(u32Reg%2) \
                { \
                    _u32SclMLoadCmd[_u16SclMLoadCmdCnt] = u32Reg-1; \
                    _u16SclMLoadMsk[_u16SclMLoadCmdCnt] = ((u16)u8Mask)<<8; \
                    _u16SclMLoadVal[_u16SclMLoadCmdCnt] = ((u16)u8Value)<<8; \
                } \
                else \
                { \
                    _u32SclMLoadCmd[_u16SclMLoadCmdCnt] = u32Reg; \
                    _u16SclMLoadMsk[_u16SclMLoadCmdCnt] = ((u16)u8Mask); \
                    _u16SclMLoadVal[_u16SclMLoadCmdCnt] = ((u16)u8Value); \
                } \
                _u16SclMLoadCmdCnt++; \
                if(_u16SclMLoadCmdCnt >= MLOAD_MAX_CMD)\
                {\
                    sclprintf("[PQ ERROR] ====ML overflow !!! \r\n");\
                    _u16SclMLoadCmdCnt = MLOAD_MAX_CMD - 1;        \
                }\
            }while(0)

    #define PQ_REG_MLOAD_FUNC(u32Reg,u8Value,u8Mask) \
            do{ \
                if(u32Reg%2) \
                { \
                    MApi_XC_MLoad_WriteCmd_And_Fire(u32Reg-1, ((u16)u8Value)<<8, ((u16)u8Mask)<<8); \
                } \
                else \
                { \
                    MApi_XC_MLoad_WriteCmd_And_Fire(u32Reg, u8Value, u8Mask); \
                } \
            }while(0)

    #define PQ_SWREG_FUNC( u16Reg, u8Value, u8Mask )  SWReg_WriteByteMask( u16Reg, u8Value, u8Mask )


#else // #if(PQ_ENABLE_CHECK == 1)

static u8 _u8PQfunction = PQ_FUNC_DUMP_REG;

static void _MDrv_Scl_PQ_SetFunction(u8 u8Func)
{
    _u8PQfunction = u8Func;
}

#define PQ_REG_FUNC( u16Reg, u8Value, u8Mask ) \
    do{ \
        if (_u8PQfunction == PQ_FUNC_DUMP_REG){ \
            MApi_XC_WriteByteMask( (u32)u16Reg, u8Value, u8Mask ); \
        }else{ \
            if ((MApi_XC_ReadByte((u32)u16Reg) & u8Mask) != ((u8Value) & (u8Mask))){ \
                sclprintf("[PQRegErr] "); \
                if (((u16Reg) >> 8) == 0x2F){ \
                    sclprintf("bk=%02x, ", (u16)SC_BK_CURRENT); \
                } \
                else if (((u16Reg) >> 8) == 0x36){ \
                    sclprintf("bk=%02x, ", (u16)COMB_BK_CURRENT); \
                } \
                sclprintf("addr=%04x, mask=%02x, val=%02x[%02x]\r\n", \
                    u16Reg, (u16)u8Mask, (u16)MApi_XC_ReadByte((u32)u16Reg), (u16)u8Value); \
            } \
        } \
    }while(0)

#define PQ_SWREG_FUNC( u16Reg, u8Value, u8Mask ) \
    do{ \
        if (_u8PQfunction == PQ_FUNC_DUMP_REG){ \
            SWReg_WriteByteMask( u16Reg, u8Value, u8Mask ); \
        }else{ \
            if (SWReg[u16Reg] & (u8Mask) != (u8Value) & (u8Mask)){ \
                sclprintf("[PQSWRegErr] "); \
                sclprintf("addr=%04x, mask=%02x, val=%02x[%02x]\r\n", \
                    u16Reg, (u16)u8Mask, (u16)SWReg[u16Reg], (u16)u8Value); \
            } \
        } \
    }while(0)

#endif  //#if (PQ_ENABLE_CHECK)


#if(PQ_ONLY_SUPPORT_BIN == 0)


static void _MDrv_Scl_PQ_DumpSclaerRegTableByData(EN_IP_Info* pIP_Info, PQ_DATA_INFO *pData)
{
    u32 u32Addr;
    u8 u8Mask;
    u8 u8Value;
    u8 u8CurBank = 0xff;
    u16 u16DataIdx;

    u8CurBank = u8CurBank;

    SC_BK_STORE_MUTEX;
    SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]%s %d:: \n"
        , __FUNCTION__, __LINE__);
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums)
    {
        PQ_ERR(sclprintf("[PQ]IP_Info error: Scaler Reg Table\r\n"));

        SC_BK_RESTORE_MUTEX;

        return;
    }

    u16DataIdx = 0;
    while (1)
    {
        u32Addr = (pIP_Info->pIPTable[0]<<8) + pIP_Info->pIPTable[1];
        u8Mask  = pIP_Info->pIPTable[2];
        u8Value =  pData->pBuf[u16DataIdx];

        if (u32Addr == _END_OF_TBL_) // check end of table
        {
            break;
        }
        u8CurBank = (u8)(u32Addr >> 8);

    #if (SCALER_REGISTER_SPREAD)
        if(pIP_Info->u8TabType == PQ_TABTYPE_NE)
        {
            u32Addr = BK_NE_BASE | u32Addr;
        }
        else
        {
            u32Addr = BK_SCALER_BASE | u32Addr;
        }
    #else
        if (u8CurBank != SC_BK_CURRENT)
        {
            SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "<<bankswitch=%02x>>\r\n",u8CurBank);
            SC_BK_SWITCH(u8CurBank);
        }
        if(pIP_Info->u8TabType == PQ_TABTYPE_NE)
        {
            u32Addr = BK_NE_BASE | (u32Addr & 0x00FF);
        }
        else
        {
            u32Addr =  BK_SCALER_BASE | (u32Addr & 0x00FF);
        }
    #endif
    SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "BK =%04X, addr=%02X, msk=%02X, value=%02X\r\n"
        ,(u16)((u32Addr&0xFFFF00)>>8), (u16)(u32Addr&0xFF), (u16)u8Mask, (u16)u8Value);

    //SCL_ERR( "BK =%04X, addr=%02X, msk=%02X, value=%02X\r\n"
    //    ,(u16)((u32Addr&0xFFFF00)>>8), (u16)(u32Addr&0xFF), (u16)u8Mask, (u16)u8Value);
        PQ_REG_FUNC(u32Addr, u8Value, u8Mask);

        pIP_Info->pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabNums); // next
        u16DataIdx++;
        if(u16DataIdx > pData->u16BufSize)
        {
            sclprintf("%s %d, Data size is smaller than PQ table, idx:%d size:%d\n",
                __FUNCTION__, __LINE__, u16DataIdx, pData->u16BufSize);
            break;
        }
    }

    if(u16DataIdx != pData->u16BufSize)
    {
        sclprintf("%s %d, Data size is bigger than PQ table, idx:%d, size:%d\n",
            __FUNCTION__, __LINE__, u16DataIdx, pData->u16BufSize);
    }

    SC_BK_RESTORE_MUTEX;
}



static void _MDrv_Scl_PQ_DumpScalerRegTable(EN_IP_Info* pIP_Info)
{
    u32 u32Addr;
    u8 u8Mask;
    u8 u8Value;
    u8 u8CurBank = 0xff;

    u8CurBank = u8CurBank;

#ifdef SCLOS_TYPE_LINUX
    #if(ENABLE_PQ_MLOAD)
    pthread_mutex_lock(&_Scl_PQ_MLoad_Mutex);
    #endif
#endif

    SC_BK_STORE_MUTEX;
    PQ_DUMP_DBG(sclprintf("tab: sc\r\n"));
    if (pIP_Info->u8TabIdx >= pIP_Info->u8TabNums)
    {
        PQ_DUMP_DBG(sclprintf("[PQ]IP_Info error: Scaler Reg Table\r\n"));

        SC_BK_RESTORE_MUTEX;
#ifdef SCLOS_TYPE_LINUX
        #if(ENABLE_PQ_MLOAD)
        pthread_mutex_unlock(&_Scl_PQ_MLoad_Mutex);
        #endif
#endif

        return;
    }

    while (1)
    {
        u32Addr = (pIP_Info->pIPTable[0]<<8) + pIP_Info->pIPTable[1];
        u8Mask  = pIP_Info->pIPTable[2];
        u8Value = pIP_Info->pIPTable[REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabIdx];

        if (u32Addr == _END_OF_TBL_) // check end of table
            break;

        u8CurBank = (u8)(u32Addr >> 8);
        PQ_DUMP_DBG(sclprintf("XC bk =%x, addr=%x, msk=%x, value=%x\r\n", (u16)((u32Addr&0xFF00)>>8), (u16)(u32Addr&0xFF), (u16)u8Mask, (u16)u8Value));
#if(ENABLE_PQ_MLOAD)
        if(_bSclMLoadEn)
        {
            //sclprintf("MLad: %lx, %x, %x\r\n", u32Addr, u8Value, u8Mask);
            PQ_REG_MLOAD_WRITE_CMD(u32Addr, u8Value, u8Mask);
            //PQ_REG_MLOAD_FUNC(u32Addr, u8Value, u8Mask);
        }
        else
#endif
        {
            #if (SCALER_REGISTER_SPREAD)
            if(pIP_Info->u8TabType == PQ_TABTYPE_NE)
            {
                u32Addr = BK_NE_BASE | u32Addr;
            }
            else
            {
                u32Addr = BK_SCALER_BASE | u32Addr;
            }
            #else
            if (u8CurBank != SC_BK_CURRENT)
            {
                PQ_DUMP_DBG(sclprintf("<<bankswitch=%02x>>\r\n", u8CurBank));
                SC_BK_SWITCH(u8CurBank);
            }

            if(pIP_Info->u8TabType == PQ_TABTYPE_NE)
            {
                u32Addr = BK_NE_BASE | (u32Addr & 0x00FF);
            }
            else
            {
                u32Addr =  BK_SCALER_BASE | (u32Addr & 0x00FF);
            }
            #endif
            PQ_REG_FUNC(u32Addr, u8Value, u8Mask);
            //SCL_ERR("XC bk =%x, addr=%x, msk=%x, value=%x\r\n", (u16)((u32Addr&0xFFFF00)>>8), (u16)(u32Addr&0xFF), (u16)u8Mask, (u16)u8Value);
        }
        pIP_Info->pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+pIP_Info->u8TabNums); // next
    }


    SC_BK_RESTORE_MUTEX;

#ifdef SCLOS_TYPE_LINUX
    #if(ENABLE_PQ_MLOAD)
    pthread_mutex_unlock(&_Scl_PQ_MLoad_Mutex);
    #endif
#endif
}


static void _MDrv_Scl_PQ_DumpTable(EN_IP_Info* pIP_Info)
{
    // to save loading SRAM table time, SRAM are only downloaded
    // when current SRAM table is different to previous SRAM table
    if (pIP_Info->pIPTable == NULL)
    {
        PQ_DUMP_DBG(sclprintf("NULL Table\r\n"));
        return;
    }
    PQ_DUMP_DBG(sclprintf("Table Type =%x, Index =%x\r\n", (u16)pIP_Info->u8TabType, (u16)pIP_Info->u8TabIdx));
    switch(pIP_Info->u8TabType )
    {

        case PQ_TABTYPE_SCALER:
        case PQ_TABTYPE_NE:
            _MDrv_Scl_PQ_DumpScalerRegTable(pIP_Info);
            break;

        default:
            PQ_DUMP_DBG(sclprintf("[PQ]DumpTable:unknown type: %u\r\n", pIP_Info->u8TabType));
            break;
    }
}

// return total IP count
static u16 _MDrv_Scl_PQ_GetIPNum(void)
{
    PQ_DBG(sclprintf("[PQ]IPNum=%u\r\n",_Scl_PQTableInfo.u8PQ_IP_Num));
    return (u16)_Scl_PQTableInfo.u8PQ_IP_Num;
}

// return total table count of given IP
static u16 _MDrv_Scl_PQ_GetTableNum(u8 u8PQIPIdx)
{
    PQ_DBG(sclprintf("[PQ]TabNum=%u\r\n", _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].u8TabNums));
    return (u16)_Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].u8TabNums;
}

// return current used table index of given IP
static u16 _MDrv_Scl_PQ_GetCurrentTableIndex(u8 u8PQIPIdx)
{
    PQ_DBG(sclprintf("[PQ]CurrTableIdx=%u\r\n", _u8SclPQTabIdx[u8PQIPIdx]));
    return (u16)_u8SclPQTabIdx[u8PQIPIdx];
}

static u16 _MDrv_Scl_PQ_GetTableIndex(u16 u16PQSrcType, u8 u8PQIPIdx)
{
    if (u16PQSrcType >=_Scl_PQTableInfo.u8PQ_InputType_Num){
        PQ_DBG(sclprintf("[PQ]invalid input type\r\n"));
        return PQ_IP_NULL;
    }
    if (u8PQIPIdx >= _Scl_PQTableInfo.u8PQ_IP_Num){
        PQ_DBG(sclprintf("[PQ]invalid ip type\r\n"));
        return PQ_IP_NULL;
    }

    PQ_DBG(sclprintf("[PQ]TableIdx=%u\r\n",(u16)_Scl_PQTableInfo.pQuality_Map_Aray[u16PQSrcType * _Scl_PQTableInfo.u8PQ_IP_Num + u8PQIPIdx]));

    return (u16)_Scl_PQTableInfo.pQuality_Map_Aray[u16PQSrcType * _Scl_PQTableInfo.u8PQ_IP_Num + u8PQIPIdx];
}

static EN_IP_Info _MDrv_Scl_PQ_GetTable(u8 u8TabIdx, u8 u8PQIPIdx)
{
    EN_IP_Info ip_Info;
    _u8SclPQTabIdx[u8PQIPIdx] = u8TabIdx;
    if (u8TabIdx != PQ_IP_NULL && u8TabIdx != PQ_IP_COMM) {
        ip_Info.pIPTable  = _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].pIPTable;
        ip_Info.u8TabNums = _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].u8TabNums;
        ip_Info.u8TabType = _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].u8TabType;
        ip_Info.u8TabIdx = u8TabIdx;
    }
    else if (u8TabIdx == PQ_IP_COMM) {
        ip_Info.pIPTable = _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].pIPCommTable;
        ip_Info.u8TabNums = 1;
        ip_Info.u8TabType = _Scl_PQTableInfo.pIPTAB_Info[u8PQIPIdx].u8TabType;
        ip_Info.u8TabIdx = 0;
    }
    else {
        ip_Info.pIPTable  = 0;
        ip_Info.u8TabNums = 0;
        ip_Info.u8TabType = 0;
        ip_Info.u8TabIdx = 0;
    }
    return ip_Info;
}


static void _MDrv_Scl_PQ_Set_CmdqCfg(PQ_CMDQ_CONFIG CmdqCfg)
{
    SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]%s %d:: En=%d, \n",__FUNCTION__, __LINE__, CmdqCfg.bEnFmCnt);
}
static u16 _MDrv_Scl_PQ_GetIPRegCount(u16 u16PQSrcType,u8 u8PQIPIdx)
{
    EN_IP_Info ip_Info;
    u8 u8TabIdx;
    u32 u32Addr;
    u16 u16DataIdx = 0;

    u8TabIdx = (u8)_MDrv_Scl_PQ_GetTableIndex(u16PQSrcType, u8PQIPIdx);

    ip_Info = _MDrv_Scl_PQ_GetTable(u8TabIdx, u8PQIPIdx);
    SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]%s %d::SrcType=%d, IPIdx=%d, TabType=%d\r\n"
        ,__FUNCTION__, __LINE__, u16PQSrcType, u8PQIPIdx, ip_Info.u8TabType);
    while (1)
    {
        u32Addr = (ip_Info.pIPTable[0]<<8) + ip_Info.pIPTable[1];
        if(u32Addr == _END_OF_TBL_) // check end of table
        {
            break;
        }
        ip_Info.pIPTable+=(REG_ADDR_SIZE+REG_MASK_SIZE+ip_Info.u8TabNums); // next
        u16DataIdx++;
    }
    return u16DataIdx;
}
static void _MDrv_Scl_PQ_SetPQDebugFlag(u8 u8PQIPIdx)
{
    switch (u8PQIPIdx)
    {
        case PQ_IP_MCNR_Main ... PQ_IP_NLM_Main:
        case PQ_IP_XNR_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_BEFORECROP;
            break;

        case PQ_IP_ColorEng_422to444_Main ... PQ_IP_ColorEng_444to422_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_COLORENG;
            break;
        case PQ_IP_VIP_HLPF_Main ... PQ_IP_VIP_UVC_Main:
        case PQ_IP_VIP_ACK_Main ... PQ_IP_VIP_YCbCr_Clip_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_VIPY;
            break;
        case PQ_IP_VIP_FCC_full_range_Main ... PQ_IP_VIP_IBC_SETTING_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_VIPC;
            break;

        case PQ_IP_YEE_Main ... PQ_IP_UV_ADJUST_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_AIP;
            break;

        case PQ_IP_PFC_Main ... PQ_IP_YUV_Gamma_Main:
            gu8debugflag = EN_DBGMG_PQLEVEL_AIPPOST;
            break;
        default:
            gu8debugflag = EN_DBGMG_PQLEVEL_ELSE;
            break;
    }

}

static void _MDrv_Scl_PQ_LoadTableByData(u16 u16PQSrcType, u8 u8PQIPIdx, PQ_DATA_INFO *pData)
{
    EN_IP_Info ip_Info;
    u8 u8TabIdx = 0;

    u8TabIdx = (u8)_MDrv_Scl_PQ_GetTableIndex(u16PQSrcType, u8PQIPIdx);
    _MDrv_Scl_PQ_SetPQDebugFlag(u8PQIPIdx);
    ip_Info = _MDrv_Scl_PQ_GetTable(u8TabIdx, u8PQIPIdx);
    SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]%s %d::SrcType=%d, IPIdx=%d, TabType=%d\r\n"
        ,__FUNCTION__, __LINE__, u16PQSrcType, u8PQIPIdx, ip_Info.u8TabType);


    switch(ip_Info.u8TabType)
    {
        case PQ_TABTYPE_SCALER:
        case PQ_TABTYPE_NE:
            _MDrv_Scl_PQ_DumpSclaerRegTableByData(&ip_Info, pData);
            break;

        default:
            sclprintf("[PQ]%s %d: Unsupport TabType:%d\n", __FUNCTION__, __LINE__, ip_Info.u8TabType);
            break;
    }
}


static void _MDrv_Scl_PQ_LoadTableBySrcType(u16 u16PQSrcType, u8 u8PQIPIdx)
{
    EN_IP_Info ip_Info;
    u8 QMIPtype_size,i;
    u8 u8TabIdx = 0;
    //XC_ApiStatusEx stXCStatusEx; Ryan

    if (u8PQIPIdx==PQ_IP_ALL)
    {
        QMIPtype_size=_Scl_PQTableInfo.u8PQ_IP_Num;
        u8PQIPIdx=0;
    }
    else
    {
        QMIPtype_size=1;
    }

    //for debug
    //msAPI_Scaler_SetBlueScreen(DISABLE, 0x00);
    //MApi_XC_GenerateBlackVideo(FALSE);

    for(i=0; i<QMIPtype_size; i++, u8PQIPIdx++)
    {
        if (_Scl_PQTableInfo.pSkipRuleIP[u8PQIPIdx]) {

            SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]SrcType=%u, IPIdx=%u, TabIdx=%u\r\n"
                ,(u16)u16PQSrcType, (u16)u8PQIPIdx, (u16)u8TabIdx);
            PQ_DBG(sclprintf("skip ip idx:%u\r\n", u8PQIPIdx));
            continue;
        }

        u8TabIdx = (u8)_MDrv_Scl_PQ_GetTableIndex(u16PQSrcType, u8PQIPIdx);
        SCL_DBG(SCL_DBG_LV_DRVPQ() &gu8debugflag, "[PQ]SrcType=%u, IPIdx=%u, TabIdx=%u\r\n"
            ,(u16)u16PQSrcType, (u16)u8PQIPIdx, (u16)u8TabIdx);

        ip_Info = _MDrv_Scl_PQ_GetTable(u8TabIdx, u8PQIPIdx);
        if(ip_Info.u8TabType == PQ_TABTYPE_SCALER ||ip_Info.u8TabType == PQ_TABTYPE_NE)//paul
        {
            _MDrv_Scl_PQ_DumpTable(&ip_Info);
        }
        //DrvSclOsDelayTask(1500);
    }
}

static void _MDrv_Scl_PQ_AddTable(PQTABLE_INFO *pPQTableInfo)
{
    _Scl_PQTableInfo = *pPQTableInfo;
}


INSTALL_PQ_FUNCTIONS(PQTABLE_NAME)
#endif

#undef _DRVPQ_TEMPLATE_H_
#endif /* _DRVPQ_TEMPLATE_H_ */
