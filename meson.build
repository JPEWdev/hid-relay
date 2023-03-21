project('HIDRelay', 'c',
  default_options: [
    'buildtype=minsize',
    'b_lto=false',
    'cpp_rtti=false',
    'cpp_eh_=none',
    'werror=true',
    'c_std=c99',
  ]
)

assert(meson.is_cross_build(), 'Must be a cross build')

cpu_speed = meson.get_cross_property('cpu_speed')
usb_ioport = meson.get_cross_property('usb_ioport')
usb_dminus_bit = meson.get_cross_property('usb_dminus_bit')
usb_dplus_bit = meson.get_cross_property('usb_dplus_bit')
num_relays = meson.get_cross_property('num_relays')

assert(num_relays.to_int() >= 1 and num_relays.to_int() <= 8, 'num_relays must be in the range [1..8]')

add_project_arguments(
	'--param=min-pagesize=0',
    '-fpack-struct',
    '-fshort-enums',
	'-funsigned-bitfields',
    '-funsigned-char',
    '-mmcu=' + host_machine.cpu(),
    '-DF_CPU=' + cpu_speed + 'UL',
    '-DUSB_CFG_IOPORTNAME=' + usb_ioport,
    '-DUSB_CFG_DMINUS_BIT=' + usb_dminus_bit,
    '-DUSB_CFG_DPLUS_BIT=' + usb_dplus_bit,
    '-DNUM_RELAYS=' + num_relays,
    language: 'c',
)

add_project_link_arguments(
    '-mmcu=' + host_machine.cpu(),
    language: 'c',
)


program = executable('hidrelay', [
    'src/main.c',
    'usbdrv/usbdrv.c',
    'usbdrv/usbdrvasm.S',
    'usbdrv/oddebug.c',
  ],
  include_directories: include_directories('src', 'usbdrv')
)

objcopy = find_program('objcopy')

program_hex = custom_target('program-hex',
  input: program,
  output: program.name() + '.hex',
  command: [
    objcopy,
    '-j', '.text',
    '-j', '.data',
    '-O', 'ihex',
    '@INPUT@',
    '@OUTPUT@'
  ],
)

eeprom_hex = custom_target('eeprom-hex',
  input: program,
  output: program.name() + '.ee.hex',
  command: [
    objcopy,
    '-j', '.eeprom',
    '--change-section-lma', '.eeprom=0',
    '-O', 'ihex',
    '@INPUT@',
    '@OUTPUT@'
  ],
)

avrdude = find_program('avrdude')
avrdude_speed = meson.get_cross_property('avrdude_speed')
avrdude_programmer = get_option('avrdude_programmer')
avrdude_port = get_option('avrdude_port')

run_target('writeflash',
  depends: [program_hex, eeprom_hex],
  command: [
    avrdude,
    '-c', avrdude_programmer,
    '-P', avrdude_port,
    '-p', host_machine.cpu(),
    '-B', avrdude_speed,
    '-e',
    '-U', 'flash:w:' + program_hex.full_path(),
    '-U', 'eeprom:w:' + eeprom_hex.full_path(),
  ]
)

lfuse = meson.get_cross_property('lfuse')
hfuse = meson.get_cross_property('hfuse')
efuse = meson.get_cross_property('efuse')


# NOTE: We write the fuses at a slower speed because on
# new devices, the external clock is not configured yet so
# the high speed transfer fails
run_target('writefuses',
  command: [
    avrdude,
    '-c', avrdude_programmer,
    '-P', avrdude_port,
    '-p', host_machine.cpu(),
    '-B', '100',
	'-U', 'lfuse:w:' + lfuse + ':m',
	'-U', 'hfuse:w:' + hfuse + ':m',
	'-U', 'efuse:w:' + efuse + ':m',
  ]
)

run_target('readfuses',
  command: [
    avrdude,
    '-c', avrdude_programmer,
    '-P', avrdude_port,
    '-p', host_machine.cpu(),
	'-B', avrdude_speed,
	'-U', 'lfuse:r:-:h',
	'-U', 'hfuse:r:-:h',
	'-U', 'efuse:r:-:h',
  ]
)

run_target('readeeprom',
  command: [
    avrdude,
    '-c', avrdude_programmer,
    '-P', avrdude_port,
    '-p', host_machine.cpu(),
    '-B', avrdude_speed,
	'-U', 'eeprom:r:eeprom.hex:i'
  ]
)
