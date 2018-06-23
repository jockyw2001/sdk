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
#ifndef TEE_CLIENT_API_H
#define TEE_CLIENT_API_H

#if defined(MSOS_TYPE_LINUX_KERNEL)
#include <linux/types.h>
#endif

#include "tee_client_api_2.4.h"
#if defined(MSOS_TYPE_LINUX)
#include "tee.h"
#endif
#include "tee_client_api_1.0.1.h"
#if defined(MSOS_TYPE_LINUX)
#include "tee_ioc.h"
#endif
#include "tee_client_api_general.h"
#include "ULog.h"

#define OPTEE_ERROR(fmt, args...)    ULOGE("OPTEE","[%06d]     " fmt, __LINE__, ## args)
#define OPTEE_WARN(fmt, args...)    ULOGW("OPTEE","[%06d]     " fmt, __LINE__, ## args)
#define OPTEE_PRINT(fmt, args...)    ULOGI("OPTEE","[%06d]     " fmt, __LINE__, ## args)

#endif
