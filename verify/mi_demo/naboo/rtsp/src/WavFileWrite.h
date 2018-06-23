#ifndef _WAVFILEWRITE_H_
#define _WAVFILEWRITE_H_

#ifdef __cplusplus 
extern "C" { 
#endif

typedef enum _MediaType_e
{
	MT_UNKNOWN = -1,
    MT_PCM   = 0x01,
    MT_G711A = 0x06,
    MT_G711U = 0x07,
	MT_ADPCM = 0x11,
    MT_G726  = 0x45,
    MT_NUM
}MediaType_e;


int wav_open(void** pHandle, const char *fileName, int sample_rate, int channels, int bit_wide, MediaType_e type);
int wav_write(void *handle, char* data, int iLen);
int wav_open_fseek(void *handle, long Offset, int Origin);
int wav_close(void** pHandle);
int wav_read_head(FILE* fp_wav, MediaType_e* playload, int* iSamples, int* iChannel, int* iBitWide);

#ifdef __cplusplus 
} 
#endif

#endif //_WAVFILEWRITE_H_