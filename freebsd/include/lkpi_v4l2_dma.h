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

#ifndef _LKPI_V4L2_DMA_H_
#define	_LKPI_V4L2_DMA_H_

/*
 * The DMA API videobuf2-dma-contig needs beyond LinuxKPI's.  Memory from
 * dma_alloc_attrs() is physically contiguous here (LinuxKPI allocates it
 * with kmem_alloc_contig()), which makes mmap and scatter tables simple.
 * Non-coherent allocations (V4L2_MEMORY_FLAG_NON_COHERENT) are refused:
 * videobuf2 then fails that request, which FFmpeg doesn't make.
 */

#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>

int	dma_mmap_attrs(struct device *dev, struct vm_area_struct *vma,
	    void *cpu_addr, dma_addr_t dma_addr, size_t size,
	    unsigned long attrs);
int	dma_get_sgtable_attrs(struct device *dev, struct sg_table *sgt,
	    void *cpu_addr, dma_addr_t dma_addr, size_t size,
	    unsigned long attrs);

static inline struct sg_table *
dma_alloc_noncontiguous(struct device *dev, size_t size,
    enum dma_data_direction dir, gfp_t gfp, unsigned long attrs)
{
	return (NULL);
}

static inline void
dma_free_noncontiguous(struct device *dev, size_t size, struct sg_table *sgt,
    enum dma_data_direction dir)
{
}

static inline void *
dma_vmap_noncontiguous(struct device *dev, size_t size, struct sg_table *sgt)
{
	return (NULL);
}

static inline void
dma_vunmap_noncontiguous(struct device *dev, void *vaddr)
{
}

static inline int
dma_mmap_noncontiguous(struct device *dev, struct vm_area_struct *vma,
    size_t size, struct sg_table *sgt)
{
	return (-ENXIO);
}

/* No virtually aliased caches on arm64 (nor amd64): nothing to do. */
#define	flush_kernel_vmap_range(addr, size)	do { } while (0)
#define	invalidate_kernel_vmap_range(addr, size) do { } while (0)

#ifndef __pfn_to_phys
#define	__pfn_to_phys(pfn)	((phys_addr_t)(pfn) << PAGE_SHIFT)
#endif

#endif /* _LKPI_V4L2_DMA_H_ */
