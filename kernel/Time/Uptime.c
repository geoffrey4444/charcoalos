// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Time/Uptime.h"
#include "arch/Interrupt.h"
#include <stdint.h>

static volatile uint64_t g_ticks_since_boot = 0;

uint64_t uptime(void) {
  return (1000 * g_ticks_since_boot) / interrupt_frequency_in_hz();
}

void increment_uptime_by_one_tick(void) { ++g_ticks_since_boot; }
