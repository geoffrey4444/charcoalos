// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/IO.h"
#include "platform/Info.h"
#include "platform/virt/IO.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_virt_mmio_base_address_matches_platform_constant(void) {
  TEST_ASSERT_EQUAL_HEX64((uint64_t)MMIO_BASE, (uint64_t)mmio_base_address());
}

void test_virt_uart_base_address_matches_platform_constant(void) {
  TEST_ASSERT_EQUAL_HEX64((uint64_t)UART0_BASE, (uint64_t)uart_base_address());
}

void test_virt_platform_name_matches_expected_name(void) {
  TEST_ASSERT_EQUAL_STRING("QEMU virt", platform_name());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_virt_mmio_base_address_matches_platform_constant);
  RUN_TEST(test_virt_uart_base_address_matches_platform_constant);
  RUN_TEST(test_virt_platform_name_matches_expected_name);
  return UNITY_END();
}
