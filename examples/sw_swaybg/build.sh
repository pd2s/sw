#!/bin/sh

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

CC="${CC:-cc}"
CFLAGS="${CFLAGS:-}"

CFLAGS="${CFLAGS} -DSW_WITH_WAYLAND_BACKEND=1 -DSW_WITH_WAYLAND_KEYBOARD=0 -DSW_WITH_TEXT=0 -DSW_WITH_SVG=1"

$CC $CFLAGS $(${ROOT_PATH}/../../build.sh header) ${ROOT_PATH}/sw_swaybg.c -o ${ROOT_PATH}/sw_swaybg
