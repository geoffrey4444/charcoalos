// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

// Constants for Raspberry Pi 4

// Base address for memory mapped IO (MMIO) peripherals
#define MMIO_BASE 0xFE000000UL

// PL011 UART
// DR = data register (TX)
// FR = flag register (STATUS)
#define UART0_BASE (MMIO_BASE + 0x201000UL)
#define UART0_DR (*(volatile uint32_t *)(UART0_BASE + 0x00)) 
#define UART0_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))
