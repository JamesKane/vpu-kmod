# vpu-kmod

The Arm China Mali-V ("Linlon VPU", mvx) video codec driver for FreeBSD,
on the CIX Sky1 (Orange Pi 6 Plus, Radxa Orion O6): CIX's Linux driver
through LinuxKPI, out of tree.

- `driver/`: CIX's driver, release 1.0.2 for Linux 7.0 (GPL-2.0-only;
  [cix_opensource__vpu_driver](https://github.com/cixtech/cix_opensource__vpu_driver),
  branch `cix_mainline_upstream_dev`, commit 5b9b07b), imported
  unmodified, then changed only where FreeBSD needs it.
- `media/`: Linux v7.0's V4L2 core and videobuf2 (GPL-2.0; the user API
  headers GPL-2.0 WITH Linux-syscall-note), under Linux's own paths:
  `drivers/media/v4l2-core`, `drivers/media/common/videobuf2`,
  `include/media`, `include/uapi/linux`.  FreeBSD has no V4L2 in its
  kernel; the driver's V4L2 layer builds on these.

The plan is AbyssBSD's `docs/boards/orangepi-6-plus/vpu.md`.

## Status

On the Orange Pi 6 Plus the codec probes (Linlon v5276, four cores)
and registers four mem2mem devices: `/dev/video0` decodes (H.263,
H.264, HEVC, MPEG-2, MPEG-4, VP8, VP9, AV1), `/dev/video1` encodes
(H.264, HEVC, VP8, VP9), `/dev/video2` and `/dev/video3` handle JPEG.
FFmpeg's `*_v4l2m2m` codecs work: H.264 and HEVC 1080p and VP9 720p
decode bit-exactly against software decoding, about 120 1080p frames a
second; H.264 and HEVC encode from NV12, NV21 or YUV420
(`tools/decode-test.sh`, `tools/encode-test.sh`).

Not yet: USERPTR buffers, the driver's secure (protected) video, DVFS
and devfreq, loading at boot.

## Requirements

- A FreeBSD kernel with LinuxKPI fixes these modules need: kobjects
  without a parent, platform devices' sysfs nodes, DMA syncs of a range
  within a mapping and of 1:1 mappings, and poll on several wait
  queues.  They are on the `orangepi-6-plus` branch of
  [JamesKane/freebsd-src](https://github.com/JamesKane/freebsd-src).
- [drm-kmod](https://github.com/freebsd/drm-kmod), for dma-buf and its
  LinuxKPI headers (`DRMKMOD=`, default `../drm-kmod`).
- CIX's firmware (`*.fwb`, from CIX's `cix-vpu-driver` package) in
  `/boot/firmware`.

## Building and loading

    make DRMKMOD=/path/to/drm-kmod SYSDIR=/path/to/freebsd-src/sys
    kldload ./v4l2/lkpi_v4l2.ko ./vpu/amvx.ko

`tools/build.sh` cross-builds both from macOS.  Log levels:
`hw.amvx.log.<if|fwif|session|dev|perf>`, 0 (panic) to 5 (verbose),
as tunables or sysctls.

`tools/v4l2info` lists a device's capabilities and formats (it builds
against the `v4l_compat` package's headers).
