// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

static char g_tx_buffer[256];
static size_t g_tx_index;

static char g_rx_buffer[256];
static size_t g_rx_size;
static size_t g_rx_index;

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) {
  if (g_rx_index < g_rx_size) {
    return g_rx_buffer[g_rx_index++];
  }
  return '\0';
}

static void load_rx(const char* data, size_t size) {
  TEST_ASSERT_TRUE(size <= sizeof(g_rx_buffer));
  memcpy(g_rx_buffer, data, size);
  g_rx_size = size;
  g_rx_index = 0;
}

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
  g_tx_index = 0;
  g_rx_size = 0;
  g_rx_index = 0;
}

void tearDown(void) {}

void test_string_length_returns_expected_length(void) {
  TEST_ASSERT_EQUAL_size_t(0, string_length(""));
  TEST_ASSERT_EQUAL_size_t(5, string_length("hello"));
}

void test_console_putc_writes_one_character(void) {
  console_putc('K');
  TEST_ASSERT_EQUAL_size_t(1, g_tx_index);
  TEST_ASSERT_EQUAL_CHAR('K', g_tx_buffer[0]);
}

void test_console_write_respects_size_and_translates_newline(void) {
  const char input[] = {'A', '\n', 'B', '\0', 'C'};
  const char expected[] = {'A', '\r', '\n', 'B', '\0'};

  console_write(input, 4);

  TEST_ASSERT_EQUAL_size_t(sizeof(expected), g_tx_index);
  TEST_ASSERT_EQUAL_MEMORY(expected, g_tx_buffer, sizeof(expected));
}

void test_console_getc_returns_platform_character(void) {
  const char rx[] = {'Q'};

  load_rx(rx, sizeof(rx));

  TEST_ASSERT_EQUAL_CHAR('Q', console_getc());
  TEST_ASSERT_EQUAL_size_t(1, g_rx_index);
}

void test_console_read_reads_requested_byte_count(void) {
  const char rx[] = {'x', 'y', 'z'};
  char out[3] = {0};

  load_rx(rx, sizeof(rx));
  console_read(out, sizeof(out));

  TEST_ASSERT_EQUAL_MEMORY(rx, out, sizeof(out));
  TEST_ASSERT_EQUAL_size_t(sizeof(rx), g_rx_index);
}

void test_console_read_with_zero_size_does_not_consume_input(void) {
  const char rx[] = {'x'};
  char out[1] = {0};

  load_rx(rx, sizeof(rx));
  console_read(out, 0);

  TEST_ASSERT_EQUAL_size_t(0, g_rx_index);
  TEST_ASSERT_EQUAL_CHAR('\0', out[0]);
}

void test_console_read_line_stops_at_newline_and_terminates(void) {
  const char rx[] = {'a', 'b', 'c', '\n', 'd'};
  char out[8] = {0};

  load_rx(rx, sizeof(rx));
  console_read_line(out, sizeof(out));

  TEST_ASSERT_EQUAL_STRING("abc", out);
  TEST_ASSERT_EQUAL_size_t(4, g_rx_index);
}

void test_console_read_line_with_zero_size_does_not_consume_input(void) {
  const char rx[] = {'a'};
  char out[1] = {0};

  load_rx(rx, sizeof(rx));
  console_read_line(out, 0);

  TEST_ASSERT_EQUAL_size_t(0, g_rx_index);
  TEST_ASSERT_EQUAL_CHAR('\0', out[0]);
}

void test_console_read_line_truncates_and_null_terminates(void) {
  const char rx[] = {'a', 'b', 'c', 'd', 'e'};
  char out[4] = {0};

  load_rx(rx, sizeof(rx));
  console_read_line(out, sizeof(out));

  TEST_ASSERT_EQUAL_STRING("abc", out);
  TEST_ASSERT_EQUAL_size_t(3, g_rx_index);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_string_length_returns_expected_length);
  RUN_TEST(test_console_putc_writes_one_character);
  RUN_TEST(test_console_write_respects_size_and_translates_newline);
  RUN_TEST(test_console_getc_returns_platform_character);
  RUN_TEST(test_console_read_reads_requested_byte_count);
  RUN_TEST(test_console_read_with_zero_size_does_not_consume_input);
  RUN_TEST(test_console_read_line_stops_at_newline_and_terminates);
  RUN_TEST(test_console_read_line_with_zero_size_does_not_consume_input);
  RUN_TEST(test_console_read_line_truncates_and_null_terminates);
  return UNITY_END();
}
