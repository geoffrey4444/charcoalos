// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    defined(__ORDER_BIG_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CHARCOALOS_LITTLE_ENDIAN 1
#define CHARCOALOS_BIG_ENDIAN 0
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CHARCOALOS_LITTLE_ENDIAN 0
#define CHARCOALOS_BIG_ENDIAN 1
#else
#error "Unknown byte order"
#endif
#else
#error "Cannot determine byte order on this platform"
#endif

/*!
 * \brief Endian 16-bit conversion function from big-endian to host byte order.
 */
static inline uint16_t be16toh_u16(uint16_t big_endian_value) {
  if (CHARCOALOS_LITTLE_ENDIAN) {
    return __builtin_bswap16(big_endian_value);
  } else {
    return big_endian_value;
  }
}

/*!
 * \brief Endian 32-bit conversion function from big-endian to host byte order.
 */
static inline uint32_t be32toh_u32(uint32_t big_endian_value) {
  if (CHARCOALOS_LITTLE_ENDIAN) {
    return __builtin_bswap32(big_endian_value);
  } else {
    return big_endian_value;
  }
}

/*!
 * \brief Endian 64-bit conversion function from big-endian to host byte order.
 */
static inline uint64_t be64toh_u64(uint64_t big_endian_value) {
  if (CHARCOALOS_LITTLE_ENDIAN) {
    return __builtin_bswap64(big_endian_value);
  } else {
    return big_endian_value;
  }
}

/*!
 * \brief Load a big-endian 32-bit unsigned integer from a byte pointer
 * \details The read is correct regardless of host byte order. Use shifts
 * to force each of the 4 bytes to read as a big-endian value, most
 * significant byte first.
 */
static inline uint32_t read_be32_from_address(uintptr_t address) {
  const uint8_t *byte_ptr = (const uint8_t *)address;
  // Shift each byte to the correct position and combine with bitwise OR
  return ((uint32_t)byte_ptr[0] << 24) | ((uint32_t)byte_ptr[1] << 16) |
         ((uint32_t)byte_ptr[2] << 8) | ((uint32_t)byte_ptr[3]);
}

/*!
 * \brief Load a big-endian 64-bit unsigned integer from a byte pointer
 * \details The read is correct regardless of host byte order. Use shifts
 * to force each of the 8 bytes to read as a big-endian value, most
 * significant byte first.
 */
static inline uint64_t read_be64_from_address(uintptr_t address) {
  const uint8_t *byte_ptr = (const uint8_t *)address;
  // Shift each byte to the correct position and combine with bitwise OR
  return ((uint64_t)byte_ptr[0] << 56) | ((uint64_t)byte_ptr[1] << 48) |
         ((uint64_t)byte_ptr[2] << 40) | ((uint64_t)byte_ptr[3] << 32) |
         ((uint64_t)byte_ptr[4] << 24) | ((uint64_t)byte_ptr[5] << 16) |
         ((uint64_t)byte_ptr[6] << 8) | ((uint64_t)byte_ptr[7]);
}
