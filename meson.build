project(
    'sw', 'c',
    version: '0.0.1-dev',
    license: 'MIT',
    #meson_version: '',
    default_options: [
        'c_std=c18',
        'warning_level=everything',
        'werror=true',
        'b_ndebug=if-release',
    ],
)

add_project_arguments(
	[
        #'-D_POSIX_C_SOURCE=200809L',
        #'-D_XOPEN_SOURCE=700',
		'-D_DEFAULT_SOURCE',

		'-Wno-pre-c11-compat',
        '-Wno-c++-compat',
		'-Wno-unused-function',
        '-Wno-unsafe-buffer-usage',
        '-Wno-covered-switch-default',
        '-Wno-cast-qual',
		'-Wno-discarded-qualifiers',
		'-Wno-strict-overflow', # it seems that this cant be disabled via pragma (for stb_image.h)
		
		'-Wno-declaration-after-statement',

        '-Wno-disabled-macro-expansion',
        '-Wno-missing-variable-declarations',
		'-Wno-inline',
		'-Wno-date-time',

		'-Wno-unused-command-line-argument',
		'-Wno-unknown-warning-option',
	],
	language: 'c',
)

cc = meson.get_compiler('c')

wayland_client_dep = dependency('wayland-client')
pixman_dep = dependency('pixman-1')
fcft_dep = dependency('fcft', required: get_option('text_blocks'))
svg_dep = dependency('resvg', fallback: 'resvg', required: get_option('svg_image_blocks'))
wayland_scanner_dep = dependency('wayland-scanner', native: true)
wayland_protocols_dep = dependency('wayland-protocols', native: true)
math_dep = cc.find_library('m')

tinyexpr_dep = dependency('tinyexpr')

sw_deps = [
    wayland_client_dep,
    pixman_dep,
    fcft_dep,
    svg_dep,
	tinyexpr_dep,
]

wayland_scanner = find_program(
    wayland_scanner_dep.get_variable('wayland_scanner'),
    native: true,
	required: true,
)
wayland_scanner_header = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.h',
	arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)
wayland_scanner_code = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.c',
	arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_protocols_dir = wayland_protocols_dep.get_variable('pkgdatadir')

sw_src = [
    'sw.c',

    wayland_scanner_code.process(wayland_protocols_dir / 'stable/xdg-shell/xdg-shell.xml'),
    wayland_scanner_code.process(wayland_protocols_dir / 'staging/cursor-shape/cursor-shape-v1.xml'),
    wayland_scanner_code.process(wayland_protocols_dir / 'unstable/tablet/tablet-unstable-v2.xml'), # required by cursor-shape-v1
    wayland_scanner_code.process('wlr-layer-shell-unstable-v1.xml'),

    wayland_scanner_header.process(wayland_protocols_dir / 'staging/cursor-shape/cursor-shape-v1.xml'),
    wayland_scanner_header.process(wayland_protocols_dir / 'stable/xdg-shell/xdg-shell.xml'),
    wayland_scanner_header.process('wlr-layer-shell-unstable-v1.xml'),
]

version = '"@0@"'.format(meson.project_version())
git = find_program('git', native: true, required: false)
if git.found()
	git_commit = run_command([git, '--git-dir=.git', 'rev-parse', '--short', 'HEAD'], check: false)
	git_branch = run_command([git, '--git-dir=.git', 'rev-parse', '--abbrev-ref', 'HEAD'], check: false)
	if git_commit.returncode() == 0 and git_branch.returncode() == 0
		version = '"@0@-@1@ (" __DATE__ ", branch \'@2@\')"'.format(
			meson.project_version(),
			git_commit.stdout().strip(),
			git_branch.stdout().strip(),
		)
	endif
endif

add_project_arguments(
	[
		'-DWITH_TEXT=@0@'.format(fcft_dep.found() ? 1 : 0),
		'-DWITH_PNG=@0@'.format(get_option('png_image_blocks').enabled() ? 1 : 0),
		'-DWITH_SVG=@0@'.format(svg_dep.found() ? 1 : 0),
		'-DWITH_JPG=@0@'.format(get_option('jpg_image_blocks').enabled() ? 1 : 0),
		'-DWITH_TGA=@0@'.format(get_option('tga_image_blocks').enabled() ? 1 : 0),
		'-DWITH_BMP=@0@'.format(get_option('bmp_image_blocks').enabled() ? 1 : 0),
		'-DWITH_PSD=@0@'.format(get_option('psd_image_blocks').enabled() ? 1 : 0),
		'-DWITH_GIF=@0@'.format(get_option('gif_image_blocks').enabled() ? 1 : 0),
		'-DWITH_HDR=@0@'.format(get_option('hdr_image_blocks').enabled() ? 1 : 0),
		'-DWITH_PIC=@0@'.format(get_option('pic_image_blocks').enabled() ? 1 : 0),
		'-DWITH_PNM=@0@'.format(get_option('pnm_image_blocks').enabled() ? 1 : 0),

		'-DDEBUG=@0@'.format(get_option('buildtype').startswith('debug') ? 1 : 0),
		'-DFUZZ_TEST=@0@'.format(get_option('fuzz_test')),

		'-DVERSION=@0@'.format(version),
	],
	language: 'c',
)

if get_option('fuzz_test')
	add_project_arguments('-fsanitize=fuzzer', language: 'c')
	add_project_link_arguments('-fsanitize=fuzzer', language: 'c')
endif

inc = include_directories('include')

executable(
    'sw',
    sw_src,
	c_args: ['-DPREFIX="sw"',],
	include_directories: inc,
    dependencies: sw_deps,
    install: true,
)

if not get_option('fuzz_test')
	subdir('examples')
endif

summary({
	'text blocks' : fcft_dep.found(),

	'png image blocks' : get_option('png_image_blocks').enabled(),
    'svg image blocks' : svg_dep.found(),
	'jpg image blocks' : get_option('jpg_image_blocks').enabled(),
	'tga image blocks' : get_option('tga_image_blocks').enabled(),
	'bmp image blocks' : get_option('bmp_image_blocks').enabled(),
	'psd image blocks' : get_option('psd_image_blocks').enabled(),
	'gif image blocks' : get_option('gif_image_blocks').enabled(),
	'hdr image blocks' : get_option('hdr_image_blocks').enabled(),
	'pic image blocks' : get_option('pic_image_blocks').enabled(),
	'pnm image blocks' : get_option('pnm_image_blocks').enabled(),

	'swaybar example' : not get_option('fuzz_test') and get_option('swaybar_example'),
	'swaybar example: system tray' : not get_option('fuzz_test') and get_option('swaybar_example_systray').enabled(),
}, bool_yn: true)
