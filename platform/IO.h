// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stddef.h>

/*!
 * \brief Prints a characater to the serial console, e.g. UART.
 * Implementation is platform dependent.
 * \param c A character
 */
void platform_console_putc(char c);

/*!
 * Returns a character read from the serial console, e.g. UART.
 * Implementation is platform dependent
 * \returns An input character.
 */
char platform_console_getc(void);

/*!
 * \brief Returns the name of the platform
 * \param name The name of the platform
 * \param size The maximum size that name cna hold
 */
const char* platform_name(void);
