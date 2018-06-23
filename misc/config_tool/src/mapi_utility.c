//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

// headers of itself
#include "mapi_utility.h"
// headers of standard C libs
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <paths.h>

#include "iniparser.h"
#define MAPI_PRINTF(fmt, arg...) printf(fmt, ##arg)

        MI_BOOL     _GetU32(dictionary* d,const  char* s, MI_U32 *pu32Num,int notfound);
        MI_BOOL   _GetBOOL(dictionary* d, const char* s, MI_BOOL *pu8Bool);
        MI_BOOL  _GetString(dictionary* d,const  char* s, MI_U8 **ppu8String);
MI_BOOL _GetArrayItems(char* in_string, const char ***thelist, int *len);
MI_BOOL _GetU32(dictionary* d,const char* s, MI_U32 *pu32Num, int notfound)
{
    int u32temp;
    if( (s == NULL) || (d==NULL) ||(pu32Num == NULL))
        return FALSE;
    u32temp               = iniparser_getint(d, s,notfound);
    if(u32temp == notfound)
    {
        *pu32Num = (MI_U32)notfound;
        return FALSE;
    }
    *pu32Num = (MI_U32)u32temp;
    return TRUE;
}
MI_BOOL _GetBOOL(dictionary* d, const char* s, MI_BOOL *pu8Bool)
{
    int bTemp;
    if( s == NULL )
        return FALSE;
    bTemp               = iniparser_getboolean(d, s,-1);
    if(bTemp == 1)
    {
        *pu8Bool = TRUE;
        return TRUE;
    }
    else if(bTemp == 0)
    {
        *pu8Bool = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
MI_BOOL _GetString(dictionary* d, const char* s, MI_U8 **ppu8String)
{
    char *s_temp=NULL;
    if(s == NULL)
        return FALSE;
    s_temp = iniparser_getstring(d,s,NULL);
    if(s_temp == NULL)
    {
        return FALSE;
    }
    else
    {
        *ppu8String = (MI_U8*)strdup(s_temp);
        return TRUE;
    }
}
MI_BOOL _GetArrayItems(char* in_string, const char ***thelist, int *len)
{
    int length;
    char *temp_token;
    char delim[5];
    int strlength = strlen(in_string)+1;
    int counter =1;
    temp_token = (char*)malloc(strlength);
    if(temp_token == NULL)
    {
        return FALSE;
    }
    *len = 0;
    memset(temp_token, 0, strlength);
    //MAPI_PRINTF("array =%s\n",in_string);
    char *s_temp = in_string;
    int bRet =0;
    bRet =sscanf(s_temp,"%*[{]%[^{}]%[{}]%n ",temp_token,delim,&length);

    //MAPI_PRINTF("temp_token = \"%s\"\ndelim = \"%s\"\nlength =\"%d\"\n",temp_token,delim,length);

    //while(counter !=0 ||strcmp(delim,"{") ==0)
    do
    {
        if((strcmp(delim,"{") == 0) && (bRet != 0))
        {
            counter++;
            s_temp+=length;
            bRet =sscanf(s_temp,"%[^{}]%[{}]%n ",temp_token,delim,&length);
            //MAPI_PRINTF("1. \ntemp_token = \"%s\"\ndelim = \"%s\"\nlength =\"%d\"\n",temp_token,delim,length);
        }
        else if( (strcmp(delim,"}") == 0)&& (bRet != 0))
        {
            counter--;
            s_temp+=length;
            if(',' !=  temp_token[0]){
                *len += 1;
            }
            bRet =sscanf(s_temp,"%[^{}]%[{}]%n ",temp_token,delim,&length);
            //MAPI_PRINTF("2. \ntemp_token = \"%s\"\ndelim = \"%s\"\nlength =\"%d\"\n",temp_token,delim,length);
        }
        //MAPI_PRINTF("counter =%d\n",counter);
    }while((counter !=0) ||(strcmp(delim,"{") ==0));

    memset(temp_token, 0, strlength);
    //MAPI_PRINTF("array =%s\n",in_string);
    s_temp = in_string;
    bRet =0;
    bRet =sscanf(s_temp,"%*[{]%[^{}]%[{}]%n ",temp_token,delim,&length);
    char *buffer = malloc(sizeof(char*)*(*len+1)+strlength);
    memset(buffer, 0, sizeof(char*)*(*len+1)+strlength);
    const char **list = (const char **)buffer;
    int buf_off = sizeof(char*)*(*len+1);
    int list_idx = 0;

    counter =1;
    do
    {
        if((strcmp(delim,"{") == 0) && (bRet != 0))
        {
            counter++;
            s_temp+=length;
            bRet =sscanf(s_temp,"%[^{}]%[{}]%n ",temp_token,delim,&length);
            //MAPI_PRINTF("1. \ntemp_token = \"%s\"\ndelim = \"%s\"\nlength =\"%d\"\n",temp_token,delim,length);
        }
        else if( (strcmp(delim,"}") == 0)&& (bRet != 0))
        {
            counter--;
            s_temp+=length;
            if(',' !=  temp_token[0]){
                int tlen = strlen(temp_token)+1;
                list[list_idx++] = buffer + buf_off;
                strncpy(buffer+buf_off, temp_token, tlen);
                buf_off += tlen;
            }
            bRet =sscanf(s_temp,"%[^{}]%[{}]%n ",temp_token,delim,&length);
            //MAPI_PRINTF("2. \ntemp_token = \"%s\"\ndelim = \"%s\"\nlength =\"%d\"\n",temp_token,delim,length);
        }
        //MAPI_PRINTF("counter =%d\n",counter);
    }while((counter !=0) ||(strcmp(delim,"{") ==0));
    free(temp_token);
    *thelist = list;
    return 0;
}

dictionary *mapi_config_helper(const char* s){
    return iniparser_load(s);
}

MI_BOOL mapi_config_helper_GetU32(dictionary *ini, const char* s, MI_U32 *pu32Num, MI_U32 notfound)
{
    if(ini != NULL)
    {
        return _GetU32(ini,s,pu32Num, notfound);
    }
    else
    {
        return FALSE;
    }
}
MI_BOOL mapi_config_helper_GetU8(dictionary *ini, const char* s, MI_U8 *u8Num)
{
    MI_U32 u32Num = 0;
    MI_BOOL bRet = FALSE;

    if(ini != NULL)
    {
        bRet = _GetU32(ini,s,&u32Num, -1);
        *u8Num = (MI_U8) u32Num;
    }

    return bRet;
}
MI_BOOL mapi_config_helper_GetBOOL(dictionary *ini, const char* s, MI_BOOL *pu8Bool)
{
    if(ini != NULL)
    {
        return _GetBOOL(ini,s,pu8Bool);
    }
    else
    {
        return FALSE;
    }
}

void u8Transformer(const char* s,MI_U8 *retString,int size)
{
    //MAPI_PRINTF("s= %s\n",s);
    const char* p = NULL;
    char n[100] ={0};
    int length;
    int index =0;
    for ( p = s; sscanf( p, "%[0-9xA-F], %n", n, &length ) == 1 && index < size; p += length, index++)
    {
        retString[index] = strtoul(n,NULL,0);
    }
    //MAPI_PRINTF("%s:\t",__func__);
    //int i;
    //for(i =0; i<size;i++)
    //{
    //    MAPI_PRINTF("%x ",retString[i]);
    //}
    //MAPI_PRINTF("\n");

}
MI_BOOL mapi_config_helper_Get1DArray(dictionary *ini, const char* s, MI_U8 * pu8Array,int size)
{
    const char **tempArrayList;
    int len;
    char* pCharStringArray = NULL;

    if(ini != NULL)
    {
        MI_BOOL bRet = _GetString(ini,s,(MI_U8**)&pCharStringArray);
        if (bRet == FALSE)
        {
            if (pCharStringArray != NULL)
            {
                free(pCharStringArray);
            }

            return FALSE;
        }

        _GetArrayItems(pCharStringArray, &tempArrayList, &len);
        free(pCharStringArray);

        if(len != 1)
        {
            return FALSE;
        }

        u8Transformer(tempArrayList[0],pu8Array,size);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

MI_BOOL mapi_config_helper_Get2DArray(dictionary *ini, const char* s, MI_U8 ** ppu8Array,int dimension1, int dimension2)
{
    const char **tempArrayList;
    int len;
    char* pCharStringArray = NULL;

    if(ini != NULL)
    {
        MI_BOOL bRet = _GetString(ini,s,(MI_U8**)&pCharStringArray);
        if (bRet == FALSE)
        {
            if (pCharStringArray != NULL)
            {
                free(pCharStringArray);
            }

            return FALSE;
        }

        _GetArrayItems(pCharStringArray, &tempArrayList, &len);
        free(pCharStringArray);

        int x  =0, i;
        for(i = 0; i < len; ++i) 
        {
            if(x >= dimension1)
            {
                break;
            }

            u8Transformer(tempArrayList[i],ppu8Array[x],dimension2);
            x++;
        }

        //MAPI_PRINTF("%s\n",__func__);
        //for(i =0;i< dimension1;i++)
        //{
        //    int j;
        //    for(j=0;j< dimension2;j++)
        //    {
        //        MAPI_PRINTF("%x ",ppu8Array[i][j]);
        //    }
        //    MAPI_PRINTF("\n");
        //}

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

MI_BOOL mapi_config_helper_GetString(dictionary *ini, const char* s, MI_U8 **ppu8String)
{
    if(ini != NULL)
    {
        return _GetString(ini,s,ppu8String);
    }
    else
    {
        return FALSE;
    }
}
