/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Reset controls (include/linux/reset.h), as much as the mvx driver uses:
 * the codec's reset on the Sky1's reset controller, by the glue
 * (amvx_freebsd.c).
 */
#ifndef _VPU_LINUX_RESET_H_
#define	_VPU_LINUX_RESET_H_
struct device;
struct reset_control;
struct reset_control *devm_reset_control_get(struct device *dev,
    const char *id);
int	reset_control_assert(struct reset_control *rstc);
int	reset_control_deassert(struct reset_control *rstc);
int	reset_control_reset(struct reset_control *rstc);

/* Bulk (Iris): resets the glue pulses itself with the clocks. */
struct reset_control_bulk_data {
	const char		*id;
	struct reset_control	*rstc;
};
int	devm_reset_control_bulk_get_exclusive(struct device *dev, int num_rstcs,
	    struct reset_control_bulk_data *rstcs);
int	reset_control_bulk_reset(int num_rstcs,
	    struct reset_control_bulk_data *rstcs);
int	reset_control_bulk_assert(int num_rstcs,
	    struct reset_control_bulk_data *rstcs);
int	reset_control_bulk_deassert(int num_rstcs,
	    struct reset_control_bulk_data *rstcs);
#endif
