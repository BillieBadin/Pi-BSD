# Neo4j

```sh
pkg install bash

zfs create zroot/home/neo4j
zfs get mountpoint zroot/home/neo4j
NAME              PROPERTY    VALUE        SOURCE
zroot/home/neo4j  mountpoint  /home/neo4j  inherited from zroot/home

pw useradd neo4j -m -d /home/neo4j -s /usr/local/bin/bash -c "Neo4j"
passwd neo4j

chown -R neo4j:neo4j /home/neo4j
chmod o-rwx /home/neo4j

# Starting with Neo4j 2025.10, Java 25 is also supported.
pkg install openjdk25-jre-headless

# Message from openjdk25-jre-headless-25.0.2+10.f.1_2:
# This OpenJDK implementation may require procfs(5) mounted on /proc.
# If you have not done it yet, please do the following:
#         mount -t procfs proc /proc
# To make it permanent, you need the following lines in /etc/fstab:
#         proc    /proc           procfs          rw      0       0
mount -t procfs proc /proc
echo 'proc    /proc           procfs          rw      0       0' >> /etc/fstab
mount | grep procfs
ls /proc/self/status

java -version
# openjdk version "25.0.2" 2026-01-20
# OpenJDK Runtime Environment (build 25.0.2+10-freebsd-1)
# OpenJDK 64-Bit Server VM (build 25.0.2+10-freebsd-1, mixed mode, sharing)

su neo4j
mkdir -p /home/neo4j/{bin,run,logs,data,import,conf,plugins,lib,dist}
cd /home/neo4j/dist
fetch https://dist.neo4j.org/neo4j-community-2026.04.0-unix.tar.gz
tar -xzf neo4j-community-2026.04.0-unix.tar.gz
rm neo4j-community-2026.04.0-unix.tar.gz
ln -s neo4j-community-2026.04.0 neo4j
```

```sh
vi /home/neo4j/dist/neo4j/conf/neo4j.conf
```
```conf
server.directories.data=/home/neo4j/data
server.directories.logs=/home/neo4j/logs
server.directories.import=/home/neo4j/import
server.directories.plugins=/home/neo4j/plugins
server.directories.lib=/home/neo4j/lib
server.default_listen_address=0.0.0.0
server.bolt.listen_address=:7687
server.http.listen_address=:7474
```

## Control script

```sh
vi /home/neo4j/bin/neo4j.sh
```
```sh
#!/bin/sh

NEO4J_HOME="/home/neo4j/dist/neo4j"
PIDFILE="/home/neo4j/run/neo4j.pid"

case "$1" in
    start)
        echo "Starting Neo4j"
        $NEO4J_HOME/bin/neo4j start
        ;;
    stop)
        echo "Stopping Neo4j"
        $NEO4J_HOME/bin/neo4j stop
        ;;
    console)
        $NEO4J_HOME/bin/neo4j console
        ;;
    status)
        $NEO4J_HOME/bin/neo4j status
        ;;
    *)
        echo "Usage: $0 {start|stop|console|status}"
        exit 1
        ;;
esac
```
Make it executable:
```sh
chmod +x /home/neo4j/bin/neo4j.sh
```

First launch of Neo4j in the console:
```sh
/home/neo4j/bin/neo4j.sh console
Directories in use:
home:         /home/neo4j/dist/neo4j
config:       /home/neo4j/dist/neo4j/conf
logs:         /home/neo4j/logs
plugins:      /home/neo4j/plugins
import:       /home/neo4j/import
data:         /home/neo4j/data
certificates: /home/neo4j/dist/neo4j/certificates
licenses:     /home/neo4j/dist/neo4j/licenses
run:          /home/neo4j/run
Starting Neo4j.
2026-04-24 00:42:25.589+0000 INFO  Logging config in use: File '/home/neo4j/dist/neo4j/conf/user-logs.xml'
2026-04-24 00:42:25.658+0000 INFO  Starting...
2026-04-24 00:42:29.606+0000 INFO  This instance is ServerId{5cd40314} (5cd40314-9622-4fc4-a177-9ceef702566f)
2026-04-24 00:42:36.666+0000 INFO  ======== Neo4j 2026.04.0 ========
2026-04-24 00:42:39.224+0000 INFO  X-Forward header processing enabled with security controls
2026-04-24 00:42:39.225+0000 WARN  SECURITY WARNING: X-Forward headers accepted from any source - configure allow_proxies
2026-04-24 00:42:39.227+0000 WARN  SECURITY WARNING: X-Forward headers accept any hostname - configure allow_hosts
2026-04-24 00:42:45.582+0000 INFO  This server has built-in support for online monitoring through Fleet Manager. Visit https://console.neo4j.io to get started!
2026-04-24 00:42:47.905+0000 INFO  Bolt enabled on 0.0.0.0:7687.
2026-04-24 00:42:55.124+0000 INFO  HTTP enabled on 0.0.0.0:7474.
2026-04-24 00:42:55.128+0000 INFO  Remote interface available at http://localhost:7474/
2026-04-24 00:42:55.141+0000 INFO  id:*****
2026-04-24 00:42:55.143+0000 INFO  name: system
2026-04-24 00:42:55.144+0000 INFO  creationDate: 2026-04-24T00:42:41.845Z
2026-04-24 00:42:55.146+0000 INFO  Started.
```

Browse to: http://<pi-address>:7474/

## Auto-start service

```sh
vi /usr/local/etc/rc.d/neo4j
```
```sh
#!/bin/sh

# PROVIDE: neo4j
# REQUIRE: LOGIN
# KEYWORD: shutdown

. /etc/rc.subr

name="neo4j"
rcvar="neo4j_enable"

load_rc_config "$name"

: ${neo4j_enable:="NO"}
: ${neo4j_user:="neo4j"}
: ${neo4j_home:="/home/neo4j"}
: ${neo4j_java_home:="/usr/local/openjdk25-jre-headless"}

command="${neo4j_home}/bin/neo4j.sh"

start_cmd="neo4j_start"
stop_cmd="neo4j_stop"
status_cmd="neo4j_status"

neo4j_run()
{
    su -m "${neo4j_user}" -c "env JAVA_HOME='${neo4j_java_home}' PATH='${neo4j_java_home}/bin:/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin' '${command}' $1"
}

neo4j_start()
{
    neo4j_run start
}

neo4j_stop()
{
    neo4j_run stop
}

neo4j_status()
{
    neo4j_run status
}

run_rc_command "$1"
```

Enable new service:
```sh
chmod +x /usr/local/etc/rc.d/neo4j
sysrc neo4j_enable=YES
service neo4j start
```

## Backup Neo4j (ZFS-style)

```sh
service neo4j stop
pgrep -u neo4j java
zfs snapshot zroot/home/neo4j@neo4j-$(date +%Y%m%d-%H%M)

zfs list -t snapshot -r zroot/home/neo4j
zfs list -t snapshot -r -o name,creation zroot/home/neo4j
```

