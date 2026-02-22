// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Memory/Memory.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static size_t g_panic_calls;
static const char *g_last_panic_message;

void console_print(const char *text) { (void)text; }

void console_print_hex_value(const void *data, size_t size) {
  (void)data;
  (void)size;
}

void kernel_panic(const char *message) {
  ++g_panic_calls;
  g_last_panic_message = message;
}

void setUp(void) {
  g_panic_calls = 0;
  g_last_panic_message = NULL;
}

void tearDown(void) {}

void test_physical_memory_regions_decodes_single_32bit_region(void) {
  struct MemoryRegion regions[16] = {0};
  size_t region_count = 0;

  const uint8_t reg_bytes[] = {
      0x40, 0x00, 0x00, 0x00,  // base 0x40000000
      0x08, 0x00, 0x00, 0x00,  // size 0x08000000
  };

  physical_memory_regions(regions, &region_count, 1, 1, reg_bytes,
                          sizeof(reg_bytes));

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, region_count);
  TEST_ASSERT_EQUAL_UINT64(0x40000000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x08000000ULL, (uint64_t)regions[0].size);
}

void test_physical_memory_regions_decodes_two_64bit_regions(void) {
  struct MemoryRegion regions[16] = {0};
  size_t region_count = 0;

  const uint8_t reg_bytes[] = {
      0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,  // base 0x0000000800000000
      0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // size 0x0000000040000000
      0x00, 0x00, 0x00, 0x08, 0x40, 0x00, 0x00, 0x00,  // base 0x0000000840000000
      0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,  // size 0x0000000010000000
  };

  physical_memory_regions(regions, &region_count, 2, 2, reg_bytes,
                          sizeof(reg_bytes));

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(2, region_count);
  TEST_ASSERT_EQUAL_UINT64(0x0000000800000000ULL,
                           (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x0000000040000000ULL, (uint64_t)regions[0].size);
  TEST_ASSERT_EQUAL_UINT64(0x0000000840000000ULL,
                           (uint64_t)regions[1].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x0000000010000000ULL, (uint64_t)regions[1].size);
}

void test_physical_memory_regions_rejects_zero_sized_region(void) {
  struct MemoryRegion regions[16] = {0};
  size_t region_count = 0;

  const uint8_t reg_bytes[] = {
      0x40, 0x00, 0x00, 0x00,  // base 0x40000000
      0x00, 0x00, 0x00, 0x00,  // size 0
  };

  physical_memory_regions(regions, &region_count, 1, 1, reg_bytes,
                          sizeof(reg_bytes));

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Physical memory region reported with zero size",
                           g_last_panic_message);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_physical_memory_regions_decodes_single_32bit_region);
  RUN_TEST(test_physical_memory_regions_decodes_two_64bit_regions);
  RUN_TEST(test_physical_memory_regions_rejects_zero_sized_region);
  return UNITY_END();
}
