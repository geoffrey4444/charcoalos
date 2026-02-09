// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Compares two strings
 * \param text The string
 * \param other The other string
 * \returns True if the strings are identical, false otherwise
 */
bool string_compare(const char *text, const char *other);

/*!
 * \brief Returns the length of a null-terminated string `str`.
 * \details Just counts the number of characters until the first `'\0'`.
 * \param str The string whose length is to be measured.
 * \returns The size in bytes of the string.
 */
size_t string_length(const char *str);
