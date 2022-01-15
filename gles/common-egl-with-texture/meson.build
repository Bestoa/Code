project('egl-with-texture', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('gbm'), dependency('libdrm'), dependency('egl'), dependency('glesv2')]
src = [ 'main.cpp' ]

if get_option('platform') == 'x11'
  dep += dependency('x11')
  src += 'x11.cpp'
elif get_option('platform') == 'wayland'
  dep += dependency('wayland-egl')
  dep += dependency('wayland-client')
  src += 'wayland.cpp'
endif


executable('egl-with-texture', src,
           dependencies : dep,
           install : true)
