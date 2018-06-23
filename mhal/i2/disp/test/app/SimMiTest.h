
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
#ifndef __SIMMI_TEST_H__
#define __SIMMI_TEST_H__



#define  u64    unsigned long long
#define  s64    signed long long
#define  u32    unsigned long
#define  s32    signed long
#define  u16    unsigned short
#define  s16    signed short
#define  u8     unsigned char
#define  s8     signed char
#define  bool   unsigned char

//---------------------------------------------------------------
// define
//---------------------------------------------------------------
#define SIM_MI_DBG_EN 1

#if SIM_MI_DBG_EN
#define SIM_MI_DBG(_fmt, _args...) printf(_fmt, ## _args)
#else
#define SIM_MI_DBG(_fmt, _args...)
#endif

#define SIMMI_DISP_IOCTL_CHAR        "MI_DISP"
#define SIMMI_HDMITX_IOCTL_CHAR      "MI_HDMITX"
#define SIMMI_GFX_IOCTL_CHAR         "MI_GFX"


typedef enum
{
    E_SIMMI_DISP_IO    = 0,
    E_SIMMI_HDMITX_IO  = 1,
    E_SIMMI_GFX_IO     = 2,
    E_MENU_NUM,
} MENU_TYPE;

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------


typedef enum
{
    E_DRV_ID_SIMMI_MODULE,
    E_DRV_ID_MSYS_MODULE,
    E_DRV_ID_MEM_MODULE,
    E_DRV_ID_NUM,
}EN_DRV_ID_TYPE;


typedef enum
{
    E_MULTI_INST_IPW_THREAD1    = 0x001,
    E_MULTI_INST_IPW_THREAD2    = 0x002,
    E_MULTI_INST_IPW2_THREAD1   = 0x004,
    E_MULTI_INST_IPW2_THREAD2   = 0x008,
    E_MULTI_INST_DIPR_THREAD1   = 0x010,
    E_MULTI_INST_DIPR_THREAD2   = 0x020,
    E_MULTI_INST_IPM2_THREAD1   = 0x040,
    E_MULTI_INST_IPM2_THREAD2   = 0x080,
    E_MULTI_INST_HVSP_THREAD1   = 0x100,
    E_MULTI_INST_HVSP_THREAD2   = 0x200,
}MULTIINST_TEST_OPTION;

#endif

#ifdef __SIMMI_TEST_C__
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE int Parsing_String(char *src, char *dest);
INTERFACE int Parsing_OnOFF(char *pOnOFF);
INTERFACE int Parsing_Num(char *pnum);
INTERFACE int Parsing_HexNum(char *pnum);
INTERFACE EN_DRV_ID_TYPE Parsing_Drv_Id(char *pStr);
INTERFACE void _ReadFile(void **pBinbuf, long *pFileSize, char *pFileName);
INTERFACE FILE* _OpenFile(char *pFileName);
INTERFACE FILE* _OpenFile_Repeat(char *pFileName);
INTERFACE void _CloseFile(FILE *pFile,unsigned char *u64Vir_data,unsigned long long buffersize);
INTERFACE void Buffer_clear(unsigned char *u64Vir_data,unsigned long long buffersize);
INTERFACE unsigned long long _GetSystemTimeStamp (void);
INTERFACE int Open_Module(EN_DRV_ID_TYPE enDrvID);
INTERFACE int Open_Device(EN_DRV_ID_TYPE enDrvID);
INTERFACE pthread_t _CreateTestThread(void *pfunc,void *pParam);
INTERFACE MSYS_DMEM_INFO _AllocateDmem(char *name ,unsigned int buffsize);
INTERFACE MSYS_ADDR_TRANSLATION_INFO _TranslationPhyToMIU(MSYS_DMEM_INFO stMsysMemInfo);
INTERFACE int _FreeDeme(char *name);

