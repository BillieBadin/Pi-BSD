# `pf` (Packet Filter) firewall

The following example blocks a certain (jailed) user from going outside of the jail network.

It complements both [WireGuard](../WireGuard/WireGuard.md) and [bastille](../bastille/bastille.md) examples in this repository.

```sh
kldload pf
ls /dev/pf

vi /etc/pf.conf
```
```conf
wg1_if = "wg1"
wg1_net = "10.0.0.0/24"
wg1_ip = "10.0.0.1"

wg1_users = "{ alice }"

# Allowed hosts (TCP/UDP only)
table <wg1_allowed_hosts> {
    10.0.0.2
    10.0.0.3
}

# Do not filter loopback
set skip on lo0

# Keep existing connections alive
pass in quick proto tcp from any to any port 22 flags S/SA keep state

# Allow ICMP to entire subnet
pass out quick on $wg1_if proto icmp \
    from any to $wg1_net user $wg1_users keep state

# Allow TCP/UDP only to specific hosts
pass out quick on $wg1_if proto { tcp udp } \
    from any to <wg1_allowed_hosts> user $wg1_users keep state

# Block everything else for those users
block out quick user $wg1_users

# Permissive
pass out quick keep state

# Allow all inbound by default for now
pass in quick keep state
```

```sh
pfctl -nf /etc/pf.conf

service pf onestart

pfctl -f /etc/pf.conf

pfctl -e
pfctl -s rules
pfctl -s info

# Enable / persist
sysrc pf_enable=YES
sysrc pf_rules="/etc/pf.conf"
sysrc pflog_enable=YES
```
