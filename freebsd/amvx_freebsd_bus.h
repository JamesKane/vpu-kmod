/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Between the FreeBSD bus glue (amvx_freebsd_bus.c) and the Linux side of
 * the glue (amvx_freebsd.c): FreeBSD types only.
 */

#ifndef _AMVX_FREEBSD_BUS_H_
#define	_AMVX_FREEBSD_BUS_H_

/* amvx_freebsd.c: the platform device the mvx driver attaches to. */
int	amvx_fbsd_linux_attach(device_t dev, uint64_t rcsu_pa,
	    uint64_t rcsu_size, uint64_t pa, uint64_t size, int irq);
void	amvx_fbsd_linux_detach(void);

/*
 * amvx_freebsd_bus.c: repair a core's memories after power-up, as the
 * driver's core power domains do on Linux.
 */
#define	AMVX_FBSD_NCORES	4
int	amvx_fbsd_core_repair(unsigned int core);

/*
 * amvx_freebsd_bus.c: an integer or integer-array _DSD property of dev, up
 * to nvals of them into vals (which may be NULL): the number it has, or -1
 * if it has none; and the SCMI id of a clock its CLKT names.
 */
int	amvx_fbsd_get_u32_prop(device_t dev, const char *name, uint32_t *vals,
	    size_t nvals);
int	amvx_fbsd_clk_id(const char *name, uint32_t *id);

#endif /* _AMVX_FREEBSD_BUS_H_ */
