#ifndef _VDEC_TEST_H_
#define _VDEC_TEST_H_

#include <apiVDEC.h>


#define MAX_CHANNEL_NUM		32
#define MAX_PATH            256

typedef enum
{
    E_VDEC_MD5_BYFRAME_VERIFY = 0,
    E_VDEC_MD5_BATCH_VERIFY,        /* not support now */
    E_VDEC_MD5_GEN,
    E_VDEC_MD5_NOT_VERIFY,
} VDEC_TEST_MD5_MODE;

typedef struct {
     VDEC_EX_CodecType eCodecType;
     MS_U32            u32BufSize;
     char              fileName[MAX_PATH];
} VDEC_TEST_CHANNEL_CFG;

typedef struct {
    int                 nSingleFrameMode;
    int                 nPerfTestMode;
    MS_U32              u8NumOfVdecChannel;
    int                 nEnableMFCodec;
    MS_U32              u32AutoMode;
    MS_U32              u32MaxFrameNum;
    MS_U32              u32DumpFrameNum;
    MS_U32              u32Md5FailGo;
    VDEC_TEST_MD5_MODE  eMd5Mode;
    VDEC_EX_DbgLevel    eDbgLevel;
    VDEC_EX_DbgLevel    eUtDbgLevel;
    MS_U32              u32TotalVdecBufSize;
    MS_U32              u32IdNum;
    MS_U32              u32VdecBufSize;
    VDEC_TEST_CHANNEL_CFG    tChannelCfg[MAX_CHANNEL_NUM];
} VDEC_TEST_CFG;


extern int Vdec_Test_Init(VDEC_TEST_CFG *ptCfg);
extern int Vdec_Test_Exit(void);
extern int Vdec_Test_Run(void);

#if 0
extern int     gSingleFrameMode;
MS_U32         gu8NumOfVdecChannel = 1;
int            gEnableMFCodec = FALSE;
VDEC_EX_DbgLevel  _gDbgLevel = E_VDEC_EX_DBG_LEVEL_ERR;

int              _gnMinPushNum[MAX_CHANNEL_NUM] = { 0 };
VDEC_EX_CodecType _geCodecType[MAX_CHANNEL_NUM] = {E_VDEC_EX_CODEC_TYPE_H264};
char             _gFileName[MAX_CHANNEL_NUM][MAX_PATH] = { 0 };
#endif

#endif   /* _VDEC_TEST_H_ */
