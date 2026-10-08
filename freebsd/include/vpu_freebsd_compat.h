/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 James Kane
 *
 * Included before every source file of lkpi_v4l2.ko and amvx.ko: what
 * LinuxKPI (and drm-kmod's additions) lack for Linux 7.0's V4L2 core,
 * videobuf2 and the mvx driver.
 */
#ifndef _VPU_FREEBSD_COMPAT_H_
#define	_VPU_FREEBSD_COMPAT_H_
/* Not for the FreeBSD bus glue (amvx_freebsd_bus.c), which is FreeBSD's. */
#ifndef AMVX_FBSD_BUS

/*
 * No Linux headers here: a source file's own pr_fmt must come first.
 * Macros expand where used, after the file's includes.
 */
#include <sys/types.h>
#include <sys/ioccom.h>

/*
 * Linux 7.0's typed allocators (include/linux/slab.h) that LinuxKPI lacks
 * (it has the kzalloc_ ones and default_gfp()), without the __counted_by
 * bookkeeping.
 */
#define	__vpu_alloc_objs(ALLOC, GFP, TYPE, COUNT)			\
	((TYPE *)ALLOC(sizeof(TYPE) * (size_t)(COUNT), GFP))
#define	__vpu_alloc_flex(ALLOC, GFP, TYPE, FAM, COUNT)			\
	((TYPE *)ALLOC(sizeof(TYPE) +					\
	    sizeof(((TYPE *)0)->FAM[0]) * (size_t)(COUNT), GFP))
#define	kvmalloc_objs(P, COUNT, ...)	\
	__vpu_alloc_objs(kvmalloc, default_gfp(__VA_ARGS__), typeof(P), COUNT)
#define	kvzalloc_obj(P, ...)	\
	__vpu_alloc_objs(kvzalloc, default_gfp(__VA_ARGS__), typeof(P), 1)
#define	kvzalloc_objs(P, COUNT, ...)	\
	__vpu_alloc_objs(kvzalloc, default_gfp(__VA_ARGS__), typeof(P), COUNT)
#define	kvzalloc_flex(P, FAM, COUNT, ...)	\
	__vpu_alloc_flex(kvzalloc, default_gfp(__VA_ARGS__), typeof(P), FAM, COUNT)

#ifndef EBADR
#define	EBADR		53	/* Linux's "invalid request descriptor" */
#endif

/* The 32-bit time types (include/vdso/time32.h). */
struct old_timespec32 {
	int32_t	tv_sec;
	int32_t	tv_nsec;
};
struct old_timeval32 {
	int32_t	tv_sec;
	int32_t	tv_usec;
};
/* include/uapi/linux/time_types.h */
struct __kernel_timespec {
	int64_t	tv_sec;
	int64_t	tv_nsec;
};

/*
 * Linux's ioctl number decoding (include/uapi/asm-generic/ioctl.h), for
 * FreeBSD's encoding, which LinuxKPI's _IOR() and friends produce: the
 * direction bits are FreeBSD's (IOC_IN, the caller writes; IOC_OUT, it
 * reads), the size IOCPARM_LEN().
 */
#ifndef _IOC_NONE
#define	_IOC_NONE	0U
#define	_IOC_WRITE	1U
#define	_IOC_READ	2U
#endif
#ifndef _IOC_DIR
#define	_IOC_DIR(nr)	((((nr) & IOC_IN) != 0 ? _IOC_WRITE : 0) |	\
			 (((nr) & IOC_OUT) != 0 ? _IOC_READ : 0))
#endif
#ifndef _IOC_SIZEMASK
#define	_IOC_SIZEMASK	IOCPARM_MASK
#define	_IOC_SIZESHIFT	16
#endif

/* No 32-bit processes on this port. */
#define	in_compat_syscall()	false

/* The Linux the media core and the driver come from (7.0.0); LinuxKPI
 * defines KERNEL_VERSION() itself. */
#define	LINUX_VERSION_CODE	((7 << 16) + (0 << 8) + 0)

/* For camera sensors (I2C, SPI, firmware graphs), which V4L2's headers
 * declare helpers for; a codec uses none of them. */
struct i2c_adapter;
struct i2c_board_info;
struct i2c_client;
#ifndef I2C_CLIENT_END
#define	I2C_CLIENT_END	0xfffeU
#endif
struct dentry;
struct fwnode_endpoint {
	unsigned int port;
	unsigned int id;
	const struct fwnode_handle *local_fwnode;
};

/* Kernel virtual mappings and dirty pages (include/linux/vmalloc.h, mm.h). */
#define	vm_map_ram(pages, count, node)	\
	vmap((pages), (count), VM_MAP, PAGE_KERNEL)
#define	vm_unmap_ram(addr, count)	vunmap(addr)
#define	set_page_dirty_lock(page)	set_page_dirty(page)

/* dma-buf: drm-kmod's has no _unlocked variants (it takes no resv lock). */
#define	dma_buf_vmap_unlocked(dmabuf, map)	dma_buf_vmap(dmabuf, map)
#define	dma_buf_vunmap_unlocked(dmabuf, map)	dma_buf_vunmap(dmabuf, map)
#ifndef IOSYS_MAP_INIT_VADDR
#define	IOSYS_MAP_INIT_VADDR(vaddr_)	{ .vaddr = (vaddr_), .is_iomem = false }
#endif

/* poll (include/linux/poll.h): no event mask to honour here. */
#ifndef DEFAULT_POLLMASK
#define	DEFAULT_POLLMASK	(POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM)
#endif
#define	poll_requested_events(p)	((__poll_t)~0)

#ifndef subsys_initcall
#define	subsys_initcall(fn)	module_init(fn)
#endif
#define	MODULE_ALIAS_CHARDEV_MAJOR(major)

/* lkpi_v4l2.ko's module parameters live under hw.lkpi_v4l2. */
#ifdef LKPI_V4L2_MODULE
#include <sys/sysctl.h>
SYSCTL_DECL(_hw_lkpi_v4l2);
#endif

#endif /* !AMVX_FBSD_BUS */
#endif /* _VPU_FREEBSD_COMPAT_H_ */
