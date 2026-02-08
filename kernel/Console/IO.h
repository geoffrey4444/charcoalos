// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include "platform/IO.h"

#include <stddef.h>

size_t string_length(const char* str);

void console_putc(char c);
void console_print(const char* text);
void console_write(const char* buffer, size_t size);

char console_getc(void);
void console_read(char* buffer, size_t size);
void console_read_line(char* buffer, size_t size);
