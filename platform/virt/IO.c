// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/virt/IO.h"

#include <stdint.h>

static void platform_console_putc(const char c) {
  // static: initialize once, before main()
  static volatile uint8_t* const uart_tx = (uint8_t*)UART_TX;
  *uart_tx = (uint8_t)c;
}
