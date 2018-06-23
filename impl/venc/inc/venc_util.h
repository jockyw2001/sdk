/*
 * file_access.h
 *
 *  Created on: Nov 28, 2017
 *      Author: giggs.huang
 */

#ifndef __VENC_UTIL_H__
#define __VENC_UTIL_H__


//#if defined(CONFIG_ARCH_INFINITY2)
#if defined(__linux__)
//#include <sys/time.h>
#include <linux/ktime.h>
#include "mi_common_datatype.h"

typedef struct MI_VENC_Time_s {
    struct timeval stTimeStart, stTimeEnd;
    struct timezone stTimeZone;
} MI_VENC_Time_t;

typedef struct MI_VENC_FPS_s {
    struct timeval stTimeStart, stTimeEnd;
    struct timezone stTimeZone;
    MI_U32 u32DiffUs;
    MI_U32 u32TotalBits;
    MI_U32 u32FrameCnt;
    MI_BOOL bRestart;
} MI_VENC_FPS_t;

typedef struct MI_VENC_Utilization_s MI_VENC_Utilization_t;
typedef void (*MI_VENC_UtilizationReportFunc) (MI_VENC_Utilization_t* pstUtil, void* pUser);

#define MI_VENC_UTIL_MAX_END (3)
typedef struct MI_VENC_Utilization_s {
	struct {
		MI_U32 u32ReportCnt;
		MI_U32 u32ReportTime;
		MI_VENC_UtilizationReportFunc pfnReport;
		void* pUser;
	} cfg;
	struct {
		struct timeval stTimeReset;
		struct timeval stTimeStart[MI_VENC_UTIL_MAX_END];
		MI_U64 au64SumDiff[MI_VENC_UTIL_MAX_END];
		struct timeval stTimeEnd[MI_VENC_UTIL_MAX_END];
		MI_U64 u64MinDiff[MI_VENC_UTIL_MAX_END];
        MI_U64 u64MaxDiff[MI_VENC_UTIL_MAX_END];
		MI_U64 u64TimeSum;
		//struct timezone stTimeZone;
		MI_U32 u32DiffUs;
		MI_U32 u32ValueSum;
		MI_U32 u32Cnt;
		MI_BOOL bRestart;
	} d;
} MI_VENC_Utilization_t;
#else
typedef struct MI_VENC_FPS_s MI_VENC_FPS_t;
typedef struct MI_VENC_Utilization_s MI_VENC_Utilization_t;
#endif

void mi_venc_InitSw(MI_VENC_Utilization_t* pstUtil, MI_U32 u32ReportCnt, MI_U32 u32ReportTime, MI_VENC_UtilizationReportFunc pfnReportFunc, void* pUser);
void mi_venc_ResetSw(MI_VENC_Utilization_t* pstUtil);
void mi_venc_RecSwStart(MI_VENC_Utilization_t* pstUtil, MI_U32 u32StartIdx);
void mi_venc_RecSwEnd(MI_VENC_Utilization_t* pstUtil, MI_U32 u32EndIdx, MI_U32 u32Value);

#endif

void mi_venc_PrintHex(char* title, void* buf, int num);
