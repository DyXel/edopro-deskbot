project('edopro-firebot', 'cpp', license : 'AGPL-3.0-or-later', default_options : 'cpp_std=c++17')

# ygopen_dep = dependency('ygopen')
firebot_dep = dependency('firebot', fallback : ['firebot', 'firebot_dep'])

edopro_firebot_src = files([
	'src/main.cpp'
])

edopro_firebot_exe = executable('firebot', edopro_firebot_src, dependencies : firebot_dep)
