// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/IO.h"
#include "platform/virt/IO.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>

static const uint32_t* g_flags_sequence = NULL;
static size_t g_flags_sequence_size = 0;
static size_t g_flags_sequence_index = 0;
static size_t g_flags_read_count = 0;

uint32_t platform_uart0_flags(void) {
  ++g_flags_read_count;
  if (g_flags_sequence_index < g_flags_sequence_size) {
    return g_flags_sequence[g_flags_sequence_index++];
  }
  if (g_flags_sequence_size == 0) {
    return 0;
  }
  return g_flags_sequence[g_flags_sequence_size - 1];
}

static void load_flags_sequence(const uint32_t* sequence, size_t size) {
  g_flags_sequence = sequence;
  g_flags_sequence_size = size;
  g_flags_sequence_index = 0;
  g_flags_read_count = 0;
}

void setUp(void) {}

void tearDown(void) {}

void test_platform_console_tx_flush_waits_until_tx_empty_and_not_busy(void) {
  const uint32_t sequence[] = {
      0u, UART_FR_TXFE | UART_FR_BUSY, UART_FR_TXFE | UART_FR_BUSY, UART_FR_TXFE};

  load_flags_sequence(sequence, sizeof(sequence) / sizeof(sequence[0]));
  platform_console_tx_flush();

  TEST_ASSERT_EQUAL_size_t(4, g_flags_read_count);
}

void test_platform_console_tx_flush_returns_immediately_when_already_flushed(void) {
  const uint32_t sequence[] = {UART_FR_TXFE};

  load_flags_sequence(sequence, sizeof(sequence) / sizeof(sequence[0]));
  platform_console_tx_flush();

  TEST_ASSERT_EQUAL_size_t(2, g_flags_read_count);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_platform_console_tx_flush_waits_until_tx_empty_and_not_busy);
  RUN_TEST(test_platform_console_tx_flush_returns_immediately_when_already_flushed);
  return UNITY_END();
}
