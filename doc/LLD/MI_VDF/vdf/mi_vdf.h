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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file    mi_vdf.h
/// @brief The vdf interface definition
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MI_VDF_H_
#define _MI_VDF_H_

//#include "mi_md.h"
//#include "mi_od.h"

#include "mi_common_datatype.h"

typedef void*               MD_HANDLE;
typedef void*               OD_HANDLE;
typedef MI_S32              MI_VDF_CHANNEL;  //定义视频侦测分析通道。
#define MI_VDF_CHANNEL_MAX  2048              //定义视频侦测分析最大通道的个数。
#define VDF_MAX_WIDTH       1920              //定义视频侦测分析通道的最大宽度。
#define VDF_MAX_HEIGHT      1080              //定义视频侦测分析通道的最大高度。
#define VDF_MIN_WIDTH       320               //定义视频侦测分析通道的最小宽度。
#define VDF_MIN_HEIGHT      180               //定义视频侦测分析通道的最小高度。


#define I2_CHANNEL_NUM_MAX            16      //I2最大Video source number
#define VDF_INPUT_CHN_NUM             0x00

#define VDF_MD_RST_BUF_NUM_MAX        16      //VDF模块保存多少路Video source的MD检测结果
#define VDF_OD_RST_BUF_NUM_MAX        16      //VDF模块保存多少路Video source的OD检测结果

typedef struct MI_VDF_OBJ_S
{
    MI_U16 u16Lt_x;     //Horizontal position of window left-top point
    MI_U16 u16Lt_y;     //Vertical position of window left-top point
    MI_U16 u16Rb_x;     //Horizontal position of window right-bottom point
    MI_U16 u16Rb_y;     //Vertical position of window right-bottom point
    MI_U16 u16ImgW;     //the Width of the input image
    MI_U16 u16ImgH;     //the Hight of the input image
}VDF_OBJ_S;


typedef enum
{
    VDF_Y = 1,
    VDF_COLOR_MAX
} VDFColor_E;

typedef struct MI_VDF_RGN_S
{
    MI_U16 u16W_div;          //The number of divisions of window in horizontal direction
    MI_U16 u16H_div;          //The number of divisions of window in vertical direction
    MI_U8  u8Col;             //子窗口行号
    MI_U8  u8Row;             //子窗口列号
    MI_U32 u32Enable;         //1:enable，0:disable; 在col与row为0xFFFFFFFF时，该变量的bit0~bit30分别对应子窗口0~31
}VDF_RGN_S;


typedef struct MI_VDF_MD_RGN_ATTR_S
{
    MI_U8  u8Enable;                //sub-window使能标志
    MI_U8  u8Size_perct_thd_min;    //相对于子窗口的最小像素百分比，范围[0,99]
    MI_U8  u8Size_perct_thd_max;    //相对于子窗口的最大像素百分比，范围[1,100],必须大于最小百分比
    MI_U8  u8Sensitivity;           //算法灵敏度， 范围[10,20,30,…..100]， 值越大越灵敏
    MI_U16 u16Learn_rate;           //单位毫秒，范围[1000,30000]，用于控制前端物体停止运动多久时，才作为背景画面
}VDF_MDParamsIn_S;


typedef struct MI_VDF_MD_ATTR_S
{
    VDF_OBJ_S stMDObjCfg;
    VDF_RGN_S stMDRgnSet;
    MDParamsIn_t stMDParamsIn;
    VDFColor_E enClrType;           //MD输入源的类型: VDF_Y = 1
    MI_U8  u8SrcChnNum;             //输入Video source源编号
    MI_U32 u32MdBufNum;             //MD 结果缓存帧个数  取值范围：[1, 16] 静态属性
    MI_U32 u32VDFIntvl;             //侦测间隔 取值范围：[0, 29]，以帧为单位 动态属性
}VDF_MD_ATTR_S;


typedef struct _ODParamsIn_t
{
    ODWindow_e endiv;              //OD窗口的类型: OD_WINDOW_3X3=3
    MI_S32  s32Thd_tamper;         //图像差异比例阀值: 3
    MI_S32  s32Tamper_blk_thd;     //图像被遮挡区域数量阀值: 1
    MI_S32  s32Min_duration;       //图像差异持续时间阀值: 15
    MI_S32  s32Alpha;              //控制生产参考图像的学习速率: 2
    MI_S32  s32M;                  //多少帧更新一次参考图像: 120
}ODParamsIn_t;

typedef struct MI_VDF_OD_ATTR_S
{
    VDF_OBJ_S stODObjCfg;
    VDF_RGN_S stODRgnSet;
    ODParamsIn_t stODParamsIn;  //区域属性，参见内部结构体
    VDFColor_E enClrType;           //OD输入源的类型: VDF_Y = 1
    MI_U8  u8SrcChnNum;             //输入Video source源编号
    MI_U32 u32OdBufNum;             //OD 结果缓存个数  取值范围：[1, 16] 静态属性
    MI_U32 u32VDFIntvl;             //侦测间隔  取值范围：[0, 29]，以帧为单位  动态属性
}VDF_OD_ATTR_S;

typedef enum MI_VDF_WORK_MODE_E
{
    VDF_WORK_MODE_MD = 0,       //移动侦测
    VDF_WORK_MODE_OD,           //遮挡侦测
    VDF_WORK_MODE_MAX
}VDF_WORK_MODE_E;

typedef union MI_VDF_WORK_MODE_ATTR_U
{
    VDF_MD_ATTR_S stMdAttr;     //移动侦测属性
    VDF_OD_ATTR_S stOdAttr;     //遮挡侦测属性
}VDF_WORK_MODE_ATTR_U;

typedef struct MI_VDF_CHN_ATTR_S
{
    VDF_WORK_MODE_E enWorkMode; //工作模式(移动侦测,遮挡侦测)  静态属性
    VDF_WORK_MODE_ATTR_U unAttr;//工作模式属性(移动侦测属性, 遮挡侦测属性)
}VDF_CHN_ATTR_S;



//======================= VDF Detect result =========================
//——MD -->MI_VDF_MD_RESULT_S
//   |——Vi-Chn0  ->MD_RESULT_HANDLE_LIST
//   |      |——Handle1 -->MD_RESULT_S
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-255
//   |      |
//   |      |——Handle2
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-255
//   |      |
//   |      |——Handle...
//   |
//   |——Vi-Chn1
//   |      |——Handle1 -->MD_RESULT_S
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-255
//   |      |
//   |      |——Handle2
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-255
//   |      |
//   |      |——Handle...
//   |
//   |——Vi-Chnx
//   |

//——OD -->MI_VDF_OD_RESULT_S
//   |——Vi-Chn0
//   |      |——Handle1 -->OD_RESULT_S
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-8
//   |      |
//   |      |——Handle2
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-8
//   |      |
//   |      |——Handle...
//   |
//   |——Vi-Chn1
//   |      |——Handle1 -->OD_RESULT_S
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-8
//   |      |
//   |      |——Handle2
//   |      |        |——sub-window-0
//   |      |        |——sub-window-1
//   |      |        |——...
//   |      |        |__sub-window-8
//   |      |
//   |      |——Handle...
//   |
//   |——Vi-Chnx
//   |


typedef struct _MD_RESULT_S
{
    MI_U8  u8Enable;
    MI_U8  u8W_div;             //The number of divisions of window in horizontal direction
    MI_U8  u8H_div;             //The number of divisions of window in vertical direction
    MI_U8  u8DataLen;			//MD detect result readable size
    MI_U64 u64Pts;              //The PTS of Image
    MI_U64 u64Md_result[32];    //The MD result of the sub-window
}MD_RESULT_S;

typedef struct _OD_RESULT_S
{
    MI_U8  u8Enable;
    MI_U8  u8W_div;             //The number of divisions of window in horizontal direction
    MI_U8  u8H_div;             //The number of divisions of window in vertical direction
    MI_U8  u8DataLen;			//OD detect result readable size
    MI_U64 u64Pts;              //The PTS of Image
    MI_S8  u8RgnAlarm[3][3];    //The OD result of the sub-window
}OD_RESULT_S;


typedef struct _MD_RESULT_HANDLE_LIST_S
{
    // each channel has a list used for backup MD detect result
    MI_VDF_CHANNEL VdfChn;
    MD_HANDLE     pMdHandle;
    MI_U16        u16WritePst;
    MI_U16        u16ReadPst;
	MI_S8         s8DeltData;
    MD_RESULT_S*  pstMdResultList;
    void* next;
	//MD_RESULT_HANDLE_LIST* next;
}MD_RESULT_HANDLE_LIST;

typedef struct _OD_RESULT_HANDLE_LIST_S
{
    // each channel has a list used for backup OD detect result
    MI_VDF_CHANNEL VdfChn;
    OD_HANDLE     pOdHandle;
    MI_U16        u16WritePst;
    MI_U16        u16ReadPst;
	MI_S8         s8DeltData;
    OD_RESULT_S*  pstOdResultList;
    void* next;
	//OD_RESULT_HANDLE_LIST* next;
}OD_RESULT_HANDLE_LIST;


typedef struct _MI_VDF_MD_RESULT_S
{
    // each channel has a list used for backup MD detect result
    MD_RESULT_HANDLE_LIST* pstMdRstHdlList;
}MI_VDF_MD_RESULT_S;


typedef struct _MI_VDF_OD_RESULT_S
{
    // each channel has a list used for backup OD detect result
    OD_RESULT_HANDLE_LIST* pstOdRstHdlList;
}MI_VDF_OD_RESULT_S;



typedef union _MI_VDF_RESULT_U
{
    MD_RESULT_S stMdResult;
    OD_RESULT_S stOdResult;
}VDF_RESULT_U;

typedef struct _MI_VDF_RESULT_S
{
    MI_U64 u64Pts;
    VDF_WORK_MODE_E enWorkMode;
    VDF_RESULT_U unVdfResult;
}MI_VDF_RESULT_S;



//====================================================================
typedef struct _VDF_NODE_LIST_S 
{
    void* phandle;
    MI_U8 u8FrameCnt;
    MI_U8 u8FrameInterval;
    MI_U32 u32Old_t;
    MI_U32 u32New_t;
    MI_S32 s32Ret;
    MI_VDF_CHANNEL  VdfChn;
    VDF_CHN_ATTR_S  stAttr;  
    void* next;
	//VDF_NODE_LIST_S* next;
}VDF_NODE_LIST_S;


typedef struct _MI_VDF_ENTRY_MODE
{
    VDF_NODE_LIST_S* pstVdfNodeList;
}MI_VDF_ENTRY_MODE_S;


typedef struct MI_VDF_CHN_STAT_S
{
    MI_BOOL bStart;
    MI_U8   u32LeftPic;
    MI_U8   u32LeftRst;
}VDF_CHN_STAT_S;

MI_S32 MI_VDF_Init(void);
MI_S32 MI_VDF_Uninit(void);
MI_S32 MI_VDF_CreateChn(MI_VDF_CHANNEL VdfChn, const VDF_CHN_ATTR_S* pstAttr);
MI_S32 MI_VDF_DestroyChn(MI_VDF_CHANNEL VdfChn);
MI_S32 MI_VDF_SetChnAttr(MI_VDF_CHANNEL VdfChn, const VDF_CHN_ATTR_S* pstAttr);
MI_S32 MI_VDF_GetChnAttr(MI_VDF_CHANNEL VdfChn, VDF_CHN_ATTR_S* pstAttr);
MI_S32 MI_VDF_EnableChn(MI_VDF_CHANNEL VdfChn, MI_U8 u8Col, MI_U8 u8Row, MI_U8 u8Enable);
MI_S32 MI_VDF_Run(VDF_WORK_MODE_E enWorkMode);
MI_S32 MI_VDF_Stop(VDF_WORK_MODE_E enWorkMode);
MI_S32 MI_VDF_GetResult(MI_VDF_CHANNEL VdfChn, MI_VDF_RESULT_S* pstVdfResult, MI_S32 s32MilliSec);
MI_S32 MI_VDF_PutResult(MI_VDF_CHANNEL VdfChn, MI_VDF_RESULT_S* pstVdfResult);
MI_S32 MI_VDF_GetLibVersion(MI_VDF_CHANNEL VdfChn, MI_U32* u32VDFVersion);




#endif///_MI_VDF_H_
