/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 James Kane
 *
 * Included before each of the mvx driver's files (after
 * vpu_freebsd_compat.h): the Linux headers its files take for granted,
 * and the few calls LinuxKPI lacks.  The driver sets no pr_fmt, so Linux
 * headers may come first here.
 */
#ifndef _AMVX_FREEBSD_H_
#define	_AMVX_FREEBSD_H_
/* Not for the FreeBSD bus glue (amvx_freebsd_bus.c), which is FreeBSD's. */
#ifndef AMVX_FBSD_BUS

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/kobject.h>
#include <linux/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <asm/io.h>

/* Linux 7.0's kobject actions; LinuxKPI's uevents take an int. */
#ifndef KOBJ_ADD
#define	KOBJ_ADD	0
#endif

/* timespec64 is FreeBSD's timespec in LinuxKPI. */
static inline struct timespec64
timespec64_sub(struct timespec64 a, struct timespec64 b)
{
	struct timespec64 r;

	timespecsub(&a, &b, &r);
	return (r);
}

static inline int
timespec64_compare(const struct timespec64 *a, const struct timespec64 *b)
{
	if (a->tv_sec != b->tv_sec)
		return (a->tv_sec < b->tv_sec ? -1 : 1);
	if (a->tv_nsec != b->tv_nsec)
		return (a->tv_nsec < b->tv_nsec ? -1 : 1);
	return (0);
}

/* Re-arm a timer only if it is pending (kernel/time/timer.c). */
static inline int
mod_timer_pending(struct timer_list *timer, unsigned long expires)
{
	if (!timer_pending(timer))
		return (0);
	return (mod_timer(timer, expires));
}

#ifndef phys_to_virt
#define	phys_to_virt(pa)	((void *)PHYS_TO_DMAP((vm_paddr_t)(pa)))
#endif

/*
 * A scatter table's pages mapped into the kernel (kernel/dma/mapping.c's
 * dma_vmap_noncontiguous(), for a table of whole pages): the codec's MMU
 * page tables.
 */
static inline void *
dma_vmap_noncontiguous(struct device *dev, size_t size, struct sg_table *sgt)
{
	unsigned int count = PAGE_ALIGN(size) >> PAGE_SHIFT, i = 0;
	struct scatterlist *sg;
	struct page **pages;
	unsigned int n;
	void *vaddr;

	pages = kmalloc_array(count, sizeof(*pages), GFP_KERNEL);
	if (pages == NULL)
		return (NULL);
	for_each_sgtable_sg(sgt, sg, n) {
		if (i < count)
			pages[i++] = sg_page(sg);
	}
	vaddr = i == count ? vmap(pages, count, VM_MAP, PAGE_KERNEL) : NULL;
	kfree(pages);
	return (vaddr);
}

static inline void
dma_vunmap_noncontiguous(struct device *dev, void *vaddr)
{
	vunmap(vaddr);
}

/*
 * The FreeBSD bus glue owns the register windows: a token in place of
 * the resource Linux's request_mem_region() reserves.
 */
static inline struct resource *
mvx_fbsd_request_mem_region(resource_size_t start, resource_size_t n,
    const char *name)
{
	struct resource *r;

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (r != NULL) {
		r->start = start;
		r->end = start + n - 1;
		r->name = name;
	}
	return (r);
}
#define	request_mem_region(start, n, name)	\
	mvx_fbsd_request_mem_region((start), (n), (name))
#define	release_mem_region(start, n)	do { } while (0)

/* ACPI _DSD properties, by the glue (amvx_freebsd.c). */
int	device_property_read_u32_array(struct device *dev,
	    const char *propname, u32 *val, size_t nval);

/*
 * The codec is an ACPI device (the glue attaches it), which keeps the
 * driver off its devicetree paths.
 */
#define	has_acpi_companion(dev)	true

/* Runtime PM: LinuxKPI's, which runs the driver's callbacks. */
#ifndef SET_RUNTIME_PM_OPS
#define	SET_RUNTIME_PM_OPS(s, r, i)	RUNTIME_PM_OPS(s, r, i)
#endif
/* No generic power domains: the glue powers the codec's cores. */
#define	dev_pm_domain_detach(dev, power_off)	do { } while (0)
#include <sys/bus.h>
#include "amvx_freebsd_bus.h"	/* amvx_fbsd_core_power() */

/* LinuxKPI's struct device has no id: one codec, number 0. */
#define	MVX_DEV_ID(dev)	0

/* sysctls under hw.amvx. */
#include <sys/sysctl.h>
SYSCTL_DECL(_hw_amvx);

#endif /* !AMVX_FBSD_BUS */
#endif /* _AMVX_FREEBSD_H_ */
