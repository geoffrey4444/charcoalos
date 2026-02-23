// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Hardware/DeviceTree.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define FDT_BEGIN_NODE 0x1u
#define FDT_END_NODE 0x2u
#define FDT_PROP 0x3u
#define FDT_END 0x9u

#define FIXTURE_CAPACITY 4096

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

struct Fixture {
  uint8_t bytes[FIXTURE_CAPACITY];
  uint8_t struct_block[2048];
  uint8_t strings_block[512];
  size_t struct_len;
  size_t strings_len;
  size_t total_size;
};

static void write_be32(uint8_t *out, uint32_t value) {
  out[0] = (uint8_t)((value >> 24) & 0xFFu);
  out[1] = (uint8_t)((value >> 16) & 0xFFu);
  out[2] = (uint8_t)((value >> 8) & 0xFFu);
  out[3] = (uint8_t)(value & 0xFFu);
}

static void write_be64(uint8_t *out, uint64_t value) {
  out[0] = (uint8_t)((value >> 56) & 0xFFu);
  out[1] = (uint8_t)((value >> 48) & 0xFFu);
  out[2] = (uint8_t)((value >> 40) & 0xFFu);
  out[3] = (uint8_t)((value >> 32) & 0xFFu);
  out[4] = (uint8_t)((value >> 24) & 0xFFu);
  out[5] = (uint8_t)((value >> 16) & 0xFFu);
  out[6] = (uint8_t)((value >> 8) & 0xFFu);
  out[7] = (uint8_t)(value & 0xFFu);
}

static void fixture_init(struct Fixture *fixture) {
  memset(fixture, 0, sizeof(*fixture));
}

static uint32_t fixture_add_string(struct Fixture *fixture, const char *text) {
  const size_t offset = fixture->strings_len;
  const size_t len = strlen(text) + 1;
  memcpy(&fixture->strings_block[fixture->strings_len], text, len);
  fixture->strings_len += len;
  return (uint32_t)offset;
}

static void struct_append_u32(struct Fixture *fixture, uint32_t value) {
  write_be32(&fixture->struct_block[fixture->struct_len], value);
  fixture->struct_len += 4;
}

static void struct_append_begin_node(struct Fixture *fixture, const char *name) {
  struct_append_u32(fixture, FDT_BEGIN_NODE);
  const size_t len = strlen(name) + 1;
  memcpy(&fixture->struct_block[fixture->struct_len], name, len);
  fixture->struct_len += len;
  while (fixture->struct_len % 4) {
    fixture->struct_block[fixture->struct_len++] = 0;
  }
}

static void struct_append_end_node(struct Fixture *fixture) {
  struct_append_u32(fixture, FDT_END_NODE);
}

static void struct_append_end(struct Fixture *fixture) {
  struct_append_u32(fixture, FDT_END);
}

static void struct_append_prop(struct Fixture *fixture, uint32_t name_off,
                               const void *data, uint32_t len) {
  struct_append_u32(fixture, FDT_PROP);
  struct_append_u32(fixture, len);
  struct_append_u32(fixture, name_off);
  memcpy(&fixture->struct_block[fixture->struct_len], data, len);
  fixture->struct_len += len;
  while (fixture->struct_len % 4) {
    fixture->struct_block[fixture->struct_len++] = 0;
  }
}

static void struct_append_prop_u32(struct Fixture *fixture, uint32_t name_off,
                                   uint32_t value) {
  uint8_t bytes[4];
  write_be32(bytes, value);
  struct_append_prop(fixture, name_off, bytes, 4);
}

static void fixture_finalize(struct Fixture *fixture, const uint64_t *bases,
                             const uint64_t *sizes, size_t memreserve_count,
                             int include_terminator) {
  const uint32_t header_size = (uint32_t)sizeof(struct DTBHeader);
  const uint32_t off_mem_rsvmap = header_size;
  size_t memreserve_bytes = memreserve_count * 16;
  if (include_terminator) {
    memreserve_bytes += 16;
  }
  const uint32_t off_dt_struct = off_mem_rsvmap + (uint32_t)memreserve_bytes;
  const uint32_t off_dt_strings = off_dt_struct + (uint32_t)fixture->struct_len;
  const uint32_t total_size = off_dt_strings + (uint32_t)fixture->strings_len;

  memset(fixture->bytes, 0, sizeof(fixture->bytes));
  write_be32(&fixture->bytes[0], 0xd00dfeedu);
  write_be32(&fixture->bytes[4], total_size);
  write_be32(&fixture->bytes[8], off_dt_struct);
  write_be32(&fixture->bytes[12], off_dt_strings);
  write_be32(&fixture->bytes[16], off_mem_rsvmap);
  write_be32(&fixture->bytes[20], 17u);
  write_be32(&fixture->bytes[24], 16u);
  write_be32(&fixture->bytes[28], 0u);
  write_be32(&fixture->bytes[32], (uint32_t)fixture->strings_len);
  write_be32(&fixture->bytes[36], (uint32_t)fixture->struct_len);

  size_t cursor = off_mem_rsvmap;
  for (size_t i = 0; i < memreserve_count; ++i) {
    write_be64(&fixture->bytes[cursor], bases[i]);
    write_be64(&fixture->bytes[cursor + 8], sizes[i]);
    cursor += 16;
  }
  if (include_terminator) {
    write_be64(&fixture->bytes[cursor], 0);
    write_be64(&fixture->bytes[cursor + 8], 0);
  }

  memcpy(&fixture->bytes[off_dt_struct], fixture->struct_block,
         fixture->struct_len);
  memcpy(&fixture->bytes[off_dt_strings], fixture->strings_block,
         fixture->strings_len);
  fixture->total_size = total_size;
}

static void build_valid_memory_dtb(struct Fixture *fixture,
                                   int reg_before_device_type,
                                   int include_reserved_subtree,
                                   int include_memreserve) {
  fixture_init(fixture);
  const uint32_t s_addr = fixture_add_string(fixture, "#address-cells");
  const uint32_t s_size = fixture_add_string(fixture, "#size-cells");
  const uint32_t s_device = fixture_add_string(fixture, "device_type");
  const uint32_t s_reg = fixture_add_string(fixture, "reg");

  struct_append_begin_node(fixture, "");
  struct_append_prop_u32(fixture, s_addr, 2);
  struct_append_prop_u32(fixture, s_size, 2);

  struct_append_begin_node(fixture, "memory@40000000");
  const uint8_t reg_mem[] = {
      0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
  };
  if (reg_before_device_type) {
    struct_append_prop(fixture, s_reg, reg_mem, sizeof(reg_mem));
    struct_append_prop(fixture, s_device, "memory\0", 7);
  } else {
    struct_append_prop(fixture, s_device, "memory\0", 7);
    struct_append_prop(fixture, s_reg, reg_mem, sizeof(reg_mem));
  }
  struct_append_end_node(fixture);

  if (include_reserved_subtree) {
    struct_append_begin_node(fixture, "reserved-memory");
    struct_append_prop_u32(fixture, s_addr, 2);
    struct_append_prop_u32(fixture, s_size, 1);

    struct_append_begin_node(fixture, "child@0");
    struct_append_prop_u32(fixture, s_addr, 1);
    struct_append_prop_u32(fixture, s_size, 1);
    const uint8_t reg_reserved[] = {
        0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x00,
    };
    struct_append_prop(fixture, s_reg, reg_reserved, sizeof(reg_reserved));
    struct_append_end_node(fixture);
    struct_append_end_node(fixture);
  }

  struct_append_end_node(fixture);
  struct_append_end(fixture);

  if (include_memreserve) {
    const uint64_t bases[] = {0x0000000040100000ULL};
    const uint64_t sizes[] = {0x0000000000001000ULL};
    fixture_finalize(fixture, bases, sizes, 1, 1);
  } else {
    fixture_finalize(fixture, NULL, NULL, 0, 1);
  }
}

void setUp(void) {
  g_panic_calls = 0;
  g_last_panic_message = NULL;
}

void tearDown(void) {}

void test_parse_device_tree_blob_rejects_null_inputs(void) {
  struct HardwareInfo hw = {0};
  parse_device_tree_blob(NULL, 1u);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  parse_device_tree_blob(&hw, 0u);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_parse_device_tree_blob_rejects_bad_magic(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  fixture.bytes[0] = 0;
  struct HardwareInfo hw = {0};

  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);

  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_parse_device_tree_blob_parses_memory_and_memreserve(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 0, 1);
  struct HardwareInfo hw = {0};

  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, hw.physical_memory_region_count);
  TEST_ASSERT_EQUAL_UINT64(0x40000000ULL,
                           (uint64_t)hw.physical_memory_regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x08000000ULL,
                           (uint64_t)hw.physical_memory_regions[0].size);
  TEST_ASSERT_EQUAL_size_t(1, hw.reserved_memory_regions_count);
  TEST_ASSERT_EQUAL_UINT64(0x40100000ULL,
                           (uint64_t)hw.reserved_memory_regions[0].base_address);
}

void test_parse_device_tree_blob_accepts_reg_before_device_type(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 1, 0, 0);
  struct HardwareInfo hw = {0};

  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, hw.physical_memory_region_count);
}

void test_parse_device_tree_blob_parses_reserved_memory_subtree(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 1, 0);
  struct HardwareInfo hw = {0};

  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);

  TEST_ASSERT_EQUAL_size_t(0, g_panic_calls);
  TEST_ASSERT_EQUAL_size_t(1, hw.reserved_memory_regions_count);
  TEST_ASSERT_EQUAL_UINT64(0x50000000ULL,
                           (uint64_t)hw.reserved_memory_regions[0].base_address);
  TEST_ASSERT_EQUAL_UINT64(0x00100000ULL,
                           (uint64_t)hw.reserved_memory_regions[0].size);
}

void test_parse_device_tree_blob_rejects_header_bounds_violations(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  struct HardwareInfo hw = {0};

  write_be32(&fixture.bytes[16], (uint32_t)fixture.total_size);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  write_be32(&fixture.bytes[8], (uint32_t)(fixture.total_size - 4));
  write_be32(&fixture.bytes[36], 8);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  write_be32(&fixture.bytes[12], (uint32_t)(fixture.total_size - 4));
  write_be32(&fixture.bytes[32], 8);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_parse_device_tree_blob_rejects_unknown_or_bad_tokens(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  struct HardwareInfo hw = {0};

  const uint32_t off_struct =
      ((uint32_t)fixture.bytes[8] << 24) | ((uint32_t)fixture.bytes[9] << 16) |
      ((uint32_t)fixture.bytes[10] << 8) | ((uint32_t)fixture.bytes[11]);
  write_be32(&fixture.bytes[off_struct], 0xA5A5A5A5u);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

  setUp();
  fixture_init(&fixture);
  struct_append_u32(&fixture, FDT_END_NODE);
  struct_append_u32(&fixture, FDT_END);
  fixture_finalize(&fixture, NULL, NULL, 0, 1);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_parse_device_tree_blob_rejects_missing_end_token(void) {
  struct Fixture fixture;
  build_valid_memory_dtb(&fixture, 0, 0, 0);
  struct HardwareInfo hw = {0};

  // Remove final FDT_END token.
  fixture.struct_len -= 4;
  fixture_finalize(&fixture, NULL, NULL, 0, 1);
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);

}

void test_parse_device_tree_blob_rejects_property_name_overflow(void) {
  struct Fixture fixture;
  fixture_init(&fixture);
  const uint32_t s_addr = fixture_add_string(&fixture, "#address-cells");

  struct_append_begin_node(&fixture, "");
  struct_append_prop_u32(&fixture, s_addr, 2);
  struct_append_end_node(&fixture);
  struct_append_end(&fixture);
  fixture_finalize(&fixture, NULL, NULL, 0, 1);

  const uint32_t off_struct =
      ((uint32_t)fixture.bytes[8] << 24) | ((uint32_t)fixture.bytes[9] << 16) |
      ((uint32_t)fixture.bytes[10] << 8) | ((uint32_t)fixture.bytes[11]);
  // First property metadata starts after FDT_BEGIN_NODE + padded empty name.
  const uint32_t prop_name_off_pos = off_struct + 12;
  const uint32_t size_strings =
      ((uint32_t)fixture.bytes[32] << 24) | ((uint32_t)fixture.bytes[33] << 16) |
      ((uint32_t)fixture.bytes[34] << 8) | ((uint32_t)fixture.bytes[35]);
  write_be32(&fixture.bytes[prop_name_off_pos], size_strings);

  struct HardwareInfo hw = {0};
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

void test_parse_device_tree_blob_rejects_excessive_depth(void) {
  struct Fixture fixture;
  fixture_init(&fixture);
  for (size_t i = 0; i < DTB_MAX_DEPTH + 1; ++i) {
    struct_append_begin_node(&fixture, "n");
  }
  struct_append_end(&fixture);
  fixture_finalize(&fixture, NULL, NULL, 0, 1);

  struct HardwareInfo hw = {0};
  parse_device_tree_blob(&hw, (uintptr_t)fixture.bytes);
  TEST_ASSERT_EQUAL_size_t(1, g_panic_calls);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_parse_device_tree_blob_rejects_null_inputs);
  RUN_TEST(test_parse_device_tree_blob_rejects_bad_magic);
  RUN_TEST(test_parse_device_tree_blob_parses_memory_and_memreserve);
  RUN_TEST(test_parse_device_tree_blob_accepts_reg_before_device_type);
  RUN_TEST(test_parse_device_tree_blob_parses_reserved_memory_subtree);
  RUN_TEST(test_parse_device_tree_blob_rejects_header_bounds_violations);
  RUN_TEST(test_parse_device_tree_blob_rejects_unknown_or_bad_tokens);
  RUN_TEST(test_parse_device_tree_blob_rejects_missing_end_token);
  RUN_TEST(test_parse_device_tree_blob_rejects_property_name_overflow);
  RUN_TEST(test_parse_device_tree_blob_rejects_excessive_depth);
  return UNITY_END();
}
