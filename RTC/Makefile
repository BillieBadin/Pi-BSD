# Pi-BSD - a FreeBSD native stack for Raspberry Pi.
# SPDX-License-Identifier: CC BY-NC 4.0
# SPDX-FileCopyrightText: 2026 Billie Badin, SIGORYX Engineering

PREFIX?=/usr/local
CC?=cc
CFLAGS?=-Wall -Wextra -O2

all: ds3231ctl

ds3231ctl: src/ds3231ctl.c
	$(CC) $(CFLAGS) -o ds3231ctl src/ds3231ctl.c

install: all
	install -m 0755 ds3231ctl $(PREFIX)/sbin/ds3231ctl
	install -m 0755 sbin/ntp_to_rtc $(PREFIX)/sbin/ntp_to_rtc
	install -m 0700 rc.d/earlyrtc /etc/rc.d/earlyrtc
	install -m 0700 rc.d/rtc_ntp_watch /etc/rc.d/rtc_ntp_watch
	install -m 0700 rc.d/rtcsync_shutdown /etc/rc.d/rtcsync_shutdown

clean:
	rm -f ds3231ctl
