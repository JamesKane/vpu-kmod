/* SPDX-License-Identifier: BSD-2-Clause */
/* Generic power domains: the glue powers the codec and its cores. */
#ifndef _VPU_LINUX_PM_DOMAIN_H_
#define	_VPU_LINUX_PM_DOMAIN_H_

/*
 * Lists of power domains (Iris): one stand-in device per domain name; the
 * glue powers the domains with the clocks.
 */
#include <linux/types.h>

struct device;
struct device_link;

#define	PD_FLAG_NO_DEV_LINK	(1u << 0)
#define	PD_FLAG_DEV_LINK_ON	(1u << 1)
#define	PD_FLAG_REQUIRED_OPP	(1u << 2)

struct dev_pm_domain_attach_data {
	const char * const	*pd_names;
	u32			num_pd_names;	/* Iris lowers it */
	u32			pd_flags;
};

struct dev_pm_domain_list {
	struct device		**pd_devs;
	struct device_link	**pd_links;
	u32			*opp_tokens;
	u32			num_pds;
};

int	devm_pm_domain_attach_list(struct device *dev,
	    const struct dev_pm_domain_attach_data *data,
	    struct dev_pm_domain_list **list);
int	dev_pm_genpd_set_hwmode(struct device *dev, bool enable);
#endif
