/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Implements the relay driver for the dcttech 8 channel boards, which split
 * the relays between port C[0:5] and port D[0:1]
 */
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

#define C_MASK (0x3F)
#define D_OFFSET (6)
#define D_MASK (0x03)

void init_relays(void) {
    DDRC |= C_MASK;
    DDRD |= D_MASK;

    set_all_relays(false);
}

void set_all_relays(bool on) {
    if (on) {
        PORTC |= C_MASK;
        PORTD |= D_MASK;
    } else {
        PORTC &= ~C_MASK;
        PORTD &= ~D_MASK;
    }
}

void set_relay(uint8_t relay, bool on) {
    uint8_t c_mask = 0;
    uint8_t d_mask = 0;
    switch (relay) {
        case 0:
            d_mask = _BV(1);
            break;
        case 1:
            d_mask = _BV(0);
            break;
        case 2:
            c_mask = _BV(5);
            break;
        case 3:
            c_mask = _BV(4);
            break;
        case 4:
            c_mask = _BV(3);
            break;
        case 5:
            c_mask = _BV(2);
            break;
        case 6:
            c_mask = _BV(1);
            break;
        case 7:
            c_mask = _BV(0);
            break;
    }

    if (on) {
        PORTC |= c_mask;
        PORTD |= d_mask;
    } else {
        PORTC &= ~c_mask;
        PORTD &= ~d_mask;
    }
}

uint8_t get_relay_state(void) {
    uint8_t state = 0;
    if (PORTC & _BV(0)) {
        state |= _BV(7);
    }
    if (PORTC & _BV(1)) {
        state |= _BV(6);
    }
    if (PORTC & _BV(2)) {
        state |= _BV(5);
    }
    if (PORTC & _BV(3)) {
        state |= _BV(4);
    }
    if (PORTC & _BV(4)) {
        state |= _BV(3);
    }
    if (PORTC & _BV(5)) {
        state |= _BV(2);
    }
    if (PORTD & _BV(0)) {
        state |= _BV(1);
    }
    if (PORTD & _BV(1)) {
        state |= _BV(0);
    }
    return state;
}

