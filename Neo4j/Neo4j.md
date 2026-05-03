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
