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

#define led_on() (LED_PORT |= LED_MASK)
#define led_off() (LED_PORT &= ~LED_MASK)
#define led_toggle() (LED_PORT ^= LED_MASK)
#else
#define led_on()
#define led_off()
#define led_toggle()
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

#if CALIBRATE_OSCILLATOR
uint8_t EEMEM saved_osccal = 0xFF;
#endif

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
      set_serial(&buf[1]);
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

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
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

#if CALIBRATE_OSCILLATOR
void calibrateOscillator(void) {
  uchar step = 128;
  uchar trialValue = 0, optimumValue;
  int x, optimumDev,
      targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

  /* do a binary search: */
  do {
    OSCCAL = trialValue + step;
    x = usbMeasureFrameLength(); /* proportional to current real frequency */
    if (x < targetValue)         /* frequency still too low */
      trialValue += step;
    step >>= 1;
  } while (step > 0);
  /* We have a precision of +/- 1 for optimum OSCCAL here */
  /* now do a neighborhood search for optimum value */
  optimumValue = trialValue;
  optimumDev = x; /* this is certainly far away from optimum */
  for (OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++) {
    x = usbMeasureFrameLength() - targetValue;
    if (x < 0)
      x = -x;
    if (x < optimumDev) {
      optimumDev = x;
      optimumValue = OSCCAL;
    }
  }
  OSCCAL = optimumValue;
}

/*
 * Note: This calibration algorithm may try OSCCAL values of up to 192 even if
 * the optimum value is far below 192. It may therefore exceed the allowed
 * clock frequency of the CPU in low voltage designs!
 * You may replace this search algorithm with any other algorithm you like if
 * you have additional constraints such as a maximum CPU clock.
 * For version 5.x RC oscillators (those with a split range of 2x128 steps,
 * e.g. ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the
 * optimum in both regions.
 */
void usbEventResetReady(void) {
  /* Disable interrupts during oscillator calibration since
   * usbMeasureFrameLength() counts CPU cycles.
   */
  cli();
  calibrateOscillator();
  sei();

  eeprom_update_byte(&saved_osccal, OSCCAL);
}

#endif

int main(void) {
  init_relays();

#ifdef LED_IOPORT_NAME
  LED_DDR |= LED_MASK;
  LED_PORT &= ~LED_MASK;
#endif

#if REPORT_SERIAL
  usbDescriptorStringSerialNumber[0] = USB_STRING_DESCRIPTOR_HEADER(SERIAL_LEN);
  eeprom_read_block(buf, serial, SERIAL_LEN);
  set_ram_serial(buf);
#endif

#if CALIBRATE_OSCILLATOR
  {
    uint8_t val = eeprom_read_byte(&saved_osccal);
    if (val != 0xFF) {
      OSCCAL = val;
    }
  }
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
