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

  volatile uint32_t *out_dtb_header = (volatile uint32_t *)&out_hw_info->header;

  const uint32_t magic = be32toh_u32(*(uint32_t *)(dtb));
  out_dtb_header[0] = magic;
  console_print("0x");
  console_print_hex((void *)&magic, 4);
  console_print("\n");
  if (magic != 0xd00dfeed) {
    kernel_panic(
        "The hardware could not be recognized because the device table blob "
        "is not in the expected format.");
    return;
  }

  out_dtb_header[1] = be32toh_u32(*(uint32_t *)(dtb + 4u));
  out_dtb_header[2] = be32toh_u32(*(uint32_t *)(dtb + 8u));
  out_dtb_header[3] = be32toh_u32(*(uint32_t *)(dtb + 12u));
  out_dtb_header[4] = be32toh_u32(*(uint32_t *)(dtb + 16u));
  out_dtb_header[5] = be32toh_u32(*(uint32_t *)(dtb + 20u));
  out_dtb_header[6] = be32toh_u32(*(uint32_t *)(dtb + 24u));
  out_dtb_header[7] = be32toh_u32(*(uint32_t *)(dtb + 28u));
  out_dtb_header[8] = be32toh_u32(*(uint32_t *)(dtb + 32u));
  out_dtb_header[9] = be32toh_u32(*(uint32_t *)(dtb + 36u));

  // Device table is valid
  console_print("Device table blob recognized with magic ");
  console_print_hex((void *)&magic, 4);
  console_print("\n");
}
