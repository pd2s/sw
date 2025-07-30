#!/bin/sh

set -eu

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_PATH="${BUILD_PATH:-${ROOT_PATH}/build}"

DEPS="pixman-1"

CFLAGS="${CFLAGS:-}"

case "$CFLAGS" in
  *-D*SW_WITH_WAYLAND=0*) ;;
  *) DEPS="${DEPS} wayland-client" ;;
esac

case "$CFLAGS" in
  *-D*SW_WITH_TEXT=0*) ;;
  *) DEPS="${DEPS} fcft" ;;
esac

case "$1" in
  clean)
    rm -rf ${BUILD_PATH}
    exit 0
    ;;
  install)
    HEADER_INSTALL_PATH="${HEADER_INSTALL_PATH:-/usr/include}"
    LIBRARY_INSTALL_PATH="${LIBRARY_INSTALL_PATH:-/usr/lib64}"
    PKGCONFIG_INSTALL_PATH="${PKGCONFIG_INSTALL_PATH:-/usr/lib64/pkgconfig}"
    cp -f ${ROOT_PATH}/sw.h ${HEADER_INSTALL_PATH}/sw.h
    cp -f ${ROOT_PATH}/su.h ${HEADER_INSTALL_PATH}/su.h
    [ -e "${BUILD_PATH}/libsw.so" ] && cp -f ${BUILD_PATH}/libsw.so ${LIBRARY_INSTALL_PATH}/libsw.so
    [ -e "${BUILD_PATH}/libsw.a" ] && cp -f ${BUILD_PATH}/libsw.a ${LIBRARY_INSTALL_PATH}/libsw.a
    echo "
Name: sw
Description: simple widgets
Version: 0.0.1
Requires.private: ${DEPS}
Libs: -L${LIBRARY_INSTALL_PATH} -lsw
Libs.private: -lm
Cflags: -I${HEADER_INSTALL_PATH} -DSW_EXPORT=extern
    " > ${PKGCONFIG_INSTALL_PATH}/sw.pc
    exit 0
    ;;
  uninstall)
    HEADER_INSTALL_PATH="${HEADER_INSTALL_PATH:-/usr/include}"
    LIBRARY_INSTALL_PATH="${LIBRARY_INSTALL_PATH:-/usr/lib64}"
    PKGCONFIG_INSTALL_PATH="${PKGCONFIG_INSTALL_PATH:-/usr/lib64/pkgconfig}"
    rm -f ${HEADER_INSTALL_PATH}/sw.h ${HEADER_INSTALL_PATH}/su.h \
      ${LIBRARY_INSTALL_PATH}/libsw.so ${LIBRARY_INSTALL_PATH}/libsw.a \
      ${PKGCONFIG_INSTALL_PATH}/sw.pc
    exit 0
    ;;
  shared)
    CFLAGS="${CFLAGS} -fPIC"
    ;;
  header|static) : ;;
  *) echo "Usage: $0 {header|shared|static|clean|install|uninstall}" >&2; exit 1 ;;
esac

CC="${CC:-cc}"
AR="${AR:-ar}"

PKGCONFIG_FLAGS="${PKGCONFIG_FLAGS:-}"
DEPS_FLAGS=$(pkg-config $PKGCONFIG_FLAGS --cflags --libs ${DEPS})

[ ! -d "${BUILD_PATH}" ] && mkdir "${BUILD_PATH}"
rm -f ${BUILD_PATH}/*.o

ln -sf ${ROOT_PATH}/sw.h ${BUILD_PATH}/sw.h
ln -sf ${ROOT_PATH}/su.h ${BUILD_PATH}/su.h
ln -sf ${ROOT_PATH}/stb_sprintf.h ${BUILD_PATH}/stb_sprintf.h
ln -sf ${ROOT_PATH}/stb_image.h ${BUILD_PATH}/stb_image.h

case "$CFLAGS" in
  *-D*SW_WITH_WAYLAND=0*) ;;
  *)
    WAYLAND_SCANNER=$(pkg-config $PKGCONFIG_FLAGS --variable=wayland_scanner wayland-scanner)
    WAYLAND_PROTOCOLS_DIR=$(pkg-config $PKGCONFIG_FLAGS --variable=pkgdatadir wayland-protocols)
    # TODO: parallel
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "${BUILD_PATH}/xdg-shell.c"
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/staging/cursor-shape/cursor-shape-v1.xml" "${BUILD_PATH}/cursor-shape-v1.c"
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/unstable/tablet/tablet-unstable-v2.xml" "${BUILD_PATH}/tablet-unstable-v2.c"
    $WAYLAND_SCANNER private-code "${ROOT_PATH}/wlr-layer-shell-unstable-v1.xml" "${BUILD_PATH}/wlr-layer-shell-unstable-v1.c"
    $WAYLAND_SCANNER client-header "${WAYLAND_PROTOCOLS_DIR}/staging/cursor-shape/cursor-shape-v1.xml" "${BUILD_PATH}/cursor-shape-v1.h"
    $WAYLAND_SCANNER client-header "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "${BUILD_PATH}/xdg-shell.h"
    $WAYLAND_SCANNER client-header "${ROOT_PATH}/wlr-layer-shell-unstable-v1.xml" "${BUILD_PATH}/wlr-layer-shell-unstable-v1.h"
    $CC $CFLAGS -std=c99 -Wno-missing-variable-declarations -c ${BUILD_PATH}/wlr-layer-shell-unstable-v1.c -o ${BUILD_PATH}/wlr-layer-shell-unstable-v1.o
    $CC $CFLAGS -std=c99 -Wno-missing-variable-declarations -c ${BUILD_PATH}/xdg-shell.c -o ${BUILD_PATH}/xdg-shell.o
    $CC $CFLAGS -std=c99 -Wno-missing-variable-declarations -c ${BUILD_PATH}/cursor-shape-v1.c -o ${BUILD_PATH}/cursor-shape-v1.o
    $CC $CFLAGS -std=c99 -Wno-missing-variable-declarations -c ${BUILD_PATH}/tablet-unstable-v2.c -o ${BUILD_PATH}/tablet-unstable-v2.o
    ;;
esac

case "$CFLAGS" in
  *-D*SW_WITH_SVG=0*) ;;
  *)
    # TODO: use pkg-config
    [ ! -d "${BUILD_PATH}/resvg" ] && git clone --quiet https://github.com/linebender/resvg "${BUILD_PATH}/resvg"
    cargo build --quiet --manifest-path="${BUILD_PATH}/resvg/Cargo.toml" --workspace --release
    cp -f "${BUILD_PATH}/resvg/crates/c-api/resvg.h" "${BUILD_PATH}/resvg.h"
    cd "${BUILD_PATH}"
    $AR x "${BUILD_PATH}/resvg/target/release/libresvg.a"
    ;;
esac

case "$1" in
  header)
    echo $DEPS_FLAGS -I${BUILD_PATH} ${BUILD_PATH}/*.o
    ;;
  shared)
    $CC $CFLAGS $DEPS_FLAGS -I${BUILD_PATH} -DSW_IMPLEMENTATION -DSW_EXPORT=extern -c -xc ${BUILD_PATH}/sw.h -o ${BUILD_PATH}/sw.o
    $CC $CFLAGS -shared $DEPS_FLAGS ${BUILD_PATH}/*.o -o ${BUILD_PATH}/libsw.so
    echo "-L${BUILD_PATH} -Wl,-rpath,${BUILD_PATH} -lsw -I${BUILD_PATH} -DSW_EXPORT=extern"
    ;;
  static)
    $CC $CFLAGS $DEPS_FLAGS -I${BUILD_PATH} -DSW_IMPLEMENTATION -DSW_EXPORT=extern -c -xc ${BUILD_PATH}/sw.h -o ${BUILD_PATH}/sw.o
    $AR rcs ${BUILD_PATH}/libsw.a ${BUILD_PATH}/*.o
    echo "${BUILD_PATH}/libsw.a $DEPS_FLAGS -I${BUILD_PATH} -DSW_EXPORT=extern"
    ;;
esac
