# Real Time Clock

When using your Pi on the go without Internet, it can be convenient to add a cheap `I²C` Real Time Clock such as a `DS3231`.

## Enable `I²C`

```sh
vi /boot/efi/config.txt
```
```conf
dtparam=i2c_arm=on
gpio=2,3=a0
```

```sh
vi /boot/loader.conf
```
````conf
iicbus_load="YES"
iic_load="YES"
```

```sh
sync
reboot
```

## Verification

```sh
# Verify that the I2C bus is detected
ls /dev/iic*
# you should see /dev/iic0

# Scan I2C bus
i2c -f /dev/iic0 -s -v
# The DS3231 should be at the address 0x68

# Try to do a raw read on teh DS3231
i2c -a 0x68 -f /dev/iic0 -o 0x00 -c 7 -d r -v
```

## `ds3231.c`

Copy `ds3231ctl.c` from this repo to your Pi and compile it
```sh
# Compile
cc -Wall -Wextra -O2 -o ds3231ctl ds3231ctl.c

# Install
install -m 0755 ds3231ctl /usr/local/sbin/ds3231ctl
```

Example usage
```sh
./ds3231ctl read
2000-01-01 00:33:10 UTC

./ds3231ctl system-to-rtc
Wrote RTC: 2026-04-27 08:23:55 UTC

./ds3231ctl read
2026-04-27 08:23:56 UTC

./ds3231ctl rtc-to-system
Set system clock from RTC: 2026-04-27 08:24:32 UTC
```


