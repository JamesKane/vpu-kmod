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
 * The Linux side of the glue: the platform device the mvx driver attaches
 * to ("amvx_dev": the RCSU's registers, the codec's, its interrupt), and
 * what the driver needs that LinuxKPI lacks: _DSD properties, the codec's
 * clock (SCMI, named by its ACPI CLKT) and its reset (the Sky1's reset
 * controller), and module parameter descriptions.
 */

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/sysctl.h>
#include <vm/vm.h>
#include <vm/pmap.h>

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/pci.h>		/* IORESOURCE_MEM, as LinuxKPI's platform bus */
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/reset.h>

#include <arm64/cix/sky1_scmi.h>

#include "amvx_freebsd_bus.h"

SYSCTL_NODE(_hw, OID_AUTO, amvx, CTLFLAG_RW | CTLFLAG_MPSAFE, 0,
    "Mali-V video codec (mvx)");

/*
 * The driver's module parameters (hw.amvx.*): drm-kmod's module_param()
 * wants a description for each, which CIX's leave out.
 */
const char linuxkpi_amvx_busctrl_ref_desc[] = "Bus control: reference";
const char linuxkpi_amvx_busctrl_split_desc[] = "Bus control: split";
const char linuxkpi_amvx_disable_dfs_desc[] = "No frequency scaling";
const char linuxkpi_amvx_fw_watchdog_timeout_desc[] =
    "Firmware watchdog (ms)";
const char linuxkpi_amvx_session_watchdog_timeout_desc[] =
    "Session watchdog (ms)";
const char linuxkpi_amvx_sw_core_mask_desc[] = "Cores to use (mask)";
const char linuxkpi_amvx_wait_pending_timeout_desc[] =
    "Wait for pending buffers (ms)";
const char linuxkpi_amvx_wait_scheduler_timeout_desc[] =
    "Wait for the scheduler (ms)";

/* Properties (ACPI _DSD) */

int
device_property_read_u32_array(struct device *dev, const char *propname,
    u32 *val, size_t nval)
{
	int n;

	n = amvx_fbsd_get_u32_prop(dev->bsddev, propname, val, nval);
	if (n < 0)
		return (-EINVAL);
	return (n < nval ? -EOVERFLOW : 0);
}

/* The codec's clock: an SCMI clock its ACPI CLKT names ("vpu_clk"). */

struct clk {
	uint32_t	id;
	bool		on;
};

struct clk *
devm_clk_get_optional(struct device *dev, const char *id)
{
	struct clk *clk;
	uint32_t scmi_id;

	if (id == NULL || amvx_fbsd_clk_id(id, &scmi_id) != 0)
		return (NULL);
	clk = devm_kzalloc(dev, sizeof(*clk), GFP_KERNEL);
	if (clk == NULL)
		return (ERR_PTR(-ENOMEM));
	clk->id = scmi_id;
	return (clk);
}

struct clk *
devm_clk_get(struct device *dev, const char *id)
{
	struct clk *clk;

	clk = devm_clk_get_optional(dev, id);
	return (clk != NULL ? clk : ERR_PTR(-ENOENT));
}

int
clk_prepare_enable(struct clk *clk)
{
	int error;

	if (clk == NULL || clk->on)
		return (0);
	error = sky1_scmi_clk_enable(clk->id, true);
	if (error == 0)
		clk->on = true;
	return (-error);
}

void
clk_disable_unprepare(struct clk *clk)
{
	if (clk == NULL || !clk->on)
		return;
	if (sky1_scmi_clk_enable(clk->id, false) == 0)
		clk->on = false;
}

unsigned long
clk_get_rate(struct clk *clk)
{
	uint64_t hz;

	if (clk == NULL || sky1_scmi_clk_get_rate(clk->id, &hz) != 0)
		return (0);
	return (hz);
}

int
clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (clk == NULL)
		return (0);
	return (-sky1_scmi_clk_set_rate(clk->id, rate));
}

/*
 * The codec's reset ("vpu_reset", RSTL index 14): a bit of the Sky1's
 * reset controller (ACPI RST0), active low; Linux's reset-sky1.c's
 * SKY1_VPU_RESET_N.
 */

#define	SKY1_RST_PA		0x16000000UL
#define	 RST_S0_DOMAIN_0	0x400
#define	  RST_VPU		(1u << 11)

struct reset_control {
	uint32_t	offset;
	uint32_t	bit;
};

static void
amvx_fbsd_reset_set(struct reset_control *rstc, bool assert)
{
	volatile uint32_t *r;

	r = pmap_mapdev(SKY1_RST_PA + rstc->offset, sizeof(*r));
	if (assert)
		*r &= ~rstc->bit;
	else
		*r |= rstc->bit;
	pmap_unmapdev(__DEVOLATILE(void *, r), sizeof(*r));
}

struct reset_control *
devm_reset_control_get(struct device *dev, const char *id)
{
	struct reset_control *rstc;

	if (id == NULL || strcmp(id, "vpu_reset") != 0)
		return (ERR_PTR(-ENOENT));
	rstc = devm_kzalloc(dev, sizeof(*rstc), GFP_KERNEL);
	if (rstc == NULL)
		return (ERR_PTR(-ENOMEM));
	rstc->offset = RST_S0_DOMAIN_0;
	rstc->bit = RST_VPU;
	return (rstc);
}

int
reset_control_assert(struct reset_control *rstc)
{
	if (rstc != NULL)
		amvx_fbsd_reset_set(rstc, true);
	return (0);
}

int
reset_control_deassert(struct reset_control *rstc)
{
	if (rstc != NULL)
		amvx_fbsd_reset_set(rstc, false);
	return (0);
}

int
reset_control_reset(struct reset_control *rstc)
{
	reset_control_assert(rstc);
	udelay(20);
	return (reset_control_deassert(rstc));
}

/* The platform device, for the FreeBSD device amvx_freebsd_bus.c attached */

static struct platform_device *amvx_fbsd_pdev;

static void
amvx_fbsd_pdev_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	kfree(pdev->resource);
	kfree(pdev);
}

int
amvx_fbsd_linux_attach(device_t dev, uint64_t rcsu_pa, uint64_t rcsu_size,
    uint64_t pa, uint64_t size, int irq)
{
	struct platform_device *pdev;
	struct resource *res;
	int error;

	linux_set_current(curthread);
	pdev = kzalloc(sizeof(*pdev), GFP_KERNEL);
	res = kcalloc(3, sizeof(*res), GFP_KERNEL);
	/* As the driver's probe takes them: RCSU, codec, interrupt. */
	res[0].start = rcsu_pa;
	res[0].end = rcsu_pa + rcsu_size - 1;
	res[0].flags = IORESOURCE_MEM;
	res[1].start = pa;
	res[1].end = pa + size - 1;
	res[1].flags = IORESOURCE_MEM;
	/* LinuxKPI's request_irq() takes FreeBSD interrupt numbers. */
	res[2].start = res[2].end = irq;
	res[2].flags = IORESOURCE_IRQ;
	pdev->name = "amvx_dev";
	pdev->id = PLATFORM_DEVID_NONE;
	pdev->resource = res;
	pdev->num_resources = 3;
	/* Not behind an SMMU (no IORT node): the ACPI bus's DMA tag. */
	pdev->dev.bsddev = dev;
	pdev->dev.release = amvx_fbsd_pdev_release;
	error = platform_device_register(pdev);
	if (error != 0) {
		platform_device_put(pdev);
		return (error);
	}
	amvx_fbsd_pdev = pdev;
	return (0);
}

void
amvx_fbsd_linux_detach(void)
{
	linux_set_current(curthread);
	if (amvx_fbsd_pdev != NULL)
		platform_device_unregister(amvx_fbsd_pdev);
	amvx_fbsd_pdev = NULL;
}
