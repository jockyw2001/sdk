////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/**
* @file     hal_cover_util.h
* @version
* @Platform I2
* @brief    This file defines the HAL SCL utility interface
*
*/

#ifndef __HAL_COVER_UTIL_H__
#define __HAL_COVER_UTIL_H__


#define WRITE_COVER_REG(addr, type, data)  ((*(volatile type *)(addr)) = (data))
#define READ_COVER_REG(addr, type)         ((*(volatile type *)(addr)))


#if defined (COVEROS_TYPE_LINUX_KERNEL)
#include "ms_platform.h"
#define COVER_READ_BYTE(x)         ms_readb(x)
#define COVER_READ_WORD(x)         ms_readw(x)
#define COVER_READ_LONG(x)         ms_readl(x)
#define COVER_WRITE_BYTE(x, y)     ms_writeb((u8)(y), x)
#define COVER_WRITE_WORD(x, y)     ms_writew((u16)(y), x)
#define COVER_WRITE_LONG(x, y)     ms_writel((u32)(y), x)
#else
#define COVER_READ_BYTE(x)         READ_COVER_REG(x, u8)
#define COVER_READ_WORD(x)         READ_COVER_REG(x, u16)
#define COVER_READ_LONG(x)         READ_COVER_REG(x, u32)
#define COVER_WRITE_BYTE(x, y)     WRITE_COVER_REG(x, u8, y)
#define COVER_WRITE_WORD(x, y)     WRITE_COVER_REG(x, u16, y)
#define COVER_WRITE_LONG(x, y)     WRITE_COVER_REG(x, u32, y)
#endif

#define COVER_RIU_READ_BYTE(addr)         ( COVER_READ_BYTE( 0x1F000000+ (addr) ) )
#define COVER_RIU_READ_2BYTE(addr)        ( COVER_READ_WORD( 0x1F000000 + (addr) ) )
#define COVER_RIU_WRITE_BYTE(addr, val)    COVER_WRITE_BYTE( (0x1F000000 + (addr)), val)
#define COVER_RIU_WRITE_2BYTE(addr, val)   COVER_WRITE_WORD( 0x1F000000 + (addr), val)


#define COVER_R2BYTE( u32Reg ) COVER_RIU_READ_2BYTE( (u32Reg) << 1)

#define COVER_R2BYTEMSK( u32Reg, u16mask)\
    ( ( COVER_RIU_READ_2BYTE( (u32Reg)<< 1) & u16mask  ) )
#define COVER_R4BYTE( u32Reg )\
    ( { ((COVER_RIU_READ_2BYTE( (u32Reg) << 1)) | ((u32)(COVER_RIU_READ_2BYTE( ( (u32Reg) + 2 ) << 1) ) << 16)) ; } )


#define COVER_WBYTEMSK(u32Reg, u8Val, u8Mask) \
    COVER_RIU_WRITE_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)), ( COVER_RIU_READ_BYTE( (((u32Reg)<<1) - ((u32Reg) & 1)) ) & ~(u8Mask)) | ((u8Val) & (u8Mask)) )

#define COVER_W2BYTE( u32Reg, u16Val) COVER_RIU_WRITE_2BYTE( (u32Reg) << 1 , u16Val )

#define COVER_W2BYTEMSK( u32Reg, u16Val, u16Mask)\
    COVER_RIU_WRITE_2BYTE( (u32Reg)<< 1 , (COVER_RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) )

#define COVER_W4BYTE( u32Reg, u32Val)\
    ( { COVER_RIU_WRITE_2BYTE( (u32Reg) << 1, ((u32Val) & 0x0000FFFF) ); \
        COVER_RIU_WRITE_2BYTE( ( (u32Reg) + 2) << 1 , (((u32Val) >> 16) & 0x0000FFFF)) ; } )

#define COVER_W3BYTE( u32Reg, u32Val)\
    ( { COVER_RIU_WRITE_2BYTE( (u32Reg) << 1,  u32Val); \
        COVER_RIU_WRITE_BYTE( (u32Reg + 2) << 1 ,  ((u32Val) >> 16)); } )




#define BIT0                           0x00000001
#define BIT1                           0x00000002
#define BIT2                           0x00000004
#define BIT3                           0x00000008
#define BIT4                           0x00000010
#define BIT5                           0x00000020
#define BIT6                           0x00000040
#define BIT7                           0x00000080
#define BIT8                           0x00000100
#define BIT9                           0x00000200
#define BIT10                          0x00000400
#define BIT11                          0x00000800
#define BIT12                          0x00001000
#define BIT13                          0x00002000
#define BIT14                          0x00004000
#define BIT15                          0x00008000
#define BIT16                          0x00010000
#define BIT17                          0x00020000
#define BIT18                          0x00040000
#define BIT19                          0x00080000
#define BIT20                          0x00100000
#define BIT21                          0x00200000
#define BIT22                          0x00400000
#define BIT23                          0x00800000
#define BIT24                          0x01000000
#define BIT25                          0x02000000
#define BIT26                          0x04000000
#define BIT27                          0x08000000
#define BIT28                          0x10000000
#define BIT29                          0x20000000
#define BIT30                          0x40000000
#define BIT31                          0x80000000

extern s8 UartSendTrace(const char *strFormat, ...);

#endif /* __HAL_COVER_UTIL_H__ */
