#ifndef AUDIOSRCPROCESS_H_
#define AUDIOSRCPROCESS_H_


typedef enum{
    SRC_8k_to_16k,
    SRC_8k_to_32k,
    SRC_8k_to_48k,
    SRC_16k_to_8k,
    SRC_16k_to_32k,
    SRC_16k_to_48k,
    SRC_32k_to_8k,
    SRC_32k_to_16k,
    SRC_32k_to_48k,
    SRC_48k_to_8k,
    SRC_48k_to_16k,
    SRC_48k_to_32k
}SrcConversionMode;

typedef enum{
    SRATE_8K  =  8,
    SRATE_16K =  16,
    SRATE_32K =  32,
    SRATE_48K =  48
}SrcInSrate;

typedef void* SRC_HANDLE;

typedef struct{
    SrcInSrate WaveIn_srate;
    SrcConversionMode mode;
    unsigned int channel;
    unsigned int point_number;
}SRCStructProcess;

unsigned int IaaSrc_GetBufferSize(SrcConversionMode mode);
SRC_HANDLE IaaSrc_Init(char *workingBufferAddress, SRCStructProcess *src_struct);
int IaaSrc_Run(SRC_HANDLE handle, short *audio_input, short *audio_output, int Npoints);
int IaaSrc_Release(SRC_HANDLE handle);

#endif /* AUDIOSRCPROCESS_H_ */
