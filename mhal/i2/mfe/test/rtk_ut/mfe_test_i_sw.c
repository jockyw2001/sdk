/****************************************************************************
 *
 *  File :   mfe_test_i_sw.c
 *
 ****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sys_MsWrapper_cus_os_mem.h"
#include "sys_sys_isw_uart.h"
#include "sys_sys_isw_cli.h"

#include "cam_os_wrapper.h"
#include "drv_mfe.h"

/*=============================================================*/
// Function Prototype
/*=============================================================*/
static int _SetMfeParameter(Mfe_param* pParam);
static int _RunMfe(Mfe_param* pParam);
static int _ShowCurrentMfeSetting(Mfe_param* pParam);
static void _DisplayMenuSetting();
static void _SetVideoBasicSettings(Mfe_param* pParam);
static void _SetH264Dblk(Mfe_param* pParam);
static void _SetH264Disposable(Mfe_param* pParam);
static void _SetH264Poc(Mfe_param* pParam);
static void _SetH264Entropy(Mfe_param* pParam);
static void _SetH264RC(Mfe_param* pParam);
static void _SetH264IntraP(Mfe_param* pParam);
static void _SetH264InterP(Mfe_param* pParam);
static void _SetH264SS(Mfe_param* pParam);
static void _SetH264ROI(Mfe_param* pParam);
static void _SetH264SEI(Mfe_param* pParam);
static void _SetH264NewSeq(Mfe_param* pParam);
static void _SetVideoClkSettings(Mfe_param* pParam);
static void _DisplayMfeSetting();

static void _GenSEINALU(Mfe_param* p,int idx,char* value);
static int _Stringlen(char* src);
static int _CpString(char* dst, char* src);

extern void MfeEnc(Mfe_param* pParam);

/*=============================================================*/
// Function Declaration
/*=============================================================*/

int MfeEncodeFlow(CLI_t * pCli, char * p)
{
    int num;
    Mfe_param* pParam = (Mfe_param*)CamOsMemCalloc(1, sizeof(Mfe_param));

    _DisplayMenuSetting();
    do
    {
        UartSendTrace("==>");
        num = vm_atoi(wait_for_command());
        switch(num)
        {
            case 1:
                _SetMfeParameter(pParam);
                _DisplayMenuSetting();
                break;
            case 2:
                _RunMfe(pParam);
                _DisplayMenuSetting();
                break;
            case 3:
                _ShowCurrentMfeSetting(pParam);
                _DisplayMenuSetting();
                break;
            case 4:
                _DisplayMenuSetting();
                break;
            default:
                break;
        }
    }
    while( 0 != num );

    CamOsMemRelease(pParam);
    return 0;
}

static int _SetMfeParameter(Mfe_param* pParam)
{
    int num;
    _DisplayMfeSetting();
    do
    {
        UartSendTrace(">>");
        num = vm_atoi(wait_for_command());
        switch(num)
        {
            case 0:
                break;
            case 1:
                _SetVideoBasicSettings(pParam);
                break;
            case 2:
                _SetH264Dblk(pParam);
                break;
            case 3:
                _SetH264Disposable(pParam);
                break;
            case 4:
                _SetH264Poc(pParam);
                break;
            case 5:
                _SetH264Entropy(pParam);
                break;
            case 6:
                _SetH264RC(pParam);
                break;
            case 7:
                _SetH264IntraP(pParam);
                break;
            case 8:
                _SetH264InterP(pParam);
                break;
            case 9:
                _SetH264SS(pParam);
                break;
            case 10:
                _SetH264ROI(pParam);
                break;
            case 11:
                _SetH264SEI(pParam);
                break;
            case 12:
                _SetH264NewSeq(pParam);
                break;
            case 13:
                _SetVideoClkSettings(pParam);
            default:
                break;
        }
        _DisplayMfeSetting();
    }
    while( 0 != num );
    return 0;
}

static int _RunMfe(Mfe_param* pParam)
{
    MfeEnc(pParam);
    return 0;
}

static int _ShowCurrentMfeSetting(Mfe_param* pParam)
{
    int i=0;

    if( pParam->sBasic.bEn )
        UartSendTrace("-Video Resolution: %lu x %lu\n",pParam->sBasic.nWidth, pParam->sBasic.nHeight);
    UartSendTrace("-Fps: %lu Btrates: %lu \n",pParam->sBasic.nFps, pParam->sBasic.nBitrate);
    UartSendTrace("-Gop: %lu Qp: %lu\n",pParam->sBasic.nGop, pParam->sBasic.bEn);

    if( pParam->sDeblk.bEn )
    {
        UartSendTrace("-deblock_filter_control: %d\n",pParam->sDeblk.nDeblockFilterControl);
        UartSendTrace("-disable_deblocking_filter_idc: %lu\n",pParam->sDeblk.nDisableDeblockingFilterIdc);
        UartSendTrace("-slice_alpha_c0_offset_div2: %lu\n",pParam->sDeblk.nSliceAlphaOffsetDiv2);
        UartSendTrace("-slice_beta_offset_div2: %lu\n",pParam->sDeblk.nSliceBetaOffsetDiv2);
    }

    if( pParam->sDisposable.bEn )
        UartSendTrace("-Disposable Mode: %d\n",pParam->sDisposable.bDisposable);

    if( pParam->sPoc.bEn )
        UartSendTrace("-POC Type: %lu\n",pParam->sPoc.nPocType);

    if( pParam->sEntropy.bEn )
        UartSendTrace("-Cabac: %d\n",pParam->sEntropy.bEntropyCodingCABAC);

    if( pParam->sRC.bEn )
    {
        UartSendTrace("-ControlRate: %d\n",pParam->sRC.eControlRate);
    }

    if( pParam->sIntraP.bEn )
        UartSendTrace("-constrained_intra_pred_flag: %d\n",pParam->sIntraP.bConstIpred);

    if( pParam->sInterP.bEn )
    {
        UartSendTrace("-nDmv_X: %lu nDmv_Y: %lu\n",pParam->sInterP.nDmv_X,pParam->sInterP.nDmv_Y);
        UartSendTrace("-nSubpel: %lu \n",pParam->sInterP.nSubpel);
        UartSendTrace("-bInter4x4PredEn:%d  bInter4x8PredEn:%d \n",pParam->sInterP.bInter4x4PredEn,pParam->sInterP.bInter4x8PredEn);
        UartSendTrace("-bInter8x4PredEn:%d  bInter8x8PredEn:%d \n",pParam->sInterP.bInter8x4PredEn,pParam->sInterP.bInter8x8PredEn);
        UartSendTrace("-bInter8x16PredEn:%d  bInter16x8PredEn:%d \n",pParam->sInterP.bInter8x16PredEn,pParam->sInterP.bInter16x8PredEn);
        UartSendTrace("-bInter16x16PredEn:%d  bInterSKIPPredEn:%d \n",pParam->sInterP.bInter16x16PredEn,pParam->sInterP.bInterSKIPPredEn);
    }

    if( pParam->sMSlice.bEn )
    {
        UartSendTrace("-nRows: %lu\n",pParam->sMSlice.nRows);
        UartSendTrace("-nBits: %lu\n",pParam->sMSlice.nBits);
    }

    for( i=0; i<MAX_ROI_NUM; i++)
    {
        if(pParam->sRoi[i].bEn)
        {
            UartSendTrace("-Idx: %lu Enable: %d (X,Y)->(%lu,%lu) (W,H)->(%lu,%lu) Qp:%lu\n",pParam->sRoi[i].nIdx,pParam->sRoi[i].bEnable,
                       pParam->sRoi[i].nMbX,pParam->sRoi[i].nMbY,pParam->sRoi[i].nMbW,pParam->sRoi[i].nMbH,pParam->sRoi[i].nDqp);
        }
    }

    for( i=0; i<MAX_SEI_NUM; i++)
    {
        if( pParam->sSEI[i].bEn )
        {
            UartSendTrace("-User Data%d: %s\n",i,&pParam->sSEI[i].U8data[23]);
        }
    }

    if( pParam->sNewSeq.bEn )
    {
        UartSendTrace("-NewSeqW: %lu\n",pParam->sNewSeq.nWidth);
        UartSendTrace("-NewSeqH: %lu\n",pParam->sNewSeq.nHeight);
        UartSendTrace("-NewSeqFps: %lu\n",pParam->sNewSeq.xFramerate);
    }

    if( pParam->sClock.bEn )
    {
        UartSendTrace("-Clock Disable: %lu\n",pParam->sClock.nClkEn);
        UartSendTrace("-Clock Source: %lu\n",pParam->sClock.nClkSor);
    }

    return 0;
}

static void _DisplayMenuSetting()
{
    UartSendTrace("===============================\n");
    UartSendTrace("==            Menu           ==\n");
    UartSendTrace("===============================\n");
    UartSendTrace("=[1] Set video enc parameters\n");
    UartSendTrace("=[2] Execute\n");
    UartSendTrace("=[3] Print the current video enc settings\n");
    UartSendTrace("=[4] Help\n");
    UartSendTrace("=[0] Quit\n");
    UartSendTrace("===============================\n");
}

static void _SetVideoBasicSettings(Mfe_param* pParam)
{
    UartSendTrace("> frame width: ");
    pParam->sBasic.nWidth = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nWidth);

    UartSendTrace("> frame height: ");
    pParam->sBasic.nHeight = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nHeight);

    UartSendTrace("> frame rate: ");
    pParam->sBasic.nFps = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nFps);

    UartSendTrace("> bitrate: ");
    pParam->sBasic.nBitrate = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nBitrate);

    UartSendTrace("> Gop: ");
    pParam->sBasic.nGop = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nGop);

    UartSendTrace("> Qp: ");
    pParam->sBasic.nQp = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sBasic.nQp);

    pParam->sBasic.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264Dblk(Mfe_param* pParam)
{
    UartSendTrace("> deblock_filter_control(0/1): ");
    pParam->sDeblk.nDeblockFilterControl = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sDeblk.nDeblockFilterControl);

    UartSendTrace("> disable_deblocking_filter_idc(0/1/2): ");
    pParam->sDeblk.nDisableDeblockingFilterIdc = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sDeblk.nDisableDeblockingFilterIdc);

    UartSendTrace("> slice_alpha_c0_offset_div2(-6~6): ");
    pParam->sDeblk.nSliceAlphaOffsetDiv2 = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sDeblk.nSliceAlphaOffsetDiv2);

    UartSendTrace("> slice_beta_offset_div2(-6~6): ");
    pParam->sDeblk.nSliceBetaOffsetDiv2 = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sDeblk.nSliceBetaOffsetDiv2);

    pParam->sDeblk.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264Disposable(Mfe_param* pParam)
{
    UartSendTrace("> Disposable Mode(0/1): ");
    pParam->sDisposable.bDisposable = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sDisposable.bDisposable);

    pParam->sDisposable.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264Poc(Mfe_param* pParam)
{
    UartSendTrace("> Poc type(0/2): ");
    pParam->sPoc.nPocType = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sPoc.nPocType);

    pParam->sPoc.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264Entropy(Mfe_param* pParam)
{
    UartSendTrace("> enable cabac (0/1): ");
    pParam->sEntropy.bEntropyCodingCABAC = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sEntropy.bEntropyCodingCABAC);

    pParam->sEntropy.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264RC(Mfe_param* pParam)
{
    UartSendTrace("> ControlRate ( 0:mrqc (fixQP) 1:msb2 ):");
    pParam->sRC.eControlRate= vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRC.eControlRate);

    pParam->sRC.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264IntraP(Mfe_param* pParam)
{
    UartSendTrace("> constrained_intra_pred_flag (0/1): ");
    pParam->sIntraP.bConstIpred = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sIntraP.bConstIpred);

    pParam->sIntraP.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264InterP(Mfe_param* pParam)
{
    UartSendTrace("> nDmv_X (8~16): ");
    pParam->sInterP.nDmv_X = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.nDmv_X);

    UartSendTrace("> nDmv_Y (8/16): ");
    pParam->sInterP.nDmv_Y = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.nDmv_Y);

    UartSendTrace("> nSubpel (0:Pixel 1:HalfPel 2:QuarterPel ): ");
    pParam->sInterP.nSubpel = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.nSubpel);

    UartSendTrace("> bInter4x4PredEn (0/1): ");
    pParam->sInterP.bInter4x4PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter4x4PredEn);

    UartSendTrace("> bInter8x4PredEn (0/1): ");
    pParam->sInterP.bInter8x4PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter8x4PredEn);

    UartSendTrace("> bInter4x8PredEn (0/1): ");
    pParam->sInterP.bInter4x8PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter4x8PredEn);

    UartSendTrace("> bInter8x8PredEn (0/1): ");
    pParam->sInterP.bInter8x8PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter8x8PredEn);

    UartSendTrace("> bInter16x8PredEn (0/1): ");
    pParam->sInterP.bInter16x8PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter16x8PredEn);

    UartSendTrace("> bInter8x16PredEn (0/1): ");
    pParam->sInterP.bInter8x16PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter8x16PredEn);

    UartSendTrace("> bInter16x16PredEn (0/1): ");
    pParam->sInterP.bInter16x16PredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInter16x16PredEn);

    UartSendTrace("> bInterSKIPPredEn (0/1): ");
    pParam->sInterP.bInterSKIPPredEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sInterP.bInterSKIPPredEn);

    pParam->sInterP.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264SS(Mfe_param* pParam)
{
    UartSendTrace("> nRows (0/1/2/4/8): ");
    pParam->sMSlice.nRows = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sMSlice.nRows);

    UartSendTrace("> nBits (0 ~ (2^16 -1)): ");
    pParam->sMSlice.nBits = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sMSlice.nBits);

    pParam->sMSlice.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264ROI(Mfe_param* pParam)
{
    int index;
    UartSendTrace("> ROI Index (0~7): ");
    index = vm_atoi(wait_for_command());
    if( index >= MAX_ROI_NUM ) return;
    pParam->sRoi[index].nIdx = index;
    UartSendTrace("\n %d \n", pParam->sRoi[index].nIdx);

    UartSendTrace("> ROI Enable (0/1): ");
    pParam->sRoi[index].bEnable = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].bEnable);

    UartSendTrace("> MbX : ");
    pParam->sRoi[index].nMbX = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].nMbX);

    UartSendTrace("> MbY : ");
    pParam->sRoi[index].nMbY = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].nMbY);

    UartSendTrace("> MbW : ");
    pParam->sRoi[index].nMbW = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].nMbW);

    UartSendTrace("> MbH : ");
    pParam->sRoi[index].nMbH = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].nMbH);

    UartSendTrace("> Qp (0~15): ");
    pParam->sRoi[index].nDqp = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sRoi[index].nDqp);

    pParam->sRoi[index].bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetH264SEI(Mfe_param* pParam)
{
    char pstr[1024];
    int a = 0;
    int i = 0;
    memset(pstr,0,1024);
    for( i=0; i< MAX_SEI_NUM; i++)
    {
        UartSendTrace("> User Data%d enable (0/1) : ",i);
        a = vm_atoi(wait_for_command());
        UartSendTrace("\n %d \n", a);
        if (a==1)
        {
            UartSendTrace("> User Data%d:",i);
            char* data = wait_for_command();
            int j = 0;
            while(*data != '\0')
            {
                pstr[j] = *data;
                data++;
                j++;
            }
            _GenSEINALU(pParam,i,pstr);
            pParam->sSEI[i].bEn = TRUE;
        }
        else
        {
            pParam->sSEI[i].bEn = FALSE;
        }
    }

    UartSendTrace("\n Done!\n");
}

static void _SetH264NewSeq(Mfe_param* pParam)
{
    UartSendTrace("> NewSeq Width:");
    pParam->sNewSeq.nWidth = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sNewSeq.nWidth);

    UartSendTrace("> NewSeq Height:");
    pParam->sNewSeq.nHeight = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sNewSeq.nHeight);

    UartSendTrace("> NewSeq Fps:");
    pParam->sNewSeq.xFramerate = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sNewSeq.xFramerate);

    pParam->sNewSeq.bEn = TRUE;
    UartSendTrace("Done!\n");
}

static void _SetVideoClkSettings(Mfe_param* pParam)
{
    UartSendTrace("> Clock Disable: (0:Enable, 1:Disable)");
    pParam->sClock.nClkEn = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sClock.nClkEn);

    UartSendTrace("> Clock Source:\n");
    UartSendTrace(" 0 : 288 Mhz \n");
    UartSendTrace(" 1 : 240 Mhz \n");
    UartSendTrace(" 2 : 192 Mhz \n");
    UartSendTrace(" 3 : 123 Mhz \n");
    UartSendTrace(" 4 : 384 Mhz \n");
    UartSendTrace(" 5 : 320 Mhz \n");
    UartSendTrace(" 6 : 172 Mhz \n");
    pParam->sClock.nClkSor = vm_atoi(wait_for_command());
    UartSendTrace("\n %d \n", pParam->sClock.nClkSor);

    pParam->sClock.bEn = TRUE;
    UartSendTrace("Done!\n");
}
static void _DisplayMfeSetting()
{
    UartSendTrace("please select item to set mfe..\n");
    UartSendTrace("===============================\n");
    UartSendTrace("[1] set video encoder basic settings\n");
    UartSendTrace("[2] set h264 deblocking\n");
    UartSendTrace("[3] set h264 disposable mode\n");
    UartSendTrace("[4] set h264 poc type\n");
    UartSendTrace("[5] set h264 entropy\n");
    UartSendTrace("[6] set h264 rate control\n");
    UartSendTrace("[7] set h264 intra prediction\n");
    UartSendTrace("[8] set h264 inter prediction\n");
    UartSendTrace("[9] set h264 slice split\n");
    UartSendTrace("[10] set h264 roi\n");
    UartSendTrace("[11] set h264 sei\n");
    UartSendTrace("[12] set h264 new sequence\n");
    UartSendTrace("[13] set clock settings\n");
    UartSendTrace("[0] exit\n");
    UartSendTrace("===============================\n");
}

static void _GenSEINALU(Mfe_param* p,int idx,char* value)
{
    int i=0;
    int data_len = _Stringlen(value);
    int total_len = data_len + 16;
    int ren_len = total_len;
    int offset=0;

    p->sSEI[idx].U8data[0] = p->sSEI[idx].U8data[1] = p->sSEI[idx].U8data[2] = 0x00;
    p->sSEI[idx].U8data[3] = 0x01;
    p->sSEI[idx].U8data[4] = 0x06;
    p->sSEI[idx].U8data[5] = 0x05; //user_data_unregistered type

    offset=6;
    while( ren_len >=255 )
    {
        p->sSEI[idx].U8data[offset] = 0xFF;
        ren_len -=255;
        offset++;
    }
    p->sSEI[idx].U8data[offset++] = ren_len;

    for( i=offset; i<=offset+15; i++)
    {
        if( idx ==0 )
            p->sSEI[idx].U8data[i] = 0xAA; //uuid_iso_iec_11578 , 16 bytes
        else if( idx ==1 )
            p->sSEI[idx].U8data[i] = 0x55; //uuid_iso_iec_11578 , 16 bytes
        else if( idx ==2 )
            p->sSEI[idx].U8data[i] = 0x66; //uuid_iso_iec_11578 , 16 bytes
        else if( idx ==3 )
            p->sSEI[idx].U8data[i] = 0x77; //uuid_iso_iec_11578 , 16 bytes
    }
    offset+=16;

    _CpString((char*)&p->sSEI[idx].U8data[offset],value);//user_data_payload_byte
    /*
         for( i=offset; i<=data_len+offset-1; i++){
               p->sSEI[idx].U8data[i]= 0xBB;//user_data_payload_byte
         }
    */
    offset+= data_len;

    p->sSEI[idx].U8data[offset++] = 0x80;//rbsp_trailing_bits
    p->sSEI[idx].U32Len = offset; //total sei size
    p->sSEI[idx].U8data[offset] ='\0';
}

static int _Stringlen(char* src)
{
    int i=0,len=0;
    for(i=0; i<strlen(src); i++)
    {
        if(*(src+i) == '\n' || *(src+i) == '\r' )
            *(src+i) ='\0';
    }
    len = strlen(src);
    return len;
}

static int _CpString(char* dst, char* src)
{
    int i=0,len=0;
    for(i=0; i<strlen(src); i++)
    {
        if(*(src+i) == '\n' || *(src+i) == '\r' )
            *(src+i) ='\0';
    }
    len = strlen(src);
    strncpy(dst,src,len);

    return len;
}