#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    echo "Run with sudo: sudo ./deploy/restore_desktop.sh"
    exit 1
fi

systemctl disable --now fsdash.service 2>/dev/null || true
setterm -term linux -cursor on < /dev/tty1 > /dev/tty1 2>/dev/null || true
for cmdline in /boot/firmware/cmdline.txt /boot/cmdline.txt; do
    if [ -f "$cmdline" ]; then
        if grep -qw "vt.global_cursor_default=0" "$cmdline"; then
            cp "$cmdline" "$cmdline.restore-desktop.bak"
            tmp="$(mktemp)"
            sed "s/ *vt.global_cursor_default=0//g" "$cmdline" > "$tmp"
            cat "$tmp" > "$cmdline"
            rm -f "$tmp"
            echo "Removed vt.global_cursor_default=0 from $cmdline"
        fi
        break
    fi
done
systemctl set-default graphical.target
systemctl enable display-manager.service 2>/dev/null || true
systemctl start display-manager.service 2>/dev/null || true

echo "Restored desktop boot."
