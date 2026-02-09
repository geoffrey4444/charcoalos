// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/virt/IO.h"

#include <stdint.h>

uintptr_t mmio_base_address(void) {
  return MMIO_BASE;
}

uintptr_t uart_base_address(void) {
  return UART0_BASE;
}
