project('edopro-firebot', 'cpp', license : 'AGPL-3.0-or-later', default_options : 'cpp_std=c++17')

# ygopen_dep  = dependency('ygopen')
boost_dep   = dependency('boost', version : '>=1.75')
firebot_dep = dependency('firebot')

edopro_firebot_src = files([
	'src/client.cpp',
	'src/main.cpp'
])

edopro_firebot_exe = executable('edopro-firebot', edopro_firebot_src, dependencies : [boost_dep, firebot_dep])
