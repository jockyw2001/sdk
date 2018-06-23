#if defined (MSOS_TYPE_LINUX)
#include <string.h>
#include "tee_client_api_general.h"
#include <stdio.h>

TEEC_Result TEEC_Operation_general_to_1_x(TEEC_Operation *general, size_t general_size,TEEC_Operation_1 *data_1, size_t data_1_size, DIRECTION from)
{
	int i;

	if(general == NULL || data_1 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_1_0_1 && from != GENERAL_FROM_1_0_1){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(general_size != sizeof(TEEC_Operation)){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d Struct TEEC_Operation not match sizeof(TEEC_Operation) = %zu org = %zu \033[m\n",__func__,__LINE__,sizeof(TEEC_Operation),general_size);
	}

	if(data_1_size != sizeof(TEEC_Operation_1)){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d Struct TEEC_Operation_1 not match sizeof(TEEC_Operation_1) = %zu org = %zu \033[m\n",__func__,__LINE__,sizeof(TEEC_Operation_1),data_1_size);
	}

	if(from == GENERAL_TO_1_0_1){
		data_1->started = general->started;

		data_1->paramTypes = general->paramTypes;

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			uint32_t param_type;

			param_type = TEEC_PARAM_TYPE_GET(general->paramTypes, i);
			switch (param_type) {
				case TEEC_NONE:
					break;
				case TEEC_VALUE_INPUT:
				case TEEC_VALUE_OUTPUT:
				case TEEC_VALUE_INOUT:
					data_1->params[i].value.a = general->params[i].value.a;
					data_1->params[i].value.b = general->params[i].value.b;
					break;
				case TEEC_MEMREF_TEMP_INPUT:
				case TEEC_MEMREF_TEMP_OUTPUT:
				case TEEC_MEMREF_TEMP_INOUT:
					data_1->params[i].tmpref.buffer = general->params[i].tmpref.buffer;
					data_1->params[i].tmpref.size = general->params[i].tmpref.size;
					break;
				case TEEC_MEMREF_WHOLE:
					break;
				case TEEC_MEMREF_PARTIAL_INPUT:
				case TEEC_MEMREF_PARTIAL_OUTPUT:
				case TEEC_MEMREF_PARTIAL_INOUT:
					data_1->params[i].memref.parent = (TEEC_SharedMemory_1 *)general->params[i].memref.parent;
					data_1->params[i].memref.size = general->params[i].memref.size;
					data_1->params[i].memref.offset = general->params[i].memref.offset;
				default:
					return TEEC_ERROR_BAD_PARAMETERS;
			}

			#if 0
			data_1->params[i].tmpref.buffer = general->params[i].tmpref.buffer;
			data_1->params[i].tmpref.size = general->params[i].tmpref.size;
			data_1->params[i].memref.parent = (TEEC_SharedMemory_1 *)general->params[i].memref.parent;
			data_1->params[i].memref.size = general->params[i].memref.size;
			data_1->params[i].memref.offset = general->params[i].memref.offset;
			data_1->params[i].value.a = general->params[i].value.a;
			data_1->params[i].value.b = general->params[i].value.b;
			#endif
		}

		data_1->session = (TEEC_Session_1 *)general->session;

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			data_1->memRefs[i].buffer = general->memRefs[i].buffer;
			data_1->memRefs[i].size = general->memRefs[i].size;
			data_1->memRefs[i].flags = general->memRefs[i].flags;
			data_1->memRefs[i].reserved = general->memRefs[i].reserved;
			data_1->memRefs[i].d.fd = general->memRefs[i].d.fd;
			data_1->memRefs[i].registered = general->memRefs[i].registered;
		}

		data_1->flags = general->flags;
	}
	else{
		general->started = data_1->started;

		general->paramTypes = data_1->paramTypes;

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			uint32_t param_type;

			param_type = TEEC_PARAM_TYPE_GET(data_1->paramTypes, i);
			switch (param_type) {
				case TEEC_VALUE_INPUT:
					break;
				case TEEC_VALUE_OUTPUT:
				case TEEC_VALUE_INOUT:
					general->params[i].value.a = data_1->params[i].value.a;
					general->params[i].value.b = data_1->params[i].value.b;
					break;
				case TEEC_MEMREF_TEMP_INPUT:
				case TEEC_MEMREF_TEMP_OUTPUT:
				case TEEC_MEMREF_TEMP_INOUT:
					general->params[i].tmpref.buffer = data_1->params[i].tmpref.buffer;
					general->params[i].tmpref.size = data_1->params[i].tmpref.size;
				case TEEC_MEMREF_WHOLE:
					break;
				case TEEC_MEMREF_PARTIAL_INPUT:
				case TEEC_MEMREF_PARTIAL_OUTPUT:
				case TEEC_MEMREF_PARTIAL_INOUT:
					general->params[i].memref.parent = (TEEC_SharedMemory *)data_1->params[i].memref.parent;
					general->params[i].memref.size = data_1->params[i].memref.size;
					general->params[i].memref.offset = data_1->params[i].memref.offset;
				default:
					break;
			}

			#if 0
			general->params[i].tmpref.buffer = data_1->params[i].tmpref.buffer;
			general->params[i].tmpref.size = data_1->params[i].tmpref.size;
			general->params[i].memref.parent = (TEEC_SharedMemory *)data_1->params[i].memref.parent;
			general->params[i].memref.size = data_1->params[i].memref.size;
			general->params[i].memref.offset = data_1->params[i].memref.offset;
			general->params[i].value.a = data_1->params[i].value.a;
			general->params[i].value.b = data_1->params[i].value.b;
			#endif
		}

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			general->memRefs[i].buffer = data_1->memRefs[i].buffer;
			general->memRefs[i].size = data_1->memRefs[i].size;
			general->memRefs[i].flags = data_1->memRefs[i].flags;
			general->memRefs[i].reserved = data_1->memRefs[i].reserved;
			general->memRefs[i].d.fd = data_1->memRefs[i].d.fd;
			general->memRefs[i].registered = data_1->memRefs[i].registered;
		}

		general->session = (TEEC_Session *)data_1->session;
		general->flags = data_1->flags;
	}

	return TEEC_SUCCESS;
}

TEEC_Result TEEC_Operation_general_to_2_x(TEEC_Operation *general, size_t general_size, TEEC_Operation_2 *data_2, size_t data_2_size, DIRECTION from)
{
	int i;

	if(general == NULL || data_2 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_2_x && from != GENERAL_FROM_2_x){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(general_size != sizeof(TEEC_Operation)){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d Struct TEEC_Operation not match sizeof(TEEC_Operation) = %zu org = %zu \033[m\n",__func__,__LINE__,sizeof(TEEC_Operation),general_size);
	}

	if(data_2_size != sizeof(TEEC_Operation_2)){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d Struct TEEC_Operation_2 not match sizeof(TEEC_Operation_2) = %zu org = %zu \033[m\n",__func__,__LINE__,sizeof(TEEC_Operation_2),data_2_size);
	}

	if(from == GENERAL_TO_2_x){
		data_2->started = general->started;
		data_2->paramTypes = general->paramTypes;

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			uint32_t param_type;

			param_type = TEEC_PARAM_TYPE_GET(general->paramTypes, i);
			switch (param_type) {
				case TEEC_NONE:
					break;
				case TEEC_VALUE_INPUT:
				case TEEC_VALUE_OUTPUT:
				case TEEC_VALUE_INOUT:
					data_2->params[i].value.a = general->params[i].value.a;
					data_2->params[i].value.b = general->params[i].value.b;
					break;
				case TEEC_MEMREF_TEMP_INPUT:
				case TEEC_MEMREF_TEMP_OUTPUT:
				case TEEC_MEMREF_TEMP_INOUT:
					data_2->params[i].tmpref.buffer = general->params[i].tmpref.buffer;
					data_2->params[i].tmpref.size = general->params[i].tmpref.size;
					break;
				case TEEC_MEMREF_WHOLE:
					break;
				case TEEC_MEMREF_PARTIAL_INPUT:
				case TEEC_MEMREF_PARTIAL_OUTPUT:
				case TEEC_MEMREF_PARTIAL_INOUT:
					data_2->params[i].memref.parent = (TEEC_SharedMemory_2 *)general->params[i].memref.parent;
					data_2->params[i].memref.size = general->params[i].memref.size;
					data_2->params[i].memref.offset = general->params[i].memref.offset;
				default:
					return TEEC_ERROR_BAD_PARAMETERS;
			}
			#if 0
			data_2->params[i].tmpref.buffer = general->params[i].tmpref.buffer;
			data_2->params[i].tmpref.size = general->params[i].tmpref.size;
			data_2->params[i].memref.parent = (TEEC_SharedMemory_2 *)general->params[i].memref.parent;
			data_2->params[i].memref.size = general->params[i].memref.size;
			data_2->params[i].memref.offset = general->params[i].memref.offset;
			data_2->params[i].value.a = general->params[i].value.a;
			data_2->params[i].value.b = general->params[i].value.b;
			#endif
		}

		data_2->session = (TEEC_Session_2 *)general->session;
	}
	else{
		general->started = data_2->started;
		general->paramTypes = data_2->paramTypes;

		for(i = 0; i < TEEC_CONFIG_PAYLOAD_REF_COUNT; i++){
			uint32_t param_type;

			param_type = TEEC_PARAM_TYPE_GET(data_2->paramTypes, i);
			switch (param_type) {
				case TEEC_VALUE_INPUT:
					break;
				case TEEC_VALUE_OUTPUT:
				case TEEC_VALUE_INOUT:
					general->params[i].value.a = data_2->params[i].value.a;
					general->params[i].value.b = data_2->params[i].value.b;
					break;
				case TEEC_MEMREF_TEMP_INPUT:
				case TEEC_MEMREF_TEMP_OUTPUT:
				case TEEC_MEMREF_TEMP_INOUT:
					general->params[i].tmpref.buffer = data_2->params[i].tmpref.buffer;
					general->params[i].tmpref.size = data_2->params[i].tmpref.size;
				case TEEC_MEMREF_WHOLE:
					break;
				case TEEC_MEMREF_PARTIAL_INPUT:
				case TEEC_MEMREF_PARTIAL_OUTPUT:
				case TEEC_MEMREF_PARTIAL_INOUT:
					general->params[i].memref.parent = (TEEC_SharedMemory *)data_2->params[i].memref.parent;
					general->params[i].memref.size = data_2->params[i].memref.size;
					general->params[i].memref.offset = data_2->params[i].memref.offset;
				default:
					break;
			}

			#if 0
			general->params[i].tmpref.buffer = data_2->params[i].tmpref.buffer;
			general->params[i].tmpref.size = data_2->params[i].tmpref.size;
			general->params[i].memref.parent = (TEEC_SharedMemory *)data_2->params[i].memref.parent;
			general->params[i].memref.size = data_2->params[i].memref.size;
			general->params[i].memref.offset = data_2->params[i].memref.offset;
			general->params[i].value.a = data_2->params[i].value.a;
			general->params[i].value.b = data_2->params[i].value.b;
			#endif
		}

		general->session = (TEEC_Session *)data_2->session;
	}

	return TEEC_SUCCESS;
}

TEEC_Result TEEC_Context_general_to_1_x(TEEC_Context *general, size_t general_size, TEEC_Context_1 *data_1, size_t data_1_size, DIRECTION from)
{
	if(general == NULL || data_1 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_1_0_1 && from != GENERAL_FROM_1_0_1){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from == GENERAL_TO_1_0_1){
		if(general_size == data_1_size)
			memcpy(data_1, general, sizeof(TEEC_Context_1));
		else{
			OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}
	else{
		if(general_size == data_1_size){
			memcpy(general, data_1, sizeof(TEEC_Context_1));
		}
		else{
			OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}

	return TEEC_SUCCESS;
}

TEEC_Result TEEC_Context_general_to_2_x(TEEC_Context *general, size_t general_size, TEEC_Context_2 *data_2, size_t data_2_size, DIRECTION from)
{

	if(general == NULL || data_2 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_2_x && from != GENERAL_FROM_2_x){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}


	if(from == GENERAL_TO_2_x){
		data_2->fd = general->fd;
	}
	else{
		general->fd = data_2->fd;
	}

	return TEEC_SUCCESS;
}

TEEC_Result TEEC_Session_general_to_1_x(TEEC_Session *general, size_t general_size, TEEC_Session_1 *data_1, size_t data_1_size, DIRECTION from)
{

	if(general == NULL || data_1 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_1_0_1 && from != GENERAL_FROM_1_0_1){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from == GENERAL_TO_1_0_1){
		data_1->fd = general->fd;
	}
	else{
		general->fd = data_1->fd;
	}

	return TEEC_SUCCESS;
}

TEEC_Result TEEC_Session_general_to_2_x(TEEC_Session *general, size_t general_size, TEEC_Session_2 *data_2, size_t data_2_size, DIRECTION from)
{

	if(general == NULL || data_2 == NULL){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d \033[m\n",__func__,__LINE__);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from != GENERAL_TO_2_x && from != GENERAL_FROM_2_x){
		OPTEE_ERROR("\033[0;32;31m [OPTEE] %s %d %x\033[m\n",__func__,__LINE__,from);
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if(from == GENERAL_TO_2_x){
		data_2->session_id = general->session_id;
		data_2->ctx = &general->ctx;
	}
	else{
		general->session_id = data_2->session_id;
		//general->ctx = data_2->ctx;
		memcpy(&general->ctx, data_2->ctx, sizeof(general->ctx));
	}

	return TEEC_SUCCESS;
}
#endif
