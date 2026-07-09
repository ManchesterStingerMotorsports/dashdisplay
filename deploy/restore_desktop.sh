#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    echo "Run with sudo: sudo ./deploy/restore_desktop.sh"
    exit 1
fi

systemctl disable --now fsdash.service 2>/dev/null || true
setterm -term linux -cursor on < /dev/tty1 > /dev/tty1 2>/dev/null || true
boot_args="vt.global_cursor_default=0 quiet loglevel=3 systemd.show_status=false logo.nologo"
for cmdline in /boot/firmware/cmdline.txt /boot/cmdline.txt; do
    if [ -f "$cmdline" ]; then
        cp "$cmdline" "$cmdline.restore-desktop.bak"
        tmp="$(mktemp)"
        new_cmdline=""
        for existing_arg in $(cat "$cmdline"); do
            keep=1
            for arg in $boot_args; do
                if [ "$existing_arg" = "$arg" ]; then
                    keep=0
                    break
                fi
            done
            if [ "$keep" -eq 1 ]; then
                new_cmdline="$new_cmdline $existing_arg"
            fi
        done
        printf "%s\n" "${new_cmdline# }" > "$tmp"
        cat "$tmp" > "$cmdline"
        rm -f "$tmp"
        echo "Removed fsdash boot args from $cmdline"
        break
    fi
done
systemctl set-default graphical.target
systemctl enable getty@tty1.service 2>/dev/null || true
systemctl enable display-manager.service 2>/dev/null || true
systemctl start display-manager.service 2>/dev/null || true

echo "Restored desktop boot."
