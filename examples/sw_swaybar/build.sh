#!/bin/sh

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

CC="${CC:-cc}"
CFLAGS="${CFLAGS:-}"
DEPS_FLAGS=""

CFLAGS="${CFLAGS} -DSW_WITH_WAYLAND_BACKEND=1 -DSW_WITH_WAYLAND_KEYBOARD=0 -DSW_WITH_TEXT=1 -DSW_WITH_SVG=1"

case "$CFLAGS" in
  *-D*WITH_TRAY=0*) ;;
  *)
    if pkg-config --exists basu; then
      DEPS_FLAGS="$(pkg-config --cflags --libs basu) -lpthread"
    elif pkg-config --exists libelogind; then
      DEPS_FLAGS="$(pkg-config --cflags --libs libelogind) -lpthread"
    elif pkg-config --exists libsystemd; then
      DEPS_FLAGS="$(pkg-config --cflags --libs libsystemd) -lpthread"
    else
      echo "neither basu, libelogind, nor libsystemd found via pkg-config (add -DWITH_TRAY=0 to CFLAGS to disable system tray feature)" >&2
      exit 1
    fi
    ;;
esac

$CC $CFLAGS $DEPS_FLAGS $(${ROOT_PATH}/../../build.sh header) ${ROOT_PATH}/sw_swaybar.c -o ${ROOT_PATH}/sw_swaybar
