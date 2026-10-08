/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 James Kane
 *
 * A V4L2 device's capabilities and formats, from a FreeBSD program
 * (v4l_compat's headers): VIDIOC_QUERYCAP, then VIDIOC_ENUM_FMT on each
 * buffer type it has.
 *
 * Usage: v4l2info /dev/videoN ...
 */
#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>

static void
fourcc(uint32_t f, char *s)
{
	s[0] = f & 0xff;
	s[1] = (f >> 8) & 0xff;
	s[2] = (f >> 16) & 0xff;
	s[3] = (f >> 24) & 0xff;
	s[4] = '\0';
}

static void
formats(int fd, enum v4l2_buf_type type, const char *what)
{
	struct v4l2_fmtdesc fd_;
	char cc[5];

	for (fd_.index = 0;; fd_.index++) {
		memset(&fd_.flags, 0, sizeof(fd_) - offsetof(struct v4l2_fmtdesc,
		    flags));
		fd_.type = type;
		if (ioctl(fd, VIDIOC_ENUM_FMT, &fd_) != 0)
			break;
		fourcc(fd_.pixelformat, cc);
		printf("  %s %2u: %s  %s%s\n", what, fd_.index, cc,
		    fd_.description,
		    (fd_.flags & V4L2_FMT_FLAG_COMPRESSED) ? " (compressed)" : "");
	}
}

int
main(int argc, char **argv)
{
	struct v4l2_capability cap;
	uint32_t caps;
	int fd, i;

	for (i = 1; i < argc; i++) {
		if ((fd = open(argv[i], O_RDWR)) < 0) {
			warn("%s", argv[i]);
			continue;
		}
		memset(&cap, 0, sizeof(cap));
		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
			warn("%s: VIDIOC_QUERYCAP", argv[i]);
			close(fd);
			continue;
		}
		caps = (cap.capabilities & V4L2_CAP_DEVICE_CAPS) ?
		    cap.device_caps : cap.capabilities;
		printf("%s: driver %s, card %s, bus %s, version %u.%u.%u, "
		    "caps %#x%s%s\n", argv[i], cap.driver, cap.card,
		    cap.bus_info, (cap.version >> 16) & 0xff,
		    (cap.version >> 8) & 0xff, cap.version & 0xff, caps,
		    (caps & V4L2_CAP_VIDEO_M2M_MPLANE) ? " m2m-mplane" : "",
		    (caps & V4L2_CAP_STREAMING) ? " streaming" : "");
		formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, "out");
		formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, "cap");
		close(fd);
	}
	return (0);
}
