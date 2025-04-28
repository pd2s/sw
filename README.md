# sw - simple widgets

sw is an ongoing project that aims to provide an easy to use and integrate immediate mode widget library that has minimal dependencies and uses native APIs of each supported platform.

This project is currently in early alpha state. The documentation is basically non-existent.

# Interfaces (this list may grow in the future)
* multiprocess json(see examples/python for more details)
* header-only c library (NOT IMPLEMENTED YET)
* c library that works with JSON (NOT IMPLEMENTED YET)

# Backends (this list may grow in the future)
* wayland (NOT FULLY IMPLEMENTED)
* wasm (NOT IMPLEMENTED YET)
* win32 (NOT IMPLEMENTED YET)
* cocoa (NOT IMPLEMENTED YET)
* user-provided buffer(platform-agnostic) (NOT IMPLEMENTED YET)
  
# External dependencies (this list will shrink in the future)
* libc
* libm
* [wayland-client] (only for wayland backend)
* [pixman]
* [fcft]

# Compile-time dependencies (this list will shrink in the future)
* [meson]
* [wayland-protocols] (only for wayland backend)

# How to use
Follow these instructions to compile: https://mesonbuild.com/Quick-guide.html#compiling-a-meson-project
For now, only multiprocess interface(see examples/python) with wayland backend is implemented.

[wayland-client]: https://gitlab.freedesktop.org/wayland/wayland
[pixman]: https://gitlab.freedesktop.org/pixman/pixman
[fcft]: https://codeberg.org/dnkl/fcft
[meson]: https://github.com/mesonbuild/meson
[wayland-protocols]: https://gitlab.freedesktop.org/wayland/wayland-protocols