#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "mhal_rgn.h"

#define MHAL_RGN_MAX_PALETTE_TABLE_NUM  16

typedef struct MHAL_RGN_PaletteElement_s
{
    MS_U8 u8Blue;
    MS_U8 u8Green;
    MS_U8 u8Red;
    MS_U8 u8Alpha;
}MHAL_RGN_PaletteElement_t;

typedef struct MHAL_RGN_PaletteTable_s
{
    MHAL_RGN_PaletteElement_t astElement[MHAL_RGN_MAX_PALETTE_TABLE_NUM];
}MHAL_RGN_PaletteTable_t;

typedef struct
{
    MS_U32 u8R;
    MS_U32 u8G;
    MS_U32 u8B;
} MHAL_RGN_CoverColorType_t;

typedef struct MHAL_RGN_GopGwinBuf_s
{
    MS_U32 u32Width;
    MS_U32 u32Height;
    MS_U32 u32Stride;
    MS_PHYADDR phyAddr;
    MS_U32 virAddr;
    MHAL_RGN_GopPixelFormat_e ePixelFmt;
}MHAL_RGN_GopGwinBuf_t;

typedef MS_U32 (*fpRgnMhalFill)(MHAL_RGN_GopGwinBuf_t *pstBufTo, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Color);
typedef MS_U32 (*fpRgnMhalBlit)(MHAL_RGN_GopGwinBuf_t *pstBufTo, MHAL_RGN_GopGwinBuf_t* pstBufFrom, MS_U32 u32X, MS_U32 u32Y);
typedef MS_U32 (*fpRgnMhalBufOpt)(MHAL_RGN_GopGwinBuf_t *pstBuf);

typedef struct MHAL_RGN_GopWnd_s
{
    MHAL_RGN_GopGwinId_e eGwinId;
    MHAL_RGN_GopGwinBuf_t stGopGwinBuf;
    MS_BOOL bShow;
    MS_U32 u32X;
    MS_U32 u32Y;
}MHAL_RGN_GopWnd_t;

typedef struct MHAL_RGN_CoverWnd_s
{
    MS_BOOL bShow;
    MS_U32 u32Width;
    MS_U32 u32Height;
    MS_U32 u32X;
    MS_U32 u32Y;
    MS_U32 u32Color;
}MHAL_RGN_CoverWnd_t;

typedef long LONG;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int Uint32;

#pragma pack(2)
typedef struct {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BMPFILEHEADER_T;

typedef struct{
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BMPINFOHEADER_T;
#pragma pack()

typedef struct
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved; // 0
} RGBQUAD_T;


#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#define MHAL_RGN_PIXELFMT_CHECK(PARA,ERRCODE)    do{    \
                                                        switch (PARA) \
                                                        { \
                                                            case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555: \
                                                            case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444: \
                                                            case E_MHAL_RGN_PIXEL_FORMAT_I2: \
                                                            case E_MHAL_RGN_PIXEL_FORMAT_I4: \
                                                                break; \
                                                            default: \
                                                                ERRCODE; \
                                                        } \
                                                    }while(0);


#define MHAL_RGN_PIXELFMT_BITCOUNT(fmt, bits) do{  \
    switch(fmt) \
    {   \
        case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555:   \
        case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444:   \
            bits = 16;  \
            break;  \
        case E_MHAL_RGN_PIXEL_FORMAT_I2: \
            bits = 2;   \
            break;  \
        case E_MHAL_RGN_PIXEL_FORMAT_I4: \
            bits = 4;    \
            break;  \
        default:    \
            printk("format %d is not supported\n", fmt);    \
            return -1;  \
    }   \
} while(0);
#define MHAL_RGN_ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#define MHAL_RGN_ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))

static MHAL_RGN_GopWnd_t _stGopBaseWnd[E_MHAL_GOP_TYPE_MAX];
static MHAL_RGN_GopWnd_t _stGopGwin[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
static MHAL_RGN_CoverWnd_t _stCoverGwin[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
static MHAL_RGN_PaletteTable_t _gstPalette;

static fpRgnMhalFill _gfpFill;
static fpRgnMhalBlit _gfpBlit;
static fpRgnMhalBufOpt _gfpAlloc;
static fpRgnMhalBufOpt _gfpFree;


#define MHAL_RGN_COVERCOLOR_TO_FORMATCOLOR(eGopId, fmtColor, coverColor, pixelFmt, bitPerPixel) do {    \
    fmtColor = 0;  \
    switch (pixelFmt)   \
    {   \
        case E_MHAL_RGN_PIXEL_FORMAT_RGB1555: \
            fmtColor |= (coverColor.u8R & 0xF) << 11; \
            fmtColor |= (coverColor.u8G & 0xF) << 6;   \
            fmtColor |= (coverColor.u8B & 0xF) << 1;   \
            bitPerPixel = 16;  \
            break;  \
        case E_MHAL_RGN_PIXEL_FORMAT_RGB4444: \
            fmtColor |= (coverColor.u8R & 0xF) << 12;  \
            fmtColor |= (coverColor.u8G & 0xF) << 8;   \
            fmtColor |= (coverColor.u8B & 0xF) << 4;   \
            bitPerPixel = 16;  \
            break;  \
        case E_MHAL_RGN_PIXEL_FORMAT_RGBI2: \
            fmtColor = 0;   \
            bitPerPixel = 4;  \
            break;  \
        case E_MHAL_RGN_PIXEL_FORMAT_RGBI4: \
            fmtColor = 0;   \
            bitPerPixel = 4;  \
            break; \
        default: \
            printk("format %d is not supported\n", pixelFmt);    \
            return -1;  \
    }   \
} while(0);

static MS_U32 MHAL_RGN_TransI2ToI4(MHAL_RGN_GopGwinBuf_t *pstBufTo, MHAL_RGN_GopGwinBuf_t* pstBufFrom)
{
    MS_U32 i = 0, j = 0;
    MS_U8 *pu8CopyAddrFrom = NULL;
    MS_U8 *pu8CopyAddrTo = NULL;
    MS_U16 *pu16CopyAddrTo = NULL;
    MS_U16 u16CopyTwoBytes = 0;
    MS_U16 u16BytesData = 0;
    MS_U8 *pu8GapAddrFrom = NULL;
    MS_U8 *pu8GapAddrTo = NULL;
    MS_BOOL bCheckGap = 0;

    BUG_ON(!pstBufFrom);
    BUG_ON(!pstBufTo);
    BUG_ON(pstBufTo->u32Height != pstBufFrom->u32Height);
    BUG_ON(pstBufTo->u32Width != pstBufFrom->u32Width);
    BUG_ON(!pstBufTo->virAddr);
    BUG_ON(!pstBufFrom->virAddr);
    BUG_ON(pstBufTo->ePixelFmt != E_MHAL_RGN_PIXEL_FORMAT_I4);
    BUG_ON(pstBufFrom->ePixelFmt != E_MHAL_RGN_PIXEL_FORMAT_I2);

    pu8CopyAddrFrom = (MS_U8 *)pstBufFrom->virAddr;
    pu8CopyAddrTo = (MS_U8 *)pstBufTo->virAddr;
    u16CopyTwoBytes = MHAL_RGN_ALIGN_DOWN(pstBufTo->u32Width/2, 2)/2 ;
    bCheckGap = ((pstBufTo->u32Width/2) % 2)?TRUE:FALSE;

    for (i = 0; i < pstBufTo->u32Height; i++)
    {
        pu16CopyAddrTo = (MS_U16 *)pu8CopyAddrTo;
        for (j = 0; j < u16CopyTwoBytes; j++)
        {
            u16BytesData = (MS_U16)pu8CopyAddrFrom[j];
            pu16CopyAddrTo[j] = (u16BytesData & 0x3) | ((u16BytesData & 0xC) << 2) | ((u16BytesData & 0x30) << 4) | ((u16BytesData & 0xC0) << 6);
        }
        if (bCheckGap)
        {
            pu8GapAddrTo[j] = (pu8GapAddrFrom[j] & 0x3) | (pu8GapAddrFrom[j] & 0xC);
        }
        pu8CopyAddrTo += pstBufTo->u32Stride;
        pu8CopyAddrFrom += pstBufFrom->u32Stride;
    }

    return MHAL_SUCCESS;
}

static MS_U32 _MHAL_RGN_SaveFile(MHAL_RGN_GopPixelFormat_e ePixelFmt, MHAL_RGN_GopType_e eGopId, MS_S32 s32ChnId, MS_U8 *pu8Buffer, MS_U32 s32Length)
{
    struct file *pFile = NULL;
    MS_U8 szFileName[64];
    mm_segment_t tFs;
    loff_t pos = 0;
    ssize_t writeBytes = 0;
    MS_BOOL bVpePort = TRUE;
    MS_U8 u8OutputPort = 0;

    BMPINFOHEADER_T bih;
    BMPFILEHEADER_T bfh;

    MS_U8 u8BitCount = 0;
    MS_U16 u16PaletteSize = 0;
    MS_U32 u32LineByte = 0;
    MS_U32 u8PicSize = 0;

    BUG_ON(!pu8Buffer);

    u16PaletteSize = (E_MHAL_RGN_PIXEL_FORMAT_I4 == ePixelFmt)?sizeof(MHAL_RGN_PaletteElement_t) * MHAL_RGN_MAX_PALETTE_TABLE_NUM:0;
    MHAL_RGN_PIXELFMT_BITCOUNT(ePixelFmt, u8BitCount);

    u32LineByte = WIDTHBYTES(1920 * u8BitCount);
    u8PicSize = 1080 * u32LineByte;
    memset(szFileName, 0, 64);

    bfh.bfType = (WORD)0x4d42;
    bfh.bfSize = u8PicSize + u16PaletteSize + sizeof(BMPFILEHEADER_T) + sizeof(BMPINFOHEADER_T);
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BMPFILEHEADER_T) + sizeof(BMPINFOHEADER_T) + u16PaletteSize;

    printk("fileheader %d infoheader %d palettesize %d \n", sizeof(BMPFILEHEADER_T), sizeof(BMPINFOHEADER_T), u16PaletteSize);

    bih.biSize = sizeof(BMPINFOHEADER_T);
    bih.biWidth = 1920;
    bih.biHeight = 1080;
    bih.biPlanes = 1;
    bih.biBitCount = u8BitCount;
    bih.biCompression = 0;
    bih.biSizeImage = u8PicSize;
    bih.biXPelsPerMeter = 2835 ;
    bih.biYPelsPerMeter = 2835 ;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    switch(eGopId)
    {
        case E_MHAL_GOP_VPE_PORT0:
            u8OutputPort = 0;
            bVpePort = TRUE;
            break;
        case E_MHAL_GOP_VPE_PORT1:
            u8OutputPort = 1;
            bVpePort = TRUE;
            break;
        case E_MHAL_GOP_VPE_PORT2:
            u8OutputPort = 2;
            bVpePort = TRUE;
            break;
        case E_MHAL_GOP_VPE_PORT3:
            u8OutputPort = 3;
            bVpePort = TRUE;
            break;
        case E_MHAL_GOP_DIVP_PORT0:
            u8OutputPort = 0;
            bVpePort = FALSE;
            break;
        default:
            printk("wrong output port to save file\n");
            return -1;
    }

    if (bVpePort)
    {
        sprintf(szFileName, "/mnt/1920X1080_Vpe_Chn%d_Port%d.bmp", s32ChnId, u8OutputPort);
    }
    else
    {
        sprintf(szFileName, "/mnt/1920X1080_Divp_Chn%d_Port%d.bmp", s32ChnId, u8OutputPort);
    }

    pFile = filp_open(szFileName, O_RDWR | O_CREAT, 0644);

    if (IS_ERR(pFile))
    {
        printk("Create file:%s error.\n", szFileName);
        return -1;
    }

    tFs = get_fs();
    set_fs(KERNEL_DS);

    writeBytes = vfs_write(pFile, (const char*)&bfh, 8, &pos);
    writeBytes += vfs_write(pFile, (const char*)&bfh.bfReserved2, sizeof(bfh.bfReserved2), &pos);
    writeBytes += vfs_write(pFile, (const char*)&bfh.bfOffBits, sizeof(bfh.bfOffBits), &pos);
    writeBytes += vfs_write(pFile, (const char*)&bih, sizeof(BMPINFOHEADER_T), &pos);
    if (u16PaletteSize)
        writeBytes += vfs_write(pFile, (const char*)(_gstPalette.astElement), u16PaletteSize, &pos);
    writeBytes += vfs_write(pFile, (const char*)pu8Buffer, s32Length, &pos);
    filp_close(pFile, NULL);
    set_fs(tFs);

    printk("_MHAL_RGN_SaveFile, write %d bytes to %s\n", writeBytes, szFileName);
    return (MS_S32)writeBytes;
}


MS_S32 MHAL_RGN_RegistBlitAlloc(fpRgnMhalFill pfFill, fpRgnMhalBlit pfBlit, fpRgnMhalBufOpt fpBufAlloc, fpRgnMhalBufOpt fpBufDel)
{
    _gfpFill = pfFill;
    _gfpBlit = pfBlit;
    _gfpAlloc = fpBufAlloc;
    _gfpFree = fpBufDel;

    return MHAL_SUCCESS;

}
MS_S32 MHAL_RGN_SetupCmdQ(MHAL_CMDQ_CmdqInterface_t* pstCmdInf, MHAL_RGN_CmdqType_e u8ModId)
{
    return MHAL_SUCCESS;
}


/*Setup cmdq*/
//MS_S32 MHAL_RGN_SetupCmdQ(cmd_mload_interface* pstCmdInf)
//{
//  return MHAL_SUCCESS;
//}

/*Cover init*/
MS_S32 MHAL_RGN_CoverInit(void)
{
    printk("CoverInit");

    memset(&_stCoverGwin, 0, sizeof(MHAL_RGN_CoverWnd_t) * E_MHAL_COVER_TYPE_MAX * E_MHAL_COVER_LAYER_MAX);
    return MHAL_SUCCESS;
}

/*Set cover color NA:[31~24] R:[23~16] G:[15~8] B:[7~0]*/
MS_S32 MHAL_RGN_CoverSetColor(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32Color)
{
//    printk("CoverSetColor: eCoverId[%d], eLayer[%d], u32Color[%d]\n", (MS_U8)eCoverId, (MS_U8)eLayer, u32Color);
    _stCoverGwin[eCoverId][eLayer].u32Color = u32Color;

    return MHAL_SUCCESS;
}

/*Cover  size*/
MS_S32 MHAL_RGN_CoverSetWindow(MHAL_RGN_CoverType_e eCoverId,
                                   MHAL_RGN_CoverLayerId_e eLayer,
                                   MS_U32 u32X,
                                   MS_U32 u32Y,
                                   MS_U32 u32Width,
                                   MS_U32 u32Height)
{
    printk("CoverSetWindow: eCoverId[%d], eLayer[%d], X[%d], Y[%d], width[%d], height[%d]\n", (MS_U8)eCoverId, (MS_U8)eLayer,
            u32X, u32Y, u32Width, u32Height);
    _stCoverGwin[eCoverId][eLayer].u32X = u32X;
    _stCoverGwin[eCoverId][eLayer].u32Y = u32Y;
    _stCoverGwin[eCoverId][eLayer].u32Width = u32Width;
    _stCoverGwin[eCoverId][eLayer].u32Height = u32Height;

    return MHAL_SUCCESS;
}

/*Enable cover*/
MS_S32 MHAL_RGN_CoverEnable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    printk("CoverEnable: eCoverId[%d], eLayer[%d]\n", (MS_U8)eCoverId, (MS_U8)eLayer);
    _stCoverGwin[eCoverId][eLayer].bShow = TRUE;

    return MHAL_SUCCESS;
}

/*Disable cover*/
MS_S32 MHAL_RGN_CoverDisable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    printk("CoverDisable: eCoverId[%d], eLayer[%d]\n", (MS_U8)eCoverId, (MS_U8)eLayer);
    _stCoverGwin[eCoverId][eLayer].bShow = FALSE;

    return MHAL_SUCCESS;
}

/*stats cover layer num*/
/*Gop init*/
MS_S32 MHAL_RGN_GopInit(void)
{
    printk("GopInit\n");

    memset(_stGopBaseWnd, 0, E_MHAL_GOP_TYPE_MAX * sizeof(MHAL_RGN_GopWnd_t));
    return MHAL_SUCCESS;
}

/*Set palette*/
MS_S32 MHAL_RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId,
                                MS_U8 u8Alpha,
                                MS_U8 u8Red,
                                MS_U8 u8Green,
                                MS_U8 u8Blue,
                                MS_U8 u8Idx)
{
    //printk("GopSetPalette: eGopId[%d], Alpha[%d], R[%d], G[%d], B[%d], index[%d]\n", (MS_U32)eGopId, u8Alpha, u8Red, u8Green, u8Blue, u8Idx);
    _gstPalette.astElement[u8Idx].u8Alpha = u8Alpha;
    _gstPalette.astElement[u8Idx].u8Red = u8Red;
    _gstPalette.astElement[u8Idx].u8Green = u8Green;
    _gstPalette.astElement[u8Idx].u8Blue = u8Blue;

    return MHAL_SUCCESS;
}

/*Set base gop window size*/
MS_S32 MHAL_RGN_GopSetBaseWindow(MHAL_RGN_GopType_e eGopId, MS_U32 u32Width, MS_U32 u32Height)
{
    _stGopBaseWnd[eGopId].stGopGwinBuf.u32Width = u32Width;
    _stGopBaseWnd[eGopId].stGopGwinBuf.u32Height = u32Height;
    _stGopBaseWnd[eGopId].stGopGwinBuf.phyAddr = 0;
    _stGopBaseWnd[eGopId].stGopGwinBuf.virAddr = 0;

//    printk("BaseWindow eGop[%d], width[%d] height[%d]\n", (MS_U8)eGopId, u32Width, u32Height);
    return MHAL_SUCCESS;
}

/*Set base gop gwin pix format*/
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(MHAL_RGN_GopType_e eGopId,
                                          MHAL_RGN_GopGwinId_e eGwinId,
                                          MHAL_RGN_GopPixelFormat_e eFormat)
{
    _stGopBaseWnd[eGopId].stGopGwinBuf.ePixelFmt = _stGopGwin[eGopId][eGwinId].stGopGwinBuf.ePixelFmt = eFormat;
//    printk("set pixel Format %d gwin id %d \n", _stGopGwin[eGopId][eGwinId].stGopGwinBuf.ePixelFmt, eGwinId);

    return MHAL_SUCCESS;
}

/*Set base gop gwin osd window*/
MS_S32 MHAL_RGN_GopGwinSetWindow(MHAL_RGN_GopType_e eGopId,
                                      MHAL_RGN_GopGwinId_e eGwinId,
                                      MS_U32 u32Width,
                                      MS_U32 u32Height,
                                      MS_U32 u32Stride,
                                      MS_U32 u32X,
                                      MS_U32 u32Y)
{
    _stGopGwin[eGopId][eGwinId].stGopGwinBuf.u32Width = u32Width;
    _stGopGwin[eGopId][eGwinId].stGopGwinBuf.u32Height = u32Height;
    _stGopGwin[eGopId][eGwinId].stGopGwinBuf.u32Stride = u32Stride;
    _stGopGwin[eGopId][eGwinId].u32X = u32X;
    _stGopGwin[eGopId][eGwinId].u32Y = u32Y;

    return MHAL_SUCCESS;
}


MS_S32 MHAL_RGN_GopGwinSetBuffer(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_PHYADDR phyAddr)
{
    _stGopGwin[eGopId][eGwinId].stGopGwinBuf.phyAddr = phyAddr;

    return MHAL_SUCCESS;
}

/*Set base gop gwin osd bufdfer addr*/
MS_S32 MHAL_RGN_GopGwinSetBufferVirt(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32VirtAddr)
{
    _stGopGwin[eGopId][eGwinId].stGopGwinBuf.virAddr = u32VirtAddr;

    return MHAL_SUCCESS;
}

/*enable Gwin*/
MS_S32 MHAL_RGN_GopGwinEnable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    _stGopGwin[eGopId][eGwinId].bShow = TRUE;

    return MHAL_SUCCESS;
}

/*disable Gwin*/
MS_S32 MHAL_RGN_GopGwinDisable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    _stGopGwin[eGopId][eGwinId].bShow = FALSE;

    return MHAL_SUCCESS;
}


MS_S32 MHAL_RGN_GwinUpdate(MHAL_RGN_GopType_e eGopId,
                              MHAL_RGN_CoverType_e eCoverType, MS_S32 s32ChnId)
{
    MS_S32 i = 0;
    MHAL_RGN_GopGwinBuf_t stFrom, stTo;

    printk("MHAL_RGN_GwinUpdate: MHAL_RGN_GopGwinUpdate eGopId[%d], eCoverType[%d], chnId[%d]\n",
           (MS_S32)eGopId, (MS_S32)eCoverType, s32ChnId);

    if (MHAL_SUCCESS == _gfpAlloc(&_stGopBaseWnd[eGopId].stGopGwinBuf))
    {
        // osd
        for (i = 0; i < (MS_S32)E_MHAL_GOP_GWIN_ID_MAX; i++)
        {
            if (_stGopGwin[eGopId][i].bShow)
            {
                _gfpBlit(&_stGopBaseWnd[eGopId].stGopGwinBuf, &_stGopGwin[eGopId][i].stGopGwinBuf, _stGopGwin[eGopId][i].u32X, _stGopGwin[eGopId][i].u32Y);
            }
        }

        //cover
        switch (eGopId)
        {
            case E_MHAL_GOP_VPE_PORT0:
            case E_MHAL_GOP_VPE_PORT1:
            case E_MHAL_GOP_VPE_PORT2:
                for (i = 0; i < E_MHAL_COVER_LAYER_MAX; i++)
                {
                    if (_stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].bShow)
                    {
                        _gfpFill(&_stGopBaseWnd[eGopId].stGopGwinBuf, _stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].u32Width,
                            _stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].u32Height, _stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].u32X,
                            _stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].u32Y, _stCoverGwin[E_MHAL_COVER_VPE_PORT0][i].u32Color);
                    }
                }
                break;
            case E_MHAL_GOP_VPE_PORT3:
                for (i = 0; i < E_MHAL_COVER_LAYER_MAX; i++)
                {
                    if (_stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].bShow)
                    {
                        _gfpFill(&_stGopBaseWnd[eGopId].stGopGwinBuf, _stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].u32Width,
                            _stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].u32Height, _stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].u32X,
                            _stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].u32Y, _stCoverGwin[E_MHAL_COVER_VPE_PORT1][i].u32Color);
                    }
                }
                break;
            case E_MHAL_GOP_DIVP_PORT0:
                for (i = 0; i < E_MHAL_COVER_LAYER_MAX; i++)
                {
                    if (_stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].bShow)
                    {
                        _gfpFill(&_stGopBaseWnd[eGopId].stGopGwinBuf, _stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].u32Width,
                            _stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].u32Height, _stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].u32X,
                            _stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].u32Y, _stCoverGwin[E_MHAL_COVER_DIVP_PORT0][i].u32Color);
                    }
                }
                break;
            default:
                break;
        }
        // save to argb1555

        if (E_MHAL_RGN_PIXEL_FORMAT_I2 == _stGopBaseWnd[eGopId].stGopGwinBuf.ePixelFmt)
        {
            memcpy(&stFrom, &_stGopBaseWnd[eGopId].stGopGwinBuf, sizeof(MHAL_RGN_GopGwinBuf_t));
            memset(&stTo, 0, sizeof(MHAL_RGN_GopGwinBuf_t));
            stTo.ePixelFmt = E_MHAL_RGN_PIXEL_FORMAT_I4;
            stTo.u32Height = stFrom.u32Height;
            stTo.u32Width = stFrom.u32Width;
            if (MHAL_SUCCESS == _gfpAlloc(&stTo))
            {
                MHAL_RGN_TransI2ToI4(&stTo, &stFrom);
                _MHAL_RGN_SaveFile(stTo.ePixelFmt, eGopId, s32ChnId, (MS_U8*)stTo.virAddr, stTo.u32Stride * stTo.u32Height);
                _gfpFree(&stTo);
            }
            else
            {
                printk("Buffer alloc error!!!!!\n");
            }
        }
        else
        {
            _MHAL_RGN_SaveFile(_stGopBaseWnd[eGopId].stGopGwinBuf.ePixelFmt, eGopId, s32ChnId, (MS_U8*)_stGopBaseWnd[eGopId].stGopGwinBuf.virAddr, _stGopBaseWnd[eGopId].stGopGwinBuf.u32Stride * _stGopBaseWnd[eGopId].stGopGwinBuf.u32Height);
        }
        _gfpFree(&_stGopBaseWnd[eGopId].stGopGwinBuf);
    }
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_GopSetColorkey(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_U8 u8R, MS_U8 u8G, MS_U8 u8B)
{
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_GopSetAlphaType(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinAlphaType_e eAlphaType, MS_U8 u8ConstAlphaVal)
{
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal)
{
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_GetChipCapability(MHAL_RGN_ChipCapType_e eType, MHAL_RGN_GopPixelFormat_e eFormat)
{
    switch(eType) {
        case E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT:
            if(eFormat <= E_MHAL_RGN_PIXEL_FORMAT_I4) {
                return 1;
            } else {
                return 0;
            }
        case E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT:   // unit:pixel
        case E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT:
            switch(eFormat) {
                case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555:
                case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444:
                case E_MHAL_RGN_PIXEL_FORMAT_I4:
                    return 2;
                case E_MHAL_RGN_PIXEL_FORMAT_I2:
                    return 4;
                default:
                    return MHAL_FAILURE;
            }
        case E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT:
            return 16; // unit:byte
        case E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT: // gwin number
            return 2;
        default:
            return MHAL_FAILURE;
    }
}

EXPORT_SYMBOL(MHAL_RGN_RegistBlitAlloc);
EXPORT_SYMBOL(MHAL_RGN_SetupCmdQ);
EXPORT_SYMBOL(MHAL_RGN_CoverInit);
EXPORT_SYMBOL(MHAL_RGN_CoverSetColor);
EXPORT_SYMBOL(MHAL_RGN_CoverSetWindow);
EXPORT_SYMBOL(MHAL_RGN_CoverEnable);
EXPORT_SYMBOL(MHAL_RGN_CoverDisable);
EXPORT_SYMBOL(MHAL_RGN_GopInit);
EXPORT_SYMBOL(MHAL_RGN_GopSetPalette);
EXPORT_SYMBOL(MHAL_RGN_GopSetBaseWindow);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetPixelFormat);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetWindow);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetBuffer);
EXPORT_SYMBOL(MHAL_RGN_GopGwinSetBufferVirt);
EXPORT_SYMBOL(MHAL_RGN_GopGwinEnable);
EXPORT_SYMBOL(MHAL_RGN_GopGwinDisable);
EXPORT_SYMBOL(MHAL_RGN_GwinUpdate);
EXPORT_SYMBOL(MHAL_RGN_GopSetColorkey);
EXPORT_SYMBOL(MHAL_RGN_GopSetAlphaType);
EXPORT_SYMBOL(MHAL_RGN_GopSetArgb1555AlphaVal);
EXPORT_SYMBOL(MHAL_RGN_GetChipCapability);
