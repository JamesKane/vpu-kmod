# Linux's V4L2 core and videobuf2 (media/, Linux v7.0) through LinuxKPI:
# lkpi_v4l2.ko, for V4L2 drivers ported from Linux.  Built against
# drm-kmod (for dma-buf).

DRMKMOD?=	${.CURDIR:H:H}/drm-kmod
.include "${DRMKMOD}/kconfig.mk"

MEDIA=	${.CURDIR:H}/media

.PATH:	${MEDIA}/drivers/media/v4l2-core ${MEDIA}/drivers/media/common/videobuf2 \
	${.CURDIR:H}/freebsd

KMOD=	lkpi_v4l2

SRCS=	v4l2-dev.c \
	v4l2-ioctl.c \
	v4l2-device.c \
	v4l2-fh.c \
	v4l2-event.c \
	v4l2-ctrls-core.c \
	v4l2-ctrls-api.c \
	v4l2-ctrls-defs.c \
	v4l2-ctrls-request.c \
	v4l2-common.c \
	videobuf2-core.c \
	videobuf2-v4l2.c \
	videobuf2-dma-sg.c \
	videobuf2-dma-contig.c \
	videobuf2-memops.c \
	v4l2-mem2mem.c \
	lkpi_v4l2_freebsd.c \
	lkpi_v4l2_dma.c \
	lkpi_v4l2_subdev.c

SRCS+=	opt_acpi.h \
	opt_platform.h \
	bus_if.h \
	device_if.h \
	pci_if.h \
	vnode_if.h

CFLAGS.videobuf2-dma-contig.c+= -include lkpi_v4l2_dma.h
CFLAGS.v4l2-mem2mem.c+=	-Wno-unused-const-variable

.include <bsd.kmod.mk>

.include "${DRMKMOD}/linuxkpi_version.mk"
.include "${DRMKMOD}/compiler_flags.mk"

CFLAGS+= -I${.CURDIR:H}/freebsd/include
CFLAGS+= -I${MEDIA}/include -I${MEDIA}/include/uapi
CFLAGS+= -I${DRMKMOD}/linuxkpi/gplv2/include
CFLAGS+= -I${DRMKMOD}/linuxkpi/bsd/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/common/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/dummy/include
CFLAGS+= -I${DRMKMOD}/include
CFLAGS+= -I${DRMKMOD}/include/uapi

CFLAGS+= -D__KERNEL__ -DLKPI_V4L2_MODULE
CFLAGS+= '-DKBUILD_MODNAME="${KMOD}"'
CFLAGS+= '-DLINUXKPI_PARAM_PREFIX=v4l2_' -DDRM_SYSCTL_PARAM_PREFIX=_${KMOD}
CFLAGS+= ${KCONFIG:C/(.*)/-DCONFIG_\1/}
CFLAGS+= -include ${.CURDIR:H}/freebsd/include/vpu_freebsd_compat.h
CWARNFLAGS+=	-Wno-format -Wno-gnu-pointer-arith -Wno-cast-qual \
		-Wno-unused-but-set-variable -Wno-default-const-init-field-unsafe
