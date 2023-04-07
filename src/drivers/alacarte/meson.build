foreach r : range(1, num_relays + 1)
  ioport = meson.get_cross_property('relay_@0@_ioport'.format(r))
  bit = meson.get_cross_property('relay_@0@_bit'.format(r))
  assert(ioport in ['A', 'B', 'C', 'D'], '"@0@" is not a valid I/O port'.format(ioport))
  assert(bit >= 0 and bit < 8, '@0@ is not valid bit'.format(bit))
  add_project_arguments(
      '-DRELAY_@0@_IOPORT_NAME=@1@'.format(r, ioport),
      '-DRELAY_@0@_BIT=@1@'.format(r, bit),
      language: 'c'
  )
endforeach

libdriver = static_library('libalacarte_driver', [
    'alacarte.c',
  ],
  include_directories: include_dir,
)

