// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>
#include <stddef.h>

/*!
 * \brief A struct that holes a memory region, with a base address and size.
 */
struct MemoryRegion {
  uintptr_t base_address;
  size_t size;
};

/*!
 * \brief Determines physical memory regions from information extracted
 * from the device tree blob (DTB).
 * \param out_memory_regions Returns the physical memory regions.
 * \param out_memory_region_count Returns the number of physical memory regions.
 * \param address_cells The number of 32-bit cells specifying address values
 * in the reg data from the DTB.
 * \param size_cells The number of 32-bit cells specifying size values in the 
 * reg data from the DTB.
 * \param reg_bytes The raw bytes
 */
void physical_memory_regions(struct MemoryRegion* out_memory_regions,
                             size_t* out_memory_region_count,
                             const uint32_t address_cells,
                             const uint32_t size_cells, const void* reg_bytes,
                             const size_t reg_size);
