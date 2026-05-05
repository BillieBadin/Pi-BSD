# ZFS tuning

## ZFS arc tuning for low memory environment

Pi Zero 2 W has only 512MB RAM so the default are a bit aggressive.
The following settings will help find a good balance.

Set new levels:
```sh
cat >> /boot/loader.conf <<'EOF'

# 32MB min
vfs.zfs.arc.min="33554432"

# ZFS ARC tuning for 512 MB RAM embedded system
# 96MB max
vfs.zfs.arc.max="100663296"
# 128MB max if required
# vfs.zfs.arc.max="134217728"

# Reduce speculative read pressure
vfs.zfs.prefetch.disable="1"
EOF
```

## Better bitrot resilience by adding redundancy at the vdev level

Where full ahrdware redundancy isn't possible, a compromise exists to add a bit more resilience.
The `copies` property copies each block as many times as requested in different locations on the same vdev.
This helps against localised bad sectors and some forms of silent corruption but will obviously not protect the failure of a complete storage device.

```sh
zfs set copies=2 zroot/home/bb
zfs get copies zroot/home/bb
```

