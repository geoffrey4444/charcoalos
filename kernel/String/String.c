// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/String/String.h"

#include <stdbool.h>
#include <stddef.h>

bool string_compare(const char *text, const char *other) {
  const size_t length_text = string_length(text);
  const size_t length_other = string_length(other);
  if (length_text != length_other) {
    return false;
  }
  for (size_t i = 0; i < length_text; ++i) {
    if (text[i] != other[i]) {
      return false;
    }
  }
  return true;
}

bool string_compare_with_length(const char *text, const char *other,
                                size_t expected_length,
                                bool check_null_termination) {
  if (check_null_termination) {
    if ((text[expected_length] != '\0') || (other[expected_length] != '\0')) {
      return false;
    }
  }
  for (size_t i = 0; i < expected_length; ++i) {
    if (text[i] != other[i]) {
      return false;
    }
  }
  return true;
}

size_t string_length(const char *string) {
  size_t i = 0;
  while (string[i] != '\0') {
    ++i;
  }
  return i;
}
