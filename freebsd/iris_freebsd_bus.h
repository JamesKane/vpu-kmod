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

#ifndef _IRIS_FREEBSD_BUS_H_
#define	_IRIS_FREEBSD_BUS_H_

/*
 * Between the FreeBSD side of the Iris glue (iris_freebsd_bus.c) and its
 * Linux side (iris_freebsd.c): FreeBSD types only.
 */

/* iris_freebsd.c: the platform device Qualcomm's Iris driver attaches to. */
int	iris_fbsd_linux_attach(device_t dev, uint64_t pa, uint64_t size,
	    int irq);
void	iris_fbsd_linux_detach(void);

/*
 * iris_freebsd_bus.c: the codec's power (qcom_videocc(4)): its controller
 * (the power domain, resets and clocks of Iris's power_on_controller), its
 * core (power_on_hw), the core under the codec's control, the core clock's
 * level; and its bandwidth (RPMh).
 */
int	iris_fbsd_ctrl_enable(void);
void	iris_fbsd_ctrl_disable(void);
int	iris_fbsd_hw_enable(void);
void	iris_fbsd_hw_disable(void);
int	iris_fbsd_hw_set_hwmode(bool hw);
int	iris_fbsd_set_rate(u_long hz);
u_long	iris_fbsd_round_rate(u_long hz);
u_long	iris_fbsd_get_rate(void);
int	iris_fbsd_bw_vote(uint32_t avg_kbps, uint32_t peak_kbps);

#endif /* _IRIS_FREEBSD_BUS_H_ */
