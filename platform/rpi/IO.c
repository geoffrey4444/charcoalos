// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/rpi/IO.h"

#include <stddef.h>
#include <stdint.h>

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

const char* platform_name(void) { return "Raspberry Pi"; }

void platform_console_tx_flush(void) {
  // Wait until TX is empty (UART_FR_TXFE bit == 1 when empty)
  while ((UART0_FR & UART_FR_TXFE) == 0) {
  }
  // Wait until UART is not busy (UART_FR_BUSY bit == 1 when busy)
  while ((UART0_FR & UART_FR_BUSY) != 0) {
  }
}
