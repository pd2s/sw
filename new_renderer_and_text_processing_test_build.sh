#!/bin/sh

set -eu

ROOT_PATH=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_PATH="${BUILD_PATH:-${ROOT_PATH}/build}"

CC="${CC:-cc}"
AR="${AR:-ar}"

CFLAGS="${CFLAGS:-}"
CFLAGS="${CFLAGS} -DSW_WITH_WAYLAND_BACKEND=1 -DSW_WITH_SVG=1 -DSW_WITH_MEMORY_BACKEND=0 -DSW_WITH_TEXT=0 -DSW_WITH_PNG=1 -DSW_WITH_JPG=0 -DSW_WITH_TGA=0 -DSW_WITH_BMP=0 -DSW_WITH_PSD=0 -DSW_WITH_GIF=0 -DSW_WITH_HDR=0 -DSW_WITH_PIC=0 -DSW_WITH_PNM=0"

SW_FLAGS=$(${ROOT_PATH}/build.sh header)

# TODO: parallel

FREETYPE_DEP_PATH="${FREETYPE_DEP_PATH:-${BUILD_PATH}/freetype}"
FREETYPE_DEP_FLAGS="${FREETYPE_DEP_FLAGS:-}"

fail() {
  cat $1 >&2
  exit 1
}

[ ! -d "${FREETYPE_DEP_PATH}" ] && git clone https://github.com/freetype/freetype "${FREETYPE_DEP_PATH}"
cd "${FREETYPE_DEP_PATH}"
# TODO: more robust check
case "$FREETYPE_DEP_FLAGS" in
  *release*) FREETYPE_DEP_BUILD_DIR="release" ;;
  *) FREETYPE_DEP_BUILD_DIR="debug" ;;
esac
# hack to replace libpng with stb_image
sed -i '/#include "pngshim.c"/d' ${FREETYPE_DEP_PATH}/src/sfnt/sfnt.c
meson setup -Dharfbuzz=enabled -Derror_strings=true -Dpng=disabled -Dc_args="-DFT_CONFIG_OPTION_USE_PNG -w" -Dbrotli=disabled -Dbzip2=disabled -Dmmap=disabled -Dtests=disabled -Dzlib=disabled -Ddefault_library=static -Dwarning_level=0 -Db_ndebug=if-release -Dwerror=false -Dprefix=${BUILD_PATH} $FREETYPE_DEP_FLAGS $FREETYPE_DEP_BUILD_DIR > ${BUILD_PATH}/freetype.log 2>&1 || fail ${BUILD_PATH}/freetype.log
meson compile -C $FREETYPE_DEP_BUILD_DIR >> ${BUILD_PATH}/freetype.log 2>&1 || fail ${BUILD_PATH}/freetype.log
meson install -C $FREETYPE_DEP_BUILD_DIR >> ${BUILD_PATH}/freetype.log 2>&1 || fail ${BUILD_PATH}/freetype.log

HARFBUZZ_DEP_PATH="${HARFBUZZ_DEP_PATH:-${BUILD_PATH}/harfbuzz}"
HARFBUZZ_DEP_FLAGS="${HARFBUZZ_DEP_FLAGS:-}"
[ ! -d "${HARFBUZZ_DEP_PATH}" ] && git clone https://github.com/harfbuzz/harfbuzz "${HARFBUZZ_DEP_PATH}"
cd "${HARFBUZZ_DEP_PATH}"
# TODO: more robust check
case "$HARFBUZZ_DEP_FLAGS" in
  *release*) HARFBUZZ_DEP_BUILD_DIR="release" ;;
  *) HARFBUZZ_DEP_BUILD_DIR="debug" ;;
esac
# TODO: force meson to use bundled ft, tried force-fallback, PKG_CONFIG_PATH, CMAKE_PREFIX_PATH overrides
meson setup -Dfreetype=enabled -Dglib=disabled -Dgobject=disabled -Dcairo=disabled -Dchafa=disabled -Dicu=disabled -Dgraphite=disabled -Dgraphite2=disabled -Dfontations=disabled -Dgdi=disabled -Ddirectwrite=disabled -Dcoretext=disabled -Dharfrust=disabled -Dkbts=disabled -Dwasm=disabled -Dtests=disabled -Dintrospection=disabled -Ddocs=disabled -Ddoc_tests=false -Dutilities=disabled -Dbenchmark=disabled -Dicu_builtin=true -Dwith_libstdcxx=false -Dexperimental_api=false -Dragel_subproject=false -Ddefault_library=static -Dwarning_level=0 -Db_ndebug=if-release -Dwerror=false -Dc_args="-DHB_CUSTOM_MALLOC -w" -Dcpp_args="-DHB_CUSTOM_MALLOC -w" -Dprefix=${BUILD_PATH} $HARFBUZZ_DEP_FLAGS $HARFBUZZ_DEP_BUILD_DIR > ${BUILD_PATH}/harfbuzz.log 2>&1 || fail ${BUILD_PATH}/harfbuzz.log
meson compile -C $HARFBUZZ_DEP_BUILD_DIR >> ${BUILD_PATH}/harfbuzz.log 2>&1 || fail ${BUILD_PATH}/harfbuzz.log
meson install -C $HARFBUZZ_DEP_BUILD_DIR >> ${BUILD_PATH}/harfbuzz.log 2>&1 || fail ${BUILD_PATH}/harfbuzz.log

#cd "${BUILD_PATH}"
#$AR x "${FREETYPE_DEP_PATH}/build/libfreetype.a"
#$AR x "${HARFBUZZ_DEP_PATH}/build/src/libharfbuzz.a"

# TODO: remove: $(pkg-config --cflags --libs librsvg-2.0) -w
$CC $CFLAGS $SW_FLAGS -isystem${BUILD_PATH}/include/freetype2 "${FREETYPE_DEP_PATH}/${FREETYPE_DEP_BUILD_DIR}/libfreetype.a" "${HARFBUZZ_DEP_PATH}/${HARFBUZZ_DEP_BUILD_DIR}/src/libharfbuzz.a" ${ROOT_PATH}/new_renderer_and_text_processing_test.c -o ${ROOT_PATH}/new_renderer_and_text_processing_test
