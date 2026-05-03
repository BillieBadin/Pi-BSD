/*
Utility to read/write DS3231 RTC over I2C bus.
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <dev/iicbus/iic.h>

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DS3231_ADDR (0x68 << 1)

static int bcd_to_int(uint8_t value)
{
    return ((value >> 4) * 10) + (value & 0x0f);
}

static uint8_t int_to_bcd(int value)
{
    return (uint8_t)(((value / 10) << 4) | (value % 10));
}

static void read_regs(int fd, uint8_t reg, uint8_t *buf, size_t len)
{
    struct iic_msg msgs[2];
    struct iic_rdwr_data data;

    msgs[0].slave = DS3231_ADDR;
    msgs[0].flags = IIC_M_WR | IIC_M_NOSTOP;
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].slave = DS3231_ADDR;
    msgs[1].flags = IIC_M_RD;
    msgs[1].len = len;
    msgs[1].buf = buf;

    data.msgs = msgs;
    data.nmsgs = 2;

    if (ioctl(fd, I2CRDWR, &data) < 0) {
        err(1, "I2CRDWR read failed");
    }
}

static void write_regs(int fd, uint8_t reg, uint8_t *buf, size_t len)
{
    uint8_t out[16];
    struct iic_msg msg;
    struct iic_rdwr_data data;

    if (len + 1 > sizeof(out)) {
        errx(1, "write too large");
    }

    out[0] = reg;
    memcpy(&out[1], buf, len);

    msg.slave = DS3231_ADDR;
    msg.flags = IIC_M_WR;
    msg.len = len + 1;
    msg.buf = out;

    data.msgs = &msg;
    data.nmsgs = 1;

    if (ioctl(fd, I2CRDWR, &data) < 0) {
        err(1, "I2CRDWR write failed");
    }
}

static time_t rtc_read_time(int fd)
{
    uint8_t r[7];
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    read_regs(fd, 0x00, r, sizeof(r));

    tm.tm_sec = bcd_to_int(r[0] & 0x7f);
    tm.tm_min = bcd_to_int(r[1] & 0x7f);
    tm.tm_hour = bcd_to_int(r[2] & 0x3f);
    tm.tm_mday = bcd_to_int(r[4] & 0x3f);
    tm.tm_mon = bcd_to_int(r[5] & 0x1f) - 1;
    tm.tm_year = bcd_to_int(r[6]) + 100;
    tm.tm_isdst = 0;

    return timegm(&tm);
}

static void rtc_write_time(int fd, time_t value)
{
    struct tm tm;
    uint8_t r[7];

    if (gmtime_r(&value, &tm) == NULL) {
        err(1, "gmtime_r failed");
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

static void print_utc(time_t value)
{
    struct tm tm;
    char buf[64];

    if (gmtime_r(&value, &tm) == NULL) {
        err(1, "gmtime_r failed");
    }

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm);
    puts(buf);
}

int main(int argc, char **argv)
{
    const char *dev = "/dev/iic0";
    int fd;
    time_t t;
    struct timespec ts;

    if (argc < 2) {
        errx(1, "usage: %s read|system-to-rtc|rtc-to-system [device]", argv[0]);
    }

    if (argc >= 3) {
        dev = argv[2];
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        err(1, "open %s", dev);
    }

    if (strcmp(argv[1], "read") == 0) {
        t = rtc_read_time(fd);
        print_utc(t);
    } else if (strcmp(argv[1], "system-to-rtc") == 0) {
        t = time(NULL);
        rtc_write_time(fd, t);
        printf("Wrote RTC: ");
        print_utc(t);
    } else if (strcmp(argv[1], "rtc-to-system") == 0) {
        t = rtc_read_time(fd);
        ts.tv_sec = t;
        ts.tv_nsec = 0;

        if (clock_settime(CLOCK_REALTIME, &ts) < 0) {
            err(1, "clock_settime failed");
        }

        printf("Set system clock from RTC: ");
        print_utc(t);
    } else {
        errx(1, "unknown command: %s", argv[1]);
    }

    close(fd);
    return 0;
}
