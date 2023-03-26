relay_ioport = meson.get_cross_property('relay_ioport')
relay_offset = meson.get_cross_property('relay_offset', 0)

assert(relay_ioport in ['A', 'B', 'C', 'D'], '"@0@" is not a valid I/O port'.format(relay_ioport))
assert(relay_offset >= 0 and num_relays + relay_offset <= 8, '@0@ is not valid relay offset'.format(relay_offset))

add_project_arguments(
    '-DRELAY_IOPORT_NAME=' + relay_ioport,
    '-DRELAY_OFFSET=' + relay_offset.to_string(),
    language: 'c'
)

libdriver = static_library('libsimple_driver', [
    'simple.c',
  ],
  include_directories: include_dir,
)
