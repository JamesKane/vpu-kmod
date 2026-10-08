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

#ifndef _VPU_LINUX_OF_H_
#define	_VPU_LINUX_OF_H_

/*
 * Devicetree nodes, for drivers ported from Linux that run under ACPI here:
 * there is no tree, so lookups find nothing, and the glue supplies what the
 * driver would read from it.  (LinuxKPI's own linux/of.h is empty.)
 */
#include <linux/kobject.h>
#include <linux/types.h>
#include <linux/errno.h>

struct device;
struct device_node;
struct resource;

struct of_device_id {
	char		name[32];
	char		type[32];
	char		compatible[128];
	const void	*data;
};

static inline void
of_node_put(struct device_node *np)
{
}

static inline bool
of_device_is_compatible(const struct device_node *np, const char *compat)
{
	return (false);
}

static inline struct device_node *
of_get_child_by_name(const struct device_node *np, const char *name)
{
	return (NULL);
}

static inline int
of_property_read_string_index(const struct device_node *np,
    const char *propname, int index, const char **output)
{
	return (-EINVAL);
}

static inline int
of_dma_configure(struct device *dev, struct device_node *np, bool force_dma)
{
	return (-ENODEV);
}

/* The match table entry of the glue's compatible string (iris_freebsd.c). */
const void *of_device_get_match_data(const struct device *dev);
/* Reserved memory: the glue's regions. */
int	of_reserved_mem_region_to_resource(const struct device_node *np,
	    unsigned int idx, struct resource *res);

#endif /* _VPU_LINUX_OF_H_ */
