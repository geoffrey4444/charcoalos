// Distributed under the MIT license.
// See LICENSE.txt for details.

// Build
// aarch64-elf-as boot.s -o boot.o
// aarach64-elf-gcc -ffreestanding -c kernel.c -o kernel.o
// aarch64-elf-ld -nostdlib -Tlinker.ld boot.o kernel.o -o kernel.elf
// Run
// qemu-system-aarch64 -machine virt,accel=hvf -cpu host -m 512M -nographic
// -serial mon:stdio -kernel kernel.elf While running: CTRL-A x exits, CTRL-A c
// switches, CTRL-A h help
//   mon is qemu monitor, --serial mon:stdio sets uart to stdio with monitor
// DTB = device table, bare metal boot puts it at start of RAM
// Examine with aarch64-elf-objdump

#include <stdint.h>

// volatile: do not optimize away or cache with repeated reads
//           don't reorder vs other volatile accesses
//           basically stops compiler from optimizing stuff away
// static: only this file can see it
// const after *: pointer address is constant
static volatile uint8_t* const uart = (uint8_t*)0x09000000;

void uart_putchar(char character) {
  *uart = (uint8_t)character;  // write one character to uart_tx
}

void uart_print(const char* text) {
  while (*text != '\0') {
    // Continue untl reaching string null terminator
    // Note: we're assuming the characters can be put quickly, and whatever
    // backs it (FIFO of some size) doesn't fill up.
    uart_putchar(*text);
    text++;
  }
}

void kmain(void) { uart_print("Hello, world!\r\n"); }
