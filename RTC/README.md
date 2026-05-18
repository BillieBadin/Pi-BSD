# Pi-BSD - RTC (Real Time Clock) integration

This package compiles and installs a small utility to communicate with a
DS3231-based RTC over `I²C`, system configuration, plus rc helpers for
Raspberry Pi systems running FreeBSD.

## Contents

```txt
RTC/
├── Makefile
├── README.md (this document)
├── install.sh
├── uninstall.sh
├── conf/
│   ├── config.txt.fragment
│   └── loader.conf.fragment
├── rc.d/
│   ├── earlyrtc
│   ├── rtc_ntp_watch
│   └── rtcsync_shutdown
├── sbin/
│   └── ntp_to_rtc
└── src/
    └── ds3231ctl.c
```

---

## What gets installed and optionally updated

- `/usr/local/sbin/ds3231ctl`
- `/usr/local/sbin/ntp_to_rtc`
- `/etc/rc.d/earlyrtc`
- `/etc/rc.d/rtc_ntp_watch`
- `/etc/rc.d/rtcsync_shutdown`

Backups are created before modifying the following two files:

### `/boot/loader.conf`

The following lines are appended to `/boot/loader.conf` if missing:

```conf
iicbus_load="YES"
iic_load="YES"
```

### `/boot/efi/config.txt`

The following lines are appended to `/boot/efi/config.txt` if missing:

```conf
dtparam=i2c_arm=on
gpio=2,3=a0
```

---

## Installation

Run as root in the `RTC` folder:

```sh
sh ./install.sh
```

To target a different I2C device:

```sh
IIC_DEV=/dev/iic1 ./install.sh
```

By default the helpers use `/dev/iic0`. The installer writes `rtc_iic_dev`
in `rc.conf` (and keeps compatibility knobs for older per-service settings).

The installer compiles `src/ds3231ctl.c` using:

```sh
cc -Wall -Wextra -O2 -o ds3231ctl src/ds3231ctl.c
```

---

## Runtime behaviour

### `earlyrtc`

`earlyrtc` runs

- after `mountcritlocal`, and
- before `FILESYSTEMS`, `syslogd`, `netif`, `ntpd`, and `ntpdate`.

Benefit: better timestamps for nearly everything after early local filesystems
are mounted, including syslog, network bring-up, DHCP, NTP, service logs, TLS
validation, package tooling, and anything that refuses bogus 1970/2025-ish time.

Limit: we still will not fix timestamps for kernel messages emitted before rc
starts. To get earlier than rc, we would need kernel/loader-level RTC support
or a custom pre-rc hack from /etc/rc itself, which is uglier and easier to break.
For our utility, early rc is the sane point.

- **REQUIRE**: mountcritlocal is usually the sensible earliest point, because `/usr/local/sbin/your-rtc-tool` and required libraries must be available. If your tool is statically linked and stored in `/sbin` or `/bin`, you can try earlier, but I would not unless you have a hard reason.
- **BEFORE**: syslogd ntpd ntpdate netif is the important part. It means logs, network, and time sync daemons start after the RTC has corrected the clock.
- Do NOT use `rc.local` for this. It runs too late for our goal.
- Keep NTP enabled as a second-stage correction after the RTC restore no matter what. The RTC gives sane early timestamps, NTP later gives accurate time.

### `rtc_ntp_watch`

`rtc_ntp_watch` runs

- after `ntpd`, and
- periodically calls `ntp_to_rtc`.

The helper updates the RTC only when `ntpq -pn` shows a selected peer,
and at least one hour has elapsed since the last successful update.

### `rtcsync_shutdown`

`rtcsync_shutdown` updates the RTC during shutdown.

FreeBSD shutdown scripts are run by rc.shutdown with the stop argument,
and scripts need the shutdown keyword to participate.

### `ds3231ctl`

The main utility can:

- read the RTC clock,
- set the system clock from the RTC clock,
- update the RTC clock from the system clock;

it:

- logs warnings and errors to stderr and syslog where useful,
- warns when the RTC year is under 2026,
- refuses to set the system clock from an RTC year under 2026,
- refuses to update the RTC from a system year under 2026,
- skips writes or clock changes when the delta is less than one second,
- reports likely missing RTC or wrong I2C bus/address cleanly.

### Kernel logs

This is an example of kernel logs on a serial console at boot:

```txt
Setting hostid: 0x223aad5d.
GEOM_ELI: Device mirror/swap.eli created.
GEOM_ELI: Encryption: AES-XTS 128
GEOM_ELI:     Crypto: software
Starting file system checks:
/dev/gpt/efiboot0: FILESYSTEM CLEAN; SKIPPING CHECKS
Mounting local filesystems:.
no pools available to import
Setting system clock from DS3231 RTC
Set system clock from RTC: 2026-05-18 01:20:12 UTC
Setting up harvesting: RANDOMDEV,[CALLOUT],[UMA],[FS_ATIME],SWI,INTERRUPT,NET_NG,[NET_ETHER],NET_TUN,MOUSE,KEYBOARD,ATTACH,CACHED
...
Starting sshd.
Starting background file system checks in 60 seconds.

Mon May 18 01:20:48 UTC
FreeBSD/arm64 (Pi-BSD) (ttyu0)

login:
```

No more January 1970 before the login prompt!
The local Unbound resolver can safely use DNSSEC without permissive workaround.

---

## Uninstallation

Run as root in the `RTC` folder:

```sh
sh ./uninstall.sh
```

By default, uninstall leaves boot configuration lines in place
because other I2C devices may need them. To remove them too:

```sh
REMOVE_BOOT_CONFIG=yes ./uninstall.sh
```

---

## TODO

- add support for other RTC chips
- add man page
- add support for DS3231 modules bundled with AT24C32 non-volatile memory

---

## Tests and troubleshooting

### Manual tests with `/dev/iic*`

After rebooting, check that the device exists and that the DS3231 responds:

```sh
# Verify that the I2C bus is detected
ls /dev/iic*
# you should see /dev/iic0

# Scan I2C bus
i2c -f /dev/iic0 -s -v
# The DS3231 should be at the address 0x68

# Try to do a raw read on the DS3231
i2c -a 0x68 -f /dev/iic0 -o 0x00 -c 7 -d r -v
```

```sh
ls /dev/iic*
i2c -f /dev/iic0 -s -v
```

### Using the `ds3231ctl` utility

Example usage
```sh
ds3231ctl read
2000-01-01 00:33:10 UTC

ds3231ctl system-to-rtc
Wrote RTC: 2026-04-27 08:23:55 UTC

ds3231ctl read
2026-04-27 08:23:56 UTC

ds3231ctl rtc-to-system
Set system clock from RTC: 2026-04-27 08:24:32 UTC
```

---

## Licence

This work is licensed under the
Creative Commons Attribution-NonCommercial 4.0 International License.

Copyright (c) 2026 Billie Badin, SIGORYX Engineering
