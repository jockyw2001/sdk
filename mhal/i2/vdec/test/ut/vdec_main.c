#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <MsTypes.h>
#include <mdrv_vdec_io.h>
#include <mdrv_vdec_io_st.h>
#include <apiVDEC.h>
#include "cam_os_wrapper.h"
//#include "md5.h"
#include "vdec_test.h"


//=============================================================================
#define H264_1080P_ES       "1080p30_h264_1.es"
#define H265_1080P_ES       "1080p30_h265_1.es"
#define H265_720P_ES        "d130p_h265_1.es"
#define H265_D1_ES          "720p30_h265_1.es"
#define MJPEG_ES             "jpeg_chn_0.es"

//=============================================================================
//
//=============================================================================
static void display_help()
{
     printf("\n");
     printf("Usage: vdec_test [-h] [-n numChannel] [-d dbgLevel] [-c codectype1 codectype2 ...]\n"
            "                 [-f infile1 inpfile2...] [-b bufSuzeCh1 ..] [-s] [-m] [-a] [-v md5_mode]\n"
            "                 [-r frameCntPerRun] [-p framCntDump]  [-i id] [-g] [-l vdecDufsize]\n"
            "                 [-t statisticsSec] [-u UTdbgLevel]\n");
     printf("h: display help\n");
     printf("d: drv/fw dbglevel: 0=none 1=error 2=info 3=dbg 4=trace 5=fw(default: 1)\n");
     printf("n: number of channels(default: 1)\n");
     printf("c: ScodecType: h264 or h265 or jpg(default: h264)\n");
     printf("s: Single frame mode, force to one channel(default: enabled)\n");
     printf("m: enable MFCodec(default: disabled)\n");
     printf("b: buffer size(format 0x10000; default: HVD=0xE00000, EVD=0x1800000)\n");
     printf("a: auto mode(no wait for key to stop; default: disabled)\n");
     printf("v: md5 mode: 0=byFrameVerify 2=genMd5 3=no_Verify  (default: 3)\n");
     printf("r: number of frames per run\n");
     printf("p: number of frames to dump\n");
     printf("i: id_num of this run; designed for being called by script to performance multiple runs\n");
     printf("g: go on decoding frames on MD5 failure\n");
     printf("l: total vdec buffer size to allocate(bs buf + frame buf\n");
     printf("t: performance test mode(only support single stream now)\n");
     printf("u: UT dbglevel: 0=none 1=error 2=info 3=dbg(default: 0)\n");
     printf("\n");
}

#if _CUNIT_TEST
int vdec_test_main(int argc, char * argv[])
#else
int main(int argc, char * argv[])
#endif
{
    int argn_dec;
    int i = 0;
    int ret;
    VDEC_TEST_CFG tTestCfg;

    memset(&tTestCfg, 0, sizeof(VDEC_TEST_CFG));
    tTestCfg.nSingleFrameMode = FALSE;
    tTestCfg.u8NumOfVdecChannel = 1;
    tTestCfg.nEnableMFCodec = FALSE;
    tTestCfg.eDbgLevel = E_VDEC_EX_DBG_LEVEL_ERR;
    tTestCfg.eMd5Mode = E_VDEC_MD5_NOT_VERIFY;

    argn_dec = 1;
    while(argn_dec < argc)
    {
        if (*(argv[argn_dec]) != '-')
        {
            printf("[Error]%d %d: %c !!\n", argc, argn_dec, argv[argn_dec]);
            display_help();
            return -1;
        }

        //printf("argv[]=%s\n", argv[argn_dec]+1);
        if (strlen(argv[argn_dec]+1) > 1)
        {
            if (strcmp((argv[argn_dec]+1), "cs") == 0)
            {
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                if( strcmp(argv[argn_dec], "h264")==0 )
                {
                    for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                        tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_H264;
                    printf("Channels : h264!!\n");
                }
                else if( strcmp(argv[argn_dec], "h265")==0 )
                {
                    for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                       tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
                    printf("Channels: h265!!\n");
                }
                else if( strcmp(argv[argn_dec], "jpg")==0 )
                {
                     for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                         tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_MJPEG;
                     printf("Channel %d: mjpeg!!\n", i);
                }
                else
                {
                     printf("[Error] wrong CodecType!!\n");
                     return -1;
                }
            }
            else if (strcmp((argv[argn_dec]+1), "fs") == 0)
            {
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                {
                    sprintf(tTestCfg.tChannelCfg[i].fileName, "%s", argv[argn_dec]);
                }
                printf("Channels: file %s !!\n", argv[argn_dec]);
            }
            else if (strcmp((argv[argn_dec]+1), "bs") == 0)
            {
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                {
                     tTestCfg.tChannelCfg[i].u32BufSize = (int)strtol(argv[argn_dec], NULL, 0);
                }
                printf("Channel %d: buffer size 0x%x !!\n", i, tTestCfg.tChannelCfg[i].u32BufSize);
            }
            else  if (strcmp((argv[argn_dec]+1), "bb") == 0)
            {
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u32TotalVdecBufSize = atoi(argv[argn_dec]);
                printf("u32TotalVdecBufSize= %d!!\n", tTestCfg.u32TotalVdecBufSize);
             }

        }
        else
        {
        switch (*(argv[argn_dec]+1))
        {
            case 'h':
                display_help();
                return 0;

            case 's':
                 tTestCfg.nSingleFrameMode = 1;
                 break;

            case 't':
                 argn_dec++;
                 if (argc <= argn_dec)
                 {
                      display_help();
                      return -1;
                 }
                 tTestCfg.nPerfTestMode = atoi(argv[argn_dec]);
                 if (tTestCfg.nPerfTestMode == 0) /* force to min 1 sec */
                 {
                     tTestCfg.nPerfTestMode = 1;
                 }
                 break;

            case 'm':
                 tTestCfg.nEnableMFCodec = TRUE;
                 break;

            case 'a':
                 tTestCfg.u32AutoMode = TRUE;
                 break;

            case 'g':
                 tTestCfg.u32Md5FailGo = TRUE;
                 break;

            case 'n':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u8NumOfVdecChannel = atoi(argv[argn_dec]);
                printf("Channel num= %d!!\n", tTestCfg.u8NumOfVdecChannel);
                break;

            case 'v':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.eMd5Mode = atoi(argv[argn_dec]);
                printf("MD5 mode= %d!!\n", tTestCfg.eMd5Mode);
                break;

            case 'r':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u32MaxFrameNum = atoi(argv[argn_dec]);
                printf("u32MaxFrameNum= %d!!\n", tTestCfg.u32MaxFrameNum);
                break;

            case 'p':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u32DumpFrameNum = atoi(argv[argn_dec]);
                printf("u32DumpFrameNum= %d!!\n", tTestCfg.u32DumpFrameNum);
                break;

            case 'd' :
                argn_dec++;
                if (argc <= argn_dec)
                 {
                     display_help();
                     exit(-1);
                 }
                tTestCfg.eDbgLevel = atoi(argv[argn_dec]);
                if ((tTestCfg.eDbgLevel < 0) || (tTestCfg.eDbgLevel > 5))
                {
                    printf("eDbgLevel= %d!!\n", tTestCfg.eDbgLevel);
                    display_help();
                    return -1;
                }
                break;

            case 'c' :
                 if ((argc - argn_dec-1) < tTestCfg.u8NumOfVdecChannel)
                 {
                     printf("[Error] Wrong num of codec type description !!\n");
                     return -1;
                 }
                 for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                 {
                      argn_dec++;
                      if( strcmp(argv[argn_dec], "h264")==0 )
                      {
                          tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_H264;
                          printf("Channel %d: h264!!\n", i);
                      }
                      else if( strcmp(argv[argn_dec], "h265")==0 )
                      {
                          tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_HEVC;
                          printf("Channel %d: h265!!\n", i);
                      }
                      else if( strcmp(argv[argn_dec], "jpg")==0 )
                      {
                          tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_MJPEG;
                          printf("Channel %d: mjpeg!!\n", i);
                      }
                      else
                      {
                          printf("[Error] wrong CodecType!!\n");
                          return -1;
                      }
                 }
                 break;

            case 'f' :
                 if ((argc - argn_dec-1) < tTestCfg.u8NumOfVdecChannel)
                 {
                      printf("[Error] Wrong num of input files !!\n");
                      return -1;
                 }

                 for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                 {
                      argn_dec++;
                      sprintf(tTestCfg.tChannelCfg[i].fileName, "%s", argv[argn_dec]);
                      printf("Channel %d: file %s !!\n", i, tTestCfg.tChannelCfg[i].fileName);
                 }
                 break;

            case 'b' :
                  if ((argc - argn_dec-1) < tTestCfg.u8NumOfVdecChannel)
                  {
                       printf("[Error] Wrong num of buffer sizes !!\n");
                       return -1;
                  }

                  for (i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
                  {
                       argn_dec++;
                       tTestCfg.tChannelCfg[i].u32BufSize = (int)strtol(argv[argn_dec], NULL, 0);
                       printf("Channel %d: buffer size 0x%x !!\n", i, tTestCfg.tChannelCfg[i].u32BufSize);
                  }
                  break;

            case 'i':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u32IdNum = atoi(argv[argn_dec]);
                printf("id num= %d!!\n", tTestCfg.u32IdNum);
                break;

            case 'l':
                argn_dec++;
                if (argc <= argn_dec)
                {
                    display_help();
                    return -1;
                }
                tTestCfg.u32VdecBufSize = (int)strtol(argv[argn_dec], NULL, 0);
                printf("Vdec buf size= 0x%x!!\n", tTestCfg.u32VdecBufSize);
                break;

            case 'u' :
                argn_dec++;
                if (argc <= argn_dec)
                {
                     display_help();
                     exit(-1);
                }
                tTestCfg.eUtDbgLevel = atoi(argv[argn_dec]);
                if ((tTestCfg.eUtDbgLevel < 0) || (tTestCfg.eUtDbgLevel > 3))
                {
                    printf("illegal eUtDbgLevel: %d!!\n", tTestCfg.eUtDbgLevel);
                    display_help();
                    return -1;
                }
                break;

            default:
                 //printf("argv[]=%s\n", argv[argn_dec]+1);
                 break;

            }  /* switch */
        }  /*else */

        // next argument
        argn_dec++;
    } /* while */

    if ((tTestCfg.nSingleFrameMode) && (tTestCfg.u8NumOfVdecChannel > 1))
    {
        printf("Single frame mode is set--> Force to one channel!!\n");
        tTestCfg.u8NumOfVdecChannel = 1;
    }

    /* set default filenames if there is no corresponding options */
    for(i = 0; i < tTestCfg.u8NumOfVdecChannel; i++)
    {
        if ((tTestCfg.tChannelCfg[i].eCodecType != E_VDEC_EX_CODEC_TYPE_H264) && (tTestCfg.tChannelCfg[i].eCodecType != E_VDEC_EX_CODEC_TYPE_HEVC) &&
            (tTestCfg.tChannelCfg[i].eCodecType != E_VDEC_EX_CODEC_TYPE_MJPEG))
        {
            tTestCfg.tChannelCfg[i].eCodecType = E_VDEC_EX_CODEC_TYPE_H264;
        }

        if (strlen(tTestCfg.tChannelCfg[i].fileName) == 0)
        {
            if (tTestCfg.tChannelCfg[i].eCodecType == E_VDEC_EX_CODEC_TYPE_H264)
            {
                sprintf(tTestCfg.tChannelCfg[i].fileName, "%s", H264_1080P_ES);
            }
            else if (tTestCfg.tChannelCfg[i].eCodecType == E_VDEC_EX_CODEC_TYPE_HEVC)
            {
                sprintf(tTestCfg.tChannelCfg[i].fileName, "%s", H265_1080P_ES);
            }
            else if (tTestCfg.tChannelCfg[i].eCodecType == E_VDEC_EX_CODEC_TYPE_MJPEG)
            {
                sprintf(tTestCfg.tChannelCfg[i].fileName, "%s", MJPEG_ES);
            }
        }
    }


    if (!Vdec_Test_Init(&tTestCfg))
    {
        return -1;
    }

    ret = Vdec_Test_Run();

    Vdec_Test_Exit();
    exit((ret) ? 0 : -1);
}

//========================================================================================
#if _CUNIT_TEST
#define MAX_LEN_CMDLINE         512
#define MAX_NUM_ARGV            10
#define VDEC_TEST(s)     { strncpy(cmdLine, s, MAX_LEN_CMDLINE); string_to_argc_argv(); \
                            if (vdec_test_main(argc, argv) < 0) printf("%s--> verfiy failed\n", cmdLine); \
                          }

int argc = 0;
char *argv[MAX_NUM_ARGV] = { 0 };
char cmdLine[MAX_LEN_CMDLINE];

void  string_to_argc_argv()
{
    int i = 0;

    printf("cmdLine=%s\n", cmdLine);
    argv[i] = strtok(cmdLine, " ");
    while ((i < MAX_NUM_ARGV-1) && (argv[i] != NULL))
    {
        printf("argv[%d]: %s\n", i, argv[i]);
        argv[++i] = strtok(NULL, " ");
    }
    argc = i;
}


int main(void)
{
    VDEC_TEST("vdec_test -c h264 -f 1080p30_h264.es");
    VDEC_TEST("vdec_test -c h265 -f 1080p30_h265.es");
    VDEC_TEST("vdec_test -c h264 -m -f 1080p30_h264.es");
    VDEC_TEST("vdec_test -c h265 -m -f 1080p30_h265.es");
    VDEC_TEST("vdec_test -n 2 -c h264 h265 -f 1080p30_h264.es 1080p30_h265.es");
    VDEC_TEST("vdec_test -n 2 -c h265 h264 -f 1080p30_h265.es 1080p30_h264.es");
}
#endif
