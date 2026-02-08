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

void console_read_line(char* text, size_t size) {
  size_t i = 0;

  if (size == 0) {
    return;
  }

  while (i + 1 < size) {
    const char c = console_getc();
    if ((c == '\n') || (c == '\r')) {
      text[i] = '\0';
      return;
    }
    text[i] = c;
    ++i;
  }
  text[i] = '\0';
  return;
}
