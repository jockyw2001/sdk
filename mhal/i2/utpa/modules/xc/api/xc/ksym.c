#include "MsTypes.h"
#include "MsCommon.h"
#include "MLog.h"
#include "apiACP.h"
#include "apiAUDIO.h"
#include "apiCEC.h"
#include "apiDAC.h"
#include "apiDMS.h"
#include "apiDMX.h"
#include "apiDMX_tee.h"
#include "apiDMX_tsio.h"
#include "apiGFX.h"
#include "apiGOP.h"
#include "apiGOPSC_Ex.h"
#include "apiGOP_SC.h"
#include "apiGPD.h"
#include "apiGPD_v2.h"
#include "apiHDMITx.h"
#include "apiHDMITx2.h"
#include "apiJPEG.h"
#include "apiJPEG_v2.h"
#include "apiLD.h"
#include "drvMBX.h"
#include "apiMFE_v2.h"
#include "apiMHL.h"
#include "apiPNL.h"
#include "apiPNL_EX.h"
#include "apiSWI2C.h"
#include "apiVOIPMFE.h"
#include "apiXC.h"
#include "apiXC_Ace.h"
#include "apiXC_Ace_EX.h"
#include "apiXC_Adc.h"
#include "apiXC_EX.h"
#include "apiXC_Adc_EX.h"
#include "apiXC_Auto.h"
#include "apiXC_Auto_EX.h"
#include "apiXC_DWIN.h"
#include "apiXC_PCMonitor.h"
#include "apiXC_ModeParse.h"
#include "apiXC_DWIN_EX.h"
#include "apiXC_Dlc.h"
#include "apiXC_ModeParse_EX.h"
#include "apiXC_PCMonitor_EX.h"
#include "apiXC_VMark.h"
#include "apiXC_VMark_EX.h"
#include "asmCPU.h"
#include "drvAESDMA.h"
#include "drvCIPHER.h"
#include "drvAUDIO.h"
#include "drvAVD.h"
#include "drvBDMA.h"
#include "drvCA.h"
#include "drvCA_v2.h"
#include "drvCH34.h"
#include "drvCIPHER_v2.h"
#include "drvCLKM.h"
#include "drvCMAPool_v2.h"
#include "drvCMDQ.h"
#include "drvCPU.h"
#include "drvDDC2BI.h"
#include "drvDIP.h"
#include "drvDMD_ATSC.h"
#include "drvDMD_ATSC_v2.h"
#include "drvDMD_DTMB.h"
#include "drvDMD_DTMB_v2.h"
#include "drvDMD_EXTERN_MSB124x.h"
#include "drvDMD_EXTERN_MSB123xc.h"
#include "drvDMD_common.h"
#include "drvDMD_ISDBT.h"
#include "drvDMD_ISDBT_v2.h"
#include "drvDMD_VD_MBX.h"
#include "drvDSCMB.h"
#include "drvDSCMB_v2.h"
#include "drvEMMFlt.h"
#include "drvGPIO.h"
#include "drvHDCP.h"
#include "drvHVD.h"
#include "drvHWI2C.h"
#include "drvIPAUTH.h"
#include "drvIR.h"
#include "drvIRQ.h"
#include "drvLDM.h"
#include "drvMFC.h"
#include "drvMIU.h"
#include "drvMMIO.h"
#include "drvMPIF.h"
#include "drvMSPI.h"
#include "drvMVOP.h"
#include "drvMVOP_v2.h"
#include "drvNDS.h"
#include "drvNSK2.h"
#include "drvPARFLASH.h"
#include "drvPCMCIA.h"
#include "drvPM.h"
#include "drvPQ.h"
#include "drvPQ_cus.h"
#include "drvPVR_IframeLUT.h"
#include "drvPWM.h"
#include "drvPWS.h"
#include "drvRTC.h"
#include "drvRVD.h"
#include "drvSAR.h"
#include "drvSC.h"
#include "drvSEAL.h"
#include "drvSEM.h"
#include "drvSERFLASH.h"
#include "drvSMBX.h"
#include "drvSYS.h"
#include "drvTCF.h"
#include "drvTVEncoder.h"
#include "drvUART.h"
#include "drvURDMA.h"
#include "drvVBI.h"
#include "drvVIF.h"
#include "drvWBLE.h"
#include "drvWDT.h"
#include "drvXC_HDMI_if.h"
#include "drvXC_HDMI_if_EX.h"
#include "drv_IR_TX.h"
#include "drv_gop_sc.h"
#include "apiGFX.h"
#include "apiXC.h"
#include "drvCIPHER.h"
#include "drvDMD_common.h"
#include "drvLDM.h"
#include "drvSAR.h"
#include "drvTVEncoder.h"
#include "MsOS.h"
#include "apiAUDIO.h"
#include "apiDAC.h"
#include "apiDMS.h"
#include "apiDMX.h"
#include "apiGOP.h"
#include "apiPNL.h"
#include "apiXC.h"
#include "apiXC_DWIN.h"
#include "apiXC_Dlc.h"
#include "drvDMD_ATSC.h"
#include "drvMVOP.h"
#include "apiGOP.h"
#include "apiXC_DBC.h"
#include "apiMBX.h"
#include "cusSAR.h"
#ifdef __MLOG_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _MS_COMMON_H_
#ifndef MSOS_TYPE_LINUX_KERNEL
#ifndef CONFIG_MBOOT
#endif
#else
#endif
#ifdef MSOS_TYPE_ECOS
#endif
#ifdef CONFIG_ENABLE_MENUCONFIG
#endif
#ifdef MSOS_TYPE_LINUX
#elif defined(MSOS_TYPE_ECOS)
#elif defined(MSOS_TYPE_OPTEE)
#else
#ifndef MSOS_TYPE_LINUX_KERNEL
#if !defined(MSOS_TYPE_OPTEE)
#else
#endif
#endif
#endif
#ifdef CONFIG_ENABLE_MENUCONFIG
#endif
#ifndef DLL_PUBLIC
#endif
#ifndef DLL_LOCAL
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#else
#endif
#if 0
#ifdef __aeon__
#endif
#if defined(__mips__)
#endif
#endif
#if 0
#endif
#ifdef MSOS_TYPE_ECOS
#ifdef BLOCK_ASSERT
#else
#endif
#else
#ifdef MSOS_TYPE_LINUX_KERNEL
#ifdef BLOCK_ASSERT
#else
#endif
#else
#ifdef BLOCK_ASSERT
#else
#endif
#endif
#endif
#ifdef MSOS_TYPE_ECOS
#elif defined(MSOS_TYPE_OPTEE)
#else
#endif
#ifndef MIN
#endif
#ifndef MAX
#endif
#ifndef BIT
#endif
#ifdef REG_SCRIPT
#ifdef REG_SCRIPT_EN
#else
#endif
#endif
#endif
#ifdef __PRANA_DEVICE_H__
#ifndef POLLIN
#endif
#ifndef POLLPRI
#endif
#ifndef POLLOUT
#endif
#ifndef POLLHUP
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#endif
#endif
#ifdef __MSDLC_H
#ifdef _MSDLC_C_
#else
#endif
#if ENABLE_DLC
#if defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#endif
#if (ENABLE_DLC == DLC_8SEG_HIS_MODE)
#endif
#endif
#endif
#ifdef __MSFS_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _MS_IRQ_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _MS_OS_H_
#ifdef __cplusplus
#endif
#if defined(MSOS_TYPE_LINUX_KERNEL)
#endif
#ifdef CONFIG_MSOS_EVENTGROUP_MAX_SEL
#else
#endif
#if defined(__MIPS16E__)
#else
#endif
#ifndef SYMBOL_WEAK
#ifndef MSOS_TYPE_LINUX_KERNEL
#else
#endif
#endif
#ifdef MSOS_PERF_DEBUG
#else
#endif
#if defined (MSOS_TYPE_ECOS) || defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_NOS) || defined (MSOS_TYPE_UCOS) || defined(MSOS_TYPE_ATOMTHREADS) || defined (MSOS_TYPE_NUTTX) || defined (MSOS_TYPE_OPTEE)
#ifdef MSOS_TYPE_UCOS
#elif defined(MSOS_TYPE_ATOMTHREADS)
#elif defined(MSOS_TYPE_NUTTX)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#elif defined (MSOS_TYPE_LINUX_KERNEL)
#ifdef CHIP_I2
#endif
#elif defined (MSOS_TYPE_XXX)
#endif
EXPORT_SYMBOL(MsOS_Init);
EXPORT_SYMBOL(MsOS_CreateMemoryPool);
EXPORT_SYMBOL(MsOS_DeleteMemoryPool);
EXPORT_SYMBOL(MsOS_InfoMemoryPool);
EXPORT_SYMBOL(MsOS_AllocateMemory);
EXPORT_SYMBOL(MsOS_FreeMemory);
EXPORT_SYMBOL(MsOS_CreateTask);
EXPORT_SYMBOL(MsOS_DeleteTask);
EXPORT_SYMBOL(MsOS_YieldTask);
EXPORT_SYMBOL(MsOS_DelayTask);
EXPORT_SYMBOL(MsOS_DelayTaskUs);
EXPORT_SYMBOL(MsOS_DelayTaskUs_Poll);
EXPORT_SYMBOL(MsOS_ResumeTask);
EXPORT_SYMBOL(MsOS_SuspendTask);
EXPORT_SYMBOL(MsOS_InfoTaskID);
EXPORT_SYMBOL(MsOS_GetTaskStatus);
EXPORT_SYMBOL(MsOS_GetOSThreadID);
EXPORT_SYMBOL(MsOS_CreateMutex);
EXPORT_SYMBOL(MsOS_DeleteMutex);
EXPORT_SYMBOL(MsOS_ObtainMutex);
EXPORT_SYMBOL(MsOS_ReleaseMutex);
EXPORT_SYMBOL(MsOS_EnableCrossThreadReleaseMutex);
EXPORT_SYMBOL(MsOS_InfoMutex);
EXPORT_SYMBOL(MsOS_CreateSemaphore);
EXPORT_SYMBOL(MsOS_DeleteSemaphore);
EXPORT_SYMBOL(MsOS_ObtainSemaphore);
EXPORT_SYMBOL(MsOS_ReleaseSemaphore);
EXPORT_SYMBOL(MsOS_CreateEventGroup);
EXPORT_SYMBOL(MsOS_DeleteEventGroup);
EXPORT_SYMBOL(MsOS_SetEvent);
EXPORT_SYMBOL(MsOS_ClearEvent);
EXPORT_SYMBOL(MsOS_WaitEvent);
EXPORT_SYMBOL(MsOS_WaitEvent_Interrupt);
EXPORT_SYMBOL(MsOS_CreateTimer);
EXPORT_SYMBOL(MsOS_DeleteTimer);
EXPORT_SYMBOL(MsOS_StartTimer);
EXPORT_SYMBOL(MsOS_StopTimer);
EXPORT_SYMBOL(MsOS_ResetTimer);
EXPORT_SYMBOL(MsOS_GetSystemTime);
EXPORT_SYMBOL(MsOS_Timer_DiffTimeFromNow);
EXPORT_SYMBOL(MsOS_Timer_DiffTime);
EXPORT_SYMBOL(MsOS_AttachInterrupt);
EXPORT_SYMBOL(MsOS_AttachInterrupt_Shared);
EXPORT_SYMBOL(MsOS_DetachInterrupt);
EXPORT_SYMBOL(MsOS_DetachInterrupt_Shared);
EXPORT_SYMBOL(MsOS_EnableInterrupt);
EXPORT_SYMBOL(MsOS_DisableInterrupt);
EXPORT_SYMBOL(MsOS_CompleteInterrupt);
EXPORT_SYMBOL(MsOS_In_Interrupt);
EXPORT_SYMBOL(MsOS_DisableAllInterrupts);
EXPORT_SYMBOL(MsOS_RestoreAllInterrupts);
EXPORT_SYMBOL(MsOS_EnableAllInterrupts);
#if defined(__aeon__)
#if defined(MSOS_TYPE_NUTTX)
#else
#endif
#ifdef __AEONR2__
#endif
#else
#endif
#if defined (__arm__) || defined (__aarch64__)
#else
#if defined(MSOS_TYPE_NUTTX)
#else
#endif
#endif
EXPORT_SYMBOL(MsOS_CPU_DisableInterrupt);
EXPORT_SYMBOL(MsOS_CPU_EnableInterrupt);
EXPORT_SYMBOL(MsOS_CPU_RestoreInterrupt);
EXPORT_SYMBOL(MsOS_CPU_MaskAllInterrupt);
EXPORT_SYMBOL(MsOS_CPU_MaskInterrupt);
EXPORT_SYMBOL(MsOS_CPU_UnMaskInterrupt);
EXPORT_SYMBOL(MsOS_CPU_LockInterrupt);
EXPORT_SYMBOL(MsOS_CPU_UnLockInterrupt);
EXPORT_SYMBOL(MsOS_CPU_AttachInterrupt);
EXPORT_SYMBOL(MsOS_CPU_DetachInterrupt);
EXPORT_SYMBOL(MsOS_CPU_AttachException);
EXPORT_SYMBOL(MsOS_CPU_DetachExceptiont);
EXPORT_SYMBOL(MsOS_CPU_SetEBASE);
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_Dcache_Flush);
#else
EXPORT_SYMBOL(MsOS_Dcache_Flush);
#endif
EXPORT_SYMBOL(MsOS_Dcache_Flush_All);
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_Dcache_Invalidate);
#else
EXPORT_SYMBOL(MsOS_Dcache_Invalidate);
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_Dcache_Writeback);
#else
EXPORT_SYMBOL(MsOS_Dcache_Writeback);
#endif
#if defined(CHIP_T12) || defined(CHIP_T8) || defined(CHIP_J2) || defined(CHIP_A2) || defined(CHIP_A5) || defined(CHIP_A3) || defined(CHIP_Edison)
#endif
EXPORT_SYMBOL(MsOS_Sync);
EXPORT_SYMBOL(MsOS_MPool_SetDbgLevel);
EXPORT_SYMBOL(MsOS_MPool_IsInitialized);
EXPORT_SYMBOL(MsOS_MPool_Init);
EXPORT_SYMBOL(MsOS_MPool_Close);
#ifdef UFO_PUBLIC_HEADER_700
EXPORT_SYMBOL(MsOS_MPool_InfoMsg);
#else
EXPORT_SYMBOL(MsOS_MPool_InfoMsg);
#endif
EXPORT_SYMBOL(MsOS_MPool_VA2PA);
EXPORT_SYMBOL(MsOS_MPool_PA2KSEG1);
EXPORT_SYMBOL(MsOS_MPool_PA2KSEG0);
EXPORT_SYMBOL(MsOS_MPool_IsPA2KSEG1Mapped);
EXPORT_SYMBOL(MsOS_MPool_IsPA2KSEG0Mapped);
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Dcache_Flush);
#else
EXPORT_SYMBOL(MsOS_MPool_Dcache_Flush);
#endif
EXPORT_SYMBOL(MsOS_MPool_Dcache_Flush_All);
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MsOS_MPool_Mapping);
#elif defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Mapping);
#else
EXPORT_SYMBOL(MsOS_MPool_Mapping);
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_UnMapping);
#else
EXPORT_SYMBOL(MsOS_MPool_UnMapping);
#endif
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MsOS_MPool_Mapping_Dynamic);
#elif defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Mapping_Dynamic);
#else
EXPORT_SYMBOL(MsOS_MPool_Mapping_Dynamic);
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Kernel_Detect);
#else
EXPORT_SYMBOL(MsOS_MPool_Kernel_Detect);
#endif
#if defined(MSOS_TYPE_LINUX)
#ifdef ENABLE_KERNEL_DLMALLOC
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Add_PA2VARange);
#else
EXPORT_SYMBOL(MsOS_MPool_Add_PA2VARange);
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
EXPORT_SYMBOL(MsOS_MPool_Remove_PA2VARange);
#else
EXPORT_SYMBOL(MsOS_MPool_Remove_PA2VARange);
#endif
EXPORT_SYMBOL(MsOS_SHM_Init);
EXPORT_SYMBOL(MsOS_SHM_GetId);
EXPORT_SYMBOL(MsOS_SHM_FreeId);
#if defined (MSOS_TYPE_NOS)
#endif
EXPORT_SYMBOL(MsOS_VA2PA);
EXPORT_SYMBOL(MsOS_PA2KSEG0);
EXPORT_SYMBOL(MsOS_PA2KSEG1);
EXPORT_SYMBOL(MsOS_FlushMemory);
EXPORT_SYMBOL(MsOS_ReadMemory);
#if defined (MSOS_TYPE_LINUX_KERNEL)
#endif
#if defined (MS_DEBUG)
#elif defined (MS_OPTIMIZE)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _MS_TYPES_H_
#ifdef CONFIG_ENABLE_MENUCONFIG
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#elif defined(UFO_PUBLIC_HEADER_212)
#else
#if defined (__aarch64__)
#elif defined(MSOS_TYPE_NUTTX)
#else
#if (defined(CONFIG_PURE_SN) || defined(CONFIG_MBOOT))
#else
#endif
#endif
#endif
#ifdef NULL
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#ifndef true
#endif
#endif
#if !defined(TRUE) && !defined(FALSE)
#endif
#if defined(ENABLE) && (ENABLE!=1)
#else
#endif
#if defined(DISABLE) && (DISABLE!=0)
#else
#endif
#endif
#ifdef _MS_VERSION_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef STELLAR
#else
#endif
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef STELLAR
#else
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef _UFO_H_
#ifdef STELLAR
#else
#endif
#endif
#ifdef _UFO_H_
#endif
#ifdef _UFO_H_
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#ifdef UFO_XC_HDR
#endif
#endif
#ifdef __ULOG_H__
#ifdef __cplusplus
#endif
#if (defined CONFIG_UTOPIA_TEE || defined MBOOT || defined MSOS_TYPE_NOS || defined MSOS_TYPE_OPTEE )
#elif (defined MSOS_TYPE_ECOS)
#ifdef MS_DEBUG
#else
#endif
#elif (defined MSOS_TYPE_LINUX_KERNEL)
#elif (defined CONFIG_MLOG)
#elif (defined ANDROID)
#ifdef ALOG
#else
#endif
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_ACP_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _ACP_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_AUDIO_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _APICEC_H_
#ifdef __cplusplus
#endif
#if(HDMI_CEC_VERSION >= CEC_VERSION_13a)
#endif
#if(HDMI_CEC_VERSION >= CEC_VERSION_13a)
#endif
#if(HDMI_CEC_VERSION >= CEC_VERSION_13a)
#endif
#if(HDMI_CEC_VERSION >= CEC_VERSION_13a)
#endif
#if(HDMI_CEC_VERSION >= CEC_VERSION_13a)
#endif
#if ENABLE_CEC_MULTIPLE
#endif
#if ENABLE_CEC_MULTIPLE
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_DAC_H_
#ifdef __cplusplus
#endif
#ifndef SYMBOL_WEAK
#endif
EXPORT_SYMBOL(MApi_DAC_GetLibVer);
EXPORT_SYMBOL(MApi_DAC_GetInfo);
EXPORT_SYMBOL(MApi_DAC_GetStatus);
#ifdef UFO_XC_GETOUTPUTINTELACETIMING
EXPORT_SYMBOL(MApi_DAC_GetOutputInterlaceTiming);
#endif
EXPORT_SYMBOL(MApi_DAC_SetDbgLevel);
EXPORT_SYMBOL(MApi_DAC_Init);
EXPORT_SYMBOL(MApi_DAC_Enable);
EXPORT_SYMBOL(MApi_DAC_SetClkInv);
EXPORT_SYMBOL(MApi_DAC_SetYPbPrOutputTiming);
EXPORT_SYMBOL(MApi_DAC_SetOutputSource);
EXPORT_SYMBOL(MApi_DAC_SetOutputLevel);
EXPORT_SYMBOL(MApi_DAC_SetOutputSwapSel);
EXPORT_SYMBOL(MApi_DAC_OnOffSD);
EXPORT_SYMBOL(MApi_DAC_GetSDStatus);
EXPORT_SYMBOL(MApi_DAC_OnOffHD);
EXPORT_SYMBOL(MApi_DAC_GetHDStatus);
EXPORT_SYMBOL(MApi_DAC_ClkSel);
EXPORT_SYMBOL(MApi_DAC_DumpTable);
EXPORT_SYMBOL(MApi_DAC_Exit);
EXPORT_SYMBOL(MApi_DAC_SetIHalfOutput);
EXPORT_SYMBOL(MApi_DAC_SetQuartOutput);
EXPORT_SYMBOL(MApi_DAC_SetDacState);
EXPORT_SYMBOL(MApi_DAC_HotPlugDetect);
EXPORT_SYMBOL(Mapi_DAC_SetPowerState);
EXPORT_SYMBOL(MApi_DAC_SetWSSOnOff);
EXPORT_SYMBOL(MApi_DAC_GetWSSStatus);
EXPORT_SYMBOL(MApi_DAC_ResetWSSData);
EXPORT_SYMBOL(MApi_DAC_SetWSSOutput);
EXPORT_SYMBOL(MApi_DAC_EnableICT);
EXPORT_SYMBOL(MApi_DAC_SetVGAHsyncVsync);
#ifdef __cplusplus
#endif
#endif
#ifdef _DAC_V2_H_
#ifdef __cplusplus
#endif
#ifdef UFO_XC_GETOUTPUTINTELACETIMING
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_DMS_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_DMS_TYPE_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DMS_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __API_DMX_H__
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if 0
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if 0
#endif
#if 0
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef DMX_UTOPIA_20
#endif
#if 0
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __API_DMX_TEE_H__
#ifdef __cplusplus
#endif
#ifdef MSOS_TYPE_OPTEE
#endif
#ifdef MSOS_TYPE_NUTTX
#endif
#ifdef MSOS_TYPE_OPTEE
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __API_DMX_TSIO_H__
#ifdef __cplusplus
#endif
#ifdef DMX_UTOPIA_20
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DMX_TSIO_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DMX_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_GFX_H_
#ifdef __cplusplus
#endif
#if 0
#endif
EXPORT_SYMBOL(MApi_GFX_Init);
EXPORT_SYMBOL(MApi_GFX_GetConfig);
EXPORT_SYMBOL(MApi_GFX_GetClip);
EXPORT_SYMBOL(MApi_GFX_GetIntensity);
EXPORT_SYMBOL(MApi_GFX_GetTAGID);
EXPORT_SYMBOL(MApi_GFX_GetNextTAGID);
EXPORT_SYMBOL(MApi_GFX_GetInfo);
EXPORT_SYMBOL(MApi_GFX_GetGECaps);
EXPORT_SYMBOL(MApi_GFX_DrawLine);
EXPORT_SYMBOL(MApi_GFX_RectFill);
EXPORT_SYMBOL(MApi_GFX_TriFill);
EXPORT_SYMBOL(MApi_GFX_SpanFill);
EXPORT_SYMBOL(MApi_GFX_SetSrcBufferInfo);
EXPORT_SYMBOL(MApi_GFX_SetDstBufferInfo);
EXPORT_SYMBOL(MApi_GFX_SetROP2);
EXPORT_SYMBOL(MApi_GFX_SetSrcColorKey);
EXPORT_SYMBOL(MApi_GFX_SetDstColorKey);
EXPORT_SYMBOL(MApi_GFX_SetAlpha);
EXPORT_SYMBOL(MApi_GFX_EnableAlphaBlending);
EXPORT_SYMBOL(MApi_GFX_EnableDFBBlending);
EXPORT_SYMBOL(MApi_GFX_SetDFBBldFlags);
EXPORT_SYMBOL(MApi_GFX_SetDFBBldOP);
EXPORT_SYMBOL(MApi_GFX_SetDFBBldConstColor);
EXPORT_SYMBOL(MApi_GFX_BitBlt);
EXPORT_SYMBOL(MApi_GFX_BitBltEx);
EXPORT_SYMBOL(MApi_GFX_SetIntensity);
EXPORT_SYMBOL(MApi_GFX_SetClip);
EXPORT_SYMBOL(MApi_GFX_SetItalic);
EXPORT_SYMBOL(MApi_GFX_SetDither);
EXPORT_SYMBOL(MApi_GFX_SetOnePixelMode);
EXPORT_SYMBOL(MApi_GFX_SetNearestMode);
EXPORT_SYMBOL(MApi_GFX_SetMirror);
EXPORT_SYMBOL(MApi_GFX_SetDstMirror);
EXPORT_SYMBOL(MApi_GFX_SetRotate);
EXPORT_SYMBOL(MApi_GFX_SetTAGID);
EXPORT_SYMBOL(MApi_GFX_WaitForTAGID);
EXPORT_SYMBOL(MApi_GFX_SetNextTAGID);
EXPORT_SYMBOL(MApi_GFX_EnableVCmdQueue);
EXPORT_SYMBOL(MApi_GFX_SetVCmdBuffer);
EXPORT_SYMBOL(MApi_GE_SetVCmd_W_Thread);
EXPORT_SYMBOL(MApi_GE_SetVCmd_R_Thread);
EXPORT_SYMBOL(MApi_GFX_FlushQueue);
EXPORT_SYMBOL(MApi_GFX_PowerOn);
EXPORT_SYMBOL(MApi_GFX_PowerOff);
EXPORT_SYMBOL(MApi_GFX_SetPowerState);
EXPORT_SYMBOL(MApi_GFX_WriteProtect);
EXPORT_SYMBOL(MApi_GFX_SetPaletteOpt);
EXPORT_SYMBOL(MApi_GFX_DrawBitmap);
EXPORT_SYMBOL(MApi_GFX_TextOut);
EXPORT_SYMBOL(MApi_GFX_CharacterOut);
EXPORT_SYMBOL(MApi_GFX_TrapezoidFill);
EXPORT_SYMBOL(MApi_GFX_EnableTrapezoidAA);
EXPORT_SYMBOL(MApi_GFX_EnableTrapSubPixCorr);
EXPORT_SYMBOL(MApi_GFX_QueryTextDispLength);
EXPORT_SYMBOL(MApi_GFX_SetAlphaSrcFrom);
EXPORT_SYMBOL(MApi_GFX_SetAlphaBlending);
EXPORT_SYMBOL(MApi_GFX_QueryDFBBldCaps);
EXPORT_SYMBOL(MApi_GFX_SetDFBBldSrcColorMask);
EXPORT_SYMBOL(MApi_GFX_Line_Pattern_Reset);
EXPORT_SYMBOL(MApi_GFX_Set_Line_Pattern);
EXPORT_SYMBOL(MApi_GFX_BeginDraw);
EXPORT_SYMBOL(MApi_GFX_EndDraw);
EXPORT_SYMBOL(MApi_GFX_RectFrame);
EXPORT_SYMBOL(MApi_GFX_SetDC_CSC_FMT);
EXPORT_SYMBOL(MApi_GFX_SetPatchMode);
EXPORT_SYMBOL(MApi_GFX_GetBufferInfo);
EXPORT_SYMBOL(MApi_GFX_ClearFrameBufferByWord);
EXPORT_SYMBOL(MApi_GFX_ClearFrameBuffer);
EXPORT_SYMBOL(MApi_GFX_SetAlpha_ARGB1555);
EXPORT_SYMBOL(MApi_GFX_GetAlpha_ARGB1555);
EXPORT_SYMBOL(MApi_GFX_RegisterGetFontCB);
EXPORT_SYMBOL(MApi_GFX_RegisterGetBMPCB);
EXPORT_SYMBOL(MApi_GFX_SetAlphaCmp);
EXPORT_SYMBOL(MApi_GFX_SetDbgLevel);
EXPORT_SYMBOL(MApi_GFX_SetStrBltSckType);
EXPORT_SYMBOL(MApi_GFX_SetHK);
EXPORT_SYMBOL(MApi_GFX_GetHK);
EXPORT_SYMBOL(MApi_GFX_DrawOval);
EXPORT_SYMBOL(MApi_GFX_GetStatus);
EXPORT_SYMBOL(MApi_GFX_GetLibVer);
EXPORT_SYMBOL(MApi_GFX_GetAlignCaps);
EXPORT_SYMBOL(MDrv_GE_EnableAlphaBlending);
EXPORT_SYMBOL(MDrv_GE_SetAlphaBlending);
EXPORT_SYMBOL(MDrv_GE_PE_SetIntensity);
EXPORT_SYMBOL(MApi_GFX_SetTLBMode);
EXPORT_SYMBOL(MApi_GFX_SetTLBBaseADDR);
EXPORT_SYMBOL(MApi_GFX_SetTLBFlushTable);
EXPORT_SYMBOL(MApi_GFX_SetTLBTag);
EXPORT_SYMBOL(MApi_GFX_BitbltByTwoSourceBuffer);
EXPORT_SYMBOL(MApi_GE_Exit);
#ifdef __cplusplus
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef _API_GOP_H_
#ifdef __cplusplus
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
#else
#endif
#ifndef MAX_GWIN_FB_SUPPORT
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
#else
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
#else
#endif
#if (defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700))
#else
#endif
#if (defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700))
#else
#endif
#if (defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700))
#else
#endif
#if (defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700))
#else
#endif
EXPORT_SYMBOL(MApi_GOP_Init);
EXPORT_SYMBOL(MApi_GOP_GetInfo);
EXPORT_SYMBOL(MApi_GOP_GetStatus);
EXPORT_SYMBOL(MApi_GOP_SetDbgLevel);
EXPORT_SYMBOL(MApi_GOP_GetLibVer);
EXPORT_SYMBOL(MApi_GOP_InitByGOP);
EXPORT_SYMBOL(MApi_GOP_GWIN_SwitchGOP);
EXPORT_SYMBOL(MApi_GOP_Initialize_StretchWindow);
EXPORT_SYMBOL(MApi_GOP_SetGOPClk);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetCurrentGOP);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMaxGOPNum);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetGwinNum);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetTotalGwinNum);
EXPORT_SYMBOL(MApi_GOP_PowerOn);
EXPORT_SYMBOL(MApi_GOP_PowerOff);
EXPORT_SYMBOL(Mapi_GOP_GWIN_ResetGOP);
EXPORT_SYMBOL(MApi_GOP_GWIN_ResetPool);
EXPORT_SYMBOL(MApi_GOP_SetPowerState);
EXPORT_SYMBOL(MApi_GOP_SetGOPBWStrength);
EXPORT_SYMBOL(MApi_GOP_GetGOPBWStrength);
EXPORT_SYMBOL(MApi_GOP_SetGOPHStart);
EXPORT_SYMBOL(MApi_GOP_SetGOPBrightness);
EXPORT_SYMBOL(MApi_GOP_GetGOPBrightness);
EXPORT_SYMBOL(MApi_GOP_EnableLBCouple);
EXPORT_SYMBOL(MApi_GOP_VE_SetOutputTiming);
EXPORT_SYMBOL(MApi_GOP_Enable_VEOSD);
EXPORT_SYMBOL(MApi_GOP_MIXER_SetOutputTiming);
EXPORT_SYMBOL(MApi_GOP_MIXER_SetMIXER2OPOutputTiming);
EXPORT_SYMBOL(MApi_GOP_MIXER_EnableVfilter);
EXPORT_SYMBOL(MApi_GOP_MIXER_EnableOldBlendMode);
EXPORT_SYMBOL(MApi_GOP_RestoreFromVsyncLimitation);
EXPORT_SYMBOL(MApi_GOP_MIUSel);
EXPORT_SYMBOL(MApi_GOP_SetUVSwap);
EXPORT_SYMBOL(MApi_GOP_SetYCSwap);
EXPORT_SYMBOL(MApi_GOP_GetMIUSel);
EXPORT_SYMBOL(MApi_GOP_SetGOPContrast);
EXPORT_SYMBOL(MApi_GOP_GetGOPContrast);
EXPORT_SYMBOL(MApi_GOP_EnaVECapture);
EXPORT_SYMBOL(MApi_GOP_GetVECaptureState);
EXPORT_SYMBOL(MApi_GOP_VECaptureWaitOnFrame);
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
EXPORT_SYMBOL(MApi_GOP_SetConfig);
#else
EXPORT_SYMBOL(MApi_GOP_SetConfig);
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
EXPORT_SYMBOL(MApi_GOP_SetConfigEx);
#else
EXPORT_SYMBOL(MApi_GOP_SetConfigEx);
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
EXPORT_SYMBOL(MApi_GOP_GetConfigEx);
#else
EXPORT_SYMBOL(MApi_GOP_GetConfigEx);
#endif
EXPORT_SYMBOL(MApi_GOP_Set3DOSDMode);
EXPORT_SYMBOL(MApi_GOP_Set3D_LR_FrameExchange);
EXPORT_SYMBOL(MApi_GOP_Set_GPIO3DPin);
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
EXPORT_SYMBOL(MApi_GOP_GetChipCaps);
#else
EXPORT_SYMBOL(MApi_GOP_GetChipCaps);
#endif
EXPORT_SYMBOL(MApi_GOP_TestPattern);
EXPORT_SYMBOL(MApi_GOP_TestPatternAlpha_Enable);
EXPORT_SYMBOL(MApi_GOP_IsRegUpdated);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetLayerFromGOP);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetGOPFromLayer);
EXPORT_SYMBOL(MApi_GOP_GWIN_Enable);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableTransClr);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableTransClr_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableProgressive);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableProgressive_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_DeleteWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_ReleaseWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_DestroyWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_DeleteWinHVSize);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetLayer);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetLayer);
EXPORT_SYMBOL(MApi_GOP_GWIN_OutputColor);
EXPORT_SYMBOL(MApi_GOP_GWIN_OutputColor_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SwapOverlapWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetRelativeWinPrio);
EXPORT_SYMBOL(MApi_GOP_GWIN_Switch2Gwin);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetAlphaInverse);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetAlphaInverse_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetAlphaInverse);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetAlphaInverse_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPalette);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableMultiAlpha);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsMultiAlphaEnable);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetAlphaValue);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetScrollRate);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetScrollType);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetSWScrollBuffer);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFMT0TransClr);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFMT0TransClr_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetTransClr_8888);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetTransClr_8888_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableT3DMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetBlending);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetARGB1555_Alpha);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetDuplication);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetWinPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetWinPositionOffset);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetWinProperty);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetGOPDst);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetBlink);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFieldInver);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFieldInver_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_STRETCHWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetStretchWinPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetHDisplaySize);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetWinInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetTransClr);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetTransparentClr);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetTransClr_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetWinDispPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetHMirror);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetHMirror_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetVMirror);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetVMirror_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsMirrorOn);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsMirrorOn_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_HSCALE);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_HSCALE_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_VSCALE);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_VSCALE_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_HStretchMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_HStretchMode_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_VStretchMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_VStretchMode_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_TranspColorStretchMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_TranspColorStretchMode_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetForceWrite);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetBnkForceWrite);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetAlphaValue);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetWinPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetWinProperty);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetWinAttr);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetWinInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_UpdateRegOnceEx);
EXPORT_SYMBOL(MApi_GOP_GWIN_UpdateRegOnceEx2);
EXPORT_SYMBOL(MApi_GOP_GWIN_UpdateRegOnceByMask);
EXPORT_SYMBOL(MApi_GOP_TriggerRegWriteIn);
EXPORT_SYMBOL(MApi_GOP_TriggerRegWriteIn_Ex);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFadeInOut);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetGWinShared);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetGWinSharedCnt);
EXPORT_SYMBOL(MApi_GOP_GWIN_EnableTileMode);
EXPORT_SYMBOL(MApi_GOP_SetPINPON);
EXPORT_SYMBOL(MApi_GOP_SetGOPYUV);
EXPORT_SYMBOL(MApi_GOP_MIXER_SetMux);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetNewAlphaMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPreAlphaMode);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateWin2);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateWin_Assign_FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateWin_Assign_32FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateStaticWin);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateStaticWin2);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateStaticWin_Assign_FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateStaticWin_Assign_32FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsGwinExist);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFreeWinID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMAXWinID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetCurrentWinId);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMaxActiveGwin_v);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get_HSTART);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetActiveGWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMaxVEnd);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsAllGWINDisabled);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsGWINEnabled);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsEnabled);
EXPORT_SYMBOL(MApi_GOP_GWIN_CheckOpmodeIsOn);
EXPORT_SYMBOL(MApi_GOP_GWIN_CheckOpmodeIsOn_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetResolution);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetResolution_32FB);
EXPORT_SYMBOL(MApi_GOP_Exit);
EXPORT_SYMBOL(MApi_GOP_FB_SEL);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetDoubleHeap);
EXPORT_SYMBOL(MApi_GOP_GWIN_MapFB2Win);
EXPORT_SYMBOL(MApi_GOP_GWIN_Map32FB2Win);
EXPORT_SYMBOL(MApi_GOP_GWIN_Switch2FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_Switch2_32FB);
EXPORT_SYMBOL(MApi_GOP_Switch_GWIN_2_FB);
EXPORT_SYMBOL(MApi_GOP_SetR2YMatrix);
EXPORT_SYMBOL(MApi_GOP_Switch_GWIN_2_32FB);
EXPORT_SYMBOL(MApi_GOP_Switch_GWIN_2_FB_BY_ADDR);
EXPORT_SYMBOL(MApi_GOP_Switch_3DGWIN_2_FB_BY_ADDR);
EXPORT_SYMBOL(MApi_GOP_Switch_Multi_GWIN_2_FB_BY_ADDR);
EXPORT_SYMBOL(MApi_GOP_GWIN_DeleteFB);
EXPORT_SYMBOL(MApi_GOP_GWIN_Delete32FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBRegion);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBRegion);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBPosition);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetFBInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set32FBInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBInfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBAddr);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBAddr);
EXPORT_SYMBOL(MApi_GOP_GWIN_ClearFlipQueue);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMaxFBNum);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMax32FBNum);
EXPORT_SYMBOL(MApi_GOP_FB_Get);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMAXFBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetMAX32FBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_DestroyFB);
EXPORT_SYMBOL(MApi_GOP_GWIN_Destroy32FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFBFrom3rdSurf);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FBFrom3rdSurf);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFBbyStaticAddr);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FBbyStaticAddr);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFBbyStaticAddr2);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FBbyStaticAddr2);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFB2);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FB2);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFB2_EXT);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FB2_EXT);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetCurrentFBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetCurrent32FBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBfromGWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBfromGWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_IsFBExist);
EXPORT_SYMBOL(MApi_GOP_GWIN_Is32FBExist);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFreeFBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFree32FBID);
EXPORT_SYMBOL(MApi_GOP_GWIN_CreateFB);
EXPORT_SYMBOL(MApi_GOP_GWIN_Create32FB);
EXPORT_SYMBOL(MApi_GOP_GWIN_GetFBFmt);
EXPORT_SYMBOL(MApi_GOP_GWIN_Get32FBFmt);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPaletteOpt);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPaletteOpt_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPaletteRead);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPaletteRead_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_ReadPalette);
EXPORT_SYMBOL(MApi_GOP_GWIN_ReadPalette_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GSetPaletteOpt);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GSetPaletteOpt_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GSetPaletteRead);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GSetPaletteRead_EX);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GReadPalette);
EXPORT_SYMBOL(MApi_GOP_GWIN_2GReadPalette_EX);
EXPORT_SYMBOL(MApi_GOP_DWIN_Init);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetSourceSel);
EXPORT_SYMBOL(MApi_GOP_DWIN_CaptureOneFrame2);
EXPORT_SYMBOL(MApi_GOP_DWIN_EnableIntr);
EXPORT_SYMBOL(MApi_GOP_DWIN_CaptureOneFrame);
EXPORT_SYMBOL(MApi_GOP_DWIN_Enable);
EXPORT_SYMBOL(MApi_GOP_DWIN_GetWinProperty);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetWinProperty);
EXPORT_SYMBOL(MApi_GOP_DWIN_SelectSourceScanType);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetDataFmt);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetAlphaValue);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetAlphaSrc);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetAlphaInverse);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetUVSample);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetSkipFrame);
EXPORT_SYMBOL(MApi_GOP_DWIN_ClearIntr);
EXPORT_SYMBOL(MApi_GOP_DWIN_GetDWinIntInfo);
EXPORT_SYMBOL(MApi_GOP_DWIN_GetDWinIntInfoTimeout);
EXPORT_SYMBOL(MApi_GOP_SetClkForCapture);
EXPORT_SYMBOL(MApi_GOP_DWIN_GetIntrStatus);
EXPORT_SYMBOL(MApi_GOP_DWIN_EnableR2YCSC);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetUVSwap);
EXPORT_SYMBOL(MApi_GOP_GWIN_BeginDraw);
EXPORT_SYMBOL(MApi_GOP_GWIN_EndDraw);
EXPORT_SYMBOL(MApi_GOP_GWIN_Enable_BGWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_BGWIN);
EXPORT_SYMBOL(MApi_GOP_GWIN_Set_BGWIN_Alpha);
EXPORT_SYMBOL(MApi_GOP_DWIN_SetBufferPINPON);
EXPORT_SYMBOL(MApi_GOP_RegisterFBFmtCB);
EXPORT_SYMBOL(MApi_GOP_RegisterXCIsInterlaceCB);
EXPORT_SYMBOL(MApi_GOP_RegisterXCGetCapHStartCB);
EXPORT_SYMBOL(MApi_GOP_RegisterXCReduceBWForOSDCB);
EXPORT_SYMBOL(MApi_GOP_RegisterEventNotify);
EXPORT_SYMBOL(MApi_GOP_RegisterXCSetDwinInfo);
EXPORT_SYMBOL(MApi_GOP_OC_SetOCFBinfo);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetGPUTile);
EXPORT_SYMBOL(MApi_GOP_TLB_Enable);
EXPORT_SYMBOL(MApi_GOP_GWIN_UpdateRegOnce);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetMux);
EXPORT_SYMBOL(MApi_GOP_GWIN_OutputLayerSwitch);
EXPORT_SYMBOL(MApi_GOP_GWIN_SetPixelShift);
#ifdef __cplusplus
#endif
#endif
#ifdef _GOPSCD_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_GOPSC_EX_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_GOP_SC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _GOP_H_
#ifdef __cplusplus
#endif
#if (defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300))
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_GPD_H_
#ifdef __cplusplus
#endif
#if !defined(U8)
#endif
#if !defined(U16)
#endif
#if !defined(U32)
#endif
#if !defined(S8)
#endif
#if !defined(S16)
#endif
#if !defined(S32)
#endif
#if !defined(TRUE) && !defined(FALSE)
#endif
#ifndef GPD_SUPPORT_MGIF
#endif
#ifndef GPD_NO_SUPPORT_MGIF
#endif
#if 1
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_GPD_V2_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_HDMITX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_HDMITx_Init);
EXPORT_SYMBOL(MApi_HDMITx_Exit);
EXPORT_SYMBOL(MApi_HDMITx_TurnOnOff);
EXPORT_SYMBOL(MApi_HDMITx_EnablePacketGen);
EXPORT_SYMBOL(MApi_HDMITx_SetHDMITxMode);
EXPORT_SYMBOL(MApi_HDMITx_SetHDMITxMode_CD);
EXPORT_SYMBOL(MApi_HDMITx_SetTMDSOnOff);
EXPORT_SYMBOL(MApi_HDMITx_DisableTMDSCtrl);
EXPORT_SYMBOL(MApi_HDMITx_SetRBChannelSwap);
EXPORT_SYMBOL(MApi_HDMITx_Exhibit);
EXPORT_SYMBOL(MApi_HDMITx_ForceHDMIOutputMode);
EXPORT_SYMBOL(MApi_HDMITx_ForceHDMIOutputColorFormat);
EXPORT_SYMBOL(MApi_HDMITx_GetRxStatus);
EXPORT_SYMBOL(MApi_HDMITx_GetRxDCInfoFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_GetRxVideoFormatFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_GetVICListFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_GetDataBlockLengthFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_GetRxAudioFormatFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_EDID_HDMISupport);
EXPORT_SYMBOL(MApi_HDMITx_GetRxIDManufacturerName);
EXPORT_SYMBOL(MApi_HDMITx_GetBksv);
EXPORT_SYMBOL(MApi_HDMITx_GetAksv);
EXPORT_SYMBOL(MApi_HDMITx_GetEDIDData);
EXPORT_SYMBOL(MApi_HDMITx_GetRx3DStructureFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_GetColorFormatFromEDID);
EXPORT_SYMBOL(MApi_HDMITx_PKT_User_Define_Clear);
EXPORT_SYMBOL(MApi_HDMITx_PKT_User_Define);
EXPORT_SYMBOL(MApi_HDMITx_PKT_Content_Define);
EXPORT_SYMBOL(MApi_HDMITx_SetVideoOnOff);
EXPORT_SYMBOL(MApi_HDMITx_SetColorFormat);
EXPORT_SYMBOL(MApi_HDMITx_SetVideoOutputTiming);
EXPORT_SYMBOL(MApi_HDMITx_SetVideoOutputAsepctRatio);
EXPORT_SYMBOL(MApi_HDMITx_SetVideoOutputOverscan_AFD);
EXPORT_SYMBOL(MApi_HDMITx_SetVideoOutputOverscan_AFD_II);
EXPORT_SYMBOL(MApi_HDMITx_Set_VS_InfoFrame);
EXPORT_SYMBOL(MApi_HDMITx_SetAVIInfoExtColorimetry);
EXPORT_SYMBOL(MApi_HDMITx_SetAudioOnOff);
EXPORT_SYMBOL(MApi_HDMITx_SetAudioFrequency);
EXPORT_SYMBOL(MApi_HDMITx_SetAudioConfiguration);
EXPORT_SYMBOL(MApi_HDMITx_GetAudioCTS);
EXPORT_SYMBOL(MApi_HDMITx_MuteAudioFIFO);
EXPORT_SYMBOL(MApi_HDMITx_SetAudioSourceFormat);
EXPORT_SYMBOL(MApi_HDMITx_GetHdcpKey);
EXPORT_SYMBOL(MApi_HDMITx_SetHDCPOnOff);
EXPORT_SYMBOL(MApi_HDMITx_SetAVMUTE);
EXPORT_SYMBOL(MApi_HDMITx_GetAVMUTEStatus);
EXPORT_SYMBOL(MApi_HDMITx_GetHDCPStatus);
EXPORT_SYMBOL(MApi_HDMITx_HDCP_StartAuth);
EXPORT_SYMBOL(MApi_HDMITx_GetINTHDCPStatus);
EXPORT_SYMBOL(MApi_HDMITx_GetHDCP_PreStatus);
EXPORT_SYMBOL(MApi_HDMITx_UnHDCPRxControl);
EXPORT_SYMBOL(MApi_HDMITx_HDCPRxFailControl);
EXPORT_SYMBOL(MApi_HDMITx_SetAksv2R0Interval);
EXPORT_SYMBOL(MApi_HDMITx_IsHDCPRxValid);
EXPORT_SYMBOL(MApi_HDMITx_HDCP_RevocationKey_Check);
EXPORT_SYMBOL(MApi_HDMITx_HDCP_RevocationKey_List);
EXPORT_SYMBOL(MApi_HDMITx_GetLibVer);
EXPORT_SYMBOL(MApi_HDMITx_GetInfo);
EXPORT_SYMBOL(MApi_HDMITx_GetStatus);
EXPORT_SYMBOL(MApi_HDMITx_HDCP_IsSRMSignatureValid);
EXPORT_SYMBOL(MApi_HDMITx_SetDbgLevel);
EXPORT_SYMBOL(MApi_HDMITx_SetHPDGpioPin);
EXPORT_SYMBOL(MApi_HDMITx_AnalogTuning);
EXPORT_SYMBOL(MApi_HDMITx_DisableRegWrite);
EXPORT_SYMBOL(MApi_HDMITx_GetEDIDPhyAdr);
EXPORT_SYMBOL(MApi_HDMITx_SetCECOnOff);
EXPORT_SYMBOL(MApi_HDMITx_GetCECStatus);
EXPORT_SYMBOL(MApi_HDMITx_EdidChecking);
EXPORT_SYMBOL(MApi_HDMITx_RxBypass_Mode);
EXPORT_SYMBOL(MApi_HDMITx_Disable_RxBypass);
EXPORT_SYMBOL(MApi_HDMITx_GetChipCaps);
EXPORT_SYMBOL(MApi_HDMITx_SetPowerState);
EXPORT_SYMBOL(MApi_HDMITx_GetEdidDataBlocks);
EXPORT_SYMBOL(MApi_HDMITx_GetKSVList);
EXPORT_SYMBOL(MApi_HDMITx_GeneralCtrl);
EXPORT_SYMBOL(MApi_HDMITx_HDCP2AccessX74);
EXPORT_SYMBOL(MApi_HDMITx_HDCP2TxInit);
EXPORT_SYMBOL(MApi_HDMITx_HDCP2TxEnableEncrypt);
EXPORT_SYMBOL(MApi_HDMITx_HDCP2TxFillCipherKey);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_HDMITX_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_HDMITX_NDS_H_
#ifdef CUSTOMER_NDS
#endif
#if 0
#ifdef __cplusplus
#endif
#ifdef CUSTOMER_NDS
#ifdef __HDMI_H__
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _API_HDMITX_V2_H_
#ifdef __cplusplus
#endif
#if (defined(VANCLEEF_MCP) || defined(RAPTORS_MCP))
#endif
#if defined(VANCLEEF_MCP) || defined(RAPTORS_MCP)
#endif
#ifdef __cplusplus
#endif
#endif
#if defined (MSOS_TYPE_LINUX)
#ifdef FRHSL_H
#ifdef __cplusplus
#endif
#if 1
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#if defined (MSOS_TYPE_LINUX)
#ifdef FRHSL_SHM_H
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _API_JPEG_H_
#ifdef __cplusplus
#endif
#if SUPPORT_MPO_FORMAT
#endif
#if SUPPORT_MPO_FORMAT
#endif
#if SUPPORT_GET_FULL_IMAGE_INFO
#endif
#if SUPPORT_MPO_FORMAT
#endif
#if SUPPORT_EXIF_EXTRA_INFO
#endif
#if SUPPORT_MPO_FORMAT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_JPEG_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_LD_H_
#ifdef __cplusplus
#endif
#if LD_LOG_DBG
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_LD_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_MBX_H
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_MBX_Init);
EXPORT_SYMBOL(MApi_MBX_Enable);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_MFE_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_MHL_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_PANEL_H_
#ifdef __cplusplus
#endif
#ifdef _API_XC_PANEL_C_
#else
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#endif
#ifdef _MS_VERSION_H_
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !defined(CONFIG_MBOOT)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !(defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300))
#if !defined (__aarch64__)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
EXPORT_SYMBOL(MApi_PNL_GetLibVer);
EXPORT_SYMBOL(MApi_PNL_GetInfo);
EXPORT_SYMBOL(MApi_PNL_GetStatus);
EXPORT_SYMBOL(MApi_PNL_GetStatusEx);
EXPORT_SYMBOL(MApi_PNL_SetDbgLevel);
EXPORT_SYMBOL(MApi_PNL_IOMapBaseInit);
EXPORT_SYMBOL(MApi_PNL_PreInit);
#ifndef _API_XC_PANEL_C_
#endif
EXPORT_SYMBOL(MApi_PNL_Init_Ex);
EXPORT_SYMBOL(MApi_PNL_GetConfig);
EXPORT_SYMBOL(MApi_PNL_SetOutput);
EXPORT_SYMBOL(MApi_PNL_ChangePanelType);
EXPORT_SYMBOL(MApi_PNL_TCONMAP_DumpTable);
EXPORT_SYMBOL(MApi_PNL_TCONMAP_Power_Sequence);
EXPORT_SYMBOL(MApi_PNL_TCON_Count_Reset);
EXPORT_SYMBOL(MApi_PNL_TCON_Init);
EXPORT_SYMBOL(MApi_PNL_GetDstInfo);
EXPORT_SYMBOL(MApi_PNL_Control_Out_Swing);
EXPORT_SYMBOL(MApi_PNL_ForceSetPanelDCLK);
EXPORT_SYMBOL(MApi_PNL_ForceSetPanelHStart);
EXPORT_SYMBOL(MApi_PNL_SetOutputPattern);
EXPORT_SYMBOL(MApi_Mod_Calibration_Setting);
EXPORT_SYMBOL(MApi_Mod_Do_Calibration);
EXPORT_SYMBOL(MApi_BD_LVDS_Output_Type);
EXPORT_SYMBOL(MApi_PNL_SetLPLLTypeExt);
EXPORT_SYMBOL(MApi_PNL_Init_MISC);
EXPORT_SYMBOL(MApi_PNL_GetMiscStatus);
EXPORT_SYMBOL(MApi_PNL_MOD_OutputConfig_User);
EXPORT_SYMBOL(MApi_PNL_MOD_OutputChannelOrder);
EXPORT_SYMBOL(MApi_PNL_HWLVDSReservedtoLRFlag);
EXPORT_SYMBOL(MApi_MOD_PVDD_Power_Setting);
EXPORT_SYMBOL(MApi_PNL_SetSSC_En);
EXPORT_SYMBOL(MApi_PNL_SetSSC_Fmodulation);
EXPORT_SYMBOL(MApi_PNL_SetSSC_Rdeviation);
EXPORT_SYMBOL(MApi_PNL_SetOSDSSC_En);
EXPORT_SYMBOL(MApi_PNL_SetOSDSSC_Fmodulation);
EXPORT_SYMBOL(MApi_PNL_SkipTimingChange);
EXPORT_SYMBOL(MApi_PNL_PreSetModeOn);
EXPORT_SYMBOL(MApi_PNL_OverDriver_Init);
EXPORT_SYMBOL(MApi_PNL_OverDriver_Setting);
EXPORT_SYMBOL(MApi_PNL_OverDriver_Enable);
EXPORT_SYMBOL(MApi_PNL_SetFilePath);
EXPORT_SYMBOL(MApi_PNL_Get_TCON_Capability);
EXPORT_SYMBOL(MApi_PNL_SetPairSwap);
EXPORT_SYMBOL(MApi_PNL_SetExt_LPLL_Type);
EXPORT_SYMBOL(MApi_PNL_CalExtLPLLSETbyDClk);
EXPORT_SYMBOL(MApi_PNL_EnableInternalTermination);
EXPORT_SYMBOL(MApi_PNL_SetPowerState);
EXPORT_SYMBOL(MApi_PNL_SetDiffSwingLevel);
EXPORT_SYMBOL(MApi_PNL_OutputDeviceHandshake);
EXPORT_SYMBOL(MApi_PNL_OutputDeviceOCHandshake);
EXPORT_SYMBOL(MApi_PNL_SetOutputInterlaceTiming);
EXPORT_SYMBOL(MApi_PNL_GetPanelData);
EXPORT_SYMBOL(MApi_PNL_DumpPanelData);
EXPORT_SYMBOL(MApi_PNL_SetSSC);
EXPORT_SYMBOL(MApi_PNL_GetPanelOnTiming);
EXPORT_SYMBOL(MApi_PNL_GetPanelOffTiming);
EXPORT_SYMBOL(MApi_PNL_GetPanelDimCtrl);
EXPORT_SYMBOL(MApi_PNL_GetAllGammaTbl);
EXPORT_SYMBOL(MApi_PNL_EnablePanel);
EXPORT_SYMBOL(MApi_PNL_SetGammaTbl);
EXPORT_SYMBOL(MApi_PNL_GetGammaTbl);
EXPORT_SYMBOL(MApi_PNL_SetGammaValue);
EXPORT_SYMBOL(MApi_PNL_Check_VBY1_Handshake_Status);
EXPORT_SYMBOL(MApi_PNL_SetVideoHWTraining);
EXPORT_SYMBOL(MApi_PNL_SetOSDHWTraining);
EXPORT_SYMBOL(MApi_PNL_GetVideoHWTraining_Status);
EXPORT_SYMBOL(MApi_PNL_GetOSDHWTraining_Status);
EXPORT_SYMBOL(MApi_PNL_GetOutputInterlaceTiming);
EXPORT_SYMBOL(MApi_PNL_Setting);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_PANEL_EX_H_
#ifdef __cplusplus
#endif
#ifdef _MS_VERSION_H_
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !(defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300))
#if !defined (__aarch64__)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
EXPORT_SYMBOL(MApi_PNL_EX_GetLibVer);
EXPORT_SYMBOL(MApi_PNL_EX_GetInfo);
EXPORT_SYMBOL(MApi_PNL_EX_GetStatus);
EXPORT_SYMBOL(MApi_PNL_EX_GetStatusEx);
EXPORT_SYMBOL(MApi_PNL_EX_SetDbgLevel);
EXPORT_SYMBOL(MApi_PNL_EX_IOMapBaseInit);
EXPORT_SYMBOL(MApi_PNL_EX_PreInit);
#ifndef _API_XC_PANEL_EX_C_
#endif
EXPORT_SYMBOL(MApi_PNL_EX_Init_Ex);
EXPORT_SYMBOL(MApi_PNL_EX_SetOutput);
EXPORT_SYMBOL(MApi_PNL_EX_ChangePanelType);
EXPORT_SYMBOL(MApi_PNL_EX_TCONMAP_DumpTable);
EXPORT_SYMBOL(MApi_PNL_EX_TCONMAP_Power_Sequence);
EXPORT_SYMBOL(MApi_PNL_EX_TCON_Count_Reset);
EXPORT_SYMBOL(MApi_PNL_EX_TCON_Init);
EXPORT_SYMBOL(MApi_PNL_EX_GetDstInfo);
EXPORT_SYMBOL(MApi_PNL_EX_Control_Out_Swing);
EXPORT_SYMBOL(MApi_PNL_EX_ForceSetPanelDCLK);
EXPORT_SYMBOL(MApi_PNL_EX_ForceSetPanelHStart);
EXPORT_SYMBOL(MApi_PNL_EX_SetOutputPattern);
EXPORT_SYMBOL(MApi_PNL_EX_Mod_Calibration_Setting);
EXPORT_SYMBOL(MApi_PNL_EX_Mod_Do_Calibration);
EXPORT_SYMBOL(MApi_PNL_EX_BD_LVDS_Output_Type);
EXPORT_SYMBOL(MApi_PNL_EX_SetLPLLTypeExt);
EXPORT_SYMBOL(MApi_PNL_EX_Init_MISC);
EXPORT_SYMBOL(MApi_PNL_EX_MOD_OutputConfig_User);
EXPORT_SYMBOL(MApi_PNL_EX_HWLVDSReservedtoLRFlag);
EXPORT_SYMBOL(MApi_PNL_EX_MOD_PVDD_Power_Setting);
EXPORT_SYMBOL(MApi_PNL_EX_SetSSC_En);
EXPORT_SYMBOL(MApi_PNL_EX_SetSSC_Fmodulation);
EXPORT_SYMBOL(MApi_PNL_EX_SetSSC_Rdeviation);
EXPORT_SYMBOL(MApi_PNL_EX_SkipTimingChange);
EXPORT_SYMBOL(MApi_PNL_EX_OverDriver_Init);
EXPORT_SYMBOL(MApi_PNL_EX_OverDriver_Enable);
EXPORT_SYMBOL(MApi_PNL_EX_Get_TCON_Capability);
EXPORT_SYMBOL(MApi_PNL_EX_SetPairSwap);
EXPORT_SYMBOL(MApi_PNL_EX_CalExtLPLLSETbyDClk);
EXPORT_SYMBOL(MApi_PNL_EX_SetDiffSwingLevel);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_PANEL_V2_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_SWI2C_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_VDEC_EX_H_
#ifndef UFO_PUBLIC_HEADER_700
#endif
#ifndef DLL_PACKED
#endif
#ifdef __cplusplus
#endif
#if (!defined(MSOS_TYPE_NUTTX) && !defined(MSOS_TYPE_OPTEE)) || defined(SUPPORT_X_MODEL_FEATURE)
#if defined(UFO_PUBLIC_HEADER_300)
#elif defined(UFO_PUBLIC_HEADER_212)
#elif defined(UFO_PUBLIC_HEADER_500_3)
#else
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700)||defined(UFO_PUBLIC_HEADER_500)
#endif
#endif
#ifndef VDEC_CAP_SYSTEM_PREGET_FB_MEM_USAGE_SIZE
#endif
#ifndef VDEC_CAP_SYSTEM_PREGET_FB_MEM_USAGE
#endif
#ifdef STELLAR
#endif
#ifdef AUTO_ASSIGN_FRAMEBUFFER
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#ifdef _USE_MDRV_VDEC_
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#ifdef _USE_MDRV_VDEC_
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if (defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_500_3))
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#ifdef VDEC_CAP_DV_OTT_API
#endif
#ifdef VDEC_CAP_DV_OTT_API
#ifdef VDEC_CAP_DV_OTT_API_V2
#else
#endif
#endif
#ifdef VDEC_CAP_HEVC_HDR
#endif
#if !(defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_500_3))
#endif
#ifdef VDEC_CAP_HEVC_HDR_V2
#endif
#ifdef VDEC_CAP_HEVC_HDR_V3
#endif
#ifdef VDEC_CAP_FRAME_INFO_EXT_V4
#endif
#ifdef VDEC_CAP_FRAME_INFO_EXT_V5
#endif
#ifdef VDEC_CAP_FRAME_INFO_EXT_V6
#endif
#ifdef VDEC_CAP_FRAME_INFO_EXT_V7
#ifdef VDEC_CAP_FRAME_INFO_EXT_V7_1
#endif
#endif
#ifdef VDEC_CAP_SYSTEM_PREGET_API
#endif
#ifdef AUTO_ASSIGN_FRAMEBUFFER
#endif
#else
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __APIVENC_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_VOIP_MFE_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_H_
#ifdef __cplusplus
#endif
#ifdef MVIDEO_C
#else
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#elif defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#elif defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#ifndef UFO_SUPPORT_NEW_IRQ
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3) ||defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3) ||defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#ifdef UFO_XC_FORCEWRITE_V2
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_XC_AUTO_DOWNLOAD)
#endif
#ifdef UFO_XC_HDR
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef STELLAR
#else
#endif
#if defined (UFO_PUBLIC_HEADER_212) ||defined (UFO_PUBLIC_HEADER_500_3) ||defined (UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#ifdef UFO_XC_ZORDER
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300)
#if API_XCSTATUS_EX_VERSION >= 9
#endif
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
#endif
#ifdef UFO_XC_TEST_PATTERN
#endif
#ifdef UFO_XC_AUTO_DOWNLOAD
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
#ifdef UFO_XC_HDMI_4K2K_DMA
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef STELLAR
#endif
#endif
#endif
#ifdef UFO_XC_HDR
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#elif defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500) ||defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500)
#endif
#if !defined (__aarch64__)
#endif
#if !defined (__aarch64__)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !defined (__aarch64__)
#endif
#if defined(UFO_PUBLIC_HEADER_700) ||defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_300)
#if defined(CONFIG_PURE_SN)
#endif
#else
#endif
#ifdef UFO_XC_SCAN_TYPE
#endif
#if !defined (__aarch64__)
#endif
#if !defined (__aarch64__)
#endif
#if !defined (__aarch64__)
#endif
EXPORT_SYMBOL(MApi_XC_GetLibVer);
EXPORT_SYMBOL(MApi_XC_GetInfo);
EXPORT_SYMBOL(MApi_XC_GetStatus);
EXPORT_SYMBOL(MApi_XC_GetStatusEx);
EXPORT_SYMBOL(MApi_XC_GetStatusNodelay);
EXPORT_SYMBOL(MApi_XC_SetDbgLevel);
EXPORT_SYMBOL(MApi_XC_FPLLDbgMode);
EXPORT_SYMBOL(MApi_XC_FPLLCustomerMode);
EXPORT_SYMBOL(MDrv_XC_SetIOMapBase);
EXPORT_SYMBOL(MApi_XC_Init);
EXPORT_SYMBOL(MApi_XC_GetConfig);
EXPORT_SYMBOL(MApi_XC_Init_MISC);
EXPORT_SYMBOL(MApi_XC_GetMISCStatus);
EXPORT_SYMBOL(MApi_XC_GetCapability);
EXPORT_SYMBOL(MApi_XC_GetChipCaps);
EXPORT_SYMBOL(MApi_XC_Exit);
EXPORT_SYMBOL(MApi_XC_SetDynamicScaling);
EXPORT_SYMBOL(MApi_XC_Set_DynamicScalingFlag);
EXPORT_SYMBOL(MApi_XC_GetDynamicScalingStatus);
#ifdef UFO_XC_SWDS_SW_FILM_ENABLE
#endif
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MApi_XC_Get_DNRBaseOffset);
#else
EXPORT_SYMBOL(MApi_XC_Get_DNRBaseOffset);
#endif
EXPORT_SYMBOL(MApi_XC_Get_FrameNumFactor);
EXPORT_SYMBOL(MApi_XC_Get_FRCMFrameNum);
EXPORT_SYMBOL(MApi_XC_SetWindow);
EXPORT_SYMBOL(MApi_XC_SetDualWindow);
EXPORT_SYMBOL(MApi_XC_SetTravelingWindow);
EXPORT_SYMBOL(MApi_XC_SetInputSource);
#ifdef UFO_PUBLIC_HEADER_212
#endif
EXPORT_SYMBOL(MApi_XC_IsYUVSpace);
EXPORT_SYMBOL(MApi_XC_IsMemoryFormat422);
EXPORT_SYMBOL(MApi_XC_EnableForceRGBin);
EXPORT_SYMBOL(MApi_XC_EnableMirrorModeEx);
EXPORT_SYMBOL(MApi_XC_GetMirrorModeTypeEx);
EXPORT_SYMBOL(MApi_XC_Is_SupportSWDS);
EXPORT_SYMBOL(MApi_XC_SetDSInfo);
EXPORT_SYMBOL(MApi_XC_GetDSInfo);
EXPORT_SYMBOL(MApi_XC_GetSWDSIndex);
EXPORT_SYMBOL(MApi_XC_GetSyncStatus);
EXPORT_SYMBOL(MApi_XC_WaitOutputVSync);
EXPORT_SYMBOL(MApi_XC_WaitInputVSync);
EXPORT_SYMBOL(MApi_XC_SetHdmiSyncMode);
EXPORT_SYMBOL(MApi_XC_GetHdmiSyncMode);
EXPORT_SYMBOL(MApi_XC_SetRepWindow);
EXPORT_SYMBOL(MApi_XC_SetSkipDisableOPWriteOffInFPLL);
EXPORT_SYMBOL(MApi_XC_Set_OPWriteOffEnable);
EXPORT_SYMBOL(MApi_XC_ForceSet_OPWriteOffEnable);
EXPORT_SYMBOL(MApi_XC_Set_OPWriteOffEnableToReg);
EXPORT_SYMBOL(MApi_XC_Get_OPWriteOffEnable);
EXPORT_SYMBOL(MApi_XC_SkipOPWriteOffInSetWindow);
EXPORT_SYMBOL(MApi_XC_GetSkipOPWriteOffInSetWindow);
EXPORT_SYMBOL(MApi_XC_SetDispWinToReg);
EXPORT_SYMBOL(MApi_XC_GetDispWinFromReg);
EXPORT_SYMBOL(MApi_XC_FreezeImg);
EXPORT_SYMBOL(MApi_XC_IsFreezeImg);
EXPORT_SYMBOL(MApi_XC_GenerateBlackVideoForBothWin);
EXPORT_SYMBOL(MApi_XC_Set_BLSK);
EXPORT_SYMBOL(MApi_XC_GenerateBlackVideo);
EXPORT_SYMBOL(MApi_XC_IsBlackVideoEnable);
EXPORT_SYMBOL(MApi_XC_EnableFrameBufferLess);
EXPORT_SYMBOL(MApi_XC_IsCurrentFrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_EnableRequest_FrameBufferLess);
EXPORT_SYMBOL(MApi_XC_IsCurrentRequest_FrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_Get_3D_HW_Version);
EXPORT_SYMBOL(MApi_XC_Get_3D_IsSupportedHW2DTo3D);
EXPORT_SYMBOL(MApi_XC_Set_3D_Mode);
EXPORT_SYMBOL(MApi_XC_Set_3D_MainWin_FirstMode);
EXPORT_SYMBOL(MApi_XC_Set_3D_LR_Frame_Exchg);
EXPORT_SYMBOL(MApi_XC_3D_Is_LR_Frame_Exchged);
EXPORT_SYMBOL(MApi_XC_Get_3D_Input_Mode);
EXPORT_SYMBOL(MApi_XC_Get_3D_Output_Mode);
EXPORT_SYMBOL(MApi_XC_Get_3D_Panel_Type);
EXPORT_SYMBOL(MApi_XC_Get_3D_MainWin_First);
EXPORT_SYMBOL(MApi_XC_3DMainSub_IPSync);
EXPORT_SYMBOL(MApi_XC_Set_3D_VerVideoOffset);
EXPORT_SYMBOL(MApi_XC_Get_3D_VerVideoOffset);
EXPORT_SYMBOL(MApi_XC_Is3DFormatSupported);
EXPORT_SYMBOL(MApi_XC_Set_3D_HShift);
EXPORT_SYMBOL(MApi_XC_Enable_3D_LR_Sbs2Line);
EXPORT_SYMBOL(MApi_XC_Get_3D_HShift);
EXPORT_SYMBOL(MApi_XC_Set_3D_HW2DTo3D_Buffer);
EXPORT_SYMBOL(MApi_XC_Set_3D_HW2DTo3D_Parameters);
EXPORT_SYMBOL(MApi_XC_Get_3D_HW2DTo3D_Parameters);
EXPORT_SYMBOL(MApi_XC_Set_3D_Detect3DFormat_Parameters);
EXPORT_SYMBOL(MApi_XC_Get_3D_Detect3DFormat_Parameters);
EXPORT_SYMBOL(MApi_XC_Detect3DFormatByContent);
EXPORT_SYMBOL(MApi_XC_DetectNL);
EXPORT_SYMBOL(MApi_XC_3D_PostPQSetting);
EXPORT_SYMBOL(MApi_XC_Set_3D_FPInfo);
EXPORT_SYMBOL(MApi_XC_EnableAutoDetect3D);
EXPORT_SYMBOL(MApi_XC_GetAutoDetect3DFlag);
EXPORT_SYMBOL(MApi_XC_Set_3D_SubWinClk);
EXPORT_SYMBOL(MApi_XC_3D_Is_LR_Sbs2Line);
EXPORT_SYMBOL(MApi_SC_3D_Is_Skip_Default_LR_Flag);
EXPORT_SYMBOL(MApi_XC_3D_Enable_Skip_Default_LR_Flag);
EXPORT_SYMBOL(MApi_XC_Mux_Init);
EXPORT_SYMBOL(MApi_XC_Mux_SourceMonitor);
EXPORT_SYMBOL(MApi_XC_Mux_CreatePath);
EXPORT_SYMBOL(MApi_XC_Mux_DeletePath);
EXPORT_SYMBOL(MApi_XC_Mux_EnablePath);
EXPORT_SYMBOL(MApi_XC_Mux_TriggerPathSyncEvent);
EXPORT_SYMBOL(MApi_XC_Mux_TriggerDestOnOffEvent);
EXPORT_SYMBOL(MApi_XC_Mux_OnOffPeriodicHandler);
EXPORT_SYMBOL(MApi_XC_Mux_GetPathInfo);
EXPORT_SYMBOL(MApi_XC_Mux_SetSupportMhlPathInfo);
EXPORT_SYMBOL(MApi_XC_Mux_SetMhlHotPlugInverseInfo);
EXPORT_SYMBOL(MApi_XC_Mux_GetHDMIPort);
EXPORT_SYMBOL(MApi_XC_Mux_GetPortMappingMatrix);
EXPORT_SYMBOL(MApi_XC_MUX_MapInputSourceToVDYMuxPORT);
EXPORT_SYMBOL(MApi_XC_Set_NR);
EXPORT_SYMBOL(MApi_XC_FilmMode_P);
EXPORT_SYMBOL(MApi_XC_GetUCEnabled);
EXPORT_SYMBOL(MApi_XC_GenSpecificTiming);
EXPORT_SYMBOL(MApi_XC_GetDEBypassMode);
EXPORT_SYMBOL(MApi_XC_GetDEWindow);
EXPORT_SYMBOL(MApi_XC_GetDEWidthHeightInDEByPassMode);
EXPORT_SYMBOL(MApi_XC_GetCaptureWindow);
EXPORT_SYMBOL(MApi_XC_SetCaptureWindowVstart);
EXPORT_SYMBOL(MApi_XC_SetCaptureWindowHstart);
EXPORT_SYMBOL(MApi_XC_SetCaptureWindowVsize);
EXPORT_SYMBOL(MApi_XC_SetCaptureWindowHsize);
EXPORT_SYMBOL(MApi_XC_SoftwareReset);
EXPORT_SYMBOL(MApi_XC_CalculateHFreqx10);
EXPORT_SYMBOL(MApi_XC_CalculateHFreqx1K);
EXPORT_SYMBOL(MApi_XC_CalculateVFreqx10);
EXPORT_SYMBOL(MApi_XC_CalculateVFreqx1K);
EXPORT_SYMBOL(MApi_XC_InterruptAttach);
EXPORT_SYMBOL(MApi_XC_InterruptDeAttach);
EXPORT_SYMBOL(MApi_XC_DisableInputSource);
EXPORT_SYMBOL(MApi_XC_IsInputSourceDisabled);
EXPORT_SYMBOL(MApi_XC_ChangePanelType);
EXPORT_SYMBOL(MApi_XC_GetCurrentReadBank);
EXPORT_SYMBOL(MApi_XC_GetCurrentWriteBank);
EXPORT_SYMBOL(MApi_XC_SetAutoPreScaling);
EXPORT_SYMBOL(MApi_XC_GetVSyncWidth);
EXPORT_SYMBOL(MApi_XC_set_GOP_Enable);
EXPORT_SYMBOL(MApi_XC_ip_sel_for_gop);
EXPORT_SYMBOL(MApi_XC_SetVOPNBL);
EXPORT_SYMBOL(MApi_XC_GetDstInfo);
EXPORT_SYMBOL(MApi_XC_set_FD_Mask_byWin);
EXPORT_SYMBOL(MApi_XC_Get_FD_Mask_byWin);
EXPORT_SYMBOL(MApi_XC_Get_FD_Mask_Status);
#if defined(UFO_PUBLIC_HEADER_500_3)
EXPORT_SYMBOL(MApi_XC_SetIP1TestPattern);
#endif
EXPORT_SYMBOL(MApi_XC_InitIPForInternalTiming);
EXPORT_SYMBOL(MApi_XC_SetIPMux);
EXPORT_SYMBOL(MApi_XC_Is_H_Sync_Active);
EXPORT_SYMBOL(MApi_XC_Is_V_Sync_Active);
EXPORT_SYMBOL(MApi_XC_GetAutoPositionWindow);
EXPORT_SYMBOL(MApi_XC_SetFrameBufferAddress);
EXPORT_SYMBOL(MApi_XC_SetFRCMFrameBufferAddress);
EXPORT_SYMBOL(MApi_XC_IsFrameBufferEnoughForCusScaling);
EXPORT_SYMBOL(MApi_XC_SetScalerMemoryRequest);
EXPORT_SYMBOL(MApi_XC_Get_PixelData);
EXPORT_SYMBOL(MApi_XC_GetAvailableSize);
EXPORT_SYMBOL(MApi_XC_SetFrameColor);
EXPORT_SYMBOL(MApi_XC_SetDispWindowColor);
EXPORT_SYMBOL(MApi_XC_SupportSourceToVE);
EXPORT_SYMBOL(MApi_XC_SetOutputCapture);
EXPORT_SYMBOL(MApi_XC_SetGammaOnOff);
EXPORT_SYMBOL(MApi_XC_SetPreGammaGain);
EXPORT_SYMBOL(MApi_XC_SetPreGammaOffset);
EXPORT_SYMBOL(MApi_XC_SetPanelTiming);
EXPORT_SYMBOL(MApi_XC_SetOutTimingMode);
EXPORT_SYMBOL(MApi_XC_SetFreeRunTiming);
EXPORT_SYMBOL(MApi_XC_Set_CustomerSyncInfo);
EXPORT_SYMBOL(MApi_XC_WaitFPLLDone);
EXPORT_SYMBOL(MApi_XC_GetOutputVFreqX100);
EXPORT_SYMBOL(MApi_XC_GetOP1OutputVFreqX100);
EXPORT_SYMBOL(MApi_XC_FrameLockCheck);
EXPORT_SYMBOL(MApi_XC_CustomizeFRCTable);
EXPORT_SYMBOL(MApi_XC_OutputFrameCtrl);
EXPORT_SYMBOL(MApi_SC_Enable_FPLL_FSM);
EXPORT_SYMBOL(MApi_SC_ForceFreerun);
EXPORT_SYMBOL(MApi_SC_IsForceFreerun);
EXPORT_SYMBOL(MApi_SC_SetFreerunVFreq);
EXPORT_SYMBOL(MApi_XC_SetExPanelInfo);
EXPORT_SYMBOL(MApi_XC_Enable_FPLL_Thresh_Mode);
EXPORT_SYMBOL(MApi_XC_Get_FPLL_Thresh_Mode);
EXPORT_SYMBOL(MApi_XC_EnableIPAutoNoSignal);
EXPORT_SYMBOL(MApi_XC_GetIPAutoNoSignal);
EXPORT_SYMBOL(MApi_XC_EnableIPAutoCoast);
EXPORT_SYMBOL(MApi_XC_EnableIPCoastDebounce);
EXPORT_SYMBOL(MApi_XC_ClearIPCoastStatus);
EXPORT_SYMBOL(MApi_XC_EnableFpllManualSetting);
EXPORT_SYMBOL(MApi_XC_FpllBoundaryTest);
EXPORT_SYMBOL(MApi_XC_SetOffLineDetection);
EXPORT_SYMBOL(MApi_XC_GetOffLineDetection);
EXPORT_SYMBOL(MApi_XC_SetOffLineSogThreshold);
EXPORT_SYMBOL(MApi_XC_SetOffLineSogBW);
EXPORT_SYMBOL(MApi_XC_OffLineInit);
EXPORT_SYMBOL(MApi_XC_Set_Extra_fetch_adv_line);
EXPORT_SYMBOL(MApi_XC_SetVGASogEn);
EXPORT_SYMBOL(MApi_XC_EnableWindow);
EXPORT_SYMBOL(MApi_XC_Is_SubWindowEanble);
EXPORT_SYMBOL(MApi_XC_SetBorderFormat);
EXPORT_SYMBOL(MApi_XC_EnableBorder);
EXPORT_SYMBOL(MApi_XC_ZorderMainWindowFirst);
EXPORT_SYMBOL(MApi_XC_PQ_LoadFunction);
EXPORT_SYMBOL(MApi_XC_Check_HNonLinearScaling);
EXPORT_SYMBOL(MApi_XC_EnableEuroHdtvSupport);
EXPORT_SYMBOL(MApi_XC_EnableEuroHdtvDetection);
EXPORT_SYMBOL(MApi_XC_ReadByte);
EXPORT_SYMBOL(MApi_XC_WriteByte);
EXPORT_SYMBOL(MApi_XC_WriteByteMask);
EXPORT_SYMBOL(MApi_XC_Write2ByteMask);
EXPORT_SYMBOL(MApi_XC_W2BYTE);
EXPORT_SYMBOL(MApi_XC_R2BYTE);
EXPORT_SYMBOL(MApi_XC_W4BYTE);
EXPORT_SYMBOL(MApi_XC_R4BYTE);
EXPORT_SYMBOL(MApi_XC_R2BYTEMSK);
EXPORT_SYMBOL(MApi_XC_W2BYTEMSK);
EXPORT_SYMBOL(MApi_XC_MLoad_Init);
EXPORT_SYMBOL(MApi_XC_MLoad_Enable);
EXPORT_SYMBOL(MApi_XC_MLoad_Cus_Init);
EXPORT_SYMBOL(MApi_XC_MLoad_Cus_Enable);
EXPORT_SYMBOL(MApi_XC_MLoad_GetStatus);
EXPORT_SYMBOL(MApi_XC_MLoad_WriteCmd_And_Fire);
EXPORT_SYMBOL(MApi_XC_MLoad_WriteCmds_And_Fire);
EXPORT_SYMBOL(MApi_XC_MLG_Init);
EXPORT_SYMBOL(MApi_XC_MLG_Enable);
EXPORT_SYMBOL(MApi_XC_MLG_GetCaps);
EXPORT_SYMBOL(MApi_XC_MLG_GetStatus);
EXPORT_SYMBOL(MApi_XC_SetOSD2VEMode);
EXPORT_SYMBOL(MApi_XC_IP2_PreFilter_Enable);
EXPORT_SYMBOL(MApi_XC_Get_Pixel_RGB);
EXPORT_SYMBOL(MApi_XC_KeepPixelPointerAppear);
EXPORT_SYMBOL(MApi_XC_Set_MemFmtEx);
EXPORT_SYMBOL(MApi_XC_IsRequestFrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_SkipSWReset);
EXPORT_SYMBOL(MApi_XC_EnableRepWindowForFrameColor);
EXPORT_SYMBOL(MApi_XC_SetOSDLayer);
EXPORT_SYMBOL(MApi_XC_GetOSDLayer);
EXPORT_SYMBOL(MApi_XC_SetVideoAlpha);
EXPORT_SYMBOL(MApi_XC_GetVideoAlpha);
EXPORT_SYMBOL(MApi_XC_SkipWaitVsync);
EXPORT_SYMBOL(MApi_XC_SetCMAHeapID);
EXPORT_SYMBOL(MApi_XC_OP2VOPDESel);
EXPORT_SYMBOL(MApi_XC_FRC_SetWindow);
#ifdef SUPPORT_BWD
#endif
EXPORT_SYMBOL(MApi_XC_ConfigCMA);
EXPORT_SYMBOL(MApi_XC_Enable_TwoInitFactor);
EXPORT_SYMBOL(MApi_XC_IsFieldPackingModeSupported);
EXPORT_SYMBOL(MApi_XC_PreInit);
EXPORT_SYMBOL(MApi_XC_Get_BufferData);
EXPORT_SYMBOL(MApi_XC_Set_BufferData);
EXPORT_SYMBOL(MApi_XC_EnableMainWindow);
EXPORT_SYMBOL(MApi_XC_EnableSubWindow);
EXPORT_SYMBOL(MApi_XC_DisableSubWindow);
EXPORT_SYMBOL(MApi_XC_SetPixelShift);
EXPORT_SYMBOL(MApi_XC_GetPixelShift);
EXPORT_SYMBOL(MApi_XC_SetPixelShiftFeatures);
EXPORT_SYMBOL(MApi_XC_Combine_MLoadEn);
EXPORT_SYMBOL(MApi_XC_SetVideoOnOSD);
EXPORT_SYMBOL(MApi_XC_SetOSDLayerBlending);
EXPORT_SYMBOL(MApi_XC_SetOSDLayerAlpha);
EXPORT_SYMBOL(MApi_XC_SetOSDBlendingFormula);
EXPORT_SYMBOL(MApi_XC_ReportPixelInfo);
EXPORT_SYMBOL(MApi_XC_SetScaling);
EXPORT_SYMBOL(MApi_XC_SetMCDIBufferAddress);
EXPORT_SYMBOL(MApi_XC_EnableMCDI);
EXPORT_SYMBOL(MApi_XC_SendCmdToFRC);
EXPORT_SYMBOL(MApi_XC_GetMsgFromFRC);
EXPORT_SYMBOL(MApi_XC_EnableRWBankAuto);
EXPORT_SYMBOL(MApi_XC_Dump_SHM);
EXPORT_SYMBOL(MApi_XC_SetWRBankMappingNum);
EXPORT_SYMBOL(MApi_XC_GetWRBankMappingNum);
EXPORT_SYMBOL(MApi_XC_GetWRBankMappingNumForZap);
EXPORT_SYMBOL(MApi_XC_SetBOBMode);
EXPORT_SYMBOL(MApi_XC_SetForceReadBank);
EXPORT_SYMBOL(MApi_XC_SetLockPoint);
EXPORT_SYMBOL(MApi_XC_LD_Init);
EXPORT_SYMBOL(MApi_XC_LD_SetMemoryAddress);
EXPORT_SYMBOL(MApi_XC_LD_Get_Value);
EXPORT_SYMBOL(MApi_XC_LD_SetLevel);
EXPORT_SYMBOL(MApi_Set_TurnoffLDBL);
EXPORT_SYMBOL(MApi_Set_notUpdateSPIDataFlags);
EXPORT_SYMBOL(MApi_Set_UsermodeLDFlags);
EXPORT_SYMBOL(MApi_Set_BLLevel);
EXPORT_SYMBOL(MApi_XC_Set_BWS_Mode);
EXPORT_SYMBOL(MApi_XC_FRC_ColorPathCtrl);
EXPORT_SYMBOL(MApi_XC_FRC_OP2_SetRGBGain);
EXPORT_SYMBOL(MApi_XC_FRC_OP2_SetRGBOffset);
EXPORT_SYMBOL(MApi_XC_FRC_OP2_SetDither);
EXPORT_SYMBOL(MApi_XC_FRC_BypassMFC);
EXPORT_SYMBOL(MApi_XC_FRC_Mute);
EXPORT_SYMBOL(MApi_XC_ForceReadFrame);
EXPORT_SYMBOL(MApi_XC_SetCsc);
EXPORT_SYMBOL(MApi_XC_RegisterPQSetFPLLThreshMode);
EXPORT_SYMBOL(MApi_XC_GetFreeRunStatus);
EXPORT_SYMBOL(MApi_XC_Get_DSForceIndexSupported);
EXPORT_SYMBOL(MApi_XC_Set_DSForceIndex);
EXPORT_SYMBOL(MApi_XC_Set_DSIndexSourceSelect);
EXPORT_SYMBOL(MApi_XC_OSDC_InitSetting);
EXPORT_SYMBOL(MApi_XC_OSDC_Control);
EXPORT_SYMBOL(MApi_XC_OSDC_GetDstInfo);
EXPORT_SYMBOL(MApi_XC_OSDC_SetOutVfreqx10);
EXPORT_SYMBOL(MApi_XC_BYPASS_SetOSDVsyncPos);
EXPORT_SYMBOL(MApi_XC_BYPASS_SetInputSrc);
EXPORT_SYMBOL(MApi_XC_BYPASS_SetCSC);
EXPORT_SYMBOL(MApi_XC_SetSeamlessZapping);
EXPORT_SYMBOL(MApi_XC_GetSeamlessZappingStatus);
EXPORT_SYMBOL(MApi_XC_Vtrack_SetPayloadData);
EXPORT_SYMBOL(MApi_XC_Vtrack_SetAllPayloadData);
EXPORT_SYMBOL(MApi_XC_Vtrack_SetVtrackKey);
EXPORT_SYMBOL(MApi_XC_Vtrack_SetUserDefindedSetting);
EXPORT_SYMBOL(MApi_XC_Vtrack_Enable);
EXPORT_SYMBOL(MApi_XC_PreSetPQInfo);
EXPORT_SYMBOL(MApi_XC_Is_OP1_TestPattern_Enabled);
EXPORT_SYMBOL(MApi_XC_Set_OP1_TestPattern);
EXPORT_SYMBOL(MApi_XC_CheckWhiteBalancePatternModeSupport);
EXPORT_SYMBOL(MApi_XC_SetHLinearScaling);
EXPORT_SYMBOL(MApi_XC_SetPowerState);
EXPORT_SYMBOL(MApi_XC_EnableT3D);
EXPORT_SYMBOL(MApi_XC_Set_FRC_InputTiming);
EXPORT_SYMBOL(MApi_XC_Get_FRC_InputTiming);
EXPORT_SYMBOL(MApi_XC_Get_VirtualBox_Info);
EXPORT_SYMBOL(MApi_XC_Set_OSD_Detect);
EXPORT_SYMBOL(MApi_XC_Get_OSD_Detect);
EXPORT_SYMBOL(MApi_XC_Is2K2KToFrcMode);
EXPORT_SYMBOL(MApi_XC_Enable_LockFreqOnly);
#ifdef UFO_XC_TEST_PATTERN
#endif
EXPORT_SYMBOL(MApi_XC_Get_Current_OutputVFreqX100);
EXPORT_SYMBOL(MApi_XC_SetMemoryWriteRequest);
EXPORT_SYMBOL(MApi_XC_Set_MemFmt);
EXPORT_SYMBOL(MApi_XC_SetOutputAdjustSetting);
EXPORT_SYMBOL(MApi_XC_EnableMirrorMode);
EXPORT_SYMBOL(MApi_XC_EnableMirrorMode2);
EXPORT_SYMBOL(MApi_XC_GetMirrorModeType);
EXPORT_SYMBOL(MApi_XC_set_FD_Mask);
EXPORT_SYMBOL(MApi_XC_Get_FD_Mask);
EXPORT_SYMBOL(MApi_XC_GetAccurateVFreqx1K);
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
EXPORT_SYMBOL(MApi_XC_GenerateBlackVideoByMode);
#endif
EXPORT_SYMBOL(MApi_XC_IsSupport2StepScaling);
#ifdef UFO_XC_PQ_PATH
#endif
EXPORT_SYMBOL(MApi_XC_SetIP1TestPattern);
EXPORT_SYMBOL(MApi_XC_Set_OP2_Pattern);
#ifdef UFO_XC_FORCEWRITE_V2
EXPORT_SYMBOL(MApi_XC_SetForceWrite);
EXPORT_SYMBOL(MApi_XC_GetForceWrite);
#endif
EXPORT_SYMBOL(MApi_XC_VideoPlaybackCtrl);
#ifdef UFO_XC_AUTO_DOWNLOAD
EXPORT_SYMBOL(MApi_XC_AutoDownload_Config);
EXPORT_SYMBOL(MApi_XC_AutoDownload_Write);
EXPORT_SYMBOL(MApi_XC_AutoDownload_Fire);
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
EXPORT_SYMBOL(MApi_XC_HDR_Control);
#endif
#endif
#ifdef UFO_XC_SCAN_TYPE
#endif
EXPORT_SYMBOL(MApi_XC_Set_FB_Level);
EXPORT_SYMBOL(MApi_XC_Get_FB_Level);
EXPORT_SYMBOL(MApi_SWDS_Fire);
EXPORT_SYMBOL(MApi_XC_GetResourceByPipeID);
EXPORT_SYMBOL(MApi_XC_OPTEE_CheckRBase);
EXPORT_SYMBOL(MApi_XC_ConfigPipe);
EXPORT_SYMBOL(MApi_XC_CheckPipe);
EXPORT_SYMBOL(MApi_XC_OPTEE_Control);
EXPORT_SYMBOL(MApi_XC_OPTEE_Mux);
EXPORT_SYMBOL(MApi_XC_PollEvent);
EXPORT_SYMBOL(MApi_XC_Cancel_PollEvent);
#ifdef UFO_XC_DE_MODE
#endif
EXPORT_SYMBOL(MApi_XC_SetSWDRInfo);
EXPORT_SYMBOL(MApi_XC_GetSWDRInfo);
#ifdef UFO_XC_SUPPORT_HDMI_DOWNSCALE_OUTPUT_POLICY
EXPORT_SYMBOL(MApi_XC_GetHDMIPolicy);
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_ACE_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#if !defined (__aarch64__)
#endif
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#endif
EXPORT_SYMBOL(MApi_XC_ACE_Init);
EXPORT_SYMBOL(MApi_XC_ACE_Exit);
EXPORT_SYMBOL(MApi_XC_ACE_DMS);
EXPORT_SYMBOL(MApi_XC_ACE_SetPCYUV2RGB);
EXPORT_SYMBOL(MApi_XC_ACE_SelectYUVtoRGBMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_SetColorCorrectionTable);
EXPORT_SYMBOL(MApi_XC_ACE_SetPCsRGBTable);
EXPORT_SYMBOL(MApi_XC_ACE_GetColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_SetColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_SetBypassColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_SetIHC);
EXPORT_SYMBOL(MApi_XC_ACE_SetICC);
EXPORT_SYMBOL(MApi_XC_ACE_SetIBC);
EXPORT_SYMBOL(MApi_XC_ACE_PatchDTGColorChecker);
EXPORT_SYMBOL(MApi_XC_ACE_GetACEInfo);
EXPORT_SYMBOL(MApi_XC_ACE_GetACEInfoEx);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetContrast);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetBrightness);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetBrightnessPrecise);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPreYOffset);
EXPORT_SYMBOL(MApi_XC_ACE_PicGetPreYOffset);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetHue);
EXPORT_SYMBOL(MApi_XC_ACE_SkipWaitVsync);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetSaturation);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetSharpness);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetColorTemp);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetColorTempEx);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPostColorTemp_V02);
#ifndef _API_XC_ACE_C_
#ifndef ENABLE_NEW_COLORTEMP_METHOD
#endif
#ifndef ENABLE_PRECISE_RGBBRIGHTNESS
#endif
#if ENABLE_NEW_COLORTEMP_METHOD
#if ENABLE_PRECISE_RGBBRIGHTNESS
#else
#endif
#else
#if ENABLE_PRECISE_RGBBRIGHTNESS
#endif
#endif
#endif
EXPORT_SYMBOL(MApi_XC_ACE_SetFleshTone);
EXPORT_SYMBOL(MApi_XC_ACE_SetBlackAdjust);
EXPORT_SYMBOL(MApi_XC_ACE_Set_IHC_SRAM);
EXPORT_SYMBOL(MApi_XC_ACE_Set_ICC_SRAM);
EXPORT_SYMBOL(MApi_XC_ACE_EnableMWE);
EXPORT_SYMBOL(MApi_XC_ACE_MWECloneVisualEffect);
EXPORT_SYMBOL(MApi_XC_ACE_MWESetRegTable);
EXPORT_SYMBOL(MApi_XC_ACE_SetMWELoadFromPQ);
EXPORT_SYMBOL(MApi_XC_ACE_MWEStatus);
EXPORT_SYMBOL(MApi_XC_ACE_MWEHandle);
EXPORT_SYMBOL(MApi_XC_ACE_MWEFuncSel);
EXPORT_SYMBOL(MApi_XC_ACE_MWESetDispWin);
EXPORT_SYMBOL(MApi_XC_ACE_3DClonePQMap);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Blending_NRTbl);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Blending_MISC);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Blending);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Get_PrecisionShift);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_GetMotion_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_GetLuma_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_GetNoise_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_GetMotion);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Init_Motion);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Init_Luma);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_Init_Noise);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_SetParam);
EXPORT_SYMBOL(MApi_XC_ACE_SetHDRInit);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_GetParam);
EXPORT_SYMBOL(MApi_XC_ACE_DNR_SetNRTbl);
EXPORT_SYMBOL(MApi_XC_ACE_GetLibVer);
EXPORT_SYMBOL(MApi_XC_ACE_GetInfo);
EXPORT_SYMBOL(MApi_XC_ACE_GetStatus);
EXPORT_SYMBOL(MApi_XC_ACE_SetDbgLevel);
EXPORT_SYMBOL(MApi_XC_ACE_ColorCorrectionTable);
EXPORT_SYMBOL(MApi_XC_ACE_SetColorMatrixControl);
EXPORT_SYMBOL(MApi_XC_ACE_SetRBChannelRange);
EXPORT_SYMBOL(MApi_XC_ACE_SetPowerState);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPostColorTemp);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPostColorTempEx);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPostColorTemp2);
EXPORT_SYMBOL(MApi_XC_ACE_PicSetPostColorTemp2Ex);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_ACE_EX_H_
#ifdef __cplusplus
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !defined (__aarch64__)
#endif
#if !defined (__aarch64__)
#endif
EXPORT_SYMBOL(MApi_XC_ACE_EX_Init);
EXPORT_SYMBOL(MApi_XC_ACE_EX_Exit);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DMS);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetPCYUV2RGB);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SelectYUVtoRGBMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetColorCorrectionTable);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetPCsRGBTable);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetBypassColorMatrix);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetIHC);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PatchDTGColorChecker);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetACEInfo);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetACEInfoEx);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetContrast);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetBrightness);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetBrightnessPrecise);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPreYOffset);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicGetPreYOffset);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetHue);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SkipWaitVsync);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetSaturation);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetSharpness);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetColorTemp);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetColorTempEx);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPostColorTemp_V02);
#ifndef _API_XC_ACE_EX_C_
#ifndef EX_ENABLE_NEW_COLORTEMP_METHOD
#endif
#ifndef EX_ENABLE_PRECISE_RGBBRIGHTNESS
#endif
#if EX_ENABLE_NEW_COLORTEMP_METHOD
#if EX_ENABLE_PRECISE_RGBBRIGHTNESS
#else
#endif
#else
#if EX_ENABLE_PRECISE_RGBBRIGHTNESS
#endif
#endif
#endif
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetFleshTone);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetBlackAdjust);
EXPORT_SYMBOL(MApi_XC_ACE_EX_Set_IHC_SRAM);
EXPORT_SYMBOL(MApi_XC_ACE_EX_Set_ICC_SRAM);
EXPORT_SYMBOL(MApi_XC_ACE_EX_EnableMWE);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWECloneVisualEffect);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWESetRegTable);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetMWELoadFromPQ);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWEStatus);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWEHandle);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWEFuncSel);
EXPORT_SYMBOL(MApi_XC_ACE_EX_MWESetDispWin);
EXPORT_SYMBOL(MApi_XC_ACE_EX_3DClonePQMap);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Blending_NRTbl);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Blending_MISC);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Blending);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Get_PrecisionShift);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_GetMotion_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_GetLuma_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_GetNoise_Weight);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_GetMotion);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Init_Motion);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Init_Luma);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_Init_Noise);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_SetParam);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_GetParam);
EXPORT_SYMBOL(MApi_XC_ACE_EX_DNR_SetNRTbl);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetLibVer);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetInfo);
EXPORT_SYMBOL(MApi_XC_ACE_EX_GetStatus);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetDbgLevel);
EXPORT_SYMBOL(MApi_XC_ACE_EX_ColorCorrectionTable);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetColorMatrixControl);
EXPORT_SYMBOL(MApi_XC_ACE_EX_SetRBChannelRange);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPostColorTemp);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPostColorTempEx);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPostColorTemp2);
EXPORT_SYMBOL(MApi_XC_ACE_EX_PicSetPostColorTemp2Ex);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_ACE_PRIV_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_ADC_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_ADC_SetCVBSOut);
EXPORT_SYMBOL(MApi_XC_ADC_IsCVBSOutEnabled);
EXPORT_SYMBOL(MApi_XC_ADC_SetPcClock);
EXPORT_SYMBOL(MApi_XC_ADC_SetPhase);
EXPORT_SYMBOL(MApi_XC_ADC_SetPhaseEx);
EXPORT_SYMBOL(MApi_XC_ADC_GetPhaseRange);
EXPORT_SYMBOL(MApi_XC_ADC_GetPhase);
EXPORT_SYMBOL(MApi_XC_ADC_GetPhaseEx);
EXPORT_SYMBOL(MApi_XC_ADC_IsScartRGB);
EXPORT_SYMBOL(MApi_XC_ADC_GetPcClock);
EXPORT_SYMBOL(MApi_XC_ADC_GetSoGLevelRange);
EXPORT_SYMBOL(MApi_XC_ADC_SetSoGLevel);
EXPORT_SYMBOL(MApi_XC_ADC_PowerOff);
EXPORT_SYMBOL(MApi_XC_ADC_GetDefaultGainOffset);
EXPORT_SYMBOL(MApi_XC_ADC_GetMaximalOffsetValue);
EXPORT_SYMBOL(MApi_XC_ADC_GetMaximalGainValue);
EXPORT_SYMBOL(MApi_XC_ADC_GetCenterGain);
EXPORT_SYMBOL(MApi_XC_ADC_GetCenterOffset);
EXPORT_SYMBOL(MApi_XC_ADC_SetGain);
EXPORT_SYMBOL(MApi_XC_ADC_SetOffset);
EXPORT_SYMBOL(MApi_XC_ADC_AdjustGainOffset);
EXPORT_SYMBOL(MApi_XC_ADC_Source_Calibrate);
EXPORT_SYMBOL(MApi_XC_ADC_SetSoGCal);
EXPORT_SYMBOL(MApi_XC_ADC_SetRGB_PIPE_Delay);
EXPORT_SYMBOL(MApi_XC_ADC_ScartRGB_SOG_ClampDelay);
EXPORT_SYMBOL(MApi_XC_ADC_Set_YPbPrLooseLPF);
EXPORT_SYMBOL(MApi_XC_ADC_Set_SOGBW);
EXPORT_SYMBOL(MApi_XC_ADC_SetClampDuration);
EXPORT_SYMBOL(MApi_XC_ADC_EnableHWCalibration);
EXPORT_SYMBOL(MApi_XC_ADC_SetIdacCurrentMode);
EXPORT_SYMBOL(MApi_XC_ADC_GetIdacCurrentMode);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_ADC_EX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetCVBSOut);
EXPORT_SYMBOL(MApi_XC_ADC_EX_IsCVBSOutEnabled);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetPcClock);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetPhase);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetPhaseEx);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetPhaseRange);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetPhase);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetPhaseEx);
EXPORT_SYMBOL(MApi_XC_ADC_EX_IsScartRGB);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetPcClock);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetSoGLevelRange);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetSoGLevel);
EXPORT_SYMBOL(MApi_XC_ADC_EX_PowerOff);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetDefaultGainOffset);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetMaximalOffsetValue);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetMaximalGainValue);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetCenterGain);
EXPORT_SYMBOL(MApi_XC_ADC_EX_GetCenterOffset);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetGain);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetOffset);
EXPORT_SYMBOL(MApi_XC_ADC_EX_AdjustGainOffset);
EXPORT_SYMBOL(MApi_XC_ADC_EX_Source_Calibrate);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetSoGCal);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetRGB_PIPE_Delay);
EXPORT_SYMBOL(MApi_XC_ADC_EX_ScartRGB_SOG_ClampDelay);
EXPORT_SYMBOL(MApi_XC_ADC_EX_Set_YPbPrLooseLPF);
EXPORT_SYMBOL(MApi_XC_ADC_EX_Set_SOGBW);
EXPORT_SYMBOL(MApi_XC_ADC_EX_SetClampDuration);
EXPORT_SYMBOL(MApi_XC_ADC_EX_EnableHWCalibration);
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_ADC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _APIXC_AUTO_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_Auto_Geometry);
EXPORT_SYMBOL(MApi_XC_Auto_Geometry_Ex);
EXPORT_SYMBOL(MApi_XC_Auto_StopAutoGeometry);
EXPORT_SYMBOL(MApi_XC_Auto_GainOffset);
EXPORT_SYMBOL(MApi_XC_Auto_GetHWFixedGainOffset);
EXPORT_SYMBOL(MApi_XC_Auto_SetValidData);
EXPORT_SYMBOL(MApi_XC_Auto_AutoOffset);
EXPORT_SYMBOL(MApi_XC_Auto_DetectWidth);
EXPORT_SYMBOL(MApi_XC_Auto_SetCalibrationMode);
EXPORT_SYMBOL(MApi_XC_Auto_GetCalibrationMode);
EXPORT_SYMBOL(MApi_XC_AUTO_GetSyncInfo);
#ifdef __cplusplus
#endif
#endif
#ifdef _APIXC_AUTO_EX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_Auto_EX_Geometry);
EXPORT_SYMBOL(MApi_XC_Auto_EX_Geometry_Ex);
EXPORT_SYMBOL(MApi_XC_Auto_EX_StopAutoGeometry);
EXPORT_SYMBOL(MApi_XC_Auto_EX_GainOffset);
EXPORT_SYMBOL(MApi_XC_Auto_EX_GetHWFixedGainOffset);
EXPORT_SYMBOL(MApi_XC_Auto_EX_SetValidData);
EXPORT_SYMBOL(MApi_XC_Auto_EX_AutoOffset);
EXPORT_SYMBOL(MApi_XC_Auto_EX_DetectWidth);
EXPORT_SYMBOL(MApi_XC_Auto_EX_SetCalibrationMode);
EXPORT_SYMBOL(MApi_XC_Auto_EX_GetCalibrationMode);
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_AUTO_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _APIXC_CUS_H_
#ifdef __cplusplus
#endif
#ifdef KR3DINTERLACE
#endif
#if 1
#endif
#if 1
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_DBC_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_DLC_DBC_Init);
EXPORT_SYMBOL(MApi_XC_DBC_Init);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_Setstatus);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_Getstatus);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_SetReady);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_SetDebugMode);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_GetDebugMode);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_UpdatePWM);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_Handler);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_AdjustYCGain);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_YCGainInit);
EXPORT_SYMBOL(MApi_XC_DLC_DBC_Reset);
EXPORT_SYMBOL(MApi_XC_DBC_DecodeExtCmd);
#ifdef __cplusplus
#endif
#endif
#ifdef APIXC_DWIN_H
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef APIXC_DWIN_EX_H
#ifdef __cplusplus
#endif
#ifdef APIXC_DWIN_EX_C
#else
#endif
#ifdef __cplusplus
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_300)|| defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500)
#endif
#ifdef _API_XC_DLC_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifndef UNUSED
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if !defined (__aarch64__)
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if (ENABLE_10_BIT_DLC)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#if defined(UFO_XC_HDR_VERSION) && (UFO_XC_HDR_VERSION == 2)
#endif
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#endif
#endif
EXPORT_SYMBOL(MApi_XC_DLC_Init_Ex);
EXPORT_SYMBOL(MApi_XC_DLC_SetSetting_Ex);
EXPORT_SYMBOL(MApi_XC_DLC_SetCurve);
EXPORT_SYMBOL(MApi_XC_DLC_SetBleSlopPoint);
EXPORT_SYMBOL(MApi_XC_DLC_SetHDRInit);
EXPORT_SYMBOL(MApi_XC_DLC_Exit);
EXPORT_SYMBOL(MApi_XC_DLC_SetOnOff);
EXPORT_SYMBOL(MApi_XC_DLC_SetDlcHandlerOnOff);
EXPORT_SYMBOL(MApi_XC_DLC_SetBleOnOff);
EXPORT_SYMBOL(MApi_XC_DLC_EnableMainSubCurveSynchronization);
EXPORT_SYMBOL(MApi_XC_DLC_Handler);
EXPORT_SYMBOL(MApi_XC_DLC_GetHistogramHandler);
EXPORT_SYMBOL(MApi_XC_DLC_GetAverageValue);
EXPORT_SYMBOL(MApi_XC_DLC_GetAverageValue_Ex);
EXPORT_SYMBOL(MApi_XC_DLC_InitCurve);
EXPORT_SYMBOL(MApi_XC_DLC_SpeedupTrigger);
EXPORT_SYMBOL(MApi_XC_DLC_GetLumaCurveStatus);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_ResetCGain);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_CheckCGainInPQCom);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_ResetYGain);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_CheckYGainInPQCom);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_Reset);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_Init);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_ReInit);
EXPORT_SYMBOL(MApi_XC_DLC_CGC_Handler);
EXPORT_SYMBOL(MApi_XC_DLC_GetLibVer);
EXPORT_SYMBOL(MApi_XC_DLC_GetInfo);
EXPORT_SYMBOL(MApi_XC_DLC_GetStatus_Ex);
EXPORT_SYMBOL(MApi_XC_DLC_SetDbgLevel);
EXPORT_SYMBOL(MApi_XC_DLC_WriteCurve);
EXPORT_SYMBOL(MApi_XC_DLC_WriteCurve_Sub);
EXPORT_SYMBOL(MApi_XC_DLC_GetHistogram);
EXPORT_SYMBOL(MApi_XC_DLC_DecodeExtCmd);
EXPORT_SYMBOL(MApi_XC_DLC_SetCaptureRange);
EXPORT_SYMBOL(MApi_XC_DLC_SetPowerState);
EXPORT_SYMBOL(MApi_XC_DLC_SetMemoryAddress);
EXPORT_SYMBOL(MApi_XC_DLC_SetSetting);
EXPORT_SYMBOL(MApi_XC_DLC_Init);
EXPORT_SYMBOL(MApi_XC_DLC_GetStatus);
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
EXPORT_SYMBOL(MApi_XC_DLC_SetTMOInfo);
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DLC_V2_H_
#ifdef _DLC_C_
#else
#endif
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_XC_HDR_VERSION) && (UFO_XC_HDR_VERSION == 2)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_EX_H_
#ifdef __cplusplus
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500) ||defined(UFO_PUBLIC_HEADER_300)||defined(UFO_PUBLIC_HEADER_700)||defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700) ||defined(UFO_PUBLIC_HEADER_300) ||defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifndef CONFIG_MBOOT
#else
#endif
#ifdef UFO_XC_SCAN_TYPE
#endif
EXPORT_SYMBOL(MApi_XC_EX_GetLibVer);
EXPORT_SYMBOL(MApi_XC_EX_GetInfo);
EXPORT_SYMBOL(MApi_XC_EX_GetStatus);
EXPORT_SYMBOL(MApi_XC_EX_GetStatusEx);
EXPORT_SYMBOL(MApi_XC_EX_SetDbgLevel);
EXPORT_SYMBOL(MApi_XC_EX_FPLLDbgMode);
EXPORT_SYMBOL(MApi_XC_EX_FPLLCustomerMode);
EXPORT_SYMBOL(MDrv_XC_EX_SetIOMapBase);
EXPORT_SYMBOL(MApi_XC_EX_Init);
EXPORT_SYMBOL(MApi_XC_EX_Init_MISC);
EXPORT_SYMBOL(MApi_XC_EX_GetMISCStatus);
EXPORT_SYMBOL(MApi_XC_EX_GetCapability);
EXPORT_SYMBOL(MApi_XC_EX_GetChipCaps);
EXPORT_SYMBOL(MApi_XC_EX_Exit);
EXPORT_SYMBOL(MApi_XC_EX_SetDynamicScaling);
EXPORT_SYMBOL(MApi_XC_EX_GetDynamicScalingStatus);
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MApi_XC_EX_Get_DNRBaseOffset);
#else
EXPORT_SYMBOL(MApi_XC_EX_Get_DNRBaseOffset);
#endif
EXPORT_SYMBOL(MApi_XC_EX_Get_FrameNumFactor);
EXPORT_SYMBOL(MApi_XC_EX_SetWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetDualWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetTravelingWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetInputSource);
EXPORT_SYMBOL(MApi_XC_EX_IsYUVSpace);
EXPORT_SYMBOL(MApi_XC_EX_IsMemoryFormat422);
EXPORT_SYMBOL(MApi_XC_EX_EnableForceRGBin);
EXPORT_SYMBOL(MApi_XC_EX_EnableMirrorModeEx);
EXPORT_SYMBOL(MApi_XC_EX_GetMirrorModeTypeEx);
EXPORT_SYMBOL(MApi_XC_EX_GetSyncStatus);
EXPORT_SYMBOL(MApi_XC_EX_WaitOutputVSync);
EXPORT_SYMBOL(MApi_XC_EX_WaitInputVSync);
EXPORT_SYMBOL(MApi_XC_EX_SetHdmiSyncMode);
EXPORT_SYMBOL(MApi_XC_EX_GetHdmiSyncMode);
EXPORT_SYMBOL(MApi_XC_EX_SetRepWindow);
EXPORT_SYMBOL(MApi_XC_EX_Set_OPWriteOffEnable);
EXPORT_SYMBOL(MApi_XC_EX_ForceSet_OPWriteOffEnable);
EXPORT_SYMBOL(MApi_XC_EX_SetDispWinToReg);
EXPORT_SYMBOL(MApi_XC_EX_GetDispWinFromReg);
EXPORT_SYMBOL(MApi_XC_EX_FreezeImg);
EXPORT_SYMBOL(MApi_XC_EX_IsFreezeImg);
EXPORT_SYMBOL(MApi_XC_EX_GenerateBlackVideoForBothWin);
EXPORT_SYMBOL(MApi_XC_EX_Set_BLSK);
EXPORT_SYMBOL(MApi_XC_EX_GenerateBlackVideo);
EXPORT_SYMBOL(MApi_XC_EX_IsBlackVideoEnable);
EXPORT_SYMBOL(MApi_XC_EX_EnableFrameBufferLess);
EXPORT_SYMBOL(MApi_XC_EX_IsCurrentFrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_EX_EnableRequest_FrameBufferLess);
EXPORT_SYMBOL(MApi_XC_EX_IsCurrentRequest_FrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_HW_Version);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_IsSupportedHW2DTo3D);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_Mode);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_MainWin_FirstMode);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_LR_Frame_Exchg);
EXPORT_SYMBOL(MApi_XC_EX_3D_Is_LR_Frame_Exchged);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_Input_Mode);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_Output_Mode);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_Panel_Type);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_MainWin_First);
EXPORT_SYMBOL(MApi_XC_EX_3DMainSub_IPSync);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_VerVideoOffset);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_VerVideoOffset);
EXPORT_SYMBOL(MApi_XC_EX_Is3DFormatSupported);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_HShift);
EXPORT_SYMBOL(MApi_XC_EX_Enable_3D_LR_Sbs2Line);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_HShift);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_HW2DTo3D_Buffer);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_HW2DTo3D_Parameters);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_HW2DTo3D_Parameters);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_Detect3DFormat_Parameters);
EXPORT_SYMBOL(MApi_XC_EX_Get_3D_Detect3DFormat_Parameters);
EXPORT_SYMBOL(MApi_XC_EX_Detect3DFormatByContent);
EXPORT_SYMBOL(MApi_XC_EX_DetectNL);
EXPORT_SYMBOL(MApi_XC_EX_3D_PostPQSetting);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_FPInfo);
EXPORT_SYMBOL(MApi_XC_EX_EnableAutoDetect3D);
EXPORT_SYMBOL(MApi_XC_EX_GetAutoDetect3DFlag);
EXPORT_SYMBOL(MApi_XC_EX_Set_3D_SubWinClk);
EXPORT_SYMBOL(MApi_XC_EX_3D_Is_LR_Sbs2Line);
EXPORT_SYMBOL(MApi_SC_EX_3D_Is_Skip_Default_LR_Flag);
EXPORT_SYMBOL(MApi_XC_EX_3D_Enable_Skip_Default_LR_Flag);
#ifdef UFO_XC_SCAN_TYPE
#endif
EXPORT_SYMBOL(MApi_XC_EX_Mux_Init);
EXPORT_SYMBOL(MApi_XC_EX_Mux_SourceMonitor);
EXPORT_SYMBOL(MApi_XC_EX_Mux_CreatePath);
EXPORT_SYMBOL(MApi_XC_EX_Mux_DeletePath);
EXPORT_SYMBOL(MApi_XC_EX_Mux_EnablePath);
EXPORT_SYMBOL(MApi_XC_EX_Mux_TriggerPathSyncEvent);
EXPORT_SYMBOL(MApi_XC_EX_Mux_TriggerDestOnOffEvent);
EXPORT_SYMBOL(MApi_XC_EX_Mux_OnOffPeriodicHandler);
EXPORT_SYMBOL(MApi_XC_EX_Mux_GetPathInfo);
EXPORT_SYMBOL(MApi_XC_EX_Mux_SetSupportMhlPathInfo);
EXPORT_SYMBOL(MApi_XC_EX_Mux_SetMhlHotPlugInverseInfo);
EXPORT_SYMBOL(MApi_XC_EX_Mux_GetHDMIPort);
EXPORT_SYMBOL(MApi_XC_EX_MUX_MapInputSourceToVDYMuxPORT);
EXPORT_SYMBOL(MApi_XC_EX_Set_NR);
EXPORT_SYMBOL(MApi_XC_EX_FilmMode_P);
EXPORT_SYMBOL(MApi_XC_EX_GetUCEnabled);
EXPORT_SYMBOL(MApi_XC_EX_GenSpecificTiming);
EXPORT_SYMBOL(MApi_XC_EX_GetDEBypassMode);
EXPORT_SYMBOL(MApi_XC_EX_GetDEWindow);
EXPORT_SYMBOL(MApi_XC_EX_GetDEWidthHeightInDEByPassMode);
EXPORT_SYMBOL(MApi_XC_EX_GetCaptureWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetCaptureWindowVstart);
EXPORT_SYMBOL(MApi_XC_EX_SetCaptureWindowHstart);
EXPORT_SYMBOL(MApi_XC_EX_SetCaptureWindowVsize);
EXPORT_SYMBOL(MApi_XC_EX_SetCaptureWindowHsize);
EXPORT_SYMBOL(MApi_XC_EX_SoftwareReset);
EXPORT_SYMBOL(MApi_XC_EX_CalculateHFreqx10);
EXPORT_SYMBOL(MApi_XC_EX_CalculateHFreqx1K);
EXPORT_SYMBOL(MApi_XC_EX_CalculateVFreqx10);
EXPORT_SYMBOL(MApi_XC_EX_CalculateVFreqx1K);
EXPORT_SYMBOL(MApi_XC_EX_GetAccurateVFreqx1K);
EXPORT_SYMBOL(MApi_XC_EX_InterruptAttach);
EXPORT_SYMBOL(MApi_XC_EX_InterruptDeAttach);
EXPORT_SYMBOL(MApi_XC_EX_DisableInputSource);
EXPORT_SYMBOL(MApi_XC_EX_IsInputSourceDisabled);
EXPORT_SYMBOL(MApi_XC_EX_ChangePanelType);
EXPORT_SYMBOL(MApi_XC_EX_GetCurrentReadBank);
EXPORT_SYMBOL(MApi_XC_EX_GetCurrentWriteBank);
EXPORT_SYMBOL(MApi_XC_EX_SetAutoPreScaling);
EXPORT_SYMBOL(MApi_XC_EX_GetVSyncWidth);
EXPORT_SYMBOL(MApi_XC_EX_set_GOP_Enable);
EXPORT_SYMBOL(MApi_XC_EX_ip_sel_for_gop);
EXPORT_SYMBOL(MApi_XC_EX_SetVOPNBL);
EXPORT_SYMBOL(MApi_XC_EX_GetDstInfo);
EXPORT_SYMBOL(MApi_XC_EX_set_FD_Mask);
EXPORT_SYMBOL(MApi_XC_EX_Get_FD_Mask);
EXPORT_SYMBOL(MApi_XC_EX_set_FD_Mask_byWin);
EXPORT_SYMBOL(MApi_XC_EX_Get_FD_Mask_byWin);
EXPORT_SYMBOL(MApi_XC_EX_Get_FD_Mask_Status);
EXPORT_SYMBOL(MApi_XC_EX_SetIP1TestPattern);
EXPORT_SYMBOL(MApi_XC_EX_InitIPForInternalTiming);
EXPORT_SYMBOL(MApi_XC_EX_SetIPMux);
EXPORT_SYMBOL(MApi_XC_EX_Is_H_Sync_Active);
EXPORT_SYMBOL(MApi_XC_EX_Is_V_Sync_Active);
EXPORT_SYMBOL(MApi_XC_EX_GetAutoPositionWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetFrameBufferAddress);
EXPORT_SYMBOL(MApi_XC_EX_IsFrameBufferEnoughForCusScaling);
EXPORT_SYMBOL(MApi_XC_EX_SetScalerMemoryRequest);
EXPORT_SYMBOL(MApi_XC_EX_Get_PixelData);
EXPORT_SYMBOL(MApi_XC_EX_GetAvailableSize);
EXPORT_SYMBOL(MApi_XC_EX_SetFrameColor);
EXPORT_SYMBOL(MApi_XC_EX_SetFrameColorYUV);
EXPORT_SYMBOL(MApi_XC_EX_SetDispWindowColor);
EXPORT_SYMBOL(MApi_XC_EX_SupportSourceToVE);
EXPORT_SYMBOL(MApi_XC_EX_SetOutputCapture);
EXPORT_SYMBOL(MApi_XC_EX_SetGammaOnOff);
EXPORT_SYMBOL(MApi_XC_EX_SetPreGammaGain);
EXPORT_SYMBOL(MApi_XC_EX_SetPreGammaOffset);
EXPORT_SYMBOL(MApi_XC_EX_SetPanelTiming);
EXPORT_SYMBOL(MApi_XC_EX_SetOutTimingMode);
EXPORT_SYMBOL(MApi_XC_EX_SetFreeRunTiming);
EXPORT_SYMBOL(MApi_XC_EX_Set_CustomerSyncInfo);
EXPORT_SYMBOL(MApi_XC_EX_WaitFPLLDone);
EXPORT_SYMBOL(MApi_XC_EX_GetOutputVFreqX100);
EXPORT_SYMBOL(MApi_XC_EX_FrameLockCheck);
EXPORT_SYMBOL(MApi_XC_EX_CustomizeFRCTable);
EXPORT_SYMBOL(MApi_XC_EX_OutputFrameCtrl);
EXPORT_SYMBOL(MApi_SC_EX_Enable_FPLL_FSM);
EXPORT_SYMBOL(MApi_SC_EX_ForceFreerun);
EXPORT_SYMBOL(MApi_SC_EX_IsForceFreerun);
EXPORT_SYMBOL(MApi_SC_EX_SetFreerunVFreq);
EXPORT_SYMBOL(MApi_XC_EX_SetExPanelInfo);
EXPORT_SYMBOL(MApi_XC_EX_Enable_FPLL_Thresh_Mode);
EXPORT_SYMBOL(MApi_XC_EX_EnableIPAutoNoSignal);
EXPORT_SYMBOL(MApi_XC_EX_GetIPAutoNoSignal);
EXPORT_SYMBOL(MApi_XC_EX_EnableIPAutoCoast);
EXPORT_SYMBOL(MApi_XC_EX_EnableIPCoastDebounce);
EXPORT_SYMBOL(MApi_XC_EX_ClearIPCoastStatus);
EXPORT_SYMBOL(MApi_XC_EX_EnableFpllManualSetting);
EXPORT_SYMBOL(MApi_XC_EX_FpllBoundaryTest);
EXPORT_SYMBOL(MApi_XC_EX_SetOffLineDetection);
EXPORT_SYMBOL(MApi_XC_EX_GetOffLineDetection);
EXPORT_SYMBOL(MApi_XC_EX_SetOffLineSogThreshold);
EXPORT_SYMBOL(MApi_XC_EX_SetOffLineSogBW);
EXPORT_SYMBOL(MApi_XC_EX_OffLineInit);
EXPORT_SYMBOL(MApi_XC_EX_Set_Extra_fetch_adv_line);
EXPORT_SYMBOL(MApi_XC_EX_SetVGASogEn);
EXPORT_SYMBOL(MApi_XC_EX_EnableWindow);
EXPORT_SYMBOL(MApi_XC_EX_Is_SubWindowEanble);
EXPORT_SYMBOL(MApi_XC_EX_SetBorderFormat);
EXPORT_SYMBOL(MApi_XC_EX_EnableBorder);
EXPORT_SYMBOL(MApi_XC_EX_ZorderMainWindowFirst);
EXPORT_SYMBOL(MApi_XC_EX_PQ_LoadFunction);
EXPORT_SYMBOL(MApi_XC_EX_Check_HNonLinearScaling);
EXPORT_SYMBOL(MApi_XC_EX_EnableEuroHdtvSupport);
EXPORT_SYMBOL(MApi_XC_EX_EnableEuroHdtvDetection);
EXPORT_SYMBOL(MApi_XC_EX_ReadByte);
EXPORT_SYMBOL(MApi_XC_EX_WriteByte);
EXPORT_SYMBOL(MApi_XC_EX_WriteByteMask);
EXPORT_SYMBOL(MApi_XC_EX_Write2ByteMask);
EXPORT_SYMBOL(MApi_XC_EX_W2BYTE);
EXPORT_SYMBOL(MApi_XC_EX_R2BYTE);
EXPORT_SYMBOL(MApi_XC_EX_W4BYTE);
EXPORT_SYMBOL(MApi_XC_EX_R4BYTE);
EXPORT_SYMBOL(MApi_XC_EX_R2BYTEMSK);
EXPORT_SYMBOL(MApi_XC_EX_W2BYTEMSK);
EXPORT_SYMBOL(MApi_XC_EX_MLoad_Init);
EXPORT_SYMBOL(MApi_XC_EX_MLoad_Enable);
EXPORT_SYMBOL(MApi_XC_EX_MLoad_GetStatus);
EXPORT_SYMBOL(MApi_XC_EX_MLoad_WriteCmd_And_Fire);
EXPORT_SYMBOL(MApi_XC_EX_MLoad_WriteCmds_And_Fire);
EXPORT_SYMBOL(MApi_XC_EX_MLG_Init);
EXPORT_SYMBOL(MApi_XC_EX_MLG_Enable);
EXPORT_SYMBOL(MApi_XC_EX_MLG_GetCaps);
EXPORT_SYMBOL(MApi_XC_EX_MLG_GetStatus);
EXPORT_SYMBOL(MApi_XC_EX_SetOSD2VEMode);
EXPORT_SYMBOL(MApi_XC_EX_IP2_PreFilter_Enable);
EXPORT_SYMBOL(MApi_XC_EX_Get_Pixel_RGB);
EXPORT_SYMBOL(MApi_XC_EX_KeepPixelPointerAppear);
EXPORT_SYMBOL(MApi_XC_EX_Set_MemFmtEx);
EXPORT_SYMBOL(MApi_XC_EX_IsRequestFrameBufferLessMode);
EXPORT_SYMBOL(MApi_XC_EX_SkipSWReset);
EXPORT_SYMBOL(MApi_XC_EX_EnableRepWindowForFrameColor);
EXPORT_SYMBOL(MApi_XC_EX_SetOSDLayer);
EXPORT_SYMBOL(MApi_XC_EX_GetOSDLayer);
EXPORT_SYMBOL(MApi_XC_EX_SetVideoAlpha);
EXPORT_SYMBOL(MApi_XC_EX_GetVideoAlpha);
EXPORT_SYMBOL(MApi_XC_EX_SkipWaitVsync);
EXPORT_SYMBOL(MApi_XC_EX_OP2VOPDESel);
EXPORT_SYMBOL(MApi_XC_EX_FRC_SetWindow);
EXPORT_SYMBOL(MApi_XC_EX_Enable_TwoInitFactor);
EXPORT_SYMBOL(MApi_XC_EX_IsFieldPackingModeSupported);
EXPORT_SYMBOL(MApi_XC_EX_PreInit);
EXPORT_SYMBOL(MApi_XC_EX_Get_BufferData);
EXPORT_SYMBOL(MApi_XC_EX_Set_BufferData);
EXPORT_SYMBOL(MApi_XC_EX_EnableMainWindow);
EXPORT_SYMBOL(MApi_XC_EX_EnableSubWindow);
EXPORT_SYMBOL(MApi_XC_EX_DisableSubWindow);
EXPORT_SYMBOL(MApi_XC_EX_SetPixelShift);
EXPORT_SYMBOL(MApi_XC_EX_SetVideoOnOSD);
EXPORT_SYMBOL(MApi_XC_EX_SetOSDBlendingFormula);
EXPORT_SYMBOL(MApi_XC_EX_ReportPixelInfo);
EXPORT_SYMBOL(MApi_XC_EX_SetScaling);
EXPORT_SYMBOL(MApi_XC_EX_SetMCDIBufferAddress);
EXPORT_SYMBOL(MApi_XC_EX_EnableMCDI);
EXPORT_SYMBOL(MApi_XC_EX_SendCmdToFRC);
EXPORT_SYMBOL(MApi_XC_EX_GetMsgFromFRC);
EXPORT_SYMBOL(MApi_XC_EX_EnableRWBankAuto);
EXPORT_SYMBOL(MApi_XC_EX_SetWRBankMappingNum);
EXPORT_SYMBOL(MApi_XC_EX_GetWRBankMappingNum);
EXPORT_SYMBOL(MApi_XC_EX_GetWRBankMappingNumForZap);
EXPORT_SYMBOL(MApi_XC_EX_SetBOBMode);
EXPORT_SYMBOL(MApi_XC_EX_SetForceReadBank);
EXPORT_SYMBOL(MApi_XC_EX_LD_Init);
EXPORT_SYMBOL(MApi_XC_EX_LD_SetMemoryAddress);
EXPORT_SYMBOL(MApi_XC_EX_LD_SetLevel);
EXPORT_SYMBOL(MApi_XC_EX_Set_TurnoffLDBL);
EXPORT_SYMBOL(MApi_XC_EX_Set_notUpdateSPIDataFlags);
EXPORT_SYMBOL(MApi_XC_EX_Set_UsermodeLDFlags);
EXPORT_SYMBOL(MApi_XC_EX_Set_BLLevel);
EXPORT_SYMBOL(MApi_XC_EX_Set_BWS_Mode);
EXPORT_SYMBOL(MApi_XC_EX_FRC_ColorPathCtrl);
EXPORT_SYMBOL(MApi_XC_EX_FRC_OP2_SetRGBGain);
EXPORT_SYMBOL(MApi_XC_EX_FRC_OP2_SetRGBOffset);
EXPORT_SYMBOL(MApi_XC_EX_FRC_OP2_SetDither);
EXPORT_SYMBOL(MApi_XC_EX_FRC_BypassMFC);
EXPORT_SYMBOL(MApi_XC_EX_ForceReadFrame);
EXPORT_SYMBOL(MApi_XC_EX_SetCsc);
EXPORT_SYMBOL(MApi_XC_EX_RegisterPQSetFPLLThreshMode);
EXPORT_SYMBOL(MApi_XC_EX_GetFreeRunStatus);
EXPORT_SYMBOL(MApi_XC_EX_Get_DSForceIndexSupported);
EXPORT_SYMBOL(MApi_XC_EX_Set_DSForceIndex);
EXPORT_SYMBOL(MApi_XC_EX_Set_DSIndexSourceSelect);
EXPORT_SYMBOL(MApi_XC_EX_OSDC_InitSetting);
EXPORT_SYMBOL(MApi_XC_EX_OSDC_Control);
EXPORT_SYMBOL(MApi_XC_EX_OSDC_GetDstInfo);
EXPORT_SYMBOL(MApi_XC_EX_OSDC_SetOutVfreqx10);
EXPORT_SYMBOL(MApi_XC_EX_BYPASS_SetOSDVsyncPos);
EXPORT_SYMBOL(MApi_XC_EX_BYPASS_SetInputSrc);
EXPORT_SYMBOL(MApi_XC_EX_BYPASS_SetCSC);
EXPORT_SYMBOL(MApi_XC_EX_SetSeamlessZapping);
EXPORT_SYMBOL(MApi_XC_EX_Vtrack_SetPayloadData);
EXPORT_SYMBOL(MApi_XC_EX_Vtrack_SetUserDefindedSetting);
EXPORT_SYMBOL(MApi_XC_EX_Vtrack_Enable);
EXPORT_SYMBOL(MApi_XC_EX_PreSetPQInfo);
EXPORT_SYMBOL(MApi_XC_EX_Is_OP1_TestPattern_Enabled);
EXPORT_SYMBOL(MApi_XC_EX_Set_OP1_TestPattern);
EXPORT_SYMBOL(MApi_XC_EX_Set_WhiteBalance_Pattern);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_MODEPARSE_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
EXPORT_SYMBOL(MApi_XC_ModeParse_Init);
EXPORT_SYMBOL(MApi_XC_ModeParse_MatchMode);
EXPORT_SYMBOL(MApi_XC_ModeParse_MatchModeEx);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_MODEPARSE_EX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_ModeParse_EX_Init);
EXPORT_SYMBOL(MApi_XC_ModeParse_EX_MatchMode);
EXPORT_SYMBOL(MApi_XC_ModeParse_EX_MatchModeEx);
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_MODE_PARSE_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_PC_MONITOR_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_PCMonitor_Init);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Restart);
EXPORT_SYMBOL(MApi_XC_PCMonitor_SetTimingCount);
EXPORT_SYMBOL(MApi_XC_PCMonitor);
EXPORT_SYMBOL(MApi_XC_PCMonitor_GetCurrentState);
EXPORT_SYMBOL(MApi_XC_PCMonitor_GetSyncStatus);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_HFreqx10);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_HFreqx1K);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_VFreqx10);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_VFreqx1K);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_Vtotal);
EXPORT_SYMBOL(MApi_XC_PCMonitor_Get_Dvi_Hdmi_De_Info);
EXPORT_SYMBOL(MApi_XC_PCMonitor_SyncLoss);
EXPORT_SYMBOL(MApi_XC_PCMonitor_InvalidTimingDetect);
EXPORT_SYMBOL(MApi_XC_PCMonitor_SetTimingCountEx);
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_PC_MONITOR_EX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Init);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Restart);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_SetTimingCount);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_GetCurrentState);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_GetSyncStatus);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_HFreqx10);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_HFreqx1K);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_VFreqx10);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_VFreqx1K);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_Vtotal);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_Get_Dvi_Hdmi_De_Info);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_SyncLoss);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_InvalidTimingDetect);
EXPORT_SYMBOL(MApi_XC_PCMonitor_EX_SetTimingCountEx);
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_PCMONITOR_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_VMARK_H_
#ifdef __cplusplus
#endif
#ifdef UFO_XC_VMARK
EXPORT_SYMBOL(MApi_XC_VMark_SetParameters);
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_XC_VMARK_EX_H_
#ifdef __cplusplus
#endif
#ifdef UFO_XC_VMARK
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_VMARK_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) ||defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) ||defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_500)||defined(UFO_PUBLIC_HEADER_700)
#ifdef UFO_XC_SET_DSINFO_V0
#endif
#elif defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) ||defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#elif (defined(UFO_PUBLIC_HEADER_500) && defined(UFO_XC_FORCEWRITE_V2))
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#ifdef UFO_XC_AUTO_DOWNLOAD
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
#endif
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#ifdef UFO_XC_SUPPORT_DUAL_MIU
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500_3)
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#ifdef UFO_XC_FB_LEVEL
#endif
#ifdef UFO_XC_DE_MODE
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_300)|| defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(STELLAR)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_300)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_300) ||defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500_3) || defined(UFO_PUBLIC_HEADER_300) ||defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#elif (defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700))
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_700) || defined(UFO_PUBLIC_HEADER_500)
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#ifdef UFO_XC_SCAN_TYPE
#endif
#ifdef UFO_XC_SET_DSINFO_V0
#endif
#if 0
#endif
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
#endif
#ifdef UFO_XC_TEST_PATTERN
#endif
#ifdef UFO_XC_AUTO_DOWNLOAD
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#ifdef UFO_XC_FRAMEBUFFERCAPS
#endif
#ifdef UFO_XC_DE_MODE
#endif
#ifdef UFO_XC_SUPPORT_DUAL_MIU
#endif
#ifdef UFO_XC_SUPPORT_HDMI_DOWNSCALE_OUTPUT_POLICY
#endif
#ifdef _XC_C_
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _API_MFE_FRMFORMAT_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _ASM_CPU_H_
#ifdef __cplusplus
#endif
#if defined(__MIPS16E__)
#else
#endif
EXPORT_SYMBOL(MAsm_CPU_Sync);
EXPORT_SYMBOL(MAsm_CPU_Nop);
EXPORT_SYMBOL(MAsm_CPU_SwDbgBp);
EXPORT_SYMBOL(MAsm_CPU_PowerDown);
EXPORT_SYMBOL(MAsm_CPU_StatusBEV);
EXPORT_SYMBOL(MAsm_CPU_Jump);
EXPORT_SYMBOL(MAsm_CPU_GetTrailOne);
EXPORT_SYMBOL(MAsm_CPU_EnableTimerInterrupt);
EXPORT_SYMBOL(MAsm_CPU_DelayMs);
EXPORT_SYMBOL(MAsm_CPU_DelayUs);
EXPORT_SYMBOL(MAsm_CPU_SetEBASE);
EXPORT_SYMBOL(MAsm_CPU_TimerInit);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_CUS_SAR_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVAESDMA_H_
#ifdef __cplusplus
#endif
#if (SHA_WAIT==1)
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#ifndef MSOS_TYPE_NUTTX
#endif
#if defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined(UFO_PUBLIC_HEADER_700)||defined(UFO_PUBLIC_HEADER_500)
#endif
#if (defined(CONFIG_NUTTX) || defined(CONFIG_OPTEE))
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_AESDMA_V2_H_
#ifdef __cplusplus
#endif
#ifdef BIFROST_32BIT_MODE
#else
#endif
#ifdef BIFROST_32BIT_MODE
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_AKL_H__
#ifdef UFO_AKL_CERT
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef __DRV_AKL_V2_H__
#ifdef UFO_AKL_CERT
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _DRVAUDIO_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_AUDIO_IF_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#endif
#if defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVAUDSP_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_AVD_H_
#ifdef __cplusplus
#endif
#if (CUSTOMER == MSTAR)
#elif (CUSTOMER == STELLAR)
#endif
#if !defined (__aarch64__)
#endif
#if !defined (__aarch64__)
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVBDMA_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef MOBF_ENABLE
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef MOBF_ENABLE
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_BDMA_V2_H_
#ifdef __cplusplus
#endif
#ifdef BIFROST_32BIT_MODE
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef MOBF_ENABLE
#endif
#else
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef MOBF_ENABLE
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_CA_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_CA_V2_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_CH34_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _CH34_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRVCIPHER_H__
#ifdef __cplusplus
#endif
#ifdef ENABLE_BGC
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_CIPHER_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVCLKM_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVCMAPOOL_H_
#if defined (MSOS_TYPE_LINUX) || defined (MSOS_TYPE_OPTEE)
#ifdef __cplusplus
#endif
#if defined(CHIP_MONET) || defined(CHIP_MANHATTAN)
#else
#endif
#ifdef UFO_PUBLIC_HEADER_300
#else
#endif
#ifdef UFO_PUBLIC_HEADER_300
#else
#endif
#ifdef UFO_PUBLIC_HEADER_300
#else
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _DRVCMAPOOL_H_
#ifdef __cplusplus
#endif
#if 1
#endif
#if defined(CHIP_MONET)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVCMDQ_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_CPU_H_
#ifdef __cplusplus
#endif
#if (CPU_UTOPIA20)
#endif
#if !defined(CONFIG_FRC)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_CPU_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVDDC2BI_H_
#endif
#ifdef _DRVDIP_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_ATSC_H_
#ifndef UTPA2
#else
#endif
#if DMD_ATSC_3PARTY_EN
#if DMD_ATSC_3PARTY_IN_KERNEL_EN
#ifndef UTPA2
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#endif
#endif
#endif
#if DMD_ATSC_UTOPIA_EN || DMD_ATSC_UTOPIA2_EN
#ifndef MSIF_TAG
#endif
#endif
#if DMD_ATSC_UTOPIA2_EN || (DMD_ATSC_STR_EN && DMD_ATSC_UTOPIA_EN)
#endif
#ifndef DLL_PACKED
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#if DMD_ATSC_3PARTY_EN
#else
#endif
#endif
#if DMD_ATSC_MULTI_DMD_EN
#else
#endif
#if DMD_ATSC_UTOPIA_EN || DMD_ATSC_UTOPIA2_EN
#endif
#ifndef BIT_
#endif
#ifdef UTPA2
#endif
#if (DMD_ATSC_STR_EN && !DMD_ATSC_UTOPIA_EN && !DMD_ATSC_UTOPIA2_EN)
#endif
#if !DMD_ATSC_UTOPIA_EN && !DMD_ATSC_UTOPIA2_EN
#endif
#if DMD_ATSC_STR_EN
#endif
#ifdef __cplusplus
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#if DMD_ATSC_STR_EN
#endif
#ifndef UTPA2
#endif
#if DMD_ATSC_STR_EN
#endif
#endif
#ifdef UTPA2
#if DMD_ATSC_STR_EN
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_ATSC_V2_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DTMB_H_
#if !defined UTPA2
#else
#endif
#if DMD_DTMB_3PARTY_EN
#if DMD_DTMB_3PARTY_IN_KERNEL_EN
#ifndef UTPA2
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#endif
#endif
#endif
#if DMD_DTMB_UTOPIA_EN || DMD_DTMB_UTOPIA2_EN
#ifndef MSIF_TAG
#endif
#endif
#if DMD_DTMB_UTOPIA2_EN || (DMD_DTMB_STR_EN && DMD_DTMB_UTOPIA_EN)
#endif
#ifndef DLL_PACKED
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#if DMD_DTMB_3PARTY_EN
#else
#endif
#endif
#if DMD_DTMB_MULTI_DMD_EN
#else
#endif
#if DMD_DTMB_UTOPIA_EN || DMD_DTMB_UTOPIA2_EN
#endif
#ifndef BIT_
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#endif
#if (DMD_DTMB_STR_EN && !DMD_DTMB_UTOPIA_EN && !DMD_DTMB_UTOPIA2_EN)
#endif
#if !DMD_DTMB_UTOPIA_EN && !DMD_DTMB_UTOPIA2_EN
#endif
#if DMD_DTMB_STR_EN
#endif
#ifdef __cplusplus
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#if DMD_DTMB_STR_EN
#endif
#if DMD_DTMB_STR_EN
#endif
#endif
#ifdef UTPA2
#if DMD_DTMB_STR_EN
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DTMB_V2_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#else
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT_H_
#ifdef __cplusplus
#endif
#if 1
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT_H_
#ifdef __cplusplus
#endif
#if 1
#else
#endif
#if (SUPPORT_MULTI_DEMOD)
#endif
#ifdef __cplusplus
#endif
#endif
#if 0
#ifdef _DRV_ATSC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _DRV_DVBC_H_
#ifdef __cplusplus
#endif
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBC_GET_AGC_INFO
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBC_V2_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBC_GET_AGC_INFO
#endif
#ifdef UFO_DEMOD_DVBC_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBC_GET_AGC_INFO
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBS_H_
#ifdef __cplusplus
#endif
#ifdef UFO_DEMOD_DVBS_BERINFO
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_DEMOD_GetParam_NEW_FLOW)
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#ifdef UFO_DEMOD_DVBS_SUPPORT_DISEQC_RX
#endif
#ifdef UFO_SUPPORT_VCM
#endif
#ifdef UFO_SUPPORT_KERNEL_FLOATING
#endif
#ifdef UFO_DEMOD_BLINDSCAN_NEW_FLOW
#endif
#ifdef UFO_DEMOD_DVBS_BERINFO
#endif
#ifdef UFO_DEMOD_DVBS_TSCONTROL
#endif
#ifdef UFO_DEMOD_DVBS_SUPPORT_DISEQC_RX
#endif
#ifdef UFO_DEMOD_DVBS_CUSTOMIZED_DISEQC_SEND_CMD
#endif
#ifdef UFO_SUPPORT_VCM
#endif
#ifdef UFO_SUPPORT_KERNEL_FLOATING
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBS_V2_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef UFO_SUPPORT_VCM
#endif
#ifdef UFO_DEMOD_DVBS_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_GetParam_NEW_FLOW
#endif
#ifdef UFO_DEMOD_GET_AGC_INFO
#endif
#ifdef UFO_DEMOD_DVBS_TSCONTROL
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT_H_
#ifdef __cplusplus
#endif
#ifdef UFO_DEMOD_DVBT_SUPPORT_DMD_INT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT2_H_
#ifdef __cplusplus
#endif
#ifdef UFO_DEMOD_DVBT2_SUPPORT_DMD_INT
#endif
#ifdef UFO_DEMOD_DVBT2_SUPPORT_DMD_INT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT2_V2_H_
#ifdef __cplusplus
#endif
#if 0
#endif
#if 0
#else
#endif
#if 0
#else
#endif
#if 0
#else
#endif
#if 0
#else
#endif
#if 0
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DVBT_V2_H_
#ifdef __cplusplus
#endif
#if(0)
#endif
#if(0)
#endif
#if(0)
#endif
#if(0)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_ISDBT_H_
#if !defined UTPA2
#else
#endif
#if DMD_ISDBT_3PARTY_EN
#if DMD_ISDBT_3PARTY_IN_KERNEL_EN
#ifndef UTPA2
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#endif
#endif
#endif
#if DMD_ISDBT_UTOPIA_EN || DMD_ISDBT_UTOPIA2_EN
#ifndef MSIF_TAG
#endif
#endif
#if DMD_ISDBT_UTOPIA2_EN || (DMD_ISDBT_STR_EN && DMD_ISDBT_UTOPIA_EN)
#endif
#ifndef DLL_PACKED
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#else
#if DMD_ISDBT_3PARTY_EN
#else
#endif
#endif
#if DMD_ISDBT_MULTI_DMD_EN
#else
#endif
#if DMD_ISDBT_UTOPIA_EN || DMD_ISDBT_UTOPIA2_EN
#endif
#ifndef BIT_
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#endif
#if (DMD_ISDBT_STR_EN && !DMD_ISDBT_UTOPIA_EN && !DMD_ISDBT_UTOPIA2_EN)
#endif
#if !DMD_ISDBT_UTOPIA_EN && !DMD_ISDBT_UTOPIA2_EN
#endif
#if DMD_ISDBT_STR_EN
#endif
#ifdef __cplusplus
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#if DMD_ISDBT_STR_EN
#endif
#if DMD_ISDBT_STR_EN
#endif
#endif
#ifdef UTPA2
#if DMD_ISDBT_STR_EN
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_ISDBT_V2_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifdef UTPA2
#else
#endif
#ifdef UTPA2
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SYS_DMD_VD_MBX_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_Init);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_SetType);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_ReadReg);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_ReadDSPReg);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_DBG_ReadReg);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_WriteReg);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_WriteDSPReg);
EXPORT_SYMBOL(MDrv_SYS_DMD_VD_MBX_DBG_WriteReg);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_DMD_COMMON_H_
#ifdef __cplusplus
#endif
#if(1)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_DSCMB_H__
#ifdef __cplusplus
#endif
#if !(defined(MSOS_TYPE_OPTEE) || defined(MSOS_TYPE_NUTTX))
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_DSCMB_V2_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVEMMFLT_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRVGPIO_H__
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MDrv_GPIO_SetIOMapBase);
EXPORT_SYMBOL(MDrv_GPIO_GetLibVer);
EXPORT_SYMBOL(MDrv_GPIO_SetDbgLevel);
EXPORT_SYMBOL(MDrv_GPIO_GetInfo);
EXPORT_SYMBOL(MDrv_GPIO_GetStatus);
EXPORT_SYMBOL(MDrv_GPIO_set_high);
EXPORT_SYMBOL(MDrv_GPIO_set_low);
EXPORT_SYMBOL(MDrv_GPIO_set_input);
EXPORT_SYMBOL(MDrv_GPIO_get_inout);
EXPORT_SYMBOL(MDrv_GPIO_get_level);
EXPORT_SYMBOL(MDrv_GPIO_SetPowerState);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_HDCP_H_
#endif
#ifdef _DRV_HVD_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_HWI2C_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_HWI2C_V2_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
#if 0
#endif
#if 1
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVCMAPOOL_H_
#if defined (MSOS_TYPE_LINUX)
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _DRVCMAPOOL_H_
#if defined (CONFIG_MSTAR_IPAPOOL)
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#endif
#ifdef _DRVIPAUTH_H_
#ifdef __cplusplus
#endif
#ifndef DLL_PUBLIC
#endif
#ifndef DLL_LOCAL
#endif
#if (IP1024_SUPPORT)
#elif(IP256_SUPPORT)
#else
#endif
EXPORT_SYMBOL(MDrv_AUTH_IPCheck);
EXPORT_SYMBOL(MDrv_AUTH_GetHashInfo);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_IR_H_
#ifdef __cplusplus
#endif
#if 1
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_IRQ_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_LDM_H_
#ifdef __cplusplus
#endif
#ifdef MDRV_LDM_C
#else
#endif
#if defined(MSOS_TYPE_LINUX)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MBX_H
#ifdef _DRV_MBX_C
#else
#endif
#ifdef __cplusplus
#endif
#if 1
#endif
#if 1
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MFC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MIU_H_
#ifdef __cplusplus
#endif
#ifdef STELLAR
#endif
#ifdef ALIGN
#else
#endif
#ifdef _MIU_INTERNEL_USE
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MIU_V2_H_
#ifdef __cplusplus
#endif
#ifdef BIFROST_32BIT_MODE
#if defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#else
#if defined(UFO_PUBLIC_HEADER_212)||defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_MMIO_H__
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MDrv_MMIO_Init);
EXPORT_SYMBOL(MDrv_MMIO_GetBASE);
EXPORT_SYMBOL(MDrv_MMIO_Close);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MPIF_H_
#ifdef __cplusplus
#endif
#ifdef MPIF_SPI_SUPPORT
#endif
#ifdef MPIF_SPI_SUPPORT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MSPI_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MSPI_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MVD_H_
#ifdef __cplusplus
#endif
#ifdef REDLION_LINUX_KERNEL_ENVI
#else
#endif
#ifndef REDLION_LINUX_KERNEL_ENVI
#endif
#ifdef MVD_ENABLE_ISR
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_MVOP_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_700)
#endif
#if defined (__aarch64__)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _MVOP_H_
#ifdef __cplusplus
#endif
#if 1
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_NDS_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVRASP_H_
#ifdef __cplusplus
#endif
#ifdef HW_PVR_ENABLE
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if 0
#endif
#ifdef UFO_PUBLIC_HEADER_500_3
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVNSK2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_NSK2TYPE_H_
#endif
#ifdef _DRV_PARFLASH_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_PCMCIA_H_
#ifdef __cplusplus
#endif
#if PCMCIA_IRQ_ENABLE
#endif
#if 0
#if PCMCIA_IRQ_ENABLE
#endif
#else
#if PCMCIA_IRQ_ENABLE
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_PM_H_
#ifdef __cplusplus
#endif
#ifdef __AEONR2__
#endif
#if defined(MSOS_TYPE_ECOS)
#else
#endif
#if defined(UFO_PM_BACKLIGHT_CTRL)
#endif
#ifdef MSOS_TYPE_LINUX
#endif
#if defined(UFO_PM_BACKLIGHT_CTRL)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_PVR_IframeLUT_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_PVR_IFRAMELUT_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_TEMP_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_PWM_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVPWS_H_
#ifdef __cplusplus
#endif
#if defined (MSOS_TYPE_LINUX)
#endif
#if defined (MSOS_TYPE_LINUX)
#endif
#if defined (MSOS_TYPE_LINUX)
#endif
#if defined (MSOS_TYPE_LINUX)
#endif
#if defined (MSOS_TYPE_LINUX)
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_RTC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_RVD_H_
#ifdef __cplusplus
#endif
#if 0
#if defined(REDLION_LINUX_KERNEL_ENVI)
#else
#endif
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SAR_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_SC_H__
#ifdef __cplusplus
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SC_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SEAL_H_
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_500)
#else
#endif
#ifdef MSOS_TYPE_OPTEE
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_SEM_H__
#ifdef __cplusplus
#endif
#if (defined(MCU_AEON))
#else
#endif
EXPORT_SYMBOL(MDrv_SEM_Init);
EXPORT_SYMBOL(MDrv_SEM_Get_Resource);
EXPORT_SYMBOL(MDrv_SEM_Free_Resource);
EXPORT_SYMBOL(MDrv_SEM_Reset_Resource);
EXPORT_SYMBOL(MDrv_SEM_Get_ResourceID);
EXPORT_SYMBOL(MDrv_SEM_Get_Num);
EXPORT_SYMBOL(MDrv_SEM_GetLibVer);
EXPORT_SYMBOL(MDrv_SEM_SetPowerState);
EXPORT_SYMBOL(MDrv_SEM_Lock);
EXPORT_SYMBOL(MDrv_SEM_Unlock);
EXPORT_SYMBOL(MDrv_SEM_Delete);
EXPORT_SYMBOL(MDrv_SEM_SetDbgLevel);
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SERFLASH_H_
#ifdef __cplusplus
#endif
#ifdef MXIC_IBPM
#endif
#ifdef MUNINN_ENABLE
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SERFLASH_V2_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_SMBX_H__
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_STBDC_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_SYS_H_
#ifdef __cplusplus
#endif
EXPORT_SYMBOL(MDrv_SYS_Init);
EXPORT_SYMBOL(MDrv_SYS_GlobalInit);
EXPORT_SYMBOL(MDrv_SYS_GetInfo);
EXPORT_SYMBOL(MDrv_SYS_GetChipRev);
EXPORT_SYMBOL(MDrv_SYS_GetChipID);
EXPORT_SYMBOL(MDrv_SYS_WDTEnable);
EXPORT_SYMBOL(MDrv_SYS_WDTClear);
EXPORT_SYMBOL(MDrv_SYS_WDTLastStatus);
EXPORT_SYMBOL(MDrv_SYS_WDTSetTime);
EXPORT_SYMBOL(MDrv_SYS_ResetChip);
EXPORT_SYMBOL(MDrv_SYS_ResetCPU);
EXPORT_SYMBOL(MDrv_SYS_SetDbgLevel);
EXPORT_SYMBOL(MDrv_SYS_VIFWriteByteByVDMbox);
EXPORT_SYMBOL(MDrv_SYS_VIFWriteByteMaskByVDMbox);
EXPORT_SYMBOL(MDrv_SYS_VIFWriteRegBitByVDMbox);
EXPORT_SYMBOL(MDrv_SYS_VIFReadByteByVDMbox);
EXPORT_SYMBOL(MDrv_SYS_VIFRead2ByteByVDMbox);
EXPORT_SYMBOL(MDrv_SYS_Query);
#ifdef _SYS_DAC_GAIN_CTRL_
#endif
EXPORT_SYMBOL(MDrv_SYS_GetSoftwareVersion);
EXPORT_SYMBOL(MDrv_SYS_SetAGCPadMux);
EXPORT_SYMBOL(MDrv_SYS_SetPadMux);
EXPORT_SYMBOL(MDrv_SYS_PackMode);
EXPORT_SYMBOL(MDrv_SYS_SetPCMCardDetectMode);
EXPORT_SYMBOL(MDrv_SYS_DisableDebugPort);
EXPORT_SYMBOL(MDrv_SYS_EnableDebugPort);
EXPORT_SYMBOL(MDrv_SYS_SetTSOutClockPhase);
EXPORT_SYMBOL(MDrv_SYS_SetTSClockPhase);
EXPORT_SYMBOL(MDrv_SYS_PadMuxTableSuspend);
EXPORT_SYMBOL(MDrv_SYS_PadMuxTableResume);
EXPORT_SYMBOL(MDrv_SYS_SetPowerState);
EXPORT_SYMBOL(MDrv_SYS_QueryDolbyHashInfo);
EXPORT_SYMBOL(MDrv_SYS_GetChipType);
EXPORT_SYMBOL(MDrv_SYS_SetChipType);
EXPORT_SYMBOL(MDrv_SYS_GetDolbyKeyCustomer);
EXPORT_SYMBOL(MDrv_SYS_ReadBrickTerminatorStatus);
EXPORT_SYMBOL(MDrv_SYS_WriteBrickTerminatorStatus);
EXPORT_SYMBOL(MDrv_SYS_GetEfuseDid);
EXPORT_SYMBOL(MDrv_SYS_ReadEfuseHDCPKey);
EXPORT_SYMBOL(MDrv_SYS_GetIpList);
EXPORT_SYMBOL(MDrv_SYS_RegisterIoProc);
EXPORT_SYMBOL(MDrv_SYS_ReadKernelCmdLine);
EXPORT_SYMBOL(MDrv_SYS_SetTEEInfo);
EXPORT_SYMBOL(MDrv_SYS_GetTEEInfo);
#ifdef UFO_SYS_PADMUX
#endif
EXPORT_SYMBOL(MDrv_SYS_GetUtopiaReleaseLabel);
#ifdef CONFIG_MSTAR_DVFS_KERNEL_SUPPORT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_TCF_H__
#endif
#ifdef _DRVTVENCODER_H
#ifdef __cplusplus
#endif
#ifdef UFO_VE_INITEX
#endif
EXPORT_SYMBOL(MDrv_VE_GetLibVer);
EXPORT_SYMBOL(MDrv_VE_GetInfo);
EXPORT_SYMBOL(MDrv_VE_GetStatus);
EXPORT_SYMBOL(MDrv_VE_SetDbgLevel);
EXPORT_SYMBOL(MDrv_VE_GetCaps);
EXPORT_SYMBOL(MDrv_VE_SwitchInputSource);
EXPORT_SYMBOL(MDrv_VE_SetInputSource);
EXPORT_SYMBOL(MDrv_VE_SwitchOuputDest);
EXPORT_SYMBOL(MDrv_VE_SetOutputCtrl);
EXPORT_SYMBOL(MDrv_VE_SetOutputVideoStd);
EXPORT_SYMBOL(MDrv_VE_SetCusTable);
EXPORT_SYMBOL(MDrv_VE_EnableCusTable);
EXPORT_SYMBOL(MDrv_VE_SetIOMapBase);
EXPORT_SYMBOL(MDrv_VE_SetMemoryBaseAddr);
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MDrv_VE_Init);
#else
EXPORT_SYMBOL(MDrv_VE_Init);
#endif
#ifdef UFO_VE_INITEX
#endif
#if defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MDrv_VE_GetConfig);
#else
EXPORT_SYMBOL(MDrv_VE_GetConfig);
#endif
EXPORT_SYMBOL(MDrv_VE_Exit);
EXPORT_SYMBOL(MDrv_VE_SetWSSData);
EXPORT_SYMBOL(MDrv_VE_GetWSSData);
EXPORT_SYMBOL(MDrv_VE_SetMode);
EXPORT_SYMBOL(MDrv_VE_SetBlackScreen);
EXPORT_SYMBOL(MDrv_VE_IsBlackScreenEnabled);
EXPORT_SYMBOL(MDrv_VE_EnableTtx);
#if defined(UFO_PUBLIC_HEADER_500_3)||defined(UFO_PUBLIC_HEADER_700)
EXPORT_SYMBOL(MDrv_VE_SetTtxBuffer);
#else
EXPORT_SYMBOL(MDrv_VE_SetTtxBuffer);
#endif
EXPORT_SYMBOL(MDrv_VE_ClearTtxReadDoneStatus);
EXPORT_SYMBOL(MDrv_VE_GetTtxReadDoneStatus);
EXPORT_SYMBOL(MDrv_VE_SetVbiTtxActiveLines);
EXPORT_SYMBOL(MDrv_VE_SetVbiTtxActiveLinesBitmap);
EXPORT_SYMBOL(MDrv_VE_SetVbiTtxRange);
EXPORT_SYMBOL(MDrv_VE_EnableCcSw);
EXPORT_SYMBOL(MDrv_VE_SetCcRange);
EXPORT_SYMBOL(MDrv_VE_SendCcData);
EXPORT_SYMBOL(MDrv_VE_set_display_window);
EXPORT_SYMBOL(MDrv_VE_SetFrameColor);
EXPORT_SYMBOL(MDrv_VE_SetOSD);
EXPORT_SYMBOL(MDrv_VE_Set_OSDLayer);
EXPORT_SYMBOL(MDrv_VE_Get_OSDLayer);
EXPORT_SYMBOL(MDrv_VE_Set_VideoAlpha);
EXPORT_SYMBOL(MDrv_VE_Get_VideoAlpha);
EXPORT_SYMBOL(MDrv_VE_SetRGBIn);
EXPORT_SYMBOL(MDrv_VE_Get_Output_Video_Std);
EXPORT_SYMBOL(MDrv_VE_SetCaptureMode);
EXPORT_SYMBOL(MApi_VE_GetDstInfo);
EXPORT_SYMBOL(MDrv_VE_Set_TestPattern);
EXPORT_SYMBOL(MApi_VE_W2BYTE_MSK);
EXPORT_SYMBOL(MApi_VE_R2BYTE_MSK);
EXPORT_SYMBOL(MDrv_VE_DumpTable);
EXPORT_SYMBOL(MDrv_VE_DisableRegWrite);
EXPORT_SYMBOL(MDrv_VE_ShowColorBar);
EXPORT_SYMBOL(MDrv_VE_AdjustPositionBase);
EXPORT_SYMBOL(MDrv_VE_SetFrameLock);
EXPORT_SYMBOL(MDrv_VE_Set_Customer_Scaling);
EXPORT_SYMBOL(MDrv_VE_set_crop_window);
EXPORT_SYMBOL(MDrv_VE_set_be_display_window);
EXPORT_SYMBOL(MDrv_VE_Get_Ext_Caps);
EXPORT_SYMBOL(MDrv_VE_SetWindow);
EXPORT_SYMBOL(MDrv_VE_InitVECapture);
EXPORT_SYMBOL(MDrv_VE_EnaVECapture);
EXPORT_SYMBOL(MDrv_VE_GetVECaptureState);
EXPORT_SYMBOL(MDrv_VE_VECaptureWaitOnFrame);
EXPORT_SYMBOL(MDrv_VE_Adjust_FrameStart);
EXPORT_SYMBOL(MDrv_VE_SetWSS525Data);
EXPORT_SYMBOL(MDrv_VE_GetWSS525Data);
EXPORT_SYMBOL(MDrv_VE_OnOffWSS);
EXPORT_SYMBOL(MDrv_VE_GetWSSStatus);
EXPORT_SYMBOL(MDrv_VE_OnOffMV);
EXPORT_SYMBOL(MDrv_VE_GetMVStatus);
EXPORT_SYMBOL(MDrv_VE_OnOffDCS);
EXPORT_SYMBOL(MDrv_VE_GetDCSStatus);
EXPORT_SYMBOL(MDrv_TVE_SetPowerState);
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_500_3)
EXPORT_SYMBOL(MDrv_VE_ReloadSetting);
#endif
#ifdef UFO_PUBLIC_HEADER_212
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __MDRV_UART_H__
#ifdef __cplusplus
#endif
#if UART_TYPE_EYWA
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __MDRV_URDMA_H__
#ifdef __cplusplus
#endif
#ifdef MS_DEBUG
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVUSB_H
#ifdef USB_SYSTEM_STR_SUPPORT
#endif
#endif
#ifdef __USBCOMMON_H
#if 1
#else
#endif
#if 1
#else
#endif
#ifndef U32
#endif
#ifndef U16
#endif
#ifndef U8
#endif
#ifndef S32
#endif
#ifndef S16
#endif
#ifndef S8
#endif
#ifndef __u32
#endif
#ifndef __u16
#endif
#ifndef __u8
#endif
#ifndef __s32
#endif
#ifndef __s16
#endif
#ifndef __s8
#endif
#ifndef BOOL
#endif
#ifndef BOOLEAN
#endif
#ifdef MS_DEBUG
#else
#endif
#if defined(CHIP_U4)
#elif defined(CHIP_K1)
#elif defined(CHIP_K2)
#elif defined(CHIP_KAPPA)
#elif defined(CHIP_KELTIC)
#elif defined(CHIP_KENYA)
#elif defined(CHIP_KRITI)
#elif defined(CHIP_KRATOS)
#elif defined(CHIP_KAISER)
#elif defined(CHIP_KERES)
#elif defined(CHIP_KIRIN)
#elif defined(CHIP_KRIS)
#elif defined(CHIP_KIWI)
#elif defined(CHIP_CLIPPERS)
#elif defined(CHIP_KAYLA)
#elif defined(CHIP_KANO)
#elif defined(CHIP_K6)
#elif defined(CHIP_CURRY)
#elif defined(CHIP_K6LITE)
#elif defined(CHIP_K5TN)
#elif defined(CHIP_K7U)
#elif defined(CHIP_C2P)
#elif defined(CHIP_K1C)
#else
#endif
#endif
#ifdef _DRV_VBI_H
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef __DRV_VBI_V2_H__
#ifdef __cplusplus
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_300)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVVIF_H_
#ifdef __cplusplus
#endif
#if 0
#endif
#ifdef _DRVVIF_C_
#else
#endif
#ifndef BOOL
#endif
#ifndef BYTE
#endif
#ifndef WORD
#endif
#ifndef DWORD
#endif
#ifndef U8
#endif
#ifndef U16
#endif
#ifndef U32
#endif
#ifndef VIRT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef DRV_WBLE_H
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRV_WDT_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef DRVXC_HDMI_IF_H
#ifdef __cplusplus
#endif
#ifdef MDRV_HDMI_C
#else
#endif
#ifdef DVI_NEW_Patch
#endif
EXPORT_SYMBOL(MDrv_HDMI_Func_Caps);
EXPORT_SYMBOL(MDrv_HDMI_init);
EXPORT_SYMBOL(MDrv_HDMI_Exit);
EXPORT_SYMBOL(MDrv_HDMI_pkt_reset);
EXPORT_SYMBOL(MDrv_HDMI_pullhpd);
EXPORT_SYMBOL(MDrv_HDMI_GC_Info);
EXPORT_SYMBOL(MDrv_HDMI_Packet_Received);
EXPORT_SYMBOL(MDrv_HDMI_Get_ColorFormat);
EXPORT_SYMBOL(MDrv_HDMI_Get_ColorRange);
EXPORT_SYMBOL(MDrv_HDMI_Get_Content_Type);
EXPORT_SYMBOL(MDrv_HDMI_Get_ExtColorimetry);
EXPORT_SYMBOL(MDrv_HDMI_Set_EQ);
EXPORT_SYMBOL(MDrv_HDMI_Set_EQ_To_Port);
EXPORT_SYMBOL(MDrv_HDMI_Audio_MUTE_Enable);
EXPORT_SYMBOL(MDrv_HDMI_Audio_Status_Clear);
EXPORT_SYMBOL(MDrv_HDMI_Get_AspectRatio);
EXPORT_SYMBOL(MDrv_HDMI_Get_ActiveFormat_AspectRatio);
EXPORT_SYMBOL(MDrv_HDMI_Get_AVIInfoFrameVer);
EXPORT_SYMBOL(MDrv_HDMI_err_status_update);
EXPORT_SYMBOL(MDrv_HDMI_Get_PollingStatus);
EXPORT_SYMBOL(MDrv_HDMI_IsHDMI_Mode);
EXPORT_SYMBOL(MDrv_HDMI_Get_MID_info);
EXPORT_SYMBOL(MDrv_HDMI_get_packet_value);
EXPORT_SYMBOL(MDrv_DVI_ChannelPhaseStatus);
EXPORT_SYMBOL(MDrv_DVI_HF_adjust);
EXPORT_SYMBOL(MDrv_DVI_SoftwareReset);
EXPORT_SYMBOL(MDrv_DVI_ClkPullLow);
EXPORT_SYMBOL(MDrv_DVI_SwitchSrc);
EXPORT_SYMBOL(MDrv_HDMI_SetForClockLessThan70Mhz);
EXPORT_SYMBOL(MDrv_HDMI_dvi_adjust);
EXPORT_SYMBOL(MDrv_HDMI_SetUpdatePhaseLineCount);
EXPORT_SYMBOL(MDrv_HDMI_GetTMDSFreq);
EXPORT_SYMBOL(MDrv_HDCP_Enable);
EXPORT_SYMBOL(MDrv_HDMI_SetHdcpEnable);
EXPORT_SYMBOL(MDrv_HDCP_ClearStatus);
EXPORT_SYMBOL(MDrv_HDCP_initproductionkey);
EXPORT_SYMBOL(MDrv_HDCP_GetStatus);
EXPORT_SYMBOL(MDrv_HDCP_Vsync_end_en);
EXPORT_SYMBOL(MDrv_HDMI_audio_output);
EXPORT_SYMBOL(MDrv_HDMI_audio_cp_hdr_info);
EXPORT_SYMBOL(MDrv_HDMI_audio_channel_status);
EXPORT_SYMBOL(MDrv_HDMI_GetLibVer);
EXPORT_SYMBOL(MDrv_HDMI_GetInfo);
EXPORT_SYMBOL(MDrv_HDMI_GetStatus);
EXPORT_SYMBOL(MDrv_HDMI_Get_AVIInfoActiveInfoPresent);
EXPORT_SYMBOL(MDrv_HDMI_READ_DDCRAM);
EXPORT_SYMBOL(MDrv_HDMI_PROG_DDCRAM);
EXPORT_SYMBOL(MDrv_HDMI_Get_Pixel_Repetition);
EXPORT_SYMBOL(MDrv_HDMI_3D_4Kx2K_Process);
EXPORT_SYMBOL(MDrv_HDMI_AVG_ScaleringDown);
EXPORT_SYMBOL(MDrv_HDMI_Check4K2K);
EXPORT_SYMBOL(MDrv_HDMI_Check_Additional_Format);
EXPORT_SYMBOL(MDrv_HDMI_Get_3D_Structure);
EXPORT_SYMBOL(MDrv_HDMI_Get_3D_Ext_Data);
EXPORT_SYMBOL(MDrv_HDMI_Get_3D_Meta_Field);
EXPORT_SYMBOL(MDrv_HDMI_Get_VIC_Code);
EXPORT_SYMBOL(MDrv_HDMI_Get_4Kx2K_VIC_Code);
EXPORT_SYMBOL(MDrv_HDMI_ARC_PINControl);
EXPORT_SYMBOL(MDrv_DVI_Software_Reset);
EXPORT_SYMBOL(MDrv_HDMI_CheckHDMI20_Setting);
EXPORT_SYMBOL(MDrv_HDMI_StablePolling);
EXPORT_SYMBOL(MDrv_HDMI_GetSourceVersion);
EXPORT_SYMBOL(MDrv_HDMI_Set5VDetectGPIOSelect);
EXPORT_SYMBOL(MDrv_HDMI_GetDEStableStatus);
EXPORT_SYMBOL(MDrv_HDMI_CheckHDCP14);
EXPORT_SYMBOL(MDrv_HDMI_CheckHDCPState);
EXPORT_SYMBOL(MDrv_HDMI_Ctrl);
EXPORT_SYMBOL(MDrv_HDMI_GetDataInfo);
EXPORT_SYMBOL(MDrv_HDMI_SetPowerState);
EXPORT_SYMBOL(MDrv_HDCP22_FillCipherKey);
EXPORT_SYMBOL(MDrv_HDCP22_InitCBFunc);
EXPORT_SYMBOL(MDrv_HDCP22_PortInit);
EXPORT_SYMBOL(MDrv_HDCP22_PollingReadDone);
EXPORT_SYMBOL(MDrv_HDCP22_EnableCipher);
EXPORT_SYMBOL(MDrv_HDCP22_SendMsg);
EXPORT_SYMBOL(MDrv_HDCP22_Handler);
#if 0
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef DRVXC_HDMI_IF_EX_H
#ifdef __cplusplus
#endif
#ifdef MDRV_HDMI_EX_C
#else
#endif
#ifdef DRVXC_HDMI_IF_H
#endif
#ifdef DRVXC_HDMI_IF_H
#endif
#ifdef DVI_NEW_Patch
#endif
EXPORT_SYMBOL(MDrv_HDMI_EX_GetLibVer);
EXPORT_SYMBOL(MDrv_HDMI_EX_GetInfo);
EXPORT_SYMBOL(MDrv_HDMI_EX_GetStatus);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_AVIInfoActiveInfoPresent);
EXPORT_SYMBOL(MDrv_HDMI_EX_Func_Caps);
EXPORT_SYMBOL(MDrv_HDMI_EX_init);
EXPORT_SYMBOL(MDrv_HDMI_EX_Exit);
EXPORT_SYMBOL(MDrv_HDMI_EX_pkt_reset);
EXPORT_SYMBOL(MDrv_HDMI_EX_pullhpd);
EXPORT_SYMBOL(MDrv_HDMI_EX_GC_Info);
EXPORT_SYMBOL(MDrv_HDMI_EX_Packet_Received);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_ColorFormat);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_ColorRange);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_Content_Type);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_ExtColorimetry);
EXPORT_SYMBOL(MDrv_HDMI_EX_Set_EQ);
EXPORT_SYMBOL(MDrv_HDMI_EX_Set_EQ_To_Port);
EXPORT_SYMBOL(MDrv_HDMI_EX_Audio_MUTE_Enable);
EXPORT_SYMBOL(MDrv_HDMI_EX_Audio_Status_Clear);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_AspectRatio);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_ActiveFormat_AspectRatio);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_AVIInfoFrameVer);
EXPORT_SYMBOL(MDrv_HDMI_EX_err_status_update);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_PollingStatus);
EXPORT_SYMBOL(MDrv_HDMI_EX_IsHDMI_Mode);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_MID_info);
EXPORT_SYMBOL(MDrv_HDMI_EX_get_packet_value);
EXPORT_SYMBOL(MDrv_DVI_EX_ChannelPhaseStatus);
EXPORT_SYMBOL(MDrv_DVI_EX_SoftwareReset);
EXPORT_SYMBOL(MDrv_DVI_EX_ClkPullLow);
EXPORT_SYMBOL(MDrv_DVI_EX_SwitchSrc);
EXPORT_SYMBOL(MDrv_HDMI_EX_SetForClockLessThan70Mhz);
EXPORT_SYMBOL(MDrv_HDMI_EX_dvi_adjust);
EXPORT_SYMBOL(MDrv_HDMI_EX_SetUpdatePhaseLineCount);
EXPORT_SYMBOL(MDrv_HDCP_EX_Enable);
EXPORT_SYMBOL(MDrv_HDMI_EX_SetHdcpEnable);
EXPORT_SYMBOL(MDrv_HDCP_EX_ClearStatus);
EXPORT_SYMBOL(MDrv_HDCP_EX_initproductionkey);
EXPORT_SYMBOL(MDrv_HDCP_EX_GetStatus);
EXPORT_SYMBOL(MDrv_HDCP_EX_Vsync_end_en);
EXPORT_SYMBOL(MDrv_HDMI_EX_audio_output);
EXPORT_SYMBOL(MDrv_HDMI_EX_audio_cp_hdr_info);
EXPORT_SYMBOL(MDrv_HDMI_EX_audio_channel_status);
EXPORT_SYMBOL(MDrv_HDMI_EX_READ_DDCRAM);
EXPORT_SYMBOL(MDrv_HDMI_EX_PROG_DDCRAM);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_Pixel_Repetition);
EXPORT_SYMBOL(MDrv_HDMI_EX_3D_4Kx2K_Process);
EXPORT_SYMBOL(MDrv_HDMI_EX_AVG_ScaleringDown);
EXPORT_SYMBOL(MDrv_HDMI_EX_Check4K2K);
EXPORT_SYMBOL(MDrv_HDMI_EX_Check_Additional_Format);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_3D_Structure);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_3D_Ext_Data);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_3D_Meta_Field);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_VIC_Code);
EXPORT_SYMBOL(MDrv_HDMI_EX_Get_4Kx2K_VIC_Code);
EXPORT_SYMBOL(MDrv_HDMI_EX_ARC_PINControl);
#if 0
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _XC_HDMI_IF_H_
#ifdef __cplusplus
#endif
#if !defined(UFO_PUBLIC_HEADER_500_3)
#endif
#if defined(UFO_PUBLIC_HEADER_212) ||defined(UFO_PUBLIC_HEADER_500_3)
#else
#endif
#if defined(UFO_PUBLIC_HEADER_212)
#else
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _IOPORT_H_
#endif
#ifdef _IOPORT_EX_H_
#endif
#ifdef _DRVIRTX_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef DRV_GOP_SC_H
#ifdef DRV_GOP_SC_C
#else
#endif
#endif
#ifdef _DRV_MFE_ST_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _LABEL_H_
#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
#endif
#ifndef MSOS_TYPE_LINUX_KERNEL
#endif
#ifdef _UTOPIA_H_
#ifdef __cplusplus
#endif
#if defined(MSOS_TYPE_LINUX_KERNEL)
#endif
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
#endif
#if defined(CONFIG_COMPAT)
#endif
#ifdef CONFIG_MBOOT
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _UTOPIA_DRIVER_ID_H_
#endif
#ifdef _DRVPQ_H_
#ifdef ANDROID
#ifndef LOGE
#else
#endif
#endif
#ifdef _MDRV_PQ_C_
#else
#endif
#ifdef __cplusplus
#endif
#ifndef ENABLE_VGA_EIA_TIMING
#endif
#ifndef PQ_QM_NETMM_VIDEO
#endif
#ifndef PQ_ENABLE_3D_STRENGTHEN_NR
#endif
#ifndef PQ_QM_24HZMM_VIDEO
#endif
#ifndef PQ_QM_HDMI_FP_VIDEO
#endif
#ifndef PQ_QM_4K2K_FS
#endif
#ifndef PQ_LCE_CTL_FOR_MVC4KX1K
#endif
#ifndef PQ_DLC_CTL_FOR_MVC4KX1K
#endif
#ifndef PQ_QM_MWE_CLONE_VER1
#endif
#ifndef PQ_QM_MWE_CLONE_VER2
#endif
#ifndef PQ_VGASubCaseForceTo422Mode
#endif
#ifndef PQ_QM_3D_CLONE_ENABLE
#endif
#ifndef PQ_GRULE_GAME_MODE_ENABLE
#endif
#ifndef PQ_GRULE_HDR_ENABLE
#endif
#ifndef PQ_QM_4K2K_60Hz
#endif
#ifndef PQ_FRCM_CBCR_SWAP_BY_SW
#endif
#ifndef PQ_SUPPORT_HDMI_4K2K_P2P
#endif
#ifndef PQ_SUPPORT_DVI_4K2K
#endif
#ifndef PQ_4K2K_PHOTO
#endif
#ifndef PQ_4K2K_P2P_H_OFFSET_LIMITIOM
#endif
#ifndef ENABLE_PQ_BIN_CF
#endif
#ifndef ENABLE_PQ_BIN_TMO
#endif
#ifndef PQ_GRULE_DS_PARAMETERS_ENABLE
#endif
#ifndef assert
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#if 0
#endif
#ifdef UFO_XC_PACKED_SHM
#if !defined (__aarch64__)
#endif
#endif
#ifdef UFO_XC_PACKED_SHM
#if !defined (__aarch64__)
#endif
#endif
#if 0
#endif
#if(ENABLE_PQ_LOAD_TABLE_INFO)
#endif
EXPORT_SYMBOL(MDrv_PQ_GetLibVer);
EXPORT_SYMBOL(MDrv_PQ_GetInfo);
EXPORT_SYMBOL(MDrv_PQ_GetStatus);
EXPORT_SYMBOL(MDrv_PQ_SetDbgLevel);
EXPORT_SYMBOL(MDrv_PQ_Init);
EXPORT_SYMBOL(MDrv_PQ_GetConfig);
EXPORT_SYMBOL(MDrv_PQ_Exit);
EXPORT_SYMBOL(MDrv_PQ_DesideSrcType);
EXPORT_SYMBOL(MDrv_PQ_GetSrcType);
EXPORT_SYMBOL(MDrv_PQ_GetDelayTimeByMemc);
EXPORT_SYMBOL(MDrv_PQ_GetDelayTime);
EXPORT_SYMBOL(MDrv_PQ_GetVideoDelayTime);
EXPORT_SYMBOL(MDrv_PQ_GetQmapExecutor);
EXPORT_SYMBOL(MDrv_PQ_GetInputSourceType);
EXPORT_SYMBOL(MDrv_PQ_SetColorRange);
EXPORT_SYMBOL(MDrv_PQ_SetPhotoYUVStandard);
EXPORT_SYMBOL(MDrv_PQ_SetCSC);
EXPORT_SYMBOL(MDrv_PQ_Get_MemYUVFmt);
EXPORT_SYMBOL(MDrv_PQ_LoadScalingTable);
EXPORT_SYMBOL(MDrv_PQ_ReduceBW_ForPVR);
EXPORT_SYMBOL(MDrv_PQ_EnableMADIForce);
EXPORT_SYMBOL(MDrv_PQ_Set420upsampling);
EXPORT_SYMBOL(MDrv_PQ_SetFilmMode);
EXPORT_SYMBOL(MDrv_PQ_SetNonLinearScaling);
EXPORT_SYMBOL(MDrv_PQ_CheckSettings);
EXPORT_SYMBOL(MDrv_PQ_SkipDuplicatedSetting);
EXPORT_SYMBOL(MDrv_PQ_GetSkipDuplicatedSettingStatus);
EXPORT_SYMBOL(MDrv_PQ_LoadSettings);
EXPORT_SYMBOL(MDrv_PQ_LoadCustomerSettings);
EXPORT_SYMBOL(MDrv_PQ_LoadTable);
EXPORT_SYMBOL(MDrv_PQ_CloneTable);
EXPORT_SYMBOL(MDrv_PQ_LoadCustomerTable);
EXPORT_SYMBOL(MDrv_PQ_GetIPNum);
EXPORT_SYMBOL(MDrv_PQ_GetTableNum);
EXPORT_SYMBOL(MDrv_PQ_GetCurrentTableIndex);
EXPORT_SYMBOL(MDrv_PQ_3DCloneforPIP);
EXPORT_SYMBOL(MDrv_PQ_DisableFilmMode);
EXPORT_SYMBOL(MDrv_PQ_GetSrcTypeName);
EXPORT_SYMBOL(MDrv_PQ_GetIPName);
EXPORT_SYMBOL(MDrv_PQ_GetTableName);
EXPORT_SYMBOL(MDrv_PQ_Set_DTVInfo);
EXPORT_SYMBOL(MDrv_PQ_Set_MultiMediaInfo);
EXPORT_SYMBOL(MDrv_PQ_Set_VDInfo);
EXPORT_SYMBOL(MDrv_PQ_Set_ModeInfo);
EXPORT_SYMBOL(MDrv_PQ_SetHDMIInfo);
EXPORT_SYMBOL(MDrv_PQ_SetHDMI_PC);
EXPORT_SYMBOL(MDrv_PQ_GetHDMI_PC_Status);
EXPORT_SYMBOL(MDrv_PQ_MADiForceMotionY);
EXPORT_SYMBOL(MDrv_PQ_MADiForceMotionC);
#ifdef UFO_XC_PQ_SUPPORT_SWMOTION
EXPORT_SYMBOL(MDrv_PQ_MADiForceMotion);
#endif
EXPORT_SYMBOL(MDrv_PQ_SetRFblMode);
EXPORT_SYMBOL(MDrv_PQ_GetRFblMode);
EXPORT_SYMBOL(MDrv_PQ_Get_VDSampling_Info);
EXPORT_SYMBOL(MDrv_PQ_IOCTL);
EXPORT_SYMBOL(MDrv_PQ_Set_MLoadEn);
EXPORT_SYMBOL(MDrv_PQ_Get_ADCSampling_Info);
EXPORT_SYMBOL(MDrv_PQ_Set3D_OnOff);
EXPORT_SYMBOL(MDrv_PQ_DisableUCFeature);
EXPORT_SYMBOL(MDrv_PQ_SetUCFeature);
EXPORT_SYMBOL(MDrv_PQ_GetCurrentUCFeature);
EXPORT_SYMBOL(MDrv_PQ_SetDMSV12L);
EXPORT_SYMBOL(MDrv_PQ_GetDMSV12LFromXRuleTable);
EXPORT_SYMBOL(MDrv_PQ_GetCaps);
EXPORT_SYMBOL(MDrv_PQ_SetDS_OnOFF);
EXPORT_SYMBOL(MDrv_PQ_Get_RFBL_Info);
EXPORT_SYMBOL(MDrv_PQ_CheckHDMode);
EXPORT_SYMBOL(MDrv_PQ_Get_Interlace_SD_mode);
EXPORT_SYMBOL(MDrv_PQ_Update_MemFormat);
EXPORT_SYMBOL(MDrv_PQ_LOW_3dQuality);
EXPORT_SYMBOL(MDrv_PQ_MWE_SetEnhanceQuality);
EXPORT_SYMBOL(MDrv_PQ_MWE_RestoreEnhanceQuality);
EXPORT_SYMBOL(MDrv_PQ_MWE_RestoreOffQuality);
EXPORT_SYMBOL(MDrv_PQ_MWE_SetOffQuality);
EXPORT_SYMBOL(MDrv_PQ_MWE_CloneWindow);
EXPORT_SYMBOL(MDrv_PQ_3D_CloneWindow);
EXPORT_SYMBOL(MDrv_PQ_3D_SettingForLBL);
EXPORT_SYMBOL(MDrv_PQ_GetHsize);
EXPORT_SYMBOL(MDrv_PQ_GetVsize);
EXPORT_SYMBOL(MDrv_PQ_IsInterlace);
EXPORT_SYMBOL(MDrv_PQ_SetSelectCSC);
EXPORT_SYMBOL(MDrv_PQ_GetR2YEqSelect);
EXPORT_SYMBOL(MDrv_PQ_Set_PointToPoint);
EXPORT_SYMBOL(MDrv_PQ_GetVGASubCaseForceTo422Mode);
EXPORT_SYMBOL(MDrv_PQ_SetDualViewState);
EXPORT_SYMBOL(MDrv_PQ_GetDualViewState);
EXPORT_SYMBOL(MDrv_PQ_ForceVideoInputMode);
EXPORT_SYMBOL(MDrv_PQ_IsForceVideoInputMode);
EXPORT_SYMBOL(MDrv_PQ_Get_PointToPoint);
EXPORT_SYMBOL(MDrv_PQ_GetMemFmtInGeneral);
EXPORT_SYMBOL(MDrv_PQ_GetMADiForRFBL);
EXPORT_SYMBOL(MDrv_PQ_Patch2Rto4RForFieldPackingMode);
EXPORT_SYMBOL(MDrv_PQ_GetBPPInfoFromMADi);
EXPORT_SYMBOL(MDrv_PQ_ChangeMemConfigFor3D);
EXPORT_SYMBOL(MDrv_PQ_ForceBPPForDynamicMemFmt);
EXPORT_SYMBOL(MDrv_PQ_SuggestFrameNum);
EXPORT_SYMBOL(MDrv_PQ_SetGameMode);
EXPORT_SYMBOL(MDrv_PQ_EnableHDRMode);
EXPORT_SYMBOL(MDrv_PQ_GetGameMode_Status);
EXPORT_SYMBOL(MDrv_PQ_SetBypassMode);
EXPORT_SYMBOL(MDrv_PQ_GetBypassModeStatus);
EXPORT_SYMBOL(MDrv_PQ_GetVersion);
EXPORT_SYMBOL(MDrv_PQ_Cus_GetVersion);
EXPORT_SYMBOL(MDrv_PQ_GetTableIndex);
EXPORT_SYMBOL(MDrv_PQ_GetCustomerTableIndex);
EXPORT_SYMBOL(MDrv_PQ_SetGRuleStatus);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_LvlNum);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_GRuleNum);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_LevelIndex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_IPIndex);
EXPORT_SYMBOL(MDrv_PQ_GetCustomerGRule_IPIndex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_TableIndex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_CustomerTableIndex);
EXPORT_SYMBOL(MDrv_PQ_SetPowerState);
EXPORT_SYMBOL(MDrv_PQ_Set_xvYCC_MatrixCoefficient);
EXPORT_SYMBOL(MDrv_PQ_Set_xvYCC_MatrixEnable);
EXPORT_SYMBOL(MDrv_PQ_GetTableIndex_Ex);
EXPORT_SYMBOL(MDrv_PQ_LoadTable_Ex);
EXPORT_SYMBOL(MDrv_PQ_Demo_CloneWindow);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_LvlNum_Ex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_LevelIndex_Ex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_IPIndex_Ex);
EXPORT_SYMBOL(MDrv_PQ_GetGRule_TableIndex_Ex);
EXPORT_SYMBOL(MDrv_PQ_Up_Layer_Set_Config);
EXPORT_SYMBOL(MDrv_PQ_EnableScalerGamma);
EXPORT_SYMBOL(MDrv_PQ_SetGammaTbl);
EXPORT_SYMBOL(MDrv_PQ_Ex_GetVersion);
EXPORT_SYMBOL(MDrv_PQ_GetAdaptiveVersion);
EXPORT_SYMBOL(MDrv_PQ_LoadUFSCSettings);
EXPORT_SYMBOL(MDrv_PQ_Get_DNR_Whole_Reg);
EXPORT_SYMBOL(MDrv_PQ_SetP2pForceToDoCsc);
EXPORT_SYMBOL(MDrv_PQ_SetOutputColorFormat);
EXPORT_SYMBOL(MDrv_PQ_GetPQPathStatus);
EXPORT_SYMBOL(MDrv_PQ_SetPQBinPath);
EXPORT_SYMBOL(MDrv_PQ_Load_HDR_Table);
EXPORT_SYMBOL(MDrv_PQ_IsFieldOrderIncorrect);
EXPORT_SYMBOL(MDrv_PQ_LoadTMOModeGRuleTable);
#ifdef STELLAR
#endif
#ifdef __cplusplus
#endif
#endif
#ifdef _DRVPQ_CUS_H_
#ifdef _MDRV_PQ_CUS_C_
#else
#endif
#ifdef __cplusplus
#endif
#if !defined (__aarch64__)
#endif
#ifdef UFO_XC_PQ_SUPPORT_REPEATER_COLOR_THROUGH_MODE
EXPORT_SYMBOL(MDrv_PQ_SetColorThroughMode);
#endif
EXPORT_SYMBOL(MDrv_PQ_LoadFilmModeTable);
EXPORT_SYMBOL(MDrv_PQ_LoadDynamicContrastTable);
EXPORT_SYMBOL(MDrv_PQ_LoadNRTable);
EXPORT_SYMBOL(MDrv_PQ_LoadMPEGNRTable);
EXPORT_SYMBOL(MDrv_PQ_Load_ULTRACLEAR_Table);
EXPORT_SYMBOL(MDrv_PQ_Load_XVYCC_Table);
EXPORT_SYMBOL(MDrv_PQ_GRULE_Get_Support_Status);
EXPORT_SYMBOL(MDrv_PQ_SetMemFormat);
EXPORT_SYMBOL(MDrv_PQ_ReduceBW_ForOSD);
EXPORT_SYMBOL(MDrv_PQ_Init_DisplayType);
EXPORT_SYMBOL(MDrv_PQ_Set_DisplayType);
EXPORT_SYMBOL(MDrv_PQDS_Update_PanelID);
EXPORT_SYMBOL(MDrv_PQ_GetMADiFromOSDBWGrule);
EXPORT_SYMBOL(MDrv_PQ_GetMADiInGeneral);
EXPORT_SYMBOL(MDrv_PQ_LoadPTPTable);
EXPORT_SYMBOL(MDrv_PQ_Check_PointToPoint_Condition);
EXPORT_SYMBOL(MDrv_PQ_SetPostCCSOnOff);
EXPORT_SYMBOL(MDrv_PQ_AdaptiveTuning);
#ifdef UFO_XC_PQ_SUPPORT_SWDRIVING_MULTI_DEVICES
EXPORT_SYMBOL(MDrv_PQ_EX_AdaptiveTuning);
#endif
EXPORT_SYMBOL(MDrv_PQ_AdaptiveTuningByClient);
EXPORT_SYMBOL(MDrv_PQ_FilmMode_AnyCandence_Enable);
EXPORT_SYMBOL(MDrv_BW_LoadTableByContext);
EXPORT_SYMBOL(MDrv_BW_LoadInitTable);
EXPORT_SYMBOL(MDrv_PQ_SetH264_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetG3D_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetMVC4kx1k_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetNetworkMM_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetMM_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetRmvb_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetVIPBypass_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetPeaking_OnOff);
EXPORT_SYMBOL(MDrv_PQ_SetFakeOutEnable);
EXPORT_SYMBOL(MDrv_BW_resetToDefault);
EXPORT_SYMBOL(MDrv_BW_CusMode_LoadTable);
#ifdef __cplusplus
#endif
#endif
EXPORT_SYMBOL(mdrv_gpio_init);
EXPORT_SYMBOL(g_IPanel);
EXPORT_SYMBOL(g_IPanelEx);
EXPORT_SYMBOL(_XC_RIU_BASE);
EXPORT_SYMBOL(g_ACPMissingCounter);
EXPORT_SYMBOL(g_AVMUTEMissingCounter);
EXPORT_SYMBOL(g_AVIMissingCounter);
EXPORT_SYMBOL(g_HdmiPollingStatus);
