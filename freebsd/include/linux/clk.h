/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Clocks (include/linux/clk.h), as much as the mvx driver uses: SCMI
 * clocks named by the device's ACPI CLKT, by the glue (amvx_freebsd.c).
 * lkpi_v4l2.ko only sees the type.
 */
#ifndef _VPU_LINUX_CLK_H_
#define	_VPU_LINUX_CLK_H_
struct clk;
struct device;
struct clk *devm_clk_get(struct device *dev, const char *id);
struct clk *devm_clk_get_optional(struct device *dev, const char *id);
int	clk_prepare_enable(struct clk *clk);
void	clk_disable_unprepare(struct clk *clk);
unsigned long clk_get_rate(struct clk *clk);
int	clk_set_rate(struct clk *clk, unsigned long rate);
#endif
