project('HIDRelay', 'c',
  default_options: [
    'buildtype=minsize',
    'b_lto=false',
    'b_staticpic=false',
    'cpp_rtti=false',
    'cpp_eh_=none',
    'werror=true',
    'warning_level=3',
    'c_std=c11',
  ]
)

assert(meson.is_cross_build(), 'Must be a cross build')

cpu_speed = meson.get_cross_property('cpu_speed')
usb_ioport = meson.get_cross_property('usb_ioport')
usb_dminus_bit = meson.get_cross_property('usb_dminus_bit')
usb_dplus_bit = meson.get_cross_property('usb_dplus_bit')
num_relays = meson.get_cross_property('num_relays')
enable_watchdog = meson.get_cross_property('enable_watchdog', true)
calibrate_oscillator = meson.get_cross_property('calibrate_oscillator', false)

assert(usb_ioport in ['A', 'B', 'C', 'D'], '"@0@" is not a valid I/O port'.format(usb_ioport))
assert(num_relays >= 1 and num_relays <= 8, 'num_relays must be in the range [1..8]')
assert(usb_dminus_bit >= 0 and usb_dminus_bit <= 7, '@0@ is not a valid port bit'.format(usb_dminus_bit))
assert(usb_dplus_bit >= 0 and usb_dplus_bit <= 7, '@0@ is not a valid port bit'.format(usb_dplus_bit))

add_project_arguments(
    '-fpack-struct',
    '-fshort-enums',
	'-funsigned-bitfields',
    '-funsigned-char',
    '-mmcu=' + host_machine.cpu(),
    '-DF_CPU=' + cpu_speed.to_string() + 'UL',
    '-DUSB_CFG_IOPORTNAME=' + usb_ioport,
    '-DUSB_CFG_DMINUS_BIT=' + usb_dminus_bit.to_string(),
    '-DUSB_CFG_DPLUS_BIT=' + usb_dplus_bit.to_string(),
    '-DNUM_RELAYS=' + num_relays.to_string(),
    '-DREPORT_SERIAL=' + (get_option('usb_serial_id') ? '1' : '0'),
    '-DENABLE_WATCHDOG=' + (enable_watchdog ? '1' : '0'),
    '-DCALIBRATE_OSCILLATOR=' + (calibrate_oscillator ? '1' : '0'),
    language: 'c',
)

c_compiler = meson.get_compiler('c')

# Needed to work around a bug in newer versions of avr-gcc
if c_compiler.has_argument('--param=min-pagesize=0')
  add_project_arguments(
      '--param=min-pagesize=0',
      language: 'c',
  )
endif

led_ioport = meson.get_cross_property('led_ioport', '')
if led_ioport != ''
  led_bit = meson.get_cross_property('led_bit')
  assert(led_ioport in ['A', 'B', 'C', 'D'], '"@0@" is not a valid I/O port'.format(led_ioport))
  assert(led_bit >= 0 and led_bit <= 7, '@0@ is not a valid port bit'.format(led_bit))

  add_project_arguments(
      '-DLED_IOPORT_NAME=' + led_ioport,
      '-DLED_BIT=' + led_bit.to_string(),
      language: 'c',
  )
endif

usb_intr_cfg = meson.get_cross_property('usb_intr_cfg', [])
foreach c : usb_intr_cfg
  add_project_arguments('-D' + c, language: 'c')
endforeach

add_project_link_arguments(
    '-mmcu=' + host_machine.cpu(),
    language: 'c',
)

include_dir = include_directories('include')

relay_driver = meson.get_cross_property('relay_driver')
subdir('src/drivers/' + relay_driver)

program = executable('hidrelay', [
    'src/main.c',
    'usbdrv/usbdrv.c',
    'usbdrv/usbdrvasm.S',
    'usbdrv/oddebug.c',
  ],
  link_with: libdriver,
  include_directories: [
    include_directories('src', 'usbdrv'),
    include_dir,
  ],
  link_args: [
    '-Wl,-Map,@0@/hidrelay.map'.format(meson.current_build_dir()),
  ]
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
efuse = meson.get_cross_property('efuse', '')


python3 = find_program('python3')

custom_target('fuse-config',
  input: 'scripts/gen_fuses.py',
  output: 'fuses.txt',
  command: [
    python3,
    '@INPUT@',
    '--cpu', host_machine.cpu(),
    '--lfuse', lfuse,
    '--hfuse', hfuse,
    '--efuse', efuse,
    '--output', '@OUTPUT@'
  ]
)

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
  ] + (efuse != '' ? ['-U', 'efuse:w:' + efuse + ':m'] : []),
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
  ] + (efuse != '' ? ['-U', 'efuse:r:-:h'] : []),
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

run_target('readflash',
  command: [
    avrdude,
    '-c', avrdude_programmer,
    '-P', avrdude_port,
    '-p', host_machine.cpu(),
    '-B', avrdude_speed,
    '-U', 'flash:r:flash.hex:i'
  ]
)

avr_size = find_program('size')

run_target('stats',
  depends: [program],
  command: [
    avr_size,
    program,
  ]
)
