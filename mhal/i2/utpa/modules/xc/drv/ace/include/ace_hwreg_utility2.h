//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _HWREG_UTILITY_H_
#define _HWREG_UTILITY_H_

#include "ace_hwreg.h"
#include "MsCommon.h"


#ifdef _MDRV_ACE_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
//!! Do not include this header in driver or api level

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
extern MS_VIRT _ACE_RIU_BASE;      // This should be inited before XC library starting.

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#define _BITMASK(loc_msb, loc_lsb) ((1U << (loc_msb)) - (1U << (loc_lsb)) + (1U << (loc_msb)))
#define BITMASK(x) _BITMASK(1?x, 0?x)
#define HBMASK    0xFF00
#define LBMASK    0x00FF

#define RIU_MACRO_START     do {
#define RIU_MACRO_END       } while (0)

// Address bus of RIU is 16 bits.
#define RIU_READ_BYTE(addr)         ( READ_BYTE( _ACE_RIU_BASE + (addr) ) )
#define RIU_READ_2BYTE(addr)        ( READ_WORD( _ACE_RIU_BASE + (addr) ) )
#define RIU_WRITE_BYTE(addr, val)   { WRITE_BYTE( _ACE_RIU_BASE + (addr), val) }
#define RIU_WRITE_2BYTE(addr, val)  { WRITE_WORD( _ACE_RIU_BASE + (addr), val) }

//Device bank offset
INTERFACE MS_U32 u32XCDeviceBankOffset[XC_ACE_SUPPORT_DEVICE_NUM];

//=============================================================
// Standard Form
#ifdef REG_SCRIPT

#define MDrv_ReadByte( u32Reg )   RIU_READ_BYTE(((u32Reg) << 1) - ((u32Reg) & 1))

#define MDrv_Read2Byte( u32Reg )    (RIU_READ_2BYTE((u32Reg)<<1))

#define MDrv_Read4Byte( u32Reg )   ( (MS_U32)RIU_READ_2BYTE((u32Reg)<<1) | ((MS_U32)RIU_READ_2BYTE(((u32Reg)+2)<<1)<<16 )  )

#define MDrv_ReadRegBit( u32Reg, u8Mask )   (RIU_READ_BYTE(((u32Reg)<<1) - ((u32Reg) & 1)) & (u8Mask))

#define MDrv_WriteRegBit( u32Reg, bEnable, u8Mask )                                     \
    RIU_MACRO_START                                                                     \
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x\n", (unsigned int)(u32Reg),  (unsigned int)((u8Mask) & 0xFF), bEnable ?  (unsigned int)((u8Mask) & 0xFF) : 0); \
    RIU_MACRO_END

#define MDrv_WriteByte( u32Reg, u8Val )                                                 \
    RIU_MACRO_START                                                                     \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(u32Reg), (unsigned int)( (u8Val) & 0xFF)); \
    RIU_MACRO_END

#define MDrv_Write2Byte( u32Reg, u16Val )                                               \
    RIU_MACRO_START                                                                     \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(u32Reg),   (unsigned int)((u16Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+1), (unsigned int)(((u16Val) & 0xFF00) >> 8)); \
    RIU_MACRO_END

#define MDrv_Write3Byte( u32Reg, u32Val )   \
    RIU_MACRO_START                         \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(u32Reg),   (unsigned int)((u32Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+1), (unsigned int)(((u32Val) & 0xFF00) >> 8)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+2), (unsigned int)(((u32Val) & 0xFF0000) >> 16)); \
    RIU_MACRO_END

#define MDrv_Write4Byte( u32Reg, u32Val )                                               \
    RIU_MACRO_START                                                                     \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(u32Reg),   (unsigned int)((u32Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+1), (unsigned int)(((u32Val) & 0x0000FF00) >> 8)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+2), (unsigned int)(((u32Val) & 0x00FF0000) >> 16)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((u32Reg)+3), (unsigned int)(((u32Val) & 0xFF000000) >> 24)); \
    RIU_MACRO_END

#define MDrv_WriteByteMask( u32Reg, u8Val, u8Msk )                                      \
    RIU_MACRO_START                                                                     \
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x\n", (unsigned int)(u32Reg),  (unsigned int)((u8Msk) & 0xFF), (unsigned int)((u8Val) & 0xFF)); \
    RIU_MACRO_END

#define MDrv_Write2ByteMask( u32Reg, u16Val, u16Mask) \
    RIU_MACRO_START                                     \
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x\n", (unsigned int)(u32Reg),  (unsigned int)((u16Mask) & 0xFF), (unsigned int)((u16Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x\n", (unsigned int)((u32Reg)+1),  (unsigned int)(((u16Mask) & 0xFF00)>> 8), (unsigned int)(((u16Val) & 0xFF00) >> 8)); \
    RIU_MACRO_END


#define SC_W2BYTE( u32Id,  u32Reg, u16Val)\
            REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) )),   (unsigned int)((u16Val) & 0xFF)); \
            REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)((REG_SCALER_BASE + ((u32Reg + 1) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) )),   (unsigned int)(((u16Val) & 0xFF00) >> 8 ));

#define SC_R2BYTE( u32Id,  u32Reg ) \
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) <<1 ) ; } )

#define SC_W4BYTE( u32Id,  u32Reg, u32Val)\
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ),   (unsigned int)((u32Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(REG_SCALER_BASE + ((u32Reg + 1) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ), (unsigned int)(((u32Val) & 0x0000FF00) >> 8)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(REG_SCALER_BASE + ((u32Reg + 2) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ), (unsigned int)(((u32Val) & 0x00FF0000) >> 16)); \
    REG_DBG("wriu -b 0x%06x 0xFF 0x%02x\n", (unsigned int)(REG_SCALER_BASE + ((u32Reg + 3) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ), (unsigned int)(((u32Val) & 0xFF000000) >> 24));

#define SC_R4BYTE( u32Id,  u32Reg )\
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1 ) | (MS_U32)(RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) + 2 ) << 1 )) << 16; })

#define SC_R2BYTEMSK( u32Id,  u32Reg, u16mask)\
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1) & (u16mask) ; } )

#define SC_W2BYTEMSK( u32Id,  u32Reg, u16Val, u16Mask)\
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x \n", (unsigned int)(REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ), (unsigned int)((u16Mask) & 0xFF), (unsigned int)((u16Val) & 0xFF)); \
    REG_DBG("wriu -b 0x%06x 0x%02x 0x%02x \n", (unsigned int)(REG_SCALER_BASE + ((u32Reg +1 ) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ), (unsigned int)(((u16Mask) & 0xFF00) >> 8), (unsigned int)(((u16Val) & 0xFF00) >> 8));



#else
#define MDrv_ReadByte( u32Reg )   RIU_READ_BYTE(((u32Reg) << 1) - ((u32Reg) & 1))

#define MDrv_Read2Byte( u32Reg )    (RIU_READ_2BYTE((u32Reg)<<1))

#define MDrv_Read4Byte( u32Reg )   ( (MS_U32)RIU_READ_2BYTE((u32Reg)<<1) | ((MS_U32)RIU_READ_2BYTE(((u32Reg)+2)<<1)<<16 )  )

#define MDrv_ReadRegBit( u32Reg, u8Mask )   (RIU_READ_BYTE(((u32Reg)<<1) - ((u32Reg) & 1)) & (u8Mask))

#define MDrv_WriteRegBit( u32Reg, bEnable, u8Mask )                                     \
    RIU_MACRO_START                                                                     \
    RIU_WRITE_BYTE( (((u32Reg) <<1) - ((u32Reg) & 1)) , (bEnable) ? (RIU_READ_BYTE(  (((u32Reg) <<1) - ((u32Reg) & 1))  ) |  (u8Mask)) :                           \
                                (RIU_READ_BYTE( (((u32Reg) <<1) - ((u32Reg) & 1)) ) & ~(u8Mask)));                            \
    RIU_MACRO_END

#define MDrv_WriteByte( u32Reg, u8Val )                                                 \
    RIU_MACRO_START                                                                     \
    RIU_WRITE_BYTE(((u32Reg) << 1) - ((u32Reg) & 1), u8Val);   \
    RIU_MACRO_END

#define MDrv_Write2Byte( u32Reg, u16Val )                                               \
    RIU_MACRO_START                                                                     \
    RIU_WRITE_2BYTE( ((u32Reg)<<1) ,  u16Val);                                          \
    RIU_MACRO_END

#define MDrv_Write3Byte( u32Reg, u32Val )   \
    RIU_MACRO_START                         \
    if ((u32Reg) & 0x01)                                                                \
    {                                                                                               \
        RIU_WRITE_BYTE((u32Reg << 1) - 1, u32Val);                                    \
        RIU_WRITE_2BYTE( (u32Reg + 1)<<1 , ((u32Val) >> 8));                                      \
    }                                                                                           \
    else                                                                                        \
    {                                                                                               \
        RIU_WRITE_2BYTE( (u32Reg) << 1,  u32Val);                                                         \
        RIU_WRITE_BYTE( (u32Reg + 2) << 1 ,  ((u32Val) >> 16));                             \
    }                           \
    RIU_MACRO_END

#define MDrv_Write4Byte( u32Reg, u32Val )                                               \
    RIU_MACRO_START                                                                     \
    if ((u32Reg) & 0x01)                                                      \
    {                                                                                               \
        RIU_WRITE_BYTE( ((u32Reg) << 1) - 1 ,  u32Val);                                         \
        RIU_WRITE_2BYTE( ((u32Reg) + 1)<<1 , ( (u32Val) >> 8));                                      \
        RIU_WRITE_BYTE( (((u32Reg) + 3) << 1) ,  ((u32Val) >> 24));                           \
    }                                                                                               \
    else                                                                                                \
    {                                                                                                   \
        RIU_WRITE_2BYTE( (u32Reg) <<1 ,  u32Val);                                                             \
        RIU_WRITE_2BYTE(  ((u32Reg) + 2)<<1 ,  ((u32Val) >> 16));                                             \
    }                                                                     \
    RIU_MACRO_END

#define MDrv_WriteByteMask( u32Reg, u8Val, u8Msk )                                      \
    RIU_MACRO_START                                                                     \
    RIU_WRITE_BYTE( (((u32Reg) <<1) - ((u32Reg) & 1)), (RIU_READ_BYTE((((u32Reg) <<1) - ((u32Reg) & 1))) & ~(u8Msk)) | ((u8Val) & (u8Msk)));                   \
    RIU_MACRO_END

#define MDrv_Write2ByteMask( u32Reg, u16Val, u16Mask) \
    RIU_MACRO_START                                     \
    RIU_WRITE_2BYTE(u32Reg<<1,   (RIU_READ_2BYTE(u32Reg<<1) & ~(u16Mask)) | (u16Val & u16Mask)) \
    RIU_MACRO_END


//=============================================================
// Just for Scaler
#if SCALER_REGISTER_SPREAD
#define SC_W2BYTE( u32Id,  u32Reg, u16Val)\
            ( { RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1 , u16Val  ) ; } )

#define SC_R2BYTE( u32Id,  u32Reg ) \
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) <<1 ) ; } )

#define SC_W4BYTE( u32Id,  u32Reg, u32Val)\
            ( { RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) <<1, (MS_U16)((u32Val) & 0x0000FFFF) ) ; \
                RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) + 2 ) << 1, (MS_U16)(((u32Val) >> 16) & 0x0000FFFF) ); } )

#define SC_R4BYTE( u32Id,  u32Reg )\
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1 ) | (MS_U32)(RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) + 2 ) << 1 )) << 16; })

#define SC_R2BYTEMSK( u32Id,  u32Reg, u16mask)\
            ( { RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1) & (u16mask) ; } )

#define SC_W2BYTEMSK( u32Id,  u32Reg, u16Val, u16Mask)\
            ( { RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1, (RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFFFF) + (u32XCDeviceBankOffset[u32Id] << 8) ) << 1) & ~(u16Mask) ) | ((u16Val) & (u16Mask)) ) ; })

#else
#define SC_W2BYTE( u32Id, u32Reg, u16Val)\
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE << 1, (((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_WRITE_2BYTE( (REG_SCALER_BASE +((u32Reg) & 0xFF) ) << 1 , u16Val  ) ; } )

#define SC_R2BYTE( u32Id, u32Reg ) \
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE << 1, ( ((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) )<<1 ) ; } )

#define SC_W4BYTE( u32Id, u32Reg, u32Val)\
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE<<1, (((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) ) <<1, (MS_U16)((u32Val) & 0x0000FFFF) ) ; \
                RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) + 2 ) << 1, (MS_U16)(((u32Val) >> 16) & 0x0000FFFF) ); } )

#define SC_R4BYTE( u32Id, u32Reg )\
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE << 1, (((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) ) << 1 ) | (MS_U32)(RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) + 2 ) << 1 )) << 16; } )

#define SC_R2BYTEMSK( u32Id, u32Reg, u16mask)\
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE << 1, (((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) ) << 1) & (u16mask) ; } )

#define SC_W2BYTEMSK( u32Id, u32Reg, u16Val, u16Mask)\
            ( { RIU_WRITE_2BYTE(REG_SCALER_BASE << 1, (((u32Reg) >> 8) & 0x00FF) + u32XCDeviceBankOffset[u32Id] ) ; \
                RIU_WRITE_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) ) << 1, (RIU_READ_2BYTE( (REG_SCALER_BASE + ((u32Reg) & 0xFF) ) << 1) & ~(u16Mask) ) | ((u16Val) & (u16Mask)) ) ; })

#endif

#endif //REG_SCRIPT

//=============================================================
// Just for MOD
#define MOD_W2BYTE( u32Reg, u16Val)\
            ( { RIU_WRITE_2BYTE(REG_MOD_BASE << 1, ((u32Reg) >> 8) & 0x00FF ); \
                RIU_WRITE_2BYTE( (REG_MOD_BASE + ((u32Reg) & 0xFF) ) << 1, u16Val ); } )

#define MOD_R2BYTE( u32Reg ) \
            ( { RIU_WRITE_2BYTE(REG_MOD_BASE << 1, ((u32Reg) >> 8) & 0x00FF ); \
                RIU_READ_2BYTE( (REG_MOD_BASE + ((u32Reg) & 0xFF) ) << 1) ; } )

#define MOD_R2BYTEMSK( u32Reg, u16mask)\
            ( { RIU_WRITE_2BYTE(REG_MOD_BASE << 1, ((u32Reg) >> 8) & 0x00FF ); \
                RIU_READ_2BYTE( (REG_MOD_BASE + ((u32Reg) & 0xFF) ) << 1) & (u16mask); })

#define MOD_W2BYTEMSK( u32Reg, u16Val, u16Mask)\
            ( { RIU_WRITE_2BYTE(REG_MOD_BASE << 1, ((u32Reg) >> 8) & 0x00FF ); \
                RIU_WRITE_2BYTE( (REG_MOD_BASE + ((u32Reg) & 0xFF) )<<1 , (RIU_READ_2BYTE( (REG_MOD_BASE + ((u32Reg) & 0xFF) ) << 1 ) & ~(u16Mask)) | ((u16Val) & (u16Mask)) ); } )


//=============================================================
//General ( Make sure u32Reg is not ODD
#define W2BYTE( u32Reg, u16Val) RIU_WRITE_2BYTE( (u32Reg) << 1 , u16Val )


#define R2BYTE( u32Reg ) RIU_READ_2BYTE( (u32Reg) << 1)

#define W4BYTE( u32Reg, u32Val)\
            ( { RIU_WRITE_2BYTE( (u32Reg) << 1, ((u32Val) & 0x0000FFFF) ); \
                RIU_WRITE_2BYTE( ( (u32Reg) + 2) << 1 , (((u32Val) >> 16) & 0x0000FFFF)) ; } )

#define R4BYTE( u32Reg )\
            ( { ((RIU_READ_2BYTE( (u32Reg) << 1)) | ((MS_U32)(RIU_READ_2BYTE( ( (u32Reg) + 2 ) << 1) ) << 16)) ; } )

#define R2BYTEMSK( u32Reg, u16mask)\
            ( { RIU_READ_2BYTE( (u32Reg)<< 1) & (u16mask) ; } )

#define W2BYTEMSK( u32Reg, u16Val, u16Mask)\
            ( { RIU_WRITE_2BYTE( (u32Reg)<< 1 , (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) ) ; } )


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#undef INTERFACE
#endif

