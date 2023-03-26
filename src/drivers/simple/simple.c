/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Simple relay driver that assumes all relays are contiguous in a single I/O
 * port
 */
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

#define _concat(a, b) a##b
#define concat(a, b) _concat(a, b)

#define RELAY_PORT concat(PORT, RELAY_IOPORT_NAME)
#define RELAY_PIN concat(PIN, RELAY_IOPORT_NAME)
#define RELAY_DDR concat(DDR, RELAY_IOPORT_NAME)
#define RELAY_MASK (((1 << NUM_RELAYS) - 1) << RELAY_OFFSET)

void init_relays(void) {
    RELAY_DDR |= RELAY_MASK;

    set_all_relays(false);
}

void set_all_relays(bool on) {
    if (on) {
#if NUM_RELAYS == 8
        RELAY_PORT = 0xFF;
#else
        RELAY_PORT |= RELAY_MASK;
#endif
    } else {
#if NUM_RELAYS == 8
        RELAY_PORT = 0;
#else
        RELAY_PORT &= ~RELAY_MASK;
#endif
    }
}

void set_relay(uint8_t relay, bool on) {
    if (on) {
        RELAY_PORT |= _BV(relay + RELAY_OFFSET);
    } else {
        RELAY_PORT &= ~_BV(relay + RELAY_OFFSET);
    }
}

uint8_t get_relay_state(void) {
    return (RELAY_PORT & RELAY_MASK) >> RELAY_OFFSET;
}
