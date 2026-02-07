// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"

void console_putc(char c) { platform_console_putc(c); }

void console_print(const char* text) {
  while (*text != '\0') {
    const char character = *text;
    // serial terminal needs carriage return CR ('\r') -- move to line start --
    // to preceed newline NL ('\n') -- move to next line
    if (character == '\n') {
      console_putc('\r');
    }
    console_putc(character);
    ++text;
  }
}
