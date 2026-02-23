// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Memory/Memory.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>

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

void test_normalize_memory_regions_handles_null_and_empty_inputs(void) {
  size_t count = 0;
  normalize_memory_regions(NULL, &count, false);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {0};
  normalize_memory_regions(regions, NULL, false);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  count = 0;
  normalize_memory_regions(regions, &count, false);
  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(0, count);
}

void test_normalize_memory_regions_removes_zero_entries_and_sorts(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x5000, .size = 0},
      {.base_address = 0x3000, .size = 0x1000},
      {.base_address = 0x1000, .size = 0},
      {.base_address = 0x2000, .size = 0x1000},
  };
  size_t count = 4;

  normalize_memory_regions(regions, &count, false);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, count);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)regions[0].size);
}

void test_normalize_memory_regions_contracts_to_page_boundaries(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1003, .size = 0x2FFD},
  };
  size_t count = 1;

  normalize_memory_regions(regions, &count, false);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, count);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)regions[0].size);
}

void test_normalize_memory_regions_expands_to_page_boundaries(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1003, .size = 0x2FFD},
  };
  size_t count = 1;

  normalize_memory_regions(regions, &count, true);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, count);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x3000ULL, (uint64_t)regions[0].size);
}

void test_normalize_memory_regions_contract_can_drop_entire_range(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1001, .size = 1},
  };
  size_t count = 1;

  normalize_memory_regions(regions, &count, false);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(0, count);
}

void test_normalize_memory_regions_merges_overlapping_and_adjacent_ranges(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x4000, .size = 0x1000},
      {.base_address = 0x1000, .size = 0x2000},
      {.base_address = 0x3000, .size = 0x1000},
      {.base_address = 0x5000, .size = 0x1000},
  };
  size_t count = 4;

  normalize_memory_regions(regions, &count, false);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, count);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x5000ULL, (uint64_t)regions[0].size);
}

void test_remove_reserved_memory_regions_handles_null_and_empty_inputs(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {0};
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {0};
  size_t pcount = 0;
  size_t rcount = 0;

  remove_reserved_memory_regions(NULL, &pcount, reserved, &rcount);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  remove_reserved_memory_regions(physical, NULL, reserved, &rcount);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);
  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
}

void test_remove_reserved_memory_regions_no_overlap_keeps_physical(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1000, .size = 0x3000},
  };
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x8000, .size = 0x1000},
  };
  size_t pcount = 1;
  size_t rcount = 1;

  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, pcount);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)physical[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x3000ULL, (uint64_t)physical[0].size);
}

void test_remove_reserved_memory_regions_handles_full_cover_and_trims(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1000, .size = 0x7000},
      {.base_address = 0x9000, .size = 0x1000},
  };
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x2000, .size = 0x1000},
      {.base_address = 0x9000, .size = 0x1000},
  };
  size_t pcount = 2;
  size_t rcount = 2;

  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(2, pcount);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)physical[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)physical[0].size);
  TEST_ASSERT_EQUAL_UINT64(0x3000ULL, (uint64_t)physical[1].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x5000ULL, (uint64_t)physical[1].size);
}

void test_remove_reserved_memory_regions_split_case_creates_two_regions(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1000, .size = 0x9000},
  };
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x3000, .size = 0x2000},
  };
  size_t pcount = 1;
  size_t rcount = 1;

  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(2, pcount);
  TEST_ASSERT_EQUAL_UINT64(0x1000ULL, (uint64_t)physical[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x2000ULL, (uint64_t)physical[0].size);
  TEST_ASSERT_EQUAL_UINT64(0x5000ULL, (uint64_t)physical[1].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x5000ULL, (uint64_t)physical[1].size);
}

void test_remove_reserved_memory_regions_split_fails_when_at_capacity(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {0};
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x3000, .size = 0x2000},
  };
  for (size_t i = 0; i < MAX_MEMORY_REGIONS; ++i) {
    physical[i].base_address = 0x1000 + i * 0x10000;
    physical[i].size = 0x4000;
  }
  physical[0].base_address = 0x1000;
  physical[0].size = 0x9000;

  size_t pcount = MAX_MEMORY_REGIONS;
  size_t rcount = 1;

  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
  TEST_ASSERT_EQUAL_STRING(
      "Cannot split memory region to account for reserved region: all memory regions already used",
      g_last_panic_message);
}

void test_remove_reserved_memory_regions_catches_overflow_inputs(void) {
  struct MemoryRegion physical[MAX_MEMORY_REGIONS] = {
      {.base_address = UINTPTR_MAX, .size = 1},
  };
  struct MemoryRegion reserved[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1000, .size = 0x1000},
  };
  size_t pcount = 1;
  size_t rcount = 1;

  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);
  TEST_ASSERT_TRUE(g_panic_calls >= 1);

  setUp();
  physical[0].base_address = 0x1000;
  physical[0].size = 0x2000;
  reserved[0].base_address = UINTPTR_MAX;
  reserved[0].size = 1;
  remove_reserved_memory_regions(physical, &pcount, reserved, &rcount);
  TEST_ASSERT_TRUE(g_panic_calls >= 1);
}

void test_get_total_memory_size_behaviors(void) {
  struct MemoryRegion regions[MAX_MEMORY_REGIONS] = {
      {.base_address = 0x1000, .size = 0x10},
      {.base_address = 0x2000, .size = 0x20},
  };

  TEST_ASSERT_EQUAL_size_t(0, get_total_memory_size(regions, 0));
  TEST_ASSERT_EQUAL_size_t(0x30, get_total_memory_size(regions, 2));

  setUp();
  TEST_ASSERT_EQUAL_size_t(0, get_total_memory_size(NULL, 1));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  regions[0].size = SIZE_MAX;
  regions[1].size = 1;
  TEST_ASSERT_EQUAL_size_t(0, get_total_memory_size(regions, 2));
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_normalize_memory_regions_handles_null_and_empty_inputs);
  RUN_TEST(test_normalize_memory_regions_removes_zero_entries_and_sorts);
  RUN_TEST(test_normalize_memory_regions_contracts_to_page_boundaries);
  RUN_TEST(test_normalize_memory_regions_expands_to_page_boundaries);
  RUN_TEST(test_normalize_memory_regions_contract_can_drop_entire_range);
  RUN_TEST(
      test_normalize_memory_regions_merges_overlapping_and_adjacent_ranges);
  RUN_TEST(test_remove_reserved_memory_regions_handles_null_and_empty_inputs);
  RUN_TEST(test_remove_reserved_memory_regions_no_overlap_keeps_physical);
  RUN_TEST(test_remove_reserved_memory_regions_handles_full_cover_and_trims);
  RUN_TEST(test_remove_reserved_memory_regions_split_case_creates_two_regions);
  RUN_TEST(test_remove_reserved_memory_regions_split_fails_when_at_capacity);
  RUN_TEST(test_remove_reserved_memory_regions_catches_overflow_inputs);
  RUN_TEST(test_get_total_memory_size_behaviors);
  return UNITY_END();
}
