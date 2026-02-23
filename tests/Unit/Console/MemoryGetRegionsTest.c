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

static void expect_single_region(const struct MemoryRegion *regions,
                                 size_t count, uintptr_t base, size_t size) {
  TEST_ASSERT_EQUAL_size_t(1, count);
  TEST_ASSERT_EQUAL_UINT64((uint64_t)base, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64((uint64_t)size, (uint64_t)regions[0].size);
}

void test_get_memory_regions_decodes_single_32bit_region(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00};

  get_memory_regions(regions, &count, 1, 1, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  expect_single_region(regions, count, 0x40000000u, 0x08000000u);
}

void test_get_memory_regions_decodes_multiple_64bit_regions(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {
      0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x08, 0x40, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  };

  get_memory_regions(regions, &count, 2, 2, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(2, count);
  TEST_ASSERT_EQUAL_UINT64(0x0000000800000000ULL,
                           (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x0000000040000000ULL, (uint64_t)regions[0].size);
  TEST_ASSERT_EQUAL_UINT64(0x0000000840000000ULL,
                           (uint64_t)regions[1].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x0000000010000000ULL, (uint64_t)regions[1].size);
}

void test_get_memory_regions_appends_to_existing_regions(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 1;
  regions[0].base_address = 0x1000u;
  regions[0].size = 0x1000u;
  const uint8_t reg[] = {0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00};

  get_memory_regions(regions, &count, 1, 1, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(2, count);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)regions[0].size);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)regions[1].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)regions[1].size);
}

void test_get_memory_regions_rejects_null_arguments(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {0, 0, 0, 1, 0, 0, 0, 1};

  get_memory_regions(NULL, &count, 1, 1, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  get_memory_regions(regions, NULL, 1, 1, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  get_memory_regions(regions, &count, 1, 1, NULL, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_get_memory_regions_rejects_zero_or_invalid_cell_counts(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {0, 0, 0, 1, 0, 0, 0, 1};

  get_memory_regions(regions, &count, 0, 1, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  get_memory_regions(regions, &count, 1, 0, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  get_memory_regions(regions, &count, 3, 1, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  get_memory_regions(regions, &count, 1, 3, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_get_memory_regions_rejects_bad_reg_size_constraints(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;

  const uint8_t short_reg[] = {0, 0, 0, 1};
  get_memory_regions(regions, &count, 2, 1, short_reg, sizeof(short_reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  const uint8_t non_multiple_reg[] = {0, 0, 0, 1, 0, 0, 0, 1, 0};
  get_memory_regions(regions, &count, 1, 1, non_multiple_reg,
                     sizeof(non_multiple_reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_get_memory_regions_rejects_capacity_overflow(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = MAX_MEMORY_REGIONS;
  const uint8_t reg[] = {0, 0, 0, 1, 0, 0, 0, 1};

  get_memory_regions(regions, &count, 1, 1, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Max number of memory regions exceeded",
                           g_last_panic_message);
}

void test_get_memory_regions_rejects_too_many_regions_in_one_reg_blob(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  uint8_t reg[(MAX_MEMORY_REGIONS + 1) * 8];
  memset(reg, 0, sizeof(reg));
  for (size_t i = 0; i < MAX_MEMORY_REGIONS + 1; ++i) {
    reg[i * 8 + 3] = (uint8_t)(i + 1);
    reg[i * 8 + 7] = 1;
  }

  get_memory_regions(regions, &count, 1, 1, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Too many memory regions in get_memory_regions",
                           g_last_panic_message);
}

void test_get_memory_regions_rejects_zero_size_region(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {0x40, 0, 0, 0, 0, 0, 0, 0};

  get_memory_regions(regions, &count, 1, 1, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Memory region reported with zero size",
                           g_last_panic_message);
}

void test_get_memory_regions_rejects_base_plus_size_overflow(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  };

  get_memory_regions(regions, &count, 2, 2, reg, sizeof(reg));

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING(
      "Memory region reported with addresses that overflow 64-bit address space",
      g_last_panic_message);
}

void test_get_memory_regions_rejects_base_above_uintptr(void) {
#if UINTPTR_MAX < UINT64_MAX
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {
      0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x01,
  };
  get_memory_regions(regions, &count, 2, 2, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Memory region base address cannot be cast to uintptr_t",
                           g_last_panic_message);
#else
  TEST_IGNORE_MESSAGE("uintptr_t is 64-bit; cannot construct out-of-range base");
#endif
}

void test_get_memory_regions_rejects_size_above_size_t(void) {
#if SIZE_MAX < UINT64_MAX
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  size_t count = 0;
  const uint8_t reg[] = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00,
  };
  get_memory_regions(regions, &count, 2, 2, reg, sizeof(reg));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING("Memory region size cannot be cast to size_t",
                           g_last_panic_message);
#else
  TEST_IGNORE_MESSAGE("size_t is 64-bit; cannot construct out-of-range size");
#endif
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_get_memory_regions_decodes_single_32bit_region);
  RUN_TEST(test_get_memory_regions_decodes_multiple_64bit_regions);
  RUN_TEST(test_get_memory_regions_appends_to_existing_regions);
  RUN_TEST(test_get_memory_regions_rejects_null_arguments);
  RUN_TEST(test_get_memory_regions_rejects_zero_or_invalid_cell_counts);
  RUN_TEST(test_get_memory_regions_rejects_bad_reg_size_constraints);
  RUN_TEST(test_get_memory_regions_rejects_capacity_overflow);
  RUN_TEST(test_get_memory_regions_rejects_too_many_regions_in_one_reg_blob);
  RUN_TEST(test_get_memory_regions_rejects_zero_size_region);
  RUN_TEST(test_get_memory_regions_rejects_base_plus_size_overflow);
  RUN_TEST(test_get_memory_regions_rejects_base_above_uintptr);
  RUN_TEST(test_get_memory_regions_rejects_size_above_size_t);
  return UNITY_END();
}
