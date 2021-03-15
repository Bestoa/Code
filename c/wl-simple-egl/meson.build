project('wayland-client-egl', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('wayland-egl'), dependency('wayland-client'), dependency('egl'), dependency('glesv2') ]

executable('wayland-client-egl',
           'OpenGLESHelloAPI_LinuxWayland.cpp',
           dependencies : dep,
           install : true)
