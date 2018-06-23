#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/compat.h>

#include "MsTypes.h"
#include "utopia.h"
#include "apiMFE_priv.h"
#include "apiMFE_v2.h"

#include "utopia_adp.h"
#include "MFE_adp.h"

#define MFE_OBUF_1080P_SIZE (2 * 1024 * 1024)

//Function parameter
UADP_STRUCT_POINTER_TABLE spt_MFE_INIT[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_ENCODE[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_GETVOL[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_COMPRESSONEPICTURE[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_GETOUTBUFFER[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_GETOUTPUTINFO[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_SETVUI[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_DEINIT[10];

//not verify
UADP_STRUCT_POINTER_TABLE spt_MFE_SET_COLORFORMAT[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_SET_BITRATE_FRAMERATE[10];


//Function parameter's pointer
//spt_MFE_INIT
UADP_STRUCT_POINTER_TABLE spt_MFE_PVR_Info[10];
UADP_STRUCT_POINTER_TABLE spt_MFE_BITSTREAM_BUFFER[10];
//spt_MFE_GETVOL
UADP_STRUCT_POINTER_TABLE spt_MFE_header_info[10];
//spt_MFE_GETOUTBUFFER
UADP_STRUCT_POINTER_TABLE spt_MFE_OUTBUF[10];


#ifdef CONFIG_COMPAT

typedef struct
{
    compat_uptr_t miuPointer;
    compat_u64    miuAddress;
    compat_uint_t size;
} MEMMAP_32;

typedef struct
{
    MEMMAP_32 Cur_Y0;
    MEMMAP_32 Cur_C0;
    MEMMAP_32 Cur_Y1;
    MEMMAP_32 Cur_C1;
} MEMMAP_CUR_32;

typedef struct
{
    compat_u64    miu_offset;
    compat_u64    mfe_phy_buf_addr;
    compat_uint_t mfe_vir_buf_ptr;
    compat_uint_t mfe_phy_buf_size;
    compat_uint_t Encode_stop;
} MFE_API_INFO_32;

typedef struct
{
    compat_uint_t low_bandwidth_en;
    compat_uint_t input_imi_en;
    MEMMAP_t imi_buf_addr;
} MFE_ADV_INFO_32;

typedef struct
{
    u8 crop_en;
    compat_uint_t crop_top;
    compat_uint_t crop_bottom;
    compat_uint_t crop_left;
    compat_uint_t crop_right;
} MFE_CROP_INFO_32;

typedef struct
{
    compat_uptr_t pHeaderSPS;
    compat_uint_t SPSLen;
    compat_uptr_t pHeaderPPS;
    compat_uint_t PPSLen;
} SPSPPS_INFO_32;

typedef struct
{
    // MFE-compliant
    compat_int_t  width;
    compat_int_t  height;
    compat_u64    BitsBuffStart;
    compat_uint_t BitsBuffSize;
    compat_int_t  nOutBuffer;
    compat_int_t  OutBufferSize;
    MEMMAP_32     InBuffer;
    MEMMAP_32     BitstreamOutBuffer;
    compat_int_t  BitstreamOutBufferSize;

    // MFE new settings
    compat_int_t  nCodecType;
    compat_int_t  nDispWidth, nDispHeight;
    compat_int_t  nBufWidth, nBufHeight;
    compat_int_t  bInterlace;
    compat_int_t  bColorFormat;
    // GOP
    compat_int_t  nPbetweenI;
    compat_int_t  nBbetweenP;
    compat_int_t  nP_is_infinite;
    // Rate control
    compat_int_t  FrameRatex100;
    compat_int_t  nBitrate;
    compat_int_t  m_bFrameMode;
    compat_int_t  out_frametag_top;
    // Video length
    compat_int_t  nTotalFrames;
    compat_int_t  TimeIncreamentRes;
    compat_int_t  VopTimeIncreament;
    //jpeg parameter
    compat_int_t  quality;
    compat_int_t  fdc_mode;
    compat_int_t  exif_enable;
    compat_int_t  MFEClk;

    MFE_API_INFO_32 MfeApiInfo;
    MFE_ADV_INFO_32 MfeAdvInfo;

    compat_uint_t nDropFlag;
    compat_uint_t nSkipFlag;
    compat_uint_t enableVTRateControl;
    compat_uint_t enableSecurityMode;
    compat_uint_t enableISR;
    compat_uint_t enableCABAC;

    MFE_CROP_INFO_32 MfeCropInfo;
    compat_uint_t enableReduceBW;
    compat_uint_t enableFastMode;
    compat_int_t  mfe_state;
    compat_uptr_t g_pCustomCtx;
    compat_uptr_t g_StreamCallBackFunc;
    compat_uptr_t g_CleanCallBackFunc;

    compat_uptr_t mfe_drvHandle;

    compat_int_t reserved0;
    compat_int_t reserved1;
    compat_int_t reserved2;
    compat_int_t reserved3;
    compat_int_t reserved4;
    compat_int_t reserved5;
} PVR_Info_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    u8            retVal;
} MFE_INIT_PARAM_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    compat_uptr_t header_info;
    u8            retVal;
} MFE_GETVOL_PARAM_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    MEMMAP_CUR_32 YUVPlane;
    u8            bForceIframe;
    u8            retVal;
} MFE_COMPRESSONEPICTURE_PARAM_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    compat_uptr_t outbuf;
    u8            retVal;
} MFE_GET_OBUF_PARAM_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    compat_int_t  frametype;
    compat_int_t  size;
    u8            retVal;
} MFE_GETOUTPUT_PARAM_32;

typedef struct
{
    compat_uptr_t mfe_Info;
    compat_uint_t setLevel;
    u8            setVUI_aspect_ratio_info_present_flag;
    compat_uint_t setVUI_aspect_ratio_idc;
    compat_uint_t setVUI_sar_width;
    compat_uint_t setVUI_sar_height;
    u8            retVal;
} MFE_SETVUI_PARAM_32;


void MFE_adp_Compat_Copy_Info(PVR_Info_32 __user *pinfo32, PVR_Info *info)
{
    info->width = pinfo32->width;
    info->height = pinfo32->height;
    info->BitsBuffStart = pinfo32->BitsBuffStart;

    info->BitsBuffSize = pinfo32->BitsBuffSize;
    info->nOutBuffer = pinfo32->nOutBuffer;
    info->OutBufferSize = pinfo32->OutBufferSize;
    info->InBuffer.miuPointer = compat_ptr(pinfo32->InBuffer.miuPointer);
    info->InBuffer.miuAddress = pinfo32->InBuffer.miuAddress;
    info->InBuffer.size = pinfo32->InBuffer.size;

    info->BitstreamOutBuffer.miuPointer = compat_ptr(pinfo32->BitstreamOutBuffer.miuPointer);
    info->BitstreamOutBuffer.miuAddress = pinfo32->BitstreamOutBuffer.miuAddress;
    info->BitstreamOutBuffer.size = pinfo32->BitstreamOutBuffer.size;
    info->BitstreamOutBufferSize = pinfo32->BitstreamOutBufferSize;

    info->nCodecType = pinfo32->nCodecType;
    info->nDispWidth = pinfo32->nDispWidth;
    info->nDispHeight = pinfo32->nDispHeight;
    info->nBufWidth = pinfo32->nBufWidth;
    info->nBufHeight = pinfo32->nBufHeight;
    info->bInterlace = pinfo32->bInterlace;
    info->bColorFormat = pinfo32->bColorFormat;
    info->nPbetweenI = pinfo32->nPbetweenI;
    info->nBbetweenP = pinfo32->nBbetweenP;
    info->nP_is_infinite = pinfo32->nP_is_infinite;
    info->FrameRatex100 = pinfo32->FrameRatex100;
    info->nBitrate = pinfo32->nBitrate;
    info->m_bFrameMode = pinfo32->m_bFrameMode;
    info->out_frametag_top = pinfo32->out_frametag_top;
    info->nTotalFrames = pinfo32->nTotalFrames;
    info->TimeIncreamentRes = pinfo32->TimeIncreamentRes;
    info->VopTimeIncreament = pinfo32->VopTimeIncreament;
    info->quality = pinfo32->quality;
    info->fdc_mode = pinfo32->fdc_mode;
    info->exif_enable = pinfo32->exif_enable;
    info->MFEClk = pinfo32->MFEClk;

    info->MfeApiInfo.miu_offset = pinfo32->MfeApiInfo.miu_offset;
    info->MfeApiInfo.mfe_phy_buf_addr = pinfo32->MfeApiInfo.mfe_phy_buf_addr;
    info->MfeApiInfo.mfe_vir_buf_ptr = pinfo32->MfeApiInfo.mfe_vir_buf_ptr;
    info->MfeApiInfo.mfe_phy_buf_size = pinfo32->MfeApiInfo.mfe_phy_buf_size;
    info->MfeApiInfo.Encode_stop = pinfo32->MfeApiInfo.Encode_stop;

    info->MfeAdvInfo.low_bandwidth_en = pinfo32->MfeAdvInfo.low_bandwidth_en;
    info->MfeAdvInfo.input_imi_en = pinfo32->MfeAdvInfo.input_imi_en;
    info->MfeAdvInfo.imi_buf_addr.miuPointer = compat_ptr(pinfo32->MfeAdvInfo.imi_buf_addr.miuPointer);
    info->MfeAdvInfo.imi_buf_addr.miuAddress = pinfo32->MfeAdvInfo.imi_buf_addr.miuAddress;
    info->MfeAdvInfo.imi_buf_addr.size = pinfo32->MfeAdvInfo.imi_buf_addr.size;

    info->nDropFlag = pinfo32->nDropFlag;
    info->nSkipFlag = pinfo32->nSkipFlag;
    info->enableVTRateControl = pinfo32->enableVTRateControl;
    info->enableSecurityMode = pinfo32->enableSecurityMode;
    info->enableISR = pinfo32->enableISR;
    info->enableCABAC = pinfo32->enableCABAC;

    info->MfeCropInfo.crop_en = pinfo32->MfeCropInfo.crop_en;
    info->MfeCropInfo.crop_top = pinfo32->MfeCropInfo.crop_top;
    info->MfeCropInfo.crop_bottom = pinfo32->MfeCropInfo.crop_bottom;
    info->MfeCropInfo.crop_left = pinfo32->MfeCropInfo.crop_left;
    info->MfeCropInfo.crop_right = pinfo32->MfeCropInfo.crop_right;

    info->enableReduceBW = pinfo32->enableReduceBW;
    info->enableFastMode = pinfo32->enableFastMode;
    info->mfe_state = pinfo32->mfe_state;
    info->g_pCustomCtx = compat_ptr(pinfo32->g_pCustomCtx);
    info->g_StreamCallBackFunc = compat_ptr(pinfo32->g_StreamCallBackFunc);
    info->g_CleanCallBackFunc = compat_ptr(pinfo32->g_CleanCallBackFunc);

    info->mfe_drvHandle = (void *)((((MS_U64)pinfo32->reserved5) << 32) | ((MS_U64)pinfo32->mfe_drvHandle));

    info->reserved0 = pinfo32->reserved0;
    info->reserved1 = pinfo32->reserved1;
    info->reserved2 = pinfo32->reserved2;
    info->reserved3 = pinfo32->reserved3;
    info->reserved4 = pinfo32->reserved4;
    info->reserved5 = pinfo32->reserved5;
}

#endif // CONFIG_COMPAT


MS_U32 MFE_adp_Init(FUtopiaIOctl* pIoctl)
{
    //set table
    printk("MFE_adp_Init\n");

    UADP_SPT_BGN( &spt_MFE_INIT[0], sizeof(MFE_INIT_PARAM));
    UADP_SPT_NXT( &spt_MFE_INIT[1], MFE_INIT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_INIT[2]);

    UADP_SPT_BGN( &spt_MFE_ENCODE[0], sizeof(MFE_INIT_PARAM));
    UADP_SPT_NXT( &spt_MFE_ENCODE[1], MFE_INIT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_ENCODE[2]);

    UADP_SPT_BGN( &spt_MFE_DEINIT[0], sizeof(MFE_INIT_PARAM));
    UADP_SPT_NXT( &spt_MFE_DEINIT[1], MFE_INIT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_DEINIT[2]);

    UADP_SPT_BGN( &spt_MFE_GETVOL[0], sizeof(MFE_GETVOL_PARAM));
    UADP_SPT_NXT( &spt_MFE_GETVOL[1], MFE_GETVOL_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_GETVOL[2]);

    UADP_SPT_BGN( &spt_MFE_SET_COLORFORMAT[0], sizeof(MFE_INIT_PARAM));
    UADP_SPT_NXT( &spt_MFE_SET_COLORFORMAT[1], MFE_INIT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_SET_COLORFORMAT[2]);

    UADP_SPT_BGN( &spt_MFE_SET_BITRATE_FRAMERATE[0], sizeof(MFE_INIT_PARAM));
    UADP_SPT_NXT( &spt_MFE_SET_BITRATE_FRAMERATE[1], MFE_INIT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_SET_BITRATE_FRAMERATE[2]);

    UADP_SPT_BGN( &spt_MFE_COMPRESSONEPICTURE[0], sizeof(MFE_COMPRESSONEPICTURE_PARAM));
    UADP_SPT_NXT( &spt_MFE_COMPRESSONEPICTURE[1], MFE_COMPRESSONEPICTURE_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_COMPRESSONEPICTURE[2]);

    UADP_SPT_BGN( &spt_MFE_GETOUTBUFFER[0], sizeof(MFE_GET_OBUF_PARAM));
    UADP_SPT_NXT( &spt_MFE_GETOUTBUFFER[1], MFE_GET_OBUF_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_NXT( &spt_MFE_GETOUTBUFFER[2], MFE_GET_OBUF_PARAM, outbuf, spt_MFE_OUTBUF);
    UADP_SPT_FIN( &spt_MFE_GETOUTBUFFER[3]);

    UADP_SPT_BGN( &spt_MFE_GETOUTPUTINFO[0], sizeof(MFE_GETOUTPUT_PARAM));
    UADP_SPT_NXT( &spt_MFE_GETOUTPUTINFO[1], MFE_GETOUTPUT_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_GETOUTPUTINFO[2]);

    UADP_SPT_BGN( &spt_MFE_SETVUI[0], sizeof(MFE_SETVUI_PARAM));
    UADP_SPT_NXT( &spt_MFE_SETVUI[1], MFE_SETVUI_PARAM, mfe_Info, spt_MFE_PVR_Info);
    UADP_SPT_FIN( &spt_MFE_SETVUI[2]);

    UADP_SPT_BGN( &spt_MFE_PVR_Info[0], sizeof(PVR_Info));
    UADP_SPT_FIN( &spt_MFE_PVR_Info[1]);

    UADP_SPT_BGN(&spt_MFE_header_info[0], sizeof(SPSPPS_INFO_t));
    UADP_SPT_FIN(&spt_MFE_header_info[1]);

    UADP_SPT_BGN(&spt_MFE_OUTBUF[0], sizeof(MEMMAP_t));
    UADP_SPT_FIN(&spt_MFE_OUTBUF[1]);

    *pIoctl= (FUtopiaIOctl)MFE_adp_Ioctl;
    printk("MFE_adp_Init done\n");
    return 0;

}

MS_U32 MFE_adp_Ioctl(void *pInstanceTmp, MS_U32 u32Cmd, void *const pArgs)
{
    MS_U32 u32Ret = 0xFF;
    char buffer_arg[1];

    if (!access_ok(VERIFY_WRITE, (void *)pArgs, sizeof(*pArgs)))
    {
        printk("[%s] with bad address with cmd 0x%x!\n", __func__, (unsigned int)u32Cmd);
        return -EFAULT;
    }

    //printk("MFE_adp_Ioctl ctrl = 0x%x\n", (unsigned int)u32Cmd);

#ifdef CONFIG_COMPAT
    PVR_Info_32 __user *pinfo32 = NULL;
    MFE_INIT_PARAM_32 __user *pinit_param32 = NULL;
    MFE_GETVOL_PARAM_32 __user *pgetvol_param32 = NULL;
    MFE_COMPRESSONEPICTURE_PARAM_32 __user *pcompone_param32 = NULL;
    MFE_GET_OBUF_PARAM_32 __user *pgetobuf_param32 = NULL;
    MFE_GETOUTPUT_PARAM_32 __user *pgetoutput_param32 = NULL;
    MFE_SETVUI_PARAM_32 __user *psetvui_param32 = NULL;

    MFE_INIT_PARAM init_param = { 0 };
    MFE_GETVOL_PARAM getvol_param = { 0 };
    MFE_COMPRESSONEPICTURE_PARAM compone_param = { 0 };
    MFE_GET_OBUF_PARAM getobuf_param = { 0 };
    MFE_GETOUTPUT_PARAM getoutput_param = { 0 };
    MFE_SETVUI_PARAM setvui_param = { 0 };

    PVR_Info info = { 0 };
    SPSPPS_INFO_t spspps = { 0 };
    MEMMAP_t obuf = { 0 };

    if (is_compat_task() == 1)
    {
        int err = 0;

        switch(u32Cmd)
        {
            case MApi_CMD_MFE_Init:
            case MApi_CMD_MFE_Encode:
            case MApi_CMD_MFE_DeInit:
            case MApi_CMD_MFE_SetColorFormat:
            case MApi_CMD_MFE_SetBitrateFramerate:
            {
                pinit_param32 = compat_ptr(pArgs);
                if (pinit_param32)
                {
                    pinfo32 = compat_ptr(pinit_param32->mfe_Info);
                }

                if (pinit_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                init_param.mfe_Info = &info;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&init_param));
                break;
            }

            case MApi_CMD_MFE_GetVOL:
            {
                pgetvol_param32 = compat_ptr(pArgs);
                if (pgetvol_param32)
                {
                    pinfo32 = compat_ptr(pgetvol_param32->mfe_Info);
                }

                if (pgetvol_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                getvol_param.mfe_Info = &info;
                getvol_param.header_info = &spspps;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&getvol_param));
                break;
            }

            case MApi_CMD_MFE_CompressOnePicture:
            {
                pcompone_param32 = compat_ptr(pArgs);
                if (pcompone_param32)
                {
                    pinfo32 = compat_ptr(pcompone_param32->mfe_Info);
                }

                if (pcompone_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                compone_param.mfe_Info = &info;
                compone_param.YUVPlane.Cur_Y0.miuPointer = compat_ptr(pcompone_param32->YUVPlane.Cur_Y0.miuPointer);
                compone_param.YUVPlane.Cur_Y0.miuAddress = compat_ptr(pcompone_param32->YUVPlane.Cur_Y0.miuAddress);
                compone_param.YUVPlane.Cur_Y0.size = compat_ptr(pcompone_param32->YUVPlane.Cur_Y0.size);
                compone_param.YUVPlane.Cur_C0.miuPointer = compat_ptr(pcompone_param32->YUVPlane.Cur_C0.miuPointer);
                compone_param.YUVPlane.Cur_C0.miuAddress = compat_ptr(pcompone_param32->YUVPlane.Cur_C0.miuAddress);
                compone_param.YUVPlane.Cur_C0.size = compat_ptr(pcompone_param32->YUVPlane.Cur_C0.size);
                compone_param.YUVPlane.Cur_Y1.miuPointer = compat_ptr(pcompone_param32->YUVPlane.Cur_Y1.miuPointer);
                compone_param.YUVPlane.Cur_Y1.miuAddress = compat_ptr(pcompone_param32->YUVPlane.Cur_Y1.miuAddress);
                compone_param.YUVPlane.Cur_Y1.size = compat_ptr(pcompone_param32->YUVPlane.Cur_Y1.size);
                compone_param.YUVPlane.Cur_C1.miuPointer = compat_ptr(pcompone_param32->YUVPlane.Cur_C1.miuPointer);
                compone_param.YUVPlane.Cur_C1.miuAddress = compat_ptr(pcompone_param32->YUVPlane.Cur_C1.miuAddress);
                compone_param.YUVPlane.Cur_C1.size = compat_ptr(pcompone_param32->YUVPlane.Cur_C1.size);
                compone_param.bForceIframe = pcompone_param32->bForceIframe;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&compone_param));
                break;
            }

            case MApi_CMD_MFE_GetOutBuffer:
            {
                pgetobuf_param32 = compat_ptr(pArgs);
                if (pgetobuf_param32)
                {
                    pinfo32 = compat_ptr(pgetobuf_param32->mfe_Info);
                }

                if (pgetobuf_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                getobuf_param.mfe_Info = &info;
                getobuf_param.outbuf = &obuf;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&getobuf_param));
                break;
            }

            case MApi_CMD_MFE_GetOutputInfo:
            {
                pgetoutput_param32 = compat_ptr(pArgs);
                if (pgetoutput_param32)
                {
                    pinfo32 = compat_ptr(pgetoutput_param32->mfe_Info);
                }

                if (pgetoutput_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                getoutput_param.mfe_Info = &info;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&getoutput_param));
                break;
            }

            case MApi_CMD_MFE_SetVUI:
            {
                psetvui_param32 = compat_ptr(pArgs);
                if (psetvui_param32)
                {
                    pinfo32 = compat_ptr(psetvui_param32->mfe_Info);
                }

                if (psetvui_param32 == NULL || pinfo32 == NULL)
                {
                    printk("[KO] param is NULL.\n");
                    return UTOPIA_STATUS_PARAMETER_ERROR;
                }

                MFE_adp_Compat_Copy_Info(pinfo32, &info);

                setvui_param.mfe_Info = &info;
                setvui_param.setLevel = psetvui_param32->setLevel;
                setvui_param.setVUI_aspect_ratio_info_present_flag = psetvui_param32->setVUI_aspect_ratio_info_present_flag;
                setvui_param.setVUI_aspect_ratio_idc = psetvui_param32->setVUI_aspect_ratio_idc;
                setvui_param.setVUI_sar_width = psetvui_param32->setVUI_sar_width;
                setvui_param.setVUI_sar_height = psetvui_param32->setVUI_sar_height;

                u32Ret = UtopiaIoctl(pInstanceTmp, u32Cmd, (void *)(&setvui_param));
                break;
            }

            default:
                break;
        }


        if (u32Ret == UTOPIA_STATUS_SUCCESS)
        {
            pinfo32->mfe_state = info.mfe_state;

            switch(u32Cmd)
            {
                case MApi_CMD_MFE_Init:
                    if (init_param.retVal == TRUE)
                    {
                        pinfo32->mfe_drvHandle = ptr_to_compat(info.mfe_drvHandle);
                        pinfo32->reserved5 = (MS_U32)(((MS_U64)info.mfe_drvHandle) >> 32);
                    }
                    pinit_param32->retVal = init_param.retVal;
                    break;

                case MApi_CMD_MFE_Encode:
                case MApi_CMD_MFE_SetColorFormat:
                case MApi_CMD_MFE_SetBitrateFramerate:
                    pinit_param32->retVal = init_param.retVal;
                    break;

                case MApi_CMD_MFE_GetVOL:
                    if (getvol_param.retVal == TRUE)
                    {
                        SPSPPS_INFO_32 *spspps32 = (SPSPPS_INFO_32 *)(pgetvol_param32->header_info);
                        spspps32->pHeaderSPS = pinfo32->BitstreamOutBuffer.miuPointer;
                        spspps32->SPSLen = info.BitstreamOutBuffer.size;
                        spspps32->pHeaderPPS = NULL;
                        spspps32->PPSLen = 0;
                        pinfo32->BitstreamOutBuffer.size = info.BitstreamOutBuffer.size;
                    }
                    pgetvol_param32->retVal = getvol_param.retVal;
                    break;

                case MApi_CMD_MFE_CompressOnePicture:
                    if (compone_param.retVal == TRUE)
                    {
                        pinfo32->BitstreamOutBuffer.size = info.BitstreamOutBuffer.size;
                    }
                    pcompone_param32->retVal = compone_param.retVal;
                    break;

                case MApi_CMD_MFE_GetOutBuffer:
                    if (getobuf_param.retVal == TRUE)
                    {
                        MEMMAP_32 *obuf32 = (MEMMAP_32*)(pgetobuf_param32->outbuf);
                        obuf32->miuPointer = pinfo32->BitstreamOutBuffer.miuPointer;
                        obuf32->miuAddress = getobuf_param.outbuf->miuAddress;
                        obuf32->size = getobuf_param.outbuf->size;
                    }
                    pgetobuf_param32->retVal = getobuf_param.retVal;
                    break;

                case MApi_CMD_MFE_DeInit:
                    if (init_param.retVal == TRUE)
                    {
                        pinfo32->mfe_drvHandle = NULL;
                        pinfo32->reserved5 = 0;
                    }
                    pinit_param32->retVal = init_param.retVal;
                    break;

                case MApi_CMD_MFE_GetOutputInfo:
                    if (getoutput_param.retVal == TRUE)
                    {
                        pgetoutput_param32->frametype = getoutput_param.frametype;
                        pgetoutput_param32->size = getoutput_param.size;
                    }
                    pgetoutput_param32->retVal = getoutput_param.retVal;
                    break;

                case MApi_CMD_MFE_SetVUI:
                    psetvui_param32->retVal = setvui_param.retVal;
                    break;

                default:
                    break;
            }
        }
    }
    else
#endif
    switch(u32Cmd)
    {
        case MApi_CMD_MFE_Init:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_INIT, spt_MFE_INIT, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_Encode:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_ENCODE, spt_MFE_ENCODE, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_GetVOL:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_GETVOL, spt_MFE_GETVOL, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_CompressOnePicture:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_COMPRESSONEPICTURE, spt_MFE_COMPRESSONEPICTURE, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_GetOutBuffer:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_GETOUTBUFFER, spt_MFE_GETOUTBUFFER, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_DeInit:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_DEINIT, spt_MFE_DEINIT, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_SetColorFormat:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_SET_COLORFORMAT, spt_MFE_SET_COLORFORMAT, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_SetBitrateFramerate:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_SET_BITRATE_FRAMERATE, spt_MFE_SET_BITRATE_FRAMERATE, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_GetOutputInfo:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_GETOUTPUTINFO, spt_MFE_GETOUTPUTINFO, buffer_arg, sizeof(buffer_arg));
            break;
        case MApi_CMD_MFE_SetVUI:
            u32Ret = UADPBypassIoctl(pInstanceTmp, u32Cmd, pArgs, spt_MFE_SETVUI, spt_MFE_SETVUI, buffer_arg, sizeof(buffer_arg));
            break;

        default:
            break;

    }

    return u32Ret;
   // return UtopiaIoctl(pModuleDDI->pInstant,u32Cmd,arg);
}
