project('edopro-deskbot', 'cpp', license : 'AGPL-3.0-or-later', default_options : 'cpp_std=c++17')

# ygopen_dep  = dependency('ygopen')
boost_dep   = dependency('boost', version : '>=1.75')
deskbot_dep = dependency('deskbot')
thread_dep  = dependency('threads')

edopro_deskbot_src = files([
	'src/client.cpp',
	'src/encode_context.cpp',
	'src/load_script.cpp',
	'src/main.cpp'
])

edopro_deskbot_exe = executable('edopro-deskbot', edopro_deskbot_src, dependencies : [boost_dep, deskbot_dep, thread_dep])
