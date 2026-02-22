// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Compares two strings
 * \details This does not check that the memory is valid; use with
 * caution when the strings are not known to be valid null-terminated
 * strings (e.g. on the stack). Use `string_compare_with_length()`
 * to compare strings with an expected length.
 * \param text The string
 * \param other The other string
 * \returns True if the strings are identical, false otherwise
 */
bool string_compare(const char *text, const char *other);

/*!
 * \brief Compares two null-terminated strings
 * \details This comparison compares strings up to a given length and then
 * optionally checks that both strings are null-terminated at that length.
 * The expected length does not include the null terminator.
 * \param text The string
 * \param other The other string
 * \param length The number of bytes to compare, excluding the null terminator
 * \param check_null_termination Whether to check both string are
 * null-terminated at the expected length. If false, just compares the first
 * `expected length` bytes of the strings.
 * \returns True if the strings are identical and null terminated, false
 * otherwise
 */
bool string_compare_with_length(const char *text, const char *other,
                                size_t expected_length,
                                bool check_null_termination);

/*!
 * \brief Returns the length of a null-terminated string `str`.
 * \details Just counts the number of characters until the first `'\0'`.
 * \param str The string whose length is to be measured.
 * \returns The size in bytes of the string (excluding the null terminator).
 */
size_t string_length(const char *str);
