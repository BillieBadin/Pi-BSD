// Pi-BSD - a FreeBSD native stack for Raspberry Pi.
// SPDX-License-Identifier: CC BY-NC 4.0
// SPDX-FileCopyrightText: 2026 Billie Badin, SIGORYX Engineering

/*
Do not send all warnings to syslog such as command line syntax.

*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <dev/iicbus/iic.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define DS3231_ADDR (0x68 << 1)
#define MIN_SAFE_YEAR 2026


// -----------------------------------------------------------------------------
static int bcd_to_int(uint8_t value) {
    // Convert BCD register value to integer

    return ((value >> 4) * 10) + (value & 0x0f);
}

// -----------------------------------------------------------------------------
static uint8_t int_to_bcd(int value) {
    // Convert integer to DS3231 BCD format

    return (uint8_t)(((value / 10) << 4) | (value % 10));
}

// -----------------------------------------------------------------------------
static void vlog_stderr(const char *prefix, const char *fmt, va_list ap) {
    // Print local warning/error text only

    fprintf(stderr, "ds3231ctl: %s", prefix);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

static void warn_log(const char *fmt, ...) {
    // Emit non-fatal warning to stderr
    va_list ap;

    va_start(ap, fmt);
    vlog_stderr("warning: ", fmt, ap);
    va_end(ap);
}

// -----------------------------------------------------------------------------
static void err_log(int eval, const char *fmt, ...) {
    // Emit fatal error to stderr and exit

    va_list ap;

    va_start(ap, fmt);
    vlog_stderr("error: ", fmt, ap);
    va_end(ap);
    exit(eval);
}

// -----------------------------------------------------------------------------
static int utc_year(time_t value) {
    // Extract UTC year from a timestamp

    struct tm tm;

    if (gmtime_r(&value, &tm) == NULL) {
        err_log(1, "gmtime_r failed");
    }
    return tm.tm_year + 1900;
}

// -----------------------------------------------------------------------------
static void read_regs(int fd, uint8_t reg, uint8_t *buf, size_t len) {
    // Read one or more RTC registers over I2C

    struct iic_msg msgs[2];
    struct iic_rdwr_data data;

    msgs[0].slave = DS3231_ADDR;
    msgs[0].flags = IIC_M_WR | IIC_M_NOSTOP;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg;

    msgs[1].slave = DS3231_ADDR;
    msgs[1].flags = IIC_M_RD;
    msgs[1].len   = len;
    msgs[1].buf   = buf;

    data.msgs     = msgs;
    data.nmsgs    = 2;

    if (ioctl(fd, I2CRDWR, &data) < 0) {
        err_log(1, "I2CRDWR read failed, DS3231 missing or I2C bus/address wrong: %s", strerror(errno));
    }
}

// -----------------------------------------------------------------------------
static void write_regs(int fd, uint8_t reg, uint8_t *buf, size_t len) {
    // Write one or more RTC registers over I2C

    uint8_t out[16];
    struct  iic_msg msg;
    struct  iic_rdwr_data data;

    if (len + 1 > sizeof(out)) {
        err_log(1, "write too large");
    }

    out[0]     = reg;
    memcpy(&out[1], buf, len);

    msg.slave  = DS3231_ADDR;
    msg.flags  = IIC_M_WR;
    msg.len    = len + 1;
    msg.buf    = out;

    data.msgs  = &msg;
    data.nmsgs = 1;

    if (ioctl(fd, I2CRDWR, &data) < 0) {
        err_log(1, "I2CRDWR write failed, DS3231 missing or I2C bus/address wrong: %s", strerror(errno));
    }
}

// -----------------------------------------------------------------------------
static time_t rtc_read_time(int fd) {
    // Read RTC calendar registers and decode UTC time

    uint8_t r[7];
    struct  tm tm;
    time_t  value;

    memset(&tm, 0, sizeof(tm));
    read_regs(fd, 0x00, r, sizeof(r));

    tm.tm_sec   = bcd_to_int(r[0] & 0x7f);
    tm.tm_min   = bcd_to_int(r[1] & 0x7f);
    tm.tm_hour  = bcd_to_int(r[2] & 0x3f);
    tm.tm_mday  = bcd_to_int(r[4] & 0x3f);
    tm.tm_mon   = bcd_to_int(r[5] & 0x1f) - 1;
    tm.tm_year  = bcd_to_int(r[6]) + 100;
    tm.tm_isdst = 0;

    value = timegm(&tm);
    if (value == (time_t)-1) {
        err_log(1, "RTC returned an invalid date");
    }

    if (utc_year(value) < MIN_SAFE_YEAR) {
        warn_log("RTC year is under %d", MIN_SAFE_YEAR);
    }

    return value;
}

// -----------------------------------------------------------------------------
static void rtc_write_time(int fd, time_t value) {
    // Encode UTC time and store it into RTC registers

    struct  tm tm;
    uint8_t r[7];

    if (gmtime_r(&value, &tm) == NULL) {
        err_log(1, "gmtime_r failed");
    }

    r[0] = int_to_bcd(tm.tm_sec);
    r[1] = int_to_bcd(tm.tm_min);
    r[2] = int_to_bcd(tm.tm_hour);
    r[3] = int_to_bcd(tm.tm_wday + 1);
    r[4] = int_to_bcd(tm.tm_mday);
    r[5] = int_to_bcd(tm.tm_mon + 1);
    r[6] = int_to_bcd(tm.tm_year - 100);

    write_regs(fd, 0x00, r, sizeof(r));
}

// -----------------------------------------------------------------------------
static void print_utc(time_t value) {
    // Print timestamp in stable UTC text format

    struct tm tm;
    char   buf[64];

    if (gmtime_r(&value, &tm) == NULL) {
        err_log(1, "gmtime_r failed");
    }

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm);
    puts(buf);
}

// -----------------------------------------------------------------------------
static time_t checked_system_time(void) {
    // Read system clock and enforce minimum year guard

    time_t value;

    value = time(NULL);
    if (value == (time_t)-1) {
        err_log(1, "time failed");
    }

    if (utc_year(value) < MIN_SAFE_YEAR) {
        err_log(1, "system year is under %d, refusing to update RTC", MIN_SAFE_YEAR);
    }

    return value;
}

// -----------------------------------------------------------------------------
static int delta_less_than_one_second(time_t a, time_t b) {
    // test if absolute difference between two timestamps is less than 1 second

    time_t delta;

    delta = a - b;
    if (delta < 0) {
        delta = -delta;
    }

    return delta < 1;
}

// -----------------------------------------------------------------------------
int main(int argc, char **argv) {
    // Parse command, then sync/read RTC as requested

    const char *dev = "/dev/iic0";
    int    fd;
    time_t rtc_t;
    time_t sys_t;
    struct timespec ts;

    openlog("ds3231ctl", LOG_PID, LOG_DAEMON);

    if (argc < 2) {
        err_log(1, "usage: %s read|system-to-rtc|rtc-to-system [device]", argv[0]);
    }

    if (argc >= 3) {
        dev = argv[2];
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        err_log(1, "open %s failed: %s", dev, strerror(errno));
    }

    if (strcmp(argv[1], "read") == 0) {
        rtc_t = rtc_read_time(fd);
        print_utc(rtc_t);
    } else if (strcmp(argv[1], "system-to-rtc") == 0) {
        sys_t = checked_system_time();
        rtc_t = rtc_read_time(fd);

        if (delta_less_than_one_second(sys_t, rtc_t)) {
            printf("RTC already within 1 second: ");
            print_utc(rtc_t);
            syslog(LOG_INFO, "RTC update skipped because delta is less than 1 second");
        } else {
            rtc_write_time(fd, sys_t);
            printf("Wrote RTC: ");
            print_utc(sys_t);
            syslog(LOG_INFO, "RTC updated from system clock");
        }
    } else if (strcmp(argv[1], "rtc-to-system") == 0) {
        rtc_t = rtc_read_time(fd);

        if (utc_year(rtc_t) < MIN_SAFE_YEAR) {
            err_log(1, "RTC year is under %d, refusing to set system clock", MIN_SAFE_YEAR);
        }

        sys_t = time(NULL);
        if (sys_t == (time_t)-1) {
            err_log(1, "time failed");
        }

        if (delta_less_than_one_second(rtc_t, sys_t)) {
            printf("System clock already within 1 second of RTC: ");
            print_utc(sys_t);
            syslog(LOG_INFO, "system clock update skipped because delta is less than 1 second");
        } else {
            ts.tv_sec  = rtc_t;
            ts.tv_nsec = 0;

            if (clock_settime(CLOCK_REALTIME, &ts) < 0) {
                err_log(1, "clock_settime failed: %s", strerror(errno));
            }

            printf("Set system clock from RTC: ");
            print_utc(rtc_t);
            syslog(LOG_INFO, "system clock updated from RTC");
        }
    } else {
        err_log(1, "unknown command: %s", argv[1]);
    }

    close(fd);
    closelog();
    return 0;
}
// -----------------------------------------------------------------------------
