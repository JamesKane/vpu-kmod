/* SPDX-License-Identifier: BSD-2-Clause */
/* No tracepoints on FreeBSD: videobuf2's buffer events compile to nothing. */
#ifndef _VPU_TRACE_EVENTS_VB2_H_
#define	_VPU_TRACE_EVENTS_VB2_H_
#define	trace_vb2_buf_done(...)		do { } while (0)
#define	trace_vb2_buf_queue(...)	do { } while (0)
#define	trace_vb2_qbuf(...)		do { } while (0)
#define	trace_vb2_dqbuf(...)		do { } while (0)
#endif
