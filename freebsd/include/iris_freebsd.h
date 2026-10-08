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

#ifndef _IRIS_FREEBSD_H_
#define	_IRIS_FREEBSD_H_

/*
 * Included before every file of Qualcomm's Iris driver (and the glue): what
 * it needs that LinuxKPI lacks, beyond the shim headers in linux/.
 */
#ifndef IRIS_FBSD_BUS

#include <linux/types.h>

/* The SC8280XP's UBWC configuration is the glue's (iris_freebsd.c). */
#define	CONFIG_QCOM_UBWC_CONFIG	1

/* linux/hash.h's, missing from LinuxKPI's. */
static inline u32
hash32_ptr(const void *ptr)
{
	unsigned long val = (unsigned long)ptr;

	val ^= (val >> 32);
	return ((u32)val);
}

/* LinuxKPI's are RUNTIME_PM_OPS(); no devicetree module tables. */
#define	SET_RUNTIME_PM_OPS(s, r, i)	RUNTIME_PM_OPS(s, r, i)
#define	MODULE_DEVICE_TABLE_BUS_of(bus, table)

struct device;

/* Runtime PM on, for good (the device lives as long as the module). */
#define	devm_pm_runtime_enable(dev)	(pm_runtime_enable(dev), 0)

/* LinuxKPI's devices have no DMA parameters: segments are unlimited. */
#define	dma_set_seg_boundary(dev, mask)	(0)

#endif /* !IRIS_FBSD_BUS */

#endif /* _IRIS_FREEBSD_H_ */
