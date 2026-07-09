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

boot_args="vt.global_cursor_default=0 quiet loglevel=3 systemd.show_status=false logo.nologo"
for cmdline in /boot/firmware/cmdline.txt /boot/cmdline.txt; do
    if [ -f "$cmdline" ]; then
        changed=0
        tmp="$(mktemp)"
        tr -d "\n" < "$cmdline" > "$tmp"
        for arg in $boot_args; do
            found=0
            for existing_arg in $(cat "$tmp"); do
                if [ "$existing_arg" = "$arg" ]; then
                    found=1
                    break
                fi
            done
            if [ "$found" -eq 0 ]; then
                printf " %s" "$arg" >> "$tmp"
                changed=1
            fi
        done
        printf "\n" >> "$tmp"
        if [ "$changed" -eq 1 ]; then
            cp "$cmdline" "$cmdline.fsdash.bak"
            cat "$tmp" > "$cmdline"
            echo "Updated fsdash boot args in $cmdline"
        fi
        rm -f "$tmp"
        break
    fi
done

cp /home/stinger/dash/deploy/fsdash.service /etc/systemd/system/fsdash.service
systemctl daemon-reload

systemctl disable --now display-manager.service 2>/dev/null || true
systemctl disable --now getty@tty1.service 2>/dev/null || true
systemctl set-default multi-user.target
systemctl enable fsdash.service

echo "Installed fsdash boot service."
echo "Start now with: sudo systemctl start fsdash"
echo "Or reboot with: sudo reboot"
