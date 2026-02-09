// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief Gets the base address of the MMIO region.
 * \returns The MMIO base address
 */
uintptr_t mmio_base_address(void);

/*!
 * \brief Gets the UART base address.
 * \returns The UART base address
 */
uintptr_t uart_base_address(void);
