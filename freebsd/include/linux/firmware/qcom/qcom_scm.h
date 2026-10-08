/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 James Kane
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _VPU_LINUX_QCOM_SCM_H_
#define	_VPU_LINUX_QCOM_SCM_H_

/*
 * Qualcomm's Secure Channel Manager calls, for Qualcomm's Iris: FreeBSD's
 * qcom_scm(4), which names them as Linux does but returns positive errnos.
 */
#include <linux/types.h>

int	(qcom_scm_pas_auth_and_reset)(uint32_t pas_id);
int	(qcom_scm_pas_shutdown)(uint32_t pas_id);
int	(qcom_scm_set_remote_state)(uint32_t state, uint32_t id);
int	(qcom_scm_mem_protect_video_var)(uint32_t cp_start, uint32_t cp_size,
	    uint32_t nonpixel_start, uint32_t nonpixel_size);

#define	qcom_scm_pas_auth_and_reset(id)	(-(qcom_scm_pas_auth_and_reset)(id))
#define	qcom_scm_pas_shutdown(id)	(-(qcom_scm_pas_shutdown)(id))
#define	qcom_scm_set_remote_state(s, id) (-(qcom_scm_set_remote_state)(s, id))
#define	qcom_scm_mem_protect_video_var(a, b, c, d)			\
	(-(qcom_scm_mem_protect_video_var)(a, b, c, d))

#endif /* _VPU_LINUX_QCOM_SCM_H_ */
