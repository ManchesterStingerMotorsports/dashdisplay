#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    echo "Run with sudo: sudo ./deploy/restore_desktop.sh"
    exit 1
fi

systemctl disable --now fsdash.service 2>/dev/null || true
setterm -term linux -cursor on < /dev/tty1 > /dev/tty1 2>/dev/null || true
systemctl set-default graphical.target
systemctl enable display-manager.service 2>/dev/null || true
systemctl start display-manager.service 2>/dev/null || true

echo "Restored desktop boot."
