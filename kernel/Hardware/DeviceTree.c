// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Hardware/DeviceTree.h"
#include "kernel/Hardware/Endian.h"
#include "kernel/Memory/Memory.h"
#include "kernel/Panic/Panic.h"
#include "kernel/String/String.h"

// Bytes I can encounter when parsing the dtb
#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE 0x2
#define FDT_PROP 0x3
#define FDT_NOP 0x4
#define FDT_END 0x9

void parse_device_tree_blob(struct HardwareInfo *out_hw_info, uintptr_t dtb) {
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
  if (out_hw_info->header.total_size < sizeof(struct DTBHeader)) {
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
  console_print_hex_value((void *)&magic, 4);
  console_print("\n");
  console_print("  Size:               0x");
  console_print_hex_value((void *)&(out_hw_info->header.total_size), 4);
  console_print("\n");
  console_print("  Struct offset:      0x");
  console_print_hex_value((void *)&(out_hw_info->header.off_dt_struct), 4);
  console_print("\n");
  console_print("  Strings offset:     0x");
  console_print_hex_value((void *)&(out_hw_info->header.off_dt_strings), 4);
  console_print("\n");
  console_print("  Memory rsv map:     0x");
  console_print_hex_value((void *)&(out_hw_info->header.off_mem_rsvmap), 4);
  console_print("\n");
  console_print("  Version:            0x");
  console_print_hex_value((void *)&(out_hw_info->header.version), 4);
  console_print("\n");
  console_print("  Last comp version:  0x");
  console_print_hex_value((void *)&(out_hw_info->header.last_comp_version), 4);
  console_print("\n");
  console_print("  Boot cpuid phys:    0x");
  console_print_hex_value((void *)&(out_hw_info->header.boot_cpuid_phys), 4);
  console_print("\n");
  console_print("  Strings size:       0x");
  console_print_hex_value((void *)&(out_hw_info->header.size_dt_strings), 4);
  console_print("\n");
  console_print("  Struct size:        0x");
  console_print_hex_value((void *)&(out_hw_info->header.size_dt_struct), 4);
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
      console_print_hex_value(
          (void *)&(out_hw_info->reserved_region_base_addresses[i]), 8);
      console_print(" of size 0x");
      console_print_hex_value((void *)&(out_hw_info->reserved_region_sizes[i]),
                              8);
      console_print("\n");
    }
    ++i;
  }
  console_print("Read 0x");
  console_print_hex_value((void *)&(out_hw_info->reserved_regions_count), 8);
  console_print(" reserved memory regions.\n");

  // Walk the table
  bool address_cells_initialized = false;
  bool size_cells_initialized = false;
  void *current_node_reg = NULL;
  uint32_t current_node_reg_size = 0;
  char *current_node_device_type = NULL;

  char *node_name = "";
  size_t node_name_length = 0;
  uint32_t prop_length = 0;
  uint32_t prop_name_offset = 0;
  void *current_data_bytes = NULL;
  uint64_t node_depth = 0;
  uint64_t end_tokens_encountered = 0;
  uintptr_t struct_end = (uintptr_t)(dtb) +
                         (uintptr_t)(out_hw_info->header.off_dt_struct) +
                         (uintptr_t)(out_hw_info->header.size_dt_struct);
  uintptr_t dtb_cursor =
      (uintptr_t)(dtb) + (uintptr_t)(out_hw_info->header.off_dt_struct);
  // Loop while cursor is 4 or more bytes from end of struct block
  // Guard against overflow
  while (true) {
    // Bounds check: if cursor out of bounds, panic
    if (dtb_cursor > struct_end) {
      kernel_panic("Malformed DTB: cursor out of bounds");
      return;
    }
    // End loop if not at least 4 bytes from struct end
    if (struct_end - dtb_cursor < 4) {
      break;
    }

    // Read token, advance cursor, and check not out of bounds
    uint32_t token = read_be32_from_address(dtb_cursor);
    dtb_cursor += 4;  // advance cursor past uint32_t token
    if (dtb_cursor > struct_end) {
      kernel_panic("Malformed DTB: token overflow");
      return;
    }

    // Handle the token
    switch (token) {
      case FDT_BEGIN_NODE:
        ++node_depth;
        // Get node name
        node_name = (char *)(dtb_cursor);
        // Advance cursor beyond node name string's null terminator, guarding
        // against overflow
        node_name_length = 0;
        if (dtb_cursor == struct_end) {
          kernel_panic("Malformed DTB: cannot parse node name");
          return;
        }
        while (*(char *)(dtb_cursor) != '\0') {
          if (dtb_cursor + 1 > struct_end) {
            kernel_panic("Malformed DTB: immediate node name overflow");
            return;
          }
          ++node_name_length;
          ++dtb_cursor;
          // Before going back and doing the loop check, guard against overflow
          if (dtb_cursor == struct_end) {
            kernel_panic("Malformed DTB: node name overflow");
            return;
          }
        }
        ++dtb_cursor;  // advance past null terminator
        // Align to 4-bytes
        while (dtb_cursor % 4) {
          ++dtb_cursor;
        }
        if (dtb_cursor > struct_end) {
          kernel_panic("Malformed DTB: token overflow after align");
          return;
        }

        // Set current_node_device_type and current_node_reg to NULL; these
        // have not yet been read for this new element
        current_node_device_type = NULL;
        current_node_reg = NULL;

        // Print info to console
        // console_print("Read DTB node: ");
        // console_print(node_name);
        // console_print("\n");

        break;
      case FDT_END_NODE:
        if (node_depth == 0) {
          kernel_panic(
              "Malformed DTB: end node token without matching begin node "
              "token");
          return;
        }
        --node_depth;
        break;
      case FDT_PROP:
        // Before proceeding, ensure that reading property length and
        // name offset will not overflow. Use
        // struct_end - dtb_cursor < 8 instead of dtb_cursor + 8 > struct_end
        // to avoid dtb_cursor +8 overflowing uintptr_t.
        if (struct_end - dtb_cursor < 8) {
          kernel_panic("Malformed DTB: property metadata overflow");
          return;
        }
        prop_length = read_be32_from_address(dtb_cursor);
        dtb_cursor += 4;
        prop_name_offset = read_be32_from_address(dtb_cursor);
        dtb_cursor += 4;
        current_data_bytes = (void *)(dtb_cursor);
        // Guard against overflow. dtb_cursor + prop_length > struct_end is
        // trouble. But check this way to also catch case where
        // prop_length + dtb_cursor overflows
        if (prop_length > struct_end - dtb_cursor) {
          kernel_panic("Malformed DTB: property length overflow");
          return;
        }
        dtb_cursor += prop_length;
        // Align to 4-byte boundary
        while (dtb_cursor % 4) {
          ++dtb_cursor;
        }
        // Guard against overflow, since we're readying a number of bytes
        // determined at runtime
        if (dtb_cursor > struct_end) {
          kernel_panic("Malformed DTB: property value overflow");
          return;
        }
        // Guard against string table overflow
        if (prop_name_offset >= out_hw_info->header.size_dt_strings) {
          kernel_panic("Malformed DTB: property name overflow");
          return;
        }
        const char *prop_name =
            (char *)(dtb + out_hw_info->header.off_dt_strings +
                     prop_name_offset);

        // Print out identified property name and its size
        // console_print("  Property read: ");
        // console_print(prop_name);
        // console_print(" -- size 0x");
        // console_print_hex_value((void *)&prop_length, 4);
        // console_print("\n");
        // console_print("    Value (hex bytes): ");
        // console_print_hex_bytes(current_data_bytes, prop_length);
        // console_print("\n");

        // Check if property should be saved in out_hw_info
        // e.g. for use to initialize memory
        // Save #adress-cells and #size-cells from root note; these
        // will decode the info in the dtb about available physical memory.
        // NOTE: this assumes memory is a child of the root dtb node, as is
        // the case for qemu-virt arm64 and rpi arm64.
        if (string_compare(node_name, "")) {
          if (string_compare(prop_name, "#address-cells")) {
            // First, confirm that the value is a 32-bit integer
            if (prop_length != 4) {
              kernel_panic("Malformed DTB: #address-cells value is not 32-bit");
              return;
            }
            address_cells_initialized = true;
            out_hw_info->address_cells =
                read_be32_from_address((uintptr_t)current_data_bytes);
          } else if (string_compare(prop_name, "#size-cells")) {
            if (prop_length != 4) {
              kernel_panic("Malformed DTB: #size-cells value is not 32-bit");
              return;
            }
            size_cells_initialized = true;
            out_hw_info->size_cells =
                read_be32_from_address((uintptr_t)current_data_bytes);
          }
        }

        // Check if current property is a property that the node specifying
        // physical RAM has: device_type (should have value "memory") or
        // reg (has one or more regions, depending on property size)
        if (string_compare(prop_name, "device_type")) {
          if (string_compare((char *)current_data_bytes, "memory")) {
            current_node_device_type = "memory";
          }
        } else if (string_compare(prop_name, "reg")) {
          current_node_reg = current_data_bytes;
          current_node_reg_size = prop_length;
        }
        if (current_node_reg) {
          if (string_compare(current_node_device_type, "memory")) {
            // Current node specifie physical memory: device_type == "memory"
            // and reg is a property of the current node
            if (address_cells_initialized && size_cells_initialized) {
              physical_memory_regions(
                  out_hw_info->physical_memory_regions,
                  &(out_hw_info->physical_memory_region_count),
                  out_hw_info->address_cells, out_hw_info->size_cells,
                  current_node_reg, current_node_reg_size);
            } else {
              kernel_panic(
                  "Malformed DTB: must initialize both address cells and size "
                  "cells before reading memory reg");
              return;
            }
          }
        }

        break;
      case FDT_NOP:
        // Do nothing
        break;
      case FDT_END:
        ++end_tokens_encountered;
        // Move cursor to end, so while loop terminates
        dtb_cursor = struct_end;
        break;
      default:
        kernel_panic("Malformed DTB: Unrecognized DTB token");
        return;
    }
  }
  // Check all node start tokens balanced by node end tokens.
  if (node_depth != 0) {
    kernel_panic("Malformed DTB: node begin / end token mismatch");
    return;
  }
  // Check end token was encountered once
  if (end_tokens_encountered != 1u) {
    kernel_panic(
        "Malformed DTB: DTB did not include exactly one FDT_END token");
  }

  // Print hardware info to console
  console_print("Memory addresses are 0x");
  const uint32_t address_bits = 32 * out_hw_info->address_cells;
  const uint32_t size_bits = 32 * out_hw_info->size_cells;
  console_print_hex_value((void *)&address_bits, 4);
  console_print(" bits\nMemory region sizes are 0x");
  console_print_hex_value((void *)&size_bits, 4);
  console_print(" bits \n");
}
