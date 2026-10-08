# vpu-kmod

The Arm China Mali-V ("Linlon VPU", mvx) video codec driver for FreeBSD,
on the CIX Sky1 (Orange Pi 6 Plus, Radxa Orion O6): CIX's Linux driver
through LinuxKPI, out of tree.

- `driver/`: CIX's driver, release 1.0.2 for Linux 7.0 (GPL-2.0-only;
  [cix_opensource__vpu_driver](https://github.com/cixtech/cix_opensource__vpu_driver),
  branch `cix_mainline_upstream_dev`, commit 5b9b07b), imported
  unmodified, then changed only where FreeBSD needs it.

The plan is AbyssBSD's `docs/boards/orangepi-6-plus/vpu.md`.
