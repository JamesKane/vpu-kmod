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
