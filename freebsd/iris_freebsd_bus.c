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
 * The FreeBSD side of the Iris glue: the SC8280XP's video codec, which
 * Windows' ACPI lists only among the GPU device's resources (registers at
 * 0x0AA00000, interrupt GSIV 206), as a device of its own, found from the
 * SoC (\_SB.SOID) as the other Qualcomm drivers find what ACPI leaves out:
 *
 * - its DMA through the apps SMMU (qcom_apps_iommu(4)), with the streams
 *   and window Linux's devicetree gives it, claimed before LinuxKPI makes
 *   the device's DMA tag; coherent, as the devicetree says;
 * - its power and clocks (qcom_videocc(4)) and bandwidth (RPMh), for the
 *   Linux side's clock and power domain shims;
 * - then the platform device Qualcomm's Iris driver attaches to
 *   (iris_freebsd.c).
 *
 * Needs a kernel with options IOMMU (GENERIC-IOMMU).
 */

#include "opt_acpi.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <dev/acpica/acpivar.h>

#include <dev/qcom_rpmh/qcom_rpmh.h>
#include <dev/qcom_smmu/qcom_apps_iommu.h>
#include <dev/qcom_videocc/qcom_videocc.h>

#include "iris_freebsd_bus.h"

#define	IRIS_FBSD_SOC_ID	449		/* SC8280XP */
#define	IRIS_FBSD_PA		0x0aa00000ul	/* as Linux maps it */
#define	IRIS_FBSD_SIZE		0x100000ul
#define	IRIS_FBSD_GSIV		206		/* SPI 174, level high */
/* Linux's: the firmware protects below the window, the DMA mask above. */
#define	IRIS_FBSD_IOVA_START	0x25800000ul
#define	IRIS_FBSD_IOVA_END	0xe0000000ul

/* The codec's streams, as Radxa's devicetree routes them. */
static const uint16_t iris_fbsd_sids[] = { 0x2a00, 0x2a07 };
static const uint16_t iris_fbsd_masks[] = { 0x400, 0x400 };

struct iris_fbsd_softc {
	device_t		dev;
	struct qcom_videocc	*vcc;
	bool			claimed;
	bool			bw_voted;
};

static struct iris_fbsd_softc *iris_fbsd_sc;

/* Power and bandwidth, for the Linux side */

int
iris_fbsd_ctrl_enable(void)
{
	return (iris_fbsd_sc == NULL ? ENXIO :
	    qcom_videocc_ctrl_enable(iris_fbsd_sc->vcc));
}

void
iris_fbsd_ctrl_disable(void)
{
	if (iris_fbsd_sc != NULL)
		qcom_videocc_ctrl_disable(iris_fbsd_sc->vcc);
}

int
iris_fbsd_hw_enable(void)
{
	return (iris_fbsd_sc == NULL ? ENXIO :
	    qcom_videocc_hw_enable(iris_fbsd_sc->vcc));
}

void
iris_fbsd_hw_disable(void)
{
	if (iris_fbsd_sc != NULL)
		qcom_videocc_hw_disable(iris_fbsd_sc->vcc);
}

int
iris_fbsd_hw_set_hwmode(bool hw)
{
	return (iris_fbsd_sc == NULL ? ENXIO :
	    qcom_videocc_hw_set_hwmode(iris_fbsd_sc->vcc, hw));
}

/*
 * The codec's path to memory (BCM MM1, shared with the display's): to its
 * peak, once.  Nothing aggregates the application processors' votes, so it
 * is never lowered (see qcom_videocc(4)).
 */
int
iris_fbsd_bw_vote(void)
{
	int error;

	if (iris_fbsd_sc == NULL)
		return (ENXIO);
	if (iris_fbsd_sc->bw_voted)
		return (0);
	error = qcom_rpmh_bcm_vote("MM1", 0, QCOM_RPMH_BCM_MAX);
	if (error != 0) {
		device_printf(iris_fbsd_sc->dev, "no bandwidth vote: %d\n",
		    error);
		return (error);
	}
	iris_fbsd_sc->bw_voted = true;
	return (0);
}

/* The device */

static void
iris_fbsd_identify(driver_t *driver, device_t parent)
{
	UINT32 id;

	if (device_find_child(parent, "qcom_iris", DEVICE_UNIT_ANY) != NULL)
		return;
	if (ACPI_FAILURE(acpi_GetInteger(ACPI_ROOT_OBJECT, "\\_SB.SOID", &id)) ||
	    id != IRIS_FBSD_SOC_ID)
		return;
	BUS_ADD_CHILD(parent, 10, "qcom_iris", 0);
}

static int
iris_fbsd_probe(device_t dev)
{
	device_set_desc(dev, "Qualcomm Iris video codec (SC8280XP)");
	return (BUS_PROBE_NOWILDCARD);
}

static int
iris_fbsd_detach(device_t dev)
{
	struct iris_fbsd_softc *sc = device_get_softc(dev);

	iris_fbsd_linux_detach();
	iris_fbsd_sc = NULL;
	if (sc->claimed)
		qcom_apps_iommu_unclaim(dev);
	sc->claimed = false;
	qcom_videocc_destroy(sc->vcc);
	sc->vcc = NULL;
	return (0);
}

static int
iris_fbsd_attach(device_t dev)
{
	struct iris_fbsd_softc *sc = device_get_softc(dev);
	int error, irq;

	if (iris_fbsd_sc != NULL)
		return (ENXIO);
	sc->dev = dev;
	sc->vcc = qcom_videocc_create(dev);
	if (sc->vcc == NULL) {
		device_printf(dev, "no clock controller\n");
		return (ENXIO);
	}
	/* ACPI maps the GSIV (level, high) as it sets the resource. */
	error = bus_set_resource(dev, SYS_RES_IRQ, 0, IRIS_FBSD_GSIV, 1);
	if (error == 0)
		error = bus_set_resource(dev, SYS_RES_MEMORY, 0, IRIS_FBSD_PA,
		    IRIS_FBSD_SIZE);
	if (error != 0)
		goto fail;
	irq = bus_get_resource_start(dev, SYS_RES_IRQ, 0);
	error = qcom_apps_iommu_claim(dev, iris_fbsd_sids, iris_fbsd_masks,
	    nitems(iris_fbsd_sids), IRIS_FBSD_IOVA_START, IRIS_FBSD_IOVA_END,
	    QCOM_APPS_IOMMU_COHERENT);
	if (error != 0) {
		device_printf(dev, "cannot claim the SMMU (a kernel with "
		    "options IOMMU?): %d\n", error);
		goto fail;
	}
	sc->claimed = true;
	iris_fbsd_sc = sc;
	error = -iris_fbsd_linux_attach(dev, IRIS_FBSD_PA, IRIS_FBSD_SIZE,
	    irq);
	if (error == 0)
		return (0);
fail:
	iris_fbsd_detach(dev);
	return (error);
}

static device_method_t iris_fbsd_methods[] = {
	DEVMETHOD(device_identify,	iris_fbsd_identify),
	DEVMETHOD(device_probe,		iris_fbsd_probe),
	DEVMETHOD(device_attach,	iris_fbsd_attach),
	DEVMETHOD(device_detach,	iris_fbsd_detach),
	DEVMETHOD_END
};

static driver_t iris_fbsd_driver = {
	"qcom_iris",
	iris_fbsd_methods,
	sizeof(struct iris_fbsd_softc),
};

DRIVER_MODULE(qcom_iris, acpi, iris_fbsd_driver, 0, 0);
MODULE_DEPEND(qcom_iris, acpi, 1, 1, 1);
MODULE_DEPEND(qcom_iris, linuxkpi, 1, 1, 1);
MODULE_DEPEND(qcom_iris, dmabuf, 1, 1, 1);
MODULE_DEPEND(qcom_iris, lindebugfs, 1, 1, 1);
MODULE_DEPEND(qcom_iris, lkpi_v4l2, 1, 1, 1);
MODULE_DEPEND(qcom_iris, qcom_apps_iommu, 1, 1, 1);
MODULE_DEPEND(qcom_iris, qcom_videocc, 1, 1, 1);
MODULE_DEPEND(qcom_iris, qcom_rpmh, 1, 1, 1);
MODULE_DEPEND(qcom_iris, qcom_scm, 1, 1, 1);
MODULE_VERSION(qcom_iris, 1);
