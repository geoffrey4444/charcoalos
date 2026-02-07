// Distributed under the MIT license.
// See LICENSE.txt for details.

// Base address for memory mapped IO (MMIO) peripherals
#define MMIO_BASE 0x08000000

// virt provides an ARM PL011 UART
#define UART_BASE (MMIO_BASE + 0x01000000)
#define UART_TX (UART_BASE + 0x00)
#define UART_RX (UART_BASE + 0x00)
