# WireGuard

```sh
pkg install wireguard-tools
```

```sh
/boot/loader.conf
```
```conf
if_wg_load="YES"
```

```sh
mkdir /usr/local/etc/wireguard
vi /usr/local/etc/wireguard/wg1.conf
```
```conf
[Interface]
# Address = address is irrelevant here, it is configured on the interface itself
PrivateKey = <your_private_key>

[Peer]
PublicKey = <peer_public__key>
AllowedIPs = ...
Endpoint = ...
```

```sh
chmod 600 /usr/local/etc/wireguard/wg1.conf

ifconfig wg1 create
sudo ifconfig wg1 inet N.N.N.N/M
wg setconf wg1 /usr/local/etc/wireguard/wg2.conf
ifconfig wg1 up
```

Make it persistent
Add to `/etc/rc.conf`:
```conf
# change DHCP to sync
# ifconfig_genet0="DHCP"
ifconfig_genet0="SYNCDHCP"

# WireGuard
cloned_interfaces="wg1"
ifconfig_wg1="inet  N.N.N.N/M"
```

Create hook
```sh
vi /etc/start_if.wg1
```
```conf
#!/bin/sh
/usr/bin/wg setconf $1 /usr/local/etc/wireguard/$1.conf
```
```sh
chmod +x /etc/start_if.wg2

# Full reboot
reboot

# Manually simulate boot sequence from a network PoV
service netif restart
```

Check after reboot:
```sh
ifconfig genet0
netstat -rn
ifconfig wg1
wg show wg1
```

