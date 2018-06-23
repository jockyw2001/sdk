#ifndef TEE_CLIENT_API_GENERAL_H
#define TEE_CLIENT_API_GENERAL_H

#include "tee_client_api.h"

typedef struct {
	char devname[256];
	union {
		struct tee_context *ctx;
		int fd;
	};
} TEEC_Context;

typedef struct {
	uint32_t session_id;
	union {
		int fd;
		struct tee_session *session;
	};
	TEEC_Context_2 ctx;
} TEEC_Session;

typedef struct {
	void *buffer;
	size_t size;
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
	} d;
	uint64_t registered;

	/* 2.x */
	int id;
	size_t alloced_size;
	void *shadow_buffer;
	int registered_fd;
} TEEC_SharedMemory;

typedef struct {
	void *buffer;
	size_t size;
} TEEC_TempMemoryReference;

typedef struct {
	TEEC_SharedMemory *parent;
	size_t size;
	size_t offset;
} TEEC_RegisteredMemoryReference;

typedef struct {
	uint32_t a;
	uint32_t b;
} TEEC_Value;

typedef union {
	TEEC_TempMemoryReference tmpref;
	TEEC_RegisteredMemoryReference memref;
	TEEC_Value value;
} TEEC_Parameter;

typedef struct {
	uint32_t started;
	uint32_t paramTypes;
	union{
		TEEC_Parameter params[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		TEEC_Parameter_1 params_1[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		TEEC_Parameter_2 params_2[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	};

	TEEC_Session *session;

	/* 1.x */
	TEEC_SharedMemory memRefs[TEEC_CONFIG_PAYLOAD_REF_COUNT];
	uint64_t flags;
} TEEC_Operation;

typedef enum{
	GENERAL_TO_1_0_1 = 0,
	GENERAL_FROM_1_0_1 = 1,
	GENERAL_TO_2_x = 2,
	GENERAL_FROM_2_x = 3,
}DIRECTION;

TEEC_Result TEEC_Operation_general_to_1_x(TEEC_Operation *general, size_t general_size,TEEC_Operation_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Operation_general_to_2_x(TEEC_Operation *general, size_t general_size, TEEC_Operation_2 *data_2, size_t data_2_size, DIRECTION from);
TEEC_Result TEEC_Context_general_to_1_x(TEEC_Context *general, size_t general_size, TEEC_Context_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Context_general_to_2_x(TEEC_Context *general, size_t general_size, TEEC_Context_2 *data_2, size_t data_2_size, DIRECTION from);
TEEC_Result TEEC_Session_general_to_1_x(TEEC_Session *general, size_t general_size, TEEC_Session_1 *data_1, size_t data_1_size, DIRECTION from);
TEEC_Result TEEC_Session_general_to_2_x(TEEC_Session *general, size_t general_size, TEEC_Session_2 *data_2, size_t data_2_size, DIRECTION from);

#define TEEC_Operation(_ver, general, data, from) TEEC_Operation_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);
#define TEEC_Context(_ver, general, data, from) TEEC_Context_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);
#define TEEC_Session(_ver, general, data, from) TEEC_Session_general_to_##_ver(general, sizeof(*general), data, sizeof(*data), from);

#define TEEC_Session_INIT { .fd = -1, .session_id = -1, .ctx = {.fd = -1}}
#define TEEC_Context_INIT { .fd = -1, .devname = ""}
#define TEEC_Operation_INIT { .started = 0, .paramTypes = 0, .session = NULL, .flags = 0}

#endif
