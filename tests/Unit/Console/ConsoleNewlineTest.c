// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

static char g_uart_buffer[128];
static size_t g_uart_index = 0;

void platform_console_putc(char c) {
  if (g_uart_index < sizeof(g_uart_buffer) - 1) {
    g_uart_buffer[g_uart_index++] = c;
  }
}

void setUp(void) {}

void tearDown(void) {}

void test_console_print_newline_adds_carriage_return(void) {
  const char* input = "alpha\nbeta\n";
  const char* expected = "alpha\r\nbeta\r\n";

  memset(g_uart_buffer, 0, sizeof(g_uart_buffer));
  g_uart_index = 0;

  console_print(input);
  g_uart_buffer[g_uart_index] = '\0';

  TEST_ASSERT_EQUAL_STRING(expected, g_uart_buffer);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_console_print_newline_adds_carriage_return);
  return UNITY_END();
}
