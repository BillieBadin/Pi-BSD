#!/bin/sh

set -u

SYS_TO_RTC="/root/sys_to_rtc.sh"
MAX_WAIT_SECONDS="300"
SLEEP_SECONDS="5"

elapsed=0

echo "Waiting for NTP synchronisation before updating DS3231 RTC..."

while [ "${elapsed}" -lt "${MAX_WAIT_SECONDS}" ]; do
    if ntpq -pn 2>/dev/null | awk '/^\*/ { found = 1 } END { exit found ? 0 : 1 }'; then
        echo "NTP is synchronised."
        ntpq -pn

        echo "Updating RTC from system clock if needed..."
        exec "${SYS_TO_RTC}"
    fi

    sleep "${SLEEP_SECONDS}"
    elapsed=$((elapsed + SLEEP_SECONDS))
done

echo "NTP did not synchronise within ${MAX_WAIT_SECONDS} seconds. RTC not updated."
exit 1

