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
 * The Linux side of the Iris glue: the platform device Qualcomm's Iris
 * driver attaches to, and what it gets from Linux's devicetree, clock,
 * reset, power domain, OPP and interconnect frameworks and Qualcomm's
 * platform drivers, here from the FreeBSD side (iris_freebsd_bus.c):
 *
 * - Clocks, resets and power domains, as Iris's power_on_controller and
 *   power_on_hw enable them, map onto qcom_videocc(4): the bus clock
 *   ("iface", the first Iris enables) brings the controller up with its
 *   power domain and resets, the core clock ("vcodec0_core") the core with
 *   its power domain.  The others are no-ops.
 * - Operating points: the core clock's levels (240 to 560 MHz), with their
 *   rails, are qcom_videocc(4)'s.
 * - Bandwidth: the codec's path to memory (MM1) in kB/s as Iris votes it.
 * - The SC8280XP's UBWC configuration and firmware memory.
 * - Firmware images (.mbn) loaded through TrustZone, as msm's glue does.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/elf32.h>

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include <linux/interconnect.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pci.h>		/* IORESOURCE_MEM, as LinuxKPI's platform bus */
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/pm_opp.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/soc/qcom/mdt_loader.h>
#include <linux/soc/qcom/ubwc.h>

#include <dev/qcom_scm/qcom_scm.h>

#include "iris_freebsd_bus.h"

/* What the devicetree would say. */
#define	IRIS_FBSD_COMPATIBLE	"qcom,sc8280xp-iris"
#define	IRIS_FBSD_FW_MEM	0x8c600000ul	/* video-region */
#define	IRIS_FBSD_FW_MEM_SIZE	0x700000ul

/* Devicetree */

const void *
of_device_get_match_data(const struct device *dev)
{
	const struct of_device_id *id;

	if (dev->driver == NULL || dev->driver->of_match_table == NULL)
		return (NULL);
	for (id = dev->driver->of_match_table; id->compatible[0] != '\0'; id++)
		if (strcmp(id->compatible, IRIS_FBSD_COMPATIBLE) == 0)
			return (id->data);
	return (NULL);
}

int
of_reserved_mem_region_to_resource(const struct device_node *np,
    unsigned int idx, struct resource *res)
{
	if (idx != 0)
		return (-ENODEV);
	memset(res, 0, sizeof(*res));
	res->start = IRIS_FBSD_FW_MEM;
	res->end = IRIS_FBSD_FW_MEM + IRIS_FBSD_FW_MEM_SIZE - 1;
	res->flags = IORESOURCE_MEM;
	return (0);
}

/* UBWC, as Linux's ubwc_config.c has it for the SC8280XP. */

static const struct qcom_ubwc_cfg_data iris_fbsd_ubwc = {
	.ubwc_enc_version = UBWC_4_0,
	.ubwc_dec_version = UBWC_4_0,
	.ubwc_swizzle = UBWC_SWIZZLE_ENABLE_LVL2 | UBWC_SWIZZLE_ENABLE_LVL3,
	.ubwc_bank_spread = true,
	.highest_bank_bit = 16,
	.macrotile_mode = true,
};

const struct qcom_ubwc_cfg_data *
qcom_ubwc_config_get_data(void)
{
	return (&iris_fbsd_ubwc);
}

/* Clocks */

enum iris_fbsd_clk_kind {
	IRIS_FBSD_CLK_BUS,	/* "iface": the controller, powered and reset */
	IRIS_FBSD_CLK_CTRL,	/* "core": with the bus clock */
	IRIS_FBSD_CLK_CORE,	/* "vcodec0_core": the core, powered */
};

struct clk {
	enum iris_fbsd_clk_kind	kind;
};

static struct clk iris_fbsd_clks[] = {
	{ IRIS_FBSD_CLK_BUS }, { IRIS_FBSD_CLK_CTRL }, { IRIS_FBSD_CLK_CORE },
};
static const char *const iris_fbsd_clk_names[] = {
	"iface", "core", "vcodec0_core",
};

int
devm_clk_bulk_get_all(struct device *dev, struct clk_bulk_data **clks)
{
	struct clk_bulk_data *c;
	int i;

	c = devm_kcalloc(dev, nitems(iris_fbsd_clks), sizeof(*c), GFP_KERNEL);
	if (c == NULL)
		return (-ENOMEM);
	for (i = 0; i < nitems(iris_fbsd_clks); i++) {
		c[i].id = iris_fbsd_clk_names[i];
		c[i].clk = &iris_fbsd_clks[i];
	}
	*clks = c;
	return (nitems(iris_fbsd_clks));
}

int
clk_prepare_enable(struct clk *clk)
{
	if (clk == NULL)
		return (0);
	switch (clk->kind) {
	case IRIS_FBSD_CLK_BUS:
		return (-iris_fbsd_ctrl_enable());
	case IRIS_FBSD_CLK_CORE:
		return (-iris_fbsd_hw_enable());
	default:
		return (0);
	}
}

void
clk_disable_unprepare(struct clk *clk)
{
	if (clk == NULL)
		return;
	switch (clk->kind) {
	case IRIS_FBSD_CLK_BUS:
		iris_fbsd_ctrl_disable();
		break;
	case IRIS_FBSD_CLK_CORE:
		iris_fbsd_hw_disable();
		break;
	default:
		break;
	}
}

unsigned long
clk_get_rate(struct clk *clk)
{
	return (clk != NULL && clk->kind == IRIS_FBSD_CLK_CORE ?
	    iris_fbsd_get_rate() : 0);
}

int
clk_set_rate(struct clk *clk, unsigned long rate)
{
	return (0);
}

/* Resets: pulsed with the controller (qcom_videocc_ctrl_enable()). */

int
devm_reset_control_bulk_get_exclusive(struct device *dev, int num_rstcs,
    struct reset_control_bulk_data *rstcs)
{
	int i;

	for (i = 0; i < num_rstcs; i++)
		rstcs[i].rstc = NULL;
	return (0);
}

int
reset_control_bulk_reset(int num_rstcs, struct reset_control_bulk_data *rstcs)
{
	return (0);
}

int
reset_control_bulk_assert(int num_rstcs, struct reset_control_bulk_data *rstcs)
{
	return (0);
}

int
reset_control_bulk_deassert(int num_rstcs,
    struct reset_control_bulk_data *rstcs)
{
	return (0);
}

/* Power domains: stand-in devices; "vcodec0" is the core's. */

static struct device *iris_fbsd_core_pd;

static void
iris_fbsd_pd_release(struct device *dev)
{
	kfree(dev);
}

int
devm_pm_domain_attach_list(struct device *dev,
    const struct dev_pm_domain_attach_data *data,
    struct dev_pm_domain_list **list)
{
	struct dev_pm_domain_list *l;
	struct device *pd;
	u32 i;

	if (data == NULL || data->num_pd_names == 0)
		return (-ENODEV);
	l = devm_kzalloc(dev, sizeof(*l), GFP_KERNEL);
	if (l == NULL)
		return (-ENOMEM);
	l->pd_devs = devm_kcalloc(dev, data->num_pd_names,
	    sizeof(*l->pd_devs), GFP_KERNEL);
	if (l->pd_devs == NULL)
		return (-ENOMEM);
	for (i = 0; i < data->num_pd_names; i++) {
		pd = kzalloc(sizeof(*pd), GFP_KERNEL);
		if (pd == NULL)
			return (-ENOMEM);
		/* Not device_initialize(): that is for class devices. */
		lkpi_device_init(pd, dev, dev->bsddev);
		kobject_init(&pd->kobj, &linux_dev_ktype);
		pd->release = iris_fbsd_pd_release;
		dev_set_name(pd, "%s", data->pd_names[i]);
		l->pd_devs[i] = pd;
		if (strcmp(data->pd_names[i], "vcodec0") == 0)
			iris_fbsd_core_pd = pd;
	}
	l->num_pds = data->num_pd_names;
	*list = l;
	return (l->num_pds);
}

int
dev_pm_genpd_set_hwmode(struct device *dev, bool enable)
{
	if (dev == NULL || dev != iris_fbsd_core_pd)
		return (-EOPNOTSUPP);
	return (-iris_fbsd_hw_set_hwmode(enable));
}

/*
 * Operating points: an OPP is a level of the core clock, its rate; the
 * clock's level brings its rails along (qcom_videocc(4)).
 */

int
dev_pm_opp_config_clks_simple(struct device *dev, struct opp_table *opp_table,
    struct dev_pm_opp *opp, void *data, bool scaling_down)
{
	return (0);
}

int
devm_pm_opp_set_config(struct device *dev, struct dev_pm_opp_config *config)
{
	return (0);
}

int
devm_pm_opp_of_add_table(struct device *dev)
{
	return (0);
}

struct dev_pm_opp {
	unsigned long	hz;
};

static struct dev_pm_opp iris_fbsd_opps[8];

/* The lowest level at least *freq (Linux's flags 0), or the highest. */
struct dev_pm_opp *
devfreq_recommended_opp(struct device *dev, unsigned long *freq, u32 flags)
{
	unsigned long hz;
	u_int i;

	hz = iris_fbsd_round_rate(*freq);
	if (hz == 0)
		return (ERR_PTR(-ENODEV));
	for (i = 0; i < nitems(iris_fbsd_opps); i++)
		if (iris_fbsd_opps[i].hz == hz || iris_fbsd_opps[i].hz == 0) {
			iris_fbsd_opps[i].hz = hz;
			*freq = hz;
			return (&iris_fbsd_opps[i]);
		}
	return (ERR_PTR(-ENOSPC));
}

int
dev_pm_opp_set_opp(struct device *dev, struct dev_pm_opp *opp)
{
	return (-iris_fbsd_set_rate(opp->hz));
}

/*
 * Interconnect: the path to memory ("video-mem") as Iris votes it; the other,
 * "cpu-cfg", crosses the configuration NoC, which RPMh keeps alive.
 */

int
devm_of_icc_bulk_get(struct device *dev, int num_paths,
    struct icc_bulk_data *paths)
{
	int i;

	for (i = 0; i < num_paths; i++)
		paths[i].path = (struct icc_path *)&paths[i];
	return (0);
}

int
icc_bulk_set_bw(int num_paths, const struct icc_bulk_data *bulk)
{
	int i;

	for (i = 0; i < num_paths; i++)
		if (bulk[i].name != NULL &&
		    strcmp(bulk[i].name, "video-mem") == 0)
			return (-iris_fbsd_bw_vote(bulk[i].avg_bw,
			    bulk[i].peak_bw));
	return (0);
}

/* Firmware images, through TrustZone: msm's glue's loader. */

#define	MDT_TYPE_MASK		(7u << 24)
#define	MDT_TYPE_HASH		(2u << 24)
#define	MDT_RELOCATABLE		(1u << 27)
#define	MDT_MAX_HASH_SIZE	(1024 * 1024)

static bool
mdt_seg_loadable(const Elf32_Phdr *ph)
{
	return (ph->p_type == PT_LOAD &&
	    (ph->p_flags & MDT_TYPE_MASK) != MDT_TYPE_HASH &&
	    ph->p_memsz != 0);
}

static int
mdt_headers(const struct firmware *fw, const Elf32_Ehdr **ehp,
    const Elf32_Phdr **php)
{
	const Elf32_Ehdr *eh = (const void *)fw->data;

	if (fw->size < sizeof(*eh) || !IS_ELF(*eh) ||
	    eh->e_ident[EI_CLASS] != ELFCLASS32 ||
	    eh->e_phentsize != sizeof(Elf32_Phdr) ||
	    eh->e_phoff + (size_t)eh->e_phnum * sizeof(Elf32_Phdr) > fw->size)
		return (-EINVAL);
	*ehp = eh;
	*php = (const void *)(fw->data + eh->e_phoff);
	return (0);
}

static void
mdt_span(const Elf32_Ehdr *eh, const Elf32_Phdr *ph, u64 *min, u64 *max,
    bool *reloc)
{
	int i;

	*min = UINT64_MAX;
	*max = 0;
	*reloc = false;
	for (i = 0; i < eh->e_phnum; i++) {
		if (!mdt_seg_loadable(&ph[i]))
			continue;
		if ((ph[i].p_flags & MDT_RELOCATABLE) != 0)
			*reloc = true;
		*min = MIN(*min, ph[i].p_paddr);
		*max = MAX(*max, roundup2((u64)ph[i].p_paddr + ph[i].p_memsz,
		    PAGE_SIZE));
	}
}

ssize_t
qcom_mdt_get_size(const struct firmware *fw)
{
	const Elf32_Ehdr *eh;
	const Elf32_Phdr *ph;
	u64 min, max;
	bool reloc;

	if (mdt_headers(fw, &eh, &ph) != 0)
		return (-EINVAL);
	mdt_span(eh, ph, &min, &max, &reloc);
	return (min < max ? (ssize_t)(max - min) : -EINVAL);
}

/* Read segment n from its own file, "<name without extension>.bNN". */
static int
mdt_load_split(void *dst, const Elf32_Phdr *ph, int n, const char *fw_name,
    struct device *dev)
{
	const struct firmware *seg;
	char *name;
	size_t len;
	int error;

	len = strlen(fw_name);
	if (len < 4)
		return (-EINVAL);
	name = kstrdup(fw_name, GFP_KERNEL);
	snprintf(name + len - 3, 4, "b%02d", n);
	error = request_firmware(&seg, name, dev);
	if (error == 0) {
		if (seg->size != ph->p_filesz)
			error = -EINVAL;
		else
			memcpy(dst, seg->data, seg->size);
		release_firmware(seg);
	}
	kfree(name);
	return (error);
}

/* Read segment n's contents, from the firmware file or its own. */
static int
mdt_read_seg(void *dst, const struct firmware *fw, const Elf32_Phdr *ph,
    int n, const char *fw_name, struct device *dev)
{
	if (ph->p_filesz == 0)
		return (0);
	if (ph->p_offset < fw->size &&
	    (size_t)ph->p_offset + ph->p_filesz <= fw->size) {
		memcpy(dst, fw->data + ph->p_offset, ph->p_filesz);
		return (0);
	}
	return (mdt_load_split(dst, ph, n, fw_name, dev));
}

static int
mdt_load(struct device *dev, const struct firmware *fw, const char *fw_name,
    int pas_id, bool init, void *mem_region, phys_addr_t mem_phys,
    size_t mem_size, phys_addr_t *reloc_base)
{
	const Elf32_Ehdr *eh;
	const Elf32_Phdr *ph;
	u8 *meta;
	size_t hdr_size, hash_size;
	u64 min, max, base;
	s64 off;
	bool reloc;
	int error, hash, i;

	if ((error = mdt_headers(fw, &eh, &ph)) != 0)
		return (error);

	if (init) {
		/* Metadata: the headers (the first segment) and the hash. */
		for (hash = 0; hash < eh->e_phnum; hash++)
			if ((ph[hash].p_flags & MDT_TYPE_MASK) == MDT_TYPE_HASH)
				break;
		if (hash == eh->e_phnum || ph[0].p_filesz > fw->size)
			return (-EINVAL);
		hdr_size = ph[0].p_filesz;
		hash_size = ph[hash].p_filesz;
		/* A hash segment is a few KB; don't trust a bad file's. */
		if (hash_size > MDT_MAX_HASH_SIZE)
			return (-EINVAL);
		meta = kmalloc(hdr_size + hash_size, GFP_KERNEL);
		if (meta == NULL)
			return (-ENOMEM);
		memcpy(meta, fw->data, hdr_size);
		if (hdr_size + hash_size == fw->size)	/* packed after them */
			memcpy(meta + hdr_size, fw->data + hdr_size, hash_size);
		else
			error = mdt_read_seg(meta + hdr_size, fw, &ph[hash],
			    hash, fw_name, dev);
		if (error == 0)
			error = -qcom_scm_pas_init_image(pas_id, meta,
			    hdr_size + hash_size);
		kfree(meta);
		if (error != 0)
			return (error);
	}

	mdt_span(eh, ph, &min, &max, &reloc);
	if (reloc) {
		if (init) {
			error = -qcom_scm_pas_mem_setup(pas_id, mem_phys,
			    max - min);
			if (error != 0)
				return (error);
		}
		base = min;
	} else
		base = mem_phys;

	for (i = 0; i < eh->e_phnum; i++) {
		if (!mdt_seg_loadable(&ph[i]))
			continue;
		off = (s64)ph[i].p_paddr - (s64)base;
		if (off < 0 || off + ph[i].p_memsz > mem_size)
			return (-EINVAL);
		if (ph[i].p_filesz > ph[i].p_memsz)
			return (-EINVAL);
		error = mdt_read_seg((u8 *)mem_region + off, fw, &ph[i], i,
		    fw_name, dev);
		if (error != 0)
			return (error);
		memset((u8 *)mem_region + off + ph[i].p_filesz, 0,
		    ph[i].p_memsz - ph[i].p_filesz);
	}
	wmb();
	if (reloc_base != NULL)
		*reloc_base = base;
	return (0);
}

int
qcom_mdt_load(struct device *dev, const struct firmware *fw,
    const char *fw_name, int pas_id, void *mem_region, phys_addr_t mem_phys,
    size_t mem_size, phys_addr_t *reloc_base)
{
	return (mdt_load(dev, fw, fw_name, pas_id, true, mem_region, mem_phys,
	    mem_size, reloc_base));
}

int
qcom_mdt_load_no_init(struct device *dev, const struct firmware *fw,
    const char *fw_name, void *mem_region, phys_addr_t mem_phys,
    size_t mem_size, phys_addr_t *reloc_base)
{
	return (mdt_load(dev, fw, fw_name, 0, false, mem_region, mem_phys,
	    mem_size, reloc_base));
}

/* The platform device, for the FreeBSD device iris_freebsd_bus.c attached. */

static struct platform_device *iris_fbsd_pdev;

static void
iris_fbsd_pdev_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	kfree(pdev->resource);
	kfree(pdev);
}

int
iris_fbsd_linux_attach(device_t dev, uint64_t pa, uint64_t size, int irq)
{
	struct platform_device *pdev;
	struct resource *res;
	int error;

	linux_set_current(curthread);
	pdev = kzalloc(sizeof(*pdev), GFP_KERNEL);
	res = kcalloc(2, sizeof(*res), GFP_KERNEL);
	res[0].start = pa;
	res[0].end = pa + size - 1;
	res[0].flags = IORESOURCE_MEM;
	/* LinuxKPI's request_irq() takes FreeBSD interrupt numbers. */
	res[1].start = res[1].end = irq;
	res[1].flags = IORESOURCE_IRQ;
	pdev->name = "qcom-iris";
	pdev->id = PLATFORM_DEVID_NONE;
	pdev->resource = res;
	pdev->num_resources = 2;
	/* Translated by the apps SMMU: the FreeBSD side claimed it. */
	pdev->dev.bsddev = dev;
	pdev->dev.release = iris_fbsd_pdev_release;
	error = platform_device_register(pdev);
	if (error != 0) {
		platform_device_put(pdev);
		return (error);
	}
	iris_fbsd_pdev = pdev;
	return (0);
}

void
iris_fbsd_linux_detach(void)
{
	linux_set_current(curthread);
	if (iris_fbsd_pdev != NULL)
		platform_device_unregister(iris_fbsd_pdev);
	iris_fbsd_pdev = NULL;
}
