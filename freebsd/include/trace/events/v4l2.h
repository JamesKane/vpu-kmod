/* SPDX-License-Identifier: BSD-2-Clause */
/* No tracepoints on FreeBSD: V4L2's buffer events compile to nothing. */
#ifndef _VPU_TRACE_EVENTS_V4L2_H_
#define	_VPU_TRACE_EVENTS_V4L2_H_
#define	trace_v4l2_qbuf(...)		do { } while (0)
#define	trace_v4l2_dqbuf(...)		do { } while (0)
#endif
