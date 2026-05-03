# bastille jails

## Set up `bastille`

```sh
pkg install bastille
sysrc bastille_enable=YES

zfs create -o compression=zstd zroot/bastille
zfs set mountpoint=/usr/local/bastille zroot/bastille

zfs create zroot/bastille/jails
zfs set mountpoint=/usr/local/bastille/jails zroot/bastille/jails

zfs create zroot/bastille/releases
zfs set mountpoint=/usr/local/bastille/releases zroot/bastille/releases

zfs create zroot/bastille/logs
zfs set mountpoint=/usr/local/bastille/logs zroot/bastille/logs
```

## Create your first jail

This example assumes a `wg1` Wireguard interface was create as documented [here](./WIreGuard.md).

```sh
bastille bootstrap 15.0-RELEASE-p8

bastille create base 15.0-RELEASE N.N.N.N/M wg1

vi /usr/local/bastille/jails/base/jail.conf
```
```conf
base {
    host.hostname = "Pi-jail.local";
    path = "/usr/local/bastille/jails/base/root";

    exec.clean;
    exec.start = "/bin/sh /etc/rc";
    exec.stop = "/bin/sh /etc/rc.shutdown jail";

    mount.devfs;

    interface = "wg1";
    ip4.addr = "N.N.N.N/M";
    ip6 = "disable";

    allow.raw_sockets = 1;
    enforce_statfs = 2;
}
```
```sh
bastille start base
bastille list
```
