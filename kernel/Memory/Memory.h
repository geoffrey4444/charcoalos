// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*!
 * \brief Max number of regions that can hold physical memory or reserved
 * memory.
 * \details This sets the size of the arrays used to store physical and reserved
 * memory regions in HardwareInfo.
 */
#define MAX_MEMORY_REGIONS 64

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
void get_memory_regions(struct MemoryRegion* out_memory_regions,
                        size_t* out_memory_region_count,
                        const uint32_t address_cells, const uint32_t size_cells,
                        const void* reg_bytes, const size_t reg_size);

/*!
 * \brief Normalizes the memory regions.
 * \details This function does the following to an array of memory regions:
 * - Sorts the regions by base address
 * - Merges overlapping regions into a single region
 * - Removes regions with zero size
 * - Expands regions to align to 4KB (4096 bytes) boundaries
 * \param out_memory_regions The memory regions to normalize (modified in place)
 * \param out_memory_region_count The number of regions (modified in place)
 * \param expand_to_page_size If true, expand region to align with page
 * boundaries. If false, contract region to align with page boundaries.
 * Expanding makes sense for reserved memory regions (it ensures the entire
 * reserved region is still reserved after normalization), while contracting
 * makes sense for physical memory regions (it ensure the normalized
 * physical memory regions are contained within the physical memory that
 * exists.)
 */
void normalize_memory_regions(struct MemoryRegion* out_memory_regions,
                              size_t* out_memory_region_count,
                              const bool expand_to_page_size);
