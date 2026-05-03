#!/bin/sh

set -u

DS3231CTL="/usr/local/sbin/ds3231ctl"
MAX_DIFF_SECONDS="2"


ntpd_is_synchronised()
{
    ntpq -pn 2>/dev/null | awk '
        /^\*/ {
            found = 1
        }
        END {
            exit found ? 0 : 1
        }
    '
}

if ntpd_is_synchronised; then
    echo "NTP is already synchronised. Not setting system clock from RTC."
    ntpq -pn
    exit 0
fi


system_epoch=$(date -u "+%s")

rtc_text=$("${DS3231CTL}" read) || {
    echo "ERROR: failed to read DS3231 RTC." >&2
    exit 1
}

rtc_epoch=$(date -j -u -f "%Y-%m-%d %H:%M:%S %Z" "${rtc_text}" "+%s") || {
    echo "ERROR: failed to parse RTC time: ${rtc_text}" >&2
    exit 1
}

diff=$((system_epoch - rtc_epoch))

if [ "${diff}" -lt 0 ]; then
    abs_diff=$((0 - diff))
else
    abs_diff="${diff}"
fi

echo "System time: $(date -u "+%Y-%m-%d %H:%M:%S UTC")"
echo "RTC time:    ${rtc_text}"
echo "Difference:  ${diff} seconds"

if [ "${abs_diff}" -le "${MAX_DIFF_SECONDS}" ]; then
    echo "RTC is already close enough. Not updating."
    exit 0
fi

echo "RTC differs by more than ${MAX_DIFF_SECONDS} seconds. Updating RTC..."

if "${DS3231CTL}" system-to-rtc; then
    echo "RTC updated from system clock."
    exit 0
fi

echo "ERROR: failed to update DS3231 RTC from system clock." >&2
exit 1



