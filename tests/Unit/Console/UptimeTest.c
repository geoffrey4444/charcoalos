// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Time/Uptime.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>

uint64_t interrupt_frequency_in_hz(void) { return 100; }

void setUp(void) {}

void tearDown(void) {}

void test_uptime_returns_milliseconds_based_on_interrupt_ticks(void) {
  TEST_ASSERT_EQUAL_UINT64(0, uptime());

  increment_uptime_by_one_tick();
  TEST_ASSERT_EQUAL_UINT64(10, uptime());

  increment_uptime_by_one_tick();
  TEST_ASSERT_EQUAL_UINT64(20, uptime());

  for (size_t i = 0; i < 98; ++i) {
    increment_uptime_by_one_tick();
  }
  TEST_ASSERT_EQUAL_UINT64(1000, uptime());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_uptime_returns_milliseconds_based_on_interrupt_ticks);
  return UNITY_END();
}
