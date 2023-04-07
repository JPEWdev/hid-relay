/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Implements an a la carte driver where any relay can be on any I/O port and
 * bit. While flexible, this driver uses the most code space; if you want to to
 * fit on a flash constrained device, designing your hardware to be able to use
 * the "simple" driver is recommended
 */
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

#define _concat(a, b) a##b
#define concat(a, b) _concat(a, b)

#define _threecat(a, b, c) a##b##c
#define threecat(a, b, c) _threecat(a, b, c)

#define INIT_RELAY(n) threecat(RELAY_, n, _DDR) |= threecat(RELAY_, n, _MASK);

#define SET_RELAY(n)                                                           \
  if (relay == n - 1) {                                                        \
    if (on) {                                                                  \
      threecat(RELAY_, n, _PORT) |= threecat(RELAY_, n, _MASK);                \
    } else {                                                                   \
      threecat(RELAY_, n, _PORT) &= ~threecat(RELAY_, n, _MASK);               \
    }                                                                          \
  }

#define GET_RELAY(n)                                                           \
  if (threecat(RELAY_, n, _PORT) & threecat(RELAY_, n, _MASK)) {               \
    state |= _BV(n - 1);                                                       \
  }

#define RELAY_1_PORT concat(PORT, RELAY_1_IOPORT_NAME)
#define RELAY_1_DDR concat(DDR, RELAY_1_IOPORT_NAME)
#define RELAY_1_MASK _BV(RELAY_1_BIT)
#define INIT_RELAY_1() INIT_RELAY(1)
#define GET_RELAY_1() GET_RELAY(1)
#define SET_RELAY_1() SET_RELAY(1)

#if NUM_RELAYS >= 2
#define RELAY_2_PORT concat(PORT, RELAY_2_IOPORT_NAME)
#define RELAY_2_DDR concat(DDR, RELAY_2_IOPORT_NAME)
#define RELAY_2_MASK _BV(RELAY_2_BIT)
#define INIT_RELAY_2() INIT_RELAY(2)
#define GET_RELAY_2() GET_RELAY(2)
#define SET_RELAY_2() SET_RELAY(2)
#else
#define INIT_RELAY_2()
#define GET_RELAY_2()
#define SET_RELAY_2()
#endif

#if NUM_RELAYS >= 3
#define RELAY_3_PORT concat(PORT, RELAY_3_IOPORT_NAME)
#define RELAY_3_DDR concat(DDR, RELAY_3_IOPORT_NAME)
#define RELAY_3_MASK _BV(RELAY_2_BIT)
#define INIT_RELAY_3() INIT_RELAY(3)
#define GET_RELAY_3() GET_RELAY(3)
#define SET_RELAY_3() SET_RELAY(3)
#else
#define INIT_RELAY_3()
#define GET_RELAY_3()
#define SET_RELAY_3()
#endif

#if NUM_RELAYS >= 4
#define RELAY_4_PORT concat(PORT, RELAY_4_IOPORT_NAME)
#define RELAY_4_DDR concat(DDR, RELAY_4_IOPORT_NAME)
#define RELAY_4_MASK _BV(RELAY_4_BIT)
#define INIT_RELAY_4() INIT_RELAY(4)
#define GET_RELAY_4() GET_RELAY(4)
#define SET_RELAY_4() SET_RELAY(4)
#else
#define INIT_RELAY_4()
#define GET_RELAY_4()
#define SET_RELAY_4()
#endif

#if NUM_RELAYS >= 5
#define RELAY_5_PORT concat(PORT, RELAY_5_IOPORT_NAME)
#define RELAY_5_DDR concat(DDR, RELAY_5_IOPORT_NAME)
#define RELAY_5_MASK _BV(RELAY_5_BIT)
#define INIT_RELAY_5() INIT_RELAY(5)
#define GET_RELAY_5() GET_RELAY(5)
#define SET_RELAY_5() SET_RELAY(5)
#else
#define INIT_RELAY_5()
#define GET_RELAY_5()
#define SET_RELAY_5()
#endif

#if NUM_RELAYS >= 6
#define RELAY_6_PORT concat(PORT, RELAY_6_IOPORT_NAME)
#define RELAY_6_DDR concat(DDR, RELAY_6_IOPORT_NAME)
#define RELAY_6_MASK _BV(RELAY_6_BIT)
#define INIT_RELAY_6() INIT_RELAY(6)
#define GET_RELAY_6() GET_RELAY(6)
#define SET_RELAY_6() SET_RELAY(6)
#else
#define INIT_RELAY_6()
#define GET_RELAY_6()
#define SET_RELAY_6()
#endif

#if NUM_RELAYS >= 7
#define RELAY_7_PORT concat(PORT, RELAY_7_IOPORT_NAME)
#define RELAY_7_DDR concat(DDR, RELAY_7_IOPORT_NAME)
#define RELAY_7_MASK _BV(RELAY_7_BIT)
#define INIT_RELAY_7() INIT_RELAY(7)
#define GET_RELAY_7() GET_RELAY(7)
#define SET_RELAY_7() SET_RELAY(7)
#else
#define INIT_RELAY_7()
#define GET_RELAY_7()
#define SET_RELAY_7()
#endif

#if NUM_RELAYS >= 8
#define RELAY_8_PORT concat(PORT, RELAY_8_IOPORT_NAME)
#define RELAY_8_DDR concat(DDR, RELAY_8_IOPORT_NAME)
#define RELAY_8_MASK _BV(RELAY_8_BIT)
#define INIT_RELAY_8() INIT_RELAY(8)
#define GET_RELAY_8() GET_RELAY(8)
#define SET_RELAY_8() SET_RELAY(8)
#else
#define INIT_RELAY_8()
#define GET_RELAY_8()
#define SET_RELAY_8()
#endif

#define DO_RELAY(action)                                                       \
  concat(action, _RELAY_1)();                                                  \
  concat(action, _RELAY_2)();                                                  \
  concat(action, _RELAY_3)();                                                  \
  concat(action, _RELAY_4)();                                                  \
  concat(action, _RELAY_5)();                                                  \
  concat(action, _RELAY_6)();                                                  \
  concat(action, _RELAY_7)();                                                  \
  concat(action, _RELAY_8)();

void init_relays(void) {
  DO_RELAY(INIT)
  set_all_relays(false);
}

void set_all_relays(bool on) {
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    set_relay(i, on);
  }
}

void set_relay(uint8_t relay, bool on) { DO_RELAY(SET); }

uint8_t get_relay_state(void) {
  uint8_t state = 0;

  DO_RELAY(GET);
  return state;
}
