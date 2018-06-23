#ifndef _VDEC_API_H_
#define _VDEC_API_H_

#include <apiVDEC_EX.h>
//#include "../include/apiVDEC_EX.h"
// VDEC_StreamId,VDEC_EX_User_Cmd and other struct used in this file are defined in apiVde_EX.h
// However, we will add some user cmd to support SetControl and GetControl functions



//============ MApi_VDEC_EX_GetFreeStream ====================
typedef struct _VDEC_EX_GET_FREE_STREAM_PARAMS_t{
	VDEC_StreamId	  streamId;
	MS_U32 		  u32Size;
	VDEC_EX_Stream 	  eStreamType;
	VDEC_EX_CodecType eCodecType;
} VDEC_EX_GET_FREE_STREAM_PARAMS_t;

VDEC_EX_Result mdrv_MApi_VDEC_EX_GetFreeStream(VDEC_EX_GET_FREE_STREAM_PARAMS_t *param);


//===========MApi_VDEC_EX_Init ========================
//VDEC_EX_InitParam defined in apiVDEC_EX.h
VDEC_EX_Result mdrv_MApi_VDEC_EX_Init(VDEC_StreamId *pStreamId, VDEC_EX_InitParam *pInitParam);


//==========MApi_VDEC_EX_PreSetControl=====================
typedef struct _VDEC_PRE_CONTROL_PARAMS_t {
    union {
      //cmd_id: E_VDEC_EX_USER_CMD_SET_DECODE_MODE
      struct {
          VDEC_EX_DecModCfg tDecModCfg;
      } decodeMode;

      //cmd_id: E_VDEC_EX_USER_CMD_CONNECT_INPUT_TSP
      struct {
          VDEC_EX_INPUT_TSP 	eInputTSP;
      } connectInputTsp;

      //cmd_id: E_VDEC_EX_USER_CMD_CONNECT_DISPLAY_PATH
      struct  {
          VDEC_EX_DynmcDispPath tDynmcDispPath;
      } connectDisplayPath;

      //cmd_id: E_VDEC_EX_USER_CMD_SET_DISPLAY_MODE
      struct  {
          VDEC_EX_DISPLAY_MODE eDispMode;
      } displayMode;

      //cmd_id: E_VDEC_EX_USER_CMD_BITSTREAMBUFFER_MONOPOLY
      struct  {
          MS_BOOL  bMonopolyBitstreamBuffer;
      } bitstreamBufferMonopoly;

      //cmd_id: E_VDEC_EX_USER_CMD_FRAMEBUFFER_MONOPOLY
      struct  {
          MS_BOOL bMonopolyFrameBuffer;
      } frameBufferMonopoly;

      //cmd_id: E_VDEC_EX_USER_CMD_DYNAMIC_CMA_MODE
      struct VDEC_PARAM_DYNAMIC_CMA_MODE {
            MS_BOOL       bEnableDynamicCMA;
        } dynamicCMAMode;

      //cmd_id: E_VDEC_EX_USER_CMD_MFCODEC_MODE
      struct  {
          VDEC_EX_MFCodec_mode eMFCodecMode;
      } mfCodecMode;

      //cmd_id: E_VDEC_EX_USER_CMD_SET_BUFFER_INFO
       struct {
          VDEC_EX_BufferInfo tBufInfo;
       } bufferInfo;

    };
} VDEC_PRE_CONTROL_PARAMS_t;

VDEC_EX_Result mdrv_MApi_VDEC_EX_PreSetControl(VDEC_StreamId *pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_PRE_CONTROL_PARAMS_t *param);

//==========MApi_VDEC_EX_SetControl=====================
typedef struct _VDEC_SET_CONTROL_PARAMS_t {
    union {
        // user_cmd: E_VDEC_EX_USER_CMD_AVSYNC_ON (new)
        struct  {
            MS_BOOL bOn;
            MS_U32 u32SyncDelay;
            MS_U16 u16SyncTolerance;
        } avSyncOn;

        // user_cmd: E_VDEC_EX_USER_CMD_SET_BLOCK_DISPLAY (new)
        struct  {
            MS_BOOL bEnable;
        } setBlockDisplay;

        // user_cmd: E_VDEC_EX_USER_CMD_ENABLE_ES_BUFF_MALLOC (new)
        struct  {
            MS_BOOL bEnable;
        } enableEsBufMalloc;

        // user_cmd: E_VDEC_EX_USER_CMD_DISABLE_DEBLOCKING (new)
        struct  {
            MS_BOOL bDisable;
        } setDisableDeblocking;

        // user_cmd: E_VDEC_EX_USER_CMD_SET_DISP_OUTSIDE_CTRL_MODE
        struct  {
            MS_BOOL bEnable;
        } setDispOutsideCtrlMode;

        // user_cmd: E_VDEC_EX_USER_CMD_SET_DISP_FINISH_MODE
        struct  {
            MS_BOOL bEnable;
        } setDispFinishMode;

        // user_cmd: E_VDEC_EX_USER_CMD_PLAY (no parameter needed) (new)
        struct  {

        } play;

        // user_cmd: E_VDEC_EX_USER_CMD_PUSH_DECQ (new)
        struct  {
            VDEC_EX_DecCmd cmd;
        } pushDecQ;

        // user_cmd: E_VDEC_EX_USER_CMD_FIRE_DEC (no parameter needed) (new)
        struct  {

        } fireDecCmd;

        // user_cmd: E_VDEC_EX_USER_CMD_RELEASE_FRAME (new)
        struct  {
	          VDEC_EX_DispFrame dispFrm;
        } releaseFrame;

        // user_cmd: E_VDEC_EX_USER_CMD_EXIT (new)
        struct  {

        } exit;

        // user_cmd: E_VDEC_EX_USER_CMD_SHOW_DECODE_ORDER
        struct  {
            MS_BOOL bEnable;
        } showDecodeOrder;

        // user_cmd: E_VDEC_EX_USER_CMD_SET_FW_DEBUG_LEVEL
        struct  {
            MS_U32 u32DebugLevel;
        } fwDebug;

    };
} VDEC_SET_CONTROL_PARAMS_t;

VDEC_EX_Result mdrv_MApi_VDEC_EX_SetControl(VDEC_StreamId* pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_SET_CONTROL_PARAMS_t* param);

//=================== MApi_VDEC_EX_GetControl =========================
typedef struct _VDEC_GET_CONTROL_PARAMS_t {

    union {
      //user_cmd: E_VDEC_EX_USER_CMD_GET_FRAME_CNT (new)
      struct  {
          MS_U32 u32Cnt;
      } frameCnt;

      // user_cmd: E_VDEC_EX_USER_CMD_GET_ERR_CNT (new)
      struct  {
           MS_U32 u32Cnt;
       } errCnt;

       //user_cmd: E_VDEC_EX_USER_CMD_GET_DROP_CNT (new)
      struct  {
          MS_U32 u32Cnt;
      } dropCnt;

      //user_cmd: E_VDEC_EX_USER_CMD_GET_SKIP_CNT (new)
      struct  {
          MS_U32 u32Cnt;
      } skipCnt;

     //user_cmd: E_VDEC_EX_USER_CMD_GET_ERR_CODE
      struct  {
          VDEC_EX_ErrCode eErrCode;
      } errCode;

      // user_cmd: E_VDEC_EX_USER_CMD_GET_DECQ_VACANCY (new)
      struct  {
          MS_U32 u32Vacancy;
      } decQVacancy;

      //E_VDEC_EX_USER_CMD_GET_ES_BUFF (new)
      struct  {
          MS_U32 u32ReqSize;     // {IN]
          MS_U32 u32AvailSize;   // [OUT]
          MS_PHY u32Addr;        // [OUT]
      } esBuffer;

      // user_cmd: E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME (new)
      struct {
        VDEC_EX_DispFrame dispFrm;
      } nextDispFrame;

        // user_md: E_VDEC_EX_USER_CMD_GET_ACTIVE_CODEC_TYPE (new)
        struct  {
            VDEC_EX_CodecType eCodecType;
        } getActiveCodecType;

        // user_cmd: E_VDEC_EX_USER_CMD_GET_ES_READ_PTR (new)
        struct  {
            MS_U32 u32value;
        } getESReadPtr;

        // user_cmd: E_VDEC_EX_USER_CMD_GET_ES_WRITE_PTR (new)
        struct  {
            MS_U32 u32value;
        } getESWritePtr;

        // user_cmd: E_VDEC_EX_USER_CMD_GET_NEXT_DISP_FRAME_INFO_EXT
        struct  {
          union
          {
             VDEC_EX_FrameInfoExt    sFrameInfoExt;
             VDEC_EX_FrameInfoExt_v2 sFrameInfoExt_v2;
             VDEC_EX_FrameInfoExt_v3 sFrameInfoExt_v3;
             VDEC_EX_FrameInfoExt_v4 sFrameInfoExt_v4;
             VDEC_EX_FrameInfoExt_v5 sFrameInfoExt_v5;
             VDEC_EX_FrameInfoExt_v6 sFrameInfoExt_v6;
          };
        } getNextDispFrameInfoExt;

        // user_cmd: E_VDEC_EX_USER_CMD_GET_DISP_INFO (new)
        struct  {
            VDEC_EX_DispInfo stDispInfo;
        } getDispInfo;

        // user_cmd: E_VDEC_EX_USER_CMD_GET_FPA_SEI_EX
        struct  {
            VDEC_EX_Frame_packing_SEI_EX stSEIex;
        } getFpaSeiEx;

    };
} VDEC_GET_CONTROL_PARAMS_t;


typedef struct
{
    MS_U8   u8MiuSel;
    MS_SIZE u32Offset;
    MS_SIZE u32MapSize;
    MS_SIZE u32KvirtAddr;
} VDEC_MAPINFO_t;

typedef struct
{
    MS_SIZE u32phyStart;
    MS_SIZE u32MapSize;
    MS_SIZE u32KvirtAddr;
} VDEC_UNMAPINFO_t;

typedef enum
{
    E_VDEC_R2EVDCLK_480M_480M_480M = 0,
    E_VDEC_R2EVDCLK_480M_480M_576M,
    E_VDEC_R2EVDCLK_480M_576M_576M,
    E_VDEC_R2EVDCLK_576M_480M_480M,
    E_VDEC_R2EVDCLK_576M_480M_576M,
    E_VDEC_R2EVDCLK_576M_576M_576M
} VDEC_R2EVDCLK_e;


VDEC_EX_Result mdrv_MApi_VDEC_EX_GetControl(VDEC_StreamId* pStreamId, VDEC_EX_User_Cmd cmd_id, VDEC_GET_CONTROL_PARAMS_t* param);

VDEC_EX_Result mdrv_MApi_VDEC_EX_DoMmap(VDEC_MAPINFO_t *ptMap);
VDEC_EX_Result mdrv_MApi_VDEC_EX_DoUnMap(VDEC_UNMAPINFO_t *ptMap);

#endif /* _VDEC_API_H */
