#!/bin/sh
set -eu

cd /home/stinger/dash
mkdir -p deps

if [ ! -d deps/lvgl/.git ]; then
    git clone --depth 1 --branch v8.3.11 https://github.com/lvgl/lvgl.git deps/lvgl
fi

if [ ! -d deps/lv_drivers/.git ]; then
    git clone --depth 1 https://github.com/lvgl/lv_drivers.git deps/lv_drivers
fi

echo "LVGL deps ready:"
echo "  deps/lvgl"
echo "  deps/lv_drivers"
