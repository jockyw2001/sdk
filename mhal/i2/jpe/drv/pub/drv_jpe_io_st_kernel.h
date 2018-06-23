////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
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

#ifndef __JPE_DRV_IO_ST_KERNEL_H__
#define __JPE_DRV_IO_ST_KERNEL_H__

#include "drv_jpe_io_st.h"

#if defined(__KERNEL__)
#include <asm/types.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/interrupt.h>

#elif defined(__I_SW__)
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_mem.h"
#include "sys_MsWrapper_cus_os_int_ctrl.h"
#include "sys_MsWrapper_cus_os_int_pub.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_rtk_vmrtkho.h"
#include "sys_sys_isw_uart.h"
#include "hal_int_ctrl_pub.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "vm_types.ht"

#endif // if defined(__KERNEL__)

#define JPE_STREAM_NR       16

#define JPE_CLOCK_ON    1
#define JPE_CLOCK_OFF   0

typedef struct JpeDev_t JpeDev_t;
typedef struct JpeCtx_t JpeCtx_t;

struct JpeCtx_t
{
    void (*release)(void*);
    CamOsTsem_t         m_stream;

    // TODO: is it possible to ignore below 2 arguments?
    JpeCfg_t            tJpeCfg;
    JpeEncOutbuf_t      tEncOutBuf;

    JpeDev_t*       p_device;
    void*           p_handle;
    int             i_state;
    int             i_index;

    unsigned short      nClkSelect;

    /* user data buffer */
    unsigned char*  p_usrdt;
    unsigned int    nHeaderLen;

    /* output buffer size threshold */
    unsigned int i_threshold;
};

typedef struct JpeDevUser_t
{
    struct JpeCtx_t*    pCtx;
} JpeDevUser_t;


struct JpeDev_t
{
#if defined(__KERNEL__)
    struct platform_device  *pPlatformDev;  // Platform device
    struct cdev             tCharDev;       // Character device
    struct device           tDevice;
    struct device           *pDevice;
#define JPE_CLOCKS_NR        4
    struct clk              *pClock[JPE_CLOCKS_NR];
    int                     nClockIdx;
    int                     nClockRate;
#endif
    int                     nRefCount;      // Reference count, how many file instances opened
    unsigned int            irq;            // IRQ number

    CamOsMutex_t            m_mutex;
    CamOsTcond_t            m_wqh;
    CamOsTsem_t             tGetBitsSem;

    JpeDevUser_t            user[JPE_STREAM_NR];

    /* ... */
    void*   p_asicip;

    int     i_state;
    int     i_users;
    int     i_usersidx;         // which instant lock dev
    /* statistic */
    int     i_counts[JPE_STREAM_NR][5];
    int     i_thresh;

};

#endif // __JPE_DRV_IO_ST_KERNEL_H__
