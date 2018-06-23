
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */
#define __SIMMI_TEST_C__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include <pthread.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
//#include <linux/delay.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <linux/netlink.h>
#include <linux/fs.h>

#include "mdrv_msys_io.h"
#include "mdrv_msys_io_st.h"

#include "SimMiTest.h"
#include "drv_scl_verchk.h"
#include "SimMiModuleIoctl.h"
#include "SimMiModuleIoctlDataType.h"


//---------------------------------------------------------------
// define
//---------------------------------------------------------------


//---------------------------------------------------------------
// enum
//---------------------------------------------------------------
char menu_string[E_MENU_NUM][20] =
{
    {SIMMI_DISP_IOCTL_CHAR},
    {SIMMI_HDMITX_IOCTL_CHAR},
    {SIMMI_GFX_IOCTL_CHAR},
};

int g_FD[E_DRV_ID_NUM] = {-1, -1, -1};


//---------------------------------------------------------------
// Prototype
//---------------------------------------------------------------
extern void Test_DISP(int argc, char *argv[]);
//----------------------------------------------------------------------------------
// parsing funciton
//----------------------------------------------------------------------------------
int Parsing_String(char *src, char *dest)
{
    if(strcmp(src, dest) == 0)
        return 1;
    else
        return 0;
}

int Parsing_OnOFF(char *pOnOFF)
{
    int bret;

    if(strcmp(pOnOFF, "1") == 0)
        bret= 1;
    else
        bret = 0;

    return bret;
}

int Parsing_Num(char *pnum)
{
    int num;
    int bnegative;
    char *num_start = NULL;

    if(pnum[0] == '-')
    {
        bnegative = 1;
        num_start = &pnum[1];
    }
    else
    {
        bnegative = 0;
        num_start = pnum;
    }

    num = strtol(num_start, NULL, 10);

    if(bnegative)
        return -num;
    else
        return num;
}

int Parsing_HexNum(char *pnum)
{
    int hexnum;

    hexnum = strtol(pnum, NULL, 16);
    return hexnum;
}


EN_DRV_ID_TYPE Parsing_Drv_Id(char *pStr)
{
    EN_DRV_ID_TYPE enID = E_DRV_ID_NUM;

    if(strcmp(pStr, "SIMMI") == 0)
        enID = E_DRV_ID_SIMMI_MODULE;

    return enID;
}


//----------------------------read file------------------------------------------------
void _ReadFile(void **pBinbuf, long *pFileSize, char *pFileName)
{
    FILE *pFile = NULL;
    long lFileSize = 0;
    long lFileRsize;
    char *pu8Buffer = NULL;

    SIM_MI_DBG("%s \n", pFileName);
    pFile = fopen(pFileName, "rb");
    if(pFile == NULL)
    {
        printf("Open File Fail %s !! \n", pFileName);
    }
    else
    {
        long i;
        char *buf;
        SIM_MI_DBG("Open File Success \n!!");

        fseek(pFile, 0, SEEK_END);//set seek start point
        lFileSize = ftell(pFile);//return position
        SIM_MI_DBG("Open File size:%d \n!!", (int)lFileSize);
        //pu8Buffer = (char*) malloc(lFileSize);
        SIM_MI_DBG("*pBinbuf=%lx\n!!",*pBinbuf);
        rewind(pFile);//back to file start point

        if(*pBinbuf)
        {
            lFileRsize = fread(*pBinbuf, 1, lFileSize, pFile);
            if(lFileRsize != lFileSize)
            {
                printf("Size is not correct %d \n", (int)lFileRsize);
            }
        }

        fclose(pFile);

        *pFileSize = lFileSize;

        //*pBinbuf = pu8Buffer;

    }
}
//----------------------------save file------------------------------------------------
FILE* _OpenFile(char *pFileName)
{
    FILE *pFile = NULL;
    pFile = fopen(pFileName, "wb");
    if(pFile == NULL)
    {
        printf("Open File Fail %s !! \n", pFileName);
    }
    else
    {
        SIM_MI_DBG("Open File Success %s !! \n", pFileName);
        fseek(pFile, 0, SEEK_END);
    }
    return pFile;
}
FILE* _OpenFile_Repeat(char *pFileName)
{
    FILE *pFile = NULL;
    pFile = fopen(pFileName, "ab");
    if(pFile == NULL)
    {
        printf("Open File Fail %s !! \n", pFileName);
    }
    else
    {
        fseek(pFile, 0, SEEK_END);
    }
    return pFile;
}


void _CloseFile(FILE *pFile,unsigned char *u64Vir_data,unsigned long long buffersize)
{
    munmap(u64Vir_data,buffersize);
    fclose(pFile);

}
void Buffer_clear(unsigned char *u64Vir_data,unsigned long long buffersize)
{
    memset(u64Vir_data, 0 , buffersize);
}

unsigned long long _GetSystemTimeStamp (void)
{
    struct timeval         tv;
    struct timezone tz;
    unsigned long long u64TimeStamp;
    gettimeofday(&tv,&tz);
    u64TimeStamp =tv.tv_sec* 1000000ULL+ tv.tv_usec;
    return u64TimeStamp;
}

//-----------------------------------------------------------------------------------
//OPNE DEV
int Open_Module(EN_DRV_ID_TYPE enDrvID)
{
    int Fd = -1;

    char device_name[E_DRV_ID_NUM][50] =
    {
        {"/dev/mhal_disp"},
        {"/dev/msys"},
        {"/dev/mem"},

    };
    if(enDrvID >= E_DRV_ID_NUM)
    {
        printf("ID is not correct\n");
        return 0;
    }

    Fd = open(&device_name[enDrvID][0], O_RDWR, S_IRUSR|S_IWUSR);

    if(Fd == -1)
    {
        printf("open %s fail\n", &device_name[enDrvID][0]);
    }
    else
    {
        SIM_MI_DBG("[open]open %s  %d sucess\n", &device_name[enDrvID][0],Fd);
    }
    return Fd;
}



int Open_Device(EN_DRV_ID_TYPE enDrvID)
{
    int ret = 1;
    char device_name[E_DRV_ID_NUM][50] =
    {
        {"/dev/mhal_disp"},
        {"/dev/msys"},
        {"/dev/mem"},
    };
    if(enDrvID >= E_DRV_ID_NUM)
    {
        printf("ID is not correct\n");
        return 0;
    }

    if(g_FD[enDrvID] != -1)
    {
    }
    else
    {
        //SIM_MI_DBG("Open: %s\n", &device_name[enDrvID][0]);
        g_FD[enDrvID] = open(&device_name[enDrvID][0], O_RDWR, S_IRUSR|S_IWUSR);

        if(g_FD[enDrvID] == -1)
        {
            printf("open %s fail\n", &device_name[enDrvID][0]);
            ret = 0;
        }
        else
        {
            SIM_MI_DBG("[open]open %s  %d sucess\n", &device_name[enDrvID][0],g_FD[enDrvID]);
            ret = 1;
        }
    }
    return ret;
}


pthread_t _CreateTestThread(void *pfunc,void *pParam)
{
    pthread_attr_t attr;
    pthread_t pThread;
    struct sched_param schp;
    memset(&schp, 0, sizeof(schp));
    schp.sched_priority = sched_get_priority_max(SCHED_FIFO);

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setschedparam(&attr, &schp);
    pthread_create(&pThread, &attr, pfunc, pParam);
    return pThread;
}


int _FreeDeme(char *name)
{
    MSYS_DMEM_INFO stMsysMemInfo;
    int bRet = 1;
    memset(&stMsysMemInfo, 0, sizeof(MSYS_DMEM_INFO));
    strcpy(stMsysMemInfo.name, name);

    stMsysMemInfo = FILL_VERCHK_TYPE(stMsysMemInfo, stMsysMemInfo.VerChk_Version,
        stMsysMemInfo.VerChk_Size,IOCTL_MSYS_VERSION);

    if(ioctl(g_FD[E_DRV_ID_MSYS_MODULE], IOCTL_MSYS_RELEASE_DMEM, &stMsysMemInfo))
    {
        printf("%s %d, fail\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    return bRet;
}

MSYS_DMEM_INFO _AllocateDmem(char *name ,unsigned int buffsize)
{
    MSYS_DMEM_INFO stMsysMemInfo;

    memset(&stMsysMemInfo, 0, sizeof(MSYS_DMEM_INFO));
    strcpy(stMsysMemInfo.name, name);

    stMsysMemInfo = FILL_VERCHK_TYPE(stMsysMemInfo, stMsysMemInfo.VerChk_Version,
        stMsysMemInfo.VerChk_Size,IOCTL_MSYS_VERSION);

    if(ioctl(g_FD[E_DRV_ID_MSYS_MODULE], IOCTL_MSYS_RELEASE_DMEM, &stMsysMemInfo))
    {
        printf("%s %d, fail\n", __FUNCTION__, __LINE__);
    }


    stMsysMemInfo.length =  buffsize;
    if(ioctl(g_FD[E_DRV_ID_MSYS_MODULE], IOCTL_MSYS_REQUEST_DMEM, &stMsysMemInfo))
    {
        printf("%s %d, fail\n", __FUNCTION__, __LINE__);
    }

    SIM_MI_DBG("[%s]Phy:%llx,Vir:%llx\n",__FUNCTION__, (stMsysMemInfo.phys),stMsysMemInfo.kvirt);
    return stMsysMemInfo;
}

MSYS_ADDR_TRANSLATION_INFO _TranslationPhyToMIU(MSYS_DMEM_INFO stMsysMemInfo)
{
    MSYS_ADDR_TRANSLATION_INFO           stMsysMemTransInfo;
    stMsysMemTransInfo.addr = stMsysMemInfo.phys;

    stMsysMemTransInfo = FILL_VERCHK_TYPE(stMsysMemTransInfo, stMsysMemTransInfo.VerChk_Version,
        stMsysMemTransInfo.VerChk_Size,IOCTL_MSYS_VERSION);

    ioctl(g_FD[E_DRV_ID_MSYS_MODULE], IOCTL_MSYS_PHYS_TO_MIU, &stMsysMemTransInfo);

    SIM_MI_DBG("[FPGA]MIU ADDR=%llx\n", stMsysMemTransInfo.addr);
    SIM_MI_DBG("[FPGA]stMsysMemInfo.phys=%llx\n", stMsysMemInfo.phys);
    SIM_MI_DBG("[FPGA]stMsysMemInfo.kvirt=%llx\n", stMsysMemInfo.kvirt);
    return stMsysMemTransInfo;
}


//----------------------------------------------------------------------------------
// Test Function
//----------------------------------------------------------------------------------
void Test_HDMITX(void)
{
    printf("%s %d\n", __FUNCTION__, __LINE__);
}

void Test_GFX(void)
{
    printf("%s %d\n", __FUNCTION__, __LINE__);
}




//----------------------------------------------------------------------------------
void Print_Menu(void)
{
    printf("---------Welcome SIM_MI TEST----------  \n");
    printf("------------- ioctl -------------  \n");
    printf("[%s]\n", &menu_string[E_SIMMI_DISP_IO][0]);
    printf("[%s]\n", &menu_string[E_SIMMI_HDMITX_IO][0]);
    printf("[%s]\n", &menu_string[E_SIMMI_GFX_IO][0]);
    printf("Q\n");

}
int main(int argc, char *argv[])
{
#if 0
    if(argc < 2)
    {
        Print_Menu();
        return 0;
    }
#endif

    Test_DISP(argc, argv);

#if 0
    if( Parsing_String(argv[1], SIMMI_DISP_IOCTL_CHAR) )
    {
        Test_DISP(argc, argv);
    }
    else if(Parsing_String(argv[1], SIMMI_HDMITX_IOCTL_CHAR))
    {
        Test_HDMITX();
    }
    else if(Parsing_String(argv[1], SIMMI_GFX_IOCTL_CHAR))
    {
        Test_GFX();
    }
    else
    {
        Print_Menu();
    }
#endif

	return 0;
}
