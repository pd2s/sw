#!/bin/sh

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

CC="${CC:-cc}"
CFLAGS="${CFLAGS:-}"
DEPS_FLAGS=""

case "$CFLAGS" in
  *-D*WITH_TRAY=0*) ;;
  *)
    DEPS_FLAGS="`pkg-config --cflags --libs basu` -lpthread"
    ;;
esac

$CC $CFLAGS $DEPS_FLAGS `${ROOT_PATH}/../../build.sh header` ${ROOT_PATH}/sw_swaybar.c -o ${ROOT_PATH}/sw_swaybar
