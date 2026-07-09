#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    echo "Run with sudo: sudo ./deploy/install_boot_service.sh"
    exit 1
fi

if [ ! -x /home/stinger/dash/fsdash ]; then
    echo "Missing /home/stinger/dash/fsdash"
    echo "Build it first with: cd /home/stinger/dash && make UI_BACKEND=fbdev"
    exit 1
fi

cp /home/stinger/dash/deploy/fsdash.service /etc/systemd/system/fsdash.service
systemctl daemon-reload

systemctl disable --now display-manager.service 2>/dev/null || true
systemctl set-default multi-user.target
systemctl enable fsdash.service

echo "Installed fsdash boot service."
echo "Start now with: sudo systemctl start fsdash"
echo "Or reboot with: sudo reboot"
