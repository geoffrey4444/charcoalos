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
 */
void handle_exception(uint64_t* saved_registers, uint64_t kind_of_exception);
