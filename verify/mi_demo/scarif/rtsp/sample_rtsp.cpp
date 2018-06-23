#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"
#include "sample_comm.h"

#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define ST_NOP(fmt, args...)
#define ST_DBG(fmt, args...) ({do{printf(COLOR_GREEN"[DBG]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_WARN(fmt, args...) ({do{printf(COLOR_YELLOW"[WARN]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_INFO(fmt, args...) ({do{printf("[INFO]:%s[%d]: \n", __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_ERR(fmt, args...) ({do{printf(COLOR_RED"[ERR]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#define RTSP_LISTEN_PORT		554
#define MAIN_STREAM				"main_stream"
#define SUB_STREAM0				"sub_stream0"
#define SUB_STREAM1				"sub_Stream1"

#define PATH_PREFIX				"/mnt"
#define DEBUG_ES_FILE			0

struct Stream_Attr_T
{
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoad;
	PIC_SIZE_E enSize;
	const char*	pszStreamName;
};

typedef struct
{
	VENC_CHN vencChn;
	int vencFd;
	PAYLOAD_TYPE_E enPayLoad;
	char szStreamName[64];

	HI_BOOL bWriteFile;
	int fd;
	char szDebugFile[128];
} StreamInfo_T;

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;

static Live555RTSPServer* g_pRTSPServer = NULL;

static Stream_Attr_T *g_pstStreamAttr = NULL;
static int g_streamNum = 0;

static struct Stream_Attr_T g_stStreamAttr1080P_CLASSIC[] =
{
	{
		.VencChn = 0,
		.enPayLoad = PT_H264,
		.enSize = PIC_HD1080,
		.pszStreamName = MAIN_STREAM,
	},
	{
		.VencChn = 1,
		.enPayLoad = PT_H264,
		.enSize = PIC_VGA,
		.pszStreamName = SUB_STREAM0,
	},
	{
		.VencChn = 2,
		.enPayLoad = PT_H264,
		.enSize = PIC_D1,
		.pszStreamName = SUB_STREAM1,
	},
};

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_RTSP_Usage(char* sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) 1*1080p H264 + 1*1080p H265 + 1*D1 H264 encode.\n");
    printf("\t 1) 1*1080p MJPEG encode + 1*1080p jpeg.\n");
    printf("\t 2) low delay encode.\n");
    printf("\t 3) roi background framerate.\n");
    printf("\t 4) svc-t H264\n");
    printf("\t 5) H264 intra refresh.\n");
    printf("\t 6) Advanced Single P.\n");
    printf("\t 7) Thumbnail of 1*1080p jpeg.\n");
    return;
}

#if 1
void* SAMPLE_OpenStream(char const* szStreamName, void* arg)
{
	StreamInfo_T *pstStreamInfo = NULL;
	HI_U32 u32ArraySize = g_streamNum;
	Stream_Attr_T *pstStreamAttr = g_pstStreamAttr;
	HI_U32 i = 0;
	HI_S32 s32Ret = HI_SUCCESS;

	pstStreamInfo = (StreamInfo_T *)malloc(sizeof(StreamInfo_T));
	if (pstStreamInfo == NULL)
	{
		ST_ERR("malloc error\n");
		return NULL;
	}

	memset(pstStreamInfo, 0, sizeof(StreamInfo_T));

	for (i = 0; i < u32ArraySize; i ++)
	{
		if (!strncmp(szStreamName, pstStreamAttr[i].pszStreamName,
			strlen(pstStreamAttr[i].pszStreamName)))
		{
			break;
		}
	}

	if (i >= u32ArraySize)
	{
		ST_ERR("not found this stream, \"%s\"", szStreamName);
		free(pstStreamInfo);
		return NULL;
	}

	pstStreamInfo->vencChn = pstStreamAttr[i].VencChn;
	pstStreamInfo->enPayLoad = pstStreamAttr[i].enPayLoad;
	snprintf(pstStreamInfo->szStreamName, sizeof(pstStreamInfo->szStreamName) - 1,
		"%s", szStreamName);

#if DEBUG_ES_FILE
	// whether write frame to file
	int len = 0;
	time_t now = 0;
	struct tm *tm = NULL;

	now = time(NULL);
	tm = localtime(&now);

	len += sprintf(pstStreamInfo->szDebugFile + len, "%s/", PATH_PREFIX);
	len += sprintf(pstStreamInfo->szDebugFile + len, "%s_venc%02d_", szStreamName, pstStreamInfo->vencChn);
	len += sprintf(pstStreamInfo->szDebugFile + len, "%d_", tm->tm_year + 1900);
    len += sprintf(pstStreamInfo->szDebugFile + len, "%02d_", tm->tm_mon);
    len += sprintf(pstStreamInfo->szDebugFile + len, "%02d-", tm->tm_mday);
    len += sprintf(pstStreamInfo->szDebugFile + len, "%02d_", tm->tm_hour);
    len += sprintf(pstStreamInfo->szDebugFile + len, "%02d_", tm->tm_min);
    len += sprintf(pstStreamInfo->szDebugFile + len, "%02d", tm->tm_sec);

	pstStreamInfo->bWriteFile = TRUE;
	pstStreamInfo->fd = 0;
	pstStreamInfo->fd = open(pstStreamInfo->szDebugFile,
							O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (pstStreamInfo->fd <= 0)
	{
		ST_WARN("create file %s error\n", pstStreamInfo->szDebugFile);
	}
	else
	{
		ST_DBG("open %s success\n", pstStreamInfo->szDebugFile);
	}
#endif

	ST_DBG("open stream \"%s\" success\n", szStreamName);

	s32Ret = HI_MPI_VENC_RequestIDR(pstStreamInfo->vencChn, HI_TRUE);
	if (HI_SUCCESS != s32Ret)
	{
		ST_WARN("request IDR fail, error:%x\n", s32Ret);
	}

	pstStreamInfo->vencFd = HI_MPI_VENC_GetFd(pstStreamInfo->vencChn);

	return pstStreamInfo;
}

int SAMPLE_VideoReadStream(void* handle, unsigned char* ucpBuf, int BufLen, struct timeval *p_Timestamp, void* arg)
{
	int writeLen = 0;
	fd_set read_fds;
	struct timeval TimeoutVal;
	HI_S32 s32Ret = HI_SUCCESS;
	VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
	VENC_PACK_S stVencPack[10];
	HI_S32 i;
	int len = 0;

	if (handle == NULL)
	{
		return -1; // disconnect
	}

	StreamInfo_T *pstStreamInfo = (StreamInfo_T *)handle;

	FD_ZERO(&read_fds);
	FD_SET(pstStreamInfo->vencFd, &read_fds);

	TimeoutVal.tv_sec  = 1;
    TimeoutVal.tv_usec = 0;
    s32Ret = select(pstStreamInfo->vencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
    if (s32Ret < 0)
    {
        SAMPLE_PRT("select failed!\n");
       	return 0;
    }
	else if (s32Ret == 0)
    {
        SAMPLE_PRT("get venc stream time out\n");
        return 0;
    }
	else
	{
		if (FD_ISSET(pstStreamInfo->vencFd, &read_fds))
		{
			/*******************************************************
             step 2.1 : query how many packs in one-frame stream.
            *******************************************************/
            memset(&stStream, 0, sizeof(stStream));
            s32Ret = HI_MPI_VENC_Query(pstStreamInfo->vencChn, &stStat);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", pstStreamInfo->vencChn, s32Ret);
                return 0;
            }

			/*******************************************************
			 step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
			 if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
			 {
				SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
				continue;
			 }
			*******************************************************/
			if(0 == stStat.u32CurPacks)
			{
				SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
				return 0;
			}

			/*******************************************************
             step 2.3 : malloc corresponding number of pack nodes.
            *******************************************************/
			stStream.pstPack = stVencPack;

			/*******************************************************
             step 2.4 : call mpi to get one-frame stream
            *******************************************************/
            stStream.u32PackCount = stStat.u32CurPacks;
            s32Ret = HI_MPI_VENC_GetStream(pstStreamInfo->vencChn, &stStream, HI_TRUE);
            if (HI_SUCCESS != s32Ret)
            {
                stStream.pstPack = NULL;
                SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                return 0;
            }

			struct timeval tv = { 0 };
			gettimeofday(&tv, NULL);
			time_t time = tv.tv_sec;
			int ms = tv.tv_usec / 1000;

			struct tm tmNow;
			localtime_r(&time, &tmNow);
			//int year = tmNow.tm_year + 1900;
			//int month = tmNow.tm_mon + 1;
			//int day = tmNow.tm_mday;
			int hour = tmNow.tm_hour;
			int minute = tmNow.tm_min;
			int second = tmNow.tm_sec;

			static int idx = -1;
			++idx;

			static struct timespec sLastTime;
			int msDelta = 0;
			if (idx == 0)
			{
				clock_gettime(CLOCK_MONOTONIC, &sLastTime);
			}
			else
			{
				struct timespec tp;
				tp.tv_sec = 0;
				tp.tv_nsec = 0;
				clock_gettime(CLOCK_MONOTONIC, &tp);
				msDelta = (tp.tv_sec - sLastTime.tv_sec) * 1000 + (tp.tv_nsec - sLastTime.tv_nsec) / 1000000;
				sLastTime = tp;
			}

			printf("channel[%d].%04d,time=%02d:%02d:%02d.%03d,delta=%d ms, count:%d",pstStreamInfo->vencChn,
					idx,hour,minute,second,ms,msDelta, stStream.u32PackCount);
			if (msDelta >= 100)
			{
				printf("###################");
			}
			printf("\r\n");

		    for (i = 0; i < stStream.u32PackCount; i++)
		    {
#if 0
		        fwrite(stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,
		               stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset, 1, fpH264File);

		        fflush(fpH264File);
#endif

				memcpy(ucpBuf + len, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,
						stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
				len += stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
		    }

			s32Ret = HI_MPI_VENC_ReleaseStream(pstStreamInfo->vencChn, &stStream);
            if (HI_SUCCESS != s32Ret)
            {
                stStream.pstPack = NULL;
                return 0;
            }
		}
	}

	return len;
}

int SAMPLE_CloseStream(void* handle, void* arg)
{
	if (handle == NULL)
	{
		return -1;
	}

	StreamInfo_T *pstStreamInfo = (StreamInfo_T *)handle;

#if DEBUG_ES_FILE
	if ((pstStreamInfo->bWriteFile == TRUE) &&
		(pstStreamInfo->fd > 0))
	{
		close(pstStreamInfo->fd);
		pstStreamInfo->fd = 0;

		ST_DBG("close %s success\n", pstStreamInfo->szDebugFile);
	}
#endif
	ST_DBG("close \"%s\" success\n", pstStreamInfo->szStreamName);

	HI_MPI_VENC_CloseFd(pstStreamInfo->vencChn);

	free(pstStreamInfo);

	return 0;
}

HI_S32 SAMPLE_RtspServerStart(Stream_Attr_T *pstStreamAttr, HI_U32 count)
{
	unsigned int rtspServerPortNum = RTSP_LISTEN_PORT;
	int iRet = 0;
	char* urlPrefix = NULL;
	HI_U32 i = 0;
	ServerMediaSession* mediaSession = NULL;
	ServerMediaSubsession* subSession = NULL;
	Live555RTSPServer *pRTSPServer = NULL;

	pRTSPServer = new Live555RTSPServer();
	if (pRTSPServer == NULL)
	{
		printf("malloc error\n");
		return -1;
	}

	iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
	while (iRet < 0)
	{
		rtspServerPortNum++;

		if (rtspServerPortNum > 65535)
		{
			printf("Failed to create RTSP server: %s\n", pRTSPServer->getResultMsg());
			delete pRTSPServer;
			pRTSPServer = NULL;
			return -2;
		}

		iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
	}

	urlPrefix = pRTSPServer->rtspURLPrefix();
	printf("=================URL===================\n");
	for (i = 0; i < count; i ++)
	{
		printf("%s%s\n", urlPrefix, pstStreamAttr[i].pszStreamName);
	}
	printf("=================URL===================\n");

	for (i = 0; i < count; i ++)
	{
		pRTSPServer->createServerMediaSession(mediaSession,
											  pstStreamAttr[i].pszStreamName,
											  NULL, NULL);

		if (pstStreamAttr[i].enPayLoad == PT_H264)
		{
			subSession = WW_H264VideoFileServerMediaSubsession::createNew(
										*(pRTSPServer->GetUsageEnvironmentObj()),
										pstStreamAttr[i].pszStreamName,
										SAMPLE_OpenStream,
										SAMPLE_VideoReadStream,
										SAMPLE_CloseStream, 30);
		}
		else if (pstStreamAttr[i].enPayLoad == PT_H265)
		{
			subSession = WW_H265VideoFileServerMediaSubsession::createNew(
										*(pRTSPServer->GetUsageEnvironmentObj()),
										pstStreamAttr[i].pszStreamName,
										SAMPLE_OpenStream,
										SAMPLE_VideoReadStream,
										SAMPLE_CloseStream, 30);
		}

		pRTSPServer->addSubsession(mediaSession, subSession);
		pRTSPServer->addServerMediaSession(mediaSession);
	}

	pRTSPServer->Start();

	g_pRTSPServer = pRTSPServer;

	return 0;
}
#endif

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_RTSP_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : to process abnormal case - the case of stream venc
******************************************************************************/
void SAMPLE_RTSP_StreamHandleSig(HI_S32 signo)
{

    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}


/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
HI_S32 SAMPLE_RTSP_1080P_CLASSIC(HI_VOID)
{
    // PAYLOAD_TYPE_E enPayLoad[3] = {PT_H264, PT_H265, PT_H264};
    // PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_VGA, PIC_D1};
	PAYLOAD_TYPE_E enPayLoad[3];
	PIC_SIZE_E enSize[3];
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChnArray[3];
	VENC_CHN VencChn = 0;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;

    HI_S32 s32ChnNum;

	HI_U32 i = 0;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

	for (i = 0; i < ARRAY_SIZE(g_stStreamAttr1080P_CLASSIC); i ++)
	{
		enPayLoad[i] = g_stStreamAttr1080P_CLASSIC[i].enPayLoad;
		enSize[i] = g_stStreamAttr1080P_CLASSIC[i].enSize;
		VencChnArray[i] = g_stStreamAttr1080P_CLASSIC[i].VencChn;
	}

    if ((SONY_IMX178_LVDS_5M_30FPS == SENSOR_TYPE)
        || (APTINA_AR0330_MIPI_1536P_25FPS == SENSOR_TYPE)
        || (APTINA_AR0330_MIPI_1296P_25FPS == SENSOR_TYPE))
    {
        s32ChnNum = 2;
    }
    else
    {
        s32ChnNum = 3;
    }

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);
    switch (SENSOR_TYPE)
    {
        case SONY_IMX178_LVDS_5M_30FPS:
        case APTINA_AR0330_MIPI_1536P_25FPS:
        case APTINA_AR0330_MIPI_1296P_25FPS:
            enSize[1] = PIC_VGA;
            break;
        default:
            break;
    }

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 20;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width       = stSize.u32Width;
    stVpssChnMode.u32Height      = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    VpssChn = 1;
    stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble         = HI_FALSE;
    stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width        = stSize.u32Width;
    stVpssChnMode.u32Height       = stSize.u32Height;
    stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    if ((SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1536P_25FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1296P_25FPS != SENSOR_TYPE))
    {

        VpssChn = 2;
        stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
        stVpssChnMode.bDouble		= HI_FALSE;
        stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
        stVpssChnMode.u32Width		= 720;
        stVpssChnMode.u32Height 	= (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 576 : 480;;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        stVpssChnAttr.s32SrcFrameRate = -1;
        stVpssChnAttr.s32DstFrameRate = -1;

        s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Enable vpss chn failed!\n");
            goto END_VENC_1080P_CLASSIC_4;
        }
    }
    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = VencChnArray[0];
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /*** 1080p **/
    VpssChn = 1;
    VencChn = VencChnArray[1];
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], \
                                    gs_enNorm, enSize[1], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /*** D1 **/
    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        VencChn = VencChnArray[2];
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[2], \
                                        gs_enNorm, enSize[2], enRcMode, u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1080P_CLASSIC_5;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1080P_CLASSIC_5;
        }
    }

	g_pstStreamAttr = g_stStreamAttr1080P_CLASSIC;
	g_streamNum = ARRAY_SIZE(g_stStreamAttr1080P_CLASSIC);

	SAMPLE_RtspServerStart(g_pstStreamAttr, g_streamNum);

#if 0
    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }
#endif

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);


    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        VencChn = 2;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }

    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    }
END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}



/******************************************************************************
* function :  1*1080p MJPEG encode + 1*1080p jpeg
******************************************************************************/
HI_S32 SAMPLE_RTSP_1080P_MJPEG_JPEG(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad = PT_MJPEG;
    PIC_SIZE_E enSize = PIC_HD1080;
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    HI_S32 s32ChnNum = 1;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    HI_S32 i = 0;
    char ch;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    stVbConf.u32MaxPoolCnt = 128;

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize);

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 12;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_MJPEG_JPEG_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_MJPEG_JPEG_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_MJPEG_JPEG_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_MJPEG_JPEG_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_MJPEG_JPEG_3;
    }


    VpssChn = 0;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_MJPEG_JPEG_4;
    }


    VpssChn = 1;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_MJPEG_JPEG_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad, \
                                    gs_enNorm, enSize, enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    VpssGrp = 0;
    VpssChn = 1;
    VencChn = 1;
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn, &stSize, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }


    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    printf("press 'q' to exit sample!\nperess ENTER to capture one picture to file\n");
    i = 0;
    while ((ch = (char)getchar()) != 'q')
    {
        s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencChn, HI_TRUE, HI_FALSE);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: sanp process failed!\n", __FUNCTION__);
            break;
        }
        printf("snap %d success!\n", i);
        i++;
    }

    printf("please press ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 8: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_MJPEG_JPEG_5:
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
END_VENC_MJPEG_JPEG_4:    //vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_MJPEG_JPEG_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_MJPEG_JPEG_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_MJPEG_JPEG_1:    //vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_MJPEG_JPEG_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

/******************************************************************************
* function :  low delay encode
******************************************************************************/
HI_S32 SAMPLE_RTSP_LOW_DELAY(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad[2] = {PT_H264, PT_H264};
    PIC_SIZE_E enSize[2] = {PIC_HD1080, PIC_VGA};
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;
    HI_U32 u32Priority;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_LOW_DELAY_INFO_S stLowDelayInfo;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    HI_S32 s32ChnNum = 2;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    printf("u32BlkSize: %d\n", u32BlkSize);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 6;

    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_LOW_DELAY_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_LOW_DELAY_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_LOW_DELAY_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_LOW_DELAY_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_LOW_DELAY_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_LOW_DELAY_4;
    }

    VpssChn = 1;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_LOW_DELAY_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_LOW_DELAY_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    /*set chnl Priority*/
    s32Ret = HI_MPI_VENC_GetChnlPriority(VencChn, &u32Priority);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get Chnl Priority failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    u32Priority = 1;

    s32Ret = HI_MPI_VENC_SetChnlPriority(VencChn, u32Priority);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Chnl Priority failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    /*set low delay*/
#if 1
    s32Ret = HI_MPI_VPSS_GetLowDelayAttr(VpssGrp, VpssChn, &stLowDelayInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_GetLowDelayAttr failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }
    stLowDelayInfo.bEnable = HI_TRUE;
    stLowDelayInfo.u32LineCnt = stVpssChnMode.u32Height / 2;
    s32Ret = HI_MPI_VPSS_SetLowDelayAttr(VpssGrp, VpssChn, &stLowDelayInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetLowDelayAttr failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }
#endif
    /*** 1080p **/
    VpssChn = 1;
    VencChn = 1;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], \
                                    gs_enNorm, enSize[1], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_LOW_DELAY_5;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_LOW_DELAY_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_LOW_DELAY_4:   //vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_LOW_DELAY_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_LOW_DELAY_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_LOW_DELAY_1:   //vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_LOW_DELAY_0:   //system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


HI_S32 SAMPLE_RTSP_ROIBG_CLASSIC(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD720, PIC_D1};
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;
    VENC_ROI_CFG_S  stVencRoiCfg;
    VENC_ROIBG_FRAME_RATE_S stRoiBgFrameRate;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    HI_S32 s32ChnNum = 1;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 6;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 6;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad, \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }
    stVencRoiCfg.bAbsQp   = HI_TRUE;
    stVencRoiCfg.bEnable  = HI_TRUE;
    stVencRoiCfg.s32Qp    = 30;
    stVencRoiCfg.u32Index = 0;
    stVencRoiCfg.stRect.s32X = 64;
    stVencRoiCfg.stRect.s32Y = 64;
    stVencRoiCfg.stRect.u32Height = 256;
    stVencRoiCfg.stRect.u32Width = 256;
    s32Ret = HI_MPI_VENC_SetRoiCfg(VencChn, &stVencRoiCfg);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = HI_MPI_VENC_GetRoiBgFrameRate(VencChn, &stRoiBgFrameRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_GetRoiBgFrameRate failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }
    stRoiBgFrameRate.s32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    stRoiBgFrameRate.s32DstFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 5 : 15;

    s32Ret = HI_MPI_VENC_SetRoiBgFrameRate(VencChn, &stRoiBgFrameRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_SetRoiBgFrameRate!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }
    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    printf("please press ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);



    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

HI_S32 SAMPLE_RTSP_SVC_H264(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD720, PIC_D1};
    HI_U32 u32Profile = 3;/* Svc-t */

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;


    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    HI_S32 s32ChnNum = 1;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 6;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 6;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm	  = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble		= HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width		= stSize.u32Width;
    stVpssChnMode.u32Height 	= stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad, \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);

    printf("SAMPLE_COMM_VENC_Start is ok\n");

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);

    printf("SAMPLE_COMM_VENC_BindVpss is ok\n");


    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream_Svc_t(s32ChnNum);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    printf("please press ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

    printf("SAMPLE_COMM_VENC_StopGetStream is ok\n");
END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);



    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_1080P_CLASSIC_3:	 //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:	 //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
HI_S32 SAMPLE_RTSP_H264_IntraRefresh(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_H264, PT_H264};
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD1080, PIC_D1};
	HI_U32 u32Profile = 2;
	char d;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;
    VENC_CHN VencChn1 = 1;
    SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;

    HI_S32 s32ChnNum = 2;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);
    switch(SENSOR_TYPE)
    {
        case SONY_IMX178_LVDS_5M_30FPS:
        case APTINA_AR0330_MIPI_1536P_25FPS:
        case APTINA_AR0330_MIPI_1296P_25FPS:
			enSize[1] = PIC_HD720;
            break;
        default:
            break;
    }

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt =20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 20;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width       = stSize.u32Width;
    stVpssChnMode.u32Height      = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    VpssChn = 1;
    stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble         = HI_FALSE;
    stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width        = stSize.u32Width;
    stVpssChnMode.u32Height       = stSize.u32Height;
    stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

	if((SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1536P_25FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1296P_25FPS != SENSOR_TYPE))
	{

		VpssChn = 2;
		stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
		stVpssChnMode.bDouble		= HI_FALSE;
		stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
		stVpssChnMode.u32Width		= 720;
		stVpssChnMode.u32Height 	= (VIDEO_ENCODING_MODE_PAL==gs_enNorm)?576:480;;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

		stVpssChnAttr.s32SrcFrameRate = -1;
		stVpssChnAttr.s32DstFrameRate = -1;

		s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Enable vpss chn failed!\n");
			goto END_VENC_1080P_CLASSIC_4;
		}
	}
    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch(c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], gs_enNorm, enSize[0], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    VpssGrp  = 0;
    VpssChn  = 0;
    VencChn1 = 1;

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn1, enPayLoad[0], gs_enNorm, enSize[0], enRcMode,u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    {
        VENC_RC_PARAM_S stRcParam;
        HI_MPI_VENC_GetRcParam(VencChn, &stRcParam);
        stRcParam.u32ThrdI[0] = 5;
        stRcParam.u32ThrdI[1] = 5;
        stRcParam.u32ThrdI[2] = 5;
        stRcParam.u32ThrdI[3] = 10;
        stRcParam.u32ThrdI[4] = 10;
        stRcParam.u32ThrdI[5] = 15;
        stRcParam.u32ThrdI[6] = 15;
        //stRcParam.u32ThrdI[7] = 25;
        //stRcParam.u32ThrdI[8] = 25;

        stRcParam.u32ThrdP[0] = 5;
        stRcParam.u32ThrdP[1] = 5;
        stRcParam.u32ThrdP[2] = 5;
        stRcParam.u32ThrdP[3] = 9;
        stRcParam.u32ThrdP[4] = 9;
        stRcParam.u32ThrdP[5] = 15;
        stRcParam.u32ThrdP[6] = 15;
        //stRcParam.u32ThrdP[7] = 25;
        //stRcParam.u32ThrdP[8] = 25;

        stRcParam.u32RowQpDelta = 4;

        stRcParam.stParamH264Cbr.s32IPQPDelta = -1;
        HI_MPI_VENC_SetRcParam(VencChn, &stRcParam);
        HI_MPI_VENC_RequestIDR(VencChn, HI_TRUE);
    }

    {
        VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
        stIntraRefresh.bRefreshEnable = HI_TRUE;
        stIntraRefresh.bISliceEnable  = HI_TRUE;
        stIntraRefresh.u32RefreshLineNum = 9;
        stIntraRefresh.u32ReqIQp = 40;

        HI_MPI_VENC_SetIntraRefresh(VencChn, &stIntraRefresh);
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn1, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }


    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

//    printf("please press twice ENTER to exit this sample\n");
//    getchar();
//    getchar();

	printf("Input 'i' to request IDR, 'q' to exit this sample\n");

    while (1)
    {
        d = (char)getchar();

        if (d == 'i')
        {
            printf("Insert IDR\n");
            HI_MPI_VENC_RequestIDR(0, HI_TRUE);
        }
        else if (d == 'q')
        {
            printf("Quit! \n");
            break;
        }


    }



    //printf("please press twice ENTER to exit this sample\n");
    //getchar();
    //getchar();


    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    SAMPLE_COMM_VENC_UnBindVpss(VencChn1, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn1);


    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
#if 0
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	if(SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
	{
		VpssChn = 2;
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	}
#endif

END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

HI_S32 SAMPLE_RTSP_AdvSingleP(HI_VOID)
{
    PAYLOAD_TYPE_E enPayLoad[3] = {PT_H265, PT_H264, PT_H264};
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD1080, PIC_D1};
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    VENC_PARAM_REF_EX_S stRefParam = {0};

    HI_S32 s32ChnNum;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;


    if ((SONY_IMX178_LVDS_5M_30FPS == SENSOR_TYPE)
        || (APTINA_AR0330_MIPI_1536P_25FPS == SENSOR_TYPE)
        || (APTINA_AR0330_MIPI_1296P_25FPS == SENSOR_TYPE))
    {
        s32ChnNum = 2;
    }
    else
    {
        s32ChnNum = 3;
    }

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);
    switch (SENSOR_TYPE)
    {
        case SONY_IMX178_LVDS_5M_30FPS:
        case APTINA_AR0330_MIPI_1536P_25FPS:
        case APTINA_AR0330_MIPI_1296P_25FPS:
            enSize[1] = PIC_VGA;
            break;
        default:
            break;
    }

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 20;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 20;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width       = stSize.u32Width;
    stVpssChnMode.u32Height      = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    VpssChn = 1;
    stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble         = HI_FALSE;
    stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width        = stSize.u32Width;
    stVpssChnMode.u32Height       = stSize.u32Height;
    stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    if ((SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1536P_25FPS != SENSOR_TYPE)
        && (APTINA_AR0330_MIPI_1296P_25FPS != SENSOR_TYPE))
    {

        VpssChn = 2;
        stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
        stVpssChnMode.bDouble		= HI_FALSE;
        stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
        stVpssChnMode.u32Width		= 720;
        stVpssChnMode.u32Height 	= (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 576 : 480;;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        stVpssChnAttr.s32SrcFrameRate = -1;
        stVpssChnAttr.s32DstFrameRate = -1;

        s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Enable vpss chn failed!\n");
            goto END_VENC_1080P_CLASSIC_4;
        }
    }
    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;

        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;

    s32Ret = SAMPLE_COMM_VENC_StartSmartCodec(VencChn, enPayLoad[0], \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = HI_MPI_VENC_GetRefParamEx(VencChn, &stRefParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get Ref Param Ex failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    stRefParam.u32Base = 2;
    stRefParam.u32Enhance = 1;
    stRefParam.bEnablePred = HI_TRUE;
    stRefParam.bVirtualIEnable = HI_TRUE;
    stRefParam.u32VirtualIInterval = 30;
    stRefParam.s32VirtualIQpDelta = 0;
    s32Ret = HI_MPI_VENC_SetRefParamEx(VencChn, &stRefParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Ref Param Ex failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /*** 1080p **/
    VpssChn = 1;
    VencChn = 1;
    s32Ret = SAMPLE_COMM_VENC_StartSmartCodec(VencChn, enPayLoad[1], \
                                    gs_enNorm, enSize[1], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = HI_MPI_VENC_GetRefParamEx(VencChn, &stRefParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get Ref Param Ex failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    stRefParam.u32Base = 2;
    stRefParam.u32Enhance = 1;
    stRefParam.bEnablePred = HI_TRUE;
    stRefParam.bVirtualIEnable = HI_TRUE;
    stRefParam.u32VirtualIInterval = 30;
    stRefParam.s32VirtualIQpDelta = 0;
    s32Ret = HI_MPI_VENC_SetRefParamEx(VencChn, &stRefParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Ref Param Ex failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /*** D1 **/
    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        VencChn = 2;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[2], \
                                        gs_enNorm, enSize[2], enRcMode, u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1080P_CLASSIC_5;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1080P_CLASSIC_5;
        }
    }
    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);


    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        VencChn = 2;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }

    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    }
END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}



/******************************************************************************
* function :  Thumbnail of 1*1080p jpeg
******************************************************************************/
HI_S32 SAMPLE_RTSP_1080P_JPEG_Thumb(HI_VOID)
{
    PIC_SIZE_E enSize = PIC_HD1080;
    ISP_DCF_INFO_S stIspDCF;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    HI_S32 i = 0;
    char ch;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    stVbConf.u32MaxPoolCnt = 128;

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize);

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 12;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init_With_DCF(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_MJPEG_JPEG_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_MJPEG_JPEG_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_MJPEG_JPEG_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_MJPEG_JPEG_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_MJPEG_JPEG_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble       = HI_FALSE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width      = stSize.u32Width;
    stVpssChnMode.u32Height     = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_MJPEG_JPEG_4;
    }

    /******************************************
     step 5: set CDF info
    ******************************************/

    HI_MPI_ISP_GetDCFInfo(&stIspDCF);

    //description: Thumbnail test
    memcpy(stIspDCF.au8ImageDescription,"Thumbnail test",strlen("Thumbnail test"));

    //manufacturer: Hisilicon
    memcpy(stIspDCF.au8Make,"Hisilicon",strlen("Hisilicon"));

    //model number: Hisilicon IP Camera
    memcpy(stIspDCF.au8Model,"Hisilicon IP Camera",strlen("Hisilicon IP Camera"));

    //firmware version: v.1.1.0
    memcpy(stIspDCF.au8Software,"v.1.1.0",strlen("v.1.1.0"));

    stIspDCF.u16ISOSpeedRatings         = 500;
    stIspDCF.u32ExposureBiasValue       = 5;
    stIspDCF.u32ExposureTime            = 0x00010004;
    stIspDCF.u32FNumber                 = 0x0001000f;
    stIspDCF.u32FocalLength             = 0x00640001;
    stIspDCF.u32MaxApertureValue        = 0x00010001;
    stIspDCF.u8Contrast                 = 5;
    stIspDCF.u8CustomRendered           = 0;
    stIspDCF.u8ExposureMode             = 0;
    stIspDCF.u8ExposureProgram          = 1;
    stIspDCF.u8FocalLengthIn35mmFilm    = 1;
    stIspDCF.u8GainControl              = 1;
    stIspDCF.u8LightSource              = 1;
    stIspDCF.u8MeteringMode             = 1;
    stIspDCF.u8Saturation               = 1;
    stIspDCF.u8SceneCaptureType         = 1;
    stIspDCF.u8SceneType                = 0;
    stIspDCF.u8Sharpness                = 5;
    stIspDCF.u8WhiteBalance             = 1;

    HI_MPI_ISP_SetDCFInfo(&stIspDCF);

    /******************************************
     step 6: start stream venc
    ******************************************/

    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn, &stSize, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_JPEG_5;
    }

    /******************************************
     step 7: stream venc process -- get stream, then save it to file.
    ******************************************/
    printf("press 'q' to exit sample!\nperess ENTER to capture one picture to file\n");
    i = 0;
    while ((ch = (char)getchar()) != 'q')
    {
        s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencChn, HI_TRUE, HI_TRUE);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: sanp process failed!\n", __FUNCTION__);
            break;
        }
        printf("snap %d success!\n", i);
        i++;
    }

    printf("please press ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 8: exit process
    ******************************************/
END_VENC_MJPEG_JPEG_5:
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);

END_VENC_MJPEG_JPEG_4:    //vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_MJPEG_JPEG_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_MJPEG_JPEG_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_MJPEG_JPEG_1:    //vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_MJPEG_JPEG_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : video venc sample
******************************************************************************/
int main(int argc, char* argv[])
{
    HI_S32 s32Ret;
    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_RTSP_Usage(argv[0]);
        return HI_FAILURE;
    }

    signal(SIGINT, SAMPLE_RTSP_HandleSig);
    signal(SIGTERM, SAMPLE_RTSP_HandleSig);

    switch (*argv[1])
    {
        case '0':/* H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps */
            s32Ret = SAMPLE_RTSP_1080P_CLASSIC();
            break;
        case '1':/* 1*1080p mjpeg encode + 1*1080p jpeg  */
            s32Ret = SAMPLE_RTSP_1080P_MJPEG_JPEG();
            break;
        case '2':/* low delay */
            s32Ret = SAMPLE_RTSP_LOW_DELAY();
            break;
        case '3':/* roibg framerate */
            s32Ret = SAMPLE_RTSP_ROIBG_CLASSIC();
            break;
        case '4':/* H.264 Svc-t */
            s32Ret = SAMPLE_RTSP_SVC_H264();
            break;
        case '5':/* H.264 Intra Refresh */
            s32Ret = SAMPLE_RTSP_H264_IntraRefresh();
            break;
        case '6':
            s32Ret = SAMPLE_RTSP_AdvSingleP();
            break;
        case '7':/* Thumbnail of 1*1080p jpeg  */
            s32Ret = SAMPLE_RTSP_1080P_JPEG_Thumb();
            break;
        default:
            printf("the index is invaild!\n");
            SAMPLE_RTSP_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
    { printf("program exit normally!\n"); }
    else
    { printf("program exit abnormally!\n"); }
    exit(s32Ret);
}


