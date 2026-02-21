// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Hardware/DeviceTree.h"
#include "kernel/Hardware/Endian.h"
#include "kernel/Panic/Panic.h"

void parse_device_tree_blob(struct hardware_info *out_hw_info, uintptr_t dtb) {
  if (!out_hw_info) {
    kernel_panic("The hardware info output pointer is null");
    return;
  }

  if (!dtb) {
    kernel_panic(
        "The hardware could not be recognized because the device table blob "
        "is null");
    return;
  }

  const uint32_t magic = read_be32_from_address(dtb);
  out_hw_info->header.magic = magic;
  if (magic != 0xd00dfeed) {
    kernel_panic(
        "The hardware could not be recognized because the device table blob "
        "is not in the expected format.");
    return;
  }

  out_hw_info->header.total_size = read_be32_from_address(dtb + 4u);
  // Confirm total_size >= size of header
  if (out_hw_info->header.total_size < sizeof(struct dtb_header)) {
    kernel_panic("dtb table size smaller than expected header size");
    return;
  }

  out_hw_info->header.off_dt_struct = read_be32_from_address(dtb + 8u);
  out_hw_info->header.off_dt_strings = read_be32_from_address(dtb + 12u);
  out_hw_info->header.off_mem_rsvmap = read_be32_from_address(dtb + 16u);
  out_hw_info->header.version = read_be32_from_address(dtb + 20u);
  out_hw_info->header.last_comp_version = read_be32_from_address(dtb + 24u);
  out_hw_info->header.boot_cpuid_phys = read_be32_from_address(dtb + 28u);
  out_hw_info->header.size_dt_strings = read_be32_from_address(dtb + 32u);
  out_hw_info->header.size_dt_struct = read_be32_from_address(dtb + 36u);

  // Device table blob is valid... parse header
  console_print("Device table blob (DTB) recognized with magic ");
  console_print_hex((void *)&magic, 4);
  console_print("\n");
  console_print("  Size:               0x");
  console_print_hex((void *)&(out_hw_info->header.total_size), 4);
  console_print("\n");
  console_print("  Struct offset:      0x");
  console_print_hex((void *)&(out_hw_info->header.off_dt_struct), 4);
  console_print("\n");
  console_print("  Strings offset:     0x");
  console_print_hex((void *)&(out_hw_info->header.off_dt_strings), 4);
  console_print("\n");
  console_print("  Memory rsv map:     0x");
  console_print_hex((void *)&(out_hw_info->header.off_mem_rsvmap), 4);
  console_print("\n");
  console_print("  Version:            0x");
  console_print_hex((void *)&(out_hw_info->header.version), 4);
  console_print("\n");
  console_print("  Last comp version:  0x");
  console_print_hex((void *)&(out_hw_info->header.last_comp_version), 4);
  console_print("\n");
  console_print("  Boot cpuid phys:    0x");
  console_print_hex((void *)&(out_hw_info->header.boot_cpuid_phys), 4);
  console_print("\n");
  console_print("  Strings size:       0x");
  console_print_hex((void *)&(out_hw_info->header.size_dt_strings), 4);
  console_print("\n");
  console_print("  Struct size:        0x");
  console_print_hex((void *)&(out_hw_info->header.size_dt_struct), 4);
  console_print("\n");

  // Begin parsing the device table blob
  // Start with sanity checks
  if (out_hw_info->header.off_mem_rsvmap >= out_hw_info->header.total_size) {
    kernel_panic("Memory reservation map is out of bounds");
    return;
  }
  // Cast to uint64_t so the check works even if it overflows 32-bit arithmetic
  if ((uint64_t)(out_hw_info->header.off_dt_struct) +
          (uint64_t)(out_hw_info->header.size_dt_struct) >
      out_hw_info->header.total_size) {
    kernel_panic("DTB struct offset is out of bounds");
    return;
  }
  if ((uint64_t)(out_hw_info->header.off_dt_strings) +
          (uint64_t)(out_hw_info->header.size_dt_strings) >
      out_hw_info->header.total_size) {
    kernel_panic("DTB string offset is out of bounds");
    return;
  }

  // Parse reserved regions: pairs of 64-bit unsigned integers
  size_t i = 0;
  uint64_t start, end;
  out_hw_info->reserved_regions_count = 0;
  while (true) {
    if (i >= 255) {
      kernel_panic("Insufficient space to store reserved memory regions");
      return;
    }
    if ((uint64_t)(out_hw_info->header.off_mem_rsvmap) +
            (uint64_t)(16 * i + 16) >
        (uint64_t)(out_hw_info->header.total_size)) {
      kernel_panic("Reserved memory region would overflow header");
      return;
    }
    start = dtb + out_hw_info->header.off_mem_rsvmap + 16 * i;
    end = start + 8;
    if (end >= dtb + out_hw_info->header.total_size) {
      break;
    }
    out_hw_info->reserved_region_base_addresses[i] =
        read_be64_from_address(start);
    out_hw_info->reserved_region_sizes[i] = read_be64_from_address(end);
    if ((out_hw_info->reserved_region_base_addresses[i] == 0) &&
        (out_hw_info->reserved_region_sizes[i] == 0)) {
      // End of table of reserved regions reached
      break;
    } else {
      ++(out_hw_info->reserved_regions_count);
      console_print("Reserved memory region at base address 0x");
      console_print_hex(
          (void *)&(out_hw_info->reserved_region_base_addresses[i]), 8);
      console_print(" of size 0x");
      console_print_hex((void *)&(out_hw_info->reserved_region_sizes[i]), 8);
      console_print("\n");
    }
    ++i;
  }
}
