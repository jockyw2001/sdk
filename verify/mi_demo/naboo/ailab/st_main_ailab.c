#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>

#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <errno.h>
#include <linux/input.h>

#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "mi_sys.h"
#include "mi_venc.h"

#include "st_common.h"
#include "st_hdmi.h"
#include "st_disp.h"
#include "st_vpe.h"
#include "st_vif.h"
#include "st_fb.h"
#include "st_warp.h"
#include "list.h"
#include "tem.h"

typedef enum
{
    FB_DRAW_BEGIN = 0,
    FB_DRAW_ING,
    FB_DRAW_END,
} FB_DRAW_E;

static MI_BOOL g_runFlag = TRUE;

#define MOUSE_DEV_NAME      "/dev/input/mice"
#define MOUSE_FILE_PATH     "/mnt/cursor.raw"
#define MOUSE_PIC_WIDTH     44
#define MOUSE_PIC_HEIGHT    56

//#define SUPPORT_WRITE_ES
//#define SUPPORT_WRITE_YUV

/* Mouse button bits*/
#define WHEEL_UP    0x10
#define WHEEL_DOWN  0x08

#define BUTTON_L    0x04
#define BUTTON_M    0x02
#define BUTTON_R    0x01
#define SCALE       1 /* default scaling factor for acceleration */
#define THRESH      1 /* default threshhold for acceleration */

static int xpos; /* current x position of mouse */
static int ypos; /* current y position of mouse */
static int minx; /* minimum allowed x position */
static int maxx; /* maximum allowed x position */
static int miny; /* minimum allowed y position */
static int maxy; /* maximum allowed y position */
// static int buttons; /* current state of buttons */

static int IMPS2_Read(int fd, int *dx, int *dy, int *dz, int *bp)
{
    static unsigned char buf[5];
    static int buttons[7] = {0, 1, 3, 0, 2, 0, 0};// 1:left button, 2: mid button, 3: right button
    static int nbytes = 0;
    int n;

    while ((n = read (fd, &buf [nbytes], 4 - nbytes)))
    {
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        nbytes += n;

        if (nbytes == 4)
        {
            int wheel;
            // printf("[luther.gliethttp]: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
            if ((buf[0] & 0xc0) != 0)
            {
                buf[0] = buf[1];
                buf[1] = buf[2];
                buf[2] = buf[3];
                nbytes = 3;
                return -1;
            }

            /* FORM XFree86 4.0.1 */
            *bp = buttons[(buf[0] & 0x07)];
            *dx = (buf[0] & 0x10) ? buf[1] - 256 : buf[1];
            *dy = (buf[0] & 0x20) ? -(buf[2] - 256) : -buf[2];

            /* Is a wheel event? */
            if ((wheel = buf[3]) != 0)
            {
                if(wheel > 0x7f)
                {
                    *bp |= WHEEL_UP;
                }
                else
                {
                    *bp |= WHEEL_DOWN;
                }
            }

            *dz = 0;
            nbytes = 0;
            return 1;
        }
    }

    return 0;
}

void mouse_setrange (int newminx, int newminy, int newmaxx, int newmaxy)
{
    minx = newminx;
    miny = newminy;
    maxx = newmaxx;
    maxy = newmaxy;
}

int mouse_update(int dx, int dy, int dz)
{
    int r;
    int sign;

    sign = 1;
    if (dx < 0)
    {
        sign = -1;
        dx = -dx;
    }

    if (dx > THRESH)
        dx = THRESH + (dx - THRESH) * SCALE;

    dx *= sign;
    xpos += dx;

    if( xpos < minx )
        xpos = minx;
    if( xpos > maxx )
        xpos = maxx;

    sign = 1;
    if (dy < 0)
    {
        sign = -1;
        dy = -dy;
    }

    if (dy > THRESH)
         dy = THRESH + (dy - THRESH) * SCALE;

    dy *= sign;
    ypos += dy;

    if (ypos < miny)
        ypos = miny;

    if (ypos > maxy)
        ypos = maxy;

    return 1;
}

void *ST_FBMouseUserProc(void *args)
{
    int fd,retval;
    fd_set readfds;
    struct input_event inputEv[64];
    int readlen = 0;
    int i = 0;
    unsigned char buf[32];
    int x_pos, y_pos;
    unsigned char imps2_param [] = {243, 200, 243, 100, 243, 80};
    int dx, dy, dz, button;
    FB_DRAW_E enDraw = FB_DRAW_END;
    int old_xpos, old_ypos;
    MI_SYS_WindowRect_t stRect;
    MI_U32 u32ColorIndex = 0;
    MI_U32 u32Color[] = {ARGB888_RED, ARGB888_GREEN, ARGB888_BLACK};
    int d_xpos, d_ypos = 0;

    do
    {
        fd = open(MOUSE_DEV_NAME, O_RDWR);
        if (fd < 0)
        {
            printf("can not open %s\n", MOUSE_DEV_NAME);
        }
        sleep (5);
    } while (fd < 0);

    printf("open %s success, fd:%d\n", MOUSE_DEV_NAME, fd);

    write(fd, imps2_param, sizeof (imps2_param));

    while(g_runFlag)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        retval = select(fd + 1, &readfds, NULL, NULL, NULL);
        if(retval == 0)
        {
            printf("Time out!\n");
            continue;
        }

        if(FD_ISSET(fd, &readfds))
        {
            IMPS2_Read(fd, &dx, &dy, &dz, &button);

            mouse_update(dx, dy, dz);

            if (button == 0x1)
            {
                // left button down
                if (enDraw == FB_DRAW_END)
                {
                    enDraw = FB_DRAW_BEGIN;
                    old_xpos = xpos;
                    old_ypos = ypos;
                }
                else if (enDraw == FB_DRAW_BEGIN)
                {
                    enDraw = FB_DRAW_ING;
                }
            }
            else
            {
                // left button up
                enDraw = FB_DRAW_END;
            }

            if (button == 0x3)
            {
                // right button down
                u32ColorIndex ++;
            }

            if (enDraw == FB_DRAW_BEGIN ||
                enDraw == FB_DRAW_ING)
            {
                d_xpos = abs(xpos - old_xpos);
                d_ypos = abs(ypos - old_ypos);
                if (d_xpos > 0)
                {
                    for (i = MIN(xpos, old_xpos); i < MAX(xpos, old_xpos); i ++)
                    {
                        ST_Fb_FillCircle(i, ypos, 10, u32Color[u32ColorIndex % ARRAY_SIZE(u32Color)]);
                    }
                }

                if (d_ypos > 0)
                {
                    for (i = MIN(ypos, old_ypos); i < MAX(ypos, old_ypos); i ++)
                    {
                        ST_Fb_FillCircle(xpos, i, 10, u32Color[u32ColorIndex % ARRAY_SIZE(u32Color)]);
                    }
                }
            }

            // printf("xpos:%d,ypos:%d, button:0x%X\n", xpos, ypos, button);
            ST_Fb_MouseSet(xpos, ypos);
            old_xpos = xpos;
            old_ypos = ypos;
        }
    }

    close(fd);

    return NULL;
}

static MI_S32 St_MouseFBInit(void)
{
    MI_SYS_WindowRect_t Rect;
    MI_FB_GlobalAlpha_t stAlphaInfo;
    pthread_t pt;

    ST_Fb_Init();
    ST_FB_Show(FALSE);

    sleep(1);

    // change fb resolution
    ST_FB_ChangeResolution(1920, 1080);
	
	// after change resolution, fb default size is 1280x720, if DISP init at 1024x768,
    // set color format will fail
    ST_Fb_SetColorFmt(E_MI_FB_COLOR_FMT_ARGB1555);
	
    ST_FB_Show(TRUE);

    ST_Fb_SetColorKey(ARGB888_BLUE);

    memset(&stAlphaInfo, 0, sizeof(MI_FB_GlobalAlpha_t));
    ST_FB_GetAlphaInfo(&stAlphaInfo);
    printf("FBIOGET_GLOBAL_ALPHA alpha info: alpha blend enable=%d,Multialpha enable=%d,Global Alpha=%d,u8Alpha0=%d,u8Alpha1=%d\n",
        stAlphaInfo.bAlphaEnable,stAlphaInfo.bAlphaChannel,stAlphaInfo.u8GlobalAlpha,stAlphaInfo.u8Alpha0,stAlphaInfo.u8Alpha1);
    stAlphaInfo.bAlphaEnable = TRUE;
    stAlphaInfo.bAlphaChannel= TRUE;
    stAlphaInfo.u8GlobalAlpha = 0x70;
    ST_FB_SetAlphaInfo(&stAlphaInfo);

    ST_Fb_InitMouse(MOUSE_PIC_WIDTH, MOUSE_PIC_HEIGHT, 4, MOUSE_FILE_PATH);
    ST_Fb_MouseSet(1, 1);

    mouse_setrange(0, 0, 1920, 1080);

    pthread_create(&pt, NULL, ST_FBMouseUserProc, NULL);
}

void *St_GetVencEsStream(void *args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_CHN vencChn = 0;
    MI_U32 u32DevId = 0;
    struct pollfd fds = {0, POLLIN | POLLERR};
    int rval = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE bufHandle;
    char szFileName[64] = {0,};
    MI_VENC_ModType_e enVencType = *((MI_VENC_ModType_e *)(args));
    int len = 0;
    int fd = -1;

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    stChnPort.eModId = E_MI_MODULE_ID_VENC;
    s32Ret = MI_VENC_GetChnDevid(vencChn, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_VENC_GetChnDevid %d error, %X\n", vencChn, s32Ret);
        return NULL;
    }
    stChnPort.u32DevId = u32DevId;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;

    s32Ret = MI_SYS_GetFd(&stChnPort, (MI_S32 *)&fds.fd);
    if(MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_SYS_GetFd %d, error, %X\n", vencChn, s32Ret);
        return NULL;
    }

    len = 0;
    len += snprintf(szFileName + len, sizeof(szFileName) - 1, "venc_dev%d_chn0_port0.",
        u32DevId);
    if (enVencType == E_MI_VENC_MODTYPE_H264E)
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "h264");
    }
    else if (enVencType == E_MI_VENC_MODTYPE_H265E)
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "h265");
    }
    else if (enVencType == E_MI_VENC_MODTYPE_JPEGE)
    {
        snprintf(szFileName + len, sizeof(szFileName) - 1, "mjpeg");
    }

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd <= 0)
    {
        ST_ERR("create %s error\n", szFileName);
    }
    else
    {
        ST_ERR("create %s success\n", szFileName);
    }

    while (g_runFlag)
    {
        rval = poll(&fds, 1, 200);
        if(rval < 0)
        {
            ST_DBG("poll error!\n");
            continue;
        }
        if(rval == 0)
        {
            ST_DBG("get fd time out!\n");
            continue;
        }
        if((fds.revents & POLLIN) != POLLIN)
        {
            ST_DBG("error !\n");
            continue;
        }

        memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
        if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &bufHandle))
        {
            ST_ERR("Get buffer error!\n");
            continue;
        }

        len = write(fd, stBufInfo.stRawData.pVirAddr, stBufInfo.stRawData.u32ContentSize);

        // ST_DBG("chn:%d write frame len=%d, real len=%d\n", stChnPort.u32ChnId,
        //        len, stBufInfo.stRawData.u32ContentSize);

        MI_SYS_ChnOutputPortPutBuf(bufHandle);
    }

    if (fd > 0)
    {
        close(fd);
    }

    MI_SYS_CloseFd((MI_S32)fds.fd);

    return NULL;
}

void *St_GetVPEYuvData(void *args)
{
    ST_VPE_PortInfo_t stVpePortInfo;
    MI_U16 u16OutputWidth, u16OutputHeight;
    MI_SYS_ChnPort_t stChnPort;
    struct pollfd fds = {0, POLLIN | POLLERR};
    MI_S32 s32Ret = MI_SUCCESS;
    int count = 0;
    char szFileName[64] = {0,};
    int fd = -1;
    int rval = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE bufHandle;

    // create VPE port to get YUV data
    memset(&stVpePortInfo, 0, sizeof(ST_VPE_PortInfo_t));

    u16OutputWidth = 1280;
    u16OutputHeight = 720;

    stVpePortInfo.DepVpeChannel = 0;
    stVpePortInfo.u16OutputWidth = u16OutputWidth;
    stVpePortInfo.u16OutputHeight = u16OutputHeight;
    stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    if (MI_SUCCESS != ST_Vpe_CreatePort(SUB_VENC_PORT, &stVpePortInfo))
    {
        ST_ERR("ST_Vpe_CreatePort error\n");
        return NULL;
    }

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    stChnPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = SUB_VENC_PORT;

    s32Ret = MI_SYS_GetFd(&stChnPort, (MI_S32 *)&fds.fd);
    if(MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_SYS_GetFd 0, error, %X\n", s32Ret);
        return NULL;
    }

    while (g_runFlag)
    {
        rval = poll(&fds, 1, 200);
        if(rval < 0)
        {
            ST_DBG("poll error!\n");
            continue;
        }
        if(rval == 0)
        {
            ST_DBG("get fd time out!\n");
            continue;
        }
        if((fds.revents & POLLIN) != POLLIN)
        {
            ST_DBG("error !\n");
            continue;
        }

        memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
        if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &bufHandle))
        {
            ST_ERR("Get buffer error!\n");
            continue;
        }
        // save Y data, luma
        memset(szFileName, 0, sizeof(szFileName) - 1);
        snprintf(szFileName, sizeof(szFileName) - 1, "vpe0_port%d_%dx%d_%04d_luma.yuv", SUB_VENC_PORT,
            u16OutputWidth, u16OutputHeight, count);
        fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd > 0)
        {
            write(fd, stBufInfo.stFrameData.pVirAddr[0],
                stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
            close(fd);
            fd = -1;
        }

        // save UV data
        memset(szFileName, 0, sizeof(szFileName) - 1);
        snprintf(szFileName, sizeof(szFileName) - 1, "vpe0_port%d_%dx%d_%04d_chrome.yuv", SUB_VENC_PORT,
            u16OutputWidth, u16OutputHeight, count);
        fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd > 0)
        {
            write(fd, stBufInfo.stFrameData.pVirAddr[1],
                stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] /2);
            close(fd);
            fd = -1;
        }

        MI_SYS_ChnOutputPortPutBuf(bufHandle);

        count ++;
    }

    return NULL;
}

static MI_S32 St_VencInit(MI_VENC_ModType_e enType)
{
    MI_VENC_ChnAttr_t stChnAttr;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_CHN vencChn = 0;
    MI_U32 u32DevId = 0;
    MI_SYS_ChnPort_t stChnPort;
    ST_VPE_PortInfo_t stVpePortInfo;
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    switch (enType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 0; // not support B frame

            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = 2 * 1024 * 1024;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
        }
        break;

        case E_MI_VENC_MODTYPE_H265E:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 2160;

            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = 1 * 1024 * 1024;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = 30;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
        }
        break;

        case E_MI_VENC_MODTYPE_JPEGE:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 3840;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 2160;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 3840;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 2160;
        }
        break;

        default:
        {
            ST_ERR("venc not support this type:%d\n", (int)enType);
            return -1;
        }
    }

    s32Ret = MI_VENC_CreateChn(vencChn, &stChnAttr);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_VENC_CreateChn %d error, %X\n", vencChn, s32Ret);
        return -1;
    }

    s32Ret = MI_VENC_GetChnDevid(vencChn, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_VENC_GetChnDevid %d error, %X\n", vencChn, s32Ret);
        return -1;
    }

    ST_DBG("VENC chn:%d, u32DevId:%d\n", vencChn, u32DevId);

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    stChnPort.u32DevId = u32DevId;
    stChnPort.eModId = E_MI_MODULE_ID_VENC;
    stChnPort.u32ChnId = vencChn;
    stChnPort.u32PortId = 0;

    //This was set to (5, 10) and might be too big for kernel
    s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 5);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_SYS_SetChnOutputPortDepth %d error, %X\n", vencChn, s32Ret);
        return -1;
    }

    s32Ret = MI_VENC_StartRecvPic(vencChn);
    if (MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_VENC_StartRecvPic %d error, %X\n", vencChn, s32Ret);
        return -1;
    }

    // create VPE port for venc
    memset(&stVpePortInfo, 0, sizeof(ST_VPE_PortInfo_t));

    stVpePortInfo.DepVpeChannel = 0;
    stVpePortInfo.u16OutputWidth = 3840;
    stVpePortInfo.u16OutputHeight = 2160;
    stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stVpePortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(ST_Vpe_CreatePort(MAIN_VENC_PORT, &stVpePortInfo));

    // bind VPE port to venc
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = MAIN_VENC_PORT;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = u32DevId;
    stBindInfo.stDstChnPort.u32ChnId = vencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;

    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    return 0;
}

static MI_S32 St_VencDeinit(void)
{

}

static MI_S32 St_DisplayInit(void)
{
    ST_VPE_PortInfo_t stPortInfo;
    ST_Sys_BindInfo_t stBindInfo;
    ST_DispChnInfo_t stDispChnInfo;

    memset(&stPortInfo, 0x0, sizeof(ST_VPE_PortInfo_t));
    memset(&stDispChnInfo, 0x0, sizeof(ST_DispChnInfo_t));

    stPortInfo.DepVpeChannel = 0;
    stPortInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stPortInfo.u16OutputWidth = 1920;
    stPortInfo.u16OutputHeight = 1080;
    STCHECKRESULT(ST_Vpe_CreatePort(DISP_PORT, &stPortInfo)); //default support port0 --->>> vdisp
    STCHECKRESULT(ST_Disp_DevInit(ST_DISP_DEV0, ST_DISP_LAYER0, E_MI_DISP_OUTPUT_1080P60)); //Dispout timing
    stDispChnInfo.InputPortNum = 1;
    stDispChnInfo.stInputPortAttr[0].u32Port = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16X = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Y = 0;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Width = 1920;
    stDispChnInfo.stInputPortAttr[0].stAttr.stDispWin.u16Height = 1080;
    STCHECKRESULT(ST_Disp_ChnInit(0, &stDispChnInfo));

    /************************************************
    Step8:  Bind VPE->VDISP
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = DISP_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
    STCHECKRESULT(ST_Hdmi_Init());
    STCHECKRESULT(ST_Hdmi_Start(E_MI_HDMI_ID_0, E_MI_HDMI_TIMING_1080_60P)); //Hdmi timing

    return MI_SUCCESS;
}

static MI_S32 St_DisplayDeinit(void)
{
    ST_VPE_PortInfo_t stPortInfo;
    ST_Sys_BindInfo_t stBindInfo;
    ST_DispChnInfo_t stDispChnInfo;

    STCHECKRESULT(ST_Hdmi_DeInit(E_MI_HDMI_ID_0));
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = DISP_PORT;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
    STCHECKRESULT(ST_Disp_DeInit(ST_DISP_DEV0, ST_DISP_LAYER0, 1));
    STCHECKRESULT(ST_Vpe_StopPort(0, DISP_PORT));

    return MI_SUCCESS;
}

static MI_S32 St_BaseModuleInit(MI_BOOL bFramOrReal)
{
    ST_VIF_PortInfo_t stVifPortInfoInfo;
    MI_SYS_ChnPort_t stChnPort;
    ST_VPE_ChannelInfo_t stVpeChannelInfo;
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stVifPortInfoInfo, 0x0, sizeof(ST_VIF_PortInfo_t));
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stVpeChannelInfo, 0x0, sizeof(ST_VPE_ChannelInfo_t));

    STCHECKRESULT(ST_Sys_Init());
    STCHECKRESULT(ST_Vif_CreateDev(0, bFramOrReal?SAMPLE_VI_MODE_MIPI_1_1080P_FRAME:SAMPLE_VI_MODE_MIPI_1_1080P_REALTIME));
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = 3840;
    stVifPortInfoInfo.u32RectHeight = 2160;
    stVifPortInfoInfo.u32DestWidth = 3840;
    stVifPortInfoInfo.u32DestHeight = 2160;
    stVifPortInfoInfo.ePixFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    STCHECKRESULT(ST_Vif_CreatePort(0, 0, &stVifPortInfoInfo));
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 3);
    STCHECKRESULT(ST_Vif_StartPort(0, 0));

    /************************************************
    Step4:  init VPE
    *************************************************/
    stVpeChannelInfo.u16VpeMaxW = 3840;
    stVpeChannelInfo.u16VpeMaxH = 2160;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 3840;
    stVpeChannelInfo.u16VpeCropH = 2160;
    stVpeChannelInfo.eRunningMode = bFramOrReal?E_MI_VPE_RUNNING_MODE_FRAMEBUF_CAM_MODE:E_MI_VPE_RUNNING_MODE_REALTIME_MODE;
    stVpeChannelInfo.eFormat = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_10BPP_RG;
    STCHECKRESULT(ST_Vpe_CreateChannel(0, &stVpeChannelInfo));
    STCHECKRESULT(ST_Vpe_StartChannel(0));

    /************************************************
    Step7:  Bind VIF->VPE
    *************************************************/
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    return MI_SUCCESS;
}

static MI_S32 St_BaseModuleDeinit(void)
{
    ST_Sys_BindInfo_t stBindInfo;

    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_t));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = 0; //VIF dev == 0
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0; //Main stream
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

    STCHECKRESULT(ST_Vpe_StopChannel(0));
    STCHECKRESULT(ST_Vpe_DestroyChannel(0));
    STCHECKRESULT(ST_Vif_StopPort(0, 0));
    STCHECKRESULT(ST_Vif_DisableDev(0));
    STCHECKRESULT(ST_Sys_Exit());

    return MI_SUCCESS;
}

static void do_fix_mantis(int mantis)
{
    printf("To fix mantis 1574080!\n");

    return;
}

MI_S32 main(int argc, char **argv)
{
    MI_S32 s32Mode = 0;
    char cmd = 0;
    pthread_t vencThreadPt, vpeThreadPt;
    MI_VENC_ModType_e enVencType = E_MI_VENC_MODTYPE_H264E;

    if (signal(SIGINT, do_fix_mantis) != 0)
    {
        perror("signal");
        return -1;
    }
    if (signal(SIGTSTP, do_fix_mantis) != 0)
    {
        perror("signal");
        return -1;
    }

    printf("Which mode do you want ?\n 0 for real time mode 1 for frame mode\n");
    scanf("%d", &s32Mode);
    printf("You select %s mode\n", s32Mode?"frame":"real time");
    St_BaseModuleInit(s32Mode);
    St_DisplayInit();
    St_VencInit(enVencType);

    St_MouseFBInit();

#ifdef SUPPORT_WRITE_ES
    // create thread to get es stream
    pthread_create(&vencThreadPt, NULL, St_GetVencEsStream, (void*)&enVencType);
#endif

#ifdef SUPPORT_WRITE_YUV
    // create thread to get YUV data
    pthread_create(&vencThreadPt, NULL, St_GetVPEYuvData, NULL);
#endif

    while(1)
    {
        printf("Type \"q\" to exit\n");
        cmd = getchar();
        if (cmd == 'q')
        {
            g_runFlag = FALSE;
            break;
        }
    }

    sleep(1);

    St_VencDeinit();
    St_DisplayDeinit();
    St_BaseModuleDeinit();

    return 0;
}

