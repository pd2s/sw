project('resvg', 'c', version: '0.45.1')

build_prog = find_program('build.sh', native: true, required: true)

resvg_target = custom_target(
	command: [
		build_prog, '@CURRENT_SOURCE_DIR@', '@PRIVATE_DIR@', '@OUTDIR@', get_option('buildtype').startswith('debug') ? 'debug' : 'release',
	],
	output: ['libresvg.a', 'resvg.h'],
)

meson.override_dependency('resvg',
	declare_dependency(
		sources: resvg_target,
	),
)
