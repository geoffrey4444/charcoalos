// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

/*!
 * platform_console_putc
 * Prints a characater to the serial console, e.g. UART.
 * Implementation is platform dependent.
 * Input: c, a character
 */
void platform_console_putc(char c);

/*!
 * platform_console_getc
 * Returns a character read from the serial console, e.g. UART.
 * Implementation is platform dependent
 */
char platform_console_getc(void);
