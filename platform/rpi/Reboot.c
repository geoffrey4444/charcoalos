// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/rpi/IO.h"
#include "kernel/Main/Lifecycle.h"

#include <stddef.h>
#include <stdint.h>

void platform_reboot(void) {
  // NOTE: later, when I have multiple cores and interrupts, first
  // disable interrupts and shut down other cores.
  //
  // 0xFE100000 = watchdog address base
  //
  // Set watchdog timer: write PM_PASSWORD | ticks to PM_WDOG
  // Request full reset: write PM_PASSWORD | <reset-config-bits> to PM_RSTC.
  // 4.	Halt, await reset

  // Volatile: compiler can't reorder or optimize out
  volatile uint32_t *rstc = (uint32_t *)WDOG_PM_RSTC;
  volatile uint32_t *wdog = (uint32_t *)WDOG_PM_WDOG;

  // wait a small time (order microseconds) for the reset request to fire
  // These are "raw wdog ticks", not seconds or any actual time unit.
  uint32_t ticks_to_time_out = 10;

  // Set the watchdog timer to a small value
  // "or in" the password, as those bits must also be set high for the
  // reset to work.
  *wdog = WDOG_PM_PASSWORD | ticks_to_time_out;

  // When the watchdog fires, to trigger a reset, set a field in
  // WDOG_PM_RTSC to 0x20 ("full reset")...but don't overwrite the rest
  // of rstc.
  uint32_t value = *rstc;
  value &=
      WDOG_PM_RSTC_WRCFG_CLR;  // "and in" the clear mask to clear that part
  value |= WDOG_PM_PASSWORD | WDOG_PM_RSTC_FULL_RESET;  // "or in" to set
  *rstc = value;

  // Spin until reset
  kernel_halt();
}
