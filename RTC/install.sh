#!/bin/sh
# Pi-BSD - a FreeBSD native stack for Raspberry Pi.
# SPDX-License-Identifier: CC BY-NC 4.0
# SPDX-FileCopyrightText: 2026 Billie Badin, SIGORYX Engineering

set -eu

PREFIX="${PREFIX:-/usr/local}"
IIC_DEV="${IIC_DEV:-/dev/iic0}"
ENABLE_SERVICES="${ENABLE_SERVICES:-yes}"
ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

need_root()
{
    if [ "$(id -u)" -ne 0 ]; then
        echo "install.sh must be run as root" >&2
        exit 1
    fi
}

append_once()
{
    file="$1"
    line="$2"

    touch "${file}"
    if ! grep -Fqx "${line}" "${file}"; then
        printf '%s\n' "${line}" >> "${file}"
    fi
}

backup_file()
{
    file="$1"

    if [ -f "${file}" ]; then
        cp -p "${file}" "${file}.ds3231rtc.bak.$(date -u +%Y%m%d%H%M%S)"
    fi
}

need_root
cd "${ROOT_DIR}"

make clean >/dev/null 2>&1 || true
make all
make install PREFIX="${PREFIX}"

backup_file /boot/loader.conf
append_once /boot/loader.conf 'iicbus_load="YES"'
append_once /boot/loader.conf 'iic_load="YES"'

if [ -f /boot/efi/config.txt ]; then
    backup_file /boot/efi/config.txt
    append_once /boot/efi/config.txt 'dtparam=i2c_arm=on'
    append_once /boot/efi/config.txt 'gpio=2,3=a0'
else
    echo "warning: /boot/efi/config.txt not found; Raspberry Pi firmware I2C settings were not installed" >&2
fi

if [ "${ENABLE_SERVICES}" = "yes" ]; then
    sysrc earlyrtc_enable=YES
    sysrc rtc_ntp_watch_enable=YES
    sysrc rtc_iic_dev="${IIC_DEV}"
    sysrc earlyrtc_iic_dev="${IIC_DEV}"
    sysrc rtcsync_shutdown_iic_dev="${IIC_DEV}"
fi

logger -t ds3231-installer "DS3231 RTC helpers installed"

echo "Installed DS3231 RTC helpers. Reboot is recommended so loader.conf and config.txt changes take effect."
echo "Test manually with: ${PREFIX}/sbin/ds3231ctl read ${IIC_DEV}"
