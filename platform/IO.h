// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

/*!
 * platform_console_putc
 * Prints a characater to the serial console, e.g. UART.
 * Implementation is platform dependent.
 * Input: c, a character
 */
static void platform_console_putc(const char c);
