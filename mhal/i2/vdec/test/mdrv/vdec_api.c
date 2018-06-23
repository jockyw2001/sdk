#include "MsOS.h"
#include "MsTypes.h"

#include "vdec_api.h"
//#include <vdec_ex_private.h>
//#include <apiVDEC_EX.h>
// VDEC_StreamId,VDEC_EX_User_Cmd and other struct used in this file are defined in apiVde_EX.h
// However, we will add some user cmd to support SetControl and GetControl functions


//-------------------------------------------------------------------------------------------------
#define VDEC_PRINT(format,args...) ULOGI("VDEC", format, ##args)

//#define _DEBUG
#ifdef _DEBUG
#ifdef MSOS_TYPE_LINUX_KERNEL
#define VPRINTF  printk
#else
#define VPRINTF(format,args...) ULOGI("VDEC", format, ##args)
#endif
#else
#define VPRINTF(format,args...)  {}
#endif

#if 0
VDEC_EX_Result mdrv_MApi_VDEC_EX_SysPreInit(void);
#endif
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
//#ifdef _USE_MDRV_VDEC_

#if defined(_USE_MDRV_VDEC_)
VDEC_EX_Result mdrv_MApi_VDEC_EX_GetFreeStream(VDEC_EX_GET_FREE_STREAM_PARAMS_t *param)
{
#if 0
    if (E_VDEC_EX_OK != mdrv_MApi_VDEC_EX_SysPreInit())
    {
       return E_VDEC_EX_FAIL;
    }
#endif

    return MApi_VDEC_EX_GetFreeStream(&param->streamId, param->u32Size, param->eStreamType, param->eCodecType);
}

//===========MApi_VDEC_EX_Init ========================
VDEC_EX_Result mdrv_MApi_VDEC_EX_Init(VDEC_StreamId *pStreamId, VDEC_EX_InitParam *pInitParam)
{
    return MApi_VDEC_EX_Init(pStreamId, pInitParam);
}

//==========MApi_VDEC_EX_PreSetControl=====================
VDEC_EX_Result mdrv_MApi_VDEC_EX_PreSetControl(VDEC_StreamId *pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_PRE_CONTROL_PARAMS_t *param)
{
  //return MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, MS_U32 param);
  VDEC_EX_Result eRet = E_VDEC_EX_OK;

  switch (cmd_id)
  {
      case E_VDEC_EX_USER_CMD_SET_DECODE_MODE:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_DECODE_MODE ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_SetControl(pStreamId, cmd_id, (MS_U32)&param->decodeMode.tDecModCfg);
                //MApi_VDEC_EX_SetControl((VDEC_StreamId *)m_pStreamId, E_VDEC_EX_USER_CMD_SET_DECODE_MODE, (MS_U32)&stDecModCfg);
           break;

       case E_VDEC_EX_USER_CMD_MFCODEC_MODE: //EVD
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_MFCODEC_MODE ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, (MS_U32)param->mfCodecMode.eMFCodecMode);
           break;

       case E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, param->mfCodecMode.eMFCodecMode);
           break;

      case E_VDEC_EX_USER_CMD_CONNECT_INPUT_TSP:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_CONNECT_INPUT_TSP ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, param->connectInputTsp.eInputTSP);
           break;

      case E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, (MS_U32)&param->connectDisplayPath.tDynmcDispPath);
           break;

      case E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, param->displayMode.eDispMode);
           break;

      case E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, (MS_U32)param->bitstreamBufferMonopoly.bMonopolyBitstreamBuffer);
           break;

      case E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, (MS_U32)param->frameBufferMonopoly.bMonopolyFrameBuffer);
           break;

      case E_VDEC_EX_USER_CMD_SET_BUFFER_INFO:
           //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_BUFFER_INFO ================ \n",__FUNCTION__);
           eRet = MApi_VDEC_EX_PreSetControl(pStreamId, cmd_id, (MS_U32)&param->bufferInfo.tBufInfo);
           break;

      default:
           VPRINTF("================[%s] E_VDEC_EX_RET_ILLEGAL_ACCESS ================ \n",__FUNCTION__);
           eRet = E_VDEC_EX_RET_ILLEGAL_ACCESS;
           break;
  }
  return eRet;
}

//==========MApi_VDEC_EX_SetControl=====================
VDEC_EX_Result mdrv_MApi_VDEC_EX_SetControl(VDEC_StreamId* pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_SET_CONTROL_PARAMS_t* param)
{
    VDEC_EX_Result ret = E_VDEC_EX_OK;
    switch (cmd_id)
    {
        case E_VDEC_EX_USER_CMD_AVSYNC_ON:
             //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_AVSYNC_ON ================ \n",__FUNCTION__);
             ret = MApi_VDEC_EX_AVSyncOn(pStreamId,
                                             param->avSyncOn.bOn,
                                             param->avSyncOn.u16SyncTolerance,
                                             param->avSyncOn.u32SyncDelay);
             break;

        case E_VDEC_EX_USER_CMD_SET_BLOCK_DISPLAY:
             //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_BLOCK_DISPLAY ================ \n",__FUNCTION__);
             ret = MApi_VDEC_EX_SetBlockDisplay(pStreamId, param->setBlockDisplay.bEnable);
             break;

        case E_VDEC_EX_USER_CMD_ENABLE_ES_BUFF_MALLOC:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_ENABLE_ES_BUFF_MALLOC ================ \n",__FUNCTION__);
            ret = MApi_VDEC_EX_EnableESBuffMalloc(pStreamId, param->enableEsBufMalloc.bEnable);
            break;
        case E_VDEC_EX_USER_CMD_DISABLE_DEBLOCKING:
             //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_DISABLE_DEBLOCKING ================ \n",__FUNCTION__);
                 ret = MApi_VDEC_EX_DisableDeblocking(pStreamId, param->setDisableDeblocking.bDisable);
              break;
        case E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE ================ \n",__FUNCTION__);
            ret =  MApi_VDEC_EX_SetControl(pStreamId, cmd_id, param->setDispOutsideCtrlMode.bEnable);
            break;

        case E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE ================ \n",__FUNCTION__);
            ret = MApi_VDEC_EX_SetControl(pStreamId, cmd_id, param->setDispFinishMode.bEnable);
            break;

        case E_VDEC_EX_USER_CMD_PLAY:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_PLAY ================ \n",__FUNCTION__);
            ret = MApi_VDEC_EX_Play(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_PUSH_DECQ:
             VPRINTF("================[%s] E_VDEC_EX_USER_CMD_PUSH_DECQ ================ \n",__FUNCTION__);
             ret = MApi_VDEC_EX_PushDecQ(pStreamId, &param->pushDecQ.cmd);
             break;

        case E_VDEC_EX_USER_CMD_FIRE_DEC:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_FIRE_DEC ================ \n",__FUNCTION__);
            ret = MApi_VDEC_EX_FireDecCmd(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_RELEASE_FRAME:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_RELEASE_FRAME ================ \n",__FUNCTION__);
            ret = MApi_VDEC_EX_ReleaseFrame(pStreamId, &param->releaseFrame.dispFrm);
            break;

        case E_VDEC_EX_USER_CMD_EXIT:
             VPRINTF("================[%s] E_VDEC_EX_USER_CMD_EXIT ================ \n",__FUNCTION__);
             ret = MApi_VDEC_EX_Exit(pStreamId);
             break;

        case E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER:
              VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER ================ \n",__FUNCTION__);
              ret = MApi_VDEC_EX_SetControl(pStreamId, cmd_id, param->showDecodeOrder.bEnable);
              break;

        case E_VDEC_EX_USER_CMD_SET_FW_DEBUG_LEVEL:
              VPRINTF("================[%s] E_VDEC_EX_USER_CMD_SET_FW_DEBUG_LEVEL ================ \n",__FUNCTION__);
              ret = MApi_VDEC_EX_SetControl(pStreamId, cmd_id, param->fwDebug.u32DebugLevel);
              break;

        default:
            VPRINTF("================[%s] E_VDEC_EX_RET_ILLEGAL_ACCESS ================ \n",__FUNCTION__);
            ret = E_VDEC_EX_RET_ILLEGAL_ACCESS;
            break;
    }

    return ret;
}

//=================== MApi_VDEC_EX_GetControl =========================
VDEC_EX_Result mdrv_MApi_VDEC_EX_GetControl(VDEC_StreamId* pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_GET_CONTROL_PARAMS_t* param)
{
    VDEC_EX_Result eRet = E_VDEC_EX_OK;

    switch (cmd_id)
    {
        case E_VDEC_EX_USER_CMD_GET_FRAME_CNT:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_FRAME_CNT ================ \n",__FUNCTION__);
            param->frameCnt.u32Cnt = MApi_VDEC_EX_GetFrameCnt(pStreamId);
            break;

          case E_VDEC_EX_USER_CMD_GET_ERR_CNT:
             //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ERR_CNT ================ \n",__FUNCTION__);
             param->errCnt.u32Cnt = MApi_VDEC_EX_GetErrCnt(pStreamId);
             break;

       case E_VDEC_EX_USER_CMD_GET_DROP_CNT:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_DROP_CNT ================ \n",__FUNCTION__);
            param->dropCnt.u32Cnt = MApi_VDEC_EX_GetDropCnt(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_GET_SKIP_CNT:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_SKIP_CNT ================ \n",__FUNCTION__);
            param->skipCnt.u32Cnt = MApi_VDEC_EX_GetSkipCnt(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_GET_ERR_CODE:
              //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ERR_CODE ================ \n",__FUNCTION__);
              param->errCode.eErrCode = MApi_VDEC_EX_GetErrCode(pStreamId);
              break;

        case E_VDEC_EX_USER_CMD_GET_DECQ_VACANCY:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_DECQ_VACANCY ================ \n",__FUNCTION__);
            param->decQVacancy.u32Vacancy = MApi_VDEC_EX_GetDecQVacancy(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_GET_ES_BUFF:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ES_BUFF ================ \n",__FUNCTION__);
            eRet = MApi_VDEC_EX_GetESBuff(pStreamId,
                                              param->esBuffer.u32ReqSize,
                                              &param->esBuffer.u32AvailSize,
                                              &param->esBuffer.u32Addr);
            break;

        case E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME:
        {
            VDEC_EX_DispFrame *pDispFrm;
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME ================ \n",__FUNCTION__);
            eRet = MApi_VDEC_EX_GetNextDispFrame(pStreamId, &pDispFrm);
            memcpy(&param->nextDispFrame.dispFrm, pDispFrm, sizeof(VDEC_EX_DispFrame));
            break;
        }
        case E_VDEC_EX_USER_CMD_GET_ACTIVE_CODEC_TYPE:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ACTIVE_CODEC_TYPE ================ \n",__FUNCTION__);
            param->getActiveCodecType.eCodecType = MApi_VDEC_EX_GetActiveCodecType(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_GET_ES_WRITE_PTR:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ES_WRITE_PTR ================ \n",__FUNCTION__);
            param->getESWritePtr.u32value = MApi_VDEC_EX_GetESWritePtr(pStreamId);
            break;
        case E_VDEC_EX_USER_CMD_GET_ES_READ_PTR:
            //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_ES_READ_PTR ================ \n",__FUNCTION__);
            param->getESReadPtr.u32value = MApi_VDEC_EX_GetESReadPtr(pStreamId);
            break;

        case E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT: //???
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT ================ \n",__FUNCTION__);
            eRet = MApi_VDEC_EX_GetControl(pStreamId, cmd_id, (MS_U32*)&param->getNextDispFrameInfoExt.sFrameInfoExt_v6);
            break;

        case E_VDEC_EX_USER_CMD_GET_DISP_INFO:
            VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_DISP_INFO ================ \n",__FUNCTION__);
            eRet = MApi_VDEC_EX_GetDispInfo(pStreamId, &param->getDispInfo.stDispInfo);
            break;

        case E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX:
             //VPRINTF("================[%s] E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX ================ \n",__FUNCTION__);
             eRet = MApi_VDEC_EX_GetControl(pStreamId, cmd_id, (MS_U32*)&param->getFpaSeiEx.stSEIex);
             break;

        default:
            VPRINTF("================[%s] E_VDEC_EX_RET_ILLEGAL_ACCESS ================ \n",__FUNCTION__);
            eRet = E_VDEC_EX_RET_ILLEGAL_ACCESS;
            break;
    }
    return eRet;
}
#endif /* _USE_MDRV_VDEC_ */

//================================================================================

/*MsOS.h */
/*#define     MSOS_CACHE_BOTH         0
#define     MSOS_NON_CACHE_BOTH     1
#define     MSOS_CACHE_USERSPACE    2
#define     MSOS_NON_CACHE_USERSPACE  3
#define     MSOS_CACHE_KERNEL       4
#define     MSOS_NON_CACHE_KERNEL   5
*/

#define VDEC_MIU 1
#define CPU_BUFFER_OFFSET       0x00000000
#define CPU_BUFFER_SIZE         0x00200000
#define VDEC_BUFFER_OFFSET      0x00200000
#define VDEC_BUFFER_SIZE        0x07F00000

#if 0
int _VDEC_DoMmap(void)
{
    int ret = TRUE;

    if(!MsOS_MPool_Mapping_Dynamic(VDEC_MIU, CPU_BUFFER_OFFSET, CPU_BUFFER_SIZE, MSOS_CACHE_BOTH))
     {
          VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(ret == TRUE && !MsOS_MPool_Mapping_Dynamic(VDEC_MIU, CPU_BUFFER_OFFSET, CPU_BUFFER_SIZE, MSOS_NON_CACHE_BOTH))
     {
          VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(!MsOS_MPool_Mapping_Dynamic(VDEC_MIU, VDEC_BUFFER_OFFSET, VDEC_BUFFER_SIZE, MSOS_CACHE_BOTH))
      {
           VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }

      if(ret == TRUE && !MsOS_MPool_Mapping_Dynamic(VDEC_MIU, VDEC_BUFFER_OFFSET, VDEC_BUFFER_SIZE, MSOS_NON_CACHE_BOTH))
      {
           VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }
      return ret;
}

//#endif /* _USE_MDRV_VDEC_ */


static int _gPreInit = 0;
VDEC_EX_Result mdrv_MApi_VDEC_EX_SysPreInit(void)
{
    VDEC_EX_Result eRet = E_VDEC_EX_FAIL;
    if (!_gPreInit)
    {
        //DriverInitBase();

#if 0
        if(MDrv_MMIO_Init() == FALSE)
        {
          VPRINTF("%s: MDrv_MMIO_Init fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        VDEC_PRINT("%s: MDrv_MMIO_Init done\n",__FUNCTION__);
#endif

        if(MsOS_Init() == FALSE)
        {
          VPRINTF("%s: MsOS_Init fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        VPRINTF("%s: MsOS_Init done\n",__FUNCTION__);

        if(MsOS_MPool_Init() == FALSE)
        {
          VPRINTF("%s: MsOS_MPool_Init fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        VPRINTF("%s: MsOS_MPool_Init done\n",__FUNCTION__);

        if(_VDEC_DoMmap() == FALSE)
        {
          VPRINTF("%s: _VDEC_DoMmap fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        _gPreInit = 1;
    }
    eRet = E_VDEC_EX_OK;

PreInit_FAIL:
    return eRet;
}

int _VDEC_DoMmap(void)
{
    int ret = TRUE;

    if(!MsOS_MPool_Mapping_Dynamic(VDEC_MIU, CPU_BUFFER_OFFSET, CPU_BUFFER_SIZE, MSOS_CACHE_BOTH))
     {
          VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(ret == TRUE && !MsOS_MPool_Mapping_Dynamic(VDEC_MIU, CPU_BUFFER_OFFSET, CPU_BUFFER_SIZE, MSOS_NON_CACHE_BOTH))
     {
          VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
          ret = FALSE;
     }

     if(!MsOS_MPool_Mapping_Dynamic(VDEC_MIU, VDEC_BUFFER_OFFSET, VDEC_BUFFER_SIZE, MSOS_CACHE_BOTH))
      {
           VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }

      if(ret == TRUE && !MsOS_MPool_Mapping_Dynamic(VDEC_MIU, VDEC_BUFFER_OFFSET, VDEC_BUFFER_SIZE, MSOS_NON_CACHE_BOTH))
      {
           VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
           ret = FALSE;
      }
      return ret;
}
#endif


static int _gPreInit = 0;
static VDEC_EX_Result _mdrv_MApi_VDEC_EX_SysPreInit(void)
{
    VDEC_EX_Result eRet = E_VDEC_EX_FAIL;
    if (!_gPreInit)
    {
        if(MsOS_Init() == FALSE)
        {
            VPRINTF("%s: MsOS_Init fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        VPRINTF("%s: MsOS_Init done\n",__FUNCTION__);

        if(MsOS_MPool_Init() == FALSE)
        {
            VPRINTF("%s: MsOS_MPool_Init fail\n",__FUNCTION__);
            goto PreInit_FAIL;
        }
        VPRINTF("%s: MsOS_MPool_Init done\n",__FUNCTION__);

        _gPreInit = 1;
    }
    eRet = E_VDEC_EX_OK;

PreInit_FAIL:
    return eRet;
}

VDEC_EX_Result mdrv_MApi_VDEC_EX_DoMmap(VDEC_MAPINFO_t *ptMap)
{
    _mdrv_MApi_VDEC_EX_SysPreInit();

   /* if(!MsOS_MPool_Mapping_Dynamic(ptMap->u8MiuSel, ptMap->u32Offset, ptMap->u32MapSize, MSOS_CACHE_BOTH))
   {
        VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
        return E_VDEC_EX_FAIL;
   }*/
#ifdef CHIP_K6
   if(!MsOS_MPool_Mapping_Dynamic(ptMap->u8MiuSel, ptMap->u32Offset, ptMap->u32MapSize, MSOS_NON_CACHE_BOTH))
   {
        VPRINTF("MsOS_MPool_Mapping_Dynamic fail!\n");
        return E_VDEC_EX_FAIL;
   }
#else
   if (!MsOS_MPool_Add_PA2VARange((MS_U64)ptMap->u32Offset, (MS_VIRT)ptMap->u32KvirtAddr, (MS_SIZE)ptMap->u32MapSize, MSOS_NON_CACHE_BOTH))
   {
       VPRINTF("MsOS_MPool_Add_PA2VARange failed: %x,%x,%x!\n",(MS_U32)ptMap->u32Offset, (MS_U32)ptMap->u32KvirtAddr, (MS_U32)ptMap->u32MapSize);
       return E_VDEC_EX_FAIL;
   }
#endif

   return E_VDEC_EX_OK;
}

VDEC_EX_Result mdrv_MApi_VDEC_EX_DoUnMap(VDEC_UNMAPINFO_t *ptMap)
{
#ifdef CHIP_K6
    if(!MsOS_MPool_UnMapping(MsOS_MPool_PA2KSEG1(ptMap->u32phyStart), ptMap->u32MapSize))    // for non-cached
    {
        VPRINTF("mdrv_MApi_VDEC_EX_DoUnMap fail!\n");
        return E_VDEC_EX_FAIL;
    }
#else
    //if (!MsOS_MPool_Remove_PA2VARange((MS_U64)MsOS_MPool_PA2KSEG1(ptMap->u32phyStart), (MS_VIRT)ptMap->u32KvirtAddr, (MS_SIZE)ptMap->u32MapSize, MSOS_NON_CACHE_BOTH))
    if (!MsOS_MPool_Remove_PA2VARange((MS_U64)ptMap->u32phyStart, (MS_VIRT)ptMap->u32KvirtAddr, (MS_SIZE)ptMap->u32MapSize, MSOS_NON_CACHE_BOTH))
    {
        VPRINTF("MsOS_MPool_Remove_PA2VARange failed: %x,%x,%x!\n",MsOS_MPool_PA2KSEG1(ptMap->u32phyStart), (MS_U32)ptMap->u32KvirtAddr, (MS_U32)ptMap->u32MapSize);
        return E_VDEC_EX_FAIL;
    }
#endif

    return E_VDEC_EX_OK;
}
