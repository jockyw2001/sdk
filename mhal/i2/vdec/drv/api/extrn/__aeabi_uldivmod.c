//#include <apiJPEG_priv.h>


////#ifdef MSOS_TYPE_LINUX_KERNEL
////#if (KERNEL_DRIVER_PATCH ==1)
////#if !defined(CONFIG_ARM64)
#include <linux/math64.h>
//__aeabi_uldivmod
unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d)
{
    return div64_u64(n, d);
}

//__aeabi_ldivmod
long long __aeabi_ldivmod(long long n, long long d)
{
    return div64_s64(n, d);
}
////#endif
////#endif
////#endif

#if 0
JPEG_Result _MApi_JPEG_Init(JPEG_InitParam *pInitParam)
{
    return E_JPEG_OKAY;
}

JPEG_Result _MApi_JPEG_DecodeHdr(void)
{
    return E_JPEG_OKAY;
}

JPEG_Result _MApi_JPEG_Decode(void)
{
    return E_JPEG_OKAY;
}

void _MApi_JPEG_Exit(void)
{

}

JPEG_ErrCode _MApi_JPEG_GetErrorCode(void)
{
    return E_JPEG_NO_ERROR;
}

JPEG_Event _MApi_JPEG_GetJPDEventFlag(void)
{
    return E_JPEG_EVENT_DEC_NONE;
}

void _MApi_JPEG_SetJPDEventFlag(JPEG_Event eEvtVal)
{

}

void _MApi_JPEG_Rst(void)
{

}

void _MApi_JPEG_PowerOn(void)
{

}

void _MApi_JPEG_PowerOff(void)
{

}

MS_U16 _MApi_JPEG_GetCurVidx(void)
{
    return 0;
}

MS_BOOL _MApi_JPEG_IsProgressive(void)
{
    return TRUE;
}

MS_BOOL _MApi_JPEG_ThumbnailFound(void)
{
    return TRUE;
}

MS_U16 _MApi_JPEG_GetWidth(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetHeight(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetOriginalWidth(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetOriginalHeight(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetNonAlignmentWidth(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetNonAlignmentHeight(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetAlignedPitch(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetAlignedPitch_H(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetAlignedWidth(void)
{
    return 0;
}

MS_U16 _MApi_JPEG_GetAlignedHeight(void)
{
    return 0;
}

MS_U8 _MApi_JPEG_GetScaleDownFactor(void)
{
    return 0;
}

void _MApi_JPEG_SetMaxDecodeResolution(MS_U16 u16Width, MS_U16 u16Height)
{

}

void _MApi_JPEG_SetProMaxDecodeResolution(MS_U16 u16ProWidth, MS_U16 u16ProHeight)
{

}

void _MApi_JPEG_SetMRBufferValid(JPEG_BuffLoadType u8MRBuffType)
{

}

void _MApi_JPEG_UpdateReadInfo(MS_U32 u32DataRead, MS_BOOL bEOFflag)
{

}

void _MApi_JPEG_ProcessEOF(JPEG_BuffLoadType u8MRBuffType)
{

}

void _MApi_JPEG_SetErrCode(JPEG_ErrCode ErrStatus)
{

}

void _MApi_JPEG_SetDbgLevel(MS_U8 u8DbgLevel)
{
}

MS_BOOL MApi_JPEG_IsNJPD(void)
{
    return TRUE;
}

MS_U8 _MApi_NJPD_JPDCount(void)
{
    return 0;
}

void _MApi_JPEG_SetNJPDInstance(MS_U8 JPDNum)
{
}
#endif
