/* SPDX-License-Identifier: BSD-2-Clause */
/* No freezer on FreeBSD: kernel threads are never frozen for suspend. */
#ifndef _VPU_LINUX_FREEZER_H_
#define	_VPU_LINUX_FREEZER_H_
#define	set_freezable()		do { } while (0)
#define	try_to_freeze()		(false)
#define	freezing(p)		(false)
#endif
