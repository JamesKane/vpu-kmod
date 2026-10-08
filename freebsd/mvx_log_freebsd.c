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
 * The mvx driver's logging, in place of mvx_log.c and mvx_log_group.c:
 * those configure log groups and drains (dmesg, a RAM ring, ftrace,
 * files) through debugfs, walking dentries LinuxKPI's debugfs does not
 * have.  Here the driver's five log groups print to the kernel's message
 * buffer, each at a severity set by sysctl hw.amvx.log.<group> (0 panic
 * to 5 verbose), with the defaults the driver gives them.  No RAM ring,
 * no binary data dumps, no performance statistics.
 */

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/stdarg.h>

#include "mvx_log.h"
#include "mvx_log_group.h"

struct mvx_log_group mvx_log_if;
struct mvx_log_group mvx_log_fwif_if;
struct mvx_log_group mvx_log_perf;
struct mvx_log_group mvx_log_session_if;
struct mvx_log_group mvx_log_dev;

static const char *const mvx_fbsd_severity[] = {
	[MVX_LOG_PANIC] = "panic",
	[MVX_LOG_ERROR] = "error",
	[MVX_LOG_WARNING] = "warning",
	[MVX_LOG_INFO] = "info",
	[MVX_LOG_DEBUG] = "debug",
	[MVX_LOG_VERBOSE] = "verbose",
};

static void
mvx_fbsd_print(struct mvx_log_drain *drain, enum mvx_log_severity severity,
    const char *tag, const char *fmt, const unsigned int n_args, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, n_args);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printf("amvx: %s %s: %s\n", tag,
	    severity < MVX_LOG_MAX ? mvx_fbsd_severity[severity] : "?", buf);
}

static void
mvx_fbsd_data(struct mvx_log_drain *drain, enum mvx_log_severity severity,
    struct iovec *vec, size_t count)
{
}

static void
mvx_fbsd_reset(struct mvx_log_drain *drain)
{
}

static struct mvx_log_drain mvx_fbsd_drain = {
	.print = mvx_fbsd_print,
	.data = mvx_fbsd_data,
	.reset = mvx_fbsd_reset,
};

static void
mvx_fbsd_group(struct mvx_log_group *group, const char *tag,
    enum mvx_log_severity severity)
{
	memset(group, 0, sizeof(*group));
	group->tag = tag;
	group->severity = severity;
	group->drain = &mvx_fbsd_drain;
	mutex_init(&group->mutex);
}

SYSCTL_NODE(_hw_amvx, OID_AUTO, log, CTLFLAG_RW | CTLFLAG_MPSAFE, 0,
    "Log severities: 0 panic, 1 error, 2 warning, 3 info, 4 debug, "
    "5 verbose");
SYSCTL_UINT(_hw_amvx_log, OID_AUTO, if, CTLFLAG_RWTUN,
    &mvx_log_if.severity, 0, "Interface");
SYSCTL_UINT(_hw_amvx_log, OID_AUTO, fwif, CTLFLAG_RWTUN,
    &mvx_log_fwif_if.severity, 0, "Firmware interface");
SYSCTL_UINT(_hw_amvx_log, OID_AUTO, session, CTLFLAG_RWTUN,
    &mvx_log_session_if.severity, 0, "Sessions");
SYSCTL_UINT(_hw_amvx_log, OID_AUTO, dev, CTLFLAG_RWTUN,
    &mvx_log_dev.severity, 0, "Device");
SYSCTL_UINT(_hw_amvx_log, OID_AUTO, perf, CTLFLAG_RWTUN,
    &mvx_log_perf.severity, 0, "Performance");

int
mvx_log_group_init(const char *entry_name)
{
	/* As mvx_log_group.c's defaults. */
	mvx_fbsd_group(&mvx_log_if, "MVX if", MVX_LOG_WARNING);
	mvx_fbsd_group(&mvx_log_fwif_if, "MVX fwif", MVX_LOG_INFO);
	mvx_fbsd_group(&mvx_log_session_if, "MVX session", MVX_LOG_WARNING);
	mvx_fbsd_group(&mvx_log_dev, "MVX dev", MVX_LOG_WARNING);
	mvx_fbsd_group(&mvx_log_perf, "MVX perf", MVX_LOG_INFO);
	return 0;
}

void
mvx_log_group_deinit(void)
{
	mutex_destroy(&mvx_log_if.mutex);
	mutex_destroy(&mvx_log_fwif_if.mutex);
	mutex_destroy(&mvx_log_session_if.mutex);
	mutex_destroy(&mvx_log_dev.mutex);
	mutex_destroy(&mvx_log_perf.mutex);
}

void
mvx_log_session_memory_record(struct mvx_log_group *group,
    struct mvx_log_memory_stats *stats, int *write_idx)
{
}
