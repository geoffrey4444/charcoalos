// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/String/String.h"

#include <stddef.h>

void console_putc(char c) { platform_console_putc(c); }

void console_write(const char* text, size_t size) {
  size_t i = 0;
  while (i < size) {
    const char character = *text;
    // serial terminal needs carriage return CR ('\r') -- move to line start --
    // to preceed newline NL ('\n') -- move to next line
    if (character == '\n') {
      console_putc('\r');
    }
    console_putc(character);
    ++text;
    ++i;
  }
}

void console_print(const char* text) {
  // First, find the length of the string (t must be null terminated)
  console_write(text, string_length(text));
}

char console_getc() { return platform_console_getc(); }

void console_read(char* text, size_t size) {
  if (size == 0) {
    return;
  }

  size_t i = 0;
  while (i < size) {
    text[i] = console_getc();
    ++i;
  }
}

void console_read_line(char* text, size_t size, bool echo) {
  size_t i = 0;

  if (size == 0) {
    return;
  }

  while (i + 1 < size) {
    const char c = console_getc();

    // Terminate and return the string if the character is a newline or
    // carriage return
    if ((c == '\n') || (c == '\r')) {
      text[i] = '\0';

      if (echo) {
        console_putc('\r');
        console_putc('\n');
      }
      return;
    }

    // Handle backspace
    if ((c == '\b') || (c == '\x7f')) {
      if (i == 0) {
        // Do not backspace past beginning of prompt.
        continue;
      }
      // Remove the most recent character from the string
      --i;
      if (echo) {
        // Move left, emit space, move left
        console_print("\b \b");
      }
    } else {
      text[i] = c;
      if (echo) {
        console_putc(c);
      }
      ++i;
    }
  }
  text[i] = '\0';
  return;
}

void console_print_hex(const void* data, size_t size) {
  uint8_t byte;
  uint8_t lo;
  uint8_t hi;
  for (size_t i = size; i > 0; --i) {
    byte = ((char*)data)[i - 1];
    lo = (byte & 15);
    if (lo > 9) {
      lo += ('A' - 10);
    } else {
      lo += '0';
    }

    hi = ((byte >> 4) & 15);
    if (hi > 9) {
      hi += ('A' - 10);
    } else {
      hi += '0';
    }

    console_putc(hi);
    console_putc(lo);
  }
  return;
}

uint64_t console_uint64_from_hex(const char* digits) {
  uint64_t result = 0;
  size_t i = 0;
  size_t max_chars = 16;  // 64 bit = 8 byte, 2 hex chars / byte
  // Skip optional 0x prefix
  if (digits[0] == '0') {
    if ((digits[1] == 'x') || (digits[1] == 'X')) {
      max_chars += 2;
      i = 2;
    }
  }
  while ((digits[i] != '\0') && (i < max_chars)) {
    if ((digits[i] >= '0') && (digits[i] <= '9')) {
      result = (result << 4) | (digits[i] - '0');
    } else if ((digits[i] >= 'A') && (digits[i] <= 'F')) {
      result = (result << 4) | (digits[i] - 'A' + 10);
    } else if ((digits[i] >= 'a') && (digits[i] <= 'f')) {
      result = (result << 4) | (digits[i] - 'a' + 10);
    } else {
      // Non-digit character encountered; stop here
      return result;
    }
    ++i;
  }
  return result;
}
