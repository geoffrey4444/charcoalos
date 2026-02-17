// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Main/Lifecycle.h"

#include <stdbool.h>
#include <stdint.h>

void kmain(uintptr_t dtb) {  
  
  kernel_init(dtb);
  kernel_run();
  kernel_halt();
}
