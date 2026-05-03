# Installation

## Preparation

- Grab the standard Raspberry 3/4 FreeBSD image from [here](https://www.freebsd.org/where/)
  - see [here]([https://gitea.badin.au/Config/PWKS](https://download.freebsd.org/releases/arm64/aarch64/ISO-IMAGES/15.0/)0) for FreeBSD 15.0
  - this guide uses [FreeBSD-15.0-RELEASE-arm64-aarch64-RPI.img.xz](https://download.freebsd.org/releases/arm64/aarch64/ISO-IMAGES/15.0/FreeBSD-15.0-RELEASE-arm64-aarch64-RPI.img.xz).
- Flash the image on an SD card using the **Raspberry Pi Imager** tool
- Prepare one or - better - two (for ZFS mirror) USB3+ USB flash drive(s) or external USB-attached SSD(s)

## Initial installation

- Boot without the USB storage attached to ensure the boot is from the micro-SD
- Connect as user `freebsd` (password `freebsd`)
- Elevate as root `su - root` (password `root`)
- The Raspberry pi doesn't have a Real Time Clock, it is therefore essential to set the system clock once with ntp to avoid certificate errors during the installation process:
  - execute `ntpdate pool.ntp.org`
- Load ZFS kernel module:
  - execute: `kldload zfs`
- Connect the USB-attached storage device(s)
- Check they appear:
  - execute `sysctl kern.disks` and `gpart show`
  - the drive(s) should show up as `da0` (and `da1`)
- Clean USB drive(s): `gpart destroy -F da0`, (`gpart destroy -F da1`)
- Run the installer: `bsdinstall`

Recommended options:
- Services: sshd, ntpd, ntpd_sync_on_start, powerd
- Hardening: hide_uids, hide_gids, hide_jails, secure_console
- Add a user, include them in the wheel group, enable ZFS encryption for the home folder of that user, tied to the user password

- reboot: `reboot`
  - keep the SD card which wil lbe used for the Raspberry Pi firmware, but the actual OS will be loaded from the USB drive(s)
  - the Raspberry Pi firmware will be copied to the USB drive(s) EFI partition after reboot

## First boot

### Set-up `sudo` to make your life easier and continue all over SSH as root

Logon as root and:
```sh
pkg update
pkg install sudo
visudo
```
Uncomment the line with `%wheel ALL=(ALL:ALL) ALL`

Logon as your regular user.

Note that your encrypted ZFS home folder will not be mounted, we will fix this later.

Elevate as root: `sudo su -`

### Install Raspberry Pi firmware on the USB drive(s) boot partition(s)

Make sure the efiboot partition is clean (I systematically have to do this on first install):
```sh
fsck_msdosfs -y /dev/gpt/efiboot0
# if any error, proceed as follows:
umount /boot/efi
fsck_msdosfs -y /dev/gpt/efiboot0
fsck_msdosfs /dev/gpt/efiboot0
mount -t msdosfs /dev/gpt/efiboot0 /boot/efi
```

Mount the efi partition from the SD card:
```sh
mkdir -p /mnt/sdboot
mount -t msdosfs /dev/mmcsd0s1 /mnt/sdboot
```

Copy the EFI from the original SD card:
```sh
cp /mnt/sdboot/start4.elf /mnt/sdboot/fixup4.dat /mnt/sdboot/bcm2711-rpi-4-b.dtb /boot/efi/
```

Install U-boot and copy `u-boot.bin` to `/boot/efi`
```sh
pkg install u-boot-rpi4
cp /usr/local/share/u-boot/u-boot-rpi4/u-boot.bin /boot/efi/
```

Create config.txt:
```sh
vi /boot/efi/config.txt`
```
```conf
arm_64bit=1
enable_uart=1
kernel=u-boot.bin
```

### Optional steps for ZFS mirror installations

At this stage, only one of the USB media will have the right files to boot. The zroot ZFS pool is safe from single point of failure because it is mirrored but the boot is only on one of the two USB mass storage devices. We wil lcopy it on the secondary one for full USB drive fault tolerance.

```sh

```

### Power off
```sh
sync
poweroff
```

Remove the SD card, the system can now boot from USB

## Second boot



