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
 * What lkpi_v4l2.ko leaves out of Linux's media core, as the built core
 * refers to it:
 *
 * - V4L2's sub-devices (sensors and bridges behind a media controller):
 *   v4l2-subdev.c is not built; a driver that registers no sub-device never
 *   calls these.
 * - User-pointer buffers (V4L2_MEMORY_USERPTR), which pin a process's
 *   pages (frame_vector.c): not supported.  FFmpeg's and GStreamer's V4L2
 *   codecs use mmap and dma-buf buffers.
 */

#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <media/frame_vector.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-dev.h>

const struct v4l2_subdev_ops v4l2_subdev_call_wrappers;
const struct v4l2_file_operations v4l2_subdev_fops;

struct frame_vector *
frame_vector_create(unsigned int nr_frames)
{
	return NULL;
}

void
frame_vector_destroy(struct frame_vector *vec)
{
}

int
frame_vector_to_pages(struct frame_vector *vec)
{
	return -EOPNOTSUPP;
}

int
get_vaddr_frames(unsigned long start, unsigned int nr_frames, bool write,
    struct frame_vector *vec)
{
	return -EOPNOTSUPP;
}

void
put_vaddr_frames(struct frame_vector *vec)
{
}
