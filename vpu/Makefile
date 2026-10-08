# Arm China's Mali-V (mvx) video codec driver (driver/, CIX's release for
# Linux 7.0) through LinuxKPI: amvx.ko, on lkpi_v4l2.ko.  Built against
# drm-kmod (for dma-buf).

DRMKMOD?=	${.CURDIR:H:H}/drm-kmod
.include "${DRMKMOD}/kconfig.mk"

MVX=	${.CURDIR:H}/driver
MEDIA=	${.CURDIR:H}/media

.PATH:	${MVX} ${MVX}/if ${MVX}/if/v4l2 ${MVX}/dev ${.CURDIR:H}/freebsd

KMOD=	amvx

# driver/Kbuild's amvx-y, but for the DSM system events (not configured)
# and the logging, which is FreeBSD's (mvx_log_freebsd.c).
SRCS=	mvx_driver.c \
	mvx_seq.c \
	mvx_log_freebsd.c \
	mvx_if.c \
	mvx_buffer.c \
	mvx_firmware_cache.c \
	mvx_firmware.c \
	mvx_firmware_v2.c \
	mvx_firmware_v3.c \
	mvx_mmu.c \
	mvx_secure.c \
	mvx_session.c \
	mvx_ext_v4l2.c \
	mvx_v4l2_buffer.c \
	mvx_v4l2_session.c \
	mvx_v4l2_vidioc.c \
	mvx_v4l2_fops.c \
	mvx_v4l2_ctrls.c \
	mvx_dev.c \
	mvx_hwreg.c \
	mvx_hwreg_v500.c \
	mvx_hwreg_v550.c \
	mvx_hwreg_v61.c \
	mvx_hwreg_v52_v76.c \
	mvx_lsid.c \
	mvx_scheduler.c \
	mvx_pm_runtime.c

# In place of CIX's ACPI power domains and Linux's clocks and resets.
SRCS+=	amvx_freebsd.c \
	amvx_freebsd_bus.c

SRCS+=	opt_acpi.h \
	opt_platform.h \
	acpi_if.h \
	bus_if.h \
	device_if.h \
	pci_if.h \
	vnode_if.h

.include <bsd.kmod.mk>

.include "${DRMKMOD}/linuxkpi_version.mk"
.include "${DRMKMOD}/compiler_flags.mk"

CFLAGS+= -I${.CURDIR:H}/freebsd/include
CFLAGS+= -I${MVX} -I${MVX}/if -I${MVX}/dev -I${MVX}/if/v4l2 -I${MVX}/external
CFLAGS+= -I${MEDIA}/include -I${MEDIA}/include/uapi
CFLAGS+= -I${DRMKMOD}/linuxkpi/gplv2/include
CFLAGS+= -I${DRMKMOD}/linuxkpi/bsd/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/common/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/dummy/include
CFLAGS+= -I${DRMKMOD}/include
CFLAGS+= -I${DRMKMOD}/include/uapi

CFLAGS+= -D__KERNEL__
CFLAGS+= '-DKBUILD_MODNAME="${KMOD}"'
CFLAGS+= '-DLINUXKPI_PARAM_PREFIX=amvx_' -DDRM_SYSCTL_PARAM_PREFIX=_${KMOD}
CFLAGS+= ${KCONFIG:C/(.*)/-DCONFIG_\1/}
CFLAGS+= -DCONFIG_VIDEO_LINLON
# As CIX's Android build: no dumping of input buffers to /data files.
CFLAGS+= -DMVX_ANDROID_KERNEL
CFLAGS+= -include ${.CURDIR:H}/freebsd/include/vpu_freebsd_compat.h
CFLAGS+= -include ${.CURDIR:H}/freebsd/include/amvx_freebsd.h
CFLAGS+= -I${.CURDIR:H}/freebsd
CFLAGS.amvx_freebsd_bus.c+= -DAMVX_FBSD_BUS
CWARNFLAGS+=	-Wno-format -Wno-gnu-pointer-arith -Wno-cast-qual \
		-Wno-unused-but-set-variable -Wno-default-const-init-field-unsafe \
		-Wno-void-pointer-to-enum-cast -Wno-unused-label -Wno-unused-variable
