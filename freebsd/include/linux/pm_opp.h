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

#ifndef _VPU_LINUX_PM_OPP_H_
#define	_VPU_LINUX_PM_OPP_H_

/*
 * Operating performance points, for Qualcomm's Iris: one, at the clock rate
 * the glue sets (iris_freebsd.c).
 */
#include <linux/types.h>
#include <linux/err.h>
#include <linux/cleanup.h>

struct device;
struct dev_pm_opp;
struct opp_table;

typedef int (*config_clks_t)(struct device *dev, struct opp_table *opp_table,
    struct dev_pm_opp *opp, void *data, bool scaling_down);

struct dev_pm_opp_config {
	const char * const	*clk_names;
	config_clks_t		config_clks;
	const char		*prop_name;
	const unsigned int	*supported_hw;
	unsigned int		supported_hw_count;
	const char * const	*regulator_names;
	struct device		***virt_devs;
};

int	dev_pm_opp_config_clks_simple(struct device *dev,
	    struct opp_table *opp_table, struct dev_pm_opp *opp, void *data,
	    bool scaling_down);
int	devm_pm_opp_set_config(struct device *dev,
	    struct dev_pm_opp_config *config);
int	devm_pm_opp_of_add_table(struct device *dev);
int	dev_pm_opp_set_opp(struct device *dev, struct dev_pm_opp *opp);
struct dev_pm_opp *devfreq_recommended_opp(struct device *dev,
	    unsigned long *freq, u32 flags);

static inline void
dev_pm_opp_put(struct dev_pm_opp *opp)
{
}

DEFINE_FREE(put_opp, struct dev_pm_opp *,
    if (!IS_ERR_OR_NULL(_T)) dev_pm_opp_put(_T))

#endif /* _VPU_LINUX_PM_OPP_H_ */
