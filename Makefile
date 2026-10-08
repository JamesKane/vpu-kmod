# Build against a drm-kmod checkout: make DRMKMOD=/path/to/drm-kmod
SUBDIR=	v4l2 vpu iris

.include <bsd.subdir.mk>
