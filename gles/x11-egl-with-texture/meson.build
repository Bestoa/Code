project('x11-egl-with-texture', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('x11'), dependency('egl'), dependency('glesv2')]

executable('x11-egl-with-texture',
           'main.cpp',
           dependencies : dep,
           install : true)
