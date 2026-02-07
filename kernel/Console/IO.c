// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "Console/IO.h"

static void console_puc(const char c) { platform_console_putc(c); }

static void console_print(const char* text) {
  while (*text != '\0') {
    char character = *text;
    // serial terminal needs carriage return CR ('\r') -- move to line start --
    // to preceed newline NL ('\n') -- move to next line
    if (character == '\n') {
      console_putc('\r');
    }
    console_putc(character);
    ++text;
  }
}
