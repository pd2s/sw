threads_dep = dependency('threads', required: get_option('swaybar_example_systray'))
basu_dep = dependency(
	'basu',
	fallback: 'basu',
	default_options: [
		'c_args=-w',
		'werror=false',
		'warning_level=0',
		'default_library=static',
		'audit=disabled',
		'libcap=disabled',
	],
	include_type: 'system',
	required: get_option('swaybar_example_systray'),
	static: true,
)

executable(
	'sw-swaybar',
	'swaybar.c',
	include_directories: [inc, inc_examples_common],
	c_args: [
		'-DPREFIX="sw-swaybar"',
		'-DWITH_TRAY=@0@'.format(basu_dep.found() ? 1 : 0),
	],
	dependencies: [basu_dep, threads_dep],
	install: true,
)
