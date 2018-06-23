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

#ifndef _MI_VDF_DATATYPE_H_
#define _MI_VDF_DATATYPE_H_

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"


#define MI_VDF_SOURCE_NUM_MAX         16
#define MI_VDF_CHANNEL_MAX            128
#define MI_VDF_MAX_WIDTH              1920
#define MI_VDF_MAX_HEIGHT             1080
#define MI_VDF_MIN_WIDTH              320
#define MI_VDF_MIN_HEIGHT             180


#define MI_VDF_MD_RST_BUF_NUM_MAX     8
#define MI_VDF_OD_RST_BUF_NUM_MAX     16
// #define BOOL                          unsigned int  // 4 byte
#define MI_RET_SUCESS                 0

typedef void*                         MD_HANDLE;
typedef void*                         OD_HANDLE;
typedef MI_S32                        MI_VDF_CHANNEL;
typedef MI_S32                        MI_VDF_PORT;

typedef struct MI_VDF_Obj_s
{
    MI_U16 u16LtX;      //Horizontal position of window left-top point
    MI_U16 u16LtY;      //Vertical position of window left-top point
    MI_U16 u16RbX;      //Horizontal position of window right-bottom point
    MI_U16 u16RbY;      //Vertical position of window right-bottom point
    MI_U16 u16ImgW;     //the Width of the input image
    MI_U16 u16ImgH;     //the Hight of the input image
} MI_VDF_Obj_t;


typedef enum
{
    E_MI_VDF_COLOR_Y = 1,
    E_MI_VDF_COLOR_MAX
} MI_VDF_Color_e;

/*typedef struct MI_VDF_Rgn_s
{
    MI_U16 u16WideDiv;        //The number of divisions of window in horizontal direction
    MI_U16 u16HightDiv;       //The number of divisions of window in vertical direction
    MI_U8  u8Col;
    MI_U8  u8Row;
    MI_U32 u32Enable;         //1:enable，0:disable; 在col与row为0xFFFFFFFF时，该变量的bit0~bit30分别对应子窗口0~31
} MI_VDF_Rgn_t;


typedef struct MI_VDF_MdRgnAttr_s
{
    MI_U8  u8Enable;
    MI_U8  u8SizePerctThdMin;
    MI_U8  u8SizePerctThdMax;
    MI_U8  u8Sensitivity;
    MI_U16 u16LearnRate;
} MI_VDF_MDParamsIn_t;*/

typedef struct MI_VDF_MdStaticParam_s
{
    MI_U16 width;
    MI_U16 height;
    MI_U8  color;
    MDMB_MODE_e mb_size;
    MDSAD_OUT_CTRL_e sad_out_ctrl;
    MDROI_t roi_md;
    MDALG_MODE_e md_alg_mode;
} MI_VDF_MdStaticParam_t;

typedef struct MI_VDF_MdDynamicParam_s
{
    MI_U8  sensitivity;
    MI_U16 learn_rate;
    MI_U32 md_thr;
    MI_U32 obj_num_max;
} MI_VDF_MdDynamicParam_t;

typedef struct MI_VDF_MdAttr_s
{
    MI_U8  u8Enable;
    MI_U8  u8SrcChnNum;
    MI_U8  u8MdBufCnt;
    MI_U8  u8VDFIntvl;
    MI_U16 u16MdSadBufferSize;
    MDCCL_ctrl_t ccl_ctrl;
    //MDOBJ_DATA_t ccobj;
    MI_VDF_MdStaticParam_t  stMdStaticParamsIn;
    MI_VDF_MdDynamicParam_t stMdDynamicParamsIn;
} MI_VDF_MdAttr_t;

typedef enum
{
    E_MI_VDF_ODWINDOW_1X1 = 0,
    E_MI_VDF_ODWINDOW_2X2,
    E_MI_VDF_ODWINDOW_3X3,
    E_MI_VDF_ODWINDOW_MAX
} MI_VDF_ODWindow_e;


typedef struct MI_VDF_ODParamsIn_s
{
    MI_VDF_ODWindow_e endiv;
    MI_S32  s32ThdTamper;
    MI_S32  s32TamperBlkThd;
    MI_S32  s32MinDuration;
    MI_S32  s32Alpha;
    MI_S32  s32M;
} MI_VDF_ODParamsIn_t;


typedef struct MI_VDF_OdAttr_s
{
    MI_OD_param_t stOdDynamicParamsIn;
    MI_OD_static_param_t stOdStaticParamsIn;
    MI_U8 u8SrcChnNum;
    MI_U8 u8OdBufCnt;
    MI_U8 u8VDFIntvl;
    MI_U8 u8Enable;
} MI_VDF_OdAttr_t;

typedef enum
{
    E_MI_VDF_WORK_MODE_MD = 0,
    E_MI_VDF_WORK_MODE_OD,
    E_MI_VDF_WORK_MODE_MAX
} MI_VDF_WorkMode_e;

typedef union
{
    MI_VDF_MdAttr_t stMdAttr;
    MI_VDF_OdAttr_t stOdAttr;
} MI_VDF_WorkModeAttr_u;

typedef struct MI_VDF_ChnAttr_s
{
    MI_VDF_WorkMode_e enWorkMode;
    MI_VDF_WorkModeAttr_u unAttr;
} MI_VDF_ChnAttr_t;



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

/*typedef struct MI_VDF_MDParamsOut_s
{
    MI_U8 md_result;
    MI_U32 obj_cnt;
} MI_VDF_MDParamsOut_t;*/


typedef struct MI_MD_Result_s
{
    MI_U64 u64Pts;                //The PTS of Image
    MI_U8  u8Enable;              //=1表明该结果值有效
    MI_U8  u8Reading;             //=1表明该结果正在被应用层读取; 
                                  //=0表示正在等待被应用读取或可以被覆盖
    MDSAD_DATA_t* pstMdResultSad; //The MD result of SAD
    MDOBJ_DATA_t* pstMdResultObj; //The MD result of Obj
} MI_MD_Result_t;


typedef struct MI_OD_Result_s
{
    MI_U8  u8Enable;
    MI_U8  u8WideDiv;           //The number of divisions of window in horizontal direction
    MI_U8  u8HightDiv;          //The number of divisions of window in vertical direction
    MI_U8  u8DataLen;           //OD detect result readable size
    MI_U64 u64Pts;              //The PTS of Image
    MI_U8  u8Reading;           //表明该结果正在被应用读取中
    MI_S8  u8RgnAlarm[3][3];    //The OD result of the sub-window
} MI_OD_Result_t;


typedef struct MI_VDF_MdRstHdlList_s
{
    // each channel has a list used for backup MD detect result
    MI_VDF_CHANNEL  VdfChn;
    MD_HANDLE       pMdHandle;
    MI_U16          u16MdRstBufSize;
    MI_U8           u8MdWritePst;
    MI_U8           u8MdReadPst;
    MI_S8           s8MdDeltData;
    MI_U8           u8MdRstBufCnt;
    void*           pstMdRstList;
    void*           next;
} MI_VDF_MdRstHdlList_t;

typedef struct MI_VDF_OdRstHdlList_S
{
    // each channel has a list used for backup OD detect result
    MI_VDF_CHANNEL  VdfChn;
    OD_HANDLE       pOdHandle;
    MI_U8           u8OdWritePst;
    MI_U8           u8OdReadPst;
    MI_S8           s8OdDeltData;
    MI_U8           u8OdRstBufCnt;
    MI_OD_Result_t* pstOdResultList;
    void*           next;
} MI_VDF_OdRstHdlList_t;


typedef struct MI_VDF_MdResult_s
{
    // each channel has a list used for backup MD detect result
    MI_VDF_MdRstHdlList_t* pstMdRstHdlList;
} MI_VDF_MdResult_t;


typedef struct _MI_VDF_OdResult_s
{
    // each channel has a list used for backup OD detect result
    MI_VDF_OdRstHdlList_t* pstOdRstHdlList;
} MI_VDF_OdResult_t;


typedef union
{
    MI_MD_Result_t stMdResult;
    MI_OD_Result_t stOdResult;
} MI_VDF_Result_u;

typedef struct MI_VDF_Result_s
{
    MI_U8 u8SrcChnNum;
    MDALG_MODE_e md_alg_mode;
    MI_VDF_WorkMode_e enWorkMode;
    MI_VDF_Result_u unVdfResult;
} MI_VDF_Result_t;



//====================================================================
typedef struct MI_VDF_NodeList_s
{
    void*  phandle;
    MI_U8  u8SrcChnNum;
    MI_U8  u8FrameCnt;
    MI_U8  u8FrameInterval;
    MI_U32 u32Old_t;
    MI_U32 u32New_t;
    MI_S32 s32Ret;
    MI_VDF_CHANNEL   VdfChn;
    MI_VDF_ChnAttr_t stAttr;
    void* next;
} MI_VDF_NodeList_t;


typedef struct MI_VDF_EntryNode_s
{
    MI_VDF_NodeList_t* pstVdfNodeList;
} MI_VDF_EntryNode_t;


typedef struct MI_VDF_ChnStat_s
{
    MI_BOOL bStart;
    MI_U8   u32LeftPic;
    MI_U8   u32LeftRst;
} MI_VDF_ChnStat_t;

#endif///_MI_VDF_DATATYPE_H_
