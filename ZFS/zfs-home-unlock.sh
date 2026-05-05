#!/bin/sh

case "$USER" in
    root|"")
        exit 0
        ;;
esac

DATASET="zroot/home/$USER"

if ! /sbin/zfs list -H "$DATASET" >/dev/null 2>&1; then
    exit 0
fi

if [ "$(/sbin/zfs get -H -o value keystatus "$DATASET" 2>/dev/null)" = "available" ]; then
    exit 0
fi

exec /usr/local/bin/sudo /sbin/zfs mount -l "$DATASET"
