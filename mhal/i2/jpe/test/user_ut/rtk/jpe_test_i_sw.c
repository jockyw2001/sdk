#ifndef __VER_JPE__
#define __VER_JPE__
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sys_MsWrapper_cus_os_msg.h"
#include "sys_sys_isw_cli.h"
#include "hal_drv_util.h"

#include "cam_os_wrapper.h"

#include "drv_jpe_io.h"
#include "drv_jpe_io_st.h"
#include "drv_jpe_module.h"
#include "drv_jpe_enc.h"

#include "md5.h"
#include "util_pattern.h"

#define RED "\e[1;31m"
#define BLUE "\e[1;34m"
#define NORMAL "\e[0m"

typedef struct KMA_t
{
    void* user_ptr;
    unsigned long miu_addr;
    unsigned long phys_addr;
    unsigned long mem_size;
} KMA_t;

static KMA_t _gKmaFileIn_1;    // used for YUV RAW data
static KMA_t _gKmaFileOut_1;    // used for JPEG encoded data
static KMA_t _gKmaFileIn_2;    // used for YUV RAW data
static KMA_t _gKmaFileOut_2;    // used for JPEG encoded data

static MsTaskId_e _gTask1;
static MsTaskId_e _gTask2;

#define JPEG_TEST_COUNT   1
#define JPEG_TEST_SUCCESS 0
#define JPEG_TEST_FAIL    -1
#define JPEG_SCALE_FACTOR 100    // valid range is 1~100

// Definition for multi task
typedef union
{
    u32 userdata;
} Body_test_t;


#ifdef Body_t
#undef Body_t
#endif

#define Body_t Body_test_t

#include "stdmsg.ht"

#ifndef VM_MSG_DEF
#define VM_MSG_DEF(_ENUM_, _UNION_)		_ENUM_,
#endif
enum
{
#include "sys_MsWrapper_cus_os_message_define__.hi"
};


// YUV Test Data
typedef struct HashResult_t
{
    char pName[16];
    int  size;
    char pDigest[33];
} HashResult_t;

HashResult_t _gpYUVHash[] =
{
    { "CIF.nv12", 152064, "2e32b62eeb3feea39d0046d15ada1db9" },
    { "CIF.nv21", 152064, "34cf41b049437fbb681717ccda4e2332" },
    { "CIF.yuyv", 202752, "d0b623f5f4f35f131fb2fae18d47f3a0" },
    { "CIF.yvyu", 202752, "949d22105ef7a93492467fa1dc18887b" },

    { "VGA.nv12", 460800, "e5fb3e8491eb71de307fc7318a502753" },
    { "VGA.nv21", 460800, "0cf5b6a6e29f898b2e0f027e7b6a3d59" },
    { "VGA.yuyv", 614400, "ba04b608701c9ef4e34116e46cf2a983" },
    { "VGA.yvyu", 614400, "bafe471ea8445185d6b9050d75c5012a" },

    { "1088P.nv12", 3133440, "929b9895da1ac761669b7fc5751ed8eb" },
    { "1088P.nv21", 3133440, "566276a1043b8440be166d9f417ad6eb" },
    { "1088P.yuyv", 4177920, "8f3b83841950a7b068ea81bca1b22ef8" },
    { "1088P.yvyu", 4177920, "71d63f2f73c1a55fbe5c5d4b25253f13" },

    // TODO: make new sample for 2688x1536
    { "1536P.nv12", 3133440, "wrong hash value" },
    { "1536P.nv21", 3133440, "wrong hash value" },
    { "1536P.yuyv", 4177920, "wrong hash value" },
    { "1536P.yvyu", 4177920, "wrong hash value" },
};

HashResult_t _gpJpegBinHash[] =
{
    { "CIF.nv12", 3851, "b6ad3146d0863cf22f9edc9b15e1bbb7" },
    { "CIF.nv21", 3851, "b6ad3146d0863cf22f9edc9b15e1bbb7" },
    { "CIF.yuyv", 5376, "5dd1c1ed4f618e5e094f109acfcea61b" },
    { "CIF.yvyu", 5376, "5dd1c1ed4f618e5e094f109acfcea61b" },

    { "VGA.nv12", 9380, "b07682d407fefa06867d0564db356f4b" },
    { "VGA.nv21", 9380, "b07682d407fefa06867d0564db356f4b" },
    { "VGA.yuyv", 12401, "e31c3c1bb5b01e2e23a09b38225ec742" },
    { "VGA.yvyu", 12401, "e31c3c1bb5b01e2e23a09b38225ec742" },

    { "1088P.nv12", 46752, "050e81bdacbab019af6ffa07c9c44e53" },
    { "1088P.nv21", 46752, "050e81bdacbab019af6ffa07c9c44e53" },
    { "1088P.yuyv", 58892, "2868008e81f83f436aea57556652f8a4" },
    { "1088P.yvyu", 58892, "2868008e81f83f436aea57556652f8a4" },

    { "1536P.nv12", 46752, "wrong hash value" },
    { "1536P.nv21", 46752, "wrong hash value" },
    { "1536P.yuyv", 58892, "wrong hash value" },
    { "1536P.yvyu", 58892, "wrong hash value" },
};


#define DCTSIZE2 64
u16 _std_luminance_quant_tbl[DCTSIZE2] =
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

u16 _std_chrominance_quant_tbl[DCTSIZE2] =
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


typedef enum JPEG_TEST_CLOCK_e
{
    // clk-select = <0>; // 0: 288MHz  1: 216MHz  2: 54MHz  3: 27MHz
    eCLK_288 = 0,
    eCLK_216,
    eCLK_54,
    eCLK_27
} JPEG_TEST_CLOCK_e;


typedef enum JPEG_TEST_RESOLUTION_e
{
    eRESOLUTION_CIF = 0,
    eRESOLUTION_VGA,
    eRESOLUTION_1088P,      // 1920x1088
    eRESOLUTION_1536P,      // 2688x1536
} JPEG_TEST_RESOLUTION_e;

char *_gpResolutionName[] =
{
    "CIF",
    "VGA",
    "1080P"
};

typedef enum JPEG_TEST_FORMAT_e
{
    eNV12 = 0,
    eNV21,
    eYUYV,
    eYVYU
} JPEG_TEST_FORMAT_e;

struct bo
{
    void *ptrHead;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
};


static JpeDev_t* _gpTestDev;
static unsigned char _gDbgMsgFlag = 0;

#if 0
unsigned int GetHash(char *pFileName, unsigned char digest[])
{
    unsigned int nSize = 0;
    md5_state_t c;

    unsigned char *pBuffer = NULL;
    FILE *pFile;

    if(!pFileName)
        printf("pFileName is NULL\n");

    pFile = fopen(pFileName, "r");
    if(pFile)
    {
        fseek(pFile, 0L, SEEK_END);
        nSize = ftell(pFile);
        fseek(pFile, 0L, SEEK_SET);

        pBuffer = malloc(nSize);

        fread(pBuffer, 1, nSize, pFile);
        fclose(pFile);

        Md5Init(&c);
        Md5Append(&c, pBuffer, nSize);
        Md5Finish(&c, digest);

        free(pBuffer);
    }
    else
    {
        printf("fopen %s fail\n", pFileName);
    }
    return 1;
}
#endif


static void _PatternRelease(struct bo *pBo)
{
    MsReleaseMemory(pBo);
}

static struct bo *
_PatternCreate(unsigned int format,
               unsigned int width, unsigned int height,
               unsigned int handles[4], unsigned int pitches[4],
               unsigned int offsets[4], UtilFillPattern_e pattern, void* pMemory)
{
    unsigned int virtual_height;
    struct bo *pBo;
    unsigned int bpp;
    void *planes[3] = { 0, };
    void *virtual;

    switch(format)
    {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
            bpp = 8;
            break;

        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            bpp = 16;
            break;

        default:
            CamOsPrintf("unsupported format 0x%08x\n",  format);
            return NULL;
    }

    switch(format)
    {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
            virtual_height = height * 3 / 2;
            break;

        default:
            virtual_height = height;
            break;
    }

    pBo = CamOsMemCalloc(1, sizeof(struct bo));
    pBo->size = width * virtual_height * bpp / 8;
    pBo->ptr = (void *)((int)(pMemory));

    pBo->pitch = width * bpp / 8;
    pBo->offset = 0;

    virtual = pBo->ptr;

    /* just testing a limited # of formats to test single
     * and multi-planar path.. would be nice to add more..
     */
    switch(format)
    {
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            offsets[0] = 0;
            pitches[0] = pBo->pitch;
            planes[0] = virtual;
            break;

        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
            offsets[0] = 0;
            pitches[0] = pBo->pitch;
            pitches[1] = pitches[0];
            offsets[1] = pitches[0] * height;

            planes[0] = virtual;
            planes[1] = virtual + offsets[1];
            break;
    }

    UtilFillPattern(format, pattern, planes, width, height, pitches[0]);

    return pBo;
}

static void _DumpDigest(unsigned char *pDigest)
{
    int j;
    for(j = 0 ; j < 16; j++)
    {
        CamOsPrintf("%02x", pDigest[j]);
    }
}


static int _CompareDigest(char *pDigestStrIn, unsigned char *pDigestHex)
{
    unsigned char j, tmp;
    unsigned char pDigestStr[33];

    for(j = 0 ; j < 16; j++)
    {
        //sprintf(pDigestStr + j*2, "%02x", pDigestHex[j]);
        tmp = (pDigestHex[j] & 0xF0) >> 4;
        pDigestStr[2 * j] = tmp >= 0x0a ? tmp - 10 + 'a' : tmp + '0';

        tmp = (pDigestHex[j] & 0x0F);
        pDigestStr[2 * j + 1] = tmp >= 0x0a ? tmp - 10 + 'a' : tmp + '0';
    }

    return memcmp(pDigestStrIn, pDigestStr, 32);
}

static void _PrepareInputBuffer_1(void)
{
    int nSize;
    CamOsRet_e eRet = CAM_OS_OK;

    u32* pInUserPtr;
    u32  nInMiuAddr;
    u64  nlInPhysAddr;

    u32* pOutUserPtr;
    u32  nOutMiuAddr;
    u64  nlOutPhysAddr;

    nSize = 4177920;

    eRet = CamOsDirectMemAlloc("JPE_IN1",
                        nSize,
                        (void**)&pInUserPtr,
                        &nInMiuAddr,
                        &nlInPhysAddr);
    if(CAM_OS_OK != eRet)
    {
        CamOsPrintf("CamOsDirectMemAlloc fail\n");
    }

    eRet = CamOsDirectMemAlloc("JPE_OUT1",
                        nSize/2,
                        (void**)&pOutUserPtr,
                        &nOutMiuAddr,
                        &nlOutPhysAddr);
    if(CAM_OS_OK != eRet)
    {
        CamOsPrintf("CamOsDirectMemAlloc fail\n");
    }

    _gKmaFileIn_1.user_ptr    = pInUserPtr;
    _gKmaFileIn_1.phys_addr   = (unsigned long)nlInPhysAddr;
    _gKmaFileIn_1.mem_size    = (unsigned long)nSize;
    _gKmaFileOut_1.user_ptr    = pOutUserPtr;
    _gKmaFileOut_1.phys_addr   = (unsigned long)nlOutPhysAddr;
    _gKmaFileOut_1.mem_size    = (unsigned long)nSize / 2;
}


static void _PrepareInputBuffer_2(void)
{
    int nSize;
    CamOsRet_e eRet = CAM_OS_OK;

    u32* pInUserPtr;
    u32  nInMiuAddr;
    u64  nlInPhysAddr;

    u32* pOutUserPtr;
    u32  nOutMiuAddr;
    u64  nlOutPhysAddr;

    nSize = 4177920;

    eRet = CamOsDirectMemAlloc("JPE_IN2",
                        nSize,
                        (void**)&pInUserPtr,
                        &nInMiuAddr,
                        &nlInPhysAddr);
    if(CAM_OS_OK != eRet)
    {
        CamOsPrintf("CamOsDirectMemAlloc fail\n");
    }

    eRet = CamOsDirectMemAlloc("JPE_OUT2",
                        nSize/2,
                        (void**)&pOutUserPtr,
                        &nOutMiuAddr,
                        &nlOutPhysAddr);
    if(CAM_OS_OK != eRet)
    {
        CamOsPrintf("CamOsDirectMemAlloc fail\n");
    }

    _gKmaFileIn_2.user_ptr    = pInUserPtr;
    _gKmaFileIn_2.phys_addr   = (unsigned long)nlInPhysAddr;
    _gKmaFileIn_2.mem_size    = (unsigned long)nSize;
    _gKmaFileOut_2.user_ptr    = pOutUserPtr;
    _gKmaFileOut_2.phys_addr   = (unsigned long)nlOutPhysAddr;
    _gKmaFileOut_2.mem_size    = (unsigned long)nSize / 2;
}

static void _ReleaseInputBuffer_1(void)
{
    MsReleaseMemory(_gKmaFileIn_1.user_ptr);
    MsReleaseMemory(_gKmaFileOut_1.user_ptr);
}

static void _ReleaseInputBuffer_2(void)
{
    MsReleaseMemory(_gKmaFileIn_2.user_ptr);
    MsReleaseMemory(_gKmaFileOut_2.user_ptr);
}

static int _DoEncode(JPEG_TEST_CLOCK_e eClkSelect, JPEG_TEST_RESOLUTION_e nRes, JPEG_TEST_FORMAT_e eRawFormat, int bDebugMsg, int nTaskId)
{
    int  pFormat[] = {DRM_FORMAT_NV12, DRM_FORMAT_NV21, DRM_FORMAT_YUYV, DRM_FORMAT_YVYU};
    //char pSuffix[][5] = { "nv12", "nv21", "yuyv", "yvyu"};

    unsigned char pDigest[16];
    int nSize;
    int width = 320;
    int height = 240;

    unsigned int handles[4] = {0};
    unsigned int pitches[4] = {0};
    unsigned int offsets[4] = {0};
    UtilFillPattern_e ePattern = UTIL_PATTERN_SMPTE;

    md5_state_t tMd5State;
    struct bo *pBo;

    KMA_t *pKmaFileIn;    // used for YUV RAW data
    KMA_t *pKmaFileOut;    // used for JPEG encoded data

    if(_gTask1 == nTaskId)
    {
        _PrepareInputBuffer_1();
        pKmaFileIn = &_gKmaFileIn_1;
        pKmaFileOut = &_gKmaFileOut_1;
    }
    else
    {
        _PrepareInputBuffer_2();
        pKmaFileIn = &_gKmaFileIn_2;
        pKmaFileOut = &_gKmaFileOut_2;
    }

    switch(nRes)
    {
        case eRESOLUTION_CIF:
            width = 352;
            height = 288;
            break;

        case eRESOLUTION_VGA:
            width = 640;
            height = 480;
            break;

        case eRESOLUTION_1088P:
            width = 1920;
            height = 1088;
            break;

        case eRESOLUTION_1536P:
            width = 2688;
            height = 1536;
            break;

        default:
            CamOsPrintf("unknow resolution\n");
            return JPEG_TEST_FAIL;
            break;
    }

    // Step 1. generate pattern
    pBo = _PatternCreate(pFormat[eRawFormat], width, height, handles, pitches, offsets, ePattern, pKmaFileIn->user_ptr);
    if(bDebugMsg)
    {
        CamOsPrintf("Expected Pattern : size = %d, Hash = %s\n", _gpYUVHash[eRawFormat + 4 * nRes].size, _gpYUVHash[eRawFormat + 4 * nRes].pDigest);
        CamOsPrintf("Actual   Pattern : size = %d, ", pBo->size);
    }

    if(_gpYUVHash[eRawFormat + 4 * nRes].size != pBo->size)
    {
        CamOsPrintf("%sError!!!! Task(%d) Size(%d) mismatch with %d%s\n", RED, nTaskId, (int)(pBo->size), _gpYUVHash[eRawFormat + 4 * nRes].size, NORMAL);
    }

#if 1 // raw data hash is slow, so we only enable it when needed.
    memset(&tMd5State, sizeof(tMd5State), 0);
    Md5Init(&tMd5State);
    Md5Append(&tMd5State, pBo->ptr, pBo->size);
    Md5Finish(&tMd5State, pDigest);
    CamOsPrintf("Hash = ");
    _DumpDigest(pDigest);
    CamOsPrintf("\n\n");
#else
    CamOsPrintf("\n");
#endif

    // Step 2. RAW data to JPEG (without header)
    JpeParam_t  tParam;
    JpeCfg_t    *pJpeCfg = &tParam.tJpeCfg;

    memset(pJpeCfg, 0, sizeof(JpeCfg_t));

    pJpeCfg->eInBufMode = JPE_IBUF_FRAME_MODE;
    pJpeCfg->eCodecFormat = JPE_CODEC_JPEG;

    if(eNV12 == eRawFormat)  pJpeCfg->eRawFormat = JPE_RAW_NV12;
    else if(eNV21 == eRawFormat)  pJpeCfg->eRawFormat = JPE_RAW_NV21;
    else if(eYUYV == eRawFormat)  pJpeCfg->eRawFormat = JPE_RAW_YUYV;
    else if(eYVYU == eRawFormat)  pJpeCfg->eRawFormat = JPE_RAW_YVYU;

    pJpeCfg->nWidth = width;
    pJpeCfg->nHeight = height;
    pJpeCfg->nQScale = JPEG_SCALE_FACTOR;
    memcpy(pJpeCfg->YQTable, _std_luminance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));
    memcpy(pJpeCfg->CQTable, _std_chrominance_quant_tbl, DCTSIZE2 * sizeof(unsigned short));

    nSize = pBo->size;

    // Assign buffer for test
    pJpeCfg->OutBuf.nAddr = (unsigned long)pKmaFileOut->phys_addr;
    pJpeCfg->OutBuf.nSize = pKmaFileOut->mem_size;

    if(eYUYV == eRawFormat || eYVYU == eRawFormat)
    {
        pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned int)pKmaFileIn->phys_addr;
        pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nSize = nSize;
        pJpeCfg->InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = 0;
        pJpeCfg->InBuf[JPE_COLOR_PLAN_CHROMA].nSize = 0;
    }
    else if(eNV21 == eRawFormat || eNV12 == eRawFormat)
    {
        pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nAddr = (unsigned int)pKmaFileIn->phys_addr;
        pJpeCfg->InBuf[JPE_COLOR_PLAN_LUMA].nSize = width * height;
        pJpeCfg->InBuf[JPE_COLOR_PLAN_CHROMA].nAddr = (unsigned int)(pKmaFileIn->phys_addr + width * height);
        pJpeCfg->InBuf[JPE_COLOR_PLAN_CHROMA].nSize = nSize - width * height;
    }
    tParam.nClkSelect = (unsigned short)eClkSelect;
    JpeEncode(&tParam, _gpTestDev);

    // Step 3. generate hash for JPEG encoded data
    memset(&tMd5State, sizeof(tMd5State), 0);
    Md5Init(&tMd5State);
    // omit the end of JFIF (0xFFD9)
    Md5Append(&tMd5State, (const Md5Byte_t *)pJpeCfg->OutBuf.nAddr, (int)tParam.nEncodeSize - 2);
    Md5Finish(&tMd5State, pDigest);

    if(tParam.nEncodeSize > pJpeCfg->OutBuf.nSize)
    {
        CamOsPrintf("%sTask(%d) Error!!!! nEncodeSize(%d) > OutBufSize(%d). This should never happen!! \n", RED, nTaskId, (int)(tParam.nEncodeSize), pJpeCfg->OutBuf.nSize, NORMAL);
    }

    if(_gpJpegBinHash[eRawFormat + 4 * nRes].size != tParam.nEncodeSize - 2)
    {
        CamOsPrintf("%sTask(%d) Error!!!! Size(%d) mismatch with %d%s\n", RED, nTaskId, (int)(tParam.nEncodeSize - 2), _gpJpegBinHash[eRawFormat + 4 * nRes].size, NORMAL);
    }

    if(0 != _CompareDigest(_gpJpegBinHash[eRawFormat + 4 * nRes].pDigest, pDigest))
    {
        CamOsPrintf("%sTask(%d) Error!!!!  Digest(", RED, nTaskId);
        _DumpDigest(pDigest);
        CamOsPrintf(") mismatch with %s%s\n", _gpJpegBinHash[eRawFormat + 4 * nRes].pDigest, NORMAL);
    }
    else
    {
        if(bDebugMsg)
        {
            CamOsPrintf("Expected result size is %d\n", _gpJpegBinHash[eRawFormat + 4 * nRes].size);
            CamOsPrintf("Expected Hash is %s\n", _gpJpegBinHash[eRawFormat + 4 * nRes].pDigest);
            CamOsPrintf("Actual   Hash is ");
            _DumpDigest(pDigest);
            CamOsPrintf("\n");
        }
        CamOsPrintf("Encode done. Hash value for %dx%d check pass.\n\n", width, height);

    }

    _PatternRelease(pBo);

    if(_gTask1 == nTaskId)
    {
        _ReleaseInputBuffer_1();
    }
    else
    {
        _ReleaseInputBuffer_2();
    }
    return JPEG_TEST_SUCCESS;
}

static int _DoTest(JPEG_TEST_FORMAT_e eRawFormat, int bDebugMsg, int nTaskId)
{
    int nRet;
    JPEG_TEST_CLOCK_e eClkSelect = eCLK_288;
    JPEG_TEST_RESOLUTION_e eRes = eRESOLUTION_1088P;

    CamOsPrintf("====");
    switch(eRawFormat)
    {
        case eNV12:
            CamOsPrintf(" Raw format: eNV12 ");
            break;
        case eNV21:
            CamOsPrintf(" Raw format: eNV21 ");
            break;
        case eYUYV:
            CamOsPrintf(" Raw format: eYUYV ");
            break;
        case eYVYU:
            CamOsPrintf(" Raw format: eYVYU ");
            break;
    }
    CamOsPrintf(" ====\n");


    for(eClkSelect = eCLK_288; eClkSelect <= eCLK_27; eClkSelect++)
    {
        switch(eClkSelect)
        {
            case eCLK_288:
                CamOsPrintf("%s Task:%d Clock: eCLK_288 %s \n", BLUE, nTaskId, NORMAL);
                break;
            case eCLK_216:
                CamOsPrintf("%s Task:%d Clock: eCLK_216 %s \n", BLUE, nTaskId, NORMAL);
                break;
            case eCLK_54:
                CamOsPrintf("%s Task:%d Clock: eCLK_54  %s \n", BLUE, nTaskId, NORMAL);
                break;
            case eCLK_27:
                CamOsPrintf("%s Task:%d Clock: eCLK_27  %s \n", BLUE, nTaskId, NORMAL);
                break;
        }

        for(eRes = eRESOLUTION_CIF; eRes <= eRESOLUTION_1088P; eRes++)
        {
            switch(eRes)
            {
                case eRESOLUTION_CIF:
                    CamOsPrintf("%s Res: CIF %s \n", BLUE, NORMAL);
                    break;
                case eRESOLUTION_VGA:
                    CamOsPrintf("%s Res: VGA %s \n", BLUE, NORMAL);
                    break;
                case eRESOLUTION_1088P:
                    CamOsPrintf("%s Res: 1920x1088 %s \n", BLUE, NORMAL);
                    break;
                case eRESOLUTION_1536P:
                    CamOsPrintf("%s Res: 2688x1536 %s \n", BLUE, NORMAL);
                    break;
            }

            nRet = _DoEncode(eClkSelect, eRes, eRawFormat, bDebugMsg, nTaskId);
            if(JPEG_TEST_SUCCESS != nRet)
            {
                CamOsPrintf("%s Error!!!! doEncode for resolution (%d) error!! %s\n", RED, eRes, NORMAL);
                return eCLI_PARSE_INPUT_ERROR;
            }
        }
    }
    return eCLI_PARSE_OK;

}


static int _TestJpeDbgMsg(CLI_t *pCli, char *p)
{
    _gDbgMsgFlag = _gDbgMsgFlag ? 0 : 1;
    return eCLI_PARSE_OK;
}

static int _TestJpeYUYV(CLI_t *pCli, char *p)
{
    _gpTestDev = JpeProbe(_gpTestDev);
    _gTask1 = MsCurrTask();
    _DoTest(eYUYV, _gDbgMsgFlag, _gTask1);
    JpeRemove(_gpTestDev);
    return eCLI_PARSE_OK;
}

static int _TestJpeYVYU(CLI_t *pCli, char *p)
{
    _gpTestDev = JpeProbe(_gpTestDev);
    _gTask1 = MsCurrTask();
    _DoTest(eYVYU, _gDbgMsgFlag, _gTask1);
    JpeRemove(_gpTestDev);
    return eCLI_PARSE_OK;
}

static int _TestJpeNV12(CLI_t *pCli, char *p)
{
    _gpTestDev = JpeProbe(_gpTestDev);
    _gTask1 = MsCurrTask();
    _DoTest(eNV12, _gDbgMsgFlag, _gTask1);
    JpeRemove(_gpTestDev);
    return eCLI_PARSE_OK;
}

static int _TestJpeNV21(CLI_t *pCli, char *p)
{
    _gpTestDev = JpeProbe(_gpTestDev);
    _gTask1 = MsCurrTask();
    _DoTest(eNV21, _gDbgMsgFlag, _gTask1);
    JpeRemove(_gpTestDev);
    return eCLI_PARSE_OK;
}

static void _EmptyParser1(vm_msg_t *pMessage)
{
    int i;
    _gTask1 = MsCurrTask();
    for(i = 0; i < JPEG_TEST_COUNT; i++)
    {
        _DoTest(eYUYV, _gDbgMsgFlag, _gTask1);
    }
}

static void _EmptyParser2(vm_msg_t *pMessage)
{
    int i;
    _gTask2 = MsCurrTask();
    for(i = 0; i < JPEG_TEST_COUNT; i++)
    {
        _DoTest(eNV12, _gDbgMsgFlag, _gTask2);
    }
}

static void _EmptyInit(void *userdata)
{
    CamOsPrintf("%s %d\n", __FUNCTION__, __LINE__);
}

static int _TestJpeMultiTask(CLI_t *pCli, char *p)
{
    _gpTestDev = JpeProbe(_gpTestDev);
    Msg_t *msg = NULL;
    MsTaskId_e JpeTaskid[2];
    MsTaskCreateArgs_t JpeTaskArgs[2] =
    {
        {54, 0, NULL, _EmptyInit, _EmptyParser1, NULL, &JpeTaskid[0], NULL, FALSE, "task0", RTK_MS_TO_TICK(50), RTK_MS_TO_TICK(50)},
        {54, 0, NULL, _EmptyInit, _EmptyParser2, NULL, &JpeTaskid[1], NULL, FALSE, "task1", RTK_MS_TO_TICK(50), RTK_MS_TO_TICK(50)},
    };
    JpeTaskArgs[0].AppliInit = _EmptyInit;
    JpeTaskArgs[0].pInitArgs = (void *)_gpTestDev;
    JpeTaskArgs[0].AppliParser = _EmptyParser1;
    JpeTaskArgs[0].StackSize = 4096;
    JpeTaskArgs[0].pStackTop = (u32*)MsAllocateMem(JpeTaskArgs[0].StackSize);
    strcpy(JpeTaskArgs[0].TaskName, "task0");

    JpeTaskArgs[1].AppliInit = _EmptyInit;
    JpeTaskArgs[1].pInitArgs = (void *)_gpTestDev;
    JpeTaskArgs[1].AppliParser = _EmptyParser2;
    JpeTaskArgs[1].StackSize = 4096;
    JpeTaskArgs[1].pStackTop = (u32*)MsAllocateMem(JpeTaskArgs[0].StackSize);
    strcpy(JpeTaskArgs[0].TaskName, "task1");

    MsCreateTask(&JpeTaskArgs[0]);
    MsStartTask(JpeTaskid[0]);
    MsCreateTask(&JpeTaskArgs[1]);
    MsStartTask(JpeTaskid[1]);

    CamOsPrintf("%s JpeTaskid[0]=%d, JpeTaskid[1]=%d\n", BLUE, JpeTaskid[0], JpeTaskid[1], NORMAL);

    msg = (Msg_t*)MsAllocateMem(sizeof(Header_t));
    msg->Header.TypMsg = KERNEL_TEST_MSG0;
    msg->Header.MbxSrc = CUS14_MBX;
    msg->Header.MbxDst = JpeTaskid[0];
    msg->Header.Length = 0;
    MsSend(JpeTaskid[0], msg);

    msg = (Msg_t*)MsAllocateMem(sizeof(Header_t));
    msg->Header.TypMsg = KERNEL_TEST_MSG1;
    msg->Header.MbxSrc = CUS14_MBX;
    msg->Header.MbxDst = JpeTaskid[1];
    msg->Header.Length = 0;
    MsSend(JpeTaskid[1], msg);

    MsSleepExt(RTK_MS_TO_TICK(10 * JPEG_TEST_COUNT * 1000));

    CamOsPrintf("%s End of %s %s\n", BLUE, __func__, NORMAL);
    JpeRemove(_gpTestDev);

    return eCLI_PARSE_OK;
}

// g_atJPEMenuTbl will be used in isw_cli_main.c
CliParseToken_t g_atJPEMenuTbl[] =
{
    {"0",    "trun on/off debug message",               "", _TestJpeDbgMsg,     NULL},
    {"1",    "encode yuvu to jpeg (CIF, VGA, 1088P)",   "", _TestJpeYUYV,       NULL},
    {"2",    "encode yvvu to jpeg (CIF, VGA, 1088P)",   "", _TestJpeYVYU,       NULL},
    {"3",    "encode nv12 to jpeg (CIF, VGA, 1088P)",   "", _TestJpeNV12,       NULL},
    {"4",    "encode nv21 to jpeg (CIF, VGA, 1088P)",   "", _TestJpeNV21,       NULL},
    {"5",    "encode with 2 tasks",                     "", _TestJpeMultiTask,  NULL},
    PARSE_TOKEN_DELIMITER
};