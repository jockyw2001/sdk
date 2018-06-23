
//==============================================================================
//
//  File        : AudioAecProcess.h
//  Description : Aduio AEC Process function source code
//  Author      : ChenChu Hsu
//  Revision    : 1.0
//
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
#ifndef _AUDIOAECPROCESS_H_
#define _AUDIOAECPROCESS_H_

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum
{
	IAA_AEC_SAMPLE_RATE_8000  =  8000 ,
	IAA_AEC_SAMPLE_RATE_16000 = 16000 ,
}IAA_AEC_SAMPLE_RATE;

typedef enum
{
	IAA_AEC_TRUE = 1,
	IAA_AEC_FALSE = 0,
}IAA_AEC_BOOL;

typedef struct
{
	unsigned int point_number;
	unsigned int nearend_channel;
	unsigned int farend_channel;
	IAA_AEC_SAMPLE_RATE sample_rate; //8000 or 16000
}AudioAecInit;

typedef struct
{
	IAA_AEC_BOOL comfort_noise_enable;
	unsigned int suppression_mode_freq[6]; //[6]
	unsigned int suppression_mode_intensity[7]; //every element 0~15 [7]
}AudioAecConfig;

typedef void* AEC_HANDLE;
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaAec_GetBufferSize(void);
AEC_HANDLE IaaAec_Init(char* working_buffer_address, AudioAecInit * aec_init);
int IaaAec_Config(AEC_HANDLE handle, AudioAecConfig *aec_config);
AEC_HANDLE IaaAec_Reset(char* working_buffer_address, AudioAecInit * aec_init);
int IaaAec_Free(AEC_HANDLE handle);
int IaaAec_Run(AEC_HANDLE handle, short* pss_aduio_near_end, short* pss_aduio_far_end);
int IaaAec_GetLibVersion(unsigned short *ver_year,
						 unsigned short *ver_date,
						 unsigned short *ver_time);
void IaaAec_GenKey(char* code_out);
void IaaAec_VerifyKey(char* code_in);
int IaaAec_GetKeyLen(void);

#endif // #ifndef _AUDIOAECPROCESS_H_
#ifdef __cplusplus
}
#endif
