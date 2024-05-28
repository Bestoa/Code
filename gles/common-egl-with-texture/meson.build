project('egl-with-texture', ['c', 'cpp'],
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

dep = [ dependency('gbm'), dependency('libdrm'), dependency('egl'), dependency('glesv2')]
src = [ 'main.cpp' ]

if get_option('platform') == 'x11'
  dep += dependency('x11')
  src += 'x11.cpp'
elif get_option('platform') == 'wayland'

  wayland_protocols_dep = dependency('wayland-protocols', version : '>= 1.12')
  wayland_scanner_dep = dependency('wayland-scanner', native: true)

  wayland_scanner = find_program(wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner'))
  wayland_protocols_dir = wayland_protocols_dep.get_pkgconfig_variable('pkgdatadir')
  xdg_shell_xml_path = wayland_protocols_dir + '/stable/xdg-shell/xdg-shell.xml'
      xdg_shell_client_header = custom_target(
        'xdg-shell client-header',
        command: [ wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@' ],
        input: xdg_shell_xml_path,
        output: 'xdg-shell-client-protocol.h',
        )
      xdg_shell_private_code = custom_target(
        'xdg-shell private-code',
        command: [ wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@' ],
        input: xdg_shell_xml_path,
        output: 'xdg-shell-protocol.c',
        )

  dep += dependency('wayland-egl')
  dep += dependency('wayland-client')

  src += [ 'wayland.cpp', xdg_shell_client_header, xdg_shell_private_code ]
endif


executable('egl-with-texture', src,
           dependencies : dep,
           install : true)
