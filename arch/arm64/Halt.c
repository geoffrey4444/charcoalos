// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "platform/IO.h"

void halt(void) {
  // Flush UART TX before spinning
  platform_console_tx_flush();

  // Put the processor politely to sleep
  // wfe = wait for next event in arm64 assembly
  // __asm__ embeds assembly
  for (;;) {
    __asm__ volatile("wfe");
  }
}
