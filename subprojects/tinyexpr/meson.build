project('tinyexpr', 'c')

meson.override_dependency(
	'tinyexpr',
	declare_dependency(
		link_with: static_library(
			'tinyexpr',
			'tinyexpr.c',
			c_args: '-w',
			dependencies: meson.get_compiler('c').find_library('m'),
		),
		include_directories: include_directories('.'),
	),
)
