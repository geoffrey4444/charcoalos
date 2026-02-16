// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

// Base address for memory mapped IO (MMIO) peripherals
#define MMIO_BASE 0x08000000

// virt provides an ARM PL011 UART
#define UART0_BASE (MMIO_BASE + 0x01000000)
#define UART0_DR (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART0_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_FR_TXFF (1u << 5)  // TX FIFO full
#define UART_FR_RXFE (1u << 4)  // RX FIFO empty
#define UART_FR_BUSY (1u << 3)  // UART busy
#define UART_FR_TXFE (1u << 7)  // TX FIFO empty
