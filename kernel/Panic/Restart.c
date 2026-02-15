// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "platform/Reboot.h"

void kernel_restart(void) {
  platform_reboot();
}
