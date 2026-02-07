// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/rpi/IO.h"

#include <stdint.h>

static inline void uart_putc(char c) {
  // if bit 5 of UART0_FR is high, the FIFO for UART is full, so wait
  while (UART0_FR & (1u << 5)) {
    // wait until FFIO is not full
  }
  UART0_DR = (uint32_t)c;
}

void platform_console_putc(char c) {
  uart_putc(c);
}
