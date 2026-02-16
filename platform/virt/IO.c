// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/virt/IO.h"

#include <stdint.h>

// __attribute__((weak)) means this is overridable (e.g. in unit tests)
__attribute__((weak)) uint32_t platform_uart0_flags(void) { return UART0_FR; }

static inline void uart_putc(char c) {
  // if bit 5 of UART0_FR is high, the FIFO for TX is full, so wait
  while (UART0_FR & UART_FR_TXFF) {
    // wait until FIFO is not full
  }
  UART0_DR = (uint32_t)c;
}

static inline char uart_getc(void) {
  // if bit 4 of UART0_FR is high, the FIFO for RX is empty, so wait
  while (UART0_FR & UART_FR_RXFE) {
    // wait until FIFO is not empty
  }
  return (char)UART0_DR;
}

void platform_console_putc(char c) { uart_putc(c); }

char platform_console_getc(void) { return uart_getc(); }

const char* platform_name(void) {
  return "QEMU virt";
}

void platform_console_tx_flush(void) {
  // Wait until TX is empty (UART_FR_TXFE bit == 1 when empty)
  while ((platform_uart0_flags() & UART_FR_TXFE) == 0) {
  }
  // Wait until UART is not busy (UART_FR_BUSY bit == 1 when busy)
  while ((platform_uart0_flags() & UART_FR_BUSY) != 0) {
  }
}
