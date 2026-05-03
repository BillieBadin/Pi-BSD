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
