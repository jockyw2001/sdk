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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    drvSYS.c
/// @brief  System Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#if defined (MSOS_TYPE_LINUX)
#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif

#define TAG_SYS "SYS"

#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <string.h>

#include "MsCommon.h"
#include "MsVersion.h"
#include "ULog.h"
#include "tee_client_api.h"

#define TEEC_MAX_DEV_SEQ	1

static pthread_mutex_t teec_mutex = PTHREAD_MUTEX_INITIALIZER;

static void teec_mutex_lock(pthread_mutex_t *mu)
{
	int e = pthread_mutex_lock(mu);

	if (e != 0)
		OPTEE_ERROR("pthread_mutex_lock failed: %d\n", e);
}

static void teec_mutex_unlock(pthread_mutex_t *mu)
{
	int e = pthread_mutex_unlock(mu);

	if (e != 0)
		OPTEE_ERROR("pthread_mutex_unlock failed: %d\n", e);
}

static int teec_open_dev(const char *devname, const char *capabilities)
{
	struct tee_ioctl_version_data vers;
	int fd;

	fd = open(devname, O_RDWR);
	if (fd < 0)
		return -1;

	if (ioctl(fd, TEE_IOC_VERSION, &vers)) {
		OPTEE_ERROR("TEE_IOC_VERSION failed");
		goto err;
	}

	/* We can only handle GP TEEs */
	if (!(vers.gen_caps & TEE_GEN_CAP_GP))
		goto err;

#if 0
	if (capabilities) {
		if (strcmp(capabilities, "optee-tz") == 0) {
			if (vers.impl_id != TEE_IMPL_ID_OPTEE)
				goto err;
			if (!(vers.impl_caps & TEE_OPTEE_CAP_TZ))
				goto err;
		} else {
			/* Unrecognized capability requested */
			goto err;
		}
	}
#endif

	return fd;
err:
	close(fd);
	return -1;
}

static int teec_shm_alloc(int fd, size_t size, int *id)
{
	int shm_fd;
	struct tee_ioctl_shm_alloc_data data;

	memset(&data, 0, sizeof(data));
	data.size = size;
	shm_fd = ioctl(fd, TEE_IOC_SHM_ALLOC, &data);
	if (shm_fd < 0)
		return -1;
	*id = data.id;
	return shm_fd;
}

static TEEC_Result teec_pre_process_tmpref(TEEC_Context_2 *ctx,
			uint32_t param_type, TEEC_TempMemoryReference_2 *tmpref,
			struct tee_ioctl_param *param,
			TEEC_SharedMemory_2 *shm)
{
	TEEC_Result res;

	switch (param_type) {
	case TEEC_MEMREF_TEMP_INPUT:
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
		shm->flags = TEEC_MEM_INPUT;
		break;
	case TEEC_MEMREF_TEMP_OUTPUT:
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
		shm->flags = TEEC_MEM_OUTPUT;
		break;
	case TEEC_MEMREF_TEMP_INOUT:
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
		shm->flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
		break;
	default:
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	shm->size = tmpref->size;

	res = TEEC_AllocateSharedMemory(ctx, shm);
	if (res != TEEC_SUCCESS)
		return res;

	memcpy(shm->buffer, tmpref->buffer, tmpref->size);
	param->u.memref.size = tmpref->size;
	param->u.memref.shm_id = shm->id;
	return TEEC_SUCCESS;
}

static TEEC_Result teec_pre_process_whole(
			TEEC_RegisteredMemoryReference_2 *memref,
			struct tee_ioctl_param *param)
{
	const uint32_t inout = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	uint32_t flags = memref->parent->flags & inout;
	TEEC_SharedMemory_2 *shm;

	if (flags == inout)
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
	else if (flags & TEEC_MEM_INPUT)
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	else if (flags & TEEC_MEM_OUTPUT)
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	else
		return TEEC_ERROR_BAD_PARAMETERS;

	shm = memref->parent;
	/*
	 * We're using a shadow buffer in this reference, copy the real buffer
	 * into the shadow buffer if needed. We'll copy it back once we've
	 * returned from the call to secure world.
	 */
	if (shm->shadow_buffer && (flags & TEEC_MEM_INPUT))
		memcpy(shm->shadow_buffer, shm->buffer, shm->size);

	param->u.memref.shm_id = shm->id;
	param->u.memref.size = shm->size;
	return TEEC_SUCCESS;
}

static TEEC_Result teec_pre_process_partial(uint32_t param_type,
			TEEC_RegisteredMemoryReference_2 *memref,
			struct tee_ioctl_param *param)
{
	uint32_t req_shm_flags;
	TEEC_SharedMemory_2 *shm;

	switch (param_type) {
	case TEEC_MEMREF_PARTIAL_INPUT:
		req_shm_flags = TEEC_MEM_INPUT;
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
		break;
	case TEEC_MEMREF_PARTIAL_OUTPUT:
		req_shm_flags = TEEC_MEM_OUTPUT;
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
		break;
	case TEEC_MEMREF_PARTIAL_INOUT:
		req_shm_flags = TEEC_MEM_OUTPUT | TEEC_MEM_INPUT;
		param->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
		break;
	default:
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	shm = memref->parent;

	if ((shm->flags & req_shm_flags) != req_shm_flags)
		return TEEC_ERROR_BAD_PARAMETERS;

	/*
	 * We're using a shadow buffer in this reference, copy the real buffer
	 * into the shadow buffer if needed. We'll copy it back once we've
	 * returned from the call to secure world.
	 */
	if (shm->shadow_buffer && param_type != TEEC_MEMREF_PARTIAL_OUTPUT)
		memcpy((char *)shm->shadow_buffer + memref->offset,
		       (char *)shm->buffer + memref->offset, memref->size);

	param->u.memref.shm_id = shm->id;
	param->u.memref.shm_offs = memref->offset;
	param->u.memref.size = memref->size;
	return TEEC_SUCCESS;
}

static TEEC_Result teec_pre_process_operation(TEEC_Context_2 *ctx,
			TEEC_Operation_2 *operation,
			struct tee_ioctl_param *params,
			TEEC_SharedMemory_2 *shms)
{
	TEEC_Result res;
	size_t n;

	memset(shms, 0, sizeof(TEEC_SharedMemory_2) *
			TEEC_CONFIG_PAYLOAD_REF_COUNT);
	if (!operation) {
		memset(params, 0, sizeof(struct tee_ioctl_param) *
				  TEEC_CONFIG_PAYLOAD_REF_COUNT);
		return TEEC_SUCCESS;
	}

	for (n = 0; n < TEEC_CONFIG_PAYLOAD_REF_COUNT; n++) {
		uint32_t param_type;

		param_type = TEEC_PARAM_TYPE_GET(operation->paramTypes, n);
		switch (param_type) {
		case TEEC_NONE:
			params[n].attr = param_type;
			break;
		case TEEC_VALUE_INPUT:
		case TEEC_VALUE_OUTPUT:
		case TEEC_VALUE_INOUT:
			params[n].attr = param_type;
			params[n].u.value.a = operation->params[n].value.a;
			params[n].u.value.b = operation->params[n].value.b;
			break;
		case TEEC_MEMREF_TEMP_INPUT:
		case TEEC_MEMREF_TEMP_OUTPUT:
		case TEEC_MEMREF_TEMP_INOUT:
			res = teec_pre_process_tmpref(ctx, param_type,
				&operation->params[n].tmpref, params + n,
				shms + n);
			if (res != TEEC_SUCCESS)
				return res;
			break;
		case TEEC_MEMREF_WHOLE:
			res = teec_pre_process_whole(
					&operation->params[n].memref,
					params + n);
			if (res != TEEC_SUCCESS)
				return res;
			break;
		case TEEC_MEMREF_PARTIAL_INPUT:
		case TEEC_MEMREF_PARTIAL_OUTPUT:
		case TEEC_MEMREF_PARTIAL_INOUT:
			res = teec_pre_process_partial(param_type,
				&operation->params[n].memref, params + n);
			if (res != TEEC_SUCCESS)
				return res;
			break;
		default:
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}

	return TEEC_SUCCESS;
}

static void teec_post_process_tmpref(uint32_t param_type,
			TEEC_TempMemoryReference_2 *tmpref,
			struct tee_ioctl_param *param,
			TEEC_SharedMemory_2 *shm)
{
	if (param_type != TEEC_MEMREF_TEMP_INPUT) {
		if (param->u.memref.size <= tmpref->size && tmpref->buffer)
			memcpy(tmpref->buffer, shm->buffer,
			       param->u.memref.size);

		tmpref->size = param->u.memref.size;
	}
}

static void teec_post_process_whole(TEEC_RegisteredMemoryReference_2 *memref,
			struct tee_ioctl_param *param)
{
	TEEC_SharedMemory_2 *shm = memref->parent;

	if (shm->flags & TEEC_MEM_OUTPUT) {

		/*
		 * We're using a shadow buffer in this reference, copy back
		 * the shadow buffer into the real buffer now that we've
		 * returned from secure world.
		 */
		if (shm->shadow_buffer && param->u.memref.size <= memref->size)
			memcpy(shm->buffer, shm->shadow_buffer,
			       param->u.memref.size);

		memref->size = param->u.memref.size;
	}
}

static void teec_post_process_partial(uint32_t param_type,
			TEEC_RegisteredMemoryReference_2 *memref,
			struct tee_ioctl_param *param)
{
	if (param_type != TEEC_MEMREF_PARTIAL_INPUT) {
		TEEC_SharedMemory_2 *shm = memref->parent;

		/*
		 * We're using a shadow buffer in this reference, copy back
		 * the shadow buffer into the real buffer now that we've
		 * returned from secure world.
		 */
		if (shm->shadow_buffer && param->u.memref.size <= memref->size)
			memcpy((char *)shm->buffer + memref->offset,
			       (char *)shm->shadow_buffer + memref->offset,
			       param->u.memref.size);

		memref->size = param->u.memref.size;
	}
}

static void teec_post_process_operation(TEEC_Operation_2 *operation,
			struct tee_ioctl_param *params,
			TEEC_SharedMemory_2 *shms)
{
	size_t n;

	if (!operation)
		return;

	for (n = 0; n < TEEC_CONFIG_PAYLOAD_REF_COUNT; n++) {
		uint32_t param_type;

		param_type = TEEC_PARAM_TYPE_GET(operation->paramTypes, n);
		switch (param_type) {
		case TEEC_VALUE_INPUT:
			break;
		case TEEC_VALUE_OUTPUT:
		case TEEC_VALUE_INOUT:
			operation->params[n].value.a = params[n].u.value.a;
			operation->params[n].value.b = params[n].u.value.b;
			break;
		case TEEC_MEMREF_TEMP_INPUT:
		case TEEC_MEMREF_TEMP_OUTPUT:
		case TEEC_MEMREF_TEMP_INOUT:
			teec_post_process_tmpref(param_type,
				&operation->params[n].tmpref, params + n,
				shms + n);
			break;
		case TEEC_MEMREF_WHOLE:
			teec_post_process_whole(&operation->params[n].memref,
						params + n);
			break;
		case TEEC_MEMREF_PARTIAL_INPUT:
		case TEEC_MEMREF_PARTIAL_OUTPUT:
		case TEEC_MEMREF_PARTIAL_INOUT:
			teec_post_process_partial(param_type,
				&operation->params[n].memref, params + n);
		default:
			break;
		}
	}
}

static void teec_free_temp_refs(TEEC_Operation_2 *operation,
			TEEC_SharedMemory_2 *shms)
{
	size_t n;

	if (!operation)
		return;

	for (n = 0; n < TEEC_CONFIG_PAYLOAD_REF_COUNT; n++) {
		switch (TEEC_PARAM_TYPE_GET(operation->paramTypes, n)) {
		case TEEC_MEMREF_TEMP_INPUT:
		case TEEC_MEMREF_TEMP_OUTPUT:
		case TEEC_MEMREF_TEMP_INOUT:
			TEEC_ReleaseSharedMemory(shms + n);
			break;
		default:
			break;
		}
	}
}

static TEEC_Result ioctl_errno_to_res(int err)
{
	switch (err) {
	case ENOMEM:
		return TEEC_ERROR_OUT_OF_MEMORY;
	default:
		return TEEC_ERROR_GENERIC;
	}
}

static void uuid_to_octets(uint8_t d[TEE_IOCTL_UUID_LEN], const TEEC_UUID *s)
{
	d[0] = s->timeLow >> 24;
	d[1] = s->timeLow >> 16;
	d[2] = s->timeLow >> 8;
	d[3] = s->timeLow;
	d[4] = s->timeMid >> 8;
	d[5] = s->timeMid;
	d[6] = s->timeHiAndVersion >> 8;
	d[7] = s->timeHiAndVersion;
	memcpy(d + 8, s->clockSeqAndNode, sizeof(s->clockSeqAndNode));
}
static void teec_resetTeeCmd(struct tee_cmd_io *cmd)
{
	cmd->fd_sess	= -1;
	cmd->cmd	= 0;
	cmd->uuid	= NULL;
	cmd->origin	= TEEC_ORIGIN_API;
	cmd->err	= TEEC_SUCCESS;
	cmd->data	= NULL;
	cmd->data_size	= 0;
	cmd->op		= NULL;
}

static int OPTEE_VERSION = -1;

#define BOOTARG_SIZE 2048
int get_optee_version(void)
{
	if(OPTEE_VERSION != -1)
		goto final;

	teec_mutex_lock(&teec_mutex);

	if(OPTEE_VERSION == -1){
		FILE* verfile = NULL;
		char sVer[BOOTARG_SIZE]={0};
		int u32ReadCount = 0;
		char* pFindString = NULL;

		verfile = fopen("/proc/tz2_mstar/version","r");
		if(verfile == NULL)
		{
			OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d can't open /proc/tz2_mstar/version capabilities to 1.0.1 \033[m\n",__func__,__LINE__);
			OPTEE_VERSION = 1;
			goto exit;
		}

		u32ReadCount = fread(sVer ,1 , BOOTARG_SIZE, verfile);

		if(u32ReadCount > BOOTARG_SIZE)
		{
			OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d warring : cmdline info more than buffer size\033[m\n",__func__,__LINE__);
			goto exit1;
		}

		sVer[BOOTARG_SIZE - 2] = '\n';
		sVer[BOOTARG_SIZE - 1] = '\0';

		pFindString = strstr(sVer,"2.4");
		if( pFindString == NULL) {
			OPTEE_VERSION = 1;
		}
		else {
			OPTEE_VERSION = 2;
		}

exit1:
		if(verfile != NULL)
			fclose(verfile);

		OPTEE_PRINT("\033[0;32;31m [OPTEE] %s %d OPTEE Version %s OPTEE_VERSION %d\033[m\n",__func__,__LINE__,sVer,OPTEE_VERSION);
	}

exit:
	teec_mutex_unlock(&teec_mutex);
final:
	return OPTEE_VERSION;
}

MS_U32 _MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context)
{
	if(get_optee_version() == 2)
	{
		char devname[PATH_MAX];
		size_t n;

		if (!context)
			return TEEC_ERROR_BAD_PARAMETERS;

		TEEC_Context_2 context_2;
		memset(&context_2, 0, sizeof(context_2));
		TEEC_Context(2_x, context, &context_2, GENERAL_TO_2_x);

		for (n = 0; n < TEEC_MAX_DEV_SEQ; n++) {
			snprintf(devname, sizeof(devname), "/dev/tee%zu", n);
			context_2.fd = teec_open_dev(devname, name);
			if (context_2.fd >= 0) {
				TEEC_Context(2_x, context, &context_2, GENERAL_FROM_2_x);
				context->fd = context_2.fd;
				return TEEC_SUCCESS;
			}
		}

		TEEC_Context(2_x, context, &context_2, GENERAL_FROM_2_x);

		return TEEC_ERROR_ITEM_NOT_FOUND;
	}
	else if(get_optee_version() == 1)
	{
		int name_size = 0;
		const char* _name = name;

		OPTEE_PRINT("TEEC initial Context: %s\n", name);

		if (context == NULL) {
			OPTEE_ERROR("\033[1;31m[%s] NULL context\033[m\n", __FUNCTION__);
			return TEEC_ERROR_BAD_PARAMETERS;
		}

		/*
		 * Specification says that when no name is provided it should fall back
		 * on a predefined TEE.
		 */
		if (name == NULL) {
			OPTEE_ERROR("\033[1;31m[%s] Name null\033[m\n", __FUNCTION__);
			return TEEC_ERROR_BAD_PARAMETERS; /* Device name truncated */
		}

		TEEC_Context_1 context_1;
		memset(&context_1, 0, sizeof(context_1));
		TEEC_Context(1_x, context, &context_1, GENERAL_TO_1_0_1);

		name_size = snprintf(context_1.devname, TEEC_MAX_DEVNAME_SIZE,
					 "/dev/%s", _name);

		if (name_size >= TEEC_MAX_DEVNAME_SIZE) {
			OPTEE_ERROR("\033[1;31m[%s] Name size over %u\033[m\n", __FUNCTION__, TEEC_MAX_DEVNAME_SIZE);
			return TEEC_ERROR_BAD_PARAMETERS; /* Device name truncated */
		}

		context_1.fd = open(context_1.devname, O_RDWR);
		if (context_1.fd == -1) {
			OPTEE_ERROR("\033[1;31m[%s]open %s fail \033[m\n", __FUNCTION__, context_1.devname);
			return TEEC_ERROR_ITEM_NOT_FOUND;
		}

		TEEC_Context(1_x, context, &context_1, GENERAL_FROM_1_0_1);

		return TEEC_SUCCESS;
	}

	return TEEC_ERROR_NOT_SUPPORTED;
}

MS_U32 _MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin)
{
	if(get_optee_version() == 2)
	{
		uint64_t buf[(sizeof(struct tee_ioctl_open_session_arg) +
				TEEC_CONFIG_PAYLOAD_REF_COUNT *
					sizeof(struct tee_ioctl_param)) /
				sizeof(uint64_t)] = { 0 };
		struct tee_ioctl_buf_data buf_data;
		struct tee_ioctl_open_session_arg *arg;
		struct tee_ioctl_param *params;
		TEEC_Result res;
		uint32_t eorig;
		TEEC_SharedMemory_2 shm[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		int rc;

		(void)&connection_data;
		TEEC_Session_2 session_2;
		memset(&session_2, 0, sizeof(session_2));
		TEEC_Context_2 context_2;
		memset(&context_2, 0, sizeof(context_2));
		TEEC_Operation_2 operation_2;
		memset(&operation_2, 0, sizeof(operation_2));

		if (!context || !session) {
			eorig = TEEC_ORIGIN_API;
			res = TEEC_ERROR_BAD_PARAMETERS;
			OPTEE_ERROR("\033[1;31m[%s] context or session is NULL\033[m\n", __FUNCTION__);
			goto out;
		}

		buf_data.buf_ptr = (uintptr_t)buf;
		buf_data.buf_len = sizeof(buf);

		arg = (struct tee_ioctl_open_session_arg *)buf;
		arg->num_params = TEEC_CONFIG_PAYLOAD_REF_COUNT;
		params = (struct tee_ioctl_param *)(arg + 1);

		uuid_to_octets(arg->uuid, destination);
		arg->clnt_login = connection_method;

		TEEC_Context(2_x, context, &context_2, GENERAL_TO_2_x);
		TEEC_Operation(2_x, operation, &operation_2, GENERAL_TO_2_x);
		TEEC_Session(2_x, session, &session_2, GENERAL_TO_2_x);

		res = teec_pre_process_operation(&context_2, &operation_2, params, shm);
		if (res != TEEC_SUCCESS) {
			eorig = TEEC_ORIGIN_API;
			goto out_free_temp_refs;
		}


		rc = ioctl(context_2.fd, TEE_IOC_OPEN_SESSION, &buf_data);
		if (rc) {
			OPTEE_ERROR("TEE_IOC_OPEN_SESSION failed");
			eorig = TEEC_ORIGIN_COMMS;
			res = ioctl_errno_to_res(errno);
			goto out_free_temp_refs;
		}
		res = arg->ret;
		eorig = arg->ret_origin;
		if (res == TEEC_SUCCESS) {
			session_2.ctx = &context_2;
			session_2.session_id = arg->session;
		}
		teec_post_process_operation(&operation_2, params, shm);

	out_free_temp_refs:
		teec_free_temp_refs(&operation_2, shm);
	out:
		if (error_origin)
			*error_origin = eorig;

		TEEC_Operation(2_x, operation, &operation_2, GENERAL_FROM_2_x);
		TEEC_Session(2_x, session, &session_2, GENERAL_FROM_2_x);
		TEEC_Context(2_x, context, &context_2, GENERAL_FROM_2_x);

		return res;
	}
	else if(get_optee_version() == 1)
	{
		TEEC_Operation dummy_op;
		uint32_t origin = TEEC_ORIGIN_API;
		TEEC_Result res = TEEC_SUCCESS;
		(void)connection_data;
		struct tee_cmd_io cmd;

		memset(&dummy_op, 0, sizeof(TEEC_Operation));
		memset(&cmd, 0, sizeof(struct tee_cmd_io));

		if (session != NULL)
			session->fd = -1;

		if ((context == NULL) || (session == NULL)) {
					OPTEE_ERROR("\033[1;31m[%s] Null context or session\033[m\n", __FUNCTION__);
			res = TEEC_ERROR_BAD_PARAMETERS;
			goto error;
		}

		if (connection_method != TEEC_LOGIN_PUBLIC) {
			OPTEE_ERROR("\033[1;31m[%s] connection_method not public\033[m\n", __FUNCTION__);
			res = TEEC_ERROR_NOT_SUPPORTED;
			goto error;
		}

		teec_resetTeeCmd(&cmd);
		cmd.uuid = (TEEC_UUID *)destination;

		if (operation == NULL) {
			/*
			 * The code here exist because Global Platform API states that
			 * it is allowed to give operation as a NULL pointer. In kernel
			 * and secure world we in most cases don't want this to be NULL,
			 * hence we use this dummy operation when a client doesn't
			 * provide any operation.
			 */
			memset(&dummy_op, 0, sizeof(TEEC_Operation));
			operation = &dummy_op;
		}

		TEEC_Context_1 context_1;
		memset(&context_1, 0, sizeof(context_1));
		TEEC_Context(1_x, context, &context_1, GENERAL_TO_1_0_1);

		TEEC_Operation_1 operation_1;
		memset(&operation_1, 0, sizeof(operation_1));
		TEEC_Operation(1_x, operation, &operation_1, GENERAL_TO_1_0_1);

		TEEC_Session_1 session_1;
		memset(&session_1, 0, sizeof(session_1));
		TEEC_Session(1_x, session, &session_1, GENERAL_TO_1_0_1);

		cmd.op = &operation_1;

		errno = 0;
		if (ioctl(context_1.fd, TEE_OPEN_SESSION_IOC, &cmd) != 0) {
			OPTEE_ERROR("Ioctl(TEE_OPEN_SESSION_IOC) failed! (%s) err %08x ori %08x\n",
				strerror(errno), cmd.err, cmd.origin);
			if (cmd.origin)
				origin = cmd.origin;
			else
				origin = TEEC_ORIGIN_COMMS;
			if (cmd.err)
				res = cmd.err;
			else
				res = TEEC_ERROR_COMMUNICATION;
			goto error;
		}
		session_1.fd = cmd.fd_sess;

		if (cmd.err != 0) {
			OPTEE_ERROR("open session to TA UUID %x %x %x failed\n",
				destination->timeLow,
				destination->timeMid, destination->timeHiAndVersion);
		}
		origin = cmd.origin;
		res = cmd.err;

	error:
		// printf("**** res=0x%08x, org=%d, seeid=%d ***\n", res, origin, cmd.fd_sess)

		/*
		 * We do this check at the end instead of checking on every place where
		 * we set the error origin.
		 */
		if (res != TEEC_SUCCESS) {
			if (session != NULL && session_1.fd != -1) {
				close(session_1.fd);
				session_1.fd = -1;
			}
		}

		if (error_origin != NULL)
			*error_origin = origin;

		TEEC_Operation(1_x, operation, &operation_1, GENERAL_FROM_1_0_1);
		TEEC_Session(1_x, session, &session_1, GENERAL_FROM_1_0_1);
		TEEC_Context(1_x, context, &context_1, GENERAL_FROM_1_0_1);

		return res;
	}

	return TEEC_ERROR_NOT_SUPPORTED;
}

void _MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context)
{
	if(get_optee_version() == 2)
	{
		TEEC_Context_2 context_2;
		memset(&context_2, 0, sizeof(context_2));
		TEEC_Context(2_x, context, &context_2, GENERAL_TO_2_x);

		if (context)
			close(context_2.fd);

		TEEC_Context(2_x, context, &context_2, GENERAL_FROM_2_x);
	}
	else if(get_optee_version() == 1)
	{
		TEEC_Context_1 context_1;
		memset(&context_1, 0, sizeof(context_1));
		TEEC_Context(1_x, context, &context_1, GENERAL_TO_1_0_1);

		if (context)
			close(context_1.fd);

		TEEC_Context(1_x, context, &context_1, GENERAL_FROM_1_0_1);
	}
}

void _MDrv_SYS_TEEC_Close(TEEC_Session *session)
{
	if(get_optee_version() == 2)
	{
		struct tee_ioctl_close_session_arg arg;
		int ret = 0;

		if (!session)
			return;

		TEEC_Session_2 session_2;
		memset(&session_2, 0, sizeof(session_2));
		TEEC_Session(2_x, session, &session_2, GENERAL_TO_2_x);

		arg.session = session_2.session_id;
		printf("\033[0;32;31m [Ian] %s %d %d\033[m\n",__func__,__LINE__,session_2.ctx->fd);
		if ((ret = ioctl(session_2.ctx->fd, TEE_IOC_CLOSE_SESSION, &arg)))
			OPTEE_ERROR("Failed to close session 0x%x %d", session_2.session_id,ret);

		TEEC_Session(2_x, session, &session_2, GENERAL_FROM_2_x);
	}
	else if(get_optee_version() == 1)
	{
		if (session == NULL)
			return;

		TEEC_Session_1 session_1;
		memset(&session_1, 0, sizeof(session_1));
		TEEC_Session(1_x, session, &session_1, GENERAL_TO_1_0_1);

		close(session_1.fd);

		TEEC_Session(1_x, session, &session_1, GENERAL_FROM_1_0_1);
	}
}

MS_U32 _MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin)
{
	if(get_optee_version() == 2)
	{
		uint64_t buf[(sizeof(struct tee_ioctl_invoke_arg) +
				TEEC_CONFIG_PAYLOAD_REF_COUNT *
					sizeof(struct tee_ioctl_param)) /
				sizeof(uint64_t)] = { 0 };
		struct tee_ioctl_buf_data buf_data;
		struct tee_ioctl_invoke_arg *arg;
		struct tee_ioctl_param *params;
		TEEC_Result res;
		uint32_t eorig;
		TEEC_SharedMemory_2 shm[TEEC_CONFIG_PAYLOAD_REF_COUNT];
		int rc;

		TEEC_Session_2 session_2;
		memset(&session_2, 0, sizeof(session_2));
		TEEC_Operation_2 operation_2;
		memset(&operation_2, 0, sizeof(operation_2));
		if (!session) {
			eorig = TEEC_ORIGIN_API;
			res = TEEC_ERROR_BAD_PARAMETERS;
			goto out;
		}

		TEEC_Session(2_x, session, &session_2, GENERAL_TO_2_x);
		TEEC_Operation(2_x, operation, &operation_2, GENERAL_TO_2_x);

		buf_data.buf_ptr = (uintptr_t)buf;
		buf_data.buf_len = sizeof(buf);

		arg = (struct tee_ioctl_invoke_arg *)buf;
		arg->num_params = TEEC_CONFIG_PAYLOAD_REF_COUNT;
		params = (struct tee_ioctl_param *)(arg + 1);

		arg->session = session_2.session_id;
		arg->func = cmd_id;

		if (operation) {
			teec_mutex_lock(&teec_mutex);
			operation_2.session = &session_2;
			teec_mutex_unlock(&teec_mutex);
		}

		res = teec_pre_process_operation(session_2.ctx, &operation_2, params, shm);
		if (res != TEEC_SUCCESS) {
			eorig = TEEC_ORIGIN_API;
			goto out_free_temp_refs;
		}

		rc = ioctl(session_2.ctx->fd, TEE_IOC_INVOKE, &buf_data);
		if (rc) {
			OPTEE_ERROR("TEE_IOC_INVOKE failed");
			eorig = TEEC_ORIGIN_COMMS;
			res = ioctl_errno_to_res(errno);
			goto out_free_temp_refs;
		}

		res = arg->ret;
		eorig = arg->ret_origin;
		teec_post_process_operation(&operation_2, params, shm);

	out_free_temp_refs:
		teec_free_temp_refs(&operation_2, shm);
	out:
		if (error_origin)
			*error_origin = eorig;

		TEEC_Session(2_x, session, &session_2, GENERAL_FROM_2_x);
		TEEC_Operation(2_x, operation, &operation_2, GENERAL_FROM_2_x);

		return res;
	}
	else if(get_optee_version() == 1)
	{
		//INMSG("session: [%p], cmd_id: [%d]", session, cmd_id);
		struct tee_cmd_io cmd;
		TEEC_Operation dummy_op;
		TEEC_Result result = TEEC_SUCCESS;
		uint32_t origin = TEEC_ORIGIN_API;

		memset(&dummy_op, 0, sizeof(TEEC_Operation));
		memset(&cmd, 0, sizeof(struct tee_cmd_io));

		if (session == NULL) {
					OPTEE_ERROR("\033[1;31m[%s] Null session\033[m\n", __FUNCTION__);
			origin = TEEC_ORIGIN_API;
			result = TEEC_ERROR_BAD_PARAMETERS;
			goto error;
		}

		if (operation == NULL) {
			/*
			 * The code here exist because Global Platform API states that
			 * it is allowed to give operation as a NULL pointer. In kernel
			 * and secure world we in most cases don't want this to be NULL,
			 * hence we use this dummy operation when a client doesn't
			 * provide any operation.
			 */
			memset(&dummy_op, 0, sizeof(TEEC_Operation));
			operation = &dummy_op;
		}

		TEEC_Session_1 session_1;
		memset(&session_1, 0, sizeof(session_1));
		TEEC_Session(1_x, session, &session_1, GENERAL_TO_1_0_1);

		TEEC_Operation_1 operation_1;
		memset(&operation_1, 0, sizeof(operation_1));
		TEEC_Operation(1_x, operation, &operation_1, GENERAL_TO_1_0_1);

		teec_mutex_lock(&teec_mutex);
		operation_1.session = &session_1;
		teec_mutex_unlock(&teec_mutex);

		teec_resetTeeCmd(&cmd);

		cmd.cmd = cmd_id;
		cmd.op = &operation_1;

		if (ioctl(session_1.fd, TEE_INVOKE_COMMAND_IOC, &cmd) != 0)
			OPTEE_ERROR("Ioctl(TEE_INVOKE_COMMAND_IOC) failed! (%s)\n",strerror(errno));

		teec_mutex_lock(&teec_mutex);
		operation_1.session = NULL;
		teec_mutex_unlock(&teec_mutex);

		origin = cmd.origin;
		result = cmd.err;

	error:

		if (error_origin != NULL)
			*error_origin = origin;

		TEEC_Session(1_x, session, &session_1, GENERAL_FROM_1_0_1);
		TEEC_Operation(1_x, operation, &operation_1, GENERAL_FROM_1_0_1);

		OPTEE_PRINT("result:%d\n",result);
		return result;
	}

	return TEEC_ERROR_NOT_SUPPORTED;
}

TEEC_Result TEEC_AllocateSharedMemory(TEEC_Context_2 *ctx, TEEC_SharedMemory_2 *shm)
{
	int fd;
	size_t s;

	if (!ctx || !shm)
		return TEEC_ERROR_BAD_PARAMETERS;

	if (!shm->flags || (shm->flags & ~(TEEC_MEM_INPUT | TEEC_MEM_OUTPUT)))
		return TEEC_ERROR_BAD_PARAMETERS;

	s = shm->size;
	if (!s)
		s = 8;

	fd = teec_shm_alloc(ctx->fd, s, &shm->id);
	if (fd < 0)
		return TEEC_ERROR_OUT_OF_MEMORY;

	shm->buffer = mmap(NULL, s, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (shm->buffer == (void *)MAP_FAILED) {
		shm->id = -1;
		return TEEC_ERROR_OUT_OF_MEMORY;
	}
	shm->shadow_buffer = NULL;
	shm->alloced_size = s;
	shm->registered_fd = -1;
	return TEEC_SUCCESS;
}

void TEEC_ReleaseSharedMemory(TEEC_SharedMemory_2 *shm)
{
	if (!shm || shm->id == -1)
		return;

	if (shm->shadow_buffer)
		munmap(shm->shadow_buffer, shm->alloced_size);
	else if (shm->buffer)
		munmap(shm->buffer, shm->alloced_size);
	else if (shm->registered_fd >= 0)
		close(shm->registered_fd);

	shm->id = -1;
	shm->shadow_buffer = NULL;
	shm->buffer = NULL;
	shm->registered_fd = -1;
}
#else
#include "tee_client_api.h"
#include "MsCommon.h"

MS_U32 _MDrv_SYS_TEEC_InitializeContext(const char *name, TEEC_Context *context)
{
	OPTEE_ERROR("\033[0;32;31m %s %d Not Impelement\033[m\n",__func__,__LINE__);
	return 0;
}

MS_U32 _MDrv_SYS_TEEC_Open(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, MS_U32 connection_method, const void *connection_data, TEEC_Operation *operation, MS_U32 *error_origin)
{
	OPTEE_ERROR("\033[0;32;31m %s %d Not Impelement\033[m\n",__func__,__LINE__);
	return 0;
}

void _MDrv_SYS_TEEC_FinalizeContext(TEEC_Context *context)
{
	OPTEE_ERROR("\033[0;32;31m %s %d Not Impelement\033[m\n",__func__,__LINE__);
}

void _MDrv_SYS_TEEC_Close(TEEC_Session *session)
{
	OPTEE_ERROR("\033[0;32;31m %s %d Not Impelement\033[m\n",__func__,__LINE__);
}

MS_U32 _MDrv_SYS_TEEC_InvokeCmd(TEEC_Session *session, MS_U32 cmd_id, TEEC_Operation *operation, MS_U32 *error_origin)
{
	OPTEE_ERROR("\033[0;32;31m %s %d Not Impelement\033[m\n",__func__,__LINE__);
	return 0;
}

#endif //MSOS_TYPE_LINUX
