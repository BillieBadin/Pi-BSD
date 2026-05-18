#!/bin/sh
# Pi-BSD - a FreeBSD native stack for Raspberry Pi.
# SPDX-License-Identifier: CC BY-NC 4.0
# SPDX-FileCopyrightText: 2026 Billie Badin, SIGORYX Engineering

set -eu

PREFIX="${PREFIX:-/usr/local}"
REMOVE_BOOT_CONFIG="${REMOVE_BOOT_CONFIG:-no}"

need_root()
{
    if [ "$(id -u)" -ne 0 ]; then
        echo "uninstall.sh must be run as root" >&2
        exit 1
    fi
}

remove_line()
{
    file="$1"
    line="$2"
    tmp="${file}.tmp.$$"

    if [ -f "${file}" ]; then
        grep -Fvx "${line}" "${file}" > "${tmp}" || true
        cat "${tmp}" > "${file}"
        rm -f "${tmp}"
    fi
}

need_root

service rtc_ntp_watch stop >/dev/null 2>&1 || true

sysrc -x earlyrtc_enable >/dev/null 2>&1 || true
sysrc -x rtc_ntp_watch_enable >/dev/null 2>&1 || true
sysrc -x rtc_iic_dev >/dev/null 2>&1 || true
sysrc -x earlyrtc_iic_dev >/dev/null 2>&1 || true
sysrc -x rtcsync_shutdown_iic_dev >/dev/null 2>&1 || true

rm -f "${PREFIX}/sbin/ds3231ctl"
rm -f "${PREFIX}/sbin/ntp_to_rtc"
rm -f /etc/rc.d/earlyrtc
rm -f /etc/rc.d/rtc_ntp_watch
rm -f /etc/rc.d/rtcsync_shutdown
rm -f /var/run/rtc-last-sync /var/run/rtc_ntp_watch.pid

if [ "${REMOVE_BOOT_CONFIG}" = "yes" ]; then
    remove_line /boot/loader.conf 'iicbus_load="YES"'
    remove_line /boot/loader.conf 'iic_load="YES"'
    remove_line /boot/efi/config.txt 'dtparam=i2c_arm=on'
    remove_line /boot/efi/config.txt 'gpio=2,3=a0'
fi

logger -t ds3231-installer "DS3231 RTC helpers uninstalled"

echo "Uninstalled DS3231 RTC helpers."
if [ "${REMOVE_BOOT_CONFIG}" != "yes" ]; then
    echo "Boot configuration lines were left in place. Use REMOVE_BOOT_CONFIG=yes ./uninstall.sh to remove them."
fi
