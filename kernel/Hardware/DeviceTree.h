// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "kernel/Memory/Memory.h"

#define DTB_MAX_DEPTH 64

/*!
 * \brief Struct to hold context when parsing the device tree blob (DTB).
 */
struct DTBContext {
  uint32_t address_cells;
  uint32_t size_cells;
  bool in_reserved_memory_subtree;
};

/*!
 * \brief The device tree blob header.
 * \details The header contains information on how to parse the dtb; it is
 * provided as a series of big-endian 32-bit integers at the start of the
 * dtb.
 */
struct DTBHeader {
  uint32_t magic;
  uint32_t total_size;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};

/*!
 * \brief Stores hardware information read at runtime.
 * \details This struct contains information parsed from
 * the device tree blob (dtb), such as available memory regions.
 */
struct HardwareInfo {
  struct DTBHeader header;
  uint32_t address_cells;
  uint32_t size_cells;

  struct MemoryRegion physical_memory_regions[MAX_MEMORY_REGIONS];
  size_t physical_memory_region_count;

  struct MemoryRegion reserved_memory_regions[MAX_MEMORY_REGIONS];
  size_t reserved_memory_regions_count;

};

/*!
 * \brief Parses the device tree blob (dtb) for hardware information.
 * \details The dtb info includes available memory regions, cpu cores, etc.
 * \param out_hw_info Destination for parsed hardware information.
 * \param dtb The device tree blob pointer.
 */
void parse_device_tree_blob(struct HardwareInfo *out_hw_info, uintptr_t dtb);
