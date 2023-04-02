/*
 * Copyright 2023 Joshua Watt <JPEWhacker@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#include "main.h"

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>

#include "oddebug.h"
#include "usbdrv.h"

#define _concat(a, b) a##b
#define concat(a, b) _concat(a, b)

#ifdef LED_IOPORT_NAME
#define LED_PORT concat(PORT, LED_IOPORT_NAME)
#define LED_PIN concat(PIN, LED_IOPORT_NAME)
#define LED_DDR concat(DDR, LED_IOPORT_NAME)
#define LED_MASK _BV(LED_BIT)
#endif

#define USB_HID_REPORT_TYPE_FEATURE 3
#define GET_REPORT 1
#define SET_REPORT 9

#define CMD_SET_SERIAL 0xFA
#define CMD_ON 0xFF
#define CMD_OFF 0xFD

#define CMD_ALL_ON 0xFE
#define CMD_ALL_OFF 0xFC

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
                   USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH,
               "usbHidReportDescriptor length does not match "
               "USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH");
uint8_t buf[8];
uint8_t buf_len;

uint8_t EEMEM serial[] = {'J', 'P', 'E', 'W', '0'};
_Static_assert(sizeof(serial) == SERIAL_LEN, "Invalid serial number length");

#if REPORT_SERIAL
int usbDescriptorStringSerialNumber[1 + SERIAL_LEN];

void set_ram_serial(uint8_t const *data) {
    for (uint8_t i = 0; i < SERIAL_LEN; i++) {
        usbDescriptorStringSerialNumber[i + 1] = data[i];
    }
}
#endif

void set_serial(uint8_t const *data) {
    eeprom_write_block(data, serial, SERIAL_LEN);
#if REPORT_SERIAL
    set_ram_serial(data);
#endif
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
        case CMD_SET_SERIAL:
            if (buf_len == 8) {
                set_serial(buf);
                return 1;
            }
            break;

        case CMD_ALL_OFF:
            set_all_relays(false);
            return 1;

        case CMD_ALL_ON:
            set_all_relays(true);
            return 1;

        case CMD_ON:
        case CMD_OFF:
            if (buf_len >= 2) {
                if (buf[1] >= 1 && buf[1] <= NUM_RELAYS) {
                    set_relay(buf[1] - 1, buf[0] == CMD_ON);
                }
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
                eeprom_read_block(buf, serial, SERIAL_LEN);
                buf[6] = 0;
                buf[7] = get_relay_state();

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
    init_relays();

#ifdef LED_IOPORT_NAME
    LED_DDR |= LED_MASK;
    LED_PORT &= ~LED_MASK;
#endif

#if REPORT_SERIAL
    usbDescriptorStringSerialNumber[0] =
        USB_STRING_DESCRIPTOR_HEADER(SERIAL_LEN);
    eeprom_read_block(buf, serial, SERIAL_LEN);
    set_ram_serial(buf);
#endif

#if ENABLE_WATCHDOG
    wdt_enable(WDTO_1S);
#endif

    odDebugInit();
    usbInit();
    usbDeviceDisconnect();
    for (uint8_t i = 0; i < 250; i++) {
#if ENABLE_WATCHDOG
        wdt_reset();
#endif
        _delay_ms(1);
    }

    usbDeviceConnect();
    sei();

    while (true) {
#if ENABLE_WATCHDOG
        wdt_reset();
#endif
        usbPoll();
    }
}
