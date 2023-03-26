relay_ioport = meson.get_cross_property('relay_ioport')

add_project_arguments(
    '-DRELAY_IOPORT_NAME=' + relay_ioport,
    language: 'c'
)

libdriver = static_library('libsimple_driver', [
    'simple.c',
  ],
  include_directories: include_dir,
)
