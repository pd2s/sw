# sw - simple widgets

This project is currently in early alpha state.

# Plans for near future
* More examples: json, immediate mode, generic window
* Scroll blocks, children wrap for composite,text blocks
* generic window, user-provided buffer backends
* user-provided renderer
* replace fcft with freetype+harfbuzz
* replace pixman
* rebrand this project to "simple utils", make sw a part of it

# Interfaces
* header-only
* shared
* static

# Backends
* user-provided buffer (NOT IMPLEMENTED YET)
* generic window (cross-platform) (NOT IMPLEMENTED YET)
* all windowing systems (ONLY WAYLAND IS PARTIALLY IMPLEMENTED)

# Renderers
* software (NOT FULLY IMPLEMENTED)
* user-provided (NOT IMPLEMENTED YET)

# External dependencies
* libc
* libm
* [wayland-client] (only if wayland backend is enabled)
* [pixman]
* [fcft] (only if text blocks feature is enabled)

# Compile-time dependencies
* [wayland-protocols] (only if wayland backend is enabled)

# Usage
```shell
build.sh header|static|shared   # build specified interface and print compiler flags similar to pkg-config --cflags --libs
build.sh install                # install .pc file and all interfaces built by build.sh header|static|shared
build.sh uninstall              # opposite to build.sh install
build.sh clean                  # clean build directory
```

These environment variables (with their default values) affect build.sh:
```shell
CC                       # cc
CFLAGS
AR                       # ar
BUILD_PATH               # ./build
HEADER_INSTALL_PATH      # /usr/include
LIBRARY_INSTALL_PATH     # /usr/lib64
PKGCONFIG_INSTALL_PATH   # /usr/lib64/pkgconfig
PKGCONFIG_FLAGS
RESVG_PATH               # ${BUILD_PATH}/resvg   path to resvg dependency(required if SW_WITH_SVG=1), if left at default, git clone is used
```

To enable/disable specific feature, add `-D FEATURE=1|0` to `CFLAGS`.
Full list of features with their default values is located at the top of sw.h

[wayland-client]: https://gitlab.freedesktop.org/wayland/wayland
[pixman]: https://gitlab.freedesktop.org/pixman/pixman
[fcft]: https://codeberg.org/dnkl/fcft
[wayland-protocols]: https://gitlab.freedesktop.org/wayland/wayland-protocols
