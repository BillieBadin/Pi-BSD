# Installation

## Preparation

- Grab the standard Raspberry 3/4 FreeBSD image from [here](https://www.freebsd.org/where/)
  - see [here]([https://gitea.badin.au/Config/PWKS](https://download.freebsd.org/releases/arm64/aarch64/ISO-IMAGES/15.0/)0) for FreeBSD 15.0
  - this guide uses [FreeBSD-15.0-RELEASE-arm64-aarch64-RPI.img.xz](https://download.freebsd.org/releases/arm64/aarch64/ISO-IMAGES/15.0/FreeBSD-15.0-RELEASE-arm64-aarch64-RPI.img.xz).
- Flash the image on an SD card using the **Raspberry Pi Imager** tool

## Initial installation

- Boot
- Connect as user `freebsd` (password `freebsd`)
- Elevate as root `su - root` (password `root`)
- The Raspberry pi doesn't have a Real Time Clock, it is therefore essential to set the system clock once with ntp to avoid certificate errors during the installation process:
  - execute `ntpdate pool.ntp.org`
- Load ZFS kernel module:
  - execute: `kldload zfs`
- Run the installer: `bsdinstall`

