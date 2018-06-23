//==============================================================================
//
//  File        : AudioProcess.h
//  Description : Audio Process function source code
//  Author      : ChenChu Hsu
//  Revision    : 1.0
//
//==============================================================================
#ifndef AUDIOPROCESS_H_
#define AUDIOPROCESS_H_
typedef void* APC_HANDLE;
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define eqCOSTDOWN_EN                   (1)
#define COSTDOWN_EN                     (0)
#define _EQ_BAND_NUM                    (10)
//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================
typedef enum {
    IAA_APC_SAMPLE_RATE_8000  =  8000 ,
    IAA_APC_SAMPLE_RATE_16000 = 16000 ,
}IAA_APC_SAMPLE_RATE;

typedef enum {
    AUDIO_HPF_FREQ_80 , /* 80Hz*/
    AUDIO_HPF_FREQ_120, /*120Hz*/
    AUDIO_HPF_FREQ_150, /*150Hz*/
    AUDIO_HPF_FREQ_BUTT,
}IAA_HPF_FREQ;


typedef enum {
    NR_SPEED_LOW,
    NR_SPEED_MID,
    NR_SPEED_HIGH
} NR_CONVERGE_SPEED;

/*ANR config structure*/

typedef struct{
    unsigned int anr_enable;
    unsigned int user_mode;
    unsigned int anr_intensity;
    unsigned int anr_smooth_level; //range???
    NR_CONVERGE_SPEED anr_converge_speed; //0 1 2 higer the speed more fast
}AudioAnrConfig;

/*EQ config structure*/
typedef struct{
    unsigned int eq_enable;
    unsigned int user_mode;
    short eq_gain_db[_EQ_BAND_NUM];

}AudioEqConfig;

/*HPF config structure*/
typedef struct{
    unsigned int hpf_enable;
    unsigned int user_mode;
    IAA_HPF_FREQ cutoff_frequency;
}AudioHpfConfig;

/*VAD config structure*/

typedef struct{
    unsigned int vad_enable;
    unsigned int user_mode;
    int vad_threshold;

}AudioVadConfig;

/*De-reverberation config structure*/
typedef struct{
    unsigned int dereverb_enable;
}AudioDereverbConfig;

/*APC init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_APC_SAMPLE_RATE sample_rate;
}AudioProcessInit;


/*AGC gain info*/
typedef struct
{
    int gain_max;  //gain maximum
    int gain_min;  //gain minimum
    int gain_init; //default gain (initial gain)
}AgcGainInfo;

/*AGC config structure*/
typedef struct
{
    unsigned int agc_enable;
    unsigned int user_mode;
    //gain setting
    AgcGainInfo gain_info;
    unsigned int drop_gain_max;

    //attack time, release time
    unsigned int attack_time;
    unsigned int release_time;

    //target level
    unsigned int compression_ratio;
    int target_level_db;

    // noise gate
    int noise_gate_db;
    unsigned int noise_gate_attenuation_db;

}AudioAgcConfig;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
// General purpose
unsigned int IaaApc_GetBufferSize(void);
APC_HANDLE IaaApc_Init(char* const working_buffer_address,AudioProcessInit *audio_process_init);
unsigned int IaaApc_Config(APC_HANDLE handle,
                         AudioAnrConfig *anr_config,
                         AudioEqConfig *eq_config,
                         AudioHpfConfig *hpf_config,
                         AudioVadConfig *vad_config,
                         AudioDereverbConfig *dereverb_config,
                         AudioAgcConfig *agc_config);

int IaaApc_Run(APC_HANDLE,short* pss_audio_in);
void IaaApc_Free(APC_HANDLE);
int IaaApc_GetLibVersion(unsigned short *ver_year,
                         unsigned short *ver_date,
                         unsigned short *ver_time);

int IaaApc_GetVadOut(APC_HANDLE handle);
unsigned int IaaApc_GetConfig(APC_HANDLE handle,
                         AudioProcessInit *audio_process_init,
                         AudioAnrConfig *anr_config,
                         AudioEqConfig *eq_config,
                         AudioHpfConfig *hpf_config,
                         AudioVadConfig *vad_config,
                         AudioDereverbConfig *dereverb_config,
                         AudioAgcConfig *agc_config);
#endif // #ifndef _AUDIOPROCESS_H_
