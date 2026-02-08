// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"

size_t string_length(const char* string) {
  size_t i = 0;
  while (string[i] != '\0') {
    ++i;
  }
  return i;
}

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
