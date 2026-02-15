// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief Handles exceptions.
 * \details All exceptions are handled by calling this function, which
 * provide a pointer to saved registers `saved_registers` and an exception
 * type code `kind_of_exception`, defined in the architecture's
 * ExceptionTypes.h.
 * \param saved_registers A pointer to an array of 32 64-bit values from
 * registers x0-x30 and sp, in that order.
 * \param kind_of_exception Type of exception (see ExceptionTypes.h) for
 * the architecture for details.
 * \returns An exception action code (see ExceptionTypes.h) for the architecture
 * for details.
 */
uint64_t handle_exception(uint64_t* saved_registers,
                          uint64_t kind_of_exception);
