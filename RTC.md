# Real Time Clock

When using your Pi on the go without Internet, it can be convenient to add a cheap `I2S` Real Time Clock such as a `DS3231`.

## Enable `I2C`

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

Copy `ds3231ctl.c` from this repo to your Pi and compile it
```sh
cc -Wall -Wextra -O2 -o ds3231ctl ds3231ctl.c
```
