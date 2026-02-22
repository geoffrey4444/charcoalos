// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
void console_print(const char *text);

/*!
 * \brief Writes a string `buffer` of size `size` to the serial console.
 * \param buffer The string to write.
 * \param size The number of bytes to write.
 */
void console_write(const char *buffer, size_t size);

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
void console_read(char *buffer, size_t size);

/*!
 * \brief Reads a line from the serial console.
 * \details Reads until a newline `\n` or carriage return `\r` is
 * encountered, and then completes the buffer with a null terminator `\0`.
 * Optionally echoes the read characters. Handles backspace.
 * \param buffer The buffer to hold the text input.
 * \param size The size in bytes of the buffer holding the input.
 * \param echo Whether or not to also echo the input to the console.
 */
void console_read_line(char *buffer, size_t size, bool echo);

/*!
 * \brief Prints a numerical value as a hex string
 * \details Bytes are printed in reverse memory order (i.e. most significant
 * byte first) with no prefix).
 * \param data The bytes to print as hex
 * \param size The size of the data to print in bytes
 */
void console_print_hex_value(const void *data, size_t size);

/*!
 * \brief Prints a series of bytes as a hex string
 * \details Bytes are printed in input byte order with no prefix.
 * \param data The bytes to print as hex
 * \param size The size of the data to print in bytes
 */
void console_print_hex_bytes(const void* data, size_t size);

/*! 
 * \brief Returns a 64-bit unsigned integer from a hex string
 * \details Converts string character-by-character into a uint64_t.
 * The first 16 characters (after optional 0x or 0X prefix) are interpreted as 
 * hex digits.
 * 
 * The function returns the accumulated total once all digits
 * have been parsed (starting with the most significant digit),
 * when the first non-hex character is encountered,
 * or when 16 digits have been parsed.
 * \param digits The string containing the hex deigits
 * \returns The numerical value of the digits as uint64_t
 */
 uint64_t console_uint64_from_hex(const char *digits);
