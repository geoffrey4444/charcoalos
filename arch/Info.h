// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief Gets the current exception level (e.g. EL0/EL1 on AARCH64).
 * \returns The exception level
 */
size_t current_exception_level(void);

/*!
 * \brief Gets the MPIDR_EL1 system register value.
 * \returns The MPIDR_EL1 value
 */
size_t mpidr_el1(void);

/*!
 * \brief Gets the SCTLR_EL1 system register value.
 * \returns The SCTLR_EL1 value
 */
size_t sctlr_el1(void);

const char* arch_name(void);
