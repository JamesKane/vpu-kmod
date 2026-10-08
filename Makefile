# Build against a drm-kmod checkout: make DRMKMOD=/path/to/drm-kmod
SUBDIR=	v4l2 vpu

.include <bsd.subdir.mk>
