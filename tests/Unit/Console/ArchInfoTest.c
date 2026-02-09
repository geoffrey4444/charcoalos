// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_current_exception_level_returns_zero_on_host(void) {
  TEST_ASSERT_EQUAL_HEX64((uint64_t)0, (uint64_t)current_exception_level());
}

void test_mpidr_el1_returns_zero_on_host(void) {
  TEST_ASSERT_EQUAL_HEX64((uint64_t)0, (uint64_t)mpidr_el1());
}

void test_sctlr_el1_returns_zero_on_host(void) {
  TEST_ASSERT_EQUAL_HEX64((uint64_t)0, (uint64_t)sctlr_el1());
}

void test_arch_name_returns_arm64(void) {
  TEST_ASSERT_EQUAL_STRING("arm64", arch_name());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_current_exception_level_returns_zero_on_host);
  RUN_TEST(test_mpidr_el1_returns_zero_on_host);
  RUN_TEST(test_sctlr_el1_returns_zero_on_host);
  RUN_TEST(test_arch_name_returns_arm64);
  return UNITY_END();
}
