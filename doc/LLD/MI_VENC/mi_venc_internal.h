
#ifndef _MI_VENC_H_
#error This header file should be on included by other headers like mi_venc.h
#endif

#ifndef _MI_VENC_MISS_H_
#define _MI_VENC_MISS_H_

#include "sys/mi_sys_datatype.h"

#define U8    unsigned char
#define U16   unsigned short
#define U32   unsigned long
#define U64   unsigned long long
#define S8    signed char
#define S16   signed short
#define S32   signed long
#define S64   signed long long
#define BOOL  unsigned char


//later should be from hardware
#define PORT_BUSY (0)
#define READ_REG(r) (0)
#define dump_reg_VPE_FENCE 0x01
#define dump_reg_VENC_FENCE 0x02
#define dump_reg_CMDQ_H265_0 0x10
#define dump_reg_CMDQ_H265_1 0x11
#define dump_reg_CMDQ_H264_0 0x12

#define VPE_ISR_IDX 0x01
#define VENC_ISR_IDX 0x02

#define CMDQ_EVENT_ISP_SC_ISR 0x01
#define CMDQ_EVENT_VENC_SC_ISR 0x02

typedef void* MI_VOID;
//+++ missing type
typedef MI_VENC_H264eNaluType_e MI_VENC_Mpeg4ePackType_e;
typedef MI_VENC_StreamInfoH264_t MI_VENC_StreamInfoMpeg4_t;
typedef MI_VENC_H265eNaulType_e MI_VENC_H264eRefType_e, H265E_REF_TYPE_E;
//--- missing type

//+++ missing type
typedef MI_VENC_AttrH264_t MI_VENC_AttrMpeg4_t;
typedef MI_VENC_ModType_e MI_VENC_PayloadType_e;
//--- missing type

//+++ missing type
typedef MI_VENC_ParamH264Cbr_t MI_VENC_ParamMjpegCbr_t, MI_VENC_ParamMpeg4Cbr_t;
typedef MI_VENC_ParamH264Vbr_t, MI_VENC_ParamMjpegVbr_t, MI_VENC_ParamMpeg4Vbr_t;
//---

//+++ missing type
typedef MI_VENC_ParamModH265e_t MI_VENC_ParamModH264e_t, MI_VENC_ParamModVenc_t;
typedef MI_U8 VENC_CHN;
//--- missing type

//+++ missing type
typedef mi_sys_ChnBufInfo_t hal_Ch_Info;

typedef struct RateControl {
    int a;
} RateControl;

typedef struct ParamH264VuiAspectRatio {
    int a;
} ParamH264VuiAspectRatio;

typedef struct ParamH264VuiTimeInfo {
    int a;
} ParamH264VuiTimeInfo;

//--- missing type

typedef struct
{
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32Stride;
    MI_SYS_PixelFormat_e eFormat;
}MI_SYS_ChnInputPortCfgInfo_t;

typedef struct RECT_S {
    MI_U32 u32Left, u32Top, u32Width, u32Height;
} RECT_S;

#if 0
typedef enum MI_RESULT {
    MI_OK,
    //MI_SUCCESS,
    MI_ERR_FAILED,
    MI_ERR_BUTT
} MI_RESULT;
#endif

typedef enum MI_BOOL {
    MI_FALSE,
    MI_TRUE
} MI_BOOL;

//from fakecode, mi_sys_internal.h
#if 0
typedef enum
{
     // driver accept current task and continue iterator
     MI_SYS_ITERATOR_ACCEPT_CONTINUTE,
     // driver reject current task and continue iterator,
     MI_SYS_ITERATOR_SKIP_CONTINUTE,
     // driver accept current task and ask to stop iterator
     MI_SYS_ITERATOR_ACCEPT_STOP,
     // driver accept reject task and ask to stop iterator
     MI_SYS_ITERATOR_SKIP_STOP
}MI_TaskIteratorCBAction;
#endif

typedef struct
{
   MI_RESULT (*OnBindChn)(MI_SYS_ChnInputPortCfgInfo_t *pstChnPortCfgInfo);
   MI_RESULT (*OnUnBindChn)(void);
}mi_sys_ModuleBindOps_t;

#if 0
typedef struct
{
    MI_U32 u32ModuleId;
    MI_U32 u32DevId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    MI_U32 u32DevChnNum;
    MI_U8 au8InputPortName[MI_SYS_MAX_INPUT_PORT_CNT][MI_SYS_PORT_ALIAS_NAME_MAX_LENGTH];
    MI_U8 au8OutputPortName[MI_SYS_MAX_OUTPUT_PORT_CNT][MI_SYS_PORT_ALIAS_NAME_MAX_LENGTH];
}mi_sys_ModuleInfo_t;
#else
typedef MI_SYS_MOD_DEV_t mi_sys_ModuleInfo_t;
#endif
//typedef MI_TaskIteratorCBAction (* MI_SYS_TaskIteratorCallBK)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);


#endif
