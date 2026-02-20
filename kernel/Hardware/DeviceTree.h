// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief The device tree blob header.
 * \details The header contains information on how to parse the dtb; it is
 * provided as a series of big-endian 32-bit integers at the start of the 
 * dtb. `__attribute((packed))` prevents the compiler from adding padding
 * to the struct entries.
 */
struct dtb_header {
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
struct hardware_info {
  struct dtb_header header;
};

/*!
 * \brief Parses the device tree blob (dtb) for hardware information.
 * \details The dtb info includes available memory regions, cpu cores, etc.
 * \param out_hw_info Destination for parsed hardware information.
 * \param dtb The device tree blob pointer.
 */
void parse_device_tree_blob(struct hardware_info *out_hw_info, uintptr_t dtb);
