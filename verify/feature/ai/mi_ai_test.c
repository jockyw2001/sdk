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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"

#define ExecFunc(func, _ret_) \
    printf("%d Start test: %s\n", __LINE__, #func);\
    if (func != _ret_)\
    {\
        printf("AI_TEST [%d] %s exec function failed\n",__LINE__, #func);\
        return 1;\
    }\
    else\
    {\
        printf("AI_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);

//#define MI_AUDIO_SAMPLE_PER_FRAME 128
#define MI_AUDIO_SAMPLE_PER_FRAME 1024

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
typedef struct _AiChn_s {
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    pthread_t tid;
    MI_S8 szOutFilePath[128];
    MI_S32 S32StopFrameSize;
}_AiChn_t;

static _AiChn_t _astAiChn[MI_AUDIO_MAX_CHN_NUM];

static MI_S32 s32StopFrameCntsize = 64;


void* _mi_ai_GetChnPortBuf(void* data)
{
    _AiChn_t* pstAiChn = (_AiChn_t*)data;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_S32 s32Ret;
    int fdWr = -1;
    MI_U32 u32FrameLenByte;

    pstAiChn->S32StopFrameSize = 0;

    //sprintf(pstAiChn->szOutFilePath, "/mnt/ai_test_chan%d.pcm", pstAiChn->AiChn);
    sprintf(pstAiChn->szOutFilePath, "/tmp/ai_test_chan%d.pcm", pstAiChn->AiChn);
    fdWr = open(pstAiChn->szOutFilePath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);


    if( (fdWr>0) )
   {
        while(1)
        {

            s32Ret = MI_AI_GetFrame(pstAiChn->AiDevId, pstAiChn->AiChn,  &stAiChFrame,  NULL, 0);

            if(s32Ret == MI_SUCCESS)
            {
                write(fdWr, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len);
                u32FrameLenByte = stAiChFrame.u32Len;
                MI_AI_ReleaseFrame(pstAiChn->AiDevId, pstAiChn->AiChn,  &stAiChFrame,  NULL);
                pstAiChn->S32StopFrameSize++;
                if((pstAiChn->S32StopFrameSize * u32FrameLenByte) >= s32StopFrameCntsize * MI_AUDIO_SAMPLE_PER_FRAME *2)
                    break;
            }
            else
            {
                //printf("get frame fail \n");
            }
            usleep(1);
        }

   }
   else
   {
        printf("Open output file  fail: fdr %d, file path:%s \n", fdWr, pstAiChn->szOutFilePath);
   }

    if(fdWr>0)
           close(fdWr);

    return NULL;

}



static void _mi_ai_test_help()
{
    printf("\n");
    printf("Please enter: feature_ai -c xxx -n yyy -f zzz -r aaa -d bbb\n");
    printf("xxx: channel number \n");
    printf("yyy: test total frame cnt \n");
    printf("zzz : sample rate of AI device \n");
    printf("aaa: output sample rate of resample \n");
    printf("bbb: AI device ID \n");
}


int main(int argc, char *argv[])
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_AUDIO_DEV AiDevId = 0; // default set device 0
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_Frame_t stAiCh0Frame;
    //MI_AUDIO_AecFrame_t stAecFrm;
    MI_BOOL bEnableRes = FALSE;
    MI_AUDIO_SampleRate_e eAttrSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    MI_AUDIO_SampleRate_e eOutSampleRate;
    MI_BOOL bEnableVqe = FALSE;
    MI_AI_VqeConfig_t stSetVqeConfig;
    MI_AI_VqeConfig_t stGetVqeConfig;

    MI_S32 s32Opt = 0;
    MI_S32 s32Idx;
    MI_S32 s32ChanNum = 0;
    MI_U32 u32TotalFrmCnt = 0;

    MI_S32 s32RdSize;

    // parsing command line
    while ((s32Opt = getopt(argc, argv, "c:n:f:r:vd:")) != -1)
    {
            switch(s32Opt)
            {
                case 'c':
                    s32ChanNum = atoi(optarg);
                    printf("AI device Channel is %d \n", s32ChanNum);
                    if(s32ChanNum > 16 || s32ChanNum < 0 )
                        s32ChanNum = 0;
                    break;

                case 'n':
                    s32StopFrameCntsize = atoi(optarg);
                    printf("AI test frame cnt is %d \n", s32StopFrameCntsize);
                    if( s32StopFrameCntsize < 0 )
                        s32StopFrameCntsize = 64;
                    break;

                case 'f':
                    eAttrSampleRate = (MI_AUDIO_SampleRate_e)atoi(optarg);
                    printf("AI device sample rate is %d \n", eAttrSampleRate);
                    break;

                case 'r':
                    bEnableRes = TRUE;
                    eOutSampleRate =(MI_AUDIO_SampleRate_e)atoi(optarg);
                    printf("MI_AI enable Resampling, Output sample rate is %d \n", eOutSampleRate);
                    break;

                case 'v':
                    bEnableVqe = TRUE;
                    printf("MI_AI enable VQE \n");
                    break;

                case 'd':
                    AiDevId = atoi(optarg);
                    printf("Enable AI device %d !\n", AiDevId);
                    break;

                case '?':
                    if(optopt == 'c')
                        printf("Missing channel number of data, please -c 'channel number' \n");
                    else if(optopt =='n')
                        printf("Missing test frame cnt, please -n ' test frame cnt' ");
                    else if(optopt == 'f')
                        printf("Missing AI device sample rate , please -f 'eAttrSampleRate' ");
                    else if(optopt == 'r')
                        printf("Missing output sample of resample, please -r 'eOutSampleRate' \n");
                    else if(optopt == 'd')
                        printf("Missing AI device ID, please -d 'AI Device ID' \n");
                    else
                        printf("Invalid option received\n");

                default:
                    _mi_ai_test_help();
                    return 1;
            }

    }

    if( s32ChanNum == 0)
    {
        _mi_ai_test_help();
        return 1;
    }


    //MI_SYS init
    ExecFunc(MI_SYS_Init(),MI_SUCCESS);

    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eSamplerate = eAttrSampleRate;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    //stSetAttr.eWorkmode = E_MI_AUDIO_MODE_TDM_MASTER;
    stSetAttr.u32FrmNum = 16;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    // stSetAttr.u32ChnCnt = 16; //if test 16 ch TDM
    stSetAttr.u32ChnCnt = 2;


    /* set ai public attr*/
    ExecFunc(MI_AI_SetPubAttr(AiDevId, &stSetAttr), MI_SUCCESS);

    /* get ai device*/
    ExecFunc(MI_AI_GetPubAttr(AiDevId, &stGetAttr), MI_SUCCESS);

    /* enable ai device */
    ExecFunc(MI_AI_Enable(AiDevId), MI_SUCCESS);


     //set output port buffer depth ???
    MI_SYS_ChnPort_t astAiChnOutputPort0[MI_AUDIO_MAX_CHN_NUM];

    if(bEnableVqe)
    {
        memset(&stSetVqeConfig, 0, sizeof(MI_AI_VqeConfig_t));
        stSetVqeConfig.bHpfOpen = FALSE;
        stSetVqeConfig.bAnrOpen = FALSE;
        stSetVqeConfig.bAgcOpen = FALSE;
        stSetVqeConfig.bEqOpen = FALSE;

        stSetVqeConfig.s32FrameSample = 128;
        stSetVqeConfig.s32WorkSampleRate = stSetAttr.eSamplerate;

        //Hpf
        stSetVqeConfig.stHpfCfg.bUsrMode = 1;
        stSetVqeConfig.stHpfCfg.eHpfFreq = E_MI_AUDIO_HPF_FREQ_120;

        //Anr
        stSetVqeConfig.stAnrCfg.bUsrMode= 1;
        stSetVqeConfig.stAnrCfg.eNrSpeed = E_MI_AUDIO_NR_SPEED_LOW;
        stSetVqeConfig.stAnrCfg.u32NrIntensity = 10;            //[0,30]
        stSetVqeConfig.stAnrCfg.u32NrSmoothLevel = 10;

        //Agc
        stSetVqeConfig.stAgcCfg.bUsrMode = 1;
        stSetVqeConfig.stAgcCfg.s32NoiseGateDb = -50;           //[-80, 0], NoiseGateDb disable when value = -80
        stSetVqeConfig.stAgcCfg.s32TargetLevelDb =   -15;       //[-80, 0]
        stSetVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit = 1;  //[-20, 30]
        stSetVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax =  20; //[0, 30]
        stSetVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin = -10; //[-20, 30]
        stSetVqeConfig.stAgcCfg.u32AttackTime = 1;
        stSetVqeConfig.stAgcCfg.u32CompressionRatio = 1;        //[1, 10]
        stSetVqeConfig.stAgcCfg.u32DropGainMax = 60;            //[0,60]
        stSetVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb = 0;  //[0,100]
        stSetVqeConfig.stAgcCfg.u32ReleaseTime = 1;

        //Eq
        stSetVqeConfig.stEqCfg.bUsrMode = 1;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain100Hz = 5;     //[-50,20]
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain200Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain250Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain350Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain500Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain800Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain1200Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain2500Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain4000Hz = 5;
        stSetVqeConfig.stEqCfg.stEqGain.s16EqGain8000Hz = 5;

    }

    // creat thread to get frame data to save file
    for(s32Idx = 0; s32Idx < s32ChanNum; s32Idx++)
    {
        _astAiChn[s32Idx].AiDevId = AiDevId;
        _astAiChn[s32Idx].AiChn= s32Idx;

       memset(&astAiChnOutputPort0[s32Idx], 0x0, sizeof(MI_SYS_ChnPort_t));
        astAiChnOutputPort0[s32Idx].eModId = E_MI_MODULE_ID_AI;
        astAiChnOutputPort0[s32Idx].u32DevId = _astAiChn[s32Idx].AiDevId;
        astAiChnOutputPort0[s32Idx].u32ChnId = _astAiChn[s32Idx].AiChn;
        astAiChnOutputPort0[s32Idx].u32PortId = 0;
        ExecFunc(MI_SYS_SetChnOutputPortDepth(&astAiChnOutputPort0[s32Idx],9,10), MI_SUCCESS);

        /* enable ai channel of device*/
        ExecFunc(MI_AI_EnableChn(_astAiChn[s32Idx].AiDevId, _astAiChn[s32Idx].AiChn), MI_SUCCESS);

        if(bEnableRes)
        {
            ExecFunc(MI_AI_EnableReSmp(_astAiChn[s32Idx].AiDevId, _astAiChn[s32Idx].AiChn, eOutSampleRate), MI_SUCCESS);
        }

           /* if test VQe: set attribute of AO VQE  */

        if(bEnableVqe)
        {
            // need to check algorithm configure
            ExecFunc(MI_AI_SetVqeAttr(_astAiChn[s32Idx].AiDevId, _astAiChn[s32Idx].AiChn, 0, 0, &stSetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetVqeAttr(_astAiChn[s32Idx].AiDevId, _astAiChn[s32Idx].AiChn, &stGetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableVqe(_astAiChn[s32Idx].AiDevId,_astAiChn[s32Idx].AiChn), MI_SUCCESS);
        }


        printf("create thread to get channel %d data\n", _astAiChn[s32Idx].AiChn);
        pthread_create(&_astAiChn[s32Idx].tid, NULL, _mi_ai_GetChnPortBuf, &_astAiChn[s32Idx]);
    }

    // wait for getting frame data threshold return
    for(s32Idx = 0; s32Idx < s32ChanNum; s32Idx++)
    {
        pthread_join(_astAiChn[s32Idx].tid,NULL);

        /* Disable Resample */
        if(bEnableRes)
        {
            ExecFunc(MI_AI_DisableReSmp(_astAiChn[s32Idx].AiDevId,_astAiChn[s32Idx].AiChn), MI_SUCCESS);
        }

        /* Disable VQE */
        if(bEnableVqe)
        {
            ExecFunc(MI_AI_DisableVqe(_astAiChn[s32Idx].AiDevId,_astAiChn[s32Idx].AiChn), MI_SUCCESS);
        }

        /* disable ai channel of device */
        ExecFunc(MI_AI_DisableChn(AiDevId, _astAiChn[s32Idx].AiChn), MI_SUCCESS);
        //MI_SYS_SetChnOutputPortDepth(&astAiChnOutputPort0[s32Idx],0,10);
    }


    /* disable ai device */
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);



    return 0;
}
