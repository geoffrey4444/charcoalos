// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/halt.h"
#include "kernel/Console/IO.h"

#include <stdint.h>

void main(void) {
  console_print("Welcome to CharcoalOS.\n");
  halt();
  return;
}
