#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "string.h"
#include <stdio.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif
#include "MsTypes.h"
#include "utopia_dapi.h"
#include "drvBDMA.h"
#include "drvBDMA_private.h"
#include "MsOS.h"
#if (BDMA_UTOPIA20)
#include "utopia.h"
#endif

#include "ULog.h"

#define TAG_BDMA "BDMA"

enum
{
    BDMA_POOL_ID_BDMA0=0
} eBdmaPoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT

static int array[5];

int atoi_mstar(char *s)
{
    int sum = 0;
    int i = 0;
    int b = 0;
    for(i = 0;s[i] != '\0';i++)
    {
        if((s[i]=='A') || s[i]=='a')
        {
          sum = sum*16+10;
          array[b] = array[b]*16+10;
        }
        else if((s[i]=='B') || (s[i]=='b'))
        {
          sum = sum*16+11;
          array[b] = array[b]*16+11;
        }
        else if((s[i]=='C') || (s[i]=='c'))
        {
          sum = sum*16+12;
          array[b] = array[b]*16+12;
        }
        else if((s[i]=='D') || s[i]=='d')
        {
          sum = sum*16+13;
          array[b] = array[b]*16+13;
        }
        else if((s[i]=='E') || s[i]=='e')
        {
          sum = sum*16+14;
          array[b] = array[b]*16+14;
        }
        else if((s[i]=='F') || s[i]=='f')
        {
          sum = sum*16+15;
          array[b] = array[b]*16+15;
        }
        else if (s[i]==' ')
        {
          b++;
        }
        else
        {
          sum = sum*16+s[i]-'0'; 
          array[b] = array[b]*16+s[i]-'0';
        }
    }
   return sum; 
}; 


char *strtok_BDMA(char * s, const char * delim)
{
    char *spanp;
    int c, sc;
    char *tok;
    static char * last;
 
    if(!s && (s=last) == NULL) return 0;
 
    c = *s++;
    for(spanp = (char*)delim ; (sc = *spanp++) ; ){
        if(c==sc){
            c = *s++;
            spanp = (char*)delim;
        }
    }
 
    if(c==0) return (last = NULL);
    tok = s - 1;
    while(1){
        c = *s++;
        spanp = (char * )delim;
        do{
            if( (sc==*spanp++)==c){
                if(!c) s = NULL;
                else s[-1] = 0;
                last = NULL;
                return (tok);
            }
        }while(sc);        
    }
}

void split(char **arr, char *str, const char *del)
 {
     char *s = strtok_BDMA(str, del);
     while(s != NULL)
     {
         *arr++ = s;
         s = strtok_BDMA(NULL, del);
     }
}

MS_U32 BDMAMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    char test[256];

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
             paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
             MdbPrint(paraCmdLine->u64ReqHdl,"LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
             MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
             MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);

             strncpy(test, paraCmdLine->pcCmdLine, 255);

             if(strncmp(paraCmdLine->pcCmdLine, "help", 4) == 0)
             {
                 MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar BDMA Help---------\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"1. Source address\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"2. Destination address\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"3. Source Device + Sorce device data width\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"4. Destination Device + Destination device data width\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"5. Transmitt Length\n");
                 MdbPrint(paraCmdLine->u64ReqHdl,"[e.g] echo 0 10 40 40 10 > /proc/utopia_mdb/bdma\n");
             }
             else
             {
                 atoi_mstar(test);
                 paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
                 MDrv_BDMA_Debug_echo(array[0],array[1],array[2],array[3],array[4]);
             }
             break;
        case MDBCMD_GETINFO:
             paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
             MdbPrint(paraGetInfo->u64ReqHdl,"LINE:%d, MDBCMD_GETINFO\n", __LINE__);
             paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
             MDrv_BDMA_Debug_cat(paraGetInfo->u64ReqHdl);
             break;
        default:
             paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
             MdbPrint(paraGetInfo->u64ReqHdl,"unknown cmd\n", __LINE__);
             break;
    }
    return 0;
}
#endif

// this func will be call to init by utopia20 framework
void BDMARegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
	UtopiaModuleCreate(MODULE_BDMA, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)BDMAOpen, (FUtopiaClose)BDMAClose, (FUtopiaIOctl)BDMAIoctl);

#if defined(MSOS_TYPE_LINUX_KERNEL)
    UtopiaModuleSetupSTRFunctionPtr(pUtopiaModule,(FUtopiaSTR)BDMAStr);
    //UtopiaModuleSetSTRPrivate(pUtopiaModule, STRPrivateDataSize);
#endif

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("bdma", (FUtopiaMdbIoctl)BDMAMdbIoctl);
#endif

    // 2. deal with resource
    void* psResource = NULL;
    // start func to add res, call once will create 2 access in resource. Also can declare BDMA_POOL_ID_BDMA1 for another channel depend on driver owner.
	UtopiaModuleAddResourceStart(pUtopiaModule, BDMA_POOL_ID_BDMA0);
    // resource can alloc private for internal use, ex, BDMA_RESOURCE_PRIVATE
    UtopiaResourceCreate("bdma0", sizeof(BDMA_RESOURCE_PRIVATE), &psResource);
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, BDMA_POOL_ID_BDMA0);

    // when there are 2 ch bdma, call UtopiaResourceCreate for BDMA_POOL_ID_BDMA0 again, then we have 2 resource.
#if 0
    psResource = (UTOPIA_RESOURCE*)UtopiaResourceCreate("bdma1", sizeof(BDMA_RESOURCE_PRIVATE));
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, BDMA_POOL_ID_BDMA0);
#endif
    // end function to add res
	UtopiaModuleAddResourceEnd(pUtopiaModule, BDMA_POOL_ID_BDMA0);
}

MS_U32 BDMAOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    BDMA_INSTANT_PRIVATE *pBdmaPri = NULL;
    void *pBdmaPriVoid = NULL;
    //printf("\n[BDMA INFO] bdma open");
    //UTOPIA_TRACE(MS_UTOPIA_DB_LEVEL_TRACE,printf("enter %s %d\n",__FUNCTION__,__LINE__));
    // instance is allocated here, also can allocate private for internal use, ex, BDMA_INSTANT_PRIVATE
    UtopiaInstanceCreate(sizeof(BDMA_INSTANT_PRIVATE), ppInstance);
    // setup func in private and assign the calling func in func ptr in instance private
    UtopiaInstanceGetPrivate(*ppInstance, &pBdmaPriVoid);
    pBdmaPri = (BDMA_INSTANT_PRIVATE*)pBdmaPriVoid;
    pBdmaPri->fpBDMAMemCpy = (IOCTL_BDMA_MEMCPY)_MDrv_BDMA_MemCopy;
    pBdmaPri->fpBDMASearch = (IOCTL_BDMA_SEARCH)_MDrv_BDMA_Search;
    pBdmaPri->fpBDMACRC32 = (IOCTL_BDMA_CRC32)_MDrv_BDMA_CRC32;
    pBdmaPri->fpBDMAPatternFill = (IOCTL_BDMA_PATTERN_FILL)_MDrv_BDMA_PatternFill;
    pBdmaPri->fpBDMAFlashCopy2Dram = (IOCTL_BDMA_FLASHCOPY2DRAM)_MDrv_BDMA_FlashCopy2Dram;
#ifdef MOBF_ENABLE
    //if (u32ModuleVersion == 0x00010001)
    {
        pBdmaPri->fpBDMAMOBFSearch = (IOCTL_BDMA_MOBFSEARCH)_MDrv_BDMA_MOBFSearch;
    }
    //else (u32ModuleVersion == 0x00010002)
    {
        //pBdmaPri->fpBDMAMOBFSearch = (IOCTL_BDMA_MOBFSEARCH)MDrv_BDMA_MOBFSearch_V2;
    }
#endif
    pBdmaPri->fpBDMAGetStatus = (IOCTL_BDMA_GETSTATUS)_MDrv_BDMA_GetStatus;
    pBdmaPri->fpBDMAGetInfo = (IOCTL_BDMA_GETINFO)MDrv_BDMA_GetInfo;
    pBdmaPri->fpBDMAGetLibVer = (IOCTL_BDMA_GETLIBVER)MDrv_BDMA_GetLibVer;
    pBdmaPri->fpBDMAGetMinSize = (IOCTL_BDMA_GETMINSIZE)MDrv_BDMA_GetMinSize;
    pBdmaPri->fpBDMASetDbgLevel = (IOCTL_BDMA_SETDBGLEVEL)MDrv_BDMA_SetDbgLevel;
    pBdmaPri->fpBDMASetSPIOffsetForMCU = (IOCTL_BDMA_SETSPIOFFSETFORMCU)_MDrv_BDMA_SetSPIOffsetForMCU;
    pBdmaPri->fpBDMAWaitFlashDone = (IOCTL_BDMA_WAITFLASHDONE)_MDrv_BDMA_WaitFlashDone;
    pBdmaPri->fpBDMAInit = (IOCTL_BDMA_INIT)_MDrv_BDMA_Init;
    pBdmaPri->fpBDMACopyHnd = (IOCTL_BDMA_COPYHND)_MDrv_BDMA_CopyHnd;

	return UTOPIA_STATUS_SUCCESS;
}

// FIXME: why static?
MS_U32 BDMAIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
	void* pModule = NULL;
	UtopiaInstanceGetModule(pInstance, &pModule);
	void* pResource = NULL;
    PBDMA_MEMCOPY_PARAM pMemCpyParam = NULL;
    PBDMA_SEARCH_PARAM pSearchParam = NULL;
    PBDMA_CRC32_PARAM pCRC32Param = NULL;
    PBDMA_PATTERN_FILL_PARAM pPattFillParam = NULL;
    PBDMA_FLASHCOPY2DRAM_PARAM pFlash2DramParam = NULL;
#ifdef MOBF_ENABLE
    PBDMA_MOBFSEARCH_PARAM pMOBFSearchParam = NULL;
#endif
    PBDMA_GETSTATUS_PARAM pGetStatusParam = NULL;
    PBDMA_GETINFO_PARAM pGetInfoParam = NULL;
    PBDMA_GETLIBVER_PARAM pGetLibVerParam = NULL;
    PBDMA_GETMINSIZE_PARAM pGetMinSize = NULL;
    PBDMA_SETDBGLEVEL_PARAM pSetDbgLevelParam = NULL;
    BDMA_Info *pBDMAInfo;
    MS_U32 u32Ret;
    MS_PHY phy64SearchAddr;
    MS_U32 u32CRC32;

    //utopia_secure_check(pInstant->pPrivate); // FIXME: check what?


    BDMA_INSTANT_PRIVATE* psBDMAInstPri = NULL;
    void* psBDMAInstPriVoid = NULL;
	UtopiaInstanceGetPrivate(pInstance, (void**)&psBDMAInstPriVoid);
    psBDMAInstPri = (BDMA_INSTANT_PRIVATE*)psBDMAInstPriVoid;

    switch(u32Cmd)
    {
        case MDrv_CMD_BDMA_Init:
        {
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PBDMA_INIT_PARAM pParam = (PBDMA_INIT_PARAM)pArgs;
            u32Ret = psBDMAInstPri->fpBDMAInit(pParam->u64Miu1Base);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        case MDrv_CMD_BDMA_CopyHnd:
        {
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;

            PBDMA_COPYHND_PARAM pParam = (PBDMA_COPYHND_PARAM)pArgs;
            u32Ret = psBDMAInstPri->fpBDMACopyHnd(pParam->phy64SrcAddr, pParam->phy64DstAddr, pParam->u32Len, pParam->eCpyType, pParam->u8OpCfg);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        }
        case MDrv_CMD_BDMA_MemCopy:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_MemCopy\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pMemCpyParam = (PBDMA_MEMCOPY_PARAM)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
	            return UTOPIA_STATUS_ERR_RESOURCE;
            }
            u32Ret = psBDMAInstPri->fpBDMAMemCpy(pMemCpyParam->phy64SrcAddr,pMemCpyParam->phy64DstAddr, pMemCpyParam->u32Len);
            UtopiaResourceRelease(pResource);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_Search:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_Search\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pSearchParam = (PBDMA_SEARCH_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            phy64SearchAddr = psBDMAInstPri->fpBDMASearch(pSearchParam->u32Addr, pSearchParam->u32Len, pSearchParam->u32Pattern, pSearchParam->u32ExcluBit, pSearchParam->eDev);
            pSearchParam->pSearchAddr = phy64SearchAddr;
            UtopiaResourceRelease(pResource);
            if (0xFFFFFFFF != phy64SearchAddr)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_CRC32:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_CRC32\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pCRC32Param = (PBDMA_CRC32_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            u32CRC32 = psBDMAInstPri->fpBDMACRC32(pCRC32Param->phy64Addr,pCRC32Param->u32Len,pCRC32Param->u32Poly,pCRC32Param->u32Seed,pCRC32Param->eDev,pCRC32Param->bReflect);
            UtopiaResourceRelease(pResource);
            pCRC32Param->u32CRC32 = u32CRC32;
            if (0xFFFFFFFF != u32CRC32)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_Pattern_Fill:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_Pattern_Fill\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pPattFillParam = (PBDMA_PATTERN_FILL_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            u32Ret = psBDMAInstPri->fpBDMAPatternFill(pPattFillParam->u32Addr,pPattFillParam->u32Len,pPattFillParam->u32Pattern,pPattFillParam->eDev);
            UtopiaResourceRelease(pResource);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_FlashCopy2Dram:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_FlashCopy2Dram\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pFlash2DramParam = (PBDMA_FLASHCOPY2DRAM_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            u32Ret = psBDMAInstPri->fpBDMAFlashCopy2Dram(pFlash2DramParam->u32FlashAddr,pFlash2DramParam->u32DramAddr,pFlash2DramParam->u32Len);
            UtopiaResourceRelease(pResource);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_MOBFSearch:
#ifdef MOBF_ENABLE
            printf("BDMAIoctl - MDrv_CMD_BDMA_MOBFSearch\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pMOBFSearchParam = (PBDMA_MOBFSEARCH_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            phy64SearchAddr = psBDMAInstPri->fpBDMAMOBFSearch(pMOBFSearchParam->phy64Addr,pMOBFSearchParam->u32Len,pMOBFSearchParam->pMobfPsCfg,pMOBFSearchParam->eDev);
            UtopiaResourceRelease(pResource);
            pMOBFSearchParam->phy64SearchAddr = phy64SearchAddr;
            if (0xFFFFFFFF != phy64SearchAddr)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
#else
            return UTOPIA_STATUS_FAIL;
#endif
        case MDrv_CMD_BDMA_GetStatus:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_GetStatus\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pGetStatusParam = (PBDMA_GETSTATUS_PARAM)pArgs;
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psBDMAInstPri->fpBDMAGetStatus(pGetStatusParam->pStatus);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;
        case MDrv_CMD_BDMA_GetInfo:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_GetInfo\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            // this function won't occupy h/w, so don't need to call UtopiaResourceObtainToInstant
            pGetInfoParam = (PBDMA_GETINFO_PARAM)pArgs;
            pBDMAInfo = psBDMAInstPri->fpBDMAGetInfo();
            memcpy (pGetInfoParam,pBDMAInfo,sizeof(BDMA_Info));
            return UTOPIA_STATUS_SUCCESS;
        case MDrv_CMD_BDMA_GetLibVer:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_GetLibVer\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pGetLibVerParam = (PBDMA_GETLIBVER_PARAM)pArgs;
            u32Ret = psBDMAInstPri->fpBDMAGetLibVer(pGetLibVerParam->ppVersion);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_BDMA_GetMinSize:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_GetMinSize\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pGetMinSize = (PBDMA_GETMINSIZE_PARAM)pArgs;
            pGetMinSize->u32MinSize = (MS_U32)psBDMAInstPri->fpBDMAGetMinSize();
            return UTOPIA_STATUS_SUCCESS;
        case MDrv_CMD_BDMA_SetDbgLevel:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_SetDbgLevel\n");
            if (NULL == pArgs)
                return UTOPIA_STATUS_PARAMETER_ERROR;
            pSetDbgLevelParam = (PBDMA_SETDBGLEVEL_PARAM)pArgs;
            psBDMAInstPri->fpBDMASetDbgLevel(pSetDbgLevelParam->eLevel);
            return UTOPIA_STATUS_SUCCESS;
        case MDrv_CMD_BDMA_SetSPIOffsetForMCU:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_SetSPIOffsetForMCU\n");
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psBDMAInstPri->fpBDMASetSPIOffsetForMCU();
            UtopiaResourceRelease(pResource);
            break;
        case MDrv_CMD_BDMA_WaitFlashDone:
            //printf("BDMAIoctl - MDrv_CMD_BDMA_WaitFlashDone\n");
            if(UtopiaResourceObtain(pModule, BDMA_POOL_ID_BDMA0, &pResource) != 0)
            {
                ULOGE(TAG_BDMA, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            u32Ret = (MS_U32)psBDMAInstPri->fpBDMAWaitFlashDone();
            UtopiaResourceRelease(pResource);
            if (E_BDMA_OK == u32Ret)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        default:
            break;
    };

	return UTOPIA_STATUS_SUCCESS; // FIXME: error code
}

MS_U32 BDMAClose(void* pInstance)
{
	UtopiaInstanceDelete(pInstance);

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 BDMAStr(MS_U32 u32PowerState, void* pModule)
{
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;

    //UtopiaModuleGetSTRPrivate(pModule, (void**));

    if (u32PowerState == E_POWER_SUSPEND)
    {
        /* Please Implement Module Suspend Flow Here. */
        u32Return = UTOPIA_STATUS_SUCCESS;//SUSPEND_OK;
    }
    else if (u32PowerState == E_POWER_RESUME)
    {
        /* Please Implement Module Resume Flow Here. */
        u32Return = UTOPIA_STATUS_SUCCESS;//RESUME_OK;
    }
    else
    {
        u32Return = UTOPIA_STATUS_FAIL;
    }
    return u32Return;// for success
}
