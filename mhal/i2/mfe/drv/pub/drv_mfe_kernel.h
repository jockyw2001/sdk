
#ifndef _MFE_KERNEL_H_
#define _MFE_KERNEL_H_

#if defined(__KERNEL__) || defined(CAM_OS_LINUX_USER)
#include <linux/clk-provider.h>
#include <ms_platform.h>
#endif // if defined(__KERNEL__)

#include "mhve_ios.h"
#include "mhve_ops.h"
#include "mhve_rqct_ops.h"
#include "cam_os_wrapper.h"

#define MMFE_TIMER_SIZE         2048

#if defined(__I_SW__)
typedef struct
{
    ClkgenModule_e eModule;
    ClkgenClkReq_e eClkReq;
    ClkgenClkUpdate_e eClk;
    ClkgenClkSpeed_e eClkSpeed;
    U16 nClkSrc;
} DrvMfeClk_t;
#endif

typedef struct MfeCtx_t mmfe_ctx;
typedef struct MfeRqc_t mmfe_rqc;
typedef struct MfeDev_t mmfe_dev;

typedef struct
{
    CamOsTimespec_t rc_calc_start;
    u64             total_byte;
    u64             total_frame;
    u32             kbps;
    u32             fps_x100;
}MfeCtxRC_t;

struct MfeCtx_t
{
    CamOsAtomic_t   i_refcnt;
    void (*release) (void* );
    void (*adduser) (void* );
    CamOsMutex_t    m_stream;       /* OS Mutex */
    mmfe_dev*       p_device;       /* Pointer to DRV mmfe_dev structure */
    void*           p_handle;       /* Pointer to HAL mhve_ops structure */
#define MMFE_CTX_STATE_NULL  -1
#define MMFE_CTX_STATE_IDLE   0
#define MMFE_CTX_STATE_BUSY   1
    int             i_state;        /* Context State */
    int             i_score;        /* Equal to MB_num x FPS */
    int             i_index;        /* Stream Context ID */
    /* direct memory resource */
    short           i_picture_w;
    short           i_picture_h;
    short           i_cropw ;
    short           i_croph;
    short           i_cropoffsetx;
    short           i_cropoffsety;
    /* user data buffer */
#define MMFE_SEI_MAX_LEN     (1072)
#define MMFE_USER_DATA_SIZE  (4*MMFE_SEI_MAX_LEN)
    unsigned char*  p_usrdt;        /* User Data Buffer Address */
    short           i_usrsz;        /* User Data Buffer Free Size */
    short           i_usrcn;        /* User Data Count */
    /* user mode output */
    unsigned char*  p_ovptr;        /* Output Bitstream Buffer Address (Virtual) */
    unsigned int    u_ophys;        /* Output Bitstream Buffer Address (Physical Across MIU) */
    int             i_osize;        /* Output Bitstream Buffer Size */
    int             i_ormdr;        /* Output Bitstream Buffer Remain Data Size */
    /* output cpbs */
#define MMFE_CPBS_NR          1
    mhve_cpb        m_mcpbs[MMFE_CPBS_NR];
#define MMFE_OMODE_MMAP       0
#define MMFE_OMODE_USER       1
#define MMFE_OMODE_PIPE       2
    short           i_omode;
#define MMFE_IMODE_PURE       0
#define MMFE_IMODE_PLUS       1
    short           i_imode;
    /* direct memory resource */
#define MMFE_DMEM_NR          4
    int             i_dmems;        /* Direct Memory Buffer Count (Output bitstream, Ref/Reconstruct...) */
    char*           m_dmems[MMFE_DMEM_NR];  /* Direct Memory Buffer pointer */
    int             s_dmems[MMFE_DMEM_NR];  /* Direct Memory Buffer Size */
    /* internal direct memory resource */
    unsigned char*  p_ialva;          /* Internal Virtual Buffer Address for algorithm(ROI,PMBR) */
    unsigned char*  p_ialma;          /* Internal Miu Buffer Address for algorithm(ROI,PMBR) */
    unsigned int    i_ialsz;          /* Internal Buffer Size for algorithm */
    unsigned char*  p_ircma;          /* Internal Ref/Reconstruct Miu Buffer Address */
    unsigned int    i_ircms;          /* Internal Ref/Reconstruct Buffer Size */
#if MMFE_TIMER_SIZE
    int             i_numbr;
    struct {
    unsigned char   tm_dur[4];
    unsigned int    tm_cycles;
    } *p_timer;
#endif
#ifdef SUPPORT_CMDQ_SERVICE
    unsigned int    p_regcmd[5120];
#endif
    unsigned int    i_enccnt;         /* Total Coded Frames Count */

    MfeCtxRC_t      rc_stat[5];
};

struct MfeRqc_t
{
    mmfe_ctx*       p_mctx;         /* Context of RQCT */
};

struct MfeDev_t
{
#define MMFE_CLOCKS_NR        4
#if defined(__KERNEL__) || defined(CAM_OS_LINUX_USER)
    struct clk*     p_clocks[MMFE_CLOCKS_NR];
#else
    DrvMfeClk_t*    p_clocks[MMFE_CLOCKS_NR];
#endif
    CamOsMutex_t    m_mutex;        /* OS Mutex */
    CamOsTsem_t     m_wqh;          /* OS Flag */
    CamOsTsem_t     tGetBitsSem;
#define MMFE_STREAM_NR        16
    mmfe_rqc        m_regrqc[MMFE_STREAM_NR]; /* RQCT (Stream) Context */
    /* asic */
    void*           p_asicip;       /* Pointer to HAL mhve_ios structure */
#define MMFE_DEV_STATE_NULL  -1
#define MMFE_DEV_STATE_IDLE   0
#define MMFE_DEV_STATE_BUSY   1
    int             i_state;        /* Device State */
    int             i_users;        /* Not Used */
    int             i_score;        /* Not Used */
    /* statistic */
    int             i_counts[MMFE_STREAM_NR][5]; /* Time Statistic Data */
    int             i_thresh;       /* Not Used */
    /* clock */
    int             i_clkidx;
    int             i_ratehz;
    /* rate-controller */
    int             i_rctidx;       /* RQCT Factory ID */
    /* irq */
    int             i_irq;
    /* register base */
    void*           p_reg_base;

    CamOsTimespec_t utilization_calc_start;
    CamOsTimespec_t utilization_calc_end;
    CamOsTimespec_t encode_start_time;
    u64             encode_total_ns0;
    u64             encode_total_ns1;
    u32             i_utilization_percentage;
};

#endif //_MFE_KERNEL_H_
