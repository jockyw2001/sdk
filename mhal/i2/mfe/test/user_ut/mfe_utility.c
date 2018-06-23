#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "mfe_utility.h"


void DisplayMenuSetting(void){
       printf("===============================\n");
       printf("==            Menu           ==\n");
       printf("===============================\n");
       printf("=[f] Set input/output path\n");
       printf("=[s] Set video enc parameters\n");
       printf("=[p] Print the current video enc settings\n");
       printf("=[r] Reset video enc parameters\n");
       printf("=[e] Execute\n");
       printf("=[h] Help\n");
       printf("=[q] Quit\n");
       printf("===============================\n");
}

void DisplayMFESetting(void){
       printf("please select item to set omx_mfe_I1..\n");
       printf("===============================\n");
       printf("[1] set video encoder basic settings\n");
       printf("[2] set h264 deblocking\n");
       printf("[3] set h264 disposable mode\n");
       printf("[4] set h264 poc type\n");
       printf("[5] set h264 entropy\n");
       printf("[6] set h264 rate control\n");
       printf("[7] set h264 intra prediction\n");
       printf("[8] set h264 inter prediction\n");
       printf("[9] set h264 slice split\n");
       printf("[10] set h264 roi\n");
       printf("[11] insert user data\n");
       printf("[12] set h264 New Sequence\n");
       printf("[13] set h264 clock setting\n");
       printf("[14] set h264 VUI\n");
       printf("[15] set h264 LTR\n");
       printf("[0] exit\n");
       printf("===============================\n");
}

int cpString(char* dst, char* src){
       int i=0,len=0;
       for(i=0;i<strlen(src);i++){
             if(*(src+i) == '\n' || *(src+i) == '\r' )
                         *(src+i) ='\0';
       }
       len = strlen(src);
       strncpy(dst,src,len);

       return len;
}

int stringlen(char* src){
       int i=0,len=0;
       for(i=0;i<strlen(src);i++){
             if(*(src+i) == '\n' || *(src+i) == '\r' )
                         *(src+i) ='\0';
       }
       len = strlen(src);
       return len;
}

 int cmpRoiString(char* name, int IdxRoi, char* lastStr){
     char tmp[20];
     memset(tmp,0,20);
     sprintf(tmp,"ROI%d_%s",IdxRoi,lastStr);
     return !strncmp(name,tmp,strlen(tmp));
 }

void getROISettings(int idx, char* name,char* value,Mfe_param* p){
     int i = idx;
     if( cmpRoiString(name, i, "Idx") ){
          p->sRoi[i].nIdx = atoi(value);
     }
     else if( cmpRoiString(name, i, "Enable") ){
          p->sRoi[i].bEnable= atoi(value);
     }
     else if( cmpRoiString(name, i, "MbX") ){
          p->sRoi[i].nMbX = atoi(value);
     }
     else if( cmpRoiString(name, i, "MbY") ){
          p->sRoi[i].nMbY = atoi(value);
     }
     else if( cmpRoiString(name, i, "MbW")  ){
          p->sRoi[i].nMbW = atoi(value);
     }
     else if( cmpRoiString(name, i, "MbH")){
          p->sRoi[i].nMbH = atoi(value);
     }
     else if(cmpRoiString(name, i, "Qp"))
     {
         p->sRoi[i].nMbqp = atoi(value);
     }
     else if(cmpRoiString(name, i, "AbsQp"))
     {
         p->sRoi[i].bAbsQp = atoi(value);
     }
}

void GenSEINALU(Mfe_param* p,int idx,char* value)
{
     int i=0;
     int data_len = stringlen(value);//888;
     int total_len = data_len + 16;
     int ren_len = total_len;
     int offset=0;

     p->sSEI[idx].U8data[0] = p->sSEI[idx].U8data[1] = p->sSEI[idx].U8data[2] = 0x00;
     p->sSEI[idx].U8data[3] = 0x01;
     p->sSEI[idx].U8data[4] = 0x06;
     p->sSEI[idx].U8data[5] = 0x05; //user_data_unregistered type

     offset=6;
     while( ren_len >=255 ){
                p->sSEI[idx].U8data[offset] = 0xFF;
                ren_len -=255;
                offset++;
     }
     p->sSEI[idx].U8data[offset++] = ren_len;

     for( i=offset; i<=offset+15; i++){
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

     cpString((char*)&p->sSEI[idx].U8data[offset],value);//user_data_payload_byte
/*
     for( i=offset; i<=data_len+offset-1; i++){
           p->sSEI[idx].U8data[i]= 0xBB;//user_data_payload_byte
     }
*/
     offset+= data_len;

     p->sSEI[idx].U8data[offset++] = 0x80;//rbsp_trailing_bits
     p->sSEI[idx].u32Len = offset; //total sei size
     p->sSEI[idx].U8data[offset] ='\0';
}

int ReadDefaultConfig(const char * filename, Mfe_param* p)
{
      char s[128];
      char* name;
      char* value;
      FILE *fp;

      memset(p,0,sizeof(Mfe_param));

      fp = fopen(filename, "rb");

      if( fp == NULL )
      {
          printf( "%s :%d [read config fail]\n", __func__,__LINE__);
          return -1;
      }

      while (fgets(s, 128, fp) != NULL) {
                 name = strtok(s, ":");
                 value = strtok(NULL, ":");

                 if(!strncmp(name, "FramesToBeEncoded", strlen("FramesToBeEncoded")))
                 {
                     p->FramesToBeEncoded = atoi(value);
                 }

                 if( !strncmp(name,"InputFile",strlen("InputFile")) ){
                       cpString(p->inputPath,value);
                 }
                 else if( !strncmp(name,"OutputFile",strlen("OutputFile")) ){
                       cpString(p->outputPath,value);
                 }

                 if( !p->basic.bEn ){
                     if( !strncmp(name,"#Basic Settings",strlen("#Basic Settings"))){
                            p->basic.bEn = atoi(value);
                     }
                 }

                 if( !p->sLTR.bEn ){
                     if( !strncmp(name,"#LTR Settings",strlen("#LTR Settings"))){
                            p->sLTR.bEn = atoi(value);
                     }
                 }

                 if( !p->sClock.bEn ){
                     if( !strncmp(name,"#Clock Settings",strlen("#Clock Settings:"))){
                            p->sClock.bEn = atoi(value);
                     }
                 }

                 if( !p->sDeblk.bEn ){
                     if( !strncmp(name,"#Deblocking Settings",strlen("#Deblocking Settings"))){
                            p->sDeblk.bEn = atoi(value);
                     }
                 }

                 if( !p->sDisposable.bEn ){
                     if( !strncmp(name,"#Disposable Settings",strlen("#Disposable Settings"))){
                            p->sDisposable.bEn = atoi(value);
                     }
                 }

                 if( !p->sPoc.bEn ){
                     if( !strncmp(name,"#Poc Settings",strlen("#Poc Settings"))){
                            p->sPoc.bEn = atoi(value);
                     }
                 }

                 if( !p->sEntropy.bEn ){
                     if( !strncmp(name,"#Entropy Settings",strlen("#Entropy Settings"))){
                            p->sEntropy.bEn = atoi(value);
                     }
                 }

                 if( !p->sRC.bEn ){
                     if( !strncmp(name,"#RC Settings",strlen("#RC Settings"))){
                            p->sRC.bEn = atoi(value);
                     }
                 }

                 if( !p->sIntraP.bEn ){
                     if( !strncmp(name,"#IntraP Settings",strlen("#IntraP Settings"))){
                            p->sIntraP.bEn = atoi(value);
                     }
                 }

                 if( !p->sInterP.bEn ){
                     if( !strncmp(name,"#InterP Settings",strlen("#InterP Settings"))){
                            p->sInterP.bEn = atoi(value);
                     }
                 }

                 if(!p->sQpMap.bEn)
                 {
                     if(!strncmp(name, "#QPMAP Settings", strlen("#QPMAP Settings")))
                     {
                         p->sQpMap.bEn = atoi(value);
                     }
                 }

                 if( !p->sMSlice.bEn ){
                     if( !strncmp(name,"#MSlice Settings",strlen("#MSlice Settings"))){
                            p->sMSlice.bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[0].bEn ){
                     if( !strncmp(name,"#ROI0 Settings",strlen("#ROI0 Settings"))){
                            p->sRoi[0].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[1].bEn ){
                     if( !strncmp(name,"#ROI1 Settings",strlen("#ROI1 Settings"))){
                            p->sRoi[1].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[2].bEn ){
                     if( !strncmp(name,"#ROI2 Settings",strlen("#ROI2 Settings"))){
                            p->sRoi[2].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[3].bEn ){
                     if( !strncmp(name,"#ROI3 Settings",strlen("#ROI3 Settings"))){
                            p->sRoi[3].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[4].bEn ){
                     if( !strncmp(name,"#ROI4 Settings",strlen("#ROI4 Settings"))){
                            p->sRoi[4].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[5].bEn ){
                     if( !strncmp(name,"#ROI5 Settings",strlen("#ROI5 Settings"))){
                            p->sRoi[5].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[6].bEn ){
                     if( !strncmp(name,"#ROI6 Settings",strlen("#ROI6 Settings"))){
                            p->sRoi[6].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[7].bEn ){
                     if( !strncmp(name,"#ROI7 Settings",strlen("#ROI7 Settings"))){
                            p->sRoi[7].bEn = atoi(value);
                     }
                 }

                 if( !p->sRoi[7].bEn ){
                     if( !strncmp(name,"#ROI7 Settings",strlen("#ROI7 Settings"))){
                            p->sRoi[7].bEn = atoi(value);
                     }
                 }

                 if( !p->sSEI[0].bEn ){
                     if( !strncmp(name,"#InsertUserData0",strlen("#InsertUserData0"))){
                            p->sSEI[0].bEn = atoi(value);
                     }
                 }

                 if( !p->sSEI[1].bEn ){
                     if( !strncmp(name,"#InsertUserData1",strlen("#InsertUserData1"))){
                            p->sSEI[1].bEn = atoi(value);
                     }
                 }

                 if( !p->sSEI[2].bEn ){
                     if( !strncmp(name,"#InsertUserData2",strlen("#InsertUserData2"))){
                            p->sSEI[2].bEn = atoi(value);
                     }
                 }

                 if( !p->sSEI[3].bEn ){
                     if( !strncmp(name,"#InsertUserData3",strlen("#InsertUserData3"))){
                            p->sSEI[3].bEn = atoi(value);
                     }
                 }

                 if(!p->sVUI.bEn)
                 {
                     if(!strncmp(name, "#VUI Setting", strlen("#VUI Setting")))
                     {
                         p->sVUI.bEn = atoi(value);
                     }
                 }

                 if( !p->sNewSeq.bEn ){
                     if( !strncmp(name,"#NewSeq Settings",strlen("#NewSeq Settings"))){
                            p->sNewSeq.bEn = atoi(value);
                     }
                 }


                 if( !p->sPMBR.tc.bEn ){
                     if( !strncmp(name,"#PMBR TC Settings",strlen("#PMBR TC Settings"))){
                            p->sPMBR.tc.bEn = atoi(value);
                     }
                 }

                 if( !p->sPMBR.pc.bEn ){
                     if( !strncmp(name,"#PMBR PC Settings",strlen("#PMBR PC Settings"))){
                            p->sPMBR.pc.bEn = atoi(value);
                     }
                 }

                 if( !p->sPMBR.ss.bEn ){
                     if( !strncmp(name,"#PMBR SS Settings",strlen("#PMBR SS Settings"))){
                            p->sPMBR.ss.bEn = atoi(value);
                     }
                 }
      }

      fseek(fp, 0, SEEK_SET);
      //int IdxROI=0;
      while (fgets(s, 128, fp) != NULL) {
                 name = strtok(s, ":");
                 value = strtok(NULL, ":");

                 //#Basic Settings
                 if( p->basic.bEn ){
                         if( !strncmp(name,"Resolution-Width",strlen("Resolution-Width")) ){
                              p->basic.w = atoi(value);
                         }
                         else if( !strncmp(name,"Resolution-Height",strlen("Resolution-Height")) ){
                              p->basic.h = atoi(value);
                         }
                         else if( !strncmp(name,"fps",strlen("fps")) ){
                              p->basic.fps= atoi(value);
                         }
                         else if( !strncmp(name,"Gop",strlen("Gop")) ){
                              p->basic.Gop = atoi(value);
                         }
                         else if( !strncmp(name,"bitrate",strlen("bitrate")) ){
                              p->basic.bitrate = atoi(value);
                         }
                         else if( !strncmp(name,"nQp",strlen("nQp")) ){
                              p->basic.nQp = atoi(value);
                         }
                         else if( !strncmp(name,"pixfm",strlen("pixfm")) ){
                               cpString(p->basic.pixfm,value);
                         }
                 }

                 if( p->sLTR.bEn ){
                     if( !strncmp(name,"Base",strlen("Base")) ){
                          p->sLTR.nBase = atoi(value);
                     }
                     else if( !strncmp(name,"Enhance",strlen("Enhance")) ){
                          p->sLTR.nEnhance = atoi(value);
                     }
                     else if( !strncmp(name,"EnablePred",strlen("EnablePred")) ){
                          p->sLTR.bEnabledPred = atoi(value);
                     }
                 }

                 //Clock Settings:
                 if( p->sClock.bEn ){
                     if( !strncmp(name,"nClkEn",strlen("nClkEn")) ){
                          p->sClock.nClkEn = atoi(value);
                     }
                     else if( !strncmp(name,"nClkSor",strlen("nClkSor")) ){
                         p->sClock.nClkSor = atoi(value);
                     }
                 }

                 //#Deblocking Settings
                 if( p->sDeblk.bEn ){
                         if( !strncmp(name,"deblock_filter_control",strlen("deblock_filter_control")) ){
                              p->sDeblk.deblock_filter_control= atoi(value);
                         }
                         else if( !strncmp(name,"disable_deblocking_filter_idc",strlen("disable_deblocking_filter_idc")) ){
                              p->sDeblk.disable_deblocking_filter_idc= atoi(value);
                         }
                         else if( !strncmp(name,"slice_alpha_c0_offset_div2",strlen("slice_alpha_c0_offset_div2")) ){
                              p->sDeblk.slice_alpha_c0_offset_div2= atoi(value);
                         }
                         else if( !strncmp(name,"slice_beta_offset_div2",strlen("slice_beta_offset_div2")) ){
                              p->sDeblk.slice_beta_offset_div2= atoi(value);
                         }
                 }
                 //#Disposable Settings
                 if( p->sDisposable.bEn ){
                         if( !strncmp(name,"Disposable Mode",strlen("Disposable Mode")) ){
                              p->sDisposable.bDisposable= atoi(value);
                         }
                 }

                 //#Poc Settings
                 if( p->sPoc.bEn ){
                         if( !strncmp(name,"POC Type",strlen("POC Type")) ){
                              p->sPoc.nPocType = atoi(value);
                         }
                 }

                 //#Entropy Settings
                 if( p->sEntropy.bEn ){
                         if( !strncmp(name,"Cabac",strlen("Cabac")) ){
                              p->sEntropy.bEntropyCodingCABAC = atoi(value);
                         }
                 }

                 //#RC Settings
                 if( p->sRC.bEn ){
                         if( !strncmp(name,"eControlRate",strlen("eControlRate")) ){
                              p->sRC.eControlRate = atoi(value);
                         }
                         else if( !strncmp(name,"nImaxqp",strlen("nImaxqp")) ){
                              p->sRC.nImaxqp= atoi(value);
                         }
                         else if( !strncmp(name,"nIminqp",strlen("nIminqp")) ){
                              p->sRC.nIminqp= atoi(value);
                         }
                         else if( !strncmp(name,"nPmaxqp",strlen("nPmaxqp")) ){
                              p->sRC.nPmaxqp= atoi(value);
                         }
                         else if( !strncmp(name,"nPminqp",strlen("nPminqp")) ){
                              p->sRC.nPminqp= atoi(value);
                         }
                 }
                 //#IntraP Settings
                 if( p->sIntraP.bEn ){
                         if( !strncmp(name,"constrained_intra_pred_flag",strlen("constrained_intra_pred_flag")) ){
                              p->sIntraP.bconstIpred = atoi(value);
                         }
                 }
                 //#InterP Settings
                 if( p->sInterP.bEn ){
                         if( !strncmp(name,"nDmv_X",strlen("nDmv_X")) ){
                              p->sInterP.nDmv_X = atoi(value);
                         }
                         else if( !strncmp(name,"nDmv_Y",strlen("nDmv_Y")) ){
                              p->sInterP.nDmv_Y = atoi(value);
                         }
                         else if( !strncmp(name,"nSubpel",strlen("nSubpel")) ){
                              p->sInterP.nSubpel = atoi(value);
                         }
                         else if( !strncmp(name,"bInter4x4PredEn",strlen("bInter4x4PredEn")) ){
                              p->sInterP.bInter4x4PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter4x8PredEn",strlen("bInter4x8PredEn")) ){
                              p->sInterP.bInter4x8PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter8x4PredEn",strlen("bInter8x4PredEn")) ){
                              p->sInterP.bInter8x4PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter8x8PredEn",strlen("bInter8x8PredEn")) ){
                              p->sInterP.bInter8x8PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter8x16PredEn",strlen("bInter8x16PredEn")) ){
                              p->sInterP.bInter8x16PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter16x8PredEn",strlen("bInter16x8PredEn")) ){
                              p->sInterP.bInter16x8PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInter16x16PredEn",strlen("bInter16x16PredEn")) ){
                              p->sInterP.bInter16x16PredEn = atoi(value);
                         }
                         else if( !strncmp(name,"bInterSKIPPredEn",strlen("bInterSKIPPredEn")) ){
                              p->sInterP.bInterSKIPPredEn = atoi(value);
                         }
                 }

                 //#MSlice Settings
                 if( p->sMSlice.bEn ){
                         if( !strncmp(name,"nRows",strlen("nRows")) ){
                              p->sMSlice.nRows= atoi(value);
                         }
                         else if( !strncmp(name,"nBits",strlen("nBits")) ){
                              p->sMSlice.nBits= atoi(value);
                         }
                 }

                 //#QPMAP Settings
                 if(p->sQpMap.bEn)
                 {
                     if(!strncmp(name, "QPMAP_Enable", strlen("QPMAP_Enable")))
                     {
                         p->sQpMap.bEnable = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry0", strlen("QPMAP_Entry0")))
                     {
                         p->sQpMap.nEntry[0] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry1", strlen("QPMAP_Entry1")))
                     {
                         p->sQpMap.nEntry[1] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry2", strlen("QPMAP_Entry2")))
                     {
                         p->sQpMap.nEntry[2] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry3", strlen("QPMAP_Entry3")))
                     {
                         p->sQpMap.nEntry[3] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry4", strlen("QPMAP_Entry4")))
                     {
                         p->sQpMap.nEntry[4] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry5", strlen("QPMAP_Entry5")))
                     {
                         p->sQpMap.nEntry[5] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry6", strlen("QPMAP_Entry6")))
                     {
                         p->sQpMap.nEntry[6] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry7", strlen("QPMAP_Entry7")))
                     {
                         p->sQpMap.nEntry[7] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry8", strlen("QPMAP_Entry8")))
                     {
                         p->sQpMap.nEntry[8] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry9", strlen("QPMAP_Entry9")))
                     {
                         p->sQpMap.nEntry[9] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_Entry10", strlen("QPMAP_Entry10")))
                     {
                         p->sQpMap.nEntry[10] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_EntryA1", strlen("QPMAP_EntryA1")))
                     {
                         p->sQpMap.nEntry[11] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_EntryA2", strlen("QPMAP_EntryA2")))
                     {
                         p->sQpMap.nEntry[12] = atoi(value);
                     }
                     else if(!strncmp(name, "QPMAP_EntryA3", strlen("QPMAP_EntryA3")))
                     {
                         p->sQpMap.nEntry[13] = atoi(value);
                     }
                 }

                 //#ROI Settings
                 if( p->sRoi[0].bEn ){
                     getROISettings(0,name,value,p);
                 }

                 if( p->sRoi[1].bEn ){
                     getROISettings(1,name,value,p);
                 }

                 if( p->sRoi[2].bEn ){
                     getROISettings(2,name,value,p);
                 }

                 if( p->sRoi[3].bEn ){
                     getROISettings(3,name,value,p);
                 }

                 if( p->sRoi[4].bEn ){
                     getROISettings(4,name,value,p);
                 }

                 if( p->sRoi[5].bEn ){
                     getROISettings(5,name,value,p);
                 }

                 if( p->sRoi[6].bEn ){
                     getROISettings(6,name,value,p);
                 }

                 if( p->sRoi[7].bEn ){
                     getROISettings(7,name,value,p);
                 }

                 //##InsertUserData0
                 if( p->sSEI[0].bEn ){
                         if( !strncmp(name,"pU8data0",strlen("pU8data0")) ){
                              GenSEINALU(p,0, value);
                         }
                 }

                 //##InsertUserData1
                 if( p->sSEI[1].bEn ){
                         if( !strncmp(name,"pU8data1",strlen("pU8data1")) ){
                              GenSEINALU(p,1, value);
                         }
                 }

                 //##InsertUserData2
                 if( p->sSEI[2].bEn ){
                         if( !strncmp(name,"pU8data2",strlen("pU8data2")) ){
                              GenSEINALU(p,2, value);
                         }
                 }

                 //##InsertUserData3
                 if( p->sSEI[3].bEn ){
                         if( !strncmp(name,"pU8data3",strlen("pU8data3")) ){
                              GenSEINALU(p,3, value);
                         }
                 }

                 //#VUI Setting:0
                 if(p->sVUI.bEn)
                 {
                     if(!strncmp(name, "iSarW", strlen("iSarW")))
                     {
                         p->sVUI.u16SarWidth = atoi(value);
                     }
                     else if(!strncmp(name, "iSarH", strlen("iSarH")))
                     {
                         p->sVUI.u16SarHeight = atoi(value);
                     }
                     else if(!strncmp(name, "bTimingInfoPres", strlen("bTimingInfoPres")))
                     {
                         p->sVUI.u8TimingInfoPresentFlag = atoi(value);
                     }
                     else if(!strncmp(name, "bVideoFullRange", strlen("bVideoFullRange")))
                     {
                         p->sVUI.u8VideoFullRangeFlag = atoi(value);
                     }
                     else if(!strncmp(name, "bVideoSignalPres", strlen("bVideoSignalPres")))
                     {
                         p->sVUI.u8VideoSignalTypePresentFlag = atoi(value);
                     }
                     else if(!strncmp(name, "iVideoFormat", strlen("iVideoFormat")))
                     {
                         p->sVUI.u8VideoFormat = atoi(value);
                     }
                     else if(!strncmp(name, "bColourDescPres", strlen("bColourDescPres")))
                     {
                         p->sVUI.u8ColourDescriptionPresentFlag = atoi(value);
                     }
                 }

                 //#NewSeq Settings
                 if( p->sNewSeq.bEn ){
                         if( !strncmp(name,"NewSeqW",strlen("NewSeqW")) ){
                              p->sNewSeq.nWidth = atoi(value);
                         }
                         else if( !strncmp(name,"NewSeqH",strlen("NewSeqH")) ){
                              p->sNewSeq.nHeight = atoi(value);
                         }
                         else if( !strncmp(name,"NewSeqFps",strlen("NewSeqFps")) ){
                              p->sNewSeq.xFramerate = atoi(value);
                         }
                 }



                 //#PMBR TC Settings
                 if( p->sPMBR.tc.bEn ){
                         if( !strncmp(name,"LutEntryClipRange",strlen("LutEntryClipRange")) ){
                              p->sPMBR.tc.i_LutEntryClipRange = atoi(value);
                         }
                         else if( !strncmp(name,"TextWeightType",strlen("TextWeightType")) ){
                              p->sPMBR.tc.i_TextWeightType = atoi(value);
                         }
                         else if( !strncmp(name,"SkinLvShift",strlen("SkinLvShift")) ){
                              p->sPMBR.tc.i_SkinLvShift = atoi(value);
                         }
                         else if( !strncmp(name,"WeightOffset",strlen("WeightOffset")) ){
                              p->sPMBR.tc.i_WeightOffset = atoi(value);
                         }
                         else if( !strncmp(name,"TcOffset",strlen("TcOffset")) ){
                              p->sPMBR.tc.i_TcOffset = atoi(value);
                         }
                         else if( !strncmp(name,"TcToSkinAlpha",strlen("TcToSkinAlpha")) ){
                              p->sPMBR.tc.i_TcToSkinAlpha = atoi(value);
                         }
                         else if( !strncmp(name,"TcGradThr",strlen("TcGradThr")) ){
                              p->sPMBR.tc.i_TcGradThr = atoi(value);
                         }
                 }

                 //#PMBR PC Settings
                 if( p->sPMBR.pc.bEn ){
                         if( !strncmp(name,"PwYMax",strlen("PwYMax")) ){
                              p->sPMBR.pc.i_PwYMax = atoi(value);
                         }
                         else if( !strncmp(name,"PwYMin",strlen("PwYMin")) ){
                              p->sPMBR.pc.i_PwYMin = atoi(value);
                         }
                         else if( !strncmp(name,"PwCbMax",strlen("PwCbMax")) ){
                              p->sPMBR.pc.i_PwCbMax = atoi(value);
                         }
                         else if( !strncmp(name,"PwCbMin",strlen("PwCbMin")) ){
                              p->sPMBR.pc.i_PwCbMin = atoi(value);
                         }
                         else if( !strncmp(name,"PwCrMax",strlen("PwCrMax")) ){
                              p->sPMBR.pc.i_PwCrMax = atoi(value);
                         }
                         else if( !strncmp(name,"PwCrMin",strlen("PwCrMin")) ){
                              p->sPMBR.pc.i_PwCrMin = atoi(value);
                         }
                         else if( !strncmp(name,"PwCbPlusCrMin",strlen("PwCbPlusCrMin")) ){
                              p->sPMBR.pc.i_PwCbPlusCrMin = atoi(value);
                         }
                         else if( !strncmp(name,"PwAddConditionEn",strlen("PwAddConditionEn")) ){
                              p->sPMBR.pc.i_PwAddConditionEn = atoi(value);
                         }
                         else if( !strncmp(name,"PwCrOffset",strlen("PwCrOffset")) ){
                              p->sPMBR.pc.i_PwCrOffset = atoi(value);
                         }
                         else if( !strncmp(name,"PwCbCrOffset",strlen("PwCbCrOffset")) ){
                              p->sPMBR.pc.i_PwCbCrOffset = atoi(value);
                         }
                         else if( !strncmp(name,"PcDeadZone",strlen("PcDeadZone")) ){
                              p->sPMBR.pc.i_PcDeadZone = atoi(value);
                         }
                 }

                 //#PMBR SS Settings
                 if( p->sPMBR.ss.bEn ){
                     if( !strncmp(name,"SmoothEn",strlen("SmoothEn")) ){
                         p->sPMBR.ss.i_SmoothEn = atoi(value);
                     }
                     else if( !strncmp(name,"SmoothClipMax",strlen("SmoothClipMax")) ){
                          p->sPMBR.ss.i_SmoothClipMax = atoi(value);
                     }
                     else if( !strncmp(name,"SecStgAlpha",strlen("SecStgAlpha")) ){
                          p->sPMBR.ss.i_SecStgAlpha = atoi(value);
                     }
                     else if( !strncmp(name,"SecStgBitWghtOffset",strlen("SecStgBitWghtOffset")) ){
                          p->sPMBR.ss.i_SecStgBitWghtOffset = atoi(value);
                     }
                 }
      }
      fclose(fp);

   return 0;
}

int SetInOutPath(Mfe_param* pParam){
    memset(pParam->inputPath,0,64);
    memset(pParam->outputPath,0,64);

    printf("> Input file: ");
    if(scanf( "%s" , pParam->inputPath ) == EOF){
        printf("get inputPath fail\n");
    }
    printf("> Output file: ");
    if(scanf( "%s" , pParam->outputPath ) == EOF){
        printf("get outputPath fail\n");
    }
   return 0;
}

void ShowCurrentMFESetting(Mfe_param* pParam){
       int i=0;

       printf("-Input file: %s\n", pParam->inputPath);
       printf("-Output file: %s\n", pParam->outputPath);

       printf("-FramesToBeEncoded: %u\n", pParam->FramesToBeEncoded);

       if( pParam->basic.bEn ){
            printf("-pixfm: %s\n", pParam->basic.pixfm);
            printf("-Video Resolution: %d x %d fps: %d bitrates: %d Gop: %d nQp: %d\n",pParam->basic.w, pParam->basic.h,pParam->basic.fps, pParam->basic.bitrate, pParam->basic.Gop, pParam->basic.nQp);
       }

       if( pParam->sLTR.bEn ){
           printf("-nBase: %d\n",pParam->sLTR.nBase);
           printf("-nEnhance: %d\n",pParam->sLTR.nEnhance);
           printf("-bEnabledPred: %d\n",pParam->sLTR.bEnabledPred);
       }

       if( pParam->sDeblk.bEn ){
            printf("-deblock_filter_control: %d\n",pParam->sDeblk.deblock_filter_control);
            printf("-disable_deblocking_filter_idc: %d\n",pParam->sDeblk.disable_deblocking_filter_idc);
            printf("-slice_alpha_c0_offset_div2: %d\n",pParam->sDeblk.slice_alpha_c0_offset_div2);
            printf("-slice_beta_offset_div2: %d\n",pParam->sDeblk.slice_beta_offset_div2);
       }

       if( pParam->sDisposable.bEn ){
            printf("-Disposable Mode: %d\n",pParam->sDisposable.bDisposable);
       }

       if( pParam->sPoc.bEn ){
            printf("-POC Type: %d\n",pParam->sPoc.nPocType);
       }

       if( pParam->sEntropy.bEn ){
            printf("-Cabac: %d\n",pParam->sEntropy.bEntropyCodingCABAC);
       }

       if( pParam->sRC.bEn ){
            printf("-eControlRate: %d (0:ConstQp 1:CBR 2:VBR)\n",pParam->sRC.eControlRate);
            if(pParam->sRC.eControlRate == Mfe_RQCT_METHOD_VBR)
            {
                printf("-nImaxqp: %d, nIminqp: %d, nPmaxqp: %d, nPminqp: %d\n",pParam->sRC.nImaxqp, pParam->sRC.nIminqp, pParam->sRC.nPmaxqp, pParam->sRC.nPminqp);
            }
       }

       if( pParam->sIntraP.bEn ){
            printf("-constrained_intra_pred_flag: %d\n",pParam->sIntraP.bconstIpred);
       }

       if( pParam->sInterP.bEn ){
            printf("-nDmv_X: %d nDmv_Y: %d\n",pParam->sInterP.nDmv_X,pParam->sInterP.nDmv_Y);
            printf("-nSubpel: %d \n",pParam->sInterP.nSubpel);
            printf("-bInter4x4PredEn:%d  bInter4x8PredEn:%d \n",pParam->sInterP.bInter4x4PredEn,pParam->sInterP.bInter4x8PredEn);
            printf("-bInter8x4PredEn:%d  bInter8x8PredEn:%d \n",pParam->sInterP.bInter8x4PredEn,pParam->sInterP.bInter8x8PredEn);
            printf("-bInter8x16PredEn:%d  bInter16x8PredEn:%d \n",pParam->sInterP.bInter8x16PredEn,pParam->sInterP.bInter16x8PredEn);
            printf("-bInter16x16PredEn:%d  bInterSKIPPredEn:%d \n",pParam->sInterP.bInter16x16PredEn,pParam->sInterP.bInterSKIPPredEn);
       }

       if( pParam->sMSlice.bEn ){
            printf("-nRows: %d\n",pParam->sMSlice.nRows);
            printf("-nBits: %d\n",pParam->sMSlice.nBits);
       }

       if(pParam->sQpMap.bEn)
       {
           printf("-nQPMap Enable: %u\n", pParam->sQpMap.bEnable);

           for(i = 0; i < 14; i++)
           {
               printf("-nQPMap[%d]: %u\n", i, pParam->sQpMap.nEntry[i]);
           }
       }

       for( i=0; i<MAX_ROI_NUM; i++){
            if(pParam->sRoi[i].bEn){
                printf("-Idx: %d Enable: %d (X,Y)->(%d,%d) (W,H)->(%d,%d) Qp:%d AbsQp:%d\n",pParam->sRoi[i].nIdx,pParam->sRoi[i].bEnable,
                        pParam->sRoi[i].nMbX,pParam->sRoi[i].nMbY,pParam->sRoi[i].nMbW,pParam->sRoi[i].nMbH,pParam->sRoi[i].nMbqp, pParam->sRoi[i].bAbsQp);
            }
       }

       for( i=0; i<MAX_SEI_NUM; i++){
           if( pParam->sSEI[i].bEn ){
                printf("-User Data%d: %s\n",i,&pParam->sSEI[i].U8data[23]);
           }
       }

       if(pParam->sVUI.bEn)
       {
        printf("-u16SarWidth: %d\n", pParam->sVUI.u16SarWidth);
        printf("-u16SarHeight: %d\n", pParam->sVUI.u16SarHeight);
        printf("-u8TimingInfoPresentFlag: %d\n", pParam->sVUI.u8TimingInfoPresentFlag);
        printf("-u8VideoFullRangeFlag: %d\n", pParam->sVUI.u8VideoFullRangeFlag);
        printf("-u8VideoSignalTypePresentFlag: %d\n", pParam->sVUI.u8VideoSignalTypePresentFlag);
        printf("-u8VideoFormat: %d\n", pParam->sVUI.u8VideoFormat);
        printf("-u8ColourDescriptionPresentFlag: %d\n", pParam->sVUI.u8ColourDescriptionPresentFlag);
       }

       if( pParam->sNewSeq.bEn ){
            printf("-NewSeqW: %d\n",pParam->sNewSeq.nWidth);
            printf("-NewSeqH: %d\n",pParam->sNewSeq.nHeight);
            printf("-NewSeqFps: %d\n",pParam->sNewSeq.xFramerate);
       }

       if( pParam->sClock.bEn )
       {
           printf("-Clock Disable: %d\n",pParam->sClock.nClkEn);
           printf("-Clock Source: %d (0:288, 1:240, 2:192, 3:123, 4:384, 5:320, 6:172 Mhz)\n",pParam->sClock.nClkSor);
       }

       if( pParam->sPMBR.tc.bEn )
       {
           printf("-PMBR LutEntryClipRange: %d\n",pParam->sPMBR.tc.i_LutEntryClipRange);
           printf("-PMBR TextWeightType: %d\n",pParam->sPMBR.tc.i_TextWeightType);
           printf("-PMBR SkinLvShift: %d\n",pParam->sPMBR.tc.i_SkinLvShift);
           printf("-PMBR WeightOffset: %d\n",pParam->sPMBR.tc.i_WeightOffset);
           printf("-PMBR TcOffset: %d\n",pParam->sPMBR.tc.i_TcOffset);
           printf("-PMBR TcToSkinAlpha: %d\n",pParam->sPMBR.tc.i_TcToSkinAlpha);
           printf("-PMBR TcGradThr: %d\n",pParam->sPMBR.tc.i_TcGradThr);
       }

       if(pParam->sPMBR.pc.bEn)
       {
           printf("-PMBR PwYMax: %d\n",pParam->sPMBR.pc.i_PwYMax);
           printf("-PMBR PwYMin: %d\n",pParam->sPMBR.pc.i_PwYMin);
           printf("-PMBR PwCbMax: %d\n",pParam->sPMBR.pc.i_PwCbMax);
           printf("-PMBR PwCbMin: %d\n",pParam->sPMBR.pc.i_PwCbMin);
           printf("-PMBR PwCrMax: %d\n",pParam->sPMBR.pc.i_PwCrMax);
           printf("-PMBR PwCrMin: %d\n",pParam->sPMBR.pc.i_PwCrMin);
           printf("-PMBR PwCbPlusCrMin: %d\n",pParam->sPMBR.pc.i_PwCbPlusCrMin);
           printf("-PMBR PwAddConditionEn: %d\n",pParam->sPMBR.pc.i_PwAddConditionEn);
           printf("-PMBR PwCrOffset: %d\n",pParam->sPMBR.pc.i_PwCrOffset);
           printf("-PMBR PwCbCrOffset: %d\n",pParam->sPMBR.pc.i_PwCbCrOffset);
           printf("-PMBR PcDeadZone: %d\n",pParam->sPMBR.pc.i_PcDeadZone);
       }

       if(pParam->sPMBR.ss.bEn)
       {
           printf("-PMBR SmoothEn: %d\n",pParam->sPMBR.ss.i_SmoothEn);
           printf("-PMBR SmoothClipMax: %d\n",pParam->sPMBR.ss.i_SmoothClipMax);
           printf("-PMBR SecStgAlpha: %d\n",pParam->sPMBR.ss.i_SecStgAlpha);
           printf("-PMBR SecStgBitWghtOffset: %d\n",pParam->sPMBR.ss.i_SecStgBitWghtOffset);
       }

}

void SetVideoBasicSettings(Mfe_param* pParam){
     int resW,resH,fps,bitrates,gop;
     printf("> frame width: ");
     if(scanf("%d",&resW) != EOF){
          pParam->basic.w = resW;
     }
     printf("> frame height: ");
     if(scanf("%d",&resH) != EOF){
          pParam->basic.h = resH;
     }
     printf("> frame rate: ");
     if(scanf("%d",&fps) != EOF){
          pParam->basic.fps = fps;
     }
     printf("> bitrate: ");
     if(scanf("%d",&bitrates) != EOF){
          pParam->basic.bitrate = bitrates;
     }
     printf("> Gop: ");
     if(scanf("%d",&gop) != EOF){
          pParam->basic.Gop = gop;
     }
     pParam->basic.bEn = TRUE;
     printf("Done!\n");
}

void SetH264LTR(Mfe_param* pParam){
    int a,b,c;
    printf("> nBase(1 ~ N): ");
    if(scanf("%d",&a) != EOF){
        pParam->sLTR.nBase = a;
    }
    printf("> nEnhance(1 ~ M): ");
    if(scanf("%d",&b) != EOF){
        pParam->sLTR.nEnhance = b;
    }
    printf("> bEnabledPred(0/1): ");
    if(scanf("%d",&c) != EOF){
        pParam->sLTR.bEnabledPred = c;
    }
}

void SetH264Dblk(Mfe_param* pParam){
     int a,b,c,d;
     printf("> deblock_filter_control(0/1): ");
     if(scanf("%d",&a) != EOF){
         pParam->sDeblk.deblock_filter_control= a;
     }
     printf("> disable_deblocking_filter_idc(0/1/2): ");
     if(scanf("%d",&b)!= EOF){
         pParam->sDeblk.disable_deblocking_filter_idc= b;
     }
     printf("> slice_alpha_c0_offset_div2(-6~6): ");
     if(scanf("%d",&c)!= EOF){
         pParam->sDeblk.slice_alpha_c0_offset_div2= c;
     }
     printf("> slice_beta_offset_div2(-6~6): ");
     if(scanf("%d",&d)!= EOF){
         pParam->sDeblk.slice_beta_offset_div2= d;
     }
     pParam->sDeblk.bEn = TRUE;
     printf("Done!\n");
}

void SetH264Disposable(Mfe_param* pParam){
     int a;
     printf("> Disposable Mode(0/1): ");
     if(scanf("%d",&a)!= EOF){
         pParam->sDisposable.bDisposable = a;
     }
     pParam->sDisposable.bEn = TRUE;
     printf("Done!\n");
}

void SetH264Poc(Mfe_param* pParam){
     int a;
     printf("> Poc type(0/2): ");
     if(scanf("%d",&a)!= EOF){
        pParam->sPoc.nPocType = a;
     }
     pParam->sPoc.bEn = TRUE;
     printf("Done!\n");
}

void SetH264Entropy(Mfe_param* pParam){
     int a;
     printf("> enable cabac (0/1): ");
     if(scanf("%d",&a)!= EOF){
         pParam->sEntropy.bEntropyCodingCABAC = a;
     }
     pParam->sEntropy.bEn = TRUE;
     printf("Done!\n");
}


void SetH264RC(Mfe_param* pParam){
     int a;
     printf("> eControlRate (0:ConstQp 1:CBR 2:VBR):");
     if(scanf("%d",&a)!= EOF){
         pParam->sRC.eControlRate= a;
     }

     if(pParam->sRC.eControlRate == Mfe_RQCT_METHOD_VBR)
     {
         printf("> nImaxqp :");
         if(scanf("%d",&a)!= EOF){
             pParam->sRC.nImaxqp= a;
         }

         printf("> nIminqp :");
         if(scanf("%d",&a)!= EOF){
             pParam->sRC.nIminqp= a;
         }

         printf("> nPmaxqp :");
         if(scanf("%d",&a)!= EOF){
             pParam->sRC.nPmaxqp= a;
         }

         printf("> nPminqp :");
         if(scanf("%d",&a)!= EOF){
             pParam->sRC.nPminqp= a;
         }
     }

     pParam->sRC.bEn = TRUE;
     printf("Done!\n");
}

void SetH264IntraP(Mfe_param* pParam){
     int a;
     printf("> constrained_intra_pred_flag (0/1): ");
     if(scanf("%d",&a)!= EOF){
         pParam->sIntraP.bconstIpred = a;
     }
     pParam->sIntraP.bEn = TRUE;
     printf("Done!\n");
}
void SetH264InterP(Mfe_param* pParam){
     int a,b,c,d,e,f,g,h,i,j,k;
     printf("> nDmv_X (8~16): ");
     if(scanf("%d",&a)!= EOF){
         pParam->sInterP.nDmv_X = a;
     }
     printf("> nDmv_Y (8/16): ");
     if(scanf("%d",&b)!= EOF){
         pParam->sInterP.nDmv_Y = b;
     }
     printf("> nSubpel (0:Pixel 1:HalfPel 2:QuarterPel ): ");
     if(scanf("%d",&c)!= EOF){
         pParam->sInterP.nSubpel = c;
     }
     printf("> bInter4x4PredEn (0/1): ");
     if(scanf("%d",&d)!= EOF){
         pParam->sInterP.bInter4x4PredEn = d;
     }
     printf("> bInter8x4PredEn (0/1): ");
     if(scanf("%d",&e)!= EOF){
         pParam->sInterP.bInter8x4PredEn = e;
     }
     printf("> bInter4x8PredEn (0/1): ");
     if(scanf("%d",&f)!= EOF){
         pParam->sInterP.bInter4x8PredEn = f;
     }
     printf("> bInter8x8PredEn (0/1): ");
     if(scanf("%d",&g)!= EOF){
         pParam->sInterP.bInter8x8PredEn = g;
     }
     printf("> bInter16x8PredEn (0/1): ");
     if(scanf("%d",&h)!= EOF){
         pParam->sInterP.bInter16x8PredEn = h;
     }
     printf("> bInter8x16PredEn (0/1): ");
     if(scanf("%d",&i)!= EOF){
         pParam->sInterP.bInter8x16PredEn = i;
     }
     printf("> bInter16x16PredEn (0/1): ");
     if(scanf("%d",&j)!= EOF){
         pParam->sInterP.bInter16x16PredEn = j;
     }
     printf("> bInterSKIPPredEn (0/1): ");
     if(scanf("%d",&k)!= EOF){
         pParam->sInterP.bInterSKIPPredEn = k;
     }
     pParam->sInterP.bEn = TRUE;
     printf("Done!\n");
}

void SetH264SS(Mfe_param* pParam){
     int a,b;
     printf("> nRows (0/1/2/4/8): ");
     if(scanf("%d",&a)!= EOF){
         pParam->sMSlice.nRows = a;
     }
     printf("> nBits (0 ~ (2^16 �V 1)): ");
     if(scanf("%d",&b)!= EOF){
         pParam->sMSlice.nBits = b;
     }
     pParam->sMSlice.bEn = TRUE;
     printf("Done!\n");
}

void SetH264ROI(Mfe_param* pParam){
     int a,b,c,d,e,f,g,h;
     printf("> ROI Index (0~7): ");
     if(scanf("%d",&a)!= EOF){
         if( a >= MAX_ROI_NUM ) return;
         pParam->sRoi[a].nIdx = a;
     }
     printf("> ROI Enable (0/1): ");
     if(scanf("%d",&b)!= EOF){
         pParam->sRoi[a].bEnable = b;
     }
     printf("> MbX : ");
     if(scanf("%d",&c)!= EOF){
         pParam->sRoi[a].nMbX = c;
     }
     printf("> MbY : ");
     if(scanf("%d",&d)!= EOF){
         pParam->sRoi[a].nMbY = d;
     }
     printf("> MbW : ");
     if(scanf("%d",&e)!= EOF){
         pParam->sRoi[a].nMbW = e;
     }
     printf("> MbH : ");
     if(scanf("%d",&f)!= EOF){
         pParam->sRoi[a].nMbH = f;
     }
     printf("> Qp (0~51): ");
     if(scanf("%d", &g) != EOF)
     {
         pParam->sRoi[a].nMbqp = g;
     }
     printf("> AbsQp (0~1): ");
     if(scanf("%d", &h) != EOF)
     {
         pParam->sRoi[a].bAbsQp = h;
     }

     pParam->sRoi[a].bEn = TRUE;
     printf("Done!\n");
}

void SetH264QPMAP(Mfe_param* pParam)
{
    int a, b, c;

    printf("> QPMAP Enable: ");
    if(scanf("%d", &c) != EOF)
    {
        pParam->sQpMap.bEnable = c;
    }
    printf("> QPMAP Entry (0~13): ");
    if(scanf("%d", &a) != EOF)
    {
        if(a >= 14) return;

    }
    printf("> Entry QP offset (-48 ~ 48): ");
    if(scanf("%d", &b) != EOF)
    {
        pParam->sQpMap.nEntry[a] = b;
    }
}

void SetH264SEI(Mfe_param* pParam){
      char pstr[1024];
      int a=0;
      int i = 0;
      memset(pstr,0,1024);
      for( i=0; i< MAX_SEI_NUM; i++)
      {
          printf("> User Data%d enable (0/1) : ",i);
          if(scanf("%d",&a)!= EOF){
              if (a==1){
                          printf("> User Data%d:",i);
                          if(scanf("%d",&a)!= EOF){}
                          fgets(pstr, 1024 , stdin);
                          GenSEINALU(pParam,i,pstr);
                          pParam->sSEI[i].bEn = TRUE;
              }
              else{
                          pParam->sSEI[i].bEn = FALSE;
              }
          }
     }

     printf("Done!\n");
}

void SetH264VUI(Mfe_param* pParam)
{
    int a, b, c, d, e, f, g;
    printf("> u16SarWidth: ");
    if(scanf("%d", &a) != EOF)
    {
        pParam->sVUI.u16SarWidth = a;
    }
    printf("> u16SarHeight: ");
    if(scanf("%d", &b) != EOF)
    {
        pParam->sVUI.u16SarHeight = b;
    }
    printf("> u8TimingInfoPresentFlag: ");
    if(scanf("%d", &c) != EOF)
    {
        pParam->sVUI.u8TimingInfoPresentFlag = c;
    }
    printf("> u8VideoFullRangeFlag: ");
    if(scanf("%d", &d) != EOF)
    {
        pParam->sVUI.u8VideoFullRangeFlag = d;
    }
    printf("> u8VideoSignalTypePresentFlag: ");
    if(scanf("%d", &e) != EOF)
    {
        pParam->sVUI.u8VideoSignalTypePresentFlag = e;
    }
    printf("> u8VideoFormat: ");
    if(scanf("%d", &f) != EOF)
    {
        pParam->sVUI.u8VideoFormat = f;
    }
    printf("> u8ColourDescriptionPresentFlag: ");
    if(scanf("%d", &g) != EOF)
    {
        pParam->sVUI.u8ColourDescriptionPresentFlag = g;
    }
    pParam->sVUI.bEn = TRUE;
    printf("Done!\n");
}

void SetH264NewSeq(Mfe_param* pParam){
     int a,b,c;
     printf("> NewSeq Width:");
     if(scanf("%d",&a)!= EOF){
         pParam->sNewSeq.nWidth = a;
     }
     printf("> NewSeq Height:");
     if(scanf("%d",&b)!= EOF){
         pParam->sNewSeq.nHeight = b;
     }
     printf("> NewSeq Fps:");
     if(scanf("%d",&c)!= EOF){
         pParam->sNewSeq.xFramerate = c;
     }

     pParam->sNewSeq.bEn = TRUE;
     printf("Done!\n");
}

void SetVideoClkSettings(Mfe_param* pParam)
{
    int a, b;

    printf("> Clock Disable: (0:Enable, 1:Disable)");
    if(scanf("%d",&a)!= EOF){
        pParam->sClock.nClkEn = a;
    }
    printf("\n %d \n", pParam->sClock.nClkEn);

    printf("> Clock Source:\n");
    printf(" 0 : 288 Mhz \n");
    printf(" 1 : 240 Mhz \n");
    printf(" 2 : 192 Mhz \n");
    printf(" 3 : 123 Mhz \n");
    printf(" 4 : 384 Mhz \n");
    printf(" 5 : 320 Mhz \n");
    printf(" 6 : 172 Mhz \n");
    if(scanf("%d",&b)!= EOF){
        pParam->sClock.nClkSor = b;
    }

    printf("\n %d \n", pParam->sClock.nClkSor);

    pParam->sClock.bEn = TRUE;
    printf("Done!\n");
}

void SetMFEParameter(Mfe_param* pParam){
     int num =-1;
     char cmd[10];
     DisplayMFESetting();
     do{
         printf(">>");
         if(scanf("%s", cmd)!= EOF){
             num = atoi(cmd);
         }
         switch(num){
            case 0:
                    break;
            case 1:
                    SetVideoBasicSettings(pParam);
                    break;
            case 2:
                    SetH264Dblk(pParam);
                    break;
            case 3:
                    SetH264Disposable(pParam);
                    break;
            case 4:
                    SetH264Poc(pParam);
                    break;
            case 5:
                    SetH264Entropy(pParam);
                    break;
            case 6:
                    SetH264RC(pParam);
                    break;
            case 7:
                    SetH264IntraP(pParam);
                    break;
            case 8:
                    SetH264InterP(pParam);
                    break;
            case 9:
                    SetH264SS(pParam);
                    break;
            case 10:
                    SetH264ROI(pParam);
                    break;
            case 11:
                    SetH264SEI(pParam);
                    break;
            case 12:
                    SetH264NewSeq(pParam);
                    break;
            case 13:
                    SetVideoClkSettings(pParam);
                    break;
            case 14:
                    SetH264VUI(pParam);
                    break;
            case 15:
                    SetH264LTR(pParam);
                    break;
            case 16:
                    SetH264QPMAP(pParam);
                    break;
            default:
                    //DisplayMFESetting();
                    break;
         }
         DisplayMFESetting();
     }while(num!=0);

}
