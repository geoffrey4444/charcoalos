// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Hardware/Endian.h"
#include "kernel/Memory/Memory.h"
#include "kernel/Panic/Panic.h"

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE_BYTES 4096

void get_memory_regions(struct MemoryRegion* out_memory_regions,
                        size_t* out_memory_region_count,
                        const uint32_t address_cells, const uint32_t size_cells,
                        const void* reg_bytes, const size_t reg_size) {
  if (out_memory_regions == NULL || out_memory_region_count == NULL ||
      reg_bytes == NULL) {
    kernel_panic("Null pointer passed to get_memory_regions");
    return;
  }
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
    console_print_hex_value((void*)&reg_size, 8);
    console_print("\nsize_of_one_region_spec: 0x");
    console_print_hex_value((void*)&size_of_one_region_spec, 8);
    console_print("\n");
    kernel_panic(
        "Unable to determine memory: reg size is not an integer "
        "multiple of region sizes");
    return;
  }
  if (number_of_regions_specified > MAX_MEMORY_REGIONS) {
    kernel_panic("Too many memory regions in get_memory_regions");
    return;
  }
  if (*out_memory_region_count >
      (MAX_MEMORY_REGIONS - number_of_regions_specified)) {
    kernel_panic("Max number of memory regions exceeded");
    return;
  }

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
      kernel_panic("Memory region reported with zero size");
      return;
    }
    // Do not accept if base + size > UINT64_MAX
    // But write as UINT64_MAX - size < base to avoid overflow
    if (UINT64_MAX - size < base) {
      kernel_panic(
          "Memory region reported with addresses that overflow 64-bit "
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
    out_memory_regions[*out_memory_region_count + i].base_address =
        (uintptr_t)(base);
    out_memory_regions[*out_memory_region_count + i].size = (size_t)(size);
  }
  *out_memory_region_count += (size_t)number_of_regions_specified;
}

void normalize_memory_regions(struct MemoryRegion* out_memory_regions,
                              size_t* out_memory_region_count,
                              const bool expand_to_page_size) {
  // If any pointer passed in is NULL, panic
  if (out_memory_regions == NULL || out_memory_region_count == NULL) {
    kernel_panic("Null pointer passed to normalize_memory_regions");
    return;
  }

  // If the array is empty, do nothing
  if (*out_memory_region_count == 0) {
    return;
  }

  // Loop over regions, removing any regions of zero size. If a region is
  // removed, all regions at higher index are shifted left by one
  // (i.e., their indices decrease by one), and out_memory_region_count
  // decreases by one.
  {
    size_t i = 0;
    while (i < *out_memory_region_count) {
      if (out_memory_regions[i].size == 0) {
        // Shift remaining elements left 1
        for (size_t j = i + 1; j < *out_memory_region_count; ++j) {
          out_memory_regions[j - 1] = out_memory_regions[j];
        }
        --(*out_memory_region_count);
        // If no regions remain, return ... array contained only zero-size
        // regions
        if (*out_memory_region_count == 0) {
          return;
        }
        // Need to check region again, since the ith region is different now.
        // Don't increment i
        continue;
      }
      ++i;
    }
  }

  // Reduce base address and increase size, or increase base address and reduce
  // size, so that start and end == start + size
  // are multiples of PAGE_SIZE_BYTES
  for (size_t i = 0; i < *out_memory_region_count; ++i) {
    const size_t start_mod_page_size =
        out_memory_regions[i].base_address % PAGE_SIZE_BYTES;
    // Before adding base + size, check for overflow: base + size > SIZE_MAX,
    // but to avoid overflow, write as base > SIZE_MAX - size
    if (out_memory_regions[i].base_address >
        (UINTPTR_MAX - out_memory_regions[i].size)) {
      kernel_panic(
          "Overflow when adding base + size when normalizing memory regions");
      return;
    }
    const size_t end_mod_page_size =
        (out_memory_regions[i].base_address + out_memory_regions[i].size) %
        PAGE_SIZE_BYTES;
    if (start_mod_page_size) {
      if (expand_to_page_size) {
        out_memory_regions[i].base_address -= start_mod_page_size;
        // No need to check for underflow above, because mod can't underflow
        // But check for overflow here
        if (out_memory_regions[i].size > SIZE_MAX - start_mod_page_size) {
          kernel_panic(
              "Overflow when increasing memory region size in "
              "normalize_memory_regions");
          return;
        }
        out_memory_regions[i].size += start_mod_page_size;
      } else {
        if (out_memory_regions[i].base_address >
            UINTPTR_MAX - (PAGE_SIZE_BYTES - start_mod_page_size)) {
          kernel_panic(
              "Overflow when contracting memory region in "
              "normalize_memory_regions");
          return;
        }
        out_memory_regions[i].base_address +=
            (PAGE_SIZE_BYTES - start_mod_page_size);
        if (out_memory_regions[i].size >=
            (PAGE_SIZE_BYTES - start_mod_page_size)) {
          out_memory_regions[i].size -= (PAGE_SIZE_BYTES - start_mod_page_size);
        } else {
          out_memory_regions[i].size = 0;
        }
      }
    }
    if (end_mod_page_size) {
      if (expand_to_page_size) {
        if (out_memory_regions[i].size >
            SIZE_MAX - (PAGE_SIZE_BYTES - end_mod_page_size)) {
          kernel_panic("Overflow adjusting end in normalize_memory_regions");
          return;
        }
        out_memory_regions[i].size += (PAGE_SIZE_BYTES - end_mod_page_size);
      } else {
        if (out_memory_regions[i].size >= end_mod_page_size) {
          out_memory_regions[i].size -= end_mod_page_size;
        } else {
          out_memory_regions[i].size = 0;
        }
      }
    }
  }

  // Once again, remove empty regions
  // Loop over regions, removing any regions of zero size. If a region is
  // removed, all regions at higher index are shifted left by one
  // (i.e., their indices decrease by one), and out_memory_region_count
  // decreases by one.
  {
    size_t i = 0;
    while (i < *out_memory_region_count) {
      if (out_memory_regions[i].size == 0) {
        // Shift remaining elements left 1
        for (size_t j = i + 1; j < *out_memory_region_count; ++j) {
          out_memory_regions[j - 1] = out_memory_regions[j];
        }
        --(*out_memory_region_count);
        // If no regions remain, return ... array contained only zero-size
        // regions
        if (*out_memory_region_count == 0) {
          return;
        }
        // Need to check region again, since the ith region is different now.
        // Don't increment i
        continue;
      }
      ++i;
    }
  }

  // Sort the regions in place using insertion sort (chosen because
  // the number of regions tends to be small... this could be changed
  // for a different sorting algorithm later if needed).
  // I learned this particular sort algorithm from
  // https://en.wikipedia.org/wiki/Insertion_sort
  struct MemoryRegion swap;
  swap.base_address = 0;
  swap.size = 0;

  for (size_t i = 1; i < *out_memory_region_count; ++i) {
    for (size_t j = i; j > 0 && out_memory_regions[j - 1].base_address >
                                    out_memory_regions[j].base_address;
         --j) {
      // Swap element j and element j - 1
      swap = out_memory_regions[j - 1];
      out_memory_regions[j - 1] = out_memory_regions[j];
      out_memory_regions[j] = swap;
    }
  }

  // Check if the sorted regions overlap. For each region except the last,
  // check if base + size extends beyond next region's base address. If it does,
  // extend the size to reach to the end of the next region, and remove the
  // next region, left-shifting regions beyond that by one.
  {
    size_t i = 0;
    while (i < (*out_memory_region_count - 1)) {
      uintptr_t start_i = out_memory_regions[i].base_address;
      if (out_memory_regions[i].base_address >
          UINTPTR_MAX - out_memory_regions[i].size) {
        kernel_panic(
            "ith memory region overflow when adding base + size in "
            "normalize_memory_regions");
        return;
      }
      uintptr_t end_i = start_i + out_memory_regions[i].size;
      uintptr_t start_i_plus_1 = out_memory_regions[i + 1].base_address;
      if (out_memory_regions[i + 1].base_address >
          UINTPTR_MAX - out_memory_regions[i + 1].size) {
        kernel_panic("i+1th region overflow in normalize_memory_regions");
        return;
      }
      uintptr_t end_i_plus_1 = start_i_plus_1 + out_memory_regions[i + 1].size;
      if (end_i >= start_i_plus_1) {
        // Merge the regions: end is now max(end[i], end[i+1])
        if (end_i_plus_1 > end_i) {
          out_memory_regions[i].size = (size_t)(end_i_plus_1 - start_i);
        }

        // Remove i+1 while preserving i (already merged).
        for (size_t j = i + 2; j < *out_memory_region_count; ++j) {
          out_memory_regions[j - 1] = out_memory_regions[j];
        }
        --(*out_memory_region_count);
        // The "ith" element is now different than before, becaue of the merge
        // So need to check case i again. Don't increment i, just go to next
        // iteration in the loop.
        continue;
      }
      ++i;
    }
  }
}

void remove_reserved_memory_regions(
    struct MemoryRegion* physical_memory_regions,
    size_t* physical_memory_region_count,
    struct MemoryRegion* reserved_memory_regions,
    size_t* reserved_memory_region_count) {
  if (physical_memory_regions == NULL || reserved_memory_regions == NULL ||
      physical_memory_region_count == NULL ||
      reserved_memory_region_count == NULL) {
    kernel_panic("Null pointer passed to remove_reserved_memory_regions");
    return;
  }

  if (*physical_memory_region_count == 0) {
    // There are no physical regions to normalize, so just return;
    return;
  }
  if (*reserved_memory_region_count == 0) {
    // There are no reserved regions to normalize, so just return;
    return;
  }

  normalize_memory_regions(physical_memory_regions,
                           physical_memory_region_count, false);
  normalize_memory_regions(reserved_memory_regions,
                           reserved_memory_region_count, true);

  for (size_t i = 0; i < *physical_memory_region_count; ++i) {
    for (size_t j = 0; j < *reserved_memory_region_count; ++j) {
      uintptr_t phys_start = physical_memory_regions[i].base_address;
      if (phys_start > UINTPTR_MAX - physical_memory_regions[i].size) {
        kernel_panic(
            "Overflow in phys region before removing reserved regions");
        return;
      }
      uintptr_t phys_end = phys_start + physical_memory_regions[i].size;
      uintptr_t res_start = reserved_memory_regions[j].base_address;
      if (res_start > UINTPTR_MAX - reserved_memory_regions[j].size) {
        kernel_panic(
            "Overflow in reserved region before removing reserved regions");
        return;
      }
      uintptr_t res_end = res_start + reserved_memory_regions[j].size;
      if (phys_start >= res_start && phys_start < res_end &&
          phys_end <= res_end && phys_end > res_start) {
        // Reserved region completely encloses physical region,
        // so drop the physical region by setting its size to 0;
        // it will be removed when normalizing
        physical_memory_regions[i].size = 0;
        // No need to continue checking against reserved regions; this
        // physical region has been eliminated
        break;
      } else if (phys_start >= res_start && phys_start < res_end) {
        // phys_end > res_end to get here
        // Trim start: phys_start -> res_end
        physical_memory_regions[i].base_address = res_end;
        physical_memory_regions[i].size = phys_end - res_end;
      } else if (phys_end <= res_end && phys_end > res_start) {
        // res_start > phys_start to get here
        // Trim end: phys_end -> res_start
        physical_memory_regions[i].size = res_start - phys_start;
      } else if (phys_start < res_start && phys_end > res_end) {
        // Split
        if (*physical_memory_region_count + 1 > MAX_MEMORY_REGIONS) {
          kernel_panic(
              "Cannot split memory region to account for reserved region: all "
              "memory regions already used");
          return;
        }
        // Shift existing physical regions right 1
        for (size_t k = *physical_memory_region_count - 1; k > i; --k) {
          physical_memory_regions[k + 1] = physical_memory_regions[k];
        }
        ++(*physical_memory_region_count);
        // First region: adjust end
        physical_memory_regions[i].size = res_start - phys_start;
        // Second region: adjust start
        physical_memory_regions[i + 1].base_address = res_end;
        physical_memory_regions[i + 1].size = phys_end - res_end;
      }
    }
  }

  normalize_memory_regions(physical_memory_regions,
                           physical_memory_region_count, false);
}

size_t get_total_memory_size(const struct MemoryRegion* memory_regions,
                             const size_t memory_region_count) {
  if (memory_region_count == 0) {
    return 0;
  }
  if (memory_regions == NULL) {
    kernel_panic("Null pointer passed to get_total_memory_size");
    return 0;
  }

  size_t total_memory = 0;
  for (size_t i = 0; i < memory_region_count; ++i) {
    if (memory_regions[i].size > SIZE_MAX - total_memory) {
      kernel_panic("Overflow in adding memory sizes");
      return 0;
    } else {
      total_memory += memory_regions[i].size;
    }
  }
  return total_memory;
}
