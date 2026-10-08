/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * devfreq: not on FreeBSD (the mvx driver's devfreq code is not built);
 * its device pointer stays NULL, and these are what is left of it.
 */
#ifndef _VPU_LINUX_DEVFREQ_H_
#define	_VPU_LINUX_DEVFREQ_H_
struct devfreq;
struct devfreq_dev_status;
static inline int devfreq_suspend_device(struct devfreq *df) { return 0; }
static inline int devfreq_resume_device(struct devfreq *df) { return 0; }
#endif
