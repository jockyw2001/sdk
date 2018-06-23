#include "MsTypes.h"
#include <stdio.h>
#include <string.h>
#include "utopia_dapi.h"
#include "utopia.h"
#include "drvSERFLASH.h"
#include "drvSERFLASH_private.h"
#include "MsOS.h"

#include "ULog.h"

#define TAG_SERFLASH "SERFLASH"

enum
{
    FLASH_POOL_ID_SERFLASH=0
} eFlashPoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static MS_U32 u32SerflashArray[2];

void SERFLASH_Atoi(char *s)
{
    int i = 0;
    int b = 0;

    memset(u32SerflashArray, 0, sizeof(u32SerflashArray));

    for(i = 0; s[i] != '\0'; i++)
    {
        if((s[i]=='A') || s[i]=='a')
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+10;
        }
        else if((s[i]=='B') || (s[i]=='b'))
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+11;
        }
        else if((s[i]=='C') || (s[i]=='c'))
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+12;
        }
        else if((s[i]=='D') || s[i]=='d')
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+13;
        }
        else if((s[i]=='E') || s[i]=='e')
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+14;
        }
        else if((s[i]=='F') || s[i]=='f')
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+15;
        }
        else if (s[i]==' ')
        {
            b++;
        }
        else
        {
            u32SerflashArray[b] = u32SerflashArray[b]*16+s[i]-'0';
        }
    }
    return;
}

MS_U32 SERFLASHMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    MS_U8 *pu8ReadBUffer;
    MS_U32 u32Count = 0;
    char test[40];
    MS_U8 u8FlashId[3];
    SERFLASH_Info* pSerFlashInfo;

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            MdbPrint(paraCmdLine->u64ReqHdl, "LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
            MdbPrint(paraCmdLine->u64ReqHdl, "u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
            MdbPrint(paraCmdLine->u64ReqHdl, "pcCmdLine: %s\n", paraCmdLine->pcCmdLine);

            strcpy(test, paraCmdLine->pcCmdLine);

            if(strncmp(paraCmdLine->pcCmdLine, "help", 4) == 0)
            {
                MdbPrint(paraCmdLine->u64ReqHdl, "---------MStar SERFLASH Help---------\n");
                MdbPrint(paraCmdLine->u64ReqHdl, "1. Read address \n");
                MdbPrint(paraCmdLine->u64ReqHdl, "2. Read size\n");
                MdbPrint(paraCmdLine->u64ReqHdl, "[e.g] echo 0 20 > /proc/utopia_mdb/serflash\n");
            }
            else
            {
                SERFLASH_Atoi(test);
                pu8ReadBUffer = malloc(sizeof(MS_U8) * u32SerflashArray[1]);

                memset(pu8ReadBUffer, 0, sizeof(MS_U8) * u32SerflashArray[1]);
                if(MDrv_FLASH_Read(u32SerflashArray[0], u32SerflashArray[1], pu8ReadBUffer) != TRUE)
                {
                    MdbPrint(paraCmdLine->u64ReqHdl,"MDrv_FLASH_Read Failed! \n");
                }
                else
                {
                    for(u32Count = 0; u32Count < u32SerflashArray[1]; u32Count++)
                    {
                        MdbPrint(paraCmdLine->u64ReqHdl,"Read buffer Data[%d]: %lx.\n", u32Count, pu8ReadBUffer[u32Count]);
                    }
                }

                free(pu8ReadBUffer);
            }
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;

            MdbPrint(paraGetInfo->u64ReqHdl,"\n---------MStar SERFLASH Cat Info--------- \n");

            MDrv_SERFLASH_ReadID( u8FlashId, sizeof(u8FlashId));
            MdbPrint(paraGetInfo->u64ReqHdl, "Read Flash ID\n"
                                   "\tMID = 0x%x\n"
                                   "\tDID0 = 0x%x\n"
                                   "\tDID1 = 0x%x\n",
                                   (int)u8FlashId[0],
                                   (int)u8FlashId[1],
                                   (int)u8FlashId[2]);

            pSerFlashInfo = MDrv_SERFLASH_GetInfo();
            MdbPrint(paraGetInfo->u64ReqHdl, "Get Flash Info\n"
                                   "\tu32AccessWidth = %d\n"
                                   "\tu32TotalSize   = %d\n"
                                   "\tu32SecNum      = %d\n"
                                   "\tu32SecSize     = %d\n",
                                   pSerFlashInfo->u32AccessWidth,
                                   pSerFlashInfo->u32TotalSize,
                                   pSerFlashInfo->u32SecNum,
                                   pSerFlashInfo->u32SecSize);
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            break;
        default:
            MdbPrint(paraGetInfo->u64ReqHdl,"unknown cmd\n", __LINE__);
            break;
    }
    return 0;
}
#endif

void FLASHRegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_FLASH, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)FLASHOpen, (FUtopiaClose)FLASHClose, (FUtopiaIOctl)FLASHIoctl);

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("serflash", (FUtopiaMdbIoctl)SERFLASHMdbIoctl);
#endif

    // 2. deal with resource
    void* psResource = NULL;
    UtopiaModuleAddResourceStart(pUtopiaModule, FLASH_POOL_ID_SERFLASH);
    UtopiaResourceCreate("SERFLASH", sizeof(SERFLASH_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pUtopiaModule, psResource, FLASH_POOL_ID_SERFLASH);
    UtopiaModuleAddResourceEnd(pUtopiaModule, FLASH_POOL_ID_SERFLASH);
}

MS_U32 FLASHOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    ULOGD(TAG_SERFLASH, "\n[SERFLASH INFO] spi flash open \n");

    SERFLASH_INSTANT_PRIVATE *pSERFlashPri = NULL;
    UtopiaInstanceCreate(sizeof(SERFLASH_INSTANT_PRIVATE), ppInstance);
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pSERFlashPri);
#if (SERFLASH_UTOPIA20)
    pSERFlashPri->fpSERFLASHGetInfo = MDrv_SERFLASH_GetInfo;
    pSERFlashPri->fpSERFLASHGetLibVer = MDrv_SERFLASH_GetLibVer;
    pSERFlashPri->fpSERFLASHGetStatus = MDrv_SERFLASH_GetStatus;
    pSERFlashPri->fpSERFLASHSetDbgLevel = _MDrv_SERFLASH_SetDbgLevel;
    pSERFlashPri->fpSERFLASHSetWPInfo = _MDrv_SERFLASH_SetWPInfo;
    pSERFlashPri->fpSERFLASHSetMcuCSCallBack = _MDrv_SERFLASH_SetMcuCSCallBack;
    pSERFlashPri->fpSERFLASHSetFlashWPCallBack = _MDrv_SERFLASH_SetFlashWPCallBack;
    pSERFlashPri->fpSERFLASHDetectType = _MDrv_SERFLASH_DetectType;
    pSERFlashPri->fpSERFLASHDetectSize = _MDrv_SERFLASH_DetectSize;
    pSERFlashPri->fpSERFLASHSet2XRead = _MDrv_SERFLASH_Set2XRead;
    pSERFlashPri->fpSERFLASHSetCKG = _MDrv_SERFLASH_SetCKG;
    pSERFlashPri->fpSERFLASHClkDiv = _MDrv_SERFLASH_ClkDiv;
    pSERFlashPri->fpSERFLASHSetMode = _MDrv_SERFLASH_SetMode;
    pSERFlashPri->fpSERFLASHReadUID = _MDrv_SERFLASH_ReadUID;
    pSERFlashPri->fpSERFLASHChipSelect = _MDrv_SERFLASH_ChipSelect;
    pSERFlashPri->fpSERFLASHInit = MDrv_SERFLASH_Init;
    pSERFlashPri->fpSERFLASHReadID = _MDrv_SERFLASH_ReadID;
    pSERFlashPri->fpSERFLASHRead = _MDrv_SERFLASH_Read;
    pSERFlashPri->fpSERFLASHEraseChip = _MDrv_SERFLASH_EraseChip;
    pSERFlashPri->fpSERFLASHAddressToBlock = _MDrv_SERFLASH_AddressToBlock;
    pSERFlashPri->fpSERFLASHBlockToAddress = _MDrv_SERFLASH_BlockToAddress;
    pSERFlashPri->fpSERFLASHAddressErase = _MDrv_SERFLASH_AddressErase;
    pSERFlashPri->fpSERFLASHBlockErase = _MDrv_SERFLASH_BlockErase;
    pSERFlashPri->fpSERFLASHSectorErase = _MDrv_SERFLASH_SectorErase;
    pSERFlashPri->fpSERFLASHCheckWriteDone = _MDrv_SERFLASH_CheckWriteDone;
    pSERFlashPri->fpSERFLASHWrite = _MDrv_SERFLASH_Write;
    pSERFlashPri->fpSERFLASHDMA = _MDrv_SERFLASH_DMA;
    pSERFlashPri->fpSERFLASHWriteProtect = _MDrv_SERFLASH_WriteProtect;
    pSERFlashPri->fpSERFLASHWriteProtectEnableAllRange = _MDrv_SERFLASH_WriteProtect_Enable_All_Range;
    pSERFlashPri->fpSERFLASHWriteProtectDisableAllRange = _MDrv_SERFLASH_WriteProtect_Disable_All_Range;
    pSERFlashPri->fpSERFLASHWriteProtectDisableRangeSet = _MDrv_SERFLASH_WriteProtect_Disable_Range_Set;
    pSERFlashPri->fpSERFLASHWriteProtectArea = _MDrv_SERFLASH_WriteProtect_Area;
    pSERFlashPri->fpSERFLASHReadStatusRegister = _MDrv_SERFLASH_ReadStatusRegister;
    pSERFlashPri->fpSERFLASHReadStatusRegister2 = _MDrv_SERFLASH_ReadStatusRegister2;
    pSERFlashPri->fpSERFLASHWriteStatusRegister = _MDrv_SERFLASH_WriteStatusRegister;
    pSERFlashPri->fpSERFLASHCopyHnd = _MDrv_SERFLASH_CopyHnd;
    pSERFlashPri->fpSERFLASHSetGPIO = _MDrv_SERFLASH_SetGPIO;
#else
    pSERFlashPri->fpSERFLASHGetInfo = MDrv_SERFLASH_GetInfo;
    pSERFlashPri->fpSERFLASHGetLibVer = MDrv_SERFLASH_GetLibVer;
    pSERFlashPri->fpSERFLASHGetStatus = MDrv_SERFLASH_GetStatus;
    pSERFlashPri->fpSERFLASHSetDbgLevel = MDrv_SERFLASH_SetDbgLevel;
    pSERFlashPri->fpSERFLASHSetWPInfo = MDrv_SERFLASH_SetWPInfo;
    pSERFlashPri->fpSERFLASHSetMcuCSCallBack = MDrv_SERFLASH_SetMcuCSCallBack;
    pSERFlashPri->fpSERFLASHSetFlashWPCallBack = MDrv_SERFLASH_SetFlashWPCallBack;
    pSERFlashPri->fpSERFLASHDetectType = MDrv_SERFLASH_DetectType;
    pSERFlashPri->fpSERFLASHDetectSize = MDrv_SERFLASH_DetectSize;
    pSERFlashPri->fpSERFLASHSet2XRead = MDrv_SERFLASH_Set2XRead;
    pSERFlashPri->fpSERFLASHSetCKG = MDrv_SERFLASH_SetCKG;
    pSERFlashPri->fpSERFLASHClkDiv = MDrv_SERFLASH_ClkDiv;
    pSERFlashPri->fpSERFLASHSetMode = MDrv_SERFLASH_SetMode;
    pSERFlashPri->fpSERFLASHReadUID = MDrv_SERFLASH_ReadUID;
    pSERFlashPri->fpSERFLASHChipSelect = MDrv_SERFLASH_ChipSelect;
    pSERFlashPri->fpSERFLASHInit = MDrv_SERFLASH_Init;
    pSERFlashPri->fpSERFLASHReadID = MDrv_SERFLASH_ReadID;
    pSERFlashPri->fpSERFLASHRead = MDrv_SERFLASH_Read;
    pSERFlashPri->fpSERFLASHEraseChip = MDrv_SERFLASH_EraseChip;
    pSERFlashPri->fpSERFLASHAddressToBlock = MDrv_SERFLASH_AddressToBlock;
    pSERFlashPri->fpSERFLASHBlockToAddress = MDrv_SERFLASH_BlockToAddress;
    pSERFlashPri->fpSERFLASHAddressErase = MDrv_SERFLASH_AddressErase;
    pSERFlashPri->fpSERFLASHBlockErase = MDrv_SERFLASH_BlockErase;
    pSERFlashPri->fpSERFLASHSectorErase = MDrv_SERFLASH_SectorErase;
    pSERFlashPri->fpSERFLASHCheckWriteDone = MDrv_SERFLASH_CheckWriteDone;
    pSERFlashPri->fpSERFLASHWrite = MDrv_SERFLASH_Write;
    pSERFlashPri->fpSERFLASHDMA = MDrv_SERFLASH_DMA;
    pSERFlashPri->fpSERFLASHWriteProtect = MDrv_SERFLASH_WriteProtect;
    pSERFlashPri->fpSERFLASHWriteProtectEnableAllRange = MDrv_SERFLASH_WriteProtect_Enable_All_Range;
    pSERFlashPri->fpSERFLASHWriteProtectDisableAllRange = MDrv_SERFLASH_WriteProtect_Disable_All_Range;
    pSERFlashPri->fpSERFLASHWriteProtectDisableRangeSet = MDrv_SERFLASH_WriteProtect_Disable_Range_Set;
    pSERFlashPri->fpSERFLASHWriteProtectArea = MDrv_SERFLASH_WriteProtect_Area;
    pSERFlashPri->fpSERFLASHReadStatusRegister = MDrv_SERFLASH_ReadStatusRegister;
    pSERFlashPri->fpSERFLASHReadStatusRegister2 = MDrv_SERFLASH_ReadStatusRegister2;
    pSERFlashPri->fpSERFLASHWriteStatusRegister = MDrv_SERFLASH_WriteStatusRegister;
    pSERFlashPri->fpSERFLASHCopyHnd = MDrv_SERFLASH_CopyHnd;
    pSERFlashPri->fpSERFLASHSetGPIO = MDrv_SERFLASH_SetGPIO;
#endif

    return TRUE;
}

// FIXME: why static?
MS_U32 FLASHIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    MS_U32 u32Ret = 0;
    void* pModule = NULL;
    UtopiaInstanceGetModule(pInstance, &pModule);
    void* pResource = NULL;
    PSERFLASH_GETLIBVER pGetLibVerParam = NULL;
    PSERFLASH_GETSTATUS pGetStatusParam = NULL;
    PSERFLASH_SETDBGLEVEL pSetDbgLevelParam = NULL;
    PSERFLASH_SETWPINFO pSetWPInfoParam = NULL;
    PSERFLASH_SETMCUCSCB pSetMcuCSCBParam = NULL;
    PSERFLASH_SETFLASHWPCB pSetFlashWPCBParam = NULL;
    PSERFLASH_DETECTSIZE pDetectSizeParam = NULL;
    PSERFLASH_SET2XREAD pSet2XReadParam = NULL;
    PSERFLASH_SETCKG pSetCkgParam = NULL;
    PSERFLASH_CLKDIV pClkDivParam = NULL;
    PSERFLASH_SETMODE pSetModeParam = NULL;
    PSERFLASH_CHIPSELECT pChipSelectParam = NULL;
    PSERFLASH_READSTATUSREG pReadStatusRegParam = NULL;
    PSERFLASH_READSTATUSREG2 pReadStatusReg2Param = NULL;
    PSERFLASH_WRITESTATUSREG pWriteStatusRegParam = NULL;
    PSERFLASH_SETGPIO pSetGPIOParam = NULL;
    PSERFLASH_READID pReadIDParam = NULL;
    PSERFLASH_READ pReadParam = NULL;
    PSERFLASH_ADDRTOBLK pAddr2BlkParam = NULL;
    PSERFLASH_BLKTOADDR pBlk2AddrParam = NULL;
    PSERFLASH_ADDRERASE pAddrEraseParam = NULL;
    PSERFLASH_BLKERASE pBlkEraseParam = NULL;
    PSERFLASH_SECERASE pSecEraseParam = NULL;
    PSERFLASH_WRITE pWriteParam = NULL;
    PSERFLASH_DMA pDMAParam = NULL;
    PSERFLASH_WP pWPParam = NULL;
    PSERFLASH_WPDISABLERANGESET pWPDisableRangeSetParam = NULL;
    PSERFLASH_WPAREA pWPAreaParam = NULL;
    PSERFLASH_COPYHND pCopyHndParam = NULL;
    SERFLASH_INSTANT_PRIVATE* psSERFLASHInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psSERFLASHInstPri);

    switch(u32Cmd)
    {
        case MDrv_CMD_SERFLASH_GetInfo:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_GetInfo\n");
            // this function won't occupy h/w, so don't need to call UtopiaResourceObtainToInstant
            u32Ret = (MS_VIRT)psSERFLASHInstPri->fpSERFLASHGetInfo();
            return u32Ret;
        case MDrv_CMD_SERFLASH_GetLibVer:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_GetLibVer\n");
            pGetLibVerParam = (PSERFLASH_GETLIBVER)pArgs;
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHGetLibVer(pGetLibVerParam->ppVersion);
            return u32Ret;
        case MDrv_CMD_SERFLASH_GetStatus:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_GetStatus\n");
            pGetStatusParam = (PSERFLASH_GETSTATUS)pArgs;
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHGetStatus(pGetStatusParam->pDrvStatus);
            return u32Ret;
        case MDrv_CMD_SERFLASH_SetDbgLevel:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetDbgLevel\n");
            pSetDbgLevelParam = (PSERFLASH_SETDBGLEVEL)pArgs;
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSetDbgLevel(pSetDbgLevelParam->u8DbgLevel);
            return u32Ret;
        case MDrv_CMD_SERFLASH_SetWPInfo:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetWPInfo\n");
            pSetWPInfoParam = (PSERFLASH_SETWPINFO)pArgs;
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSetWPInfo(pSetWPInfoParam->bWPInfo);
            break;
        case MDrv_CMD_SERFLASH_SetMcuCSCallBack:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetMcuCSCallBack\n");
            pSetMcuCSCBParam = (PSERFLASH_SETMCUCSCB)pArgs;
            psSERFLASHInstPri->fpSERFLASHSetMcuCSCallBack(pSetMcuCSCBParam->ChipSel_cb);
            return 0;
        case MDrv_CMD_SERFLASH_SetFlashWPCallBack:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetFlashWPCallBack\n");
            pSetFlashWPCBParam = (PSERFLASH_SETFLASHWPCB)pArgs;
            psSERFLASHInstPri->fpSERFLASHSetFlashWPCallBack(pSetFlashWPCBParam->FlashWP_cb);
            return 0;
        case MDrv_CMD_SERFLASH_DetectType:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_DetectType\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHDetectType();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_DetectSize:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_DetectSize\n");
            pDetectSizeParam = (PSERFLASH_DETECTSIZE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHDetectSize(pDetectSizeParam->u32FlashSize);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_Set2XRead:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_Set2XRead\n");
            pSet2XReadParam = (PSERFLASH_SET2XREAD)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSet2XRead(pSet2XReadParam->b2XMode);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_SetCKG:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetCKG\n");
            pSetCkgParam = (PSERFLASH_SETCKG)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSetCKG(pSetCkgParam->eCKGspi);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ClkDiv:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ClkDiv\n");
            pClkDivParam = (PSERFLASH_CLKDIV)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            psSERFLASHInstPri->fpSERFLASHClkDiv(pClkDivParam->eClkDivspi);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_SetMode:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetMode\n");
            pSetModeParam = (PSERFLASH_SETMODE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSetMode(pSetModeParam->bXiuRiu);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ReadUID:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ReadUID\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHReadUID();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ChipSelect:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ChipSelect\n");
            pChipSelectParam = (PSERFLASH_CHIPSELECT)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHChipSelect(pChipSelectParam->u8FlashIndex);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_Init:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_Init\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            psSERFLASHInstPri->fpSERFLASHInit();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ReadID:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ReadID\n");
            pReadIDParam = (PSERFLASH_READID)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHReadID(pReadIDParam->pu8FlashID,pReadIDParam->u32IDSize);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_Read:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_Read\n");
            pReadParam = (PSERFLASH_READ)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHRead(pReadParam->u32FlashAddr,pReadParam->u32FlashSize,pReadParam->user_buffer);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_EraseChip:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_EraseChip\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHEraseChip();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_AddressToBlock:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_AddressToBlock\n");
            pAddr2BlkParam = (PSERFLASH_ADDRTOBLK)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHAddressToBlock(pAddr2BlkParam->u32FlashAddr,pAddr2BlkParam->pu32BlockIndex);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_BlockToAddress:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_BlockToAddress\n");
            pBlk2AddrParam = (PSERFLASH_BLKTOADDR)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHBlockToAddress(pBlk2AddrParam->u32BlockIndex,pBlk2AddrParam->pu32FlashAddr);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_AddressErase:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_AddressErase\n");
            pAddrEraseParam = (PSERFLASH_ADDRERASE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHAddressErase(pAddrEraseParam->u32StartAddr,pAddrEraseParam->u32EraseSize,pAddrEraseParam->bWait);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_BlockErase:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_BlockErase\n");
            pBlkEraseParam = (PSERFLASH_BLKERASE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHBlockErase(pBlkEraseParam->u32StartBlock,pBlkEraseParam->u32EndBlock,pBlkEraseParam->bWait);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_SectorErase:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SectorErase\n");
            pSecEraseParam = (PSERFLASH_SECERASE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHSectorErase(pSecEraseParam->u32StartAddr,pSecEraseParam->u32EndAddr);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_CheckWriteDone:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_CheckWriteDone\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHCheckWriteDone();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_Write:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_Write\n");
            pWriteParam = (PSERFLASH_WRITE)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWrite(pWriteParam->u32FlashAddr,pWriteParam->u32FlashSize,pWriteParam->user_buffer);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_DMA:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_DMA\n");
            pDMAParam = (PSERFLASH_DMA)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHDMA(pDMAParam->u32FlashStart,pDMAParam->u32DRAMStart,pDMAParam->u32Size);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteProtect:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteProtect\n");
            pWPParam = (PSERFLASH_WP)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteProtect(pWPParam->bEnable);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteProtect_Enable_All_Range:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteProtect_Enable_All_Range\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteProtectEnableAllRange();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteProtect_Disable_All_Range:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteProtect_Disable_All_Range\n");
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteProtectDisableAllRange();
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteProtect_Disable_Range_Set:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteProtect_Disable_Range_Set\n");
            pWPDisableRangeSetParam = (PSERFLASH_WPDISABLERANGESET)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteProtectDisableRangeSet(pWPDisableRangeSetParam->u32DisableLowerBound,pWPDisableRangeSetParam->u32DisableSize);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteProtect_Area:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteProtect_Disable_Range_Set\n");
            pWPAreaParam = (PSERFLASH_WPAREA)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteProtectArea(pWPAreaParam->bEnableAllArea,pWPAreaParam->u8BlockProtectBits);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ReadStatusRegister:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ReadStatusRegister\n");
            pReadStatusRegParam = (PSERFLASH_READSTATUSREG)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHReadStatusRegister(pReadStatusRegParam->pu8StatusReg);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_ReadStatusRegister2:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_ReadStatusRegister2\n");
            pReadStatusReg2Param = (PSERFLASH_READSTATUSREG2)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHReadStatusRegister2(pReadStatusReg2Param->pu8StatusReg);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_WriteStatusRegister:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_WriteStatusRegister\n");
            pWriteStatusRegParam = (PSERFLASH_WRITESTATUSREG)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHWriteStatusRegister(pWriteStatusRegParam->u16StatusReg);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_CopyHnd:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_CopyHnd\n");
            pCopyHndParam = (PSERFLASH_COPYHND)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            u32Ret = (MS_U32)psSERFLASHInstPri->fpSERFLASHCopyHnd(pCopyHndParam->u32FlashAddr,
                                                                pCopyHndParam->u32DstAddr,
                                                                pCopyHndParam->u32Len,
                                                                pCopyHndParam->eDstDev,
                                                                pCopyHndParam->u8OpCfg);
            UtopiaResourceRelease(pResource);
            return 0;
        case MDrv_CMD_SERFLASH_SetGPIO:
            ULOGD(TAG_SERFLASH,"FLASHIoctl - MDrv_CMD_SERFLASH_SetGPIO\n");
            pSetGPIOParam = (PSERFLASH_SETGPIO)pArgs;
            if(UtopiaResourceObtain(pModule, FLASH_POOL_ID_SERFLASH, &pResource) != 0)
            {
                ULOGE(TAG_SERFLASH,"UtopiaResourceObtainToInstant fail\n");
                return 0xFFFFFFFF;
            }
            psSERFLASHInstPri->fpSERFLASHSetGPIO(pSetGPIOParam->bSwitch);
            UtopiaResourceRelease(pResource);
            return 0;
        default:
            break;
    };

    return 0; // FIXME: error code
}

MS_U32 FLASHClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return TRUE;
}

