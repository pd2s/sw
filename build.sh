#!/bin/sh

set -eu

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_PATH="${BUILD_PATH:-${ROOT_PATH}/build}"

DEPS="pixman-1"

CFLAGS="${CFLAGS:-}"

case "$CFLAGS" in
  *-D*SW_WITH_WAYLAND_BACKEND=0*) ;;
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
    cp -f ${ROOT_PATH}/swidgets.h ${HEADER_INSTALL_PATH}/swidgets.h
    cp -f ${ROOT_PATH}/sutil.h ${HEADER_INSTALL_PATH}/sutil.h
    [ -e "${BUILD_PATH}/libsw.so" ] && cp -f ${BUILD_PATH}/libsw.so ${LIBRARY_INSTALL_PATH}/libsw.so
    [ -e "${BUILD_PATH}/libsw.a" ] && cp -f ${BUILD_PATH}/libsw.a ${LIBRARY_INSTALL_PATH}/libsw.a
    echo "
Name: sw
Description: simple/suckless widgets
Version: 0.0.1
Requires.private: ${DEPS}
Libs: -L${LIBRARY_INSTALL_PATH} -lsw
Libs.private: -lm
Cflags: -I${HEADER_INSTALL_PATH} -DSW_FUNC_DEF=extern
    " > ${PKGCONFIG_INSTALL_PATH}/sw.pc
    exit 0
    ;;
  uninstall)
    HEADER_INSTALL_PATH="${HEADER_INSTALL_PATH:-/usr/include}"
    LIBRARY_INSTALL_PATH="${LIBRARY_INSTALL_PATH:-/usr/lib64}"
    PKGCONFIG_INSTALL_PATH="${PKGCONFIG_INSTALL_PATH:-/usr/lib64/pkgconfig}"
    rm -f ${HEADER_INSTALL_PATH}/swidgets.h ${HEADER_INSTALL_PATH}/sutil.h \
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
DEPS_FLAGS="-lm $(pkg-config $PKGCONFIG_FLAGS --cflags --libs ${DEPS})"

RESVG_DEP_PATH="${RESVG_DEP_PATH:-${BUILD_PATH}/resvg}"
RESVG_DEP_FLAGS="${RESVG_DEP_FLAGS:-}"

[ ! -d "${BUILD_PATH}/include" ] && mkdir -p "${BUILD_PATH}/include"
rm -f ${BUILD_PATH}/*.o

ln -sf ${ROOT_PATH}/swidgets.h ${BUILD_PATH}/include/swidgets.h
ln -sf ${ROOT_PATH}/sutil.h ${BUILD_PATH}/include/sutil.h
ln -sf ${ROOT_PATH}/stb_sprintf.h ${BUILD_PATH}/include/stb_sprintf.h
ln -sf ${ROOT_PATH}/stb_image.h ${BUILD_PATH}/include/stb_image.h

case "$CFLAGS" in
  *-D*SW_WITH_WAYLAND_BACKEND=0*) ;;
  *)
    WAYLAND_SCANNER=$(pkg-config $PKGCONFIG_FLAGS --variable=wayland_scanner wayland-scanner)
    WAYLAND_PROTOCOLS_DIR=$(pkg-config $PKGCONFIG_FLAGS --variable=pkgdatadir wayland-protocols)
    # TODO: parallel
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "${BUILD_PATH}/include/xdg-shell.c"
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/staging/cursor-shape/cursor-shape-v1.xml" "${BUILD_PATH}/include/cursor-shape-v1.c"
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/unstable/tablet/tablet-unstable-v2.xml" "${BUILD_PATH}/include/tablet-unstable-v2.c"
    $WAYLAND_SCANNER private-code "${ROOT_PATH}/wlr-layer-shell-unstable-v1.xml" "${BUILD_PATH}/include/wlr-layer-shell-unstable-v1.c"
    $WAYLAND_SCANNER private-code "${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml" "${BUILD_PATH}/include/xdg-decoration-unstable-v1.c"
    $WAYLAND_SCANNER client-header "${WAYLAND_PROTOCOLS_DIR}/staging/cursor-shape/cursor-shape-v1.xml" "${BUILD_PATH}/include/cursor-shape-v1.h"
    $WAYLAND_SCANNER client-header "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "${BUILD_PATH}/include/xdg-shell.h"
    $WAYLAND_SCANNER client-header "${ROOT_PATH}/wlr-layer-shell-unstable-v1.xml" "${BUILD_PATH}/include/wlr-layer-shell-unstable-v1.h"
    $WAYLAND_SCANNER client-header "${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml" "${BUILD_PATH}/include/xdg-decoration-unstable-v1.h"
    ;;
esac

case "$CFLAGS" in
  *-D*SW_WITH_SVG=0*) ;;
  *)
    [ ! -d "${RESVG_DEP_PATH}" ] && git clone https://github.com/linebender/resvg "${RESVG_DEP_PATH}"
    cargo build --manifest-path="${RESVG_DEP_PATH}/crates/c-api/Cargo.toml" $RESVG_DEP_FLAGS > ${BUILD_PATH}/resvg.log 2>&1 \
    || {
      cat ${BUILD_PATH}/resvg.log >&2
      exit 1
    }
    cp -f "${RESVG_DEP_PATH}/crates/c-api/resvg.h" "${BUILD_PATH}/include/resvg.h"
    cd "${BUILD_PATH}"
    # TODO: more robust check
    case "$RESVG_DEP_FLAGS" in
      *--release*) $AR x "${RESVG_DEP_PATH}/target/release/libresvg.a" ;;
      *) $AR x "${RESVG_DEP_PATH}/target/debug/libresvg.a" ;;
    esac
esac

OBJS=$(find "$BUILD_PATH" -maxdepth 1 -type f -name '*.o' -print)

case "$1" in
  header)
    echo $DEPS_FLAGS -isystem${BUILD_PATH}/include $OBJS
    ;;
  shared)
    $CC $CFLAGS $DEPS_FLAGS -D_XOPEN_SOURCE=700 -isystem${BUILD_PATH}/include -DSW_IMPLEMENTATION -DSW_FUNC_DEF=extern -c -xc ${BUILD_PATH}/include/swidgets.h -o ${BUILD_PATH}/sw.o
    $CC $CFLAGS -shared $DEPS_FLAGS $OBJS -o ${BUILD_PATH}/libsw.so
    echo "-L${BUILD_PATH} -Wl,-rpath,${BUILD_PATH} -lsw -isystem${BUILD_PATH}/include -DSW_FUNC_DEF=extern"
    ;;
  static)
    $CC $CFLAGS $DEPS_FLAGS -D_XOPEN_SOURCE=700 -isystem${BUILD_PATH}/include -DSW_IMPLEMENTATION -DSW_FUNC_DEF=extern -c -xc ${BUILD_PATH}/include/swidgets.h -o ${BUILD_PATH}/sw.o
    $AR rcs ${BUILD_PATH}/libsw.a $OBJS
    echo "${BUILD_PATH}/libsw.a $DEPS_FLAGS -isystem${BUILD_PATH}/include -DSW_FUNC_DEF=extern"
    ;;
esac
