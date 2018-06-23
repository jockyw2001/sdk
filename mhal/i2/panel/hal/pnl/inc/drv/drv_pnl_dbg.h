////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_PNL_DBG_H__
#define __DRV_PNL_DBG_H__

#define PNL_DBG_LEVEL_NONE              0x00000000
#define PNL_DBG_LEVEL_DRV               0x00000001
#define PNL_DBG_LEVEL_HAL               0x00000002
#define PNL_DBG_LEVEL_IO                0x00000004
#define PNL_DBG_LEVEL_MHAL              0x00000008
#define PNL_DBG_LEVE_REG_RW             0x00000010
#define PNL_DBG_LEVEL_RW_PACKET         0x00000020

#ifndef __DRV_PNL_C__
extern u32 _gu32PnlDbgLevel;
#endif

#define PNL_DBG_ENABLE 1

#if PNL_DBG_ENABLE

#define PNL_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)

#define PNL_DBG(dbglv, _fmt, _args...)          \
    do                                          \
    if(_gu32PnlDbgLevel & dbglv)                   \
    {                                           \
        printk(KERN_INFO _fmt, ## _args);       \
    }while(0)

#define PNL_ERR(_fmt, _args...)                 \
    do{                                         \
        printk(KERN_INFO "\33[1;33m");          \
        printk(KERN_INFO _fmt, ## _args);       \
        printk(KERN_INFO"\33[m");               \
    }while(0)

#else

#define     PNL_ASSERT(arg)
#define     PNL_DBG(dbglv, _fmt, _args...)
#define     PNL_ERR( _fmt, _args...)
#endif


#endif
