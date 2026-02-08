// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include "platform/IO.h"

#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Returns the length of a null-terminated string `str`.
 * \details Just counts the number of characters until the first `'\0'`.
 */
size_t string_length(const char* str);

/*!
 * \brief Writes a single character `c` to the serial console.
 */
void console_putc(char c);

/*!
 * \brief Writes a null-terminated string `text` to the serial console.
 * \details First calls `string_length()` and then uses `console_write()`.
 */
void console_print(const char* text);

/*!
 * \brief Writes a string `buffer` of size `size` to the serial console.
 */
void console_write(const char* buffer, size_t size);

/*!
 * \brief Reads a single character from the serial console.
 */
char console_getc(void);

/*!
 * \brief Reads a buffer `buffer` of size `size` from the serial console.
 * \details Currently just reads literal characters, without handling for
 * backspace/delete, etc. Does not echo the read characters.
 */
void console_read(char* buffer, size_t size);

/*!
 * \brief Reads a line from the serial console.
 * \details Reads until a newline `'\n'` or carriage return `'\b'` is
 * encountered, and then completes the buffer with a null terminator `'\0'`.
 * Optionally echoes the read characters. Handles backspace.
 */
void console_read_line(char* buffer, size_t size, bool echo);
