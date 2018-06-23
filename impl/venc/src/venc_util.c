/**
 * @file venc_util.c
 *
 * Small utilities helps to monitor and debugging.
 */
//#if defined(CONFIG_ARCH_INFINITY2)
#if defined(__linux__)
#include "inc/venc_util.h"
//#include "mi_common_datatype.h"
#if defined(CONFIG_ARCH_INFINITY2)
#include "ms_types.h"
#endif
//#include <sys/time.h>
#include <linux/ktime.h>
//#include <time.h>
#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#define TIMEVAL_US_DIFF(start, end)     ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

//void* user, callback
void mi_venc_InitSw(MI_VENC_Utilization_t* pstUtil, MI_U32 u32ReportCnt, MI_U32 u32ReportTime, MI_VENC_UtilizationReportFunc pfnReportFunc, void* pUser)
{
    if(pstUtil == NULL)
        return;
    memset(pstUtil, 0, sizeof(*pstUtil));
    pstUtil->cfg.u32ReportCnt = u32ReportCnt;
    pstUtil->cfg.u32ReportTime = u32ReportTime;
    pstUtil->cfg.pfnReport = pfnReportFunc;
    pstUtil->cfg.pUser = pUser;
    pstUtil->d.bRestart = TRUE;
}

void mi_venc_ResetSw(MI_VENC_Utilization_t* pstUtil)
{
	if(pstUtil == NULL)
		return;
	memset(&pstUtil->d, 0, sizeof(pstUtil->d));
	pstUtil->d.bRestart = TRUE;
}

void mi_venc_RecSwStart(MI_VENC_Utilization_t* pstUtil, MI_U32 u32StartIdx)
{
    if(pstUtil == NULL)
        return;
    if(u32StartIdx >= MI_VENC_UTIL_MAX_END)
        return;
    if(pstUtil->d.bRestart)
    {
        mi_venc_ResetSw(pstUtil);
        pstUtil->d.bRestart = FALSE;
        do_gettimeofday(&pstUtil->d.stTimeReset);
    }
    do_gettimeofday(&pstUtil->d.stTimeStart[u32StartIdx]);
}


void mi_venc_RecSwEnd(MI_VENC_Utilization_t* pstUtil, MI_U32 u32EndIdx, MI_U32 u32Value)
{
    MI_BOOL bLastIdx = FALSE;

    if (pstUtil == NULL)
        return;
    if (u32EndIdx >= MI_VENC_UTIL_MAX_END)
        return;

    do_gettimeofday(&pstUtil->d.stTimeEnd[u32EndIdx]);
    pstUtil->d.u32DiffUs = TIMEVAL_US_DIFF(pstUtil->d.stTimeReset, pstUtil->d.stTimeEnd[u32EndIdx]);
    pstUtil->d.u32ValueSum += u32Value;
    if(u32EndIdx == 0)
        pstUtil->d.u32Cnt++;
    pstUtil->d.au64SumDiff[u32EndIdx] += TIMEVAL_US_DIFF(pstUtil->d.stTimeStart[u32EndIdx], pstUtil->d.stTimeEnd[u32EndIdx]);
    if((u32EndIdx == MI_VENC_UTIL_MAX_END - 1) || pstUtil->d.au64SumDiff[u32EndIdx + 1] == 0)
    {
        bLastIdx = TRUE;
    }
    if(bLastIdx)
    {
        if((pstUtil->cfg.u32ReportTime != 0 && (pstUtil->d.u32DiffUs >= pstUtil->cfg.u32ReportTime)) ||
           (pstUtil->cfg.u32ReportCnt != 0 && (pstUtil->d.u32Cnt >= pstUtil->cfg.u32ReportCnt)))
        {
            pstUtil->d.u64TimeSum = TIMEVAL_US_DIFF(pstUtil->d.stTimeReset, pstUtil->d.stTimeEnd[u32EndIdx]);

            if(pstUtil->cfg.pfnReport)
                pstUtil->cfg.pfnReport(pstUtil, pstUtil->cfg.pUser);
            pstUtil->d.bRestart = TRUE;
        }
    }
}
#endif

void mi_venc_PrintHex(char* title, void* buf, int num)
{
    int i;
    char *data = (char *) buf;

    printk("%s\nOffset(h) \n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
           "----------------------------------------------------------", title);
    for(i = 0; i < num; i++)
    {
        if((i & 0xF) == 0xF)
        {
            //CamOsPrintf("\n%08X   ", i);
            printk("\n");
        }
        printk("%02X ", data[i]);
    }
    printk("\n");
}
