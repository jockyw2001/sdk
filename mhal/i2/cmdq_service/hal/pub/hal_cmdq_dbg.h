///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2011 MStar Semiconductor, Inc.
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
///////////////////////////////////////////////////////////////////////////////


#ifndef _MDRV_DIP_DBG_H
#define _MDRV_DIP_DBG_H

#if !defined(CAM_OS_RTK)
#include <linux/bug.h>
#include <linux/printk.h>
extern u32 gCmdqDbgLvl;

typedef enum
{
    CMDQ_DBG_NONE_TYPE = 0,
    CMDQ_DBG_LVL0_TYPE = 0x1,
    CMDQ_DBG_LVL1_TYPE = 0x2,
    CMDQ_DBG_LVL2_TYPE = 0x4,
} DrvCmdqDbgLvl_e;

#define CMDQ_DBG(dbglv, _fmt, _args...)            \
    do{                                             \
        if((dbglv & gCmdqDbgLvl))                       \
        {                                              \
            printk(KERN_WARNING _fmt, ## _args);       \
        }\
      }while(0)

#define CMDQ_ERR(_fmt, _args...)       printk(KERN_WARNING _fmt, ## _args)

#define CMDQ_LOG(_fmt, _args...)       printk(_fmt, ## _args)

#define CMDQ_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                   __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)

#else
extern s8 UartSendTrace(const char *strFormat, ...);

extern u32 gCmdqDbgLvl;

typedef enum
{
    CMDQ_DBG_NONE_TYPE = 0,
    CMDQ_DBG_LVL0_TYPE = 0x1,
    CMDQ_DBG_LVL1_TYPE = 0x2,
    CMDQ_DBG_LVL2_TYPE = 0x4,
} DrvCmdqDbgLvl_e;

#define CMDQ_DBG(dbglv, _fmt, _args...)            \
    do{                                           \
        if((dbglv & gCmdqDbgLvl))                       \
        {                                              \
            UartSendTrace(_fmt, ## _args);       \
        }\
      }while(0)

#define CMDQ_ERR(_fmt, _args...)       UartSendTrace(_fmt, ## _args)

#define CMDQ_LOG(_fmt, _args...)       UartSendTrace(_fmt, ## _args)

#define CMDQ_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            UartSendTrace("BUG at %s:%d assert(%s)\n", __FILE__, __LINE__, #_con);\
        }\
    } while (0); \
    while(1);

#endif
#endif
