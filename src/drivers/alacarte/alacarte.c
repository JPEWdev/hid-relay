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

#define DDRn(n) concat(DDR, n)
#define PORTn(n) concat(PORT, n)

#define RELAY_IOPORT_NAME(n) threecat(RELAY_, n, _IOPORT_NAME)

#define RELAY_DDR(n) DDRn(RELAY_IOPORT_NAME(n))
#define RELAY_PORT(n) PORTn(RELAY_IOPORT_NAME(n))
#define RELAY_MASK(n) (1 << threecat(RELAY_, n, _BIT))

#define INIT_RELAY(n) RELAY_DDR(n) |= RELAY_MASK(n)

#define SET_RELAY(n)                                                           \
  if (relay == n - 1) {                                                        \
    if (on) {                                                                  \
      RELAY_PORT(n) |= RELAY_MASK(n);                                          \
    } else {                                                                   \
      RELAY_PORT(n) &= ~RELAY_MASK(n);                                         \
    }                                                                          \
  }

#define GET_RELAY(n)                                                           \
  if (RELAY_PORT(n) & RELAY_MASK(n)) {                                         \
    state |= (1 << (n - 1));                                                   \
  }

#define INIT_RELAY_1() INIT_RELAY(1)
#define GET_RELAY_1() GET_RELAY(1)
#define SET_RELAY_1() SET_RELAY(1)

#if NUM_RELAYS >= 2
#define INIT_RELAY_2() INIT_RELAY(2)
#define GET_RELAY_2() GET_RELAY(2)
#define SET_RELAY_2() SET_RELAY(2)
#else
#define INIT_RELAY_2()
#define GET_RELAY_2()
#define SET_RELAY_2()
#endif

#if NUM_RELAYS >= 3
#define INIT_RELAY_3() INIT_RELAY(3)
#define GET_RELAY_3() GET_RELAY(3)
#define SET_RELAY_3() SET_RELAY(3)
#else
#define INIT_RELAY_3()
#define GET_RELAY_3()
#define SET_RELAY_3()
#endif

#if NUM_RELAYS >= 4
#define INIT_RELAY_4() INIT_RELAY(4)
#define GET_RELAY_4() GET_RELAY(4)
#define SET_RELAY_4() SET_RELAY(4)
#else
#define INIT_RELAY_4()
#define GET_RELAY_4()
#define SET_RELAY_4()
#endif

#if NUM_RELAYS >= 5
#define INIT_RELAY_5() INIT_RELAY(5)
#define GET_RELAY_5() GET_RELAY(5)
#define SET_RELAY_5() SET_RELAY(5)
#else
#define INIT_RELAY_5()
#define GET_RELAY_5()
#define SET_RELAY_5()
#endif

#if NUM_RELAYS >= 6
#define INIT_RELAY_6() INIT_RELAY(6)
#define GET_RELAY_6() GET_RELAY(6)
#define SET_RELAY_6() SET_RELAY(6)
#else
#define INIT_RELAY_6()
#define GET_RELAY_6()
#define SET_RELAY_6()
#endif

#if NUM_RELAYS >= 7
#define INIT_RELAY_7() INIT_RELAY(7)
#define GET_RELAY_7() GET_RELAY(7)
#define SET_RELAY_7() SET_RELAY(7)
#else
#define INIT_RELAY_7()
#define GET_RELAY_7()
#define SET_RELAY_7()
#endif

#if NUM_RELAYS >= 8
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
