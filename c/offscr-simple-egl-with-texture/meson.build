project('offscr-simple-egl', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('egl'), dependency('glesv2') ]

executable('offscr-simple-egl',
           'OpenGLESHelloAPI_LinuxOffscr.cpp',
           dependencies: dep,
           install : true)
