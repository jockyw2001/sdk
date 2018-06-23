/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TEE_CLIENT_API_1_0_1_H
#define TEE_CLIENT_API_1_0_1_H

#ifndef __KERNEL__
#include <stdint.h>
#include <stddef.h>
#endif /* __KERNEL__ */

/*
 * Defines the number of available memory references in an open session or
 * invoke command operation payload.
 */
#define TEEC_CONFIG_PAYLOAD_REF_COUNT 4

/**
 * Defines the maximum size of a single shared memory block, in bytes, of both
 * API allocated and API registered memory. The size is currently set to
 * 512 * kB (512 * 1024).
 */
#define TEEC_CONFIG_SHAREDMEM_MAX_SIZE 0x8000

/**
 * Flag constants indicating the type of parameters encoded inside the
 * operation payload (TEEC_Operation_1), Type is uint32_t.
 *
 * TEEC_NONE                   The Parameter is not used
 *
 * TEEC_VALUE_INPUT            The Parameter is a TEEC_Value_1 tagged as input.
 *
 * TEEC_VALUE_OUTPUT           The Parameter is a TEEC_Value_1 tagged as output.
 *
 * TEEC_VALUE_INOUT            The Parameter is a TEEC_Value_1 tagged as both as
 *                             input and output, i.e., for which both the
 *                             behaviors of TEEC_VALUE_INPUT and
 *                             TEEC_VALUE_OUTPUT apply.
 *
 * TEEC_MEMREF_TEMP_INPUT      The Parameter is a TEEC_TempMemoryReference_1
 *                             describing a region of memory which needs to be
 *                             temporarily registered for the duration of the
 *                             Operation and is tagged as input.
 *
 * TEEC_MEMREF_TEMP_OUTPUT     Same as TEEC_MEMREF_TEMP_INPUT, but the Memory
 *                             Reference is tagged as output. The
 *                             Implementation may update the size field to
 *                             reflect the required output size in some use
 *                             cases.
 *
 * TEEC_MEMREF_TEMP_INOUT      A Temporary Memory Reference tagged as both
 *                             input and output, i.e., for which both the
 *                             behaviors of TEEC_MEMREF_TEMP_INPUT and
 *                             TEEC_MEMREF_TEMP_OUTPUT apply.
 *
 * TEEC_MEMREF_WHOLE           The Parameter is a Registered Memory Reference
 *                             that refers to the entirety of its parent Shared
 *                             Memory block. The parameter structure is a
 *                             TEEC_MemoryReference. In this structure, the
 *                             Implementation MUST read only the parent field
 *                             and MAY update the size field when the operation
 *                             completes.
 *
 * TEEC_MEMREF_PARTIAL_INPUT   A Registered Memory Reference structure that
 *                             refers to a partial region of its parent Shared
 *                             Memory block and is tagged as input.
 *
 * TEEC_MEMREF_PARTIAL_OUTPUT  Registered Memory Reference structure that
 *                             refers to a partial region of its parent Shared
 *                             Memory block and is tagged as output.
 *
 * TEEC_MEMREF_PARTIAL_INOUT   The Registered Memory Reference structure that
 *                             refers to a partial region of its parent Shared
 *                             Memory block and is tagged as both input and
 *                             output, i.e., for which both the behaviors of
 *                             TEEC_MEMREF_PARTIAL_INPUT and
 *                             TEEC_MEMREF_PARTIAL_OUTPUT apply.
 */
#define TEEC_NONE                   0x00000000
#define TEEC_VALUE_INPUT            0x00000001
#define TEEC_VALUE_OUTPUT           0x00000002
#define TEEC_VALUE_INOUT            0x00000003
#define TEEC_MEMREF_TEMP_INPUT      0x00000005
#define TEEC_MEMREF_TEMP_OUTPUT     0x00000006
#define TEEC_MEMREF_TEMP_INOUT      0x00000007
#define TEEC_MEMREF_WHOLE           0x0000000C
#define TEEC_MEMREF_PARTIAL_INPUT   0x0000000D
#define TEEC_MEMREF_PARTIAL_OUTPUT  0x0000000E
#define TEEC_MEMREF_PARTIAL_INOUT   0x0000000F

/**
 * Flag constants indicating the data transfer direction of memory in
 * TEEC_Parameter_1. TEEC_MEM_INPUT signifies data transfer direction from the
 * client application to the TEE. TEEC_MEM_OUTPUT signifies data transfer
 * direction from the TEE to the client application. Type is uint32_t.
 *
 * TEEC_MEM_INPUT   The Shared Memory can carry data from the client
 *                  application to the Trusted Application.
 * TEEC_MEM_OUTPUT  The Shared Memory can carry data from the Trusted
 *                  Application to the client application.
 * TEEC_MEM_DMABUF  The Shared Memory is allocated with the dma buf api and
 *                  not necessarily user mapped. The handle will be then the
 *                  fd instead of the buffer
 * TEEC_MEM_KAPI    Shared memory is required from another Linux module.
 *                  Dma buf file descriptor is not created.
 */
#define TEEC_MEM_INPUT   0x00000001
#define TEEC_MEM_OUTPUT  0x00000002
#define TEEC_MEM_DMABUF  0x00010000
#define TEEC_MEM_KAPI    0x00020000

/**
 * Return values. Type is TEEC_Result
 *
 * TEEC_SUCCESS                 The operation was successful.
 * TEEC_ERROR_GENERIC           Non-specific cause.
 * TEEC_ERROR_ACCESS_DENIED     Access privileges are not sufficient.
 * TEEC_ERROR_CANCEL            The operation was canceled.
 * TEEC_ERROR_ACCESS_CONFLICT   Concurrent accesses caused conflict.
 * TEEC_ERROR_EXCESS_DATA       Too much data for the requested operation was
 *                              passed.
 * TEEC_ERROR_BAD_FORMAT        Input data was of invalid format.
 * TEEC_ERROR_BAD_PARAMETERS    Input parameters were invalid.
 * TEEC_ERROR_BAD_STATE         Operation is not valid in the current state.
 * TEEC_ERROR_ITEM_NOT_FOUND    The requested data item is not found.
 * TEEC_ERROR_NOT_IMPLEMENTED   The requested operation should exist but is not
 *                              yet implemented.
 * TEEC_ERROR_NOT_SUPPORTED     The requested operation is valid but is not
 *                              supported in this implementation.
 * TEEC_ERROR_NO_DATA           Expected data was missing.
 * TEEC_ERROR_OUT_OF_MEMORY     System ran out of resources.
 * TEEC_ERROR_BUSY              The system is busy working on something else.
 * TEEC_ERROR_COMMUNICATION     Communication with a remote party failed.
 * TEEC_ERROR_SECURITY          A security fault was detected.
 * TEEC_ERROR_SHORT_BUFFER      The supplied buffer is too short for the
 *                              generated output.
 * TEEC_ERROR_TARGET_DEAD       Trusted Application has panicked
 *                              during the operation.
 */

/**
 *  Standard defined error codes.
 */
#define TEEC_SUCCESS                0x00000000
#define TEEC_ERROR_GENERIC          0xFFFF0000
#define TEEC_ERROR_ACCESS_DENIED    0xFFFF0001
#define TEEC_ERROR_CANCEL           0xFFFF0002
#define TEEC_ERROR_ACCESS_CONFLICT  0xFFFF0003
#define TEEC_ERROR_EXCESS_DATA      0xFFFF0004
#define TEEC_ERROR_BAD_FORMAT       0xFFFF0005
#define TEEC_ERROR_BAD_PARAMETERS   0xFFFF0006
#define TEEC_ERROR_BAD_STATE        0xFFFF0007
#define TEEC_ERROR_ITEM_NOT_FOUND   0xFFFF0008
#define TEEC_ERROR_NOT_IMPLEMENTED  0xFFFF0009
#define TEEC_ERROR_NOT_SUPPORTED    0xFFFF000A
#define TEEC_ERROR_NO_DATA          0xFFFF000B
#define TEEC_ERROR_OUT_OF_MEMORY    0xFFFF000C
#define TEEC_ERROR_BUSY             0xFFFF000D
#define TEEC_ERROR_COMMUNICATION    0xFFFF000E
#define TEEC_ERROR_SECURITY         0xFFFF000F
#define TEEC_ERROR_SHORT_BUFFER     0xFFFF0010
#define TEEC_ERROR_EXTERNAL_CANCEL  0xFFFF0011
#define TEEC_ERROR_TARGET_DEAD      0xFFFF3024

/**
 * Function error origins, of type TEEC_ErrorOrigin. These indicate where in
 * the software stack a particular return value originates from.
 *
 * TEEC_ORIGIN_API          The error originated within the TEE Client API
 *                          implementation.
 * TEEC_ORIGIN_COMMS        The error originated within the underlying
 *                          communications stack linking the rich OS with
 *                          the TEE.
 * TEEC_ORIGIN_TEE          The error originated within the common TEE code.
 * TEEC_ORIGIN_TRUSTED_APP  The error originated within the Trusted Application
 *                          code.
 */
#define TEEC_ORIGIN_API          0x00000001
#define TEEC_ORIGIN_COMMS        0x00000002
#define TEEC_ORIGIN_TEE          0x00000003
#define TEEC_ORIGIN_TRUSTED_APP  0x00000004

/**
 * Session login methods, for use in TEEC_OpenSession_1() as parameter
 * connectionMethod. Type is uint32_t.
 *
 * TEEC_LOGIN_PUBLIC       No login data is provided.
 * TEEC_LOGIN_USER         Login data about the user running the Client
 *                         Application process is provided.
 * TEEC_LOGIN_GROUP        Login data about the group running the Client
 *                         Application process is provided.
 * TEEC_LOGIN_APPLICATION  Login data about the running Client Application
 *                         itself is provided.
 */
#define TEEC_LOGIN_PUBLIC       0x00000000
#define TEEC_LOGIN_USER         0x00000001
#define TEEC_LOGIN_GROUP        0x00000002
#define TEEC_LOGIN_APPLICATION  0x00000004

/**
 * Encode the paramTypes according to the supplied types.
 *
 * @param p0 The first param type.
 * @param p1 The second param type.
 * @param p2 The third param type.
 * @param p3 The fourth param type.
 */
#define TEEC_PARAM_TYPES(p0, p1, p2, p3) \
	((p0) | ((p1) << 4) | ((p2) << 8) | ((p3) << 12))

/**
 * Get the i_th param type from the paramType.
 *
 * @param p The paramType.
 * @param i The i-th parameter to get the type for.
 */
#define TEEC_PARAM_TYPE_GET(p, i) (((p) >> (i * 4)) & 0xF)

#define TEEC_MAX_DEVNAME_SIZE 256
//typedef uint32_t TEEC_Result;

/**
 * struct TEEC_Context_1 - Represents a connection between a client application
 * and a TEE.
 *
 * Context identifier can be a handle (when opened from user land)
 * or a structure pointer (when opened from kernel land).
 * Identifier is defined as an union to match type sizes on all architectures.
 */
typedef struct {
	char devname[256];
	union {
		struct tee_context *ctx;
		int fd;
	};
} TEEC_Context_1;

/**
 * This type contains a Universally Unique Resource Identifier (UUID) type as
 * defined in RFC4122. These UUID values are used to identify Trusted
 * Applications.
 */
typedef struct {
	uint32_t timeLow;
	uint16_t timeMid;
	uint16_t timeHiAndVersion;
	uint8_t clockSeqAndNode[8];
} TEEC_UUID;

/**
 * In terms of compatible kernel, the data struct shared by client application
 * and TEE driver should be restructrued in "compatible" rules. To keep GP's
 * standard in compatibility mode, the anonymous padding members are filled
 * in the struct definition below.
 */


/**
 * struct TEEC_SharedMemory_1 - Memory to transfer data between a client
 * application and trusted code.
 *
 * @param buffer      The memory buffer which is to be, or has been, shared
 *                    with the TEE.
 * @param size        The size, in bytes, of the memory buffer.
 * @param flags       Bit-vector which holds properties of buffer.
 *                    The bit-vector can contain either or both of the
 *                    TEEC_MEM_INPUT and TEEC_MEM_OUTPUT flags.
 *
 * A shared memory block is a region of memory allocated in the context of the
 * client application memory space that can be used to transfer data between
 * that client application and a trusted application. The user of this struct
 * is responsible to populate the buffer pointer.
 */
typedef struct {
	union {
		void *buffer;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
	uint32_t flags;
	/*
	 * Implementation-Defined, must match what the kernel driver have
	 *
	 * Identifier can store a handle (int) or a structure pointer (void *).
	 * Define this union to match case where sizeof(int)!=sizeof(void *).
	 */
	uint32_t reserved;
	union {
		int fd;
		void *ptr;
		uint64_t padding_d;
	} d;
	uint64_t registered;
} TEEC_SharedMemory_1;

/**
 * struct TEEC_TempMemoryReference_1 - Temporary memory to transfer data between
 * a client application and trusted code, only used for the duration of the
 * operation.
 *
 * @param buffer  The memory buffer which is to be, or has been shared with
 *                the TEE.
 * @param size    The size, in bytes, of the memory buffer.
 *
 * A memory buffer that is registered temporarily for the duration of the
 * operation to be called.
 */
typedef struct {
	union {
		void *buffer;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
} TEEC_TempMemoryReference_1;

/**
 * struct TEEC_RegisteredMemoryReference_1 - use a pre-registered or
 * pre-allocated shared memory block of memory to transfer data between
 * a client application and trusted code.
 *
 * @param parent  Points to a shared memory structure. The memory reference
 *                may utilize the whole shared memory or only a part of it.
 *                Must not be NULL
 *
 * @param size    The size, in bytes, of the memory buffer.
 *
 * @param offset  The offset, in bytes, of the referenced memory region from
 *                the start of the shared memory block.
 *
 */
typedef struct {
	union {
		TEEC_SharedMemory_1 *parent;
		uint64_t padding_ptr;
	};
	union {
		size_t size;
		uint64_t padding_sz;
	};
	union {
		size_t offset;
		uint64_t padding_off;
	};
} TEEC_RegisteredMemoryReference_1;

/**
 * struct TEEC_Value_1 - Small raw data container
 *
 * Instead of allocating a shared memory buffer this structure can be used
 * to pass small raw data between a client application and trusted code.
 *
 * @param a  The first integer value.
 *
 * @param b  The second second value.
 */
typedef struct {
	uint32_t a;
	uint32_t b;
} TEEC_Value_1;

/**
 * union TEEC_Parameter_1 - Memory container to be used when passing data between
 *                        client application and trusted code.
 *
 * Either the client uses a shared memory reference, parts of it or a small raw
 * data container.
 *
 * @param tmpref  A temporary memory reference only valid for the duration
 *                of the operation.
 *
 * @param memref  The entire shared memory or parts of it.
 *
 * @param value   The small raw data container to use
 */
typedef union {
	TEEC_TempMemoryReference_1 tmpref;
	TEEC_RegisteredMemoryReference_1 memref;
	TEEC_Value_1 value;
} TEEC_Parameter_1;

/**
 * struct TEEC_Session_1 - Represents a connection between a client application
 * and a trusted application.
 *
 * Session identifier can be a handle (when opened from user land) or a
 * structure pointer (when opened from kernel land).
 * Identifier is defined as an union to match type sizes on all architectures.
 */
typedef union {
	int fd;
	struct tee_session *session;
} TEEC_Session_1;

/**
 * struct TEEC_Operation_1 - Holds information and memory references used in
 * TEEC_InvokeCommand_1().
 *
 * @param   started     Client must initialize to zero if it needs to cancel
 *                      an operation about to be performed.
 * @param   paramTypes  Type of data passed. Use TEEC_PARAMS_TYPE macro to
 *                      create the correct flags.
 *                      0 means TEEC_NONE is passed for all params.
 * @param   params      Array of parameters of type TEEC_Parameter_1.
 * @param   session     Internal pointer to the last session used by
 *                      TEEC_InvokeCommand_1 with this operation.
 *
 */
typedef struct {
	uint32_t started;
	uint32_t paramTypes;
	TEEC_Parameter_1 params[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	/* Implementation-Defined */
	union {
		TEEC_Session_1 *session;
		uint64_t padding_ptr;
	};
	TEEC_SharedMemory_1 memRefs[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	uint64_t flags;
} TEEC_Operation_1;

#endif
