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

/*
 * The DMA API videobuf2-dma-contig needs beyond LinuxKPI's; see
 * lkpi_v4l2_dma.h.
 */

#include <sys/param.h>
#include <vm/vm.h>
#include <vm/pmap.h>

#include "lkpi_v4l2_dma.h"

/* Map memory from dma_alloc_attrs() into a process, as Linux's direct map. */
int
dma_mmap_attrs(struct device *dev, struct vm_area_struct *vma,
    void *cpu_addr, dma_addr_t dma_addr, size_t size, unsigned long attrs)
{
	unsigned long len, pages;

	len = vma->vm_end - vma->vm_start;
	pages = PAGE_ALIGN(size) >> PAGE_SHIFT;
	if (vma->vm_pgoff >= pages ||
	    (len >> PAGE_SHIFT) > pages - vma->vm_pgoff)
		return (-ENXIO);
	return (remap_pfn_range(vma, vma->vm_start,
	    (vtophys(cpu_addr) >> PAGE_SHIFT) + vma->vm_pgoff, len,
	    vma->vm_page_prot));
}

/* One entry: the memory is physically contiguous. */
int
dma_get_sgtable_attrs(struct device *dev, struct sg_table *sgt,
    void *cpu_addr, dma_addr_t dma_addr, size_t size, unsigned long attrs)
{
	int error;

	error = sg_alloc_table(sgt, 1, GFP_KERNEL);
	if (error != 0)
		return (error);
	sg_set_page(sgt->sgl, virt_to_page(cpu_addr), PAGE_ALIGN(size), 0);
	return (0);
}
