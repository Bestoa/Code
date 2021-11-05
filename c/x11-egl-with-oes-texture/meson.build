project('x11-egl-oes-texture', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('x11'), dependency('egl'), dependency('glesv2'), dependency('libdrm') ]

executable('x11-egl-oes-texture',
           'main.cpp',
           'buffers.cpp',
           dependencies : dep,
           install : true)
