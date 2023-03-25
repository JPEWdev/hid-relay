/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>

#include "oddebug.h"
#include "usbdrv.h"

#define LED_PORT PORTB
#define LED_PIN PINB
#define LED_DDR DDRB
#define LED_MASK _BV(1)

#define RELAY_PORT PORTA
#define RELAY_PIN PINA
#define RELAY_DDR DDRA

#define USB_HID_REPORT_TYPE_FEATURE 3
#define GET_REPORT 1
#define SET_REPORT 9

static uint8_t relay_state;
#define set_relays() (RELAY_PORT = relay_state)

PROGMEM const char usbHidReportDescriptor[] = {
    // clang-format off
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x09, 0x00,        //   Usage (0x00)
    0xB2, 0x02, 0x01,  //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    0xC0,              // End Collection
    // clang-format on
};
_Static_assert(sizeof(usbHidReportDescriptor) ==
               USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

static uint8_t buf[8];
static uint8_t buf_len;

static uint8_t EEMEM serial[6] = {'J', 'P', 'E', 'W', ' ', ' '};

void set_serial(uint8_t const *data) {
    eeprom_write_block(data, serial, sizeof(serial));
}

void set_all_relays(bool on) {
    relay_state = on ? (1 << NUM_RELAYS) - 1 : 0x00;
    set_relays();
}

void set_relay(uint8_t relay, bool on) {
    if (relay >= NUM_RELAYS) {
        return;
    }

    if (on) {
        relay_state |= _BV(relay);
    } else {
        relay_state &= ~_BV(relay);
    }

    set_relays();
}

uchar usbFunctionWrite(uchar *data, uchar len) {
    if (buf_len + len >= sizeof(buf)) {
        len = sizeof(buf) - buf_len;
    }

    if (len == 0) {
        return 0xff;
    }

    memcpy(&buf[buf_len], data, len);
    buf_len += len;

    switch (buf[0]) {
        case 0xFA:
            if (buf_len == 8) {
                set_serial(buf);
                return 1;
            }
            break;

        case 0xFC:
            set_all_relays(false);
            return 1;
            break;

        case 0xFD:
            if (buf_len >= 2) {
                set_relay(buf[1], false);
                return 1;
            }
            break;

        case 0xFE:
            set_all_relays(true);
            return 1;
            break;

        case 0xFF:
            if (buf_len >= 2) {
                set_relay(buf[1], true);
                return 1;
            }
            break;
    }

    return 0;
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR) {
        DBG1(0x50, &rq->bRequest, 1); /* debug output: print our request */
        if (rq->bRequest == GET_REPORT) {
            if (rq->wValue.bytes[0] == 0 &&
                rq->wValue.bytes[1] == USB_HID_REPORT_TYPE_FEATURE) {
                eeprom_read_block(buf, serial, sizeof(serial));
                buf[6] = 0;
                buf[7] = relay_state;

                usbMsgPtr = buf;

                return 8;
            }

        } else if (rq->bRequest == SET_REPORT) {
            if (rq->wValue.bytes[0] == 0 &&
                rq->wValue.bytes[1] == USB_HID_REPORT_TYPE_FEATURE) {
                memset(buf, 0, sizeof(buf));
                buf_len = 0;

                return USB_NO_MSG;
            }
        }
    } else {
        /* class requests USBRQ_HID_GET_REPORT and USBRQ_HID_SET_REPORT are
         * not implemented since we never call them. The operating system
         * won't call them either because our descriptor defines no meaning.
         */
    }
    return 0; /* default for not implemented requests: return no data back to
                 host */
}

int main(void) {
    RELAY_DDR = 0xFF;
    RELAY_PORT = 0;
    relay_state = 0;

    LED_DDR |= LED_MASK;
    LED_PORT &= ~LED_MASK;

    wdt_enable(WDTO_1S);

    odDebugInit();
    usbInit();
    usbDeviceDisconnect();
    for (uint8_t i = 0; i < 250; i++) {
        wdt_reset();
        _delay_ms(1);
    }

    usbDeviceConnect();
    sei();

    while (true) {
        wdt_reset();
        usbPoll();
    }
}