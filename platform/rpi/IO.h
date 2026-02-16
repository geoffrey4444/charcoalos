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
#define UART_FR_TXFF (1u << 5)  // TX FIFO full
#define UART_FR_RXFE (1u << 4)  // RX FIFO empty
#define UART_FR_BUSY (1u << 3)  // UART busy
#define UART_FR_TXFE (1u << 7)  // TX FIFO empty

// Watchdog block: for triggering a reset
// These are defined e.g. in
// https://android.googlesource.com/kernel/common/%2B/6754baabb890/drivers/watchdog/bcm2835_wdt.c?utm_source=chatgpt.com
#define WDOG_BASE (MMIO_BASE + 0x00100000UL)
#define WDOG_PM_RSTC (WDOG_BASE + 0x1c) // PM = power manager, RSTC = reset controller
#define WDOG_PM_WDOG (WDOG_BASE + 0x24)
#define WDOG_PM_PASSWORD 0x5a000000u
#define WDOG_PM_RSTC_FULL_RESET 0x20
#define WDOG_PM_RSTC_WRCFG_CLR 0xffffffcfu // WRCFG = watchdog reset config
