
#ifndef __HAL_CMDQ_H__
#define __HAL_CMDQ_H__

#include "hal_cmdq_reg.h"

#if defined(HAL_K6_SIMULATE) || defined(HAL_I3_SIMULATE)
typedef enum
{
    HAL_EN_CMDQ_TYPE_IP0,
    HAL_EN_CMDQ_TYPE_MAX
} HALCmdqIPSupport_e;

#else
typedef enum
{
    HAL_EN_CMDQ_TYPE_IP0,
    HAL_EN_CMDQ_TYPE_IP1,
    HAL_EN_CMDQ_TYPE_IP2,
    HAL_EN_CMDQ_TYPE_IP3,
    HAL_EN_CMDQ_TYPE_IP4,
    HAL_EN_CMDQ_TYPE_MAX
} HALCmdqIPSupport_e;
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Macro of bit operations
//---------------------------------------------------------------------------

#define HAS_FLAG(flag, bit)        ((flag) & (bit))
#define SET_FLAG(flag, bit)        ((flag)|= (bit))
#define RESET_FLAG(flag, bit)      ((flag)&= (~(bit)))
#define SET_FLAG1(flag, bit)       ((flag)|  (bit))
#define RESET_FLAG1(flag, bit)     ((flag)&  (~(bit)))
////////////////////////////////////////////////
void HAL_CMDQ_SetBank(HALCmdqIPSupport_e eIpNum, u32 u32BankAddr);

//---------------------------------------------------------------------------
///set the element of _CMDQCtrl
///  .CMDQ_Enable
///  .CMDQ_Length_ReadMode
///  .CMDQ_Mask_Setting
//---------------------------------------------------------------------------
void HAL_CMDQ_Enable(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Stop(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Reset(HALCmdqIPSupport_e eIpNum);
bool HAL_CMDQ_Set_Mode(HALCmdqIPSupport_e eIpNum, u32 ModeSel);
void HAL_CMDQ_Set_Start_Pointer(HALCmdqIPSupport_e eIpNum, u32 StartAddr);
void HAL_CMDQ_Set_End_Pointer(HALCmdqIPSupport_e eIpNum, u32 EndAddr);
void HAL_CMDQ_Set_Offset_Pointer(HALCmdqIPSupport_e eIpNum, u32 OffsetAddr);
void HAL_CMDQ_Set_Ratio(HALCmdqIPSupport_e eIpNum, u32 Ratio);
void HAL_CMDQ_Reset_Soft_Interrupt(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Reset_Start_Pointer_bit(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Read_Soft_Interrupt(HALCmdqIPSupport_e eIpNum);
bool HAL_CMDQ_Read_Dec_Done(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///Trigger for update start pointer and end pointer
//---------------------------------------------------------------------------
void HAL_CMDQ_Start(HALCmdqIPSupport_e eIpNum);
u32 HAL_CMDQ_Read_Dummy_Register(HALCmdqIPSupport_e eIpNum) ;
s32 HAL_CMDQ_Write_Dummy_Register(HALCmdqIPSupport_e eIpNum, u32 DummyValue);
void HAL_CMDQ_Set_MIU_SELECT(HALCmdqIPSupport_e eIpNum, u32 miu_select);
u32 HAL_CMDQ_Read_Start_Pointer(HALCmdqIPSupport_e eIpNum);
u32 HAL_CMDQ_Read_End_Pointer(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///Set Previous Dummy Register bit to be 1(which means this CAF is already write to DRAM)
//---------------------------------------------------------------------------
void HAL_CMDQ_Write_Dummy_Register_release_polling(HALCmdqIPSupport_e eIpNum, u32 Write_bit);
u32 HAL_CMDQ_Error_Command(HALCmdqIPSupport_e eIpNum, u32 select_bit);
void HAL_CMDQ_Write_Pointer(HALCmdqIPSupport_e eIpNum, u32 Write_value);
//---------------------------------------------------------------------------
///Set rd_mi_radr_trig to grab current read address pointer(rd_mi_radr will be store in CMDQ_Rd_Mi_Radr)
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Read_Pointer(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///get the Write_Pointer(for multi-process, preventing write address is wrong(each process will start from Buffer_Start) ==> get current write_cmd ptr,next will write from here
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Get_Write_Pointer(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///Force trigger signal on wait bus. (good for debug), current sigbits 0-15
//---------------------------------------------------------------------------
void HAL_CMDQ_Trigger_Wait(HALCmdqIPSupport_e eIpNum, u32 sigbits);
//---------------------------------------------------------------------------
///Skip WR[0] WAIT[1], POLLEQ[2], POLLNEQ[3] commands. EX. 0xF skip those 4 cmds.
//---------------------------------------------------------------------------
void HAL_CMDQ_Skip_Commands(HALCmdqIPSupport_e eIpNum, u32 skipbits);
//---------------------------------------------------------------------------
///Real CMDQ reset
//---------------------------------------------------------------------------
void HAL_CMDQ_Reset2(HALCmdqIPSupport_e eIpNum, bool high);
//---------------------------------------------------------------------------
///Read misc status bits.
//---------------------------------------------------------------------------
u32 HAL_CMDQ_Read_Misc_Status(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Set_Debug_Step_Mode(HALCmdqIPSupport_e eIpNum, bool on);
u32 HAL_CMDQ_Get_Debug_Step_Mode(HALCmdqIPSupport_e eIpNum);
//---------------------------------------------------------------------------
///Trigger one step when debug-mode, step-mode enabled.
//---------------------------------------------------------------------------
void HAL_CMDQ_Debug_One_Step(HALCmdqIPSupport_e eIpNum);

void HAL_CMDQ_Set_Timer(HALCmdqIPSupport_e eIpNum, u32 time);
void HAL_CMDQ_SetISRMSK(HALCmdqIPSupport_e eIpNum, u32 nMakVal);
u32  HAL_CMDQ_Get_Dummy_Register_RiuAddr(HALCmdqIPSupport_e eIpNum);
bool HAL_CMDQ_Read_Idle_Done(HALCmdqIPSupport_e eIpNum);
u16 HAL_CMDQ_Read_Irq_Status(HALCmdqIPSupport_e eIpNum);
u16 HAL_CMDQ_Read_FinalIrq_Status(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Clear_IRQByFlag(HALCmdqIPSupport_e eIpNum, u16 u16IRQ);
void HAL_CMDQ_Print_All(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Enable_TriggerBusSample(HALCmdqIPSupport_e eIpNum);

u16 HAL_CMDQ_Read_TiggerBusFinalIrq(HALCmdqIPSupport_e eIpNum);
void HAL_CMDQ_Clear_TiggerBusIrq(HALCmdqIPSupport_e eIpNum, u16 nClrVal);
u32 HAL_CMDQ_Get_TriggerClr_Register_RiuAddr(HALCmdqIPSupport_e eIpNum);
s32 HAL_CMDQ_Clear_HwDummy_Register(HALCmdqIPSupport_e eIpNum);
#endif
