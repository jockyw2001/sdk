///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2016-2017 MStar Semiconductor, Inc.
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
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// @file   mdrv_graphic_fbdev.c
// @brief  Graphic Driver Interface
// @author MStar Semiconductor Inc.
//////////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Include Files
//=============================================================================
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/pfn.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/uaccess.h>

//drver header files
#include "mdrv_graphic_adapter.h"
#include "mdrv_graphic_fbdev.h"
#include "mstar_chip.h"
//mi header file get mmap info
#include "mi_syscfg_datatype.h"
#include "mi_syscfg.h"
#include "mi_sys_internal.h"
#include "mi_sys_proc_fs_internal.h"

//ini parser.h
#include "iniparser.h"
#include "mstar/mstarFb.h"

#define ALIGN_DOWNTO_16(_val_) (((_val_) >> 4) << 4)
#define MIU0_BUS_OFFSET   ARM_MIU0_BUS_BASE
#define MIU1_BUS_OFFSET   ARM_MIU1_BUS_BASE
#define MIU2_BUS_OFFSET   ARM_MIU2_BUS_BASE
#define MIU1_INTERVAL     ARM_MIU0_BASE_ADDR
#define MIU2_INTERVAL     ARM_MIU1_BASE_ADDR
#define CHECK_IFNOT(exp, dst, ret) if ((exp) != (dst)) { return ret; }

#define HWCURSOR_BUF_CNT 2
#define HWCURSOR_MAX_WIDTH 128
#define HWCURSOR_MAX_HEIGHT 128
#define MAX_HWLAYER_CNT 5

#define ASCII_COLOR_CYAN                         "\033[1;36m"
#define ASCII_COLOR_END                          "\033[0m"
#define FBDEV_INFO(fmt,args...)  ({do{printk(ASCII_COLOR_CYAN);printk(fmt,##args);printk(ASCII_COLOR_END);}while(0);})
static struct fb_ops mstar_fb_ops =
{
    .owner = THIS_MODULE,
    .fb_open = mstar_fb_open,
    .fb_release = mstar_fb_release,
    .fb_mmap = mstar_fb_mmap,
    .fb_set_par = mstar_fb_set_par,
    .fb_check_var = mstar_fb_check_var,
    .fb_blank = mstar_fb_blank,
    .fb_pan_display = mstar_fb_pan_display,
    .fb_setcolreg = mstar_fb_setcolreg,
    .fb_fillrect = mstar_fb_fillrect,
    .fb_copyarea = mstar_fb_copyarea,
    .fb_imageblit = mstar_fb_imageblit,
    .fb_destroy = mstar_fb_destroy,
    .fb_ioctl = mstar_fb_ioctl,
};

//-------------------------------------------------------------------------------------------------
//  MstarFB sturct and function
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MS_U8 u8GopIdx;
    MS_U8 u8GwinId;
    DRV_FB_GOP_MiuSel_e eMiuSel;
    DRV_FB_GOP_DstType_e eGopDst;
    DRV_FB_OutputColorSpace_e eOutputColorSpace;
    MS_PHYADDR phyAddr;
    //GWIN Info
    DRV_FB_GwinInfo_t stGwinInfo;
    //Stretch win info
    DRV_FB_StretchWinInfo_t stStretchWinInfo;
    //Global ALpha
    MI_FB_GlobalAlpha_t stAlpha;
    //Color key
    MI_FB_ColorKey_t stColorKey;
    //Whether shown
    MS_BOOL bShown;
    //Premultiply alpha
    MS_BOOL bPremultiply;
    //Hstart
    MS_U32 u32Hstart;
    //reference count
    unsigned int ref_count;
    //used to store pseudo_palette
    u32 pseudo_palette[16];
}HwLayerInfo_t;

typedef struct
{
    MS_U8 u8GopIdx;
    MS_U8 u8GwinId;
    DRV_FB_GOP_MiuSel_e eMiuSel;
    DRV_FB_GOP_DstType_e eGopDst;
    DRV_FB_OutputColorSpace_e eOutputColorSpace;
    //GWIN info
    DRV_FB_GwinInfo_t stGwinInfo;
    //Stretch win info
    DRV_FB_StretchWinInfo_t stStretchWinInfo;
    //Cursor Info
    MS_U16 u16PositionX;
    MS_U16 u16PositionY;
    MS_U16 u16HotSpotX;
    MS_U16 u16HotSpotY;
    MS_U16 u16IconWidth;
    MS_U16 u16IconHeight;
    MI_FB_GlobalAlpha_t stAlpha;
    MI_FB_ColorKey_t stColorKey;
    MS_PHYADDR hwCursorAddr[HWCURSOR_BUF_CNT];
    MS_VIRT hwCursorVaddr[HWCURSOR_BUF_CNT];
    MS_U8 u8DstbufIdx;
    //Whether has been initialize
    MS_BOOL bInitialized;
    //Whether shown
    MS_BOOL bShown;
}HwCursorInfo_t;

static const char* config_file = "/config/fbdev.ini";
static const char* hwlayer_section_name = "FB_DEVICE";
static HwLayerInfo_t* fbHwlayerInfos = NULL;
static HwCursorInfo_t* fbHwCursorInfo = NULL;
static struct fb_fix_screeninfo* mstar_fb_fix_infos  = NULL;
static struct fb_var_screeninfo* mstar_fb_var_infos = NULL;
static int numFbHwlayer = 0;
static int first_fb_node = 0;
//Layer zorder Info
static MS_U32 hwlayer_gopIdx[MAX_HWLAYER_CNT] = {0};
static MS_U32 hwlayer_layerIdx[MAX_HWLAYER_CNT] = {0};

#ifdef MI_SYS_PROC_FS_DEBUG
//Used to mutex access between FBIOSET_CURSOR_ATTRIBUTE and proc
static DEFINE_MUTEX(s_cursor_proc_mutex_lock);
static MI_SYS_DRV_HANDLE gFbdevHdl = NULL;

static const char* outputColorSpace2String(DRV_FB_OutputColorSpace_e eOutputColSpace)
{
    switch (eOutputColSpace)
    {
        case E_DRV_FB_GOPOUT_RGB:
            return "RGB";
        case E_DRV_FB_GOPOUT_YUV:
            return "YUV";
        default:
            return "COLORSPACE_UNKNOW";
    }
}

static const char* colorFormat2String(DRV_FB_GOP_ColorFmt_e eClrFmt)
{
    switch (eClrFmt)
    {
        case E_DRV_FB_GOP_COLOR_RGB565:
            return "RGB565";
        case E_DRV_FB_GOP_COLOR_ARGB4444:
            return "ARGB4444";
        case E_DRV_FB_GOP_COLOR_ARGB8888:
            return "ARGB8888";
        case E_DRV_FB_GOP_COLOR_ARGB1555:
            return "ARGB1555";
        case E_DRV_FB_GOP_COLOR_YUV422:
            return "YUV422";
        default:
            return "COLORFORMAT_UNKNOW";
    }
}

static MI_S32 _MI_Fbdev_IMPL_DumpOsdAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId,
    MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    int i = 0;
    struct fb_info* pFbInfo = NULL;
    HwLayerInfo_t* par = NULL;
    int curTimingWidth = 1920;
    int curTimingHeight = 1080;
    int hstart = 192;
    handle.OnPrintOut(handle, "############################OnDumpOSDAttr####################################\n");
    for (i = first_fb_node; i < first_fb_node + numFbHwlayer; i++)
    {
        pFbInfo = registered_fb[i];
        if (pFbInfo)
        {
            if (!lock_fb_info(pFbInfo))
                return -ENODEV;
            par = pFbInfo->par;
            if (par)
            {
                if (par->ref_count > 0) {
                    mstar_FB_getCurOPTiming(&curTimingWidth, &curTimingHeight, &hstart);
                }
                //Linux Framebuffer info
                handle.OnPrintOut(handle, "Framebuffer id = %s \n", pFbInfo->fix.id);
                handle.OnPrintOut(handle, "xres=%d, yres=%d\n",pFbInfo->var.xres, pFbInfo->var.yres);
                handle.OnPrintOut(handle, "xres_virtual=%d, yres_virtual=%d\n",pFbInfo->var.xres_virtual, pFbInfo->var.yres_virtual);
                handle.OnPrintOut(handle, "xoffset=%d,yoffset=%d\n", pFbInfo->var.xoffset, pFbInfo->var.yoffset);
                handle.OnPrintOut(handle, "fix.line_length=0x%x Bytes\n", pFbInfo->fix.line_length);
                handle.OnPrintOut(handle, "fix.smem_start=0x%lx\n", pFbInfo->fix.smem_start);
                handle.OnPrintOut(handle, "Memory Size=0x%x Bytes\n", pFbInfo->fix.smem_len);
                //Mstar hw info
                handle.OnPrintOut(handle, "Gop ID=%u\n", par->u8GopIdx);
                handle.OnPrintOut(handle, "Gwin ID=%u\n", par->u8GwinId);
                handle.OnPrintOut(handle, "Open Count=%d\n", par->ref_count);
                handle.OnPrintOut(handle, "Visible State=%d\n", par->bShown);
                handle.OnPrintOut(handle, "MIU Sel=%d\n", par->eMiuSel);
                handle.OnPrintOut(handle, "ColorSpace=%s\n",outputColorSpace2String(par->eOutputColorSpace));
                handle.OnPrintOut(handle, "ColorFomrmat=%s\n",colorFormat2String(par->stGwinInfo.clrType));
                //Stretch Win info
                handle.OnPrintOut(handle, "StretchWindow Pos[%d,%d]\n",
                    par->stStretchWinInfo.u16Xpos, par->stStretchWinInfo.u16Ypos);
                handle.OnPrintOut(handle, "StretchWindow Src[%d,%d],StretchWindow Dst[%d,%d]\n",
                    par->stStretchWinInfo.u16SrcWidth, par->stStretchWinInfo.u16SrcHeight,
                    par->stStretchWinInfo.u16DstWidth, par->stStretchWinInfo.u16DstHeight);
                //Gwin Info
                handle.OnPrintOut(handle, "Gwin Pos[%d,%d]\n",par->stGwinInfo.u16HStart,par->stGwinInfo.u16VStart);
                handle.OnPrintOut(handle, "Gwin Size[%d,%d]\n",
                    (par->stGwinInfo.u16HEnd-par->stGwinInfo.u16HStart),(par->stGwinInfo.u16VEnd - par->stGwinInfo.u16VStart));
                handle.OnPrintOut(handle, "Gwin PhyAddr=0x%x\n",par->stGwinInfo.u32Addr);
                //ColorKey Info
                handle.OnPrintOut(handle, "ColorKey Enable=%d\n",par->stColorKey.bKeyEnable);
                handle.OnPrintOut(handle, "ColorKey Val=[%x,%x,%x]\n",
                    par->stColorKey.u8Red,par->stColorKey.u8Green,par->stColorKey.u8Blue);
                //Alpha Info
                handle.OnPrintOut(handle, "Enable Alpha Blend=%d\n",par->stAlpha.bAlphaEnable);
                handle.OnPrintOut(handle, "Enalbe Multi Alpha=%d\n",par->stAlpha.bAlphaChannel);
                handle.OnPrintOut(handle, "Global Alpha Val=0x%x\n", par->stAlpha.u8GlobalAlpha);
                handle.OnPrintOut(handle, "Alpha0=0x%x,Alpha1=0x%x,(Only for ARGB1555)\n",
                    par->stAlpha.u8Alpha0,par->stAlpha.u8Alpha1);
#if SUPPORT_SET_ARGB1555_ALPHA
                handle.OnPrintOut(handle, "Support Set pixel alha val for ARGB1555=1\n");
#else
                handle.OnPrintOut(handle, "Support Set pixel alha val for ARGB1555=0\n");
#endif
                //OSD GOP Hstart
                handle.OnPrintOut(handle, "GOP Hstart=%d\n",par->u32Hstart);
                //Current Timing info
                if (par->ref_count > 0) {
                    handle.OnPrintOut(handle, "Current TimingWidth=%d,TimingWidth=%d,hstar=%d\n",
                        curTimingWidth, curTimingHeight, hstart);
                }
            }
            unlock_fb_info(pFbInfo);
        }
    }
    handle.OnPrintOut(handle, "##################################end#######################################\n");
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_DumpHwcursorAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId,
    MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    handle.OnPrintOut(handle, "############################OnDumpHwcursorAttr####################################\n");
    if (fbHwCursorInfo
        && fbHwCursorInfo->bInitialized)
    {
        mutex_lock(&s_cursor_proc_mutex_lock);
        handle.OnPrintOut(handle, "Cursor Gop ID=%d\n",fbHwCursorInfo->u8GopIdx);
        handle.OnPrintOut(handle, "Cursor Gwin ID=%d\n", fbHwCursorInfo->u8GwinId);
        handle.OnPrintOut(handle, "Cursor MIU Sel=%d\n",fbHwCursorInfo->eMiuSel);
        handle.OnPrintOut(handle, "Cursor PhyAddr=0x%x\n",fbHwCursorInfo->stGwinInfo.u32Addr);
        handle.OnPrintOut(handle, "Cursor ColorFmt=%s\n",
            colorFormat2String(fbHwCursorInfo->stGwinInfo.clrType));
        //Cursor request info
        handle.OnPrintOut(handle, "Cursor Icon Width=%d,Height=%d\n",
            fbHwCursorInfo->u16IconWidth,fbHwCursorInfo->u16IconHeight);
        handle.OnPrintOut(handle, "Cursor HotSpot[%d,%d]\n",
            fbHwCursorInfo->u16HotSpotX,fbHwCursorInfo->u16HotSpotY);
        handle.OnPrintOut(handle, "Cursor request pos[%d,%d]\n",
            fbHwCursorInfo->u16PositionX,fbHwCursorInfo->u16PositionY);
        //Visible
        handle.OnPrintOut(handle, "Cursor Visible=%d\n",fbHwCursorInfo->bShown);
        //Gwin Info
        handle.OnPrintOut(handle, "Cursor Gwin Pos[%d,%d]\n",
            fbHwCursorInfo->stGwinInfo.u16HStart,fbHwCursorInfo->stGwinInfo.u16VStart);
        handle.OnPrintOut(handle, "Cursor Gwin Size[%d,%d]\n",
            (fbHwCursorInfo->stGwinInfo.u16HEnd - fbHwCursorInfo->stGwinInfo.u16HStart),
            (fbHwCursorInfo->stGwinInfo.u16VEnd - fbHwCursorInfo->stGwinInfo.u16VStart));
        handle.OnPrintOut(handle, "Cursor Gwin Pitch=0x%x Bytes\n",fbHwCursorInfo->stGwinInfo.u16Pitch);
        //Stretch Win Info
        handle.OnPrintOut(handle, "Curosr StretchWindow pos[%d,%d]\n",
            fbHwCursorInfo->stStretchWinInfo.u16Xpos,
            fbHwCursorInfo->stStretchWinInfo.u16Ypos);
        handle.OnPrintOut(handle, "Cursor StretchWin Src[%d,%d],Dst[%d,%d]\n",
            fbHwCursorInfo->stStretchWinInfo.u16SrcWidth,
            fbHwCursorInfo->stStretchWinInfo.u16SrcHeight,
            fbHwCursorInfo->stStretchWinInfo.u16DstWidth,
            fbHwCursorInfo->stStretchWinInfo.u16DstHeight);
        //Color Key Enable
        handle.OnPrintOut(handle, "Cursor ColorKey Enable=%d\n",fbHwCursorInfo->stColorKey.bKeyEnable);
        handle.OnPrintOut(handle, "Cursor ColorKey Value=[0x%x,0x%x,0x%x]\n",
            fbHwCursorInfo->stColorKey.u8Red,fbHwCursorInfo->stColorKey.u8Green,fbHwCursorInfo->stColorKey.u8Blue);
        mutex_unlock(&s_cursor_proc_mutex_lock);
    }
    handle.OnPrintOut(handle, "############################OnDumpHwcursorAttr####################################\n");
    return MI_SUCCESS;
}

static MI_S32 _MI_FBDEV_IMPL_OnDumpDevAttr(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevID, void *pUsrData)
{
    _MI_Fbdev_IMPL_DumpOsdAttr(handle, u32DevID, 0, NULL, pUsrData);
    _MI_Fbdev_IMPL_DumpHwcursorAttr(handle, u32DevID, 0, NULL, pUsrData);
    return MI_SUCCESS;
}

static MI_S32 _MI_FBDEV_IMPL_OnHelp(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevID, void *pUsrData)
{
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_ShowOSD(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    struct fb_info* pFbInfo = NULL;
    HwLayerInfo_t* par = NULL;
    MS_U8 u8GopId = 0;
    MS_U8 u8GwinId = 0;
    u8 u8Fbidx = 0;
    FBDEV_INFO("%s invokded argc=%d,argv[0]=%s,argv[1]=%s,argv[2]=%s\n",__FUNCTION__,argc,argv[0],argv[1],argv[2]);
    CHECK_IFNOT(kstrtou8(argv[1], 10, &u8Fbidx), 0, -EINVAL)
    if (u8Fbidx < first_fb_node ||
        u8Fbidx > first_fb_node + numFbHwlayer) {
        FBDEV_INFO("argv[1]=%d is not invalid, the valid range of argv[1] from %d to %d\n",
            u8Fbidx,first_fb_node,(first_fb_node + numFbHwlayer));
        return -EINVAL;
    }
    if (!strcmp(argv[2], "on")) {
        pFbInfo = registered_fb[u8Fbidx];
        if (pFbInfo)
        {
            if (!lock_fb_info(pFbInfo))
                return -ENODEV;
            par = pFbInfo->par;
            if (par)
            {
                u8GopId = par->u8GopIdx;
                u8GwinId = par->u8GwinId;
                mstar_FB_BeginTransaction(u8GopId);
                _fb_gwin_enable(u8GopId, u8GwinId, TRUE);
                mstar_FB_EndTransaction(u8GopId);
             }
             unlock_fb_info(pFbInfo);
        }
    }
    else if (!strcmp(argv[2], "off")) {
        pFbInfo = registered_fb[u8Fbidx];
        if (pFbInfo)
        {
            if (!lock_fb_info(pFbInfo))
                return -ENODEV;
            par = pFbInfo->par;
            if (par)
            {
                u8GopId = par->u8GopIdx;
                u8GwinId = par->u8GwinId;
                mstar_FB_BeginTransaction(u8GopId);
                _fb_gwin_enable(u8GopId, u8GwinId, FALSE);
                mstar_FB_EndTransaction(u8GopId);
            }
            unlock_fb_info(pFbInfo);
        }
    }
    else {
        FBDEV_INFO("argv[2]=%s is invalid should set on or off!\n",argv[2]);
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_ShowCursor(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    MS_U8 u8GopId = 0;
    MS_U8 u8GwinId = 0;
    FBDEV_INFO("%s invokded argc=%d,argv[0]=%s,argv[1]=%s\n",__FUNCTION__,argc,argv[0],argv[1]);
    if (fbHwCursorInfo && fbHwCursorInfo->bInitialized)
    {
        u8GopId = fbHwCursorInfo->u8GopIdx;
        u8GwinId = fbHwCursorInfo->u8GwinId;
        mutex_lock(&s_cursor_proc_mutex_lock);
        if (!strcmp(argv[1], "on"))
        {
            mstar_FB_BeginTransaction(u8GopId);
            _fb_gwin_enable(u8GopId, u8GwinId, TRUE);
            mstar_FB_EndTransaction(u8GopId);
        }
        else if (!strcmp(argv[1], "off"))
        {
            mstar_FB_BeginTransaction(u8GopId);
            _fb_gwin_enable(u8GopId, u8GwinId, FALSE);
            mstar_FB_EndTransaction(u8GopId);
        }
        else
        {
            FBDEV_INFO("argv[1]=%s is invalid should set on or off!\n",argv[1]);
        }
        mutex_unlock(&s_cursor_proc_mutex_lock);
    }
    else
    {
        FBDEV_INFO("The Hwcurosr did not initialize yet!!\n");
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_SetClrKey(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    struct fb_info* pFbInfo = NULL;
    HwLayerInfo_t* par = NULL;
    MS_U8 u8GopId = 0;
    MS_U8 u8GwinId = 0;
    u8 u8Fbidx = 0;
    u8 u8Red = 0;
    u8 u8Green = 0;
    u8 u8Blue = 0;
    MS_U32 fullclr = 0;
    FBDEV_INFO("%s invokded argc=%d,argv[0]=%s,argv[1]=%s,argv[2]=%s,argv[3]=%s,argv[4]=%s\n",
        __FUNCTION__, argc, argv[0], argv[1], argv[2], argv[3], argv[4]);
    CHECK_IFNOT(kstrtou8(argv[1], 10, &u8Fbidx), 0, -EINVAL)
    if (u8Fbidx < first_fb_node ||
        u8Fbidx > first_fb_node + numFbHwlayer) {
        FBDEV_INFO("argv[1]=%d is not invalid, the valid range of argv[1] from %d to %d\n",
            u8Fbidx,first_fb_node,(first_fb_node + numFbHwlayer));
        return -EINVAL;
    }
    CHECK_IFNOT(kstrtou8((const char*)(argv[2]), 16, &u8Red), 0, -EINVAL)
    CHECK_IFNOT(kstrtou8((const char*)(argv[3]), 16, &u8Green), 0, -EINVAL)
    CHECK_IFNOT(kstrtou8((const char*)(argv[4]), 16, &u8Blue), 0, -EINVAL)
    if ( u8Red > 0xff || u8Green > 0xff || u8Blue > 0xff)
    {
        FBDEV_INFO("The Colorkey is [%x,%x,%x], Should set it from 0 to 255\n",u8Red, u8Green,u8Blue);
        return -EINVAL;
    }
    pFbInfo = registered_fb[u8Fbidx];
    if (pFbInfo)
    {
        if (!lock_fb_info(pFbInfo))
            return -ENODEV;
        par = pFbInfo->par;
        if (par)
        {
            u8GopId = par->u8GopIdx;
            u8GwinId = par->u8GwinId;
            mstar_FB_BeginTransaction(u8GopId);
            mstar_FB_EnableTransClr_EX(u8GopId, E_DRV_FB_GOPTRANSCLR_FMT0, TRUE);
            fullclr = (0xff<<24) | (u8Red<<16)
                    | (u8Green<<8) | (u8Blue);
            mstar_FB_SetTransClr_8888(u8GopId,fullclr,0);
            mstar_FB_EndTransaction(u8GopId);
        }
        unlock_fb_info(pFbInfo);
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_DisableClrKey(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    struct fb_info* pFbInfo = NULL;
    HwLayerInfo_t* par = NULL;
    MS_U8 u8GopId = 0;
    MS_U8 u8GwinId = 0;
    u8 u8Fbidx = 0;
    FBDEV_INFO("%s invokded argc=%d,argv[0]=%s,argv[1]=%s\n", __FUNCTION__,argc,argv[0],argv[1]);
     CHECK_IFNOT(kstrtou8(argv[1], 10, &u8Fbidx), 0, -EINVAL)
    if (u8Fbidx < first_fb_node ||
        u8Fbidx > first_fb_node + numFbHwlayer) {
        FBDEV_INFO("argv[1]=%d is not invalid, the valid range of argv[1] from %d to %d\n",
            u8Fbidx,first_fb_node,(first_fb_node + numFbHwlayer));
        return -EINVAL;
    }
    pFbInfo = registered_fb[u8Fbidx];
    if (pFbInfo)
    {
        if (!lock_fb_info(pFbInfo))
            return -ENODEV;
        par = pFbInfo->par;
        if (par)
        {
            u8GopId = par->u8GopIdx;
            u8GwinId = par->u8GwinId;
            mstar_FB_BeginTransaction(u8GopId);
            mstar_FB_EnableTransClr_EX(u8GopId, E_DRV_FB_GOPTRANSCLR_FMT0, FALSE);
            mstar_FB_EndTransaction(u8GopId);
        }
        unlock_fb_info(pFbInfo);
    }
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_SetAlpha(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    //TODO K6l is not support for set alpha0 and Alpha1 for ARGB1555
    //implement it on I2
    return MI_SUCCESS;
}

static MI_S32 _MI_Fbdev_IMPL_DumpCursor(MI_SYS_DEBUG_HANDLE_t handle,
    MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData)
{
    struct file *fp = NULL;
    char path_file_name_buf[256] = {0};
    MI_S32 current_name_length = 0;
    const char* fileName = "CursorData.raw";
    mm_segment_t old_fs;
    int dumpbufIdx = -1;
    MS_U32 buflength = 0x10000;
    int nSize = 0;
    FBDEV_INFO("%s invokded argc=%d,argv[0]=%s,argv[1]=%s\n", __FUNCTION__,argc,argv[0],argv[1]);
    if (fbHwCursorInfo && fbHwCursorInfo->bInitialized)
    {
        strncpy(path_file_name_buf, (const char*)argv[1], strlen(argv[1]));
        current_name_length += strlen(argv[1]);
        if(path_file_name_buf[current_name_length-1] != '/')
        {
            path_file_name_buf[current_name_length] = '/';
            current_name_length++;
        }
        strncpy(path_file_name_buf+current_name_length, fileName, strlen(fileName));
        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fp = filp_open((char *)(path_file_name_buf),O_WRONLY|O_CREAT|O_TRUNC, 0777);
        if(IS_ERR(fp))
        {
            FBDEV_INFO("filp_open fail   PTR_ERR_fp = %lx\n",PTR_ERR(fp));//here use PTR_ERR(fp) to show errno
            set_fs(old_fs);
            return E_MI_ERR_FAILED;
        }
        mutex_lock(&s_cursor_proc_mutex_lock);
        //dump cursor data display by gwin
        if ((fbHwCursorInfo->stGwinInfo.u32Addr >=fbHwCursorInfo->hwCursorAddr[0])
            &&(fbHwCursorInfo->stGwinInfo.u32Addr <= fbHwCursorInfo->hwCursorAddr[0] + buflength) )
        {
            dumpbufIdx = 0;
        }
        else if ((fbHwCursorInfo->stGwinInfo.u32Addr >=fbHwCursorInfo->hwCursorAddr[1])
            &&(fbHwCursorInfo->stGwinInfo.u32Addr <= fbHwCursorInfo->hwCursorAddr[1] + buflength) )
        {
            dumpbufIdx = 1;
        }
        if (dumpbufIdx != -1)
        {
            nSize = vfs_write(fp, (char*)(fbHwCursorInfo->hwCursorVaddr[dumpbufIdx]),
                buflength, &fp->f_pos);
        }
        mutex_unlock(&s_cursor_proc_mutex_lock);
        filp_close(fp, NULL);
        set_fs(old_fs);
        if (nSize != buflength)
        {
            FBDEV_INFO("DumpCursor write faile write size=0x%x,BufSize=0x%x",
                nSize,buflength);
            return E_MI_ERR_FAILED;
        }
    }
    else
    {
        FBDEV_INFO("The Hwcurosr did not initialize yet!!\n");
    }
    return MI_SUCCESS;
}
static MI_U32 MI_Moduledev_RegisterDev(void)
{
    mi_sys_ModuleDevBindOps_t stFbdevOps;
    mi_sys_ModuleDevInfo_t stModInfo;
    mi_sys_ModuleDevProcfsOps_t pstModuleProcfsOps;
    FBDEV_INFO("MI_Moduledev_RegisterDev Enter\n");
    memset(&stFbdevOps, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stFbdevOps.OnBindInputPort = NULL;
    stFbdevOps.OnUnBindInputPort = NULL;
    stFbdevOps.OnBindOutputPort = NULL;
    stFbdevOps.OnUnBindOutputPort = NULL;
    stFbdevOps.OnOutputPortBufRelease = NULL;

    memset(&stModInfo, 0, sizeof(mi_sys_ModuleDevInfo_t));
    stModInfo.eModuleId = E_MI_MODULE_ID_FB;
    stModInfo.u32DevId = 0;
    stModInfo.u32DevChnNum = 0;
    stModInfo.u32InputPortNum = 0;
    stModInfo.u32OutputPortNum = 0;

    memset(&pstModuleProcfsOps, 0 , sizeof(pstModuleProcfsOps));
    pstModuleProcfsOps.OnDumpDevAttr = _MI_FBDEV_IMPL_OnDumpDevAttr;
    pstModuleProcfsOps.OnDumpChannelAttr = NULL;
    pstModuleProcfsOps.OnDumpInputPortAttr = NULL;
    pstModuleProcfsOps.OnDumpOutPortAttr = NULL;

    pstModuleProcfsOps.OnHelp = _MI_FBDEV_IMPL_OnHelp;
    gFbdevHdl = mi_sys_RegisterDev(&stModInfo, &stFbdevOps, NULL, &pstModuleProcfsOps,NULL);
    if(!gFbdevHdl)
    {
        FBDEV_INFO("mi_sys_RegisterDev error.\n");
    }
    FBDEV_INFO("gGfxDevHdl = %p.\n", gFbdevHdl);

    mi_sys_RegistCommand("GUI_SHOW", 2, _MI_Fbdev_IMPL_ShowOSD, gFbdevHdl);
    mi_sys_RegistCommand("CURSOR_SHOW", 1, _MI_Fbdev_IMPL_ShowCursor, gFbdevHdl);
    mi_sys_RegistCommand("GUI_SET_CLRKEY", 4, _MI_Fbdev_IMPL_SetClrKey, gFbdevHdl);
    mi_sys_RegistCommand("GUI_DISABLE_CLRKEY", 1, _MI_Fbdev_IMPL_DisableClrKey, gFbdevHdl);
    mi_sys_RegistCommand("GUI_SETALPHA_ARGB1555", 3, _MI_Fbdev_IMPL_SetAlpha, gFbdevHdl);
    mi_sys_RegistCommand("CURSOR_DUMP", 1, _MI_Fbdev_IMPL_DumpCursor, gFbdevHdl);
    return MI_SUCCESS;
}
#endif
static void _fb_gwin_update(struct fb_var_screeninfo *fbvar,  struct fb_info *pinfo,
    DRV_FB_GwinInfo_t* winInfo)
{
    MS_U32 bytes_per_pixel;
    HwLayerInfo_t* par = pinfo->par;
    MS_U8 u8GopIdx = par->u8GopIdx;
    MS_U8 u8GwinIdx = par->u8GwinId;
    MS_U16 u16HorStart = 0;
    MS_U16 u16HorEnd = fbvar->xres;
    MS_U16 u16VerStart = 0;
    MS_U16 u16VerEnd = fbvar->yres;

    if(fbvar->bits_per_pixel == 1)
       bytes_per_pixel = 1;
    else
       bytes_per_pixel = fbvar->bits_per_pixel/8;
    winInfo->u32Addr = pinfo->fix.smem_start;
    if(fbvar->xoffset || fbvar->yoffset)
    {
        winInfo->u32Addr += fbvar->xoffset * bytes_per_pixel
            + pinfo->fix.line_length * fbvar->yoffset;
    }
    winInfo->clrType  = get_color_fmt(fbvar);
    winInfo->u16HStart = u16HorStart;
    winInfo->u16HEnd = u16HorEnd;
    winInfo->u16VStart = u16VerStart;
    winInfo->u16VEnd = u16VerEnd;
    winInfo->u16Pitch = pinfo->fix.line_length;
    mstar_FB_SetGwinInfo(u8GopIdx, u8GwinIdx, *winInfo);
}

static int mstar_fb_set_par(struct fb_info *pinfo)
{
    struct fb_var_screeninfo *var = &pinfo->var;
    MS_U32 bits_per_pixel = 32;
    switch (var->bits_per_pixel)
    {
        case 32:
        case 16:
            pinfo->fix.visual = FB_VISUAL_TRUECOLOR;
            bits_per_pixel = var->bits_per_pixel;
            break;
        case 1:
            pinfo->fix.visual = FB_VISUAL_MONO01;
            bits_per_pixel = 8;
            break;
        default:
            pinfo->fix.visual = FB_VISUAL_PSEUDOCOLOR;
            bits_per_pixel = 8;
            break;
    }
    pinfo->fix.line_length = (var->xres_virtual * bits_per_pixel) / 8;
    pinfo->fix.xpanstep = 1;
    /* activate this new configuration */
    return 0;
}

static int mstar_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
    MS_U32 line_length,bits_per_pixel;
    DRV_FB_GOP_ColorFmt_e ColorFmt;
    //HwLayerInfo_t* par = info->par;
    /*
     *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
     *  as FB_VMODE_SMOOTH_XPAN is only used internally
     */
     if (var->vmode & FB_VMODE_CONUPDATE)
     {
        var->vmode |= FB_VMODE_YWRAP;
        var->xoffset = info->var.xoffset;
        var->yoffset = info->var.yoffset;
     }
     //Alignment xres and xres_virtual down to 16 pixel and stored it
     var->xres = ALIGN_DOWNTO_16(var->xres);
     var->xres_virtual = ALIGN_DOWNTO_16(var->xres_virtual);
      /*
      *  Some very basic checks
      */
    if (!var->xres)
        var->xres = info->var.xres;
    if (!var->yres)
        var->yres = info->var.xres;
    if (var->xres > var->xres_virtual)
        var->xres_virtual = var->xres;
    if (var->yres > var->yres_virtual)
        var->yres_virtual = var->yres;
    if (var->bits_per_pixel <= 1)
    {
        var->bits_per_pixel = 1;
        bits_per_pixel = 8;
    }else if (var->bits_per_pixel <= 8)
    {
        var->bits_per_pixel = 8;
        bits_per_pixel = 8;
     }else if (var->bits_per_pixel <= 16)
     {
        var->bits_per_pixel = 16;
        bits_per_pixel = 16;
     }else if (var->bits_per_pixel <= 32)
     {
        var->bits_per_pixel = 32;
        bits_per_pixel = 32;
     }else
         return -EINVAL;
    if (var->xres_virtual < var->xoffset + var->xres)
        var->xres_virtual = var->xoffset + var->xres;
    if (var->yres_virtual < var->yoffset + var->yres)
        var->yres_virtual = var->yoffset + var->yres;
    /*
     *  Memory limit
     */
    line_length = get_line_length(var->xres_virtual,bits_per_pixel);
    if(line_length * var->yres_virtual > info->fix.smem_len)
        return -ENOMEM;
    /*
     * Now that we checked it we alter var. The reason being is that the video
     * mode passed in might not work but slight changes to it might make it
     * work. This way we let the user know what is acceptable.
     */
    ColorFmt = get_color_fmt(var);
    if(ColorFmt == E_DRV_FB_GOP_COLOR_INVALID)
        return -EINVAL;
/*
 *The fraembuffer device should not consider Mstar hwlimit
 */
#if 0
    /*
     *GOP can not scaledown
     */
     if (var->xres > par->stStretchWinInfo.u16DstWidth ||
        var->yres > par->stStretchWinInfo.u16DstHeight) {
        FBDEV_INFO("mstar_fb_check_var fb:%d request resolution[%d,%d] can not bigger than gop stretchwin dst[%d,%d]\n",
            info->node,var->xres,var->yres,par->stStretchWinInfo.u16DstWidth,par->stStretchWinInfo.u16DstHeight);
        return -EINVAL;
     }
#endif
    return 0;
}
static int mstar_fb_blank(int blank, struct fb_info *info)
{
    return 0;
}

static int mstar_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *pinfo)
{
    HwLayerInfo_t* par = pinfo->par;
    MS_U8 u8GopId = par->u8GopIdx;
    MS_U8 u8GwinId = par->u8GwinId;
    MS_U16 u16DstX = par->stStretchWinInfo.u16Xpos;
    MS_U16 u16DstY = par->stStretchWinInfo.u16Ypos;
    MS_U16 u16Width = par->stStretchWinInfo.u16DstWidth;
    MS_U16 u16Height = par->stStretchWinInfo.u16DstHeight;
    MS_U16 u16FbWidth = par->stStretchWinInfo.u16SrcWidth;
    MS_U16 u16FbHeight = par->stStretchWinInfo.u16SrcHeight;
    MS_BOOL bShown = par->bShown;
    MS_BOOL bNeedRecrateFB = FALSE;
    DRV_FB_GOP_ColorFmt_e eclrfmt = get_color_fmt(var);
    DRV_FB_GwinInfo_t gwinInfo;
    //invoked by linux framebuffer fraemwork fbcon notify
    if (!par->ref_count) {
        FBDEV_INFO("fb:%d mstar_fb_pan_display invoked by fbcon notify do nothing\n",pinfo->node);
        return 0;
    }
    mstar_FB_BeginTransaction(u8GopId);
    //Update Stretch Win
    if( (u16FbWidth != var->xres) || (u16FbHeight != var->yres)) {
        _fb_strewin_update(var, u8GopId, u16DstX, u16DstY, u16Width, u16Height);
    }
    bNeedRecrateFB = (var->xres != (par->stGwinInfo.u16HEnd-par->stGwinInfo.u16HStart)) ||
        (var->yres != (par->stGwinInfo.u16VEnd - par->stGwinInfo.u16VStart)) ||
        (eclrfmt != par->stGwinInfo.clrType) ||
        (pinfo->fix.line_length != par->stGwinInfo.u16Pitch);
    if (bNeedRecrateFB) {
        mstar_FB_DestroyFBInfo(par->u8GwinId);
        mstar_FB_CreateFBInfo(par->u8GwinId, var->xres, var->yres,
            eclrfmt, pinfo->fix.line_length, pinfo->fix.smem_start);
    }
    _fb_gwin_update(var, pinfo, &gwinInfo);
    if (bShown == FALSE) {
        _fb_gwin_enable(u8GopId, u8GwinId, TRUE);
        par->bShown = TRUE;
    }
    mstar_FB_EndTransaction(u8GopId);
    //update StretchWin srcWidth srcHeight
    par->stStretchWinInfo.u16SrcWidth = pinfo->var.xres;
    par->stStretchWinInfo.u16SrcHeight = pinfo->var.yres;
    //update gwininfo
    memcpy(&par->stGwinInfo, &gwinInfo, sizeof(DRV_FB_GwinInfo_t));
    return 0;
}

//It's not necessary to support pseudo_palette
static int mstar_fb_setcolreg(unsigned regno, unsigned red, unsigned green,
                              unsigned blue, unsigned transp, struct fb_info *info)
{
    /* grayscale works only partially under directcolor */
    if(info->var.grayscale)
    {
        /* grayscale = 0.30*R + 0.59*G + 0.11*B */
        red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;
    }

    if(info->fix.visual == FB_VISUAL_TRUECOLOR || info->fix.visual == FB_VISUAL_DIRECTCOLOR)
    {
        MS_U32 v;

        if(regno >= 16)
            return -EINVAL;

        v = (red << info->var.red.offset) | (green << info->var.green.offset) | (blue << info->var.blue.offset) | (transp << info->var.transp.offset);
            ((MS_U32*)(info->pseudo_palette))[regno] = v;
    }
    return 0;
}

static void mstar_fb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
#ifdef CONFIG_FB_VIRTUAL
        sys_fillrect(p, rect);
#endif
}

static void mstar_fb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
#ifdef CONFIG_FB_VIRTUAL
        sys_copyarea(p, area);
#endif
}
static void mstar_fb_imageblit(struct fb_info *p, const struct fb_image *image)
{
#ifdef CONFIG_FB_VIRTUAL
        sys_imageblit(p, image);
#endif
}
static void mstar_fb_destroy(struct fb_info *info)
{
    FBDEV_INFO("fb%d mstar_fb_destroy was invoked!\n",info->node);
    if(info->screen_base) {
#if 0
        iounmap(info->screen_base);
        info->screen_base = NULL;
#else
        mi_sys_UnVmap(info->screen_base);
        info->screen_base = NULL;
#endif
    }
}
static MS_U32 get_line_length(int xres_virtual, int bpp)
{
    MS_U32 length;

    length = xres_virtual * bpp;
    length = (length + 31) & ~31;
    length >>= 3;

    return (length);
}
static DRV_FB_GOP_ColorFmt_e get_color_fmt(struct fb_var_screeninfo *var)
{
    DRV_FB_GOP_ColorFmt_e ColorFmt = E_DRV_FB_GOP_COLOR_INVALID;
    switch (var->bits_per_pixel) {
        case 1:
        case 8:
            var->red.offset = 0;
            var->red.length = 8;
            var->green.offset = 0;
            var->green.length = 8;
            var->blue.offset = 0;
            var->blue.length = 8;
            var->transp.offset = 0;
            var->transp.length = 0;
            ColorFmt = E_DRV_FB_GOP_COLOR_I8;
            break;
        case 16:
            if ( (var->transp.length) && (var->transp.offset == 15))
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 5;
                var->red.offset = 10;
                var->red.length = 5;
                var->transp.offset = 15;
                var->transp.length = 1;
                ColorFmt = E_DRV_FB_GOP_COLOR_ARGB1555;
            }else if ((var->transp.length) && (var->transp.offset == 12))
            {
                var->blue.offset = 0;
                var->blue.length = 4;
                var->green.offset = 4;
                var->green.length = 4;
                var->red.offset = 8;
                var->red.length = 4;
                var->transp.offset = 12;
                var->transp.length = 4;
                ColorFmt = E_DRV_FB_GOP_COLOR_ARGB4444;
            }else if ( (var->transp.length) && (var->transp.offset == 0))
            {
                var->transp.offset = 0;
                var->transp.length = 1;
                var->blue.offset = 1;
                var->blue.length = 5;
                var->green.offset = 6;
                var->green.length = 5;
                var->red.offset = 11;
                var->red.length = 5;
                ColorFmt = E_DRV_FB_GOP_COLOR_RGBA5551;
            }else {
                 /* RGB 565 */
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 6;
                var->red.offset = 11;
                var->red.length = 5;
                var->transp.offset = 0;
                var->transp.length = 0;
                ColorFmt = E_DRV_FB_GOP_COLOR_RGB565;
            }
            break;
            case 32:/* ARGB 8888 */
            {
                var->red.offset = 0;
                var->red.length = 8;
                var->green.offset = 8;
                var->green.length = 8;
                var->blue.offset = 16;
                var->blue.length = 8;
                var->transp.offset = 24;
                var->transp.length = 8;
                ColorFmt = E_DRV_FB_GOP_COLOR_ARGB8888;
            }
            break;
    }
    var->red.msb_right = 0;
    var->green.msb_right = 0;
    var->blue.msb_right = 0;
    var->transp.msb_right = 0;
    return ColorFmt;
}

static void _fb_buf_init(struct fb_info *pinfo, unsigned long pa)
{
    HwLayerInfo_t* par = pinfo->par;
    DRV_FB_GOP_MiuSel_e miuSel = par->eMiuSel;
    pinfo->fix.smem_start = pa;

    if (miuSel==E_DRV_FB_GOP_SEL_MIU0) {
        #if 0
        pinfo->screen_base =
            (char __iomem *)ioremap(pa + MIU0_BUS_OFFSET, pinfo->fix.smem_len);
        #else
        pinfo->screen_base =
            (char __iomem *)mi_sys_Vmap(pa, pinfo->fix.smem_len, FALSE);
        #endif
    } else if (miuSel==E_DRV_FB_GOP_SEL_MIU1) {
        #if 0
        pinfo->screen_base =
            (char __iomem *)ioremap(pa - MIU1_INTERVAL + MIU1_BUS_OFFSET, pinfo->fix.smem_len);
        #else
        pinfo->screen_base =
            (char __iomem *)mi_sys_Vmap(pa, pinfo->fix.smem_len, FALSE);
        #endif
    } else if (miuSel==E_DRV_FB_GOP_SEL_MIU2) {
        #if 0
        pinfo->screen_base =
            (char __iomem *)ioremap(pa - MIU2_INTERVAL + MIU2_BUS_OFFSET, pinfo->fix.smem_len);
        #else
        pinfo->screen_base =
            (char __iomem *)mi_sys_Vmap(pa, pinfo->fix.smem_len, FALSE);
        #endif
    }

    FBDEV_INFO("screen_base:%p\n", pinfo->screen_base);

    if (pinfo->screen_base)
        memset(pinfo->screen_base, 0x0, pinfo->fix.smem_len);
}
static void _fb_gwin_init(struct fb_info *pinfo)
{
    HwLayerInfo_t* par = pinfo->par;
    MS_U8 u8GopIdx = par->u8GopIdx;
    MS_U8 u8GwinIdx = par->u8GwinId;
    DRV_FB_OutputColorSpace_e eOutputColorSpace
        = par->eOutputColorSpace;
   DRV_FB_GOP_MiuSel_e eMiuSel = par->eMiuSel;
   DRV_FB_GOP_DstType_e eGopDst = par->eGopDst;
   //default timing info as set as 1080P
   int curTimingWidth = 1920;
   int curTimingHeight = 1080;
   int hstart = 192;
   mstar_FB_getCurOPTiming(&curTimingWidth, &curTimingHeight, &hstart);
   mstar_FB_Init(u8GopIdx, curTimingWidth, curTimingHeight, hstart);
   mstar_FB_SetHMirror(u8GopIdx, FALSE);
   mstar_FB_SetVMirror(u8GopIdx, FALSE);
   mstar_FB_OutputColor_EX(u8GopIdx, eOutputColorSpace);
   mstar_FB_MIUSel(u8GopIdx, eMiuSel);

   //Init gop zorder
   mstar_FB_InitLayerSetting(hwlayer_gopIdx, hwlayer_layerIdx, MAX_HWLAYER_CNT);
   //default set colorkey,const alpha,multialpha as false
   mstar_FB_EnableTransClr_EX(u8GopIdx, GOPTRANSCLR_FMT0, FALSE);
   mstar_FB_SetBlending(u8GopIdx, u8GwinIdx, TRUE, 0xFF);
#if SUPPORT_SET_ARGB1555_ALPHA
   if (par->stGwinInfo.clrType == E_DRV_FB_GOP_COLOR_ARGB1555) {
        mstar_FB_SetAlpha0(par->u8GwinId, 0xFF);
        mstar_FB_SetAlpha1(par->u8GwinId, 0xFF);
        par->stAlpha.u8Alpha0 = 0xff;
        par->stAlpha.u8Alpha1 = 0xff;
   }
#endif
   mstar_FB_EnableMultiAlpha(u8GopIdx, FALSE);
   mstar_FB_SetDst(u8GopIdx, eGopDst, FALSE);
   //set stretchwin dst as timing
   par->stStretchWinInfo.u16DstWidth = curTimingWidth;
   par->stStretchWinInfo.u16DstHeight = curTimingHeight;
   par->u32Hstart = hstart;
}
static void _fb_gwin_enable(MS_U8 u8GopIdx,MS_U8 u8GwinIdx ,MS_BOOL bEnable)
{
    mstar_FB_EnableGwin(u8GopIdx, u8GwinIdx, bEnable);
}
static void _fb_strewin_update(struct fb_var_screeninfo *var,
    MS_U8 u8GopId, MS_U16 dstX, MS_U16 dstY, MS_U16 u16DstWidth, MS_U16 u16DstHeight)
{
    mstar_FB_SetStretchWin(u8GopId, dstX, dstY, var->xres, var->yres);
    mstar_FB_SetHScale(u8GopId, TRUE, var->xres, u16DstWidth);
    mstar_FB_SetVScale(u8GopId, TRUE, var->yres, u16DstHeight);
}
static void _fb_hwcursor_init(MS_U16 timingWidth, MS_U16 timingHeight, MS_U16 hstart)
{
    MS_U8 u8GopIdx = fbHwCursorInfo->u8GopIdx;
    MS_U8 u8GwinIdx = fbHwCursorInfo->u8GwinId;
    DRV_FB_OutputColorSpace_e eOutputColorSpace
        = fbHwCursorInfo->eOutputColorSpace;
    DRV_FB_GOP_MiuSel_e eMiuSel = fbHwCursorInfo->eMiuSel;
    DRV_FB_GOP_DstType_e eGopDst = fbHwCursorInfo->eGopDst;
    unsigned long pa = fbHwCursorInfo->hwCursorAddr[0];
    MS_U32 length = 0x20000;
    char __iomem *virAddr = NULL;

    mstar_FB_Init(u8GopIdx, timingWidth, timingHeight, hstart);
    mstar_FB_SetHMirror(u8GopIdx, FALSE);
    mstar_FB_SetVMirror(u8GopIdx, FALSE);
    mstar_FB_OutputColor_EX(u8GopIdx, eOutputColorSpace);
    mstar_FB_MIUSel(u8GopIdx, eMiuSel);
    mstar_FB_SetDst(u8GopIdx, eGopDst, FALSE);
    //defult set colorkey,const alpha,multialpha as false
    mstar_FB_EnableTransClr_EX(u8GopIdx, GOPTRANSCLR_FMT0, FALSE);
    mstar_FB_SetBlending(u8GopIdx, u8GwinIdx, TRUE, 0xFF);
    mstar_FB_EnableMultiAlpha(u8GopIdx, FALSE);
    //map iomem
    if (eMiuSel == E_DRV_FB_GOP_SEL_MIU0) {
        virAddr = (char __iomem *)ioremap(pa + MIU0_BUS_OFFSET, length);
    } else if (eMiuSel == E_DRV_FB_GOP_SEL_MIU1) {
        virAddr = (char __iomem *)ioremap(pa + MIU1_BUS_OFFSET, length);
    } else if (eMiuSel == E_DRV_FB_GOP_SEL_MIU2) {
        virAddr = (char __iomem *)ioremap(pa + MIU2_BUS_OFFSET, length);
    }
    fbHwCursorInfo->hwCursorVaddr[0] = (MS_VIRT)virAddr;
    fbHwCursorInfo->hwCursorVaddr[1] = fbHwCursorInfo->hwCursorVaddr[0] + 0x10000;
    memset(virAddr, 0, length);
}

static void _fb_hwcursor_deinit(void)
{
    char __iomem *virAddr = NULL;
    if (fbHwCursorInfo) {
        fbHwCursorInfo->bInitialized = FALSE;
        fbHwCursorInfo->bShown = FALSE;
        fbHwCursorInfo->u8DstbufIdx = 0;
        if (fbHwCursorInfo->hwCursorVaddr[0]) {
            virAddr = (char __iomem *)(fbHwCursorInfo->hwCursorVaddr[0]);
            iounmap(virAddr);
            fbHwCursorInfo->hwCursorVaddr[0] = NULL;
            fbHwCursorInfo->hwCursorVaddr[1] = NULL;
        }
        fbHwCursorInfo->u16IconWidth = 0;
        fbHwCursorInfo->u16IconHeight = 0;
        fbHwCursorInfo->u16HotSpotX = 0;
        fbHwCursorInfo->u16HotSpotY = 0;
        fbHwCursorInfo->u16PositionX = 0;
        fbHwCursorInfo->u16PositionY = 0;
        mstar_FB_DestroyFBInfo(fbHwCursorInfo->u8GwinId);
        memset(&fbHwCursorInfo->stStretchWinInfo, 0, sizeof(DRV_FB_StretchWinInfo_t));
        fbHwCursorInfo->stGwinInfo.u16HStart = 0;
        fbHwCursorInfo->stGwinInfo.u16HEnd= 0;
        fbHwCursorInfo->stGwinInfo.u16VStart = 0;
        fbHwCursorInfo->stGwinInfo.u16VEnd = 0;
        fbHwCursorInfo->stGwinInfo.u32Addr = 0;
        //Reset colorkey info
        memset(&fbHwCursorInfo->stColorKey, 0, sizeof(MI_FB_ColorKey_t));
    }
}
static void calHwcursorInfo(MS_U16 iconWidth, MS_U16 iconHeight,
    MS_U16 hotSpotX, MS_U16 hotSpotY,
    MS_U16 positionX, MS_U16 positionY,
    MS_U16 osdWidth, MS_U16 osdHeight,
    MS_U16* retCropX, MS_U16* retCropY,
    MS_U16* retCropWidth, MS_U16* retCropHeight,
    MS_U16* gwinposX, MS_U16* gwinposY)
{
    MS_BOOL leftEdge = FALSE;
    MS_BOOL rightEdge = FALSE;
    MS_BOOL topEdge = FALSE;
    MS_BOOL bottomEdge = FALSE;
    MS_BOOL bNeedCalCrop = FALSE;
    leftEdge = (hotSpotX > positionX);
    rightEdge = ((osdWidth- (positionX - hotSpotX)) < iconWidth);
    topEdge = (hotSpotY > positionY);
    bottomEdge = ((osdHeight- (positionY - hotSpotY)) < iconHeight);
    bNeedCalCrop = (leftEdge || rightEdge || topEdge || bottomEdge);
    *retCropX = 0;
    *retCropY = 0;
    *retCropWidth = iconWidth &(~0x1);
    *retCropHeight = iconHeight;
    *gwinposX = positionX - hotSpotX;
    *gwinposY = positionY - hotSpotY;
    if (!bNeedCalCrop)
        return;
    if (leftEdge) {
        *retCropX = hotSpotX - positionX;
        *retCropWidth = (iconWidth - *retCropX) & (~0x1);
        *gwinposX = 0;
    }
    if (topEdge) {
        *retCropY = hotSpotY - positionY;
        *retCropHeight = iconHeight - *retCropY;
        *gwinposY = 0;
    }
    if (rightEdge) {
        *retCropX = 0;
        *retCropWidth = osdWidth- (positionX - hotSpotX);
        *retCropWidth = (*retCropWidth) & (~0x1);
        *gwinposX = osdWidth - *retCropWidth;
    }
    if (bottomEdge) {
        *retCropY = 0;
        *retCropHeight = osdHeight- (positionY - hotSpotY);
        *gwinposY = osdHeight - *retCropHeight;
    }
}

//-------------------------------------------------------------------------------------------------
// Parse Configuration file
//-------------------------------------------------------------------------------------------------
static int readConfile(char** data)
{
    struct file* filp = NULL;
    mm_segment_t fs;
    loff_t size = 0;
    ssize_t ret = 0;
    int result = 0;
    filp = filp_open(config_file, O_RDONLY, 0644);
    if (IS_ERR_OR_NULL(filp))
    {
        printk(KERN_ERR "\33[0;36m read /config/fbdev.ini failed!\n");
        return -1;
    }
    do
    {
        fs = get_fs();
        set_fs(KERNEL_DS);
        size = filp->f_op->llseek(filp, 0, SEEK_END);
        *data = (char*)kzalloc(size + 1, GFP_KERNEL);
        if (IS_ERR_OR_NULL(*data)) {
            printk(KERN_ERR "Can not allocate buffer for conf file!\n");
            result = -1;
            break;
        }
        filp->f_op->llseek(filp, 0, SEEK_SET);
        ret = vfs_read(filp, *data, size, &filp->f_pos);
        if (ret != size) {
            printk(KERN_ERR "fs_read return ret=%zu, But real size=%lld\n",ret,size);
            kfree(*data);
            result = -1;
            break;
        }
    }while(0);
    set_fs(fs);
    filp_close(filp, NULL);
    return result;
}

static void set_fb_bitfield(MI_FB_ColorFmt_e eColorFmt , struct fb_var_screeninfo *var)
    {
        switch (eColorFmt)
        {
            case E_MI_FB_COLOR_FMT_RGB565:
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 6;
                var->red.offset = 11;
                var->red.length = 5;
                var->transp.offset = 0;
                var->transp.length = 0;
            }
            break;
            case E_MI_FB_COLOR_FMT_ARGB4444:
            {
                var->blue.offset = 0;
                var->blue.length = 4;
                var->green.offset = 4;
                var->green.length= 4;
                var->red.offset = 8;
                var->red.length = 4;
                var->transp.offset = 12;
                var->transp.length = 4;
            }
            break;
            case E_MI_FB_COLOR_FMT_ARGB8888:
            {
                var->red.offset = 16;
                var->red.length = 8;
                var->green.offset = 8;
                var->green.length = 8;
                var->blue.offset = 0;
                var->blue.length = 8;
                var->transp.offset = 24;
                var->transp.length = 8;
            }
            break;
            case E_MI_FB_COLOR_FMT_ARGB1555:
            {
                var->blue.offset = 0;
                var->blue.length = 5;
                var->green.offset = 5;
                var->green.length = 5;
                var->red.offset = 10;
                var->red.length = 5;
                var->transp.offset = 15;
                var->transp.length = 1;
            }
            break;
            case E_MI_FB_COLOR_FMT_YUV422:
            default:
            {
                FBDEV_INFO("set_fb_bitfield colorFmt=%d \n",eColorFmt);
            }
        break;
    }
}
static u16 getBufferCount(u32 fbLength, u16 width, u16 height, u16 bpx)
{
    u16 bufferCount = 0;
    u32 bufferSize = width * height * bpx;
    if (fbLength / bufferSize >= 3) {
        bufferCount = 3;
    } else {
        bufferCount = fbLength / bufferSize;
    }
    return bufferCount;
}

//-------------------------------------------------------------------------------------------------
// Restore FbInfo when open /dev/fb
//-------------------------------------------------------------------------------------------------
static void restoreFbInfo(struct fb_info* pinfo)
{
    HwLayerInfo_t* par = pinfo->par;
    int fbIdx = pinfo->node - first_fb_node;
    MS_U16 bpx = 4;
    u16 bufferCount = 2;
    memcpy(par, &fbHwlayerInfos[fbIdx], sizeof(HwLayerInfo_t));
    //restore fb_var_screeninfo
    pinfo->var.xres = par->stGwinInfo.u16HEnd;
    pinfo->var.yres = par->stGwinInfo.u16VEnd;
    pinfo->var.xres_virtual = pinfo->var.xres;
    //double buffer
    bpx =  mstar_FB_GetBpp(par->stGwinInfo.clrType);
    bufferCount = getBufferCount(pinfo->fix.smem_len,
        pinfo->var.xres, pinfo->var.yres, bpx);
    pinfo->var.yres_virtual = pinfo->var.yres * bufferCount;
    pinfo->var.xoffset = 0;
    pinfo->var.yoffset = 0;
    pinfo->var.bits_per_pixel = bpx << 3;
    pinfo->var.activate = FB_ACTIVATE_NOW;
    set_fb_bitfield((MI_FB_ColorFmt_e)(par->stGwinInfo.clrType), &(pinfo->var));
    //set width, heigth as maxium
    pinfo->var.width = -1;
    pinfo->var.height = -1;
    pinfo->var.grayscale = 0;
    /*timing useless? use the vfb default */
    pinfo->var.pixclock
        = 100000000000LLU / (6 *  1920 * 1080);
    pinfo->var.left_margin = 64;
    pinfo->var.right_margin = 64;
    pinfo->var.upper_margin = 32;
    pinfo->var.lower_margin = 32;
    pinfo->var.hsync_len  =64;
    pinfo->var.vsync_len = 2;
    pinfo->var.vmode  = FB_VMODE_NONINTERLACED;
    //Restore pitch, maybe chage it via FBIO_OUTPUT
    pinfo->fix.line_length = par->stGwinInfo.u16Pitch;
}

static int parse_hwcursorInfo(IniSectionNode *fbCursorSection)
{
    const char* gopId = NULL;
    const char* gwinId = NULL;
    const char* gopDst = NULL;
    const char* gwinFmt = NULL;
    const char* outputColorSpace = NULL;
    u8 u8GopDst,u8GwinFmt,u8OutputColor;
    const MI_SYSCFG_MmapInfo_t* pstMmapInfo = NULL;
    const char* mmapName = NULL;

    gopId = get_key_value(fbCursorSection, "FB_HWLAYER_ID");
    CHECK_IFNOT(kstrtou8(gopId, 10, &(fbHwCursorInfo->u8GopIdx)), 0, -1)

    gwinId = get_key_value(fbCursorSection, "FB_HWWIN_ID");
    CHECK_IFNOT(kstrtou8(gwinId, 10, &(fbHwCursorInfo->u8GwinId)), 0, -1)

    gopDst = get_key_value(fbCursorSection, "FB_HWLAYER_DST");
    CHECK_IFNOT(kstrtou8(gopDst,10, &u8GopDst), 0, -1)
    fbHwCursorInfo->eGopDst = (DRV_FB_GOP_DstType_e)u8GopDst;

    gwinFmt = get_key_value(fbCursorSection, "FB_HWWIN_FORMAT");
    CHECK_IFNOT(kstrtou8(gwinFmt,10, &u8GwinFmt), 0, -1 )
    fbHwCursorInfo->stGwinInfo.clrType = (DRV_FB_GOP_ColorFmt_e)u8GwinFmt;
    //get mmap config from fbdev.ini
    mmapName = get_key_value(fbCursorSection, "FB_MMAP_NAME");
    if (mmapName != NULL)
    {
        MI_SYSCFG_GetMmapInfo(mmapName, &pstMmapInfo);
        if (pstMmapInfo)
        {
            fbHwCursorInfo->hwCursorAddr[0] = pstMmapInfo->u32Addr;
            fbHwCursorInfo->hwCursorAddr[1] = pstMmapInfo->u32Addr + 0x10000;
            fbHwCursorInfo->eMiuSel = pstMmapInfo->u8MiuNo;
            FBDEV_INFO("parse_hwcursorInfo use %s in mmap.ini to store cursor icon!\n",mmapName);
        }
        else
        {
            FBDEV_INFO("Can not find %s section in mmap.ini,use default setting\n",mmapName);
        }
    }
    else
    {
        FBDEV_INFO("\33[0;36m parse_hwcursorInfo did not config memory in fbdev.ini,Use OSD Bottom to store cursor Icon! \33[m \n");
    }
    //for hwcursor pitch was fixed set as 128x4
    fbHwCursorInfo->stGwinInfo.u16Pitch = 512;
    outputColorSpace = get_key_value(fbCursorSection, "FB_HWLAYER_OUTPUTCOLOR");
    CHECK_IFNOT(kstrtou8(outputColorSpace,10, &u8OutputColor), 0, -1)
    fbHwCursorInfo->eOutputColorSpace = (DRV_FB_OutputColorSpace_e)u8OutputColor;
    fbHwCursorInfo->bInitialized = FALSE;
    fbHwCursorInfo->bShown = FALSE;
    //Default set MIUSEL as E_DRV_FB_GOP_SEL_MIU0
    fbHwCursorInfo->eMiuSel = E_DRV_FB_GOP_SEL_MIU0;
    fbHwCursorInfo->u8DstbufIdx = 0;
    //Disable colorkey
    fbHwCursorInfo->stColorKey.bKeyEnable = FALSE;
    //Enalbe alpha blend default and disalbe multi alpha
    fbHwCursorInfo->stAlpha.bAlphaEnable = TRUE;
    fbHwCursorInfo->stAlpha.bAlphaChannel = FALSE;
    return 0;
}

static void setHwcursrPhyAddr(void)
{
    u32 u32PhyAddr = 0;
    if (fbHwCursorInfo)
    {
        if (fbHwCursorInfo->hwCursorAddr[0] == 0)
        {
            if (mstar_fb_fix_infos)
            {
                //0x20000 equal with 128x128x4x2
                u32PhyAddr = mstar_fb_fix_infos[0].smem_start
                    + mstar_fb_fix_infos[0].smem_len - 0x20000;
                 fbHwCursorInfo->hwCursorAddr[0] = u32PhyAddr;
                fbHwCursorInfo->hwCursorAddr[1] = u32PhyAddr + 0x10000;
                if (u32PhyAddr > MIU1_INTERVAL)
                {
                    fbHwCursorInfo->eMiuSel = E_DRV_FB_GOP_SEL_MIU1;
                }
                else
                {
                    fbHwCursorInfo->eMiuSel = E_DRV_FB_GOP_SEL_MIU0;
                }
            }
            else
            {
                FBDEV_INFO("setHwcursrPhyAddr failed because did not init osdinfo yet!\n");
            }
       }
       else
       {
             FBDEV_INFO("setHwcursrPhyAddr hwcursor phyaddr has been set!\n");
       }
    }
}
static int parse_hwLayerInfo(IniSectionNode *hwLayerInfo, int seq)
{
    const char* gopId = NULL;
    const char* gwinId = NULL;
    const char* gopDst = NULL;
    const char* gwinFmt = NULL;
    const char* outputColorSpace = NULL;
    const char* fbWidth = NULL;
    const char* fbHeight = NULL;
    const char* timingWidth = NULL;
    const char* timingHeight = NULL;
    u8 u8GopDst,u8GwinFmt,u8OutputColor;
    u16 u16FbWidth,u16FbHeight;
    u16 u16TimingWidth,u16TimingHeight;
    MS_U16 bpx = 4;
    const MI_SYSCFG_MmapInfo_t* pstMmapInfo = NULL;
    const char* mmapName = NULL;
    char strId[16] = {0};
    //double buffer
    u16 bufferCount = 2;
    snprintf(strId, sizeof(strId), "MStar FB%d",seq);
    gopId = get_key_value(hwLayerInfo, "FB_HWLAYER_ID");
    CHECK_IFNOT(kstrtou8(gopId, 10, &(fbHwlayerInfos[seq].u8GopIdx)), 0, -1)

    gwinId = get_key_value(hwLayerInfo, "FB_HWWIN_ID");
    CHECK_IFNOT(kstrtou8(gwinId, 10, &(fbHwlayerInfos[seq].u8GwinId)), 0, -1)

    gopDst = get_key_value(hwLayerInfo, "FB_HWLAYER_DST");
    CHECK_IFNOT(kstrtou8(gopDst, 10, &u8GopDst), 0, -1)
    fbHwlayerInfos[seq].eGopDst = (MI_FB_DstDisplayplane_e)u8GopDst;

    gwinFmt = get_key_value(hwLayerInfo, "FB_HWWIN_FORMAT");
    CHECK_IFNOT(kstrtou8(gwinFmt, 10, &u8GwinFmt), 0, -1)
    fbHwlayerInfos[seq].stGwinInfo.clrType = (DRV_FB_GOP_ColorFmt_e)u8GwinFmt;
    bpx =  mstar_FB_GetBpp(u8GwinFmt);

    outputColorSpace = get_key_value(hwLayerInfo, "FB_HWLAYER_OUTPUTCOLOR");
    CHECK_IFNOT(kstrtou8(outputColorSpace, 10, &u8OutputColor), 0, -1)
    fbHwlayerInfos[seq].eOutputColorSpace = (DRV_FB_OutputColorSpace_e)u8OutputColor;

    fbWidth = get_key_value(hwLayerInfo, "FB_WIDTH");
    CHECK_IFNOT(kstrtou16(fbWidth, 10, &u16FbWidth), 0, -1)
    //Alignment fbwidth down to 16 pixel
    u16FbWidth  = ALIGN_DOWNTO_16(u16FbWidth);
    fbHwlayerInfos[seq].stGwinInfo.u16HStart = 0;
    fbHwlayerInfos[seq].stGwinInfo.u16HEnd = u16FbWidth;

    fbHeight = get_key_value(hwLayerInfo, "FB_HEIGHT");
    CHECK_IFNOT(kstrtou16(fbHeight, 10, &u16FbHeight), 0, -1)
    fbHwlayerInfos[seq].stGwinInfo.u16VEnd= u16FbHeight;
    fbHwlayerInfos[seq].stGwinInfo.u16Pitch = u16FbWidth * bpx;

    //Stretch Window Info
    fbHwlayerInfos[seq].stStretchWinInfo.u16Xpos = 0;
    fbHwlayerInfos[seq].stStretchWinInfo.u16Ypos = 0;
    fbHwlayerInfos[seq].stStretchWinInfo.u16SrcWidth = u16FbWidth;
    fbHwlayerInfos[seq].stStretchWinInfo.u16SrcHeight= u16FbHeight;
    timingWidth = get_key_value(hwLayerInfo, "FB_TIMMING_WIDTH");
    CHECK_IFNOT(kstrtou16(timingWidth, 10, &u16TimingWidth), 0, -1)
    fbHwlayerInfos[seq].stStretchWinInfo.u16DstWidth = u16TimingWidth;
    timingHeight = get_key_value(hwLayerInfo, "FB_TIMMING_HEIGHT");
    CHECK_IFNOT(kstrtou16(timingHeight, 10, &u16TimingHeight), 0, -1)
    fbHwlayerInfos[seq].stStretchWinInfo.u16DstHeight = u16TimingHeight;
    //get mmap config
    mmapName = get_key_value(hwLayerInfo, "FB_MMAP_NAME");
    MI_SYSCFG_GetMmapInfo(mmapName, &pstMmapInfo);
    if (pstMmapInfo != NULL)
    {
        fbHwlayerInfos[seq].phyAddr = pstMmapInfo->u32Addr;
        fbHwlayerInfos[seq].eMiuSel = (DRV_FB_GOP_MiuSel_e)pstMmapInfo->u8MiuNo;
        mstar_fb_fix_infos[seq].smem_len = pstMmapInfo->u32Size;
    }
    else
    {
        FBDEV_INFO("fb%d parse_hwLayerInfo fail can not find mmap %s\n",seq,mmapName);
        return -1;
    }
    fbHwlayerInfos[seq].bShown = FALSE;
    //Disable colorkey default
    fbHwlayerInfos[seq].stColorKey.bKeyEnable = FALSE;
    //Enalbe alpha blend default
    fbHwlayerInfos[seq].stAlpha.bAlphaEnable = TRUE;
    fbHwlayerInfos[seq].stAlpha.bAlphaChannel = FALSE;
    //init mstar_fb_fix_infos
    memcpy(mstar_fb_fix_infos[seq].id,strId,strlen(strId));
    mstar_fb_fix_infos[seq].type = FB_TYPE_PACKED_PIXELS;
    mstar_fb_fix_infos[seq].visual = FB_VISUAL_TRUECOLOR;
    mstar_fb_fix_infos[seq].xpanstep = 1;
    mstar_fb_fix_infos[seq].ypanstep = 1;
    mstar_fb_fix_infos[seq].ywrapstep = 1;
    mstar_fb_fix_infos[seq].line_length =
            fbHwlayerInfos[seq].stGwinInfo.u16Pitch;
    /**
      *mmio_start and mmio_len was used in fb_mmap
      *If fb_mmap has been implement by vendor,it'll
      *invoke vender implement.the mstar_fb_mmap is
      *the implementation of MStar, it's not necessary to
      *init mmio_start and mmio_len. So hardcode it.
      */
    mstar_fb_fix_infos[seq].mmio_start = 0x08000000;
    mstar_fb_fix_infos[seq].mmio_len = 0x10000000;
    mstar_fb_fix_infos[seq].accel = FB_ACCEL_NONE;
    //framebuffer size and phyaddr
    mstar_fb_fix_infos[seq].smem_start = fbHwlayerInfos[seq].phyAddr;
    //init mstar_fb_var_infos
    mstar_fb_var_infos[seq].xres = u16FbWidth;
    mstar_fb_var_infos[seq].yres = u16FbHeight;
    mstar_fb_var_infos[seq].xres_virtual = mstar_fb_var_infos[seq].xres;
    //double buffer
    bufferCount = getBufferCount(mstar_fb_fix_infos[seq].smem_len,
                        u16FbWidth, u16FbHeight, bpx);
    mstar_fb_var_infos[seq].yres_virtual = u16FbHeight * bufferCount;
    FBDEV_INFO("fb%d parse_hwLayerInfo buffercount=%d,mstar_fb_var_infos[seq].yres_virtual=%d\n",
        seq,bufferCount,mstar_fb_var_infos[seq].yres_virtual);
    mstar_fb_var_infos[seq].bits_per_pixel = bpx << 3;
    mstar_fb_var_infos[seq].activate = FB_ACTIVATE_NOW;
    set_fb_bitfield((MI_FB_ColorFmt_e)u8GwinFmt, &mstar_fb_var_infos[seq]);
    //set width, heigth as maxium
    mstar_fb_var_infos[seq].width = -1;
    mstar_fb_var_infos[seq].height = -1;
    mstar_fb_var_infos[seq].grayscale = 0;
    /*timing useless? use the vfb default */
    mstar_fb_var_infos[seq].pixclock
        = 100000000000LLU / (6 *  1920 * 1080);
    mstar_fb_var_infos[seq].left_margin = 64;
    mstar_fb_var_infos[seq].right_margin = 64;
    mstar_fb_var_infos[seq].upper_margin = 32;
    mstar_fb_var_infos[seq].lower_margin = 32;
    mstar_fb_var_infos[seq].hsync_len  =64;
    mstar_fb_var_infos[seq].vsync_len = 2;
    mstar_fb_var_infos[seq].vmode  = FB_VMODE_NONINTERLACED;
    return 0;
}
static void calHwLayerCount(IniSectionNode *root, IniSectionNode *node, const char *name)
{
    if (!strcmp(name,hwlayer_section_name)) {
        numFbHwlayer++;
    }
}
static int parse_hwLayerZorder(IniSectionNode *fbLayerZorderSection)
{
    const char* gopIdx = NULL;
    u32 u32GopIdx = 0;
    int i=0;
    char layerId[14] = {0};
    for( i=0; i < MAX_HWLAYER_CNT; i++) {
        snprintf(layerId, sizeof(layerId),"LAYER_ZORDER%d",i);
        gopIdx = get_key_value(fbLayerZorderSection, layerId);
        CHECK_IFNOT(kstrtou32(gopIdx, 10, &u32GopIdx), 0, -1)
        hwlayer_gopIdx[i] = u32GopIdx;
        hwlayer_layerIdx[i] = i;
        FBDEV_INFO("%s:%d hwlayer_gopIdx[%d]=%d,hwlayer_layerIdx[%d]=%d\n",
            __FUNCTION__,__LINE__,i,hwlayer_gopIdx[i],i,hwlayer_layerIdx[i]);
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int __init mstar_fb_probe(struct platform_device *dev)
{
    struct fb_info *pinfo = NULL;
    struct fb_info **pFbInfos = NULL;
    struct fb_info *pCurrentFbInfo = NULL;
    int retval = 0;
    IniSectionNode* root = NULL;
    IniSectionNode* fbCursorSection = NULL;
    IniSectionNode* fbDevSection = NULL;
    IniSectionNode* fbLayerZorderSection = NULL;
    char* contents = NULL;
    int i = 0;
    int j = 0;
    int ret = 0;
    if (NULL == dev)
    {
        printk("ERROR: in mstar_fb_prob: dev is NULL pointer \r\n");
        return -ENOTTY;
    }
    ret = readConfile(&contents);
    if (ret < 0) {
        printk(KERN_ERR "ERROR: in mstar_fb_prob: read fbdev.ini failed\n");
        return -1;
    }
    if (alloc_and_init_ini_tree(&root, contents)) {
        retval= -1;
        goto out;
    }
    dump_ini(root);
    //parse hwcursor info
    fbCursorSection = get_section(root, "FB_CURSOR");
    if (fbCursorSection) {
        fbHwCursorInfo = (HwCursorInfo_t*)kzalloc(sizeof(HwCursorInfo_t), GFP_KERNEL);
        if (IS_ERR_OR_NULL(fbHwCursorInfo)) {
            printk(KERN_ERR "allocate memroy for HwCursorInfo_t failed!\n");
            retval = -1;
            goto out;
        }
        parse_hwcursorInfo(fbCursorSection);
    }
    //parse GOP LayerSetting
    fbLayerZorderSection = get_section(root, "LAYER_ZORDER");
    if (fbLayerZorderSection) {
        parse_hwLayerZorder(fbLayerZorderSection);
    }
    //parse fbdev info
    foreach_section(root, calHwLayerCount);
    FBDEV_INFO("Mstar frame buffer device,numFbHwlayer:%d\n",numFbHwlayer);
    if (numFbHwlayer > 0) {
        fbHwlayerInfos = (HwLayerInfo_t*)kzalloc(numFbHwlayer* sizeof(HwLayerInfo_t),GFP_KERNEL);
        if (IS_ERR_OR_NULL(fbHwlayerInfos)) {
            printk(KERN_ERR "allocate memory for HwLayerInfo_t failed!\n");
            retval = -1;
            goto out;
        }
        mstar_fb_fix_infos = (struct fb_fix_screeninfo*)kzalloc(
            numFbHwlayer * sizeof(struct fb_fix_screeninfo), GFP_KERNEL);
        if (IS_ERR_OR_NULL(mstar_fb_fix_infos)) {
            printk(KERN_ERR "allocate memory for mstar_fb_fix_infos failed!\n");
            retval = -1;
            goto out;
        }
        mstar_fb_var_infos = (struct fb_var_screeninfo*)kzalloc(
            numFbHwlayer*sizeof(struct fb_var_screeninfo), GFP_KERNEL);
        if (IS_ERR_OR_NULL(mstar_fb_var_infos)) {
            printk(KERN_ERR "allocate memory for mstar_fb_var_infos failed!\n");
            retval = -1;
            goto out;
        }
        pFbInfos = (struct fb_info**)kzalloc(
            numFbHwlayer * sizeof(struct fb_info*), GFP_KERNEL);
        if (IS_ERR_OR_NULL(pFbInfos)) {
            printk(KERN_ERR "allocate memory for pFbInfos failed!\n");
            retval = -1;
            goto out;
        }
        fbDevSection = get_section(root, "FB_DEVICE");
        for (i = 0; i < numFbHwlayer; i++) {
            retval = parse_hwLayerInfo(fbDevSection, i);
            if (retval==-1)
                goto out;
            fbDevSection = fbDevSection->next;
        }
        setHwcursrPhyAddr();
        //resister framebufferInfo and init GOP && GWIN
        for (i = 0; i <  numFbHwlayer; i++) {
            pinfo = framebuffer_alloc(sizeof(HwLayerInfo_t), &dev->dev);
            if (!pinfo) {
                printk(KERN_ERR "framebuffer_alloc:%d failed!\n", i);
                retval = -1;
                break;
            }
            pinfo->fbops = &mstar_fb_ops;
            pinfo->var = mstar_fb_var_infos[i];
            pinfo->fix = mstar_fb_fix_infos[i];
            pinfo->pseudo_palette = fbHwlayerInfos[i].pseudo_palette;
            memcpy(pinfo->par, &(fbHwlayerInfos[i]), sizeof(HwLayerInfo_t));
            pinfo->flags = FBINFO_FLAG_DEFAULT;
            pFbInfos[i] = pinfo;
        }
        //If framebuffer_alloc fail, release fb_info already allocated
        if (i < numFbHwlayer) {
            for (j = 0; j < i; j++) {
                pCurrentFbInfo = pFbInfos[j];
                framebuffer_release(pCurrentFbInfo);
            }
            pCurrentFbInfo = NULL;
            goto out;
        }
        //fb_alloc_cmap for all fb_info
        for (i = 0; i < numFbHwlayer; i++) {
            pCurrentFbInfo = pFbInfos[i];
            retval  = fb_alloc_cmap(&pCurrentFbInfo->cmap, 256, 0);
            if (retval < 0) {
                printk(KERN_ERR "fb_alloc_cmap:%d failed!\n", i);
                break;
            }
        }
        if (i < numFbHwlayer) {
            //release cmap and fb_info already been alocated
            for (j = 0; j < i; j++) {
                pCurrentFbInfo = pFbInfos[j];
                fb_dealloc_cmap(&pCurrentFbInfo->cmap);
            }
            for (j=0; j < numFbHwlayer; j++) {
                pCurrentFbInfo = pFbInfos[j];
                framebuffer_release(pCurrentFbInfo);
            }
            pCurrentFbInfo = NULL;
            goto out;
        }
        //register framebuffer
        for (i = 0; i < numFbHwlayer; i++) {
            pCurrentFbInfo = pFbInfos[i];
            retval = register_framebuffer(pCurrentFbInfo);
            if (retval < 0) {
                printk(KERN_ERR "register_framebuffer:%d failed!\n", i);
                break;
            }
        }
        if ( i < numFbHwlayer) {
            //unregister framebuffer already registerd
            for (j = 0; j < i; j++) {
                pCurrentFbInfo = pFbInfos[j];
                unregister_framebuffer(pCurrentFbInfo);
            }
            //release cmap and fb_info
            for (j = 0; j < numFbHwlayer; j++) {
                pCurrentFbInfo = pFbInfos[j];
                fb_dealloc_cmap(&pCurrentFbInfo->cmap);
                framebuffer_release(pCurrentFbInfo);
            }
            pCurrentFbInfo = NULL;
            goto out;
        }
        //save pFirstFbInfo as private data
        platform_set_drvdata(dev, pFbInfos);
        first_fb_node = pFbInfos[0]->node;
        mstar_FB_InitContext();
#ifdef MI_SYS_PROC_FS_DEBUG
    MI_Moduledev_RegisterDev();
#endif
        FBDEV_INFO("fb%d: Mstar frame buffer device\n",pFbInfos[0]->node);
    }
    out:
        if (retval < 0) {
            if (fbHwCursorInfo) {
                kfree(fbHwCursorInfo);
                fbHwCursorInfo = NULL;
            }
            if (fbHwlayerInfos) {
                kfree(fbHwlayerInfos);
                fbHwlayerInfos = NULL;
            }
            if (pFbInfos) {
                kfree(pFbInfos);
                pFbInfos = NULL;
            }
        }
        if (mstar_fb_fix_infos) {
                kfree(mstar_fb_fix_infos);
                mstar_fb_fix_infos = NULL;
            }
        if (mstar_fb_var_infos) {
            kfree(mstar_fb_var_infos);
            mstar_fb_var_infos = NULL;
        }
        release_ini_tree(root);
        kfree(contents);
    return retval;
}
static int mstar_fb_remove(struct platform_device *dev)
{
    struct fb_info **pinfo;
    int i=0;
    pinfo = 0;
    if (NULL == dev)
    {
        printk("ERROR: mstar_fb_remove: dev is NULL pointer \n");
        return -ENOTTY;
    }
    for (i = 0;  i < numFbHwlayer; i++) {
        _fb_gwin_enable(fbHwlayerInfos[i].u8GopIdx, fbHwlayerInfos[i].u8GwinId, FALSE);
        fbHwlayerInfos[i].bShown  =FALSE;
    }
    pinfo = platform_get_drvdata(dev);
    for (i = 0;  i < numFbHwlayer; i++) {
        FBDEV_INFO("%s:%d unregister and release fb:%d\n",
            __FUNCTION__,__LINE__,pinfo[i]->node);
        unregister_framebuffer(pinfo[i]);
        fb_dealloc_cmap(&pinfo[i]->cmap);
        framebuffer_release(pinfo[i]);
    }
    if (fbHwCursorInfo) {
        kfree(fbHwCursorInfo);
        fbHwCursorInfo = NULL;
    }
    if (fbHwlayerInfos) {
        kfree(fbHwlayerInfos);
       fbHwlayerInfos = NULL;
    }
    if (mstar_fb_fix_infos) {
        kfree(mstar_fb_fix_infos);
        mstar_fb_fix_infos = NULL;
    }
    if (mstar_fb_var_infos) {
        kfree(mstar_fb_var_infos);
        mstar_fb_var_infos = NULL;
    }
    if (pinfo) {
        kfree(pinfo);
        pinfo = NULL;
    }
    platform_set_drvdata(dev, NULL);
#ifdef MI_SYS_PROC_FS_DEBUG
    if (gFbdevHdl) {
        mi_sys_UnRegisterDev(gFbdevHdl);
    }
#endif
    return 0;
}
static void mstar_fb_platform_release(struct device *device)
{
    if (NULL == device)
    {
        printk("ERROR: in mstar_fb_platform_release, \
                device is NULL pointer !\r\n");
    }
    else
    {
        printk("in mstar_fb_platform_release, module unload!\n");
    }
}
/*device .name and driver .name must be the same, then it will call
       probe function */
static struct platform_driver Mstar_fb_driver =
{
    .probe  = mstar_fb_probe,    //initiailize
    .remove = mstar_fb_remove,   /*it free(mem),
                                   release framebuffer, free irq etc. */
    .driver =
    {
        .name = "Mstar-fb",
    },
};

static u64 mstar_fb_device_lcd_dmamask = 0xffffffffUL;

static struct platform_device Mstar_fb_device =
{
    .name = "Mstar-fb",
    .id = 0,
    .dev =
    {
        .release = mstar_fb_platform_release,
        .dma_mask = &mstar_fb_device_lcd_dmamask,
        .coherent_dma_mask = 0xffffffffUL
    }
};
static int __init mstar_fbdev_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&Mstar_fb_driver);

    if (!ret)
    {
        /*register driver sucess
          register device*/
        ret = platform_device_register(&Mstar_fb_device);
        if(ret)    /*if register device fail, then unregister the driver.*/
        {
            platform_driver_unregister(&Mstar_fb_driver);
        }
    }
    return ret;
}

static void __exit mstar_fbdev_exit(void)
{
    platform_device_unregister(&Mstar_fb_device);
    platform_driver_unregister(&Mstar_fb_driver);
}
static int mstar_fb_open(struct fb_info *info, int user)
{
    HwLayerInfo_t* par = info->par;
    //Triger by userspace api
    if (user)
    {
        if (!par->ref_count)
        {
            if (0 != info->fix.smem_start)
            {
                FBDEV_INFO("mstar_fb_open fb:%d info->fix.smem_start = 0x%lx,user=%d\n",info->node,info->fix.smem_start,user);
                restoreFbInfo(info);
                _fb_buf_init(info, info->fix.smem_start);
                _fb_gwin_init(info);
                _fb_strewin_update(&info->var,par->u8GopIdx, 0, 0,
                    par->stStretchWinInfo.u16DstWidth,par->stStretchWinInfo.u16DstHeight);
                mstar_FB_CreateFBInfo(par->u8GwinId, info->var.xres, info->var.yres,
                    par->stGwinInfo.clrType, info->fix.line_length, info->fix.smem_start);
            }
       }
      par->ref_count++;
   }
   else
   {
        FBDEV_INFO("mstar_fb_open fb:%d user=%d\n",info->node,user);
   }
   return 0;
}
static int mstar_fb_release(struct fb_info *info, int user)
{
    HwLayerInfo_t* par = info->par;
    //Trigered by userspace app
    if (user)
    {
        FBDEV_INFO("mstar_fb_release fb:%d user=%d,par->ref_count=%d\n",info->node,user,par->ref_count);
        if (!par->ref_count)
            return -EINVAL;
        if (par->ref_count ==1) {
            if (info->screen_base != NULL) {
                #if 0
                iounmap(info->screen_base);
                #else
                mi_sys_UnVmap(info->screen_base);
                #endif
                info->screen_base = NULL;
            }
            //Temp solution, restore hwcursor info
            _fb_hwcursor_deinit();
        }
        par->ref_count--;
    }
    else
    {
        FBDEV_INFO("mstar_fb_release fb:%d user=%d",info->node,user);
    }
    return 0;
}

static int mstar_fb_mmap(struct fb_info *pinfo, struct vm_area_struct *vma)
    {
      size_t size;
      HwLayerInfo_t* par = pinfo->par;
      DRV_FB_GOP_MiuSel_e emiuSel =  par->eMiuSel;
      size = 0;
      if (NULL == pinfo)
      {
         printk("ERROR: mstar_fb_mmap, pinfo is NULL pointer !\n");
         return -ENOTTY;
      }
      if (NULL == vma)
      {
          printk("ERROR: mstar_fb_mmap, vma is NULL pointer !\n");
          return -ENOTTY;
      }
      if (0 == pinfo->fix.smem_start)
      {
          printk("ERROR: mstar_fb_mmap, physical addr is NULL pointer !\n");
          return -ENOTTY;
      }

      size = vma->vm_end - vma->vm_start;
      if (emiuSel == E_DRV_FB_GOP_SEL_MIU0) {
            vma->vm_pgoff = (pinfo->fix.smem_start + MIU0_BUS_OFFSET) >> PAGE_SHIFT;
      } else if (emiuSel == E_DRV_FB_GOP_SEL_MIU1) {
            vma->vm_pgoff = (pinfo->fix.smem_start - MIU1_INTERVAL + MIU1_BUS_OFFSET) >> PAGE_SHIFT;
      }
      FBDEV_INFO(
        "mstar_fb_mmap vma->vm_start=%x\n vma->vm_end=%x\n vma->vm_pgoff =%x\n",
        (unsigned int) vma->vm_start, (unsigned int)vma->vm_end ,
        (unsigned int)vma->vm_pgoff);

       //set page to no cache
       #if defined(CONFIG_MIPS)
       {
            pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
            pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;
       }
       #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
       {
            //vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
            pgprot_val(vma->vm_page_prot) = pgprot_noncached(vma->vm_page_prot);
       }
       #endif
      // Remap-pfn-range will mark the range VM_IO and VM_RESERVED

      if (remap_pfn_range(vma, vma->vm_start,
          vma->vm_pgoff, size, vma->vm_page_prot))
      return -EAGAIN;

      return 0;
}
static int mstar_fb_ioctl(struct fb_info *pinfo, MS_U32 u32Cmd, unsigned long u32Arg)
{
    int retval = 0;
    unsigned int dir;
    HwLayerInfo_t* par = pinfo->par;
    int curTimingWidth = 1920;
    int curTimingHeight = 1080;
    int hstart = 192;
    MS_U8 gopId = par->u8GopIdx;
    MS_U8 gwinId = par->u8GwinId;
    DRV_FB_GOP_ColorFmt_e currentFmt = par->stGwinInfo.clrType;
    MS_U16 stretchWinXpos = par->stStretchWinInfo.u16Xpos;
    MS_U16 stretchWinYpos = par->stStretchWinInfo.u16Ypos;
    MS_U16 stretchWinSrcWidth = par->stStretchWinInfo.u16SrcWidth;
    MS_U16 stretchWinSrcHeight = par->stStretchWinInfo.u16SrcHeight;
    MS_U16 stretchWinDstWidth = par->stStretchWinInfo.u16DstWidth;
    MS_U16 stretchWinDstHeight = par->stStretchWinInfo.u16DstHeight;
    int i = 0;
    MS_U16 bpx = 4;
    MS_U32 lineLength = pinfo->fix.line_length;
    union
    {
        MI_FB_Rectangle_t dispRegion;
        MI_FB_GlobalAlpha_t alpahInfo;
        MI_FB_ColorKey_t colorKeyInfo;
        MI_FB_DisplayLayerAttr_t dispLayerAttr;
        MI_FB_CursorAttr_t hwcursorAttr;
        MS_BOOL bShown;
    }data;
    //How to get op1 timming??
    mstar_FB_getCurOPTiming(&curTimingWidth, &curTimingHeight, &hstart);
    if (_IOC_TYPE(u32Cmd) != FB_IOC_MAGIC) {
        return -ENOTTY;
    }
    if (_IOC_SIZE(u32Cmd) > sizeof(data)) {
        return  -EINVAL;
    }
    dir = _IOC_DIR(u32Cmd);
    if (dir & _IOC_WRITE) {
        if (copy_from_user(&data, (void __user*)u32Arg,_IOC_SIZE(u32Cmd)))
            return -EFAULT;
    }
    switch(u32Cmd) {
        case FBIOGET_SCREEN_LOCATION:
        {
            data.dispRegion.u16Xpos = stretchWinXpos;
            data.dispRegion.u16Ypos = stretchWinYpos;
            data.dispRegion.u16Width = stretchWinDstWidth;
            data.dispRegion.u16Height= stretchWinDstHeight;
            retval  = copy_to_user((MI_FB_Rectangle_t __user*)u32Arg,
                &data.dispRegion, sizeof(MI_FB_Rectangle_t));
        }
        break;
        case FBIOSET_SCREEN_LOCATION:
        {
            MS_BOOL xPosEqual =
                (data.dispRegion.u16Xpos == stretchWinXpos);
            MS_BOOL yPosEqual =
                (data.dispRegion.u16Ypos == stretchWinYpos);
            MS_BOOL widthEqual =
                (data.dispRegion.u16Width == stretchWinDstWidth);
            MS_BOOL heightEqual =
                (data.dispRegion.u16Height== stretchWinDstHeight);
            if (xPosEqual && yPosEqual
                && widthEqual && heightEqual) {
                return retval;
            }
            if ((data.dispRegion.u16Width < stretchWinSrcWidth)
                 || (data.dispRegion.u16Height < stretchWinSrcHeight)) {
                FBDEV_INFO("fb%d FBIOSET_SCREEN_LOCATION parameter is invalid srcSize[%d,%d] is larger than destSize[%d,%d]\n",
                    pinfo->node,stretchWinSrcWidth,stretchWinSrcHeight, data.dispRegion.u16Width, data.dispRegion.u16Height);
                return -EINVAL;
            }
            if (data.dispRegion.u16Xpos + data.dispRegion.u16Width > curTimingWidth) {
                 FBDEV_INFO(
                 "fb%d FBIOSET_SCREEN_LOCATION param is invalid the sum of xpos and width: %d should not  larger than current timming width: %d\n",
                    pinfo->node,data.dispRegion.u16Xpos + data.dispRegion.u16Width,curTimingWidth);
                return -EINVAL;
            }
            if (data.dispRegion.u16Ypos + data.dispRegion.u16Height > curTimingHeight) {
               FBDEV_INFO(
                "fb%d FBIOSET_SCREEN_LOCATION param is invalid the sum of ypos and height: %d should not  larger than current timming height: %d\n",
                    pinfo->node,data.dispRegion.u16Ypos + data.dispRegion.u16Height,curTimingHeight);
                return -EINVAL;
            }
            mstar_FB_BeginTransaction(gopId);
            _fb_strewin_update(&pinfo->var,gopId,
                data.dispRegion.u16Xpos,data.dispRegion.u16Ypos,data.dispRegion.u16Width,data.dispRegion.u16Height);
            mstar_FB_EndTransaction(gopId);
            //Store stretchwindow information
            par->stStretchWinInfo.u16Xpos = data.dispRegion.u16Xpos;
            par->stStretchWinInfo.u16Ypos = data.dispRegion.u16Ypos;
            par->stStretchWinInfo.u16DstWidth = data.dispRegion.u16Width;
            par->stStretchWinInfo.u16DstHeight  = data.dispRegion.u16Height;
        }
        break;
        case FBIOGET_SHOW:
        {
            retval =
                __put_user(par->bShown, (MS_BOOL __user*)u32Arg);
        }
        break;
        case FBIOSET_SHOW:
        {
            MS_BOOL bNeedShow = data.bShown;
            if (bNeedShow != par->bShown) {
                mstar_FB_BeginTransaction(gopId);
                _fb_gwin_enable(gopId,gwinId,bNeedShow);
                mstar_FB_EndTransaction(gopId);
                par->bShown = bNeedShow;
            }
        }
        break;
        case FBIOGET_GLOBAL_ALPHA:
        {
             retval  = copy_to_user((MI_FB_GlobalAlpha_t __user*)u32Arg,
                &(par->stAlpha), sizeof(MI_FB_GlobalAlpha_t));
        }
        break;
        case FBIOSET_GLOBAL_ALPHA:
        {
            if (!memcmp(&(par->stAlpha),
                &data.alpahInfo, sizeof(MI_FB_GlobalAlpha_t))){
                return retval;
            }
            if (!data.alpahInfo.bAlphaEnable) {
                mstar_FB_BeginTransaction(gopId);
                mstar_FB_SetBlending(gopId, gwinId, FALSE, 0xFF);
                mstar_FB_EndTransaction(gopId);
            } else {
                mstar_FB_BeginTransaction(gopId);
                if (currentFmt == E_DRV_FB_GOP_COLOR_ARGB1555) {
#if SUPPORT_SET_ARGB1555_ALPHA
                    mstar_FB_SetAlpha0(gwinId, data.alpahInfo.u8Alpha0);
                    mstar_FB_SetAlpha1(gwinId, data.alpahInfo.u8Alpha1);
#else
                    FBDEV_INFO("This platform can not support set pixel alpha for ARGB1555");
#endif
                }
                if (data.alpahInfo.bAlphaChannel) {
#if SUPPORT_SET_ARGB1555_ALPHA
                    mstar_FB_SetBlending(gopId, gwinId, TRUE, data.alpahInfo.u8GlobalAlpha);
                    if (mstar_FB_IsSupportMultiAlpha(gopId)) {
                            mstar_FB_EnableMultiAlpha(gopId, TRUE);
                    }
#else
                    if (currentFmt==E_DRV_FB_GOP_COLOR_ARGB1555) {
                         mstar_FB_SetBlending(gopId, gwinId, FALSE, data.alpahInfo.u8GlobalAlpha);
                    }
                    else {
                        mstar_FB_SetBlending(gopId, gwinId, TRUE, data.alpahInfo.u8GlobalAlpha);
                        if (mstar_FB_IsSupportMultiAlpha(gopId)) {
                            mstar_FB_EnableMultiAlpha(gopId, TRUE);
                        }
                    }
#endif
                } else {
                    if (par->stAlpha.bAlphaChannel) {
                        if (mstar_FB_IsSupportMultiAlpha(gopId)) {
                            mstar_FB_EnableMultiAlpha(gopId, FALSE);
                        }
                    }
                }
                mstar_FB_EndTransaction(gopId);
                memcpy(&(par->stAlpha), &data.alpahInfo, sizeof(MI_FB_GlobalAlpha_t));
            }
        }
        break;
        case FBIOGET_COLORKEY:
        {
            retval = copy_to_user((MI_FB_ColorKey_t __user*)u32Arg,
                &(par->stColorKey), sizeof(MI_FB_ColorKey_t));
        }
        break;
        case FBIOSET_COLORKEY:
        {
            MS_U32 fullclr = 0;
            if (!memcmp(&(par->stColorKey),
                &data.colorKeyInfo, sizeof(MI_FB_ColorKey_t)) ){
                return retval;
            }
            if ((currentFmt == E_DRV_FB_GOP_COLOR_YUV422) ||
                 (currentFmt == E_DRV_FB_GOP_COLOR_INVALID) ) {
                FBDEV_INFO("fb%d can not set colorkey prop for clrFmt:%d\n",pinfo->node,currentFmt);
                return  -EINVAL;
            }
            mstar_FB_BeginTransaction(gopId);
            if (!data.colorKeyInfo.bKeyEnable) {
                mstar_FB_EnableTransClr_EX(gopId, E_DRV_FB_GOPTRANSCLR_FMT0, FALSE);
            } else {
                mstar_FB_EnableTransClr_EX(gopId, E_DRV_FB_GOPTRANSCLR_FMT0, TRUE);
                fullclr = (0xff<<24) | (data.colorKeyInfo.u8Red<<16)
                    | (data.colorKeyInfo.u8Green<<8) | (data.colorKeyInfo.u8Blue);
                mstar_FB_SetTransClr_8888(gopId,fullclr,0);
            }
            mstar_FB_EndTransaction(gopId);
            memcpy(&(par->stColorKey), &data.colorKeyInfo, sizeof(MI_FB_ColorKey_t));
        }
        break;
        case FBIOGET_DISPLAYLAYER_ATTRIBUTES:
        {
            data.dispLayerAttr.u32Xpos = stretchWinXpos;
            data.dispLayerAttr.u32YPos = stretchWinYpos;
            data.dispLayerAttr.u32dstWidth = stretchWinDstWidth;
            data.dispLayerAttr.u32dstHeight = stretchWinDstHeight;
            data.dispLayerAttr.u32DisplayWidth = stretchWinSrcWidth;
            data.dispLayerAttr.u32DisplayHeight = stretchWinSrcHeight;
            data.dispLayerAttr.u32ScreenWidth = curTimingWidth;
            data.dispLayerAttr.u32ScreenHeight = curTimingHeight;
            data.dispLayerAttr.eFbColorFmt = (MI_FB_ColorFmt_e)currentFmt;
            data.dispLayerAttr.eFbOutputColorSpace = par->eOutputColorSpace;
            data.dispLayerAttr.eFbDestDisplayPlane = (MI_FB_DstDisplayplane_e)par->eGopDst;
            data.dispLayerAttr.bPreMul = par->bPremultiply;
            data.dispLayerAttr.u32SetAttrMask = 0x0;
            retval = copy_to_user((MI_FB_DisplayLayerAttr_t __user*)u32Arg,
                &data.dispLayerAttr, sizeof(MI_FB_DisplayLayerAttr_t));
        }
        break;
        case FBIOSET_DISPLAYLAYER_ATTRIBUTES:
        {
            MS_U16 dispXpos = stretchWinXpos;
            MS_U16 dispYpos = stretchWinYpos;
            MS_U16 dispWidth = stretchWinDstWidth;
            MS_U16 dispHeight = stretchWinDstHeight;
            MS_U16 fbWidth = pinfo->var.xres;
            MS_U16 fbHeight = pinfo->var.yres;
            MS_U16 timingWidth = curTimingWidth;
            MS_U16 timingHeight = curTimingHeight;
            DRV_FB_GOP_ColorFmt_e colorFmt = currentFmt;
            DRV_FB_GwinInfo_t gwinInfo = par->stGwinInfo;
            DRV_FB_StretchWinInfo_t stretchWinInfo = par->stStretchWinInfo;
            MS_BOOL bCurPremul  = par->bPremultiply;
            MS_U8 dispMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS |
                              E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE |
                              E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE |
                              E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB;
            MS_BOOL bNeedUpdateGwinInfo = FALSE;
            MS_BOOL bNeedUpdateStretchWin = FALSE;
            MS_BOOL bNeedUpdateHstart = FALSE;
            MS_BOOL bNeedUpdateColorSpace = FALSE;
            MS_BOOL bNeedUpdateGopDst = FALSE;
            MS_BOOL bNeedUpdatePremul = FALSE;
            MS_U8 surfaceCnt = 2;
            //The xPosition and yPosition of stretchwindow
            if (data.dispLayerAttr.u32SetAttrMask &
                E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS) {
                dispXpos = data.dispLayerAttr.u32Xpos;
                dispYpos = data.dispLayerAttr.u32YPos;
                FBDEV_INFO("E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS dispPos[%d,%d]\n",dispXpos,dispYpos);
            }
            //The dstWidth and dstHeigth of StretchWindow
            if (data.dispLayerAttr.u32SetAttrMask &
                E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE) {
                dispWidth = data.dispLayerAttr.u32dstWidth;
                dispHeight = data.dispLayerAttr.u32dstHeight;
                FBDEV_INFO("E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE dispSize[%d,%d]\n",dispWidth,dispHeight);
            }
            //The framebuffer size
            if (data.dispLayerAttr.u32SetAttrMask &
                E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE) {
                fbWidth = ALIGN_DOWNTO_16(data.dispLayerAttr.u32DisplayWidth);
                fbHeight = data.dispLayerAttr.u32DisplayHeight;
                FBDEV_INFO("E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE fb size[%d,%d]\n",
                    fbWidth,fbHeight);
            }
            if ( (fbWidth <=0) || (fbWidth > curTimingWidth) ||
                (fbHeight <= 0) || (fbHeight > curTimingHeight)) {
                FBDEV_INFO("fbWidth:%d,fbHeight:%d is not invalid,CurrentTiming[%d,%d]\n",
                    fbWidth,fbHeight,curTimingWidth,curTimingHeight);
                return -EINVAL;
            }
            //The colorformat of framebuffer
            if (data.dispLayerAttr.u32SetAttrMask &
                E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB) {
                colorFmt = data.dispLayerAttr.eFbColorFmt;
                if ((colorFmt == (DRV_FB_GOP_ColorFmt_e)E_MI_FB_COLOR_FMT_INVALID) ||
                    (colorFmt == (DRV_FB_GOP_ColorFmt_e)E_MI_FB_COLOR_FMT_YUV422)) {
                    FBDEV_INFO("The request colorfmt : %d is not support!\n",colorFmt);
                    return -EINVAL;
                 }
            }
            //Gop dstplane change
            if (data.dispLayerAttr.u32SetAttrMask &
                E_MI_FB_DISPLAYLAYER_ATTR_MASK_DST_DISP) {
                if ((data.dispLayerAttr.eFbDestDisplayPlane != E_MI_FB_DST_OP0) &&
                    (data.dispLayerAttr.eFbDestDisplayPlane != E_MI_FB_DST_OP1))
                    FBDEV_INFO("The DestDisplayPlane only support OP0 or OP1\n");
                    return -EINVAL;
            }
            mstar_FB_BeginTransaction(gopId);
            if (data.dispLayerAttr.u32SetAttrMask & dispMask) {
                //Hwlimit
                if (dispWidth < fbWidth || dispHeight < fbHeight) {
                    FBDEV_INFO("The framebuffer Rect can not larger than StretchWin Dst!\n");
                    retval = -EINVAL;
                    goto end_transaction;
                }
                if (((dispXpos + dispWidth) > timingWidth)||
                    ((dispYpos + dispHeight) > timingHeight)) {
                    FBDEV_INFO("The StretchWindow display rect can not beyond scrren!\n");
                    retval = -EINVAL;
                    goto end_transaction;
                }
                //Check request resolution is larger than framebuffer mem size
                bpx = mstar_FB_GetBpp(colorFmt);
                lineLength = fbWidth * bpx;
                if (lineLength * fbHeight > pinfo->fix.smem_len) {
                    retval = -ENOMEM;
                    goto end_transaction;
                }
                //update Gwin Info
                gwinInfo.u16HStart = 0;
                gwinInfo.u16HEnd = fbWidth;
                gwinInfo.u16VStart = 0;
                gwinInfo.u16VEnd = fbHeight;
                gwinInfo.clrType = colorFmt;
                if (memcmp(&gwinInfo, &(par->stGwinInfo), sizeof(DRV_FB_GwinInfo_t))) {
                    mstar_FB_DestroyFBInfo(par->u8GwinId);
                    mstar_FB_CreateFBInfo(par->u8GwinId, fbWidth, fbHeight,
                        colorFmt, lineLength, pinfo->fix.smem_start);
                    //gwin addr was set as pinfo->fix.smem_start
                    gwinInfo.u32Addr = pinfo->fix.smem_start;
                    mstar_FB_SetGwinInfo(gopId, gwinId, gwinInfo);
                    bNeedUpdateGwinInfo = TRUE;
                    FBDEV_INFO("Process E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE process\n");
                }
                //update stretchwindow info
                stretchWinInfo.u16Xpos = dispXpos;
                stretchWinInfo.u16Ypos = dispYpos;
                stretchWinInfo.u16SrcWidth = fbWidth;
                stretchWinInfo.u16SrcHeight = fbHeight;
                stretchWinInfo.u16DstWidth = dispWidth;
                stretchWinInfo.u16DstHeight = dispHeight;
                if (memcmp(&stretchWinInfo,
                    &(par->stStretchWinInfo), sizeof(DRV_FB_StretchWinInfo_t))) {
                    mstar_FB_SetStretchWin(gopId, dispXpos, dispYpos, fbWidth, fbHeight);
                    mstar_FB_SetHScale(gopId, TRUE, fbWidth, dispWidth);
                    mstar_FB_SetVScale(gopId, TRUE, fbHeight, dispHeight);
                    bNeedUpdateStretchWin = TRUE;
                    FBDEV_INFO("Process E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE and POS process\n");
                }
            }
            //Premultiply alpha change
            if (data.dispLayerAttr.u32SetAttrMask
                & E_MI_FB_DISPLAYLAYER_ATTR_MASK_PREMUL) {
                if (data.dispLayerAttr.bPreMul != bCurPremul) {
                    mstar_FB_SetNewAlphaMode(gopId, gwinId, data.dispLayerAttr.bPreMul);
                    mstar_FB_SetOSDBlendingFormula(gopId, data.dispLayerAttr.bPreMul);
                    mstar_FB_EnableAlphaInverse(gopId, !data.dispLayerAttr.bPreMul);
                    bNeedUpdatePremul = TRUE;
                }
            }
            //Output colorspace change
            if (data.dispLayerAttr.u32SetAttrMask
                & E_MI_FB_DISPLAYLAYER_ATTR_MASK_OUTPUT_COLORSPACE) {
                if (par->eOutputColorSpace !=
                        (DRV_FB_OutputColorSpace_e)data.dispLayerAttr.eFbOutputColorSpace) {
                    mstar_FB_OutputColor_EX(gopId,
                        (DRV_FB_OutputColorSpace_e)data.dispLayerAttr.eFbOutputColorSpace);
                    bNeedUpdateColorSpace = TRUE;
                }
            }
            //Timing change
            if (data.dispLayerAttr.u32SetAttrMask
                & E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE) {
                //timing change
                mstar_FB_setHstart(gopId, hstart);
                bNeedUpdateHstart = TRUE;
                FBDEV_INFO("Process E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE request!\n");
            }
            //GOP Dst change
            if (data.dispLayerAttr.u32SetAttrMask
                & E_MI_FB_DISPLAYLAYER_ATTR_MASK_DST_DISP) {
                if (par->eGopDst !=
                        (DRV_FB_GOP_DstType_e)data.dispLayerAttr.eFbDestDisplayPlane) {
                    mstar_FB_SetDst(gopId,
                        (DRV_FB_GOP_DstType_e)data.dispLayerAttr.eFbDestDisplayPlane,FALSE);
                    bNeedUpdateGopDst = TRUE;
                }
            }
end_transaction:
            mstar_FB_EndTransaction(gopId);
            if (bNeedUpdateGopDst) {
                par->eGopDst =
                    (DRV_FB_GOP_DstType_e)data.dispLayerAttr.eFbDestDisplayPlane;
            }
            if (bNeedUpdateHstart) {
                par->u32Hstart = hstart;
            }
            if (bNeedUpdateColorSpace) {
                par->eOutputColorSpace =
                    (DRV_FB_OutputColorSpace_e)data.dispLayerAttr.eFbOutputColorSpace;
            }
            if (bNeedUpdatePremul) {
                par->bPremultiply = data.dispLayerAttr.bPreMul;
            }
            if (bNeedUpdateStretchWin) {
                memcpy(&(par->stStretchWinInfo), &stretchWinInfo,
                            sizeof(DRV_FB_StretchWinInfo_t));
            }
            if (bNeedUpdateGwinInfo) {
                //store gwininfo
                memcpy(&(par->stGwinInfo), &gwinInfo,
                    sizeof(DRV_FB_GwinInfo_t));
                //update var info
                if (data.dispLayerAttr.u32SetAttrMask &
                    (E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE|
                    E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB)) {
                    pinfo->var.xres = fbWidth;
                    pinfo->var.yres = fbHeight;
                    set_fb_bitfield(colorFmt, &pinfo->var);
                    bpx = mstar_FB_GetBpp(colorFmt);
                    pinfo->var.bits_per_pixel = ( bpx << 3);
                    //Recalculate xres_virtual,yres_virtual according to current osd info
                    //and set xoffset yoffset as 0
                    surfaceCnt = getBufferCount(pinfo->fix.smem_len, fbWidth, fbHeight, bpx);
                    pinfo->var.xres_virtual = pinfo->var.xres;
                    pinfo->var.yres_virtual = pinfo->var.yres * surfaceCnt;
                    pinfo->var.xoffset = 0;
                    pinfo->var.yoffset = 0;
                    pinfo->fix.line_length = pinfo->var.xres_virtual * bpx;
                }
            }
            FBDEV_INFO("FBIOSET_DISPLAYLAYER_ATTRIBUTES finish!!\n");
        }
        break;
        case FBIOGET_CURSOR_ATTRIBUTE:
        {
            if (!fbHwCursorInfo->bInitialized) {
                FBDEV_INFO("fb%d hwcursor has not been initialized!\n",pinfo->node);
                return -EINVAL;
            }
            //set gwin position as cursor pos
            data.hwcursorAttr.u32XPos = fbHwCursorInfo->stGwinInfo.u16HStart;
            data.hwcursorAttr.u32YPos = fbHwCursorInfo->stGwinInfo.u16VStart;
            data.hwcursorAttr.u32HotSpotX = fbHwCursorInfo->u16HotSpotX;
            data.hwcursorAttr.u32HotSpotY = fbHwCursorInfo->u16HotSpotY;
            memcpy(&(data.hwcursorAttr.stAlpha),
                &(fbHwCursorInfo->stAlpha), sizeof(MI_FB_GlobalAlpha_t));
            memcpy (&(data.hwcursorAttr.stColorKey),
                &(fbHwCursorInfo->stColorKey),sizeof(MI_FB_ColorKey_t));
            data.hwcursorAttr.bShown = fbHwCursorInfo->bShown;
            data.hwcursorAttr.stCursorImageInfo.u32Width = fbHwCursorInfo->u16IconWidth;
            data.hwcursorAttr.stCursorImageInfo.u32Height = fbHwCursorInfo->u16IconHeight;
            data.hwcursorAttr.stCursorImageInfo.u32Pitch = fbHwCursorInfo->stGwinInfo.u16Pitch;
            data.hwcursorAttr.stCursorImageInfo.eColorFmt =
                (MI_FB_ColorFmt_e)fbHwCursorInfo->stGwinInfo.clrType;
            //It's not necessary to transfer icon raw data to userspace
            data.hwcursorAttr.stCursorImageInfo.data  = NULL;
            data.hwcursorAttr.u16CursorAttrMask = 0x0;
            retval = copy_to_user((MI_FB_CursorAttr_t __user *)u32Arg,
                &data.hwcursorAttr, sizeof(MI_FB_CursorAttr_t));
        }
        break;
        case FBIOSET_CURSOR_ATTRIBUTE:
        {
            MS_U16 iconWidth = fbHwCursorInfo->u16IconWidth;
            MS_U16 iconHeight = fbHwCursorInfo->u16IconHeight;
            MS_U16 hotSpotX = fbHwCursorInfo->u16HotSpotX;
            MS_U16 hotSpotY = fbHwCursorInfo->u16HotSpotY;
            MS_U16 positionX = fbHwCursorInfo->u16PositionX;
            MS_U16 positionY = fbHwCursorInfo->u16PositionY;
            MS_U16 osdWidth = stretchWinSrcWidth;
            MS_U16 osdHeight = stretchWinSrcHeight;
            MS_U16 retCropX = 0;
            MS_U16 retCropY = 0;
            MS_U16 retCropWidth = iconWidth & (~0x1);
            MS_U16 retCropHeight = iconHeight;
            DRV_FB_GwinInfo_t cursorGwinInfo = fbHwCursorInfo->stGwinInfo;
            MS_BOOL bNeedUpdateIconInfo = FALSE;
            MS_BOOL bNeedUpdateCursorPos = FALSE;
            MS_U8 dstAddrIdx = fbHwCursorInfo->u8DstbufIdx;
            MS_U16 retGwinPosX = positionX - hotSpotX;
            MS_U16 retGwinPosY = positionY - hotSpotY;
            MI_FB_ColorKey_t cursorClrKeyInfo = fbHwCursorInfo->stColorKey;
            MS_BOOL bNeedUpdateClrKeyInfo = FALSE;
            MS_U32 fullclr_cursor = 0;
            //do nothing
            if (!(data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK)) {
                return retval;
            }
            if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_ICON) {
                if (data.hwcursorAttr.stCursorImageInfo.u32Width > HWCURSOR_MAX_WIDTH ||
                    data.hwcursorAttr.stCursorImageInfo.u32Height> HWCURSOR_MAX_HEIGHT) {
                   FBDEV_INFO("fb%d hwcursor setIcon, Icon size[%d,%d] is larger than max size[%d,%d]\n",
                        pinfo->node,data.hwcursorAttr.stCursorImageInfo.u32Width, data.hwcursorAttr.stCursorImageInfo.u32Height,HWCURSOR_MAX_WIDTH,HWCURSOR_MAX_HEIGHT);
                    return -EINVAL;
                }
            }
#ifdef MI_SYS_PROC_FS_DEBUG
            mutex_lock(&s_cursor_proc_mutex_lock);
#endif
            if (!fbHwCursorInfo->bInitialized)
            {
                _fb_hwcursor_init(curTimingWidth, curTimingHeight, hstart);
                fbHwCursorInfo->bInitialized = TRUE;
            }
            mstar_FB_BeginTransaction(fbHwCursorInfo->u8GopIdx);
            //Set Stretch Window info
            if (fbHwCursorInfo->stStretchWinInfo.u16SrcWidth != osdWidth ||
                 fbHwCursorInfo->stStretchWinInfo.u16SrcHeight != osdHeight ||
                 fbHwCursorInfo->stStretchWinInfo.u16DstWidth != curTimingWidth ||
                 fbHwCursorInfo->stStretchWinInfo.u16DstHeight != curTimingHeight)
            {
                mstar_FB_SetStretchWin(fbHwCursorInfo->u8GopIdx, 0, 0, osdWidth, osdHeight);
                mstar_FB_SetHScale(fbHwCursorInfo->u8GopIdx, TRUE, osdWidth, curTimingWidth);
                mstar_FB_SetVScale(fbHwCursorInfo->u8GopIdx, TRUE, osdHeight, curTimingHeight);
                mstar_FB_setHstart(fbHwCursorInfo->u8GopIdx, hstart);
                fbHwCursorInfo->stStretchWinInfo.u16SrcWidth = osdWidth;
                fbHwCursorInfo->stStretchWinInfo.u16SrcHeight = osdHeight;
                fbHwCursorInfo->stStretchWinInfo.u16DstWidth = curTimingWidth;
                fbHwCursorInfo->stStretchWinInfo.u16DstHeight = curTimingHeight;
            }
            //Hide
            if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_HIDE) {
                if (fbHwCursorInfo->bShown) {
                    mstar_FB_EnableGwin(fbHwCursorInfo->u8GopIdx,
                        fbHwCursorInfo->u8GwinId, FALSE);
                    fbHwCursorInfo->bShown = FALSE;
                }
            }
            //Change Icon
            if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_ICON) {
                MS_BOOL iconWidthEqual = (fbHwCursorInfo->u16IconWidth
                    == data.hwcursorAttr.stCursorImageInfo.u32Width);
                MS_BOOL iconHeightEqual = (fbHwCursorInfo->u16IconHeight
                    == data.hwcursorAttr.stCursorImageInfo.u32Height);
                MS_BOOL clrFmtEqual = (fbHwCursorInfo->stGwinInfo.clrType
                    == (DRV_FB_GOP_ColorFmt_e)data.hwcursorAttr.stCursorImageInfo.eColorFmt);
                MS_BOOL bRecrateFB = !(iconWidthEqual && iconHeightEqual && clrFmtEqual);
                const u8* u8Src = NULL;
                u8* u8Dst = NULL;
                u32 u32SrcStride = 0;
                bpx = mstar_FB_GetBpp(data.hwcursorAttr.stCursorImageInfo.eColorFmt);
                if (bRecrateFB)
                {
                    mstar_FB_DestroyFBInfo(fbHwCursorInfo->u8GwinId);
                    // the gwin width must be 2 pixel alignment
                    mstar_FB_CreateFBInfo(fbHwCursorInfo->u8GwinId,
                        data.hwcursorAttr.stCursorImageInfo.u32Width & (~0x1),
                        data.hwcursorAttr.stCursorImageInfo.u32Height,
                        data.hwcursorAttr.stCursorImageInfo.eColorFmt,
                        fbHwCursorInfo->stGwinInfo.u16Pitch,
                        fbHwCursorInfo->hwCursorAddr[dstAddrIdx]
                        );
                }
                //Copy Icon data from userspace
                u8Src = data.hwcursorAttr.stCursorImageInfo.data;
                u8Dst = (u8*)(fbHwCursorInfo->hwCursorVaddr[dstAddrIdx]);
                u32SrcStride =
                    data.hwcursorAttr.stCursorImageInfo.u32Pitch * bpx;
                for (i = 0; i < data.hwcursorAttr.stCursorImageInfo.u32Height; i++)
                {
                    if (copy_from_user(u8Dst, u8Src, u32SrcStride)) {
                        retval = -EFAULT;
                        goto commit_transaction;
                    }
                    u8Src +=  u32SrcStride;
                    u8Dst += fbHwCursorInfo->stGwinInfo.u16Pitch;
                }
                //Calculate cursor icon crop region
                iconWidth = data.hwcursorAttr.stCursorImageInfo.u32Width;
                iconHeight = data.hwcursorAttr.stCursorImageInfo.u32Height;
                hotSpotX = data.hwcursorAttr.u32HotSpotX;
                hotSpotY = data.hwcursorAttr.u32HotSpotY;
                //If request move postion && change Icon, position referece position set by App
                if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_POS) {
                    positionX = data.hwcursorAttr.u32XPos;
                    positionY = data.hwcursorAttr.u32YPos;
                }
                calHwcursorInfo(iconWidth, iconHeight, hotSpotX, hotSpotY,
                    positionX, positionY,osdWidth, osdHeight,
                    &retCropX, &retCropY, &retCropWidth, &retCropHeight,
                    &retGwinPosX, &retGwinPosY);
                cursorGwinInfo.u16HStart = retGwinPosX;
                cursorGwinInfo.u16HEnd = retGwinPosX + retCropWidth;
                cursorGwinInfo.u16VStart = retGwinPosY;
                cursorGwinInfo.u16VEnd = retGwinPosY + retCropHeight;
                cursorGwinInfo.clrType = (DRV_FB_GOP_ColorFmt_e)(data.hwcursorAttr.stCursorImageInfo.eColorFmt);
                cursorGwinInfo.u32Addr = fbHwCursorInfo->hwCursorAddr[dstAddrIdx] +
                    (retCropX * bpx + retCropY * cursorGwinInfo.u16Pitch);
                fbHwCursorInfo->u8DstbufIdx = (dstAddrIdx + 1) % HWCURSOR_BUF_CNT;
                bNeedUpdateIconInfo = TRUE;
            }
            //Move cursor position
           if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_POS) {
                positionX = data.hwcursorAttr.u32XPos;
                positionY = data.hwcursorAttr.u32YPos;
                //If chage Icon, Icon info reference Icon info set by App
                if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_ICON) {
                    iconWidth = data.hwcursorAttr.stCursorImageInfo.u32Width;
                    iconHeight = data.hwcursorAttr.stCursorImageInfo.u32Height;
                    hotSpotX = data.hwcursorAttr.u32HotSpotX;
                    hotSpotY = data.hwcursorAttr.u32HotSpotY;
                } else {
                    dstAddrIdx = (dstAddrIdx - 1 + HWCURSOR_BUF_CNT) % HWCURSOR_BUF_CNT;
                }
                calHwcursorInfo(iconWidth, iconHeight, hotSpotX, hotSpotY,
                    positionX, positionY, osdWidth, osdHeight,
                    &retCropX, &retCropY, &retCropWidth, &retCropHeight,
                    &retGwinPosX, &retGwinPosY);
                cursorGwinInfo.u16HStart = retGwinPosX;
                cursorGwinInfo.u16HEnd = retGwinPosX + retCropWidth;
                cursorGwinInfo.u16VStart = retGwinPosY;
                cursorGwinInfo.u16VEnd = retGwinPosY + retCropHeight;
                cursorGwinInfo.u32Addr = fbHwCursorInfo->hwCursorAddr[dstAddrIdx] +
                        (retCropX * bpx + retCropY * cursorGwinInfo.u16Pitch);
                bNeedUpdateCursorPos  = TRUE;
           }
           //Show
           if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_SHOW) {
                if (!fbHwCursorInfo->bShown) {
                    mstar_FB_EnableGwin(fbHwCursorInfo->u8GopIdx,
                        fbHwCursorInfo->u8GwinId, TRUE);
                    fbHwCursorInfo->bShown = TRUE;
                }
           }
           //Process colorkey op for hwcursor
           if (data.hwcursorAttr.u16CursorAttrMask & E_MI_FB_CURSOR_ATTR_MASK_COLORKEY) {
                if (memcmp(&cursorClrKeyInfo, &(data.hwcursorAttr.stColorKey),
                        sizeof(MI_FB_ColorKey_t))) {
                    if (data.hwcursorAttr.stColorKey.bKeyEnable){
                        mstar_FB_EnableTransClr_EX(fbHwCursorInfo->u8GopIdx, E_DRV_FB_GOPTRANSCLR_FMT0, TRUE);
                        fullclr_cursor = (0xff<<24) | (data.hwcursorAttr.stColorKey.u8Red<<16)
                            | (data.hwcursorAttr.stColorKey.u8Green<<8) | (data.hwcursorAttr.stColorKey.u8Blue);
                        mstar_FB_SetTransClr_8888(fbHwCursorInfo->u8GopIdx,
                            fullclr_cursor,0);
                    }
                    else {
                        mstar_FB_EnableTransClr_EX(fbHwCursorInfo->u8GopIdx,
                            E_DRV_FB_GOPTRANSCLR_FMT0, FALSE);
                    }
                    bNeedUpdateClrKeyInfo = TRUE;
                }
           }
           //Update gwin info
           if (memcmp(&cursorGwinInfo, &(fbHwCursorInfo->stGwinInfo),
                sizeof(DRV_FB_GwinInfo_t))) {
                mstar_FB_SetGwinInfo(fbHwCursorInfo->u8GopIdx,
                    fbHwCursorInfo->u8GwinId, cursorGwinInfo);
                memcpy(&(fbHwCursorInfo->stGwinInfo), &cursorGwinInfo, sizeof(DRV_FB_GwinInfo_t));
           }
 commit_transaction:
           mstar_FB_EndTransaction(fbHwCursorInfo->u8GopIdx);
           if (bNeedUpdateIconInfo) {
                fbHwCursorInfo->u16IconWidth = iconWidth;
                fbHwCursorInfo->u16IconHeight = iconHeight;
                fbHwCursorInfo->u16HotSpotX = hotSpotX;
                fbHwCursorInfo->u16HotSpotY = hotSpotY;
           }
           if (bNeedUpdateCursorPos) {
                fbHwCursorInfo->u16PositionX = positionX;
                fbHwCursorInfo->u16PositionY = positionY;
           }
           //Store current colorkey Info
           if (bNeedUpdateClrKeyInfo) {
                 memcpy(&(fbHwCursorInfo->stColorKey), &(data.hwcursorAttr.stColorKey),
                    sizeof(MI_FB_ColorKey_t));
           }
#ifdef MI_SYS_PROC_FS_DEBUG
           mutex_unlock(&s_cursor_proc_mutex_lock);
#endif
        }
        break;
    }
    return retval;
}
#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
#else
module_init(mstar_fbdev_init);
module_exit(mstar_fbdev_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GRAPHIC ioctrl driver");
MODULE_LICENSE("GPL");
#endif//#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
