# ZFS

By default, your user cannot mount its home directory without using sudo, and it won't be automatic on reboot.

First check that things work manually:
```sh
sudo zfs load-key zroot/home/<your-user-name>
sudo zfs mount zroot/home/<your-user-name>
```

## Configure `sshd`

You won't be able to use public key authentication unless your user's `~/.ssh/authorized_keys` is available which requires connecting and unlocking your encrypted ZFS dataset.

Configure `sshd` to use `authorized_keys` from outside your home directory.
```sh
# Create a folder for the out-of-home-directory authorized_keyfile
mkdir -p /etc/ssh/authorized_keys
chmod 755 /etc/ssh/authorized_keys

# relocated your authorized_keys
mv ~<your-user-name>/.ssh/authorized_keys /etc/ssh/authorized_keys/<your-user-name>

vi /etc/ssh/sshd_config
```
Add/update:
```conf
AuthorizedKeysFile      /etc/ssh/authorized_keys/%u .ssh/authorized_keys
PubkeyAuthentication yes
```
Restart sshd:
```sh
service sshd restart
```

## ZFS unlock hook

### Allow users to load key and mount via sudo

```sh
zfs get mounted,mountpoint,canmount zroot/home/<your-user-name>
zfs allow <your-user-name> load-key,mount zroot/home/<your-user-name>
zfs allow zroot/home/<your-user-name>
zfs get mounted,mountpoint,canmount zroot/home/<your-user-name>

vi /usr/local/etc/sudoers.d/zfs-home
```
```conf
<your-user-name> ALL=(root) NOPASSWD: /sbin/zfs mount -l zroot/home/<your-user-name>
```
```sh
chmod 440 /usr/local/etc/sudoers.d/zfs-home
```


### Create an ZFS unlock hook script

```sh
vi /usr/local/libexec/zfs-home-unlock.sh
```
```sh
#!/bin/sh

case "$-" in
    *i*) ;;
    *) exit 0 ;;
esac

DATASET="zroot/home/$USER"

case "$USER" in
    root|"")
        exit 0
        ;;
esac

if ! zfs list -H "$DATASET" >/dev/null 2>&1; then
    exit 0
fi

if [ "$(zfs get -H -o value keystatus "$DATASET" 2>/dev/null)" = "available" ]; then
    exit 0
fi

printf "Unlock ZFS home for %s: " "$USER"
stty -echo
IFS= read -r PASSPHRASE
stty echo
printf "\n"

printf '%s\n' "$PASSPHRASE" | zfs load-key "$DATASET"
zfs mount "$DATASET" 2>/dev/null

unset PASSPHRASE
```
```sh
chmod 755 /usr/local/libexec/zfs-home-unlock.sh
```

### Hook the script

```sh
mkdir -p /usr/local/etc/profile.d
chmod 755 /usr/local/etc/profile.d

vi /usr/local/etc/profile.d/zfs-home-unlock.sh
```
```sh
#!/bin/sh
if [ -x /usr/local/etc/zfs-home-unlock.sh ]; then
    /usr/local/etc/zfs-home-unlock.sh
fi
```
```sh
chmod 755 /usr/local/etc/profile.d/zfs-home-unlock.sh
```

From now on, when you log-on - whether it is with passowrd or public key authentication - the hook will be invoked and if your home directory is encrypted and locked, you will be prompted for your password to unlock it.

## Harden SSH

```sh
vi /etc/ssh/sshd_config
```
Add/update:
```conf
PasswordAuthentication no
```
Restart sshd:
```sh
service sshd restart
```

## Activate zstd compression

ZFS ships with `zstd` which is a very good compromise between compression ratio and CPU usage.

```sh
zfs set compression=zstd zroot/<dataset>
```
