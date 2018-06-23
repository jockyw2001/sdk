////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2017 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_wrapper.h
/// @brief     Cam OS Wrapper Header File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_OS_WRAPPER_H__
#define __CAM_OS_WRAPPER_H__

#if !defined(__KERNEL__) && !defined(_LINUX_TYPES_H)
typedef unsigned char       bool;
#endif
typedef unsigned char       u8;
typedef signed   char       s8;
typedef unsigned short      u16;
typedef signed   short      s16;
typedef unsigned int        u32;
typedef signed   int        s32;
typedef unsigned long long  u64;
typedef signed   long long  s64;

typedef enum
{
    CAM_OS_OK               = 0,
    CAM_OS_FAIL             = -1,
    CAM_OS_PARAM_ERR        = -2,
    CAM_OS_ALLOCMEM_FAIL    = -3,
    CAM_OS_TIMEOUT          = -4,
} CamOsRet_e;

typedef enum
{
    CAM_OS_MEM_1MB      = 0,
    CAM_OS_MEM_2MB      = 1,
    CAM_OS_MEM_4MB      = 2,
    CAM_OS_MEM_8MB      = 3,
    CAM_OS_MEM_16MB     = 4,
    CAM_OS_MEM_32MB     = 5,
    CAM_OS_MEM_64MB     = 6,
    CAM_OS_MEM_128MB    = 7,
    CAM_OS_MEM_256MB    = 8,
    CAM_OS_MEM_512MB    = 9,
    CAM_OS_MEM_1024MB   = 10,
    CAM_OS_MEM_UNKNOWN  = 99,
} CamOsMemSize_e;

typedef struct
{
    u8 nPriv[44];
} CamOsMutex_t;

typedef struct
{
    u8 nPriv[80];
} CamOsTsem_t;

typedef struct
{
    u32 nSec;
    u32 nNanoSec;
} CamOsTimespec_t;

typedef struct
{
    u32 nPriority;      /* From 1(lowest) to 99(highest), use OS default priority if set 0 */
    u32 nStackSize;     /* If nStackSize is zero, use OS default value */
} CamOsThreadAttrb_t, *pCamOsThreadAttrb;

typedef struct
{
    volatile s32 nCounter;
} CamOsAtomic_t;

typedef void * CamOsThread;

//=============================================================================
// Description:
//      Writes the C string pointed by format to the standard output.
// Parameters:
//      [in]  szFmt: C string that contains the text to be written, it can
//                   optionally contain embedded format specifiers.
// Return:
//      N/A
//=============================================================================
void CamOsPrintf(const char *szFmt, ...);

//=============================================================================
// Description:
//      Reads data from stdin and stores them according to the parameter format
//      into the locations pointed by the additional arguments.
// Parameters:
//      [in]  szFmt: C string that contains the text to be parsing, it can
//                   optionally contain embedded format specifiers.
// Return:
//      The number of items of the argument list successfully filled.
//=============================================================================
s32 CamOsScanf(const char *szFmt, ...);

//=============================================================================
// Description:
//      Returns the next character from the standard input.
// Parameters:
//      N/A
// Return:
//      the character read is returned.
//=============================================================================
s32 CamOsGetChar(void);

//=============================================================================
// Description:
//      Suspend execution for millisecond intervals.
// Parameters:
//      [in]  nMsec: Millisecond to suspend.
// Return:
//      N/A
//=============================================================================
void CamOsMsSleep(u32 nMsec);

//=============================================================================
// Description:
//      Get the number of seconds and nanoseconds since the Epoch.
// Parameters:
//      [out] ptRes: A pointer to a CamOsTimespec_t structure where
//                   CamOsGetTimeOfDay() can store the time.
// Return:
//      N/A
//=============================================================================
void CamOsGetTimeOfDay(CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      Set the number of seconds and nanoseconds since the Epoch.
// Parameters:
//      [in] ptRes: A pointer to a CamOsTimespec_t structure.
// Return:
//      N/A
//=============================================================================
void CamOsSetTimeOfDay(const CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      Gets the current time of the clock specified, and puts it into the
//      buffer pointed to by ptRes.
// Parameters:
//      [out] ptRes: A pointer to a CamOsTimespec_t structure where
//                   CamOsGetMonotonicTime() can store the time.
// Return:
//      N/A
//=============================================================================
void CamOsGetMonotonicTime(CamOsTimespec_t *ptRes);

//=============================================================================
// Description:
//      The CamOsThreadCreate() function is used to create a new thread/task,
//      with attributes specified by ptAttrb. If ptAttrb is NULL, the default
//      attributes are used.
// Parameters:
//      [out] pThread: A successful call to CamOsThreadCreate() stores the handle
//                     of the new thread in the buffer pointed to by pThread.
//      [in]  ptAttrb: Argument points to a CamOsThreadAttrb_t structure whose
//                     contents are used at thread creation time to determine
//                     thread priority, stack size and thread name. Thread
//                     priority range from 1(lowest) to 99(highest), use OS
//                     default priority if set 0.
//      [in]  pfnStartRoutine(): The new thread starts execution by invoking it.
//      [in]  pArg: It is passed as the sole argument of pfnStartRoutine().
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadCreate(CamOsThread *pThread,
                             CamOsThreadAttrb_t *ptAttrb,
                             void *(*pfnStartRoutine)(void *),
                             void *pArg);

//=============================================================================
// Description:
//      Waits for the thread specified by pThread to terminate. If that thread
//      has already terminated, then CamOsThreadJoin() returns immediately. This
//      function is not applicable to Linux kernel space.
// Parameters:
//      [in]  thread: Handle of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadJoin(CamOsThread thread);

//=============================================================================
// Description:
//      Stop a thread created by CamOsThreadCreate in Linux kernel space. This
//      function is not applicable to RTK and Linux user space.
// Parameters:
//      [in]  thread: Handle of target thread.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadStop(CamOsThread thread);

//=============================================================================
// Description:
//      When someone calls CamOsThreadStop, it will be woken and this will
//      return true. You should then return from the thread. This function is
//      not applicable to RTK and Linux user space.
// Parameters:
//      N/A
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadShouldStop(void);

//=============================================================================
// Description:
//      Set the name of a thread. The thread name is a meaningful C language
//      string, whose length is restricted to 16 characters, including the
//      terminating null byte ('\0').
// Parameters:
//      [in]  thread: Handle of target thread.
//      [in]  szName: specifies the new name.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadSetName(CamOsThread thread, const char *szName);

//=============================================================================
// Description:
//      Get the name of a thread. The buffer specified by name should be at
//      least 16 characters in length.
// Parameters:
//      [in]  thread: Handle of target thread.
//      [out] szName: Buffer used to return the thread name.
//      [in]  nLen: Specifies the number of bytes available in szName
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsThreadGetName(CamOsThread thread, char *szName, u32 nLen);

//=============================================================================
// Description:
//      Get thread identification.
// Parameters:
//      N/A
// Return:
//      On success, returns the thread ID of the calling process.
//=============================================================================
u32 CamOsThreadGetID(void);

//=============================================================================
// Description:
//      Initializes the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to initialize.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexInit(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Destroys the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexDestroy(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Lock the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to lock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexLock(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Unlock the mutex.
// Parameters:
//      [in]  ptMutex: The mutex to unlock.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsMutexUnlock(CamOsMutex_t *ptMutex);

//=============================================================================
// Description:
//      Initializes the semaphore at a given value.
// Parameters:
//      [in]  ptTsem: The semaphore to initialize.
//      [in]  nVal: the initial value of the semaphore.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemInit(CamOsTsem_t *ptTsem, u32 nVal);

//=============================================================================
// Description:
//      Destroy the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to destroy.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemDeinit(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Increases the value of the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to increase.
// Return:
//      N/A
//=============================================================================
void CamOsTsemUp(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Decreases the value of the semaphore. Blocks if the semaphore value is
//      zero.
// Parameters:
//      [in]  ptTsem: The semaphore to decrease.
// Return:
//      N/A
//=============================================================================
void CamOsTsemDown(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Decreases the value of the semaphore. Blocks if the semaphore value is
//      zero.
// Parameters:
//      [in]  ptTsem: The semaphore to decrease
// Return:
//      If the timeout is reached the function exits with error CAM_OS_TIMEOUT.
//      CAM_OS_OK is returned if down successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemTimedDown(CamOsTsem_t *ptTsem, u32 nMsec);

//=============================================================================
// Description:
//      Signal the condition, if waiting.
// Parameters:
//      [in]  ptTsem: The semaphore to signal
// Return:
//      N/A
//=============================================================================
void CamOsTsemSignal(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Wait on the condition.
// Parameters:
//      [in]  ptTsem: The semaphore to wait.
// Return:
//      N/A
//=============================================================================
void CamOsTsemWait(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Wait on the condition.
// Parameters:
//      [in]  ptTsem: The semaphore to wait.
//      [in]  nMsec: The value of delay for the timeout.
// Return:
//      If the timeout is reached the function exits with error CAM_OS_TIMEOUT.
//      CAM_OS_OK is returned if wait successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsTsemTimedWait(CamOsTsem_t *ptTsem, u32 nMsec);

//=============================================================================
// Description:
//      Get the value of the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to reset.
// Return:
//      The value of the semaphore.
//=============================================================================
u32 CamOsTsemGetValue(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Reset the value of the semaphore.
// Parameters:
//      [in]  ptTsem: The semaphore to reset.
// Return:
//      N/A
//=============================================================================
void CamOsTsemReset(CamOsTsem_t *ptTsem);

//=============================================================================
// Description:
//      Allocates a block of nSize bytes of memory, returning a pointer to the
//      beginning of the block.
// Parameters:
//      [in]  nSize: Size of the memory block, in bytes.
// Return:
//      On success, a pointer to the memory block allocated by the function. If
//      failed to allocate, a null pointer is returned.
//=============================================================================
void* CamOsMemAlloc(u32 nSize);

//=============================================================================
// Description:
//      Allocates a block of memory for an array of nNum elements, each of them
//      nSize bytes long, and initializes all its bits to zero.
// Parameters:
//      [in]  nNum: Number of elements to allocate.
//      [in]  nSize: Size of each element.
// Return:
//      On success, a pointer to the memory block allocated by the function. If
//      failed to allocate, a null pointer is returned.
//=============================================================================
void* CamOsMemCalloc(u32 nNum, u32 nSize);

//=============================================================================
// Description:
//      Changes the size of the memory block pointed to by pPtr. The function
//      may move the memory block to a new location (whose address is returned
//      by the function).
// Parameters:
//      [in]  pPtr: Pointer to a memory block previously allocated with
//                  CamOsMemAlloc, CamOsMemCalloc or CamOsMemRealloc.
//      [in]  nSize: New size for the memory block, in bytes.
// Return:
//      A pointer to the reallocated memory block, which may be either the same
//      as pPtr or a new location.
//=============================================================================
void* CamOsMemRealloc(void* pPtr, u32 nSize);

//=============================================================================
// Description:
//      A block of memory previously allocated by a call to CamOsMemAlloc,
//      CamOsMemCalloc or CamOsMemRealloc is deallocated, making it available
//      again for further allocations. If pPtr is a null pointer, the function
//      does nothing.
// Parameters:
//      [in]  pPtr: Pointer to a memory block previously allocated with
//                  CamOsMemAlloc, CamOsMemCalloc or CamOsMemRealloc.
// Return:
//      N/A
//=============================================================================
void CamOsMemRelease(void* pPtr);

//=============================================================================
// Description:
//      Allocates a block of nSize bytes of direct memory (non-cached memory),
//      returning three pointer for different address domain to the beginning
//      of the block.
// Parameters:
//      [in]  szName: Name of the memory block, whose length is restricted to
//                    16 characters.
//      [in]  nSize: Size of the memory block, in bytes.
//      [out] ppVirtPtr: Virtual address pointer to the memory block.
//      [out] ppPhysPtr: Physical address pointer to the memory block.
//      [out] ppMiuPtr: Memory Interface Unit address pointer to the memory block.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemAlloc(const char* szName,
                               u32 nSize,
                               void** ppVirtPtr,
                               void** ppPhysPtr,
                               void** ppMiuPtr);

//=============================================================================
// Description:
//      A block of memory previously allocated by a call to CamOsDirectMemAlloc,
//      is deallocated, making it available again for further allocations.
// Parameters:
//      [in]  pVirtPtr: Virtual address pointer to a memory block previously
//                      allocated with CamOsDirectMemAlloc.
//      [in]  nSize: Size of the memory block, in bytes.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemRelease(void* pVirtPtr, u32 nSize);

//=============================================================================
// Description:
//      Flush chche of a block of memory previously allocated by a call to
//      CamOsDirectMemAlloc.
// Parameters:
//      [in]  pVirtPtr: Virtual address pointer to a memory block previously
//                      allocated with CamOsDirectMemAlloc.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemFlush(void* pVirtPtr);

//=============================================================================
// Description:
//      Print all allocated direct memory information to the standard output.
// Parameters:
//      N/A
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsDirectMemStat(void);

//=============================================================================
// Description:
//      Transfer Physical address to MIU address.
// Parameters:
//      [in]  pPtr: Physical address.
// Return:
//      MIU address.
//=============================================================================
void* CamOsDirectMemPhysToMiu(void* pPtr);

//=============================================================================
// Description:
//      Transfer MIU address to Physical address.
// Parameters:
//      [in]  pPtr: MIU address.
// Return:
//      Physical address.
//=============================================================================
void* CamOsDirectMemMiuToPhys(void* pPtr);

//=============================================================================
// Description:
//      Transfer Physical address to Virtual address.
// Parameters:
//      [in]  pPtr: Physical address.
// Return:
//      Virtual address.
//=============================================================================
void* CamOsDirectMemPhysToVirt(void* pPtr);

//=============================================================================
// Description:
//      Transfer Virtual address to Physical address.
// Parameters:
//      [in]  pPtr: Virtual address.
// Return:
//      Physical address.
//=============================================================================
void* CamOsDirectMemVirtToPhys(void* pPtr);

//=============================================================================
// Description:
//      Set property value by property name.
// Parameters:
//      [in]  szKey: Name of property.
//      [in]  szValue: Value if property.
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsPropertySet(const char *szKey, const char *szValue);

//=============================================================================
// Description:
//      Get property value by property name.
// Parameters:
//      [in]  szKey: Name of property.
//      [out] szValue: Value if property.
//      [in]  szDefaultValue: If the property read fails or returns an empty
//                            value, the default value is used
// Return:
//      CAM_OS_OK is returned if successful; otherwise, returns CamOsRet_e.
//=============================================================================
CamOsRet_e CamOsPropertyGet(const char *szkey, char *szValue, const char *szDefaultValue);

//=============================================================================
// Description:
//      Unsigned 64bit divide with Unsigned 64bit divisor with remainder.
// Parameters:
//      [in]  nDividend: Dividend.
//      [in]  nDivisor: Divisor.
//      [out]  pRemainder: Pointer to the remainder.
// Return:
//      Quotient of division.
//=============================================================================
u64 CamOsMathDivU64(u64 nDividend, u64 nDivisor, u64 *pRemainder);

//=============================================================================
// Description:
//      Signed 64bit divide with signed 64bit divisor with remainder.
// Parameters:
//      [in]  nDividend: Dividend.
//      [in]  nDivisor: Divisor.
//      [out]  pRemainder: Pointer to the remainder.
// Return:
//      Quotient of division.
//=============================================================================
s64 CamOsMathDivS64(s64 nDividend, s64 nDivisor, s64 *pRemainder);

//=============================================================================
// Description:
//      Copy a block of data from user space in Linux kernel space, it just
//      memory copy in RTOS.
// Parameters:
//      [in]  pTo: Destination address, in kernel space.
//      [in]  pFrom: Source address, in user space.
//      [in]  nLen: Number of bytes to copy.
// Return:
//      Number of bytes that could not be copied. On success, this will be zero.
//=============================================================================
u32 CamOsCopyFromUpperLayer(void *pTo, const void *pFrom, u32 nLen);

//=============================================================================
// Description:
//      Copy a block of data into user space in Linux kernel space, it just
//      memory copy in RTOS.
// Parameters:
//      [in]  pTo: Destination address, in user space.
//      [in]  pFrom: Source address, in kernel space.
//      [in]  nLen: Number of bytes to copy.
// Return:
//      Number of bytes that could not be copied. On success, this will be zero.
//=============================================================================
u32 CamOsCopyToUpperLayer(void *pTo, const void * pFrom, u32 nLen);

//=============================================================================
// Description:
//      Read atomic variable.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      the value of ptAtomic.
//=============================================================================
s32 CamOsAtomicRead(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Set atomic variable.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
//      [in]  nValue: required value.
// Return:
//      N/A
//=============================================================================
void CamOsAtomicSet(CamOsAtomic_t *ptAtomic, s32 nValue);

//=============================================================================
// Description:
//      Add to the atomic variable and return value.
// Parameters:
//      [in]  nValue: integer value to add.
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      the value of ptAtomic.
//=============================================================================
s32 CamOsAtomicAddReturn(s32 nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Subtract the atomic variable and return value.
// Parameters:
//      [in]  nValue: integer value to subtract.
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      the value of ptAtomic.
//=============================================================================
s32 CamOsAtomicSubReturn(s32 nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Subtract value from variable and test result.
// Parameters:
//      [in]  nValue: integer value to subtract.
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      returns true if the result is zero, or false for all other cases.
//=============================================================================
s32 CamOsAtomicSubAndTest(s32 nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Increment atomic variable and return value.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      the value of ptAtomic.
//=============================================================================
s32 CamOsAtomicIncReturn(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      decrement atomic variable and return value.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      the value of ptAtomic.
//=============================================================================
s32 CamOsAtomicDecReturn(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Increment and test result.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      returns true if the result is zero, or false for all other cases.
//=============================================================================
s32 CamOsAtomicIncAndTest(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Decrement and test result.
// Parameters:
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      returns true if the result is zero, or false for all other cases.
//=============================================================================
s32 CamOsAtomicDecAndTest(CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Add to the atomic variable and test if negative.
// Parameters:
//      [in]  nValue: integer value to subtract.
//      [in]  ptAtomic: pointer of type CamOsAtomic_t.
// Return:
//      returns true if the result is negative, or false when result is greater
//      than or equal to zero.
//=============================================================================
s32 CamOsAtomicAddNegative(s32 nValue, CamOsAtomic_t *ptAtomic);

//=============================================================================
// Description:
//      Get physical memory size of system.
// Parameters:
//      N/A
// Return:
//      Enumeration of memory size.
//=============================================================================
CamOsMemSize_e CamOsPhysMemSize(void);

//=============================================================================
// Description:
//      Get Chip ID.
// Parameters:
//      N/A
// Return:
//      Chip ID.
//=============================================================================
u32 CamOsChipId(void);

#endif /* __CAM_OS_WRAPPER_H__ */
