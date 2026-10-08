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

#ifndef _VPU_LINUX_INTERCONNECT_H_
#define	_VPU_LINUX_INTERCONNECT_H_

/*
 * Interconnect paths, for Qualcomm's Iris: the glue turns bandwidth into
 * RPMh votes (iris_freebsd.c).
 */
#include <linux/types.h>

struct device;
struct icc_path;

struct icc_bulk_data {
	struct icc_path	*path;
	const char	*name;
	u32		avg_bw;
	u32		peak_bw;
};

#define	Bps_to_icc(x)	((x) / 1000)
#define	kBps_to_icc(x)	(x)
#define	MBps_to_icc(x)	((x) * 1000)

int	devm_of_icc_bulk_get(struct device *dev, int num_paths,
	    struct icc_bulk_data *paths);
int	icc_bulk_set_bw(int num_paths, const struct icc_bulk_data *bulk);

#endif /* _VPU_LINUX_INTERCONNECT_H_ */
