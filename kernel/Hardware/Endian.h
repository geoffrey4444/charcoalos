// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief Endian 16-bit conversion function from big-endian to host byte order.
 * \details The device tree blob (dtb) is big-endian. But supported hosts
 * (e.g. QEMU virt arm64, Raspberry Pi 4) are little-endian.
 * This function assumes the host is little-endian.
 */
static inline uint16_t be16toh_u16(uint16_t big_endian_value) {
  return __builtin_bswap16(big_endian_value);
}

/*!
 * \brief Endian 32-bit conversion function from big-endian to host byte order.
 * \details The device tree blob (dtb) is big-endian. But supported hosts
 * (e.g. QEMU virt arm64, Raspberry Pi 4) are little-endian.
 * This function assumes the host is little-endian.
 */
static inline uint32_t be32toh_u32(uint32_t big_endian_value) {
  return __builtin_bswap32(big_endian_value);
}

/*!
 * \brief Endian 64-bit conversion function from big-endian to host byte order.
 * \details The device tree blob (dtb) is big-endian. But supported hosts
 * (e.g. QEMU virt arm64, Raspberry Pi 4) are little-endian.
 * This function assumes the host is little-endian.
 */
static inline uint64_t be64toh_u64(uint64_t big_endian_value) {
  return __builtin_bswap64(big_endian_value);
}
