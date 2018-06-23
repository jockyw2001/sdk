
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "drv_jpe_io.h"
#include "drv_jpe_io_st.h"
#include "jpegenc_marker.h"
#include "cam_os_wrapper.h"
#include "cam_os_util_list.h"
#include "NV12_qvga_10frame.h"

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))

#define SPECVERSIONMAJOR 1
#define SPECVERSIONMINOR 0
#define SPECREVISION 0
#define SPECSTEP 0

#define DCTSIZE2 64
u16 std_luminance_quant_tbl[DCTSIZE2] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};

u16 std_chrominance_quant_tbl[DCTSIZE2] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

void print_hex(char* title, void* buf, int num) {
    int i;
    char *data = (char *) buf;

    CamOsPrintf(
                    "%s\nOffset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
                    "----------------------------------------------------------",
                    title);
    for (i = 0; i < num; i++) {
            if (i % 16 == 0)
            {
                //CamOsPrintf("\n%08X   ", i);
                CamOsPrintf("\n");
            }
            CamOsPrintf("%02X ", data[i]);
    }
    CamOsPrintf("\n");
}

#define JPE_ENC_WIDTH   320
#define JPE_ENC_HEIGHT  240
#define JPEG_SCALE_FACTOR 100

static int drv_jpegenc_encoder_HwJpegMarkerInit(void *pOutputBuf, int nBufSize, JpeCfg_t *pCInfo)
{
    struct CamOsListHead_t head;
    u32 size;
    void* data;
    u32 yuv422;

    switch(pCInfo->eRawFormat)
    {
    case JPE_RAW_YUYV:
    case JPE_RAW_YVYU:
        yuv422 = 1;
        break;

    case JPE_RAW_NV12:
    case JPE_RAW_NV21:
        yuv422 = 0;
        break;

    default:
        CamOsPrintf("Unknown RAW format\n");
        return 0;
    }

    /* Initialize & create baseline marker */
    drv_jpegenc_marker_init(&head);
    drv_jpegenc_marker_create_baseline(&head, yuv422, pCInfo->nWidth, pCInfo->nHeight, pCInfo->YQTable, pCInfo->CQTable, pCInfo->nQScale);

#if 1
    /* Add dummy app 15 for alignment */
    size = 0x10 - ((u32)(pOutputBuf + drv_jpegenc_marker_size(&head)) & 0xF);

    if(size)
    {
        if(size < 4)
            size += 0x10 - 4;
        else
            size -= 4;

        CamOsPrintf("In %s() Dummy app 15 size is %ld\n", __func__, size);

        data = drv_jpegenc_marker_create_app_n(&head, JPEG_MARKER_APP15, size);
        memset(data, 0x00, size);
    }
#endif

    /* Check necessary size of marker, we do not expect that buffer size is smaller than marker size. */
    size = drv_jpegenc_marker_size(&head);
    if(nBufSize < size)
        return 0;

    /* Dump markers to the output buffer */
    size = drv_jpegenc_marker_dump(&head, pOutputBuf);

    /* free maker */
    drv_jpegenc_marker_release(&head);

    CamOsPrintf("Out %s() Total header size is %ld\n", __func__, size);

    return size;
}

int main(int argc, char** argv)
{
    int jpefd = 0;
    void* pYuvBufVitr = NULL;
    void* pYuvBufPhys = NULL;
    void* pYuvBufMiu = NULL;
    void* pBsBufVitr = NULL;
    void* pBsBufPhys = NULL;
    void* pBsBufMiu = NULL;
    unsigned int YUV_length = JPE_ENC_WIDTH*JPE_ENC_HEIGHT*2;
    JpeCfg_t JpeCfg;
    int nRet = JPE_IOC_RET_SUCCESS;
    //JpeCaps_t tCaps;
    //unsigned long nClkSelect = 0;
    JpeBufInfo_t tOutBuf;
    JpeBitstreamInfo_t tBitInfo;
    u32 nHeaderSize;
    FILE *pFile;

    CamOsPrintf("JPE UT start\n");

    jpefd = open("/dev/mstar_jpe0", O_RDWR);
    CamOsDirectMemAlloc("JPE-IBUFF", YUV_length, &pYuvBufVitr, &pYuvBufPhys, &pYuvBufMiu);
    if(pYuvBufVitr == NULL || pYuvBufPhys == NULL)
    {
        CamOsPrintf("Alloc Input Buf error\n");
    }
    memcpy(pYuvBufVitr, gYUV, JPE_ENC_WIDTH*JPE_ENC_HEIGHT*3/2);

    CamOsDirectMemAlloc("JPE-OBUFF", YUV_length, &pBsBufVitr, &pBsBufPhys, &pBsBufMiu);
    if(pBsBufVitr == NULL || pBsBufPhys == NULL)
    {
        CamOsPrintf("Alloc Output Buf error\n");
    }
    memset(pBsBufVitr, 0x00, YUV_length);

    /*nRet = ioctl(jpefd, JPE_IOC_GET_CAPS, &tCaps);
    if(nRet != JPE_IOC_RET_SUCCESS)
    {
        CamOsPrintf("> JPE_IOC_GET_CAPS Fail , err %d\n", nRet);
    }*/


    JpeCfg.eInBufMode       = JPE_IBUF_FRAME_MODE;
    JpeCfg.eRawFormat       = JPE_RAW_NV12;
    JpeCfg.eCodecFormat     = JPE_CODEC_JPEG;
    JpeCfg.nWidth           = JPE_ENC_WIDTH;
    JpeCfg.nHeight          = JPE_ENC_HEIGHT;
    memcpy(JpeCfg.YQTable, std_luminance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));
    memcpy(JpeCfg.CQTable, std_chrominance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));
    JpeCfg.nQScale          = JPEG_SCALE_FACTOR;
    if(JPE_RAW_YUYV == JpeCfg.eRawFormat || JPE_RAW_YVYU == JpeCfg.eRawFormat)
    {
        JpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned int)pYuvBufMiu;
        JpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nSize = JPE_ENC_WIDTH*JPE_ENC_HEIGHT*2;
        JpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = 0;
        JpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nSize = 0;
    }
    else if(JPE_RAW_NV12 == JpeCfg.eRawFormat || JPE_RAW_NV21 == JpeCfg.eRawFormat)
    {
        JpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned int)pYuvBufMiu;
        JpeCfg.InBuf[JPE_COLOR_PLAN_LUMA].nSize = JPE_ENC_WIDTH*JPE_ENC_HEIGHT;
        JpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = (unsigned int)(pYuvBufMiu + JPE_ENC_WIDTH*JPE_ENC_HEIGHT);
        JpeCfg.InBuf[JPE_COLOR_PLAN_CHROMA].nSize = JPE_ENC_WIDTH*JPE_ENC_HEIGHT/2;
    }

    nHeaderSize = drv_jpegenc_encoder_HwJpegMarkerInit(pBsBufVitr, YUV_length, &JpeCfg);

    JpeCfg.OutBuf.nAddr = (unsigned int)pBsBufMiu+nHeaderSize;
    JpeCfg.OutBuf.nSize = YUV_length-nHeaderSize;
    JpeCfg.nJpeOutBitOffset = 0;


    nRet = ioctl(jpefd, JPE_IOC_INIT, &JpeCfg);
    if(nRet != JPE_IOC_RET_SUCCESS)
    {
        CamOsPrintf("> JPE_IOC_INIT Fail , err %d\n", nRet);
    }


    /*nClkSelect = 0;
    nRet = ioctl(jpefd, JPE_IOC_SET_CLOCKRATE, nClkSelect);
    if(nRet != JPE_IOC_RET_SUCCESS)
    {
        CamOsPrintf("> JPE_IOC_SET_CLOCKRATE Fail , err %d\n", nRet);
    }*/


    tOutBuf.nAddr = JpeCfg.OutBuf.nAddr;
    tOutBuf.nSize = JpeCfg.OutBuf.nSize;
    nRet = ioctl(jpefd, JPE_IOC_SET_OUTBUF, &tOutBuf);
    if(nRet != JPE_IOC_RET_SUCCESS)
    {
        CamOsPrintf("> JPE_IOC_SET_OUTBUF Fail , err %d\n", nRet);
    }


    nRet = ioctl(jpefd, JPE_IOC_ENCODE_FRAME, NULL);
    if(nRet != JPE_IOC_RET_SUCCESS)
    {
        CamOsPrintf("> JPE_IOC_ENCODE_FRAME Fail , err %d\n", nRet);
    }


    nRet = ioctl(jpefd, JPE_IOC_GETBITS, &tBitInfo);
    if (nRet)
    {
        CamOsPrintf("%s: JPE_IOC_GETBITS error(%d)\n", __func__, nRet);
    }
    else
    {
        CamOsPrintf("%p %d %d %d\n",
                    tBitInfo.nAddr,
                    tBitInfo.nOrigSize,
                    tBitInfo.nOutputSize,
                    tBitInfo.eState);
        //print_hex("JPE output", (void *)pBsBufVitr, tBitInfo.nOutputSize+nHeaderSize);

        pFile = fopen( "jpe_ut.jpg","w" );
        if ( NULL == pFile ){
            CamOsPrintf("Open Jpeg Output File Fail\n");
        }
        else
        {
            fwrite(pBsBufVitr, 1, tBitInfo.nOutputSize+nHeaderSize, pFile);
            fclose(pFile);
        }
    }

    CamOsDirectMemRelease(pYuvBufVitr, YUV_length);
    CamOsDirectMemRelease(pBsBufVitr, YUV_length);

    if (jpefd > 0)  close(jpefd);

    CamOsPrintf("JPE UT end\n");

    return 0;
}
