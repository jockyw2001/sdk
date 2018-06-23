#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        printf("[%d]exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("(%d)exec function pass\n", __LINE__);\
    }

#ifdef MI_INFINITY3_ENABLE
#include "mi_common.h"
#include "mi_vi.h"
#include "mi_venc.h"
#include "mi_sys.h"

#define MI_SUCCESS 0

typedef char MI_S8;

typedef struct{
    VideoPlayLoadType_e enType;
    U32 Width;
    U32 Height;
    double FrameRate;
    U32 Chn;
} VideoParam;

typedef struct {
      VideoPlayLoadType_e enType;
      ViDevAttr_t devAttr;
      VencChnAttr_t vencAttr;
      ViChnAttr_t viChnAttr;
      Chn_t SrcChn, DestChn;
      VideoParam Param;
}VideoSet;

extern U32 MI_OS_GetTime();
void video_device_prepareSet(VideoSet *Set,VideoParam Param);
void video_device_init(VideoSet *Set);
void video_device_setAttr(VideoSet *Set);
void video_device_destroyAttr(VideoSet *Set);
U32 video_get_frame(VideoSet *Set,U32 *length,void *buf);
void video_device_exit(VideoSet *Set);
void time_get_interval(char *s);
#else //MI_INFINITY2_ENABLE
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_shadow.h"
#endif
