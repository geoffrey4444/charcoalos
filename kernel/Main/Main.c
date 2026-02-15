// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Main/Lifecycle.h"

#include <stdbool.h>

void kmain(void) {  
  
  kernel_init();
  kernel_run();
  kernel_halt();
}
