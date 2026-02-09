// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include "platform/IO.h"

#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Writes a single character `c` to the serial console.
 * \param c The character to write.
 */
void console_putc(char c);

/*!
 * \brief Writes a null-terminated string `text` to the serial console.
 * \details First calls `string_length()` and then uses `console_write()`.
 * \param text The string to print.
 */
void console_print(const char* text);

/*!
 * \brief Writes a string `buffer` of size `size` to the serial console.
 * \param buffer The string to write.
 * \param size The number of bytes to write.
 */
void console_write(const char* buffer, size_t size);

/*!
 * \brief Reads a single character from the serial console.
 * \returns A character.
 */
char console_getc(void);

/*!
 * \brief Reads a buffer `buffer` of size `size` from the serial console.
 * \details Currently just reads literal characters, without handling for
 * backspace/delete, etc. Does not echo the read characters.
 * \param buffer The buffer to hold the text input.
 * \param size The size in bytes of the buffer holding the input.
 */
void console_read(char* buffer, size_t size);

/*!
 * \brief Reads a line from the serial console.
 * \details Reads until a newline `\n` or carriage return `\r` is
 * encountered, and then completes the buffer with a null terminator `\0`.
 * Optionally echoes the read characters. Handles backspace.
 * \param buffer The buffer to hold the text input.
 * \param size The size in bytes of the buffer holding the input.
 * \param echo Whether or not to also echo the input to the console.
 */
void console_read_line(char* buffer, size_t size, bool echo);
