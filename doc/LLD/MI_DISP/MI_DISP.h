#ifndef __MI_DISP_HH__
#define __MI_DISP_HH__
#define MI_DISP_VIDEO_LAYER_INPUT_PORT_MAX 16
#define  MI_DISP_VIDEO_LAYER_MAX 2

typedef struct _MI_DISP_REGION_s
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 w;
    MI_U32 h;
}MI_DISP_REGION_t;
typedef  struct _MI_DISP_LAYER_Param_s
{
   MI_BOOL  bLayerEnabled;
   MI_U32     u32LayerWidth;
   MI_U32     u32LayerHeight;

   MI_DISP_REGION_t  stlayerCropRegionOnDeviceSrcTiming; 
 
   MI_BOOL  bPortEnabled[MI_DISP_LAYER_INPUT_PORT_MAX];
   MI_DISP_REGION_t  stPortWinSrcRegion[MI_DISP_LAYER_INPUT_PORT_MAX];

   struct mutex stDispLayerPendingQueueMutex;

   list_head   port_pending_buf_queue[MI_DISP_LAYER_INPUT_PORT_MAX];
   
   MI_SYS_BufInfo_t *pstOnScreenBufInfo[MI_DISP_LAYER_INPUT_PORT_MAX];
   MI_SYS_BufInfo_t *pstCurrentFiredBufInfo[MI_DISP_LAYER_INPUT_PORT_MAX];

   MI_DISP_SyncMode_e ePort_Sync_Mode[MI_DISP_LAYER_INPUT_PORT_MAX];
   MI_U64     u64Last_Flipped_Buf_PTS[MI_DISP_LAYER_INPUT_PORT_MAX];
   MI_U64     last_VSYNC_SYS_Mono_Time_in_NS;
}MI_DISP_LAYER_Param_t;

typedef struct _MI_DISP_DEV_Param_s
{
   MI_BOOL bDISPEnabled;
   wait_queue_head_t  stWaitQueueHead;
   MI_U32 u32DevId;
   MI_DISP_LAYER_Param_t *stBindedVideoLayer[MI_DISP_VIDEO_LAYER_MAX];

   MI_U32 u32SrcW;
   MI_U32 u32SrcH;
   MI_U32 u32SrcFps;

   //.... dst timing
}MI_DISP_DEV_Param_t;

int MI_DISP_Init();
#endif
