// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Hardware/Endian.h"
#include "kernel/Memory/Memory.h"
#include "kernel/Panic/Panic.h"

#include <stdint.h>
#include <stddef.h>

void physical_memory_regions(struct MemoryRegion* out_memory_regions,
                             size_t* out_memory_region_count,
                             const uint32_t address_cells,
                             const uint32_t size_cells, const void* reg_bytes,
                             const size_t reg_size) {
  // Sanity checks on sizes
  if ((address_cells > reg_size) || (size_cells > reg_size)) {
    kernel_panic(
        "Unable to determine physical memory regions: address_cells or "
        "size_cells are larger than reg size");
    return;
  }
  if ((address_cells < 1) || (size_cells < 1) || (reg_size < 1)) {
    kernel_panic(
        "Unable to determin physical memory regions: address_cells, "
        "size_cells, and reg_size must be greater than 0.");
    return;
  }
  // Only support 1 or 2 cells (32-bit or 64-bit)
  if ((address_cells > 2) || (size_cells > 2)) {
    kernel_panic(
        "Unable to determin physical memory regions: address_cells"
        " and size_cells must be less than 3.");
    return;
  }

  // Size of one region specification: (address_cells + size_cells) * 4 bytes
  const uint64_t size_of_one_region_spec =
      4u * (((uint64_t)address_cells) + ((uint64_t)size_cells));
  const uint64_t number_of_regions_specified =
      reg_size / size_of_one_region_spec;

  // Make sure reg_size is an integer multiple of size_of_one_region_spec
  if (reg_size % size_of_one_region_spec) {
    console_print("reg_size: 0x");
    console_print_hex_value((void *)&reg_size, 8);
    console_print("\nsize_of_one_region_spec: 0x");
    console_print_hex_value((void *)&size_of_one_region_spec, 8);
    console_print("\n");
    kernel_panic(
        "Unable to determine physical memory: reg size is not an integer "
        "multiple of region sizes");
    return;
  }
  if (number_of_regions_specified > 16) {
    kernel_panic("Only up to 16 regions of physical memory supported");
    return;
  }
  // Safe to cast: 16 way less than SIZE_MAX
  *out_memory_region_count = (size_t)number_of_regions_specified;

  // Start decoding reg bytes
  uint64_t base;
  uint64_t size;
  uintptr_t reg_cursor = (uintptr_t)reg_bytes;
  for (size_t i = 0; i < number_of_regions_specified; ++i) {
    // Read address
    if (address_cells == 2) {
      base = read_be64_from_address(reg_cursor);
      reg_cursor += 8u;
    } else {
      // Already verified address_cells and size_cells are either 1 or 2
      base = (uint64_t)read_be32_from_address(reg_cursor);
      reg_cursor += 4u;
    }

    // Read size
    if (size_cells == 2) {
      size = read_be64_from_address(reg_cursor);
      reg_cursor += 8u;
    } else {
      // Already verified address_cells and size_cells are either 1 or 2
      size = (uint64_t)read_be32_from_address(reg_cursor);
      reg_cursor += 4u;
    }

    // Do not accept zero size for a region; that makes no sense
    if (size == 0) {
      kernel_panic("Physical memory region reported with zero size");
      return;
    }
    // Do not accept if base + size > UINT64_MAX
    // But write as UINT64_MAX - size < base to avoid overflow
    if (UINT64_MAX - size < base) {
      kernel_panic(
          "Physical memory region reported with addresses that overflow 64-bit "
          "address space");
      return;
    }
    // Ensure I can cast to uintptr_t and size_t
    if (base > UINTPTR_MAX) {
      kernel_panic("Memory region base address cannot be cast to uintptr_t");
      return;
    }
    if (size > SIZE_MAX) {
      kernel_panic("Memory region size cannot be cast to size_t");
      return;
    }
    out_memory_regions[i].base_address = (uintptr_t)(base);
    out_memory_regions[i].size = (size_t)(size);
  }
}
