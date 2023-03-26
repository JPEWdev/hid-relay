/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef _MAIN_H

#include <stdbool.h>
#include <stdint.h>

void init_relays(void);
void set_all_relays(bool on);
void set_relay(uint8_t relay, bool on);
uint8_t get_relay_state(void);

#endif /* _MAIN_H */
