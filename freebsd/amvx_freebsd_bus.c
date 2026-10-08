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
 * The FreeBSD side of the Mali-V codec on the CIX Sky1: the ACPI device
 * (CIXH3010), powered with its four cores, its RCSU out of reset, handed to
 * the mvx driver as a platform device.  In place of CIX's Linux ACPI power
 * domains: the driver's runtime PM then switches only the codec's clock
 * and reset (amvx_freebsd.c).
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <dev/acpica/acpivar.h>

#include "acpi_if.h"
#include "amvx_freebsd_bus.h"

/* The codec's cores: power-only ACPI children, each with power resources. */
#define	AMVX_FBSD_NCORES	4

/* The Sky1's reset controller (ACPI RST0), active low. */
#define	SKY1_RST_PA		0x16000000UL
#define	 RST_GROUP1_RCSU	0x804
#define	  RST_VPU_RCSU		(1u << 19)	/* RSTL vpu_rcsu_reset */

static char *amvx_fbsd_acpi_ids[] = { "CIXH3010", NULL };

struct amvx_fbsd_softc {
	device_t	dev;
	bool		powered;
};

static struct amvx_fbsd_softc *amvx_fbsd_sc;

int
amvx_fbsd_get_u32_prop(device_t dev, const char *name, uint32_t *vals,
    size_t nvals)
{
	const ACPI_OBJECT *obj, *e;
	uint32_t i;

	if (ACPI_FAILURE(ACPI_GET_PROPERTY(device_get_parent(dev), dev, name,
	    &obj)))
		return (-1);
	switch (obj->Type) {
	case ACPI_TYPE_INTEGER:
		if (vals != NULL && nvals > 0)
			vals[0] = obj->Integer.Value;
		return (1);
	case ACPI_TYPE_PACKAGE:
		for (i = 0; i < obj->Package.Count; i++) {
			e = &obj->Package.Elements[i];
			if (e->Type != ACPI_TYPE_INTEGER)
				return (-1);
			if (vals != NULL && i < nvals)
				vals[i] = e->Integer.Value;
		}
		return (obj->Package.Count);
	default:
		return (-1);
	}
}

/* The SCMI id of a clock the device's CLKT names: (id, name, device). */
int
amvx_fbsd_clk_id(const char *name, uint32_t *id)
{
	struct amvx_fbsd_softc *sc = amvx_fbsd_sc;
	ACPI_BUFFER buf;
	ACPI_OBJECT *pkg, *e;
	int error;
	u_int i;

	if (sc == NULL)
		return (ENXIO);
	buf.Pointer = NULL;
	buf.Length = ACPI_ALLOCATE_BUFFER;
	if (ACPI_FAILURE(AcpiEvaluateObject(acpi_get_handle(sc->dev), "CLKT",
	    NULL, &buf)))
		return (ENOENT);
	pkg = buf.Pointer;
	error = ENOENT;
	for (i = 0; pkg->Type == ACPI_TYPE_PACKAGE && i < pkg->Package.Count;
	    i++) {
		e = &pkg->Package.Elements[i];
		if (e->Type != ACPI_TYPE_PACKAGE || e->Package.Count < 2 ||
		    e->Package.Elements[0].Type != ACPI_TYPE_INTEGER ||
		    e->Package.Elements[1].Type != ACPI_TYPE_STRING ||
		    strcmp(e->Package.Elements[1].String.Pointer, name) != 0)
			continue;
		*id = e->Package.Elements[0].Integer.Value;
		error = 0;
		break;
	}
	AcpiOsFree(buf.Pointer);
	return (error);
}

/*
 * The block, then each core: their _PR0 power resources (_ON releases
 * each from reset through the firmware's DMRP method).
 */
static int
amvx_fbsd_power(struct amvx_fbsd_softc *sc, int state)
{
	ACPI_HANDLE h, core;
	ACPI_STATUS st;
	char name[5];
	int i;

	h = acpi_get_handle(sc->dev);
	if (state == ACPI_STATE_D0) {
		st = acpi_pwr_switch_consumer(h, state);
		if (ACPI_FAILURE(st)) {
			device_printf(sc->dev, "cannot power the codec: %s\n",
			    AcpiFormatException(st));
			return (ENXIO);
		}
	}
	for (i = 0; i < AMVX_FBSD_NCORES; i++) {
		snprintf(name, sizeof(name), "CRE%d", i);
		if (ACPI_FAILURE(AcpiGetHandle(h, name, &core)))
			continue;
		st = acpi_pwr_switch_consumer(core, state);
		if (ACPI_FAILURE(st) && state == ACPI_STATE_D0) {
			device_printf(sc->dev, "cannot power %s: %s\n", name,
			    AcpiFormatException(st));
			return (ENXIO);
		}
	}
	if (state != ACPI_STATE_D0)
		(void)acpi_pwr_switch_consumer(h, state);
	return (0);
}

/* The RCSU (the block's control registers) out of reset. */
static void
amvx_fbsd_rcsu_reset(bool assert)
{
	volatile uint32_t *r;

	r = pmap_mapdev(SKY1_RST_PA + RST_GROUP1_RCSU, sizeof(*r));
	if (assert)
		*r &= ~RST_VPU_RCSU;
	else
		*r |= RST_VPU_RCSU;
	pmap_unmapdev(__DEVOLATILE(void *, r), sizeof(*r));
}

static int
amvx_fbsd_probe(device_t dev)
{
	int rv;

	rv = ACPI_ID_PROBE(device_get_parent(dev), dev, amvx_fbsd_acpi_ids,
	    NULL);
	if (rv <= 0)
		device_set_desc(dev, "Arm Mali-V video codec (CIX Sky1)");
	return (rv);
}

static int
amvx_fbsd_attach(device_t dev)
{
	struct amvx_fbsd_softc *sc = device_get_softc(dev);
	rman_res_t rcsu_pa, rcsu_size, pa, size, irq, n;
	int error;

	if (amvx_fbsd_sc != NULL)
		return (ENXIO);
	sc->dev = dev;
	/*
	 * The resources stay the bus's: the driver maps the registers itself,
	 * and LinuxKPI's request_irq() takes the FreeBSD interrupt number.
	 */
	if (bus_get_resource(dev, SYS_RES_MEMORY, 0, &rcsu_pa, &rcsu_size) != 0 ||
	    bus_get_resource(dev, SYS_RES_MEMORY, 1, &pa, &size) != 0 ||
	    bus_get_resource(dev, SYS_RES_IRQ, 0, &irq, &n) != 0) {
		device_printf(dev, "no registers or interrupt\n");
		return (ENXIO);
	}
	amvx_fbsd_sc = sc;
	if ((error = amvx_fbsd_power(sc, ACPI_STATE_D0)) != 0)
		goto fail;
	sc->powered = true;
	amvx_fbsd_rcsu_reset(false);
	DELAY(20);

	error = -amvx_fbsd_linux_attach(dev, rcsu_pa, rcsu_size, pa, size, irq);
	if (error == 0)
		return (0);
	amvx_fbsd_linux_detach();
fail:
	if (sc->powered) {
		amvx_fbsd_rcsu_reset(true);
		(void)amvx_fbsd_power(sc, ACPI_STATE_D3);
	}
	sc->powered = false;
	amvx_fbsd_sc = NULL;
	return (error);
}

static int
amvx_fbsd_detach(device_t dev)
{
	struct amvx_fbsd_softc *sc = device_get_softc(dev);

	amvx_fbsd_linux_detach();
	if (sc->powered) {
		amvx_fbsd_rcsu_reset(true);
		(void)amvx_fbsd_power(sc, ACPI_STATE_D3);
	}
	sc->powered = false;
	amvx_fbsd_sc = NULL;
	return (0);
}

static device_method_t amvx_fbsd_methods[] = {
	DEVMETHOD(device_probe,		amvx_fbsd_probe),
	DEVMETHOD(device_attach,	amvx_fbsd_attach),
	DEVMETHOD(device_detach,	amvx_fbsd_detach),
	DEVMETHOD_END
};

static driver_t amvx_fbsd_driver = {
	"amvx",
	amvx_fbsd_methods,
	sizeof(struct amvx_fbsd_softc),
};

DRIVER_MODULE(amvx, acpi, amvx_fbsd_driver, 0, 0);
ACPI_PNP_INFO(amvx_fbsd_acpi_ids);
MODULE_DEPEND(amvx, acpi, 1, 1, 1);
MODULE_DEPEND(amvx, linuxkpi, 1, 1, 1);
MODULE_DEPEND(amvx, dmabuf, 1, 1, 1);
MODULE_DEPEND(amvx, lindebugfs, 1, 1, 1);
MODULE_DEPEND(amvx, lkpi_v4l2, 1, 1, 1);
MODULE_VERSION(amvx, 1);
