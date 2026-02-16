// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief Returns system uptime as milliseconds since boot as an integer.
 * \returns Uptime in milliseconds as uint64_t
 */
uint64_t uptime(void);

/*!
 * \brief Increments the uptime timer by one tick
 */
void increment_uptime_by_one_tick(void);
