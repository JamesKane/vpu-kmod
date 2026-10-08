# Qualcomm's Iris video codec driver (qcom-iris/, Radxa's Linux 7.0.11)
# through LinuxKPI: qcom_iris.ko, on lkpi_v4l2.ko, for the SC8280XP (Radxa
# Dragon Q8B).  Built against drm-kmod (for dma-buf).

DRMKMOD?=	${.CURDIR:H:H}/drm-kmod
.include "${DRMKMOD}/kconfig.mk"

IRIS=	${.CURDIR:H}/qcom-iris
MEDIA=	${.CURDIR:H}/media

.PATH:	${IRIS} ${.CURDIR:H}/freebsd

KMOD=	qcom_iris

# qcom-iris/Makefile's qcom-iris-objs.
SRCS=	iris_buffer.c \
	iris_common.c \
	iris_core.c \
	iris_ctrls.c \
	iris_firmware.c \
	iris_hfi_common.c \
	iris_hfi_gen1.c \
	iris_hfi_gen1_command.c \
	iris_hfi_gen1_response.c \
	iris_hfi_gen2.c \
	iris_hfi_gen2_command.c \
	iris_hfi_gen2_packet.c \
	iris_hfi_gen2_response.c \
	iris_hfi_queue.c \
	iris_platform_vpu2.c \
	iris_platform_vpu3x.c \
	iris_power.c \
	iris_probe.c \
	iris_resources.c \
	iris_state.c \
	iris_utils.c \
	iris_vidc.c \
	iris_vb2.c \
	iris_vdec.c \
	iris_venc.c \
	iris_vpu2.c \
	iris_vpu3x.c \
	iris_vpu4x.c \
	iris_vpu_buffer.c \
	iris_vpu_common.c

# In place of Linux's devicetree, clocks, power domains, interconnect and
# Qualcomm platform calls.
SRCS+=	iris_freebsd.c \
	iris_freebsd_bus.c

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
CFLAGS+= -I${IRIS}
CFLAGS+= -I${MEDIA}/include -I${MEDIA}/include/uapi
CFLAGS+= -I${DRMKMOD}/linuxkpi/gplv2/include
CFLAGS+= -I${DRMKMOD}/linuxkpi/bsd/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/common/include
CFLAGS+= -I${SYSDIR}/compat/linuxkpi/dummy/include
CFLAGS+= -I${DRMKMOD}/include
CFLAGS+= -I${DRMKMOD}/include/uapi

CFLAGS+= -D__KERNEL__
CFLAGS+= '-DKBUILD_MODNAME="${KMOD}"'
CFLAGS+= '-DLINUXKPI_PARAM_PREFIX=qcom_iris_' -DDRM_SYSCTL_PARAM_PREFIX=_${KMOD}
CFLAGS+= ${KCONFIG:C/(.*)/-DCONFIG_\1/}
CFLAGS+= -include ${.CURDIR:H}/freebsd/include/vpu_freebsd_compat.h
CFLAGS+= -include ${.CURDIR:H}/freebsd/include/iris_freebsd.h
CFLAGS+= -I${.CURDIR:H}/freebsd
CFLAGS.iris_freebsd_bus.c+= -DIRIS_FBSD_BUS
CWARNFLAGS+=	-Wno-format -Wno-gnu-pointer-arith -Wno-cast-qual \
		-Wno-unused-but-set-variable -Wno-default-const-init-field-unsafe \
		-Wno-unused-variable
